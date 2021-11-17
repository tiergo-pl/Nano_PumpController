// #include <Arduino.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "globals.h"
#include "uart.h"
#include "debug.h"
#include "parse.h"
//#include "outputs.h"

void timer_init(void)
{
  OCR2A = 2 * MAIN_CLOCK_TICK - 1;                    // reset Timer2 at 2 * MAIN_CLOCK_TICK
  TCCR2A = 0b00000010;                                //((1<<WGM21) | (0<<WGM20));        // Timer2 CTC mode
  TCCR2B = ((0 << CS22) | (1 << CS21) | (0 << CS20)); // clk/8
  TIMSK2 = (1 << OCIE2A);                             // Timer/Counter Interrupt Mask
}

void port_init(void)
{
  DDRB = (1 << Led_builtin) | _BV(debugPin3); // Led builtin (13)
  //DDRC = (1 << debugPin0) | (1 << debugPin1) | (1 << debugPin2) | _BV(Buzzer);
  DDRD = (1 << debugPin0) | (1 << debugPin1 | (1 << BEEPER));
}

void adc_init()
{
  ADMUX = 0x67; // Vref = Vcc,ADLAR=1 ,ADC7
  ADCSRA = 7;   //prescaler /128
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
  eeprom_read_block(sequence1, saved_sequence1, SEQUENCE1_SIZE);
  eeprom_read_block(sequence2, saved_sequence2, SEQUENCE2_SIZE);
  char uartInputString[64] = "\0";
  char cmdLine[64] = "\0";

  // PORTC |= _BV(debugPin2);

  while (1)
  {

    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
      mainClock_us_temp = mainClock_us;
    }
    if ((mainClock_us_temp - tickAtLastSec) >= 1000000 / MAIN_CLOCK_TICK)
    {
      tickAtLastSec = mainClock_us_temp;
      mainClock_seconds++;
      beeper.setBeep(mainClock_us_temp, 20000);
    }

    beeper.beep();

    if ((mainClock_us_temp - clk1) >= interval1)
    {
      clk1 = mainClock_us_temp;
      PORTB ^= 1 << Led_builtin;
      // PORTC ^= _BV(debugPin2);
    }
    if ((mainClock_us_temp - clk2) >= interval2)
    {
      clk2 = mainClock_us_temp;
      PORTD ^= 1 << debugPin0;
    }

    if ((mainClock_us_temp - clk3) >= interval3)
    {
      clk3 = mainClock_us_temp;
      PORTD ^= 1 << debugPin1;
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
      }
    }
  }
}
