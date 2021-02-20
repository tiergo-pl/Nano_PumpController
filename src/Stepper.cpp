#include "Stepper.h"

void stepperInit(){
  DDRC |= (_BV(DDC0) | _BV(DDC1) | _BV(DDC2) | _BV(DDC3));
  stepperClr();
  PORTC |= _BV(PC1);
}

void stepperClr(){
  PORTC &= ~(_BV(PC0) | _BV(PC1) | _BV(PC2) | _BV(PC3));
}

void stepUp(){
  uint8_t tempPORTC = PORTC & 0b00001111;
  stepperClr();
  switch (tempPORTC){
    case _BV(PC0):
      PORTC |= _BV(PC1);
    break;

    case _BV(PC1):
      PORTC |= _BV(PC2);
    break;

    case _BV(PC2):
      PORTC |= _BV(PC3);
    break;

    default:
      PORTC |= _BV(PC0);
    break;

  }
}

void stepHalfUp(){
  uint8_t tempPORTC = PORTC & 0b00001111;
  stepperClr();
  switch (tempPORTC){
    case _BV(PC0):
      PORTC |= (_BV(PC0) | _BV(PC1));
    break;

    case (_BV(PC0) | _BV(PC1)):
      PORTC |= _BV(PC1);
    break;

    case _BV(PC1):
      PORTC |= (_BV(PC1) | _BV(PC2));
    break;

    case (_BV(PC1) | _BV(PC2)):
      PORTC |= _BV(PC2);
    break;

    case _BV(PC2):
      PORTC |= (_BV(PC2) | _BV(PC3));
    break;

    case (_BV(PC2) | _BV(PC3)):
      PORTC |= _BV(PC3);
    break;

    case _BV(PC3):
      PORTC |= (_BV(PC3) | _BV(PC0));
    break;


    default:
      PORTC |= _BV(PC0);
    break;

  }
}
