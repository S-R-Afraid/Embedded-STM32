#ifndef MYSPI_H
#define MYSPI_H
#include "./SYSTEM/sys/sys.h"


#define SPI_SCK_GPIO_PORT GPIOB
#define SPI_SCK_GPIO_PIN GPIO_PIN_13

#define SPI_DO_GPIO_PORT GPIOB
#define SPI_DO_GPIO_PIN GPIO_PIN_14

#define SPI_DI_GPIO_PORT GPIOB
#define SPI_DI_GPIO_PIN GPIO_PIN_15

#define SPI_CLE_ENABLE()    do{__HAL_RCC_GPIOB_CLK_ENABLE();}while(0)



void MySPI_Init(void);
void MySPI_Start(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin);
void MySPI_Stop(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin);
uint8_t MySPI_SwapByte(uint8_t ByteSend);

#endif
