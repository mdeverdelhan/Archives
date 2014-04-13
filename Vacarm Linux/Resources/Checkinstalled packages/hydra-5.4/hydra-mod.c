#include "hydra-mod.h"

#ifdef LIBOPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#define MAX_CONNECT_RETRY 1
#define WAIT_BETWEEN_CONNECT_RETRY 3
#define HYDRA_DUMP_ROWS 16

int intern_socket, extern_socket;
char pair[260];
char HYDRA_EXIT[5] = "\x00\xff\x00\xff\x00";
char *HYDRA_EMPTY = "\x00\x00\x00\x00";
int fail = 0;
int alarm_went_off = 0;
int use_ssl = 0;
char ipaddr_str[INET_ADDRSTRLEN];
int src_port=0;

#ifdef LIBOPENSSL
SSL *ssl = NULL;
SSL_CTX *sslContext = NULL;
RSA *rsa = NULL;
#endif

/* prototype */
int my_select(int fd,fd_set *fdread, fd_set *fdwrite,fd_set *fdex,long sec, long usec);

/* ----------------- alarming functions ---------------- */

void
alarming()
{
  fail++;
  alarm_went_off++;

/* uh, I think it's not good for performance if we try to reconnect to a timeout system!
 *  if (fail > MAX_CONNECT_RETRY) {
 */
  fprintf(stderr, "Process %d: Can not connect [timeout], process exiting\n", (int) getpid());
  if (debug)
    printf("DEBUG_CONNECT_TIMEOUT\n");
  hydra_child_exit(1);

/*
 *   } else {
 *     if (verbose) fprintf(stderr, "Process %d: Can not connect [timeout], retrying (%d of %d retries)\n", (int)getpid(), fail, MAX_CONNECT_RETRY);
 *   }
 */
}

void
interrupt()
{
  if (debug)
    printf("DEBUG_INTERRUPTED\n");
}

/* ----------------- internal functions ----------------- */

int
internal__hydra_connect(unsigned long int host, int port, int protocol, int type)
{
  int s, ret = -1;
  struct sockaddr_in target;
  char *buf, *tmpptr = NULL;
  struct sockaddr_in sin;

#ifndef CYGWIN
  char out[16];
#endif

  if ((s = socket(PF_INET, protocol, type)) >= 0) {
    if (src_port != 0)
    {
      int bind_ok=0;
      sin.sin_family = PF_INET;
      sin.sin_port = htons(src_port);
      sin.sin_addr.s_addr = INADDR_ANY;
      
      //we will try to find a free port down to 512
      while (!bind_ok && src_port >= 512)
      {
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin))==-1)
	{
	      if (verbose)
	        perror("error:");
	      if (errno == EADDRINUSE)
	      {
	      	  src_port--;
		  sin.sin_port = htons(src_port);
	      }
	      else
	      {
		if (errno == EACCES && (getuid() > 0))
		{
			printf("You need to be root to test this service\n");
			return -1;
		}
	      }
	}
	else
		bind_ok=1;
      }
    }  
    if (use_proxy > 0) {
      target.sin_port = htons(proxy_string_port);
      memcpy(&target.sin_addr.s_addr, &proxy_string_ip, 4);
    } else {
      target.sin_port = htons(port);
      memcpy(&target.sin_addr.s_addr, &host, 4);
    }
    target.sin_family = AF_INET;
    signal(SIGALRM, alarming);
    do {
      if (fail > 0)
        sleep(WAIT_BETWEEN_CONNECT_RETRY);
      alarm_went_off = 0;
      alarm(waittime);
      ret = connect(s, (struct sockaddr *) &target, sizeof(target));
      alarm(0);
      if (ret < 0 && alarm_went_off == 0) {
        fail++;
        if (verbose && fail <= MAX_CONNECT_RETRY)
          fprintf(stderr, "Process %d: Can not connect [unreachable], retrying (%d of %d retries)\n", (int) getpid(), fail, MAX_CONNECT_RETRY);
      }
    } while (ret < 0 && fail <= MAX_CONNECT_RETRY);
    if (ret < 0 && fail > MAX_CONNECT_RETRY) {
      if (debug)
        printf("DEBUG_CONNECT_UNREACHABLE\n");

/* we wont quit here, thats up to the module to decide what to do 
 *              fprintf(stderr, "Process %d: Can not connect [unreachable], process exiting\n", (int)getpid());
 *              hydra_child_exit(1);
 */
      extern_socket = -1;
      ret = -1;
      return ret;
    }
    ret = s;
    extern_socket = s;
    if (debug)
      printf("DEBUG_CONNECT_OK\n");

    if (use_proxy == 2) {
      buf = malloc(4096);
      memset(&target, 0, sizeof(target));
      memcpy(&target.sin_addr.s_addr, &host, 4);
      target.sin_family = AF_INET;
#ifdef CYGWIN
      if (proxy_authentication == NULL)
        snprintf(buf, 4096, "CONNECT %s:%d HTTP/1.0\r\n\r\n", inet_ntoa((struct in_addr) target.sin_addr), port);
      else
        snprintf(buf, 4096, "CONNECT %s:%d HTTP/1.0\r\nProxy-Authorization: Basic %s\r\n\r\n", inet_ntoa((struct in_addr) target.sin_addr), port, proxy_authentication);
#else
      if (proxy_authentication == NULL)
        snprintf(buf, 4096, "CONNECT %s:%d HTTP/1.0\r\n\r\n", inet_ntop(AF_INET, &target.sin_addr, out, sizeof(out)), port);
      else
        snprintf(buf, 4096, "CONNECT %s:%d HTTP/1.0\r\nProxy-Authorization: Basic %s\r\n\r\n", inet_ntop(AF_INET, &target.sin_addr, out, sizeof(out)), port,
                 proxy_authentication);
#endif
      send(s, buf, strlen(buf), 0);
      recv(s, buf, 4096, 0);
      if (strncmp("HTTP/", buf, strlen("HTTP/")) == 0 && (tmpptr = index(buf, ' ')) != NULL && *++tmpptr == '2') {
        if (debug)
          printf("DEBUG_CONNECT_SSL_OK\n");
      } else {
        if (debug)
          printf("DEBUG_CONNECT_SSL_FAILED (Code: %c%c%c)\n", *tmpptr, *(tmpptr + 1), *(tmpptr + 2));
        if (verbose)
          fprintf(stderr, "Error: CONNECT call to proxy failed with code %c%c%c\n", *tmpptr, *(tmpptr + 1), *(tmpptr + 2));
        close(s);
        extern_socket = -1;
        ret = -1;
        free(buf);
        return ret;
      }
      free(buf);
    }
    fail = 0;
    return ret;
  }
  return ret;
}

