//#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/BEEP/beep.h"
#include "./BSP/EXTI/exti.h"
#include "./BSP/KEY/key.h"
//#include "./BSP/WDG/wdg.h"
//#include "./BSP/TIMER/btim.h"
#include "./BSP/TIMER/gtim.h"
//#include "./BSP/OLED/OLED.h"
//#include "./BSP/DHT11/DHT11.h"
#include "string.h"
#include "./BSP/RTC/rtc.h"
//#include "./BSP/DMA/dma.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/ADC/adc.h"
#include "./BSP/TB6612FNG/TB6612FNG.h"

char* weekdays[]={"星期天","星期一","星期二","星期三",
                  "星期四","星期五","星期六"};

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    usart_init(115200);
    led_init();
    beep_init();
    key_init();
    extix_init();
    //OLED_Init();
    rtc_init();                             /* 初始化RTC */
    lcd_init();                             /* 初始化LCD */
    adc_init();                             /* 初始化ADC */
    TB6612_init();
    toggle_exit_bool();
    /* RTC相关变量 */
    uint8_t tbuf[40];
    
    uint8_t x = 95;
    uint8_t lcd_id[12];
    //sprintf((char *)lcd_id, "LCD ID:%04X", lcddev.id);  /* 将LCD ID打印到lcd_id数组 */
    g_back_color = BLACK ;
    g_point_color=WHITE;
    
    /* ADC */
    uint16_t adcx;
    float temp;
    
    /*  */
    
    
    lcd_clear(BLACK);
    //my_lcd_show_image(0,0,128,128,gImage_MYGO);
    //my_lcd_show_string(10,20,"星期一",32,0,g_point_color);
    my_lcd_show_string(30, 130, "ADC1_CH1_VAL:", 16, 0, WHITE);
    my_lcd_show_string(30, 150, "ADC1_CH1_VOL:0.000V", 16, 0, WHITE); /* 先在固定位置显示小数点 */
    
    TB6612_STBY_ENABLE();
    TB6612_AHL();
    while (1)
    {
        adcx = adc_get_result_average(ADC_ADCX_CHY, 10);    /* 获取通道5的转换值，10次取平均 */
        lcd_show_xnum(134, 130, adcx, 5, 16, 0, WHITE);        /* 显示ADCC采样后的原始值 */
 
        TB6612_SET_A_VALUE(adcx);
        
        temp = (float)adcx * (3.3 / 4096);                  /* 获取计算后的带小数的实际电压值，比如3.1111 */
        adcx = temp;                                        /* 赋值整数部分给adcx变量，因为adcx为u16整形 */
        lcd_show_xnum(134, 150, adcx, 1, 16, 0, WHITE);      /* 显示电压值的整数部分，3.1111的话，这里就是显示3 */

        temp -= adcx;                                       /* 把已经显示的整数部分去掉，留下小数部分，比如3.1111-3=0.1111 */
        temp *= 1000;                                       /* 小数部分乘以1000，例如：0.1111就转换为111.1，相当于保留三位小数。 */
        lcd_show_xnum(150, 150, temp, 3, 16, 0X80, WHITE);   /* 显示小数部分（前面转换为了整形显示），这里显示的就是111. */
        
        
        
//        LED0_TOGGLE(); /*红灯闪烁*/
        delay_ms(20);
    }
    
}






