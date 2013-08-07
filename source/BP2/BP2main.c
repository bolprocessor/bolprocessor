/* BP2main.c (BP2 version CVS) */

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


#include "-BP2.h"
#include "-BP2main.h"

static int MakeNewDriverRecord(BPMidiDriver*** p_handle);
static int RegisterMidiDrivers(int oms);

int main (int argc, char* args[])
{
int i,w,startupscript,what,eventfound,rep,oms;
long leak;
EventRecord event;

if(Inits() != OK) return(OK);

TraceMemory = FALSE;
#if BIGTEST
TraceMemory = TRUE;
#endif

#ifdef __POWERPC
# if !TARGET_API_MAC_CARBON
    SetZone(ApplicationZone());
    // NewGrowZoneProc(MyGrowZone); /* This does not seem to be used anywhere on PPC - akozar */
# endif
#else
    SetGrowZone((GrowZoneProcPtr) MyGrowZone);
#endif

leak = 0;	/* Adjust for leak indication = 0 */


// The following two files are found in BP2's folder which was identified by
// GetProcessInformation() in Inits.c

if(OpenHelp() == ABORT) return(OK);
HideWindow(Window[wInfo]);
LoadSettings(TRUE,TRUE,TRUE,FALSE,&oms);

// Install time scheduler
#if WITH_REAL_TIME_SCHEDULER
if(NEWTIMER) {
	if(InstallTMTask() != OK) {
 		Alert1("Unknown problem installing time scheduler. May be this version of BP2 is obsolete with the current system");
		return(OK);
		}
	}
#endif

RegisterMidiDrivers(oms);

#if WITH_REAL_TIME_MIDI
if(SetDriver() != OK) goto END;
#endif

LoadMidiDriverStartup();

ResetTicks(TRUE,TRUE,ZERO,0);

#if !WASTE
for(w=0; w < WMAX; w++) {
	if(!Editable[w] || LockedWindow[w] || w == wStartString) continue;
	SetSelect(ZERO,GetTextLength(w),TEH[w]);
	TextDelete(w);
	}
#endif
ClearWindow(YES,wCsoundTables);

if(ResetScriptQueue() != OK) goto END;
CurrentDir = WindowParID[wScript];
CurrentVref = TheVRefNum[wScript];
startupscript = FALSE;

HideWindow(GetDialogWindow(GreetingsPtr));
MemoryUsedInit = MemoryUsed + leak;
ForceTextColor = ForceGraphicColor = 0;
BPActivateWindow(SLOW,wMessage);
{ Cursor arrow;
  SetCursor(GetQDGlobalsArrow(&arrow));
}

ClearWindow(TRUE,wInteraction);
ClearWindow(TRUE,wGlossary);
ClearWindow(TRUE,wScript);
/* At least one window must be active so that Apple Events are detected even if the... */
/* ... application is not active ! */
BPActivateWindow(SLOW,wMessage);
HideWindow(Window[wInfo]);

// Registration is no longer necessary -- 060506 akozar
// if(CheckRegistration() != OK) goto END;

// Let's have some fun in year 2000
// Y2K();  // not needed anymore :-) 010207 akozar

if(MustChangeInput) goto NOEVENT;

event.what = nullEvent;
for(i=0; i < 50; i++) {
	/* Identify and process the Apple Event that might have launched BP2 */
GETEVENT:
	eventfound = GetNextEvent(everyEvent,&event);
	if(!eventfound || event.what == activateEvt) continue;
	if(event.what == updateEvt) {
		DoEvent(&event); 
		goto GETEVENT;
		}
	if((event.what != kHighLevelEvent || !GoodEvent(&event))
		&& (event.what != keyDown || (event.modifiers & cmdKey) == 0)) continue;
	break;
	}
SessionTime = clock();
if(eventfound && ((event.what == keyDown && (event.modifiers & cmdKey) != 0)
		|| (event.what == kHighLevelEvent && GoodEvent(&event)))) {
	FlushEvents(everyEvent,0);
	InitOn = FALSE;
	HideWindow(Window[wInfo]);
	rep = DoEvent(&event);
	if(rep == EXIT) goto END;
	if(LoadedScript) {
		InitOn = FALSE;
		if(RunScript(wScript,TRUE) == EXIT) goto END;
		}
	if(ResetScriptQueue() != OK) goto END;
	}
if (ReceivedOpenAppEvent) {
NOEVENT:
	InitOn = FALSE;
	if(!MustChangeInput) {
		if (RunScriptInPrefsOrAppFolder(kBPStartupScript, &startupscript) != OK) goto END;
		ScriptExecOn = 0;
		}

	ShowMessage(TRUE,wMessage,"Type ‘-?’ and select button, word or menu item for help");
	if(!startupscript && !MustChangeInput) {
		ClearWindow(TRUE,wGrammar);
		ClearWindow(TRUE,wAlphabet);
		ClearWindow(TRUE,wData);
SHOWOPTIONS:
		StopWait();
		SndSetSysBeepState(sysBeepDisable);
		what = Alert(WorkAlert,0L);
		SndSetSysBeepState(sysBeepEnable);
		HideWindow(Window[wInfo]);
		switch(what) {
			case dData:
				LastEditWindow = wData;
				BPActivateWindow(SLOW,wData);
				BPActivateWindow(SLOW,wControlPannel);
				break;
			case dGrammars:
				LastEditWindow = wGrammar;
				BPActivateWindow(SLOW,wGrammar);
				BPActivateWindow(SLOW,wControlPannel);
				break;
			case dAlphabets:
				LastEditWindow = wAlphabet;
				BPActivateWindow(SLOW,wAlphabet);
				break;
			case dScripts:
				LastEditWindow = wScript;
				mScript(wScript);
				break;
			case dSoundObjects:
				mObjectPrototypes(wPrototype1);
				break;
			case dInteraction:
				LastEditWindow = wInteraction;
				BPActivateWindow(SLOW,wInteraction);
				break;
			case dGlossary:
				LastEditWindow = wGlossary;
				BPActivateWindow(SLOW,wGlossary);
				break;
			case dTimeBase:
				mTimeBase(wTimeBase);
				break;
			case dCsoundInstruments:
				mCsoundInstrumentsSpecs(wCsoundInstruments);
				break;
			case dFAQ:
				BPActivateWindow(SLOW,wData);
				mFAQ(0);
				break;
			case dLoadProject:
				LastEditWindow = wGrammar;
				if(mLoadProject(wGrammar) == OK)
					BPActivateWindow(SLOW,wControlPannel);
				else BPActivateWindow(SLOW,wGrammar);
				break;
			case dRegister:
				DisplayFile(wNotice,"License.txt");
			//	Register();
				break;
			case dQuitWork:
				goto END;
				break;
			}
		}
	}

START:
InitOn = FALSE;
SessionTime = clock();
HideWindow(Window[wInfo]);

if(MustChangeInput) {
	Alert1("Select a MIDI device on the OMS MIDI-input menu. Close window for no device…");
	mOMSinout(0);
	}

////////////////////////////
while(MainEvent() != EXIT && EventState != EXIT) {
	if(Beta && LoadOn > 0) {
		Alert1("Err. LoadOn > 0 ");
		LoadOn = 0;
		}
	}
////////////////////////////

if(!EmergencyExit && !MustChangeInput && !ScriptExecOn) {
	EventState = NO;  // necessary so that script will run
	RunScriptInPrefsOrAppFolder(kBPShutdownScript, &startupscript);
	}
StopWait();
/* KillSubTree(PrefixTree); KillSubTree(SuffixTree); $$$ */
MyDisposeHandle((Handle*)&Stream.code);
Stream.imax = ZERO;
Stream.period = ZERO;
LoadedCsoundInstruments = TRUE;
if(TraceMemory && Beta && !MustChangeInput) {  // replaced CheckMem with TraceMemory -- akozar 032707
	if(ResetProject(FALSE) != OK && !EmergencyExit) goto START;
	if(ClearWindow(FALSE,wData) != OK) goto START;
	ForgetFileName(wData);
	sprintf(Message,"This session used %ld Kbytes maximum.  %ld handles created and released. [%ld bytes leaked]",
		(long) MaxMemoryUsed/1000L,(long)MaxHandles,
		(long) (MemoryUsed - MemoryUsedInit));
	ShowMessage(TRUE,wMessage,Message);
	if(MemoryUsed != MemoryUsedInit) {
		sprintf(Message,"%ld bytes leaked",(long) (MemoryUsed - MemoryUsedInit));
		Alert1(Message);
		}
	}
StopWait();
if(!ScriptExecOn && !AEventOn && !EmergencyExit && !MustChangeInput) {
	if(Beta && mAbout(wUnknown) == RESUME) {  // only display About in beta builds -- akozar 060107
		InitButtons();
		goto START;
		}
	if(MaxTempMemoryUsed > ZERO) {
		MaxTempMemoryUsed = 1L + (MaxTempMemoryUsed / 1000000L);
		sprintf(Message,
			"Additional memory was required during this session. You should give BP2 at least %ld more Mbyte(s)",
			(long)MaxTempMemoryUsed);
		Alert1(Message);
		}
	}

END:

CloseMIDIFile();
CloseMe(&HelpRefnum);
CloseFileAndUpdateVolume(&TraceRefnum);
CloseFileAndUpdateVolume(&TempRefnum);
CloseCsScore();
MyDisposeHandle((Handle*)&p_Oldvalue);
ClearLockedSpace();

#if USE_OMS
  if(Oms) ExitOMS();
#endif
#if USE_BUILT_IN_MIDI_DRIVER /* WITH_REAL_TIME_MIDI */
  // FIXME: CloseCurrentDriver should eventually work for all drivers - akozar
  if(InBuiltDriverOn) CloseCurrentDriver(FALSE);
#endif
#if WITH_REAL_TIME_SCHEDULER
  if(NEWTIMER) RemoveTMTask();
#endif

#if USE_MLTE
TXNTerminateTextension();
#endif

return(EXIT_SUCCESS);
}

