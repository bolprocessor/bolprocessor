/* CompileGrammar.c (BP3) */

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

int trace_compile_alphabet = 0;
int trace_compile_grammar = 0;

int CompileGrammar(int verbose,t_gram* p_gram) {
	int i,istart,igram,irul,gap,check,needsnumber,fatal,onerulefound,tracecompile,r,rep,
		dirtymem,done,changednumber;
	long pos,posmax,posline,posinstr,starttrace,origin,end,dummy,startsel,endsel;
	char *p,*q,**p_line,**p_line2;
	t_rule **ptr;
	Handle ptr1;

	dummy = ZERO;
	strcpy(LastSeen_scale,"");
	if(CheckEmergency() != OK) {
		Panic = TRUE; return(ABORT);
		}
	// SelectBehind(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
	if(!ScriptExecOn) PrintBehind(wTrace,"\n");
	fatal = changednumber = FALSE;
	p_gram->trueBP = p_gram->hasTEMP = p_gram->hasproc = WillRandomize = FALSE;
	NotBPCase[8] = NotFoundMetronom = NotFoundNatureTime = TRUE;

	if(IsEmpty(wGrammar)) {
		if(trace_compile_grammar) BPPrintMessage(0,odInfo,"Grammar is empty\n");
		CompiledGr = TRUE;
		return(MISSED);
		}
	if(verbose) BPPrintMessage(0,odInfo,"Compiling grammar...\n");

	TextGetSelection(&GramSelStart,&GramSelEnd,TEH[wGrammar]);
	startsel = GramSelStart;
	endsel = GramSelEnd;
	if(trace_compile_grammar) BPPrintMessage(0,odInfo,"startsel = %ld, endsel = %ld\n",(long)startsel,(long)endsel);

	CompileDate = (long) time(NULL);

	NoAlphabet = TRUE;
	GetAlphaName(wGrammar);
	N_err = 0;

	CompileOn++;

	if(!CompiledAl || (AddBolsInGrammar() > 0)) {
		CompiledAl = FALSE;
		if(CompileAlphabet() != OK) {
			if(CompileOn) CompileOn--;
			return(MISSED);
			}
		}
	if(!CompiledPt) {
		if((rep=CompilePatterns()) != OK) {
			if(CompileOn) CompileOn--;
			return(rep);
			}
		}
	starttrace = GetTextLength(wTrace);
	tracecompile = FALSE; rep = YES;
	if(!ScriptExecOn && 0 && DisplayProduce && (rep=Answer("Trace grammar compilation",'N')) == YES)
		tracecompile = TRUE;
	if(rep == ABORT) {
		if(CompileOn) CompileOn--;
		return(MISSED);
		}
	ReleaseGrammarSpace(p_gram,FALSE);
	if(GetGrammarSpace(p_gram) == ABORT || ResetVariables(wGrammar) != OK) {
		if(CompileOn) CompileOn--;
		ReleaseGrammarSpace(p_gram,FALSE);
		return(MISSED);
		}
	p_gram->number_gram = igram = 1;
	(*(p_gram->p_subgram))[1].number_rule = 0;
	done = TRUE; /* Flag meaning that current number of rules is correct. */
	(*(p_gram->p_subgram))[1].oldindex = 0;
	(*(p_gram->p_subgram))[1].type = RNDtype;
	(*(p_gram->p_subgram))[igram].stop = (*(p_gram->p_subgram))[igram].print
	= (*(p_gram->p_subgram))[igram].printon = (*(p_gram->p_subgram))[igram].printoff
	= (*(p_gram->p_subgram))[igram].stepon = (*(p_gram->p_subgram))[igram].stepoff
	= (*(p_gram->p_subgram))[igram].traceon = (*(p_gram->p_subgram))[igram].traceoff
	= (*(p_gram->p_subgram))[igram].destru = (*(p_gram->p_subgram))[igram].randomize
	= 0;
	(*(p_gram->p_subgram))[igram].seed = NOSEED;
	(*(p_gram->p_subgram))[1].p_rule = NULL;
	if((ptr = (t_rule**) GiveSpace((Size)(MaxRul+1)*sizeof(t_rule))) == NULL) {
			if(CompileOn) CompileOn--;
			return(ABORT);
			}
	(*(p_gram->p_subgram))[1].p_rule = ptr;
	for(i=1; i <= MaxRul; i++) {
		(*((*(p_gram->p_subgram))[1].p_rule))[i].p_leftarg
		= (*((*(p_gram->p_subgram))[1].p_rule))[i].p_rightarg = NULL;
		(*((*(p_gram->p_subgram))[1].p_rule))[i].p_leftcontext =
		(*((*(p_gram->p_subgram))[1].p_rule))[i].p_rightcontext = NULL;
		(*((*(p_gram->p_subgram))[1].p_rule))[i].p_leftflag = NULL;
		(*((*(p_gram->p_subgram))[1].p_rule))[i].p_rightflag = NULL;
		(*((*(p_gram->p_subgram))[1].p_rule))[i].mode
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].operator
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].weight
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].gotogram
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].gotorule
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].failedgram
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].failedrule
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].oldgramindex
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].oldrulindex
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].repeat
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].repeatcontrol
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].stop
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].print
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].printon
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].printoff
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].stepon
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].stepoff
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].traceon
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].traceoff
			= (*((*(p_gram->p_subgram))[1].p_rule))[i].destru
			= 0;
		}

	onerulefound = FALSE;

	pos = posline = ZERO;
	posmax = GetTextLength(wGrammar);
	if(verbose) ShowMessage(TRUE,wMessage,"Compiling subgrammar #1...");
	if(check_memory_use) BPPrintMessage(0,odInfo,"MemoryUsed start compilegrammar = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	for(i=0; i < MAXNOTBPCASES; i++) NotBPCase[i] = FALSE;
	NotBPCase[8] = NotBPCase[3] = TRUE;
	for(i=1; i < MAXPARAMCTRL; i++) {
		ParamInit[i] = ParamValue[i] = INT_MAX;
		ParamChan[i] = -1;
		}
	p_line = NULL;
	InitThere = 0; p_InitScriptLine = NULL;
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
			p_gram->hasTEMP = TRUE;
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
				ReleaseGrammarSpace(p_gram,FALSE);
				MyDisposeHandle((Handle*)&p_line);
				if(CompileOn) CompileOn--;
				return(MISSED);
				}
			SelectBehind(origin,end,TEH[wGrammar]);
			if((p_line2 = (char**) GiveSpace((Size)(MAXLIN * sizeof(char))))
				== NULL) {
				MyDisposeHandle((Handle*)&p_line);
				if(CompileOn) CompileOn--;
				return(ABORT);
				}
			if(ReadToBuff(YES,FALSE,wGrammar,&origin,end,&p_line2) != OK) {
				ReleaseGrammarSpace(p_gram,FALSE);
				MyDisposeHandle((Handle*)&p_line);
				if(CompileOn) CompileOn--;
				return(MISSED);
				}
			check = 2; // Will not create a script line
			rep = ExecScriptLine(NULL,wScript,check,FALSE,p_line2,dummy,&posinstr,&i,&i);
			if(rep == OK) {
				/* This will set InitThere to 1 if "Play ..." instruction has been found. */
				if(InitThere == 0) {
					InitThere = 2;
					if((p_InitScriptLine=(char**) GiveSpace((Size)
							(1 + MyHandleLen(p_line2)) * sizeof(char))) == NULL) {
						ReleaseGrammarSpace(p_gram,FALSE);
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
				ReleaseGrammarSpace(p_gram,FALSE);
				MyDisposeHandle((Handle*)&p_line);
				if(CompileOn) CompileOn--;
				return(MISSED);
				}
			goto NEXTLINE;
			}
			
		/* Skip headers */
		p = &(*p_line)[0]; q = &(FilePrefix[wAlphabet][0]);
		if((Match(TRUE,p_line,&q,4)) && p_gram->number_gram == 1
			&& (*(p_gram->p_subgram))[1].number_rule == 0) goto NEXTLINE;
		p = &(*p_line)[0]; q = &(FileOldPrefix[wAlphabet][0]);
		if((Match(TRUE,p_line,&q,4)) && p_gram->number_gram == 1
			&& (*(p_gram->p_subgram))[1].number_rule == 0) goto NEXTLINE;
		p = &(*p_line)[0]; q = &(FilePrefix[wInteraction][0]);
		if((Match(TRUE,p_line,&q,4)) && p_gram->number_gram == 1
			&& (*(p_gram->p_subgram))[1].number_rule == 0) goto NEXTLINE;
		p = &(*p_line)[0]; q = &(FilePrefix[wGlossary][0]);
		if((Match(TRUE,p_line,&q,4)) && p_gram->number_gram == 1
			&& (*(p_gram->p_subgram))[1].number_rule == 0) goto NEXTLINE;
		p = &(*p_line)[0]; q = &(FilePrefix[iSettings][0]);
		if((Match(TRUE,p_line,&q,4)) && p_gram->number_gram == 1
			&& (*(p_gram->p_subgram))[1].number_rule == 0) goto NEXTLINE;
		p = &(*p_line)[0]; q = &(FilePrefix[wTimeBase][0]);
		if((Match(TRUE,p_line,&q,4)) && p_gram->number_gram == 1
			&& (*(p_gram->p_subgram))[1].number_rule == 0) goto NEXTLINE;
		p = &(*p_line)[0]; q = &(FilePrefix[wCsoundResources][0]);
		if((Match(TRUE,p_line,&q,4)) && p_gram->number_gram == 1
			&& (*(p_gram->p_subgram))[1].number_rule == 0) goto NEXTLINE;
		p = &(*p_line)[0]; q = &(FilePrefix[wTonality][0]);
		if((Match(TRUE,p_line,&q,4)) && p_gram->number_gram == 1
			&& (*(p_gram->p_subgram))[1].number_rule == 0) goto NEXTLINE;
		p = &(*p_line)[0]; q = &(FilePrefix[iMidiDriver][0]);
		if((Match(TRUE,p_line,&q,4)) && p_gram->number_gram == 1
			&& (*(p_gram->p_subgram))[1].number_rule == 0) goto NEXTLINE;
		p = &(*p_line)[0]; q = &(FilePrefix[iObjects][0]);
		if((Match(TRUE,p_line,&q,4)) && p_gram->number_gram == 1
			&& (*(p_gram->p_subgram))[1].number_rule == 0) goto NEXTLINE;
		p = &(*p_line)[0]; q = &(FileOldPrefix[iObjects][0]);
		if((Match(TRUE,p_line,&q,4)) && p_gram->number_gram == 1
			&& (*(p_gram->p_subgram))[1].number_rule == 0) goto NEXTLINE;
			
		needsnumber = FALSE; igram = 0; irul = 0;
		if(trace_compile_grammar) BPPrintMessage(0,odInfo,"Parsing: %s\n",*p_line);
		i = ParseGrammarLine(p_gram,p_line,&onerulefound,tracecompile,&igram,&irul,
															&needsnumber,&done);
		if(i != 0) {
			if(EmergencyExit || (i < 0)) {
				MyDisposeHandle((Handle*)&p_line);
				ReleaseGrammarSpace(p_gram,FALSE);
				if(CompileOn) CompileOn--;
				if(Panic || EmergencyExit) return(ABORT);
				else return(i);
				}
			if(verbose) {
				Print(wTrace,"\n");
				fatal = ShowError(i,igram,irul);
				Print(wTrace,"??? ");
				Println(wTrace,*p_line);
				Print(wTrace,"\n");
				}
			N_err++;
			if(fatal) {
				if(verbose && !ScriptExecOn) BPPrintMessage(0,odError,"Fatal error found.  Compilation aborted...");
				else PrintBehindln(wTrace,"Fatal error found.  Compilation aborted...");
				break;
				}
			}
		if(InsertGramRuleNumbers && needsnumber && N_err == 0) {
			Renumber(p_gram,p_line,posline+gap,&pos,igram,irul,&posmax,&changednumber);
			}
		
	NEXTLINE:
		posline = pos;
		}  // while(Readline())

	END:
	MyDisposeHandle((Handle*)&p_line);
	if(verbose) BPPrintMessage(0,odInfo,"Parsing completed\n");

	// DisplayGrammar(p_gram,wData,TRUE,TRUE);

	if(trace_compile_grammar) BPPrintMessage(0,odInfo,"\n");
	/* if((*(p_gram->p_subgram))[p_gram->number_gram].number_rule > MaxRul) {
		my_sprintf(Message,"=> Err. number rules gram#%ld.",(long)p_gram->number_gram);
		BPPrintMessage(0,odError,"%s",Message);
		if(CompileOn) CompileOn--;
		Panic =  TRUE; // 2024-06-18
		return(ABORT);
		} */
	if((*(p_gram->p_subgram))[p_gram->number_gram].number_rule < 1) {
		ptr = (t_rule**) (*(p_gram->p_subgram))[p_gram->number_gram].p_rule;
		MyDisposeHandle((Handle*)&ptr);
		(*(p_gram->p_subgram))[p_gram->number_gram].p_rule = NULL;
		p_gram->number_gram--;
		}
	MaxGram = p_gram->number_gram;
	if(p_gram->number_gram == 0) {
		ptr1 = (Handle) p_gram->p_subgram;
		MyDisposeHandle(&ptr1);
		p_gram->p_subgram = NULL;
		}
	CheckGotoFailed(p_gram);
	if(verbose) BPPrintMessage(0,odInfo,"\nErrors: %ld\n",(long)N_err);
	for(i=1; i < MAXPARAMCTRL; i++) {
		if(ParamInit[i] == INT_MAX) ParamInit[i] = ParamValue[i] = 127;
		}
	if(Jflag > 0) for(i=1; i <= Jflag; i++) (*p_Flag)[i] = ZERO;
	if(verbose) {
		SelectBehind(starttrace,GetTextLength(wTrace),TEH[wTrace]);
		if(changednumber) SelectBehind(ZERO,ZERO,TEH[wGrammar]);
		}

	if((N_err == 0) && onerulefound) {
		CompiledGr = JustCompiled = TRUE;
		p_gram->trueBP = TRUE;
		NotBPCase[8] = FALSE;
		for(i=0; i < MAXNOTBPCASES; i++) {
			if(NotBPCase[i]) {
				p_gram->trueBP = FALSE; break;
				}
			}
		if(InsertGramCorrections) InsertSubgramTypes(p_gram);
		ResetRuleWeights(p_gram,0);
		if(CompileOn) CompileOn--;
		return(OK);
		}
	else {
		if(verbose) ShowSelect(CENTRE,wTrace);
		p_gram->trueBP = p_gram->hasTEMP = p_gram->hasproc = FALSE;
		if(CompileOn) CompileOn--;
		return(N_err == 0);
		}
	}


