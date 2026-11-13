/* SetObjectFeatures.c (BP3) */ 

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

int trace_object_features = 0;
int trace_get_duration = 0;
int trace_set_variation = 0;
int trace_table = 0;
int check_limits = 0;

// 2026-11-12 The last fixed value of a parameter (_pitchbend(), _volume(), _mod(), _press(), _pan() or an additional parameter
// set by _value()) is remembered, so that it applies to the following expressions that do not set it. Each value is attached
// to the instrument if _ins() was given, or else to the part if _part() was given, or else to the MIDI channel if _chan() was
// given, or else to the current instrument: the one guessed by _value() for an additional parameter, or else instrument 1, as
// an item starts with the implicit values _ins(1) _part(1) _chan(1). An instrument guessed by _value() does not count as
// given, so that the values of parts in "{_part(1) _value(blah,10) C4, _part(2) _value(blah,20) E4}" stay apart. An object without a value
// of its own takes the most recent value attached to its instrument, its part or its channel (implicit values included).
// On equal dates, the rank is: instrument given by _ins(), part, channel, implicit or guessed instrument.
// For example:
//    "{_part(1) _pan(23) C4, _part(2) _pan(48) E4} {_part(1) D4, _part(2) F4}": D4 gets 23 and F4 gets 48
//    "{_volume(20) C4} {_chan(2) D4}": D4 gets 20 (attached to the implicit instrument 1)
//    "{_ins(3) _volume(50) C4} {_ins(3) _part(2) _chan(5) D4}": D4 gets 50 (attached to instrument 3)
// Only a fixed value is remembered. A value of a continuous or stepwise variation, such as the end of a slide
// "_pitchcont _pitchbend(0) sa3 _ _pitchbend(100) _pitchfixed", belongs to its expression: it erases the remembered
// value, so that the following expressions get the default value. Otherwise "-gr.vina3", whose notes are all on
// channel 1, played notes out of tune after a slide.
// A value applies only to objects starting after its date (position in the phase diagram). Fields of a polymetric
// expression are read one after the other, so in "{_volumecont _volume(0) - - _volume(60), C4 D4 E4}" the value 60 is
// read before C4, which starts earlier. A field starting at the date of the value, such as "C4 D4 E4" with "_volume(0)",
// does not get it either, otherwise the result would depend on the order of the fields - fixed by Claude
#define NREMEMBERED (IPANORAMIC + 1 + MAXPARAMS)
#define MAXREMEMBEREDINS 256
#define KEYINS 0
#define KEYPART 1
#define KEYCHAN 2
static int KeySize[3] = {MAXREMEMBEREDINS + 1,MAXPARTS + 1,MAXCHAN + 1};
// Two layers: the values of the phase diagram being built (cleared by StartRememberedParameters() at the start of
// each diagram, so that separate items and recomputed diagrams are independent), and the values carried over from
// the previous chunks of the same item (moved there by CarryRememberedParameters() after each chunk), which are
// older than any value of the current chunk. A value erased by a continuous variation hides the carried value.
#define NOTHING 0
#define AVALUE 1
#define ERASED 2
static double *RememberedValue[3] = {NULL,NULL,NULL},*RememberedDate[3] = {NULL,NULL,NULL},*CarriedValue[3] = {NULL,NULL,NULL};
static char *RememberedState[3] = {NULL,NULL,NULL},*IsCarried[3] = {NULL,NULL,NULL};
// For instruments: TRUE if the value was attached to an instrument given by _ins(), FALSE if attached to the
// implicit _ins(1) or to an instrument guessed by _value(). On equal dates, the rank is: _ins() given, part,
// channel, implicit or guessed instrument. In "{_volume(80) C4, _part(2) _volume(30) E4} {_part(2) D4}",
// D4 gets 30: both values have the date 0, and 80 is attached to the implicit _ins(1)
static char *RememberedExplicit = NULL,*CarriedExplicit = NULL;
static double RememberClock = 0.;

void SetRememberClock(double date) {
	RememberClock = date;
	}

static int AllocateRememberedParameters(void) {
	int kind;
	long n;
	for(kind = 0; kind < 3; kind++) {
		if(RememberedState[kind] != NULL) continue;
		n = (long) NREMEMBERED * KeySize[kind];
		RememberedValue[kind] = (double*) malloc(n * sizeof(double));
		RememberedDate[kind] = (double*) malloc(n * sizeof(double));
		CarriedValue[kind] = (double*) malloc(n * sizeof(double));
		RememberedState[kind] = (char*) malloc(n * sizeof(char));
		IsCarried[kind] = (char*) malloc(n * sizeof(char));
		if(kind == KEYINS) {
			RememberedExplicit = (char*) malloc(n * sizeof(char));
			CarriedExplicit = (char*) malloc(n * sizeof(char));
			}
		if(RememberedValue[kind] == NULL || RememberedDate[kind] == NULL || CarriedValue[kind] == NULL
				|| RememberedState[kind] == NULL || IsCarried[kind] == NULL
				|| (kind == KEYINS && (RememberedExplicit == NULL || CarriedExplicit == NULL))) {
			BPPrintMessage(0,odError,"=> Err. AllocateRememberedParameters(). Out of memory\n");
			RememberedState[kind] = NULL;
			return(ABORT);
			}
		}
	return(OK);
	}

void ResetRememberedParameters(void) {
	// Forget everything, e.g. at the start of a play when controllers are reset
	int kind;
	long i,n;
	if(AllocateRememberedParameters() != OK) return;
	for(kind = 0; kind < 3; kind++) {
		n = (long) NREMEMBERED * KeySize[kind];
		for(i = 0; i < n; i++) {
			RememberedState[kind][i] = NOTHING;
			IsCarried[kind][i] = FALSE;
			}
		}
	}

void StartRememberedParameters(void) {
	// Called at the start of a phase diagram: only the values carried over from previous chunks are kept
	int kind;
	long i,n;
	if(AllocateRememberedParameters() != OK) return;
	for(kind = 0; kind < 3; kind++) {
		n = (long) NREMEMBERED * KeySize[kind];
		for(i = 0; i < n; i++) RememberedState[kind][i] = NOTHING;
		}
	}

void CarryRememberedParameters(void) {
	// Called after a chunk has been played: its values will apply to the following chunks of the item
	int kind;
	long i,n;
	if(AllocateRememberedParameters() != OK) return;
	for(kind = 0; kind < 3; kind++) {
		n = (long) NREMEMBERED * KeySize[kind];
		for(i = 0; i < n; i++) {
			if(RememberedState[kind][i] == AVALUE) {
				CarriedValue[kind][i] = RememberedValue[kind][i];
				if(kind == KEYINS) CarriedExplicit[i] = RememberedExplicit[i];
				IsCarried[kind][i] = TRUE;
				}
			else if(RememberedState[kind][i] == ERASED) IsCarried[kind][i] = FALSE;
			RememberedState[kind][i] = NOTHING;
			}
		}
	}

static long RememberedSlot(int kind,int key,int index) {
	// Returns -1 if out of range
	if(RememberedState[kind] == NULL || index < 0 || index >= NREMEMBERED || key < 0 || key >= KeySize[kind]) return(-1);
	return((long) index * KeySize[kind] + key);
	}

void RememberParameter(int index,CurrentParameters *p_currentparameters,double value,int mode) {
	int kind,key,explicit;
	long slot;

	// The value is attached to the instrument, part or channel that was given, in this order of priority
	explicit = FALSE;
	if(p_currentparameters->insset && p_currentparameters->currinstr > 0) {
		kind = KEYINS; key = p_currentparameters->currinstr;
		explicit = TRUE;
		}
	else if(p_currentparameters->currpart > 0) {
		kind = KEYPART; key = p_currentparameters->currpart;
		}
	else if(p_currentparameters->chanset) {
		kind = KEYCHAN; key = p_currentparameters->currchan;
		}
	else {
		// The current instrument: the one guessed by _value() or _cont() for an additional parameter, or else the implicit _ins(1)
		kind = KEYINS; key = (p_currentparameters->currinstr > 0) ? p_currentparameters->currinstr : 1;
		}
	if((slot = RememberedSlot(kind,key,index)) < 0) return;
	RememberedState[kind][slot] = (mode == FIXMAPMODE) ? AVALUE : ERASED;
	RememberedValue[kind][slot] = value;
	RememberedDate[kind][slot] = RememberClock;
	if(kind == KEYINS) RememberedExplicit[slot] = explicit;
	}

int RememberedParameter(int index,CurrentParameters *p_currentparameters,double *p_value) {
	// Returns TRUE and the most recent value attached to the instrument, part or channel of the current object
	// (implicit values included), set before the current date. On equal dates the rank is: instrument given by _ins(),
	// part, channel, implicit or guessed instrument. Values carried over from previous chunks are older than those of
	// the current chunk.
	int kind,key[3],found,rank,thisrank;
	long slot;
	double date,thisdate,thisvalue;

	key[KEYINS] = p_currentparameters->currinstr > 0 ? p_currentparameters->currinstr : 1;
	key[KEYPART] = p_currentparameters->currpart > 0 ? p_currentparameters->currpart : 1;
	key[KEYCHAN] = p_currentparameters->currchan;
	found = FALSE; date = 0.; rank = 0;
	for(kind = 0; kind < 3; kind++) {
		if((slot = RememberedSlot(kind,key[kind],index)) < 0) continue;
		if(RememberedState[kind][slot] != NOTHING && RememberedDate[kind][slot] < RememberClock) {
			if(RememberedState[kind][slot] == ERASED) continue;
			thisvalue = RememberedValue[kind][slot];
			thisdate = RememberedDate[kind][slot];
			thisrank = (kind == KEYINS) ? (RememberedExplicit[slot] ? 0 : 3) : kind;
			}
		else if(IsCarried[kind][slot]) {
			thisvalue = CarriedValue[kind][slot];
			thisdate = -1.;
			thisrank = (kind == KEYINS) ? (CarriedExplicit[slot] ? 0 : 3) : kind;
			}
		else continue;
		if(!found || thisdate > date || (thisdate == date && thisrank < rank)) {
			*p_value = thisvalue;
			date = thisdate;
			rank = thisrank;
			found = TRUE;
			}
		}
	return(found);
	}

