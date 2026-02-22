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
#include "PID.h"
#include "math.h"
#include "Store.h"

extern uint8_t CurrMode, NextMode;

PID_t SpeedPID = {
	.Kp = 0.3,
	.Ki = 0.3,
	.Kd = 0,
	.OutMax = 100,
	.OutMin = -100,
};

PID_t LocationPID = {
	.Kp = 0.4,
	.Ki = 0.1,
	.Kd = 0.2,
	.OutMax = 100,
	.OutMin = -100,
	.ErrIntThreshold = 40,
};

void Mode21_SaveParam(void)
{
	Store_Data[1] = *((uint16_t *)&SpeedPID.Kp);
	Store_Data[2] = *((uint16_t *)&SpeedPID.Kp + 1);
	Store_Data[3] = *((uint16_t *)&SpeedPID.Ki);
	Store_Data[4] = *((uint16_t *)&SpeedPID.Ki + 1);
	Store_Data[5] = *((uint16_t *)&SpeedPID.Kd);
	Store_Data[6] = *((uint16_t *)&SpeedPID.Kd + 1);
	Store_Save();
}

void Mode21_LoadParam(void)
{
	uint32_t Temp;
	Temp = (Store_Data[2] << 16) | Store_Data[1];
	SpeedPID.Kp = *(float *)&Temp;
	Temp = (Store_Data[4] << 16) | Store_Data[3];
	SpeedPID.Ki = *(float *)&Temp;
	Temp = (Store_Data[6] << 16) | Store_Data[5];
	SpeedPID.Kd = *(float *)&Temp;
}

void Mode22_SaveParam(void)
{
	Store_Data[11] = *((uint16_t *)&LocationPID.Kp);
	Store_Data[12] = *((uint16_t *)&LocationPID.Kp + 1);
	Store_Data[13] = *((uint16_t *)&LocationPID.Ki);
	Store_Data[14] = *((uint16_t *)&LocationPID.Ki + 1);
	Store_Data[15] = *((uint16_t *)&LocationPID.Kd);
	Store_Data[16] = *((uint16_t *)&LocationPID.Kd + 1);
	Store_Save();
}

void Mode22_LoadParam(void)
{
	uint32_t Temp;
	Temp = (Store_Data[12] << 16) | Store_Data[11];
	LocationPID.Kp = *(float *)&Temp;
	Temp = (Store_Data[14] << 16) | Store_Data[13];
	LocationPID.Ki = *(float *)&Temp;
	Temp = (Store_Data[16] << 16) | Store_Data[15];
	LocationPID.Kd = *(float *)&Temp;
}

void Mode20_Init(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
	OLED_ShowString(0, 16, "  请将设备置于  ", OLED_8X16);
	OLED_ShowString(0, 32, "  电机控制状态  ", OLED_8X16);
	OLED_ShowString(0, 48, "             K4>", OLED_8X16);
	OLED_Update();
	
	while (Key_Check(4, KEY_CLICK) == 0);
	Key_Clear();
	
	OLED_Clear();
	OLED_ShowString(0, 0,  "    电机控制    ", OLED_8X16);
	OLED_ShowString(0, 16, " K1：定速控制   ", OLED_8X16);
	OLED_ShowString(0, 32, " K2：定位置控制 ", OLED_8X16);
	OLED_ShowString(0, 48, "                ", OLED_8X16);
	OLED_Update();
	
	while (1)
	{
		if (Key_Check(1, KEY_CLICK))
		{
			NextMode = 0x21;
			break;
		}
		if (Key_Check(2, KEY_CLICK))
		{
			NextMode = 0x22;
			break;
		}
	}
	Key_Clear();
	
	Mode21_LoadParam();
	Mode22_LoadParam();
}

void Mode21_Init(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0,  "    定速控制    ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "                ", OLED_8X16);
	OLED_Update();
	
}

