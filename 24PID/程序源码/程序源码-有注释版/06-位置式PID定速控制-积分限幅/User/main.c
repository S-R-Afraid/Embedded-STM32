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
uint8_t MotorFlag = 1;		//电机标志位，此标志位置0，模拟电机因故障而停止

/*定义变量*/
float Target, Actual, Out;			//目标值，实际值，输出值
float Kp, Ki, Kd;					//比例项，积分项，微分项的权重
float Error0, Error1, ErrorInt;		//本次误差，上次误差，误差积分

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	LED_Init();			//LED初始化
	Key_Init();			//非阻塞式按键初始化
	Motor_Init();		//电机初始化
	Encoder_Init();		//编码器初始化
	RP_Init();			//电位器旋钮初始化
	Serial_Init();		//串口初始化，波特率9600
	
	Timer_Init();		//定时器初始化，定时中断时间1ms
	
	/*OLED打印一个标题*/
	OLED_Printf(0, 0, OLED_8X16, "Speed Control");
	OLED_Update();
	
	while (1)
	{
		KeyNum = Key_GetNum();		//获取键码
		if (KeyNum == 1)			//如果K1按下
		{
			MotorFlag = !MotorFlag;	//电机标志位取非，模拟电机故障和恢复正常
		}
		
		/*LED指示电机工作状态*/
		if (MotorFlag)				//如果电机标志位非0
		{
			LED_ON();				//LED点亮，指示电机正常工作
		}
		else						//否则
		{
			LED_OFF();				//LED熄灭，指示电机故障
		}
		
		/*电位器旋钮修改Kp、Ki、Kd和目标值*/
		/*RP_GetValue函数返回电位器旋钮的AD值，范围：0~4095*/
		/* 除4095.0可以把AD值归一化，再乘上一个系数，可以调整到一个合适的范围*/
		Kp = RP_GetValue(1) / 4095.0 * 2;				//修改Kp，调整范围：0~2
		Ki = RP_GetValue(2) / 4095.0 * 2;				//修改Ki，调整范围：0~2
		Kd = RP_GetValue(3) / 4095.0 * 2;				//修改Kd，调整范围：0~2
		Target = RP_GetValue(4) / 4095.0 * 300 - 150;	//修改目标值，调整范围：-150~150
		
		/*OLED显示*/
		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", Kp);			//显示Kp
		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", Ki);			//显示Ki
		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", Kd);			//显示Kd
		
		OLED_Printf(64, 16, OLED_8X16, "Tar:%+04.0f", Target);	//显示目标值
		OLED_Printf(64, 32, OLED_8X16, "Act:%+04.0f", Actual);	//显示实际值
		OLED_Printf(64, 48, OLED_8X16, "Out:%+04.0f", Out);		//显示输出值
		
		OLED_Update();	//OLED更新，调用显示函数后必须调用此函数更新，否则显示的内容不会更新到OLED上
		
		Serial_Printf("%f,%f,%f,%f\r\n", Target, Actual, Out, ErrorInt);	//串口打印目标值、实际值、输出值和误差积分
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
		
		Key_Tick();				//调用按键的Tick函数
		
		/*计次分频*/
		Count ++;				//计次自增
		if (Count >= 40)		//如果计次40次，则if成立，即if每隔40ms进一次
		{
			Count = 0;			//计次清零，便于下次计次
			
			/*获取实际速度值*/
			/*Encoder_Get函数，可以获取两次读取编码器的计次值增量*/
			/*此值正比于速度，所以可以表示速度，但它的单位并不是速度的标准单位*/
			/*此处每隔40ms获取一次计次值增量，电机旋转一周的计次值增量约为408*/
			/*因此如果想转换为标准单位，比如转/秒*/
			/*则可将此句代码改成Actual = Encoder_Get() / 408.0 / 0.04;*/
			Actual = Encoder_Get();
			
			/*获取本次误差和上次误差*/
			Error1 = Error0;			//获取上次误差
			Error0 = Target - Actual;	//获取本次误差，目标值减实际值，即为误差值
			
			/*误差积分（累加）*/
			ErrorInt += Error0;
			
			/*积分限幅*/
			if (ErrorInt > 500) {ErrorInt = 500;}		//限制误差积分最大为500
			if (ErrorInt < -500) {ErrorInt = -500;}		//限制误差积分最小为-500
			
			/*PID计算*/
			/*使用位置式PID公式，计算得到输出值*/
			Out = Kp * Error0 + Ki * ErrorInt + Kd * (Error0 - Error1);
			
			/*输出限幅*/
			if (Out > 100) {Out = 100;}		//限制输出值最大为100
			if (Out < -100) {Out = -100;}	//限制输出值最小为100
			
			/*执行控制*/
			/*输出值给到电机PWM*/
			/*因为此函数的输入范围是-100~100，所以上面输出限幅，需要给Out值限定在-100~100*/
			/*电机标志位模拟电机正常运行或者故障*/
			if (MotorFlag)					//如果标志位为1，表示正常运行
			{
				Motor_SetPWM(Out);			//正常将Out值赋值给电机PWM
			}
			else							//否则，表示电机故障
			{
				Motor_SetPWM(0);			//电机PWM始终给0，电机不转
			}
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
