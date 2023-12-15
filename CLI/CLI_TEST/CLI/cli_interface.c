
#include "usart.h"
#include "cli_interface.h"
#include "system_config.h"
#include "menu.h"
#include "event_log.h"


//----------------------------------------
// UART PRINTF
//----------------------------------------

uint8_t uart_rx_byte = 0;

#define RETARGET_1 0
#define RETARGET_2 0
#define RETARGET_3 1

#if RETARGET_1
#ifdef __cplusplus
extern "C" int _write(int32_t file, uint8_t *ptr, int32_t len) {
#else
int _write(int32_t file, uint8_t *ptr, int32_t len) {
#endif
    if( HAL_UART_Transmit(&huart2, ptr, len, len) == HAL_OK ) return len;
    else return 0;
}
#endif

#if RETARGET_2
int _write( int32_t file , uint8_t *ptr , int32_t len )
{
    /* Implement your write code here, this is used by puts and printf for example */
    for ( int16_t i = 0 ; i < len ; ++i )
    {
        HAL_UART_Transmit( &huart2, ptr++, 1, 1000);
    }
    return len;
}
#endif

#if RETARGET_3
int __io_putchar(int ch)
{
    #if USE_TX_DMA
	HAL_UART_Transmit_DMA( &huart2, (uint8_t *)&ch, 1);
    #else
    HAL_UART_Transmit( &huart2, (uint8_t *)&ch, 1, 100);
    #endif
	return ch;
}
#endif


//----------------------------------------
// CLI Def
//----------------------------------------
CLI_t cli;


DEBUG_VIEW_t view;


//static char *last_command;


/* -------------------------
 * CLI LIST
 * -------------------------
 * (1) - Register here CallBack Function that you want to be added
 * (2) - Define callback function
 * (3) - Fill in the callback function
 */
const CMD_LIST cmd_list[] =
{
    {"?"            , cbf_help             , "Show All Command"                 }, 
    {"RESET"        , cbf_reset            , "System Reboot"                    },
    {"MODEL?"       , cbf_boot_logo        , "Check Model Info"                 },
    {"SN?"          , cbf_sn               , "Check SN Info"                    },
    {"TEST"         , cbf_test             , "Test CLI Arguments"               },
    {"X"            , cbf_xmodem           , "F/W Download"                     },
    {"DUMP"         , cbf_dump             , "Dump Memory"                      },
    {"FLASH_TEST"   , cbf_flash_test       , "Test Flash WR"                    },
    //{"JUMP"         , cbf_app_fw_jump      , "Jump from Bootloader to Main App" },
    {"TAG"          , cbf_tag              , "Check Tag Info"                   },
    {"ASSERT"       , cbf_test_assert      , "Test Assert"                      },
    {"EVENT?"       , cbf_event_print      , "Print Event Log"                  },
    {"EVENT_TEST"   , cbf_event_test       , "Test Event Log"                   },
    {"EVENT_RESET"  , cbf_event_reset      , "Reset Event Log"                  },
    {"DBG"          , cbf_dbg_view         , "View Dbg Message"                 },

    {(char*)NULL    , (CBF)NULL            , (char*)NULL                        }
};


//----------------------------------------
// CLI UART CALLBACK for stm32f
//----------------------------------------
/* USER CODE BEGIN 1 */
void Convert_Char(uint8_t *byte)
{
    #if 1 // LOW to UP
    if ((*byte >= LOWER_A) && (*byte <= LOWER_Z)) {
        *byte -= CONVERT_CHAR_OFFSET;
    }
    #else
    if ((*byte >= UPPER_A) && (*byte <= UPPER_Z)) {
        *byte += CONVERT_CHAR_OFFSET;
    }
    #endif
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART2)
  {
        // echo back test
        //HAL_UART_Transmit(&huart2, &uart_rx_byte, UART_BYTE, UART_TIME_OUT);
        if (uart_rx_byte == ASCII_LF || uart_rx_byte == ASCII_CR) {
            cli.rx_done  = CLI_READY;
        }
         else if (uart_rx_byte == ASCII_BACKSPACE) {
            if (cli.rx_index > 0) {
                cli.buffer[--cli.rx_index] = 0;
                printf(" %c",  ASCII_BACKSPACE);
            } else {
                printf(" ");
            }
        }
         else {
            Convert_Char(&uart_rx_byte);
            cli.buffer[cli.rx_index] = uart_rx_byte;
            cli.rx_index = (cli.rx_index + 1) % UART_BUF_MAX;
        }
        // uart rxne pending clear
        HAL_UART_Receive_IT(&huart2, &uart_rx_byte, UART_BYTE);
  }
}
//----------------------------------------


