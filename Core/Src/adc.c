#include "adc.h"


static ADC_HandleTypeDef * adc;


void adc_init(ADC_HandleTypeDef * adc_ptr)
{
  adc = adc_ptr;
  HAL_ADCEx_Calibration_Start(adc, ADC_SINGLE_ENDED);
}

uint32_t adc_value;
uint32_t res = 4840000;// 1610* 3 * 1000
uint32_t temp = 0;
uint16_t adc_get_voltage(void)
{
  HAL_ADC_Start(adc);
  //HAL_ADCEx_Calibration_Start(adc, ADC_SINGLE_ENDED);
  //HAL_ADC_PollForConversion(adc, 10000); 
  
  adc_value = HAL_ADC_GetValue(adc); 
  HAL_ADC_Stop(adc);
  temp = res / adc_value;
  //adc_value = 0;
  return temp;
}