/* Ticks.c (BP2 version CVS) */ 

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


#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"


PlayTick(int before)
{
// Play time base ticks in real time

MIDI_Event e;
unsigned long time,nexttick,nextcommondate;
int i,imax,rs,itick,overflow,vel,ch,key,allzero,allfinished,newbeat;
long gap;
double fgap;

if(!IsMidiDriverOn() || !TickThere || !PlayTicks || SoundOn || ComputeOn || PlaySelectionOn
	|| LoadOn || InitOn || !OutMIDI) return(OK);

#if WITH_REAL_TIME_MIDI
if(Pclock < 1.) {
	PlayTicks = FALSE;
	Alert1("Can't play ticks because there is no clock");
	ShowWindow(Window[wMetronom]);
	BringToFront(Window[wMetronom]);
	ResetTickFlag = TRUE;
	return(OK);
	}
if(ResetTickFlag) ResetTicks(FALSE,TRUE,ZERO,0);
time = GetDriverTime() * Time_res;
if(before && time > (3 * Quantization)) time -= - (3 * Quantization);
fgap = (double) time - (double) NextBeatDate;
newbeat = FALSE;
if(fgap > EPSILON) {
	newbeat = TRUE;
	fgap = (double) ((unsigned long) time - NextBeatDate);	/* More accurate */
	fgap = fgap * (double) (Qclock) / (double) Pclock / 1000.;
	gap = 1L + (long) fgap;
	if(FoundNote && ReadKeyBoardOn && Jcontrol == -1 /* && !TickDone */) {
/*		if(gap < 16L) imax = gap;
		else imax = 1; */
		imax = 1;
		for(i=0; i < imax; i++) {
			if(EmptyBeat) Print(LastEditWindow,"_.");
			else {
				Print(LastEditWindow,".");
				EmptyBeat = TRUE;
				}
			}
		TickDone = EmptyBeat = TRUE;
		}
	if(BeatPosGrandCycle > ZERO && (BeatPosGrandCycle % BeatGrandCycle) == ZERO) {
		/* Resynchronize to maintain time accuracy */
		LastCommonBeatDate += GrandPeriod * (BeatPosGrandCycle / BeatGrandCycle);
		BeatPosGrandCycle = 1L;
		NextBeatDate = LastCommonBeatDate + (GrandPeriod / BeatPosGrandCycle)
			+ TICKDELAY;
		}
	else {
		BeatPosGrandCycle += gap;
		NextBeatDate = LastCommonBeatDate + TICKDELAY
			+ ((double)BeatPosGrandCycle * GrandPeriod) / BeatGrandCycle;
		}
	}
allfinished = allzero = TRUE;
for(itick=0; itick < MAXTICKS; itick++) {
	if(TickPosition[itick] != (TickCycle[itick] - 1)) allfinished = FALSE;
	if(TickPosition[itick] > 0) allzero = FALSE;
	time = GetDriverTime() * Time_res;
	fgap = (double) time - (double) NextTickDate[itick];
	if(fgap > EPSILON) {
		fgap = (unsigned long) time - NextTickDate[itick];	/* More accurate */
		fgap = fgap * (double) (Qclock * Ptick[itick]) / (double) Pclock
			/ (double) Qtick[itick] / 1000.;
		gap = (long) fgap;
		TickPosition[itick] = (TickPosition[itick] + gap) % TickCycle[itick];
		nexttick = NextTickDate[itick]
			+ ((1000. * (double) Pclock * gap * (double) Qtick[itick])
			/ ((double) Qclock * Ptick[itick]));
		nexttick += TICKDELAY;
		if(!MuteTick[itick] && (ThisTick[itick][TickPosition[itick]] % 2L)) {
			/* Send NoteOn */
			vel = TickVelocity[itick];
			ch = TickChannel[itick];
			key = TickKey[itick];
			FindTickValues(ThisTick[itick][TickPosition[itick]],&vel,&ch,&key);
			e.type = NORMAL_EVENT;
			e.time = nexttick / Time_res;
			e.status = NoteOn + ch - 1;
			e.data1 = key;
			e.data2 = vel;
			rs = 0;
			SendToDriver(nexttick,itick,&rs,&e);
			/* Send NoteOff */
			e.type = NORMAL_EVENT;
			e.time = (nexttick + TickDuration[itick]) / Time_res;
			e.status = NoteOn + ch - 1;
			e.data1 = key;
			e.data2 = 0;
			rs = 0;
			SendToDriver((nexttick + TickDuration[itick]),itick,&rs,&e);
			}
		TickPosition[itick] = (TickPosition[itick] + 1L) % TickCycle[itick];
		if(PosGrandCycle[itick] > ZERO
				&& (PosGrandCycle[itick] % GrandCycle[itick]) == ZERO) {
			/* Resynchronize to maintain time accuracy */
			LastCommonDate[itick] += GrandPeriod
				* (PosGrandCycle[itick] / GrandCycle[itick]);
			PosGrandCycle[itick] = 1L;
			NextTickDate[itick] = LastCommonDate[itick]
												+ (GrandPeriod / GrandCycle[itick]);
			}
		else {
			PosGrandCycle[itick] += gap + 1L;
			NextTickDate[itick] = LastCommonDate[itick]
				+ ((double)PosGrandCycle[itick] * GrandPeriod) / GrandCycle[itick];
			}
		}
	}
if(newbeat && TickCapture && allfinished) {
	if(!TickCaptureStarted) {
		TickCaptureStarted = YES;
		}
	else {
		PlayTicks = TickCapture = TickCaptureStarted = FALSE;
		ResetTickFlag = TRUE;
		if(Stream.i <= ZERO) Alert1("No events were captured...");
		else {
			(*Stream.code)[Stream.i].time = Stream.period;
			(*Stream.code)[Stream.i].byte = TimingClock;
			(*Stream.code)[Stream.i].sequence = 0;
			Stream.imax = Stream.i + 1;
			sprintf(Message,"A cycle of %ld ms has been captured and may be pasted to a sound-object prototype",
				(long)Stream.period);
			Alert1(Message);
			}
		Stream.pclock = (long) Pclock;
		Stream.qclock = (long) Qclock;
#if BP_CARBON_GUI
		SetPrototypePage1(iProto);
#endif /* BP_CARBON_GUI */
		return(STOP);
		}
	}
return(OK);
#endif
}


