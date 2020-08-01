/* Interface1.c  (BP2 version CVS) */
 
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

static unsigned long gEventCount = 0;

MainEvent(void)
{
EventRecord event;
int eventfound,rep;
long sleeptime;
WindowPtr whichwindow;

if(EmergencyExit) {
	SetTimeOn = ComputeOn = PolyOn = CompileOn = SoundOn = SelectOn
		= PlaySelectionOn = PrintOn = GraphicOn = HangOn = ScriptExecOn
		= FALSE;
	if(ResumeStopOn) {
		ResumeStopOn = FALSE;
		HideWindow(GetDialogWindow(ResumeUndoStopPtr));
		HideWindow(GetDialogWindow(ResumeStopPtr));
		}
	}
	
EventState = NO;

if(!AlertOn) {
	MaintainCursor();
	MaintainMenus();
	if(Nw >= 0 && Nw < WMAX) {
		if(Editable[Nw] && !LockedWindow[Nw]) Idle(TEH[Nw]);
		if(HasFields[Nw]) TEIdle(GetDialogTextEditHandle(gpDialogs[Nw]));
		}
	if((rep=ListenMIDI(0,0,0)) != OK) return(rep);
	GetValues(0);
	}
if(ClockOverFlow) {
	ClockOverFlow = FALSE;
	if(Beta) Alert1("Clock overflow. Increase CLOCKSIZE!");
	}
if ((ComputeOn || CompileOn) && !WaitOn)  sleeptime = 1L;
else  sleeptime = GetCaretTime();
eventfound = WaitNextEvent(everyEvent,&event, sleeptime, NULL);
FindWindow(event.where,&whichwindow);
if(whichwindow == GetDialogWindow(FileSavePreferencesPtr)
	|| whichwindow == GetDialogWindow(TuningPtr)
	|| whichwindow == GetDialogWindow(DefaultPerformanceValuesPtr)
	|| whichwindow == GetDialogWindow(CsoundInstrMorePtr)) TEIdle(GetDialogTextEditHandle(GetDialogFromWindow(whichwindow)));
if(eventfound) { 
	++gEventCount; 
	rep = DoEvent(&event);
	PrintWindowState();
	return(rep);
	}
else return(DoSystem());
}


DoEvent(EventRecord *p_event)
{
WindowPtr whichwindow;
Rect r,dragrect;
int i,j,n,w,theclick,firstchar,diffchar,rep,start,found,intext;
long k,pos;
char thechar;
Point pt,newcorner,uppercorner;
GrafPtr saveport;

if(!AlertOn && (p_event->what == keyDown && ((p_event->modifiers & cmdKey) != 0))) {
	if(Jcontrol != -1) {
		Jcontrol = -1; ReadKeyBoardOn = FALSE; HideWindow(Window[wMessage]);
		return(OK);
		}
	rep = DoKeyCommand(p_event);
	return(rep);
	}
if(IsDialogEvent(p_event) && !AlertOn) {
	rep = DoDialog(p_event);
	if(rep != OK && rep != AGAIN) {
		if(rep == DONE) rep = OK;
		return(rep);
		}
	}
	
switch(p_event->what) {
	case kHighLevelEvent:
		Panic = FALSE;
		rep = DoHighLevelEvent(p_event);
		if(EventState != NO) return(EventState);
		break;
	case app4Evt:	/* was used by OMS */
		break;
	case mouseDown:
		Panic = FALSE;
		if(!ScriptRecOn) SwitchOff(NULL,wScriptDialog,bRecordScript);
		theclick = FindWindow(p_event->where,&whichwindow);
		PrintEvent(p_event, "case mouseDown", whichwindow);
		for(w=0; w < WMAX; w++) {
			if(whichwindow == Window[w]) break;
			}
		if(w == Nw && w == wGraphic) ShowDuration(NO);
		/* FIXME: This check can cause an alert box when the user 
		   clicks in the menubar or tries to switch windows. */
		if(Nbytes == ZERO
			&& Nw >= 0
			&& Nw < WMAX
			&& IsDialog[Nw]
			&& (rep=GetDialogValues(Nw)) != OK) return(rep);
		switch(theclick) {
			case inDesk:
				Help = FALSE;
				SysBeep(10);
				if(ResumeStopOn) {
					if(UndoFlag) BringToFront(GetDialogWindow(ResumeUndoStopPtr));
					else BringToFront(GetDialogWindow(ResumeStopPtr));
					}
				break;
			case inGoAway:
				PrintEvent(p_event, "case inGoAway", whichwindow);
				Help = FALSE;
				if(whichwindow == GetDialogWindow(DefaultPerformanceValuesPtr)) {
					if(GetDefaultPerformanceValues() != OK) return(OK);
					}
				if(whichwindow == GetDialogWindow(FileSavePreferencesPtr)) {
					if(GetFileSavePreferences() != OK) return(OK);
					}
				if(w == Nw) {
					if(TrackGoAway(whichwindow,p_event->where)) GoAway(w);
					}
				else {
					if(TrackGoAway(whichwindow,p_event->where))
						HideWindow(whichwindow);
					}
				if(whichwindow == GetDialogWindow(SixteenPtr)) HideWindow(Window[wInfo]);
				break;
			case inMenuBar:
				PrintEvent(p_event, "case inMenuBar", whichwindow);
				Option = FALSE;
				if((p_event->modifiers & optionKey) != 0) {
					Option = TRUE;
					SetOptionMenu(TRUE);
					}
				k = MenuSelect(p_event->where);
				// On OS X, QUIT AE handler is called from MenuSelect().
				if (EventState == EXIT) return(EXIT);
				SetOptionMenu(FALSE);
				ResetTickFlag = TRUE;
				rep = DoCommand(Nw,k);
				if(ResumeStopOn) {
					if(UndoFlag) BringToFront(GetDialogWindow(ResumeUndoStopPtr));
					else BringToFront(GetDialogWindow(ResumeStopPtr));
					}
				return(rep);
				break;
			case inSysWindow:	// FIXME ? Can we remove this entire event class in Carbon?
				Help = FALSE;
				GetPort(&saveport);
				SetPortWindowPort(whichwindow);
				GetWindowPortBounds(whichwindow, &r);
				InvalWindowRect(whichwindow, &r);
				r = LongRectToRect(TextGetViewRect(TEH[LastEditWindow]));
				SetPortWindowPort(Window[LastEditWindow]);
				InvalWindowRect(Window[LastEditWindow], &r);
				if(saveport != NULL) SetPort(saveport);
				else if(Beta) Alert1("Err DoEvent(). saveport == NULL");
#if !TARGET_API_MAC_CARBON
				SystemClick(p_event,whichwindow);
#endif
				ResetTickFlag = TRUE;
				if(0 <= w && w < WMAX) BPActivateWindow(SLOW,w);
				if(ResumeStopOn) {
					if(UndoFlag) BringToFront(GetDialogWindow(ResumeUndoStopPtr));
					else BringToFront(GetDialogWindow(ResumeStopPtr));
					}
				break;
			case inDrag:
				PrintEvent(p_event, "case inDrag", whichwindow);
				Jcontrol = -1; ReadKeyBoardOn = FALSE;
				if(whichwindow != FrontWindow()
						&& GetDialogWindow(ResumeStopPtr) != FrontWindow()) {
					Help = FALSE;
					if(w >= 0 && w < WMAX) BPActivateWindow(SLOW,w);
					else SelectWindow(whichwindow);
					}
				if(w == Nw) {
					dragrect = Set_Window_Drag_Boundaries();
					uppercorner = topLeft(r);
					LocalToGlobal(&uppercorner);
					DragWindow(whichwindow,p_event->where,&dragrect);
					GetWindowPortBounds(whichwindow, &r);
					newcorner = topLeft(r);
					LocalToGlobal(&newcorner);
					if(newcorner.v != uppercorner.v
							|| newcorner.h != uppercorner.h) {
						ChangedCoordinates[w] = TRUE;
						}
					if(w == wData) ShowDuration(NO);
					if(ResumeStopOn) {
						if(UndoFlag) BringToFront(GetDialogWindow(ResumeUndoStopPtr));
						else BringToFront(GetDialogWindow(ResumeStopPtr));
						}
					}
				else {
					Help = FALSE;
					if(w < WMAX) BPActivateWindow(QUICK,w);
					else {
						dragrect = Set_Window_Drag_Boundaries();
						DragWindow(whichwindow,p_event->where,&dragrect);
						/* FIXME: should modify ChangedCoordinates here too? */
						}
					}
				break;
			case inGrow:
				PrintEvent(p_event, "case inGrow", whichwindow);
				if(w == Nw /* && (!ClickRuleOn || w != wTrace) */) {
#if USE_MLTE
					if (w < MAXWIND && OKgrow[w]) { // growable text windows are all < MAXWIND
						TXNGrowWindow((*(TEH[w]))->textobj, p_event);
						}
					else
#endif
					if(MyGrowWindow(w,p_event->where) == OK) {
						if(w < WMAX) BPActivateWindow(SLOW,w);
						NewEnvironment = ChangedCoordinates[w] = TRUE;
						}
					}
				break;
			case inContent:
				PrintEvent(p_event, "case inContent", whichwindow);
				Jcontrol = -1;
				if(LastAction == TYPEWIND || LastAction == TYPEDLG) LastAction = NO;
				if(GetDialogWindow(FAQPtr) == FrontWindow()) {
					if(whichwindow != FrontWindow()) {
						//if(w < WMAX) {
						//	Help = FALSE;
						//	BPActivateWindow(SLOW,w);
						//	}
						//else SysBeep(10);
						}
					else {
						Help = TRUE;
						DoContent(whichwindow,p_event,&intext);
						break;	
						}
					}

				/* These windows "float", so we do content clicks when not the front window */
				if(whichwindow == GetDialogWindow(ResumeStopPtr)
							|| whichwindow == GetDialogWindow(ResumeUndoStopPtr)
							|| whichwindow == GetDialogWindow(SixteenPtr)
							|| whichwindow == GetDialogWindow(MIDIprogramPtr)
							|| whichwindow == GetDialogWindow(gpDialogs[wControlPannel])
							|| whichwindow == GetDialogWindow(gpDialogs[wScriptDialog])) {
						if(w >= 0 && w < WMAX) BPActivateWindow(QUICK,w);
						else {
							SelectWindow(whichwindow);
							UpdateWindow(FALSE,whichwindow);
							}
						rep = DoDialog(p_event);
						if((rep == OK || rep == AGAIN) && w < WMAX) {
							BPActivateWindow(SLOW,w);
							}
						if(rep == DONE) rep = OK;
						return(rep);
					}
				if(whichwindow != FrontWindow()) {
					if(w >= 0 && w < WMAX) BPActivateWindow(SLOW,w);
					else {
						SelectWindow(whichwindow);
						UpdateWindow(FALSE,whichwindow);
						}
					if(whichwindow == GetDialogWindow(FAQPtr)) Help = TRUE;
					}
				else {
					if(Help || w == Nw) {
						if(w < 0 || w >= WMAX || !LockedWindow[w])
							DoContent(whichwindow,p_event,&intext);
						else SysBeep(10);
						if(w >= 0 && w < WMAX) UpdateWindow(TRUE,Window[w]);
						}
					else {	// FIXME: what is this clause for?
						Help = FALSE;
						if(w < WMAX) BPActivateWindow(QUICK,w);
						}
					if(w == wGraphic) {
						ShowDuration(NO);
						if(ResumeStopOn) {
							if(UndoFlag) BringToFront(GetDialogWindow(ResumeUndoStopPtr));
							else BringToFront(GetDialogWindow(ResumeStopPtr));
							}
						}
					}
				break;
			default:
				break;
			}
		break;
	case keyDown:
	case autoKey:
		PrintEvent(p_event, "case keyDown", NULL);
		Panic = FALSE;
		if(Jcontrol != -1 || EnterOn) {
			Jcontrol = -1; ReadKeyBoardOn = FALSE;
			HideWindow(Window[wMessage]);
			return(OK);
			}
		if((ClickRuleOn && Nw == wTrace) || (Nw > -1 && Nw < WMAX && LockedWindow[Nw])) {
			SysBeep(10);
			break;
			}
		thechar = p_event->message & charCodeMask;
		if(thechar == '\5') return DoHelpKey();
		if(thechar >= 0 && thechar < 10 && thechar != '\b') {
			SysBeep(10);
			break;
			}
		Help = FALSE;
		if(Nw >= 0 && Nw < WMAX) {
			ScriptKeyStroke(Nw,p_event);
			if(thechar == '\b') {	// delete key
				// FIXME: with TextEdit, this destroys the contents of the clipboard
				// so we need another way to remember the deleted text for Undo-ing
				if(!USE_MLTE && !WASTE && Editable[Nw] && !LockedWindow[Nw]) {
					TextOffset selbegin, selend;
					TextGetSelection(&selbegin, &selend, TEH[Nw]);
					if((start=selbegin) < selend) {
						if(LastAction != COPY && LastAction != CUTWIND
													&& LastAction != PASTEWIND) {
							TextCopy(Nw);
							LastAction = DELETEWIND;
							UndoWindow = Nw;
							UndoPos = start;
							}
						else {
							if(LastAction == PASTEWIND) {
								if(WASTE) LastAction = DELETEWIND;
								else LastAction = NO;
								}
							}
						}
					else {
						if((start=selbegin) > 0 && LastAction != COPY
								&& LastAction != CUTWIND && LastAction != PASTEWIND) {
							
							SetSelect(start - 1,start,TEH[Nw]);
							TextCopy(Nw);
							LastAction = DELETEWIND;
							UndoWindow = Nw;
							UndoPos = start - 1;
							SetSelect(start,start,TEH[Nw]);
							}
						else {
							if(LastAction == PASTEWIND) {
								if(WASTE) LastAction = DELETEWIND;
								else LastAction = NO;
								}
							}
						}
					}
				else {
					if(Editable[Nw] && !LockedWindow[Nw]) {
						UndoWindow = Nw;
						LastAction = DELETEWIND;
						}
					if(HasFields[Nw]) {
						DialogDelete(gpDialogs[Nw]);
						LastAction = DELETEDLG;
						UndoWindow = Nw;
						}
					}
				}
			}
		if((p_event->modifiers & controlKey) != 0) return(OK);
		if((p_event->modifiers & optionKey) != 0) {
			if(DoArrowKey(Nw,thechar,(int) (p_event->modifiers & shiftKey),TRUE) == OK)
				return(OK);
			if(thechar == '\r') thechar = 'Â';
			}
		/* Can't be command key */
		if(Nw >= 0 && Nw < WMAX) TypeChar((int)thechar,(int)(p_event->modifiers & shiftKey));
		break;
	case activateEvt:
		PrintEvent(p_event, "case activateEvt", (WindowPtr)p_event->message);
		TEFromScrap();
		for(w=0; w < WMAX; w++) {
			if((WindowPtr) p_event->message == Window[w]) break;
			}
		if(w >= 0 && w < WMAX) {
			if(p_event->modifiers & activeFlag) {
				if(Editable[w] && !LockedWindow[w]) Activate(TEH[w]);
				if(OKvScroll[w]) ShowControl(vScroll[w]);
				if(OKhScroll[w]) ShowControl(hScroll[w]);
				/* DisableMenuItem(myMenus[editM],undoCommand); */
				}
			else {
				if(w != Nw) {
					if(Editable[w] && !LockedWindow[w]) Deactivate(TEH[w]);
					if(OKvScroll[w]) HideControl(vScroll[w]);
					if(OKhScroll[w]) HideControl(hScroll[w]);
					}
				}
			}
		break;
	case updateEvt:
		PrintEvent(p_event, "case updateEvt", (WindowPtr)p_event->message);
		for(w=0; w < WMAX; w++) {
			if((WindowPtr) p_event->message == Window[w]) break;
			}
#if USE_MLTE
		if(USE_MLTE && w >= 0 && w < MAXWIND) {
			TXNUpdate((*(TEH[w]))->textobj);
			}
		else
#endif
		{
			TEFromScrap();
			UpdateWindow(FALSE,(WindowPtr)p_event->message);
			}
		break;
	default: ;
	}
rep = OK;
SetResumeStop(FALSE);
return(rep);
}


