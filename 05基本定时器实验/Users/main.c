#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
//#include "./BSP/BEEP/beep.h"
//#include "./BSP/EXTI/exti.h"
#include "./BSP/KEY/key.h"    //如果运行IWDG、exit该行取消注释
//#include "./BSP/WDG/wdg.h"
#include "./BSP/TIMER/btim.h"
#include "./SYSTEM/usart/usart.h"

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    usart_init(115200);
    led_init();
    //LED0_TOGGLE();
    btim_tim6_int_init(10000 - 1, 7200 - 1); /* 10Khz的计数频率，计数5K次为500ms */
    btim_tim7_int_init(3000 - 1, 7200 - 1);
    delay_ms(200);
    lcd_init();
    lcd_clear(BLACK);
    while (1)
    {
//        LED0_TOGGLE();
//        delay_ms(200);
    }
    
}



