/* Compute.c (BP3) */

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


#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"

long Tstart;
int trace_compute = 0;
int trace_weights = 0;
/* long LastTime = ZERO;
long PianorollShift = ZERO; */

int Compute(tokenbyte ***pp_a,int fromigram,int toigram,long *p_length,int *p_repeat,int time_end_compute)
{
int r,igram,inrul,finish,again,outgram,outrul,displayproducemem,level;
unsigned long ix;

ReleaseProduceStackSpace();
MaxDeriv = MAXDERIV;
NumberInferences = 0L;

if(MakeComputeSpace(MaxDeriv) != OK) return(ABORT);
displayproducemem = DisplayProduce;
if(DisplayProduce && !ScriptExecOn) {
	BPActivateWindow(QUICK,wTrace);
	}
again = FALSE;
if((*p_repeat)) {
	if(ProduceStackDepth == -1) {
		my_sprintf(Message,"Can't repeat: more than %ld computations",(long)MAXDERIV);
		Alert1(Message);
		return(MISSED);
		}
	ShowMessage(TRUE,wMessage,"Repeating...");
	}
else ProduceStackDepth = 0;

// Tstart = GetDriverTime();  
Tstart = getClockTime();
r = OK;
NeedZouleb = 0;
Ctrlinit(); // Added 2024-07-06

REDO:
for(igram=fromigram; igram <= toigram; igram++) {
	inrul = 0;
	if((*p_repeat) && p_MemGram != NULL && igram < (*p_MemGram)[ProduceStackIndex])
		continue;
	if(again && p_MemGram != NULL && igram < (*p_MemGram)[ProduceStackDepth]) continue;
TRYSUBGRAM:
	finish = FALSE;
	r = ComputeInGram(pp_a,&Gram,igram,inrul,p_length,&finish,p_repeat,PROD,FALSE,
		&outgram,&outrul,time_end_compute);
		
	if(DisplayProduce) ShowSelect(CENTRE,wTrace);
	if(r == ABORT || r == EXIT || r == STOP) goto SORTIR;
	if(outgram > 0) {
		igram = outgram; inrul = outrul; goto TRYSUBGRAM;
		}
	if(r == MISSED) break;
	if(r == FINISH) continue;
	if(finish) {
		if(!SkipFlag
			&& (((r = ShowItem(igram,&Gram,FALSE,pp_a,(*p_repeat),PROD,FALSE)) == ABORT)
				|| r == EXIT || r == STOP)) goto SORTIR;
		StepProduce = DisplayProduce = TRUE;
		PlanProduce = TraceProduce = finish - 1;
		}
	if((r == BACKTRACK || r == AGAIN) && p_MemGram != NULL) {
		igram = (*p_MemGram)[ProduceStackIndex];
		igram--;
		continue;
		}
	if((r == UNDO)  || (r == ENDREPEAT)) {
		if((*p_repeat)) {
			if((r = Answer("Forget further choices",'N')) == NO) {
				if((r = Undo(pp_a,(*p_repeat))) != OK) goto SORTIR;
				goto REDO;
				}
			if(r != YES) goto SORTIR;
			(*p_repeat) = FALSE;
			if((r = Answer("Choose computations",'Y')) == YES)
				PlanProduce = TraceProduce = TRUE;
			if(r != NO) goto SORTIR;
			}
		again = TRUE;
		if((r = Undo(pp_a,(*p_repeat))) != OK) goto SORTIR;
		goto REDO;
		}
/*	if(StepGrammars && !StepProduce) {
		r = InterruptCompute(igram,&Gram,*p_repeat,0,PROD);
		if(r != OK) break;
		} */
	}
CompleteDecisions = TRUE;

SORTIR:

if(r == OK) {
	ix = ZERO;
	level = 0;
	if(NeedZouleb > 0) {
		if(ShowMessages) ShowMessage(TRUE,wMessage,"Applying serial tools...");
		do {
			r = Zouleb(pp_a,&level,&ix,FALSE,FALSE,0,(*p_repeat),FALSE,NOSEED);
			if(r != OK) break;
			}
		while(level >= 0);
		r = ShowItem(igram,&Gram,FALSE,pp_a,(*p_repeat),PROD,FALSE);
	//	if(ShowMessages) HideWindow(Window[wMessage]);
		}
	}
	
// HideWindow(Window[wMessage]);
if(DisplayProduce != displayproducemem) {
	DisplayProduce = displayproducemem;
	}
if(Beta && NeedZouleb != 0) {
	my_sprintf(Message,"NeedZouleb = %ld after Compute(). Should be 0",(long)NeedZouleb);
	ShowMessage(TRUE,wMessage,Message);
	}
if(ItemNumber == 0) ItemNumber = 1;
return(r);
}


