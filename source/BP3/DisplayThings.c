/* DisplayThings.c (BP3) */

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

int trace_display = 0;

int Print(int w,char* t)
{
long length;
char *ptr;

if(w < 0 || w >= WMAX) {
	BPPrintMessage(0,odError,"=> Err1. Print()");
	return(OK);
	}
if(!Editable[w]) {
	BPPrintMessage(0,odError,"=> Err2. Print()");
	return(MISSED);
	}
length = strlen(t);
BPActivateWindow(QUICK,w);
ptr = t;
TextInsert(ptr,length,TEH[w]);
if(LockedWindow[w]) Deactivate(TEH[w]);
CheckTextSize(w);
w = FindGoodIndex(w);
if(NeedSave[w]) Dirty[w] = TRUE;
return(OK);
}


int PrintHandle(int w,char** p_t)
{
long length;

if(w < 0 || w >= WMAX) {
	BPPrintMessage(0,odError,"=> Err1. Print()");
	return(OK);
	}
if(!Editable[w]) {
	BPPrintMessage(0,odError,"=> Err2. Print()");
	return(MISSED);
	}
if(p_t == NULL) {
	BPPrintMessage(0,odError,"=> Err. PrintHandle(). p_t == NULL");
	return(OK);
	}
length = MyHandleLen(p_t);
BPActivateWindow(QUICK,w);
MyLock(FALSE,(Handle)p_t);
TextInsert(*p_t,length,TEH[w]);
MyUnlock((Handle)p_t);
CheckTextSize(w);
if(LockedWindow[w]) Deactivate(TEH[w]);
w = FindGoodIndex(w);
if(NeedSave[w]) Dirty[w] = TRUE;
return(OK);
}


int PrintHandleBehind(int w,char** p_t)
{
long length;

if(w < 0 || w >= WMAX) {
	BPPrintMessage(0,odError,"=> Err1. PrintHandleBehind()");
	return(OK);
	}
if(!Editable[w]) {
	BPPrintMessage(0,odError,"=> Err2. PrintHandleBehind()");
	return(MISSED);
	}
if(p_t == NULL) {
	BPPrintMessage(0,odError,"=> Err. PrintHandleBehind(). p_t == NULL");
	return(OK);
	}
length = MyHandleLen(p_t);
MyLock(FALSE,(Handle)p_t);
TextInsert(*p_t,length,TEH[w]);
MyUnlock((Handle)p_t);
CheckTextSize(w);
if(LockedWindow[w]) Deactivate(TEH[w]);
w = FindGoodIndex(w);
if(NeedSave[w]) Dirty[w] = TRUE;
return(OK);
}


int PrintHandleln(int w,char** p_t)
{
PrintHandle(w,p_t);
Print(w,"\n");
return(OK);
}


int Println(int w,char* t)
{
Print(w,t);
Print(w,"\n");
return(OK);
}


int PrintBehind(int w,char* t)
{
long length;
char *ptr;

if(w < 0 || w >= WMAX) {
	BPPrintMessage(0,odError,"=> Err1. Print()");
	return(OK);
	}
if(!Editable[w]) {
	BPPrintMessage(0,odError,"=> Err2. Print()");
	return(MISSED);
	}
length = strlen(t);
ptr = t;
TextInsert(ptr,length,TEH[w]);
CheckTextSize(w);
if(LockedWindow[w]) Deactivate(TEH[w]);
w = FindGoodIndex(w);
if(NeedSave[w]) Dirty[w] = TRUE;
return(OK);
}


int PrintBehindln(int w,char* t)
{
PrintBehind(w,t);
PrintBehind(w,"\n");
return(OK);
}


