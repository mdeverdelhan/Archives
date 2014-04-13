#ifdef PALM
#include "palm/hydra-mod.h"
#else
#include "hydra-mod.h"
#endif

extern char *HYDRA_EXIT;

unsigned char *buf;

int
start_socks5(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp)
{
  char *empty = "";
  char *login, *pass, buffer[300];
  int pport;

  if (strlen(login = hydra_get_next_login()) == 0)
    login = empty;
  if (strlen(pass = hydra_get_next_password()) == 0)
    pass = empty;

  memcpy(buffer, "\x05\x02\x00\x02", 4);
  if (hydra_send(s, buffer, 4, 0) < 0) {
    return 1;
  }
  if ((buf = (unsigned char *) hydra_receive_line(s)) == NULL)
    return (1);
  if (buf[0] != 5) {
    hydra_report(stderr, "Socks5 protocol error or service shutdown: %s\n", buf);
    free(buf);
    return (4);
  }
  if (buf[1] == 0 || buf[1] == 32) {
    hydra_report(stderr, "Socks5 server does NOT require any authentication!\n");
    free(buf);
    return(4);
  }
  if (buf[1] != 0x2) {
    hydra_report(stderr, "Socks5 protocol error or service shutdown: %s\n", buf);
    free(buf);
    return(4);
  }
  free(buf);

#ifdef PALM
  sprintf(buffer, "\x01%c%s%c%s", (char)strlen(login), login, (char)strlen(pass), pass);
#else
  snprintf(buffer, sizeof(buffer), "\x01%c%s%c%s", (char)strlen(login), login, (char)strlen(pass), pass);
#endif

  if (hydra_send(s, buffer, strlen(buffer), 0) < 0)
    return 1;

  if ((buf = (unsigned char *) hydra_receive_line(s)) == NULL)
    return (1);
  if (buf[1] != 255) {
    /* new: false positive check */
    free(buf);
    pport = htons(port);
    memcpy(buffer, "\x05\x01\x00\x01", 4);
    memcpy(buffer + 4, &ip, 4);
    memcpy(buffer + 8, &pport, 2);
    hydra_send(s, buffer, 10, 0);
    if ((buf = (unsigned char *) hydra_receive_line(s)) != NULL) {
      if (buf[1] == 0 || buf[1] == 32) {
        hydra_report_found_host(port, ip, "socks5", fp);
      } else if (buf[1] != 2) {
        hydra_report_found_host_msg(port, ip, "socks5", fp, "might be a false positive!");
      }
    }
  }
  if (buf != NULL)
    free(buf);
  hydra_completed_pair();
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return 3;

  return 2;
}

void
service_socks5(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  int run = 1, next_run, sock = -1;
  int myport = PORT_SOCKS5, mysslport = PORT_SOCKS5_SSL;

  hydra_register_socket(sp);
  if (port != 0)
    myport = port;
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
      next_run = 2;
      break;
    case 2:                    /* run the cracking function */
      next_run = start_socks5(sock, ip, port, options, miscptr, fp);
      break;
    case 3:                    /* clean exit */
      if (sock >= 0)
        sock = hydra_disconnect(sock);
      hydra_child_exit(0);
      return;
    case 4:                    /* clean exit */
      if (sock >= 0)
        sock = hydra_disconnect(sock);
      hydra_child_exit(2);
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
