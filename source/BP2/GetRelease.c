/* GetRelease.c (BP2 version CVS) */
/* WARNING: Record carefully any change done here as it may take ages to trace bad consequences */

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

/* WARNING: Record carefully any change done here as it may take ages to trace bad consequences */

ResetProject(int init)
{
long count;
int i,w,r;
Handle ptr;

PleaseWait();
if((r=ClearWindow(init,wGrammar)) != OK) return(r);
WillRandomize = FALSE;
if((r=ClearWindow(init,wAlphabet)) != OK) return(r);
PleaseWait();
if((r=ClearWindow(TRUE,wStartString)) != OK) return(r);
if((r=ClearWindow(init,wInteraction)) != OK) return(r);
if((r=ClearWindow(init,wGlossary)) != OK) return(r);
if((r=ClearWindow(init,iObjects)) != OK) return(r);
PleaseWait();
if((r=ClearWindow(init,wCsoundInstruments)) != OK) return(r);
if((r=ClearWindow(init,wTimeBase)) != OK) return(r);
if((r=ClearWindow(init,wMIDIorchestra)) != OK) return(r);
#if BP_CARBON_GUI
if((r=ResetKeyboard(YES)) != OK) return(r);
#endif /* BP_CARBON_GUI */
for(w=0; w < WMAX; w++) {
	PleaseWait();
#if BP_CARBON_GUI
	if(FileName[w][0] == '\0') continue;
	RemoveFirstLine(wData,FilePrefix[w]);
	if(w != wScript && w != wData && w != wScrap) ForgetFileName(w);
#else
	// just clearing the name of all files enough ? - akozar 20130830
	FileName[w][0] = '\0';
#endif /* BP_CARBON_GUI */
	}
/* TellOthersMyName(wKeyboard); */
PlayTicks = FALSE; SetTickParameters(0,MAXBEATS); ResetTickFlag = TRUE;
NotSaidKpress = TRUE;
IgnoreUndefinedVariables = ToldAboutPianoRoll = FALSE;
for(i=0; i < MAXPARAMCTRL; i++) ParamControl[i] = ParamKey[i] = -1;
HideWindow(Window[wPrototype2]);
HideWindow(Window[wPrototype3]);
HideWindow(Window[wPrototype4]);
HideWindow(Window[wPrototype5]);
HideWindow(Window[wPrototype6]);
HideWindow(Window[wPrototype7]);
HideWindow(Window[wPrototype8]);
if(init && !ScriptExecOn) {
	if(!ScriptExecOn) {
		WindowParID[iSettings] = ParIDbp2;
		TheVRefNum[iSettings] = RefNumbp2;
		}
	if(InitButtons() != OK) return(FAILED);
	if(LoadSettings(NULL, TRUE) == ABORT) return(ABORT);
	for(w=0; w < WMAX; w++) {
		switch(w) {
			case wGrammar:
			case wData:
			case wAlphabet:
			case wStartString:
			case wInteraction:
			case wGlossary:
			case wTimeBase:
			case iWeights:
			case iSettings:
			case iObjects:
			case wCsoundInstruments:
			case wMIDIorchestra:
				TheVRefNum[w] = RefNumStartUp;
				WindowParID[w] = ParIDstartup;
				break;
			}
		}
	}
#if BP_CARBON_GUI
if(ResetPannel() != OK) return(FAILED);
#endif /* BP_CARBON_GUI */
PleaseWait();
if(ReleaseProduceStackSpace() != OK) return(FAILED);
if(ReleaseObjectPrototypes() != OK) return(FAILED);
PleaseWait();
if(ReleaseGrammarSpace() != OK) return(FAILED);
PleaseWait();
if(ReleaseVariableSpace() != OK) return(FAILED);
if(ReleaseAlphabetSpace() != OK) return(FAILED);
if(ReleasePatternSpace() != OK) return(FAILED);
PleaseWait();
if(ReleaseGlossarySpace() != OK) return(FAILED);
if(ReleaseScriptSpace() != OK) return(FAILED);
if(ReleaseConstants() != OK) return(FAILED);
PleaseWait();
#if BP_CARBON_GUI
if(ResetInteraction() != OK) return(FAILED);
#endif /* BP_CARBON_GUI */
ItemNumber = 0L;
ptr = (Handle) p_Initbuff;
MyDisposeHandle(&ptr);
p_Initbuff = NULL;
for(i=0; i <= MAXCHAN; i++) {
	PleaseWait();
	PressRate[i] = SamplingRate;
	PitchbendRate[i] = SamplingRate;
	ModulationRate[i] = SamplingRate;
	VolumeRate[i] = SamplingRate;
	PanoramicRate[i] = SamplingRate;
	VolumeControl[i] = VolumeController;
	PitchbendRange[i] = DeftPitchbendRange;
	PanoramicControl[i] = PanoramicController;
	}
LastAction = NO;
if(TraceMemory) {
	sprintf(Message,"New project memory use: %ld [leaked: %ld]",
		(long) MemoryUsed,(long)MemoryUsed - MemoryUsedInit);
	ShowMessage(TRUE, wMessage, Message);
	}
return(DoSystem());
}


ResetScriptQueue(void)
// Reset the (stacked) list of scripts currently being executed
{
int i;
Handle ptr;

ScriptExecOn = 0;
if(p_StringList != NULL) {
	for(i=0; i < (MyGetHandleSize((Handle)p_StringList) / sizeof(char**)); i++) {
		ptr = (Handle) (*p_StringList)[i];
		MyDisposeHandle(&ptr);
		}
	}
MyDisposeHandle((Handle*)pp_StringList);
SizeStringList = 10; NrStrings = 0;
if((p_StringList=(char****)GiveSpace((Size)(SizeStringList * sizeof(char**))))
	== NULL) return(ABORT);
for(i=0; i < SizeStringList; i++) (*p_StringList)[i] = NULL;
return(DoSystem());
}


ReleasePhaseDiagram(int nmax,unsigned long*** pp_imaxseq)
{
int i,k,nseq;
p_list **ptag,**newptag;
Handle ptr;

ptr = (Handle) p_T;
MyDisposeHandle(&ptr);
p_T = NULL;
MyDisposeHandle((Handle*)pp_imaxseq);
if(p_Seq != NULL) {
	for(nseq = 0; nseq < (MyGetHandleSize((Handle)p_Seq) / sizeof(short**)); nseq++) {
		ptr = (Handle) (*p_Seq)[nseq];
		MyDisposeHandle(&ptr);
		}
	ptr = (Handle) p_Seq;
	MyDisposeHandle(&ptr);
	p_Seq = NULL;
	}
	
if(p_Instance != NULL) {
	for(k=0; k < (MyGetHandleSize((Handle)p_Instance) / sizeof(SoundObjectInstanceParameters)); k++) {
		for(i=0; i < ((*p_Instance)[k].contparameters.number); i++) {
			ptr = (Handle) (*((*p_Instance)[k].contparameters.values))[i].point;
			MyDisposeHandle(&ptr);
			}
		ptr = (Handle) (*p_Instance)[k].contparameters.values;
		MyDisposeHandle(&ptr);
		}
	}
	
if(p_ObjectSpecs != NULL) {
	for(i=0; i < (MyGetHandleSize((Handle)p_ObjectSpecs) / sizeof(objectspecs**)); i++) {
		if((*p_ObjectSpecs)[i] != NULL) {
			if(WaitList(i) != NULL) {
				ptag = WaitList(i);
				if((**ptag).p != NULL) {
					do {
						newptag = (**ptag).p;
						MyDisposeHandle((Handle*)&ptag);
						ptag = newptag;
						}
					while(ptag != NULL);
					WaitList(i) = NULL;	/* Already disposed of */
					}
				else {
					MyDisposeHandle((Handle*)&ptag);
					WaitList(i) = NULL;
					}
				}
			if(ObjScriptLine(i) != NULL) {
				ptag = ObjScriptLine(i);
				if((**ptag).p != NULL) {
					do {
						newptag = (**ptag).p;
						MyDisposeHandle((Handle*)&ptag);
						ptag = newptag;
						}
					while(ptag != NULL);
					ObjScriptLine(i) = NULL;	/* Already disposed of */
					}
				else {
					MyDisposeHandle((Handle*)&ptag);
					ObjScriptLine(i) = NULL;
					}
				}
			if(SwitchState(i) != NULL) {
				ptr = (Handle) SwitchState(i);
				MyDisposeHandle(&ptr);
				SwitchState(i) = NULL;
				}
			ptr = (Handle) (*p_ObjectSpecs)[i];
			MyDisposeHandle(&ptr);
			(*p_ObjectSpecs)[i] = NULL;
			}
		
		}
	ptr = (Handle) p_ObjectSpecs;
	MyDisposeHandle(&ptr);
	p_ObjectSpecs = NULL;
	}
ptr = (Handle) p_Instance;
MyDisposeHandle(&ptr);
p_Instance = NULL;

return(DoSystem());
}

