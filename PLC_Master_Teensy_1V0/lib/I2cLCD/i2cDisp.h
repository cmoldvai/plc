#ifndef i2cDisp_h
#define i2cDisp_h

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// i2c LCD Display Parameters
#define I2C_ADDRESS_LCD (0x50 >> 1) // Default I2C address
// lCD table of commands
#define LCD_COMMAND_PREFIX 0xFE
#define LCD_DISPLAYON 0x41
#define LCD_DISPLAYOFF 0x42
#define LCD_SETCURSOR 0x45 //Takes 1 byte cursor location as parameter
#define LCD_CURSORHOME 0x46
#define LCD_UNDERLINECURSORON 0x47
#define LCD_UNDERLINECURSOROFF 0x48
#define LCD_MOVECURSORLEFT 0x49
#define LCD_MOVECURSORRIGHT 0x4A
#define LCD_CURSORBLINKON 0x4B
#define LCD_CURSORBLINKOFF 0x4C
#define LCD_BACKSPACE 0x4E
#define LCD_CLEARSCREEN 0x51
#define LCD_SETCONTRAST 0x52     // Takes 1 byte value (1-50) as parameter. 50 is highest. Default is 40
#define LCD_SETBLBRIGHTNESS 0x53 // Takes 1 byte value (1-8) as parameter. 8 is highest. Default is 1
#define LCD_CUSTOMCHARACTER 0x54 // Takes buffer of 9 bytes as parameter.
#define LCD_MOVEDISPLAY1LEFT 0x55
#define LCD_MOVEDISPLAY1RIGHT 0x56
#define LCD_CHANGEBAUDRATE 0x61 // 4:9600(default),5:14400,6:19.2K,7:57.6K,8:115.2K
#define LCD_CHANGEI2CADDRESS 0x62
#define LCD_DISPLAYFWVERSION 0x70
#define LCD_DISPLAYBAUDRATE 0x71
#define LCN_DISPLAYI2CADDRESS 0x72

#define DISP_NUM_LINES 4
#define DISP_CHAR_PER_LINE 20

#define DISP_LINE_1 0
#define DISP_LINE_2 1
#define DISP_LINE_3 2
#define DISP_LINE_4 3

#define CRSR_LINE_1 0  // 0x00
#define CRSR_LINE_2 64 // 0x40
#define CRSR_LINE_3 20 // 0x14
#define CRSR_LINE_4 84 // 0x54

class I2cDisp
{
public:
    // Constructor
    I2cDisp(bool echoMsg = false, uint8_t i2cAddr = I2C_ADDRESS_LCD);

    // Public method
    void init();
    void home();
    void clear();
    void clearln(uint8_t dispLine);
    void lcdSetCursor(uint8_t cursos_pos);
    void i2cWriteByte(uint8_t data);
    void i2cWriteCmd(uint8_t cmd);
    void i2cWriteStr(char *msg);
    void print(char *msg);
    void print(char *msg, uint8_t absCursor);
    void print(char *msg, uint8_t inLineCursor, uint8_t displayLine);
    void println(char *msg, uint8_t displayLine);

private:
    bool _uartMsg;
    uint8_t _i2cAddr;
};

extern I2cDisp Disp;

#endif