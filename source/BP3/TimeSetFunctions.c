/* TimeSetFunctions.c (BP3) */

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

char Possible_choices(solset,char,int,int,int,long,int,char**,
	int,Milliseconds,Milliseconds,Milliseconds,Milliseconds,Milliseconds,Milliseconds,int);
int Situation_ok(int,int,int,int,Milliseconds,Milliseconds,Milliseconds,Milliseconds,Milliseconds,Milliseconds,
	Milliseconds,Milliseconds,char**,int);
int Get_choice(solset,p_list2***,p_list2***,int*,int*,int,int,int,Milliseconds,
										Milliseconds,Milliseconds,Milliseconds);
int Next_choice(solset,int,int,int,int,Milliseconds,Milliseconds,Milliseconds,Milliseconds,int);
int Store(int,char,int,int*,p_list2***,p_list2***,int);
int Erase_stack(int,int*,p_list2**);
int Solution_is_accepted(int,int,unsigned long**,long,Milliseconds**,Milliseconds**,Milliseconds**,Milliseconds**,
	Milliseconds**,Milliseconds**);
Milliseconds Alternate_correction1(int,int,int,Milliseconds,Milliseconds**,Milliseconds,Milliseconds,
	Milliseconds,Milliseconds,Milliseconds,Milliseconds);


