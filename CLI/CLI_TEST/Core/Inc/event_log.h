#ifndef __EVENT_LOG_H__
#define __EVENT_LOG_H__

//------------------------------------------------
// Define List
//------------------------------------------------
#define EVENT_LIST_MAX      1024

#define _USE_FLASH_         0
#define _USE_CRC16_         0


//------------------------------------------------
// Enum List
//------------------------------------------------
typedef enum
{
    EVENT_SRAM,
    EVENT_FLASH,
    EVENT_NONE
} EVENT_MEM_TYPE_e;

typedef enum 
{
    EVENT_INVALID,
    EVENT_LOG_RESET,
    EVENT_BOOT,
    EVENT_LED_ON,
    EVENT_LED_OFF,
    EVENT_MAX
} EVENT_TYPE_e;


//------------------------------------------------
// Structure List
//------------------------------------------------
typedef struct 
{
    uint32_t index;
    uint32_t type[EVENT_LIST_MAX];
    //const char *name[EVENT_LIST_MAX];
    uint16_t crc;
} EVENT_MANAGE_t;


//------------------------------------------------
// Extern List
//------------------------------------------------
extern EVENT_MANAGE_t *event_manage;


//------------------------------------------------
// Function List
//------------------------------------------------
void RESET_EVENT_LOG(void);
void SAVE_EVENT_LOG(EVENT_TYPE_e event);
void PRINT_EVENT_LOG(void);
void SAVE_SRAM_EVENT_LOG(EVENT_TYPE_e event);
void PRINT_SRAM_EVENT_LOG(void);

#endif /* __EVENT_LOG_H__ */
