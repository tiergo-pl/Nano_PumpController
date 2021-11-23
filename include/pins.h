#ifndef _pins_h_
#define _pins_h_

#include <avr/io.h>

class Pin
{
  private:
  public:
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