int DoPreMacOS8KeyCommand(char thechar)
{
	/* On pre-MacOS 8 systems, we must trap commands that only have
	   an option-key shortcut and for commands that the option key modifies
	   we must map characters typed with the option key to their
	   non-modified keyboard equivalent.  This code only maps characters
	   typed with an American or French keyboard layout. */
	switch(thechar) {
		case 'É':	/* cmd option ; */
			Option = TRUE;
			thechar = ';';
			break;
		case '©':	/* cmd option G */
		case 'Þ':
			Option = TRUE;
			thechar = 'g';
			break;
		case 'í':	/* cmd option H */
		case 'ú':
			Option = TRUE;
			thechar = 'h';
			break;
		case '¿':	/* cmd option O */
		case 'Ï':
			Option = TRUE;
			thechar = 'o';
			break;
		case '¾':	/* cmd option A */
		case 'Œ':
			if(Finding) return(mFindAgain(Nw));
			break;
		case '«':	/* cmd option E */
		case '':
			return(mEnterFind(Nw));
			break;
		case ' ':	/* cmd option T */
			Token = 1 - Token;
			if(Token) sprintf(Message,"Type tokens enabled");
			else sprintf(Message,"Type tokens disabled");
			Alert1(Message);
			return (OK);
			break;
	}
	
	return DoCommand(Nw,MenuKey(thechar));
}


int DoKeyCommand(EventRecord *p_event)
{
	char thechar;
	int  rep;

	thechar = (char)(p_event->message & charCodeMask);
	Option = FALSE;
	/* trap key commands without menu equivalents or with special requirements */
	switch(thechar) {
		case -54:	/* cmd option space - don't set Option */
			if (!RunningOnOSX) return(mMiscSettings(Nw));
			break;
		case ' ':	/* cmd space */
			if(!RunningOnOSX && (Nw >= wPrototype1 && Nw <= wPrototype7) || Nw == wPrototype8)
				return(mPlaySelectionOrPrototype(Nw));
			else if((Oms || NEWTIMER) && (SoundOn || ComputeOn || PlaySelectionOn)
					&& OutMIDI && !PlayPrototypeOn) {
				Mute = 1 - Mute;
				MaintainMenus();
				BPActivateWindow(SLOW,wControlPannel);
				if(Mute) {
					sprintf(Message,"MUTE is ON...   cmd-space will turn if off");
					FlashInfo(Message);
					ShowMessage(TRUE,wMessage,Message);
					}
				else {
					HideWindow(Window[wInfo]);
					ClearMessage();
					}
				}
			break;
		case '?':
		case '/':
			return DoHelpKey();
			break;
		case '\34':	/* Left arrow */
			if(Nw >= 0 && Nw < WMAX && Editable[Nw]) {
				MoveLine(Nw,-1,(int) (p_event->modifiers & shiftKey));
				}
			else {
				if((*p_Tpict)[iProto] != Infneg) {
					(*p_Tpict)[iProto] -= 1;
					DrawPrototype(iProto,wPrototype1,&PictFrame);
					}
				}
			return(OK);
			break;
		case '\35':	/* Right arrow */
			if(Nw >= 0 && Nw < WMAX && Editable[Nw]) {
				MoveLine(Nw,+1,(int) (p_event->modifiers & shiftKey));
				}
			else {
				if((*p_Tpict)[iProto] != Infneg) {
					(*p_Tpict)[iProto] += 1;
					DrawPrototype(iProto,wPrototype1,&PictFrame);
					}
				}
			return(OK);
			break;
		case '\36':	/* Up arrow */
			if(Nw >= 0 && Nw < WMAX && OKvScroll[Nw]) {
				SetControlValue(vScroll[Nw],0);
				if(Editable[Nw]) {
					AdjustTextInWindow(Nw);
					SetSelect(ZERO,ZERO,TEH[Nw]);
					}
				else if(GrafWindow[Nw]) AdjustGraph(TRUE,Nw,vScroll[Nw]);
				}
			else {
				if((*p_Tpict)[iProto] != Infneg) {
					(*p_Tpict)[iProto] -= 10;
					DrawPrototype(iProto,wPrototype1,&PictFrame);
					}
				}
			return(OK);
			break;
		case '\37':	/* Down arrow */
			if(Nw >= 0 && Nw < WMAX && OKvScroll[Nw]) {
				SetControlValue(vScroll[Nw],GetControlMaximum(vScroll[Nw]));
				if(Editable[Nw]) {
					AdjustTextInWindow(Nw);
					SetSelect(GetTextLength(Nw),GetTextLength(Nw),TEH[Nw]);
					}
				else if(GrafWindow[Nw]) AdjustGraph(TRUE,Nw,vScroll[Nw]);
				}
			else {
				if((*p_Tpict)[iProto] != Infneg) {
					(*p_Tpict)[iProto] += 10;
					DrawPrototype(iProto,wPrototype1,&PictFrame);
					}
				}
			return(OK);
			break;
		}
	if((LastAction == TYPEWIND || LastAction == TYPEDLG) && thechar != 'z')
		LastAction = NO;
	// On pre-MacOS 8 systems without the Appearance manager, 
	// we must use the old code that only works for some keyboard layouts
	if (!HaveAppearanceManager)  rep = DoPreMacOS8KeyCommand(thechar);
	else {
		if((p_event->modifiers & optionKey) != 0) {
			Option = TRUE;
			SetOptionMenu(TRUE);
			}
		// MenuEvent() handles mapping option-key shortcuts correctly
		rep = DoCommand(Nw,MenuEvent(p_event));
		SetOptionMenu(FALSE);
		}
	return(rep);
}


