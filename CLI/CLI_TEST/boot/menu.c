/**
  ******************************************************************************
  * @file    IAP_Main/Src/menu.c 
  * @author  MCD Application Team
  * @brief   This file provides the software which contains the main menu routine.
  *          The main menu gives the options of:
  *             - downloading a new binary file, 
  *             - uploading internal flash memory,
  *             - executing the binary file already loaded 
  *             - configuring the write protection of the Flash sectors where the 
  *               user loads his binary file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */ 

/** @addtogroup STM32H7xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "string.h"
#include "main.h"
#include "common.h"
#include "flash_if.h"
#include "menu.h"
#include "ymodem.h"
#include "xmodem.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
pFunction JumpToApplication;
#if 0
uint32_t JumpAddress;
uint32_t FlashProtection_M7 = 0;
#endif
uint32_t FlashProtection_M4 = 0;
uint8_t aFileName[FILE_NAME_LENGTH];

extern UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
void SerialDownload(void);
void SerialUpload(uint8_t Process);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Download a file via serial port
  * @param  None
  * @retval None
  */

uint32_t    XmodemSize=0;

void SerialDownload(void)
{
  uint8_t number[11] = {0};
  
  BOOL_e result;

  Serial_PutString((uint8_t *)"\n\rWaiting for the File to be sent Xmodem ... (press 'a' to abort)\n\r");  
    
  result = XMODEM_Rx((U32 *)&XmodemSize, (U8 *)APPLICATION_BUFADDRESS);

  if (result == TRUE)
  {
    Serial_PutString((uint8_t *)"\n\r --------------------------------\n\r Transfer Completed Successfully!\r\n Size: ");
    Int2Str(number, XmodemSize);
    Serial_PutString(number);
    Serial_PutString((uint8_t *)" Bytes\r\n");
    Serial_PutString((uint8_t *)" --------------------------------\n\r");
  }
  else
  {
    Serial_PutString((uint8_t *)"\n\n\rAborted by user.\n\n\r");
  }
}

/**
  * @brief  Upload a file via serial port.
  * @param  None
  * @retval None
  */
void SerialUpload(uint8_t Process)
{
  uint8_t status = 0;

  Serial_PutString((uint8_t *)"\n\n\rSelect Receive File\n\r");

  HAL_UART_Receive(&huart2, &status, 1, RX_TIMEOUT);
  if ( status == CRC16)
  {
    /* Transmit the flash image through ymodem protocol */
    status = Ymodem_Transmit((uint8_t*)APPLICATION_ADDRESS_M4, (const uint8_t*)"UploadedFlashImage.bin", APPLICATION_ADDRESS_M4_SIZE);

    if (status != 0)
    {
      Serial_PutString((uint8_t *)"\n\rError Occurred while Transmitting File\n\r");
    }
    else
    {
      Serial_PutString((uint8_t *)"\n\rFile uploaded successfully \n\r");
    }
  }
}

/**
  * @brief  Display the Main Menu on HyperTerminal
  * @param  None
  * @retval None
  */
void Main_Menu(void)
{
  uint8_t key = 0;

  /* Test if any sector of Flash memory where user application will be loaded is write protected */
  
  Serial_PutString((uint8_t *)"\n\n\r======================== Main Menu =========================\r\n\n");
  Serial_PutString((uint8_t *)"  Wait for send X-modem --------------------------------- X\r\n\n");  
  Serial_PutString((uint8_t *)"  Download image to the internal Flash -------------- D\r\n\n");
  Serial_PutString((uint8_t *)"  Reset ------------------------------------------------- R\r\n\n");
  Serial_PutString((uint8_t *)"============================================================\r\n\n");
  
  /* Clean the input path */
  __HAL_UART_FLUSH_DRREGISTER(&huart2);

  while (1)
  {
    /* Receive key */
    HAL_UART_Receive(&huart2, &key, 1, RX_TIMEOUT);

    switch (key)
    {
      case 'X' :
      case 'x' :
        /* Download user application in the Flash */
        SerialDownload();
        break;

      case 'M' :
      case 'm' :
        /* Download user application in the Flash */
        if (((*(__IO uint32_t*)APPLICATION_BUFADDRESS) & 0x1FF80000 ) != 0x10000000)
        {
            Serial_PutString((uint8_t *)"\n\r --------------------------------\n\r The program is invalid.\n\r");
            Serial_PutString((uint8_t *)" --------------------------------\n\r");
        }
        else
        {
            Serial_PutString((uint8_t *)"\n\r --------------------------------\n\r Waiting for the Flash write......");
            FLASH_If_Erase(APPLICATION_ADDRESS_M4); Serial_PutString((uint8_t *)"\n\r Flash Erase Ok.....");
            if (XmodemSize <= APPLICATION_BUFADDRESS_SIZE)
            {
                if (FLASH_If_Write(APPLICATION_ADDRESS_M4, (uint32_t*) APPLICATION_BUFADDRESS, XmodemSize/4) == FLASHIF_OK)     Serial_PutString((uint8_t *)"\n\r Flash Write Ok.....");
                else                                                                                                            Serial_PutString((uint8_t *)"\n\r Flash Write Fail...");
            }
            else
            {
                if (FLASH_If_Write(APPLICATION_ADDRESS_M4, (uint32_t*) APPLICATION_BUFADDRESS, APPLICATION_BUFADDRESS_SIZE/4) == FLASHIF_OK)
                {
                    U32 u32AddAddress = 0;
                    
                    u32AddAddress = APPLICATION_ADDRESS_M4 + APPLICATION_BUFADDRESS_SIZE;
                    XmodemSize = XmodemSize - APPLICATION_BUFADDRESS_SIZE;
                    if (FLASH_If_Write(u32AddAddress, (uint32_t*) APPLICATION_BUFADDRESS2, XmodemSize/4) == FLASHIF_OK)
                    {
                        Serial_PutString((uint8_t *)"\n\r Flash Write Ok.....");
                    }
                    else
                    {
                        Serial_PutString((uint8_t *)"\n\r Flash Write Fail...");
                    }
                }    
                else
                {
                    Serial_PutString((uint8_t *)"\n\r CM4 Flash Write Fail...");
                }    

            }
            Serial_PutString((uint8_t *)"\n\r Flash Write Completed !\n\r");
            Serial_PutString((uint8_t *)" --------------------------------\n\r");
        }    
        break;

      case 'R' :
      case 'r' :
        __NVIC_SystemReset();
        break;
      default:
        Serial_PutString((uint8_t *)"Invalid word ! ==> The word should be either X, D, M, R\n\n\r");
        break;
    }

    huart2.Instance->DR;
    //__HAL_UART_CLEAR_FLAG(&huart2, UART_CLEAR_PEF | UART_CLEAR_CMF | UART_CLEAR_FEF | UART_CLEAR_NEF | UART_CLEAR_OREF | UART_CLEAR_RTOF);
  }
}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