int InsertSubgramTypes(t_gram* p_gram)	/* Insert 'ORD', 'RND', etc. */
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
				my_sprintf(Message,"%s(%.4f) ",*((*p_GramProcedure)[13]),((double)n)/d);
				}
			else my_sprintf(Message,"%s(no clock) ",*((*p_GramProcedure)[13]));
			PrintBehind(wGrammar,Message);
			dif = strlen(Message);
			MaintainSelectionInGrammar(pos,dif);
			pos += dif; posmax += dif;
			NotFoundMetronom = FALSE;
			}
		if(NotFoundNatureTime) {
			if(Nature_of_time == STRIATED)
				my_sprintf(Message,"%s\n",*((*p_GramProcedure)[14]));
			else
				my_sprintf(Message,"%s\n",*((*p_GramProcedure)[15]));
			PrintBehind(wGrammar,Message);
			dif = strlen(Message);
			MaintainSelectionInGrammar(pos,dif);
			pos += dif; posmax += dif;
			NotFoundNatureTime = FALSE;
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
			my_sprintf(Message,"%s\n",SubgramType[(*(p_gram->p_subgram))[igram].type]);
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


int Renumber(t_gram* p_gram,char **p_line,long posline,long *p_pos,int igram,int irul,
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
line2[i] = '\0';	/* Grammar numbers are in the beginning of 'line'... */
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
		my_sprintf(line2,"%ld",(long)igram);
		PrintBehind(wGrammar,line2);
		dif = strlen(line2) - (pos2 - pos1);
		MaintainSelectionInGrammar(pos1,dif);
		if(pos1 < Sel1) Sel1 += dif; /* Used in PlaySelection() */
		if(pos1 < Sel2) Sel2 += dif; /* Used in PlaySelection() */
		*p_pos += dif; *p_posmax += dif; pos3 += dif;
		(*((*(p_gram->p_subgram))[igram].p_rule))[irul].oldgramindex = k;
		(*((*(p_gram->p_subgram))[igram].p_rule))[irul].oldrulindex = irul;
		if((*(p_gram->p_subgram))[igram].oldindex == 0) {
			(*(p_gram->p_subgram))[igram].oldindex = k;
			if(k == (igram - 1)) {
				if((*(p_gram->p_subgram))[k].oldindex == 0)
					(*(p_gram->p_subgram))[k].oldindex = k;
				}
			}
		}
	}
else {
	/* Insert 'gram#' and subgram index */
	*p_changednumber = TRUE;
	SelectBehind(posline,posline,TEH[wGrammar]);
	my_sprintf(line2,"%s%ld",GRAMstring,(long)igram);
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
	my_sprintf(line2,"%ld",(long)irul);
	PrintBehind(wGrammar,line2);
	dif = strlen(line2) - (pos2 - pos1);
	MaintainSelectionInGrammar(pos1,dif);
	if(pos1 < Sel1) Sel1 += dif;
	if(pos1 < Sel2) Sel2 += dif;
	*p_pos += dif; *p_posmax += dif;
	if(k != 0)  {
		if((*((*(p_gram->p_subgram))[igram].p_rule))[irul].oldgramindex == 0)
			(*((*(p_gram->p_subgram))[igram].p_rule))[irul].oldgramindex = igram;
		(*((*(p_gram->p_subgram))[igram].p_rule))[irul].oldrulindex = k;
		}
	return(OK);
	}
