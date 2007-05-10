/* CoreMIDIdriver.c (BP2 version CVS) */

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

// This file should be the only one that includes CoreMIDI calls

#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"

#include <stdlib.h>
#include <string.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreMIDI/CoreMIDI.h>
#include <CoreFoundation/CFString.h>

typedef int (*CompareFuncType)(const void *, const void *);  // for qsort()

/* trying the pthread library for synchronization for now */
#include <pthread.h>

static Boolean		CoreMidiOutputOn = false;	// true when we have an output port & CMActiveDestinations != NULL
static Boolean		CoreMidiInputOn = false;	// true when we have an input port & CMActiveSources is not empty
static Boolean		CMMidiThruOn = false;
static MIDIPortRef	CMOutPort = NULL;
static MIDIEndpointRef*	CMActiveDestinations = NULL;	// NULL-terminated array of destinations
static Size			CMActiveDestinationsSize = 8;	// allocated size of array; NOT the number of used elements
static MIDIPortRef	CMInPort = NULL;
static MIDIEndpointRef*	CMActiveSources = NULL;		// NULL-terminated array of sources
static Size			CMActiveSourcesSize = 8;	// allocated size of array; NOT the number of used elements	
static MIDIClientRef	CMClient = NULL;

static MIDITimeStamp	ClockZero = 0;		// CoreAudio's host time that we consider "zero"

static Size			QueueSize = 5000L;
static Boolean		QueueOverflowed = false;
static Boolean		QueueEmpty = true;
static MIDIcode*		pInputQueue = NULL;	// beginning of allocated space for storing MIDI bytes
static MIDIcode*		pInputQueueEnd = NULL;	// one past the last allocated space for storing MIDI bytes
static MIDIcode*		pQueueFront = NULL;	// current location to remove bytes
static MIDIcode*		pQueueBack = NULL;	// current location to add bytes
static pthread_mutex_t	QueueMutex;

typedef struct {
	MIDIEndpointRef	endpoint;
	MIDIUniqueID	id;
	char**		name; 		// handle to C string
	Boolean		selected;		// currently selected by user
	Boolean		offline;		// previously existed; but not currently registered with CoreMIDI
	// Boolean		stillexists;	// we use this while updating the list contents
} CMListEntry;

typedef struct {
	ListHandle		list;
	CMListEntry**	entries;		// handle to an array of list entry structs
	Size			entriesSize;	// size of the array 'entries' (in CMListEntry's, not bytes)
	Size			numEntries;		// number of entries in use
} CMListData;

DialogPtr			CMSettings = NULL;
CMListData**		CMInputListData = NULL;
CMListData**		CMOutputListData = NULL;

const unsigned long	MAXENDPOINTNAME = 128;	// this is our own max; no guarantees from CoreMIDI!
const unsigned long	SUFFIXSIZE = 10;
const char			OfflineSuffix[SUFFIXSIZE+1] = " (offline)";

OSStatus CMCreateAndInitQueue();
OSStatus CMResizeQueue();
void CMReInitQueue();
void CMDestroyQueue();

OSStatus CMConnectToSources(MIDIEndpointRef* endpoints, Boolean tellConnections);
OSStatus CMDisconnectFromSources(MIDIEndpointRef* endpoints);
void CMNotifyCallback(const MIDINotification *message, void *refCon);
void CMReadCallback(const MIDIPacketList* pktlist, void* readProcRefCon, void* srcConnRefCon);
int  CMAddEventToQueue(const MIDIPacket* pkt);
int  CMRemoveEventFromQueue(MIDI_Event* p_e);

void ResizeListBox(ListHandle list, long numRows);
CMListData** NewListDataHandle(ListHandle list, Size entriesSize);
void MarkAllOffline(CMListData* ld);
void MarkAllNotSelected(CMListData* ld);
CMListEntry* FindListEntryByID(CMListData* ld, MIDIUniqueID id);
CMListEntry* FindListEntryByName(CMListData* ld, char* endname);
CMListEntry* FindListEntryByRef(CMListData* ld, MIDIEndpointRef endpt);
int  GetNewListEntry(CMListData* ld, CMListEntry** p_entryptr);
int  InitNewListEntry(CMListData* ld, CMListEntry** p_entryptr, MIDIEndpointRef endpt, MIDIUniqueID endID, 
                        char* endname, Boolean selected, Boolean offline);
int  CompareEntryNames(const CMListEntry* a, const CMListEntry* b);
void ResizeListBox(ListHandle list, long numRows);
int  UpdateListBoxEntries(CMListData** ldh, ItemCount (*countFunc)(void), MIDIEndpointRef (*getEndpointFunc)(ItemCount));
int  SelectListBoxItem(CMListData** ldh, Size itemnumber);
int  GetListBoxSelection(CMListData** ldh);
int  SetListBoxContentsAndSelection(CMListData** ldh);
int  UpdateCMSettingsListBoxes();
int  UpdateActiveEndpoints(CMListData** ldh, MIDIEndpointRef** endpoints, Size* epArraySize, Boolean* status);

/* Increment SETTINGS_FILE_VERSION for each change to the CoreMIDI settings file format.
   Change OLDEST_COMPATIBLE_VERSION to the current version whenever making an incompatible change.
   Compatible changes including adding extra fields to the end of the file or reinterpreting
   an existing field but continuing to write that field in a way that older versions will understand */
const int	SETTINGS_FILE_VERSION = 1;	 // the current version of the CoreMIDI settings file format
const int	OLDEST_COMPATIBLE_VERSION = 1; // the oldest version of the format that the current version is compatible with

int WriteActiveEndpoints(CMListData** ldh, short refnum);
int WriteCoreMIDISettings(short refnum);
int ReadActiveEndpoints(CMListData** ldh, short refnum, long* pos);
int ReadCoreMIDISettings(short refnum, long* pos);

/*  Returns whether the CoreMIDI driver is on */
Boolean IsMidiDriverOn()
{
	return (CoreMidiOutputOn || CoreMidiInputOn);
}


