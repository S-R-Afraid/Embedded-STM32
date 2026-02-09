#ifndef __MYRTC_H
#define __MYRTC_H

#include "stm32f1xx_hal.h"
#include "rtc.h"
#include "time.h"
#define RTC_KEY 0xA5A5
/* 时间结构体, 包括年月日周时分秒等信息 */
typedef struct
{
    uint8_t hour;       /* 时 */
    uint8_t min;        /* 分 */
    uint8_t sec;        /* 秒 */
    /* 公历年月日周 */
    uint16_t year;      /* 年 */
    uint8_t  month;     /* 月 */
    uint8_t  date;      /* 日 */
    uint8_t  week;      /* 周 */
} _calendar_obj;
extern _calendar_obj calendar;                      /* 时间结构体 */

void MyRTC_GetTime(void);

void MyRTC_Init(void);


HAL_StatusTypeDef MyRTC_SetTime(struct tm* time);
uint8_t rtc_set_time(uint16_t syear, uint8_t smon, uint8_t sday, uint8_t hour, uint8_t min, uint8_t sec);


#endif
