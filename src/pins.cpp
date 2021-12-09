#include "pins.h"
#include "globals.h"

Pin::Pin(volatile uint8_t *port, uint8_t pinNo)
{
  mPinNo = pinNo;
  pPort = port;
  if (pPort == &PORTB)
  {
    pDdr = &DDRB;
    pPin = &PINB;
  }
  if (pPort == &PORTC)
  {
    pDdr = &DDRC;
    pPin = &PINC;
  }
  if (pPort == &PORTD)
  {
    pDdr = &DDRD;
    pPin = &PIND;
  }
}
void Pin::inputHiZ()
{
  *pDdr &= ~_BV(mPinNo); // input
  low_HiZ();
}
void Pin::inputPullUp()
{
  *pDdr &= ~_BV(mPinNo); // input
  high_PullUp();
}
void Pin::low_HiZ()
{
  *pPort &= ~_BV(mPinNo);
}
void Pin::high_PullUp()
{
  *pPort |= _BV(mPinNo);
}
void Pin::outputLow()
{
  *pDdr |= _BV(mPinNo); // output
  low_HiZ();
}
void Pin::outputHigh()
{
  *pDdr |= _BV(mPinNo); // output
  high_PullUp();
}
void Pin::toggle()
{
  *pPort ^= _BV(mPinNo);
  //(*this->pPin) |= _BV(mPinNo); //why not working??
}
bool Pin::readInput()
{
  return *pPin & _BV(mPinNo);
}
bool Pin::readOutput()
{
  return *pPort & _BV(mPinNo);
}
uint8_t Pin::getPinNo()
{
  return mPinNo;
}
volatile uint8_t *Pin::getPort()
{
  return pPort;
}
volatile uint8_t *Pin::getDdr()
{
  return pDdr;
}
volatile uint8_t *Pin::getPin()
{
  return pPin;
}

void Beeper::setBeep(uint32_t startTime, uint32_t duration, uint8_t repeatCount, uint32_t pauseDuration)
{
  start = startTime;
  end = startTime + duration / MAIN_CLOCK_TICK;
  repeat = repeatCount;
  if (pauseDuration)
    pause = pauseDuration / MAIN_CLOCK_TICK;
  else
    pause = duration / MAIN_CLOCK_TICK;
}
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
}
void Beeper::beepOnce()
{
  setBeep(mainClock_us_temp + 10000, 50000);
}
void Beeper::beepTwice()
{
  setBeep(mainClock_us_temp + 10000, 50000, 2);
}
void Beeper::setOn()
{
  // PORTD |= _BV(BEEPER);
  high_PullUp();
}
void Beeper::setOff()
{
  // PORTD &= ~_BV(BEEPER);
  low_HiZ();
}
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
  return readOutput();
}

void Key::registerCallback(void (*func)(), uint8_t keyFunction)
{
  switch (keyFunction)
  {
  case 1:
    longPressCallback = func;
    break;

  default:
    shortPressCallback = func;
    break;
  }
}

bool Key::execute()
{
  if (!readInput() && (keyState == 0))
  {
    keyState = 1; // predebounce
  }
  if (!readInput() && (keyState == 1))
  {
    keyState = 2; // debounced key, no function yet
  }
  if (readInput() && (keyState == 2))
  {
    keyState = 0; // debounced and short press

    if (shortPressCallback != nullptr)
    {
      shortPressCallback();
      return true;
    }
  }
  return false;
}
