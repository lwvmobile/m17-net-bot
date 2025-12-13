#include "main.h"

//Base40 Call Sign Data Character Set
static char b40[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-/.";

//NOTE: memset the global callsign string from calling function before sending here, or else it may splice stale callsign data
void convert_array_to_csd(uint8_t * input, char * callsign)
{
  uint64_t callsign_hex = convert_bytes_into_output(input, 6);
  if (callsign_hex == 0xFFFFFFFFFFFF)
    sprintf (callsign, "%s", "BROADCAST"); //might consider using #ALL 'cuz everybody else does
  else if (callsign_hex == 0)
    sprintf (callsign, "%s", "RESERVED ");
  else if (callsign_hex >= 0xEE6B28000000)
    sprintf (callsign, "R: %lX", callsign_hex);
  else
  {
    uint8_t k = 0;
    while (callsign_hex != 0) //if we change back to a for i loop to 9, then it won't splice, but leaves trailing spaces in csd
    {
      callsign[k++] = b40[callsign_hex % 40];
      callsign_hex /= 40;
    }
  }
}

void convert_csd_to_array(uint8_t * output, char * callsign)
{
  uint64_t callsign_hex = 0;

  //check for #ALL, if so, then its the broadcast address
  if (strncmp (callsign, "#ALL", 4) == 0)
    callsign_hex = 0xFFFFFFFFFFFF;
  //check for @ALL, if so, then its the broadcast address
  else if (strncmp (callsign, "@ALL", 4) == 0)
    callsign_hex = 0xFFFFFFFFFFFF;
  else if (strncmp (callsign, "BROADCAST", 9) == 0)
    callsign_hex = 0xFFFFFFFFFFFF;
  else
  {
    int i = 0; int j = 0;
    for(i = strlen((const char*)callsign)-1; i >= 0; i--)
    {
      for(j = 0; j < 40; j++)
      {
        if(callsign[i]==b40[j])
        {
          callsign_hex = callsign_hex * 40 + j;
          break;
        }
      }
    }
  }

  //convert to output byte array
  for (int i = 0; i < 6; i++)
    output[i] = ( callsign_hex >> (40-(i*8)) ) & 0xFF;

}

//break callsign data without spaces into individually spaced letters for TTS
int my_callsign_to_letters(char * csd, char * output)
{
  
  for (int i = 0; i < 10; i++)
  {
    output[(i*2)+0] = csd[i];
    output[(i*2)+1] = 0x20;
  }

  return 20;
}
