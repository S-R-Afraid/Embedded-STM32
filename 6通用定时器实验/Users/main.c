//#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
//#include "./BSP/BEEP/beep.h"
//#include "./BSP/EXTI/exti.h"
#include "./BSP/KEY/key.h"    //如果运行IWDG该行取消注释
//#include "./BSP/WDG/wdg.h"
#include "./BSP/TIMER/btim.h"
#include "./BSP/TIMER/gtim.h"





int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    usart_init(115200);
    led_init();
    key_init();
//    gtim_tim2_int_init(5000 - 1, 7200 - 1); /* 10Khz的计数频率，计数5K次为500ms，绿灯闪烁 */
    /*------------------------PWM实验------------------------*/
    gtim_timx_pwm_chy_init(500 - 1, 72 - 1);/* 1Mhz的计数频率,2Khz的PWM. */
    uint32_t ledrpwmval = 0;
    uint32_t dir_pul = 1 ,dir_arr = 1;
    uint32_t pwm_arr=500-1;

    while (1)
    {
        delay_ms(10);

        ledrpwmval+=dir_pul;                    /* dir==1 ledrpwmval递增 */
                                                /* dir==-1 ledrpwmval递减 */
        pwm_arr+=dir_arr;

        if (ledrpwmval >= 500-1)dir_pul = -1;    /* ledrpwmval到达500后，方向为递减 */
        else if (ledrpwmval <= 0)dir_pul = 1;        /* ledrpwmval递减到0后，方向改为递增 */
        if(pwm_arr<100-1)dir_arr=1;
        if(pwm_arr>900-1)dir_arr=-1;
        //ledrpwmval=pwm_arr/2;
        /*修改比较值*/
        __HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, ledrpwmval);
        /*修改重装载值*/
        //__HAL_TIM_SetAutoreload(&g_timx_pwm_chy_handle,pwm_arr);
    }
    
    /*------------------------输入捕获实验------------------------*/
//    g_timxchy_cap_sta   输入捕获状态 
//    g_timxchy_cap_val   输入捕获值 

//    uint32_t temp = 0;
//    uint8_t t = 0;
//    gtim_timx_cap_chy_init(0XFFFF, 72 - 1); /* 以1Mhz的频率计数 捕获 */

//    while (1)
//    {
//        if (g_timxchy_cap_sta & 0X80)       /* 成功捕获到了一次高电平 */
//        {
//            temp = g_timxchy_cap_sta & 0X3F;
//            temp *= 65536;                  /* 溢出时间总和 */
//            temp += g_timxchy_cap_val;      /* 得到总的高电平时间 */
//            printf("HIGH:%d us\r\n", temp); /* 打印总的高点平时间 */
//            g_timxchy_cap_sta = 0;          /* 开启下一次捕获*/
//        }

//        t++;

//        if (t > 20)         /* 200ms进入一次 */
//        {
//            t = 0;
//            LED0_TOGGLE();  /* LED0闪烁 ,提示程序运行 */
//        }
//        delay_ms(10);
//    }
    
}





