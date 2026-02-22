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

uint8_t KeyNum;

int16_t Speed, Location;		//速度，位置

/*定义内环变量*/
float InnerTarget, InnerActual, InnerOut;			//目标值，实际值，输出值
float InnerKp = 0.3, InnerKi = 0.3, InnerKd = 0;	//比例项，积分项，微分项的权重
float InnerError0, InnerError1, InnerErrorInt;		//本次误差，上次误差，误差积分

/*定义外环变量*/
float OuterTarget, OuterActual, OuterOut;			//目标值，实际值，输出值
float OuterKp = 0.3, OuterKi = 0, OuterKd = 0.4;	//比例项，积分项，微分项的权重
float OuterError0, OuterError1, OuterErrorInt;		//本次误差，上次误差，误差积分

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	Key_Init();			//非阻塞式按键初始化
	Motor_Init();		//电机初始化
	Encoder_Init();		//编码器初始化
	RP_Init();			//电位器旋钮初始化
	Serial_Init();		//串口初始化，波特率9600
	
	Timer_Init();		//定时器初始化，定时中断时间1ms
	
	/*OLED打印一个标题*/
	OLED_Printf(0, 0, OLED_8X16, "2*PID Control");
	OLED_Update();
	
	while (1)
	{
		/*按键修改目标值*/
		/*解除以下注释后，记得屏蔽电位器旋钮修改目标值的代码*/
		/*调节内环目标值使用InnerTarget，调节外环目标值使用OuterTarget*/
//		KeyNum = Key_GetNum();		//获取键码
//		if (KeyNum == 1)			//如果K1按下
//		{
//			InnerTarget += 10;		//目标值加10
//		}
//		if (KeyNum == 2)			//如果K2按下
//		{
//			InnerTarget -= 10;		//目标值减10
//		}
//		if (KeyNum == 3)			//如果K3按下
//		{
//			InnerTarget = 0;		//目标值归0
//		}
		
		/*解除下面一段代码的注释，进行内环PID调参*/
		/*进行内环PID调参时，请注释掉外环控制内环的部分代码*/
		/*RP_GetValue函数返回电位器旋钮的AD值，范围：0~4095*/
		/* 除4095.0可以把AD值归一化，再乘上一个系数，可以调整到一个合适的范围*/
//		InnerKp = RP_GetValue(1) / 4095.0 * 2;				//修改Kp，调整范围：0~2
//		InnerKi = RP_GetValue(2) / 4095.0 * 2;				//修改Ki，调整范围：0~2
//		InnerKd = RP_GetValue(3) / 4095.0 * 2;				//修改Kd，调整范围：0~2
//		InnerTarget = RP_GetValue(4) / 4095.0 * 300 - 150;	//修改目标值，调整范围：-150~150
//		
//		/*OLED显示*/
//		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", InnerKp);			//显示Kp
//		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", InnerKi);			//显示Ki
//		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", InnerKd);			//显示Kd
//		
//		OLED_Printf(64, 16, OLED_8X16, "Tar:%+04.0f", InnerTarget);	//显示目标值
//		OLED_Printf(64, 32, OLED_8X16, "Act:%+04.0f", InnerActual);	//显示实际值
//		OLED_Printf(64, 48, OLED_8X16, "Out:%+04.0f", InnerOut);	//显示输出值
//		
//		OLED_Update();	//OLED更新，调用显示函数后必须调用此函数更新，否则显示的内容不会更新到OLED上
//		
//		Serial_Printf("%f,%f,%f\r\n", InnerTarget, InnerActual, InnerOut);	//串口打印目标值、实际值和输出值
//																	//配合SerialPlot绘图软件，可以显示数据的波形
		
		/*解除下面一段代码的注释，进行外环PID调参*/
		/*内环PID调参完成后，加上外环控制内环的部分代码，再进行外环PID调参*/
		/*RP_GetValue函数返回电位器旋钮的AD值，范围：0~4095*/
		/* 除4095.0可以把AD值归一化，再乘上一个系数，可以调整到一个合适的范围*/
//		OuterKp = RP_GetValue(1) / 4095.0 * 2;				//修改Kp，调整范围：0~2
//		OuterKi = RP_GetValue(2) / 4095.0 * 2;				//修改Ki，调整范围：0~2
//		OuterKd = RP_GetValue(3) / 4095.0 * 2;				//修改Kd，调整范围：0~2
		OuterTarget = RP_GetValue(4) / 4095.0 * 816 - 408;	//修改目标值，调整范围：-408~408
		
		/*OLED显示*/
		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", OuterKp);			//显示Kp
		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", OuterKi);			//显示Ki
		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", OuterKd);			//显示Kd
		
		OLED_Printf(64, 16, OLED_8X16, "Tar:%+04.0f", OuterTarget);	//显示目标值
		OLED_Printf(64, 32, OLED_8X16, "Act:%+04.0f", OuterActual);	//显示实际值
		OLED_Printf(64, 48, OLED_8X16, "Out:%+04.0f", OuterOut);	//显示输出值
		
		OLED_Update();	//OLED更新，调用显示函数后必须调用此函数更新，否则显示的内容不会更新到OLED上
		
		Serial_Printf("%f,%f,%f\r\n", OuterTarget, OuterActual, OuterOut);	//串口打印目标值、实际值和输出值
																	//配合SerialPlot绘图软件，可以显示数据的波形
	}
}