else {
/* No index found.  Insert it. */
NOTFOUND:
	if(insertedgram) my_sprintf(line2,"[%ld] ",(long)irul);
	else {
		pos3--;
		my_sprintf(line2,"[%ld]",(long)irul);
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


int CheckGotoFailed(t_gram* p_gram) {
	int i,igram,irul,ig,ir,newig,newir;

	for(igram=1; igram <= p_gram->number_gram; igram++) {
		for(irul=1; irul <= (*(p_gram->p_subgram))[igram].number_rule; irul++) {
			if((*((*(p_gram->p_subgram))[igram].p_rule))[irul].repeat > 0) {
				if((i=(*(p_gram->p_subgram))[igram].type) == SUBtype ||
						i == SUB1type || i == POSLONGtype) {
					my_sprintf(Message,
						"=> gram#%ld is 'SUB' or 'SUB1' or 'POSLONG' and should not contain '%s'.\n",
						(long)igram,*((*p_GramProcedure)[2]));
					N_err++;
					Print(wTrace,Message);
					}
				}
			if((newig=ig=(*((*(p_gram->p_subgram))[igram].p_rule))[irul].gotogram) > 0) {
				newir = ir = (*((*(p_gram->p_subgram))[igram].p_rule))[irul].gotorule;
				NewIndex(p_gram,&newig,&newir);
				if((i=(*(p_gram->p_subgram))[igram].type) == SUBtype ||
						i == SUB1type || i == POSLONGtype) {
					my_sprintf(Message,
						"=> gram#%ld is 'SUB' or 'SUB1' or 'POSLONG' and should not contain '%s'.\n",
						(long)igram,*((*p_GramProcedure)[0]));
					N_err++;
					Print(wTrace,Message);
					UpdateProcedureIndex(0,igram,irul,newig,newir,1);
					continue;
					}
				if(newig != ig || newir != ir) {
					(*((*(p_gram->p_subgram))[igram].p_rule))[irul].gotogram = newig;
					(*((*(p_gram->p_subgram))[igram].p_rule))[irul].gotorule = newir;
					UpdateProcedureIndex(0,igram,irul,newig,newir,0);
					}
				if(newig > p_gram->number_gram) {
					my_sprintf(Message,"gram#%ld[%ld] has incorrect grammar index in '%s'\n",
						(long)igram,(long)irul,*((*p_GramProcedure)[0]));
					N_err++;
					Print(wTrace,Message);
					UpdateProcedureIndex(0,igram,irul,newig,newir,1);
					}
				else {
					if(((i=(*(p_gram->p_subgram))[newig].type) == SUBtype ||
						i == SUB1type || i == POSLONGtype) && newir > 0) {
						my_sprintf(Message,
							"gram#%ld[%ld] contains '%s' addressing rule in 'SUB' or 'SUB1' or 'POSLONG' subgrammar.\n",
							(long)igram,(long)irul,*((*p_GramProcedure)[0]));
						N_err++;
						Print(wTrace,Message);
						UpdateProcedureIndex(0,igram,irul,newig,newir,1);
						}
					else {
						if(newir > (*(p_gram->p_subgram))[newig].number_rule) {
							my_sprintf(Message,"gram#%ld[%ld] has incorrect rule index in '%s'\n",(long)igram,(long)irul,*((*p_GramProcedure)[0]));
							N_err++;
							Print(wTrace,Message);
							UpdateProcedureIndex(0,igram,irul,newig,newir,2);
							}
						}
					}
				}
			if((newig=ig=(*((*(p_gram->p_subgram))[igram].p_rule))[irul].failedgram) > 0) {
				newir = ir = (*((*(p_gram->p_subgram))[igram].p_rule))[irul].failedrule;
				NewIndex(p_gram,&newig,&newir);
				if((i=(*(p_gram->p_subgram))[igram].type) == SUBtype ||
						i == SUB1type || i == POSLONGtype) {
					my_sprintf(Message,
						"=> gram#%ld is 'SUB' or 'SUB1' or 'POSLONG' and should not contain '%s'.\n",
						(long)igram,*((*p_GramProcedure)[1]));
					N_err++;
					Print(wTrace,Message);
					UpdateProcedureIndex(1,igram,irul,newig,newir,1);
					continue;
					}
				if(newig != ig || newir != ir) {
					(*((*(p_gram->p_subgram))[igram].p_rule))[irul].failedgram = newig;
					(*((*(p_gram->p_subgram))[igram].p_rule))[irul].failedrule = newir;
					UpdateProcedureIndex(1,igram,irul,newig,newir,0);
					}
				if(newig > p_gram->number_gram) {
					my_sprintf(Message,"gram#%ld[%ld] has incorrect grammar index in '%s'\n",
						(long)igram,(long)irul,*((*p_GramProcedure)[1]));
					N_err++;
					Print(wTrace,Message);
					UpdateProcedureIndex(1,igram,irul,newig,newir,1);
					}
				else {
					if(((i=(*(p_gram->p_subgram))[newig].type) == SUBtype ||
							i == SUB1type || i == POSLONGtype) && newir > 0) {
						my_sprintf(Message,
							"gram#%ld[%ld] contains '%s' addressing rule in 'SUB' or 'SUB1' or 'POSLONG' subgrammar.\n",
							(long)igram,(long)irul,*((*p_GramProcedure)[1]));
						N_err++;
						Print(wTrace,Message);
						UpdateProcedureIndex(1,igram,irul,newig,newir,1);
						}
					else {
						if(newir > (*(p_gram->p_subgram))[newig].number_rule) {
							my_sprintf(Message,"gram#%ld[%ld] has incorrect rule index in '%s'\n",
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


int UpdateProcedureIndex(int jproc,int igram,int irul,int ig,int ir,int mode) {
	return OK; // 2024-10-12
	long pos,posmax,pos1,pos2,posline;
	char c,*p,*q,**qq,**p_line;
	int i,k,dif,gap;

	pos = posline = ZERO;
	posmax = GetTextLength(wGrammar);
	p_line = NULL;
	while(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) == OK) {
	//	PleaseWait();
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
							my_sprintf(Message,"%ld",(long)ig); break;
						case 1:
							my_sprintf(Message,"%ld?",(long)ig); break;
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
							my_sprintf(Message,"%ld",(long)ir); break;
						case 2:
							my_sprintf(Message,"%ld?",(long)ir); break;
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


int NewIndex(t_gram* p_gram,int *p_ig, int *p_ir)
{
int igram,irul;
for(igram=1; igram <= p_gram->number_gram; igram++) {
	if(*p_ir == 0 && (*(p_gram->p_subgram))[igram].oldindex == *p_ig) {
		*p_ig = igram; return(OK);
		}
	for(irul=1; irul <= (*(p_gram->p_subgram))[igram].number_rule; irul++) {
		if(*p_ig == (*((*(p_gram->p_subgram))[igram].p_rule))[irul].oldgramindex
			&& *p_ir == (*((*(p_gram->p_subgram))[igram].p_rule))[irul].oldrulindex) {
			*p_ig = igram; *p_ir = irul; return(OK);
			}
		}
	}
return(OK);
}


int CompileAlphabet(void) {
int rep, i, j;
int **ptr1;
char **ptr2;

if(PrototypesLoaded) return(OK);

rep = MISSED;
N_err = 0;

// if(ReleaseObjectPrototypes() != OK) return(ABORT);

BPPrintMessage(0,odInfo,"Compiling alphabet...\n");
if(check_memory_use) BPPrintMessage(0,odInfo,"MemoryUsed start compilealphabet = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
if(!NoAlphabet && IsEmpty(wAlphabet) && (LoadAlphabet(-1) != OK)) goto ERR;

if(ReleaseAlphabetSpace() != OK) return(ABORT);

Jhomo = 0; Jbol = 2;	/* Counting will not include "_" and "-" */

if((rep=GetAlphabetSpace()) != OK) {	// This creates "_" and "-"
	ReleaseAlphabetSpace();
	goto ERR;
	}
if((rep=ReadAlphabet(TRUE)) != OK){		/* Just count */
	ReleaseAlphabetSpace();
	goto ERR;
	}
if(Jhomo > 0) {
	if(trace_compile_alphabet) BPPrintMessage(0,odInfo,"Found %d homomorphism(s)\n",Jhomo);
	if((p_Image = (int****) GiveSpace((Size)(Jhomo) * sizeof(int**))) == NULL)
		return(ABORT);
	if((p_NoteImage = (int****) GiveSpace((Size)(Jhomo) * sizeof(int**))) == NULL)
		return(ABORT);
	if((p_Homo = (char****) GiveSpace((Size)(Jhomo) * sizeof(char**))) == NULL) return(ABORT);
	for(i=0; i < Jhomo; i++) {
		if((ptr2 = (char**) GiveSpace((Size)HOMOSIZE)) == NULL) return(ABORT);
		(*p_Homo)[i] = ptr2;
		MystrcpyStringToTable(p_Homo,i,"\0");
		if((ptr1 = (int**) GiveSpace((Size)(Jbol) * sizeof(int))) == NULL) return(ABORT);
		(*p_Image)[i] = ptr1;
		/* Every homomorphism is set to identity */
		for(j=0; j < Jbol; j++) (*((*p_Image)[i]))[j] = j;
		if((ptr1 = (int**) GiveSpace((Size)(128) * sizeof(int))) == NULL) return(ABORT);
		(*p_NoteImage)[i] = ptr1;
		/* Every homomorphism is set to identity */
		for(j=0; j < 128; j++) (*((*p_NoteImage)[i]))[j] = 16384 + j;
		}
	}
BolsInGrammar = AddBolsInGrammar();
Jbol = Jbol + BolsInGrammar;
if(trace_compile_alphabet) BPPrintMessage(0,odInfo,"Jbol = %d, BolsInGrammar = %d\n",Jbol,BolsInGrammar);
Jhomo = 0;

MakeSoundObjectSpace(); // 2024-07-05

/* if((rep=GetAlphabetSpace()) != OK) {	// This creates "_" and "-"
	ReleaseAlphabetSpace();
	goto ERR;
	} */

Jbol = 2;
if((rep=ReadAlphabet(FALSE)) != OK) {	/* Now creating bols */
	ReleaseAlphabetSpace();
	goto ERR;
	}
CompiledAl = TRUE;
if(check_memory_use) BPPrintMessage(0,odInfo,"MemoryUsed end compilealphabet = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
return(OK);

ERR:
BPPrintMessage(0,odError,"=> Can't compile alphabet\n");
return(rep);
}


int ReadAlphabet(int justcount) {
	long pos,posmax;
	char *q,**p_line,line[MAXLIN],operatorbetweenquotes[MAXLIN];
	int done,foundoperator,foundoperatorthere,fatal,i,operatorinline,
		miknown=FALSE,inknown=FALSE,kbknown=FALSE,glknown=FALSE,tbknown=FALSE,
		csknown=FALSE,rep,gap;

	pos = ZERO; done = foundoperator = FALSE;
	posmax = GetTextLength(wAlphabet);

	CompileOn++;

	p_line = NULL;
	my_sprintf(operatorbetweenquotes,"\'%s\'",Arrowstring);
	while(ReadLine(YES,wAlphabet,&pos,posmax,&p_line,&gap) == OK) {
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
		if(trace_compile_alphabet) BPPrintMessage(0,odInfo,"Reading: %s\n",(*p_line));
		operatorinline = FALSE;
		MystrcpyHandleToString(MAXLIN,0,line,p_line);
		adjust_prefix(line); // Added 2024-06-13
		strcpy(*p_line, line);
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
			q = &(FilePrefix[wCsoundResources][0]);
			if(Match(TRUE,p_line,&q,4)) {
				csknown = TRUE; goto NEXTLINE;
				}
			}
	/*	if(!orknown) {
			q = &(FilePrefix[wMIDIorchestra][0]);
			if(Match(TRUE,p_line,&q,4)) {
				orknown = TRUE; goto NEXTLINE;
				}
			} */
		if(!tbknown) {
			q = &(FilePrefix[wTimeBase][0]);
			if(Match(TRUE,p_line,&q,4)) {
				tbknown = TRUE; goto NEXTLINE;
				}
			}
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
			if(trace_compile_alphabet) BPPrintMessage(0,odInfo,"\nFound homomorphism operator: %s\n",(*p_line));
			}
		else {
			if((*p_line)[0] == '-' && (*p_line)[1] == '-') {
				done = FALSE;
				if(!foundoperatorthere) {
					Print(wTrace,"=> Error in alphabet: section should contain the definition of an homomorphism.");
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
		if(!ScriptExecOn) BPPrintMessage(0,odError,"Alphabet is incorrect...");
		else PrintBehindln(wTrace,"Alphabet is incorrect...");
		return(MISSED);
		}
	else return(OK);
	}


int AddBolsInGrammar(void)
{
int i,j=0,rem;
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


int GetHomomorph(char **p_line,int justcount)
{
/* register */ int i,j;

i = 0; j = 0;
if((*p_line)[0] == '\0') {
	BPPrintMessage(0,odError,"=> Err. GetHomomorph(). (*p_line)[0] == '\0'");
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


int GetBols(char **p_line,int justcount,int operatorthere)
{
char c,**p_y,*p,*q;
int i,j,k,k1,k2,l,length,r;

l = MyHandleLen(p_line)-1;
if(trace_compile_alphabet) BPPrintMessage(0,odInfo, "Starting GetBols(%s) length of line = %d\n",(*p_line),l);
if((p_y = (char**) GiveSpace((Size)((BOLSIZE+1) * sizeof(char)))) == NULL) {
	return(26);
	}
r = 0;
for(i=0,k1=0; i <= l;) {
	if((length=GetBol(p_line,&i)) > BOLSIZE) {
		my_sprintf(Message,"\nMaximum length: %ld chars.\n",(long)BOLSIZE);
		Print(wTrace,Message);
		if(trace_compile_alphabet) BPPrintMessage(0,odError,Message);
	//	ShowError(22,0,0);
		MyDisposeHandle((Handle*)&p_y);
		return(26);
		}
	if(length == -1) {
		if(trace_compile_alphabet) BPPrintMessage(0,odError, "=> GetBols() failed, length = %d\n",length);
		MyDisposeHandle((Handle*)&p_y);
		return(27);
		}
	j = i + length;
	c = (*p_line)[j];
	if(!isspace(c) && c != '\0') {
		my_sprintf(Message,"Can't accept character \"%c\" in alphabet\n",c);
		Print(wTrace,Message);
		if(trace_compile_alphabet) BPPrintMessage(0,odError,"Can't accept character \"%c\" in alphabet. length = %d\n",c,length);
		r = ABORT; goto QUIT;
		} 
	(*p_line)[j++] = '\0';
	for(k=0; (i+k) < j; k++) (*p_y)[k] = (*p_line)[i+k];
	if(trace_compile_alphabet) BPPrintMessage(0,odInfo, "Will try CreateBol for (*p_y) = %s\n",(*p_y));
	k2 = CreateBol(TRUE,TRUE,p_y,justcount,FALSE,BOL);
	if(k2 < 0) {
		r = ABORT; goto QUIT;
		}
	if(k2 >= (16384+128)) {
		BPPrintMessage(0,odError,"=> Err. GetBols(). k2 >= (16384+128)");
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
			my_sprintf(Message,"'-->' not found!\n");
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


int GetBol(char **p_line,int *p_i)
{
int i,j;
char c,line[MAXLIN];

// firstc = (*p_line)[*p_i];
while(MySpace(c=(*p_line)[*p_i])) (*p_i)++;
i = (*p_i);
if(trace_compile_alphabet) BPPrintMessage(0,odInfo,"Getting bols in line: %s\n",(*p_line));
if((*p_line)[*p_i] == '\'') {
	/* Read terminal between single quotes */
	for(j=(*p_i)+1;(c=(*p_line)[j]) != '\0' && c != '\''; j++){};
	j++;
	if(trace_compile_alphabet) BPPrintMessage(0,odInfo,"terminal between single quotes j = %d\n",j);
	}
else {
	if(!OkBolChar((*p_line)[*p_i])) goto ERR;
	for(j=(*p_i)+1; OkBolChar2(c=(*p_line)[j]); j++) {
		if(c == '-') {
			Print(wTrace,"Found '-' in terminal symbol\n");
			goto ERR;
			}
		}
	if(trace_compile_alphabet) BPPrintMessage(0,odInfo,"normal terminal j = %d\n",j);
	}
if(trace_compile_alphabet) BPPrintMessage(0,odInfo,"length = %d\n",j-(*p_i));
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
(*p_Type)[j] = 0;
if(trace_compile_alphabet) BPPrintMessage(0,odError,"Can't make sense of \"%s\"\n",line);
my_sprintf(Message,"Can't make sense of \"%s\"\n",line);
Print(wTrace,Message);
return(-1);
}


int OkChar(char c)
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
		return(TRUE);
	}
return(FALSE);
}


int OkBolChar(char c)
// Characters accepted as first one of terminal name
{
if(islower(c) || isupper(c)) return(YES); /* Modified 23/11/2009 BB */
switch(c) {
	case '\'':
		return(YES);
	}
return(NO);
}


int OkBolChar2(char c)
// Characters accepted in terminal names (after the first one)
{
if(isdigit(c) || isalpha(c) || OkBolChar(c)) return(OK);
switch(c) {
	case '-':	/* Discarded in GetBol() */
	case '@':
	case '%':
	case '#':
	case '"':
	case '\'':
	case '`':
		return(YES);
		break;
	}
return(NO);
}
 

int CreateBol(int reload,int checknotes,char **p_x, int justcount, int mark, int type) {
	int j,ln,diff,jmax,cv,compiledptmem,dirtyalphabetmem;
	char **ptr,****p_t,*q,line[MAXLIN];
	// char **ptr,*q,line[MAXLIN];
	// char **p_t[MAXBOL];

	if(trace_compile_alphabet)
		BPPrintMessage(0,odInfo, "CreateBol() Jbol = %d, *px = “%s”\n",Jbol,*p_x);
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
	if(p_t == NULL) {
		BPPrintMessage(0,odError, "=> Err. CreateBol(). p_t == NULL\n");
		return ABORT;
		}
	diff = TRUE;
	for(j=0; j < MAXNIL; j++) {
//		BPPrintMessage(0,odError, "j = %d\n",j);
		if(Mystrcmp(p_x,NilString[j]) == 0) {
			ShowError(54,0,0);
			return(ABORT);
			}
		}
	if(trace_compile_alphabet) BPPrintMessage(0,odInfo, "jmax = %d\n",jmax);
	if(jmax > 0) {
		if(p_t == NULL) {
			BPPrintMessage(0,odError,"=> Err. CreateBol(). p_t == NULL");
			return(ABORT);
			}
		for(j=0; j < jmax; j++) {
	//		BPPrintMessage(0,odError, "j = %d\n",j);
			if((MyHandlecmp((*p_t)[j],p_x)) == 0) {
				diff = FALSE;
				break;
				}
			}
		}
	if(diff && checknotes) {
		/* Maybe it's a simple note */
		cv = NoteConvention;
		if(trace_compile_alphabet) BPPrintMessage(0,odInfo, "Checking notes, convention = %d\n",cv);
		for(j=0; j < 128; j++) {
			MystrcpyHandleToString(MAXLIN,0,line,(*(p_NoteName[cv]))[j]);
			q = &(line[0]); ln = strlen(line);
			if(Match(TRUE,p_x,&q,ln) && !isdigit((*p_x)[ln])) {
	FOUNDNOTE:
				j += (C4key - 60);
				if(j < 0 || j > 127) {
					BPPrintMessage(0,odError, "=> A simple note is out of range. Probably wrong value of C4 key number = %ld\n",(long)C4key);
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
		if(trace_compile_alphabet) {
			if(type == BOL) BPPrintMessage(0,odInfo, "Creating Bol[%d] = %s\n",j,*p_x);
			else BPPrintMessage(0,odInfo, "Creating Patt[%d] = %s\n",j,*p_x);
			}
		if(type == BOL) {
			if(reload) ObjectMode = ObjectTry = FALSE;
		//	if(Mystrcmp(p_x,"'-->'") == 0) Jfunc = j;
			}
		if(j >= Jbol) { // 2024-08-18
			MySetHandleSize((Handle*)&p_t,(Size)(j + 1) * sizeof(char**));
			MySetHandleSize((Handle*)&p_Type,(Size)(j + 1) * sizeof(char**));
			}
	//	BPPrintMessage(0,odInfo, "size of bol %s = %ld, j = %d\n",*p_x,(long)MyHandleLen(p_x),j);
		if((ptr=(char**) GiveSpace((Size)MyHandleLen(p_x)+1)) == NULL) return(ABORT);
		(*p_t)[j] = ptr;
	//	BPPrintMessage(0,odInfo, "j = %d, Jbol = %d\n",j,Jbol);
		MystrcpyHandleToHandle(0,&((*p_t)[j]),p_x);
		if(type == BOL && j > 1 && j <= Jbol) (*p_Type)[j] = 0;
		jmax++;
		}
	else if(type == PATTERN) {
		my_sprintf(Message,"\nCan't accept this as a time-pattern: %s\n",*p_x);
		Print(wTrace,Message);
		ShowError(48,0,0);
		return(ABORT);
		}
	if(type == BOL) {
		Jbol = jmax;
		}
	else Jpatt = jmax;
	return(j);
	}


int ParseGrammarLine(t_gram* p_gram,char** p_line,int *p_onerulefound,int tracecompile,int *p_igram,
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
igram = *p_igram = p_gram->number_gram;
irul = *p_irul = (*(p_gram->p_subgram))[igram].number_rule;

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
	if((*(p_gram->p_subgram))[igram].number_rule > MaxRul) {
		my_sprintf(Message,
			"=> Err. number rules gram#%ld. ",(long)p_gram->number_gram);
		BPPrintMessage(0,odError,"%s",Message);
		N_err++; return(2);
		}
	if(tracecompile) Print(wTrace,"------------------------\n");
	p_gram->number_gram += 1;
	if(p_gram->number_gram > MaxGram) {
		if(FirstGrammar) BPPrintMessage(0,odError,"=> Err. number grams = %ld  MaxGram = %ld\n",
				(long)p_gram->number_gram,(long)MaxGram);
		N_err++; return(1);
		}
	*p_igram = ++igram;
	(*(p_gram->p_subgram))[igram].number_rule = *p_irul = 0;
	my_sprintf(Message,"Compiling subgrammar #%ld...",(long)igram);
	ShowMessage(TRUE,wMessage,Message);
	(*(p_gram->p_subgram))[igram].type = RNDtype;
	(*(p_gram->p_subgram))[igram].oldindex = 0;
	(*(p_gram->p_subgram))[igram].stop = (*(p_gram->p_subgram))[igram].print
	= (*(p_gram->p_subgram))[igram].printon = (*(p_gram->p_subgram))[igram].printoff
	= (*(p_gram->p_subgram))[igram].stepon = (*(p_gram->p_subgram))[igram].stepoff
	= (*(p_gram->p_subgram))[igram].traceon = (*(p_gram->p_subgram))[igram].traceoff
	= (*(p_gram->p_subgram))[igram].destru = (*(p_gram->p_subgram))[igram].randomize
	= 0;
	(*(p_gram->p_subgram))[igram].seed = NOSEED;
	(*(p_gram->p_subgram))[igram].p_rule = NULL;
	if((ptrule = (t_rule**) GiveSpace((Size)(MaxRul+1)*sizeof(t_rule)))
								== NULL) return(ABORT);
	(*(p_gram->p_subgram))[igram].p_rule = ptrule;
	for(i=1; i <= MaxRul; i++) {
		(*((*(p_gram->p_subgram))[igram].p_rule))[i].p_leftarg
		= (*((*(p_gram->p_subgram))[igram].p_rule))[i].p_rightarg = NULL;
		(*((*(p_gram->p_subgram))[igram].p_rule))[i].p_leftcontext =
		(*((*(p_gram->p_subgram))[igram].p_rule))[i].p_rightcontext = NULL;
		(*((*(p_gram->p_subgram))[igram].p_rule))[i].p_leftflag = NULL;
		(*((*(p_gram->p_subgram))[igram].p_rule))[i].p_rightflag = NULL;
		(*((*(p_gram->p_subgram))[igram].p_rule))[i].mode
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].operator
		 	= (*((*(p_gram->p_subgram))[igram].p_rule))[i].weight
		 	= (*((*(p_gram->p_subgram))[igram].p_rule))[i].gotogram
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].gotorule
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].failedgram
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].failedrule
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].oldgramindex 
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].oldrulindex
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].repeat
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].repeatcontrol
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].stop
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].print
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].printon
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].printoff
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].stepon
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].stepoff
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].traceon
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].traceoff
			= (*((*(p_gram->p_subgram))[igram].p_rule))[i].destru = 0;
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
		(*(p_gram->p_subgram))[igram].type = type;
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
						(*(p_gram->p_subgram))[igram].stop = TRUE;
						break;
					case 4:	/* _print */
						(*(p_gram->p_subgram))[igram].print = TRUE;
						break;
					case 5:	/* _printOn */
						(*(p_gram->p_subgram))[igram].printon = TRUE;
						break;
					case 6:	/* _printOff */
						(*(p_gram->p_subgram))[igram].printoff = TRUE;
						break;
					case 7:	/* _stepOn */
						(*(p_gram->p_subgram))[igram].stepon = TRUE;
						break;
					case 8:	/* _stepOff */
						(*(p_gram->p_subgram))[igram].stepoff = TRUE;
						break;
					case 9:	/* _traceOn */
						(*(p_gram->p_subgram))[igram].traceon = TRUE;
						break;
					case 10:	/* _traceOff */
						(*(p_gram->p_subgram))[igram].traceoff = TRUE;
						break;
					case 11:	/* _destru */
						(*(p_gram->p_subgram))[igram].destru = TRUE;
						p_gram->hasproc = TRUE;
						break;
					case 12:	/* _randomize */
						(*(p_gram->p_subgram))[igram].randomize = TRUE;
						WillRandomize = TRUE;
						break;
					case 13:	/* _mm */
						if(!NotFoundMetronom) {
							Println(wTrace,"=> Instruction '_mm()' should not appear more than once in the grammar");
							return(7);
							}
						NotFoundMetronom = FALSE;
						if(ChangeMetronom(j,x) != OK) return(7);
						break;
					case 14:	/* _striated */
					case 15:	/* _smooth */
						if(!NotFoundNatureTime) {
							Println(wTrace,"=> Instruction '_smooth' or '_striated' should not appear more than once in the grammar");
							return(7);
							}
						NotFoundNatureTime = FALSE;
						if(ChangeMetronom(j,x) != OK) return(7);
						break;
					case 16:	/* _srand */
						(*(p_gram->p_subgram))[igram].seed = y;
						break;
					}
				}
			}
		}
	if(proc) {
		if(((c=NextChar(ptr)) != '\0') && c != '[') {
			Print(wTrace,"=> This line should contain only procedures\n");
			return(7);
			}
		else return(0);
		}
	}
*p_irul = (*(p_gram->p_subgram))[igram].number_rule = ++irul;
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
				my_sprintf(Message,"'<K%ld>' not accepted.  Range [1,%ld]\n",
					(long)i,(long)MAXPARAMCTRL-1);
				Print(wTrace,Message);
				return(20);
				}
			if(initparam == INT_MAX) {
				w = 127;
				}
			else {	/* Found '<Kx=y>' */
				if(initparam < 0) {
					my_sprintf(Message,"Negative weight '<K%ld=%ld>' not allowed...\n",
						(long)i,(long)initparam);
					Print(wTrace,Message);
					return(20);
					}
				w = initparam; // 2025-03-13
				if(ParamInit[i] == INT_MAX)
					ParamInit[i] = ParamValue[i] = (int) initparam;
				else {
					if(ParamInit[i] != initparam) {
						my_sprintf(Message,
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
				my_sprintf(Message,"Negative weight '<%ld>' not allowed...\n",(long)w);
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
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].ctrl = i;
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].weight = w;
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].incweight = incweight;
type = (*(p_gram->p_subgram))[igram].type;
c = NextChar(ptr);
if(CheckEnd(c)) return(5);
meta = FALSE;
if((imode=GetMode(ptr,type)) < 0) return(3);
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].mode = imode;
if(SkipRem(ptr) != OK) return(10);
if(CheckEnd(**ptr)) return(5);
if((operator=GetArg(ptr,pp1,pp2,pp3,pp4)) == -1) {
	(*((*(p_gram->p_subgram))[igram].p_rule))[irul].operator = 0;
	return(8);
	}
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].operator = operator;
*p_needsnumber = TRUE;
*p_pleftcontext = *p_prightcontext = NULL; 
if((*pp4) == NULL) return(9);
h_flag = NULL;
if((pp_leftp=Encode(p_gram,FALSE,FALSE,igram,irul,pp1,pp2,p_pleftcontext,p_prightcontext,&meta,1,
		&h_flag,FALSE,&result)) == NULL) {
	(*((*(p_gram->p_subgram))[igram].p_rule))[irul].p_leftarg = NULL;
	(*((*(p_gram->p_subgram))[igram].p_rule))[irul].p_leftcontext =
	(*((*(p_gram->p_subgram))[igram].p_rule))[irul].p_rightcontext = NULL;
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
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].p_leftarg = pp_leftp;
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].p_leftflag = h_flag;
if(h_flag != NULL) NotBPCase[6] = TRUE;
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].ismeta = meta;
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].p_leftcontext = *p_pleftcontext;
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].p_rightcontext = *p_prightcontext;
/* if(*p_pleftcontext != NULL || *p_prightcontext != NULL)
	NotBPCase[3] = TRUE;	Remote context(s) found */
