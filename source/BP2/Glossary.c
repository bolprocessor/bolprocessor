/* Glossary.c (BP2 version CVS) */

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



CompileGlossary(void)
// A glossary is a mixture of certain script commands and ‘Define…’ commands
// that are compiled as a context-free grammar the left arguments of which
// must be a single variable.
{
long pos,posmax,posdir,posline;
int i,is,j,r,length,irul,gap,meta,flagindex,changed,type,errors,undefined,keep,
	noteconventionmem;
char **p_line,*p,*q,*p1,*p2,**pp1,**pp2,line[MAXLIN],line2[MAXNAME+1];
t_rule **ptr;
tokenbyte **p_arg;
p_context ppl,ppr;
p_flaglist **h_flag;

if(DoSystem() != OK) return(ABORT);
// if(CompileOn) return(FAILED);
if(GlossGram.p_subgram != NULL && ReleaseGlossarySpace() != OK) return(ABORT);

p_line = NULL; r = OK;
pp1 = &p1; pp2 = &p2;
/* Check that grammar is compiled so that grammar variables, if any, are created */
if(!CompiledAl  || (!CompiledGr && (AddBolsInGrammar() > BolsInGrammar)))
	CompiledAl = FALSE;
if(!CompiledGr || !CompiledAl) {
	if((r=CompileGrammar(2)) == OK) {
		if(ResetRuleWeights(0) == ABORT) return(ABORT);
		}
	else return(FAILED);
	}
if(DoSystem() != OK) return(ABORT);

sprintf(Message,"Compiling '%s' glossary...",FileName[wGlossary]);
ShowMessage(TRUE,wMessage,Message);

if(p_Var == NULL && GetVariableSpace() != OK) return(ABORT);
if((r=ResetVariables(wGlossary)) != OK) return(r);
if(p_Flagname == NULL && GetFlagSpace() != OK) return(ABORT); /* Normally not used */
if((GlossGram.p_subgram=(t_subgram**) GiveSpace((Size)(2)*sizeof(t_subgram)))
					== NULL) return(ABORT);
GlossGram.number_gram = 1;
noteconventionmem = NoteConvention;

CompileOn++;
CompiledGl = TRUE; /* See remark at CompileInteraction() */
errors = 0;

/* Then create grammar using 'Define…' instructions */
pos = ZERO;
posmax = GetTextLength(wGlossary);
irul = (*(GlossGram.p_subgram))[1].number_rule = 0;
(*(GlossGram.p_subgram))[1].type = SUB1type;
(*(GlossGram.p_subgram))[1].oldindex = 0;
(*(GlossGram.p_subgram))[1].stop = (*(GlossGram.p_subgram))[1].print
	= (*(GlossGram.p_subgram))[1].printon = (*(GlossGram.p_subgram))[1].printoff
	= (*(GlossGram.p_subgram))[1].stepon = (*(GlossGram.p_subgram))[1].stepoff
	= (*(GlossGram.p_subgram))[1].traceon = (*(GlossGram.p_subgram))[1].traceoff
	= (*(GlossGram.p_subgram))[1].destru = (*(GlossGram.p_subgram))[1].randomize
	= 0;
(*(GlossGram.p_subgram))[1].seed = NOSEED;

MystrcpyHandleToString(MAXNAME,0,line2,p_ScriptLabelPart(144,0));	/* "Define" */
while(ReadLine(YES,wGlossary,&pos,posmax,&p_line,&gap) == OK) {
	if((*p_line)[0] != '\0') irul++;
	}
if((ptr = (t_rule**) GiveSpace((Size)(irul+1)*sizeof(t_rule))) == NULL) goto ERR;
(*(GlossGram.p_subgram))[1].p_rule = ptr;
GlossGram.trueBP = GlossGram.hasTEMP = GlossGram.hasproc = FALSE;
for(i=0; i <= irul; i++) {
	(*((*(GlossGram.p_subgram))[1].p_rule))[i].p_leftarg
	= (*((*(GlossGram.p_subgram))[1].p_rule))[i].p_rightarg = NULL;
	(*((*(GlossGram.p_subgram))[1].p_rule))[i].p_leftcontext =
	(*((*(GlossGram.p_subgram))[1].p_rule))[i].p_rightcontext = NULL;
	(*((*(GlossGram.p_subgram))[1].p_rule))[i].p_leftflag = NULL;
	(*((*(GlossGram.p_subgram))[1].p_rule))[i].p_rightflag = NULL;
	(*((*(GlossGram.p_subgram))[1].p_rule))[i].leftoffset = 0;
	(*((*(GlossGram.p_subgram))[1].p_rule))[i].rightoffset = 0;
	(*((*(GlossGram.p_subgram))[1].p_rule))[i].leftnegcontext = 0;
	(*((*(GlossGram.p_subgram))[1].p_rule))[i].mode = 1;
	(*((*(GlossGram.p_subgram))[1].p_rule))[i].gotogram
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].gotorule
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].failedgram
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].failedrule
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].oldgramindex 
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].oldrulindex
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].repeat
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].repeatcontrol
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].stop
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].print
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].printon
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].printoff
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].stepon
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].stepoff
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].traceon
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].traceoff
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].destru
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].incweight
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].ctrl
		= 0;
	(*((*(GlossGram.p_subgram))[1].p_rule))[i].weight
		= (*((*(GlossGram.p_subgram))[1].p_rule))[i].w = 127;
	(*((*(GlossGram.p_subgram))[1].p_rule))[i].operator = 1;
	}