/* CMCreateAndInitQueue() should be called before creating a CoreMIDI input port */
static OSStatus CMCreateAndInitQueue()
{
	OSStatus err;
	// create queue buffer for receiving messages
	pInputQueue = (MIDIcode*) NewPtr(QueueSize * sizeof(MIDIcode));
	err = MemError();
	if (pInputQueue != NULL && err == noErr)  {
		err = pthread_mutex_init(&QueueMutex, PTHREAD_MUTEX_NORMAL);
		if (err == 0)  CMReInitQueue();
	}
	return err;
}

/* CMResizeQueue() should only be called by CMReInitQueue() or another
   queue function that has already obtained the mutex lock. */
static OSStatus CMResizeQueue()
{
	OSStatus  err;
	MIDIcode* newqueue;
	Size      newsize;
	
	// attempt to increase queue size by 50% before releasing the old one
	newsize = QueueSize * 3 / 2;
	newqueue = (MIDIcode*) NewPtr(newsize * sizeof(MIDIcode));
	err = MemError();
	if (newqueue != NULL && err == noErr)  {
		DisposePtr((Ptr)pInputQueue);
		pInputQueue = newqueue;
		QueueSize = newsize;
	}
	return err;
}

static void CMReInitQueue()
{
	if (pthread_mutex_lock(&QueueMutex) != 0)  return;
	
	// resize queue if we exceeded its size at some point
	if (QueueOverflowed)  CMResizeQueue();
	
	// reinitialize all queue variables
	if (pInputQueue != NULL) {
		pQueueFront = pQueueBack = pInputQueue;
		pInputQueueEnd = pInputQueue + QueueSize;
		QueueEmpty = true;
		QueueOverflowed = false;
	}
	pthread_mutex_unlock(&QueueMutex);
	return;
}

/* Should only be called when there is no possibility of the CoreMIDI 
   ReadCallback being called (i.e. driver input is off) */
static void CMDestroyQueue()
{
	if (pInputQueue != NULL) {
		DisposePtr((Ptr)pInputQueue);
		pInputQueue = NULL;
	}
	pQueueFront = pQueueBack = pInputQueueEnd = NULL;
	QueueEmpty = true;
	QueueOverflowed = false;
	return;
}

void	EnumerateCMDevices()
{
	// Enumerate available CoreMidi devices
	// This code is from the Echo.cpp CM example
	int i, n;
	OSStatus err;
	Boolean ok;
	CFStringRef pname, pmanuf, pmodel;
	char name[64], manuf[64], model[64], line[256];
	
	ok = TRUE;
	n = MIDIGetNumberOfDevices();
	for (i = 0; i < n; ++i) {
		MIDIDeviceRef dev = MIDIGetDevice(i);
		
		err = MIDIObjectGetStringProperty(dev, kMIDIPropertyName, &pname);
		if (err != noErr) continue;
		err = MIDIObjectGetStringProperty(dev, kMIDIPropertyManufacturer, &pmanuf);
		if (err != noErr) {
			CFRelease(pname);
			continue;
		}
		err = MIDIObjectGetStringProperty(dev, kMIDIPropertyModel, &pmodel);
		if (err != noErr) {
			CFRelease(pname);
			CFRelease(pmanuf);
			continue;
		}
		
		ok = ok & CFStringGetCString(pname, name, sizeof(name), kCFStringEncodingMacRoman);
		ok = ok & CFStringGetCString(pmanuf, manuf, sizeof(manuf), kCFStringEncodingMacRoman);
		ok = ok & CFStringGetCString(pmodel, model, sizeof(model), kCFStringEncodingMacRoman);
		
		CFRelease(pname);
		CFRelease(pmanuf);
		CFRelease(pmodel);

		if (ok) {
			sprintf(line, "name=%s, manuf=%s, model=%s", name, manuf, model);
			Println(wTrace, line);
		}
	}
}

OSStatus InitCoreMidiDriver()
{
	OSStatus err;
	int result;
	Boolean ok, haveInputPort;
	MIDIEndpointRef endpt;
	CFStringRef pname, strtemp;
	ItemCount num;
	char name[MAXENDPOINTNAME];
	
	haveInputPort = false;
	strtemp = CFSTR("Bol Processor");
	if (strtemp == NULL)  return -1;
	err = MIDIClientCreate(strtemp, CMNotifyCallback, NULL, &CMClient);
	if (err == noErr) {
		ShowMessage(TRUE, wMessage, "Signed into CoreMIDI.");
		strtemp = CFSTR("BP OutPort");
		if (strtemp == NULL)  return -1;
		// create a port so we can send messages
		err = MIDIOutputPortCreate(CMClient, strtemp, &CMOutPort);
		if (err == noErr) {
			// EnumerateCMDevices();
			CMActiveDestinations = (MIDIEndpointRef*) NewPtr(CMActiveDestinationsSize * sizeof(MIDIEndpointRef));		
			if ((err = MemError()) == noErr) {
				CoreMidiOutputOn = true;
				CMActiveDestinations[0] = NULL;
			}
			else  {
				Alert1("Not enough memory to allocate MIDI destinations array!");
				return err;
			}
		}
		else ShowMessage(TRUE, wMessage, "Error: Could not create a MIDI output port.");
		
		// create queue before trying to make CoreMIDI input port
		if (CMCreateAndInitQueue() == noErr) {
			strtemp = CFSTR("BP InPort");
			if (strtemp == NULL)  return -1;
			// create a port so we can receive messages
			err = MIDIInputPortCreate(CMClient, strtemp, CMReadCallback, NULL, &CMInPort);
			if (err == noErr) {
				CMActiveSources = (MIDIEndpointRef*) NewPtr(CMActiveSourcesSize * sizeof(MIDIEndpointRef));
				if ((err = MemError()) == noErr) {
					haveInputPort = true;
					CMActiveSources[0] = NULL;
				}
				else  {
					Alert1("Not enough memory to allocate MIDI sources array!");
					return err;
				}
			}
			else ShowMessage(TRUE, wMessage, "Error: Could not create a MIDI input port.");
			
			/*if (err != noErr) CMDestroyQueue();*/
		}		
	}
	else {
		ShowMessage(TRUE, wMessage, "Error: Could not sign into CoreMIDI.");
		return err;
	}
	
	// populate the list boxes with sources and destinations
	result = UpdateListBoxEntries(CMInputListData, MIDIGetNumberOfSources, MIDIGetSource);
	if (result != OK) return -1;
	result = UpdateListBoxEntries(CMOutputListData, MIDIGetNumberOfDestinations, MIDIGetDestination);
	if (result != OK) return -1;
	
	if (CoreMidiOutputOn) {
		// find the first destination
		// (code adapted from the Echo.cpp CM example)
		num = MIDIGetNumberOfDestinations();
		if (num > 0 && (endpt = MIDIGetDestination(0)) != NULL) {
			err = MIDIObjectGetStringProperty(endpt, kMIDIPropertyName, &pname);
			if (err == noErr) {
				ok = CFStringGetCString(pname, name, sizeof(name), kCFStringEncodingMacRoman);
				CFRelease(pname);
			}
			if (err != noErr || !ok) strcpy(name, "<unknown>");
			sprintf(Message, "Sending Midi to destination: %s.", name);
			ShowMessage(TRUE, wMessage, Message);
			err = noErr;
			
			// select the first destination in our list box
			SelectListBoxItem(CMOutputListData, 0);
		}
		else ShowMessage(TRUE, wMessage, "No MIDI destinations present.");
	}
	if (haveInputPort) {
		// find the first source and connect to it
		num = MIDIGetNumberOfSources();
		if (num > 0) {
			// select the first source in our list box
			SelectListBoxItem(CMInputListData, 0);
		}
		else ShowMessage(TRUE, wMessage, "No MIDI sources present.");
	}
	
	// do a full update of list boxes to connect to the selections made (if any)
	result = UpdateCMSettingsListBoxes();
	if (result != OK) return -1;
	
	return err;
}