/* Registering Devices, Drivers and their interfaces */

static int MakeNewDriverRecord(BPMidiDriver*** p_handle)
{
BPMidiDriver** driver;
driver = (BPMidiDriver**) GiveSpace(sizeof(BPMidiDriver));
if (!driver) return(FAILED);

(*driver)->id = ++NumInstalledDrivers;
(*driver)->name[0] = '\0';
(*driver)->initOK = false;
(*driver)->settingsDialog = NULL;
(*driver)->firstMItem = 0;
(*driver)->lastMItem = 0;
(*driver)->next = InstalledDrivers;

InstalledDrivers = driver;
*p_handle = driver;
return(OK);
}

static int RegisterMidiDrivers(int oms)
{
BPMidiDriver** driver;
OSErr io;

/* We can only handle certain statically linked drivers for now,
   but eventually will support dynamically loading driver plugins. */

// OMS initialisation
#if USE_OMS
// if (MakeNewDriverRecord(&driver) != OK) return(FAILED);
// strcpy((*driver)->name, "Opcode OMS");
if(oms) {
	if(InitOMS('Bel0') == noErr) {
		Oms = TRUE;
		ChangeControlValue(TRUE,Hbutt[bOMS],Oms);
		}
	else Alert1("To avoid OMS warning when using internal MIDI driver, change settings ‘-se.startup’ after launching BP2");
	ClearMessage();
	}
#else
  Oms = FALSE;
  ChangeControlValue(TRUE,Hbutt[bOMS],Oms);
#endif

PleaseWait();
	
#if USE_BUILT_IN_MIDI_DRIVER
// if (MakeNewDriverRecord(&driver) != OK) return(FAILED);
// strcpy((*driver)->name, "BP2 Serial Driver");
if(!Oms && (io = DriverOpen("\p.MIDI")) != noErr) {
	Alert1("Unexpected error opening MIDI driver. OMS is off, but some other device might be conflicting");
	return(OK);
}
#else
// no real-time MIDI ... what should we do?? - akozar 010307
#endif

#if BP_MACHO
//	if (MakeNewDriverRecord(&driver) != OK) return(FAILED);
//	strcpy((*driver)->name, "CoreMIDI");
	InitCoreMidiDriver();
#endif

return (OK);
}


