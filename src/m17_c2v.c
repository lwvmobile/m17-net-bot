#include "main.h"

//encode and send any encoded 48k/1 wav file over stream as 3200 voice
int wav_to_stream(char * wav_filename)
{

  struct timeval t0, t1;
  float elapsed;

  SF_INFO info;
  info.samplerate = 48000;
  info.channels = 1;
  info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
  SNDFILE * wav_file = sf_open (wav_filename, SFM_READ, &info);

  if (wav_file == NULL)
  {
    fprintf (stderr, "\n Wav File %s not found.", wav_filename);
    return -1;
  }

  //crete codec2 context
  struct CODEC2 *codec2_3200;
  codec2_3200 = codec2_create(CODEC2_MODE_3200);

  //setup a SID
  srand(time(NULL));
  uint8_t sid[2]; memset(sid, 0, sizeof(sid));
  sid[0] = rand() & 0xFF;
  sid[1] = rand() & 0xFF;

  //setup a FN (frame number iterator)
  uint16_t fn = 0;

  //ip frame
  uint8_t ip_frame[1000]; memset(ip_frame, 0, sizeof(ip_frame));

  uint8_t read = 1;
  while (read)
  {
    //Get encoding start time
    gettimeofday(&t0, NULL);

    int k = 0;
    short voice_samp1[160]; memset(voice_samp1, 0, sizeof(voice_samp1));
    short voice_samp2[160]; memset(voice_samp2, 0, sizeof(voice_samp2));
    
    //start reading on fn 1 (fn 0 is silence)
    if (fn != 0)
    {

      //first sub-frame
      for (int i = 0; i < 960; i++)
      {
        short sample = 0;
        sf_count_t result = sf_read_short(wav_file, &sample, 1);

        //close file and break if EOT on the wav_file
        if (result == 0)
        {
          sf_close(wav_file);
          read = 0;
          break;
        }

        //take every 6th sample to decimate from 48k to 8k (1 channel)
        if ( ((i%6) == 0) )
          voice_samp1[k++] = sample;

      }

      //second sub-frame
      k = 0;
      for (int i = 0; i < 960; i++)
      {
        short sample = 0;
        sf_count_t result = sf_read_short(wav_file, &sample, 1);

        //close file and break if EOT on the wav_file
        if (result == 0)
        {
          sf_close(wav_file);
          read = 0;
          break;
        }

        //take every 6th sample to decimate from 48k to 8k (1 channel)
        if ( ((i%6) == 0) )
          voice_samp2[k++] = sample;

      }

    }

    //encode to codec2 3200
    uint8_t codec2_1[8]; memset(codec2_1, 0, sizeof(codec2_1));
    uint8_t codec2_2[8]; memset(codec2_2, 0, sizeof(codec2_2));
    codec2_encode(codec2_3200, codec2_1, voice_samp1);
    codec2_encode(codec2_3200, codec2_2, voice_samp2);

    //debug the voice encoding
    // fprintf (stderr, "\n CODEC2 (3200): ");
    // for (int i = 0; i < 8; i++)
    //   fprintf(stderr, "%02X", codec2_1[i]);
    // fprintf (stderr, "\n CODEC2 (3200): ");
    // for (int i = 0; i < 8; i++)
    //   fprintf(stderr, "%02X", codec2_2[i]);

    //encode the IP frame and send
    memset(ip_frame, 0, sizeof(ip_frame));
    ip_frame[0] = 0x4D; ip_frame[1] = 0x31; ip_frame[2] = 0x37; ip_frame[3] = 0x20;
    ip_frame[4] = sid[0]; ip_frame[5] = sid[1]; 

    ip_lsf_encoder(ip_frame+6, 1);

    //set read to 0 if exitflag at this point to get the EOT and exit loop
    if (exitflag)
      read = 0;

    //append EOT bit if this is the last frame
    if (read == 0)
      fn |= 0x8000;

    //append FN to the ip frame
    ip_frame[34] = (fn >> 8) & 0xFF;
    ip_frame[35] = (fn >> 0) & 0xFF;

    //insert silence on starting frame and ending frame
    if (fn == 0 || read == 0)
    {
      uint64_t silence = 0x010009439CE42108;

      //append silence to the ip frame
      for (int i = 0; i < 8; i++)
      {
        ip_frame[i+36] = (silence >> (56ULL-(i*8))) & 0xFF;
        ip_frame[i+44] = (silence >> (56ULL-(i*8))) & 0xFF;
      }
    }
    //append voice to the ip frame
    else
    {
      for (int i = 0; i < 8; i++)
      {
        ip_frame[i+36] = codec2_1[i];
        ip_frame[i+44] = codec2_2[i];
      }
    }

    //calculate and attach CRC
    uint16_t crc = crc16(ip_frame, 52);
    ip_frame[52] = (crc >> 8) & 0xFF;
    ip_frame[53] = (crc >> 0) & 0xFF;

    //send the IP frame
    udp_socket_blaster(m17_udp_socket, (size_t)54, ip_frame);

    if (adhoc_mode) //read frame back in to discard if ad-hoc mode
      udp_socket_receiver(m17_udp_bind, ip_frame);

    //Get encoding end time
    gettimeofday(&t1, NULL);

    //evaluate difference
    elapsed = time_difference_msec(t0, t1);

    //debug
    // fprintf(stderr, "Stream executed in %f milliseconds.\n", elapsed);
    
    //calculate delta and sleep for ~40 ms to simulate RF audio timing and not overload UDP buffers
    elapsed *= 1000.0f;
    if (elapsed < 40000.0f)
    {
      float delta = 40000.0f - elapsed;

      //debug
      // fprintf(stderr, "Elapsed: %f; Delta: %f; \n", elapsed, delta);

      usleep(delta);
    }

    //increment the frame number
    fn++;

  } //while (read)

  //destroy context after encoding session
  codec2_destroy(codec2_3200);

  return 0;

}