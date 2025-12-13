#ifndef MAIN_H
#define MAIN_H

//M17 Versioning
#define SPEC_VERSION "3.0.0-draft"
#define SPEC_DATE "Dec 04, 2025"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

//Useful Warning Shoosh
#define UNUSED(x) ((void)x)

//libsndfile support
#include <sndfile.h>

//codec2 support
#include <codec2/codec2.h>

//signal handling exitflag
extern volatile uint8_t exitflag;

//utility functions
uint64_t convert_bits_into_output(uint8_t * input, int len);
uint64_t convert_bytes_into_output(uint8_t * input, int len);
void pack_bit_array_into_byte_array (uint8_t * input, uint8_t * output, int len);
void pack_bit_array_into_byte_array_asym (uint8_t * input, uint8_t * output, int len);
void unpack_byte_array_into_bit_array (uint8_t * input, uint8_t * output, int len);
void left_shift_byte_array (uint8_t * input, uint8_t * output, int len);
uint16_t convert_hex_string_to_array (char * input, uint8_t * output);
uint16_t convert_utf8_text_to_array (char * input, uint8_t * output);

//time functions
void get_date_time_format(time_t t, char * string);
void get_date_time_file(time_t t, char * string);
void get_local_asc_time(time_t t, char * string);
float time_difference_msec(struct timeval t0, struct timeval t1);

//CRC16 For IP Frames
uint16_t crc16(const uint8_t * in, const uint16_t len);

//UDP Socket
int udp_socket_bind(int mode, int portno);
int udp_socket_connect(int sockfd, char * hostname, int portno);
int udp_socket_blaster(int sockfd, size_t nsam, void * data);
int udp_socket_receiver(int sockfd, void * data);
extern int m17_udp_socket;
extern int m17_udp_bind;
extern char m17_udp_hostname[1024];
extern char m17_udp_bindname[1024];
extern int m17_udp_portno;

//Callsign Encode and Decode
void convert_array_to_csd(uint8_t * input, char * callsign);
void convert_csd_to_array(uint8_t * output, char * callsign);
int my_callsign_to_letters(char * csd, char * output);

//Link Setup Data (LSF)
int ip_lsf_encoder(uint8_t * input, uint8_t is_stream);
extern uint8_t lsf_type_version; //version 2, or version 3

//Voice Stream
int wav_to_stream(char * wav_filename);

//User Config or Option Files
extern char advertisement_text_file[1024];
extern char pre_encoded_wav_file[1024];
extern char my_tle_source[900];
extern char my_weather_location[50];

//This Bot's LSF Data
extern uint8_t my_can;
extern char my_src_callsign[20];
extern char my_dst_callsign[20];
extern uint8_t my_src_hex[6];

//LSF Data Received from RX
extern uint8_t your_can;
extern char your_src_callsign[20];
extern char your_dst_callsign[20];

//Contextual LSF that is actually encoded and sent
extern uint8_t this_can;
extern char this_src_callsign[20];
extern char this_dst_callsign[20];

//IP Frame
void ip_send_conn_disc_ping_pong (int sockfd, uint8_t * source, uint8_t cd);
void ip_frame_decoder (uint8_t * input, int len);
int ip_packet_frame_encoder(char * reply_string, uint8_t reply_protocol);
extern uint8_t adhoc_mode;
extern uint8_t reflector_module;
extern time_t last_ping_received;
extern time_t last_stream_traffic_received;
extern time_t last_traffic_sent_time;
extern time_t last_advertisement_traffic_sent;
extern time_t advertisement_time_interval;
extern uint8_t busy_signal;
extern uint8_t send_advertisement_traffic;

//Bot Capabilities
void ip_bot_start(void);
int ip_text_response (uint8_t * input, int len);
int tts_to_stream(char * tts_string);
int text_file_to_string(char * filename, char * output_string);
int text_file_to_stream(char * filename);
int get_tle_string (char * reply_string);
int get_eight_ball_string (char * reply_string);
int get_lucky_numbers_string (char * reply_string);
int get_wttr_string (char * reply_string, char * weather_location);
void bot_advertisement(void);

#endif // MAIN_H
