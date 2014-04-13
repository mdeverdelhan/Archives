#ifndef _HYDRA_MOD_H
#define _HYDRA_MOD_H

#ifdef NESSUS_PLUGIN
#include <includes.h>
#endif

#include "hydra.h"

extern void hydra_child_exit(int code);
extern void hydra_register_socket(int s);
extern char *hydra_get_next_pair();
extern char *hydra_get_next_login();
extern char *hydra_get_next_password();
extern void hydra_completed_pair();
extern void hydra_completed_pair_found();
extern void hydra_completed_pair_skip();
extern void hydra_report_found(int port, char *svc, FILE * fp);
extern void hydra_report_found_host(int port, unsigned int ip, char *svc, FILE * fp);
extern void hydra_report_found_host_msg(int port, unsigned int ip, char *svc, FILE *fp, char *msg);
extern int hydra_connect_ssl(unsigned long int host, int port);
extern int hydra_connect_tcp(unsigned long int host, int port);
extern int hydra_connect_udp(unsigned long int host, int port);
extern int hydra_disconnect(int socket);
extern int hydra_data_ready(int socket);
extern int hydra_recv(int socket, char *buf, int length);
extern char *hydra_receive_line(int socket);
extern int hydra_send(int socket, char *buf, int size, int options);
extern int make_to_lower(char *buf);
extern unsigned char hydra_conv64(unsigned char in);
extern void hydra_tobase64(unsigned char *buf, int buflen, int bufsize);
extern void hydra_dump_asciihex(unsigned char *string, int length);
extern void hydra_set_srcport(int port);

int debug;
int verbose;
int waittime;
int port;
int use_proxy;
int found;
unsigned long int proxy_string_ip;
int proxy_string_port;
char *proxy_authentication;
char *cmdlinetarget;

#define hydra_report fprintf

#endif
