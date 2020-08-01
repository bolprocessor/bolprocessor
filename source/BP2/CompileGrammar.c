/* CompileGrammar.c (BP2 version CVS) */

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


CompileGrammar(int mode)
{
int i,istart,igram,irul,gap,check,needsnumber,fatal,onerulefound,tracecompile,r,rep,
	dirtymem,done,changednumber;
long pos,posmax,posline,posinstr,starttrace,origin,end,dummy,startsel,endsel;
char *p,*q,**p_line,**p_line2;
t_rule **ptr;
Handle ptr1;

dummy = ZERO;
if(CheckEmergency() != OK) return(ABORT);
#if BP_CARBON_GUI
if(GetTuning() != OK) return(ABORT);
ResetPannel();
#endif /* BP_CARBON_GUI */
SelectBehind(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
if(!ScriptExecOn) PrintBehind(wTrace,"\n");
fatal = changednumber = FALSE;
CompiledGr = Gram.trueBP = Gram.hasTEMP = Gram.hasproc = WillRandomize = FALSE;
NotBPCase[8] = NotFoundMetronom = NotFoundNatureTime = TRUE;
if(mode == 1 && IsEmpty(wGrammar)) {
	CompiledGr = TRUE;
	return(FAILED);
	}

TextGetSelection(&GramSelStart, &GramSelEnd, TEH[wGrammar]);
startsel = GramSelStart;
endsel = GramSelEnd;

#if BP_CARBON_GUI
GetDateTime((unsigned long*)&CompileDate);
#else
CompileDate = (long) time(NULL);
#endif /* BP_CARBON_GUI */

NoAlphabet = TRUE;
GetAlphaName(wGrammar);
N_err = 0;

CompileOn++;

if(!CompiledAl || (AddBolsInGrammar() > 0)) {
	CompiledAl = CompiledGl = FALSE;
	if(CompileAlphabet() != OK) {
		if(CompileOn) CompileOn--;
		return(FAILED);
		}
	}
if(mode == 1 && !CompiledPt) {
	if((rep=CompilePatterns()) != OK) {
		if(CompileOn) CompileOn--;
		return(rep);
		}
	}
starttrace = GetTextLength(wTrace);
tracecompile = FALSE; rep = YES;
dirtymem = Dirty[wGrammar];
if(mode == 1 && !ScriptExecOn && 0 && DisplayProduce && (rep=Answer("Trace grammar compilation",'N')) == YES)
	tracecompile = TRUE; /* $$$ */
if(rep == ABORT) {
	if(CompileOn) CompileOn--;
	return(FAILED);
	}
ReleaseGrammarSpace();
if(GetGrammarSpace() == ABORT || ResetVariables(wGrammar) != OK) {
	if(CompileOn) CompileOn--;
	ReleaseGrammarSpace();
	return(FAILED);
	}
Gram.number_gram = igram = 1;
(*(Gram.p_subgram))[1].number_rule = 0;
done = TRUE; /* Flag meaning that current number of rules is correct. */
(*(Gram.p_subgram))[1].oldindex = 0;
(*(Gram.p_subgram))[1].type = RNDtype;

(*(Gram.p_subgram))[igram].stop = (*(Gram.p_subgram))[igram].print
= (*(Gram.p_subgram))[igram].printon = (*(Gram.p_subgram))[igram].printoff
= (*(Gram.p_subgram))[igram].stepon = (*(Gram.p_subgram))[igram].stepoff
= (*(Gram.p_subgram))[igram].traceon = (*(Gram.p_subgram))[igram].traceoff
= (*(Gram.p_subgram))[igram].destru = (*(Gram.p_subgram))[igram].randomize
= 0;
(*(Gram.p_subgram))[igram].seed = NOSEED;

(*(Gram.p_subgram))[1].p_rule = NULL;
if((ptr =
	(t_rule**) GiveSpace((Size)(MaxRul+1)*sizeof(t_rule))) == NULL) {
		if(CompileOn) CompileOn--;
		return(ABORT);
		}
(*(Gram.p_subgram))[1].p_rule = ptr;
for(i=1; i <= MaxRul; i++) {
	(*((*(Gram.p_subgram))[1].p_rule))[i].p_leftarg
	= (*((*(Gram.p_subgram))[1].p_rule))[i].p_rightarg = NULL;
	(*((*(Gram.p_subgram))[1].p_rule))[i].p_leftcontext =
	(*((*(Gram.p_subgram))[1].p_rule))[i].p_rightcontext = NULL;
	(*((*(Gram.p_subgram))[1].p_rule))[i].p_leftflag = NULL;
	(*((*(Gram.p_subgram))[1].p_rule))[i].p_rightflag = NULL;
	(*((*(Gram.p_subgram))[1].p_rule))[i].mode
		= (*((*(Gram.p_subgram))[1].p_rule))[i].operator
	 	= (*((*(Gram.p_subgram))[1].p_rule))[i].weight
	 	= (*((*(Gram.p_subgram))[1].p_rule))[i].gotogram
		= (*((*(Gram.p_subgram))[1].p_rule))[i].gotorule
		= (*((*(Gram.p_subgram))[1].p_rule))[i].failedgram
		= (*((*(Gram.p_subgram))[1].p_rule))[i].failedrule
		= (*((*(Gram.p_subgram))[1].p_rule))[i].oldgramindex
		= (*((*(Gram.p_subgram))[1].p_rule))[i].oldrulindex
		= (*((*(Gram.p_subgram))[1].p_rule))[i].repeat
		= (*((*(Gram.p_subgram))[1].p_rule))[i].repeatcontrol
		= (*((*(Gram.p_subgram))[1].p_rule))[i].stop
		= (*((*(Gram.p_subgram))[1].p_rule))[i].print
		= (*((*(Gram.p_subgram))[1].p_rule))[i].printon
		= (*((*(Gram.p_subgram))[1].p_rule))[i].printoff
		= (*((*(Gram.p_subgram))[1].p_rule))[i].stepon
		= (*((*(Gram.p_subgram))[1].p_rule))[i].stepoff
		= (*((*(Gram.p_subgram))[1].p_rule))[i].traceon
		= (*((*(Gram.p_subgram))[1].p_rule))[i].traceoff
		= (*((*(Gram.p_subgram))[1].p_rule))[i].destru
		= 0;
	}

onerulefound = FALSE;

pos = posline = ZERO;
posmax = GetTextLength(wGrammar);

ShowMessage(TRUE,wMessage,"Compiling subgrammar #1...");
for(i=0; i < MAXNOTBPCASES; i++) NotBPCase[i] = FALSE;
NotBPCase[8] = NotBPCase[3] = TRUE;
for(i=1; i < MAXPARAMCTRL; i++) ParamInit[i] = ParamValue[i] = INT_MAX;
p_line = NULL;
InitThere = 0; p_InitScriptLine = NULL;
Deactivate(TEH[wGrammar]);
while(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) == OK) {
	if(DoSystem() != OK) {
		if(CompileOn) CompileOn--;
		return(ABORT);
		}
	PleaseWait();
	if((*p_line)[0] == '\0') goto NEXTLINE;
	if(Mystrcmp(p_line,"DATA:") == 0) break;
	if(Mystrcmp(p_line,"COMMENT:") == 0) break;
	if(Mystrcmp(p_line,"TIMEPATTERNS:") == 0) {
		do {
			if(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) != OK) goto END;
			if((*p_line)[0] == '\0') continue;
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		goto NEXTLINE;
		}
	if(Mystrcmp(p_line,"TEMPLATES:") == 0) {
		do {
			if(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) != OK) goto END;
			if((*p_line)[0] == '\0') continue;
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		Gram.hasTEMP = TRUE;
		goto NEXTLINE;
		}
	p = &((*p_line)[0]); q = &(InitToken[0]);
	if(Match(TRUE,p_line,&q,strlen(InitToken))) {
		istart = strlen(InitToken);
		origin = (long) posline + istart + gap;
		end = (long) posline + MyHandleLen(p_line) + gap;
		while(MySpace(GetTextChar(wGrammar,origin))) origin++;
		posinstr = origin;
		if(origin >= end) goto NEXTLINE;
		if((end - origin) >= MAXLIN) {
			Print(wTrace,"Too long argument for 'INIT:'\n");
			ReleaseGrammarSpace();
			MyDisposeHandle((Handle*)&p_line);
			if(CompileOn) CompileOn--;
			return(FAILED);
			}
		SelectBehind(origin,end,TEH[wGrammar]);
		if((p_line2 = (char**) GiveSpace((Size)(MAXLIN * sizeof(char))))
			== NULL) {
			MyDisposeHandle((Handle*)&p_line);
			if(CompileOn) CompileOn--;
			return(ABORT);
			}
		if(ReadToBuff(YES,FALSE,wGrammar,&origin,end,&p_line2) != OK) {
			ReleaseGrammarSpace();
			MyDisposeHandle((Handle*)&p_line);
			if(CompileOn) CompileOn--;
			return(FAILED);
			}
		check = 2;
		ScriptExecOn++;
		rep = ExecScriptLine(NULL,wScript,check,FALSE,p_line2,dummy,&posinstr,&i,&i);
		EndScript();
		if(rep == OK) {
			/* This will set InitThere to 1 if "Play…" instruction has been found. */
			if(InitThere == 0) {
				InitThere = 2;
				if((p_InitScriptLine=(char**) GiveSpace((Size)
						(1 + MyHandleLen(p_line2)) * sizeof(char))) == NULL) {
					ReleaseGrammarSpace();
					MyDisposeHandle((Handle*)&p_line);
					if(CompileOn) CompileOn--;
					return(ABORT);
					}
				MystrcpyHandleToHandle(0,&p_InitScriptLine,p_line2);
				}
			MyDisposeHandle((Handle*)&p_line2);
			goto NEXTLINE;
			}
		else Print(wTrace,"Unable to make sense of 'INIT:'\n");
		MyDisposeHandle((Handle*)&p_line2);
		if(rep != OK) {
			ReleaseGrammarSpace();
			MyDisposeHandle((Handle*)&p_line);
			if(CompileOn) CompileOn--;
			return(FAILED);
			}
		goto NEXTLINE;
		}
		
	/* Skip headers */
	p = &(*p_line)[0]; q = &(FilePrefix[wAlphabet][0]);
	if((Match(TRUE,p_line,&q,4)) && Gram.number_gram == 1
		&& (*(Gram.p_subgram))[1].number_rule == 0) goto NEXTLINE;
	p = &(*p_line)[0]; q = &(FilePrefix[wInteraction][0]);
	if((Match(TRUE,p_line,&q,4)) && Gram.number_gram == 1
		&& (*(Gram.p_subgram))[1].number_rule == 0) goto NEXTLINE;
	p = &(*p_line)[0]; q = &(FilePrefix[wGlossary][0]);
	if((Match(TRUE,p_line,&q,4)) && Gram.number_gram == 1
		&& (*(Gram.p_subgram))[1].number_rule == 0) goto NEXTLINE;
	p = &(*p_line)[0]; q = &(FilePrefix[iSettings][0]);
	if((Match(TRUE,p_line,&q,4)) && Gram.number_gram == 1
		&& (*(Gram.p_subgram))[1].number_rule == 0) goto NEXTLINE;
	p = &(*p_line)[0]; q = &(FilePrefix[wTimeBase][0]);
	if((Match(TRUE,p_line,&q,4)) && Gram.number_gram == 1
		&& (*(Gram.p_subgram))[1].number_rule == 0) goto NEXTLINE;
	p = &(*p_line)[0]; q = &(FilePrefix[wCsoundInstruments][0]);
	if((Match(TRUE,p_line,&q,4)) && Gram.number_gram == 1
		&& (*(Gram.p_subgram))[1].number_rule == 0) goto NEXTLINE;
	p = &(*p_line)[0]; q = &(FilePrefix[wMIDIorchestra][0]);
	if((Match(TRUE,p_line,&q,4)) && Gram.number_gram == 1
		&& (*(Gram.p_subgram))[1].number_rule == 0) goto NEXTLINE;
	p = &(*p_line)[0]; q = &(FilePrefix[iMidiDriver][0]);
	if((Match(TRUE,p_line,&q,4)) && Gram.number_gram == 1
		&& (*(Gram.p_subgram))[1].number_rule == 0) goto NEXTLINE;
		
	needsnumber = FALSE; igram = 0; irul = 0;
	MyLock(TRUE,(Handle)p_line);
	i = ParseGrammarLine(p_line,&onerulefound,tracecompile,&igram,&irul,
														&needsnumber,&done);
	MyUnlock((Handle)p_line);
	if(i != 0) {
		if(EmergencyExit || (i < 0)) {
			MyDisposeHandle((Handle*)&p_line);
			ReleaseGrammarSpace();
			if(CompileOn) CompileOn--;
			if(EmergencyExit) return(ABORT);
			else return(i);
			}
		Print(wTrace,"\n");
		fatal = ShowError(i,igram,irul);
		Print(wTrace,"??? ");
		Println(wTrace,*p_line);
		Print(wTrace,"\n");
		N_err++;
		if(fatal) {
			if(!ScriptExecOn) Alert1("Fatal error found.  Compilation aborted...");
			else PrintBehindln(wTrace,"Fatal error found.  Compilation aborted...");
			break;
			}
		}
	if(InsertGramRuleNumbers && needsnumber && N_err == 0) {
		Renumber(p_line,posline+gap,&pos,igram,irul,&posmax,&changednumber);
		}
	Dirty[wGrammar] = FALSE;  // FIXME ? What if the grammar was unsaved before CompileGrammar() ?
	
NEXTLINE:
	posline = pos;
#if BP_CARBON_GUI
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	if((r=MyButton(1)) != FAILED) {
		Interrupted = TRUE;
		if(r != OK) goto OUT;
		if((r=InterruptCompile()) != OK) {
OUT:
			if((*(Gram.p_subgram))[Gram.number_gram].number_rule < 1) Gram.number_gram--;
			MaxGram = Gram.number_gram;
			for(i=1; i < MAXPARAMCTRL; i++) {
				if(ParamInit[i] == INT_MAX) ParamInit[i] = ParamValue[i] = 127;
				}
			ReleaseGrammarSpace();
			MyDisposeHandle((Handle*)&p_line);
			if(CompileOn) CompileOn--;
			if(r == EXIT || r == STOP) return(r);
			return(ABORT);
			}
		}
	r = OK;
	if(EventState != NO) {
		r = EventState; goto OUT;
		}
#endif /* BP_CARBON_GUI */
	}  // while (Readline())

END:
MyDisposeHandle((Handle*)&p_line);
if(tracecompile) Print(wTrace,"\n");
if((*(Gram.p_subgram))[Gram.number_gram].number_rule > MaxRul) {
	sprintf(Message,"Err. number rules gram#%ld.",(long)Gram.number_gram);
	if(Beta) Alert1(Message);
	if(CompileOn) CompileOn--;
	return(ABORT);
	}
if((*(Gram.p_subgram))[Gram.number_gram].number_rule < 1) {
	ptr = (t_rule**) (*(Gram.p_subgram))[Gram.number_gram].p_rule;
	MyDisposeHandle((Handle*)&ptr);
	(*(Gram.p_subgram))[Gram.number_gram].p_rule = NULL;
	Gram.number_gram--;
	}
MaxGram = Gram.number_gram;
if(Gram.number_gram == 0) {
	ptr1 = (Handle) Gram.p_subgram;
	MyDisposeHandle(&ptr1);
	Gram.p_subgram = NULL;
	}
CheckGotoFailed();
sprintf(Message,"Errors: %ld",(long)N_err);
ShowMessage(TRUE,wMessage,Message);
for(i=1; i < MAXPARAMCTRL; i++) {
	if(ParamInit[i] == INT_MAX) ParamInit[i] = ParamValue[i] = 127;
	}
if(Jflag > 0) for(i=1; i <= Jflag; i++) (*p_Flag)[i] = ZERO;
SelectBehind(starttrace,GetTextLength(wTrace),TEH[wTrace]);
if(changednumber) {
/*	SelectBehind(ZERO,ZERO,TEH[wGrammar]);
	ShowSelect(CENTRE,wGrammar); */
	Dirty[wGrammar] = TRUE;
	}

if((N_err == 0) && onerulefound) {
	CompiledGr = JustCompiled = TRUE;
	Gram.trueBP = TRUE;
	NotBPCase[8] = FALSE;
	for(i=0; i < MAXNOTBPCASES; i++) {
		if(NotBPCase[i]) {
			Gram.trueBP = FALSE; break;
			}
		}
	if (InsertGramCorrections) InsertSubgramTypes();
#if BP_CARBON_GUI
	ShowPannel(wControlPannel,dSaveDecisions);
	ShowPannel(wControlPannel,dShowGramWeights);
	ShowPannel(wControlPannel,dAnalyze);
#endif /* BP_CARBON_GUI */
	ResetRuleWeights(0);
	Dirty[wGrammar] = dirtymem;
	if(CompileOn) CompileOn--;
	SelectBehind(GramSelStart,GramSelEnd,TEH[wGrammar]);
	return(OK);
	}
else {
	if(mode == 1) {
		ShowSelect(CENTRE,wTrace);
		// Created[wGrammar] = FALSE;
		}
	if(mode == 2) CompiledGr = CompiledAl = TRUE;
	Gram.trueBP = Gram.hasTEMP = Gram.hasproc = FALSE;
	Dirty[wGrammar] = dirtymem;
	if(CompileOn) CompileOn--;
	SelectBehind(GramSelStart,GramSelEnd,TEH[wGrammar]);
	return(N_err == 0);
	}
}


