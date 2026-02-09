#ifndef MYUI_H
#define MYUI_H
#include "main.h"
#include "bsp_LCD_ILI9341.h"

extern char g_records[4][30];

void UI_Init(void);
void UI_ShowState(char*);
void UI_ShowCardID(char*);
void UI_Success(void);
void UI_Waiting(void);
void UI_ShowString(char* str,uint16_t color);
void UI_UpdateRecord(void);
void UI_ShowTime(void);


#endif
