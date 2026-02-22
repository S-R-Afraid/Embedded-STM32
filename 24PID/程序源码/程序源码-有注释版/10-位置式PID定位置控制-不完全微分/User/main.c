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
#include <stdlib.h>

uint8_t KeyNum;

/*定义变量*/
float Target, Actual, Out;			//目标值，实际值，输出值
float Kp, Ki, Kd;					//比例项，积分项，微分项的权重
float Error0, Error1, ErrorInt;		//本次误差，上次误差，误差积分
float DifOut;						//微分项输出

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
	OLED_Printf(0, 0, OLED_8X16, "Location Control");
	OLED_Update();
	
	while (1)
	{
		/*按键修改目标值*/
		/*解除以下注释后，记得屏蔽电位器旋钮修改目标值的代码*/
//		KeyNum = Key_GetNum();		//获取键码
//		if (KeyNum == 1)			//如果K1按下
//		{
//			Target += 10;			//目标值加10
//		}
//		if (KeyNum == 2)			//如果K2按下
//		{
//			Target -= 10;			//目标值减10
//		}
//		if (KeyNum == 3)			//如果K3按下
//		{
//			Target = 0;				//目标值归0
//		}
		
		/*电位器旋钮修改Kp、Ki、Kd和目标值*/
		/*RP_GetValue函数返回电位器旋钮的AD值，范围：0~4095*/
		/* 除4095.0可以把AD值归一化，再乘上一个系数，可以调整到一个合适的范围*/
		Kp = RP_GetValue(1) / 4095.0 * 2;				//修改Kp，调整范围：0~2
		Ki = RP_GetValue(2) / 4095.0 * 2;				//修改Ki，调整范围：0~2
		Kd = RP_GetValue(3) / 4095.0 * 2;				//修改Kd，调整范围：0~2
		Target = RP_GetValue(4) / 4095.0 * 816 - 408;	//修改目标值，调整范围：-408~408
		
		/*OLED显示*/
		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", Kp);			//显示Kp
		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", Ki);			//显示Ki
		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", Kd);			//显示Kd
		
		OLED_Printf(64, 16, OLED_8X16, "Tar:%+04.0f", Target);	//显示目标值
		OLED_Printf(64, 32, OLED_8X16, "Act:%+04.0f", Actual);	//显示实际值
		OLED_Printf(64, 48, OLED_8X16, "Out:%+04.0f", Out);		//显示输出值
		
		OLED_Update();	//OLED更新，调用显示函数后必须调用此函数更新，否则显示的内容不会更新到OLED上
		
		Serial_Printf("%f,%f,%f,%f\r\n", Target, Actual, Out, DifOut);	//串口打印目标值、实际值、输出值和微分项输出
																//配合SerialPlot绘图软件，可以显示数据的波形
	}
}

void TIM1_UP_IRQHandler(void)
{
	/*定义静态变量（默认初值为0，函数退出后保留值和存储空间）*/
	static uint16_t Count;		//用于计次分频
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		/*每隔1ms，程序执行到这里一次*/
		
		Key_Tick();			//调用按键的Tick函数
		
		/*计次分频*/
		Count ++;				//计次自增
		if (Count >= 40)		//如果计次40次，则if成立，即if每隔40ms进一次
		{
			Count = 0;			//计次清零，便于下次计次
			
			/*获取实际位置值*/
			/*Encoder_Get函数，可以获取两次读取编码器的计次值增量*/
			/*计次值增量进行累加，即可得到计次值本身（即实际位置）*/
			/*这里先获取增量，再进行累加，实际上是绕了个弯子*/
			/*如果只需要得到编码器的位置，而不需要得到速度*/
			/*则Encode_Get函数内部的代码可以修改为return TIM_GetCounter(TIM3);直接返回CNT计数器的值*/
			/*修改后，此处代码改为Actual = Encoder_Get();直接得到位置，就不再需要累加了，这样更直接*/
			Actual += Encoder_Get();
			
			/*模拟给实际值添加一些噪声干扰*/
			Actual += rand() % 41 - 20;		//使用随机数生成函数rand生成随机数，并将结果限定在-20~20的范围内
			
			/*获取本次误差和上次误差*/
			Error1 = Error0;			//获取上次误差
			Error0 = Target - Actual;	//获取本次误差，目标值减实际值，即为误差值
			
			/*误差积分（累加）*/
			/*如果Ki不为0，才进行误差积分，这样做的目的是便于调试*/
			/*因为在调试时，我们可能先把Ki设置为0，这时积分项无作用，误差消除不了，误差积分会积累到很大的值*/
			/*后续一旦Ki不为0，那么因为误差积分已经积累到很大的值了，这就导致积分项疯狂输出，不利于调试*/
			if (Ki != 0)				//如果Ki不为0
			{
				ErrorInt += Error0;		//进行误差积分
			}
			else						//否则
			{
				ErrorInt = 0;			//误差积分直接归0
			}
			
			/*PID计算，先得到微分项*/
//			DifOut = Kd * (Error0 - Error1);		//这一句是普通PID的微分项计算公式
			
			/*定义一个滤波强度系数a，a的取值范围是0~1，值越大表示滤波作用越强*/
			float a = 0.9;
			DifOut = (1 - a) * Kd * (Error0 - Error1) + a * DifOut;		//这一句是不完全微分的微分项计算公式
			
			/*PID计算*/
			/*使用位置式PID公式，计算得到输出值*/
			Out = Kp * Error0 + Ki * ErrorInt + DifOut;
			
			/*输出限幅*/
			if (Out > 100) {Out = 100;}		//限制输出值最大为100
			if (Out < -100) {Out = -100;}	//限制输出值最小为100
			
			/*执行控制*/
			/*输出值给到电机PWM*/
			/*因为此函数的输入范围是-100~100，所以上面输出限幅，需要给Out值限定在-100~100*/
			Motor_SetPWM(Out);
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
