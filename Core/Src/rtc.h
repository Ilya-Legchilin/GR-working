#ifndef RTC_H
#define RTC_H

#include "stm32l0xx_hal.h"
#include "wtimer.h"


typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
} rtc_time_t;


typedef struct {
    uint8_t day;
    uint8_t month;
    uint8_t year;
} rtc_date_t;


typedef struct {
    uint32_t ts;
    void (*callback)(void);
} rtc_alarm_t;


void rtc_init(RTC_HandleTypeDef * rtc_ptr, RTC_TimeTypeDef * time_ptr, RTC_DateTypeDef * date_ptr, RTC_AlarmTypeDef * alarm_ptr);
void rtc_start(struct wtimer_desc *desc);

//void rtc_correct(uint32_t time);

//uint32_t    rtc_get_utc();
//rtc_time_t  rtc_get_time();
//rtc_date_t  rtc_get_date();

void rtc_add_alarm(uint32_t ts, void (*callback)(void));
uint32_t rtc_to_unix(int day, int month, int year, int hour, int minute, int second);
void rtc_callback();

#endif // RTC_H
