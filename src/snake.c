/******************************************************************************
 *
 * Copyright:
 *    (C) 2006 Embedded Artists AB
 *
 * File:
 *    exampleGame.c
 *
 * Description:
 *    Implementation of example game.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include "startup/lpc2xxx.h"
#include <printf_P.h>
#include <ea_init.h>
#include <stdlib.h>
#include "lcd.h"
#include "key.h"
#include "select.h"
#include "time.h"

#include "configAppl.h"


/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/
#define GAME_NOT_STARTED 0
#define GAME_RUNNING     1
#define GAME_OVER        2
#define GAME_END         3

#define BACKGROUND_COLOR 0
#define SNAKE_COLOR 0x5d
#define SNACK_COLOR 0x8d

#define MOVE_UP         4
#define MOVE_DOWN       1
#define MOVE_LEFT       2
#define MOVE_RIGHT      3

#define ARENA_SIZE      32
#define MAX_SNAKE_SIZE  64
//#define BOARD_WIDTH  10
//#define BOARD_HEIGHT 16

//#define FIGURE_SIZE 4
//#define NUM_OF_FIGURE 19
#define LED_GREEN 0
#define LED_RED 1

#define SOH 1
#define NAK 21
#define ACK 6
#define EOT 4
#define CAN 24
#define C 67


/*****************************************************************************
 * Local variables
 ****************************************************************************/
static tU8 gameStatus;
static tU8 snakeXPos;
static tU8 snakeYPos;
static tU8 snakeSize;
static tU8 lastKey;
static tU8 isOn;

static tU32 timeVal;
static tU8 gameArena[ARENA_SIZE][ARENA_SIZE];
static tU8 snakeTail[MAX_SNAKE_SIZE];
static tU8 snakeTailGuard;
//static tU8 score;

//static tS32 shift[NUM_OF_FIGURE]={1,0,3,4,5,2,7,8,9,6,11,12,13,10,15,14,17,16,18};
//static tS32 board[BOARD_WIDTH][BOARD_HEIGHT];
//static tS32 currFigure;
//static tS32 currXpos;
//static tS32 currYpos;
//static volatile tU32 lastUpdate;
//static volatile tU32 delayTimeMs;


/*****************************************************************************
 * External variables
 ****************************************************************************/
//extern volatile tU32 ms;


//char figures[NUM_OF_FIGURE][FIGURE_SIZE][FIGURE_SIZE]=
//{{".X..",
//  ".X..",
//  ".X..",
//  ".X.."},
// {"....",
//  "XXXX",
//  "....",
//  "...."},

// {"XXX.",
//  "X...",
//  "....",
//  "...."},
// {".XX.",
//  "..X.",
//  "..X.",
//  "...."},
// {"..X.",
//  "XXX.",
//  "....",
//  "...."},
// {".X..",
//  ".X..",
//  ".XX.",
//  "...."},

// {"XXX.",
//  "..X.",
//  "....",
//  "...."},
// {"..X.",
//  "..X.",
//  ".XX.",
//  "...."},
// {"X...",
//  "XXX.",
//  "....",
//  "...."},
// {".XX.",
//  ".X..",
//  ".X..",
//  "...."},

// {".X..",
//  "XXX.",
//  "....",
//  "...."},
// {".X..",
//  ".XX.",
//  ".X..",
//  "...."},
// {"XXX.",
//  ".X..",
//  "....",
//  "...."},
// {".X..",
//  "XX..",
//  ".X..",
//  "...."},

// {"X...",
//  "XX..",
//  ".X..",
//  "...."},
// {".XX.",
//  "XX..",
//  "....",
//  "...."},

// {".X..",
//  "XX..",
//  "X...",
//  "...."},
// {"XX..",
//  ".XX.",
//  "....",
//  "...."},
// {"XX..",
//  "XX..",
//  "....",
//  "...."}};


///*****************************************************************************
// *
// * Description:
// *    Check if current place for figure is valid
// *
// ****************************************************************************/
//static tS8
//isValid(tS32 x, tS32 y, tS32 shape)
//{
//  tS32 i,j;

