/* ProduceItems.c (BP3) */

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

int trace_produce_all = 0;

int ProduceItems(int w,int repeat,int template,tokenbyte ***pp_start)
/* Produce items. Start string is selection in window w or in buffer p_start */
{
tokenbyte **p_buff,***pp_buff,**p_a,***pp_a;
int i,ifunc,j,ch,splitmem,r,undefined,datamode,weightloss,hastabs,maxsounds,check;
long endofselection,size,lengthA;
unsigned long time_end_compute;

// BPPrintMessage(0,odInfo,"Maximum time allowed = %d seconds\n",MaxConsoleTime);
if(Improvize && ItemNumber == 0) {
	ShowMessage(TRUE,wMessage,"\n👉 Most of the messages will be discarded during the improvisation\n");
	if(!rtMIDI) BPPrintMessage(1,odInfo,"Only %ld items will be produced.\n",MaxItemsProduce);
	}
time_end_compute = getClockTime() + (MaxConsoleTime * 1000000);

if(CheckEmergency() != OK) return(ABORT);

if(((SetTimeOn || PrintOn || SoundOn || SelectOn) && !repeat)
					|| CompileOn || GraphicOn || PolyOn) return(RESUME);

if(CompileCheck() != OK) {
	Improvize = FALSE;
	BPPrintMessage(0,odError,"=> Compilation failed\n");
	return(MISSED);
	}

if(!template && CheckLoadedPrototypes() != OK) return(MISSED);

#if BP_CARBON_GUI_FORGET_THIS
if(GetTuning() != OK) return(ABORT);
#endif /* BP_CARBON_GUI_FORGET_THIS */

p_a = NULL; pp_a = &p_a;
pp_buff = &p_buff; p_buff = NULL;
r = OK;

ComputeOn++;
// if(ShowGraphic) CreateImageFile(-1.);
if(Panic) return ABORT;

SaidTooComplex = ShownBufferSize = FALSE;
if(ResetControllers) {
	for(ch=0; ch < MAXCHAN; ch++) {
		(*p_Oldvalue)[ch].volume = -1;
		(*p_Oldvalue)[ch].panoramic = -1;
		(*p_Oldvalue)[ch].pressure = -1;
		(*p_Oldvalue)[ch].pitchbend = -1;
		(*p_Oldvalue)[ch].modulation = -1;
		}
	}
// if(!PlaySelectionOn && ResetControllers) ResetMIDIControllers(YES,YES,NO);
if(!PlaySelectionOn && (InitThere == 2)) {
	if(!ScriptExecOn) {
		CurrentDir = WindowParID[wScript];
		CurrentVref = TheVRefNum[wScript];
		}
	check = 0; // This will create a script line
	if(ExecScriptLine(NULL,wScript,check,TRUE,p_InitScriptLine,size,&size,&i,&i) != OK) {
		r = MISSED; goto QUIT;
		}
	}
SetSelect(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
ifunc = weightloss = FALSE;
splitmem = SplitTimeObjects;

undefined = FALSE;
for(j=1; j <= Jvar; j++) {
	if(((*p_VarStatus)[j] & 2) && !((*p_VarStatus)[j] & 1) && !((*p_VarStatus)[j] & 4)) {
		undefined = TRUE; break;
		}
	}
if(!OutBPdata && undefined && !repeat && !IgnoreUndefinedVariables) {
	BPPrintMessage(0,odError,"\n=> Undefined variable(s) found and ignored:\n=> ");
	for(j = 1, i = 0; j <= Jvar; j++) {
		if(((*p_VarStatus)[j] & 2) && !((*p_VarStatus)[j] & 1)
									&& !((*p_VarStatus)[j] & 4))  {
			if(i > 0) BPPrintMessage(0,odError,", ");
			my_sprintf(Message,"%s",*((*p_Var)[j]));
			BPPrintMessage(0,odError,Message);
			i++;
			}
		}
	BPPrintMessage(0,odError,"\n");
	}
PedalOrigin = -1;
if(Jflag > 0) for(i=1; i <= Jflag; i++) (*p_Flag)[i] = ZERO;
#if BP_CARBON_GUI_FORGET_THIS

if(!repeat && !DeriveFurther) GetValues(TRUE);
#endif /* BP_CARBON_GUI_FORGET_THIS */
MaxDeriv = MAXDERIV;
if(pp_start != NULL) pp_a = pp_start;
else if(CreateBuffer(pp_a) != OK)  {
	r = MISSED; goto QUIT;
	}
if(check_memory_use) BPPrintMessage(0,odInfo,"MemoryUsed start MakeComputeSpace = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
if(MakeComputeSpace(MaxDeriv) != OK) {
	r = MISSED; goto QUIT;
	}
if(check_memory_use) BPPrintMessage(0,odInfo,"MemoryUsed end MakeComputeSpace = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
if(!ResetWeights && !NeverResetWeights && Varweight && !JustCompiled) {
	if((r=Answer("Reset rule weights to initial values",'N')) == OK) {
		Varweight = ResetRuleWeights(0);
		}
	if(r == NO && (r=Answer("OK to never reset rule weights",'Y')) == OK) {
		NeverResetWeights = TRUE;
		}
	if(r == ABORT) goto QUIT;
	}
JustCompiled = FALSE;

maxsounds = Jbol + Jpatt;
// BPPrintMessage(0,odInfo,"Jbol = %ld, Jpatt = %ld\n",(long)Jbol,(long)Jpatt);
ResizeObjectSpace(FALSE,maxsounds,0);

r = OK;
/* if(!PlaySelectionOn && ScriptRecOn) {
	my_sprintf(Message,"%.0f",(double) Qclock);
	MystrcpyStringToTable(ScriptLine.arg,0,Message);
	my_sprintf(Message,"%.0f",(double) Pclock);
	MystrcpyStringToTable(ScriptLine.arg,1,Message);
	AppendScript(45);
	if(UseBufferLimit) {
		AppendScript(46);
		my_sprintf(Message,"%ld",(long) BufferSize / 2L - 1L);
		MystrcpyStringToTable(ScriptLine.arg,0,Message);
		AppendScript(48);
		my_sprintf(Message,"%ld",(long) DeftBufferSize / 2L - 1L);
		MystrcpyStringToTable(ScriptLine.arg,0,Message);
		AppendScript(73);
		}
	else AppendScript(47);
	if(Quantize) {
		my_sprintf(Message,"%ld",(long) Quantization);
		MystrcpyStringToTable(ScriptLine.arg,0,Message);
		AppendScript(65);
		AppendScript(52);
		}
	else AppendScript(72);
	my_sprintf(Message,"%ld",(long) MIDIsetUpTime);
	MystrcpyStringToTable(ScriptLine.arg,0,Message);
	AppendScript(58);
	} */
StartCount();
SetSelect(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
LimTimeSet = LimCompute = FALSE;
TimeMax = MAXTIME;
MaxComputeTime = ZERO;
Nplay = 1; SynchroSignal = OFF;
if(!PlaySelectionOn && InitThere == 1) FirstTime = TRUE;
else FirstTime = FALSE;
if(!PlaySelectionOn && DisplayItems && !template) BPActivateWindow(SLOW,OutputWindow);

if(!PlaySelectionOn && (AllItems || template)) {
	ProduceAll(&Gram,pp_a,template);
	goto QUIT;
	}
if(pp_start == NULL && IsEmpty(w)) {
	if(w == wStartString) {
		SetSelect(ZERO,GetTextLength(wStartString),TEH[wStartString]);
		TextDelete(wStartString);
		Print(wStartString,"S\n");
		Dirty[wStartString] = FALSE;
		}
	else goto QUIT;
	}

MAKE:
if(!Improvize || ShowMessages) ShowMessage(TRUE,wMessage,"\nProducing item(s)...");
BufferSize = DeftBufferSize;
ProduceStackIndex = DisplayStackIndex = SkipFlag = FALSE;
/* if(!PlaySelectionOn && UseTimeLimit) {
	LimTimeSet = LimCompute = TRUE;
	} */
if(!PlaySelectionOn && Improvize) {
	if(DeriveFurther && Varweight) weightloss = TRUE;
	}
if((PlaySelectionOn || ResetWeights) && Varweight) {
	if(ResetRuleWeights(0) == ABORT) {
		if(CompileCheck() != OK) goto QUIT;
		}
	weightloss = TRUE;
	}
if((PlaySelectionOn || ResetFlags) && Jflag > 0)
	for(i=1; i <= Jflag; i++) (*p_Flag)[i] = ZERO;
if(!PlaySelectionOn && DisplayProduce) {
	SetSelect(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
	Print(wTrace,"\n");
	}
if(Improvize) {
	if(!rtMIDI && ItemNumber > MaxItemsProduce) {
		BPPrintMessage(0,odInfo,"%ld items have been produced.\n",(long)ItemNumber);
		Improvize =  FALSE;
		r = ABORT;
		goto QUIT;
		}
	if(rtMIDI) {
		if(ItemNumber > 10) ShowGraphic = ShowPianoRoll = FALSE;
	//	if(ItemNumber >= 20 || (r=ListenToEvents()) == ABORT) {
		if((r=ListenToEvents()) == ABORT) {
			Improvize = FALSE;
			BPPrintMessage(0,odInfo,"%ld items have been produced.\n",(long)ItemNumber);
			if(r == ABORT) BPPrintMessage(0,odInfo,"Production has been canceled by user.\n");
			r = ABORT;
			goto QUIT;
			}
		}
	}
if((r = stop(1,"ProduceItems")) != OK) {
	goto QUIT;
	}
if(pp_start != NULL) goto DOIT;
if(!PlaySelectionOn && DeriveFurther) {
	if(!Improvize && DisplayItems) {
		if((SelectionToBuffer(FALSE,FALSE,OutputWindow,pp_buff,&endofselection,PROD) != OK)
				|| (CopyBuf(pp_buff,pp_a) == ABORT)) {
			MyDisposeHandle((Handle*)pp_buff);
			goto QUIT;
			}
		MyDisposeHandle((Handle*)pp_buff);
		ShowMessage(TRUE,wMessage,"Deriving same item further...");
		}  /* Item is already in buffer if Improvize or not displayed */
	}
else {
	if(SelectSomething(w) != OK) {
		/* Default start string is "S" */
		i = 0; r = OK;
		(**(pp_a))[i++] = T0; (**(pp_a))[i++] = (tokenbyte) 10;
		(**(pp_a))[i++] = (tokenbyte) TEND;
		(**(pp_a))[i++] = (tokenbyte) TEND;
		}
	else {
		if((r=SelectionToBuffer(FALSE,FALSE,w,pp_buff,&endofselection,PROD)) == OK) {
			r = (CopyBuf(pp_buff,pp_a) > ZERO);
			}
		MyDisposeHandle((Handle*)pp_buff);
		}
	if(r != OK) goto QUIT;
	}

DOIT:
DataOrigin = GetTextLength(OutputWindow);
if(!PlaySelectionOn) SetSelect(DataOrigin,DataOrigin,TEH[OutputWindow]);
lengthA = LengthOf(pp_a);
if(lengthA < 1) {
	if(!DeriveFurther || PlaySelectionOn)
		BPPrintMessage(0,odError,"=> Empty start string. Can't produce anything");
	else
		BPPrintMessage(0,odError,"=> Item is empty. Can't derive further");
	goto QUIT;
	}
Final = FALSE;
Prod = 1.;
if(!PlaySelectionOn && !Improvize && ShowItem(-1,&Gram,0,pp_a,repeat,PROD,FALSE) == ABORT) goto QUIT;
if(pp_start == NULL) LastComputeWindow = w;

// BPPrintMessage(0,odError,"OK OK!\n");
// HERE WE DO IT ///////////////////////////////////////////////////////////
if((((r=Compute(pp_a,1,Gram.number_gram,&lengthA,&repeat,time_end_compute)) != OK) && !SkipFlag) || r == EXIT) goto QUIT;
////////////////////////////////////////////////////////////////////////////

// if(ShowGraphic) BPPrintMessage(0,odInfo, "After computing we'll try graphics\n");
if(!ShowGraphic && !PlaySelectionOn && !Improvize && DisplayItems)
	BPActivateWindow(QUICK,OutputWindow);
Final = TRUE;
ResetDone = ifunc = FALSE;
OkShowExpand = FALSE;
SplitTimeObjects = splitmem;
if(!PlaySelectionOn && Improvize) {
	my_sprintf(Message,"Item #%ld\n",(long)(ItemNumber + 1L));
	FlashInfo(Message);
	if(!rtMIDI && !template && Improvize) BPPrintMessage(0,odInfo,Message);
	ItemNumber++; // Needs to bee checked. Sometimes only 5 items created in improvize mode.
	if(SkipFlag) goto MAKE;
	if(!PlaySelectionOn && DisplayItems) {
	//	Dirty[OutputWindow] = TRUE;
		if(NumberCharsData < MAXCHARDATA) {
			datamode = DisplayMode(pp_a,&ifunc,&hastabs);
			if((r=PrintResult(datamode && hastabs,OutputWindow,hastabs,ifunc,pp_a)) != OK) goto QUIT;
			Print(OutputWindow,"\n");
			NumberCharsData += 20;
			}
		}
	if((rtMIDI || OutCsound || WriteMIDIfile || OutBPdata)
		&& ((r=PlayBuffer(pp_a,NO)) == ABORT || r == EXIT)) goto QUIT;
	goto MAKE;
	}
if(!StepProduce && !TraceProduce && !PlaySelectionOn
	&& ((r=ShowItem(-1,&Gram,0,pp_a,repeat,PROD,FALSE)) == ABORT || r == EXIT)) {
		BPPrintMessage(0,odError, "=> Failed in ShowItem()\n");
		goto QUIT;
		}
// BPPrintMessage(0,odInfo, "DisplayItems = %d\n",DisplayItems);
if(!PlaySelectionOn && DisplayItems) {
	if(NumberCharsData < MAXCHARDATA) {
		SetSelect(DataOrigin,DataOrigin,TEH[OutputWindow]);
		datamode = DisplayMode(pp_a,&ifunc,&hastabs);
	//	BPPrintMessage(0,odInfo, "\nBol Processor score:\n");
		if((r=PrintResult(datamode && hastabs,OutputWindow,hastabs,ifunc,pp_a)) != OK) goto QUIT;
		Print(OutputWindow,"\n\n");
		DataEnd = GetTextLength(OutputWindow);
		SetSelect(DataOrigin,DataEnd,TEH[OutputWindow]);
		NumberCharsData += 20;
	//	if(Improvize) ShowMessages = FALSE;
		}
	}
// if((!DisplayItems || PlaySelectionOn) && (rtMIDI || OutCsound || WriteMIDIfile)) {
if(ShowGraphic || rtMIDI || OutCsound || WriteMIDIfile  || OutBPdata) {
	r = PlayBuffer(pp_a,NO);
	if(r == RESUME) goto MAKE;
	goto QUIT;
	}
r = OK;

QUIT:
ComputeOn--;

// if(ResetControllers) ResetMIDIControllers(NO,NO,YES);
ResetMIDIfile();
// if(ResetMIDI(TRUE) == EXIT) r = EXIT;
// if(ResetControllers) ResetMIDIControllers(YES,NO,YES);
PedalOrigin = -1;
MyDisposeHandle((Handle*)pp_a);
// ReleaseComputeSpace();
return(r);
}


int ResetRuleWeights(int mode)
{
int igram,irul;
t_rule rule;

Varweight = FALSE;
if(Gram.p_subgram == NULL || Gram.number_gram < 1 || !CompiledGr) return(0);
for(igram=1; igram <= Gram.number_gram; igram++) {
	for(irul=1; irul <= (*(Gram.p_subgram))[igram].number_rule; irul++) {
		rule = (*((*(Gram.p_subgram))[igram].p_rule))[irul];
		if((rule.p_leftarg == NULL) || (rule.p_rightarg == NULL)) continue;
		switch(mode) {
			case 0:	/* Producing or analyzing items */
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].w = rule.weight;
				break;
			case 1:	/* Reset before learning weights */
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].w = 0;
				break;
			case 2:	/* Copy learned weights */
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].weight = rule.w;
				break;
			case 3:	/* Add learned weights */
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].weight += rule.w;
				break;
			}	
		if(!Varweight && (rule.incweight != 0)) Varweight = TRUE;
		if(!Flagthere && (rule.p_leftflag != NULL || rule.p_rightflag != NULL))
			Flagthere = TRUE;
		}
	}
ResetDone = TRUE;
return(Varweight);
}


int AnalyzeSelection(int learn)
{
int i,w,r,templates,all,dif,result,gap;
tokenbyte **p_a,***pp_a;
char c,*q,**p_line,remark[MAXLIN];
long origin,neworigin,end,pos,posmax;

if(!CompiledGr) return(MISSED);
w = LastEditWindow;
BPActivateWindow(SLOW,w);
p_a = NULL; pp_a = &p_a;
TextGetSelection(&origin, &end, TEH[w]);
SetSelect(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
if(end <= origin) {
	if(ScriptExecOn) Print(wTrace,"\n*** Can't analyze.  No item selected...\n");
	BPPrintMessage(0,odError,"=> Can't analyze. No item selected...");
	return(MISSED);
	}
/*
if(Gram.trueBP && !Gram.hasTEMP) {
	if(ScriptExecOn) Print(wTrace,"\n*** Templates have not been produced.\n");
	if((r=Answer("Templates have not been produced. Will you need them",'Y'))
		!= NO) return(ABORT);
	} */
templates = NO;
if(Gram.hasTEMP && (templates=Answer("Use templates",'Y')) == ABORT) return(ABORT);
pos = ZERO; all = NO;
p_line = NULL;
if(templates) {
	posmax = GetTextLength(wGrammar);
	while(posmax > 0 && GetTextChar(wGrammar,posmax-1) == '\r') posmax--;
	while(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) == OK) {
		if((*p_line)[0] == '\0') continue;
		if(Mystrcmp(p_line,"TEMPLATES:") == 0) break;
		}
	if((r=Answer("Show only first matching template",'Y')) == ABORT) {
		MyDisposeHandle((Handle*)&p_line); return(r);
		}
	if(r == NO) all = YES;
	}
MyDisposeHandle((Handle*)&p_line);
if(wTrace != w) SetSelect(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
MaxDeriv = MAXDERIV;
if(MakeComputeSpace(MaxDeriv) != OK) return(MISSED);
if(learn) ResetRuleWeights(1);
neworigin = origin;

#if BP_CARBON_GUI_FORGET_THIS
SwitchOn(NULL,wControlPannel,dAnalyze);
#endif /* BP_CARBON_GUI_FORGET_THIS */

/* Skip headers */
p_line = NULL;
while(ReadLine(YES,w,&origin,end,&p_line,&gap) == OK) {
	if((*p_line)[0] == '\0') goto NEXTLINE;
	for(i=0; i < WMAX; i++) {
		if(FilePrefix[i][0] == '\0') continue;
		q = &(FilePrefix[i][0]);
		if(Match(TRUE,p_line,&q,4)) {
			neworigin = origin; goto NEXTLINE;
			}
		}
NEXTLINE: ;
	}
origin = neworigin;
r = OK; p_line = NULL;
while(origin < end) {
	r = OK;
	PleaseWait();
	ShowMessage(TRUE,wMessage,"Analyzing selection...");
	while(MySpace(c=GetTextChar(w,origin)) || c == '\r') {
		origin++;
		if(origin >= end) goto END;
		}
	SetSelect(origin,end,TEH[w]);
	neworigin = origin;
	if(ReadLine(YES,w,&neworigin,end,&p_line,&i) != OK) goto END;
	GetInitialRemark(p_line,remark);
	neworigin = origin;
	if((r = SelectionToBuffer(FALSE,FALSE,w,pp_a,&neworigin,ANAL)) != OK) {
		MyDisposeHandle((Handle*)pp_a);
		BPActivateWindow(SLOW,wTrace);
		r = MISSED; goto END;
		}
	SetSelect(origin,neworigin,TEH[w]);
	BPActivateWindow(QUICK,w);
	SelectBehind(origin,origin,TEH[w]);
	origin = neworigin;
	PleaseWait();
	if(r == OK) r = AnalyzeBuffer(pp_a,learn,templates,all,pos,&result,remark);
	MyDisposeHandle((Handle*)pp_a);
	if(r == ABORT || r == EXIT) goto END;
	else r = OK;
	my_sprintf(Message,""); // Fixed by BB 2022-02-2
	if(result == OK) my_sprintf(Message,"[PASSED] ");
	if(result == MISSED) my_sprintf(Message,"[MISSED] ");
	dif = strlen(Message);
	if(dif > 0 && w != wTrace) {
		Print(w,Message);
		origin += dif; end += dif;
		}
	}
SetSelect(end,end,TEH[w]);
ShowSelect(CENTRE,w);
if(learn) {
	if((r=Answer("Add infered weights\nto current weights",'Y'))
		== ABORT) {
			ResetRuleWeights(0); r = OK; goto END;
			}
	if(r == OK) ResetRuleWeights(3);
	else {
		if((r=Answer("Keep infered weights\nin current grammar",'Y'))
			!= OK) {
			ResetRuleWeights(0); r = OK; goto END;
			}
		ResetRuleWeights(2);
		}
	if((r=Answer("Update grammar window\nwith new weights",'Y'))
		!= OK) {
			r = OK; goto END;
			}
	else Dirty[wGrammar] = TRUE;
	return(AdjustWeights());
	}
END:
MyDisposeHandle((Handle*)&p_line);
#if BP_CARBON_GUI_FORGET_THIS
SwitchOff(NULL,wControlPannel,dAnalyze);
#endif /* BP_CARBON_GUI_FORGET_THIS */
return(r);
}



int LearnWeights(void)
{
if(ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn || CompileOn || GraphicOn
	|| PolyOn) return(RESUME);
if(CompileCheck() != OK) return(MISSED);
if(ShowNotBP() != OK) return(OK);
return(AnalyzeSelection(TRUE));
}


int SetWeights(void)
{
int r,w,igram,irul;

if(CompileCheck() != OK) return(MISSED);
if(Gram.p_subgram == NULL || Gram.number_gram < 1) return(MISSED);
#if BP_CARBON_GUI_FORGET_THIS
if((r=Answer("Save current weights",'Y')) == ABORT) return(OK);
if(r == OK) SaveWeights();
#endif /* BP_CARBON_GUI_FORGET_THIS */

TRY:
if((r=AnswerWith("Set weights to...","0",Message)) == ABORT) return(OK);
w = (int) atol(Message); 	/* Don't use atoi() because int's are 4 bytes */
if(w < 0) {
	BPPrintMessage(0,odError,"=> Weights should not be negative"); goto TRY;
	}
for(igram=1; igram <= Gram.number_gram; igram++) {
	for(irul=1; irul <= (*(Gram.p_subgram))[igram].number_rule; irul++) {
		(*((*(Gram.p_subgram))[igram].p_rule))[irul].weight = w;
		}
	}
return(AdjustWeights());
}


int AdjustWeights(void)		/* Copy weights in memory to grammar window */
{
int i,j,igram,irul,newsubgram,dif,gap;
long pos,posline,posmax,pos1,pos2;
char **p_line,*p,*q,c;

if(IsEmpty(wGrammar)) return(0);
pos = posline = ZERO; igram = irul = 1;
posmax = GetTextLength(wGrammar);
ShowMessage(TRUE,wMessage,"Copying weights to all rules...");
p_line = NULL;
while(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) == OK) {
	PleaseWait();
	if((*p_line)[0] == '\0') goto NEXTLINE;
	j = MyHandleLen(p_line); while(j > 0 && MySpace((*p_line)[j])) j--;
	if(j == 0) goto NEXT;
	for(i=0; i < WMAX; i++) {
		if(FilePrefix[i][0] == '\0') continue;
		q = &(FilePrefix[i][0]);
		if(Match(TRUE,p_line,&q,4)) goto NEXT;
		}
	if(Mystrcmp(p_line,"DATA:") == 0) goto END;
	if(Mystrcmp(p_line,"COMMENT:") == 0) goto END;
	if(Mystrcmp(p_line,"TIMEPATTERNS:") == 0) {
		do {
			if(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) != OK) goto END;
			if((*p_line)[0] == '\0') continue;
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		goto NEXT;
		}
	if(Mystrcmp(p_line,"TEMPLATES:") == 0) {
		do {
			if(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) != OK) goto END;
			if((*p_line)[0] == '\0') continue;
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		goto NEXT;
		}
	newsubgram = FALSE;
	if((*p_line)[0] == '-' && (*p_line)[MyHandleLen(p_line)-1] == '-') {
		newsubgram = TRUE;
		for(i=0; i < MAXARROW; i++) {
			if(strstr(*p_line,Arrow[i]) != NULLSTR) newsubgram = FALSE;
			}
		}
	if(newsubgram) {
		igram++; irul = 1;
		goto NEXT;
		}
	i = 0;
	p = &(*p_line)[i]; if(irul < 2 && GetSubgramType(&p) > -1) goto NEXT;
	p = &(*p_line)[i]; if(irul < 2 && SkipGramProc(&p) == OK) goto NEXT;
	p = &(*p_line)[i]; q = &GRAMstring[0];
	if(Match(FALSE,p_line,&q,5)) {
		i += 5; while(i < j && (!MySpace(c=(*p_line)[i]))) i++; 
		}
	while(MySpace(c=(*p_line)[i]) && i < j) i++;
	if((*p_line)[i] == '[') {
		while((*p_line)[++i] != ']' && i < j){};
		i++;
		}
	if(i >= j) goto NEXT;
	for(i=i; i < j; i++) {
		while(MySpace(c=(*p_line)[i]) && i < j) i++;
		if((*p_line)[i] == '[')
			while((*p_line)[++i] != ']' && i < j){};
		if((*p_line)[i] == '<' || (*p_line)[i] == '\334') {
			pos1 = posline + gap + i + 1; pos2 = pos1;
			while((*p_line)[++i] != '>' && (*p_line)[i] != '\334' && i < j) pos2++;
			SetSelect(pos1,pos2,TEH[wGrammar]);
			TextDelete(wGrammar);
			my_sprintf(Message,"%ld",
				(long)((*((*(Gram.p_subgram))[igram].p_rule))[irul].weight));
			Print(wGrammar,Message);
			dif = strlen(Message) - (pos2 - pos1);
			pos += dif; posmax += dif;
			break;
			}
		else {	/* No weight found.  Create one. */
			pos1 = posline + gap + i;
			SetSelect(pos1,pos1,TEH[wGrammar]);
			my_sprintf(Message," <%ld> ",
				(long)((*((*(Gram.p_subgram))[igram].p_rule))[irul].weight));
			Print(wGrammar,Message);
			dif = strlen(Message);
			pos += dif; posmax += dif;
			break;
			}
		}
	irul++;
NEXT:
	posline = pos;
NEXTLINE: ;
	}
END:
MyDisposeHandle((Handle*)&p_line);
return(OK);
}


int ProduceAll(t_gram *p_gram,tokenbyte ***pp_a,int template) {
	int i,igram,irul,irep,r,showmessages,depth,weightloss,endgram,mode;
	long maxdepth,length,****p_flag,****p_weight;
	tokenbyte ****p_stack;
	OSErr io;
	unsigned long time_end_compute;

	if(template && ShowNotBP() != OK) return(OK);
	p_flag = NULL; p_weight = NULL;
	depth = 0;
	maxdepth = 20L;
	ProduceStackIndex = DisplayStackIndex = SkipFlag = FALSE;

	time_end_compute = getClockTime() + (MaxConsoleTime * 1000000);
	HideMessages = TRUE;

	if(Varweight) {
		if(ResetRuleWeights(0) == ABORT) {
			if(CompileCheck() != OK) return(OK);
			}
		weightloss = TRUE;
		}
	if(!template) my_sprintf(Message,"Computing all possible items...\n");
	else my_sprintf(Message,"Computing templates...\n");
	BPPrintMessage(0,odInfo,Message);
	(**(pp_a))[0] = T0; (**(pp_a))[1] = 10; // 'S'
	(**(pp_a))[2] = TEND; (**(pp_a))[3] = TEND;
	if((p_stack = (tokenbyte****) GiveSpace((Size) maxdepth * sizeof(tokenbyte**))) == NULL) return(ABORT);
	if(Jflag > 0) {
		for(i=1; i <= Jflag; i++) (*p_Flag)[i] = ZERO;
		if((p_flag = (long****) GiveSpace((Size) maxdepth * sizeof(long**))) == NULL) {
			r = ABORT; goto END;
			}
		}
	ResetRuleWeights(0);
	if((p_weight=(long****) GiveSpace((Size) maxdepth * sizeof(long**))) == NULL) {
		r = ABORT; goto END;
		}
	if(!template) {
		DataOrigin = GetTextLength(OutputWindow);
		SetSelect(DataOrigin,DataOrigin,TEH[OutputWindow]);
		}
	Prod = 1.; r = OK;
	length = LengthOf(pp_a);
	if(template) {
		mode = TEMP;
		endgram = LastStructuralSubgrammar();
		if(endgram == ABORT) goto END;
		if(endgram == 0) {
			BPPrintMessage(0,odError,"=> This grammar has no structural rules\nand does not require templates\n");
			goto END;
			}
		if((r=DeleteTemplates()) != OK) goto END;
		}
	else {
		mode = PROD;
		endgram = (*p_gram).number_gram;
/*		if((r = Answer("Try all rules\nin all grammars",'Y')) == ABORT) goto END;
		if(r == OK) endgram = (*p_gram).number_gram;
		else {
			r = OK;
			if((endgram=LastGrammarWanted((*p_gram).number_gram)) == ABORT) goto END;
			} */
		}
	igram = irul = irep = 1;
	showmessages = ShowMessages;
	ShowMessages = FALSE;
	if((r = ShowItem(1,p_gram,FALSE,pp_a,FALSE,mode,TRUE)) == ABORT || r == FINISH || r == EXIT) goto END;
	r = AllFollowingItems(p_gram,pp_a,p_weight,p_flag,&length,igram,irul,irep,TRUE,
		template,endgram,p_stack,&depth,&maxdepth,mode,time_end_compute);
	ShowMessages = showmessages;
	BPPrintMessage(0,odInfo,"👉 %d items have been produced\n",ItemNumber);
END:
	MyDisposeHandle((Handle*)&p_stack);
	MyDisposeHandle((Handle*)&p_weight);
	MyDisposeHandle((Handle*)&p_flag);
	HideMessages = FALSE;
	return(r);
	}


int AllFollowingItems(t_gram *p_gram,tokenbyte ***pp_a,long ****p_weight,long ****p_flag,
	long *p_length,int igram,int irul,int irep,int all,int template,int endgram,tokenbyte ****p_stack,
	int *p_depth,long *p_maxdepth,int mode,unsigned long time_end_compute) {

		// RECENT

	int icandidate,r,w,repeat,changed,grtype,nrep,old_gram,old_rul,new_gram,new_rul;
	static int try_number = 1;
	long leftpos,lastpos,incmark,new_pos;
	long ipos = ZERO;
	icandidate = nrep = old_gram = old_rul = 0;

/*	if(!Improvize && !Interactive && time_end_compute > 0L && getClockTime() > time_end_compute) {
		EmergencyExit = TRUE;
		BPPrintMessage(1,odError,"=> (6) Maximum allowed time (%d seconds) has been spent in AllFollowingItems(). Stopped computing...\n➡ This limit can be modified in the settings\n\n",MaxConsoleTime);
		return ABORT;
		} */
//	if(igram > endgram) return MISSED;
	if(FALSE && igram > (*p_gram).number_gram) {
		BPPrintMessage(1,odError,"=> Err. AllFollowingItems(). igram (%d) > number_gram (%d) [1]\n",igram,(*p_gram).number_gram);
		return ABORT;
		}
	if(FALSE && irul > (*((*p_gram).p_subgram))[igram].number_rule) {
		BPPrintMessage(1,odError,"=> Err. AllFollowingItems(). irul (%d) > number_rule (%d) [1]\n",irul,(*((*p_gram).p_subgram))[igram].number_rule);
		return ABORT;
		}
	if((grtype=(*((*p_gram).p_subgram))[igram].type) == SUBtype || grtype == SUB1type || grtype == POSLONGtype) {
		BPPrintMessage(1,odError,"=> Can't produce all items in 'SUB' or 'SUB1' or 'POSLONG' subgrammar gram#%d\n",igram);
		return ABORT;
		}
	irep = 1;
NEXTPOS:
	(*p_length) = LengthOf(pp_a);
	old_gram = igram; old_rul = irul;
	if(trace_produce_all) BPPrintMessage(1,odInfo,"\n%d) NextDerivation igram = %d irul = %d icandidate = %d ipos = %d, irep = %d\n",try_number,igram,irul,icandidate,ipos,irep);
	if((r = NextDerivation(pp_a,p_length,&igram,&irul,&ipos,&icandidate,mode,time_end_compute)) == OK) {
		if(trace_produce_all) BPPrintMessage(1,odInfo,"PUSH %d\n",(*p_depth));
		if(igram > endgram) goto END;
		if(PushStack(pp_a,&p_weight,&p_flag,p_length,&p_stack,p_depth,p_maxdepth) != OK) return ABORT;
		grtype = ORDtype;
		repeat = TRUE;	// This forces imode to 1 in Insert()
		leftpos = ZERO; // Not used
		if(trace_produce_all) BPPrintMessage(1,odInfo,"Derive igram = %d irul = %d ipos = %d\n",igram,irul,ipos);
		r = Derive(pp_a,p_gram,pp_a,p_length,igram,irul,ipos,&leftpos,grtype,repeat,&changed,&lastpos,&incmark,mode,time_end_compute);
		if(r < ZERO) {
			BPPrintMessage(1,odError,"=> Could not derive in igram = %d irul = %d\n",igram,irul);
			return r; // Happens if buffer problem
			}
		if((r = ShowItem(igram,p_gram,FALSE,pp_a,FALSE,mode,TRUE)) != OK) return r;
		// Check '_repeat', '_goto'
		nrep = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].repeat;
		if(nrep == 0) irep = 0;
		new_gram = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].gotogram;
		new_rul = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].gotorule;
		if(nrep > 0) {
			if(irep <= nrep) {
				if(trace_produce_all) BPPrintMessage(1,odInfo,"repeat irep = %d nrep = %d\n",irep,nrep);
				irep++;
				ipos += 2L;
				goto NEXTPOS;
				}
			BPPrintMessage(1,odInfo,"=> Gram#%d[%d] _repeat(%d): this rule is repeating and could create an infinite size. It has been used only %d times.\n",igram,irul,nrep,nrep);
			(*((*((*p_gram).p_subgram))[igram].p_rule))[irul].w = 0;
			// Rule will no longer be candidate. Even PullStack() will not restore its weight.
			}
		if(new_gram > 0) {
			if(trace_produce_all) BPPrintMessage(1,odInfo,"_goto: new_gram = %d new_rul = %d\n",new_gram,new_rul);
			r = AllFollowingItems(p_gram,pp_a,p_weight,p_flag,p_length,new_gram,new_rul,irep,all,template,endgram,p_stack,p_depth,p_maxdepth,mode,time_end_compute);
			if(r == ABORT || r == EXIT) return r;
			if(PullStack(pp_a,p_weight,p_flag,p_length,p_stack,p_depth,p_maxdepth) != OK) {
				BPPrintMessage(0,odError,"=> PullStack() != OK in AllFollowingItems()\n");
				return(ABORT);
				}
			if(trace_produce_all) BPPrintMessage(1,odInfo,"PULL %d\n",(*p_depth));
			ipos += 2L;
			goto NEXTPOS;
			}
		if(ChangeFlagsInRule(p_gram,igram,irul) != OK) {
			BPPrintMessage(1,odError,"=> Could not change flags in igram = %d irul = %d\n",igram,irul);
			return ABORT;
			}
		if(Varweight) {
			w = (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].w;
			w += (*((*((*p_gram).p_subgram))[igram].p_rule))[irul].incweight;
			if(w < 0) w = 0;
			(*((*((*p_gram).p_subgram))[igram].p_rule))[irul].w = w;
			}
		try_number++;
		r = AllFollowingItems(p_gram,pp_a,p_weight,p_flag,p_length,igram,irul,irep,all,template,endgram,p_stack,p_depth,p_maxdepth,mode,time_end_compute);
		if(PullStack(pp_a,p_weight,p_flag,p_length,p_stack,p_depth,p_maxdepth) != OK) {
			BPPrintMessage(0,odError,"=> PullStack() != OK in AllFollowingItems()\n");
			return(ABORT);
			}
		if(trace_produce_all) BPPrintMessage(1,odInfo,"PULL %d\n",(*p_depth));
		if(r == ABORT || r == EXIT) return r;
		ipos += 2L;
		goto NEXTPOS;
		}
