#include <display_TM1637.h>

#define DATA_COMMAND 0x40
#define DISPLAY_COMMAND 0x80
#define ADDRESS_COMMAND 0xC0

DisplayTM1637::DisplayTM1637(volatile uint8_t *clkPort, uint8_t clkPinNo, volatile uint8_t *dioPort, uint8_t dioPinNo, uint16_t bitDelay)
{
  pClkPort = clkPort;
  mClkPinNo = clkPinNo;
  if (*pClkPort == PORTB)
  {
    pClkDdr = &DDRB;
    pClkPin = &PINB;
  }
  if (*pClkPort == PORTC)
  {
    pClkDdr = &DDRC;
    pClkPin = &PINC;
  }
  if (*pClkPort == PORTD)
  {
    pClkDdr = &DDRD;
    pClkPin = &PIND;
  }

  pDioPort = dioPort;
  mDioPinNo = dioPinNo;
  if (*pDioPort == PORTB)
  {
    pDioDdr = &DDRB;
    pDioPin = &PINB;
  }
  if (*pDioPort == PORTC)
  {
    pDioDdr = &DDRC;
    pDioPin = &PINC;
  }
  if (*pDioPort == PORTD)
  {
    pDioDdr = &DDRD;
    pDioPin = &PIND;
  }

  mBitDelay = bitDelay;
  mBrightness = 0x0f; //full brightness
}

inline void DisplayTM1637::delay()
{
  _delay_us(DEFAULT_BIT_DELAY); //normal delay - to upgrade later into non-pausing delay
}
void DisplayTM1637::clkInput()
{
  *pClkDdr &= ~_BV(mClkPinNo); //input
  *pClkPort |= _BV(mClkPinNo); //switch to Pull-Up state of input
}
void DisplayTM1637::clkOutput()
{
  *pClkPort &= ~_BV(mClkPinNo); //set low state of output or switch to Hi-Z state of input
  *pClkDdr |= _BV(mClkPinNo);   //output
}
void DisplayTM1637::dioInput()
{
  *pDioDdr &= ~_BV(mDioPinNo); //input
  *pDioPort |= _BV(mDioPinNo); //switch to Pull-Up state of input
}
void DisplayTM1637::dioOutput()
{
  *pDioPort &= ~_BV(mDioPinNo); //set low state of output or switch to Hi-Z state of input
  *pDioDdr |= _BV(mDioPinNo);   //output
}

void DisplayTM1637::start()
{
  dioOutput();
  delay();
}

void DisplayTM1637::stop()
{
  dioOutput();
  delay();
  clkInput();
  delay();
  dioInput();
  delay();
}

bool DisplayTM1637::sendByte(uint8_t dataByte)
{
  uint8_t data = dataByte;
  //serial transmit of 8 bits LSB-MSB
  for (uint8_t i = 0; i < 8; i++)
  {
    clkOutput(); //CLK falling edge
    delay();

    if (data & 1) //transmit 1 bit
      dioInput();
    else
      dioOutput();
    delay();
    clkInput(); //CLK rising edge
    delay();
    data = data >> 1; //next bit
  }
  // wait for ACK
  clkOutput();
  dioInput();
  delay();
  clkInput();
  bool ack = *pDioPin & _BV(mDioPinNo);
  if (ack == 0)
    dioOutput();
  delay();
  clkOutput();
  delay();
  return ack;
}
void DisplayTM1637::setSegments(uint8_t segments[], uint8_t lenght, uint8_t position)
{
  start();
  sendByte(DATA_COMMAND);
  stop();

  start();
  sendByte(ADDRESS_COMMAND | (position & 0x07));

  for (uint8_t i = 0; i < lenght; i++)
    sendByte(segments[i]);
  stop();

  start();
  sendByte(DISPLAY_COMMAND | (mBrightness & 0x0f));
  stop();
}