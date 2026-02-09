/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "key.h"
#include "esp32.h"
#include "wifi.h"
#include "rc522.h"
#include "bsp_LCD_ILI9341.h"
#include <string.h>
#include <stdio.h>
#include "MyRTC.h"
#include "MyUI.h"
#include "E:\projects\STM32\MyProject\KaoQingSystem_Server_V3\Hardware\I2C\MyI2C.h"
#include "systemfunction.h"
#include "E:\projects\STM32\MyProject\KaoQingSystem_Server_V3\Hardware\Delay\delay.h"
#include "E:\projects\STM32\MyProject\KaoQingSystem_Server_V3\Hardware\AHT20\atk_aht20.h"

extern uint8_t clientId;


// 格式化时间字符串
extern char timeStr[20];
//RTC校准计数器
uint8_t RTC_Request = 0;
uint8_t AHT20_Request = 0;
/* USER CODE END PV */
void SystemClock_Config(void);


void send_AHT_Data(void)
{
    float temperature;
    float humidity;
    
    //return;
    //while(1){
        
        atk_aht20_init();
        atk_aht20_read_data(&temperature, &humidity);       /* 读取ATH20传感器数据 */

            printf("temperature: %.2f\r\n", temperature);            /* 计算得到湿度值 */
            printf("humidity: %.2f%%\r\n", humidity);               /* 计算得到温度值 */
            printf("\r\n");
            char sendBuffer[64];
            sprintf(sendBuffer,"AHT>%.2f,%.2f",temperature,humidity);
            WIFI_TCP_SendData(clientId, (uint8_t*)sendBuffer ,strlen(sendBuffer));
            printf("%s\n",sendBuffer);
            //delay_ms(500);
    //}
    
}


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_UART4_Init();
    MX_SPI1_Init();
    MX_RTC_Init();
    
    /* USER CODE BEGIN 2 */
    LCD_Init();
    MFRC522_Init(); // RC522初始化
    /*设置默认RTC，用于演示RTC校准*/
    rtc_set_time(2025,7,13,1,1,1);
    UI_Init();     // 初始化UI界面
    UI_ShowState("System be lunching...");
    TcpServe_Init(); // 初始化TCP服务器
    atk_aht20_init();
    //LCD_Init();

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
//        printf("\nwhile\n");        
        // 检测卡片,如果有卡片则进入阻塞
            checkCard();
        // 无卡时显示提示
        static uint32_t lastNoCardTime = 0;
        if (++lastNoCardTime >= 5){
            lastNoCardTime = 0;
            UI_ShowCardID("No card");
            UI_ShowState("Ready.");
            UI_ShowTime();//显示当前时间
        }
        if(AHT20_Request++>10){
            send_AHT_Data();
            AHT20_Request=0;
            LCD_Init();
            UI_Init();
            UI_ShowCardID("No card");
            UI_ShowState("Ready.");
        }
        
        delay_ms(200); // 检测间隔

        
        
        if(Key3_Status()==Key_Press){//当key3按下则主动发起时钟校准
            if(!RTC_Request){
                UI_ShowString("SET RTC VALUE...",RED);
                //RTC_Request = 1;
                systemRTCInit();//校准时钟
                UI_ShowString("RTC SET SUCCESS!",GREEN);
            }
                
        }

    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