END:
	CheckItemProduced(p_gram,old_gram,pp_a,p_length,template,mode);
	// Look for '_failed'				
	if(old_gram > 0 && (igram = (*((*((*p_gram).p_subgram))[old_gram].p_rule))[old_rul].failedgram) > 0) {
		irul = (*((*((*p_gram).p_subgram))[old_gram].p_rule))[old_rul].failedrule;
		if(trace_produce_all) BPPrintMessage(1,odInfo,"_failed, goto: igram = %d irul = %d\n",igram,irul);
		ipos += 2L;
		goto NEXTPOS;
		}
	return(r);
	}


int PushStack(tokenbyte ***pp_a,long *****pp_weight,long *****pp_flag,long *p_length,
	tokenbyte *****pp_stack,int *p_depth,long *p_maxdepth) {
	long i,igram,irul,**ptr4;
	Handle ptr;
	tokenbyte **ptr2;
	long **ptr3;

	if((++(*p_depth)) >= (*p_maxdepth)) {
		if(ThreeOverTwo(p_maxdepth) != OK) return(ABORT);
		BPPrintMessage(0,odInfo,"Increasing stack depth to %ld levels\n",(*p_maxdepth));
		ptr = (Handle) (*pp_stack);
		if((ptr = IncreaseSpace(ptr)) == NULL) return(ABORT);
		(*pp_stack) = (tokenbyte****) ptr;
		ptr = (Handle) (*pp_weight);
		if((ptr = IncreaseSpace(ptr)) == NULL) return(ABORT);
		(*pp_weight) = (long****) ptr;
		if((*pp_flag) != NULL) {
			ptr = (Handle) (*pp_flag);
			if((ptr = IncreaseSpace(ptr)) == NULL) return(ABORT);
			(*pp_flag) = (long****) ptr;
			}
		}
	if((ptr2 = (tokenbyte**) GiveSpace((Size)(*p_length+2L) * sizeof(tokenbyte))) == NULL)
		return(ABORT);
	(**pp_stack)[(*p_depth)] = ptr2;
	for(i=ZERO; i < (*p_length+2L); i++) (*((**pp_stack)[(*p_depth)]))[i] = (**pp_a)[i];
	if((*pp_flag) != NULL) {
		if((ptr4 = (long**) GiveSpace((Size)(Jflag+1) * sizeof(long))) == NULL) return(ABORT);
		(**pp_flag)[(*p_depth)] = ptr4;
		for(i = 1L; i <= Jflag; i++) (*((**pp_flag)[(*p_depth)]))[i] = (*p_Flag)[i];
		}
	if((ptr3 = (long**) GiveSpace((Size)(MaxRul+1) * (Gram.number_gram + 1) * sizeof(long)))
		== NULL) return(ABORT);
	(**pp_weight)[(*p_depth)] = ptr3;
	for(igram = 1,i = 0; igram <= Gram.number_gram; igram++) {
		for(irul=1; irul <= (*(Gram.p_subgram))[igram].number_rule; irul++) {
			(*((**pp_weight)[(*p_depth)]))[i] = (*((*(Gram.p_subgram))[igram].p_rule))[irul].w;
			i++;
			}
		}
	return(OK);
	}

