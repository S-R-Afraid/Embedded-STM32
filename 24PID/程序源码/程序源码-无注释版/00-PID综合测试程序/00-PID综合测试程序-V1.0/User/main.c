#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Key.h"
#include "AD.h"
#include "RP.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include "Store.h"
#include "Timer.h"
#include "Mode1.h"
#include "Mode2.h"
#include "Mode3.h"

uint8_t CurrMode, NextMode;

int main(void)
{
	OLED_Init();
	LED_Init();
	Key_Init();
	AD_Init();
	RP_Init();
	Motor_Init();
	Encoder_Init();
	Serial_Init();
	Timer_Init();
	if (Store_Init())
	{
		Mode21_SaveParam();
		Mode22_SaveParam();
		Mode31_SaveParam();
		
		OLED_Clear();
		OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
		OLED_ShowString(0, 16, "   已重置参数   ", OLED_8X16);
		OLED_ShowString(0, 32, "    Kp Ki Kd    ", OLED_8X16);
		OLED_ShowString(0, 48, "             K4>", OLED_8X16);
		OLED_Update();
		
		Delay_ms(1000);
	}
	
	OLED_Clear();
	OLED_ShowString(0, 0,  "   [江协科技]   ", OLED_8X16);
	OLED_ShowString(4, 16, "PID综合测试程序", OLED_8X16);
	OLED_ShowString(0, 32, "      V1.0      ", OLED_8X16);
	OLED_ShowString(0, 48, "             K4>", OLED_8X16);
	OLED_Update();
	
	while (Key_Check(4, KEY_CLICK) == 0);
	Key_Clear();
	
	OLED_Clear();
	OLED_ShowString(0, 0,  "   请选择系统   ", OLED_8X16);
	OLED_ShowString(0, 16, " K1：硬件测试   ", OLED_8X16);
	OLED_ShowString(0, 32, " K2：电机控制   ", OLED_8X16);
	OLED_ShowString(0, 48, " K3：倒立摆     ", OLED_8X16);
	OLED_Update();
	
	while (1)
	{
		if (Key_Check(1, KEY_CLICK))
		{
			NextMode = 0x11;
			break;
		}
		if (Key_Check(2, KEY_CLICK))
		{
			NextMode = 0x20;
			break;
		}
		if (Key_Check(3, KEY_CLICK))
		{
			NextMode = 0x30;
			break;
		}
		if (Key_Check(4, KEY_LONG))
		{
			Mode21_SaveParam();
			Mode22_SaveParam();
			Mode31_SaveParam();
			
			OLED_Clear();
			OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
			OLED_ShowString(0, 16, "   已重置参数   ", OLED_8X16);
			OLED_ShowString(0, 32, "    Kp Ki Kd    ", OLED_8X16);
			OLED_ShowString(0, 48, "             K4>", OLED_8X16);
			OLED_Update();
			
			while (Key_Check(4, KEY_CLICK) == 0);
			Key_Clear();
				
			OLED_Clear();
			OLED_ShowString(0, 0,  "   请选择系统   ", OLED_8X16);
			OLED_ShowString(0, 16, " K1：硬件测试   ", OLED_8X16);
			OLED_ShowString(0, 32, " K2：电机控制   ", OLED_8X16);
			OLED_ShowString(0, 48, " K3：倒立摆     ", OLED_8X16);
			OLED_Update();
		}
	}
	Key_Clear();
	
	while (1)
	{
		if (CurrMode == NextMode)
		{
			switch (CurrMode)
			{
				case 0x11: Mode11_Loop(); break;
				case 0x12: Mode12_Loop(); break;
				case 0x13: Mode13_Loop(); break;
				case 0x14: Mode14_Loop(); break;
				case 0x15: Mode15_Loop(); break;
				case 0x16: Mode16_Loop(); break;
				case 0x21: Mode21_Loop(); break;
				case 0x22: Mode22_Loop(); break;
				case 0x31: Mode31_Loop(); break;
			}
		}
		else
		{
			switch (CurrMode)
			{
				case 0x13: Mode13_Exit(); break;
			}
			CurrMode = NextMode;
			Key_Clear();
			switch (NextMode)
			{
				case 0x11: Mode11_Init(); break;
				case 0x12: Mode12_Init(); break;
				case 0x13: Mode13_Init(); break;
				case 0x14: Mode14_Init(); break;
				case 0x15: Mode15_Init(); break;
				case 0x16: Mode16_Init(); break;
				case 0x20: Mode20_Init(); break;
				case 0x21: Mode21_Init(); break;
				case 0x22: Mode22_Init(); break;
				case 0x30: Mode30_Init(); break;
				case 0x31: Mode31_Init(); break;
			}
		}
	}
}

void TIM1_UP_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		Key_Tick();
		
		if (CurrMode == NextMode)
		{
			switch (CurrMode)
			{
				case 0x21: Mode21_Tick(); break;
				case 0x22: Mode22_Tick(); break;
				case 0x31: Mode31_Tick(); break;
			}
		}
		if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
		{
			while (1)
			{
				OLED_ShowString(0, 0, "TIM ERROR", OLED_8X16);
				OLED_Update();
			}
		}
	}
}

