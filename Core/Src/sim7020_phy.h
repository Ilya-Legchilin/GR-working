#ifndef SIM7020_PHY_H
#define SIM7020_PHY_H


#include "stdint.h"
#include "stm32l0xx_hal.h"
#include "schedule.h"
#include "wtimer.h"



#define const_string_t const char *

#define SIM7020_PHY_RX_BUFF_LEN 1024
#define NORM_SIM7020_PHY(x) (x & 0x3FF) //fast version of (x % RX_BUFF_LEN)

void sim7020_phy_init(UART_HandleTypeDef *new_huart);
void sim7020_phy_deinit();
void sim7020_phy_power_on();
void sim7020_phy_power_off();
void sim7020_phy_send(const_string_t str);
void sim7020_phy_rxcallback(char* str);
void sim7020_phy_set_rx_callback(void (*cb) (const_string_t));


void sim7020_set_on_cap(struct wtimer_desc * desc);
void sim7020_set_on_sim(struct wtimer_desc * desc);
void sim7020_set_pwrkey(struct wtimer_desc * desc);
void sim7020_set_modem_reset(struct wtimer_desc * desc);
void sim7020_reset_on_cap(struct wtimer_desc * desc);
void sim7020_reset_on_sim(struct wtimer_desc * desc);
void sim7020_reset_pwrkey(struct wtimer_desc * desc);
void sim7020_reset_modem_reset(struct wtimer_desc * desc);
void sim7020_phy_processor(struct wtimer_desc * desc);
void Sim7020_Phy_Rx_Callback(uint8_t i); 

#endif //SIM7020_PHY_H