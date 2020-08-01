/* SoundObjects2.c (BP2 version CVS) */

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


CheckPrototypeSize(int j)
{
if((*p_MIDIsize)[j] > ZERO || (*p_CsoundSize)[j] > ZERO) return(OK);
else return(FAILED);
}


CheckDuration(int j)
{
if(j >= Jbol) {
	if(Beta) Alert1("Err. CheckDuration()");
	return(FAILED);
	}
if(j < 2 || (*p_Dur)[j] < EPSILON) {
	Alert1("Invalid option because the duration of this object is null");
	return(FAILED);
	}
else return(OK);
}


#if BP_CARBON_GUI
RecordPrototype(int j)
{
int r;
long imax,oldfilter;
Handle ptr;

if(p_Code != NULL) {
	if(Beta) Alert1("MIDI input had not been cleared properly. Will proceed anyway");
	ptr = (Handle) p_Code;
	MyDisposeHandle(&ptr);
	p_Code = NULL;
	}
imax = 10000L;	/* Initial size of MIDI buffer. Will be resized if needed. */
if((p_Code = (MIDIcode**) GiveSpace((Size)imax * sizeof(MIDIcode))) == NULL) return(ABORT);
HideWindow(Window[wMessage]);
if(ResetControllers) ResetMIDIControllers(YES,YES,NO);
oldfilter = MIDIinputFilter;
MIDIinputFilter = 0xffffffffL;
if(!Oms) SetDriver();
InputOn = TRUE;
r = LoadMIDIprototype(j,imax);
HideWindow(Window[wInfo]);
SuppressAllNotesOff(NO,j);
MIDIinputFilter = oldfilter;
if(!Oms) SetDriver();
InputOn = FALSE;
ptr = (Handle) p_Code;
MyDisposeHandle(&ptr);
p_Code = NULL;
ChangedProtoType(j);
return(r);
}
#endif /* BP_CARBON_GUI */


