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
#include "Timer.h"

extern uint8_t CurrMode, NextMode;

void Mode11_Init(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0,  "    按键测试    ", OLED_8X16);
	OLED_ShowString(0, 16, "K1 K2 K3        ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "1/6          K4>", OLED_8X16);
	OLED_Update();
}

void Mode11_Loop(void)
{
	if (Key_Check(1, KEY_CLICK))
	{
		OLED_ReverseArea(0, 16, 16, 16);
		OLED_Update();
		Delay_ms(100);
		OLED_ReverseArea(0, 16, 16, 16);
		OLED_Update();
	}
	if (Key_Check(2, KEY_CLICK))
	{
		OLED_ReverseArea(24, 16, 16, 16);
		OLED_Update();
		Delay_ms(100);
		OLED_ReverseArea(24, 16, 16, 16);
		OLED_Update();
	}
	if (Key_Check(3, KEY_CLICK))
	{
		OLED_ReverseArea(48, 16, 16, 16);
		OLED_Update();
		Delay_ms(100);
		OLED_ReverseArea(48, 16, 16, 16);
		OLED_Update();
	}
	if (Key_Check(4, KEY_CLICK))
	{
		NextMode = 0x12;
	}
}

void Mode12_Init(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0,  "   电位器测试   ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "2/6          K4>", OLED_8X16);
	OLED_Update();
}

void Mode12_Loop(void)
{
	if (Key_Check(4, KEY_CLICK))
	{
		NextMode = 0x13;
	}
	
	OLED_Printf(0, 16, OLED_8X16, "1:%04d  2:%04d", RP_GetValue(1), RP_GetValue(2));
	OLED_Printf(0, 32, OLED_8X16, "3:%04d  4:%04d", RP_GetValue(3), RP_GetValue(4));
	OLED_Update();
}

void Mode13_Init(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0,  "    电机测试    ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "3/6          K4>", OLED_8X16);
	OLED_Update();
}

int16_t PWM_Duty;

void Mode13_Loop(void)
{
	if (Key_Check(1, KEY_CLICK))
	{
		PWM_Duty += 10;
	}
	if (Key_Check(2, KEY_CLICK))
	{
		PWM_Duty -= 10;
	}
	if (Key_Check(3, KEY_CLICK))
	{
		PWM_Duty = 0;
	}
	if (Key_Check(4, KEY_CLICK))
	{
		NextMode = 0x14;
	}
	
	Motor_SetPWM(1, PWM_Duty);
	Motor_SetPWM(2, PWM_Duty);
	
	OLED_Printf(0, 16, OLED_8X16, "MA:%+04d", PWM_Duty);
	OLED_Printf(0, 32, OLED_8X16, "MB:%+04d", PWM_Duty);
	OLED_Update();
}

void Mode13_Exit(void)
{
	PWM_Duty = 0;
	Motor_SetPWM(1, 0);
	Motor_SetPWM(2, 0);
}

void Mode14_Init(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0,  "   编码器测试   ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "4/6          K4>", OLED_8X16);
	OLED_Update();
	
	Encoder_SetLocation(1, 0);
	Encoder_SetLocation(2, 0);
}

void Mode14_Loop(void)
{
	if (Key_Check(4, KEY_CLICK))
	{
		NextMode = 0x15;
	}
	
	OLED_Printf(0, 16, OLED_8X16, "EA:%+06d", Encoder_GetLocation(1));
	OLED_Printf(0, 32, OLED_8X16, "EB:%+06d", Encoder_GetLocation(2));
	OLED_Update();
}

void Mode15_Init(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0,  " 角度传感器测试 ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "5/6          K4>", OLED_8X16);
	OLED_Update();
}

void Mode15_Loop(void)
{
	if (Key_Check(4, KEY_CLICK))
	{
		NextMode = 0x16;
	}
	
	OLED_Printf(0, 16, OLED_8X16, "AD1:%04d", AD_GetValue(1));
	OLED_Printf(0, 32, OLED_8X16, "AD2:%04d", AD_GetValue(2));
	OLED_Update();
}

void Mode16_Init(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0,  "    串口测试    ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "6/6          K4>", OLED_8X16);
	OLED_Update();
}

void Mode16_Loop(void)
{
	static uint8_t TxData, RxData;
	
	if (Key_Check(1, KEY_CLICK))
	{
		TxData += 1;
	}
	if (Key_Check(2, KEY_CLICK))
	{
		TxData -= 1;
	}
	if (Key_Check(3, KEY_CLICK))
	{
		OLED_ReverseArea(56, 16, 16, 16);
		OLED_Update();
		Delay_ms(100);
		OLED_ReverseArea(56, 16, 16, 16);
		OLED_Update();
		
		Serial_SendByte(TxData);
	}
	if (Key_Check(4, KEY_CLICK))
	{
		NextMode = 0x11;
	}
	
	if (Serial_GetRxFlag())
	{
		RxData = Serial_GetRxData();
		
		OLED_Printf(0, 32, OLED_8X16, "RxData:%02X", RxData);
		
		OLED_ReverseArea(56, 32, 16, 16);
		OLED_Update();
		Delay_ms(100);
		OLED_ReverseArea(56, 32, 16, 16);
		OLED_Update();
	}
	
	OLED_Printf(0, 16, OLED_8X16, "TxData:%02X", TxData);
	OLED_Printf(0, 32, OLED_8X16, "RxData:%02X", RxData);
	OLED_Update();
}

