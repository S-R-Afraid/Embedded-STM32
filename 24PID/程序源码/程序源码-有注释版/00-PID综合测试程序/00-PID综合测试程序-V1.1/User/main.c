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
#include "Store.h"
#include "Timer.h"
#include "Mode0.h"
#include "Mode1.h"
#include "Mode2.h"
#include "Mode3.h"
#include "Menu.h"

uint8_t CurrMode, NextMode;		//当前模式和下一个模式，用于多模式切换
								//模式0：选择系统
								//模式1：硬件测试
								//模式2：电机控制
								//模式3：倒立摆

/*具体来说，模式切换的思路是，在主循环里判断
如果CurrMode和NextMode相同，表示模式没有切换，正常循环执行CurrMode指定模式的Loop函数
在程序中，更改NextMode的值，可以切换模式
改变NextMode后，会导致CurrMode与NextMode不相等
则主循环里，会先执行CurrMode指定模式的Exit函数，处理掉当前模式的收尾工作
随后将CurrMode更新为NextMode，此时CurrMode和NextMode又相同了
之后再执行一遍切换后模式的Init函数，用于初始化
最后CurrMode和NextMode相同，循环执行切换后模式的Loop函数，完成整个模式切换过程*/

int main(void)
{
	/*模块初始化*/
	OLED_Init();
	LED_Init();
	Key_Init();
	AD_Init();
	RP_Init();
	Motor_Init();
	Encoder_Init();
	Serial_Init();
	Timer_Init();
	
	if (Store_Init())		//STM32的FLASH存储器里有一些掉电不丢失的参数
							//此处判断FLASH是第一次使用，还是已经存储过数据了
	{
		/*进入if，表示FLAH是第一次使用*/
		
		/*调用保存参数的函数，初始化FLASH内存储的参数，即重置参数*/
		Mode21_SaveParam();
		Mode22_SaveParam();
		Mode31_SaveParam();
		
		/*OLED显示已重置参数*/
		OLED_Clear();
		OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
		OLED_ShowString(0, 16, "   已重置参数   ", OLED_8X16);
		OLED_ShowString(0, 32, "                ", OLED_8X16);
		OLED_ShowString(0, 48, "             K4>", OLED_8X16);
		OLED_Update();
		
		Delay_ms(1000);
	}
	
	/*显示初始界面*/
	OLED_Clear();
	OLED_ShowString(0, 0,  "   [江协科技]   ", OLED_8X16);
	OLED_ShowString(4, 16, "PID综合测试程序", OLED_8X16);
	OLED_ShowString(0, 32, "      V1.1      ", OLED_8X16);
	OLED_ShowString(0, 48, "             K4>", OLED_8X16);
	OLED_Update();
	
	/*等待K4按键按下*/
	while (Key_Check(4, KEY_CLICK) == 0);
	Key_Clear();
	
	/*K4按键已按下，切换下一个模式为0x01*/
	NextMode = 0x01;
	
	while (1)
	{
		if (CurrMode == NextMode)		//CurrMode和NextMode相同，表示模式没有切换
		{
			switch (CurrMode)			//根据CurrMode的值，执行对应模式的Loop函数
			{
				case 0x01: Mode01_Loop(); break;
				case 0x11: Mode11_Loop(); break;
				case 0x12: Mode12_Loop(); break;
				case 0x13: Mode13_Loop(); break;
				case 0x14: Mode14_Loop(); break;
				case 0x15: Mode15_Loop(); break;
				case 0x16: Mode16_Loop(); break;
				case 0x21: Mode21_Loop(); break;
				case 0x22: Mode22_Loop(); break;
				case 0x31: Mode31_Loop(); break;
				case 0x32: Menu_Loop(); break;
			}
		}
		else						//CurrMode和NextMode不相同，表示当前处于模式切换的瞬间
		{
			switch (CurrMode)		//首先调用当前模式的Exit函数，处理掉当前模式的收尾工作
			{
				case 0x13: Mode13_Exit(); break;
				case 0x31: Mode31_Exit(); break;
				case 0x32: Menu_Exit(); break;
			}
			CurrMode = NextMode;	//CurrMode更新为NextMode，完成模式切换
			Key_Clear();			//模式切换时，清除一下按键标志位，避免干扰下一个模式对按键的判断
			switch (NextMode)		//模式切换后，执行一遍切换后模式的Init函数，用于初始化
			{
				case 0x01: Mode01_Init(); break;
				case 0x11: Mode11_Init(); break;
				case 0x12: Mode12_Init(); break;
				case 0x13: Mode13_Init(); break;
				case 0x14: Mode14_Init(); break;
				case 0x15: Mode15_Init(); break;
				case 0x16: Mode16_Init(); break;
				case 0x20: Mode20_Init(); break;
				case 0x21: Mode21_Init(); break;
				case 0x22: Mode22_Init(); break;
				case 0x30: Mode30_Init(); break;
				case 0x31: Mode31_Init(); break;
				case 0x32: Menu_Init(); break;
			}
		}
	}
}

void TIM1_UP_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		
		/*每隔1ms，程序执行到这里一次*/
		
		Key_Tick();		//调用按键的Tick函数
		
		/*根据当前模式的不同，调用不同模式的Tick函数*/
		if (CurrMode == NextMode)
		{
			switch (CurrMode)
			{
				case 0x21: Mode21_Tick(); break;
				case 0x22: Mode22_Tick(); break;
				case 0x31: Mode31_Tick(); break;
			}
		}
		
		/*定时器中断重叠判断*/
		/*进中断后立刻清除标志位，中断退出之前再次判断标志位*/
		/*如果此时标志位又置1了，说明中断函数的代码执行时间，超过了定时时间*/
		/*这会引起错误，因此这里检查是否出现了此错误，如果出现，则OLED显示TIM ERROR指示一下*/
		if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
		{
			while (1)
			{
				OLED_ShowString(0, 0, "TIM ERROR", OLED_8X16);
				OLED_Update();
			}
		}
	}
}

