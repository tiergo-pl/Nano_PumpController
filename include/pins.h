#ifndef _pins_h_
#define _pins_h_

#include <avr/io.h>

class Pin
{
protected:
  uint8_t mPinNo;

public:
  volatile uint8_t *pPort;
  volatile uint8_t *pDdr;
  volatile uint8_t *pPin;
  /*Initialize PIN
  *Choose PORTx and PINxy
  *@param port eeeee
  *@param pinNo fffff*/
  Pin(volatile uint8_t *port, uint8_t pinNo);
  void inputHiZ();
  void inputPullUp();
  void low_HiZ();     //switch output to Low or input to Hi-Z
  void high_PullUp(); //switch output to High or input to Pull-Up
  void outputLow();
  void outputHigh();
  void toggle(); //toggle PORT register - toggle output level or input pull-up resistor
  bool readInput();
  bool readOutput();
};

class Beeper
{
private:
  bool active;
  bool isActivated;
  uint8_t repeat;
  uint32_t start;
  uint32_t end;
  uint32_t pause;

public:
  Beeper();
  void setOn();
  void setOff();
  void setBeep(uint32_t startTime, uint32_t duration, uint8_t repeatCount = 1, uint32_t pauseDuration = 0); // defaults: repeatCount = 1, pauseDuration = duration
  void beep();
  void beepOnce();
  void beepTwice();
  uint32_t getStart();
  uint32_t getEnd();
  bool isOn();
};

#endif // !_pins_h_