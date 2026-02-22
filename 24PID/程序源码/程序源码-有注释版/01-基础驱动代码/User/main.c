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

/*OLED测试*/
/*下载此段程序后，OLED屏幕上会显示一些测试内容*/
//int main(void)
//{
//	/*模块初始化*/
//	OLED_Init();	//OLED初始化
//	
//	/*显示测试字符串*/
//	/*其中汉字必须事先取模，数据存入OLED_Data.c中的OLED_CF16x16数组*/
//	/*此OLED模块与STM32入门教程中的OLED模块不一样，不能通用*/
//	/*OLED模块教程，可以参考链接：https://www.bilibili.com/video/BV1EN41177Pc*/
//	OLED_ShowString(0, 0, "Hello,世界。", OLED_8X16);
//	
//	/*显示测试浮点数*/
//	OLED_ShowFloatNum(0, 16, 12.345, 2, 3, OLED_8X16);
//	
//	/*OLED_Printf格式化打印测试*/
//	OLED_Printf(0, 32, OLED_8X16, "Num=%d", 666);
//	
//	/*调用OLED功能函数后，必须调用OLED_Update，否则OLED将不会收到任何数据*/
//	OLED_Update();
//	
//	while (1)
//	{
//		
//	}
//}


/*LED测试*/
/*下载此段程序后，STM32板载的LED会以1s为周期闪烁*/
//int main(void)
//{
//	/*模块初始化*/
//	LED_Init();				//LED初始化，此LED为STM32板载连接在PC13端口的LED
//	
//	while (1)
//	{
//		LED_ON();			//LED点亮
//		Delay_ms(500);		//延时500ms
//		LED_OFF();			//LED熄灭
//		Delay_ms(500);		//延时500ms
//		LED_Turn();			//LED亮灭状态取反
//		Delay_ms(500);		//延时500ms
//		LED_Turn();			//LED亮灭状态取反
//		Delay_ms(500);		//延时500ms
//	}
//}


/*定时中断和非阻塞式按键测试*/
/*下载此段程序后，OLED显示变量i会1ms自增一次*/
/*按下K1按键，OLED显示变量j加1，按下K2，j减1，按下K3，j加10，按下K4，j减10*/
//uint16_t i;
//uint16_t j;
//uint8_t KeyNum;

//int main(void)
//{
//	/*模块初始化*/
//	OLED_Init();		//OLED初始化
//	Key_Init();			//非阻塞式按键初始化
//	
//	Timer_Init();		//定时器初始化，1ms定时中断一次
//	
//	while (1)
//	{
//		KeyNum = Key_GetNum();		//获取键码
//		if (KeyNum == 1)			//如果K1按下
//		{
//			j ++;					//变量j加1
//		}
//		if (KeyNum == 2)			//如果K2按下
//		{
//			j --;					//变量j减1
//		}
//		if (KeyNum == 3)			//如果K3按下
//		{
//			j += 10;				//变量j加10
//		}
//		if (KeyNum == 4)			//如果K4按下
//		{
//			j -= 10;				//变量j减10
//		}
//		
//		/*OLED显示*/
//		OLED_Printf(0, 0, OLED_8X16, "i:%05d", i);		//显示变量i
//		OLED_Printf(0, 16, OLED_8X16, "j:%05d", j);		//显示变量j
//		
//		/*OLED更新*/
//		OLED_Update();
//	}
//}

//void TIM1_UP_IRQHandler(void)
//{
//	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
//	{
//		/*定时中断函数1ms自动执行一次*/
//		
//		i ++;			//测试变量i自增
//		
//		Key_Tick();		//调用按键模块的Tick函数，用于驱动按键模块工作
//		
//		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
//	}
//}

/*电位器旋钮测试*/
/*下载此段程序后，分别旋转4个电位器旋钮，OLED显示的4个AD值会对应变化*/
int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	RP_Init();			//电位器旋钮初始化
	
	while (1)
	{
		/*OLED显示*/
		OLED_Printf(0, 0, OLED_8X16, "RP1:%04d", RP_GetValue(1));		//显示电位器旋钮RP1的AD值
		OLED_Printf(0, 16, OLED_8X16, "RP2:%04d", RP_GetValue(2));		//显示电位器旋钮RP2的AD值
		OLED_Printf(0, 32, OLED_8X16, "RP3:%04d", RP_GetValue(3));		//显示电位器旋钮RP3的AD值
		OLED_Printf(0, 48, OLED_8X16, "RP4:%04d", RP_GetValue(4));		//显示电位器旋钮RP4的AD值
		
		/*OLED更新*/
		OLED_Update();
	}
}

/*电机测试*/
/*下载此段程序后，按下K1，电机速度增加，按下K2，电机速度减小，按下K3，电机停止*/
//uint8_t KeyNum;
//int16_t PWM;

