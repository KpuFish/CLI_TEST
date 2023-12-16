/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

#include "cli_interface.h"
#include "system_config.h"
#include "event_log.h"
#include "app_main.h"




/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, (uint8_t*) &uart_rx_byte, UART_BYTE);
  
  setbuf(stdin, NULL);
  setbuf(stdout,NULL); 

  cbf_boot_logo(0, NULL);

  #if MAIN_APP
  CONSOLE_SPLIT;
  printf("if you don't want this, press any key\r\n");
  printf("entering main fw... \r\n");
  CONSOLE_SPLIT;
  #else
  CONSOLE_SPLIT;
  printf("Main App Jumped addr @0x0802 0000\r\n");
  CONSOLE_SPLIT;
  #endif
  
  app_main();

  while (1)
  {
      ;
  }
}
