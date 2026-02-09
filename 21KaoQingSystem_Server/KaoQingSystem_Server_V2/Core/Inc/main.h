/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CS_NSS_Pin GPIO_PIN_2
#define CS_NSS_GPIO_Port GPIOC
#define Key1_Pin GPIO_PIN_0
#define Key1_GPIO_Port GPIOA
#define Key1_EXTI_IRQn EXTI0_IRQn
#define Key2_Pin GPIO_PIN_1
#define Key2_GPIO_Port GPIOA
#define Key2_EXTI_IRQn EXTI1_IRQn
#define Key3_Pin GPIO_PIN_4
#define Key3_GPIO_Port GPIOA
#define Key3_EXTI_IRQn EXTI4_IRQn
#define R_LED_Pin GPIO_PIN_5
#define R_LED_GPIO_Port GPIOC
#define B_LED_Pin GPIO_PIN_2
#define B_LED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
