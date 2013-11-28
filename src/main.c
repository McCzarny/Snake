/******************************************************************************
 *
 * Copyright:
 *    (C) 2006 Embedded Artists AB
 *
 * File:
 *    main.c
 *
 * Description:
 *    Main function of the "LPC2104 Color LCD Game Board with Bluetooth"
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

#include "lcd.h"
#include "key.h"
#include "uart.h"
#include "snake.h"
#include "bt.h"
#include "hw.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/
#define PROC1_STACK_SIZE 800
#define INIT_STACK_SIZE  600


/*****************************************************************************
 * Global variables
 ****************************************************************************/
volatile tU32 ms;


/*****************************************************************************
 * Local variables
 ****************************************************************************/
static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;

static tU8 contrast = 56;
static tU8 cursor   = 0;

tU8 memblock[] = {'W', 'i', 'a', 'd', 'o', 'm', 'o', 's', 'c', ' ', 'w', 'y', 's', 'l', 'a', 'n', 'a', ' ', 'z', ' ', 'G','a','m','e','b','o','a','r','d','a'};
int memblockSize = 30;


/*****************************************************************************
 * Local prototypes
 ****************************************************************************/
static void proc1(void* arg);
static void initProc(void* arg);


/*****************************************************************************
 *
 * Description:
 *    The first function to execute 
 *
 ****************************************************************************/
int
main(void)
{
  tU8 error;
  tU8 pid;
  
  osInit();

  //immediate initilaizeation of hardware I/O pins
  immediateIoInit();
  
  osCreateProcess(initProc, initStack, INIT_STACK_SIZE, &pid, 1, NULL, &error);
  osStartProcess(pid, &error);
  
  osStart();
  return 0;
}


/*****************************************************************************
 *
 * Description:
 *    Draw cursor in main menu
 *
 * Params:
 *    [in] cursor - Cursor position
 *
 ****************************************************************************/
static void
drawMenuCursor(tU8 cursor)
{
  tU32 row;

  for(row=0; row<4; row++)
  {
    lcdGotoxy(18,52+(14*row));
    if(row == cursor)
      lcdColor(0x00,0xe0);
    else
      lcdColor(0x00,0xfd);
    
    switch(row)
    {
      case 0: lcdPuts("Snake/TRON"); break;
	  case 1: lcdPuts("Send score"); break;
      default: break;
    }
  }
}


/*****************************************************************************
 *
 * Description:
 *    Draw main menu
 *
 ****************************************************************************/
static void
drawMenu(void)
{
  lcdColor(0,0);
  lcdClrscr();

  //lcdRect(14, 0, 102, 128, 0x6d);
  //lcdRect(15, 17, 100, 110, 0);
  lcdRect(14, 32, 102, 64, 0x6d);
  lcdRect(15, 49, 100, 46, 0);

  lcdGotoxy(48,33);
  lcdColor(0x6d,0);
  lcdPuts("MENU");
  drawMenuCursor(cursor);
}


/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc1(void* arg)
{
  //shortly bleep with the buzzer and flash with the LEDs
  tU8 i,j;

  for(i=0; i<3; i++)
  {
    for(j=0; j<3; j++)
    {
      setBuzzer(TRUE);
      setLED(LED_GREEN, FALSE);
      setLED(LED_RED,   TRUE);
      osSleep(1);

      setBuzzer(FALSE);
      setLED(LED_GREEN, TRUE);
      setLED(LED_RED,   FALSE);
      osSleep(1);
    }
    osSleep(10);
  }
  setLED(LED_GREEN, FALSE);
  setLED(LED_RED,   FALSE);

  //display startup message
  resetLCD();
  lcdInit();


  //print menu
  drawMenu();
  
  for(;;)
  {
    tU8 anyKey;
    static tU8 i = 0;

    anyKey = checkKey();
    if (anyKey != KEY_NOTHING)
    {
      //select specific function
      if (anyKey == KEY_CENTER)
      {
        switch(cursor)
        {
          case 0: playSnake(); break;
          // Tutaj trzeba podać wiadomosc i rozmiar wiadomosci.
          case 1: sendScore(memblock, memblockSize); break;
          default: break;
        }
        drawMenu();
      }
      
      //move cursor up
      else if (anyKey == KEY_UP)
      {
        if (cursor > 0)
          cursor--;
        else
          cursor = 1;
        drawMenuCursor(cursor);
      }
      
      //move cursor down
      else if (anyKey == KEY_DOWN)
      {
        if (cursor < 1)
          cursor++;
        else
          cursor = 0;
        drawMenuCursor(cursor);
      }
      
      //adjust contrast
      else if (anyKey == KEY_RIGHT)
      {
        contrast++;
        if (contrast > 127)
          contrast = 127;
        lcdContrast(contrast);
      }
      else if (anyKey == KEY_LEFT)
      {
        if (contrast > 0)
          contrast--;
        lcdContrast(contrast);
      }
    }
    osSleep(20);
  }
}


/*****************************************************************************
 *
 * Description:
 *    The entry function for the initialization process. 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
initProc(void* arg)
{
  tU8 error;

  eaInit();
  
  osCreateProcess(proc1, proc1Stack, PROC1_STACK_SIZE, &pid1, 3, NULL, &error);
  osStartProcess(pid1, &error);

  initBtProc();
  
  initKeyProc();

  osDeleteProcess();
}

/*****************************************************************************
 *
 * Description:
 *    The timer tick entry function that is called once every timer tick
 *    interrupt in the RTOS. Observe that any processing in this
 *    function must be kept as short as possible since this function
 *    execute in interrupt context.
 *
 * Params:
 *    [in] elapsedTime - The number of elapsed milliseconds since last call.
 *
 ****************************************************************************/
void
appTick(tU32 elapsedTime)
{
  ms += elapsedTime;
}





