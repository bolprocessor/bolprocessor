/* Dialogs.c (BP2 version CVS) */

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


DoDialog(EventRecord *p_event)
{
DialogPtr thedialog;
WindowPtr thewindow;
int i,ip,j,jj,improvizemem,stepProducemem,displayProducemem,traceProducemem,rs,
	w,rep,rep2,loadgrammar,rtn,showgraphic,noconstraint,showmessages,doneit,
	displaytimeset,vmin,vmax,changedtick,diff,oldcycle,hit,longerCsound,
	changed, tab;
short itemHit,itemtype,nature_time;
double dur;
Str255 t;
Rect r;
ControlHandle itemhandle;
char line[MAXFIELDCONTENT],c,**h;//, key;
unsigned long pclock,qclock;
long p,q;
Point point;
Handle ptr;
MIDI_Event e;

longerCsound = 0;

/* Trap 'return'; and 'tab' now too (akozar 061207) */
rtn = tab = FALSE;

if(p_event->what == keyDown) {
	/* Does not matter where mouse cursor is; key events should 
	   always go to the active window - akozar 052307 */
	// FindWindow(p_event->where,&thewindow);
	thewindow = FrontWindow();
	thedialog = GetDialogFromWindow(thewindow);
	if(Nw == wFindReplace && thedialog != gpDialogs[wFindReplace]) { // FIXME ??
		thedialog = gpDialogs[wFindReplace];
		thewindow = GetDialogWindow(thedialog);
		SelectWindow(thewindow);
		}
	for(w=0; w < WMAX; w++) {
		if(thewindow == Window[w]) {
			if(!IsDialog[w]) return(AGAIN);
			break;
			}
		}
	c = (char)(p_event->message & charCodeMask);
	// key = (char)(p_event->message & keyCodeMask) >> 8;
	if(c == '\t') tab = TRUE;
	if(c == '\r' || c == '\3') {	/* Return or Enter */
		if(w >= 0 && w < WMAX && Editable[w]
				&& w != wTimeBase  && w != wCsoundInstruments)
			return(AGAIN);  /* Return in editable text */
		switch(w) { // FIXME ? This switch statement is suspect ...
			case wTimeBase:
			case wMetronom:
			case wCsoundInstruments:
			case wTimeAccuracy:
			case wRandomSequence:
			case wKeyboard:
			case wBufferSize:
		//	case wFindReplace:
			case wGraphicSettings:
				SysBeep(10);
				if(GetDialogValues(w) != OK) return(DONE);
				if(w == wCsoundInstruments) GetRegressions(iCsoundInstrument);
				switch(w) {
					case wTimeAccuracy:
					case wRandomSequence:
					case wBufferSize:
					case wGraphicSettings:
						HideWindow(Window[w]);
						break;
					}
				return(DONE);
				break;
			}
		rtn = TRUE;
		}
	else {
		if(w >= 0 && w < WMAX && Editable[w]) {
			// Don't check mouse location! - akozar 052307
			/* r = LongRectToRect(TextGetViewRect(TEH[w]));
			if(PtInRect(p_event->where,&r)) */
			return(AGAIN);
			/* Typing in text area */
			}
		}
	if(w >= 0 && w < WMAX && !rtn && HasFields[w]) UpdateDirty(TRUE,w);
	}
	
/* Note: DialogSelect returns FALSE for tab key events but TRUE for all(?) others. */
hit = DialogSelect(p_event,&thedialog,&itemHit);
PrintEvent(p_event, "DoDialog()", GetDialogWindow(thedialog));
	
if(tab && !hit)  return(DONE);  // this hack prevents BP from beeping when tabbing to disabled edit text
if(!rtn && !hit) return(OK);

thewindow = GetDialogWindow(thedialog);
for(w=0; w < WMAX; w++) {
	if(thewindow == Window[w]) {
		if(!IsDialog[w]) {
			return(AGAIN);
			}
		break;
		}
	}

PrintEvent(p_event, "DoDialog() cont...", GetDialogWindow(thedialog));
if(w < WMAX && rtn) {
	if(FindGoodIndex(w) == wPrototype1) {
		if(GetDialogValues(w) != OK) return(DONE);
		SetPrototype(iProto);
		return(DONE);
		}
	}

rep = OK;
if(!rtn) {
	GetDialogItem(thedialog,itemHit,&itemtype,(Handle*)&itemhandle,&r);
	}
else {
	itemHit = 1;
	itemtype = (ctrlItem+btnCtrl); /* Replace with button type */
	rep = OK;
	}
/* FIXME: this next line only detects enabled edit text or static text items,
   thus events in disabled items are passed thru to the dialog handlers below.
   Probably should compare to (itemtype & 127) to remove the disable bit. */
if(itemtype == editText || (itemtype == statText && !Help)) {
	if(itemtype == editText && w < WMAX && Editable[w]) {
		OutlineTextInDialog(w,FALSE);
		}
	if(itemtype == statText) return(OK);
	if(p_event->what == keyDown) {
		if(thedialog == CsoundInstrMorePtr) UpdateDirty(TRUE,wCsoundInstruments);
		if(thedialog == FileSavePreferencesPtr
				|| thedialog == StrikeModePtr
				|| thedialog == TuningPtr
				|| thedialog == DefaultPerformanceValuesPtr)
			UpdateDirty(TRUE,iSettings);
		}
	return(DONE);
	}
SetCursor(&WatchCursor);
if((itemtype & 127) == (ctrlItem+btnCtrl) && itemHit == 1 && thedialog == FAQPtr) {
	Help = FALSE;
	HideWindow(GetDialogWindow(FAQPtr));
	return(DONE);
	}
if(Help && !rtn) {
	if(itemtype == statText) GetDialogItemText((Handle)itemhandle,t);
	else GetControlTitle(itemhandle,t);
	MyPtoCstr(255,t,line);
	FilterHelpSelector(line);
	DisplayHelp(line);
	if(thedialog == FAQPtr && strcmp("How do I register for BP2?",line) == 0) {
		// DisplayFile(wNotice,"License.txt");
		// Register();
		}
	return(DONE);
	}

Option = FALSE;
if((p_event->modifiers & optionKey) != 0) {
	Option = TRUE;
	SetOptionMenu(TRUE);
	}
if(w == wTickDialog) {
	if(iTick < 0 || jTick < 0) {
		if(Beta) Alert1("Err. DoDialog(). iTick < 0 || jTick < 0");
		return(ABORT);
		}
	switch(itemHit) {
		case dOK:
			GetThisTick();
			HideWindow(GetDialogWindow(gpDialogs[wTickDialog]));
			BPActivateWindow(QUICK,wTimeBase);
			jj = dPlayBeat + 55*iTick + jTick;
			GetDialogItem(gpDialogs[wTimeBase],(short)jj,&itemtype,(Handle*)&itemhandle,&r);
			if(ThisTick[iTick][jTick] < 2) HiliteControl(itemhandle,11);
			iTick = -1; jTick = -1;
			return(DONE);
			break;
		case dTickOn:
			SwitchOn(NULL,wTickDialog,dTickOn);
			SwitchOff(NULL,wTickDialog,dTickOff);
			break;
		case dTickOff:
			SwitchOn(NULL,wTickDialog,dTickOff);
			SwitchOff(NULL,wTickDialog,dTickOn);
			break;
		case dDefaultTick:
			SwitchOn(NULL,wTickDialog,dDefaultTick);
			SwitchOff(NULL,wTickDialog,dSpecialTick);
			HideDialogItem(gpDialogs[wTickDialog],fThisTickVelocity);
			HideDialogItem(gpDialogs[wTickDialog],fThisTickChannel);
			HideDialogItem(gpDialogs[wTickDialog],fThisTickKey);
			break;
		case dSpecialTick:
			SwitchOn(NULL,wTickDialog,dSpecialTick);
			SwitchOff(NULL,wTickDialog,dDefaultTick);
			ShowDialogItem(gpDialogs[wTickDialog],fThisTickVelocity);
			ShowDialogItem(gpDialogs[wTickDialog],fThisTickChannel);
			ShowDialogItem(gpDialogs[wTickDialog],fThisTickKey);
			SelectField(NULL,wTickDialog,fThisTickChannel,FALSE);
			break;
		case dRecordThisTick:
			RecordTick(3,0);
			break;
		}
	GetThisTick();
	return(DONE);
	}
if(w == wControlPannel) {
	if(GetDialogValues(w) != OK) return(DONE);
	switch(itemHit) {
		case dProduceItems:
			rep = mProduce(Nw);
			if(rep != EXIT) rep = DONE;
			break;
		case dTemplates:
			rep = mTemplates(Nw);
			if(rep != EXIT) rep = DONE;
			break;
		case dAnalyze:
			rep = mAnalyze(Nw);
			if(rep != EXIT) rep = DONE;
			break;
		case dTransposeInput:
			TransposeInput = 1 - TransposeInput;
			GetControlParameters();
			if(TransposeInput && ReadKeyBoardOn && Jcontrol == -1
					&& LastEditWindow != wScript && TransposeValue != 0) {
				if(!EmptyBeat) Print(LastEditWindow," ");
				PrintHandle(LastEditWindow,(*p_PerformanceControl)[33]);
				sprintf(Message,"(%ld)",(long) -TransposeValue);
				Print(LastEditWindow,Message);
				EmptyBeat = FALSE;
				}
			break;
		case dDeriveFurther:
			if(CompileCheck() == OK) {
				DeriveFurther = TRUE;
				if(!ResetWeights && Varweight
					&& (rep = Answer("Reset rule weights",'Y')) == OK) {
					ResetWeights = TRUE;
					NeverResetWeights = FALSE;
					}
				if(rep == ABORT) return(rep);
				if(!ResetFlags && Flagthere
					&& (rep = Answer("Reset rule flags",'Y')) == OK) {
					ResetFlags = TRUE;
					}
				if(rep == ABORT) return(rep);
				rep = ProduceItems(LastComputeWindow,FALSE,FALSE,NULL);
				if(rep != EXIT) rep = DONE;
				DeriveFurther = FALSE;
				}
			break;
		case bMute:
			if(!Oms && !NEWTIMER) Alert1("'Mute' only works when Opcode OMS is active");
			else {
				Mute = 1 - Mute;
				MaintainMenus();
				if(Mute) FlashInfo("MUTE is ON...   cmd-space will turn if off");
				else HideWindow(Window[wInfo]);
				}
			break;
		case dRepeatComputation:
			if(ProduceStackDepth == -1) {
				Alert1("Can't repeat... Decisions are lost!");
				return(DONE);
				}
			if(ProduceStackDepth < 2) {
				Alert1("Can't repeat... No item produced");
				return(DONE);
				}
			if(CompileOn) {
				Alert1("Can't repeat while compiling");
				return(DONE);
				}
			if(!CompiledAl || !CompiledGr) {
				if(Answer("Grammar has changed.\nTry anyway",'Y') != OK)
					 return(DONE);
				if(CompileCheck() != OK)  return(DONE);
				}
			stepProducemem = StepProduce;
			displayProducemem = DisplayProduce;
			traceProducemem = TraceProduce;
			improvizemem = Improvize;
			Improvize = FALSE;
			rep2 = OK;
			if(!StepProduce
				&& (rep = Answer("Step by step",'N')) == YES)
				StepProduce = DisplayProduce = TRUE;
			if(rep == ABORT) goto QUIT;
			if(!TraceProduce
				&& (rep = Answer("Show decisions",'N')) == YES) {
				TraceProduce = DisplayProduce = TRUE;
				}
			if(rep == ABORT) goto QUIT;
			if(!DisplayProduce && !StepProduce
				&& (rep = Answer("Display computation",'N')) == YES)
				DisplayProduce = TRUE;
			if(rep == ABORT) goto QUIT;
			SetButtons(FALSE);
			rep = ProduceItems(wStartString,TRUE,FALSE,NULL);
QUIT:		Improvize = improvizemem;
			StepProduce = stepProducemem;
			DisplayProduce = displayProducemem;
			TraceProduce = traceProducemem;
			SetButtons(TRUE);
			/* ComputeOn = FALSE will cause InterruptCompute() to abort */
			if(rep != OK) return(rep);
			return(mStop(Nw));
			break;
		case dLoadDecisions:
			loadgrammar = NO;
			if(IsEmpty(wGrammar)) loadgrammar = TRUE;
			else {
				if(!CompiledAl || !CompiledGr) {
					GetMiName();
					if(Answer("Grammar may have changed.\nTry anyway",'Y') != OK)
						return(OK);
					if((rep=Answer("Load grammar/alphabet\ninserted in decision file",
							'Y')) == OK) {
						if(ResetProject(FALSE) != OK) return(ABORT);
						loadgrammar = YES;
						}
					if(rep == ABORT) return(rep);
					}
				}
			rep = LoadDecisions(loadgrammar);
			if(rep != EXIT) rep = DONE;
			break;
		case dSaveDecisions:
			mSaveDecisions(w); break;
		case dShowGramWeights:
			if(CompileCheck() != OK) {
				Alert1("No valid grammar");
				return(DONE);
				}
			rep = DisplayGrammar(&Gram,wTrace,TRUE,TRUE,TRUE);
			if(rep != EXIT) rep = DONE;
			break;
		case dMIDIcheck:
			rep = Answer("Check MIDI output",'Y');
			if(rep != CANCEL) {
				if(rep == YES) rep = mMIDIoutputcheck(w);
				else {
					rep = Answer("Check MIDI input",'Y');
					if(rep == YES) rep = mMIDIinputcheck(w);
					}
				}
			if(rep != EXIT) rep = DONE;
			break;
		case dPlaySelection:
			rep = mPlaySelect(Nw);
			if(rep != EXIT) rep = DONE;
			break;
		case dExpandSelection:
			rep = mExpandSelection(Nw);
			if(rep != EXIT) rep = DONE;
			break;
		case dShowPeriods:
			rep = mShowPeriods(Nw);
			if(rep != EXIT) rep = DONE;
			break;
		case dLoadWeights:
			LoadWeights(); break;
		case dSaveWeights:
			SaveWeights(); break;
		case dLearnWeights:
			LearnWeights(); break;
		case dSetWeights:
			SetWeights(); break;
		case bResetControllers:
			ResetMIDIControllers(YES,YES,NO); break;
		case bMIDIpanic:
			if(!IsMidiDriverOn()) {
				Alert1("MIDI output is inactive (check the 'Devices' menu)");
				return(DONE);
				}
#if WITH_REAL_TIME_MIDI
			SwitchOn(NULL,wControlPannel,bMIDIpanic);
			if(Oms || NEWTIMER) {
				Mute++;
				AllNotesOffAllChannels();
				FlushDriver();
				Mute--; 
				}
			else {
				FlushDriver();
				AllNotesOffAllChannels();
				}
			Panic = TRUE;
			SwitchOff(NULL,wControlPannel,bMIDIpanic);
#endif
			return(DONE);
			break;
		case bCaptureSelection:
			rep = mCaptureSelection(Nw);
			if(rep != EXIT) rep = DONE;
			break;
		}
	return(DONE); /* Avoids reprocessing mouseclick event: */
	/* This window is special because you can select buttonsÉ */
	/* É even if it is not active. */
	}
if(w == wMetronom) {
	switch(itemHit) {
		UpdateDirty(TRUE,iSettings);
		case dSmooth:
			if(Nature_of_time == STRIATED) {
				Nature_of_time = SMOOTH;
				Dirty[wMetronom] = TRUE;
				SetGrammarTempo();
				AppendScript(43);
				}
			break;
		case dStriated:
			if(Nature_of_time == SMOOTH) {
				Nature_of_time = STRIATED;
				Dirty[wMetronom] = TRUE;
				SetGrammarTempo();
				AppendScript(44);
				}
			break;
		}
	if((rep=GetTempo()) == OK) {
		if(((rep=SetTempo()) != OK) || ((rep=SetTimeBase()) != OK)) return(rep);
		}
	else return(rep);
	}
if(w == wTimeBase) {
	if(GetDialogValues(w) != OK) return(DONE);
	i = -1;
	switch(itemHit) {
		case dNewTimeBase:
			if(SaveCheck(w) != OK) return(DONE);
			for(i=0; i < MAXTICKS; i++) {
				TickCycle[i] = 1;
				if(Quantization < 50) TickDuration[i] = 50;
				else  TickDuration[i] = Quantization;
				for(j=0; j < MAXBEATS; j++) ThisTick[i][j] = ZERO;
				ThisTick[0][0] = 1L;
				Ptick[i] = Qtick[i] = 1L;
				MuteTick[i] = FALSE;
				SetTickParameters(i+1,MAXBEATS);
				}
			SetTickParameters(0,MAXBEATS);
			SetField(NULL,wTimeBase,fTimeBaseComment,"[Comment on time base]");
			ForgetFileName(w);
			BPActivateWindow(SLOW,w);
			ResetTickFlag = TRUE;
			i = -1;
			break;
		case dLoadTimeBase:
			if((rep=mOpenFile(w)) != OK) return(rep);
			ResetTickFlag = TRUE;
			BPActivateWindow(SLOW,w);
			return(DONE);
		case dSaveAsTimeBase:
			Created[wTimeBase] = FALSE;	/* No break! */
		case dSaveTimeBase:
			if((rep=mSaveFile(w)) != OK) return(rep);
			return(DONE);
		case dCaptureTicks:
			if(!OutMIDI) {
				Alert1("Can't capture ticks because MIDI output is not active");
				return(DONE);
				}
#if WITH_REAL_TIME_MIDI
			rep = CaptureTicks();
			if(rep != EXIT) 
				rep = DONE;
#else
			rep = DONE;
#endif
			break;
		case dPlayTicks:
			if(!PlayTicks && !OutMIDI) {
				Alert1("Can't play ticks because MIDI output is not active");
				return(DONE);
				}
#if WITH_REAL_TIME_MIDI
			PlayTicks = 1 - PlayTicks;
			if(PlayTicks) AppendScript(178);
			else AppendScript(177);
#endif
			ResetTickFlag = TRUE;
			goto DONETIMEBASE;
		case dResetCycle:
			ResetTickFlag = TRUE;
			return(DONE);
		case dClearCycle:
			ClearCycle(0); i = 0;
			UpdateDirty(TRUE,w);
			goto DONETIMEBASE;
		case (dClearCycle+1):
			ClearCycle(1); i = 1;
			UpdateDirty(TRUE,w);
			goto DONETIMEBASE;
		case (dClearCycle+2):
			ClearCycle(2); i = 2;
			UpdateDirty(TRUE,w);
			goto DONETIMEBASE;
		case dRecordTick:
			RecordTick(0,0);
			UpdateDirty(TRUE,w);
			goto DONETIMEBASE;
		case (dRecordTick+55):
			RecordTick(FALSE,1);
			UpdateDirty(TRUE,w);
			goto DONETIMEBASE;
		case (dRecordTick+110):
			RecordTick(FALSE,2);
			UpdateDirty(TRUE,w);
			goto DONETIMEBASE;
		case dMute:
			MuteTick[0] = 1 - MuteTick[0];
			goto DONETIMEBASE;
		case (dMute+5):
			MuteTick[1] = 1 - MuteTick[1];
			goto DONETIMEBASE;
		case dMute+10:
			MuteTick[2] = 1 - MuteTick[2];
			goto DONETIMEBASE;
		}
	TickThere = changedtick = FALSE;
	for(i=0; i < MAXTICKS; i++) {
FINDBOX:
		for(j=0; j < TickCycle[i]; j++) {
			if(itemHit == dPlayBeat+55*i+j) {
				UpdateDirty(TRUE,w); changedtick = TRUE;
				if((p_event->modifiers & optionKey) == 0) {
					if(ThisTick[i][j] % 2L) ThisTick[i][j]--;
					else ThisTick[i][j]++;
					if(iTick == i && jTick == j) {
						SetThisTick();
						}
					SetTickParameters(i+1,0);
					}
				else {
					if(iTick >= 0 && jTick >= 0) {
						GetThisTick();
						jj = dPlayBeat + 55*iTick + jTick;
						GetDialogItem(gpDialogs[wTimeBase],(short)jj,&itemtype,(Handle*)&itemhandle,&r);
						if(ThisTick[iTick][jTick] < 2) HiliteControl(itemhandle,11);
						}
					iTick = i; jTick = j;
					SetThisTick();
					BPActivateWindow(SLOW,wTickDialog);
					return(DONE);
					}
				}
			if(ThisTick[i][j] % 2L) TickThere = TRUE;
			}
		for(j=TickCycle[i]; j < MAXBEATS; j++) {
			if(itemHit == dPlayBeat+55*i+j) {
				sprintf(line,"Increase tick cycle to %ld beats",(long)(j+1));
				if(Answer(line,'Y') == OK) {
					oldcycle = TickCycle[i];
					TickCycle[i] = j + 1;
					SetTickParameters(i+1,oldcycle);
					SetTickParameters(0,MAXBEATS);
					UpdateDirty(TRUE,wTimeBase);
					ResetTickFlag = TRUE;
					goto FINDBOX;
					}
				}
			}
		}
	if(changedtick) return(DONE);
	i = -1;
DONETIMEBASE:
	SetTickParameters(i+1,0);
	return(DONE);
	}
if(w == wBufferSize) {
	GetBufferSize();
	switch(itemHit) {
		case dNoSizeLimit:
			UseBufferLimit = FALSE;
			AppendScript(47);
			break;
		case dYesSizeLimit:
			UseBufferLimit = TRUE;
			AppendScript(46);
			break;
		}
	SetBufferSize();
	}
if(w == wGraphicSettings) {
	GetGraphicSettings();
	switch(itemHit) {
		case dZero:
			StartFromOne = FALSE;
			AppendScript(49);
			break;
		case dOne:
			StartFromOne = TRUE;
			AppendScript(50);
			break;
		}
	SetGraphicSettings();
	}
if(w == wFindReplace) {
	GetFindReplace();
	switch(itemHit) {
		case dFind:
			HideWindow(GetDialogWindow(gpDialogs[wFindReplace]));
			FindReplace(FALSE);
			return(DONE);
			break;
		case dCancel:
			HideWindow(GetDialogWindow(gpDialogs[wFindReplace]));
			BPActivateWindow(SLOW,TargetWindow);
			return(DONE);
			break;
		case dIgnoreCase:
			IgnoreCase = 1 - IgnoreCase;
			break;
		case dMatchWords:
			MatchWords = 1 - MatchWords;
			break;
		case dReplaceAll:
			HideWindow(GetDialogWindow(gpDialogs[wFindReplace]));
			FindReplace(TRUE);
			return(DONE);
			break;
		}
	SetFindReplace();
	}
if(w == wTimeAccuracy) {
	if(GetDialogValues(w) != OK) return(DONE);
	UpdateDirty(TRUE,iSettings);
	switch(itemHit) {
		case dOff:
			QuantizeOK = FixedMaxQuantization = AskedTempMemory = FALSE;
			AppendScript(72);
			NotSaidKpress = TRUE;
			break;
		case dOn:
			QuantizeOK = TRUE;
			FixedMaxQuantization = AskedTempMemory = FALSE;
			AppendScript(52);
			break;
		}
	SetTimeAccuracy();
	}
if(w == wRandomSequence) {
	if(GetDialogValues(w) != OK) return(DONE);
	switch(itemHit) {
		case dReset:
			ResetRandom();
			break;
		case dNewSeed:
			Randomize();
			break;
		}
	}
if(w == wKeyboard) {
	if(GetDialogValues(w) != OK) return(DONE);
	switch(itemHit) {
		case dLoad:
			if((rep=mOpenFile(w)) != OK) return(rep);
			BPActivateWindow(SLOW,w);
			break;
		case dSave:
			mSaveFile(w);
			break;
		case dOK:
			HideWindow(Window[w]);
			// BPActivateWindow(SLOW,LastEditWindow); // suppressed 060107 akozar
			break;
		case dToken:
			Token = 1 - Token;
			if(Token) AppendScript(66);
			else AppendScript(67);
			SetKeyboard();
			break;
		case dResetkeys:
			AppendScript(53);
			ResetKeyboard(NO); break;
		}
	rep = DONE;
	}
if(w == wScriptDialog) {
	StopCount(0);
	rep = DONE;
	switch(itemHit) {
		case bLoadScript:
			EndWriteScript();
			if((rep=mOpenFile(wScript)) != OK) return(rep);
			BPActivateWindow(SLOW,wScript);
			break;
		case bSaveScript:
			rep = mSaveFile(wScript);
			if(rep != EXIT) rep = DONE;
			break;
		case bClearScript:
			ClearWindow(FALSE,wScript);
			ForgetFileName(wScript);
			break;
		case bWaitForSyncScript:
			WaitForSyncScript();
			break;
		case bRecordScript:
			ScriptRecOn = 1 - ScriptRecOn;
			if(ScriptRecOn) InitWriteScript();
			else EndWriteScript();
			break;
		case bExecScript:
			rep = mExecuteScript(wScript);
			if(rep != EXIT) rep = DONE;
			break;
		case bCheckScriptSyntax:
			rep = mCheckScript(w);
			if(rep != EXIT) rep = DONE;
			break;
		}
	}
if(thedialog == ResumeStopPtr) {
	switch(itemHit) {
		case dResume:
			return(mResume(0));
		case dStop:
			return(mStop(0));
		}
	}
if(thedialog == ResumeUndoStopPtr) {
	switch(itemHit) {
		case dResume:
			return(mResume(0));
		case dStop:
			return(mStop(0));
		case dUndo:
			return(mUndo(0));
		}
	}
if(w == wMIDIorchestra) {
	rep = DONE;
	ClearMessage();
	switch(itemHit) {
		case bMIDIorchestraSave:
			if((rep=mSaveAs(w)) != OK) return(rep);
			break;
		case bMIDIorchestraLoad:
			if((rep=mOpenFile(w)) != OK) return(rep);
			break;
		case bMIDIorchestraNew:
			if(Answer("Clear all instruments",'N') == OK) {
				ClearWindow(NO,wMIDIorchestra);
				ForgetFileName(wMIDIorchestra);
				NewOrchestra = TRUE;
				}
			break;
		}
	return(DONE);
	}
if(thedialog == MIDIprogramPtr) {
	rep = DONE;
	ClearMessage();
	if(GetField(MIDIprogramPtr,TRUE,-1,fMIDIchannel,line,&p,&q) == OK) {
		if(p/q < 1 || p/q > MAXCHAN) {
			sprintf(Message,
				"MIDI channel range is 1..%ld. Can't accept %ld",(long)MAXCHAN,(long)(p/q));
			Alert1(Message);
			SetField(MIDIprogramPtr,-1,fMIDIchannel,"[?]");
			SelectField(MIDIprogramPtr,-1,fMIDIchannel,TRUE);
			}
		else TestMIDIChannel = p / q;
		}
	switch(itemHit) {
		case bMIDIprogramHelp:
			Alert1("If you run an old version of QuickTime, some instruments may not yet be implemented");
			break;
		case bMIDIprogramAllNotesOff:
			AllNotesOffAllChannels();
/*			Tcurr = ZERO;
			ResetMIDI(FALSE); */
			break;
		case bShowMIDIorchestra:
			BPActivateWindow(SLOW,wMIDIorchestra); // was mMIDIorchestra(Nw); - akozar 060707
			break;
		case bMIDItest:
			CheckMIDIOutPut(TestMIDIChannel - 1);
			break;
		case bMIDIprogramList:
			ClearWindow(NO,wNotice);
			Println(wNotice,"List of all instruments (General MIDI)\n");
			for(i=0; i < 128; i++) {
				sprintf(line,"[%ld] %s",(long)(*p_GeneralMIDIpatchNdx)[i],
					*((*p_GeneralMIDIpatch)[i]));
				Println(wNotice,line);
				}
			SetSelect(ZERO,ZERO,TEH[wNotice]);
			ShowSelect(UP,wNotice);
			break;
		default:
			for(i=1; i <= 128; i++) {
				if(itemHit == i) {
					if(TestMIDIChannel < 2) {
						TestMIDIChannel = 1;
						GetDialogItem(SixteenPtr, (short)button1 + TestMIDIChannel - 1, &itemtype,
									(Handle*)&itemhandle, &r);
						if(itemhandle != NULL) HiliteControl((ControlHandle) itemhandle,kControlButtonPart);
						sprintf(line,"%ld",(long)TestMIDIChannel);
						SetField(MIDIprogramPtr,-1,fMIDIchannel,line);
						}
					if(CurrentMIDIprogram[TestMIDIChannel] == i) changed = FALSE;
					else changed = TRUE;
					e.time = Tcurr;
					e.type = TWO_BYTE_EVENT;
					e.status = ProgramChange + TestMIDIChannel - 1;
					e.data2 = i - 1;
					SendToDriver(Tcurr * Time_res,0,&rs,&e);
					ChangedMIDIprogram = TRUE;
					WaitABit(500L);	/* This is necessary notably if sending a program change */
					GetDialogItem(MIDIprogramPtr, (short)i, &itemtype, (Handle*)&itemhandle, &r);
					if(itemhandle != NULL) HiliteControl((ControlHandle) itemhandle,kControlButtonPart);
					if(!changed) {
						CheckMIDIOutPut(TestMIDIChannel-1);
						}
					else {
						if(CurrentMIDIprogram[TestMIDIChannel] > 0) {
							GetDialogItem(MIDIprogramPtr, (short)CurrentMIDIprogram[TestMIDIChannel],
										&itemtype, (Handle*)&itemhandle, &r);
							if(itemhandle != NULL) HiliteControl((ControlHandle) itemhandle,0);
							WritePatchName();
							}
						CurrentMIDIprogram[TestMIDIChannel] = i; /* useless, already done by SendToDriver() */
						for(j=0; j < 128; j++) {
							if((*p_GeneralMIDIpatchNdx)[j] == i) {
								sprintf(line,"[%ld] %s",(long)i,*((*p_GeneralMIDIpatch)[j]));
								SetField(MIDIprogramPtr,-1,fPatchName,line);
								SetField(NULL,wMIDIorchestra,TestMIDIChannel,line);
								NewOrchestra = TRUE;
								UpdateDirty(TRUE,wMIDIorchestra);
								break;
								}
							}
						ShowMessage(TRUE,wMessage,"Click again this button to hear a sound sample...");
						}
					break;
					}
				}
			break;
		}
	return(DONE);
	}
if(thedialog == SixteenPtr) {
	rep = DONE;
	ClearMessage();
	HideWindow(Window[wInfo]);
	switch(itemHit) {
		case bChangePatch:
			ShowWindow(GetDialogWindow(MIDIprogramPtr));
			SelectWindow(GetDialogWindow(MIDIprogramPtr));
			BPUpdateDialog(MIDIprogramPtr); /* Needed to make static text visible */
			break;			
		case bOKSixteen:
			HideWindow(GetDialogWindow(SixteenPtr));
			HideWindow(GetDialogWindow(MIDIprogramPtr));
//			Alert1("To check input, select the 'Data' window and type cmd-J (\"type from MIDI\")");
			break;
		default:
			for(i=0; i < MAXCHAN; i++) {
				if(itemHit == (button1 + i)) {
					if(TestMIDIChannel != (i + 1)) {
						if(CurrentMIDIprogram[TestMIDIChannel] > 0) {
							GetDialogItem(MIDIprogramPtr, (short)CurrentMIDIprogram[TestMIDIChannel],
									&itemtype, (Handle*)&itemhandle, &r);
							if(itemhandle != NULL) HiliteControl((ControlHandle) itemhandle,0);
							}
					
						GetDialogItem(SixteenPtr, (short)button1 + TestMIDIChannel - 1, &itemtype,
									(Handle*)&itemhandle, &r);
						if(itemhandle != NULL) HiliteControl((ControlHandle) itemhandle,0);
						}
					TestMIDIChannel = i + 1;
					
					if(CurrentMIDIprogram[TestMIDIChannel] > 0) {
						GetDialogItem(MIDIprogramPtr, (short)CurrentMIDIprogram[TestMIDIChannel],
									&itemtype, (Handle*)&itemhandle, &r);
						if(itemhandle != NULL) HiliteControl((ControlHandle) itemhandle,kControlButtonPart);
						WritePatchName();
						}
					
					GetDialogItem(SixteenPtr, (short)itemHit, &itemtype, (Handle*)&itemhandle, &r);
					if(itemhandle != NULL) HiliteControl((ControlHandle) itemhandle,kControlButtonPart);
					
					sprintf(line,"%ld",(long)TestMIDIChannel);
					SetField(MIDIprogramPtr,-1,fMIDIchannel,line);
#if TARGET_API_MAC_CARBON
					QDFlushPortBuffer(GetDialogPort(SixteenPtr), NULL);
					QDFlushPortBuffer(GetDialogPort(MIDIprogramPtr), NULL);
#endif
					
					CheckMIDIOutPut(TestMIDIChannel-1);
					
					break;
					}
				}
			break;
		}
	return(DONE);
	}
if(thedialog == MIDIkeyboardPtr) {
	switch(itemHit) {
		case dOKkeyboard:
			HideWindow(GetDialogWindow(MIDIkeyboardPtr));
			break;
		default:
			if(itemHit > 24) return(OK);
			if(itemHit < 13)
				NameChoice[itemHit - 1] = 0;
			else
				NameChoice[itemHit - 13] = 1;
			Dirty[iSettings] = TRUE;
			break;
		}
	SetNameChoice();
	rep = DONE; 
	}
if(thedialog == StrikeModePtr) {
	switch(itemHit) {
		case bOKSaveStrikeMode:
			HideWindow(GetDialogWindow(StrikeModePtr));
			break;
		case bDefaultStrikeAgain:
			StrikeAgainDefault = TRUE;
			Dirty[iSettings] = TRUE;
			break;
		case bDefaultDontStrikeAgain:
			StrikeAgainDefault = FALSE;
			Dirty[iSettings] = TRUE;
			break;
		}
	SetDefaultStrikeMode();
	return(DONE);
	}
if(thedialog == FileSavePreferencesPtr) {
	switch(itemHit) {
		case bOKFileSavePreferences:
			if(GetFileSavePreferences() == OK)
				HideWindow(GetDialogWindow(FileSavePreferencesPtr));
			return(DONE);
			break;
		case bSaveAllToSame:
			FileSaveMode = ALLSAME;
			if(OutCsound) GetCsoundScoreName();
			if(WriteMIDIfile) GetMIDIfileName();
			break;
		case bSaveAllToSameWithPrompt:
			FileSaveMode = ALLSAMEPROMPT;
			if(OutCsound) GetCsoundScoreName();
			if(WriteMIDIfile) GetMIDIfileName();
			break;
		case bEachToNew:
			FileSaveMode = NEWFILE;
			break;
		case bChangeCsFile:
			CloseCsScore();
			PrepareCsFile();
			break;
		case bChangeMIDIfile:
			CloseMIDIFile();
			PrepareMIDIFile();
			break;
		case bWriteNow:
			FileWriteMode = NOW;
			break;
		case bWriteLater:
			FileWriteMode = LATER;
			if(!OutMIDI) {
				OutMIDI = TRUE;	// FIXME ? probably should check driver availability first! - akozar
				SetButtons(TRUE);
				}
			break;
		case bType0:
			CloseMIDIFile();
			MIDIfileType = 0;
			break;
		case bType1:
			CloseMIDIFile();
			MIDIfileType = 1;
			break;
		case bType2:
			CloseMIDIFile();
			MIDIfileType = 2;
			break;
		case bTypeMac:
			CloseCsScore();
			CsoundFileFormat = MAC;
			break;
		case bTypeDos:
			CloseCsScore();
			CsoundFileFormat = DOS;
			break;
		case bTypeUnix:
			CloseCsScore();
			CsoundFileFormat = UNIX;
			break;
		}
	SetFileSavePreferences();
	rep = DONE; 
	}
if(thedialog == TuningPtr) {
	if(GetTuning() != OK) return(DONE);
	switch(itemHit) {
		case bOKtuning:
			HideWindow(GetDialogWindow(TuningPtr));
			return(DONE);
			break;
		}
	SetTuning();
	rep = DONE; 
	}
if(thedialog == DefaultPerformanceValuesPtr) {
	if(GetDefaultPerformanceValues() != OK) return(DONE);
	SetDefaultPerformanceValues();
	rep = DONE; 
	}
if(thedialog == CsoundInstrMorePtr) {
	if(GetDialogValues(wCsoundInstruments) != OK) return(DONE);
	doneit = FALSE;
	if(iCsoundInstrument >= 0 && iCsoundInstrument < Jinstr) {
		if((*p_CsInstrument)[iCsoundInstrument].paramlist != NULL) {
			for(ip=0; ip < (*p_CsInstrument)[iCsoundInstrument].ipmax; ip++) {
				if(itemHit == (bMULTval + (2*ip))) {
					if(fabs((*((*p_CsInstrument)[iCsoundInstrument].paramlist))[ip].defaultvalue) < 0.01) {
						Alert1("Can't set parameter to multiplicative mode because its default value is close to zero");
						return(DONE);
						}
					(*((*p_CsInstrument)[iCsoundInstrument].paramlist))[ip].combinationtype
						= MULT;
					UpdateDirty(TRUE,wCsoundInstruments);
					doneit = TRUE;
					}
				if(itemHit == (bADDval + (2*ip))) {
					(*((*p_CsInstrument)[iCsoundInstrument].paramlist))[ip].combinationtype
						= ADD;
					UpdateDirty(TRUE,wCsoundInstruments);
					doneit = TRUE;
					}
				}
			}
		}
	switch(itemHit) {
		default:
			break;
		}
	if(doneit) SetCsoundMoreParametersWindow(iCsoundInstrument,-1);
	return(DONE);
	}
if(w == wPrototype1) {
	Nw = w;
	switch(itemHit) {
		case bOKprototype1:
			if(SaveCheck(w) != OK) break;
			if(GetPrototype(TRUE) != OK) return(DONE);
			HideWindow(Window[wPrototype1]);
			HideWindow(Window[wPrototype2]);
			HideWindow(Window[wPrototype3]);
			HideWindow(Window[wPrototype4]);
			HideWindow(Window[wPrototype5]);
			HideWindow(Window[wPrototype6]);
			HideWindow(Window[wPrototype7]);
			HideWindow(Window[wPrototype8]);
			ClearWindow(FALSE,wGraphic);
			HideWindow(Window[wGraphic]);
			return(DONE);
			break;
		case bShowPianoRoll:
		case bPlayObjectAlone:
			if(CheckiProto() != OK) return(DONE);
			if(GetPrototype(TRUE) != OK) return(DONE);
			if((*p_MIDIsize)[iProto] == ZERO) {
				Alert1("This sound-object contains no MIDI message");
				return(DONE);
				}
			if(!((*p_Type)[iProto] & 1)) {
				Alert1("Can't play this sound-object unless 'MIDI sequence' is checked");
				return(DONE);
				}
			pclock = (unsigned long) Pclock; qclock = (unsigned long) Qclock;
			if(GetField(NULL,TRUE,wPrototype1,fMetronomTry,line,&p,&q) != OK) {
				Alert1("Metronom value is not specified");
				SetField(NULL,wPrototype1,fMetronomTry,"[?]");
				SelectField(NULL,wPrototype1,fMetronomTry,TRUE);
				return(DONE);
				}
			if(Simplify((double)INT_MAX,(double)p,(double)60L*q,&Qclock,&Pclock) != OK)
				Simplify((double)INT_MAX,floor((double)p/60.),(double)q,&Qclock,&Pclock);
			SetTempo();
			if((GetCtrlValue(wPrototype1,bWithParameters) > 0)) {
				if(GetField(NULL,TRUE,wPrototype1,fDilationRatio,line,&p,&q) != OK) {
					Alert1("Dilation ratio is not specified");
					SetField(NULL,wPrototype1,fDilationRatio,"[?]");
					SelectField(NULL,wPrototype1,fDilationRatio,TRUE);
					return(DONE);
					}
				ForceRatio = ((double) p) / q / 100.;
				if(ForceRatio < 0.) {
					Alert1("Incorrect dilation ratio. Will be forced to 100%");
					ForceRatio = 1.;
					}
				}
			else ForceRatio = 1.;
			showgraphic = ShowGraphic; ShowGraphic = FALSE;
			showmessages = ShowMessages; ShowMessages = FALSE;
			displaytimeset = DisplayTimeSet; DisplayTimeSet = FALSE;
			nature_time = Nature_of_time; SetButtons(TRUE);
			Nature_of_time = (GetCtrlValue(wPrototype1,bStriatedTimeTry) > 0);
			MystrcpyTableToString(MAXFIELDCONTENT,line,p_Bol,iProto);
			DefaultVolume = 127;
			if((h = (char**) GiveSpace((Size)((1+strlen(line)) * sizeof(char)))) == NULL)
				return(ABORT);
			if(MystrcpyStringToHandle(&h,line) != OK) return(ABORT);
			NoRepeat = TRUE;
			PlayPrototypeOn = TRUE;
			rep = PlayHandle(h,(int)(itemHit == bShowPianoRoll));
			PlayPrototypeOn = FALSE;
			if(rep != EXIT && rep != ABORT) rep = DONE;
			NoRepeat = FALSE;
			MyDisposeHandle((Handle*)&h);
			if(rep != DONE) return(rep);
			DefaultVolume = DeftVolume;
			ShowMessages = showmessages;
			DisplayTimeSet = displaytimeset;
			ForceRatio = -1.;
			ShowGraphic = showgraphic;
			SetButtons(TRUE);
			Pclock = (double)pclock;
			Qclock = (double)qclock;
			Nature_of_time = nature_time;
			SetTempo();
			break;
		case bPlayExpression:
			if(CheckiProto() != OK) return(DONE);
			if(GetPrototype(TRUE) != OK) return(DONE);
			pclock = (unsigned long) Pclock; qclock = (unsigned long) Qclock;
			if(GetField(NULL,TRUE,wPrototype1,fMetronomTry,line,&p,&q) != OK) {
				Alert1("Metronom value is not specified");
				SetField(NULL,wPrototype1,fMetronomTry,"[?]");
				SelectField(NULL,wPrototype1,fMetronomTry,TRUE);
				return(DONE);
				}
			if(Simplify((double)INT_MAX,(double)p,(double)60L*q,&Qclock,&Pclock) != OK)
				Simplify((double)INT_MAX,floor((double)p/60L),(double)q,&Qclock,&Pclock);
			SetTempo();
			nature_time = Nature_of_time;
			showgraphic = ShowGraphic;
			ShowGraphic = (GetCtrlValue(wPrototype1,bShowGraphicTry) > 0);
			noconstraint = NoConstraint;
			NoConstraint = (GetCtrlValue(wPrototype1,bIgnorePropertiesTry) > 0);
			SetButtons(TRUE);
			GetField(NULL,FALSE,wPrototype1,fExpression,line,&p,&q);
			if((h = (char**) GiveSpace((Size)((1+strlen(line)) * sizeof(char)))) == NULL)
				return(ABORT);
			if(MystrcpyStringToHandle(&h,line) != OK) return(ABORT);
			NoRepeat = TRUE;
			rep = PlayHandle(h,NO);
			if(rep != EXIT && rep != ABORT) rep = DONE;
			NoRepeat = FALSE;
			MyDisposeHandle((Handle*)&h);
			if(rep != DONE) return(rep);
			NoConstraint = noconstraint;
			ShowGraphic = showgraphic; SetButtons(TRUE);
			Pclock = (double)pclock;
			Qclock = (double)qclock;
			Nature_of_time = nature_time;
			SetTempo();
			break;
		
		// Assign object type
		case bMIDIsequence:
			if(CheckiProto() != OK) return(DONE);
			if((*p_Type)[iProto] & 1) {
				if(Answer("MIDI messages in this sound-object will be ignored. Is it that you want",
						'N') == YES)
					(*p_Type)[iProto] &= (255-1);
				else return(DONE);
				}
			else {
				if((*p_MIDIsize)[iProto] == ZERO) {
					Alert1("This sound-object does not contain MIDI messages");
					return(DONE);
					}
				(*p_Type)[iProto] |= 1;
				}
			UpdateDirty(TRUE,wPrototype1);
			break;
		case bSampledSound:
			if(CheckiProto() != OK) return(DONE);
			if((*p_Type)[iProto] & 2) (*p_Type)[iProto] &= (255-2);
			else // (*p_Type)[iProto] |= 2;	// comment out until implemented - akozar 050707
				Alert1("This type of object is not yet supported...");
			// UpdateDirty(TRUE,wPrototype1);
			break;
		case bCsoundInstrument:
			if(CheckiProto() != OK) return(DONE);
			if((*p_Type)[iProto] & 4) {
				if((*p_CsoundSize)[iProto] > ZERO) {
					Alert1("This sound-object contains a Csound score and must remain declared as 'Csound'");
					return(DONE);
					}
				(*p_Type)[iProto] &= (255-4);
				}	
			else {
				if((*p_CsoundSize)[iProto] <= ZERO) {
					Alert1("This sound-object does not contain a Csound score. Click 'Csound' to create it");
					return(DONE);
					}
				(*p_Type)[iProto] |= 4;
				}
			UpdateDirty(TRUE,wPrototype1);
			break;
			
		case bResetPrototype:
			if(CheckiProto() != OK) return(DONE);
			sprintf(Message,"Reset prototype '%s'",*((*p_Bol)[iProto]));
			if(Answer(Message,'N') != OK) return(DONE);
			if(ResetPrototype(iProto) != OK) return(ABORT);
			Hpos = -1;
			SwitchOn(NULL,wPrototype5,bPlayPrototypeTicks);
			UpdateDirty(TRUE,wPrototype1);
			break;
		case bCopyFrom1:
			if(CheckiProto() != OK) return(DONE);
			CopyFrom(w);
			UpdateDirty(TRUE,wPrototype1);
			if(GetPrototype(YES) != OK) return(DONE);
			if(CompileObjectScore(iProto,&longerCsound) != OK) return(DONE);
			StopWait();
			break;
			
		case bEditRecord:
			if(CheckiProto() != OK) return(DONE);
			if(GetPrototype(TRUE) != OK) return(DONE);
			if(CompileCsoundObjects() != OK) return(DONE);
			BPActivateWindow(QUICK,wPrototype5);
			return(DONE);
			break;
		case bChangeInstrumentFile:
			if(CheckiProto() != OK) return(DONE);
			if(ClearWindow(FALSE,wCsoundInstruments) != OK) return(DONE);
			ForgetFileName(wCsoundInstruments);
			if((rep=mOpenFile(wCsoundInstruments)) != OK) return(rep);
			CompileCsoundObjects();
			SetPrototypePage1(iProto);
			UpdateDirty(TRUE,wPrototype1);
			return(DONE);
			break;
		case bConvertToCsound:
			if(CheckiProto() != OK) return(DONE);
			if(GetPrototype(TRUE) != OK) return(DONE);
			if((*p_MIDIsize)[iProto] == ZERO) {
				Alert1("This sound-object contains no MIDI message");
				return(DONE);
				}
			if(!((*p_Type)[iProto] & 1)) {
				Alert1("Can't convert this sound-object unless 'MIDI sequence' is checked");
				return(DONE);
				}
			if(!IsEmpty(wPrototype7))
				rep = Answer("Replace current Csound score for this object",'N');
			if(rep != YES) return(DONE);
			ClearWindow(NO,wPrototype7);
			GetCsoundScore(iProto);
			ConvertMIDItoCsound = TRUE;
			rep = PlayPrototype(iProto);
			UpdateDirty(TRUE,wPrototype1);
			ConvertMIDItoCsound = FALSE;
			(*p_CompiledCsoundScore)[iProto] = FALSE;
			CompileObjectScore(iProto,&longerCsound);
			break;
		case bStriatedTimeTry:
			if(CheckiProto() != OK) return(DONE);
			ToggleButton(w,bStriatedTimeTry);
			return(DONE);
			break;
		case bShowGraphicTry:
			if(CheckiProto() != OK) return(DONE);
			ToggleButton(w,bShowGraphicTry);
			return(DONE);
			break;
		case bIgnorePropertiesTry:
			if(CheckiProto() != OK) return(DONE);
			ToggleButton(w,bIgnorePropertiesTry);
			return(DONE);
			break;
		case bWithParameters:
			if(CheckiProto() != OK) return(DONE);
			ToggleButton(w,bWithParameters);
			return(DONE);
			break;
		
		// Open other dialogs	
		case bChannelInstrument:
			BPActivateWindow(SLOW,wPrototype8);
			return(DONE);
			break;
		case bCsoundWindow:
			if(CheckiProto() != OK) return(DONE);
			SetCsoundScore(iProto);
			BPActivateWindow(SLOW,wPrototype7);
			OutlineTextInDialog(wPrototype7,TRUE);
			return(DONE);
			break;
		case bDurationPivot:
			BPActivateWindow(SLOW,wPrototype2);
			return(DONE);
			break;
		case bCoverTruncate:
			BPActivateWindow(SLOW,wPrototype3);
			return(DONE);
			break;
		case bContinuityPreRoll:
			BPActivateWindow(SLOW,wPrototype4);
			return(DONE);
			break;
		case bPeriodCyclicObject:
			BPActivateWindow(SLOW,wPrototype6);
			return(DONE);
			break;
			
		// Go to another prototype
		case bLeftArrowPrototype:
			if(CheckiProto() != OK) return(DONE);
			if(GetPrototype(TRUE) != OK) return(DONE);
			w = Nw;
			if(Jbol > 2) {
				if(CompileCsoundObjects() != OK) return(DONE);
				i = iProto;
				iProto--; if(iProto < 2 || iProto >= Jbol) iProto = Jbol - 1;
				if(SetPrototype(iProto) != OK) iProto = i;
				SetCsoundScore(iProto);
				}
			Hpos = -1;
			if(PrototypeWindow(w)) BPActivateWindow(QUICK,w);
			return(DONE);
			break;
		case bRightArrowPrototype:
			if(CheckiProto() != OK) return(DONE);
			if(GetPrototype(TRUE) != OK) return(DONE);
			w = Nw;
			if(Jbol > 2) {
				if(CompileCsoundObjects() != OK) return(DONE);
				i = iProto;
				iProto++; if(iProto >= Jbol || iProto < 2) iProto = 2;
				if(SetPrototype(iProto) != OK) iProto = i;
				SetCsoundScore(iProto);
				}
			Hpos = -1;
			if(PrototypeWindow(w)) BPActivateWindow(QUICK,w);
			return(DONE);
			break;
		case bGoToPrototype:
			if(CheckiProto() != OK) return(DONE);
			if(GetPrototype(TRUE) != OK) return(DONE);
			w = Nw;
			if(CompileCsoundObjects() != OK) return(DONE);
			i = iProto;
			ShowObjects(w);
			if(SetPrototype(iProto) != OK) iProto = i;
			SetCsoundScore(iProto);
			if(PrototypeWindow(w)) BPActivateWindow(QUICK,w);
			Hpos = -1;
			return(DONE);
			break;
		
		case bLoadPrototypeFile:
			Hpos = -1;
			mOpenFile(iObjects);
			break;
		case bSavePrototypeFile:
			if(CheckiProto() != OK) return(DONE);
			if(GetPrototype(TRUE) != OK) return(DONE);
			Created[iObjects] = FALSE;
			mSaveFile(iObjects);
			DrawPrototype(iProto,wPrototype1,&PictFrame);
			break;
		
		case bPasteSelection:
			if(CheckiProto() != OK) return(DONE);
			if((rep=PasteStreamToPrototype(iProto, bAskPasteAction)) != OK) return(rep);
			else rep = DONE;
			break;
		case bUndoPasteSelection:
			if(CheckiProto() != OK) return(DONE);
			rep = UndoPasteSelection(iProto);
			if(rep == OK) rep = DONE;
			break;
		case bCapture:
			if(CheckiProto() != OK) return(DONE);
			if(GetPrototype(TRUE) != OK) return(DONE);
			rep = CaptureCodes(iProto);
			if(rep == OK) rep = DONE;
			break;
		case bImportMIDIfile:
IMPORT:
			if(CheckiProto() != OK) return(DONE);
			if(GetPrototype(TRUE) != OK) return(DONE);
			sprintf(Message,"Discard MIDI stream of prototype '%s'",*((*p_Bol)[iProto]));
			if((*pp_MIDIcode)[iProto] != NULL && Answer(Message,'Y') != OK) return(DONE);
			rep = ImportMIDIfile(iProto);
			if(rep == OK) rep = DONE;
			break;
			
		case pPicture:
			if(iProto < 2 || iProto >= Jbol) return(DONE);
			if(GetPrototype(TRUE) != OK) return(DONE);
			SelectPictureOn = TRUE;
			w = Nw;
			point = p_event->where;
			Hpos = point.h;
			(*p_Tpict)[iProto] = Infneg;
			if(DrawPrototype(iProto,wPrototype1,&PictFrame) != OK) return(DONE);
			if(w != wPrototype1) BPActivateWindow(QUICK,w);
			return(DONE);
			break;
			
		case bCreateObject:
			if(GetPrototype(TRUE) != OK) return(DONE);
			if(AnswerWith("Name the new sound-object","",line) == ABORT)
				return(DONE);
			Strip(line);
			if(line[0] == '\0') return(DONE);
			if((h = (char**) GiveSpace((Size)((1+strlen(line)) * sizeof(char)))) == NULL)
				return(ABORT);
			if(MystrcpyStringToHandle(&h,line) != OK) return(ABORT);
			diff = TRUE;
			for(j=0; j < Jbol; j++) {
				if((diff = MyHandlecmp((*p_Bol)[j],h)) == 0) break;
				}
			if(!diff) {
				Alert1("This name is already assigned to a sound-object. Use the 'Goto...' button");
				goto EXITNAME;
				}
			Jbol++;
			if(CheckTerminalSpace() != OK) return(ABORT);
			Jbol--;
			iProto = CreateBol(FALSE,TRUE,h,FALSE,FALSE,BOL);
			Jbol--;
			if((rep=ResizeObjectSpace(NO,Jbol + 1 + Jpatt,1)) != OK) return(rep);
			(*pp_MIDIcode)[Jbol] = NULL;
			(*pp_CsoundTime)[Jbol] = NULL;
			(*pp_Comment)[Jbol] = (*pp_CsoundScoreText)[Jbol] = NULL;
			(*pp_CsoundScore)[Jbol] = NULL;
			SetCsoundScore(Jbol);
			Jbol++;
			SelectBehind(GetTextLength(wAlphabet),GetTextLength(wAlphabet),
																		TEH[wAlphabet]);
			sprintf(Message,"\n%s",line);
			PrintBehind(wAlphabet,Message);
			CompiledAl = TRUE;  Dirty[wAlphabet] = FALSE;
			CompiledAl = CompiledGr = NeedAlphabet = TRUE;
			ObjectMode = ObjectTry = TRUE;
			if(ResetPrototype(iProto) != OK) return(ABORT);
			SwitchOn(NULL,wPrototype5,bPlayPrototypeTicks);
			UpdateDirty(TRUE,wPrototype1);
EXITNAME:
			MyDisposeHandle((Handle*)&h);
			break;
		}
	SetPrototype(iProto);
	/* DrawPrototype(iProto,wPrototype1,&PictFrame); */
	/* SetCsoundScore(iProto) would slow down */
	rep = DONE;
	}
if(w == wPrototype2) {
	if(GetDialogValues(w) != OK) return(DONE);
	if(itemHit != bOKprototype2) {
		if(CheckiProto() != OK) return(DONE);
		UpdateDirty(TRUE,w);
		}
	switch(itemHit) {
		case bOKprototype2:
			if(GetPrototype(YES) != OK) return(DONE);
			HideWindow(Window[wPrototype2]);
			BPActivateWindow(QUICK,wPrototype1);
	/*		return(DONE); */
			break;
		case bOKrescale:
			(*p_FixScale)[iProto] = FALSE; (*p_OkExpand)[iProto]
				= (*p_OkCompress)[iProto] = TRUE;
			break;
		case bNeverRescale:
			(*p_FixScale)[iProto] = TRUE; (*p_OkExpand)[iProto]
				= (*p_OkCompress)[iProto] = FALSE;
			break;
		case bDilationRatioRange:
			if(GetField(NULL,TRUE,w,fMinDilationRatio,line,&p,&q) == OK)
				(*p_AlphaMin)[iProto] = ((float)p)/q;
			else {
				Alert1("Minimum duration ratio is not specified");
				SetField(NULL,w,fMinDilationRatio,"[?]");
				SelectField(NULL,w,fMinDilationRatio,TRUE);
				return(DONE);
				}
			if(GetField(NULL,TRUE,w,fMaxDilationRatio,line,&p,&q) == OK)
				(*p_AlphaMax)[iProto] = ((float)p)/q;
			else {
				Alert1("Maximum duration ratio is not specified");
				SetField(NULL,w,fMaxDilationRatio,"[?]");
				SelectField(NULL,w,fMinDilationRatio,TRUE);
				return(DONE);
				}
			(*p_FixScale)[iProto] = (*p_OkExpand)[iProto] = (*p_OkCompress)[iProto]
					= FALSE;
			break;
		case bExpandAtWill:
			if(!(*p_OkExpand)[iProto]) {
				(*p_FixScale)[iProto] = FALSE;
				(*p_OkExpand)[iProto] = TRUE;
				}
			else (*p_OkExpand)[iProto] = FALSE;
			break;
		case bCompressAtWill:
			if(!(*p_OkCompress)[iProto]) {
				(*p_FixScale)[iProto] = FALSE;
				(*p_OkCompress)[iProto] = TRUE;
				}
			else (*p_OkCompress)[iProto] = FALSE;
			break;
		case bSendAlpha:
			(*p_AlphaCtrl)[iProto] = 1 - (*p_AlphaCtrl)[iProto];
			break;
		case bPivBeg:
			(*p_PivType)[iProto] = 1;
			break;
		case bPivMiddle:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_PivType)[iProto] = 5;
			break;
		case bPivEnd:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_PivType)[iProto] = 2;
			break;
		case bPivFirstNoteOn:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_PivType)[iProto] = 3;
			break;
		case bPivMiddleNoteOnOff:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_PivType)[iProto] = 6;
			break;
		case bPivLastNoteOff:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_PivType)[iProto] = 4;
			break;
		case bSetPivotms:
			if(CheckDuration(iProto) != OK) return(DONE);
			GetField(NULL,TRUE,w,fSetPivotms,line,&p,&q);
			(*p_PivPos)[iProto] = ((float)p)/q;
			(*p_PivType)[iProto] = 7;
			(*p_PivMode)[iProto] = ABSOLUTE;
			break;
		case bSetPivotPC:
			if(CheckDuration(iProto) != OK) return(DONE);
			GetField(NULL,TRUE,w,fSetPivotPC,line,&p,&q);
			(*p_PivPos)[iProto] = ((float)p)/q;
			(*p_PivType)[iProto] = 7;
			(*p_PivMode)[iProto] = RELATIVE;
			break;
		case bNeverRelocate:
			(*p_MaxDelay)[iProto] = (*p_MaxForward)[iProto] = ZERO;
			(*p_DelayMode)[iProto] = (*p_ForwardMode)[iProto] = RELATIVE;
			(*p_OkRelocate)[iProto] = FALSE;
			break;
		case bRelocateAtWill:
			(*p_OkRelocate)[iProto] = TRUE;
			break;
		case bAllowDelayms:
			(*p_OkRelocate)[iProto] = FALSE;
			(*p_DelayMode)[iProto] = ABSOLUTE;
			GetField(NULL,TRUE,w,fAllowDelayms,line,&p,&q);
			(*p_MaxDelay)[iProto] = (long) p/q;
			break;
		case bAllowDelayPC:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_OkRelocate)[iProto] = FALSE;
			(*p_DelayMode)[iProto] = RELATIVE;
			GetField(NULL,TRUE,w,fAllowDelayPC,line,&p,&q);
			(*p_MaxDelay)[iProto] = (long) p/q;
			break;
		case bAllowFwdms:
			(*p_OkRelocate)[iProto] = FALSE;
			(*p_ForwardMode)[iProto] = ABSOLUTE;
			GetField(NULL,TRUE,w,fAllowFwdms,line,&p,&q);
			(*p_MaxForward)[iProto] = (long) p/q;
			break;
		case bAllowFwdPC:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_OkRelocate)[iProto] = FALSE;
			(*p_ForwardMode)[iProto] = RELATIVE;
			GetField(NULL,TRUE,w,fAllowFwdPC,line,&p,&q);
			(*p_MaxForward)[iProto] = (long) p/q;
			break;
		case bCopyFrom2:
			CopyFrom(w);
			break;
		}
	SetPrototype(iProto);
	rep = DONE;
	}
