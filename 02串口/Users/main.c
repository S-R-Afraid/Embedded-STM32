#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/BEEP/beep.h"
#include "./BSP/EXTI/exti.h"

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);//初始化时钟
    delay_init(72);//初始化延时
    usart_init(115200);//初始化串口
    led_init();//初始化LED
    //extix_init();//初始化外部中断
    LED0(0);
    //toggle_exit_bool();

    uint8_t len;
    uint16_t times = 0;
    UART_HandleTypeDef g_uart_handle=g_uart1_handle;
    while (1)
    {
        if (g_usart_rx_sta & 0x8000)        /* 接收到了数据? */
        {
            len = g_usart_rx_sta & 0x3fff;  /* 得到此次接收到的数据长度 */
            printf("\r\nyour sended messages were:%d\r\n",len);

            HAL_UART_Transmit(&g_uart_handle,(uint8_t*)g_usart_rx_buf, len, 1000);    /* 以阻塞方式发送接收到的数据 */
            //四个参数分别是串口句柄、发送数据起始地址、数据长度(字节单位)、超时时间
            //其中串口句柄就是在usart.c里初始化用到的那个结构体
            //我试过了自己单独定义一个一模一样的结构体也行
            while(__HAL_UART_GET_FLAG(&g_uart_handle,UART_FLAG_TC) != SET);           /* 等待发送结束 */
            printf("\r\n\r\n");             /* 插入换行 */
            g_usart_rx_sta = 0;
        }
        else
        {
            times++;

            if (times % 200 == 0)printf("input,end as enter:exti:%d\r\n",GET_MY_USART_EXTI_NUM());

            if (times % 30  == 0) LED0_TOGGLE(); /* 闪烁LED,提示系统正在运行. */

            delay_ms(10);
        }
    }
}


