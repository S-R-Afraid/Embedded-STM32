//#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/BEEP/beep.h"
#include "./BSP/EXTI/exti.h"
#include "./BSP/KEY/key.h"    //如果运行IWDG该行取消注释
//#include "./BSP/WDG/wdg.h"
//#include "./BSP/TIMER/btim.h"
//#include "./BSP/TIMER/gtim.h"
#include "./BSP/OLED/OLED.h"
//#include "./BSP/DHT11/DHT11.h"
#include "string.h"
//#include "./BSP/RTC/rtc.h"
#include "./BSP/DMA/dma.h"

 /* DMA相关 */
    const uint8_t TEXT_TO_SEND[] = {"正点原子 STM32 DMA 串口实验"}; /* 要循环发送的字符串 */

    #define SEND_BUF_SIZE       (sizeof(TEXT_TO_SEND) + 2) * 20        /* 发送数据长度, 等于sizeof(TEXT_TO_SEND) + 2的200倍；加2是回车和换行 */

    uint8_t g_sendbuf[SEND_BUF_SIZE];   /* 发送数据缓冲区 */

    extern DMA_HandleTypeDef  g_dma_handle;     /* DMA句柄 */
    
int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    usart_init(115200);
    led_init();
    beep_init();
    key_init();
    OLED_Init();
    //rtc_init();                             /* 初始化RTC */
    dma_init(DMA1_Channel4);                /* 初始化串口1 TX DMA */
    dma_m2m_init();                         /* 初始化内存到内存DMA */

   
    /* DMA */
    uint8_t  key = 0;
    uint16_t i, k;
    uint16_t len;
    uint8_t  mask = 0;
    float pro = 0;          /* 进度 */

    
    len = sizeof(TEXT_TO_SEND);
    k = 0;
    
    for (i = 0; i < SEND_BUF_SIZE; i++) /* 填充ASCII字符集数据 */
    {
        if (k >= len)   /* 入换行符 */
        {
            if(i < (SEND_BUF_SIZE-2))
            {
                g_sendbuf[i++] = 0x0d;    /* 回车\t */
                g_sendbuf[i] = 0x0a;    /* 换行\n */
                
            }
            else{
                g_sendbuf[i] = 0x20;    /* 空格  */
            }
            k = 0;
        }
        else     /* 复制TEXT_TO_SEND语句 */
        {
            mask = 0;
            g_sendbuf[i] = TEXT_TO_SEND[k];
            k++;
        }
    }
 
    i = 0;
    while (1)
    {
        key = key_scan(0);

        //if (key == KEY0_PRES)       /* KEY0按下 */
        if (g_usart_rx_sta & 0x8000||(key == KEY0_PRES))        /* 接收到了数据? */
        {
            g_usart_rx_sta=0;
            pro =0 ;
            OLED_Clear();
            printf("\r\nDMA DATA:\r\n");
            OLED_ShowString(0, 0, "Start Transimit....", OLED_6X8);
            OLED_ShowString(0, 8, "   %", OLED_6X8);    /* 显示百分号 */
            
            OLED_Update();
            
            dma_uart_Transmit(&g_uart1_handle, g_sendbuf, SEND_BUF_SIZE);
            //HAL_UART_Transmit_DMA(&g_uart1_handle, g_sendbuf, SEND_BUF_SIZE);
            /*串口用函数，目的地址，源地址，大小
            这个函数同时会使能dma中断，不过会把DMA的传输完成、一半、错误回调函数替换成串口的对应回调函数：
                                    UART_DMATransmitCplt、UART_DMATxHalfCplt、UART_DMAError；
            */
            /* 等待DMA传输完成，此时我们来做另外一些事情，比如点灯  
             * 实际应用中，传输数据期间，可以执行另外的任务 */

//            OLED_ShowNum(0, 8, 100, 3, OLED_6X8);    /* 显示100% */
//            OLED_ShowString(0, 16, "Transimit Finished!", OLED_6X8); /* 提示传送完成 */
        }

        i++;
        delay_ms(10);

        if (i == 20)
        {
            LED0_TOGGLE();  /* LED0闪烁,提示系统正在运行 */
            i = 0;
        }
        pro = __HAL_DMA_GET_COUNTER(&g_dma_handle);
        len = SEND_BUF_SIZE;        /* 总长度 */
        pro = 1 - (pro / len);      /* 得到百分比 */
        pro *= 100;                 /* 扩大100倍 */
        OLED_ShowNum(0, 8, pro, 3, OLED_6X8);
        //OLED_Update();
        OLED_Update();
    }
    

}





