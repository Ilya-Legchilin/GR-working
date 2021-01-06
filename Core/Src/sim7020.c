#include "sim7020.h"
#include "uart_simple_protocol.h"


sim7020_t sim7020;

/***|********************|
    |*                  *|
    |*  COMMANDS STACK  *|
    |*                  *|
    |********************|
*/

#define MAX_WORK_TIME_MS 70000

static at_cmd_t cmd_stack[40] = {AT_NONE};
static char temp_tcp_send[128];
const char send_ok[] = "\r\nSEND OK\r\n";
const char connect_ok[] = "\r\nCONNECT OK\r\n";
uint8_t temp_ip[32];
/**** Pull Function
    * @brief This function takes first cmd from stack and resorts stack
    * @param None
    * @retval at_cmd_t
    */
static at_cmd_t cmd_stack_pull()
{
    at_cmd_t res = cmd_stack[0];

    if (res != AT_NONE) {
        uint8_t i  = 1;
        while(cmd_stack[i] != AT_NONE) {
            cmd_stack[i - 1] = cmd_stack[i];
            i += 1;
        }
        cmd_stack[i - 1] = AT_NONE;
    }

    return res;
}


/**** Push Function
    * @brief This function pushes your cmd to the end of stack
    * @param at_cmd_t
    * @retval None
    */
static void cmd_stack_push(at_cmd_t cmd)
{
    uint8_t i = 0;
    while(cmd_stack[i] != AT_NONE) i += 1;
    if (i > 1)
        if (cmd_stack[i - 1] == cmd) return;
    cmd_stack[i] = cmd;
}


/**** Force Push Function
    * @brief This function pushes your cmd to the start of stack
    * @param at_cmd_t
    * @retval None
    */
static void cmd_stack_force_push(at_cmd_t cmd)
{
    uint8_t i = 0;
    at_cmd_t tmp;
    while(cmd != AT_NONE) {
        tmp = cmd_stack[i];
        cmd_stack[i] = cmd;
        cmd = tmp;
        i += 1;
    }
}


/**** Check Stack Function
    * @brief This function checks stack on commands
    * @param None
    * @retval 0 if stack is empty
    */
static uint8_t cmd_stack_is_empty(void)
{
    return cmd_stack[0] == AT_NONE;
}

void cmd_stack_clear()
{
    uint8_t i;
    for (i = 0; i < sizeof(cmd_stack); ++i)
        cmd_stack[i] = AT_NONE;
}


/***|*********************|
    |*                   *|
    |*  TRANSPORT LAYER  *|
    |*                   *|
    |*********************|
*/
static struct wtimer_desc sim7020_desc;
static uint32_t last_msg_time = 0;
static uint16_t timeout;

extern uint8_t sim7020_tra_rx_buff[];
extern uint8_t sim7020_tra_rx_buff_p;
extern uint8_t sim7020_tra_tx_buff[];