h_flag = NULL;
NextChar(pp3); /* $$$  GetNilString(pp3); skip "lambda" */
if((pp_rightp=Encode(p_gram,FALSE,FALSE,igram,irul,pp3,pp4,p_plx,p_prx,&meta,2,
		&h_flag,FALSE,&result)) == NULL) {
	(*((*(p_gram->p_subgram))[igram].p_rule))[irul].p_rightarg = NULL;
	if(result < 0) return(result);
	else return(15); /* error in argument */
	}
if(h_flag != NULL && (type == SUBtype || type == SUB1type || type == POSLONGtype)) return(49);
/* ClearMarkers(&pp_rightp); */
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].p_rightarg = pp_rightp;
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].p_rightflag = h_flag;
if(h_flag != NULL) NotBPCase[6] = TRUE;
if((i = FindLeftoffset(pp_leftp,pp_rightp,&lenc)) == -1) {
	(*((*(p_gram->p_subgram))[igram].p_rule))[irul].leftoffset = 0;
	(*((*(p_gram->p_subgram))[igram].p_rule))[irul].leftnegcontext = 0;
	my_sprintf(Message,"Can't have only negative contexts in argument!\n");
	Print(wTrace,Message);
	return(15);
	}
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].leftoffset = i;
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].leftnegcontext = lenc;
i = FindRightoffset(pp_leftp,pp_rightp,&lenarg);
(*((*(p_gram->p_subgram))[igram].p_rule))[irul].rightoffset = i;
if((i=NumberWildCards(pp_leftp)) < (j=NumberWildCards(pp_rightp))) {
	ShowError(29,igram,irul);
	my_sprintf(Message,"NumberWildCards left side (%ld) is less than on right side (%ld)\n",(long)i,(long)j);
	Print(wTrace,Message);
	N_err++;
	}