//  for (i=0; i<FIGURE_SIZE; i++)
//    for (j=0; j<FIGURE_SIZE; j++)
//      if((figures[shape][i][j] == 'X') &&
//         ((i+x >= BOARD_WIDTH) || (j+y >= BOARD_HEIGHT) || (i+x < 0)))
//        return 0;

//  for (i=0; i<FIGURE_SIZE; i++)
//    for (j=0; j<FIGURE_SIZE; j++)
//      if((i+x < BOARD_WIDTH) &&
//         (j+y < BOARD_HEIGHT) &&
//         (figures[shape][i][j] == 'X') &&
//         (board[i+x][j+y] != 0))
//        return 0;

//  return 1;
//}


///*****************************************************************************
// *
// * Description:
// *    Draw/refresh game board
// *
// ****************************************************************************/
//static void
//drawGame(void)
//{
//  tS32 i,j;

//  for (i=1; i<=BOARD_WIDTH; i++)
//    for (j=1; j<=BOARD_HEIGHT; j++)
//      if(board[i-1][j-1] != 0)
//        lcdRectBrd(30+(i*6), 10+(j*6), 6, 6, 209,50,5);
//      else
//        lcdRect(30+(i*6), 10+(j*6), 6, 6, 0);
//}


///*****************************************************************************
// *
// * Description:
// *    Insert one figure at a specific x,y position on the game board
// *
// ****************************************************************************/
//static void
//insertFigure(tS32 shape, tS32 x, tS32 y)
//{
//  tS32 i,j;

//  for (i=0; i<FIGURE_SIZE; i++)
//    for (j=0; j<FIGURE_SIZE; j++)
//      if(figures[shape][i][j] == 'X')
//        board[x+i][y+j] = 1;
//}


///*****************************************************************************
// *
// * Description:
// *    Delete one figure at a specific x,y position on the game board
// *
// ****************************************************************************/
//static void
//deleteFigure(tS32 shape, tS32 x, tS32 y)
//{
//  tS32 i,j;

//  for (i=0; i<FIGURE_SIZE; i++)
//    for (j=0; j<FIGURE_SIZE; j++)
//      if(figures[shape][i][j] == 'X')
//        board[x+i][y+j] = 0;
//}


///*****************************************************************************
// *
// * Description:
// *    Returns TRUE if one row of the board is full
// *
// ****************************************************************************/
//static char
//full(tS32 j)
//{
//  tS32 i;

//  for(i=0; i<BOARD_WIDTH; i++)
//    if(board[i][j] != 1)
//      return 0;

//  return 1;
//}


///*****************************************************************************
// *
// * Description:
// *    Draw the current score
// *
// ****************************************************************************/
//static void
//drawScore(void)
//{
//  tU8 str[4];

//  str[0] = score/100 + '0';
//  str[1] = (score/10)%10 + '0';
//  str[2] = score%10 + '0';
//  str[3] = 0;
//  if (str[0] == '0')
//  {
//    str[0] = ' ';
//    if (str[1] == '0')
//      str[1] = ' ';
//  }
//  lcdGotoxy(80,116);
//  lcdPuts(str);
//}


///*****************************************************************************
// *
// * Description:
// *    Check if time to update score
// *
// ****************************************************************************/
//static void
//checkScore(void)
//{
//  tS32 i,j,k;

//  for (j=0; j<BOARD_HEIGHT; j++)
//  {
//    //check if one row is full
//    if(full(j))
//    {
//      //if so, erase that row...
//      for(k=j; k>0; k--)
//        for(i=0; i<BOARD_WIDTH; i++)
//          board[i][k] = board[i][k-1];

//      //and update score
//      score++;
//      drawScore();
//    }
//  }
//}


///*****************************************************************************
// *
// * Description:
// *    Returns TRUE if game is over
// *
// ****************************************************************************/
//static char
//gameOver(void)
//{
//  tS32 i;

//  for(i=0; i<NUM_OF_FIGURE; i++)
//    if(!isValid(3,0,i))
//      return 1;

//  return 0;
//}


///*****************************************************************************
// *
// * Description:
// *    Calculate new update/advance time, based in current score
// *    The higher score, the shorter update time
// *
// ****************************************************************************/
//static void
//updateDelayTime(void)
//{
//  if (score >= 75)
//    delayTimeMs = 200;
//  else if (score >= 50)
//    delayTimeMs = 300;
//  else if (score >= 25)
//    delayTimeMs = 350;
//  else if (score >= 10)
//    delayTimeMs = 400;
//  else
//    delayTimeMs = 500;
//}


