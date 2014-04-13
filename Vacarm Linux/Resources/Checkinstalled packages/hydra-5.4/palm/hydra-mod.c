#include <PalmOS.h>
#include "hydra-mod.h"

char HYDRA_EXIT[5] = "\x00\xff\x00\xff\x00";
char *HYDRA_EMPTY  = "\x00\x00\x00\x00";

int  use_ssl = 0;
char pair[52];    /* login \0x0 password is stored here ...
		     username/pass not longer than 24 chars */

UInt16 AppNetRefnum = 0;
char fixed_login[25];	     
char fixed_pass[25];
DmOpenRef loginData = NULL;
DmOpenRef passData = NULL;
int pass_index = 0;
int login_index = 0;

int running = false;

Int16 intern_connect(unsigned long int ip, UInt16 port, NetSocketTypeEnum protocol){
	NetSocketRef MasterSocket;
	Err errno;
	NetSocketAddrINType sockAddrP;
	Int16 addrLen = sizeof(sockAddrP);
	Int32 AppNetTimeout = SysTicksPerSecond() * 10;   /* 10 secs timeout is enough... */
	unsigned long ipaddr;

	MasterSocket = NetLibSocketOpen (AppNetRefnum, netSocketAddrINET, netSocketTypeStream, protocol, AppNetTimeout, &errno);
		
	if ( MasterSocket < 0 )
		return 0;

	memset ((char *) &sockAddrP, 0, addrLen);
	sockAddrP.family = netSocketAddrINET;
	sockAddrP.addr = ip;	
	sockAddrP.port = htons(port);
	
	NetLibSocketConnect(AppNetRefnum, MasterSocket, &sockAddrP, addrLen, AppNetTimeout, &errno);
	return MasterSocket;
}


/*-----------------------------------------------------------------------*/

void hydra_report(FILE *stream, const char *format, ... ) {
	char buf[300];  /* eeek ... no vsnprintf :(  */

	va_list ap;
        va_start(ap,format);
	StrVPrintF(buf, format, ap);
	SetText(idStatus, buf);
	va_end(ap);
}

int getpid(){
	return 0;
}


/*-----------------------------------------------------------------------*/

char *hydra_get_next_pair() {
	MemHandle h;
	char *mypass;
	char *mylogin;

	if ( running == false )
		return HYDRA_EXIT;
	
	if (( loginData == NULL ) && ( login_index > 0 ))
		return HYDRA_EXIT;

	if (( passData == NULL ) && ( pass_index > 0 ))
		return HYDRA_EXIT;		
			
	if ( loginData != NULL ) {
		if ( login_index < DmNumRecords(loginData) ){
			h = DmQueryRecord(loginData, login_index);
			if ( h == NULL ) {
				pair[0] = 0;
				running = false;
				return HYDRA_EXIT;
			}
			mylogin = MemHandleLock(h);
			strcpy(pair, mylogin);
			MemHandleUnlock(h);
			login_index++;
		} else {
			pair[0] = 0;
			return HYDRA_EXIT;	
		}
	} else {
		strcpy(pair, fixed_login);
	}

	if ( passData != NULL ) {
		if ( pass_index < DmNumRecords(passData) ) {
			h = DmQueryRecord(passData, pass_index);
			if ( h == NULL ){
				pass_index = 0;
				login_index++;
				return hydra_get_next_pair();
			}
			mypass = MemHandleLock(h);
			strcpy(pair+1+StrLen(pair), mypass);
			MemHandleUnlock(h);
			pass_index++;
		} else {
				pass_index = 0;
				login_index++;
				return hydra_get_next_pair();
		}
		
	} else {
		strcpy(pair+1+StrLen(pair), fixed_login);
	}
	
	return pair;
}

char *hydra_get_next_login() {
	if ( !running ) {
		pair[0] = 0;
		return HYDRA_EMPTY;
	}
	if ( pair[0] == 0 )
		return HYDRA_EMPTY;
	return pair;
}


char *hydra_get_next_password() {
	char *ptr = pair;
	if ( !running ) {
		pair[0] = 0;
		return HYDRA_EMPTY;
	}
	
	while (*ptr != '\0') ptr++;
	ptr++;
	if ( *ptr == 0 )
		return HYDRA_EMPTY;
	return ptr;
}


Int16 hydra_connect_tcp(unsigned long int ip, UInt16 port){
	return intern_connect(ip, port, netSocketProtoIPTCP);
}

Int16 hydra_connect_udp(unsigned long int ip, UInt16 port){
	return intern_connect(ip, port, netSocketProtoIPUDP);
}

int hydra_send(Int16 socket, char *buf, int size, int options){
	Err error;
	Int32 AppNetTimeout = SysTicksPerSecond() * 10;

	if ( running == false )
		return -1;
	
	if ( use_ssl ){
		return 0; // SSL_send....
	} else {
		return NetLibSend(AppNetRefnum, socket, buf, size, 0, 0, 0, AppNetTimeout, &error);
	}
}

