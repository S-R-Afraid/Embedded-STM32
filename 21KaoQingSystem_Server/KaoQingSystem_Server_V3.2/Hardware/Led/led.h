#ifndef __LED_H__
#define __LED_H__


#include <stm32f1xx_hal.h>
#include "main.h"
//#include "delay.h"

#define R_LED 1
#define B_LED 2

void LED_Trun_On(uint8_t led);
void LED_Trun_Off(uint8_t led);
void LED_Trun_ALL_Off(void);
void LED_Trun_ALL_On(void);
void LED_Toggle(uint8_t led);
void LED_Loop(void);

#endif