//int main(void)
//{
//	/*模块初始化*/
//	OLED_Init();		//OLED初始化
//	Key_Init();			//非阻塞式按键初始化
//	Motor_Init();		//电机初始化
//	
//	Timer_Init();		//定时器初始化，1ms定时中断一次
//	
//	while (1)
//	{
//		KeyNum = Key_GetNum();		//获取键码
//		if (KeyNum == 1)			//如果K1按下
//		{
//			PWM += 10;				//PWM变量加10
//			if (PWM > 100) {PWM = 100;}		//加到100后不再增加
//		}
//		if (KeyNum == 2)			//如果K2按下
//		{
//			PWM -= 10;				//PWM变量减10
//			if (PWM < -100) {PWM = -100;}	//减到-100后不再减小
//		}
//		if (KeyNum == 3)			//如果K3按下
//		{
//			PWM = 0;				//PWM变量归0
//		}
//		
//		Motor_SetPWM(PWM);			//电机设定PWM，将PWM变量的值给电机
//		
//		OLED_Printf(0, 0, OLED_8X16, "PWM:%+04d", PWM);		//OLED显示PWM变量值
//		
//		OLED_Update();				//OLED更新
//	}
//}

//void TIM1_UP_IRQHandler(void)
//{
//	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
//	{
//		/*定时中断函数1ms自动执行一次*/
//		
//		Key_Tick();		//调用按键模块的Tick函数，用于驱动按键模块工作
//		
//		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
//	}
//}


/*编码器测试*/
/*下载此段程序后，手动旋转电机编码器，OLED上会显示旋转速度和位置*/
//int16_t Speed;
//int16_t Location;

//int main(void)
//{
//	/*模块初始化*/
//	OLED_Init();		//OLED初始化
//	Encoder_Init();		//编码器初始化
//	
//	Timer_Init();		//定时器初始化，1ms定时中断一次
//	
//	while (1)
//	{
//		OLED_Printf(0, 0, OLED_8X16, "Speed:%+05d", Speed);			//OLED显示速度
//		OLED_Printf(0, 16, OLED_8X16, "Location:%+05d", Location);	//OLED显示位置
//		
//		OLED_Update();		//OLED更新
//	}
//}

//void TIM1_UP_IRQHandler(void)
//{
//	static uint16_t Count;		//定义静态变量，默认初始化为0，函数结束后，仍保留值和存储空间
//	
//	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
//	{
//		/*定时中断函数1ms自动执行一次*/
//		
//		Count ++;				//使用Count变量计次进行分频
//		if (Count >= 40)		//计次到达40后，进入一次if，即40ms进一次if
//		{
//			Count = 0;			//Count清零，以便下一个周期的计数
//			
//			/*获取编码器的计次值增量，即为速度*/
//			/*此Encoder_Get函数，可以获取两次读取编码器的计次值增量*/
//			/*此Speed值正比于速度，所以可以表示速度，但它的单位并不是速度的标准单位*/
//			/*此处每隔40ms获取一次计次值增量，电机旋转一周的计次值增量约为408*/
//			/*因此如果想转换为标准单位，比如转/秒*/
//			/*则可将此句代码改成Speed = Encoder_Get() / 408.0 / 0.04;*/
//			/*注意Speed变量的类型改为float*/
//			Speed = Encoder_Get();
//			
//			/*速度累加，即为位置*/
//			Location += Speed;
//		}
//		
//		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
//	}
//}


///*串口测试*/
///*下载此段程序后，分别旋转4个电位器旋钮，OLED显示的4个AD值会对应变化*/
///*同时这些值也会通过串口以文本格式发送出去*/
//uint16_t RP1, RP2, RP3, RP4;

//int main(void)
//{
//	/*模块初始化*/
//	OLED_Init();		//OLED初始化
//	RP_Init();			//电位器旋钮初始化
//	Serial_Init();		//串口初始化，波特率9600
//	
//	while (1)
//	{
//		/*获取4个电位器旋钮的AD值*/
//		RP1 = RP_GetValue(1);
//		RP2 = RP_GetValue(2);
//		RP3 = RP_GetValue(3);
//		RP4 = RP_GetValue(4);
//		
//		/*OLED显示4个电位器旋钮的AD值*/
//		OLED_Printf(0, 0, OLED_8X16, "RP1:%04d", RP1);
//		OLED_Printf(0, 16, OLED_8X16, "RP2:%04d", RP2);
//		OLED_Printf(0, 32, OLED_8X16, "RP3:%04d", RP3);
//		OLED_Printf(0, 48, OLED_8X16, "RP4:%04d", RP4);
//		
//		/*OLED更新*/
//		OLED_Update();
//		
//		/*使用串口格式化打印函数，将4个AD值通过串口发送出去*/
//		Serial_Printf("%d,%d,%d,%d\r\n", RP1, RP2, RP3, RP4);
//		
//		/*延时10ms，避免发送过快*/
//		Delay_ms(10);
//	}
//}
