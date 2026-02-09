/*
 * @Author: wushengran
 * @Date: 2024-12-28 11:07:59
 * @Description: 
 * 
 * Copyright (c) 2024 by atguigu, All Rights Reserved. 
 */
#ifndef __WIFI_H
#define __WIFI_H

#include "esp32.h"

extern uint8_t g_currentClientID;
// 定义枚举类型，STA模式和AP模式
extern uint8_t uart_rx_byte;                // 单字节接收

extern uint8_t uart_rx_buffer[1024];  // 接收缓冲区
typedef enum
{
    STA = 1,
    AP = 2
} WIFI_MODE;

// 初始化，传入WiFi工作模式
void WIFI_Init(WIFI_MODE mode);

// 启动TCP服务器
void WIFI_TCP_ServerStart(void);

// 发送数据（包含连接ID）
void WIFI_TCP_SendData(uint8_t id, uint8_t *data, uint16_t len);

// 读取数据（包含连接ID、对端IP和端口号）
void WIFI_TCP_ReadData(uint8_t rxBuff[], uint16_t *rxLen, uint8_t *id, uint8_t *ip, uint16_t *port);

void TcpServe_Init(void);
#endif
