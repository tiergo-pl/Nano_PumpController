#include "globals.h"
#include "parse.h"

volatile uint32_t mainClock_us = 0; // Software counter incremented by timer interrupt
uint32_t mainClock_us_temp = 0;
uint32_t mainClock_seconds = 0; // Software counter incremented every 1 second
uint32_t tickAtLastSec = 0;     // Used in 1 second timer calculations
int8_t minutesLeft = eeprom_read_byte(&savedMinutesLeft);
int8_t hoursLeft = eeprom_read_byte(&savedHoursLeft);

/*volatile uint32_t clk1 = 0;
volatile uint32_t clk2 = 0;
volatile uint32_t clk3 = 0;
volatile uint32_t interval1;
uint32_t *saved_interval1 = (uint32_t *)0;
volatile uint32_t interval2;
uint32_t *saved_interval2 = (uint32_t *)4;
volatile uint32_t interval3;
uint32_t *saved_interval3 ;

volatile uint32_t clkBuzzer;
volatile uint32_t intervalBuzzer;
uint32_t *saved_intervalBuzzer = (uint32_t *)12;
*/
bool consoleDebugOn = false;
char debugString[];

char uartInputString[] = "\0";
char cmdLine[] = "\0";

DisplayTM1637 display(&DISP_CLK, &DISP_DIO);
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
#ifdef DEBUG
  uartTransmitString(text);
#endif
}
//--------------------------------------------------------------------------

ProgramState::ProgramState()
{
  eeprom_read_block((void *)timer, (const void *)savedTimer, sizeof savedTimer);
  currentState = stateHold;
  holdedState = stateAeration;
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
void ProgramState::recoverFromPowerLoss()
{
  currentState = (State)eeprom_read_byte(&savedCurrentState);
  holdedState = (State)eeprom_read_byte(&savedHoldedState);
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
  {
    if (holdedState == stateHold)
      holdedState = stateAeration;                  //reset state in case of permanent state lock
    currentState = holdedState;
  }
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
            eeprom_update_byte(&savedCurrentState, (uint8_t)mainProgramState.currentState); // move to power loss routine
            eeprom_update_byte(&savedHoursLeft, (uint8_t)hoursLeft);                        // move to power loss routine
            eeprom_update_byte(&savedMinutesLeft, (uint8_t)minutesLeft);                    // move to power loss routine
            beeper.setBeep(mainClock_us_temp + 10000, 500000, 4);
          });

      kbUp.registerCallback(
          []()
          {
            mainProgramState.previousState();
            debugDiode.high_PullUp();
            beeper.beepOnce();
          },
          nullptr, nullptr,
          []()
          { 
            eeprom_read_block((void *)mainProgramState.timer, (const void *)savedTimer, sizeof savedTimer);
            beeper.setBeep(mainClock_us_temp + 10000, 500000, 2); });
      kbDown.registerCallback(
          []()
          {
            mainProgramState.nextState();
            debugDiode.low_HiZ();
            beeper.beepOnce();
          },
          nullptr, nullptr,
          []()
          { 
            eeprom_update_block( (const void *)mainProgramState.timer,(void *)savedTimer, sizeof savedTimer);
            beeper.setBeep(mainClock_us_temp + 10000, 500000, 3); });
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

// EEPROM variables =================================================================================

int8_t EEMEM savedTimer[(int)ProgramState::stateAfterPumping + 1][2] = {{0, 0}, {1, 30}, {0, 40}, {0, 5}, {0, 1}}; // Defaults in eeprom
uint8_t EEMEM savedCurrentState = (uint8_t)ProgramState::stateHold;
uint8_t EEMEM savedHoldedState = (uint8_t)ProgramState::stateAeration; // Defaults in eeprom
uint8_t EEMEM savedHoursLeft = 1;
uint8_t EEMEM savedMinutesLeft = 11;