static void sim7020_processor(struct wtimer_desc *desc)
{
    if (!sim7020.state) return;

    if (sim7020.active_cmd) {
        if (sim7020_tra_rx_buff_p && sim7020.active_cmd != WAIT_ONE_SEC)
        {
            timeout = 0;
            sim7020.error = 0;
            sim7020.last_cmd = sim7020.active_cmd;
            switch (sim7020.active_cmd) {
            case POWER_ON:
                break;
            case PING:
                if (    strstr("'\r\r\nOK\r\n",     sim7020_tra_rx_buff) \
                    ||  strstr("AT\r\r\nOK\r\n",    sim7020_tra_rx_buff) \
                    ||  strstr("\r\nOK\r\n",        sim7020_tra_rx_buff))
                    sim7020.state = SIM7020_CONNECTED_ECHO;
                else
                    sim7020.error = SIM7020_BAD_ANSWER;
                break;
            case ECHO_OFF:
                if (    strstr("\r\nOK\r\n",        sim7020_tra_rx_buff) \
                    ||  strstr("ATE0\r\r\nOK\r\n",  sim7020_tra_rx_buff) \
                    ||  strstr("ATE0\r\nOK\r\n",    sim7020_tra_rx_buff))
                    sim7020.state = SIM7020_CONNECTED;
                else
                    sim7020.error = SIM7020_BAD_ANSWER;
                break;
            case CHECK_SIM:
                if (strstr("\r\n+CPIN: READY\r\n\r\nOK\r\n", sim7020_tra_rx_buff) || 
                    strstr("\r\n+CPIN: READY\r\n", sim7020_tra_rx_buff))
                    sim7020.state = SIM7020_SIM_CONNECTED;
                else
                    sim7020.error = SIM7020_BAD_ANSWER;
                break;
            case GET_CSQ:
                if (strnstr("\r\n+CSQ:", sim7020_tra_rx_buff, 0)){
                      sim7020.sq = str2uint8(sim7020_tra_rx_buff);
                        if(sim7020.sq == 0) {
                            cmd_stack_force_push(GET_CSQ);
                            cmd_stack_force_push(WAIT_ONE_SEC);
                            cmd_stack_force_push(WAIT_ONE_SEC);
                        }        
                } 
                else
                    sim7020.error = SIM7020_BAD_ANSWER;
                break;
            case TCP_GET_IP:
                if (str_is_ip(sim7020_tra_rx_buff + 2)){
                    tcp.state = TCP_IP_GOTTEN;
                }
                else
                    sim7020.error = SIM7020_BAD_ANSWER;
                break;
                
            case TCP_SEND:
                if (strstr("\r\n> ", sim7020_tra_rx_buff) || \
                     (strstr("> ", sim7020_tra_rx_buff))) {
                    tcp_prepair_msg_to_send(sim7020_tra_tx_buff);
                    char end[2] = {0x1A, '\0'};
                    stradd(end, sim7020_tra_tx_buff);
                    UART_SendString(sim7020_tra_tx_buff);
                    sim7020_phy_send(sim7020_tra_tx_buff);
                    ((tcp_t*)sim7020.app)->state = TCP_SENDING;
                } else {
                    sim7020.error = SIM7020_TCP_ERROR;
                }
                break;
             case TCP_GET_STATUS:
                if (strstr("\r\nOK\r\n\r\nSTATE: IP INITIAL\r\n", sim7020_tra_rx_buff) ||
                    strstr("\r\nSTATE: IP INITIAL\r\n", sim7020_tra_rx_buff)) {
                    ((tcp_t*)sim7020.app)->cstate = TCP_IP_INITIAL;
                } else if (strstr("\r\nOK\r\n\r\nSTATE: IP START\r\n", sim7020_tra_rx_buff) ||
                    strstr("\r\nSTATE: IP START\r\n", sim7020_tra_rx_buff)) {
                    ((tcp_t*)sim7020.app)->cstate = TCP_IP_START;
                } else if (strstr("\r\nOK\r\n\r\nSTATE: IP STATUS\r\n", sim7020_tra_rx_buff) ||
                    strstr("\r\nSTATE: IP STATUS\r\n", sim7020_tra_rx_buff)) {
                    ((tcp_t*)sim7020.app)->cstate = TCP_IP_STATUS;
                } else if (strstr("\r\nOK\r\n\r\nSTATE: PDP DEACT\r\n", sim7020_tra_rx_buff) ||
                    strstr("\r\nSTATE: PDP DEACT\r\n", sim7020_tra_rx_buff)) {
                    ((tcp_t*)sim7020.app)->cstate = TCP_PDP_DEACT;
                } else if (strstr("\r\nOK\r\n\r\nSTATE: IP CONFIG\r\n", sim7020_tra_rx_buff) ||
                    strstr("\r\nSTATE: IP CONFIG\r\n", sim7020_tra_rx_buff)) {
                    ((tcp_t*)sim7020.app)->cstate = TCP_IP_CONFIG;
                } else if (strstr("\r\nOK\r\n\r\nSTATE: CONNECT OK\r\n", sim7020_tra_rx_buff) ||
                    strstr("\r\nSTATE: CONNECT OK\r\n", sim7020_tra_rx_buff)) {
                    ((tcp_t*)sim7020.app)->cstate = TCP_CONNECT_ON;
                    ((tcp_t*)sim7020.app)->state = TCP_CONNECTED;
                } else if (strstr("\r\nOK\r\n\r\nSTATE: TCP CLOSED\r\n", sim7020_tra_rx_buff) ||
                    strstr("\r\nSTATE: TCP CLOSED\r\n", sim7020_tra_rx_buff)) {
                    ((tcp_t*)sim7020.app)->cstate = TCP_CONNECT_CLOSED;
                    if (((tcp_t*)sim7020.app)->state > TCP_TASK_STARTED)
                        ((tcp_t*)sim7020.app)->state = TCP_TASK_STARTED;
                } else if (strstr("\r\nOK\r\n\r\nSTATE: TCP CONNECTING\r\n", sim7020_tra_rx_buff) ||
                    strstr("\r\nSTATE: TCP CONNECTING\r\n", sim7020_tra_rx_buff)) {
                    ((tcp_t*)sim7020.app)->cstate = TCP_CONNECTING;
                } else {
                    sim7020.error = SIM7020_BAD_ANSWER;
                }
                break;
            case TCP_SET_ONE_SOCKET_MODE:
                if (strstr("\r\nOK\r\n", sim7020_tra_rx_buff))
                    ((tcp_t*)sim7020.app)->state = TCP_ONE_SOCKET_MODE;
                else
                    sim7020.error = SIM7020_BAD_ANSWER;
                break;
            case TCP_SET_COMMAND_MODE:
                if (strstr("\r\nOK\r\n", sim7020_tra_rx_buff))
                    ((tcp_t*)sim7020.app)->state = TCP_COMMAND_MODE;
                else
                    sim7020.error = SIM7020_BAD_ANSWER;
                break;
            case TCP_ACTIVATE_CONTEXT:
                if (strstr("\r\nOK\r\n", sim7020_tra_rx_buff))
                    ((tcp_t*)sim7020.app)->state = TCP_CONTEXT_ACTIVATED;
                else {
                    //sim7020.error = SIM7020_BAD_ANSWER;
                    cmd_stack_force_push(TCP_ACTIVATE_CONTEXT);
                    cmd_stack_force_push(TCP_START_TASK);
                    cmd_stack_force_push(TCP_DEACTIVATE_CONTEXT);
                }
                break;
            case TCP_DEACTIVATE_CONTEXT:
                if (strstr("\r\nSHUT OK\r\n", sim7020_tra_rx_buff)) {
                    ((tcp_t*)sim7020.app)->state = TCP_COMMAND_MODE;
                    ((tcp_t*)sim7020.app)->cstate = TCP_IP_INITIAL;
                } else {
                    sim7020.error = SIM7020_BAD_ANSWER;
                }
                break;
            case TCP_START_TASK:
                if (strstr("\r\nOK\r\n", sim7020_tra_rx_buff)) {
                    ((tcp_t*)sim7020.app)->state = TCP_TASK_STARTED;
                    ((tcp_t*)sim7020.app)->cstate = TCP_IP_START;
                } else {
                    sim7020.error = SIM7020_BAD_ANSWER;
                }
                break;
            case TCP_OPEN_SOCKET:
                if (strstr("\r\nOK\r\n", sim7020_tra_rx_buff))
                    ((tcp_t*)sim7020.app)->state = TCP_SOCKET_ACTIVATED;
                else
                    sim7020.error = SIM7020_BAD_ANSWER;
                break;
            case TCP_WAIT_OPEN_SOCKET:
                if (strstr("\r\nCONNECT OK\r\n", sim7020_tra_rx_buff))
                   ((tcp_t*)sim7020.app)->state = TCP_CONNECTED;
               else
                    sim7020.error = SIM7020_BAD_ANSWER;
                break;
            case SET_CONTYPE:
            case SET_APN:
            case SET_USER:
            case SET_PWD:
                if (strstr("\r\nOK\r\n", sim7020_tra_rx_buff)) {
                    if ( (sim7020.state >= SIM7020_SIM_CONNECTED) && (SIM7020_SIM_CONNECTED < SIM7020_SIM_READY) )
                        sim7020.state += 1;
                        sim7020.state = SIM7020_SIM_READY;
                } else {
                    sim7020.state = SIM7020_SIM_CONNECTED;
                    sim7020.error = SIM7020_BAD_ANSWER;
                }
                break;

            default:
                sim7020.error = SIM7020_BAD_CMD;
                break;
            }

            sim7020.last_cmd = sim7020.active_cmd;

            if (!sim7020.error) {
                //nothing now
            } else if (sim7020.error == SIM7020_BAD_ANSWER) {
                if ( strnstr("\r\nCall", sim7020_tra_rx_buff, 0))
                    sim7020.error = SIM7020_NO_PING;
                else if ( !(strstr("\r\nERROR\r\n", sim7020_tra_rx_buff)) )
                    sim7020.error = SIM7020_INCORRECT_ANSWER;
            }
            sim7020.active_cmd = AT_NONE;
            sim7020_tra_rx_buff_p = 0;
        }
    } else if (sim7020_tra_rx_buff_p) { //parse unsolisited messages
        if (0) {}
        else if (sim7020.state == SIM7020_SIM_READY) {
            if (((tcp_t*)sim7020.app)->state >= TCP_SOCKET_ACTIVATED) {
                if (((tcp_t*)sim7020.app)->state >= TCP_CONNECTED) {
                    if (((tcp_t*)sim7020.app)->state == TCP_SENDING) {
                        if (strnstr(send_ok, sim7020_tra_rx_buff, 0)) {
                            ((tcp_t*)sim7020.app)->error = TCP_OK;
                            sim7020_tra_rx_buff_p = 0;
                        } else {
                             tcp_rx_callback(sim7020_tra_rx_buff); //{"pd":3600,"tf":4000}
                             sim7020_tra_rx_buff_p = 0;            //
                        }
                        ((tcp_t*)sim7020.app)->state = TCP_CONNECTED;
                    } else if (strstr("\r\nCLOSED\r\n", sim7020_tra_rx_buff)) {
                            ((tcp_t*)sim7020.app)->state = TCP_IP_GOTTEN;
                            tcp_closed_callback();
                            sim7020_tra_rx_buff_p = 0;
                    } else {
                        tcp_rx_callback(sim7020_tra_rx_buff); //first callback - to get salt
                        sim7020_tra_rx_buff_p = 0;
                    }
                }
            } else 
              sim7020_tra_rx_buff_p = 0;
        }
        else
          sim7020_tra_rx_buff_p = 0;

    } else if (!cmd_stack_is_empty()) { // no active command
        sim7020.active_cmd = cmd_stack_pull();
        timeout = 0;
        if (sim7020.active_cmd)
        {
            switch ( sim7020.active_cmd ) {
            case POWER_ON:
                timeout = 2000;
                break;
            case POWER_OFF:
                sim7020_phy_power_off();
                sim7020.state = SIM7020_IS_OFF;
                timeout = 2000;
                break; 
            case PING:
                if (sim7020.state >= SIM7020_IS_ON) {
                    sim7020_phy_send("AT\r\n");
                    timeout = 1500;
                } else sim7020.error = SIM7020_NO_POWER;
                break;
            case ECHO_OFF:
                if (sim7020.state == SIM7020_CONNECTED_ECHO) {
                    sim7020_phy_send("ATE0\r\n");
                    timeout = 1500;
                } else sim7020.error = SIM7020_NO_PING;
                break;
            case CHECK_SIM:
                if (sim7020.state >= SIM7020_CONNECTED) {
                    sim7020_phy_send("AT+CPIN?\r\n");
                    timeout = 2500;
                } else sim7020.error = SIM7020_ECHO_ENABLED;
                break;
            case SET_APN:
                if (sim7020.state >= SIM7020_CONNECTED) {
                    //stradd("AT+SAPBR=3,1,\"APN\",\"",   sim7020_tra_tx_buff);
                    //stradd(sim7020.simcard.apn,          sim7020_tra_tx_buff);
                    //stradd("\"\r\n",                    sim7020_tra_tx_buff);
                    sim7020_phy_send("AT+CSTT\n\r");
                    timeout = 1500;
                } else sim7020.error = SIM7020_NO_SIM;
                break;
            case GET_CSQ:
                if (sim7020.state == SIM7020_SIM_READY) {
                    sim7020_phy_send("AT+CSQ\r\n");
                    timeout = 500;
                } else {
                    sim7020.error = SIM7020_NO_NET;
                }
                break;
            case TCP_GET_STATUS:
                if (sim7020.state == SIM7020_SIM_READY) {
                    sim7020_phy_send("AT+CIPSTATUS\r\n");
                    timeout = 500;
                } else {
                    sim7020.error = SIM7020_NO_NET;
                }
                break;
            case TCP_SET_ONE_SOCKET_MODE:
                if (sim7020.state == SIM7020_SIM_READY) {
                    if (((tcp_t*)sim7020.app)->cstate == TCP_IP_INITIAL) {
                        sim7020_phy_send("AT+CIPMUX=0\r\n");
                        timeout = 500;
                    } else {
                        sim7020.active_cmd = AT_NONE;
                        cmd_stack_force_push(TCP_SET_ONE_SOCKET_MODE);
                        cmd_stack_force_push(TCP_GET_STATUS);
                        cmd_stack_force_push(TCP_DEACTIVATE_CONTEXT);
                    }
                } else {
                    sim7020.error = SIM7020_NO_NET;
                }
                break;
             case TCP_SET_COMMAND_MODE:
                if (sim7020.state == SIM7020_SIM_READY) {
                    if (((tcp_t*)sim7020.app)->cstate == TCP_IP_INITIAL) {
                        sim7020_phy_send("AT+CIPMODE=0\r\n");
                        timeout = 500;
                    } else {
                        sim7020.active_cmd = AT_NONE;
                        cmd_stack_force_push(TCP_SET_COMMAND_MODE);
                        cmd_stack_force_push(TCP_GET_STATUS);
                        cmd_stack_force_push(TCP_DEACTIVATE_CONTEXT);
                    }
                } else {
                    sim7020.error = SIM7020_NO_NET;
                }
                break;
            case TCP_START_TASK:
                if (sim7020.state == SIM7020_SIM_READY) {
                     if (((tcp_t*)sim7020.app)->state ==  TCP_COMMAND_MODE) {
                        if (((tcp_t*)sim7020.app)->cstate !=  TCP_IP_INITIAL){
                            cmd_stack_force_push(TCP_START_TASK);
                            cmd_stack_force_push(TCP_GET_STATUS);
                            cmd_stack_force_push(TCP_DEACTIVATE_CONTEXT);
                        }
                        sim7020_phy_send("AT+CSTT\r\n");
                        timeout = 500;
                     } else {
                        ((tcp_t*)sim7020.app)->error = TCP_SETUP_ERROR;
                     }
                } else {
                    sim7020.error = SIM7020_NO_NET;
                }
                break;
            case TCP_ACTIVATE_CONTEXT:
                if (sim7020.state == SIM7020_SIM_READY) {
                     if (((tcp_t*)sim7020.app)->state == TCP_TASK_STARTED) {
                       if (((tcp_t*)sim7020.app)->cstate != TCP_IP_START){
                          cmd_stack_force_push(TCP_ACTIVATE_CONTEXT);
                          cmd_stack_force_push(TCP_START_TASK);
                          cmd_stack_force_push(TCP_GET_STATUS);
                          cmd_stack_force_push(TCP_DEACTIVATE_CONTEXT);
                          break;
                       }
                        sim7020_phy_send("AT+CIICR\r\n");
                        timeout = 20000;
                     } else {
                        ((tcp_t*)sim7020.app)->error = TCP_SETUP_ERROR;
                        sim7020.error = SIM7020_TCP_ERROR;
                     }
                } else {
                    sim7020.error = SIM7020_NO_NET;
                }
                break;
            case TCP_DEACTIVATE_CONTEXT:
                if (sim7020.state == SIM7020_SIM_READY) {
                     if (((tcp_t*)sim7020.app)->state >= TCP_STARTED) {
                        sim7020_phy_send("AT+CIPSHUT\r\n");
                        timeout = 4000;
                     } else {
                        ((tcp_t*)sim7020.app)->error = TCP_SETUP_ERROR;
                     }
                } else {
                    sim7020.error = SIM7020_NO_NET;
                }
                break;
            case TCP_GET_IP:
                if (sim7020.state == SIM7020_SIM_READY) {
                     if (((tcp_t*)sim7020.app)->state >= TCP_CONTEXT_ACTIVATED) {
                        sim7020_phy_send("AT+CIFSR\r\n");
                        timeout = 1500;
                     } else {
                        ((tcp_t*)sim7020.app)->error = TCP_SETUP_ERROR;
                     }
                } else {
                    sim7020.error = SIM7020_NO_NET;
                }
                break;
            case TCP_OPEN_SOCKET:
                if (sim7020.state == SIM7020_SIM_READY){
                  if (((tcp_t*)sim7020.app)->state == TCP_IP_GOTTEN) {
                    UART_SendString("AT+CIPSTART=\"TCP\",\"185.119.57.213\",4001\r\n");
                    sim7020_phy_send("AT+CIPSTART=\"TCP\",\"185.119.57.213\",4001\r\n");
                    timeout = 5000;
                  } else{
                    ((tcp_t*)sim7020.app)->error = TCP_SETUP_ERROR;
                  }
                } else {
                    sim7020.error = SIM7020_NO_NET;
                }
                break;
            case TCP_WAIT_OPEN_SOCKET:
                if (sim7020.state == SIM7020_SIM_READY) {
                    if (((tcp_t*)sim7020.app)->state == TCP_SOCKET_ACTIVATED) {
                        timeout =15000;
                    } else {
                        ((tcp_t*)sim7020.app)->error = TCP_SETUP_ERROR;
                    }
                } else {
                    sim7020.error = SIM7020_NO_NET;
                }
                break;

                
            case TCP_SEND:
                if (sim7020.state >= SIM7020_IS_ON) {
                    sim7020_phy_send("AT+CIPSEND\r\n");
                    timeout = 1500;
                } else {
                    sim7020.error = SIM7020_NO_NET;
                }
                break;
            case WAIT_ONE_SEC:
                timeout = 1000;
                break;
            case WAIT_TWO_SEC:
                timeout = 2000;
                break;
            case WAIT_FOUR_SEC:
                timeout = 4000;
                break;
            case WAIT_TEN_SEC:
                timeout = 10000;
                break;
            }

            if (sim7020.error) {
                sim7020.last_cmd = sim7020.active_cmd;
                sim7020.active_cmd = AT_NONE;
            }

            last_msg_time = sim7020.work_time_ms;
        }
    }
    

    if (timeout && (sim7020.work_time_ms > last_msg_time + timeout))
    {
        if (sim7020.active_cmd == WAIT_ONE_SEC)
          sim7020.active_cmd = AT_NONE;
        else if (sim7020.active_cmd != POWER_ON) {
            sim7020.last_cmd = sim7020.active_cmd;
            sim7020.active_cmd = AT_NONE;
            sim7020.error = SIM7020_TIMEOUT;
        } else {
            sim7020.active_cmd = AT_NONE;
            sim7020.state = SIM7020_IS_ON;
        }
        timeout = 0;
    }

    if (sim7020.error) {
        if ( ((sim7020.error == SIM7020_INCORRECT_ANSWER) || (sim7020.error == SIM7020_TIMEOUT) ) && (sim7020.tries < SIM7020_TRIES)) {
            cmd_stack_force_push(sim7020.last_cmd);
            sim7020.tries += 1;
        } else if ((sim7020.error == SIM7020_ECHO_ENABLED) && (sim7020.tries < SIM7020_TRIES)) {
            cmd_stack_force_push(ECHO_OFF);
            sim7020.tries += 1;
        } else if ((sim7020.error == SIM7020_NO_PING) && (sim7020.tries < SIM7020_TRIES)) {
            cmd_stack_force_push(PING);
            sim7020.tries += 1;
        } else if ((sim7020.error == SIM7020_NO_SIM) && (sim7020.tries < SIM7020_TRIES)) {
            cmd_stack_force_push(CHECK_SIM);
            sim7020.tries += 1;
        } else {
            //sim7020_error_callback();
            sim7020.tries = 0;
            sim7020.error = 0;
            sim7020.active_cmd = AT_NONE;
        }
    }
    
    if (sim7020.work_time_ms > MAX_WORK_TIME_MS) {
        //sim7020.error = SIM800_WORK_TIME_TIMEOUT;
        //sim7020_error_callback();
        cmd_stack_clear();
        sim7020_phy_power_off();
        sim7020.state = 0;
    }
    
    sim7020.work_time_ms += 100;
    ScheduleTask(desc, 0, RELATIVE, MILLISECONDS(100));
}

