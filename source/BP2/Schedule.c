/* Schedule.c (BP2 version CVS) */

// This is based on code communicated by Tom Demeyer from STEIM (tomd@xs4all.nl)
// I mainly changed the way events are linked together around places of the clock.
// On each place, events are now ordered chronologically.  This speeds up TDecr().
// In addition, routines with identical dates are executed in the same
// order as they have been received by the scheduler.

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

#include <Timer.h>
#include "-BP2decl.h"

// The following may be defined in a common project header:
// #define CLOCKSIZE 256	// must be a power of 2
// #define CLOCKLOG 8		// log base 2 of CLOCKSIZE
// #define CLOCKRES 5L		// 5ms clock resolution
// Note that we could use a power of 2 for CLOCKRES, e.g. 4, to replace
// divisions by CLOCKRES with right shifts.  I am not sure it makes a noticeable difference
// (Bernard)

// The following type may be defined in the project's common header:
// typedef OMSAPI(void) (*voidOMSdoPacket)(OMSMIDIPacket*,short,short);
// typedef struct Slice {
// 	voidOMSdoPacket routine;	// routine to be called at time n
// 	OMSMIDIPacket pkt;			// parameter for routine
// 	short dat1,dat2;			// parameters for routine
// 	long loopstep;				// number of clock cycles from the preceding event on
								// the same tick.
// 	struct Slice *next;			// next timeslice
// 	} Slice;

typedef struct {				// Time Manager information record
	TMTask atmTask;
	long tmRefCon;				// space to pass address of A5 world
	} TMInfo;

/* typedef struct {
	long loops,oldtotloops,totloops,loopstep,t,time,ticks,totalticks,tabs,timeslice;
	char size;
	} TestStruct;	$$$ */

// Function prototypes
pascal void TDecr(TMInfo*);
void TDecr68K(void);

// The following prototypes may be defined in the project's common header:
// int Cause(voidOMSdoPacket,Milliseconds,OMSMIDIPacket*,short,short);
// int InstallTMTask(void);
// int RemoveTMTask(void);

// The following globals may be defined in the project's common header:
// extern volatile unsigned long TotalTicks;
// extern volatile char ClockOverFlow,SchedulerIsActive,OKsend;
// extern char Mute;
// extern volatile unsigned long TimeSlice;		// current time slice
// extern Slice **Clock;
// extern Slice *SlicePool;

TMInfo myTMInfo;
TimerUPP myTimerProc;	
char DontSendMore;		// flag to avoid heap corruption
volatile char In_cause;	// flags to avoid heap corruption

/* int iTest=0;
TestStruct **p_Test; $$$ */

/*
Use 'Cause' to schedule a routine at a particular time.
Depending on when the routine is to be executed a time slice structure is
    inserted into one of CLOCKSIZE places around the 'clock'; this is to avoid having to check
    all scheduled routines at every tick.
'routine' will be called when due, with the three supplied parameters as arguments.

params:
	routine:		routine to be executed
	time:			the date (in milliseconds) at which it will be executed
	pkt,d1,d2:		various parameters to 'routine'

	returns			OK no error
				   	FAILED scheduler full
*/


