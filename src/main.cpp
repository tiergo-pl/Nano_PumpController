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
#include "pwm.h"
#include "i2c.h"
#include "Stepper.h"

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
}

void adc_init()
{
  ADMUX = 0x67; // Vref = Vcc,ADLAR=1 ,ADC7
  ADCSRA = 7;   //prescaler /128
  ADCSRA |= _BV(ADEN) | _BV(ADIE) | _BV(ADSC);
}

ISR(ADC_vect)
{
  pwm0_2 = ADCH / 2 + 127;
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
  pwm_init();
  // adc_init();
  // i2c_init();  // attention!! make i2c missing exception handling
  stepperInit();
  sei();

  interval1 = eeprom_read_dword(saved_interval1);
  interval2 = eeprom_read_dword(saved_interval2);
  interval3 = eeprom_read_dword(saved_interval3);
  intervalBuzzer = eeprom_read_dword(saved_intervalBuzzer);
  eeprom_read_block(sequence1, saved_sequence1, SEQUENCE1_SIZE);
  eeprom_read_block(sequence2, saved_sequence2, SEQUENCE2_SIZE);
  char uartInputString[64] = "\0";
  char cmdLine[64] = "\0";
  //  uint8_t pwm0SequenceIndex = 0;
  uint8_t pwm0_1SequenceIndex = 0;
  uint8_t pwm0_2SequenceIndex = 0;
  // PORTC |= _BV(debugPin2);
  // eeprom_read_block(&displaySeq[1], saved_displaySeq, DISPLAY_SEQ_SIZE); // attention!! make i2c missing exception handling
  // displayFill();  // attention!! make i2c missing exception handling

  while (1)
  {

    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
      mainClock_us_temp = mainClock_us;
    }

    if ((mainClock_us_temp - clk1) >= interval1)
    {
      clk1 = mainClock_us_temp;
      PORTB ^= 1 << Led_builtin;
      // PORTC ^= _BV(debugPin2);
      //      pwm0SequenceIndex = pwmSequence(& pwm0, sequence1, pwm0SequenceIndex, SEQUENCE1_SIZE);
      pwm0_1SequenceIndex = pwmSequence(&pwm0_1, sequence1, pwm0_1SequenceIndex, SEQUENCE1_SIZE);

/*      if (PORTB & _BV(Led_builtin))
        i2cSeq[1] = 0xa7;
      else
        i2cSeq[1] = 0xa6;
      i2cSeq[0] = 0;
      i2c_write(DISPLAY_ADDRESS, i2cSeq, 2);*/

    }
    if ((mainClock_us_temp - clk2) >= interval2)
    {
      clk2 = mainClock_us_temp;
      
      /* removed - stepper motor usage

      PORTC ^= _BV(debugPin0);
      pwm1 = rand();
      //      PORTC ^= (1 << debugPin0);
      /*
      if (sizeof(uartInputString) <= (strlen(uartInputString) + 1))
        uartInputString[0] = 0;
      uartReceiveString(uartInputString);
      //  uartTransmit(sprintf(uartOutputString, "It's UART test. You wrote %s. clk2= %lu, buffer= %s, buffer 1st char= %c\r\n", uartInputString, clk2, uartInputBuffer, uartInputBuffer[0]), uartOutputString);
      sprintf(uartOutputString, "clk2= %lu, UART debug. uartInputString: [%s] , cmdLine: [%s] \r\n", clk2, uartInputString, cmdLine);
      PORTC |= _BV(debugPin0);
      PORTC &= ~_BV(debugPin0);
      uartTransmitString(uartOutputString);
      */
      stepUp();
      
    }

    if ((mainClock_us_temp - clk3) >= interval3)
    {
      clk3 = mainClock_us_temp;

          /* removed - stepper motor usage

      PORTC ^= _BV(debugPin1);
      //      pwm0_2SequenceIndex = pwmSequence(&pwm0_2, sequence2, pwm0_2SequenceIndex, SEQUENCE2_SIZE);
      */
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

    //    uartReceive(uartInputString, uartInputBuffer);
    // uartReceive(uartBuffer.data, uartInputBuffer);

    //    _delay_us(5);
    //    PORTC |= _BV(debugPin0);
    //    PORTC &= ~_BV(debugPin0);
    //    PORTC |= _BV(debugPin2);

    //    PORTC &= ~_BV(debugPin2);
    //      for (uint8_t i = 0; i < 50; ++i) //DEBUG ONLY !!!!!
    //        asm("nop");
    //uartTransmitString((char *)"LED_builtin toggle\n");//DEBUG ONLY!!!
  }
}
