#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "sim7020_tcp.h"

typedef struct {
    char id[16];
    char password[10];
    char spassword[33];
    //tcp_server_r server;
    uint32_t value;
    uint32_t period;
    uint32_t tarif;
    uint32_t balance;
    uint32_t time;
    uint32_t crc;
} parameters_t;

extern parameters_t parameters;
extern const uint8_t   parameters_mf;
extern const uint8_t   parameters_hw;
extern const char      parameters_vr[];
extern const uint16_t  parameters_tm; //default temperature
extern const uint8_t   parameters_op;

extern uint32_t parameters_work_time;
extern uint32_t parameters_sabotage_time;


void parameters_read();
void parameters_save();

#endif // PARAMETERS_H