int ComputeInGram(tokenbyte ***pp_a,t_gram *p_gram,int igram,int inrul,long *p_length,
	int *p_finish,int *p_repeat,int mode,int learn,int *p_outgram,
	int *p_outrul, int time_end_compute)
{
char c;
int rep,datamode,ifunc,ig,ir,j,irul,irep,nrep,**p_candidate,foundone,
	**p_prefrule,grtype,maxpref,nb_candidates,r,choice,shootagain,hastabs,
	freedom,w,notsaid,changed,randomnumber,
	halt,startfrom,try,maxtry,equalweight;
t_rule rule;
long i,position,firstposition,**p_origin,**p_pos,leftpos,lastpos,pos1,incmark,
	**p_totwght;
unsigned long datemem,time;
tokenbyte ***pp_b,**p_b,**p_c,***pp_c,instan[MAXLIN],meta[MAXMETA2];
t_subgram subgram;

if(p_gram == NULL) {
	Alert1("=> Err. in ComputeInGram(). p_gram == NULL");
	return(ABORT);
	}
/* if(!Improvize && time_end_compute > 0L && getClockTime() > time_end_compute) {
	EmergencyExit = TRUE; BPPrintMessage(0,odInfo,"\n➡ Maximum allowed time (%d seconds) has been spent in ComputeInGram(). Stopped computing...\n➡ This limit can be modified in the settings\n\n",MaxConsoleTime);
	return(ABORT);
	} */
if(p_gram->p_subgram == NULL) return(OK);
subgram = (*((*p_gram).p_subgram))[igram];
grtype = subgram.type;
pp_c = &p_c; p_c = NULL;	/* Buffer used for playing items */
p_candidate = p_prefrule = NULL;
p_totwght = NULL;
p_origin = p_pos = NULL;
*p_outgram = *p_outrul = 0;
rep = ABORT;
shootagain = FALSE;
try = irep = 0;
if(trace_compute) BPPrintMessage(0,odInfo,"ComputeInGram() igram = %d subgram.number_rule = %d\n",igram,subgram.number_rule);

if((p_candidate = (int**) GiveSpace((Size)(subgram.number_rule+1) * sizeof(int))) == NULL)
	goto QUIT;
if((p_totwght = (long**) GiveSpace((Size)(subgram.number_rule+1) * sizeof(long))) == NULL)
	goto QUIT;
if((p_pos = (long**) GiveSpace((Size)(subgram.number_rule+1) * sizeof(long))) == NULL)
	goto QUIT;
if((p_prefrule = (int**) GiveSpace((Size)(subgram.number_rule+1) * sizeof(int))) == NULL)
	goto QUIT;
rep = MISSED;

/* if(StepProduce || StepGrammars || TraceProduce) {
	my_sprintf(Message,"\n// Subgrammar %ld/%ld",(long)igram,(long)(*p_gram).number_gram);
	Println(wTrace,Message); 
//	ShowSelect(CENTRE,wTrace);
	}
else { */
	if(DisplayProduce || (ShowMessages && (*p_gram).number_gram > 1)) {
		my_sprintf(Message,"• Subgrammar %ld/%ld\n",(long)igram,(long)(*p_gram).number_gram);
		BPPrintMessage(0,odInfo,Message);
/*		if(DisplayProduce && !ScriptExecOn) {
			Print(wTrace,"\n// "); Println(wTrace,Message);
			} */
		}
//	}
if((*p_repeat) && (ProduceStackIndex >= ProduceStackDepth)) {
	(*p_repeat) = PlanProduce = FALSE;
	if(CompleteDecisions
		|| ((Answer("End of known computation. Continue",'Y') == YES)
			&& (PlanProduce=Answer("Choose candidate rules",'N')) != ABORT)) {
		TraceProduce = DisplayProduce = TRUE;
		}
	else {
		rep = ABORT;
		goto QUIT;
		}
	}
maxtry = subgram.number_rule / 2; /* See note on RND */

if(mode != PROD) goto NOPROD1;

ReseedOrShuffle(subgram.seed);

if(subgram.destru) {
	if(Destroy(pp_a) == ABORT) {
		rep = ABORT;
		goto QUIT;
		}
	if((rep = ShowItem(igram,p_gram,FALSE,pp_a,(*p_repeat),PROD,FALSE)) == ABORT
			|| rep == FINISH || rep == EXIT || rep == STOP) goto QUIT;
	}
if(subgram.print) {
	datamode = DisplayMode(pp_a,&ifunc,&hastabs);
	if((rep=PrintResult(datamode && hastabs,wTrace,hastabs,ifunc,pp_a)) != OK) goto QUIT;
	Print(wTrace,"\n");
	}
if(subgram.stop) {
	rep = InterruptCompute(igram,p_gram,(*p_repeat),grtype,PROD);
	if(rep != OK) goto QUIT;
	}
if(subgram.printon) DisplayProduce = TRUE;
if(subgram.printoff) DisplayProduce = FALSE;
if(subgram.stepon) StepProduce = TRUE;
if(subgram.stepoff) StepProduce = DisplayProduce = FALSE;
if(subgram.traceon) TraceProduce = DisplayProduce = TRUE;
if(subgram.traceoff) TraceProduce = StepProduce = DisplayProduce = FALSE;
if(subgram.randomize) ReseedOrShuffle(NEWSEED);
	
NOPROD1:
rep = OK;
maxpref = 0;
leftpos = ZERO;
incmark = 0;	/* Marker correction in SUB */
lastpos = 0;	/* Position for writing into B (SUB only) */
if(grtype == SUBtype) {
	if(*pp_a == NULL) {
		if(Beta) Alert1("=> Err. ComputeInGram(). *pp_a = NULL");
		rep = ABORT;
		goto QUIT;
		}
	if((p_b = (tokenbyte**)
		GiveSpace((Size) MyGetHandleSize((Handle)*pp_a))) == NULL) {
		rep = ABORT;
		goto QUIT;
		}
	pp_b = &p_b;
	}
else pp_b = pp_a;
foundone = FALSE;

RETRY:
if(grtype == SUBtype)
/* memset((void*)**pp_b,(int) TEND,MyGetHandleSize((Handle)*pp_b)); */
	for(i=0; i < MyGetHandleSize((Handle)*pp_b)/sizeof(tokenbyte); i++)
		(**pp_b)[i] = TEND;
changed = FALSE;

RETRY1:
notsaid = TRUE; halt = FALSE;
if((*p_repeat) && (ProduceStackIndex >= ProduceStackDepth)) {
	(*p_repeat) = PlanProduce = FALSE;
	if(CompleteDecisions
		|| (Answer("End of known computation. Continue",'Y') == YES
		&& (PlanProduce=Answer("Choose candidate rules",'N')) != ABORT)) {
		TraceProduce = DisplayProduce = TRUE;
		}
	else {
		rep = ABORT;
		goto QUIT;
		}
	}
if((*p_repeat) && p_MemGram != NULL
								&& ((*p_MemGram)[ProduceStackIndex] != igram)) {
	rep = OK;
	goto QUIT;
	}

CHECKINRUL:
freedom = FALSE; startfrom = 1;
try = irep = 0;
if(inrul > 0) {	/* Entering after '_goto' or '_failed' jump */
	irul = inrul; j = 0;
	rule = (*(subgram.p_rule))[irul];
	if(((*p_pos)[j] = FindArg(pp_a,grtype,rule.p_leftarg,TRUE,p_length,meta,
		instan,rule,mode,time_end_compute)) > -1 && !(*p_repeat)) goto TRY3;
	if((ig=(*(subgram.p_rule))[irul].failedgram) > 0) {
		/* Rule inrul is not candidate but it contains '_failed' procedure */
		ir = (*(subgram.p_rule))[irul].failedrule;
		if(ig != igram) {
			*p_outgram = ig;
			*p_outrul = ir;
			rep = OK;
			goto QUIT;
			}
		inrul = ir;
		goto CHECKINRUL;
		}
	/* If '_goto' and '_failed' jumps failed we start looking for any candidate rule... */
	if(subgram.type == ORDtype) {
		if(inrul < subgram.number_rule) {
			inrul++;
			goto CHECKINRUL;
			}
		}
	}
(*p_length) = LengthOf(pp_a);
if((*p_length) <= ZERO) return(OK);

while(((nb_candidates = FindCandidateRules(pp_a,p_gram,startfrom,igram,grtype,p_candidate,
	p_totwght,p_pos,p_prefrule,leftpos,&maxpref,&freedom,*p_repeat,
	mode,&equalweight,learn,time_end_compute)) > 0) || (nb_candidates == AGAIN)) {

/*	if(!Improvize &&  && getClockTime() > time_end_compute) {
		EmergencyExit =TRUE; BPPrintMessage(0,odInfo,"\n➡ Maximum allowed time (%d seconds) has been spent in ComputeInGram(). Stopped computing...\n➡ This limit can be modified in the settings\n\n",MaxConsoleTime);
		return(ABORT);
		} */
	try = irep = 0;
	if(trace_compute) BPPrintMessage(0,odInfo,"nb_candidates = %d (*p_repeat) = %d\n",nb_candidates,(*p_repeat));
	if(nb_candidates == EXIT) {
		rep = ABORT; goto QUIT;
		}
	if(nb_candidates == AGAIN) continue;
	if((*p_repeat)  && p_MemGram != NULL && ((*p_MemGram)[ProduceStackIndex] != igram)) {
		rep = OK;
		goto QUIT;
		}
	foundone = TRUE;
	if(LimCompute && rtMIDI) {
		Tstart = getClockTime();
		if(Tstart + TimeMax/10L < time) {
			my_sprintf(Message,"Max time elapsed!");
			if(!ScriptExecOn && !Improvize) {
				Alert1(Message);
				rep = ABORT; goto QUIT;
				}
			if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
			foundone = FALSE;
			halt = TRUE;
			break;
			}
		}
	if(PlanProduce || (*p_repeat)) {
		TextOffset dummy, selend;
		if(grtype == SUBtype && !freedom){  
			j = 0;
			goto DOIT;
			}
		TextGetSelection(&dummy, &firstposition, TEH[wTrace]);
		if(TraceProduce) {
			if(grtype == ORDtype) {
				my_sprintf(Message,"'ORD' grammar: selecting first candidate rule:\n");
				Print(wTrace,Message);
				if(trace_compute) BPPrintMessage(0,odInfo,Message);
				}
			else {
				my_sprintf(Message,"\nChoice:\n");
				Print(wTrace,Message);
				if(trace_compute) BPPrintMessage(0,odInfo,Message);
				}
			}
		if((p_origin=(long**) GiveSpace((Size)(nb_candidates+1) * sizeof(long))) == NULL) {
			rep = ABORT;
			goto QUIT;
			}
		for(j=0; j < nb_candidates; j++) {
			irul = (*p_candidate)[j];
			if(TraceProduce) {
				TextGetSelection(&dummy, &((*p_origin)[j]), TEH[wTrace]);
				Print(wTrace,"Candidate: ");
				ShowRule(p_gram,igram,irul,wTrace,1,NULL,TRUE,TRUE,TRUE);
				ShowSelect(CENTRE,wTrace);
				}
			}
		TextGetSelection(&dummy, &((*p_origin)[nb_candidates]), TEH[wTrace]);
ENTER:
		if(grtype == ORDtype || grtype == SUB1type || grtype == POSLONGtype) j = 0;
		else {
			if((nb_candidates == 1) || (*p_repeat)) {
				if(!(*p_repeat) && notsaid && grtype != ORDtype) {
					my_sprintf(Message,"Only one candidate rule...\n");
					Print(wTrace,Message);
					}
				j = 0;
				MyDisposeHandle((Handle*)&p_origin);
				goto DOIT;
				}
			ShowMessage(TRUE,wMessage,"Choosing candidate rule and repeating production not supported yet!");
			ShowMessage(TRUE,wMessage,"Selecting first rule.");
			j = 0;
			MyDisposeHandle((Handle*)&p_origin);
			}
		}
	else {
		if(trace_compute) BPPrintMessage(0,odInfo,"nb_candidates (2) = %d\n",nb_candidates);
		if(nb_candidates > 1) {
			randomnumber = rand();
			int total = (*p_totwght)[nb_candidates-1];
			choice = (total + 1) * (randomnumber/((double)RAND_MAX)); // Fixed 2024-07-06
			if(choice > total) choice = total;
		if(trace_weights) BPPrintMessage(0,odInfo,"ComputeInGram(%d). choice = %ld nb_candidates =  %d (*p_totwght)[nb_candidates-1] = %ld, randomnumber = %ld RAND_MAX = %ld\n",igram,(long)choice,nb_candidates,(long)(*p_totwght)[nb_candidates-1],(long)randomnumber,(long)RAND_MAX);
			UsedRandom = TRUE;
			j = 0;
			while(choice > (*p_totwght)[j]) { // Fixed 2024-07-06
			//	BPPrintMessage(0,odInfo,"igram =  %d (*p_totwght)[%d] = %ld\n",igram,j,(*p_totwght)[j]);
				if(++j >= nb_candidates) {
					BPPrintMessage(0,odInfo,"=> Err. ComputeInGram(). j = %ld, nb_candidates = %ld\n",
							(long)j,(long)nb_candidates);
					j = nb_candidates - 1; break;
					}
				}
/*			my_sprintf(Message,"j = %ld",(long)j);
			ShowMessage(TRUE,wMessage,Message); */
			}
		else j = 0;			/* ORD or SUB1 or POSLONG */
		}
DOIT:
	if((*p_repeat)) {
		irul = (*p_MemRul)[ProduceStackIndex];
		if(trace_compute) BPPrintMessage(0,odInfo,"Repeating irul= %d\n",irul);
		if(irul > subgram.number_rule) {
			Alert1("New grammar does not match old one. Aborting...");
			rep = ABORT; goto QUIT;
			}
		if(TraceProduce) {
			Print(wTrace,"Rule selected: ");
			ShowRule(p_gram,igram,irul,wTrace,1,NULL,TRUE,TRUE,TRUE);
			}
		}
	else irul = (*p_candidate)[j];
	if(trace_compute) BPPrintMessage(0,odInfo,"irul = %d\n",irul);
	rule = (*(subgram.p_rule))[irul];
	if(grtype == SUB1type) startfrom = irul;
	if(grtype == SUBtype && nb_candidates > 1) {
		(*p_prefrule)[maxpref++] = irul;
		}
	if(equalweight && try > 0) {
TRY2:
// Try any rule (see doc "Random problem")
		if(trace_compute) BPPrintMessage(0,odInfo,"try = %ld maxtry = %ld\n",(long)try,(long)maxtry);
		if(try > maxtry) {
			(*p_length) = LengthOf(pp_a);	/* was changed by FindArg() */
			if((*p_length) <= ZERO) {
				r = ABORT; goto QUIT;
				}
			try = 0;
			continue;
			}
		j = 0;
		randomnumber = rand();
		UsedRandom = TRUE;
		irul = 1 + (subgram.number_rule - 1) * (randomnumber / ((double)RAND_MAX));
		rule = (*(subgram.p_rule))[irul];
		irep = 0;
		if((rule.w == 0) ||
		((*p_pos)[j] = FindArg(pp_a,grtype,rule.p_leftarg,TRUE,p_length,meta,
							instan,rule,mode,time_end_compute)) == -1) {
			try++;
			if(CheckEmergency() != OK) return(ABORT);
			goto TRY2;
			}
TRY3:	(*p_length) = LengthOf(pp_a);	/* was changed by FindArg() */
		if((*p_length) <= ZERO)  {
			r = ABORT; goto QUIT;
			}
		try = 0;
		}
	rule = (*(subgram.p_rule))[irul];	/* Added 24/11/97 */
	if(TraceProduce) {
		 if(NumberCharsTrace < MAXCHARTRACE) {
			if(ProduceStackIndex >= 0)
				my_sprintf(Message,"\n[Step #%ld] Selected: ",(long)ProduceStackIndex + 1);
			else
				my_sprintf(Message,"\nSelected: ");
			Print(wTrace,Message);
			ShowRule(p_gram,igram,irul,wTrace,1,NULL,TRUE,TRUE,TRUE);
			NumberCharsTrace += strlen(Message);
			}
		}
	if(grtype == SUBtype) {
		(*p_pos)[j] = leftpos;
		}
	if((*p_repeat)) {
		(*p_pos)[j] = (*p_MemPos)[ProduceStackIndex++];
		if((*p_pos)[j] > ((*p_length) - 2)) {
			Alert1("New grammar does not match old one. Aborting...");
			rep = ABORT; goto QUIT;
			}
		}
	if(trace_compute) BPPrintMessage(0,odInfo,"(*p_pos)[j] = %ld (*p_repeat) = %ld mode = %d PROD = %d\n",(long)(*p_pos)[j],(long)(*p_repeat),mode,PROD);
	if(mode != PROD) goto NOPROD;
	if((c=rule.print) == 1 || c == 3) {
		datamode = DisplayMode(pp_a,&ifunc,&hastabs);
		if((rep=PrintResult(datamode && hastabs,wTrace,hastabs,ifunc,pp_a)) != OK) goto QUIT;
		Print(wTrace,"\n");
		}
	if((c=rule.stop) == 1 || c == 3) {
		rep = InterruptCompute(igram,p_gram,(*p_repeat),grtype,PROD);
		if(rep != OK) goto QUIT;
		}
	if((c=rule.printon) == 1 || c == 3) {
		DisplayProduce = TRUE;
		}
	if((c=rule.printoff) == 1 || c == 3) {
		DisplayProduce = FALSE;
		}
	if((c=rule.stepon) == 1 || c == 3) {
		StepProduce = TRUE;
		}
	if((c=rule.stepoff) == 1 || c == 3) {
		StepProduce = DisplayProduce = FALSE;
		}
	if((c=rule.traceon) == 1 || c == 3) {
		TraceProduce = DisplayProduce = TRUE;
		}
	if((c=rule.traceoff) == 1 || c == 3) {
		TraceProduce = StepProduce = DisplayProduce = FALSE;
		}
NOPROD:
	(*p_length) = LengthOf(pp_a);
	if((*p_length) <= ZERO)  {
		r = ABORT; goto QUIT;
		}
////////////////
	pos1 = Derive(pp_a,p_gram,pp_b,p_length,igram,irul,(*p_pos)[j],
		&leftpos,grtype,(*p_repeat),&changed,&lastpos,&incmark,mode,time_end_compute);
///////////////		
	irep++; try = 1;
	if(pos1 == ABORT || pos1 == EXIT) {
		rep = pos1; goto QUIT;
		}
/*	if(!Improvize &&  && getClockTime() > time_end_compute) {
		EmergencyExit =TRUE; BPPrintMessage(0,odInfo,"\n➡ Maximum allowed time (%d seconds) has been spent in ComputeInGram(). Stopped computing...\n➡ This limit can be modified in the settings\n\n",MaxConsoleTime);
		return(ABORT); 
		} */
	if(pos1 == STOP) {
		rep = MISSED;
		if(igram < (*p_gram).number_gram) {
			my_sprintf(Message,"Jump to subgrammar #%ld",(long)igram+1);
			rep = Answer(Message,'Y');
			}
		goto QUIT;
		}
	if(!(*p_repeat) && !DeriveFurther && mode == PROD && p_MemGram != NULL) {
		(*p_MemGram)[ProduceStackDepth] = igram;
		(*p_MemRul)[ProduceStackDepth] = irul;
		(*p_MemPos)[ProduceStackDepth] = pos1;
		if(++ProduceStackDepth >= MaxDeriv && IncreaseComputeSpace() != OK) return(ABORT);
		ProduceStackIndex = ProduceStackDepth;
		}
		
	if((*p_repeat) && (ProduceStackIndex >= ProduceStackDepth)) {
		(*p_repeat) = PlanProduce = FALSE;
		if((rep = ShowItem(igram,p_gram,FALSE,pp_a,(*p_repeat),mode,FALSE)) == ABORT
			|| rep == FINISH || rep == EXIT || rep == STOP) goto QUIT;
		if(CompleteDecisions ||
			(Answer("End of known computation.\nContinue",'Y') == YES
				&& (PlanProduce=Answer("Choose candidate rules",'N')) != ABORT)) {
			TraceProduce = DisplayProduce = TRUE;
			goto MORE;
			}
		else {
			rep = MISSED;
			goto QUIT;
			}
		}
	if(grtype != SUBtype && grtype != SUB1type) {
		if((c = ShowItem(igram,p_gram,FALSE,pp_a,(*p_repeat),mode,FALSE))
				== ABORT || c == STOP || c == FINISH || c == EXIT) {
			rep = c;
			goto QUIT;
			}
		else {
			if(c == UNDO) {
				if((*p_repeat)) {
					if((c = Answer("Forget further choices",'N')) == YES) {
						(*p_repeat) = FALSE;
						if(!PlanProduce
							&& (c = Answer("Choose computations",'Y')) == YES) {
							PlanProduce = TraceProduce = TRUE;
							}
						}
					if(c == ABORT) {
						rep = c;
						goto QUIT;
						}
					}
				if((c = Undo(pp_a,(*p_repeat))) != OK) {
					rep = c;
					goto QUIT;
					}
				if(p_MemGram != NULL && igram == (*p_MemGram)[ProduceStackIndex])
					continue;
				else {
					rep = BACKTRACK;
					goto QUIT;
					}
				}
			}
		}
	if(learn) (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].w++;

MORE:	
	if(Varweight && (grtype != SUBtype) && !shootagain) {
		w = rule.w;
		w = w + rule.incweight;
		if(w < 0) w = 0;
		(*((*((*p_gram).p_subgram))[igram].p_rule))[irul].w = w;
		}
/*	if(!Improvize && time_end_compute > 0L && getClockTime() > time_end_compute) {
		EmergencyExit =TRUE; BPPrintMessage(0,odInfo,"\n➡ Maximum allowed time (%d seconds) has been spent in ComputeInGram(). Stopped computing...\n➡ This limit can be modified in the settings\n\n",MaxConsoleTime);
		return(ABORT);
		} */
	if(Flagthere && (grtype != SUBtype) && !shootagain)
		if((rep=ChangeFlagsInRule(p_gram,igram,irul)) != OK) goto QUIT;
	shootagain = FALSE;
	if(grtype == SUBtype) {
		if(leftpos >= (*p_length)) {
			halt = TRUE;
			if(DisplayProduce || (changed && (!Improvize || !UseEachSub))) {
				if((p_c=(tokenbyte**) GiveSpace((Size) MyGetHandleSize((Handle)*pp_b))) == NULL) {
					rep = ABORT;
					goto QUIT;
					}
				if(CopyBuf(pp_b,pp_c) == ABORT) return(ABORT);
			//	BPPrintMessage(0,odInfo,"ShowItem()\n");
				c = ShowItem(igram,p_gram,FALSE,pp_c,(*p_repeat),mode,FALSE);
				MyDisposeHandle((Handle*)pp_c);
				if(c == ABORT || c == FINISH || c == EXIT || c == STOP) {
					if(!(*p_repeat))	{
						rep = c;
						goto QUIT;
						}
					else {
						StepProduce = DisplayProduce
							= PlanProduce = TraceProduce = FALSE;
						}
					}
				if(c == UNDO) {
					rep = UNDO;
					goto QUIT;
					}
				}
			break;
			}
		}
	if(mode == PROD && ((c=rule.print) == 2 || c == 3)) {
		datamode = DisplayMode(pp_a,&ifunc,&hastabs);
		if((rep=PrintResult(datamode && hastabs,wTrace,hastabs,ifunc,pp_a)) != OK) goto QUIT;
		Print(wTrace,"\n");
		}
	if(mode == PROD && ((c=rule.stop) == 2 || c == 3)) {
		rep = InterruptCompute(igram,p_gram,(*p_repeat),grtype,PROD);
		if(rep != OK) goto QUIT;
		}
	if(mode == PROD && ((c=rule.printon) == 2 || c == 3)) {
		DisplayProduce = TRUE;
		}
	if(mode == PROD && ((c=rule.printoff) == 2 || c == 3)) {
		DisplayProduce = FALSE;
		}
	if(mode == PROD && ((c=rule.stepon) == 2 || c == 3)) {
		StepProduce = TRUE;
		}
	if(mode == PROD && ((c=rule.stepoff) == 2 || c == 3)) {
		StepProduce = DisplayProduce = FALSE;
		}
	if(mode == PROD && ((c=rule.traceon) == 2 || c == 3)) {
		TraceProduce = DisplayProduce = TRUE;
		}
	if(mode == PROD && ((c=rule.traceoff) == 2 || c == 3)) {
		TraceProduce = StepProduce = DisplayProduce = FALSE;
		}
	if(mode == PROD && rule.destru) {
		if((rep=Destroy(pp_a)) != OK) goto QUIT;
		
		if((rep = ShowItem(igram,p_gram,FALSE,pp_a,(*p_repeat),PROD,FALSE)) == ABORT
				|| rep == FINISH || rep == EXIT || rep == STOP) goto QUIT;
		}
	if(equalweight && !(*p_repeat) && !PlanProduce) goto TRY2;
	
	/* Check '_repeat' */
	if(rule.repeatcontrol > 0) {
		nrep = ParamValue[rule.repeatcontrol];
		}
	else nrep = rule.repeat;
	if(irep < nrep) {
		j = 0;
		if(((*p_pos)[j] = FindArg(pp_a,grtype,rule.p_leftarg,TRUE,p_length,meta,
				instan,rule,mode,time_end_compute)) > -1 /* && !(*p_repeat) */) {
			shootagain = TRUE;
			goto TRY3;
			}
		if(CheckEmergency() != OK) return(ABORT);
		shootagain = FALSE;
		if((*p_pos)[j] == EXIT) {
			rep = (*p_pos)[j]; goto QUIT;
			}
		/* Look for '_failed' */					
		if((ig=rule.failedgram) > 0) {
			ir = rule.failedrule;
			if(ig != igram) {
				*p_outgram = ig;
				*p_outrul = ir;
				rep = OK;
				goto QUIT;
				}
			inrul = ir;
			goto CHECKINRUL;
			}
		}
	/* Check '_goto' jump */
	if((ig=rule.gotogram) > 0) {
		ir = rule.gotorule;
		if(ig != igram) {
			*p_outgram = ig; *p_outrul = ir;
			rep = OK;
			goto QUIT;
			}
		inrul = ir;
		goto CHECKINRUL;
		}
	}   /* End of while() loop */

if(nb_candidates == ABORT || nb_candidates == EXIT || nb_candidates == FINISH
		|| nb_candidates == STOP) {
	rep = nb_candidates;
	if(rep == ABORT && !Panic) {
		BPPrintMessage(0,odError,"=> Error finding nb_candidates in Compute.c\n");
		}
	goto QUIT;
	}
if(grtype == SUBtype) {
	if(*pp_b == NULL) {
		BPPrintMessage(0,odError,"=> Error ComputeInGram(). *pp_b = NULL");
		goto QUIT;
		}
//	BPPrintMessage(0,odInfo,"\nfoundone = %d, UseEachSub = %d, DisplayItems = %d, PlaySelectionOn =%d\n",foundone,UseEachSub,DisplayItems,PlaySelectionOn);
	if(UseEachSub && foundone && !PlaySelectionOn && DisplayItems) { // Added by BB 7 Nov 2020
		datamode = DisplayMode(pp_b,&ifunc,&hastabs);
		long length = LengthOf(pp_b);
	//	BPPrintMessage(0,odError,"Subst ItemNumber = %d length = %ld datamode = %d hastabs = %d ifunc = %d\n",ItemNumber,length,datamode,hastabs,ifunc);
		if(TraceProduce) {
			if(NumberCharsTrace < MAXCHARTRACE) {
				if((rep=PrintResult(datamode && hastabs,wTrace,hastabs,ifunc,pp_b)) != OK) {
					BPPrintMessage(0,odError,"=> Error calling PrintResult(1) in Compute.c\n");
					goto QUIT;
					}
				Print(wTrace,"\n");
				NumberCharsTrace += 20;
				}
			}
		if(NumberCharsData < MAXCHARDATA) {
			if((rep=PrintResult(datamode && hastabs,OutputWindow,hastabs,ifunc,pp_b)) != OK) {
				BPPrintMessage(0,odError,"=> Error calling PrintResult(2) in Compute.c\n");
				goto QUIT;
				}
			Print(OutputWindow,"\n");
			NumberCharsData += 20;
			}
	//	if(length > 0) BPPrintMessage(1,odInfo,"\n");
		}
	if(!halt && (leftpos = NextPos(pp_a,pp_b,&lastpos,&incmark,leftpos,1)) > -1L) {
		/* Skip next symbol and retry */
		goto RETRY1;
		}
	if(foundone) (*p_length) = CopyBuf(pp_b,pp_a); // Copy B to A
	if(*p_length == ABORT) {
		rep = ABORT;
		if(rep == ABORT) {
			BPPrintMessage(0,odError,"=> Error calling CopyBuf() in Compute.c\n");
			goto QUIT;
			}
		}
	lastpos = leftpos = ZERO; incmark = 0;
	if(UseEachSub) { 
		if(foundone && !PlaySelectionOn && DisplayItems) {
			datamode = DisplayMode(pp_a,&ifunc,&hastabs);
		//	BPPrintMessage(0,odInfo,"OutputWindow\n");
		//	BPPrintMessage(0,odError,"ItemNumber = %d datamode = %d hastabs = %d ifunc = %d\n",ItemNumber,datamode,hastabs,ifunc);
			if(NumberCharsData < MAXCHARDATA) {
				if((rep=PrintResult(datamode && hastabs,OutputWindow,hastabs,ifunc,pp_a)) != OK) {
					BPPrintMessage(0,odError,"=> Error calling PrintResult(3) in Compute.c\n");
					goto QUIT;
					}
			//	Print(OutputWindow,"\n");
				NumberCharsData += 20;
				}
		//	BPPrintMessage(0,odError,"@\n");  // odError to force printing if Improvize
			}
		if(!SkipFlag) {
			if((p_c = (tokenbyte**)
				GiveSpace((Size) MyGetHandleSize((Handle)*pp_b))) == NULL) {
				rep = ABORT;
				goto QUIT;
				}
			if(CopyBuf(pp_b,pp_c) == ABORT) return(ABORT);
		/*	PrintResult(datamode && hastabs,OutputWindow,hastabs,ifunc,pp_c);
			Print(OutputWindow," (2)\n"); */
			rep = ShowItem(igram,p_gram,TRUE,pp_c,(*p_repeat),mode,FALSE);
			MyDisposeHandle((Handle*)pp_c);
			if(rep == ABORT) {
				if(!Panic) BPPrintMessage(0,odError,"=> Error calling ShowItem() in Compute.c\n");
				goto QUIT;
				}
			if(rep == ABORT || rep == FINISH || rep == EXIT || rep == STOP) goto QUIT;
			}
		SkipFlag = FALSE;
		}
	if(rtMIDI) Tstart = getClockTime();
	if((Varweight || Flagthere) && (maxpref > 0)) {
		for(j=0; j < maxpref; j++) {
			irul = (*p_prefrule)[j];
			rule = (*(subgram.p_rule))[irul];
			if((rep=ChangeFlagsInRule(p_gram,igram,irul)) != OK) goto QUIT;
			w = rule.w;
			w = w + rule.incweight;
			if(w < 0) w = 0;
			(*((*((*p_gram).p_subgram))[igram].p_rule))[irul].w = w;
			if(w <= 0) {	/* stop substitutions when any prefered rule... */
							/* ... becomes unvalid */
 				if(DisplayProduce) {
					ShowMessage(TRUE,wMessage,"A weight has decreased to 0...");
					}
				if(TraceProduce) {
 					Print(wTrace,"A weight has decreased to 0.\n");
					}
				rep = OK;
				goto QUIT;
				}
			}
		}
	maxpref = 0;
	if(*pp_b == NULL) {
		if(Beta) Alert1("=> Err2. ComputeInGram(). *pp_b = NULL");
		goto QUIT;
		}
	leftpos = NextPos(pp_a,pp_b,&lastpos,&incmark,leftpos,0);
	if(leftpos == ABORT) {
		if(Beta) Alert1("Pos. 2");
		}
	if(changed && (nb_candidates
			= FindCandidateRules(pp_a,p_gram,1,igram,grtype,p_candidate,p_totwght,
				p_pos,p_prefrule,leftpos,&maxpref,&freedom,*p_repeat,
				mode,&equalweight,learn,time_end_compute)) > 0) {
		if(TraceProduce || trace_compute) {
			if(NumberCharsTrace < MAXCHARTRACE) {
				my_sprintf(Message,"Trying same grammar for new rules\n");
				Print(wTrace,Message);
				NumberCharsTrace += strlen(Message);
				}
			}
		lastpos = leftpos = ZERO; maxpref = incmark = 0;
		goto RETRY;
		}
	if(nb_candidates == AGAIN) goto RETRY;
	if(nb_candidates == ABORT || nb_candidates == FINISH || nb_candidates == EXIT) {
		rep = nb_candidates;
		goto QUIT;
		}
	}
rep = OK;

QUIT:

MyDisposeHandle((Handle*)&p_candidate);
MyDisposeHandle((Handle*)&p_totwght);
MyDisposeHandle((Handle*)&p_pos);
MyDisposeHandle((Handle*)&p_prefrule);
if(pp_b != pp_a) MyDisposeHandle((Handle*)pp_b);
return(rep);
}


