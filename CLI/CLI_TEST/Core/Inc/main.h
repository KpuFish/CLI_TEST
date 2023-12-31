/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

//------------------------------------------------
// Include List
//------------------------------------------------
#include "system_config.h"
#include "stm32f4xx_hal.h"

#include "gpio.h"



//------------------------------------------------
// Function List
//------------------------------------------------
void Error_Handler(void);



//------------------------------------------------
// Define List
//------------------------------------------------
#define B1_Pin            GPIO_PIN_13
#define B1_GPIO_Port      GPIOC
#define USART_TX_Pin      GPIO_PIN_2
#define USART_TX_GPIO_    Port GPIOA
#define USART_RX_Pin      GPIO_PIN_3
#define USART_RX_GPIO_    Port GPIOA
#define LD2_Pin           GPIO_PIN_5
#define LD2_GPIO_Port     GPIOA
#define TMS_Pin           GPIO_PIN_13
#define TMS_GPIO_Port     GPIOA
#define TCK_Pin           GPIO_PIN_14
#define TCK_GPIO_Port     GPIOA
#define SWO_Pin           GPIO_PIN_3
#define SWO_GPIO_Port     GPIOB

#define DBG_LED           1


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
