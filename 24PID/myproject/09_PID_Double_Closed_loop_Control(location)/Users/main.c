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
#include "PID.h"


//当前电机速度和位置
int16_t speed=0,location=0;

PID_t SpeedLoop,LocationLoop;



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
    
    Motor_SetPWM(0);
    
    /*初始化PID结构体*/
    PID_InitStruct(&SpeedLoop);
    PID_InitStruct(&LocationLoop);
    
    SpeedLoop.Kp = 5,SpeedLoop.Ki = 5,SpeedLoop.Kd=0;
    SpeedLoop.Out_max = 1000;SpeedLoop.Out_min=-1000;
    
    LocationLoop.Kp = 0.3,LocationLoop.Ki = 0.01,LocationLoop.Kd=0.1;
    LocationLoop.Out_max = 140;LocationLoop.Out_min=-140;
    
    PID_Enable_Opt(&SpeedLoop,PID_OPT_II);/*积分限幅*/
    SpeedLoop.II_MAX = SpeedLoop.Out_max/SpeedLoop.Ki;
    SpeedLoop.II_MIN = SpeedLoop.Out_min/SpeedLoop.Ki;
    
    
    PID_Enable_Opt(&LocationLoop,PID_OPT_IS);/*积分分离*/
    LocationLoop.IS_A=10;
    
    PID_Enable_Opt(&LocationLoop,PID_OPT_II);/*积分限幅*/
    LocationLoop.II_MAX = LocationLoop.Out_max/LocationLoop.Ki;
    LocationLoop.II_MIN = LocationLoop.Out_min/LocationLoop.Ki;
    
    while (1)
    {
        OLED_Clear();
        //OLED_Update();
        switch(Key_GetNum()){
            case 1:
                key_v+=204;
                break;
            case 2:
                key_v-=204;
                break;
            case 3:
                key_v=0;
                break;
            case 4:
                key_v=-key_v;
                break;
        }
        
        LocationLoop.Kp = RP_GetValue(1)/4095.0*7;
        LocationLoop.Ki = RP_GetValue(2)/4095.0*7;
        LocationLoop.Kd = RP_GetValue(3)/4095.0*7;
        //Target = RP_GetValue(4)/4095.0*1832 - 916 + key_v;
        //LocationLoop.Target = key_v;
        PID_SetTarget(&LocationLoop,key_v);
        
        OLED_Printf(0,0,OLED_6X8,"ADC:%-4d",RP_GetValue(1));
        OLED_Printf(0,8,OLED_6X8,"TAR:%+04.0f",LocationLoop.Target);
        OLED_Printf(0,16,OLED_6X8,"ACT:%+04.0f",LocationLoop.Actual_n);
        OLED_Printf(0,24,OLED_6X8,"OUT:%+04.0f",LocationLoop.Out);
        OLED_Printf(0,32,OLED_6X8,"EINT:%+04.0f",LocationLoop.ErrorInt);
        
        OLED_Printf(60,0,OLED_6X8,"P:%03.2f",LocationLoop.Kp);
        OLED_Printf(60,8,OLED_6X8,"I:%03.2f",LocationLoop.Ki);
        OLED_Printf(60,16,OLED_6X8,"D:%03.2f",LocationLoop.Kd);
        
        my_USART_printf(1,"%.0f,%.0f,%.0f\n",LocationLoop.Target,LocationLoop.Actual_n,LocationLoop.Out);
        OLED_Reverse();
        //OLED_Update();
        
        
        delay_ms(20);
    }
    
}










void TIM1_UP_IRQHandler()
{
    
    if(__HAL_TIM_GET_FLAG(&g_tim1_handle, TIM_FLAG_UPDATE) == SET)
    {
        Key_Tick();
        static int8_t count=0;
        count++;
        if(count%20==0){
            OLED_Update();
            
            speed = Encoder_Get();
            location+=speed;
            
            PID_UpdateActual(&SpeedLoop,speed);
            PID_UpdateOut(&SpeedLoop);
            
            Motor_SetPWM(SpeedLoop.Out);
        }
        
        if(count==40){
            count=0;
            
            PID_UpdateActual(&LocationLoop,location);
            
            PID_UpdateOut(&LocationLoop);
            
            
            /*执行控制*/
            PID_SetTarget(&SpeedLoop,LocationLoop.Out);
            
        }
        
        __HAL_TIM_CLEAR_IT(&g_tim1_handle, TIM_IT_UPDATE);  /* 清除定时器溢出中断标志位 */
    }
}