// --------------------  MEMORY MANGEMENT -------------------------

#ifndef __POWERPC
pascal long MyGrowZone(Size size)
// This doesn't seem to work any more since system 7.5.3
{
long result;
long oldA5;

oldA5 = SetCurrentA5();

result = GetMoreSpace(size);

SetA5(oldA5);

return(result);
}
#endif

int **GiveSpace(Size size)
{
int **p,rep;
OSErr err,memerr;
long totalbytes,contigbytes,grow,dummy;

TRY:
if(!RunningOnOSX && TempMemory) { // temp mem functions the same on X as non-temp mem - akozar
	/* We are using memory outside BP2's partition */
	SchedulerIsActive--;
	totalbytes = TempFreeMem();
	if(size > totalbytes)
		contigbytes = TempMaxMem(&dummy);
	else contigbytes = size;
	if(contigbytes < size) {
		EmergencyExit = TRUE;
		Alert1("BP2 didn't get enough additional memory. (Insufficient RAM?) Save your work and quit");
		SchedulerIsActive++;
		TempMemory = FALSE;
		TempMemoryUsed = ZERO;
		return(NULL);
		}
	p = (int**) TempNewHandle(size,&err);
	SchedulerIsActive++;
	if(p == NULL || err != noErr) {
		EmergencyExit = TRUE;
		Alert1("BP2 didn't get enough additional memory. (Insufficient RAM?) Save your work and quit");
		TempMemory = FALSE;
		TempMemoryUsed = ZERO;
		return(NULL);
		}
	}
else {
	if(!RunningOnOSX && LowOnMemory) { // FreeMem() and MaxMem() aren't very useful on X - akozar
		totalbytes = FreeMem();
		grow = ZERO;
		contigbytes = size;
		if(size > totalbytes) contigbytes = MaxMem(&grow);
		if((contigbytes + grow) < size) {
			rep = GetMoreSpace(size);
			if(rep == OK) goto TRY;
			else return(NULL);
			}
		}
	p = (int**) NewHandle(size);
	}

if(p == NULL || /*(Beta &&*/ ((memerr=MemError()) != noErr)) { // should always check MemError - akozar
	if (!RunningOnOSX) { // on OS 9, we might be able to find more space
		rep = GetMoreSpace(size);
		if(rep == OK) goto TRY;
		else return(NULL);
		}
	else { // on OS X, this is pretty much the end of the road ?? - akozar 040207
		Alert1("BP2 ran out of memory before completing the current task."
			 "You may want to save your work before continuing.");
		return NULL;
		}
	}
else {
	MaxHandles++;
	MemoryUsed += (unsigned long) size;
	if(MemoryUsed > MaxMemoryUsed) {
		MaxMemoryUsed = MemoryUsed;
		}
	if(TempMemory) {
		TempMemoryUsed += (unsigned long) size;
		if(TempMemoryUsed > MaxTempMemoryUsed) {
			MaxTempMemoryUsed = TempMemoryUsed;
			}
		}
	if((MaxHandles % 200) == 0) {
		if(TempMemory) FlashInfo("••• Using additional memory…");
		else if(LowOnMemory) FlashInfo("••• Low on memory… Reset project as soon as possible!");
		}
	}
return(p);
}


