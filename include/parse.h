#ifndef _parse_h_
#define _parse_h_

#include <string.h>
#include <avr/eeprom.h>

#include "globals.h"
#include "uart.h"

// commands in cli
#define CMD_SAVE          "save"
#define CMD_SHOW          "show"
#define CMD_HELP          "?"
#define CMD_interval1     "interval1"
#define CMD_interval2     "interval2"
#define CMD_interval3     "interval3"
#define CMD_freqBuzzer    "freq"
#define CMD_pwm0          "pwm0"
#define CMD_pwm1          "pwm1"
#define CMD_random        "rand"
#define CMD_contrast      "contrast"
#define CMD_displayfill   "df"
#define CMD_beepOn        "beepon"
#define CMD_beepOff       "beepoff"
#define CMD_DEBUG         "debug"

#define ENDL "\r\n" // end of line string
//---------------------

uint8_t detectEndlCmdline(char *cmdline, char *inputString);
void parseCmdline(char *cmdLine);

#endif // !_parse_h_