int SetObjectParams(int part_of_score,int level,int nseq,long k,int j,CurrentParameters *p_currentparameters,ContParameters **p_contparameters,Table **h_table) {

	ParameterSpecs **currentinstancevalues;
	long size;
	int paramindex,ii,ins,instrument,n,ip,ipmax,chan,scale,blockkey;
	short mode;
	double v0,v1;
	Coordinates **ptr;
	Handle hdl;

	if(k < 2) {
		BPPrintMessage(0,odError,"=> Err. SetObjectParams(). k < 2\n");
		return(ABORT); // Fixed by BB 2021-02-26
		}
	if(nseq >= Maxconc) {
		BPPrintMessage(0,odError,"=> Err. SetObjectParams(). nseq >= Maxconc\n");
		return(ABORT); // Fixed by BB 2021-02-26
		}

	if((*p_Instance)[k].contparameters.values != NULL) {
		BPPrintMessage(0,odError,"=> Err. SetObjectParams(). (*p_Instance)[k].contparameters.values != NULL\n");
		return(ABORT); // Fixed by BB 2021-02-26
		}

	if(j > 0 && j < Jbol) { 
		if((*p_DefaultChannel)[j] > 0) {	// Object has a specific MIDI channel
			// Here we need (*p_DefaultChannel)[1] = 0.
			(*p_Instance)[k].channel = (*p_DefaultChannel)[j];
			if((*p_Instance)[k].channel > MAXCHAN || (*p_Instance)[k].channel < 1) {
				my_sprintf(Message,"'%s' has channel %ld.  Should be 1..%ld\n",
					(*((*p_Bol)[j])),(long)(*p_DefaultChannel)[j],(long)MAXCHAN);
				Print(wTrace,Message);
				ShowError(32,0,0);
				return(MISSED);
				}
			}
		else {
			if((*p_DefaultChannel)[j] == 0)
				(*p_Instance)[k].channel = p_currentparameters->currchan;
			else (*p_Instance)[k].channel = 0; // No change
			}
		if((*p_CsoundInstr)[j] > 0) {	// Object has specific instrument
			// Here we need (*p_CsoundInstr)[1] = 0
			(*p_Instance)[k].instrument = (*p_CsoundInstr)[j];
			if(trace_object_features) BPPrintMessage(0,odInfo,"€€ specific instrument for j = %d, k = %d, instrument = %d\n",j,k,(*p_Instance)[k].instrument);
			}
		else {
			(*p_Instance)[k].instrument = p_currentparameters->currinstr;
			if(trace_object_features) BPPrintMessage(0,odInfo,"€€ current instrument for j = %d, k = %d, instrument = %d\n",j,k,(*p_Instance)[k].instrument);
			}
		}
	else {
		(*p_Instance)[k].channel = p_currentparameters->currchan;
		(*p_Instance)[k].instrument = p_currentparameters->currinstr;
			if(trace_object_features) BPPrintMessage(0,odInfo,"€€ current instrument for k = %d, instrument = %d\n",k,(*p_Instance)[k].instrument);
	
		}
	(*p_Instance)[k].scale = p_currentparameters->scale;
	if(trace_scale) BPPrintMessage(0,odInfo,"4) scale = %d\n",p_currentparameters->scale); 
	(*p_Instance)[k].blockkey = p_currentparameters->blockkey;


	(*p_Instance)[k].contparameters.values = NULL;
	(*p_Instance)[k].contparameters.number = 0;

	instrument = (*p_Instance)[k].instrument;
	j = (*p_Instance)[k].object; // not needed as 'j' is known
	if(j > 1 && j < Jbol) { // 2026-10-29
		mode = (*p_CsoundInstrumentMode)[j];
		if(mode > 0) {
			instrument = (*p_CsoundInstr)[j];
			(*p_Instance)[k].instrument = instrument;
			}
		else {
			// Here we need to guess the instrument given the line of the Csound score
		/* 	if(mode == LOCAL_CS) {
				instrument = ThisInstrument(TRUE,k,iline,&ins);
				} */
			}
		}
	ins = InstrumentIndex(instrument);
	ipmax = (*p_CsInstrument)[ins].ipmax;
	if(trace_set_variation) BPPrintMessage(1,odInfo,"SetObjectParams() part_of_score = %d, level = %d, k = %d, j = %d, object = %d, ins = %d, instrument = %d, ipmax = %d,\n",part_of_score,level,k,j,(*p_Instance)[k].object,ins,(*p_Instance)[k].instrument,ipmax);
	if(level >= 0) {
		n = MaxParameters + IPANORAMIC + 1;
		// BPPrintMessage(1,odInfo,"ooo n2 = %d, k = %d, ins = %d, ipmax = %d, level = %d\n",n,k,ins,ipmax,level);
		(*p_contparameters)[level].number = n;
		(*p_Instance)[k].contparameters.number = n;

		if((currentinstancevalues=(ParameterSpecs**) GiveSpace((Size)(n) * sizeof(ParameterSpecs))) == NULL) return(ABORT);
		(*p_Instance)[k].contparameters.values = currentinstancevalues;

		hdl = (Handle) (*p_contparameters)[level].values;
		MySetHandleSize((Handle*)&hdl,(Size) n * sizeof(ContParameterSpecs));
		(*p_contparameters)[level].values = (ContParameterSpecs**) hdl;

	/*	for(paramindex = (IPANORAMIC + 1); paramindex < n; paramindex++) {
			BPPrintMessage(1,odInfo,"===--  paramindex = %d, v0 = %.2f, v1 = %.2f\n",paramindex,(*currentinstancevalues)[paramindex].v0,(*currentinstancevalues)[paramindex].v1);
			} */
		for(paramindex = 0; paramindex < n; paramindex++) {
		//	BPPrintMessage(1,odInfo,"===++ paramindex = %d, v0 = %.2f\n",paramindex,(*currentinstancevalues)[paramindex].v0);
			chan = (*((*p_contparameters)[level].values))[paramindex].channel;
			scale = (*((*p_contparameters)[level].values))[paramindex].scale;
			blockkey = (*((*p_contparameters)[level].values))[paramindex].blockkey;
			if(!(*((*p_contparameters)[level].values))[paramindex].known
					&& RememberedParameter(paramindex,p_currentparameters,&v0)) {
				// 2026-11-12 The parameter is not set at this level: it takes the last value of the part (or channel), which is
				// kept at this level so that the following objects get it too - fixed by Claude
				(*((*p_contparameters)[level].values))[paramindex].v0
					= (*((*p_contparameters)[level].values))[paramindex].v1
					= (*((*p_contparameters)[level].values))[paramindex].start = v0;
				(*((*p_contparameters)[level].values))[paramindex].mode = FIXMAPMODE;
				(*((*p_contparameters)[level].values))[paramindex].active = TRUE;
				(*((*p_contparameters)[level].values))[paramindex].known = TRUE;
				if(trace_set_variation) BPPrintMessage(1,odInfo,"Remembered value of paramindex = %d: %.2f\n",paramindex,v0);
				}
			if(paramindex <= IPANORAMIC && !(*((*p_contparameters)[level].values))[paramindex].known) {
				if(trace_set_variation) BPPrintMessage(1,odInfo,"NOT known, paramindex = %d\n",paramindex);
				switch(paramindex) {
					case IMODULATION:
						if(!ResetControllers && (*p_Oldvalue)[chan].modulation > -1)
							v0 = (*p_Oldvalue)[chan].modulation;
						else
							v0 = DEFTMODULATION;
						break;
					case IPITCHBEND:
						if(!ResetControllers && (*p_Oldvalue)[chan].pitchbend > -1.)
							v0 = (*p_Oldvalue)[chan].pitchbend;
						else
							v0 = DEFTPITCHBEND;
						break;
					case IPRESSURE:
						if(!ResetControllers && (*p_Oldvalue)[chan].pressure > -1)
							v0 = (*p_Oldvalue)[chan].pressure;
						else
							v0 = DEFTPRESSURE;
						break;
					case IVOLUME:
						if(!ResetControllers && (*p_Oldvalue)[chan].volume > -1)
							v0 = (*p_Oldvalue)[chan].volume;
						else
							v0 = DeftVolume;
						break;
					case IPANORAMIC:
						if(!ResetControllers && (*p_Oldvalue)[chan].panoramic > -1)
							v0 = (*p_Oldvalue)[chan].panoramic;
						else
							v0 = DeftPanoramic;
						break;
					}
				(*((*p_contparameters)[level].values))[paramindex].known = TRUE;
				v1 = v0;
				mode = FIXMAPMODE;
				if(trace_set_variation) BPPrintMessage(1,odInfo,"paramindex1 = %d, v0 = %.2f v1 = %.2f, mode = %d\n",paramindex,(float)v0,(float)v1,mode);
				}
			else {
				if(trace_set_variation) {
					if(paramindex <= IPANORAMIC) BPPrintMessage(1,odInfo,"KNOWN paramindex = %d\n",paramindex);
					else BPPrintMessage(1,odInfo,"NOT Known2 paramindex = %d\n",paramindex);
					}
				v0 = (*((*p_contparameters)[level].values))[paramindex].v0;
				v1 = (*((*p_contparameters)[level].values))[paramindex].v1;
				mode = (*((*p_contparameters)[level].values))[paramindex].mode;
				if(trace_set_variation) BPPrintMessage(1,odInfo,"paramindex2 = %d, v0 = %.2f, v1 = %.2f, mode = %d\n",paramindex,v0,v1,mode);
				}
			(*currentinstancevalues)[paramindex].v0 = v0;
			(*currentinstancevalues)[paramindex].v1 = v1;
			(*currentinstancevalues)[paramindex].mode = mode;
			// 2026-11-10: an additional parameter (e.g. "oops") that was never assigned a value takes the default value
			// of the instrument in CscoreWrite(). Its v0 and v1 were 0, which was combined with the values of the score - fixed by Claude
			if(paramindex <= IPANORAMIC) (*currentinstancevalues)[paramindex].known = TRUE;
			else (*currentinstancevalues)[paramindex].known = (*((*p_contparameters)[level].values))[paramindex].known;
			if(trace_set_variation) BPPrintMessage(0,odInfo,"-> start value v0 = %.2f, end value v1 = %.2f, mode = %d, paramindex = %d\n",v0,v1,mode,paramindex);
			(*currentinstancevalues)[paramindex].control = (*((*p_contparameters)[level].values))[paramindex].control;
			(*currentinstancevalues)[paramindex].channel = chan;
			(*currentinstancevalues)[paramindex].scale = scale;
			if(trace_scale) 
				BPPrintMessage(0,odInfo,"1) scale = %d\n",scale);
			(*currentinstancevalues)[paramindex].blockkey = blockkey;
			(*currentinstancevalues)[paramindex].index = (*((*p_contparameters)[level].values))[paramindex].index;
			(*currentinstancevalues)[paramindex].imax = ZERO;
			(*currentinstancevalues)[paramindex].point = NULL;
			if(paramindex == IPANORAMIC && j >= 0 && j < Jbol && !(*p_OkPan)[j]) continue;
			if(paramindex == IVOLUME && j >= 0 && j < Jbol && !(*p_OkVolume)[j]) continue;
			if(h_table == NULL) {
				BPPrintMessage(0,odError,"=> Err. SetObjectParams(). h_table == NULL\n");
				}
			
			if(part_of_score && (*h_table)[paramindex].point != NULL) {
				if((*h_table)[paramindex].offset == 0) {
					if((ptr=(Coordinates**) GiveSpace((Size)((*h_table)[paramindex].imax) * sizeof(Coordinates))) == NULL)
						return(ABORT);
					(*currentinstancevalues)[paramindex].point = ptr;
					(*currentinstancevalues)[paramindex].imax = (*h_table)[paramindex].imax;
					for(ii=0; ii < (*h_table)[paramindex].imax; ii++) {
						(*((*currentinstancevalues)[paramindex].point))[ii] = (*((*h_table)[paramindex].point))[ii];
						}
					// Here we set v1, at last
					(*currentinstancevalues)[paramindex].v1 = (*((*h_table)[paramindex].point))[ii-1].value;
					if(trace_set_variation) BPPrintMessage(1,odInfo,"-> end value v1 = %ld\n",(long)(*currentinstancevalues)[paramindex].v1);
					}
				((*h_table)[paramindex].offset)--;
				}
			// 2026-11-08: tables saved for other channels also wait for an object counted among all channels - fixed by Claude
			if(part_of_score) DecrementSavedTableOffsets(paramindex);
			
			// BPPrintMessage(0,odInfo,"SetObjectFeatures k = %ld mode = %ld ?\n",(long)k,(*currentinstancevalues)[paramindex].mode);
			if((*((*p_contparameters)[level].values))[paramindex].v0 == (*((*p_contparameters)[level].values))[paramindex].v1 && (*h_table)[paramindex].point == NULL) {
				(*currentinstancevalues)[paramindex].mode = FIXMAPMODE;
				(*currentinstancevalues)[paramindex].point = NULL;
				if(trace_set_variation) BPPrintMessage(1,odInfo,"SetObjectFeatures k = %ld paramindex = %d mode = FIXMAPMODE v0 = %ld v1 = %ld\n",(long)k,paramindex,(long)(*((*p_contparameters)[level].values))[paramindex].v0,(long)(*((*p_contparameters)[level].values))[paramindex].v1);
				}
				
			if((*((*p_contparameters)[level].values))[paramindex].channel > 0)
				(*currentinstancevalues)[paramindex].channel
					= (*((*p_contparameters)[level].values))[paramindex].channel;
			else
				(*currentinstancevalues)[paramindex].channel = p_currentparameters->currchan;
				
			if((*((*p_contparameters)[level].values))[paramindex].scale != 0) {
				(*currentinstancevalues)[paramindex].scale
					= (*((*p_contparameters)[level].values))[paramindex].scale;
				if(trace_scale) BPPrintMessage(0,odInfo,"2) scale = %d\n",(*((*p_contparameters)[level].values))[paramindex].scale);
				}
			else {
				(*currentinstancevalues)[paramindex].scale = p_currentparameters->scale;
				if(trace_scale) BPPrintMessage(0,odInfo,"3) scale = %d\n",p_currentparameters->scale);
				}
				
			if((*((*p_contparameters)[level].values))[paramindex].blockkey != DefaultBlockKey)
				(*currentinstancevalues)[paramindex].blockkey
					= (*((*p_contparameters)[level].values))[paramindex].blockkey;
			else
				(*currentinstancevalues)[paramindex].blockkey = p_currentparameters->blockkey;
			}
		}

	if(!part_of_score) (*p_Instance)[k].nseq = nseq;
	// Otherwise the nseq field has been set in Plot()

	(*p_Instance)[k].velocity = p_currentparameters->currvel;
	(*p_Instance)[k].rndvel = p_currentparameters->rndvel;
	(*p_Instance)[k].velcontrol = p_currentparameters->velcontrol;
	(*p_Instance)[k].randomtime = p_currentparameters->randomtime;
	(*p_Instance)[k].seed = p_currentparameters->seed;

	if(j < 1 || j >= Jbol || (*p_OkMap)[j]) {
		(*p_Instance)[k].map0 = p_currentparameters->map0;
		(*p_Instance)[k].map1 = p_currentparameters->map1;
		(*p_Instance)[k].mapmode = p_currentparameters->mapmode;
		(*p_Instance)[k].xpandkey = p_currentparameters->xpandkey;
		(*p_Instance)[k].xpandval = p_currentparameters->xpandval;
		}
	else {
		(*p_Instance)[k].map0.p1 = (*p_Instance)[k].map1.p1 = -1;
		(*p_Instance)[k].xpandkey = -1;
		(*p_Instance)[k].xpandval = 0;
		}
	(*p_Instance)[k].transposition = 0;
	(*p_Instance)[k].transposefirst = p_currentparameters->transposefirst;
	(*p_Instance)[k].capture = p_currentparameters->capture;
	(*p_Instance)[k].part = p_currentparameters->currpart;
	if(j < 1 || j >= Jbol) {
		(*p_Instance)[k].transposition = p_currentparameters->currtranspose;
		if(p_Articul != NULL) {
	//	if(p_currentparameters->currarticul >= 0)
			(*p_Articul)[k] = p_currentparameters->currarticul;
	//	else (*p_Articul)[k] = MAXINT + p_currentparameters->currarticul;
			}
		}
	else {
		if((*p_OkTransp)[j]) (*p_Instance)[k].transposition = p_currentparameters->currtranspose;
		if(p_Articul != NULL) {
			if((*p_OkArticul)[j]) {
	//		if(p_currentparameters->currarticul >= 0)
				(*p_Articul)[k] = p_currentparameters->currarticul;
	//		else (*p_Articul)[k] = MAXINT + p_currentparameters->currarticul;
				}
			}
		}
	return(OK);
	}


int AttachObjectLists(long k,int nseq,p_list ****p_waitlist,p_list ****p_scriptlist,
	int* p_newswitch,unsigned int* currswitchstate) {
	objectspecs** pto;
	unsigned int** pts;
	int i;

	if(nseq >= Maxconc) {
		BPPrintMessage(0,odError,"=> Err. AttachObjectLists(). nseq >= Maxconc, k = %d\n",k);
		return(OK);
		}
	if((*p_waitlist)[nseq] == NULL && (*p_scriptlist)[nseq] == NULL && !(*p_newswitch)) return(OK);
	if(k < 2) {
		BPPrintMessage(0,odError,"=> Err. AttachObjectLists()");
		return(ABORT);
		}
	if((*p_ObjectSpecs)[k] == NULL) {
		if((pto = (objectspecs**) GiveSpace((Size)sizeof(objectspecs))) == NULL) return(ABORT);
		(*p_ObjectSpecs)[k] = pto;
		SwitchState(k) = NULL;
		}
	WaitList(k) = (*p_waitlist)[nseq];
	ObjScriptLine(k) = (*p_scriptlist)[nseq];
	if(*p_newswitch) {
		if((pts = (unsigned int**) GiveSpace((Size) (MAXCHAN + 1) * sizeof(unsigned int))) == NULL) return(ABORT);
		SwitchState(k) = pts;
		for(i=0; i < MAXCHAN; i++) (*(SwitchState(k)))[i] = currswitchstate[i];
		}
	(*p_waitlist)[nseq] = NULL;
	(*p_scriptlist)[nseq] = NULL;
	*p_newswitch = FALSE;
	return(OK);
	}