*p_done = FALSE;
*p_onerulefound = TRUE;
if(tracecompile) ShowRule(p_gram,igram,irul,wTrace,FALSE,NULL,TRUE,TRUE,TRUE);
return(0);
}


int ShowNotBP(t_gram* p_gram)
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

if(!CompiledGr || p_gram->trueBP) return(OK);
BPActivateWindow(SLOW,wTrace);
my_sprintf(Message,"\nThis is not a true BP grammar.\nThe following features are not standard:\n");
Print(wTrace,Message);
for(i=0; i < MAXNOTBPCASES; i++) {
	if(NotBPCase[i]) {
		my_sprintf(Message,"[%ld] %s\n",(long)j,err[i]);
		Print(wTrace,Message);
		j++;
		}
	}
ShowSelect(CENTRE,wTrace);
if(!ScriptExecOn) BPPrintMessage(0,odError,"Not a true BP grammar...");
else PrintBehind(wTrace,"Not a true BP grammar...\n");
return(MISSED);
}


int MaintainSelectionInGrammar(long pos,int dif) {
	if(pos <= GramSelStart) GramSelStart += dif;
	if(pos <= GramSelEnd) GramSelEnd += dif;
	return(OK);
	}


int CheckDeterminism(t_gram *p_gram)
{
int igram,irul,jrul,newrule,err;
tokenbyte **leftargi,**leftargj;
t_subgram subgram;
t_rule rule;

err = 0;
SelectBehind(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
PrintBehind(wTrace,"\n");

for(igram=1; igram <= p_gram->number_gram; igram++) {
	subgram = (*(p_gram->p_subgram))[igram];
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
					ShowRule(p_gram,igram,irul,wTrace,FALSE,NULL,TRUE,FALSE,TRUE);
					err++;
					}
				ShowRule(p_gram,igram,jrul,wTrace,FALSE,NULL,TRUE,FALSE,TRUE);
				}
			}
		}
	}
