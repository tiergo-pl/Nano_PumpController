#ifndef _i2c_h_
#define _i2c_h_

#include <util/twi.h>
#include "uart.h"
#include "globals.h"

//these macros to delete:
#define I2C_BR 3 // TWBR = 3 
#define I2C_PS 3 // prescaler /64
#define DISPLAY_ADDRESS 0b01111000

#define I2C_PRESCALER 3 // prescaler /64
#define I2C_CLOCK


extern uint8_t i2cSeq[];

void i2c_init();
void i2c_error();
void displayFill();
void i2c_start();
void i2c_stop();
uint16_t i2c_write(uint8_t i2cAddress, uint8_t *i2cData, uint16_t i2cDataCount);

class i2c
{
private:
  /* data */
  uint8_t address;
  void error();

public:
  i2c(uint8_t address);
  ~i2c();
  uint64_t write(uint8_t *data, uint16_t dataCount);

};


#endif // !_i2c_h_

