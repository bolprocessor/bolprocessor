/* OMS.c (BP2 version CVS) */

// Written by Bernard Bel (bel@kagi.com) on the basis of the OMS Software Developers Kit
// supplied by OpCode (ref. OMS 2.0 SDK 960523)

/*  This file is a part of Bol Processor 2
    Copyright (c) 1990-2000 by Bernard Bel, Jim Kippen and Srikumar K. Subramanian
    All rights reserved. 
    
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met: 
    
       Redistributions of source code must retain the above copyright notice, 
       this list of conditions and the following disclaimer. 
    
       Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution. 
    
       Neither the names of the Bol Processor authors nor the names of project
       contributors may be used to endorse or promote products derived from this
       software without specific prior written permission. 
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

/* This is defined by both Carbon and non-Carbon prefix headers */
#if  !defined(TARGET_API_MAC_CARBON)
   /* so if it is not defined yet, there is no prefix file, 
      and we are compiling the "Transitional" build. */
   /* Use MacHeaders.h until ready to convert this file.
      Then change to MacHeadersTransitional.h. */
// #  include	"MacHeaders.h"
#  include	"MacHeadersTransitional.h"
#endif

#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"

Boolean gMIDIMgrExists;	/* is MIDI Manager present? */
short gCompatMode;			/* current OMS compatibility mode */
short MyAppRefCon;

// The following globals should be defined in a common header:
// extern Boolean gSignedInToMIDIMgr;		/* are we signed into MIDI Manager? */
// extern Boolean gNodesChanged;
// extern short gInputPortRefNum;		/* refNum of the OMS input port */
// extern short gOutputPortRefNum;		/* refNum of the OMS output port */
// extern OMSDeviceMenuH gInputMenu,gOutputMenu;

// extern short gChosenInputID;		/* uniqueID of selected input; 0 means none */
// extern short gChosenOutputID;	/* uniqueID of selected output; 0 means none */
// extern short gOutNodeRefNum;	/* node refNum of the selected output; -1 means non existant */
// extern long BytesReceived,BytesProcessed,MaxOMSinputBufferSize;
// extern char DownBuffer,OMSinputOverflow;
// extern MIDIcode **h_OMSinputMessage;

