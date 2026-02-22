#ifndef __ENCODER_H
#define __ENCODER_H
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"

#define ENCODER_TIM                         TIM3
#define ENCODER_TIM_CLK_ENABLE()            __HAL_RCC_TIM3_CLK_ENABLE()
#define ENCODER_TIM_PERIOD                  65535           /* 定时器溢出值 */

#define ENCODER_TIM_PRESCALER               0      /* 定时器预分频值 */
/* 定时器3中断TIM3_IRQn、TIM3_IRQHandler在gtim里,不过没用到 */






void Encoder_Init(void);
int16_t Encoder_Get(void);

#endif
