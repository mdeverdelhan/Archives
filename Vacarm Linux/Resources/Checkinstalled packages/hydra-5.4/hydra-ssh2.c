#include "hydra-mod.h"
#ifndef LIBSSH
void
dummy_ssh()
{
  printf("\n");
}
#else

#warning "If compilation of hydra-ssh2 fails, you are not using v0.11. Download from http://www.0xbadc0de.be/"

#include <libssh/libssh.h>

extern char *HYDRA_EXIT;

int
start_ssh2(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp)
{
  char *empty = "";
  char *login, *pass;
  char *buf;
  char *rc;
  struct sockaddr_in targetip;
  SSH_SESSION *ssh_session;
  SSH_OPTIONS *ssh_opt;
  int auth_state;
  int i = 0;

  if (strlen(login = hydra_get_next_login()) == 0)
    login = empty;
  if (strlen(pass = hydra_get_next_password()) == 0)
    pass = empty;

  ssh_opt=options_new();
  memset(&targetip, 0, sizeof(targetip));
  memcpy(&targetip.sin_addr.s_addr, &ip, 4);
  targetip.sin_family = AF_INET;
#ifdef CYGWIN
  buf = inet_ntoa((struct in_addr) targetip.sin_addr);
#else
  buf = malloc(20);
  inet_ntop(AF_INET, &targetip.sin_addr, buf, 20);
#endif
  options_set_wanted_method(ssh_opt,KEX_COMP_C_S,"none");
  options_set_wanted_method(ssh_opt,KEX_COMP_S_C,"none");
  options_set_port(ssh_opt, port);
  options_set_host(ssh_opt, buf);
  options_set_username(ssh_opt, login);

  if ((ssh_session = ssh_connect(ssh_opt)) == NULL) {
    rc = ssh_get_error(ssh_session);
    if ((rc != NULL) && (rc[0] != '\0')) {
      if (strncmp("connect:", ssh_get_error(ssh_session), strlen("connect:")) == 0)
        return 3;
      else
        return 4;
    }
  }
#ifndef CYGWIN
  free(buf);
  buf = NULL;
#endif

  do {
    /* why this crap? */
    auth_state = ssh_userauth_kbdint(ssh_session, login, NULL);
    while (i < 10 && auth_state == SSH_AUTH_INFO) {
      ssh_userauth_kbdint_setanswer(ssh_session, i, pass);
      auth_state = ssh_userauth_kbdint(ssh_session, login, NULL);
      i++;
    }
    
    if (auth_state == SSH_AUTH_SUCCESS || ssh_userauth_password(ssh_session, login, pass) == SSH_AUTH_SUCCESS) {
      ssh_disconnect(ssh_session);	/* this automagically frees the ssh_opt buffer */
      hydra_report_found_host(port, ip, "ssh2", fp);
      hydra_completed_pair_found();
      if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
        return 2;
      /* free(ssh_opt); */ /* DOUBLE FREE ! */
      return 1;
    } else {
      if (ssh_error_code(ssh_session) == 1) {
        hydra_completed_pair();
        if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
          return 2;
      } else {
        ssh_disconnect(ssh_session);	/* this automagically frees the ssh_opt buffer */
        hydra_completed_pair(); /* really? */
        if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
          return 2;
        /* free(ssh_opt); */ /* DOUBLE FREE ! */
        return 1;
      }
    }
  } while(1);

  /* not reached */

  /* free(ssh_opt); */ /* risk of double free */
  return 1;
}

void
service_ssh2(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  int run = 1, next_run = 1, sock = -1;

  hydra_register_socket(sp);
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return;
  while (1) {
    switch (run) {
    case 1:                    /* connect and service init function */
      next_run = start_ssh2(sock, ip, port, options, miscptr, fp);
      break;
    case 2:
      hydra_child_exit(0);
    case 3:                    /* clean exit */
      fprintf(stderr, "Error: could not connect to target port %d\n", port);
      hydra_child_exit(1);
    case 4:
      fprintf(stderr, "Error: ssh2 protocol error\n");
      hydra_child_exit(2);
    default:
      hydra_report(stderr, "Caught unknown return code, exiting!\n");
      hydra_child_exit(-1);
    }
    run = next_run;
  }
}

#endif
