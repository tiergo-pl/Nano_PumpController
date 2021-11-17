#include "outputs.h"
#include "globals.h"

Beeper::Beeper(){};
void Beeper::setBeep(uint32_t beepStartTime, uint32_t beepDuration)
{
  beepStart = beepStartTime;
  beepEnd = beepStartTime + beepDuration / MAIN_CLOCK_TICK;
};
void Beeper::beep()
{
  if ((mainClock_us_temp >= beepStart) && (mainClock_us_temp < beepEnd) && ~(beepActivated))
  {
    setOn();
    beepActivated = true;
  }
  if (((mainClock_us_temp < beepStart) || (mainClock_us_temp > beepEnd)) && beepActivated)
  {
    setOff();
    beepActivated = false;
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
  return beepStart;
}
uint32_t Beeper::getEnd()
{
  return beepEnd;
}
bool Beeper::isOn()
{
  return active;
};