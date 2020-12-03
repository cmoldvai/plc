#include "plcModule.h"
#include "plcUtil.h"

#define DEBUG 0
#define TEENSY 1

// NOTE: in Teensy:
// Serial = communication via the UART2USB bridge (on USB cable to the host PC)
// Serial1 = communication on pins 2 (RX) and 3 (TX) of the Teensy board

PlcModule ::PlcModule(bool echoMsg, int plcBaudRate)
{
  // Constructor code. Place everything needed to initialze the instance
  _msg = echoMsg;
  _plcBaudRate = plcBaudRate;
}

///////////////////////////////////////////////////////////////////////////////////////
// This function is used to initialize the PLC object. 
// Reboots the PLC controller (to avoid having to manually push the RESET button after
// each Arduino code reload). It also sets PLC from "Command" to "Data" mode
// Arguments
// plcBaudRate: initializes the Baud Rate used by PLCs to communicate with each other
// new arguments to be added as needed (comms standard, ...)
///////////////////////////////////////////////////////////////////////////////////////
void PlcModule ::begin(uint32_t plcBaudRate)
{
  char msg1[4];
  char msg2[4];
  sprintf(msg1, "ATZ\r"); // CR (Carriage Return) = \r
  sprintf(msg2, "ATO\r"); // CR (Carriage Return) = \r
#if TEENSY
  Serial.begin(plcBaudRate);
  Serial1.begin(plcBaudRate);
  Serial1.write(msg1); // reboot PLC
  Serial1.write(msg2); // set PLC to DATA mode
#else
  Serial.begin(plcBaudRate);
  Serial.write(msg1); // set PLC to DATA mode
  Serial.write(msg2); // set PLC to DATA mode
#endif
}

///////////////////////////////////////////////////////////////////////////////////////
// available - checks if there is data in the RX buffer, using if(Serial.available())
// If there is data in the buffer, it decodes the packet and saves the extracted
// parameters into a structure. It verifies if the checksum is correct.
// If it is: it returns a '1' to the calling function
// Otherwise: it returns a '0'
///////////////////////////////////////////////////////////////////////////////////////
int PlcModule ::available(uint8_t devAddr)
{
  if (recvPacket() > 0)
  {                           // check if there is data in RX buffer
    processRxPacket(devAddr); // if yes, check if properly decoded
    return (1);               // if yes, return a 1
  }
  return (0); // else return a 0
}

///////////////////////////////////////////////////////////////////////////////////////
// sendPacket - assembles a packet to be sent over the PLC communications channel
// The packet consists of the following elements:
// 2 bytes: NumBytes = Number of bytes in the packet      [2 hexadecimal ASCII digits]
// 2 bytes: DestAddr = Destination Address                [2 hexadecimal ASCII digits]
// 2 bytes: SrcAddr  = Source Address                     [2 hexadecimal ASCII digits]
// 2 bytes: Port on the destination                       [2 hexadecimal ASCII digits]
// 2 bytes: Cmd = Command for the device on the specified Port  [2 hexadecimal ASCII digits]
// N bytes: Payload. The length of the payload is calculated as NumBytes - Overhead
// 2 bytes: Checksum                                      [2 hexadecimal ASCII digits]
// Overhead is currently 12 Bytes: NumBytes + DestAddr + SrcAddr + Port + Cmd + Checksum
///////////////////////////////////////////////////////////////////////////////////////
int PlcModule ::sendPacket(char *payload)
{
  txPacket.pLen = strlen(payload) + PACKET_OVERHEAD; // pLen(2)+destAddr(2)+port(2)+cmd(2)+checksum(2) = 10 bytes
  //Serial.println(txPacket.pLen);     // packet length
  sprintf(_txBuffer, "%02X%02X%02X%02X%02X", txPacket.pLen, txPacket.destAddr, txPacket.srcAddr, txPacket.port, txPacket.command);
  sprintf(&_txBuffer[PACKET_OVERHEAD - 2], "%s", payload);              // checksum will be attached to the end
  txPacket.calcChksum = computeChksum(_txBuffer, 0, txPacket.pLen - 2); // pktLen-2: calc checksum up to end of payload
  sprintf(&_txBuffer[txPacket.pLen - 2], "%02X\r", txPacket.calcChksum);

#if DEBUG
  Serial.println("");
  Serial.print("_txBuffer: ");
  Serial.println(_txBuffer);
  Serial.print("_txBuffer length: ");
  Serial.println(strlen(_txBuffer)); // has an extra '\r' at the end (lenght is len+1)
#endif

#if TEENSY
  Serial1.write(_txBuffer); // Send ASCII character to the PLC (UART port)
#else
  Serial.write(_txBuffer);
#endif
  // Serial1.flush(); // wait for transmission of data to complete. Slows program down significantly
  return (txPacket.pLen);
}

