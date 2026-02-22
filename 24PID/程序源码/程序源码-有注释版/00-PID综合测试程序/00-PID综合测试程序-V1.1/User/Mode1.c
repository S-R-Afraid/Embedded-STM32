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
	/*按键测试界面初始化*/
	OLED_Clear();
	OLED_ShowString(0, 0,  "    按键测试    ", OLED_8X16);
	OLED_ShowString(0, 16, "K1 K2 K3        ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "1/6          K4>", OLED_8X16);
	OLED_Update();
}

void Mode11_Loop(void)
{
	/*按键测试*/
	if (Key_Check(1, KEY_CLICK))	//K1按下，K1所在位置反色闪烁一次
	{
		OLED_ReverseArea(0, 16, 16, 16);
		OLED_Update();
		Delay_ms(100);
		OLED_ReverseArea(0, 16, 16, 16);
		OLED_Update();
	}
	if (Key_Check(2, KEY_CLICK))	//K2按下，K2所在位置反色闪烁一次
	{
		OLED_ReverseArea(24, 16, 16, 16);
		OLED_Update();
		Delay_ms(100);
		OLED_ReverseArea(24, 16, 16, 16);
		OLED_Update();
	}
	if (Key_Check(3, KEY_CLICK))	//K3按下，K3所在位置反色闪烁一次
	{
		OLED_ReverseArea(48, 16, 16, 16);
		OLED_Update();
		Delay_ms(100);
		OLED_ReverseArea(48, 16, 16, 16);
		OLED_Update();
	}
	if (Key_Check(4, KEY_CLICK))	//K4按下，切换到下一个测试界面
	{
		NextMode = 0x12;
	}
}

void Mode12_Init(void)
{
	/*电位器测试界面初始化*/
	OLED_Clear();
	OLED_ShowString(0, 0,  "   电位器测试   ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "2/6          K4>", OLED_8X16);
	OLED_Update();
}

void Mode12_Loop(void)
{
	if (Key_Check(4, KEY_CLICK))	//K4按下，切换到下一个测试界面
	{
		NextMode = 0x13;
	}
	
	/*循环读取电位器旋钮的AD值并显示在OLED上*/
	OLED_Printf(0, 16, OLED_8X16, "1:%04d  2:%04d", RP_GetValue(1), RP_GetValue(2));
	OLED_Printf(0, 32, OLED_8X16, "3:%04d  4:%04d", RP_GetValue(3), RP_GetValue(4));
	OLED_Update();
}

void Mode13_Init(void)
{
	/*电机测试界面初始化*/
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
	if (Key_Check(1, KEY_CLICK))	//K1按下，PWM_Duty加5
	{
		PWM_Duty += 5;
	}
	if (Key_Check(2, KEY_CLICK))	//K2按下，PWM_Duty减5
	{
		PWM_Duty -= 5;
	}
	if (Key_Check(3, KEY_CLICK))	//K3按下，PWM_Duty归0
	{
		PWM_Duty = 0;
	}
	if (Key_Check(4, KEY_CLICK))	//K4按下，切换到下一个测试界面
	{
		NextMode = 0x14;
	}
	
	Motor_SetPWM(1, PWM_Duty);		//把PWM_Duty变量的值赋值给左侧两个电机接口
	Motor_SetPWM(2, PWM_Duty);		//把PWM_Duty变量的值赋值给右侧两个电机接口，默认未使用
	
	OLED_Printf(0, 16, OLED_8X16, "MA:%+04d", PWM_Duty);	//OLED显示PWM_Duty值
	OLED_Printf(0, 32, OLED_8X16, "MB:%+04d", PWM_Duty);
	OLED_Update();
}

void Mode13_Exit(void)
{
	/*电机测试模式收尾工作*/
	PWM_Duty = 0;					//在此模式退出后，电机自动停止
	Motor_SetPWM(1, 0);
	Motor_SetPWM(2, 0);
}

void Mode14_Init(void)
{
	/*编码器测试界面初始化*/
	OLED_Clear();
	OLED_ShowString(0, 0,  "   编码器测试   ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "4/6          K4>", OLED_8X16);
	OLED_Update();
	
	/*初始位置归0*/
	Encoder_SetLocation(1, 0);
	Encoder_SetLocation(2, 0);
}

void Mode14_Loop(void)
{
	if (Key_Check(4, KEY_CLICK))	//K4按下，切换到下一个测试界面
	{
		NextMode = 0x15;
	}
	
	/*循环读取两个编码器的位置值并显示在OLED上*/
	OLED_Printf(0, 16, OLED_8X16, "EA:%+06d", Encoder_GetLocation(1));	//左侧两个电机接口的编码器
	OLED_Printf(0, 32, OLED_8X16, "EB:%+06d", Encoder_GetLocation(2));	//右侧两个电机接口的编码器，默认未使用
	OLED_Update();
}

void Mode15_Init(void)
{
	/*角度传感器测试界面初始化*/
	OLED_Clear();
	OLED_ShowString(0, 0,  " 角度传感器测试 ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "5/6          K4>", OLED_8X16);
	OLED_Update();
}

void Mode15_Loop(void)
{
	if (Key_Check(4, KEY_CLICK))	//K4按下，切换到下一个测试界面
	{
		NextMode = 0x16;
	}
	
	/*循环读取角度传感器的AD值并显示在OLED上*/
	OLED_Printf(0, 16, OLED_8X16, "AD1:%04d", AD_GetValue(1));
	OLED_Printf(0, 32, OLED_8X16, "AD2:%04d", AD_GetValue(2));
	OLED_Update();
}

void Mode16_Init(void)
{
	/*串口测试界面初始化*/
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
	
	if (Key_Check(1, KEY_CLICK))		//K1按下，TxData加1
	{
		TxData += 1;
	}
	if (Key_Check(2, KEY_CLICK))		//K2按下，TxData减1
	{
		TxData -= 1;
	}
	if (Key_Check(3, KEY_CLICK))		//K3按下，TxData所在的位置反色闪烁一次，同时发送数据
	{
		OLED_ReverseArea(56, 16, 16, 16);	//反色闪烁一次
		OLED_Update();
		Delay_ms(100);
		OLED_ReverseArea(56, 16, 16, 16);
		OLED_Update();
		
		Serial_SendByte(TxData);			//将TxData通过串口发送出去
	}
	if (Key_Check(4, KEY_CLICK))		//K4按下，返回第一个测试界面
	{
		NextMode = 0x11;
	}
	
	/*循环判断是否收到数据*/
	if (Serial_GetRxFlag())					//如果收到数据
	{
		RxData = Serial_GetRxData();		//获取收到的数据
		
		OLED_Printf(0, 32, OLED_8X16, "RxData:%02X", RxData);	//OLED显示此数据
		
		OLED_ReverseArea(56, 32, 16, 16);	//RxData所在的位置反色闪烁一次
		OLED_Update();
		Delay_ms(100);
		OLED_ReverseArea(56, 32, 16, 16);
		OLED_Update();
	}
	
	/*OLED显示TxData和RxData*/
	OLED_Printf(0, 16, OLED_8X16, "TxData:%02X", TxData);
	OLED_Printf(0, 32, OLED_8X16, "RxData:%02X", RxData);
	OLED_Update();
}

