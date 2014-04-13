#ifdef PALM
#include "palm/hydra-mod.h"
#else
#include "hydra-mod.h"
#endif

extern char *HYDRA_EXIT;
char *buf;

int
start_nntp(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp)
{
  char *empty = "\"\"";
  char *login, *pass, buffer[300];
  int i = 1;

  if (strlen(login = hydra_get_next_login()) == 0)
    login = empty;
  if (strlen(pass = hydra_get_next_password()) == 0)
    pass = empty;

  while (i > 0 && hydra_data_ready(s) > 0)
    i = hydra_recv(s, buffer, 300);

#ifdef PALM
  sprintf(buffer, "AUTHINFO USER %s\r\n", login);
#else
  sprintf(buffer, "AUTHINFO USER %.250s\r\n", login);
#endif
  if (hydra_send(s, buffer, strlen(buffer), 0) < 0) {
    return 1;
  }
  buf = hydra_receive_line(s);
  if (buf == NULL)
    return 1;
  if (buf[0] != '3') {
    hydra_report(stderr, "Error: Not an NNTP protocol or service shutdown: %s\n", buf);
    free(buf);
    return (3);
  }
  free(buf);

#ifdef PALM
  sprintf(buffer, "AUTHINFO PASS %s\r\n", pass);
#else
  sprintf(buffer, "AUTHINFO PASS %.250s\r\n", pass);
#endif

  if (hydra_send(s, buffer, strlen(buffer), 0) < 0) {
    return 1;
  }
  buf = hydra_receive_line(s);
  if (buf == NULL)
    return 1;
  if (buf[0] == '2') {
    hydra_report_found_host(port, ip, "nntp", fp);
    hydra_completed_pair_found();
    if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
      return 3;
    free(buf);
    return 1;
  }

  free(buf);
  hydra_completed_pair();
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return 3;

  return 2;
}

void
service_nntp(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  int run = 1, next_run, sock = -1;
  int myport = PORT_NNTP, mysslport = PORT_NNTP_SSL;

  hydra_register_socket(sp);
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return;
  while (1) {
    switch (run) {
    case 1:                    /* connect and service init function */
      if (sock >= 0)
        sock = hydra_disconnect(sock);
//      usleep(300000);
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
        hydra_report(stderr, "Error: Child with pid %d terminating, can not connect\n", (int) getpid());
        hydra_child_exit(1);
      }
//      usleep(300000);
      buf = hydra_receive_line(sock);
      if (buf == NULL || buf[0] != '2') {       /* check the first line */
        hydra_report(stderr, "Error: Not an NNTP protocol or service shutdown: %s\n", buf);
        hydra_child_exit(2);
        free(buf);
#ifdef PALM
	return;
#else
        exit(-1);
#endif
      }
      free(buf);

/*                    buf = hydra_receive_line(sock); */

/*                    free(buf);                      */
//      usleep(1500000);
      usleep(25000);
      buf = malloc(1024);
      while (hydra_data_ready(sock) > 0)
        hydra_recv(sock, buf, 1024);
      free(buf);
      next_run = 2;
      break;
    case 2:                    /* run the cracking function */
      next_run = start_nntp(sock, ip, port, options, miscptr, fp);
      break;
    case 3:                    /* clean exit */
      if (sock >= 0)
        sock = hydra_disconnect(sock);
      hydra_child_exit(0);
      return;
    default:
      hydra_report(stderr, "Caught unknown return code, exiting!\n");
      hydra_child_exit(0);
#ifdef PALM
	return;
#else
        exit(-1);
#endif
    }
    run = next_run;
  }
}