#ifdef LIBOPENSSL
RSA *
ssl_temp_rsa_cb(SSL * ssl, int export, int keylength)
{
  if (rsa == NULL)
    rsa = RSA_generate_key(512, RSA_F4, NULL, NULL);
  return rsa;
}

int
internal__hydra_connect_ssl(unsigned long int host, int port, int protocol, int type)
{
  int socket, err;

  /* XXX is this needed? */
  SSL_load_error_strings();
  SSLeay_add_ssl_algorithms();

  /* context: ssl2 + ssl3 is allowed, whatever the server demands */
  if ((sslContext = SSL_CTX_new(SSLv23_method())) == NULL) {
    if (verbose) {
      err = ERR_get_error();
      fprintf(stderr, "SSL: Error allocating context: %s\n", ERR_error_string(err, NULL));
    }
    return -1;
  }
  /* set the compatbility mode */
  SSL_CTX_set_options(sslContext, SSL_OP_ALL);

  /* we set the default verifiers and dont care for the results */
  (void) SSL_CTX_set_default_verify_paths(sslContext);
  SSL_CTX_set_tmp_rsa_callback(sslContext, ssl_temp_rsa_cb);
  SSL_CTX_set_verify(sslContext, SSL_VERIFY_NONE, NULL);

  if ((socket = internal__hydra_connect(host, port, protocol, type)) < 0)
    return -1;

  if ((ssl = SSL_new(sslContext)) == NULL) {
    if (verbose) {
      err = ERR_get_error();
      fprintf(stderr, "Error preparing an SSL context: %s\n", ERR_error_string(err, NULL));
    }
    return -1;
  }
  SSL_set_fd(ssl, socket);
  if (SSL_connect(ssl) <= 0) {
    printf("ERROR %d\n", SSL_connect(ssl));
    if (verbose) {
      err = ERR_get_error();
      fprintf(stderr, "Could not create an SSL session: %s\n", ERR_error_string(err, NULL));
    }
    return -1;
  }

  if (debug)
    fprintf(stderr, "SSL negotiated cipher: %s\n", SSL_get_cipher(ssl));

  use_ssl = 1;

  return socket;
}
#endif

