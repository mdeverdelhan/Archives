#include "hydra-mod.h"

#include "ntlm.h"

extern char *HYDRA_EXIT;

char *buf;
int counter;

int
start_imap_ntlm(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp)
{
  char *empty = "";
  char *login, *pass, buffer[300];
  //beware of fixed sized buffer, asserts may fail, don't use long strings :)
  //Yes, I Know, year 2k6 and still with this shit..
  unsigned char buf1[4096];
  unsigned char buf2[4096];


  if (strlen(login = hydra_get_next_login()) == 0)
    login = empty;
  if (strlen(pass = hydra_get_next_password()) == 0)
    pass = empty;

  while (hydra_data_ready(s)) {
    if ((buf = hydra_receive_line(s)) == NULL)
      return (1);
    free(buf);
  }

  //Send auth request
  sprintf(buffer, "51477 AUTHENTICATE NTLM\r\n");
  if (hydra_send(s, buffer, strlen(buffer), 0) < 0) return 1;
  //receive +  
  if ((buf = hydra_receive_line(s)) == NULL) return (1);

  //send auth and receive challenge
  //send auth request: let the server send it's own hostname and domainname
  buildAuthRequest((tSmbNtlmAuthRequest*)buf2,0,NULL,NULL);
  to64frombits(buf1, buf2, SmbLength((tSmbNtlmAuthResponse*)buf2));

  sprintf(buffer, "%s\r\n", buf1);
  if (hydra_send(s, buffer, strlen(buffer), 0) < 0) return 1;
  if ((buf = hydra_receive_line(s)) == NULL) return (1);

  //recover challenge
  from64tobits((char*)buf1, buf+2);

  //Send response
  buildAuthResponse((tSmbNtlmAuthChallenge*)buf1,(tSmbNtlmAuthResponse*)buf2,0,login,pass,NULL,NULL);
  to64frombits(buf1, buf2, SmbLength((tSmbNtlmAuthResponse*)buf2));

  sprintf(buffer, "%s\r\n", buf1);
  if (hydra_send(s, buffer, strlen(buffer), 0) < 0) return 1;
  if ((buf = hydra_receive_line(s)) == NULL) return (1);

  if (strstr(buf, " NO ") != NULL || strstr(buf, "failed") != NULL || strstr(buf, " BAD ") != NULL) {
    free(buf);
    hydra_completed_pair();
    if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
      return 3;
    if (counter == 3)
      return 1;
    return (2);
  }
  free(buf);

  hydra_report_found_host(port, ip, "imap-ntlm", fp);
  hydra_completed_pair_found();
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return 3;
  return 1;
}

void
service_imap_ntlm(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  int run = 1, next_run, sock = -1;
  int myport = PORT_IMAP, mysslport = PORT_IMAP_SSL;

  hydra_register_socket(sp);
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return;
  while (1) {
    switch (run) {
    case 1:                    /* connect and service init function */
      if (sock >= 0)
        sock = hydra_disconnect(sock);
//      usleep(275000);
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
      buf = hydra_receive_line(sock);
      if ((buf == NULL ) ||  ( strstr(buf, "OK") == NULL && buf[0] != '*')) {  /* check the first line */
        hydra_report(stderr, "Error: Not an IMAP protocol or service shutdown:\n");
        if ( buf != NULL ) {
	     	free(buf);   
        }
        hydra_child_exit(2);
        exit(-1);
      }
      free(buf);
      counter = 1;
      next_run = 2;
      break;
    case 2:                    /* run the cracking function */
      next_run = start_imap_ntlm(sock, ip, port, options, miscptr, fp);
      counter++;
      break;
    case 3:                    /* clean exit */
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
