#include "clock.h"
#include "schedule.h"
#include "stm32l0xx_hal.h"
#include "tic33.h"
#include "uart_simple_protocol.h"
#include "stm32l0xx_hal_rtc.h"
#include <time.h>

void clock_set_alarm_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day);
void clock_set_alarm_callback();


static RTC_HandleTypeDef * __clock;
static RTC_TimeTypeDef * clock_time;
static RTC_DateTypeDef * clock_date;
static RTC_AlarmTypeDef * clock_alarm;
char trans_str[64] = {0,};
struct wtimer_desc rtc_desc;
uint32_t utc = 0;


clock_time_t alarm_time;
uint8_t current_seconds;
uint8_t current_minutes;
uint8_t current_hours;
uint8_t current_day;
uint8_t day;


void empty(){}
void (*clock_function)(void);


long get_timestep(){
  time_t timestamp;
  struct tm currTime={0,0,0,0,0,0,0,0,0};
  RTC_TimeTypeDef currentTime;
  RTC_DateTypeDef currentDate;
  HAL_RTC_GetTime(__clock, &currentTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(__clock, &currentDate, RTC_FORMAT_BIN);
  currTime.tm_year = (currentDate.Year) + 100;
  currTime.tm_mday = (currentDate.Date);
  currTime.tm_mon  = (currentDate.Month) - 1;

  currTime.tm_hour = (currentTime.Hours);
  currTime.tm_min  = (currentTime.Minutes);
  currTime.tm_sec  = (currentTime.Seconds);

  currTime.tm_isdst = 0;
  timestamp = mktime(&currTime);

  return timestamp;
}


void clock_init(RTC_HandleTypeDef * clock_ptr, RTC_TimeTypeDef * time_ptr, RTC_DateTypeDef * date_ptr, RTC_AlarmTypeDef * alarm_ptr)
{
  clock_function = empty;
  __clock = clock_ptr;
  clock_time = time_ptr;
  clock_date = date_ptr;
  clock_alarm = alarm_ptr;
}


void clock_start(struct wtimer_desc *desc)
{
  
}


#define SECONDS_IN_DAY (86400)


clock_time_t clock_get_time(uint32_t input_ts)
{
    clock_time_t _time;
    uint32_t ts = input_ts % SECONDS_IN_DAY;
    _time.hours = ts / 3600;
    _time.minutes = (ts % 3600) / 60;
    _time.seconds = ts % 60;
    return _time;
}

uint8_t seconds_proper;
uint8_t minutes_proper;
uint8_t hours_proper;
uint8_t day_proper;
uint8_t month_proper;



void clock_add_alarm(uint32_t ts, void (*callback)(void))
{   
    time_t now = ts + get_timestep();
    
    RTC_TimeTypeDef aTime;
    RTC_DateTypeDef aDate;
 
    struct tm time_tm;
    time_tm = *(localtime(&now));
 
    aTime.Hours = (uint8_t)time_tm.tm_hour;
    aTime.Minutes = (uint8_t)time_tm.tm_min;
    aTime.Seconds = (uint8_t)time_tm.tm_sec;
    
     if (time_tm.tm_wday == 0) { time_tm.tm_wday = 7; } // the chip goes mon tue wed thu fri sat sun
    aDate.WeekDay = (uint8_t)time_tm.tm_wday;
    aDate.Month = (uint8_t)time_tm.tm_mon+1; //momth 1- This is why date math is frustrating.
    aDate.Date = (uint8_t)time_tm.tm_mday;
    aDate.Year = (uint16_t)(time_tm.tm_year+1900-2000); // time.h is years since 1900, chip is years since 2000
    
    clock_alarm->AlarmTime.Hours = aTime.Hours;
    clock_alarm->AlarmTime.Minutes = aTime.Minutes;
    clock_alarm->AlarmTime.Seconds = aTime.Seconds;
    clock_alarm->AlarmDateWeekDay = aDate.Date;
    clock_alarm->AlarmTime.SubSeconds = 0;
    clock_alarm->AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    clock_alarm->AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    clock_alarm->AlarmMask = RTC_ALARMMASK_NONE;
    clock_alarm->AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
    clock_alarm->AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    clock_alarm->AlarmDateWeekDay = aDate.Date;
    clock_alarm->Alarm = RTC_ALARM_A;
    HAL_RTC_SetAlarm_IT(__clock, clock_alarm, RTC_FORMAT_BIN);
    
    
    clock_set_alarm_callback(callback);
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

/*
void clock_set_alarm_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day)
{
  HAL_RTC_GetTime(__clock, clock_time, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(clock, clock_date, RTC_FORMAT_BIN);
  
  HAL_RTC_DeactivateAlarm(clock, RTC_ALARM_A);
}*/

void clock_set_alarm_callback(void (*callback)(void))
{
  clock_function = callback;
}

void clock_callback(struct wtimer_desc *desc)
{
  clock_function();
}