#if 0  /* this function appears not to be called - akozar */
ReleaseWindowSpace(void)
{
int i,w;
Handle ptr;

for(i=0; i < Jbutt; i++) {
	DisposeControl(Hbutt[i]);
	}
Jbutt = 0;
for(w=0; w < MAXWIND; w++) {
	if(Window[w] != NULL) {
		DisposeWindow(Window[w]);
		Window[w] = NULL;
		}
	else {
		sprintf(Message,"Window #%ld is NULL. ",(long)w);
		if(Beta) Alert1(Message);
		}
	}
for(w=MAXWIND; w < WMAX; w++) {
	if(gpDialogs[w] != NULL) {	// Releasing DITLs by hand seems bad, especially in Carbon - akozar
		/*ptr = (Handle) ((DialogPeek)gpDialogs[w])->items;
		MyDisposeHandle(&ptr);
		((DialogPeek)gpDialogs[w])->items = NULL;*/
		DisposeDialog(gpDialogs[w]);
		gpDialogs[w] = NULL;
		Window[w] = NULL;
		}
	else {
		sprintf(Message,"Dialog #%ld is NULL. ",(long)w);
		if(Beta) Alert1(Message);
		}
	/* if(Editable[w]) TextDispose(TEH[w]); */
	}
DisposeDialog(ReplaceCommandPtr);
DisposeDialog(ResumeStopPtr);
DisposeDialog(ResumeUndoStopPtr);
DisposeDialog(MIDIkeyboardPtr);
DisposeDialog(GreetingsPtr);
DisposeDialog(FAQPtr);
DisposeDialog(SixteenPtr);
DisposeDialog(MIDIprogramPtr);
return(DoSystem());
}
#endif

ReleaseAlphabetSpace(void)
{
int j,jmax;
Handle ptr;

CompiledAl = CompiledGr = CompiledGl = FALSE;
NoAlphabet = TRUE; Jfunc = 0;
if(p_Bol == NULL) jmax = 0;
else jmax = MyGetHandleSize((Handle)p_Bol) / sizeof(char**);
for(j=0; j < jmax; j++) {
	ptr = (Handle)(*p_Bol)[j];
	MyDisposeHandle(&ptr);
	}
ptr = (Handle) p_Bol;
MyDisposeHandle(&ptr);
p_Bol = NULL;

if(DoSystem() != OK) return(ABORT);

if(p_Image == NULL) jmax = 0;
else {
	jmax = MyGetHandleSize((Handle)p_Image) / sizeof(int**);
	if(p_Homo == NULL) if(Beta) Alert1("Err. ReleaseAlphabetSpace(). p_Image = NULL");
	}
for(j=0; j < jmax; j++) {
	ptr = (Handle)(*p_Image)[j];
	MyDisposeHandle(&ptr);
	(*p_Image)[j] = NULL;
	}
if(p_Homo == NULL) jmax = 0;
else jmax = MyGetHandleSize((Handle)p_Homo) / sizeof(int**);
for(j=0; j < jmax; j++) {
	ptr = (Handle)(*p_Homo)[j];
	MyDisposeHandle(&ptr);
	(*p_Homo)[j] = NULL;
	}
if(p_NoteImage == NULL) jmax = 0;
else jmax = MyGetHandleSize((Handle)p_NoteImage) / sizeof(int**);
for(j=0; j < jmax; j++) {
	ptr = (Handle)(*p_NoteImage)[j];
	MyDisposeHandle(&ptr);
	(*p_NoteImage)[j] = NULL;
	}
ptr = (Handle) p_Image;
MyDisposeHandle(&ptr);
p_Image = NULL;
ptr = (Handle) p_NoteImage;
MyDisposeHandle(&ptr);
p_NoteImage = NULL;
ptr = (Handle) p_Homo;
MyDisposeHandle(&ptr);
p_Homo = NULL;
Jbol = Jhomo = 0;
return(DoSystem());
}


ReleasePatternSpace(void)
{
int j;
Handle ptr;

for(j=0; j < (MyGetHandleSize((Handle)p_Patt) / sizeof(char**)); j++) {
	ptr = (Handle) (*p_Patt)[j];
	MyDisposeHandle(&ptr);
	}
ptr = (Handle) p_Patt;
MyDisposeHandle(&ptr);
p_Patt = NULL;
ptr = (Handle) p_Ppatt;
MyDisposeHandle(&ptr);
p_Ppatt = NULL;
ptr = (Handle) p_Qpatt;
MyDisposeHandle(&ptr);
p_Qpatt = NULL;
Jpatt = 0; CompiledPt = FALSE;
return(DoSystem());
}


