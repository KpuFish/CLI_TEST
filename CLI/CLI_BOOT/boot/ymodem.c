/**
  ******************************************************************************
  * @file    IAP_Main/Src/ymodem.c 
  * @author  MCD Application Team
  * @brief   This file provides all the software functions related to the ymodem 
  *          protocol.
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
#include "flash_if.h"
#include "common.h"
#include "ymodem.h"
#include "string.h"
#include "main.h"
#include "menu.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CRC16_F       /* activate the CRC16 integrity */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* @note ATTENTION - please keep this variable 32bit alligned */
uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length);
static void PreparePacket(uint8_t *p_source, uint8_t *p_packet, uint8_t pkt_nr, uint32_t size_blk);
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout);
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size);
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size);

extern UART_HandleTypeDef huart2;
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Receive a packet from sender
  * @param  data
  * @param  length
  *     0: end of transmission
  *     2: abort by sender
  *    >0: packet length
  * @param  timeout
  * @retval HAL_OK: normally return
  *         HAL_BUSY: abort by user
  */

U8 YMODEM_Getchar(U8 *retChar)
{
    if ((__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) ? SET : RESET) == SET)
    {
        *retChar = (U8)huart2.Instance->DR;
    }
    else
    {
        return (FALSE );
    }
    
    return( TRUE );
}

HAL_StatusTypeDef YMODEM_WaitForChar(U8 *u8CPtr, U32 u32ProcessTime)
{
    U32     u32Thisms;

    ++u32ProcessTime;

    u32Thisms = uwTick;

    while (u32ProcessTime)
    {
        if (YMODEM_Getchar(u8CPtr))
        {
            return (HAL_OK);
        }

        if (uwTick != u32Thisms) {
            --u32ProcessTime;
            u32Thisms = uwTick;
        }
    }

    return (HAL_TIMEOUT);
}

  
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout)
{
  uint32_t i=0, crc;
  uint32_t packet_size = 0;
  HAL_StatusTypeDef status;
  uint8_t char1;

  *p_length = 0;
  status = YMODEM_WaitForChar(&char1, timeout);

  if (status == HAL_OK)
  {
    switch (char1)
    {
      case SOH:
        packet_size = PACKET_SIZE;
        break;
      case STX:
        packet_size = PACKET_1K_SIZE;
        break;
      case EOT:
        break;
      case CA:
        if ((YMODEM_WaitForChar(&char1, timeout) == HAL_OK) && (char1 == CA))
        {
          packet_size = 2;
        }
        else
        {
          status = HAL_ERROR;
        }
        break;
      case ABORT1:
      case ABORT2:
        status = HAL_BUSY;
        break;
      default:
        status = HAL_ERROR;
        break;
    }
    *p_data = char1;

    if (packet_size >= PACKET_SIZE )
    {
      status = HAL_OK;
      for (i=0; i<packet_size + PACKET_OVERHEAD_SIZE; i++)
      {
          if (YMODEM_WaitForChar(&p_data[PACKET_NUMBER_INDEX+i], timeout) == HAL_TIMEOUT)
          {
            status = HAL_TIMEOUT;
            //HAL_UART_Receive(&huart2, &p_data[PACKET_NUMBER_INDEX], packet_size + PACKET_OVERHEAD_SIZE, timeout);
          }    
      }  

      /* Simple packet sanity check */
      if (status == HAL_OK )
      {
        if (p_data[PACKET_NUMBER_INDEX] != ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE))
        {
          packet_size = 0;
          status = HAL_ERROR;
        }
        else
        {
          /* Check packet CRC */
          crc = p_data[ packet_size + PACKET_DATA_INDEX ] << 8;
          crc += p_data[ packet_size + PACKET_DATA_INDEX + 1 ];
          if (Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_size) != crc )
          {
            packet_size = 0;
            status = HAL_ERROR;
          }
        }
      }
      else
      {
        packet_size = 0;
      }
    }
  }
  *p_length = packet_size;
  return status;
}

