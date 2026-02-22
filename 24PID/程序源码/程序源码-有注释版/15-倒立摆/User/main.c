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
#include "AD.h"
#include "PID.h"

/*宏定义参数*/
#define CENTER_ANGLE		2010		//中心角度值
										//此值需要根据自己的设备对应更改，一般在1900~2200之间
										//中心角度值不准确会导致摆杆总是有往一个方向跑的趋势
										//中心角度值不准确会导致位置环PD控制器有稳态误差
										//中心角度值不准确会导致最终按键控制横杆正转一圈和反转一圈的速度不同

#define CENTER_RANGE		500			//中心区间范围
										//规定中心区间的角度值在CENTER_ANGLE - CENTER_RANGE至CENTER_ANGLE + CENTER_RANGE之间

/*定义全局变量*/
uint8_t KeyNum;				//按键键码
uint8_t RunState;			//运行状态，规定0为停止状态，1为运行状态

uint16_t Angle;				//摆杆角度值
int16_t Speed, Location;	//电机的速度和位置

/*定义PID结构体变量*/
PID_t AnglePID = {			//内环角度环PID结构体变量，定义的时候同时给部分成员赋初值
	.Target = CENTER_ANGLE,	//角度环目标值初值设定为中心角度值
	
	.Kp = 0.3,				//比例项权重
	.Ki = 0.01,				//积分项权重
	.Kd = 0.4,				//微分项权重
	
	.OutMax = 100,			//输出限幅的最大值
	.OutMin = -100,			//输出限幅的最小值
};

PID_t LocationPID = {		//外环位置环PID结构体变量，定义的时候同时给部分成员赋初值
	.Target = 0,			//位置环目标值初值设定为0
	
	.Kp = 0.4,				//比例项权重
	.Ki = 0,				//积分项权重
	.Kd = 4,				//微分项权重
	
	.OutMax = 100,			//输出限幅的最大值
	.OutMin = -100,			//输出限幅的最小值
};

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	LED_Init();			//LED初始化
	Key_Init();			//非阻塞式按键初始化
	RP_Init();			//电位器旋钮初始化
	Motor_Init();		//电机初始化
	Encoder_Init();		//编码器初始化
	Serial_Init();		//串口初始化，波特率9600，当前程序暂未使用串口
	AD_Init();			//角度传感器初始化
	
	Timer_Init();		//定时器初始化，定时中断时间1ms
	
	while (1)
	{
		/*按键控制*/
		KeyNum = Key_GetNum();		//获取键码
		if (KeyNum == 1)			//如果K1按下
		{
			RunState = !RunState;	//运行状态取非，用于控制程序启动和停止
		}
		if (KeyNum == 2)			//如果K2按下
		{
			LocationPID.Target += 408;		//位置环目标位置加408，控制横杆正转一圈
			if (LocationPID.Target > 4080)	//如果正转了10圈
			{
				LocationPID.Target = 4080;	//不再允许正转，避免位置越界
			}
		}
		if (KeyNum == 3)			//如果K3按下
		{
			LocationPID.Target -= 408;		//位置环目标位置减408，控制横杆反转一圈
			if (LocationPID.Target < -4080)	//如果反转了10圈
			{
				LocationPID.Target = -4080;	//不再允许反转，避免位置越界
			}
		}
		
		/*LED指示程序运行状态*/
		if (RunState)		//如果运行状态非0
		{
			LED_ON();		//点亮LED，指示程序正在运行
		}
		else				//否则
		{
			LED_OFF();		//熄灭LED，指示程序停止运行
		}
		
		/*电位器旋钮调节角度环和位置环PID参数*/
		/*RP_GetValue函数返回电位器旋钮的AD值，范围：0~4095*/
		/* 除4095.0可以把AD值归一化，再乘上一个系数，可以调整到一个合适的范围*/
//		AnglePID.Kp = RP_GetValue(1) / 4095.0 * 1;				//修改角度环Kp，调整范围：0~1
//		AnglePID.Ki = RP_GetValue(2) / 4095.0 * 1;				//修改角度环Ki，调整范围：0~1
//		AnglePID.Kd = RP_GetValue(3) / 4095.0 * 1;				//修改角度环Kd，调整范围：0~1
		
//		LocationPID.Kp = RP_GetValue(1) / 4095.0 * 1;			//修改位置环Kp，调整范围：0~1
//		LocationPID.Ki = RP_GetValue(2) / 4095.0 * 1;			//修改位置环Ki，调整范围：0~1
//		LocationPID.Kd = RP_GetValue(3) / 4095.0 * 9;			//修改位置环Kd，调整范围：0~9
		
		/*OLED显示*/
		OLED_Printf(0, 0, OLED_6X8, "Angle");							//OLED左侧显示静态字符串Angel
		OLED_Printf(0, 12, OLED_6X8, "Kp:%05.3f", AnglePID.Kp);			//显示Kp
		OLED_Printf(0, 20, OLED_6X8, "Ki:%05.3f", AnglePID.Ki);			//显示Ki
		OLED_Printf(0, 28, OLED_6X8, "Kd:%05.3f", AnglePID.Kd);			//显示Kd
		OLED_Printf(0, 40, OLED_6X8, "Tar:%04.0f", AnglePID.Target);	//显示目标值
		OLED_Printf(0, 48, OLED_6X8, "Act:%04d", Angle);	//显示实际值，使用Angle而不是AnglePID.Actual，可以实现PID程序停止运行时仍然刷新实际值
		OLED_Printf(0, 56, OLED_6X8, "Out:%+04.0f", AnglePID.Out);		//显示输出值
		
		OLED_Printf(64, 0, OLED_6X8, "Location");						//OLED右侧显示静态字符串Location
		OLED_Printf(64, 12, OLED_6X8, "Kp:%05.3f", LocationPID.Kp);		//显示Kp
		OLED_Printf(64, 20, OLED_6X8, "Ki:%05.3f", LocationPID.Ki);		//显示Ki
		OLED_Printf(64, 28, OLED_6X8, "Kd:%05.3f", LocationPID.Kd);		//显示Kd
		OLED_Printf(64, 40, OLED_6X8, "Tar:%+05.0f", LocationPID.Target);	//显示目标值
		OLED_Printf(64, 48, OLED_6X8, "Act:%+05d", Location);	//显示实际值，使用Location而不是LocationPID.Actual，可以实现PID程序停止运行时仍然刷新实际值
		OLED_Printf(64, 56, OLED_6X8, "Out:%+04.0f", LocationPID.Out);	//显示输出值
		
		OLED_Update();	//OLED更新，调用显示函数后必须调用此函数更新，否则显示的内容不会更新到OLED上
	}
}