ClearWindow(int reset,int w)
{
int i,j,rep,maxsounds,newinstruments,rs;
long count;
GrafPtr saveport;
char line[MAXLIN];
Rect r;
short itemtype;
ControlHandle itemhandle;
MIDI_Event e;

if(w >= WMAX || w < 0) {
	if(Beta) Alert1("Error ClearWindow(). w >= WMAX || w < 0");
	return(FAILED);
	}
GetPort(&saveport);
SetPortWindowPort(Window[w]);
rep = OK;

ReadKeyBoardOn = FALSE; Jcontrol = -1;

newinstruments
	= LoadedCsoundInstruments || Dirty[wCsoundInstruments] || Created[wCsoundInstruments];
if(!ScriptExecOn && (rep=SaveCheck(w)) != OK) goto OUT;
if(w != wMessage && Editable[w] && !LockedWindow[w]) {
	if(ScriptExecOn && w == wTrace) goto OUT;
	SetSelect(ZERO,GetTextLength(w),TEH[w]);
	TextDelete(w);
	SetSelect(ZERO,ZERO,TEH[w]);
	WindowFullAlertLevel[w] = 0;
	if(reset) {
		switch(w) {
			case wCsoundTables:
				sprintf(line,"f1 0 256 10 1 ; This table may be changed\n\n");
				break;
			case wData:
				sprintf(line,"%s<alphabet>\n%s<interactive code file>\n%s<settings file>\n%s<glossary file>\n%s<time base file>\n%s<MIDI orchestra file>\n<Generate or type items here>\n",
					FilePrefix[wAlphabet],FilePrefix[wInteraction],
					FilePrefix[iSettings],FilePrefix[wGlossary],FilePrefix[wTimeBase],
					FilePrefix[wMIDIorchestra]);
				break;
			case wGrammar:
				sprintf(line,"%s<alphabet>\n%s<interactive code file>\n%s<settings file>\n%s<glossary file>\n%s<time base file>\n%s<MIDI orchestra file>\n\n// Put grammar rules here\n\nCOMMENT:\nThis is an empty grammar...",
					FilePrefix[wAlphabet],FilePrefix[wInteraction],
					FilePrefix[iSettings],FilePrefix[wGlossary],FilePrefix[wTimeBase],
					FilePrefix[wMIDIorchestra]);
				break;
			case wAlphabet:
				sprintf(line,"%s<sound-object file>\n%s<keyboard file>\n%s<interactive code file>\n%s<MIDI orchestra file>\n",
					FilePrefix[iObjects],FilePrefix[wKeyboard],FilePrefix[wInteraction],
					FilePrefix[wMIDIorchestra]);
				break;
			case wInteraction:
				sprintf(line,"BP2 script\n// Here you may write a script fixing the interactive environment (IN... instructions) or load a '%s' interactive code file.\n",
					FilePrefix[wInteraction]);
				break;
			case wGlossary:
				sprintf(line,"BP2 script\n// Here you may write the glossary ('Define...' instructions) or load a '%s' glossary file.\n",
					FilePrefix[wGlossary]);
				break;
			case wScript:
				sprintf(line,"BP2 script\n// Here you may write a script (See 'Script' menu) or load a '%s' script file.\n",
					FilePrefix[wScript]);
				break;
			case wStartString:
				sprintf(line,"S\n");
				break;
			default: sprintf(line,"\0");
			}
		count = (long) strlen(line);
		PrintBehind(w,line);
		switch(w) {
			case wGrammar:
			case wAlphabet:
			case wData:
			case wCsoundTables:
				SetSelect(ZERO,count,TEH[w]);
				break;
			case wInteraction:
			case wGlossary:
			case wScript:
				SetSelect((long) strlen("BP2 script\n"),count,TEH[w]);
				break;
			default:
				SetSelect(count,count,TEH[w]);
				break;
			}
		}
	if(OKvScroll[w]) {
		SetControlValue(vScroll[w],0);
		SetVScroll(w);
		AdjustTextInWindow(w);
		}
	r = LongRectToRect(TextGetViewRect(TEH[w]));
	InvalWindowRect(Window[w], &r);
	}
if(GrafWindow[w] && reset) {
	KillDiagrams(w);
	if(w != wPrototype1) {
		GetWindowPortBounds(Window[w], &r);
		if(OKhScroll[w]) r.bottom = r.bottom - SBARWIDTH - 1;
		if(OKvScroll[w]) r.right = r.right - SBARWIDTH - 1;
		EraseRect(&r);
		}
	}
if(w == wPrototype1) {
	KillDiagrams(w);
	r = PictFrame;
	EraseRect(&r);
	}
switch(w) {
	case wData: ItemNumber = 0L; break;
	case wInteraction: LoadedIn = CompiledIn = FALSE; break;
	case wGlossary: LoadedGl = CompiledGl = FALSE; break;
	case wGrammar: CompiledGr = NeedAlphabet = FALSE; break;
	case wAlphabet: CompiledAl = CompiledGr = CompiledGl = NeedAlphabet = FALSE;
		if(!ScriptExecOn) {
			if(SaveCheck(iObjects) == OK) {
				ClearWindow(FALSE,iObjects);
				ForgetFileName(iObjects);
				}
			else TellOthersMyName(iObjects);
			}
		break;
	case iObjects:
	/*	if(ReleaseObjectPrototypes() != OK) {  Suppressed 30/3/2007
			rep = FAILED; goto OUT;
			} */
		break;
	case wCsoundTables:
		Dirty[wCsoundInstruments] = Created[wCsoundInstruments] = FALSE;
		break;
	case wCsoundInstruments:
		ClearWindow(YES,wCsoundTables);
		ResizeCsoundInstrumentsSpace(1);
		if(newinstruments) ResetCsoundInstrument(iCsoundInstrument,YES,NO);
		for(i=1; i <= MAXCHAN; i++) WhichCsoundInstrument[i] = -1;
		if(newinstruments) SetCsoundInstrument(iCsoundInstrument,-1);
		maxsounds = MyGetHandleSize((Handle)p_CompiledCsoundScore) / sizeof(char);
		for(j=2; j < maxsounds; j++) (*p_CompiledCsoundScore)[j] = FALSE;
		CompiledCsObjects = LoadedCsoundInstruments = FALSE;
		break;
	case wMIDIorchestra:
		SetField(MIDIprogramPtr,-1,fPatchName," ");
		if(TestMIDIChannel > 0 && TestMIDIChannel <= MAXCHAN) {
			if(CurrentMIDIprogram[TestMIDIChannel] > 0) {
				GetDialogItem(MIDIprogramPtr,(short)CurrentMIDIprogram[TestMIDIChannel],
							&itemtype, (Handle*)&itemhandle, &r);
				HiliteControl((ControlHandle) itemhandle,0);
				}
			GetDialogItem(SixteenPtr,(short)button1 + TestMIDIChannel - 1,
						&itemtype, (Handle*)&itemhandle, &r);
			HiliteControl((ControlHandle) itemhandle,0);
			}
		for(i=1; i <= MAXCHAN; i++) {
			CurrentMIDIprogram[i] = 0;
			SetField(NULL,wMIDIorchestra,i,"[unknown instrument]");
			if(ChangedMIDIprogram) {
				e.time = Tcurr;
				e.type = TWO_BYTE_EVENT;
				e.status = ProgramChange + i - 1;
				e.data2 = 0;
				rs = 0;
				if((IsMidiDriverOn()) && !InitOn)
					SendToDriver(Tcurr * Time_res,0,&rs,&e);
				}
			}
		ChangedMIDIprogram = FALSE;
		NewOrchestra = TRUE;
		break;
	}
Dirty[w] = Created[w] = FALSE;

OUT:
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err ClearWindow(). saveport == NULL");
return(rep);
}


GoAway(int w)
{
int wmem,r;

wmem = w; r = OK;
if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. GoAway()");
	return(OK);
	}
if(w == wPrototype1) {
	if(GetPrototype(TRUE) != OK) return(FAILED);
	HideWindow(Window[wPrototype2]);
	HideWindow(Window[wPrototype3]);
	HideWindow(Window[wPrototype4]);
	HideWindow(Window[wPrototype5]);
	HideWindow(Window[wPrototype6]);
	HideWindow(Window[wPrototype7]);
	HideWindow(Window[wPrototype8]);
	}
if(((w == wTrace || w == wHelp || GrafWindow[w]) && (r=ClearWindow(TRUE,w)) == OK)
		|| (!(w == wTrace || w == wHelp || GrafWindow[w]) && SaveCheck(w) == OK)) {
	HideWindow(Window[w]);
	if(GrafWindow[w]) KillDiagrams(w);
	for(w=0; w < WMAX; w++) {
		if(FrontWindow() == Window[w]) break;
		}
	if(w < WMAX && w != wmem) BPActivateWindow(SLOW,w);
	sprintf(Message,"\"%s\"",WindowName[wmem]);
	MystrcpyStringToTable(ScriptLine.arg,0,Message);
	AppendScript(158);
	}
return(r);
}


GetDialogValues(int w)
{
OSErr io;
int rep,dirty;

// FlushVolume();
if(w < 0 || w >= WMAX) return(OK);
switch(w) {
	case wTimeAccuracy: return(GetTimeAccuracy());
	case wRandomSequence: return(GetSeed());
	case wMetronom:
		dirty = Dirty[wMetronom];
		if((rep=GetTempo()) != OK) return(rep);
		if(dirty) {
			Dirty[wMetronom] = TRUE;
			SetGrammarTempo();
			Dirty[wMetronom] = FALSE;
			}
		return(OK);
	case wTimeBase:
		GetTimeBase();
		return(GetTickParameters());
	case wKeyboard: return(GetKeyboard());
	case wBufferSize: return(GetBufferSize());
	case wFindReplace: return(GetFindReplace());
	case wGraphicSettings: return(GetGraphicSettings());
	case wControlPannel: return(GetControlParameters());
	case wTickDialog: return(GetThisTick());
	case wCsoundInstruments:
	case wCsoundTables:
		return(GetCsoundInstrument(iCsoundInstrument));
	case wPrototype7:
		GetCsoundScore(iProto);
		break;
	case wPrototype1:
	case wPrototype2:
	case wPrototype3:
	case wPrototype4:
	case wPrototype5:
	case wPrototype6:
	case wPrototype8:
		return(GetPrototype(YES));
	}
/* SetNameChoice(); $$$ */
return(OK);
}


BPActivateWindow(int mode,int newNw)
{
Rect r,r1;
FontInfo myInfo;
int height,i,rep;
FontInfo font;
TextStyle thestyle;
char line[MAXFIELDCONTENT];

if((Nw == newNw && mode != SLOW && mode != AGAIN)  || newNw < 0) return(OK);
if(Nw >= WMAX || newNw >= WMAX) return(OK);
PrintCall("BPActivateWindow()", Window[newNw]);
if((LastAction == TYPEWIND || LastAction == TYPEDLG)
	&& (Editable[newNw] || HasFields[newNw]) && Nw != newNw) LastAction = NO;
if(TypeScript && ScriptRecOn && Nw != newNw) {
	TypeScript = FALSE;
	PrintBehind(wScript,"\n");
	}
if(Nw == newNw) goto NEXT;
if(Nw > -1 && (rep=GetDialogValues(Nw)) != OK) return(rep);
if(Nw == wInteraction && !CompiledIn) if(CompileInteraction() != OK) return(OK);
if(Nw == wGlossary && Nw != newNw && mode == SLOW) UpdateGlossary();
if(newNw != wInfo) HideWindow(Window[wInfo]);
if(newNw == wRandomSequence || Nw == wRandomSequence) SetSeed();
if(newNw == wMetronom || Nw == wMetronom) SetTempo();
if(newNw == wTimeAccuracy || Nw == wTimeAccuracy) SetTimeAccuracy();
if(newNw == wTimeBase || Nw == wTimeBase) {
	SetTimeBase();
	GetTickParameters();
	}
if(newNw == wFilter) {
	SetFilterDialog();
	}
if(newNw == wKeyboard || Nw == wKeyboard) SetKeyboard();
if(newNw == wBufferSize || Nw == wBufferSize) SetBufferSize();
if(newNw == wFindReplace || Nw == wFindReplace) SetFindReplace();
if(newNw == wGraphicSettings || Nw == wGraphicSettings) SetGraphicSettings();
if(newNw == wGraphicSettings) SelectField(NULL,wGraphicSettings,fGraphicScale,TRUE);
if(newNw == wPrototype1 && !CompileOn && mode != AGAIN && (!CompiledGr || !CompiledAl))
	return(mObjectPrototypes(Nw));
if(newNw == wCsoundInstruments) {
	if(iCsoundInstrument >= Jinstr) iCsoundInstrument = 0;
	SetCsoundInstrument(iCsoundInstrument,-1);
	}
if(Nw == wCsoundInstruments && newNw != wCsoundInstruments)
	GetCsoundInstrument(iCsoundInstrument);

NEXT:
if(Nw > -1 && Nw < WMAX) {
	if(Editable[Nw] && !IsDialog[Nw]) {
		if(Nw != newNw) Deactivate(TEH[Nw]);
		else Activate(TEH[Nw]);
		}
	SetPortWindowPort(Window[Nw]);
	{ RgnHandle cliprgn;
	  cliprgn = NewRgn();	// FIXME: should check return value; is it OK to move memory here?
	  GetPortClipRegion(GetWindowPort(Window[Nw]), cliprgn);
	  GetRegionBounds(cliprgn, &r1);
	  DisposeRgn(cliprgn);
	}
	GetWindowPortBounds(Window[Nw], &r);
	if(Nw != newNw) {
		ClipRect(&r);
		if(OKvScroll[Nw]) HideControl(vScroll[Nw]);
		if(OKhScroll[Nw]) HideControl(hScroll[Nw]);
		if(OKgrow[Nw]) DrawGrowIcon(Window[Nw]); // draws scrollbar outlines in inactive windows
		}
	ClipRect(&r1);
	OutlineTextInDialog(Nw,FALSE);
	GetWindowPortBounds(Window[Nw], &r);
	}
else ResumeStopOn = FALSE;
/* Help = FALSE; */
ShowWindow(Window[newNw]);
SetPortWindowPort(Window[newNw]);
if(Editable[newNw] && !LockedWindow[newNw]) {
	TextSize(WindowTextSize[newNw]);
	thestyle.tsSize = (short) WindowTextSize[newNw];
	TextSetStyle(doSize,&thestyle,0,TEH[newNw]);
	if(newNw != wHelp) LastEditWindow = newNw;
	if(WASTE && mode == SLOW) CalText(TEH[newNw]);
	}
SelectWindow(Window[newNw]);
if(OKvScroll[newNw]) ShowControl(vScroll[newNw]);
if(OKhScroll[newNw]) ShowControl(hScroll[newNw]);
if(OKgrow[newNw]) DrawGrowIcon(Window[newNw]);

/* DrawControls(Window[Nw]); */
if(OKvScroll[newNw] || OKhScroll[newNw])
{ RgnHandle cliprgn;
  cliprgn = NewRgn();	// FIXME: should check return value; is it OK to move memory here?
  GetPortClipRegion(GetWindowPort(Window[newNw]), cliprgn);
  UpdateControls(Window[newNw], cliprgn);	/* Fixed 14/2/98 */
  DisposeRgn(cliprgn);
}
Nw = newNw;
MaintainMenus(); DrawMenuBar();

#if 0  // strange difference in behavior between TE & WASTE; neither is necessary? - akozar
if(Nw < 0 || Nw >= WMAX) return(OK);
  if(!WASTE) UpdateWindow(FALSE,Window[Nw]);
  else if(mode == SLOW && Editable[Nw]) ShowSelect(CENTRE,Nw);
#endif
if((mode == SLOW || mode == AGAIN) && Nw == wPrototype1)
	DrawPrototype(iProto,wPrototype1,&PictFrame);

return(OK);
}


