/* MIDIstubs.c (BP3) */

/* Contains substitute functions for MIDIdrivers.
   Used when no MIDI drivers are being compiled. */
   
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

/* Test to see if we can use gettimeofday() in <sys/time.h> */
#ifndef HAVE_SYS_TIME_H
  #if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
    #include <unistd.h>
    #if defined(_POSIX_VERSION) && _POSIX_VERSION > 200100L
      /* This is a conservative test assuming that the needed features will be 
         available if the POSIX version is 2001 or later.  You can also just
		 define HAVE_SYS_TIME_H with your build options if you know it is available. */
      #define HAVE_SYS_TIME_H
	#endif
  #endif
#endif

/* There is no standard library function in ISO C for getting better
   than one-second precision from the system clock.  Therefore, we use
   various libraries depending on the operating system. */
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#include <stdint.h>
#elif BP_CARBON_GUI_FORGET_THIS
// no special headers needed here to get Carbon TickCount() function
#else
#include <time.h>
#endif

#ifndef _H_BP2
#include "-BP2.h"
#endif

#ifndef BP2_DECL_H
#include "-BP2decl.h"
#endif

MIDIClientRef midiClient;
MIDIPortRef MIDIoutPort;
MIDIEndpointRef MIDIdestination;

/* ClockZero is the time the driver considers "zero".  It is stored 
   in different types depending on the time library being used. */
#ifdef HAVE_SYS_TIME_H
static int64_t ClockZero = 0;
#elif BP_CARBON_GUI_FORGET_THIS
static unsigned long ClockZero = 0;
#else
static time_t ClockZero = 0;
#endif

int initializeMIDISystem() {
    #if defined(_WIN32) || defined(_WIN64)
    UINT deviceId = 0; // Typically, 0 represents the default MIDI device
    MMRESULT result = midiOutOpen(&hMidiOut, deviceId, 0, 0, CALLBACK_NULL);
    if (result != MMSYSERR_NOERROR) {
        fprintf(stderr, "Error opening MIDI output device.\n");
        return -1;
    }
    else return(0);
    #elif defined(__APPLE__)
        OSStatus status;
        BPPrintMessage(odInfo,"Initializing MacOS MIDI system (CoreMIDI)\n");
        status = MIDIClientCreate(CFSTR("MIDIcheck Client"),NULL,NULL,&midiClient);
        if(status != noErr) {
            BPPrintMessage(odError,"=> Error: Could not create MIDI client.\n");
            return -1;
            }
        status = MIDIOutputPortCreate(midiClient, CFSTR("Output Port"),&MIDIoutPort);
        if(status != noErr) {
            BPPrintMessage(odError,"=> Error: Could not create output port.\n");
            return -1;
            }
        ItemCount MIDIdestinationCount = MIDIGetNumberOfDestinations();
        if(MIDIdestinationCount == 0) {
            BPPrintMessage(odError,"=> Error: No MIDI destinations available.\n");
            return -1;
            }
        for(ItemCount i = 0; i < MIDIdestinationCount; ++i) {
            MIDIdestination = MIDIGetDestination(i);
            CFStringRef endpointName = NULL;
            if (MIDIObjectGetStringProperty(MIDIdestination, kMIDIPropertyName, &endpointName) == noErr) {
                char name[64];
                CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                BPPrintMessage(odInfo,"Destination %lu: %s\n", i + 1, name);
                CFRelease(endpointName);
                }
            }
        // Assuming the first destination
        MIDIdestination = MIDIGetDestination(0);
        return(0);
    #elif defined(__linux__)
        BPPrintMessage(odInfo,"Initializing Linux MIDI system\n");
        if(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
            BPPrintMessage(odError,"=> Error: Opening ALSA sequencer.\n");
            return -1;
            }
        snd_seq_set_client_name(seq_handle, "My MIDI Application");
        out_port = snd_seq_create_simple_port(seq_handle, "Out Port",SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,SND_SEQ_PORT_TYPE_APPLICATION);
        if(out_port < 0) {
            BPPrintMessage(odError,"=> Error: Creating sequencer port.\n");
            return -1;
            }
        else return(0);
    #endif
    }