return(err == 0);
}


int SameBuffer(tokenbyte **p_a,tokenbyte **p_b)
{
long i;

for(i=ZERO;;i+=2L){
	if((*p_a)[i] != (*p_b)[i]) return(NO);
	if((*p_a)[i+1] != (*p_b)[i+1]) return(NO);
	if((*p_a)[i] == TEND && (*p_a)[i+1] == TEND) break;
	}
return(YES);
}


void adjust_prefix(char *line) {
    char temp[MAXLIN];
    if(strncmp(line,"-mi.",4) == 0) {
        strcpy(temp, line + 4);
        strcpy(line,"-so.");
        strcat(line, temp);
		}
    if(strncmp(line,"-ho.",4) == 0) {
        strcpy(temp, line + 4);
        strcpy(line,"-al.");
        strcat(line, temp);
		}
	}

/******** COPY GRAMMAR ********/


void copy_grammar(t_gram *dest, t_gram *src, int verbose) {
    // Copy top-level grammar properties
    dest->number_gram = src->number_gram;
    dest->trueBP = src->trueBP;
    dest->hasTEMP = src->hasTEMP;
    dest->hasproc = src->hasproc;
    
    // Allocate memory for subgrammar pointers
    dest->p_subgram = (t_subgram **)malloc(sizeof(t_subgram *));
    if (!dest->p_subgram) {
        if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for subgrammars\n");
        return;
    	}
    
    // Allocate memory for the array of subgrammars
    // Add 1 to number_gram since indices start from 1
    *dest->p_subgram = (t_subgram *)malloc((src->number_gram + 1) * sizeof(t_subgram));
    if (!*dest->p_subgram) {
        if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for subgrammar array\n");
        free(dest->p_subgram);
        dest->p_subgram = NULL;
        return;
    	}
    
    // Copy each subgrammar (indices 1 to number_gram)
    for (int igram = 1; igram <= src->number_gram; igram++) {
        // Get the correct number of rules from the source
        int num_rules = (*(src->p_subgram))[igram].number_rule;
        // Copy subgrammar properties
        (*(dest->p_subgram))[igram].number_rule = num_rules;
        (*(dest->p_subgram))[igram].type = (*(src->p_subgram))[igram].type;
        (*(dest->p_subgram))[igram].oldindex = (*(src->p_subgram))[igram].oldindex;
        (*(dest->p_subgram))[igram].seed = (*(src->p_subgram))[igram].seed;
        (*(dest->p_subgram))[igram].stop = (*(src->p_subgram))[igram].stop;
        (*(dest->p_subgram))[igram].print = (*(src->p_subgram))[igram].print;
        (*(dest->p_subgram))[igram].printon = (*(src->p_subgram))[igram].printon;
        (*(dest->p_subgram))[igram].printoff = (*(src->p_subgram))[igram].printoff;
        (*(dest->p_subgram))[igram].stepon = (*(src->p_subgram))[igram].stepon;
        (*(dest->p_subgram))[igram].stepoff = (*(src->p_subgram))[igram].stepoff;
        (*(dest->p_subgram))[igram].traceon = (*(src->p_subgram))[igram].traceon;
        (*(dest->p_subgram))[igram].traceoff = (*(src->p_subgram))[igram].traceoff;
        (*(dest->p_subgram))[igram].destru = (*(src->p_subgram))[igram].destru;
        (*(dest->p_subgram))[igram].randomize = (*(src->p_subgram))[igram].randomize;
        // Allocate memory for rule pointers
        (*(dest->p_subgram))[igram].p_rule = (t_rule **)malloc(sizeof(t_rule *));
        if (!(*(dest->p_subgram))[igram].p_rule) {
            if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for rules in subgrammar %d\n", igram);
            return;
        	}
        // Allocate memory for the array of rules
        // Note: Rule indices also start from 1, adjust allocation size
        *((*(dest->p_subgram))[igram].p_rule) = (t_rule *)malloc((num_rules + 1) * sizeof(t_rule));
        if (!*((*(dest->p_subgram))[igram].p_rule)) {
            if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for rule array in subgrammar %d\n", igram);
            free((*(dest->p_subgram))[igram].p_rule);
            (*(dest->p_subgram))[igram].p_rule = NULL;
            return;
        	}
        // Copy each rule (rule indices start from 1)
        for (int irul = 1; irul <= num_rules; irul++) {
            // Use the correct pattern for accessing rules
            t_rule *dest_rule = &(*((*(dest->p_subgram))[igram].p_rule))[irul];
            t_rule *src_rule = &(*((*(src->p_subgram))[igram].p_rule))[irul];      
			// BPPrintMessage(0,odInfo, "GRAM#%d rule %d, w = %d\n", igram,irul,src_rule->w);
            // Copy rule properties
            dest_rule->mode = src_rule->mode;
            dest_rule->operator = src_rule->operator;
            dest_rule->weight = src_rule->weight;
            dest_rule->incweight = src_rule->incweight;
            dest_rule->w = src_rule->w;
            dest_rule->ctrl = src_rule->ctrl;
            dest_rule->ismeta = src_rule->ismeta;
            dest_rule->leftoffset = src_rule->leftoffset;
            dest_rule->rightoffset = src_rule->rightoffset;
            dest_rule->leftnegcontext = src_rule->leftnegcontext;
            dest_rule->oldgramindex = src_rule->oldgramindex;
            dest_rule->oldrulindex = src_rule->oldrulindex;
            dest_rule->gotogram = src_rule->gotogram;
            dest_rule->gotorule = src_rule->gotorule;
            dest_rule->failedgram = src_rule->failedgram;
            dest_rule->failedrule = src_rule->failedrule;
            dest_rule->repeat = src_rule->repeat;
            dest_rule->repeatcontrol = src_rule->repeatcontrol;
            dest_rule->stop = src_rule->stop;
            dest_rule->print = src_rule->print;
            dest_rule->printon = src_rule->printon;
            dest_rule->printoff = src_rule->printoff;
            dest_rule->stepon = src_rule->stepon;
            dest_rule->stepoff = src_rule->stepoff;
            dest_rule->traceon = src_rule->traceon;
            dest_rule->traceoff = src_rule->traceoff;
            dest_rule->destru = src_rule->destru;

			// Handle p_leftflag
			if (src_rule->p_leftflag) {
				// Allocate memory for the pointer
				dest_rule->p_leftflag = (p_flaglist **)malloc(sizeof(p_flaglist *));
				if (!dest_rule->p_leftflag) {
					fprintf(stderr, "Memory allocation failed for left flags\n");
					return;
					}
				// Allocate memory for the first flaglist
				*(dest_rule->p_leftflag) = (p_flaglist *)malloc(sizeof(p_flaglist));
				if (!*(dest_rule->p_leftflag)) {
					fprintf(stderr, "Memory allocation failed for left flag\n");
					free(dest_rule->p_leftflag);
					dest_rule->p_leftflag = NULL;
					return;
					}
				// Copy the first flaglist structure
				(**(dest_rule->p_leftflag)).x = (**(src_rule->p_leftflag)).x;
				(**(dest_rule->p_leftflag)).increment = (**(src_rule->p_leftflag)).increment;
				(**(dest_rule->p_leftflag)).refvalue = (**(src_rule->p_leftflag)).refvalue;
				(**(dest_rule->p_leftflag)).refflag = (**(src_rule->p_leftflag)).refflag;
				(**(dest_rule->p_leftflag)).paramcontrol = (**(src_rule->p_leftflag)).paramcontrol;
				(**(dest_rule->p_leftflag)).operator = (**(src_rule->p_leftflag)).operator;
				(**(dest_rule->p_leftflag)).p = NULL;
				// If source has a chain, copy it
				if ((**(src_rule->p_leftflag)).p) {
					// Allocate memory for the next pointer in chain
					(**(dest_rule->p_leftflag)).p = (p_flaglist **)malloc(sizeof(p_flaglist *));
					if (!(**(dest_rule->p_leftflag)).p) {
						fprintf(stderr, "Memory allocation failed for left flag chain\n");
						return;
						}
					copy_flaglist_chain((**(dest_rule->p_leftflag)).p, (**(src_rule->p_leftflag)).p);
					}
				}
			else dest_rule->p_leftflag = NULL;

			// Handle p_rightflag
			if (src_rule->p_rightflag) {
				// Allocate memory for the pointer
				dest_rule->p_rightflag = (p_flaglist **)malloc(sizeof(p_flaglist *));
				if (!dest_rule->p_rightflag) {
					fprintf(stderr, "Memory allocation failed for right flags\n");
					return;
					}
				// Allocate memory for the first flaglist
				*(dest_rule->p_rightflag) = (p_flaglist *)malloc(sizeof(p_flaglist));
				if (!*(dest_rule->p_rightflag)) {
					fprintf(stderr, "Memory allocation failed for right flag\n");
					free(dest_rule->p_rightflag);
					dest_rule->p_rightflag = NULL;
					return;
					}
				// Copy the first flaglist structure
			//    memcpy(*(dest_rule->p_rightflag), *(src_rule->p_rightflag), sizeof(p_flaglist));
				(**(dest_rule->p_rightflag)).x = (**(src_rule->p_rightflag)).x;
				(**(dest_rule->p_rightflag)).increment = (**(src_rule->p_rightflag)).increment;
				(**(dest_rule->p_rightflag)).refvalue = (**(src_rule->p_rightflag)).refvalue;
				(**(dest_rule->p_rightflag)).refflag = (**(src_rule->p_rightflag)).refflag;
				(**(dest_rule->p_rightflag)).paramcontrol = (**(src_rule->p_rightflag)).paramcontrol;
				(**(dest_rule->p_rightflag)).operator = (**(src_rule->p_rightflag)).operator;
				(**(dest_rule->p_rightflag)).p = NULL;
				// If source has a chain, copy it
				if ((**(src_rule->p_rightflag)).p) {
					// Allocate memory for the next pointer in chain
					(**(dest_rule->p_rightflag)).p = (p_flaglist **)malloc(sizeof(p_flaglist *));
					if (!(**(dest_rule->p_rightflag)).p) {
						fprintf(stderr, "Memory allocation failed for right flag chain\n");
						return;
						}
					copy_flaglist_chain((**(dest_rule->p_rightflag)).p, (**(src_rule->p_rightflag)).p);
					}
				}
			else dest_rule->p_rightflag = NULL;

            // Handle p_leftarg
            if (src_rule->p_leftarg) {
                // Calculate the size of the array using TEND markers
                int leftarg_count = 0;
                for (int idx = 0; ; idx += 2) {
                    int m = (*src_rule->p_leftarg)[idx];
                    int p = (*src_rule->p_leftarg)[idx+1];
                    leftarg_count += 2;
                    if (m == TEND && p == TEND) break;
                	}
                // Allocate memory for the token array
                dest_rule->p_leftarg = (tokenbyte **)malloc(sizeof(tokenbyte *));
                if (!dest_rule->p_leftarg) {
                    if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for left arg tokens\n");
                    return;
                	}
                *dest_rule->p_leftarg = (tokenbyte *)malloc(leftarg_count * sizeof(tokenbyte));
                if (!*dest_rule->p_leftarg) {
                    if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for left arg tokens\n");
                    free(dest_rule->p_leftarg);
                    dest_rule->p_leftarg = NULL;
                    return;
                	}
                // Copy the token array
                memcpy(*dest_rule->p_leftarg, *src_rule->p_leftarg, leftarg_count * sizeof(tokenbyte));
				}
			else dest_rule->p_leftarg = NULL;
            
            // Similar handling for p_rightarg
            if (src_rule->p_rightarg) {
                // Calculate the size of the array using TEND markers
                int rightarg_count = 0;
                for (int idx = 0; ; idx += 2) {
                    int m = (*src_rule->p_rightarg)[idx];
                    int p = (*src_rule->p_rightarg)[idx+1];
                    rightarg_count += 2;
                    if (m == TEND && p == TEND) break;
                	}
                // Allocate memory for the token array
                dest_rule->p_rightarg = (tokenbyte **)malloc(sizeof(tokenbyte *));
                if (!dest_rule->p_rightarg) {
                    if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for right arg tokens\n");
                    return;
                	}
                *dest_rule->p_rightarg = (tokenbyte *)malloc(rightarg_count * sizeof(tokenbyte));
                if (!*dest_rule->p_rightarg) {
                    if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for right arg tokens\n");
                    free(dest_rule->p_rightarg);
                    dest_rule->p_rightarg = NULL;
                    return;
                	}
                // Copy the token array
                memcpy(*dest_rule->p_rightarg, *src_rule->p_rightarg, rightarg_count * sizeof(tokenbyte));
				}
			else dest_rule->p_rightarg = NULL;


            
            // Handle p_leftcontext
            if (src_rule->p_leftcontext) {
                // Allocate new array of context pointers
                dest_rule->p_leftcontext = (p_context)malloc(sizeof(t_context*));
                if (!dest_rule->p_leftcontext) {
                    if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for left context\n");
                    return;
                	}  
                // Allocate the context struct
                *dest_rule->p_leftcontext = (t_context*)malloc(sizeof(t_context));
                if (!*dest_rule->p_leftcontext) {
                    if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for left context struct\n");
                    return;
                	}
                // Copy context properties
                (*dest_rule->p_leftcontext)->sign = (*src_rule->p_leftcontext)->sign;
                // Handle the p_arg array using TEND-based calculation
                if ((*src_rule->p_leftcontext)->p_arg) {
                    // Calculate the size of the array using TEND markers
                    int arg_count = 0;
                    for (int idx = 0; ; idx += 2) {
                        tokenbyte m = (*(*src_rule->p_leftcontext)->p_arg)[idx];
                        tokenbyte p = (*(*src_rule->p_leftcontext)->p_arg)[idx+1];
				//		BPPrintMessage(0,odInfo, "left context igram = %d irul = %d m = %d, p = %d\n",igram,irul,m,p);
                        arg_count += 2;
                        if (m == TEND && p == TEND) break;
                    	}
                    // Allocate memory for the token array
                    (*dest_rule->p_leftcontext)->p_arg = (tokenbyte **)malloc(sizeof(tokenbyte *));
                    if (!(*dest_rule->p_leftcontext)->p_arg) {
                        if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for left context args\n");
                        return;
                    	}
                    *(*dest_rule->p_leftcontext)->p_arg = (tokenbyte *)malloc(arg_count * sizeof(tokenbyte));
                    if (!*(*dest_rule->p_leftcontext)->p_arg) {
                        if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for left context arg tokens\n");
                        free((*dest_rule->p_leftcontext)->p_arg);
                        (*dest_rule->p_leftcontext)->p_arg = NULL;
                        return;
                   		}
               //     BPPrintMessage(1,odInfo,"Left context igram = %d, irule = %d, arg_count = %d sign = %d\n",igram,irul,arg_count,(*dest_rule->p_leftcontext)->sign);
                    // Copy the token array
					if ((*src_rule->p_leftcontext)->p_arg == NULL || (*dest_rule->p_leftcontext)->p_arg == NULL) {
						fprintf(stderr, "Error: p_arg is NULL!\n");
						if(verbose) BPPrintMessage(0,odError, "=> Error: igram = %d, irule = %d, p_arg is NULL!\n",igram,irul);
						return;
						}
                    memcpy(*(*dest_rule->p_leftcontext)->p_arg, *(*src_rule->p_leftcontext)->p_arg, arg_count * sizeof(tokenbyte));
                	} 
				else (*dest_rule->p_leftcontext)->p_arg = NULL;
            	} 
			else dest_rule->p_leftcontext = NULL;
            

            // Similar handling for p_rightcontext
            if (src_rule->p_rightcontext) {
                // Allocate new array of context pointers
                dest_rule->p_rightcontext = (p_context)malloc(sizeof(t_context*));
                if (!dest_rule->p_rightcontext) {
                    if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for right context\n");
                    return;
                	}
                // Allocate the context struct
                *(dest_rule->p_rightcontext) = (t_context *)malloc(sizeof(t_context));
                if (!*dest_rule->p_rightcontext) {
                    if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for right context struct\n");
                    return;
                	}
                // Copy context properties
                (*dest_rule->p_rightcontext)->sign = (*src_rule->p_rightcontext)->sign;
                // Handle the p_arg array using TEND-based calculation
                if ((*src_rule->p_rightcontext)->p_arg) {
                    // Calculate the size of the array using TEND markers
                    int arg_count = 0;
                    for (int idx = 0; ; idx += 2) {
                        tokenbyte m = (*(*src_rule->p_rightcontext)->p_arg)[idx];
                        tokenbyte p = (*(*src_rule->p_rightcontext)->p_arg)[idx+1];
				//		BPPrintMessage(0,odInfo, "right context igram = %d irul = %d m = %d, p = %d\n",igram,irul,m,p);
                        arg_count += 2;
                        if (m == TEND && p == TEND) break;
                    	}
                    // Allocate memory for the token array
                    (*dest_rule->p_rightcontext)->p_arg = (tokenbyte **)malloc(sizeof(tokenbyte *));
                    if (!(*dest_rule->p_rightcontext)->p_arg) {
                        if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for right context args\n");
                        return;
                    	}
                    *(*dest_rule->p_rightcontext)->p_arg = (tokenbyte *)malloc(arg_count * sizeof(tokenbyte));
                    if (!*(*dest_rule->p_rightcontext)->p_arg) {
                        if(verbose) BPPrintMessage(0,odError, "=> Memory allocation failed for right context arg tokens\n");
                        free((*dest_rule->p_rightcontext)->p_arg);
                        (*dest_rule->p_rightcontext)->p_arg = NULL;
                        return;
                    	}
               //     BPPrintMessage(1,odInfo,"Right context igram = %d, irule = %d, arg_count = %d sign = %d\n",igram,irul,arg_count,(*dest_rule->p_rightcontext)->sign);
				/*	printf("dest_rule->p_rightcontext: %p\n", (void *)dest_rule->p_rightcontext);
					printf("*dest_rule->p_rightcontext: %p\n", (void *)*dest_rule->p_rightcontext);
					printf("(*dest_rule->p_rightcontext)->p_arg: %p\n", (void *)(*dest_rule->p_rightcontext)->p_arg);
					printf("*(*dest_rule->p_rightcontext)->p_arg: %p\n", (void *)*(*dest_rule->p_rightcontext)->p_arg); */

                    // Copy the token array
					if ((*src_rule->p_rightcontext)->p_arg == NULL || (*dest_rule->p_rightcontext)->p_arg == NULL) {
						fprintf(stderr, "Error: p_arg is NULL!\n");
						if(verbose) BPPrintMessage(0,odError, "=> Error: igram = %d, irule = %d, p_arg is NULL!\n",igram,irul);
						return;
						}
                    memcpy(*(*dest_rule->p_rightcontext)->p_arg, *(*src_rule->p_rightcontext)->p_arg, arg_count * sizeof(tokenbyte));
                	} 
				else (*dest_rule->p_rightcontext)->p_arg = NULL;
            	} 
			else dest_rule->p_rightcontext = NULL;
			}
		}
	}