if(w == wPrototype3) {
	if(GetDialogValues(w) != OK) return(DONE);
	if(itemHit != bOKprototype3) {
		if(CheckiProto() != OK) return(DONE);
		UpdateDirty(TRUE,w);
		}
	switch(itemHit) {
		case bOKprototype3:
			if(GetPrototype(YES) != OK) return(DONE);
			HideWindow(Window[wPrototype3]);
			BPActivateWindow(QUICK,wPrototype1);
	/*		return(DONE); */
			break;
		case bNeverCoverBeg:
			if(CheckDuration(iProto) != OK) return(DONE);
			sprintf(line,"0");
			SetField(NULL,w,fCoverBegLessThanms,line);
			SetField(NULL,w,fCoverBegLessThanPC,line);
			(*p_MaxCoverBeg)[iProto] = ZERO;
			(*p_CoverBeg)[iProto] = FALSE;
			(*p_CoverBegMode)[iProto] = RELATIVE;
			break;
		case bCoverBegAtWill:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_CoverBeg)[iProto] = TRUE;
			(*p_CoverBegMode)[iProto] = RELATIVE;
			(*p_MaxCoverBeg)[iProto] = 100L;
			break;
		case bCoverBegLessThanms:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_CoverBeg)[iProto] = FALSE;
			(*p_CoverBegMode)[iProto] = ABSOLUTE;
			GetField(NULL,TRUE,w,fCoverBegLessThanms,line,&p,&q);
			(*p_MaxCoverBeg)[iProto] = (long) p/q;
			break;
		case bCoverBegLessThanPC:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_CoverBeg)[iProto] = FALSE;
			(*p_CoverBegMode)[iProto] = RELATIVE;
			GetField(NULL,TRUE,w,fCoverBegLessThanPC,line,&p,&q);
			(*p_MaxCoverBeg)[iProto] = (long) p/q;
			break;
		case bNeverCoverEnd:
			if(CheckDuration(iProto) != OK) return(DONE);
			sprintf(line,"0");
			SetField(NULL,w,fCoverEndLessThanms,line);
			SetField(NULL,w,fCoverEndLessThanPC,line);
			(*p_MaxCoverEnd)[iProto] = ZERO;
			(*p_CoverEnd)[iProto] = FALSE;
			(*p_CoverEndMode)[iProto] = RELATIVE;
			break;
		case bCoverEndAtWill:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_CoverEnd)[iProto] = TRUE;
			(*p_CoverEndMode)[iProto] = RELATIVE;
			(*p_MaxCoverEnd)[iProto] = 100L;
			break;
		case bCoverEndLessThanms:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_CoverEnd)[iProto] = FALSE;
			(*p_CoverEndMode)[iProto] = ABSOLUTE;
			GetField(NULL,TRUE,w,fCoverEndLessThanms,line,&p,&q);
			(*p_MaxCoverEnd)[iProto] = (long) p/q;
			break;
		case bCoverEndLessThanPC:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_CoverEnd)[iProto] = FALSE;
			(*p_CoverEndMode)[iProto] = RELATIVE;
			GetField(NULL,TRUE,w,fCoverEndLessThanPC,line,&p,&q);
			(*p_MaxCoverEnd)[iProto] = (long) p/q;
			break;
		case bNeverTruncBeg:
			if(CheckDuration(iProto) != OK) return(DONE);
			sprintf(line,"0");
			SetField(NULL,w,fTruncBegLessThanms,line);
			SetField(NULL,w,fTruncBegLessThanPC,line);
			(*p_MaxTruncBeg)[iProto] = ZERO;
			(*p_TruncBeg)[iProto] = FALSE;
			(*p_TruncBegMode)[iProto] = RELATIVE;
			break;
		case bTruncBegAtWill:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_TruncBeg)[iProto] = TRUE;
			(*p_TruncBegMode)[iProto] = RELATIVE;
			(*p_MaxTruncBeg)[iProto] = 100L;
			break;
		case bTruncBegLessThanms:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_TruncBeg)[iProto] = FALSE;
			(*p_TruncBegMode)[iProto] = ABSOLUTE;
			GetField(NULL,TRUE,w,fTruncBegLessThanms,line,&p,&q);
			(*p_MaxTruncBeg)[iProto] = (long) p/q;
			break;
		case bTruncBegLessThanPC:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_TruncBeg)[iProto] = FALSE;
			(*p_TruncBegMode)[iProto] = RELATIVE;
			GetField(NULL,TRUE,w,fTruncBegLessThanPC,line,&p,&q);
			(*p_MaxTruncBeg)[iProto] = (long) p/q;
			break;
		case bNeverTruncEnd:
			if(CheckDuration(iProto) != OK) return(DONE);
			sprintf(line,"0");
			SetField(NULL,w,fTruncEndLessThanms,line);
			SetField(NULL,w,fTruncEndLessThanPC,line);
			(*p_MaxTruncEnd)[iProto] = ZERO;
			(*p_TruncEnd)[iProto] = FALSE;
			(*p_TruncEndMode)[iProto] = RELATIVE;
			break;
		case bTruncEndAtWill:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_TruncEnd)[iProto] = TRUE;
			(*p_TruncEndMode)[iProto] = RELATIVE;
			(*p_MaxTruncEnd)[iProto] = 100L;
			break;
		case bTruncEndLessThanms:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_TruncEnd)[iProto] = FALSE;
			(*p_TruncEndMode)[iProto] = ABSOLUTE;
			GetField(NULL,TRUE,w,fTruncEndLessThanms,line,&p,&q);
			(*p_MaxTruncEnd)[iProto] = (long) p/q;
			break;
		case bTruncEndLessThanPC:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_TruncEnd)[iProto] = FALSE;
			(*p_TruncEndMode)[iProto] = RELATIVE;
			GetField(NULL,TRUE,w,fTruncEndLessThanPC,line,&p,&q);
			(*p_MaxTruncEnd)[iProto] = (long) p/q;
			break;
		case bNeverBreakTempo:
			(*p_BreakTempo)[iProto] = FALSE;
			break;
		case bBreakTempoAtWill:
			(*p_BreakTempo)[iProto] = TRUE;
			break;
		case bCopyFrom3:
			CopyFrom(w);
			break;
		}
	SetPrototype(iProto);
	rep = DONE;
	}