int SetVariation(tokenbyte targettoken,CurrentParameters **p_deftcurrentparameters,
	CurrentParameters *p_currentparameters,
	ContParameters **p_contparameters,int levelorg,int paramindex_org,
	unsigned long id,tokenbyte **p_buff,double orgspeed,double orgscaling,
	float *p_endvalue,
	KeyNumberMap *p_mapendvalue,float *p_maxbeats,Table **h_table) {
// This procedure does two things:
// 1) Given a target token representing a stepwise/continuous parameter assignment,
//    or the 'paramindex_org' of a continuous parameter, search the next target token and
//    return the number of beats between the two target tokens and the end value.
//    If the parameter is continuous, calculate the increment from start to end value.
// 2) Build a table 'h_table' containing at least three values if these relate to
//    the same sound-object or simple note.  The table will be attached to the
//    object in SetObjectParams(). An offset value is returned so that attachment
//    will occur on the proper object.

	int level,seq,**p_seq,**p_chan,**p_instr,**p_part,ibeats,foundtimeobject,foundsecondvalue,
		paramvalueindex,oldpitchbendrange[MAXCHAN+1],foundconcatenation,oldm,oldp,
		foundendconcatenation,levelmem,seqmem,foundtarget,imap,
		paramindex,maketable,tableisbeingbuilt,tablemade,iobjmem,
		okincrease,objectsfound,nonemptyfound,nonemptyobject,chan,instrument,part,targettokenfoundafterobject,
		forgetmakingtable,followedwithgap,result,chanorg,instrorg,partorg,startedatmidivalue,
		blockkey,scale,scaleorg,blockkeyorg,newval,newkeyval;
	KeyNumberMap vmap;
	unsigned long i,tablesize;
	char **p_notinthisfield,notinthisfield,foundclosingbracket;
	Coordinates **ptr;
	double v,vv,oldv,x,orgtranspose,startvalue,tempo,speed,scaling,s;
	tokenbyte m,p;

	// The following ones must be double because they may be incremented with values < 1 when the sequence is too fast
	double prodtempo,maxbeats,itargettoken,itable;

	Table t;
	Handle h;

	result = OK;
	if((*p_buff)[id] == TEND && (*p_buff)[id+1] == TEND) return(OK);

	if((p_seq=(int**) GiveSpace((Size) Maxlevel * sizeof(int))) == NULL) return(ABORT);
	if((p_chan=(int**) GiveSpace((Size) Maxlevel * sizeof(int))) == NULL) return(ABORT);
	if((p_instr=(int**) GiveSpace((Size) Maxlevel * sizeof(int))) == NULL) return(ABORT);
	if((p_part=(int**) GiveSpace((Size) Maxlevel * sizeof(int))) == NULL) return(ABORT);
	if((p_notinthisfield=(char**) GiveSpace((Size) Maxlevel * sizeof(char))) == NULL)
		return(ABORT);

	for(i = 1; i <= MAXCHAN; i++) oldpitchbendrange[i] = PitchbendRange[i];

	level = levelmem = 0;
	seq = seqmem = (*p_seq)[level] = 0;

	speed = orgspeed;
	scaling = orgscaling;
	if(scaling != 0.) {
		tempo = speed / scaling;
	//	tempo = speed; // 2025-01-21
		// 2026-11-08: back to speed / scaling, as in the loop below and in GetSymbolicDuration(). The callers in FillPhaseDiagram()
		// pass 'speed' and 'scale', so with "tempo = speed" a variation in a scaled polymetric field (e.g. "{..., _pitchbend(-200)
		// _pitchcont sol4 ___ _pitchbend(+200)}") was counted in other units than the duration of its objects, and values went out
		// of range (item #29 of -da.checkControls). AssignValue(), which passed the tempo, now passes tempo * scaling - fixed by Claude
		prodtempo = (Prod / tempo);
		}
	else tempo = prodtempo = 0.;

	ibeats = itable = maxbeats = 0.;
	objectsfound = nonemptyfound = targettokenfoundafterobject = 0;
	foundtimeobject = foundsecondvalue = maketable = tableisbeingbuilt = tablemade
		= forgetmakingtable
		= followedwithgap = foundconcatenation = foundendconcatenation = foundtarget
		= notinthisfield = foundclosingbracket = FALSE;
	okincrease = TRUE;

	if(targettoken == -1) targettoken = IndexToToken(paramindex_org);
	// This will be required for FindValue()

	// 2026-11-08: TRUE if the scan starts at a value of a MIDI parameter (see _chan() below) - fixed by Claude
	startedatmidivalue = (paramindex_org >= 0 && paramindex_org <= IPANORAMIC && ((*p_buff)[id] == targettoken || (*p_buff)[id] == T35));

	if(trace_set_variation) BPPrintMessage(0,odInfo,"@@ Start SetVariation() targettoken = %ld, paramindex_org = %ld, id = %ld, Prod = %.2f speed = %.2f scaling = %.2f tempo = %.2f prodtempo = %.2f, endvalue = %.2f\n",(long)targettoken,(long)paramindex_org,(long)id,Prod,speed,scaling,tempo,prodtempo,(*p_endvalue));

	// 2026-11-08: set before "goto RECORDVALUES" below, which leads to STORECHAN: for a fixed value such as "_volume(30)", the
	// channel of the parameter was set to the uninitialized 'chanorg', e.g. 24928 in an AddressSanitizer build - fixed by Claude
	chan = chanorg = p_currentparameters->currchan;
	instrument = instrorg = p_currentparameters->currinstr;
	part = partorg = p_currentparameters->currpart;

	if(paramindex_org > -1) {
		maketable = TRUE;
		startvalue = (*p_endvalue) = (*((*p_contparameters)[levelorg].values))[paramindex_org].v0;
		if((*((*p_contparameters)[levelorg].values))[paramindex_org].mode == FIXMAPMODE)
			goto RECORDVALUES;
		if(trace_set_variation) BPPrintMessage(0,odInfo,"maketable = TRUE, startvalue = %ld, paramindex_org = %d\n",(long)startvalue,paramindex_org);
		}
	else {
		switch(targettoken)	{
			case T11:	/* _vel() */
				startvalue = (*p_endvalue) = p_currentparameters->currvel;
				if(p_currentparameters->velmode == FIXMAPMODE) goto END;
				break;
			case T20:	/* _legato() or _staccato() */
				startvalue = (*p_endvalue) = p_currentparameters->currarticul;
				break;
			case T26:	/* _transpose */
				startvalue = (*p_endvalue) = p_currentparameters->currtranspose;
				break;
			case T37:	/* _keymap() */
				*p_mapendvalue = p_currentparameters->map0;
				if(p_currentparameters->mapmode == FIXMAPMODE
					|| p_currentparameters->mapmode == OFF) goto END;
				break;
			}
		}
		
	oldv = startvalue;
	if(levelorg >= 0) orgtranspose = (*p_deftcurrentparameters)[levelorg].currtranspose;
	t.point = NULL;
	t.imax = t.offset = tablesize = ZERO;
	itargettoken = 0.;
	iobjmem = oldm = oldp = -1;

	chan = chanorg = p_currentparameters->currchan;
	scale = scaleorg = p_currentparameters->scale;
	blockkey = blockkeyorg = p_currentparameters->blockkey;
	instrument = instrorg = p_currentparameters->currinstr;
	part = partorg = p_currentparameters->currpart;

	for(i = id+2L; ; i += 2L) {
		m = (*p_buff)[i]; p = (*p_buff)[i+1];
		if(m == TEND && p == TEND) break;
		if(trace_set_variation) BPPrintMessage(0,odInfo,"• m = %d, p = %d\n",m,p);
		if(m == T0) {
			switch(p) {
				case 11:	/* '/' speed up */
					speed = GetScalingValue(p_buff,i);
					if(scaling != 0.) {
						tempo = speed / scaling;
						prodtempo = (Prod / tempo);
						if(trace_set_variation) BPPrintMessage(0,odInfo,"In SetVariation(1) tempo = %.2f prodtempo = %.2f\n",tempo,prodtempo);
						}
					else tempo = prodtempo = 0.;
					i += 4;
					break;
				case 25:	/* '\' speed down */
					speed = 1. / (GetScalingValue(p_buff,i));
					if(scaling != 0.) {
						tempo = speed / scaling;
						prodtempo = (Prod / tempo);
						}
					else tempo = prodtempo = 0.;
					i += 4;
					break;
				case 21:	/* '*' scaling up */
					scaling = GetScalingValue(p_buff,i);
					if(scaling != 0.) {
						tempo = speed / scaling;
						prodtempo = (Prod / tempo);
						if(trace_set_variation) BPPrintMessage(0,odInfo,"In SetVariation(1) tempo = %.2f prodtempo = %.2f\n",tempo,prodtempo);
						}
					else tempo = prodtempo = 0.;
					i += 4;
					continue;
				case 24:	/* '**' scaling down */
					s = GetScalingValue(p_buff,i);
					scaling = 1. / s;
					if(scaling != 0.) {
						tempo = speed / scaling;
						prodtempo = (Prod / tempo);
						}
					else tempo = prodtempo = 0.;
					i += 4;
					continue;
				case 12:	/* '{' */
				case 22:
					if(seq == seqmem && objectsfound > 0 && !foundconcatenation) {
						okincrease = FALSE;
						}
					if(level >= 0) {
						(*p_seq)[level] = seq;
						(*p_notinthisfield)[level] = notinthisfield;
						if(seq == seqmem) {
							(*p_chan)[level] = chan;
							(*p_instr)[level] = instrument;
							(*p_part)[level] = part;
							}
						}
					level++;
					if(level >= Maxlevel) {
						BPPrintMessage(0,odError,"=> Err. SetVariation(). level >= Maxlevel\n");
						goto ENDLOOP;
						}
					seq = 0;
					break;
				case 13:	/* '}' */
				case 23:
					if(seq == seqmem && objectsfound > 0 && !foundconcatenation)
						okincrease = FALSE;
					if(level == levelmem && foundconcatenation) foundclosingbracket = TRUE;
					level--;
					if(level >= 0) {
						seq = (*p_seq)[level];
						notinthisfield = (*p_notinthisfield)[level];
						if(seq == seqmem) {
							chan = (*p_chan)[level];
							instrument = (*p_instr)[level];
							part = (*p_part)[level];
							}
						}
					else {
						seq = 0;
						chan = chanorg;
						instrument = instrorg;
						goto ENDLOOP;
						}
					continue;
					break;
				case 14:	/* ',' */
					if(seq == seqmem && objectsfound > 0 && !foundconcatenation) {
						okincrease = FALSE;
						}
					if(foundconcatenation && !foundclosingbracket) notinthisfield = TRUE;
					seq++;
					break;
				case 18:	/* '&' following terminal */
					if(level == levelmem && seq == seqmem) {
						foundconcatenation = TRUE;
						notinthisfield = foundclosingbracket = FALSE;
						}
					break;
				case 19:	/* '&' preceding terminal */
					if(!foundconcatenation) continue;
					if(!notinthisfield) foundendconcatenation = TRUE;
					break;
				}
			continue;
			}
		
		if(m == T3 || m == T47 || m == T25 || m == T9) {
			// Sound-object, silence, prolongation, simple note, time pattern
			if(foundendconcatenation && foundconcatenation
					&& (m == T25 || ((m == T3 || m == T47) && p > 0))) {
				foundendconcatenation = FALSE;
				if(m == oldm && p == oldp) {
					foundconcatenation = FALSE;
					m = T3; p = 0; // Check this for T47, 2026-04-06
					levelmem = level;
					seqmem = seq;
					}
				}
				
			if(seq != seqmem) continue;
			
			if((m == T25 || m == T9 || ((m == T3 || m == T47) && p >= 1)) && level <= 0) {
				nonemptyobject = TRUE;
				objectsfound++;
				nonemptyfound++;
				if(trace_set_variation) BPPrintMessage(0,odInfo,"nonemptyobject, m = %d, p = %d -> objectsfound = %d\n",m,p,objectsfound);
				if(!foundconcatenation && objectsfound == 1) {
					oldm = m; oldp = p;
					}
				}
			else {
				// When level > 0 all objects are accounted as empty
				nonemptyobject = FALSE;
				if(targettokenfoundafterobject > 0) followedwithgap = TRUE;
				if(m == T3 && p < 1) objectsfound++; // 2026-10-31
				if(trace_set_variation) BPPrintMessage(0,odInfo,"emptyobject, m = %d, p = %d -> objectsfound = %d\n",m,p,objectsfound);
				}
			if(!foundsecondvalue) {
				maxbeats += prodtempo;
				if(trace_set_variation) BPPrintMessage(0,odInfo,"!foundsecondvalue --> maxbeats = %.2f\n",maxbeats);
				}
			if(nonemptyobject && (iobjmem == -1 || !tablemade)) iobjmem = itargettoken;
			itargettoken += prodtempo;
			if(trace_set_variation) BPPrintMessage(0,odInfo,"itargettoken = %.2f\n",itargettoken);
			if(maketable && !tableisbeingbuilt && !tablemade && !forgetmakingtable
					&& nonemptyobject) {
				tableisbeingbuilt = TRUE;
				itable = 0.;
				if(trace_set_variation) BPPrintMessage(0,odInfo,"tableisbeingbuilt = TRUE\n",tableisbeingbuilt);
				}
			if(!foundtimeobject) {	// Found first sound-object or prolongation or silence
				foundtimeobject = TRUE;
				maxbeats = prodtempo;
				if(trace_set_variation) BPPrintMessage(0,odInfo,"foundtimeobject, maxbeats = prodtempo = %.2f\n",prodtempo);
				goto ENOUGH;
				}
			// 2026-11-08 Since 2026-10-31, objectsfound also counts prolongations, so that the first sound-object after
			// "_pitchbend(20) _ _" was taken as a following one, and t.offset attached its table to the next object
			// (item #19 of -da.checkAllCsound). Only non-empty objects are counted here - fixed by Claude
			if(nonemptyobject && !tablemade && !forgetmakingtable && (nonemptyfound > 1)) {
				// Found following sound-object or silence
				okincrease = FALSE;
				if(trace_set_variation) BPPrintMessage(0,odInfo,"found following object, okincrease = FALSE, tableisbeingbuilt = %d\n",tableisbeingbuilt);
				if(tableisbeingbuilt && t.point != NULL) {
					if(targettokenfoundafterobject > 0 && followedwithgap) {
						if(trace_set_variation) BPPrintMessage(0,odInfo,"targettokenfoundafterobject > 0 && followedwithgap, itable = %.2f\n",itable);
						(*(t.point))[t.imax].i = itable;
						// The last value is not yet known
						(*(t.point))[t.imax].value = 0.; // zero is default (fixed by BB 10 Nov 2020)
						// Probably this fixed a bug in -gr.vina3 ending with _fixed(slide)
						// but missing the final value of "slide"
						// $$$ However this might be the default value of this parameter…
						// (*((*p_CsInstrument)[j].paramlist))[ip].defaultvalue
						tableisbeingbuilt = FALSE;
						tablemade = TRUE;
						(t.imax)++;
						if(trace_set_variation) BPPrintMessage(0,odInfo,"++ tablemade = TRUE, t.imax = %d\n", (t.imax));
						}
					else { 
						// No table can be built
						forgetmakingtable = TRUE;
						tableisbeingbuilt = FALSE;
						if(trace_set_variation) BPPrintMessage(0,odInfo,"No table can be built\n");
						}
					}
				else {
					if(maketable) {
						tableisbeingbuilt = TRUE;
						itable = 0.;
						(t.offset)++;
						if(trace_set_variation) BPPrintMessage(0,odInfo,"tableisbeingbuilt, t.offset = %d\n",(t.offset));
						}
					}
				}
	ENOUGH:
			itable += prodtempo;
			if(trace_set_variation) BPPrintMessage(0,odInfo,"ENOUGH itable = %.2f, nonemptyobject = %d\n",itable,nonemptyobject);
			if(nonemptyobject) {
				targettokenfoundafterobject = 0;
				followedwithgap = FALSE;
				}
			continue;
			}
			
		if((level > levelmem || seq != seqmem) && !(foundconcatenation && !notinthisfield))
			continue;
		
		if(trace_scale && m == T44) BPPrintMessage(0,odInfo,"SetVariation() T44 paramindex_org = %d\n",paramindex_org);
		
	/*	if((level == levelmem || (foundconcatenation && !notinthisfield))
				&& chan == chanorg
				&& (m == targettoken || (paramindex_org >= 0 && TokenToIndex(m,p) == paramindex_org))) { 2026-11-01 */

		// 2026-11-08 Back to TokenToIndex(m,paramindex_org): for _value() (T35), TokenToIndex(m,p) returned 'p', which is not the storage index
		// 'paramindex_org', so that _value() tokens were never found and continuous arbitrary parameters, such as
		// _cont(volume) _value(volume,...), got no variation. The parameter of a _value() token is checked below
		// with FindParameterIndexAndCreateStorage() - fixed by Claude
		if((level == levelmem || (foundconcatenation && !notinthisfield))
				&& chan == chanorg
				&& (m == targettoken || (paramindex_org >= 0 && TokenToIndex(m,paramindex_org) == paramindex_org))) {
			if(trace_set_variation) BPPrintMessage(0,odInfo,"Found targettoken m = %d, p = %d, chan = %d, targettoken = %d, paramindex_org = %d\n",m,p,chan,targettoken,paramindex_org);
			if(m == T35) {  // _value() 
				paramindex = p % 256;
				if(trace_set_variation) BPPrintMessage(0,odInfo,"&&& paramindex (%d,%d,%d) = %d, paramindex_org = %d\n",m,p,chan,paramindex,paramindex_org);
				if(paramindex_org != FindParameterIndexAndCreateStorage(p_contparameters,levelorg,paramindex)) {
					if(trace_set_variation) BPPrintMessage(0,odInfo,"&&+!!! FindParameterIndex = %d, paramindex_org = %d\n",FindParameterIndexAndCreateStorage(p_contparameters,levelorg,paramindex),paramindex_org);
						continue;
					}
				}
			v = FindValue(m,p,chan);
			if(trace_set_variation) BPPrintMessage(0,odInfo,"Found value(%d,%d,%d) = %.2f\n",m,p,chan,v);
			if(v == Infpos) {
				BPPrintMessage(0,odError,"=> Error in SetVariation(). v = Infpos");
				result = ABORT;
				break;
				}
			if(targettoken == T26) {	/* _transpose */
				v += orgtranspose;
				}
			if(targettoken == T37) {	/* _keymap */
				vmap.p1 = p % 128;
				vmap.q1 = (p - (p % 128)) / 128;
				i += 2L;
				m = (*p_buff)[i]; p = (*p_buff)[i+1];
				if(m != T37) {
					BPPrintMessage(0,odError,"=> Err. SetVariation(). m != T37");
					break;
					}
				vmap.p2 = p % 128;
				vmap.q2 = (p - (p % 128)) / 128;
				}
			foundtarget = TRUE;
			if(objectsfound > 0) targettokenfoundafterobject++;
			if(!foundsecondvalue) {  
				if(maxbeats > 0.) {
					foundsecondvalue = TRUE;
					if(trace_set_variation) BPPrintMessage(0,odInfo,"foundsecondvalue = TRUE, maxbeats = %.2f, targettoken = %d, m = %d, p = %d, id = %ld, i = %ld\n",maxbeats,targettoken,m,p,id,i);
					if(targettoken != T37) *p_endvalue = v;
					else *p_mapendvalue = vmap;
					if(!maketable) break;
					}
				else startvalue = v;
				}
			
			if(targettoken == T11 || targettoken == T20 || targettoken == T37) goto END;
			// _vel(), articulation and _keymap() are not concerned with tables
			
		/*	if(paramindex_org < 0) {
				BPPrintMessage(0,odError,"=> Err. SetVariation(). paramindex_org < 0");
				goto END;
				} */
			
			if(!maketable || !tableisbeingbuilt) goto MORE;
			if(t.point == NULL) {
				tablesize = 10L;
				t.imax = 1L;
				if((ptr = (Coordinates**) GiveSpace((Size)(tablesize * sizeof(Coordinates)))) == NULL)
					return(ABORT);
				t.point = ptr;
				(*(t.point))[ZERO].i = ZERO;
				// The value of this first point is not yet known
				}
			(*(t.point))[t.imax].i = itable;
			(*(t.point))[t.imax].value = v;
			if(trace_table) BPPrintMessage(0,odInfo,"Set value 1 for t.imax = %ld value = %.3f\n",(long)t.imax,(long)v);
			(t.imax)++;
			if(t.imax >= (tablesize - 1L)) {
				if(trace_table) BPPrintMessage(0,odInfo,"Increase size of t.point: t.imax = %ld tablesize = %ld\n",(long)t.imax,(long)tablesize);
				ptr = t.point;
				if((ptr = (Coordinates**) IncreaseSpace((Handle)ptr)) == NULL)
					return(ABORT);
				t.point = ptr;
				tablesize = MyGetHandleSize((Handle)ptr) / sizeof(Coordinates);
				}
	MORE:
			if(t.point != NULL && iobjmem >= 0) {
				if(itargettoken > 0.)
					vv = oldv + ((v - oldv) * iobjmem) / itargettoken;
				else vv = oldv;
				if(tablemade) {
					(*(t.point))[t.imax-1L].value = vv;
					if(trace_table) BPPrintMessage(0,odInfo,"Set value for t.imax = %ld, value = %.3f, iobjmem = %d, v = %.2F, oldv = %.2f, itargettoken = %.2f\n",t.imax,vv,iobjmem,v,oldv,itargettoken);
					break;
					}
				else (*(t.point))[ZERO].value = vv;
				}
			oldv = v;
			iobjmem = -1;
			itargettoken = 0.;
			if(trace_set_variation) BPPrintMessage(0,odInfo,"itargettoken = 0.\n");
			continue;
			}
		if(m == T36) { // _fixed()
			paramindex = p % 256; // Needs completion
		/*	paramindex_org = TokenToIndex(m,paramindex_org);
			BPPrintMessage(0,odInfo,"&&& paramindex (%d,%d,%d) = %d, paramindex_org = %d\n",m,p,chan,paramindex,paramindex_org);
			if(paramindex_org != FindParameterIndexAndCreateStorage(p_contparameters,levelorg,paramindex)) {
				continue;
				} */
			}
		if(m == T21) {		// _pitchrange()
			if(p > 0) PitchbendRange[chan] = p;
			else
				BPPrintMessage(0,odError,"=> Err. SetVariation(). p <= 0");
			continue;
			}
		
		if(m == T10) {		// _chan()
			if(!MIDImicrotonality) {
				x = FindValue(m,p,chan);
				if(chan == (int) x) continue;
				// A channel change in the sequence should be the end of the variation
				// ... of a continuous MIDI parameter
				if(objectsfound > 0 && paramindex_org >= 0 && paramindex_org <= IPANORAMIC) okincrease = FALSE;
				// BPPrintMessage(0,odInfo,"_chan() chan = %d\n",x);
				chan = (int) x;
				// 2026-11-08: when the scan starts at a value of a MIDI parameter, this value belongs to the channel current at its token,
				// even if a _chan() follows before the next object: in "_chan(2) ... sol4__ _pitchbend(150) _chan(4) do4" the 150 is for
				// channel 2. Otherwise (e.g. at the start of a polymetric field) the variation goes to the channel of the next objects,
				// as in "_volumecont _volume(0) C4 {B3 A3, _chan(2) D4 E4} F4 _volume(127)" (-da.checkContControl) - fixed by Claude
				if(maxbeats == 0. && !startedatmidivalue) chanorg = chan;
				}
			continue;
			}
		if(m == T32) {		// _ins()
			x = FindValue(m,p,chan);
			if(instrument == (int) x) continue;
			instrument = (int) x;
			if(maxbeats == 0.) instrorg = instrument;
			continue;
			}
		if(m == T46) {		// _part()
			x = FindValue(m,p,chan);
			if(part == (int) x) continue;
			part = (int) x;
			if(maxbeats == 0.) partorg = part;
			continue;
			}
		if(m == T7) {	// Out-time object
			if(objectsfound > 0) okincrease = FALSE;
			continue;
			}
		}
		
	ENDLOOP:
	if(maketable && tableisbeingbuilt && !tablemade && t.point != NULL
			&& t.imax > ZERO && targettokenfoundafterobject > 0 && followedwithgap
			&& result == OK) {
		if((*(t.point))[t.imax-1L].i != itable) {
			(*(t.point))[t.imax].i = itable;
			(*(t.point))[t.imax].value = oldv;
			if(trace_set_variation) BPPrintMessage(0,odInfo,"(*(t.point))[t.imax].value = oldv = %.3f\n",oldv);
			(t.imax)++;
			}
		iobjmem = 0;
		tablemade = TRUE;
		if(trace_table) 
			BPPrintMessage(0,odInfo,"@ tablemade = TRUE\n");
		}
	if(!foundsecondvalue || result != OK) {
		maxbeats = 0.;
		if(trace_set_variation) BPPrintMessage(0,odInfo,"!foundsecondvalue\n");
		}

	(*p_maxbeats) = maxbeats;
	if(trace_set_variation) BPPrintMessage(0,odInfo,"(*p_maxbeats) = %.2f\n",maxbeats);

	if(paramindex_org < 0 || result != OK) goto END;

	h = (Handle)(*h_table)[paramindex_org].point;
	MyDisposeHandle(&h);

	(*h_table)[paramindex_org].point = NULL;
	(*h_table)[paramindex_org].imax = ZERO;

	if(!foundtarget) goto STORECHAN;

	if(maketable && t.point != NULL) {
		if(tablemade && result == OK) {
			if(trace_set_variation) BPPrintMessage(0,odInfo,"iobjmem = %d, oldv = %.2f\n",iobjmem,oldv);
			if(iobjmem == -1) (*(t.point))[t.imax-1L].value = oldv;
			}
		else {
			ptr = t.point;
			MyDisposeHandle((Handle*)&ptr);
			t.point = NULL;
			t.imax = ZERO;
			}
		(*h_table)[paramindex_org] = t;
		if(trace_set_variation) {
			BPPrintMessage(0,odInfo,"# Copying table t to (*h_table)[%d]\n",paramindex_org);
			for(int ii = 0; ii < (*h_table)[paramindex_org].imax; ii++) {
				BPPrintMessage(0,odInfo,"ii = %d -> val = %.2f\n",ii,(*(t.point))[ii].value);
				}
			}
		}
	else {
		(*h_table)[paramindex_org].point = NULL;
		(*h_table)[paramindex_org].imax = ZERO;
		if(trace_set_variation) BPPrintMessage(0,odInfo,"# Table (*h_table)[%d] is made empty\n",paramindex_org);
		}

	RECORDVALUES:
	if((*p_contparameters)[levelorg].values == NULL) {
		BPPrintMessage(0,odError,"=> Err. SetVariation(). (*p_contparameters)[levelorg].values == NULL");
		goto END;
		}
	(*((*p_contparameters)[levelorg].values))[paramindex_org].maxbeats = (long) maxbeats;
	(*((*p_contparameters)[levelorg].values))[paramindex_org].ibeats = 0;
	(*((*p_contparameters)[levelorg].values))[paramindex_org].start
		= (*((*p_contparameters)[levelorg].values))[paramindex_org].v0 = startvalue;
	(*((*p_contparameters)[levelorg].values))[paramindex_org].known = TRUE;
	// (*((*p_contparameters)[levelorg].values))[paramindex_org].instrument = instrorg;
	if((maxbeats == 0.) || (*((*p_contparameters)[levelorg].values))[paramindex_org].mode == FIXMAPMODE) {
		(*((*p_contparameters)[levelorg].values))[paramindex_org].maxbeats = 1.;
		(*((*p_contparameters)[levelorg].values))[paramindex_org].increment = 0.;
		(*((*p_contparameters)[levelorg].values))[paramindex_org].v1 = startvalue;
		(*((*p_contparameters)[levelorg].values))[paramindex_org].point = NULL;
		if(trace_set_variation) BPPrintMessage(0,odInfo,"& startvalue = %.2f, maxbeats = %.2f\n",startvalue,maxbeats);
		}
	else {
		(*((*p_contparameters)[levelorg].values))[paramindex_org].increment
			= (*p_endvalue) - startvalue;
		(*((*p_contparameters)[levelorg].values))[paramindex_org].v1 = (*p_endvalue);
		(*((*p_contparameters)[levelorg].values))[paramindex_org].active = TRUE;
		// 2026-11-08 Needed when e.g. '_volumecont' follows objects played with a fixed value: IncrementParameter() had set 'active' to FALSE, and the variation was ignored
		if(trace_set_variation) BPPrintMessage(0,odInfo,"&& startvalue = %.2f, endvalue = %.2f, maxbeats = %.2f, increment = %.2f\n",startvalue,(*p_endvalue),maxbeats,(*((*p_contparameters)[levelorg].values))[paramindex_org].increment);
		// Normally this is useless as the v1 value will be calculated again later
		}
					
	STORECHAN:
	if(paramindex_org >= 0 && (*p_contparameters)[levelorg].values != NULL) {
		(*((*p_contparameters)[levelorg].values))[paramindex_org].channel = chanorg;
		}

	END:
	for(i=1; i <= MAXCHAN; i++) PitchbendRange[i] = oldpitchbendrange[i];
	MyDisposeHandle((Handle*)&p_seq);
	MyDisposeHandle((Handle*)&p_chan);
	MyDisposeHandle((Handle*)&p_instr);
	MyDisposeHandle((Handle*)&p_part);
	MyDisposeHandle((Handle*)&p_notinthisfield);
	return(result);
	}


