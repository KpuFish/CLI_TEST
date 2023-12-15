#include <stdio.h>
#include <string.h>

#include "main.h"
#include "event_log.h"
#include "cli_interface.h"
#include "../boot/flash_if.h"



//----------------------------------------------
// STM32F401RE Flash
//----------------------------------------------
// Sector 7 0x0806 0000 - 0x0807 FFFF 128 Kbytes
//----------------------------------------------
#define EVENT_MEMORY_START_ADDR     (volatile uint32_t *)0x08060000U

EVENT_MANAGE_t *event_manage = EVENT_MEMORY_START_ADDR;
EVENT_MANAGE_t  event_main;


#if _USE_CRC16_
const uint16_t crc[] =
{

}
#endif


//----------------------------------------------
// Event Name Table
//----------------------------------------------
const char *gpEVENT_tb_NAME_LIST[EVENT_MAX] =
{
    "None",
    "SYSTEM LOG Reset",
    "SYSTEM BOOTING ON",
    "SYSTEM LED ON",
    "SYSTEM LED OFF"
};


#if _USE_FLASH_
static void FLASH_WORD_WRITE(uint32_t address, uint64_t data)
{
    FLASH_If_Erase(event_manage);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
    HAL_FLASH_Lock();
}
#endif

void COPY_EVENT_LOG(EVENT_MEM_TYPE_e type)
{
    switch (type) {
        case EVENT_SRAM  : memcpy(&event_main, event_manage, sizeof(EVENT_MANAGE_t)); break;
        case EVENT_FLASH : memcpy(event_manage, &event_main, sizeof(EVENT_MANAGE_t)); break;
        default : break;
    }
    SAVE_SRAM_EVENT_LOG(EVENT_LOG_RESET);
}

#if _USE_FLASH_
void SAVE_FLASH_EVENT_LOG(EVENT_TYPE_e event)
{
    const uint8_t INDEX_OFFSET = 1;

    if ((event < EVENT_LOG_RESET) || (event >= EVENT_MAX)) {
        return;
    }
    
    FLASH_If_Erase(event_manage);

    //HAL_FLASH_Unlock();
    
    #if 0
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
    FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
    #endif

    if (event_manage->index >= EVENT_LIST_MAX) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, &event_manage->index, 0);
    }

    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, &event_manage->index, event_main.index);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, &event_manage->type[event_manage->index], event_main.type[event_main.index]);
    HAL_FLASH_Lock();

    if (VIEW_DBG_FLASH & view.dbg_value) {
        printf("Flash ADDR DBG\r\n");
        printf("event_manage->index 0x%08x, value : %d\r\n", &event_manage->index, event_manage->index);
        printf("event_manage->type  0x%08x, value : %d\r\n", &event_manage->type[event_manage->index - INDEX_OFFSET], event_manage->type[event_manage->index - INDEX_OFFSET]);
    }
}

void PRINT_FLASH_EVENT_LOG(void)
{
    if (event_manage->index >= EVENT_LIST_MAX) {
        printf("EVENT Index Over\r\n");
        return;
    }

    printf("[EVENT]\t\t\t [Index]\r\n");
    for (uint32_t cnt = 0; cnt <= event_manage->index; cnt++) {
        printf(" %s\t\t\t %d\r\n", gpEVENT_tb_NAME_LIST[event_manage->type[cnt]], cnt);
    }
}
#endif

void SAVE_SRAM_EVENT_LOG(EVENT_TYPE_e event)
{
    const uint8_t INDEX_OFFSET = 1;
    if ((event < EVENT_LOG_RESET) || (event >= EVENT_MAX)) {
        return;
    }
    
    event_main.type[event_main.index] = event;
    event_main.index = (event_main.index + 1) % EVENT_LIST_MAX;

    if (VIEW_DBG_SRAM & view.dbg_value) {
        printf("SRAM ADDR DBG\r\n");
        printf("event_main.index 0x%08x, value : %d\r\n", &event_main.index, event_main.index);
        printf("event_main.type  0x%08x, value : %d\r\n", &event_main.type[event_main.index - INDEX_OFFSET], event_main.type[event_main.index - INDEX_OFFSET]);
    }

    #if _USE_FLASH_
    SAVE_FLASH_EVENT_LOG(event);
    #endif
}

void PRINT_SRAM_EVENT_LOG(void)
{
    if (event_main.index >= EVENT_LIST_MAX) {
        event_main.index = 0;
        //printf("EVENT Index Over\r\n");
    }
    CONSOLE_SPLIT;
    printf("[EVENT]\t\t\t [Index]\t[Type Number]\r\n");
    CONSOLE_SPLIT;
    for (uint32_t cnt = 0; cnt < event_main.index; cnt++) {
        printf("%-28s %-14d %d\r\n", gpEVENT_tb_NAME_LIST[event_main.type[cnt]], cnt, event_main.type[cnt]);
    }
    CONSOLE_SPLIT;
}

void RESET_EVENT_LOG(void)
{
    memset(&event_main, 0, sizeof(EVENT_MANAGE_t));

    #if 1 // _USE_FLASH_
    FLASH_If_Erase(event_manage);

    SAVE_SRAM_EVENT_LOG(EVENT_LOG_RESET);
    #endif
}
