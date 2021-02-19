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
  if (PORTC & _BV(PC0)){
    stepperClr();
    PORTC |= _BV(PC1);
  } else
  if (PORTC & _BV(PC1)){
    stepperClr();
    PORTC |= _BV(PC2);
  } else 
  if (PORTC & _BV(PC2)){
    stepperClr();
    PORTC |= _BV(PC3);
  } else
  if (PORTC & _BV(PC3)){
    stepperClr();
    PORTC |= _BV(PC0);
  }
}
