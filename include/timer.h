#ifndef _TIMER_H_
#define _TIMER_H_
#include <stdint.h>

#include "globals.h"

class Timer
{
public:
  Timer(const uint32_t *clockSource, uint32_t duration);
  bool isActive();
  void setDuration(const uint32_t &mDuration_);
  void registerCallback(void (*func)());
  void execute();

private:
  const uint32_t *pClockSource;
  uint32_t mDefaultDuration = 10000 / MAIN_CLOCK_TICK;

  bool mActive;
  uint32_t mStartPoint = 0;
  uint32_t mDuration;
  void (*callback)();
};

#endif // _TIMER_H_