int
internal__hydra_recv(int socket, char *buf, int length)
{
#ifdef LIBOPENSSL
  if (use_ssl) {
    return SSL_read(ssl, buf, length);
  } else
#endif
    return recv(socket, buf, length, 0);
}

int
internal__hydra_send(int socket, char *buf, int size, int options)
{
#ifdef LIBOPENSSL
  if (use_ssl) {
    return SSL_write(ssl, buf, size);
  } else
#endif
    return send(socket, buf, size, options);
}

/* ------------------ public functions ------------------ */

void
hydra_child_exit(int code)
{
  if (code == 0)                /* normal quitting */
    write(intern_socket, "Q", 1);
  if (code == 1)                /* no connect possible */
    write(intern_socket, "C", 1);
  if (code == 2)                /* application protocol error or service shutdown */
    write(intern_socket, "E", 1);
  exit(-1);
}

void
hydra_register_socket(int s)
{
  intern_socket = s;
}

char *
hydra_get_next_pair()
{
  if (pair[0] == 0) {
    read(intern_socket, pair, sizeof(pair));
    if (memcmp(&HYDRA_EXIT, &pair, sizeof(HYDRA_EXIT)) == 0)
      return HYDRA_EXIT;
    if (pair[0] == 0)
      return HYDRA_EMPTY;
  }
  return pair;
}

char *
hydra_get_next_login()
{
  if (pair[0] == 0)
    return HYDRA_EMPTY;
  return pair;
}

char *
hydra_get_next_password()
{
  char *ptr = pair;

  while (*ptr != '\0')
    ptr++;
  ptr++;
  if (*ptr == 0)
    return HYDRA_EMPTY;
  return ptr;
}

void
hydra_completed_pair()
{
  write(intern_socket, "N", 1);
  pair[0] = 0;
}

void
hydra_completed_pair_found()
{
  char *login;

  write(intern_socket, "F", 1);
  login = hydra_get_next_login();
  write(intern_socket, login, strlen(login) + 1);
  pair[0] = 0;
}

void
hydra_completed_pair_skip()
{
  char *login;

  write(intern_socket, "f", 1);
  login = hydra_get_next_login();
  write(intern_socket, login, strlen(login) + 1);
  pair[0] = 0;
}

void
hydra_report_found(int port, char *svc, FILE * fp)
{
  if (!strcmp(svc,"rsh"))
	fprintf(fp, "[%d][%s] login: %s\n", port, svc, hydra_get_next_login());  
  else
  	fprintf(fp, "[%d][%s] login: %s   password: %s\n", port, svc, hydra_get_next_login(), hydra_get_next_password());
  fflush(fp);
}

void
hydra_report_found_host(int port, unsigned int ip, char *svc, FILE * fp)
{
#ifdef CYGWIN
  struct in_addr *ipptr = (struct in_addr *) &ip;

  strcpy(ipaddr_str, inet_ntoa((struct in_addr) *ipptr));
#else
  inet_ntop(AF_INET, &ip, ipaddr_str, sizeof(ipaddr_str));
#endif
  if (!strcmp(svc,"rsh"))
	fprintf(fp, "[%d][%s] host: %s   login: %s\n", port, svc, ipaddr_str, hydra_get_next_login());
  else
  	fprintf(fp, "[%d][%s] host: %s   login: %s   password: %s\n", port, svc, ipaddr_str, hydra_get_next_login(), hydra_get_next_password());
  if (stdout != fp)
  {
   if (!strcmp(svc,"rsh"))
    	printf("[%d][%s] host: %s   login: %s\n", port, svc, ipaddr_str, hydra_get_next_login());
    else
        printf("[%d][%s] host: %s   login: %s   password: %s\n", port, svc, ipaddr_str, hydra_get_next_login(), hydra_get_next_password());
   } 
   fflush(fp); 
}