static OSStatus CMConnectToSources(MIDIEndpointRef* endpoints, Boolean tellConnections)
{
	OSStatus err, err2, anyerr;
	Boolean ok;
	CFStringRef pname;
	char name[MAXENDPOINTNAME];
	MIDIEndpointRef* ep = endpoints;
	
	CoreMidiInputOn = false;
	if (ep == NULL)  return paramErr;
	anyerr = noErr;
	while (*ep != NULL) {
		err = MIDIPortConnectSource(CMInPort, *ep, NULL);
		if (tellConnections) {
			err2 = MIDIObjectGetStringProperty(*ep, kMIDIPropertyName, &pname);
			if (err2 == noErr) {
				ok = CFStringGetCString(pname, name, sizeof(name), kCFStringEncodingMacRoman);
				CFRelease(pname);
			}
			if (err2 != noErr || !ok) strcpy(name, "<unknown>");
			if (err == noErr) {
				sprintf(Message, "Receiving Midi from source: %s.", name);
				ShowMessage(TRUE, wMessage, Message);
			}
			else ShowMessage(TRUE, wMessage, "Error: Could not connect to a selected MIDI input source.");
		}
		if (err == noErr)  CoreMidiInputOn = true;
		else  anyerr = err;
		++ep;
	}
	
	return anyerr;
}

static OSStatus CMDisconnectFromSources(MIDIEndpointRef* endpoints)
{
	OSStatus err, anyerr;
	MIDIEndpointRef* ep = endpoints;
	
	if (ep == NULL)  return paramErr;
	anyerr = noErr;
	while (*ep != NULL) {
		err = MIDIPortDisconnectSource(CMInPort, *ep);
		if (err != noErr) anyerr = err;
		++ep;
	}
	
	return anyerr;
}

/* This callback seems to always be called in our main thread (via GetNextEvent),
   so I do not think that it is a synchronization risk. */
static void CMNotifyCallback(const MIDINotification *message, void *refCon)
{
	if (message->messageID == kMIDIMsgSetupChanged)
		UpdateCMSettingsListBoxes();
	
	return;
}

static void CMReadCallback(const MIDIPacketList* pktlist, void* readProcRefCon, void* srcConnRefCon)
{
	int i;
	const MIDIPacket *pkt = &pktlist->packet[0];
	
	// if (!OutMIDI || !CoreMidiInputOn)  return;	// FIXME ? should we check these ?
	for (i = 0; i < pktlist->numPackets; ++i) {
		// check if we are receiving this type of event
		if(AcceptEvent(ByteToInt(pkt->data[0]))) {
			if(CMMidiThruOn && PassEvent(ByteToInt(pkt->data[0]))) {
				// should we allow Midi Thru? Make it an option?
				/*SchedulerIsActive--;
				OMSWritePacket2(pkt,gOutNodeRefNum,gOutputPortRefNum);
				SchedulerIsActive++;*/
			}
			CMAddEventToQueue(pkt);
		}
		pkt = MIDIPacketNext(pkt);
	}
	
	return;
}

static int CMAddEventToQueue(const MIDIPacket* pkt)
{
	int i;
	UInt16 nbytes = pkt->length;
	Milliseconds time = (unsigned long)(AudioConvertHostTimeToNanos(pkt->timeStamp) 
					/ ((UInt64)1000000 /** (UInt64)Time_res)*/));
	
	if (pthread_mutex_lock(&QueueMutex) != 0)  return(FAILED);
	for(i=0; i < nbytes; i++) {
		pQueueBack->time = time;
		pQueueBack->byte = pkt->data[i];
		pQueueBack->sequence = 0;
		if (++pQueueBack == pInputQueueEnd) pQueueBack = pInputQueue;
		QueueEmpty = false;
		if (pQueueBack == pQueueFront) {
			// if we have filled the queue just remove the oldest event
			// (we always leave at least one MIDIcode space "empty");
			// must look for status byte so that we don't leave pQueueFront
			// pointing to the middle of an event
			do if (++pQueueFront == pInputQueueEnd) pQueueFront = pInputQueue;
			while (pQueueFront->byte < 128);
			QueueOverflowed = true;
		}
	}
	pthread_mutex_unlock(&QueueMutex);
	return(OK);
}

static int CMRemoveEventFromQueue(MIDI_Event* p_e)
{
	if (pthread_mutex_lock(&QueueMutex) != 0)   return(FAILED);
	if(QueueEmpty || pQueueBack == pQueueFront) {
		pthread_mutex_unlock(&QueueMutex);
		return(FAILED);
	}
	p_e->type = RAW_EVENT;
	p_e->time = pQueueFront->time; 
	p_e->data2 = pQueueFront->byte;
	if (++pQueueFront == pInputQueueEnd) pQueueFront = pInputQueue;
	if (pQueueFront == pQueueBack) QueueEmpty = true;

	pthread_mutex_unlock(&QueueMutex);
	return(OK);
}


