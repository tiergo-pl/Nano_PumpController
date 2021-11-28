#include "globals.h"

volatile uint32_t mainClock_us = 0; //Software counter incremented by timer interrupt
uint32_t mainClock_us_temp=0;
volatile uint32_t mainClock_seconds = 0; //Software counter incremented every 1 second
volatile uint32_t tickAtLastSec = 0;     //Used in 1 second timer calculations

volatile uint32_t clk1 = 0;
volatile uint32_t clk2 = 0;
volatile uint32_t clk3 = 0;
volatile uint32_t interval1;               //use variable for mutable value readable from eeprom
uint32_t *saved_interval1 = (uint32_t *)0; //and pointer to its saved value
volatile uint32_t interval2;               //use variable for mutable value readable from eeprom
uint32_t *saved_interval2 = (uint32_t *)4; //and pointer to its saved value
volatile uint32_t interval3;               //use variable for mutable value readable from eeprom
uint32_t *saved_interval3                 /* = (uint32_t *)8*/; //and pointer to its saved value

volatile uint32_t clkBuzzer;
volatile uint32_t intervalBuzzer;
uint32_t *saved_intervalBuzzer = (uint32_t *)12;

/* not used - to delete in future
uint8_t sequence1[SEQUENCE1_SIZE];           // array of pwm walues
uint8_t *saved_sequence1 = (uint8_t *)0x3a0; //pointer to array of pwm values in eeprom
uint8_t sequence2[SEQUENCE2_SIZE];           // array of pwm walues
uint8_t *saved_sequence2 = (uint8_t *)0x340; //pointer to array of pwm values in eeprom
uint8_t displaySeq[DISPLAY_SEQ_SIZE + 1];
uint8_t *saved_displaySeq = (uint8_t *)0x100;
*/

bool consoleDebugOn = false;

Beeper beeper(&BEEPER);
Pin ledBuiltin(&LED_BUILTIN);
Pin aeration(&AERATION);
Pin pump(&PUMP);
Pin kbMenu(&KB_MENU);
Pin kbUp(&KB_UP);
Pin kbDown(&KB_DOWN);
Pin debugDiode(&PORTD,7);

//needed function wrappers
/*void debugDiode_toggle()
{
  debugDiode.toggle();
}*/ 
//No need thanks to Lambdas???
