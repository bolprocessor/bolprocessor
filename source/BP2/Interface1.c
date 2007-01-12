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

/* This is defined by both Carbon and non-Carbon prefix headers */
#if  !defined(TARGET_API_MAC_CARBON)
   /* so if it is not defined yet, there is no prefix file, 
      and we are compiling the "Transitional" build. */
   /* Use MacHeaders.h until ready to convert this file.
      Then change to MacHeadersTransitional.h. */
// #  include	"MacHeaders.h"
#  include	"MacHeadersTransitional.h"
#endif

#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"


MainEvent(void)
{
EventRecord event;
int eventfound,rep;
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

/*	Use this deferral mechanism of dealing with MIDI Manager in response
	to null events.  The appHook may be called at a level where the application's
	resources are not available.  But now the resources are available. */
if(Oms && !InitOn) {
	CheckSignInOrOutOfMIDIManager();
	if(gNodesChanged) {
		gNodesChanged = FALSE;
		if(gInputMenu != NULL) RebuildOMSDeviceMenu(gInputMenu);
		if((rep=InputMenuSideEffects()) != OK) return(rep);
		if(gOutputMenu != NULL) RebuildOMSDeviceMenu(gOutputMenu);
		if((rep=OutputMenuSideEffects()) != OK) return(rep);
		}
	}
			
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
eventfound = GetNextEvent(everyEvent,&event);
FindWindow(event.where,&whichwindow);
if(whichwindow == GetDialogWindow(FileSavePreferencesPtr)
	|| whichwindow == GetDialogWindow(TuningPtr)
	|| whichwindow == GetDialogWindow(DefaultPerformanceValuesPtr)
	|| whichwindow == GetDialogWindow(CsoundInstrMorePtr)) TEIdle(GetDialogTextEditHandle(GetDialogFromWindow(whichwindow)));
if(eventfound) return(DoEvent(&event));
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
	thechar = (char)(p_event->message & charCodeMask);
	if(Jcontrol != -1) {
		Jcontrol = -1; ReadKeyBoardOn = FALSE; HideWindow(Window[wMessage]);
		return(OK);
		}
	Option = FALSE;
	switch(thechar) {
		case -54:	/* cmd option space */
			return(mMiscSettings(Nw));
			break;
		case '©':	/* cmd option G */
		case 'ﬁ':
			Option = TRUE;
			thechar = 'g';
			break;
		case 'Ì':	/* cmd option H */
		case '˙':
			Option = TRUE;
			thechar = 'h';
			break;
		case ' ':
			if(iProto > 1 && iProto < Jbol && Nw >= wPrototype1 && Nw <= wPrototype7) {
				if((*p_MIDIsize)[iProto] > ZERO) return(PlayPrototype(iProto));
				else if((*p_CsoundSize)[iProto] > ZERO) DeCompileObjectScore(iProto);
				}
			else if((Oms || NEWTIMER) && (SoundOn || ComputeOn || PlaySelectionOn)
					&& OutMIDI && !PlayPrototypeOn) {
				Mute = 1 - Mute;
				MaintainMenus();
				BPActivateWindow(SLOW,wControlPannel);
				if(Mute) {
					sprintf(Message,"MUTE is ON…   cmd-space will turn if off");
					FlashInfo(Message);
					ShowMessage(TRUE,wMessage,Message);
					}
				else {
					HideWindow(Window[wInfo]);
					ClearMessage();
					}
				}
			break;
		case 'ø':	/* cmd option O */
		case 'œ':
			Option = TRUE;
			thechar = 'o';
			break;
		case 'æ':	/* cmd option A */
		case 'å':
			if(Finding) return(mFindAgain(Nw));
			break;
		case '´':	/* cmd option E */
		case 'ê':
			return(mEnterFind(Nw));
			break;
		case '†':	/* cmd option T */
			Token = 1 - Token;
			if(Token) sprintf(Message,"Type tokens enabled");
			else sprintf(Message,"Type tokens disabled");
			Alert1(Message);
			break;
		case 'È':	/* cmd option K */
		case '˚':
#if 0
			if(Beta) {
				if(Answer("Create a new key",'N') == YES) {
					rep = AnswerWith("Enter admin password…","\0",Message);
					if(rep != OK) break;
					if(strcmp("bell95",Message) != 0) {
						Alert1("Sorry, wrong password!");
						break;
						}
					MakeNewKeyFile(FALSE);
					}
				}
#endif
			break;
		case '?':
		case '/':
HELP:
			if(!Help) return(mHelp(Nw));
			else {
				Help = FALSE;
				DisplayHelp("Help");
				}
			return(OK);
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
	rep = DoCommand(Nw,MenuKey(thechar));
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
	case app4Evt:	/* Used by OMS */
		if(Oms && (p_event->message & 0x01000000)) {	/* suspend/resume */
			if(p_event->message & 0x00000001) {
				OMSResume('Bel0');
				// SetDriver();
				/* $$$ add whatever else to resume your application */
				}
			else {
				OMSSuspend('Bel0');
				/* $$$ add whatever else to suspend your application */
				}
			}
		break;
	case mouseDown:
		Panic = FALSE;
		if(!ScriptRecOn) SwitchOff(NULL,wScriptDialog,bRecordScript);
		theclick = FindWindow(p_event->where,&whichwindow);
		for(w=0; w < WMAX; w++) {
			if(whichwindow == Window[w]) break;
			}
		if(whichwindow == GetDialogWindow(MIDIprogramPtr) || whichwindow == GetDialogWindow(SixteenPtr))
			goto DOTHECLICK;
		if((w < 0) || (w >= WMAX) || (w == Nw)) goto DOTHECLICK;
		switch(w) {
			case wControlPannel:
			case wScriptDialog:
			case wPrototype1:
				goto DOTHECLICK;
				break;
			default:
				if(GrafWindow[w]) {
					if(w == wGraphic) ShowDuration(NO);
					BPActivateWindow(SLOW,w);
					}
				else BPActivateWindow(QUICK,w);
				rep = DoContent(whichwindow,p_event,&intext);
				goto END;
				break;
			}

DOTHECLICK:
		if(w == Nw && w == wGraphic) ShowDuration(NO);
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
				Help = FALSE;
				if(whichwindow == GetDialogWindow(DefaultPerformanceValuesPtr)) {
					if(GetDefaultPerformanceValues() != OK) return(OK);
					}
				if(whichwindow == GetDialogWindow(FileSavePreferencesPtr)) {
					if(GetFileSavePreferences() != OK) return(OK);
					}
				if(whichwindow == GetDialogWindow(OMSinoutPtr)) {
					rep = StoreDefaultOMSinput();
					ClearMessage();
					if(rep == EXIT) return(rep);
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
				Option = FALSE;
				if((p_event->modifiers & optionKey) != 0) {
					Option = TRUE;
					SetOptionMenu(TRUE);
					}
				k = MenuSelect(p_event->where);
				SetOptionMenu(FALSE);
				ResetTickFlag = TRUE;
				rep = DoCommand(Nw,k);
				if(ResumeStopOn) {
					if(UndoFlag) BringToFront(GetDialogWindow(ResumeUndoStopPtr));
					else BringToFront(GetDialogWindow(ResumeStopPtr));
					}
				return(rep);
				break;
			case inSysWindow:
				Help = FALSE;
				GetPort(&saveport);
				SetPortWindowPort(whichwindow);
				GetWindowPortBounds(whichwindow, &r);
				InvalRect(&r);
				r = LongRectToRect((*(TEH[LastEditWindow]))->viewRect);
				SetPortWindowPort(Window[LastEditWindow]);
				InvalRect(&r);
				if(saveport != NULL) SetPort(saveport);
				else if(Beta) Alert1("Err DoEvent(). saveport == NULL");
				SystemClick(p_event,whichwindow);
				ResetTickFlag = TRUE;
				if(0 <= w && w < WMAX) BPActivateWindow(SLOW,w);
				if(ResumeStopOn) {
					if(UndoFlag) BringToFront(GetDialogWindow(ResumeUndoStopPtr));
					else BringToFront(GetDialogWindow(ResumeStopPtr));
					}
				break;
			case inDrag:
				if(Oms && !InitOn && whichwindow == GetDialogWindow(OMSinoutPtr)) {
					if(gInputMenu != NULL) DrawOMSDeviceMenu(gInputMenu);
					if(gOutputMenu != NULL) DrawOMSDeviceMenu(gOutputMenu);
					}
				else {
					Jcontrol = -1; ReadKeyBoardOn = FALSE;
					}
				if(whichwindow != FrontWindow()
						&& GetDialogWindow(ResumeStopPtr) != FrontWindow()) {
					Help = FALSE;
					if(w >= 0 && w < WMAX) BPActivateWindow(SLOW,w);
					else SelectWindow(whichwindow);
					}
				else {
					if(w == Nw) {
						GetPort(&saveport);
						SetPortWindowPort(whichwindow);
						GetWindowPortBounds(whichwindow, &r);
						InvalRect(&r);
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
						if(saveport != NULL) SetPort(saveport);
						else if(Beta) Alert1("Err DoEvent(). saveport == NULL");
						if(w == wScript) BPActivateWindow(SLOW,wScriptDialog);
						if(w == wScriptDialog) BPActivateWindow(SLOW,wScript);
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
							GetPort(&saveport);
							SetPortWindowPort(whichwindow);
							GetWindowPortBounds(whichwindow, &r);
							InvalRect(&r);
							if(saveport != NULL) SetPort(saveport);
							else if(Beta) Alert1("Err DoEvent(). saveport == NULL");
							dragrect = Set_Window_Drag_Boundaries();
							DragWindow(whichwindow,p_event->where,&dragrect);
							if(saveport != NULL) SetPort(saveport);
							else if(Beta) Alert1("Err DoEvent(). saveport == NULL");
							}
						}
					}
				break;
			case inGrow:
				if(w == Nw /* && (!ClickRuleOn || w != wTrace) */) {
					if(MyGrowWindow(w,p_event->where) == OK) {
						if(w < WMAX) BPActivateWindow(SLOW,w);
						NewEnvironment = ChangedCoordinates[w] = TRUE;
						}
					}
				break;
			case inContent:
				Jcontrol = -1;
				if(LastAction == TYPEWIND || LastAction == TYPEDLG) LastAction = NO;
				if(GetDialogWindow(FAQPtr) == FrontWindow()) {
					if(whichwindow != FrontWindow()) {
						if(w < WMAX) {
							Help = FALSE;
							BPActivateWindow(SLOW,w);
							}
						else SysBeep(10);
						}
					else {
						Help = TRUE;
						DoContent(whichwindow,p_event,&intext);
						}
					break;	
					}
				if(Oms && !InitOn && whichwindow == GetDialogWindow(OMSinoutPtr)) {
					pt = p_event->where;ShowWindow(GetDialogWindow(OMSinoutPtr));
					SelectWindow(GetDialogWindow(OMSinoutPtr));
					if(gInputMenu != NULL) DrawOMSDeviceMenu(gInputMenu);
					if(gOutputMenu != NULL) DrawOMSDeviceMenu(gOutputMenu);
					SetPortWindowPort(whichwindow);
					GlobalToLocal(&pt);
					found = FALSE;
					if(gInputMenu != NULL && TestOMSDeviceMenu(gInputMenu,pt)) {
						if(ClickOMSDeviceMenu(gInputMenu)) {
							found = TRUE;
							if((rep=InputMenuSideEffects()) == EXIT) return(EXIT);
							}
						}
					else if(gOutputMenu != NULL && TestOMSDeviceMenu(gOutputMenu,pt)) {
						if(ClickOMSDeviceMenu(gOutputMenu)) {
							found = TRUE;
							OutputMenuSideEffects();
							}
						}
					if(!found) {
						if(gInputMenu != NULL) DrawOMSDeviceMenu(gInputMenu);
						if(gOutputMenu != NULL) DrawOMSDeviceMenu(gOutputMenu);
						}
					break;
					}
				if(whichwindow != FrontWindow()) {
					if(whichwindow == GetDialogWindow(ResumeStopPtr)
							|| whichwindow == GetDialogWindow(FileSavePreferencesPtr)
							|| whichwindow == GetDialogWindow(StrikeModePtr)	
							|| whichwindow == GetDialogWindow(TuningPtr)
							|| whichwindow == GetDialogWindow(DefaultPerformanceValuesPtr)
							|| whichwindow == GetDialogWindow(CsoundInstrMorePtr)
							|| whichwindow == GetDialogWindow(ResumeUndoStopPtr)
							|| whichwindow == GetDialogWindow(MIDIkeyboardPtr)
							|| whichwindow == GetDialogWindow(SixteenPtr)
							|| whichwindow == GetDialogWindow(MIDIprogramPtr)
							|| whichwindow == GetDialogWindow(gpDialogs[wControlPannel])
							|| whichwindow == GetDialogWindow(gpDialogs[wPrototype1])
							|| whichwindow == GetDialogWindow(gpDialogs[wScriptDialog])) {
						if(1 || !Help) {
							if(w >= 0 && w < WMAX) BPActivateWindow(QUICK,w);
							else {
								SelectWindow(whichwindow);
								UpdateWindow(FALSE,whichwindow);
								}
							}
						rep = DoDialog(p_event);
						if((rep == OK || rep == AGAIN) && w < WMAX) {
							BPActivateWindow(SLOW,w);
							}
						if(rep == DONE) rep = OK;
						return(rep);
						}
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
					else {
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
		if(thechar == '\5') goto HELP;
		if(thechar >= 0 && thechar < 10 && thechar != '\b') {
			SysBeep(10);
			break;
			}
		Help = FALSE;
		if(Nw >= 0 && Nw < WMAX) {
			ScriptKeyStroke(Nw,p_event);
			if(thechar == '\b') {
				if(!WASTE && Editable[Nw] && !LockedWindow[Nw]) {
					if((start=(**(TEH[Nw])).selStart) < (**(TEH[Nw])).selEnd) {
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
						if((start=(**(TEH[Nw])).selStart) > 0 && LastAction != COPY
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
			if(thechar == '\r') thechar = '¬';
			}
		/* Can't be command key */
		if(Nw >= 0 && Nw < WMAX) TypeChar((int)thechar,(int)(p_event->modifiers & shiftKey));
		break;
	case activateEvt:
		TEFromScrap();
		UpdateWindow(TRUE,(WindowPtr)p_event->message);
		for(w=0; w < WMAX; w++) {
			if((WindowPtr) p_event->message == Window[w]) break;
			}
		if(w >= 0 && w < WMAX) {
			if(w == Nw && OKvScroll[w]) {
				GetPort(&saveport);
				SetPortWindowPort(Window[w]);
				GetWindowPortBounds(Window[w], &r);
				InvalRect(&r);
				if(saveport != NULL) SetPort(saveport);
				else if(Beta) Alert1("Err DoEvent(). saveport == NULL");
				}
			if(p_event->modifiers & activeFlag) {
				if(Editable[w] && !LockedWindow[w]) Activate(TEH[w]);
				if(HasFields[w]) TEActivate(GetDialogTextEditHandle(gpDialogs[w]));
				if(OKvScroll[w]) ShowControl(vScroll[w]);
				if(OKhScroll[w]) ShowControl(hScroll[w]);
		/*		DisableItem(myMenus[editM],undoCommand); */
				}
			else {
				if(w != Nw) {
					if(Editable[w] && !LockedWindow[w]) Deactivate(TEH[w]);
					if(HasFields[w]) TEDeactivate(GetDialogTextEditHandle(gpDialogs[w]));
					if(OKvScroll[w]) HideControl(vScroll[w]);
					if(OKhScroll[w]) HideControl(hScroll[w]);
					}
				}
			}
		break;
	case updateEvt:
		TEFromScrap();
		UpdateWindow(FALSE,(WindowPtr)p_event->message);
		break;
	default: ;
	}
rep = OK;

END:
SetResumeStop(FALSE);
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
	if(reset) {
		switch(w) {
			case wCsoundTables:
				sprintf(line,"f1 0 256 10 1 ; This table may be changed\r\r");
				break;
			case wData:
				sprintf(line,"%s<alphabet>\r%s<interactive code file>\r%s<settings file>\r%s<glossary file>\r%s<time base file>\r%s<MIDI orchestra file>\r<Generate or type items here>\r",
					FilePrefix[wAlphabet],FilePrefix[wInteraction],
					FilePrefix[iSettings],FilePrefix[wGlossary],FilePrefix[wTimeBase],
					FilePrefix[wMIDIorchestra]);
				break;
			case wGrammar:
				sprintf(line,"%s<alphabet>\r%s<interactive code file>\r%s<settings file>\r%s<glossary file>\r%s<time base file>\r%s<MIDI orchestra file>\r\r// Put grammar rules here\r\rCOMMENT:\rThis is an empty grammar…",
					FilePrefix[wAlphabet],FilePrefix[wInteraction],
					FilePrefix[iSettings],FilePrefix[wGlossary],FilePrefix[wTimeBase],
					FilePrefix[wMIDIorchestra]);
				break;
			case wAlphabet:
				sprintf(line,"%s<sound-object file>\r%s<keyboard file>\r%s<interactive code file>\r%s<MIDI orchestra file>\r",
					FilePrefix[iObjects],FilePrefix[wKeyboard],FilePrefix[wInteraction],
					FilePrefix[wMIDIorchestra]);
				break;
			case wInteraction:
				sprintf(line,"BP2 script\r// Here you may write a script fixing the interactive environment (IN... instructions) or load a ‘%s’ interactive code file.\r",
					FilePrefix[wInteraction]);
				break;
			case wGlossary:
				sprintf(line,"BP2 script\r// Here you may write the glossary (‘Define…’ instructions) or load a ‘%s’ glossary file.\r",
					FilePrefix[wGlossary]);
				break;
			case wScript:
				sprintf(line,"BP2 script\r// Here you may write a script (See ‘Script’ menu) or load a ‘%s’ script file.\r",
					FilePrefix[wScript]);
				break;
			case wStartString:
				sprintf(line,"S\r");
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
				SetSelect((long) strlen("BP2 script\r"),count,TEH[w]);
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
	r = LongRectToRect((**(TEH[w])).viewRect);
	InvalRect(&r);
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
		if(ReleaseObjectPrototypes() != OK) {
			rep = FAILED; goto OUT;
			}
		break;
	case wCsoundTables:
		Dirty[wCsoundInstruments] = Created[wCsoundInstruments] = FALSE;
		break;
	case wCsoundInstruments:
		ClearWindow(YES,wCsoundTables);
		ResizeCsoundInstrumentsSpace(1);
		if(newinstruments) ResetCsoundInstrument(iCsoundInstrument,YES);
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
				GetDialogItem((DialogPtr)MIDIprogramPtr,(short)CurrentMIDIprogram[TestMIDIChannel],
					&itemtype,(Handle*)&itemhandle,&r);
				HiliteControl((ControlHandle) itemhandle,0);
				}
			GetDialogItem((DialogPtr)SixteenPtr,(short)button1 + TestMIDIChannel - 1,
				&itemtype,(Handle*)&itemhandle,&r);
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
				if((InBuiltDriverOn || Oms) && !InitOn)
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
if((LastAction == TYPEWIND || LastAction == TYPEDLG)
	&& (Editable[newNw] || HasFields[newNw]) && Nw != newNw) LastAction = NO;
if(TypeScript && ScriptRecOn && Nw != newNw) {
	TypeScript = FALSE;
	PrintBehind(wScript,"\r");
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
	if(!InBuiltDriverOn && !Oms) {
		if(Answer("The MIDI driver is not open, i.e. BP2 can't receive MIDI messages.\rDo you want to open it?",
			'Y') == OK) {
			MIDI = OutMIDI = Dirty[iSettings] = TRUE; SetButtons(TRUE);
			ResetMIDI(FALSE);
			Alert1("The ‘Interactive’ option has been set ON.\rSee the “Settings” dialog if you don't want BP2 to receive messages");
			}
		}
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
	if(HasFields[Nw]) {
		if(Nw != newNw) TEDeactivate(GetDialogTextEditHandle(gpDialogs[Nw]));
		else TEActivate(GetDialogTextEditHandle(gpDialogs[Nw]));
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
		if(OKgrow[Nw]) DrawGrowIcon(Window[Nw]);
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

if(Nw < 0 || Nw >= WMAX) return(OK);

if(!WASTE) UpdateWindow(FALSE,Window[Nw]);
else if(mode == SLOW && Editable[Nw]) ShowSelect(CENTRE,Nw);
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
	oldScroll = (**(TEH[w])).viewRect.top - (**(TEH[w])).destRect.top;
	newScroll = GetControlValue(vScroll[w]) * LineHeight(w);
	delta = oldScroll - newScroll;
	if(delta != 0) {
		TextScroll(0,delta,TEH[w]);
		r = LongRectToRect((**(TEH[w])).viewRect);
/*		ValidRect(&r); */
		}
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
long selstart,selend;
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
				ValidRect(&r1);
				if(EmergencyExit) return(ABORT);
				}
			}
		}
	else {	// FIXME ? Should we call LockPixels before CopyBits?
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

if(HasFields[w]) TEActivate(GetDialogTextEditHandle(gpDialogs[w]));
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
	if(HasFields[w]) InsetRect(&dr,2,2);
	if(OKvScroll[w]) {
		dr.right -= SBARWIDTH;
		dr.bottom = dr.bottom - SBARWIDTH - 1 - Freebottom[w];
		}
	linesInFolder[w] = (dr.bottom - dr.top - 2) / LineHeight(w);
	if(linesInFolder[w] > 0) {
		dr.bottom = dr.top + 1 + LineHeight(w) * linesInFolder[w];
		}
	else linesInFolder[w] = 1;
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

for(w=0; w < WMAX; w++) {
	if(theWindow == Window[w]) break;
	}
GetPort(&saveport);
SetPortWindowPort(theWindow);
if(!quick && w < WMAX && (!WASTE || !Editable[w])) SetViewRect(w);

// save clipping rectangle
cliprgn = NewRgn();	// FIXME: should check return value; is it OK to move memory here?
GetPortClipRegion(GetWindowPort(theWindow), cliprgn);
GetRegionBounds(cliprgn, &r1);

GetWindowPortBounds(theWindow, &r);
ClipRect(&r);
if(w < WMAX && GrafWindow[w] && (!quick || GotAlert)) ShowSelect(CENTRE,w);
BeginUpdate(theWindow);	/* This should not be placed higher */
UpdateControls(theWindow, cliprgn);
DisposeRgn(cliprgn);

if(w < WMAX) {
	if(IsDialog[w]) { 
		GrafPtr port;
		RgnHandle rgn;
		port = GetDialogPort(gpDialogs[w]);
		rgn = NewRgn();	// FIXME: should check return value; is it OK to move memory here?
		GetPortVisibleRegion(port, rgn);
		UpdateDialog(gpDialogs[w], rgn);
		DisposeRgn(rgn);
		}
	/* UpdateDialog needed to make static text visible */
	/* … in spite of Inside Mac saying it isn't p.I-418 */
	if(OKgrow[w]) DrawGrowIcon(theWindow);
	if(Editable[w]) TextUpdate(w);
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
		RgnHandle rgn;
		DialogPtr dp;
		if(Oms && !InitOn && theWindow == GetDialogWindow(OMSinoutPtr)) {
			if(gInputMenu != NULL) DrawOMSDeviceMenu(gInputMenu);
			if(gOutputMenu != NULL) DrawOMSDeviceMenu(gOutputMenu);
			}
		dp = GetDialogFromWindow(theWindow);
		rgn = NewRgn();	// FIXME: should check return value; is it OK to move memory here?
		GetPortVisibleRegion(GetDialogPort(dp), rgn);
		UpdateDialog(dp, rgn);
		DisposeRgn(rgn);
		}
	}
EndUpdate(theWindow);
ClipRect(&r1);
if(w < WMAX && HasFields[w] && !Editable[w]) HiliteDefault(theWindow);
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

if(Nw < 0 || Nw >= WMAX) return ;
if(Editable[Nw]) {
	teh = TEH[Nw];
	pageSize = ((**teh).viewRect.bottom-(**teh).viewRect.top) / 
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
UniversalProcPtr vscrollptr,hscrollptr; /* ControlActionUPP */

for(w=0; w < WMAX; w++) {
	if(theWindow == Window[w]) break;
	}
if(Help && w < WMAX) UpdateWindow(FALSE,Window[w]);
GetPort(&saveport);
SetPort(theWindow);
GlobalToLocal(&p_event->where);

vscrollptr = NewRoutineDescriptor((ProcPtr)vScrollProc,uppControlActionProcInfo,
	GetCurrentISA());
hscrollptr = NewRoutineDescriptor((ProcPtr)hScrollProc,uppControlActionProcInfo,
	GetCurrentISA());

(*p_intext) = FALSE;
	
if(w < WMAX && Editable[w]) {
	if((cntlCode = FindControl(p_event->where,theWindow,&theControl)) == 0) {
		MyLock(FALSE,(Handle)TEH[w]);
		r = LongRectToRect((**(TEH[w])).viewRect);
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
		else TrackControl(theControl,p_event->where,(ControlActionUPP)vscrollptr);
	/*	InvalRect(&r); */
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
			SelectWindow(Window[w]);
			GetPort(&saveport);
			SetPort(Window[w]);
			r1 = (*(Window[w]->clipRgn))->rgnBBox;
			r = Window[w]->portRect;
			ClipRect(&r);
			if(cntlCode == inThumb) {
				TrackControl(theControl,p_event->where,(ControlActionUPP)0L);
				}
			else {
				if(OKhScroll[w] && theControl == hScroll[w])
					TrackControl(theControl,p_event->where,(ControlActionUPP)hscrollptr);
				if(OKvScroll[w] && theControl == vScroll[w])
					TrackControl(theControl,p_event->where,(ControlActionUPP)vscrollptr);
				}
			InvalRect(&r);
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
SetPort(Window[w]);
r = Window[w]->portRect;
InvalRect(&r);
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
GrafPtr saveport;
long theresult;
Rect r,r0;
int result;

if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. MyGrowWindow() ");
	return(OK);
	}
result = OK;
if(!OKgrow[w]) return(FAILED);
GetPort(&saveport);
SetPort(Window[w]);
SetRect(&r,MINWINDOWHEIGHT,MINWINDOWWIDTH,
	screenBits.bounds.right - screenBits.bounds.left,
	screenBits.bounds.bottom - screenBits.bounds.top - SBARWIDTH);
theresult = GrowWindow(Window[w],p,&r);
if(theresult == 0) return(FAILED);
r0 = Window[w]->portRect;
SizeWindow(Window[w],LoWord(theresult),HiWord(theresult),TRUE);
r = Window[w]->portRect;
if(r.top == r0.top && r.left == r0.left && r.bottom == r0.bottom
		&& r.right == r0.right) {
	result = FAILED;
	goto QUIT;
	}
ClipRect(&r);
EraseRect(&r);
InvalRect(&r);
SetViewRect(w);
HidePen();
if(OKvScroll[w]) {
	MoveControl(vScroll[w],r.right - SBARWIDTH,r.top - 1);
	SizeControl(vScroll[w],SBARWIDTH + 1,r.bottom - r.top - (SBARWIDTH - 2)
		- Freebottom[w]);
	}
if(OKhScroll[w]) {
	MoveControl(hScroll[w],r.left - 1,r.bottom - SBARWIDTH);
	SizeControl(hScroll[w],r.right - r.left - (SBARWIDTH - 2),SBARWIDTH + 1);
	r.bottom -= SBARWIDTH;
	}
if(OKvScroll[w]) {
	r.right -= SBARWIDTH;
	SetVScroll(w);
	}
ShowPen();
if(GrafWindow[w]) {
	SetMaxControlValues(w,r);
	if(OKvScroll[w]) {
		SetControlValue(vScroll[w],0);
		AdjustGraph(FALSE,w,vScroll[w]);
		}
	if(OKhScroll[w]) {
		SetControlValue(hScroll[w],0);
		AdjustGraph(FALSE,w,hScroll[w]);
		}
	SlideH[w] = SlideV[w] = 0;
/*	ClipRect(&r); */
	r = Window[w]->portRect;
	InvalRect(&r);
	UpdateWindow(FALSE,Window[w]);  /* 5/9/97 */
	}
AdjustTextInWindow(w);

QUIT:
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err MyGrowWindow(). saveport == NULL");
return(result);
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
		&& w != wPrototype7) {
	if(FileName[w][0] == '\0') strcpy(Message,DeftName[w]);
	else strcpy(Message,FileName[w]);
	SetWTitle(Window[w],c2pstr(Message));
	strcpy(Message," ");
	/* This avoids messing the menu as name starts with '-' */
	if(FileName[w][0] == '\0') strcpy(Message,WindowName[w]);
	else strcat(Message,FileName[w]);
	pStrCopy((char*)c2pstr(Message),PascalLine);
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
else {
	strcpy(Message,DeftName[w]);
	if(Message[0] != '\0') SetWTitle(Window[w],c2pstr(Message));
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
strcpy(line1,s); strcpy(line2,prefix);
origin = (**(TEH[w])).selStart; end = (**(TEH[w])).selEnd;
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
PrintBehindln(w,s);
SetSelect(origin+count1+1L,end+count1+1L,TEH[w]);
if(wasempty) Dirty[w] = dirtymem;
return(OK);

FOUND:	/* but it was different name */
SetSelect(posinit,(pos - 1L),TEH[w]);
TextDelete(w);
PrintBehind(w,s);
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
origin = (**(TEH[w])).selStart; end = (**(TEH[w])).selEnd;
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
	case wTimeBase:
		RemoveFirstLine(wGrammar,FilePrefix[w]);
		RemoveFirstLine(wData,FilePrefix[w]);
		break;
	case wKeyboard:
	case iObjects:
	case wCsoundInstruments:
	case wMIDIorchestra:
		RemoveFirstLine(wAlphabet,FilePrefix[w]);
		RemoveFirstLine(wData,FilePrefix[w]);
		RemoveFirstLine(wGrammar,FilePrefix[w]);
		if(w != wKeyboard) SetField(NULL,wPrototype1,fInstrumentFileName,"[no file]");
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
if(!ScriptExecOn || ResumeStopOn) SetCursor(&arrow);
/* else TurnWheel(); */
return(OK);
}


IsEmpty(int w)
{
int j,len,gap,rep;
char c,*q,**p_line,line[MAXLIN];
long i,pos,posmax;

if(w >= 0 && w < WMAX && Editable[w]) {
	posmax = GetTextLength(w);
	if(w == wPrototype7) {
		for(i=0; i < posmax; i++) {
			if(!isspace(GetTextChar(w,i))) return(FALSE);
			}
		return(TRUE);
		}
	
	p_line = NULL;
	pos = ZERO; rep = TRUE;
	
	if(w == wScript || w == wGlossary || w == wInteraction) {
		MystrcpyHandleToString(MAXLIN,0,line,p_ScriptLabelPart(110,0));
		/* line is "BP2 script" */
		}
	if(w == wGrammar) strcpy(line,"COMMENT:");
	len = strlen(line);
	while(ReadLine(YES,w,&pos,posmax,&p_line,&gap) == OK) {
		if((*p_line)[0] == '\0' || (*p_line)[0] == '\r') continue;
		for(j=0; j < WMAX; j++) {
			if(FilePrefix[j][0] == '\0') continue;
			q = &(FilePrefix[j][0]);
			if(Match(TRUE,p_line,&q,4)) goto NEXTLINE;
			}
		if(w == wScript || w == wGlossary || w == wInteraction) {
			/* Discard line */
			q = line;
			if(Match(FALSE,p_line,&q,len)) goto NEXTLINE;
			}
		if(w == wGrammar) {
			q = line;
			if(Match(FALSE,p_line,&q,len)) break;
			}
		rep = FALSE;
		break;
NEXTLINE: ;
		}
	MyDisposeHandle((Handle*)&p_line);
	return(rep);
	}
else return(FALSE);
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
	DrawMenuBar();	/* Needed to update the “save” command */
	}
return(OK);
}


MaintainCursor(void)
{
Point pt;
Rect r;
WindowPtr wPtr;
GrafPtr saveport;
int found;

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
GetPort(&saveport);
wPtr = FrontWindow();
SetPort((GrafPtr)wPtr);
GetMouse(&pt);
if(Nw > -1 && Nw < WMAX && Ours(wPtr,Window[Nw])) {
	if(Editable[Nw] && !LockedWindow[Nw]) {
		r = LongRectToRect((**(TEH[Nw])).viewRect);
		if(PtInRect(pt,&r)) {
			SetCursor(&EditCursor);
			goto OUT;
			}
		}
	if(HasFields[Nw]) {
		r = (**(((DialogPeek)gpDialogs[Nw])->textH)).viewRect;
		if(PtInRect(pt,&r)) {
			SetCursor(&EditCursor);
			goto OUT;
			}
		}
	}
else {
	found = FALSE;
	if(wPtr == EnterPtr) {
		r = (*(((DialogPeek)EnterPtr)->textH))->viewRect;
		found = TRUE;
		}
	if(wPtr == TuningPtr) {
		r = (*(((DialogPeek)TuningPtr)->textH))->viewRect;
		found = TRUE;
		}
	if(wPtr == DefaultPerformanceValuesPtr) {
		r = (*(((DialogPeek)DefaultPerformanceValuesPtr)->textH))->viewRect;
		found = TRUE;
		}
	if(wPtr == CsoundInstrMorePtr) {
		r = (*(((DialogPeek)CsoundInstrMorePtr)->textH))->viewRect;
		found = TRUE;
		}
	if(wPtr == PatternPtr) {
		r = (*(((DialogPeek)PatternPtr)->textH))->viewRect;
		found = TRUE;
		}
	if(wPtr == FileSavePreferencesPtr) {
		r = (*(((DialogPeek)FileSavePreferencesPtr)->textH))->viewRect;
		found = TRUE;
		}
	if(wPtr == MIDIprogramPtr) {
		r = (*(((DialogPeek)MIDIprogramPtr)->textH))->viewRect;
		found = TRUE;
		}
	if(found && PtInRect(pt,&r)) {
		SetCursor(&EditCursor);
		goto OUT;
		}
	}
SetCursor(&arrow);

OUT:
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err MaintainCursor(). saveport == NULL");
return(OK);
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
long dif;
Handle itemhandle;
int w;

if(InitOn) return(OK);

if(Nw < 0) w = LastEditWindow;
else w = FindGoodIndex(Nw);

if(Oms) {
	EnableItem(myMenus[deviceM],OMSmidiCommand);
	EnableItem(myMenus[deviceM],OMSstudioCommand);
	EnableItem(myMenus[deviceM],OMSinoutCommand);
	CheckItem(myMenus[deviceM],outOMSCommand,TRUE);
	}
else {
	DisableItem(myMenus[deviceM],OMSmidiCommand);
	DisableItem(myMenus[deviceM],OMSstudioCommand);
	DisableItem(myMenus[deviceM],OMSinoutCommand);
	CheckItem(myMenus[deviceM],outOMSCommand,FALSE);
	}
	
if(UndoFlag || LastAction != NO)
	EnableItem(myMenus[editM],undoCommand);
else
	DisableItem(myMenus[editM],undoCommand);
if(UseGraphicsColor)
	CheckItem(myMenus[layoutM],graphicsColorCommand,TRUE);
else
	CheckItem(myMenus[layoutM],graphicsColorCommand,FALSE);
if(SmartCursor)
	CheckItem(myMenus[miscM],smartcursorCommand,TRUE);
else
	CheckItem(myMenus[miscM],smartcursorCommand,FALSE);
if(UseTextColor)
	CheckItem(myMenus[layoutM],textColorCommand,TRUE);
else
	CheckItem(myMenus[layoutM],textColorCommand,FALSE);
if(ReadKeyBoardOn)
	CheckItem(myMenus[actionM],typenoteCommand,TRUE);
else
	CheckItem(myMenus[actionM],typenoteCommand,FALSE);

if(UseBullet)
	CheckItem(myMenus[layoutM],bulletCommand,TRUE);
else
	CheckItem(myMenus[layoutM],bulletCommand,FALSE);

if(ShowPianoRoll)
	CheckItem(myMenus[layoutM],pianorollCommand,TRUE);
else
	CheckItem(myMenus[layoutM],pianorollCommand,FALSE);
	
switch(KeyboardType) {
	case AZERTY:
		CheckItem(myMenus[layoutM],azertyCommand,TRUE);
		CheckItem(myMenus[layoutM],qwertyCommand,FALSE);
		break;
	case QWERTY:
		CheckItem(myMenus[layoutM],azertyCommand,FALSE);
		CheckItem(myMenus[layoutM],qwertyCommand,TRUE);
		break;
	}
if(!ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn
		&& !SetTimeOn && !GraphicOn && !PrintOn && !ScriptExecOn) {
	EnableItem(myMenus[fileM],fmNewProject);
	EnableItem(myMenus[fileM],fmLoadProject);
	EnableItem(myMenus[fileM],fmReceiveMIDI);
	EnableItem(myMenus[fileM],fmLoadPattern);
	EnableItem(myMenus[actionM],produceCommand);
	EnableItem(myMenus[actionM],analyzeCommand);
//	ShowPannel(wControlPannel,dProduceItems);
	DisableItem(myMenus[actionM],resumeCommand);
	DisableItem(myMenus[actionM],stopCommand);
	DisableItem(myMenus[actionM],pauseCommand);
	}
else {
	DisableItem(myMenus[fileM],fmNewProject);
	DisableItem(myMenus[fileM],fmLoadProject);
	DisableItem(myMenus[fileM],fmReceiveMIDI);
	DisableItem(myMenus[fileM],fmLoadPattern);
	DisableItem(myMenus[actionM],produceCommand);
	DisableItem(myMenus[actionM],analyzeCommand);
//	HidePannel(wControlPannel,dProduceItems);
	EnableItem(myMenus[actionM],resumeCommand);
	EnableItem(myMenus[actionM],stopCommand);
	EnableItem(myMenus[actionM],pauseCommand);
	}
if(PauseOn) DisableItem(myMenus[actionM],pauseCommand);
EnableItem(myMenus[editM],graphicCommand);
EnableItem(myMenus[editM],interactionCommand);
EnableItem(myMenus[actionM],adjustTimeBaseCommand);
EnableItem(myMenus[miscM],randomizeCommand);
EnableItem(myMenus[miscM],accuracyCommand);
EnableItem(myMenus[miscM],buffersizeCommand);
EnableItem(myMenus[miscM],graphicsettingsCommand);
EnableItem(myMenus[miscM],defaultPerformanceValuesCommand);
EnableItem(myMenus[miscM],defaultStrikeModeCommand);
EnableItem(myMenus[miscM],defaultFileSavePreferencesCommand);
EnableItem(myMenus[miscM],tuningCommand);
EnableItem(myMenus[editM],pasteCommand);
DisableItem(myMenus[searchM],findagainCommand);
DisableItem(myMenus[fileM],fmRevert);
if(Nw >= wRandomSequence && Nw != wScriptDialog) {
	DisableItem(myMenus[fileM],fmOpen);
	DisableItem(myMenus[fileM],fmClearWindow);
	DisableItem(myMenus[fileM],fmSave);
	DisableItem(myMenus[fileM],fmSaveAs);
	}
if((Nw < 0) || (Nw >= WMAX) || !IsWindowVisible(Window[Nw])
		|| (!Editable[Nw] && !HasFields[Nw] && Nw != wScriptDialog)
		|| (Nw == wInfo) || (Nw == wGraphic) || (Nw == wMessage)) {
	DisableItem(myMenus[fileM],fmOpen);
	DisableItem(myMenus[fileM],fmClearWindow);
	DisableItem(myMenus[fileM],fmSave);
	DisableItem(myMenus[fileM],fmSaveAs);
	DisableItem(myMenus[editM],cutCommand);
	DisableItem(myMenus[editM],copyCommand);
	DisableItem(myMenus[editM],pasteCommand);
	DisableItem(myMenus[editM],clearCommand);
	}
else {
	if(Finding) EnableItem(myMenus[searchM],findagainCommand);
	if(!ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn
			&& !SetTimeOn && !GraphicOn && !PrintOn && !ScriptExecOn) {
		EnableItem(myMenus[fileM],fmOpen);
		EnableItem(myMenus[fileM],fmClearWindow);
		EnableItem(myMenus[fileM],fmGoAway);
		}
	else {
		DisableItem(myMenus[fileM],fmOpen);
		DisableItem(myMenus[fileM],fmClearWindow);
		DisableItem(myMenus[fileM],fmGoAway);
		}
		
	EnableItem(myMenus[fileM],fmSaveAs);
	
	if(((!LockedWindow[w] && FileName[w][0] != '\0'
			&& (Dirty[w] || (w == wPrototype7 && Dirty[iObjects])))
			|| (CsoundInstrMorePtr == FrontWindow() && Dirty[wCsoundInstruments]
			&& FileName[wCsoundInstruments][0] != '\0')) && !ClickRuleOn)
		EnableItem(myMenus[fileM],fmRevert);
		
	if(((Dirty[w] || (w == wPrototype7 && Dirty[iObjects]))
			|| (CsoundInstrMorePtr == FrontWindow() && Dirty[wCsoundInstruments]))
			&& !ClickRuleOn) {
		EnableItem(myMenus[fileM],fmSave);
		}
	else {
		DisableItem(myMenus[fileM],fmSave);
		}
	dif = ZERO;
	if(Nw >= 0 && Nw < WMAX && Editable[Nw]) {
		dif = (**(TEH[Nw])).selEnd - (**(TEH[Nw])).selStart;
		}
	else {
		if(Nw >= 0 && Nw < WMAX) dif = (long) HasFields[Nw];
		}
	if((Nw == wTrace && ClickRuleOn) || Nw < 0 || Nw >= WMAX
							|| (!Editable[Nw] && !HasFields[Nw]))
			DisableItem(myMenus[editM],pasteCommand);
	else 	EnableItem(myMenus[editM],pasteCommand);
	if(dif == ZERO || (Nw == wTrace && ClickRuleOn)) {
		DisableItem(myMenus[editM],cutCommand);
		DisableItem(myMenus[editM],copyCommand);
		DisableItem(myMenus[editM],clearCommand);
		DisableItem(myMenus[searchM],enterfindCommand);
		}
	else {
		EnableItem(myMenus[editM],cutCommand);
		EnableItem(myMenus[editM],copyCommand);
		EnableItem(myMenus[editM],clearCommand);
		EnableItem(myMenus[searchM],enterfindCommand);
		}
	}
if(Nw >= 0 && Nw < WMAX && (Editable[Nw] || GrafWindow[Nw]))
	EnableItem(myMenus[fileM],fmPrint);
else
	DisableItem(myMenus[fileM],fmPrint);
DisableItem(myMenus[actionM],analyzeCommand);
if(((**(TEH[LastEditWindow])).selEnd
		- (**(TEH[LastEditWindow])).selStart > 0)
		&& !ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn
		&& !GraphicOn && !SetTimeOn && !PrintOn && !ScriptExecOn) {
	EnableItem(myMenus[actionM],playCommand);
	ShowPannel(wControlPannel,dPlaySelection);
	EnableItem(myMenus[actionM],expandCommand);
	EnableItem(myMenus[actionM],showPeriodsCommand);
	ShowPannel(wControlPannel,dExpandSelection);
	ShowPannel(wControlPannel,dShowPeriods);
	ShowPannel(wControlPannel,dAnalyze);
	ShowPannel(wControlPannel,bCaptureSelection);
	EnableItem(myMenus[actionM],analyzeCommand);
	}
else {
	DisableItem(myMenus[actionM],playCommand);
	HidePannel(wControlPannel,dPlaySelection);
	DisableItem(myMenus[actionM],expandCommand);
	DisableItem(myMenus[actionM],showPeriodsCommand);
	HidePannel(wControlPannel,dExpandSelection);
	HidePannel(wControlPannel,dShowPeriods);
	HidePannel(wControlPannel,dAnalyze);
	HidePannel(wControlPannel,bCaptureSelection);
	}
if(CompileOn || (CompiledGr && (CompiledGl || !LoadedGl)) || ClickRuleOn) {
	DisableItem(myMenus[actionM],compileCommand);
	}
else EnableItem(myMenus[actionM],compileCommand);

if(Oms) {
	CheckItem(myMenus[deviceM],modemportCommand,FALSE);
	CheckItem(myMenus[deviceM],printerportCommand,FALSE);
	DisableItem(myMenus[deviceM],modemportCommand);
	DisableItem(myMenus[deviceM],printerportCommand);
	}
else {
	EnableItem(myMenus[deviceM],modemportCommand);
	EnableItem(myMenus[deviceM],printerportCommand);
	if(Port == 1) {
		CheckItem(myMenus[deviceM],modemportCommand,TRUE);
		CheckItem(myMenus[deviceM],printerportCommand,FALSE);
		}
	else {
		CheckItem(myMenus[deviceM],modemportCommand,FALSE);
		CheckItem(myMenus[deviceM],printerportCommand,TRUE);
		}
	}
if(OutMIDI) CheckItem(myMenus[deviceM],outMIDICommand,TRUE);
else CheckItem(myMenus[deviceM],outMIDICommand,FALSE);
if(WriteMIDIfile) CheckItem(myMenus[deviceM],outMIDIfileCommand,TRUE);
else CheckItem(myMenus[deviceM],outMIDIfileCommand,FALSE);
if(OutCsound) CheckItem(myMenus[deviceM],outCsoundCommand,TRUE);
else CheckItem(myMenus[deviceM],outCsoundCommand,FALSE);
switch(NoteConvention) {
	case ENGLISH:
		CheckItem(myMenus[miscM],englishconventionCommand,TRUE);
		CheckItem(myMenus[miscM],frenchconventionCommand,FALSE);
		CheckItem(myMenus[miscM],indianconventionCommand,FALSE);
		CheckItem(myMenus[miscM],keyconventionCommand,FALSE);
		break;
	case FRENCH:
		CheckItem(myMenus[miscM],frenchconventionCommand,TRUE);
		CheckItem(myMenus[miscM],englishconventionCommand,FALSE);
		CheckItem(myMenus[miscM],indianconventionCommand,FALSE);
		CheckItem(myMenus[miscM],keyconventionCommand,FALSE);
		break;
	case INDIAN:
		CheckItem(myMenus[miscM],indianconventionCommand,TRUE);
		CheckItem(myMenus[miscM],frenchconventionCommand,FALSE);
		CheckItem(myMenus[miscM],englishconventionCommand,FALSE);
		CheckItem(myMenus[miscM],keyconventionCommand,FALSE);
		break;
	case KEYS:
		CheckItem(myMenus[miscM],keyconventionCommand,TRUE);
		CheckItem(myMenus[miscM],frenchconventionCommand,FALSE);
		CheckItem(myMenus[miscM],indianconventionCommand,FALSE);
		CheckItem(myMenus[miscM],englishconventionCommand,FALSE);
		break;
	}
DisableItem(myMenus[layoutM],f9Command);
DisableItem(myMenus[layoutM],f10Command);
DisableItem(myMenus[layoutM],f12Command);
DisableItem(myMenus[layoutM],f14Command);
DisableItem(myMenus[editM],selectallCommand);
if(Nw >= 0 && Nw < WMAX
			&& (Editable[Nw] || Nw == wGraphic)) {
	EnableItem(myMenus[layoutM],f9Command);
	EnableItem(myMenus[layoutM],f10Command);
	EnableItem(myMenus[layoutM],f12Command);
	EnableItem(myMenus[layoutM],f14Command);
	CheckItem(myMenus[layoutM],f9Command,FALSE);
	CheckItem(myMenus[layoutM],f10Command,FALSE);
	CheckItem(myMenus[layoutM],f12Command,FALSE);
	CheckItem(myMenus[layoutM],f14Command,FALSE);
	switch(WindowTextSize[Nw]) {
		case 9:
			CheckItem(myMenus[layoutM],f9Command,TRUE); break;
		case 10:
			CheckItem(myMenus[layoutM],f10Command,TRUE); break;
		case 12:
			CheckItem(myMenus[layoutM],f12Command,TRUE); break;
		case 14:
			CheckItem(myMenus[layoutM],f14Command,TRUE); break;
		}
	}
if(Nw >= 0 && Nw < WMAX && (Editable[Nw] || HasFields[Nw])) {
	if(Editable[Nw]) EnableItem(myMenus[searchM],0);
	EnableItem(myMenus[editM],selectallCommand);
	}
if(SplitTimeObjects) CheckItem(myMenus[layoutM],splitTimeObjectCommand,TRUE);
else CheckItem(myMenus[layoutM],splitTimeObjectCommand,FALSE);
if(SplitVariables) CheckItem(myMenus[layoutM],splitVariableCommand,TRUE);
else CheckItem(myMenus[layoutM],splitVariableCommand,FALSE);
if(Token) {
	CheckItem(myMenus[miscM],tokenCommand,TRUE);
	CheckItem(myMenus[miscM],textCommand,FALSE);
	SwitchOn(NULL,wKeyboard,dToken);
	}
else {
	CheckItem(myMenus[miscM],tokenCommand,FALSE);
	CheckItem(myMenus[miscM],textCommand,TRUE);
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
DisableItem(myMenus[actionM],templatesCommand);
if(CompiledGr && Gram.trueBP) {
	EnableItem(myMenus[actionM],templatesCommand);
	ShowPannel(wControlPannel,dTemplates);
	}
else {
/*	HidePannel(wControlPannel,dTemplates); */
	DisableItem(myMenus[actionM],templatesCommand);
	}
EnableItem(myMenus[searchM],listterminalsCommand);
EnableItem(myMenus[searchM],listvariablesCommand);
return(OK);
}


Rect Set_Window_Drag_Boundaries(void)     
{           
RgnHandle  the_gray_rgn;
Rect r;
   
the_gray_rgn = GetGrayRgn();
r = (**(the_gray_rgn)).rgnBBox;  
r.left   += DRAG_EDGE;   
r.right  -= DRAG_EDGE;    
r.bottom -= DRAG_EDGE; 
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


ShowDuration(int store)
{
char line[MAXLIN];
int i;

if(Pduration > 0.) {
	if(Qduration > 1.)
		sprintf(Message,"Dur = %.0f/%.0f = %.1f ticks",
			Pduration,Qduration,Pduration/Qduration);
	else sprintf(Message,"Dur = %.0f ticks",Pduration);
		
	if(Ratio != Prod) {
		if(Ratio < ULONG_MAX) sprintf(line,"  Ratio = %u",(unsigned long)Ratio);
		else sprintf(line,"   Ratio = %.0f",Ratio);
		if((strlen(Message) + strlen(line)) < MAXLIN) strcat(Message,line);
		}
		
	sprintf(line,"  Prod = %.0f",Prod);
	if((strlen(Message) + strlen(line)) < MAXLIN) strcat(Message,line);
	
	sprintf(line,"  [%ld objects]",(long)Maxevent);
	if((strlen(Message) + strlen(line)) < MAXLIN) strcat(Message,line);
	
	if(Kpress > 1.) sprintf(line,"  Comp = %.0f",Kpress);
	else sprintf(line,"  (no compression)");
	
	if((strlen(Message) + strlen(line)) < MAXLIN) strcat(Message,line);
	
	ShowWindow(Window[wMessage]);
	SetSelect(ZERO,GetTextLength(wMessage),TEH[wMessage]);
	TextDelete(wMessage);
	PrintBehind(wMessage,Message);
	TextUpdate(wMessage);
	if(store) {
		Jmessage++; if(Jmessage >= MAXMESSAGE) Jmessage = 0;
		MystrcpyStringToHandle(&(p_MessageMem[Jmessage]),Message);
		}
	}
return(OK);
}


PageClick(int up)
{
EventRecord event;
Point pt;
Rect r;

if(Nw < 0 || Nw >= WMAX || !Editable[Nw]) return(OK);
SetPort(Window[Nw]);
r = Window[Nw]->portRect;
event.what = mouseDown;
pt.h = r.right - r.left + SBARWIDTH/2;
if(up) pt.v = SBARWIDTH;
else pt.v = r.bottom - r.top - SBARWIDTH;
LocalToGlobal(&pt);
event.where = pt;
DoEvent(&event);
return(OK);
}