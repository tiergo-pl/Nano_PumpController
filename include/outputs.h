#ifndef _outputs_h_
#define _outputs_h_

#include <avr/io.h>

class Beeper
{
private:
  bool active;
  bool beepActivated;
  bool beepOnceActivated;
  uint32_t beepStart;
  uint32_t beepEnd;

public:
  Beeper();
  void setOn();
  void setOff();
  void setBeep(uint32_t beepStartTime, uint32_t beepDuration);
  void beep();
  void beepOnce();
  void beepTwice();
  uint32_t getStart();
  uint32_t getEnd();
  bool isOn();
};

#endif // !_outputs_h_