irul = 1; pos = posline = ZERO;
posdir = -1L;
while(ReadLine(YES,wGlossary,&pos,posmax,&p_line,&gap) == OK) {
	PleaseWait();
	is = 0;
	if((*p_line)[is] == '[') {
		while((*p_line)[is] != ']' && (*p_line)[is] != '\0') is++;
		if((*p_line)[is] == '\0') goto NEXTLINE;
		is++;
		while(MySpace((*p_line)[is])) is++;
		}
	if((*p_line)[is] == '\0') goto NEXTLINE;
	type = 0; r = OK; posdir = -1L;
	if((r=ExecScriptLine(NULL,wGlossary,FALSE,FALSE,p_line,posline+gap,&posdir,
		&changed,&keep)) != OK) goto BADLINE;
	MystrcpyHandleToString(MAXLIN,is,line,p_line);
	p = &(line[0]); q = &(line2[0]); length = strlen(line2);
	/* Line should either start with "Define" or contain "-->" */
	if(Match(TRUE,&p,&q,length)) type = 1;
	else {
		if(strstr(line,Arrow[1]) != NULLSTR) {
			type = 2; length = 0;
			}
		else goto NEXTLINE;
		}
	*pp1 = &(line[length]); while(MySpace(**pp1)) (*pp1)++;
	if(((**pp1) < 'A' || (**pp1) > 'Z') && (**pp1) != '|') {
		sprintf(Message,
		"Left argument in glossary must be a variable, starting with uppercase character or '|'. Can't make sense of '%c'...\n",
		(**pp1));
		Print(wTrace,Message); r = FAILED;
		goto BADLINE;
		}
	*pp2 = *pp1; while(OkChar(**pp2) || (**pp2) == '|') (*pp2)++;
	j = GetVar(pp1,pp2);
	if(j < 1 || j > Jvar || p_VarStatus == NULL) {
		if(Beta) Alert1("Err. CompileGlossary(). j < 1 || j > Jvar || p_VarStatus == NULL");
		if(CompileOn) CompileOn--;
		MyDisposeHandle((Handle*)&p_line);
		return(ABORT);
		}
	if((*p_VarStatus)[j] & 4) {
		sprintf(Message,"Can't accept multiple definitions of variable '%s' in glossary.\n",
			*((*p_Var)[j]));
		Print(wTrace,Message); r = FAILED;
		goto BADLINE;
		}
	if((*p_VarStatus)[j] & 1) {
		sprintf(Message,"Variable '%s' is already derived in grammar. Probably a mistake.\n",
			*((*p_Var)[j]));
		Print(wTrace,Message);
		}
	*pp1 = &(line[length]); while(MySpace(**pp1)) (*pp1)++;
	ppl = ppr = NULL; h_flag = NULL;
	p_arg = Encode(FALSE,TRUE,0,irul,pp1,pp2,&ppl,&ppr,&meta,4,&h_flag,FALSE,&r);
	if(r != OK) goto ERR;
	if(p_arg == NULL) {
		r = FAILED; goto BADLINE;
		}
	(*((*(GlossGram.p_subgram))[1].p_rule))[irul].p_leftarg = p_arg;
	*pp1 = *pp2; while(MySpace(**pp1)) (*pp1)++;
	if(type == 2) {
		i = 0;
		while((**pp1) == Arrow[1][i] && (**pp1) != '\0') {
			(*pp1)++; i++;
			}
		if(i != strlen(Arrow[1])) {
			Print(wTrace,"Left argument in glossary may contain only a variable\n");
			r = FAILED;
			goto BADLINE;
			}
		while(MySpace(**pp1)) (*pp1)++;
		*pp2 = *pp1;
		}
	while((**pp2) != '\0') (*pp2)++;
	(*pp2)--;
	ppl = ppr = NULL; h_flag = NULL;
	p_arg = Encode(FALSE,TRUE,0,irul,pp1,pp2,&ppl,&ppr,&meta,8,&h_flag,TRUE,&r);
	if(r != OK) goto ERR;
	if(p_arg == NULL) {
		r = FAILED; goto BADLINE;
		}
	(*((*(GlossGram.p_subgram))[1].p_rule))[irul].p_rightarg = p_arg;
	irul++;
	goto NEXTLINE;
	 
BADLINE:
	if(type > 1) Println(wTrace,(*p_line));
	errors++;
NEXTLINE:
	posline = pos;
#if BP_CARBON_GUI
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	if((r=MyButton(1)) != FAILED) {
		if(r == OK) while((r = MainEvent()) != RESUME && r != STOP && r != EXIT);
		if(r == EXIT) goto ERR;
		if(r == STOP) r = ABORT;
		if(r == RESUME) {
			r = OK; EventState = NO;
			}
		if(r != OK) break;
		}
	r = OK;
	if(EventState) {
		r = EventState; goto ERR;
		}
#endif /* BP_CARBON_GUI */
	}
	