int Undo(tokenbyte ***pp_a, int repeat)
{
long endofselection;
unsigned long datemem;
int r;
tokenbyte **p_x;

if((*pp_a) == NULL) {
	Alert1("No item has been produced...");
	return(MISSED);
	}
if(ProduceStackDepth <= 0) {
	Alert1("Can't undo. Decisions not available");
	return(MISSED);
	}
if(DisplayProduce && DisplayStackIndex < 2) {
	Alert1("Can't undo");
	return(MISSED);
	}
DisplayStackIndex--;
ProduceStackIndex = (*p_LastStackIndex)[DisplayStackIndex-1];
if(!repeat) {
	ProduceStackDepth = ProduceStackIndex;
	}
if(p_ItemStart == NULL
		|| DisplayStackIndex >= MyGetHandleSize((Handle)p_ItemStart) / sizeof(long)) {
	if(Beta) Alert1("=> Err. Undo(). p_ItemStart = NULL");
	return(ABORT);
	}
SetSelect((*p_ItemStart)[DisplayStackIndex-1],(*p_ItemEnd)[DisplayStackIndex-1],
	TEH[wTrace]);
p_x = NULL;
if(SelectionToBuffer(FALSE,FALSE,wTrace,&p_x,&endofselection,PROD) != OK) {
	MyDisposeHandle((Handle*)&p_x);
	SetSelect(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
	Alert1("Can't find previous work string");
	return(MISSED);
	}
if(CopyBuf(&p_x,pp_a) == ABORT) return(ABORT);
MyDisposeHandle((Handle*)&p_x);
SetSelect(endofselection,GetTextLength(wTrace),TEH[wTrace]);
TextDelete(wTrace);
if(DisplayStackIndex > 1) {
	UndoFlag = TRUE;
	datemem = CompileDate;
	ShowMessage(TRUE, wMessage, "FIXME: Choosing to RESUME in function Undo().");
	r = RESUME;
	UndoFlag = FALSE;
	if((datemem != CompileDate) || !CompiledGr || !CompiledPt) {
		Alert1("Grammar changed or recompiled.  Must abort...");
		return(ABORT);
		}
	if(r == STOP) {
		if(repeat) {
			StepProduce = DisplayProduce = PlanProduce
				= TraceProduce = FALSE;
			r = OK;	/* Computation should be completed. */
			}
		else {
			r = ABORT;
			}
		}
	if(r == RESUME) {
		r = OK; EventState = NO;
		}
	if(r == UNDO) {
		if(repeat) {
			if((r = Answer("Forget further choices",'N')) == YES) {
				repeat = FALSE;
				if(!PlanProduce && (r = Answer("Choose computations",'Y'))
						== YES) PlanProduce = TraceProduce = TRUE;
				}
			if(r == ABORT) return(r);
			}
		return(Undo(pp_a,repeat));
		}
	if(r != OK) return(r);
	}
if((r=CompileCheck()) != OK) return(r);
return(OK);
}


int Destroy(tokenbyte ***pp_a)
{
tokenbyte **p_x;
int rep;
long size;

if(*pp_a == NULL) {
	BPPrintMessage(0,odError,"=> Err. Destroy(). *pp_a = NULL\n");
	Panic = TRUE;
	return(ABORT);
	}
size = MyGetHandleSize((Handle)*pp_a);
if((p_x = (tokenbyte**) GiveSpace((Size)size)) == NULL) return(ABORT);
if((rep=PrintArg(FALSE,FALSE,0,FALSE,0,1,stdout,wData,&p_x,pp_a)) != OK) return(ABORT);
		/* stdout and wData are dummy: only copying A to x */
if(CopyBuf(&p_x,pp_a) == ABORT) return(ABORT);
MyDisposeHandle((Handle*)&p_x);
return(OK);
}


long NextPos(tokenbyte ***pp_a,tokenbyte ***pp_b,long *p_lastpos,long *p_incmark,
	long i,int skip)
/* Look for next variable or terminal */
{
tokenbyte m,p;
long j,jmax;

j = *p_lastpos;
if(i < 0) return(i);
if(*pp_b == NULL) {
	if(Beta) Alert1("=> Err. NextPos(). *pp_b = NULL");
	return(-1L);
	}
jmax = MyGetHandleSize((Handle)*pp_b) / sizeof(tokenbyte) - 2L;
while(TRUE) {
	m = (**pp_a)[i]; p = (**pp_a)[i+1];
	if(!skip || (m == TEND && p == TEND)) {
		switch(m) {
			case T3:
			case T4:
			case T7:
			case T8:
			case T9:
			case T25:
				*p_lastpos = j;
				return(i);		/* found */
			}
		if(m == TEND && p == TEND) {
			*p_lastpos = j;
			return(-1L);		/* failed */
			}
		}
	skip = FALSE;
	i++; i++;
	if(m == T2 && p != 0) {	/* '(:' Slave marker */
		p = p + (*p_incmark);
		}
	if(j >= jmax-1) {
		my_sprintf(Message,"=> Err. NextPos(). j=%ld  jmax=%ld ",(long)j,(long)jmax);
		if(Beta) Alert1(Message);
		return(ABORT);
		}
	(**pp_b)[j++] = m; (**pp_b)[j++] = p;	/* Copy any other symbol */
	}
}


int FindCandidateRules(tokenbyte ***pp_a,t_gram *p_gram,int startfrom,int igram,int grtype,
	int **p_candidate,long **p_totwght,long **p_pos,int **p_prefrule,
	long leftpos,int *p_maxpref,int *p_freedom,int repeat,int mode,
	int *p_equalweight,int learn,int time_end_compute) {
	
// Does this grammar contain candidate rules ?
// enlist them in *p_candidate[], store their cumulated weights
// in *p_totwght[], and leftmost occurrence of left arg in *p_pos[]

	t_rule rule;
	/* register */ int i,j,rep;
	tokenbyte **arg,instan[MAXLIN],meta[MAXMETA2],meta1[MAXMETA2];
	long pos,posmin,posmax,length,istart,jstart,value,lenc1,**p_length,lengthmax;
	int jj,sumwght,irul,n,w,s,r,dir,weight;
	p_flaglist **h;

	if((rep=ListenToEvents()) < 0) return(rep);

	if(CheckEmergency() != OK) return(ABORT);

	/* if(!Improvize && time_end_compute > 0L && getClockTime() > time_end_compute) {
		EmergencyExit =TRUE; BPPrintMessage(0,odInfo,"\n➡ Maximum allowed time (%d seconds) has been spent in FindCandidateRules(). Stopped computing...\n➡ This limit can be modified in the settings\n\n",MaxConsoleTime);
		return(ABORT);
		} */

	if(trace_compute) BPPrintMessage(0,odInfo,"FindCandidateRules() leftpos = %ld\n",leftpos);

	p_length = NULL;
	*p_freedom = FALSE; *p_equalweight = FALSE; weight = -1;
	if(AllItems && (grtype == SUBtype)) {
		Alert1("You cannot produce all items in a 'SUB' subgrammar");
		return(ABORT);
		}
	if(p_candidate == NULL || p_totwght == NULL || p_pos == NULL || p_prefrule == NULL) {
		if(Beta) Alert1("=> Err. FindCandidateRules(). Null handle");
		return(ABORT);
		}
		
	if(leftpos < 0L) return(0); 		/* Happens in 'SUB' grammars... */
										/* ... because of NextPos() */

	n = (*((*p_gram).p_subgram))[igram].number_rule;
	// BPPrintMessage(0,odInfo,"igram = %d n = %d\n",igram,n);
	(*p_totwght)[0] = 0;
	sumwght = 0;

		/* SUB: first try preference rules... */

	if(grtype == SUBtype && (*p_maxpref) > 0) {
		for(j=0; j < *p_maxpref; j++) {
			if(Improvize && SkipFlag) return(ABORT);
			if(Improvize && ((rep=ListenToEvents()) != OK)) return(rep);
			irul = (*p_prefrule)[j];
			rule = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul];
			h = rule.p_leftflag;
			if(h != NULL) {
				do {
					s = (**h).x;
					if(s > Jflag) {
						if(Beta) Alert1("=> Err. p_Swtch. ");
						return(ABORT);
						}
					if(s > 0) {
						if((**h).operator != OFF) {
							if((**h).paramcontrol == 1)
								value = ParamValue[(**h).refvalue];
							if((**h).paramcontrol == 2)
								value = (*p_Flag)[(**h).refvalue];
							else value = (**h).refvalue;
							switch((**h).operator) {
								case EQUAL:
									if((*p_Flag)[s] != value) goto NEXTRULE;
									break;
								case DIF:
									if((*p_Flag)[s] == value) goto NEXTRULE;
									break;
								case INFEQUAL:
									if((*p_Flag)[s] > value) goto NEXTRULE;
									break;
								case SUPEQUAL:
									if((*p_Flag)[s] < value) goto NEXTRULE;
									break;
								case INF:
									if((*p_Flag)[s] >= value) goto NEXTRULE;
									break;
								case SUP:
									if((*p_Flag)[s] <= value) goto NEXTRULE;
									break;
								default:
									if(Beta) Alert1("=> Err.FindCandidateRules(). Invalid flag comparison");
									goto NEXTRULE;
								}
							}
						else {
							if((*p_Flag)[s] <= 0) goto NEXTRULE;
							}
						}
					h = (**h).p;
					}
				while(h != NULL);
				}
			if(rule.ctrl > 0) {		/* <Kx> */
				rule.w = rule.weight = ParamValue[rule.ctrl];
				if(trace_weights) BPPrintMessage(0,odInfo,"ParamValue[%d] = %ld for rule.ctrl = %d\n",rule.ctrl,ParamValue[rule.ctrl],rule.ctrl);
				}
			
			if((w = rule.w) > 0
				&& Found(pp_a,grtype,rule.p_leftarg,rule.leftoffset,rule.leftnegcontext,
				&lenc1,leftpos,1,instan,meta,meta1,&istart,&jstart,
				&length,rule.ismeta,time_end_compute)
				&& OkContext(pp_a,grtype,rule,leftpos,length,meta,instan,
					mode,time_end_compute)) {
				if(TraceProduce) {
					my_sprintf(Message,"\nRule already selected: ");
					Print(wTrace,Message);
					ShowRule(p_gram,igram,irul,wTrace,1,NULL,TRUE,TRUE,TRUE);
					}
				(*p_candidate)[0] = irul;
				(*p_totwght)[0] = 1;
				*p_freedom = FALSE;
				return(1);
				}
	NEXTRULE: ;
			}
		}
		
		/* Now try all the remaining rules... */


	if(trace_compute) BPPrintMessage(0,odInfo,"Remaining rules startfrom = %d\n",startfrom);
	if(startfrom > n) return(0);
	if(grtype == POSLONGtype && ((p_length)=(long**)GiveSpace((Size) (n + 1) * sizeof(long))) == NULL)
		return(ABORT);
	(*p_totwght)[0] = 1;	/* Needed if only one candidate in a LIN grammar */
	if(mode == ANAL) {
		startfrom = n; dir = -1; *p_equalweight = FALSE;
		}
	else {
		dir = 1;
		if(!(*p_gram).hasproc && (grtype == RNDtype) && !AllItems) *p_equalweight = TRUE;
		}

	for(irul=startfrom,i=0,sumwght=0; irul >= 1 && irul <= n; irul+=dir) {
	//	if(trace_compute) 
		
	/*	if(!StepProduce && !StepGrammars) {
			if(Improvize && SkipFlag) {
				i = ABORT; goto OVER;
				}
			if(Improvize && ((rep=ListenToEvents()) != OK)) {
				i = rep; goto OVER;
				}
			} */
		rule = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul];
		if(mode == PROD) {
			if(rule.ctrl > 0) {		/* <Kx> */
				rule.w = rule.weight = ParamValue[rule.ctrl];
				*p_equalweight = FALSE;
				if(trace_weights) BPPrintMessage(0,odInfo,"irul = %d rule.ctrl = %d w = %ld\n",irul,rule.ctrl,(long)rule.w);
				}
			if((w = rule.w) == 0) {
				continue;
				}
			if(rule.operator == 2) continue; /*  <--  */
			}
		if(mode == TEMP && rule.operator == 2) continue; /*  <--  */
		if(mode == ANAL) {
			if(rule.operator == 1) continue; /*  -->  */
			if(!learn && (w = rule.weight) == 0) continue;
			}
		if(mode == PROD || mode == TEMP) {
			h = rule.p_leftflag;
			if(h != NULL) {
				do {
					s = (**h).x;
					if(s > Jflag) {
						if(Beta) Alert1("=> Err. p_Swtch. ");
						i = ABORT;
						goto OVER;
						}
					if(s > 0) {
						if((**h).operator != OFF) {
							if((**h).paramcontrol == 1)
								value = ParamValue[(**h).refvalue];
							if((**h).paramcontrol == 2)
								value = (*p_Flag)[(**h).refvalue];
							else value = (**h).refvalue;
							switch((**h).operator) {
								case EQUAL:
									if((*p_Flag)[s] != value) goto NEXTRULE2;
									break;
								case DIF:
									if((*p_Flag)[s] == value) goto NEXTRULE2;
									break;
								case INFEQUAL:
									if((*p_Flag)[s] > value) goto NEXTRULE2;
									break;
								case SUPEQUAL:
									if((*p_Flag)[s] < value) goto NEXTRULE2;
									break;
								case INF:
									if((*p_Flag)[s] >= value) goto NEXTRULE2;
									break;
								case SUP:
									if((*p_Flag)[s] <= value) goto NEXTRULE2;
									break;
								default:
									if(Beta) Alert1("=> Err.FindCandidateRules(). Invalid flag comparison");
									goto NEXTRULE2;
								}
							}
						else {
							if((*p_Flag)[s] <= 0) goto NEXTRULE2;
							}
						}
					h = (**h).p;
					}
				while(h != NULL);
				*p_equalweight = FALSE;
				}
			arg = rule.p_leftarg;
			if(mode == TEMP) *p_equalweight = FALSE;
			if(*p_equalweight) {
				if(weight == -1) weight = rule.w;
				else if(weight != rule.w) *p_equalweight = FALSE;
				}
			}
		else {
			arg = rule.p_rightarg;
			}
		if(CheckEmergency() != OK) return(ABORT);
		if((grtype != SUBtype && (pos=FindArg(pp_a,grtype,arg,TRUE,&length,meta,instan,rule,
						mode,time_end_compute)) != -1)
				|| (grtype == SUBtype && Found(pp_a,grtype,arg,rule.leftoffset,rule.leftnegcontext,&lenc1,
					leftpos,1,instan,meta,meta1,&istart,&jstart,&length,rule.ismeta,time_end_compute)
						&& OkContext(pp_a,grtype,rule,leftpos,length,meta,instan,PROD,time_end_compute))) {
			(*p_pos)[i] = pos;
			if(pos == ABORT) return(ABORT);
			if(grtype == POSLONGtype) (*p_length)[i] = length;
			if(((grtype != LINtype && grtype != SUBtype) && ((mode == ANAL)
					|| ((mode == PROD) && (w == INT_MAX || grtype == ORDtype || grtype == SUB1type))))) {
				(*p_candidate)[0] = (*p_prefrule)[0] = irul;
				(*p_totwght)[0] = 1; (*p_pos)[0] = pos;
				(*p_maxpref) = 1;
				(*p_freedom) = (*p_equalweight) = FALSE;
				i = 1;
				goto OVER;
				}
			(*p_candidate)[i] = irul;
			if(trace_compute) BPPrintMessage(0,odInfo,"(*p_candidate)[%d] = %d\n",i,irul);
			if(grtype != LINtype && grtype != POSLONGtype) {
				if(trace_weights)
					BPPrintMessage(0,odInfo,"1) sumwght = %ld w = %ld\n",(long)sumwght,(long)w);
				sumwght = (*p_totwght)[i] = sumwght + w;
				}
			i++;
			}
	NEXTRULE2: ;
		}
		
	if(i > 1) {
		(*p_freedom) = TRUE;
		if(grtype == LINtype) {	/* Find left/rightmost derivation(s) */
			switch(mode) {
				case PROD:
				case TEMP:
					posmin = INT_MAX;
					for(j=0; j < i; j++) {
						irul = (*p_candidate)[j];
						rule = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul];
						(*p_pos)[j] += rule.leftoffset;
						if((pos=(*p_pos)[j]) <= posmin) {
							posmin = pos;
							}
						}
					sumwght = 0;
					for(j=jj=0; j < i; j++) {
						if((*p_pos)[j] == posmin) {
							(*p_candidate)[jj] = irul = (*p_candidate)[j];
							rule = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul];
							if(rule.w == INT_MAX) {
								(*p_candidate)[0] = irul;
								(*p_totwght)[0] = 1;	/* Not used */
								(*p_pos)[0] = posmin - rule.leftoffset;
								*p_maxpref = 1;
								jj = 1;
								break;
								}
							(*p_pos)[jj] = posmin - rule.leftoffset;
							if(trace_weights)
								BPPrintMessage(0,odInfo,"2) sumwght = %ld w = %ld weight = %ld irul = %d ParamValue[%d] = %d\n",(long)sumwght,(long)rule.w,(long)rule.weight,irul,rule.ctrl,ParamValue[rule.ctrl]);
							sumwght = (*p_totwght)[jj] = sumwght + rule.w;
							jj++;
							}
						}
					i = jj; if(i < 2) *p_freedom = FALSE;
					break;
				case ANAL:
					posmax = -1;
					for(j=0; j < i; j++) {
						irul = (*p_candidate)[j];
						rule = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul];
						(*p_pos)[j] -= rule.rightoffset;
						if((pos=(*p_pos)[j]) >= posmax) {
							posmax = pos;
							}
						}
					for(j=0; j < i; j++) {
						if((*p_pos)[j] == posmax) {
							(*p_candidate)[0] = irul = (*p_candidate)[j];
							rule = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul];
							(*p_pos)[0] = posmax + rule.rightoffset;
							break;
							}
						}
					i = 1; break;
					break;
				}
			}
		if(grtype == POSLONGtype) {	/* Find leftmost + longest derivation */
			posmin = INT_MAX;
			for(j=0; j < i; j++) {
				irul = (*p_candidate)[j];
				rule = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul];
				(*p_pos)[j] += rule.leftoffset;
				if((pos=(*p_pos)[j]) <= posmin) posmin = pos;
				}
			lengthmax = ZERO;
			for(j=0; j < i; j++) {
				if((*p_pos)[j] == posmin) {
					if(lengthmax < (*p_length)[j]) lengthmax = (*p_length)[j];
					}
				}
			sumwght = 0;
			for(j=jj=0; j < i; j++) {
				if((*p_pos)[j] == posmin && lengthmax == (*p_length)[j]) {
					(*p_candidate)[jj] = irul = (*p_candidate)[j];
					rule = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul];
					(*p_candidate)[0] = irul;
					(*p_totwght)[0] = 1;	/* Not used */
					(*p_pos)[0] = posmin - rule.leftoffset;
					*p_maxpref = 1;
					jj = 1;
					break;
					}
				}
			i = jj; if(i < 2) *p_freedom = FALSE;
			}
		}
	else {
		if(grtype == SUBtype && i == 1) (*p_prefrule)[(*p_maxpref)++] = (*p_candidate)[0];
		}

	OVER:
	MyDisposeHandle((Handle*)&p_length);
	if(trace_compute) BPPrintMessage(0,odInfo,"End FindCandidateRules i = %d\n",i);

	return(i);
	}


