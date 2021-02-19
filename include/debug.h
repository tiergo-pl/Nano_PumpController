#ifndef _debug_h_
#define _debug_h_

#include <avr/io.h>
/* removed due to stepper motor drive usage
#define debugPin0 PC0
#define debugPin1 PC1
#define debugPin2 PC2 */
#define debugPin3 PB4


/*
    PORTB |= _BV(debugPin3);
    PORTB &= ~_BV(debugPin3);
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    PORTB |= _BV(debugPin3);
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    PORTB &= ~_BV(debugPin3);
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    PORTB |= _BV(debugPin3);
    PORTB &= ~_BV(debugPin3);
*/




#endif