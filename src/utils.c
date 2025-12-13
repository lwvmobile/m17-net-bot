#include "main.h"

//input uint8_t bit array, return output as up to a 64-bit value
uint64_t convert_bits_into_output(uint8_t * input, int len)
{
  int i;
  uint64_t output = 0;
  for(i = 0; i < len; i++)
  {
    output <<= 1;
    output |= (uint64_t)(input[i] & 1);
  }
  return output;
}

//input uint8_t byte array, return output as up to a 64-bit value
uint64_t convert_bytes_into_output(uint8_t * input, int len)
{
  int i;
  uint64_t output = 0;
  for(i = 0; i < len; i++)
  {
    output <<= 8;
    output |= (uint64_t)input[i] & 0xFF;
  }
  return output;
}

//take x amount of bits and pack into len amount of bytes (symmetrical)
void pack_bit_array_into_byte_array (uint8_t * input, uint8_t * output, int len)
{
  int i;
  for (i = 0; i < len; i++)
    output[i] = (uint8_t)convert_bits_into_output(&input[i*8], 8);
}

//take len amount of bits and pack into x amount of bytes (asymmetrical)
void pack_bit_array_into_byte_array_asym (uint8_t * input, uint8_t * output, int len)
{
  int i = 0; int k = len % 8;
  for (i = 0; i < len; i++)
  {
    output[i/8] <<= 1;
    output[i/8] |= input[i];
  }
  //if any leftover bits that don't flush the last byte fully packed, shift them over left
  if (k)
    output[i/8] <<= 8-k;
}

//take len amount of bytes and unpack back into a bit array
void unpack_byte_array_into_bit_array (uint8_t * input, uint8_t * output, int len)
{
  int i = 0, k = 0;
  for (i = 0; i < len; i++)
  {
    output[k++] = (input[i] >> 7) & 1;
    output[k++] = (input[i] >> 6) & 1;
    output[k++] = (input[i] >> 5) & 1;
    output[k++] = (input[i] >> 4) & 1;
    output[k++] = (input[i] >> 3) & 1;
    output[k++] = (input[i] >> 2) & 1;
    output[k++] = (input[i] >> 1) & 1;
    output[k++] = (input[i] >> 0) & 1;
  }
}

//left shift an input array of len x bytes one to the left (MSB-wards)
void left_shift_byte_array (uint8_t * input, uint8_t * output, int len)
{
  int i;
  output[len-1] = input[0]; //swing MSB position to LSB (byte) position
  for (i = 1; i < len; i++) //left shift other bytes one towards MSB
    output[i-1] = input[i];
}

//convert a char string of hex into a uint8_t array, return len
uint16_t convert_hex_string_to_array (char * input, uint8_t * output)
{

  //since we want this as octets, get strlen value, then divide by two
  uint16_t len = strlen((const char*)input);
  
  //if the sting len is 0, return as 0
  if (len == 0)
    return 0;

  //if odd number, then user didn't pass complete octets, but just add one to len value to make it even
  if (len&1) len++;

  //divide by two to get octet len
  len /= 2;

  //sanity check, maximum strlen should not exceed 823 for a full encode of Packet SMS Data
  if (len > 823) len = 823;

  char octet_char[3];
  octet_char[2] = 0;
  uint16_t k = 0;
  uint16_t i = 0;

  //debug
  // fprintf (stderr, "\n Raw Len: %d; Raw Octets:", len);
  for (i = 0; i < len; i++)
  {
    strncpy (octet_char, input+k, 2);
    octet_char[2] = 0;
    sscanf (octet_char, "%hhX", &output[i]);

    //debug
    // fprintf (stderr, " (%s)", octet_char);
    // fprintf (stderr, " %02X", output[i]);

    k += 2;
  }
  // fprintf (stderr, "\n");

  return len;

}

//convert a text string into a uint8_t array, return len
uint16_t convert_utf8_text_to_array (char * input, uint8_t * output)
{
  uint16_t len = strlen((const char*)input);

  for (uint16_t i = 0; i < len; i++)
    output[i] = (uint8_t)input[i];

  return len;
}

