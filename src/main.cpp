// #include <Arduino.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/atomic.h>
#include <util/delay.h>
//#include <avr/eeprom.h>

//#include "uart.h"
#include "parse.h"
//#include "display_TM1637.h"
//#include "timer.h"
#include "globals.h"
//#include "pins.h"

void timerInit(void)
{
  // divide F_CPU by 1600 - gives 10 kHz system clock
  TCCR2A = 0b00000010; //((1<<WGM21) | (0<<WGM20));        // Timer2 CTC mode
#if F_CPU == 16000000L
  TCCR2B = ((1 << CS22) | (0 << CS21) | (0 << CS20)); // prescaler clk/64 - 16 MHz clock
#endif
#if F_CPU == 8000000L
  TCCR2B = ((0 << CS22) | (1 << CS21) | (1 << CS20)); // prescaler clk/32 - 8 MHz clock
#endif
  OCR2A = (250000 / SYS_FREQ - 1); // count to 250000/SYS_FREQ (divide prescaled freq by 250000/SYS_FREQ), max value is 255!
  TIMSK2 = (1 << OCIE2A);          // Timer/Counter Interrupt Mask
}

void portInit(void)
{
  // DDRB = (1 << LED_BUILTIN) | _BV(debugPin3); // Led builtin (13)
  DDRC &= ~_BV(PLOSS_DETECT);
  PORTC |= _BV(PLOSS_DETECT);
#ifdef DEBUG_DIODE_2
  DDRB |= _BV(DEBUG_DIODE_2);
#endif
  // DDRD = ((1 << BEEPER));
  beeper.outputLow();
  ledBuiltin.outputLow();
  aeration.outputLow();
  pump.outputLow();
  kbMenu.inputPullUp();
  kbUp.inputPullUp();
  kbDown.inputPullUp();
  debugDiode.outputLow();
}

void pcintInit()
{
  PCMSK1 = _BV(PLOSS_DETECT);
  PCICR = _BV(PCIE1);
}

ISR(PCINT1_vect) // interrupt from power loss detection pin
{
#ifdef DEBUG_DIODE_2
  PORTB |= _BV(DEBUG_DIODE_2);
#endif
  if (~(PINC & _BV(PLOSS_DETECT)))
  {
    eeprom_update_byte(&savedCurrentState, (uint8_t)mainProgramState.currentState);
    eeprom_update_byte(&savedHoldedState, (uint8_t)mainProgramState.holdedState);
    eeprom_update_byte(&savedHoursLeft, (uint8_t)hoursLeft);
    eeprom_update_byte(&savedMinutesLeft, (uint8_t)minutesLeft);
    eeprom_busy_wait();
    beeper.setOn();
    _delay_ms(4000);
    beeper.setOff();
    _delay_ms(1000);
  }
#ifdef DEBUG_DIODE_2
  PORTB &= ~_BV(DEBUG_DIODE_2);
#endif
  beeper.setOn();
  _delay_ms(2);
  beeper.setOff();
}

ISR(TIMER2_COMPA_vect) // TIMER2 interrupt
{
  //  PORTC |= _BV(debugPin1);
  // mainClock_us++;
  sysClkMaster++;
  //  PORTC &= ~_BV(debugPin1);
}

void halfSecondRoutine()
{
  if (aeration.readOutput())
  {
    display.prepareDots(0x01, 1, 0);
  }
  else
  {
    display.prepareDots(0x0, 1, 0);
  }
  if (pump.readOutput())
  {
    display.prepareDots(0x01, 1, 3);
  }
  else
  {
    display.prepareDots(0x0, 1, 3);
  }
  if (ledBuiltin.readOutput() && mainProgramState.isRunning())
    display.prepareDots(0, 1, 1);
  else
    display.prepareDots(0x01, 1, 1);
  if (mainMenu.getMenuLevel()) // true if menulevel != rootLevel
  {
    if (ledBuiltin.readOutput())
    {
      if (mainMenu.getMenuLevel() != Menu::MenuEntry::keypadLocked)
        display.setBrightness(BRIGHTNESS_BLINK_HI); // Blinking higher brightness in settings change mode
    }
    else
      display.setBrightness(BRIGHTNESS_BLINK_LO); // Brightness in keypad locked and blinking lower brightness in settings change mode
  }
  else
    display.setBrightness(BRIGHTNESS); // Brightness in normal display mode
  ledBuiltin.toggle();
}

