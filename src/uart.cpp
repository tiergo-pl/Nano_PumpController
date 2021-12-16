#include "uart.h"

circBuffer uartRXBuf(128, 0);
circBuffer uartTXBuf(240, 0);

ISR(USART_RX_vect)
{
  uartRXBuf.write(UDR0);
}

ISR(USART_UDRE_vect)
{
  UDR0 = uartTXBuf.read();
  if (uartTXBuf.isEmpty())
    UCSR0B &= ~(_BV(UDRIE0)); // disable interrupt from transmit buffer
}

void uart_init(void)
{

  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;

#if USE_2X
  UCSR0A |= _BV(U2X0);
#else
  UCSR0A &= ~(_BV(U2X0));
#endif

  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);             /* 8-bit data */
  UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0); /* Enable RX and TX and RX interrupt*/
}

void uartTransmitBinary(char *buffer, uint8_t lenght)
{
  for (uint8_t i = 0; i < lenght; ++i)
  {
    uartTXBuf.write(buffer[i]);
  }
  UCSR0B |= (_BV(UDRIE0)); // enable interrupt from transmit buffer
}

void uartTransmitString(const char *buffer)
{
  uint8_t i = 0;
  while (buffer[i] != 0)
  {
    uartTXBuf.write(buffer[i]);
    ++i;
  }
  UCSR0B |= (_BV(UDRIE0)); // enable interrupt from transmit buffer
}

uint8_t uartReceiveString(char *rxString)
{
  uint8_t rxCharN = 0;
  while (uartRXBuf.isNotEmpty())
  {
    char rxChar = uartRXBuf.read();
    strncat(rxString, &rxChar, 1);
    ++rxCharN;
  }
  return rxCharN;
}
uint8_t uartEcho()
{
  if (uartRXBuf.isNotEmpty())
  {
    uint8_t len = uartRXBuf.getLength();
    char buf[len];
    uartRXBuf.getData(buf);
    //    char buf[]="TEST STRING buffer asdfg";
    //    uint8_t len = 23;
    uartTransmitBinary(buf, len);
    return len;
  }
  else
    return 0;
}
