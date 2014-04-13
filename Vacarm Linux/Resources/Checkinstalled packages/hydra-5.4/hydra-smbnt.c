#include "hydra-mod.h"
#ifndef LIBOPENSSL
void
dummy_smbnt()
{
  printf("\n");
}
#else
#include <openssl/md4.h>
#include <openssl/des.h>

/*
   SMB NTLM Password/HASH Checking Hydra Module
   Version: $Id: hydra-smbnt.c 67 2005-03-08 20:14:40Z jmk $
   
   ------------------------------------------------------------------------
    Copyright (C) 2005 Joseph N. Mondloch
    JoMo-Kun / jmk@foofus.net 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2,
    as published by the Free Software Foundation
   
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
   
    http://www.gnu.org/licenses/gpl.txt
   ------------------------------------------------------------------------

   Based on code from: SMB Auditing Tool
   [Copyright (C) Patrik Karlsson 2001]

   This code allows Hydra to directly test NTLM hashes against
   a Windows. This may be useful for an auditor who has aquired 
   a sam._ or pwdump file and would like to quickly determine 
   which are valid entries. This module can also be used to test 
   SMB passwords against devices that do not allow clear text 
   LanMan passwords.

   The "-m 'METHOD'" option is required for this module. The
   following are valid methods: L, LH, D, DH, B, BH and M
   (in quotes).

     L == Check local account.
     D == Check credentials against this hosts primary
          domain controller via this host.
     B == Check both. This leaves the workgroup field set
          blank and then attempts to check the credentials
          against the host. If the account does not exist
          locally on the host being tested, that host then 
          queries its domain controller.
     H == Use a NTLM hash rather than a password. 
     M == Use the Machine's NetBIOS name as the password. 

   Be careful of mass domain account lockout with this. For
   example, assume you are checking several accounts against 
   many domain workstations. If you are not using the 'L'
   options and these accounts do not exist locally on the 
   workstations, each workstation will in turn check their
   respective domain controller. This could cause a bunch of 
   lockouts. Of course, it'd look like the workstations, not 
   you, were doing it. ;)

   **FYI, this code is unable to test accounts on default XP
   hosts which are not part of a domain and do not have normal
   file sharing enabled. Default XP does not allow shares and
   returns STATUS_LOGON_FAILED for both valid and invalid 
   credentials. XP with simple sharing enabled returns SUCCESS
   for both valid and invalid credentials. If anyone knows a
   way to test in these configurations...

   See http://www.foofus.net/jmk/passhash.html for further
   examples.

   Greets: Foofus, Phenfen & Fizzgig  
*/

#define WIN2000_NATIVEMODE 1
#define WIN_NETBIOSMODE 2

extern char *HYDRA_EXIT;
char challenge[8];
unsigned char workgroup[16];
unsigned char machine_name[16];
int hashFlag, accntFlag, protoFlag;

static unsigned char
Get7Bits(unsigned char *input, int startBit)
{
  register unsigned int word;

  word = (unsigned) input[startBit / 8] << 8;
  word |= (unsigned) input[startBit / 8 + 1];

  word >>= 15 - (startBit % 8 + 7);

  return word & 0xFE;
}

/* Make the key */
static void
MakeKey(unsigned char *key, unsigned char *des_key)
{
  des_key[0] = Get7Bits(key, 0);
  des_key[1] = Get7Bits(key, 7);
  des_key[2] = Get7Bits(key, 14);
  des_key[3] = Get7Bits(key, 21);
  des_key[4] = Get7Bits(key, 28);
  des_key[5] = Get7Bits(key, 35);
  des_key[6] = Get7Bits(key, 42);
  des_key[7] = Get7Bits(key, 49);

  des_set_odd_parity((des_cblock *) des_key);
}

/* Do the DesEncryption */
void
DesEncrypt(unsigned char *clear, unsigned char *key, unsigned char *cipher)
{
  des_cblock des_key;
  des_key_schedule key_schedule;

  MakeKey(key, des_key);
  des_set_key(&des_key, key_schedule);
  des_ecb_encrypt((des_cblock *) clear, (des_cblock *) cipher, key_schedule, 1);
}

