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

PID_t SpeedPID = {			//定义定速控制PID结构体变量，并初始化部分成员
	.Kp = 0.3,
	.Ki = 0.3,
	.Kd = 0,
	.OutMax = 1000,
	.OutMin = -1000,
};

PID_t LocationPID = {		//定义定位置控制PID结构体变量，并初始化部分成员
	.Kp = 0.4,
	.Ki = 0.1,
	.Kd = 0.2,
	.OutMax = 100,
	.OutMin = -100,
	.ErrIntThreshold = 40,	//积分分离阈值
};

void Mode21_SaveParam(void)	//保存参数（定速控制的Kp、Ki、Kd）
{
	/*使用uint16_t *类型的指针，指向浮点数，把浮点数的存储内容，存入Store_Data数组中*/
	/*浮点数占用4字节，Store_Data一个数据2字节，因此需要两个Store_Data数据来存储一个浮点数*/
	/*需要保存的参数数量较少，因此存储位置是自己分配的，不重复即可*/
	Store_Data[1] = *((uint16_t *)&SpeedPID.Kp);
	Store_Data[2] = *((uint16_t *)&SpeedPID.Kp + 1);
	Store_Data[3] = *((uint16_t *)&SpeedPID.Ki);
	Store_Data[4] = *((uint16_t *)&SpeedPID.Ki + 1);
	Store_Data[5] = *((uint16_t *)&SpeedPID.Kd);
	Store_Data[6] = *((uint16_t *)&SpeedPID.Kd + 1);
	
	/*将Store_Data的数据，保存到STM32内部的FLASH中，实现掉电不丢失*/
	Store_Save();
}

void Mode21_LoadParam(void)	//加载参数（定速控制的Kp、Ki、Kd）
{
	/*使用uint16_t *类型的指针，读出Store_Data的两个数据，拼接为32位*/
	/*再强转为float，即可恢复原始保存的float类型数据*/
	uint32_t Temp;
	Temp = (Store_Data[2] << 16) | Store_Data[1];
	SpeedPID.Kp = *(float *)&Temp;
	Temp = (Store_Data[4] << 16) | Store_Data[3];
	SpeedPID.Ki = *(float *)&Temp;
	Temp = (Store_Data[6] << 16) | Store_Data[5];
	SpeedPID.Kd = *(float *)&Temp;
}

void Mode22_SaveParam(void)	//保存参数（定位置控制的Kp、Ki、Kd）
{
	/*使用uint16_t *类型的指针，指向浮点数，把浮点数的存储内容，存入Store_Data数组中*/
	/*浮点数占用4字节，Store_Data一个数据2字节，因此需要两个Store_Data数据来存储一个浮点数*/
	/*需要保存的参数数量较少，因此存储位置是自己分配的，不重复即可*/
	Store_Data[11] = *((uint16_t *)&LocationPID.Kp);
	Store_Data[12] = *((uint16_t *)&LocationPID.Kp + 1);
	Store_Data[13] = *((uint16_t *)&LocationPID.Ki);
	Store_Data[14] = *((uint16_t *)&LocationPID.Ki + 1);
	Store_Data[15] = *((uint16_t *)&LocationPID.Kd);
	Store_Data[16] = *((uint16_t *)&LocationPID.Kd + 1);

	/*将Store_Data的数据，保存到STM32内部的FLASH中，实现掉电不丢失*/
	Store_Save();
}

void Mode22_LoadParam(void)	//加载参数（定位置控制的Kp、Ki、Kd）
{
	/*使用uint16_t *类型的指针，读出Store_Data的两个数据，拼接为32位*/
	/*再强转为float，即可恢复原始保存的float类型数据*/
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
	/*显示提示界面*/
	OLED_Clear();
	OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
	OLED_ShowString(0, 16, "  请将设备置于  ", OLED_8X16);
	OLED_ShowString(0, 32, "  电机控制状态  ", OLED_8X16);
	OLED_ShowString(0, 48, "             K4>", OLED_8X16);
	OLED_Update();
	
	/*等待K4按下*/
	while (Key_Check(4, KEY_CLICK) == 0);
	Key_Clear();
	
	/*显示电机控制的选择界面*/
	OLED_Clear();
	OLED_ShowString(0, 0,  "    电机控制    ", OLED_8X16);
	OLED_ShowString(0, 16, " K1：定速控制   ", OLED_8X16);
	OLED_ShowString(0, 32, " K2：定位置控制 ", OLED_8X16);
	OLED_ShowString(0, 48, "                ", OLED_8X16);
	OLED_Update();
	
	/*等待按键按下*/
	while (1)
	{
		if (Key_Check(1, KEY_CLICK))	//K1按下，转入定速控制模式
		{
			NextMode = 0x21;
			break;
		}
		if (Key_Check(2, KEY_CLICK))	//K2按下，转入定位置控制模式
		{
			NextMode = 0x22;
			break;
		}
	}
	Key_Clear();
	
	/*加载参数，恢复之前用户保存过的参数*/
	Mode21_LoadParam();
	Mode22_LoadParam();
}

