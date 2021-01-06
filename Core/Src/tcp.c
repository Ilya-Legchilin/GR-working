#include "tcp.h"
#include "uart_simple_protocol.h"


void tcp_create_socket(void);
void tcp_connect(void);
void tcp_close(void);


void tcp_start(void)
{
  tcp_create_socket();
  tcp_connect();
}


void tcp_stop(void)
{
  tcp_close();
}


void tcp_send_and_free(const char * str)
{
  sim7020_tcp_send();
}


void tcp_rx_callback(char * str)
{
  UART_SendString("> tcp answer:\n> ");
  UART_SendString(str);
}


void tcp_create_socket(void)
{
  sim7020_tcp_create_socket();
}


void tcp_connect(void)
{
  sim7020_tcp_connect();
}


void tcp_close(void)
{
  sim7020_tcp_close();
}


static json_t* tcp_json;


void tcp_send_and_free(json_t *j)
{
  tcp_json = j;
  sim7020_tcp_send();
}


void tcp_prepair_msg_to_send(char* buff)
{
  json_print_to_str(tcp_json, buff);
  json_free(tcp_json);
}