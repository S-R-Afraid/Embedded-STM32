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
#include <stdlib.h>

uint8_t KeyNum;

float Target, Actual, Out;
float Kp, Ki, Kd;
float Error0, Error1, ErrorInt;
float DifOut;

int main(void)
{
	OLED_Init();
	Key_Init();
	Motor_Init();
	Encoder_Init();
	RP_Init();
	Serial_Init();
	
	Timer_Init();
	
	OLED_Printf(0, 0, OLED_8X16, "Location Control");
	OLED_Update();
	
	while (1)
	{
//		KeyNum = Key_GetNum();
//		if (KeyNum == 1)
//		{
//			Target += 10;
//		}
//		if (KeyNum == 2)
//		{
//			Target -= 10;
//		}
//		if (KeyNum == 3)
//		{
//			Target = 0;
//		}
		
		Kp = RP_GetValue(1) / 4095.0 * 2;
		Ki = RP_GetValue(2) / 4095.0 * 2;
		Kd = RP_GetValue(3) / 4095.0 * 2;
		Target = RP_GetValue(4) / 4095.0 * 816 - 408;
		
		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", Kp);
		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", Ki);
		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", Kd);
		
		OLED_Printf(64, 16, OLED_8X16, "Tar:%+04.0f", Target);
		OLED_Printf(64, 32, OLED_8X16, "Act:%+04.0f", Actual);
		OLED_Printf(64, 48, OLED_8X16, "Out:%+04.0f", Out);
		
		OLED_Update();
		
		Serial_Printf("%f,%f,%f,%f\r\n", Target, Actual, Out, DifOut);
	}
}

void TIM1_UP_IRQHandler(void)
{
	static uint16_t Count;
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		Key_Tick();
		
		Count ++;
		if (Count >= 40)
		{
			Count = 0;
			
			Actual += Encoder_Get();
			Actual += rand() % 41 - 20;
			
			Error1 = Error0;
			Error0 = Target - Actual;
			
			if (Ki != 0)
			{
				ErrorInt += Error0;
			}
			else
			{
				ErrorInt = 0;
			}
			
//			DifOut = Kd * (Error0 - Error1);
			
			float a = 0.9;
			DifOut = (1 - a) * Kd * (Error0 - Error1) + a * DifOut;
			
			Out = Kp * Error0 + Ki * ErrorInt + DifOut;
			
			if (Out > 100) {Out = 100;}
			if (Out < -100) {Out = -100;}
			
			Motor_SetPWM(Out);
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
