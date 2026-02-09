/*
 * @Author: wushengran
 * @Date: 2024-12-28 11:07:55
 * @Description:
 *
 * Copyright (c) 2024 by atguigu, All Rights Reserved.
 */
#include "wifi.h"
#include <stdio.h>
#include <usart.h>
#include "MyUI.h"
#define RX_BUFFER_SIZE 1024

uint8_t uart_rx_byte;                // 单字节接收
uint8_t uart_rx_buffer[RX_BUFFER_SIZE];  // 接收缓冲区
volatile uint16_t uart_rx_index = 0;     // 当前缓冲区索引
volatile uint8_t data_ready_flag = 0;    // 数据解析标志
// 定义内部函数，设置Wi-Fi工作模式
static void WIFI_STA_Mode(void);
static void WIFI_AP_Mode(void);

// 初始化，传入WiFi工作模式
void WIFI_Init(WIFI_MODE mode)
{
  // 1. ESP32初始化
  ESP32_Init();

  // 2. 设置工作模式及IP设置
  if (mode == STA)
  {
    WIFI_STA_Mode();
  }
  else if (mode == AP)
  {
    WIFI_AP_Mode();
  }

  printf("Wi-Fi initialization completed!\n");
}

static void WIFI_STA_Mode(void)
{
    // 1. 设置工作模式为STA
    printf("设置为STA模式...\n");
    uint8_t *cmd = "AT+CWMODE=1\r\n";
    ESP32_SendCmd(cmd, "OK", 1000);

    // 2. 连接到指定的AP
    printf("连接AP...\n");
    cmd = "AT+CWJAP=\"WenZhimo\",\"20050617\"\r\n";
    ESP32_SendCmd(cmd, "OK", 1000);

    // 3. 查询本机IP
    printf("本机IP地址为：\n");
    cmd = "AT+CIPSTA?\r\n";
    ESP32_SendCmd(cmd, "OK", 1000);
}

static void WIFI_AP_Mode(void)
{
  // 1. 设置工作模式为AP
    printf("[[INFO]]设置工作模式为AP...\n");
    uint8_t *cmd = "AT+CWMODE=2\r\n";
    while (!ESP32_SendCmd(cmd, "OK", 1000))
    {
        printf("设置工作模式为AP失败，正在重试...\n");
        delay_ms(500);
    }
    printf("[[INFO]]设置工作模式为AP成功\n");
    
    // 2. 设置AP参数
    printf("[[INFO]]设置AP参数...\n");
    cmd = "AT+CWSAP=\"WENZHIMO\",\"WENZHIMO\",1,4\r\n";
    while (!ESP32_SendCmd(cmd, "OK", 1000))
    {
        printf("设设置AP参数失败，正在重试...\n");
        delay_ms(500);
    }
    printf("[[INFO]]设置AP参数成功\n");
    
    // 3. 设置本机IP
    printf("[[INFO]]设置本机IP...\n");
    cmd = "AT+CIPAP=\"192.168.8.1\"\r\n";
    while (!ESP32_SendCmd(cmd, "OK", 1000))
    {
        printf("设置本机IP失败，正在重试...\n");
        delay_ms(500);
    }
    printf("[[INFO]]设置本机IP成功\n");
}

// 启动TCP服务器
void WIFI_TCP_ServerStart(void)
{
  uint8_t *cmd;
  // 1. 设置使能多连接
  cmd = "AT+CIPMUX=1\r\n";
  while (!ESP32_SendCmd(cmd, "OK", 1000))
  {
    printf("多连接使能失败，正在重试...\n");
    delay_ms(500);
  }
  printf("[[INFO]]已使能多连接\n");

  // 2. 开启TCP服务器
  cmd = "AT+CIPSERVER=1,8080\r\n";
  while (!ESP32_SendCmd(cmd, "OK", 1000))
  {
    printf("TCP服务器建立失败，重试...\n");
    delay_ms(500);
  }
  printf("[[INFO]]TCP服务器建立成功，端口号8080.\n");

  // 3. 设置IPD数据格式
  cmd = "AT+CIPDINFO=1\r\n";
  while (!ESP32_SendCmd(cmd, "OK", 1000))
  {
    printf("设置IPD数据格式失败，重试...\n");
    delay_ms(500);
  }
  printf("[[INFO]]设置IPD数据格式成功.\n");
}

