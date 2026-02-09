//#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/BEEP/beep.h"
//#include "./BSP/EXTI/exti.h"
//#include "./BSP/KEY/key.h"    //如果运行IWDG该行取消注释
//#include "./BSP/WDG/wdg.h"
#include "./BSP/TIMER/btim.h"
#include "./BSP/TIMER/gtim.h"
//#include "./BSP/OLED/OLED.h"
//#include "./BSP/DHT11/DHT11.h"
//#include "string.h"


int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    usart_init(115200);
    led_init();
    beep_init();
    //key_init();
    
    while(1){
        
    }
}