int OkContext(tokenbyte ***pp_a,int grtype,t_rule rule,long pos,long length,
	tokenbyte meta[],tokenbyte instan[],int mode,int time_end_compute)
/* Check remote context */
{
int sign;
long pos1,pos2,length1;

if(rule.p_leftcontext != NULL) {
	sign = (*rule.p_leftcontext)->sign;
	pos1 = FindArg(pp_a,grtype,(*rule.p_leftcontext)->p_arg,FALSE,&length1,meta,instan,
		rule,PROD, time_end_compute);
	if(pos1 == ABORT) return(ABORT);
	pos2 = pos1 + length1;
	if((sign && (pos1 == -1 || (pos2 > pos))) || (!sign && (pos1 != -1)
		&& (pos2 <= pos))) {
		return(NO);
		}
	}
if(rule.p_rightcontext != NULL) {
	sign = (*rule.p_rightcontext)->sign;
	pos1 = FindArg(pp_a,grtype,(*rule.p_rightcontext)->p_arg,FALSE,
							&length1,meta,instan,rule,PROD,time_end_compute);
	if(pos1 == ABORT) return(ABORT);
	pos2 = pos + length;
	if(!((sign && (pos1 >= pos2)) || (!sign && (pos1 < pos2)))) {
		return(NO);
		}
	}
return(YES);
}


