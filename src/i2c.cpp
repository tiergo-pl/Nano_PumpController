#include "i2c.h"

//uint8_t i2cSeq[16] = {0, 0x20, 0x00, 0x8d, 0x14, 0xaf}; //6 display initialisation sequence
//uint8_t i2cSeq[16] = {0, 0x20, 0x00, 0x8d, 0x14, 0xd9, 0x11, 0xa4, 0xd5, 0x81, 0xa8, 0x1f, 0xda, 0x02, 0xaf}; // display initialisation sequence
//uint8_t i2cSeq[24] = {0, 0xAE, 0xD5, 0x80, 0xA8, 0x1F, 0xD3, 0x00, 0x1, 0xC8, 0xDA, 0x02, 0xd9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0x2E, 0xaf}; // display initialisation sequence
//uint8_t i2cSeq[32] = {0, 0xAE, 0xD5, 0x80, 0xA8, 0x1F, 0xD3, 0x00, 0x40, 0x8d, 0x14, 0x20, 0x00, 0x41, 0xC8, 0xDA, 0x02, 0xd9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0x2E, 0xaf}; //25 display initialisation sequence
uint8_t i2cSeq[32] = {0, 0xAE, 0xA8, 0x1F, 0xD3, 0x00, 0x40, 0x8d, 0x14, 0x20, 0x00, 0xA0, 0xC0, 0xDA, 0x02, 0xDB, 0x40, 0xA4, 0xA6, 0x2E, 0xaf}; //21 display initialisation sequence

void i2c_init()
{
  DDRC &= ~(_BV(PC4) | _BV(PC5));
  PORTC |= (_BV(PC4) | _BV(PC5));
  TWBR = I2C_BR;
  TWSR |= I2C_PS;
  i2c_write(DISPLAY_ADDRESS, i2cSeq, 21);
  i2cSeq[0] = 0;
  i2cSeq[1] = 0x21;
  i2cSeq[2] = 0;
  i2cSeq[3] = 0x7f;
  i2cSeq[4] = 0x22;
  i2cSeq[5] = 0;
  i2cSeq[6] = 0xff;
  i2c_write(DISPLAY_ADDRESS, i2cSeq, 7);
}
void i2c_error()
{
  uartTransmitString((char *)"I2C error!!!!!\r\n");
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

void displayFill()
{
  char uartOutputString[64];

  displaySeq[0] = 0x40;
  i2c_write(DISPLAY_ADDRESS, displaySeq, DISPLAY_SEQ_SIZE + 1);
  sprintf(uartOutputString, "Filled with %i bytes of data from EEPROM\r\n", (uint8_t)DISPLAY_SEQ_SIZE);
  uartTransmitString(uartOutputString);
}

uint16_t i2c_write(uint8_t i2cAddress, uint8_t *i2cData, uint16_t i2cDataCount)
{
  uint16_t i;
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)))
    ;
  if ((TWSR & 0xF8) != TW_START)
  {
    i2c_error();
    return 0;
  }
  TWDR = i2cAddress;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)))
    ;
  if ((TWSR & 0xF8) != TW_MT_SLA_ACK)
  {
    i2c_error();
    return 0;
  }
  for (i = 0; i < i2cDataCount; ++i)
  {
    TWDR = i2cData[i];
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))
      ;
    if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
    {
      i2c_error();
      return 0;
    }
  }
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
  while (TWCR & (1 << TWSTO))
    ;
  return i;
}

i2c::i2c(uint8_t address)
{
  this->address = address;
  DDRC &= ~(_BV(PC4) | _BV(PC5));
  PORTC |= (_BV(PC4) | _BV(PC5));
  TWBR = I2C_BR;
  TWSR = I2C_PRESCALER;
}

i2c::~i2c()
{
}

uint64_t i2c::write(uint8_t *data, uint16_t dataCount = 1)
{
  uint16_t i;
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)))
    ;
  if ((TWSR & 0xF8) != TW_START)
  {
    this->error();
    return 0;
  }
  TWDR = this->address;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)))
    ;
  if ((TWSR & 0xF8) != TW_MT_SLA_ACK)
  {
    this->error();
    return 0;
  }
  for (i = 0; i < dataCount; ++i)
  {
    TWDR = data[i];
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))
      ;
    if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
    {
      this->error();
      return 0;
    }
  }
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
  while (TWCR & (1 << TWSTO))
    ;
  return i;
}

void i2c::error()
{
  uartTransmitString((char *)"I2C error!!!!!\r\n");
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}