ResetTicks(int force,int zero,Milliseconds time,int pos)
{
// Reset tick cycles (real-time)

int i;
int itick;

if(!IsMidiDriverOn() || (!force && (!TickThere || !PlayTicks
	|| LoadOn || InitOn || !OutMIDI))) return(OK);

#if WITH_REAL_TIME_MIDI
if(zero) {
	time = GetDriverTime() * Time_res;
	}
NextBeatDate = time + TICKDELAY;
for(i=0; i < MAXTICKS; i++) {
	TickPosition[i] = (TickCycle[i] + pos) % TickCycle[i];
	NextTickDate[i] = time;
	PosGrandCycle[i] = ZERO;
	LastCommonDate[i] = time;
	}
LastCommonBeatDate = time;
BeatPosGrandCycle = ZERO;
EmptyBeat = TRUE; TickDone = FALSE;
ResetTickFlag = FALSE;
return(OK);
#endif
}


WaitForLastTicks(void)
/* Terminate real-time ticks if any */
{
long time;
int itick,messageshown;
unsigned long nexttick,maxticktime,timeleft,formertime;

if(OutMIDI && PlayTicks && TickThere && IsMidiDriverOn() && !PlaySelectionOn && !ComputeOn
		&& !LoadOn && !SoundOn && Nbytes > ZERO) {
#if WITH_REAL_TIME_MIDI
	maxticktime = ZERO;
/*	ShowMessage(TRUE,wMessage,"Playing ticks still in queue..."); */
	for(itick=0; itick < MAXTICKS; itick++) {
		nexttick = NextTickDate[itick] + TickDuration[itick] + TICKDELAY;
		if(nexttick > maxticktime) maxticktime = nexttick;
		}
	time = GetDriverTime();

	if(((long) (maxticktime/Time_res) - time) > (2000L/Time_res)) {
		sprintf(Message,"Playing ticks still in queue. (%ld seconds) Click to cancel",
			((long) maxticktime - (time * Time_res)) / 1000L);
		ShowMessage(TRUE,wMessage,Message);
		messageshown = TRUE;
		}
	else messageshown = FALSE;
	formertime = ZERO;
	while(time < (long)((maxticktime)/Time_res)) {
		time = GetDriverTime();
		timeleft = ((long) maxticktime - (time * Time_res)) / 1000L;
		if(messageshown && timeleft != formertime) {
			formertime = timeleft;
			sprintf(Message,"Playing ticks still in queue. (%ld seconds) Click to cancel",
				timeleft);
			ShowMessage(FALSE,wMessage,Message);
			PleaseWait();
			}
		if(Button()) break;
		}
	if(messageshown) HideWindow(Window[wMessage]);
	ResetTickFlag = TRUE;
#endif
	}
return(OK);
}


