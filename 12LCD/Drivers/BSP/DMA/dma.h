#ifndef __DMA_H
#define	__DMA_H

#include "./SYSTEM/sys/sys.h"

void dma_init(DMA_Channel_TypeDef* DMAx_CHx);

HAL_StatusTypeDef dma_uart_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);

void dma_m2m_init(void);

void dma_m2m_startTransmit(void *dp,void *sp,uint32_t len);

#endif






