/*
  HashNTLM
  Function: Create a NTLM hash from the challenge
  Variables:
        ntlmhash  = the hash created from this function
        pass      = users password
        challenge = the challenge recieved from the server
*/
void
HashNTLM(unsigned char **ntlmhash, unsigned char *pass, unsigned char *challenge, char *miscptr)
{
  MD4_CTX md4Context;
  unsigned char hash[16];       /* MD4_SIGNATURE_SIZE = 16 */
  unsigned char unicodePassword[256 * 2];       /* MAX_NT_PASSWORD = 256 */
  unsigned char p21[21];
  unsigned char ntlm_response[24];
  int i = 0, j = 0;
  int mdlen;
  unsigned char *p;
  char HexChar;
  int HexValue;

  /* Use NTLM Hash instead of password */
  if (hashFlag == 1) {
    /* 1000:D42E35E1A1E4C22BD32E2170E4857C20:5E20780DD45857A68402938C7629D3B2::: */
    p = pass;
    while ((*p != '\0') && (i < 2)) {
      if (*p == ':')
        i++;
      p++;
    }

    if (*p == '\0') {
      fprintf(stderr, "Error reading PWDUMP file.\n");
      hydra_child_exit(0);
      exit(1);
    }

    for (i = 0; i < 16; i++) {
      HexValue = 0x0;
      for (j = 0; j < 2; j++) {
        HexChar = (char) p[2 * i + j];

        if (HexChar > 0x39)
          HexChar = HexChar | 0x20;     /* convert upper case to lower */

        if (!(((HexChar >= 0x30) && (HexChar <= 0x39)) ||       /* 0 - 9 */
              ((HexChar >= 0x61) && (HexChar <= 0x66)))) {      /* a - f */
          /*
           *  fprintf(stderr, "Error invalid char (%c) for hash.\n", HexChar);
           *  hydra_child_exit(0);
           *  exit(1);
           */
          HexChar = 0x30;
        }

        HexChar -= 0x30;
        if (HexChar > 0x09)     /* HexChar is "a" - "f" */
          HexChar -= 0x27;

        HexValue = (HexValue << 4) | (char) HexChar;
      }
      hash[i] = (unsigned char) HexValue;
    }
  } else {
    /* Password == Machine Name */
    if (hashFlag == 2) {
      for (i = 0; i < 16; i++) {
        if (machine_name[i] > 0x39)
          machine_name[i] = machine_name[i] | 0x20;     /* convert upper case to lower */
        pass = machine_name;
      }
    }
   
    /* Initialize the Unicode version of the secret (== password). */
    /* This implicitly supports 8-bit ISO8859/1 characters. */
    bzero(unicodePassword, sizeof(unicodePassword));
    for (i = 0; i < strlen((char *) pass); i++)
      unicodePassword[i * 2] = (unsigned char) pass[i];

    mdlen = strlen((char *) pass) * 2;    /* length in bytes */
    MD4_Init(&md4Context);
    MD4_Update(&md4Context, unicodePassword, mdlen);
    MD4_Final(hash, &md4Context);        /* Tell MD4 we're done */
  }

  memset(p21, '\0', 21);
  memcpy(p21, hash, 16);

  DesEncrypt(challenge, p21 + 0, ntlm_response + 0);
  DesEncrypt(challenge, p21 + 7, ntlm_response + 8);
  DesEncrypt(challenge, p21 + 14, ntlm_response + 16);

  memcpy(*ntlmhash, ntlm_response, 24);
}

/*
   NBS Session Request
   Function: Request a new session from the server
   Returns: TRUE on success else FALSE.
*/
int
NBSSessionRequest(int s)
{
  char nb_name[32];             /* netbiosname */
  char nb_local[32];            /* netbios localredirector */
  unsigned char rqbuf[7] = { 0x81, 0x00, 0x00, 0x48, 0x20, 0x00, 0x20 };
  char *buf;
  unsigned char rbuf[400];

  /* if we are running in native mode (aka port 445) don't do netbios */
  if (protoFlag == WIN2000_NATIVEMODE)
    return 0;
  
  /* convert computer name to netbios name */
  memset(nb_name, 0, 32);
  memset(nb_local, 0, 32);
  memcpy(nb_name, "CKFDENECFDEFFCFGEFFCCACACACACACA", 32);      /* *SMBSERVER */
  memcpy(nb_local, "EIFJEEFCEBCACACACACACACACACACACA", 32);     /* HYDRA */

  buf = (char *) malloc(100);
  memset(buf, 0, 100);
  memcpy(buf, (char *) rqbuf, 5);
  memcpy(buf + 5, nb_name, 32);
  memcpy(buf + 37, (char *) rqbuf + 5, 2);
  memcpy(buf + 39, nb_local, 32);
  memcpy(buf + 71, (char *) rqbuf + 5, 1);

  hydra_send(s, buf, 76, 0);
  free(buf);

  memset(rbuf, 0, 400);
  hydra_recv(s, (char *) rbuf, sizeof(rbuf));

  if (rbuf[0] == 0x82)
    return 0;                   /* success */
  else
    return -1;                  /* failed */
}


