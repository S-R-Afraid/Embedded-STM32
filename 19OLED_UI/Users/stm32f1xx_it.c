/**
  ******************************************************************************
  * @file    Templates/Src/stm32f1xx.c
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/ESP8266/esp8266.h"
/** @addtogroup STM32F1xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f1xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/**
  * @}
  */ 

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
/**
 * @brief       串口1中断服务函数
 * @param       无
 * @retval      无
 */
void USART_UX_IRQHandler(void)
    /*和EXTI一样，这里定义的实际上是USART1_IRQHandler函数，由系统自动调用
    该中断服务函数调用次数与接收到的字符数成正比：（英文字符）+（中文字符*2），
    Windows系统的回车算两个英文字符,并且我试过了这个次数与HAL_UART_Receive_IT最后一个参数没有关系*/
{
#if SYS_SUPPORT_OS                          /* 使用OS */
    OSIntEnter();    
#endif
//    __HAL_UART_CLEAR_FLAG(&g_uart1_handle,UART_FLAG_TXE);
//    __HAL_UART_CLEAR_FLAG(&g_uart1_handle, UART_FLAG_RXNE);
    HAL_UART_IRQHandler(&g_uartx_handle[1]);   /* 调用HAL库中断处理公用函数 */

    //printf("%c",g_rx_buffer[0]);
#if SYS_SUPPORT_OS                          /* 使用OS */
    OSIntExit();
#endif

}

/**
 * @brief       串口2中断服务函数
 * @param       无
 * @retval      无
 */
void USART2_IRQHandler(void){
#if SYS_SUPPORT_OS                          /* 使用OS */
    OSIntEnter();    
#endif
//    __HAL_UART_CLEAR_FLAG(&g_uartx_handle[2],UART_FLAG_TXE);
//    __HAL_UART_CLEAR_FLAG(&g_uartx_handle[2], UART_FLAG_RXNE);
    //uint8_t ucCh;
    HAL_UART_IRQHandler(&g_uartx_handle[2]);   /* 调用HAL库中断处理公用函数 */
	if ( __HAL_UART_GET_IT_SOURCE ( &g_uartx_handle[2], USART_IT_RXNE ) != RESET )
	{
		//ucCh  = USART_ReceiveData( USART2 );
        //HAL_UART_Receive_IT(&g_uartx_handle[2], (uint8_t *)&g_uart2_rx_buffer, 1);//一次接收一个字节
        //HAL_UART_Receive_IT(&g_uartx_handle[2], &ucCh, 1);
		
		if ( strEsp8266_Fram_Record .InfBit .FramLength < ( RX_BUF_MAX_LEN - 1 ) )     //预留1个字节写结束符
			strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ++ ]  = g_uart2_rx_buffer;
  
	}

	if ( __HAL_UART_GET_IT_SOURCE (&g_uartx_handle[2], UART_FLAG_IDLE) == SET )   //数据帧接收完毕
	{
    strEsp8266_Fram_Record .InfBit .FramFinishFlag = 1;
		
		//ucCh = USART_ReceiveData( USART2 );          //由软件序列清除中断标志位(先读USART_SR，然后读USART_DR)
        //HAL_UART_Receive_IT(&g_uartx_handle[2], (uint8_t *)&g_uart2_rx_buffer, 1);
		ucTcpClosedFlag = strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "CLOSED\r\n" ) ? 1 : 0;
		
    }
	//Usart_SendString( USART1, q);
		ESP8266_USART_receiveByte =g_uart2_rx_buffer;
		//my_USART_printf(1,(char *)&ESP8266_USART_receiveByte);  //将接收到的数据从串口一发送回去
	
    //HAL_UART_IRQHandler(&g_uartx_handle[2]);   /* 调用HAL库中断处理公用函数 */
#if SYS_SUPPORT_OS                          /* 使用OS */
    OSIntExit();
#endif
}