#include "main.h"

int ip_lsf_encoder(uint8_t * input, uint8_t is_stream)
{

  //DST Callsign
  convert_csd_to_array(input+0, this_dst_callsign);

  //SRC Callsign
  convert_csd_to_array(input+6, this_src_callsign);

  //META (empty)

  //Frame Type Configuration
  uint16_t frame_type = 0x0000;
  if (lsf_type_version == 3)
  {
    if (is_stream == 0)
      frame_type = 0xF000; //Data Payload Type
    else frame_type = 0x2000; //3200 voice stream
    frame_type |= ((this_can & 0xF) << 0);
  }
  else
  {
    if (is_stream == 1)
      frame_type = 0x0005; //v2 3200 voice stream
    frame_type |= ((this_can & 0xF) << 7);
  }

  //attach frame type
  input[12] = (frame_type >> 8) & 0xFF;
  input[13] = (frame_type >> 0) & 0xFF;

  //calculate and attach CRC16
  uint16_t crc = crc16(input, 28);
  input[28] = (crc >> 8) & 0xFF;
  input[29] = (crc >> 0) & 0xFF;

  //debug
  // fprintf (stderr, "\n R_LSF: ");
  // for (int i = 0; i < 30; i++)
  //   fprintf (stderr, "%02X", input[i]);

  return 30; //full size of an lsf frame w/ crc
}