/*
   SMBNegProt
   Function: Negotiate protocol with server ...
       Actually a pseudo negotiation since the whole
       program counts on NTLM support :)

    The challenge is retrieved from the answer
    No error checking is performed i.e cross your fingers....
*/
int
SMBNegProt(int s)
{
  unsigned char buf[168] = {
    0x00, 0x00, 0x00, 0xa4, 0xff, 0x53, 0x4d, 0x42,
    0x72, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x7d,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x81, 0x00, 0x02,
    0x50, 0x43, 0x20, 0x4e, 0x45, 0x54, 0x57, 0x4f,
    0x52, 0x4b, 0x20, 0x50, 0x52, 0x4f, 0x47, 0x52,
    0x41, 0x4d, 0x20, 0x31, 0x2e, 0x30, 0x00, 0x02,
    0x4d, 0x49, 0x43, 0x52, 0x4f, 0x53, 0x4f, 0x46,
    0x54, 0x20, 0x4e, 0x45, 0x54, 0x57, 0x4f, 0x52,
    0x4b, 0x53, 0x20, 0x31, 0x2e, 0x30, 0x33, 0x00,
    0x02, 0x4d, 0x49, 0x43, 0x52, 0x4f, 0x53, 0x4f,
    0x46, 0x54, 0x20, 0x4e, 0x45, 0x54, 0x57, 0x4f,
    0x52, 0x4b, 0x53, 0x20, 0x33, 0x2e, 0x30, 0x00,
    0x02, 0x4c, 0x41, 0x4e, 0x4d, 0x41, 0x4e, 0x31,
    0x2e, 0x30, 0x00, 0x02, 0x4c, 0x4d, 0x31, 0x2e,
    0x32, 0x58, 0x30, 0x30, 0x32, 0x00, 0x02, 0x53,
    0x61, 0x6d, 0x62, 0x61, 0x00, 0x02, 0x4e, 0x54,
    0x20, 0x4c, 0x41, 0x4e, 0x4d, 0x41, 0x4e, 0x20,
    0x31, 0x2e, 0x30, 0x00, 0x02, 0x4e, 0x54, 0x20,
    0x4c, 0x4d, 0x20, 0x30, 0x2e, 0x31, 0x32, 0x00
  };

  unsigned char rbuf[400];
  unsigned char sess_key[2];
  unsigned char userid[2] = {0xCD, 0xEF};
  int i = 0, j = 0;

  memset((char *) rbuf, 0, 400);

  /* set session key */
  sess_key[1] = getpid() / 100;
  sess_key[0] = getpid() - (100 * sess_key[1]);
  memcpy(buf + 30, sess_key, 2);
  memcpy(buf + 32, userid, 2);
  
  hydra_send(s, (char *) buf, 168, 0);
  hydra_recv(s, (char *) rbuf, sizeof(rbuf));

  /* retrieve the challenge */
  memcpy(challenge, (char *) rbuf + 73, sizeof(challenge));

  /* find the primary domain/workgroup name */
  memset(workgroup, 0, 16);
  memset(machine_name, 0, 16);

  while ((rbuf[81 + i * 2] != 0) && (i < 16)) {
    workgroup[i] = rbuf[81 + i * 2];
    i++;
  }

  while ((rbuf[81 + (i + j + 1) * 2] != 0) && (j < 16)) {
    machine_name[j] = rbuf[81 + (i + j + 1) * 2];
    j++;
  }

  return 2;
}


