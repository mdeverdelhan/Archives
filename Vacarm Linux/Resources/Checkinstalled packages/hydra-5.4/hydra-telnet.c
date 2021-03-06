f
      }
      if (buf == NULL) {
        hydra_report(stderr, "No response from server, exiting...\n");
        next_run = 3;
      }
#ifdef PALM
      if (buf != NULL && (StrCompare(buf, "RFB.003.003") != 0)) { /* check the first line */
#else
      if (buf != NULL && (strcmp(buf, "RFB.003.003") != 0)) { /* check the first line */
#endif
        hydra_report(stderr, "Warning: protocol %s is not verified to work. Please report if not.\n", buf);
        free(buf);
        buf = strdup("RFC.003.003");
      }
      if (buf != NULL) {
        hydra_send(sock, buf, strlen(buf), 0);
        free(buf);
        next_run = 2;
      }
      break;
    case 2:                    /* run the cracking function */
      next_run = start_vnc(sock, ip, port, options, miscptr, fp);
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
#ifdef PALM
        return;
#else
        exit(-1);
#endif
    }
    run = next_run;
  }
}
                                                                                                                                                                                                                                                                                           ith pid %d terminating, can not connect\n", (int) getpid());
        hydra_child_exit(1);
      }
      buf = hydra_receive_line(sock);
#ifdef PALM
      if (buf == NULL || (StrNCompare(buf, "RFB", 3) != 0)) { /* check the first line */
#else
      if (buf == NULL || (strncmp(buf, "RFB", 3) != 0)) { /* check the first line */
#endif
        hydra_report(stderr, "Error: Not an VNC protocol or service shutdown: %s\n", buf);
        hydra_child_exit(2);
#ifdef PALM
        return;
#else
        exit(-1);
#endif
      }
      if (buf == NULL) {
        hydra_report(stderr, "No response from server, exiting...\n");
        next_run = 3;
      }
#ifdef PALM
      if (buf != NULL && (StrCompare(buf, "RFB.003.003") != 0)) { /* check the first line */
#else
      if (buf != NULL && (strcmp(buf, "RFB.003.003") != 0)) { /* check the first line */
#endif
        hydra_report(stderr, "Warning: protocol %s is not verified to work. Please report if not.\n", buf);
        free(buf);
        buf = strdup("RFC.003.003");
      }
      if (buf != NULL) {
        hydra_send(sock, buf, strlen(buf), 0);
        free(buf);
        next_run = 2;
      }
      break;
    case 2:                    /* run the cracking function */
      next_run = start_vnc(sock, ip, port, options, miscptr, fp);
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
#ifdef PALM
        return;
#else
        exit(-1);
#endif
    }
    run = next_run;
  }
}
                                                                                                                                                                                                                                                                                           ith pid %d terminating, can not connect\n", (int) getpid());
        hydra_child_exit(1);
      }
      buf = hydra_receive_line(sock);
#ifdef PALM
      if (buf == NULL || (StrNCompare(buf, "RFB", 3) != 0)) { /* check the first line */
#else
      if (buf == NULL || (strncmp(buf, "RFB", 3) != 0)) { /* check the first line */
#endif
        hydra_report(stderr, "Error: Not an VNC protocol or service shutdown: %s\n", buf);
        hydra_child_exit(2);
#ifdef PALM
        return;
#else
        exit(-1);
#endif
      }
      if (buf == NULL) {
        hydra_report(stderr, "No response from server, exiting...\n");
        next_run = 3;
      }
#ifdef PALM
      if (buf != NULL && (StrCompare(buf, "RFB.003.003") != 0)) { /* check the first line */
#else
      if (buf != NULL && (strcmp(buf, "RFB.003.003") != 0)) { /* check the first line */
#endif
        hydra_report(stderr, "Warning: protocol %s is not verified to work. Please report if not.\n", buf);
        free(buf);
        buf = strdup("RFC.003.003");
      }
      if (buf != NULL) {
        hydra_send(sock, buf, strlen(buf), 0);
        free(buf);
        next_run = 2;
      }
      break;
    case 2:                    /* run the cracking function */
      next_run = start_vnc(sock, ip, port, options, miscptr, fp);
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
#ifdef PALM
        return;
#else
        exit(-1);
#endif
    }
    run = next_run;
  }
}
                                                                                                                                                                                                                                                                                           ith pid %d terminating, can not connect\n", (int) getpid());
        hydra_child_exit(1);
      }
      buf = hydra_receive_line(sock);
#ifdef PALM
      if (buf == NULL || (StrNCompare(buf, "RFB", 3) != 0)) { /* check the first line */
#else
      if (buf == NULL || (strncmp(buf, "RFB", 3) != 0)) { /* check the first line */
#endif
        hydra_report(stderr, "Error: Not an VNC protocol or service shutdown: %s\n", buf);
        hydra_child_exit(2);
#ifdef PALM
        return;
#else
        exit(-1);
#endif
      }
      if (buf == NULL) {
        hydra_report(stderr, "No response from server, exiting...\n");
        next_run = 3;
      }
#ifdef PALM
      if (buf != NULL && (StrCompare(buf, "RFB.003.003") != 0)) { /* check the first line */
#else
      if (buf != NULL && (strcmp(buf, "RFB.003.003") != 0)) { /* check the first line */
#endif
        hydra_report(stderr, "Warning: protocol %s is not verified to work. Please report if not.\n", buf);
        free(buf);
        buf = strdup("RFC.003.003");
      }
      if (buf != NULL) {
        hydra_send(sock, buf, strlen(buf), 0);
        free(buf);
        next_run = 2;
      }
      break;
    case 2:                    /* run the cracking function */
      next_run = start_vnc(sock, ip, port, options, miscptr, fp);
      break;
  