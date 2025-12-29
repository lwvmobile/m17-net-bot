#include "main.h"

void ip_bot_start(void)
{
  //udp connections for adhoc
  if (adhoc_mode == 1)
  {
    m17_udp_bind = udp_socket_bind(4, m17_udp_portno); //4 is ip4, but unused in this context for now
    m17_udp_socket = udp_socket_connect(m17_udp_socket, m17_udp_hostname, m17_udp_portno);
  }
  else //reflector
    m17_udp_socket = udp_socket_connect(m17_udp_socket, m17_udp_hostname, m17_udp_portno);

  //send CONN
  ip_send_conn_disc_ping_pong(m17_udp_socket, my_src_hex, 1);

  //this is the main loop
  while (!exitflag)
  {
    int len = 0;
    uint8_t ip_frame[1000];
    memset(ip_frame, 0, sizeof(ip_frame));

    if (adhoc_mode == 1)
      len = udp_socket_receiver(m17_udp_bind, ip_frame);
    else len = udp_socket_receiver(m17_udp_socket, ip_frame);

    if (len > 0)
      ip_frame_decoder(ip_frame, len);

    //lift a busy signal after 2 seconds of stream traffic
    if ( (busy_signal == 1) && ((time(NULL) - last_stream_traffic_received) > 2) )
    {
      busy_signal = 0;
      fprintf (stderr, "Stream Timeout;");
    }

    if (busy_signal == 0 && send_advertisement_traffic == 1 && (time(NULL) - last_advertisement_traffic_sent) > advertisement_time_interval)
      bot_advertisement();

    //check reflector connection
    if ((adhoc_mode == 0) && ((time(NULL) - last_ping_received) > 60))
    {
      fprintf (stderr, "\n Connection Issue? Attempting Reconnect.");
      last_ping_received -= 30;

      //send DISC
      ip_send_conn_disc_ping_pong(m17_udp_socket, my_src_hex, 0);

      //close sockets, if open
      if (m17_udp_socket > 0)
        close (m17_udp_socket);
      if (m17_udp_bind > 0)
        close (m17_udp_bind);

      //reconfigure sockets
      //udp connections for adhoc
      if (adhoc_mode == 1)
      {
        m17_udp_bind = udp_socket_bind(4, m17_udp_portno); //4 is ip4, but unused in this context for now
        m17_udp_socket = udp_socket_connect(m17_udp_socket, m17_udp_hostname, m17_udp_portno);
      }
      else //reflector
        m17_udp_socket = udp_socket_connect(m17_udp_socket, m17_udp_hostname, m17_udp_portno);

      //SEND CONN
      ip_send_conn_disc_ping_pong(m17_udp_socket, my_src_hex, 1);
      
    }

  }

  //send DISC
  ip_send_conn_disc_ping_pong(m17_udp_socket, my_src_hex, 0);

  //close sockets, if open
  if (m17_udp_socket > 0)
    close (m17_udp_socket);
  if (m17_udp_bind > 0)
    close (m17_udp_bind);

}

/* espeak-ng cheat sheet

-g <integer>
	   Word gap. Pause between words, units of 10mS at the default speed
-a <integer>
	   Amplitude, 0 to 200, default is 100
-p <integer>
	   Pitch adjustment, 0 to 99, default is 50
-s <integer>
	   Speed in approximate words per minute. The default is 175
-w <wave file name>
	   Write speech to this WAV file, rather than speaking it directly
-v <voice name>
	   Use voice file of this name from espeak-ng-data/voices

 5  en-029          --/M      English_(Caribbean) gmw/en-029           (en 10)
 2  en-gb           --/M      English_(Great_Britain) gmw/en               (en 2)
 5  en-gb-scotland  --/M      English_(Scotland) gmw/en-GB-scotland   (en 4)
 5  en-gb-x-gbclan  --/M      English_(Lancaster) gmw/en-GB-x-gbclan   (en-gb 3)(en 5)
 5  en-gb-x-gbcwmd  --/M      English_(West_Midlands) gmw/en-GB-x-gbcwmd   (en-gb 9)(en 9)
 5  en-gb-x-rp      --/M      English_(Received_Pronunciation) gmw/en-GB-x-rp       (en-gb 4)(en 5)
 2  en-us           --/M      English_(America)  gmw/en-US            (en 3)
 5  en-us-nyc       --/M      English_(America,_New_York_City) gmw/en-US-ny

*/