/*
  SMBSessionSetup
  Function: Send username + response to the challenge from
            the server.
       Currently we're sendin ZEROES for the LMHASH since
       NT4/2000 doesn't seem to look at this if we got a
       valid NTLM hash.
  Returns: TRUE on success else FALSE.
*/
short
SMBSessionSetup(int s, char *user, char *pass, char *miscptr)
{
  unsigned char b[137] = {
    0x00, 0x00, 0x00, 0x85, 0xff, 0x53, 0x4d,
    0x42, 0x73, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c,
    0x7d, 0x00, 0x00, 0x01, 0x00, 0x0d, 0xff, 0x00,
    0x00, 0x00, 0xff, 0xff, 0x02, 0x00, 0x3c, 0x7d,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x18, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
    0x48, 0x00, 0xdf, 0x82, 0xb9, 0x2f, 0xda, 0xdb,
    0x43, 0x47, 0x09, 0xdb, 0x7f, 0xfc, 0xb0, 0xa8,
    0xf0, 0x46, 0xe2, 0xfe, 0x64, 0x6d, 0x67, 0x58,
    0xe0, 0xf9, 0xca, 0x9f, 0x5e, 0xdb, 0xe0, 0x15,
    0x80, 0xf8, 0x54, 0xe3, 0x6e, 0xe9, 0xf8, 0x88,
    0x80, 0x19, 0xb6, 0xf9, 0xae, 0xb7, 0xa6, 0x62,
    0x14, 0xfc, 0x54, 0x45, 0x53, 0x54, 0x00, 0x4d,
    0x59, 0x47, 0x52, 0x4f, 0x55, 0x50, 0x00, 0x55,
    0x6e, 0x69, 0x78, 0x00, 0x53, 0x61, 0x6d, 0x62,
    0x61, 0x00
  };

  unsigned char buf[512];
  unsigned char *NTLMhash;
  unsigned char LMhash[24];
  unsigned char rbuf[400];
  unsigned char sess_key[2];
  int userlen;

  NTLMhash = (unsigned char *) malloc(24);

  memset(NTLMhash, 0, 24);
  memset(LMhash, 0, 24);
  memset(rbuf, 0, 400);

  if (accntFlag == 0) {
    strcpy((char *) workgroup, "localhost");
  } else if (accntFlag == 2) {
    memset(workgroup, 0, 16);
  }

  HashNTLM(&NTLMhash, (unsigned char *) pass, (unsigned char *) challenge, miscptr);

  memset(buf, 0, 512);
  memcpy(buf, b, 89);
  memcpy(buf + 65, LMhash, 24);
  memcpy(buf + 89, NTLMhash, 24);

  /* set session key */
  sess_key[1] = getpid() / 100;
  sess_key[0] = getpid() - (100 * sess_key[1]);
  memcpy(buf + 30, sess_key, 2);
  
  userlen = strlen(user);

  memcpy(buf + 113, user, userlen);
  memset(buf + (113 + userlen), 0, 1);
  memcpy(buf + (114 + userlen), workgroup, strlen((char *) workgroup));
  memcpy(buf + (114 + userlen + strlen((char *) workgroup)), b + 125, 12);

  /* set the header length */
  buf[3] = (userlen + strlen((char *) workgroup) + 0x7A) % 256;
  buf[2] = (userlen + strlen((char *) workgroup) + 0x7A) / 256;

  /* set data length */
  buf[63] = 0x1F + strlen((char *) workgroup) + userlen;

  hydra_send(s, (char *) buf, 126 + userlen + strlen((char *) workgroup), 0);
  hydra_recv(s, (char *) rbuf, sizeof(rbuf));

  /* 41 - Action (Guest/Non-Guest Account) */
  /*  9 - NT Status (Error code) */
  return ((rbuf[41] << 8) | rbuf[9]);
}

int
start_smbnt(int s, unsigned long int ip, int port, unsigned char options, char *miscptr, FILE * fp)
{
  char *empty = "";
  char *login, *pass;
  int SMBerr, SMBaction;
  short SMBSessionRet;
  char ipaddr_str[INET_ADDRSTRLEN];

  if (strlen(login = hydra_get_next_login()) == 0)
    login = empty;
  if (strlen(pass = hydra_get_next_password()) == 0)
    pass = empty;

#ifdef CYGWIN
  strcpy(ipaddr_str, "10.244.112.61"); // XXX TODO: temp fix!
  
#else
  inet_ntop(AF_INET, &ip, ipaddr_str, sizeof(ipaddr_str));
#endif
  SMBSessionRet = SMBSessionSetup(s, login, pass, miscptr);
  SMBerr = (short) SMBSessionRet & 0x00FF;
  SMBaction = ((short) SMBSessionRet & 0xFF00) >> 8;

  if (0x00 == SMBerr) {         /* success */
    if (0x01 == SMBaction) {  /* invalid account - anonymous connection */
      fprintf(stderr, "[%d][smb] Host: %s Account: %s Error: Invalid Account (Anonymous Mapping Created)\n", port, ipaddr_str, login);
      hydra_completed_pair();
    } else {         /* valid account */
      hydra_report_found_host(port, ip, "smbnt", fp);
    hydra_completed_pair_found();
    }
  } else if (0x24 == SMBerr) {  /* change password on next login [success] */
    fprintf(stderr, "[%d][smb] Host: %s Account: %s Error: ACCOUNT_CHANGE_PASSWORD\n", port, ipaddr_str, login);
    hydra_completed_pair_found();
  } else if (0x6D == SMBerr) {  /* STATUS_LOGON_FAILURE */
    hydra_completed_pair();
  } else if (0x72 == SMBerr) {  /* account disabled */
    fprintf(stderr, "[%d][smb] Host: %s Account: %s Error: ACCOUNT_DISABLED\n", port, ipaddr_str, login);
    hydra_completed_pair();
  } else if (0x34 == SMBerr) {  /* account locked out */
    fprintf(stderr, "[%d][smb] Host: %s Account: %s Error: ACCOUNT_LOCKED\n", port, ipaddr_str, login);
    hydra_completed_pair();
  } else if (0x8D == SMBerr) {  /* ummm... broken client-domain membership  */
    fprintf(stderr, "[%d][smb] Host: %s Account: %s Error: NT_STATUS_TRUSTED_RELATIONSHIP_FAILURE\n", port, ipaddr_str, login);
    hydra_completed_pair();
  } else {                      /* failed */
    fprintf(stderr, "[%d][smb] Host: %s Account: %s Unknown Error: %2.2X\n", port, ipaddr_str, login, SMBerr);
    hydra_completed_pair();
  }

  hydra_disconnect(s);
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return 3;
  return 1;
}