GetNextMIDIevent(MIDI_Event *p_e,int loop,int force)
{
	if(!CoreMidiInputOn) return(FAILED);
	
	while(TRUE) {
		// p_e->type = RAW_EVENT;
		// if(CMRemoveEventFromQueue(p_e) != OK) goto NEXTRY;
		// if(p_e->type != RAW_EVENT) goto NEXTRY;
		if(CMRemoveEventFromQueue(p_e) == OK) return(OK);
		
	NEXTRY:
		if(!loop) return(FAILED);
		if(/*!force &&*/ Button()) return(ABORT);
	}
	return(OK);
}


OSErr DriverWrite(Milliseconds time,int nseq,MIDI_Event *p_e)
{
	OSStatus err;
	int result;
	
	err = noErr;
	/*if(!CoreMidiOutputOn && OutMIDI) {
		if(Beta) Println(wTrace,"Err. DriverWrite(). Driver output is OFF");
		return(noErr);
	}*/
	if(EmergencyExit || Panic || InitOn) return(noErr);
	if((ItemCapture && ItemOutPutOn) || TickCaptureStarted) {
		result = CaptureMidiEvent(time, nseq, p_e);
		if (result != OK) return(noErr);
	}
	if(!OutMIDI || MIDIfileOn) return(noErr);

	// Register program change to the MIDI orchestra
	if(SoundOn && p_e->type == TWO_BYTE_EVENT && !ConvertMIDItoCsound && !ItemCapture
			&& !PlayPrototypeOn && ItemNumber < 2L) {
		RegisterProgramChange(p_e);
	}

	if(!CoreMidiOutputOn)  return(noErr);
	if(!NEWTIMER) {
		Byte pktbuf[1024];
		Byte databuf[4];
		ByteCount len;
		MIDITimeStamp cmtime;
		UInt64 nanoseconds;
		MIDIPacketList *pktlist = (MIDIPacketList *)pktbuf;
		MIDIPacket *curpkt;

		if(time < ZERO) {
			if(Beta) {
				sprintf(Message,"Err. DriverWrite(). time  = %ld",(long)time);
				Println(wTrace,Message);
			}
			time = ZERO;
		}
		
		curpkt = MIDIPacketListInit(pktlist);

		nanoseconds = AudioConvertHostTimeToNanos(ClockZero);
		nanoseconds += ((UInt64)time * (UInt64)1000000); // add our time offset (ms) to the host clock (ns)
		cmtime = AudioConvertNanosToHostTime(nanoseconds);
		switch(p_e->type) {
			case RAW_EVENT:
				len = 1;
				databuf[0] = p_e->data2;
				break;
			case TWO_BYTE_EVENT:
				len = 2;
				databuf[0] = p_e->status;
				databuf[1] = p_e->data2;
				break;
			case NORMAL_EVENT:
				len = 3;
				databuf[0] = p_e->status;
				databuf[1] = p_e->data1;
				databuf[2] = p_e->data2;
				break;
		}
		curpkt = MIDIPacketListAdd(pktlist, sizeof(pktbuf), curpkt, cmtime, len, databuf);
		if (curpkt != NULL) {
			MIDIEndpointRef* dest = CMActiveDestinations;
			while (*dest != NULL) {
				err = MIDISend(CMOutPort, *dest, pktlist);
				++dest;
				++Nbytes;
			}
		}
		else if (Beta) Alert1("Err. DriverWrite(). MIDIPacketListAdd() failed");
	}
	
	return(err);
}


int FlushDriver(void)
{
	OSStatus err;
	
	RunningStatus = 0;
	Nbytes = Tbytes2 = ZERO;

	if(!IsMidiDriverOn()) {
		if(Beta) Alert1("Err. FlushDriver(). Driver is OFF");
		return(ABORT);
	}
	/* flush output */
	if (CoreMidiOutputOn && CMActiveDestinations != NULL) {
		MIDIEndpointRef* dest = CMActiveDestinations;
		while (*dest != NULL) err = MIDIFlushOutput(*dest++);
	}
	
	/* flush input */
	if (CoreMidiInputOn)  CMReInitQueue();
	
	return(OK);
}


int ResetDriver(void)
{
	if(SetOutputFilterWord() != OK) return(ABORT);
	SetDriverTime(ZERO);

	Tcurr = Nbytes = Tbytes2 = ZERO;
	ResetTickFlag = TRUE;
	Dirty[wTimeAccuracy] = FALSE;
	RunningStatus = 0;
	return(OK);
}


int SetDriver(void)
{
	if(SetOutputFilterWord() != OK) return(ABORT);
	SetDriverTime(ZERO);

	Tcurr = Nbytes = Tbytes2 = ZERO;
	MaxMIDIbytes = LONG_MAX;
	Dirty[wTimeAccuracy] = FALSE;
	RunningStatus = 0;
	return(OK);
}


int ResetMIDI(int wait)
{
	int rep;

	rep = OK;

	if(!OutMIDI || (AEventOn && !IsMidiDriverOn())) return(OK);

	if(!IsMidiDriverOn()) {
		if(Beta) Alert1("Err. ResetMIDI(). Driver is OFF");
		return FAILED;
	}
	
	if(wait && !InitOn && !WaitOn) rep = WaitForLastTicks();

	if(wait && rep == OK && !InitOn && !WaitOn) rep = WaitForEmptyBuffer();
		
	WaitABit(200L);
	FlushDriver();
	WaitABit(200L);
	ResetDriver();

	ResetTicks(FALSE,TRUE,ZERO,0);

	return(rep);
}


/* GetDriverTime() returns a value that is the number of milliseconds
   since the driver clock's "zero point" divided by Time_res.
   SetDriverTime() sets the current time in these units. */

unsigned long GetDriverTime(void)
{
	unsigned long time;
	MIDITimeStamp sincezero;

	sincezero = AudioGetCurrentHostTime() - ClockZero;
	time = (unsigned long)(AudioConvertHostTimeToNanos(sincezero) / ((UInt64)1000000 * (UInt64)Time_res));
	return(time);
}