int tts_to_stream(char * tts_string)
{
  char command[4096]; memset(command, 0, sizeof(command));
  char src_csd[20]; memset(src_csd, 0, sizeof(src_csd));
  char dst_csd[20]; memset(dst_csd, 0, sizeof(dst_csd));

  //callsign to callsign as first portion of TTS
  my_callsign_to_letters(this_src_callsign, src_csd);
  my_callsign_to_letters(this_dst_callsign, dst_csd);

  //local playback for debug
  // #ifdef ESPEAK
  // sprintf(command, "espeak \"%s to %s %s\" -v en-gb-x-rp -s 140 -p 20 -a 75 &", src_csd, dst_csd, tts_string); //the & is a non-blocking way to execute
  // #endif
  // #ifdef ESPEAKNG
  // sprintf(command, "espeak-ng \"%s to %s %s\" -v en-gb-x-rp -s 140 -p 20 -a 75 &", src_csd, dst_csd, tts_string); //the & is a non-blocking way to execute
  // #endif
  // #ifdef PIPER1
  // sprintf(command, "python3 -m piper -m en_US-norman-medium --volume 0.85 -- \'%s to %s %s\' &", src_csd, dst_csd, tts_string); //note: local playback on ARCH / Python?? is broken ATM
  // #endif
  // system(command);

  //to wav file
  #ifdef ESPEAK
  sprintf(command, "espeak \"%s to %s %s\" -v en-gb-x-rp -s 140 -p 20 -a 75 -w tts.wav", src_csd, dst_csd, tts_string);
  #endif
  #ifdef ESPEAKNG
  sprintf(command, "espeak-ng \"%s to %s %s\" -v en-gb-x-rp -s 140 -p 20 -a 75 -w tts.wav", src_csd, dst_csd, tts_string);
  #endif
  #ifdef PIPER1
  sprintf(command, "python3 -m piper -m en_US-norman-medium -f tts.wav --volume 0.45 -- \'%s to %s %s\'", src_csd, dst_csd, tts_string);
  #endif
  system(command);

  //debug
  fprintf (stderr, "\nTTS Message: %s", tts_string);

  //if TTS command entered, then execute steps below
  if (command[0] != 0)
  {
    //resample wav file to 48k that is easily decimated to 8k that can be read in with sndfile and encoded to codec2
    memset(command, 0, sizeof(command));
    sprintf(command, "%s", "sox tts.wav -r 48000 encode.wav");
    system(command);

    //send to stream encoder to encode and send over IP
    wav_to_stream("encode.wav");
    remove("tts.wav");    //remove the file after using
    remove("encode.wav"); //remove the file after using
  }
  else fprintf (stderr, "\n No TTS support enabled.");

  int len = strlen((const char*)tts_string);
  return len;
}

int text_file_to_stream(char * filename)
{
  int len = -1;

  //open file, if it exists
  FILE * text_file = fopen(filename, "r");

  //if it doesn't exist, return
  if (text_file == NULL)
  {
    fprintf (stderr, "\n Text File %s not found.", filename);
    return len;
  }

  char * input_string = calloc(4096, sizeof(char));

  //read the file in to as the reply string
  len = fread(input_string, 1, 4096, text_file);

  //text file can only be up to 4000 characters to fit in command size
  char tts_string[4000]; memset(tts_string, 0, sizeof(tts_string));

  //copy out only 4000 characters
  strncpy(tts_string, input_string, 4000);

  //send to tts encoder
  len = tts_to_stream(tts_string);

  //close file
  fclose(text_file);

  //free allocated memory
  free (input_string);

  return len;

}

int text_file_to_string(char * filename, char * output_string)
{
  int len = -1;

  //open file, if it exists
  FILE * text_file = fopen(filename, "r");

  //if it doesn't exist, return
  if (text_file == NULL)
  {
    fprintf (stderr, "\n Text File %s not found.", filename);
    return len;
  }

  char * input_string = calloc(4096, sizeof(char));

  //read the file in to as the reply string
  len = fread(input_string, 1, 4096, text_file);

  //copy out only 823 characters
  strncpy(output_string, input_string, 823);

  //close file
  fclose(text_file);

  //free allocated memory
  free (input_string);

  return len;

}