int FixObjectTimings(int nseq,Milliseconds **p_time1,Milliseconds **p_time2,int nature_time) {
	int j,k;
	short dont_randomize;
	long i,inext;
	double local_period;
	Milliseconds t1,t2;
	unsigned int seed;

	if(nseq >= Maxconc) {
		BPPrintMessage(0,odError,"=> Err. FixObjectTimings(). nseq >= Maxconc\n");
		return(OK);
		}
	i = ZERO;
	local_period = 0.;
	dont_randomize = FALSE;
	while(TRUE) {
		k = (*((*p_Seq)[nseq]))[i];
	//	if(k > 2 && k < 6) BPPrintMessage(0,odInfo,"FixObjectTimings() k = %ld alpha = %.2f\n",(long)k,(*p_Instance)[k].alpha);
		if(k < 0) break;
		inext = i;
		while((*((*p_Seq)[nseq]))[++inext] == 0);
		if(k >= 1) {	// Ignoring silences "-" except if specs attached
			if(k >= MaxObjects) {
				BPPrintMessage(0,odError,"=> Err. FixObjectTimings(). k >= MaxObjects");
				return(ABORT);
				}
			j = (*p_Instance)[k].object;
			if(j > 0) {
				if(j < 16384 && j != 1) { // j != 1 added by BB 2022-02-24
					if(j >= Jbol) { // Time pattern
						t1 = (*p_T)[i];
						(*p_time1)[i] = t1;
						t2 = (*p_time2)[i] = t1 + (Milliseconds) ((*p_Instance)[k].alpha * (*p_Dur)[j]);
				//		BPPrintMessage(0,odInfo,"FixObjectTimings() time pattern k = %ld j = %ld alpha = %.2f Dur = %.2f, t1 = %ld t2 = %ld\n",(long)k,(long)j,(*p_Instance)[k].alpha,(*p_Dur)[j],(long)t1,(long)t2);
						}
					else { // Sound-object
						if((*p_PivMode)[j] == PERCENT) {
							t1 = (*p_T)[i] - (Milliseconds) ((*p_Instance)[k].alpha * (*p_Dur)[j] * (*p_PivPos)[j] / 100.);
							// BPPrintMessage(0,odInfo,"@@ PERCENT t1 = %ld, T = %ld, alpha = %.2f, Dur[%d] = %ld, PivPos = %.2f\n",t1,(*p_T)[i],(*p_Instance)[k].alpha,j,(*p_Dur)[j],(*p_PivPos)[j]);
							}
						else {
							t1 = (*p_T)[i] - (*p_PivPos)[j];
							// BPPrintMessage(0,odInfo,"@@ FIXVALUE t1 = %ld, T = %ld, alpha = %.2f, Dur[%d] = %ld, PivPos = %.2f\n",t1,(*p_T)[i],(*p_Instance)[k].alpha,j,(*p_Dur)[j],(*p_PivPos)[j]);
							}
						RandomTime(&t1,(*p_Instance)[k].randomtime,(*p_Instance)[k].alpha * (*p_Dur)[j],&dont_randomize);
						if(PlayFromInsertionPoint) t1 = (*p_time1)[i] = (*p_T)[i];
						t2 =  t1
							+ (Milliseconds)((*p_Instance)[k].alpha * (*p_Dur)[j]);
						if((*p_PreRoll)[j] < ZERO) 
							t1 += (*p_PreRoll)[j];
						if((*p_PostRoll)[j] > ZERO)
							t2 += (*p_PostRoll)[j];
						(*p_time1)[i] = t1;
						(*p_time2)[i] = t2;
						}
					}
				else {	// Simple note or silence
					t1 = (*p_T)[i];
					if(j == 1) { // Added by BB 2022-02-24
						(*p_time1)[i] = t1;
						if(nseq == 0 && nature_time == SMOOTH) {
							t2 = (*p_time2)[i] = t1 + (Milliseconds)((*p_Instance)[k].alpha * (*p_Dur)[j]);
						//	BPPrintMessage(0,odInfo,"nseq = %ld k = %ld alpha = %.2f Dur[j] = %ld\n",(long)nseq,(long)k,(*p_Instance)[k].alpha,(long)(*p_Dur)[j]);
						//	t2 = (*p_time2)[i] = (*p_time1)[i] + (inext - i) * local_period;
							}
						else {
							t2 = (*p_time2)[i] = t1;
						//	BPPrintMessage(0,odInfo,"nseq = %ld k = %ld t1 = t2 = %ld\n",(long)nseq,(long)k,(long)t1);
							}
						}
					else {
						RandomTime(&t1,(*p_Instance)[k].randomtime,(*p_Instance)[k].alpha * 1000L,&dont_randomize);
						(*p_time1)[i] = t1;
						t2 = (*p_time2)[i] = t1 + (*p_Instance)[k].alpha * 1000L;
						}
					// BPPrintMessage(0,odInfo,"FixObjectTimings() simple note or silence k = %ld j = %ld, i = %ld alpha = %.2f t1 = %ldms t2 = %ldms\n",(long)k,(long)j,(long)i,(*p_Instance)[k].alpha,(long)t1,(long)t2);
					}
				}
			else {
				// Out-time sound-object 
				j = -j;
				t1 = t2 = (*p_time1)[i] = (*p_time2)[i] = (*p_T)[i];
				}
			}
		else {
			t1 = t2 = (*p_time1)[i] = (*p_time2)[i] = (*p_T)[i];
			if(nseq == 0 && nature_time == SMOOTH) {
				t2 = (*p_time2)[i] = (*p_time1)[i] + (inext - i) * local_period;
				}
			}
		if(nseq == 0 && nature_time == SMOOTH) {
			local_period = (double) (t2 - t1) / (inext - i);
			i = inext;
			}
		else i = inext;
		}
	return(OK);
	}