///*****************************************************************************
// *
// * Description:
// *    Advance the game, check if game is over
// *
// ****************************************************************************/
//static void
//advanceGame(void)
//{
//  insertFigure(currFigure, currXpos, currYpos);
//  drawGame();
//  deleteFigure(currFigure, currXpos, currYpos);

//  /*******************************************************
//   * Check if time to update figure position
//   *******************************************************/
//  updateDelayTime();
//  if ((ms - lastUpdate) > delayTimeMs)
//  {
//    lastUpdate = ms;

//    if (isValid(currXpos, currYpos+1, currFigure))
//      currYpos++;

//    //figure has hit bottom
//    //insert new, random figure on the game board
//    else
//    {
//      insertFigure(currFigure, currXpos, currYpos);
//      currYpos  = 0;
//      currXpos  = 3;
//      currFigure = rand() % NUM_OF_FIGURE;

//      //check if game is over
//      if (gameOver())
//        gameStatus = GAME_OVER;
//    }

//    //check score, i.e., if any rows are full
//    checkScore();
//  }
//}


/*****************************************************************************
 *
 * Description:
 *    Draw game background and game board, initialize all variables
 *
 ****************************************************************************/
//static void
//setupGame (tBool drawBoard)
//{
//  tS32 i,j;

//  //initialize random generator and reset score
//  srand(ms);
//  score = 0;


//  //draw game board
//  if (drawBoard == TRUE)
//  {
//    lcdColor(1, 0xe0);
//    lcdGotoxy(16, 116);
//    lcdPuts(" Score:   0 ");

//    lcdRect(34, 14, 64, 100, 3);
//    lcdRect(36, 16, 60, 96, 0);
//  }

//  //initialise game board
//  for (i=0; i<BOARD_WIDTH; i++)
//    for (j=0; j<BOARD_HEIGHT; j++)
//      board[i][j] = 0;

//  currFigure = rand() % NUM_OF_FIGURE;
//  currXpos  = 3;
//  currYpos  = 0;
//  insertFigure(currFigure, currXpos, currYpos);

//  lastUpdate = ms;
//}

int toggle = 0;
int count = 0;

tU16 notes[] = {262, 294, 330, 349, 392, 440, 494, 523}; //C D E F G A B C
int note = 0;

static void
startMusic(void)
{
        setBuzzer(toggle == 0 ? FALSE : TRUE);

        toggle = ~toggle;
        count++;

        if(count == notes[note]*10) //Note delay
        {
                if(note < 8)
                                note++;                 //choose next note
                if(note == 8)
                {
                                T1MCR = 0x06;   //Stop and reset timer at MR0
                                setBuzzer(FALSE);
                                setLED(LED_GREEN, FALSE);
                                setLED(LED_RED, FALSE);
                                note = 0;
                }
                T1TCR = 0x02;                                              //Disable and reset timer
                T1MR0 = CORE_FREQ / (100*notes[note]); //set new timer's ticks
                T1TCR = 0x01;                                              //Start timer
                count = 0;
        }

        T1IR        = 0x000000ff;        //reset all IRQ flags
        VICVectAddr = 0x00000000;        //dummy write to VIC to signal end of interrupt
}

static void
eatNoise(void)
{
    setBuzzer(toggle == 0 ? FALSE : TRUE);

    toggle = ~toggle;
    count++;

    if(count == notes[note]*10) //Note delay
    {
            if(note < 3)
                            note++;                 //choose next note
            if(note == 3)
            {
                            T1MCR = 0x06;   //Stop and reset timer at MR0
                            setBuzzer(FALSE);
                            setLED(LED_GREEN, FALSE);
                            setLED(LED_RED, FALSE);
                            note = 0;
            }
            T1TCR = 0x02;                                              //Disable and reset timer
            T1MR0 = CORE_FREQ / (100*(notes[note] + 15 * snakeSize)); //set new timer's ticks
            T1TCR = 0x01;                                              //Start timer
            count = 0;
    }

    T1IR        = 0x000000ff;        //reset all IRQ flags
    VICVectAddr = 0x00000000;        //dummy write to VIC to signal end of interrupt
}