//https://github.com/chubin/wttr.in?tab=readme-ov-file#one-line-output
int get_wttr_string (char * reply_string, char * weather_location)
{

  // char * format = "%t+%f+%C+%w+%h+%l";
  char * format = "Weather:%l+Temp:%t+Conditions:%C+Wind:%w+Humidity:%h";
  int len = -1;
  char command[1024]; memset(command, 0, sizeof(command));
  fprintf (stderr, "\n\n");
  sprintf (command, "wget https://wttr.in/%s?format=%s -O weather.txt --timeout=20", weather_location, format);
  system(command);

  //sleep 1 second to allow this to write
  sleep(1);

  //open file, if it exists
  FILE * weather_file = fopen("weather.txt", "r");

  //if it doesn't exist, return
  if (weather_file == NULL)
  {
    sprintf(reply_string, "%s", "Weather Data Not Acquired.");
    len = strlen((const char*)reply_string);
  }

  //read the file in to as the reply string
  else len = fread(reply_string, 1, 1023, weather_file);

  //close file and remove it for next time
  if (weather_file != NULL)
  {
    fclose(weather_file);
    remove("weather.txt");
  }

  //safety len / term check on this, had a larger file come in a few times
  //when location wasn't resolved and it overloaded the encoder and crashed
  if (len > 1023)
    len = 1023;
  reply_string[1023] = '\0';

  return len;
}

//get tle data from preconfigured my_tle_source
int get_tle_string (char * reply_string)
{
  int len = -1;
  char command[1024]; memset(command, 0, sizeof(command));
  fprintf (stderr, "\n\n");
  sprintf (command, "wget --timeout=20 -O tle.txt %s", my_tle_source);
  system(command);

  //sleep 1 second to allow this to write
  sleep(1);

  //open file, if it exists
  FILE * tle_file = fopen("tle.txt", "r");

  //if it doesn't exist, return
  if (tle_file == NULL)
  {
    sprintf(reply_string, "%s", "TLE Data Not Acquired.");
    len = strlen((const char*)reply_string);
  }

  //read the file in to as the reply string
  else len = fread(reply_string, 1, 1023, tle_file);

  //close file and remove it for next time
  if (tle_file != NULL)
  {
    fclose(tle_file);
    remove("tle.txt");
  }

  return len;
}

int get_eight_ball_string (char * reply_string)
{
  srand(time(NULL));
  uint8_t number = rand()%20;

  //yes answers
  if (number == 0)
    sprintf (reply_string, "%s", "It is certain.");
  else if (number == 1)
    sprintf (reply_string, "%s", "It is decidedly so.");
  else if (number == 2)
    sprintf (reply_string, "%s", "Without a doubt.");
  else if (number == 3)
    sprintf (reply_string, "%s", "Yes, definitely.");
  else if (number == 4)
    sprintf (reply_string, "%s", "You may rely on it.");
  else if (number == 5)
    sprintf (reply_string, "%s", "As I see it, yes.");
  else if (number == 6)
    sprintf (reply_string, "%s", "Most likely.");
  else if (number == 7)
    sprintf (reply_string, "%s", "Outlook good.");
  else if (number == 8)
    sprintf (reply_string, "%s", "Yes.");
  else if (number == 9)
    sprintf (reply_string, "%s", "Signs point to yes.");

  //no answers
  else if (number == 10)
    sprintf (reply_string, "%s", "Don't count on it.");
  else if (number == 11)
    sprintf (reply_string, "%s", "My reply is no.");
  else if (number == 12)
    sprintf (reply_string, "%s", "My sources say no.");
  else if (number == 13)
    sprintf (reply_string, "%s", "Outlook not so good.");
  else if (number == 14)
    sprintf (reply_string, "%s", "Very doubtful.");

  //non-comittal answers
  else if (number == 15)
    sprintf (reply_string, "%s", "Reply hazy, try again.");
  else if (number == 16)
    sprintf (reply_string, "%s", "Ask again later.");
  else if (number == 17)
    sprintf (reply_string, "%s", "Better not tell you now.");
  else if (number == 18)
    sprintf (reply_string, "%s", "Cannot predict now.");
  else if (number == 19)
    sprintf (reply_string, "%s", "Concentrate and ask again.");

  //catch-all (should never occur)
  else sprintf (reply_string, "%s", "Tell the dev the 8-ball is broken today.");

  int len = strlen((const char*)reply_string);
  return len;
}

