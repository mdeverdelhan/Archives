#include "hydra-mod.h"

extern char *HYDRA_EXIT;
char *buf;

int
start_http_proxy(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp)
{
  char *empty = "";
  char *login, *pass, buffer[500], buffer2[110];
  char url[210], host[30];
  char *header = "";            /* XXX TODO */
  char *ptr;

  if (strlen(login = hydra_get_next_login()) == 0)
    login = empty;
  if (strlen(pass = hydra_get_next_password()) == 0)
    pass = empty;

  sprintf(buffer2, "%.50s:%.50s", login, pass);
  hydra_tobase64((unsigned char *) buffer2, strlen(buffer2), sizeof(buffer2));

  if (miscptr == NULL) {
    strcpy(url, "http://www.suse.com/");
    strcpy(host, "Host: www.suse.com\r\n");
  } else {
    sprintf(url, "%.200s", miscptr);
    strcpy(host, ""); /* too lazy to parse the URL */
  }

  /* to be portable, no snprintf, buffer is big enough so it cant overflow */
  sprintf(buffer, "HEAD %s HTTP/1.0\r\n%sProxy-Authorization: Basic %s\r\nUser-Agent: Mozilla/4.0 (Hydra)\r\n%s\r\n", url, host, buffer2, header);

  if (hydra_send(s, buffer, strlen(buffer), 0) < 0)
    return 1;

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
  if (*ptr == '2' || (*ptr == '3' && *(ptr + 2) == '1')) {
    hydra_report_found_host(port, ip, "http-proxy", fp);
    hydra_completed_pair_found();
  } else {
    if (*ptr != '4')
      printf("Unusual return code: %c for %s:%s\n", (char) *(index(buf, ' ') + 1), login, pass);
    hydra_completed_pair();
  }

  free(buf);

  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return 3;
  return 1;

}

void
service_http_proxy(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  int run = 1, next_run, sock = -1;
  int myport = PORT_HTTP_PROXY, mysslport = PORT_HTTP_PROXY_SSL;

  hydra_register_socket(sp);
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return;

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
      next_run = start_http_proxy(sock, ip, port, options, miscptr, fp);
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
