//#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/BEEP/beep.h"
//#include "./BSP/EXTI/exti.h"
#include "./BSP/KEY/key.h"
//#include "./BSP/WDG/wdg.h"
//#include "./BSP/TIMER/btim.h"
#include "./BSP/TIMER/gtim.h"
//#include "./BSP/OLED/OLED.h"
//#include "./BSP/DHT11/DHT11.h"
#include "string.h"
//#include "./BSP/RTC/rtc.h"
//#include "./BSP/DMA/dma.h"
#include "./BSP/LCD/lcd.h"
//#include "./BSP/ADC/adc.h"
//#include "./BSP/MPU6050/MPU6050.h"
//#include "./BSP/terazzo/terazzo.h"
#include "./BSP/HC-SR04/HC-SR04.h"



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
    
    //extix_init();
    //toggle_exit_bool();
    
    Init_HC_SR04();
    
    //OLED_Init();
    
    //rtc_init();                             /* 初始化RTC */
    
    lcd_init();                             /* 初始化LCD */
    
    //adc_init();                             /* 初始化ADC */
    
    
    /*  */
    
    g_back_color = BLACK ;
    g_point_color=WHITE;
    lcd_clear(BLACK);
    
    double c=15;char str[20];
    uint32_t temp = 0;
    //update_terazzo(320,480,100);
    while (1)
    {
        //LED1_TOGGLE();
        HC_SR04_start_measure();
        delay_ms(500);
//        if (g_timxchy_cap_sta & 0X80)       /* 成功捕获到了一次高电平 */
//        {
//            temp = g_timxchy_cap_sta & 0X3F;
//            temp *= 65536;                  /* 溢出时间总和 */
//            temp += g_timxchy_cap_val;      /* 得到总的高电平时间 */
//            //printf("HIGH:%d us\r\n", temp); /* 打印总的高点平时间 */
//            g_timxchy_cap_sta = 0;          /* 开启下一次捕获*/
//        }
//        c = temp/58.0f;
        c = HC_SR04_get_distance();
        sprintf((char *)str,"%lf",c);
        my_lcd_show_string(20,200,str,16,0,g_point_color);
        LED0_TOGGLE(); /*红灯闪烁*/
        
        //delay_ms(500);
    }
    
}






