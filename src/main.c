#include "main.h"
#include "git_ver.h"

#include <signal.h>

//global extern variables from main.h

//signal handling
volatile uint8_t exitflag;

//Version 2 or Version 3
uint8_t lsf_type_version;

//UDP
int m17_udp_socket;
int m17_udp_bind;
char m17_udp_hostname[1024];
char m17_udp_bindname[1024];
int m17_udp_portno;

uint8_t reflector_module;
uint8_t adhoc_mode;
time_t last_ping_received;
time_t last_stream_traffic_received;
time_t last_traffic_sent_time;
time_t last_advertisement_traffic_sent;
time_t advertisement_time_interval;
uint8_t busy_signal;
uint8_t send_advertisement_traffic;

//User Config or Option Files
char advertisement_text_file[1024];
char pre_encoded_wav_file[1024];
char my_tle_source[900];
char my_weather_location[50];

//This Bot's LSF Data
uint8_t my_can;
char my_src_callsign[20];
char my_dst_callsign[20];
uint8_t my_src_hex[6];

//LSF Data Received from RX
uint8_t your_can;
char your_src_callsign[20];
char your_dst_callsign[20];

//Contextual LSF that is actually encoded and sent
uint8_t this_can;
char this_src_callsign[20];
char this_dst_callsign[20];

//end global extern variables

//init all global extern variables
void init_global(void)
{
  //set the exitflag to 0
  exitflag = 0;

  reflector_module = 0x43; // 'C'
  adhoc_mode = 0; //adhoc, or reflector

  last_ping_received = time(NULL);
  last_stream_traffic_received = time(NULL);
  last_traffic_sent_time = time(NULL);
  last_advertisement_traffic_sent = time(NULL);
  advertisement_time_interval = 3600; //1 hour in seconds
  busy_signal = 0;
  send_advertisement_traffic = 0;

  m17_udp_socket = -1;
  m17_udp_bind = -1;
  memset(m17_udp_hostname, 0, sizeof(m17_udp_hostname));
  sprintf(m17_udp_hostname, "%s", "localhost");
  memset(m17_udp_bindname, 0, sizeof(m17_udp_bindname));
  sprintf(m17_udp_bindname, "%s", "0.0.0.0");
  m17_udp_portno = 17000;

  lsf_type_version = 2; //2 for compatibility mode

  //User Config or Option Files
  memset(advertisement_text_file, 0, sizeof(advertisement_text_file));
  memset(pre_encoded_wav_file, 0, sizeof(pre_encoded_wav_file));
  memset(my_tle_source, 0, sizeof(my_tle_source));
  memset(my_weather_location, 0, sizeof(my_weather_location));

  //TLE sources -- browse more at: http://celestrak.org/
  sprintf(my_tle_source, "%s", "https://live.ariss.org/iss.txt");
  // sprintf(my_tle_source, "%s", "http://celestrak.org/NORAD/elements/supplemental/sup-gp.php?FILE=starlink-g6-86&FORMAT=tle");

  //This Bot's LSF Data
  my_can = 0;
  memset(my_src_callsign, 0, sizeof(my_src_callsign));
  memset(my_dst_callsign, 0, sizeof(my_dst_callsign));
  memset(my_src_hex, 0, sizeof(my_src_hex));

  //LSF Data Received from RX
  your_can = 0;
  memset(your_src_callsign, 0, sizeof(your_src_callsign));
  memset(your_dst_callsign, 0, sizeof(your_dst_callsign));

  sprintf(my_src_callsign, "%s", "");
  sprintf(my_dst_callsign, "%s", "BROADCAST"); //default to broadcast

  //Contextual LSF that is actually encoded and sent
  memset(this_src_callsign, 0, sizeof(this_src_callsign));
  memset(this_dst_callsign, 0, sizeof(this_dst_callsign));

  //activate the Python environment with PIPER1 installed in it
  //create environment with "python3 -m venv piper1" -- no quotations
  //activate with "source piper1/bin/activate"
  //install piper1 with "pip install piper-tts"
  //list voices with "python3 -m piper.download_voices"
  //download voice with "python3 -m piper.download_voices en_US-norman-medium"
  #ifdef PIPER1
  system("source piper1/bin/activate");
  #endif

}

void handler(int sgnl)
{
  UNUSED(sgnl);
  exitflag = 1;
}