void refreshDisplayKeyboardRoutine()
{
  kbMenu.execute();
  kbUp.execute();
  kbDown.execute();
  switch (mainMenu.getMenuLevel())
  {
  default:
  case Menu::rootLevel:
  case Menu::keypadLocked:
    display.prepareSegments(display.toBcd(minutesLeft, dispContent, 2), 2, 2); // displays current minute counter
    display.prepareSegments(display.toBcd(hoursLeft, dispContent, 2), 2, 0);   // displays current hour counter
    break;

  case Menu::MenuEntry::changeTimerAeration:
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::stateAeration][1], dispContent, 2), 2, 2); // displays set minute counter
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::stateAeration][0], dispContent, 2), 2, 0); // displays set hour counter
    break;

  case Menu::MenuEntry::changeTimerAfterAeration:
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::stateAfterAeration][1], dispContent, 2), 2, 2); // displays set minute counter
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::stateAfterAeration][0], dispContent, 2), 2, 0); // displays set hour counter
    break;

  case Menu::MenuEntry::changeTimerPumping:
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::statePumping][1], dispContent, 2), 2, 2); // displays set minute counter
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::statePumping][0], dispContent, 2), 2, 0); // displays set hour counter
    break;

  case Menu::MenuEntry::changeTimerAfterPumping:
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::stateAfterPumping][1], dispContent, 2), 2, 2); // displays set minute counter
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::stateAfterPumping][0], dispContent, 2), 2, 0); // displays set hour counter
    break;
  }
  /*
  if (mainMenu.getMenuLevel() == Menu::rootLevel || mainMenu.getMenuLevel() == Menu::keypadLocked)
  {
    display.prepareSegments(display.toBcd(minutesLeft, dispContent, 2), 2, 2); // displays current minute counter
    display.prepareSegments(display.toBcd(hoursLeft, dispContent, 2), 2, 0);   // displays current hour counter
  }
  if (mainMenu.getMenuLevel() == Menu::MenuEntry::changeTimerAeration)
  {
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::stateAeration][1], dispContent, 2), 2, 2); // displays set minute counter
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::stateAeration][0], dispContent, 2), 2, 0); // displays set hour counter
  }
  if (mainMenu.getMenuLevel() == Menu::MenuEntry::changeTimerAfterAeration)
  {
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::stateAfterAeration][1], dispContent, 2), 2, 2); // displays set minute counter
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::stateAfterAeration][0], dispContent, 2), 2, 0); // displays set hour counter
  }
  if (mainMenu.getMenuLevel() == Menu::MenuEntry::changeTimerPumping)
  {
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::statePumping][1], dispContent, 2), 2, 2); // displays set minute counter
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::statePumping][0], dispContent, 2), 2, 0); // displays set hour counter
  }
  if (mainMenu.getMenuLevel() == Menu::MenuEntry::changeTimerAfterPumping)
  {
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::stateAfterPumping][1], dispContent, 2), 2, 2); // displays set minute counter
    display.prepareSegments(display.toBcd(mainProgramState.timer[ProgramState::stateAfterPumping][0], dispContent, 2), 2, 0); // displays set hour counter
  }
  */
}
void minuteTickDownwardsRoutine()
{
  minutesLeft--;
  if (minutesLeft < 0)
  {
#ifdef DEBUG
    minutesLeft = 19; // Quicker time flow (to debug)
#else
    minutesLeft = 59;
#endif
    hoursLeft--;
    if (hoursLeft < 0)
      mainProgramState.transit();
  }
}
void debugRoutine()
{
  sprintf(debugString, "uptime: %lus, PORT_addr %p PORT=%x, DDR_addr %p DDR=%x, PIN_addr %p PIN=%x\n",
          clkSeconds32bit, pump.getPort(), *pump.getPort(), pump.getDdr(), *pump.getDdr(), pump.getPin(), *pump.getPin());
  uartTransmitString(debugString);
  sprintf(debugString, "uptime: %lus, PORT_addr %p PORT=%x, DDR_addr %p DDR=%x, PIN_addr %p PIN=%x\n",
          clkSeconds32bit, kbMenu.getPort(), *kbMenu.getPort(), kbMenu.getDdr(), *kbMenu.getDdr(), kbMenu.getPin(), *kbMenu.getPin());
  uartTransmitString(debugString);
}
void consoleInput()
{
  if (uartEcho())
  {
    if (sizeof(uartInputString) < (strlen(uartInputString) + 2))
      uartInputString[0] = 0;
    uartReceiveString(uartInputString);
    if (detectEndlCmdline(cmdLine, uartInputString))
    {
      parseCmdline(cmdLine);
      uartTransmitString("\r\n");
    }
  }
}
// main starts here
// main starts here
int main()
{
  timerInit();
  portInit();
  uartInit();
  sei();

  beeper.setBeep(0, 500 * SYS_MILLISECONDS); // initial beep on system start

  // Initial display test
  display.prepareSegments(dispContent);
  display.prepareDots(0x0f);
  display.execute();

  // Prepare one second timer
  Timer oneSecondTick(&sysClk, 1 * SYS_FREQ);
  oneSecondTick.registerCallback(
      []()
      { clkSeconds32bit++;
      clkSeconds16bit= (uint16_t)clkSeconds32bit; }); // Lambda function wrapper - gives pointer to function

  // Prepare half second timer
  Timer halfSecondTick(&sysClk, 0.5 * SYS_FREQ);
  halfSecondTick.registerCallback(halfSecondRoutine);

  Timer refreshDisplayKeyboard(&sysClk, (KB_REFRESH_PERIOD * 0.001 * SYS_FREQ));
  refreshDisplayKeyboard.registerCallback(refreshDisplayKeyboardRoutine);

// MINUTES AND HOURS
#ifdef DEBUG
  Timer minuteTickDownwards(&clkSeconds16bit, 2); // CHANGE TO (&clkSeconds32bit, 60) !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#else
  Timer minuteTickDownwards(&clkSeconds16bit, 60);
#endif
  minuteTickDownwards.registerCallback(minuteTickDownwardsRoutine);

  Timer debugTimer(&sysClk, 2 * SYS_FREQ);
  debugTimer.registerCallback(debugRoutine);
  // MAIN PROGRAM LOOP
  // MAIN PROGRAM LOOP
  // MAIN PROGRAM LOOP
  while (1)
  {
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
      // mainClock_us_temp = mainClock_us;
      sysClk = sysClkMaster;
    }
#ifdef DEBUG //[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
    debugDiode.high_PullUp();
    debugDiode.low_HiZ();
#endif
    beeper.beep();
#ifdef DEBUG //[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
    debugDiode.high_PullUp();
    debugDiode.low_HiZ();
#endif

    // reset output states after initial test
    if (!beeper.isOn() && clkSeconds32bit == 0)
    {
      display.prepareSegments(display.toBcd(clkSeconds32bit, dispContent));
      aeration.outputLow();
      pump.outputLow();
      display.prepareDots(0x0);
      if (sysClk > 900 * SYS_MILLISECONDS)
      {
        mainProgramState.recoverFromPowerLoss();
        beeper.beepOnce();
        pcintInit(); // enabling save on power loss
      }
    }
    else
    {
      refreshDisplayKeyboard.execute();
      mainProgramState.execute();
      mainMenu.execute();
    }
#ifdef DEBUG //[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
    debugDiode.high_PullUp();
    debugDiode.low_HiZ();
#endif

    oneSecondTick.execute(); // increment every second
#ifdef DEBUG                 //[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
    debugDiode.high_PullUp();
    debugDiode.low_HiZ();
#endif

    minuteTickDownwards.execute(mainProgramState.isRunning());
#ifdef DEBUG //[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
    debugDiode.high_PullUp();
    debugDiode.low_HiZ();
#endif

    display.execute();
#ifdef DEBUG //[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
    debugDiode.high_PullUp();
    debugDiode.low_HiZ();
#endif
    halfSecondTick.execute();

#ifdef DEBUG //[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
    debugDiode.high_PullUp();
    debugDiode.low_HiZ();
#endif
    if (consoleDebugOn)
      debugTimer.execute();

    consoleInput();
#ifdef DEBUG //[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
    debugDiode.high_PullUp();
    debugDiode.low_HiZ();
    debugDiode.high_PullUp();
    debugDiode.low_HiZ();
#endif
  }
}
