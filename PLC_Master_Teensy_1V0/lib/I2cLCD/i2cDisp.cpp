#include "Wire.h"
#include "i2cDisp.h"

//#define i2cAddr (0x50 >> 1)

I2cDisp ::I2cDisp(bool echoUartMsg, uint8_t i2cAddr)
{
  // Constructor code. Place everything needed to initialze the instance
  _uartMsg = echoUartMsg;
  _i2cAddr = i2cAddr;
}

/* The i2c Display function definitions */

void I2cDisp ::init()
{
  Wire.begin();
  i2cWriteCmd(LCD_DISPLAYON);
  clear();
  home();
  i2cWriteByte('$');
}

void I2cDisp ::i2cWriteByte(uint8_t data)
{
  Wire.beginTransmission(_i2cAddr);
  Wire.write(data);
  Wire.endTransmission();
}

void I2cDisp ::i2cWriteCmd(uint8_t cmd)
{
  Wire.beginTransmission(_i2cAddr);
  Wire.write(LCD_COMMAND_PREFIX);
  Wire.write(cmd);
  Wire.endTransmission();
}

void I2cDisp ::i2cWriteStr(char *msg)
{
  uint8_t i = 0;

  while (*msg) // (*msg != '\0')
  {
    if (isPrintable(*msg))
    {
      i2cWriteByte(*msg);
    }
    i++;
    if (i == 0)
      lcdSetCursor(CRSR_LINE_1); // Line 1
    if (i == 20)
      lcdSetCursor(DISP_LINE_2); // Line 2
    if (i == 40)
      lcdSetCursor(DISP_LINE_3); // Line 3
    if (i == 60)
      lcdSetCursor(DISP_LINE_4); // Line 4
    if (i == 80)
      break;
    msg++;
  } // end sending characters to LCD
delay(10); // Prevents printing chunks of msg to different parts of ths LCD (weird)
}

void I2cDisp ::print(char *msg)
// Prints message (*msg) starting at current cursor position,
// until it encounters a terminating \0 in the string.
// This routing does not know where the current cursor position is.
// Suitable for printing short messages, such as numerical results
{
  uint8_t i = 0;

  while (*msg) // (*msg != '\0')
  {
    if (isPrintable(*msg))
    {
      i2cWriteByte(*msg);
    }
    i++;
    msg++;
  } // end sending characters to LCD
  delay(10); // Prevents printing chunks of msg to different parts of ths LCD (weird)
}

void I2cDisp ::print(char *msg, uint8_t absCursor)
// Prints message (*msg) starting at absolute cursor position (absCursor),
// until it encounters a terminating \0 in the string. When it reaches the end
// of the display line, in continues printing on the next line below.
{
  lcdSetCursor(absCursor);

  while (*msg) // (*msg != '\0')
  {
    if (isPrintable(*msg))
    {
      i2cWriteByte(*msg);
    }
    msg++;
    absCursor++;
    switch (absCursor)
    {
    case CRSR_LINE_1 + DISP_CHAR_PER_LINE: // reached end of Line 1 (0+20=20, 0+0x14=0x14)
      absCursor = CRSR_LINE_2;             // continue on line 2 (0x40=64)
      lcdSetCursor(absCursor);
      break;
    case CRSR_LINE_2 + DISP_CHAR_PER_LINE: // reached end of Line 2 (64+20=84, 0x54+0x14=0x68)
      absCursor = CRSR_LINE_3;             // continue on Line 3 (0x14=20)
      lcdSetCursor(absCursor);
      break;
    case CRSR_LINE_3 + DISP_CHAR_PER_LINE: // reached end of Line 3 (20+20=40, 0x14+0x14=0x28)
      absCursor = CRSR_LINE_4;             // continue on Line 4 (0x54=84)
      lcdSetCursor(absCursor);
      break;
    case CRSR_LINE_4 + DISP_CHAR_PER_LINE: // reached end of Line 4 (84+20=104, 0x54+0x14=0x64)
      absCursor = CRSR_LINE_1;             // continue on Line 1 (0)
      lcdSetCursor(absCursor);
      break;
    }
  } // end sending characters to LCD
delay(10); // Prevents printing chunks of msg to different parts of ths LCD (weird)
}

