#include <PalmOS.h>
#include <sys_socket.h>
#include <DataMgr.h>
#include <SoundMgr.h>
#include <StringMgr.h>
#include "hydra.h"

/*--------------------------------------------------------*/

#define PORT_FTP          21
#define PORT_FTP_SSL     990
#define PORT_POP3        110
#define PORT_POP3_SSL    995
#define PORT_TELNET       23
#define PORT_TELNET_SSL  992
#define PORT_VNC        5900
#define PORT_VNC_SSL    5901
#define PORT_IMAP	143
#define PORT_IMAP_SSL	993
#define PORT_NNTP       119
#define PORT_NNTP_SSL   563
#define PORT_VMAUTHD    902
#define PORT_VMAUTHD_SSL 902


#define OPTION_SSL         1

#define DB_ID_PRUT      0x50525554   /* "PRUT" */
#define DB_ID_PASS      0x50415353   /* "PASS" */
#define DB_ID_LOGI      0x4c4f4749   /* "LOGI" */

#define DB_ID_PC_DATA   0x61446174   /* "Data" */
#define DB_ID_PC_CREA   0x72436b63   /* "Crck" */

#define FILE unsigned char

/*--------------------------------------------------------*/


extern UInt16 AppNetRefnum;
extern int running;
extern DmOpenRef passData;
extern DmOpenRef loginData;
extern int pass_index;
extern int login_index;
extern char fixed_login[25];
extern char fixed_pass[25];

int waittime;
int debug;
int verbose;

/*--------------------------------------------------------*/


Int16 hydra_connect_tcp(unsigned long int ip, UInt16 port);
Int16 hydra_connect_udp(unsigned long int ip, UInt16 port);
int   hydra_recv(Int16 socket, char *buf, int length);
int   hydra_send(Int16 socket, char *buf, int size, int options);
int   hydra_disconnect(Int16 socket);
Int16 hydra_connect_ssl(unsigned long int ip, UInt16 port);
char *hydra_receive_line(Int16 socket);
int   hydra_data_ready(Int16 socket);
int   hydra_data_ready_timed(Int16 socket, long sec, long usec );
void  hydra_report_found(UInt16 port, char *svc, int fp);
void  hydra_report_found_host(UInt16 port, unsigned long int ip, char* svc, FILE * fp);
void  hydra_completed_pair_found();
void  hydra_completed_pair();
char *hydra_get_next_login();
char *hydra_get_next_password();
char *hydra_get_next_pair();
void  hydra_register_socket(int s);
void  SetText(unsigned int id, char *text);
void  usleep(unsigned long usec);
void  hydra_child_exit(int i);
int   make_to_lower(char *buf);


/*some functions to make porting easier...*/
void  hydra_report(FILE *stream, const char *format, ... );
int   getpid();

/*--------------------------------------------------------*/

int start_ftp(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp);
void service_ftp(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);

int start_pop3(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp);
void service_pop3(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);

int start_telnet(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp);
void service_telnet(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);

int start_vnc(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp);
void service_vnc(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);

int start_imap(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp);
void service_imap(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);

int start_nntp(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp);
void service_nntp(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