/* Proposed by Claude AI, not used
 void free_grammar(t_gram *gram) {
    if (!gram || !gram->p_subgram) return;
    
    // Iterate through subgrammars from 1 to number_gram
    for (int i = 1; i <= gram->number_gram; i++) {
        t_subgram *subgram = &(*(gram->p_subgram))[i];
        
        if (subgram->p_rule) {
            // Iterate through rules from 1 to number_rule
            for (int j = 1; j <= subgram->number_rule; j++) {
                t_rule *rule = &(*(subgram->p_rule))[j];
                
                // Free leftflag structures
                if (rule->p_leftflag) {
                    for (int k = 0; rule->p_leftflag[k]; k++) {
                        if (rule->p_leftflag[k]->p) {
                            for (int m = 0; rule->p_leftflag[k]->p[m]; m++) {
                                free(rule->p_leftflag[k]->p[m]);
                            }
                            free(rule->p_leftflag[k]->p);
                        }
                        free(rule->p_leftflag[k]);
                    }
                    free(rule->p_leftflag);
                }
                
                // Free rightflag structures
                if (rule->p_rightflag) {
                    for (int k = 0; rule->p_rightflag[k]; k++) {
                        if (rule->p_rightflag[k]->p) {
                            for (int m = 0; rule->p_rightflag[k]->p[m]; m++) {
                                free(rule->p_rightflag[k]->p[m]);
                            }
                            free(rule->p_rightflag[k]->p);
                        }
                        free(rule->p_rightflag[k]);
                    }
                    free(rule->p_rightflag);
                }
                
                // Free leftarg structure
                if (rule->p_leftarg) {
                    if (*rule->p_leftarg) {
                        free(*rule->p_leftarg);
                    }
                    free(rule->p_leftarg);
                }
                
                // Free rightarg structure
                if (rule->p_rightarg) {
                    if (*rule->p_rightarg) {
                        free(*rule->p_rightarg);
                    }
                    free(rule->p_rightarg);
                }
                
                // Free leftcontext structures
                if (rule->p_leftcontext) {
                    if (*rule->p_leftcontext) {
                        if ((*rule->p_leftcontext)->p_arg) {
                            if (*(*rule->p_leftcontext)->p_arg) {
                                free(*(*rule->p_leftcontext)->p_arg);
                            }
                            free((*rule->p_leftcontext)->p_arg);
                        }
                        free(*rule->p_leftcontext);
                    }
                    free(rule->p_leftcontext);
                }
                
                // Free rightcontext structures
                if (rule->p_rightcontext) {
                    if (*rule->p_rightcontext) {
                        if ((*rule->p_rightcontext)->p_arg) {
                            if (*(*rule->p_rightcontext)->p_arg) {
                                free(*(*rule->p_rightcontext)->p_arg);
                            }
                            free((*rule->p_rightcontext)->p_arg);
                        }
                        free(*rule->p_rightcontext);
                    }
                    free(rule->p_rightcontext);
                }
            }
            
            // Free the rule array
            if (*(subgram->p_rule)) {
                free(*(subgram->p_rule));
            }
            free(subgram->p_rule);
        }
    }
    
    // Free the subgrammar array
    if (*gram->p_subgram) {
        free(*gram->p_subgram);
    }
    free(gram->p_subgram);
    gram->p_subgram = NULL;
} */