int PullStack(tokenbyte ***pp_a,long ****p_weight,long ****p_flag,long *p_length,
	tokenbyte ****p_stack,int *p_depth,long *p_maxdepth) {
	int p,q;
	long i,igram,irul;
	Handle ptr;

	if((*p_depth) < 0  || (*p_depth) >= *p_maxdepth) {
		BPPrintMessage(0,odError,"=> Err. PullStack(). *p_depth < 0  || (*p_depth) >= *p_maxdepth");
		return(ABORT);
		}
	(*p_length) = ZERO;
	for(i=0; ; i+=2) {
		p = (*((*p_stack)[(*p_depth)]))[i];
		q = (*((*p_stack)[(*p_depth)]))[i+1];
		if(p == TEND && q == TEND) break;
		(**pp_a)[i] = p; (**pp_a)[i+1] = q;
		(*p_length) += 2;
		}
	(**pp_a)[i] = (**pp_a)[i+1] = TEND;
	ptr = (Handle) (*p_stack)[(*p_depth)];
	MyDisposeHandle(&ptr);
	(*p_stack)[(*p_depth)] = NULL;
	if(p_flag != NULL && (*p_flag)[(*p_depth)] != NULL) {
		for(i=1; i <= Jflag; i++) (*p_Flag)[i] = (*((*p_flag)[(*p_depth)]))[i];
		ptr = (Handle) (*p_flag)[(*p_depth)];
		MyDisposeHandle(&ptr);
		(*p_flag)[(*p_depth)] = NULL;
		}
	for(igram=1,i=0; igram <= Gram.number_gram; igram++) {
		for(irul=1; irul <= (*(Gram.p_subgram))[igram].number_rule; irul++) {
			if(!(AllItems && (*((*(Gram.p_subgram))[igram].p_rule))[irul].repeat > 0 && (*((*(Gram.p_subgram))[igram].p_rule))[irul].w == 0))
				// This happens in a repeating rule when max number of repetitions has been reached
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].w = (*((*p_weight)[(*p_depth)]))[i];
			i++;
			}
		}
	ptr = (Handle) (*p_weight)[(*p_depth)];
	MyDisposeHandle(&ptr);
	(*p_weight)[(*p_depth)] = NULL;
	(*p_depth)--;
	return(OK);
	}