ClearCycle(int itick)
{
int j;

if(Answer("Clear all ticks in this cycle",'N') != OK) return(OK);
for(j=0; j < MAXBEATS; j++) ThisTick[itick][j] = ZERO;
return(OK);
}


InsertTickInItem(double fstreak,int clickon[],int hidden[],double tickdate[],
	int *p_rs,char ***p_keyon,double streakposition[],
	Milliseconds t0,int tickposition[],long imaxstreak)
// Play timebase ticks during item performance
{
int i,itick,pos,key,vel,ch,localchan,postick;
MIDI_Event e;
long time,istreak,localperiod,i1,i2,oldi2,gap;
double ffstreak;
Milliseconds oldt;

if(!OutMIDI) return(FAILED);

#if WITH_REAL_TIME_MIDI
for(itick=0; itick < MAXTICKS; itick++) {
	postick = tickposition[itick];
	time =  (t0 + tickdate[itick]) / Time_res;
	if(fstreak >= streakposition[itick] && postick >= 0
												&& (ThisTick[itick][postick] % 2L)) {
		key = TickKey[itick];
		vel = TickVelocity[itick];
		ch = TickChannel[itick];
		FindTickValues(ThisTick[itick][postick],&vel,&ch,&key);
		localchan = ch - 1;
		if(!MuteTick[itick] && !hidden[itick]) {
			if(clickon[itick]) {
				/* Send NoteOff if allowed */
				if(ByteToInt((*p_keyon[localchan])[key]) > 0) {
					(*p_keyon[localchan])[key]--;
					if((*p_keyon[localchan])[key] == 0) {
						e.time = time;
						e.type = NORMAL_EVENT;
						e.status = NoteOn + localchan;
						e.data1 = key;
						e.data2 = 0;
						SendToDriver(time * Time_res,itick,p_rs,&e);
						}
					}
				}
			else {
				if(!HideTicks) {
					/* Send NoteOn */
					if(((*p_keyon[localchan])[key]) > 0) {
						/* First send NoteOff if necessary */
						e.time = time;
						e.type = NORMAL_EVENT;
						e.status = NoteOn + localchan;
						e.data1 = key;
						e.data2 = 0;
						SendToDriver(time * Time_res,itick,p_rs,&e);
						}
					((*p_keyon[localchan])[key])++;
					e.time = time;
					e.type = NORMAL_EVENT;
					e.status = NoteOn + localchan;
					e.data1 = key;
					e.data2 = vel;
					SendToDriver(time * Time_res,itick,p_rs,&e);
					}
				}
			}
		clickon[itick] = 1 - clickon[itick];
		if(clickon[itick]) {
			if(HideTicks) hidden[itick] = TRUE;
			tickdate[itick] += TickDuration[itick];
			}
		else {
			hidden[itick] = FALSE;
			/* Increment tick position */
			pos = tickposition[itick];
			i = 0;
			while(TRUE) {
				i++;
				pos = (++pos) % TickCycle[itick];
				if(ThisTick[itick][pos] % 2L) break;
				}
			tickposition[itick] = pos;
			i1 = streakposition[itick];
			streakposition[itick] +=  ((Ratio / Kpress) * i
				* (double) Qtick[itick]) / (double) Ptick[itick];
			ffstreak = (double) streakposition[itick];
			istreak = (int) ffstreak;
			i2 = istreak;
			localperiod = LocalPeriod(&i1,&i2,imaxstreak);
			if(i2 < imaxstreak && (localperiod > -1L)) {
				tickdate[itick] = (*p_T)[i1] + (ffstreak - (double)i1) * localperiod;
				}
			else {
				tickposition[itick] = -1;	/* Tick beyond reach */
				tickdate[itick] = (double) DBL_MAX;
				}
			}
		}
	}
#endif
return(OK);
}


