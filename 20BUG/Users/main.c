//#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/BEEP/beep.h"
//#include "./BSP/EXTI/exti.h"
#include "./BSP/KEY/key.h"
//#include "./BSP/WDG/wdg.h"
#include "./BSP/TIMER/btim.h"
//#include "./BSP/TIMER/gtim.h"
#include "./BSP/OLED/OLED.h"
//#include "./BSP/DHT11/DHT11.h"
#include "string.h"
//#include "./BSP/RTC/rtc.h"
//#include "./BSP/DMA/dma.h"
#include "./BSP/LCD/lcd.h"
//#include "./BSP/ADC/adc.h"
//#include "./BSP/MPU6050/MPU6050.h"
//#include "./BSP/terazzo/terazzo.h"
//#include "./BSP/HC-SR04/HC-SR04.h"
#include "./BSP/ESP8266/esp8266.h"
#include "./BSP/OLED/OLEDUI.h"
#include "./BSP/DRAWBUG/DRAWBUG.h"


//char* weekdays[]={"星期天","星期一","星期二","星期三","星期四","星期五","星期六"};


int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    
    usart_init(1,115200);
    
    led_init();
    
    beep_init();
    
    key_init();
    
    //extix_init();
    //toggle_exit_bool();
    
    //Init_HC_SR04();
    
    OLED_Init();
    OLED_UI_Init();
    
    //rtc_init();                             /* 初始化RTC */
    
    lcd_init();                             /* 初始化LCD */
    
    //adc_init();                             /* 初始化ADC */
    
    g_back_color = BLACK ;
    g_point_color=WHITE;
    lcd_clear(BLACK);
    
    
    while (1)
    {
        //LED1_TOGGLE();
        drawbug();
        //delay_ms(20);
    }
    
}