int DisplayGrammar(t_gram *p_gram,int wind,int producemode,int showweights,int isgrammar)
{
int i,igram,irul,inc,j,proc,w,splitmem,shownctrlval[MAXPARAMCTRL];
char *ptr;
p_context pp_ctxt;
p_flaglist **h;
long starttrace;
t_subgram subgram;
double p,q;

if(wind < 0 || wind >= WMAX || !Editable[wind])  {
	BPPrintMessage(0,odError,"=> Err. DisplayGrammar(). Incorrect index");
	return(MISSED);
	}
if(p_gram->p_subgram == NULL) {
	BPPrintMessage(0,odError,"=> Err. DisplayGrammar(). Grammar is empty");
	return(MISSED);
	}
//if(isgrammar && !producemode) UseTextColor = TRUE;
for(i=1; i < MAXPARAMCTRL; i++) shownctrlval[i] = FALSE;
splitmem = SplitVariables;
BPActivateWindow(SLOW,wind);
starttrace = GetTextLength(wind);
SetSelect(starttrace,starttrace,TEH[wind]);
// Reformat(wind,-1,-1,(int) 0,&Black,NO,NO);
Print(wind,"\n");
if(isgrammar) {
	if(InitThere > 0) {
		// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
		Print(wind,InitToken);
		Print(wind," ");
		// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
		}
	switch(InitThere) {
		case 1:
			my_sprintf(Message,"%s  ",*(p_ScriptLabelPart(85,0)));
			Print(wind,Message);
			PrintArg(FALSE,FALSE,0,FALSE,0,0,stdout,wind,pp_Scrap,&p_Initbuff);
			Print(wind,"\n");
			break;
		case 2:
			if(p_InitScriptLine != NULL) Println(wind,(*p_InitScriptLine));
			break;
		}
	}
else {
	Print(wind,"// ------------ GLOSSARY ------------\n\n");
	}
for(igram=1; igram <= (*p_gram).number_gram; igram++) {
	my_sprintf(Message,"\n------------- SUBGRAMMAR #%ld --------------\n",
		(long)igram);
	if(isgrammar && igram > 1) Print(wind,Message);
	subgram = (*((*p_gram).p_subgram))[igram];
	if(isgrammar && subgram.number_rule == 0) {
		my_sprintf(Message,"Subgrammar %ld has no rules...",(long)igram);
		BPPrintMessage(0,odError,"%s",Message);
		continue;
		}
	irul = 0;
	// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
	if(isgrammar) Println(wind,SubgramType[subgram.type]);
	if(igram == 1) {
		if(Simplify((double)INT_MAX,(double)60L*Qclock,Pclock,&p,&q) != OK)
			Simplify((double)INT_MAX,Qclock,floor((double)Pclock/60.),&p,&q);
		my_sprintf(Message,"%s(%.4f) ",*((*p_GramProcedure)[13]),((double)p)/q);
		Print(wind,Message);
		if(Nature_of_time == STRIATED)
			my_sprintf(Message,"%s",*((*p_GramProcedure)[14]));
		else
			my_sprintf(Message,"%s",*((*p_GramProcedure)[15]));
		Println(wind,Message);
		}
	proc = FALSE;
	if(subgram.stop) {
		my_sprintf(Message,"%s ",*((*p_GramProcedure)[3]));
		Print(wind,Message); proc = TRUE;
		}
	if(subgram.print) {
		my_sprintf(Message,"%s ",*((*p_GramProcedure)[4]));
		Print(wind,Message); proc = TRUE;
		}
	if(subgram.printon) {
		my_sprintf(Message,"%s ",*((*p_GramProcedure)[5]));
		Print(wind,Message); proc = TRUE;
		}
	if(subgram.printoff) {
		my_sprintf(Message,"%s ",*((*p_GramProcedure)[6]));
		Print(wind,Message); proc = TRUE;
		}
	if(subgram.stepon) {
		my_sprintf(Message,"%s ",*((*p_GramProcedure)[7]));
		Print(wind,Message); proc = TRUE;
		}
	if(subgram.stepoff) {
		my_sprintf(Message,"%s ",*((*p_GramProcedure)[8]));
		Print(wind,Message); proc = TRUE;
		}
	if(subgram.traceon) {
		my_sprintf(Message,"%s ",*((*p_GramProcedure)[9]));
		Print(wind,Message); proc = TRUE;
		}
	if(subgram.traceoff) {
		my_sprintf(Message,"%s ",*((*p_GramProcedure)[10]));
		Print(wind,Message); proc = TRUE;
		}
	if(subgram.destru) {
		my_sprintf(Message,"%s ",*((*p_GramProcedure)[11]));
		Print(wind,Message); proc = TRUE;
		}
	if(subgram.randomize) {
		my_sprintf(Message,"%s ",*((*p_GramProcedure)[12]));
		Print(wind,Message); proc = TRUE;
		}
	if(subgram.seed > -1) {
		my_sprintf(Message,"%s(%ld) ",*((*p_GramProcedure)[16]),(long)subgram.seed);
		Print(wind,Message); proc = TRUE;
		}
	irul++; if(proc) Print(wind,"\n");
	for(irul=irul; irul <= subgram.number_rule; irul++) {
		PleaseWait();
		if(ShowRule(p_gram,igram,irul,wind,producemode,shownctrlval,FALSE,
			showweights,isgrammar) != OK) goto END;
		}
	}
// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
if(Jpatt > 0) {
	Print(wind,"\nTIMEPATTERNS:\n");
	for(j=0; j < Jpatt; j++) {
		my_sprintf(Message,"%s = %ld/%ld  ",*((*p_Patt)[j]),(long)(*p_Ppatt)[j],
			(long)(*p_Qpatt)[j]);
		Print(wind,Message);
		}
	Print(wind,"\n");
	}
my_sprintf(Message,"// %ld error(s)\n",(long)N_err);
Print(wind,Message);

END:
SetSelect(starttrace,GetTextLength(wind),TEH[wind]);
ShowSelect(CENTRE,wind);
SplitVariables = splitmem;
return(OK);
}