ResetTicksInItem(Milliseconds t1,int* tickposition,double* streakposition,
	double* tickdate,int* clickon,int* hidden,long imaxstreak,int *p_rs,char ***p_keyon)
// Reset tick cycles during item performance, exactly at the current time t1.
{
int itick,pos,oldi2,gap;
long localperiod,istreak,i1,i2;
Milliseconds oldt;
int key,vel,ch,localchan;
MIDI_Event e;
long time;

if(!OutMIDI) return(FAILED);

#if WITH_REAL_TIME_MIDI
for(itick=0; itick < MAXTICKS; itick++) {
	if(clickon[itick] && !hidden[itick] && !MuteTick[itick] && !HideTicks) {
		key = TickKey[itick];
		vel = TickVelocity[itick];
		ch = TickChannel[itick];
		FindTickValues(ThisTick[itick][tickposition[itick]],&vel,&ch,&key);
		localchan = ch - 1;
		/* Send NoteOff if allowed */
		if(ByteToInt((*p_keyon[localchan])[key]) > 0) {
			((*p_keyon[localchan])[key])--;
			if((*p_keyon[localchan])[key] == 0) {
				e.time = Tcurr;
				e.type = NORMAL_EVENT;
				e.status = NoteOn + localchan;
				e.data1 = key;
				e.data2 = 0;
				SendToDriver(Tcurr * Time_res,itick,p_rs,&e);
				}
			}
		}
	clickon[itick] = hidden[itick] = FALSE;
	for(pos=0; pos < TickCycle[itick]; pos++) {
		if(ThisTick[itick][pos] % 2L) break;
		}
	if(pos < TickCycle[itick]) {
		tickposition[itick] = pos;
		for(istreak=1; istreak < imaxstreak; istreak++) {
			if((*p_T)[istreak] >= t1) break;
			}
		if(istreak >= imaxstreak) return(FAILED);
		i1 = i2 = istreak;
		localperiod = LocalPeriod(&i1,&i2,imaxstreak);
		if(i2 < imaxstreak && (localperiod > -1L)) {
			tickdate[itick]
				= t1 + ((double) localperiod * (Ratio / Kpress) * pos
					* (double) Qtick[itick]) / (double) Ptick[itick];
			streakposition[itick]
				= istreak + (tickdate[itick] - (double) (*p_T)[istreak])
					/ ((double) localperiod);
			}
		else {
			tickposition[itick] = -1;	/* Tick beyond reach */
			tickdate[itick] = (double) DBL_MAX;
			}
		}
	else {
		tickposition[itick] = -1;	/* No tick on this cycle */
		tickdate[itick] = (double) DBL_MAX;
		}
	}
#endif
return(OK);
}


FindTickValues(long x,int *p_vel,int *p_ch,int *p_key)
{
// x = s + 2 * (vel + 128 * ((ch - 1) + 128 * key))
long s,y;

if(x < 2L) return(OK);
s = x % 2L;
*p_vel = ((x - s) / 2L) % 128L;
y = ((x - s) / 2L - *p_vel) / 128L;
*p_ch = (y % 128L) + 1L;
*p_key = (y - *p_ch + 1L) / 128L;
return(OK);
}