InsertSubgramTypes(void)	/* Insert 'ORD', 'RND', etc. */
{
long pos,posmax,posline;
double n,d;
char *p,*q,**p_line;
int i,dif,gap,igram,irul,found,newsubgram;

pos = posline = ZERO;
posmax = GetTextLength(wGrammar);
igram = 1; irul = 0; found = FALSE;
p_line = NULL;
while(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) == OK) {
	if((*p_line)[0] == '\0') goto NEXTLINE;
	if(Mystrcmp(p_line,"DATA:") == 0) break;
	if(Mystrcmp(p_line,"COMMENT:") == 0) break;
	if(Mystrcmp(p_line,"TIMEPATTERNS:") == 0) {
		do {
			if(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) != OK) goto END;
			if((*p_line)[0] == '\0') continue;
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		goto NEXTLINE;
		}
	if(Mystrcmp(p_line,"TEMPLATES:") == 0) {
		do {
			if(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) != OK) goto END;
			if((*p_line)[0] == '\0') continue;
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		goto NEXTLINE;
		}
	p = &(*p_line)[0]; q = &(InitToken[0]);
	if(Match(TRUE,p_line,&q,4)) goto NEXTLINE;
	
	for(i=0; i < WMAX; i++) {
		if(FilePrefix[i][0] == '\0') continue;
		q = &(FilePrefix[i][0]);
		if(Match(TRUE,p_line,&q,4)) goto NEXTLINE;
		}
	
	p = &(*p_line)[0];
	if(GetSubgramType(&p) != -1) {
		found = TRUE;
		SelectBehind(pos,pos,TEH[wGrammar]);
		if(NotFoundMetronom) {
			if(Pclock != 0.) {
				if(Simplify((double)INT_MAX,(double)60L*Qclock,Pclock,&n,&d) != OK)
					Simplify((double)INT_MAX,Qclock,floor((double)Pclock/60.),&n,&d);
				sprintf(Message,"%s(%.4f) ",*((*p_GramProcedure)[13]),((double)n)/d);
				}
			else sprintf(Message,"%s(no clock) ",*((*p_GramProcedure)[13]));
			PrintBehind(wGrammar,Message);
			dif = strlen(Message);
			MaintainSelectionInGrammar(pos,dif);
			pos += dif; posmax += dif;
			NotFoundMetronom = FALSE;
			Dirty[wGrammar] = TRUE;
			}
		if(NotFoundNatureTime) {
			if(Nature_of_time == STRIATED)
				sprintf(Message,"%s\n",*((*p_GramProcedure)[14]));
			else
				sprintf(Message,"%s\n",*((*p_GramProcedure)[15]));
			PrintBehind(wGrammar,Message);
			dif = strlen(Message);
			MaintainSelectionInGrammar(pos,dif);
			pos += dif; posmax += dif;
			NotFoundNatureTime = FALSE;
			Dirty[wGrammar] = TRUE;
			}
		goto NEXTLINE;
		}
	p = &(*p_line)[0];
	if(irul == 0 && !found && SkipGramProc(&p) == OK) goto PUTIT;
	q = &GRAMstring[0];
	if(Match(FALSE,p_line,&q,strlen(GRAMstring))) {
		if(!found) {
PUTIT:
			SelectBehind(posline+gap,posline+gap,TEH[wGrammar]);
			sprintf(Message,"%s\n",SubgramType[(*(Gram.p_subgram))[igram].type]);
			PrintBehind(wGrammar,Message);
			dif = strlen(Message);
			MaintainSelectionInGrammar(posline+gap,dif);
			pos += dif; posmax += dif;
			found = TRUE;
			}
		irul++; goto NEXTLINE;
		}
	newsubgram = FALSE;
	if((*p_line)[0] == '-' && (*p_line)[MyHandleLen(p_line)-1] == '-') {
		newsubgram = TRUE;
		for(i=0; i < MAXARROW; i++) {
			if(strstr(*p_line,Arrow[i]) != NULLSTR) newsubgram = FALSE;
			}
		}
	if(newsubgram) {
		igram++; irul = 0; found = FALSE;
		}
NEXTLINE:
	posline = pos;
	}