int SetDriverTime(long time)
{
	MIDITimeStamp clockcurrent, offset;
	
	clockcurrent = AudioGetCurrentHostTime();
	offset = AudioConvertNanosToHostTime(((UInt64)(time * Time_res) * (UInt64)1000000));
	/* save the clock time that we are calling "zero" */
	ClockZero = clockcurrent - offset;
	return(OK);
}


/* CoreMIDI settings dialog */
const  short	CMSettingsID = 1000;
const  short	diInputList = 4;
const  short	diOutputList = 5;
const  short	diCreateDestination = 6;
const  short	diCreateSource = 7;
const  short	diMidiThru = 8;
const  short	diSaveStartup = 9;
const  short	diSaveSettings = 10;
const  short	diLoadSettings = 11;

OSStatus CreateCMSettings()
{
	ListHandle	inputLH, outputLH;
	ControlRef	cntl;
	
	CMSettings = GetNewDialog(CMSettingsID, NULL, kLastWindowOfClass);
	if (CMSettings != NULL) {
		// get list handles
		GetDialogItemAsControl(CMSettings, diInputList, &cntl);
		GetControlData(cntl, kControlEntireControl, kControlListBoxListHandleTag, sizeof(inputLH), &inputLH, NULL);
		// SetKeyboardFocus(GetDialogWindow(CMSettings), cntl, 1);
		
		GetDialogItemAsControl(CMSettings, diOutputList, &cntl);
		GetControlData(cntl, kControlEntireControl, kControlListBoxListHandleTag, sizeof(outputLH), &outputLH, NULL);
		
		// create our list data structs
		CMInputListData = NewListDataHandle(inputLH, 1);
		if (CMInputListData == NULL)   return memFullErr;
		CMOutputListData = NewListDataHandle(outputLH, 1);
		if (CMOutputListData == NULL)  return memFullErr;
		
		SetThemeWindowBackground(GetDialogWindow(CMSettings), kThemeBrushDialogBackgroundActive, false);
		// DrawDialog(CMSettings);
		// ShowWindow(GetDialogWindow(CMSettings));
		// SelectWindow(GetDialogWindow(CMSettings));
	}
	else return -1;
	
	return noErr;
}

int DoCMSettingsEvent(EventRecord* event, short itemHit)
{
	int result;
	
	result = OK;
	switch(itemHit) {
		case	diInputList:
			result = GetListBoxSelection(CMInputListData);
			if (result != OK) return result;
			result = CMDisconnectFromSources(CMActiveSources); // err result is not fatal
			result = UpdateActiveEndpoints(CMInputListData, &CMActiveSources, &CMActiveSourcesSize, &CoreMidiInputOn);
			if (result != OK) return result;
			result = CMConnectToSources(CMActiveSources, true); // err result is not fatal
			break;
		case	diOutputList:
			result = GetListBoxSelection(CMOutputListData);
			if (result != OK) return result;
			result = UpdateActiveEndpoints(CMOutputListData, &CMActiveDestinations, &CMActiveDestinationsSize, &CoreMidiOutputOn);
			if (result != OK) return result;
			break;
		case	diCreateDestination:
			break;
		case	diCreateSource:
			break;
		case	diMidiThru:
			break;
		case	diSaveStartup:
			result = SaveMidiDriverStartup();
			break;
		case	diSaveSettings:
			result = SaveMidiDriverSettings();
			break;
		case	diLoadSettings:
			result = LoadMidiDriverSettings();
			break;
	}
	
	// Dirty[iSettings] = TRUE;
	if (result != OK) return result;
	else  return (DONE);
}

static void ResizeListBox(ListHandle list, long numRows)
{
	long currentRows = (*list)->dataBounds.bottom - (*list)->dataBounds.top;
	
	if (currentRows < numRows)
		LAddRow(numRows-currentRows, currentRows, list);
	else if (currentRows > numRows)
		LDelRow(currentRows-numRows, 0, list);
	
	return;
}

static CMListData** NewListDataHandle(ListHandle list, Size entriesSize)
{
	CMListData** ldh;
	
	ldh = (CMListData**) GiveSpace(sizeof(CMListData));
	if (ldh == NULL) return NULL;
	
	(*ldh)->list = list;
	(*ldh)->entries = (CMListEntry**) GiveSpace(entriesSize * sizeof(CMListEntry));
	if ((*ldh)->entries == NULL) {
		MyDisposeHandle((Handle*)&ldh);
		return NULL;
	}
	(*ldh)->entriesSize = entriesSize;
	(*ldh)->numEntries = 0;
	
	return ldh;
}

static void	MarkAllOffline(CMListData* ld)
{
	int i;
	CMListEntry* ep = *(ld->entries);
	
	// set all offline flags
	for (i = 0; i < ld->numEntries; ++i, ++ep)
		ep->offline = TRUE;
	
	return;
}

static void	MarkAllNotSelected(CMListData* ld)
{
	int i;
	CMListEntry* ep = *(ld->entries);
	
	// clear all selected flags
	for (i = 0; i < ld->numEntries; ++i, ++ep)
		ep->selected = FALSE;
	
	return;
}

static CMListEntry* FindListEntryByID(CMListData* ld, MIDIUniqueID id)
{
	int i;
	CMListEntry* ep = *(ld->entries);
	
	for (i = 0; i < ld->numEntries; ++i, ++ep)
		if (ep->id == id) return ep;
	
	return NULL;
}

static CMListEntry* FindListEntryByName(CMListData* ld, char* endname)
{
	int i;
	CMListEntry* ep = *(ld->entries);
	
	for (i = 0; i < ld->numEntries; ++i, ++ep)
		if (strcmp(*(ep->name), endname) == 0) return ep;
	
	return NULL;

}

static CMListEntry* FindListEntryByRef(CMListData* ld, MIDIEndpointRef endpt)
{
	int i;
	CMListEntry* ep = *(ld->entries);
	
	for (i = 0; i < ld->numEntries; ++i, ++ep)
		if (ep->endpoint == endpt) return ep;
	
	return NULL;

}

/* Warning: this function may temporarily unlock ld->entries if it is locked
   so that it can resize it; therefore, the address of ld->entries may change! 
   (ld should be locked since this function allocates memory) */
