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
#include "AD.h"
#include "PID.h"

#define CENTER_ANGLE		2010
#define CENTER_RANGE		500

uint8_t KeyNum;
uint8_t RunState;

uint16_t Angle;
int16_t Speed, Location;

PID_t AnglePID = {
	.Target = CENTER_ANGLE,
	
	.Kp = 0.3,
	.Ki = 0.01,
	.Kd = 0.4,
	
	.OutMax = 100,
	.OutMin = -100,
};

PID_t LocationPID = {
	.Target = 0,
	
	.Kp = 0.4,
	.Ki = 0,
	.Kd = 4,
	
	.OutMax = 100,
	.OutMin = -100,
};

int main(void)
{
	OLED_Init();
	LED_Init();
	Key_Init();
	RP_Init();
	Motor_Init();
	Encoder_Init();
	Serial_Init();
	AD_Init();
	
	Timer_Init();
	
	while (1)
	{
		KeyNum = Key_GetNum();
		if (KeyNum == 1)
		{
			RunState = !RunState;
		}
		if (KeyNum == 2)
		{
			LocationPID.Target += 408;
			if (LocationPID.Target > 4080)
			{
				LocationPID.Target = 4080;
			}
		}
		if (KeyNum == 3)
		{
			LocationPID.Target -= 408;
			if (LocationPID.Target < -4080)
			{
				LocationPID.Target = -4080;
			}
		}
		
		if (RunState)
		{
			LED_ON();
		}
		else
		{
			LED_OFF();
		}
		
//		AnglePID.Kp = RP_GetValue(1) / 4095.0 * 1;
//		AnglePID.Ki = RP_GetValue(2) / 4095.0 * 1;
//		AnglePID.Kd = RP_GetValue(3) / 4095.0 * 1;
		
//		LocationPID.Kp = RP_GetValue(1) / 4095.0 * 1;
//		LocationPID.Ki = RP_GetValue(2) / 4095.0 * 1;
//		LocationPID.Kd = RP_GetValue(3) / 4095.0 * 9;
		
		OLED_Printf(0, 0, OLED_6X8, "Angle");
		OLED_Printf(0, 12, OLED_6X8, "Kp:%05.3f", AnglePID.Kp);
		OLED_Printf(0, 20, OLED_6X8, "Ki:%05.3f", AnglePID.Ki);
		OLED_Printf(0, 28, OLED_6X8, "Kd:%05.3f", AnglePID.Kd);
		OLED_Printf(0, 40, OLED_6X8, "Tar:%04.0f", AnglePID.Target);
		OLED_Printf(0, 48, OLED_6X8, "Act:%04d", Angle);
		OLED_Printf(0, 56, OLED_6X8, "Out:%+04.0f", AnglePID.Out);
		
		OLED_Printf(64, 0, OLED_6X8, "Location");
		OLED_Printf(64, 12, OLED_6X8, "Kp:%05.3f", LocationPID.Kp);
		OLED_Printf(64, 20, OLED_6X8, "Ki:%05.3f", LocationPID.Ki);
		OLED_Printf(64, 28, OLED_6X8, "Kd:%05.3f", LocationPID.Kd);
		OLED_Printf(64, 40, OLED_6X8, "Tar:%+05.0f", LocationPID.Target);
		OLED_Printf(64, 48, OLED_6X8, "Act:%+05d", Location);
		OLED_Printf(64, 56, OLED_6X8, "Out:%+04.0f", LocationPID.Out);
		
		OLED_Update();
	}
}

void TIM1_UP_IRQHandler(void)
{
	static uint16_t Count1, Count2;
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		Key_Tick();
		
		Angle = AD_GetValue();
		Speed = Encoder_Get();
		Location += Speed;
		
		if (! (Angle > CENTER_ANGLE - CENTER_RANGE
			&& Angle < CENTER_ANGLE + CENTER_RANGE))
		{
			RunState = 0;
		}
		
		if (RunState)
		{
			Count1 ++;
			if (Count1 >= 5)
			{
				Count1 = 0;
				
				AnglePID.Actual = Angle;
				PID_Update(&AnglePID);
				Motor_SetPWM(AnglePID.Out);
			}
			
			Count2 ++;
			if (Count2 >= 50)
			{
				Count2 = 0;
				
				LocationPID.Actual = Location;
				PID_Update(&LocationPID);
				AnglePID.Target = CENTER_ANGLE - LocationPID.Out;
			}
		}
		else
		{
			Motor_SetPWM(0);
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
