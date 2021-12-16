#include "globals.h"
#include "parse.h"

volatile uint32_t mainClock_us = 0; // Software counter incremented by timer interrupt
uint32_t mainClock_us_temp = 0;
uint32_t mainClock_seconds = 0; // Software counter incremented every 1 second
uint32_t tickAtLastSec = 0;     // Used in 1 second timer calculations
int8_t minutesLeft = 59;
int8_t hoursLeft = 23;

volatile uint32_t clk1 = 0;
volatile uint32_t clk2 = 0;
volatile uint32_t clk3 = 0;
volatile uint32_t interval1;                    // use variable for mutable value readable from eeprom
uint32_t *saved_interval1 = (uint32_t *)0;      // and pointer to its saved value
volatile uint32_t interval2;                    // use variable for mutable value readable from eeprom
uint32_t *saved_interval2 = (uint32_t *)4;      // and pointer to its saved value
volatile uint32_t interval3;                    // use variable for mutable value readable from eeprom
uint32_t *saved_interval3 /* = (uint32_t *)8*/; // and pointer to its saved value

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

DisplayTM1637 display(&PORTD, 5, &PORTD, 6);
uint8_t dispContent[] = {0xff, 0xff, 0xff, 0xff};
Beeper beeper(&BEEPER);
Pin ledBuiltin(&LED_BUILTIN);
Pin aeration(&AERATION);
Pin pump(&PUMP);
Key kbMenu(&KB_MENU);
Key kbUp(&KB_UP);
Key kbDown(&KB_DOWN);
Pin debugDiode(&PORTD, 7);

// needed function wrappers
/*void debugDiode_toggle()
{
  debugDiode.toggle();
}*/

ProgramState mainProgramState;
Menu mainMenu;
//--------------------------------------------------------------------------
void log(const char *text)
{
  uartTransmitString(text);
}
//--------------------------------------------------------------------------

ProgramState::ProgramState()
{
  timer[stateAeration][0] = 4;      // Hours
  timer[stateAeration][1] = 5;      // Minutes
  timer[stateAfterAeration][0] = 5; // Hours
  timer[stateAfterAeration][1] = 6; // Minutes
  timer[statePumping][0] = 6;       // Hours
  timer[statePumping][1] = 7;       // Minutes
  timer[stateAfterPumping][0] = 7;  // Hours
  timer[stateAfterPumping][1] = 8;  // Minutes
}

bool ProgramState::execute()
{
  if ((currentState != stateHold) && (toUpdate))
  {
    if (transition)
    {
      nextState();
      transition = false;
    }
    switch (currentState)
    {
    case stateAeration:
      aeration.high_PullUp();
      pump.low_HiZ();
      // hoursLeft = timer[stateAeration][0];
      // minutesLeft = timer[stateAeration][1];
      break;
    case stateAfterAeration:
      aeration.low_HiZ();
      pump.low_HiZ();
      // hoursLeft = timer[stateAfterAeration][0];
      // minutesLeft = timer[stateAfterAeration][1];
      break;
    case statePumping:
      aeration.low_HiZ();
      pump.high_PullUp();
      // hoursLeft = timer[statePumping][0];
      // minutesLeft = timer[statePumping][1];
      break;
    case stateAfterPumping:
      aeration.low_HiZ();
      pump.low_HiZ();
      // hoursLeft = timer[stateAfterPumping][0];
      // minutesLeft = timer[stateAfterPumping][1];
      break;

    default:
      break;
    }
    // debugDiode.toggle();
    hoursLeft = timer[currentState][0];
    minutesLeft = timer[currentState][1];
    toUpdate = false;
    return true;
  }
  else
    return false;
}

void ProgramState::start()
{
  currentState = stateAeration;
  update();
}

void ProgramState::hold()
{
  if (currentState != stateHold)
  {
    holdedState = currentState;
    currentState = stateHold;
  }
}

void ProgramState::resume()
{
  if (currentState == stateHold)
    currentState = holdedState;
}

void ProgramState::toggle()
{
  if (currentState == stateHold)
    resume();
  else
    hold();
}

void ProgramState::nextState()
{
  switch (currentState)
  {
  case stateAeration:
    currentState = stateAfterAeration;
    break;
  case stateAfterAeration:
    currentState = statePumping;
    break;
  case statePumping:
    currentState = stateAfterPumping;
    break;
  case stateAfterPumping:
    currentState = stateAeration;
    break;

  default:
    break;
  }
  update();
}

void ProgramState::previousState()
{
  switch (currentState)
  {
  case stateAeration:
    currentState = stateAfterPumping;
    break;
  case stateAfterAeration:
    currentState = stateAeration;
    break;
  case statePumping:
    currentState = stateAfterAeration;
    break;
  case stateAfterPumping:
    currentState = statePumping;
    break;

  default:
    break;
  }
  update();
}

bool ProgramState::isRunning()
{
  if (currentState == stateHold)
    return false;
  else
    return true;
}

void ProgramState::transit()
{
  transition = true;
  update();
}

void ProgramState::update()
{
  toUpdate = true;
}

//--------------------------------------------------------------------------
Menu::Menu()
{
  menuLevel = rootLevel;
  toUpdate = true;
}

/**
 * @brief Increade timer
 *
 * @param pTimer pointer to timer to change
 * @param max max timer value (e.g. 60 - minutes, 24 - hours)
 */