CaptureTicks(void)
{
int i,rep,playticks;
MIDIcode **ptr1;
int im,overflow;
unsigned long g,h,pmax,qmax,plcm,ppqlcm,ppq[MAXTICKS];

if(!IsMidiDriverOn() || SoundOn || ComputeOn || PlaySelectionOn || LoadOn || InitOn
	|| ItemCapture || !OutMIDI) return(OK);
	
#if WITH_REAL_TIME_MIDI
playticks = PlayTicks;
PlayTicks = FALSE;
rep = FAILED;

if(Pclock < 1.) {	/* Non measured smooth time */
	Alert1("Tick cycle cannot be captured because there is no clock");
#if BP_CARBON_GUI
	mMetronom(Nw);
#endif /* BP_CARBON_GUI */
	goto OUT;
	}
if(!TickThere) {
	Alert1("Tick cycle cannot be captured because no tick is active");
	goto OUT;
	}
if(Answer("Capture tick cycle as a MIDI stream and paste it later to a sound-object prototype",'Y')
	!= OK) goto OUT;
if(Stream.imax > ZERO) {
	if(Answer("Tick capture will delete previously captured stream of MIDI codes. Proceed anyway",
		'N') != OK) goto OUT;
	}
if(Stream.code == NULL) {
	if((ptr1 = (MIDIcode**) GiveSpace((Size) 200L * sizeof(MIDIcode))) == NULL) {
		rep = ABORT; goto OUT;
		}
	Stream.code = ptr1;
	}

plcm = LCMofTable(Ptick,MAXTICKS,&overflow);
/* Overflow has already been checked when setting TimeBase dialog */
for(i=0; i < MAXTICKS; i++)
	ppq[i] = (plcm / Ptick[i]) * Qtick[i] * TickCycle[i];
ppqlcm = LCMofTable(ppq,MAXTICKS,&overflow);
if(ppqlcm == ZERO || overflow) {
	qmax = ZERO;
	for(i=im=0; i < MAXTICKS; i++) {
		if(qmax < Qtick[i]) {
			im = i; qmax = Qtick[i];
			}
		}
	sprintf(Message,"Combination of cycles is too complex. You should reduce '%ld'",
		(long)Qtick[im]);
	Alert1(Message);
#if BP_CARBON_GUI
	SelectField(NULL,wTimeBase,fQratio+5*im,TRUE);
#endif /* BP_CARBON_GUI */
	goto OUT;
	}

Stream.period = (ppqlcm * 1000L * ((unsigned long) Pclock)) / ((unsigned long) Qclock);

ResetTickFlag = TRUE;
TickCapture = PlayTicks = TRUE;
TickCaptureStarted = FALSE;
Stream.i = Stream.imax = ZERO;
Stream.cyclic = TRUE;

i = 0; rep = OK;
do {
	if((rep=DoSystem()) != OK) break;
	if(Button()) {
		Stream.imax = ZERO;
		Alert1("Capture aborted");
		break;
		}
	if(++i > 25) {
		i = 0; PleaseWait();
		}
	}
// WARNING: This loop appears to rely on DoSystem() calling PlayTick()
// which can set TickCapture to FALSE in order to exit.
while(TickCapture);

OUT:
PlayTicks = playticks;
return(rep);
#endif
}


SetTimeBase(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char line[MAXFIELDCONTENT];
double p,q;

p = Pclock;
q = Qclock;
if(Pclock > 0.) {
	Simplify((double)INT_MAX,q,p,&Qclock,&Pclock);
	sprintf(line,"%.0f",Pclock);
	}
else {
	sprintf(line,"1");
	Qclock = 1L;
	}
#if BP_CARBON_GUI
SetField(NULL, wTimeBase, fP, line);
if(Pclock > 0.) sprintf(line,"%.0f",Qclock);
else sprintf(line,"[no clock]");
SetField(NULL, wTimeBase, fQ, line);
#endif /* BP_CARBON_GUI */
return(OK);
}


#if BP_CARBON_GUI

GetTimeBase(void)
{
Rect r;
Handle itemhandle;
short itemtype;
int i,j,rep;
char line1[MAXFIELDCONTENT],line2[MAXFIELDCONTENT];
Str255 t;
long pPclock,pQclock,qPclock,qQclock;
double oldp,oldq;

InputOn++;
oldp = Pclock; oldq = Qclock;

if((rep=GetField(NULL,TRUE,wTimeBase,fP,line1,&pPclock,&qPclock)) != OK) goto OUT;
if(pPclock < 0) {
	sprintf(Message,"Can't accept negative values like '%s'",line1);
	Alert1(Message); rep = FAILED; goto OUT;
	}
if(pPclock < 1.) goto NOCLOCK;

if((rep=GetField(NULL,TRUE,wTimeBase,fQ,line2,&pQclock,&qQclock)) != OK) goto OUT;
if(pQclock < 0) {
	sprintf(Message,"Can't accept negative values like '%s'",line2);
	Alert1(Message); rep = FAILED; goto OUT;
	}
if(pQclock == ZERO) goto NOCLOCK;

if((rep=Simplify((double)INT_MAX,(double)pPclock * qQclock,(double)qPclock * pQclock,
	&Pclock,&Qclock)) != OK) goto OUT;
if(qPclock != 1) {
	sprintf(Message,"Non-integer value '%s' has been converted.",line1);
	ShowMessage(TRUE,wMessage,Message);
	}
if(qQclock != 1) {
	sprintf(Message,"Non-integer value '%s' has been converted.",line2);
	ShowMessage(TRUE,wMessage,Message);
	}
goto OUT;

NOCLOCK:
Pclock = 0.;
Qclock = 1.;
SetTimeBase();
SetTempo();

OUT:
InputOn--;
if(oldp != Pclock || oldq != Qclock) {
	SetTimeBase();
	SetTempo();
	SetTickParameters(0,MAXBEATS);
	ResetTickFlag = TRUE;
	}
return(rep);
}


