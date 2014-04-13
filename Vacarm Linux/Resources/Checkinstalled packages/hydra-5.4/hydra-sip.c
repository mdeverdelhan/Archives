// simple sip digest auth (md5) module
// written by gh0st 2005

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/md5.h>

#include "hydra-mod.h"

extern int hydra_data_ready_timed(int socket, long sec, long usec);

extern char *HYDRA_EXIT;

#define SIP_MAX_BUF	1024

typedef struct {
    char nonce[SIP_MAX_BUF];
    int cseq;
    char method[SIP_MAX_BUF];
    char realm[SIP_MAX_BUF];
} sip_digest_cred;

void md5_crypt(char *inbuffer, char *outhexstr) {
    unsigned char md5_raw[MD5_DIGEST_LENGTH], b=0x0;
    int i=0;
    MD5_CTX c;
    char *hex = "0123456789abcdef";

    memset(outhexstr,0,MD5_DIGEST_LENGTH*2+1);

    MD5_Init(&c);
    MD5_Update(&c, inbuffer, strlen(inbuffer));
    MD5_Final(md5_raw, &c);

    for(i = 0; i< MD5_DIGEST_LENGTH; i++) {
        b = (md5_raw[i] & 0xF0) >> 4;
        b = b & 0x0F;
        outhexstr[i+i]   = hex[(int)b];
        outhexstr[i+i+1] = hex[md5_raw[i] & 0x0F];
    }
    outhexstr[i+i] = 0;
}

void empty_register(char *buf, char *host, int port,char *user) {
    memset(buf,0, SIP_MAX_BUF);
    snprintf(buf, SIP_MAX_BUF,
             "REGISTER sip:%s SIP/2.0\n" \
             "Via: SIP/2.0/UDP %s:%i\n" \
             "From: <sip:%s@%s>\n" \
             "To: <sip:%s@%s>\n" \
             "Content-Length: 0\n\n",
             host, host, port,
             user,host,
             user,host
            );
}

void sip_register(char *buf, char *host, int port,char *user, char *pass,
                  sip_digest_cred sdc) {
    char temp[SIP_MAX_BUF], urp[MD5_DIGEST_LENGTH*2+1],
    mu[MD5_DIGEST_LENGTH*2+1], resp[MD5_DIGEST_LENGTH*2+1];

    snprintf(temp,SIP_MAX_BUF,"%s:%s:%s", user,sdc.realm, pass);
    md5_crypt(temp, urp);
    snprintf(temp,SIP_MAX_BUF,"%s:sip:%s", sdc.method, host);
    md5_crypt(temp, mu);
    snprintf(temp,SIP_MAX_BUF,"%s:%s:%s", urp,sdc.nonce, mu);
    md5_crypt(temp, resp);

    memset(buf,0, SIP_MAX_BUF);
    snprintf(buf, SIP_MAX_BUF,
             "REGISTER sip:%s SIP/2.0\n" \
             "Via: SIP/2.0/UDP %s:%i\n" \
             "From: <sip:%s@%s>\n" \
             "To: <sip:%s@%s>\n" \
             "Call-ID: 1337@%s\n" \
             "CSeq: %i %s\n" \
             "Authorization: Digest username=\"%s\", " \
             " realm=\"%s\", nonce=\"%s\", uri=\"sip:%s\"," \
             " response=\"%s\"\n" \
             "Content-Length: 0\n\n",
             host, host, port,
             user,host,
             user,host,
             host,
             sdc.cseq, sdc.method,
             user, sdc.realm, sdc.nonce, host,
             resp
            );
}

int get_sip_code(char *buf) {
    int code;
    char tmpbuf[SIP_MAX_BUF], word[SIP_MAX_BUF];
    if(sscanf(buf, "%s %i %s", tmpbuf, &code, word) != 3)
        return -1;
    return code;
}