// 发送数据（包含连接ID）
void WIFI_TCP_SendData(uint8_t id, uint8_t *data, uint16_t len)
{
  // 1. 设置发送数据的连接ID和数据长度
  printf("Sending data to client...\n");

  uint8_t sendCmd[50] = {0};
  sprintf((char *)sendCmd, "AT+CIPSEND=%d,%d\r\n", id, len);
  ESP32_SendCmd(sendCmd, "OK", 1000);

  // 2. 进入数据模式，直接通过串口发送数据
  printf("send:%s",data);
  HAL_UART_Transmit(&huart4, data, len, 1000);
}

// 用临时缓冲区接收IPD数据
uint8_t tempBuff[1024];
uint16_t tempLen;


void TcpServe_Init(void)
{
    UI_ShowString("Waiting for set TCP server...",RED);
    HAL_UART_Receive_IT(&huart4, &uart_rx_byte, 1); 
    uint8_t *cmd;
    cmd = (uint8_t *)"AT+RST\r\n";
    while (!ESP32_SendCmd(cmd, "OK", 3000))
    {
        printf("[[WARING]]ESP8266 重启失败，重试...\n");
        delay_ms(500);
    }
    delay_ms(1000);//等待重启完成
    // 1. 设置工作模式为AP

    printf("[[INFO]]设置AP模式...\n");

    WIFI_AP_Mode();
    printf("[[INFO]]设置AP模式成功.\n");
    

    printf("[[INFO]]配置TCP服务器...");
    WIFI_TCP_ServerStart();
    printf("[[INFO]]配置TCP服务器完成");
    
    printf("TCP服务器全部配置完成.\n");
    UI_ShowString("Set TCP server success!",LIGHTGREEN);
    delay_ms(500);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    //这个函数在stm32f1xx_hal_uart.c里被UART_DMAReceiveCplt(DMA_HandleTypeDef *hdma)调用，
// UART_DMAReceiveCplt确保了在DMA接收完成后，能够正确地关闭DMA接收，恢复UART状态，并触发相应的用户回调函数来处理接收到的数据。
    if (huart == &huart4)  // ESP8266 连接到 USART4
    {
        printf("%c",uart_rx_byte);
        if (uart_rx_index < RX_BUFFER_SIZE - 1)
        {
            uart_rx_buffer[uart_rx_index++] = uart_rx_byte;

            // 简单判断是否包含 "+IPD" 且含 ':'，认为一帧完成
            if (strstr((char *)uart_rx_buffer, "+IPD") != NULL &&
                strchr((char *)uart_rx_buffer, ':') != NULL)
            //if (strchr((char *)uart_rx_buffer, ':') != NULL)
            {
                //printf("recvEND:%s",uart_rx_buffer);
                data_ready_flag = 1;
            }
        }
        else
        {
            uart_rx_index = 0;  // 超出缓冲长度，清空重新接收
        }

        // 继续接收下一个字节
        //HAL_UART_Transmit(&huart1,(uint8_t *)&uart_rx_byte, 1, HAL_MAX_DELAY);//将窗口4接收到的参数全转发到串口一
        //HAL_UARTEx_ReceiveToIdle_IT(&huart4, uart_rx_buffer, 1024);
        HAL_UART_Receive_IT(&huart4, &uart_rx_byte, 1);//开启下一轮接收
        /*HAL_UART_Receive_IT是通过中断方式异步接收UART串口数据，它会通过使能在收到数据的时候触发中断(即USART_UX_IRQHandler()),
        中断函数会调用公共处理函数、公共处理又会再次调用UART_Receive_IT清除标志位,
        接收操作最终在UART_Receive_IT里完成，HAL_UART_RxCpltCallback(本函数)也在该函数里被再次调用
        因此通常会在本函数里再次调用HAL_UART_Receive_IT来实现接收循环*/
    }
}

