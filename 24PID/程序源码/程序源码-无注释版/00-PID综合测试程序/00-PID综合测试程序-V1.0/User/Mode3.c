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
uint8_t RunState;

#define CENTER_ANGLE		2050		//中心角度值，先按K2再按K3，先慢后快调小，先快后慢调大
										//值大概范围：1900~2100，理想值2048

#define ANGLE_A		1700				//立起来的角度下限，一般不调
#define ANGLE_B		2300				//立起来的角度上限，一般不调


#define START_PWM	40					//启动时的力度，启动不了调大，启动太猛调小
										//值大概范围：35力度小些，40力度大些
										
#define START_TIME	90					//启动时每次旋转的时间，一般不调

#define OFFSET_PWM	5					//电机旋转时，PWM的偏移，一般不调


PID_t AnglePID = {
	.Target = CENTER_ANGLE,
	.Kp = 0.25,
	.Ki = 0.009,
	.Kd = 0.41,
	.OutMax = 100,
	.OutMin = -100,
};

static PID_t LocationPID = {
	.Target = 0,
	.Kp = 0.52,
	.Ki = 0.01,
	.Kd = 4.56,
	.OutMax = 100,							//旋转的速度，值大概范围：50~100
	.OutMin = -100,							//旋转的速度，值大概范围：-50~-100
	.ErrIntThreshold = 100,
};

void Mode30_Init(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
	OLED_ShowString(0, 16, "  请将设备置于  ", OLED_8X16);
	OLED_ShowString(0, 32, "   倒立摆状态   ", OLED_8X16);
	OLED_ShowString(0, 48, "             K4>", OLED_8X16);
	OLED_Update();
	
	while (Key_Check(4, KEY_CLICK) == 0);
	Key_Clear();
	
	NextMode = 0x31;
}	

void Mode31_SaveParam(void)
{
	Store_Data[21] = *((uint16_t *)&AnglePID.Kp);
	Store_Data[22] = *((uint16_t *)&AnglePID.Kp + 1);
	Store_Data[23] = *((uint16_t *)&AnglePID.Ki);
	Store_Data[24] = *((uint16_t *)&AnglePID.Ki + 1);
	Store_Data[25] = *((uint16_t *)&AnglePID.Kd);
	Store_Data[26] = *((uint16_t *)&AnglePID.Kd + 1);

	Store_Data[31] = *((uint16_t *)&LocationPID.Kp);
	Store_Data[32] = *((uint16_t *)&LocationPID.Kp + 1);
	Store_Data[33] = *((uint16_t *)&LocationPID.Ki);
	Store_Data[34] = *((uint16_t *)&LocationPID.Ki + 1);
	Store_Data[35] = *((uint16_t *)&LocationPID.Kd);
	Store_Data[36] = *((uint16_t *)&LocationPID.Kd + 1);
	Store_Save();
}

void Mode31_LoadParam(void)
{
	uint32_t Temp;
	Temp = (Store_Data[22] << 16) | Store_Data[21];
	AnglePID.Kp = *(float *)&Temp;
	Temp = (Store_Data[24] << 16) | Store_Data[23];
	AnglePID.Ki = *(float *)&Temp;
	Temp = (Store_Data[26] << 16) | Store_Data[25];
	AnglePID.Kd = *(float *)&Temp;
	
	Temp = (Store_Data[32] << 16) | Store_Data[31];
	LocationPID.Kp = *(float *)&Temp;
	Temp = (Store_Data[34] << 16) | Store_Data[33];
	LocationPID.Ki = *(float *)&Temp;
	Temp = (Store_Data[36] << 16) | Store_Data[35];
	LocationPID.Kd = *(float *)&Temp;
}

void Mode31_Init(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0,  "                ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "                ", OLED_8X16);
	OLED_Update();
	
	Mode31_LoadParam();
}

