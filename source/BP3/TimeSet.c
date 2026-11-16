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

// Comments "// note N" in this file refer to the numbered notes of the technical description of this algorithm: https://bolprocessor.org/tech-analysis-of-timeset/

#ifndef DISPLAY_PHASE_DIAGRAM
#define DISPLAY_PHASE_DIAGRAM 0
#endif

#ifndef _H_BP3 
#include "-BP3.h"
#endif

#include "-BP3decl.h"

int trace_timeset = 0;
int trace_fix = 0;

#define LINE_RELEASE 2	// Constraint released for the whole sequence, as opposed to TRUE (1) for a single object (2026-11-15)

static void SetConstraintRelease(char**,unsigned long,int);
static void NameOfInstance(long,char*,size_t);

int TimeSet(tokenbyte ***pp_buff,long* p_kmx,long *p_tmin,long *p_tmax,unsigned long *p_maxseq,int* p_nmax,unsigned long **p_imaxseq,double maxseqapprox) {

	int i,result,bigitem,maxties,j,missed_ties;
	time_t start_time,end_time;

	if(CheckEmergency() != OK) return(ABORT);

	// note 1
	*p_tmin = Infpos; *p_tmax = Infneg;
	if(!Improvize) Chunk_number++;
	// BPPrintMessage(0,odError,"\nChunk_number = %d\n",Chunk_number);

	// 2026-11-14 All handles start as NULL, and a failed allocation goes to SORTIR, which frees those already allocated - fixed by Claude
	maxties = Jbol + Jpatt;
	for(i = 0; i < MAXINSTRUMENTS; i++) { p_Tie_event[i] = NULL; p_Missed_tie_event[i] = NULL; }
	for(i = 0; i <= MAXCHAN; i++) { p_Tie_note[i] = NULL; p_Missed_tie_note[i] = NULL; }

	result = ABORT;
	if((p_Articul = (short**) GiveSpace((Size)MaxObjects*sizeof(short))) == NULL) goto SORTIR;

	// note 2
	// BPPrintMessage(0,odInfo,"\n\n@@ maxties = %d\n\n",maxties);
	for(i = 0; i < MAXINSTRUMENTS; i++)
		if((p_Tie_event[i] = (char**) GiveSpace((Size)maxties*sizeof(short))) == NULL) goto SORTIR;
	for(i = 0; i <= MAXCHAN; i++)
		if((p_Tie_note[i] = (char**) GiveSpace((Size)128*sizeof(short))) == NULL) goto SORTIR;
	for(i = 0; i < MAXINSTRUMENTS; i++)
		if((p_Missed_tie_event[i] = (int**) GiveSpace((Size)maxties*sizeof(int))) == NULL) goto SORTIR;
	for(i = 0; i <= MAXCHAN; i++)
		if((p_Missed_tie_note[i] = (int**) GiveSpace((Size)128*sizeof(int))) == NULL) goto SORTIR;

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

	// note 5
	time(&start_time);
	ProductionTime += start_time - ProductionStartTime;

	// note 3
	result = MakeEmptyTokensSilent(pp_buff,&maxseqapprox);
	if(result != OK) goto SORTIR;

	if(Jbol + Jpatt > maxties) { // 2026-11-06 MakeEmptyTokensSilent() may have created new bols from variables - fixed by Claude
		int newmaxties = Jbol + Jpatt;
		for(j = 0; j < MAXINSTRUMENTS; j++) {
			// On failure, the handle keeps its former size, which is enough for reading maxties values in SORTIR
			if(MySetHandleSize((Handle*)&p_Tie_event[j],(Size)newmaxties*sizeof(short)) != OK) {
				result = ABORT; goto SORTIR;
				}
			if(MySetHandleSize((Handle*)&p_Missed_tie_event[j],(Size)newmaxties*sizeof(int)) != OK) {
				result = ABORT; goto SORTIR;
				}
			for(i = maxties; i < newmaxties; i++) {
				(*(p_Tie_event[j]))[i] = FALSE;
				(*(p_Missed_tie_event[j]))[i] = 0;
				}
			}
		maxties = newmaxties;
		}

	// note 4
	result = FillPhaseDiagram(pp_buff,p_kmx,p_maxseq,p_nmax,p_imaxseq,maxseqapprox,&bigitem);
	if(result != OK) goto SORTIR;

	// note 5
	time(&end_time);
	PhaseDiagramTime += end_time - start_time;

	start_time = end_time;
	result = SetTimeObjects(bigitem,p_imaxseq,*p_maxseq,p_nmax,p_kmx,p_tmin,p_tmax,p_Articul);
	time(&end_time);
	TimeSettingTime += end_time - start_time;
	// ProductionTime += TimeSettingTime;

	if(trace_timeset) BPPrintMessage(0,odInfo,"End TimeSet() maxseq = %ld\n\n",(long)*p_maxseq);

	SORTIR:
	MyDisposeHandle((Handle*)&p_Articul);

	// note 6
	missed_ties = 0;
	for(j = 0; j <= MAXCHAN; j++) {
		for(i = 0; i < 128 && p_Missed_tie_note[j] != NULL; i++) {
			if((*(p_Missed_tie_note[j]))[i] > 0) {
				PrintThisNote(-1,i,-1,-1,Message);
				BPPrintMessage(0,odError,"=> Missed tied note %s, %d occurrence(s) MIDI channel %d\n",Message,(*(p_Missed_tie_note[j]))[i],j); // 2026-11-12: j is the MIDI channel (1..16) - fixed by Claude
				missed_ties++;
				}
			}
		MyDisposeHandle((Handle*)&(p_Tie_note[j]));
		MyDisposeHandle((Handle*)&(p_Missed_tie_note[j]));
		}
	if(missed_ties > 0) {
		if(!PlayChunks)
			BPPrintMessage(0,odError,"=> Total %d note(s) with missed ties\n\n",missed_ties); // 2026-11-13: missed_ties counts notes, not occurrences - fixed by Claude
		else BPPrintMessage(0,odError,"=> Total %d note(s) with missed ties in chunk #%d\n\n",missed_ties,Chunk_number);
		}

	missed_ties = 0;
	for(j = 0; j < MAXINSTRUMENTS; j++) {
		for(i = 0; i < maxties && p_Missed_tie_event[j] != NULL; i++) {
			if((*(p_Missed_tie_event[j]))[i] > 0) {
				BPPrintMessage(0,odError,"=> Missed tied event #%d (%d occurrences Csound instrument %d)\n",i,(*(p_Missed_tie_event[j]))[i],j);
				missed_ties++;
				}
			}
		MyDisposeHandle((Handle*)&(p_Tie_event[j]));
		MyDisposeHandle((Handle*)&(p_Missed_tie_event[j]));
		}
	if(missed_ties > 0) BPPrintMessage(0,odError,"=> Total %d missed tied events in chunk #%d\n\n",missed_ties,Chunk_number);

	strcpy(Message,"");
	return(result);
	}


