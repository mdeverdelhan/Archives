#include <PalmOS.h>
#include <DataMgr.h>
#include <sys_socket.h>
#include "hydra-mod.h"
#include "hydra.h"

Boolean 
HelpHandleEvent(EventPtr event){
Boolean handled = false;
  FormPtr form;
 
  switch (event->eType) {
  case frmOpenEvent:
    form = FrmGetActiveForm();
    FrmDrawForm(form);
    handled = true;
    break;
      
      
  case ctlSelectEvent:
    switch (event->data.ctlSelect.controlID) {
    case idBack:
      FrmGotoForm(idMainForm);
      handled = true;
      break;
    }
    break;
 
  case frmCloseEvent:
    handled = false;
    break;
 
  default:
    break;
  }
  
  return handled;
}

Boolean 
AboutHandleEvent(EventPtr event){
Boolean handled = false;
  FormPtr form;
 
  switch (event->eType) {
  case frmOpenEvent:
    form = FrmGetActiveForm();
    FrmDrawForm(form);
    handled = true;
    break;
      
      
  case ctlSelectEvent:
    switch (event->data.ctlSelect.controlID) {
    case idBack:
      FrmGotoForm(idMainForm);
      handled = true;
      break;
    }
    break;
 
  case frmCloseEvent:
    handled = false;
    break;
 
  default:
    break;
  }
  
  return handled;
}


void StartApplication(void)
{
	FrmGotoForm(idMainForm);
}

void StopApplication(void)
{
	FrmCloseAllForms();
}

UInt16 getPort(){
  char *port;
  MemHandle f;
  FormPtr frmP;
  FieldType *fldP;
  UInt16 port2 = 0;
  
  frmP = FrmGetActiveForm();
  fldP = (FieldType *) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, idPort));
  f = FldGetTextHandle(fldP);
  if ( f != NULL ) {
	port = (char *) MemHandleLock(f);
	port2 = atoi(port);
	MemHandleUnlock(f);
 }
 return port2;
}


Boolean getPassword() {
  FormPtr frmP;
  FieldType *fldP;
  MemHandle f;
  char *pass;

  frmP = FrmGetActiveForm();
  fldP = ( FieldType *) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, idPassword));
  f = FldGetTextHandle(fldP);
  
  if ( f != NULL ) {
	pass = (char *) MemHandleLock(f);
	strncpy(fixed_pass, pass, 24);
	MemHandleUnlock(f);
	passData = NULL;
	return true;
  } else { 
  	pass_index = 0;
	/* if palmcrack is installed, try to open the palmcrack dictionairy first */
	passData = DmOpenDatabaseByTypeCreator(DB_ID_PC_DATA, DB_ID_PC_CREA, dmModeReadOnly);
        if ( passData == NULL ) {
		passData = DmOpenDatabaseByTypeCreator(DB_ID_PASS, DB_ID_PRUT, dmModeReadOnly);
		if ( passData == NULL ){
	  		SetText(idStatus, "Cant open password database");
			return false;
		}
	}
  }

  return true;
}


Boolean getLogin() {
  FormPtr frmP;
  FieldType *fldP;
  MemHandle f;
  char *login;

  frmP = FrmGetActiveForm();
  fldP = ( FieldType *) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, idUsername));
  f = FldGetTextHandle(fldP);
  
  if ( f != NULL ) {
	login = (char *) MemHandleLock(f);
	strncpy(fixed_login, login, 24);
	MemHandleUnlock(f);
	loginData = NULL;
	return true;
  } else { 
  	login_index = 0;
	loginData = DmOpenDatabaseByTypeCreator(DB_ID_LOGI, DB_ID_PRUT, dmModeReadOnly);
	if ( loginData == NULL ){
	  	SetText(idStatus, "Cant open login database");
		return false;
	}
  }

  return true;
}


char *getIP( MemHandle *h) {
  FormPtr frmP;
  MemHandle q1, q2, q3, q4;
  char *p;
  char *ip;
  char *ip1, *ip2, *ip3, *ip4;
  FieldType *fldP2;

  *h = MemHandleNew(16);
  ip = (char *) MemHandleLock(*h);
  frmP = FrmGetActiveForm();
  
  fldP2 = (FieldType *) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, idIp1));
  q1 = FldGetTextHandle(fldP2);
  fldP2 = (FieldType *) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, idIp2));
  q2 = FldGetTextHandle(fldP2);
  fldP2 = (FieldType *) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, idIp3));
  q3 = FldGetTextHandle(fldP2);
  fldP2 = (FieldType *) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, idIp4));
  q4 = FldGetTextHandle(fldP2);
		
  if ((q1 == NULL ) || (q2 == NULL ) || (q3 == 0 ) || (q4 == NULL )){
	running = false;
	SetText(idStatus, "No valid IP");
	
	MemHandleUnlock(*h);
	MemHandleFree(*h);
	return NULL;
  } else {
	ip1 = (char *) MemHandleLock(q1);
	ip2 = (char *) MemHandleLock(q2);
	ip3 = (char *) MemHandleLock(q3);
	ip4 = (char *) MemHandleLock(q4);
  } 
	
  sprintf(ip, "%s.%s.%s.%s", ip1, ip2, ip3, ip4);

  MemHandleUnlock(q1);
  MemHandleUnlock(q2);
  MemHandleUnlock(q3);
  MemHandleUnlock(q4);
  
  return ip;

}