AdjustTextInWindow(int w)
{
long oldScroll,newScroll,maxScroll,delta;
Rect r;
GrafPtr saveport;

if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. AdjustTextInWindow(). w < 0 || w >= WMAX");
	return(OK);
	}
GetPort(&saveport);
SetPortWindowPort(Window[w]);
if(OKvScroll[w] && Editable[w]) {
#if !USE_MLTE
	oldScroll = (**(TEH[w])).viewRect.top - (**(TEH[w])).destRect.top;
	newScroll = GetControlValue(vScroll[w]) * LineHeight(w);
	delta = oldScroll - newScroll;
	if(delta != 0) {
		TextScroll(0,delta,TEH[w]);
/*		r = LongRectToRect(TextGetViewRect(TEH[w]));
		ValidWindowRect(Window[w], &r); */
		}
#endif
	}
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err AdjustTextInWindow(). saveport == NULL");
return(OK);
}


LineHeight(int w)
{
FontInfo font;
GrafPtr saveport;

GetPort(&saveport);
SetPortWindowPort(Window[w]);
GetFontInfo(&font);
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err LineHeight(). saveport == NULL");
return(font.ascent + font.descent + font.leading);
}


SetVScroll(int w)
{
int n;
long length;

if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. SetVScroll(). ");
	return(OK);
	}
if(OKvScroll[w] && Editable[w]) {
	n = LinesInText(w) - linesInFolder[w];
	length = GetTextLength(w);
	if(length > 0 && GetTextChar(w,length-1) == '\r') n++;
	if(n < 0) n = 0;
	SetControlMaximum(vScroll[w],n);
	if(GetControlValue(vScroll[w]) > n) SetControlValue(vScroll[w],n);
	}
return(OK);
}


ShowSelect(int dir,int w)
{
/* register */ int topLine,bottomLine,startline,endline,n,rep;
TextOffset selstart,selend;
short height;
Point pt1,pt2;
Rect r,rclip,r1;
WindowPtr window;
GrafPtr saveport;
#if WASTE
LongPt startpoint,endpoint;
#endif

if(w < 0 || w >= WMAX) {
	sprintf(Message,"Err. ShowSelect(). w = %ld. ",(long)w);
	if(Beta) Alert1(Message);
	return(OK);
	}
window = Window[w];
if(GrafWindow[w]) {
	GetPort(&saveport);
	SetPortWindowPort(window);
	GetWindowPortBounds(window, &rclip);
	if(OKhScroll[w]) rclip.bottom -= (SBARWIDTH + 1);
	if(OKvScroll[w]) rclip.right -= (SBARWIDTH + 1);
	ClipRect(&rclip);
	for(n=0; n < Ndiagram; n++) {
		if(DiagramWindow[n] == w && p_Diagram[n] != NULL) {
			if((rep=DrawGraph(w,p_Diagram[n])) != OK) return(rep);
			if(EmergencyExit) return(ABORT);
			}
		}
	if(!Offscreen) {
		for(n=0; n < Npicture; n++) {
			if(PictureWindow[n] == w && p_Picture[n] != NULL) {
				r = PictRect[n];
				DrawPicture(p_Picture[n],&r);
				r1 = r;
				if(r1.top < rclip.top) r1.top = rclip.top;
				if(r1.left < rclip.left) r1.left = rclip.left;
				if(r1.right > rclip.right) r1.right = rclip.right;
				if(r1.bottom > rclip.bottom) r1.bottom = rclip.bottom;
				ValidWindowRect(window, &r1);
				if(EmergencyExit) return(ABORT);
				}
			}
		}
	else {	// FIXME ? Should we call LockPixels before CopyBits?
#if NEWGRAF
		Rect rtemp;
		RgnHandle cliprgn;
		GetPortBounds(gMainGWorld, &rtemp);
		cliprgn = NewRgn();	// FIXME: should check return value; is it OK to move memory here?
		GetPortClipRegion(GetWindowPort(window), cliprgn);
		CopyBits((BitMap*)*GetGWorldPixMap(gMainGWorld),		// was (BitMap *)*gMainGWorld->portPixMap,
			   (BitMap*)*GetPortPixMap(GetWindowPort(window)),	// was &window->portBits,
			   &rtemp,								// was &gMainGWorld->portRect,
			   &rtemp,								// was &gMainGWorld->portRect,
			   srcCopy,
			   cliprgn);							// was window->clipRgn);
		DisposeRgn(cliprgn);
#endif
		}
	if(w == wGraphic && NoteScalePicture != NULL) {
		DrawPicture(NoteScalePicture,&NoteScaleRect);
		}
	GetWindowPortBounds(window, &rclip);
	ClipRect(&rclip);
	if(saveport != NULL) SetPort(saveport);
	else if(Beta) Alert1("Err ShowSelect(). saveport == NULL");
	GotAlert = FALSE;
	}

if(!Editable[w]) return(OK);
Activate(TEH[w]);
if(!OKvScroll[w]) return(OK);
SetVScroll(w);
topLine = GetControlValue(vScroll[w]);
bottomLine = topLine + linesInFolder[w] - 1;
#if WASTE
WEGetSelection(&selstart,&selend,TEH[w]);
WEGetPoint(selstart,&startpoint,&height,TEH[w]);
startline = topLine + (startpoint.v / height);
WEGetPoint(selend,&endpoint,&height,TEH[w]);
endline = topLine + (endpoint.v / height);
#elif USE_MLTE
TextGetSelection(&selstart,&selend,TEH[w]);
// FIXME: what else needs to be done for MLTE ??
#else
selstart = (**(TEH[w])).selStart;
selend = (**(TEH[w])).selEnd;
#endif
if(selstart < LineStartPos(topLine,topLine,w)
		|| selend >= LineStartPos(bottomLine+1,topLine,w)) {
#if !WASTE
	for(startline = 0; selstart >= LineStartPos(startline,topLine,w); startline++);
	for(endline = startline; selend >= LineStartPos(endline,topLine,w); endline++);
#endif
	if((endline - startline + 1) < linesInFolder[w])
		SetControlValue(vScroll[w],
			startline - (linesInFolder[w] - (endline - startline + 1)) / 2);
	else {
		switch(dir) {
			case CENTRE:
			case UP:
				SetControlValue(vScroll[w],startline - (linesInFolder[w]-1) / 2);
				break;
			case DOWN:
				SetControlValue(vScroll[w],endline - (linesInFolder[w]-1) / 2);
				break;
			}
		}
	AdjustTextInWindow(w);
	}
return(OK);
}


SetViewRect(int w)
{
Rect dr,vr;

if(w >= 0 && w < WMAX && Editable[w]) {
	GetWindowPortBounds(Window[w], &dr);
#if !USE_MLTE
	if(HasFields[w]) InsetRect(&dr,2,2); // Remove since it does nothing?
	if(OKvScroll[w]) {
		dr.right -= SBARWIDTH;
		if (!RunningOnOSX || Freebottom[w] > 0)
			dr.bottom = dr.bottom - SBARWIDTH - 1 - Freebottom[w];
		}
	linesInFolder[w] = (dr.bottom - dr.top - 2) / LineHeight(w);
	if(linesInFolder[w] > 0) {
		dr.bottom = dr.top + 1 + LineHeight(w) * linesInFolder[w];
		}
	else linesInFolder[w] = 1;
#else // FIXME: not sure we should do the rest of this for MLTE
	linesInFolder[w] = 1;
#endif
	if(w == wMessage) InsetRect(&dr,16,0);
//	if(w == wInfo) InsetRect(&dr,4,0); 
	vr = dr;
	if(linesInFolder[w] > 1) InsetRect(&dr,4,1);
	else InsetRect(&dr,4,0);
	SetTextDestRect(&dr,TEH[w]);
	SetTextViewRect(&vr,TEH[w]);
	CalText(TEH[w]);
	}
return(OK);
}

	
UpdateWindow(int quick, WindowPtr theWindow)
{
GrafPtr	saveport;
RgnHandle	cliprgn;
Rect r,r1;
int w;

if(theWindow == NULL || !IsWindowVisible(theWindow)) return(OK);

PrintCall("UpdateWindow()", theWindow);
for(w=0; w < WMAX; w++) {
	if(theWindow == Window[w]) break;
	}
GetPort(&saveport);
SetPortWindowPort(theWindow);
//if(!quick && w < WMAX && (!WASTE || !Editable[w])) SetViewRect(w);  // causing problems with TextEdit scroll position - 020907 akozar

// Because the Dialog manager has already updated our dialog with a text handle, 
// the update region may be empty.  Thus, this hack is needed to get the text to
// redraw.  A better solution might be to use a UserItem in the dialog - akozar 052307
if (w < WMAX && IsDialog[w] && Editable[w]) {
	r = LongRectToRect(TextGetViewRect(TEH[w]));
	InvalWindowRect(Window[w], &r);
	} 

// save clipping rectangle
cliprgn = NewRgn();	// FIXME: should check return value; is it OK to move memory here?
GetPortClipRegion(GetWindowPort(theWindow), cliprgn);
GetRegionBounds(cliprgn, &r1);

GetWindowPortBounds(theWindow, &r);
ClipRect(&r);
if(w < WMAX && GrafWindow[w] && (!quick || GotAlert)) ShowSelect(CENTRE,w);
BeginUpdate(theWindow);	/* This should not be placed higher */
GetPortClipRegion(GetWindowPort(theWindow), cliprgn);
UpdateControls(theWindow, cliprgn);
DisposeRgn(cliprgn);

if(w < WMAX) {
	if(IsDialog[w]) BPUpdateDialog(gpDialogs[w]);
	/* UpdateDialog needed to make static text visible */
	/* É in spite of Inside Mac saying it isn't p.I-418 */
	if(OKgrow[w]) DrawGrowIcon(theWindow);
	if(Editable[w]) {
		// AdjustTextInWindow(w); // add this or remove SetViewRect() call - 020907
		TextUpdate(w);
	}
	if(w == Nw) OutlineTextInDialog(w,TRUE);
	else OutlineTextInDialog(w,FALSE);
	}
else {
	if(theWindow == GetDialogWindow(ReplaceCommandPtr)
			|| theWindow == GetDialogWindow(FileSavePreferencesPtr)
			|| theWindow == GetDialogWindow(StrikeModePtr)
			|| theWindow == GetDialogWindow(TuningPtr)
			|| theWindow == GetDialogWindow(DefaultPerformanceValuesPtr)
			|| theWindow == GetDialogWindow(CsoundInstrMorePtr)
			|| theWindow == GetDialogWindow(MIDIkeyboardPtr)
			|| theWindow == GetDialogWindow(EnterPtr)
			|| theWindow == GetDialogWindow(GreetingsPtr)
			|| theWindow == GetDialogWindow(FAQPtr)
			|| theWindow == GetDialogWindow(SixteenPtr)
			|| theWindow == GetDialogWindow(MIDIprogramPtr)
			|| theWindow == GetDialogWindow(OMSinoutPtr)) {
		BPUpdateDialog(GetDialogFromWindow(theWindow));
		}
	}
EndUpdate(theWindow);
ClipRect(&r1);
// if(w < WMAX && HasFields[w] && !Editable[w]) HiliteDefault(GetDialogFromWindow(theWindow));
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err UpdateWindow(). saveport == NULL");
return(OK);
}


pascal void vScrollProc(ControlHandle theControl,short where)
{
int	pageSize;
int	scrollAmt,oldvalue,maxvalue;
TextHandle teh;
Rect r;
#if WASTE
  LongRect lr;
#else
  Rect lr;
#endif

if(Nw < 0 || Nw >= WMAX) return ;
if(Editable[Nw]) {
	teh = TEH[Nw];
	lr = TextGetViewRect(teh);
	pageSize = (lr.bottom-lr.top) / 
			WindowTextSize[Nw] - 1;
	}
else {
	GetWindowPortBounds(Window[Nw], &r);
	pageSize = (r.bottom - r.top) / 2;
	}
oldvalue = GetControlValue(theControl);
maxvalue = GetControlMaximum(theControl);
scrollAmt = 0;
switch(where) {
	case inUpButton: 
		if(oldvalue > 0) scrollAmt = -1;
		break;
	case inDownButton: 
		if(oldvalue < maxvalue) scrollAmt = 1;
		break;
	case inPageUp: 
		if(oldvalue > 0) scrollAmt = - pageSize;
		break;
	case inPageDown: 
		if(oldvalue < maxvalue) scrollAmt = pageSize;
		break;
	}
SetControlValue(theControl,oldvalue + scrollAmt);
if(scrollAmt != 0) AdjustTextInWindow(Nw);
}


