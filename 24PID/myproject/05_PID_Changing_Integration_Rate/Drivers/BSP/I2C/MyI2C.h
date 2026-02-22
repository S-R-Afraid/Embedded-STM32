#ifndef __MYI2C_H
#define __MYI2C_H

#include "stm32f1xx_hal.h"                  // Device header
#include "./SYSTEM/delay/delay.h"

#define IIC_SCL_GPIO_PORT GPIOB
#define IIC_SCL_GPIO_PIN GPIO_PIN_11
#define IIC_SDA_GPIO_PORT GPIOB
#define IIC_SDA_GPIO_PIN GPIO_PIN_10

void MyI2C_Init(void);
void MyI2C_Start(void);
void MyI2C_Stop(void);
void MyI2C_SendByte(uint8_t Byte);
uint8_t MyI2C_ReceiveByte(void);
void MyI2C_SendAck(uint8_t AckBit);
uint8_t MyI2C_ReceiveAck(void);

#endif
