#ifndef _globals_h_
#define _globals_h_

#include <stdint.h>
//-----------------------------
// macros and vars used in software clock
// remember [extern] statement
extern volatile uint32_t mainClock_us;      //Software timer incremented by hw timer interrupt - System tick
extern uint32_t mainClock_us_temp; //Copy of sw timer used in time calculations
extern volatile uint32_t mainClock_seconds; //Software counter incremented every 1 second
extern volatile uint32_t tickAtLastSec;     //Used in 1 second timer calculations

extern volatile uint32_t clk1;
extern volatile uint32_t clk2;
extern volatile uint32_t clk3;
extern volatile uint32_t clkBuzzer; // buzzer tone generation
#define MAIN_CLOCK_TICK 10          // 1 tick of mainClock_us duration in microseconds (max 128 due to hw limitations)
// set following values in microsecs.:
//#define interval1 500000 / MAIN_CLOCK_TICK     //use macro for constant OR:
//uint32_t interval1 = 500000 / MAIN_CLOCK_TICK; //use variable for mutable value OR:
extern volatile uint32_t interval1; //use variable for mutable value readable from eeprom
extern uint32_t *saved_interval1;   //and pointer to its saved value
//#define interval2 1000000 / MAIN_CLOCK_TICK
//#define interval3 55500 / MAIN_CLOCK_TICK
extern volatile uint32_t interval2;      //use variable for mutable value readable from eeprom
extern uint32_t *saved_interval2;        //and pointer to its saved value
extern volatile uint32_t interval3;      //use variable for mutable value readable from eeprom
extern uint32_t *saved_interval3;        //and pointer to its saved value
extern volatile uint32_t intervalBuzzer; // buzzer tone generation
extern uint32_t *saved_intervalBuzzer;

/* not used - to delete in future
#define SEQUENCE1_SIZE 0x60
#define SEQUENCE2_SIZE 0x60
extern uint8_t sequence1[SEQUENCE1_SIZE]; // array of pwm walues
extern uint8_t *saved_sequence1;          //pointer to array of pwm values in eeprom
extern uint8_t sequence2[SEQUENCE2_SIZE]; // array of pwm walues
extern uint8_t *saved_sequence2;          //pointer to array of pwm values in eeprom

#define DISPLAY_SEQ_SIZE 0x200
extern uint8_t displaySeq[DISPLAY_SEQ_SIZE + 1];
extern uint8_t *saved_displaySeq;
*/

extern bool consoleDebugOn; //debugging via serial port (console)

//-----------------------
// used gpio pins
#define LED_BUILTIN PORTB,PB5
#define AERATION PORTD,PD2
#define PUMP PORTD,PD3
#define KB_MENU PORTB,0
#define KB_UP PORTB,1
#define KB_DOWN PORTB,2

#define BEEPER PORTD,PD4
// #define Buzzer PC3
//--------------------

#include "pins.h"
extern Beeper beeper;
extern Pin ledBuiltin;
extern Pin aeration;
extern Pin pump;
extern Pin kbMenu;
extern Pin kbUp;
extern Pin kbDown;
extern Pin debugDiode;

#endif