AdjustDuration(int j,Milliseconds newdur)
{
double ratio,preroll,postroll,dur,t;
long i;
int longerCsound;
Milliseconds oldeventrange,neweventrange;

if(j < 0 || j >= Jbol) {
	if(Beta) {
		sprintf(Message,"Err. AdjustDuration(). j = %ld",(long)j);
		Alert1(Message);
		}
	return(FAILED);
	}

if((*p_Dur)[j] <= EPSILON) {
	Alert1("Duration of sound-object prototype is null. Can't be rescaled");
	return(OK);
	}
if(Answer("Adjusting duration can't be undone. Proceed anyway",'N') != YES)
	return(FAILED);
GetPrePostRoll(j,&preroll,&postroll);

oldeventrange = (*p_Dur)[j];
neweventrange = newdur;
ratio = ((double)neweventrange) / oldeventrange;
(*p_Dur)[j] = newdur;

if(DurationToPoint(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

if(ShowMessages && (*p_MIDIsize)[j] > ZERO)
	ShowMessage(TRUE,wMessage,"Adjusting durations of MIDI events...");
for(i=ZERO; i < (*p_MIDIsize)[j]; i++) {
	t = (*((*pp_MIDIcode)[j]))[i].time - preroll;
	if(t >= 0.) {	/* Modify only those events within the time-span interval */
		t *= ratio;
		(*((*pp_MIDIcode)[j]))[i].time = t + preroll;
		}
	}
if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

if(ShowMessages && (*p_CsoundSize)[j] > ZERO)
	ShowMessage(TRUE,wMessage,"Adjusting durations of Csound events...");
if((*p_CsoundSize)[j] > ZERO) {
	if(DurationToPoint(NULL,pp_CsoundTime,p_CsoundSize,j) != OK) return(ABORT);
	for(i=ZERO; i < (*p_CsoundSize)[j]; i++) {
		t = (*((*pp_CsoundTime)[j]))[i] - preroll;
		if(t >= 0.) {	/* Modify only those events within the time-span interval */
			t *= ratio;
			(*((*pp_CsoundTime)[j]))[i] = t + preroll;
			dur = (*((*pp_CsoundScore)[j]))[i].duration * ratio;
			(*((*pp_CsoundScore)[j]))[i].duration = dur;
			}
		}
	if(PointToDuration(NULL,pp_CsoundTime,p_CsoundSize,j) != OK) return(ABORT);
	DeCompileObjectScore(j);
	}

ChangedProtoType(j);
return(OK);
}


AdjustVelocities(int j,int vmin,int vmax)
{
double a,b;
int c,c0,v,vmin0,vmax0,allsame;
long i;

if(CheckNonEmptyMIDI(j) != OK) return(FAILED);
if(vmin == vmax) {
	allsame = TRUE;
	}
else {
	allsame = FALSE;
	vmin0 = 127; vmax0 = 0;
	for(i=0; i < (*p_MIDIsize)[j]-2; i++) {
		c = (*((*pp_MIDIcode)[j]))[i].byte;
		c0 = c - (c % 16);
		v = (*((*pp_MIDIcode)[j]))[i+2].byte;
		if(c0 == NoteOn && v > 0) {
			if(v < vmin0) vmin0 = v;
			if(v > vmax0) vmax0 = v;
			}
		}
	if(vmin0 == 127 || vmax0 == 0) {
		Alert1("Velocity range is not significant in this object");
		return(OK);
		}
	if(vmax0 == vmin0) a = 0.;
	else a = ((double)(vmax - vmin)) / (vmax0 - vmin0);
	}
if(Answer("Adjusting velocities can't be undone. Proceed anyway",'N') != YES)
	return(FAILED);
for(i=0; i < (*p_MIDIsize)[j]-2; i++) {
	c = (*((*pp_MIDIcode)[j]))[i].byte;
	v = (*((*pp_MIDIcode)[j]))[i+2].byte;
	c0 = c - (c % 16);
	if(c0 == NoteOn && v > 0) {
		if(!allsame) (*((*pp_MIDIcode)[j]))[i+2].byte = a * (v - vmin0) + vmin;
		else (*((*pp_MIDIcode)[j]))[i+2].byte = vmin;
		}
	}
ChangedProtoType(j);
return(OK);
}


QuantizeNoteOn(int j)
{
int c,c0,q;
long i,size;
Milliseconds time,tmin;
Handle ptr;
double preroll,postroll;

if(CheckNonEmptyMIDI(j) != OK) return(FAILED);
if((*p_Quan)[j] < EPSILON) {
	if(Beta) Alert1("Err. QuantizeNoteOn()");
	return(OK);
	}
	
if(Answer("Quantizing can't be undone. Proceed anyway",'N') != YES)
	return(FAILED);
if(DurationToPoint(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

tmin = ZERO;
for(i=0; i < (*p_MIDIsize)[j] - 2; i++) {
	c = (*((*pp_MIDIcode)[j]))[i].byte;
	if((*((*pp_MIDIcode)[j]))[i].time < tmin) (*((*pp_MIDIcode)[j]))[i].time = tmin;
	else tmin = (*((*pp_MIDIcode)[j]))[i].time;
	c0 = c - (c % 16);
	if(c0 == NoteOn) {
		if((*((*pp_MIDIcode)[j]))[i+2].byte == 0) continue;	/* Velocity = 0 ==> NoteOff */
		time = (*((*pp_MIDIcode)[j]))[i].time;
		q = (int) ((((double) time) / (*p_Quan)[j]) + 0.5);
		tmin = (*((*pp_MIDIcode)[j]))[i].time = (*((*pp_MIDIcode)[j]))[i+1].time
		 = (*((*pp_MIDIcode)[j]))[i+2].time = q * (*p_Quan)[j];
		i += 2;
		}
	}
/* Force resulting time list to be monotonous */
size = (*p_MIDIsize)[j];
time = (*((*pp_MIDIcode)[j]))[size - 1].time;
for(i=size-1; i >= 0; i--) {
	if((*((*pp_MIDIcode)[j]))[i].time > time) (*((*pp_MIDIcode)[j]))[i].time = time;
	else time = (*((*pp_MIDIcode)[j]))[i].time;
	}
if(size > 0) {
	GetPrePostRoll(j,&preroll,&postroll);
	(*p_Dur)[j] = ((*((*pp_MIDIcode)[j]))[size-1].time) - preroll + postroll;
	if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
	}
else {
	(*p_Dur)[j] = ZERO;
	ptr = (Handle)(*pp_MIDIcode)[j];
	if(MyDisposeHandle(&ptr) != OK) return(ABORT);
	(*pp_MIDIcode)[j] = NULL;
	}
ChangedProtoType(j);
return(OK);
}


ExpandDurations(int j,Milliseconds mindur)
{
int k,c,c0,ch;
MIDIcode **p_MIDI,**ptr;
long i,ii;
Size size;
Milliseconds t;
int **p_keyon[MAXCHAN];
Milliseconds **p_start[MAXCHAN];
Handle ptr3;
double preroll,postroll;

if(CheckNonEmptyMIDI(j) != OK) return(FAILED);
if(Answer("Expand durations can't be undone. Proceed anyway",'N') != YES)
	return(FAILED);
for(ch=0; ch < MAXCHAN; ch++) {
	if((p_keyon[ch] = (int**) GiveSpace((Size)(MAXKEY+1)*sizeof(int)))
			== NULL) return(ABORT);
	for(k=0; k < MAXKEY; k++) (*p_keyon[ch])[k] = 0;
	}
	
if(DurationToPoint(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

size = MyGetHandleSize((Handle) (*pp_MIDIcode)[j]);
if((p_MIDI = (MIDIcode**) GiveSpace((Size) size+size)) == NULL) return(ABORT);
/* size = MyGetHandleSize((Handle) (*pp_MIDIcode)[j]);
if((p_time = (MIDIcode**) GiveSpace((Size) size+size)) == NULL) return(ABORT); */

/* First suppress identical note overlappings */
for(i=ii=0; i < (*p_MIDIsize)[j]; i++) {
	c = (*((*pp_MIDIcode)[j]))[i].byte;
	t = (*((*pp_MIDIcode)[j]))[i].time;
	ch = c % 16;
	c0 = c - ch;
	if(c0 == NoteOn && (*((*pp_MIDIcode)[j]))[i+2].byte == 0) c0 = NoteOff;
	if(c0 == NoteOn) {
		k = (*((*pp_MIDIcode)[j]))[i+1].byte;
		if(++((*p_keyon[ch])[k]) > 1) {
			(*p_MIDI)[ii].byte = NoteOn + ch;
			(*p_MIDI)[ii++].time = t;
			(*p_MIDI)[ii].byte = k;
			(*p_MIDI)[ii++].time = t;
			(*p_MIDI)[ii].byte = 0;
			(*p_MIDI)[ii++].time = t;
			}
		(*p_MIDI)[ii].byte = c;
		(*p_MIDI)[ii++].time = t;
		(*p_MIDI)[ii].byte = (*((*pp_MIDIcode)[j]))[i+1].byte;
		(*p_MIDI)[ii++].time = t;
		(*p_MIDI)[ii].byte = (*((*pp_MIDIcode)[j]))[i+2].byte;
		(*p_MIDI)[ii++].time = t;
		i += 2; continue;
		}
	if(c0 != NoteOff) {
		(*p_MIDI)[ii].byte = c;
		(*p_MIDI)[ii++].time = t;
		continue;
		}
	k = (*((*pp_MIDIcode)[j]))[i+1].byte;
	if(--((*p_keyon[ch])[k]) == 0) {
		(*p_MIDI)[ii].byte = c;
		(*p_MIDI)[ii++].time = t;
		(*p_MIDI)[ii].byte = (*((*pp_MIDIcode)[j]))[i+1].byte;
		(*p_MIDI)[ii++].time = t;
		(*p_MIDI)[ii].byte = (*((*pp_MIDIcode)[j]))[i+2].byte;
		(*p_MIDI)[ii++].time = t;
		}
	i += 2;
	}
size = (*p_MIDIsize)[j] = ii;
ptr = (*pp_MIDIcode)[j];
MySetHandleSize((Handle*)&ptr,(Size) size * sizeof(MIDIcode));
(*pp_MIDIcode)[j] = ptr;
for(ch=0; ch < MAXCHAN; ch++) {
	ptr3 = (Handle) p_keyon[ch];
	MyDisposeHandle(&ptr3);
	p_keyon[ch] = NULL;
	}

/* Now adjust NoteOff dates */
for(ch=0; ch < MAXCHAN; ch++) {
	if((p_start[ch] = (Milliseconds**) GiveSpace((Size)(MAXKEY+1)*sizeof(Milliseconds)))
			== NULL) return(ABORT);
	for(k=0; k < MAXKEY; k++) (*p_start[ch])[k] = ZERO;
	}
for(i=0; i < size; i++) {
	c = (*p_MIDI)[i].byte;
	t = (*p_MIDI)[i].time;
	ch = c % 16;
	c0 = c - ch;
	if(c0 == NoteOn && (*p_MIDI)[i+2].byte == 0) c0 = NoteOff;
	if(c0 == NoteOn) {
		k = (*p_MIDI)[i+1].byte;
		(*p_start[ch])[k] = t;
		i += 2;
		}
	if(c0 != NoteOff) continue;
	k = (*p_MIDI)[i+1].byte;
	if((t - (*p_start[ch])[k]) < mindur) {
		t = mindur + (*p_start[ch])[k];
		(*p_MIDI)[i].time = (*p_MIDI)[i+1].time = (*p_MIDI)[i+2].time = t;
		}
	i += 2;
	}
for(ch=0; ch < MAXCHAN; ch++) {
	ptr3 = (Handle) p_start[ch];
	MyDisposeHandle(&ptr3);
	p_start[ch] = NULL;
	}

/* Copy back to prototype */
for(i=0; i < size; i++) {
	(*((*pp_MIDIcode)[j]))[i] = (*p_MIDI)[i];
	}
if(MyDisposeHandle((Handle*)&p_MIDI) != OK) return(ABORT);

SortMIDIdates(ZERO,j);
GetPrePostRoll(j,&preroll,&postroll);
(*p_Dur)[j] = (*((*pp_MIDIcode)[j]))[size-1].time - preroll + postroll;

if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

ChangedProtoType(j);
return(OK);
}


MakeMonodic(int j)
{
char on[MAXCHAN];
int lastkey[MAXCHAN];
int c,c0,ch;
MIDIcode **p_MIDI,**ptr;
long i,ii;
Size size;
Milliseconds t;

if(CheckNonEmptyMIDI(j) != OK) return(FAILED);
if(Answer("Make monodic can't be undone. Proceed anyway",'N') != YES)
	return(FAILED);
for(ch=0; ch < MAXCHAN; ch++) on[ch] = 0;

if(DurationToPoint(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

size = MyGetHandleSize((Handle) (*pp_MIDIcode)[j]);
if((p_MIDI = (MIDIcode**) GiveSpace((Size) size+size)) == NULL) return(ABORT);
for(i=ii=0; i < (*p_MIDIsize)[j]; i++) {
	c = (*((*pp_MIDIcode)[j]))[i].byte;
	t = (*((*pp_MIDIcode)[j]))[i].time;
	ch = c % 16;
	c0 = c - ch;
	if(c0 == NoteOn && (*((*pp_MIDIcode)[j]))[i+2].byte == 0) c0 = NoteOff;
	if(c0 == NoteOn) {
		if(++(on[ch]) > 1) {
			(*p_MIDI)[ii].byte = NoteOn + ch;
			(*p_MIDI)[ii].sequence = 0;
			(*p_MIDI)[ii++].time = t;
			(*p_MIDI)[ii].byte = lastkey[ch];
			(*p_MIDI)[ii].sequence = 0;
			(*p_MIDI)[ii++].time = t;
			(*p_MIDI)[ii].byte = 0;
			(*p_MIDI)[ii].sequence = 0;
			(*p_MIDI)[ii++].time = t;
			}
		(*p_MIDI)[ii].byte = c;
		(*p_MIDI)[ii].sequence = 0;
		(*p_MIDI)[ii++].time = t;
		(*p_MIDI)[ii].byte = lastkey[ch] = (*((*pp_MIDIcode)[j]))[i+1].byte;
		(*p_MIDI)[ii].sequence = (*((*pp_MIDIcode)[j]))[i+1].sequence;
		(*p_MIDI)[ii++].time = t;
		(*p_MIDI)[ii].byte = (*((*pp_MIDIcode)[j]))[i+2].byte;
		(*p_MIDI)[ii].sequence = (*((*pp_MIDIcode)[j]))[i+2].sequence;
		(*p_MIDI)[ii++].time = t;
		i += 2; continue;
		}
	if(c0 != NoteOff) {
		(*p_MIDI)[ii].byte = c;
		(*p_MIDI)[ii].sequence = 0;
		(*p_MIDI)[ii++].time = t;
		continue;
		}
	if(--(on[ch]) == 0) {
		(*p_MIDI)[ii].byte = c;
		(*p_MIDI)[ii].sequence = 0;
		(*p_MIDI)[ii++].time = t;
		(*p_MIDI)[ii].byte = (*((*pp_MIDIcode)[j]))[i+1].byte;
		(*p_MIDI)[ii].sequence = (*((*pp_MIDIcode)[j]))[i+1].sequence;
		(*p_MIDI)[ii++].time = t;
		(*p_MIDI)[ii].byte = (*((*pp_MIDIcode)[j]))[i+2].byte;
		(*p_MIDI)[ii].sequence = (*((*pp_MIDIcode)[j]))[i+2].sequence;
		(*p_MIDI)[ii++].time = t;
		}
	i += 2;
	}
size = (*p_MIDIsize)[j] = ii;
ptr = (*pp_MIDIcode)[j];
MySetHandleSize((Handle*)&ptr,(Size) size * sizeof(MIDIcode));
(*pp_MIDIcode)[j] = ptr;
for(i=0; i < ii; i++) {
	(*((*pp_MIDIcode)[j]))[i] = (*p_MIDI)[i];
	}
if(MyDisposeHandle((Handle*)&p_MIDI) != OK) return(ABORT);

if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

ChangedProtoType(j);
return(OK);
}


AppendAllNotesOff(int j)
{
MIDIcode **ptr;
long size;

if(CheckNonEmptyMIDI(j) != OK) return(FAILED);

(*p_MIDIsize)[j] += 3L;
size = (*p_MIDIsize)[j];
ptr = (*pp_MIDIcode)[j];
MySetHandleSize((Handle*)&ptr,(Size) size * sizeof(MIDIcode));
(*pp_MIDIcode)[j] = ptr;

(*((*pp_MIDIcode)[j]))[size-3].byte = ControlChange;
(*((*pp_MIDIcode)[j]))[size-2].byte = 123;
(*((*pp_MIDIcode)[j]))[size-1].byte = 0;
(*((*pp_MIDIcode)[j]))[size-3].sequence = 0;
(*((*pp_MIDIcode)[j]))[size-2].sequence = 0;
(*((*pp_MIDIcode)[j]))[size-1].sequence = 0;
(*((*pp_MIDIcode)[j]))[size-3].time = (*((*pp_MIDIcode)[j]))[size-2].time
	= (*((*pp_MIDIcode)[j]))[size-1].time = 0;
ChangedProtoType(j);
return(OK);
}


SuppressAllNotesOff(int warn,int j)
{
int c,c0,found,only;
MIDIcode **ptr;
long i,k,size;
Milliseconds t;
Handle ptr3;
double preroll,postroll;

if(CheckNonEmptyMIDI(j) != OK) return(FAILED);
if(warn && Answer("Suppress AllNotesOff can't be undone. Proceed anyway",'N') != YES)
	return(FAILED);
if(GetPrePostRoll(j,&preroll,&postroll) != OK) {
	if(Beta) {
		sprintf(Message,"Err. SuppressAllNotesOff(). j = %ld",(long)j);
		Alert1(Message);
		}
	return(FAILED);
	}
	
if(DurationToPoint(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

found = FALSE;
if(/* SelectPictureOn && */ (*p_Tpict)[j] > ZERO) {
	only = Answer("Suppress messages only after insertion point",'Y');
	if(only == ABORT) return(FAILED);
	}
for(i=k=0; (i+k) < (*p_MIDIsize)[j]; i++) {
	c = (*((*pp_MIDIcode)[j]))[i+k].byte;
	t = (*((*pp_MIDIcode)[j]))[i+k].time; 
	c0 = c - (c % 16);
	if(c0 == ControlChange && (*((*pp_MIDIcode)[j]))[i+k+1].byte == 123
			&& (*((*pp_MIDIcode)[j]))[i+k+2].byte == 0 && (!only || t >= ((*p_Tpict)[j]+preroll))) {
		found = TRUE;
		k += 3; i -= 1;
		continue;
		}
	(*((*pp_MIDIcode)[j]))[i] = (*((*pp_MIDIcode)[j]))[i+k];
	}
(*p_MIDIsize)[j] -= k;
size = (*p_MIDIsize)[j];
if(size > 0) {
	ptr = (*pp_MIDIcode)[j];
	MySetHandleSize((Handle*)&ptr,(Size) size * sizeof(MIDIcode));
	(*pp_MIDIcode)[j] = ptr;
	(*p_Dur)[j] = (*((*pp_MIDIcode)[j]))[size-1].time - preroll + postroll;
	if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
	}
else {
	(*p_Dur)[j] = ZERO;
	ptr3 = (Handle)(*pp_MIDIcode)[j];
	if(MyDisposeHandle(&ptr3) != OK) return(ABORT);
	(*pp_MIDIcode)[j] = NULL;
	}
if(!found) {
	ShowMessage(TRUE,wMessage,"No 'AllNotesOff' message was found.");
	}
else ChangedProtoType(j);
return(OK);
}


SuppressMessages(int tell,int j,int themessage)
{
int i,c,c0,k,nbytes,size,time,found,only;
MIDIcode **ptr;
Milliseconds t;
Handle ptr3;
double preroll,postroll;

if(CheckNonEmptyMIDI(j) != OK) return(FAILED);
if(Answer("Suppress messages can't be undone. Proceed anyway",'N') != YES)
	return(FAILED);
if(GetPrePostRoll(j,&preroll,&postroll) != OK) {
	if(Beta) {
		sprintf(Message,"Err. SuppressMessages(). j = %ld",(long)j);
		Alert1(Message);
		}
	return(FAILED);
	}
switch(themessage) {
	case ChannelPressure: nbytes = 2; break;	/* 208 */
	case PitchBend: nbytes = 3; break;			/* 224 */
	case KeyPressure: nbytes = 3; break;		/* 160 */
	case TimingClock: nbytes = 1; break;		/* 160 (suppressing silences) */
	default:
		if(Beta) Alert1("Err. SuppressMessages(). Case not supported");
		return(FAILED);
	}
if(/* SelectPictureOn && */ (*p_Tpict)[j] > ZERO) {
	only = Answer("Suppress messages only after insertion point",'Y');
	if(only == ABORT) return(FAILED);
	}

found = FALSE;
	
/* Suppress initial silence that is only made of a non-zero timing of the first event */
if(themessage == TimingClock && (t=(*((*pp_MIDIcode)[j]))[0].time) > ZERO
			&& (!only || t >= ((*p_Tpict)[j]+preroll))) {
	(*((*pp_MIDIcode)[j]))[0].time = ZERO;
	found = TRUE;
	}
	
if(DurationToPoint(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

for(i=k=0; (i+k) < (*p_MIDIsize)[j]; i++) {
	c = (*((*pp_MIDIcode)[j]))[i+k].byte;
	t = (*((*pp_MIDIcode)[j]))[i+k].time;
	if(ChannelEvent(c)) c0 = c - (c % 16);
	else c0 = c;
	if(c0 == themessage && (!only || t >= ((*p_Tpict)[j]+preroll))) {
		found = TRUE;
		k += nbytes; i -= 1;
		continue;
		}
	(*((*pp_MIDIcode)[j]))[i] = (*((*pp_MIDIcode)[j]))[i+k];
	}
(*p_MIDIsize)[j] -= k;
size = (*p_MIDIsize)[j];
if(size > 0) {
	ptr = (*pp_MIDIcode)[j];
	MySetHandleSize((Handle*)&ptr,(Size) size * sizeof(MIDIcode));
	(*pp_MIDIcode)[j] = ptr;
	(*p_Dur)[j] = (*((*pp_MIDIcode)[j]))[size-1].time - preroll + postroll;
	}
else {
	(*p_Dur)[j] = ZERO;
	ptr3 = (Handle)(*pp_MIDIcode)[j];
	if(MyDisposeHandle(&ptr3) != OK) return(ABORT);
	(*pp_MIDIcode)[j] = NULL;
	}
	
if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

if(!found) {
	if(tell) Alert1("No event was found. Object is unchanged...");
	ShowMessage(TRUE,wMessage,"No event was found. Object is unchanged...");
	}
else ChangedProtoType(j);
return(OK);
}


InsertSilence(int j,Milliseconds dur)
{
int result,only;
long i,i0;
Milliseconds t;
double preroll,postroll;

if(CheckNonEmptyMIDI(j) != OK) return(FAILED);
if(GetPrePostRoll(j,&preroll,&postroll) != OK) {
	if(Beta) {
		sprintf(Message,"Err. InsertSilence(). j = %ld",(long)j);
		Alert1(Message);
		}
	return(FAILED);
	}
result = FAILED;
if((*p_MIDIsize)[j] < 1) {
	Alert1("Can't do this on an object containing no MIDI code");
	return(result);
	}
if(dur < ZERO) {
	Alert1("A silence with negative duration does not make sense.\nYou perhaps want to set the pre-roll");
	return(result);
	}
if(/* SelectPictureOn && */ (*p_Tpict)[j] > ZERO) {
	only = Answer("Insert silence at insertion point ('No' will place it at the beginning)",'Y');
	if(only == ABORT) return(FAILED);
	}

if(Answer("Insert silence can't be undone. Proceed anyway",'N') != YES)
	return(FAILED);

if(DurationToPoint(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

for(i=i0=0; i < (*p_MIDIsize)[j]; i++) {
	if(!only) break;
	if((*((*pp_MIDIcode)[j]))[i].time >= ((*p_Tpict)[j]+preroll)) {
		i0 = i; break;
		}
	}
result = OK;
if(i0 == 0 && Answer("May be you want to set up a negative pre-roll value",'Y') != 'N') {
	dur = ZERO;
	result = FAILED;
	}
if(i == (*p_MIDIsize)[j]) {
	dur = ZERO;
	Alert1("Insertion point is beyond the last event!");
	result = FAILED;
	}
	
if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

(*((*pp_MIDIcode)[j]))[i0].time += dur;
(*p_Dur)[j] += dur;
ChangedProtoType(j);
return(result);
}


AppendSilence(int j,Milliseconds dur)
{
MIDIcode **ptr;
long size;

if(CheckNonEmptyMIDI(j) != OK) return(FAILED);
if(dur < ZERO) {
	Alert1("A silence with negative duration does not make sense.\nYou perhaps want to set the post-roll");
	return(FAILED);
	}
if(Answer("May be you want to set up a positive post-roll value",'Y') != 'N')
	return(FAILED);
	
(*p_MIDIsize)[j] += 1;
size = (*p_MIDIsize)[j];
ptr = (*pp_MIDIcode)[j];
MySetHandleSize((Handle*)&ptr,(Size) size * sizeof(MIDIcode));
(*pp_MIDIcode)[j] = ptr;

(*((*pp_MIDIcode)[j]))[size-1].time = dur;
(*((*pp_MIDIcode)[j]))[size-1].byte = TimingClock;
(*((*pp_MIDIcode)[j]))[size-1].sequence = 0;
(*p_Dur)[j] += dur;
ChangedProtoType(j);
return(OK);
}


DurationToPoint(MIDIcode ****pp_midicode,Milliseconds ****pp_csoundtime,long **p_size,int j)
// Change time information in prototype j from durations (the usual format)
// to dates
{
long i;
Milliseconds time;

if(pp_csoundtime != NULL && PointCsound) {
	if((*p_size)[j] == ZERO) return(OK);
 	else {
 		if(Beta) Alert1("Err. DurationToPoint(). Point mode in Csound");
		return(FAILED);
		}
	}
if(pp_midicode != NULL && PointMIDI) {
	if((*p_size)[j] == ZERO) return(OK);
 	else {
 		if(Beta) Alert1("Err. DurationToPoint(). Point mode in MIDI");
		return(FAILED);
		}
	}
	
if(pp_csoundtime != NULL) {
	for(i=0,time = ZERO; i < (*p_size)[j]; i++) {
		time += (*((*pp_csoundtime)[j]))[i];
		(*((*pp_csoundtime)[j]))[i] = time;
		}
	PointCsound = TRUE;
	}
if(pp_midicode != NULL) {
	for(i=0,time = ZERO; i < (*p_size)[j]; i++) {
		time += (*((*pp_midicode)[j]))[i].time;
		(*((*pp_midicode)[j]))[i].time = time;
		}
	PointMIDI = TRUE;
	}
return(OK);
}

	
PointToDuration(MIDIcode ****pp_midicode,Milliseconds ****pp_csoundtime,long **p_size,int j)
// Change time information in prototype j from dates to durations (the usual format)
{
long i;
Milliseconds time;

if(pp_csoundtime != NULL && !PointCsound) {
	if((*p_size)[j] == ZERO) {
		return(OK);
		}
 	else {
 		if(Beta) Alert1("Err. PointToDuration(). Not point mode in Csound");
		return(FAILED);
		}
	}
if(pp_midicode != NULL && !PointMIDI) {
	if((*p_size)[j] == ZERO) return(OK);
 	else {
 		if(Beta) Alert1("Err. PointToDuration(). Not point mode in MIDI");
		return(FAILED);
		}
	}

if(pp_csoundtime != NULL) {
	for(i=(*p_size)[j] - 1; i > 0; i--) {
		(*((*pp_csoundtime)[j]))[i]
			= (*((*pp_csoundtime)[j]))[i] - (*((*pp_csoundtime)[j]))[i-1];
		}
	PointCsound = FALSE;
	}
if(pp_midicode != NULL) {
	for(i=(*p_size)[j] - 1; i > 0; i--) {
		(*((*pp_midicode)[j]))[i].time
			= (*((*pp_midicode)[j]))[i].time - (*((*pp_midicode)[j]))[i-1].time;
		}
	PointMIDI = FALSE;
	}
return(OK);
}


SortMIDIdates(long i0, int j)
// Sort MIDI stream on dates in prototype j
// (*pp_MIDIcode)[j].time is points, not durations
// misdated events are 3-bytes
{
int c0,c1,c2,r;
long i;
Milliseconds t,t0,t1;

if(j < 2 || j >= Jbol) {
	if(Beta) Alert1("Err. SortMIDIdates(). j < 2 || j >= Jbol");
	return(FAILED);
	}
t = (*((*pp_MIDIcode)[j]))[i0].time;
for(i=i0+1; i < (*p_MIDIsize)[j]; i++) {
	if((*((*pp_MIDIcode)[j]))[i].time < t) {
		if((r=SortMIDIdates(i,j)) != OK) return(r);
		i -= 3;
		c0 = (*((*pp_MIDIcode)[j]))[i].byte;
		t0 = (*((*pp_MIDIcode)[j]))[i].time;
		c1 = (*((*pp_MIDIcode)[j]))[i+1].byte;
		c2 = (*((*pp_MIDIcode)[j]))[i+2].byte;
		for(i=i+3; i < (*p_MIDIsize)[j]; i++) {
			if((t1=(*((*pp_MIDIcode)[j]))[i].time) > t0) {
				(*((*pp_MIDIcode)[j]))[i-3].byte = c0;
				(*((*pp_MIDIcode)[j]))[i-3].sequence = 0;
				(*((*pp_MIDIcode)[j]))[i-3].time = t0;
				(*((*pp_MIDIcode)[j]))[i-2].byte = c1;
				(*((*pp_MIDIcode)[j]))[i-2].sequence = 0;
				(*((*pp_MIDIcode)[j]))[i-2].time = t0;
				(*((*pp_MIDIcode)[j]))[i-1].byte = c2;
				(*((*pp_MIDIcode)[j]))[i-1].sequence = 0;
				t = (*((*pp_MIDIcode)[j]))[i-1].time = t0;
				return(OK);
				}
			else {
				(*((*pp_MIDIcode)[j]))[i-3] = (*((*pp_MIDIcode)[j]))[i];
				}
			}
		(*((*pp_MIDIcode)[j]))[i-3].byte = c0; /* Added 5/3/97 */
		(*((*pp_MIDIcode)[j]))[i-3].sequence = 0; /* Added 5/3/97 */
		(*((*pp_MIDIcode)[j]))[i-3].time = t0; /* Added 5/3/97 */
		(*((*pp_MIDIcode)[j]))[i-2].byte = c1; /* Added 5/3/97 */
		(*((*pp_MIDIcode)[j]))[i-2].sequence = 0; /* Added 5/3/97 */
		(*((*pp_MIDIcode)[j]))[i-2].time = t0; /* Added 5/3/97 */
		(*((*pp_MIDIcode)[j]))[i-1].byte = c2; /* Added 5/3/97 */
		(*((*pp_MIDIcode)[j]))[i-1].sequence = 0; /* Added 5/3/97 */
		(*((*pp_MIDIcode)[j]))[i-1].time = t0; /* Added 5/3/97 */
		}
	else {
		t = (*((*pp_MIDIcode)[j]))[i].time;
		}
	}
return(OK);
}


SortCsoundDates(long i0, int j)
// Sort Csound score on dates in prototype j
// (*((*pp_CsoundTime)[j]))[i] is points, not durations
{
long i;
Milliseconds t,t0,t1;
CsoundLine s0;
int r;

if(j < 2 || j >= Jbol) {
	if(Beta) Alert1("Err. SortCsoundDates(). j < 2 || j >= Jbol");
	return(FAILED);
	}
t = (*((*pp_CsoundTime)[j]))[i0];
for(i=i0+1; i < (*p_CsoundSize)[j]; i++) {
	if((*((*pp_CsoundTime)[j]))[i] < t) {
		if((r=SortCsoundDates(i,j)) != OK) return(r);
		i--;
		s0 = (*((*pp_CsoundScore)[j]))[i];
		t0 = (*((*pp_CsoundTime)[j]))[i];
		for(i=i+1; i < (*p_CsoundSize)[j]; i++) {
			if((t1=(*((*pp_CsoundTime)[j]))[i]) > t0) {
				(*((*pp_CsoundScore)[j]))[i-1] = s0;
				t = (*((*pp_CsoundTime)[j]))[i-1] = t0;
				return(OK);
				}
			else {
				(*((*pp_CsoundScore)[j]))[i-1] = (*((*pp_CsoundScore)[j]))[i];
				(*((*pp_CsoundTime)[j]))[i-1] = (*((*pp_CsoundTime)[j]))[i];
				}
			}
		(*((*pp_CsoundScore)[j]))[i-1] = s0;
		(*((*pp_CsoundTime)[j]))[i-1] = t0;
		}
	else {
		t = (*((*pp_CsoundTime)[j]))[i];
		}
	}
return(OK);
}


#if BP_CARBON_GUI	// CheckiProto() is only called from DoDialog()
CheckiProto(void)
{
int r;

if(Jbol < 3) {
	if(CompileCheck() != OK) return(OK);
	if(!ObjectMode && !ObjectTry && Jbol > 2 && LoadObjectPrototypes(YES,NO) != OK) {
		if((r=Answer("Load a '-mi' sound-object prototype file",'N')) == YES) {
			if((r=CheckPrototypes()) != OK) return(r);
			}
		else {
			if(r == ABORT) return(r);
			if((r=CheckTerminalSpace()) != OK) return(ABORT);
			if((r=ResizeObjectSpace(YES,Jbol + Jpatt,0)) != OK) return(ABORT);
			ObjectMode = ObjectTry = NeedAlphabet = TRUE;
			}
		}
	if(Jbol < 3) iProto = 0;
	if(iProto >= Jbol) iProto = Jbol - 1;
	}
if(iProto < 2) iProto = 2;
if(iProto >= Jbol) {
	Alert1("Before designing prototypes you must load or create an alphabet");
	BPActivateWindow(SLOW,wAlphabet);
	NeedAlphabet = TRUE;
	iProto = 0;
	return(FAILED);
	}
return(OK);
}
#endif /* BP_CARBON_GUI */


PlayPrototype(int j)
{
double pclock,qclock;
long p,q;
double preroll,postroll;
int showgraphic,displaytimeset,r,infothere;
char line[MAXFIELDCONTENT],**h;

if(j < 2 || j >= Jbol || (*p_MIDIsize)[j] == ZERO) {
	Alert1("This sound-object contains no MIDI message");
	return(STOP);
	}
if(!((*p_Type)[j] & 1)) {
	Alert1("Can't play this sound-object unless 'MIDI sequence' is checked");
	return(DONE);
	}
if(GetPrePostRoll(j,&preroll,&postroll) != OK) {
	if(Beta) {
		sprintf(Message,"Err. PlayPrototype(). j = %ld",(long)j);
		Alert1(Message);
		}
	return(FAILED);
	}
pclock = Pclock; qclock = Qclock;
showgraphic = ShowGraphic; ShowGraphic = FALSE;
displaytimeset = DisplayTimeSet; DisplayTimeSet = FALSE;
SetButtons(TRUE);
infothere = FALSE;
if(Nature_of_time == STRIATED) {
	p = (*p_Tref)[j];
	if(p < EPSILON) p = (*p_Dur)[j];
	}
else {
	FlashInfo("Time is 'smooth': type cmd-T to modify");
	infothere = TRUE;
	p = (*p_Dur)[j];
	}
PlayFromInsertionPoint = FALSE;
if((*p_Tpict)[j] > ZERO && !ConvertMIDItoCsound) {
	PlayFromInsertionPoint = TRUE;
	if(infothere) WaitABit(1000L);
	FlashInfo("Playing object after insertion point");
	infothere = TRUE;
	}
if(p > EPSILON) {
	if((r=Simplify((double)INT_MAX,1000.,(double)p,&Qclock,&Pclock)) != OK) goto OUT;
	SetTempo();
	}
MystrcpyTableToString(MAXFIELDCONTENT,line,p_Bol,j);
DefaultVolume = 127;
if((h = (char**) GiveSpace((Size)((1+strlen(line)) * sizeof(char)))) == NULL) {
	r = ABORT; goto OUT;
	}
if((r=MystrcpyStringToHandle(&h,line)) != OK) goto OUT;
if(infothere) WaitABit(500L);
NoRepeat = PlayPrototypeOn = TRUE;

r = PlayHandle(h,NO);

NoRepeat = PlayPrototypeOn = FALSE;
if(MyDisposeHandle((Handle*)&h) != OK) goto OUT;

#if BP_CARBON_GUI
if(ConvertMIDItoCsound) GetCsoundScore(iProto);
#endif /* BP_CARBON_GUI */

OUT:
PlayFromInsertionPoint = FALSE;
DefaultVolume = DeftVolume;
ShowGraphic = showgraphic;
DisplayTimeSet = displaytimeset;
SetButtons(TRUE);
Pclock = pclock; Qclock = qclock;
SetTempo();
HideWindow(Window[wInfo]);
return(r);
}


CheckChannelRange(long *p_p,long *p_q)
{
int c;

c = 0;
if(*p_q == ZERO || (c = *p_p / *p_q) < 1 || c > MAXCHAN) {
	sprintf(Message,"MIDI channel should be in range 1..%ld. Can't accept %ld",
		(long)MAXCHAN,(long)c);
	Alert1(Message);
	*p_p = *p_q = 1;
	return(FAILED);
	}
return(OK);
}


SetPrototypeDuration(int j,int *p_longerCsound)
{
double preroll,postroll,dur,maxdur;
int rep;
long size,i;

rep = OK; *p_longerCsound = 0;
(*p_Dur)[j] = ZERO;

if(DurationToPoint(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

size = (*p_MIDIsize)[j];
dur = 0.;
(*p_Dur)[j] = dur;
if(size < 1L) goto CSOUND;

if((*pp_MIDIcode)[j] != NULL) dur = ((*((*pp_MIDIcode)[j]))[size-1].time);
else dur = 0.;
if(dur > EPSILON) dur = dur - (*p_PreRoll)[j] + (*p_PostRoll)[j];
else dur = 0.;
if(dur < 0.) dur = 0.;
(*p_Dur)[j] = dur;

CSOUND:
size = (*p_CsoundSize)[j];
if(size < 1L) goto OUT;

if(DurationToPoint(NULL,pp_CsoundTime,p_CsoundSize,j) != OK) return(ABORT);

maxdur = 0.;
for(i=0; i < (*p_CsoundSize)[j]; i++) {
	dur = (*((*pp_CsoundTime)[j]))[i] + (*((*pp_CsoundScore)[j]))[i].duration;
	if(dur > maxdur) maxdur = dur;
	}
if(maxdur > EPSILON) dur = maxdur - (*p_PreRoll)[j] + (*p_PostRoll)[j];
else dur = 0.;

if(dur - (*p_Dur)[j] > Time_res) {
	if((*p_MIDIsize)[j] > ZERO) *p_longerCsound = dur - (*p_Dur)[j];
	else (*p_Dur)[j] = dur;
	}
if(((dur - (*p_Dur)[j]) < (- Time_res)) && (*p_MIDIsize)[j] > ZERO)
	*p_longerCsound = dur - (*p_Dur)[j];

if(PointToDuration(NULL,pp_CsoundTime,p_CsoundSize,j) != OK) return(ABORT);

OUT:
if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
#if BP_CARBON_GUI
SetPrototypePage5(j);
#endif /* BP_CARBON_GUI */
return(rep);
}


GetPrePostRoll(int j,double *p_preroll,double *p_postroll)
{
if(j < 0 || j >= Jbol || (*p_Dur)[j] < EPSILON) {
	*p_preroll = *p_postroll = 0.;
	return(FAILED);
	}
*p_preroll = (*p_PreRoll)[j];
*p_postroll = (*p_PostRoll)[j];
return(OK);
}


GetPeriod(int j,double beta,double *p_objectperiod,double *p_beforeperiod)
{
double dur;

*p_objectperiod = *p_beforeperiod = 0.;
if(j < 0 || j >= Jbol || (*p_PeriodMode)[j] == IRRELEVANT) return(FAILED);
dur = beta * (*p_Dur)[j];
if(dur < EPSILON) return(FAILED);
if((*p_PeriodMode)[j] == ABSOLUTE) *p_beforeperiod = (*p_BeforePeriod)[j];
else *p_beforeperiod = ((double)(*p_BeforePeriod)[j] * dur) / 100.;
*p_objectperiod =  dur - *p_beforeperiod;
if(*p_objectperiod < 0.) {
	*p_objectperiod = 0.;
	if(Beta) Alert1("Err. GetPeriod(). *p_objectperiod < ZERO");
	}
if(*p_objectperiod < EPSILON) return(FAILED);
return(OK);
}


CheckNonEmptyMIDI(int j)
{
if(j < 2 || j >= Jbol) {
	if(Beta) Alert1("Err. CheckNonEmptyMIDI(). j < 2 || j >= Jbol");
	return(FAILED);
	}
if((*p_MIDIsize)[j] < 1L) {
	Alert1("This sound-object prototype does not contain any MIDI message");
	return(FAILED);
	}
return(OK);
}