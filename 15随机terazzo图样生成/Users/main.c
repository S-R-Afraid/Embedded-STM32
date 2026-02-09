//#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/BEEP/beep.h"
#include "./BSP/EXTI/exti.h"
#include "./BSP/KEY/key.h"
//#include "./BSP/WDG/wdg.h"
//#include "./BSP/TIMER/btim.h"
//#include "./BSP/TIMER/gtim.h"
//#include "./BSP/OLED/OLED.h"
//#include "./BSP/DHT11/DHT11.h"
#include "string.h"
//#include "./BSP/RTC/rtc.h"
//#include "./BSP/DMA/dma.h"
#include "./BSP/LCD/lcd.h"
//#include "./BSP/ADC/adc.h"
//#include "./BSP/MPU6050/MPU6050.h"
#include "./BSP/terazzo/terazzo.h"

//char* weekdays[]={"星期天","星期一","星期二","星期三","星期四","星期五","星期六"};

//uint8_t ID;                             //定义用于存放ID号的变量
 
int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    
    usart_init(115200);
    
    led_init();
    
    beep_init();
    
    key_init();
    
    extix_init();
    toggle_exit_bool();
    
    //OLED_Init();
    
    //rtc_init();                             /* 初始化RTC */
    
    lcd_init();                             /* 初始化LCD */
    
    //adc_init();                             /* 初始化ADC */
    
    init_terazzo();
    
    
    /*  */
    
    g_back_color = BLACK ;
    g_point_color=WHITE;
    lcd_clear(BLACK);
    //my_lcd_show_image(0,0,128,128,gImage_MYGO);
    //my_lcd_show_string(10,20,"星期一",32,0,g_point_color);
    //lcd_draw_line(100,200,200,-1,WHITE);
    int c=0;
    //update_terazzo(320,480,100);
    while (1)
    {
//        lcd_clear(terazzo_COLORS[c++]);
//        if(c==13){
//            c=0;
//        }
        
        LED0_TOGGLE(); /*红灯闪烁*/
        
        delay_ms(500);
    }
    
}