/**
  * @brief  Prepare the first block
  * @param  p_data:  output buffer
  * @param  p_file_name: name of the file to be sent
  * @param  length: length of the file to be sent in bytes
  * @retval None
  */
static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length)
{
  uint32_t i, j = 0;
  uint8_t astring[10];

  /* first 3 bytes are constant */
  p_data[PACKET_START_INDEX] = SOH;
  p_data[PACKET_NUMBER_INDEX] = 0x00;
  p_data[PACKET_CNUMBER_INDEX] = 0xff;

  /* Filename written */
  for (i = 0; (p_file_name[i] != '\0') && (i < FILE_NAME_LENGTH); i++)
  {
    p_data[i + PACKET_DATA_INDEX] = p_file_name[i];
  }

  p_data[i + PACKET_DATA_INDEX] = 0x00;

  /* file size written */
  Int2Str (astring, length);
  i = i + PACKET_DATA_INDEX + 1;
  while (astring[j] != '\0')
  {
    p_data[i++] = astring[j++];
  }

  /* padding with zeros */
  for (j = i; j < PACKET_SIZE + PACKET_DATA_INDEX; j++)
  {
    p_data[j] = 0;
  }
}

/**
  * @brief  Prepare the data packet
  * @param  p_source: pointer to the data to be sent
  * @param  p_packet: pointer to the output buffer
  * @param  pkt_nr: number of the packet
  * @param  size_blk: length of the block to be sent in bytes
  * @retval None
  */
static void PreparePacket(uint8_t *p_source, uint8_t *p_packet, uint8_t pkt_nr, uint32_t size_blk)
{
  uint8_t *p_record;
  uint32_t i, size, packet_size;

  /* Make first three packet */
  packet_size = size_blk >= PACKET_1K_SIZE ? PACKET_1K_SIZE : PACKET_SIZE;
  size = size_blk < packet_size ? size_blk : packet_size;
  if (packet_size == PACKET_1K_SIZE)
  {
    p_packet[PACKET_START_INDEX] = STX;
  }
  else
  {
    p_packet[PACKET_START_INDEX] = SOH;
  }
  p_packet[PACKET_NUMBER_INDEX] = pkt_nr;
  p_packet[PACKET_CNUMBER_INDEX] = (~pkt_nr);
  p_record = p_source;

  /* Filename packet has valid data */
  for (i = PACKET_DATA_INDEX; i < size + PACKET_DATA_INDEX;i++)
  {
    p_packet[i] = *p_record++;
  }
  if ( size  <= packet_size)
  {
    for (i = size + PACKET_DATA_INDEX; i < packet_size + PACKET_DATA_INDEX; i++)
    {
      p_packet[i] = 0x1A; /* EOF (0x1A) or 0x00 */
    }
  }
}

/**
  * @brief  Update CRC16 for input byte
  * @param  crc_in input value 
  * @param  input byte
  * @retval None
  */
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte)
{
  uint32_t crc = crc_in;
  uint32_t in = byte | 0x100;

  do
  {
    crc <<= 1;
    in <<= 1;
    if(in & 0x100)
      ++crc;
    if(crc & 0x10000)
      crc ^= 0x1021;
  }
  
  while(!(in & 0x10000));

  return crc & 0xffffu;
}

/**
  * @brief  Cal CRC16 for YModem Packet
  * @param  data
  * @param  length
  * @retval None
  */
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size)
{
  uint32_t crc = 0;
  const uint8_t* dataEnd = p_data+size;

  while(p_data < dataEnd)
    crc = UpdateCRC16(crc, *p_data++);
 
  crc = UpdateCRC16(crc, 0);
  crc = UpdateCRC16(crc, 0);

  return crc&0xffffu;
}