OSErr InitOMS(OSType appSignature)
{
OSErr err;
OMSAppHookUPP appHook;
OMSReadHook2UPP readHook;
long version,size;
OMSNodeInfoListH nodelist;
OMSNodeInfoList node;
short oldid;
	
#if UseMIDIMgr
	gMIDIMgrExists = (SndDispVersion(midiToolNum) != 0);
#else
	gMIDIMgrExists = FALSE;
#endif
	gSignedInToMIDIMgr = FALSE;
	
if((version=OMSVersion()) == 0) {
	Alert1("Opcode OMS is not installed or inactive. Internal MIDI driver will be used instead");
	ShowMessage(TRUE,wInfo,"Download OMS from ftp://ftp.opcode.com/pub/oms/mac");
	return 1;	/* any old error number */
	}
if(version < 0x02000000) {
	Alert1("BP2 requires OMS version 2.0 or higher.\nInternal MIDI driver will be used");
	ShowMessage(TRUE,wInfo,"Download OMS from ftp://ftp.opcode.com/pub/oms/mac");
	return 1;
	}
	
ShowMessage(TRUE,wMessage,"BP2 is signing in to OMS...");
	
#ifdef powerc
	appHook = NewOMSAppHook(MyAppHook);
	readHook = NewOMSReadHook2(MyReadHook);
#else
	appHook = MyAppHook;
	readHook = MyReadHook;
#endif

MyAppRefCon = 104;

err = OMSSignIn(appSignature,(long)LMGetCurrentA5(),LMGetCurApName(),appHook,
		&gCompatMode);
/*	Passing CurrentA5 as the refCon solves the problem of A5 setup in the appHook.
	Using other Apple-recommended techniques for setting up A5 in the appHook
	are fine as well.  The client name will be the same as the application's name,
	as stored in the low-memory global CurApName. */

if(err) {
	Alert1("OMS is installed but BP2 failed to sign in.\nIt's a good idea to restart the computer and run 'OMS Setup'");
	TellError(88,err);
	return err;
	}
FlashInfo("OMS MIDI driver is open...");

/*	Add an input port */

OPENINPUT:
gChosenInputID = 0; /* $$$ */
// if(gChosenInputID != 0) {
	err = OMSAddPort(appSignature,InputPortID,omsPortTypeInput2,(OMSReadHook2UPP)readHook,
		(long)LMGetCurrentA5(),&gInputPortRefNum);
	if(err) {
		Alert1("OMS couldn't set up its MIDI input port using default settings. You will need to setup the input, modify '-se.startup' and restart");
		MustChangeInput = TRUE;
		}
//	}

/*	Add an output port */
OUTPUT:
nodelist = OMSGetNodeInfo(omsIncludeReal+omsIncludeOutputs);
gOutNodeRefNum = OMSUniqueIDToRefNum(gChosenOutputID);
if(gOutNodeRefNum == OMSInvalidRefNum) {
	/* Select the first device available, if any */
	if(nodelist != NULL) {
		node = (*nodelist)[0];
		if(node.numNodes > 0) {
			gChosenOutputID = node.info[0].uniqueID;
			gOutNodeRefNum = OMSUniqueIDToRefNum(gChosenOutputID);
			}
		}
	}
if(nodelist != NULL) OMSDisposeHandle(nodelist);

err = OMSAddPort(appSignature,gChosenOutputID,omsPortTypeOutput,NULL,0L,&gOutputPortRefNum);

if(err) goto ERREXIT;

size = MaxOMSinputBufferSize * sizeof(MIDIcode);
if((h_OMSinputMessage = (MIDIcode**) GiveSpace((Size)size)) == NULL) {
	err = memFullErr;
	goto ERREXIT;
	}
if(!InitOn) MemoryUsed -= (unsigned long) size;
BytesReceived = BytesProcessed = ZERO;
DownBuffer = TRUE; OMSinputOverflow = FALSE;
MaxMIDIbytes = MAXTIMESLICES - 50;

if(!NEWTIMER && InstallTMTask() != OK) {
	err = memFullErr;
	goto ERREXIT;
	}

return noErr;

ERREXIT:
Alert1("OMS couldn't be initialised for some unknown reason. You should check its setup");
HideWindow(Window[wInfo]);
ClearMessage();
TellError(89,err);
OMSSignOut(appSignature);
Oms = FALSE;
ChangeControlValue(TRUE,Hbutt[bOMS],Oms);
return err;
}


int ExitOMS(void)
{
long size;

size = MyGetHandleSize((Handle)h_OMSinputMessage);
MyDisposeHandle((Handle*)&h_OMSinputMessage);
MemoryUsed += (unsigned long) size;

if(!NEWTIMER) RemoveTMTask();

if(gSignedInToMIDIMgr) SignOutFromMIDIMgr();
gSignedInToMIDIMgr = FALSE;

if(gInputMenu != NULL) DisposeOMSDeviceMenu(gInputMenu);
if(gOutputMenu != NULL) DisposeOMSDeviceMenu(gOutputMenu);
gInputMenu = gOutputMenu = NULL;

OMSSignOut('Bel0');
Oms = FALSE;
ChangeControlValue(TRUE,Hbutt[bOMS],Oms);
return(OK);
}


void SignInToMIDIMgr(void)
{
/* whatever you want to do */
gSignedInToMIDIMgr = TRUE;
}


void SignOutFromMIDIMgr(void)
{
/* whatever you want to do */
gSignedInToMIDIMgr = FALSE;
}


void CheckSignInOrOutOfMIDIManager(void)
{
/* Make sure MIDI Manager is installed; it's possible (but unlikely) that OMS will
	say that the compatibility mode is omsModeUseMIDIMgr though MIDI Manager is
	not present */
	
if(!gMIDIMgrExists)
	return;
if(gCompatMode == omsModeUseMIDIMgr) {
	if(!gSignedInToMIDIMgr) SignInToMIDIMgr();
	}
else {
	if(gSignedInToMIDIMgr) SignOutFromMIDIMgr();
	}
}

/*
	If opening is true,open a connection to the device specified by gChosenInputID.
	If false, close the connection.
	
	This is overly simplistic; applications should support multiple input
	devices enabled simultaneously,typically using OMSChooseNodes.
*/