if(w == wPrototype4) {
	if(GetDialogValues(w) != OK) return(DONE);
	if(itemHit != bOKprototype4) {
		if(CheckiProto() != OK) return(DONE);
		UpdateDirty(TRUE,w);
		}
	switch(itemHit) {
		case bOKprototype4:
			if(GetPrototype(YES) != OK) return(DONE);
			HideWindow(Window[wPrototype4]);
			BPActivateWindow(QUICK,wPrototype1);
	/*		DrawPrototype(iProto,wPrototype1,&PictFrame);
			return(DONE); */
			break;
		case bDontForceContBeg:
			(*p_ContBeg)[iProto] = FALSE;
			(*p_ContBegMode)[iProto] = RELATIVE;
			break;
		case bForceContBeg:
			sprintf(line,"0");
			SetField(NULL,w,fAllowGapBegms,line);
			SetField(NULL,w,fAllowGapBegPC,line);
			(*p_ContBeg)[iProto] = TRUE;
			(*p_MaxBegGap)[iProto] = ZERO;
			(*p_ContBegMode)[iProto] = RELATIVE;
			break;
		case bAllowGapBegms:
			(*p_ContBeg)[iProto] = TRUE;
			(*p_ContBegMode)[iProto] = ABSOLUTE;
			GetField(NULL,TRUE,w,fAllowGapBegms,line,&p,&q);
			(*p_MaxBegGap)[iProto] = (long) p/q;
			break;
		case bAllowGapBegPC:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_ContBeg)[iProto] = TRUE;
			(*p_ContBegMode)[iProto] = RELATIVE;
			GetField(NULL,TRUE,w,fAllowGapBegPC,line,&p,&q);
			(*p_MaxBegGap)[iProto] = p/q;
			break;
		case bDontForceContEnd:
			(*p_ContEnd)[iProto] = FALSE;
			(*p_ContEndMode)[iProto] = RELATIVE;
			break;
		case bForceContEnd:
			sprintf(line,"0");
			SetField(NULL,w,fAllowGapEndms,line);
			SetField(NULL,w,fAllowGapEndPC,line);
			(*p_ContEnd)[iProto] = TRUE;
			(*p_MaxEndGap)[iProto] = ZERO;
			(*p_ContEndMode)[iProto] = RELATIVE;
			break;
		case bAllowGapEndms:
			(*p_ContEnd)[iProto] = TRUE;
			(*p_ContEndMode)[iProto] = ABSOLUTE;
			GetField(NULL,TRUE,w,fAllowGapEndms,line,&p,&q);
			(*p_MaxEndGap)[iProto] = (long) p/q;
			break;
		case bAllowGapEndPC:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_ContEnd)[iProto] = TRUE;
			(*p_ContEndMode)[iProto] = RELATIVE;
			GetField(NULL,TRUE,w,fAllowGapEndPC,line,&p,&q);
			(*p_MaxEndGap)[iProto] = (long) p/q;
			break;
		case bPreRollms:
			(*p_PreRollMode)[iProto] = ABSOLUTE;
			GetField(NULL,TRUE,w,fPreRollms,line,&p,&q);
			(*p_PreRoll)[iProto] = (long) p/q;
			SetPrototypeDuration(iProto,&longerCsound);
			break;
		case bPreRollPC:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_PreRollMode)[iProto] = RELATIVE;
			GetField(NULL,TRUE,w,fPreRollPC,line,&p,&q);
			if((*p_Dur)[iProto] > EPSILON)
				dur = (*p_Dur)[iProto]
					* ((100. + ((*p_PreRoll)[iProto] * 100. / (*p_Dur)[iProto]))
					/ (100. + ((double)p / q)));
			else dur = 0.;
			(*p_PreRoll)[iProto] = (long) (((double)p * dur) / 100. / q);
			SetPrototypeDuration(iProto,&longerCsound);
			break;
		case bPostRollms:
			(*p_PostRollMode)[iProto] = ABSOLUTE;
			GetField(NULL,TRUE,w,fPostRollms,line,&p,&q);
			if(p/q >= (*p_Dur)[iProto]) {
				sprintf(Message,"Pre-roll can't exceed object duration. Value '%ld' rejected",
					p/q);
				Alert1(Message);
				p = ZERO; q = 1L;
				}
			(*p_PostRoll)[iProto] = (long) p/q;
			SetPrototypeDuration(iProto,&longerCsound);
			break;
		case bPostRollPC:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_PostRollMode)[iProto] = RELATIVE;
			GetField(NULL,TRUE,w,fPostRollPC,line,&p,&q);
			if((((double) p) / q) >= 100.) {
				sprintf(Message,"Pre-roll can't exceed object duration. Value '%.2f' rejected",
					(((double) p) / q));
				Alert1(Message);
				p = ZERO; q = 1L;
				}
			if((*p_Dur)[iProto] > EPSILON)
				dur = (*p_Dur)[iProto] * ((100. - ((*p_PostRoll)[iProto] * 100.
					/ (*p_Dur)[iProto])) / (100. - ((double)p / q)));
			else dur = 0.;
			(*p_PostRoll)[iProto] = (long) (((double)p * dur) / 100. / q);
			SetPrototypeDuration(iProto,&longerCsound);
			break;
		case bCopyFrom4:
			CopyFrom(w);
			break;
		}
	SetPrototype(iProto);
	rep = DONE;
	}
