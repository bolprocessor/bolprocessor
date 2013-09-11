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
#elif BP_CARBON_GUI
// no special headers needed here to get Carbon TickCount() function
#else
#include <time.h>
#endif

#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"

static unsigned long NumEventsWritten = 0;

/* ClockZero is the time the driver considers "zero".  It is stored 
   in different types depending on the time library being used. */
#ifdef HAVE_SYS_TIME_H
static int64_t ClockZero = 0;
#elif BP_CARBON_GUI
static unsigned long ClockZero = 0;
#else
static time_t ClockZero = 0;
#endif


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
	
#elif BP_CARBON_GUI
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
	
#elif BP_CARBON_GUI
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
