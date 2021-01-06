#include "rtc.h"
#include "schedule.h"
#include "stm32l0xx_hal.h"
#include "tic33.h"
#include "uart_simple_protocol.h"


void rtc_set_alarm_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day);
void rtc_set_alarm_callback();


static RTC_HandleTypeDef * rtc;
static RTC_TimeTypeDef * sTime;
static RTC_DateTypeDef * sDate;
static RTC_AlarmTypeDef * rtc_sAlarm;
char trans_str[64] = {0,};
struct wtimer_desc rtc_desc;
uint32_t utc = 0;


void empty()
{
  
}


void (*rtc_function)(void);



void rtc_init(RTC_HandleTypeDef * rtc_ptr, RTC_TimeTypeDef * time_ptr, RTC_DateTypeDef * date_ptr, RTC_AlarmTypeDef * alarm_ptr)
{
  rtc_function = empty;
  rtc = rtc_ptr;
  sTime = time_ptr;
  sDate = date_ptr;
  rtc_sAlarm = alarm_ptr;
  lcd_init_();
  ScheduleTask(&rtc_desc, rtc_start, RELATIVE, SECONDS(1));
}


void rtc_start(struct wtimer_desc *desc)
{
   LCD_SCLK_SWITCH();
   HAL_RTC_GetTime(rtc, sTime, RTC_FORMAT_BIN); // RTC_FORMAT_BIN , RTC_FORMAT_BCD
   snprintf(trans_str, 63, "%.2d.%.2d.%.2d\n ", sTime->Hours, sTime->Minutes, sTime->Seconds);
   UART_SendString(trans_str);
   LCD_WriteStr(trans_str);
   
   HAL_RTC_GetDate(rtc, sDate, RTC_FORMAT_BIN);
   snprintf(trans_str, 63, "Date %d-%d-20%d\n", sDate->Date, sDate->Month, sDate->Year);
   UART_SendString(trans_str);
  
   
   ScheduleTask(desc, 0, RELATIVE, SECONDS(1));
}


#define SECONDS_IN_DAY (86400)


rtc_time_t rtc_get_time(uint32_t input_ts)
{
    rtc_time_t _time;
    uint32_t ts = input_ts % SECONDS_IN_DAY;
    _time.hours = ts / 3600;
    _time.minutes = (ts % 3600) / 60;
    _time.seconds = ts % 60;
    return _time;
}

rtc_time_t alarm_time;
uint8_t current_seconds;
uint8_t current_minutes;
uint8_t current_hours;
uint8_t current_day;
uint8_t day;

void rtc_add_alarm(uint32_t ts, void (*callback)(void))
{   
    alarm_time = rtc_get_time(ts);
    
    HAL_RTC_GetTime(rtc, sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(rtc, sDate, RTC_FORMAT_BIN);
    
    current_seconds = sTime->Seconds;
    current_minutes = sTime->Minutes;
    current_hours   =   sTime->Hours;
    current_day     =    sDate->Date;
    
    day = alarm_time.hours / 24;
    alarm_time.hours = alarm_time.hours % 24;
    
    rtc_set_alarm_time(current_seconds + alarm_time.seconds, current_minutes + alarm_time.minutes, current_hours + alarm_time.hours, current_day + day);
    rtc_set_alarm_callback(callback);
}


uint32_t UNIXTime(int Day, int Month, int Year, int Hour, int Minute, int Second)
{
  if (Month <= 2){
    Year--;
    Month += 12;
  }
  Day = ((36525 * Year) / 100) + ((306001 * (Month + 1)) / 10000) + Day - 719606;
  Second += (Minute * 60) + (Hour * 3600);
  return(((unsigned int)Day * (24 * 60 * 60)) + (unsigned int)Second);
}


void rtc_set_alarm_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day)
{
  rtc_sAlarm->AlarmTime.Hours = hours;
  rtc_sAlarm->AlarmTime.Minutes = minutes;
  rtc_sAlarm->AlarmTime.Seconds = seconds;
  rtc_sAlarm->AlarmDateWeekDay = day;
  HAL_RTC_SetAlarm_IT(rtc, rtc_sAlarm, RTC_FORMAT_BCD);
}

void rtc_set_alarm_callback(void (*callback)(void))
{
  rtc_function = callback;
}

void rtc_callback()
{
  rtc_function();
}