int ShowRule(t_gram *p_gram,int igram,int irul,int wind,int producemode,
	int* shownctrlval,int showgram,int showweights,int showmode)
{
t_rule rule;
p_context pp_ctxt;
TextHandle teh;
int w,i,ig,ir,inc,j,r,s;
p_flaglist **h;
tokenbyte **ptr;

if(wind < 0 || wind >= WMAX || !Editable[wind]) return(MISSED);
if((*p_gram).p_subgram == NULL) {
	BPPrintMessage(0,odError,"=> Err. DisplayGrammar(). Grammar is empty");
	return(MISSED);
	}
rule = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul];
teh = TEH[wind];
Dirty[wind] = TRUE;
// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
if(showgram) {
	my_sprintf(Message,"gram#%ld[%ld]",(long)igram,(long)irul);
	Print(wind,Message);
	}
if(showweights) {
	if(!showgram) {
		my_sprintf(Message,"[%ld]",(long)irul);
		Print(wind,Message);
		// Reformat(wind,-1,-1,(int) 0,&None,NO,NO); 
		}
	inc = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].incweight;
	if(rule.ctrl > 0) {
		Print(wind," <");
		// Reformat(wind,-1,-1,-1,&Color[ControlC],NO,NO);
		my_sprintf(Message,"K%ld",(long)(rule.ctrl));
		Print(wind,Message);
		// Reformat(wind,-1,-1,-1,&Black,NO,NO);
		if(shownctrlval != NULL && !shownctrlval[rule.ctrl] && inc == 0) {
			shownctrlval[rule.ctrl] = TRUE;
			my_sprintf(Message,"=%ld",(long)ParamInit[rule.ctrl]);
			Print(wind,Message);
			}
		}
	else {
		w = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].weight;
		if(w < INT_MAX) {
			my_sprintf(Message," <%ld",(long)w);
			Print(wind,Message);
			}
		else {
			my_sprintf(Message," <�");
			Print(wind,Message);
			}
		}
	if(inc > 0) my_sprintf(Message,"+%ld",(long)inc);
	if(inc < 0) my_sprintf(Message,"-%ld",(long)-inc);
	if(inc != 0) Print(wind,Message);
	if(producemode) {
		my_sprintf(Message,":%ld",
				(long)((*((*((*p_gram).p_subgram))[igram].p_rule))[irul].w));
		Print(wind,Message);
		}
	Print(wind,"> ");
	}
else {
	if(!showgram) my_sprintf(Message,"[%ld] ",(long)irul);
	else my_sprintf(Message," ");
	Print(wind,Message);
	// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
	}
if(showmode) {
	// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
	my_sprintf(Message,"%s ",Mode[(*((*((*p_gram).p_subgram))[igram].p_rule))[irul].mode]);
	Print(wind,Message);
	// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
	}
if(Beta && 0) {
	my_sprintf(LineBuff,"lo=%ld ",
		(long)((*((*((*p_gram).p_subgram))[igram].p_rule))[irul].leftoffset));
	Print(wind,LineBuff);
	my_sprintf(LineBuff,"ro=%ld ",
		(long)((*((*((*p_gram).p_subgram))[igram].p_rule))[irul].rightoffset));
	Print(wind,LineBuff);
	my_sprintf(LineBuff,"lenc=%ld ",
		(long)((*((*((*p_gram).p_subgram))[igram].p_rule))[irul].leftnegcontext));
	Print(wind,LineBuff);
	}
// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
if(rule.stop == 1 || rule.stop == 3) {
	my_sprintf(Message,"%s ",*((*p_GramProcedure)[3]));
	Print(wind,Message);
	}
if(rule.print == 1 || rule.print == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[4]));
	Print(wind,Message);
	}
if(rule.printon == 1 || rule.printon == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[5]));
	Print(wind,Message);
	}
if(rule.printoff == 1 || rule.printoff == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[6]));
	Print(wind,Message);
	}
if(rule.stepon == 1 || rule.stepon == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[7]));
	Print(wind,Message);
	}
if(rule.stepoff == 1 || rule.stepoff == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[8]));
	Print(wind,Message);
	}
if(rule.traceon == 1 || rule.traceon == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[9]));
	Print(wind,Message);
	}
if(rule.traceoff == 1 || rule.traceoff == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[10]));
	Print(wind,Message);
	}
// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
h = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].p_leftflag;
if(h != NULL) {
	do {
		if((s=(**h).x) > Jflag || s < 0) {
			my_sprintf(Message,"=> Err in flag list");
			BPPrintMessage(0,odError,"%s",Message);
			r = ABORT; goto END;
			}
		// Reformat(wind,-1,-1,-1,&Color[FlagC],NO,NO);
		if(producemode)
			my_sprintf(Message," /%s: %ld/ ",*((*p_Flagname)[s]),(long)(*p_Flag)[s]);
		else {
			my_sprintf(Message," /%s/ ",*((*p_Flagname)[s]));
			if((**h).increment > 0)
				my_sprintf(Message," /%s +%ld/ ",*((*p_Flagname)[s]),
					(long)((**h).increment));
			if((**h).increment < 0)
				my_sprintf(Message," /%s -%ld/ ",*((*p_Flagname)[s]),
					(long)(-(**h).increment));
			if((**h).operator != OFF) {
				switch((**h).operator) {
					case EQUAL:
						my_sprintf(Message," /%s = ",*((*p_Flagname)[s]));
						break;
					case DIF:
						my_sprintf(Message," /%s � ",*((*p_Flagname)[s]));
						break;
					case INFEQUAL:
						my_sprintf(Message," /%s � ",*((*p_Flagname)[s]));
						break;
					case SUPEQUAL:
						my_sprintf(Message," /%s � ",*((*p_Flagname)[s]));
						break;
					case INF:
						my_sprintf(Message," /%s < ",*((*p_Flagname)[s]));
						break;
					case SUP:
						my_sprintf(Message," /%s > ",*((*p_Flagname)[s]));
						break;
					}
				Print(wind,Message);
				switch((**h).paramcontrol) {
					case 1: my_sprintf(Message,"K%ld/ ",
						(long)((**h).refvalue));
						break;
					case 2: my_sprintf(Message,"%s/ ",
												*((*p_Flagname)[(**h).refvalue]));
							break;
					default: my_sprintf(Message,"%ld/ ",(long)((**h).refvalue));
					}
				}
			}
		Print(wind,Message);
		// Reformat(wind,-1,-1,-1,&Black,NO,NO);
		h = (**h).p;
		}
	while(h != NULL);
	}
if((pp_ctxt = rule.p_leftcontext) != NULL) {
	// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
	if((*pp_ctxt)->sign == 0) {
		my_sprintf(Message," #(");
		}
	else {
		my_sprintf(Message," (");
		}
	Print(wind,Message);
	// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
	ptr = (*pp_ctxt)->p_arg;
	if(PrintArg(FALSE,FALSE,0,FALSE,0,0,stdout,wind,pp_Scrap,&ptr) != OK) {
		r = ABORT; goto END;
		}
	// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
	Print(wind,")");
	// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
	}
ptr = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].p_leftarg;
if(PrintArg(FALSE,FALSE,0,FALSE,0,0,stdout,wind,pp_Scrap,&ptr) != OK) {
	r = ABORT; goto END;
	}
