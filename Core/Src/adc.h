#ifndef ADC_H
#define ADC_H
#include "stm32l0xx_hal.h"

void adc_init(ADC_HandleTypeDef * adc);
uint16_t adc_get_voltage(void);


#endif //ADC_H