int LastGrammarWanted(int igram) {
	char defaultvalue[MAXFIELDCONTENT],value[MAXFIELDCONTENT];
	int r,endgram,wantgram;

	endgram = Gram.number_gram;
	if(endgram == 1 || igram == 1) return(igram);
	my_sprintf(defaultvalue,"%ld",(long)igram);
	wantgram = igram;

	TRY:
	if(!ScriptExecOn && !AEventOn) {
		if((r=AnswerWith("Last grammar in which\nall rules should be tried?",
				defaultvalue,value)) != OK) return(r);
		wantgram = (int) atol(value); 	/* Don't use atoi() because int's are 4 bytes */
		if(wantgram < 1 || wantgram > endgram) {
			my_sprintf(Message,"Range should be [1,%ld]. Try again...",(long)endgram);
			BPPrintMessage(0,odError,"%s",Message);
			goto TRY;
			}
		}
	return(wantgram);
	}


int NextDerivation(tokenbyte ***pp_a,long *p_length,int *p_igram,int *p_irul,
	long *p_ipos,int *p_icandidate,int mode,unsigned long time_end_compute) {
	int r,nb_candidates,**p_prefrule,**p_candidate,maxpref,freedom,equalweight,maxrul,
		repeat;
	tokenbyte instan[MAXLIN],meta[MAXMETA2],meta1[MAXMETA2];
	long **p_pos,leftpos,istart,jstart,pos,length,lenc1,**p_totwght;

	maxrul = (*(Gram.p_subgram))[*p_igram].number_rule + 1;
	if((p_totwght = (long**) GiveSpace((Size)maxrul * sizeof(long))) == NULL)
		return(ABORT);
	if((p_pos = (long**) GiveSpace((Size)maxrul * sizeof(long))) == NULL)
		return(ABORT);
	if((p_candidate = (int**) GiveSpace((Size)maxrul * sizeof(int))) == NULL)
		return(ABORT);
	if((p_prefrule = (int**) GiveSpace((Size)maxrul * sizeof(int))) == NULL)
		return(ABORT);
	leftpos = maxpref = 0;
	repeat = FALSE;
	nb_candidates = FindCandidateRules(pp_a,&Gram,1,*p_igram,(*(Gram.p_subgram))[*p_igram].type,
		p_candidate,p_totwght,p_pos,p_prefrule,leftpos,&maxpref,&freedom,repeat,mode,&equalweight,
		FALSE,time_end_compute);
	if(trace_produce_all) BPPrintMessage(1,odInfo,"igram = %d, nb_candidates = %d, icandidate = %d\n",(*p_igram),nb_candidates,(*p_icandidate));
	if(nb_candidates <= (*p_icandidate)) {
		if(nb_candidates == ABORT || nb_candidates == EXIT) {
			r = nb_candidates; goto QUIT;
			}
NEXTGRAM:
		if(++(*p_igram) > Gram.number_gram) {
			r = MISSED; goto QUIT;
			}
		(*p_icandidate) = 0; (*p_ipos) = 0;
		MyDisposeHandle((Handle*)&p_candidate);
		MyDisposeHandle((Handle*)&p_totwght);
		MyDisposeHandle((Handle*)&p_pos);
		MyDisposeHandle((Handle*)&p_prefrule);
		return(NextDerivation(pp_a,p_length,p_igram,p_irul,p_ipos,p_icandidate,mode,time_end_compute));
		}
FOUND:
	if((*p_irul) != (*p_candidate)[*p_icandidate]) (*p_ipos) = 0;
	if((*p_ipos) < (*p_pos)[*p_icandidate]) (*p_ipos) = (*p_pos)[*p_icandidate];
	(*p_irul) = (*p_candidate)[*p_icandidate];
	if((*(Gram.p_subgram))[*p_igram].type == LINtype) {
		if((*p_ipos) == (*p_pos)[*p_icandidate]) {
			r = OK;
			goto QUIT;
			}
		else goto NEXTCANDIDATE;
		}
	for(pos = (*p_ipos); pos < (*p_length); pos += 2L) {
		r = Found(pp_a,ORDtype,(*((*(Gram.p_subgram))[*p_igram].p_rule))[*p_irul].p_leftarg,0,
				(*((*(Gram.p_subgram))[*p_igram].p_rule))[*p_irul].leftnegcontext,&lenc1,pos,TRUE,
				instan,meta,meta1,&istart,&jstart,&length,
				(*((*(Gram.p_subgram))[*p_igram].p_rule))[*p_irul].ismeta,time_end_compute);
		if(r == ABORT) goto QUIT; // Fixed by BB 7 Nov 2020
		if(r && OkContext(pp_a,ORDtype,(*((*(Gram.p_subgram))[*p_igram].p_rule))[*p_irul],pos,
				length,meta,instan,mode,time_end_compute)) {
			(*p_ipos) = pos;
			r = OK;
			goto QUIT;
			}
		}
NEXTCANDIDATE:
	if(++(*p_icandidate) >= nb_candidates) goto NEXTGRAM;
	(*p_ipos) = 0;
	goto FOUND;
QUIT:
	MyDisposeHandle((Handle*)&p_candidate);
	MyDisposeHandle((Handle*)&p_totwght);
	MyDisposeHandle((Handle*)&p_pos);
	MyDisposeHandle((Handle*)&p_prefrule);
	return(r);
	}