void Mode21_Loop(void)
{
	static uint8_t RPFlag, KpFlag, KiFlag, KdFlag, TargetFlag;
	float RP_Kp, RP_Ki, RP_Kd, RP_Target;
	
	if (Key_Check(1, KEY_CLICK))
	{
		TargetFlag = 0;
		SpeedPID.Target += 20;
		if (SpeedPID.Target > 100)
		{
			SpeedPID.Target = 100;
		}
	}
	if (Key_Check(2, KEY_CLICK))
	{
		TargetFlag = 0;
		SpeedPID.Target -= 20;
		if (SpeedPID.Target < -100)
		{
			SpeedPID.Target = -100;
		}
	}
	if (Key_Check(3, KEY_CLICK))
	{
		TargetFlag = 0;
		SpeedPID.Target = 0;
	}
	if (Key_Check(4, KEY_CLICK))
	{
		RPFlag = !RPFlag;
		KpFlag = 0;
		KiFlag = 0;
		KdFlag = 0;
		TargetFlag = 0;
	}
	if (Key_Check(4, KEY_LONG))
	{
		Mode21_SaveParam();
		
		OLED_Clear();
		OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
		OLED_ShowString(0, 16, "   已保存参数   ", OLED_8X16);
		OLED_ShowString(0, 32, "                ", OLED_8X16);
		OLED_ShowString(0, 48, "             K4>", OLED_8X16);
		OLED_Update();
		
		while (Key_Check(4, KEY_CLICK) == 0);
		Key_Clear();
		
		OLED_Clear();
		OLED_ShowString(0, 0,  "    定速控制    ", OLED_8X16);
		OLED_ShowString(0, 16, "                ", OLED_8X16);
		OLED_ShowString(0, 32, "                ", OLED_8X16);
		OLED_ShowString(0, 48, "                ", OLED_8X16);
		OLED_Update();
	}
	
	if (RPFlag)
	{
		RP_Kp = RP_GetValue(1) / 4095.0 * 3;
		RP_Ki = RP_GetValue(2) / 4095.0 * 3;
		RP_Kd = RP_GetValue(3) / 4095.0 * 3;
		RP_Target = RP_GetValue(4) / 4095.0 * 200 - 100;
		
		if (fabs(RP_Kp - SpeedPID.Kp) < 0.05)
		{
			KpFlag = 1;
		}
		if (fabs(RP_Ki - SpeedPID.Ki) < 0.05)
		{
			KiFlag = 1;
		}
		if (fabs(RP_Kd - SpeedPID.Kd) < 0.05)
		{
			KdFlag = 1;
		}
		if (fabs(RP_Target - SpeedPID.Target) < 5)
		{
			TargetFlag = 1;
		}
		
		if (KpFlag)
		{
			SpeedPID.Kp = RP_Kp;
		}
		if (KiFlag)
		{
			SpeedPID.Ki = RP_Ki;
		}
		if (KdFlag)
		{
			SpeedPID.Kd = RP_Kd;
		}
		if (TargetFlag)
		{
			SpeedPID.Target = RP_Target;
		}
	}
	
	OLED_Printf(0, 16, OLED_8X16, "Kp:%04.2f", SpeedPID.Kp);
	OLED_Printf(0, 32, OLED_8X16, "Ki:%04.2f", SpeedPID.Ki);
	OLED_Printf(0, 48, OLED_8X16, "Kd:%04.2f", SpeedPID.Kd);
	OLED_Printf(64, 16, OLED_8X16, "Tar:%+04d", (int16_t)SpeedPID.Target);
	OLED_Printf(64, 32, OLED_8X16, "Act:%+04d", (int16_t)SpeedPID.Actual);
	OLED_Printf(64, 48, OLED_8X16, "Out:%+04d", (int16_t)SpeedPID.Out);
	
	if (RPFlag)
	{
		if (KpFlag)
		{
			OLED_ReverseArea(24, 17, 32, 15);
		}
		else
		{
			OLED_DrawRectangle(24, 17, 32, 15, OLED_UNFILLED);
		}
		if (KiFlag)
		{
			OLED_ReverseArea(24, 33, 32, 15);
		}
		else
		{
			OLED_DrawRectangle(24, 33, 32, 15, OLED_UNFILLED);
		}
		if (KdFlag)
		{
			OLED_ReverseArea(24, 49, 32, 15);
		}
		else
		{
			OLED_DrawRectangle(24, 49, 32, 15, OLED_UNFILLED);
		}
		if (TargetFlag)
		{
			OLED_ReverseArea(96, 17, 32, 15);
		}
		else
		{
			OLED_DrawRectangle(96, 17, 32, 15, OLED_UNFILLED);
		}
	}
	
	OLED_Update();
}

void Mode21_Tick(void)
{
	static uint16_t Count;
	static int16_t PrevLocation, CurrLocation;
	Count ++;
	if (Count >= 40)
	{
		Count = 0;
		
		PrevLocation = CurrLocation;
		CurrLocation = Encoder_GetLocation(1);
		
		SpeedPID.Actual = (int16_t)((uint16_t)CurrLocation - (uint16_t)PrevLocation);
		PID_Update(&SpeedPID);
		Motor_SetPWM(1, SpeedPID.Out);
	}
}

void Mode22_Init(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0,  "   定位置控制   ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "                ", OLED_8X16);
	OLED_Update();
}

