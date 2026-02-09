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
    led_init();
    beep_init();
    //key_init();
    
    OLED_Init();
    DHT11_GPIO_Init();
    
    /*串口相关变量*/
    uint8_t len;
    uint16_t times = 0;
    UART_HandleTypeDef g_uart_handle=g_uart1_handle;
    
    /*温湿度传感器相关变量*/
    DHT11_Data Data1;
    
    /*OLED相关变量*/
    uint8_t han[4][32]={
        0x08,0x08,0x08,0x78,0x88,0x0B,0x0B,0x0C,
        0x08,0x88,0x88,0x78,0x08,0x08,0x08,0x00,
        0x80,0x80,0x80,0x40,0x43,0x34,0x34,0x08,
        0x34,0x43,0x43,0x40,0x80,0x80,0x80,0x00,//文

        0x80,0x83,0x83,0x83,0x83,0x83,0x83,0xFF,
        0x83,0x83,0x83,0x83,0x83,0x80,0x80,0x00,
        0x00,0x00,0x00,0x00,0x80,0x80,0x80,0xFF,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//于

        0x00,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,
        0xFF,0x40,0x40,0x40,0x40,0x00,0x00,0x00,
        0x80,0x80,0x80,0xFF,0x80,0x80,0x80,0x80,
        0xFF,0x80,0x80,0x80,0x80,0x80,0x80,0x00,//止

        0x00,0x4F,0x4F,0x4B,0x4F,0x4B,0x4B,0xFF,
        0x4B,0x4F,0x4F,0x4B,0x4F,0x00,0x00,0x00,
        0x83,0xB7,0xB7,0xB3,0xB3,0xB7,0xB7,0xFB,
        0xB7,0xB3,0xB3,0xB3,0xB7,0x83,0x83,0x00,//墨
    };
    OLED_ShowHan(1,1,4,han);
    OLED_ShowString(2, 1, "temp:");
    OLED_ShowString(3, 1, "humidity:");
    OLED_ShowString(4, 1, "BEEP:");
    printf("beep\r\n");
    while(1){
        /*读取数据*/
        if(DHT11_ReadData(&Data1) == SUCCESS)
        {
            OLED_ShowNum(2, 6, Data1.Temperature_H, 2);
            OLED_ShowChar(2, 8, '.');
            OLED_ShowNum(2, 9, Data1.Temperature_L, 2);
            OLED_ShowChar(2, 11, 'C');
            OLED_ShowNum(3, 9, Data1.Humidity_H, 2);
            OLED_ShowChar(3, 11, '.');
            OLED_ShowNum(3, 12, Data1.Humidity_L, 2);
            OLED_ShowChar(3, 14, '%');
        }
        /*判断BEEP*/
        if(HAL_GPIO_ReadPin(BEEP_GPIO_PORT,BEEP_GPIO_PIN)){
            OLED_ShowString(4, 6, "      ");
            OLED_ShowString(4, 6, "OPEN");
        }else{
            OLED_ShowString(4, 6, "      ");
            OLED_ShowString(4, 6, "CLOSE");
        }
        /*串口接收*/
        if (g_usart_rx_sta & 0x8000)        /* 接收到了数据? */
        {
            len = g_usart_rx_sta & 0x3fff;  /* 得到此次接收到的数据长度 */
            char* recv_str = (char*)g_usart_rx_buf;
            if(strcmp(recv_str,"beep")==0){
                BEEP_TOGGLE();
                printf("e:beep\r\n");
            }
            printf("recv:%s",recv_str);
            g_usart_rx_sta = 0;
        }
        
    }
    
}