int extract_sip_cred(char *buf, sip_digest_cred *sdc) {
    char *line;
    char *word;
    char *end;
    int len=0;

    while((line = strsep(&buf, "\n")) != NULL) {
        if(strncasecmp(line, "WWW-Authenticate:",17) == 0) {
            if((word = strstr(line, "realm=")) != NULL) {
                end = strchr(word, ',');
                if(end == NULL)
                    end=strchr(word, '\r');
                if(end == NULL)
                    end=strchr(word, '\n');
                word = word+6;
                word++; // remove leading "
                end--; // remove trailing "
                len=end-word;
                strncpy(sdc->realm, word, len);
                *(sdc->realm+len)=0;
            } else {
                return -1;
            }
            if((word = strstr(line, "nonce=")) != NULL) {
                end = strchr(word, ',');
                if(end == NULL)
                    end=strchr(word, '\r');
                if(end == NULL)
                    end=strchr(word, '\n');

                word = word+6;
                word++; // remove leading "
                end--; // remove trailing "
                len=end-word;
                strncpy(sdc->nonce, word, len);
                *(sdc->nonce+len)=0;
            } else {
                return -1;
            }
            return 1;
        }
    }
    return -1;
}

int start_sip(int s, unsigned long int ip, int port, unsigned char options,
          char *miscptr, FILE * fp) {
    char *login, *pass, *host, buffer[SIP_MAX_BUF];
    int i;
    char buf[SIP_MAX_BUF];
    sip_digest_cred sdc;

    if (strlen(login = hydra_get_next_login()) == 0)
        login = NULL;
    if (strlen(pass = hydra_get_next_password()) == 0)
        pass = NULL;

    host = miscptr;

    empty_register(buffer, host, port, login);


    if (hydra_send(s, buffer, strlen(buffer), 0) < 0) {
        return 3;
    }

    if (hydra_data_ready_timed(s, 5, 0) > 0) {
        i = hydra_recv(s, (char *) buf, sizeof(buf));
        if(extract_sip_cred(buf, &sdc) < 0) {
            hydra_report(stderr,"fatal fuck up while parsing sip reply!\n");
            return -1;
        }
        sdc.cseq=1;
        strncpy(sdc.method, "REGISTER", SIP_MAX_BUF);
        sip_register(buffer,host, port, login, pass,sdc);
        if (hydra_send(s, buffer, strlen(buffer), 0) < 0) {
            return 3;
        }
        if (hydra_data_ready_timed(s, 5, 0) > 0) {
            memset(buf, 0, sizeof(buf));
            i = hydra_recv(s, (char *) buf, sizeof(buf));
            if(get_sip_code(buf) == 200) {
                hydra_report_found_host(port, ip, "sip", fp);
                hydra_completed_pair_found();
            }
        }

    }
    hydra_completed_pair();
    if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
        return 3;

    return 1;
}

void
service_sip(unsigned long int ip, int sp, unsigned char options,
            char *miscptr, FILE * fp, int port)
{
    int run = 1, next_run, sock = -1;
    int myport = PORT_SIP;

    hydra_register_socket(sp);

    if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
        run = 3;

    while (1) {
        switch (run) {
        case 1:
            if (sock < 0) {
                if (port != 0)
                    myport = port;
                sock = hydra_connect_udp(ip, myport);
                port = myport;
                if (sock < 0) {
                    hydra_report(stderr,
                                 "Error: Child with pid %d terminating, can not connect\n",
                                 (int) getpid());
                    hydra_child_exit(1);
                }
            }
            next_run = start_sip(sock, ip, port, options, miscptr, fp);
            break;
        case 3:
            if (sock >= 0)
                sock = hydra_disconnect(sock);
            hydra_child_exit(-1);
            return;
        default:
            hydra_report(stderr, "Caught unknown return code, exiting!\n");
            hydra_child_exit(-1);
            exit(-1);
        }
        run = next_run;
    }
}