int Calculate_alpha(int nseq,int nmax,long maxseq,unsigned long imaxseq,int nature_time,
	char** p_marked) {
	int j,k,kprev,unfinished,gotcurrenttime,ncycles;
	long i,inext,inextm,imax,iprev,jprev,ii;
	double d,alpha,beta,r,sigmaridi,clockperiod;
	Milliseconds To,ton,toff,currenttime,dur;

	if(nseq >= Maxconc) {
		BPPrintMessage(0,odError,"=> Err. Calculate_alpha(). nseq >= Maxconc\n");
		return(OK);
		}
	(*p_Instance)[0].alpha = (*p_Instance)[1].alpha = 0.;	/* '_' and '-' */
	if(nature_time == SMOOTH) clockperiod = ((double) Pclock) * 1000. / Qclock;
	i = -1; // Fixed by BB 2021-01-25
	while((k=(*((*p_Seq)[nseq]))[++i]) < 1) {
		if(k == -1) return(OK);
		}
	if(nature_time == STRIATED || nseq == 0) {
		while(TRUE) {
			k = (*((*p_Seq)[nseq]))[i];
			if(k == -1) break;
			inext = i;
			while((*((*p_Seq)[nseq]))[++inext] == 0);
			if(k < 2) { // Reject first silence and null events
				i = inext;
				continue;
				}
			if(k >= MaxObjects) {
				BPPrintMessage(0,odError,"=> Err. Calculate_alpha(). k >= MaxObjects\n");
				return(ABORT);
				}
			j = (*p_Instance)[k].object;
			ncycles = 1;
			if(j <= 0) {
				beta = alpha = 0.; goto OKALPHA1;
				}
			d = (double) (inext - i) * Kpress / Ratio; // Symbolic duration
		//	BPPrintMessage(0,odInfo,"@ k = %ld j = %ld nseq = %d i = %ld inext = %ld seq[inext] = %d d = %.2f, T[i] = %ld T[i+1] = %ld, T[inext] = %ld T[inext+1] = %ld Kpress = %.0f Ratio = %.0f Kpress/Ratio = %.3f Pclock = %.2f\n",(long)k,(long)j,nseq,(long)i,(long)inext,(*((*p_Seq)[nseq]))[inext],d,(long)(*p_T)[i],(long)(*p_T)[i+1],(long)(*p_T)[inext],(long)(*p_T)[inext+1],Kpress,Ratio,Kpress/Ratio,(double)Pclock);
			// BPPrintMessage(0,odInfo,"@@@ j = %d, Tref = %ld, Dur = %ld\n",j,(*p_Tref)[j],(*p_Dur)[j]);
			if(nature_time == SMOOTH) {
				if(Qclock < 1L) {
					BPPrintMessage(0,odError,"=> Err. Calculate_alpha(). Qclock < 1.\n");
					return(ABORT);
					}
				if(Pclock > 0.) {	// Measured smooth time
					if(j >= 16383) // simple note
						alpha = (double) d * clockperiod / 1000L;
					else if(j >= Jbol) // time pattern
						alpha = (double) d  * clockperiod / (*p_Tref)[j];
					else if((*p_Tref)[j] > EPSILON) // Striated object
						alpha = (double) d * clockperiod / (*p_Tref)[j];
					if((*p_Dur)[j] == ZERO) (*p_SilentObject)[j] = TRUE;
					if((*p_SilentObject)[j]) {
						(*p_Dur)[j] = (*p_Tref)[j];
						alpha = (double) (*p_Tref)[j] / 1000.;
						}
					else if((*p_Dur)[j] > EPSILON) // Smooth object
						alpha = (double) d * clockperiod / (*p_Dur)[j];
					else alpha = d;
					}
				else {		// Pclock = ZERO; non-measured smooth time
					alpha = d;
					}
				if(trace_object_features && j > 0 && j < Jbol) BPPrintMessage(0,odInfo,"Calculate_alpha() smooth 1st line k = %ld j = %ld alpha = %.2f d = %.2f clockperiod = %ld i = %ld inext = %ld Dur = %ld Tref = %ld\n",(long)k,(long)j,(double)alpha,(long)d,(long)clockperiod,(long)i,(long)inext,(long)(*p_Dur)[j],(long)(*p_Tref)[j]);
				}
			else {					// Striated time or nseq > 0
				if(d > 0.) {
					if(Kpress > 2 && inext > i && (*p_T)[inext] == 0.)
						BPPrintMessage(0,odError,"=> Probable rounding error: (*p_T)[%ld] = 0 for object #%ld\n",(long)inext,(long)k);
					if(j >= 16383) // simple note
						alpha = ((double)(*p_T)[inext] - (*p_T)[i]) / 1000L;
					else if(j >= Jbol) {     // time pattern
						if(nature_time == STRIATED) alpha = 0.; // 2025-02-05
						else  alpha = (double) ((*p_T)[inext] - (*p_T)[i]) / (*p_Tref)[j];
						}
					else if((*p_Tref)[j] > EPSILON) { // Silence or striated object declared as a prototype
						alpha = ((double)(*p_T)[inext] - (*p_T)[i]) / (*p_Tref)[j];
						// BPPrintMessage(0,odInfo,"@@ alpha = %.2f, i = %ld, j = %d, inext = %ld\n",alpha,i,j,inext);
						if((*p_Dur)[j] == ZERO) (*p_SilentObject)[j] = TRUE;
						if((*p_SilentObject)[j]) {
						//	BPPrintMessage(0,odInfo,"@@ j = %d, Dur = %ld\n",j,(*p_Dur)[j]);
							if((*p_Dur)[j] < EPSILON) (*p_Dur)[j] = (*p_Tref)[j];
							if((*p_Dur)[j] < EPSILON) (*p_Dur)[j] = (long) (((*p_T)[inext] - (*p_T)[i]));
					//		alpha = (double) (inext - i) * (*p_Tref)[j] / (*p_Dur)[j];
							}
						}
					else if((*p_Dur)[j] > EPSILON) // Smooth object
						alpha = ((double)(*p_T)[inext] - (*p_T)[i]) / (*p_Dur)[j];
					else alpha = 1.;
					}
				else alpha = 0.;
				if(trace_object_features && j > 0 && j < Jbol)
					if(j > 1 && j < Jbol) BPPrintMessage(0,odInfo,"Calculate_alpha striated nseq = %ld k = %ld j = %ld « %s » nseq = %d alpha = %.2f d = %.2f i = %ld inext = %ld, Dur = %ld, Tref = %ld, T[i] = %ld, T[inext] = %ld, silent = %d\n",(long)nseq,(long)k,(long)j,*((*p_Bol)[j]),nseq,alpha,d,(long)i,(long)inext,(long)(*p_Dur)[j],(long)(*p_Tref)[j],(long)(*p_T)[i],(long)(*p_T)[inext],(*p_SilentObject)[j]);
				}
		
			beta = alpha;
			if(j >= Jbol) goto OKALPHA1;
			beta = 1.;
			if((*p_RescaleMode)[j] == OK_RESCALE) goto CHECKMORE1;
			if((*p_FixScale)[j]) {
				if((*p_CyclicMode)[j] == IRRELEVANT) {
					alpha = 1.; 
					goto OKALPHA1;
					}
				}
			if(FixDilationRatioInCyclicObject(j,d,&alpha,&beta,&ncycles) == OK) goto OKALPHA1;
CHECKMORE1:
			if((*p_RescaleMode)[j] == DILATION_RATIO) { // 2026-09-15
				if(alpha > (*p_AlphaMax)[j]) beta = alpha = (*p_AlphaMax)[j];
				if(alpha < (*p_AlphaMin)[j]) beta = alpha = (*p_AlphaMin)[j];
				goto OKALPHA1;
				}
			if((*p_OkExpand)[j] && alpha >= 1.) goto OKALPHA1;
			if(!(*p_OkExpand)[j] && alpha > 1.) {
				alpha = 1.; goto OKALPHA1;
				}
			if((*p_OkCompress)[j] && alpha <= 1.) goto OKALPHA1;
			if(!(*p_OkCompress)[j] && alpha <= 1.) {
				alpha = 1.; goto OKALPHA1;
				}
			beta = alpha = 1.;
		
OKALPHA1:
			if(alpha < 0.) alpha = 0.;
			if(beta < 0.) beta = 0.;
			(*p_Instance)[k].alpha = alpha;
			if(trace_object_features && j > 0 && j < Jbol)
				BPPrintMessage(0,odInfo,"Calculate_alpha() nseq = %d k = %d i = %ld j = %d inext = %ld, alpha = %.2f, beta = %.2f ncycles = %d, Dur = %ld, RescaleMode = %d, ForceRatio = %.2f\n",nseq,k,i,j,inext,alpha,beta,ncycles,(*p_Dur)[j],(*p_RescaleMode)[j],ForceRatio);
			if(ForceRatio >= 0.) (*p_Instance)[k].alpha = beta = ForceRatio;
			(*p_Instance)[k].beta = beta;
			(*p_Instance)[k].ncycles = ncycles;
			i = inext;
			}
		return(OK);
		}
	
	// Now, case nseq > 0 in SMOOTH time

	imax = imaxseq;
	if(nseq < nmax && (imax < maxseq)) {
		imax = maxseq;
		/* We will complete streak markings beyond the end of... */
		/* ... the sequence, up to end of the phase diagram. */
		/* This is only needed if there are more sequences. */
		}
	// iprev = i;
	iprev = i + 1; // Fixed by BB 2022-02-17, checked this with -gr.tryTimePatterns
	jprev = -1;
	while(TRUE) {
		if(iprev >= imax) break;
		inextm = iprev;
FINDNEXTMARKED:
		while(!(*p_marked)[++inextm]) {
			if(inextm >= imax) return(OK);
			}
		To = (*p_T)[inextm] - (*p_T)[iprev];
		if(To <= ZERO) {	/* May be ZERO because of roundings */
			goto FINDNEXTMARKED;
			}
		sigmaridi = 0.;
		for(i=iprev; i < inextm;) {			/* Calculate sigmaridi */
			if(i < imaxseq) {
				k = (*((*p_Seq)[nseq]))[i];
				/* Here we will also consider silences "-" and empty objects "_" */
				inext = i; while((*((*p_Seq)[nseq]))[++inext] == 0);
				}
			else {
				k = 1;	/* Beyond end of sequence we create silences. */
				inext = i + 1;
				}
			if(inext > inextm) inext = inextm;
			/* In this case alpha will be too small on current object */
		
			if(k >= MaxObjects) {
			//	BPPrintMessage(0,odError,"=> Err. Calculate_alpha(). k >= MaxObjects (2)");
				BPPrintMessage(0,odError,"=> Err. Calculate_alpha(). k >= MaxObjects (2)\n");
				return(ABORT);
				}
			j = (*p_Instance)[k].object;
			if(k == 0) j = jprev;
			if(j <= 0) {
				i = inext; continue;
				}
			d = (double) (inext - i) * Kpress / Ratio;	/* Symbolic duration */
			if(j > 16383 || j == 1) { // Simple note or silence Fixed by BB 2022-02-24
				dur = 1000L; r = 1.;
				// BPPrintMessage(0,odInfo,"\nCalculating alpha (note or silence): k = %ld j = %ld, i = %ld, inext = %ld\n",(long)k,(long)j,(long)i,(long)inext);
				}
			else if(j >= Jbol) {  // time pattern
				r = ((double) (*p_Dur)[j]) / (*p_Tref)[j];
				if(trace_object_features) BPPrintMessage(0,odInfo,"This time pattern j = %ld r = %.2f d = %.2f Dur = %ld Tref = %ld\n",(long)k,(long)j,(double)r,(double)d,(long)(*p_Dur)[j],(long)(*p_Tref)[j]);
				// BPPrintMessage(0,odInfo,"\nCalculating alpha (pattern): k = %ld j = %ld, i = %ld, inext = %ld\n",(long)j,(long)i,(long)inext);
				}
			else { // Sound-object
				dur = (*p_Dur)[j];
				// BPPrintMessage(0,odInfo,"\nCalculating alpha (object): k = %ld j = %ld, i = %ld, inext = %ld\n",(long)k,(long)j,(long)i,(long)inext);
				if((*p_Tref)[j] > EPSILON && dur > EPSILON) {
					/* Striated object */
					r = ((double) dur) / (*p_Tref)[j];
					BPPrintMessage(0,odInfo,"-> striated\n");
					}
				else r = 1.;
				}
			sigmaridi += r * d;
			if(trace_object_features) BPPrintMessage(0,odInfo,"Calculate_alpha() (smooth) line > 0 To = %ld k = %ld j = %ld Jbol = %ld r = %.2f d = %.2f sigmaridi = %.2f i = %ld inext = %ld Dur = %ld Tref = %ld\n",(long)To,(long)k,(long)j,(long)Jbol,(double)r,(double)d,(double)sigmaridi,(long)i,(long)inext,(long)(*p_Dur)[j],(long)(*p_Tref)[j]);
			i = inext;
			}
		unfinished = FALSE;
		if(sigmaridi <= 0.) {	/* May happen if only out-time objects */
			iprev = inextm;
			continue;
			}
		currenttime = (*p_T)[iprev];
		for(i=iprev; i < inextm;) {			/* Now calculate alpha's */
			if(i < imaxseq) {
				k = (*((*p_Seq)[nseq]))[i];
				inext = i; while((*((*p_Seq)[nseq]))[++inext] == 0);
				}
			else {
				k = 1;	/* Beyond end of sequence we create silences. */
				inext = i + 1;
				}
			if(inext > inextm) {
				if(inext <= imaxseq) unfinished = TRUE;
				inext = inextm;
				}
			j = (*p_Instance)[k].object;
			if(k == 0) j = jprev;
			if(j <= 0) {
				(*p_Instance)[k].alpha = 0.;
				i = inext;
				continue;
				}
			d = (double) (inext - i) * Kpress / Ratio;
			if(j > 16383) {	// Simple note
				dur = 1000L; r = 1.;
				}
			else if(j >= Jbol) { // Time-pattern
				dur = (*p_Dur)[j];
				r = ((double) dur) / (*p_Tref)[j];
				}
			else { // Sound-object
				dur = (*p_Dur)[j];
				if((*p_Tref)[j] > EPSILON && dur > EPSILON) {
					/* Striated object */
					r = ((double) dur) / (*p_Tref)[j];
					}
				else r = 1.;
				}
			alpha = To * r * d / dur / sigmaridi;
			if(trace_object_features)
				BPPrintMessage(0,odInfo,"+ k = %ld j = %ld alpha = %.2f To = %ld, r = %.2f d = %.2f dur = %ld sigmaridi = %.2f\n",(long)k,(long)j,(double)alpha,(long)To,(double)r,(double)d,(long)dur,(double)sigmaridi);
			if(!unfinished && inext == inextm) {	/* Last object in section */
				toff = (*p_T)[inext];	/* This will compensate cumulated roundings */
				alpha = ((double)(toff - currenttime)) / dur;
				}
			gotcurrenttime = FALSE;
			if(k == 0 && !unfinished) {
				currenttime = (*p_T)[iprev] + (Milliseconds)(alpha * dur);
				/* Must calculate it now because alpha is going to change */
				gotcurrenttime = TRUE;
				toff = currenttime;
				alpha = ((double)(toff - ton)) / dur;
				k = kprev;
				if(k == 1 && !(*p_marked)[inext]) {
					/* alpha is meaningless for silences */
					(*p_T)[inext] = toff;
					(*p_marked)[inext] = TRUE;
					}
				}
			if(unfinished && k != 0) {
				ton = currenttime;
				/* Here alpha was incorrect but will be recalculated */
				jprev = j; kprev = k;
				}
			if(k == 1 && !(*p_marked)[inext]) {
				/* This is a "short" silence beginning in the same section */
				(*p_T)[inext] = currenttime + alpha * dur;
				(*p_marked)[inext] = TRUE;
				}
			if(alpha < 0.) {	/* Should no longer happen since To > ZERO */
				BPPrintMessage(0,odError,"=> Err. Calculate_alpha(). alpha=%.2f",alpha);
				alpha = 0.;
				}
			if(!gotcurrenttime) currenttime += (Milliseconds) (alpha * dur);
		
			beta = alpha; ncycles = 1;
			if(j >= Jbol) goto OKALPHA2;
			if((*p_RescaleMode)[j] == OK_RESCALE) goto CHECKMORE2;
			if((*p_FixScale)[j]) {
				beta = 1.;
				if((*p_CyclicMode)[j] == IRRELEVANT) {
					alpha = 1.; 
					goto OKALPHA2;
					}
				}
		/*	if((*p_FixScale)[j] && (*p_CyclicMode)[j] == IRRELEVANT) {
				alpha = beta = 1.; goto OKALPHA2;
				} */
			if(FixDilationRatioInCyclicObject(j,d,&alpha,&beta,&ncycles) == OK) goto OKALPHA2;
CHECKMORE2:
			if((*p_RescaleMode)[j] == DILATION_RATIO) { // 2026-09-15
				if(alpha > (*p_AlphaMax)[j]) beta = alpha = (*p_AlphaMax)[j];
				if(alpha < (*p_AlphaMin)[j]) beta = alpha = (*p_AlphaMin)[j];
				goto OKALPHA2;
				}
			if((*p_OkExpand)[j] && alpha >= 1.) goto OKALPHA2;
			if(!(*p_OkExpand)[j] && alpha > 1.) {
				alpha = 1.; goto OKALPHA2;
				}
			if((*p_OkCompress)[j] && alpha <= 1.) goto OKALPHA2;
			if(!(*p_OkCompress)[j] && alpha <= 1.) {
				alpha = 1.; goto OKALPHA2;
				}
	/*		if((!(*p_FixScale)[j] && !(*p_OkExpand)[j] && !(*p_OkCompress)[j]) || (*p_RescaleMode)[j] == DILATION_RATIO) {
				if(alpha > (*p_AlphaMax)[j]) beta = alpha = (*p_AlphaMax)[j];
				if(alpha < (*p_AlphaMin)[j]) beta = alpha = (*p_AlphaMin)[j];
				goto OKALPHA2;
				} */
			alpha = 1.;
		
OKALPHA2:
			if(alpha < 0.) alpha = 0.;
			if(beta < 0.) beta = 0.;
			(*p_Instance)[k].alpha = alpha;
			if(ForceRatio >= 0.) (*p_Instance)[k].alpha = beta = ForceRatio;
			(*p_Instance)[k].beta = beta;
			(*p_Instance)[k].ncycles = ncycles;
			i = inext;
			}
		iprev = inextm;
		}
	return(OK);
	}


