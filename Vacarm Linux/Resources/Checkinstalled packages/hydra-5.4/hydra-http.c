#include "hydra-mod.h"

extern char *HYDRA_EXIT;
char *buf;
char *webtarget = NULL;
char *slash = "/";
int webport;

int
start_http(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp, char *type)
{
  char *empty = "";
  char *login, *pass, buffer[500], buffer2[110];
  char *header = "";            /* XXX TODO */
  char *ptr;

  if (strlen(login = hydra_get_next_login()) == 0)
    login = empty;
  if (strlen(pass = hydra_get_next_password()) == 0)
    pass = empty;

  sprintf(buffer2, "%.50s:%.50s", login, pass);
  hydra_tobase64((unsigned char *) buffer2, strlen(buffer2), sizeof(buffer2));

  /* again: no snprintf to be portable. dont worry, buffer cant overflow */
  if (use_proxy == 1 && proxy_authentication != NULL)
    sprintf(buffer, "%s http://%s:%d%.250s HTTP/1.0\r\nHost: %s\r\nAuthorization: Basic %s\r\nProxy-Authorization: Basic %s\r\nUser-Agent: Mozilla/4.0 (Hydra)\r\n%s\r\n",
            type, webtarget, webport, miscptr, webtarget, buffer2, proxy_authentication, header);
  else {
    if (use_proxy == 1)
      sprintf(buffer, "%s http://%s:%d%.250s HTTP/1.0\r\nHost: %s\r\nAuthorization: Basic %s\r\nUser-Agent: Mozilla/4.0 (Hydra)\r\n%s\r\n",
            type, webtarget, webport, miscptr, webtarget, buffer2, header);
    else
      sprintf(buffer, "%s %.250s HTTP/1.0\r\nHost: %s\r\nAuthorization: Basic %s\r\nUser-Agent: Mozilla/4.0 (Hydra)\r\n%s\r\n",
            type, miscptr, webtarget, buffer2, header);
  }

  if (hydra_send(s, buffer, strlen(buffer), 0) < 0) {
    return 1;
  }

  buf = hydra_receive_line(s);
  while (strstr(buf, "HTTP/1.") == NULL && buf != NULL)
    buf = hydra_receive_line(s);

  if (buf == NULL)
    return 1;

/*
    while (hydra_data_ready(s) > 0)
      recv(s, buffer, sizeof(buf), 0);
        buf = hydra_receive_line(s);
*/

  ptr = ((char *) index(buf, ' ')) + 1;
  if (ptr != NULL && (*ptr == '2' || strncmp(ptr, "301", 3) == 0)) {
    hydra_report_found_host(port, ip, "www", fp);
    hydra_completed_pair_found();
  } else {
    if (ptr != NULL && *ptr != '4')
      printf("Unusual return code: %c for %s:%s\n", (char) *(index(buf, ' ') + 1), login, pass);
    hydra_completed_pair();
  }

  free(buf);

  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return 3;
  return 1;

}

void
service_http(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port, char *type)
{
  int run = 1, next_run, sock = -1;
  int myport = PORT_HTTP, mysslport = PORT_HTTP_SSL;
  struct sockaddr_in targetip;
  char *ptr, *ptr2;

  hydra_register_socket(sp);
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return;

  if ((webtarget = strstr(miscptr, "://")) != NULL) {
    webtarget += strlen("://");
    if ((ptr2 = index(webtarget, ':')) != NULL) { /* step over port if present */
      *ptr2 = 0;
      ptr2++;
      ptr = ptr2;
      if (*ptr == '/' || (ptr = index(ptr2, '/')) != NULL)
        miscptr = ptr;
      else
        miscptr = slash; /* to make things easier to user */
    } else if ((ptr2 = index(webtarget, '/')) != NULL) {
      miscptr = malloc(strlen(ptr2) + 1);
      strcpy(miscptr, ptr2);
      *ptr2 = 0;
    } else
      webtarget = NULL;
  }
  if (cmdlinetarget != NULL && webtarget == NULL)
    webtarget = cmdlinetarget;
  else
   if (webtarget == NULL && cmdlinetarget == NULL) {
    memset(&targetip, 0, sizeof(targetip));
    memcpy(&targetip.sin_addr.s_addr, &ip, 4);
    targetip.sin_family = AF_INET;
#ifdef CYGWIN
    buf = inet_ntoa((struct in_addr) targetip.sin_addr);
#else
    buf = malloc(20);
    inet_ntop(AF_INET, &targetip.sin_addr, buf, 20);
#endif
    webtarget = malloc(strlen(buf) + 1);
    strcpy(webtarget, buf);
#ifndef CYGWIN
    free(buf);
#endif
  }
  if (port != 0)
    webport = port;
  else if ((options & OPTION_SSL) == 0)
    webport = myport;
  else
    webport = mysslport;

  while (1) {
    next_run = 0;
    switch (run) {
    case 1:                    /* connect and service init function */
      {
        if (sock >= 0)
          sock = hydra_disconnect(sock);
//        usleep(275000);
        if ((options & OPTION_SSL) == 0) {
          if (port != 0)
            myport = port;
          sock = hydra_connect_tcp(ip, myport);
          port = myport;
        } else {
          if (port != 0)
            mysslport = port;
          sock = hydra_connect_ssl(ip, mysslport);
          port = mysslport;
        }
        if (sock < 0) {
          fprintf(stderr, "Error: Child with pid %d terminating, can not connect\n", (int) getpid());
          hydra_child_exit(1);
        }
        next_run = 2;
        break;
      }
    case 2:                    /* run the cracking function */
      next_run = start_http(sock, ip, port, options, miscptr, fp, type);
      break;
    case 3:                    /* clean exit */
      if (sock >= 0)
        sock = hydra_disconnect(sock);
      hydra_child_exit(0);
      return;
    default:
      fprintf(stderr, "Caught unknown return code, exiting!\n");
      hydra_child_exit(0);
      exit(-1);
    }
    run = next_run;
  }
}

void
service_http_get(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  service_http(ip, sp, options, miscptr, fp, port, "GET");
}

void
service_http_head(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  service_http(ip, sp, options, miscptr, fp, port, "HEAD");
}