long FindArg(tokenbyte ***pp_a,int grtype,tokenbyte **p_arg,int reset,
	long *p_length,tokenbyte meta[],tokenbyte instan[],t_rule rule,int mode,int time_end_compute)
/* Search left/rightmost pattern position in A[] */
{
long pos;
long startpos,istart,jstart,lenc1;
int dir;
tokenbyte meta1[MAXMETA2];

switch(mode) {
	case TEMP:
	case PROD:	/* leftmost search */
		dir = 2; startpos = 0;
		break;
	case ANAL:	/* rightmost search */
		pos = 0; while((**pp_a)[pos] != TEND || (**pp_a)[pos+1] != TEND) pos+=2;
		*p_length = pos; if(pos < 2) return(-1);
		dir = -2; startpos = *p_length - 2;
		if(startpos != (startpos / 2) * 2) {
			if(Beta) Alert1("=> Error FindArg(). Odd 'startpos'");
			startpos = (startpos / 2) * 2;
			}
		break;
	}
for(pos=startpos; pos >= 0 && ((**pp_a)[pos] != TEND || (**pp_a)[pos+1] != TEND);
		pos += dir) {
			
	if(CheckEmergency() != OK) return(ABORT);
	
	if(Found(pp_a,grtype,p_arg,0,rule.leftnegcontext,&lenc1,pos,reset,instan,meta,
		meta1,&istart,&jstart,p_length,rule.ismeta,time_end_compute)) {
		if(reset) {
			if(OkContext(pp_a,grtype,rule,pos,*p_length,meta,instan,
				mode,time_end_compute)) return(pos);
			}
		else {
			return(pos);
			}
		}
	}
return(-1);
}


