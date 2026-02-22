#include "stm32f10x.h"                  // Device header
#include "PWM.h"

/**
  * 函    数：直流电机初始化
  * 参    数：无
  * 返 回 值：无
  */
void Motor_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIOB的时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PB12、PB13、PB14和PB15引脚初始化为推挽输出
	
	PWM_Init();												//初始化直流电机的底层PWM
}

/**
  * 函    数：直流电机设置PWM
  * 参    数：n 指定哪个电机，范围：1~2
  * 参    数：PWM 要设置的PWM值，范围：-100~100（负数为反转）
  * 返 回 值：无
  */
void Motor_SetPWM(uint8_t n, int16_t Duty)
{
	if (n == 1)		//指定电机1
	{
		if (Duty >= 0)							//如果设置正转
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_12);	//置方向控制引脚电平
			GPIO_SetBits(GPIOB, GPIO_Pin_13);
			PWM_SetCompare1(Duty);				//设置PWM占空比
		}
		else									//否则，即设置反转
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_12);	//置方向控制引脚电平
			GPIO_ResetBits(GPIOB, GPIO_Pin_13);
			PWM_SetCompare1(-Duty);				//设置PWM占空比
		}
	}
	else if (n == 2)		//指定电机2
	{
		if (Duty >= 0)							//如果设置正转
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_14);	//置方向控制引脚电平
			GPIO_SetBits(GPIOB, GPIO_Pin_15);
			PWM_SetCompare2(Duty);				//设置PWM占空比
		}
		else									//否则，即设置反转
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_14);	//置方向控制引脚电平
			GPIO_ResetBits(GPIOB, GPIO_Pin_15);
			PWM_SetCompare2(-Duty);				//设置PWM占空比
		}
	}
	
}
