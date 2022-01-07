#ifndef _TIMER_H_
#define _TIMER_H_
#include <stdint.h>

#include "globals.h"

class Timer
{
public:
  Timer(const uint16_t *clockSource, uint16_t duration=SYS_FREQ);
  bool isActive();
  void setDuration(const uint16_t &mDuration_);
  void registerCallback(void (*func)());
  bool execute(bool run = true);

private:
  const uint16_t *pClockSource;
  //uint32_t mDefaultDuration = 10000 / MAIN_CLOCK_TICK;

  bool mActive;
  uint16_t mStartPoint = 0;
  uint16_t mDuration;
  void (*callback)();
};


#endif // _TIMER_H_
