#include "stm32f10x.h"                  // Device header
#include "Menu.h"
#include "OLED.h"
#include "LED.h"
#include "Key.h"

extern uint8_t CurrMode, NextMode;

struct Menu_List ParamMenu = {
	.Name = "参数调节",
	
	.WindowIndex = 0,
	.WindowLength = 4,
	
	.ItemIndex = 0,
	.ItemLength = 4,
	.Items = {
		{.Name = "中心角度", .Type = 2,
		 .Num = 2050, .NumMax = 2200, .NumMin = 1900, .NumStep = 10, .NumDisplayFormat = "%04.0f"},
		
		{.Name = "启动力度", .Type = 2,
		 .Num = 35, .NumMax = 100, .NumMin = 0, .NumStep = 1, .NumDisplayFormat = "%03.0f"},
		
		{.Name = "启动时间", .Type = 2,
		 .Num = 90, .NumMax = 200, .NumMin = 0, .NumStep = 1, .NumDisplayFormat = "%03.0f"},
		
		{.Name = "PWM偏移", .Type = 2,
		 .Num = 0, .NumMax = 50, .NumMin = 0, .NumStep = 1, .NumDisplayFormat = "%03.0f"},
	}
};


struct Menu_List *ListStack[32];
int16_t pListStack = 0;

void Menu_Display(void)
{
	OLED_Clear();
	
	struct Menu_List *pList = ListStack[pListStack];
	
	for (uint8_t i = 0; i < pList->WindowLength; i ++)
	{
		int16_t j = pList->WindowIndex + i;
		
		struct Menu_Item Item = pList->Items[j];
		
		OLED_Printf(0, 16 * i, OLED_8X16, "%s", Item.Name);
		
		if (Item.Type == 1)
		{
			OLED_Printf(80, 16 * i, OLED_8X16, "%d", Item.ON_OFF);
		}
		else if (Item.Type == 2)
		{
			OLED_Printf(80, 16 * i, OLED_8X16, *Item.NumDisplayFormat, Item.Num);
			
			if (Item.NumSetFlag)
			{
				OLED_Printf(72, 16 * i, OLED_8X16, "<");
				OLED_Printf(128 - 8, 16 * i, OLED_8X16, ">");
			}
		}
	}
	
	int16_t Cursor = pList->ItemIndex - pList->WindowIndex;
	
	OLED_ReverseArea(0, 16 * Cursor, 128, 16);
		
	OLED_Update();
}

void Menu_Up(void)
{
	struct Menu_List *pList = ListStack[pListStack];
	
	struct Menu_Item *Item = &pList->Items[pList->ItemIndex];
	
	if (Item->NumSetFlag)
	{
		Item->Num -= Item->NumStep;
		if (Item->Num < Item->NumMin)
		{
			Item->Num = Item->NumMin;
		}
	}
	else
	{
		pList->ItemIndex --;
		
		if (pList->ItemIndex < 0)
		{
			pList->ItemIndex = pList->ItemLength - 1;
			pList->WindowIndex = pList->ItemLength - pList->WindowLength;
		}
		else
		{
			int16_t Cursor = pList->ItemIndex - pList->WindowIndex;
			if (Cursor < 0)
			{
				pList->WindowIndex --;
			}
		}
	}
}

void Menu_Down(void)
{
	struct Menu_List *pList = ListStack[pListStack];
	
	struct Menu_Item *Item = &pList->Items[pList->ItemIndex];
	
	if (Item->NumSetFlag)
	{
		Item->Num += Item->NumStep;
		if (Item->Num > Item->NumMax)
		{
			Item->Num = Item->NumMax;
		}
	}
	else
	{
		pList->ItemIndex ++;
		
		if (pList->ItemIndex >= pList->ItemLength)
		{
			pList->ItemIndex = 0;
			pList->WindowIndex = 0;
		}
		else
		{
			int16_t Cursor = pList->ItemIndex - pList->WindowIndex;
			if (Cursor >= pList->WindowLength)
			{
				pList->WindowIndex ++;
			}
		}
	}
}

void Menu_Enter(void)
{
	struct Menu_Item *Item = &ListStack[pListStack]->Items[ListStack[pListStack]->ItemIndex];
	
	if (Item->Function != 0)
	{
		Item->Function();
	}
	if (Item->Type == 1)
	{
		Item->ON_OFF = !Item->ON_OFF;
	}
	else if (Item->Type == 2)
	{
		Item->NumSetFlag = !Item->NumSetFlag;
	}
	else
	{
		if (Item->NextList != 0)
		{
			pListStack ++;
			ListStack[pListStack] = Item->NextList;
		}
	}
}

void Menu_Back(void)
{
	struct Menu_Item *Item = &ListStack[pListStack]->Items[ListStack[pListStack]->ItemIndex];
	Item->NumSetFlag = 0;
	
	if (pListStack > 0)
	{
		pListStack --;
	}
}

void Menu_Init(void)
{
	ListStack[pListStack] = &ParamMenu;
	
	OLED_Clear();
	OLED_Update();
}

void Menu_Loop(void)
{
	if (Key_Check(1, KEY_CLICK))
	{
		Menu_Up();
	}
	if (Key_Check(2, KEY_CLICK))
	{
		Menu_Down();
	}
	if (Key_Check(3, KEY_CLICK))
	{
		Menu_Enter();
	}
	if (Key_Check(4, KEY_CLICK))
	{
		Menu_Back();
		NextMode = 0x31;
	}
	
	Menu_Display();
}

void Menu_Exit(void)
{
	
}