END:
MyDisposeHandle((Handle*)&p_line);
return(OK);
}


Renumber(char **p_line,long posline,long *p_pos,int igram,int irul,
	long *p_posmax,int *p_changednumber)
{
/* register */ int i,j,k;
char c,*p,*q,line2[MAXLIN];
int insertedgram;
long pos1,pos2,pos3,dif;

for(i=0; i < MAXLIN; i++) {
	if((*p_line)[i] == '\0') break;
	line2[i] = (*p_line)[i];
	}
line2[i] = '\0';	/* Gram numbers are in the beginning of 'line'… */
					/* No need to check beyond MAXLIN. */
if(line2[0] == '\0') return(OK);

j = MyHandleLen(p_line) - 1; while(j > 0 && MySpace((*p_line)[j])) j--;
i = 0; while(MySpace(c=(*p_line)[i]) && i < j) i++;

/* Insert 'gram#' */
p = &line2[i]; q = &GRAMstring[0]; pos3 = posline;
insertedgram = TRUE;
if(Match(FALSE,&p,&q,5)) {
	insertedgram = FALSE;
	i += 5; k = 0; pos2 = pos1 = posline + 5;
	pos3 = pos2;
	/* Find first digit of subgram index */
	while(MySpace(c=(*p_line)[i])) {
		i++; pos1++; pos2++; pos3++;
		}
	while(!MySpace(c=(*p_line)[i]) && c != '\0' && c != '[') {
		c -= '0'; k = 10 * k + c; i++; pos2++; pos3++;
		}
	if(k != igram) {
		/* Modify subgram index */
		*p_changednumber = TRUE;
		SelectBehind(pos1,pos2,TEH[wGrammar]);
		TextDeleteBehind(wGrammar);
		sprintf(line2,"%ld",(long)igram);
		PrintBehind(wGrammar,line2);
		dif = strlen(line2) - (pos2 - pos1);
		MaintainSelectionInGrammar(pos1,dif);
		if(pos1 < Sel1) Sel1 += dif; /* Used in PlaySelection() */
		if(pos1 < Sel2) Sel2 += dif; /* Used in PlaySelection() */
		*p_pos += dif; *p_posmax += dif; pos3 += dif;
		(*((*(Gram.p_subgram))[igram].p_rule))[irul].oldgramindex = k;
		(*((*(Gram.p_subgram))[igram].p_rule))[irul].oldrulindex = irul;
		if((*(Gram.p_subgram))[igram].oldindex == 0) {
			(*(Gram.p_subgram))[igram].oldindex = k;
			if(k == (igram - 1)) {
				if((*(Gram.p_subgram))[k].oldindex == 0)
					(*(Gram.p_subgram))[k].oldindex = k;
				}
			}
		}
	}
else {
	/* Insert 'gram#' and subgram index */
	*p_changednumber = TRUE;
	SelectBehind(posline,posline,TEH[wGrammar]);
	sprintf(line2,"%s%ld",GRAMstring,(long)igram);
	PrintBehind(wGrammar,line2);
	dif = strlen(line2);
	MaintainSelectionInGrammar(posline,dif);
	if(posline < Sel1) Sel1 += dif;
	if(posline < Sel2) Sel2 += dif;
	*p_pos += dif; *p_posmax += dif; pos3 += dif;
	}
	
/* Modify or insert rule index */
k = 0;
while(MySpace(c=(*p_line)[i]) && i < j) {
	i++; k++;
	}
if(i == j) goto NOTFOUND;
pos3 += k;
if((*p_line)[i] == '[') {
	pos1 = pos3 + 1; pos2 = pos1;
	k = 0;
	while((c=(*p_line)[++i]) != ']' && i < j) {
		c -= '0';
		if(c < 0 || c > 9) goto NOTFOUND;
		k = 10 * k + c;
		pos2++;
		}
	if(k == irul) return(OK); 	/* Index was correct */
	SelectBehind(pos1,pos2,TEH[wGrammar]);
	TextDeleteBehind(wGrammar);
	sprintf(line2,"%ld",(long)irul);
	PrintBehind(wGrammar,line2);
	dif = strlen(line2) - (pos2 - pos1);
	MaintainSelectionInGrammar(pos1,dif);
	if(pos1 < Sel1) Sel1 += dif;
	if(pos1 < Sel2) Sel2 += dif;
	*p_pos += dif; *p_posmax += dif;
	if(k != 0)  {
		if((*((*(Gram.p_subgram))[igram].p_rule))[irul].oldgramindex == 0)
			(*((*(Gram.p_subgram))[igram].p_rule))[irul].oldgramindex = igram;
		(*((*(Gram.p_subgram))[igram].p_rule))[irul].oldrulindex = k;
		}
	return(OK);
	}
else {
/* No index found.  Insert it. */
NOTFOUND:
	if(insertedgram) sprintf(line2,"[%ld] ",(long)irul);
	else {
		pos3--;
		sprintf(line2,"[%ld]",(long)irul);
		}
	SelectBehind(pos3,pos3,TEH[wGrammar]);
	PrintBehind(wGrammar,line2);
	dif = strlen(line2);
	MaintainSelectionInGrammar(pos3,dif);
	if(pos3 < Sel1) Sel1 += dif;
	if(pos3 < Sel2) Sel2 += dif;
	*p_pos += dif; *p_posmax += dif;
	}
return(OK);
}


