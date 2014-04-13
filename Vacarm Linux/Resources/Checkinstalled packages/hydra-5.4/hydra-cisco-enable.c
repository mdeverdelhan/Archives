#include "hydra-mod.h"

extern char *HYDRA_EXIT;
char *buf;

int
start_cisco_enable(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp)
{
  char *empty = "";
  char *pass, buffer[300];

  if (strlen(pass = hydra_get_next_password()) == 0)
    pass = empty;

  sprintf(buffer, "%.250s\r\n", pass);
  if (hydra_send(s, buffer, strlen(buffer), 0) < 0) {
    return 1;
  }
  buf = hydra_receive_line(s);
  if (strstr(buf, "assw") != NULL) {
    hydra_completed_pair();
    free(buf);
    if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
      return 3;
    if (strlen(pass = hydra_get_next_password()) == 0)
      pass = empty;
    sprintf(buffer, "%.250s\r\n", pass);
    if (hydra_send(s, buffer, strlen(buffer), 0) < 0) {
      return 1;
    }
    buf = hydra_receive_line(s);
    if (strstr(buf, "assw") != NULL) {
      hydra_completed_pair();
      free(buf);
      if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
        return 3;
      if (strlen(pass = hydra_get_next_password()) == 0)
        pass = empty;
      sprintf(buffer, "%.250s\r\n", pass);
      if (hydra_send(s, buffer, strlen(buffer), 0) < 0) {
        return 1;
      }
      buf = hydra_receive_line(s);
    }
  }

  if (strstr(buf, "assw") != NULL || strstr(buf, "ad ") != NULL || strstr(buf, "attempt") != NULL || strstr(buf, "fail") != NULL || strstr(buf, "denied") != NULL) {
    free(buf);
    hydra_completed_pair();
    if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
      return 3;
    return 2;
  }

  hydra_report_found_host(port, ip, "cisco-enable", fp);
  hydra_completed_pair_found();
  return 3;
}

void
service_cisco_enable(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  int run = 1, failc = 0, retry = 1, next_run, sock = -1;
  int myport = PORT_TELNET, mysslport = PORT_TELNET_SSL;
  char buffer[300];
  char *login;

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

        /* Cisco AAA Support */
        if (strlen(login = hydra_get_next_login()) != 0) {
          while ((buf = hydra_receive_line(sock)) != NULL && strstr(buf, "name") == NULL) {
            free(buf);
          }

          sprintf(buffer, "%.250s\r", login);
          if (hydra_send(sock, buffer, strlen(buffer) + 1, 0) < 0) {
            fprintf(stderr, "Error: Child with pid %d terminating, can not send login\n", (int) getpid());
            hydra_child_exit(2);
          }
        }

        if (miscptr != NULL) {
          while ((buf = hydra_receive_line(sock)) != NULL && strstr(buf, "assw") == NULL) {
            free(buf);
          }

          sprintf(buffer, "%.250s\r\n", miscptr);
          if (hydra_send(sock, buffer, strlen(buffer), 0) < 0) {
            fprintf(stderr, "Error: Child with pid %d terminating, can not send login\n", (int) getpid());
            hydra_child_exit(2);
          }
        }

        buf = hydra_receive_line(sock);
        if (strstr(buf, "assw") != NULL) {
          fprintf(stderr, "Error: Child with pid %d terminating - can not login, can not login\n", (int) getpid());
          hydra_child_exit(2);
        }
        free(buf);

        next_run = 2;
        break;
      }
    case 2:                    /* run the cracking function */
      {
        unsigned char *buf2 = malloc(256);
        int f = 0;

        sprintf(buffer, "%.250s\r\n", "ena");
        if (hydra_send(sock, buffer, strlen(buffer), 0) < 0) {
          fprintf(stderr, "Error: Child with pid %d terminating, can not send 'ena'\n", (int) getpid());
          hydra_child_exit(2);
        }

        do {
          if (f != 0)
            free(buf2);
          else
            f = 1;
          if ((buf2 = (unsigned char *) hydra_receive_line(sock)) == NULL) {
            if (failc < retry) {
              next_run = 1;
              failc++;
              fprintf(stderr, "Error: Child with pid %d was disconnected - retrying (%d of %d retries)\n", getpid(), failc, retry);
              sleep(3);
              break;
            } else {
              fprintf(stderr, "Error: Child with pid %d was disconnected - exiting\n", getpid());
              hydra_child_exit(0);
            }
          }
        } while (strstr((char *) buf2, "assw") == NULL);
        free(buf2);
        if (next_run != 0)
          break;
        failc = 0;

        next_run = start_cisco_enable(sock, ip, port, options, miscptr, fp);
        break;
      }
    case 3:                    /* clean exit */
      sprintf(buffer, "%.250s\r\n", "exit");
      if (hydra_send(sock, buffer, strlen(buffer), 0) < 0) {
        fprintf(stderr, "Error: Child with pid %d terminating, can not send 'exit'\n", (int) getpid());
        hydra_child_exit(0);
      }
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
