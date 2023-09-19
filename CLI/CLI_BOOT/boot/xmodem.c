#include "main.h"
#include "common.h"
#include "xmodem.h"
#include "ymodem.h"
#include "flash_if.h"


/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **                                                                         **
 *****************************************************************************/

/* 1K Xmodem  */
U8  gu8PacketNumber;
U32 gu32PacketSize;
U32 gu32PacketTSize;
U32 gu32Xmodem_Size;



extern UART_HandleTypeDef huart2;



/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **                                                                         **
 *****************************************************************************/

void Xmodem_InitVariable(void)
{
    /* 1K  */
    gu8PacketNumber  = 0;
    gu32PacketSize   = FALSE;
    gu32Xmodem_Size  = FALSE;
}

/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **                                                                         **
 *****************************************************************************/

U8 Xmodem_Getchar(U8 *retChar)
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


/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **                                                                         **
 *****************************************************************************/

BOOL_e XMODEM_WaitForChar(U8 *u8CPtr, U32 u32ProcessTime)
{
    U32     u32Thisms;

    ++u32ProcessTime;

    u32Thisms = uwTick;

    while (u32ProcessTime)
    {
        if (Xmodem_Getchar(u8CPtr))
        {
            return (TRUE);
        }

        if (uwTick != u32Thisms) {
            --u32ProcessTime;
            u32Thisms = uwTick;
        }
    }

    return (FALSE);
}


/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **                                                                         **
 *****************************************************************************/

BOOL_e XMODEM_GetRecord(U8 *u8DestAddress)
{
    U32         u32Size = 0;
    U16         u16CRC_check;
    U8          u8GetChar;
    BOOL_e      bGetCheck;

    u16CRC_check = 0;

    /* Packet number */
    bGetCheck = XMODEM_WaitForChar(&u8GetChar, DOWNLOAD_TIMEOUT);
    if ((!bGetCheck) || (u8GetChar != gu8PacketNumber))
    {
        return (FALSE);
    }    

    bGetCheck = XMODEM_WaitForChar(&u8GetChar, DOWNLOAD_TIMEOUT);
    if ((!bGetCheck) || (u8GetChar != (U8)~gu8PacketNumber))
    {
        return (FALSE);
    }    


    /* Get data - 128 or 1024 byte */
    for (u32Size = 0; u32Size < gu32PacketSize; ++u32Size)
    {
        if (!XMODEM_WaitForChar(&u8GetChar, DOWNLOAD_TIMEOUT))
        {
            return (FALSE);
        }

        /* CRC calculation */
        u16CRC_check = (u16CRC_check<<8) ^ crc16tab[((u16CRC_check>>8) ^ u8GetChar)&0x00FF];
        
        #if 1
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, u8DestAddress++, u8GetChar);
        #else
        *u8DestAddress++ = u8GetChar;
        #endif
    }

    /* high & low CRC bit */
    u16CRC_check &= 0xFFFF;
    bGetCheck = XMODEM_WaitForChar(&u8GetChar, DOWNLOAD_TIMEOUT);
    if ((!bGetCheck) || (u8GetChar != (U8)((u16CRC_check >> 8) & 0xFF)))
    {
        return (FALSE);
    }    

    bGetCheck = XMODEM_WaitForChar(&u8GetChar, DOWNLOAD_TIMEOUT);
    if ((!bGetCheck) || (u8GetChar != (U8)(u16CRC_check & 0xFF)))
    {
        return (FALSE);
    }    

    return (TRUE);
}


/*****************************************************************************
 **                                                                         **
 **  128 or 1024 xmodem Rx                                                  **
 **                                                                         **
 *****************************************************************************/

BOOL_e XMODEM_Rx(U32 *p_size, U8 *u8DestAddress)
{

    U32     u32State;
    U32     u32Retrynum = 10;                            
    U8      u8GetChar;
    U8      *u8StartAddress =  u8DestAddress;

    gu32PacketTSize = 0;
    gu8PacketNumber = 1;
    u32State = WAITING_START;
    
#if DBG_LED
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
#endif

    while (u32Retrynum)
    {                              
        if (u32State == WAITING_START)                 
        {
            Serial_PutByte('C');

            if (XMODEM_WaitForChar(&u8GetChar, DOWNLOAD_TIMEOUT))
            {
                /* packet head packet size  */
                if (u8GetChar == XMODEM_SOH)
                {
                    gu32PacketSize = PACKET_SIZE_SOH;
                    TransitionState(u32State, RX_PACKET);
                    #if DBG_LED
                    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
                    #endif
                }
                
                // x modem 1024byte
                if (u8GetChar == XMODEM_STX)
                {
                    gu32PacketSize = PACKET_SIZE_STX;
                    TransitionState(u32State, RX_PACKET);
                }

                if ((u8GetChar == ABORT1) || (u8GetChar == ABORT2))
                {
                    return (FALSE);
                }
            }
        }

        if (u32State == WAIT_HEAD)                      
        {
            if (!XMODEM_WaitForChar(&u8GetChar, DOWNLOAD_TIMEOUT))     
            {                                           
                Serial_PutByte(XMODEM_NAK);

                u32Retrynum--;                         
            }
            else if (u8GetChar == XMODEM_SOH)
            {
                gu32PacketSize = PACKET_SIZE_SOH;
                TransitionState(u32State, RX_PACKET);
            }
            else if (u8GetChar == XMODEM_STX)
            {
                gu32PacketSize = PACKET_SIZE_STX;
                TransitionState(u32State, RX_PACKET);
            }
            else if (u8GetChar == XMODEM_EOT) 
            {
                Serial_PutByte(XMODEM_ACK);

                *p_size = (u8DestAddress - u8StartAddress);
                //*p_size = gu32PacketTSize;
                return (TRUE);
            }

            if (u8GetChar == XMODEM_CAN) /* CANCEL */
            {
                return (FALSE);                        
            }
        }

        if (u32State == RX_PACKET)
        {
            #if DBG_LED
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
            #endif
            if (XMODEM_GetRecord(u8DestAddress))
            {
                Serial_PutByte(XMODEM_ACK);
                u8DestAddress += gu32PacketSize;
                gu32PacketTSize += gu32PacketSize;
                ++gu8PacketNumber;
                TransitionState(u32State, WAIT_HEAD);
            }
            else
            {
                Serial_PutByte(XMODEM_ACK);
                u32Retrynum--;
                TransitionState(u32State, WAIT_HEAD);
            }
        }
    }

    return (FALSE);
}


