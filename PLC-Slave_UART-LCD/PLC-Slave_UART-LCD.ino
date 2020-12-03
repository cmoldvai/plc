// I2C Display
// Written by Csaba Moldvai
// Date: July 8, 2019

#include <Wire.h>

#define i2c_addr 0x28 // 0x50>>1
#define PREFIX        0xFE
#define DISP_ON       0x41
#define DISP_OFF      0x42
#define CRSR_SET      0x45
#define CRSR_HOME     0x46
#define DISP_CLR      0x51
#define DISP_1_LEFT   0x55
#define DISP_1_RGHT   0x56
#define DISP_FW_VER   0x70

char mystr[80]; // array to store received data

void setup() {
  //Serial.begin (9600);    // SM2400 system default baud-rate is 9600
  Serial.begin (115200); 

  // wait for serial port to connect
  while (!Serial)
  {
  }
  Serial.println ();
  Serial.println ("I2C display NewHaven NHD-0420D3Z-FL-GBW");

  Wire.begin();
  Wire.beginTransmission (i2c_addr);
  if (Wire.endTransmission () == 0) {
    Serial.println("ACK!");
  }
  // i2cWriteCmd(i2c_addr, DISP_ON);
  i2cWriteCmd(i2c_addr, DISP_CLR);
  i2cWriteByte(i2c_addr, '>');
}  // end of setup

void loop() {
int i = 0;
  if (Serial.available()) {
    i2cWriteCmd(i2c_addr, DISP_CLR);
    Serial.readBytes(mystr, 80); // read serial data from UART
    Serial.println(mystr);      // print data on serial monitor
    Serial.println ("Start sending characters to LCD display");
    lcdSetCursor(i2c_addr, 0);
    i2cWriteByte(i2c_addr, '>');
    while(mystr[i] != 0x0D) {
      if (isPrintable(mystr[i])) {
        i2cWriteByte(i2c_addr, mystr[i]);
        //delay(100);
      }
      i++;
      if(i==0) lcdSetCursor(i2c_addr, 0x00); // Line 1
      if(i==20) lcdSetCursor(i2c_addr, 0x40); // Line 2
      if(i==40) lcdSetCursor(i2c_addr, 0x14); // Line 3
      if(i==60) lcdSetCursor(i2c_addr, 0x54); // Line 4
      if(i==80) break;
    }
    i = 0;
  }
}


void i2cWriteByte(int device, byte data) {
  Wire.beginTransmission(device);
  Wire.write(data);
  Wire.endTransmission();
  delay(10);
}

void i2cWriteCmd(int device, byte cmd) {
  Wire.beginTransmission(device);
  Wire.write(PREFIX);
  Wire.write(cmd);
  Wire.endTransmission();
  delay(10);
}

void lcdSetCursor(int device, byte xyCursor) {
  //        Col_1  Col_20
  // Line1   0x00   0x0F
  // Line2   0x40   0x4F
  // Line3   0x14   0x27
  // Line4   0x54   0x67
  Wire.beginTransmission(device);
  Wire.write(PREFIX);
  Wire.write(CRSR_SET);
  Wire.write(xyCursor);
  Wire.endTransmission();
  delay(10);
}

//void lcdClrDispMemory(int device, char* mystr) {
//}

void i2cWrite(int device, unsigned int address, byte data) {
  Wire.beginTransmission(device);
  Wire.write((byte)(address >> 8));   // left part of pointer address
  Wire.write((byte)(address & 0xFF)); // and the right
  Wire.write(data);
  Wire.endTransmission();
  delay(10);
}

byte i2cRead(int device, unsigned int address) {
  byte result;
  Wire.beginTransmission(device);
  Wire.write((byte)(address >> 8));
  Wire.write((byte)(address & 0xFF));
  Wire.endTransmission();
  Wire.requestFrom(device, 1);
  result = Wire.read();
  return result;
}