/**
  * @brief  Calculate Check sum for YModem Packet
  * @param  p_data Pointer to input data
  * @param  size length of input data
  * @retval uint8_t checksum value
  */
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size)
{
  uint32_t sum = 0;
  const uint8_t *p_data_end = p_data + size;

  while (p_data < p_data_end )
  {
    sum += *p_data++;
  }

  return (sum & 0xffu);
}

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Receive a file using the ymodem protocol with CRC16.
  * @param  p_size The size of the file.
  * @retval COM_StatusTypeDef result of reception/programming
  */

U8 Tmp=0;
COM_StatusTypeDef Ymodem_Receive ( uint32_t *p_size)
{
    uint32_t i, j, packet_length, session_done = 0, file_done, errors = 0, session_begin = 0, packets_received = 0;
    uint32_t filesize, sramsize;
    uint32_t *sramdestination, *ramsource;
    uint32_t delay;
    uint8_t *file_ptr;
    uint8_t file_size[FILE_SIZE_LENGTH], tmp;
    uint8_t status;
    COM_StatusTypeDef result = COM_OK;

    /* Initialize sramdestination variable */
    sramdestination = (uint32_t *)APPLICATION_BUFADDRESS;
    sramsize = APPLICATION_BUFADDRESS_SIZE;

    Tmp =0;

    while ((session_done == 0) && (result == COM_OK))
    {
        packets_received = 0;
        file_done = 0;
        while ((file_done == 0) && (result == COM_OK))
        {
            status = ReceivePacket(aPacketData, &packet_length, DOWNLOAD_TIMEOUT);
            switch (status)
            {
              case HAL_OK:
                errors = 0;
                switch (packet_length)
                {
                  case 2:
                    /* Abort by sender */
                    if (session_begin == 2)
                    {
                        /* End session */
                        Serial_PutByte(CA);
                        Serial_PutByte(CA);
                        result = COM_DATA;
                    }
                    else
                    {
                        /* Abort communication */
                        Serial_PutByte(CA);
                        Serial_PutByte(CA);
                        result = COM_ABORT;
                    }
                    break;
                  case 0:
                    /* End of transmission */
                    Serial_PutByte(ACK);
                    Serial_PutByte(CA);
                    Serial_PutByte(CA);
                    file_done = 1;
                    session_done = 1;
                    break;
                  default:
                    /* Normal packet */
                    if (aPacketData[PACKET_NUMBER_INDEX] != (0xFFU & packets_received))
                    {
                      Serial_PutByte(NAK);
                    }
                    else
                    {
                      if (packets_received == 0)
                      {
                        /* File name packet */
                        if (aPacketData[PACKET_DATA_INDEX] != 0)
                        {
                          /* File name extraction */
                          i = 0;
                          file_ptr = aPacketData + PACKET_DATA_INDEX;
                          while ( (*file_ptr != 0) && (i < FILE_NAME_LENGTH))
                          {
                            aFileName[i++] = *file_ptr++;
                          }
      
                          /* File size extraction */
                          aFileName[i++] = '\0';
                          i = 0;
                          file_ptr ++;
                          while ( (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH))
                          {
                            file_size[i++] = *file_ptr++;
                          }
                          file_size[i++] = '\0';
                          Str2Int(file_size, &filesize);
      
                          /* Test the size of the image to be sent */
                          /* Image size is greater than Flash size */
                          if (*p_size > (sramsize + 1))
                          {
                            /* End session */
                            tmp = CA;
                            HAL_UART_Transmit(&huart2, &tmp, 1, NAK_TIMEOUT);
                            HAL_UART_Transmit(&huart2, &tmp, 1, NAK_TIMEOUT);
                            result = COM_LIMIT;
                          }
                          
                          /* erase user application area */
                          #if(0)
                          FLASH_If_Erase(sramdestination);
                          #endif
                          *p_size = filesize;
                          session_begin = 1;
      
                          Tmp++;
                          delay = 100000;
                          while (delay--);
                          
                          Serial_PutByte(ACK);
                          Serial_PutByte(CRC16);
                        }
                        /* File header packet is empty, end session */
                        else
                        {
                          Serial_PutByte(ACK);
                          file_done = 1;
                          session_done = 1;
                          break;
                        }
                      }
                      else /* Data packet */
                      {
                        ramsource = (uint32_t *)&aPacketData[PACKET_DATA_INDEX];
      
                        /* Write received data in Flash */
                        for (j=0; j<(packet_length/4); j++)
                        {
                          *(uint32_t*)sramdestination++ = *(uint32_t*)ramsource++;
                        }
      
                        Tmp++;
      
                        Serial_PutByte(ACK);
                        session_begin = 2;
                      }
                      packets_received ++;
                    }
                    break;
                }
                break;
              case HAL_BUSY: /* Abort actually */
                Serial_PutByte(CA);
                Serial_PutByte(CA);
                result = COM_ABORT;
                break;
              case HAL_ERROR:
                Serial_PutByte(CA);
                Serial_PutByte(CA);
                result = COM_ABORT;
                break;
              default:
                if (session_begin > 0)
                {
                  errors ++;
                }
                if (errors >= MAX_ERRORS)
                {
                  if (session_begin == 2)
                  {
                      /* End session */
                      Serial_PutByte(CA);
                      Serial_PutByte(CA);
                      result = COM_DATA;
                  }
                  else
                  {
                      /* Abort communication */
                      Serial_PutByte(CA);
                      Serial_PutByte(CA);
                      result = COM_ABORT;
                  }
                }
                else
                {
                  Serial_PutByte(CRC16); /* Ask for a packet */
                }
                break;
            }
        }
    }
    return result;
}

