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

bool Timer::execute()
{
  if ((*pClockSource - mStartPoint) >= mDuration)
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

StateMachine::StateMachine()
{
  timer[stateAeration][0] = 3;       // Hours
  timer[stateAeration][1] = 15;      // Minutes
  timer[stateAfterAeration][0] = 1;  // Hours
  timer[stateAfterAeration][1] = 30; // Minutes
  timer[statePumping][0] = 0;        // Hours
  timer[statePumping][1] = 45;       // Minutes
  timer[stateAfterPumping][0] = 2;   // Hours
  timer[stateAfterPumping][1] = 20;  // Minutes
}

bool StateMachine::execute()
{
  if (currentState != stateHold)
  {
    switch (currentState)
    {
    case stateAeration:
      aeration.high_PullUp();
      break;
    case stateAfterAeration:
      /* code */
      break;
    case statePumping:
      /* code */
      break;
    case stateAfterPumping:
      /* code */
      break;

    default:
      break;
    }

    return true;
  }
  else
    return false;
}

void StateMachine::start()
{
  State holdedState = stateAeration;
}

void StateMachine::hold()
{
  holdedState = currentState;
  currentState = stateHold;
}

void StateMachine::resume()
{
  currentState = holdedState;
}

void StateMachine::nextState()
{
  switch (currentState)
  {
  case stateAeration:
    currentState = stateAfterAeration;
    break;
  case stateAfterAeration:
    currentState = statePumping;
    break;
  case statePumping:
    currentState = stateAfterPumping;
    break;
  case stateAfterPumping:
    currentState = stateAeration;
    break;

  default:
    break;
  }
}

void StateMachine::previousState()
{
  switch (currentState)
  {
  case stateAeration:
    currentState = stateAfterPumping;
    break;
  case stateAfterAeration:
    currentState = stateAeration;
    break;
  case statePumping:
    currentState = stateAfterAeration;
    break;
  case stateAfterPumping:
    currentState = statePumping;
    break;

  default:
    break;
  }
}
