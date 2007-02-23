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

#include <string.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreMIDI/CoreMIDI.h>

static Boolean		CoreMidiOutputOn = false;
static Boolean		CoreMidiInputOn = false;
static Boolean		CMMidiThruOn = false;
static MIDIPortRef	CMOutPort = NULL;
static MIDIEndpointRef	CMDest = NULL;
static MIDIPortRef	CMInPort = NULL;
static MIDIEndpointRef	CMSource = NULL;
static MIDIClientRef	CMClient = NULL;

static MIDITimeStamp	ClockZero = 0;		// CoreAudio's host time that we consider "zero"

const  Size			QueueSize = 5000L;
static Boolean		QueueOverflowed = false;
static Boolean		QueueEmpty = true;
static MIDIcode*		pInputQueue = NULL;	// beginning of allocated space for storing MIDI bytes
static MIDIcode*		pInputQueueEnd = NULL;	// one past the last allocated space for storing MIDI bytes
static MIDIcode*		pQueueFront = NULL;	// current location to remove bytes
static MIDIcode*		pQueueBack = NULL;	// current location to add bytes

static DialogPtr		CMSettings = NULL;

OSStatus CMCreateAndInitQueue();
void CMReInitQueue();
void CMDestroyQueue();
void CMNotifyCallback(const MIDINotification *message, void *refCon);
void CMReadCallback(const MIDIPacketList* pktlist, void* readProcRefCon, void* srcConnRefCon);
int  CMAddEventToQueue(const MIDIPacket* pkt);
int  CMRemoveEventFromQueue(MIDI_Event* p_e);
OSStatus UpdateCMSettingsListBoxes();
void ResizeListBox(ListHandle list, long numRows);
OSStatus UpdateListBox(ListHandle list, ItemCount (*countFunc)(void), MIDIEndpointRef (*getEndpointFunc)(ItemCount));

/*  Returns whether the CoreMIDI driver is on */
Boolean IsMidiDriverOn()
{
	return (CoreMidiOutputOn || CoreMidiInputOn);
}


static OSStatus CMCreateAndInitQueue()
{
	OSStatus err;
	// create queue buffer for receiving messages
	pInputQueue = (MIDIcode*) NewPtr(QueueSize * sizeof(MIDIcode));
	err = MemError();
	if (pInputQueue != NULL && err == noErr)  CMReInitQueue();
	return err;
}

static void CMReInitQueue()
{
	/*** FIXME: need to add thread synchronization here !! ***/
	if (pInputQueue != NULL) {
		pQueueFront = pQueueBack = pInputQueue;
		pInputQueueEnd = pInputQueue + QueueSize;
		QueueEmpty = true;
		QueueOverflowed = false;
	}
	return;
}

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
	Boolean ok;
	CFStringRef pname;
	ItemCount num;
	char name[128];
	
	err = MIDIClientCreate(CFSTR("Bol Processor"), CMNotifyCallback, NULL, &CMClient);
	if (err == noErr) {
		ShowMessage(TRUE, wMessage, "Signed into CoreMIDI.");
		
		// create a port so we can send messages
		err = MIDIOutputPortCreate(CMClient, CFSTR("BP OutPort"), &CMOutPort);
		if (err == noErr) {
			// EnumerateCMDevices();
			
			// find the first destination
			// (code adapted from the Echo.cpp CM example)
			num = MIDIGetNumberOfDestinations();
			if (num > 0) CMDest = MIDIGetDestination(0);

			if (CMDest != NULL) {
				err = MIDIObjectGetStringProperty(CMDest, kMIDIPropertyName, &pname);
				if (err == noErr) {
					ok = CFStringGetCString(pname, name, sizeof(name), kCFStringEncodingMacRoman);
					CFRelease(pname);
				}
				if (err != noErr || !ok) strcpy(name, "<unknown>");
				sprintf(Message, "Sending Midi to destination: %s.", name);
				ShowMessage(TRUE, wMessage, Message);
				CoreMidiOutputOn = true;
			}
			else ShowMessage(TRUE, wMessage, "No MIDI destinations present.");
		}
		else ShowMessage(TRUE, wMessage, "Error: Could not create a MIDI output port.");
		
		// create queue before trying to make CoreMIDI input port
		if (CMCreateAndInitQueue() == noErr) {
			// create a port so we can receive messages
			err = MIDIInputPortCreate(CMClient, CFSTR("BP InPort"), CMReadCallback, NULL, &CMInPort);
			if (err == noErr) {
				// find the first source and connect to it
				num = MIDIGetNumberOfSources();
				if (num > 0) CMSource = MIDIGetSource(0);
				if (CMSource != NULL) {
					err = MIDIPortConnectSource(CMInPort, CMSource, NULL);
					if (err == noErr) {
						err = MIDIObjectGetStringProperty(CMSource, kMIDIPropertyName, &pname);
						if (err == noErr) {
							ok = CFStringGetCString(pname, name, sizeof(name), kCFStringEncodingMacRoman);
							CFRelease(pname);
						}
						if (err != noErr || !ok) strcpy(name, "<unknown>");
						err = noErr;
						sprintf(Message, "Receiving Midi from source: %s.", name);
						ShowMessage(TRUE, wMessage, Message);
						CoreMidiInputOn = true;
					}
					else ShowMessage(TRUE, wMessage, "Error: Could not connect to MIDI input source.");
				}
				else ShowMessage(TRUE, wMessage, "No MIDI sources present.");
			}
			else ShowMessage(TRUE, wMessage, "Error: Could not create a MIDI input port.");
			
			if (err != noErr) CMDestroyQueue();
		}		
	}
	
	err = UpdateCMSettingsListBoxes();

	return err;
}

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
	
	/*** FIXME: need to add thread synchronization here !! ***/
	for(i=0; i < nbytes; i++) {
		pQueueBack->time = time;
		pQueueBack->byte = pkt->data[i];
		pQueueBack->sequence = 0;
		if (++pQueueBack == pInputQueueEnd) pQueueBack = pInputQueue;
		QueueEmpty = false;
		if (pQueueBack == pQueueFront) {
			// if we have filled the queue just remove the oldest event
			// (we always leave one space "empty")
			if (++pQueueFront == pInputQueueEnd) pQueueFront = pInputQueue;
			QueueOverflowed = true;
		}
	}
	return(OK);
}

