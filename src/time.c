#include "main.h"

void get_date_time_format(time_t t, char * string)
{
  struct tm * ptm = localtime(& t);
  sprintf(string, "(%04d-%02d-%02d %02d:%02d:%02d)", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}

void get_date_time_file(time_t t, char * string)
{
  struct tm * ptm = localtime(& t);
  sprintf(string, "%04d%02d%02d_%02d%02d%02d", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}

void get_local_asc_time(time_t t, char * string)
{
  struct tm * ptm = localtime(& t);
  sprintf(string, "Local Time: %s", asctime(ptm));

  int len = strlen(string);
  string[len-1] = '\0'; //remove endling line break
}

float time_difference_msec(struct timeval t0, struct timeval t1)
{
  return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}
