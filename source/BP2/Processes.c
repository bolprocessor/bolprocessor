/* Processes.c (BP2 version CVS) */

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


#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"


// Application global variable to keep track of cursor rgn
RgnHandle gCursorRgn;

pascal Boolean MyIdleFunction(EventRecord *p_event,long *sleepTime,
	RgnHandle *mouseRgn)
{

	// Function Prototypes
/*	Boolean 	MyCancelInQueue(void); */
//	void 	DoIdle (void);
//	void 	AdjustCursor (Point where, RgnHandle cursRgn);
//	void 	DoEvent (EventRecord*);

	// the MyCancelInQueue function checks for Command-period
/*	if(MyCancelInQueue())
		return TRUE; */

switch(p_event->what) {
	case updateEvt:
	case activateEvt: 	// every idle function should handle
	case app4Evt:	// these kinds of events
//		AdjustCursor(p_event->where, gCursorRgn);
		MaintainCursor();
		MaintainMenus();
		DoEvent(p_event);
		break;
	case nullEvent:
		// set the   sleeptime and mouseRgn parameters
		*mouseRgn = gCursorRgn;
		*sleepTime = 10; // use the correct value for your app
//		DoIdle();  // the application's idle handling
		break;
	}
return FALSE;
}


CallUser(int howmany)
{
int i;
OSErr ierr;
AEIdleUPP idle;
NMRec myNote;
Handle icon;
static Str255 message = "\pPlease bring Bol Processor to front";
SndChannelPtr myChan = 0L;
SndListHandle sound;
ProcessSerialNumber psn;

GetFrontProcess(&psn);
if(psn.highLongOfPSN == PSN.highLongOfPSN && psn.lowLongOfPSN == PSN.lowLongOfPSN)
	return(OK);

ierr = SetFrontProcess(&PSN);
if(Beta && ierr!= noErr) TellError(87,ierr);

GetFrontProcess(&psn);
if(psn.highLongOfPSN != PSN.highLongOfPSN || psn.lowLongOfPSN != PSN.lowLongOfPSN) {
	for(i=0; i < 5; i++) {
		WaitABit(500L);
		/* Give some time to current 'psn' process to call WaitNextEvent() */
		GetFrontProcess(&psn);
		if(psn.highLongOfPSN == PSN.highLongOfPSN && psn.lowLongOfPSN == PSN.lowLongOfPSN)
			return(OK);
		}
	}

// The current application refused to abandon its current process. Let's notify the userÉ 
SndSetSysBeepState(sysBeepEnable);
icon = NULL; sound = NULL;

/* // None of this is used - 011907 akozar
// In the following some features don't work: dialog and sound
myNote.qType = nmType;	//queue type -- nmType = 8
myNote.nmMark = 1;	//get mark in Apple menu

icon = GetResource('ics8',BP2smallIconID);
myNote.nmIcon = icon;
*/

//get the sound you want out of your resources
sound = (SndListHandle) GetResource(soundListRsrc,kclocID);

/* // None of this is used - 011907 akozar
myNote.nmSound = (Handle) sound;	//set the sound to be played

myNote.nmStr = message;

#ifndef powerc
myNote.nmResp = NULL;	//no response procedure
myNote.nmRefCon = SetCurrentA5();
#else
myNote.nmResp = (RoutineDescriptor*) NULL;	//no response procedure
#endif
*/

if(howmany > 0 && sound != NULL) {
	SndNewChannel(&myChan,0,0,0L);
	HLock((Handle)sound);
	for(i=0; i < howmany; i++) {
		SndPlay(myChan,sound,FALSE);
		}
	HUnlock((Handle)sound);
	SndDisposeChannel(myChan,FALSE);
	}

/* // None of this is used - 011907 akozar
#ifndef powerc
idle = MyIdleFunction;
#else
idle = NewRoutineDescriptor((ProcPtr)MyIdleFunction,uppAEIdleProcInfo,
			GetCurrentArchitecture());
#endif */
idle = NULL;	/* It works but it doesn't seem useful now */

// We allow the user to bring BP2 to foreground within 2400 ticks
ierr = AEInteractWithUser(2400L,NIL /* &myNote */,idle);

if(icon != NULL) ReleaseResource(icon);
if(sound != NULL) ReleaseResource((Handle) sound);

return(ierr == noErr);
}


