#ifndef __CLI_INTERFACE_H__
#define __CLI_INTERFACE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>    // strtok 함수가 선언된 헤더 파일
#include <stdint.h>
//#include <ctype.h>     // isdigit


//------------------------------------------------
// Define List
//------------------------------------------------
#define PARSER_ERROR                        2

#define LIST_NOT_FOUND                      1
                    
#define CLI_END                             0
                    
#define CLI_MATCH                           0
                    
#define D_DELIMITER                         " "

#define NUMBER_OF_DELIMITER_VALUE           20

#define UART_BUF_MAX                        128

#define CLI_LAST_CMD                        5

#define CLI_READY                           1

#define CLI_CLEAR                           0

#define UART_BYTE                           1


//------------------------------------------------
// Enum List
//------------------------------------------------
typedef enum {
    VIEW_NONE       = 0x0000,
    VIEW_DBG_MEASE  = 0x1000,
    VIEW_DBG_RELAY  = 0x1001,
    VIEW_DBG_ETC    = 0x1002,
    VIEW_DBG_SRAM   = 0x1003,
    VIEW_DBG_FLASH  = 0x1004,
    VIEW_MAX        = 0xFFFF
} VIEW_DBG_POINT_e;


//------------------------------------------------
// ASCII
//------------------------------------------------
#define ASCII_LF                            0x0A

#define ASCII_CR                            0x0D

#define ASCII_SPACE                         0x20

#define ASCII_BACKSPACE                     0x08

#define ASCII_ESC                           0x1B

#define ASCII_CTRL_C                        0x03


#define LAST_CMD                            "!"
#if 1
#define LOWER_A                             'a'
#define LOWER_Z                             'z'
#else
#define UPPER_A                             'A'
#define UPPER_Z                             'Z'
#endif
#define CONVERT_CHAR_OFFSET                 0x20

#define REPEAT_LAST_CMD                     "*"

#define CONSOLE_SPLIT                       printf("--------------------------------------------------\r\n")


//------------------------------------------------
// Structure List
//------------------------------------------------
typedef struct 
{
    uint8_t buffer[UART_BUF_MAX];
    uint8_t rx_index;
    uint8_t rx_done;
} CLI_t;

extern CLI_t cli;


typedef int (*CBF)(int argc, char *argv[]);
typedef struct 
{
    const char *name;
    CBF   func;
    const char *description;
} CMD_LIST;


typedef struct 
{
    uint16_t dbg_set;
    uint16_t dbg_value;
} DEBUG_VIEW_t;

extern DEBUG_VIEW_t view;

extern uint8_t uart_rx_byte;

//------------------------------------------------
// Function List
//------------------------------------------------
int parser(char *cmd);


int cbf_help(int argc, char *argv[]);
int cbf_reset(int argc, char *argv[]);
int cbf_test(int argc, char *argv[]);
int cbf_xmodem(int argc, char *argv[]);
int cbf_dump(int argc, char *argv[]);
int cbf_flash_test(int argc, char *argv[]);
int cbf_app_fw_jump(int argc, char *argv[]);
int cbf_tag(int argc, char *argv[]);
int cbf_test_assert(int argc, char *argvp[]);
int cbf_event_test(int argc, char *argv[]);
int cbf_event_reset(int argc, char *argv[]);
int cbf_dbg_view(int argc, char *argv[]);


int cbf_sn(int argc, char *argv[]);
int cbf_boot_logo(int argc, char *argv[]);


#endif /* __CLI_INTERFACE_H__ */