if((pp_ctxt =
	(*((*((*p_gram).p_subgram))[igram].p_rule))[irul].p_rightcontext) != NULL) {
	// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
	if((*pp_ctxt)->sign == 0) {
		my_sprintf(Message," #(");
		}
	else {
		my_sprintf(Message," (");
		}
	Print(wind,Message);
	// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
	ptr = (*pp_ctxt)->p_arg;
	if(PrintArg(FALSE,FALSE,0,FALSE,0,0,stdout,wind,pp_Scrap,&ptr) != OK) {
		r = ABORT; goto END;
		}
	
	// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
	Print(wind,")");
	}
// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
my_sprintf(Message," %s ",Arrow[(*((*((*p_gram).p_subgram))[igram].p_rule))[irul].operator]);
Print(wind,Message);
ptr = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].p_rightarg;
if((*ptr)[0] == TEND && (*ptr)[1] == TEND) {
	Print(wind,"lambda");
	}
else {
	// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
	if(PrintArg(FALSE,FALSE,0,FALSE,0,0,stdout,wind,pp_Scrap,&ptr) != OK) {
		r = ABORT; goto END;
		}
	}
// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
h = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].p_rightflag;
if(h != NULL) {
	do {
		if((**h).x > Jflag || (**h).x < 0) {
			my_sprintf(Message,"=> Err in flag list. ");
			BPPrintMessage(0,odError,"%s",Message);
			r = ABORT; goto END;
			}
		// Reformat(wind,-1,-1,-1,&Color[FlagC],NO,NO);
		if((**h).operator == ASSIGN) {
			if((**h).paramcontrol == 1)
				my_sprintf(Message," /%s = K%ld/",*((*p_Flagname)[(**h).x]),
					(long)((**h).refvalue));
			else if((**h).paramcontrol == 2) my_sprintf(Message," /%s = %s/ ",
							*((*p_Flagname)[(**h).x]),*((*p_Flagname)[(**h).refvalue]));
			else my_sprintf(Message," /%s = %ld/",*((*p_Flagname)[(**h).x]),
				(long)((**h).refvalue));
			}
		else {
			if((**h).increment > 0) my_sprintf(Message," /%s +%ld",*((*p_Flagname)[(**h).x]),
				(long)((**h).increment));
			else my_sprintf(Message," /%s -%ld",*((*p_Flagname)[(**h).x]),
				(long)(-(**h).increment));
			Print(wind,Message);
			if(producemode) {
			my_sprintf(Message,": %ld/",(long)(*p_Flag)[(**h).x]);
				}
			else my_sprintf(Message,"/");
			}
		Print(wind,Message);
		// Reformat(wind,-1,-1,-1,&Black,NO,NO);
		h = (**h).p;
		}
	while(h != NULL);
	}
// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
if(rule.stop == 2 || rule.stop == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[3]));
	Print(wind,Message);
	}
if(rule.print == 2 || rule.print == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[4]));
	Print(wind,Message);
	}
if(rule.printon == 2 || rule.printon == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[5]));
	Print(wind,Message);
	}
if(rule.printoff == 2 || rule.printoff == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[6]));
	Print(wind,Message);
	}
if(rule.stepon == 2 || rule.stepon == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[7]));
	Print(wind,Message);
	}
if(rule.stepoff == 2 || rule.stepoff == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[8]));
	Print(wind,Message);
	}
if(rule.traceon == 2 || rule.traceon == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[9]));
	Print(wind,Message);
	}
if(rule.traceoff == 2 || rule.traceoff == 3) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[10]));
	Print(wind,Message);
	}
if(rule.destru) {
	my_sprintf(Message," %s ",*((*p_GramProcedure)[11]));
	Print(wind,Message);
	}
if((i=rule.repeat) > 0) {
	if(rule.repeatcontrol > 0) {
		my_sprintf(Message," %s(",*((*p_GramProcedure)[2]));
		Print(wind,Message); // Reformat(wind,-1,-1,(int) 0,&Color[ControlC],NO,NO);
		my_sprintf(Message,"K%ld",(long)(rule.repeatcontrol));
		Print(wind,Message);
		// Reformat(wind,-1,-1,-1,&Black,NO,NO);
		if(shownctrlval != NULL && !shownctrlval[rule.repeatcontrol]) {
			shownctrlval[rule.repeatcontrol] = TRUE;
			my_sprintf(Message,"=%ld",(long)ParamInit[rule.repeatcontrol]);
			Print(wind,Message);
			}
		// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
		my_sprintf(Message,")");
		Print(wind,Message);
		}
	else {
		my_sprintf(Message," %s(",*((*p_GramProcedure)[2]));
		Print(wind,Message);
		// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
		my_sprintf(Message,"%ld",(long)i);
		Print(wind,Message);
		// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
		Print(wind,")");
		}
	}
