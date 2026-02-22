#ifndef __MOTOR_H
#define __MOTOR_H
#include "./SYSTEM/sys/sys.h"
#include "./BSP/TIMER/gtim.h"
void Motor_Init(void);
void Motor_SetPWM(int16_t Speed);
void PWM_Init();
void PWM_SetCompare1(uint16_t Compare);
#endif