int OpenOrCloseConnection(int startup,int opening)
{
OSErr err;
OMSConnectionParams conn;
OMSConnectionListH list;
OMSReadHook2UPP readHook;
int tried,result;

if(gChosenInputID == 0) {
	if(opening) return(FAILED);
	else return(OK);
	}

// list = OMSGetConnections(0);
// if(list != NULL) OMSDisposeHandle(list);
	
#ifdef powerc
	readHook = NewOMSReadHook2(MyReadHook);
#else
	readHook = MyReadHook;
#endif

conn.nodeUniqueID = gChosenInputID;
conn.appRefCon = MyAppRefCon;
SchedulerIsActive--;
tried = FALSE; result = OK;

TRY:
if(opening) {
	err = OMSOpenConnections('Bel0',InputPortID,1,&conn,FALSE);
	switch(err) {
		case noErr:
			sprintf(Message,"Opened MIDI input '%s' ID = %ld",
				OMSinputName,(long)InputPortID);
			ShowMessage(TRUE,wMessage,Message);
			SchedulerIsActive++;
			return(OK);
			break;
		case omsAppPortNotFoundErr:
			if(!tried) {
				tried = TRUE;
				if(Beta) {
					sprintf(Message,"Port not found, MIDI input '%s' ID = %ld. Trying to add it...",
						OMSinputName,(long)InputPortID);
					ShowMessage(YES,wMessage,Message);
					}
				err = OMSAddPort('Bel0',InputPortID,omsPortTypeInput2,(OMSReadHook2UPP)readHook,
					(long)LMGetCurrentA5(),&gInputPortRefNum);
				if(err == noErr) goto TRY;
				if(Beta) {
					sprintf(Message,"Unable to add this port, MIDI input '%s' ID = %ld",
						OMSinputName,(long)InputPortID);
					ShowMessage(YES,wMessage,Message);
					}
				}
			/* no break */
		case omsNodeNotFoundErr:
			gChosenInputID = 0;
			if(ScriptExecOn) {
				sprintf(Message,
					"\nOMS failed to find '%s' input device. You should modify project settings or '-se.startup'...",
					OMSinputName);
				Println(wTrace,Message);
				}
			else {
				if(startup) {
					sprintf(Message,
						"OMS failed to find '%s' input device. I will help you to find it and modify '-se.startup'...",
						OMSinputName);
					Alert1(Message);
					DisplayHelp("OMS setting default input device");
					MustChangeInput = TRUE;
					break;
					}
				else {
					sprintf(Message,
						"OMS failed to find '%s' input device. You should modify project settings or '-se.startup'...",
						OMSinputName);
					Alert1(Message);
					}
				}
			SchedulerIsActive++;
			mOMSinout(Nw);
			return(FAILED);
			break;
		default:
			if(Beta) TellError(90,err);
			result = FAILED;
			gChosenInputID = 0;
			break;
		
		}
	}
else {
	sprintf(Message,"Closed MIDI input '%s' ID = %ld",OMSinputName,(long)gChosenInputID);
	ShowMessage(TRUE,wMessage,Message);
	if(gInputMenu != NULL)
		SetOMSDeviceMenuSelection(gInputMenu,0,gChosenInputID,"\p",FALSE);
	OMSCloseConnections('Bel0',InputPortID,1,&conn);
	gChosenInputID = 0;
	if(gInputMenu != NULL) RebuildOMSDeviceMenu(gInputMenu);
	}
SchedulerIsActive++;
return(result);
}


OMSAPI(void) MyAppHook(OMSAppHookMsg *pkt,long myrefcon)
{
#ifdef THINK_C
	/*	Set up A5 for access to globals during this routine. */
	asm {
		move.l	a5,-(sp)		; save previous A5
		move.l	myrefcon,a5		; set up A5 from myrefcon
	}
#elif __MWERKS__
	long olda5 = SetA5(myrefcon);
#endif
	
	switch (pkt->msgType) {
	case omsMsgModeChanged:
		/* Respond to compatibility mode having changed */
		gCompatMode = pkt->u.modeChanged.newMode;
		/* this will cause side effects in the event loop */
		break;
	case omsMsgDestDeleted:
		if(gChosenOutputID == pkt->u.nodeDeleted.uniqueID) {
			gOutNodeRefNum = OMSInvalidRefNum;	/* invalid */
		}
		break;
	case omsMsgNodesChanged:
		gNodesChanged = TRUE;
		break;
	}

#ifdef THINK_C
	asm {
		move.l	(sp)+,a5		; restore previous A5
	}
#elif __MWERKS__
	SetA5(olda5);
#endif
}