static int CMRemoveEventFromQueue(MIDI_Event* p_e)
{
	/*** FIXME: need to add thread synchronization here !! ***/
	if(QueueEmpty || pQueueBack == pQueueFront) return(FAILED);
	p_e->type = RAW_EVENT;
	p_e->time = pQueueFront->time; 
	p_e->data2 = pQueueFront->byte;
	if (++pQueueFront == pInputQueueEnd) pQueueFront = pInputQueue;
	if (pQueueFront == pQueueBack) QueueEmpty = true;
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
	if(!CoreMidiOutputOn && OutMIDI) {
		if(Beta) Println(wTrace,"Err. DriverWrite(). Driver output is OFF");
		return(noErr);
	}
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
			if (CMDest != NULL) {
				err = MIDISend(CMOutPort, CMDest, pktlist);
				Nbytes++;
			}
			else FlashInfo("MIDI output destination was not found ... check CoreMIDI setup!");
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
	if (CoreMidiOutputOn && CMDest != NULL)  err = MIDIFlushOutput(CMDest);
	WaitABit(100L);

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

OSStatus CreateCMSettings()
{
	ListHandle	inputLH, outputLH;
	ControlRef	cntl;
	Cell		selection;
	
	CMSettings = GetNewDialog(CMSettingsID, NULL, kLastWindowOfClass);
	if (CMSettings != NULL) {
		// get list handles
		GetDialogItemAsControl(CMSettings, diInputList, &cntl);
		GetControlData(cntl, kControlEntireControl, kControlListBoxListHandleTag, sizeof(inputLH), &inputLH, NULL);
		// SetKeyboardFocus(GetDialogWindow(CMSettings), cntl, 1);
		
		GetDialogItemAsControl(CMSettings, diOutputList, &cntl);
		GetControlData(cntl, kControlEntireControl, kControlListBoxListHandleTag, sizeof(outputLH), &outputLH, NULL);
		
		// set the selections
		SetPt(&selection, 0, 0);
		LSetSelect(true, selection, inputLH);
		LSetSelect(true, selection, outputLH);
		
		SetThemeWindowBackground(GetDialogWindow(CMSettings), kThemeBrushDialogBackgroundActive, false);
		DrawDialog(CMSettings);
		ShowWindow(GetDialogWindow(CMSettings));
		SelectWindow(GetDialogWindow(CMSettings));
	}
	else return -1;
	
	return noErr;
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

static OSStatus UpdateListBox(ListHandle list, ItemCount (*countFunc)(void), MIDIEndpointRef (*getEndpointFunc)(ItemCount))
{
	OSStatus err;
	Boolean ok;
	CFStringRef name;
	MIDIEndpointRef endpt;
	ItemCount count, index;
	char cname[128];
	Cell cellnum;
	
	count = countFunc();
	ResizeListBox(list, count);
	for (index = 0; index < count; ++index) {
		endpt = getEndpointFunc(index);
		if (endpt != NULL) {
			err = MIDIObjectGetStringProperty(endpt, kMIDIPropertyName, &name);
			if (err == noErr) {
				ok = CFStringGetCString(name, cname, sizeof(cname), kCFStringEncodingMacRoman);
				CFRelease(name);
			}
			if (err != noErr || !ok) strcpy(cname, "<unknown>");
			err = noErr;
			SetPt(&cellnum, 0, index);
			LSetCell(cname, strlen(cname), cellnum, list);
		}
		else if (Beta) Alert1("Err. UpdateCMSettingsListBoxes():  MIDIGetSource() returned NULL.");
	}
	
	return err;
}

OSStatus UpdateCMSettingsListBoxes()
{
	OSStatus err;	
	ListHandle	inputLH, outputLH;
	ControlRef	cntl;
	
	if (CMSettings == NULL) return -1;
	
	// get list handles
	GetDialogItemAsControl(CMSettings, diInputList, &cntl);
	GetControlData(cntl, kControlEntireControl, kControlListBoxListHandleTag, sizeof(inputLH), &inputLH, NULL);
	
	GetDialogItemAsControl(CMSettings, diOutputList, &cntl);
	GetControlData(cntl, kControlEntireControl, kControlListBoxListHandleTag, sizeof(outputLH), &outputLH, NULL);
	
	// update source list
	err = UpdateListBox(inputLH, MIDIGetNumberOfSources, MIDIGetSource);
	
	// update destination list
	err = UpdateListBox(outputLH, MIDIGetNumberOfDestinations, MIDIGetDestination);
	
	DrawDialog(CMSettings);
	return noErr;
}
