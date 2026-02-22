#include "stm32f1xx_hal.h"
#include "./BSP/DMA/dma.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/OLED/OLED.h"

/* DMA简介 */
/*
直接存储器存取(DMA)用来提供在外设和存储器之间或者存储器和存储器之间的高速数据传输。

DMA 传输方式无需 CPU 直接控制传输，也没有中断处理方式那样保留现场和恢复现场的过程，
通过硬件为 RAM 与 I/O 设备开辟一条直接传送数据的通路，能使 CPU 的效率大为提高。

DMA可以链接内存与内存、内存与外设。“内存”指诸如数组地址这样的连续的内存空间，“外设”指某个外设的数据寄存器。
    DMA不可以链接外设与外设。
    连通是单向的。内存到外设与外设到内存是两条不同的链接。
*/

/* 传输过程简介 */
/*
DMA传输数据，先向DMA控制器发出请求->不同外设向DMA的不同通道发出请求->如果右多个请求同时进行，由仲裁器管理先后顺序

    DMA1有7个通道，DMA2有5个；只有大容量以及互联型产品才有DMA2；
    优先级管理分为软件阶段和硬件阶段；在七个请求间的优先权可以通过软件编程设置(共有四级：很高、高、中等和低)，
        假如在相等优先权时由硬件决定(请求 0 优先于请求 1，依此类推)。
    
详细过程：
    外设想通过DMA发送数据，先给DMA控制器发送一个请求；
    控制器收到请求后，返回给外设一个应答信号；
    当外设释放请求；
    DMA 控制器收到应答信号之后，就会启动 DMA 的传输，直到传输完毕。
    
单次DMA传输的数据量是可编程的，最大达到65535。
    
额外解释：
    DMA每个通道对应不同的外设的DMA请求，这是预先规定好的，可以查表得知；
    虽然DMA的每个通道都可接收不止一个请求，但同一时间只能接收一个请求；
    仲裁器管理DMA通道请求分为两个阶段。
        第一阶段属于软件阶段，可以在DMA_CCRx 寄存器中设置，有4个等级：非常高，高，中和低四个优先级。
        第二阶段属于硬件阶段，如果两个或以上的DMA通道请求设置的优先级一样，则他们优先级取决于通道编号，编号越低优先权越高，
            比如通道0高于通道1。
        在大容量产品和互联型产品中，DMA1控制器拥有高于DMA2控制器的优先级。

*/


DMA_HandleTypeDef  g_dma_handle;            /* DMA句柄 */

DMA_HandleTypeDef  g_dma_m2m_handle;            /* 内存到内存DMA句柄 */

extern UART_HandleTypeDef g_uartx_handle[4];   /* UART句柄 */




/**
 * @brief       DMA传输完成回调函数
 *   @note      初始化结构体里设置后，系统自动调用
 *
 * @param       无
 * @retval      无
 */
void MyXferCpltCallback(DMA_HandleTypeDef *hdma){
    if(hdma->Instance == DMA1_Channel4){
        __HAL_DMA_CLEAR_FLAG(&g_dma_handle, DMA_FLAG_TC4);
        OLED_ShowNum(0, 8, 100, 3, OLED_6X8);    /* 显示100% */
        OLED_ShowString(0, 16, "Transimit Finished!", OLED_6X8); /* 提示传送完成 */
        delay_ms(50);
        printf("\r\nMyXferCpltCallback:Transimit Finished!\r\n");
        HAL_UART_DMAStop(&g_uartx_handle[1]);                  /* 传输完成以后关闭串口DMA */
    }
}




/**
 * @brief       串口TX DMA初始化函数
 *   @note      这里的传输形式是固定的, 这点要根据不同的情况来修改
 *              从存储器 -> 外设模式/8位数据宽度/存储器增量模式
 *
 * @param       dmax_chy    : DMA的通道, DMA1_Channel1 ~ DMA1_Channel7, DMA2_Channel1 ~ DMA2_Channel5
 *                            某个外设对应哪个DMA, 哪个通道, 请参考<<STM32中文参考手册 V10>> 10.3.7节
 *                            必须设置正确的DMA及通道, 才能正常使用! 
 * @retval      无
 */