int PlcModule ::recvPacket()
{
  uint8_t recvBuffLen;
  char *pRxBuffer = _rxBuffer;                      // points to the beginning of the packet
  char *pPayload = _rxBuffer + PACKET_OVERHEAD - 2; // points to the beginning of the payload (packet data)

#if TEENSY
#else
#endif

#if TEENSY
  if (Serial1.available())
#else
  if (Serial.available())
#endif
  {
    // flushRxBuffer(); // to remove remnants of previous, longer packets *TODO: Check if needed

#if TEENSY
    recvBuffLen = Serial1.readBytesUntil('\r', pRxBuffer, RX_BUFFER_SIZE);
#else
    recvBuffLen = Serial.readBytesUntil('\r', pRxBuffer, RX_BUFFER_SIZE);
#endif
    rxPacket.pLen = ascii2int(pRxBuffer, 0, 2, 16);
    rxPacket.dLen = rxPacket.pLen - PACKET_OVERHEAD;
    rxPacket.destAddr = ascii2int(pRxBuffer, 2, 2, 16);
    rxPacket.srcAddr = ascii2int(pRxBuffer, 4, 2, 16);
    rxPacket.port = ascii2int(pRxBuffer, 6, 2, 16);
    rxPacket.command = ascii2int(pRxBuffer, 8, 2, 16);
    rxPacket.sentChksum = ascii2int(pRxBuffer, rxPacket.pLen - 2, 2, 16); // extract checksum
    rxPacket.calcChksum = computeChksum(pRxBuffer, 0, rxPacket.pLen - 2); // calculate checksum
    // memcpy(packet.payload, pPayload, packet.dLen);  // WORKS. Use it with binary data
    strncpy(rxPacket.payload, pPayload, rxPacket.dLen); // WORK with ASCII
    *(rxPacket.payload + rxPacket.dLen) = '\0';

    // // *********** DEBUG ********
    // // only one response sent from the Slave, but the Master processes two messages.
    // // almost as if after reading the rxBuffer, it is still not empty
    // pRxBuffer = _rxBuffer;
    //   if(Serial1.available()){
    //     Serial.println("looks like the buffer is not empty yet");
    //     recvBuffLen = Serial1.readBytes(pRxBuffer, 20);
    //     Serial.println(recvBuffLen);
    //     Serial.println(pRxBuffer);
    //   }
    //   Serial.println("just after checking if rxBuffer is empty");
    //   // ********** END DEBUG ********

    if (recvBuffLen != rxPacket.pLen)
    {
#if DEBUG
      uint8_t calcLen = strlen(pRxBuffer);
      Serial.println("ERROR: rx packet length mismatch"); // different embedded vs calculate lengths
      Serial.println(pRxBuffer);
      Serial.print("encoded packet length: ");
      Serial.println(rxPacket.pLen); // print the length of the _rxBuffer
      Serial.print("calculated packet length: ");
      Serial.println(calcLen); // print the length of the _rxBuffer
#endif
      return (-1);
    }
    return (rxPacket.pLen);
  } // end: if Serial.avaialble()
  else
  {
    // Serial.println("Serial not available");
    return (-1);
  }
}

int PlcModule ::processRxPacket(uint8_t devAddr)
{
  if (rxPacket.destAddr == devAddr)
  {
    if (rxPacket.calcChksum == rxPacket.sentChksum) // check if sent and calculated checksums are the same
    {
      // Serial.write("SLAVE ACK"); // if YES, acknowledge (send an ACK)
      // Serial.write(CR);
      // Serial.write(LF);
    }
    else
    {
      // Serial.write("ERROR: SLAVE NACK"); // if NOT, No acknowledge (send a NACK)
      // Serial.write(CR);
      // Serial.write(LF);
      led_blink(3, 100, 50);
      return (-1);
    }
    led_blink(2, 20, 20);
  }
  return (0);
}

char *PlcModule ::getRxBuffer()
{
  char *pRxBuffer = _rxBuffer;
  return (pRxBuffer);
}

uint8_t PlcModule ::flushRxBuffer()
{
  char *pRxBuffer = _rxBuffer;
  for (int i = 0; i < RX_BUFFER_SIZE; i++)
  {
    *(pRxBuffer + i) = '\0';
  }
  return (0);
}

int PlcModule ::computeChksum(char *buffer, uint8_t index, uint8_t len)
{
  int chksum = 0;
  for (int i = index; i < len; i++)
  {
    chksum += buffer[i];
  }
  chksum = chksum % 256;
  return chksum;
}