GetTickParameters(void)
{
long p,q,minduration,maxduration;
char line[MAXFIELDCONTENT];
int i,j,oldcycle,result;

InputOn++;
result = ABORT;
for(i=0; i < MAXTICKS; i++) {
	if(GetField(NULL,TRUE,wTimeBase,fPratio+5*i,line,&p,&q) == OK) {
		if(p/q < 1L || q != 1) {
			sprintf(Message,"Can't accept '%s' in speed ratio. Must be positive integer",
				line);
			Alert1(Message);
			sprintf(line,"[?]");
			SetField(NULL,wTimeBase,fPratio+5*i,line);
			SelectField(NULL,wTimeBase,fPratio+5*i,TRUE);
			return(FAILED);
			}
		else {
			if(Ptick[i] != p/q) {
				UpdateDirty(TRUE,wTimeBase);
				ResetTickFlag = TRUE;
				}
			Ptick[i] = p/q;
			}
		}
	else Ptick[i] = 1L;
	
	if(GetField(NULL,TRUE,wTimeBase,fQratio+5*i,line,&p,&q) == OK) {
		if(p/q < 1L || q != 1) {
			sprintf(Message,"Can't accept '%s' in speed ratio. Must be positive integer",
				line);
			Alert1(Message);
			sprintf(line,"[?]");
			SetField(NULL,wTimeBase,fQratio+5*i,line);
			SelectField(NULL,wTimeBase,fQratio+5*i,TRUE);
			return(FAILED);
			}
		else {
			if(Qtick[i] != p/q)	{
				UpdateDirty(TRUE,wTimeBase);
				ResetTickFlag = TRUE;
				}
			Qtick[i] = p/q;
			}
		}
	else Qtick[i] = 1L;
	
	if(GetField(NULL,TRUE,wTimeBase,fTickKey+55*i,line,&p,&q) == OK) {
		if(p/q < 0 || p/q > 127) {
			sprintf(Message,"Tick key should be in range 0..127. Can't accept '%ld'",
				(long)(p/q));
			Alert1(Message);
			sprintf(line,"[?]");
			SetField(NULL,wTimeBase,fTickKey+55*i,line);
			SelectField(NULL,wTimeBase,fTickKey+55*i,TRUE);
			return(FAILED);
			}
		else {
			if(TickKey[i] != p/q)  UpdateDirty(TRUE,wTimeBase);
			TickKey[i] = p/q;
			}
		}
	else TickKey[i] = 64;
	
	if(GetField(NULL,TRUE,wTimeBase,fTickChannel+55*i,line,&p,&q) == OK) {
		if(p/q < 1 || p/q > 16) {
			sprintf(Message,"Tick channel should be in range 1..16. Can't accept '%ld'",
				(long)(p/q));
			Alert1(Message);
			sprintf(line,"[?]");
			SetField(NULL,wTimeBase,fTickChannel+55*i,line);
			SelectField(NULL,wTimeBase,fTickChannel+55*i,TRUE);
			return(FAILED);
			}
		else {
			if(TickChannel[i] != p/q)  UpdateDirty(TRUE,wTimeBase);
			TickChannel[i] = p/q;
			}
		}
	else TickChannel[i] = 1;
	
	if(GetField(NULL,TRUE,wTimeBase,fTickVelocity+55*i,line,&p,&q) == OK) {
		if(p/q < 1 || p/q > 127) {
			sprintf(Message,"Tick velocity should be in range 1..127. Can't accept '%ld'",
				(long)(p/q));
			Alert1(Message);
			sprintf(line,"[?]");
			SetField(NULL,wTimeBase,fTickVelocity+55*i,line);
			SelectField(NULL,wTimeBase,fTickVelocity+55*i,TRUE);
			return(FAILED);
			}
		else {
			if(TickVelocity[i] != p/q) UpdateDirty(TRUE,wTimeBase);
			TickVelocity[i] = p/q;
			}
		}
	else TickVelocity[i] = 127;
	
	if(GetField(NULL,TRUE,wTimeBase,fTickDuration+3*i,line,&p,&q) == OK) {
		if(Pclock > 0.) {
			minduration = 2 * Time_res;
			maxduration = 1000L * Pclock * Qtick[i] / Qclock / Ptick[i] - minduration - 2L;
			if(maxduration < minduration) maxduration = minduration;
			if(p/q < minduration || p/q > maxduration) {
				ShowWindow(Window[wTimeAccuracy]);
				BringToFront(Window[wTimeAccuracy]);
				sprintf(Message,"Tick duration must be in range %ld .. %ldms.\n(See time resolution)\nCan't accept '%ld'",
					(long)minduration,(long)maxduration,(long)(p/q));
				Alert1(Message);
				if(p/q > maxduration) TickDuration[i] = maxduration;
				else TickDuration[i] = minduration;
				UpdateDirty(TRUE,wTimeBase);
				sprintf(line,"%ld",(long)TickDuration[i]);
				if(SetField(NULL,wTimeBase,fTickDuration+3*i,line) != OK) goto ERR;
				}
			else {
				if(TickDuration[i] != p/q) UpdateDirty(TRUE,wTimeBase);
				TickDuration[i] = p/q;
				}
			}
		else TickDuration[i] = p/q;
		}
	else  TickDuration[i] = 2 * Time_res;
	
	if(GetField(NULL,TRUE,wTimeBase,fTickCycle+55*i,line,&p,&q) == OK) {
		if(p/q < 1 || p/q > MAXBEATS) {
			sprintf(Message,"Cycle should be in range 1..%ld. Can't accept '%ld'",
				(long)MAXBEATS,(long)(p/q));
			Alert1(Message);
			sprintf(line,"[?]");
			SetField(NULL,wTimeBase,fTickCycle+55*i,line);
			SelectField(NULL,wTimeBase,fTickCycle+55*i,TRUE);
			return(FAILED);
			}
		else {
			if(TickCycle[i] != p/q) {
				oldcycle = TickCycle[i];
				TickCycle[i] = p/q;
				TickThere = FALSE;
				SetTickParameters(i+1,oldcycle);
				UpdateDirty(TRUE,wTimeBase);
				ResetTickFlag = TRUE;
				}
			}
		}
	else {
		oldcycle = TickCycle[i];
		TickCycle[i] = 4;
		SetTickParameters(i+1,oldcycle);
		UpdateDirty(TRUE,wTimeBase);
		ResetTickFlag = TRUE;
		}
	}
result = OK;

ERR:
InputOn--;
return(result);
}

