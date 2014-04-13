#include "hydra-mod.h"

extern char *HYDRA_EXIT;

char *buf;

int
start_smtpauth(int s, unsigned long int ip, int port, unsigned char options,
	       char *miscptr, FILE * fp)
{
  char *empty = "";
  char *login, *pass, buffer[300], buffer2[250];

  if (strlen(login = hydra_get_next_login()) == 0)
    login = empty;
  if (strlen(pass = hydra_get_next_password()) == 0)
    pass = empty;

  while (hydra_data_ready(s) > 0) {
    if ((buf = hydra_receive_line(s)) == NULL)
      return (1);
    free(buf);
  }
  sprintf(buffer, "AUTH LOGIN\r\n");
  if (hydra_send(s, buffer, strlen(buffer), 0) < 0) {
    return 1;
  }
  if ((buf = hydra_receive_line(s)) == NULL)
    return 1;
  if (strstr(buf, "334") == NULL) {
    hydra_report(stderr, "Error: SMTP AUTH LOGIN error: %s\n", buf);
    free(buf);
    return 3;
  }
  free(buf);

  sprintf(buffer2, "%.250s", login);
  hydra_tobase64((unsigned char *) buffer2, strlen(buffer2), sizeof(buffer2));
  sprintf(buffer, "%.250s\r\n", buffer2);

  if (hydra_send(s, buffer, strlen(buffer), 0) < 0) {
    return 1;
  }
  if ((buf = hydra_receive_line(s)) == NULL)
    return (1);
  if (strstr(buf, "334") == NULL) {
    hydra_report(stderr, "Error: SMTP AUTH LOGIN error: %s\n", buf);
    free(buf);
    return (3);
  }
  free(buf);

  sprintf(buffer2, "%.250s", pass);
  hydra_tobase64((unsigned char *) buffer2, strlen(buffer2), sizeof(buffer2));
  sprintf(buffer, "%.250s\r\n", buffer2);

  if (hydra_send(s, buffer, strlen(buffer), 0) < 0) {
    return 1;
  }
  if ((buf = hydra_receive_line(s)) == NULL)
    return (1);
  if (strstr(buf, "235") != NULL) {
    hydra_report_found_host(port, ip, "smtpauth", fp);
    hydra_completed_pair_found();
    free(buf);
    if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
      return 3;
    return 1;
  }
  free(buf);
  hydra_completed_pair();
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return 3;

  return 2;
}

void
service_smtpauth(unsigned long int ip, int sp, unsigned char options,
		 char *miscptr, FILE * fp, int port)
{
  int run = 1, next_run, sock = -1, resp = 0;
  int myport = PORT_SMTPAUTH, mysslport = PORT_SMTPAUTH_SSL;
  char *ptr;

  /*  char *buffer = "EHLO mail.thc.org\r\n"; */
  char *buffer1 = "EHLO hydra\r\n";
  char *buffer2 = "HELO hydra\r\n";

  hydra_register_socket(sp);
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return;
  while (1) {
    switch (run) {
    case 1:			/* connect and service init function */
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
	port = myport;
      }
      if (sock < 0) {
	hydra_report(stderr,
		     "Error: Child with pid %d terminating, can not connect\n",
		     (int) getpid());
	hydra_child_exit(1);
      }

//      while (hydra_data_ready(sock)) {
//	if ((buf = hydra_receive_line(sock)) == NULL)
//	  exit(-1);
//	free(buf);
//      }

      /* receive initial header */
      if ((buf = hydra_receive_line(sock)) == NULL)
	exit(-1);
      if (strstr(buf, "220") == NULL) {
	hydra_report(stderr, "Warning: SMTP does not allow to connect: %s\n",
		     buf);
	hydra_child_exit(2);
	exit(-1);
      }
      while (strstr(buf, "220 ") == NULL) {
        free(buf);
        buf = hydra_receive_line(sock);
      }
      free(buf);

      /* send ehlo and receive/ignore reply */
      if (hydra_send(sock, buffer1, strlen(buffer1), 0) < 0)
	exit(-1);
      resp = 0;
      do {
        ptr = buf = hydra_receive_line(sock);
        if (buf != NULL) {
          if (isdigit(buf[0]) && buf[3] == ' ')
            resp = 1;
          else {
            if (buf[strlen(buf) - 1] == '\n')
              buf[strlen(buf) - 1] = 0;
            if (buf[strlen(buf) - 1] == '\r')
              buf[strlen(buf) - 1] = 0;
            if ((ptr = rindex(buf, '\n')) != NULL) {
              ptr++;
              if (isdigit(*ptr) && *(ptr + 3) == ' ')
                resp = 1;
            }
          }
        }
      } while (buf != NULL && resp == 0);
//      buf != NULL && buf[3] != ' ' && strstr(buf, "250 ") == NULL);
      if (buf == NULL)
        exit(-1); 
     if (*ptr != '2') {
      if (hydra_send(sock, buffer2, strlen(buffer2), 0) < 0)
	exit(-1);
      resp = 0;
      do {
        ptr = buf = hydra_receive_line(sock);
        if (buf != NULL) {
          if (isdigit(buf[0]) && buf[3] == ' ')
            resp = 1;
          else {
            if (buf[strlen(buf) - 1] == '\n')
              buf[strlen(buf) - 1] = 0;
            if (buf[strlen(buf) - 1] == '\r')
              buf[strlen(buf) - 1] = 0;
            if ((ptr = rindex(buf, '\n')) != NULL) {
              ptr++;
              if (isdigit(*ptr) && *(ptr + 3) == ' ')
                resp = 1;
            }
          }
        }
      } while (buf != NULL && resp == 0);
//      buf != NULL && buf[3] != ' ' && strstr(buf, "250 ") == NULL);
      if (buf == NULL)
        exit(-1); 
     }
/*    old
      if ((buf = hydra_receive_line(sock)) == NULL)
	exit(-1);
*/

      /* check AUTH LOGIN supported */
/*
      if (strstr(buf, "LOGIN") == NULL) {
        hydra_report(stderr, "Warning: SMTP AUTH LOGIN might not be supported: %s\n", buf);
      }
*/
      free(buf);

      next_run = 2;
      break;
    case 2:			/* run the cracking function */
      next_run = start_smtpauth(sock, ip, port, options, miscptr, fp);
      break;
    case 3:			/* clean exit */
      if (sock >= 0)
	sock = hydra_disconnect(sock);
      hydra_child_exit(0);
      return;
    default:
      hydra_report(stderr, "Caught unknown return code, exiting!\n");
      hydra_child_exit(0);
      exit(-1);
    }
    run = next_run;
  }
}
