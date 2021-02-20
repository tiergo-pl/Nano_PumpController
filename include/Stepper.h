#ifndef _Stepper_h_
#define _Stepper_h_

#include <util/twi.h>


void stepperInit();
void stepperClr();
void stepUp();
void stepHalfUp();

#endif // !_Stepper_h_