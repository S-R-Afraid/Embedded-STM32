#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
//#include "./BSP/BEEP/beep.h"
#include "./BSP/EXTI/exti.h"
#include "./BSP/KEY/key.h"    //如果运行IWDG该行取消注释
#include "./BSP/WDG/wdg.h"

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    usart_init(115200);
    led_init();
    if(__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET){//通过读取状态寄存器，判断上次复位是什么原因
        printf("WWDG_RESET\r\n");
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }else if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET){
        printf("IWDG_RESET\r\n");
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }else if(__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET){
        printf("PIN_RESET\r\n");                //复位按键复位
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }else if(__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST) != RESET){
        printf("LOW_POWER_RESET\r\n");          //低功耗管理复位
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }
    /*
    //- - - - - - - - - -独立看门狗代码 - - - - - - - - - - 
    key_init();
    float t=1000;//走马灯间隔，设置在4095以上即可验证自动设置分频数和重载值功能
    LED0(0);
    delay_ms(t);
    LED0(1);
    LED1(0);
    //delay_ms(100);
    //iwdg_init(IWDG_PRESCALER_64, 625);      // 预分频数为64,重载值为625,溢出时间约为1s
    //iwdg_init_jcq(IWDG_PRESCALER_64, 625);
    my_iwdg_init(t);
    //my_iwdg_init(t*2);                        //经过试验，在已经启动看门狗的情况下重复启动会覆盖第一次的设置

    while (1)
    {
        if (key_scan(1) == WKUP_PRES)        //如果WK_UP按下,则喂狗 
        {
            iwdg_feed();                     //喂狗 
        }

        delay_ms(10);
    }
    */
    
    //- - - - - - - - - -窗口看门狗代码 - - - - - - - - - - 
    LED0(0);
    delay_ms(300);
    wwdg_init(0X7F, 0X5F, WWDG_PRESCALER_8);// 计数器值为7f,窗口寄存器为5f,分频数为8

    while (1)
    {
        LED0(1);                            // 关闭红灯
    }
    
}



