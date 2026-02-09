#include "systemfunction.h"
#include "led.h"
#include "key.h"
#include "esp32.h"
#include "wifi.h"
#include "rc522.h"
#include "bsp_LCD_ILI9341.h"
#include <string.h>
#include <stdio.h>
#include "MyRTC.h"
#include "MyUI.h"
#include "main.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
static uint8_t lastUID[4] = {0};
static uint32_t lastDetectTime = 0;
uint8_t uid[5];       // 存储卡片UID (4字节 + 校验字节)
char CardMessage[64]; // 显示缓冲区
extern uint8_t key1_flag;
extern uint8_t key2_flag;
extern uint8_t key3_flag;
extern uint8_t is_RecvOk;
extern char buff[1024];
extern uint16_t len;

uint8_t rxBuff[1024];
uint16_t rxLen;
uint8_t clientId = 0;
uint8_t clientIp[16];
uint16_t clientPort;

uint8_t status;
uint8_t cardID[5]; // 存储卡片UID（4字节UID + 1字节校验）

// 格式化时间字符串
char timeStr[20];
// 打卡记录存储数组
char g_records[4][30] = {
    "No record",
    "No record",
    "No record",
    "No record"};




void systemRTCInit(void){
    char sendBuffer[64];
    snprintf(sendBuffer, sizeof(sendBuffer),"RTC>ANYTIME");
    //memset(uart_rx_buffer,0,sizeof(uart_rx_buffer));
    WIFI_TCP_SendData(clientId, (uint8_t*)sendBuffer ,strlen(sendBuffer));//发送校准请求
    //读取回复
    HAL_Delay(10);
    memset(rxBuff,0,sizeof(rxBuff));
    //WIFI_TCP_ReadData(rxBuff, &rxLen, &clientId, clientIp, &clientPort);
    //HAL_Delay(10);
    HAL_UARTEx_ReceiveToIdle(&huart4, rxBuff, sizeof(rxBuff), &rxLen, 2000);
    HAL_UART_Receive_IT(&huart4, rxBuff, 1024);//开启下一轮接收
    if(rxLen!=0){
        printf("[recive:]%s[reciveEND]\n",rxBuff);
        int dd=-1,dh=0,dm=0,ds=0;
        char tim[20];
//        char *pstr = strstr(rxBuff,":");
//        
//        for(int i = 0 ;*pstr!='\n'&&pstr!=NULL;i++){
//            tim[i] = *pstr++;
//            
//        }
//        tim[19]='\0';
//        printf("\ntim:%s\n");
//        for(uint16_t i = 0 ;i<sizeof(rxBuff);i++){
//            if(dd!=-1){
//                tim[dd++]=rxBuff[i];
//            }
//            if(rxBuff[i]==':'){
//                dd = 0;
//            }
//        }
        char *colon_pos = strchr(rxBuff, ':')+1;
        printf("\ntim:%s\n",colon_pos);
        sscanf(colon_pos, "%d>%d>%d",&dh,&dm,&ds);//读取数据
        
//        if(dh<24)calendar.hour = dh;
//        if(dm<60)calendar.min = dm;
//        if(ds<60)calendar.sec = ds;
        if(dh>23)dh = 23;
        if(dm>59)dm = 59;
        if(ds>59)ds = 59;
        printf("%d:%d:%d",dh,dm,ds);
//        memset(rxBuff,0,sizeof(rxBuff));
        rtc_set_time(2025,7,13,dh,dm,ds);//设置时间
        UI_ShowTime();//显示当前时间
        HAL_Delay(20);
        printf("RTC OVER\n");
        
        //MyRTC_SetTime(time_now);
    }
}

uint8_t checkCard(void){
    if (MFRC522_Check(uid) == MI_OK)
    {
      // 检查是否是同一张卡且未超过3秒
        if (memcmp(uid, lastUID, 4) != 0 || (HAL_GetTick() - lastDetectTime > 3000))
        {
            memcpy(lastUID, uid, 4);
            lastDetectTime = HAL_GetTick();
            
            // 更新卡片信息显示
            snprintf(CardMessage, sizeof(CardMessage), "UID: %02X%02X%02X%02X",
                     uid[0], uid[1], uid[2], uid[3]);
            UI_ShowCardID(CardMessage);
            
            // 显示打卡成功效果
            UI_ShowState("Card Readed!");
            UI_Success();
            
            // 添加打卡记录
            char timeStr[9];
            MyRTC_GetTime();
            sprintf(timeStr, "%02d:%02d:%02d\n", calendar.hour, calendar.min, calendar.sec);
            
            char record[30];
            snprintf(record, sizeof(record), "> %02d:%02d:%02d UID:%02X%02X%02X%02X",
                     calendar.hour, calendar.min, calendar.sec,
                     uid[0], uid[1], uid[2], uid[3]);
        
            // 更新记录显示
            
            for (int i = 3; i > 0; i--)// 更新记录列表
            {
                strcpy(g_records[i], g_records[i - 1]);
            }
            strcpy(g_records[0], record);
      
            UI_UpdateRecord();//刷新显示区域
    
    
            // 准备发送数据
            char sendBuffer[64];
            snprintf(sendBuffer, sizeof(sendBuffer),
                    "CARD>%02X%02X%02X%02X",
                    uid[0], uid[1], uid[2], uid[3]);
            
            // 发送卡号数据给所有连接的客户端
            printf("Already Send CardID: %s\n", sendBuffer);
            WIFI_TCP_SendData(clientId, (uint8_t*)sendBuffer ,strlen(sendBuffer));
            
            // 2. 选择卡片
            if (MFRC522_SelectTag(uid) > 0)
            {
                // 休眠卡片
                MFRC522_Halt();
            }
            
            // 延时显示成功效果
            HAL_Delay(1500);
            
            // 恢复状态显示
            UI_ShowState("Ready.");
            UI_Waiting();
        }
    }
}