int Found(tokenbyte ***pp_a,int grtype,tokenbyte **p_arg,long offset,int lenc,
	long *p_lenc1,long pos,int reset,tokenbyte instan[],tokenbyte meta[],
	tokenbyte meta1[],long *p_istart,long *p_jstart,long *p_length,int ismeta,int time_end_compute)
{
int i,j,i1,i2,j1,j2,xi,istart,jstart;
int nexist,nefound,result;

if(CheckEmergency() != OK) return(ABORT);

/* if(!Improvize && time_end_compute > 0L && getClockTime() > time_end_compute) {
		EmergencyExit =TRUE; BPPrintMessage(0,odInfo,"\n➡ Maximum allowed time (%d seconds) has been spent in Found(). Stopped computing...\n➡ This limit can be modified in the settings\n\n",MaxConsoleTime);
		return(ABORT);
		} */
		
// offset = rule.leftoffset if grtype = SUBtype; offset = 0 otherwise.
if(offset > 0 && grtype != SUBtype) {
	if(Beta) Alert1("=> Err. Found(). offset > 0");
	offset = 0;
	}
*p_length = 0;
if(ismeta) {
	for(i=0; i< MAXMETA2; i++) meta1[i] = -1; /* context wildcard table */
	if(reset) {
		for(i=0; i< MAXMETA2; i++) meta[i] = -1; /* reset wildcard table */
		}
	}

// Calculate "lenc" the length of leftmost neg context
// for(i=(*p_lenc1)= 0; (*p_arg)[i] == T0 && (*p_arg)[i+1] == 2; (*p_lenc1)++, i+=4);

xi = 0; nexist = FALSE;
if(pos < offset) return(FALSE);
istart = 4 * (lenc - (pos - offset) / 2);
jstart = 0; nefound = TRUE;
if(istart <=  0) {
	jstart = pos - offset - 2 * lenc; istart = 0; nefound = FALSE;
	if(trace_compute) BPPrintMessage(0,odInfo,"Found() istart = %d lenc = %d jstart = %d pos = %ld offset = %ld\n",istart,lenc,jstart,(long)pos,(long)offset);
	}
*p_lenc1 = lenc - (istart / 4);	/* Used only for SUB */
if(jstart < 0) return(FALSE);	/* Not found */
(*p_istart) = istart; (*p_jstart) = jstart;
for(i=istart,j=jstart;
	(*p_arg)[i] != TEND || (*p_arg)[i+1] != TEND; i+=2,j+=2) {
	i1 = (int) (*p_arg)[i]; i2 = (int) (*p_arg)[i+1];
	j1 = (**pp_a)[j]; j2 = (**pp_a)[j+1];
	if(j1 == TEND && j2 == TEND) { 	/* end of buffer */
		if(i1 == T0 && i2 == 2) {			/* '#' */
			nefound = TRUE;
			i+=2; j-=2;
			continue;			/* will redo this loop till end of arg */
			}
		else return(FALSE);			/* failed */
		}
	if(i1 == T0 && i2 == 2) {	/* '#' */
		nexist = TRUE;
		i+=2;
		if((*p_arg)[i] == T0 && (*p_arg)[i+1] == 1) return(FALSE);	/* #? */
		if((*p_arg)[i] != (tokenbyte) j1 || (*p_arg)[i+1] != (tokenbyte) j2)
			nefound = TRUE;
		instan[xi++] = (tokenbyte) j1; instan[xi++] = (tokenbyte) j2;
		goto NEXT;
		}
	if(i1 == T0 && i2 == 1)	{			/* '?' */
		switch(j1) {
			case T0:
				if(j2 != 8 && j2 != 9) return(FALSE);
				instan[xi++] = (tokenbyte) j1; instan[xi++] = (tokenbyte) j2;
				goto NEXT;
				break;
			case T3:	/* terminal */
			case T4:	/* variable */
			case T7:	/* out-time object or simple note */
			case T8:	/* synchronization tag */
			case T9:	/* time pattern */
			case T10:	/* performance control */
			case T11:	/* performance control */
			case T12:	/* performance control */
			case T13:	/* performance control */
			case T14:	/* performance control */
			case T15:	/* performance control */
			case T16:	/* performance control */
			case T17:	/* performance control */
			case T18:	/* performance control */
			case T19:	/* performance control */
			case T20:	/* performance control */
			case T21:	/* performance control */
			case T22:	/* performance control */
			case T23:	/* performance control */
			case T24:	/* performance control */
			case T25:	/* simple note */
			case T26:	/* performance control */
			case T27:	/* performance control */
			case T28:	/* performance control */
			case T29:	/* performance control */
			case T30:	/* performance control */
			case T31:	/* performance control */
			case T32:	/* _ins() */
			case T33:	/* performance control */
			case T34:	/* performance control */
			case T35:	/* performance control */
			case T36:	/* performance control */
				instan[xi++] = (tokenbyte) j1;
				instan[xi++] = (tokenbyte) j2;
				goto NEXT;
				break;
			default: return(FALSE);					/* not found */
			}
		}
	if(i1 == T6)	{							/* '?i2' */
		switch(j1) {
			case T0:
				if(j2 != 8 && j2 != 9) return(FALSE);
				goto OKMETA;
				break;
			case T3:	/* terminal */
			case T4:	/* variable */
			case T7:	/* out-time object or simple note */
			case T8:	/* synchronization tag */
			case T9:	/* time pattern */
			case T10:	/* performance control */
			case T11:	/* performance control */
			case T12:	/* performance control */
			case T13:	/* performance control */
			case T14:	/* performance control */
			case T15:	/* performance control */
			case T16:	/* performance control */
			case T17:	/* performance control */
			case T18:	/* performance control */
			case T19:	/* performance control */
			case T20:	/* performance control */
			case T21:	/* performance control */
			case T22:	/* performance control */
			case T23:	/* performance control */
			case T24:	/* performance control */
			case T25:	/* simple note */
			case T26:	/* performance control */
			case T27:	/* performance control */
			case T28:	/* performance control */
			case T29:	/* performance control */
			case T30:	/* performance control */
			case T31:	/* performance control */
			case T32:	/* _ins() */
			case T33:	/* performance control */
			case T34:	/* performance control */
			case T35:	/* performance control */
			case T36:	/* performance control */
OKMETA:
				i2 = i2 + i2;
				if(meta[i2] != -1)	{	/* metavariable unification */
					if(meta[i2] == (tokenbyte)j1
						&& meta[++i2] == (tokenbyte)j2) goto NEXT;
					else return(FALSE);
					}
				else {
					if(!reset) {			 /* context */
						if(meta1[i2] != -1)	{
							if(meta1[i2] == (tokenbyte)j1
								&& meta1[++i2] == (tokenbyte)j2) goto NEXT;
							else return(FALSE);
							}
						else {
							meta1[i2] = (tokenbyte)j1;
							meta1[++i2] = (tokenbyte)j2;
							goto NEXT;
							}
						}
					else {
						meta[i2] = (tokenbyte)j1;
						meta[++i2] = (tokenbyte)j2;
						goto NEXT;
						}
					}
				break;
			}
		}
	if(i1 != j1 || i2 != j2) return(FALSE);	/* not found */
NEXT:	(*p_length)++;
	}
if(grtype == SUBtype) (*p_length) = (*p_length) - (*p_lenc1); 
(*p_length) *= 2;
if(trace_compute) BPPrintMessage(0,odInfo,"Found() length = %ld grtype = %d SUBtype = %d\n",(long)(*p_length),grtype,SUBtype);
result = (!nexist || nefound);
if(ismeta && result && !reset) {	/* searching context */
	for(i=0; i< MAXMETA2; i+=2) {
		if(meta1[i] != -1) {
			meta[i] = meta1[i];		/* Copy metavariables created in context */
			meta[i+1] = meta1[i+1];
			}
		if((j1 = (int) meta[i]) != -1) {	/* Check that metavariables have ... */
			j2 = (int) meta[i+1];			/* ... distinct instances. */
			for(j=0; j < i; j+=2) {
				if(meta[j] == (tokenbyte) j1 && meta[j+1] == (tokenbyte) j2) {
					return(FALSE);
					}
				}
			}
		}
	}
return(result);
/* 'OK' if there was no negative context or at least it was defeated once */
}


long Derive(tokenbyte ***pp_a,t_gram *p_gram,tokenbyte ***pp_b,long *p_length,int igram,
	int irul,
	long pos,long *p_leftpos,int grtype,int repeat,int *p_changed,long *p_lastpos,
	long *p_incmark,int mode,int time_end_compute)
/* Apply rule 'irul' of gram 'igram'. */
/* 'pos' is the leftmost occurrence of the left argument in a[] */
{
tokenbyte **p_arg1,**p_arg2;
int imode,m,p;
unsigned long i,i1;
long pos1,offset,dif,inmark;
t_rule rule;

if(NumberInferences > MAXINFERENCES) {
	Panic = TRUE;
	BPPrintMessage(1,odError,"=> Calculation overflow: task abandoned. Loop?\n");
	return(ABORT);
	}
NumberInferences++;
if(trace_compute) BPPrintMessage(0,odInfo,"Derive() igram = %d irul = %d\n",igram,irul);
rule = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul];
switch(mode) {
	case PROD:
	case TEMP:
		p_arg1 = rule.p_leftarg; p_arg2 = rule.p_rightarg; break;
	case ANAL:
		p_arg1 = rule.p_rightarg; p_arg2 = rule.p_leftarg; break;
	}
if(p_arg1 == NULL || p_arg2 == NULL) {
	if(Beta) Alert1("Bug in Derive().  Will be fixed: recompile grammar!");
	return(ABORT);
	}
