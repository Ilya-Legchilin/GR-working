#ifndef TCP_H
#define TCP_H


#include "sim7020.h"
#include "misc.h"
#include "uart_simple_protocol.h"
#include "json.h"


typedef enum {
    TCP_OK,
    TCP_ERROR,
    TCP_CONNECTING_TIMEOUT,
    TCP_SETUP_ERROR,
    TCP_SEND_ERROR
} tcp_ret_t;


typedef enum {
    TCP_IP_INITIAL,
    TCP_IP_START,
    TCP_IP_STATUS,
    TCP_IP_CONFIG,
    TCP_IP_GPRSACT,
    TCP_PDP_DEACT,
    TCP_CONNECT_ON,
    TCP_CONNECTING,
    TCP_CONNECT_CLOSING,
    TCP_CONNECT_CLOSED,
} tcp_connection_state_t;


typedef enum {
    TCP_IS_OFF = 0,
    TCP_STARTED,
    TCP_ONE_SOCKET_MODE,
    TCP_COMMAND_MODE,
    TCP_TASK_STARTED,
    TCP_CONTEXT_ACTIVATED,
    TCP_IP_GOTTEN,
    TCP_SOCKET_ACTIVATED,
    TCP_CONNECTED,
    TCP_SENDING
} tcp_state_t;


typedef struct {
    char ip[20];
    char port[10];
} tcp_server_r;


typedef struct {
    tcp_state_t             state;
    tcp_connection_state_t  cstate;
    tcp_ret_t               error;
    tcp_server_r            server;
    uint16_t timeout;
} tcp_t;

extern tcp_t tcp;

tcp_ret_t tcp_start();
tcp_ret_t tcp_stop();

tcp_ret_t tcp_send_and_free(json_t *j);

void tcp_prepair_msg_to_send(char* buff);

void tcp_rx_callback(const_string_t rx_str);
void tcp_socket_closed_callback();

#endif //TCP_H