OMSAPI(void) MyReadHook(OMSMIDIPacket *pkt,long myrefcon)
{
long i;

/*	Set up A5 for access to globals during this routine. */
#ifdef THINK_C
asm {
	move.l	a5,-(sp)		; save previous A5
	move.l	myrefcon,a5		; set up A5 from myrefcon
	}
#elif __MWERKS__
	long olda5 = SetA5(myrefcon);
#endif
	
	/*	Process the MIDI packet as efficiently as possible.  It is guaranteed to be MIDI
		data, not some high-level event.  The applicationÕs refCon (appRefCon) that was 
		passed to OMSOpenConnections is in the low order word of pkt->tStamp. 
		A convenient way for an application to determine the source of the MIDI data is for 
		it to pass a number identifying the source as the appRefCon to OMSOpenConnection.
		The high-order word of pkt->tStamp is the sourceÕs ioRefNum (not its uniqueID); 
		applications can also look at this to determine the source of the MIDI data. */
	
	if(Oms && gChosenInputID != 0 && !LoadOn && !InitOn && SchedulerIsActive > 0
			&& ((short)pkt->appConnRefCon) == MyAppRefCon
			/* && ((short)pkt->srcIORefNum) == gInputPortRefNum $$$ this doesn't work */) {
		/* Types are not consistent: OMSMIDIPacket uses unsigned short ! */
				
		/* Transmit message immediately if allowed. No time should be wasted. */
		if(AcceptEvent(ByteToInt(pkt->data[0]))) {
			if(gOutNodeRefNum != OMSInvalidRefNum && PassEvent(ByteToInt(pkt->data[0]))) {
				SchedulerIsActive--;
				OMSWritePacket2(pkt,gOutNodeRefNum,gOutputPortRefNum);
				SchedulerIsActive++;
				}
			PushMIDIdata(pkt->len,pkt->data);
			}
		}

#ifdef THINK_C
asm {
	move.l	(sp)+,a5	; restore previous A5
	}
#elif __MWERKS__
SetA5(olda5);
#endif
}


void TestClientVirtualNodes(void)
{
OMSClientListH clients;
short iclient;

clients = OMSGetClients();
if(clients != NULL) {
	for(iclient = (*clients)->numClients; --iclient >= 0; ) {
		OMSSignature clientID = (*clients)->client[iclient].clientID;
		OMSVirtualNodeIDListH nodes = OMSGetClientVirtualNodes(clientID);
		if(nodes != NULL) OMSDisposeHandle(nodes);
		}
	OMSDisposeHandle(clients);
	}
}


TryOMSoutput(void)
/* This procedure sends a stream of notes in real time to OMS */
{
short note;
OMSMIDIPacket pkt;
long time;
unsigned long delay; // It was 'long' until 30/10/00

time = 0L;
for(note = 60; note <= 72; note++) {
	pkt.flags = 0;
	pkt.len = 3;
	pkt.data[0] = NoteOn;
	pkt.data[1] = note;
	pkt.data[2] = 64;
	pkt.appConnRefCon = 'Bel0';
	pkt.smpteTimeStamp = 0;
	OMSWritePacket2(&pkt,gOutNodeRefNum,gOutputPortRefNum);
	Delay(10L,&delay);
	pkt.data[2] = 0;
	OMSWritePacket2(&pkt,gOutNodeRefNum,gOutputPortRefNum);
	}
return(OK);
}


InputMenuSideEffects(void)
{
int result;

result = OK;
if(!Oms || gInputMenu == NULL) {
	if(Beta) Alert1("Err. InputMenuSideEffects(). !Oms || gInputMenu == NULL");
	return(OK);
	}
if(InitOn || LoadOn) return(OK);
ClearMessage();
if((result = OpenOrCloseConnection(FALSE,FALSE)) != OK) return(result);
if(gInputMenu != NULL) {
	gChosenInputID = gChosenInputIDbydefault = (*gInputMenu)->select.uniqueID;
	MyPtoCstr(255,(*gInputMenu)->selectedName,OMSinputName);
	}
if(!MustChangeInput) result = OpenOrCloseConnection(FALSE,TRUE);
else return(StoreDefaultOMSinput());
return(result);
}


