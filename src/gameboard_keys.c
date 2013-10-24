/*****************************************************************************
 *
 * Description:
 *    Function to check if any key press has been detected
 *
 ****************************************************************************/
tU8
checkKey(void)
{
  tU8 retVal = activeKey;
  activeKey = KEY_NOTHING;
  return retVal;
}


/*****************************************************************************
 *
 * Description:
 *    Creates and starts the key sampling process. 
 *
 ****************************************************************************/
void
initKeyProc(void)
{
  tU8 error;

  osCreateProcess(procKey, keyProcStack, KEYPROC_STACK_SIZE, &keyProcPid, 3, NULL, &error);
  osStartProcess(keyProcPid, &error);
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
procKey(void* arg)
{
  //sample keys each 50 ms, i.e., 20 times per second
  while(1)
  {
    sampleKey();
    osSleep(5);
  }
}


/*****************************************************************************
 *
 * Description:
 *    Sample key states
 *
 ****************************************************************************/
void
sampleKey(void)
{
  tBool nothing = TRUE;
  tU8   readKeys;
  
  //get sample
  readKeys = getKeys();
  

  //check center key
  if (readKeys & KEY_CENTER)
  {
    nothing = FALSE;
  	if (centerReleased == TRUE)
  	{
  		centerReleased = FALSE;
  		centerKeyCnt = 0;
  		activeKey = KEY_CENTER;
  		activeKey2 = KEY_CENTER;
  	}
  	else
  	{
  	  centerKeyCnt++;
  	  if (centerKeyCnt == FIRST_REPEAT)
  	  {
  		  activeKey = KEY_CENTER;
  		  activeKey2 = KEY_CENTER;
  	  }
  	  else if (centerKeyCnt >= FIRST_REPEAT + SECOND_REPEAT)
  	  {
  		  centerKeyCnt = FIRST_REPEAT;
  		  activeKey = KEY_CENTER;
  		  activeKey2 = KEY_CENTER;
  	  }
  	}
  }
  else
  	centerReleased = TRUE;

  //check up key
  if (readKeys & KEY_UP)
  {
    nothing = FALSE;
  	if (keyUpReleased == TRUE)
  	{
  		keyUpReleased = FALSE;
  		upKeyCnt = 0;
  		activeKey = KEY_UP;
  		activeKey2 = KEY_UP;
  	}
  	else
  	{
  	  upKeyCnt++;
  	  if (upKeyCnt == FIRST_REPEAT)
  	  {
  		  activeKey = KEY_UP;
  		  activeKey2 = KEY_UP;
  	  }
  	  else if (upKeyCnt >= FIRST_REPEAT + SECOND_REPEAT)
  	  {
  		  upKeyCnt = FIRST_REPEAT;
  		  activeKey = KEY_UP;
  		  activeKey2 = KEY_UP;
  	  }
  	}
  }
  else
  	keyUpReleased = TRUE;

  //check down key
  if (readKeys & KEY_DOWN)
  {
    nothing = FALSE;
  	if (keyDownReleased == TRUE)
  	{
  		keyDownReleased = FALSE;
  		downKeyCnt = 0;
  		activeKey = KEY_DOWN;
  		activeKey2 = KEY_DOWN;
  	}
  	else
  	{
  	  downKeyCnt++;
  	  if (downKeyCnt == FIRST_REPEAT)
  	  {
  		  activeKey = KEY_DOWN;
  		  activeKey2 = KEY_DOWN;
  	  }
  	  else if (downKeyCnt >= FIRST_REPEAT + SECOND_REPEAT)
  	  {
  		  downKeyCnt = FIRST_REPEAT;
  		  activeKey = KEY_DOWN;
  		  activeKey2 = KEY_DOWN;
  	  }
  	}
  }
  else
  	keyDownReleased = TRUE;

  //check left key
  if (readKeys & KEY_LEFT)
  {
    nothing = FALSE;
  	if (keyLeftReleased == TRUE)
  	{
  		keyLeftReleased = FALSE;
  		leftKeyCnt = 0;
  		activeKey = KEY_LEFT;
  		activeKey2 = KEY_LEFT;
  	}
  	else
  	{
  	  leftKeyCnt++;
  	  if (leftKeyCnt == FIRST_REPEAT)
  	  {
  		  activeKey = KEY_LEFT;
  		  activeKey2 = KEY_LEFT;
  	  }
  	  else if (leftKeyCnt >= FIRST_REPEAT + SECOND_REPEAT)
  	  {
  		  leftKeyCnt = FIRST_REPEAT;
  		  activeKey = KEY_LEFT;
  		  activeKey2 = KEY_LEFT;
  	  }
  	}
  }
  else
  	keyLeftReleased = TRUE;

  //check right key
  if (readKeys & KEY_RIGHT)
  {
    nothing = FALSE;
  	if (keyRightReleased == TRUE)
  	{
  		keyRightReleased = FALSE;
  		rightKeyCnt = 0;
  		activeKey = KEY_RIGHT;
  		activeKey2 = KEY_RIGHT;
  	}
  	else
  	{
  	  rightKeyCnt++;
  	  if (rightKeyCnt == FIRST_REPEAT)
  	  {
  		  activeKey = KEY_RIGHT;
  		  activeKey2 = KEY_RIGHT;
  	  }
  	  else if (rightKeyCnt >= FIRST_REPEAT + SECOND_REPEAT)
  	  {
  		  rightKeyCnt = FIRST_REPEAT;
  		  activeKey = KEY_RIGHT;
  		  activeKey2 = KEY_RIGHT;
  	  }
  	}
  }
  else
  	keyRightReleased = TRUE;
  
  if (nothing == TRUE)
    activeKey2 = KEY_NOTHING;
}


/*****************************************************************************
 *
 * Description:
 *    Get current state of joystick switch
 *
 ****************************************************************************/
tU8
getKeys(void)
{
  tU8 commandReadKeys[] = {0x00};
  tU8 readKeys = KEY_NOTHING;
  tU8 keySample;

  //check if ver 1.0 of HW
  if (TRUE == ver1_0)
  {
    if ((IOPIN & KEYPIN_CENTER) == 0) readKeys |= KEY_CENTER;
    if ((IOPIN & KEYPIN_UP) == 0)     readKeys |= KEY_UP;
    if ((IOPIN & KEYPIN_DOWN) == 0)   readKeys |= KEY_DOWN;
    if ((IOPIN & KEYPIN_LEFT) == 0)   readKeys |= KEY_LEFT;
    if ((IOPIN & KEYPIN_RIGHT) == 0)  readKeys |= KEY_RIGHT;
  }

  //HW is ver 1.1
  else
  {
    pca9532(commandReadKeys, sizeof(commandReadKeys), &keySample, 1);
    if ((keySample & 0x01) == 0) readKeys |= KEY_CENTER;
    if ((keySample & 0x04) == 0) readKeys |= KEY_UP;
    if ((keySample & 0x10) == 0) readKeys |= KEY_DOWN;
    if ((keySample & 0x02) == 0) readKeys |= KEY_LEFT;
    if ((keySample & 0x08) == 0) readKeys |= KEY_RIGHT;
  }

  return readKeys;
}


//in eeprom.c
/******************************************************************************
 *
 * Description:
 *    Communicate with the PCA9532
 *    First pBuf/len = bytes to write
 *    Second pBuf2/len2 = bytes to read
 *
 *****************************************************************************/
tS8
pca9532(tU8* pBuf, tU16 len, tU8* pBuf2, tU16 len2) 
{
  tS8  retCode = 0;
  tU8  status  = 0;
  tU16 i       = 0;

getI2cLock();
  do
  {
    /* generate Start condition */
    retCode = i2cStart();
    if(retCode != I2C_CODE_OK)
      break;


    /* write pca9532 address */
    retCode = i2cWriteWithWait(0xc0);
    if(retCode != I2C_CODE_OK)
      break;

    /* write data */
    for(i = 0; i <len; i++)
    {
      retCode = i2cWriteWithWait(*pBuf);
      if(retCode != I2C_CODE_OK)
        break;

      pBuf++;
    }

  } while(0);

  if (len2 > 0)
  {
    /* Generate Start condition */
    retCode = i2cRepeatStart();

    /* Transmit device address */
    if( retCode == I2C_CODE_OK)
    {
      /* Write SLA+R */
      retCode = i2cPutChar( 0xc0 + 0x01 );
      while( retCode == I2C_CODE_BUSY )
      {
        retCode = i2cPutChar( 0xc0 + 0x01 );
      }
    }

    /* Wait until SLA+R transmitted */
    while(1)
    {
      /* Get new status */
      status = i2cCheckStatus();

      if(status == 0x40)
      {
        /* Data transmitted and ACK received */
        break;
      }
      else if(status != 0xf8)
      {
        /* error */
        retCode = I2C_CODE_ERROR;
        break;
      }
    }

    if( retCode == I2C_CODE_OK )
    {
      /* wait until address transmitted and receive data */
      for(i = 1; i <= len2; i++ )
      {
        /* wait until data transmitted */
        while(1)
        {
          /* Get new status */
          status = i2cCheckStatus();

          if(( status == 0x40 ) || ( status == 0x48 ) || ( status == 0x50 ))
          {
            /* Data received */

            if(i == len2 )
            {
              /* Set generate NACK */
              retCode = i2cGetChar( I2C_MODE_ACK1, pBuf2 );
            }
            else
            {
              retCode = i2cGetChar( I2C_MODE_ACK0, pBuf2 );
            }

            /* Read data */
            retCode = i2cGetChar( I2C_MODE_READ, pBuf2 );
            while( retCode == I2C_CODE_EMPTY )
            {
              retCode = i2cGetChar( I2C_MODE_READ, pBuf2 );
            }
            pBuf2++;

            break;
          }
          else if( status != 0xf8 )
          {
            /* ERROR */
            i = len2;
            retCode = I2C_CODE_ERROR;
            break;
          }
        }
      }
    }
  }

  /* Generate Stop condition */
  i2cStop();
releaseI2cLock();

  return retCode;

}
