#include "main.h"
#include "schedule.h"
#include "wtimer.h"
#include "stm32l0xx_hal_conf.h"
    
static LPTIM_HandleTypeDef hlptim;

void RADIO_LPTIM_Init(void)
{
  hlptim.Instance = LPTIM1;
  hlptim.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
  hlptim.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  if (HAL_LPTIM_Init(&hlptim) != HAL_OK)
  {
    Error_Handler();
  }
}

void ax5043_enable_global_irq(void)
{
  __enable_irq();
}

void ax5043_disable_global_irq(void)
{
  __disable_irq();
}




void wtimer_cc_irq_enable(uint8_t chan)
{
	__HAL_LPTIM_ENABLE_IT(&hlptim, LPTIM_IT_CMPM);
}

void wtimer_cc_irq_disable(uint8_t chan)
{
	__HAL_LPTIM_DISABLE_IT(&hlptim, LPTIM_IT_CMPM);
}

void wtimer_cc_set(uint8_t chan, uint16_t data)
{
	hlptim.Instance->CMP = data;
}

uint16_t wtimer_cc_get(uint8_t chan)
{
  return (uint16_t) hlptim.Instance->CMP;
}

uint16_t wtimer_cnt_get(uint8_t chan)
{
  static uint16_t prev = 0; 
  uint16_t timer = (uint16_t) hlptim.Instance->CNT;
  if((timer < prev) && ((prev - timer) < 10000))
  {
    return prev;
  }
  prev = timer;
  return timer;
}

uint8_t wtimer_check_cc_irq(uint8_t chan)
{
  return __HAL_LPTIM_GET_FLAG(&hlptim, LPTIM_IT_CMPM);
}

void Schedule_init(void)
{
        RADIO_LPTIM_Init();       
        HAL_LPTIM_Counter_Start(&hlptim, 0xffff);
        
	wtimer_reg_func(WTIMER_GLOBAL_IRQ_ENABLE, (void*)ax5043_enable_global_irq);
	wtimer_reg_func(WTIMER_GLOBAL_IRQ_DISABLE, (void*)ax5043_disable_global_irq);
	wtimer_reg_func(WTIMER_CC_IRQ_ENABLE, (void*)wtimer_cc_irq_enable);
	wtimer_reg_func(WTIMER_CC_IRQ_DISABLE, (void*)wtimer_cc_irq_disable);
	wtimer_reg_func(WTIMER_SET_CC, (void*)wtimer_cc_set);
	wtimer_reg_func(WTIMER_GET_CC, (void*)wtimer_cc_get);
	wtimer_reg_func(WTIMER_GET_CNT, (void*)wtimer_cnt_get);
	wtimer_reg_func(WTIMER_CHECK_CC_IRQ, (void*)wtimer_check_cc_irq);
        
	wtimer_init();
}



