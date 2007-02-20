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

#include <CoreAudio/CoreAudio.h>
#include <CoreMIDI/CoreMIDI.h>

static Boolean		CoreMidiDriverOn = 0;
static MIDIPortRef	CMOutPort = NULL;
static MIDIEndpointRef	CMDest = NULL;
static MIDIClientRef	CMClient = NULL;

static MIDITimeStamp	ClockZero = 0;	// CoreAudio's host time that we consider "zero"

/*  Returns whether the CoreMIDI driver is on */
Boolean IsMidiDriverOn()
{
	return CoreMidiDriverOn;
}


void	EnumerateCMDevices()
{
	// Enumerate available CoreMidi devices
	// This code is from the Echo.cpp CM example
	int i, n;
	CFStringRef pname, pmanuf, pmodel;
	char name[64], manuf[64], model[64];
	
	n = MIDIGetNumberOfDevices();
	for (i = 0; i < n; ++i) {
		MIDIDeviceRef dev = MIDIGetDevice(i);
		
		MIDIObjectGetStringProperty(dev, kMIDIPropertyName, &pname);
		MIDIObjectGetStringProperty(dev, kMIDIPropertyManufacturer, &pmanuf);
		MIDIObjectGetStringProperty(dev, kMIDIPropertyModel, &pmodel);
		
		CFStringGetCString(pname, name, sizeof(name), 0);
		CFStringGetCString(pmanuf, manuf, sizeof(manuf), 0);
		CFStringGetCString(pmodel, model, sizeof(model), 0);
		CFRelease(pname);
		CFRelease(pmanuf);
		CFRelease(pmodel);

		sprintf(Message, "name=%s, manuf=%s, model=%s", name, manuf, model);
		Println(wTrace, Message);
	}
}

OSStatus InitCoreMidiDriver()
{
	OSStatus err;
	CFStringRef pname;
	int numdest;
	char name[64];
	
	err = MIDIClientCreate(CFSTR("Bol Processor"), NULL, NULL, &CMClient);
	if (err == noErr) {
		ShowMessage(TRUE, wMessage, "Signed into CoreMIDI.");
		err = MIDIOutputPortCreate(CMClient, CFSTR("BP OutPort"), &CMOutPort);
		if (err == noErr) {
			EnumerateCMDevices();
			
			// find the first destination
			// (code adapted from the Echo.cpp CM example)
			numdest = MIDIGetNumberOfDestinations();
			if (numdest > 0) CMDest = MIDIGetDestination(0);

			if (CMDest != NULL) {
				MIDIObjectGetStringProperty(CMDest, kMIDIPropertyName, &pname);
				CFStringGetCString(pname, name, sizeof(name), 0);
				CFRelease(pname);
				sprintf(Message, "Sending Midi to destination: %s.", name);
				ShowMessage(TRUE, wMessage, Message);
				CoreMidiDriverOn = TRUE;
			}
			else ShowMessage(TRUE, wMessage, "No MIDI destinations present.");
		}			
	}
	
	return err;
}

GetNextMIDIevent(MIDI_Event *p_e,int loop,int force)
{
	return(FAILED);

	/*if(Oms) return(PullMIDIdata(p_e)); 
		
	while(TRUE) {
		p_e->type = RAW_EVENT;
		if(DriverRead(p_e) != noErr) goto NEXTRY;
		if(p_e->type != RAW_EVENT) goto NEXTRY;
		return(OK);
		
	NEXTRY:
		if(!loop) return(FAILED);
		if(!force && Button()) return(ABORT);
		}
	return(OK);*/
}


OSErr DriverWrite(Milliseconds time,int nseq,MIDI_Event *p_e)
{
	int result;
	
	if(!CoreMidiDriverOn && OutMIDI) {
		if(Beta) Println(wTrace,"Err. DriverWrite(). Driver is OFF");
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

		if (CMDest != NULL) {
			MIDISend(CMOutPort, CMDest, pktlist);
			Nbytes++;
		}
		else FlashInfo("MIDI output destination was not found ... check CoreMIDI setup!");
	}
	
	return(noErr);
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
	if (CMDest != NULL)  err = MIDIFlushOutput(CMDest);
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
	int i,ch,rep,rs,channel;
	OSErr io;
	MIDI_Event e;

	rep = OK;

	if(!OutMIDI || (AEventOn && !CoreMidiDriverOn)) return(OK);

	if(!CoreMidiDriverOn) {
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
