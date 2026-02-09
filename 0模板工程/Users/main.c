/**
 ******************************************************************************
 * @file     main.c
 * @author   正点原子团队(ALIENTEK)
 * @version  V1.0
 * @date     2020-08-20
 * @brief    新建工程实验-HAL库版本 实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * @attention
 * 
 * 实验平台:正点原子 STM32F103 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 ******************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"

void led_init(void);                       /* LED初始化函数声明 */

int main(void)
{
    HAL_Init();                              /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9);      /* 设置时钟, 72Mhz */
    delay_init(72);                          /* 延时初始化 */
    led_init();                              /* LED初始化 */
    
    while(1)
    { 
        HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5);    /* PB5置1 */ 
//        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_RESET);  /* PE5置0 */ 
        delay_ms(50);
//        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);  /* PB5置0 */
//        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_SET);    /* PE5置1 */
//        delay_ms(500); 
    }
}

/**
 * @brief       初始化LED相关IO口, 并使能时钟
 * @param       无
 * @retval      无
 */
void led_init(void)
{
    GPIO_InitTypeDef gpio_initstruct;
    __HAL_RCC_GPIOB_CLK_ENABLE();                          /* IO口PB时钟使能 */
//    __HAL_RCC_GPIOE_CLK_ENABLE();                          /* IO口PE时钟使能 */

    gpio_initstruct.Pin = GPIO_PIN_5;                      /* LED0引脚 */
    gpio_initstruct.Mode = GPIO_MODE_OUTPUT_PP;            /* 推挽输出 */
    gpio_initstruct.Pull = GPIO_PULLUP;                    /* 上拉 */
    gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;          /* 高速 */
    HAL_GPIO_Init(GPIOB, &gpio_initstruct);                /* 初始化LED0引脚 */

//    gpio_initstruct.Pin = GPIO_PIN_5;                      /* LED1引脚 */
//    HAL_GPIO_Init(GPIOE, &gpio_initstruct);                /* 初始化LED1引脚 */
}
