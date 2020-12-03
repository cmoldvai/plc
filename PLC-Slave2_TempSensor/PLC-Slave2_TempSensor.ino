// #include <Arduino.h>  // Needed for PlatformIO
// PLC Temperature Sensor
// Written by Csaba Moldvai
// Date: Oct 2019

#define tempSensePin A0
#define VREF 1.375
#define mV_C 0.0225

char CR = 0x0d; // "Return"
char rx_buff[80]; // array to store received data

void setup() {
  Serial.begin (115200); // Need to configure SM2400 system for 9600
  while (!Serial){}  // wait for serial port to connect. Do we need this?   
  pinMode(tempSensePin, INPUT);
  Serial.write("ATO");   // set PLC to DATA mode
  Serial.write(CR);      // must send a CR
}  // end of setup

void loop() {
  // put your main code here, to run repeatedly:
  int cnt = 0;
  int i = 0;
  int msg_len;

  float VtempSense;
  float digTempSense;
  float temperature;
  static char fstr[15]; // used for float to string conversion

  if (Serial.available()) {
    msg_len = Serial.readBytes(rx_buff, 80); // read serial data from UART
    //Serial.println(msg_len);      // print message length
    //Serial.println(rx_buff);      // print data on serial monitor
    if(rx_buff[0] == '2') {     // If command is "READ"
      //Serial.println(rx_buff[0]);
      if(rx_buff[1] == '2') {   // 1: Temperature Sensor
        //Serial.println(rx_buff[1]);
        digTempSense = analogRead(tempSensePin); // must use 5.0 to correctly perform float operations
        VtempSense = 5.0 / 1023 * digTempSense;
        temperature = (VtempSense - VREF) / mV_C;
        //temperature1 = 0.217 * (digTempVal - 281.3);
        //  Serial.print(VtempSense);
        //  Serial.print("V ");
        //  Serial.println(temperature);
        delay(1000);
      } // end dealing with the peripheral
    } // end of the WRITE command
    Serial.write("SLAVE2: OK");
    Serial.write(CR);
    i = 0;
  } // end if Serial.available()

} // end main

// ***********************
// FUNCTION DEFINITIONS
// ***********************
