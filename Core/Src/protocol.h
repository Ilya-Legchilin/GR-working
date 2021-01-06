#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "misc.h"

#define const_string_t const char *

typedef enum {
    PROTO_OK,
    PROTO_ERROR
} proto_ret_t;


typedef struct {
    uint32_t period;
    uint32_t dt;
} proto_settings_t;


typedef enum {
    PROTO_OFF,
    PROTO_WAIT_FOR_SEND,
    PROTO_START_SENDING,
    PROTO_AUTH,
    PROTO_WAIT_HANDLER_CMD,
    PROTO_CLOSE_CONNECTION,

    PROTO_SERVER_ERROR,
    PROTO_TCP_ERROR
} proto_state_t;


typedef struct {

    char valve[8]; // "close" or "open"
    uint32_t value;
    uint32_t ts;
    uint32_t detectors[4];
    proto_state_t state;
} proto_status_t;


void proto_init();
proto_ret_t proto_set_ts(uint32_t ts);
proto_ret_t proto_set_dt(uint32_t dt);
proto_ret_t proto_set_period(uint32_t period);

uint32_t* proto_get_detectors_p();

void tcp_rx_callback(const_string_t rx_str);
void tcp_closed_callback(void);

void proto_init();
void proto_send();


#endif //PROTOCOL_H