//return 5 random 2 digit numbers (1-69) and 1 random 2-digit number (1-26)
int get_lucky_numbers_string (char * reply_string)
{

  //seed the randomizer
  srand(time(NULL));

  sprintf (reply_string, "Your lucky numbers are %02d, %02d, %02d, %02d, %02d, and %02d.",
              1+rand()%69, 1+rand()%69, 1+rand()%69, 1+rand()%69, 1+rand()%69, 1+rand()%26);

  int len = strlen((const char*)reply_string);
  return len;
}

int ip_text_response (uint8_t * input, int len)
{

  len -= (34 + 1 + 2); //relevant len at this point

  if (len < 0)
    return len;

  uint8_t protocol = input[34];
  uint8_t reply_protocol = 0x05;
  char text_string[850]; memset(text_string, 0, sizeof(text_string));
  char reply_string[1024]; memset(reply_string, 0, sizeof(reply_string));
  strncpy(text_string, (const char *)input+36, len);
  text_string[len] = '\0';

  //todo: proper lsf decode, for now, just get their callsign and can decoded
  memset(your_src_callsign, 0, sizeof(your_src_callsign));
  convert_array_to_csd(input+10, your_src_callsign);
  uint16_t type = (input[16] << 8) | (input[17] << 0);
  if ( (type & 0xF000) ) //version 3
    your_can = (type >> 0) & 0xF;
  else //version 2
    your_can = (type >> 7) & 0xF;

  //setup reply can, src, and dst callsigns
  this_can = your_can; //can will be requestors can
  memset(this_dst_callsign, 0, sizeof(this_dst_callsign)); 
  strncpy(this_dst_callsign, your_src_callsign, 9); //dst callsign will be the requestors callsign
  memset(this_src_callsign, 0, sizeof(this_src_callsign));
  strncpy(this_src_callsign, my_src_callsign, 9); //src callsign will be this bot's callsign

  //read the string to see if there is a command (test using * as shorthand command, since T9 should have that on right on 0 key)
  if (protocol == 0x05 && input[35] == '*')
  {
    if (strncmp(text_string, "get tle", 7) == 0)
    {
      sprintf (reply_string, "Attempting to get TLE Data.");
      reply_protocol = 0x05;
      ip_packet_frame_encoder(reply_string, reply_protocol);
      memset(reply_string, 0, sizeof(reply_string));
      get_tle_string(reply_string);
      reply_protocol = 0x07;
      ip_packet_frame_encoder(reply_string, reply_protocol);
    }
    //this has potential to crash if malformed location, just fyi
    else if (strncmp(text_string, "get wttr", 8) == 0)
    {
      char weather_location[50]; memset(weather_location, 0, sizeof(weather_location));
      if (text_string[8] == 0x20 && text_string[9] != 0x20 && text_string[9] != 0x00)
        sprintf (weather_location, "%s", text_string+9);
      else sprintf (weather_location, "%s", my_weather_location);

      //need to ensure weather_location doesn't have spaces in it -- new york city becomes new_york_city
      for (int i = 0; i < 50; i++)
      {
        if (weather_location[i] == 0x20)
          weather_location[i] = 0x5F;
      }

      sprintf (reply_string, "Attempting to get Weather Data for %s. Please Wait!", weather_location);
      reply_protocol = 0x05;
      ip_packet_frame_encoder(reply_string, reply_protocol);
      memset(reply_string, 0, sizeof(reply_string));
      get_wttr_string(reply_string, weather_location);
      reply_protocol = 0x05;
      ip_packet_frame_encoder(reply_string, reply_protocol);

      //encode TTS using espeak-ng and return play the encoded audio over voice stream.
      if (busy_signal == 0)
        tts_to_stream (reply_string);
    }
    //get local asctime and return as SMS Text
    else if (strncmp(text_string, "get time", 8) == 0)
    {
      get_local_asc_time(time(NULL), reply_string);
      reply_protocol = 0x05;
      ip_packet_frame_encoder(reply_string, reply_protocol);
    }
    else if (strncmp(text_string, "get lucky", 9) == 0)
    {
      reply_protocol = 0x05;
      get_lucky_numbers_string(reply_string);
      ip_packet_frame_encoder(reply_string, reply_protocol);
    }
    else if (strncmp(text_string, "get 8ball", 9) == 0)
    {
      reply_protocol = 0x05;
      get_eight_ball_string(reply_string);
      ip_packet_frame_encoder(reply_string, reply_protocol);
    }
    else if (strncmp(text_string, "tts ", 4) == 0)
    {
      //encode TTS using espeak-ng and play the encoded audio over voice stream.
      if (busy_signal == 0)
        tts_to_stream (text_string+4);
      else
      {
        sprintf (reply_string, "%s", "Reflector Busy with Stream. Please Try Again.");
        reply_protocol = 0x05;
        ip_packet_frame_encoder(reply_string, reply_protocol);
      }
    }
    else if (strncmp(text_string, "play ", 5) == 0)
    {
      //send name of file to stream encoder to encode and send over IP
      if (busy_signal == 0)
        wav_to_stream(text_string+5);
      else
      {
        sprintf (reply_string, "%s", "Reflector Busy with Stream. Please Try Again.");
        reply_protocol = 0x05;
        ip_packet_frame_encoder(reply_string, reply_protocol);
      }
      //NOTE: If file isn't found, it will just put out 2 frames of silence and EOT
    }
    else if (strncmp(text_string, "read ", 5) == 0)
    {
      //send name of file to read in and send to tts to encode and send over IP
      if (busy_signal == 0)
        text_file_to_stream(text_string+5);
      else
      {
        sprintf (reply_string, "%s", "Reflector Busy with Stream. Please Try Again.");
        reply_protocol = 0x05;
        ip_packet_frame_encoder(reply_string, reply_protocol);
      }
      //NOTE: will probably be a few frames at most, if empty.
    }
    else
    {
      //default case
      sprintf (reply_string, "Don't know how to \"%s\"", text_string);
      reply_protocol = 0x05;
      ip_packet_frame_encoder(reply_string, reply_protocol);
    }

    //debug print
    fprintf (stderr, "\nBot Reply to %s command \"%s\": \n%s", this_dst_callsign, text_string, reply_string);

  }

  return len;

}