if(w == wPrototype8) {
	if(GetDialogValues(w) != OK) return(DONE);
	if(itemHit != bOKprototype8) {
		if(CheckiProto() != OK) return(DONE);
		UpdateDirty(TRUE,w);
		}
	switch(itemHit) {
		case bOKprototype8:
			if(GetPrototype(YES) != OK) return(DONE);
			if(CompileObjectScore(iProto,&longerCsound) != OK) return(DONE);
			StopWait();
			HideWindow(Window[wPrototype8]);
			BPActivateWindow(QUICK,wPrototype1);
	/*		return(DONE); */
			break;
		case bCopyFrom8:
			CopyFrom(w);
			break;
		case bForceCurrChannel:
			(*p_DefaultChannel)[iProto] = 0;
			break;
		case bDontChangeChannel:
			(*p_DefaultChannel)[iProto] = -1;
			break;
		case bForceToChannel:
			if(GetField(NULL,TRUE,w,fForceToChannel,line,&p,&q) == OK
									&& CheckChannelRange(&p,&q) == OK) {
				(*p_DefaultChannel)[iProto] = p/q;
				}
			else {
				SetField(NULL,w,fForceToChannel,"[?]");
				SelectField(NULL,w,fForceToChannel,TRUE);
				return(FAILED);
				}
			break;
		case bForceCurrInstrument:
			if((*p_CsoundSize)[iProto] <= ZERO) {
				Alert1("This sound-object does not contain a Csound score");
				return(DONE);
				}
			(*p_CsoundInstr)[iProto] = 0;
			break;
		case bDontChangeInstrument:
			if((*p_CsoundSize)[iProto] <= ZERO) {
				Alert1("This sound-object does not contain a Csound score");
				return(DONE);
				}
			(*p_CsoundInstr)[iProto] = -1;
			break;
		case bForceToInstrument:
			if((*p_CsoundSize)[iProto] <= ZERO) {
				Alert1("This sound-object does not contain a Csound score");
				return(DONE);
				}
			if(GetField(NULL,TRUE,w,fForceToInstrument,line,&p,&q) == OK
									&& p/q > 0) {
				(*p_CsoundInstr)[iProto] = p/q;
				}
			else {
				SetField(NULL,w,fForceToInstrument,"[?]");
				SelectField(NULL,w,fForceToInstrument,TRUE);
				return(FAILED);
				}
			break;
		case bAcceptTransposition:
			(*p_OkTransp)[iProto] = 1 - (*p_OkTransp)[iProto];
			break;
		case bAcceptArticulation:
			(*p_OkArticul)[iProto] = 1 - (*p_OkArticul)[iProto];
			break;
		case bAcceptVolume:
			(*p_OkVolume)[iProto] = 1 - (*p_OkVolume)[iProto];
			break;
		case bAcceptPanoramic:
			(*p_OkPan)[iProto] = 1 - (*p_OkPan)[iProto];
			break;
		case bAcceptMap:
			(*p_OkMap)[iProto] = 1 - (*p_OkMap)[iProto];
			break;
		case bAcceptVelocity:
			(*p_OkVelocity)[iProto] = 1 - (*p_OkVelocity)[iProto];
			break;
		}
	SetPrototype(iProto);
	rep = DONE;
	}