CheckGotoFailed(void)
{
int i,igram,irul,ig,ir,newig,newir;

for(igram=1; igram <= Gram.number_gram; igram++) {
	for(irul=1; irul <= (*(Gram.p_subgram))[igram].number_rule; irul++) {
		if((*((*(Gram.p_subgram))[igram].p_rule))[irul].repeat > 0) {
			if((i=(*(Gram.p_subgram))[igram].type) == SUBtype ||
					i == SUB1type || i == POSLONGtype) {
				sprintf(Message,
					"gram#%ld is 'SUB' or 'SUB1' or 'POSLONG' and should not contain '%s'.\n",
					(long)igram,*((*p_GramProcedure)[2]));
				N_err++;
				Print(wTrace,Message);
				}
			}
		if((newig=ig=(*((*(Gram.p_subgram))[igram].p_rule))[irul].gotogram) > 0) {
			newir = ir = (*((*(Gram.p_subgram))[igram].p_rule))[irul].gotorule;
			NewIndex(&newig,&newir);
			if((i=(*(Gram.p_subgram))[igram].type) == SUBtype ||
					i == SUB1type || i == POSLONGtype) {
				sprintf(Message,
					"gram#%ld is 'SUB' or 'SUB1' or 'POSLONG' and should not contain '%s'.\n",
					(long)igram,*((*p_GramProcedure)[0]));
				N_err++;
				Print(wTrace,Message);
				UpdateProcedureIndex(0,igram,irul,newig,newir,1);
				continue;
				}
			if(newig != ig || newir != ir) {
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].gotogram = newig;
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].gotorule = newir;
				UpdateProcedureIndex(0,igram,irul,newig,newir,0);
				}
			if(newig > Gram.number_gram) {
				sprintf(Message,"gram#%ld[%ld] has incorrect grammar index in '%s'\n",
					(long)igram,(long)irul,*((*p_GramProcedure)[0]));
				N_err++;
				Print(wTrace,Message);
				UpdateProcedureIndex(0,igram,irul,newig,newir,1);
				}
			else {
				if(((i=(*(Gram.p_subgram))[newig].type) == SUBtype ||
					i == SUB1type || i == POSLONGtype) && newir > 0) {
					sprintf(Message,
						"gram#%ld[%ld] contains '%s' addressing rule in 'SUB' or 'SUB1' or 'POSLONG' subgrammar.\n",
						(long)igram,(long)irul,*((*p_GramProcedure)[0]));
					N_err++;
					Print(wTrace,Message);
					UpdateProcedureIndex(0,igram,irul,newig,newir,1);
					}
				else {
					if(newir > (*(Gram.p_subgram))[newig].number_rule) {
						sprintf(Message,"gram#%ld[%ld] has incorrect rule index in '%s'\n",
							(long)igram,(long)irul,*((*p_GramProcedure)[0]));
						N_err++;
						Print(wTrace,Message);
						UpdateProcedureIndex(0,igram,irul,newig,newir,2);
						}
					}
				}
			}
		if((newig=ig=(*((*(Gram.p_subgram))[igram].p_rule))[irul].failedgram) > 0) {
			newir = ir = (*((*(Gram.p_subgram))[igram].p_rule))[irul].failedrule;
			NewIndex(&newig,&newir);
			if((i=(*(Gram.p_subgram))[igram].type) == SUBtype ||
					i == SUB1type || i == POSLONGtype) {
				sprintf(Message,
					"gram#%ld is 'SUB' or 'SUB1' or 'POSLONG' and should not contain '%s'.\n",
					(long)igram,*((*p_GramProcedure)[1]));
				N_err++;
				Print(wTrace,Message);
				UpdateProcedureIndex(1,igram,irul,newig,newir,1);
				continue;
				}
			if(newig != ig || newir != ir) {
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].failedgram = newig;
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].failedrule = newir;
				UpdateProcedureIndex(1,igram,irul,newig,newir,0);
				}
			if(newig > Gram.number_gram) {
				sprintf(Message,"gram#%ld[%ld] has incorrect grammar index in '%s'\n",
					(long)igram,(long)irul,*((*p_GramProcedure)[1]));
				N_err++;
				Print(wTrace,Message);
				UpdateProcedureIndex(1,igram,irul,newig,newir,1);
				}
			else {
				if(((i=(*(Gram.p_subgram))[newig].type) == SUBtype ||
						i == SUB1type || i == POSLONGtype) && newir > 0) {
					sprintf(Message,
						"gram#%ld[%ld] contains '%s' addressing rule in 'SUB' or 'SUB1' or 'POSLONG' subgrammar.\n",
						(long)igram,(long)irul,*((*p_GramProcedure)[1]));
					N_err++;
					Print(wTrace,Message);
					UpdateProcedureIndex(1,igram,irul,newig,newir,1);
					}
				else {
					if(newir > (*(Gram.p_subgram))[newig].number_rule) {
						sprintf(Message,"gram#%ld[%ld] has incorrect rule index in '%s'\n",
							(long)igram,(long)irul,*((*p_GramProcedure)[1]));
						N_err++;
						Print(wTrace,Message);
						UpdateProcedureIndex(1,igram,irul,newig,newir,2);
						}
					}
				}
			}
		}
	}
return(OK);
}


UpdateProcedureIndex(int jproc,int igram,int irul,int ig,int ir,int mode)
{
long pos,posmax,pos1,pos2,posline;
char c,*p,*q,**qq,**p_line;
int i,k,dif,gap;

pos = posline = ZERO;
posmax = GetTextLength(wGrammar);
p_line = NULL;
while(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) == OK) {
	PleaseWait();
	if((*p_line)[0] == '\0') goto NEXTLINE;
	i = 0;
	p = &(*p_line)[i]; q = &GRAMstring[0];
	if(Match(FALSE,&p,&q,5)) {
		i += 5; k = 0;
		/* Find subgram index */
		while(MySpace(c=(*p_line)[i])) i++;
		while(!MySpace(c=(*p_line)[i]) && c != '\0' && c != '[') {
			c -= '0';
			if(c >= 0 && c <= 9) k = 10 * k + c;
			i++;
			}
		if(k != igram) goto NEXTLINE;
		k = 0;
		while((c=(*p_line)[i]) != '[') i++;
		k = 0;
		while((c=(*p_line)[++i]) != ']') {
			c -= '0';
			if(c >= 0 && c <= 9) k = 10 * k + c;
			}
		if(k != irul) goto NEXTLINE;
		while(TRUE) {
			p = &(*p_line)[++i]; qq = (*p_GramProcedure)[jproc];
			if(Match(FALSE,&p,qq,MyHandleLen(((*p_GramProcedure)[jproc])))) {
				while((c=(*p_line)[i]) != '(') i++;
				pos1 = posline + gap + i + 1; pos2 = pos1;
				while((c=(*p_line)[++i]) != ',') pos2++;
				SelectBehind(pos1,pos2,TEH[wGrammar]);
				TextDeleteBehind(wGrammar);
				switch(mode) {
					case 0:
					case 2:
						sprintf(Message,"%ld",(long)ig); break;
					case 1:
						sprintf(Message,"%ld?",(long)ig); break;
					}
				PrintBehind(wGrammar,Message);
				dif = strlen(Message) - (pos2 - pos1);
				MaintainSelectionInGrammar(pos1,dif);
				pos += dif; posmax += dif;
				pos1 = posline + gap + i + 1 + dif; pos2 = pos1;
				while((c=(*p_line)[++i]) != ')') pos2++;
				SelectBehind(pos1,pos2,TEH[wGrammar]);
				TextDeleteBehind(wGrammar);
				switch(mode) {
					case 0:
					case 1:
						sprintf(Message,"%ld",(long)ir); break;
					case 2:
						sprintf(Message,"%ld?",(long)ir); break;
					}
				PrintBehind(wGrammar,Message);
				dif = strlen(Message) - (pos2 - pos1);
				MaintainSelectionInGrammar(pos1,dif);
				pos += dif; posmax += dif;
				goto END;
				}
			}
		}
NEXTLINE:
	posline = pos;
	}
END:
MyDisposeHandle((Handle*)&p_line);
return(OK);
}


NewIndex(int *p_ig, int *p_ir)
{
int igram,irul;
for(igram=1; igram <= Gram.number_gram; igram++) {
	if(*p_ir == 0 && (*(Gram.p_subgram))[igram].oldindex == *p_ig) {
		*p_ig = igram; return(OK);
		}
	for(irul=1; irul <= (*(Gram.p_subgram))[igram].number_rule; irul++) {
		if(*p_ig == (*((*(Gram.p_subgram))[igram].p_rule))[irul].oldgramindex
			&& *p_ir == (*((*(Gram.p_subgram))[igram].p_rule))[irul].oldrulindex) {
			*p_ig = igram; *p_ir = irul; return(OK);
			}
		}
	}
return(OK);
}