void closeMIDISystem() {
    BPPrintMessage(odInfo,"Closing MIDI system\n");
    #if defined(_WIN32) || defined(_WIN64)
    // Windows MIDI cleanup
    printf("Closing Windows MIDI system\n");
    if (hMidiOut != NULL) {
        midiOutClose(hMidiOut);  // Close the MIDI output device
        hMidiOut = NULL;         // Reset the handle to NULL after closing
        }
    #elif defined(__APPLE__)
        // MacOS MIDI cleanup
        MIDIPortDispose(MIDIoutPort);
        MIDIClientDispose(midiClient);

    #elif defined(__linux__)
        // Linux MIDI cleanup
        if(seq_handle != NULL) {
            snd_seq_close(seq_handle);  // Close the ALSA sequencer
            seq_handle = NULL;          // Reset the handle to NULL after closing
            }
    #endif
    InBuiltDriverOn = FALSE;
    }

void sendMIDIEvent(unsigned char* midiData,int dataSize,long time) {
    int note,status,value,test_first_events,improvize;
    long clocktime;
    
    test_first_events = 0;
    status = midiData[0];
    note = midiData[1];
    value = midiData[2];
    if(test_first_events && NumEventsWritten < 500) {
        clocktime = getClockTime() - initTime;
        improvize = Improvize;
        Improvize = 0; // Necessary to activate BPPrintMessage(odInfo,...
        if(status == NoteOn || status == NoteOff)
            BPPrintMessage(odInfo,"%.3f -> %.3f s status = %d, note = %d, value = %d\n",(float)clocktime/1000000,(float)time/1000000,status,note,value);
        else
            BPPrintMessage(odInfo,"%.3f -> %.3f s event %d-%d-%d\n",(float)clocktime/1000000,(float)time/1000000,status,note,value);
        Improvize = improvize;
        }
    if(NumEventsWritten < LONG_MAX) NumEventsWritten++;
 //   BPPrintMessage(odInfo,"Sending MIDI event time = %ld ms, status = %ld, note %ld, value = %ld\n",(long)time/1000L,(long)status,(long)note,(long)value);
    #if defined(_WIN32) || defined(_WIN64)
    // Windows MIDI event sending
    // Ensure MIDI device is opened
    if (hMidiOut == NULL) {
        if (midiOutOpen(&hMidiOut, 0, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
            fprintf(stderr, "Error opening MIDI output.\n");
            return;
        }
    }
    // Pack the bytes into a DWORD message
    DWORD msg = 0;
    for (int i = 0; i < dataSize; i++) {
        msg |= (midiData[i] << (i * 8));
    }
    // Send the MIDI message
    midiOutShortMsg(hMidiOut, msg);
    #elif defined(__APPLE__)
        // MacOS MIDI event sending
        MIDIPacketList packetList;
        MIDIPacket *packet = MIDIPacketListInit(&packetList);
        packet = MIDIPacketListAdd(&packetList, sizeof(packetList), packet, mach_absolute_time(), dataSize, midiData);
        if(packet) MIDISend(MIDIoutPort, MIDIdestination, &packetList);
    #elif defined(__linux__)
        // Ensure ALSA sequencer is setup
        if(seq_handle == NULL) {
            if(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
                BPPrintMessage(odError,"=> Error opening ALSA sequencer.\n");
                return;
                }
            snd_seq_set_client_name(seq_handle, "MIDI Sender");
            out_port = snd_seq_create_simple_port(seq_handle, "Out",SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,SND_SEQ_PORT_TYPE_APPLICATION);
            }
        // Create an ALSA MIDI event
        snd_seq_event_t ev;
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_source(&ev, out_port);
        // Send the data
        for(int i = 0; i < dataSize; i++) {
            snd_seq_ev_set_noteon(&ev, 0, midiData[i], midiData[++i]);  // Channel, note, velocity
            snd_seq_event_output(seq_handle, &ev);
            snd_seq_drain_output(seq_handle);
            }
    #endif
    }

void MIDIflush() {
    unsigned long currentTime = getClockTime();
    currentTime -= initTime;
 //   BPPrintMessage(odInfo,"currentTime = %ld eventCount = %ld\n",(long)currentTime,(long)eventCount);
    long i = 0;
    long time;
    unsigned char midiData[4];
    int dataSize = 3;
    if(Panic) {
        eventCount = 0L;
        AllNotesOffAllChannels();
        }
    while(i < eventCount) {
        if(stop() == ABORT) {
            eventCount = 0L;
            return;
            }
        if(eventStack[i].time <= currentTime) {
            midiData[0] = eventStack[i].status;
            midiData[1] = eventStack[i].data1;
            midiData[2] = eventStack[i].data2;
            time = eventStack[i].time;
            sendMIDIEvent(midiData,dataSize,time);
            // Move remaining events forward
            memmove(&eventStack[i], &eventStack[i + 1], (eventCount - i - 1) * sizeof(MIDI_Event));
            eventCount--;
            }
        else i++; 
        }
    }

long getClockTime(void) {
    long the_time; // Microseconds
    #if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq); // Get the frequency of the high-resolution performance counter
    QueryPerformanceCounter(&count);  // Get the current value of the performance counter
    the_time = (unsigned long)((count.QuadPart * 1000000) / freq.QuadPart);
    #elif defined(__APPLE__)
        the_time = clock_gettime_nsec_np(CLOCK_UPTIME_RAW) / 1000L;
    #elif defined(__linux__)
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts); // CLOCK_MONOTONIC provides uptime, not affected by system time changes
        the_time = (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
    #endif
    return the_time;
    }

/*  Null driver is always on */
Boolean IsMidiDriverOn()
{
	return TRUE;
}

/*  Reading a MIDI event with the Null driver always fails */
int GetNextMIDIevent(MIDI_Event *p_e,int loop,int force)
{
	return(MISSED);
	
	// The code below can loop infinitely
	/* while(TRUE) {
	NEXTRY:
		if(!loop) return(MISSED);
		if(!force && Button()) return(ABORT);
	}
	return(OK); */
}


/*  DriverWrite is the general-purpose "high-level" function for 
    writing a MIDI event to the current driver.  This stub just increments
    a counter of how many events have been written. */
OSErr DriverWrite(Milliseconds time,int nseq,MIDI_Event *p_e)
{
	/* FIXME: should we do the ItemCapture bit from the original DriverWrite() ?
		(Break it out into a function?) - akozar */
	++NumEventsWritten;
	
	if(!OutMIDI || MIDIfileOn) return(noErr);

	/* FIXME: should we also register program changes to the MIDI orchestra ? */
	return noErr;
}


/*  ResetMIDI() mimics the basic functionality of its counterpart in
    MIDIdrivers.c */
int ResetMIDI(int wait)
{
	if(!OutMIDI || AEventOn) return(OK);

//	FlushDriver();
	WaitABit(200L);
//	ResetDriver();

	ResetTicks(FALSE,TRUE,ZERO,0);

	return(OK);
}


/*  FlushDriver() is supposed to remove any pending Midi events from the
    driver's queue (or scheduler).  Null driver just reports how many events
    were written. */
int FlushDriver()
{
	RunningStatus = 0;
	sprintf(Message, "FlushDriver(): null driver wrote %ld events.", NumEventsWritten);
	ShowMessage(TRUE,wMessage,Message);
	NumEventsWritten = 0;
	return(OK);
}


/*  ResetDriver() mimics the basic functionality of its counterpart in 
    MIDIdrivers.c */
int ResetDriver()
{
	if(SetOutputFilterWord() != OK) return(ABORT);
	RunningStatus = 0;
	return(OK);
}


/*  SetDriver() mimics the basic functionality of its counterpart in 
    MIDIdrivers.c */
int SetDriver()
{
/*	if(SetOutputFilterWord() != OK) return(ABORT);
	SetDriverTime(ZERO);
	RunningStatus = 0; */
	return(OK);
}


/* We do need to keep accurate time with these functions so that
   WaitForLastSeconds, etc. will work.  - akozar */

/* GetDriverTime() returns a value that is the number of milliseconds
   since the driver clock's "zero point" divided by Time_res. */
unsigned long GetDriverTime(void)
{
	unsigned long dtime;
	
#ifdef HAVE_SYS_TIME_H
	// prefer Unix time functions to Carbon's TickCount()
	struct timeval clocktime;
	struct timezone tzone;
	int64_t clockcurrent, sincezero;
	
	gettimeofday(&clocktime, &tzone);
	// timeval splits clock time into two values: seconds and microseconds
	// convert everything to microseconds
	clockcurrent = (int64_t)(clocktime.tv_sec) * 1000000 + clocktime.tv_usec;
	sincezero = clockcurrent - ClockZero;

	// 1 unit of 'dtime' = Time_res milliseconds = Time_res * 1000 microseconds
	dtime = (unsigned long)(sincezero / ((int64_t)Time_res * (int64_t)1000));
	
#elif BP_CARBON_GUI_FORGET_THIS
	/* The null driver on Carbon uses system ticks (1/60 seconds) as
       its internal time representation. */
	unsigned long sincezero;

	// dtime = (clock() * 60) / Time_res;  // overflows unsigned long
	sincezero = TickCount() - ClockZero;
	dtime = (unsigned long)(((double)sincezero * (1000.0/60.0)) / (double)Time_res);

// #elif WIN32
	/* On Windows, we could use the GetSystemTime() function as described here:
	   http://msdn.microsoft.com/en-us/library/ms724950%28v=VS.85%29.aspx
	 */

#else
	// if there is no other option, then we use the Std. C time() function :(
	time_t sincezero;

	sincezero = time(NULL) - ClockZero;
	// convert sincezero (in seconds) to "driver time" (milliseconds/Time_res)
	dtime = (unsigned long)((sincezero * 1000) / Time_res);
#endif

	return(dtime);
}


/*  SetDriverTime() sets the current time in the same units returned
	by GetDriverTime() above (milliseconds/Time_res) */
int SetDriverTime(long dtime)
{

#ifdef HAVE_SYS_TIME_H
	// prefer Unix time functions to Carbon's TickCount()
	struct timeval clocktime;
	struct timezone tzone;
	int64_t clockcurrent, offset;
	
	gettimeofday(&clocktime, &tzone);
	// timeval splits clock time into two values: seconds and microseconds
	// convert everything to microseconds
	clockcurrent = (int64_t)(clocktime.tv_sec) * 1000000 + clocktime.tv_usec;
	// 1 unit of 'dtime' = Time_res milliseconds = Time_res * 1000 microseconds
	offset = (int64_t)(dtime) * Time_res * 1000;
	// save the clock time that we are calling "zero"
	ClockZero = clockcurrent - offset;
	
#elif BP_CARBON_GUI_FORGET_THIS
	unsigned long clockcurrent, offset;
	
	clockcurrent = TickCount();
	offset = (unsigned long)((double)(dtime * Time_res) * 0.060);
	/* save the clock time that we are calling "zero" */
	ClockZero = clockcurrent - offset;

// #elif WIN32
	/* On Windows, we could use the GetSystemTime() function as described here:
	   http://msdn.microsoft.com/en-us/library/ms724950%28v=VS.85%29.aspx
	 */

#else
	// if there is no other option, then we use the Std. C time() function :(
	time_t clockcurrent, offset;

	clockcurrent = time(NULL);
	offset = (time_t)((dtime * Time_res) / 1000);
	/* save the clock time that we are calling "zero" */
	ClockZero = clockcurrent - offset;
#endif

	return(OK);
}