int FixDilationRatioInCyclicObject(int j,double d,double *p_alpha,double *p_beta,int *p_ncycles) {
/* Examine objects with periodical part and see whether that part could be repeated
If so, fix the number of repetitions and the actual dilation ratio,...
 ... and change alpha if necessary */
	int n1,n2,limit;
	double objectperiod,cyclicafter,ncycles,pivpos;

	if(j > 16383) {
		*p_beta = *p_alpha;
		return(OK);
		}
	if(!(*p_FixScale)[j]) *p_beta = *p_alpha;

	// We're only concerned about cyclic objects with dilation ratio > 1.
	if((*p_alpha) <= 1. || (*p_CyclicMode)[j] == IRRELEVANT) return(MISSED);
	if(GetPeriod(j,1.,&objectperiod,&cyclicafter) == MISSED) return(MISSED);

	if(!(*p_FixScale)[j] && !(*p_OkExpand)[j] && !(*p_OkCompress)[j])
		limit = TRUE;		/* dilation ratio has a specified upper limit */
	else limit = FALSE;		/* it hasn't */

	if((*p_PivMode)[j] == PERCENT) {
		pivpos = (*p_alpha) * (*p_Dur)[j] * (*p_PivPos)[j] / 100.;
	//	BPPrintMessage(0,odInfo,"@@ PERCENT pivpos = %.2f, alpha = %.2f, Dur = %ld, PivPos = %.2f\n",pivpos,(*p_alpha),(*p_Dur)[j],(*p_PivPos)[j]);
		}
	else {
		pivpos = (*p_PivPos)[j];
	//	BPPrintMessage(0,odInfo,"@@ FIXVALUE pivpos = %.2f, alpha = %.2f, Dur = %ld, PivPos = %.2f\n",pivpos,(*p_alpha),(*p_Dur)[j],(*p_PivPos)[j]);
		}
	if(PlayFromInsertionPoint) pivpos = 0.;
		
	// First consider objects that can't be stretched

	if(!limit && (!(*p_OkExpand)[j] || (*p_CyclicMode)[j] == FIXVALUE)) {
		*p_beta = 1.;
	FINDCYCLES:
		ncycles = (double)(((*p_alpha) * (*p_Dur)[j] / (*p_beta))
			- cyclicafter + pivpos) / objectperiod;
		if((*p_ForceIntegerCycles)[j]) {
			if((ncycles - (int)ncycles) > 0.5) ncycles = ceil(ncycles);
			else ncycles = floor(ncycles);
			}
		// BPPrintMessage(0,odInfo,"@@@ j = %d, ncycles = %.4f, CyclicMode = %d\n",j,ncycles,(int)(*p_CyclicMode)[j]);
		goto SORTIR;
		}

	// Now, objects that can be stretched
		
	ncycles = d;

	// Adjust dilation ratio so that the duration is correct

	if(!(*p_FixScale)[j]) *p_beta = (*p_alpha) * (*p_Dur)[j] / (cyclicafter - pivpos + (ncycles * objectperiod));

	// If beta is too large, fix limit value and increase ncycles

	if((!limit && !(*p_OkExpand)[j] && *p_beta > 1.) || (limit && (*p_beta > (*p_AlphaMax)[j]))) {
		if(limit) *p_beta = (*p_AlphaMax)[j];
		else *p_beta = 1.;
		goto FINDCYCLES;
		}

	SORTIR:
	if((*p_Dur)[j] > EPSILON) {
		*p_alpha = *p_beta * (cyclicafter + (ncycles * objectperiod)) / (*p_Dur)[j];
		*p_ncycles = (int) ncycles;
		}
	else {
		*p_alpha = 0.;
		*p_ncycles = 0;
		}
	return(OK);
	}