/* CheckGrowingHandle() assesses how much memory is free to determine
   whether p_h can grow to the new desired size.  The logic here may not
   always accurately predict whether enough memory is available because
   it does not take into account that the handle may need to be moved
   to be resized (i.e. comparing available memory to 'resize' is not enough
   to guarantee that memory can by combined with this handle.
   If CheckGrowingHandle() does not think there is enough memory in the
   application heap (or we already exhausted that route), then it tries
   using temporary memory (with the user's permission).
   
   On OS X, this is all pretty much irrelevant, so we just return OK.
 */
CheckGrowingHandle(Handle *p_h,Size oldsize,Size size)
{
long contigbytes,grow,totalbytes,rep,dummy,resize;
Handle p;
OSErr memerr;

if (RunningOnOSX || size < oldsize) return(OK);

resize = size - oldsize;

if(!TempMemory) {
	totalbytes = FreeMem();
	grow = ZERO;
	contigbytes = resize;
	if(resize > totalbytes) {
		contigbytes = MaxMem(&grow);
		}
	if((contigbytes + grow) < resize) {
		rep = GetMoreSpace(resize);
		if(rep != OK) return(ABORT);
		if(TempMemory) {
			/* Move the current handle to the temp memory */
			SchedulerIsActive--;
			p = (Handle) TempNewHandle(ZERO,&memerr);
			SchedulerIsActive++;
			if(p == NULL || memerr != noErr) {
				EmergencyExit = TRUE;
				Alert1("BP2 didn't get enough additional memory. (Insufficient RAM?) Save your work and quit");
				TempMemory = FALSE;
				TempMemoryUsed = ZERO;
				return(ABORT);
				}
			HLock(*p_h);
			memerr = HandAndHand(*p_h,p);
			HUnlock(*p_h);
			if(memerr != noErr) {
				TellError(28,memerr);
				if(Beta && !InitOn) Alert1("HandAndHand() returned error in MySetHandleSize()");
				TempMemory = FALSE;
				TempMemoryUsed = ZERO;
				return(ABORT);
				}
			DisposeHandle(*p_h);
			(*p_h) = p;
			}
		}
	}
else {
	totalbytes = TempFreeMem();
	if(resize > totalbytes)
		contigbytes = TempMaxMem(&dummy);
	else contigbytes = resize;
	if(contigbytes < resize) {
		EmergencyExit = TRUE;
		Alert1("BP2 didn't get enough additional memory. (Insufficient RAM?) Save your work and quit");
		SchedulerIsActive++;
		TempMemory = FALSE;
		TempMemoryUsed = ZERO;
		return(ABORT);
		}
	}
return(OK);
}