int LastStructuralSubgrammar(void) {
	int igram,irul,found,ilast;
	ilast = 0;
	for(igram=1; igram <= Gram.number_gram; igram++) {
		found = FALSE;
		for(irul=1; irul <= (*(Gram.p_subgram))[igram].number_rule; irul++) {
			if(StructuralRule(igram,irul)) {
	/*			my_sprintf(Message,"igram=%ld  irul=%ld  is structural\n",(long)igram,
					(long)irul);
				Println(wTrace,Message); */
				found = TRUE; break;
				}
			}
		if(found) ilast = igram;
		}
	if(ilast == 0) return(ilast);
	return(LastGrammarWanted(ilast));
	}


int StructuralRule(int igram, int irul) {
	t_rule rule;
	tokenbyte **p_arg;
	int i,i0,i1,p,q,lenc,funct;

	/* Rule is structural if some structure markers are found in its... */
	/* ... argument, yet not in its left or right context. */
	rule = (*((*(Gram.p_subgram))[igram].p_rule))[irul];
	funct = rule.operator; if(funct == 2) return(FALSE);	/* "<--" */
	p_arg = rule.p_rightarg;
	if(p_arg == NULL) return(FALSE);
	lenc = 4 * (*((*(Gram.p_subgram))[igram].p_rule))[irul].leftnegcontext;
	i0 = (*((*(Gram.p_subgram))[igram].p_rule))[irul].leftoffset + lenc;
	i1 = (int) LengthOf(&p_arg) - 1 /* -(*((*(Gram.p_subgram))[igram].p_rule))[irul].rightoffset */;
	for(i=i0; ((*p_arg)[i] != TEND || (*p_arg)[i+1] != TEND) && i < i1; i+=2) {
		p = (int) (*p_arg)[i]; q = (int) (*p_arg)[i+1];
		switch(p) {
			case T2:		/* Parenthesis */
			case T5:		/* Homomorphism */
	/*			my_sprintf(Message,"Found %ld.%ld position %ld i0=%ld  i1=%ld",
					(long)p,(long)q,(long)i,(long)i0,(long)i1);
				Println(wTrace,Message);
				my_sprintf(Message,"lenc=%ld  LengthOf(&p_arg)=%ld  (*((*(Gram.p_subgram))[igram].p_rule))[irul].leftoffset=%ld  (*((*(Gram.p_subgram))[igram].p_rule))[irul].rightoffset=%ld",
					(long)lenc,(long)LengthOf(&p_arg),(long)(*((*(Gram.p_subgram))[igram].p_rule))[irul].leftoffset,
					(long)(*((*(Gram.p_subgram))[igram].p_rule))[irul].rightoffset);
				Println(wTrace,Message);
				for(i=0; ((*p_arg)[i] != TEND || (*p_arg)[i+1] != TEND); i+=2) {
					my_sprintf(Message,"%ld.%ld ",(long)(*p_arg)[i],(long)(*p_arg)[i+1]);
					Print(wTrace,Message);
					}
				Print(wTrace,"\n");  */
				return(TRUE);
			case T0:
				switch(q) {
					case 3:		/* '+' */
					case 4:		/* ':' */
					case 5:		/* ';' */
					case 6:		/* '=' */
					case 11:	/* '/' */
					case 25:	/* '\' */
						return(TRUE);
					}
			}
		}
	return(FALSE);
	}


