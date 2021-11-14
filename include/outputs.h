#ifndef _outputs_h_
#define _outputs_h_

#include <avr/io.h>

#include "globals.h"

class Beeper
{
private:
public:
  Beeper();
  void setOn();
  void setOff();
  void beep(uint8_t beepDuration, uint8_t beepPause);
  void beepOnce();
  void beepTwice();
};


#endif // !_outputs_h_