Size MyGetHandleSize(Handle h)
{
if(h == NULL) return((Size) ZERO);
else return(GetHandleSize(h));
}


MyDisposeHandle(Handle *p_h)
{
Size size;
int r;
OSErr memerr;

r = OK;
if(*p_h != NULL) {
	size = MyGetHandleSize(*p_h);
	if(size < 1L) {
		if(!EmergencyExit && Beta) Alert1("Err. MyDisposeHandle. size < 1L");
		*p_h = NULL;
		return(ABORT);
		}
	DisposeHandle(*p_h);
	if(!EmergencyExit && Beta && ((memerr = MemError()) != noErr)) {
		TellError(29,memerr);
		if(Beta) Alert1("Memory error in MyDisposeHandle()");
		r = ABORT;
		}
	else MemoryUsed -= (unsigned long) size;
	if(TempMemory) TempMemoryUsed -= (unsigned long) size;
	}
*p_h = NULL;
return(r);
}


Handle IncreaseSpace(Handle h)
{
Size oldsize,newsize;
int rep;
OSErr memerr;

TRY:
if(h == NULL) {
	if(Beta) Alert1("Err. IncreaseSpace(). h = NULL");
	return(NULL);
	}
oldsize = MyGetHandleSize(h);
newsize = 2L + ((oldsize * 3L) / 2L);
if(CheckGrowingHandle(&h,oldsize,newsize) != OK) return(NULL);

MemoryUsed += (newsize - oldsize);
if(MemoryUsed > MaxMemoryUsed) {
	MaxMemoryUsed = MemoryUsed;
	}
if(TempMemory) {
	TempMemoryUsed += (newsize - oldsize);
	if(TempMemoryUsed > MaxTempMemoryUsed) {
		MaxTempMemoryUsed = TempMemoryUsed;
		}
	}
SchedulerIsActive--;
SetHandleSize(h,newsize);  // FIXME: this could fail if the handle is locked and we are not really too low on memory - akozar
SchedulerIsActive++;

if((memerr=MemError()) != noErr) {
	if (!RunningOnOSX) { // on OS 9, we might be able to find more space
		rep = GetMoreSpace(newsize);
		if(rep == OK) goto TRY;
		else return(NULL);
		}
	else { // on OS X, this is pretty much the end of the road ?? - akozar 040207
		Alert1("BP2 ran out of memory before completing the current task."
			 "You may want to save your work before continuing.");
		return NULL;
		}
	}
return(h);
}


