/* Processes.c (BP2 version 2.9.4) */

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

// The current application refused to abandon its current process. Let's notify the user… 
SndSetSysBeepState(sysBeepEnable);

// In the following some features don't work: dialog and sound
myNote.qType = nmType;	//queue type -- nmType = 8
myNote.nmMark = 1;	//get mark in Apple menu

icon = GetResource('ics8',BP2smallIconID);
myNote.nmIcon = icon;

//get the sound you want out of your resources
sound = (SndListHandle) GetResource(soundListRsrc,kclocID);
myNote.nmSound = (Handle) sound;	//set the sound to be played

myNote.nmStr = message;

#ifndef powerc
myNote.nmResp = NULL;	//no response procedure
myNote.nmRefCon = SetCurrentA5();
#else
myNote.nmResp = (RoutineDescriptor*) NULL;	//no response procedure
#endif

if(howmany > 0 && sound != NULL) {
	SndNewChannel(&myChan,0,0,0L);
	HLock((Handle)sound);
	for(i=0; i < howmany; i++) {
		SndPlay(myChan,sound,FALSE);
		}
	HUnlock((Handle)sound);
	SndDisposeChannel(myChan,FALSE);
	}

#ifndef powerc
idle = MyIdleFunction;
#else
idle = NewRoutineDescriptor((ProcPtr)MyIdleFunction,uppAEIdleProcInfo,
			GetCurrentArchitecture());
#endif
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
		sprintf(Message,"BP2 is unable to open ‘%s’",name);
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
strcpy(Message,"Register");
pStrCopy((char*)c2pstr(Message),spec.name);

return(LaunchAnApplication(spec));
}