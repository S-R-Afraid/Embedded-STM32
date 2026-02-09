/*
 * @Author: wushengran
 * @Date: 2024-12-27 15:06:13
 * @Description:
 *
 * Copyright (c) 2024 by atguigu, All Rights Reserved.
 */
#include "esp32.h"

// 定义全局变量，接收响应缓冲区和长度
uint8_t respBuff[1024];
uint16_t respLen;

// 初始化
void ESP32_Init(void)
{
    // 1. 初始化串口4
    MX_UART4_Init();

    //2. 发送AT指令，复位ESP32
    uint8_t *cmd = "AT+RST=0\r\n";
    ESP32_SendCmd(cmd, "ready", 1000);

    //3. 引入延时，确保重启成功
    HAL_Delay(2000);
}

// 发送命令
uint8_t ESP32_SendCmd(uint8_t * cmd, const char * expect_response, uint32_t timeout)
{
    // 清除串口错误标志
    __HAL_UART_CLEAR_FLAG(&huart4, UART_FLAG_ORE | UART_FLAG_NE | UART_FLAG_FE | UART_FLAG_PE);

    // 打印发送命令
    printf(">>> %s", cmd);
    HAL_UART_Transmit(&huart4, cmd, strlen((char *)cmd), timeout);

    // 定义接收缓冲
    uint8_t buffer[552] = {0};
    uint16_t recvLen = 0;

    // 接收模块返回
    HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle(&huart4, buffer, sizeof(buffer), &recvLen, timeout);
    //HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle(&huart4, buffer, sizeof(buffer), &recvLen, timeout);
    printf("[[ 串口状态:%d]]\n",status);
    if (status == HAL_OK)
    {
        buffer[recvLen] = '\0'; // 保证字符串安全
        printf("<<< %s\n", buffer);

        if (strstr((char *)buffer, expect_response) != NULL)
            return 1;

        printf("!!! Expected response not found: %s\n", expect_response);
    }
    else
    {
        printf("!!! UART receive timeout or error\n");
    }

    return 0;
}

// 读取响应
void ESP32_ReadResp(uint8_t buff[], uint16_t *len)
{
    // 串口2接收变长数据
    HAL_UARTEx_ReceiveToIdle(&huart4, buff, 1024, len, 1000);
}