void Mode22_Loop(void)
{
	static uint8_t RPFlag, KpFlag, KiFlag, KdFlag, TargetFlag;
	float RP_Kp, RP_Ki, RP_Kd, RP_Target;
	
	if (Key_Check(1, KEY_CLICK))
	{
		TargetFlag = 0;
		LocationPID.Target += 408;
		if (LocationPID.Target > 816)
		{
			LocationPID.Target = 816;
		}
	}
	if (Key_Check(2, KEY_CLICK))
	{
		TargetFlag = 0;
		LocationPID.Target -= 408;
		if (LocationPID.Target < -816)
		{
			LocationPID.Target = -816;
		}
	}
	if (Key_Check(3, KEY_CLICK))
	{
		TargetFlag = 0;
		LocationPID.Target = 0;
	}
	if (Key_Check(4, KEY_CLICK))
	{
		RPFlag = !RPFlag;
		KpFlag = 0;
		KiFlag = 0;
		KdFlag = 0;
		TargetFlag = 0;
	}
	if (Key_Check(4, KEY_LONG))
	{
		Mode22_SaveParam();
		
		OLED_Clear();
		OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
		OLED_ShowString(0, 16, "   已保存参数   ", OLED_8X16);
		OLED_ShowString(0, 32, "                ", OLED_8X16);
		OLED_ShowString(0, 48, "             K4>", OLED_8X16);
		OLED_Update();
		
		while (Key_Check(4, KEY_CLICK) == 0);
		Key_Clear();
		
		OLED_Clear();
		OLED_ShowString(0, 0,  "   定位置控制   ", OLED_8X16);
		OLED_ShowString(0, 16, "                ", OLED_8X16);
		OLED_ShowString(0, 32, "                ", OLED_8X16);
		OLED_ShowString(0, 48, "                ", OLED_8X16);
		OLED_Update();
	}
	
	if (RPFlag)
	{
		RP_Kp = RP_GetValue(1) / 4095.0 * 3;
		RP_Ki = RP_GetValue(2) / 4095.0 * 3;
		RP_Kd = RP_GetValue(3) / 4095.0 * 3;
		RP_Target = RP_GetValue(4) / 4095.0 * 408 - 204;
		
		if (fabs(RP_Kp - LocationPID.Kp) < 0.05)
		{
			KpFlag = 1;
		}
		if (fabs(RP_Ki - LocationPID.Ki) < 0.05)
		{
			KiFlag = 1;
		}
		if (fabs(RP_Kd - LocationPID.Kd) < 0.05)
		{
			KdFlag = 1;
		}
		if (fabs(RP_Target - LocationPID.Target) < 5)
		{
			TargetFlag = 1;
		}
		
		if (KpFlag)
		{
			LocationPID.Kp = RP_Kp;
		}
		if (KiFlag)
		{
			LocationPID.Ki = RP_Ki;
		}
		if (KdFlag)
		{
			LocationPID.Kd = RP_Kd;
		}
		if (TargetFlag)
		{
			LocationPID.Target = RP_Target;
		}
	}
	
	OLED_Printf(0, 16, OLED_8X16, "Kp:%04.2f", LocationPID.Kp);
	OLED_Printf(0, 32, OLED_8X16, "Ki:%04.2f", LocationPID.Ki);
	OLED_Printf(0, 48, OLED_8X16, "Kd:%04.2f", LocationPID.Kd);
	OLED_Printf(64, 16, OLED_8X16, "Tar:%+04d", (int16_t)LocationPID.Target);
	OLED_Printf(64, 32, OLED_8X16, "Act:%+04d", (int16_t)LocationPID.Actual);
	OLED_Printf(64, 48, OLED_8X16, "Out:%+04d", (int16_t)LocationPID.Out);
	
	if (RPFlag)
	{
		if (KpFlag)
		{
			OLED_ReverseArea(24, 17, 32, 15);
		}
		else
		{
			OLED_DrawRectangle(24, 17, 32, 15, OLED_UNFILLED);
		}
		if (KiFlag)
		{
			OLED_ReverseArea(24, 33, 32, 15);
		}
		else
		{
			OLED_DrawRectangle(24, 33, 32, 15, OLED_UNFILLED);
		}
		if (KdFlag)
		{
			OLED_ReverseArea(24, 49, 32, 15);
		}
		else
		{
			OLED_DrawRectangle(24, 49, 32, 15, OLED_UNFILLED);
		}
		if (TargetFlag)
		{
			OLED_ReverseArea(96, 17, 32, 15);
		}
		else
		{
			OLED_DrawRectangle(96, 17, 32, 15, OLED_UNFILLED);
		}
	}
	
	OLED_Update();
}

void Mode22_Tick(void)
{
	static uint16_t Count;
	Count ++;
	if (Count >= 40)
	{
		Count = 0;
		
		LocationPID.Actual = Encoder_GetLocation(1);
		PID_Update(&LocationPID);
		Motor_SetPWM(1, LocationPID.Out);
	}
}
