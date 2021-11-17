#include "outputs.h"
#include "globals.h"

Beeper::Beeper(){};
void Beeper::setBeep(uint32_t startTime, uint32_t duration, uint8_t repeatCount, uint32_t pauseDuration)
{
  start = startTime;
  end = startTime + duration / MAIN_CLOCK_TICK;
  repeat = repeatCount;
  if (pauseDuration)
    pause = pauseDuration;
  else
    pause = duration;
};
void Beeper::beep()
{
  if (repeat)
  {
    if ((mainClock_us_temp >= start) && (mainClock_us_temp < end) && ~(isActivated))
    {
      setOn();
      isActivated = true;
    }
    if (((mainClock_us_temp < start) || (mainClock_us_temp > end)) && isActivated)
    {
      setOff();
      isActivated = false;
      if (repeat > 1)
      {
        uint32_t dur = end - start;
        start = end + pause;
        end = start + dur;
      }
      repeat--;
    }
  }
};
void Beeper::beepOnce(){

};
void Beeper::beepTwice(){

};
void Beeper::setOn()
{
  PORTD |= _BV(BEEPER);
  active = true;
};
void Beeper::setOff()
{
  PORTD &= ~_BV(BEEPER);
  active = false;
};
uint32_t Beeper::getStart()
{
  return start;
}
uint32_t Beeper::getEnd()
{
  return end;
}
bool Beeper::isOn()
{
  return active;
};