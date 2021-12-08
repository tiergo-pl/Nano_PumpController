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
  bool execute(bool run = true);

private:
  const uint32_t *pClockSource;
  uint32_t mDefaultDuration = 10000 / MAIN_CLOCK_TICK;

  bool mActive;
  uint32_t mStartPoint = 0;
  uint32_t mDuration;
  void (*callback)();
};

class StateMachine
{
public:
  StateMachine();
  bool execute();
  void start();
  void hold();
  void resume();
  void toggle();
  void nextState();
  void previousState();
  bool isRunning();
  void transit();
  void update();

private:
  enum State
  {
    stateHold = 0,
    stateAeration,
    stateAfterAeration,
    statePumping,
    stateAfterPumping
  };
  State currentState = stateHold;
  State holdedState = stateAeration;
  bool transition = false;
  bool toUpdate = false;
  int8_t timer[(int)stateAfterPumping + 1][2];
};

#endif // _TIMER_H_
