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
	/*选择系统界面初始化*/
	OLED_Clear();
	OLED_ShowString(0, 0,  "   请选择系统   ", OLED_8X16);
	OLED_ShowString(0, 16, " K1：硬件测试   ", OLED_8X16);
	OLED_ShowString(0, 32, " K2：电机控制   ", OLED_8X16);
	OLED_ShowString(0, 48, " K3：倒立摆     ", OLED_8X16);
	OLED_Update();
}

void Mode01_Loop(void)
{
	if (Key_Check(1, KEY_CLICK))	//K1按下，切换到模式0x11（硬件测试）
	{
		NextMode = 0x11;
	}
	if (Key_Check(2, KEY_CLICK))	//K2按下，切换到模式0x20（电机控制的选择界面）
	{
		NextMode = 0x20;
	}
	if (Key_Check(3, KEY_CLICK))	//K3按下，切换到模式0x30（倒立摆）
	{
		NextMode = 0x30;
	}
	if (Key_Check(4, KEY_LONG))		//K4长按，重置参数
	{
		/*调用保存参数的函数，初始情况下保存的参数是程序里指定的默认参数*/
		Mode21_SaveParam();
		Mode22_SaveParam();
		Mode31_SaveParam();
		
		/*OLED提示已重置参数*/
		OLED_Clear();
		OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
		OLED_ShowString(0, 16, "   已重置参数   ", OLED_8X16);
		OLED_ShowString(0, 32, "                ", OLED_8X16);
		OLED_ShowString(0, 48, "             K4>", OLED_8X16);
		OLED_Update();
		
		/*等待K4按键按下*/
		while (Key_Check(4, KEY_CLICK) == 0);
		Key_Clear();
		
		/*重新初始化模式0x01，显示选择系统界面*/
		Mode01_Init();
	}
}
