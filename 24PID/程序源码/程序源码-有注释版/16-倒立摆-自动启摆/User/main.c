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

#define START_PWM			35			//启摆时电机转动的力度
										//此值需要根据自己的设备对应更改，一般在30~40之间
										//如果启摆力度太小，摆杆始终摆不上去，则需要加大此值
										//如果启摆力度太大，摆杆经常摆过头，则需要减小此值

#define START_TIME			100			//启摆时电机施加瞬时驱动力的时间，一般在80~120之间
										//启摆时间太小，则力度发出出来
										//启摆时间太大，则不利于共振启摆

/*定义全局变量*/
uint8_t KeyNum;				//按键键码
uint8_t RunState;			//运行状态，规定0为停止状态，1为判断状态
							//21、22、23、24为向左施加瞬时驱动力状态
							//31、32、33、34为向右施加瞬时驱动力状态
							//4为PID控制倒立摆状态

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
			if (RunState == 0)		//如果当前处于停止状态
			{
				RunState = 21;		//则把状态转入21，开始启摆
									//转入状态21，可以让倒立摆以向左施加瞬时驱动力开始运行
									//使摆杆离开角度传感器盲区，避免盲区干扰
			}
			else					//如果当前处于运行状态（包括启摆和倒立摆状态）
			{
				RunState = 0;		//则把状态转入0，停止程序
			}
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
		OLED_Printf(42, 0, OLED_6X8, "%02d", RunState);			//显示运行状态，便于调试
		
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
	static uint16_t Count0, Count1, Count2, CountTime;	//分别用于判断状态、角度环、位置环和计数计时的计次分频
	static uint16_t Angle0, Angle1, Angle2;				//本次角度、上次角度、上上次角度，用于判断摆杆处于最高点位置
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		/*每隔1ms，程序执行到这里一次*/
		
		Key_Tick();			//调用按键的Tick函数
		
		/*每隔1ms，统一获取角度传感器值*/
		Angle = AD_GetValue();		//获取角度传感器的角度值
		Speed = Encoder_Get();		//获取电机的速度值
		Location += Speed;			//获取电机的位置值
		
		/*以下是状态机代码，即根据运行状态的不同，执行不同的功能*/
		if (RunState == 0)			//当前处于状态0，停止状态
		{
			Motor_SetPWM(0);		//电机设置PWM为0，停止运行
		}
		else if (RunState == 1)		//当前处于状态1，判断状态
		{
			/*计次分频*/
			Count0 ++;				//计次自增
			if (Count0 >= 40)		//如果计次40次，则if成立，即if每隔40ms进一次
			{
				Count0 = 0;			//计次清零，便于下次计次
				
				/*获取本次、上次、上上次的角度值，以40ms为间隔，连续采样3次角度值*/
				Angle2 = Angle1;
				Angle1 = Angle0;
				Angle0 = Angle;
				
				/*判断摆杆当前是否处于右侧最高点*/
				if (Angle0 > CENTER_ANGLE + CENTER_RANGE		//3次角度值均位于右侧区间
				 && Angle1 > CENTER_ANGLE + CENTER_RANGE
				 && Angle2 > CENTER_ANGLE + CENTER_RANGE
				 && Angle1 < Angle0								//且中间一次角度值是最小的，表示摆杆处于右侧最高点
				 && Angle1 < Angle2)
				{
					RunState = 21;	//状态转入21，执行向左施加瞬时驱动力的程序
				}
				
				/*判断摆杆当前是否处于左侧最高点*/
				if (Angle0 < CENTER_ANGLE - CENTER_RANGE		//3次角度值均位于左侧区间
				 && Angle1 < CENTER_ANGLE - CENTER_RANGE
				 && Angle2 < CENTER_ANGLE - CENTER_RANGE
				 && Angle1 > Angle0								//且中间一次角度值是最大的，表示摆杆处于左侧最高点
				 && Angle1 > Angle2)
				{
					RunState = 31;				//状态转入31，执行向右施加瞬时驱动力的程序
				}
				
				/*判断摆杆当前是否进入中心区间*/
				if (Angle0 > CENTER_ANGLE - CENTER_RANGE		//2次角度值均位于中心区间
				 && Angle0 < CENTER_ANGLE + CENTER_RANGE
				 && Angle1 > CENTER_ANGLE - CENTER_RANGE
				 && Angle1 < CENTER_ANGLE + CENTER_RANGE)
				{
					/*一些变量初始化归零，避免错误的初值影响PID程序运行*/
					Location = 0;				//实际位置归零
					AnglePID.ErrorInt = 0;		//角度环误差积分归零
					LocationPID.ErrorInt = 0;	//位置环误差积分归零
					
					/*启摆完成*/
					RunState = 4;				//状态转入4，执行PID控制程序
				}
			}
		}
		else if (RunState == 21)		//当前处于状态21，向左摆动
		{
			Motor_SetPWM(START_PWM);	//电机左转，旋转力度由START_PWM指定
			CountTime = START_TIME;		//初始化计数计时器，时间由START_TIME指定
			RunState = 22;				//随后状态自动转入22
		}
		else if (RunState == 22)		//当前处于状态22，延时
		{
			CountTime --;				//计数计时变量递减
			if (CountTime == 0)			//如果减到0了，说明计时时间到
			{
				RunState = 23;			//随后状态转入23
			}
		}
		else if (RunState == 23)		//当前处于状态23，向右摆动
		{
			Motor_SetPWM(-START_PWM);	//电机右转
			CountTime = START_TIME;		//初始化计数计时器
			RunState = 24;				//随后状态自动转入24
		}
		else if (RunState == 24)		//当前处于状态24，延时
		{
			CountTime --;				//计数计时变量递减
			if (CountTime == 0)			//如果减到0了，说明计时时间到
			{
				Motor_SetPWM(0);		//向左施加瞬时驱动力过程结束，电机停止
				RunState = 1;			//随后状态转入1，继续下一次判断
			}
		}
		else if (RunState == 31)		//当前处于状态31，向右摆动
		{
			Motor_SetPWM(-START_PWM);	//电机右转
			CountTime = START_TIME;		//初始化计数计时器
			RunState = 32;				//随后状态自动转入32
		}
		else if (RunState == 32)		//当前处于状态32，延时
		{
			CountTime --;				//计数计时变量递减
			if (CountTime == 0)			//如果减到0了，说明计时时间到
			{
				RunState = 33;			//随后状态转入33
			}
		}
		else if (RunState == 33)		//当前处于状态33，向左摆动
		{
			Motor_SetPWM(START_PWM);	//电机左转
			CountTime = START_TIME;		//初始化计数计时器
			RunState = 34;				//随后状态自动转入34
		}
		else if (RunState == 34)		//当前处于状态24，延时
		{
			CountTime --;				//计数计时变量递减
			if (CountTime == 0)			//如果减到0了，说明计时时间到
			{
				Motor_SetPWM(0);		//向右施加瞬时驱动力过程结束，电机停止
				RunState = 1;			//随后状态转入1，继续下一次判断
			}
		}
		else if (RunState == 4)			//当前处于状态4，倒立摆
		{
			/*摆杆倒下自动停止PID程序*/
			if (! (Angle > CENTER_ANGLE - CENTER_RANGE
				&& Angle < CENTER_ANGLE + CENTER_RANGE))	//如果角度值超过了规定的中心区间
			{
				RunState = 0;			//运行状态变量置0，自动停止PID程序
			}
			
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
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
