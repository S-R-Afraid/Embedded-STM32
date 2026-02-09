#include "./BSP/TB6612FNG/TB6612FNG.h"


/*
初始化函数，初始化PWM以及AN/BN
*/
void TB6612_init(){
    
    gtim_timx_pwm_chy_init(4095-1, 72 - 1);/* 1000Khz的计数频率,重装载值为4095 */
    TB6612_AN1_GPIO_CLK_ENABLE();
    TB6612_AN2_GPIO_CLK_ENABLE();
    TB6612_BN1_GPIO_CLK_ENABLE();
    TB6612_BN2_GPIO_CLK_ENABLE();
    TB6612_STBY_GPIO_CLK_ENABLE();
    
    GPIO_InitTypeDef gpio_init_struct;
    
    gpio_init_struct.Pin = TB6612_AN1_GPIO_PIN;                   /* AN1引脚 */
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;            /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;          /* 高速 */
    HAL_GPIO_Init(TB6612_AN1_GPIO_PORT, &gpio_init_struct);       /* 初始化引脚 */
    
    gpio_init_struct.Pin = TB6612_AN2_GPIO_PIN;                   /* AN2引脚 */
    HAL_GPIO_Init(TB6612_AN2_GPIO_PORT, &gpio_init_struct);
    
    gpio_init_struct.Pin = TB6612_STBY_GPIO_PIN;
    HAL_GPIO_Init(TB6612_STBY_GPIO_PORT, &gpio_init_struct);
    
    TB6612_ASTOP();
    //TB6612_BSTOP();
}

/* 设置比较值，即设置转速 */
void TB6612_SET_A_VALUE(uint16_t val){
    __HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, val);
}