CompileAlphabet(void)
{
int rep;

rep = FAILED;
N_err = 0;

#if BP_CARBON_GUI
if(GetTuning() != OK) return(ABORT);
#endif /* BP_CARBON_GUI */
if(ReleaseObjectPrototypes() != OK) return(ABORT);
ShowMessage(TRUE,wMessage,"Compiling alphabet...");
if(!NoAlphabet && IsEmpty(wAlphabet) && (LoadAlphabet(-1,NULL) != OK)) goto ERR;

GetMiName();
#if BP_CARBON_GUI
GetKbName(wAlphabet);
// FIXME ? Probably will want to get Cs instr. & Midi orch. in console build ?
if(GetCsName(wAlphabet) != OK && GetCsName(wData) != OK) GetCsName(wGrammar);
if(GetFileNameAndLoadIt(wMIDIorchestra,wAlphabet,LoadMIDIorchestra) != OK
		&& GetFileNameAndLoadIt(wMIDIorchestra,wData,LoadMIDIorchestra) != OK) 
	GetFileNameAndLoadIt(wMIDIorchestra,wGrammar,LoadMIDIorchestra);
#endif /* BP_CARBON_GUI */
if(ReleaseAlphabetSpace() != OK) return(ABORT);
Jhomo = 0; Jbol = 2;	/* Counting will not include "_" and "-" */
if((rep=ReadAlphabet(TRUE)) != OK){		/* Just count */
	Jbol = Jhomo = 0;
	goto ERR;
	}
BolsInGrammar = AddBolsInGrammar();
Jbol = Jbol + BolsInGrammar;
if((rep=GetAlphabetSpace()) != OK) {	/* This creates "_" and "-" */
	ReleaseAlphabetSpace();
	goto ERR;
	}
if((rep=ReadAlphabet(FALSE)) != OK) {	/* Now creating bols */
	ReleaseAlphabetSpace();
	goto ERR;
	}
CompiledAl = TRUE;
return(OK);

ERR:
BPActivateWindow(SLOW,wTrace);
ShowMessage(TRUE,wMessage,"Can't compile alphabet");
return(rep);
}


ReadAlphabet(int justcount)
{
long pos,posmax;
char *q,**p_line,line[MAXLIN],operatorbetweenquotes[MAXLIN];
int done,foundoperator,foundoperatorthere,fatal,i,operatorinline,
	miknown=FALSE,inknown=FALSE,kbknown=FALSE,glknown=FALSE,tbknown=FALSE,
	csknown=FALSE,orknown=FALSE,rep,gap;

pos = ZERO; done = foundoperator = FALSE;
posmax = GetTextLength(wAlphabet);

CompileOn++;

p_line = NULL;
sprintf(operatorbetweenquotes,"\'%s\'",Arrowstring);
while(ReadLine(YES,wAlphabet,&pos,posmax,&p_line,&gap) == OK) {
	PleaseWait();
	if((*p_line)[0] != '\0' && strstr(*p_line,Arrowstring) != NULLSTR
			&& strstr(*p_line,operatorbetweenquotes) == NULLSTR) {
		/*  Arrow is there and it is not between single quotes (check is incomplete) $$$ */
		foundoperator = TRUE;
		break;
		}
	}
MyDisposeHandle((Handle*)&p_line);
pos = ZERO; foundoperatorthere = FALSE;
while(ReadLine(YES,wAlphabet,&pos,posmax,&p_line,&gap) == OK) {
	if((*p_line)[0] == '\0' || (*p_line)[0] == '\r') goto NEXTLINE;
	PleaseWait();
	operatorinline = FALSE;
	MystrcpyHandleToString(MAXLIN,0,line,p_line);
	if(strstr(line,Arrowstring) != NULLSTR && strstr(line,operatorbetweenquotes) == NULLSTR)
		foundoperatorthere = operatorinline = TRUE;
	if(!inknown) {
		q = &(FilePrefix[wInteraction][0]);
		if(Match(TRUE,p_line,&q,4)) {
			inknown = TRUE; goto NEXTLINE;
			}
		}
	if(!glknown) {
		q = &(FilePrefix[wGlossary][0]);
		if(Match(TRUE,p_line,&q,4)) {
			glknown = TRUE; goto NEXTLINE;
			}
		}
	if(!miknown) {
		q = &(FilePrefix[iObjects][0]);
		if(Match(TRUE,p_line,&q,4)) {
			miknown = TRUE; goto NEXTLINE;
			}
		}
	if(!kbknown) {
		q = &(FilePrefix[wKeyboard][0]);
		if(Match(TRUE,p_line,&q,4)) {
			kbknown = TRUE; goto NEXTLINE;
			}
		}
	if(!csknown) {
		q = &(FilePrefix[wCsoundInstruments][0]);
		if(Match(TRUE,p_line,&q,4)) {
			csknown = TRUE; goto NEXTLINE;
			}
		}
	if(!orknown) {
		q = &(FilePrefix[wMIDIorchestra][0]);
		if(Match(TRUE,p_line,&q,4)) {
			orknown = TRUE; goto NEXTLINE;
			}
		}
	if(!tbknown) {
		q = &(FilePrefix[wTimeBase][0]);
		if(Match(TRUE,p_line,&q,4)) {
			tbknown = TRUE; goto NEXTLINE;
			}
		}
#if BP_CARBON_GUI
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	if(!justcount && (rep=MyButton(1)) != FAILED) {
		if(rep != OK || (rep=InterruptCompile()) != OK) {
			MyDisposeHandle((Handle*)&p_line);
			if(CompileOn) CompileOn--;
			return(rep);
			}
		}
	rep = OK;
	if(EventState != NO) {
		MyDisposeHandle((Handle*)&p_line);
		if(CompileOn) CompileOn--;
		return(EventState);
		}
#endif /* BP_CARBON_GUI */
	if(Mystrcmp(p_line,"TIMEPATTERNS:") == 0) {
		do {
			if(ReadLine(YES,wAlphabet,&pos,posmax,&p_line,&gap) != OK) goto END;
			if((*p_line)[0] == '\0') {
				goto NEXTLINE;
				}
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		goto NEXTLINE;
		}
	if(foundoperator && !done) {
		done = TRUE;
		if(strstr(line,Arrowstring) != NULLSTR && strstr(line,operatorbetweenquotes) == NULLSTR) {
			Print(wTrace,"??? "); Print(wTrace,line);
			Println(wTrace,"  This line is expected to contain a homomorphism label only.");
			if(CompileOn) CompileOn--;
			MyDisposeHandle((Handle*)&p_line);
			return(ABORT);
			}
		if((i=GetHomomorph(p_line,justcount)) > 0) {
			N_err++;
			Print(wTrace,"??? "); Println(wTrace,line);
			if((fatal=ShowError(i,0,0))) {
				if(CompileOn) CompileOn--;
				MyDisposeHandle((Handle*)&p_line);
				return(ABORT);
				}
			}
		}
	else {
		if((*p_line)[0] == '-' && (*p_line)[1] == '-') {
			done = FALSE;
			if(!foundoperatorthere) {
				Print(wTrace,"Error in alphabet: section should contain the definition of an homomorphism.");
				if(CompileOn) CompileOn--;
				MyDisposeHandle((Handle*)&p_line);
				return(ABORT);
				}
			foundoperatorthere = FALSE;
			}
		else {
			if((i=GetBols(p_line,justcount,operatorinline)) != 0) {
				N_err++;
				if(i == ABORT) goto BAD;
				Print(wTrace,"??? "); Println(wTrace,line);
				if((fatal=ShowError(i,0,0))) {
BAD:
					MyDisposeHandle((Handle*)&p_line);
					if(CompileOn) CompileOn--;
					return(ABORT);
					}
				}
			}
		}
NEXTLINE: ;
	}
	
END:
MyDisposeHandle((Handle*)&p_line);
if(CompileOn) CompileOn--;
if(Jbol < 3) NoAlphabet = TRUE;
else NoAlphabet = FALSE;
if(N_err) {
	if(!ScriptExecOn) Alert1("Alphabet is incorrect...");
	else PrintBehindln(wTrace,"Alphabet is incorrect...");
	return(FAILED);
	}
else return(OK);
}


AddBolsInGrammar(void)
{
/* register */ int i,j=0,rem;
int gap;
long pos,posmax;
char **p_line;

if(IsEmpty(wGrammar)) return(0);
pos = ZERO;
posmax = GetTextLength(wGrammar);
p_line = NULL;
PleaseWait();
while(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) == OK) {
	if((*p_line)[0] == '\0') continue;
	rem = FALSE;
	for(i=0; i < MyHandleLen(p_line)-3; i++) {
		if((*p_line)[i] == '[') rem = TRUE;
		if((*p_line)[i] == ']') rem = FALSE;
		if(rem) continue;
		if(MySpace((*p_line)[i]) && (*p_line)[i+1] == '\'') j++;
		if((*p_line)[i] == '<' && (*p_line)[i+1] == '<') j++;
		if((*p_line)[i] == '\334' && (*p_line)[i+1] == '\334') j++;
		}
	}
MyDisposeHandle((Handle*)&p_line);
return(j);
}


GetHomomorph(char **p_line,int justcount)
{
/* register */ int i,j;

i = 0; j = 0;
if((*p_line)[0] == '\0') {
	if(Beta) Alert1("Err. GetHomomorph(). (*p_line)[0] == '\0'");
	return(24);
	}
while(MySpace((*p_line)[i])) i++;
while(i < MyHandleLen(p_line) && !MySpace((*p_line)[i]) && (*p_line)[i] != '\0') {
	if(j >= HOMOSIZE-1) {
		ShowError(26,0,0);
		return(24);
		}
	if(!justcount) (*((*p_Homo)[Jhomo]))[j++] = (*p_line)[i];
	i++;
	}
if(!justcount) (*((*p_Homo)[Jhomo]))[j] = '\0';
Jhomo++;
return(0);
}