MySetHandleSize(Handle* p_h,Size size)
{
Size oldsize;
OSErr memerr;
int rep;

TRY:
if(p_h == NULL) {
	sprintf(Message,"Err. MySetHandleSize(). p_h == NULL");
	if(Beta) Alert1(Message);
	return(ABORT);
	}
if((*p_h) == NULL) oldsize = ZERO;
else {
	oldsize = MyGetHandleSize(*p_h);
	}
if((*p_h) != NULL && oldsize > ZERO) {
	if(CheckGrowingHandle(p_h,oldsize,size) != OK) return(ABORT);
	SchedulerIsActive--;
	SetHandleSize(*p_h,size);  // FIXME: this could fail if the handle is locked and we are not really too low on memory - akozar
	SchedulerIsActive++;
	}
else {
	if(Beta && (*p_h) != NULL && !InitOn) {
		sprintf(Message,"Err. MySetHandleSize(). oldsize = %ld",
			(long) oldsize);
		Alert1(Message);
		}
	if(((*p_h) = (Handle) GiveSpace(size)) == NULL) return(ABORT);
	}

if((memerr=MemError()) == noErr) {
	MemoryUsed += (unsigned long)(size - oldsize);	// FIXME ? oldsize can be > size (but seems to work anyways?)
	if(MemoryUsed > MaxMemoryUsed) {
		MaxMemoryUsed = MemoryUsed;
		}
	if(TempMemory) {
		TempMemoryUsed += (unsigned long)(size - oldsize);
		if(TempMemoryUsed > MaxTempMemoryUsed) {
			MaxTempMemoryUsed = TempMemoryUsed;
			}
		}
	return(OK);
	}
else {
	if (!RunningOnOSX) { // on OS 9, we might be able to find more space
		rep = GetMoreSpace(size);
		if(rep == OK) goto TRY;
		else return(ABORT);
		}
	else { // on OS X, this is pretty much the end of the road ?? - akozar 040207
		Alert1("BP2 ran out of memory before completing the current task."
			 "You may want to save your work before continuing.");
		return (ABORT);
		}
	}
}


IsMemoryAvailable(long memrequest)	/* Not used here */
{
long total,contig;

if (RunningOnOSX)  return (TRUE);
if(!IsEmergencyMemory()) return(FALSE);
PurgeSpace(&total,&contig);
if(memrequest > /* EMERGENCYMEMORYSIZE + */ contig) return(FALSE);
else return(TRUE);
}


IsEmergencyMemory(void)
{
return((h_EmergencyMemory != NULL) && (*h_EmergencyMemory != NULL)
	/* First condition: created; second case: allocated */
	&& !LowOnMemory && !TempMemory && !InitOn);
}


RecoverEmergencyMemory()
{
if(RunningOnOSX || (h_EmergencyMemory != NULL) && (*h_EmergencyMemory != NULL)) return(OK);
ShowMessage(TRUE,wMessage,"Recovered emergency memory…");
// h_EmergencyMemory = NewHandle(EMERGENCYMEMORYSIZE);
ReallocateHandle(h_EmergencyMemory,EMERGENCYMEMORYSIZE);
if((h_EmergencyMemory == NULL) ||  (*h_EmergencyMemory == NULL))
	return(ABORT);
LowOnMemory = FALSE;
return(OK);
}


