#ifndef __BTIM_H
#define __BTIM_H

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/LCD/lcd.h"

/* 基本定时器 定义 */

/* TIMX 中断定义 
 * 默认是针对TIM6/TIM7
 * 注意: 通过修改这4个宏定义,可以支持TIM1~TIM8任意一个定时器.
 */
 
#define BTIM_TIMX_INT                       TIM6
#define BTIM_TIMX_INT_IRQn                  TIM6_DAC_IRQn           /* 就是TIM6_IRQn，不过注意TIM7没有DAC，只有IRQn，下同。 */
#define BTIM_TIMX_INT_IRQHandler            TIM6_DAC_IRQHandler     /* DAC_IROHander就是IRQHender */
#define BTIM_TIMX_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM6_CLK_ENABLE(); }while(0)   /* TIM6 时钟使能 */

/*---------------------------------------------------------------------------------*/

void btim_tim6_int_init(uint16_t arr, uint16_t psc);    /* 基本定时器 定时中断初始化函数 */
void btim_tim7_int_init(uint16_t arr, uint16_t psc);    /* 基本定时器 定时中断初始化函数 */

#endif

















