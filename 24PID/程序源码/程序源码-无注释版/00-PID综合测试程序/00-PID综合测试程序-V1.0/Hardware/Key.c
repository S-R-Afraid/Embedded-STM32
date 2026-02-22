#include "stm32f10x.h"                  // Device header
#include "Key.h"

uint8_t Key_Code[5];

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

uint8_t Key_ReadPin(uint8_t n)
{
	if (n == 1)
	{
		return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10);
	}
	if (n == 2)
	{
		return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
	}
	if (n == 3)
	{
		return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11);
	}
	if (n == 4)
	{
		return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12);
	}
	return 1;
}

uint8_t Key_Check(uint8_t n, uint8_t Event)
{
	if (Key_Code[n] & Event)
	{
		Key_Code[n] &= ~Event;
		return 1;
	}
	return 0;
}

void Key_Clear(void)
{
	uint8_t i;
	for (i = 1; i < 5; i ++)
	{
		Key_Code[i] = 0;
	}
}

void Key_Tick(void)
{
	static uint8_t Count;
	static uint8_t PrevState[5], CurrState[5];
	static uint8_t S[5];
	static uint8_t KeyCount[5];
	uint8_t i;
	
	Count ++;
	if (Count >= 20)
	{
		Count = 0;
		
		for (i = 1; i < 5; i ++)
		{
			PrevState[i] = CurrState[i];
			CurrState[i] = Key_ReadPin(i);
			
			switch (S[i])
			{
				case 0:
					if (PrevState[i] == 1 && CurrState[i] == 0)
					{
						S[i] = 1;
						KeyCount[i] = 0;
					}
				break;
				case 1:
					KeyCount[i] ++;
					if (KeyCount[i] >= 50)		//100*20=2000ms
					{
						S[i] = 0;
						Key_Code[i] |= KEY_LONG;
					}
					if (PrevState[i] == 0 && CurrState[i] == 1)
					{
						S[i] = 2;
						KeyCount[i] = 0;
					}
				break;
				case 2:
					KeyCount[i] ++;
					if (KeyCount[i] >= 1)		//1*20=20ms
					{
						S[i] = 0;
						Key_Code[i] |= KEY_CLICK;
					}
					if (PrevState[i] == 0 && CurrState[i] == 1)
					{
						S[i] = 0;
						Key_Code[i] |= KEY_DOUBLE;
					}
				break;
			}
		}
	}
}
