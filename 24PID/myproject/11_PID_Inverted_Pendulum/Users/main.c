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


#define CENTER_ANGLE    2035
#define CENTER_RANGE    500

//当前电机速度和位置
int16_t speed=0,location=0;

PID_t AngleLoop,LocationLoop;

uint16_t Angle;
uint8_t RunState = 0;

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    
    usart_init(1,115200);
    
    led_init();
    
    key_init();
    
    OLED_Init();
    
    
    
    LED_ON();
    
    RP_Init();
    
    Motor_Init();
    
    Encoder_Init();
    
    adc_dma_init((uint32_t)&Angle);
    adc_dma_enable(1);
    
    Timer_Init();
    
    /*初始化PID结构体*/
    PID_InitStruct(&AngleLoop);
    PID_SetTarget(&AngleLoop,CENTER_ANGLE);
    //PID_Enable_Opt(&AngleLoop,PID_OPT_II);  //积分限幅
    //AngleLoop.II_MAX = 5000;AngleLoop.II_MIN = -5000;
    //PID_Enable_Opt(&AngleLoop,PID_OPT_ID);  //不完全微分
    //AngleLoop.ID_A = 0.5;
    //PID_Enable_Opt(&AngleLoop,PID_OPT_IS);  //积分分离
    //AngleLoop.IS_A = 50;
    //PID_Enable_Opt(&AngleLoop,PID_OPT_DL);  //微分先行
    AngleLoop.Kp = 2.0,AngleLoop.Ki = 0.03,AngleLoop.Kd = 6.21;
    AngleLoop.Out_max = 1000;AngleLoop.Out_min = -1000;
    
    PID_InitStruct(&LocationLoop);
    PID_SetTarget(&LocationLoop,0);
    LocationLoop.Kp = 0.5,LocationLoop.Ki = 0,LocationLoop.Kd = 5.72;
    LocationLoop.Out_max = 70;LocationLoop.Out_min = -70;
    
    int key_v = 0;
    float* pid[3] = {&AngleLoop.Kp,&AngleLoop.Ki,&AngleLoop.Kd};
    
    while (1)
    {
        OLED_Clear();
        //OLED_Update();
        switch(Key_GetNum()){
            case 1:
                RunState=!RunState;
                AngleLoop.Out =0;
                AngleLoop.ErrorInt=0;
                break;
            case 2:
                *pid[key_v]+=0.02f;
                break;
            case 3:
                *pid[key_v]-=0.02f;
                break;
            case 4:
                key_v++;
                if(key_v==3)key_v=0;
                break;
        }
        
        //AngleLoop.Kp=RP_GetValue(1)/4095.0 *15;
        //AngleLoop.Ki=RP_GetValue(2)/4095.0 *15;
        //AngleLoop.Kd=RP_GetValue(3)/4095.0 *15;
        LocationLoop.II_MAX = LocationLoop.Out_max/LocationLoop.Ki;
        LocationLoop.II_MIN = LocationLoop.Out_min/LocationLoop.Ki;
        
        //Motor_SetPWM(RP_GetValue(4)/4095.0 *1000);
        my_USART_printf(1,"%d,%d,%d\n",(int)LocationLoop.Target,location,(int)LocationLoop.Out);
        
        //OLED_Update();
        //adc_dma_enable(1);
        OLED_Printf(0,0,OLED_6X8,"P:%-3.2f",AngleLoop.Kp);
        OLED_Printf(0,8,OLED_6X8,"I:%-3.2f",AngleLoop.Ki);
        OLED_Printf(0,16,OLED_6X8,"D:%-3.2f",AngleLoop.Kd);
        OLED_Printf(0,24,OLED_6X8,"tar:%d",(int)AngleLoop.Target);
        OLED_Printf(0,32,OLED_6X8,"act:%d",Angle);
        OLED_Printf(0,40,OLED_6X8,"out:%d",(int)AngleLoop.Out);
        
        OLED_Printf(64,0,OLED_6X8,"P:%-3.2f",LocationLoop.Kp);
        OLED_Printf(64,8,OLED_6X8,"I:%-3.2f",LocationLoop.Ki);
        OLED_Printf(64,16,OLED_6X8,"D:%-3.2f",LocationLoop.Kd);
        OLED_Printf(64,24,OLED_6X8,"tar:%d",(int)LocationLoop.Target);
        OLED_Printf(64,32,OLED_6X8,"act:%d",location);
        OLED_Printf(64,40,OLED_6X8,"out:%d",(int)LocationLoop.Out);
        OLED_Reverse();
        OLED_ReverseArea(64,key_v*8,8,8);
        OLED_Update();
        LED0(!RunState);
        delay_ms(20);
    }
    
}




void TIM1_UP_IRQHandler()
{
    
    if(__HAL_TIM_GET_FLAG(&g_tim1_handle, TIM_FLAG_UPDATE) == SET)
    {
        Key_Tick();
        adc_dma_enable(1);//读取角度传感器值
        
        speed = Encoder_Get();
        location+=speed;    //获取电机的速度和位置
        
        if(abs(Angle-CENTER_ANGLE) > CENTER_RANGE){ //如果误差太大则自动退出调控
            RunState=0;
            AngleLoop.Out =0;
            AngleLoop.ErrorInt=0;
            
            PID_SetTarget(&AngleLoop,CENTER_ANGLE);
            LocationLoop.Out=0;
            LocationLoop.ErrorInt=0;
            location=0;
        }
        
        static uint8_t count1 = 0,count2 = 0;
        
        if(RunState){       //如果当前处于调控状态
            count1++;
            if(count1>=2){
                count1=0;
                PID_UpdateActual(&AngleLoop,Angle);     //更新实际值
                PID_UpdateOut(&AngleLoop);              //计算输出
                Motor_SetPWM(AngleLoop.Out);            //调控
            }
        }else{
            
            Motor_SetPWM(0);
        }
        
        if(RunState){
            count2++;
            if(count2>=20){
                
                count2=0;
                PID_UpdateActual(&LocationLoop,location);
                PID_UpdateOut(&LocationLoop);
                
                PID_SetTarget(&AngleLoop,CENTER_ANGLE - LocationLoop.Out);    //调控
            }
        }else{
            
            
        }
        

        
        __HAL_TIM_CLEAR_IT(&g_tim1_handle, TIM_IT_UPDATE);  /* 清除定时器溢出中断标志位 */
    }
}