void
hydra_report_found_host_msg(int port, unsigned int ip, char *svc, FILE *fp, char *msg)
{
#ifdef CYGWIN
  struct in_addr *ipptr = (struct in_addr *) &ip;

  strcpy(ipaddr_str, inet_ntoa((struct in_addr) *ipptr));
#else
  inet_ntop(AF_INET, &ip, ipaddr_str, sizeof(ipaddr_str));
#endif
  if (!strcmp(svc,"rsh"))
  	fprintf(fp, "[%d][%s] host: %s   login: %s [%s]\n", port, svc, ipaddr_str, hydra_get_next_login(), msg);
  else
	fprintf(fp, "[%d][%s] host: %s   login: %s   password: %s  [%s]\n", port, svc, ipaddr_str, hydra_get_next_login(), hydra_get_next_password(), msg);  	

  if (stdout != fp)
  {
	if (!strcmp(svc,"rsh"))
		printf("[%d][%s] host: %s   login: %s\n", port, svc, ipaddr_str, hydra_get_next_login());
	else
    		printf("[%d][%s] host: %s   login: %s   password: %s\n", port, svc, ipaddr_str, hydra_get_next_login(), hydra_get_next_password());
  }   
  fflush(fp);
}

int
hydra_connect_ssl(unsigned long int host, int port)
{
#ifdef LIBOPENSSL
  return (internal__hydra_connect_ssl(host, port, SOCK_STREAM, 6));
#else
  return (internal__hydra_connect(host, port, SOCK_STREAM, 6));
#endif
}

int
hydra_connect_tcp(unsigned long int host, int port)
{
  return (internal__hydra_connect(host, port, SOCK_STREAM, 6));
}

int
hydra_connect_udp(unsigned long int host, int port)
{
  return (internal__hydra_connect(host, port, SOCK_DGRAM, 17));
}

int
hydra_disconnect(int socket)
{
  close(socket);
  if (debug)
    printf("DEBUG_DISCONNECT\n");
  return -1;
}

int
hydra_data_ready_writing_timed(int socket, long sec, long usec)
{
  fd_set fds;
  
  FD_ZERO(&fds);
  FD_SET(socket, &fds);
  return(my_select(socket+1,&fds,NULL,NULL,sec,usec));
}

int
hydra_data_ready_writing(int socket)
{
  return (hydra_data_ready_writing_timed(socket, 30, 0));
}

int
hydra_data_ready_timed(int socket, long sec, long usec)
{
  fd_set fds;

  FD_ZERO(&fds);
  FD_SET(socket, &fds);
  return(my_select(socket+1,&fds,NULL,NULL,sec,usec));
}

int
hydra_data_ready(int socket)
{
  return (hydra_data_ready_timed(socket, 0, 100));
}

int
hydra_recv(int socket, char *buf, int length)
{
  int ret;

  ret = internal__hydra_recv(socket, buf, length);
  if (debug)
    printf("DEBUG_RECV_BEGIN|%s|END\n", buf);
  return ret;
}

char *
hydra_receive_line(int socket)
{
  char buf[300], *buff;
  int i = 0, j = 0, k;

  buff = malloc(sizeof(buf));
  memset(buff, 0, sizeof(buf));

  i = hydra_data_ready_timed(socket, (long) waittime, 0);
  if (i > 0) {
    if ((i = internal__hydra_recv(socket, buff, sizeof(buf))) < 0) {
      free(buff);
      return NULL;
    }
  }
  if (i <= 0) {
    if (debug)
      printf("DEBUG_RECV_BEGIN|%s|END\n", buff);
    free(buff);
    return NULL;
  } else {
    for (k = 0; k < i; k++)
      if (buff[k] == 0)
        buff[k] = 32;
  }

  j = 1;
  while (hydra_data_ready(socket) > 0 && j > 0) {
    j = internal__hydra_recv(socket, buf, sizeof(buf));
    if (j > 0)
      for (k = 0; k < j; k++)
        if (buf[k] == 0)
          buf[k] = 32;
    buff = realloc(buff, i + j);
    memcpy(buff + i, &buf, j);
    i = i + j;
  }

  if (debug)
    printf("DEBUG_RECV_BEGIN|%s|END\n", buff);
  return buff;
}

int
hydra_send(int socket, char *buf, int size, int options)
{
  if (debug) {
    int k;
    char *debugbuf = malloc(size + 1);

    for (k = 0; k < size; k++)
      if (buf[k] == 0)
        debugbuf[k] = 32;
      else
        debugbuf[k] = buf[k];
    debugbuf[size] = 0;
    printf("DEBUG_SEND_BEGIN|%s|END\n", debugbuf);
    free(debugbuf);
  }

/*    if (hydra_data_ready_writing(socket)) < 1) return -1; XXX maybe needed in the future */
  return (internal__hydra_send(socket, buf, size, options));
}