if(w == wPrototype5) {
	if(itemHit != bOKprototype5 && itemHit != bPlayPrototype
		&& itemHit != bPlayPrototypeTicks && itemHit != bShowCodes) UpdateDirty(TRUE,w);
	if(itemHit != bOKprototype5)	{
		if(CheckiProto() != OK) return(DONE);
		}
	switch(itemHit) {
		case bOKprototype5:
			if(GetDialogValues(w) != OK) return(DONE);
			if(GetPrototype(YES) != OK) return(DONE);
			HideWindow(Window[wPrototype5]);
			BPActivateWindow(QUICK,wPrototype1);
	/*		return(DONE); */
			break;
		case bRecordPrototype:
			if(GetDialogValues(w) != OK) return(DONE);
			Hpos = -1;
			GetDialogItem(gpDialogs[w],itemHit,&itemtype,(Handle*)&itemhandle,&r);
			HiliteControl(itemhandle,11);
			if(RecordPrototype(iProto) == OK) UpdateDirty(TRUE,w);
			ClearWindow(NO,wNotice);
			HiliteControl(itemhandle,0);
			break;
		case bPlayPrototype:
			if(GetDialogValues(w) != OK) return(DONE);
			PlayPrototype(iProto);
			return(DONE);
			break;
		case bStriatedObject:
			ToggleButton(w,bStriatedObject);
			if(GetCtrlValue(wPrototype5,bStriatedObject) > 0) {
				GetField(NULL,TRUE,wPrototype5,fTref,line,&p,&q);
				if(((float)p)/q < EPSILON) {
					Alert1("Tref should be positive if object is striated");
					ToggleButton(w,bStriatedObject);
					}
				else (*p_Tref)[iProto] = p/q;
				}
			else {
				if(Answer("Making this object 'smooth' will suppress its pivot. Do you want it",
					'Y') != OK) ToggleButton(w,bStriatedObject);
				}
			if(GetPrototype(YES) != OK) return(DONE);
			UpdateDirty(TRUE,w);
			break;
		case bAdjustDurationms:
			if(GetField(NULL,TRUE,wPrototype5,fDurationAdjustms,line,&p,&q) != OK) break;
			if(AdjustDuration(iProto,(Milliseconds) p/q) == OK) UpdateDirty(TRUE,w);
			break;
		case bAdjustDBeats:
			if(GetField(NULL,TRUE,wPrototype5,fDurationAdjustbeats,line,&p,&q) != OK) break;
			if(AdjustDuration(iProto,(Milliseconds) ((*p_Tref)[iProto] * p)/q) == OK)
				UpdateDirty(TRUE,w);
			break;
		case bAdjustVelocities:
			if(GetField(NULL,TRUE,wPrototype5,fMinVelocity,line,&p,&q) != OK) break;
			vmin = p/q;
			if(vmin < 0 || vmin > 127) {
				sprintf(Message,"Velocity range is 0..127. Can't accept %ld",(long)vmin);
				Alert1(Message);
				break;
				}
			if(GetField(NULL,TRUE,wPrototype5,fMaxVelocity,line,&p,&q) != OK) break;
			vmax = p/q;
			if(vmax < 0 || vmax > 127) {
				sprintf(Message,"Velocity range is 0..127. Can't accept %ld",(long)vmax);
				Alert1(Message);
				break;
				}
			if(vmin > vmax) {
				sprintf(Message,
					"Max velocity should be larger or equal to minimum. Can't accept range %ld..%ld",
					(long)vmin,(long)vmax);
				Alert1(Message);
				break;
				}
			if(AdjustVelocities(iProto,vmin,vmax) == OK) UpdateDirty(TRUE,w);
			ClearWindow(NO,wNotice);
			break;
		case bQuantizeNoteOn:
			GetField(NULL,TRUE,wPrototype5,fQuantizeFractionBeat,line,&p,&q);
			if(p > ZERO) {
				if(Answer("Quantize NoteOn's",'N') != OK) return(DONE);
				(*p_Quan)[iProto] = ((double)(*p_Tref)[iProto] * q) / p;
				if(QuantizeNoteOn(iProto) == OK) UpdateDirty(TRUE,w);
				}
			else Alert1("Quantization should first be specified");
			break;
		case bExpandDurations:
			GetField(NULL,TRUE,wPrototype5,fExpandDurationsFractionBeat,line,&p,&q);
			if(p > ZERO) {
				if(Answer("Expand minimum durations",'N') != OK) return(DONE);
				if(ExpandDurations(iProto,(Milliseconds)((*p_Tref)[iProto] * q) / p) == OK)
					UpdateDirty(TRUE,w);
				}
			else Alert1("Minimum duration should first be specified");
			break;
		case bMakeMonodic:
			if(GetDialogValues(w) != OK) return(DONE);
			if(Answer("Suppress overlapping notes",'N') != OK) return(DONE);
			if(MakeMonodic(iProto) == OK) UpdateDirty(TRUE,w);
			ClearWindow(NO,wNotice);
			break;
		case bAppendAllNotesOff:
			if(GetDialogValues(w) != OK) return(DONE);
			if(AppendAllNotesOff(iProto) == OK) UpdateDirty(TRUE,w);
			ClearWindow(NO,wNotice);
			break;
		case bSuppressAllNotesOff:
			if(GetDialogValues(w) != OK) return(DONE);
			if(SuppressAllNotesOff(YES,iProto) == OK) UpdateDirty(TRUE,w);
			ClearWindow(NO,wNotice);
			break;
		case bSuppressKeyPressure:
			if(GetDialogValues(w) != OK) return(DONE);
			if(SuppressMessages(YES,iProto,KeyPressure) == OK) UpdateDirty(TRUE,w);
			ClearWindow(NO,wNotice);
			break;
		case bSuppressTrailingSilences:
			if(GetDialogValues(w) != OK) return(DONE);
			if((*pp_MIDIcode)[iProto] != NULL) (*((*pp_MIDIcode)[iProto]))[0].time = ZERO;
			if(SuppressMessages(NO,iProto,TimingClock) == OK) UpdateDirty(TRUE,w);
			ClearWindow(NO,wNotice);
			break;
		case bSuppressChannelPressure:
			if(GetDialogValues(w) != OK) return(DONE);
			if(SuppressMessages(YES,iProto,ChannelPressure) == OK) UpdateDirty(TRUE,w);
			ClearWindow(NO,wNotice);
			break;
		case bSuppressPitchBend:
			if(GetDialogValues(w) != OK) return(DONE);
			if(SuppressMessages(YES,iProto,PitchBend) == OK) UpdateDirty(TRUE,w);
			ClearWindow(NO,wNotice);
			break;
		case bInsertSilence:
			if(GetField(NULL,TRUE,wPrototype5,fInsertSilence,line,&p,&q) == OK) {
				if(InsertSilence(iProto,(Milliseconds) p/q) == OK) UpdateDirty(TRUE,w);
				ClearWindow(NO,wNotice);
				}
			else Alert1("Duration of silence should be specified");
			break;
		case bAppendSilence:
			if(GetField(NULL,TRUE,wPrototype5,fAppendSilence,line,&p,&q) == OK) {
				if(AppendSilence(iProto,(Milliseconds) p/q) == OK) UpdateDirty(TRUE,w);
				ClearWindow(NO,wNotice);
				}
			else Alert1("Duration of silence should be specified");
			break;
			
		case bCopyFrom5:
			CopyFrom(w);
			ClearWindow(NO,wNotice);
			break;
			
		case bImportPrototype:
			if (rep != DONE)	// stops loop on first import - 020807 akozar
				goto IMPORT;
			break;
			
		case bPlayPrototypeTicks:
			ToggleButton(w,bPlayPrototypeTicks);
			return(DONE);
			break;
		case bRecordPrototypeTick:
			RecordTick(TRUE,0);
			break;
			
		case bClearStreamInPrototype:
			if(Answer("Clear MIDI stream in this sound-object prototype",'N') == OK) {
				ptr = (Handle)(*pp_MIDIcode)[iProto];
				MyDisposeHandle(&ptr);
				(*pp_MIDIcode)[iProto] = NULL;
				(*p_MIDIsize)[iProto] = ZERO;
				ChangedProtoType(iProto);
				}
			break;
		case bShowCodes:
			ShowCodes(iProto);
			return(DONE);
			break;
		}
	SetPrototype(iProto);
	rep = DONE;
	}
