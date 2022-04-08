#include <display_TM1637.h>

#define DATA_COMMAND 0x40
#define DISPLAY_COMMAND 0x80
#define ADDRESS_COMMAND 0xC0

const uint8_t digitToSegment[] = {
    DIGIT_0,
    DIGIT_1,
    DIGIT_2,
    DIGIT_3,
    DIGIT_4,
    DIGIT_5,
    DIGIT_6,
    DIGIT_7,
    DIGIT_8,
    DIGIT_9,
    DIGIT_A,
    DIGIT_b,
    DIGIT_C,
    DIGIT_d,
    DIGIT_E,
    DIGIT_F};

DisplayTM1637::DisplayTM1637(volatile uint8_t *clkPort, uint8_t clkPinNo, volatile uint8_t *dioPort, uint8_t dioPinNo, uint8_t dispQuantity, uint16_t bitDelay)
{
  pClkPort = clkPort;
  mClkPinNo = clkPinNo;
  if (pClkPort == &PORTB)
  {
    pClkDdr = &DDRB;
    pClkPin = &PINB;
  }
  if (pClkPort == &PORTC)
  {
    pClkDdr = &DDRC;
    pClkPin = &PINC;
  }
  if (pClkPort == &PORTD)
  {
    pClkDdr = &DDRD;
    pClkPin = &PIND;
  }

  pDioPort = dioPort;
  mDioPinNo = dioPinNo;
  if (pDioPort == &PORTB)
  {
    pDioDdr = &DDRB;
    pDioPin = &PINB;
  }
  if (pDioPort == &PORTC)
  {
    pDioDdr = &DDRC;
    pDioPin = &PINC;
  }
  if (pDioPort == &PORTD)
  {
    pDioDdr = &DDRD;
    pDioPin = &PIND;
  }

  mQuantity = dispQuantity;
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

uint8_t DisplayTM1637::convertDigit(uint8_t digit)
{
  return digitToSegment[digit];
}

uint8_t *DisplayTM1637::toBcd(uint16_t number, uint8_t bcd[], uint8_t digitCount, bool leadingZeros, uint8_t base)
{
  for (int8_t i = digitCount - 1; i >= 0; i--)
  {
    uint8_t digit = number % base;
    bcd[i] = convertDigit(digit);
    number /= base;
  }
  return bcd;
}

void DisplayTM1637::prepareSegments(uint8_t segments[], uint8_t lenght, uint8_t position)
{
  for (uint8_t i = 0; i < lenght; i++)
  {
    mSegments[i+position] &= ~(0b01111111);              //clear only digit segments, leave dots
    mSegments[i+position] |= segments[i] & (0b01111111); //set only digit segments, leave dots
  }
  /*mLenght = lenght;
  mPosition = position;*/
  eToExecute = true;
}
void DisplayTM1637::prepareDots(uint8_t dots, uint8_t lenght, uint8_t position)
{
  uint8_t dot = dots;
  dot = dot << (8 - lenght);
  for (uint8_t i = position; i < lenght+position; i++)
  {
    mSegments[i] &= ~(0b10000000);      //clear only dot, leave digit
    mSegments[i] |= dot & (0b10000000); //set only dot, leave digit
    dot = dot << 1;
  }
  eToExecute = true;
}

void DisplayTM1637::setSegments()
{
  start();
  sendByte(DATA_COMMAND);
  stop();

  start();
  sendByte(ADDRESS_COMMAND);

  for (uint8_t i = 0; i < mQuantity; i++)
    sendByte(mSegments[i]);
  stop();

  start();
  sendByte(DISPLAY_COMMAND | (mBrightness & 0x0f));
  stop();
}
bool DisplayTM1637::execute()
{
  if (eToExecute)
  {
    setSegments();
    eToExecute = false;
    return true;
  }
  else
    return false;
}

void DisplayTM1637::setBrightness(uint8_t brightness)
{
  mBrightness = brightness;
}