void Mode31_Loop(void)
{
	static uint8_t RPFlag;
	static uint8_t KpFlag1, KiFlag1, KdFlag1;
	static uint8_t KpFlag2, KiFlag2, KdFlag2;
	
	float RP_Kp, RP_Ki, RP_Kd;
	
	if (Key_Check(1, KEY_CLICK))
	{
		if (RunState == 0)
		{
			RunState = 13;
		}
		else
		{
			RunState = 0;
		}
	}
	if (Key_Check(2, KEY_CLICK))
	{
		LocationPID.Target += 408;
		if (LocationPID.Target > 8999)
		{
			LocationPID.Target = 8999;
		}
	}
	if (Key_Check(3, KEY_CLICK))
	{
		LocationPID.Target -= 408;
		if (LocationPID.Target < -8999)
		{
			LocationPID.Target = -8999;
		}
	}
	if (Key_Check(4, KEY_CLICK))
	{
		RPFlag ++;
		RPFlag %= 3;
		KpFlag1 = 0;
		KiFlag1 = 0;
		KdFlag1 = 0;
		KpFlag2 = 0;
		KiFlag2 = 0;
		KdFlag2 = 0;
	}
	if (Key_Check(4, KEY_LONG))
	{
		Mode31_SaveParam();
		
		OLED_Clear();
		OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
		OLED_ShowString(0, 16, "   已保存参数   ", OLED_8X16);
		OLED_ShowString(0, 32, "    Kp Ki Kd    ", OLED_8X16);
		OLED_ShowString(0, 48, "             K4>", OLED_8X16);
		OLED_Update();
		
		while (Key_Check(4, KEY_CLICK) == 0);
		Key_Clear();
		
		OLED_Clear();
		OLED_ShowString(0, 0,  "                ", OLED_8X16);
		OLED_ShowString(0, 16, "                ", OLED_8X16);
		OLED_ShowString(0, 32, "                ", OLED_8X16);
		OLED_ShowString(0, 48, "                ", OLED_8X16);
		OLED_Update();
	}
	
	if (RPFlag == 1)
	{
		RP_Kp = RP_GetValue(1) / 4095.0 * 1;
		RP_Ki = RP_GetValue(2) / 4095.0 * 0.5;
		RP_Kd = RP_GetValue(3) / 4095.0 * 1;
		
		if (fabs(RP_Kp - AnglePID.Kp) < 0.03)
		{
			KpFlag1 = 1;
		}
		if (fabs(RP_Ki - AnglePID.Ki) < 0.003)
		{
			KiFlag1 = 1;
		}
		if (fabs(RP_Kd - AnglePID.Kd) < 0.03)
		{
			KdFlag1 = 1;
		}
		
		if (KpFlag1)
		{
			AnglePID.Kp = RP_Kp;
		}
		if (KiFlag1)
		{
			AnglePID.Ki = RP_Ki;
		}
		if (KdFlag1)
		{
			AnglePID.Kd = RP_Kd;
		}
	}
	if (RPFlag == 2)
	{
		RP_Kp = RP_GetValue(1) / 4095.0 * 5;
		RP_Ki = RP_GetValue(2) / 4095.0 * 1;
		RP_Kd = RP_GetValue(3) / 4095.0 * 10;
		
		if (fabs(RP_Kp - LocationPID.Kp) < 0.03)
		{
			KpFlag2 = 1;
		}
		if (fabs(RP_Ki - LocationPID.Ki) < 0.003)
		{
			KiFlag2 = 1;
		}
		if (fabs(RP_Kd - LocationPID.Kd) < 0.03)
		{
			KdFlag2 = 1;
		}
		
		if (KpFlag2)
		{
			LocationPID.Kp = RP_Kp;
		}
		if (KiFlag2)
		{
			LocationPID.Ki = RP_Ki;
		}
		if (KdFlag2)
		{
			LocationPID.Kd = RP_Kd;
		}
	}
	
	if (RunState == 0)
	{
		LED_OFF();
	}
	else
	{
		LED_ON();
	}
	
	OLED_DrawLine(63, 0, 63, 63);
	
	OLED_Printf(0, 0, OLED_6X8, "Angle");
	OLED_Printf(0, 8+4, OLED_6X8, "Kp:%.3f", AnglePID.Kp);
	OLED_Printf(0, 16+4, OLED_6X8, "Ki:%.3f", AnglePID.Ki);
	OLED_Printf(0, 24+4, OLED_6X8, "Kd:%.3f", AnglePID.Kd);
	OLED_Printf(0, 40, OLED_6X8, "Tar:%+05d", (int16_t)AnglePID.Target);
	OLED_Printf(0, 48, OLED_6X8, "Act:%+05d", (int16_t)AnglePID.Actual);
	OLED_Printf(0, 56, OLED_6X8, "Out:%+05d", (int16_t)AnglePID.Out);
	
	OLED_Printf(68, 0, OLED_6X8, "Location");
	OLED_Printf(68, 8+4, OLED_6X8, "Kp:%.3f", LocationPID.Kp);
	OLED_Printf(68, 16+4, OLED_6X8, "Ki:%.3f", LocationPID.Ki);
	OLED_Printf(68, 24+4, OLED_6X8, "Kd:%.3f", LocationPID.Kd);
	OLED_Printf(68, 40, OLED_6X8, "Tar:%+05d", (int16_t)LocationPID.Target);
	OLED_Printf(68, 48, OLED_6X8, "Act:%+05d", (int16_t)LocationPID.Actual);
	OLED_Printf(68, 56, OLED_6X8, "Out:%+05d", (int16_t)LocationPID.Out);	
	
	if (RPFlag == 1)
	{
		if (KpFlag1)
		{
			OLED_ReverseArea(18, 8+4, 30, 7);
		}
		else
		{
			OLED_DrawRectangle(18, 8+4, 30, 7, OLED_UNFILLED);
		}
		if (KiFlag1)
		{
			OLED_ReverseArea(18, 16+4, 30, 7);
		}
		else
		{
			OLED_DrawRectangle(18, 16+4, 30, 7, OLED_UNFILLED);
		}
		if (KdFlag1)
		{
			OLED_ReverseArea(18, 24+4, 30, 7);
		}
		else
		{
			OLED_DrawRectangle(18, 24+4, 30, 7, OLED_UNFILLED);
		}
	}
	if (RPFlag == 2)
	{
		if (KpFlag2)
		{
			OLED_ReverseArea(86, 8+4, 30, 7);
		}
		else
		{
			OLED_DrawRectangle(86, 8+4, 30, 7, OLED_UNFILLED);
		}
		if (KiFlag2)
		{
			OLED_ReverseArea(86, 16+4, 30, 7);
		}
		else
		{
			OLED_DrawRectangle(86, 16+4, 30, 7, OLED_UNFILLED);
		}
		if (KdFlag2)
		{
			OLED_ReverseArea(86, 24+4, 30, 7);
		}
		else
		{
			OLED_DrawRectangle(86, 24+4, 30, 7, OLED_UNFILLED);
		}
	}
	
	OLED_Update();
}