static void
waitUp(void)
{
    RTC_CIIR = 0x1;
}

static void
initTimer1(tU32 functionAdress)
{
        //initialize and start Timer #0
        T1TCR = 0x00000002;                           //disable and reset Timer0
        T1PC  = 0x00000000;                           //no prescale of clock
        T1MR0 = CORE_FREQ / (100*notes[note]);        //calculate no of timer ticks for note[0] its 3.8ms
        T1IR  = 0x000000ff;                           //reset all flags before enable IRQs
        T1MCR = 0x00000003;                           //reset counter and generate IRQ on MR0 match

        //initialize VIC for Timer0 interrupts
        VICIntSelect &= ~0x20;           //Timer1 interrupt is assigned to IRQ (not FIQ)
        VICVectAddr5  = functionAdress; //register ISR address
        VICVectCntl5  = 0x25;            //enable vector interrupt for timer1
        VICIntEnable  |= 0x20;           //enable timer1 interrupt
        T1TCR = 0x00000001;              //start Timer1
}

void stopTimer1()
{
    T1MCR = 0x06;   //Stop and reset timer at MR0
}

static void
drawArena(void)
{
    tU8 step = 128 / ARENA_SIZE;
    tU8 i,j;
    for(i = 0; i < ARENA_SIZE; i++)
    {
        for(j = 0; j < ARENA_SIZE; j++)
        {
            lcdRect(step * i, step * j, step, step, gameArena[i][j]);
        }
    }
}

static void
setBorder(tU8 color)
{
    tU8 i,j;
    for(i = 0; i < ARENA_SIZE; i++)
    {
        for(j = 0; j < ARENA_SIZE; j++)
        {
            if (i == 0 || j == 0 || i == ARENA_SIZE - 1 || j == ARENA_SIZE - 1)
            {
                gameArena[i][j] = color;
            }
        }
    }
}

static void
setField(tU8 x, tU8 y, tU8 color)
{
    gameArena[x][y] = color;
}

static void
resetSnake()
{
	int j = 0, i = snakeTailGuard;
	int index;
	tU8 snakeTailCopy[snakeSize];
	/*
	for(index = 0; index < snakeSize; index++)
	{
		snakeTailCopy[snakeSize-index-1] = snakeTail[(snakeTailGuard+index)%snakeSize];
	}
	*/
	do 
    {
		snakeTailCopy[snakeSize-1-i] = snakeTail[i];
        i = (i + 1) % snakeSize;
    }while(i != snakeTailGuard % snakeSize);
	snakeTailGuard = snakeSize - 1;
}

static void
makeNoise()
{
//    setBuzzer(TRUE);
//    setLED(LED_GREEN, FALSE);
//    setLED(LED_RED,   TRUE);
//    osSleep(1);

//    setBuzzer(FALSE);
//    setLED(LED_GREEN, TRUE);
//    setLED(LED_RED,   FALSE);
//    osSleep(1);

    initTimer1((tU32)eatNoise);
}

static void
moveField(tU8 x, tU8 y, tU8 direction)
{
    int actualX = x;
    int actualY = y;
    // Draw point.
    switch(direction)
    {
    case MOVE_DOWN:
        actualY++;
        break;
    case MOVE_UP:
        actualY--;
        break;
    case MOVE_LEFT:
        actualX--;
        break;
    case MOVE_RIGHT:
        actualX++;
        break;
    default:
        return;
    }

	cleanEndOfTail();
	

	
    // Remember move
    snakeTail[snakeTailGuard] = direction;
    snakeTailGuard = (snakeTailGuard + 1) % snakeSize;
	
	if(gameArena[actualX][actualY] == SNAKE_COLOR)
    {		
        //GAMEOVER
    }
    else if (gameArena[actualX][actualY] == SNACK_COLOR && snakeSize < MAX_SNAKE_SIZE)
    {
		//Move and eat.
		//TODO brzeczyk zjadania

        makeNoise();
		resetSnake();
        snakeSize++;
		putSnack();
    }
	else
	{
		//Normal move. Erase last point
	}
	
	//add snake point
	setField(actualX, actualY, SNAKE_COLOR);
}

