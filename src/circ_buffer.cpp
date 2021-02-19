#include "circ_buffer.h"

void *operator new[](size_t size)
{
  return malloc(size);
}

void operator delete[](void *ptr)
{
  free(ptr);
}

circBuffer::circBuffer(uint8_t buf_size, uint8_t TEST_startpoint)
{
  bufSize = buf_size;
  data = new char[buf_size];
  for (uint8_t i = 0; i < buf_size; ++i)
    data[i] = '\0';
  headIndex = TEST_startpoint;
  tailIndex = TEST_startpoint;
  isFullFlag = false;
}
void circBuffer::write(char cell)
{
  data[headIndex] = cell;
  if (isFullFlag)
  {
    //    tailIndex = (tailIndex + 1) % bufSize; // very time consuming
    ++tailIndex;
    if (tailIndex == bufSize)
      tailIndex = 0;
  }
  //  headIndex = (headIndex + 1) % bufSize; // very time consuming
  ++headIndex;
  if (headIndex == bufSize)
    headIndex = 0;
  isFullFlag = headIndex == tailIndex;
}
char circBuffer::read(void)
{
  if (isNotEmpty())
  {
    char cell = data[tailIndex];
    //  tailIndex = (tailIndex + 1) % bufSize; // very time consuming
    ++tailIndex;
    if (tailIndex == bufSize)
      tailIndex = 0;
    isFullFlag = false;
    return cell;
  }
  else
    return 0;
}
bool circBuffer::isEmpty()
{
  return !isFullFlag && (headIndex == tailIndex);
}
bool circBuffer::isNotEmpty()
{
  return isFullFlag || (headIndex != tailIndex);
}
bool circBuffer::isFull()
{
  return isFullFlag;
}
uint8_t circBuffer::getLength()
{
  if (headIndex >= tailIndex)
    return headIndex - tailIndex;
  else
    return bufSize - tailIndex + headIndex;
}

uint8_t circBuffer::getData(char *bufferData)
{
  uint8_t len =  getLength();
  uint8_t index = tailIndex;
  for (uint8_t i = 0; i < len; ++i)
  {
    bufferData[i] = data[index];
    ++index;
    if (index >= bufSize)
      index = 0;
  }
  return len;
}
