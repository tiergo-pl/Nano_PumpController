#include "outputs.h"

Beeper::Beeper()
{
};
void Beeper::setOn()
{
  PORTD |= _BV(BEEPER);
};
void Beeper::setOff()
{
  PORTD &= ~_BV(BEEPER);
};