OpenApplication(OSType thesignature)
{
FSSpec spec;
char csign[5];
int i;

for(i=3; i >= 0; i--) {
	csign[3-i] = ((unsigned long)(thesignature >> (8 * i))) & 0xff;
	}
csign[4] = '\0';

if(FindApplication(thesignature,RefNumbp2,&spec) == OK
		|| FindApplication(thesignature,0,&spec) == OK
		|| FindApplication(thesignature,RefNumStartUp,&spec) == OK
		|| FindApplication(thesignature,kOnSystemDisk,&spec) == OK)
	return(LaunchAnApplication(spec));
else 
	sprintf(Message,
		"Application '%s' cannot be found. Perhaps you need to rebuild the desktop",
		csign);
	ShowMessage(TRUE,wMessage,Message);
	return(FAILED);
}


FindApplication(OSType thesignature,short vrefnum,FSSpec* p_spec)
{
// See "Finder Interface" in Think Reference

OSErr io;
DTPBRec pb;

pb.ioCompletion = ZERO;
pb.ioVRefNum = vrefnum;
pb.ioIndex = 0; /* This is probably not required */

io = PBDTGetPath(&pb);
// io = PBDTGetInfoSync(&pb);  This was the wrong call !!!
if(io != noErr) goto BAD;

pb.ioCompletion = ZERO;
pb.ioFileCreator = thesignature;
pb.ioFileType = 'APPL';
pb.ioNamePtr = p_spec->name;
pb.ioIndex = 0;

io = PBDTGetAPPLSync(&pb);

if(io == noErr && pb.ioResult == noErr) {
	p_spec->vRefNum = vrefnum;
	p_spec->parID = pb.ioAPPLParID;
	return(OK);
	}
else {
BAD:
	return(FAILED);
	}
}


LaunchAnApplication(FSSpec spec)
{
LaunchParamBlockRec myLaunchParams;
ProcessSerialNumber launchedProcessSN;
OSErr launchErr;
char name[MAXNAME+1];

SetCursor(&WatchCursor);
myLaunchParams.launchBlockID = extendedBlock;
myLaunchParams.launchEPBLength = extendedBlockLen;
myLaunchParams.launchFileFlags = 0;
myLaunchParams.launchControlFlags = launchContinue + launchNoFileFlags;
myLaunchParams.launchAppSpec = &spec;
myLaunchParams.launchAppParameters = nil;

launchErr = LaunchApplication(&myLaunchParams);

StopWait();
if(!launchErr) {
	launchedProcessSN = myLaunchParams.launchProcessSN;
	return(OK);
	}
else {
	MyPtoCstr(MAXNAME,spec.name,name);
	if(name[0] != '\0')
		sprintf(Message,"BP2 is unable to open '%s'",name);
	else
		sprintf(Message,"BP2 is unable to open application with unknown name");
	Alert1(Message);
	return(FAILED);
	}
}


Register(void)
/* Launch KAGI "Register" */
{
FSSpec spec;

spec.vRefNum = RefNumbp2;
spec.parID = ParIDbp2;
c2pstrcpy(spec.name, "Register");

return(LaunchAnApplication(spec));
}


#if BP_MACHO
/* Code for launching applications on OS X */

#include <ApplicationServices/ApplicationServices.h>	// for <LaunchServices/LaunchServices.h>
int LaunchOSXApplication(OSType signature)
{
	OSStatus err;
	FSRef outAppRef;
	
	err = LSGetApplicationForInfo(kLSUnknownType, signature, nil, kLSRolesAll, &outAppRef, NULL);
	if (err != noErr) return(FAILED);
	
	err = LSOpenFSRef(&outAppRef, NULL);
	if (err != noErr) return(FAILED);

	return (OK);	
}

#endif
