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
#include "PID.h"

uint8_t KeyNum;

int16_t Speed, Location;

PID_t Inner = {
	.Kp = 0.3,
	.Ki = 0.3,
	.Kd = 0,
	.OutMax = 100,
	.OutMin = -100,
};

PID_t Outer = {
	.Kp = 0.3,
	.Ki = 0,
	.Kd = 0.4,
	.OutMax = 20,
	.OutMin = -20,
};

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
//			Inner.Target += 10;
//		}
//		if (KeyNum == 2)
//		{
//			Inner.Target -= 10;
//		}
//		if (KeyNum == 3)
//		{
//			Inner.Target = 0;
//		}
		
//		Inner.Kp = RP_GetValue(1) / 4095.0 * 2;
//		Inner.Ki = RP_GetValue(2) / 4095.0 * 2;
//		Inner.Kd = RP_GetValue(3) / 4095.0 * 2;
//		Inner.Target = RP_GetValue(4) / 4095.0 * 300 - 150;
//		
//		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", Inner.Kp);
//		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", Inner.Ki);
//		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", Inner.Kd);
//		
//		OLED_Printf(64, 16, OLED_8X16, "Tar:%+04.0f", Inner.Target);
//		OLED_Printf(64, 32, OLED_8X16, "Act:%+04.0f", Inner.Actual);
//		OLED_Printf(64, 48, OLED_8X16, "Out:%+04.0f", Inner.Out);
//		
//		OLED_Update();
//		
//		Serial_Printf("%f,%f,%f\r\n", Inner.Target, Inner.Actual, Inner.Out);
		
		
//		Outer.Kp = RP_GetValue(1) / 4095.0 * 2;
//		Outer.Ki = RP_GetValue(2) / 4095.0 * 2;
//		Outer.Kd = RP_GetValue(3) / 4095.0 * 2;
		Outer.Target = RP_GetValue(4) / 4095.0 * 816 - 408;
		
		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", Outer.Kp);
		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", Outer.Ki);
		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", Outer.Kd);
		
		OLED_Printf(64, 16, OLED_8X16, "Tar:%+04.0f", Outer.Target);
		OLED_Printf(64, 32, OLED_8X16, "Act:%+04.0f", Outer.Actual);
		OLED_Printf(64, 48, OLED_8X16, "Out:%+04.0f", Outer.Out);
		
		OLED_Update();
		
		Serial_Printf("%f,%f,%f\r\n", Outer.Target, Outer.Actual, Outer.Out);
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
			
			Inner.Actual = Speed;
			
			PID_Update(&Inner);
			
			Motor_SetPWM(Inner.Out);
		}
		
		Count2 ++;
		if (Count2 >= 40)
		{
			Count2 = 0;
			
			Outer.Actual = Location;
			
			PID_Update(&Outer);
			
			Inner.Target = Outer.Out;
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
