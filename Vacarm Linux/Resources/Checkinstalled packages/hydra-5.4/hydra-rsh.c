//This plugin was written by <david dot maciejak at kyxar dot fr>
//under GPLv2 license
//
#ifdef PALM
#include "palm/hydra-mod.h"
#else
#include "hydra-mod.h"
#endif

#define COMMAND "/bin/ls /"

extern char *HYDRA_EXIT;
char *buf;

int
start_rsh(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp)
{
  char *empty = "";
  char *login, buffer[300] = "", buffer2[100], *bptr = buffer2;  
  int ret;

  if (strlen(login = hydra_get_next_login()) == 0)
    login = empty;
  
  memset(buffer2, 0, sizeof(buffer2));
  bptr++;
  
  strcpy(bptr, login);
  bptr += 1 + strlen(login);

  strcpy(bptr, login);
  bptr += 1 + strlen(login);

  strcpy(bptr, COMMAND);

  if (hydra_send(s, buffer2, 4 + strlen(login) + strlen(login) + strlen(COMMAND), 0) < 0)
  {
  	return 4;
  }
  ret = hydra_recv(s, buffer, sizeof(buffer));  
  
  if (ret > 0 && buffer[0] == 0) {
    hydra_report_found_host(port, ip, "rsh", fp);
    hydra_completed_pair_found();
  } else
    hydra_completed_pair();

  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return 3;
  return 2;
}

void
service_rsh(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  int run = 1, next_run, sock = -1;
  int myport = PORT_RSH, mysslport = PORT_RSH_SSL;
 
  hydra_register_socket(sp);
  hydra_set_srcport(1023);
  
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
          hydra_report(stderr, "Error: Child with pid %d terminating, can not connect\n", (int) getpid());
          hydra_child_exit(1);
        }
        next_run = 2;
        break;
      }
    case 2:                    /* run the cracking function */
      next_run = start_rsh(sock, ip, port, options, miscptr, fp);
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
