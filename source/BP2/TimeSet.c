/* TimeSet.c (BP2 version CVS) */ 

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


#ifndef DISPLAY_PHASE_DIAGRAM
#define DISPLAY_PHASE_DIAGRAM 0
#endif

#ifndef _H_BP2 
#include "-BP2.h"
#endif

#include "-BP2decl.h"


TimeSet(tokenbyte ***pp_buff,int* p_kmx,long *p_tmin,long *p_tmax,unsigned long *p_maxseq,
	int* p_nmax,unsigned long **p_imaxseq,double maxseqapprox)
{
int result,bigitem;
short **p_articul;

HideWindow(Window[wInfo]);
if(CheckEmergency() != OK) return(ABORT);

*p_tmin = Infpos; *p_tmax = Infneg;

if((p_articul = (short**) GiveSpace((Size)Maxevent*sizeof(short))) == NULL) return(ABORT);

result = FillPhaseDiagram(pp_buff,p_kmx,p_maxseq,p_nmax,p_imaxseq,
	maxseqapprox,&bigitem,p_articul);
	
if(result != OK) goto OUT;

result = SetTimeObjects(bigitem,p_imaxseq,*p_maxseq,p_nmax,
	p_kmx,p_tmin,p_tmax,p_articul);
	
OUT:
MyDisposeHandle((Handle*)&p_articul);
return(result);
}


SetTimeObjects(int bigitem,unsigned long **p_imaxseq,unsigned long maxseq,int *p_nmax,
	int *p_kmx,long *p_tmin,long *p_tmax,short **p_articul)