static struct wtimer_desc sim7020_desc;

void sim7020_start()
{
    sim7020_phy_power_on();
    sim7020.state = SIM7020_IS_ON;
    sim7020.tries = 0;
    sim7020.error = 0;
    sim7020.sq = 0;
    sim7020.bypass = 0;
    sim7020.work_time_ms = 0;
    sim7020.active_cmd = AT_NONE;
    sim7020.last_cmd = AT_NONE;
    
    
    /*strcp("internet",   sim7020.simcard.apn);
    strcp("gdata",      sim7020.simcard.user);
    strcp("gdata",      sim7020.simcard.pwd);*/
    
    
    cmd_stack_push(POWER_ON);
    cmd_stack_push(PING);
    cmd_stack_push(ECHO_OFF);
    cmd_stack_push(WAIT_ONE_SEC);

    
    ScheduleTask(&sim7020_desc, sim7020_processor, RELATIVE, SECONDS(1));
}

void sim7020_start_sim()
{
    cmd_stack_push(CHECK_SIM);
    cmd_stack_push(SET_APN);
    //cmd_stack_push(SET_USER);
    //cmd_stack_push(SET_PWD);
    //cmd_stack_push(SET_CONTYPE);
    cmd_stack_push(GET_CSQ);
}


void sim7020_tcp_update_status()
{
    cmd_stack_push(TCP_GET_STATUS);
}

