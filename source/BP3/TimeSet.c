/* TimeSet.c (BP3) */ 

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


#ifndef DISPLAY_PHASE_DIAGRAM
#define DISPLAY_PHASE_DIAGRAM 0
#endif

#ifndef _H_BP2 
#include "-BP2.h"
#endif

#include "-BP2decl.h"

int trace_timeset = 0;

int TimeSet(tokenbyte ***pp_buff,long* p_kmx,long *p_tmin,long *p_tmax,unsigned long *p_maxseq,
	int* p_nmax,unsigned long **p_imaxseq,double maxseqapprox)
{
int i,result,bigitem,maxties,j,missed_ties;
short **p_articul;
time_t start_time,end_time;

// // HideWindow(Window[wInfo]);

if(CheckEmergency() != OK) return(ABORT);

*p_tmin = Infpos; *p_tmax = Infneg;
if(!Improvize) Chunk_number++;
// BPPrintMessage(0,odError,"\nChunk_number = %d\n",Chunk_number);

if((p_articul = (short**) GiveSpace((Size)Maxevent*sizeof(short))) == NULL) return(ABORT);

maxties = Jbol + Jpatt;
// BPPrintMessage(0,odInfo,"\n\n@@ maxties = %d\n\n",maxties);
for(i = 0; i < MAXINSTRUMENTS; i++)
	if((p_Tie_event[i] = (char**) GiveSpace((Size)maxties*sizeof(short))) == NULL) return(ABORT); // Added by BB 2021-02-07
for(i = 0; i <= MAXCHAN; i++)
	if((p_Tie_note[i] = (char**) GiveSpace((Size)128*sizeof(short))) == NULL) return(ABORT); // Added by BB 2021-02-07
for(i = 0; i < MAXINSTRUMENTS; i++)
	if((p_Missed_tie_event[i] = (int**) GiveSpace((Size)maxties*sizeof(int))) == NULL) return(ABORT); // Added by BB 2021-02-11
for(i = 0; i <= MAXCHAN; i++)
	if((p_Missed_tie_note[i] = (int**) GiveSpace((Size)128*sizeof(int))) == NULL) return(ABORT); // Added by BB 2021-02-11

for(j = 0; j <= MAXCHAN; j++)
	for(i = 0; i < 128; i++) {
		(*(p_Tie_note[j]))[i] = FALSE;
		(*(p_Missed_tie_note[j]))[i] = 0;
		}
for(j = 0; j < MAXINSTRUMENTS; j++)
	for(i = 0; i < maxties; i++) {
		(*(p_Tie_event[j]))[i] = FALSE;
		(*(p_Missed_tie_event[j]))[i] = 0;
		}

time(&start_time);
ProductionTime += start_time - ProductionStartTime;
result = FillPhaseDiagram(pp_buff,p_kmx,p_maxseq,p_nmax,p_imaxseq,
	maxseqapprox,&bigitem,p_articul);
time(&end_time);
PhaseDiagramTime += end_time - start_time;
	
if(result != OK) goto SORTIR;

start_time = end_time;
result = SetTimeObjects(bigitem,p_imaxseq,*p_maxseq,p_nmax,p_kmx,p_tmin,p_tmax,p_articul);
time(&end_time);
TimeSettingTime += end_time - start_time;
// ProductionTime += TimeSettingTime;

if(trace_timeset) BPPrintMessage(0,odInfo,"End TimeSet() maxseq = %ld\n\n",(long)*p_maxseq);

SORTIR:
MyDisposeHandle((Handle*)&p_articul);

missed_ties = 0;
for(j = 0; j <= MAXCHAN; j++) {
	for(i = 0; i < 128; i++) {
		if((*(p_Missed_tie_note[j]))[i] > 0) {
			BPPrintMessage(0,odError,"=> Missed tied note key #%d (%d occurrences MIDI channel %d)\n",i,(*(p_Missed_tie_note[j]))[i],j);
			missed_ties++;
			}
		}
	MyDisposeHandle((Handle*)&(p_Tie_note[j]));
	MyDisposeHandle((Handle*)&(p_Missed_tie_note[j]));
	}
if(missed_ties > 0) {
	if(!PlayChunks)
		BPPrintMessage(0,odError,"=> Total %d tied note(s) have been missed\n\n",missed_ties);
	else BPPrintMessage(0,odError,"=> Total %d tied note(s) have been missed in chunk #%d\n\n",missed_ties,Chunk_number);
	}

missed_ties = 0;
for(j = 0; j < MAXINSTRUMENTS; j++) {
	for(i = 0; i < maxties; i++) {
		if((*(p_Missed_tie_event[j]))[i] > 0) {
			BPPrintMessage(0,odError,"=> Missed tied event #%d (%d occurrences Csound instrument %d)\n",i,(*(p_Missed_tie_event[j]))[i],j);
			missed_ties++;
			}
		}
	MyDisposeHandle((Handle*)&(p_Tie_event[j]));
	MyDisposeHandle((Handle*)&(p_Missed_tie_event[j]));
	}
if(missed_ties > 0) BPPrintMessage(0,odError,"=> Total %d missed tied events in chunk #%d\n\n",missed_ties,Chunk_number);

// my_sprintf(Message,"");
strcpy(Message,""); // Fixed by BB 2022-02-20
return(result);
}