static int GetNewListEntry(CMListData* ld, CMListEntry** p_entryptr)
{
	if (ld->numEntries < ld->entriesSize) {
		*p_entryptr = (*(ld->entries)) + ld->numEntries;
		++ld->numEntries;
		return (OK);
	}
	else {
		int result;
		Size newsize;
		char hflags = HGetState((Handle)ld->entries);
		if ((result = MyUnlock((Handle)ld->entries)) != OK) return result;
		newsize = ld->entriesSize + 3; // increase 3 slots at a time
		result = MySetHandleSize((Handle*)&(ld->entries), newsize * sizeof(CMListEntry));
		HSetState((Handle)ld->entries, hflags);
		if ( result != OK) return result;
		ld->entriesSize= newsize;
		*p_entryptr = (*(ld->entries)) + ld->numEntries;
		++ld->numEntries;
		return (OK);
	}
}

/* Calls GetNewListEntry() and then sets the values for the new entry record.
   Maintains numEntries if a failure occurs.  See caveats for GetNewListEntry() above. 
   (ld should be locked since this function allocates memory) */
static int InitNewListEntry(CMListData* ld, CMListEntry** p_entryptr, MIDIEndpointRef endpt, MIDIUniqueID endID, 
                            char* endname, Boolean selected, Boolean offline)
{
	int result;
	CMListEntry* entry;
	
	// Note: address of ld->entries may change with this call
	result = GetNewListEntry(ld, &entry);
	if (result != OK) return result;
	
	// fill in the entry's data
	entry->endpoint = endpt;
	entry->id = endID;
	entry->name = (char**) GiveSpace(strlen(endname)+1);
	if (entry->name == NULL) {
		ld->numEntries--;
		return FAILED;
	}
	strcpy(*(entry->name), endname);
	entry->selected = selected;
	entry->offline = offline;
	// entry->stillexists = TRUE;
	
	*p_entryptr = entry;
	return OK;
}

static int CompareEntryNames(const CMListEntry* a, const CMListEntry* b)
{
	return strcmp(*(a->name), *(b->name));
}

static int UpdateListBoxEntries(CMListData** ldh, ItemCount (*countFunc)(void), MIDIEndpointRef (*getEndpointFunc)(ItemCount))
{
	OSStatus err;
	Boolean ok;
	int result;
	CMListData* ld;
	CFStringRef name;
	MIDIEndpointRef endpt;
	MIDIUniqueID    endID;
	ItemCount count, index;
	char endname[MAXENDPOINTNAME];
	CMListEntry* entry;
	
	if (ldh == NULL)  {
		if(Beta) Alert1("Err. UpdateListBox(): ldh is NULL");
		return (FAILED);
	}
	if ((result = MyLock(FALSE, (Handle)ldh)) != OK)  return result;
	ld = *ldh;
	
	MarkAllOffline(ld);
	
	// resize the list data entries
	count = countFunc();
	{	Size newEntriesSize;			// only valid for this block
		newEntriesSize = (count * 3) / 2;	// try to anticipate how many extra entries we may need
		if (ld->entriesSize < newEntriesSize) {
			result = MySetHandleSize((Handle*)&(ld->entries), newEntriesSize * sizeof(CMListEntry));
			// we don't have to give up yet if resizing failed
			if (result == OK) ld->entriesSize = newEntriesSize;
		}
	}
	
	if ((result = MyLock(FALSE, (Handle)ld->entries)) != OK) {
		MyUnlock((Handle)ldh);
		return result;
	}

	// update our CMListEntry's from CoreMIDI, adding new endpoints
	// and keeping data for all others, including those that have went "offline" 
	for (index = 0; index < count; ++index) {
		endpt = getEndpointFunc(index);
		if (endpt != NULL) {
			// attempt to find an existing entry for this endpoint
			entry = NULL;
			
			// check for matching CoreMIDI unique ID
			err = MIDIObjectGetIntegerProperty(endpt, kMIDIPropertyUniqueID, &endID);
			if (err == noErr)  entry = FindListEntryByID(ld, endID);
			else endID = 0;
			
			// get the endpoint name & check for match if we don't have one yet
			err = MIDIObjectGetStringProperty(endpt, kMIDIPropertyName, &name);
			if (err == noErr) {
				ok = CFStringGetCString(name, endname, sizeof(endname), kCFStringEncodingMacRoman);
				CFRelease(name);
				if (entry == NULL & ok)  entry = FindListEntryByName(ld, endname);
			}
			if (err != noErr || !ok) strcpy(endname, "<unknown>");
			err = noErr;

			// use MIDIEndpointRef as a last attempt to find it
			if (entry == NULL)  entry = FindListEntryByRef(ld, endpt);
			
			// create new entry if no match
			if (entry == NULL)  {
				// Note: address of ld->entries may change with this call
				result = InitNewListEntry(ld, &entry, endpt, endID, endname, FALSE, FALSE);
				if (result != OK) goto EXITNOW;
			}
			else {
				entry->offline = FALSE;		// mark matching entry as online
				entry->endpoint = endpt;	// MIDIEndPointRef may have changed ?
			}
		}
		else if (Beta) Alert1("Err. UpdateListBox():  getEndpointFunc() returned NULL.");
	}
	
	MyUnlock((Handle)ld->entries);
	MyUnlock((Handle)ldh);
	result = SetListBoxContentsAndSelection(ldh);
	return result;
	
EXITNOW:
	MyUnlock((Handle)ld->entries);
	MyUnlock((Handle)ldh);
	return result;
}

/* Selects one item of one of our CM lists; does not affect the selection
   status of other items */
int SelectListBoxItem(CMListData** ldh, Size itemnumber)
{
	Cell selection;

	// set the list selection
	if (ldh != NULL && (*ldh)->list != NULL) {
		SetPt(&selection, 0, itemnumber);
		LSetSelect(true, selection, (*ldh)->list);
	}
	else return (FAILED);

	// set CMListEntry to selected (if it exists)
	if ((*ldh)->numEntries > itemnumber) {
		(*(*ldh)->entries)->selected = true;
	}
	
	return (OK);
}
	
