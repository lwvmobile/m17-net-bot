#include "main.h"

struct sockaddr_in addressM17;
struct sockaddr_in addressM17duplex;

int udp_socket_bind(int mode, int portno)
{

  UNUSED(mode);

  int sockfd;
  struct sockaddr_in serveraddr;

  /* socket: create the socket */
  //UDP socket
  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (sockfd < 0)
  {
    fprintf(stderr,"ERROR opening UDP socket\n");
    exit(0);
  }

  /* build the server's Internet address */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY
  serveraddr.sin_port = htons(portno);

  //Bind socket to listening
  if (bind(sockfd, (struct sockaddr *) &serveraddr,  sizeof(serveraddr)) < 0)
  { 
    fprintf(stderr,"ERROR on binding UDP Port");
    exit(0);
  }

  //set these for non blocking when no samples to read
  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = 10;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

  return sockfd;
}

int udp_socket_connect(int sockfd, char * hostname, int portno)
{
  int err = 0;
  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sockfd < 0)
  {
    fprintf (stderr, " UDP Socket Error %d\n", sockfd);
    return (sockfd);
  }

  // Don't think this is needed, but doesn't seem to hurt to keep it here either
  int broadcastEnable = 1;
  err = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

  if (err < 0)
  {
    fprintf (stderr, " UDP Broadcast Set Error %d\n", err);
    return (err);
  }

  //set these for non blocking when no samples to read (or speed up responsiveness to ncurses)
  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = 10;
  err = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

  if (err < 0)
  {
    fprintf (stderr, " UDP Read Timeout Set Error %d\n", err);
    return (err);
  }

  memset((char * ) & addressM17, 0, sizeof(addressM17));
  addressM17.sin_family = AF_INET;
  err = inet_aton(hostname, &addressM17.sin_addr); //inet_aton handles broadcast .255 addresses correctly in some environments (Raspbian GNU/Linux 11 (bullseye) armv7l)
  if (err < 0)
  {
    fprintf (stderr, " UDP inet_addr Error %d\n", err);
    return (err);
  }

  addressM17.sin_port = htons(portno);
  if (err < 0)
  {
    fprintf (stderr, " UDP htons Error %d\n", err);
    return (err);
  }

  return sockfd;
}

int udp_socket_blaster(int sockfd, size_t nsam, void * data)
{
  int err = 0;
  err = sendto(sockfd, data, nsam, 0, (const struct sockaddr * ) & addressM17, sizeof(struct sockaddr_in));
  return (err);
}

int udp_socket_receiver(int sockfd, void * data)
{
  size_t err = 0;
  struct sockaddr_in cliaddr; 
  socklen_t len = sizeof(cliaddr); 

  //receive data from socket
  err = recvfrom(sockfd, data, 1000, 0, (struct sockaddr * ) & addressM17duplex, &len);

  return err;
}