int hydra_recv(Int16 socket, char *buf, int length) {
	NetSocketAddrINType fromAddr;
	UInt16 fromLenP = sizeof(fromAddr);
	Err    error;
	int    tmp;
	Int32  AppNetTimeout = SysTicksPerSecond() * 10;   /* 10 secs timeout is enough... */

	if ( running == false )
		return -1;
		
	if ( use_ssl ) {
		return  0 ; //SSL_read(ssl, buf, length);
	} else {
		tmp = NetLibReceive(AppNetRefnum, socket, buf, length, 0, NULL, 0, AppNetTimeout, &error);
		return tmp;
	}
}

int hydra_disconnect(Int16 socket) {
	Err error;
	Int32 AppNetTimeout = SysTicksPerSecond() * 10;
	return NetLibSocketClose(AppNetRefnum, socket, AppNetTimeout, &error);
}

Int16 hydra_connect_ssl(unsigned long int ip, UInt16 port) {
	/* placeholder for now */
	return 0;
}

char *hydra_receive_line(Int16 socket) {
	char buf[300];
	MemHandle h;
	char *p;
	int i = 0, j = 1, k;

	h = MemHandleNew(sizeof(buf));
	p = (char *) MemHandleLock(h);
	memset(p, 0, sizeof(buf));
	i = hydra_data_ready_timed(socket, 30, 0);
	if ( i > 0 ) {
		if (( hydra_recv(socket, p, sizeof(buf))) < 0 ) {
			goto out;
		}
	}
	if ( i <= 0 ) {
		goto out;
	} else {
		for (k = 0; k < i; k++ ) {
			if ( p[k] == 0 ) {
				p[k] = 32;
			}
		}
	}
	
	while ((hydra_data_ready(socket) > 0) && (j > 0) ) {
	
		j = hydra_recv(socket, buf, sizeof(buf));
		if ( j > 0 ) {
			for (k=0; k < j; k++) {
				if ( buf[k] == 0 ) {
					buf[k] = 32;
				}
			}
		
			MemHandleUnlock(h);
			MemHandleResize(h, i+j);
			p = (char *) MemHandleLock(h);
			memcpy(p+i, &buf, j);
			i = i + j;
		} else {
			break;
		}
	}
	return p;

out:
	MemHandleUnlock(h);
	MemHandleFree(h);
	return NULL;

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

int hydra_data_ready_timed(Int16 socket, long sec, long usec ) {
	Err    error;
	UInt16 width;
	NetFDSetType readFDs, writeFDs, exceptFDs;	
	int    result;
	Int32  AppNetTimeout = SysTicksPerSecond() * sec;
	Int32  tmp = ( SysTicksPerSecond() * usec ) / 1000000;

//	SysTaskDelay(SysTicksPerSecond() * 2);

	AppNetTimeout += tmp + 1;
	netFDZero(&readFDs);
	netFDZero(&writeFDs);
	netFDZero(&exceptFDs);
	netFDSet(sysFileDescStdIn, &readFDs);
	netFDSet(socket, &readFDs);
	
	width = sysFileDescStdIn;
	if ( socket > width ) width = socket;

	result = NetLibSelect(AppNetRefnum, width+1, &readFDs, &writeFDs, NULL, AppNetTimeout, &error); 
	return result;
}

void usleep(unsigned long usec) {
	/* minimum value SysTicksPerSeconds returns, which I know is 100,
	   so lets hope we get a value passed which is bigger than 10000  */
	unsigned int temp = SysTicksPerSecond() * usec;

	temp = temp / 1000000;
	SysTaskDelay(temp+1);
	return;
}

int hydra_data_ready(Int16 socket) {
	return hydra_data_ready_timed(socket, 0, 100);
}

void hydra_completed_pair_found(){
	login_index++;
	pass_index = 0;	
	(void) hydra_get_next_pair();
}


void hydra_completed_pair() {
	pair[0] = 0;
}

void  hydra_register_socket(int s) {
	/* dummy */
	return;
}

void hydra_report_found(UInt16 port, char *svc, int fd) {
	char buff[200];
	sprintf(buff, "[%d][%s] login: %s   password: %s",port, svc, hydra_get_next_login(),hydra_get_next_password());
	SetText(idStatus, buff);
	SndPlaySystemSound(sndWarning);
	hydra_completed_pair_found();
}

void hydra_report_found_host(UInt16 port, unsigned long int ip, char* svc, FILE *fd) {
	char buff[200];
	char my_ip[18];
	
	NetLibAddrINToA(AppNetRefnum, ip, my_ip);
	sprintf(buff, "[%d][%s] host: %s   login: %s   password: %s", port, svc, my_ip, hydra_get_next_login(),hydra_get_next_password());
	SetText(idStatus, buff);
	SndPlaySystemSound(sndWarning);
	hydra_completed_pair_found();
}

void hydra_child_exit(int i) {
	running = false;
}

void SetText(unsigned int id, char *text ) {
 	FieldType* fldP;
	FormType* frmP;
	char *p;
	MemHandle h, oldHandle;
	frmP = FrmGetActiveForm();
	
	h = MemHandleNew(StrLen(text)+1);
	p = (char *) MemHandleLock(h);
	StrCopy(p, text);
	MemHandleUnlock(h);
	fldP = (FieldType *) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, idStatus));
	oldHandle = FldGetTextHandle(fldP);
	FldSetTextHandle(fldP, h);
	if ( oldHandle)
		MemHandleFree(oldHandle);
	FldDrawField(fldP);
	return;
}