/**
  * @brief  Transmit a file using the ymodem protocol
  * @param  p_buf: Address of the first byte
  * @param  p_file_name: Name of the file sent
  * @param  file_size: Size of the transmission
  * @retval COM_StatusTypeDef result of the communication
  */
COM_StatusTypeDef Ymodem_Transmit (uint8_t *p_buf, const uint8_t *p_file_name, uint32_t file_size)
{
  uint32_t errors = 0, ack_recpt = 0, size = 0, pkt_size;
  uint8_t *p_buf_int;
  COM_StatusTypeDef result = COM_OK;
  uint32_t blk_number = 1;
  uint8_t a_rx_ctrl[2];
  uint8_t i;
#ifdef CRC16_F    
  uint32_t temp_crc;
#else /* CRC16_F */   
  uint8_t temp_chksum;
#endif /* CRC16_F */  

  /* Prepare first block - header */
  PrepareIntialPacket(aPacketData, p_file_name, file_size);

  while (( !ack_recpt ) && ( result == COM_OK ))
  {
    /* Send Packet */
    HAL_UART_Transmit(&huart2, &aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE, NAK_TIMEOUT);

    /* Send CRC or Check Sum based on CRC16_F */
#ifdef CRC16_F    
    temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_crc >> 8);
    Serial_PutByte(temp_crc & 0xFF);
#else /* CRC16_F */   
    temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_chksum);
#endif /* CRC16_F */

    /* Wait for Ack and 'C' */
    if (HAL_UART_Receive(&huart2, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK)
    {
      if (a_rx_ctrl[0] == ACK)
      {
        ack_recpt = 1;
      }
      else if (a_rx_ctrl[0] == CA)
      {
        if ((HAL_UART_Receive(&huart2, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK) && (a_rx_ctrl[0] == CA))
        {
          HAL_Delay( 2 );
          __HAL_UART_FLUSH_DRREGISTER(&huart2);
          result = COM_ABORT;
        }
      }
    }
    else
    {
      errors++;
    }
    if (errors >= MAX_ERRORS)
    {
      result = COM_ERROR;
    }
  }

  p_buf_int = p_buf;
  size = file_size;

  /* Here 1024 bytes length is used to send the packets */
  while ((size) && (result == COM_OK ))
  {
    /* Prepare next packet */
    PreparePacket(p_buf_int, aPacketData, blk_number, size);
    ack_recpt = 0;
    a_rx_ctrl[0] = 0;
    errors = 0;

    /* Resend packet if NAK for few times else end of communication */
    while (( !ack_recpt ) && ( result == COM_OK ))
    {
      /* Send next packet */
      if (size >= PACKET_1K_SIZE)
      {
        pkt_size = PACKET_1K_SIZE;
      }
      else
      {
        pkt_size = PACKET_SIZE;
      }

      HAL_UART_Transmit(&huart2, &aPacketData[PACKET_START_INDEX], pkt_size + PACKET_HEADER_SIZE, NAK_TIMEOUT);
      
      /* Send CRC or Check Sum based on CRC16_F */
#ifdef CRC16_F    
      temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], pkt_size);
      Serial_PutByte(temp_crc >> 8);
      Serial_PutByte(temp_crc & 0xFF);
#else /* CRC16_F */   
      temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], pkt_size);
      Serial_PutByte(temp_chksum);
