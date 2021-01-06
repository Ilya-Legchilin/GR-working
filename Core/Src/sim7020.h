#ifndef SIM7020_H
#define SIM7020_H


#include "sim7020_phy.h"
#include "misc.h"
#include "uart_simple_protocol.h"
#include "sim7020_tcp.h"

#define SIM7020_TRIES 3


typedef enum{
    SIM7020_OK=0,
    SIM7020_ERROR=0x80,
    SIM7020_BAD_STATUS,
    SIM7020_BAD_ANSWER,
    SIM7020_INCORRECT_ANSWER,
    SIM7020_UNSOLISITED,
    SIM7020_BUFF_OVERFLOW,
    SIM7020_BAD_CMD,
    SIM7020_TIMEOUT,
    SIM7020_WORK_TIME_TIMEOUT,
    SIM7020_NO_POWER,
    SIM7020_NO_PING,
    SIM7020_ECHO_ENABLED,
    SIM7020_NO_SIM,
    SIM7020_NO_NET,
    SIM7020_NO_BEARER,
    SIM7020_TCP_ERROR
} sim7020_ret_t;


typedef enum {
    AT_NONE=0,
    POWER_ON,
    PING,
    ECHO_OFF,
    SET_APN,
    SET_USER,
    SET_PWD,
    CHECK_SIM,
    GET_CSQ,
    WAIT_ONE_SEC,
    SET_CONTYPE,
    TCP_SET_ONE_SOCKET_MODE,
    TCP_SET_COMMAND_MODE,
    TCP_GET_STATUS,
    TCP_ACTIVATE_CONTEXT,
    TCP_OPEN_SOCKET,
    TCP_WAIT_OPEN_SOCKET,
    TCP_SEND,
    TCP_CLOSE_SOCKET,
    TCP_DEACTIVATE_CONTEXT,
    TCP_START_TASK,
    TCP_GET_IP,

    POWER_OFF=0xFF,

    WAIT_TWO_SEC,
    WAIT_FOUR_SEC,
    WAIT_TEN_SEC
} at_cmd_t;


void cmd_stack_clear();


typedef enum {
    SIM7020_IS_OFF=0,
    SIM7020_IS_ON,
    SIM7020_CONNECTED_ECHO,
    SIM7020_CONNECTED,
    SIM7020_SIM_CONNECTED,
    SIM7020_SIM__APN,
    SIM7020_SIM__USER,
    SIM7020_SIM__PWD,
    SIM7020_SIM_READY
} sim7020_state_t;


typedef struct {
    char apn[20];
    char user[20];
    char pwd[20];
} simcard_t;


typedef struct {
    uint8_t         bypass : 1;
    uint8_t         sq : 6;
    uint8_t         tries;
    uint32_t        work_time_ms;
    simcard_t       simcard;
    at_cmd_t        active_cmd;
    at_cmd_t        last_cmd;
    sim7020_ret_t   error;
    sim7020_state_t state;
    void*           app;
} sim7020_t;

extern sim7020_t sim7020;


typedef struct {
  int16_t rssi;
  uint8_t ber;
  uint8_t mode; 
  uint8_t format; 
  char oper[32]; 
  uint8_t AcT; 
  uint8_t cid;
  uint8_t bearer_id;
  char apn[32];
  char local_adress[32];
  char subnet_mask[32];
  char primary_DNS[32];
  char secondary_DNS[32];
  
  /* --- add some properties -------------------------------------------------*/
} nbiot_t;


uint8_t sim7020_can_sleep();


void sim7020_start();
void sim7020_stop();


void sim7020_start_sim();
void sim7020_force_stop();


void sim7020_ping(void);
void sim7020_echo_off(void);
void sim7020_get_csq(void);
void sim7020_check_sim(void);
void sim7020_open_bearer(void);


void sim7020_tcp_update_status();
void sim7020_tcp_set_one_socket_mode();
void sim7020_tcp_set_command_mode();
void sim7020_tcp_start_task();
void sim7020_tcp_activate_context();
void sim7020_tcp_deactivate_context();
void sim7020_tcp_open_socket();
void sim7020_tcp_send();
void sim7020_tcp_close_socket();
void sim7020_tcp_get_ip();
void sim7020_tcp_send();


#endif