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
#include "Mode0.h"
#include "Mode1.h"
#include "Mode2.h"
#include "Mode3.h"

extern uint8_t CurrMode, NextMode;

void Mode01_Init(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0,  "   请选择系统   ", OLED_8X16);
	OLED_ShowString(0, 16, " K1：硬件测试   ", OLED_8X16);
	OLED_ShowString(0, 32, " K2：电机控制   ", OLED_8X16);
	OLED_ShowString(0, 48, " K3：倒立摆     ", OLED_8X16);
	OLED_Update();
}

void Mode01_Loop(void)
{
	if (Key_Check(1, KEY_CLICK))
	{
		NextMode = 0x11;
	}
	if (Key_Check(2, KEY_CLICK))
	{
		NextMode = 0x20;
	}
	if (Key_Check(3, KEY_CLICK))
	{
		NextMode = 0x30;
	}
	if (Key_Check(4, KEY_LONG))
	{
		Mode21_SaveParam();
		Mode22_SaveParam();
		Mode31_SaveParam();
		
		OLED_Clear();
		OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
		OLED_ShowString(0, 16, "   已重置参数   ", OLED_8X16);
		OLED_ShowString(0, 32, "                ", OLED_8X16);
		OLED_ShowString(0, 48, "             K4>", OLED_8X16);
		OLED_Update();
		
		while (Key_Check(4, KEY_CLICK) == 0);
		Key_Clear();
			
		Mode01_Init();
	}
}
