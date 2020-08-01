/* MIDIdrivers.c (BP2 version CVS) */

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

// This file should be the only one edited to include OMS driver calls

#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"


/*  Returns whether OMS or built-in driver is on */
Boolean IsMidiDriverOn()
{
	return (Oms || InBuiltDriverOn);
}

GetNextMIDIevent(MIDI_Event *p_e,int loop,int force)
{
int result;
long i;

if(Oms) return(PullMIDIdata(p_e)); 
	
while(TRUE) {
	p_e->type = RAW_EVENT;
	if(DriverRead(p_e) != noErr) goto NEXTRY;
	if(p_e->type != RAW_EVENT) goto NEXTRY;
	return(OK);
	
NEXTRY:
	if(!loop) return(FAILED);
	if(!force && Button()) return(ABORT);
	}
return(OK);
}


OSErr DriverRead(MIDI_Event *ptr)
{
OSErr io;
long count = 12L;

if(!Oms) {
	if(!InBuiltDriverOn) {
		if(Beta) Alert1("Err. DriverRead(). Driver is OFF");
		return(1);
		}
	ptr->type += Portbit;
	Pb.ioBuffer = (Ptr) ptr;
	Pb.ioReqCount = count;
	Pb.ioPosOffset = ZERO;
	Pb.ioCompletion = ZERO;
	Pb.ioPosMode = 0;
	
	/* Don't allow writing while reading */
	OKsend = FALSE;
	io = PBRead((ParmBlkPtr)p_Pb,FALSE);
	OKsend = TRUE;
	
	if(io != noErr) TellError(51,io);
	}
else {
	/* $$$ */
	if(Beta) Alert1("Err. DriverRead(). Oms is ON");
	return(noErr);
	}
return(io);
}