GetBols(char **p_line,int justcount,int operatorthere)
{
char c,**p_y,*p,*q;
int i,j,k,k1,k2,l,length,r;

l = MyHandleLen(p_line)-1;
if((p_y = (char**) GiveSpace((Size)((BOLSIZE+1) * sizeof(char)))) == NULL) {
	return(26);
	}
r = 0;
for(i=0,k1=0; i <= l;) {
	if((length=GetBol(p_line,&i)) > BOLSIZE) {
		sprintf(Message,"\nMaximum length: %ld chars.\n",(long)BOLSIZE);
		Print(wTrace,Message);
		ShowError(22,0,0);
		MyDisposeHandle((Handle*)&p_y);
		return(26);
		}
	if(length == -1) {
		MyDisposeHandle((Handle*)&p_y);
		return(27);
		}
	j = i + length;
	c = (*p_line)[j];
	if(!isspace(c) && c != '\0') {
		sprintf(Message,"Can't accept character '%c' in alphabet\n",c);
		Print(wTrace,Message);
		r = ABORT; goto QUIT;
		} 
	(*p_line)[j++] = '\0';
	for(k=0; (i+k) < j; k++) (*p_y)[k] = (*p_line)[i+k];
	k2 = CreateBol(TRUE,TRUE,p_y,justcount,FALSE,BOL);
	if(k2 < 0) {
		r = ABORT; goto QUIT;
		}
	if(k2 >= (16384+128)) {
		if(Beta) Alert1("Err. GetBols(). k2 >= (16384+128)");
		r = ABORT; goto QUIT;
		}
	if(!justcount && k1 > 0 && Jhomo > 0 && operatorthere) {
		if(k1 < 16384) (*((*p_Image)[Jhomo-1]))[k1] = k2;
		else (*((*p_NoteImage)[Jhomo-1]))[k1-16384] = k2;
		}
	k1 = k2;
	while(MySpace((*p_line)[j]) && j < l) j++;
	if(j > l) goto QUIT;
	if(operatorthere) {
		p = &((*p_line)[j]); q = &Arrowstring[0];
		if(!Match(TRUE,&p,&q,3)) {
			sprintf(Message,"'-->' not found!\n");
			Print(wTrace,Message);
			r = 24; goto QUIT;
			}
		j += 3;
		if(j > (l-1)) {
			r = 24; goto QUIT;
			}
		}
	i = j;
	}
QUIT:
MyDisposeHandle((Handle*)&p_y);
return(r);
}


GetBol(char **p_line,int *p_i)
{
int i,j;
char c,line[MAXLIN];

// firstc = (*p_line)[*p_i];
while(MySpace(c=(*p_line)[*p_i])) (*p_i)++;
i = (*p_i);
if((*p_line)[*p_i] == '\'') {
	/* Read terminal between single quotes */
	for(j=(*p_i)+1;(c=(*p_line)[j]) != '\0' && c != '\''; j++){};
	j++;
	}
else {
	if(!OkBolChar((*p_line)[*p_i])) goto ERR;
	for(j=(*p_i)+1; OkBolChar2(c=(*p_line)[j]); j++) {
		if(c == '-') {
			Print(wTrace,"Found '-' in terminal symbol\n");
			goto ERR;
			}
		}
	}
return(j-(*p_i));

ERR:
j = 0;
while(TRUE) {
	c = (*p_line)[i];
	if(isspace(c) || j >= MAXLIN) break;
	line[j++] = c;
	i++;
	}
line[j] = '\0';
sprintf(Message,"Can't make sense of \"%s\"\n",line);
Print(wTrace,Message);
return(-1);
}


OkChar(char c)
// Characters accepted in variable names
{
/* if(isalnum(c)) return(TRUE); */
if(!iscntrl(c) && !ispunct(c) && !MySpace(c)) return(TRUE);
switch(c) {
	case '\'':
	case '-':
	case '_':
	case '#':
	case '@':
	case '%':
	case '$':
	case '"':
	case '\\':
	case '§':
	case '•':
		return(TRUE);
	}
return(FALSE);
}


OkBolChar(char c)
// Characters accepted as first one of terminal name
{
if(islower(c) || isupper(c)) return(YES); /* Modified 23/11/2009 BB */
switch(c) {
	case 'Ä':
	case 'Å':
	case 'Ç':
	case 'É':
	case 'Ñ':
	case 'Ö':
	case 'Ü':
	case 'á':
	case 'à':
	case 'â':
	case 'ä':
	case 'ã':
	case 'å':
	case 'ç':
	case 'é':
	case 'è':
	case 'ê':
	case 'ë':
	case 'í':
	case 'ì':
	case 'î':
	case 'ï':
	case 'ñ':
	case 'ó':
	case 'ò':
	case 'ô':
	case 'ö':
	case 'õ':
	case 'ú':
	case 'ù':
	case 'û':
	case 'ü':
	case 'Æ':
	case 'æ':
	case 'ø':
	case 'À':
	case 'Ã':
	case 'Õ':
	case 'Œ':
	case 'œ':
	case 'ÿ':
	case 'ß':
	case 'Ø':
	case 'µ':
	case '∂':
	case '∑':
	case '∏':
	case 'π':
	case 'Ω':
	case 'ƒ':
	case '∆':
	case '¥':
	case '#':
	case '\'':
		return(YES);
	}
return(NO);
}


OkBolChar2(char c)
// Characters accepted in terminal names (after the first one)
{
if(isdigit(c) || isalpha(c) || OkBolChar(c)) return(OK);
switch(c) {
	case '-':	/* Discarded in GetBol() */
	case '@':
	case '%':
	case '¢':
	case '£':
	case '§':
	case '◊':
	case '$':
	case '®':
	case '©':
	case '™':
	case '"':
	case '†':
	case '°':
	case '`':
	case '´':
	case '¨':
	case '≠':
	case '∞':
	case '±':
	case '≤':
	case '≥':
	case 'ª':
	case 'º':
	case '¿':
	case '¡':
	case '√':
	case '«':
	case '»':
	case '“':
	case '”':
	case '‘':
	case '’':
	case '÷':
		return(YES);
	}
return(NO);
}


CreateBol(int reload,int checknotes,char **p_x, int justcount, int mark, int type)
{
int j,ln,diff,jmax,cv,compiledptmem,dirtyalphabetmem;
char **ptr,****p_t,*q,line[MAXLIN];

if(type == BOL) {
	jmax = Jbol; p_t = p_Bol;
	}
else {
	jmax = Jpatt; p_t = p_Patt;
	}
if(justcount) {
	jmax++;
	if(type == BOL) Jbol = jmax;
	else Jpatt = jmax;
	return(0);
	}
diff = TRUE;
for(j=0; j < MAXNIL; j++) {
	if(Mystrcmp(p_x,NilString[j]) == 0) {
		ShowError(54,0,0);
		return(ABORT);
		}
	}
if(jmax > 0) {
	if(p_t == NULL) {
		if(Beta) Alert1("Err. CreateBol(). p_t == NULL");
		return(ABORT);
		}
	for(j=0; j < jmax; j++) {
		if((diff = MyHandlecmp((*p_t)[j],p_x)) == 0) break;
		}
	}
if(diff && checknotes) {
	/* Maybe it's a simple note */
	cv = NoteConvention;
	for(j=0; j < 128; j++) {
		MystrcpyHandleToString(MAXLIN,0,line,(*(p_NoteName[cv]))[j]);
		q = &(line[0]); ln = strlen(line);
		if(Match(TRUE,p_x,&q,ln) && !isdigit((*p_x)[ln])) {
FOUNDNOTE:
			j += (C4key - 60);
			if(j < 0 || j > 127) {
				sprintf(Message,
					"Simple note '%s' is out of range. (Maybe check \"Tuning\")",
					line);
				Alert1(Message);
				ShowWindow(GetDialogWindow(TuningPtr));
				SelectWindow(GetDialogWindow(TuningPtr));
				return(ABORT);
				}
			return(j+16384);
			}
		}
	for(j=0; j < 128; j++) {
		MystrcpyHandleToString(MAXLIN,0,line,(*(p_AltNoteName[cv]))[j]);
		q = &(line[0]); ln = strlen(line);
		if(Match(TRUE,p_x,&q,ln) && !isdigit((*p_x)[ln])) {
			goto FOUNDNOTE;
			}
		}
	}
if(diff) {
	j = jmax;
	if(type == BOL) {
		if(reload) ObjectMode = ObjectTry = FALSE;
		dirtyalphabetmem = Dirty[wAlphabet];
		if(Mystrcmp(p_x,"'-->'") == 0) Jfunc = j;
		if(mark) {
			if(ReleaseObjectPrototypes() != OK) return(ABORT);
			Jbol = jmax + 1;
			if(CheckTerminalSpace() != OK) return(ABORT);
			p_t = p_Bol;	/* may have changed */
			compiledptmem = CompiledPt;
			SelectBehind(GetTextLength(wAlphabet),GetTextLength(wAlphabet),
																		TEH[wAlphabet]);
			sprintf(Message,"\n%s",*p_x);
			PrintBehind(wAlphabet,Message);
			CompiledAl = TRUE;
			Dirty[wAlphabet] = dirtyalphabetmem;
			CompiledPt = compiledptmem;
			ShowMessage(TRUE,wMessage,"New terminal symbols have been created.");
			}
		}
	if((ptr=(char**) GiveSpace((Size)MyHandleLen(p_x)+1)) == NULL) return(ABORT);
	(*p_t)[jmax] = ptr;
	MystrcpyHandleToHandle(0,&((*p_t)[jmax]),p_x);
	jmax++;
	}
else if(type == PATTERN) {
	sprintf(Message,"\nCan't accept %s\n",*p_x);
	Print(wTrace,Message);
	ShowError(48,0,0);
	return(ABORT);
	}
if(type == BOL) Jbol = jmax;
else Jpatt = jmax;
return(j);
}