int Locate(int nseq,unsigned long** p_imaxseq,long imax,long kmax,Milliseconds **p_DELTA,
	unsigned long* p_tstart,Milliseconds **p_time1,Milliseconds **p_time2,
	Milliseconds **p_maxcoverbeg,
	Milliseconds **p_maxcoverend,Milliseconds **p_maxgapbeg,Milliseconds **p_maxgapend,
	Milliseconds **p_maxtruncbeg,Milliseconds **p_maxtruncend,
	int nature_time,int first,char **p_marked)
{
char **p_BreakTempoPrev,**p_choice1,choice2;
int j,sol1,sol2,n,nsol,redo,iii,
	krep,r,result,stack_depth,okmove;
long i,ii,i0,iprev,inext,ibreak,imax2,k,kk,**p_tp1,**p_tp2,**p_ts1,**p_ts2,ts1mem,ts2mem,
	imaxseq,imaxseq2,delta2mem,ddelta2mem,**p_delta,**p_delta1,**p_delta2,
	shift1,shift2,shift3,shift4,DELTA,**p_ddelta0,**p_ddelta1,**p_ddelta2;
long **p_Ts,**p_tscover,**p_tsgap,Tsm;
p_list2 **s0,**s,**olds,**ss;
solset **p_sol_set1,sol_set2,**ptr;
Milliseconds maxmove,maxcover1,maxcover2;

if(nseq >= Maxconc) {
	if(Beta) Alert1("=> Err. Locate(). nseq >= Maxconc");
	return(OK);
	}	
imaxseq = (*p_imaxseq)[nseq];
// imaxseq2 = imaxseq + 2; Fixed by BB 2021-03-23
imaxseq2 = imaxseq + 20;
//imax2 = imax + 1; Fixed by BB 2021-03-23
imax2 = imax + 20;
n = 0;

/* All these below should be replaced with single records !!! */

if((p_BreakTempoPrev = (char**) GiveSpace((Size)imaxseq2 * sizeof(char))) == NULL)
	return(ABORT);
if((p_choice1 = (char**) GiveSpace((Size)imaxseq2 * sizeof(char))) == NULL)
	return(ABORT);
if((p_tp1 = (Milliseconds**) GiveSpace((Size)imaxseq2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_tp2 = (Milliseconds**) GiveSpace((Size)imaxseq2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_ts1 = (Milliseconds**) GiveSpace((Size)imax2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_ts2 = (Milliseconds**) GiveSpace((Size)imax2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_delta = (Milliseconds**) GiveSpace((Size)Maxevent * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_delta1 = (Milliseconds**) GiveSpace((Size)imax2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_delta2 = (Milliseconds**) GiveSpace((Size)imax2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_Ts = (Milliseconds**) GiveSpace((Size)imaxseq2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_tscover = (Milliseconds**) GiveSpace((Size)imaxseq2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_tsgap = (Milliseconds**) GiveSpace((Size)imaxseq2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_ddelta0 = (Milliseconds**) GiveSpace((Size)imax2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_ddelta1 = (Milliseconds**) GiveSpace((Size)imax2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_ddelta2 = (Milliseconds**) GiveSpace((Size)imax2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_sol_set1 = (solset**) GiveSpace((Size)imaxseq2 * sizeof(solset)))
	== NULL) return(ABORT);
	
redo = FALSE; nsol = 0; stack_depth = 0; s = s0 = NULL;

START:
if(kmax > 100 && ++n > 10) {
	PleaseWait(); n = 0;
	if((r=InterruptTimeSet(TRUE,p_tstart)) != OK){
		result = r;
		goto QUIT;
		}
	}
/* if((r=DoSystem()) != OK) {
	result = r;
	goto QUIT;
	} */
olds = NULL;
if(!redo) {
	stack_depth = 0;
	s0 = NULL;
	}
else s = s0;
iprev = i0 = ibreak = 0;
// for(i=1; i <= imax; i++) { // Fixed by BB 2021-03-20
for(i=ZERO; i <= imax; i++) {
	(*p_ddelta0)[i] = (*p_ddelta1)[i] = (*p_ddelta2)[i]
		= (*p_delta1)[i] = (*p_delta2)[i] = ZERO;
	k = (*((*p_Seq)[nseq]))[i];
	if(k > 1 || k < 0) { /* Ignore silence "-" */
		/* k < 0 if empty sequence */
		i0 = i;
		// BPPrintMessage(0,odError,"nseq = %ld i0 = %ld k = %ld\n",(long)nseq,(long)i0,(long)k);
		break;
		}
	iprev = i;
	}

i = i0;
(*p_BreakTempoPrev)[i] = TRUE;
(*p_Ts)[i] = (*p_tscover)[i] = Infneg;
(*p_tsgap)[i] = Infpos;
(*p_ts1)[0] = (*p_ts2)[0] = (*p_tp1)[0] = (*p_tp2)[0] = (*p_delta)[0] = (*p_delta1)[0] = (*p_delta2)[0] = ZERO; // Added by BB 2021-03-20
goto CHECK;
	
INCREMENT:
/* if(kmax > 100 && ++n > 10) {
	PleaseWait(); n = 0;
//	if(((r=DoSystem()) != OK) || (r=InterruptTimeSet(TRUE,p_tstart)) != OK) {
	if((r=InterruptTimeSet(TRUE,p_tstart)) != OK) {
		result = r;
		goto QUIT;
		}
	} */
if(DisplayTimeSet) {
	Print(wTrace,">");
	}
inext = i; while((*((*p_Seq)[nseq]))[++inext] == 0);
if(nature_time == STRIATED)
	(*p_ddelta0)[i+1] = (*p_ddelta0)[i] + (*p_ddelta1)[i] + (*p_ddelta2)[i];
else (*p_ddelta0)[i+1] = ZERO;
for(ii=i+1; ii <= inext; ii++) {
	if(nature_time == STRIATED) (*p_ddelta0)[ii] = (*p_ddelta0)[i+1];
	else (*p_ddelta0)[ii] = ZERO;
	(*p_ddelta1)[ii] = (*p_ddelta2)[ii] = ZERO;
	}
	
/* Update tscover */
(*p_tscover)[inext] = (*p_tscover)[i];
if(j < 16384) {	/* Not a simple note */
	if(((*p_maxcoverend)[i] != Infpos
					&& (*p_tscover)[i] < ((*p_ts2)[i] - (*p_maxcoverend)[i])))
		(*p_tscover)[inext] = (*p_ts2)[i] - (*p_maxcoverend)[i];
	}

/* Update tsgap */
if((*p_maxgapend)[i] != Infpos) (*p_tsgap)[inext] = (*p_ts2)[i] + (*p_maxgapend)[i];
else (*p_tsgap)[inext] = (*p_maxgapend)[i];
	
/* Update Ts */
if((*p_Ts)[i] < (*p_ts2)[i]) {
	(*p_Ts)[inext] = (*p_ts2)[i];
	if(j < 16384 && j > 1 && j < Jbol) (*p_BreakTempoPrev)[inext] = (*p_BreakTempo)[j];
	// Fixed j > 1 && j < Jbol by BB 2022-02-23
	else (*p_BreakTempoPrev)[inext] = FALSE; /* Simple note or silence */
	}
else {
	(*p_Ts)[inext] = (*p_Ts)[i];
	(*p_BreakTempoPrev)[inext] = (*p_BreakTempoPrev)[i];
	}
// BPPrintMessage(0,odInfo,"Update Ts: nseq = %ld i = %ld inext = %ld ts2[i] = %ld Ts[i] = %ld Ts[inext] = %ld\n",(long)nseq,(long)i,(long)inext,(long)(*p_ts2)[i],(long)(*p_Ts)[i],(long)(*p_Ts)[inext]);
	
iprev = i;
i = inext;

CHECK:
/* if(kmax > 100 && ++n > 10) {
	PleaseWait(); n = 0;
	} */
k = (*((*p_Seq)[nseq]))[i];
(*p_delta1)[i] = (*p_delta2)[i] = ZERO;

// BPPrintMessage(0,odInfo,"nseq = %ld p_imaxseq = %ld\n",(long)nseq,(long)(*p_imaxseq)[nseq]); 

if(k == -1) {	/* 'NIL' end-of-line marker */
	if((nature_time == SMOOTH) && (nseq == 0) && first) {
		(*p_ddelta0)[i] = (*p_ts2)[iprev];
	//	BPPrintMessage(0,odError,"\n=> ddelta0[%ld] = %ld\n",(long)i,(long)(*p_ddelta0)[i]);
		}
	for(ii=ZERO; ii <= (*p_imaxseq)[nseq]; ii++) {
		kk = (*((*p_Seq)[nseq]))[ii];
		if(kk > 1) (*p_delta)[kk] = (*p_delta1)[ii] + (*p_delta2)[ii];
		//* if(kk > 1) BPPrintMessage(0,odError,"\nBefore Solution_is_accepted:\nii = %ld kk = %ld starttime = %ld endtime = %ld, ts1[%ld] = %ld ts2[%ld] = %ld\n",(long)ii,(long)kk,(long)(*p_Instance)[kk].starttime,(long)(*p_Instance)[kk].endtime,ii,(*p_ts1)[ii],ii,(*p_ts2)[ii]); */
		}
	// BPPrintMessage(0,odError,"1) ts1[1] = %ld ts2[1] = %ld\n",(*p_ts1)[1],(*p_ts2)[1]);
	if((result=Solution_is_accepted(++nsol,nseq,p_imaxseq,kmax,p_ts1,p_ts2,p_delta,
				p_ddelta0,p_ddelta1,p_ddelta2)) == OK) {
		if(imaxseq > 1) {
		//	for(ii=1; ii <= (*p_imaxseq)[nseq]; ii++) { Fixed by BB 2021-03-20
			for(ii=ZERO; ii <= (*p_imaxseq)[nseq]; ii++) {
				kk = (*((*p_Seq)[nseq]))[ii];
				// if(kk ==  4) BPPrintMessage(0,odError,"2) ts1[%ld] = %ld ts2[%ld] = %ld\n",ii,(*p_ts1)[ii],ii,(*p_ts2)[ii]);
				if(kk > 1) { // Fixed by BB 2021-03-21
		//		if(kk >= 1) {
		//			if((*p_ts1)[ii]/Kpress > 100000 || (*p_ts2)[ii]/Kpress > 100000) BPPrintMessage(0,odError,"ERR: kk = %ld ts1[%ld] = %ld ts2[%ld] = %ld Kpress = %.0f\n",(long)kk,ii,(*p_ts1)[ii],ii,(*p_ts2)[ii],Kpress);
					(*p_Instance)[kk].starttime = (*p_ts1)[ii];
					(*p_Instance)[kk].endtime = (*p_ts2)[ii];
				//	if(kk > 1) BPPrintMessage(0,odError,"kk = %ld starttime = %ld endtime = %ld\n",(long)kk,(long)(*p_Instance)[kk].starttime,(long)(*p_Instance)[kk].endtime);
				//	if((*p_ts1)[ii]/Kpress > 100000 || (*p_ts2)[ii]/Kpress > 100000) BPPrintMessage(0,odError,"ERR: Solution_is_accepted Chunk_number = %d, Kpress = %ld, kk = %ld, ii = %ld starttime = %ld endtime = %ld\n",Chunk_number,(long)Kpress,(long)kk,ii,(long)(*p_Instance)[kk].starttime,(long)(*p_Instance)[kk].endtime);
					}
				// BPPrintMessage(0,odInfo,"Solution_is_accepted Chunk_number = %d, kk = %ld, ii = %ld starttime = %ld endtime = %ld, delta0 = %ld delta1 = %ld delta2 = %ld\n",Chunk_number,(long)kk,ii,(long)(*p_Instance)[kk].starttime,(long)(*p_Instance)[kk].endtime,(long)(*p_ddelta0)[ii],(long)(*p_ddelta1)[ii],(long)(*p_ddelta2)[ii]);
				if(ii <= imaxseq) {
					DELTA = (*p_DELTA)[ii]
						= (*p_ddelta0)[ii] + (*p_ddelta1)[ii] + (*p_ddelta2)[ii];
				//	BPPrintMessage(0,odInfo,"DELTA = %ld\n",(long)DELTA);
					}
				else {
					(*p_DELTA)[ii] = DELTA;
					}
				}
			}
		result = OK;
		goto QUIT;
		}
	else {
		if(result == ABORT) goto QUIT;
		/* result = MISSED */
		goto FINDMORE;
		}
	}

inext = i; while((*((*p_Seq)[nseq]))[++inext] == 0);
j = (*p_Instance)[k].object;	/* j can't be 0 because of previous inext calculation */

(*p_Instance)[k].truncbeg = ZERO;
(*p_Instance)[k].truncend = ZERO;
(*p_ts1)[i] = (*p_tp1)[i] = (*p_time1)[i] + (*p_ddelta0)[i];
(*p_ts2)[i] = (*p_tp2)[i] = (*p_time2)[i] + (*p_ddelta0)[i];
// BPPrintMessage(0,odInfo,"*** k = %ld j = %ld ts1[%ld] = %ld ts2[%ld] = %ld, time1[%ld] = %ld time2[%ld] = %ld, delta = %ld\n",(long)k,(long)j,(long)i,(*p_ts1)[i],i,(*p_ts2)[i],i,(*p_time1)[i],i,(*p_time2)[i],(*p_ddelta0)[i]);

if(i == i0 || (NoAlphabet && (nature_time != SMOOTH))) goto INCREMENT;

if(j < 0) j = -j;	/* Out-time object */
if(ibreak == 0) {
	if(j >= 16384 || (*p_BreakTempo)[j]) ibreak = i;
	}

shift1 = (*p_Ts)[i] - (*p_tp1)[i];
if((*p_Ts)[i] < Veryneg) shift1 = ZERO;
/* BPPrintMessage(0,odInfo,"\nTs[i]:\n");
for(iii = 1;  iii <= (*p_imaxseq)[nseq]; iii++) {
	BPPrintMessage(0,odInfo,"[%ld]%ld ",(long)iii,(long)(*p_Ts)[iii]);
	}
BPPrintMessage(0,odInfo,"\n"); */
// BPPrintMessage(0,odInfo,"shift1 = %ld, nseq = %ld, j = %ld, i = %ld Ts[i] = %ld tp1[i] = %ld tp2[i] = %ld\n",(long)shift1,(long)nseq,(long)j,(long)i,(long)(*p_Ts)[i],(long)(*p_tp1)[i],(long)(*p_tp2)[i]);

if(shift1 == ZERO) goto INCREMENT;
/* Added 8/3/97 */

if(Situation_ok(nseq,i,i0,j,shift1,(*p_tp1)[i],(*p_tp2)[i],(*p_Ts)[i],(*p_tscover)[i],
	(*p_tsgap)[i],(*p_maxgapbeg)[i],(*p_maxcoverbeg)[i],p_marked,nature_time)) {
	shift1 = ZERO;
	}

// BPPrintMessage(0,odInfo,"\nBefore Possible_choices() nseq = %ld, i= %ld, j = %ld, k = %ld, tp1[i] = %ld, tp2[i] = %ld, shift1 = %ld, Ts[i] = %ld, BreakTempoPrev[i] = %ld\n",(long)nseq,(long)i,(long)j,(long)k,(long)(*p_tp1)[i],(long)(*p_tp2)[i],(long)shift1,(long)(*p_Ts)[i],(long)(*p_BreakTempoPrev)[i]);
(*p_choice1)[i] = Possible_choices((*p_sol_set1)[i],(*p_BreakTempoPrev)[i],
	i,i0,j,k,nseq,p_marked,nature_time,(*p_tp1)[i],(*p_tp2)[i],shift1,(*p_Ts)[i],
	(*p_maxtruncbeg)[i],(*p_maxtruncend)[i],1);
	
if((*p_choice1)[i] > 5) {
	my_sprintf(Message,"=> Error1 choice1[%ld]=%ld.\n",(long)i,(long)(*p_choice1)[i]);
	if(Beta) Alert1(Message);
	}
if(shift1 == ZERO) goto INCREMENT;
if((*p_choice1)[i] > 1) goto NEWCHOICE1;

if(TraceTimeSet) {
#if BP_CARBON_GUI_FORGET_THIS
	if(StepTimeSet && (result=DrawSequence(nseq,NULL,p_ts1,p_ts2,
				kmax,(unsigned long)i,p_imaxseq,FALSE,p_ddelta0,p_ddelta1,p_ddelta2)) != OK)
		goto QUIT;
#endif /* BP_CARBON_GUI_FORGET_THIS */
	if(j < 16384)
		my_sprintf(Message,"\nCol#%ld side %ld  Ts=%ld t1=%ld  t2=%ld  \"%s\" ",
			(long)i,1L,(long)(*p_Ts)[i],(long)(*p_tp1)[i],(long)(*p_tp2)[i],*((*p_Bol)[j]));
	else
		my_sprintf(Message,"\nCol#%ld side %ld  Ts=%ld t1=%ld  t2=%ld  \"%s\" ",
			(long)i,1L,(long)(*p_Ts)[i],(long)(*p_tp1)[i],(long)(*p_tp2)[i],
			*((*(p_NoteName[NoteConvention]))[j-16384]));
	Print(wTrace,Message);
	if(shift1 > 0) {
		my_sprintf(Message,"should spend %ld milliseconds, but no solution.\n",(long)shift1);
		Print(wTrace,Message);
		}
	else {
		my_sprintf(Message,"should save %ld milliseconds, but no solution.\n",(long)-shift1);
		Print(wTrace,Message);
		}
	ShowSelect(CENTRE,wTrace);
	}
	
shift2 = - shift1;
Tsm = (*p_Ts)[i];
goto DECREMENT;
	
NEWCHOICE1:
if(kmax > 100 && ++n > 10) {
	PleaseWait(); n = 0;
	if(((r=InterruptTimeSet(TRUE,p_tstart)) != OK) || ((r=DoSystem()) != OK)) {
		result = r;
		goto QUIT;
		}
	}
if(redo && ((*p_choice1)[i] > 1)) {
	sol1 = Get_choice((*p_sol_set1)[i],&s,&olds,&redo,&stack_depth,1,i,j,
			(*p_tp1)[i],(*p_tp2)[i],(*p_Ts)[i],shift1);
	// BPPrintMessage(0,odInfo,"\nAfter Get_choice() sol1 = %ld\n",(long)sol1);
	if(sol1 == BACKTRACK) goto FINDMORE;
	if(sol1 == ABORT) {
		result = BACKTRACK;
		goto QUIT;
		}
	}
else {
#if BP_CARBON_GUI_FORGET_THIS
	if(StepTimeSet && (result=DrawSequence(nseq,NULL,p_ts1,p_ts2,kmax,(unsigned long)i,p_imaxseq,FALSE,
		p_ddelta0,p_ddelta1,p_ddelta2)) != OK) goto QUIT;
#endif /* BP_CARBON_GUI_FORGET_THIS */
	/* BPPrintMessage(0,odInfo,"\nTs[i]:\n");
	for(iii = 1;  iii <= (*p_imaxseq)[nseq]; iii++) {
		BPPrintMessage(0,odInfo,"[%ld]%ld ",(long)iii,(long)(*p_Ts)[iii]);
		}
	BPPrintMessage(0,odInfo,"\n"); */
	// BPPrintMessage(0,odInfo,"\nBefore Next_choice() shift1 = %ld, i = %ld, Ts[i] = %ld ts1[i] = %ld ts2[i] = %ld\n",(long)shift1,(long)i,(long)(*p_Ts)[i],(long)(*p_ts1)[i],(long)(*p_ts2)[i]);
	sol1 = Next_choice((*p_sol_set1)[i],nseq,i,i0,j,(*p_Ts)[i],(*p_ts1)[i],
																(*p_ts2)[i],shift1,1);
	// BPPrintMessage(0,odInfo,"After Next_choice() sol1 = %ld\n",(long)sol1);
	if((*p_choice1)[i] > 5) {
		my_sprintf(Message,"=> Error2 choice1[%ld]=%ld.\n",
			(long)i,(long)(*p_choice1)[i]);
		if(Beta) Alert1(Message);
		}
	if(sol1 == -8) {
		my_sprintf(Message,"\nnseq=%ld i=%ld side=1 j=%ld choice1=%ld sol_set=",
					(long)nseq,(long)i,(long)j,(long)(*p_choice1)[i]);
		Print(wTrace,Message);
		for(kk=0; kk < 5; kk++) {
			my_sprintf(Message,"%ld ",(long)((*p_sol_set1)[i])[kk]);
			Print(wTrace,Message);
			}
		Pause();
		DisplayTimeSet = TraceTimeSet = StepTimeSet = TRUE; goto START;
		}
	
	if(StackFlag && !redo) {
		if(Store(sol1,(*p_choice1)[i],1,&stack_depth,&s,&s0,i) == ABORT) {
			result = ABORT;
			goto QUIT;
			}
		}
	}
(*p_choice1)[i]--;
if(i < 0 || i >= imaxseq2) {
	if(Beta) Alert1("=> Err. Locate(). i > imaxseq2");
	result = ABORT;
	goto QUIT;
	}
((*p_sol_set1)[i])[sol1] = 0;
if(sol1 == ABORT) {
	result = ABORT;
	goto QUIT;
	}
if(sol1 == 4) {
	shift2 = - shift1;
	goto DECREMENT;
	}
(*p_tp1)[i] = (*p_time1)[i] + (*p_ddelta0)[i] + shift1;
(*p_tp2)[i] = (*p_time2)[i] + (*p_ddelta0)[i];
(*p_ddelta1)[i] = (*p_delta1)[i] = ZERO;
(*p_Instance)[k].truncend = ZERO;
if(sol1 == 2) { /* Truncate beginning */
	(*p_Instance)[k].truncbeg = shift1;
	}
else {
	(*p_Instance)[k].truncbeg = ZERO;
	(*p_tp2)[i] = (*p_tp2)[i] + shift1;
	if(sol1 == 0) (*p_delta1)[i] = shift1;	/* Shift object */
	if(sol1 == 1) {
		(*p_ddelta1)[i] = shift1;	/* Break tempo */
		// BPPrintMessage(0,odInfo,"\nBreaking tempo, shift1 = %ld\n",(long)shift1);
		}
	}
(*p_ts1)[i] = (*p_tp1)[i];
(*p_ts2)[i] = (*p_tp2)[i];
goto INCREMENT;

NEWCHOICE2:
if(kmax > 100 && ++n > 10) {
	PleaseWait(); n = 0;
	if(((r=InterruptTimeSet(TRUE,p_tstart)) != OK) || ((r=DoSystem()) != OK)) {
		result = r;
		goto QUIT;
		}
	}
if(choice2 > 5) {
	my_sprintf(Message,"=> Error1 choice2[%ld]=%ld",(long)i,(long)choice2);
	if(Beta) Alert1(Message);
	}
if(redo && (choice2 > 1)) {
	sol2 = Get_choice(sol_set2,&s,&olds,&redo,&stack_depth,2,i,j,(*p_ts1)[i],(*p_ts2)[i],
		(*p_Ts)[i],shift2);
	// BPPrintMessage(0,odInfo,"\nAfter Get_choice, sol2 = %ld\n",(long)sol2);
	if(sol2 == BACKTRACK) goto FINDMORE;
	if(sol2 == ABORT) {
		result = BACKTRACK;
		goto QUIT;
		}
	}
else {
#if BP_CARBON_GUI_FORGET_THIS
	if(StepTimeSet && (result=DrawSequence(nseq,NULL,p_ts1,p_ts2,kmax,(unsigned long)i,p_imaxseq,FALSE,p_ddelta0,p_ddelta1,p_ddelta2)) != OK)
		goto QUIT;
#endif /* BP_CARBON_GUI_FORGET_THIS */
	sol2 = Next_choice(sol_set2,nseq,i,i0,j,(*p_Ts)[i],(*p_ts1)[i],(*p_ts2)[i],shift2,2);
	if(sol2 == -8) {
		my_sprintf(Message,"\nnseq=%ld i=%ld side=2 j=%ld choice2=%ld sol_set=",
			(long)nseq,(long)i,(long)j,(long)choice2);
		Print(wTrace,Message);
		for(kk=0; kk<5; kk++) {
			my_sprintf(Message,"%ld ",(long)sol_set2[kk]);
			Print(wTrace,Message);
			}
		Pause(); DisplayTimeSet = TraceTimeSet = StepTimeSet = TRUE; goto START;
		}
	if(StackFlag && !redo) {
		if(Store(sol2,choice2,2,&stack_depth,&s,&s0,i) == ABORT) {
			result = ABORT;
			goto QUIT;
			}
		}
	}
choice2--;
sol_set2[sol2] = 0;
if(sol2 == ABORT) {
	result = ABORT;
	goto QUIT;
	}
if(sol2 == 4) {		/* Revise preceding object(s) */
	if((*p_choice1)[i] > 1) {
		goto NEWCHOICE1;
		}
	else {
		/* Possibly choice1[i] = 0 when i = i0 */
		if(((*p_choice1)[i] < 1) && (i != i0)) {
			my_sprintf(Message,"=> Err: choice1[%ld]=%ld i0=%ld",
				(long)i,(long)(*p_choice1)[i],(long)i0);
			if(Beta) Alert1(Message);
			}
		okmove = FALSE;
		if(j < 16384 && !(*p_OkRelocate)[j]) {
			if(shift2 < ZERO) {
				if((*p_ForwardMode)[j] == ABSOLU)
					maxmove = -(*p_MaxForward)[j];
				else
					maxmove = -(*p_MaxForward)[j] * ((*p_Instance)[k].dilationratio
						* (double)(*p_Dur)[j] / 100.);
				if(shift2 >= maxmove) okmove = TRUE;
				}
			else {
				if((*p_DelayMode)[j] == ABSOLU)
					maxmove = (*p_MaxDelay)[j];
				else
					maxmove = (*p_MaxDelay)[j] * ((*p_Instance)[k].dilationratio
						* (double)(*p_Dur)[j] / 100.);
				if(shift2 <= maxmove) okmove = TRUE;
				}
			}
		else okmove = TRUE;
		if((i > i0) && (okmove || ((shift2 > ZERO) && (ibreak > 0) && (ibreak < i)))) {
			if(TraceTimeSet) {
				if(j < 16384) {
					my_sprintf(Message,"Backtracking on \"%s\" col#%ld ibreak=%ld shift2=%ld\n",
						*((*p_Bol)[j]),(long)i,(long)ibreak,(long)shift2);
					}
				else {
					my_sprintf(Message,"Backtracking on \"%s\" col#%ld ibreak=%ld shift2=%ld\n",
						*((*(p_NoteName[NoteConvention]))[j-16384]),(long)i,(long)ibreak,(long)shift2);
					}
				Print(wTrace,Message);
				}
			goto DECREMENT;
			}
		else {
			if(TraceTimeSet) {
				if(j < 16384)
					my_sprintf(Message,
					"No use to backtrack on \"%s\" col#%ld ibreak=%ld shift2=%ld\n",
						*((*p_Bol)[j]),(long)i,(long)ibreak,(long)shift2);
				else
					my_sprintf(Message,
					"No use to backtrack on \"%s\" col#%ld ibreak=%ld shift2=%ld\n",
						*((*(p_NoteName[NoteConvention]))[j-16384]),(long)i,
						(long)ibreak,(long)shift2);
				Print(wTrace,Message);
				}
			if(AllSolTimeSet) {
				goto FINDMORE;
				}
			else {
				result = MISSED; goto QUIT;
				}
			}
		}
	}
ts1mem = (*p_ts1)[i]; ts2mem = (*p_ts2)[i];
delta2mem = (*p_delta2)[i]; ddelta2mem = (*p_ddelta2)[i];

(*p_ts2)[i] = (*p_ts2)[i] + shift2;
if(sol2 == 3) { /* Truncate end */
	(*p_Instance)[k].truncend = (*p_Instance)[k].truncend - shift2;
	}
else {
	(*p_ts1)[i] = (*p_ts1)[i] + shift2;
	if(sol2 == 0) (*p_delta2)[i] = (*p_delta2)[i] + shift2;	/* Shift object */
	if(sol2 == 1) (*p_ddelta2)[i] = (*p_ddelta2)[i] + shift2;	/* Break tempo */
	}
	
shift3 = (*p_Ts)[i] - (*p_ts1)[i];
if((*p_Ts)[i] < Veryneg) shift3 = ZERO;
shift4 = Alternate_correction1(i,i0,nseq,shift3,p_ts1,(*p_ts2)[i],(*p_tsgap)[i],
			(*p_tscover)[i],(*p_maxgapbeg)[i],(*p_maxcoverbeg)[i],(*p_maxtruncbeg)[i]);
if(shift4 == ZERO) {
	goto INCREMENT;
	}
else {
	if(choice2 > 1) {
		(*p_ts1)[i] = ts1mem; (*p_ts2)[i] = ts2mem;	/* TrEnd[] is unchanged */
		(*p_delta2)[i] = delta2mem; (*p_ddelta2)[i] = ddelta2mem;
		goto NEWCHOICE2;
		}
	else {
		shift2 = -shift4;
		goto DECREMENT;
		}
	}

DECREMENT:
if(kmax > 100 && ++n > 10) {
	PleaseWait(); n = 0;
	if(((r=InterruptTimeSet(TRUE,p_tstart)) != OK) || ((r=DoSystem()) != OK)) {
		result = r;
		goto QUIT;
		}
	}
Tsm = (*p_Ts)[i];
if(DisplayTimeSet) Print(wTrace,"<"); 
if(i == i0) {
	if(StepTimeSet) {
#if BP_CARBON_GUI_FORGET_THIS
		if((result=DrawSequence(nseq,NULL,p_ts1,p_ts2,kmax,(unsigned long)i,p_imaxseq,FALSE,p_ddelta0,p_ddelta1,p_ddelta2)) != OK)
			goto QUIT;
#endif /* BP_CARBON_GUI_FORGET_THIS */
		if(Pause() == 'Q') {
			result = ABORT; goto QUIT;
			}
		}
	if(AllSolTimeSet) {
		goto FINDMORE;
		}
	else {
		result = MISSED; goto QUIT;
		}
	}
i = iprev;
if(iprev > 0) {
	for(iprev=i-1; iprev >= (i0-1); iprev--) {
		if(iprev < 0) {
			my_sprintf(Message,"=> Error iprev = %ld",(long)iprev);
			if(Beta) Alert1(Message);
			result = ABORT; goto QUIT;
			}
		if(iprev == ZERO || (*((*p_Seq)[nseq]))[iprev] > 1) break;
		}
	}
k = (*((*p_Seq)[nseq]))[i];
j = (*p_Instance)[k].object;
if(j < 0) j = -j;
if((*p_Ts)[i] == Tsm) {
	if(TraceTimeSet) {
		if(j < 16384)
			my_sprintf(Message,"\nCol %ld NewTs=%ld ts1= %ld ts2=%ld Tsm=%ld \"%s\"  This object is not the one concerned.",
			(long)i,(long)(*p_Ts)[i],(long)(*p_ts1)[i],(long)(*p_ts2)[i],(long)Tsm,*((*p_Bol)[j]));
		else	/* Simple note */
			my_sprintf(Message,"\nCol %ld NewTs=%ld ts1= %ld ts2=%ld Tsm=%ld \"%s\"  This object is not the one concerned.",
			(long)i,(long)(*p_Ts)[i],(long)(*p_ts1)[i],(long)(*p_ts2)[i],(long)Tsm,
			*((*(p_NoteName[NoteConvention]))[j-16384]));
		Print(wTrace,Message);
		}
	if(DisplayTimeSet) PrintBehind(wTrace,"<");
	goto DECREMENT;
	}

choice2 = Possible_choices(sol_set2,(*p_BreakTempoPrev)[i],i,i0,j,k,nseq,p_marked,
	nature_time,(*p_ts1)[i],(*p_ts2)[i],shift2,Tsm,(*p_maxtruncbeg)[i],
	(*p_maxtruncend)[i],2);
if(choice2 > 5) {
	my_sprintf(Message,"=> Error2 choice2[%ld]=%ld",(long)i,(long)choice2);
	if(Beta) Alert1(Message);
	}
goto NEWCHOICE2;

FINDMORE:
redo = TRUE;
goto START;

QUIT:
if(StackFlag) Erase_stack(0,&stack_depth,s0);
MyDisposeHandle((Handle*)&p_BreakTempoPrev);
MyDisposeHandle((Handle*)&p_choice1);
MyDisposeHandle((Handle*)&p_tp1);
MyDisposeHandle((Handle*)&p_tp2);
MyDisposeHandle((Handle*)&p_ts1);
MyDisposeHandle((Handle*)&p_ts2);
MyDisposeHandle((Handle*)&p_delta);
MyDisposeHandle((Handle*)&p_delta1);
MyDisposeHandle((Handle*)&p_delta2);
MyDisposeHandle((Handle*)&p_Ts);
MyDisposeHandle((Handle*)&p_tscover);
MyDisposeHandle((Handle*)&p_tsgap);
MyDisposeHandle((Handle*)&p_ddelta0);
MyDisposeHandle((Handle*)&p_ddelta1);
MyDisposeHandle((Handle*)&p_ddelta2);
MyDisposeHandle((Handle*)&p_sol_set1);
return(result);
}


int Solution_is_accepted(int nsol,int nseq,unsigned long **p_imaxseq,long kmax,Milliseconds **p_t1,
	Milliseconds **p_t2,Milliseconds **p_delta,Milliseconds **p_ddelta0,Milliseconds **p_ddelta1,
	Milliseconds **p_ddelta2)
{
int j,rep,result;
unsigned long i,imaxseq,k;

if(nseq >= Maxconc) {
	if(Beta) Alert1("=> Err. Solution_is_accepted(). nseq >= Maxconc");
	return(OK);
	}
imaxseq = (*p_imaxseq)[nseq];
if(imaxseq < 2L) return(OK);

if(DisplayTimeSet) {
#if BP_CARBON_GUI_FORGET_THIS
	if((result=DrawSequence(nseq,NULL,p_t1,p_t2,kmax,(unsigned long)(*p_imaxseq)[nseq],p_imaxseq,FALSE,p_ddelta0,p_ddelta1,p_ddelta2))
		!= OK) return(result);
#endif /* BP_CARBON_GUI_FORGET_THIS */
	my_sprintf(Message,"\nsol#%ld ---------- SEQUENCE %ld ---------------------\n",
			(long)nsol,(long)(nseq+1));
	Print(wTrace,Message);
//	for(i=1L; i < imaxseq; i++) { // Fixed by BB 2021-03-20
	for(i=ZERO; i < imaxseq; i++) {
		k = (*((*p_Seq)[nseq]))[i];
		if(k < 2) continue;
		j = (*p_Instance)[k].object;
		if(j >= Jbol) {
			if(j < 16384)
				my_sprintf(Message,
	"#%ld \"%s\" [%ld,%ld] TrBeg=%ld TrEnd=%ld alpha=%f delta=%ld DELTA=%ld\n",
	   (long)i,*((*p_Patt)[j-Jbol]),(long)(*p_t1)[i],(long)(*p_t2)[i],(long)(*p_Instance)[k].truncbeg ,
	   (long)(*p_Instance)[k].truncend ,(*p_Instance)[k].alpha,(long)(*p_delta)[k] ,
	   (long)((*p_ddelta0)[i]+(*p_ddelta1)[i]+(*p_ddelta2)[i]));
	   		else	/* Simple note */
				my_sprintf(Message,
	"#%ld \"%s\" [%ld,%ld] TrBeg=%ld TrEnd=%ld alpha=%f delta=%ld DELTA=%ld\n",
	   (long)i,*((*(p_NoteName[NoteConvention]))[j-16384]),(long)(*p_t1)[i],
	   (long)(*p_t2)[i],(long)(*p_Instance)[k].truncbeg ,
	   (long)(*p_Instance)[k].truncend ,(*p_Instance)[k].alpha,(long)(*p_delta)[k],
	   (long)((*p_ddelta0)[i]+(*p_ddelta1)[i]+(*p_ddelta2)[i]));
   			Print(wTrace,Message);
   			}
   		else {
			if(j > 0) {
				my_sprintf(Message,
	"#%ld \"%s\" [%ld,%ld] TrBeg=%ld TrEnd=%ld alpha=%f delta=%ld DELTA=%ld\n",
	   (long)i,*((*p_Bol)[j]),(long)(*p_t1)[i],(long)(*p_t2)[i],(long)(*p_Instance)[k].truncbeg,
	   (long)(*p_Instance)[k].truncend,(*p_Instance)[k].alpha,(long)(*p_delta)[k],
	   (long)((*p_ddelta0)[i]+(*p_ddelta1)[i]+(*p_ddelta2)[i]) );
	   			Print(wTrace,Message);
	   			}
			if(j < 0) {
				j = -j;
				if(j > 1) {
					if(j < 16384)
						my_sprintf(Message,
			"#%ld \"<<%s>>\" [%ld,%ld] TrBeg=%ld TrEnd=%ld alpha=%f delta=%ld DELTA=%ld\n",
			   (long)i,*((*p_Bol)[j]),(long)(*p_t1)[i],(long)(*p_t2)[i],(long)(*p_Instance)[k].truncbeg,
			   (long)(*p_Instance)[k].truncend,(*p_Instance)[k].alpha,(long)(*p_delta)[k] ,
			   (long)((*p_ddelta0)[i]+(*p_ddelta1)[i]+(*p_ddelta2)[i]) );
			   		else
						my_sprintf(Message,
			"#%ld \"<<%s>>\" [%ld,%ld] TrBeg=%ld TrEnd=%ld alpha=%f delta=%ld DELTA=%ld\n",
			   (long)i,*((*(p_NoteName[NoteConvention]))[j-16384]),(long)(*p_t1)[i],
			   (long)(*p_t2)[i],(long)(*p_Instance)[k].truncbeg,
			   (long)(*p_Instance)[k].truncend ,(*p_Instance)[k].alpha,(long)(*p_delta)[k],
			   (long)((*p_ddelta0)[i]+(*p_ddelta1)[i]+(*p_ddelta2)[i]) );
		   			Print(wTrace,Message);
		   			}
	   			}
	   		}
		}
	my_sprintf(Message,"---------- (time resolution = %ld ms) ------------",(long)Time_res);
	Print(wTrace,Message);
	ShowSelect(CENTRE,wTrace);
	if(!StepTimeSet && !AllSolTimeSet) {
		if((rep=Pause()) == 'Q') return(ABORT);
		if(rep == 'S') StepTimeSet = TRUE;
		}
	else {
		if(AllSolTimeSet) {
			ShowMessage(TRUE,wMessage,"Accept this solution?");
			if((rep=Pause()) == ' ') {
				return(OK);
				}
			else {
				if(rep == 'S')  {
					StepTimeSet = TRUE;
					rep = 'U';
					}
				if(rep == 'U') {
					return(MISSED);
					}
				if(rep == 'Q') return(ABORT);
				}
			}
		}
	}
return(OK);
}


int Situation_ok(int nseq, int i, int i0, int j, Milliseconds shift, Milliseconds t1, Milliseconds t2,
	Milliseconds ts, Milliseconds tscover, Milliseconds tsgap,Milliseconds maxgapbeg,Milliseconds maxcoverbeg,
	char **p_marked,int nature_time)
{	
if(nseq >= Maxconc) {
	if(Beta) Alert1("=> Err. Situation_ok(). nseq >= Maxconc");
	return(YES);
	}
if(i == i0) return(YES);
if((nature_time == SMOOTH) && (nseq == 0)) return(NO);
/* else if(NoAlphabet) return(YES); */
if(shift == ZERO) return(YES);													/* 2 */
if((nature_time != SMOOTH) && j == 1) return(YES);		/* Silence "-" */
if(shift < ZERO) {
	if(j > 16383) return(YES);		/* Simple note */
	if(DiscontinuityOK || (tsgap >= t1 && (-shift <= maxgapbeg))) return(YES);	/* 1 */
	}																	
if(shift > ZERO) {
	if(nature_time == SMOOTH && !(*p_marked)[i]) return(NO);
	if((j == 1 || j > 16383) && nature_time == SMOOTH) return(NO);	/* Fixed 5/7/97, modified 25/4/98 and 10/6/98 */
	if(CoverOK || (t1 >= tscover && (maxcoverbeg == Infpos
						|| ts <= (t1 + maxcoverbeg)))) return(YES); 			/* 3 */
	}
return(NO);
}


char Possible_choices(solset sol,char BreakTempoPrev,int i,int i0,int j,long k,int nseq,
	char **p_marked,int nature_time,Milliseconds t1,Milliseconds t2,Milliseconds shift,
	Milliseconds ts,Milliseconds maxtruncbeg,Milliseconds maxtruncend,int side)

/*	sol[0] = Shift object
	sol[1] = Break tempo
	sol[2] = Truncate beginning
	sol[3] = Truncate end
	sol[4] = Revise previous object(s)	*/
	
/*	side = 1 means correcting clockwise, side = 2 anticlockwise */
{
int n,nmax,choice;
Milliseconds maxmove,mustmove;

if(nseq >= Maxconc) {
	if(Beta) Alert1("=> Err. Possible_choices(). nseq >= Maxconc");
	return(0);
	}
nmax = 5; choice = 0;
for(n=0; n < nmax; n++) sol[n] = FALSE;
if((i > i0) || side == 2) {
	(sol)[4] = TRUE; choice++;
	}
if(shift == ZERO) return(choice);
if((side == 1) && (nature_time == SMOOTH) && (shift > ZERO)) {
	(sol)[1] = TRUE; choice++;
	if(nseq == 0 || !(*p_marked)[i]) return(choice);
	}
if(j > 16383) {
	(sol)[0] = TRUE; choice++;
	}
else {
	if((*p_OkRelocate)[j]) {
		(sol)[0] = TRUE; choice++;
		}
	else {
		if(side == 1) mustmove = shift;
		else mustmove = -shift;
		if(mustmove > ZERO) {
			if((*p_DelayMode)[j] == ABSOLU) maxmove = (*p_MaxDelay)[j];
			else
				maxmove = (*p_MaxDelay)[j] * ((*p_Instance)[k].dilationratio
					* (double)(*p_Dur)[j] / 100.);
			if(mustmove <= maxmove) {
				sol[0] = TRUE; choice++;
				}
			}
		else {
			if((*p_ForwardMode)[j] == ABSOLU) maxmove = -(*p_MaxForward)[j];
			else
				maxmove = -(*p_MaxForward)[j] * ((*p_Instance)[k].dilationratio
					* (double)(*p_Dur)[j] / 100.);
			if(mustmove >= maxmove) {
				(sol)[0] = TRUE; choice++;
				}
			}
		}
	}	
if((side == 1) && (i > 1) && (shift > ZERO) && (BreakTempoPrev || (nature_time == SMOOTH))) {
	(sol)[1] = TRUE; choice++;
	}
if(side == 1 && shift > ZERO && shift <= maxtruncbeg) {
	(sol)[2] = TRUE; choice++;
	}
if(side == 2 && shift < ZERO && (-shift) <= maxtruncend) {
	(sol)[3] = TRUE; choice++;
	}
return(choice);
}


int Next_choice(solset sol_set, int nseq, int i, int i0, int j, Milliseconds ts,
	Milliseconds t1,Milliseconds t2, Milliseconds shift, int side)
{
char rep;
int n;

if(nseq >= Maxconc) {
	if(Beta) Alert1("=> Err. Next_choice(). nseq >= Maxconc");
	// BPPrintMessage(0,odError,"=> Err. Next_choice(). nseq >= Maxconc\n");
	return(OK);
	}
if(TraceTimeSet) {
	if(j < 16384) {
		if(j < Jbol)
			my_sprintf(Message,"Col#%ld nseq = %ld side = %ld ts = %ld  t1 = %ld  t2 = %ld \"%s\" \n",
			(long)i,(long)nseq,(long)side,(long)ts,(long)t1,(long)t2,*((*p_Bol)[j]));
		else // Fixed by BB 2022-02-24
			my_sprintf(Message,"Col#%ld nseq = %ld side = %ld ts = %ld  t1 = %ld  t2 = %ld \"%s\" \n",
			(long)i,(long)nseq,(long)side,(long)ts,(long)t1,(long)t2,*((*p_Patt)[j-Jbol]));
		}
	else
		my_sprintf(Message,"Col#%ld nseq = %ld side = %ld ts = %ld  t1 = %ld  t2 = %ld \"%s\" \n",
			(long)i,(long)nseq,(long)side,(long)ts,(long)t1,(long)t2,
			*((*(p_NoteName[NoteConvention]))[j-16384]));
	Print(wTrace,Message);
	// BPPrintMessage(0,odInfo,"%s",Message);
	if(shift > 0) {
		my_sprintf(Message,"must spend %ld milliseconds\n",(long)shift);
		}
	else {
		my_sprintf(Message,"must save %ld milliseconds\n",(long)-shift);
		}
	Print(wTrace,Message);
	// BPPrintMessage(0,odInfo,"%s",Message);
	}
if(StepTimeSet) {
	if(AllSolTimeSet && i > i0
			&& !sol_set[0] && !sol_set[1] && !sol_set[2] && !sol_set[3]) {
		goto REVISE;
		}
	if(sol_set[0]) Print(wTrace,"S)hift sound-object\n");
	if(sol_set[1]) Print(wTrace,"B)reak tempo\n");
	if(sol_set[2]) Print(wTrace,"T)runcate beginning\n");
	if(sol_set[3]) Print(wTrace,"T)runcate end\n");
	if(i > i0 && !AllSolTimeSet) Print(wTrace,"R)evise previous sound-object(s)\n");
	Print(wTrace,"Q)uit\n");
	
QUEST3:
	rep = GetCap();
	// HideWindow(Window[wMessage]);
	if(rep == 'Q') {
		Print(wTrace,"\nAborted...\n"); ShowSelect(CENTRE,wTrace);
		return(ABORT);
		}
	if(rep == 'R' && !AllSolTimeSet) {
REVISE:
		Print(wTrace,"Revising previous object(s)...\n"); ShowSelect(CENTRE,wTrace);
		return(4);
		}
	if(sol_set[0] && rep == 'S') {
		Print(wTrace,"Shifting sound-object...\n"); ShowSelect(CENTRE,wTrace);
		return(0);
		}
	else {
		if((sol_set[1]) && rep == 'B') {
			Print(wTrace,"Breaking tempo...\n"); ShowSelect(CENTRE,wTrace);
			return(1);
			}
		else {
			if(sol_set[2] && rep == 'T') {
				Print(wTrace,"Truncating beginning...\n"); ShowSelect(CENTRE,wTrace);
				return(2);
				}
			else {
				if(sol_set[3] && rep == 'T') {
					Print(wTrace,"Truncating end...\n"); ShowSelect(CENTRE,wTrace);
					return(3);
					}
				else {
					SysBeep(10); goto QUEST3;
					}
				}
			}
		}
	}

for(n=0; n < 5; n++) {
	if(sol_set[n] > 0) {
		// BPPrintMessage(0,odInfo,"sol_set[%ld] = %ld\n",(long)n,(long)sol_set[n]);
		return(n);
		}
	}
if(Beta) Alert1("=> Error Next_choice()");
Print(wTrace,"\nsol_set[n]=\n");
for(n=0; n < 5; n++) {
	my_sprintf(Message,"%ld ",(long)sol_set[n]);
	Print(wTrace,Message);
	}
Pause();
return(-8);
}


Milliseconds Alternate_correction1(int i,int i0,int nseq,Milliseconds shift,Milliseconds** p_ts1,
	Milliseconds t2,Milliseconds tsgap,Milliseconds tscover,Milliseconds maxgapbeg,Milliseconds maxcoverbeg,
	Milliseconds maxtruncbeg)
{
int j,k;
Milliseconds t1,shiftmore,s1,s2;

if(nseq >= Maxconc) {
	BPPrintMessage(0,odError,"=> Err. Alternate_correction1(). nseq >= Maxconc\n");
	return(ZERO);
	}
k = (*((*p_Seq)[nseq]))[i];
if(k < 1) {
	BPPrintMessage(0,odError,"=> Err. Alternate_correction1(). k < 1"); 
	return(ZERO);
	}
j = (*p_Instance)[k].object;
if(j < 0) j = -j;
t1 = (*p_ts1)[i];
if(tscover != Infneg) s1 = tscover - t1;
else s1 = Infneg;
if(!CoverOK && s1 > ZERO && s1 <= maxtruncbeg) {
	(*p_Instance)[k].truncbeg += s1;
	(*p_ts1)[i] += s1;
	t1 = (*p_ts1)[i];
	shift -= s1;
	}
if(shift == ZERO) return(ZERO); 	/* Situation 2 */
if(shift < ZERO) {
	if(DiscontinuityOK) return(ZERO);
	s1 = t1 - tsgap;				/* Situation 1 */
	if(maxgapbeg != Infpos) s2 = - shift - maxgapbeg;
	else s2 = Infneg;
	if(s1 <= ZERO && s2 <= ZERO) return(ZERO);
	if(s1 > s2) shiftmore = -s1;
	else shiftmore = -s2;
	
	if(TraceTimeSet) {
		if(j < 16384)
			my_sprintf(Message,"\nCol#%ld side %ld Ts=%ld  t1 =%ld  t2=%ld  \"%s\" ",
				(long)i,1L,(long)(t1+shift),(long)t1,(long)t2,*((*p_Bol)[j]));
		else
			my_sprintf(Message,"\nCol#%ld side %ld Ts=%ld  t1 =%ld  t2=%ld  \"%s\" ",
				(long)i,1L,(long)(t1+shift),(long)t1,(long)t2,
				*((*(p_NoteName[NoteConvention]))[j-16384]));
		Print(wTrace,Message);
		if(shiftmore < 0) {
			my_sprintf(Message,"forces continuity. Previous object(s) must spend %ld milliseconds\n",
				(long)-shiftmore);
			}
		else {
			my_sprintf(Message,"forces continuity. Previous object(s) must save %ld milliseconds\n",
				(long)shiftmore);
			}
		Print(wTrace,Message);
		}
	return(shiftmore);
	}
if(shift > ZERO) {
	if(CoverOK) return(ZERO);
	if(maxcoverbeg != Infpos) s2 = shift - maxcoverbeg;
	else s2 = Infneg;
	if(s1 <= ZERO && s2 <= ZERO) return(ZERO);
	if(s1 > s2) shiftmore = s1;
	else  shiftmore = s2;
	if(TraceTimeSet) {
		if(j < 16384)
			my_sprintf(Message,"\nCol#%ld side %ld Ts=%ld  t1 =%ld  t2=%ld  \"%s\" ",
				(long)i,1L,(long)(t1+shift),(long)t1,(long)t2,*((*p_Bol)[j]));
		else
			my_sprintf(Message,"\nCol#%ld side %ld Ts=%ld  t1 =%ld  t2=%ld  \"%s\" ",
				(long)i,1L,(long)(t1+shift),(long)t1,(long)t2,
				*((*(p_NoteName[NoteConvention]))[j-16384]));
		Print(wTrace,Message);
		if(shiftmore < 0) {
			my_sprintf(Message,"can't be covered. Previous object(s) must spend %ld milliseconds. Err. Alternate_correction1().\n",
				(long)-shiftmore);
			if(Beta) Alert1(Message);
			}
		else {
			my_sprintf(Message,"can't be covered. Previous object(s) must save %ld milliseconds\n",
				(long)shiftmore);
			}
		Print(wTrace,Message);
		}
	return(shiftmore);
	}
return(shift);
}


int Get_choice(solset sol_set, p_list2 ***p_s, p_list2 ***p_olds, int *p_redo,
	int *p_stack_depth, int side, int i, int j, Milliseconds t1, Milliseconds t2,
	Milliseconds ts, Milliseconds shift)
{
int ii,isol,oldisol,r;
p_list2 **nexts;
Handle ptr;

nexts = (***(p_s)).p;
if(nexts == NULL) {
	*(p_redo) = FALSE;
	oldisol = (***(p_s)).n;
	isol = BACKTRACK;
	for(ii= ++oldisol; ii < 5; ii++) {
		if(sol_set[ii]) {
			isol = ii; break;
			}
		}
	if(side != (***(p_s)).side) {
		my_sprintf(Message,"=> Error side: Get_choice. ");
		if(Beta) Alert1(Message);
		}
	if(i != (***(p_s)).i) {
		my_sprintf(Message,"=> Error i: Get_choice. ");
		if(Beta) Alert1(Message);
		}
	if(isol == BACKTRACK) {	/* All choices have been exhausted */
		if(**p_olds == NULL) {
			isol = ABORT;
			}
		else {
			*(p_s) = *(p_olds);
			ptr = (Handle) (***(p_s)).p;
			MyDisposeHandle(&ptr);
			(***(p_s)).p = NULL;
			(*p_stack_depth)--;
			}
		}
	else {
		(***(p_s)).n = isol;
		sol_set[isol] = 0;
		}
	}
else {
	isol = (***(p_s)).n;
	if(side != (***(p_s)).side) {
		my_sprintf(Message,"=> Error side: Get_choice. ");
		if(Beta) Alert1(Message);
		}
	if(i != (***(p_s)).i) {
		my_sprintf(Message,"=> Error i: Get_choice. ");
		if(Beta) Alert1(Message);
		}
	*(p_olds) = *(p_s);
	*(p_s) = nexts;
	}
return(isol);
}


int Store(int n,char nmax,int side,int *p_stack_depth,p_list2 ***p_s,p_list2 ***p_s0,int i)
{
p_list2 **nexts;

if(nmax < 2) return(OK);

if((nexts = (p_list2**) GiveSpace((Size)sizeof(p_list2))) == NULL) return(ABORT);
(**nexts).n = n;
(**nexts).side = side;
(**nexts).i = i;
(**nexts).p = NULL;
(*p_stack_depth)++;
if(*(p_s0) == NULL) {
	*(p_s0) = nexts;
	}
else {
	(***(p_s)).p = nexts;
	}
*(p_s) = nexts;
return(OK);
}


int Erase_stack(int i0, int *p_stack_depth, p_list2 **s0)
{
p_list2 **s,**ss;
int i;

s = s0;
if(i0 > 0) {
	for(i=0; i < i0-1; i++) {
		s = (**s).p;
		}
	ss = (**s).p;
	(**s).p = NULL;
	s = ss;
	}
else {
	s0 = NULL;
	}
for(i=i0; /* i < *p_stack_depth */; i++) {
	if(s == NULL) break;
	ss = (**s).p;
	MyDisposeHandle((Handle*)&s);
	s = ss;
	}
*p_stack_depth = i0;
return(OK);
}


long LocalPeriod(long *p_i1,long *p_i2,long imaxstreak)
{
long localperiod,gap;
Milliseconds oldt,oldi2;

while(TRUE) {
	if((*p_T)[*p_i1] > ZERO) break;
	if((*p_i1) < 2L) break;
	(*p_i1)--;
	}
oldt = (*p_T)[*p_i2]; oldi2 = *p_i2;
while(TRUE) {
	if((*p_i2) >= imaxstreak) break;
	(*p_i2)++;
	if((*p_T)[*p_i2] > ZERO) {
		if(oldt == (*p_T)[*p_i2]) (*p_i2) = oldi2;
		break;
		}
	}
gap = (*p_i2) - (*p_i1);
if(gap > ZERO && (*p_i2) < imaxstreak) {
	localperiod = ((*p_T)[*p_i2] - (*p_T)[*p_i1]) / gap;
	}
else localperiod = -1L;
return(localperiod);
}


int InterruptTimeSet(int uselim,unsigned long *p_tstart)
{
int result,dirtymem,compiledmem;
char rep;
// MIDI_Parameters parms;
long lastbyte;
unsigned long drivertime;

compiledmem = CompiledGr;
if(Improvize && !WriteMIDIfile && !OutCsound) {
	result = ListenToEvents();
	if(result == ENDREPEAT) return(result);
	if(Improvize && (result == ABORT || SkipFlag)) return(ABORT);
	if(result == QUICK || result == EXIT) return(result);
	}
#if BP_CARBON_GUI_FORGET_THIS
// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
if((result=MyButton(2)) != MISSED) {
	StopCount(0);
	
	Interrupted = TRUE;
	dirtymem = Dirty[wAlphabet]; Dirty[wAlphabet] = FALSE;
	if(result == OK)
		while((result = MainEvent()) != RESUME && result != STOP && result != EXIT);
	if(result == EXIT) return(result);
	if(result == STOP) return(ABORT);
	if(compiledmem && Dirty[wAlphabet]) {
		Alert1("Alphabet changed. Must recompile...");
		return(ABORT);
		}
	Dirty[wAlphabet] = dirtymem;
	if(LoadedIn && (!CompiledIn && (result=CompileInteraction()) != OK))
		return(result);
	if(Dirty[wTimeAccuracy]) {
		result = ResetMIDI(FALSE);
		if(result == ABORT || result == EXIT) return(result);
		if((result=CheckSettings()) == ABORT) return(result);
		else {
			Dirty[wTimeAccuracy] = FALSE; result = AGAIN;
			}
		return(result);
		}
		
	if(IsMidiDriverOn()) *p_tstart = GetDriverTime();
	
	PleaseWait();
	}
result = OK;
if(EventState != NO) return(EventState);
else {
	if(LimTimeSet && uselim && (IsMidiDriverOn())) {
	/*	DriverStatus(CLOCKTIME_CODE,(MIDI_Parameters*) &parms); */
		drivertime = GetDriverTime();
		if(*p_tstart + (TimeMax / Time_res) < drivertime) {
			ShowMessage(TRUE,wMessage,"Max time elapsed!");
			return(QUICK);
			}
		}
	}
#endif /* BP_CARBON_GUI_FORGET_THIS */
return(OK);
}