// Helper function to recursively copy a chain of flaglists
void copy_flaglist_chain(p_flaglist **dest, p_flaglist **src) {
    if (!src || !*src) {
        *dest = NULL;
        return;
    	}
    // Allocate memory for the flaglist
    *dest = (p_flaglist *)malloc(sizeof(p_flaglist));
    if (!*dest) {
        fprintf(stderr, "Memory allocation failed for flaglist in chain\n");
        return;
    	}
    // Copy each field individually
    (*dest)->x = (*src)->x;
    (*dest)->increment = (*src)->increment;
    (*dest)->refvalue = (*src)->refvalue;
    (*dest)->refflag = (*src)->refflag;
    (*dest)->paramcontrol = (*src)->paramcontrol;
    (*dest)->operator = (*src)->operator;
    // Initialize p to NULL to avoid dangling pointer
    (*dest)->p = NULL;
    // If source has a next item in chain, copy it
    if ((*src)->p) {
        // Allocate memory for the next pointer in chain
        (*dest)->p = (p_flaglist **)malloc(sizeof(p_flaglist *));
        if (!(*dest)->p) {
            fprintf(stderr, "Memory allocation failed for flaglist chain\n");
            return;
        	}
        // Recursively copy the rest of the chain
        copy_flaglist_chain((*dest)->p, (*src)->p);
		}
	return;
	}

// Function to copy Gram_compile to Gram
void CopyGramcompileToGram(int verbose) {
    copy_grammar(&Gram,&Gram_compile,verbose);
	ReleaseGrammarSpace(&Gram_compile,FALSE);
	if(trace_compile_grammar) {
		BPPrintMessage(1,odInfo,"This is the grammar:");
		DisplayGrammar(&Gram,wData,TRUE,TRUE);
		}
	}