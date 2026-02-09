/**
 ****************************************************************************************************
 * @file        gtim.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2020-04-20
 * @brief       通用定时器 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211216
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __GTIM_H
#define __GTIM_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* 通用定时器 定义 */

/* TIMX 中断定义 
 * 默认是针对TIM2~TIM5.
 * 注意: 通过修改这4个宏定义,可以支持TIM1~TIM17任意一个定时器.
 */
 
#define GTIM_TIMX_INT                       TIM3
#define GTIM_TIMX_INT_IRQn                  TIM3_IRQn
#define GTIM_TIMX_INT_IRQHandler            TIM3_IRQHandler
#define GTIM_TIMX_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM3_CLK_ENABLE(); }while(0)  /* TIM3 时钟使能 */

/* TIMX PWM输出定义 
 * 这里输出的PWM控制LED0(RED)的亮度
 * 默认是针对TIM2~TIM5
 * 注意: 通过修改这几个宏定义,可以支持TIM1~TIM8任意一个定时器,任意一个IO口输出PWM
 */
#define GTIM_TIMX_PWM_CHY_GPIO_PORT         GPIOA
#define GTIM_TIMX_PWM_CHY_GPIO_PIN          GPIO_PIN_6
#define GTIM_TIMX_PWM_CHY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* PB口时钟使能 */

/* TIMX REMAP设置
 * 因为我们LED0接在PB5上, 必须通过开启TIM3的部分重映射功能, 才能将TIM3_CH2输出到PB5上
 * 因此, 必须实现GTIM_TIMX_PWM_CHY_GPIO_REMAP
 * 对那些使用默认设置的定时器PWM输出脚, 不用设置重映射, 是不需要该函数的!
 * 具体用哪个函数、用不用重映射，查看stm32f1xx_hal_gpio_ex.h，搜__HAL_AFIO_REMAP_TIM3_PARTIAL，看相关系列函数
 */
#define GTIM_TIMX_PWM_CHY_GPIO_REMAP()      do{__HAL_RCC_AFIO_CLK_ENABLE();\
                                                __HAL_AFIO_REMAP_TIM3_PARTIAL();\
                                            }while(0)            /* 通道REMAP设置, 该函数不是必须的, 根据需要实现 */

#define GTIM_TIMX_PWM                       TIM3 
#define GTIM_TIMX_PWM_CHY                   TIM_CHANNEL_1                               /* 通道Y,  1<= Y <=4 */
#define GTIM_TIMX_PWM_CHY_CCRX              TIM3->CCR2                                  /* 通道Y的输出比较寄存器 */
#define GTIM_TIMX_PWM_CHY_CLK_ENABLE()      do{ __HAL_RCC_TIM3_CLK_ENABLE(); }while(0)  /* TIM3 时钟使能 */

/*********************************以下是通用定时器输入捕获实验相关宏定义*************************************/

 /* TIMX 输入捕获定义 
 * 这里的输入捕获使用定时器TIM5_CH1,捕获WK_UP按键的输入
 * 默认是针对TIM2~TIM5. 
 * 注意: 通过修改这几个宏定义,可以支持TIM1~TIM8任意一个定时器,任意一个IO口做输入捕获
 *       特别要注意:默认用的PA0,设置的是下拉输入!如果改其他IO,对应的上下拉方式也得改!
 */
#define GTIM_TIMX_CAP_CHY_GPIO_PORT            GPIOA
#define GTIM_TIMX_CAP_CHY_GPIO_PIN             GPIO_PIN_0
#define GTIM_TIMX_CAP_CHY_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* PA口时钟使能 */

#define GTIM_TIMX_CAP                          TIM5                       
#define GTIM_TIMX_CAP_IRQn                     TIM5_IRQn
#define GTIM_TIMX_CAP_IRQHandler               TIM5_IRQHandler
#define GTIM_TIMX_CAP_CHY                      TIM_CHANNEL_1                                 /* 通道Y,  1<= Y <=4 */
#define GTIM_TIMX_CAP_CHY_CCRX                 TIM5->CCR1                                    /* 通道Y的输出比较寄存器 */
#define GTIM_TIMX_CAP_CHY_CLK_ENABLE()         do{ __HAL_RCC_TIM5_CLK_ENABLE(); }while(0)    /* TIM5 时钟使能 */

/******************************************************************************************/
extern uint8_t  g_timxchy_cap_sta;  /* 输入捕获状态 */
extern uint16_t g_timxchy_cap_val;  /* 输入捕获值 */
extern TIM_HandleTypeDef g_timx_pwm_chy_handle;        /* 定时器x句柄 */


void gtim_tim3_int_init(uint16_t arr, uint16_t psc);        /* 通用定时器 定时中断初始化函数 */
void gtim_tim2_int_init(uint16_t arr, uint16_t psc);
void gtim_timx_pwm_chy_init(uint16_t arr, uint16_t psc);    /* 通用定时器 PWM初始化函数 */
void gtim_timx_cap_chy_init(uint16_t arr, uint16_t psc);    /* 通用定时器 输入捕获初始化函数 */

#endif

















