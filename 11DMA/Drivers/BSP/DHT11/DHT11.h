#ifndef __DHT11_H__
#define __DHT11_H__

#include "stm32f1xx_hal.h"
#define DHT_DATA_Pin GPIO_PIN_0
#define DHT_DATA_GPIO_Port GPIOA

typedef enum 
{
    Low_Pin = 0,
    High_Pin = !RESET
} DataValue;

typedef struct
{
    uint8_t Humidity_H;
    uint8_t Humidity_L;
    uint8_t Temperature_H;
    uint8_t Temperature_L;
    uint8_t CheckSum;
}DHT11_Data;

uint8_t DHT11_ReadByte(void);
uint8_t DHT11_ReadData(DHT11_Data *Data);
void DHT11_GPIO_Init(void);

#endif