static void
putSnack()
{
	tU8 x,y, snackSet = 0;
	do
	{
		x = rand() % ARENA_SIZE;
		y = rand() % ARENA_SIZE;
		if (gameArena[x][y]==BACKGROUND_COLOR)
		{
			snackSet = 1;
			gameArena[x][y] = SNACK_COLOR;
		}
	}while(!snackSet);
}

static tU8
cleanEndOfTail()
{
    tU8 actualX = snakeXPos;
    tU8 actualY = snakeYPos;
	tU8 i = snakeTailGuard;
    do 
    {
        switch(snakeTail[i])
        {
        case MOVE_DOWN:
            actualY--;
            break;
        case MOVE_UP:
            actualY++;
            break;
        case MOVE_LEFT:
            actualX++;
            break;
        case MOVE_RIGHT:
            actualX--;
            break;
        default:
            return;
        }
        i = (i + 1) % snakeSize;
    }while(i != snakeTailGuard % snakeSize);
	
    gameArena[actualX][actualY] = BACKGROUND_COLOR;
}

static tU8
mod (tU8 a, tU8 b)
{
   tU8 ret = a % b;
   if(ret < 0)
     ret+=b;
   return ret;
}

void
enterSleepMode()
{
    PCON = 0x2;
}

void
moveSnake(tU8 anyKey)
{
    if (anyKey == KEY_UP)
    {
        if(snakeYPos != 1)
        {
            moveField(snakeXPos, snakeYPos, MOVE_UP);
            snakeYPos -= 1;
        }
    }
    else if (anyKey == KEY_DOWN)
    {
        if(snakeYPos != ARENA_SIZE - 2)
        {
            moveField(snakeXPos, snakeYPos, MOVE_DOWN);
            snakeYPos += 1;
        }
    }
    else if (anyKey == KEY_RIGHT)
    {
        if(snakeXPos != ARENA_SIZE - 2)
        {
            moveField(snakeXPos, snakeYPos, MOVE_RIGHT);
            snakeXPos += 1;
        }
    }
    else if (anyKey == KEY_LEFT)
    {
        if(snakeXPos != 1)
        {
            moveField(snakeXPos, snakeYPos, MOVE_LEFT);
            snakeXPos -= 1;
        }
    }
    else if (anyKey == KEY_CENTER)
    {
        if (isOn == 0)
        {
            setLED(1, 1);
            setLED(2, 1);
            isOn = 1;
            osSleep(1);
        }
        else
        {
            setLED(1, 0);
            setLED(2, 0);
            isOn = 0;
            osSleep(1);
        }
        waitUp();
        //enterSleepMode();
    }
}


/*****************************************************************************
 *
 * Description:
 *    Implements example game
 *
 ****************************************************************************/