pascal void hScrollProc(ControlHandle theControl,short where)
{
int	pageSize,oldvalue,maxvalue;
int	scrollAmt;
TextHandle teh;
Rect r;

if(Nw < 0 || Nw >= WMAX) return;
GetWindowPortBounds(Window[Nw], &r);
pageSize = (r.right - r.left) / 2;
oldvalue = GetControlValue(theControl);
maxvalue = GetControlMaximum(theControl);
scrollAmt = 0;
switch(where) {
	case inUpButton: 
		if(oldvalue > 0) scrollAmt = -1;
		break;
	case inDownButton: 
		if(oldvalue < maxvalue) scrollAmt = 1;
		break;
	case inPageUp: 
		if(oldvalue > 0) scrollAmt = - pageSize;
		break;
	case inPageDown: 
		if(oldvalue < maxvalue) scrollAmt = pageSize;
		break;
	}
/* SetCtlValue(theControl,oldvalue + scrollAmt); */
SetControlValue(theControl,oldvalue + scrollAmt);
}


DoContent(WindowPtr theWindow,EventRecord *p_event,int *p_intext)
{
int cntlCode,w,i,rep;
ControlHandle theControl;
int pageSize;
GrafPtr saveport;
Rect r,r1;
Str255 t;
char line[MAXLIN];

for(w=0; w < WMAX; w++) {
	if(theWindow == Window[w]) break;
	}
if(Help && w < WMAX) UpdateWindow(FALSE,Window[w]);
GetPort(&saveport);
SetPortWindowPort(theWindow);
GlobalToLocal(&p_event->where);

/* Moved allocation of UPPs to Inits() to avoid memory leaks */
/* vscrollptr = NewRoutineDescriptor((ProcPtr)vScrollProc,uppControlActionProcInfo,
	GetCurrentISA());
hscrollptr = NewRoutineDescriptor((ProcPtr)hScrollProc,uppControlActionProcInfo,
	GetCurrentISA()); */

(*p_intext) = FALSE;
	
if(w < WMAX && Editable[w]) {
	if((cntlCode = FindControl(p_event->where,theWindow,&theControl)) == 0) {
		MyLock(FALSE,(Handle)TEH[w]);
		r = LongRectToRect(TextGetViewRect(TEH[w]));
		if(PtInRect(p_event->where,&r)) {
			VariableOn = FALSE;
			TextAutoView(TRUE,TRUE,TEH[w]);
			TextClick(w,p_event);
			TextAutoView(TRUE,FALSE,TEH[w]);
			OutlineTextInDialog(w,TRUE);
			if(Help) ShowHelpOnText(w);
			(*p_intext) = TRUE;
			if(ReadKeyBoardOn && (GetClickedWord(w,line) == OK)
										&& (SelectControlArgument(w,line) == OK)) {
				LastEditWindow = w;
				MyUnlock((Handle)TEH[w]);
				if(saveport != NULL) SetPort(saveport);
				else if(Beta) Alert1("Err DoContent(). saveport == NULL");
				return(OK);
				}
			}
		else OutlineTextInDialog(w,FALSE);
		MyUnlock((Handle)TEH[w]);
		}
	else {
		if(cntlCode == inThumb) {
			TrackControl(theControl,p_event->where,(ControlActionUPP)0L);
			AdjustTextInWindow(w);
			}
		else TrackControl(theControl,p_event->where,vScrollUPP);
	/*	InvalWindowRect(Window[w], &r); */
		if(saveport != NULL) SetPort(saveport);
		else if(Beta) Alert1("Err DoContent(). saveport == NULL");
		return(OK);
		}
	}
if((cntlCode=FindControl(p_event->where,theWindow,&theControl)) != 0) {
	if(cntlCode == kControlButtonPart || cntlCode == inCheckBox) {
		if(Help) {
			Help = FALSE;
			GetControlTitle(theControl,t);
			MyPtoCstr(255,t,Message);
			FilterHelpSelector(Message);
			DisplayHelp(Message);
			}
		else {
			TrackControl(theControl,p_event->where,(ControlActionUPP)MINUSPROC);
			for(i=0; i < Jbutt; i++) {
				if(theControl == Hbutt[i]) {
					SetControlValue(theControl,(1 - GetControlValue(theControl)));
					if(MustBeSaved(i)) UpdateDirty(TRUE,iSettings);
					RecordButtonClick(i,GetControlValue(theControl));
					break;
					}
				}
			}
		goto OUT;
		}
	if(cntlCode != 0) {		/* Scrolling a graphic window */
		if(w < WMAX) {
			RgnHandle cliprgn;
			SelectWindow(Window[w]);
			GetPort(&saveport);
			SetPortWindowPort(Window[w]);
			cliprgn = NewRgn();	// FIXME: should check return value; is it OK to move memory here?
			GetPortClipRegion(GetWindowPort(Window[w]), cliprgn);
			GetRegionBounds(cliprgn, &r1);
			DisposeRgn(cliprgn);
			GetWindowPortBounds(Window[w], &r);
			ClipRect(&r);
			if(cntlCode == inThumb) {
				TrackControl(theControl,p_event->where,(ControlActionUPP)0L);
				}
			else {
				if(OKhScroll[w] && theControl == hScroll[w])
					TrackControl(theControl,p_event->where,hScrollUPP);
				if(OKvScroll[w] && theControl == vScroll[w])
					TrackControl(theControl,p_event->where,vScrollUPP);
				}
			InvalWindowRect(Window[w], &r);
			ClipRect(&r1);
			EraseRect(&r1);
			if(saveport != NULL) SetPort(saveport);
				else if(Beta) Alert1("Err DoContent(). saveport == NULL");
			AdjustGraph(TRUE,w,theControl);
			if((rep=BPActivateWindow(SLOW,w)) != OK) return(rep);
			}
		}
	}
else if(w < WMAX && (GrafWindow[w] || IsDialog[w])) (*p_intext) = TRUE;

OUT:																
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err DoContent(). saveport == NULL");
return(OK);
}


AdjustGraph(int redraw, int w, ControlHandle theControl)
{
int offseth,offsetv,n;
Rect r;
GrafPtr saveport;

if(!GrafWindow[w]) return(OK);

offseth = offsetv = 0;
if(OKhScroll[w] && theControl == hScroll[w])
	offseth =  GetControlValue(theControl) - SlideH[w];
if(OKvScroll[w] && theControl == vScroll[w])
	offsetv =  GetControlValue(theControl) - SlideV[w];
if(offseth != 0 || offsetv != 0) OffsetGraphs(w,offseth,offsetv);
SlideH[w] += offseth;
SlideV[w] += offsetv;

if(!redraw) return(OK);

OUT:
GetPort(&saveport);
SetPortWindowPort(Window[w]);
GetWindowPortBounds(Window[w], &r);
InvalWindowRect(Window[w], &r);
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err AdjustGraph(). saveport == NULL");
UpdateWindow(FALSE,Window[w]);

return(OK);
}


OffsetGraphs(int w, int offseth, int offsetv)
{
int n;

ClearWindow(FALSE,w);
for(n=0; n < Ndiagram; n++) {
	if(DiagramWindow[n] == w && p_Diagram[n] != NULL)
						OffsetPoly(p_Diagram[n],-offseth,-offsetv);
	}
	
if(!Offscreen) {
	for(n=0; n < Npicture; n++) {
		if(PictureWindow[n] == w && p_Picture[n] != NULL) {
			OffsetRect(&(PictRect[n]),-offseth,-offsetv);
			}
		}
	}

Vmin[w] -= offsetv;
Vmax[w] -= offsetv;
Vzero[w] -= offsetv;
Hmin[w] -= offseth;
Hmax[w] -= offseth;
Hzero[w] -= offseth;
return(OK);
}


MyGrowWindow(int w, Point p)
{
BitMap screenBits;
GrafPtr saveport;
long theresult;
Rect r;
int result;

if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. MyGrowWindow() ");
	return(OK);
	}
result = OK;
if(!OKgrow[w]) return(FAILED);
GetPort(&saveport);
SetPortWindowPort(Window[w]);
GetQDGlobalsScreenBits(&screenBits);
SetRect(&r,MINWINDOWHEIGHT,MINWINDOWWIDTH,
	screenBits.bounds.right - screenBits.bounds.left,
	screenBits.bounds.bottom - screenBits.bounds.top - SBARWIDTH);
theresult = GrowWindow(Window[w],p,&r);
if(theresult == 0)	// 0 means window did not change size
	result = FAILED;
else	{
	SizeWindow(Window[w],LoWord(theresult),HiWord(theresult),TRUE);
	AdjustWindowContents(w);
	}
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err MyGrowWindow(). saveport == NULL");
return(result);
}


/* Called after resizing a window (either in AdjustWindow() or MyGrowWindow()).
   You should set the window port before calling this function */
void AdjustWindowContents(int w)
{
Rect r;

GetWindowPortBounds(Window[w], &r);
ClipRect(&r);
EraseRect(&r);
InvalWindowRect(Window[w], &r);
SetViewRect(w);
MoveScrollBars(w, &r);
/* This next line was only in AdjustWindow() */
if(GrafWindow[w]) ClipRect(&r);
/* the following commented code was only in MyGrowWindow() */
/* suppressed resetting scroll position on 040507 - akozar */
/*if(GrafWindow[w]) {
	if(OKvScroll[w]) {
		SetControlValue(vScroll[w],0);
		AdjustGraph(FALSE,w,vScroll[w]);
		}
	if(OKhScroll[w]) {
		SetControlValue(hScroll[w],0);
		AdjustGraph(FALSE,w,hScroll[w]);
		}
	SlideH[w] = SlideV[w] = 0;
/*	ClipRect(&r); *\/
	GetWindowPortBounds(Window[w], &r);
	InvalWindowRect(Window[w], &r);
	UpdateWindow(FALSE,Window[w]);  /* 5/9/97 *\/
	}*/
AdjustTextInWindow(w);

return;
}

/* Pass the window bounds for w in r; MoveScrollbars moves and
   resizes the scroll bars and returns in r the bounds of the 
   content rectangle not occupied by the scroll bars. 
   You should set the window port before calling this function */
void MoveScrollBars(int w, Rect* r)
{
	HidePen();
	if(OKvScroll[w]) {
		MoveControl(vScroll[w],r->right - SBARWIDTH,r->top - 1);
		SizeControl(vScroll[w],SBARWIDTH + 1,r->bottom - r->top - (SBARWIDTH - 2)
			- Freebottom[w]);
		}
	if(OKhScroll[w]) {
		MoveControl(hScroll[w],r->left - 1,r->bottom - SBARWIDTH);
		SizeControl(hScroll[w],r->right - r->left - (SBARWIDTH - 2),SBARWIDTH + 1);
		r->bottom -= SBARWIDTH;
		}
	if(OKvScroll[w]) {
		r->right -= SBARWIDTH;
		SetVScroll(w);
		}
	ShowPen();
	if(GrafWindow[w]) SetMaxControlValues(w,*r);
	
	return;
}

SetMaxControlValues(int w, Rect r)
{
int maxslideh,maxslidev,v;

if(!GrafWindow[w]) return(OK);
maxslideh = Hmax[w] - Hmin[w] + 4 - (r.right - r.left);
if(OKhScroll[w]) SetControlMaximum(hScroll[w],maxslideh > 0 ? maxslideh : 0);
maxslidev = Vmax[w] - Vmin[w] + 4 - (r.bottom - r.top);
if(OKvScroll[w]) SetControlMaximum(vScroll[w],maxslidev > 0 ? maxslidev : 0);
return(OK);
}


ForgetFileName(int w)
{
RemoveWindowName(w);
FileName[w][0] = '\0';
SetName(w,YES,YES);
Created[w] = IsHTML[w] = IsText[w] = Weird[w] = FALSE;
IsHTML[wCsoundTables] = TRUE;
return(OK);
}


