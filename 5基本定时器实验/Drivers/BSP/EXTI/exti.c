#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/BEEP/beep.h"
#include "./BSP/KEY/key.h"
#include "./BSP/EXTI/exti.h"

int exit_bool=0;
void toggle_exit_bool(void){
    exit_bool=1-exit_bool;
}


void KEY0_INT_IRQHandler(void)/*这些外部中断程序的函数名经过宏处理后会转换成对应的中断线处理程序（向量表里的weak函数）
                                如本函数实际上是EXTI4_IRQHandler()，在startup_stm32f103xe.s里211行*/
{
    
    HAL_GPIO_EXTI_IRQHandler(KEY0_INT_GPIO_PIN);         /* 调用中断处理公用函数 清除中断标志位 */
    if(exit_bool){
        delay_ms(20);
        if (KEY0 == 0)
        {
            LED0_TOGGLE();  /* LED0 状态取反 */ 
            LED1_TOGGLE();  /* LED1 状态取反 */ 
        }
    }
    __HAL_GPIO_EXTI_CLEAR_IT(KEY0_INT_GPIO_PIN);         /* 再清一次中断*/

}


void KEY1_INT_IRQHandler(void)
{ 
    HAL_GPIO_EXTI_IRQHandler(KEY1_INT_GPIO_PIN);         
    if(exit_bool){
        delay_ms(20);
        if (KEY1 == 0)
        {
            LED0_TOGGLE();  /* LED0 状态取反 */ 
            //toggle_exit_bool();
        }
    }
    __HAL_GPIO_EXTI_CLEAR_IT(KEY1_INT_GPIO_PIN);         
}


void WKUP_INT_IRQHandler(void)
{ 
    HAL_GPIO_EXTI_IRQHandler(WKUP_INT_GPIO_PIN);        
    delay_ms(20);
    if (WK_UP == 1)
    {
        toggle_exit_bool();
        //BEEP_TOGGLE();  /* 蜂鸣器状态取反 */ 
    }
    __HAL_GPIO_EXTI_CLEAR_IT(WKUP_INT_GPIO_PIN);        
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
    /*程序在遇到中断后调用对应的外部中断服务程序，
    再由外部中断服务程序调用HAL_GPIO_EXTI_IRQHandler()函数，
    最后本函数在HAL_GPIO_EXTI_IRQHandler里被调用，并传递对应的引脚编号。
    HAL_GPIO_EXTI_IRQHandler是外部中断公共处理函数，它只会清除对应中断标志位，
    真正的操作还是在本函数里完成，
    因此本函数还要根据传进的参数才能知道到底触发了哪个中断*/
{
//    delay_ms(20);      /* 消抖 */
//    switch(GPIO_Pin)
//    {
//        case KEY0_INT_GPIO_PIN:
//            if (KEY1 == 0)
//            {
//                LED0_TOGGLE();  /* LED0 状态取反 */ 
//            }
//            break;
//        case KEY1_INT_GPIO_PIN:
//            if (KEY1 == 0)
//            {
//                LED0_TOGGLE();  /* LED0 状态取反 */ 
//            }
//            break;
//        case WKUP_INT_GPIO_PIN:
//            if (WK_UP == 1)
//            {
//                BEEP_TOGGLE();  /* 蜂鸣器状态取反 */ 
//            }
//            break;
//    }
}

void extix_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    KEY0_GPIO_CLK_ENABLE();                                  /* KEY0时钟使能 */
    KEY1_GPIO_CLK_ENABLE();                                  /* KEY1时钟使能 */
    WKUP_GPIO_CLK_ENABLE();                                  /* WKUP时钟使能 */
    /*__HAL_RCC_AFIO_CLK_ENABLE在HAL_Init里系统自动设置*/

    gpio_init_struct.Pin = KEY0_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_FALLING;            /* 下升沿触发 */
    gpio_init_struct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(KEY0_INT_GPIO_PORT, &gpio_init_struct);    /* KEY0配置为下降沿触发中断 */

    gpio_init_struct.Pin = KEY1_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_FALLING;            /* 下升沿触发 */
    gpio_init_struct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(KEY1_INT_GPIO_PORT, &gpio_init_struct);    /* KEY1配置为下降沿触发中断 */
    
    gpio_init_struct.Pin = WKUP_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_RISING;             /* 上升沿触发 */
    gpio_init_struct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(WKUP_GPIO_PORT, &gpio_init_struct);        /* WKUP配置为下降沿触发中断 */

    HAL_NVIC_SetPriority(KEY0_INT_IRQn, 0, 2);               /* 抢占0，子优先级2 */
    HAL_NVIC_EnableIRQ(KEY0_INT_IRQn);                       /* 使能中断线4 */

    HAL_NVIC_SetPriority(KEY1_INT_IRQn, 1, 2);               /* 抢占1，子优先级2 */
    HAL_NVIC_EnableIRQ(KEY1_INT_IRQn);                       /* 使能中断线3 */

    HAL_NVIC_SetPriority(WKUP_INT_IRQn, 2, 2);               /* 抢占2，子优先级2 */
    HAL_NVIC_EnableIRQ(WKUP_INT_IRQn);                       /* 使能中断线0 */
}