#endif /* BP_CARBON_GUI */


SetTickParameters(int itick,int oldcycle)
{
char line[MAXFIELDCONTENT];
int i,j,jj,im,overflow;
Rect r;
short itemtype;
Handle itemhandle;
unsigned long g,h,pmax,qmax,plcm,ppqlcm,ppq[MAXTICKS];

PleaseWait();
if(itick > 0) {
#if BP_CARBON_GUI
	i = itick - 1;
	for(j=0; j < TickCycle[i]; j++) {
		jj = dPlayBeat + 55*i + j;
		GetDialogItem(gpDialogs[wTimeBase],(short)jj,&itemtype,&itemhandle,&r);
		if(ThisTick[i][j] < 2) HiliteControl((ControlHandle) itemhandle,11);
		if(ThisTick[i][j] % 2L) {
			SwitchOn(NULL,wTimeBase,jj);
			TickThere = TRUE;
			}
		else SwitchOff(NULL,wTimeBase,jj);
		}
	for(j=TickCycle[i]; j < oldcycle; j++) {
		jj = dPlayBeat + 55*i + j;
		SwitchOff(NULL,wTimeBase,jj);
		GetDialogItem(gpDialogs[wTimeBase],(short)jj,&itemtype,&itemhandle,&r);
		HiliteControl((ControlHandle) itemhandle,0);
		}
#endif /* BP_CARBON_GUI */
	}
else {
#if BP_CARBON_GUI
	for(i=0; i < MAXTICKS; i++) {
		PleaseWait();
		sprintf(line,"%ld",(long)TickKey[i]);
		SetField(NULL,wTimeBase,fTickKey+55*i,line);
		sprintf(line,"%ld",(long)TickChannel[i]);
		SetField(NULL,wTimeBase,fTickChannel+55*i,line);
		sprintf(line,"%ld",(long)TickVelocity[i]);
		SetField(NULL,wTimeBase,fTickVelocity+55*i,line);
		sprintf(line,"%ld",(long)TickCycle[i]);
		SetField(NULL,wTimeBase,fTickCycle+55*i,line);
		sprintf(line,"%ld",(long)Ptick[i]);
		SetField(NULL,wTimeBase,fPratio+5*i,line);
		sprintf(line,"%ld",(long)Qtick[i]);
		SetField(NULL,wTimeBase,fQratio+5*i,line);
		sprintf(line,"%ld",(long)TickDuration[i]);
		SetField(NULL,wTimeBase,fTickDuration+3*i,line);
		}
#endif /* BP_CARBON_GUI */
	plcm = LCMofTable(Ptick,MAXTICKS,&overflow);
	if(plcm == ZERO || overflow) {
		pmax = ZERO;
		for(i=im=0; i < MAXTICKS; i++) {
			if(pmax < Ptick[i]) {
				im = i; pmax = Ptick[i];
				}
			}
		sprintf(Message,"Combination of cycles is too complex. You should reduce '%ld'",
			(long)Ptick[im]);
		Alert1(Message);
#if BP_CARBON_GUI
		SelectField(NULL,wTimeBase,fPratio+5*im,TRUE);
#endif /* BP_CARBON_GUI */
		PlayTicks = FALSE;
		goto OUT;
		}
	for(i=0; i < MAXTICKS; i++) {
		if(Ptick[i] == ZERO) {
			if(Beta) Alert1("Err. SetTickParameters(). Ptick[i] == ZERO");
			continue;
			}
		ppq[i] = (plcm / Ptick[i]) * Qtick[i];
		}
	ppqlcm = LCMofTable(ppq,MAXTICKS,&overflow);
	if(ppqlcm == ZERO || overflow) {
		qmax = ZERO;
		for(i=im=0; i < MAXTICKS; i++) {
			if(qmax < Qtick[i]) {
				im = i; qmax = Qtick[i];
				}
			}
		sprintf(Message,"Combination of cycles is too complex. You should reduce '%ld'",
			(long)Qtick[im]);
		Alert1(Message);
#if BP_CARBON_GUI
		SelectField(NULL,wTimeBase,fQratio+5*im,TRUE);
#endif /* BP_CARBON_GUI */
		PlayTicks = FALSE;
		goto OUT;
		}
	g = GCD((double)ppqlcm,plcm);
	if(g == ZERO) {
		if(Beta) Println(wTrace,"Err. SetTickParameters(). g == ZERO");
		g = 1L;
		}
	BeatGrandCycle = ppqlcm / g;
	if(Qclock > ZERO)
		GrandPeriod = (1000. * (double) Pclock * BeatGrandCycle) / Qclock;
	else {
		if(Beta) Println(wTrace,"Err. SetTickParameters(). Qclock == ZERO");
		goto OUT;
		}
	h = plcm / g;
	for(i=0; i < MAXTICKS; i++) {
		PleaseWait();
		if(ppq[i] == ZERO) {
			if(Beta) Println(wTrace,"Err. SetTickParameters(). ppq[i] == ZERO");
			continue;
			}
		GrandCycle[i] = (ppqlcm / ppq[i]) * h;
		}
	}
OUT:
#if BP_CARBON_GUI
if(PlayTicks) SwitchOn(NULL,wTimeBase,dPlayTicks);
else SwitchOff(NULL,wTimeBase,dPlayTicks);
for(i=0; i < MAXTICKS; i++) {
	if(MuteTick[i]) SwitchOn(NULL,wTimeBase,dMute + 5*i);
	else SwitchOff(NULL,wTimeBase,dMute + 5*i);
	}
#endif /* BP_CARBON_GUI */
/* StopWait(); */
return(OK);
}
