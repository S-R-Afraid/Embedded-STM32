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

uint8_t KeyNum;

int16_t Speed, Location;

float InnerTarget, InnerActual, InnerOut;
float InnerKp = 0.3, InnerKi = 0.3, InnerKd = 0;
float InnerError0, InnerError1, InnerErrorInt;

float OuterTarget, OuterActual, OuterOut;
float OuterKp = 0.3, OuterKi = 0, OuterKd = 0.4;
float OuterError0, OuterError1, OuterErrorInt;

int main(void)
{
	OLED_Init();
	Key_Init();
	Motor_Init();
	Encoder_Init();
	RP_Init();
	Serial_Init();
	
	Timer_Init();
	
	OLED_Printf(0, 0, OLED_8X16, "2*PID Control");
	OLED_Update();
	
	while (1)
	{
//		KeyNum = Key_GetNum();
//		if (KeyNum == 1)
//		{
//			InnerTarget += 10;
//		}
//		if (KeyNum == 2)
//		{
//			InnerTarget -= 10;
//		}
//		if (KeyNum == 3)
//		{
//			InnerTarget = 0;
//		}
		
//		InnerKp = RP_GetValue(1) / 4095.0 * 2;
//		InnerKi = RP_GetValue(2) / 4095.0 * 2;
//		InnerKd = RP_GetValue(3) / 4095.0 * 2;
//		InnerTarget = RP_GetValue(4) / 4095.0 * 300 - 150;
//		
//		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", InnerKp);
//		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", InnerKi);
//		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", InnerKd);
//		
//		OLED_Printf(64, 16, OLED_8X16, "Tar:%+04.0f", InnerTarget);
//		OLED_Printf(64, 32, OLED_8X16, "Act:%+04.0f", InnerActual);
//		OLED_Printf(64, 48, OLED_8X16, "Out:%+04.0f", InnerOut);
//		
//		OLED_Update();
//		
//		Serial_Printf("%f,%f,%f\r\n", InnerTarget, InnerActual, InnerOut);
		
		
//		OuterKp = RP_GetValue(1) / 4095.0 * 2;
//		OuterKi = RP_GetValue(2) / 4095.0 * 2;
//		OuterKd = RP_GetValue(3) / 4095.0 * 2;
		OuterTarget = RP_GetValue(4) / 4095.0 * 816 - 408;
		
		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", OuterKp);
		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", OuterKi);
		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", OuterKd);
		
		OLED_Printf(64, 16, OLED_8X16, "Tar:%+04.0f", OuterTarget);
		OLED_Printf(64, 32, OLED_8X16, "Act:%+04.0f", OuterActual);
		OLED_Printf(64, 48, OLED_8X16, "Out:%+04.0f", OuterOut);
		
		OLED_Update();
		
		Serial_Printf("%f,%f,%f\r\n", OuterTarget, OuterActual, OuterOut);
	}
}

void TIM1_UP_IRQHandler(void)
{
	static uint16_t Count1, Count2;
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		Key_Tick();
		
		Count1 ++;
		if (Count1 >= 40)
		{
			Count1 = 0;
			
			Speed = Encoder_Get();
			Location += Speed;
			
			InnerActual = Speed;
			
			InnerError1 = InnerError0;
			InnerError0 = InnerTarget - InnerActual;
			
			if (InnerKi != 0)
			{
				InnerErrorInt += InnerError0;
			}
			else
			{
				InnerErrorInt = 0;
			}
			
			InnerOut = InnerKp * InnerError0 + InnerKi * InnerErrorInt + InnerKd * (InnerError0 - InnerError1);
			
			if (InnerOut > 100) {InnerOut = 100;}
			if (InnerOut < -100) {InnerOut = -100;}
			
			Motor_SetPWM(InnerOut);
		}
		
		Count2 ++;
		if (Count2 >= 40)
		{
			Count2 = 0;
			
			OuterActual = Location;
			
			OuterError1 = OuterError0;
			OuterError0 = OuterTarget - OuterActual;
			
			if (OuterKi != 0)
			{
				OuterErrorInt += OuterError0;
			}
			else
			{
				OuterErrorInt = 0;
			}
			
			OuterOut = OuterKp * OuterError0 + OuterKi * OuterErrorInt + OuterKd * (OuterError0 - OuterError1);
			
			if (OuterOut > 20) {OuterOut = 20;}
			if (OuterOut < -20) {OuterOut = -20;}
			
			InnerTarget = OuterOut;
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
