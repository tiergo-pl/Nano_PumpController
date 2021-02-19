#include "pwm.h"

volatile uint8_t pwm0_1;
volatile uint8_t pwm0_2;
volatile uint8_t pwm0_3;
volatile uint8_t pwm0_4;

volatile uint8_t pwm0_PIN;
volatile uint8_t pwm0_PIN_previous;
volatile uint8_t pwm0SeqNo;

ISR(TIMER0_OVF_vect)
{
  switch (pwm0SeqNo)
  {
  case 1:
    pwm0 = pwm0_1;
    pwm0_PIN_previous = pwm0_PIN; //unfortunatelly TIMER0_OVF interrupt req. happens in the middle of pwm pulse, so additional variable is required
    pwm0_PIN = _BV(PWM0_1); // as above, here are copied pin number of NEXT pwm pulse
    pwm0SeqNo = 255;
    break;

  case 255: // blank pwm pause
    pwm0 = 0;
    pwm0_PIN_previous = pwm0_PIN;
    pwm0_PIN = 0;
    pwm0SeqNo = 2;
    break;

  case 2:
    pwm0 = pwm0_2;
    pwm0_PIN_previous = pwm0_PIN;
    pwm0_PIN = _BV(PWM0_2);
    pwm0SeqNo = 0;
    break;
  
  case 0: // blank pwm pause
    pwm0 = 0;
    pwm0_PIN_previous = pwm0_PIN;
    pwm0_PIN = 0;
    pwm0SeqNo = 1;
    break;

  default:
    pwm0_PIN_previous = 0;
    pwm0_PIN = 0;
    pwm0SeqNo = 0;
    break;
  }
}

ISR(TIMER0_COMPA_vect)
{
  //  (PIND & _BV(PWM0)) ? (PORTD |= _BV(PWM0_1)) : (PORTD &= ~_BV(PWM0_1));
  (PIND & _BV(PWM0)) ? (PORTD |= pwm0_PIN) : (PORTD &= ~pwm0_PIN_previous);
}

void pwm_init()
{
  DDRD = (1 << PWM0) | (1 << PWM1) | _BV(PWM0_1) | _BV(PWM0_2);
  //  Timer0 PWM, phase correct , clk/64:
  TCCR0B = ((0 << WGM02) | (0 << CS02) | (1 << CS01) | (1 << CS00));
  // OC0A, OC0B non inverting; Timer0 PWM, phase correct:
  TCCR0A = ((1 << COM0A1) | (0 << COM0A0) | (1 << COM0B1) | (0 << COM0B0) | (0 << WGM01) | (1 << WGM00));
  TIMSK0 = _BV(OCIE0A) | _BV(TOV0);
  pwm0 = 100; //
  pwm1 = 200; //
}
uint8_t pwmSequence(volatile uint8_t *pwmPin, uint8_t *pwmVal, uint8_t index, volatile const uint8_t seqSize)
{
  *pwmPin = pwmVal[index];
  ++index;
  if (index >= seqSize)
    index = 0;
  if (pwmVal[index] == 0)
    index = 0;
  return index;
}