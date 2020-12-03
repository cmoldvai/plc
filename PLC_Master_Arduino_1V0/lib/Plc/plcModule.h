#ifndef plcModule_h
#define plcModule_h

#if (ARDUINO >= 100)
#include "Arduino.h"
// #else
// #include "WProgram.h"
#endif

// Definitions for PLC Devices
#define PLC_MASTER  0
#define PLC_SLAVE_1 1
#define PLC_SLAVE_2 2
#define PLC_SLAVE_3 3
#define PLC_SLAVE_4 4
#define PLC_SLAVE_5 5
#define PLC_SLAVE_6 6
#define PLC_SLAVE_7 7

// Arduino Peripheral Device List
#define PLC_PORT_0 0 // RESERVED
#define PLC_PORT_1 1 // LCD
#define PLC_PORT_2 2 // TEMP SENSOR
#define PLC_PORT_3 3 // SERVO
#define PLC_PORT_4 4 // MOTOR
#define PLC_PORT_5 5
#define PLC_PORT_6 6
#define PLC_PORT_7 7

// PLC parameters
#define RX_BUFFER_SIZE 64
#define TX_BUFFER_SIZE 64
#define PLC_BAUDRATE 9600
#define PACKET_OVERHEAD 12

typedef struct
{
    uint8_t pLen; // packet length
    uint8_t destAddr;
    uint8_t srcAddr;
    uint8_t port;
    uint8_t command;
    uint8_t dLen;                                       // payload (data) length
    uint8_t sentChksum;                                 // checksum embedded in the package
    uint8_t calcChksum;                                 // calculated checksum
    char payload[RX_BUFFER_SIZE - PACKET_OVERHEAD + 1]; // the +1 is for '\0'
} packetElements_t;

class PlcModule
{
public:
    // Constructor
    PlcModule(bool echoMsg = false, int plcBaudRate = PLC_BAUDRATE);

    // Public attributes
    packetElements_t rxPacket;
    packetElements_t txPacket;

    // Public methods
    void begin(uint32_t plcBaudRate = PLC_BAUDRATE);
    int available(uint8_t devAddr);
    int sendPacket(char *data);
    int recvPacket();
    int processRxPacket(uint8_t devAddr);
    char *getRxBuffer();
    uint8_t flushRxBuffer();

private:
    char _txBuffer[TX_BUFFER_SIZE];
    char _rxBuffer[RX_BUFFER_SIZE];
    bool _msg;
    int _plcBaudRate;
    int computeChksum(char *buffer, uint8_t index, uint8_t len); // calculates checksum of the packet (passed in a buffer)
};

extern PlcModule Plc;

#endif