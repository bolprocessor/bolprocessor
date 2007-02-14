/* MIDIstubs.c (BP2 version CVS) */

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

static unsigned long NumEventsWritten = 0;
static unsigned long ClockZero = 0;

/*  Null driver is always on */
Boolean IsMidiDriverOn()
{
	return TRUE;
}

/*  Reading a MIDI event with the Null driver always fails */
int GetNextMIDIevent(MIDI_Event *p_e,int loop,int force)
{
	return(FAILED);
	
	// The code below can loop infinitely
	/* while(TRUE) {
	NEXTRY:
		if(!loop) return(FAILED);
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

	FlushDriver();
	WaitABit(200L);
	ResetDriver();

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
	if(SetOutputFilterWord() != OK) return(ABORT);
	SetDriverTime(ZERO);
	RunningStatus = 0;
	return(OK);
}


/* We do need to keep accurate time with these functions so that
   WaitForLastSeconds, etc. will work.  - akozar */
/* GetDriverTime() returns a value that is the number of milliseconds
   since the driver clock's "zero point" divided by Time_res.
   SetDriverTime() sets the current time in these units. */
/* The null driver uses system ticks (1/60 seconds) as its internal
   time representation. */
unsigned long GetDriverTime(void)
{
	unsigned long time, sincezero;

	// time = (clock() * 60) / Time_res;  // overflows unsigned long
	sincezero = TickCount() - ClockZero;
	time = (unsigned long)(((double)sincezero * (1000.0/60.0)) / (double)Time_res);
	return(time);
}

int SetDriverTime(long time)
{
	unsigned long clockcurrent, offset;
	
	clockcurrent = TickCount();
	offset = (unsigned long)((double)(time * Time_res) * 0.060);
	/* save the clock time that we are calling "zero" */
	ClockZero = clockcurrent - offset;
	return(OK);
}