if(w == wPrototype6) {
	if(itemHit != bOKprototype6 && itemHit != bShowDefaultStrikeMode) {
		if(CheckiProto() != OK) return(DONE);
		UpdateDirty(TRUE,w);
		}
	switch(itemHit) {
		case bOKprototype6:
			if(GetDialogValues(w) != OK) return(DONE);
			if(GetPrototype(YES) != OK) return(DONE);
			HideWindow(Window[wPrototype6]);
			BPActivateWindow(QUICK,wPrototype1);
	/*		return(DONE); */
			break;
		case bIrrelevantPeriod:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_PeriodMode)[iProto] = IRRELEVANT;
			if(GetPrototype(YES) != OK) return(DONE);
			break;
		case bBeforePeriodms:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_PeriodMode)[iProto] = ABSOLUTE;
			if(GetPrototype(YES) != OK) return(DONE);
			break;
		case bBeforePeriodPC:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_PeriodMode)[iProto] = RELATIVE;
			if(GetPrototype(YES) != OK) return(DONE);
			break;
		case bForceIntegerPeriod:
			if(CheckDuration(iProto) != OK) return(DONE);
			(*p_ForceIntegerPeriod)[iProto] = 1 - (*p_ForceIntegerPeriod)[iProto];
			if(GetPrototype(YES) != OK) return(DONE);
			break;
		case bDiscardNoteOffs:
			(*p_DiscardNoteOffs)[iProto] = 1 - (*p_DiscardNoteOffs)[iProto];
			if((*p_DiscardNoteOffs)[iProto] && (*p_StrikeAgain)[iProto] != FALSE) {
				Alert1("Since some NoteOn's will be discarded it is recommended to select the 'Don't strike again NoteOn's' mode");
				}
			if(GetPrototype(YES) != OK) return(DONE);
			break;
		case bStrikeAgain:
			(*p_StrikeAgain)[iProto] = TRUE;
			if(GetPrototype(YES) != OK) return(DONE);
			break;
		case bDontStrikeAgain:
			(*p_StrikeAgain)[iProto] = FALSE;
			if(GetPrototype(YES) != OK) return(DONE);
			break;
		case bStrikeDefault:
			(*p_StrikeAgain)[iProto] = -1;
			if(GetPrototype(YES) != OK) return(DONE);
			break;
		case bShowDefaultStrikeMode:
			ShowWindow(GetDialogWindow(StrikeModePtr));
			BringToFront(GetDialogWindow(StrikeModePtr));
			SetDefaultStrikeMode();
			BPUpdateDialog(StrikeModePtr);
			return(DONE);
			break;
		}
	SetPrototype(iProto);
	rep = DONE;
	}