void timerIncrease(int8_t *pTimer, int8_t max)
{
  (*pTimer)++;
  if (*pTimer >= max)
    *pTimer = 0;
}

bool Menu::execute()
{
  if (toUpdate)
  {
    toUpdate = false;

    switch (menuLevel)
    {

    case rootLevel:
      kbMenu.registerCallback(
          []()
          {
            mainProgramState.toggle();
            beeper.beepTwice();
          },
          []()
          {
            mainMenu.menuLevel = changeTimerAeration;
            mainMenu.update();
            beeper.setBeep(mainClock_us_temp + 10000, 500000);
          },
          []()
          {
            debugDiode.toggle();
          },
          []()
          {
            beeper.setBeep(mainClock_us_temp + 10000, 500000, 4);
          });

      kbUp.registerCallback(
          []()
          {
            mainProgramState.previousState();
            debugDiode.high_PullUp();
            beeper.beepOnce();
          },
          nullptr, nullptr, nullptr);
      kbDown.registerCallback(
          []()
          {
            mainProgramState.nextState();
            debugDiode.low_HiZ();
            beeper.beepOnce();
          },
          nullptr, nullptr, nullptr);
      log("root level\n");
      break;

    case changeTimerAeration:
      kbMenu.registerCallback(
          []()
          {
            mainMenu.menuLevel = changeTimerAfterAeration;
            mainMenu.update();
            beeper.beepTwice();
          },
          []()
          {
            mainMenu.menuLevel = rootLevel;
            mainMenu.update();
            beeper.setBeep(mainClock_us_temp + 10000, 500000);
          },
          []()
          {
            debugDiode.toggle();
          },
          []()
          {
            beeper.setBeep(mainClock_us_temp + 10000, 500000, 4);
          });
      kbUp.registerCallback(
          []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAeration][0], 96);
            beeper.beepOnce();
          },
          nullptr, []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAeration][0], 96);
            beeper.beepOnce(); },
          nullptr);
      kbDown.registerCallback(
          []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAeration][1], 60);
            beeper.beepOnce();
          },
          nullptr, []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAeration][1], 60);
            beeper.beepOnce(); },
          nullptr);
      log("changeTimerAeration\n");
      break;
    case changeTimerAfterAeration:
      kbMenu.registerCallback(
          []()
          {
            mainMenu.menuLevel = changeTimerPumping;
            mainMenu.update();
            beeper.beepTwice();
          },
          []()
          {
            mainMenu.menuLevel = rootLevel;
            mainMenu.update();
            beeper.setBeep(mainClock_us_temp + 10000, 500000);
          },
          []()
          {
            debugDiode.toggle();
          },
          []()
          {
            beeper.setBeep(mainClock_us_temp + 10000, 500000, 4);
          });
      kbUp.registerCallback(
          []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAfterAeration][0], 96);
            beeper.beepOnce();
          },
          nullptr, []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAfterAeration][0], 96);
            beeper.beepOnce(); },
          nullptr);
      kbDown.registerCallback(
          []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAfterAeration][1], 60);
            beeper.beepOnce();
          },
          nullptr, []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAfterAeration][1], 60);
            beeper.beepOnce(); },
          nullptr);
      log("changeTimerAfterAeration\n");
      break;
    case changeTimerPumping:
      kbMenu.registerCallback(
          []()
          {
            mainMenu.menuLevel = changeTimerAfterPumping;
            mainMenu.update();
            beeper.beepTwice();
          },
          []()
          {
            mainMenu.menuLevel = rootLevel;
            mainMenu.update();
            beeper.setBeep(mainClock_us_temp + 10000, 500000);
          },
          []()
          {
            debugDiode.toggle();
          },
          []()
          {
            beeper.setBeep(mainClock_us_temp + 10000, 500000, 4);
          });
      kbUp.registerCallback(
          []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::statePumping][0], 96);
            beeper.beepOnce();
          },
          nullptr, []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::statePumping][0], 96);
            beeper.beepOnce(); },
          nullptr);
      kbDown.registerCallback(
          []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::statePumping][1], 60);
            beeper.beepOnce();
          },
          nullptr, []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::statePumping][1], 60);
            beeper.beepOnce(); },
          nullptr);
      log("changeTimerPumping\n");
      break;
    case changeTimerAfterPumping:
      kbMenu.registerCallback(
          []()
          {
            mainMenu.menuLevel = changeTimerAeration;
            mainMenu.update();
            beeper.beepTwice();
          },
          []()
          {
            mainMenu.menuLevel = rootLevel;
            mainMenu.update();
            beeper.setBeep(mainClock_us_temp + 10000, 500000);
          },
          []()
          {
            debugDiode.toggle();
          },
          []()
          {
            beeper.setBeep(mainClock_us_temp + 10000, 500000, 4);
          });
      kbUp.registerCallback(
          []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAfterPumping][0], 96);
            beeper.beepOnce();
          },
          nullptr, []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAfterPumping][0], 96);
            beeper.beepOnce(); },
          nullptr);
      kbDown.registerCallback(
          []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAfterPumping][1], 60);
            beeper.beepOnce();
          },
          nullptr, []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAfterPumping][1], 60);
            beeper.beepOnce(); },
          nullptr);
      log("changeTimerAfterPumping\n");
      break;
    default:
      break;
    }
    return true;
  }
  else
    return false;
}
Menu::MenuEntry Menu::getMenuLevel()
{
  return menuLevel;
}
void Menu::update()
{
  toUpdate = true;
}
