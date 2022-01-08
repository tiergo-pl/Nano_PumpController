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
  // DDRC = (1 << debugPin0) | (1 << debugPin1) | (1 << debugPin2) | _BV(Buzzer);
  // DDRD = ((1 << BEEPER));
  beeper.outputLow();
  ledBuiltin.outputLow();
  aeration.outputLow();
  pump.outputLow();
  kbMenu.inputPullUp();
  kbUp.inputPullUp();
  kbDown.inputPullUp();
  debugDiode.outputHigh();
}

void adcInit()
{
  ADMUX = 0x67; // Vref = Vcc,ADLAR=1 ,ADC7
  ADCSRA = 7;   // prescaler /128
  ADCSRA |= _BV(ADEN) | _BV(ADIE) | _BV(ADSC);
}

ISR(ADC_vect)
{
  ADCSRA |= _BV(ADSC);
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
  ledBuiltin.toggle();
}

void refreshDisplayKeyboardRoutine()
{
  kbMenu.execute();
  kbUp.execute();
  kbDown.execute();
  if (mainMenu.getMenuLevel() == Menu::rootLevel)
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
}
void minuteTickDownwardsRoutine()
{
  minutesLeft--;
  if (minutesLeft < 0)
  {
    minutesLeft = 11; // CHANGE THIS TO 59 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
  /*
  interval1 = eeprom_read_dword(saved_interval1);
  interval2 = eeprom_read_dword(saved_interval2);
  interval3 = eeprom_read_dword(saved_interval3);
  intervalBuzzer = eeprom_read_dword(saved_intervalBuzzer);
  */
  /* not used - to delete in future
eeprom_read_block(sequence1, saved_sequence1, SEQUENCE1_SIZE);
  eeprom_read_block(sequence2, saved_sequence2, SEQUENCE2_SIZE);
  */

  beeper.setBeep(0, 800 * SYS_MILLISECONDS); // initial beep on system start

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
  Timer minuteTickDownwards(&clkSeconds16bit, 2); // CHANGE TO (&clkSeconds32bit, 60) !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
    beeper.beep();

    // reset output states after initial test
    if (!beeper.isOn() && clkSeconds32bit == 0)
    {
      display.prepareSegments(display.toBcd(clkSeconds32bit, dispContent));
      aeration.outputLow();
      pump.outputLow();
      display.prepareDots(0x0);
      if (sysClk > 9500)
      {
        mainProgramState.recoverFromPowerLoss();
        mainProgramState.update();
      }
    }
    else
    {
      refreshDisplayKeyboard.execute();
      mainProgramState.execute();
      mainMenu.execute();
    }

    oneSecondTick.execute(); // increment every second

    minuteTickDownwards.execute(mainProgramState.isRunning());
#ifdef DEBUG
    debugDiode.toggle();
#endif

    display.execute();
    halfSecondTick.execute();

    if (consoleDebugOn)
      debugTimer.execute();

    consoleInput();
  }
}