Cause(voidOMSdoPacket routine,Milliseconds time,OMSMIDIPacket *p_pkt,short d1,short d2)
{
Slice *now,*event,*next,*prev;
long t,tabs,ticks,loops,totloops;

if(InitOn) return(OK);
if(SchedulerIsActive > 0 && !DontSendMore) {
	now = SlicePool;
	if(now) {
		/* 'ticks' is the number of ticks after which the routine must be executed, from now */
		In_cause = TRUE;
		ticks = (time / CLOCKRES) - TotalTicks;
		if(ticks < ZERO) ticks = ZERO;			// can't send to the past, eh?
		loops = ticks >> CLOCKLOG;				// how many loops around the 'clock'
		tabs = ticks & (CLOCKSIZE-1);			// absolute tick
		t = (tabs + TimeSlice) & (CLOCKSIZE-1);	// at what relative tick to insert?
		totloops = ZERO;
		event = Clock[t];
		prev = NULL;
		while(event) {
			next = event->next;
			if((totloops + event->loopstep) > loops) break;
			totloops += event->loopstep;
			prev = event;
			event = next;
			}
		if(prev) prev->next = now;
		else Clock[t] = now;
		SlicePool = now->next;
		now->next = event;
		
		now->loopstep = loops - totloops;
		if(event) event->loopstep -= now->loopstep;
		
		now->routine = routine;				// routine to call when time is up
		now->dat1 = d1;
		now->dat2 = d2;
		now->pkt = *p_pkt;					// data for routine
/*		(*p_Test)[iTest].size = p_pkt->len;
		(*p_Test)[iTest].totalticks = TotalTicks;
		(*p_Test)[iTest].ticks = ticks;
		(*p_Test)[iTest].tabs = tabs;
		(*p_Test)[iTest].timeslice = TimeSlice;
		(*p_Test)[iTest].loops = loops;
		(*p_Test)[iTest].totloops = totloops;
		(*p_Test)[iTest].loopstep = now->loopstep;
		(*p_Test)[iTest].t = t;
		(*p_Test)[iTest].time = time;
		if(iTest < 999) iTest++; $$$ */
		In_cause = FALSE;
		return(OK);
		}
	else return(ABORT); // buffer is full. Forget it!
	}
else return(AGAIN); // try again later
}


/*
Tdecr gets called every 'CLOCKRES' ms and checks the current timeslice to see if
anything needs to be done. If so, it removes the data from the current timeSlice in the 'clock'
and executes the routine stored therein.
*/

pascal void TDecr(TMInfo *tmTask)
{
Slice *now,*next,*prev;
OMSMIDIPacket pkt;
long loops;
voidOMSdoPacket routine;

TotalTicks++;

if(!In_cause && SchedulerIsActive > 0 && OKsend && !InitOn) {
	// when manipulating the list, stay away, TimeSlice is not
	// updated, so next time around we'll try the same slot again
	now = Clock[TimeSlice];	// linked list for current tick
	if(now) {	
		prev = NULL;
		while(now && (now->loopstep == ZERO)) {
			next = now->next;
			if(!Mute && (routine = now->routine) != (voidOMSdoPacket) NULL) {
				pkt = now->pkt;
				SchedulerIsActive = FALSE;
				/* Don't allow interruptions while we are doing all this */
				if(Oms) (*routine)(&pkt,now->dat1,now->dat2); // do it !!
				else if(InBuiltDriverOn) WriteInBuiltDriver(&pkt);
				SchedulerIsActive = TRUE;
				}
			now->routine = (voidOMSdoPacket) NULL;
			Clock[TimeSlice] = next;
			now->next = SlicePool;	// remove entry & put back into the pool
			SlicePool = now;
			now = next;
			}
		if(now) (now->loopstep)--;
		}
	TimeSlice = (TimeSlice + 1) & (CLOCKSIZE-1);		// bump up timeslice
	}
PrimeTime((QElemPtr)&(tmTask->atmTask),CLOCKRES);
}


// Stub for 68k interface
// Ah ah! This reminds me of the old good days.  BRUN BIGMAC and get into 6502 code...
// (Bernard)

#ifndef powerc
void TDecr68K(void)
{
asm {
	movem.l	a0-a6/d0-d7,-(sp)
	move.l	sizeof(TMTask)(a1),a5
	move.l	a1,-(sp)
	jsr		TDecr
	movem.l	(sp)+,a0-a6/d0-d7
	rts
	}
}
#endif


// I added this for flushing all events consecutive to a given date 'endtime'.
// When endtime = 0 all of them should be flushed. (Bernard)

