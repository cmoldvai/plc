// Written by Csaba Moldvai
// Year: 2019
// This code was developed for MASTER PLC ARDUINO
//
// Packet format
// [PacketLength(2), destAddr(2), srcAddr, port(2), cmd(2), payload(N), checksum(2)]
// destAddr = 1
// Port(s):
//     1 = LCD Display
//     2 = Temp Sensor
//     3 = Servo Motor
//     4 = Proximity Sensor

#include <Arduino.h>
#include <Wire.h>
#include "plcModule.h"
#include "plcUtil.h"
#include "i2cDisp.h"

#define DEBUG 0

#define UART_BAUD_RATE 115200
#define PLC_BAUD_RATE 115200
#define RX_BUFFER_SIZE 64
#define TX_BUFFER_SIZE 64

// Classes
I2cDisp Disp(false, I2C_ADDRESS_LCD);
PlcModule Plc(true);

char rxBuffer[RX_BUFFER_SIZE]; // array to store received data

// Constant character strings
char msg1[] = "S1: Disp message";
char msg2[] = "S1: Get temp";
char msg3[] = "S2: Set position";
char msg4[] = "S2: Get rpm";
char version[] = "Ver MST-191122-1650";
char prompt[] = ">";

void setup()
{
  initButtons();                // initialize the keypad inputs
  pinMode(LED_BUILTIN, OUTPUT); // builtin LED
  Disp.init();
  Disp.clear();
  Disp.print(prompt);
  Plc.begin(PLC_BAUD_RATE);
}

void loop()
{
  uint8_t button;
  char istr[3]; // used for dec to ascii string conversion
  char str[20]; // used for string conversions mostly for display

  button = getButton(); // read the keypad and get the button number that was pressed
  sprintf(str, "Button %d pressed", button);

  switch (button)
  {
  case 1:
    Plc.txPacket.destAddr = PLC_SLAVE_1;
    Plc.txPacket.srcAddr = PLC_MASTER;
    Plc.txPacket.port = PLC_PORT_LCD;
    Plc.txPacket.command = LCD_WRITE;
    Plc.sendPacket(msg1);
    Disp.clear();
    Disp.println(msg1, DISP_LINE_1);
    printPacket(DISP_LINE_3, 't');
    break;

  case 2:
    Plc.txPacket.destAddr = PLC_SLAVE_1;
    Plc.txPacket.srcAddr = PLC_MASTER;
    Plc.txPacket.port = PLC_PORT_TEMP;
    Plc.txPacket.command = TEMP_SENSE_READ;
    Plc.sendPacket(msg2);
    Disp.clear();
    Disp.println(msg2, DISP_LINE_1);
    printPacket(DISP_LINE_3, 't');
    break;

  case 3:
    Plc.txPacket.destAddr = PLC_SLAVE_2;
    Plc.txPacket.srcAddr = PLC_MASTER;
    Plc.txPacket.port = PLC_PORT_SERVO;
    Plc.txPacket.command = SERVO_SET_POSITION;
    Plc.sendPacket(msg3);
    Disp.clear();
    Disp.println(msg3, DISP_LINE_1);
    printPacket(DISP_LINE_3, 't');
    break;

  case 4:
    Plc.txPacket.destAddr = PLC_SLAVE_2;
    Plc.txPacket.srcAddr = PLC_MASTER;
    Plc.txPacket.port = PLC_PORT_SERVO;
    Plc.txPacket.command = SERVO_GET_POSITION;
    Plc.sendPacket(msg4);
    Disp.clear();
    printPacket(DISP_LINE_3, 't');
    Disp.println(msg4, DISP_LINE_1);
    break;

  case 5:
    Disp.clear();
    Disp.println(version, DISP_LINE_1);
    break;

  default:
    break;
  }

  // check if there is data in RX buffer and if sent to the intended recepient
  if (Plc.available(PLC_MASTER))
  {
#if DEBUG
    Serial.println(Plc.rxPacket.destAddr);
    Serial.println(Plc.rxPacket.port);
    Serial.println(Plc.rxPacket.command);
    Serial.println(Plc.rxPacket.sentChksum);
    Serial.println(Plc.rxPacket.calcChksum);
#endif
    printPacket(DISP_LINE_4, 'r'); // Display received packet elements (RX) on the LCD

    ////////////////////////////////
    //  SLAVE 1
    //  [0 1 port cmd]
    ////////////////////////////////
    if (Plc.rxPacket.srcAddr == 1)
    {
      switch (Plc.rxPacket.port)
      {
      case 1: // PORT1 = [0 1 1 cmd]: DISPLAY
        if (Plc.rxPacket.command == 1)
        {
          sprintf(istr, "DispMsg on Slave1");
          Disp.println(istr, DISP_LINE_2);
        }
        break;

      case 2: // PORT2 = [0 1 2 cmd]: TEMPERATURE SENSOR
        if (Plc.rxPacket.command == 1)
        {
          sprintf(istr, "Slave1 temp: %s", Plc.rxPacket.payload);
          Disp.println(istr, DISP_LINE_2);
        }
        break;

      default:
        break;
      }
    }
    ////////////////////////////////
    //  SLAVE 2
    //  [0 2 port cmd]
    ////////////////////////////////
    else if (Plc.rxPacket.srcAddr == 2)
    {
      switch (Plc.rxPacket.port)
      {
      case 1: // PORT1 = [0 2 1 cmd]: DISPLAY
        if (Plc.rxPacket.command == 1)
        {
          sprintf(istr, "DispMsg on Slave2");
          Disp.print(istr, 0, DISP_LINE_2);
        }
        break;

      case 3: // PORT3 = [0 2 3 cmd]
        if (Plc.rxPacket.command == 1)
        {
          // sprintf(istr, "S2: RPM set to %s", Plc.rxPacket.payload);
          sprintf(istr, "S2: RPM set to 999");
          Disp.println(istr, DISP_LINE_2);
        }
        else if (Plc.rxPacket.command == 2)
        {
          sprintf(istr, "S2: rpm = %s", Plc.rxPacket.payload);
          Disp.println(istr, DISP_LINE_2);
        }
        break;

      default:
        break;
      }
    }
    else // rxdestAddr other than the tested choices
    {
      ;
    } // end: if (source address match) process the packet
  }   // end: if buffer not empty (new packet received)
} // end loop