int NoVariable(tokenbyte ***pp_a) {
	int i,p,q;
	for(i=0;((p=(**pp_a)[i]) != TEND || (**pp_a)[i+1] != TEND); i+=2) {
		q = (**pp_a)[i+1];
		if(p == T0 && q == 10) return(FALSE);	/* 'S' */
		if(p == T4 && ((Gram.p_subgram != NULL && p_VarStatus != NULL
			&& q <= Jvar && (((*p_VarStatus)[q] & 1) || ((*p_VarStatus)[q] & 4)))
				|| ((*p_VarStatus)[q] == 0 && (!CompiledGr || !CompiledGl))))
					return(FALSE);
		}
	return(TRUE);
	}


int MakeTemplate(tokenbyte ***pp_a) {
	unsigned long i;
	tokenbyte m,p;

	ClearMarkers(pp_a);
	for(i=ZERO; ((m=(**pp_a)[i]) != TEND || (**pp_a)[i+1] != TEND); i+=2L) {
		p = (**pp_a)[i+1];
		if(m == T3 || m == T7 || m == T25) {	/* Terminal or out-time object or simple note */
			if(p >= 0) {
				(**pp_a)[i] = T3; (**pp_a)[i+1] = 0;	/* '_' */
				}
			}
		}
	ClearMarkers(pp_a);
	return(OK);
	}


int ClearMarkers(tokenbyte ***pp_a) {
/* Suppress '¥' beat markers */
	unsigned long i,k;
	tokenbyte m,p;
	int setting_sections;

	if(pp_a == NULL || (*pp_a) == NULL || (**pp_a) == NULL) {
		BPPrintMessage(0,odError,"=> Err. ClearMarkers(). pp_a == NULL || (*pp_a) == NULL || (**pp_a) == NULL");
		return(OK);
		}
	setting_sections = TRUE;
	for(i=k=ZERO; ;) {
		m = (**pp_a)[i+k]; p = (**pp_a)[i+k+1L];
		if(setting_sections && m == T0 && p == 21)	{ /* '*' */
			k += 6; continue;
			}
	//	if(m == T0 && p == 7) {		/* '¥' */
	//		k += 2; continue;
	//		}
		if(k > 0) {
			(**pp_a)[i] = m;
			(**pp_a)[i+1] = p;
			}
		if(m != T1 && (m != T0 || p != 3)) setting_sections = FALSE;
		i += 2L;
		if(m == TEND && p == TEND) break;
		}
	return(OK);
	}


int WriteTemplate(int w,tokenbyte ***pp_a)
{
unsigned long i;
tokenbyte m,p;
int foundspeed,foundscaling,setting_section;
long speed;
char line[MAXLIN];

if(pp_a == NULL || (*pp_a) == NULL || (**pp_a) == NULL) {
	BPPrintMessage(0,odError,"=> Err. WriteTemplate(). pp_a == NULL || (*pp_a) == NULL || (**pp_a) == NULL");
	return(OK);
	}
foundspeed = foundscaling = FALSE; speed = 1L; setting_section = TRUE;
my_sprintf(Message,"[%ld] ",(long)ItemNumber);
Print(w,Message);
for(i=ZERO; ; i+=2L) {
	m = (**pp_a)[i]; p = (**pp_a)[i+1];
	if(m == TEND && p == TEND) break;
	if(m != T1 && (m != T0 || p != 3)) setting_section = FALSE;
	if(m == T1 || (m == T0 && (p == 3 || p == 11))) foundspeed = TRUE;
	if(m == T0 && (p == 21 || p == 24)) {
		foundscaling = TRUE;	/* '*' or '**' */
		goto TAKEIT;
		}
	if(!foundscaling && !setting_section) {
		foundscaling = TRUE;
		Print(w,"*1");
		}
	if(m == T0 && (p == 11 || p == 25)) {
		foundspeed = TRUE;	/* '/' or '\' */
		}
	if(!foundspeed && !setting_section) {
		foundspeed = TRUE;
		Print(w,"/1 ");
		}
TAKEIT:
	switch(m) {
		case T0:
			if(p < MAXCODE2) {
				line[0] = Code[p];
				line[1] = '\0';
				Print(w,line);
				}
			break;
		case T1:	/* Number */
			my_sprintf(line,"%.0f",(double)(((double)TOKBASE * p) + (**pp_a)[i+3]));
			Print(w,line);
			if(!setting_section && ((**pp_a)[i+4] != T0 || (**pp_a)[i+5] != 11))
				Print(w," ");
			i += 2;
			break;
		case T2:	/* Parenthesis marker */
			my_sprintf(line,"@%ld ",(long)p);
			Print(w,line); break;
		case T3:	/* Sound-object */
		case T25:	/* Simple note */
		case T7:	/* Out-time object */
			Print(w,"_"); break;
		case T5:	/* Homomorphism */
			if(p < Jhomo) {
				my_sprintf(line," %s ",*((*p_Homo)[p]));
				Print(w,line);
				}
			break;
		case T9:	/* Time pattern */
			my_sprintf(line," %s ",*((*p_Patt)[p]));
			Print(w,line); break;
		case T43:	/* _tempo */
			my_sprintf(line," %s(%ld/",*((*p_PerformanceControl)[61]),(long)p);
			Print(w,line);
			i += 2L;
			m = (**pp_a)[i];
			p = (**pp_a)[i+1];
			my_sprintf(line,"%ld)",(long)p);
			Print(w,line);
			break;
		}
	}
Print(w,"\n");
Gram.hasTEMP = TRUE;
return(OK);
}


int ReadTemplate(int w,long pos,long *p_posend,tokenbyte ***pp_a,int *p_i)
{
int i,j,jj,l,n,lmax;
long im,u,v;
unsigned long k;
char c,d,*p,**qq,**h;
tokenbyte **ptr;
KeyNumberMap map;

(*p_posend) = pos;
if(*pp_a == NULL) {
	BPPrintMessage(0,odError,"=> Err. ReadTemplate(). *pp_a = NULL");
	return(ABORT);
	}
h = WindowTextHandle(TEH[w]);
im = (long) MyGetHandleSize((Handle)*pp_a);
im = (long) (im / sizeof(tokenbyte)) - 6L;
for(i=0; (c=GetTextChar(w,pos)) != '\r' && c != '\0'; pos++) {
	if(MySpace(c)) continue;
	if(c == '[') {
		(*p_i) = 0;
		while((c=GetTextChar(w,++pos)) != ']') {
			c -= '0';
			if(c < 0 || c > 9) {
				BPPrintMessage(0,odError,"=> Err. ReadTemplate(). Wrong number");
				c = 0;
				}
			*p_i = *p_i * 10 + c;
			}
		continue;
		}
	if(i > im) {
		if(ThreeOverTwo(&im) != OK) {
			BPPrintMessage(0,odError,"=> Err. ReadTemplate(). Can't resize");
			return(ABORT);
			}
		ptr = *pp_a;
		if((ptr = (tokenbyte**) IncreaseSpace((Handle)ptr)) == NULL) return(ABORT);
		*pp_a = ptr;
		}
	if(c == '@') {	/* Bracket marker */
		j = 0;
		while(!MySpace(c=GetTextChar(w,++pos))) {
			c = c - '0';
			if(c < 0 || c > 9) {
				BPPrintMessage(0,odError,"=> Err. ReadTemplate(). Not digit"); return(MISSED);
				}
			j = 10 * j + c;
			}
		(**pp_a)[i++] = T2;
		(**pp_a)[i++] = (tokenbyte) j; continue;
		}
		
	if(c == '_') {
		if(isalpha(GetTextChar(w,pos+1))) {
		//	BPPrintMessage(0,odInfo,"c == '_'\n");
			/* Look for tool or performance control */
			p = &((*h)[pos]);
			j = GetPerformanceControl(&p,0,&n,NO,&u,&v,&map);
			if(j == 61) {	/* _tempo */
				(**pp_a)[i++] = T43; (**pp_a)[i++] = (tokenbyte) u;
				(**pp_a)[i++] = T43; (**pp_a)[i++] = (tokenbyte) v;
				while((c=GetTextChar(w,pos)) != ')') pos++;
				continue;
				}
			}
		(**pp_a)[i++] = T3;	/* '_' */
		(**pp_a)[i++] = (tokenbyte) 0; continue;
		}
		
	if(c == '-') {	/* Separating line below templates */
		return(MISSED);
		}
	
	if(c == '*') {
		if(isdigit(d=GetTextChar(w,pos+1))) {
			(**pp_a)[i++] = T0;		/* scaling up '*' */
			(**pp_a)[i++] = (tokenbyte) 21;
			continue;
			}
		if(d == '*' && isdigit(GetTextChar(w,pos+2))) {
			(**pp_a)[i++] = T0;		/* scaling down '**' */
			(**pp_a)[i++] = (tokenbyte) 24;
			pos++;
			continue;
			}
		}
		
	/* Look for homomorphism */
	MyLock(NO,(Handle) h);
	for(j=lmax=0; j < Jhomo; j++) {
		p = &((*h)[pos]);
		qq = (*p_Homo)[j];
		l = MyHandleLen(qq);
		if(Match(TRUE,&p,qq,l) && l > lmax) {
			lmax = l; jj = j;
			}
		}
	MyUnlock((Handle) h);
	if(lmax  > 0) {
		(**pp_a)[i++] = T5; (**pp_a)[i++] = (tokenbyte) jj;
		pos += (lmax - 1);
		continue;
		}
		
	/* Look for code */
	if((j=FindCode(c)) != -1) {
		(**pp_a)[i++] = T0;
		(**pp_a)[i++] = (tokenbyte) j;
		continue;
		}
	
	/* Look for number */
	c -= '0'; k = ZERO;
	if(c >= 0 && c <= 9) {
		do {
			k = (10L * k) + c;
			c = GetTextChar(w,++pos) - '0';
			}
		while(c >= 0 && c <= 9);
		(**pp_a)[i++] = T1;
		(**pp_a)[i++] = (tokenbyte) ((k - (k % TOKBASE)) / TOKBASE);
		(**pp_a)[i++] = T1;
		(**pp_a)[i++] = (tokenbyte) (k % TOKBASE);
		pos--;
		continue;
		}
				
	/* Look for time pattern */
	MyLock(NO,(Handle) h);
	for(j=lmax=0; j < Jpatt; j++) {
		p = &((*h)[pos]);
		qq = (*p_Patt)[j];
		l = MyHandleLen(qq);
		if(Match(TRUE,&p,qq,l) && l > lmax) {
			lmax = l; jj = j;
			}
		}
	MyUnlock((Handle) h);
	if(lmax  > 0) {
		(**pp_a)[i++] = T9; (**pp_a)[i++] = (tokenbyte) jj;
		pos += (lmax - 1);
		continue;
		}
	}
(**pp_a)[i++] = (tokenbyte) TEND; (**pp_a)[i++] = (tokenbyte) TEND;
(*p_posend) = pos + 1;
return(OK);
}