FlushOutputEventQueueAfter(Milliseconds endtime)
{
long t,endt,endloops,endticks,totloops;
Slice *now,*next,*prev;
char toolate;

if(EmergencyExit) return(OK);

if(endticks <= ZERO) endticks = ZERO;	// can't change the past, eh?
else endticks = (endtime / CLOCKRES) - TotalTicks;
endloops = endticks >> CLOCKLOG;		// how many loops around the 'clock'
endt = endticks & (CLOCKSIZE-1);		// absolute tick
endt = (endt + TimeSlice) & (CLOCKSIZE-1);	// at what relative tick is the beginning of the end?

if(endtime == ZERO)
	SchedulerIsActive = FALSE;	// Don't store and don't play any more event
else
	DontSendMore = TRUE;		// Continue playing but don't store any more event

for(t=0; t < CLOCKSIZE; t++) {
	now = Clock[t];	// linked list for current tick
	totloops = ZERO;
	if(endtime == ZERO) toolate = TRUE;	// flush everything
	else toolate = FALSE;
	prev = NULL;
	while(now) {
		next = now->next;
		if(!toolate) {
			totloops += now->loopstep;
			if(totloops > endloops || (totloops == endloops && t > endt))
				toolate = TRUE;
			}
		if(toolate) {	// we inhibit the routine rather than returning the timeslice
					// to the spool.  This is faster.  TDecr() will return it.
			now->routine = (voidOMSdoPacket) NULL;
			now->loopstep = ZERO;	/* It should be recycled as fast as possibleÉ */
			}
/*		if(toolate)	{
			now->routine = (voidOMSdoPacket) NULL;
			if(prev) prev->next = next;
			else Clock[t] = next;
			now->next = SlicePool;	// remove entry & put back into the pool
			SlicePool = now;
			} */
		prev = now;
		now = next;					// get next entry for current timeslice
		}
	}
if(endtime == ZERO) {
	ClockOverFlow = FALSE;
	SchedulerIsActive = TRUE;
	}
DontSendMore = FALSE;
return(OK);
}


// Set up the interrupt routine

InstallTMTask(void)
{

In_cause = DontSendMore = ClockOverFlow = FALSE;
SchedulerIsActive = OKsend = FALSE;
TimeSlice = 0;

/* $$$ p_Test = (TestStruct**) NewHandle((Size)1000 * sizeof(TestStruct)); */

#ifndef powerc
	myTimerProc = NewTimerProc(TDecr68K);
	myTMInfo.tmRefCon = SetCurrentA5();			// store address of A5
#else
	myTimerProc = NewTimerProc(TDecr);
	myTMInfo.tmRefCon = 0;
#endif

myTMInfo.atmTask.tmAddr = myTimerProc;			// setup of task
myTMInfo.atmTask.tmWakeUp = 0;
myTMInfo.atmTask.tmReserved = 0;

InsXTime((QElemPtr) &myTMInfo.atmTask);			// xtime is accurate to some 50 microseconds
PrimeTime((QElemPtr) &myTMInfo.atmTask,CLOCKRES);	// install & activate the tdecr

SchedulerIsActive = OKsend = TRUE;
ClearMessage();
return(OK);
}


// Remove the interrupt routine

RemoveTMTask(void)
{

SchedulerIsActive = OKsend = FALSE;
ClockOverFlow = FALSE;

RmvTime((QElemPtr)&myTMInfo);
return(OK);
}


/* $$$
int TestIt(void);
TestIt(void)
{
int i;

for(i=0; i < iTest; i++) {
	sprintf(Message,"time=%ld size=%ld loops=%ld t=%ld loopstep=%ld totloops=%ld ticks=%ld totalticks=%ld tabs=%ld timeslice=%ld\n",
		(*p_Test)[i].time,(long)(*p_Test)[i].size,(*p_Test)[i].loops,(*p_Test)[i].t,(*p_Test)[i].loopstep,
		(*p_Test)[i].totloops,(*p_Test)[i].ticks,(*p_Test)[i].totalticks,(*p_Test)[i].tabs,(*p_Test)[i].timeslice);
	Print(wTrace,Message);
	}
return(OK);
} */