void TIM1_UP_IRQHandler(void)
{
	/*定义静态变量（默认初值为0，函数退出后保留值和存储空间）*/
	static uint16_t Count1, Count2;		//分别用于内环和外环的计次分频
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		/*每隔1ms，程序执行到这里一次*/
		
		Key_Tick();			//调用按键的Tick函数
		
		/*内环计次分频*/
		Count1 ++;				//计次自增
		if (Count1 >= 40)		//如果计次40次，则if成立，即if每隔40ms进一次
		{
			Count1 = 0;			//计次清零，便于下次计次
			
			/*获取实际速度值和实际位置值*/
			/*Encoder_Get函数，可以获取两次读取编码器的计次值增量*/
			/*此值正比于速度，所以可以表示速度，但它的单位并不是速度的标准单位*/
			/*此处每隔40ms获取一次计次值增量，电机旋转一周的计次值增量约为408*/
			/*因此如果想转换为标准单位，比如转/秒*/
			/*则可将此句代码改成Speed = Encoder_Get() / 408.0 / 0.04;*/
			Speed = Encoder_Get();		//获取编码器增量，得到实际速度
			Location += Speed;			//实际速度累加，得到实际位置
			
			/*以下进行内环PID控制*/
			
			/*内环获取实际值*/
			InnerActual = Speed;		//内环为速度环，实际值为速度值
			
			/*内环获取本次误差和上次误差*/
			InnerError1 = InnerError0;					//获取上次误差
			InnerError0 = InnerTarget - InnerActual;	//获取本次误差，目标值减实际值，即为误差值
			
			/*内环误差积分（累加）*/
			/*如果Ki不为0，才进行误差积分，这样做的目的是便于调试*/
			/*因为在调试时，我们可能先把Ki设置为0，这时积分项无作用，误差消除不了，误差积分会积累到很大的值*/
			/*后续一旦Ki不为0，那么因为误差积分已经积累到很大的值了，这就导致积分项疯狂输出，不利于调试*/
			if (InnerKi != 0)					//如果Ki不为0
			{
				InnerErrorInt += InnerError0;	//进行误差积分
			}
			else								//否则
			{
				InnerErrorInt = 0;				//误差积分直接归0
			}
			
			/*内环PID计算*/
			/*使用位置式PID公式，计算得到输出值*/
			InnerOut = InnerKp * InnerError0 + InnerKi * InnerErrorInt + InnerKd * (InnerError0 - InnerError1);
			
			/*内环输出限幅*/
			if (InnerOut > 100) {InnerOut = 100;}		//限制输出值最大为100
			if (InnerOut < -100) {InnerOut = -100;}		//限制输出值最小为100
			
			/*内环执行控制*/
			/*内环输出值给到电机PWM*/
			/*因为此函数的输入范围是-100~100，所以上面输出限幅，需要给Out值限定在-100~100*/
			Motor_SetPWM(InnerOut);
		}
		
		/*外环计次分频*/
		Count2 ++;				//计次自增
		if (Count2 >= 40)		//如果计次40次，则if成立，即if每隔40ms进一次
		{
			Count2 = 0;			//计次清零，便于下次计次
			
			/*以下进行外环PID控制*/
			
			/*外环获取实际值*/
			OuterActual = Location;		//外环为位置环，实际值为位置值
			
			/*外环获取本次误差和上次误差*/
			OuterError1 = OuterError0;					//获取上次误差
			OuterError0 = OuterTarget - OuterActual;	//获取本次误差，目标值减实际值，即为误差值
			
			/*外环误差积分（累加）*/
			/*如果Ki不为0，才进行误差积分，这样做的目的是便于调试*/
			/*因为在调试时，我们可能先把Ki设置为0，这时积分项无作用，误差消除不了，误差积分会积累到很大的值*/
			/*后续一旦Ki不为0，那么因为误差积分已经积累到很大的值了，这就导致积分项疯狂输出，不利于调试*/
			if (OuterKi != 0)					//如果Ki不为0
			{
				OuterErrorInt += OuterError0;	//进行误差积分
			}
			else								//否则
			{
				OuterErrorInt = 0;				//误差积分直接归0
			}
			
			/*外环PID计算*/
			/*使用位置式PID公式，计算得到输出值*/
			OuterOut = OuterKp * OuterError0 + OuterKi * OuterErrorInt + OuterKd * (OuterError0 - OuterError1);
			
			/*外环输出限幅*/
			/*外环输出限幅可以控制内环旋转的速度，外环限幅越大，电机旋转速度越快，但最快不会快过电机满速旋转的速度*/
			if (OuterOut > 20) {OuterOut = 20;}			//限制输出值最大为20
			if (OuterOut < -20) {OuterOut = -20;}		//限制输出值最大为20
			
			/*外环执行控制*/
			/*外环的输出值作用于内环的目标值，组成串级PID结构*/
			InnerTarget = OuterOut;
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