OutputMenuSideEffects(void)
{
OSErr err;

if(!Oms || gOutputMenu == NULL) {
	if(Beta) Alert1("Err. OutputMenuSideEffects(). !Oms || gOutputMenu == NULL");
	return(FAILED);
	}
ClearMessage();
gChosenOutputID = (*gOutputMenu)->select.uniqueID;
gOutNodeRefNum = (*gOutputMenu)->selectedRefNum;
MyPtoCstr(255,(*gOutputMenu)->selectedName,OMSoutputName);
if(gOutNodeRefNum == OMSInvalidRefNum) {
	if(Beta) Alert1("Err. OutputMenuSideEffects(). gOutNodeRefNum == OMSInvalidRefNum");
	return(FAILED);
	}
	
err = OMSAddPort('Bel0',OutputPortID,omsPortTypeOutput,NULL,0L,&gOutputPortRefNum);
if(err) {
	if(err != 4) TellError(91,err);
	else ShowMessage(TRUE,wMessage,"This port was already open...");
	return(FAILED);
	}
else {
	sprintf(Message,"Opened MIDI output '%s' ID = %ld...",OMSoutputName,gChosenOutputID);
	ShowMessage(TRUE,wMessage,Message);
	}
SetMIDIPrograms();
return(OK);
}


SetOMSdriver(void)
{
OMSNodeInfoListH nodelist;
OMSNodeInfoList node;
Rect r;
GrafPtr saveport;

GetPort(&saveport);
SetPortDialogPort(OMSinoutPtr);
SetRect(&r,70,10,210,26);

gInputMenu = NewOMSDeviceMenu(NULL,odmFrameBox,&r,
	omsIncludeInputs + omsIncludeReal + omsIncludeVirtual,NULL);
SetRect(&r,70,40,210,56);
gOutputMenu = NewOMSDeviceMenu(NULL,odmFrameBox,&r,
	omsIncludeOutputs + omsIncludeReal + omsIncludeVirtual,NULL);
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err SetOMSdriver(). saveport == NULL");
if(gInputMenu == NULL || gOutputMenu == NULL) {
	Alert1("Internal problem setting OMS input/output menus");
	return(ABORT);
	}

if(gChosenOutputID == 0) gChosenOutputID = FindOMSdevice(NO,OMSoutputName);
nodelist = OMSGetNodeInfo(omsIncludeReal+omsIncludeOutputs);
gOutNodeRefNum = OMSUniqueIDToRefNum(gChosenOutputID);
if(gOutNodeRefNum == OMSInvalidRefNum) {
	/* Select the first device available, if any */
	if(nodelist == NULL) {
		if(Beta) Alert1("Err SetOMSdriver(). Output 'nodelist' == NULL");
		gChosenOutputID = 0;
		}
	else {
		node = (*nodelist)[0];
		if(node.numNodes > 0) {
			gChosenOutputID = node.info[0].uniqueID;
			gOutNodeRefNum = OMSUniqueIDToRefNum(gChosenOutputID);
			if(gOutNodeRefNum != OMSInvalidRefNum) {
				MyPtoCstr(255,node.info[0].name,OMSoutputName);
				}
			}
		}
	}
/* else gChosenOutputID had been read in '-se.startup' and it was acceptable */
if(nodelist != NULL) OMSDisposeHandle(nodelist);

if(gChosenInputID == 0) gChosenInputID = FindOMSdevice(YES,OMSinputName);

if(gInputMenu != NULL && gChosenInputID > 0)
	SetOMSDeviceMenuSelection(gInputMenu,0,gChosenInputID,"\p",TRUE);
if(gOutputMenu != NULL && gChosenOutputID > 0)
	SetOMSDeviceMenuSelection(gOutputMenu,0,gChosenOutputID,"\p",TRUE);

OpenOrCloseConnection(TRUE,TRUE);
return(OK);
}


