//#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
//#include "./BSP/EXTI/exti.h"
#include "./BSP/KEY/key.h"
#include "./BSP/TIMER/gtim.h"
#include "./BSP/TIMER/atim.h"
#include "./BSP/OLED/OLED.h"
#include "./BSP/ADC/adc.h"
#include "string.h"
#include "./BSP/Motor/Motor.h"
#include "./BSP/Encoder/Encoder.h"





int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    
    usart_init(1,115200);
    
    led_init();
    
    key_init();
    
    //extix_init();
    //toggle_exit_bool();

    
    OLED_Init();
    
    LED_ON();
    
    Timer_Init();
    
    RP_Init();
    
    Motor_Init();
    
    Encoder_Init();
    
    int key_v = 0;
    extern int16_t speed,location;
    Motor_SetPWM(0);
    while (1)
    {
        OLED_Clear();
        OLED_Update();
        switch(Key_GetNum()){
            case 1:
                key_v+=100;
                break;
            case 2:
                key_v-=100;
                break;
            case 3:
                key_v=0;
                break;
            case 4:
                key_v=-key_v;
                break;
        }
        
        
        OLED_Printf(0,0,OLED_8X16,"ADC:%-4d",RP_GetValue(1));
        OLED_Printf(0,16,OLED_8X16,"KEY:%-4d",key_v);
        OLED_Printf(0,32,OLED_8X16,"SPD:%-3d",speed);
        OLED_Printf(0,48,OLED_8X16,"LCA:%-4d",location);
        OLED_ReverseArea(0,0,70,70);
        OLED_Update();
        
        Motor_SetPWM((RP_GetValue(1)/4095.0)*2000-1000+key_v);
        my_USART_printf(1,"%d,%d,%d,%d\n",RP_GetValue(1),RP_GetValue(2),RP_GetValue(3),RP_GetValue(4));
        
        delay_ms(20);
    }
    
}






