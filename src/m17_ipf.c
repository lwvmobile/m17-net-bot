#include "main.h"

//detached ip frame lstn, conn, disc, ping, and pong replies
void ip_send_conn_disc_ping_pong (int sockfd, uint8_t * source, uint8_t cd)
{

  uint8_t lstn[11]; memset (lstn, 0, sizeof(lstn));
  uint8_t conn[11]; memset (conn, 0, sizeof(conn));
  uint8_t disc[10]; memset (disc, 0, sizeof(disc));
  uint8_t ping[10]; memset (ping, 0, sizeof(ping));
  uint8_t pong[10]; memset (pong, 0, sizeof(pong));

  lstn[0] = 0x4C; lstn[1] = 0x53; lstn[2] = 0x54; lstn[3] = 0x4E; lstn[10] = reflector_module;
  conn[0] = 0x43; conn[1] = 0x4F; conn[2] = 0x4E; conn[3] = 0x4E; conn[10] = reflector_module;
  disc[0] = 0x44; disc[1] = 0x49; disc[2] = 0x53; disc[3] = 0x43;
  ping[0] = 0x50; ping[1] = 0x49; ping[2] = 0x4E; ping[3] = 0x47;
  pong[0] = 0x50; pong[1] = 0x4F; pong[2] = 0x4E; pong[3] = 0x47;

  for (uint8_t i = 0; i < 6; i++)
  {
    conn[i+4] = source[i];
    lstn[i+4] = source[i];
    disc[i+4] = source[i];
    ping[i+4] = source[i];
    pong[i+4] = source[i];
  }

  //0 for disc, 1 for conn, 2 for ping, 3 for pong, 4 for lstn
  if (cd == 0)
    udp_socket_blaster (sockfd, 10, disc);
  else if (cd == 1)
    udp_socket_blaster (sockfd, 11, conn);
  else if (cd == 2)
    udp_socket_blaster (sockfd, 10, ping);
  else if (cd == 3)
    udp_socket_blaster (sockfd, 10, pong);
  else if (cd == 4)
    udp_socket_blaster (sockfd, 11, lstn);

}

static uint8_t m17s[4] = {0x4D, 0x31, 0x37, 0x20};
static uint8_t m17p[4] = {0x4D, 0x31, 0x37, 0x50};