#endif /* CRC16_F */
      
      /* Wait for Ack */
      if ((HAL_UART_Receive(&huart2, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK) && (a_rx_ctrl[0] == ACK))
      {
        ack_recpt = 1;
        if (size > pkt_size)
        {
          p_buf_int += pkt_size;
          size -= pkt_size;
          if (blk_number == (file_size / PACKET_1K_SIZE))
          {
            result = COM_LIMIT; /* boundary error */
          }
          else
          {
            blk_number++;
          }
        }
        else
        {
          p_buf_int += pkt_size;
          size = 0;
        }
      }
      else
      {
        errors++;
      }

      /* Resend packet if NAK  for a count of 10 else end of communication */
      if (errors >= MAX_ERRORS)
      {
        result = COM_ERROR;
      }
    }
  }

  /* Sending End Of Transmission char */
  ack_recpt = 0;
  a_rx_ctrl[0] = 0x00;
  errors = 0;
  while (( !ack_recpt ) && ( result == COM_OK ))
  {
    Serial_PutByte(EOT);

    /* Wait for Ack */
    if (HAL_UART_Receive(&huart2, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK)
    {
      if (a_rx_ctrl[0] == ACK)
      {
        ack_recpt = 1;
      }
      else if (a_rx_ctrl[0] == CA)
      {
        if ((HAL_UART_Receive(&huart2, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK) && (a_rx_ctrl[0] == CA))
        {
          HAL_Delay( 2 );
          __HAL_UART_FLUSH_DRREGISTER(&huart2);
          result = COM_ABORT;
        }
      }
    }
    else
    {
      errors++;
    }

    if (errors >=  MAX_ERRORS)
    {
      result = COM_ERROR;
    }
  }

  /* Empty packet sent - some terminal emulators need this to close session */
  if ( result == COM_OK )
  {
    /* Preparing an empty packet */
    aPacketData[PACKET_START_INDEX] = SOH;
    aPacketData[PACKET_NUMBER_INDEX] = 0;
    aPacketData[PACKET_CNUMBER_INDEX] = 0xFF;
    for (i = PACKET_DATA_INDEX; i < (PACKET_SIZE + PACKET_DATA_INDEX); i++)
    {
      aPacketData [i] = 0x00;
    }

    /* Send Packet */
    HAL_UART_Transmit(&huart2, &aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE, NAK_TIMEOUT);

    /* Send CRC or Check Sum based on CRC16_F */
#ifdef CRC16_F    
    temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_crc >> 8);
    Serial_PutByte(temp_crc & 0xFF);
#else /* CRC16_F */   
    temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_chksum);
#endif /* CRC16_F */

    /* Wait for Ack and 'C' */
    if (HAL_UART_Receive(&huart2, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK)
    {
      if (a_rx_ctrl[0] == CA)
      {
        HAL_Delay( 2 );
        __HAL_UART_FLUSH_DRREGISTER(&huart2);
        result = COM_ABORT;
      }
    }
  }

  return result; /* file transmitted successfully */
}

/**
  * @}
  */

/*******************(C)COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/
