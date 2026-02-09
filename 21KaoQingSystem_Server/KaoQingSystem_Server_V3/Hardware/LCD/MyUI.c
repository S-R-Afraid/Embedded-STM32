#include "MyUI.h"
#include "bsp_LCD_ILI9341.h"
#include <string.h>
#include <stdio.h>
#include "MyRTC.h"
extern struct tm *time_now;
static int zx = 20,zy = 100;
void UI_Init(void)
{
    LCD_SetDir(0);
    LCD_Fill(0, 0, 240, 320, WHITE);
    
    // 标题栏
    LCD_Fill(0, 0, 240, 20, VIOLET);
    LCD_String(10, 1, "Attendance System", 16, WHITE, VIOLET);
    UI_ShowTime();

    // 状态栏
    LCD_Fill(0, 20, 240, 50, VIOLET);
    LCD_Fill(2, 22, 68, 48, LIGHTVIOLET);
    LCD_String(4, 30, "Status:", 16, BLACK, LIGHTVIOLET);
    UI_ShowState("System be lunching...");
    //LCD_String(73, 30, "System be lunching...", 16, BLACK, WHITE);

    // 读卡号
    LCD_Fill(0, 50, 240, 80, VIOLET);
    LCD_Fill(2, 52, 68, 78, LIGHTVIOLET);
    LCD_String(4, 60, "CardID:", 16, BLACK, LIGHTVIOLET);
    //状态提示栏
    UI_ShowString("Waiting for set TCP server...");
    // 记录标题栏
    LCD_Fill(0, 150, 240, 170, DARKGREY);
    LCD_String(10, 151, "Recent Records:", 24, WHITE, DARKGREY);
    
    // Initialize records area
    LCD_Fill(10, 175, 230, 300, WHITE);
    UI_UpdateRecord();
    
    // Draw footer info
    LCD_String(20, 310, "ID: 239074423", 16, BLACK, WHITE);
}

void UI_ShowState(char* str){
    LCD_Fill(72,22,238,48,LIGHTVIOLET);
    LCD_String(75, 30, str, 16, BLACK, WHITE);
}

void UI_ShowCardID(char* str){
    LCD_Fill(72,52,238,78,LIGHTVIOLET);
    LCD_String(75, 60, str, 16, BLACK, WHITE);
}

void UI_UpdateRecord(void){
    // 更新记录显示
    LCD_Fill(10, 175, 230, 300, WHITE);
    for (int i = 0; i < 4; i++){
        LCD_String(15, 180 + i * 30, g_records[i], 16, BLACK, WHITE);
    }
}

void UI_Success(void){
    LCD_Fill(0, 80, 240, 150, GREEN);
    LCD_Fill(4, 84, 236, 146, WHITE);
    LCD_String(60, 95, "Success!", 32, GREEN, GREEN);
}

void UI_Waiting(void){
    LCD_Fill(0, 80, 240, 150, ORANGE);
    LCD_Fill(4, 84, 236, 146, WHITE);
    LCD_String(60, 100, "Waiting...", 32, ORANGE, GREEN);
}

void UI_ShowString(char* str){
    LCD_Fill(0, 80, 240, 150, RED);
    LCD_Fill(4, 84, 236, 146, WHITE);
    LCD_String(5, 90, str, 24, RED, GREEN);
}

void UI_ShowTime(void){
    MyRTC_GetTime();
    char uitime[10] ;
    sprintf(uitime,"%02d:%02d:%02d",calendar.hour, calendar.min, calendar.sec);
    LCD_Fill(160,0,240,20,VIOLET);
    LCD_String(160, 2, uitime, 16, WHITE, GREEN);
}

