#ifndef _display_TM1637_h_
#define _display_TM1637_h_

#include <avr/io.h>
#include <util/delay.h>

#define DEFAULT_BIT_DELAY 100

#define DIGIT_0 0b00111111
#define DIGIT_1 0b00000110
#define DIGIT_2 0b01011011
#define DIGIT_3 0b01001111
#define DIGIT_4 0b01100110
#define DIGIT_5 0b01101101
#define DIGIT_6 0b01111101
#define DIGIT_7 0b00000111
#define DIGIT_8 0b01111111
#define DIGIT_9 0b01101111
#define DIGIT_A 0b01110111
#define DIGIT_b 0b01111100
#define DIGIT_C 0b00111001
#define DIGIT_d 0b01011110
#define DIGIT_E 0b01111001
#define DIGIT_F 0b01110001

//On initialization set (CLK Port, CLK Pin, DIO Port, DIO Pin, bit delay in us (default 100))
class DisplayTM1637
{
public:
  /* initialize object
set CLK & DIO pins */
  DisplayTM1637(volatile uint8_t *clkPort, uint8_t clkPinNo, volatile uint8_t *dioPort, uint8_t dioPinNo, uint8_t dispQuantity = 4, uint16_t bitDelay = DEFAULT_BIT_DELAY);
  //convert digit to 7seg
  uint8_t convertDigit(uint8_t digit);
  //convert number to decimal BCD
  uint8_t *toBcd(uint16_t number, uint8_t bcd[], uint8_t base = 10, bool leadingZeros = false, uint8_t digitCount = 4);
  //Prepare display segments to send
  void prepareSegments(uint8_t segments[], uint8_t lenght = 4, uint8_t position = 0);
  /*Prepare display dots to send.
  @param dots lower bits as dots, eg 0b00000100 - third from right dot
  @param lenght amount of dots changed
  @param position start point to change dots from left */
  void prepareDots(uint8_t dots, uint8_t lenght = 4, uint8_t position = 0);
  /*display segments immediately*/
  void setSegments();
  // execute display routine on event
  bool execute();

  //only for testing
  int classTest();

protected:
  volatile uint8_t *pClkPort;
  volatile uint8_t *pDioPort;
  volatile uint8_t *pClkDdr;
  volatile uint8_t *pDioDdr;
  volatile uint8_t *pClkPin;
  volatile uint8_t *pDioPin;
  uint8_t mClkPinNo;
  uint8_t mDioPinNo;

  uint8_t mQuantity;
  uint8_t mSegments[6];
  uint16_t mBitDelay;
  uint8_t mBrightness;
  bool eToExecute;

  void delay();
  void clkInput();  //  Hi-state by pullup resistor
  void clkOutput(); // Low state by low level of output
  void dioInput();  //  Hi-state by pullup resistor
  void dioOutput(); // Low state by low level of output
  void start();     //pseudo I2C start sequence
  void stop();      //pseudo I2C stop sequence
  bool sendByte(uint8_t dataByte);
};

//for testing only
//for testing only
//for testing only
//for testing only
//for testing only
//for testing only
//for testing only
//for testing only

class Klasa : public DisplayTM1637
{
public:
  using DisplayTM1637::DisplayTM1637;
  uint8_t getDioPinNo();
  uint8_t childClassTest();
};

class BaseClass
{
protected:
  int member;

public:
  int getBaseMember();
};
class ChildClass : public BaseClass
{
public:
  int childMember;
  int getMember();
  int getChildMember();
};

#endif // !_display_TM1637_h_