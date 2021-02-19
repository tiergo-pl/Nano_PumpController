#ifndef _pwm_h_
#define _pwm_h_ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "globals.h"
#include "debug.h"

#define PWM0 PORTD6
#define PWM1 PORTD5
#define pwm0 OCR0A
#define pwm1 OCR0B
#define PWM0_1 PORTD7
#define PWM0_2 PORTD4

volatile extern uint8_t pwm0_1;
volatile extern uint8_t pwm0_2;
volatile extern uint8_t pwm0_3;
volatile extern uint8_t pwm0_4;

volatile extern uint8_t pwm0_PIN;
volatile extern uint8_t pwm0_PIN_previous;

void pwm_init();
uint8_t pwmSequence(volatile uint8_t *pwmPin, uint8_t *pwmVal, uint8_t index, volatile uint8_t seqSize);




#endif // !_pwm_h_