if(grtype == SUBtype) {
	offset = rule.leftoffset;
	if(!(*p_changed)) {
		for(i=ZERO,i1=1; ((*p_arg1)[i] != TEND || (*p_arg1)[i1] != TEND)
			|| ((*p_arg2)[i] != TEND || (*p_arg2)[i1] != TEND); i+=2L,i1+=2L) {
			if((*p_arg1)[i] != (*p_arg2)[i] || (*p_arg1)[i1] != (*p_arg2)[i1]) {
				*p_changed = TRUE;
				break;
				}
			}
		}
	}
else {
	offset = 0;
	}
if((grtype == RNDtype || grtype == ORDtype) && !repeat
													&& mode != ANAL) {
	imode = rule.mode;
	}
else {
	if(grtype == SUBtype) imode = 3;
	else imode = 1;	/* 'repeat' or LIN or SUB1 or POSLONG or ANAL */
	}
inmark = CountMarkers(&dif,p_arg1,p_arg2);

// BPPrintMessage(0,odInfo,"Begin Derive() pos = %ld dif = %d repeat = %d\n",(long)pos,(long)dif,repeat);
pos1 = Insert(grtype,pp_a,pp_b,rule,pos,offset,dif,p_arg1,p_arg2,p_length,p_leftpos,
	imode,inmark,p_lastpos,p_incmark,repeat,mode,time_end_compute);
// if(trace_compute)
// BPPrintMessage(0,odInfo,"End Derive() pos1 = %ld\n",(long)pos1);
return(pos1);
}


void ExpandBufferLimit(long requiredSize) {
	while(requiredSize >= (BufferSize - 2L))
		BufferSize = (BufferSize * 3L) / 2L;
	BPPrintMessage(0,odInfo,"Buffer limit expanded to %ld tokens (%ld were required)\n",(long)BufferSize,requiredSize);
	return;
	}


long Insert(int grtype,tokenbyte ***pp_origin,tokenbyte ***pp_dest,t_rule rule,long pos,
	long offset,long dif,tokenbyte **p_arg1,tokenbyte **p_arg2,long *p_lengthorigin,
	long *p_leftpos,int imode,long inmark,long *p_lastpos,long *p_incmark,
	int repeat,int mode,int time_end_compute)
{
int randomnumber;
tokenbyte m,p;
long i,ii,j,jmax,i0,j0,pos1,xi,sizedest,istart,jstart,length,length1,lenc1; // Fixed lenc1 is not Size (BB 2022-02-20)
tokenbyte *ptr1,*ptr2,posdif,instan[MAXLIN],meta[MAXMETA2],meta1[MAXMETA2];
Size oldsize,newsize,incmark,blocksize;

/* *pp_origin = *pp_dest except in 'SUB' subgrammars. */

if(*pp_dest == NULL) {
	if(Beta) Alert1("=> Err. Insert(). *pp_dest = NULL");
	return(-1L);
	}
oldsize = MyGetHandleSize((Handle)*pp_dest);
sizedest = (oldsize / sizeof(tokenbyte)) - 2L;
if((grtype != SUBtype) && (((*p_lengthorigin) + dif) >= BufferSize)) {
	if(trace_compute) 
		BPPrintMessage(0,odInfo,"ExpandBufferLimit lengthorigin = %ld dif = %ld BufferSize = %ld\n",(long)(*p_lengthorigin),(long)dif,(long)BufferSize);
	ExpandBufferLimit((*p_lengthorigin) + dif);
	}
if(((*p_lengthorigin) + dif) >= sizedest) {
	while(((*p_lengthorigin) + dif) >= sizedest) {
		sizedest = (sizedest * 3L) / 2L;
		}
	newsize = (Size)(sizedest + 2L) * sizeof(tokenbyte);
//	BPPrintMessage(0,odInfo,"MySetHandleSize %ld\n",(long) newsize);
	MySetHandleSize((Handle*)pp_dest,newsize);
	}
incmark = (*p_incmark); /* used in SUB subgrammar only */
pos1 = pos;
if(trace_compute) BPPrintMessage(0,odInfo,"Insert() pos1 = %ld\n",(long)pos1);

if(trace_compute) {
	BPPrintMessage(0,odInfo,"\nWork string:\n");
	for(i = 0; (**pp_origin)[i] != TEND || (**pp_origin)[i+1] != TEND; i+=2) {
		m = (int) (**pp_origin)[i]; p = (int)(**pp_origin)[i+1];
		BPPrintMessage(0,odInfo,"i = %ld m = %d p = %d\n",(long)i,m,p);
		}
	BPPrintMessage(0,odInfo,"\n");
	}

if(CheckEmergency() != OK) return(ABORT);

switch(imode) {
case 0:	{						/* RND rule */
		UsedRandom = TRUE;
		do {
			randomnumber = rand();
			posdif = ((*p_lengthorigin) - pos - 1);
			pos1 = pos + 2 * (int)(posdif
			* (randomnumber / ((double)RAND_MAX) / 2.));
	/*		if(!Improvize && time_end_compute > 0L && getClockTime() > time_end_compute) {
				EmergencyExit =TRUE; BPPrintMessage(0,odInfo,"\n➡ Maximum allowed time (%d seconds) has been spent in Insert(). Stopped computing...\n➡ This limit can be modified in the settings\n\n",MaxConsoleTime);
				return(ABORT);
				} */
		if(trace_compute) BPPrintMessage(0,odInfo,"Insert() randomnumber = %ld pos1 = %ld\n",(long)randomnumber,(long)pos1);
			}
		while(!Found(pp_origin,grtype,p_arg1,offset,rule.leftnegcontext,&lenc1,pos1,
			1,instan,meta,meta1,&istart,&jstart,&length,rule.ismeta,time_end_compute) ||
			!OkContext(pp_origin,grtype,rule,pos1,length,meta,instan,PROD,time_end_compute));
		break;
		}
case 1:	{ 			/* ANAL or LEFT rule or LIN or ORD or SUB1 or POSLONG grammar */
		if(!Found(pp_origin,grtype,p_arg1,offset,rule.leftnegcontext,&lenc1,pos1,1,
			instan,meta,meta1,&istart,&jstart,&length,rule.ismeta,time_end_compute)) {
			if(Beta) Alert1("=> Err Insert().  Not found");
			return(ABORT);
			}
		if(!OkContext(pp_origin,grtype,rule,pos1,length,meta,instan,mode,time_end_compute)) {
			if(Beta) Alert1("=> Err Insert().  Not OkContext");
			return(ABORT);
			}
		/* this was necessary for loading instan[] and meta[] */
		break;
		}
case 2:	{								/* RIGHT rule */
		pos1 = (*p_lengthorigin) - 2;
		while(!Found(pp_origin,grtype,p_arg1,offset,rule.leftnegcontext,&lenc1,pos1,
			1,instan,meta,meta1,&istart,&jstart,&length,rule.ismeta,time_end_compute)
			|| !OkContext(pp_origin,grtype,rule,pos1,length,meta,instan,
				PROD,time_end_compute)) {
				pos1 -= 2;
				if(pos1 < 0) {
					if(Beta) Alert1("=> Err. Insert(). pos1 < 0");
					return(ABORT);
					}
				}
		break;
		}
case 3:	{								/* SUB grammar */
		if(!Found(pp_origin,grtype,p_arg1,offset,rule.leftnegcontext,&lenc1,pos1,1,
			instan,meta,meta1,&istart,&jstart,&length,rule.ismeta,time_end_compute)) {
			if(Beta) Alert1("=> Err Insert().  Not found");
			return(ABORT);
			}
		if(!OkContext(pp_origin,grtype,rule,pos1,length,meta,instan,PROD,time_end_compute)) {
			if(Beta) Alert1("=> Err Insert().  Not OkContext");
			return(ABORT);
			}
		Countmark(rule.p_leftarg,&length1); /* Only to calculate length1 */
		*p_leftpos +=
			length1 - (2 * rule.leftnegcontext) - offset - rule.rightoffset;
				/* Rem: offset = rule.leftoffset in SUB subgrammar */
				/* *p_leftpos used only in SUB subgrammars anyway. */
		break;
		}
	}

xi = 0;
jmax = jstart + length + dif - 1;
// jmax = jstart + length + dif; // Maybe?

if(grtype == SUBtype) {	/* Don't rewrite leftmost contexts */ 
	istart += rule.leftoffset + 4L * lenc1;
	jstart = (*p_lastpos);
	jmax = jstart + length1 - (2 * rule.leftnegcontext) + dif - rule.leftoffset
				- rule.rightoffset;
	}

if(jmax >= (BufferSize - 2L)) ExpandBufferLimit(jmax);

if(trace_compute) BPPrintMessage(0,odInfo,"jstart = %ld jmax = %ld pos1 = %ld length = %ld lenc1 = %ld dif = %ld\n",(long)jstart,(long)jmax,(long)pos1,(long)length,(long)lenc1,(long)dif);

if(grtype != SUBtype) {
	i0 = pos1 + length - (2 * lenc1);
	j0 = i0 + dif;
#if ! _FASTCODE
	if(dif < 0) {
		for(i=i0,j=j0; (**pp_dest)[i] != TEND || (**pp_dest)[i+1] != TEND; i+=2,j+=2) {
			(**pp_dest)[j] = (**pp_dest)[i];
			(**pp_dest)[j+1] = (**pp_dest)[i+1];
			}
		for(i=j; (**pp_dest)[i] != TEND || (**pp_dest)[i+1] != TEND; i+=2)
			(**pp_dest)[i] = (**pp_dest)[i+1] = TEND;
		}
	else {
		if(dif > 0) {
			i = i0;
			while((**pp_dest)[i] != TEND || (**pp_dest)[i+1] != TEND) i+=2;
			for(j=i+dif; i >= i0; i-=2,j-=2) {
				(**pp_dest)[j] = (**pp_dest)[i];
				(**pp_dest)[j+1] = (**pp_dest)[i+1];
				}
			}
		}
#else
	if(dif != 0) {	/* This is a quicker procedure */
		if(trace_compute) 
			BPPrintMessage(0,odError,"Moving memory quickly dif = %ld\n",(long)dif);
		blocksize = (LengthOf(pp_dest) - i0 + 2) * sizeof(tokenbyte);
		ptr1 = &(**pp_dest)[i0]; ptr2 = &(**pp_dest)[j0];
		memmove(ptr2, ptr1, blocksize);
		}
#endif
	}

if(trace_compute) {
	BPPrintMessage(0,odInfo,"\nRight argument of rule:\n");
	for(i = 0; (*p_arg2)[i] != TEND || (*p_arg2)[i+1] != TEND; i+=2) {
		m = (int) (*p_arg2)[i]; p = (int)(*p_arg2)[i+1];
		BPPrintMessage(0,odInfo,"i = %ld m = %d p = %d\n",(long)i,m,p);
		}
	BPPrintMessage(0,odInfo,"\n");
	}
	
if(trace_compute) BPPrintMessage(0,odInfo,"istart = %ld jstart = %d\n",(long)istart,(long)jstart);
for(i=istart,j=jstart; j < jmax; i+=2,j+=2) {
	if(trace_compute) BPPrintMessage(0,odInfo,"wild cards? i = %d j = %d jmax = %d xi = %d\n",i,j,jmax,xi);
	if(xi > MAXLIN - 2) {
		my_sprintf(Message,"Too many wild cards in a rule argument. Not more than %ld allowed.\n",
			(long)(MAXLIN / 2 - 1));
		Print(wTrace,Message);
		return(ABORT);
		}
	// if((*p_arg2)[i] == TEND && (*p_arg2)[i+1] == TEND) break;  // Maybe for security
	if((*p_arg2)[i] == T0 && (*p_arg2)[i+1] == 2) {		/* '#' */
		m = (**pp_dest)[j] = instan[xi++];
		p = (**pp_dest)[j+1] = instan[xi++];
		if(trace_compute) BPPrintMessage(0,odInfo,"wild card '#' i = %d j = %d m = %d p = %d xi = %d\n",i,j,m,p,xi);
		i += 2;
		}
	else {
		if((*p_arg2)[i] == T0 && (*p_arg2)[i+1] == 1) {		/* '?' */
			m = (**pp_dest)[j] = instan[xi++];
			p = (**pp_dest)[j+1] = instan[xi++];
			}
		else {
			if((*p_arg2)[i] == T6) {					/* '?ii' */
				ii = (int) (*p_arg2)[i+1];
				ii += ii;
				m = (**pp_dest)[j] = meta[ii];
				p = (**pp_dest)[j+1] = meta[++ii];
				}
			else {
				m = (**pp_dest)[j] = (*p_arg2)[i];
				p = (**pp_dest)[j+1] = (*p_arg2)[i+1];
				if(grtype == SUBtype) {
					if(m == T2 && p == 0) {		/* "(=" */
						incmark++;
						}
					if(m == T2 && p != 0) {		/* "(:" */
						(**pp_dest)[j+1] += (tokenbyte)(*p_incmark);
						}
					}
				}
			}
		}
	if(trace_compute) BPPrintMessage(0,odInfo,"p_arg2[%d] m = %d p = %d\n",j,m,p);
	if((m == T12 && (p == 21 || p == 22 || p == 24)) || m == T39)
		NeedZouleb++;
	}
					
if(grtype != SUBtype) { /* Not in SUB subgrammar */
	(*p_lengthorigin) += dif;
	Cormark(pp_dest,pos1+length+dif,inmark);
	}
else {
	(**pp_dest)[j++] = TEND;
	(**pp_dest)[j] = TEND;
	(*p_lastpos) = jmax;
	(*p_incmark) = incmark;
	}
if(trace_compute) {
	BPPrintMessage(0,odInfo,"\nBuffer after:\n");
	for(i = 0; (**pp_dest)[i] != TEND || (**pp_dest)[i+1] != TEND; i+=2) {
		m = (int) (**pp_dest)[i]; p = (int)(**pp_dest)[i+1];
		BPPrintMessage(0,odInfo,"i = %ld m = %d p = %d\n",(long)i,m,p);
		}
	BPPrintMessage(0,odInfo,"\n");
	}
return(pos1);
}


