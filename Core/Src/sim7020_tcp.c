#include "sim7020_tcp.h"
#include "uart_simple_protocol.h"
#include "json.h"


tcp_t tcp;

void tcp_set_apn();
void tcp_bring_up();
void tcp_get_local_ip();
void tcp_connect();
void tcp_close();


tcp_ret_t tcp_start(void)
{
  if (!sim7020.state)
    return TCP_ERROR;

  tcp.state = TCP_STARTED;
  tcp.cstate = TCP_IP_INITIAL;
  sim7020.app = &tcp;

  sim7020_tcp_update_status();
  sim7020_tcp_set_one_socket_mode();
  sim7020_tcp_set_command_mode();
  sim7020_tcp_start_task();
  sim7020_tcp_activate_context();
  sim7020_tcp_get_ip();
  sim7020_get_csq();
  sim7020_tcp_open_socket();
  sim7020_tcp_wait_open_socket();
  return TCP_OK;
}

tcp_ret_t tcp_stop()
{
    tcp.state = TCP_IS_OFF;
    tcp.cstate = TCP_IP_INITIAL;
    sim7020_tcp_deactivate_context();
    sim7020.app = 0;
    return TCP_OK;
}


static json_t* tcp_json;


tcp_ret_t tcp_send_and_free(json_t *j)
{
    tcp_json = j;
    sim7020_tcp_send();
}


void tcp_prepair_msg_to_send(char* buff)
{
    json_print_to_str(tcp_json, buff);
    json_free(tcp_json);
}