int GetListBoxSelection(CMListData** ldh)
{
	int result;
	CMListData* ld;
	CMListEntry* entry;
	Size index;
	Cell cellnum;
	
	if (ldh == NULL)  {
		if(Beta) Alert1("Err. GetListBoxSelection(): ldh is NULL");
		return (FAILED);
	}
	if ((result = MyLock(FALSE, (Handle)ldh)) != OK)  return result;
	ld = *ldh;
	if ((result = MyLock(FALSE, (Handle)ld->entries)) != OK) {
		MyUnlock((Handle)ldh);
		return result;
	}
	
	entry = *(ld->entries);
	for (index = 0; index < ld->numEntries; ++index) {
		SetPt(&cellnum, 0, index);
		entry->selected = LGetSelect(FALSE, &cellnum, ld->list);
		++entry;
	}

EXITNOW:
	MyUnlock((Handle)ld->entries);
	MyUnlock((Handle)ldh);
	return result;
}

/* Sorts the entries, resizes the list box, and then fills the box with entry names,
   marking their selection status.  */
static int SetListBoxContentsAndSelection(CMListData** ldh)
{
	int result;
	CMListData* ld;
	CMListEntry* entry;
	Size index;
	Cell cellnum;

	if (ldh == NULL)  {
		if(Beta) Alert1("Err. SetListBoxContentsAndSelection(): ldh is NULL");
		return (FAILED);
	}
	if ((result = MyLock(FALSE, (Handle)ldh)) != OK)  return result;
	ld = *ldh;
	if ((result = MyLock(FALSE, (Handle)ld->entries)) != OK) {
		MyUnlock((Handle)ldh);
		return result;
	}
	
	// sort the entries
	qsort(*(ld->entries), ld->numEntries, sizeof(CMListEntry), (CompareFuncType)CompareEntryNames);
	
	ResizeListBox(ld->list, ld->numEntries);
	// set text and selected status for each list box entry
	{	char entryname[MAXENDPOINTNAME+SUFFIXSIZE];
		entry = *(ld->entries);
		for (index = 0; index < ld->numEntries; ++index) {
			SetPt(&cellnum, 0, index);
			if ((result = MyLock(FALSE, (Handle)entry->name)) != OK) goto EXITNOW;
			strcpy(entryname, *(entry->name));
			if (entry->offline) strcat(entryname, OfflineSuffix);
			LSetCell(entryname, strlen(entryname), cellnum, ld->list);
			LSetSelect(entry->selected, cellnum, ld->list);
			MyUnlock((Handle)entry->name);
			++entry;
		}
	}

	result = OK;
	
EXITNOW:
	MyUnlock((Handle)ld->entries);
	MyUnlock((Handle)ldh);
	return result;
}

int UpdateCMSettingsListBoxes()
{
	OSStatus err;
	int result;	
	
	if (CMSettings == NULL) return (FAILED);
	
	// save list selections
	result = GetListBoxSelection(CMInputListData);
	if (result != OK) return result;
	result = GetListBoxSelection(CMOutputListData);
	if (result != OK) return result;
	
	// update source list
	result = UpdateListBoxEntries(CMInputListData, MIDIGetNumberOfSources, MIDIGetSource);
	if (result != OK) return result;
	err = CMDisconnectFromSources(CMActiveSources);  // err result is not fatal
	result = UpdateActiveEndpoints(CMInputListData, &CMActiveSources, &CMActiveSourcesSize, &CoreMidiInputOn);
	if (result != OK) return result;
	err = CMConnectToSources(CMActiveSources, true); // err result is not fatal
	
	// update destination list
	result = UpdateListBoxEntries(CMOutputListData, MIDIGetNumberOfDestinations, MIDIGetDestination);
	if (result != OK) return result;
	result = UpdateActiveEndpoints(CMOutputListData, &CMActiveDestinations, &CMActiveDestinationsSize, &CoreMidiOutputOn);
	if (result != OK) return result;
	
	DrawDialog(CMSettings);
	return (OK);
}

static int UpdateActiveEndpoints(CMListData** ldh, MIDIEndpointRef** endpoints, Size* epArraySize, Boolean* status)
{
	OSStatus err;
	Size	index, newsize, numSelected, numEntries;
	MIDIEndpointRef*	ep;
	CMListEntry*	entry;
	
	// count the selected list entries that are also "online"
	entry = *((*ldh)->entries);
	numEntries = (*ldh)->numEntries;
	numSelected = 0;
	for (index = 0; index < numEntries; ++index) {
		if (entry->selected && !entry->offline) ++numSelected;
		++entry;
	}
	
	// resize array if necessary (include space for NULL)
	if (*epArraySize <= numSelected && numSelected > 0) {
		if (*endpoints != NULL) DisposePtr((Ptr)*endpoints);
		// leave some extra room in the array
		newsize = ((numSelected*3)/2) + 1;
		*endpoints = (MIDIEndpointRef*) NewPtr(newsize * sizeof(MIDIEndpointRef));
		err = MemError();
		if (err != noErr) {
			*endpoints = NULL;
			*epArraySize = 0;
			*status = FALSE;
			return (FAILED);
		}
		*epArraySize = newsize;
		*status = TRUE;
	}
	
	// copy selected MIDIEndpointRefs to endpoints array
	entry = *((*ldh)->entries);
	ep = *endpoints;
	for (index = 0; index < numEntries; ++index) {
		if (entry->selected && !entry->offline) *ep++ = entry->endpoint;
		++entry;
	}
	*ep = NULL;	// terminate the array with NULL
	
	return (OK);
}

static int WriteActiveEndpoints(CMListData** ldh, short refnum)
{
	int result;
	Size	index, numSelected, numEntries;
	CMListEntry*	entry;
	char			line[32];	// only used for writing numbers
	
	// count the selected list entries that are also "online"
	entry = *((*ldh)->entries);
	numEntries = (*ldh)->numEntries;
	numSelected = 0;
	for (index = 0; index < numEntries; ++index) {
		if (entry->selected && !entry->offline) ++numSelected;
		++entry;
	}

	// write the number of endpoints to save
	sprintf(line, "%ld", (long)numSelected);
	WriteToFile(NO,MAC,line,refnum);

	// write only the selected list entries that are also "online"
	if ((result = MyLock(FALSE, (Handle)(*ldh)->entries)) != OK)  return result;
	entry = *((*ldh)->entries);
	for (index = 0; index < numEntries; ++index) {
		if (entry->selected && !entry->offline) {
			sprintf(line, "%ld", entry->id);
			WriteToFile(NO,MAC,line,refnum);
			if ((result = MyLock(FALSE, (Handle)entry->name)) != OK)  return result;
			WriteToFile(NO,MAC,*(entry->name),refnum);			
			if ((result = MyUnlock((Handle)entry->name)) != OK)  return result;
		}
		++entry;
	}
	if ((result = MyUnlock((Handle)(*ldh)->entries)) != OK)  return result;

	return (OK);
}

