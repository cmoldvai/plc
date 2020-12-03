// Written by Csaba Moldvai
// Year: 2019
// This code was developed for Slave Address 1
//
// Packet format
// [PacketLength(2), DestAddr(2), SrcAddr(3), Port(2), Command(2), Payload(N), Checksum(2)]
// DeviceAddr = 2
// Port(s):
//     1 = LCD Display
//     2 = Temp Sensor
//     3 = Servo Motor
//     4 = Proximity Sensor

#include <Arduino.h>
#include "plcModule.h"
#include "plcUtil.h"
#include "i2cDisp.h"
#include "Wire.h"

#define DEBUG 0
#define UART_BAUD_RATE 115200
#define PLC_BAUD_RATE 115200
#define RX_BUFFER_SIZE 64
#define TX_BUFFER_SIZE 64

// Classes
I2cDisp Disp(true, I2C_ADDRESS_LCD);
PlcModule Plc(true);

// Function declarations
void printPacket(uint8_t dispLine, char buffSelect);

//char txBuffer[TX_BUFFER_SIZE];  // array to assemble data to be transmitted
char rxBuffer[RX_BUFFER_SIZE]; // array to store received data

// Constant strings

// Slave Arduino parameters
uint8_t servoMotor = 0; // the servo motor number (if more than 1)

// Globals
char version[] = "Ver SL2-191122-1420";

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT); // builtin LED
  Disp.init();
  Plc.begin(PLC_BAUD_RATE);
  Disp.clear();
  Disp.i2cWriteByte('>');
  // led_blink(10, 20, 20); // debug: confirm with LED flashes when setup completed
} // end of setup

void loop()
{
  char fstr[15]; // used for float to string conversion (TODO: check if this shall be static?)
  char str[20];
  float rpm;

  rpm = getRpm(servoMotor); // get a temperature reading
  dtostrf(rpm, 4, 0, fstr); // convert float to a string
  // strcpy(Plc.txPacket.payload, fstr);
  sprintf(str, "S2 RPM: %s", fstr);
  Disp.println(str, DISP_LINE_1);

  if (Plc.available(PLC_SLAVE_2))
  {

#if DEBUG
    Serial.println("*****************");
    Serial.print("Packet: ");
    Serial.println(Plc.getRxBuffer());
    Serial.print("Packet pLen: ");
    Serial.println(Plc.rxPacket.pLen);
    Serial.print("Packet destAddr: ");
    Serial.println(slAddr);
    Serial.print("Packet port: ");
    Serial.println(port);
    Serial.print("Packet command: ");
    Serial.println(cmd);
    Serial.print("data: ");
    Serial.println(Plc.rxPacket.payload);
    Serial.print("data length: ");
    Serial.println(Plc.rxPacket.dLen);
    Serial.print("Packet sent ckecksum: ");
    Serial.println(Plc.rxPacket.sentChksum);
    Serial.print("Packet calc checksum: ");
    Serial.println(Plc.rxPacket.calcChksum);
    Serial.println("*****************\n");
#endif

    /* This is the beginning of the main command processing loop */

    if (Plc.rxPacket.destAddr == PLC_SLAVE_2) // each PLC_SLAVE has its own capabilities
    {
      if (Plc.rxPacket.port == PLC_PORT_0)
      {
        switch (Plc.rxPacket.command)
        {
        case 1: // PORT0 = [2 0 1]: SYSTEM GET VERSION
          strcpy(Plc.txPacket.payload, version);
          Plc.sendPacket(Plc.txPacket.payload);
          Disp.println(Plc.rxPacket.payload, DISP_LINE_2);
          printPacket(DISP_LINE_3, 't'); // Display tx packet elements on the LCD
          printPacket(DISP_LINE_4, 'r'); // Display rx packet elements on the LCD
          break;

        default:
          break;
        }
      }
      else if (Plc.rxPacket.port == PLC_PORT_3)
      {
        switch (Plc.rxPacket.command)
        {
        case 1: // [2 3 1]: SERVO MOTOR, SET SERVO POSITION
          Plc.sendPacket(Plc.txPacket.payload);
          Disp.println(Plc.rxPacket.payload, DISP_LINE_2);
          printPacket(DISP_LINE_3, 't'); // Display tx packet elements on the LCD
          printPacket(DISP_LINE_4, 'r'); // Display rx packet elements on the LCD
          break;

        case 2: // [2 3 2] = SERVO MOTOR, GET SERVO POSITION
          strcpy(Plc.txPacket.payload, fstr); // copying current rpm into fstr for responding to request
          Plc.sendPacket(Plc.txPacket.payload); // sending the response with current rpm
          Disp.println(Plc.rxPacket.payload, DISP_LINE_2);
          printPacket(DISP_LINE_3, 't'); // Display tx packet elements on the LCD
          printPacket(DISP_LINE_4, 'r'); // Display rx packet elements on the LCD
          break;

        default:
          break;
        }
      }
      else // rxdestAddr other than the tested choices
      {
        //Disp.println(Plc.rxPacket.payload, DISP_LINE_2);
        printPacket(DISP_LINE_3, 't'); // Display tx packet elements on the LCD
        printPacket(DISP_LINE_4, 'r'); // Display rx packet elements on the LCD
      }                                // end: Port Address processing
    }                                  // end: Device Address processing
  }                                    // end: if packet received over Serial
  delay(100);
} // end loop