//this is a partial ip frame decoder
void ip_frame_decoder (uint8_t * input, int len)
{

  // uint8_t m17s[4] = {0x4D, 0x31, 0x37, 0x20};
  uint8_t ackn[4] = {0x41, 0x43, 0x4B, 0x4E};
  uint8_t nack[4] = {0x4E, 0x41, 0x43, 0x4B};
  uint8_t disc[4] = {0x44, 0x49, 0x53, 0x43};
  uint8_t ping[4] = {0x50, 0x49, 0x4E, 0x47};
  uint8_t eotx[4]  = {0x45, 0x4F, 0x54, 0x58}; //EOTX is not Standard, but may be received from M17-FME, depending on version
  // uint8_t m17p[4] = {0x4D, 0x31, 0x37, 0x50};

  //mainly just used here for quick source on ackn, nack, disc, etc
  char src_csd[20]; memset(src_csd, 0, sizeof(src_csd));
  convert_array_to_csd(input+4, src_csd);

  if (memcmp(input, m17s, 4) == 0) //m17 stream frame
  {

    //source info on stream starts at 12
    memset(src_csd, 0, sizeof(src_csd));
    convert_array_to_csd(input+12, src_csd);

    //new voice stream detection / notification
    if (busy_signal == 0)
      fprintf (stderr, "\nNew Stream From: %s ", src_csd);
    else fprintf (stderr, ".");

    //update stream traffic receive time and lockout stream 
    //replies (TTS, Play, Ads) until busy_signal is lifted
    last_stream_traffic_received = time(NULL);
    busy_signal = 1;

    //copy received CRC for the stream frame
    uint16_t str_crc_ext = (uint16_t)convert_bytes_into_output(input+52, 2);

    //calculate CRC for the stream frame
    uint16_t str_crc_cmp = crc16(input, 52);

    if (str_crc_ext == str_crc_cmp)
    {
      uint16_t fn = (input[34] << 8) | input[35]; fn &= 0x7FFF;
      uint8_t eot = input[34] >> 7;

      //debug
      // fprintf (stderr, " \n Stream FN: %04X;", fn);

      //debug 
      // fprintf (stderr, " CRC: %04X / %04X;", str_crc_ext, str_crc_cmp);

      //set busy_signal to 0 if EOT
      if (eot == 1)
      {
        busy_signal = 0;

        //add ~stream seconds so that an ad won't immediately hitch the end of a call if one became due during a voice stream
        if ( (send_advertisement_traffic == 1) && (time(NULL) - last_advertisement_traffic_sent) > advertisement_time_interval)
          last_advertisement_traffic_sent += (fn/25)+2;

        //end of stream detection
        fprintf (stderr, " EOT;");
      }
    }
    // else fprintf (stderr, "\n Stream CRC ERR: %04X / %04X;", str_crc_ext, str_crc_cmp);

  }
  else if (memcmp(input, m17p, 4) == 0) //m17 packet frame
  {

    //copy received CRC on LSF Portion
    uint16_t lsf_crc_ext = (uint16_t)convert_bytes_into_output(input+32, 2);

    //calculate CRC on LSF Portion
    uint16_t lsf_crc_cmp = crc16(input+4, 28);

    //copy received CRC on Data Portion
    uint16_t pkt_crc_ext = (uint16_t)convert_bytes_into_output(input+(len-2), 2);

    //calculate CRC on Data Portion
    uint16_t pkt_crc_cmp = crc16(input+34, len-2-34);

    //we have good crc now, so we can proceed
    if (lsf_crc_ext == lsf_crc_cmp && pkt_crc_ext == pkt_crc_cmp)
    {
      int ret = -1; UNUSED(ret);
      uint8_t protocol = input[34];
      if (protocol == 0x05)
        ret = ip_text_response(input, len-2);
    }

  }
  else if (memcmp(input, ackn, 4) == 0)
  {
    fprintf (stderr, "\nACKN from %s;", src_csd);
    last_ping_received = time(NULL);
  }
  else if (memcmp(input, ping, 4) == 0)
  {
    //Send the Pong w/ pre-encoded my_src_hex value
    ip_send_conn_disc_ping_pong(m17_udp_socket, my_src_hex, 3);
    fprintf (stderr, "."); //hearbeat
    last_ping_received = time(NULL);
  }
  else if (memcmp(input, nack, 4) == 0)
  {
    //connection issue?
    fprintf (stderr, "\nNACK from %s;", src_csd);
    exitflag = 1;
  }
  else if (memcmp(input, disc, 4) == 0)
  {
    //disconnected by reflector, or could be other user disc if ad-hoc
    fprintf (stderr, "\nDISC from %s;", src_csd);
  }
  else if (memcmp(input, eotx, 4) == 0)
  {
    //M17-FME sourced EOTX, consume and busy_signal 0
    // fprintf (stderr, "\nEOTX from %s;", src_csd);
    busy_signal = 0;
  }

  //dump
  // fprintf (stderr, "\n IP: ");
  // for (int i = 0; i < len; i++)
  //   fprintf (stderr, "%02X ", input[i]);

}

int ip_packet_frame_encoder(char * reply_string, uint8_t reply_protocol)
{
  //incrementing len of this packet
  int len = 0;

  uint8_t output[1000]; memset(output, 0, sizeof(output));

  //magic word m17p
  for (int i = 0; i < 4; i++)
    output[len++] = m17p[i];

  //add link setup data (LSF)
  len += ip_lsf_encoder(output+len, 0);

  //add the protocol byte we are replying with
  output[len++] = reply_protocol;

  //convert reply_string to array at len index
  len += convert_utf8_text_to_array(reply_string, output+len);

  //terminate if SMS or TLE
  if (reply_protocol == 0x05 || reply_protocol == 0x07)
    output[len++] = '\0'; //just 0x00

  //calculate and attach CRC16
  uint16_t crc = crc16(output+34, len-34);
  output[len++] = (crc >> 8) & 0xFF;
  output[len++] = (crc >> 0) & 0xFF;

  //debug dump the IP Frame created here
  // fprintf (stderr, "\n  R_IP: ");
  // for (int i = 0; i < len; i++)
  //   fprintf (stderr, "%02X ", output[i]);

  //send it
  len = udp_socket_blaster(m17_udp_socket, (size_t)len, output);

  return len;
}
