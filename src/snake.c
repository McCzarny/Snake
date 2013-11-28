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
#define MOVE_NULL       5

#define ARENA_SIZE      32
#define MAX_SNAKE_SIZE  64

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
    for (index = 0; index < snakeSize; index++)
    {
        snakeTailCopy[index] = snakeTail[(snakeTailGuard+index)%snakeSize];
    }
    for (index = 0; index < snakeSize; index++)
    {
        snakeTail[index] = snakeTailCopy[index];
    }
}

static void
makeNoise()
{
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

<<<<<<< HEAD
	cleanEndOfTail();
	
=======
 cleanEndOfTail();
 
>>>>>>> Ostatni commit.
    // Remember move
    snakeTail[snakeTailGuard] = direction;
    snakeTailGuard = (snakeTailGuard + 1) % snakeSize;
 
 if(gameArena[actualX][actualY] == SNAKE_COLOR)
    {  
        //GAMEOVER
        enterSleepMode();
    }
    else if (gameArena[actualX][actualY] == SNACK_COLOR && snakeSize < MAX_SNAKE_SIZE)
    {
  //Move and eat.
        makeNoise();
		resetSnake();
        snakeSize++;
        snakeTailGuard = snakeSize-1;
        snakeTail[snakeTailGuard] = MOVE_NULL;
		putSnack();
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
	putSnack();
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
        setLED(LED_RED, TRUE);
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
				setLED(LED_GREEN, TRUE);
				osSleep(1);
				setLED(LED_GREEN, FALSE);
                nr_pakietu++;
                i++;
            }
			else
			{
				setLED(LED_RED, TRUE);
				osSleep(1);
				setLED(LED_RED, FALSE);
			}
        }
        do
        {
            uart1SendChar(EOT);
            osSleep(100);
        }while(uart1GetCh() != ACK);
    }
}
