#include "globals.h"
#include "parse.h"

// volatile uint32_t mainClock_us = 0; // Software counter incremented by timer interrupt
// uint32_t mainClock_us_temp = 0;
volatile uint16_t sysClkMaster;
uint16_t sysClk;
uint32_t clkSeconds32bit = 0; // Software counter incremented every 1 second
uint16_t clkSeconds16bit = 0;
int8_t minutesLeft = eeprom_read_byte(&savedMinutesLeft);
int8_t hoursLeft = eeprom_read_byte(&savedHoursLeft);

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

ProgramState mainProgramState;
Menu mainMenu;
//--------------------------------------------------------------------------
void log(const char *text)
{
#ifdef DEBUG //[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
  char uartOutputString[128];
  sprintf(uartOutputString, "sysClk= %u, uptime: %lu s, ", sysClk, clkSeconds32bit);
  uartTransmitString(uartOutputString);

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
    default:
    case stateAeration:
      aeration.high_PullUp();
      pump.low_HiZ();
      log("program state: aeration (1)\n");
      break;
    case stateAfterAeration:
      aeration.low_HiZ();
      pump.low_HiZ();
      log("program state: after aeration (2)\n");
      break;
    case statePumping:
      aeration.low_HiZ();
      pump.high_PullUp();
      log("program state: pumping (3)\n");
      break;
    case stateAfterPumping:
      aeration.low_HiZ();
      pump.low_HiZ();
      log("program state: after pumping (4)\n");
      break;
    }
    toUpdate = false;
    if (!recoveryFromPowerLoss)
    {
      hoursLeft = timer[currentState][0];
      minutesLeft = timer[currentState][1] - 1;
      if (minutesLeft < 0)
      {
        hoursLeft--;
        minutesLeft = 59;
        if (hoursLeft < 0)
        {
          if (prevStateExecuted)
          {
            previousState();
          }
          else
          {
            nextState();
          }
          log("omitting empty program state\n");
        }
        else
        {
          prevStateExecuted = false;
        }
      }
    }
    else
    {
      recoveryFromPowerLoss = false;
    }
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
  recoveryFromPowerLoss = true;
  update();
}
void ProgramState::hold()
{
  if (currentState != stateHold)
  {
    holdedState = currentState;
    currentState = stateHold;
    log("program state holded\n");
  }
}

void ProgramState::resume()
{

  if (currentState == stateHold)
  {
    if (holdedState == stateHold)
      holdedState = stateAeration; // reset state in case of permanent state lock
    currentState = holdedState;
    log("program state resumed\n");
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
  prevStateExecuted = false;
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
  prevStateExecuted = true;
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
  {
    return false;
  }
  else
  {
    return true;
  }
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
 * @brief Increase timer
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

    default:
    case rootLevel: //**************************************************************************************************
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
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 250 * SYS_MILLISECONDS);
          },
          nullptr,
          []()
          {
            mainMenu.menuLevel = keypadLocked;
            mainMenu.update();
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 1000 * SYS_MILLISECONDS);
          });

      kbUp.registerCallback(
          []()
          {
            mainProgramState.previousState();
            beeper.beepOnce();
          },
          nullptr, nullptr,
          []()
          { 
            eeprom_read_block((void *)mainProgramState.timer, (const void *)savedTimer, sizeof savedTimer);
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 500 * SYS_MILLISECONDS, 2); });
      kbDown.registerCallback(
          []()
          {
            mainProgramState.nextState();
            beeper.beepOnce();
          },
          nullptr, nullptr,
          []()
          { 
            eeprom_update_block( (const void *)mainProgramState.timer,(void *)savedTimer, sizeof savedTimer);
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 500 * SYS_MILLISECONDS, 3); });
      log("root level\n");
      break;

    case changeTimerAeration: //**************************************************************************************************
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
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 500 * SYS_MILLISECONDS);
          },
          nullptr,
          loadSaveDefaults);

      kbUp.registerCallback(
          []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAeration][0], 96);
            beeper.beepOnce();
          },
          nullptr,
          []()
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
          nullptr,
          []()
          {
            timerIncrease(&mainProgramState.timer[ProgramState::stateAeration][1], 60);
            beeper.beepOnce(); },
          nullptr);
      log("changeTimerAeration\n");
      break;
    case changeTimerAfterAeration: //**************************************************************************************************
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
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 500 * SYS_MILLISECONDS);
          },
          nullptr,
          loadSaveDefaults);
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
    case changeTimerPumping: //**************************************************************************************************
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
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 500 * SYS_MILLISECONDS);
          },
          nullptr,
          loadSaveDefaults);
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
    case changeTimerAfterPumping: //**************************************************************************************************
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
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 500 * SYS_MILLISECONDS);
          },
          nullptr,
          loadSaveDefaults);
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
    case keypadLocked: //**************************************************************************************************
      kbMenu.registerCallback(
          []()
          {
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 50 * SYS_MILLISECONDS, 4, 100 * SYS_MILLISECONDS);
          },
          nullptr, nullptr,
          []()
          {
            mainMenu.menuLevel = rootLevel;
            mainMenu.update();
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 1000 * SYS_MILLISECONDS);
          });

      kbUp.registerCallback(
          []()
          {
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 50 * SYS_MILLISECONDS, 4, 100 * SYS_MILLISECONDS);
          },
          nullptr, nullptr,
          []()
          {
            mainMenu.menuLevel = rootLevel;
            mainMenu.update();
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 1000 * SYS_MILLISECONDS);
          });
      kbDown.registerCallback(
          []()
          {
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 50 * SYS_MILLISECONDS, 4, 100 * SYS_MILLISECONDS);
          },
          nullptr, nullptr,
          []()
          {
            mainMenu.menuLevel = rootLevel;
            mainMenu.update();
            beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 1000 * SYS_MILLISECONDS);
          });
      log("keypad locked\n");
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

int8_t EEMEM savedTimer[(int)ProgramState::stateAfterPumping + 1][2] = SAVED_TIMERS; // Defaults in eeprom
uint8_t EEMEM savedCurrentState = (uint8_t)ProgramState::stateHold;
uint8_t EEMEM savedHoldedState = (uint8_t)ProgramState::stateAeration; // Defaults in eeprom
uint8_t EEMEM savedHoursLeft = 1;
uint8_t EEMEM savedMinutesLeft = 11;

// load defaults wrapper
void loadSaveDefaults()
{
  int8_t temp[(int)ProgramState::stateAfterPumping + 1][2] = SAVED_TIMERS;
  eeprom_write_block((const void *)temp, (void *)savedTimer, sizeof savedTimer);
  eeprom_busy_wait();
  eeprom_read_block((void *)mainProgramState.timer, (const void *)savedTimer, sizeof savedTimer);
  eeprom_busy_wait();
  mainProgramState.start();
  beeper.setBeep(sysClk + 100 * SYS_MILLISECONDS, 500 * SYS_MILLISECONDS, 4);
}
