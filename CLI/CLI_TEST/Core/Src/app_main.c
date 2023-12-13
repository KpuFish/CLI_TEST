#include "main.h"
#include "cli_interface.h"
#include "system_config.h"
#include "event_log.h"


//-----------------------------------------------------
// Checking main app fw jumped
//-----------------------------------------------------
#define MAIN_APP          1
//-----------------------------------------------------
#define TIME_1SEC         1000


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

    RESET_EVENT_LOG();
    COPY_EVENT_LOG(EVENT_SRAM);
    SAVE_SRAM_EVENT_LOG(EVENT_BOOT);
    
    while (1)
    {
        /* USER CODE END WHILE */
        if (cli.rx_done == CLI_READY) {
            cli.rx_done = CLI_CLEAR;
            parser((char *) &cli.buffer[0]);
        }

        #if MAIN_APP
        // for checking main app fw
        if (HAL_GetTick() - time >= TIME_1SEC) {
            time = HAL_GetTick();
            #if DBG_LED
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
            #endif
        }
        #endif
    }
}
