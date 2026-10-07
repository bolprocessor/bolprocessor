/* SoundObjects2.c (BP3) */

/*  This file is a part of Bol Processor
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


#ifndef _H_BP3
#include "-BP3.h"
#endif

#include "-BP3decl.h"


int CheckPrototypeSize(int j)
{
if((*p_MIDIsize)[j] > ZERO || (*p_CsoundSize)[j] > ZERO) return(OK);
else return(MISSED);
}


int CheckDuration(int j)
{
if(j >= Jbol) {
	BPPrintMessage(0,odError,"=> Err. CheckDuration()");
	return(MISSED);
	}
if(j < 2 || (*p_Dur)[j] < EPSILON) {
	BPPrintMessage(0,odError,"Invalid option because the duration of this object is null");
	return(MISSED);
	}
else return(OK);
}

int AdjustVelocities(int j,int vmin,int vmax)
{
double a,b;
int c,c0,v,vmin0,vmax0,allsame;
long i;

if(CheckNonEmptyMIDI(j) != OK) return(MISSED);
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
		BPPrintMessage(0,odError,"Velocity range is not significant in this object");
		return(OK);
		}
	if(vmax0 == vmin0) a = 0.;
	else a = ((double)(vmax - vmin)) / (vmax0 - vmin0);
	}
if(Answer("Adjusting velocities can't be undone. Proceed anyway",'N') != YES)
	return(MISSED);
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



int MakeMonodic(int j)
{
char on[MAXCHAN+1];
int lastkey[MAXCHAN+1];
int c,c0,ch;
MIDIcode **p_MIDI,**ptr;
long i,ii;
Size size;
Milliseconds t;

if(CheckNonEmptyMIDI(j) != OK) return(MISSED);
if(Answer("Make monodic can't be undone. Proceed anyway",'N') != YES)
	return(MISSED);
for(ch=0; ch <= MAXCHAN; ch++) on[ch] = 0;

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


int AppendAllNotesOff(int j)
{
MIDIcode **ptr;
long size;

if(CheckNonEmptyMIDI(j) != OK) return(MISSED);

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

int DurationToPoint(MIDIcode ****pp_midicode,Milliseconds ****pp_csoundtime,long **p_size,int j)
// Change time information in prototype j from durations (the usual format)
// to dates
{
long i;
Milliseconds time;

if(pp_csoundtime != NULL && PointCsound) {
	if((*p_size)[j] == ZERO) return(OK);
 	else {
 		BPPrintMessage(0,odError,"=> Err. DurationToPoint(). Point mode in Csound");
		return(MISSED);
		}
	}
if(pp_midicode != NULL && PointMIDI) {
	if((*p_size)[j] == ZERO) return(OK);
 	else {
 		BPPrintMessage(0,odError,"=> Err. DurationToPoint(). Point mode in MIDI");
		return(MISSED);
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

	
int PointToDuration(MIDIcode ****pp_midicode,Milliseconds ****pp_csoundtime,long **p_size,int j)
// Change time information in prototype j from dates to durations (the usual format)
{
long i;
Milliseconds time;

if(pp_csoundtime != NULL && !PointCsound) {
	if((*p_size)[j] == ZERO) {
		return(OK);
		}
 	else {
 		BPPrintMessage(0,odError,"=> Err. PointToDuration(). Not point mode in Csound");
		return(MISSED);
		}
	}
if(pp_midicode != NULL && !PointMIDI) {
	if((*p_size)[j] == ZERO) return(OK);
 	else {
 		BPPrintMessage(0,odError,"=> Err. PointToDuration(). Not point mode in MIDI");
		return(MISSED);
		}
	}

if(pp_csoundtime != NULL) {
	for(i=(*p_size)[j] - 1; i > 0; i--) {
		(*((*pp_csoundtime)[j]))[i]
			= (*((*pp_csoundtime)[j]))[i] - (*((*pp_csoundtime)[j]))[i-1];
		// BPPrintMessage(0,odInfo,"#@§ csoundtime[%d][%d] = %ld\n",j,i,(*((*pp_csoundtime)[j]))[i]);
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


int SortMIDIdates(long i0, int j)
// Sort MIDI stream on dates in prototype j
// (*pp_MIDIcode)[j].time is points, not durations
// misdated events are 3-bytes
{
int c0,c1,c2,r;
long i;
Milliseconds t,t0,t1;

if(j < 2 || j >= Jbol) {
	BPPrintMessage(0,odError,"=> Err. SortMIDIdates(). j < 2 || j >= Jbol");
	return(MISSED);
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


int SortCsoundDates(long i0, int j)
// Sort Csound score on dates in prototype j
// (*((*pp_CsoundTime)[j]))[i] is points, not durations
{
long i;
Milliseconds t,t0,t1;
CsoundLine s0;
int r;

if(j < 2 || j >= Jbol) {
	BPPrintMessage(0,odError,"=> Err. SortCsoundDates(). j < 2 || j >= Jbol");
	return(MISSED);
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


int SetPrototypeDuration(int j,int *p_longerCsound) {
	double preroll,postroll,dur,maxdur;
	int rep;
	long size,i;

	if(j < 2 || j >= Jbol) return(OK);
	rep = OK; *p_longerCsound = 0;

	(*p_Dur)[j] = ZERO;

	if(DurationToPoint(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(MISSED);

	size = (*p_MIDIsize)[j];
	if(size < 1L) goto CSOUND;

	if((*pp_MIDIcode)[j] != NULL) dur = ((*((*pp_MIDIcode)[j]))[size-1].time);
	else dur = 0.;
	if(dur < 0.) dur = 0.;
	(*p_Dur)[j] = dur;

CSOUND:
	size = (*p_CsoundSize)[j];
	// BPPrintMessage(0,odInfo,"§ size = %ld\n",size);
	if(size < 1L) goto SORTIR;

	if(DurationToPoint(NULL,pp_CsoundTime,p_CsoundSize,j) != OK) return(ABORT);

	maxdur = 0.;
	for(i=0; i < (*p_CsoundSize)[j]; i++) {
		dur = (*((*pp_CsoundTime)[j]))[i] + (*((*pp_CsoundScore)[j]))[i].duration;
		if(dur > maxdur) maxdur = dur;
		}
	dur = maxdur;

	// BPPrintMessage(0,odInfo,"§§ dur = %.2f\n",dur);

	if(dur - (*p_Dur)[j] > Time_res) {
		if((*p_MIDIsize)[j] > ZERO) *p_longerCsound = dur - (*p_Dur)[j];
		else (*p_Dur)[j] = dur;
		}
	if(((dur - (*p_Dur)[j]) < (- Time_res)) && (*p_MIDIsize)[j] > ZERO)
		*p_longerCsound = dur - (*p_Dur)[j];

	if(PointToDuration(NULL,pp_CsoundTime,p_CsoundSize,j) != OK) return(ABORT);

	SORTIR:
	if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
	return(rep);
	}


int GetPrePostRoll(int j,double *p_preroll,double *p_postroll) { // NOT USED
	if(j < 2 || j >= Jbol) {
		*p_preroll = *p_postroll = 0.;
		return(MISSED);
		}
	// (*p_PreRollMode)[j] is not used here
	*p_preroll = (*p_PreRoll)[j];
	*p_postroll = (*p_PostRoll)[j];
	return(OK);
	}


int GetPeriod(int j,double beta,double *p_objectperiod,double *p_cyclicafter) {
	double dur;

	*p_objectperiod = *p_cyclicafter = 0.;
	if(j < 0 || j >= Jbol || (*p_CyclicMode)[j] == IRRELEVANT) return(MISSED);
	dur = beta * (*p_Dur)[j];
	if(dur < EPSILON) return(MISSED);
	if((*p_CyclicMode)[j] == FIXVALUE) *p_cyclicafter = (*p_CyclicAfter)[j];
	else {
		*p_cyclicafter = ((double)(*p_CyclicAfter)[j] * dur) / 100.;
		}
	*p_objectperiod =  dur - *p_cyclicafter;
	// BPPrintMessage(1,odInfo,"@@@ j = %d, cyclicafter = %.4f, objectperiod = %.4f\n",j,*p_cyclicafter,*p_objectperiod);
	if(*p_objectperiod < 0.) {
		*p_objectperiod = 0.;
		BPPrintMessage(0,odError,"=> Err. GetPeriod(). *p_objectperiod =  %.4f\n",*p_objectperiod);
		}
	if(*p_objectperiod < EPSILON) return(MISSED);
	return(OK);
	}


int CheckNonEmptyMIDI(int j) {
	if(j < 2 || j >= Jbol) {
		BPPrintMessage(0,odError,"=> Err. CheckNonEmptyMIDI(). j < 2 || j >= Jbol");
		return(MISSED);
		}
	if((*p_MIDIsize)[j] < 1L) {
		BPPrintMessage(0,odError,"This sound-object prototype does not contain any MIDI message");
		return(MISSED);
		}
	return(OK);
	}