int SetTimeObjects(int bigitem,unsigned long **p_imaxseq,unsigned long maxseq,int *p_nmax,
	long *p_kmx,long *p_tmin,long *p_tmax,short **p_articul)

{
int nseq,r,rep,BTflag,result,stepthis,first,dirtymem,compiledmem,nature_time,a,j,key,last_line,outtimeevents;
long **ptr;
long k;
long i,iseq;
unsigned long jj,jn;
char **p_marked,**p_alphadone;
OSErr oserr;
double period,in,imax;
p_list **ptag;
Milliseconds tmax,t,**p_DELTA,t1,t2,**p_time1,**p_time2,**p_maxcoverbeg,**p_maxcoverend,
	**p_maxgapbeg,**p_maxgapend,**p_maxtruncbeg,**p_maxtruncend,
	olddelta,max_end_time;
unsigned long tstart,handle_size;

nature_time = Nature_of_time;	/* Must not change during computation */

handle_size = maxseq + 50; // Added by BB 2021-03-23
if(trace_timeset)
	BPPrintMessage(0,odInfo,"\nhandle_size for Seq[nseq] and DELTA = %ld\n",(long)handle_size);

for(nseq=0; nseq <= (*p_nmax); nseq++) {
	ptr = (*p_Seq)[nseq];
	if(MySetHandleSize((Handle*)&ptr,(Size)handle_size * sizeof(long)) != OK) return(ABORT); 
	(*p_Seq)[nseq] = ptr;
	}

// Calculate time structure
	
in = 1.; jn = ZERO;
period = ((double) Pclock) * 1000. * CorrectionFactor / Qclock;
// BPPrintMessage(0,odInfo,"Pclock = %ld Qclock = %ld, CorrectionFactor = %.3f\n",(long)Pclock,(long)Qclock,CorrectionFactor);

// if(trace_timeset)
BPPrintMessage(0,odInfo,"Setting time streaks on %d lines\n",(*p_nmax));

while(TRUE) {
	jj= Class(in);
	if(jj > maxseq) break;
	if(jj > jn || in == 1.) {
		jn = jj;		/* Write only once */
		if(nature_time == STRIATED) {
			(*p_T)[jn] = (Milliseconds) ((period * (in - 1.)) / Ratio);
			// BPPrintMessage(0,odError,"jn = %ld (*p_T)[jn] = %ld in = %ld period = %.0f Ratio = %.0f\n",(long)jn,(long)(*p_T)[jn],(long)in,(double)period,(double)Ratio);
			if(Kpress > 2) { // Compensates roundings // Added by BB 2021-03-21
				if(jn > 0) (*p_T)[jn-1] = (*p_T)[jn];
				if((jn + 1) <= maxseq) (*p_T)[jn+1] = (*p_T)[jn];
				}
			}
		else (*p_T)[jn] = ZERO;
		}
	in += Kpress;
	}

// Now display phase diagram (optional)
#if DISPLAY_PHASE_DIAGRAM

if(trace_timeset) {
	BPPrintMessage(0,odInfo,"\n");
	BPPrintMessage(0,odInfo,"@Minconc = %ld    Maxconc = %ld\n\n",(long)Minconc,(long)Maxconc);
	}

for(k=ZERO; k < Maxevent; k++) {
	if((j=(*p_Instance)[k].object) >= 0) {
		if(j >= Jbol) {
			if(j < 16384)
				my_sprintf(Message,"(Pattern %s)",*((*p_Patt)[j-Jbol]));
			else {
				key = j - 16384;
				key = MapThisKey(key,0.,(*p_Instance)[k].mapmode,
					&((*p_Instance)[k].map0),&((*p_Instance)[k].map1));
				PrintThisNote(-1,key,0,-1,LineBuff);
				my_sprintf(Message,"(%ld %s chan %ld)",(long)k,
					LineBuff,(long)ChannelConvert((*p_Instance)[k].channel));
				}
			}
		else my_sprintf(Message,"(%s chan %ld)",*((*p_Bol)[j]),(long)ChannelConvert((*p_Instance)[k].channel));
		}
	else {
		j = -j;
		if(j >= Jbol) {
			if(j < 16384)
				my_sprintf(Message,"(<<Pattern %s>>)",*((*p_Patt)[j-Jbol]));
			else {
				key = j - 16384;
				key = MapThisKey(key,0.,(*p_Instance)[k].mapmode,
					&((*p_Instance)[k].map0),&((*p_Instance)[k].map1));
				PrintThisNote(-1,key,0,-1,LineBuff);
				my_sprintf(Message,"(%ld <<%s>> chan %ld)",(long)k,
					LineBuff,(long)ChannelConvert((*p_Instance)[k].channel));
				}
			}
		else my_sprintf(Message,"(<<%s>> chan %ld)",*((*p_Bol)[j]),(long)ChannelConvert((*p_Instance)[k].channel));;
		}
	if(trace_timeset)
		BPPrintMessage(0,odInfo,Message);
	}
BPPrintMessage(0,odInfo,"\n");
	
if(Maxevent < 1000) {
	BPPrintMessage(0,odInfo,"\n");
//	if((*p_nmax) > 1) last_line = (*p_nmax) - 1;
//	else last_line =  1;
	last_line = (*p_nmax);
	for(nseq=0; nseq <= last_line; nseq++) {
		for(iseq=0L; iseq <= (*p_imaxseq)[nseq]; iseq++) {
			k = (*((*p_Seq)[nseq]))[iseq];
			if(k >= 0) {
				if((*p_ObjectSpecs)[k] != NULL) {
					ptag = WaitList(k);
					while(ptag != NULL) {
						my_sprintf(Message,"<<W%ld>>",(long)((**ptag).x));
						ptag = (**ptag).p;
						}
					}
				j = (*p_Instance)[k].object;
				if(j >= 0) {
					if(j >= Jbol) {
						if(j < 16384)
							my_sprintf(Message,"%s ",*((*p_Patt)[j-Jbol]));
						else {
							key = j - 16384;
							key = MapThisKey(key,0.,(*p_Instance)[k].mapmode,
								&((*p_Instance)[k].map0),&((*p_Instance)[k].map1));
							PrintThisNote(-1,key,0,-1,LineBuff);
							my_sprintf(Message,"%s ",LineBuff);
							}
						}
					else {
					/*	if(k > 1 && j == 1)
							my_sprintf(Message,"~ ");
						else */ my_sprintf(Message,"%s ",*((*p_Bol)[j]));
						}
					}
				else {
					j = -j;
			/*		if(k > 1 && j == 1)
						my_sprintf(Message,"<<~>> ");
					else { */
						if(j < 16384)
							my_sprintf(Message,"<<%s>> ",*((*p_Bol)[j]));
						else {
							key = j - 16384;
							key = MapThisKey(key,0.,(*p_Instance)[k].mapmode,
								&((*p_Instance)[k].map0),&((*p_Instance)[k].map1));
							PrintThisNote(-1,key,0,-1,LineBuff);
							my_sprintf(Message,"<<%s>> ",LineBuff);
							}
				//		}
					}
				BPPrintMessage(0,odInfo,Message);
				}
			else BPPrintMessage(0,odInfo,"(%ld) ",(long)k);
			}
		BPPrintMessage(0,odInfo,"\n");
		}
	BPPrintMessage(0,odInfo,"\n");
	}

/* if(trace_timeset) {
	BPPrintMessage(0,odInfo,"\nT[i], i = 1,%ld:\n",(long)maxseq);
	for(i=1L; i <= maxseq; i++)
		BPPrintMessage(0,odInfo,"%ld ",(long)(*p_T)[i]);
	} */

BPPrintMessage(0,odInfo,"\n");

for(nseq=0; nseq <= (*p_nmax); nseq++) {
	if(trace_timeset) BPPrintMessage(0,odInfo,"\nnseq = %d, imaxseq[nseq] = %ld\n",nseq,(long)(*p_imaxseq)[nseq]);
	for(iseq=0L; iseq <= (*p_imaxseq)[nseq]; iseq++) {
		k = (*((*p_Seq)[nseq]))[iseq];
		if(k == -1) break;
		if(k > 0) {
			if(trace_timeset) BPPrintMessage(0,odInfo,"(k=%d ",k);
			if((*p_ObjectSpecs)[k] != NULL) {
				ptag = WaitList(k);
				while(ptag != NULL) {
					ptag = (**ptag).p;
					}
				}
			j = (*p_Instance)[k].object;
			if(trace_timeset) {
				if(j > 16383) BPPrintMessage(0,odInfo,"%s) ",*((*(p_NoteName[NoteConvention]))[j-16384]));
				else BPPrintMessage(0,odInfo,"j=%d) ",j);
				}
			if(j >= Jbol+Jpatt && j < 16384) {
				if(trace_timeset) BPPrintMessage(0,odError,"\n=> ERROR: j >= Jbol+Jpatt\n");
				return(ABORT);
				}
			}
		else {
			if(trace_timeset && k < 0) BPPrintMessage(0,odError,"=> ERROR: k < 0 nseq=%ld i=%ld im=%ul k=%ld\n",(long)nseq,(long)i,(unsigned long)maxseq,(long)k);
			}
		}
	if(trace_timeset) BPPrintMessage(0,odInfo,"\n");
	}
#endif
// End DISPLAY_PHASE_DIAGRAM

if(trace_timeset) {
	BPPrintMessage(0,odError,"\nT[i], i = 1,%ld:\n",(long)maxseq);
	for(i=1L; i <= maxseq; i++)
		BPPrintMessage(0,odError,"%ld ",(long)(*p_T)[i]);
	}
max_end_time = (*p_T)[maxseq];
if(trace_timeset) BPPrintMessage(0,odError,"\nmax_end_time = %ld\n",(long)max_end_time);

if(trace_timeset)
	BPPrintMessage(0,odError,"\nPositioning %ld sound-objects...\n",(long)(*p_kmx));

// if(DoSystem() != OK) return(ABORT);

if((p_marked = (char**) GiveSpace((Size)handle_size * sizeof(char))) == NULL)
	return(ABORT);
if((p_DELTA = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds)))
		== NULL) return(ABORT);