ReleaseGrammarSpace(void)
{
int igram,irul,j;
t_rule rule;
p_flaglist **h,**h1;
Handle ptr;

if(Gram.number_gram >= 1 && Gram.p_subgram != NULL) {
	for(igram=1; igram < (MyGetHandleSize((Handle)Gram.p_subgram) / sizeof(t_subgram));
																			igram++) {
		if((*(Gram.p_subgram))[igram].p_rule != NULL) {
			for(irul=1; irul < (MyGetHandleSize((Handle)(*(Gram.p_subgram))[igram].p_rule)
					/ sizeof(t_rule)); irul++) {
				rule = (*((*(Gram.p_subgram))[igram].p_rule))[irul];
				if(rule.p_leftcontext != NULL) {
					ptr = (Handle)
				(*((*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftcontext))->p_arg;
					MyDisposeHandle(&ptr);
					ptr = (Handle)
							(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftcontext;
					MyDisposeHandle(&ptr);
					}
				if(rule.p_rightcontext != NULL) {
					ptr = (Handle)
				(*((*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightcontext))->p_arg;
					MyDisposeHandle(&ptr);
					ptr = (Handle)
							(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightcontext;
					MyDisposeHandle(&ptr);
					}
				ptr = (Handle) (*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftarg;
				MyDisposeHandle(&ptr);
				ptr = (Handle) (*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightarg;
				MyDisposeHandle(&ptr);
				h = (*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftflag;
				if(h != NULL) {
					do {
						if((**h).x > Jflag || (**h).x < 0) {
							sprintf(Message,"Err in flag list. ");
							if(Beta) Alert1(Message);
							break;
							}
						h1 = (**h).p;
						MyDisposeHandle((Handle*)&h);
						h = h1;
						}
					while(h != NULL);
					}
				h = (*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightflag;
				if(h != NULL) {
					do {
						if((**h).x > Jflag || (**h).x < 0) {
							sprintf(Message,"Err in flag list. ");
							if(Beta) Alert1(Message);
							break;
							}
						h1 = (**h).p;
						MyDisposeHandle((Handle*)&h);
						h = h1;
						}
					while(h != NULL);
					}
				}
			ptr = (Handle) (*(Gram.p_subgram))[igram].p_rule;
			MyDisposeHandle(&ptr);
			(*(Gram.p_subgram))[igram].p_rule = NULL;
			}
		}
	ptr = (Handle) Gram.p_subgram;
	MyDisposeHandle(&ptr);
	Gram.p_subgram = NULL;
	}
ptr = (Handle) p_InitScriptLine;
MyDisposeHandle(&ptr);
p_InitScriptLine = NULL;
MaxRul = N_err = 0;
MaxGram = Gram.number_gram = 0;
CompiledGr = Gram.trueBP = Gram.hasTEMP = Gram.hasproc = FALSE;
BolsInGrammar = 0;
InitThere = 0;
FirstTime = FALSE;
return(ReleaseFlagSpace());
}


ReleaseGlossarySpace(void)
{
int irul,j;
t_rule rule;
Handle ptr;

if(GlossGram.p_subgram != NULL) {
	if(MyGetHandleSize((Handle)(GlossGram.p_subgram)) < 2 * sizeof(t_gram)) {
		if(Beta) Alert1("Err. ReleaseGlossarySpace()");
		return(ABORT);
		}
	if((*(GlossGram.p_subgram))[1].p_rule != NULL) {
		for(irul=1; irul < (MyGetHandleSize((Handle)(*(GlossGram.p_subgram))[1].p_rule)
					/ sizeof(t_rule)); irul++) {
			ptr = (Handle) (*((*(GlossGram.p_subgram))[1].p_rule))[irul].p_leftarg;
			MyDisposeHandle((Handle*)&ptr);
			ptr = (Handle) (*((*(GlossGram.p_subgram))[1].p_rule))[irul].p_rightarg;
			MyDisposeHandle((Handle*)&ptr);
			}
		ptr = (Handle) (*(GlossGram.p_subgram))[1].p_rule;
		MyDisposeHandle(&ptr);
		(*(GlossGram.p_subgram))[1].p_rule = NULL;
		}
	}
ptr = (Handle) GlossGram.p_subgram;
MyDisposeHandle((Handle*)&ptr);
GlossGram.p_subgram = NULL;
GlossGram.number_gram = 0;
ResetVariables(wGlossary);
CompiledGl = FALSE;
return(DoSystem());
}


ReleaseFlagSpace(void)
{
int j;
Handle ptr;

if(p_Flagname != NULL) {
	if(Jflag > 0) {
		for(j=1; j < (MyGetHandleSize((Handle)p_Flagname) / sizeof(char**)); j++) {
			ptr = (Handle) (*p_Flagname)[j];
			MyDisposeHandle(&ptr);
			}
		}
	ptr = (Handle) p_Flagname;
	MyDisposeHandle(&ptr);
	p_Flagname = NULL;
	}
Jflag = MaxFlag = 0;
ptr = (Handle) p_Flag;
MyDisposeHandle(&ptr);
p_Flag = NULL;
CompiledGr = FALSE;
return(DoSystem());
}


ReleaseVariableSpace(void)
{
int j;
Handle ptr;

if(p_Var != NULL) {
	for(j=0; j < (MyGetHandleSize((Handle)p_Var) / sizeof(char**)); j++) {
		ptr = (Handle) (*p_Var)[j];
		MyDisposeHandle(&ptr);
		}
	}
ptr = (Handle) p_Var;
MyDisposeHandle(&ptr);
p_Var = NULL;
ptr = (Handle) p_VarStatus;
MyDisposeHandle(&ptr);
p_VarStatus = NULL;
Jvar = MaxVar = 0;
CompiledGl = CompiledGr = FALSE;
return(DoSystem());
}


ReleaseScriptSpace(void)
{
int j;
Handle ptr;

if(p_Script != NULL) {
	for(j=0; j < (MyGetHandleSize((Handle)p_Script) / sizeof(char**)); j++) {
		ptr = (Handle) (*p_Script)[j];
		MyDisposeHandle(&ptr);
		}
	ptr = (Handle) p_Script;
	MyDisposeHandle(&ptr);
	p_Script = NULL;
	}
Jscriptline = 0;
Jinscript = 0; /* also impossible to execute script instructions from interaction */
CompiledGr = CompiledGl = FALSE;
return(DoSystem());
}


ReleaseProduceStackSpace(void)
{
Handle ptr;

ptr = (Handle) p_MemGram;
MyDisposeHandle(&ptr);
p_MemGram = NULL;
ptr = (Handle) p_MemRul;
MyDisposeHandle(&ptr);
p_MemRul = NULL;
ptr = (Handle) p_MemPos;
MyDisposeHandle(&ptr);
p_MemPos = NULL;
ptr = (Handle) p_LastStackIndex;
MyDisposeHandle(&ptr);
p_LastStackIndex = NULL;
ptr = (Handle) p_ItemStart;
MyDisposeHandle(&ptr);
p_ItemStart = NULL;
ptr = (Handle) p_ItemEnd;
MyDisposeHandle(&ptr);
p_ItemEnd = NULL;
return(DoSystem());
}


ReleaseConstants(void)
{
int j,maxparam;
Handle ptr;

if(p_StringConstant == NULL) goto MORE;

maxparam = (MyGetHandleSize((Handle)p_StringConstant) / sizeof(char**));

for(j=0; j < maxparam; j++) {
	ptr = (Handle) (*p_StringConstant)[j];
	MyDisposeHandle(&ptr);
	}
ptr = (Handle) p_StringConstant;
MyDisposeHandle(&ptr);
p_StringConstant = NULL;

MORE:

ptr = (Handle) p_NumberConstant;
MyDisposeHandle(&ptr);
p_NumberConstant = NULL;

return(OK);
}


ReleaseCsoundInstruments(void)
{
int i,j;
Handle ptr;

for(j=0; j < Jinstr; j++) {
	ptr = (Handle)(*pp_CsInstrumentName)[j];
	MyDisposeHandle(&ptr);
	ptr = (Handle)(*pp_CsInstrumentComment)[j];
	MyDisposeHandle(&ptr);
	for(i=0; i < (*p_CsInstrument)[j].ipmax; i++) {
		if((*p_CsInstrument)[j].paramlist == NULL) {
			if(Beta) Alert1("Err. ReleaseCsoundInstruments(). (*p_CsInstrument)[j].paramlist == NULL");
			break;
			}
		ptr = (Handle) (*((*p_CsInstrument)[j].paramlist))[i].name;
		MyDisposeHandle(&ptr);
		ptr = (Handle) (*((*p_CsInstrument)[j].paramlist))[i].comment;
		MyDisposeHandle(&ptr);
		}
	ptr = (Handle) (*p_CsInstrument)[j].paramlist;
	MyDisposeHandle(&ptr);
	}
MyDisposeHandle((Handle*)&p_CsInstrument);
MyDisposeHandle((Handle*)&pp_CsInstrumentName);
MyDisposeHandle((Handle*)&pp_CsInstrumentComment);
MyDisposeHandle((Handle*)&p_CsInstrumentIndex);
MyDisposeHandle((Handle*)&p_CsPitchFormat);
MyDisposeHandle((Handle*)&p_CsDilationRatioIndex);
MyDisposeHandle((Handle*)&p_CsAttackVelocityIndex);
MyDisposeHandle((Handle*)&p_CsReleaseVelocityIndex);
MyDisposeHandle((Handle*)&p_CsPitchIndex);
MyDisposeHandle((Handle*)&p_CsPitchBendStartIndex);
MyDisposeHandle((Handle*)&p_CsVolumeStartIndex);
MyDisposeHandle((Handle*)&p_CsPressureStartIndex);
MyDisposeHandle((Handle*)&p_CsModulationStartIndex);
MyDisposeHandle((Handle*)&p_CsPanoramicStartIndex);
MyDisposeHandle((Handle*)&p_CsPitchBendEndIndex);
MyDisposeHandle((Handle*)&p_CsVolumeEndIndex);
MyDisposeHandle((Handle*)&p_CsPressureEndIndex);
MyDisposeHandle((Handle*)&p_CsModulationEndIndex);
MyDisposeHandle((Handle*)&p_CsPanoramicEndIndex);
for(i=0; i < 6; i++) {
	MyDisposeHandle((Handle*)&(p_CsPitchBend[i]));
	MyDisposeHandle((Handle*)&(p_CsVolume[i]));
	MyDisposeHandle((Handle*)&(p_CsPressure[i]));
	MyDisposeHandle((Handle*)&(p_CsModulation[i]));
	MyDisposeHandle((Handle*)&(p_CsPanoramic[i]));
	}
Jinstr = 0;
return(OK);
}


ResizeCsoundInstrumentsSpace(int howmany)
{
int i,j;
char **ptr;

if(howmany != Jinstr) CompiledCsObjects = CompiledRegressions = FALSE;

if(howmany == 0) {
	if(Beta) Alert1("Err. ResizeCsoundInstrumentsSpace(). howmany == 0");
	return(ReleaseCsoundInstruments());
	}
if(Jinstr == 0) {
	if((p_CsInstrument=(CsoundInstrument**) GiveSpace((Size)howmany * sizeof(CsoundInstrument))) == NULL) return(ABORT);
	if((p_CsInstrumentIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsPitchFormat=(char**) GiveSpace((Size)howmany * sizeof(char))) == NULL) return(ABORT);
	if((p_CsDilationRatioIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsAttackVelocityIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsReleaseVelocityIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsPitchIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsPitchBendStartIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsVolumeStartIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsPressureStartIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsModulationStartIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsPanoramicStartIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsPitchBendEndIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsVolumeEndIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsPressureEndIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsModulationEndIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((p_CsPanoramicEndIndex=(int**) GiveSpace((Size)howmany * sizeof(int))) == NULL) return(ABORT);
	if((pp_CsInstrumentName=(char****) GiveSpace((Size)howmany * sizeof(char**))) == NULL) return(ABORT);
	if((pp_CsInstrumentComment=(char****) GiveSpace((Size)howmany * sizeof(char**))) == NULL) return(ABORT);
	for(i=0; i < 6; i++) {
		if((p_CsPitchBend[i]=(double**) GiveSpace((Size)howmany * sizeof(double))) == NULL) return(ABORT);
		if((p_CsVolume[i]=(double**) GiveSpace((Size)howmany * sizeof(double))) == NULL) return(ABORT);
		if((p_CsPressure[i]=(double**) GiveSpace((Size)howmany * sizeof(double))) == NULL) return(ABORT);
		if((p_CsModulation[i]=(double**) GiveSpace((Size)howmany * sizeof(double))) == NULL) return(ABORT);
		if((p_CsPanoramic[i]=(double**) GiveSpace((Size)howmany * sizeof(double))) == NULL) return(ABORT);
		}
	}
	
for(j=howmany; j < Jinstr; j++) {
	/* Downsizing */
	ptr = (*pp_CsInstrumentName)[j];
	MyDisposeHandle((Handle*)&ptr);
	ptr = (*pp_CsInstrumentComment)[j];
	MyDisposeHandle((Handle*)&ptr);
	for(i=0; i < (*p_CsInstrument)[j].ipmax; i++) {
		if((*p_CsInstrument)[j].paramlist == NULL) {
			if(Beta) Alert1("Err. ResizeCsoundInstrumentsSpace(). (*p_CsInstrument)[j].paramlist == NULL");
			break;
			}
		ptr = (*((*p_CsInstrument)[j].paramlist))[i].name;
		MyDisposeHandle((Handle*)&ptr);
		ptr = (*((*p_CsInstrument)[j].paramlist))[i].comment;
		MyDisposeHandle((Handle*)&ptr);
		}
	MyDisposeHandle((Handle*)&((*p_CsInstrument)[j].paramlist));
	}

if(MySetHandleSize((Handle*)&p_CsInstrument,(Size)howmany * sizeof(CsoundInstrument)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsInstrumentIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsPitchFormat,(Size)howmany * sizeof(char)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsDilationRatioIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsAttackVelocityIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsReleaseVelocityIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsPitchIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsPitchBendStartIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsVolumeStartIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsPressureStartIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsModulationStartIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsPanoramicStartIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsPitchBendEndIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsVolumeEndIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsPressureEndIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsModulationEndIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&p_CsPanoramicEndIndex,(Size)howmany * sizeof(int)) != OK) return(ABORT);

if(MySetHandleSize((Handle*)&pp_CsInstrumentName,(Size)howmany * sizeof(char**)) != OK) return(ABORT);
if(MySetHandleSize((Handle*)&pp_CsInstrumentComment,(Size)howmany * sizeof(char**)) != OK) return(ABORT);

for(i=0; i < 6; i++) {
	if(MySetHandleSize((Handle*)&(p_CsPitchBend[i]),(Size)howmany * sizeof(double)) != OK) return(ABORT);
	if(MySetHandleSize((Handle*)&(p_CsVolume[i]),(Size)howmany * sizeof(double)) != OK) return(ABORT);
	if(MySetHandleSize((Handle*)&(p_CsPressure[i]),(Size)howmany * sizeof(double)) != OK) return(ABORT);
	if(MySetHandleSize((Handle*)&(p_CsModulation[i]),(Size)howmany * sizeof(double)) != OK) return(ABORT);
	if(MySetHandleSize((Handle*)&(p_CsPanoramic[i]),(Size)howmany * sizeof(double)) != OK) return(ABORT);
	}

for(j=Jinstr; j < howmany; j++) {
	if((ptr=(char**) GiveSpace((Size)2L * sizeof(char))) == NULL) return(ABORT);
	(*pp_CsInstrumentName)[j] = ptr;
	(*((*pp_CsInstrumentName)[j]))[0] = '\0';
	if((ptr=(char**) GiveSpace((Size)2L * sizeof(char))) == NULL) return(ABORT);
	(*pp_CsInstrumentComment)[j] = ptr;
	(*((*pp_CsInstrumentComment)[j]))[0] = '\0';
	ResetCsoundInstrument(j,NO,YES);
	}
	
Jinstr = howmany;
if(iCsoundInstrument >= Jinstr) iCsoundInstrument = 0;
#if BP_CARBON_GUI
SetCsoundInstrument(iCsoundInstrument,-1);
#endif /* BP_CARBON_GUI */
return(OK);
}


ReleaseObjectPrototypes(void)
{
int i,j,maxsounds,max;
Handle ptr;

#if BP_CARBON_GUI
if(SaveCheck(iObjects) != OK) return(FAILED);
#endif /* BP_CARBON_GUI */
maxsounds = Jbol + Jpatt;
if(pp_MIDIcode == NULL) max = 0;
else max = MyGetHandleSize((Handle) pp_MIDIcode) / sizeof(MIDIcode**);
for(j=2; j < max; j++) {
	/* Here we also release time-patterns */
	ptr = (Handle) (*pp_MIDIcode)[j];
	if(MyDisposeHandle(&ptr) != OK) return(ABORT);
	(*pp_MIDIcode)[j] = NULL;
	(*p_MIDIsize)[j]  = ZERO; /* Added 30/3/2007 */
	ptr = (Handle) (*pp_CsoundTime)[j];
	if(MyDisposeHandle(&ptr) != OK) return(ABORT);
	(*pp_CsoundTime)[j] = NULL;
	ptr = (Handle) (*pp_Comment)[j];
	if(MyDisposeHandle(&ptr) != OK) return(ABORT);
	(*pp_Comment)[j] = NULL;
	ptr = (Handle) (*pp_CsoundScoreText)[j];
	if(MyDisposeHandle(&ptr) != OK) return(ABORT);
	(*pp_CsoundScoreText)[j] = NULL;
	if((*pp_CsoundScore)[j] != NULL) {
		for(i=0; i < (*p_CsoundSize)[j]; i++) {
			ptr = (Handle) (*((*pp_CsoundScore)[j]))[i].h_param;
			if(MyDisposeHandle(&ptr) != OK) return(ABORT);
			(*((*pp_CsoundScore)[j]))[i].h_param = NULL;
			}
		}
	ptr = (Handle) (*pp_CsoundScore)[j];
	if(MyDisposeHandle(&ptr) != OK) return(ABORT);
	(*pp_CsoundScore)[j] = NULL;
	(*p_CsoundSize)[j] = 0; /* Added 7/3/98 */
	}
if(DoSystem() != OK) return(ABORT);
if(ResizeObjectSpace(YES,2,0) != OK) return(ABORT);
#if BP_CARBON_GUI
SetField(NULL,wPrototype1,fProtoName,"[no sound-object prototype]");
SetField(NULL,wPrototype1,fDuration,"0");
SetField(NULL,wPrototype1,fPrototypeFileComment,"[Comments on file]");
SetField(NULL,wPrototype1,fPrototypeComment,"[Comments on prototype]");
SwitchOff(NULL,wPrototype1,bMIDIsequence);
SwitchOff(NULL,wPrototype1,bSampledSound);
SwitchOff(NULL,wPrototype1,bCsoundInstrument);
KillDiagrams(wPrototype1);
#endif /* BP_CARBON_GUI */

Dirty[iObjects] = Created[iObjects] = FALSE;
ObjectMode = ObjectTry = TempMemory = AskedTempMemory = FixedMaxQuantization = FALSE;
TempMemoryUsed = ZERO;

return(DoSystem());
}


MakeSoundObjectSpace(void)
{
int j,**ptr;
MIDIcode **ptr1;
Milliseconds **ptr2;

if((p_Type = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_FixScale = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_OkExpand = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_OkCompress = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_AlphaMin = (double**) GiveSpace((Size)2 *sizeof(double))) == NULL) goto ERR;
if((p_AlphaMax = (double**) GiveSpace((Size)2 *sizeof(double))) == NULL) goto ERR;
if((p_OkRelocate = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_BreakTempo = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_OkTransp = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_OkArticul = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_OkVolume = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_OkPan = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_OkMap = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_OkVelocity = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_ContBeg = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_ContEnd = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_CoverBeg = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_CoverEnd = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_TruncBeg = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_TruncEnd = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_PivType = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_PivMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_RescaleMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_DelayMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_ForwardMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_BreakTempoMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_ContBegMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_ContEndMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_CoverBegMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_CoverEndMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_TruncBegMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_TruncEndMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_PreRollMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_PostRollMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_PeriodMode = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_StrikeAgain = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_CompiledCsoundScore = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_DiscardNoteOffs = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_ForceIntegerPeriod = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_PivPos = (float**) GiveSpace((Size)2 *sizeof(float))) == NULL) goto ERR;
if((p_PreRoll = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_PostRoll = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_BeforePeriod = (float**) GiveSpace((Size)2 *sizeof(float))) == NULL) goto ERR;
if((p_MaxDelay = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_MaxForward = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_MaxBegGap = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_MaxEndGap = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_MaxCoverBeg = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_MaxCoverEnd = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_MaxTruncBeg = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_MaxTruncEnd = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;

if((p_AlphaCtrl = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_AlphaCtrlNr = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_AlphaCtrlChan = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;

if((p_MIDIsize = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_CsoundSize = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_Ifrom = (int**) GiveSpace((Size)2 *sizeof(int))) == NULL) goto ERR;
if((p_Quan = (double**) GiveSpace((Size)2 *sizeof(double))) == NULL) goto ERR;
if((p_DefaultChannel = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_PasteDone = (char**) GiveSpace((Size)2 *sizeof(char))) == NULL) goto ERR;
if((p_Tref = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_Tpict = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((p_ObjectColor = (RGBColor**) GiveSpace((Size)2 *sizeof(RGBColor))) == NULL) goto ERR;
if((p_Resolution = (int**) GiveSpace((Size)2 *sizeof(int))) == NULL) goto ERR;
if((p_CsoundInstr = (int**) GiveSpace((Size)2 *sizeof(int))) == NULL) goto ERR;
if((p_CsoundAssignedInstr = (int**) GiveSpace((Size)2 *sizeof(int))) == NULL) goto ERR;
if((p_CsoundTempo = (float**) GiveSpace((Size)2 *sizeof(float))) == NULL) goto ERR;

if((p_Dur = (long**) GiveSpace((Size)2 *sizeof(long))) == NULL) goto ERR;
if((pp_MIDIcode = (MIDIcode****) GiveSpace((Size)2 *sizeof(MIDIcode**))) == NULL) goto ERR;
if((pp_CsoundTime = (Milliseconds****) GiveSpace((Size)2 *sizeof(Milliseconds**))) == NULL) goto ERR;
if((pp_Comment = (char****) GiveSpace((Size)2 *sizeof(char**))) == NULL) goto ERR;
if((pp_CsoundScoreText = (char****) GiveSpace((Size)2 *sizeof(char**))) == NULL) goto ERR;
if((pp_CsoundScore = (CsoundLine****) GiveSpace((Size)2 *sizeof(CsoundLine**))) == NULL) goto ERR;
	 
for(j=0; j < 2 ; j++) {
	(*p_MIDIsize)[j] = (*p_CsoundSize)[j] = ZERO;
	(*p_Ifrom)[j] = 0; (*p_Type)[j] = 0;
	(*p_PivPos)[j] = (*p_BeforePeriod)[j] = 0.;
	(*p_PeriodMode)[j] = IRRELEVANT;
	(*p_PreRoll)[j] = (*p_PostRoll)[j] = ZERO;
	(*p_AlphaCtrlNr)[j] = 0; (*p_AlphaCtrlChan)[j] = 1;
	(*pp_MIDIcode)[j] = NULL; (*pp_CsoundTime)[j] = NULL;
	(*pp_Comment)[j] = (*pp_CsoundScoreText)[j] = NULL;
	(*pp_CsoundScore)[j] = NULL;
	(*p_CoverBeg)[j] = (*p_CoverEnd)[j] = (*p_OkRelocate)[j] = (*p_OkExpand)[j]
		= (*p_OkCompress)[j] = TRUE;
	(*p_StrikeAgain)[j] = -1;
	(*p_CompiledCsoundScore)[j] = TRUE;
	(*p_OkTransp)[j] = (*p_OkPan)[j] = (*p_OkMap)[j] = (*p_OkVolume)[j] = (*p_OkArticul)[j]
		= (*p_OkVelocity)[j] = (*p_DiscardNoteOffs)[j] = (*p_PasteDone)[j] = FALSE;
	(*p_BreakTempo)[j] = TRUE;
	(*p_FixScale)[j] = (*p_ContBeg)[j] = (*p_ContEnd)[j]
		= (*p_TruncBeg)[j] = (*p_TruncEnd)[j] = (*p_AlphaCtrl)[j] = (*p_ForceIntegerPeriod)[j]
		= FALSE;
	(*p_PivType)[j] = 1; (*p_PivMode)[j] = ABSOLUTE;
	(*p_PreRollMode)[j] = (*p_PostRollMode)[j] = RELATIVE;
	(*p_MaxDelay)[j] = (*p_MaxForward)[j] = ZERO;
	(*p_RescaleMode)[j] = LINEAR;
	(*p_AlphaMin)[j] = 0; (*p_AlphaMax)[j] = 100.;
		(*p_DelayMode)[j] = (*p_ForwardMode)[j] = (*p_BreakTempoMode)[j]
			= (*p_ContBegMode)[j] = (*p_ContEndMode)[j] = ABSOLUTE;
		(*p_CoverBegMode)[j] = (*p_CoverEndMode)[j] = (*p_TruncBegMode)[j]
			= (*p_TruncEndMode)[j] = RELATIVE;
	(*p_PeriodMode)[j] = IRRELEVANT;
	(*p_MaxBegGap)[j] = (*p_MaxEndGap)[j] = Infpos;
	(*p_MaxCoverBeg)[j] = (*p_MaxCoverEnd)[j] = 100L;
	(*p_MaxTruncBeg)[j] = (*p_MaxTruncEnd)[j] = 0L;
	(*p_Tref)[j] = ZERO; (*p_Resolution)[j] = 1; (*p_Dur)[j] = (*p_PivPos)[j]
	 = (*p_PreRoll)[j] = (*p_PostRoll)[j] = (*p_BeforePeriod)[j] = ZERO;
	(*p_DefaultChannel)[j] = (*p_Quan)[j] = 0;
	(*p_Tpict)[j] = Infneg;
	(*p_CsoundInstr)[j] = 0;
	(*p_CsoundAssignedInstr)[j] = -1;
	(*p_ObjectColor)[j].red = (*p_ObjectColor)[j].green = (*p_ObjectColor)[j].blue = -1L;
	}
(*p_Tref)[1] = (*p_Dur)[1] = 1000L;
(*p_OkPan)[1] = (*p_OkVolume)[1] = (*p_OkMap)[1] = (*p_OkVelocity)[1] = TRUE;

// Silence should have (empty) MIDI and TIME
// (these are used if silence is preceded by synchro tag)

if((ptr1 = (MIDIcode**) GiveSpace((Size)sizeof(MIDIcode))) == NULL) goto ERR;
(*pp_MIDIcode)[1] = ptr1;
(*((*pp_MIDIcode)[1]))[0].time = ZERO;
(*((*pp_MIDIcode)[1]))[0].byte = TimingClock;
(*((*pp_MIDIcode)[1]))[0].sequence = 0;

return(DoSystem());

ERR:
return(ABORT);
}


ResizeObjectSpace(int reset,int maxsounds,int addbol)
{
// Beware that if resizing down you should first dispose
// (*pp_MIDIcode)[j]), (*pp_CsoundTime)[j],
// (*pp_Comment)[j]) and (*pp_CsoundScoreText)[j] and (*pp_CsoundScore)[j]
// for unreachable values of j

int j;

MySetHandleSize((Handle*)&p_Type,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_FixScale,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_OkExpand,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_OkCompress,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_AlphaMin,(Size)maxsounds*sizeof(double));
MySetHandleSize((Handle*)&p_AlphaMax,(Size)maxsounds*sizeof(double));
MySetHandleSize((Handle*)&p_OkRelocate,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_BreakTempo,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_OkTransp,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_OkArticul,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_OkVolume,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_OkPan,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_OkMap,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_OkVelocity,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_ContBeg,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_ContEnd,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_CoverBeg,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_CoverEnd,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_TruncBeg,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_TruncEnd,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_PivType,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_PivMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_RescaleMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_DelayMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_ForwardMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_BreakTempoMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_ContBegMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_ContEndMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_CoverBegMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_CoverEndMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_TruncBegMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_TruncEndMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_PreRollMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_PostRollMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_PeriodMode,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_StrikeAgain,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_CompiledCsoundScore,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_DiscardNoteOffs,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_ForceIntegerPeriod,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_PivPos,(Size)maxsounds*sizeof(float));
MySetHandleSize((Handle*)&p_PreRoll,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_PostRoll,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_BeforePeriod,(Size)maxsounds*sizeof(float));
MySetHandleSize((Handle*)&p_MaxDelay,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_MaxForward,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_MaxBegGap,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_MaxEndGap,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_MaxCoverBeg,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_MaxCoverEnd,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_MaxTruncBeg,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_MaxTruncEnd,(Size)maxsounds*sizeof(long));

MySetHandleSize((Handle*)&p_AlphaCtrl,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_AlphaCtrlNr,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_AlphaCtrlChan,(Size)maxsounds*sizeof(char));

MySetHandleSize((Handle*)&p_MIDIsize,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_CsoundSize,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_Ifrom,(Size)maxsounds*sizeof(int));
MySetHandleSize((Handle*)&p_Quan,(Size)maxsounds*sizeof(double));
MySetHandleSize((Handle*)&p_DefaultChannel,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_PasteDone,(Size)maxsounds*sizeof(char));
MySetHandleSize((Handle*)&p_Tref,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_Tpict,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&p_ObjectColor,(Size)maxsounds*sizeof(RGBColor));
MySetHandleSize((Handle*)&p_Resolution,(Size)maxsounds*sizeof(int));
MySetHandleSize((Handle*)&p_CsoundInstr,(Size)maxsounds*sizeof(int));
MySetHandleSize((Handle*)&p_CsoundAssignedInstr,(Size)maxsounds*sizeof(int));
MySetHandleSize((Handle*)&p_CsoundTempo,(Size)maxsounds*sizeof(float));

MySetHandleSize((Handle*)&p_Dur,(Size)maxsounds*sizeof(long));
MySetHandleSize((Handle*)&pp_MIDIcode,(Size)maxsounds*sizeof(MIDIcode**));
MySetHandleSize((Handle*)&pp_CsoundTime,(Size)maxsounds*sizeof(Milliseconds**));
MySetHandleSize((Handle*)&pp_Comment,(Size)maxsounds*sizeof(char**));
MySetHandleSize((Handle*)&pp_CsoundScoreText,(Size)maxsounds*sizeof(char**));
MySetHandleSize((Handle*)&pp_CsoundScore,(Size)maxsounds*sizeof(CsoundLine**));

if(DoSystem() != OK) return(ABORT);

if(reset) {
	for(j=2; j < Jbol && j < maxsounds; j++) {
		(*pp_MIDIcode)[j] = NULL; (*pp_CsoundTime)[j] = NULL;
		(*pp_Comment)[j] = (*pp_CsoundScoreText)[j] = NULL;
		(*pp_CsoundScore)[j] = NULL;
		if(ResetPrototype(j) != OK) return(ABORT);
		(*p_BreakTempo)[j] = TRUE;
		(*p_Type)[j] = 0;
		}
#if BP_CARBON_GUI
	SetField(NULL,wPrototype5,fMinVelocity,"64");
	SetField(NULL,wPrototype5,fMaxVelocity,"64");
#endif /* BP_CARBON_GUI */
	}

// Create objects for time patterns
if(Jbol < maxsounds) {
	if(Jbol >= 2) j = Jbol;
	else j = 2;
	for(j=j; j < maxsounds; j++) {
		(*p_MIDIsize)[j] = (*p_CsoundSize)[j] = ZERO;
		(*p_Ifrom)[j] = 0; (*p_Type)[j] = 0;
		(*pp_MIDIcode)[j] = NULL; (*pp_CsoundTime)[j] = NULL;
		(*pp_Comment)[j] = (*pp_CsoundScoreText)[j] = NULL;
		(*pp_CsoundScore)[j] = NULL;
		(*p_AlphaCtrlNr)[j] = 0; (*p_AlphaCtrlChan)[j] = 1;
		(*p_CoverBeg)[j] = (*p_CoverEnd)[j] = (*p_PasteDone)[j] = FALSE;
		(*p_OkRelocate)[j] = FALSE; (*p_OkExpand)[j] = (*p_OkCompress)[j]
			= TRUE;
		(*p_StrikeAgain)[j] = -1;
		(*p_CompiledCsoundScore)[j] = TRUE;
		(*p_ContBeg)[j] = (*p_ContEnd)[j] = (*p_OkTransp)[j] = (*p_OkPan)[j] = (*p_OkMap)[j]
			= (*p_OkVelocity)[j] = (*p_OkArticul)[j] = (*p_OkVolume)[j]
			= (*p_DiscardNoteOffs)[j] = FALSE;
		(*p_BreakTempo)[j] = TRUE; (*p_FixScale)[j] = (*p_TruncBeg)[j]
			= (*p_TruncEnd)[j] = (*p_AlphaCtrl)[j] = (*p_ForceIntegerPeriod)[j] = FALSE;
		(*p_PivType)[j] = 1; (*p_PivMode)[j] = ABSOLUTE;
		(*p_PreRollMode)[j] = (*p_PostRollMode)[j] = RELATIVE;
		(*p_MaxDelay)[j] = (*p_MaxForward)[j] = ZERO;
		(*p_RescaleMode)[j] = LINEAR;
		(*p_AlphaMin)[j] = 0; (*p_AlphaMax)[j] = 100.;
		(*p_DelayMode)[j] = (*p_ForwardMode)[j] = (*p_BreakTempoMode)[j]
			= (*p_ContBegMode)[j] = (*p_ContEndMode)[j] = ABSOLUTE;
		(*p_CoverBegMode)[j] = (*p_CoverEndMode)[j] = (*p_TruncBegMode)[j]
			= (*p_TruncEndMode)[j] = RELATIVE;
		(*p_PeriodMode)[j] = IRRELEVANT;
		(*p_MaxBegGap)[j] = (*p_MaxEndGap)[j] = Infpos;
		(*p_MaxCoverBeg)[j] = (*p_MaxCoverEnd)[j] = ZERO;
		(*p_MaxTruncBeg)[j] = (*p_MaxTruncEnd)[j] = (*p_PivPos)[j]
			 = (*p_PreRoll)[j] = (*p_PostRoll)[j] = (*p_BeforePeriod)[j] = ZERO;
		(*p_Tref)[j] = 1000L;
		(*p_Dur)[j] = ZERO;
		if(j >= (Jbol + addbol)) {	/* Fixed 13/4/98 */
			if(Jpatt <= 0 || p_Ppatt != NULL && p_Qpatt != NULL) {
				if((*p_Ppatt)[j-Jbol-addbol] < 100L || (*p_Qpatt)[j-Jbol-addbol] < 100L) {
					(*p_Ppatt)[j-Jbol-addbol] = 100L * (*p_Ppatt)[j-Jbol-addbol];
					(*p_Qpatt)[j-Jbol-addbol] = 100L * (*p_Qpatt)[j-Jbol-addbol];
					}
				(*p_Tref)[j] = (*p_Qpatt)[j-Jbol-addbol];
				(*p_Dur)[j] = (*p_Ppatt)[j-Jbol-addbol];
				}
			else if(Beta) Alert1("Err. ResizeObjectSpace(). Jpatt <= 0 || p_Ppatt != NULL && p_Qpatt != NULL");
			}
		(*p_Resolution)[j] = 1;
		(*p_CsoundInstr)[j] = 0;
		(*p_CsoundAssignedInstr)[j] = -1;
		(*p_DefaultChannel)[j] = (*p_Quan)[j] = 0;
		(*p_Tpict)[j] = Infneg;
		(*p_ObjectColor)[j].red = (*p_ObjectColor)[j].green = (*p_ObjectColor)[j].blue = -1L;
		}
	}
return(DoSystem());
}


MakeEventSpace(unsigned long ***pp_imaxseq)
{
int nseq;
long k;

if((*pp_imaxseq = (unsigned long**) GiveSpace((Size)Maxconc*sizeof(unsigned long))) == NULL)
	return(ABORT);
if((p_Seq = (short****) GiveSpace((Size)Maxconc*sizeof(short**))) == NULL)
	return(ABORT);
for(nseq = 0; nseq < Maxconc; nseq++) (*p_Seq)[nseq] = NULL;

/* The following will be resized when Maxevent is better know, in FillPhaseDiagram() */

if((p_Instance = (SoundObjectInstanceParameters**) GiveSpace((Size)Maxevent*sizeof(SoundObjectInstanceParameters))) == NULL)
	return(ABORT);
	
if((p_ObjectSpecs = (objectspecs****) GiveSpace((Size)Maxevent*sizeof(objectspecs**)))
	== NULL) return(FAILED);
for(k = 0; k < Maxevent; k++) {
	(*p_ObjectSpecs)[k] = NULL;
	(*p_Instance)[k].contparameters.values = NULL;
	(*p_Instance)[k].contparameters.number = 0;
	}

return(DoSystem());
}


GetPatternSpace(void)
{
int i,j,**ptr1;
char **ptr2;

if((p_Patt = (char****) GiveSpace((Size)(Jpatt) * sizeof(char**))) == NULL) return(ABORT);
for(j=0; j < Jpatt; j++) (*p_Patt)[j] = NULL;
if((p_Ppatt = (long**) GiveSpace((Size)(Jpatt) * sizeof(long))) == NULL) return(ABORT);
if((p_Qpatt = (long**) GiveSpace((Size)(Jpatt) * sizeof(long))) == NULL) return(ABORT);
return(DoSystem());
}


GetGrammarSpace(void)
{
long pos,posmax;
int i,igram,newsubgram,maxrulesinsubgram,gap,numbergram;
char *p,*q,**p_line;

if(CheckEmergency() != OK) return(ABORT);
if(p_Var == NULL && GetVariableSpace() != OK) return(ABORT);
if(p_Flagname == NULL && GetFlagSpace() != OK) return(ABORT);
if(p_Script == NULL && GetScriptSpace() != OK) return(ABORT);
MaxGram = Gram.number_gram = 0;
numbergram = 1;
pos = ZERO;
posmax = GetTextLength(wGrammar);
p_line = NULL; maxrulesinsubgram = 0;
while(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) == OK) {
	if((*p_line)[0] == '\0') goto NEXTLINE;
	q = &(InitToken[0]);
	if(Match(TRUE,p_line,&q,4)) goto NEXTLINE;	/* Found "INIT:" */
	if((numbergram == 1) && (maxrulesinsubgram == 0)) {
		for(i=0; i < WMAX; i++) {
			if(FilePrefix[i][0] == '\0') continue;
			q = &(FilePrefix[i][0]);
			if(Match(TRUE,p_line,&q,4)) goto NEXTLINE;
			}
		}
	if(Mystrcmp(p_line,"DATA:") == 0) goto END;
	if(Mystrcmp(p_line,"COMMENT:") == 0) goto END;
	if(Mystrcmp(p_line,"TIMEPATTERNS:") == 0) {
		do {
			if(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) != OK)  goto END;
			if((*p_line)[0] == '\0') {
				goto NEXTLINE;
				}
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		goto NEXTLINE;
		}
	if(Mystrcmp(p_line,"TEMPLATES:") == 0) {
		do {
			if(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) != OK) goto END;
			if((*p_line)[0] == '\0') {
				goto NEXTLINE;
				}
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		goto NEXTLINE;
		}
	newsubgram = FALSE;
	if((*p_line)[0] == '-' && (*p_line)[MyHandleLen(p_line)-1] == '-') {
		newsubgram = TRUE;
		for(i=0; i < MAXARROW; i++) {
			if(strstr(*p_line,Arrow[i]) != NULLSTR) newsubgram = FALSE;
			}
		}
	if(newsubgram) {
		numbergram++;
		if(maxrulesinsubgram > MaxRul) MaxRul = maxrulesinsubgram;
		maxrulesinsubgram = 0;
		goto NEXTLINE;
		}
	maxrulesinsubgram++;
NEXTLINE: ;
	}

END:
MyDisposeHandle((Handle*)&p_line);
if(maxrulesinsubgram > MaxRul) MaxRul = maxrulesinsubgram;
if((Gram.p_subgram=(t_subgram**) GiveSpace((Size)(numbergram+1)*sizeof(t_subgram)))
						== NULL) return(ABORT);
for(igram=1; igram <= numbergram; igram++) {
	(*(Gram.p_subgram))[igram].p_rule = NULL;
	(*(Gram.p_subgram))[igram].number_rule = 0;
	}
MaxGram = numbergram;
return(DoSystem());
}


GetAlphabetSpace(void)
{
int i,j,**ptr1;
char **ptr2,**p_x;

if(Jbol < 2) {
	if(Beta) Alert1("Err. GetAlphabetSpace(). Jbol < 2");
	return(ABORT);
	}
if((p_x = (char**) GiveSpace((Size)((2) * sizeof(char)))) == NULL) {
	return(ABORT);
	}
if((p_Bol = (char****) GiveSpace((Size)(Jbol) * sizeof(char**))) == NULL) return(ABORT);
for(j=0; j < Jbol; j++) (*p_Bol)[j] = NULL;
if(p_Image != NULL || p_NoteImage != NULL || p_Homo != NULL) {
	if(Beta) Alert1("Err. GetAlphabetSpace(). p_Image != NULL || p_NoteImage != NULL || p_Homo != NULL");
	return(ABORT);
	}
/* p_Image = p_NoteImage = NULL; p_Homo = NULL; */
if(Jhomo > 0) {
	if((p_Image = (int****) GiveSpace((Size)(Jhomo) * sizeof(int**))) == NULL)
		return(ABORT);
	if((p_NoteImage = (int****) GiveSpace((Size)(Jhomo) * sizeof(int**))) == NULL)
		return(ABORT);
	if((p_Homo = (char****) GiveSpace((Size)(Jhomo) * sizeof(char**))) == NULL) return(ABORT);
	}
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
Jbol = 0; Jhomo = 0;
MystrcpyStringToHandle(&p_x,"_");
if(CreateBol(TRUE,FALSE,p_x,FALSE,FALSE,BOL) < 0) return(ABORT);
MystrcpyStringToHandle(&p_x,"-");
if(CreateBol(TRUE,FALSE,p_x,FALSE,FALSE,BOL) < 0) return(ABORT);
MyDisposeHandle((Handle*)&p_x);
return(DoSystem());
}


GetVariableSpace(void)
{
int i;

if(ReleaseVariableSpace() != OK) return(ABORT);
MaxVar = MAXVAR;
if((p_Var = (char****) GiveSpace((Size)MaxVar * sizeof(char**))) == NULL) {
	return(ABORT);
	}
if((p_VarStatus = (int**) GiveSpace((Size)MaxVar * sizeof(int))) == NULL) {
	return(ABORT);
	}
for(i=0; i < MaxVar; i++) {
	(*p_Var)[i] = NULL; (*p_VarStatus)[i] = 0;
	}
return(DoSystem());
}


GetFlagSpace(void)
{
int i;

if(ReleaseFlagSpace() != OK) return(ABORT);
MaxFlag = MAXFLAG;
if((p_Flagname = (char****) GiveSpace((Size)MaxFlag * sizeof(char**))) == NULL) {
	return(ABORT);
	}
for(i=0; i < MaxFlag; i++) (*p_Flagname)[i] = NULL;
if((p_Flag = (long**) GiveSpace((Size)MaxFlag * sizeof(long))) == NULL) {
	return(ABORT);
	}
return(DoSystem());
}


GetScriptSpace(void)
{
int i;

if(ReleaseScriptSpace() != OK) return(ABORT);
MaxScript = MAXEVENTSCRIPT;
if((p_Script = (char****) GiveSpace((Size)MaxScript * sizeof(char**))) == NULL) {
	return(ABORT);
	}
for(i=0; i < MaxScript; i++) (*p_Script)[i] = NULL;
Jscriptline = 0;
return(DoSystem());
}


CreateBuffer(tokenbyte*** pp_buff)
{
tokenbyte** ptr;

BufferSize = DeftBufferSize;
if(*pp_buff == NULL) {
	if((ptr=(tokenbyte**) GiveSpace((Size) MAXDISPL*sizeof(tokenbyte)))
		== NULL) return(ABORT);
	*pp_buff = ptr;
	}
else {
	MySetHandleSize((Handle*)pp_buff,(Size) MAXDISPL*sizeof(tokenbyte));
	}
(**pp_buff)[0] = (**pp_buff)[1] = TEND;
return(DoSystem());
}


MakeComputeSpace(int maxderiv)
{
if(maxderiv < 2) {
	if(Beta) Alert1("Err. MakeComputeSpace()");
	maxderiv = 10;
	}
if((p_MemGram == NULL) &&
	((p_MemGram = (int**) GiveSpace((Size)(maxderiv*sizeof(int)))) == NULL))
	return(ABORT);
if((p_MemRul == NULL) &&
	((p_MemRul = (int**) GiveSpace((Size)(maxderiv*sizeof(int)))) == NULL))
	return(ABORT);
if((p_MemPos == NULL) &&
	((p_MemPos = (long**) GiveSpace((Size)(maxderiv*sizeof(long)))) == NULL))
	return(ABORT);
if((p_LastStackIndex == NULL) &&
	((p_LastStackIndex = (long**) GiveSpace((Size)(maxderiv*sizeof(long))))
		== NULL)) return(ABORT);
if((p_ItemStart == NULL) &&
	((p_ItemStart = (long**) GiveSpace((Size)maxderiv*sizeof(long)))
		== NULL)) return(ABORT);
if((p_ItemEnd == NULL) &&
	((p_ItemEnd = (long**) GiveSpace((Size)maxderiv*sizeof(long)))
		== NULL)) return(ABORT);
return(DoSystem());
}


IncreaseComputeSpace(void)
{
Handle ptr;

if(ThreeOverTwo(&MaxDeriv) != OK) return(ABORT);
ptr = (Handle) p_MemGram;
if(ptr == NULL) {
	if(Beta) Alert1("Err. IncreaseComputeSpace(). ptr = NULL");
	return(ABORT);
	}
if((ptr = IncreaseSpace(ptr)) == NULL) return(ABORT);
p_MemGram = (int**) ptr;
ptr = (Handle) p_MemRul;
if((ptr = IncreaseSpace(ptr)) == NULL) return(ABORT);
p_MemRul = (int**) ptr;
ptr = (Handle) p_MemPos;
if((ptr = IncreaseSpace(ptr)) == NULL) return(ABORT);
p_MemPos = (long**) ptr;
ptr = (Handle) p_LastStackIndex;
if((ptr = IncreaseSpace(ptr)) == NULL) return(ABORT);
p_LastStackIndex = (long**) ptr;
ptr = (Handle) p_ItemStart;
if((ptr = IncreaseSpace(ptr)) == NULL) return(ABORT);
p_ItemStart = (long**) ptr;
ptr = (Handle) p_ItemEnd;
if((ptr = IncreaseSpace(ptr)) == NULL) return(ABORT);
p_ItemEnd = (long**) ptr;
return(DoSystem());
}


CheckTerminalSpace(void)
{
int i,j,oldjbol;
int **ptr;
char **p_x;

if(p_Bol == NULL) oldjbol = 0;
else oldjbol = MyGetHandleSize((Handle)p_Bol) / sizeof(char**);
if((p_x = (char**) GiveSpace((Size)((2) * sizeof(char)))) == NULL) {
	return(ABORT);
	}
if(Jbol > oldjbol) {
	CompiledPt = FALSE;
	MySetHandleSize((Handle*)&p_Bol,(Size)Jbol*sizeof(char**));
	for(i=oldjbol; i < Jbol; i++) {
		(*p_Bol)[i] = NULL;
		}
	for(i=0; i < Jhomo; i++) {
		ptr = (*p_Image)[i];
		MySetHandleSize((Handle*)&ptr,(Size)Jbol*sizeof(int));
		(*p_Image)[i] = ptr;
		/* Every homomorphism is set to identity */
		for(j=oldjbol; j < Jbol; j++) (*((*p_Image)[i]))[j] = j;
		}
	}
if(Jbol < 2) {
	Jbol = Jhomo = 0;
	MystrcpyStringToHandle(&p_x,"_");
	if(CreateBol(TRUE,FALSE,p_x,FALSE,FALSE,BOL) < 0) return(ABORT);
	MystrcpyStringToHandle(&p_x,"-");
	if(CreateBol(TRUE,FALSE,p_x,FALSE,FALSE,BOL) < 0) return(ABORT);
	}
MyDisposeHandle((Handle*)&p_x);
return(DoSystem());
}


ResetVariables(int w)
// Reset status of variables
{
int j;

if(p_VarStatus == NULL) return(OK);
if(p_Var == NULL) {
	if(Beta) Alert1("Err. ResetVariables(). p_Var = NULL");
	return(ABORT);
	}
if(w != wGrammar && w != wGlossary) return(FAILED);
for(j=1; j < (MyGetHandleSize((Handle)p_VarStatus) / sizeof(int)); j++) {
	if(w == wGrammar) (*p_VarStatus)[j] = (*p_VarStatus)[j] & (4+8);
	if(w == wGlossary) (*p_VarStatus)[j] = (*p_VarStatus)[j] & (1+2);
	}
return(OK);
}


KillSubTree(node n)
{
arc a,**h_a,**newh_a;
Handle ptr;

h_a = n.p;
while(h_a != NULL) {
	a = **h_a;
	KillSubTree(**a.p);
	ptr = (Handle) a.p;
	MyDisposeHandle(&ptr);
	newh_a = a.next;
	MyDisposeHandle((Handle*)&h_a);
	h_a = newh_a;
	}
n.accept = FALSE;
n.p = NULL;
return(OK);
}


ClearLockedSpace(void)
{
long i;

#if WITH_REAL_TIME_SCHEDULER
MyUnlock((Handle)p_Clock);
DisposeHandle((Handle)p_Clock);
MyUnlock((Handle)p_AllSlices);
DisposeHandle((Handle)p_AllSlices);
#endif

for(i=0; i < MaxProc; i++) {
	MyUnlock((Handle)(*p_GramProcedure)[i]);
	MyDisposeHandle((Handle*)&(*p_GramProcedure)[i]);
	}
MyUnlock((Handle)p_GramProcedure);
MyDisposeHandle((Handle*)&p_GramProcedure);

for(i=0; i < MaxPerformanceControl; i++) {
	MyUnlock((Handle)(*p_PerformanceControl)[i]);
	MyDisposeHandle((Handle*)&(*p_PerformanceControl)[i]);
	}
MyUnlock((Handle)p_PerformanceControl);
MyDisposeHandle((Handle*)&p_PerformanceControl);

for(i=0; i < 127; i++) {
	MyUnlock((Handle)(*p_GeneralMIDIpatch)[i]);
	MyDisposeHandle((Handle*)&(*p_GeneralMIDIpatch)[i]);
	}
MyUnlock((Handle)p_GeneralMIDIpatch);
MyDisposeHandle((Handle*)&p_GeneralMIDIpatch);

for(i=0; i < MAXHTMLDIACR; i++) {
	MyUnlock((Handle)(*p_Diacritical)[i]);
	MyDisposeHandle((Handle*)&(*p_Diacritical)[i]);
	}
MyUnlock((Handle)p_Diacritical);
MyDisposeHandle((Handle*)&p_Diacritical);

MyUnlock((Handle)p_HTMLdiacritical);
MyDisposeHandle((Handle*)&p_HTMLdiacritical);

return(OK);
}


CheckBuffer(unsigned long i,unsigned long *p_maxi,tokenbyte ***pp_c)
{
tokenbyte **ptr;
double size;

while(i > (*p_maxi)) {
	size = ((*p_maxi) * 3.) / 2.;
	if(size >= ULONG_MAX) {
		Alert1("Structure is getting too large. Task will be cancelled");
		return(ABORT);
		}
	(*p_maxi) = size;
	ptr = (*pp_c);
	if((ptr=(tokenbyte**) IncreaseSpace((Handle)ptr)) == NULL) {
		return(ABORT);
		}
	(*pp_c) = ptr;
	}
return(OK);
}


/*	FIXME ? The return value of ThreeOverTwo() is being used to abort every
	function that it is called from.  (It is called before calls to 
	IncreaseSpace()).  Should it check to make sure the calculation does 
	not overflow ?? -- akozar, 20130812
 */
ThreeOverTwo(long *p_x)
{
	long y;
	
	y = ((*p_x) * 3L) / 2L;
	*p_x = y;
	return(OK);
}


int CheckEmergency(void)
{
if(EmergencyExit) {
	Alert1("Out of memory. Save your work and exit...");
	return(NO);
	}
return(OK);
}


/*	FIXME ? There are about 60 calls to this function left,
	mostly as the last step in a function:
		return(DoSystem());
	It used to call SystemTask() and MemError() but now
	obviously only calls PlayTick().  It is not clear to me
	though whether those calls are needed or not.  PlayTick()
	always returns OK except after finishing capturing a 
	"tick cycle" when it returns STOP.  I don't see why dozens
	of other functions should potentially fail randomly in that
	rare case.
	
	So, should we 
		1.  Replace all/most calls to DoSystem() with PlayTick(FALSE), or
		2.  Remove all calls to DoSystem() by replacing calls in return
			statements with OK and by (carefully!) editing out other calls.
			EXCEPTION: The call from MainEvent() should be changed to
					   PlayTick(FALSE) or else it won't get called!
 */
DoSystem(void)
{
	if(Panic || EmergencyExit) return(OK);
	if(!SoundOn) return(PlayTick(FALSE));
	return(OK);
}
