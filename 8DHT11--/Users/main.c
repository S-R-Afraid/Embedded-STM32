//#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/BEEP/beep.h"
//#include "./BSP/EXTI/exti.h"
//#include "./BSP/KEY/key.h"    //如果运行IWDG该行取消注释
//#include "./BSP/WDG/wdg.h"
//#include "./BSP/TIMER/btim.h"
//#include "./BSP/TIMER/gtim.h"
#include "./BSP/OLED/OLED.h"
#include "./BSP/DHT11/DHT11.h"
#include "string.h"


int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    usart_init(115200);
    //key_init();

    DHT11_GPIO_Init();
    
    /*串口相关变量*/
    uint8_t len;
    uint16_t times = 0;
    UART_HandleTypeDef g_uart_handle=g_uart1_handle;
    
    /*温湿度传感器相关变量*/
    DHT11_Data Data1;
    while(1){
        /*读取数据*/
        if(DHT11_ReadData(&Data1) == SUCCESS)
        {
            printf("%d.%d>%d.%d",Data1.Temperature_H,
                                Data1.Temperature_L,
                                Data1.Humidity_H,
                                Data1.Humidity_L);
            
        }
        /*串口接收*/
        if (g_usart_rx_sta & 0x8000)        /* 接收到了数据? */
        {
            len = g_usart_rx_sta & 0x3fff;  /* 得到此次接收到的数据长度 */
            char* recv_str = (char*)g_usart_rx_buf;
            if(strcmp(recv_str,"beep")==0){
                printf("e:beep\r\n");
            }
            printf("recv:%s",recv_str);
            g_usart_rx_sta = 0;
        }
        delay_ms(200);
        
    }
    
}





