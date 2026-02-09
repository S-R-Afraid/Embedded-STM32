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
//#include "./BSP/HC-SR04/HC-SR04.h"
#include "./BSP/MyRandom/MyRandom.h"
#include "./BSP/Maze/maze.h"

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
    
    
    //sprintf((char *)str,"%d",(MAZE_WALL){5,2,3,4}.i1);
    
    ergodic_separate_generate(0,0,29,39,(PRNG_SEED){16807,0,pow(2,31)-1,35});
    
    //my_lcd_show_string(20,200,str,16,0,g_point_color);
    while (1)
    {
       
        for(int x=0;x<MAP_WIDTH;x++){
            for(int y =0;y<MAP_LENGTH;y++){
                lcd_show_num(MAP_LX+x*MAP_BLOCK_WIDTH,MAP_LY + y*MAP_BLOCK_WIDTH,maze_map[x][y],2,12,WHITE);
            }
            
        }
        LED0_TOGGLE(); /*红灯闪烁*/
        
        delay_ms(500);
    }
    
}






