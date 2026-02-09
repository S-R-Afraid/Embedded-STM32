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
#include "./BSP/OLED/OLED.h"
//#include "./BSP/DHT11/DHT11.h"
#include "string.h"
#include "./BSP/RTC/rtc.h"


/* 定义字符数组用于显示周 */
char* weekdays[]={"Sunday","Monday","Tuesday","Wednesday",
                  "Thursday","Friday","Saterday"};

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    usart_init(115200);
    led_init();
    beep_init();
    //key_init();
    OLED_Init();
    
    rtc_init();                             /* 初始化RTC */
    
    /*串口相关变量*/
    uint8_t len;
    uint16_t times = 0;
    UART_HandleTypeDef g_uart_handle=g_uart1_handle;
    //通过串口调整RTC时间
    int usart_year=2025;
    int usart_month=3,usart_day=30,usart_hour=19,usart_min=50,usart_sec=0;
    
    /* RTC相关变量 */
    uint8_t tbuf[40];
    uint8_t t = 0;
    //rtc_set_alarm(2025, 3, 30, 17, 38, 45); /* 设置一次闹钟 */

    printf("open");
    while (1)
    {
        t++;
        //OLED_ShowString(8,16,"123",OLED_8X16);
        
        if ((t % 10) == 0)                  /* 每100ms更新一次显示数据 */
        {
            OLED_Clear();
            rtc_get_time();
            sprintf((char *)tbuf, "Time:%02d:%02d:%02d", calendar.hour, calendar.min, calendar.sec);
            OLED_ShowString(0, 0, (char *)tbuf, OLED_8X16);
            sprintf((char *)tbuf, "Date:%04d-%02d-%02d", calendar.year, calendar.month, calendar.date);
            OLED_ShowString(0, 16, (char *)tbuf, OLED_8X16);
            sprintf((char *)tbuf, "Week:%s", weekdays[calendar.week]);
            OLED_ShowString(0, 32, (char *)tbuf, OLED_8X16);
            OLED_Update();
        }

        if ((t % 20) == 0)
        {
            LED0_TOGGLE();              /* 每200ms,翻转一次LED0 */
            BEEP(0);
        }

        delay_ms(10);
        /*串口接收*/
        if (g_usart_rx_sta & 0x8000)        /* 接收到了数据? */
        {
            
            len = g_usart_rx_sta & 0x3fff;  /* 得到此次接收到的数据长度 */
            char* recv_str = (char*)g_usart_rx_buf;
            
            sscanf(recv_str,"%d%d%d%d%d%d",&usart_year,&usart_month,&usart_day,&usart_hour,&usart_min,&usart_sec);
            printf("%d %d %d %d %d %d\n",usart_year,usart_month,usart_day,usart_hour,usart_min,usart_sec);
            rtc_set_time(usart_year,usart_month,usart_day,usart_hour,usart_min,usart_sec);
            
            g_usart_rx_sta = 0;
        }
    }
    

}