ParseGrammarLine(char** p_line,int *p_onerulefound,int tracecompile,int *p_igram,
	int *p_irul,int *p_needsnumber,int *p_done)
{
int i,j,igram,irul,w,imode,type,operator,incweight,lenc,result,foundk,
	newsubgram,meta,lenarg,beforefirstrule,proc;
long initparam,y,u,v;
tokenbyte **pp_leftp,**pp_rightp,**pp_a;
char c,**ptr,*p,**pp1,*p1,**pp2,*p2,**pp3,*p3,**pp4,*p4,*q;
p_context *p_pleftcontext,ppl,*p_prightcontext,ppr,*p_plx,plx,*p_prx,prx;
p_flaglist **h_flag;
t_rule **ptrule;
double x;

p_pleftcontext = &ppl; p_prightcontext = &ppr;
p_plx = &plx; p_prx = &prx;
igram = *p_igram = Gram.number_gram;
irul = *p_irul = (*(Gram.p_subgram))[igram].number_rule;

if((*p_line)[0] == '\0') return(0);

newsubgram = FALSE;
if((*p_line)[0] == '-' && (*p_line)[MyHandleLen(p_line)-1] == '-') {
	newsubgram = TRUE;
	for(i=0; i < MAXARROW; i++) {
		if(strstr(*p_line,Arrow[i]) != NULLSTR) newsubgram = FALSE;
		}
	}
if(newsubgram) {
	*p_done = TRUE;
	if((*(Gram.p_subgram))[igram].number_rule > MaxRul) {
		sprintf(Message,
			"Err. number rules gram#%ld. ",(long)Gram.number_gram);
		if(Beta) Alert1(Message);
		N_err++; return(2);
		}
	if(tracecompile) Print(wTrace,"------------------------\n");
	if((++Gram.number_gram) > MaxGram) {
		sprintf(Message,"Err. number grams = %ld  MaxGram = %ld. ",
				(long)Gram.number_gram,(long)MaxGram);
		if(Beta) Alert1(Message);
		N_err++; return(1);
		}
	*p_igram = ++igram;
	(*(Gram.p_subgram))[igram].number_rule = *p_irul = 0;
	sprintf(Message,"Compiling subgrammar #%ld...",(long)igram);
	ShowMessage(TRUE,wMessage,Message);
	(*(Gram.p_subgram))[igram].type = RNDtype;
	(*(Gram.p_subgram))[igram].oldindex = 0;
	(*(Gram.p_subgram))[igram].stop = (*(Gram.p_subgram))[igram].print
	= (*(Gram.p_subgram))[igram].printon = (*(Gram.p_subgram))[igram].printoff
	= (*(Gram.p_subgram))[igram].stepon = (*(Gram.p_subgram))[igram].stepoff
	= (*(Gram.p_subgram))[igram].traceon = (*(Gram.p_subgram))[igram].traceoff
	= (*(Gram.p_subgram))[igram].destru = (*(Gram.p_subgram))[igram].randomize
	= 0;
	(*(Gram.p_subgram))[igram].seed = NOSEED;
	(*(Gram.p_subgram))[igram].p_rule = NULL;
	if((ptrule = (t_rule**) GiveSpace((Size)(MaxRul+1)*sizeof(t_rule)))
								== NULL) return(ABORT);
	(*(Gram.p_subgram))[igram].p_rule = ptrule;
	for(i=1; i <= MaxRul; i++) {
		(*((*(Gram.p_subgram))[igram].p_rule))[i].p_leftarg
		= (*((*(Gram.p_subgram))[igram].p_rule))[i].p_rightarg = NULL;
		(*((*(Gram.p_subgram))[igram].p_rule))[i].p_leftcontext =
		(*((*(Gram.p_subgram))[igram].p_rule))[i].p_rightcontext = NULL;
		(*((*(Gram.p_subgram))[igram].p_rule))[i].p_leftflag = NULL;
		(*((*(Gram.p_subgram))[igram].p_rule))[i].p_rightflag = NULL;
		(*((*(Gram.p_subgram))[igram].p_rule))[i].mode
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].operator
		 	= (*((*(Gram.p_subgram))[igram].p_rule))[i].weight
		 	= (*((*(Gram.p_subgram))[igram].p_rule))[i].gotogram
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].gotorule
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].failedgram
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].failedrule
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].oldgramindex 
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].oldrulindex
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].repeat
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].repeatcontrol
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].stop
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].print
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].printon
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].printoff
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].stepon
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].stepoff
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].traceon
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].traceoff
			= (*((*(Gram.p_subgram))[igram].p_rule))[i].destru = 0;
		}
	return(0);
	}
ptr = &p; pp1 = &p1; pp2 = &p2; pp3 = &p3; pp4 = &p4;
p = &((*p_line)[0]); beforefirstrule = TRUE;
if((c=NextChar(ptr))  == '\0') return(0);
if(c == '/' && (*ptr)[1] == '/') return(0);	/* Found '//' */
if(c != '_' && !isalpha(c)) beforefirstrule = FALSE;
*pp1 = *ptr; q = &GRAMstring[0];
if(Match(FALSE,ptr,&q,strlen(GRAMstring))) {	/* Found 'gram#' */
	beforefirstrule = FALSE;
	(*ptr) += strlen(GRAMstring);	/* Skip subgram number */
	while(MySpace(**ptr)) (*ptr)++;
	while((**ptr) >= '0' && (**ptr) <= '9') (*ptr)++;
	while(!MySpace(**ptr) && (**ptr) != '\0') (*ptr)++;
	}
if(CheckEnd(**ptr) || CheckEnd(c=NextChar(ptr))) return(5);
if(SkipRem(ptr) != OK) return(10);
if(NextChar(ptr)  == '\0') return(0);
if(beforefirstrule && irul == 0) {
	if((type = GetSubgramType(ptr)) > -1) {
		(*(Gram.p_subgram))[igram].type = type;
		if(type == SUBtype || type == SUB1type || type == POSLONGtype) SUBthere = TRUE;
		if((c=NextChar(ptr))  == '\0' || c == '[') return(0);
		else return(35);
		}
	proc = FALSE;
	if(NextChar(ptr)  == '_') {
		while((j=GetProcedure(igram,ptr,0,&i,&i,&x,&y)) != RESUME) {
			if(j == ABORT) return(0);
			if(j > 0) {
				proc = TRUE;
				switch(j) {
					case 3:	/* _stop */
						(*(Gram.p_subgram))[igram].stop = TRUE;
						break;
					case 4:	/* _print */
						(*(Gram.p_subgram))[igram].print = TRUE;
						break;
					case 5:	/* _printOn */
						(*(Gram.p_subgram))[igram].printon = TRUE;
						break;
					case 6:	/* _printOff */
						(*(Gram.p_subgram))[igram].printoff = TRUE;
						break;
					case 7:	/* _stepOn */
						(*(Gram.p_subgram))[igram].stepon = TRUE;
						break;
					case 8:	/* _stepOff */
						(*(Gram.p_subgram))[igram].stepoff = TRUE;
						break;
					case 9:	/* _traceOn */
						(*(Gram.p_subgram))[igram].traceon = TRUE;
						break;
					case 10:	/* _traceOff */
						(*(Gram.p_subgram))[igram].traceoff = TRUE;
						break;
					case 11:	/* _destru */
						(*(Gram.p_subgram))[igram].destru = TRUE;
						Gram.hasproc = TRUE;
						break;
					case 12:	/* _randomize */
						(*(Gram.p_subgram))[igram].randomize = TRUE;
						WillRandomize = TRUE;
						break;
					case 13:	/* _mm */
						if(!NotFoundMetronom) {
							Println(wTrace,"Instruction '_mm()' should not appear more than once in the grammar");
							return(7);
							}
						NotFoundMetronom = FALSE;
						if(ChangeMetronom(j,x) != OK) return(7);
						break;
					case 14:	/* _striated */
					case 15:	/* _smooth */
						if(!NotFoundNatureTime) {
							Println(wTrace,"Instruction '_smooth' or '_striated' should not appear more than once in the grammar");
							return(7);
							}
						NotFoundNatureTime = FALSE;
						if(ChangeMetronom(j,x) != OK) return(7);
						break;
					case 16:	/* _srand */
						(*(Gram.p_subgram))[igram].seed = y;
						break;
					}
				}
			}
		}
	if(proc) {
		if(((c=NextChar(ptr)) != '\0') && c != '[') {
			Print(wTrace,"This line should contain only procedures\n");
			return(7);
			}
		else return(0);
		}
	}
