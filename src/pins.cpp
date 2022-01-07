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

void Beeper::setBeep(uint16_t startTime, uint16_t duration, uint8_t repeatCount, uint16_t pauseDuration)
{
  start = startTime;
  end = startTime + duration;
  repeat = repeatCount;
  if (pauseDuration)
    pause = pauseDuration;
  else
    pause = duration;
}
void Beeper::beep()
{
  if (repeat)
  {
    if ((sysClk >= start) && (sysClk < end) && ~(isActivated))
    {
      setOn();
      isActivated = true;
    }
    if (((sysClk < start) || (sysClk > end)) && isActivated)
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
  setBeep(sysClk + 1000, 500);
}
void Beeper::beepTwice()
{
  setBeep(sysClk + 1000, 500, 2);
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
uint16_t Beeper::getStart()
{
  return start;
}
uint16_t Beeper::getEnd()
{
  return end;
}
bool Beeper::isOn()
{
  return readOutput();
}

void Key::registerCallback(void (*shortPressFunc)(), void (*longPressFunc)(), void (*longPressingFunc)(), void (*veryLongPressFunc)())
{
  pShortPressCallback = shortPressFunc;
  pLongPressCallback = longPressFunc;
  pLongPressingCallback = longPressingFunc;
  pVeryLongPressCallback = veryLongPressFunc;
}

bool Key::execute()
{
  if (pressed() && (keyState == 0))
  {
    keyState = 1; // predebounce
  }
  if (pressed() && (keyState == 1))
  {
    keyState = 2; // debounced key, no function yet
  }
  if (pressed() && (keyState == 2))
  {
    pressTime++;                                                // measure press lenght
    if (pressTime > KB_LONG_PRESS_DURATION / KB_REFRESH_PERIOD) // number of keyboard readouts when long pressed
      keyState = 3;
  }
  if (pressed() && (keyState == 3))
  {
    pressTime++;
    if (pressTime > KB_VERYLONG_PRESS_DURATION / KB_REFRESH_PERIOD) // number of keyboard readouts when very long pressed
      keyState = 4;
  }
  if (pressed() && (keyState == 4))
  {
    pressTime = -1; //infinitely keep in keyState = 4 until pressed
  }

  if ((keyState == 5)) // temporary keyboard blocking
  {
    if (pressTime < KB_BLOCK_DURATION / KB_REFRESH_PERIOD) // blocking time in keyboard readouts
      pressTime++;
    else
    {
      keyState = 0;
      pressTime = 0;
    }
  }

  if (!pressed() && (keyState == 2)) // debounced and short press
  {
    keyState = 0;

    if (pShortPressCallback != nullptr)
    {
      pShortPressCallback();
    }
    pressTime = 0;
    return true;
  }

  if (!pressed() && (keyState == 3)) // debounced and long press
  {
    keyState = 0;

    if (pLongPressCallback != nullptr)
    {
      pLongPressCallback();
    }
    pressTime = 0;
    return true;
  }

  if (pressed() && ((keyState == 3)||(keyState == 4))) // long pressing
  {
    if (pLongPressingCallback != nullptr)
    {
      pLongPressingCallback();
    }
  }

  if (!pressed() && (keyState == 4)) // very long press
  {
    keyState = 5;
    if (pVeryLongPressCallback != nullptr)
    {
      pVeryLongPressCallback();
    }
    pressTime = 0;
    return true;
  }
  return false;
}

bool Key::pressed()
{
  return !readInput();
}