void dma_init(DMA_Channel_TypeDef* DMAx_CHx)
{
    if ((uint32_t)DMAx_CHx > (uint32_t)DMA1_Channel7)     /* 大于DMA1_Channel7, 则为DMA2的通道了 */
    {
        __HAL_RCC_DMA2_CLK_ENABLE();                      /* DMA2时钟使能 */
    }
    else 
    {
        __HAL_RCC_DMA1_CLK_ENABLE();                      /* DMA1时钟使能 */
    }
    
    __HAL_LINKDMA(&g_uartx_handle[1], hdmatx, g_dma_handle);           /* 将DMA与USART1联系起来(发送DMA) */
                                                        /* 目标句柄地址，目标结构体定义里用于存储DMA信息的成员变量名称，DMA句柄 */
    
    /* Tx DMA配置 */
    g_dma_handle.Instance = DMAx_CHx;                               /* USART1_TX使用的DMA通道为: DMA1_Channel4 */
    g_dma_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;             /* DIR = 1 , 存储器到外设模式 */
    g_dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;                 /* 外设非增量模式 */
                                                        /* 增量模式是指像数组一样读写数据，
                                                            一个地址读写完了读下一个相邻的地址
                                                            非增量是指永远只读写一个地址 */
    g_dma_handle.Init.MemInc = DMA_MINC_ENABLE;                     /* 存储器增量模式 */
    g_dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;    /* 外设数据长度:8位 */
                                                    /*  DMA_PDATAALIGN_BYTE     一字节，8位
                                                        DMA_PDATAALIGN_HALFWORD 半字，16位
                                                        DMA_PDATAALIGN_WORD     一字，32位
                                                        */
    g_dma_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;       /* 存储器数据长度:8位 */
                                                    /*
                                                    同上，当PSIZE和MSIZE不一样时，查10.3.4节，表57：可编程的数据传输宽度和大小端操作(当PINC = MINC = 1)
                                                    */
    
    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 1, 3);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);                         /* 这两条使能中断 */
    g_dma_handle.XferCpltCallback=MyXferCpltCallback;               /* 绑定传输完成回调函数 */
    
    g_dma_handle.Init.Mode = DMA_NORMAL;                            /* DMA模式:正常模式(NORMAL) 
                                                                        分为正常模式和循环模式(CIRCULAR)，
                                                                        循环模式下完成一次传输会自动开始下一次传输
                                                                        内存到内存的话是不支持循环模式的*/
    g_dma_handle.Init.Priority = DMA_PRIORITY_MEDIUM;               /* 中等优先级 */

    HAL_DMA_Init(&g_dma_handle);
}


/**
 * @brief       串口TX DMA发送函数，该函数是仿照HAL_UART_Transmit_DMA函数写的，不过删除了修改DMA中断的部分
 *   @note      这里的传输形式是固定的, 这点要根据不同的情况来修改
 *              从存储器 -> 外设模式/8位数据宽度/存储器增量模式
 *
 * @param       huart:  串口句柄
                pData:  源地址
                Size:   大小，字节为单位
 * @retval      无
 */
HAL_StatusTypeDef dma_uart_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    uint32_t *tmp;

  /* Check that a Tx process is not already ongoing */
  if (huart->gState == HAL_UART_STATE_READY)
  {
    if ((pData == NULL) || (Size == 0U))
    {
      return HAL_ERROR;
    }

    /* Process Locked */
    __HAL_LOCK(huart);

    huart->pTxBuffPtr = pData;
    huart->TxXferSize = Size;
    huart->TxXferCount = Size;

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_BUSY_TX;

    /* Enable the UART transmit DMA channel */
    tmp = (uint32_t *)&pData;
    HAL_DMA_Start_IT(huart->hdmatx, *(uint32_t *)tmp, (uint32_t)&huart->Instance->DR, Size);

    /* Clear the TC flag in the SR register by writing 0 to it */
    __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_TC);

    /* Process Unlocked */
    __HAL_UNLOCK(huart);

    /* Enable the DMA transfer for transmit request by setting the DMAT bit
       in the UART CR3 register */
    SET_BIT(huart->Instance->CR3, USART_CR3_DMAT);

    return HAL_OK;
    }
  else
  {
    return HAL_BUSY;
  }
}

