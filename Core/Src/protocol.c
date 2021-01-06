#include "protocol.h"
#include "json.h"
#include "uart_simple_protocol.h"
#include "md5.h"
#include "sim7020_tcp.h"
#include "sim7020.h"
#include "adc.h"
#include "main.h"

proto_status_t proto_status;
uint8_t calls = 0;

char salt[70];
//char password[] = "qwerty12";
//static parameters_t new_parameters = {0};
//static uint8_t dbg3 = 0;
//static uint8_t display_locked = 0;

//1 - open
//0 - close
//void proto_valve_process(uint8_t st)
//{
//    if (st == 1)
//        clapan_change_state(CL_OPEN);
//    else
//        clapan_change_state(CL_CLOSE_SERVER);
//}
json_t * json_dbg;
void tcp_rx_callback(const_string_t rx_str)
{
    calls++;
    
    UART_SendString("-------from callback-------");
    UART_SendString(rx_str);
    uint32_t tmp_int;
    uint32_with_div_t tmp_int_wd;
    json_value_t* tmp_json_value;

    //uint8_t i;
    //for (i = 0; i < sizeof(parameters_t); ++i)
    //    ((uint8_t*)(&new_parameters))[i] = 0;

    //uint8_t vv = 0xFF;

    json_t * json;

    switch(proto_status.state)
    {
    case (PROTO_AUTH):
        char server_name[10] = {0};
        JSON_GET_STR_FROM_STR(rx_str, "server", server_name);
        
        //if (!strstr(server_name, "gas")) {
        //    proto_status.state = PROTO_SERVER_ERROR;
        //    break;
        //}
        salt[0] = '\0';
        JSON_GET_STR_FROM_STR(rx_str, "salt", salt);
        //JSON_GET_INT_FROM_STR(rx_str, "time", new_parameters.time);

        //parameters_update(&new_parameters);

        json = new_json();

        JSON_ADD_STRING(json,       "id",   "2020.370001");
        if (salt[0]) {
            stradd("aaaabbbbccccddddeeeeffffgggghhhh", salt);
            count_md5(salt, salt); //lifehack
            JSON_ADD_STRING(json,   "auth", salt);
        }
        JSON_ADD_INT(json,          "mf",   1);
        JSON_ADD_INT_WITH_DIV(json, "vl",   0, 2);
        JSON_ADD_INT_WITH_DIV(json, "tm",   0, 1);
        JSON_ADD_INT(json,          "wt",   0);
        JSON_ADD_INT(json,          "st",   0);
        JSON_ADD_INT(json,          "sb",   0); 
        JSON_ADD_INT(json,          "vv",   0);
        JSON_ADD_INT_WITH_DIV(json, "vb",   adc_get_voltage(), 3);
        JSON_ADD_INT(json,          "hw",   1234);
        JSON_ADD_INT(json,          "gt",   1234);
        JSON_ADD_INT(json,          "sq",   1234);
        JSON_ADD_INT(json,          "er",   0); ///TODO
        JSON_ADD_INT(json,          "op",   0);
        json_dbg = json;
        
        //uint8_t* ptr = (uint8_t *)malloc(0x050);

        //if (ptr)
        //  UART_SendString("Success\n");
        
        
        tcp_send_and_free(json);
        proto_status.state = PROTO_WAIT_HANDLER_CMD;
        break;
    case PROTO_WAIT_HANDLER_CMD:
        {
            //JSON_GET_INT_FROM_STR(rx_str, "vv", vv);
            //if (vv != 0xFF)
            //    proto_valve_process(vv);

            //JSON_GET_INT_FROM_STR(rx_str, "pd", new_parameters.period);
            //JSON_GET_INT_FROM_STR(rx_str, "tf", new_parameters.tarif);
            //JSON_GET_INT_FROM_STR(rx_str, "bl", new_parameters.balance);
            //char new_server[32] = {0};
            //JSON_GET_STR_FROM_STR(rx_str, "ip", new_server);
            //JSON_GET_STR_FROM_STR(rx_str, "pw", new_parameters.password);
            //JSON_GET_STR_FROM_STR(rx_str, "sc", new_parameters.spassword);

            //if (new_server[0]) {
            //    uint8_t div_p = strfindc(new_server, ':');
            //    for(i = 0; i < div_p; ++i)
            //        new_parameters.server.ip[i] = new_server[i];
            //    strcp(new_server + div_p + 1, new_parameters.server.port);
            //}

            //parameters_update(&new_parameters);

            proto_status.state = PROTO_CLOSE_CONNECTION;
        }

        {
            json = new_json();
            JSON_ADD_INT(json,          "st",   1);
            //json_array_t* json_empty_array = 0;
            //json_add_object(json, json_new_object("ar", json_new_value(JSON_ARRAY, json_empty_array)));
            json_dbg = json;
            tcp_send_and_free(json);
            
        }
        break;
    default:
        break;
    }
}


void tcp_closed_callback()
{
    //bypass_send_str("TCP CALLBACK: connection closed\n");

    //if (parameters.period)
    //    rtc_add_alarm(parameters.period, proto_send);

    //rtc_add_alarm(10, proto_send);

    //if (!sim800.error)
     //   display_set_state(DISP_TRANSMIT_SUCCESS);

    tcp_stop();
    sim7020_stop();

    proto_status.state = PROTO_OFF;
}


//void tcp_closed_callback()
//{
//    bypass_send_str("TCP CALLBACK: connection closed\n");

//    if (parameters.period)
//        rtc_add_alarm(parameters.period, proto_init);

//    if ((!sim800.error) && (!display_locked))
//        display_force_state(DISP_TRANSMIT_SUCCESS);

//    sim800_stop();
//    proto_status.state = 0;
//}

//char msg[400] = "{\"server\":\"gas\",\"time\":1603372931,\"salt\":\"Jg04EX3UGjjTtNGMlO3FBKHVLEiOPJtr\"}";


void proto_init()
{
    proto_status.state = PROTO_OFF;
}


void proto_send()
{
    if (sim7020.state) return;
    
    proto_ret_t res = PROTO_OK;

    strcp("internet",   sim7020.simcard.apn);
    strcp("gdata",      sim7020.simcard.user);
    strcp("gdata",      sim7020.simcard.pwd);
    
    
    //strcp(parameters.server.ip,     tcp.server.ip);
    //strcp(parameters.server.port,   tcp.server.port);

    sim7020_start();
    sim7020_start_sim();
    tcp_start();
    proto_status.state = PROTO_AUTH;
}




