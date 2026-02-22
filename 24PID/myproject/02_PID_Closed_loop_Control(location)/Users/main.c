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


//当前电机速度和位置
int16_t speed=0,location=0;
float Target = 0,Actual = 0,Out = 0;		//目标值，实际值，输出值
float Kp =4,Ki =2,Kd=0.3;		//比例项，积分项，微分项的权重
float Error0, Error1, ErrorInt;	//本次误差，上次误差，误差积分

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
        
        Kp = RP_GetValue(1)/4095.0*20;
        Ki = RP_GetValue(2)/4095.0*20;
        Kd = RP_GetValue(3)/4095.0*20;
        //Target = RP_GetValue(4)/4095.0*1832 - 916 + key_v;
        Target = key_v;
        
        OLED_Printf(0,0,OLED_6X8,"ADC:%-4d",RP_GetValue(1));
        OLED_Printf(0,8,OLED_6X8,"TAR:%+04.0f",Target);
        OLED_Printf(0,16,OLED_6X8,"ACT:%+04.0f",Actual);
        OLED_Printf(0,24,OLED_6X8,"OUT:%+04.0f",Out);
        OLED_Printf(0,32,OLED_6X8,"EINT:%+04.0f",ErrorInt);
        
        OLED_Printf(60,0,OLED_6X8,"P:%02.1f",Kp);
        OLED_Printf(60,8,OLED_6X8,"I:%02.1f",Ki);
        OLED_Printf(60,16,OLED_6X8,"D:%02.1f",Kd);
        
        my_USART_printf(1,"%.0f,%.0f,%.0f\n",Target,Actual,Out);
        //OLED_ReverseArea(0,0,54,63);
        OLED_Reverse();
        //OLED_Update();
        
        //Motor_SetPWM((RP_GetValue(1)/4095.0)*2000-1000+Target);
        //Motor_SetPWM(Target);
        //my_USART_printf(1,"%d,%d,%d,%d\n",RP_GetValue(1),RP_GetValue(2),RP_GetValue(3),RP_GetValue(4));
        
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
        if(count>20){
            count=0;
            OLED_Update();
            
            speed = Encoder_Get();
            location+=speed;
            
            
            Actual = location;
            Error1 = Error0;
            Error0 = Target - Actual;
            
            /*误差积分（累加）*/
            ErrorInt += Error0;
            /*积分限幅*/
            if(ErrorInt > 600){ErrorInt = 600;}
            if(ErrorInt < -600){ErrorInt = -600;}
            
            /*PID计算*/
            Out = Kp * Error0 + Ki * ErrorInt + Kd * (Error0 - Error1);
            
            /*输岀限幅*/
            if(Out > 1000){Out = 1000;}
            if(Out < -1000){Out = -1000;}
            
            /*执行控制*/
            Motor_SetPWM(Out);
            
        }
        __HAL_TIM_CLEAR_IT(&g_tim1_handle, TIM_IT_UPDATE);  /* 清除定时器溢出中断标志位 */
    }
}







