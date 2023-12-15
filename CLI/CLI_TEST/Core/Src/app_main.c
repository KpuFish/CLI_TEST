#include "main.h"
#include "cli_interface.h"
#include "system_config.h"
#include "event_log.h"


//-----------------------------------------------------
// Checking main app fw jumped
//-----------------------------------------------------
#define MAIN_APP          1
//-----------------------------------------------------
#define TIME_SEC         10000



LED_STATE_e gLED = kLED_OFF;
static void STATE_LED(void);



void app_main(void)
{
	volatile uint32_t time = 0;
    #if !MAIN_APP
    volatile uint32_t sec = 3;
    time = HAL_GetTick();
    do {
        // key detection
        if (cli.rx_done == CLI_READY) {
        break;
        }

        // Check Time sec
        if (HAL_GetTick() - time >= TIME_1SEC) {
        time = HAL_GetTick();
        printf("%d\r\n", sec--);

        // time over & jump main app
        if (sec <= 0) {
            cbf_app_fw_jump(0, NULL);
        }
        }
    } while (1);
    #endif

    //-------------------------------
    // Event System Log Start ...
    //-------------------------------
    RESET_EVENT_LOG();
    #if _USE_FLASH_
    COPY_EVENT_LOG(EVENT_SRAM);
    #endif
    SAVE_SRAM_EVENT_LOG(EVENT_BOOT);
    //-------------------------------
    
    while (1)
    {
        if (cli.rx_done == CLI_READY) {
            cli.rx_done = CLI_CLEAR;
            parser((char *) &cli.buffer[0]);
        }

        #if MAIN_APP
        if (HAL_GetTick() - time >= TIME_SEC) {
            time = HAL_GetTick();
            #if DBG_LED
            STATE_LED();
            #endif
        }
        #endif
    }
}

#if DBG_LED
static void STATE_LED(void)
{
    switch (gLED) {
        case kLED_ON   : 
            gLED = kLED_OFF;
            LED_OFF();
            SAVE_SRAM_EVENT_LOG(EVENT_LED_OFF);        
        break;
        
        case kLED_OFF  : 
        case kLED_NONE :
        default        :  
            gLED = kLED_ON;
            LED_ON();
            SAVE_SRAM_EVENT_LOG(EVENT_LED_ON);
        break;
    }
}
#endif