if(w == wPrototype7) {
	if(itemHit != bOKprototype7) {
		if(CheckiProto() != OK) return(DONE);
		UpdateDirty(TRUE,w);
		}
	switch(itemHit) {
		case bOKprototype7:
			if(GetDialogValues(w) != OK) return(DONE);
			if(GetCsoundScore(iProto) != OK) return(DONE);
			if(CompileObjectScore(iProto,&longerCsound) != OK) return(DONE);
			StopWait();
			if(longerCsound > 0) {
				sprintf(Message,"The duration of the Csound sequence is %ld ms longer than that of the MIDI stream",
					(long) longerCsound);
				Alert1(Message);
				}
			if(longerCsound < 0) {
				sprintf(Message,"The duration of the Csound sequence is %ld ms shorter than that of the MIDI stream",
					(long) -longerCsound);
				Alert1(Message);
				}
			HideWindow(Window[wPrototype7]);
			BPActivateWindow(QUICK,wPrototype1);
			return(DONE);
			break;
		case bImportScore:
			if((rep=mOpenFile(w)) != OK) return(rep);
			break;
		case bExportScore:
			if((rep=mSaveAs(w)) != OK) return(rep);
			Created[w] = FALSE;
			return(DONE);
			break;
		}
	SetPrototype(iProto);
	rep = DONE;
	}
if(w == wCsoundInstruments) {
	if(itemHit != bResetCsoundInstrument
		&& itemHit != bLoadCsoundInstruments
		&& itemHit != bCsoundInstrumentHelp
		&& itemHit != bCopyCsoundInstrumentFrom
		&& GetDialogValues(w) != OK) return(DONE);
	switch(itemHit) {
		case bOKCsoundInstruments:
			if(GetRegressions(iCsoundInstrument) != OK) return(DONE);
			HideWindow(Window[w]);
			return(DONE);
			break;
		case bNewCsoundInstrumentFile:
			if(Answer("Create a new Csound instrument file",'N') != OK)
				return(DONE);
			if(ClearWindow(FALSE,wCsoundInstruments) == OK)
				ForgetFileName(wCsoundInstruments);
			return(DONE);
			break;
		case bLoadCsoundInstruments:
			if((rep=mOpenFile(w)) != OK) return(rep);
			BPActivateWindow(SLOW,w);
			return(DONE);
			break;
		case bSaveAsCsoundInstruments:
			Created[w] = FALSE;	/* No break! */
		case bSaveCsoundInstruments:
			if((rep=mSaveFile(w)) != OK) return(rep);
			return(DONE);
			break;
		case bResetCsoundInstrument:
			if((rep=SaveCheck(w)) != OK) return(rep);
			if(Answer("Reset all settings of this instrument",'N') != OK)
				return(DONE);
			ResetCsoundInstrument(iCsoundInstrument,NO,NO);
			UpdateDirty(TRUE,w);
			break;
		case bCopyCsoundInstrumentFrom:
			if(Jinstr < 2) {
				Alert1("No other instrument in this file");
				return(DONE);
				}
			if((rep=Answer("Copy specifications from another instrument",'N'))
				!= OK) return(DONE);
			i = DoThings(pp_CsInstrumentName,0,Jinstr,NULL,16,MINUSPROC,line,
				(int) pushButProc);
			if(i < 0) return(DONE);
			CopyCsoundInstrument(i,iCsoundInstrument);
			UpdateDirty(TRUE,w);
			break;
		case bNewCsoundInstrument:
			if(GetRegressions(iCsoundInstrument) != OK) return(DONE);
			if((rep=ResizeCsoundInstrumentsSpace(Jinstr + 1)) != OK) return(rep);
			iCsoundInstrument = Jinstr - 1;
			break;
		case bPrecedingCsoundInstrument:
			if(Jinstr < 2) {
				Alert1("No other instrument in this file");
				return(DONE);
				}
			if(GetRegressions(iCsoundInstrument) != OK) return(DONE);
			iCsoundInstrument--;
			if(iCsoundInstrument < 0) iCsoundInstrument = Jinstr - 1;
			break;
		case bNextCsoundInstrument:
			if(Jinstr < 2) {
				Alert1("No other instrument in this file");
				return(DONE);
				}
			if(GetRegressions(iCsoundInstrument) != OK) return(DONE);
			iCsoundInstrument++;
			if(iCsoundInstrument >= Jinstr) iCsoundInstrument = 0;
			break;
		case bGotoCsoundInstrument:
			if(Jinstr < 2) {
				Alert1("No other instrument in this file");
				return(DONE);
				}
			if(GetRegressions(iCsoundInstrument) != OK) return(DONE);
			sprintf(line,"Empty object");
			i = DoThings(pp_CsInstrumentName,0,Jinstr,NULL,16,MINUSPROC,line,
				(int) pushButProc);
			if(i >= 0) iCsoundInstrument = i;
			break;
		case bOctavePitchClass:
			(*p_CsPitchFormat)[iCsoundInstrument] = OPPC;
			UpdateDirty(TRUE,w);
			break;
		case bOctaveDecimal:
			(*p_CsPitchFormat)[iCsoundInstrument] = OPD;
			UpdateDirty(TRUE,w);
			break;
		case bHz:
			(*p_CsPitchFormat)[iCsoundInstrument] = CPS;
			UpdateDirty(TRUE,w);
	/* $$$		if(GetDialogValues(w) != OK) return(DONE); Check pitchbendrange */
			break;
		case (7+fPitchBendIndex):
			(*p_CsInstrument)[iCsoundInstrument].rPitchBend.islogx
				= 1 - (*p_CsInstrument)[iCsoundInstrument].rPitchBend.islogx;
			UpdateDirty(TRUE,w);
			break;
		case (8+fPitchBendIndex):
			(*p_CsInstrument)[iCsoundInstrument].rPitchBend.islogy
				= 1 - (*p_CsInstrument)[iCsoundInstrument].rPitchBend.islogy;
			UpdateDirty(TRUE,w);
			break;
		case (7+fVolumeIndex):
			(*p_CsInstrument)[iCsoundInstrument].rVolume.islogx
				= 1 - (*p_CsInstrument)[iCsoundInstrument].rVolume.islogx;
			UpdateDirty(TRUE,w);
			break;
		case (8+fVolumeIndex):
			(*p_CsInstrument)[iCsoundInstrument].rVolume.islogy
				= 1 - (*p_CsInstrument)[iCsoundInstrument].rVolume.islogy;
			UpdateDirty(TRUE,w);
			break;
		case (7+fPressureIndex):
			(*p_CsInstrument)[iCsoundInstrument].rPressure.islogx
				= 1 - (*p_CsInstrument)[iCsoundInstrument].rPressure.islogx;
			UpdateDirty(TRUE,w);
			break;
		case (8+fPressureIndex):
			(*p_CsInstrument)[iCsoundInstrument].rPressure.islogy
				= 1 - (*p_CsInstrument)[iCsoundInstrument].rPressure.islogy;
			UpdateDirty(TRUE,w);
			break;
		case (7+fModulationIndex):
			(*p_CsInstrument)[iCsoundInstrument].rModulation.islogx
				= 1 - (*p_CsInstrument)[iCsoundInstrument].rModulation.islogx;
			UpdateDirty(TRUE,w);
			break;
		case (8+fModulationIndex):
			(*p_CsInstrument)[iCsoundInstrument].rModulation.islogy
				= 1 - (*p_CsInstrument)[iCsoundInstrument].rModulation.islogy;
			UpdateDirty(TRUE,w);
			break;
		case (7+fPanoramicIndex):
			(*p_CsInstrument)[iCsoundInstrument].rPanoramic.islogx
				= 1 - (*p_CsInstrument)[iCsoundInstrument].rPanoramic.islogx;
			UpdateDirty(TRUE,w);
			break;
		case (8+fPanoramicIndex):
			(*p_CsInstrument)[iCsoundInstrument].rPanoramic.islogy
				= 1 - (*p_CsInstrument)[iCsoundInstrument].rPanoramic.islogy;
			UpdateDirty(TRUE,w);
			break;
		case bCsoundInstrumentHelp:
			DisplayHelp("Csound instruments");
			return(DONE);
			break;
		case bCheckCsoundOrchestra:
			Alert1("This version does not check consistency with the Csound orchestra file");
			return(DONE);
			break;
		case bq1:
		case bq2:
		case bq3:
		case bq4:
		case bq5:
			DisplayHelp("Argument boxes");
			return(DONE);
			break;
		case bMore:
			ShowWindow(GetDialogWindow(CsoundInstrMorePtr));
			SelectWindow(GetDialogWindow(CsoundInstrMorePtr));
			BPUpdateDialog(CsoundInstrMorePtr);
			return(DONE);
			break;
		case bCsoundTables:
			BPActivateWindow(SLOW,wCsoundTables);
			return(DONE);
			break;
		case bDisplayAsText:
			if(Answer("Display profile of this instrument in the 'Trace' window",'Y') == OK) {
				SetCsoundInstrument(iCsoundInstrument,wTrace);
				ShowSelect(CENTRE,wTrace);
				}
			return(DONE);
			break;
		case bExportAllInstruments:
			if(Answer("Instrument profiles will be displayed in the 'Trace' window, then you may save it as text.\nProceed",'Y') == OK) {
				if(ClearWindow(FALSE,wTrace) != OK) return(DONE);
				HideWindow(Window[wCsoundInstruments]);
				for(j=0; j < Jinstr; j++) {
					PleaseWait();
					SetCsoundInstrument(j,wTrace);
					}
				SetCsoundInstrument(iCsoundInstrument,-1);
				ShowWindow(Window[wCsoundInstruments]);
				ShowSelect(CENTRE,wTrace);
				}
			return(DONE);
			break;
		}
	SetCsoundInstrument(iCsoundInstrument,-1);
	if(GetRegressions(iCsoundInstrument) != OK) return(DONE);
	rep = DONE;
	}
