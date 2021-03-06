#ifndef _PINS_H_
#define _PINS_H_
#pragma once
#include <avr/io.h>

class Pin
{
public:
  /*Initialize PIN.
   *Choose PORTx and PINxy
   *@param port Port name, e.g. PORTD
   *@param pinNo Pin number of port, e.g. PD2 or just 2 */
  Pin(volatile uint8_t *port, uint8_t pinNo);
  void inputHiZ();
  void inputPullUp();
  void low_HiZ();     // switch output to Low or input to Hi-Z
  void high_PullUp(); // switch output to High or input to Pull-Up
  void outputLow();
  void outputHigh();
  void toggle(); // toggle PORT register - toggle output level or input pull-up resistor
  bool readInput();
  bool readOutput();

  uint8_t getPinNo();
  volatile uint8_t *getPort();
  volatile uint8_t *getDdr();
  volatile uint8_t *getPin();

protected:
  uint8_t mPinNo;
  volatile uint8_t *pPort;
  volatile uint8_t *pDdr;
  volatile uint8_t *pPin;
};

class Beeper : public Pin
{
private:
  bool isActivated;
  uint8_t repeat;
  uint16_t start;
  uint16_t end;
  uint16_t pause;

public:
  using Pin::Pin;
  void setOn();
  void setOff();
  void setBeep(uint16_t startTime, uint16_t duration, uint8_t repeatCount = 1, uint16_t pauseDuration = 0); // defaults: repeatCount = 1, pauseDuration = duration
  void beep();
  void beepOnce();
  void beepTwice();
  uint16_t getStart();
  uint16_t getEnd();
  bool isOn();
};

class Key : public Pin
{
public:
  using Pin::Pin;
  /**
   * @brief Feed Key:: object with functions.
   * @param *[...]Func short press, long press, long pressing, very long press
   */
  void registerCallback(void (*shortPressFunc)(), void (*longPressFunc)(), void (*longPressingFunc)(), void (*veryLongPressFunc)());
  bool execute();

private:
  uint8_t keyState = 0;
  uint8_t pressTime = 0;
  void (*pShortPressCallback)();
  void (*pLongPressCallback)();
  void (*pLongPressingCallback)();
  void (*pVeryLongPressCallback)();
  bool pressed();
};

#endif // _PINS_H_
