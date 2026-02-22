#include "stm32f10x.h"                  // Device header
#include "Menu.h"
#include "OLED.h"
#include "LED.h"
#include "Key.h"

extern uint8_t CurrMode, NextMode;

/*定义菜单列表结构体*/
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


struct Menu_List *ListStack[32];		//列表栈，用于列表层叠
int16_t pListStack = 0;					//指示当前列表位于列表栈的第几层

void Menu_Display(void)		//显示菜单
{
	OLED_Clear();
	
	struct Menu_List *pList = ListStack[pListStack];
	
	for (uint8_t i = 0; i < pList->WindowLength; i ++)		//循环显示列表的每一项
	{
		int16_t j = pList->WindowIndex + i;					//获取此项的索引
		
		struct Menu_Item Item = pList->Items[j];			//找到此项
		
		OLED_Printf(0, 16 * i, OLED_8X16, "%s", Item.Name);	//显示此项的名字
		
		if (Item.Type == 1)					//如果项目类型为1（开关型）
		{
			OLED_Printf(80, 16 * i, OLED_8X16, "%d", Item.ON_OFF);					//显示开关值
		}
		else if (Item.Type == 2)			//如果项目类型为2（数值型）
		{
			OLED_Printf(80, 16 * i, OLED_8X16, *Item.NumDisplayFormat, Item.Num);	//显示数值
			
			if (Item.NumSetFlag)			//如果是设置数值模式
			{
				OLED_Printf(72, 16 * i, OLED_8X16, "<");			//在数值左右显示两个箭头
				OLED_Printf(128 - 8, 16 * i, OLED_8X16, ">");
			}
		}
	}
	
	int16_t Cursor = pList->ItemIndex - pList->WindowIndex;			//计算光标位置
	
	OLED_ReverseArea(0, 16 * Cursor, 128, 16);						//将光标选中的项目反色显示
		
	OLED_Update();			//OLED更新
}

void Menu_Up(void)			//上键按下
{
	struct Menu_List *pList = ListStack[pListStack];
	
	struct Menu_Item *Item = &pList->Items[pList->ItemIndex];
	
	if (Item->NumSetFlag)	//当前是设置数值模式
	{
		Item->Num -= Item->NumStep;		//减小当前项的数值
		if (Item->Num < Item->NumMin)
		{
			Item->Num = Item->NumMin;
		}
	}
	else					//否则，即当前不是设置数值模式
	{
		pList->ItemIndex --;			//项目索引减1
		
		if (pList->ItemIndex < 0)		//如果已经是第一项了
		{
			pList->ItemIndex = pList->ItemLength - 1;		//项目索引设置为末尾
			pList->WindowIndex = pList->ItemLength - pList->WindowLength;	//窗口索引设置为末尾
		}
		else							//否则，即没有移到第一项
		{
			int16_t Cursor = pList->ItemIndex - pList->WindowIndex;			//计算光标位置
			if (Cursor < 0)				//如果光标已经移出屏幕了
			{
				pList->WindowIndex --;	//则窗口索引减1，列表整体移动
			}
		}
	}
}

void Menu_Down(void)			//下键按下
{
	struct Menu_List *pList = ListStack[pListStack];
	
	struct Menu_Item *Item = &pList->Items[pList->ItemIndex];
	
	if (Item->NumSetFlag)		//当前是设置数值模式
	{
		Item->Num += Item->NumStep;		//增大当前项的数值
		if (Item->Num > Item->NumMax)
		{
			Item->Num = Item->NumMax;
		}
	}
	else					//否则，即当前不是设置数值模式
	{
		pList->ItemIndex ++;			//项目索引加1
		
		if (pList->ItemIndex >= pList->ItemLength)			//如果已经是最后一项了
		{
			pList->ItemIndex = 0;		//项目索引归零
			pList->WindowIndex = 0;		//窗口索引归零
		}
		else							//否则，即没有移到最后一项
		{
			int16_t Cursor = pList->ItemIndex - pList->WindowIndex;			//计算光标位置
			if (Cursor >= pList->WindowLength)				//如果光标已经移出屏幕了
			{
				pList->WindowIndex ++;	//则窗口索引加1，列表整体移动
			}
		}
	}
}

void Menu_Enter(void)			//确认键按下
{
	/*找到按下确认键的项目*/
	struct Menu_Item *Item = &ListStack[pListStack]->Items[ListStack[pListStack]->ItemIndex];
	
	if (Item->Function != 0)
	{
		Item->Function();		//如果有定义功能函数，则执行功能函数
	}
	if (Item->Type == 1)		//如果项目类型是1（开关型）
	{
		Item->ON_OFF = !Item->ON_OFF;			//则切换开关值
	}
	else if (Item->Type == 2)	//如果项目类型是2（数值型）
	{
		Item->NumSetFlag = !Item->NumSetFlag;	//则切换设置数值模式标志位
	}
	else						//否则，即类型为普通项目
	{
		if (Item->NextList != 0)				//如果绑定了下一个列表
		{
			pListStack ++;		//列表层叠
			ListStack[pListStack] = Item->NextList;		//将绑定的下一个列表显示在当前列表之上
		}
	}
}

void Menu_Back(void)			//返回键按下
{
	/*找到按下返回键的项目*/
	struct Menu_Item *Item = &ListStack[pListStack]->Items[ListStack[pListStack]->ItemIndex];
	
	/*取消设置数值模式标志位*/
	Item->NumSetFlag = 0;
	
	/*如果列表层叠不止一层，则移除最上层列表，返回上一个列表*/
	if (pListStack > 0)
	{
		pListStack --;
	}
}

void Menu_Init(void)
{
	/*列表模式初始化*/
	ListStack[pListStack] = &ParamMenu;
	
	OLED_Clear();
	OLED_Update();
}

void Menu_Loop(void)
{
	if (Key_Check(1, KEY_CLICK))		//K1按下
	{
		Menu_Up();						//执行列表上键按下的功能
	}
	if (Key_Check(2, KEY_CLICK))		//K2按下
	{
		Menu_Down();					//执行列表下键按下的功能
	}
	if (Key_Check(3, KEY_CLICK))		//K3按下
	{
		Menu_Enter();					//执行列表确认键按下的功能
	}
	if (Key_Check(4, KEY_CLICK))		//K4按下
	{
		Menu_Back();					//执行列表返回键按下的功能
		NextMode = 0x31;				//目前只用到了一层列表，因此直接回退到倒立摆模式
	}
	
	Menu_Display();						//显示列表
}

void Menu_Exit(void)
{
	
}

