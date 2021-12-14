#include "timer.h"

Timer::Timer(const uint32_t *clockSource, uint32_t duration)
{
  pClockSource = clockSource;
  if (duration)
    mDuration = duration;
  else
    mDuration = mDefaultDuration;
}

bool Timer::isActive()
{
  return mActive;
}

void Timer::setDuration(const uint32_t &mDuration_)
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

