/*
 *	PostgresSQL Support - by Diaul (at) devilopers.org 
 *
 *	SSL support and password encryption not (yet) supported
 *	
 */

#include "hydra-mod.h"

#ifndef LIBPOSTGRES
void
dummy_postgres()
{
  printf("\n");
}
#else

#include "libpq-fe.h" // Postgres connection functions
#include <stdio.h>

#define DEFAULT_DB "template1"

extern char *HYDRA_EXIT;

int
start_postgres(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp)
{
    char *empty = "";
    char *login, *pass;
    char database[256];
    char connection_string[1024];
    struct in_addr remote_ip;

    if(miscptr)
        strncpy(database,miscptr,sizeof(database));
    else
        strncpy(database,DEFAULT_DB,sizeof(database));
        
    remote_ip.s_addr = ip;
    
    
	if (strlen(login = hydra_get_next_login()) == 0)
		login = empty;
	if (strlen(pass = hydra_get_next_password()) == 0)
		pass = empty;

	/*
   	 *	Building the connection string
	 */

    
    snprintf(connection_string, sizeof(connection_string), "host = '%s' dbname = '%s' user = '%s' password = '%s' ", inet_ntoa(remote_ip), database, login, pass);


    // printf("%s\n",connection_string);
    
	if (PQstatus(PQconnectdb(connection_string)) == CONNECTION_OK)
	{
		hydra_report_found_host(port, ip, "postgres", fp);
		hydra_completed_pair();
		if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
			return 3;
		return 2;
	}
    else
    {

        hydra_completed_pair();
        if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
            return 2;            
    }
    return 1;
}

void
service_postgres(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE *fp, int port)
{
	int run = 1, next_run, sock = -1;
	int myport = PORT_POSTGRES, mysslport = PORT_POSTGRES_SSL;

	hydra_register_socket(sp);
	if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
		return;
	
	while (1)
	{

		switch (run)
		{
			case 1:                    /* connect and service init function */
				if (sock >= 0)
					sock = hydra_disconnect(sock);
//				usleep(275000);
				if ((options & OPTION_SSL) == 0) 
				{
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
				if (sock < 0)
				{
					fprintf(stderr, "Error: Child with pid %d terminating, can not connect\n", (int) getpid());
					hydra_child_exit(1);
				}
				
				next_run = 2;
				break;

			case 2:
      
      				/*
      				 *	Here we start the password cracking process  
				     */
                
				next_run = start_postgres(sock, ip, port, options, miscptr, fp);
				break;
			case 3:
				
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
}

#endif