//annoying advertisements
void bot_advertisement(void)
{

  int err = -1;

  //setup ad LSF info (basically just my to this)
  this_can = my_can;
  memset(this_dst_callsign, 0, sizeof(this_dst_callsign));
  strncpy(this_dst_callsign, my_dst_callsign, 9);
  memset(this_src_callsign, 0, sizeof(this_src_callsign));
  strncpy(this_src_callsign, my_src_callsign, 9);

  if (pre_encoded_wav_file[0] == 0)
    err = text_file_to_stream(advertisement_text_file);
  else err = wav_to_stream(pre_encoded_wav_file);

  //err == -1 means it didn't find advertisement_text_file or pre_encoded_wav_file
  if (err == -1)
  {
    fprintf (stderr, " Disabling Advertising.");
    send_advertisement_traffic = 0;
    return;
  } 

  char ad_string[823]; memset(ad_string, 0, sizeof(ad_string));
  err = text_file_to_string(advertisement_text_file, ad_string);

  //err == -1 means it didn't find advertisement_text_file or pre_encoded_wav_file
  if (err == -1)
  {
    fprintf (stderr, " Disabling Advertising.");
    send_advertisement_traffic = 0;
    return;
  }

  ip_packet_frame_encoder(ad_string, 0x05);
  last_advertisement_traffic_sent = time(NULL);

  //debug print
  char ad_time_string[100]; memset(ad_time_string, 0, sizeof(ad_time_string));
  get_local_asc_time(last_advertisement_traffic_sent, ad_time_string);
  fprintf (stderr, "\nAdvertisement Traffic Sent at %s ", ad_time_string);
}
