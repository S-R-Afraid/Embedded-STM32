#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "Key.h"
#include "RP.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"

/*OLED测试*/
//int main(void)
//{
//	OLED_Init();
//	
//	OLED_ShowString(0, 0, "Hello,世界。", OLED_8X16);
//	OLED_ShowFloatNum(0, 16, 12.345, 2, 3, OLED_8X16);
//	OLED_Printf(0, 32, OLED_8X16, "Num=%d", 666);
//	
//	OLED_Update();
//	
//	while (1)
//	{
//		
//	}
//}


/*LED测试*/
//int main(void)
//{
//	LED_Init();
//	
//	while (1)
//	{
//		LED_ON();
//		Delay_ms(500);
//		LED_OFF();
//		Delay_ms(500);
//		LED_Turn();
//		Delay_ms(500);
//		LED_Turn();
//		Delay_ms(500);
//	}
//}


/*定时中断和非阻塞式按键测试*/
//uint16_t i;
//uint16_t j;
//uint8_t KeyNum;

//int main(void)
//{
//	OLED_Init();
//	Key_Init();
//	
//	Timer_Init();
//	
//	while (1)
//	{
//		KeyNum = Key_GetNum();
//		if (KeyNum == 1)
//		{
//			j ++;
//		}
//		if (KeyNum == 2)
//		{
//			j --;
//		}
//		if (KeyNum == 3)
//		{
//			j += 10;
//		}
//		if (KeyNum == 4)
//		{
//			j -= 10;
//		}
//		
//		OLED_Printf(0, 0, OLED_8X16, "i:%05d", i);
//		OLED_Printf(0, 16, OLED_8X16, "j:%05d", j);
//		
//		OLED_Update();
//	}
//}

//void TIM1_UP_IRQHandler(void)
//{
//	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
//	{
//		i ++;
//		
//		Key_Tick();
//		
//		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
//	}
//}

/*电位器旋钮测试*/
//int main(void)
//{
//	OLED_Init();
//	RP_Init();
//	
//	while (1)
//	{
//		OLED_Printf(0, 0, OLED_8X16, "RP1:%04d", RP_GetValue(1));
//		OLED_Printf(0, 16, OLED_8X16, "RP2:%04d", RP_GetValue(2));
//		OLED_Printf(0, 32, OLED_8X16, "RP3:%04d", RP_GetValue(3));
//		OLED_Printf(0, 48, OLED_8X16, "RP4:%04d", RP_GetValue(4));
//		
//		OLED_Update();
//	}
//}

/*电机测试*/
//uint8_t KeyNum;
//int16_t PWM;

//int main(void)
//{
//	OLED_Init();
//	Key_Init();
//	Motor_Init();
//	
//	Timer_Init();
//	
//	while (1)
//	{
//		KeyNum = Key_GetNum();
//		if (KeyNum == 1)
//		{
//			PWM += 10;
//			if (PWM > 100) {PWM = 100;}
//		}
//		if (KeyNum == 2)
//		{
//			PWM -= 10;
//			if (PWM < -100) {PWM = -100;}
//		}
//		if (KeyNum == 3)
//		{
//			PWM = 0;
//		}
//		
//		Motor_SetPWM(PWM);
//		
//		OLED_Printf(0, 0, OLED_8X16, "PWM:%+04d", PWM);
//		
//		OLED_Update();
//	}
//}

//void TIM1_UP_IRQHandler(void)
//{
//	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
//	{
//		Key_Tick();
//		
//		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
//	}
//}


/*编码器测试*/
//int16_t Speed;
//int16_t Location;

//int main(void)
//{
//	OLED_Init();
//	Encoder_Init();
//	
//	Timer_Init();
//	
//	while (1)
//	{
//		OLED_Printf(0, 0, OLED_8X16, "Speed:%+05d", Speed);
//		OLED_Printf(0, 16, OLED_8X16, "Location:%+05d", Location);
//		
//		OLED_Update();
//	}
//}

//void TIM1_UP_IRQHandler(void)
//{
//	static uint16_t Count;
//	
//	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
//	{
//		Count ++;
//		if (Count >= 40)
//		{
//			Count = 0;
//			
//			Speed = Encoder_Get();
//			Location += Speed;
//		}
//		
//		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
//	}
//}


/*串口测试*/
uint16_t RP1, RP2, RP3, RP4;

int main(void)
{
	OLED_Init();
	RP_Init();
	Serial_Init();
	
	while (1)
	{
		RP1 = RP_GetValue(1);
		RP2 = RP_GetValue(2);
		RP3 = RP_GetValue(3);
		RP4 = RP_GetValue(4);
		
		OLED_Printf(0, 0, OLED_8X16, "RP1:%04d", RP1);
		OLED_Printf(0, 16, OLED_8X16, "RP2:%04d", RP2);
		OLED_Printf(0, 32, OLED_8X16, "RP3:%04d", RP3);
		OLED_Printf(0, 48, OLED_8X16, "RP4:%04d", RP4);
		
		OLED_Update();
		
		Serial_Printf("%d,%d,%d,%d\r\n", RP1, RP2, RP3, RP4);
		
		Delay_ms(10);
	}
}