CheckMemory(void)
{
int r;

if((r=DoSystem()) != OK) return(r);
return(OK);
}


ThreeOverTwo(long *p_x)
{
long y;

y = ((*p_x) * 3L) / 2L;
*p_x = y;
return(OK);
}

int MyLock(int high,Handle h)
{
OSErr memerr;

if(h == NULL) {
	if(Beta) Alert1("Attempted to lock NIL handle");
	return(ABORT);
	}
if(high) HLockHi(h);
else HLock(h);
if((memerr=MemError()) != noErr) {
	TellError(30,memerr);
	if(Beta) Alert1("Error locking handle");
	return(ABORT);
	}
return(OK);
}


int MyUnlock(Handle h)
{
OSErr memerr;

if(h == NULL) {
	if(Beta) Alert1("Attempted to unlock NIL handle");
	return(ABORT);
	}
HUnlock(h);
if((memerr=MemError()) != noErr) {
	TellError(31,memerr);
	if(Beta) Alert1("Error unlocking handle");
	return(ABORT);
	}
return(OK);
}


/* GetMoreSpace() just releases the small reserve in h_EmergencyMemory
   and asks the user if they want to use temporary memory if this is
   not enough (but does not allocate temp mem).
   This is irrelevant on OS X (and h_EmergencyMemory is NULL). */
GetMoreSpace(Size size)
{
long contigbytes,grow;
int rep,result;

if (RunningOnOSX)  return (OK);

SchedulerIsActive--;
if(IsEmergencyMemory() && (h_EmergencyMemory != GZSaveHnd())) {
	EmptyHandle(h_EmergencyMemory);
	LowOnMemory = TRUE;
	contigbytes = MaxMem(&grow);
	size += 50000L;
	if((contigbytes+grow) < size) {
		if(AskedTempMemory) goto USEIT;
		sprintf(LineBuff,"Memory is low");
		if(SelectOn) sprintf(LineBuff,"Need more memory for playing selection");
		if(ComputeOn) sprintf(LineBuff,"Need more memory for computation");
		if(CompileOn) sprintf(LineBuff,"Need more memory for compilation");
		if(SetTimeOn) sprintf(LineBuff,"Need more memory for time setting");
		if(PolyOn) sprintf(LineBuff,"Need more memory for polymetric expansion");
		if(GraphicOn) sprintf(LineBuff,"Need more memory for graphics");
		if(PrintOn) sprintf(LineBuff,"Need more memory to print");
		if(SoundOn) sprintf(LineBuff,"Need more memory to play sound");
		sprintf(Message,"%s. %s",LineBuff,"Use additional memory (otherwise task will be abandoned)");
NOSPACE:
		rep = Answer(Message,'Y');
		if(rep != YES) {
			if((rep=Answer("Do you really want to abort this job",'N')) == YES)
				goto FORGETITALL;
			else goto NOSPACE;
			}
		FlashInfo("Using additional memory…");
		if(!ScriptExecOn && ShowGraphic)
			Alert1("Graphics will not be displayed because of lack of memory");
		ShowWindow(Window[wTimeAccuracy]);
		BringToFront(Window[wTimeAccuracy]);
		// since updating allocates memory, it is not essential right now -- akozar 040207
		// BPUpdateDialog(gpDialogs[wTimeAccuracy]);
		if(!ScriptExecOn)
			Alert1("You should size up the memory allocation for BP2, or increase the quantization");
USEIT:		
		AskedTempMemory = TempMemory = TRUE;
		result = OK;
		}
	else result = OK;
	}
else {
	if(!TempMemory) {
		if(AskedTempMemory) goto USEIT;
		sprintf(Message,"Use additional memory (otherwise task will be abandoned)");
		goto NOSPACE;
		}
/* Here we could try other schemes for recovering memory */
	Alert1("Out of memory. Task abandoned. Save your work and quit!");
	EmergencyExit = TRUE;
	
FORGETITALL:
	TempMemory = FALSE;
	TempMemoryUsed = ZERO;
	result = NO;
	}
	
SchedulerIsActive++;
return(result);
}