int DeleteTemplates(void)
{
int separator,gap;
long pos,pos1,posmax;
char **p_line;

#if BP_CARBON_GUI_FORGET_THIS
UpdateThisWindow(FALSE,Window[wGrammar]); /* Update text length */
ShowSelect(CENTRE,wGrammar);
#endif /* BP_CARBON_GUI_FORGET_THIS */
pos = pos1 = ZERO;
posmax = GetTextLength(wGrammar);
while(posmax > 0 && GetTextChar(wGrammar,posmax-1) == '\r') posmax--;
separator = TRUE;
p_line = NULL;
while(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) == OK) {
	if((*p_line)[0] == '\0') {
		pos1 = pos; continue;
		}
	if(Mystrcmp(p_line,"DATA:") == 0) goto SORTIR;
	if(Mystrcmp(p_line,"COMMENT:") == 0) goto SORTIR;
	if(Mystrcmp(p_line,"TEMPLATES:") == 0) {
		do {
			if(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) != OK) goto SORTIR;
			if((*p_line)[0] == '\0') continue;
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		pos1--;
		SetSelect(pos1,pos,TEH[wGrammar]);
		TextDelete(wGrammar);
		goto SORTIR;
		}
	if((*p_line)[0] == '-' && (*p_line)[1] == '-') separator = TRUE;
	else separator = FALSE;
	pos1 = pos;
	}
pos1 = posmax;

SORTIR:
MyDisposeHandle((Handle*)&p_line);
SetSelect(pos1,pos1,TEH[wGrammar]);
if(!separator) {
	strcpy(Message,"\n------------");
	Print(wGrammar,Message);
	pos1 += strlen(Message);
	}
Print(wGrammar,"\nTEMPLATES:\n------------\n");
strcpy(Message,"\nTEMPLATES:\n");
pos1 += strlen(Message);
SetSelect(pos1,pos1,TEH[wGrammar]);
ShowSelect(CENTRE,wGrammar);

END:
return(OK);
}


int AnalyzeBuffer(tokenbyte ***pp_a,int learn,int templates,int all,long pos,int *p_result,
	char *remark) {
/* Analyze item in a[] */
/* 'pos' is position of first template considered in grammar window */
	int repeat,r;
	long lengthA;

	ComputeOn++;
	FirstTime = InitThere = FALSE;
	lengthA = LengthOf(pp_a);
	repeat = FALSE; r = OK;
	if(lengthA == ZERO) goto QUIT;
	do {
		r = Analyze(pp_a,&lengthA,&repeat,learn,templates,all,pos,p_result,remark);
		}
	while(repeat == TRUE && r == OK);

	QUIT:
	ComputeOn--;
	return(r);
	}


int Analyze(tokenbyte ***pp_a,long *p_lengthA,int *p_repeat,int learn,int templates,int all,
	long pos,int *p_result,char *remark) {
/* pos = position of first template considered in grammar window */
	int i,itemp,r,igram,finish,again,foundone,good,hasperiods;
	long posend,lastbyte;
	tokenbyte m,p,**p_b,***pp_b,**p_c,***pp_c,**p_d,***pp_d;
	double maxseqapprox;
	unsigned long time_end_compute;

	ProduceStackDepth = itemp = 0; posend = pos; foundone = good = again = FALSE;
	p_b = p_c = p_d = NULL;
	pp_b = &p_b; pp_c = &p_c; pp_d = &p_d;

	r = OK; (*p_result) = ABORT;

	lastbyte = GetTextLength(wTrace);
	SetSelect(lastbyte,lastbyte,TEH[wTrace]);
	if(remark[0] == '\0') my_sprintf(LineBuff,"");
	else my_sprintf(LineBuff,"[%s] ",remark);
	if((all && templates) || DisplayProduce) {
		if(remark[0] == '\0') {
			Print(wTrace,">>> Analyzing item: ");
			if((r=PrintArg(FALSE,FALSE,TRUE,FALSE,FALSE,FALSE,stdout,wTrace,pp_Scrap,pp_a)) != OK)
				return(r);
			}
		else {
			my_sprintf(Message,">>> Analyzing item [%s]\n",remark);
			Print(wTrace,Message);
			}
		if(StepProduce) {
			r = InterruptCompute(-1,&Gram,*p_repeat,-1,ANAL);
			if(r != OK) return(r);
			}
		}
	ShowSelect(CENTRE,wTrace);
	BufferSize = MAXDISPL;
	if(*pp_a == NULL) {
		BPPrintMessage(0,odError,"=> Err. Analyze(). *pp_a = NULL");
		return(ABORT);
		}

	if(templates) {
		if((*pp_d = (tokenbyte**) GiveSpace((Size)MyGetHandleSize((Handle)*pp_a))) == NULL) {
			return(ABORT);
			}
		if(CopyBuf(pp_a,pp_d) == ABORT) {
			r = ABORT; goto END;
			}
		
		if(FoundPeriod(pp_a) || HasStructure(*pp_a)) {
			if((r=PolyMake(pp_a,&maxseqapprox,NO)) == ABORT || r == EXIT) {
				goto END;
				}
			if(r != OK) {
				r = MISSED; goto END;
				}
			}
		}
		
	if((*pp_c = (tokenbyte**) GiveSpace((Size)MyGetHandleSize((Handle)*pp_a))) == NULL) {
		return(ABORT);
		}
	if(CopyBuf(pp_a,pp_c) == ABORT) {
		r = ABORT; goto END;
		}
		
	NEXTTEMPLATE:
	PleaseWait();
	if(CopyBuf(pp_c,pp_a) == ABORT) {
		r = ABORT; goto END;
		}
	if(templates) {
		pos = posend;
		if((*pp_b = (tokenbyte**)
			GiveSpace((Size) MAXDISPL * sizeof(tokenbyte))) == NULL) {
			r = ABORT; goto END;
			}
		if(ReadTemplate(wGrammar,pos,&posend,pp_b,&itemp) != OK) {
			(*p_result) = MISSED;
			if(!foundone) {
				my_sprintf(Message,"Item matched no template...");
				ShowMessage(TRUE,wMessage,Message);
				if(StepProduce) {
					r = InterruptCompute(-1,&Gram,*p_repeat,-1,ANAL);
					if(r != OK) {
						MyDisposeHandle((Handle*)pp_b);
						goto END;
						}
					}
				my_sprintf(Message,"Item %smatched no template...\n\n",LineBuff);
				Print(wTrace,Message);
				}
			else if(good) *p_result = OK;
			MyDisposeHandle((Handle*)pp_b);
		/*	if(!all) r = MISSED;
			else */ r = OK;
			goto END;
			}
		hasperiods = FoundPeriod(pp_b);
		if((r=MatchTemplate(pp_a,pp_b)) != OK) {
			MyDisposeHandle((Handle*)pp_b);
			if(r == ABORT || r == EXIT) goto END;
			goto NEXTTEMPLATE;
			}
		if(hasperiods) {
			if(CopyBuf(pp_d,pp_a) == ABORT) {
				MyDisposeHandle((Handle*)pp_b);
				r = ABORT; goto END;
				}
			}
		else if(CopyBuf(pp_b,pp_a) == ABORT) {
			MyDisposeHandle((Handle*)pp_b);
			r = ABORT; goto END;
			}
		MyDisposeHandle((Handle*)pp_b);
		foundone = TRUE;
	/*	my_sprintf(Message,"Item %smatched template [%ld]",LineBuff,(long)itemp);
		ShowMessage(TRUE,wMessage,Message); */
		if(ScriptExecOn || all || DisplayProduce) {
			lastbyte = GetTextLength(wTrace);
			SetSelect(lastbyte,lastbyte,TEH[wTrace]);
			if(DisplayProduce) {
				my_sprintf(Message,"Item matched template [%ld], yielding:\n",(long)itemp);
				Print(wTrace,Message);
				if((r=PrintArg(FALSE,FALSE,TRUE,FALSE,FALSE,FALSE,stdout,wTrace,pp_Scrap,pp_a)) != OK) goto END;
				ShowSelect(CENTRE,wTrace);
				}
			else {
				my_sprintf(Message,"Item %smatched template [%ld]\n",LineBuff,(long)itemp);
				Print(wTrace,Message);
				ShowSelect(CENTRE,wTrace);
				}
			}
		if(StepProduce) {
			r = InterruptCompute(-1,&Gram,*p_repeat,-1,ANAL);
			if(r != OK) goto END;
			}
		}
	ClearMarkers(pp_a);

	time_end_compute = 0L;

	for(igram=Gram.number_gram; igram >= 1; igram--) {
		PleaseWait();
		finish = FALSE;
		r = ComputeInGram(pp_a,&Gram,igram,0,p_lengthA,&finish,p_repeat,ANAL,learn,&i,&i,time_end_compute);
		if(r == MISSED) break;
		if(r == FINISH) continue;
		if(finish) {
			if((r = ShowItem(igram,&Gram,FALSE,pp_a,(*p_repeat),PROD,FALSE))
				== ENDREPEAT || r == ABORT || r == EXIT) {
				goto END;
				}
			StepProduce = DisplayProduce = TRUE;
			PlanProduce = TraceProduce = finish - 1;
			}
		if(r == ABORT || r == EXIT) goto END;
		if((r == BACKTRACK || r == AGAIN)  && p_MemGram != NULL) {
			igram = (*p_MemGram)[ProduceStackIndex];
			igram++; continue;
			}
	/*	if(StepGrammars && !StepProduce) {
			r = InterruptCompute(igram,&Gram,*p_repeat,-1,ANAL);
			if(r != OK) goto END;
			} */
		}
	for(i=0; ; i+=2) {
		m = (**pp_a)[i]; p = (**pp_a)[i+1];
		if(m == TEND && p == TEND) goto NEXTTEMPLATE;
		/* Skip speed markers */
		if(m == T0) {
			switch(p) {
				case 11:
				case 21:
				case 24:
				case 25:
					continue;
					break;
				}
			}
		if(m == T1) continue;
		break;
		}
	if((**pp_a)[i] == T0 && (**pp_a)[i+1] == 10) { /* 'S' */
		good = (*p_result) = YES;
		if(!templates) my_sprintf(Message,"Item %saccepted by grammar...",LineBuff);
		else my_sprintf(Message,
			"Item %smatching template [%ld] accepted by grammar...",LineBuff,(long)itemp);
		ShowMessage(TRUE,wMessage,Message);
		if(!templates) my_sprintf(Message,"Item %saccepted by grammar...\n\n",LineBuff);
		else my_sprintf(Message,"Item %smatching template [%ld] accepted by grammar...\n\n",
			LineBuff,(long)itemp);
		if(1 || all || DisplayProduce || ScriptExecOn) {
			Print(wTrace,Message);
			ShowSelect(CENTRE,wTrace);
			}
		if(StepProduce) {
			r = InterruptCompute(-1,&Gram,*p_repeat,-1,ANAL);
			if(r != OK) goto END;
			}
		if(all && templates) goto NEXTTEMPLATE;
		}
	else {
		my_sprintf(Message,"Item %srejected by grammar...",LineBuff);
		(*p_result) = MISSED;
		ShowMessage(TRUE,wMessage,Message);
		if(templates)
			my_sprintf(Message,"Item %smatching template [%ld] rejected by grammar...\n",LineBuff,
				(long)itemp);
		else
			my_sprintf(Message,"Item %srejected by grammar...\n",LineBuff);
		Print(wTrace,Message);
		my_sprintf(Message,"Result of failed analysis:\n");
		Print(wTrace,Message);
		if((r=PrintArg(FALSE,FALSE,TRUE,FALSE,FALSE,FALSE,stdout,wTrace,pp_Scrap,pp_a)) != OK)
			goto END;
		ShowSelect(CENTRE,wTrace);
		if(templates) goto NEXTTEMPLATE;
		if(learn) {
			r = ABORT; goto END;
			}
		if((r=InterruptCompute(-1,&Gram,FALSE,-1,ANAL)) != OK) goto END;
		}

	END:
	ShowSelect(CENTRE,wTrace);
	MyDisposeHandle((Handle*)pp_c);
	MyDisposeHandle((Handle*)pp_d);
	return(r);
	}