/* 1K XMODEM Upload Function */
/*****************************************************************************
 **                                                                         **
 **   Send the 1k packet                                                    **
 **                                                                         **
 *****************************************************************************/

BOOL_e XMODEM_SendRecord(U8 *u8DataAddress, S32 s32DataLength, U32 u32TypeHead)
{
    U32         u32Size;
    U16         u16CRC_check;
    U8          u8CPtr;

    u16CRC_check = 0;

    Serial_PutByte(XMODEM_SOH + u32TypeHead);    /* Head */
    Serial_PutByte(gu8PacketNumber);              /* Packet number */
    Serial_PutByte((U8)~gu8PacketNumber);

    for (u32Size = 0; u32Size < gu32PacketSize; ++u32Size)
    {
        if (DebugGetchar(&u8CPtr))                                      /* CANCEL check */
        {
            if (u8CPtr == XMODEM_CAN)   return (FALSE);
        }

        if (s32DataLength < u32Size)
        {
            u16CRC_check = (u16CRC_check<<8) ^ crc16tab[((u16CRC_check>>8) ^ 0)&0x00FF];
            Serial_PutByte(0);
        }
        else
        {
            u16CRC_check = (u16CRC_check<<8) ^ crc16tab[((u16CRC_check>>8) ^ (*u8DataAddress))&0x00FF];
            Serial_PutByte(*u8DataAddress++);
        }
    }

    Serial_PutByte(u16CRC_check>>8);         /* CRC */
    Serial_PutByte(u16CRC_check);

    return (TRUE);
}


/*****************************************************************************
 **                                                                         **
 **  1k xmodem Tx                                                           **
 **                                                                         **
 *****************************************************************************/

BOOL_e XMODEM_Tx(U8 *u8DataAddress, S32 s32DataLength) {
    U32     u32State, u32Retrynum=0;
    U32     u32TypeCheck=0, u32TypeHead=1;
    U8      u8GetChar;
    BOOL_e    bCancelCheck;

    gu8PacketNumber = 1;
    gu32PacketSize = PACKET_SIZE_STX;
    u32State = WAITING_SEND;

    while (1)
    {
        if (u32State == WAITING_SEND)
        {                 /* Packet Send start */
            Serial_PutByte('.');
            
            if (XMODEM_WaitForChar(&u8GetChar, DOWNLOAD_TIMEOUT))
            {
                if (u8GetChar == 'C')
                {
                    gu32PacketSize = PACKET_SIZE_STX;
                    u32TypeHead=1;
                    TransitionState(u32State, XMODEM_GET_ACK);
                }
            }
        }

        if (u32State == XMODEM_GET_ACK)
        {
            if (s32DataLength > 0)
            {
                bCancelCheck = XMODEM_SendRecord(u8DataAddress, s32DataLength, u32TypeHead);    /* Send Packet */
                if (!bCancelCheck)  return (FALSE);

                if (XMODEM_WaitForChar(&u8GetChar, DOWNLOAD_TIMEOUT))
                {
                    if (u8GetChar == XMODEM_ACK)    /* Send success */
                    {
                        u8DataAddress += gu32PacketSize;
                        s32DataLength -= gu32PacketSize;
                        ++gu8PacketNumber;
                        u32TypeCheck = 0;
                        TransitionState(u32State, XMODEM_GET_ACK);
                    }
                    if (u8GetChar == XMODEM_NAK)    /* Send fail */
                    {
                        u32Retrynum++;
                        u32TypeCheck++;
                        
                        if (u32Retrynum > 10)
                        {
                            Serial_PutByte(XMODEM_CAN);

                            return (FALSE);
                        }
                        
                        if ((u32TypeCheck == 3) && (gu8PacketNumber == 1))
                        {
                            gu32PacketSize = PACKET_SIZE_SOH;
                            u32TypeHead=0;
                        }

                        TransitionState(u32State, XMODEM_GET_ACK);
                    }
                    
                    if (u8GetChar == XMODEM_CAN)    /* Cancel */
                    {
                        return (FALSE);
                    }
                }
            }
            else
            {
                Serial_PutByte(XMODEM_EOT);          /* End */

                if (XMODEM_WaitForChar(&u8GetChar, DOWNLOAD_TIMEOUT))
                {
                    if (u8GetChar == XMODEM_ACK)
                    {
                        return (TRUE);
                    }

                    if (u8GetChar == XMODEM_CAN)    /* Cancel */
                    {
                        return (FALSE);
                    }
                }
            }
        }
    }
}

