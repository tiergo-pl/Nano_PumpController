#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdint.h>
#include <avr/eeprom.h>

#include "debug.h"

//-----------------------------
// macros and vars used in software clock
// remember [extern] statement
#define MAIN_CLOCK_TICK 10             // 1 tick of mainClock_us duration in microseconds (max 128 due to hw limitations)
extern volatile uint32_t mainClock_us; // Software timer incremented by hw timer interrupt - System tick
extern uint32_t mainClock_us_temp;     // Copy of sw timer used in time calculations
extern uint32_t mainClock_seconds;     // Software counter incremented every 1 second
extern uint32_t tickAtLastSec;         // Used in 1 second timer calculations
extern int8_t minutesLeft;
extern int8_t hoursLeft;

/*extern volatile uint32_t clk1;
extern volatile uint32_t clk2;
extern volatile uint32_t clk3;
extern volatile uint32_t clkBuzzer; // buzzer tone generation
*/
// set following values in microsecs.:
//#define interval1 500000 / MAIN_CLOCK_TICK     //use macro for constant OR:
// uint32_t interval1 = 500000 / MAIN_CLOCK_TICK; //use variable for mutable value OR:
/*extern volatile uint32_t interval1; // use variable for mutable value readable from eeprom
extern uint32_t *saved_interval1;   // and pointer to its saved value
//#define interval2 1000000 / MAIN_CLOCK_TICK
//#define interval3 55500 / MAIN_CLOCK_TICK
extern volatile uint32_t interval2;      // use variable for mutable value readable from eeprom
extern uint32_t *saved_interval2;        // and pointer to its saved value
extern volatile uint32_t interval3;      // use variable for mutable value readable from eeprom
extern uint32_t *saved_interval3;        // and pointer to its saved value
extern volatile uint32_t intervalBuzzer; // buzzer tone generation
extern uint32_t *saved_intervalBuzzer;
*/
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
extern bool consoleDebugOn;
extern char debugString[120];

extern char uartInputString[64];
extern char cmdLine[64];

//-----------------------
// used gpio pins
#ifdef RELEASE_V1
#define LED_BUILTIN PORTB, PB5
#define AERATION PORTD, 4
#define PUMP PORTD, 5
#define KB_MENU PORTC, 3
#define KB_UP PORTB, 4
#define KB_DOWN PORTB, 3

#define BEEPER PORTB, 2

#define DISP_CLK PORTD, 2
#define DISP_DIO PORTD, 3
#else
#define LED_BUILTIN PORTB, PB5
#define AERATION PORTD, PD2
#define PUMP PORTD, PD3
#define KB_MENU PORTB, 0
#define KB_UP PORTB, 1
#define KB_DOWN PORTB, 2

#define BEEPER PORTD, PD4

#define DISP_CLK PORTD, 5
#define DISP_DIO PORTD, 6
#endif

#define KB_REFRESH_PERIOD 40            // in miliseconds
#define KB_LONG_PRESS_DURATION 500      // in miliseconds
#define KB_VERYLONG_PRESS_DURATION 3000 // in miliseconds
#define KB_BLOCK_DURATION 500           // in miliseconds

//--------------------

#include "pins.h"
#include "display_TM1637.h"
#include "timer.h"

// classes  ========================================================================================

class ProgramState
{
public:
  enum State
  {
    stateHold = 0,
    stateAeration,
    stateAfterAeration,
    statePumping,
    stateAfterPumping,
  };
  int8_t timer[(int)stateAfterPumping + 1][2];

  ProgramState();
  bool execute();
  void start();
  void recoverFromPowerLoss();
  void hold();
  void resume();
  void toggle();
  void nextState();
  void previousState();
  bool isRunning();
  void transit();
  void update();
  void someFunc()
  {
    int a;
    a++;
  }
  State currentState = stateHold;

private:
  State holdedState = stateAeration;
  bool transition = false;
  bool toUpdate = false;
};

class Menu
{
public:
  enum MenuEntry
  {
    rootLevel = 0,
    changeTimerAeration,
    changeTimerAfterAeration,
    changeTimerPumping,
    changeTimerAfterPumping,
  };

public:
  Menu();
  bool execute();
  MenuEntry getMenuLevel();
  void update();

private:
  MenuEntry menuLevel;
  bool toUpdate;
};

// EEPROM variables =================================================================================
extern int8_t EEMEM savedTimer[(int)ProgramState::stateAfterPumping + 1][2];
extern uint8_t EEMEM savedCurrentState;
extern uint8_t EEMEM savedHoldedState;
extern uint8_t EEMEM savedHoursLeft;
extern uint8_t EEMEM savedMinutesLeft;

// Global objects and variables----------------------------------------------------------------------------
extern DisplayTM1637 display;
extern uint8_t dispContent[];

extern Beeper beeper;
extern Pin ledBuiltin;
extern Pin aeration;
extern Pin pump;
extern Key kbMenu;
extern Key kbUp;
extern Key kbDown;
extern Pin debugDiode;
extern ProgramState mainProgramState;
extern Menu mainMenu;

#endif // _GLOBALS_H_