int WriteCoreMIDISettings(short refnum)
{
	int  result;
	char line[32];	// only used for writing numbers
	
	/**** IF YOU CHANGE ANY PART OF THE FILE FORMAT, CHANGE THE VERSION CONSTANTS ABOVE ****/
	
	// write version numbers
	sprintf(line, "%d", SETTINGS_FILE_VERSION);
	WriteToFile(NO,MAC,line,refnum);
	sprintf(line, "%d", OLDEST_COMPATIBLE_VERSION);
	WriteToFile(NO,MAC,line,refnum);

	// write general settings next (none of these are implemented yet)
	sprintf(line, "%d", (int)CMMidiThruOn);
	WriteToFile(NO,MAC,line,refnum);	// MIDI Thru
	WriteToFile(NO,MAC,"0",refnum);	// Create virtual input
	WriteToFile(NO,MAC,"0",refnum);	// Create virtual output
	WriteToFile(NO,MAC,"0",refnum);	// Reserved 1
	WriteToFile(NO,MAC,"0",refnum);	// Reserved 2
	WriteToFile(NO,MAC,"0",refnum);	// Reserved 3
	
	result = WriteActiveEndpoints(CMInputListData, refnum);
	if (result != OK) return result;
	result = WriteActiveEndpoints(CMOutputListData, refnum);
	if (result != OK) return result;
	
	return OK;
}

static int ReadActiveEndpoints(CMListData** ldh, short refnum, long* pos)
{
	int  result, rep;
	long i, endID, epcount;
	char **p_line, **nameStrHandle;
	CMListEntry* matchingEntry = NULL;
	
	p_line = nameStrHandle = NULL;
	result = FAILED;
	
	// read the number of endpoints
	if (ReadLong(refnum, &epcount, pos) == FAILED) return FAILED;
	
	if (MyLock(FALSE, (Handle)ldh) != OK) return FAILED;
	MarkAllNotSelected(*ldh);
	
	// read each endpoint id & name pair
	for (i = 0; i < epcount; ++i) {
		if (ReadLong(refnum, &endID, pos) == FAILED) goto ERR;
		if (ReadOne(FALSE,FALSE,TRUE,refnum,TRUE,&p_line,&nameStrHandle,pos) == FAILED) goto ERR;
		// try to match values to an existing list entry
		matchingEntry = FindListEntryByID(*ldh, endID);
		if (matchingEntry == NULL)  matchingEntry = FindListEntryByName(*ldh, *nameStrHandle);
		if (matchingEntry == NULL)  {
			// if there is no match, create a new entry that is selected and offline
			// Note: address of (*ldh)->entries may change with this call
			rep = InitNewListEntry(*ldh, &matchingEntry, NULL, endID, *nameStrHandle, TRUE, TRUE);
			if (rep != OK) goto ERR;
			
		}
		else {
			matchingEntry->selected = TRUE;	// mark matching entry as selected
		}
	}
	
	result = OK;
ERR:
	MyUnlock((Handle)ldh);
	MyDisposeHandle((Handle*)&p_line);
	MyDisposeHandle((Handle*)&nameStrHandle);
	return result;
}

int ReadCoreMIDISettings(short refnum, long* pos)
{
	OSErr err;
	int   dummy;
	int   value, result;
	
	// read and check the file's version numbers
	if (ReadInteger(refnum, &value, pos) == FAILED) return (FAILED);		// file version
	// sanity check
	if (value < 1) return (FAILED);	
	// if code below is incapable of reading versions older than OLDEST_COMPATIBLE_VERSION
	// then we should check that file version is not less than that version
	// if (value < OLDEST_COMPATIBLE_VERSION) return (FAILED);
	if (ReadInteger(refnum, &value, pos) == FAILED) return (FAILED);		// compatible version
	if (value > SETTINGS_FILE_VERSION) return (FAILED);				// file is incompatible
	
	// read general settings next (none of these are implemented yet)
	if (ReadInteger(refnum, &value, pos) == FAILED) return (FAILED);		// MIDI Thru
	// CMMidiThruOn = (value != 0);
	if (ReadInteger(refnum, &dummy, pos) == FAILED) return (FAILED);		// Create virtual input
	if (ReadInteger(refnum, &dummy, pos) == FAILED) return (FAILED);		// Create virtual output
	if (ReadInteger(refnum, &dummy, pos) == FAILED) return (FAILED);		// Reserved 1
	if (ReadInteger(refnum, &dummy, pos) == FAILED) return (FAILED);		// Reserved 2
	if (ReadInteger(refnum, &dummy, pos) == FAILED) return (FAILED);		// Reserved 3
	
	// read information for MIDI sources
	result = ReadActiveEndpoints(CMInputListData, refnum, pos);
	if (result != OK) return result;
	
	// update sources
	result = SetListBoxContentsAndSelection(CMInputListData);
	if (result != OK) return result;
	err = CMDisconnectFromSources(CMActiveSources);  // err result is not fatal
	result = UpdateActiveEndpoints(CMInputListData, &CMActiveSources, &CMActiveSourcesSize, &CoreMidiInputOn);
	if (result != OK) return result;
	err = CMConnectToSources(CMActiveSources, true); // err result is not fatal

	// read information for MIDI destinations
	result = ReadActiveEndpoints(CMOutputListData, refnum, pos);
	if (result != OK) return result;
	
	// update destinations
	result = SetListBoxContentsAndSelection(CMOutputListData);
	if (result != OK) return result;
	result = UpdateActiveEndpoints(CMOutputListData, &CMActiveDestinations, &CMActiveDestinationsSize, &CoreMidiOutputOn);
	if (result != OK) return result;
	
	DrawDialog(CMSettings);
	return OK;
}