int MatchTemplate(tokenbyte ***pp_a,tokenbyte ***pp_b) {
/* p_a is item without structure, p_b is template */
/* On normal exit p_a should have structure derived from template */
	tokenbyte **p_c;
	double maxseqapprox;
	int r,hasperiods;

	r = OK;
	hasperiods = FALSE; p_c = NULL;
	// datamode = DisplayMode(pp_a,&ifunc,&hastabs);
	if((hasperiods=FoundPeriod(pp_b) && !HasStructure(*pp_b))) {
		if((p_c=(tokenbyte**) GiveSpace((Size) MyGetHandleSize((Handle)*pp_b))) == NULL) {
			r = ABORT; goto QUIT;
			}
		if(CopyBuf(pp_b,&p_c) == ABORT) {
			r = ABORT; goto QUIT;
			}
		if((r=PolyMake(pp_b,&maxseqapprox,NO)) == ABORT || r == EXIT) {
			goto QUIT;
			}
		if(r != OK) {
			MyDisposeHandle((Handle*)&p_c);
			return(MISSED);
			}
		}
	r = PrintArg(FALSE,TRUE,FALSE,FALSE,FALSE,TRUE,stdout,0,pp_a,pp_b);
	if(r == ABORT || r == EXIT) goto QUIT;
	if(r != OK) r = MISSED;
	if(hasperiods) {
	//	if(CopyBuf(&p_c,pp_b) == ABORT) r = ABORT;
		MyDisposeHandle((Handle*)&p_c);
		} 
		
	QUIT:
	return(r);
	}


int PrintResult(int expand,int w,int datamode,int ifunc,tokenbyte ***pp_a) {
	int r;
	tokenbyte **p_b;
	double maxseqapprox;

	r = OK;
	if(expand && datamode && !ifunc) {
		if((p_b=(tokenbyte**) GiveSpace((Size) MyGetHandleSize((Handle)*pp_a))) == NULL) {
			r = ABORT; goto QUIT;
			}
		if(CopyBuf(pp_a,&p_b) == ABORT) {
			r = ABORT; goto QUIT;
			}
		if((r=PolyMake(pp_a,&maxseqapprox,NO)) == ABORT || r == EXIT) {
			MyDisposeHandle((Handle*)&p_b);
			goto QUIT;
			}
		if(r != OK) {
			if((r=CopyBuf(&p_b,pp_a)) != OK) goto QUIT;
			MyDisposeHandle((Handle*)&p_b);
			datamode = FALSE;
			}
		}
	if((r = PrintArg(datamode,FALSE,1,datamode,ifunc,0,stdout,w,pp_Scrap,pp_a)) != OK) goto QUIT;
	if(expand && datamode && !ifunc) {
		if(CopyBuf(&p_b,pp_a) == ABORT) {
			r = ABORT; goto QUIT;
			}
		MyDisposeHandle((Handle*)&p_b);
		}
QUIT:
	return(r);
	}


int CheckItemProduced(t_gram *p_gram,int igram,tokenbyte ***pp_a,long *p_length,
	int template,int mode)  {
	int j,sign,result,r,datamode,ifunc,hastabs;
	unsigned long i,imax;
	long count;
	OSErr io;
	char c,buffer[2];
	result = OK;
	if(NoVariable(pp_a)) {
		datamode = DisplayMode(pp_a,&ifunc,&hastabs);
		/* if(datamode && !ifunc && !template) {
			if((result=PolyMake(pp_a,&maxseqapprox)) == ABORT || result == EXIT) {
				goto END;
				}
			} */
		(*p_length) = LengthOf(pp_a);
		if(template) MakeTemplate(pp_a);
		ResetDone = ifunc = FALSE;
		OkShowExpand = FALSE;
		if(template) {
			WriteTemplate(wGrammar,pp_a);
			Dirty[wGrammar] = TRUE;
			}
		else {
			if((*((*p_gram).p_subgram))[igram].destru) {
				if((result=Destroy(pp_a)) != OK) goto END;
				}
			(*p_length) = LengthOf(pp_a);
			if((result=PrintResult(datamode && hastabs,OutputWindow,hastabs,ifunc,pp_a)) != OK) goto END;
			Print(OutputWindow,"\n");
			r = check_and_remove_duplicate_last_line(OutFileName);
			if(trace_produce_all) BPPrintMessage(1,odInfo,"=> ???\n");
			if(r) {
				ItemNumber++;
				if(trace_produce_all) BPPrintMessage(1,odInfo,"=> *** %d\n",ItemNumber);
				if(!template && (rtMIDI || OutCsound || WriteMIDIfile)) {
					if(trace_produce_all) BPPrintMessage(1,odInfo,"Play #%d\n",ItemNumber);
					result = PlayBuffer(pp_a,NO);
					}
				}
			if(ItemNumber >= MaxItemsProduce) {
				BPPrintMessage(0,odInfo,"%ld items have been produced, as per the limit in settings.\n",(long)ItemNumber);
				result = ABORT;
				goto END;
				}
			}
		}
END:
	return(result);
	}


int check_and_remove_duplicate_last_line(const char *filename) {
	int result = TRUE;
	if(outPtr) {
		fflush(outPtr);
		fclose(outPtr);
		outPtr = NULL;
		}
	else return FALSE;
    FILE *file = my_fopen(1,filename,"r");
    if(file == NULL) return FALSE;
    char *lines[1000]; // Assuming a maximum of 1000 lines
    int line_count = 0;
    char buffer[5000];
    while(fgets(buffer,5000, file) != NULL) {
		remove_trailing_newline(buffer);
        lines[line_count] = strdup(buffer);
        line_count++;
   		}
    fclose(file);
    int is_duplicate = 0;
    if(line_count > 1) {
        for (int i = 0; i < line_count - 1; i++) {
            if(strcmp(lines[i], lines[line_count - 1]) == 0) {
                is_duplicate = 1;
                break;
				}
			}
		}
    // If duplicate, remove the last line.
    if(is_duplicate) {
        free(lines[line_count - 1]);
        line_count--;
		result = FALSE;
    	}
    // Write the updated content back to the file.
 	file = my_fopen(1,filename,"wb");
    if(file == NULL) return FALSE;
    for(int i = 0; i < line_count; i++) {
		fprintf(file,"%s\n",lines[i]);
        free(lines[i]);
    	}
    fclose(file);
	outPtr = fopen(filename,"a");
	return result;
	}