void
playSnake(void)
{
    //srand(time(NULL));
    initTimer1((tU32)startMusic);
    snakeXPos = 16;
    snakeYPos = 16;
    snakeSize = 1; //TODO (nie obslugujemy wiekszych)
    snakeTailGuard = 0;
    snakeTail[0] = MOVE_RIGHT;
    lcdClrscr();
    setBorder(0xfd);
    setField(snakeXPos, snakeYPos, SNAKE_COLOR);
    lastKey = KEY_RIGHT;
    isOn = 0;
	//init snacks
	gameArena[10][10] = SNACK_COLOR;
	putSnack();
	putSnack();
    for(;;)
    {
        timeVal++;


        tU8 anyKey;

        anyKey = checkKey();
        if (anyKey != KEY_NOTHING)
        {
            lastKey = anyKey;
            osSleep(1);
        }

        moveSnake(lastKey);
        drawArena();
    }


    //  setupGame(FALSE);
    //  lcdGotoxy(5,40);
    //  lcdColor(0,0xe0);
    //  lcdPuts("Press to start");

    //  while(gameStatus != GAME_END)
    //  {
    //    tU8 anyKey;

    //    anyKey = checkKey();
    //    switch(gameStatus)
    //    {
    //      case GAME_NOT_STARTED:
    //        if (anyKey != KEY_NOTHING)
    //        {
    //          gameStatus = GAME_RUNNING;
    //          setupGame(TRUE);
    //        }
    //        break;

    //      case GAME_RUNNING:
    //        if (anyKey != KEY_NOTHING)
    //        {
    //          if (anyKey == KEY_UP)
    //          {
    //            if(isValid(currXpos, currYpos, shift[currFigure]))
    //              currFigure = shift[currFigure];
    //          }
    //          else if (anyKey == KEY_DOWN)
    //          {
    //            if(isValid(currXpos, currYpos+1, currFigure))
    //              currYpos++;
    //          }
    //          else if (anyKey == KEY_RIGHT)
    //          {
    //            if(isValid(currXpos+1, currYpos, currFigure))
    //              currXpos++;
    //          }
    //          else if (anyKey == KEY_LEFT)
    //          {
    //            if(isValid(currXpos-1, currYpos, currFigure))
    //              currXpos--;
    //          }
    //          else if (anyKey == KEY_CENTER)
    //            gameStatus = GAME_OVER;

    //          advanceGame();
    //          osSleep(1);
    //        }
    //        else
    //        {
    //          advanceGame();
    //          osSleep(1);
    //        }
    //        break;

    //      case GAME_OVER:
    //      {
    //        tMenu menu;

    //        menu.xPos = 10;
    //        menu.yPos = 40;
    //        menu.xLen = 6+(12*8);
    //        menu.yLen = 4*14;
    //        menu.noOfChoices = 2;
    //        menu.initialChoice = 0;
    //        menu.pHeaderText = "Game over!";
    //        menu.headerTextXpos = 20;
    //        menu.pChoice[0] = "Restart game";
    //        menu.pChoice[1] = "End game";
    //        menu.bgColor       = 0;
    //        menu.borderColor   = 0x6d;
    //        menu.headerColor   = 0;
    //        menu.choicesColor  = 0xfd;
    //        menu.selectedColor = 0xe0;

    //        switch(drawMenu(menu))
    //        {
    //          case 0: gameStatus = GAME_RUNNING; setupGame(TRUE); break;  //Restart game
    //          case 1: gameStatus = GAME_END; break;                       //End game
    //          default: break;
    //        }
    //      }
    //        break;

    //      default:
    //        gameStatus = GAME_END;
    //        break;
    //    }
    //  }
}

void
sendScore(tU8 *memblock, int rozmiar)
{
    tU8 i;
    //Wyliczenie liczby pakietow
    int liczba_pakietow = rozmiar / 16;
    if (liczba_pakietow * 16 != rozmiar)
    {
        liczba_pakietow++;
    }

    //Sprawdzenie czy odbiornik jest gotowy do przyjecia pliku, tj. czy wyslany jest sygnal NAK
    tU8 brak_odpowiedzi = TRUE;
    for (i = 0; i < 20; i++)
    {
        tU8 sprawdz = uart1GetCh();
        if(sprawdz == NAK)
        {
            brak_odpowiedzi = FALSE;
            break;
        }
        else
        {
            osSleep(10);
        }
    }
    if(brak_odpowiedzi)
    {
        setLED(2, 1);
    }
    else
    {
        tU8 nr_pakietu = 1;
        tU8 zmienna;
        tU8 naglowek[3];
        tU8 checksum;
        for (i = 0; i < liczba_pakietow; )
        {
            checksum = 0;
            naglowek[0] = SOH;
            naglowek[1] = nr_pakietu;
            naglowek[2] = 255 - nr_pakietu;

            uart1SendChar(naglowek[0]);
            uart1SendChar(naglowek[1]);
            uart1SendChar(naglowek[2]);
            tU8 j;
            for(j = 0; j < 16; j++)
            {
                if(((nr_pakietu - 1) * 16 + j) < rozmiar)
                {
                    uart1SendChar(memblock[(nr_pakietu - 1) * 16 + j]);
                    checksum += memblock[(nr_pakietu - 1) * 16 + j];
                }
                else
                {
                    uart1SendChar(26);
                    checksum += 26;
                }
            }

            uart1SendChar(checksum);

            while(zmienna != ACK && zmienna != NAK)
            {
                zmienna = uart1GetCh();
            }
            if(zmienna == ACK || zmienna == C)
            {
                nr_pakietu++;
                i++;
            }
        }
        do
        {
            uart1SendChar(EOT);
            osSleep(100);
        }while(uart1GetCh() != ACK);
    }
}
