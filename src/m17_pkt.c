#include "main.h"

//not fully featured, but mainly for things that are handled by the bot (SMS, TLE)
void decode_packet_data (uint8_t * input, char * packet_string, int len)
{

  //Decode the completed packet
  uint8_t protocol = input[0];

  if      (protocol == 0x00) fprintf (stderr, " Raw;");
  else if (protocol == 0x01) fprintf (stderr, " AX.25;");
  else if (protocol == 0x02) fprintf (stderr, " APRS;");
  else if (protocol == 0x03) fprintf (stderr, " 6LoWPAN;");
  else if (protocol == 0x04) fprintf (stderr, " IPv4;");
  else if (protocol == 0x05) fprintf (stderr, " SMS;");
  else if (protocol == 0x06) fprintf (stderr, " Winlink;");
  else if (protocol == 0x07) fprintf (stderr, " TLE;");

  //simple UTF-8 SMS Decoder
  if (protocol == 0x05)
  {
    fprintf (stderr, " Text: ");
    for (int i = 1; i < len; i++)
      fprintf (stderr, "%c", input[i]);

    //string should be self-terminating
    strncpy(packet_string, (const char *)input+1, len);

    //but just in case it isn't
    packet_string[len] = '\0';
  }
  //TLE UTF-8 Text Decoder
  else if (protocol == 0x07)
  {
    //print first to console, preserving formatting
    fprintf (stderr, "\n");
    for (int i = 1; i < len; i++)
      fprintf (stderr, "%c", input[i]);

    //scan input, replace linebreak with forward slash
    for (int i = 1; i < len; i++)
    {
      if (input[i] == '\n')
        input[i] = '/';
    }

    //string should be self-terminating
    strncpy(packet_string, (const char *)input+1, len);

    //but just in case it isn't
    packet_string[len] = '\0';
  }
  //other formats, no decode, just dump
  else
  {
    fprintf (stderr, "\n");
    for (int i = 1; i < len; i++)
      fprintf (stderr, "%02X", input[i]);
  }

}
