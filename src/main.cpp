// #include <Arduino.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "uart.h"
#include "debug.h"
#include "parse.h"
#include "display_TM1637.h"
#include "timer.h"
#include "globals.h"
//#include "pins.h"

StateMachine mainStateMachine;

void timer_init(void)
{
  OCR2A = 2 * MAIN_CLOCK_TICK - 1;                    // reset Timer2 at 2 * MAIN_CLOCK_TICK
  TCCR2A = 0b00000010;                                //((1<<WGM21) | (0<<WGM20));        // Timer2 CTC mode
  TCCR2B = ((0 << CS22) | (1 << CS21) | (0 << CS20)); // clk/8
  TIMSK2 = (1 << OCIE2A);                             // Timer/Counter Interrupt Mask
}

void port_init(void)
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

void adc_init()
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
  mainClock_us++;
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
  if (ledBuiltin.readOutput())
    display.prepareDots(0, 1, 1);
  else
    display.prepareDots(0x01, 1, 1);
  ledBuiltin.toggle();
}

void refreshDisplayKeyboardRoutine()
{
  if (!kbUp.readInput())
  {
    mainStateMachine.previousState();
    debugDiode.high_PullUp();
    beeper.beepOnce();
  }
  if (!kbDown.readInput())
  {
    mainStateMachine.nextState();
    debugDiode.low_HiZ();
    beeper.beepOnce();
  }
  if (!kbMenu.readInput())
  {
    mainStateMachine.toggle();
    debugDiode.toggle();
    beeper.beepTwice();
  }

  // debugDiode.toggle();
  display.prepareSegments(display.toBcd(minutesLeft, dispContent, 2), 2, 2);
  display.prepareSegments(display.toBcd(hoursLeft, dispContent, 2), 2, 0);
}
void minuteTickDownwardsRoutine()
{
  minutesLeft--;
  if (minutesLeft < 0)
  {
    minutesLeft = 11; //CHANGE THIS TO 59 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    hoursLeft--;
    if (hoursLeft < 0)
      mainStateMachine.transit();
  }
}

// main starts here
// main starts here
int main()
{
  timer_init();
  port_init();
  uart_init();
  sei();

  interval1 = eeprom_read_dword(saved_interval1);
  interval2 = eeprom_read_dword(saved_interval2);
  interval3 = eeprom_read_dword(saved_interval3);
  intervalBuzzer = eeprom_read_dword(saved_intervalBuzzer);
  /* not used - to delete in future
eeprom_read_block(sequence1, saved_sequence1, SEQUENCE1_SIZE);
  eeprom_read_block(sequence2, saved_sequence2, SEQUENCE2_SIZE);
  */
  char uartInputString[64] = "\0";
  char cmdLine[64] = "\0";
  char debugString[120];

  beeper.setBeep(0, 500000); // initial beep on system start

  // Initial display test
  display.prepareSegments(dispContent);
  display.prepareDots(0x0f);
  display.execute();

  // Prepare one second timer
  Timer oneSecondTick(&mainClock_us_temp, 1000000 / MAIN_CLOCK_TICK);
  oneSecondTick.registerCallback(
      []()
      { mainClock_seconds++; }); // Lambda function wrapper - gives pointer to function

  // Prepare half second timer
  Timer halfSecondTick(&mainClock_us_temp, 500000 / MAIN_CLOCK_TICK);
  halfSecondTick.registerCallback(halfSecondRoutine);

  Timer refreshDisplayKeyboard(&mainClock_us_temp, 200000 / MAIN_CLOCK_TICK);
  refreshDisplayKeyboard.registerCallback(refreshDisplayKeyboardRoutine);

  // MINUTES AND HOURS
  Timer minuteTickDownwards(&mainClock_us_temp, 20000); // CHANGE TO (&mainClock_seconds, 60) !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  minuteTickDownwards.registerCallback(minuteTickDownwardsRoutine);

  // MAIN PROGRAM LOOP
  // MAIN PROGRAM LOOP
  // MAIN PROGRAM LOOP
  while (1)
  {
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
      mainClock_us_temp = mainClock_us;
    }
    beeper.beep();

    // reset output states after initial test
    if (!beeper.isOn() && mainClock_seconds == 0)
    {
      display.prepareSegments(display.toBcd(mainClock_seconds, dispContent));
      aeration.outputLow();
      pump.outputLow();
      display.prepareDots(0x0);
      if (mainClock_us_temp > 95000)
        mainStateMachine.start();
    }
    else
      refreshDisplayKeyboard.execute();

    oneSecondTick.execute(); // increment every second


    minuteTickDownwards.execute(mainStateMachine.isRunning());
    mainStateMachine.execute();
    display.execute();
    halfSecondTick.execute();
    if ((mainClock_us_temp - clk1) >= interval1)
    {
      clk1 = mainClock_us_temp;
      // PORTB ^= 1 << LED_BUILTIN;
      //  PORTC ^= _BV(debugPin2);
      /*
      if (ledBuiltin.readOutput())
        display.prepareDots(0, 1, 1);
      else
        display.prepareDots(0x01, 1, 1);
      ledBuiltin.toggle();
      // PINB |= _BV(PB5);
      */
    }
    if ((mainClock_us_temp - clk2) >= interval2)
    {
      clk2 = mainClock_us_temp;

    }

    if ((mainClock_us_temp - clk3) >= interval3)
    {
      clk3 = mainClock_us_temp;
      if (consoleDebugOn)
      {
        sprintf(debugString, "uptime: %lus, PORT_addr %p PORT=%x, DDR_addr %p DDR=%x, PIN_addr %p PIN=%x\n",
                mainClock_seconds, pump.getPort(), *pump.getPort(), pump.getDdr(), *pump.getDdr(), pump.getPin(), *pump.getPin());
        uartTransmitString(debugString);
        sprintf(debugString, "uptime: %lus, PORT_addr %p PORT=%x, DDR_addr %p DDR=%x, PIN_addr %p PIN=%x\n",
                mainClock_seconds, kbMenu.getPort(), *kbMenu.getPort(), kbMenu.getDdr(), *kbMenu.getDdr(), kbMenu.getPin(), *kbMenu.getPin());
        uartTransmitString(debugString);
      }
    }
    if ((mainClock_us_temp - clkBuzzer) >= intervalBuzzer)
    {
      clkBuzzer = mainClock_us_temp;
      // PORTC ^= _BV(Buzzer);
    }

    if (uartEcho())
    {
      if (sizeof(uartInputString) < (strlen(uartInputString) + 2))
        uartInputString[0] = 0;
      uartReceiveString(uartInputString);
      if (detectEndlCmdline(cmdLine, uartInputString))
      {
        parseCmdline(cmdLine);
        uartTransmitString((char *)"\r\n");
      }
    }
  }
}