/* CLI PARSER */
int parser(char *cmd)
{
    int    argc = 0;
    char  *argv[NUMBER_OF_DELIMITER_VALUE];
    
#if USE_LAST_CMD
    static char *last_cmd;
#endif

    if (cmd == NULL) {
        printf("CMD Error\r\n");
    }
    
	//----------------------------------------
    // SPLIT THE UART RX STRING
    //----------------------------------------
    argv[argc++] = strtok(cmd, D_DELIMITER);

    while (1) {
        argv[argc] = strtok(NULL, D_DELIMITER);
        
        if (argv[argc] == NULL) {
            break;
        }
        argc++;
    }

#if USE_LAST_CMD
    if (strcmp(argv[0], REPEAT_LAST_CMD) == CLI_MATCH) {
        strcpy(argv[0], last_cmd);
    }
#endif

    //----------------------------------------
    // FIND THE MATCHED STRING
    //----------------------------------------
    for (int cnt = 0; cmd_list[cnt].name != NULL; cnt++) {
        if (strcmp(cmd_list[cnt].name, argv[0]) == CLI_MATCH) {
            cmd_list[cnt].func(argc, argv);
        }
    }

#if USE_LAST_CMD
    // Last Commad Copy
    strcpy(last_cmd, argv[0]);
#endif

    memset(&cli, 0x0, sizeof(CLI_t));
    printf(" $Fish >> ");
    
#if USE_LAST_CMD
    if (strcmp(last_command, LAST_CMD) == CLI_MATCH) {
        printf("%s", last_command);
    }
#endif

    return LIST_NOT_FOUND;
}




//----------------------------------------
// CALL BACK FUNCTION
//----------------------------------------
int cbf_boot_logo(int argc, char *argv[])
{
    CONSOLE_SPLIT;
    #if 1 // TYPE 1
    printf("╭━━━╮///╭╮/////\r\n");
    printf("┃╭━━╯///┃┃/////\r\n");
    printf("┃╰━━┳┳━━┫╰━╮///\r\n");
    printf("┃╭━━╋┫━━┫╭╮┃///*%s\r\n", tag->fw_name);
    printf("┃┃//┃┣━━┃┃┃┃///\r\n"); // *%s\r\n", ); //tag->fw_date);
    printf("╰╯//╰┻━━┻╯╰╯\r\n");
    #else // TYPE 2
    printf("  *%s\r\n", tag->fw_name);
    printf("  *%s\r\n", tag->fw_date);
    #endif
    CONSOLE_SPLIT;
    printf(" $Fish >> ");
    return 0;
}

int cbf_sn(int argc, char *argv[])
{
    printf("SN : %s\r\n", (int)tag->fw_sn);
    return 0;
}

int cbf_help(int argc, char *argv[])
{
    CONSOLE_SPLIT;
    printf("Command List %-6s Description\r\n", "||");
    CONSOLE_SPLIT;
    for (int cnt = 1; cmd_list[cnt].name != NULL; cnt++) {
        printf("%-20s", cmd_list[cnt].name);
        printf("%-30s\r", cmd_list[cnt].description);
    }
    return 0;
}

int cbf_reset(int argc, char *argv[])
{
    // software reset
    HAL_NVIC_SystemReset();
    return 0;
}

int cbf_test(int argc, char *argv[])
{
	printf("argv : %s \r\n", argv[0]);
	//printf("argc : %d , argv : %s \r\n", argc, argv[1]);
	printf("TEST \r\n");
	return 0;
}


int cbf_xmodem(int argc, char *argv[])
{
    uint32_t x_modem_size = 0;
    // f/w update using uart polling
    HAL_NVIC_DisableIRQ(USART2_IRQn);

    #if 0
    if (HAL_FLASH_Unlock() != HAL_OK) {
        printf("Flash Unlcok failed\r\n");
        return 0;
    }

    FLASH_If_Init();
    #else
    FLASH_If_Erase(FLASH_BASE_MAIN_APP);
    #endif

    // entering x-modem ...
    uint8_t ret = XMODEM_Rx((uint32_t*)&x_modem_size, (uint32_t *)FLASH_BASE_MAIN_APP);
    
    HAL_FLASH_Lock();

    printf("\r\n");
    CONSOLE_SPLIT;
    if (ret == FALSE) {
        printf("X-Modem Failed\r\n");

    } else {
        printf("X-Modem Completed size : %d byte\r\n", (int)x_modem_size);
    }
    CONSOLE_SPLIT;

    // resetting uart isr
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    return 0;
}