SetName(int w,int changed,int record)
{
int i,j,k;

if((changed || FileName[w][0] != '\0') && w != iSettings && w != wTrace
		&& w != wStartString && w != wHelp && w != wScrap && w != wNotice
		&& w != wPrototype7 && w != wCsoundTables) {
	if(FileName[w][0] == '\0') strcpy(Message,DeftName[w]);
	else strcpy(Message,FileName[w]);
	SetWTitle(Window[w],in_place_c2pstr(Message));
	strcpy(Message," ");
	/* This avoids messing the menu as name starts with '-' */
	if(FileName[w][0] == '\0') strcpy(Message,WindowName[w]);
	else strcat(Message,FileName[w]);
	c2pstrcpy(PascalLine, Message);
	switch(w) {
		case wGrammar:
			SetMenuItemText(myMenus[windowM],grammarCommand,PascalLine);
			break;
		case wData:
			SetMenuItemText(myMenus[windowM],dataCommand,PascalLine);
			break;
		case wAlphabet:
			SetMenuItemText(myMenus[windowM],alphabetCommand,PascalLine);
			break;
		case wKeyboard:
			SetMenuItemText(myMenus[windowM],keyboardCommand,PascalLine);
			break;
		case wScript:
			SetMenuItemText(myMenus[windowM],scriptCommand,PascalLine);
			break;
		case wInteraction:
			SetMenuItemText(myMenus[windowM],interactionCommand,PascalLine);
			break;
		case wGlossary:
			SetMenuItemText(myMenus[windowM],glossaryCommand,PascalLine);
			break;
		case wTimeBase:
			SetMenuItemText(myMenus[windowM],timebaseCommand,PascalLine);
			break;
		case wCsoundInstruments:
			SetMenuItemText(myMenus[deviceM],CsoundInstrumentSpecsCommand,PascalLine);
			break;
		case wMIDIorchestra:
			SetMenuItemText(myMenus[deviceM],MIDIorchestracommand,PascalLine);
			break;
		case iObjects:
			SetMenuItemText(myMenus[windowM],prototypeCommand,PascalLine);
			break;
		}
	DrawMenuBar();
	}
else if (w == iSettings) {
	if (Created[iSettings] && FileName[iSettings][0] != 0) {
		sprintf(Message, "Save '%s'", FileName[iSettings]);
		c2pstrcpy(PascalLine, Message);
		SetMenuItemText(myMenus[fileM], fmSaveSettings, PascalLine);
		}
	else  SetMenuItemText(myMenus[fileM], fmSaveSettings, "\pSave settings");
	}	
else {
	strcpy(Message,DeftName[w]);
	if(Message[0] != '\0') SetWTitle(Window[w],in_place_c2pstr(Message));
	}
if(record) TellOthersMyName(w);
return(OK);
}


TellOthersMyName(w)
{
if(FileName[w][0] != '\0') {
	switch(w) {
		case wAlphabet:
		case wInteraction:
		case wGlossary:
		case iSettings:
		case iMidiDriver:
		case wTimeBase:
			PutFirstLine(wGrammar,FileName[w],FilePrefix[w]);
			PutFirstLine(wData,FileName[w],FilePrefix[w]);
			break;
		case wCsoundInstruments:
			SetField(NULL,wPrototype1,fInstrumentFileName,FileName[w]);
			/* No break */
		case wMIDIorchestra:
			PutFirstLine(wGrammar,FileName[w],FilePrefix[w]);
			/* No break */
		case wKeyboard:
			PutFirstLine(wData,FileName[w],FilePrefix[w]);
			/* No break */
		case iObjects:
			PutFirstLine(wAlphabet,FileName[w],FilePrefix[w]);
			break;
		}
	}
return(OK);
}
		

PutFirstLine(int w, char s[], char prefix[])
{
int i,diff,count1,count2,dirtymem,compilemem,wasempty;
char *p,*q,line1[MAXLIN],line2[MAXLIN];
long pos,posinit,posmax,origin,end;

if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. PutFirstLine(). ");
	return(OK);
	}
/* New scheme 031607:  if filename (s) does not begin with prefix,
   then we add prefix + ":" to the beginning of the name, thus
   ensuring that it can be identified later.  This is just for the
   name that gets written into the top of the window; the actual 
   filename remains unchanged.  -- akozar */
if (strstr(s,prefix) == s) {
	strcpy(line1,s); strcpy(line2,prefix);
	}
else {
	strcpy(line2,prefix);
	strcpy(line1,prefix);
	strcat(line1,":");
	strcat(line1,s);
	}
TextGetSelection(&origin,&end,TEH[w]);
pos = ZERO;
posmax = GetTextLength(w);
count1 = strlen(line1); count2 = strlen(line2);
wasempty = IsEmpty(w);
dirtymem = Dirty[w];
if(w == wAlphabet) compilemem = CompiledAl;
if(w == wGrammar) compilemem = CompiledGr;
do {
	posinit = pos;
	if(ReadLine1(FALSE,w,&pos,posmax,LineBuff,MAXLIN) != OK) goto NOTFOUND;
	p = &LineBuff[0]; q = &line2[0];
	if(!Match(TRUE,&p,&q,count2)) continue;
	q = &line1[0];
	if(Match(TRUE,&p,&q,count1)
		&& (MySpace(LineBuff[count1]) ||  LineBuff[count1] == '\0')) return(OK);
	goto FOUND;
	}
while(TRUE);

NOTFOUND:
pos = posinit = ZERO;
SetSelect(pos,pos,TEH[w]);
PrintBehindln(w,line1);
SetSelect(origin+count1+1L,end+count1+1L,TEH[w]);
if(wasempty) Dirty[w] = dirtymem;
return(OK);

FOUND:	/* but it was different name */
SetSelect(posinit,(pos - 1L),TEH[w]);
TextDelete(w);
PrintBehind(w,line1);
if(origin >= posinit) origin += count1-pos+posinit+1L;
if(end >= posinit) end += count1-pos+posinit+1L;
SetSelect(origin,end,TEH[w]);
if(wasempty /* || w == wData || w == wAlphabet || w == wGrammar */) Dirty[w] = dirtymem;
if(w == wAlphabet) CompiledAl = compilemem;
if(w == wGrammar) CompiledGr = compilemem;
return(OK);
}


RemoveFirstLine(int w, char prefix[])
{
int i,diff,count,dirtymem,compilemem;
char *p,*q,line[MAXLIN],line2[MAXLIN];
long pos,posinit,posmax,origin,end;

if(prefix[0] == '\0') return(OK);
if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. RemoveFirstLine(). ");
	return(OK);
	}
dirtymem = Dirty[w];
if(w == wAlphabet) compilemem = CompiledAl;
if(w == wGrammar) compilemem = CompiledGr;
TextGetSelection(&origin,&end,TEH[w]);
pos = ZERO;
posmax = GetTextLength(w);
strcpy(line,prefix);
count = strlen(line);
do {
	posinit = pos;
	if(ReadLine1(FALSE,w,&pos,posmax,line2,MAXLIN) != OK) goto NOTFOUND;
	p = &line2[0]; q = &line[0];
	if(Match(TRUE,&p,&q,count) && line2[4] != '<') goto FOUND;
	}
while(TRUE);

NOTFOUND:
return(OK);

FOUND:
SetSelect(posinit,pos,TEH[w]);
TextDelete(w);
if(w == wAlphabet || w == wData || w == wGrammar) Dirty[w] = dirtymem;
else UpdateDirty(TRUE,w);
Created[w] = FALSE;
if(w == wAlphabet) CompiledAl = compilemem;
if(w == wGrammar) CompiledGr = compilemem;
if(origin >= posinit) origin -= pos-posinit;
if(end >= posinit) end -= pos-posinit;
if(origin < 0) origin = 0;
if(end < origin) end = origin;
SetSelect(origin,end,TEH[w]);
ShowSelect(CENTRE,w);
return(OK);
}


RemoveWindowName(int w)
{
if(FileName[w][0] == '\0')  return(OK);
switch(w) {
	case wGlossary:
	case wAlphabet:
	case wInteraction:
	case iMidiDriver:
	case wTimeBase:
		RemoveFirstLine(wGrammar,FilePrefix[w]);
		RemoveFirstLine(wData,FilePrefix[w]);
		break;
	case wCsoundInstruments:
		SetField(NULL,wPrototype1,fInstrumentFileName,"[no file]");
		// no break
	case wKeyboard:
	case iObjects:
	case wMIDIorchestra:
		RemoveFirstLine(wAlphabet,FilePrefix[w]);
		RemoveFirstLine(wData,FilePrefix[w]);
		RemoveFirstLine(wGrammar,FilePrefix[w]);
		break;
	}
return(OK);
}


PleaseWait(void)
{
if(ScriptExecOn) TurnWheel();
else if((LoadOn || SaveOn) && !InitOn) MoveDisk();
	else MoveFeet();
return(OK);
}


StopWait(void)
{
if(!ScriptExecOn || ResumeStopOn) SetDefaultCursor();
/* else TurnWheel(); */
return(OK);
}


UpdateDirty(int force,int w)
{
if(ScriptExecOn && !ResumeStopOn) return(OK);
#if WASTE
if(!force && Editable[w] && !HasFields[w] && WEGetModCount(TEH[w]) == 0) return(OK);
#endif
switch(w) {
	case wGrammar:
		CompiledGr = CompiledPt = NeedAlphabet = FALSE;
		break;
	case wAlphabet:
		CompiledAl = CompiledGr = CompiledGl = CompiledPt = NeedAlphabet = FALSE;
		break;
	case wInteraction:
		CompiledIn = FALSE; LoadedIn = TRUE;
		break;
	case wGlossary:
		CompiledGl = FALSE; LoadedGl = TRUE;
		break;
	case wCsoundInstruments:
		CompiledRegressions = FALSE;
		CompiledCsObjects = (*p_CompiledCsoundScore)[iProto] = FALSE;
		break;
	case wCsoundTables:
		w = wCsoundInstruments;
		break;
	case wPrototype7:
		CompiledCsObjects = (*p_CompiledCsoundScore)[iProto] = FALSE;
		/* no break */
	case wPrototype1:
	case wPrototype2:
	case wPrototype3:
	case wPrototype4:
	case wPrototype5:
	case wPrototype6:
	case wPrototype8:
		w = iObjects;
		break;
	}

if((!Dirty[w] && !LockedWindow[w] && (NeedSave[w] || HasFields[w]))
		|| w == iSettings) {
	Dirty[w] = TRUE;
	MaintainMenus();
	DrawMenuBar();	/* Needed to update the "save" command */
	}
return(OK);
}


Boolean PointIsInEditTextItem(DialogRef dp, Point pt)
{
	Handle		itemH;
	Rect			itemrect;
	DialogItemType	itemtype;
	DialogItemIndex	item;
	
	item = FindDialogItem(dp, pt) + 1; // + 1 because number returned is one less than needed
	GetDialogItem(dp, item, &itemtype, &itemH, &itemrect);
	if((itemtype & 127) == kEditTextDialogItem && PtInRect(pt,&itemrect))
		return TRUE;
	else	return FALSE;
}


MaintainCursor(void)
{
Point pt;
Rect r;
WindowPtr wPtr;
GrafPtr saveport;
int found;
Cursor arrow, *newcursor;

if(Help) {
	SetCursor(&HelpCursor);
	return(OK);
	}
if(ReadKeyBoardOn || Jcontrol >= 0) {
	SetCursor(&KeyboardCursor);
	return(OK);
	}
if(ScriptExecOn && !ResumeStopOn) {
	 SetCursor(&(WheelCursor[Jwheel]));
	 return(OK);
	 }
newcursor = GetQDGlobalsArrow(&arrow);
GetPort(&saveport);
wPtr = FrontWindow();
SetPortWindowPort(wPtr);
GetMouse(&pt);
if(Nw > -1 && Nw < WMAX && Ours(wPtr,Window[Nw])) {
	if(Editable[Nw] && !LockedWindow[Nw]) {
		r = LongRectToRect(TextGetViewRect(TEH[Nw]));
		if(PtInRect(pt,&r)) {
			newcursor = &EditCursor;
			}
		}
	else if(HasFields[Nw]) {
		/* r = (**(GetDialogTextEditHandle(gpDialogs[Nw]))).viewRect;
		if(PtInRect(pt,&r)) { */
		if (PointIsInEditTextItem(gpDialogs[Nw], pt)) {
			newcursor = &EditCursor;
			}
		}
	}
else if (wPtr == GetDialogWindow(EnterPtr) ||
         wPtr == GetDialogWindow(TuningPtr) ||
         wPtr == GetDialogWindow(DefaultPerformanceValuesPtr) ||
         wPtr == GetDialogWindow(CsoundInstrMorePtr) ||
         wPtr == GetDialogWindow(PatternPtr) ||
         wPtr == GetDialogWindow(FileSavePreferencesPtr) ||
         wPtr == GetDialogWindow(MIDIprogramPtr)) {
	if (PointIsInEditTextItem(GetDialogFromWindow(wPtr), pt)) {
		newcursor = &EditCursor;
		}
	}
SetCursor(newcursor);

if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err MaintainCursor(). saveport == NULL");
return(OK);
}


void SetDefaultCursor(void)
{	
	Cursor arrow;
	SetCursor(GetQDGlobalsArrow(&arrow));
	return;
}


TurnWheel(void)
{
if(NoCursor) return(OK);
if(--Jwheel < 0) Jwheel = 3;
if(ScriptExecOn) SetCursor(&(WheelCursor[Jwheel]));
return(OK);
}