void sim7020_tcp_set_one_socket_mode()
{
    cmd_stack_push(TCP_SET_ONE_SOCKET_MODE);
}

void sim7020_tcp_set_command_mode()
{
    cmd_stack_push(TCP_SET_COMMAND_MODE);
}

void sim7020_tcp_activate_context()
{
    cmd_stack_push(TCP_GET_STATUS);
    cmd_stack_push(TCP_ACTIVATE_CONTEXT);
}

void sim7020_tcp_deactivate_context()
{
    cmd_stack_push(TCP_DEACTIVATE_CONTEXT);
}

void sim7020_tcp_start_task()
{
    cmd_stack_push(TCP_GET_STATUS);
    cmd_stack_push(TCP_START_TASK);
    //cmd_stack_push(WAIT_TEN_SEC);
}

void sim7020_tcp_open_socket()
{
    cmd_stack_push(TCP_OPEN_SOCKET);
}

void sim7020_tcp_wait_open_socket()
{
    cmd_stack_push(TCP_WAIT_OPEN_SOCKET);
}

void sim7020_tcp_close_socket()
{
    cmd_stack_push(TCP_CLOSE_SOCKET);
}

void sim7020_tcp_get_ip()
{
    cmd_stack_push(TCP_GET_IP);
}

void sim7020_tcp_send()
{
    cmd_stack_push(TCP_SEND);
}

void sim7020_get_csq(void)
{
    cmd_stack_push(GET_CSQ);
}

void sim7020_check_sim(void)
{
    cmd_stack_push(CHECK_SIM);
}

void sim7020_force_stop()
{
    cmd_stack_clear();
    sim7020_phy_power_off();
    sim7020.state = SIM7020_IS_OFF;
}

void sim7020_stop()
{
    cmd_stack_clear();
    cmd_stack_push(POWER_OFF);
}

uint8_t sim7020_can_sleep()
{
    return sim7020.state ? 0 : 1;
}

void sim7020_set_contype(const_string_t contype)
{
    cmd_stack_push(SET_CONTYPE);
}