#define FLASH_RANGE_START   0x08000000
#define FLASH_RANGE_END     0x080F0000
#define LINE				4
#define CHAR_SPACE          ' '
#define CHAR_z              'z'
#define CHAR_dot            '.'
#define ASCII_CHAR_DUMP     0
int cbf_dump(int argc, char *argv[])
{
    uint32_t size  = atoi(argv[2]);
    volatile uint32_t *addr = (volatile uint32_t *) strtol(argv[1], NULL, 16);

    #if ASCII_CHAR_DUMP
    uint8_t buffer[LINE] = { 0, };
    #endif

    if (addr < (uint32_t*)FLASH_RANGE_START || addr > (uint32_t*)FLASH_RANGE_END) {
        printf("Flash Range is 0x%08x ~ 0x%08x\r\n", FLASH_RANGE_START, FLASH_RANGE_END);
        return 0;
    }

    CONSOLE_SPLIT;
    printf("Base Addrr // dump data ... \n");
    CONSOLE_SPLIT;
    printf("0x%08x : ", (uint32_t *)addr);
    
    for (uint16_t range = 1; range <= size; range++) {
        #if ASCII_CHAR_DUMP
        if (( *(uint8_t*)addr > CHAR_SPACE) && ( *(uint8_t*)addr <= CHAR_z)) {
            buffer[range - 1] = *(uint8_t*)addr;  
        } else { 
            buffer[range - 1] = CHAR_dot;
        }
        #endif

        printf("%04x\t",  *(uint32_t *)addr++);
        if (range % LINE == 0) {
            #if ASCII_CHAR_DUMP
            printf("\t: %s", buffer);
            #endif
            printf("\r\n");
            printf("0x%08x : ", (uint32_t *)addr);
        }
    }
    printf("\r\n");
    return 0;
}

int cbf_flash_test(int argc, char *argv[])
{
    volatile uint32_t *flash_addr = (volatile uint32_t *) strtol(argv[1], NULL, 16);
    uint32_t addr = (uint32_t)flash_addr;
    uint32_t data = atoi(argv[2]);
    
    #if 0
    if (HAL_FLASH_Unlock() != HAL_OK) {
        printf("Flash Unlcok failed\r\n");
        return 0;
    }

    FLASH_If_Init();
    #else
    FLASH_If_Erase(addr);
    #endif

    #if 1
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)flash_addr, data);
    #else
    *flash_addr = data;
    #endif

    HAL_FLASH_Lock();
    
    printf("0x%08x - 0x%08x\r\n", flash_addr, *flash_addr);
    return 0;
}

#define VCCTOR_TABLE_OFFSET     4
int cbf_app_fw_jump(int argc, char *argv[])
{
    printf("Start...\r\n");
    pFunction Jump_To_Application;

    volatile uint32_t JumpAddress = *(__IO uint32_t*) (FLASH_BASE_MAIN_APP + VCCTOR_TABLE_OFFSET);
    /* Jump to user application */
    Jump_To_Application = (pFunction) JumpAddress;
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) FLASH_BASE_MAIN_APP);
    Jump_To_Application(); 
    return 0;
}

int cbf_tag(int argc, char *argv[])
{
    CONSOLE_SPLIT;
    printf("FW Info\t\t\t%s\r\n", tag->fw_name);
    printf("FW Date\t\t\t%s\r\n", tag->fw_date);
    printf("FW SN\t\t\t%s\r\n", tag->fw_sn );
    printf("FW Version\t\t%s\r\n", tag->fw_version );
    printf("FW Compiled date\t%s\r\n", tag->fw_compile_data);
    printf("FW Compiled time\t%s\r\n", tag->fw_compile_time);
    CONSOLE_SPLIT;
    return 0;
}

int cbf_test_assert(int argc, char *argv[])
{
	#if 0
	int ret = atoi(argv[1]);
    assert_param(ret);
	#endif
    return 0;
}

int cbf_event_print(int argc, char *argv[])
{
    PRINT_SRAM_EVENT_LOG();
    return 0;
}

int cbf_event_test(int argc, char *argv[])
{
    EVENT_TYPE_e type = atoi(argv[1]);
    if ((type < EVENT_LOG_RESET) || (type >= EVENT_MAX)) {
        return FALSE;
    }

    //printf("type : %d \r\n", type);
    SAVE_SRAM_EVENT_LOG(type);
    return 0;
}

int cbf_event_reset(int argc, char *argv[])
{
    RESET_EVENT_LOG();
    printf("Event Reset Completed\r\n");
    return 0;
}

int cbf_dbg_view(int argc, char *argv[])
{
    VIEW_DBG_POINT_e view_list[] = 
    {
        VIEW_NONE     ,
        VIEW_DBG_MEASE,
        VIEW_DBG_RELAY,
        VIEW_DBG_ETC  ,
        VIEW_DBG_SRAM ,
        VIEW_DBG_FLASH,
        VIEW_MAX
    };
    
    const uint8_t max_size = sizeof(view_list) / sizeof(view_list[0]);
    uint32_t view_point = (uint16_t *) strtol(argv[1], NULL, 16);
    
    if (view_point <= VIEW_NONE || view_point >= VIEW_MAX) {
        CONSOLE_SPLIT;
        printf("Invalid View Point\r\n");        
        printf("VIEW POINT HEX is ... \r\n");
        CONSOLE_SPLIT;
        for (uint8_t cnt = 1; cnt < max_size - 1; cnt++) {
            printf("0x%08x\r\n", view_list[cnt]);
        }
        CONSOLE_SPLIT;
        return 0;
    }
    
    view.dbg_value = (uint16_t)view_point;

    CONSOLE_SPLIT;
    printf("view_point is 0x%04x\r\n", view.dbg_value);
    CONSOLE_SPLIT;

    return 0;
}
