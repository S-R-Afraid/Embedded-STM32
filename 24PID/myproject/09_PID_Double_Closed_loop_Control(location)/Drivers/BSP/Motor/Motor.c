#include "./SYSTEM/sys/sys.h"
//#include "PWM.h"

#include "./BSP/Motor/Motor.h"
/**
  * 函    数：直流电机初始化
  * 参    数：无
  * 返 回 值：无
  */
void Motor_Init(void)
{
    
    GPIO_InitTypeDef gpio_init_struct;
    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio_init_struct.Pin = GPIO_PIN_13;                     /* 引脚 */
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;            /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;          /* 高速 */
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);       /* 初始化引脚 */
    
    gpio_init_struct.Pin = GPIO_PIN_12;                     /* 引脚 */
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);       /* 初始化引脚 */
    PWM_Init();                                             //初始化直流电机的底层PWM
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12,GPIO_PIN_SET);	    //PA4置高电平
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13,GPIO_PIN_RESET);	//PA5置低电平，设置方向为正转
}

/**
  * 函    数：直流电机设置速度
  * 参    数：Speed 要设置的速度，范围：-100~100
  * 返 回 值：无
  */
void Motor_SetPWM(int16_t Speed)
{
    
	if (Speed >= 0)							//如果设置正转的速度值
	{   
        if(Speed>1000)Speed=1000;
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12,GPIO_PIN_RESET);	    //PA4置低电平
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13,GPIO_PIN_SET);	//PA5置高电平，设置方向为正转
		PWM_SetCompare1(Speed);				//PWM设置为速度值
	}
	else									//否则，即设置反转的速度值
	{   
        if(Speed<-1000)Speed=-1000;
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12,GPIO_PIN_SET);	//PA4置低电平
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13,GPIO_PIN_RESET);	    //PA5置高电平，设置方向为反转
		PWM_SetCompare1(-Speed);			//PWM设置为负的速度值，因为此时速度值为负数，而PWM只能给正数
	}
}



void PWM_Init(){
    gtim_timx_pwm_chy_init(1000-1,36-1);
}

void PWM_SetCompare1(uint16_t Compare){
    __HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle,GTIM_TIMX_PWM_CHY,Compare);
}