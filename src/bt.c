/******************************************************************************
 *
 * Copyright:
 *    (C) 2006 Embedded Artists AB
 *
 * File:
 *    bt.c
 *
 * Description:
 *    Demonstrate communication with the Philips Bluetooth module, BGB203-S06
 *
 *****************************************************************************/
 
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <stdlib.h>
#include <string.h>
#include "lcd.h"
#include "key.h"
#include "uart.h"
#include "hw.h"
#include "select.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/
#define RECV_BUF_LEN 32
#define PROC_BT_STACK_SIZE 800

#define BT_BACKGROUND_COLOR     0x01
#define BT_BACKBACKGROUND_COLOR 0x03

#define MAX_BT_UNITS 5

typedef struct
{
  tU8 active;
  tU8 btAddress[13];
  tU8 btName[17];
} tBtRecord;


/*****************************************************************************
 * Local variables
 ****************************************************************************/
static tU8 procBtStack[PROC_BT_STACK_SIZE];
static tU8 pidBt;

static volatile tBool stopRecvProc = FALSE;
static tCntSem recvSem;

static tU8 btCursor = 0;

static tBool btSleepState = FALSE;
static tBool btCommandMode = FALSE;

static tU8 localName[17];
static tU8 btAddress[13];

static tBtRecord foundBtUnits[MAX_BT_UNITS];


/*****************************************************************************
 * External variables
 ****************************************************************************/
extern volatile tU32 ms;


/*****************************************************************************
 * Local function prototype
 ****************************************************************************/
static void procBt(void* arg);
static void btSetMode(tBool commandMode);


/*****************************************************************************
 *
 * Description:
 *    Initialize and start a process handling the BGB203-S06 communication
 *    (over UART channel #1).
 *
 ****************************************************************************/
void
initBtProc(void)
{
  tU8 error;
  
  osSemInit(&recvSem, 1);

  osCreateProcess(procBt, procBtStack, PROC_BT_STACK_SIZE, &pidBt, 4, NULL, &error);
  osStartProcess(pidBt, &error);
}


/*****************************************************************************
 *
 * Description:
 *    A process entry function - the BGb203-S06 handling process
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
procBt(void* arg)
{
  tU8 error;

  //reset BGB203 modules
  resetBT(TRUE);
  osSleep(2);
  resetBT(FALSE);
  osSleep(5);

  //extra sleep
  osSleep(50);
  
  //initialize uart #1: 115200 kbps, 8N1, FIFO
  initUart1(B115200((CORE_FREQ) / PBSD), UART_8N1, UART_FIFO_16);

  /***************************************************************************
   * Startup command sequence
   *   This command sequence is just used for production testing
   **************************************************************************/
  osSleep(50);
  uart1SendString("+++");
  osSleep(50);
  uart1SendString("ATI\r");
  osSleep(50);
  uart1SendString("AT+BTLNM\r");
  
  osSleep(100);
  uart1SendString("AT+BTBDA\r");
  osSleep(100);
  uart1SendString("AT+BTINQ=5\r");
  osSleep(600);

  //Switch to server mode where other BT devices can detect this unit
  //during an 'inquiry'
  uart1SendString("AT+BTSRV=1\r");

  /***************************************************************************
   * Loop forever and create a terminal directly between the
   * USB serial port and the BT module
   **************************************************************************/
  while(1)
  {
    osSemTake(&recvSem, 0, &error);

    while (stopRecvProc == FALSE)
    {
      tU8 rxChar;
      tU8 nothing;


      nothing = TRUE;

      //check if any character has been received from terminal
      if (consolGetChar(&rxChar) == TRUE)
      {
        uart1SendChar(rxChar);
        nothing = FALSE;
      }

      //check if any character has been received from BT
      if (uart1GetChar(&rxChar) == TRUE)
      {
        printf("%c", rxChar);
        nothing = FALSE;
      }
        
      if (nothing == TRUE)
        osSleep(1);
    }
    stopRecvProc = FALSE;
  }
}