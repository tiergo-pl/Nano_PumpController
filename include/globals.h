#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdint.h>
#include <avr/eeprom.h>

#include "debug.h"


//-----------------------------
// macros and vars used in software clock
#define SYS_FREQ 1000 // in Hz, must be divisor of 250000
#define SYS_MILLISECONDS (SYS_FREQ/1000)
extern volatile uint16_t sysClkMaster;
extern uint16_t sysClk;
extern uint32_t clkSeconds32bit; // Software counter incremented every 1 second
extern uint16_t clkSeconds16bit;
extern int8_t minutesLeft;
extern int8_t hoursLeft;


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

#define PLOSS_DETECT PCINT8 // = PC0

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

#define PLOSS_DETECT PCINT8 // = PC0
#define DEBUG_DIODE_2 PB4
#endif

#define KB_REFRESH_PERIOD 40            // in miliseconds
#define KB_LONG_PRESS_DURATION 500      // in miliseconds
#define KB_VERYLONG_PRESS_DURATION 3000 // in miliseconds
#define KB_BLOCK_DURATION 500           // in miliseconds
#define SAVED_TIMERS {{0, 0}, {1, 30}, {0, 40}, {0, 5}, {0, 1}}
#define BRIGHTNESS_BLINK_HI 0x0b // Blinking higher brightness in settings change mode (0X0f - max, 0x08 - min)
#define BRIGHTNESS_BLINK_LO 0x0a // Brightness in keypad locked mode and blinking lower brightness in settings change mode
#define BRIGHTNESS 0x0d // Brightness in normal display mode

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
  State currentState = stateHold;

private:
  State holdedState = stateAeration;
  bool transition = false;
  bool toUpdate = false;
  bool recoveryFromPowerLoss = false;
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
    keypadLocked
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

// load defaults wrapper
void loadSaveDefaults();

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
