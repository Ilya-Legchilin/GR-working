#ifndef CLOCK_H
#define CLOCK_H

#include "stm32l0xx_hal.h"
#include "wtimer.h"


typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
} clock_time_t;


typedef struct {
    uint8_t day;
    uint8_t month;
    uint8_t year;
} clock_date_t;


typedef struct {
    uint32_t ts;
    void (*callback)(void);
} clock_alarm_t;


void clock_init(RTC_HandleTypeDef * clock_ptr, RTC_TimeTypeDef * time_ptr, RTC_DateTypeDef * date_ptr, RTC_AlarmTypeDef * alarm_ptr);
void clock_start(struct wtimer_desc *desc);

void clock_add_alarm(uint32_t ts, void (*callback)(void));
uint32_t clock_to_unix(int day, int month, int year, int hour, int minute, int second);
void clock_callback(struct wtimer_desc *desc);

#endif // CLOCK_H