int SetLimits(int nseq,Milliseconds** p_maxcoverbeg,Milliseconds** p_maxcoverend,
	Milliseconds** p_maxgapbeg,Milliseconds** p_maxgapend,Milliseconds** p_maxtruncbeg,
	Milliseconds** p_maxtruncend) {
	int j,k;
	long i;		
	Milliseconds maxcover1,maxcover2,maxgap1,maxgap2,maxtrunc1,maxtrunc2,dur;

	if(nseq >= Maxconc) {
		BPPrintMessage(0,odError,"=> Err. SetLimits(). nseq >= Maxconc\n");
		return(OK);
		}
	for(i=ZERO;; i++) {
		k = (*((*p_Seq)[nseq]))[i];
		if(k == -1) break;
		if(k < 1) continue;
		if(k >= MaxObjects) {
			BPPrintMessage(0,odError,"=> Err. SetLimits(). k >= MaxObjects\n");
			return(ABORT);
			}
		j = (*p_Instance)[k].object;
		if(j <= 0) {
			j = -j; dur = ZERO;
			}
		else {
			if(j < 16384) dur = (*p_Instance)[k].beta * (*p_Dur)[j];
			else dur = (*p_Instance)[k].beta * 1000L;	// Simple note
			}

		maxcover1 = maxcover2 = Infpos;
		if(j > 1 && j < 16384) {
			if((*p_CoverBeg)[j]) {
				if((*p_CoverBegMode)[j] == IRRELEVANT) maxcover1 = Infpos;
				else if((*p_CoverBegMode)[j] == FIXVALUE) maxcover1 = (*p_MaxCoverBeg)[j];
				else maxcover1 = (dur * (*p_MaxCoverBeg)[j]) / 100.;
				if(check_limits) BPPrintMessage(0,odInfo,"@@ maxcover1 = %ld, j = %d\n",maxcover1,j);
				}
			else maxcover1 = ZERO;
			}
		if(j > 1 && j < 16384) {
			if((*p_CoverEnd)[j]) {
				if((*p_CoverEndMode)[j] == IRRELEVANT) maxcover2 = Infpos;
				else if((*p_CoverEndMode)[j] == FIXVALUE) maxcover2 = (*p_MaxCoverEnd)[j];
				else maxcover2 = (dur * (*p_MaxCoverEnd)[j]) / 100.;
				if(check_limits) BPPrintMessage(0,odInfo,"@@ maxcover2 = %ld, j = %d\n",maxcover2,j);
				}
			else maxcover2 = ZERO;
			} 
		if(maxcover1 != Infpos && maxcover2 != Infpos && (maxcover1 + maxcover2) >= dur)
			(*p_maxcoverbeg)[i] = (*p_maxcoverend)[i] = Infpos;
		else {
			(*p_maxcoverbeg)[i] = maxcover1;
			(*p_maxcoverend)[i] = maxcover2;
			if(check_limits) BPPrintMessage(0,odInfo,"@@ (*p_maxcoverbeg)[%d] = %ld\n",i,(*p_maxcoverbeg)[i]);
			if(check_limits) BPPrintMessage(0,odInfo,"@@ (*p_maxcoverend)[%d] = %ld\n",i,(*p_maxcoverend)[i]);
			}
		
		maxgap1 = maxgap2 = Infpos;
		if(j > 1 && j < 16384) {
			if((*p_ContBeg)[j]) {
				if((*p_ContBegMode)[j] == IRRELEVANT) maxgap1 = ZERO;
				else if((*p_ContBegMode)[j] == FIXVALUE) maxgap1 = (*p_MaxBegGap)[j];
				else maxgap1 = (dur * (*p_MaxBegGap)[j]) / 100.;
				if(check_limits) BPPrintMessage(0,odInfo,"@@ maxgap1 = %ld, j = %d, MaxBegGap = %ld\n",maxgap1,j,(*p_MaxBegGap)[j]);
				}
			}
		(*p_maxgapbeg)[i] = maxgap1;	
		if(j > 1 && j < 16384) {
			if((*p_ContEnd)[j]) {
				if((*p_ContEndMode)[j] == IRRELEVANT) maxgap2 = ZERO;
				else if((*p_ContEndMode)[j] == FIXVALUE) maxgap2 = (*p_MaxEndGap)[j];
				else maxgap2 = (dur * (*p_MaxEndGap)[j]) / 100.;
				if(check_limits) BPPrintMessage(0,odInfo,"@@ maxgap2 = %ld, j = %d\n",maxgap2,j);
				}
			}
		(*p_maxgapend)[i] = maxgap2;
		
		maxtrunc1 = maxtrunc2 = dur;
		if(j > 1 && j < 16384) {
			if((*p_TruncBeg)[j]) {
				if((*p_TruncBegMode)[j] == IRRELEVANT) maxtrunc1 = dur;
				else if((*p_TruncBegMode)[j] == FIXVALUE) maxtrunc1 = (*p_MaxTruncBeg)[j];
				else maxtrunc1 = (dur * (*p_MaxTruncBeg)[j]) / 100.;
				if(check_limits) BPPrintMessage(0,odInfo,"@@ maxtrunc1 = %ld, j = %d\n",maxtrunc1,j);
				}
			else maxtrunc1 = ZERO;
			}
		if(maxtrunc1 < dur) (*p_maxtruncbeg)[i] = maxtrunc1;
		else (*p_maxtruncbeg)[i] = dur;
		if(check_limits) BPPrintMessage(0,odInfo,"@@ (*p_maxtruncbeg)[%d] = %ld\n",i,(*p_maxtruncbeg)[i]);
		
		if(j > 1 && j < 16384) {
			if((*p_TruncEnd)[j]) {
				if((*p_TruncEndMode)[j] == IRRELEVANT) maxtrunc2 = dur;
				else if((*p_TruncEndMode)[j] == FIXVALUE) maxtrunc2 = (*p_MaxTruncEnd)[j];
				else maxtrunc2 = (dur * (*p_MaxTruncEnd)[j]) / 100.;
				if(check_limits) BPPrintMessage(0,odInfo,"@@ maxtrunc2 = %ld, j = %d\n",maxtrunc2,j);
				}
			else maxtrunc2 = ZERO;
			}
		if(maxtrunc2 < dur) (*p_maxtruncend)[i] = maxtrunc2;
		else (*p_maxtruncend)[i] = dur;
		if(check_limits) BPPrintMessage(0,odInfo,"@@ (*p_maxtruncend)[%d] = %ld\n",i,(*p_maxtruncend)[i]);
		}
	return(OK);
	}


int AssignValue(int index,double value,int p,int level,long* p_kmx,
	CurrentParameters **p_deftcurrentparameters,
	CurrentParameters *p_currentparameters,ContParameters **p_contparameters,
	long id,tokenbyte ***pp_buff,double tempo,double scaling,Table **h_table) {

	float endval,maxbeats;
	KeyNumberMap mapendvalue;

	if((*p_contparameters)[level].values == NULL) {
		BPPrintMessage(0,odError,"=> Err. AssignValue(). (*p_contparameters)[level].values == NULL\n");
		return(ABORT);
		}

	(*((*p_contparameters)[level].values))[index].v0
		= (*((*p_contparameters)[level].values))[index].v1
		= (*((*p_contparameters)[level].values))[index].start
		= value;
	(*((*p_contparameters)[level].values))[index].active = TRUE;
	(*((*p_contparameters)[level].values))[index].known = TRUE;
	RememberParameter(index,p_currentparameters,value,(*((*p_contparameters)[level].values))[index].mode); // 2026-11-12 - fixed by Claude


	if(trace_set_variation) {
		BPPrintMessage(0,odInfo,"AssignValue() active = TRUE, value = v0 = %.2f v1 = %.2f for id = %d, paramindex = %d, level = %d\n",value,(long)(*((*p_contparameters)[level].values))[index].v1,id,index,level);
		}

	if(p < 128)
		(*((*p_contparameters)[level].values))[index].control = -1;
	else
		(*((*p_contparameters)[level].values))[index].control = p - 128;
		
	// 2026-11-08: SetVariation() expects the speed, from which it calculates tempo = speed / scaling - fixed by Claude
	if(SetVariation(-1,p_deftcurrentparameters,p_currentparameters,p_contparameters,level,index,id,*pp_buff,tempo * scaling,scaling,&endval,&mapendvalue,&maxbeats,h_table) != OK) return(ABORT);
	if(trace_set_variation) BPPrintMessage(0,odInfo,"AssignValue() endval = %d maxbeats = %.2f\n",(long)endval,maxbeats);
	(*((*p_contparameters)[level].values))[index].v1
		= endval;
	return(OK);
	}


int TokenToIndex(tokenbyte m,int index) {
	switch(m) {
		case T14:	/* _mod() */
			return(IMODULATION); break;
		case T15:	/* _pitchbend() */
			return(IPITCHBEND); break;
		case T16:	/* _press() */
			return(IPRESSURE); break;
		case T19:	/* _volume() */
			return(IVOLUME); break;
		case T29:	/* _pan() */
			return(IPANORAMIC); break;
		case T35:	/* _value() */
			return(index); break;
		}
	return(-1);
	}


int IndexToToken(int index) {
	if(index < 0) return(-1);
	switch(index) {
		case IMODULATION:	/* _mod() */
			return(T14); break;
		case IPITCHBEND:	/* _pitchbend() */
			return(T15); break;
		case IPRESSURE:	/* _press() */
			return(T16); break;
		case IVOLUME:	/* _volume() */
			return(T19); break;
		case IPANORAMIC:	/* _pan() */
			return(T29); break;
		default:	/* _value() */
			return(T35); break;
		}
	}


double FindValue(tokenbyte m,tokenbyte p,int chan) {
	double x,xx;
	int value,paramvalueindex,paramnameindex;

	// BPPrintMessage(0,odInfo,"FindValue m = %ld p = %ld chan = %d\n",(long)m,(long)p,chan);
	paramnameindex = -1;
	switch(m) {
		case T37: // _keymap()
			return(0.);
		case T32: // _ins()
		case T26: // _transpose()
		case T20: // _legato()
			return (double) p;
		case T10: // _chan()
		case T11: // _vel()
		case T16: // _press()
		case T19: // _volume()
		case T29: // _pan()
		case T38: // _rndvel()
		case T39: // _rotate() 
			if(p >= 128) {
				value = ParamValue[p-128];
				if(value == INT_MAX) {
					BPPrintMessage(0,odError,"=> Err. FindValue(). value == INT_MAX\n");
					value = 127;
					}
				return((double) value);
				}
			else return((double) p);
		}
	if(m == T35) {	// _value()
		paramnameindex = (p % 256);
		paramvalueindex = (p - paramnameindex) / 256;
		x = (*p_NumberConstant)[paramvalueindex];
		if(trace_set_variation)
			BPPrintMessage(0,odInfo,"T35 paramnameindex = %d paramvalueindex = %d, x = %.3f\n",paramnameindex,paramvalueindex,x);
		}
	else x = (double) p;
	// 2026-11-08 _value(pitchbend,x) is converted like _pitchbend(x), so that it is in cents when _pitchrange() is set.
	// It was taken as a raw MIDI value, and e.g. _value(pitchbend,-160) gave a negative pitchbend - fixed by Claude
	if(m == T15 || (m == T35 && paramnameindex == IPITCHBEND)) {  // _pitchbend() or _value(pitchbend,...)
		xx = x;
		if(PitchbendRange[chan] > 0)
			x = DEFTPITCHBEND + ((double) x * DEFTPITCHBEND / (double) PitchbendRange[chan]);
		if(trace_set_variation) BPPrintMessage(0,odInfo,"Pitchbend x = %ld, xx = %ld, range = %ld\n",(long)x,(long)xx,(long)PitchbendRange[chan]);
		if(x == 16384) x = 16383;
		if(x < 0 || x > 16383) {
			if(PitchbendRange[chan] > 0)
				my_sprintf(Message,"=> Pitchbend value (%ld cents) on channel %ld out of range (-%ld..%ld cents)",(long)xx,(long)chan,(long)PitchbendRange[chan],(long)PitchbendRange[chan]);
			else
				my_sprintf(Message,"=> Pitchbend value (%ld) on channel %ld out of range (0..16384)",(long)xx,(long)chan);
			BPPrintMessage(0,odError,"%s\n",Message);
			if(x < 0) x = 0;
			if(x >= 16384) x = 16383;
			return(x);
			}
		}
	return(x);
	}


