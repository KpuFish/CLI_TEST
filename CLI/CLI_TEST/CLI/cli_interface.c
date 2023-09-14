
#include "usart.h"
#include "cli_interface.h"





//----------------------------------------
// UART PRINTF
//----------------------------------------

uint8_t uart_rx_byte = 0;

#if 0
#ifdef __cplusplus
extern "C" int _write(int32_t file, uint8_t *ptr, int32_t len) {
#else
int _write(int32_t file, uint8_t *ptr, int32_t len) {
#endif
    if( HAL_UART_Transmit(&huart2, ptr, len, len) == HAL_OK ) return len;
    else return 0;
}
#endif

#if 0
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

#if 1
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
    {"?"            , cbf_help      }, 
    {"RESET"        , cbf_reset     }, // software reset for entering the bootloader
    {"MODEL?"       , cbf_boot_logo },
    {"SN?"          , cbf_sn        },
    {"TEST"         , cbf_test      },
    {(char*)NULL    , (CBF)NULL     }
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
    printf("┃╭━━╋┫━━┫╭╮┃///*%s\r\n", version_info);
    printf("┃┃//┃┣━━┃┃┃┃///*%s\r\n", model_info);
    printf("╰╯//╰┻━━┻╯╰╯\r\n");

    #else // TYPE 2
    printf("  *%s\r\n", version_info);
    printf("  *%s\r\n", model_info);
    #endif
    CONSOLE_SPLIT;
    printf(" $Fish >> ");
    return 0;
}

int cbf_sn(int argc, char *argv[])
{
    printf("SN : %06d\r\n", model_sn);
    return 0;
}

int cbf_help(int argc, char *argv[])
{
    CONSOLE_SPLIT;
    printf("Command List \r\n");
    CONSOLE_SPLIT;
    for (int cnt = 0; cmd_list[cnt].name != NULL; cnt++) {
        printf("%s \r\n", cmd_list[cnt].name);
    }
    return 0;
}

int cbf_reset(int argc, char *argv[])
{
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
