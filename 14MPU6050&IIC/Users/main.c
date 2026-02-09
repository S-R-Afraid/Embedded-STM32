//#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/BEEP/beep.h"
#include "./BSP/EXTI/exti.h"
#include "./BSP/KEY/key.h"
//#include "./BSP/WDG/wdg.h"
#include "./BSP/TIMER/btim.h"
//#include "./BSP/TIMER/gtim.h"
//#include "./BSP/OLED/OLED.h"
//#include "./BSP/DHT11/DHT11.h"
#include "string.h"
//#include "./BSP/RTC/rtc.h"
//#include "./BSP/DMA/dma.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/ADC/adc.h"
#include "./BSP/MPU6050/MPU6050.h"
#include "./BSP/Sandbox/sand.h"

/*手柄x\y接PC0\1,z不接*/
int16_t AX, AY, AZ, GX, GY, GZ;         //定义用于存放各个数据的变量,A是加速度，G是陀螺仪



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
    //rtc_init();                             /* 初始化RTC */
    lcd_init();                             /* 初始化LCD */
    //adc_init();                             /* 初始化ADC */
    toggle_exit_bool();
    MPU6050_Init();
    
    /* 定时器 */
    btim_tim6_int_init(500 - 1, 7200 - 1); /* 10Khz的计数频率，计数5K次为500ms */
    /* ADC */
    adc_nch_dma_init((uint32_t)&g_adc_dma_buf); /* 初始化ADC DMA采集 */
    adc_dma_enable(ADC_DMA_BUF_SIZE);   /* 启动ADC DMA采集 */
    
    
    
    lcd_clear(BLACK);
    //my_lcd_show_image(0,0,128,128,gImage_MYGO);
    //my_lcd_show_string(10,20,"星期一",32,0,g_point_color);
    lcd_draw_circle(200,420,40,WHITE);
    
    init_block();
    
    int t=0;
    
    
    //lcd_fill(10,450,20,460,WHITE);
    while (1)
    {
        /*
//        MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);		//获取MPU6050的数据
//        double aX=AX/32768.0*16;
//        double aY=AY/32768.0*16;
//        double aZ=AZ/32768.0*16;
//        double gX=GX/32768.0*16;
//        double gY=GY/32768.0*16;
//        double gZ=GZ/32768.0*16;
//        double G = sqrt(aX*aX+aY*aY+aZ*aZ);
//        gX=atan(aX/sqrt((aZ*aZ+aY*aY)))*180/3.14;
//        gY=atan(aY/sqrt((aX*aX+aY*aY)))*180/3.14;
//        gZ=atan(sqrt((aX*aX+aY*aY))/aZ)*180/3.14;
//        gX=aX/G;
//        gY=aY/G;
//        gZ=aZ/G;
//        
//        printf("%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",aX, aY, aZ, gX, gY, gZ,G);
//        lcd_fill(60,140,260,340,g_back_color);
//        lcd_draw_line(160,240,160+(100.0*gX),240+(100.0*-gY),g_point_color);
          
        */
        
        update_local(&block1);
        update_local(&block2);
        draw_sandblock(2,2,&block1);
        draw_sandblock(150,170,&block2);
        
        
        t++;
        if(t>=5){
            t=0;
            sand_clock();
            LED0_TOGGLE(); /*红灯闪烁*/
        }
        //delay_ms(10);
    }
    
}






