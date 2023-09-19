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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cli_interface.h"
#include "system_config.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//-----------------------------------------------------
// Checking main app fw jumped
//-----------------------------------------------------
#define MAIN_APP          1
//-----------------------------------------------------

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, (uint8_t*) &uart_rx_byte, UART_BYTE);
  
  setbuf(stdin, NULL);
  setbuf(stdout,NULL); 

  cbf_boot_logo(0, NULL);

  #if !MAIN_APP
  CONSOLE_SPLIT;
  printf("if you don't want this, press any key\r\n");
  printf("entering main fw... \r\n");
  CONSOLE_SPLIT;
  #else
  CONSOLE_SPLIT;
  printf("Main App Jumped addr @0x0802 0000\r\n");
  CONSOLE_SPLIT;
  #endif
  
  uint16_t sec = 3, time = 0;
  time = HAL_GetTick();
  #if !MAIN_APP
  do {
    // key detection
    if (cli.rx_done == CLI_READY) {
      break;
    }

    // Check Time sec
    if (HAL_GetTick() - time >= 1000) {
      time = HAL_GetTick();
      printf("%d\r\n", sec--);

      // time over & jump main app
      if (sec <= 0) {
        cbf_app_fw_jump(0, NULL);
      }
    }
  } while (1);
  #endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
      if (cli.rx_done == CLI_READY) {
          cli.rx_done = CLI_CLEAR;
          parser((char *) &cli.buffer[0]);
      }

      #if MAIN_APP
      // for checking main app fw
      if (HAL_GetTick() - time >= 1000) {
        time = HAL_GetTick();
        #if DBG_LED
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        #endif
      }
      #endif
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