{
int nseq,r,rep,BTflag,result,stepthis,first,dirtymem,compiledmem,nature_time,a,j,key;
short **ptr;
long k;
long i,iseq;
unsigned long jj,jn;
char **p_marked,**p_alphadone;
OSErr oserr;
double period,in,imax;
p_list **ptag;
Milliseconds tmax,t,**p_DELTA,t1,t2,**p_time1,**p_time2,**p_maxcoverbeg,**p_maxcoverend,
	**p_maxgapbeg,**p_maxgapend,**p_maxtruncbeg,**p_maxtruncend,
	olddelta;
unsigned long tstart;

nature_time = Nature_of_time;	/* Must not change during computation */
for(nseq=0; nseq <= (*p_nmax); nseq++) {
	ptr = (*p_Seq)[nseq];
	if(MySetHandleSize((Handle*)&ptr,(Size)(maxseq+2) * sizeof(short)) != OK) return(ABORT); 
	(*p_Seq)[nseq] = ptr;
	}

// Calculate time structure

if(ShowMessages || bigitem) {
	HideWindow(Window[wInfo]);
	ShowMessage(TRUE,wMessage,"Setting time streaks...");
	}
	
in = 1.; jn = ZERO;
period = ((double) Pclock) * 1000. * CorrectionFactor / Qclock;
k = 0;
while(TRUE) {
	jj= Class(in);
	if(jj > maxseq) break;
	if(jj > jn || in == 1.) {
		jn = jj;		/* Write only once */
		if(++k > 50) {
			PleaseWait();
			k = 0;
			}
		if(nature_time == STRIATED) {
			(*p_T)[jn] = (Milliseconds) ((period * (in - 1.)) / Ratio);
			}
		else (*p_T)[jn] = ZERO;
		}
	in += Kpress;
	}

// Now display phase diagram (optional)

#if DISPLAY_PHASE_DIAGRAM

Print(wTrace,"\n");
sprintf(Message,"Minconc = %ld    Maxconc = %ld",Minconc,Maxconc);
Println(wTrace,Message);
while(Button());
for(k=ZERO; k < Maxevent; k++) {
	if(Button()) break;
	if((j=(*p_Instance)[k].object) >= 0) {
		if(j >= Jbol) {
			if(j < 16384)
				sprintf(Message,"(Pattern %s)",*((*p_Patt)[j-Jbol]));
			else {
				key = j - 16384;
				key = MapThisKey(key,0.,(*p_Instance)[k].mapmode,
					&((*p_Instance)[k].map0),&((*p_Instance)[k].map1));
				PrintNote(key,0,-1,LineBuff);
				sprintf(Message,"(%s chan %ld)",
					LineBuff,(long)ChannelConvert((*p_Instance)[k].channel));
				}
			}
		else sprintf(Message,"(%s chan %ld)",*((*p_Bol)[j]),(long)ChannelConvert((*p_Instance)[k].channel));
		}
	else {
		j = -j;
		if(j >= Jbol) {
			if(j < 16384)
				sprintf(Message,"(<<Pattern %s>>)",*((*p_Patt)[j-Jbol]));
			else {
				key = j - 16384;
				key = MapThisKey(key,0.,(*p_Instance)[k].mapmode,
					&((*p_Instance)[k].map0),&((*p_Instance)[k].map1));
				PrintNote(key,0,-1,LineBuff);
				sprintf(Message,"(<<%s>> chan %ld)",
					LineBuff,(long)ChannelConvert((*p_Instance)[k].channel));
				}
			}
		else sprintf(Message,"(<<%s>> chan %ld)",*((*p_Bol)[j]),(long)ChannelConvert((*p_Instance)[k].channel));;
		}
	Print(wTrace,Message);
	}
Print(wTrace,"\n");
for(nseq=0; nseq <= (*p_nmax); nseq++) {
	if(Button()) break;
	for(iseq=1L; iseq <= (*p_imaxseq)[nseq]; iseq++) {
		if(Button()) break;
		k = (*((*p_Seq)[nseq]))[iseq];
		if(k >= 0) {
			if((*p_ObjectSpecs)[k] != NULL) {
				ptag = WaitList(k);
				while(ptag != NULL) {
					sprintf(Message,"<<W%ld>>",(long)((**ptag).x));
					Print(wTrace,Message);
					ptag = (**ptag).p;
					}
				}
			j = (*p_Instance)[k].object;
			if(j >= 0) {
				if(j >= Jbol) {
					if(j < 16384)
						sprintf(Message,"%s ",*((*p_Patt)[j-Jbol]));
					else {
						key = j - 16384;
						key = MapThisKey(key,0.,(*p_Instance)[k].mapmode,
							&((*p_Instance)[k].map0),&((*p_Instance)[k].map1));
						PrintNote(key,0,-1,LineBuff);
						sprintf(Message,"%s ",LineBuff);
						}
					}
				else {
					if(k > 1 && j == 1)
					sprintf(Message,"~ ");
					else sprintf(Message,"%s ",*((*p_Bol)[j]));
					}
				}
			else {
				j = -j;
				if(k > 1 && j == 1)
					sprintf(Message,"<<~>> ");
				else {
					if(j < 16384)
						sprintf(Message,"<<%s>> ",*((*p_Bol)[j]));
					else {
						key = j - 16384;
						key = MapThisKey(key,0.,(*p_Instance)[k].mapmode,
							&((*p_Instance)[k].map0),&((*p_Instance)[k].map1));
						PrintNote(key,0,-1,LineBuff);
						sprintf(Message,"<<%s>> ",LineBuff);
						}
					}
				}
			}
		else sprintf(Message,"%ld ",(long)k);
		Print(wTrace,Message);
		}
	Print(wTrace,"\n");
	}
ShowSelect(CENTRE,wTrace);
sprintf(Message,"\nT[i], i = 1,%ul:\n",(unsigned long)maxseq);
Print(wTrace,Message);
for(i=1L; i <= maxseq; i++) {
	if(Button()) break;
	sprintf(Message,"%ld ",(long)(*p_T)[i]);
	Print(wTrace,Message);
	}
Print(wTrace,"\n");
ShowSelect(CENTRE,wTrace);
for(nseq=0; nseq <= (*p_nmax); nseq++) {
	if(Button()) break;
	for(iseq=1L; iseq <= (*p_imaxseq)[nseq]; iseq++) {
		if(Button()) break;
		k = (*((*p_Seq)[nseq]))[iseq];
		if(k == -1) break;
		if(k >= 0) {
			if((*p_ObjectSpecs)[k] != NULL) {
				ptag = WaitList(k);
				while(ptag != NULL) {
					ptag = (**ptag).p;
					}
				}
			j = (*p_Instance)[k].object;
			if(j >= Jbol+Jpatt && j < 16384) {
				Print(wTrace,"\nERR: j >= Jbol+Jpatt\n");
				return(ABORT);
				}
			}
		else {
			sprintf(Message,"nseq=%ld i=%ld im=%ul k=%ld",(long)nseq,(long)i,
				(unsigned long)maxseq,(long)k);
			if(Beta) Alert1(Message);
			}
		}
	}
#endif
// End display

if(ShowMessages || bigitem) {
	sprintf(Message,"Positioning %ld sound-objects...",(long)(*p_kmx));
	ShowMessage(TRUE,wMessage,Message);
	}
if(DoSystem() != OK) return(ABORT);

if((p_marked = (char**) GiveSpace((Size) (maxseq+4)*sizeof(char))) == NULL)
	return(ABORT);
if((p_DELTA = (Milliseconds**) GiveSpace((Size)(maxseq+4)*sizeof(Milliseconds)))
		== NULL) return(ABORT);
for(i=ZERO; i <= maxseq; i++) {
	(*p_DELTA)[i] = ZERO; (*p_marked)[i] = FALSE;
	}
if((p_time1 = (Milliseconds**) GiveSpace((Size)(maxseq+4)*sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_time2 = (Milliseconds**) GiveSpace((Size)(maxseq+4)*sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_maxcoverbeg = (Milliseconds**) GiveSpace((Size)(maxseq+4)*sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_maxcoverend = (Milliseconds**) GiveSpace((Size)(maxseq+4)*sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_maxgapbeg = (Milliseconds**) GiveSpace((Size)(maxseq+4)*sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_maxgapend = (Milliseconds**) GiveSpace((Size)(maxseq+4)*sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_maxtruncbeg = (Milliseconds**) GiveSpace((Size)(maxseq+4)*sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_maxtruncend = (Milliseconds**) GiveSpace((Size)(maxseq+4)*sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_alphadone = (char**) GiveSpace((Size)((*p_nmax)+1)*sizeof(char))) == NULL)
	return(ABORT);
	
result = OK; first = TRUE;

if(IsMidiDriverOn()) tstart = GetDriverTime();

for(nseq=0; nseq <= (*p_nmax); nseq++) (*p_alphadone)[nseq] = FALSE;

STARTAGAIN:
for(nseq=0; nseq <= (*p_nmax); nseq++) {
	PleaseWait();
	if(ShowMessages && bigitem) {
		if(Kpress > 1.) {
			if(Kpress < ULONG_MAX)
				sprintf(Message,"%ld objects. Compression rate = %u  Sequence %ld/%ld",
					(long)(*p_kmx),(unsigned long)Kpress,((long)nseq)+1L,((long)(*p_nmax))+1L);
			else
				sprintf(Message,"%ld objects. Compression rate = %.0f  Sequence %ld/%ld",
					(long)(*p_kmx),Kpress,((long)nseq)+1L,((long)(*p_nmax))+1L);
			}
		else
			sprintf(Message,"%ld objects. No compression.  Sequence %ld/%ld",
				(long)(*p_kmx),(long)nseq+1,(long)(*p_nmax)+1L);
		FlashInfo(Message);
		}
	if(!(*p_alphadone)[nseq]) {
		if(ShowMessages && bigitem) ShowMessage(FALSE,wMessage,"Sizing objects");
		if((result=Calculate_alpha(nseq,*p_nmax,maxseq,(*p_imaxseq)[nseq],nature_time,
			p_marked)) != OK) goto EXIT1;
		if(ShowMessages && bigitem) ShowMessage(FALSE,wMessage,"Setting limits");
		if((result=SetLimits(nseq,p_maxcoverbeg,p_maxcoverend,p_maxgapbeg,p_maxgapend,
			p_maxtruncbeg,p_maxtruncend)) != OK) goto EXIT1;
		}
	(*p_alphadone)[nseq] = TRUE;

#if BP_CARBON_GUI
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	// This block is very similar to the middle of InterruptTimeSet().  Could we
	// refactor the shared code into a function ?? -- akozar, 20130830
	if((r=MyButton(1)) != FAILED) {
		StopCount(0);
		SetButtons(TRUE);
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
		}
#endif /* BP_CARBON_GUI */

	if(DisplayTimeSet) {
		sprintf(Message,"\nSequence #%ld\n",(long)(nseq+1L));
		Print(wTrace,Message);
		}
	CoverOK = DiscontinuityOK = stepthis = FALSE;
	if(NoConstraint) {
		CoverOK = DiscontinuityOK = TRUE;
		}
		
TRY:
	if(ShowMessages && bigitem) ShowMessage(FALSE,wMessage,"Placing objects");
	if((result=Fix(nseq,p_time1,p_time2,nature_time)) != OK) goto EXIT1;
	if(ShowMessages && bigitem) ShowMessage(FALSE,wMessage,"Solving constraints");
	r = Locate(nseq,p_imaxseq,maxseq,*p_kmx,p_DELTA,&tstart,p_time1,
			p_time2,p_maxcoverbeg,p_maxcoverend,p_maxgapbeg,p_maxgapend,p_maxtruncbeg,
			p_maxtruncend,nature_time,first,p_marked);
	if(r == ABORT || r == ENDREPEAT || r == AGAIN || r == EXIT) {
		result = r;
		goto EXIT1;
		}
	if(r == FAILED || r == QUICK) {
		result = FAILED;
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
		if(ShowMessages && bigitem) ShowMessage(FALSE,wMessage,"Calculating global drift");
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
		if(ShowMessages && bigitem) ShowMessage(FALSE,wMessage,"Interpolating streaks");
		for(i=1L; i <= maxseq; i++) {
			if(i <= (*p_imaxseq)[nseq] && ((k=(*((*p_Seq)[nseq]))[i]) >= 1 || k == -1))
						(*p_marked)[i] = TRUE;
			if((*p_marked)[i] == TRUE) {
				(*p_T)[i] = (*p_T)[i] + (*p_DELTA)[i];
				if((*p_T)[i] >= tmax) tmax = (*p_T)[i];
				else {	/* Must be increasing.  Sometimes local errors… */
						/* …in smooth time */
					(*p_T)[i] = tmax; /* This correction is very small */
					}
				}
			else {
				if(i <= (*p_imaxseq)[nseq] && (*p_T)[i] != ZERO) {
					if(Beta) {
						sprintf(Message,"Err. SetTimeObjects() nseq = %ld maxseq = %ul (*p_T)[%ld] = %ld",
							(long)nseq,(unsigned long)maxseq,(long)i,(long)(*p_T)[i]);
						Println(wTrace,Message);
						}
					(*p_T)[i] = ZERO;
					}
				}
			(*p_DELTA)[i] = ZERO;
			}
		}
	if(BTflag) {
		if(ShowMessages && bigitem)
			ShowMessage(TRUE,wMessage,"Starting again algorithm (broke tempo)...");
		if(DisplayTimeSet) {
			sprintf(Message,"\nBroke tempo. T[i], i = 1,maxseq:\n");
			Print(wTrace,Message);
			for(i=1; i <= maxseq; i++) {
				sprintf(Message,"%ld ",(long)(*p_T)[i]);
				Print(wTrace,Message);
				}
			}
		goto STARTAGAIN;
		}
	/* Update tmin and tmax */
	imax = (*p_imaxseq)[nseq] - 1L;
	if(imax > 0) {
		i = imax;
		while(i > 0. && (k=(*((*p_Seq)[nseq]))[i]) < 2) i--;
		if(k > *p_kmx) {
			if(Beta) Alert1("Err. TimeSet(). k > *p_kmx");
			}
		else if(i > 0. && (t=((*p_Instance)[k].endtime+(*p_Instance)[k].truncend)) > *p_tmax)
			*p_tmax = t;
		i = 1L;
		while(i <= imax && (k=(*((*p_Seq)[nseq]))[i]) < 2) i++;
		if(k > *p_kmx) {
			if(Beta) Alert1("Err. TimeSet(). k > *p_kmx");
			}
		else if(i <= imax && (t=((*p_Instance)[k].starttime-(*p_Instance)[k].truncbeg)) < *p_tmin)
			*p_tmin = t;
		}
		
	/* Last object must not be played legato */
	i = (*p_imaxseq)[nseq] - 1L;
	while((k=(*((*p_Seq)[nseq]))[i]) <= 0 || (*p_Instance)[k].object <= 1) {
		i--;
		if(i < 1L) break;
		}
	a = (*p_articul)[k];
	if(a > 127) a = a - 256;
	if(a > 0 && k > 1) (*p_articul)[k] = 0;
	}

/* Modify Alpha according to articulation (legato/staccato) */
if(ShowMessages && bigitem) ShowMessage(TRUE,wMessage,"Calculating legato/staccato");
for(k=2; k <= *p_kmx; k++) {
	a = (*p_articul)[k];
	if(a == 0) continue;
	if(a > 127) a = a - 256;
	if(a < -99) a = -99;
	
	/* Can't play legato if object's end was truncated */
	if(a > 0 && (*p_Instance)[k].truncend > ZERO) continue;
	(*p_Instance)[k].alpha += ((*p_Instance)[k].alpha * a) / 100.;
	(*p_Instance)[k].endtime = (*p_Instance)[k].starttime + ((*p_Instance)[k].endtime
		- (*p_Instance)[k].starttime) * (1. + ((double) a) / 100.);
	if((*p_Instance)[k].ncycles < 2)	/* Object is not cyclic */ /* 14/4/98 */
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
if(ShowMessages || bigitem) {
	HideWindow(Window[wMessage]);
	HideWindow(Window[wInfo]);
	}
return(result);
}
