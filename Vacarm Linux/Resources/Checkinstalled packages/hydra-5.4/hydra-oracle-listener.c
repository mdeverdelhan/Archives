/*
 * with help from ...
 *
 */

#include "hydra-mod.h"

#define HASHSIZE 16

#warning "The Oracle Listener module does not work yet"

extern char *HYDRA_EXIT;
char *buf;
unsigned char *hash;

void ora_hash_password(char *pass) {
  // secret hash function comes here, and written to char *hash
  memcpy(hash, "9BD20802761D432E", HASHSIZE);
  // ...
  // ...
}

int
start_oracle_listener(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp)
{
  unsigned char tns_packet[280] = {
    "\x01\x18\x00\x00\x01\x00\x00\x00\x01\x38\x01\x2c\x00\x00\x08\x00"
    "\x7f\xff\x86\x0e\x00\x00\x01\x00\x00\xde\x00\x3a\x00\x00\x02\x00"
    "\x41\x41\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x09\x94\x00\x00"
    "\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00"
    "(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=10.0.0.2)(PORT=1521))"
    "(CONNECT_DATA=(COMMAND=RELOAD)(PASSWORD=XXXXXXXXXXXXXXXX)"
    "(SERVICE=LISTENER)(CID=(PROGRAM=c:\\oracle\\ora92\\bin\\sqlplus.exe)"
    "(HOST=LAPTOP00)(USER=Administrator))))"
  };
  char *empty = "";
  char *pass;

  if (strlen(pass = hydra_get_next_password()) == 0)
    pass = empty;

  hash = &tns_packet[0] + 161;
  
  ora_hash_password(pass);
  
  if (hydra_send(s, (char*)tns_packet, sizeof(tns_packet), 0) < 0) {
    return 1;
  }
  
  if ((buf = hydra_receive_line(s)) == NULL)
    return 1;

  if (strstr(buf, "ERR=0") != NULL) {
    hydra_report_found_host(port, ip, "oracle-listener", fp);
    hydra_completed_pair_found();
  } else
    hydra_completed_pair();

  free(buf);
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return 3;
  return 1;
}

void
service_oracle_listener(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  int run = 1, next_run, sock = -1;
  int myport = PORT_ORACLE, mysslport = PORT_ORACLE_SSL;

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
      /* run the cracking function */
      next_run = start_oracle_listener(sock, ip, port, options, miscptr, fp);
      break;
    case 3:                    /* clean exit */
      if (sock >= 0)
        sock = hydra_disconnect(sock);
      hydra_child_exit(0);
      return;
    case 4:
      if (sock >=0)
        sock = hydra_disconnect(sock);
      hydra_child_exit(2);
      return;
    default:
      hydra_report(stderr, "Caught unknown return code, exiting!\n");
      hydra_child_exit(0);
        exit(-1);
    }
    run = next_run;
  }
}