OSErr DriverWrite(Milliseconds time,int nseq,MIDI_Event *p_e)
{
OSErr io;
long count = 12L;
MIDIcode **ptr;
long size;
int result,i,j,thisevent,channel,program;
Milliseconds currenttime;
OMSMIDIPacket pkt;
char status,data1,data2;
short itemtype;
ControlHandle itemhandle;
Rect r;

if(!InBuiltDriverOn && OutMIDI && !Oms) {
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

if(Oms || NEWTIMER) {
	if(time < ZERO) {
		if(Beta) {
			sprintf(Message,"Err. DriverWrite(). time  = %ld",(long)time);
			Println(wTrace,Message);
			}
		time = ZERO;
		}
	pkt.flags = 0;
	pkt.appConnRefCon = 'Bel0';
	pkt.smpteTimeStamp = 0;
	switch(p_e->type) {
		case RAW_EVENT:
			pkt.len = 1;
			pkt.data[0] = ByteToInt(p_e->data2);
			break;
		case TWO_BYTE_EVENT:
			pkt.len = 2;
			pkt.data[0] = ByteToInt(p_e->status);
			pkt.data[1] = ByteToInt(p_e->data2);
			break;
		case NORMAL_EVENT:
			pkt.len = 3;
			pkt.data[0] = ByteToInt(p_e->status);
			pkt.data[1] = ByteToInt(p_e->data1);
			pkt.data[2] = ByteToInt(p_e->data2);
			break;
		}
	/* Send OMSWritePacket2() event to the time scheduler */
	/* If NEWTIMER is true while Oms is false, the scheduler will only use the packet and send it to the in-built MIDI driver */
	if(gOutNodeRefNum != OMSInvalidRefNum) {
		i = 0;
		do {
			result = Cause(OMSWritePacket2,time,&pkt,gOutNodeRefNum,gOutputPortRefNum);
			i++;
			}
		while(result == AGAIN && i < 1000);
		Nbytes++;
		}
	else FlashInfo("Output MIDI port was not found... Check OMS setup!");
	if(result == ABORT) {
		if(Beta) Println(wTrace,"Err. DriverWrite(). Cause() returned ABORT");
		io = memFullErr;
		}
	else io = noErr;
	}
else {	/* Using in-built MIDI driver with its own time scheduler */
	status = p_e->status;
	data1 = p_e->data1;
	data2 = p_e->data2;
	if(p_e->type == TWO_BYTE_EVENT) {	/* Not standard with this driver */
		p_e->type = RAW_EVENT;
		p_e->data2 = status;
		p_e->type += Portbit;	/* Changing value depending on printer or modem port */
		Pb.ioBuffer = (Ptr) p_e;
		Pb.ioReqCount = count;
		Pb.ioPosOffset = ZERO;
		Pb.ioCompletion = ZERO;
		Pb.ioPosMode = 0;
		
		io = PBWrite((ParmBlkPtr)p_Pb,FALSE);
		Nbytes++;
		
		if(io != noErr) TellError(52,io);
		p_e->type = RAW_EVENT;
		p_e->data2 = data2;
		Nbytes++;
		}
	p_e->type += Portbit;	/* Changing value depending on printer or modem port */
	Pb.ioBuffer = (Ptr) p_e;
	Pb.ioReqCount = count;
	Pb.ioPosOffset = ZERO;
	Pb.ioCompletion = ZERO;
	Pb.ioPosMode = 0;
	
	io = PBWrite((ParmBlkPtr)p_Pb,FALSE);
	Nbytes++;
	
	if(io != noErr) TellError(53,io);
	}
return(io);
}


WriteInBuiltDriver(OMSMIDIPacket *p_pkt)
/* Write the packet immediately to the in-built MIDI driver */
/* Beware that this is called at interrupt time! */
{
OSErr io;
long time,count = 12L;
int result;
MIDI_Event e;
char status,data1,data2;
MIDI_Parameters parms;

io = DriverStatus(CLOCKTIME_CODE,(MIDI_Parameters*) &parms);
time = parms.clockTime;
time += 50L;
	
switch(p_pkt->len) {
	case 1:
		e.type = RAW_EVENT;
		e.data2 = p_pkt->data[0];
		break;
	case 2:
		e.type = TWO_BYTE_EVENT;
		e.status = p_pkt->data[0];
		e.data2 = p_pkt->data[1];
		break;
	case 3:
		e.type = NORMAL_EVENT;
		e.status = p_pkt->data[0];
		e.data1 = p_pkt->data[1];
		e.data2 = p_pkt->data[2];
		break;
	}
status = e.status;
data1 = e.data1;
data2 = e.data2;
if(e.type == TWO_BYTE_EVENT) {	/* Not standard with this driver */
	e.time = time;
	e.type = RAW_EVENT;
	e.data2 = status;
	e.type += Portbit;	/* Changing value depending on printer or modem port */
	Pb.ioBuffer = (Ptr) &e;
	Pb.ioReqCount = count;
	Pb.ioPosOffset = ZERO;
	Pb.ioCompletion = ZERO;
	Pb.ioPosMode = 0;
	
	io = PBWrite((ParmBlkPtr)p_Pb,FALSE);
	Nbytes++;
	
	e.type = RAW_EVENT;
	e.data2 = data2;
	Nbytes++;
	}
e.time = time;
e.type += Portbit;	/* Changing value depending on printer or modem port */
Pb.ioBuffer = (Ptr) &e;
Pb.ioReqCount = count;
Pb.ioPosOffset = ZERO;
Pb.ioCompletion = ZERO;
Pb.ioPosMode = 0;

io = PBWrite((ParmBlkPtr)p_Pb,FALSE);
Nbytes++;

return(OK);
}

	
FixPort(int i)
{
// Choosing between printer and modem serial ports

if(Oms) return(OK);

if(!InBuiltDriverOn) {
	if(Beta) Alert1("Err. FixPort(). Driver is OFF");
	return(ABORT);
	}
sprintf(Message,"MIDI output is now '%s' port",Portname[i]);
Alert1(Message);
Dirty[iSettings] = TRUE;

if(!OutMIDI) {
	OutMIDI = TRUE;
	SetButtons(TRUE);
	}
ResetMIDI(FALSE);
Port = i;
ResetDriver();
	
Port = i;
switch(Port) {
	case 1:
		Portbit = PORTA; break;
	case 2:
		Portbit = PORTB; break;
	}
return(OK);
}


SetReceiveRaw(void)
{
int i;

if(Oms) return(OK);
else {
	if(!InBuiltDriverOn) {
		if(Beta) Alert1("Err. SetReceiveRaw(). Driver is off");
		return(ABORT);
		}
	i = RAW_EVENT;
	if(Port == 1) DriverControl(READMODE_CODEA,(MIDI_Parameters*)&i);
	if(Port == 2) DriverControl(READMODE_CODEB,(MIDI_Parameters*)&i);
	}
return(OK);
}


FlushDriver(void)
{
MIDI_Parameters parms;
MIDI_Event e;
DriverDataPtr driver;
OSErr ierr;
ParmBlkPtr pb;
int i;
	
RunningStatus = 0;
Nbytes = Tbytes2 = ZERO;

if(Oms || NEWTIMER) {
	/* Flushing output */
	FlushOutputEventQueueAfter(ZERO);
	TotalTicks = ZERO;
	}
	
	/* Flushing input */
if(Oms) {
	BytesReceived = BytesProcessed = ZERO;
	DownBuffer = TRUE; OMSinputOverflow = FALSE;
	return(OK);
	}
	
if(!InBuiltDriverOn) {
	if(Beta) Alert1("Err. FlushDriver(). Driver is OFF");
	return(ABORT);
	}

if(InitOn || NEWTIMER) return(OK);

if(Port == 1) DriverControl(CLEAR_RECEIVE_CODEA,(MIDI_Parameters*) &parms);
if(Port == 2) DriverControl(CLEAR_RECEIVE_CODEB,(MIDI_Parameters*) &parms);

if(Port == 1) DriverControl(CLEAR_TRANSMIT_CODEA,(MIDI_Parameters*) &parms);
if(Port == 2) DriverControl(CLEAR_TRANSMIT_CODEB,(MIDI_Parameters*) &parms);

FlushEvents(driverEvt,0);

WaitABit(100L);

if(p_Pb->ioRefNum == 0) {
	if(Beta)  Alert1("Err. FlushDriver(). p_Pb->ioRefNum == 0");
	return(OK);
	}
return(OK);
}


ResetDriver(void)
{
MIDI_Parameters parms;
long tr;
int i;

if(SetOutputFilterWord() != OK) return(ABORT);

if(Oms || NEWTIMER) {
/*	FlushOutputEventQueueAfter(ZERO); */
	goto OUT;
	}

if(!InBuiltDriverOn) {
	if(Beta) Alert1("Err. ResetDriver(). Driver is OFF");
	return(ABORT);
	}
	
if(Port == 1) {
	DriverControl(CLOSE_CODEA,(MIDI_Parameters*) &parms);
	DriverControl(OPEN_CODEA,(MIDI_Parameters*) &parms);
	parms.clockTime = MIDIinputFilter;
	DriverControl(FILTER_CODEA,(MIDI_Parameters*) &parms);
	}
if(Port == 2) {
	DriverControl(CLOSE_CODEB,(MIDI_Parameters*) &parms);
	DriverControl(OPEN_CODEB,(MIDI_Parameters*) &parms);
	parms.clockTime = MIDIinputFilter;
	DriverControl(FILTER_CODEB,(MIDI_Parameters*) &parms);
	}
	
tr = Time_res;
DriverControl(TICKSIZE_CODE,(MIDI_Parameters*)&tr);

OUT:

SetDriverTime(ZERO);

Tcurr = Nbytes = Tbytes2 = ZERO;
ResetTickFlag = TRUE;
Dirty[wTimeAccuracy] = FALSE;

SetReceiveRaw();

RunningStatus = 0;
return(OK);
}


SetDriver(void)
{
MIDI_Parameters parms;
long tr;

if(Oms) {
	SetOMSdriver();
	goto OUT;
	}
	
if(!InBuiltDriverOn) {
	if(Beta) Alert1("Err. SetDriver(). Driver is OFF");
	return(ABORT);
	}
if(SetOutputFilterWord() != OK) return(ABORT);
	
if(Port == 1) {
	DriverControl(OPEN_CODEA,(MIDI_Parameters*) &parms);
	parms.clockTime = MIDIinputFilter;
	DriverControl(FILTER_CODEA,(MIDI_Parameters*) &parms);
	}
if(Port == 2) {
	DriverControl(OPEN_CODEB,(MIDI_Parameters*) &parms);
	parms.clockTime = MIDIinputFilter;
	DriverControl(FILTER_CODEB,(MIDI_Parameters*) &parms);
	}

tr = Time_res;
DriverControl(TICKSIZE_CODE,(MIDI_Parameters*) &tr);

OUT:

SetDriverTime(ZERO);
Tcurr = Nbytes = Tbytes2 = ZERO;
Dirty[wTimeAccuracy] = FALSE;

SetReceiveRaw();

RunningStatus = 0;
return(OK);
}


CloseCurrentDriver(int quick)
{
OSErr io;
/* It is probably never necessary to close an OMS driver */

FlushDriver();
WaitABit(200L);
ResetDriver();
if(!quick) WaitABit(500L);	/* 500ms */
// if(Oms) InBuiltDriverOn = FALSE;
io = DriverClose();
if(io == noErr) return(OK);
else TellError(54,io);
return(FAILED);
}


ResetMIDI(int wait)
{
int i,ch,rep,rs,channel;
OSErr io;
MIDI_Event e;

rep = OK;

if(!OutMIDI || (AEventOn && !Oms && !NEWTIMER)) return(OK);

if(!InBuiltDriverOn && !Oms) {
	if(Beta) Alert1("Err. ResetMIDI(). Driver is OFF");
	DriverOpen("\p.MIDI");
	goto RESET;
	}
	
if(wait && !InitOn && !WaitOn) rep = WaitForLastTicks();

if(wait && rep == OK && !InitOn && !WaitOn) rep = WaitForEmptyBuffer();
	
RESET:

WaitABit(200L);
FlushDriver();
WaitABit(200L);
ResetDriver();

ResetTicks(FALSE,TRUE,ZERO,0);

return(rep);
}


unsigned long GetDriverTime(void)
{
unsigned long time;
OSErr io;
MIDI_Parameters parms;

if(Oms || NEWTIMER) {
	time = (TotalTicks * CLOCKRES) / Time_res;
	return(time);
	}
io = DriverStatus(CLOCKTIME_CODE,(MIDI_Parameters*) &parms);
time = parms.clockTime;
return(time);
}


SetDriverTime(long time)
{
MIDI_Parameters parms;
OSErr io;

if(Oms || NEWTIMER) {
	FlushOutputEventQueueAfter(time * Time_res);
	SchedulerIsActive--;
	TotalTicks = (time * Time_res) / CLOCKRES;
	TimeSlice = TotalTicks & (CLOCKSIZE-1);
	SchedulerIsActive++;
	if(Oms) return(OK);
	}
parms.clockTime = time;
io = DriverControl(CLOCKTIME_CODE,(MIDI_Parameters*) &parms);
return(OK);
}


// -----------------  LOW-LEVEL STUFF FOR IN-BUILT DRIVER ----------------


OSErr DriverStatus(short csCode,MIDI_Parameters* csParam)
{
short i;
OSErr err;
CntrlParam c;

if(!InBuiltDriverOn) {
	if(Beta) Alert1("Err. DriverStatus(). Driver is OFF");
	return(1);
	}
c.ioCompletion = 0L;
c.ioVRefNum = Pb.ioVRefNum;
c.ioCRefNum = Pb.ioRefNum;
c.csCode = csCode;
err = PBStatus((ParmBlkPtr)&c,FALSE);
for(i=0; i < 10; i++) ((short*)csParam)[i] = c.csParam[i];
return(err);
}


OSErr DriverOpen(unsigned char name[])
{
OSErr io;

#if USE_BUILT_IN_MIDI_DRIVER
if(InBuiltDriverOn) {
	if(Beta) Alert1("Err. DriverOpen(). Driver already ON");
	return(1);
	}
Pb.ioCompletion = 0L;
Pb.ioNamePtr = name;
Pb.ioPermssn = fsCurPerm;
Pb.ioMisc = 0L;
Pb.ioVRefNum = RefNumbp2;
Pb.ioVersNum = 0;
io = PBOpen((ParmBlkPtr)p_Pb,FALSE);
if(io != noErr) TellError(55,io);
if(p_Pb->ioRefNum == 0) {
	if(Beta)  Alert1("Err. DriverOpen(). p_Pb->ioRefNum == 0");
	return(noErr);
	}
InBuiltDriverOn = TRUE;
if(!NEWTIMER) MaxMIDIbytes = MIDI_FIFO_MAX - 50;
return(io);
#else
  InBuiltDriverOn = FALSE;
  return -1;
#endif
}


OSErr DriverClose(void)
{
OSErr io;
ParmBlkPtr pb;
long tr;


if(!InBuiltDriverOn) {
	if(Beta) Alert1("Err. DriverClose(). Driver already OFF");
	return(noErr);
	}

/* tr = 10L;
DriverControl(TICKSIZE_CODE,(MIDI_Parameters*) &tr); */

io = DriverKill();
InBuiltDriverOn = FALSE;

if(io == noErr) {
	if(p_Pb->ioRefNum == 0) {
		if(Beta)  Alert1("Err. DriverClose(). p_Pb->ioRefNum == 0");
		return(noErr);
		}

	return(io);
	}
else {
	TellError(56,io);
	return(io);
	}
}


Errors(DriverDataPtr driver)
{
sprintf(Message,"XmtCountA=%ld XmtCountB=%ld ",(long)driver->xmtCountA,(long)driver->xmtCountB);
Print(wTrace,Message);
sprintf(Message,"XmtErrorA=%ld XmtErrorB=%ld ",(long)driver->xmtErrorA,(long)driver->xmtErrorB);
Print(wTrace,Message);
sprintf(Message,"RcvCountA=%ld RcvCountB=%ld ",(long)driver->rcvCountA,(long)driver->rcvCountB);
Print(wTrace,Message);
sprintf(Message,"RcvErrorA=%ld RcvErrorB=%ld ",(long)driver->rcvErrorA,(long)driver->rcvErrorB);
if(driver->lastvalid) {
	sprintf(Message,"lastvalid\n");
	Print(wTrace,Message);
	}
return(OK);
}


EmptyDriverInput(void)
{
MIDI_Parameters parms;
DriverDataPtr driver;

if(!InBuiltDriverOn) {
	if(Beta) Alert1("Err. EmptyDriverInput(). Driver is OFF");
	return(ABORT);
	}
DriverStatus(30,(MIDI_Parameters*) &parms);
driver = (DriverDataPtr) parms.clockTime;
if(driver->waitQueue[0]) return(NO);
return(YES);
}


OSErr DriverKill(void)
{
CntrlParam c;

if(!InBuiltDriverOn) {
	if(Beta) Alert1("Err. DriverKill(). Driver is OFF");
	return(notOpenErr);
	}
c.ioCompletion = 0L;
c.ioVRefNum = Pb.ioVRefNum;
c.ioCRefNum = Pb.ioRefNum;
c.csCode = 1;
return(PBControl((ParmBlkPtr)&c,0));
}


OSErr DriverControl(short csCode,MIDI_Parameters* csParam)
{
short i;
CntrlParam c;

if(!InBuiltDriverOn) {
	if(Beta) Alert1("Err. DriverControl(). Driver is OFF");
	return(1);
	}
c.ioCompletion = 0L;
c.ioVRefNum = Pb.ioVRefNum;
c.ioCRefNum = Pb.ioRefNum;
c.csCode = csCode;
for(i=0; i < 10; i++) c.csParam[i] = ((short*)csParam)[i];
return(PBControl((ParmBlkPtr) &c,0));
}


Events(DriverDataPtr driver)
{
short i=0, count=0;
TaskPtr task,remtasks;

if(!InBuiltDriverOn) {
	if(Beta) Alert1("Err. Events(). Driver is OFF");
	return(OK);
	}
while(i < WAIT_CACHE_SIZE) {
	if(driver->waitQueue[i]) {
		sprintf(Message,"\n%ld:",(long)i);
		Print(wTrace,Message);
		remtasks = driver->waitQueue[i];
		while(task = remtasks) {
			sprintf(Message,"exec = %lx ",task->exectime);
			Print(wTrace,Message);
			count++;
			remtasks = task->link;
			}
		}
	i++;
	}
sprintf(Message,"\ncount = %ld\n",(long)count);
Print(wTrace,Message);
return(OK);
}


DriverTime(DriverDataPtr driver)
{
sprintf(Message,"Time : %lx",driver->time);
ShowMessage(TRUE,wMessage,Message);
return(OK);
}