void Mode21_Init(void)
{
	/*定速控制显示界面初始化*/
	OLED_Clear();
	OLED_ShowString(0, 0,  "    定速控制    ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "                ", OLED_8X16);
	OLED_Update();
	
}

void Mode21_Loop(void)
{
	/*这些Flag标志位用于判断对应的数据是实心填充还是空心填充*/
	/*因为电位器旋钮可以调节参数，但是初始电位器的位置可能偏差很大*/
	/*如果始终允许电位器调节对应的参数*/
	/*那么模式刚进入时，参数会迅速跳变为电位器所对应的值，这不利于快速测试*/
	/*因此这里的效果是，先旋转电位器，使电位器的位置与当前值对应，之后才允许电位器调节参数*/
	/*电位器的位置与当前值对应时，Flag为1，数据为实心填充*/
	/*电位器的位置与当前值不对应时，Flag为0，数据为空心填充*/
	static uint8_t RPFlag, KpFlag, KiFlag, KdFlag, TargetFlag;
	
	float RP_Kp, RP_Ki, RP_Kd, RP_Target;
	
	if (Key_Check(1, KEY_CLICK))		//K1按下
	{
		TargetFlag = 0;
		SpeedPID.Target += 20;			//定速控制PID的目标值加200
		if (SpeedPID.Target > 100)
		{
			SpeedPID.Target = 100;
		}
	}
	if (Key_Check(2, KEY_CLICK))		//K2按下
	{
		TargetFlag = 0;
		SpeedPID.Target -= 20;			//定速控制PID的目标值减200
		if (SpeedPID.Target < -100)
		{
			SpeedPID.Target = -100;
		}
	}
	if (Key_Check(3, KEY_CLICK))		//K3按下
	{
		TargetFlag = 0;
		SpeedPID.Target = 0;			//定速控制PID的目标值归0
	}
	if (Key_Check(4, KEY_CLICK))		//K4按下
	{
		RPFlag = !RPFlag;				//RPFlag取非，表示进入或退出参数调节模式
		KpFlag = 0;
		KiFlag = 0;
		KdFlag = 0;
		TargetFlag = 0;
	}
	if (Key_Check(4, KEY_LONG))			//K4长按
	{
		Mode21_SaveParam();				//保存参数
		
		/*OLED提示已保存参数*/
		OLED_Clear();
		OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
		OLED_ShowString(0, 16, "   已保存参数   ", OLED_8X16);
		OLED_ShowString(0, 32, "                ", OLED_8X16);
		OLED_ShowString(0, 48, "             K4>", OLED_8X16);
		OLED_Update();
		
		/*等待K4按下*/
		while (Key_Check(4, KEY_CLICK) == 0);
		Key_Clear();
		
		/*恢复定速控制界面*/
		OLED_Clear();
		OLED_ShowString(0, 0,  "    定速控制    ", OLED_8X16);
		OLED_ShowString(0, 16, "                ", OLED_8X16);
		OLED_ShowString(0, 32, "                ", OLED_8X16);
		OLED_ShowString(0, 48, "                ", OLED_8X16);
		OLED_Update();
	}
	
	if (RPFlag)				//RPFlag非0，表示进入参数调节模式
	{
		RP_Kp = RP_GetValue(1) / 4095.0 * 3;		//读取电位器的值并转换范围
		RP_Ki = RP_GetValue(2) / 4095.0 * 3;
		RP_Kd = RP_GetValue(3) / 4095.0 * 3;
		RP_Target = RP_GetValue(4) / 4095.0 * 200 - 100;
		
		if (fabs(RP_Kp - SpeedPID.Kp) < 0.05)		//如果RP1旋钮的值与Kp参数对应
		{
			KpFlag = 1;		//允许RP1调节Kp
		}
		if (fabs(RP_Ki - SpeedPID.Ki) < 0.05)		//如果RP2旋钮的值与Ki参数对应
		{
			KiFlag = 1;		//允许RP2调节Ki
		}
		if (fabs(RP_Kd - SpeedPID.Kd) < 0.05)		//如果RP3旋钮的值与Kd参数对应
		{
			KdFlag = 1;		//允许RP3调节Kd
		}
		if (fabs(RP_Target - SpeedPID.Target) < 5)	//如果RP4旋钮的值与Target参数对应
		{
			TargetFlag = 1;	//允许RP4调节Target
		}
		
		if (KpFlag)			//实现RP1调节Kp
		{
			SpeedPID.Kp = RP_Kp;
		}
		if (KiFlag)			//实现RP2调节Ki
		{
			SpeedPID.Ki = RP_Ki;
		}
		if (KdFlag)			//实现RP3调节Kd
		{
			SpeedPID.Kd = RP_Kd;
		}
		if (TargetFlag)		//实现RP4调节Target
		{
			SpeedPID.Target = RP_Target;
		}
	}
	
	/*OLED显示PID各个参数*/
	OLED_Printf(0, 16, OLED_8X16, "Kp:%04.2f", SpeedPID.Kp);
	OLED_Printf(0, 32, OLED_8X16, "Ki:%04.2f", SpeedPID.Ki);
	OLED_Printf(0, 48, OLED_8X16, "Kd:%04.2f", SpeedPID.Kd);
	OLED_Printf(64, 16, OLED_8X16, "Tar:%+04d", (int16_t)SpeedPID.Target);
	OLED_Printf(64, 32, OLED_8X16, "Act:%+04d", (int16_t)SpeedPID.Actual);
	OLED_Printf(64, 48, OLED_8X16, "Out:%+04d", (int16_t)SpeedPID.Out);
	
	/*根据各个标志位的值，在对应数据上显示空心填充或者实心填充，以指示是否正在调节此参数*/
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
	
	/*OLED更新*/
	OLED_Update();
}

void Mode21_Tick(void)
{
	static uint16_t Count;
	static int16_t PrevLocation, CurrLocation;
	Count ++;
	if (Count >= 40)		//计次分频，40ms执行一次PID调控
	{
		Count = 0;
		
		/*获取上次位置和本次位置*/
		PrevLocation = CurrLocation;
		CurrLocation = Encoder_GetLocation(1);
		
		/*PID调控*/
		SpeedPID.Actual = (int16_t)((uint16_t)CurrLocation - (uint16_t)PrevLocation);	//两次位置相减，得到实际速度
		PID_Update(&SpeedPID);			//PID计算
		Motor_SetPWM(1, SpeedPID.Out);	//PID输出值作用与电机PWM
	}
}

void Mode22_Init(void)
{
	/*定位置控制显示界面初始化*/
	OLED_Clear();
	OLED_ShowString(0, 0,  "   定位置控制   ", OLED_8X16);
	OLED_ShowString(0, 16, "                ", OLED_8X16);
	OLED_ShowString(0, 32, "                ", OLED_8X16);
	OLED_ShowString(0, 48, "                ", OLED_8X16);
	OLED_Update();
}

void Mode22_Loop(void)
{
	/*这些Flag标志位用于判断对应的数据是实心填充还是空心填充*/
	/*因为电位器旋钮可以调节参数，但是初始电位器的位置可能偏差很大*/
	/*如果始终允许电位器调节对应的参数*/
	/*那么模式刚进入时，参数会迅速跳变为电位器所对应的值，这不利于快速测试*/
	/*因此这里的效果是，先旋转电位器，使电位器的位置与当前值对应，之后才允许电位器调节参数*/
	/*电位器的位置与当前值对应时，Flag为1，数据为实心填充*/
	/*电位器的位置与当前值不对应时，Flag为0，数据为空心填充*/
	static uint8_t RPFlag, KpFlag, KiFlag, KdFlag, TargetFlag;
	
	float RP_Kp, RP_Ki, RP_Kd, RP_Target;
	
	if (Key_Check(1, KEY_CLICK))		//K1按下
	{
		TargetFlag = 0;
		LocationPID.Target += 408;		//定位置控制目标值加408
		if (LocationPID.Target > 816)
		{
			LocationPID.Target = 816;
		}
	}
	if (Key_Check(2, KEY_CLICK))		//K2按下
	{
		TargetFlag = 0;
		LocationPID.Target -= 408;		//定位置控制目标值减408
		if (LocationPID.Target < -816)
		{
			LocationPID.Target = -816;
		}
	}
	if (Key_Check(3, KEY_CLICK))		//K3按下
	{
		TargetFlag = 0;
		LocationPID.Target = 0;			//定位置控制目标值归0
	}
	if (Key_Check(4, KEY_CLICK))		//K4按下
	{
		RPFlag = !RPFlag;				//RPFlag取非，表示进入或退出参数调节模式
		KpFlag = 0;
		KiFlag = 0;
		KdFlag = 0;
		TargetFlag = 0;
	}
	if (Key_Check(4, KEY_LONG))			//K4长按
	{
		Mode22_SaveParam();			//保存参数
		
		/*OLED提示已保存参数*/
		OLED_Clear();
		OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
		OLED_ShowString(0, 16, "   已保存参数   ", OLED_8X16);
		OLED_ShowString(0, 32, "                ", OLED_8X16);
		OLED_ShowString(0, 48, "             K4>", OLED_8X16);
		OLED_Update();
		
		/*等待K4按下*/
		while (Key_Check(4, KEY_CLICK) == 0);
		Key_Clear();
		
		/*恢复定位置控制界面*/
		OLED_Clear();
		OLED_ShowString(0, 0,  "   定位置控制   ", OLED_8X16);
		OLED_ShowString(0, 16, "                ", OLED_8X16);
		OLED_ShowString(0, 32, "                ", OLED_8X16);
		OLED_ShowString(0, 48, "                ", OLED_8X16);
		OLED_Update();
	}
	
	if (RPFlag)				//RPFlag非0，表示进入参数调节模式
	{
		RP_Kp = RP_GetValue(1) / 4095.0 * 3;		//读取电位器的值并转换范围
		RP_Ki = RP_GetValue(2) / 4095.0 * 3;
		RP_Kd = RP_GetValue(3) / 4095.0 * 3;
		RP_Target = RP_GetValue(4) / 4095.0 * 408 - 204;
		
		if (fabs(RP_Kp - LocationPID.Kp) < 0.05)		//如果RP1旋钮的值与Kp参数对应
		{
			KpFlag = 1;		//允许RP1调节Kp
		}
		if (fabs(RP_Ki - LocationPID.Ki) < 0.05)		//如果RP2旋钮的值与Ki参数对应
		{
			KiFlag = 1;		//允许RP2调节Ki
		}
		if (fabs(RP_Kd - LocationPID.Kd) < 0.05)		//如果RP3旋钮的值与Kd参数对应
		{
			KdFlag = 1;		//允许RP3调节Kd
		}
		if (fabs(RP_Target - LocationPID.Target) < 5)	//如果RP4旋钮的值与Target参数对应
		{
			TargetFlag = 1;	//允许RP4调节Target
		}
		
		if (KpFlag)			//实现RP1调节Kp
		{
			LocationPID.Kp = RP_Kp;
		}
		if (KiFlag)			//实现RP2调节Ki
		{
			LocationPID.Ki = RP_Ki;
		}
		if (KdFlag)			//实现RP3调节Kd
		{
			LocationPID.Kd = RP_Kd;
		}
		if (TargetFlag)		//实现RP4调节Target
		{
			LocationPID.Target = RP_Target;
		}
	}
	
	/*OLED显示PID各个参数*/
	OLED_Printf(0, 16, OLED_8X16, "Kp:%04.2f", LocationPID.Kp);
	OLED_Printf(0, 32, OLED_8X16, "Ki:%04.2f", LocationPID.Ki);
	OLED_Printf(0, 48, OLED_8X16, "Kd:%04.2f", LocationPID.Kd);
	OLED_Printf(64, 16, OLED_8X16, "Tar:%+04d", (int16_t)LocationPID.Target);
	OLED_Printf(64, 32, OLED_8X16, "Act:%+04d", (int16_t)LocationPID.Actual);
	OLED_Printf(64, 48, OLED_8X16, "Out:%+04d", (int16_t)LocationPID.Out);
	
	/*根据各个标志位的值，在对应数据上显示空心填充或者实心填充，以指示是否正在调节此参数*/
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
	
	/*OLED更新*/
	OLED_Update();
}

void Mode22_Tick(void)
{
	static uint16_t Count;
	Count ++;
	if (Count >= 40)		//计次分频，40ms执行一次PID调控
	{
		Count = 0;
		
		/*PID调控*/
		LocationPID.Actual = Encoder_GetLocation(1);	//获取实际位置
		PID_Update(&LocationPID);			//PID计算
		Motor_SetPWM(1, LocationPID.Out);	//PID输出值作用与电机PWM
	}
}
