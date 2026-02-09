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
//#include "./BSP/TIMER/gtim.h"
#include "./BSP/OLED/OLED.h"
//#include "./BSP/DHT11/DHT11.h"
#include "string.h"
#include "./BSP/ADC/adc.h"
#include "./BSP/SNAKE/snake.h"
/*
前排说明：OLED的SCL位接PB5，SDA接PB6；
        
        PS2摇杆X、Y接PA0、PA1,Z不接；
*/


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
    
    /* 定时器 */
    btim_tim6_int_init(500 - 1, 7200 - 1); /* 10Khz的计数频率，计数5K次为500ms */

    
    /*OLED相关变量*/
    /*在(96, 48)位置显示图像，宽16像素，高16像素，图像数据为Diode数组*/
	//OLED_ShowImage(96, 48, 16, 16, Diode);

    /* ADC */
    adc_nch_dma_init((uint32_t)&g_adc_dma_buf); /* 初始化ADC DMA采集 */
    adc_dma_enable(ADC_DMA_BUF_SIZE);   /* 启动ADC DMA采集 */
    
    /* 贪吃蛇初始化 */
    Ini_snake();
//    int forward=2;
    
    while(1){
        if(game_end==0){
            Forward(forward);
            Draw();
            int judge = Judge();
            if(judge==0){       //碰到身体
                GameEnd();      //结束游戏
            }else if(judge==1){ //碰到食物
                Increase();
                PutFood();
            }
        }
        //OLED_Update();
        
        delay_ms(500);
    }
    
}