PushMIDIdata(unsigned char nbytes,unsigned char* data)
{
long newtop;
int i;

if(!OutMIDI && !Interactive && !ReadKeyBoardOn && !ScriptRecOn) return(FAILED);

newtop = BytesReceived + nbytes;
if(newtop >= MaxOMSinputBufferSize) {
	if(!DownBuffer || ((newtop - MaxOMSinputBufferSize) >= BytesProcessed)) {
		OMSinputOverflow = TRUE;
		return(FAILED);
		}
	}
else {
	if(!DownBuffer && newtop >= BytesProcessed) {
		OMSinputOverflow = TRUE;
		return(FAILED);
		}
	}
OMSinputOverflow = FALSE;
for(i=0; i < nbytes; i++) {
	(*h_OMSinputMessage)[BytesReceived].time = TotalTicks * CLOCKRES;
	(*h_OMSinputMessage)[BytesReceived].byte = data[i];
	(*h_OMSinputMessage)[BytesReceived].sequence = 0;
	BytesReceived++;
	if(BytesReceived >= MaxOMSinputBufferSize) {
		BytesReceived -= MaxOMSinputBufferSize;
		DownBuffer = FALSE;
		}
	}
return(OK);
}


PullMIDIdata(MIDI_Event *p_e)
{
if(DownBuffer && BytesProcessed >= BytesReceived) return(FAILED);
p_e->type = RAW_EVENT;
p_e->time = (*h_OMSinputMessage)[BytesProcessed].time; 
p_e->data2 = (*h_OMSinputMessage)[BytesProcessed].byte;
BytesProcessed++;
if(BytesProcessed >= MaxOMSinputBufferSize) {
	BytesProcessed -= MaxOMSinputBufferSize;
	DownBuffer = TRUE;
	}
return(OK);
}


short FindOMSdevice(int input,char* name)
{
OMSNodeInfoListH nodelist;
OMSNodeInfoList node;
short id,refnum,numnodes;
int i,im,j,k,len;

// This does not yet work fine because node.info[j].name is sometimes overridden by node.numNodes.
// Perhaps the OMS SDK is obsolete.

if(name[0] == '\0') return(0);

if(input)
	nodelist = OMSGetNodeInfo(omsIncludeInputs + omsIncludeReal + omsIncludeVirtual);
else
	nodelist = OMSGetNodeInfo(omsIncludeOutputs + omsIncludeReal + omsIncludeVirtual);
if(nodelist == NULL) return(0);

i = id = 0;
im = GetHandleSize((Handle)nodelist) / sizeof(OMSNodeInfoList);
for(i=0; i < im; i++) {
	node = (*nodelist)[i];
	if(node.numNodes <= 0 || node.numNodes > 5) continue;
	/* node.numNodes > 5 is temporary */
	for(j=0; j < node.numNodes; j++) {
		if(node.info[j].deviceH == NULL || (node.info[j].name)[0] == 0) break;
		k = 0;
		len = node.info[j].name[0];
		while (--len >= 0) {
			Message[k] = node.info[j].name[k+1];
			k++;
			}
		Message[k] = '\0';
		Strip(Message);
		if(strcmp(Message,name) == 0) {
			id = node.info[j].uniqueID;
			goto OUT;
			}
		}
	}
OUT:
OMSDisposeHandle(nodelist);
return(id);
}


short GetIDandName(char* line)
{
int i,j,n;
short id;
char c;

id = 0;
Strip(line);
if(line[0] == '\0' || line[0] == '<') return(0);
for(i=n=0; i < strlen(line); i++) {
	c = line[i];
	if(!isdigit(c)) break;
	n = (10 * n) + c - '0';
	}
id = (short) n;
while(MySpace(c=line[i]) && c != '\0') i++;
for(j=i; j < strlen(line); j++) {
	line[j-i] = line[j];
	}
line[j-i] = '\0';
return(id);
}


StoreDefaultOMSinput(void)
{
int rep;

if(!MustChangeInput) return(OK);

Alert1("Let us store the default MIDI input to '-se.startup', then restart BP2...");
if(mSaveStartup(0) == OK) {
	rep = Answer("Now you should quit and restart BP2...  OK",'Y');
	if(rep == YES) return(EXIT);
	}
else {
	rep = Answer("You refused to store the selected input. Try again later",'Y');
	if(rep != NO) {
		OpenOrCloseConnection(FALSE,FALSE);
		OMSinputName[0] = '\0';
		return(OK);
		}
	}
MustChangeInput = FALSE;
return(OK);
}