Boolean Hydra01HandleEvent(EventPtr event)
{
	FormPtr formular, frmP;
	Boolean handled=false;
	MemHandle fldH, h;
	FieldType *fldP, *fldP2;
	UInt16 formID;
	UInt16 port = 0;
	char *ip;
	Int16 s;
	FormPtr form;
	int tmp = 0;
	unsigned long int final_ip;
	
	switch(event->eType)
	{
		case frmOpenEvent:
			formular=FrmGetActiveForm();
			FrmDrawForm(formular);
			handled=true;
			break;
		case ctlSelectEvent:
			switch(event->data.ctlEnter.controlID) 
			{
				case idStart:
					handled = true;
					if ( !running ) {
						passData = NULL;
						loginData = NULL;
						
						SetText(idStatus, "Started...");
					        debug = false;	
						if ( getPassword() == false ){ break; }
						if ( getLogin() == false ){ break; }	
						ip = getIP(&h);
						if ( ip == NULL ) { break; }
						final_ip = NetLibAddrAToIN(AppNetRefnum,ip);
						MemHandleUnlock(h);
						MemHandleFree(h);
						running = true;
						port = getPort();

						frmP = FrmGetActiveForm();
						fldP2 = (FieldType *) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, idProtocols));
						s = LstGetSelection(fldP2);
						switch (s) {
							case sFTP:
								service_ftp(final_ip, 0, 0, NULL, NULL, port);
								break;
							case sPOP3:
								service_pop3(final_ip, 0, 0, NULL, NULL, port);
								break;
							case sTELNET:
								service_telnet(final_ip, 0, 0, NULL, NULL, port);
								break;
							case sVNC:
								service_vnc(final_ip, 0, 0, NULL, NULL, port);
								break;
							case sIMAP:
								service_imap(final_ip, 0, 0, NULL, NULL, port);
								break;
							case sCISCO:
								service_cisco(final_ip, 0, 0, NULL, NULL, port);
								break;
							case sNNTP:
								service_nntp(final_ip, 0, 0, NULL, NULL, port);
								break;
							case sVMAUTHD:
								service_vmauthd(final_ip, 0, 0, NULL, NULL, port);
								break;
						}
				
						if ( passData != NULL ){ DmCloseDatabase(passData); }
						if ( loginData != NULL ){ DmCloseDatabase(loginData); }
					
						running = false;
					
						break;
					}
				case idStop:
					if ( running ) {
						running = false;
						SetText(idStatus, "Stopped...");
						handled = true;
						break;
					}
				case idPopuplist:
					handled = false;
					break;
			}	
			break;
		case frmCloseEvent:
//			formular=FrmGetActiveForm();
//			fldP = (FieldType *)FrmGetObjectPtr(formular, FrmGetObjectIndex(formular, idStatus));
//			fldH = FldGetTextHandle(fldP);
//			if ( fldH )
//				MemHandleFree(fldH);
		case menuEvent:
			switch (event->data.menu.itemID){
			case idMenuAbout:
				FrmGotoForm(idAboutForm);
				break;
			case idMenuHelp:
				FrmGotoForm(idHelpForm);
				break;
			}
			break;
	}
	return handled;
}

Boolean ApplicationHandleEvent(EventPtr event)
{
	FormPtr formular;
	int formID;
	Boolean handled=false;

	switch (event->eType)
	{
		case frmLoadEvent:
			formID=event->data.frmLoad.formID;
			formular=FrmInitForm(formID);
			FrmSetActiveForm(formular);

			switch(formID)
			{
				case idMainForm:
					FrmSetEventHandler(formular, Hydra01HandleEvent);
					break;
				case idHelpForm:
					FrmSetEventHandler(formular, HelpHandleEvent);
					break;
				case idAboutForm:
					FrmSetEventHandler(formular, AboutHandleEvent);
					break;
			}
			handled=true;
			break;
	}
	return handled;
}

UInt32 PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
	UInt32 OSVersion;
	UInt32 benoetigteOSVersion;
	EventType event;
	Err error;
	UInt16 netIFErrsP;


	if (cmd == sysAppLaunchCmdNormalLaunch)
	{
		FtrGet(sysFtrCreator, sysFtrNumROMVersion, &OSVersion);
		benoetigteOSVersion=sysMakeROMVersion(3, 0, 0, sysROMStageRelease, 0);
//		if (OSVersion<benoetigteOSVersion)
//		{
//			FrmCustomAlert(OldOSAlert, NULL, NULL, NULL);
//			return 0;
//		}

		/* Load Networking Library */
		/* yeah... bad error handling... *yuck* */
		error = SysLibFind ("Net.lib", &AppNetRefnum);
		if (error != 0) {
			error = SysLibLoad (sysFileTLibrary, sysFileCNet, &AppNetRefnum);
			if ( error )
				return 0;
		}
		
		error = NetLibOpen(AppNetRefnum, &netIFErrsP);				
		if (( error == netErrOutOfMemory ) || ( error == netErrNoInterfaces  ) || ( error == netErrPrefNotFound ))
			return 0;				

		
		StartApplication();
		// Behandlung der Events
		do
		{
			EvtGetEvent(&event, evtWaitForever);
			if (!SysHandleEvent(&event))
			{
				if (!MenuHandleEvent(0, &event, &error))
				{
					if (!ApplicationHandleEvent(&event))
					{
						FrmDispatchEvent(&event);
					}
				}
			}
		}
		while (event.eType != appStopEvent);
		StopApplication();
	}
	return 0;
}