*p_irul = (*(Gram.p_subgram))[igram].number_rule = ++irul;
i = incweight = 0;
if((**ptr) == '<' || (**ptr) == '\334') {
	if((w=GetArgument(1,ptr,&incweight,&initparam,&foundk,&x,&u,&v)) == INT_MAX){
		return(20);
		}
	else {
		if(foundk) {
			i = - w - 1;
			NotBPCase[5] = TRUE;
			if(i < 1 || i >= MAXPARAMCTRL) {
				sprintf(Message,"'<K%ld>' not accepted.  Range [1,%ld]\n",
					(long)i,(long)MAXPARAMCTRL-1);
				Print(wTrace,Message);
				return(20);
				}
			if(initparam == INT_MAX) {
				w = 127;
				}
			else {	/* Found '<Kx=y>' */
				if(initparam < 0) {
					sprintf(Message,"Negative weight '<K%ld=%ld>' not allowed...\n",
						(long)i,(long)initparam);
					Print(wTrace,Message);
					return(20);
					}
				if(ParamInit[i] == INT_MAX)
					ParamInit[i] = ParamValue[i] = (int) initparam;
				else {
					if(ParamInit[i] != initparam) {
						sprintf(Message,
							"Initial value of '<K%ld>' already set to %ld...\n",
								(long)i,(long)ParamInit[i]);
						Print(wTrace,Message);
						return(20);
						}
					}
				}
			}
		else {
			if(w == INT_MIN) w = INT_MAX;
			if(w < 0) {
				sprintf(Message,"Negative weight '<%ld>' not allowed...\n",(long)w);
				Print(wTrace,Message);
				return(20);
				}
			}
		}
	}
else {
	w = 127;
	}
if(SkipRem(ptr) != OK) return(10);
if(CheckEnd(**ptr)) return(5);
(*((*(Gram.p_subgram))[igram].p_rule))[irul].ctrl = i;
(*((*(Gram.p_subgram))[igram].p_rule))[irul].weight = w;
(*((*(Gram.p_subgram))[igram].p_rule))[irul].incweight = incweight;
type = (*(Gram.p_subgram))[igram].type;
c = NextChar(ptr);
if(CheckEnd(c)) return(5);
meta = FALSE;
if((imode=GetMode(ptr,type)) < 0) return(3);
(*((*(Gram.p_subgram))[igram].p_rule))[irul].mode = imode;
if(SkipRem(ptr) != OK) return(10);
if(CheckEnd(**ptr)) return(5);
if((operator=GetArg(ptr,pp1,pp2,pp3,pp4)) == -1) {
	(*((*(Gram.p_subgram))[igram].p_rule))[irul].operator = 0;
	return(8);
	}
(*((*(Gram.p_subgram))[igram].p_rule))[irul].operator = operator;
*p_needsnumber = TRUE;
*p_pleftcontext = *p_prightcontext = NULL; 
if((*pp4) == NULL) return(9);
h_flag = NULL;
if((pp_leftp=Encode(FALSE,FALSE,igram,irul,pp1,pp2,p_pleftcontext,p_prightcontext,&meta,1,
		&h_flag,FALSE,&result)) == NULL) {
	(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftarg = NULL;
	(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftcontext =
	(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightcontext = NULL;
	if(result < 0) return(result);
	else return(15); /* error in argument */
	}
/* ClearMarkers(&pp_leftp); */
if(*p_pleftcontext != NULL) {
	pp_a = (**p_pleftcontext)->p_arg; /* ClearMarkers(&pp_a); */
	}
if(*p_prightcontext != NULL) {
	pp_a = (**p_prightcontext)->p_arg; /* ClearMarkers(&pp_a); */
	}
(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftarg = pp_leftp;
(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftflag = h_flag;
if(h_flag != NULL) NotBPCase[6] = TRUE;
(*((*(Gram.p_subgram))[igram].p_rule))[irul].ismeta = meta;
(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftcontext = *p_pleftcontext;
(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightcontext = *p_prightcontext;
/* if(*p_pleftcontext != NULL || *p_prightcontext != NULL)
	NotBPCase[3] = TRUE;	Remote context(s) found */
h_flag = NULL;
NextChar(pp3); /* $$$  GetNilString(pp3); skip "lambda" */
if((pp_rightp=Encode(FALSE,FALSE,igram,irul,pp3,pp4,p_plx,p_prx,&meta,2,
		&h_flag,FALSE,&result)) == NULL) {
	(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightarg = NULL;
	if(result < 0) return(result);
	else return(15); /* error in argument */
	}
if(h_flag != NULL && (type == SUBtype || type == SUB1type || type == POSLONGtype)) return(49);
/* ClearMarkers(&pp_rightp); */
(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightarg = pp_rightp;
(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightflag = h_flag;
if(h_flag != NULL) NotBPCase[6] = TRUE;
if((i = FindLeftoffset(pp_leftp,pp_rightp,&lenc)) == -1) {
	(*((*(Gram.p_subgram))[igram].p_rule))[irul].leftoffset = 0;
	(*((*(Gram.p_subgram))[igram].p_rule))[irul].leftnegcontext = 0;
	sprintf(Message,"Can't have only negative contexts in argument!\n");
	Print(wTrace,Message);
	return(15);
	}
(*((*(Gram.p_subgram))[igram].p_rule))[irul].leftoffset = i;
(*((*(Gram.p_subgram))[igram].p_rule))[irul].leftnegcontext = lenc;
i = FindRightoffset(pp_leftp,pp_rightp,&lenarg);
(*((*(Gram.p_subgram))[igram].p_rule))[irul].rightoffset = i;
if((i=NumberWildCards(pp_leftp)) < (j=NumberWildCards(pp_rightp))) {
	ShowError(29,igram,irul);
	sprintf(Message,"(left side %ld, right side %ld)\n",(long)i,(long)j);
	Print(wTrace,Message);
	N_err++;
	}
*p_done = FALSE;
*p_onerulefound = TRUE;
if(tracecompile) ShowRule(&Gram,igram,irul,wTrace,FALSE,NULL,TRUE,TRUE,TRUE);
return(0);
}


ShowNotBP(void)
{
int i,j=1;
static char *err[] = {
"Rule(s) with 'lambda', 'empty', 'null' or 'nil' as right argument",	/* 0 */
" ", 		/* 1 */
"'SUB' or 'SUB1' or 'POSLONG' substitutions(s)",	/* 2 */
"No rule is valid for parsing.  Use '<->' instead of '-->'", /* 3 */
"Item contains polymetric structure(s)",		/* 4 */
"'<Kx>' controlled rule weight(s)",	/* 5 */
"'/flag/' programmed grammar(s)",	/* 6 */
"Using tool(s): '_destru','_goto','_failed','_repeat','_retro','_rndseq'", /* 7 */
"Grammar is empty!",	/* 8 */
"Period notation is not handled in grammars"	/* 9 $$$ suppressed */
	};

if(!CompiledGr || Gram.trueBP) return(OK);
BPActivateWindow(SLOW,wTrace);
sprintf(Message,"\nThis is not a true BP grammar.\nThe following features are not standard:\n");
Print(wTrace,Message);
for(i=0; i < MAXNOTBPCASES; i++) {
	if(NotBPCase[i]) {
		sprintf(Message,"[%ld] %s\n",(long)j,err[i]);
		Print(wTrace,Message);
		j++;
		}
	}
ShowSelect(CENTRE,wTrace);
if(!ScriptExecOn) Alert1("Not a true BP grammar...");
else PrintBehind(wTrace,"Not a true BP grammar...\n");
return(FAILED);
}


MaintainSelectionInGrammar(long pos,int dif)
{
if(pos <= GramSelStart) GramSelStart += dif;
if(pos <= GramSelEnd) GramSelEnd += dif;
return(OK);
}


CheckDeterminism(t_gram *p_gram)
{
int igram,irul,jrul,newrule,err;
tokenbyte **leftargi,**leftargj;
t_subgram subgram;
t_rule rule;

err = 0;
if(ClearWindow(NO,wTrace) == ABORT) return(ABORT);
SelectBehind(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
PrintBehind(wTrace,"\n");

for(igram=1; igram <= (*p_gram).number_gram; igram++) {
	subgram = (*((*p_gram).p_subgram))[igram];
	for(irul=1; irul <= subgram.number_rule; irul++) {
		PleaseWait();
		rule = (*(subgram.p_rule))[irul];
		leftargi = rule.p_leftarg;
		newrule = TRUE;
		for(jrul=(irul+1); jrul <= subgram.number_rule; jrul++) {
			rule = (*(subgram.p_rule))[jrul];
			leftargj = rule.p_leftarg;
			if(SameBuffer(leftargi,leftargj)) {
				if(newrule) {
					newrule = FALSE;
					Println(wTrace,"The following rules make the grammar non-deterministic:");
					ShowRule(p_gram,igram,irul,wTrace,FALSE,NULL,TRUE,FALSE,FALSE);
					err++;
					}
				ShowRule(p_gram,igram,jrul,wTrace,FALSE,NULL,TRUE,FALSE,FALSE);
				}
			}
		}
	}
return(err == 0);
}


SameBuffer(tokenbyte **p_a,tokenbyte **p_b)
{
long i;

for(i=ZERO;;i+=2L){
	if((*p_a)[i] != (*p_b)[i]) return(NO);
	if((*p_a)[i+1] != (*p_b)[i+1]) return(NO);
	if((*p_a)[i] == TEND && (*p_a)[i+1] == TEND) break;
	}
return(YES);
}