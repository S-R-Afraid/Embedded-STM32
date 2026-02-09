//#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/BEEP/beep.h"
//#include "./BSP/EXTI/exti.h"
#include "./BSP/KEY/key.h"
//#include "./BSP/WDG/wdg.h"
#include "./BSP/TIMER/btim.h"
//#include "./BSP/TIMER/gtim.h"
#include "./BSP/OLED/OLED.h"
//#include "./BSP/DHT11/DHT11.h"
#include "string.h"
//#include "./BSP/RTC/rtc.h"
//#include "./BSP/DMA/dma.h"
#include "./BSP/LCD/lcd.h"
//#include "./BSP/ADC/adc.h"
//#include "./BSP/MPU6050/MPU6050.h"
//#include "./BSP/terazzo/terazzo.h"
//#include "./BSP/HC-SR04/HC-SR04.h"
#include "./BSP/ESP8266/esp8266.h"
#include "./BSP/OLED/OLEDUI.h"


//char* weekdays[]={"星期天","星期一","星期二","星期三","星期四","星期五","星期六"};

//uint8_t ID;                             //定义用于存放ID号的变量

void * blank_func(void *args){
    return args;
}
void * line_LEDT(void *args){
//    if(((char*)args)[9]==OLED_KeepPress)
//        LED0(0);
    if(((char*)args)[9]==OLED_ReleaseKeepPress)
        LED0_TOGGLE();
    if(((char*)args)[9]==OLED_Press)
        LED1_TOGGLE();
    return args;
}
void * comepage(void* args){
    if(((char*)args)[9]==OLED_DoubleClick)
        OLED_UI_ShowPage(((char*)args)[0],0);
    return args;
}

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
    
    //Init_HC_SR04();
    
    OLED_Init();
    OLED_UI_Init();
    
    //rtc_init();                             /* 初始化RTC */
    
    //lcd_init();                             /* 初始化LCD */
    
    //adc_init();                             /* 初始化ADC */
    
    
    
    g_back_color = BLACK ;
    g_point_color=WHITE;
    //lcd_clear(BLACK);
    
    char ar[10] = { 1,1, };
    OLED_Pages[0].linenum = 6;
    
    OLED_UI_SetLine(0,0,"line1\0",blank_func,ar);
    OLED_UI_SetLine(0,1,"line2\0",blank_func,ar);
    OLED_UI_SetLine(0,2,"line3\0",blank_func,ar);
    OLED_UI_SetLine(0,3,"短按LED1长按切换LED0\0",line_LEDT,ar);
    OLED_UI_SetLine(0,4,"长长长长长长的一段话\0",blank_func,ar);
    OLED_UI_SetLine(0,5,"line6\0",blank_func,ar);
    OLED_UI_SetLine(0,6,"双击下一页\0",comepage,ar);
    
    
    char ar2[10] = { 0,0, };
    int pi = OLED_UI_AddNewPage();
    OLED_Pages[pi].linenum = 6;
    OLED_UI_SetLine(pi,0,"双击返回上一页\0",comepage,ar2);
    OLED_UI_SetLine(pi,1,"2line2\0",blank_func,ar2);
    OLED_UI_SetLine(pi,2,"2line3\0",blank_func,ar2);
    OLED_UI_SetLine(pi,3,"2LED1longlonglong\0",line_LEDT,ar2);
    OLED_UI_SetLine(pi,4,"2line5\0",blank_func,ar2);
    OLED_UI_SetLine(pi,5,"2line6\0",blank_func,ar2);
    OLED_UI_SetLine(pi,6,"2line7\0",blank_func,ar2);
    OLED_Update();
    
    int x=0;
    uint32_t temp = 0;
    while (1)
    {
        //LED1_TOGGLE();
        my_USART_printf(1,"%d\n",__HAL_TIM_GetCounter(&OLED_TIM_Hander));
        OLED_UI_NewFrame();
        //OLED_Update();
        delay_ms(20);
    }
    
}






