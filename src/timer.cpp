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

void Timer::execute()
{
  if ((*pClockSource - mStartPoint) >= mDuration)
  {
    mStartPoint = *pClockSource;
    if (callback != nullptr)
    {
      callback();
    }
  }
}