void Mode31_Tick(void)
{
	static uint8_t Count0;
	static uint8_t Count1;
	static uint8_t Count2;
	
	static uint16_t AD[3];
	static int16_t TimeCount;
	
	LocationPID.Actual = Encoder_GetLocation(1);
	AnglePID.Actual = AD_GetValue(1);
	
	if (RunState == 0)
	{
		Motor_SetPWM(1, 0);
	}
	else if (RunState == 1)
	{
		Count0 ++;
		if (Count0 > 40)
		{
			Count0 = 0;
			
			AD[2] = AD[1];
			AD[1] = AD[0];
			AD[0] = AnglePID.Actual;
			
			if (AD[0] > ANGLE_A && AD[0] < ANGLE_B &&
				AD[1] > ANGLE_A && AD[1] < ANGLE_B)
			{
				LocationPID.Target = 0;
				Encoder_SetLocation(1, 0);
				PID_Clear(&AnglePID);
				PID_Clear(&LocationPID);
				RunState = 2;
			}
			if (AD[0] >= ANGLE_B && AD[1] >= ANGLE_B && AD[2] >= ANGLE_B && 
				AD[1] < AD[0] - 2 && AD[1] < AD[2] - 2)
			{
				RunState = 11;
			}
			if (AD[0] <= ANGLE_A && AD[1] <= ANGLE_A && AD[2] <= ANGLE_A && 
				AD[1] > AD[0] + 2 && AD[1] > AD[2] + 2)
			{
				RunState = 21;
			}
		}
		
	}
	
	else if (RunState == 11)
	{
		Motor_SetPWM(1, START_PWM);
		TimeCount = START_TIME;
		RunState = 12;
	}
	else if (RunState == 12)
	{
		TimeCount --;
		if (TimeCount == 0)
		{
			RunState = 13;
		}
	}
	else if (RunState == 13)
	{
		Motor_SetPWM(1, -START_PWM);
		TimeCount = START_TIME;
		RunState = 14;
	}
	else if (RunState == 14)
	{
		TimeCount --;
		if (TimeCount == 0)
		{
			Motor_SetPWM(1, 0);
			RunState = 1;
		}
	}
	
	else if (RunState == 21)
	{
		Motor_SetPWM(1, -START_PWM);
		TimeCount = START_TIME;
		RunState = 22;
	}
	else if (RunState == 22)
	{
		TimeCount --;
		if (TimeCount == 0)
		{
			RunState = 23;
		}
	}
	else if (RunState == 23)
	{
		Motor_SetPWM(1, START_PWM);
		TimeCount = START_TIME;
		RunState = 24;
	}
	else if (RunState == 24)
	{
		TimeCount --;
		if (TimeCount == 0)
		{
			Motor_SetPWM(1, 0);
			RunState = 1;
		}
	}
	
	
	else if (RunState == 2)
	{
		Count1 ++;
		if (Count1 > 50)
		{
			Count1 = 0;
			
			
			PID_Update(&LocationPID);
			
			AnglePID.Target = CENTER_ANGLE - LocationPID.Out;
		}
		
		Count2 ++;
		if (Count2 > 5)
		{
			Count2 = 0;
			
			if (AnglePID.Actual > 3000 || AnglePID.Actual < 1000)
			{
				RunState = 0;
			}
			
			PID_Update(&AnglePID);
			
			if (AnglePID.Out == 0)
			{
				Motor_SetPWM(1, 0);
			}
			else if (AnglePID.Out > 0)
			{
				Motor_SetPWM(1, AnglePID.Out + OFFSET_PWM);
			}
			else if (AnglePID.Out < 0)
			{
				Motor_SetPWM(1, AnglePID.Out - OFFSET_PWM);
			}
		}
	}
}