/**
 * @brief       内存到内存DMA初始化函数
 *   @note      这里的传输形式是固定的, 这点要根据不同的情况来修改
 *              从存储器 -> 外设模式/8位数据宽度/存储器增量模式
 *
 * @param       无
 * @retval      无
 */
void dma_m2m_init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();                      /* DMA1时钟使能 */
    g_dma_m2m_handle.Instance = DMA1_Channel1;
    g_dma_m2m_handle.Init.Direction = DMA_MEMORY_TO_MEMORY;             /* 存储器到存储器模式 */
    g_dma_m2m_handle.Init.PeriphInc = DMA_PINC_ENABLE;                  /* 外设增量模式 */
    g_dma_m2m_handle.Init.MemInc = DMA_MINC_ENABLE;                     /* 存储器增量模式 */
    g_dma_m2m_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;    /* 外设数据长度:8位 */
    g_dma_m2m_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;       /* 存储器数据长度:8位 */
    g_dma_m2m_handle.Init.Mode = DMA_NORMAL;                                /* DMA模式:正常模式 */
    g_dma_m2m_handle.Init.Priority = DMA_PRIORITY_MEDIUM;                   /* 中等优先级 */
    HAL_DMA_Init(&g_dma_m2m_handle);
    
}
/**
 * @brief       内存到内存DMA开始传输函数,寄存器操作
 *   @note      这里的传输形式是固定的, 这点要根据不同的情况来修改
 *              从存储器 -> 外设模式/8位数据宽度/存储器增量模式
 *
 * @param       dp,传输目标地址，sp,传输源地址，len,数据长度（以字为单位(32位)）
 * @retval      无
 */
void dma_m2m_startTransmit(void *dp,void *sp,uint32_t len)
{
    __HAL_DMA_DISABLE(&g_dma_m2m_handle);                   /* 失能该DMA通道，因为传输数量寄存器只有在通道失能的情况下才能修改 */
    g_dma_m2m_handle.Instance->CNDTR = len ;                /* 重设值 */
    g_dma_m2m_handle.Instance->CPAR = (uint32_t)dp ;        /* 设置外设寄存器地址 */
    g_dma_m2m_handle.Instance->CMAR = (uint32_t)sp ;        /* 设置存储器地址 */
    __HAL_DMA_ENABLE(&g_dma_m2m_handle);                    /* 使能通道 */
    /*--------------------------------下面与上面等效--------------------------------------*/
//    HAL_DMA_Start(&g_dma_m2m_handle, (uint32_t)sp, (uint32_t)dp, sizeof(uint32_t) * len);
}

/**
 * @brief       等待内存到内存DMA传输函数完成
 *   @note      这里的传输形式是固定的, 这点要根据不同的情况来修改
 *              从存储器 -> 存储器模式/8位数据宽度/存储器增量模式
 *
 * @param       无
 * @retval      0表示传输完成，1表示传输失败
 */
uint8_t dma_m2m_waitFin()
{
    while(  (__HAL_DMA_GET_FLAG(&g_dma_m2m_handle, DMA_FLAG_TC1) == RESET)  &&
            (__HAL_DMA_GET_FLAG(&g_dma_m2m_handle, DMA_FLAG_TE1) == RESET)  )
    {
        
    }
    if(__HAL_DMA_GET_FLAG(&g_dma_m2m_handle, DMA_FLAG_TC1)!= RESET){
        return 0;
    }else{
        return 1;
    }
}





void DMA1_Channel4_IRQHandler(void) {
    
    HAL_DMA_IRQHandler(&g_dma_handle);
    
    printf("\r\nIRQH\r\n");
}






