// for(i=ZERO; i <= maxseq; i++) {
for(i=ZERO; i < handle_size; i++) { // Fixed by BB 2021-03-23
	(*p_DELTA)[i] = ZERO; (*p_marked)[i] = FALSE;
	}
if((p_time1 = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_time2 = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_maxcoverbeg = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_maxcoverend = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_maxgapbeg = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_maxgapend = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_maxtruncbeg = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_maxtruncend = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
	return(ABORT);
	
if((p_alphadone = (char**) GiveSpace((Size)((*p_nmax)+1)*sizeof(char))) == NULL)
	return(ABORT);
for(nseq=0; nseq <= (*p_nmax); nseq++) (*p_alphadone)[nseq] = FALSE;

result = OK; first = TRUE;

// if(IsMidiDriverOn()) tstart = GetDriverTime();

STARTAGAIN:
for(nseq=0; nseq <= (*p_nmax); nseq++) {
	PleaseWait();
	if(trace_timeset) {
		if(Kpress > 1.) {
			if(Kpress < ULONG_MAX)
				my_sprintf(Message,"%ld objects. Compression rate = %.0f  Sequence %ld/%ld",
					(long)(*p_kmx),Kpress,((long)nseq)+1L,((long)(*p_nmax))+1L);
			else
				my_sprintf(Message,"%ld objects. Compression rate = %.0f  Sequence %ld/%ld",
					(long)(*p_kmx),Kpress,((long)nseq)+1L,((long)(*p_nmax))+1L);
			}
		else
			my_sprintf(Message,"%ld objects. No compression.  Sequence %ld/%ld",
				(long)(*p_kmx),(long)nseq+1,(long)(*p_nmax)+1L);
		FlashInfo(Message);
		}
	if(!(*p_alphadone)[nseq]) {
		if(trace_timeset) ShowMessage(FALSE,wMessage,"Sizing objects");
		if((result=Calculate_alpha(nseq,*p_nmax,maxseq,(*p_imaxseq)[nseq],nature_time,
			p_marked)) != OK) goto EXIT1;
		if(trace_timeset) ShowMessage(FALSE,wMessage,"Setting limits");
		if((result=SetLimits(nseq,p_maxcoverbeg,p_maxcoverend,p_maxgapbeg,p_maxgapend,
			p_maxtruncbeg,p_maxtruncend)) != OK) goto EXIT1;
		}
	(*p_alphadone)[nseq] = TRUE;

#if BP_CARBON_GUI_FORGET_THIS
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	// This block is very similar to the middle of InterruptTimeSet().  Could we
	// refactor the shared code into a function ?? -- akozar, 20130830
/*	if((r=MyButton(1)) != MISSED) {
		StopCount(0);
		
		Interrupted = TRUE;
		dirtymem = Dirty[wAlphabet]; Dirty[wAlphabet] = FALSE;
		compiledmem = CompiledGr;
		if(r == OK) while((r = MainEvent()) != RESUME && r != STOP && r != EXIT);
		if(r == EXIT) {
			result = r; goto EXIT1;
			}
		if(Dirty[wAlphabet]) {
			Alert1("Alphabet changed. Must recompile...");
			result = ABORT; goto EXIT1;
			}
		Dirty[wAlphabet] = dirtymem;
		if((r == STOP) || (compiledmem && (CompileCheck() != OK))) {
			result = ABORT; goto EXIT1;
			}
		if(LoadedIn && (!CompiledIn && (result=CompileInteraction()) != OK))
			goto EXIT1;
		if(Dirty[wTimeAccuracy]) {
			ResetMIDI(FALSE);
			if((result=CheckSettings()) == ABORT) goto EXIT1;
			else {
				Dirty[wTimeAccuracy] = FALSE; result = AGAIN;
				}
			goto EXIT1;
			}
			
		if(IsMidiDriverOn()) tstart = GetDriverTime();
		
		PleaseWait();
		}
	r = OK;
	if(EventState != NO) {
		result = EventState; goto EXIT1;
		} */
#endif /* BP_CARBON_GUI_FORGET_THIS */

	if(DisplayTimeSet) {
		if(trace_timeset) BPPrintMessage(0,odInfo,"\nSequence #%ld\n",(long)(nseq+1L));
		}
	CoverOK = DiscontinuityOK = stepthis = FALSE;
	if(NoConstraint) {
		CoverOK = DiscontinuityOK = TRUE;
		}

/*	if(trace_timeset) {
		BPPrintMessage(0,odInfo,"\nBefore Fix(), T[i], i = 1,%ld:\n",(long)maxseq);
		for(i=1L; i <= maxseq; i++)
			BPPrintMessage(0,odInfo,"%ld ",(long)(*p_T)[i]);
		BPPrintMessage(0,odInfo,"%s","\n");
		} */

TRY:
	if(trace_timeset) ShowMessage(FALSE,wMessage,"\nPlacing objects");
/*	if(trace_timeset) {
		BPPrintMessage(0,odInfo,"\nBefore Fix(), T[i], i = 1,%ld:\n",(long)maxseq);
		for(i=1L; i <= maxseq; i++)
			BPPrintMessage(0,odInfo,"%ld ",(long)(*p_T)[i]);
		BPPrintMessage(0,odInfo,"%s","\n");
		BPPrintMessage(0,odInfo,"Before Fix(), time2:\n");
		for(i=1L; i <= maxseq; i++)
			BPPrintMessage(0,odInfo,"[%ld]%ld ",(long)i,(long)(*p_time2)[i]);
		BPPrintMessage(0,odInfo,"%s","\n");
		} */
	if((result=Fix(nseq,p_time1,p_time2,nature_time)) != OK) goto EXIT1;

/*	if(trace_timeset) {
		BPPrintMessage(0,odInfo,"\nAfter Fix(), T[i], i = 1,%ld:\n",(long)maxseq);
		for(i=1L; i <= maxseq; i++)
			BPPrintMessage(0,odInfo,"%ld ",(long)(*p_T)[i]);
		BPPrintMessage(0,odInfo,"%s","\n");
		BPPrintMessage(0,odInfo,"After Fix(), time2:\n");
		for(i=1L; i <= maxseq; i++)
			BPPrintMessage(0,odInfo,"[%ld]%ld ",(long)i,(long)(*p_time2)[i]);
		BPPrintMessage(0,odInfo,"%s","\n");
		} */
	
	if(trace_timeset) BPPrintMessage(0,odInfo,"Solving constraints Locate() for nseq = %ld\n",(long)nseq);
	r = Locate(nseq,p_imaxseq,maxseq,*p_kmx,p_DELTA,&tstart,p_time1,
			p_time2,p_maxcoverbeg,p_maxcoverend,p_maxgapbeg,p_maxgapend,p_maxtruncbeg,
			p_maxtruncend,nature_time,first,p_marked);
	if(r == ABORT || r == ENDREPEAT || r == AGAIN || r == EXIT) {
		result = r;
		goto EXIT1;
		}
	
	/* if(trace_timeset) {
		BPPrintMessage(0,odInfo,"\nAfter Locate(), T[i], i = 1,%ld:\n",(long)maxseq);
		for(i=1L; i <= maxseq; i++)
			BPPrintMessage(0,odInfo,"%ld ",(long)(*p_T)[i]);
		BPPrintMessage(0,odInfo,"%s","\n");
		BPPrintMessage(0,odInfo,"After Locate(), DELTA[i], i = 1,%ld:\n",(long)maxseq);
		for(i=1L; i <= maxseq; i++)
			BPPrintMessage(0,odInfo,"%ld ",(long)(*p_DELTA)[i]);
		BPPrintMessage(0,odInfo,"%s","\n");
		} */
	
	if(r == MISSED || r == QUICK) {
		result = MISSED;
		if(ShowGraphic || !Improvize) {
			FlashInfo("Must release constraints...");
			}
		if(r == QUICK) {	/* Find a quick solution */
			if(ShowMessages) ShowMessage(TRUE,wMessage,"Quick!");
			DiscontinuityOK = CoverOK = TRUE;
			goto TRY;
			}
		if(!StepTimeSet) { 
			if(!CoverOK && !DiscontinuityOK) {
				if(ShowGraphic || PlaySelectionOn || !Improvize)
					ShowMessage(TRUE,wMessage,"Releasing overlapping");
				CoverOK = TRUE;
				goto TRY;
				}
			if(!DiscontinuityOK) {
				if(ShowGraphic || PlaySelectionOn || !Improvize)
					ShowMessage(TRUE,wMessage,"Releasing continuity");
				CoverOK = FALSE;
				DiscontinuityOK = TRUE;
				goto TRY;
				}
			if(!CoverOK) {
				if(ShowGraphic || PlaySelectionOn || !Improvize)
					ShowMessage(TRUE,wMessage,"Releasing continuity and overlapping");
				CoverOK = TRUE;
				goto TRY;
				}
			goto EXIT1;
			}
		Print(wTrace,"\nRelease:\n\n");
		if(!DiscontinuityOK) Print(wTrace,"C)ontinuity\n");
		if(!CoverOK) Print(wTrace,"O)verlapping\n");
		if(!CoverOK || !DiscontinuityOK)
			Print(wTrace,"B)oth continuity and overlapping\n");
		if(!StepTimeSet && !stepthis)
			Print(wTrace,"S)tep by step decision\n\n");
		Print(wTrace,"Q)uit\n");
		
QUEST1:
		rep = GetCap();
		if(rep == 'Q') {
				result = ABORT;
				goto EXIT1;
				}
		if(rep == 'C') {
			DiscontinuityOK = TRUE; CoverOK = FALSE;
			}
		else
			if(rep == 'O') {
				CoverOK = TRUE; DiscontinuityOK = FALSE;
				}
			else
				if(rep == 'B') {
					CoverOK = TRUE; DiscontinuityOK = TRUE;
					}
				else
					if(rep == 'S') {
						CoverOK = FALSE; DiscontinuityOK = FALSE;
						stepthis = TRUE;
						}
					else {
						SysBeep(10);
						goto QUEST1;
						}
		goto TRY;
		}
		
	if(r == BACKTRACK) {
		Print(wTrace,"\nS)how again solution\n");
		Print(wTrace,"A)ccept solution\n");
		Print(wTrace,"Q)uit\n");
QUEST2:
		rep = GetCap();
		Print(wTrace,"\n");
		switch(rep) {
			case 'S':	goto TRY;
			case 'A':	break;
			case 'Q':	{
					result = ABORT;
					goto EXIT1;
					}
			default: SysBeep(10); goto QUEST2;
			}
		}
	first = FALSE;

	BTflag = FALSE;
	tmax = Infneg;
	olddelta = ZERO;
	if(nature_time == STRIATED) {
		if(trace_timeset) ShowMessage(FALSE,wMessage,"Calculating global drift");
		for(i=1L; i <= maxseq; i++) {
			if((*p_DELTA)[i] > EPSILON) BTflag = TRUE;
			else {		/* Correct last streaks for which DELTA is not known */
				if(BTflag) (*p_DELTA)[i] = olddelta;
				}
			(*p_T)[i] += (*p_DELTA)[i];
			olddelta = (*p_DELTA)[i];
			(*p_DELTA)[i] = ZERO;
			}
		}
	else {
		if(trace_timeset) ShowMessage(FALSE,wMessage,"Interpolating streaks");
		(*p_T)[0] = ZERO; // Fixed by BB 2022-02-23
	//	for(i=1L; i <= maxseq; i++) { Fixed by BB 2021-03-20
		for(i=ZERO; i <= maxseq; i++) {
			if(i <= (*p_imaxseq)[nseq] && ((k=(*((*p_Seq)[nseq]))[i]) >= 1 || k == -1))
						(*p_marked)[i] = TRUE;
			if((*p_marked)[i] == TRUE) {
				// BPPrintMessage(0,odInfo,"+ marked i = %ld k = %ld T[i] = %ld, DELTA[i] = %ld\n",(long)i,(long)k,(long)(*p_T)[i],(long)(*p_DELTA)[i]);
				(*p_T)[i] = (*p_T)[i] + (*p_DELTA)[i];
				if((*p_T)[i] >= tmax) tmax = (*p_T)[i];
				else {	/* Must be increasing.  Sometimes local errors in smooth time */
					(*p_T)[i] = tmax; /* This correction is very small */
					}
				}
			else {
				if(i <= (*p_imaxseq)[nseq] && (*p_T)[i] != ZERO) {
					if(trace_timeset) BPPrintMessage(0,odError,"=> Err. SetTimeObjects() nseq = %ld maxseq = %ld (*p_T)[%ld] = %ld\n",(long)nseq,(long)maxseq,(long)i,(long)(*p_T)[i]);
					(*p_T)[i] = ZERO;
					}
				}
			(*p_DELTA)[i] = ZERO;
			}
		}
	
	/* if(trace_timeset) {
		BPPrintMessage(0,odInfo,"\nAfter Interpolating streaks, T[i], i = 1,%ld:\n",(long)maxseq);
		for(i=1L; i <= maxseq; i++)
			BPPrintMessage(0,odInfo,"%ld ",(long)(*p_T)[i]);
		BPPrintMessage(0,odInfo,"%s","\n");
		} */

	if(BTflag) {
		if(trace_timeset)
			ShowMessage(TRUE,wMessage,"Starting again algorithm (broke tempo)...");
		if(DisplayTimeSet) {
			my_sprintf(Message,"\nBroke tempo. T[i], i = 1,maxseq:\n");
			Print(wTrace,Message);
			for(i=1; i <= maxseq; i++) {
				my_sprintf(Message,"%ld ",(long)(*p_T)[i]);
				Print(wTrace,Message);
				}
			}
		goto STARTAGAIN;
		}
	
	if(trace_timeset) {
		BPPrintMessage(0,odInfo,"\nT[i], i = 1,%ld:\n",(long)maxseq);
		for(i=1L; i <= maxseq; i++)
			BPPrintMessage(0,odInfo,"%ld ",(long)(*p_T)[i]);
		BPPrintMessage(0,odInfo,"\n");
		for(k = 2; k < 8; k++) {
			BPPrintMessage(0,odInfo,"k = %d, starttime = %ld, endtime = %ld\n",k,(long)(*p_Instance)[k].starttime,(long)(*p_Instance)[k].endtime);
			}
		}

	/* Update tmin and tmax */
	imax = (*p_imaxseq)[nseq] - 1L;
	if(trace_timeset) {
		BPPrintMessage(0,odInfo,"\nimax = %ld\n",(long)imax);
		for(i = 0; i <= imax; i++) {
			BPPrintMessage(0,odInfo,"i = %ld, k = %ld\n",(long)i,(long)(*((*p_Seq)[nseq]))[i]);
			}
		}
	if(imax >= 0) {  // 2024-05-11 fixed ">=" this condition is probably useless
		if(trace_timeset) BPPrintMessage(0,odInfo,"\nUpdating tmin and tmax, nseq = %ld, imax = %ld, kmx = %ld\n",(long)nseq,(long)imax,(long)*p_kmx);
		i = imax;
		while(i >= 0 && (k=(*((*p_Seq)[nseq]))[i]) < 2) i--;
		if(trace_timeset) BPPrintMessage(0,odInfo,"max: i = %ld, k = %ld\n",(long)i,(long)k);
		if(k > *p_kmx)
			BPPrintMessage(0,odError,"=> Error in TimeSet(). k > *p_kmx\n");
		else if(i > 0 && (t=((*p_Instance)[k].endtime+(*p_Instance)[k].truncend)) >= *p_tmax) // 2024-05-11 fixed ">="
			*p_tmax = t;
		if(trace_timeset) BPPrintMessage(0,odInfo,"t = %ld, tmax = %ld, endtime = %ld, truncend = %ld\n",(long)t,(long)*p_tmax,(long)(*p_Instance)[k].endtime,(long)(*p_Instance)[k].truncend);
		// i = 1L;
		i = ZERO; // Fixed by BB 2021-03-22
		while(i <= imax && (k=(*((*p_Seq)[nseq]))[i]) < 2) i++;
		if(trace_timeset) BPPrintMessage(0,odInfo,"min: i = %ld, k = %ld\n",(long)i,(long)k);
		if(k > *p_kmx) {
			BPPrintMessage(0,odError,"=> Error in TimeSet(). k > *p_kmx\n");
			}
		else if(i <= imax && (t=((*p_Instance)[k].starttime-(*p_Instance)[k].truncbeg)) <= *p_tmin) // 2024-05-11 fixed "<="
			*p_tmin = t;
		if(trace_timeset) BPPrintMessage(0,odInfo,"t = %ld\n",(long)t);
		// BPPrintMessage(0,odInfo,"nseq = %d tmin = %ld tmax = %ld\n",nseq,(long)*p_tmin,(long)*p_tmax);
		}
	else if(imax < 0 && trace_timeset) BPPrintMessage(0,odInfo,"\n=> imax = %ld for nseq = %ld\n",(long)imax,(long)nseq);
		
	/* Last object must not be played legato */ // Revised by BB 2022-03-02 
	if((PlayChunks && !LastChunk) || AllItems || Improvize) {
	// Suppressing last legato
	//	BPPrintMessage(0,odError,"\nSuppressing last legato\n");
		i = (*p_imaxseq)[nseq] - 1L;
		while((k=(*((*p_Seq)[nseq]))[i]) <= 0 || (*p_Instance)[k].object <= 1) {
			i--;
			if(i < ZERO) break;
			}
		if(i > ZERO) {
			a = (*p_articul)[k];
			if(a > 127) a = a - 256;
			if(a > 0 && k > 1) (*p_articul)[k] = 0;
			}
		}
	}

/* Modify Alpha according to articulation (legato/staccato) */
if(trace_timeset) BPPrintMessage(0,odInfo,"\nCalculating legato/staccato\n");
outtimeevents = TRUE;
for(k=2; k <= (*p_kmx); k++) {
	j = (*p_Instance)[k].object; if(j > 0) outtimeevents = FALSE;
	if((nature_time == STRIATED) && (*p_Instance)[k].starttime > max_end_time) {
		// This can happen with an out-time object at the beginning of an item
		if(0 && TraceMIDIinteraction) {  // Reactivate this to check -gr.koto3
			BPPrintMessage(0,odError,"=> Incorrect start/end values for object #%d (j = %d) in chunk #%d\n",k,(*p_Instance)[k].object,Chunk_number);
			BPPrintMessage(0,odError,"starttime = %ld endtime = %ld ms, max_end_time = %ld ms, kmx = %ld\n",(long)(*p_Instance)[k].starttime,(long)(*p_Instance)[k].endtime,(long)max_end_time,(long)*p_kmx);
			}
		if(outtimeevents) (*p_Instance)[k].starttime = (*p_Instance)[k].endtime = 0L;
		continue; // Well, needs to be checked
		// result = ABORT; goto EXIT1;
		}
	a = (*p_articul)[k];
	if(a == 0) continue;
	// if(a > 127) a = a - 256;
	if(a < -99) a = -99;
	
	/* Can't play legato if object's end was truncated */
	if(a > 0 && (*p_Instance)[k].truncend > ZERO) continue;
	(*p_Instance)[k].alpha += ((*p_Instance)[k].alpha * a) / 100.;
	(*p_Instance)[k].endtime = (*p_Instance)[k].starttime + ((*p_Instance)[k].endtime
		- (*p_Instance)[k].starttime) * (1. + ((double) a) / 100.);
		
//		BPPrintMessage(0,odInfo,"legato k = %ld starttime = %ld endtime = %ld  a = %ld\n",(long)k,(long)(*p_Instance)[k].starttime,(long)(*p_Instance)[k].endtime,(long)a);
		
	if((*p_Instance)[k].ncycles < 2)	/* Object is not cyclic */
		(*p_Instance)[k].dilationratio = (*p_Instance)[k].alpha;
	}
result = OK;

EXIT1:
MyDisposeHandle((Handle*)&p_DELTA);
MyDisposeHandle((Handle*)&p_marked);
MyDisposeHandle((Handle*)&p_time1);
MyDisposeHandle((Handle*)&p_time2);
MyDisposeHandle((Handle*)&p_maxcoverbeg);
MyDisposeHandle((Handle*)&p_maxcoverend);
MyDisposeHandle((Handle*)&p_maxgapbeg);
MyDisposeHandle((Handle*)&p_maxgapend);
MyDisposeHandle((Handle*)&p_maxtruncbeg);
MyDisposeHandle((Handle*)&p_maxtruncend);
MyDisposeHandle((Handle*)&p_alphadone);
if(EmergencyExit) result = ABORT;
/* if(trace_timeset) {
	// HideWindow(Window[wMessage]);
	// HideWindow(Window[wInfo]);
	} */
return(result);
}
