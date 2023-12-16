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
#define EVENT_MEMORY_START_ADDR     0x08060000UL

EVENT_MANAGE_t *event_manage = (EVENT_MANAGE_t *)EVENT_MEMORY_START_ADDR;
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
void SAVE_FLASH_EVENT_LOG(void)
{
    void *cpy_data = &event_main.index;
    uint32_t front =  event_main.index;
    uint32_t rear  = 0;

    FLASH_If_Init();
    FLASH_If_Erase(event_manage);
    HAL_FLASH_Unlock();
    do {
        if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, &event_manage->index, *(uint32_t*)cpy_data)) {
            printf("Flash Write Error\r\n");
            return;
        }
        event_manage->index++;
        (uint32_t *)cpy_data++;
        rear++;
        printf("idx 0x%08x, cpy 0x%08x, cnt %d \r\n", &event_manage->index, cpy_data, rear);
    } while (rear != front) ;

    HAL_FLASH_Lock();
}

void PRINT_FLASH_EVENT_LOG(void)
{
    if (event_manage->index >= EVENT_LIST_MAX) {
        printf("EVENT Index Over\r\n");
        return;
    }

    printf("[EVENT]\t\t\t [Index]\r\n");
    for (uint32_t cnt = 0; cnt <= event_manage->index; cnt++) {
        printf(" %s\t\t\t %u\r\n", gpEVENT_tb_NAME_LIST[event_manage->type[cnt]], cnt);
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
        printf("event_main.index 0x%08lx, value : %lu\r\n", (uint32_t)&event_main.index, event_main.index);
        printf("event_main.type  0x%08lx, value : %lu\r\n", (uint32_t)&event_main.type[event_main.index - INDEX_OFFSET], event_main.type[event_main.index - INDEX_OFFSET]);
    }

    #if _USE_FLASH_
    SAVE_FLASH_EVENT_LOG();
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
        printf("%-28s %-14lu %lu\r\n", gpEVENT_tb_NAME_LIST[event_main.type[cnt]], cnt, event_main.type[cnt]);
    }
    CONSOLE_SPLIT;
}

void RESET_EVENT_LOG(void)
{
    memset(&event_main, 0, sizeof(EVENT_MANAGE_t));

    #if 1 // _USE_FLASH_
    FLASH_If_Erase((uint32_t)event_manage);

    SAVE_SRAM_EVENT_LOG(EVENT_LOG_RESET);
    #endif
}