irul--;
MyDisposeHandle((Handle*)&p_line);
(*(GlossGram.p_subgram))[1].number_rule = irul;
NoteConvention = noteconventionmem;

// Look for undefined variables.
if(!errors && r == OK && irul > 0) {
	undefined = 0;
	for(j=1; j <= Jvar; j++) {
		if(!((*p_VarStatus)[j] & 4) && ((*p_VarStatus)[j] & 8)) undefined++;
		}
	if(undefined) {
		Print(wTrace,"\nUNDEFINED VARIABLES IN GLOSSARY:\n");
		for(j=1; j <= Jvar; j++) {
			if(!((*p_VarStatus)[j] & 4) && ((*p_VarStatus)[j] & 8)) { 
				sprintf(Message,"%s ",*((*p_Var)[j]));
				Print(wTrace,Message);
				errors++;
				}
			}
		Print(wTrace,"\n");
		}
	}
	
if(errors || r != OK) goto ERR;
sprintf(Message,"Errors: %ld",(long)errors);
if(irul == 0) ReleaseGlossarySpace();
else ShowMessage(TRUE,wMessage,Message);
CompiledGl = TRUE;
if(CompileOn) CompileOn--;
return(r);

ERR:
MyDisposeHandle((Handle*)&p_line);
if(errors) {
	Print(wTrace,"\nError(s) in 'Glossary' window.\n\n");
	ShowSelect(CENTRE,wTrace);
	sprintf(Message,"Errors: %ld",(long)errors);
	ShowMessage(TRUE,wMessage,Message);
	}
CompiledGl = FALSE;
if(CompileOn) CompileOn--;
NoteConvention = noteconventionmem;
ReleaseGlossarySpace();
return(ABORT);
}


UpdateGlossary(void)
{
if(!LoadedGl) {
	if(GetGlName(wData) == OK || GetGlName(wGrammar) == OK) {
		if(LoadGlossary(FALSE,FALSE) != OK) {
			CompiledGl = FALSE;
			return(FAILED);
			}
		}
	}
if(LoadedGl && !CompiledGl) return(CompileGlossary());
else CompiledGl = TRUE;
return(OK);
}