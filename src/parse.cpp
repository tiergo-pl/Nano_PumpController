#include "parse.h"
Beeper beeper;
uint8_t detectEndlCmdline(char *cmdline, char *inputString)
{
  uint8_t i = 0;
  while (i < strlen(inputString))
  {
    if (inputString[i] == '\n' || inputString[i] == '\r')
    {
      cmdline[0] = 0;
      strncpy(cmdline, inputString, i);
      cmdline[i] = 0;
      inputString[0] = 0;
      return i;
    }
    else
      ++i;
  }
  return 0;
}

void parseCmdline(char *cmdLine)
{
  char uartOutputString[128];
  char *s_value;
  uint32_t value;
  char *end_str;
  char *token = strtok_r(cmdLine, " ", &end_str);

  while (token != NULL)
  {
    char *end_token;
    char *cmd = strtok_r(token, "=-:", &end_token);
    s_value = strtok_r(NULL, "=-:", &end_token);
    value = atol(s_value);

    if (!strcmp(cmd, CMD_SAVE))
    {
      eeprom_update_dword(saved_interval1, interval1);
      eeprom_update_dword(saved_interval2, interval2);
      eeprom_update_dword(saved_interval3, interval3);
      eeprom_update_dword(saved_intervalBuzzer, intervalBuzzer);
      uartTransmitString((char *)"...Saved\r\n");
    }
    else if (!strcmp(cmd, CMD_SHOW))
    {
      /*
      sprintf(uartOutputString, "mainClk= %lu, cmdLine: [%s], interval1= %lu, interval2= %lu, interval3= %lu, intervalBuzzer= %lu \r\n",
              mainClock_us_temp, cmd, interval1 * MAIN_CLOCK_TICK, interval2 * MAIN_CLOCK_TICK, interval3 * MAIN_CLOCK_TICK, intervalBuzzer);
      uartTransmitString(uartOutputString);
      */
      sprintf(uartOutputString, "mainClk= %lu, uptime: %lu s, interval1= %lu, interval2= %lu, interval3= %lu, intervalBuzzer= %lu \r\n",
              mainClock_us_temp, mainClock_seconds, interval1 * MAIN_CLOCK_TICK, interval2 * MAIN_CLOCK_TICK, interval3 * MAIN_CLOCK_TICK, intervalBuzzer);
      uartTransmitString(uartOutputString);
    }
    else if (!strcmp(cmd, CMD_HELP))
    {
      uartTransmitString((char *)"Usage: [COMMAND] ... [VARIABLE=VALUE] ... \r\nexample: interval2=100000 freq=1000 save show ?\r\n");
    }
    else if (!strcmp(cmd, CMD_interval1))
    {
      if (value)
      {
        interval1 = value / MAIN_CLOCK_TICK;
      }
      else
        uartTransmitString((char *)"Input valid number of microseconds. Nothing changed yet...\r\n");
    }
    else if (!strcmp(cmd, CMD_interval2))
    {
      if (value)
      {
        interval2 = value / MAIN_CLOCK_TICK;
      }
      else
        uartTransmitString((char *)"Input valid number of microseconds. Nothing changed yet...\r\n");
    }
    else if (!strcmp(cmd, CMD_interval3))
    {
      if (value)
      {
        interval3 = value / MAIN_CLOCK_TICK;
      }
      else
        uartTransmitString((char *)"Input valid number of microseconds. Nothing changed yet...\r\n");
    }
    else if (!strcmp(cmd, CMD_freqBuzzer))
    {
      if (value)
      {
        intervalBuzzer = 50000 / value;
      }
      else
        uartTransmitString((char *)"Input valid frequency in Hertz. Nothing changed yet...\r\n");
    }
    else if (!strcmp(cmd, CMD_pwm0))
    {
    }
    else if (!strcmp(cmd, CMD_pwm1))
    {
    }
    else if (!strcmp(cmd, CMD_random))
    {
      int val_rand = rand();
      if (value)
      {
        val_rand %= value;
      }
      sprintf(uartOutputString, "Randomly drawn number is: %i \r\n", val_rand);
      uartTransmitString(uartOutputString);
    }
    else if (!strcmp(cmd, CMD_contrast))
    {

      sprintf(uartOutputString, "Display contrast set to: %i \r\n", (uint8_t)value);
      uartTransmitString(uartOutputString);
    }
    else if (!strcmp(cmd, CMD_displayfill))
    {
    }
    else if (!strcmp(cmd, CMD_beepOn))
    {
      beeper.setOn();
    }
    else if (!strcmp(cmd, CMD_beepOff))
    {
      beeper.setOff();
    }

    else
      uartTransmitString((char *)"Nothing entered\r\n");

    token = strtok_r(NULL, " ", &end_str);
  }
}