int read_config_file(char * filename)
{
  int len = -1;

  //open file, if it exists
  FILE * config_file = fopen(filename, "r");

  if (config_file == NULL)
  {
    fprintf (stderr, "\n Configuration File %s not found. Exiting.", filename);
    exitflag = 1;
    return len;
  }

  //debug
  fprintf (stderr, "\n Config:");

  while (!feof(config_file))
  {
    char * config_string = calloc(100, sizeof(char));

    fgets(config_string, 99, config_file);

    //replace ending linebreak with a terminator
    len = strcspn(config_string, "\n");
    config_string[len] = '\0';

    fprintf (stderr, "\n %s", config_string);

    //evaluate first part of string, then copy second part of string
    if (strncmp(config_string, "my_src_callsign=", 16) == 0)
    {
      memset(my_src_callsign, 0, sizeof(my_src_callsign));
      strncpy(my_src_callsign, config_string+16, 9); //only copy out 9 chars for callsigns

      memset(my_src_hex, 0, sizeof(my_src_hex));
      convert_csd_to_array(my_src_hex, my_src_callsign);

      //debug
      // fprintf (stderr, " - %s", my_src_callsign);
    }
    else if (strncmp(config_string, "my_dst_callsign=", 16) == 0)
    {
      memset(my_dst_callsign, 0, sizeof(my_dst_callsign));
      strncpy(my_dst_callsign, config_string+16, 9); //only copy out 9 chars for callsigns

      //debug
      // fprintf (stderr, " - %s", my_dst_callsign);
    }
    else if (strncmp(config_string, "m17_udp_hostname=", 17) == 0)
    {
      memset(m17_udp_hostname, 0, sizeof(m17_udp_hostname));
      sprintf(m17_udp_hostname, "%s", config_string+17);

      //debug
      // fprintf (stderr, " - %s", m17_udp_hostname);
    }
    else if (strncmp(config_string, "m17_udp_portno=", 15) == 0)
    {
      m17_udp_portno = strtol(config_string+15, NULL, 10);

      //debug
      // fprintf (stderr, " - %d", m17_udp_portno);
    }
    else if (strncmp(config_string, "reflector_module=", 17) == 0)
    {
      reflector_module = config_string[17];

      //debug
      // fprintf (stderr, " - 0x%02X", reflector_module);
    }
    else if (strncmp(config_string, "adhoc_mode=", 11) == 0)
    {
      if (config_string[11] == '1')
        adhoc_mode = 1;
      else adhoc_mode = 0;

      //debug
      // fprintf (stderr, " - %d", adhoc_mode);
    }
    else if (strncmp(config_string, "send_advertisement_traffic=", 27) == 0)
    {
      if (config_string[27] == '1')
        send_advertisement_traffic = 1;
      else send_advertisement_traffic = 0;

      //debug
      // fprintf (stderr, " - %d", send_advertisement_traffic);
    }
    else if (strncmp(config_string, "advertisement_text_file=", 24) == 0)
    {
      memset(advertisement_text_file, 0, sizeof(advertisement_text_file));
      sprintf(advertisement_text_file, "%s", config_string+24);

      //debug
      // fprintf (stderr, " - %s", advertisement_text_file);
    }
    else if (strncmp(config_string, "pre_encoded_wav_file=", 21) == 0)
    {
      memset(pre_encoded_wav_file, 0, sizeof(pre_encoded_wav_file));
      sprintf(pre_encoded_wav_file, "%s", config_string+21);

      //debug
      // fprintf (stderr, " - %s", pre_encoded_wav_file);
    }
    else if (strncmp(config_string, "lsf_type_version=", 17) == 0)
    {
      if (config_string[17] == '3')
        lsf_type_version = 3;
      else lsf_type_version = 2;

      //debug
      // fprintf (stderr, " - V%d", lsf_type_version);
    }
    else if (strncmp(config_string, "my_can=", 7) == 0)
    {
      my_can = strtol(config_string+7, NULL, 10);

      if(my_can <= 15) {} //valid can
      else my_can = 0;

      //debug
      // fprintf (stderr, " - %d", my_can);
    }
    else if (strncmp(config_string, "advertisement_time_interval=", 28) == 0)
    {
      advertisement_time_interval = strtol(config_string+28, NULL, 10);

      //debug
      // fprintf (stderr, " - %ld", advertisement_time_interval);
    }
    else if (strncmp(config_string, "my_weather_location=", 20) == 0)
    {
      memset(my_weather_location, 0, sizeof(my_weather_location));
      sprintf(my_weather_location, "%s", config_string+20);

      //debug
      // fprintf (stderr, " - %s", my_weather_location);
    }
    else if (strncmp(config_string, "my_tle_source=", 14) == 0)
    {
      memset(my_tle_source, 0, sizeof(my_tle_source));
      sprintf(my_tle_source, "%s", config_string+14);

      //debug
      // fprintf (stderr, " - %s", my_tle_source);
    }

    //free allocated memory
    free (config_string);
  }

  //close file
  fclose(config_file);

  fprintf (stderr, "\n\n");

  return len;

}

int main(int argc, char ** argv)
{

  fprintf (stderr, "\n M17 Net Bot");
  fprintf (stderr, "\n Build Version: %s", GIT_TAG);
  fprintf (stderr, "\n Spec Version:  %s", SPEC_VERSION);
  fprintf (stderr, "\n Spec Date:     %s", SPEC_DATE);
  #ifdef PIPER1
  fprintf (stderr, "\n TTS Engine:    piper1-gpl");
  #elif ESPEAK
  fprintf (stderr, "\n TTS Engine:    espeak");
  #elif ESPEAKNG
  fprintf (stderr, "\n TTS Engine:    espeak-ng");
  #else
  fprintf (stderr, "\n TTS Engine:    none");
  #endif
  fprintf (stderr, "\n");

  //init global variables
  init_global();

  //read in user configuration file as solo argv
  if(argc > 1)
    read_config_file(argv[argc-1]);
  else
  {
    fprintf (stderr, "\n No configuration file supplied.");
    return 0;
  }

  //call signal handler so things like ctrl+c will allow us to gracefully close
  signal (SIGINT, handler);
  signal (SIGTERM, handler);

  //the main loop
  ip_bot_start();

  //linebreak
  fprintf (stderr, "\n");

  return 0;
}