if((ig=rule.failedgram) > 0) {
	ir = rule.failedrule;
	my_sprintf(Message," %s(",*((*p_GramProcedure)[1]));
	Print(wind,Message);
	// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
	my_sprintf(Message,"%ld,%ld",(long)ig,(long)ir);
	Print(wind,Message);
	// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
	Print(wind,")");
	}
if((ig=rule.gotogram) > 0) {
	ir = rule.gotorule;
	my_sprintf(Message," %s(",*((*p_GramProcedure)[0]));
	Print(wind,Message);
	// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
	my_sprintf(Message,"%ld,%ld",(long)ig,(long)ir);
	Print(wind,Message);
	// Reformat(wind,-1,-1,(int) 1,&None,NO,NO);
	Print(wind,")");
	}
// Reformat(wind,-1,-1,(int) 0,&None,NO,NO);
r = OK;

END:
my_sprintf(Message,"\n");
Print(wind,Message);
ShowSelect(CENTRE,wind);
return(r);
}


int ShowAlphabet(void)
{
int i,j,jj,dirtymem;

dirtymem = Dirty[wTrace];
SetSelect(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
Println(wTrace,"\nALPHABET:");
if(Jhomo == 0) {
	for(j=0; j < Jbol; j++) {
		Println(wTrace,*((*p_Bol)[j]));
		}
	}
for(i=0; i < Jhomo; i++) {
	// Reformat(wTrace,-1,-1,-1,&Color[HomomorphismC],NO,NO);
	my_sprintf(Message,"%s\n",*((*p_Homo)[i]));
	Print(wTrace,Message);
	// Reformat(wTrace,-1,-1,-1,&Black,NO,NO);
	for(j=0; j < Jbol; j++){
		jj = (*((*p_Image)[i]))[j];
		if(jj >= 16384) {
			my_sprintf(Message,"%s --> %s\n",*((*p_Bol)[j]),
							*((*(p_NoteName[NoteConvention]))[jj-16384]));
			}
		else {
			my_sprintf(Message,"%s --> %s\n",*((*p_Bol)[j]),*((*p_Bol)[jj]));
			}
		Print(wTrace,Message);
		}
	for(j=0; j < 128; j++){
		jj = (*((*p_NoteImage)[i]))[j];
		if(jj < Jbol) {
			my_sprintf(Message,"%s --> %s\n",*((*(p_NoteName[NoteConvention]))[j]),
				*((*p_Bol)[jj]));
			Print(wTrace,Message);
			}
		if(jj >= 16384) {
			if((jj-16384) != j) {
				my_sprintf(Message,"%s --> %s\n",*((*(p_NoteName[NoteConvention]))[j]),
					*((*(p_NoteName[NoteConvention]))[jj-16384]));
				Print(wTrace,Message);
				}
			}
		else
			if(jj >= Jbol) {
				BPPrintMessage(0,odError,"=> Err. ShowAlphabet(). jj >= Jbol");
				return(ABORT);
				}
		}
	my_sprintf(Message,"------------\n");
	Print(wTrace,Message);
	}
ShowSelect(CENTRE,wTrace);
Dirty[wTrace] = dirtymem;
return(OK);
}


int DisplayMode(tokenbyte ***pp_a,int *p_ifunc,int *p_hastabs) {
	int datamode,setting_section,founddigit;
	tokenbyte m,p;
	long i;

	datamode = TRUE;
	(*p_ifunc) = (*p_hastabs) = founddigit = FALSE;

	for(i=0,setting_section=TRUE; ; i+=2) {
		m = (**pp_a)[i]; p = (**pp_a)[i+1];
		if(m == TEND && p == TEND) break;
		if(Jfunc && m == T3 && (**pp_a)[i+1] == Jfunc) {
			(*p_ifunc) = TRUE;
			datamode = FALSE;
			break;
			}
		switch(m) {
			case T1:
				founddigit = TRUE;
				break;
			case T12:
				switch(p) {
					case 21:	/* _retro */
					case 22:	/* _rndseq */
					case 24:	/* _ordseq */
						return(FALSE);
						break;
					}
				break;
			case T4:	/* variable */
			case T6:	/* wild card */
			case T26:	/* _transpose */
			case T37:	/* _keymap */
			case T39:	/* _rotate */
			case T40:	/* _keyxpand */
				return(FALSE);
				break;
			}
		if(m == T3 || m == T25 || m == T7) setting_section = FALSE;
	/*	if(!setting_section && (m != T0 || p != 11)
			&& (i < (length-5)) && (**pp_a)[i+2] == T1 &&
				(**pp_a)[i+4] == T0 && (**pp_a)[i+5] == 11) {
			Digit not preceded, but followed, by slash: fractional gap
			datamode = FALSE; break;
			} */
		if(m == T0) {
			switch(p) {
				case 1:
				case 2:
				case 10:
				case 17:
				case 24: /* '**' scale down */
				case 25: /* '\' speed down */
					break;
				case 3:  /* '+' */
				case 11: /* '/' */
					if(setting_section && founddigit) (*p_hastabs) = TRUE;
					setting_section = FALSE;
					continue;
					break;
				default:
					continue;
				}
			datamode = setting_section = FALSE;
			break;
			}
		}
	return(datamode);
	}


int SequenceField(tokenbyte ***pp_a,long id)
{
long i,foundperiod;
tokenbyte m,p;
int level;

level = 0; foundperiod = FALSE;
for(i=id; ; i+=2) {
	m = (**pp_a)[i]; p = (**pp_a)[i+1];
	if(m == TEND && p == TEND) break;
	if(m == T0 && p == 7 && level == 0) foundperiod = TRUE;
	if(m == T0 && (p == 13 || p == 14) && level == 0) return(foundperiod);
	if(m == T0 && p == 12) level++;
	if(m == T0 && p == 13) level--;
	}
return(FALSE);
}


int HasStructure(tokenbyte **p_a)
{
tokenbyte m,p;
unsigned long i;
int datamode;

for(i=ZERO; ; i+=2L) {
	m = (*p_a)[i]; p = (*p_a)[i+1];
	if(m == TEND && p == TEND) break;
	switch(m) {
		case T0:
			switch(p) {
				case 6:		/* master parenthesis */
				case 8:		/* '(' */
					return(YES);
					break;
				}
			break;
		case T5:	/* homomorphism */
			return(YES);
			break;
		}
	}
return(NO);
}

/*
HasPeriods(tokenbyte **p_a)
{
tokenbyte m,p;
unsigned long i;

for(i=ZERO; ; i+=2L) {
	m = (*p_a)[i]; p = (*p_a)[i+1];
	if(m == TEND && p == TEND) break;
	if(m == T0 && p == 7) return(YES);
	}
return(NO);
} */


int ShowDuration(int store)
{
char line[MAXLIN];
int i;

if(Pduration > 0.) {
	if(Qduration > 1.)
		my_sprintf(Message,"Dur = %.0f/%.0f = %.1f ticks",
			Pduration,Qduration,Pduration/Qduration);
	else my_sprintf(Message,"Dur = %.0f ticks",Pduration);
		
	if(Ratio != Prod) {
		if(Ratio < ULONG_MAX) my_sprintf(line,"  Ratio = %lu",(unsigned long)Ratio);
		else my_sprintf(line,"   Ratio = %.0f",Ratio);
		if((strlen(Message) + strlen(line)) < MAXLIN) strcat(Message,line);
		}
		
	my_sprintf(line,"  Prod = %.0f",Prod);
	if((strlen(Message) + strlen(line)) < MAXLIN) strcat(Message,line);
	
	my_sprintf(line,"  [%ld objects]",(long)Maxevent);
	if((strlen(Message) + strlen(line)) < MAXLIN) strcat(Message,line);
	
	if(Kpress > 1.) my_sprintf(line,"  Comp = %.0f",Kpress);
	else my_sprintf(line,"  (no compression)");
	
	if((strlen(Message) + strlen(line)) < MAXLIN) strcat(Message,line);
	
	if(trace_display) ShowMessage(store, wMessage, Message);
	}
return(OK);
}
