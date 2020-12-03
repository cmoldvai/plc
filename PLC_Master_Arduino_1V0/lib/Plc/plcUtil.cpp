#include "plcUtil.h"
#include "plcModule.h"
#include "i2cDisp.h"

uint8_t buttonPressed[NUM_PUSHBUTTONS];
uint8_t buttonState[NUM_PUSHBUTTONS];
uint8_t buttonLastState[NUM_PUSHBUTTONS];

uint8_t ascii2int(char *ascii, uint8_t index, uint8_t len, uint8_t base)
// extracting a number represented by an ASCII string
// and converting it into an integer. Parameters:
// ascii = string containing the number we want to extract
// index = pointing to the beginning of the number within the ASCII string
// len = number of ASCII characters we want to extract and convert
// base = radix of the number e.g. 2=binary, 8=octal, 10=decimal, 16=hex
{
  uint8_t i;
  uint8_t digit = 0;
  uint8_t intValue = 0;

  if (2 <= base && base <= 16)
  {
    for (i = 0; i < len; i++)
    {
      // Extract a character
      char c = ascii[index + i];
      // Run the character through char2int
      digit = char2int(c);
      intValue = intValue * base + digit;
    }
    return (intValue);
  }
  return (-1);
}

int char2int(char digit)
{
  int converted = 0;
  digit = toupper(digit);
  if (digit >= '0' && digit <= '9')
  {
    converted = digit - '0';
  }
  else if (digit >= 'A' && digit <= 'F')
  {
    converted = digit - 'A' + 10;
  }
  else
  {
    converted = -1;
  }
  return converted;
}

void led_blink(uint8_t numFlashes, int t_on, int t_off)
{
  for (int i = 0; i < numFlashes; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(t_on);
    digitalWrite(LED_BUILTIN, LOW);
    delay(t_off);
  }
}

float getTemperature(uint8_t tempSensor)
{
  float temperature;
  float Tmin, Tmax;
  float xmax = 1000;

  switch (tempSensor)
  {
  case 0:
    Tmin = -10;
    Tmax = -9;
    //temperature = random(Tmin, Tmax);
    temperature = (Tmax - Tmin) / xmax * random(0, 1000) + Tmin;
    return (temperature);
    break;

  case 1:
    Tmin = 21;
    Tmax = 22.5;
    temperature = (Tmax - Tmin) / xmax * random(1000) + Tmin;
    return (temperature);
    break;

  default:
    return (-1);
    break;
  }
}

float getRpm(uint8_t servoMotor)
{
  float rpm;
  float RPMmin, RPMmax;

  switch (servoMotor)
  {
  case 0:
    RPMmin = 2200;
    RPMmax = 2550;
    //temperature = random(Tmin, Tmax);
    rpm = random(RPMmin, RPMmax);
    return (rpm);
    break;

  case 1:
    RPMmin = 60;
    RPMmax = 90;
    rpm = random(RPMmin, RPMmax);
    return (rpm);
    break;

  default:
    return (-1);
    break;
  }
}

void initButtons(void)
{
  for (int i = 0; i < NUM_PUSHBUTTONS; i++)
  {
    pinMode(pushButton[i], INPUT_PULLUP);
    buttonPressed[i] = 0;
    buttonState[i] = 1;
    buttonLastState[i] = 1;
  }
}

uint8_t getButton(void)
{
  uint8_t i;
  uint8_t pressedButton;

  for (i = 0; i < NUM_PUSHBUTTONS; i++)
  {
    buttonState[i] = digitalRead(pushButton[i]);
  }

  delay(50);

  // Detecting falling edge
  for (i = 0; i < NUM_PUSHBUTTONS; i++)
  {
    if (buttonState[i] != buttonLastState[i]) // change in button state
    {
      if (buttonState[i] == 0) // if '0', button was pressed
      {
        buttonPressed[i] = 1;
        buttonLastState[i] = 0;
      }
    }
    buttonLastState[i] = buttonState[i];
  }

  // checking which button was pressed
  for (i = 0; i < NUM_PUSHBUTTONS; i++)
  {
    if (buttonPressed[i])
    {
      // sprintf(str, "Button %d was pressed", i);
      // Serial.println(str);
      buttonPressed[i] = 0; // arm button press detector
      pressedButton = i + 1;
      return (pressedButton);
    }
  }
  return (0);
}

void printPacket(uint8_t dispLine, char buffSelect)
// buffSelect = 't': TX BUFFER
// buffSelect = 'r': RX BUFFER
{
  char str[3];
  uint8_t cursor = 0;
  char strRX[] = "RX:";
  char strTX[] = "TX:";

  Disp.clearln(dispLine);
  switch (buffSelect)
  {
  case 'r': // select RX Buffer
    Disp.print(strRX, cursor, dispLine);
    itoa(Plc.rxPacket.destAddr, str, 16); // destination address
    Disp.print(str, cursor += 3, dispLine);
    itoa(Plc.rxPacket.srcAddr, str, 16); // source address
    Disp.print(str, cursor += 3, dispLine);
    itoa(Plc.rxPacket.port, str, 16); // port
    Disp.print(str, cursor += 3, dispLine);
    itoa(Plc.rxPacket.command, str, 16); // command
    Disp.print(str, cursor += 3, dispLine);
    itoa(Plc.rxPacket.sentChksum, str, 16); // sent checksum
    Disp.print(str, cursor += 3, dispLine);
    itoa(Plc.rxPacket.calcChksum, str, 16); // calculated checksum
    Disp.print(str, cursor += 3, dispLine);
    break;
  case 't': // select TX BUFFER
    Disp.print(strTX, cursor, dispLine);
    itoa(Plc.txPacket.destAddr, str, 16); // destination address
    Disp.print(str, cursor += 3, dispLine);
    itoa(Plc.txPacket.srcAddr, str, 16); // source address
    Disp.print(str, cursor += 3, dispLine);
    itoa(Plc.txPacket.port, str, 16); // port
    Disp.print(str, cursor += 3, dispLine);
    itoa(Plc.txPacket.command, str, 16); // command
    Disp.print(str, cursor += 3, dispLine);
    itoa(Plc.txPacket.calcChksum, str, 16); // calculated checksum
    Disp.print(str, cursor += 3, dispLine);
    break;
  default:
    break;
  }
}
