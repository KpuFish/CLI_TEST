/**
  ******************************************************************************
  * @file    IAP_Main/Inc/flash_if.h 
  * @author  MCD Application Team
  * @brief   This file provides all the headers of the flash_if functions.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/*
Sector  0     0x0800 0000 - 0x0800 3FFF 16  Kbytes
Sector  1     0x0800 4000 - 0x0800 7FFF 16  Kbytes
Sector  2     0x0800 8000 - 0x0800 BFFF 16  Kbytes
Sector  3     0x0800 C000 - 0x0800 FFFF 16  Kbytes
Sector  4     0x0801 0000 - 0x0801 FFFF 64  Kbytes
Sector  5     0x0802 0000 - 0x0803 FFFF 128 Kbytes
Sector  6     0x0804 0000 - 0x0805 FFFF 128 Kbytes
Sector  7     0x0806 0000 - 0x0807 FFFF 128 Kbytes
System memory 0x1FFF 0000 - 0x1FFF 77FF 30  Kbytes
OTP area      0x1FFF 7800 - 0x1FFF 7A0F 528 bytes
Option bytes  0x1FFF C000 - 0x1FFF C00F 16  bytes
*/

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0_BANK1     ((uint32_t)0x08000000) /* Base @ of Sector 0, Bank1, 16  Kbytes */
#define ADDR_FLASH_SECTOR_1_BANK1     ((uint32_t)0x08004000) /* Base @ of Sector 1, Bank1, 16  Kbytes */
#define ADDR_FLASH_SECTOR_2_BANK1     ((uint32_t)0x08008000) /* Base @ of Sector 2, Bank1, 16  Kbytes */
#define ADDR_FLASH_SECTOR_3_BANK1     ((uint32_t)0x0800C000) /* Base @ of Sector 3, Bank1, 16  Kbytes */
#define ADDR_FLASH_SECTOR_4_BANK1     ((uint32_t)0x08010000) /* Base @ of Sector 4, Bank1, 64  Kbytes */
#define ADDR_FLASH_SECTOR_5_BANK1     ((uint32_t)0x08020000) /* Base @ of Sector 5, Bank1, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6_BANK1     ((uint32_t)0x08040000) /* Base @ of Sector 6, Bank1, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7_BANK1     ((uint32_t)0x08060000) /* Base @ of Sector 7, Bank1, 128 Kbytes */

#if 0
#define ADDR_FLASH_SECTOR_0_BANK2     ((uint32_t)0x08100000) /* Base @ of Sector 0, Bank2, 128 Kbyte */
#define ADDR_FLASH_SECTOR_1_BANK2     ((uint32_t)0x08120000) /* Base @ of Sector 1, Bank2, 128 Kbyte */
#define ADDR_FLASH_SECTOR_2_BANK2     ((uint32_t)0x08140000) /* Base @ of Sector 2, Bank2, 128 Kbyte */
#define ADDR_FLASH_SECTOR_3_BANK2     ((uint32_t)0x08160000) /* Base @ of Sector 3, Bank2, 128 Kbyte */
#define ADDR_FLASH_SECTOR_4_BANK2     ((uint32_t)0x08180000) /* Base @ of Sector 4, Bank2, 128 Kbyte */
#define ADDR_FLASH_SECTOR_5_BANK2     ((uint32_t)0x081A0000) /* Base @ of Sector 5, Bank2, 128 Kbyte */
#define ADDR_FLASH_SECTOR_6_BANK2     ((uint32_t)0x081C0000) /* Base @ of Sector 6, Bank2, 128 Kbyte */
#define ADDR_FLASH_SECTOR_7_BANK2     ((uint32_t)0x081E0000) /* Base @ of Sector 7, Bank2, 128 Kbyte */
#endif

/* Error code */
enum 
{
  FLASHIF_OK = 0,
  FLASHIF_ERASEKO,
  FLASHIF_WRITINGCTRL_ERROR,
  FLASHIF_WRITING_ERROR
};
  
enum{
  FLASHIF_PROTECTION_NONE         = 0,
  FLASHIF_PROTECTION_PCROPENABLED = 0x1,
  FLASHIF_PROTECTION_WRPENABLED   = 0x2,
  FLASHIF_PROTECTION_RDPENABLED   = 0x4,
};

/* End of the Flash address */
#define USER_FLASH_END_ADDRESS        APPLICATION_ADDRESS_M4_END
/* Define the user application size */
#define USER_FLASH_SIZE   (USER_FLASH_END_ADDRESS - APPLICATION_ADDRESS + 1)

#define FLASH_BASE_MAIN_APP     0x08020000

/* Define the address from where user application will be loaded.
   Note: the 1st sector 0x08000000-0x0807FFFF is reserved for the IAP code */
#define STM32H7_M7_CORE          (uint8_t) 0x01
#define STM32H7_M4_CORE          (uint8_t) 0x02

#define APPLICATION_ADDRESS_M7      (uint32_t)0x08020000
#define APPLICATION_ADDRESS_M7_END  (uint32_t)0x080FFFFF
#define APPLICATION_ADDRESS_M7_SIZE (uint32_t)(APPLICATION_ADDRESS_M7_END - APPLICATION_ADDRESS_M7 + 1)

#define APPLICATION_ADDRESS_M4      (uint32_t)0x08000000
#define APPLICATION_ADDRESS_M4_END  (uint32_t)0x0807FFFF
#define APPLICATION_ADDRESS_M4_SIZE (uint32_t)(APPLICATION_ADDRESS_M4_END - APPLICATION_ADDRESS_M4 + 1)

// external sdram
#define APPLICATION_BUFADDRESS      (uint32_t)0x24002000
#define APPLICATION_BUFADDRESS_END  (uint32_t)0x2407FFFF
#define APPLICATION_BUFADDRESS_SIZE (uint32_t)(APPLICATION_BUFADDRESS_END - APPLICATION_BUFADDRESS + 1)

#define APPLICATION_BUFADDRESS2      (uint32_t)0x30000000
#define APPLICATION_BUFADDRESS2_END  (uint32_t)0x30047FFF
#define APPLICATION_BUFADDRESS2_SIZE (uint32_t)(APPLICATION_BUFADDRESS2_END - APPLICATION_BUFADDRESS2 + 1)


/* Define bitmap representing user flash area that could be write protected (check restricted to pages 8-39). */
#define FLASH_SECTOR_TO_BE_PROTECTED (OB_WRP_SECTOR_0 | OB_WRP_SECTOR_1 | OB_WRP_SECTOR_2 | OB_WRP_SECTOR_3 |\
                                      OB_WRP_SECTOR_4 | OB_WRP_SECTOR_5 | OB_WRP_SECTOR_6 | OB_WRP_SECTOR_7)

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void              FLASH_If_Init(void);
uint32_t FLASH_If_Erase(uint32_t StartSector);
uint32_t          FLASH_If_Write(uint32_t FlashAddress, uint32_t* Data, uint32_t DataLength);
uint16_t FLASH_If_GetWriteProtectionStatus(uint32_t ProcessAddress);
HAL_StatusTypeDef FLASH_If_WriteProtectionConfig(uint32_t modifier, uint32_t ProcessAddress);

#endif  /* __FLASH_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
