#ifndef __24C02_H__
#define __24C02_H__


#include "delay.h"
#include "stm32f1xx_hal.h"

#define ADDR 0xA0


void W24C02_Init(void);
uint8_t W24C02_WriteByte(uint8_t innerAddr, uint8_t byte);
uint8_t W24C02_ReadByte(uint8_t innerAddr);
uint8_t W24C02_WriteBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t len);
uint8_t W24C02_ReadBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t len);


#endif
