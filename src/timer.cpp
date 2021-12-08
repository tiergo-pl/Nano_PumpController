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

StateMachine::StateMachine()
{
  timer[stateAeration][0] = 4;      // Hours
  timer[stateAeration][1] = 4;      // Minutes
  timer[stateAfterAeration][0] = 2; // Hours
  timer[stateAfterAeration][1] = 5; // Minutes
  timer[statePumping][0] = 8;       // Hours
  timer[statePumping][1] = 6;       // Minutes
  timer[stateAfterPumping][0] = 12; // Hours
  timer[stateAfterPumping][1] = 7;  // Minutes
}

bool StateMachine::execute()
{
  if ((currentState != stateHold) && (toUpdate))
  {
    if (transition)
    {
      nextState();
      transition = false;
    }
    switch (currentState)
    {
    case stateAeration:
      aeration.high_PullUp();
      pump.low_HiZ();
      // hoursLeft = timer[stateAeration][0];
      // minutesLeft = timer[stateAeration][1];
      break;
    case stateAfterAeration:
      aeration.low_HiZ();
      pump.low_HiZ();
      // hoursLeft = timer[stateAfterAeration][0];
      // minutesLeft = timer[stateAfterAeration][1];
      break;
    case statePumping:
      aeration.low_HiZ();
      pump.high_PullUp();
      // hoursLeft = timer[statePumping][0];
      // minutesLeft = timer[statePumping][1];
      break;
    case stateAfterPumping:
      aeration.low_HiZ();
      pump.low_HiZ();
      // hoursLeft = timer[stateAfterPumping][0];
      // minutesLeft = timer[stateAfterPumping][1];
      break;

    default:
      break;
    }
    // debugDiode.toggle();
    hoursLeft = timer[currentState][0];
    minutesLeft = timer[currentState][1];
    toUpdate = false;
    return true;
  }
  else
    return false;
}

void StateMachine::start()
{
  currentState = stateAeration;
  update();
}

void StateMachine::hold()
{
  if (currentState != stateHold)
  {
    holdedState = currentState;
    currentState = stateHold;
  }
}

void StateMachine::resume()
{
  if (currentState == stateHold)
    currentState = holdedState;
}

void StateMachine::toggle()
{
  if (currentState == stateHold)
    resume();
  else
    hold();
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
  update();
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
  update();
}

bool StateMachine::isRunning()
{
  if (currentState == stateHold)
    return false;
  else
    return true;
}

void StateMachine::transit()
{
  transition = true;
  update();
}

void StateMachine::update()
{
  toUpdate = true;
}