int Cormark(tokenbyte ***pp_a,long from,long inmark)
/* Recalculate slave markers after derivation... */
// Checked by BB, 6 Nov 2020, with -gr.dhin-- seems OK
// But "from" is too large when work string does not contain markers…
{
long i;
tokenbyte m,p,q;		/* 'from' is the position of rightmost symbol inserted */

for(i = from, q = 1; (**pp_a)[i] != TEND || (**pp_a)[i+1] != TEND; i+=2) {
	m = (int) (**pp_a)[i];
	if(m >= MAXTOKENBYTE) {
		if(trace_compute) BPPrintMessage(0,odError,"=> Error in Cormark() from = %ld inmark = %d\n",(long)from,inmark);
		return(MISSED);
		}
	p = (int)(**pp_a)[i+1];
	if(trace_compute) BPPrintMessage(0,odInfo,"Cormark() i = %ld m = %d p =%d q = %ld\n",(long)i,m,p,(long)q);
	if(m == 2 && p == 0) q++; /* zero marker */
	else if(m == 2 && p >= q) (**pp_a)[i+1] = (tokenbyte)(p + inmark);
	}
return(OK);
}


long CountMarkers(long *p_dif,tokenbyte **p_arg1,tokenbyte **p_arg2)
/* How many zero markers are created by this derivation ? */
{
long n1,n2;
long l1,l2;

n1 = Countmark(p_arg1,&l1);
n2 = Countmark(p_arg2,&l2);
(*p_dif) = l2 - l1;		/* also return how much length of buffer */
return(n2-n1);						/* will be increased */
}


long Countmark(tokenbyte **p_arg,long *p_length)
{
long i,len,k;
tokenbyte m,p;

if(p_arg == NULL) {
	if(Beta) Alert1("=> Err. Countmark()");
	return(0);
	}
for(i=k=len=0; ; i+=2,len++) {
	m = (int) (*p_arg)[i]; p = (int) (*p_arg)[i+1];
	if(m == TEND && p == TEND) break;
 	if(m == T2 && p == 0) k++;		/* zero marker */
 	if(m == T0 && p == 2) {	
 		len--;					/* Don't count '#' */
 		}
 	}
*p_length = len + len;
return(k);
}


int ChangeFlagsInRule(t_gram *p_gram,int igram,int irul)
{
int rep;
p_flaglist **h;

rep = OK;
h = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].p_leftflag;
if(h != NULL) {
	do {
		if((**h).x > Jflag) {
			if(Beta) Alert1("=> Err. p_Swtch. ");
			rep = ABORT;
			goto QUIT;
			}
		/* Increment flag used to activate rule */
		if((**h).x > 0) (*p_Flag)[(**h).x] += (**h).increment;
		h = (**h).p;
		}
	while(h != NULL);
	}
h = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].p_rightflag;
if(h != NULL) {
	do {					/* Change flags created by rule */
		if((**h).x > Jflag) {
			if(Beta) Alert1("=> Err. p_Swtch. ");
			rep = ABORT;
			goto QUIT;
			}
		if((**h).operator == ASSIGN) {
			if((**h).paramcontrol == 1) (*p_Flag)[(**h).x]
												= ParamValue[(**h).refvalue];
			else if((**h).paramcontrol == 2) (*p_Flag)[(**h).x]
												= (*p_Flag)[(**h).refvalue];
			else (*p_Flag)[(**h).x] = (**h).refvalue;
			}
		else {
			if((*p_Flag)[(**h).x] < INT_MAX) (*p_Flag)[(**h).x] += (**h).increment;
			else {
				my_sprintf(Message,"Too many /%s/ flags produced.",
													*((*p_Flagname)[(**h).x]));
				if(!ScriptExecOn) Alert1(Message);
				else PrintBehindln(wTrace,Message);
				(*p_Flag)[(**h).x] = ZERO;
				}
			}
		h = (**h).p;
		}
	while(h != NULL);
	}

QUIT:
return(rep);
}


int ShowItem(int igram,t_gram *p_gram,int justplay,tokenbyte ***pp_a,int repeat,
	int mode,int all)
{
int r,rep,ifunc,datamode,hastabs;
long lastbyte;

if(justplay) {
	r = PlayBuffer(pp_a,NO);
	goto QUIT;
	}
if(TraceProduce) {
	datamode = DisplayMode(pp_a,&ifunc,&hastabs);
	if(mode == ANAL || all) datamode = FALSE;
	Dirty[wTrace] = TRUE;
	BPActivateWindow(QUICK,wTrace);
	if(p_ItemStart == NULL
			|| DisplayStackIndex >= MyGetHandleSize((Handle)p_ItemStart) / sizeof(long)) {
		if(Beta) Alert1("=> Err. ShowItem(). p_ItemStart = NULL");
		return(OK);
		}
	(*p_ItemStart)[DisplayStackIndex] = lastbyte = GetTextLength(wTrace);
	SetSelect(lastbyte,lastbyte,TEH[wTrace]);
	if(NumberCharsTrace < MAXCHARTRACE) {
		if((r=PrintResult(datamode && hastabs,wTrace,hastabs,ifunc,pp_a)) != OK) goto QUIT;
		Print(wTrace,"\n");
		NumberCharsTrace += 20;
		}
	(*p_ItemEnd)[DisplayStackIndex] = GetTextLength(wTrace);
	(*p_LastStackIndex)[DisplayStackIndex] = ProduceStackIndex;
	if(++DisplayStackIndex >= MaxDeriv
		&& IncreaseComputeSpace() != OK) return(ABORT);
	ShowSelect(CENTRE,wTrace);
	}
if(mode == PROD && PlanProduce && (DisplayStackIndex > 1)) UndoFlag = TRUE;
r = OK;

QUIT:
#if BP_CARBON_GUI_FORGET_THIS
// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
// StepProduce should be revised as an API call that returns to host! 
if((rep=MyButton(2)) != MISSED || StepProduce) {
	if(rep == OK || StepProduce) return(InterruptCompute(igram,p_gram,repeat,0,mode));
	r = rep;
	}
if(EventState != NO) r = EventState;
#endif /* BP_CARBON_GUI_FORGET_THIS */
return(r);
}


#if BP_CARBON_GUI_FORGET_THIS
int InterruptCompute(int igram,t_gram *p_gram,int repeat,int grtype,int mode)
{
long lastbyte;
int r,rep;
unsigned long datemem;

StopCount(0);

Interrupted = TRUE;
datemem = CompileDate;
if(StepProduce) {
	if(UndoFlag) {
		ShowWindow(GetDialogWindow(ResumeUndoStopPtr));
		BringToFront(GetDialogWindow(ResumeUndoStopPtr));
		}
	else {
		ShowWindow(GetDialogWindow(ResumeStopPtr));
		BringToFront(GetDialogWindow(ResumeStopPtr));
		}
	}
while((r = MainEvent()) != RESUME && r != STOP && r != ABORT && r != EXIT
											&& !(StepProduce && r == UNDO)){};
UndoFlag = FALSE;
lastbyte = GetTextLength(wTrace);
SetSelect(lastbyte,lastbyte,TEH[wTrace]);
if(Nw >= 0 && Nw < WMAX && Editable[Nw]) {
	lastbyte =  GetTextLength(Nw);
	SetSelect(lastbyte,lastbyte,TEH[Nw]);
	}
if(r == ABORT || r == EXIT) return(r);
if((datemem != CompileDate) || !CompiledGr || !CompiledPt) {
	Alert1("Grammar changed or recompiled. Must abort...");
	return(ABORT);
	}
if(!ComputeOn) return(ABORT); /* Happens if repeating */
if(LoadedIn && (!CompiledIn && (r=CompileInteraction()) != OK))
	return(r);
if(r == STOP && mode == PROD) {
	if(repeat) {
		StepProduce = StepGrammars = DisplayProduce
			= PlanProduce = TraceProduce = FALSE;
		r = OK;	/* Computation should be finished. */
		}
	else {
		if(!Improvize
			&& (igram >= 0 && igram < (*p_gram).number_gram) && !AllOn) {
				my_sprintf(Message,"Jump to subgrammar #%ld\nand terminate computation",
					(long)igram + 1L);
			if((r=Answer(Message,'Y')) == OK) {
				TextOffset dummy, selend;
				r = FINISH;
				TextGetSelection(&dummy, &selend, TEH[wTrace]);
				SetSelect(selend,selend,TEH[wTrace]);
				if(StepProduce
					&& ((rep=Answer("Terminate step by step",'N')) != OK)) {
					if(rep == ABORT) return(AGAIN);
					StepProduce = PlanProduce = TraceProduce = FALSE;
					}
				if(!StepProduce && DisplayProduce && ((rep=Answer("Show computation",'N'))
						!= OK)) {
					if(rep == ABORT) {
						StepProduce = PlanProduce = TraceProduce = TRUE;
						 return(AGAIN);
						}
					DisplayProduce = FALSE;
					}
				}
			else {
				if(r == ABORT) return(AGAIN);
				if(r == NO) r = STOP;
				if(Improvize && SkipFlag && (grtype == SUBtype)) {
					SkipFlag = FALSE;
					r = OK;
					}
				else r = ABORT;
				}
			}
		}
	}
if(r == STOP) r = ABORT;
if(r == RESUME) {
	r = OK; EventState = NO;
	}
PleaseWait();
return(r);
}
#endif /* BP_CARBON_GUI_FORGET_THIS */
