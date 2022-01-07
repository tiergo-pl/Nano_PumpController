#include "timer.h"

Timer::Timer(const uint16_t *clockSource, uint16_t duration)
{
  pClockSource = clockSource;
  if (duration)
    mDuration = duration;
  else
    mDuration = SYS_FREQ;
}

bool Timer::isActive()
{
  return mActive;
}

void Timer::setDuration(const uint16_t &mDuration_)
{
  mDuration = mDuration_;
}

void Timer::registerCallback(void (*func)())
{
  callback = func;
}

bool Timer::execute(bool run)
{
  if (((*pClockSource - mStartPoint) >= mDuration) && run)
  {
    mStartPoint = *pClockSource;
    if (callback != nullptr)
    {
      callback();
      return true;
    }
  }
  return false;
}

