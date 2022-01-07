#ifndef _uart_h_
#define _uart_h_
 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "circ_buffer.h"
//#include "debug.h"
#ifdef RELEASE_V1
#define BAUD 57600
#else
#define BAUD 250000
#endif
#include <util/setbaud.h>


void uartInit(void);
void uartTransmitBinary(char *buffer, uint8_t lenght);
void uartTransmitString(const char *buffer);
uint8_t uartReceiveString(char *rxString);
uint8_t uartEcho();

#endif