if(w == wCsoundTables) {
	switch(itemHit) {
		case bOKCsoundTables:
			HideWindow(Window[wCsoundTables]);
			BPActivateWindow(SLOW,wCsoundInstruments);
			break;
		}
	return(DONE);
	}
if(w == wFilter) {
	switch(itemHit) {
		case bNoteOnOffIn:
			NoteOnIn = NoteOffIn = 1 - NoteOnIn;
			if(!NoteOnIn) {
				NoteOnPass = FALSE;
				Alert1("You disabled the reception of NoteOn's. Most MIDI data received by BP2 is now meaningless");
				}
			if(!NoteOffIn) NoteOffPass = FALSE;
			break;
		case bKeyPressureIn:
			KeyPressureIn = 1 - KeyPressureIn;
			if(!KeyPressureIn) KeyPressurePass = FALSE;
			break;
		case bControlIn:
			ControlTypeIn = 1 - ControlTypeIn;
			if(!ControlTypeIn) ControlTypePass = FALSE;
			break;
		case bProgramChangeIn:
			ProgramTypeIn = 1 - ProgramTypeIn;
			if(!ProgramTypeIn) ProgramTypePass = FALSE;
			break;
		case bChannelPressureIn:
			ChannelPressureIn = 1 - ChannelPressureIn;
			if(!ChannelPressureIn) ChannelPressurePass = FALSE;
			break;
		case bPitchBendIn:
			PitchBendIn = 1 - PitchBendIn;
			if(!PitchBendIn) PitchBendPass = FALSE;
			break;
		case bSysExIn:
			SysExIn = EndSysExIn = 1 - SysExIn;
			if(!SysExIn) SysExPass = FALSE;
			if(!EndSysExIn) EndSysExPass = FALSE;
			break;
		case bTimeCodeIn:
			TimeCodeIn = 1 - TimeCodeIn;
			if(!TimeCodeIn) TimeCodePass = FALSE;
			break;
		case bSongPositionIn:
			SongPosIn = 1 - SongPosIn;
			if(!SongPosIn) SongPosPass = FALSE;
			break;
		case bSongSelectIn:
			SongSelIn = 1 - SongSelIn;
			if(!SongSelIn) SongSelPass = FALSE;
			break;
		case bTuneRequestIn:
			TuneTypeIn = 1 - TuneTypeIn;
			if(!TuneTypeIn) TuneTypePass = FALSE;
			break;
		case bTimingClockIn:
			ClockTypeIn = 1 - ClockTypeIn;
			if(!ClockTypeIn) ClockTypePass = FALSE;
			break;
		case bStartStopIn:
			StartTypeIn = ContTypeIn = 1 - StartTypeIn;
			if(!StartTypeIn) StartTypePass = FALSE;
			if(!ContTypeIn) ContTypePass = FALSE;
			break;
		case bActiveSensingIn:
			ActiveSenseIn = 1 - ActiveSenseIn;
			if(!ActiveSenseIn) ActiveSensePass = FALSE;
			break;
		case bSystemResetIn:
			ResetIn = 1 - ResetIn;
			if(!ResetIn) ResetPass = FALSE;
			break;
			
		case bNoteOnOffPass:
			NoteOnPass = NoteOffPass = 1 - NoteOnPass;
			if(NoteOnPass) NoteOnIn = TRUE;
			if(NoteOffPass) NoteOffIn = TRUE;
			break;
		case bKeyPressurePass:
			KeyPressurePass = 1 - KeyPressurePass;
			if(KeyPressurePass) KeyPressureIn = TRUE;
			break;
		case bControlPass:
			ControlTypePass = 1 - ControlTypePass;
			if(ControlTypePass) ControlTypeIn = TRUE;
			break;
		case bProgramChangePass:
			ProgramTypePass = 1 - ProgramTypePass;
			if(ProgramTypePass) ProgramTypeIn = TRUE;
			break;
		case bChannelPressurePass:
			ChannelPressurePass = 1 - ChannelPressurePass;
			if(ChannelPressurePass) ChannelPressureIn = TRUE;
			break;
		case bPitchBendPass:
			PitchBendPass = 1 - PitchBendPass;
			if(PitchBendPass) PitchBendIn = TRUE;
			break;
		case bSysExPass:
			SysExPass = EndSysExPass = 1 - SysExPass;
			if(SysExPass) SysExIn = TRUE;
			if(EndSysExPass) EndSysExIn = TRUE;
			break;
		case bTimeCodePass:
			TimeCodePass = 1 - TimeCodePass;
			if(TimeCodePass) TimeCodeIn = TRUE;
			break;
		case bSongPositionPass:
			SongPosPass = 1 - SongPosPass;
			if(SongPosPass) SongPosIn = TRUE;
			break;
		case bSongSelectPass:
			SongSelPass = 1 - SongSelPass;
			if(SongSelPass) SongSelIn = TRUE;
			break;
		case bTuneRequestPass:
			TuneTypePass = 1 - TuneTypePass;
			if(TuneTypePass) TuneTypeIn = TRUE;
			break;
		case bTimingClockPass:
			ClockTypePass = 1 - ClockTypePass;
			if(ClockTypePass) ClockTypeIn = TRUE;
			break;
		case bStartStopPass:
			StartTypePass = ContTypePass = 1 - StartTypePass;
			if(StartTypePass) StartTypeIn = TRUE;
			if(ContTypePass) ContTypeIn = TRUE;
			break;
		case bActiveSensingPass:
			ActiveSensePass = 1 - ActiveSensePass;
			if(ActiveSensePass) ActiveSenseIn = TRUE;
			break;
		case bSystemResetPass:
			ResetPass = 1 - ResetPass;
			if(ResetPass) ResetIn = TRUE;
			break;
		case bFilterReset:
			if(Answer("Reset filter to startup settings",'N') == YES)
				ResetMIDIFilter();
			break;
		case bFilterReceiveSetAll:
			MIDIinputFilter = FILTER_ALL_ON;
			GetInputFilterWord();
			break;
		case bFilterReceiveClear:
			MIDIinputFilter = FILTER_ALL_OFF;
			GetInputFilterWord();
			// To be consistent for now, turn off transmit buttons too
			MIDIoutputFilter = FILTER_ALL_OFF;
			GetOutputFilterWord();
			break;
		case bFilterTransmitSetAll:
			MIDIoutputFilter = FILTER_ALL_ON;
			GetOutputFilterWord();
			// To be consistent for now, turn on receive buttons too
			MIDIinputFilter = FILTER_ALL_ON;
			GetInputFilterWord();
			break;
		case bFilterTransmitClear:
			MIDIoutputFilter = FILTER_ALL_OFF;
			GetOutputFilterWord();
			break;
		}
	Dirty[iSettings] = TRUE;
#if USE_BUILT_IN_MIDI_DRIVER
	// only necessary to send the filter settings to the serial driver - akozar
	if(InBuiltDriverOn && (rep=ResetMIDI(FALSE)) != OK) return(rep);
#endif
	if((rep=SetFilterDialog()) != OK) return(rep);
	rep = DONE;
	}
#if BP_MACHO
if(thedialog == CMSettings) {
	rep = DoCMSettingsEvent(p_event, itemHit);
	return(rep); // avoid hiding Message window
}
#endif

HideWindow(Window[wMessage]);
return(rep);
}


WritePatchName(void)
{
int j;
char line[MAXFIELDCONTENT];

if(TestMIDIChannel < 1 || TestMIDIChannel > MAXCHAN || CurrentMIDIprogram[TestMIDIChannel] < 1)
	return(OK);
for(j=0; j < 128; j++) {
	if((*p_GeneralMIDIpatchNdx)[j] == CurrentMIDIprogram[TestMIDIChannel]) {
		sprintf(line,"[%ld] %s",(long)CurrentMIDIprogram[TestMIDIChannel],
			*((*p_GeneralMIDIpatch)[j]));
		SetField(MIDIprogramPtr,-1,fPatchName,line);
		break;
		}
	}
return(OK);
}


/* This function may move memory on OS 9 and earlier */
int BPUpdateDialog(DialogPtr dp)
{
	RgnHandle rgn;

	rgn = NULL;
	rgn = NewRgn();
	if (rgn == NULL) return (FAILED);	// how are we supposed to check that NewRgn failed?
	GetPortVisibleRegion(GetDialogPort(dp), rgn);
	UpdateDialog(dp, rgn);
	DisposeRgn(rgn);

	return (OK);
}


int BPSetDialogAppearance(DialogPtr d, Boolean useThemeBackground)
{
	OSErr err;
	GrafPtr oldport;
	ControlRef ctrl;
	short numitems, item;
	ControlFontStyleRec fontstyle = { kControlUseSizeMask, 0, 11, 0, 0, 0, {0,0,0}, {0,0,0} };
	
	GetPort(&oldport);
	SetPortDialogPort(d);
	
	// these calls only require AppearanceLib, not CarbonLib
	// but we only are using them in the Carbon build for now
#if TARGET_API_MAC_CARBON
	if (RunningOnOSX) {
		TextSize(11);
		
		numitems = CountDITL(d);
		for (item = 1; item <= numitems; ++item) {
			err = GetDialogItemAsControl(d, item, &ctrl);
			if (err == noErr) SetControlFontStyle(ctrl, &fontstyle);
		}
	}
	if (useThemeBackground)
		SetThemeWindowBackground(GetDialogWindow(d), kThemeBrushDialogBackgroundActive, false);
#endif

	SetPort(oldport);
	return (OK);
}
