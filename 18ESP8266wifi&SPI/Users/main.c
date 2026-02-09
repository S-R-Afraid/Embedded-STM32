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
#include "./BSP/ESP8266/esp8266.h"



//char* weekdays[]={"星期天","星期一","星期二","星期三","星期四","星期五","星期六"};

//uint8_t ID;                             //定义用于存放ID号的变量



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
    
    Init_HC_SR04();
    
    //OLED_Init();
    
    //rtc_init();                             /* 初始化RTC */
    
    lcd_init();                             /* 初始化LCD */
    
    //adc_init();                             /* 初始化ADC */
    
    ESP8266_Init();
    ESP8266_CH_ENABLE();
    //ESP8266_Usart("AT\r\n");
    //ESP8266_JoinAP ( User_ESP8266_ApSsid, User_ESP8266_ApPwd );
    ESP8266_StaTcpClient(); //WiFi模块设置
    //ESP8266_AT_Test ();
    
    
    /*  */
    my_USART_printf(1,"设备已开启\r\n");
    g_back_color = BLACK ;
    g_point_color=WHITE;
    lcd_clear(BLACK);
    
    double c=15;char str[20];
    uint32_t temp = 0;
    while (1)
    {
        LED1_TOGGLE();
        switch(ESP8266_USART_receiveByte)
		{
			case 'a': LED0(0);break;//开灯
			
			case 'c': LED0(1);break;//关灯
		}	
    }
    
}