void I2cDisp ::print(char *msg, uint8_t cursor, uint8_t dispLine)
// Prints to the LCD display on line specified by dispLine and at cursor position
// specified by cursor. It prints until it either:
// - encounters a terminating \0 in the message string (*msg) or
// - reaches the end of the current line on the display
{
  uint8_t crsr;
  if (cursor > DISP_CHAR_PER_LINE) // if cursor is larger than display line lenght, take modulo
    cursor %= DISP_CHAR_PER_LINE;

  crsr = cursor;
  switch (dispLine)
  {
  case DISP_LINE_1:
    crsr = CRSR_LINE_1 + cursor; // print on Line 1 + the specified cursor offset
    break;
  case DISP_LINE_2:
    crsr = CRSR_LINE_2 + cursor; // print on Line 2 + the specified cursor offset
    break;
  case DISP_LINE_3:
    crsr = CRSR_LINE_3 + cursor; // print on Line 3 + the specified cursor offset
    break;
  case DISP_LINE_4:
    crsr = CRSR_LINE_4 + cursor; // print on Line 4 + the specified cursor offset
    break;
  }
  lcdSetCursor(crsr);

  while (*msg && (cursor <= DISP_CHAR_PER_LINE))
  {
    if (isPrintable(*msg))
    {
      i2cWriteByte(*msg);
    }
    msg++;
    cursor++;
  }
delay(10); // Prevents printing chunks of msg to different parts of ths LCD (weird)
}

void I2cDisp ::println(char *msg, uint8_t dispLine)
// Print message (*msg) from beginning of the specified line to the end of that line
// if the end of the message (\0) is reached before the end of the display line, pad with
// SPACE characters, to avoid displaying the remants of previous (longer) messages on the display
{
  uint8_t cursor = 0;
  uint8_t len = strlen(msg);

  switch (dispLine)
  {
  case 0:
    cursor = CRSR_LINE_1; // start printing on beginning of Line 1
    break;
  case 1:
    cursor = CRSR_LINE_2; // start printing on beginning of Line 2
    break;
  case 2:
    cursor = CRSR_LINE_3; // start printing on beginning of Line 3
    break;
  case 3:
    cursor = CRSR_LINE_4; // start printing on beginning of Line 4
    break;
    // default:
    //   cursor = CRSR_LINE_1; // start printing on beginning of Line 1
    //   break;
  }
  lcdSetCursor(cursor);

  for (uint8_t i = 0; i < DISP_CHAR_PER_LINE; i++)
  {
    if (i < len)
      i2cWriteByte(*msg++);
    else
      i2cWriteByte(' ');
  }
delay(10); // Prevents printing chunks of msg to different parts of ths LCD (weird)
}

void I2cDisp ::clear()
{
  i2cWriteCmd(LCD_CLEARSCREEN);
  // delayMicroseconds(1000); // Datasheet claims, this command takes a long time
  delayMicroseconds(1000); // Datasheet claims, this command takes a long time
}

void I2cDisp ::clearln(uint8_t dispLine)
{
  uint8_t cursor = 0;
  switch (dispLine)
  {
  case DISP_LINE_1:
    cursor = CRSR_LINE_1; // start printing on beginning of Line 1
    break;
  case DISP_LINE_2:
    cursor = CRSR_LINE_2; // start printing on beginning of Line 2
    break;
  case DISP_LINE_3:
    cursor = CRSR_LINE_3; // start printing on beginning of Line 3
    break;
  case DISP_LINE_4:
    cursor = CRSR_LINE_4; // start printing on beginning of Line 4
    break;
  }
  lcdSetCursor(cursor);

  for (uint8_t i = 0; i < DISP_CHAR_PER_LINE; i++)
  {
    i2cWriteByte(' ');
  }
}

void I2cDisp ::home()
{
  i2cWriteCmd(LCD_CURSORHOME); // set cursor position to zero
  delayMicroseconds(1000);     //CM changed from 2000. this command takes a long time!
}

void I2cDisp ::lcdSetCursor(uint8_t cursor)
{
  Wire.beginTransmission(_i2cAddr);
  Wire.write(LCD_COMMAND_PREFIX);
  Wire.write(LCD_SETCURSOR);
  Wire.write(cursor);
  Wire.endTransmission();
  //delay(10);
}