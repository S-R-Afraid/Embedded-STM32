#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
//#include "./BSP/BEEP/beep.h"
//#include "./BSP/EXTI/exti.h"
#include "./BSP/KEY/key.h"
#include "./BSP/WDG/wdg.h"

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    usart_init(115200);
    led_init();
    key_init();
    float t=1000;//走马灯间隔，设置在4095以上即可验证自动设置分频数和重载值功能
    LED0(0);
    delay_ms(t);
    LED0(1);
    LED1(0);
    //delay_ms(100);
    //iwdg_init(IWDG_PRESCALER_64, 625);      /* 预分频数为64,重载值为625,溢出时间约为1s */
    //iwdg_init_jcq(IWDG_PRESCALER_64, 625);
    my_iwdg_init(t);
    while (1)
    {
        if (key_scan(1) == WKUP_PRES)       /* 如果WK_UP按下,则喂狗 */
        {
            iwdg_feed();                    /* 喂狗 */
        }

        delay_ms(10);
    }
}