MoveFeet(void)
{
if(NoCursor) return(OK);
if(--Jfeet < 0) Jfeet = 7;
SetCursor(&(FootCursor[Jfeet]));
return(OK);
}


MoveDisk(void)
{
if(NoCursor) return(OK);
if(--Jdisk < 0) Jdisk = 1;
SetCursor(&(DiskCursor[Jdisk]));
return(OK);
}


MaintainMenus(void)
{
Rect r;
short itemtype;
Boolean haveSelection;
Handle itemhandle;
int w;

if(InitOn) return(OK);

if(Nw < 0) w = LastEditWindow;
else w = FindGoodIndex(Nw);

#if !BP_MACHO // currently, these menu items are in all builds except Mach-O
if(Oms) {
	EnableMenuItem(myMenus[deviceM],OMSmidiCommand);
	EnableMenuItem(myMenus[deviceM],OMSstudioCommand);
	EnableMenuItem(myMenus[deviceM],OMSinoutCommand);
	CheckMenuItem(myMenus[deviceM],outOMSCommand,TRUE);
	}
else {
	DisableMenuItem(myMenus[deviceM],OMSmidiCommand);
	DisableMenuItem(myMenus[deviceM],OMSstudioCommand);
	DisableMenuItem(myMenus[deviceM],OMSinoutCommand);
	CheckMenuItem(myMenus[deviceM],outOMSCommand,FALSE);
	}
#endif

if(UndoFlag || LastAction != NO)
	EnableMenuItem(myMenus[editM],undoCommand);
else
	DisableMenuItem(myMenus[editM],undoCommand);
if(UseGraphicsColor)
	CheckMenuItem(myMenus[layoutM],graphicsColorCommand,TRUE);
else
	CheckMenuItem(myMenus[layoutM],graphicsColorCommand,FALSE);

#if 0 // smart cursor not implemented -- akozar
if(SmartCursor)
	CheckMenuItem(myMenus[miscM],smartcursorCommand,TRUE);
else
	CheckMenuItem(myMenus[miscM],smartcursorCommand,FALSE);
#endif

if(UseTextColor)
	CheckMenuItem(myMenus[layoutM],textColorCommand,TRUE);
else
	CheckMenuItem(myMenus[layoutM],textColorCommand,FALSE);
if(ReadKeyBoardOn)
	CheckMenuItem(myMenus[editM],typenoteCommand,TRUE);
else
	CheckMenuItem(myMenus[editM],typenoteCommand,FALSE);

if(UseBullet)
	CheckMenuItem(myMenus[layoutM],bulletCommand,TRUE);
else
	CheckMenuItem(myMenus[layoutM],bulletCommand,FALSE);

if(ShowPianoRoll)
	CheckMenuItem(myMenus[miscM],pianorollCommand,TRUE);
else
	CheckMenuItem(myMenus[miscM],pianorollCommand,FALSE);
	
switch(KeyboardType) {
	case AZERTY:
		CheckMenuItem(myMenus[layoutM],azertyCommand,TRUE);
		CheckMenuItem(myMenus[layoutM],qwertyCommand,FALSE);
		break;
	case QWERTY:
		CheckMenuItem(myMenus[layoutM],azertyCommand,FALSE);
		CheckMenuItem(myMenus[layoutM],qwertyCommand,TRUE);
		break;
	}
if(!ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn
		&& !SetTimeOn && !GraphicOn && !PrintOn && !ScriptExecOn) {
	EnableMenuItem(myMenus[fileM],fmNewProject);
	EnableMenuItem(myMenus[fileM],fmLoadProject);
	EnableMenuItem(myMenus[fileM],fmReceiveMIDI);
	EnableMenuItem(myMenus[fileM],fmLoadPattern);
	EnableMenuItem(myMenus[actionM],produceCommand);
	EnableMenuItem(myMenus[actionM],analyzeCommand);
//	ShowPannel(wControlPannel,dProduceItems);
	DisableMenuItem(myMenus[actionM],resumeCommand);
	DisableMenuItem(myMenus[actionM],stopCommand);
	DisableMenuItem(myMenus[actionM],pauseCommand);
	}
else {
	DisableMenuItem(myMenus[fileM],fmNewProject);
	DisableMenuItem(myMenus[fileM],fmLoadProject);
	DisableMenuItem(myMenus[fileM],fmReceiveMIDI);
	DisableMenuItem(myMenus[fileM],fmLoadPattern);
	DisableMenuItem(myMenus[actionM],produceCommand);
	DisableMenuItem(myMenus[actionM],analyzeCommand);
//	HidePannel(wControlPannel,dProduceItems);
	EnableMenuItem(myMenus[actionM],resumeCommand);
	EnableMenuItem(myMenus[actionM],stopCommand);
	EnableMenuItem(myMenus[actionM],pauseCommand);
	}
if(PauseOn) DisableMenuItem(myMenus[actionM],pauseCommand);
// FIXME ? Enabling these miscM items unnecessary ??
EnableMenuItem(myMenus[miscM],randomizeCommand);
EnableMenuItem(myMenus[miscM],accuracyCommand);
EnableMenuItem(myMenus[miscM],buffersizeCommand);
EnableMenuItem(myMenus[miscM],graphicsettingsCommand);
EnableMenuItem(myMenus[miscM],defaultPerformanceValuesCommand);
EnableMenuItem(myMenus[miscM],defaultStrikeModeCommand);
EnableMenuItem(myMenus[miscM],defaultFileSavePreferencesCommand);
EnableMenuItem(myMenus[miscM],tuningCommand);
EnableMenuItem(myMenus[editM],pasteCommand);
DisableMenuItem(myMenus[searchM],findagainCommand);
DisableMenuItem(myMenus[fileM],fmRevert);
if(Nw >= wRandomSequence && Nw != wScriptDialog) {
	DisableMenuItem(myMenus[fileM],fmOpen);
	DisableMenuItem(myMenus[fileM],fmClearWindow);
	DisableMenuItem(myMenus[fileM],fmSave);
	DisableMenuItem(myMenus[fileM],fmSaveAs);
	}
if((Nw < 0) || (Nw >= WMAX) || !IsWindowVisible(Window[Nw])
		|| (!Editable[Nw] && !HasFields[Nw] && Nw != wScriptDialog)
		|| (Nw == wInfo) || (Nw == wGraphic) || (Nw == wMessage)) {
	DisableMenuItem(myMenus[fileM],fmOpen);
	DisableMenuItem(myMenus[fileM],fmClearWindow);
	DisableMenuItem(myMenus[fileM],fmSave);
	DisableMenuItem(myMenus[fileM],fmSaveAs);
	DisableMenuItem(myMenus[editM],cutCommand);
	DisableMenuItem(myMenus[editM],copyCommand);
	DisableMenuItem(myMenus[editM],pasteCommand);
	DisableMenuItem(myMenus[editM],clearCommand);
	}
else {
	if(Finding) EnableMenuItem(myMenus[searchM],findagainCommand);
	if(!ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn
			&& !SetTimeOn && !GraphicOn && !PrintOn && !ScriptExecOn) {
		EnableMenuItem(myMenus[fileM],fmOpen);
		EnableMenuItem(myMenus[fileM],fmClearWindow);
		EnableMenuItem(myMenus[fileM],fmGoAway);
		}
	else {
		DisableMenuItem(myMenus[fileM],fmOpen);
		DisableMenuItem(myMenus[fileM],fmClearWindow);
		DisableMenuItem(myMenus[fileM],fmGoAway);
		}
		
	EnableMenuItem(myMenus[fileM],fmSaveAs);
	
	if(((!LockedWindow[w] && FileName[w][0] != '\0'
			&& (Dirty[w] || (w == wPrototype7 && Dirty[iObjects])))
			|| (GetDialogWindow(CsoundInstrMorePtr) == FrontWindow() && Dirty[wCsoundInstruments]
			&& FileName[wCsoundInstruments][0] != '\0')) && !ClickRuleOn)
		EnableMenuItem(myMenus[fileM],fmRevert);
		
	if(((Dirty[w] || (w == wPrototype7 && Dirty[iObjects]))
			|| (GetDialogWindow(CsoundInstrMorePtr) == FrontWindow() && Dirty[wCsoundInstruments]))
			&& !ClickRuleOn) {
		EnableMenuItem(myMenus[fileM],fmSave);
		}
	else {
		DisableMenuItem(myMenus[fileM],fmSave);
		}
	haveSelection = FALSE;
	if(Nw >= 0 && Nw < WMAX && Editable[Nw]) {
		haveSelection = !TextIsSelectionEmpty(TEH[Nw]);
		}
	else {
		if(Nw >= 0 && Nw < WMAX) haveSelection = HasFields[Nw];
		}
	if((Nw == wTrace && ClickRuleOn) || Nw < 0 || Nw >= WMAX
							|| (!Editable[Nw] && !HasFields[Nw]))
			DisableMenuItem(myMenus[editM],pasteCommand);
	else 	EnableMenuItem(myMenus[editM],pasteCommand);
	if(!haveSelection || (Nw == wTrace && ClickRuleOn)) {
		DisableMenuItem(myMenus[editM],cutCommand);
		DisableMenuItem(myMenus[editM],copyCommand);
		DisableMenuItem(myMenus[editM],clearCommand);
		DisableMenuItem(myMenus[searchM],enterfindCommand);
		}
	else {
		EnableMenuItem(myMenus[editM],cutCommand);
		EnableMenuItem(myMenus[editM],copyCommand);
		EnableMenuItem(myMenus[editM],clearCommand);
		EnableMenuItem(myMenus[searchM],enterfindCommand);
		}
	}
if(Nw >= 0 && Nw < WMAX && (Editable[Nw] || GrafWindow[Nw]))
	EnableMenuItem(myMenus[fileM],fmPrint);
else
	DisableMenuItem(myMenus[fileM],fmPrint);
DisableMenuItem(myMenus[actionM],analyzeCommand);
if (!TextIsSelectionEmpty(TEH[LastEditWindow])
		&& !ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn
		&& !GraphicOn && !SetTimeOn && !PrintOn && !ScriptExecOn) {
	EnableMenuItem(myMenus[actionM],playCommand);
	ShowPannel(wControlPannel,dPlaySelection);
	EnableMenuItem(myMenus[actionM],expandCommand);
	EnableMenuItem(myMenus[actionM],showPeriodsCommand);
	ShowPannel(wControlPannel,dExpandSelection);
	ShowPannel(wControlPannel,dShowPeriods);
	ShowPannel(wControlPannel,dAnalyze);
	ShowPannel(wControlPannel,bCaptureSelection);
	EnableMenuItem(myMenus[actionM],analyzeCommand);
	}
else {
	DisableMenuItem(myMenus[actionM],playCommand);
	HidePannel(wControlPannel,dPlaySelection);
	DisableMenuItem(myMenus[actionM],expandCommand);
	DisableMenuItem(myMenus[actionM],showPeriodsCommand);
	HidePannel(wControlPannel,dExpandSelection);
	HidePannel(wControlPannel,dShowPeriods);
	HidePannel(wControlPannel,dAnalyze);
	HidePannel(wControlPannel,bCaptureSelection);
	}

// change 'Play selection' to 'Play sound-object' if one of the prototype windows is active
if((Nw >= wPrototype1 && Nw <= wPrototype7) || Nw == wPrototype8) {
	SetMenuItemText(myMenus[actionM], playCommand, "\pPlay sound-object");
	// only enable if there is prototype to play
	if(iProto > 1 && iProto < Jbol) EnableMenuItem(myMenus[actionM],playCommand);
	else DisableMenuItem(myMenus[actionM],playCommand);
	}
else	SetMenuItemText(myMenus[actionM], playCommand, "\pPlay selection");

if(CompileOn || (CompiledGr && (CompiledGl || !LoadedGl)) || ClickRuleOn) {
	DisableMenuItem(myMenus[actionM],compileCommand);
	}
else EnableMenuItem(myMenus[actionM],compileCommand);

#if !BP_MACHO // currently, these menu items are in all builds except Mach-O
if(!InBuiltDriverOn) {	// was if(Oms) - 011607 - akozar
	CheckMenuItem(myMenus[deviceM],modemportCommand,FALSE);
	CheckMenuItem(myMenus[deviceM],printerportCommand,FALSE);
	DisableMenuItem(myMenus[deviceM],modemportCommand);
	DisableMenuItem(myMenus[deviceM],printerportCommand);
	}
else {
	EnableMenuItem(myMenus[deviceM],modemportCommand);
	EnableMenuItem(myMenus[deviceM],printerportCommand);
	if(Port == 1) {
		CheckMenuItem(myMenus[deviceM],modemportCommand,TRUE);
		CheckMenuItem(myMenus[deviceM],printerportCommand,FALSE);
		}
	else {
		CheckMenuItem(myMenus[deviceM],modemportCommand,FALSE);
		CheckMenuItem(myMenus[deviceM],printerportCommand,TRUE);
		}
	}
#endif

if(OutMIDI) CheckMenuItem(myMenus[deviceM],outMIDICommand,TRUE);
else CheckMenuItem(myMenus[deviceM],outMIDICommand,FALSE);
if(WriteMIDIfile) CheckMenuItem(myMenus[deviceM],outMIDIfileCommand,TRUE);
else CheckMenuItem(myMenus[deviceM],outMIDIfileCommand,FALSE);
if(OutCsound) CheckMenuItem(myMenus[deviceM],outCsoundCommand,TRUE);
else CheckMenuItem(myMenus[deviceM],outCsoundCommand,FALSE);
switch(NoteConvention) {
	case ENGLISH:
		CheckMenuItem(myMenus[miscM],englishconventionCommand,TRUE);
		CheckMenuItem(myMenus[miscM],frenchconventionCommand,FALSE);
		CheckMenuItem(myMenus[miscM],indianconventionCommand,FALSE);
		CheckMenuItem(myMenus[miscM],keyconventionCommand,FALSE);
		break;
	case FRENCH:
		CheckMenuItem(myMenus[miscM],frenchconventionCommand,TRUE);
		CheckMenuItem(myMenus[miscM],englishconventionCommand,FALSE);
		CheckMenuItem(myMenus[miscM],indianconventionCommand,FALSE);
		CheckMenuItem(myMenus[miscM],keyconventionCommand,FALSE);
		break;
	case INDIAN:
		CheckMenuItem(myMenus[miscM],indianconventionCommand,TRUE);
		CheckMenuItem(myMenus[miscM],frenchconventionCommand,FALSE);
		CheckMenuItem(myMenus[miscM],englishconventionCommand,FALSE);
		CheckMenuItem(myMenus[miscM],keyconventionCommand,FALSE);
		break;
	case KEYS:
		CheckMenuItem(myMenus[miscM],keyconventionCommand,TRUE);
		CheckMenuItem(myMenus[miscM],frenchconventionCommand,FALSE);
		CheckMenuItem(myMenus[miscM],indianconventionCommand,FALSE);
		CheckMenuItem(myMenus[miscM],englishconventionCommand,FALSE);
		break;
	}
DisableMenuItem(myMenus[layoutM],f9Command);
DisableMenuItem(myMenus[layoutM],f10Command);
DisableMenuItem(myMenus[layoutM],f12Command);
DisableMenuItem(myMenus[layoutM],f14Command);
DisableMenuItem(myMenus[editM],selectallCommand);
if(Nw >= 0 && Nw < WMAX
			&& (Editable[Nw] || Nw == wGraphic)) {
	EnableMenuItem(myMenus[layoutM],f9Command);
	EnableMenuItem(myMenus[layoutM],f10Command);
	EnableMenuItem(myMenus[layoutM],f12Command);
	EnableMenuItem(myMenus[layoutM],f14Command);
	CheckMenuItem(myMenus[layoutM],f9Command,FALSE);
	CheckMenuItem(myMenus[layoutM],f10Command,FALSE);
	CheckMenuItem(myMenus[layoutM],f12Command,FALSE);
	CheckMenuItem(myMenus[layoutM],f14Command,FALSE);
	switch(WindowTextSize[Nw]) {
		case 9:
			CheckMenuItem(myMenus[layoutM],f9Command,TRUE); break;
		case 10:
			CheckMenuItem(myMenus[layoutM],f10Command,TRUE); break;
		case 12:
			CheckMenuItem(myMenus[layoutM],f12Command,TRUE); break;
		case 14:
			CheckMenuItem(myMenus[layoutM],f14Command,TRUE); break;
		}
	}
if(Nw >= 0 && Nw < WMAX && (Editable[Nw] || HasFields[Nw])) {
	if(Editable[Nw]) EnableMenuItem(myMenus[searchM],0);
	EnableMenuItem(myMenus[editM],selectallCommand);
	}
if(SplitTimeObjects) CheckMenuItem(myMenus[layoutM],splitTimeObjectCommand,TRUE);
else CheckMenuItem(myMenus[layoutM],splitTimeObjectCommand,FALSE);
if(SplitVariables) CheckMenuItem(myMenus[layoutM],splitVariableCommand,TRUE);
else CheckMenuItem(myMenus[layoutM],splitVariableCommand,FALSE);
if(Token) {
	CheckMenuItem(myMenus[editM],tokenCommand,TRUE);
	CheckMenuItem(myMenus[editM],textCommand,FALSE);
	SwitchOn(NULL,wKeyboard,dToken);
	}
else {
	CheckMenuItem(myMenus[editM],tokenCommand,FALSE);
	CheckMenuItem(myMenus[editM],textCommand,TRUE);
	SwitchOff(NULL,wKeyboard,dToken);
	}
	
if(Dirty[wTimeBase]) ShowPannel(wTimeBase,dSaveTimeBase);
else HidePannel(wTimeBase,dSaveTimeBase);

/* if(ScriptExecOn) {
	SwitchOn(NULL,wScriptDialog,bExecScript);
	}
else {
	SwitchOff(NULL,wScriptDialog,bExecScript);
	} */

/* if(ComputeOn) SwitchOn(NULL,wControlPannel,dProduceItems);
else SwitchOff(NULL,wControlPannel,dProduceItems); */

if(Mute) SwitchOn(NULL,wControlPannel,bMute);
else SwitchOff(NULL,wControlPannel,bMute);

if(TransposeInput) SwitchOn(NULL,wControlPannel,dTransposeInput);
else SwitchOff(NULL,wControlPannel,dTransposeInput);

if(ProduceStackDepth > 0 && !ComputeOn) {
	ShowPannel(wControlPannel,dRepeatComputation);
	ShowPannel(wControlPannel,dDeriveFurther);
	}
else {
	HidePannel(wControlPannel,dRepeatComputation);
	HidePannel(wControlPannel,dDeriveFurther);
	}
DisableMenuItem(myMenus[actionM],templatesCommand);
if(CompiledGr && Gram.trueBP) {
	EnableMenuItem(myMenus[actionM],templatesCommand);
	ShowPannel(wControlPannel,dTemplates);
	}
else {
/*	HidePannel(wControlPannel,dTemplates); */
	DisableMenuItem(myMenus[actionM],templatesCommand);
	}
EnableMenuItem(myMenus[searchM],listterminalsCommand);
EnableMenuItem(myMenus[searchM],listvariablesCommand);
return(OK);
}