int
make_to_lower(char *buf)
{
  if (buf == NULL)
    return 1;
  while (buf[0] != 0) {
    buf[0] = tolower(buf[0]);
    buf++;
  }
  return 1;
}

unsigned char
hydra_conv64(unsigned char in)
{
  if (in < 26)
    return (in + 'A');
  else if (in >= 26 && in < 52)
    return (in + 'a' - 26);
  else if (in >= 52 && in < 62)
    return (in + '0' - 52);
  else if (in == 62)
    return '+';
  else if (in == 63)
    return '/';
  else {
    fprintf(stderr, "Too high for base64: %d\n", in);
    return 0;
  }
}

void
hydra_tobase64(unsigned char *buf, int buflen, int bufsize)
{
  unsigned char small[3] = { 0, 0, 0 };
  unsigned char big[5];
  unsigned char *ptr = buf;
  int i = bufsize;
  unsigned int len = 0;
  unsigned char bof[i];

  if (buf == NULL || strlen((char *) buf) == 0)
    return;
  bof[0] = 0;
  memset(big, 0, sizeof(big));
  memset(bof, 0, bufsize);

  for (i = 0; i < buflen / 3; i++) {
    memset(big, 0, sizeof(big));
    big[0] = hydra_conv64(*ptr >> 2);
    big[1] = hydra_conv64(((*ptr & 3) << 4) + (*(ptr + 1) >> 4));
    big[2] = hydra_conv64(((*(ptr + 1) & 15) << 2) + (*(ptr + 2) >> 6));
    big[3] = hydra_conv64(*(ptr + 2) & 63);
    len += strlen((char*)big);
    if (len > bufsize) {
      buf[0] = 0;
      return;
    }
    strcat((char *) bof, (char *) big);
    ptr += 3;
  }

  if (*ptr != 0) {
    small[0] = *ptr;
    if (*(ptr + 1) != 0)
      small[1] = *(ptr + 1);
    else
      small[1] = 0;
    ptr = small;
    big[0] = hydra_conv64(*ptr >> 2);
    big[1] = hydra_conv64(((*ptr & 3) << 4) + (*(ptr + 1) >> 4));
    big[2] = hydra_conv64(((*(ptr + 1) & 15) << 2) + (*(ptr + 2) >> 6));
    big[3] = hydra_conv64(*(ptr + 2) & 63);
    if (small[1] == 0)
      big[2] = '=';
    big[3] = '=';
    strcat((char *) bof, (char *) big);
  }

  strcpy((char *) buf, (char *) bof); /* can not overflow */
}

void
hydra_dump_asciihex(unsigned char *string, int length)
{
    unsigned char *p = (unsigned char *) string;
    unsigned char lastrow_data[16];
    int rows = length / HYDRA_DUMP_ROWS;
    int lastrow = length % HYDRA_DUMP_ROWS;
    int i, j;

    for (i = 0; i < rows; i++) {
        printf("%04hx:  ", i * 16);
        for (j = 0; j < HYDRA_DUMP_ROWS; j++) {
            printf("%02x", p[(i * 16) + j]);
            if (j % 2 == 1)
                printf(" ");
        }
        printf("   [ ");
        for (j = 0; j < HYDRA_DUMP_ROWS; j++) {
            if (isprint(p[(i * 16) + j]))
                printf("%c", p[(i * 16) + j]);
            else
                printf(".");
        }
        printf(" ]\n");
    }
    if (lastrow > 0) {
        memset(lastrow_data, 0, sizeof(lastrow_data));
        memcpy(lastrow_data, p + length - lastrow, lastrow);
        printf("%04hx:  ", i * 16);
        for (j = 0; j < lastrow; j++) {
            printf("%02x", p[(i * 16) + j]);
            if (j % 2 == 1)
                printf(" ");
        }
        while(j < HYDRA_DUMP_ROWS) {
            printf("  ");
            if (j % 2 == 1)
                printf(" ");
            j++;
        }
        printf("   [ ");
        for (j = 0; j < lastrow; j++) {
            if (isprint(p[(i * 16) + j]))
                printf("%c", p[(i * 16) + j]);
            else
                printf(".");
        }
        while(j < HYDRA_DUMP_ROWS) {
            printf(" ");
            j++;
        }
        printf(" ]\n");
    }
}

void
hydra_set_srcport(int port)
{
	src_port=port;
}
