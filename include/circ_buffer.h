#ifndef _circ_buffer_h_
#define _circ_buffer_h_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


class circBuffer
{
private:
  uint8_t tailIndex; // lower end of usable data
  uint8_t headIndex; // higher end od usable data
  uint8_t bufSize;
  bool isFullFlag;

public:
  char *data;
  circBuffer(uint8_t buf_size, uint8_t TEST_startpoint);
  void write(char cell);
  char read();
  bool isEmpty();
  bool isNotEmpty();
  bool isFull();
  uint8_t getLength();
  uint8_t getData(char *bufferData);
};

#endif