int SetTimeObjects(int bigitem,unsigned long **p_imaxseq,unsigned long maxseq,int *p_nmax,
	long *p_kmx,long *p_tmin,long *p_tmax,short **p_Articul) {

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
	char **p_coverok,**p_discontinuityok,name[MAXLIN];
	const char *what;
	long iconflict;
	Milliseconds shiftconflict;
	int linerelease;

	nature_time = Nature_of_time;	/* Must not change during computation */

	// note 7
	handle_size = maxseq + 50; // Added by BB 2021-03-23
	if(trace_timeset)
		BPPrintMessage(0,odInfo,"\nhandle_size for Seq[nseq] and DELTA = %ld\n",(long)handle_size);

	for(nseq=0; nseq <= (*p_nmax); nseq++) {
		ptr = (*p_Seq)[nseq];
		if(MySetHandleSize((Handle*)&ptr,(Size)handle_size * sizeof(long)) != OK) return(ABORT); 
		(*p_Seq)[nseq] = ptr;
		}

	// Calculate time structure
		
	// note 8
	in = 1.; jn = ZERO;
	period = ((double) Pclock) * 1000. * CorrectionFactor / Qclock;
	// BPPrintMessage(0,odInfo,"Pclock = %ld Qclock = %ld, CorrectionFactor = %.3f\n",(long)Pclock,(long)Qclock,CorrectionFactor);

	if(trace_timeset)
		BPPrintMessage(0,odInfo,"Setting time streaks on %d lines\n",(*p_nmax));

	while(TRUE) {
		jj= Class(in);
		if(jj > maxseq) break;
		if(jj > jn || in == 1.) {
			jn = jj;		// Write only once
			if(nature_time == STRIATED) {
				(*p_T)[jn] = (Milliseconds) ((period * (in - 1.)) / Ratio);
				// BPPrintMessage(0,odInfo,"jn = %ld (*p_T)[jn] = %ld in = %ld period = %.0f Ratio = %.0f\n",(long)jn,(long)(*p_T)[jn],(long)in,(double)period,(double)Ratio);
				if(Kpress > 2) { // Compensates roundings
					if(jn > 0) (*p_T)[jn-1] = (*p_T)[jn];
					if((jn + 1) <= maxseq) (*p_T)[jn+1] = (*p_T)[jn];
					}
				}
			else (*p_T)[jn] = ZERO;
			}
		in += Kpress;
		}
	if(trace_timeset)
		BPPrintMessage(0,odInfo,"End of setting time streaks\n");

// Now display phase diagram (optional)
// note 9
	#if DISPLAY_PHASE_DIAGRAM

	if(trace_timeset) {
		BPPrintMessage(0,odInfo,"\n");
		BPPrintMessage(0,odInfo,"@Minconc = %ld    Maxconc = %ld\n\n",(long)Minconc,(long)Maxconc);
		}

	for(k=ZERO; k < MaxObjects; k++) {
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
		
	if(MaxObjects < 1000) {
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
	// note 24
	max_end_time = (*p_T)[maxseq];
	if(trace_timeset) BPPrintMessage(0,odError,"\nmax_end_time = %ld\n",(long)max_end_time);

	if(trace_timeset)
		BPPrintMessage(0,odError,"\nPositioning %ld sound-objects...\n",(long)(*p_kmx));

	// if(DoSystem() != OK) return(ABORT);

	// note 10
	// 2026-11-14 All handles start as NULL, and a failed allocation goes to EXIT1, which frees those already allocated - fixed by Claude
	p_marked = p_alphadone = p_coverok = p_discontinuityok = NULL;
	p_DELTA = p_time1 = p_time2 = p_maxcoverbeg = p_maxcoverend = p_maxgapbeg = p_maxgapend
		= p_maxtruncbeg = p_maxtruncend = NULL;
	result = ABORT;
	if((p_marked = (char**) GiveSpace((Size)handle_size * sizeof(char))) == NULL)
		goto EXIT1;
	if((p_DELTA = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds)))
			== NULL) goto EXIT1;
	// for(i=ZERO; i <= maxseq; i++) {
	for(i=ZERO; i < handle_size; i++) { // Fixed by BB 2021-03-23
		(*p_DELTA)[i] = ZERO; (*p_marked)[i] = FALSE;
		}
	if((p_time1 = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
		goto EXIT1;
	if((p_time2 = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
		goto EXIT1;
	if((p_maxcoverbeg = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
		goto EXIT1;
	if((p_maxcoverend = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
		goto EXIT1;
	if((p_maxgapbeg = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
		goto EXIT1;
	if((p_maxgapend = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
		goto EXIT1;
	if((p_maxtruncbeg = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
		goto EXIT1;
	if((p_maxtruncend = (Milliseconds**) GiveSpace((Size)handle_size * sizeof(Milliseconds))) == NULL)
		goto EXIT1;

	if((p_alphadone = (char**) GiveSpace((Size)((*p_nmax)+1)*sizeof(char))) == NULL)
		goto EXIT1;
	if((p_coverok = (char**) GiveSpace((Size)handle_size * sizeof(char))) == NULL)
		goto EXIT1;
	if((p_discontinuityok = (char**) GiveSpace((Size)handle_size * sizeof(char))) == NULL)
		goto EXIT1;
	for(nseq=0; nseq <= (*p_nmax); nseq++) (*p_alphadone)[nseq] = FALSE;

	result = OK; first = TRUE;

	// if(IsMidiDriverOn()) tstart = GetDriverTime();

STARTAGAIN:
	// note 11
	for(nseq=0; nseq <= (*p_nmax); nseq++) {
		PleaseWait();
		if(trace_timeset) {
			if(Kpress > 1.) {
				if(Kpress < (double) ULONG_MAX)
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
		// note 12
		if(!(*p_alphadone)[nseq]) {
			if(trace_timeset) ShowMessage(FALSE,wMessage,"Sizing objects");
			if((result=Calculate_alpha(nseq,*p_nmax,maxseq,(*p_imaxseq)[nseq],nature_time,
				p_marked)) != OK) goto EXIT1;
			if(trace_timeset) ShowMessage(FALSE,wMessage,"Setting limits");
			// note 13
			if((result=SetLimits(nseq,p_maxcoverbeg,p_maxcoverend,p_maxgapbeg,p_maxgapend,
				p_maxtruncbeg,p_maxtruncend)) != OK) goto EXIT1;
			}
		(*p_alphadone)[nseq] = TRUE;

		if(DisplayTimeSet) {
			if(trace_timeset) 
				BPPrintMessage(0,odInfo,"\nSequence #%ld\n",(long)(nseq+1L));
			}
		// note 14
		// 2026-11-14 Constraints of overlapping and continuity are released per object (column), no longer for the whole sequence - by Claude
		stepthis = FALSE;
		linerelease = 0;
		SetConstraintRelease(p_coverok,handle_size,NoConstraint);
		SetConstraintRelease(p_discontinuityok,handle_size,NoConstraint);

TRY:
		if(DisplayTimeSet) Print(wTrace,"\nPlacing objects…\n");
		if(trace_fix) {
			BPPrintMessage(0,odInfo,"\nBefore FixObjectTimings(), T[i], i = 1,%ld:\n",(long)maxseq);
			for(i=1L; i <= maxseq; i++)
				BPPrintMessage(0,odInfo,"%ld ",(long)(*p_T)[i]);
			BPPrintMessage(0,odInfo,"%s","\n");
			BPPrintMessage(0,odInfo,"Before FixObjectTimings(), time2:\n");
			for(i=1L; i <= maxseq; i++)
				BPPrintMessage(0,odInfo,"[%ld]%ld ",(long)i,(long)(*p_time2)[i]);
			BPPrintMessage(0,odInfo,"%s","\n");
			} 
		// note 15
		if((result=FixObjectTimings(nseq,p_time1,p_time2,nature_time)) != OK) goto EXIT1;

		if(trace_fix) {
			BPPrintMessage(0,odInfo,"\nAfter FixObjectTimings(), T[i], i = 1,%ld:\n",(long)maxseq);
			for(i=1L; i <= maxseq; i++)
				BPPrintMessage(0,odInfo,"%ld ",(long)(*p_T)[i]);
			BPPrintMessage(0,odInfo,"%s","\n");
			BPPrintMessage(0,odInfo,"After FixObjectTimings(), time2:\n");
			for(i=1L; i <= maxseq; i++)
				BPPrintMessage(0,odInfo,"[%ld]%ld ",(long)i,(long)(*p_time2)[i]);
			BPPrintMessage(0,odInfo,"%s","\n");
			}
		
		// note 16
		if(trace_timeset) BPPrintMessage(0,odInfo,"Solving constraints Locate() for nseq = %ld\n",(long)nseq);
		r = Locate(nseq,p_imaxseq,maxseq,*p_kmx,p_DELTA,&tstart,p_time1,
				p_time2,p_maxcoverbeg,p_maxcoverend,p_maxgapbeg,p_maxgapend,p_maxtruncbeg,
				p_maxtruncend,nature_time,first,p_marked,p_coverok,p_discontinuityok,
				&iconflict,&shiftconflict);
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
		
		// note 17
		if(r == MISSED || r == QUICK) {
			if(r == QUICK) {	// Find a quick solution
				if(ShowMessages) ShowMessage(TRUE,wMessage,"Quick!");
				SetConstraintRelease(p_coverok,handle_size,TRUE);
				SetConstraintRelease(p_discontinuityok,handle_size,TRUE);
				goto TRY;
				}
			BPPrintMessage(0,odInfo,"👉  We must release time constraint(s):\n");
			if(TraceTimeSet) Print(wTrace,"\n➡ We must release time constraint(s)!\n");
			result = MISSED;
			// 2026-11-14 First release a constraint of the object on which Locate() failed: overlapping if it overlapped the preceding object(s), continuity if there was a gap, then both. Only if this does not help, release the constraints of the whole sequence, as was done before - by Claude
			if(linerelease == 0 && iconflict >= 0 && iconflict < (long) handle_size
					&& (k = (*((*p_Seq)[nseq]))[iconflict]) > 0) {
				what = NULL;
				if(shiftconflict > ZERO && !(*p_coverok)[iconflict]) {
					(*p_coverok)[iconflict] = TRUE;
					what = "overlapping";
					}
				else if(shiftconflict < ZERO && !(*p_discontinuityok)[iconflict]) {
					(*p_discontinuityok)[iconflict] = TRUE;
					what = "continuity";
					}
				else if(!(*p_coverok)[iconflict] || !(*p_discontinuityok)[iconflict]) {
					(*p_coverok)[iconflict] = (*p_discontinuityok)[iconflict] = TRUE;
					what = "continuity and overlapping";
					}
				if(what != NULL) {
					NameOfInstance(k,name,sizeof(name));
					BPPrintMessage(0,odInfo,"• Releasing %s before “%s” in sequence #%d\n",what,name,nseq+1);
					if(TraceTimeSet) {
						my_sprintf(Message,"• Releasing %s before “%s” in sequence #%d\n",what,name,nseq+1);
						Print(wTrace,Message);
						}
					goto TRY;
					}
				}
			// 2026-11-15 Whole sequence: overlapping, then continuity with overlapping required again, then both, as before 2026-11-14. Overlapping released for the whole sequence is marked LINE_RELEASE, so that it can be withdrawn without withdrawing the overlapping released for single objects - by Claude
			linerelease++;
			if(linerelease == 1) {
				BPPrintMessage(0,odInfo,"• Releasing overlapping in sequence #%d\n",nseq+1);
				if(TraceTimeSet) Print(wTrace,"• Releasing overlapping in the whole sequence\n");
				for(i = 0; i < (long) handle_size; i++)
					if(!(*p_coverok)[i]) (*p_coverok)[i] = LINE_RELEASE;
				goto TRY;
				}
			if(linerelease == 2) {
				BPPrintMessage(0,odInfo,"• Releasing continuity in sequence #%d\n",nseq+1);
				if(TraceTimeSet) Print(wTrace,"• Releasing continuity in the whole sequence\n");
				for(i = 0; i < (long) handle_size; i++)
					if((*p_coverok)[i] == LINE_RELEASE) (*p_coverok)[i] = FALSE;
				SetConstraintRelease(p_discontinuityok,handle_size,TRUE);
				goto TRY;
				}
			if(linerelease == 3) {
				BPPrintMessage(0,odInfo,"• Releasing continuity and overlapping in sequence #%d\n",nseq+1);
				if(TraceTimeSet) Print(wTrace,"• Releasing continuity and overlapping in the whole sequence\n");
				SetConstraintRelease(p_coverok,handle_size,TRUE);
				goto TRY;
				}
			goto EXIT1;
		// note 18
		/*	if(TraceTimeSet) {
				Print(wTrace,"\nRelease:\n\n");
				if(!DiscontinuityOK) Print(wTrace,"Continuity\n");
				if(!CoverOK) Print(wTrace,"Overlapping\n");
				if(!CoverOK || !DiscontinuityOK)
					Print(wTrace,"Both continuity and overlapping\n");
				if(!StepTimeSet && !stepthis)
					Print(wTrace,"S)tep by step decision\n\n");
				} */
			
QUEST1:
			rep = GetCap();
			if(rep == 'Q') {
				result = ABORT;
				goto EXIT1;
				}
			if(rep == 'C') {
				SetConstraintRelease(p_discontinuityok,handle_size,TRUE);
				SetConstraintRelease(p_coverok,handle_size,FALSE);
				}
			else
				if(rep == 'O') {
					SetConstraintRelease(p_coverok,handle_size,TRUE);
					SetConstraintRelease(p_discontinuityok,handle_size,FALSE);
					}
				else
					if(rep == 'B') {
						SetConstraintRelease(p_coverok,handle_size,TRUE);
						SetConstraintRelease(p_discontinuityok,handle_size,TRUE);
						}
					else
						if(rep == 'S') {
							SetConstraintRelease(p_coverok,handle_size,FALSE);
							SetConstraintRelease(p_discontinuityok,handle_size,FALSE);
							stepthis = TRUE;
							}
						else {
							SysBeep(10);
							goto QUEST1;
							}
			goto TRY;
			}
			
		// note 18
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

		// note 19
		BTflag = FALSE;
		tmax = Infneg;
		olddelta = ZERO;
		if(nature_time == STRIATED) {
			if(trace_timeset) BPPrintMessage(0,odInfo,"Calculating global drift\n");
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
			// note 20
			if(trace_timeset) BPPrintMessage(0,odInfo,"Interpolating streaks\n");
			(*p_T)[0] = ZERO;
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

		// note 19
		if(BTflag) {
			if(trace_timeset)
				BPPrintMessage(0,odInfo,"Starting again algorithm (broke tempo)...\n");
			if(DisplayTimeSet) {
				BPPrintMessage(0,odInfo,"\nBroke tempo. T[i], i = 1,maxseq:\n");
				for(i=1; i <= maxseq; i++) {
					BPPrintMessage(0,odInfo,"%ld ",(long)(*p_T)[i]);
					}
				BPPrintMessage(0,odInfo,"\n");
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

		// note 21
		/* Update tmin and tmax */
		imax = (*p_imaxseq)[nseq] - 1L;
		if(trace_timeset) {
			BPPrintMessage(0,odInfo,"\nimax = %ld\n",(long)imax);
			for(i = 0; i <= imax; i++) {
		//		BPPrintMessage(0,odInfo,"i = %ld, k = %ld\n",(long)i,(long)(*((*p_Seq)[nseq]))[i]);
				}
			}
		if(imax >= 0) {  // 2024-05-11 fixed ">=" this condition is probably useless
			if(trace_timeset) BPPrintMessage(0,odInfo,"\nUpdating tmin and tmax, nseq = %ld, imax = %ld, kmx = %ld\n",(long)nseq,(long)imax,(long)*p_kmx);
			// 2026-11-15 'tmax' is the latest end of all objects of the line, not the end of its last object, which may end
			// earlier than another object of the same line, e.g. one with a long fade-out after its pivot (item #83 of
			// -da.tryMiscObjects). The ruler of the graphic and 'ChunkEndTime' (MakeSound()) were too short - fixed by Claude
			for(i = 1; i <= imax; i++) {
				k = (*((*p_Seq)[nseq]))[i];
				if(k < 2) continue;
				if(k > *p_kmx) {
					BPPrintMessage(0,odError,"=> Error in TimeSet(). k > *p_kmx\n");
					continue;
					}
				if((t = ((*p_Instance)[k].endtime + (*p_Instance)[k].truncend)) >= *p_tmax) // 2024-05-11 fixed ">="
					*p_tmax = t;
				}
			if(trace_timeset) BPPrintMessage(0,odInfo,"tmax = %ld\n",(long)*p_tmax);
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
			
		// note 22
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
				a = (*p_Articul)[k];
				if(a > 127) a = a - 256;
				if(a > 0 && k > 1) (*p_Articul)[k] = 0;
				}
			}
		}

	// note 23
	// Modify Alpha according to articulation (legato/staccato)
	if(trace_timeset) BPPrintMessage(0,odInfo,"\nCalculating legato/staccato\n");
	outtimeevents = TRUE;
	for(k=2; k <= (*p_kmx); k++) {
		j = (*p_Instance)[k].object;
		if(j > 0) outtimeevents = FALSE;
		// note 24
		if((nature_time == STRIATED) && (*p_Instance)[k].starttime > max_end_time) {
			// This can happen with an out-time object at the beginning of an item
			if(FALSE && TraceMIDIinteraction) {  // Reactivate this to check -gr.koto3
				BPPrintMessage(0,odError,"=> Incorrect start/end values for object #%d (j = %d) in chunk #%d\n",k,(*p_Instance)[k].object,Chunk_number);
				BPPrintMessage(0,odError,"starttime = %ld endtime = %ld ms, max_end_time = %ld ms, kmx = %ld\n",(long)(*p_Instance)[k].starttime,(long)(*p_Instance)[k].endtime,(long)max_end_time,(long)*p_kmx);
				}
			if(outtimeevents) (*p_Instance)[k].starttime = (*p_Instance)[k].endtime = 0L;
			continue; // Well, needs to be checked
			// result = ABORT; goto EXIT1;
			}
		a = (*p_Articul)[k];
		if(a == 0) continue;
		if(a < -99) a = -99;
		
		// Can't play legato if object's end was truncated
		if(a > 0 && (*p_Instance)[k].truncend > ZERO) continue;
		(*p_Instance)[k].alpha += ((*p_Instance)[k].alpha * a) / 100.;
		(*p_Instance)[k].endtime = (*p_Instance)[k].starttime + ((*p_Instance)[k].endtime
			- (*p_Instance)[k].starttime) * (1. + ((double) a) / 100.);
		// 2026-11-15 A legato object may now end after 'tmax' (note 21) - fixed by Claude
		if((*p_Instance)[k].endtime + (*p_Instance)[k].truncend > *p_tmax)
			*p_tmax = (*p_Instance)[k].endtime + (*p_Instance)[k].truncend;
		// BPPrintMessage(0,odInfo,"@@ legato k = %ld starttime = %ld endtime = %ld  a = %ld\n",(long)k,(long)(*p_Instance)[k].starttime,(long)(*p_Instance)[k].endtime,(long)a);	
		if((*p_Instance)[k].ncycles < 2)	// Object is not cyclic
			(*p_Instance)[k].beta = (*p_Instance)[k].alpha;
		}
	result = OK;

	EXIT1:
	// note 25
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
	MyDisposeHandle((Handle*)&p_coverok);
	MyDisposeHandle((Handle*)&p_discontinuityok);
	if(EmergencyExit) result = ABORT;
	/* if(trace_timeset) {
		// HideWindow(Window[wMessage]);
		// HideWindow(Window[wInfo]);
		} */
	return(result);
	}

// note 18
char GetCap() {
	return 'Q';
	}

// note 14
static void SetConstraintRelease(char **p_release,unsigned long size,int value) {
	// Set the release of a constraint (overlapping or continuity) for all columns of a sequence. 2026-11-14 by Claude
	unsigned long i;
	for(i = 0; i < size; i++) (*p_release)[i] = value;
	}

// note 17
static void NameOfInstance(long k,char *name,size_t size) {
	// Name of the object of instance k, for messages. 2026-11-14 by Claude
	int j;
	char note[MAXLIN];
	j = (*p_Instance)[k].object;
	if(j < 0) j = -j;
	if(j >= 16384) {
		PrintThisNote(-1,j - 16384,-1,-1,note);
		snprintf(name,size,"%s",note);
		}
	else if(j >= Jbol) snprintf(name,size,"%s",*((*p_Patt)[j-Jbol]));
	else snprintf(name,size,"%s",*((*p_Bol)[j]));
	}