void
service_smbnt(unsigned long int ip, int sp, unsigned char options, char *miscptr, FILE * fp, int port)
{
  int run = 1, next_run, sock = -1;

  if ((strcmp(miscptr, "L") == 0)) {
    accntFlag = 0;
    hashFlag = 0;
  } else if ((strcmp(miscptr, "LH") == 0)) {
    accntFlag = 0;
    hashFlag = 1;
  } else if ((strcmp(miscptr, "D") == 0)) {
    accntFlag = 1;
    hashFlag = 0;
  } else if ((strcmp(miscptr, "DH") == 0)) {
    accntFlag = 1;
    hashFlag = 1;
  } else if ((strcmp(miscptr, "B") == 0)) {
    accntFlag = 2;
    hashFlag = 0;
  } else if ((strcmp(miscptr, "BH") == 0)) {
    accntFlag = 2;
    hashFlag = 1;
  } else if ((strcmp(miscptr, "M") == 0)) {
    accntFlag = 0;
    hashFlag = 2;
  } else {
    printf("SMBNT Module requires -m METHOD\n");
    hydra_child_exit(0);
  }

  hydra_register_socket(sp);
  if (memcmp(hydra_get_next_pair(), &HYDRA_EXIT, sizeof(HYDRA_EXIT)) == 0)
    return;
  for (;;) {
    switch (run) {
    case 1:                    /* connect and service init function */
      if (sock >= 0)
        sock = hydra_disconnect(sock);
//      usleep(300000);

      if (port != 0) {
        sock = hydra_connect_tcp(ip, port);
        if (port == PORT_SMB) 
          protoFlag = WIN_NETBIOSMODE;
        else  
          protoFlag = WIN2000_NATIVEMODE;
      }
      else {
        sock = hydra_connect_tcp(ip, PORT_SMBNT);
        if (sock > 0) {
          port = PORT_SMBNT;
          protoFlag = WIN2000_NATIVEMODE;
        }
        else {
          fprintf(stderr, "Failed to establish WIN2000_NATIVE mode. Attempting WIN_NETBIOS mode.\n");
          port = PORT_SMB;
          protoFlag = WIN_NETBIOSMODE;
          sock = hydra_connect_tcp(ip, PORT_SMB);
        }
      }
      if (sock < 0) {
        fprintf(stderr, "Error: Child with pid %d terminating, can not connect\n", (int) getpid());
        hydra_child_exit(1);
      }
      if (NBSSessionRequest(sock) < 0) {
        fprintf(stderr, "Session Setup Failed: ");
        fprintf(stderr, "(Is the server service running?)\n");
        exit(-1);
      }
      next_run = SMBNegProt(sock);
      break;
    case 2:                    /* run the cracking function */
      next_run = start_smbnt(sock, ip, port, options, miscptr, fp);
      break;
    case 3:                    /* clean exit */
      if (sock >= 0)
        sock = hydra_disconnect(sock);
      hydra_child_exit(0);
      return;
    default:
      fprintf(stderr, "Caught unknown return code (%d), exiting!\n", run);
      hydra_child_exit(0);
      exit(-1);
    }
    run = next_run;
  }
}
#endif