Rect Set_Window_Drag_Boundaries(void)     
{           
RgnHandle  the_gray_rgn;
Rect r;
   
the_gray_rgn = GetGrayRgn();
GetRegionBounds(the_gray_rgn, &r);  
return(r);  
}


MustBeSaved(int i)
{
switch(i) {
	case bImprovize:
	case bCyclicPlay:
	case bUseEachSub:
	case bAllItems:
	case bShowGraphic:
	case bDisplayItems:
	case bMIDI:
	case bSynchronizeStart:
	case bComputeWhilePlay:
	case bInteractive:
	case bResetWeights:
	case bResetFlags:
	case bResetControllers:
	case bNoConstraint:
	case bWriteMIDIfile:
	case bCsound:
	case bAllowRandomize:
		return(YES);
		break;
	case bDisplayProduce:
	case bStepProduce:
	case bStepGrammars:
	case bTraceProduce:
	case bPlanProduce:
	case bDisplayTimeSet:
	case bStepTimeSet:
	case bTraceTimeSet:
	case bCsoundTrace:
	case bShowMessages:
	case bOMS:
		break;
	}
return(NO);
}


GetHighLevelEvent(void)
{
EventRecord event;
int rep,eventfound;

rep = OK;
eventfound = GetNextEvent(highLevelEventMask,&event);
if(eventfound && event.what == kHighLevelEvent) {
	rep = DoHighLevelEvent(&event);
	if(EventState != NO) return(EventState);
	}
return(rep);
}


int DoHelpKey()
{
	if(!Help) return(mHelp(Nw));
	else {
		Help = FALSE;
		DisplayHelp("Help");
		}
	return(OK);
}


#if 0
PageClick(int up)
{
EventRecord event;
Point pt;
Rect r;

if(Nw < 0 || Nw >= WMAX || !Editable[Nw]) return(OK);
SetPortWindowPort(Window[Nw]);
GetWindowPortBounds(Window[Nw], &r);
event.what = mouseDown;
pt.h = r.right - r.left + SBARWIDTH/2;
if(up) pt.v = SBARWIDTH;
else pt.v = r.bottom - r.top - SBARWIDTH;
LocalToGlobal(&pt);
event.where = pt;
DoEvent(&event);
return(OK);
}
#endif


#if TRACE_EVENTS
/* Functions for tracing event handling via the console */
/* Added 040507 by akozar. */


const char	EVENT_NAMES[25][20] = { "nullEvent", "mouseDown", "mouseUp", "keyDown", "keyUp", "autoKey",
						"updateEvt", "diskEvt", "activateEvt", "", "", "", "", "", "",
						"osEvt", "", "", "", "", "", "", "", "highLvlEvt", "unknown" };

char* TEWindowName(WindowPtr wp)
{
	int w;
	
	for(w=0; w < WMAX; w++)
		if(wp == Window[w]) break;
	if(w >= 0 && w < WMAX)  return WindowName[w];
	else if (wp == GetDialogWindow(GreetingsPtr))				return "Greetings";
	else if (wp == GetDialogWindow(FAQPtr))					return "FAQ";
	else if (wp == GetDialogWindow(EnterPtr))					return "Enter";
	else if (wp == GetDialogWindow(ReplaceCommandPtr))			return "Replace-Next-Stop";
	else if (wp == GetDialogWindow(ResumeStopPtr))				return "Resume-Stop";
	else if (wp == GetDialogWindow(ResumeUndoStopPtr))			return "Resume-Undo-Stop";
	else if (wp == GetDialogWindow(FileSavePreferencesPtr))		return "File Save Prefs";
	else if (wp == GetDialogWindow(PatternPtr))				return "Pattern";
	else if (wp == GetDialogWindow(StrikeModePtr))				return "Default Strike Mode";
	else if (wp == GetDialogWindow(TuningPtr))				return "Tuning";
	else if (wp == GetDialogWindow(DefaultPerformanceValuesPtr))	return "Default Perf Values";
	else if (wp == GetDialogWindow(CsoundInstrMorePtr))			return "Csound Instr More";
	else if (wp == GetDialogWindow(MIDIkeyboardPtr))			return "Note Conventions";
	else if (wp == GetDialogWindow(SixteenPtr))				return "MIDI Channels";
	else if (wp == GetDialogWindow(MIDIprogramPtr))				return "MIDI programs";
	else return "";
}

void	PrintEvent(EventRecord* e, char* funcname, WindowPtr wp)
{
	static int lastwhat = -1, lastwhen = -1;
	int	type = e->what, when = gEventCount;
	
	if	(lastwhat == nullEvent && type == nullEvent && lastwhen != when)	{
		// don't print > 1 null event in a row but trace the first null event
		lastwhat = type;
		lastwhen = when;
		return;
	}
	printf("%3d  %-20s:  ", when, funcname);			// print event count and calling function's name
	if	(type < 0 || type > 23)  type = 24;			// unknown event type
	printf("%-11s  %10d", EVENT_NAMES[type], e->message);	// print type and message, and act/deact
	if	(type == activateEvt)
		printf(" %s", (e->modifiers&1)?"act  ":"deact"); // activate or deactivate?
	else  printf("      ");
	if 	(wp != NULL)  printf("  %s", TEWindowName(wp));	// print window title
	printf("\n");
	fflush(stdout);
	
	lastwhat = type;
	lastwhen = when;
	return;
}

void	PrintWindowState()
{
	printf("     FrontWindow() == %-20s Nw == %2d       %s\n", TEWindowName(FrontWindow()), 
		Nw, ((Nw >= 0 && Nw < WMAX) ? TEWindowName(Window[Nw]) : ""));
	return;
}

void	PrintCall(char* funcname, WindowPtr wp)
{
	printf("     %-20s:  ", funcname);				// print calling function's name
	printf("                                ");		// leave space
	if 	(wp != NULL)  printf("%s", TEWindowName(wp));	// print window title
	printf("\n");
	fflush(stdout);
	return;
}


#endif

