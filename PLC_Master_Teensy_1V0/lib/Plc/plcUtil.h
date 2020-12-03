#ifndef plcUtil_h
#define plcUtil_h

#if (ARDUINO >= 100)
#include "Arduino.h"
// #else
// #include "WProgram.h"
#endif

// ASCII code definitions
#define CR 0x0d // "Carriage Return"
#define LF 0x0a // "Line Feed"

// Definitions for PLC Devices
#define PLC_SLAVE_1 1
#define PLC_SLAVE_2 2
#define PLC_SLAVE_3 3
#define PLC_SLAVE_4 4
#define PLC_SLAVE_5 5
#define PLC_SLAVE_6 6
#define PLC_SLAVE_7 7

// Arduino Peripheral Device List
#define PLC_PORT_LCD 1
#define PLC_PORT_TEMP 2
#define PLC_PORT_SERVO 3
#define PLC_PORT_MOTOR 4
#define PLC_PORT_OTHER 255

// Commands specific to LCD
#define LCD_WRITE 1

// Commands specific to Temperature Sensor
#define TEMP_SENSE_READ 1

// Commands specific to Servo Motor
#define SERVO_SET_POSITION 1
#define SERVO_GET_POSITION 2

// Commands specific to Motor
#define SERVO_SET_RPM 1
#define SERVO_GET_RPM 2

#define NUM_PUSHBUTTONS 5
// define the pin numbers corresponding to pushbuttons
const uint8_t pushButton[NUM_PUSHBUTTONS] = {2, 3, 4, 5, 6};

uint8_t ascii2int(char *ascii, uint8_t index, uint8_t len, uint8_t base);
int char2int(char digit);
void initButtons(void);
uint8_t getButton(void);
void led_blink(uint8_t numFlashes, int t_on, int t_off);
float getTemperature(uint8_t tempSensor);
float getRpm(uint8_t servoMotor);
void printPacket(uint8_t dispLine, char buffSelect);

#endif