double GetSymbolicDuration(int ignoreconcat,tokenbyte **p_buff,
	tokenbyte m_org,tokenbyte p_org,long id,double orgspeed,double orgscaling,
	int channel_org,int instrument_org,int part_org,int foundendconcatenation_org, int level_org) {
	unsigned long i;
	tokenbyte m,p,old_m,old_p;
	int level,instrument,part,channel;
	double tempo,tempomax,prodtempo,objectduration,speed,tick_start,tick_end,this_end,this_point,
		s,scaling,**p_duration_of_field,**p_duration_org;
	char tie_is_open,foundendconcatenation,found_beginning,justfinishedconcatenation,tie_closed;
	double reattack = -1.; // Date of the first later attack of the same key while the tie is open (2026-11-13)

	tie_is_open = found_beginning = tie_closed = FALSE;
	if(trace_get_duration)
		BPPrintMessage(1,odInfo,"GetSymbolicDuration()\n");
	if(m_org != T3 && m_org != T47 && m_org != T25 && m_org != T9) {
		BPPrintMessage(0,odError,"=> Err. GetSymbolicDuration(). m_org = %ld\n",(long)m_org);
		return(0);
		}
	if((p_duration_of_field = (double**) GiveSpace((Size)(Maxlevel+1)*sizeof(double))) == NULL)
		goto SORTIR;
	if((p_duration_org = (double**) GiveSpace((Size)(Maxlevel+1)*sizeof(double))) == NULL)
		goto SORTIR;
	for(level = 0; level <= Maxlevel; level++) {
		(*p_duration_of_field)[level] = (*p_duration_org)[level] = 0.;
		}
	if(orgspeed == 0.) {
		BPPrintMessage(0,odError,"=> Err. GetSymbolicDuration(). orgspeed == ZERO");
		objectduration = 0.;
		goto SORTIR;
		}
	tempomax = Prod / Kpress;
	scaling = orgscaling;
	speed = orgspeed;

	if(scaling != 0.) tempo = speed / scaling;
	else tempo = 0.;
	if(tempo == 0.) prodtempo = 0.;
	else prodtempo = (Prod / tempo);

	if(trace_get_duration)
		BPPrintMessage(1,odInfo,"GetSymbolicDuration Maxlevel = %ld, m = %d, p = %d, scaling = %.2f orgspeed = %.2f Prod = %.2f tempo = %.2f prodtempo = %.2f id = %ld channel = %d instrument = %d part = %d endconcatenation = %d\n",(long)Maxlevel,m_org,p_org,scaling,orgspeed,Prod,tempo,prodtempo,id,channel_org,instrument_org,part_org,foundendconcatenation_org);

	m = (*p_buff)[id+2L]; p = (*p_buff)[id+3L];
	if(m != T0 || p != 18) {
		ignoreconcat = TRUE;
		if(trace_get_duration)
			BPPrintMessage(1,odInfo,"no tie %ld|%ld\n",(long)m_org,(long)p_org);
		}
	else if(trace_get_duration)
			BPPrintMessage(1,odInfo,"tie %ld|%ld %ld|%ld\n",(long)m_org,(long)p_org,(long)m,(long)p);

	if(ignoreconcat) {
		i = id + 2L;
		old_m = m_org; old_p = p_org;
		}
	else {
		i = 0;
		old_m = old_p = 0;
		level_org = 0;
		}

	level = level_org;
	(*p_duration_of_field)[level] = (*p_duration_org)[level] = 0.;
	tie_is_open = found_beginning = foundendconcatenation = justfinishedconcatenation = FALSE;
	instrument = instrument_org;
	part = part_org;
	channel = channel_org;
	tick_start = 0.;
	this_end = -1.;
	tick_end = 0.;

	if(ignoreconcat) { // 2025-01-14
		tick_start = 0;
		if(((*p_buff)[i] != T3 && (*p_buff)[i] != T47) || (*p_buff)[i+1] != 0) {
			// No prolongation 2025-01-19
			if(trace_get_duration) BPPrintMessage(1,odInfo,"No prolongation\n");
			tick_end = prodtempo;
			goto SORTIR;
			}
		else {
			tick_end = prodtempo;
			}
		}
	for(i = i; ; i+=2) { 
		m = (*p_buff)[i]; p = (*p_buff)[i+1];
		if(m == TEND && p == TEND) break;
		if(trace_get_duration) BPPrintMessage(1,odInfo,"• %d|%d\n",m,p);
		if(!ignoreconcat && i == id) {
			if(m != m_org || p != p_org || (*p_buff)[i+2] != T0 || (*p_buff)[i+3] != 18 || channel != channel_org || instrument != instrument_org || part != part_org) {
				if(trace_get_duration) 
					BPPrintMessage(1,odInfo,"\n=> Error: %ld|%ld channel %d instrument %d part %d does not match the call %ld|%ld channel %d instrument %d part %d\n",(long)m,(long)p,channel,instrument,part,(long)m_org,(long)p_org,channel_org,instrument_org,part_org);
				goto SORTIR;
				}
			else {
				tick_start = (*p_duration_of_field)[level];
				(*p_duration_of_field)[level] += prodtempo;
				if(trace_get_duration) 
					BPPrintMessage(1,odInfo,"• Got it: %ld|%ld id = %ld level %d channel %d instrument %d part %d tick_start = %.2f foundendconcatenation = %d\n",(long)m,(long)p,(long)id,level,channel,instrument,part,tick_start,(int)foundendconcatenation);
				tie_is_open = TRUE;
				found_beginning = TRUE;
				old_m = m; old_p = p;
				continue; // Added by BB 2021-04-01
				}
			}
		
		if(m == T10) {	// Channel assignment _chan()
			channel = (int) FindValue(m,p,0);
			if(trace_get_duration) BPPrintMessage(1,odInfo,"channel = %d\n",channel);
			continue;
			}
		if(m == T32) { // Instrument assignment _ins()
			instrument = (int) FindValue(m,p,0);
			if(trace_get_duration) BPPrintMessage(1,odInfo,"\ninstrument = %d\n",instrument);
			continue;
			}
		if(m == T46) { // Part assignment _part()
			part = (int) FindValue(m,p,0);
			if(trace_get_duration) BPPrintMessage(1,odInfo,"\npart = %d\n",part);
			continue;
			}
		if(m == T0) {
			switch(p) {
				case 11:	// '/' speed up
					speed = GetScalingValue(p_buff,i);
					if(scaling != 0.) {
						tempo = speed / scaling;
						prodtempo = (Prod / tempo);
						}
					else tempo = 0.;
					if(tempo == 0.) prodtempo = 0.;
					else prodtempo = Prod / tempo;
					i += 4;
					continue;
					break;
				case 25:	// '\' speed down
					speed = 1. / (GetScalingValue(p_buff,i));
					if(scaling != 0.) {
						tempo = speed / scaling;
						prodtempo = (Prod / tempo);
						}
					else tempo = 0.;
					if(tempo == 0.) prodtempo = 0.;
					else prodtempo = (Prod / tempo);
					i += 4;
					continue;
					break;
				case 21:	// '*' scaling up 
					scaling = GetScalingValue(p_buff,i);
					if(scaling != 0.) {
						tempo = speed / scaling;
						prodtempo = (Prod / tempo);
						}
					else tempo = 0.;
					if(tempo == 0.) prodtempo = 0.;
					else prodtempo = (Prod / tempo);
					i += 4;
					continue;
					break;
				case 24:	// '**' scaling down
					s = GetScalingValue(p_buff,i);
					scaling = 1. / s;
					if(scaling != 0.) {
						tempo = speed / scaling;
						prodtempo = (Prod / tempo);
						}
					else tempo = 0.;
					if(tempo == 0.) prodtempo = 0.;
					else prodtempo = (Prod / tempo);
					i += 4;
					continue;
					break;
				case 12:	// '{'
				case 22:
					if(ignoreconcat || justfinishedconcatenation || tie_closed) goto SORTIR; // fixed 2025-12-01
					(*p_duration_org)[level+1] = (*p_duration_of_field)[level+1] = (*p_duration_of_field)[level];
					level++;
					if(trace_get_duration) BPPrintMessage(1,odInfo,"{ (%ld)\n",(long)(*p_duration_of_field)[level]);
					break;
				case 13:	// '}'
				case 23:
					if(ignoreconcat || justfinishedconcatenation || tie_closed) goto SORTIR; // fixed 2025-12-01
					level--;
					(*p_duration_of_field)[level] = (*p_duration_of_field)[level+1];
					if(trace_get_duration) BPPrintMessage(1,odInfo,"} (%ld)\n",(long)(*p_duration_of_field)[level]);
					break;
				case 14:	// comma
					if(ignoreconcat || justfinishedconcatenation || tie_closed) goto SORTIR; // Added 2025-01-21  fixed 2025-12-01
					(*p_duration_of_field)[level] = (*p_duration_org)[level];
					if(trace_get_duration) BPPrintMessage(1,odInfo,", (%ld)\n",(long)(*p_duration_of_field)[level]);
					(*p_duration_of_field)[level] = (*p_duration_org)[level];
					if(!foundendconcatenation)  // 2025-01-22 fixed 2025-12-01
						tick_end = tick_start;
					break;
				case 7:		// period 
					if(trace_get_duration) BPPrintMessage(1,odInfo,"•\n");
					break;
				case 18:	// '&' following terminal
					if(!found_beginning || ignoreconcat) continue;
					if(trace_get_duration) BPPrintMessage(1,odInfo,"+&\n");
					if(instrument != instrument_org || channel != channel_org || part != part_org) continue;
					if(old_m == m_org && old_p == p_org && instrument == instrument_org && part == part_org && channel == channel_org) {
						if(trace_get_duration)
							BPPrintMessage(1,odInfo,"Found '&' following terminal\n");
						if(!tie_is_open && !justfinishedconcatenation) {
							goto SORTIR;
							}
						if(trace_get_duration)
							BPPrintMessage(1,odInfo,"• Starting tie: %ld|%ld location %.2f\n",(long)old_m,(long)old_p,tick_start);
						tie_is_open = TRUE;
						tie_closed = FALSE; // The tie continues: "C4& &C4& &C4"
						}
					justfinishedconcatenation = FALSE;
					break;
				case 19:	// '&' preceding terminal
					if(!found_beginning || ignoreconcat) continue;
					if(trace_get_duration) BPPrintMessage(1,odInfo,"&+\n");
					if(instrument != instrument_org || channel != channel_org || part != part_org ) continue;
					this_end = (*p_duration_of_field)[level] + prodtempo;
					if(trace_get_duration)
						BPPrintMessage(1,odInfo,"Is this the end? this_end = %.2f tick_end = %.2f\n",this_end,tick_end);
					if(tie_is_open && this_end > tick_end) {
						if(trace_get_duration)
							BPPrintMessage(1,odInfo,"yes\n");
						foundendconcatenation = TRUE;
						}
					break;
				}
			old_m = m; old_p = p;
		//	continue; Fixed by BB 2021-04-01
			}
	/*	if(ignoreconcat) {  // 2025-01-14
			tick_start = 0.; tick_end = prodtempo;
			goto SORTIR;
			} */
		justfinishedconcatenation = FALSE;
		old_m = m; old_p = p;
//		if(m == T7 || m == T4) continue; // 2026-03-20
		if(m == T7) continue; // out-time object 
		if(m == T9) {	// time pattern
			(*p_duration_of_field)[level] += prodtempo;
			continue;
			}
		if((m == T3 || m == T47) && p == 0) {
			if(trace_get_duration) BPPrintMessage(1,odInfo,"_\n");
			(*p_duration_of_field)[level] += prodtempo;
			// 2026-11-12: count the prolongations only from the object itself. The scan of a tied object starts at the beginning
			// of the buffer, so the prolongations of earlier objects were counted, and the tied object got their duration
			// (oversized or shortened ties in -da.Beethoven-fugue-b-flat-major) - fixed by Claude
			if(found_beginning || ignoreconcat) tick_end += prodtempo; // 2025-01-21
			continue;
			}
		if(m == T3 || m == T47 || m == T25 || m == T4) {  // 2026-03-20
			// 2026-11-08: without a tie, the duration of the object is that of its own prolongations '_'.
			// Any following object ends it, even on another channel, instrument or part. Otherwise the prolongations
			// of the following objects were added, e.g. in "_chan(4) do4_____ _chan(2) sol4__" the duration of do4 was 8
			// instead of 6, and continuous parameters went out of range (item #30 of -da.checkControls) - fixed by Claude
			if(ignoreconcat) break;
			(*p_duration_of_field)[level] += prodtempo;
			if(trace_get_duration) {
				BPPrintMessage(0,odInfo,"<%ld|%ld> (%ld)\n",m,p,(long)(*p_duration_of_field)[level]);
				}
			if(found_beginning && tie_is_open && foundendconcatenation && m == m_org && p == p_org && instrument == instrument_org && part == part_org && channel == channel_org && this_end >= tick_start) {
				if(trace_get_duration) 
					BPPrintMessage(0,odInfo,"foundendconcatenation\n");
				// 2026-11-13: the tie is broken if this end tie starts after a new attack of the same key - fixed by Claude
				if(reattack >= 0. && (this_end - prodtempo - reattack) > (0.001 * prodtempo)) goto SORTIR;
				tick_end = this_end;
				tie_is_open = FALSE;
				tie_closed = TRUE;
				if(justfinishedconcatenation) break; // 2026-11-01
				justfinishedconcatenation = TRUE;
				if(trace_get_duration)
					BPPrintMessage(1,odInfo,"Perhaps end of tie: <%d|%d> i = %ld tick_start = %.2f this_end = %.2f found_beginning = %d\n",m,p,(long)i,tick_start,this_end,(int)found_beginning);
				continue;
				}
			// 2026-11-12: a new attack of the same key while the tie is open breaks the tie, which is then unbound.
			// Otherwise a later '&' of this key closed it, e.g. in "B3& Bb3 C4& &C4 - Cb4& &Cb4" B3 lasted until the end of &Cb4 - fixed by Claude
			// 2026-11-13: only an attack that starts after the tied object (the same key at the same date in another field is a parallel
			// note, e.g. cello and double bass on the same channel without _part() in -da.Prelude-to-a-Tragedy). And the scan follows the
			// order of the buffer, not of time: in "{E5&,...} {{1,C#5 G5 E5 G5}...,{1,&E5 D5}...}" the E5 of the first field is read before
			// "&E5" but starts when it ends (-da.Goldberg_5). So only its date is recorded, and the end tie decides - fixed by Claude
			if(found_beginning && tie_is_open && m == m_org && p == p_org && instrument == instrument_org && part == part_org && channel == channel_org
					&& reattack < 0. && ((*p_duration_of_field)[level] - prodtempo - tick_start) > (0.001 * prodtempo))
				reattack = (*p_duration_of_field)[level] - prodtempo;
			if(tie_closed) break; // 2026-11-09: the tie is closed, any following object ends it (item "bad 0.733" of -da.checkBadDurations) - fixed by Claude
		//	if((!tie_is_open) && !foundendconcatenation && !justfinishedconcatenation && p > 0 && this_end >= tick_start  && instrument == instrument_org && part == part_org && channel == channel_org ) {
			// 2026-11-12: only after the beginning of the tied object, otherwise the scan stopped before reaching it - fixed by Claude
			if(found_beginning && !tie_is_open && !foundendconcatenation && p > 0 && instrument == instrument_org && part == part_org && channel == channel_org) {
				// 2026-11-01 check with -da.Beethoven-fuge-flat-major
				if(tick_end > 0.) break;
				continue;
				}
			foundendconcatenation = FALSE;
			continue;
			}
		}

	SORTIR:

	if(!ignoreconcat && trace_get_duration)
		BPPrintMessage(1,odInfo,"Duration of expression = %.2f\n",(*p_duration_of_field)[level]);

	// 2026-11-09: a tie that was never closed is unbound. Its tick_end had accumulated the prolongations '_' of the following
	// fields, e.g. on another channel, and FillPhaseDiagram() then extended the line (item "bad 0.787" of -da.checkBadDurations) - fixed by Claude
	if(!ignoreconcat && found_beginning && tie_is_open) objectduration = -1.;
	else if(tick_start >= 0. && tick_end >= 0. && tick_end >= tick_start)
		objectduration = tick_end - tick_start;
	else objectduration = -1.;
	if(objectduration < 0.) {
		objectduration = 0.;
		if(!ignoreconcat && (m_org == T3 || m_org == T47)) {
			if(trace_get_duration) BPPrintMessage(1,odError,"\n=> An unbound tied object was ignored\n");
			(*(p_Missed_tie_event[instrument_org]))[p_org] += 1;
			}
		if(!ignoreconcat && m_org == T25) {
			if(trace_get_duration)
				BPPrintMessage(1,odError,"\n=> An unbound tied note was ignored\n");
			(*(p_Missed_tie_note[channel_org]))[p_org] += 1;
			}
		}

	if(trace_get_duration)
		BPPrintMessage(1,odInfo,"OBJECT DURATION = %.2f channel %d tick_start = %.2f tick_end = %.2f\n",objectduration,channel,tick_start,tick_end);
	MyDisposeHandle((Handle*)&p_duration_of_field);
	MyDisposeHandle((Handle*)&p_duration_org);
	return(objectduration);
	}


int RandomTime(Milliseconds *p_t1,int randomtime,Milliseconds duration,short *p_dont_randomize) {
	float x;

	if(randomtime == 0) return(OK);
	/* if(duration < (2 * randomtime)) { 2026-08-10
		*p_dont_randomize = TRUE;
		return(OK);
		} */
	if(*p_dont_randomize) {
		*p_dont_randomize = FALSE;
		return(OK);
		}
	x = (2. * bp3_rand() * ((float)randomtime)) / ((float)BP3_RAND_MAX);
	UsedRandom = TRUE;
	if(x > randomtime) x = randomtime - x;
	(*p_t1) += x;
	if((*p_t1) < ZERO) (*p_t1) = ZERO;
	return(OK);
	}