void TIM1_UP_IRQHandler(void)
{
	/*定义静态变量（默认初值为0，函数退出后保留值和存储空间）*/
	static uint16_t Count1, Count2;		//分别用于角度环和位置环的计次分频
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		/*每隔1ms，程序执行到这里一次*/
		
		Key_Tick();			//调用按键的Tick函数
		
		/*每隔1ms，统一获取角度传感器值*/
		Angle = AD_GetValue();		//获取角度传感器的角度值
		Speed = Encoder_Get();		//获取电机的速度值
		Location += Speed;			//获取电机的位置值
		
		/*摆杆倒下自动停止PID程序*/
		if (! (Angle > CENTER_ANGLE - CENTER_RANGE
			&& Angle < CENTER_ANGLE + CENTER_RANGE))	//如果角度值超过了规定的中心区间
		{
			RunState = 0;			//运行状态变量置0，自动停止PID程序
		}
		
		/*根据运行状态执行PID程序或者停止*/
		if (RunState)				//如果运行状态不为0
		{
			/*角度环计次分频*/
			Count1 ++;				//计次自增
			if (Count1 >= 5)		//如果计次5次，则if成立，即if每隔5ms进一次
			{
				Count1 = 0;			//计次清零，便于下次计次
				
				/*以下进行角度环PID控制*/
				AnglePID.Actual = Angle;		//内环为角度环，实际值为角度值
				PID_Update(&AnglePID);			//调用封装好的函数，一步完成PID计算和更新
				Motor_SetPWM(AnglePID.Out);		//角度环的输出值给到电机PWM
			}
			
			/*位置环计次分频*/
			Count2 ++;				//计次自增
			if (Count2 >= 50)		//如果计次50次，则if成立，即if每隔50ms进一次
			{
				Count2 = 0;			//计次清零，便于下次计次
				
				/*以下进行位置环PID控制*/
				LocationPID.Actual = Location;	//外环为位置环，实际值为位置值
				PID_Update(&LocationPID);		//调用封装好的函数，一步完成PID计算和更新
				AnglePID.Target = CENTER_ANGLE - LocationPID.Out;	//外环的输出值作用于内环的目标值，组成串级PID结构
			}
		}
		else						//如果运行状态为0
		{
			Motor_SetPWM(0);		//不执行PID程序且电机PWM直接设置为0，电机停止
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
