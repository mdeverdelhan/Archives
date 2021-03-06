store = 0;
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
                                                                                                                                 );
    hydra_completed_pair_found();
  } else {
    if (*ptr != '4')
      printf("Unusual return code: %c for %s:%s\n", (char) *(index(buf, ' ') + 1), login, pass);
    hydra_completed_pair();
  }

  free(buf);

  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return 3;
  return 1;

}

void
service_http_proxy(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  int run = 1, next_run, sock = -1;
  int myport = PORT_HTTP_PROXY, mysslport =store = 0;
    unlink(RESTOREFILE);
    for (i = 0; i < hydra_options.tasks; i++)
      if (hydra_arms[i]->active > 0)
        write(hydra_arms[i]->sp[0], HYDRA_EXIT, sizeof(HYDRA_EXIT));
    error = j = 0;
    while (error < MAXENDWAIT && j < hydra_options.tasks) {      /* har