#ifndef _display_TM1637_h_
#define _display_TM1637_h_

#include <avr/io.h>
#include <util/delay.h>

#define DEFAULT_BIT_DELAY 50

//On initialization set (CLK Port, CLK Pin, DIO Port, DIO Pin, bit delay in us (default 100))
class DisplayTM1637
{
private:
  volatile uint8_t *pClkPort;
  volatile uint8_t *pDioPort;
  volatile uint8_t *pClkDdr;
  volatile uint8_t *pDioDdr;
  volatile uint8_t *pClkPin;
  volatile uint8_t *pDioPin;
  uint8_t mClkPinNo;
  uint8_t mDioPinNo;

  uint16_t mBitDelay;
  uint8_t mBrightness;

  void delay();
  void clkInput();  //  Hi-state by pullup resistor
  void clkOutput(); // Low state by low level of output
  void dioInput();  //  Hi-state by pullup resistor
  void dioOutput(); // Low state by low level of output
  void start(); //pseudo I2C start sequence
  void stop();  //pseudo I2C stop sequence
  bool sendByte(uint8_t dataByte);

public:
/* initialize object
set CLK & DIO pins */
  DisplayTM1637(volatile uint8_t *clkPort, uint8_t clkPinNo, volatile uint8_t *dioPort, uint8_t dioPinNo, uint16_t bitDelay = DEFAULT_BIT_DELAY);
  /*display segments
  */
  void setSegments(uint8_t segments[], uint8_t lenght = 4, uint8_t position = 0);
};


#endif // !_display_TM1637_h_