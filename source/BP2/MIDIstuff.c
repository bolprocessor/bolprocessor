/* MIDIstuff.c (BP2 version CVS) */ 

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



ListenMIDI(int x0, int x1, int x2)
{
int i,j,r,c,c0,c1,c2,filter,idummy,eventfound;
long jdummy;
MIDI_Event e;
char **p_line;


r = OK;
if(EmergencyExit || Panic) return(OK);

#if BP_CARBON_GUI
if(Panic || CheckEndOfWait() != OK) return(ABORT);
if((r=GetHighLevelEvent()) != OK) return(r);  
#endif /* BP_CARBON_GUI */

if((Oms || !OutMIDI) && !Interactive && !ReadKeyBoardOn && !ScriptRecOn) return(OK);

if(!IsMidiDriverOn()) {
	if(Beta) {
		Alert1("Err. ListenMIDI(). Driver is OFF");
		OutMIDI = Interactive = ReadKeyBoardOn = ScriptRecOn = FALSE;
		SetButtons(TRUE);
		}
	return(ABORT);
	}
#if WITH_REAL_TIME_MIDI
filter = x0 + x1 + x2;

// if(!Oms) return(OK);  /* $$$ */
if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) return(OK);

STARTCHECK:
if(e.type == NULL_EVENT) return(OK);

if(RunningStatus > 0) {
	if(e.data2 < 128) {
		c0 = RunningStatus;
		c1 = e.data2;
		if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) {
			RunningStatus = 0;
			return(OK);
			}
		c2 = e.data2;
		goto INTERPRET;
		}
	else RunningStatus = 0;
	}

c0 = e.data2;
c = c0 - c0 % 16;

if(!ThreeByteChannelEvent(c)) {
	RunningStatus = 0;
	if(c0 < 128) return(OK);
	if(c0 == SystemExclusive) {
		do {
			e.time = 0;
			e.type = RAW_EVENT;
			e.data2 = c0;
			if(!Oms && SysExPass) DriverWrite(ZERO,0,&e);
			if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) break;
			c0 = e.data2;
			}
		while(c0 != EndSysEx && e.type != NULL_EVENT);
		e.time = 0;
		e.type = RAW_EVENT;
		e.data2 = c0;
		if(!Oms && SysExPass) DriverWrite(ZERO,0,&e);
		return(OK);
		}
#if BP_CARBON_GUI
	if(ReadKeyBoardOn && Jcontrol == -1 && LastEditWindow != wScript) {
		if(c0 == Start) Print(LastEditWindow," Start");
		if(c0 == Stop) Print(LastEditWindow," Stop");
		if(c0 == Continue) Print(LastEditWindow," Continue");
		}
#endif /* BP_CARBON_GUI */
	if(c0 == SongPosition) {
		for(i=0; i < 2; i++) {
			e.time = 0;
			e.type = RAW_EVENT;
			e.data2 = c0;
			if(!Oms && SongPosPass) DriverWrite(ZERO,0,&e);
			if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) break;
			c0 = e.data2;
			}
		e.time = 0;
		e.type = RAW_EVENT;
		e.data2 = c0;
		if(!Oms && SongPosPass) DriverWrite(ZERO,0,&e);
		return(OK);
		}
	if(c0 == SongSelect) {
		e.time = 0;
		e.type = RAW_EVENT;
		e.data2 = c0;
		if(!Oms && SongSelPass) DriverWrite(ZERO,0,&e);
		if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) return(OK);
		c0 = e.data2;
		e.time = 0;
		e.type = RAW_EVENT;
		e.data2 = c0;
		if(!Oms && SongSelPass) DriverWrite(ZERO,0,&e);
		return(OK);
		}
	if(c == ProgramChange) {
		e.time = 0;
		e.type = RAW_EVENT;
		e.data2 = c0;
		if(!Oms && ProgramTypePass) DriverWrite(ZERO,0,&e);
		if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) return(OK);
		sprintf(Message,"%ld",(long)(e.data2) + ProgNrFrom);
		if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
		e.time = 0;
		e.type = RAW_EVENT;
		if(!Oms && ProgramTypePass) DriverWrite(ZERO,0,&e);
		sprintf(Message,"%ld",(long)((c0 % 16) + 1));
		if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,1,Message);
		if(!ScriptExecOn) AppendScript(71);
#if BP_CARBON_GUI
		if(ReadKeyBoardOn && Jcontrol == -1 && LastEditWindow != wScript) {
			sprintf(Message,"%s(",*((*p_PerformanceControl)[4]));
			Print(LastEditWindow,Message);
			for(j=0; j < ScriptNrLabel(71); j++) {
				PrintHandle(LastEditWindow,p_ScriptLabelPart(71,j));
				if(j < ScriptNrArg(71)) {
					Print(LastEditWindow," ");
					PrintHandle(LastEditWindow,(*(ScriptLine.arg))[j]);
					if(j < ScriptNrLabel(71) - 1) Print(LastEditWindow," ");
					}
				}
			Print(LastEditWindow,") ");
			ShowSelect(CENTRE,LastEditWindow);
			}
#endif /* BP_CARBON_GUI */
		return(OK);
		}
	if(c == ChannelPressure) {
		e.type = RAW_EVENT;
		if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) return(OK);
		c1 = e.data2;
		e.time = 0;
		e.type = TWO_BYTE_EVENT;
		e.status = c;
		if(!Oms && ChannelPressurePass) DriverWrite(ZERO,0,&e);
		if(Jcontrol < 0) {
			sprintf(Message,"Pressure = %ld channel %ld",(long)c1,
				(long)(c0 - c + 1));
			if(Interactive && ShowMessages) ShowMessage(TRUE,wMessage,Message);
			if(Interactive) {
				(*p_Oldvalue)[c0-c].pressure = c1;
				ChangedPressure[c0-c] = TRUE;
				}
			}		
#if BP_CARBON_GUI
		else  ReadMIDIparameter(c0,c1,0,LastEditWindow);
#endif /* BP_CARBON_GUI */
		
		return(OK);
		}
	e.time = 0;
	e.type = RAW_EVENT;
	e.data2 = c0;
	if(!Oms && PassEvent(c0)) DriverWrite(ZERO,0,&e);
	return(OK);
	}

RunningStatus = c0;
if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) return(OK);
c1 = e.data2;

if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) return(OK);
c2 = e.data2;

INTERPRET:

c = c0 - c0 % 16;

if(!Oms && PassEvent(c0)) {
	e.time = 0;
	e.type = NORMAL_EVENT;
	e.status = c0;
	e.data1 = c1;
	e.data2 = c2;
	DriverWrite(ZERO,0,&e);
	}
if(filter && c2 != 0 && (x0 == 0 || x0 == c0) && (x1 == 0 || x1 == c1)
			&& (x2 == 0 || x2 == c2)) return(RESUME);
#if BP_CARBON_GUI
if(ReadKeyBoardOn && LastEditWindow != wScript) {
	if(c == ControlChange && c1 == 64 && Jcontrol == -1) {
		if(c2 > 0) {	/* Pushed hold pedal */
			if(SplitTimeObjects && !EmptyBeat) Print(LastEditWindow," ");
			Print(LastEditWindow,"-");
			EmptyBeat = FALSE; FoundNote = TRUE;
			ShowSelect(CENTRE,LastEditWindow);
			return(OK);
			}
		}
	else {
		if(Jcontrol == -1) {
			GetControlParameters();
			if(ReadNoteOn(c0,c1,c2,LastEditWindow) == OK) {
				ShowSelect(CENTRE,LastEditWindow);
				return(OK);
				}
			}
		else {
			if(ReadMIDIparameter(c0,c1,c2,LastEditWindow) == OK) {
			/* Modulation will be processed separately because it requires 2 messages */
				ShowSelect(CENTRE,LastEditWindow);
				return(OK);
				}
			}
		}
	}
#endif /* BP_CARBON_GUI */

if((Interactive || ScriptRecOn || ReadKeyBoardOn) && c == ControlChange && c1 > 95
		&& c1 < 122) {
	/* Undefined controllers */
	if(!ScriptExecOn) {
		sprintf(Message,"%ld",(long)c1);
		MystrcpyStringToTable(ScriptLine.arg,0,Message);
		sprintf(Message,"%ld",(long)c2);
		MystrcpyStringToTable(ScriptLine.arg,1,Message);
		sprintf(Message,"%ld",(long)(c0 - c + 1));
		MystrcpyStringToTable(ScriptLine.arg,2,Message);
		AppendScript(75);
		}
	sprintf(LineBuff,"Controller #%ld = %ld channel %ld",(long)c1,(long)c2,
		(long)c0-c+1);
	if(Interactive && ShowMessages) ShowMessage(TRUE,wMessage,LineBuff);
#if BP_CARBON_GUI
	if(ReadKeyBoardOn && Jcontrol == -1 && LastEditWindow != wScript) {
		sprintf(Message,"%s(",*((*p_PerformanceControl)[4]));
		Print(LastEditWindow,Message);
		for(j=0; j < ScriptNrLabel(75); j++) {
			PrintHandle(LastEditWindow,p_ScriptLabelPart(75,j));
			if(j < ScriptNrArg(75)) {
				Print(LastEditWindow," ");
				PrintHandle(LastEditWindow,(*(ScriptLine.arg))[j]);
				if(j < ScriptNrLabel(75) - 1) Print(LastEditWindow," ");
				}
			}
		Print(LastEditWindow,") ");
		ShowSelect(CENTRE,LastEditWindow);
		}
#endif /* BP_CARBON_GUI */
	return(OK);
	}
if((Interactive || ScriptRecOn) && c == ChannelMode && c1 > 121) {
	LineBuff[0] = '\0';
	sprintf(Message,"%ld",(long)(c0 - c + 1));
	if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
	switch(c1) {
		case 122:
			if(c2 == 0) {	/* Local control off */
				AppendScript(76); sprintf(LineBuff,"Local control off channel %ld",
					(long)(c0-c+1));
				break;
				}
			if(c2 == 127) {	/* Local control on */
				AppendScript(77); sprintf(LineBuff,"Local control on channel %ld",
					(long)(c0-c+1));
				break;
				}
			break;
		case 123: /* if(c2 == 0) AppendScript(78) */ break;	/* All notes off */
		case 124: if(c2 == 0) {	/* Omni mode off */
			AppendScript(79); sprintf(LineBuff,"Omni mode off channel %ld",(long)(c0-c+1));
			break;
			}
		case 125: if(c2 == 0) {	/* Omni mode on */
			AppendScript(80); sprintf(LineBuff,"Omni mode on channel %ld",(long)(c0-c+1));
			break;
			}
		case 127: if(c2 == 0) {	/* Poly mode on */
			AppendScript(82); sprintf(LineBuff,"Poly mode on channel %ld",(long)(c0-c+1));
			break;
			}
		case 126:	/* Mono mode on, c2 channels */
			sprintf(LineBuff,"Mono mode on (%ld channels) channel %ld",(long)c2,
				(long)(c0-c+1));
			sprintf(Message,"%ld",(long)(c0 - c + 1));
			if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,1,Message);
			sprintf(Message,"%ld",(long)c2);
			if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
			AppendScript(81);
			break;
		}
	if(LineBuff[0] != '\0' && ShowMessages) ShowMessage(TRUE,wMessage,LineBuff);
	return(OK);
	}
if(Interactive || ScriptRecOn || ReadKeyBoardOn) {
	if(c == PitchBend) {
		if(Jcontrol == -1) {
			sprintf(Message,"Pitchbend = %ld channel %ld",(long) c1 + 128L*c2,
				(long)(c0 - c + 1));
			if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
			if(Interactive) {
				(*p_Oldvalue)[c0-c].pitchbend = c1 + (128L * c2);
				ChangedPitchbend[c0-c] = TRUE;
				}
			}
		return(OK);
		}
	if(c == ControlChange) {
		if(c1 == VolumeControl[c0-c+1]) {	/* Volume */
			if(Jcontrol == -1) {
				sprintf(Message,"Volume = %ld channel %ld",(long)c2,
					(long)(c0 - c + 1));
				if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
				if(Interactive) {
					(*p_Oldvalue)[c0-c].volume = c2;
					ChangedVolume[c0-c] = TRUE;
					}
				}
			return(OK);
			}
		else
		if(c1 == PanoramicControl[c0-c+1]) {	/* Panoramic */
			if(Jcontrol == -1) {
				sprintf(Message,"Panoramic = %ld channel %ld",(long)c2,
					(long)(c0 - c + 1));
				if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
				if(Interactive) {
					(*p_Oldvalue)[c0-c].panoramic = c2;
					ChangedPanoramic[c0-c] = TRUE;
					}
				}
			return(OK);
			}
		else
		if(c1 == 1) {	/* Modulation MSB */
			OldModulation = (long) 128L * c2;
			sprintf(Message,"Modulation = %ld channel %ld",(long)OldModulation,
				(long)(c0 - c + 1));
			if(ShowMessages && Jcontrol == -1) ShowMessage(TRUE,wMessage,Message);
			return(OK);
			}
		else
		if(c1 == 33) {	/* Modulation LSB */
			if(Jcontrol == -1) {
				sprintf(Message,"Modulation = %ld channel %ld",(long)(c2+OldModulation),
					(long)(c0 - c + 1));
				if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
				if(Interactive) {
					(*p_Oldvalue)[c0-c].modulation = c2 + OldModulation;
					ChangedModulation[c0-c] = TRUE;
					}
				}
			return(OK);
			}
		else
		if(c1 >= 64 && c1 <= 95) {
			if(!ScriptExecOn) {
				sprintf(Message,"%ld",(long)(c0-c+1));
				MystrcpyStringToTable(ScriptLine.arg,1,Message);
				}
			if(c2 == 0) {
				sprintf(LineBuff,"Switch %ld OFF channel %ld",(long)c1,(long)(c0-c+1));
				sprintf(Message,"%ld",(long)c1);
				if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
				AppendScript(87);
				}
			else {
				sprintf(LineBuff,"Switch %ld ON channel %ld",(long)c1,(long)(c0-c+1));
				sprintf(Message,"%ld",(long)c1);
				if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
				AppendScript(86);
				}
			if(ShowMessages && Jcontrol == -1) ShowMessage(TRUE,wMessage,LineBuff);
			return(OK);
			}
		}
	}
if(Interactive && Ctrl_adjust(&e,c0,c1,c2) == OK) {
	if(e.type == NULL_EVENT) return(OK);
	RunningStatus = 0;
	c0 = e.data2;
	goto STARTCHECK;
	}
if(Interactive && c2 > 0) {
	if(EndRepeatChan > 0 && c1 == EndRepeatKey && c0 == (NoteOn+EndRepeatChan-1)) {
		Nplay = 1; SynchroSignal = OFF;
		ShowMessage(TRUE,wMessage,"Stop repeating!");
		return(ENDREPEAT);
		}
	else {
	if(MuteOnChan > 0 && c1 == MuteOnKey && c0 == (NoteOn+MuteOnChan-1)) {
		Mute = TRUE;
		ShowMessage(TRUE,wMessage,"Received MIDI message telling to mute output");
		FlashInfo("MUTE is ON...   cmd-space will turn if off");
		return(OK);
		}
	else {
	if(MuteOffChan > 0 && c1 == MuteOffKey && c0 == (NoteOn+MuteOffChan-1)) {
		Mute = FALSE;
		ShowMessage(TRUE,wMessage,"Received MIDI message telling to stop muting output");
		HideWindow(Window[wInfo]);
		return(OK);
		}
	else {
	if(QuitChan > 0 && c1 == QuitKey && c0 == (NoteOn+QuitChan-1)) {
		SkipFlag = FALSE;
		ShowMessage(TRUE,wMessage,"Abort!");
		return(ABORT);
		}
	else {
	if(PlayChan > 0 && c1 == PlayKey && c0 == (NoteOn+PlayChan-1)) {
		SynchroSignal = PLAYNOW;
		ShowMessage(TRUE,wMessage,"Play now!");
		return(QUICK);
		/* Used to interrupt TimeSet() and play immediately */
		}
	else {
	if(EverChan > 0 && c1 == EverKey && c0 == (NoteOn + EverChan - 1)) {
		SynchroSignal = PLAYFOREVER;
		ClearMessage();
		Print(wMessage,"Play forever! (");
		PrintNote(EndRepeatKey,EndRepeatChan,wMessage,Message);
		Print(wMessage," will stop)");
		return(OK);
		}
	else {
	if(RepeatChan > 0 && c1 == RepeatKey && c0 == (NoteOn + RepeatChan -1)) {
		Nplay = c2;
		SynchroSignal = OFF;
		ClearMessage();
		sprintf(Message,"Playing %ld times... (",(long)Nplay);
		Print(wMessage,Message);
		PrintNote(EndRepeatKey,EndRepeatChan,wMessage,Message);
		Print(wMessage," will stop)");
		return(OK);
		}
	else {
	if(AgainChan > 0 && c1 == AgainKey && c0 == (NoteOn+AgainChan-1)) {
		ShowMessage(TRUE,wMessage,"Play again!");
		return(AGAIN);
		}
	else {
	if(DeriveFurtherChan > 0 && c1 == DeriveFurtherKey && c0 == (NoteOn+
			DeriveFurtherChan-1)) {
		DeriveFurther = 1 - DeriveFurther;
		if(DeriveFurther)
			ShowMessage(TRUE,wMessage,"Derive further flag ON");
		else
			ShowMessage(TRUE,wMessage,"Derive further flag OFF");
		return(OK);
		}
	else {
		for(j=1; j <= Jinscript; j++) {
			if(((*p_INscript)[j]).chan != -1) {
				if(c0 == (NoteOn + ((*p_INscript)[j]).chan - 1)
							&& c1 == ((*p_INscript)[j]).key && c2 > 0) {
					ScriptExecOn++;
					MystrcpyTableToString(MAXLIN,Message,p_Script,((*p_INscript)[j]).scriptline);
					if((p_line = (char**) GiveSpace((Size)(strlen(Message)+1))) == NULL)
						return(ABORT);
					MystrcpyStringToHandle(&p_line,Message);
					if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
					r = ExecScriptLine((char***) NULL,wScript,FALSE,FALSE,p_line,jdummy,
						&jdummy,&idummy,&idummy);
					MyDisposeHandle((Handle*)&p_line);
					EndScript();
					if(r != OK) return(r);
					if(ShowMessages) HideWindow(Window[wMessage]);
					}
				}
			}
		}}}}}}}}}
	if(ParamControlChan > 0) {
		for(i=1; i < MAXPARAMCTRL; i++) {
			if(ParamChan[i] != -1 && ParamKey[i] == c1
								  && c0 == (NoteOn + ParamChan[i] - 1)) {
			/* IN Param key «Kx» = velocity «note» channel «1..16» */
				sprintf(Message,"K%ld = %ld",(long)i,(long)c2);
				ShowMessage(TRUE,wMessage,Message);
				ParamValue[i] = c2;
				return(OK);
				}
			}
		}
	r = ChangeStatus(c0,c1,c2);
	}
return(r);
#endif
}


Ctrl_adjust(MIDI_Event *p_e,int c0,int c1,int c2)
{
int speed_change,i,j,r,c11;
long count = 12L,oldn,dt;

if(!IsMidiDriverOn()) {
	if(Beta) Alert1("Err. Ctrl_adjust(). Driver is OFF");
	return(ABORT);
	}
r = FAILED;
if(ParamControlChan > 0) {
	for(i=1; i < MAXPARAMCTRL; i++) {
		if(ParamChan[i] != -1 && c0 == (ControlChange + ParamChan[i] - 1)
			&& ParamControl[i] == c1) {
		/* IN Param «Kx» = controller #«0..127» channel «1..16» */
			ParamControlChan = ParamChan[i];
			sprintf(Message,"K%ld = %ld",(long)i,(long)c2);
			ShowMessage(TRUE,wMessage,Message);
			ParamValue[i] = c2;
			r= OK;
			}
		}
	}
if(SpeedChan > 0 && c0 == ControlChange+SpeedChan-1 && c1 == SpeedCtrl) {
	while(TRUE) {
		if(GetNextMIDIevent(p_e,FALSE,FALSE) != OK) break;
		if(p_e->data2 > 127 || p_e->type == NULL_EVENT) break;
		c11 = p_e->data2;
		p_e->type = RAW_EVENT;
		if(GetNextMIDIevent(p_e,FALSE,FALSE) != OK) break;
		if(p_e->data2 > 127 || p_e->type == NULL_EVENT) break;
		c1 = c11;
		c2 = p_e->data2;
		}
	Newstatus = TRUE;
	if(PedalOrigin == -1) {
		PedalOrigin = c2;
		OldPclock = Pclock;
		OldQclock = Qclock;
		Nalpha = 100L;
		}
	else {
		PedalPosition = c2;
		if(c2 == PedalOrigin) return(OK);
		SetTempo(); SetTimeBase();
		ShowWindow(Window[wMetronom]);
		BringToFront(Window[wMetronom]);
		}
	r = OK;
	}
return(r);
}


ChangeStatus(int c0,int c1,int c2)
{
long newP,newQ;

if(!IsMidiDriverOn()) {
	if(Beta) Alert1("Err. ChangeStatus(). Driver is OFF");
	return(ABORT);
	}
if(ResetWeightChan > 0 && c1 == ResetWeightKey
		&& c0 == (NoteOn + ResetWeightChan - 1)) {
	ResetWeights = 1 - ResetWeights;
	if(ResetWeights) NeverResetWeights = FALSE;
	if(!Improvize) SetButtons(TRUE);
	sprintf(Message,"Reset weights: %s",Reality[ResetWeights]);
	ShowMessage(TRUE,wMessage,Message);
	}
else {
	if(UseEachSubChan > 0 && SUBthere && c1 == UseEachSubKey
		&& c0 == (NoteOn + UseEachSubChan - 1)) {
		UseEachSub = 1 - UseEachSub; if(!Improvize) SetButtons(TRUE);
		sprintf(Message,"Play each substitution: %s",Reality[UseEachSub]);
		ShowMessage(TRUE,wMessage,Message);
		}
	else {
		if(SynchronizeStartChan > 0 && c1 == SynchronizeStartKey
			&& c0 == (NoteOn + SynchronizeStartChan - 1)) {
			SynchronizeStart = 1 - SynchronizeStart; if(!Improvize) SetButtons(TRUE);
			sprintf(Message,"Synchronize start: %s",Reality[SynchronizeStart]);
			ShowMessage(TRUE,wMessage,Message);
			}
		else {
			if(c1 == SetTimeKey && SetTimeChan > 0
				&& Improvize && c2 > 0
					&& c0 == (NoteOn + SetTimeChan - 1)) {
				TimeMax = (long) c2 * 472;
				/* UseTimeLimit = */ LimCompute = LimTimeSet = TRUE;
				sprintf(Message,
					"Max computation time: %4.2f s.",(double) TimeMax/1000.);
				ShowMessage(TRUE,wMessage,Message);
				}
			else {
				if(StriatedChan > 0 && c1 == StriatedKey
							&& c0 == (NoteOn + StriatedChan - 1)) {
					Nature_of_time = 1 - Nature_of_time;
					sprintf(Message,
						"Striated time: %s",Reality[Nature_of_time]);
					ShowMessage(TRUE,wMessage,Message);
					SetTempo(); SetTimeBase();
					Newstatus = TRUE;
					}
				else {
					if(NoConstraintChan > 0 && c1 == NoConstraintKey
								&& c0 == (NoteOn + NoConstraintChan - 1)) {
						NoConstraint = 1 - NoConstraint;
						if(!Improvize) SetButtons(TRUE);
						sprintf(Message,
							"Ignore constraints: %s",Reality[NoConstraint]);
						ShowMessage(TRUE,wMessage,Message);
						Newstatus = TRUE;
						}
					else {
						if(TclockChan > 0 && c0 == (NoteOn + TclockChan - 1)
								&& ((c1 >= MinTclockKey && c1 <= MaxTclockKey)
								|| (c1 <= MinTclockKey && c1 >= MaxTclockKey))) {
							Pclock = 100L * MinPclock +
								(long) ((100L * (c1 - MinTclockKey) * (MaxPclock - MinPclock)) /
								(MaxTclockKey - MinTclockKey));
							Qclock = 100L * MinQclock +
								(long) ((100L * (c1 - MinTclockKey) * (MaxQclock - MinQclock)) /
								(MaxTclockKey - MinTclockKey));
							Simplify((double)INT_MAX,Pclock,Qclock,&Pclock,&Qclock);
							Nalpha = 100L;
							PedalOrigin = -1;
							SetTempo(); SetTimeBase();
							ShowWindow(Window[wMetronom]);
							BringToFront(Window[wMetronom]);
							Newstatus = TRUE;
							}
						else {
							if(Improvize && SkipChan > 0 && c1 == SkipKey
										&& c0 == (NoteOn + SkipChan - 1)) {
								SkipFlag = TRUE;
								ShowMessage(TRUE,wMessage,"Skip next item");
								}
							}
						}
					}
				}
			}
		}
	}
return(OK);
}


SetInputFilterWord(void)
{
if(EndSysExIn) SysExIn = TRUE;
if(SysExIn) EndSysExIn = TRUE;
if(StartTypeIn) ContTypeIn = TRUE;
if(ContTypeIn) StartTypeIn = TRUE;
if(NoteOnIn) NoteOffIn = TRUE;
if(NoteOffIn) NoteOnIn = TRUE;
MIDIinputFilter =
	NoteOffIn + 2L * (NoteOnIn + 2L * (KeyPressureIn + 2L * (ControlTypeIn
		+ 2L * (ProgramTypeIn + 2L * (ChannelPressureIn + 2L * (PitchBendIn
		+ 2L * (SysExIn + 2L * (TimeCodeIn + 2L * (SongPosIn + 2L * (SongSelIn
		+ 2L * (TuneTypeIn + 2L * (EndSysExIn + 2L * (ClockTypeIn
		+ 2L * (StartTypeIn + 2L * (ContTypeIn + 2L * (ActiveSenseIn
		+ 2L * ResetIn))))))))))))))));
return(OK);
}


SetOutputFilterWord(void)
{
if(SetInputFilterWord() != OK) return(ABORT);
if(EndSysExPass) SysExPass = TRUE;
if(SysExPass) EndSysExPass = TRUE;
if(StartTypePass) ContTypePass = TRUE;
if(ContTypePass) StartTypePass = TRUE;
if(NoteOnPass) NoteOffPass = TRUE;
if(NoteOffPass) NoteOnPass = TRUE;
MIDIoutputFilter =
	NoteOffPass + 2L * (NoteOnPass + 2L * (KeyPressurePass + 2L * (ControlTypePass
		+ 2L * (ProgramTypePass + 2L * (ChannelPressurePass + 2L * (PitchBendPass
		+ 2L * (SysExPass + 2L * (TimeCodePass + 2L * (SongPosPass + 2L * (SongSelPass
		+ 2L * (TuneTypePass + 2L * (EndSysExPass + 2L * (ClockTypePass
		+ 2L * (StartTypePass + 2L * (ContTypePass + 2L * (ActiveSensePass
		+ 2L * ResetPass))))))))))))))));
/* To pass an event you should enable the driver to receive it… */
MIDIinputFilter = MIDIinputFilter | MIDIoutputFilter;
GetInputFilterWord();
return(OK);
}


GetInputFilterWord(void)
{
long n = 1L;

NoteOffIn = (MIDIinputFilter & n) != 0; n = n << 1;
NoteOnIn = (MIDIinputFilter & n) != 0; n = n << 1;
KeyPressureIn = (MIDIinputFilter & n) != 0; n = n << 1;
ControlTypeIn = (MIDIinputFilter & n) != 0; n = n << 1;
ProgramTypeIn = (MIDIinputFilter & n) != 0; n = n << 1;
ChannelPressureIn = (MIDIinputFilter & n) != 0; n = n << 1;
PitchBendIn = (MIDIinputFilter & n) != 0; n = n << 1;
SysExIn = (MIDIinputFilter & n) != 0; n = n << 1;
TimeCodeIn = (MIDIinputFilter & n) != 0; n = n << 1;
SongPosIn = (MIDIinputFilter & n) != 0; n = n << 1;
SongSelIn = (MIDIinputFilter & n) != 0; n = n << 1;
TuneTypeIn = (MIDIinputFilter & n) != 0; n = n << 1;
EndSysExIn = (MIDIinputFilter & n) != 0; n = n << 1;
ClockTypeIn = (MIDIinputFilter & n) != 0; n = n << 1;
StartTypeIn = (MIDIinputFilter & n) != 0; n = n << 1;
ContTypeIn = (MIDIinputFilter & n) != 0; n = n << 1;
ActiveSenseIn = (MIDIinputFilter & n) != 0; n = n << 1;
ResetIn  = (MIDIinputFilter & n) != 0;
return(OK);
}


GetOutputFilterWord(void)
{
long n = 1L;

NoteOffPass = (MIDIoutputFilter & n) != 0; n = n << 1;
NoteOnPass = (MIDIoutputFilter & n) != 0; n = n << 1;
KeyPressurePass = (MIDIoutputFilter & n) != 0; n = n << 1;
ControlTypePass = (MIDIoutputFilter & n) != 0; n = n << 1;
ProgramTypePass = (MIDIoutputFilter & n) != 0; n = n << 1;
ChannelPressurePass = (MIDIoutputFilter & n) != 0; n = n << 1;
PitchBendPass = (MIDIoutputFilter & n) != 0; n = n << 1;
SysExPass = (MIDIoutputFilter & n) != 0; n = n << 1;
TimeCodePass = (MIDIoutputFilter & n) != 0; n = n << 1;
SongPosPass = (MIDIoutputFilter & n) != 0; n = n << 1;
SongSelPass = (MIDIoutputFilter & n) != 0; n = n << 1;
TuneTypePass = (MIDIoutputFilter & n) != 0; n = n << 1;
EndSysExPass = (MIDIoutputFilter & n) != 0; n = n << 1;
ClockTypePass = (MIDIoutputFilter & n) != 0; n = n << 1;
StartTypePass = (MIDIoutputFilter & n) != 0; n = n << 1;
ContTypePass = (MIDIoutputFilter & n) != 0; n = n << 1;
ActiveSensePass = (MIDIoutputFilter & n) != 0; n = n << 1;
ResetPass  = (MIDIoutputFilter & n) != 0;
return(OK);
}


ResetMIDIFilter(void)
{
MIDIinputFilter = MIDIinputFilterstartup;
MIDIoutputFilter = MIDIoutputFilterstartup;
GetInputFilterWord();
GetOutputFilterWord();
SetOutputFilterWord();	/* Verifies consistency */
return(OK);
}


TwoByteEvent(int c)
{
int c0;

if(c < NoteOff) return(NO);
if(c == SongSelect) return(YES);
c0 = c - c % 16;
if(c0 == ProgramChange || c0 == ChannelPressure) return(YES);
return(NO);
}


ThreeByteEvent(int c)
{
int c0;

if(c < NoteOff) return(NO);
if(c == SongPosition) return(YES);
c0 = c - c % 16;
if(ThreeByteChannelEvent(c0)) return(YES);
return(NO);
}


ThreeByteChannelEvent(int c)
{
if(c < NoteOff) return(NO);
if(c == ProgramChange || c == ChannelPressure) return(NO);
if(c > PitchBend) return(NO);
return(YES);
}


ChannelEvent(int c)
{
int c0;

if(c < NoteOff) return(NO);
c0 = c - c % 16;
if(c0 < SystemExclusive) return(YES);
return(NO);
}


AcceptEvent(int c)
{
int c0;

switch(c) {
	case SystemExclusive:
		if(SysExIn) return(YES);
		break;
	case SongPosition:
		if(SongPosIn) return(YES);
		break;
	case SongSelect:
		if(SongSelIn) return(YES);
		break;
	case TuneRequest:
		if(TuneTypeIn) return(YES);
		break;
	case EndSysEx:
		if(EndSysExIn) return(YES);
		break;
	case TimingClock:
		if(ClockTypeIn) return(YES);
		break;
	case Start:
	case Continue:
	case Stop:
		if(StartTypeIn) return(YES);
		break;
	case ActiveSensing:
		if(ActiveSenseIn) return(YES);
		break;
	case SystemReset:
		if(ResetIn) return(YES);
		break;
	}
c0 = c - (c % 16);
switch(c0) {
	case NoteOff:
		if(NoteOffIn) return(YES);
		break;
	case NoteOn:
		if(NoteOnIn) return(YES);
		break;
	case KeyPressure:
		if(KeyPressureIn) return(YES);
		break;
	case ControlChange:
		if(ControlTypeIn) return(YES);
		break;
	case ProgramChange:
		if(ProgramTypeIn) return(YES);
		break;
	case ChannelPressure:
		if(ChannelPressureIn) return(YES);
		break;
	case PitchBend:
		if(PitchBendIn) return(YES);
		break;
	}
return(NO);
}


PassEvent(int c)
{
int c0;

switch(c) {
	case SystemExclusive:
		if(SysExPass) return(YES);
		break;
	case SongPosition:
		if(SongPosPass) return(YES);
		break;
	case SongSelect:
		if(SongSelPass) return(YES);
		break;
	case TuneRequest:
		if(TuneTypePass) return(YES);
		break;
	case EndSysEx:
		if(EndSysExPass) return(YES);
		break;
	case TimingClock:
		if(ClockTypePass) return(YES);
		break;
	case Start:
	case Continue:
	case Stop:
		if(StartTypePass) return(YES);
		break;
	case ActiveSensing:
		if(ActiveSensePass) return(YES);
		break;
	case SystemReset:
		if(ResetPass) return(YES);
		break;
	}
c0 = c - (c % 16);
switch(c0) {
	case NoteOff:
		if(NoteOffPass) return(YES);
		break;
	case NoteOn:
		if(NoteOnPass) return(YES);
		break;
	case KeyPressure:
		if(KeyPressurePass) return(YES);
		break;
	case ControlChange:
		if(ControlTypePass) return(YES);
		break;
	case ProgramChange:
		if(ProgramTypePass) return(YES);
		break;
	case ChannelPressure:
		if(ChannelPressurePass) return(YES);
		break;
	case PitchBend:
		if(PitchBendPass) return(YES);
		break;
	}
return(NO);
}


SendMIDIstream(int check,char** p_line,int hexa)
{
int i,r;
long n,time;
MIDI_Event e;

if((*p_line)[0] == '\0') return(FAILED);
i = 0; r = FAILED;

#if WITH_REAL_TIME_MIDI
MyLock(FALSE,(Handle)p_line);
time = ZERO;
do {
	while(MySpace((*p_line)[i])) i++;
	if((*p_line)[i] == '\0') break;
	if(!hexa && (n = GetInteger(YES,*p_line,&i)) == INT_MAX) break;
	if(hexa && (n = GetHexa(*p_line,&i)) == INT_MAX) break;
	if(n > 255 || n < 0) {
		sprintf(Message,"\nCan't send %ld as MIDI data.\n",(long)n);
		Print(wTrace,Message);
		 goto QUIT;
		}
	i++;
	e.time = time;
	e.type = RAW_EVENT;
	e.data2 = n;
	DriverWrite(time,0,&e);
	}
while(TRUE);
r = OK;

QUIT:
MyUnlock((Handle)p_line);
#endif

return(r);
}


ResetMIDIControllers(int now,int force,int givetime)
{
int ch,j,rs,result,lsb,msb;
MIDI_Event e;
long count = 12L;
Milliseconds tcurr;

if(!IsMidiDriverOn()) return(OK);
if(!OutMIDI || MIDIfileOpened) return(OK);

#if WITH_REAL_TIME_MIDI  // FIXME? do we need to reset BP's internal values anyways? - akozar
#if BP_CARBON_GUI
SwitchOn(NULL,wControlPannel,bResetControllers);
#endif /* BP_CARBON_GUI */

rs = 0;	/* Running status */

// givetime = (!Improvize || !ComputeOn) && !MIDIfileOn;

if(now) tcurr = ZERO;
else tcurr = Tcurr;

result = ABORT;
for(ch=0; ch < 16; ch++) {
	if(/* force || */ ChangedSwitch[ch]) {	/* 'force' suppressed on 18/11/97 */
		ChangedSwitch[ch] = FALSE;
		for(j=0; j < 32; j++) {	/* Switches */
			e.time = tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + ch;
			e.data1 = 64 + j;
			e.data2 = 0;
			if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
			if(givetime) Tcurr += 10L / Time_res;
			}
		}
	if(force || ChangedPitchbend[ch]) {
		ChangedPitchbend[ch] = FALSE;
		e.time = tcurr;	/* Pitchbend */
		e.type = NORMAL_EVENT;
		e.status = PitchBend + ch;
		(*p_Oldvalue)[ch].pitchbend = DEFTPITCHBEND;
		lsb = ((long)DEFTPITCHBEND) % 128;
		msb = (((long)DEFTPITCHBEND) - lsb) >> 7;
		e.data1 = lsb;
		e.data2 = msb;
		if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
		if(givetime) Tcurr += 10L / Time_res;
		}
	if(force || ChangedPressure[ch]) {
		ChangedPressure[ch] = FALSE;
		e.time = tcurr;	/* Pressure */
		e.type = TWO_BYTE_EVENT;
		e.status = ChannelPressure + ch;
		(*p_Oldvalue)[ch].pressure = DEFTPRESSURE;
		e.data2 = DEFTPRESSURE;
		if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
		if(givetime) Tcurr += 10L / Time_res;
		}
	if(force || ChangedModulation[ch]) {
		ChangedModulation[ch] = FALSE;
		e.time = tcurr;	/* Modulation */
		e.type = NORMAL_EVENT;
		e.status = ControlChange + ch;
		(*p_Oldvalue)[ch].modulation = DEFTMODULATION;
		lsb = ((long)DEFTMODULATION) % 128;
		msb = (((long)DEFTMODULATION) - lsb) >> 7;
		e.data1 = 1;
		e.data2 = msb;	/* MSB */
		if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
		e.time = tcurr;
		e.type = NORMAL_EVENT;
		e.status = ControlChange + ch;
		e.data1 = 33;
		e.data2 = lsb;	/* LSB */
		if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
		if(givetime) Tcurr += 10L / Time_res;
		}
	if(force || ChangedVolume[ch]) {
		ChangedVolume[ch] = FALSE;
		e.time = tcurr;	/* Volume */
		e.type = NORMAL_EVENT;
		e.status = ControlChange + ch;
		(*p_Oldvalue)[ch].volume = DeftVolume;
		e.data1 = VolumeControl[ch+1];
		e.data2 = DeftVolume;
		if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
		if(givetime) Tcurr += 10L / Time_res;
		}
	if(force || ChangedPanoramic[ch]) {
		ChangedPanoramic[ch] = FALSE;
		e.time = tcurr;	/* Panoramic */
		e.type = NORMAL_EVENT;
		e.status = ControlChange + ch;
		(*p_Oldvalue)[ch].panoramic = DeftPanoramic;
		e.data1 = PanoramicControl[ch+1];
		e.data2 = DeftPanoramic;
		if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
		if(givetime) Tcurr += 10L / Time_res;
		}
	}
HideTicks = FALSE;
if(now) WaitABit(500L);
result = OK;

OUT:
#if BP_CARBON_GUI
SwitchOff(NULL,wControlPannel,bResetControllers);
#endif /* BP_CARBON_GUI */
return(result);
#endif
}

#if BP_CARBON_GUI
// I'm not sure whether we need PlayPrototypeTicks() and RecordTick()
// in ANSI console build, but seems unlikely.  - akozar

PlayPrototypeTicks(int j)
{
MIDI_Event e;
/* long count = 12L;
MIDI_Parameters parms; */
int key,duration,channel,velocity,rep,rs;
double r,x,kx;
long p,q;
char line[MAXFIELDCONTENT];
unsigned long drivertime;

if(SoundOn || !OutMIDI || CheckEmergency() != OK) return(FAILED);

#if WITH_REAL_TIME_MIDI
rep = NO;
if(!GetCtrlValue(wPrototype5,bPlayPrototypeTicks)) goto DOIT;
key = PrototypeTickKey;
channel = PrototypeTickChannel - 1;
velocity = PrototypeTickVelocity;
if(rep == OK) rep = GetField(NULL,TRUE,wPrototype5,fTref,line,&p,&q);
duration = p/q;
if(duration < EPSILON) {
	sprintf(Message,"You can't play ticks when Tref is only %ldms",(long)duration);
	Alert1(Message);
	rep = FAILED;
	}
if(j > 2 && j < Jbol && (*p_Tref)[j] != duration) {
	if(Answer("You want to play ticks although this object is not striated. Proceed anyway?",
		'Y') != OK) return(FAILED);
	}
duration = duration / Time_res / 2;

DOIT:
if(rep != OK) {
	ResetMIDI(FALSE);
//	#ifndef __POWERPC
	FlushEvents(everyEvent,0);
//	#endif
	Alert1("Recording MIDI input... Click when over");
	return(OK);
	}
else {
	SetCursor(&WatchCursor);
	FlashInfo("Receiving MIDI data. Click mouse when completed.");
	}
ResetMIDI(FALSE);
// #ifndef __POWERPC
FlushEvents(everyEvent,0);
// #endif
rs = 0;
while(TRUE) {
	e.type = NORMAL_EVENT;
	e.time = Tcurr;
	e.status = NoteOn + channel;
	e.data1 = key;
	e.data2 = velocity;
	SendToDriver(Tcurr * Time_res,0,&rs,&e);
	Tcurr += duration;
	e.type = NORMAL_EVENT;
	e.time = Tcurr;
	e.status = NoteOn + channel;
	e.data1 = key;
	e.data2 = 0;
	SendToDriver(Tcurr * Time_res,0,&rs,&e);
	drivertime = GetDriverTime();
	while((Tcurr > drivertime + (SetUpTime / Time_res)) && !Button()) {
		/* Wait for the end of preceding play */
		drivertime = GetDriverTime();
		}
	Tcurr += duration;
	if(Button()) break;
	}
while(Button());

FlushEvents(mDownMask+mUpMask,0);

HideWindow(Window[wInfo]);
return(OK);
#endif
}


RecordTick(int where,int i)
{
MIDI_Event e;
int key,channel,c0,c1,velocity;
long count = 12L;
char line[MAXFIELDCONTENT];

if(!OutMIDI) {
	Alert1("Cannot record tick because MIDI output is inactive");
	return(FAILED);
	}
#if WITH_REAL_TIME_MIDI
SetCursor(&WatchCursor);
ShowMessage(TRUE,wMessage,"Play tick on MIDI keyboard. (Click mouse to abort)");
key = -1;
ResetMIDI(FALSE);

FlushEvents(everyEvent,0);
ReadKeyBoardOn = TRUE;

while(!Button()) {
	MaintainCursor();
	if(GetNextMIDIevent(&e,TRUE,FALSE) != OK) break;
	c0 = e.data2;
	if(c0 < NoteOn || c0 >= (NoteOn+16)) continue;
	if(GetNextMIDIevent(&e,TRUE,FALSE) != OK) break;
	c1 = e.data2;
	if(GetNextMIDIevent(&e,TRUE,FALSE) != OK) break;
	velocity = e.data2;
	if(velocity > 0) { 	/* NoteOn */
		key = c1;
		channel = (c0 % 16) + 1;
		break;
		}
	}
	
ReadKeyBoardOn = FALSE;
FlushEvents(mDownMask+mUpMask,0);

HideWindow(Window[wMessage]);
SetDefaultCursor();

if(key > -1) {
	switch(where) {
		case 1:
			PrototypeTickKey = key;
			PrototypeTickChannel = channel;
			PrototypeTickVelocity = velocity;
			break;
		case 0:
			TickKey[i] = key;
			TickChannel[i] = channel;
			TickVelocity[i] = velocity;
			break;
		case 3:
			sprintf(line,"%ld",(long)velocity);
			SetField(NULL,wTickDialog,fThisTickVelocity,line);
			sprintf(line,"%ld",(long)key);
			SetField(NULL,wTickDialog,fThisTickKey,line);
			sprintf(line,"%ld",(long)channel);
			SetField(NULL,wTickDialog,fThisTickChannel,line);
			GetThisTick();
		}
	}
return(OK);
#endif
}

#endif /* BP_CARBON_GUI */

MIDItoPrototype(int zerostart,int filter,int j,MIDIcode **p_b,long imax)
// Store MIDI codes to prototype j
{
long im2,nbytes;
MIDIcode **ptr1;
Handle ptr;
double preroll,postroll;

im2 = 2L * imax;
if((ptr1 = (MIDIcode**) GiveSpace((Size) im2 * sizeof(MIDIcode))) == NULL)
	return(ABORT);

if(FormatMIDIstream(p_b,imax,ptr1,zerostart,im2,&nbytes,filter) != OK) return(FAILED);

ptr = (Handle)(*pp_MIDIcode)[j];
if(MyDisposeHandle(&ptr) != OK) return(ABORT);
(*pp_MIDIcode)[j] = NULL;

(*pp_MIDIcode)[j] = ptr1;

GetPrePostRoll(j,&preroll,&postroll);
if(nbytes > ZERO) {
	(*p_MIDIsize)[j] = nbytes;
	(*p_Type)[j] |= 1;
	(*p_Dur)[j] = ((*((*pp_MIDIcode)[j]))[nbytes-1].time) - preroll + postroll;
	ptr1 = (*pp_MIDIcode)[j];
	MySetHandleSize((Handle*)&ptr1,(Size) nbytes * sizeof(MIDIcode));
	(*pp_MIDIcode)[j]  = ptr1;
	PointMIDI = TRUE;
	if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
	}
else {
	PointMIDI = FALSE;
	(*p_MIDIsize)[j] = ZERO;
	(*p_Dur)[j] = ZERO;
	}
return(OK);
}


FormatMIDIstream(MIDIcode **p_b,long imax,MIDIcode **p_c,int zerostart,
	long im2,long *p_nbytes,int filter)
{
long i,ii,time,runtime,t0;
int b,br,rc;

// Make sure dates are increasing, starting from 0

time = (*p_b)[0].time;
if(zerostart) t0 = time;
else t0 = ZERO;
for(i=0; i < imax; i++) {
	(*p_c)[i].sequence = (*p_b)[i].sequence;
	if((*p_b)[i].time < time) (*p_b)[i].time = time - t0;
	else {
		time = (*p_b)[i].time;
		(*p_b)[i].time -= t0;
		}
	}
ii = ZERO;
i = -1; br = 0;

// Restore missing status bytes

NEXTBYTE:
if(ii >= im2) {
	if(Beta) Alert1("Err. FormatMIDIstream(). ii >= im2");
	return(FAILED);
	}
i++; if(i >= imax) goto QUIT;
b = ByteToInt((*p_b)[i].byte);
time = (*p_b)[i].time;
if(!filter || b == TimingClock) {
	(*p_c)[ii].time = time;
	(*p_c)[ii].byte = b;
	(*p_c)[ii++].sequence = (*p_b)[i].sequence;
	goto NEXTBYTE;
	}
if(b == SystemExclusive) {
	br = 0;
	while(((b = (*p_b)[i].byte) != EndSysEx) && i < imax - 1) {
		(*p_c)[ii].time = time;
		(*p_c)[ii].byte = ByteToInt((*p_b)[i].byte);
		(*p_c)[ii++].sequence = (*p_b)[i].sequence;
		if(ii >= im2) {
			if(Beta) Alert1("Err. FormatMIDIstream(). ii >= im2");
			return(FAILED);
			}
		i++;
		}
	(*p_c)[ii].time = time;
	(*p_c)[ii].byte = EndSysEx;
	(*p_c)[ii++].sequence = 0;
	goto NEXTBYTE;
	}
if(b < 128) {	/* Data bytes following a status byte */
	if(br == 0) goto NEXTBYTE;	/* happens in beginning */
	(*p_c)[ii].time = time;
	(*p_c)[ii].byte = br + rc;
	(*p_c)[ii++].sequence = 0;
	if(ii >= im2) {
		if(Beta) Alert1("Err. FormatMIDIstream(). ii >= im2");
		return(FAILED);
		}
	(*p_c)[ii].time = time;
	(*p_c)[ii].byte = ByteToInt((*p_b)[i].byte);
	(*p_c)[ii++].sequence = (*p_b)[i].sequence;
	if(ii >= im2) {
		if(Beta) Alert1("Err. FormatMIDIstream(). ii >= im2");
		return(FAILED);
		}
	if(br == ProgramChange || br == ChannelPressure) goto NEXTBYTE;
	i++; if(i >= imax) goto QUIT;
	(*p_c)[ii].time = time;
	(*p_c)[ii].byte = ByteToInt((*p_b)[i].byte);
	(*p_c)[ii++].sequence = (*p_b)[i].sequence;
	goto NEXTBYTE;
	}
if(b >= SystemExclusive) goto NEXTBYTE;
rc = b % 16;
b -= rc;
br = b;
if(ThreeByteChannelEvent(b) || b == ProgramChange || b == ChannelPressure) {
	runtime = time;
	goto NEXTBYTE;
	}
br = 0;	/* b doesn't have a value that may be taken for running status */
goto NEXTBYTE;

QUIT:
*p_nbytes = ii;
return(OK);
}

#if BP_CARBON_GUI

SelectControlArgument(int w,char* line)
{
int i,ii,j,ok,n;
long iorg,iend,length;
char c;

TextGetSelection(&iorg, &iend, TEH[w]);
length = GetTextLength(w);
if(line[0] == '\0') return(FAILED);

/* Suppress bracket */
line[strlen(line)-1] = '\0';
Jcontrol = -1;
for(j=0; j < MaxPerformanceControl; j++) {
	if(Mystrcmp((*p_PerformanceControl)[j],line) == 0) {
		Jcontrol = j; break;
		}
	}
ok = YES;
switch(Jcontrol) {
	case 0:	/* _chan */
		ShowMessage(TRUE,wMessage,"Waiting for MIDI message indicating channel...");
		break;
	case 1:	/* _vel */
		ShowMessage(TRUE,wMessage,"Waiting for NoteOn message indicating velocity...");
		break;
	case 5:	/* _mod */
		ShowMessage(TRUE,wMessage,"Move the modulation bender...");
		break;
	case 8:	/* _pitchbend */
		ShowMessage(TRUE,wMessage,"Move the pitch bender...");
		break;
	case 11:	/* _press */
		ShowMessage(TRUE,wMessage,"Play a note and adjust pressure...");
		break;
	case 16:	/* _volume */
		ShowMessage(TRUE,wMessage,"Move the volume pedal...");
		break;
	case 35:	/* _volumecontrol */
		ShowMessage(TRUE,wMessage,"Move the controller you are using for volume...");
		break;
	case 36:	/* _pan */
		ShowMessage(TRUE,wMessage,"Move the panoramic controller...");
		break;
	case 41:	/* _pancontrol */
		ShowMessage(TRUE,wMessage,"Move the controller you are using for panoramic...");
		break;
	default:
		ok = NO; break;
	}
if(!ok) return(FAILED);

/* Select argument */
i = iend;
while(GetTextChar(w,i) != ')') {
	i++;
	if(i >= length) return(FAILED);
	}
SetSelect(iend,i,TEH[w]);
BPActivateWindow(SLOW,w);
if(Jcontrol != 8 && Jcontrol != 16 && Jcontrol != 36) return(OK);

/* Look for particular settings on same line */
i = iend;
while(GetTextChar(w,i) != '\r') {
	i--; if(i < 0) break;
	}
i++;
PitchbendRange[0] = DeftPitchbendRange;
VolumeControl[0] = VolumeController;
PanoramicControl[0] = PanoramicController;
for(i=i; i < iend; i++) {
	if((c=GetTextChar(w,i)) != '_') continue;
	iorg = i;
	line[i-iorg] = c;
	for(i=iorg+1;;i++) {
		c = GetTextChar(w,i);
		if(c == '(' || !isalnum(c) || i >= iend) break;
		line[i-iorg] = c;
		}
	line[i-iorg] = '\0';
	if(c != '(') continue;
	for(j=0; j < MaxPerformanceControl; j++) {
		if(Mystrcmp((*p_PerformanceControl)[j],line) == 0) {
			break;
			}
		}
	if(j == MaxPerformanceControl) continue;
	iorg = i + 1;
	/* Copy numeric argument */
	for(i=iorg;;i++) {
		c = GetTextChar(w,i);
		if(!isdigit(c)) break;
		if(c == ')' || i >= iend) break;
		line[i-iorg] = c;
		}
	if(i >= iend) break;
	line[i-iorg] = '\0'; ii = 0;
	if((n=GetInteger(YES,line,&ii)) == INT_MAX) continue;
	switch(j) {
		case 29:	/* _pitchrange */
			if(Jcontrol == 8) {
				if(n < 0 || n > 16383) {
					sprintf(Message,
						"Range for _pitchrange(x) is 0..16383. Can't accept %ld",(long)n);
					Alert1(Message);
					return(FAILED);
					}
				PitchbendRange[0] = n;	/* 0 is special index */
				}
			break;
		case 35:	/* _volumecontrol */
			if(Jcontrol == 16) {
				if(n < 0 || n > 127) {
					sprintf(Message,
						"Range for _volumecontrol(x) is 0..127. Can't accept %ld",(long)n);
					Alert1(Message);
					return(FAILED);
					}
				VolumeControl[0] = n;	/* 0 is special index */
				}
			break;
		case 41:	/* _pancontrol */
			if(Jcontrol == 36) {
				if(n < 0 || n > 127) {
					sprintf(Message,
						"Range for _pancontrol(x) is 0..127. Can't accept %ld",(long)n);
					Alert1(Message);
					return(FAILED);
					}
				PanoramicControl[0] = n;	/* 0 is special index */
				}
			break;
		default: continue;
		}
	}
return(OK);
}


ReadMIDIparameter(int c0, int c1, int c2, int wind)
// Get parameter of incoming MIDI message and put it as an argument
// of performance control.
// Argument is already selected.
{
int x,chan,c;
long origin,end;

if(Jcontrol == -1) return(FAILED);
chan = c0 % 16;
c = c0 - chan;
switch(Jcontrol) {
	case 0:	/* _chan */
		x = chan + 1; break;
	case 1:	/* _vel */
		if(c == NoteOn && c2 > 0) x = c2;
		else return(FAILED);
		break;
	case 5:	/* _mod */
		if(c == ControlChange && (c1 == 33 || c1 == 1)) {
			if(c1 == 33) x = c2 + OldModulation;
		/* Here c2 may be greater than 256 because it contains MSB and LSB. */
			else {
				x = OldModulation = 128 * c2;
				}
			}
		else return(FAILED);
		break;
	case 8:	/* _pitchbend */
		if(c == PitchBend) x = c1 + (128L * c2);
		else return(FAILED);
		if(PitchbendRange[0] > 0) {
			x =  PitchbendRange[0] * ((double) x - DEFTPITCHBEND) / ((double) DEFTPITCHBEND);
			}
		break;
	case 11:	/* _press */
		if(c == ChannelPressure) x = c1;
		else return(FAILED);
		break;
	case 16:	/* _volume */
		if(c == ControlChange && c1 == VolumeControl[0]) x = c2;
		else return(FAILED);
		break;
	case 35:	/* _volumecontrol */
		if(c == ControlChange) x = c1;
		else return(FAILED);
		break;
	case 36:	/* _pan */
		if(c == ControlChange && c1 == PanoramicControl[0]) x = c2;
		else return(FAILED);
		break;
	case 41:	/* _pancontrol */
		if(c == ControlChange) x = c1;
		else return(FAILED);
		break;
	default: return(FAILED);
	}
sprintf(Message,"%ld",(long)x);
TextDelete(LastEditWindow);
#if USE_MLTE
// FIXME ? Is there a reason the Print() call is inbetween getting origin & end below ?
TextGetSelection(&origin, &end, TEH[LastEditWindow]);
Print(LastEditWindow,Message);
#else
origin = (**(TEH[LastEditWindow])).selStart;
Print(LastEditWindow,Message);
end = (**(TEH[LastEditWindow])).selEnd;
#endif
SetSelect(origin,end,TEH[LastEditWindow]);
return(OK);
}


SetFilterDialog(void)
// Set buttons in MIDI filter dialog
{
SwitchOnOff(NULL,wFilter,bNoteOnOffIn,NoteOffIn);
SwitchOnOff(NULL,wFilter,bKeyPressureIn,KeyPressureIn);
SwitchOnOff(NULL,wFilter,bControlIn,ControlTypeIn);
SwitchOnOff(NULL,wFilter,bProgramChangeIn,ProgramTypeIn);
SwitchOnOff(NULL,wFilter,bChannelPressureIn,ChannelPressureIn);
SwitchOnOff(NULL,wFilter,bPitchBendIn,PitchBendIn);
SwitchOnOff(NULL,wFilter,bSysExIn,SysExIn);
SwitchOnOff(NULL,wFilter,bTimeCodeIn,TimeCodeIn);
SwitchOnOff(NULL,wFilter,bSongPositionIn,SongPosIn);
SwitchOnOff(NULL,wFilter,bSongSelectIn,SongSelIn);
SwitchOnOff(NULL,wFilter,bTuneRequestIn,TuneTypeIn);
SwitchOnOff(NULL,wFilter,bTimingClockIn,ClockTypeIn);
SwitchOnOff(NULL,wFilter,bStartStopIn,StartTypeIn);
SwitchOnOff(NULL,wFilter,bActiveSensingIn,ActiveSenseIn);
SwitchOnOff(NULL,wFilter,bSystemResetIn,ResetIn);

SwitchOnOff(NULL,wFilter,bNoteOnOffPass,NoteOffPass);
SwitchOnOff(NULL,wFilter,bKeyPressurePass,KeyPressurePass);
SwitchOnOff(NULL,wFilter,bControlPass,ControlTypePass);
SwitchOnOff(NULL,wFilter,bProgramChangePass,ProgramTypePass);
SwitchOnOff(NULL,wFilter,bChannelPressurePass,ChannelPressurePass);
SwitchOnOff(NULL,wFilter,bPitchBendPass,PitchBendPass);
SwitchOnOff(NULL,wFilter,bSysExPass,SysExPass);
SwitchOnOff(NULL,wFilter,bTimeCodePass,TimeCodePass);
SwitchOnOff(NULL,wFilter,bSongPositionPass,SongPosPass);
SwitchOnOff(NULL,wFilter,bSongSelectPass,SongSelPass);
SwitchOnOff(NULL,wFilter,bTuneRequestPass,TuneTypePass);
SwitchOnOff(NULL,wFilter,bTimingClockPass,ClockTypePass);
SwitchOnOff(NULL,wFilter,bStartStopPass,StartTypePass);
SwitchOnOff(NULL,wFilter,bActiveSensingPass,ActiveSensePass);
SwitchOnOff(NULL,wFilter,bSystemResetPass,ResetPass);
return(OK);
}


CheckMIDIOutPut(int channel)
{
MIDI_Event e;
int key,duration,minkey,maxkey,stepkey,ch,stop;
double r,x,kx;
unsigned long drivertime;

if (!OutMIDI) {
	Alert1("MIDI output is off,  Check the Drivers menu.");
	return(FAILED);
}

#if WITH_REAL_TIME_MIDI
if(Mute) {
	Alert1("The 'Mute' button was checked on the control pannel...");
	Mute = FALSE;
	HideWindow(Window[wInfo]);
	MaintainMenus();
	BPActivateWindow(SLOW,wControlPannel);
	return(FAILED);
	}
if(SoundOn) {
	Alert1("BP2 is already sending messages to the MIDI output...");
	return(FAILED);
	}
ResetMIDIControllers(YES,YES,YES);

sprintf(Message,"Playing test on MIDI channel %ld... (Click to stop)",(long)(channel+1));
FlashInfo(Message);
ShowMessage(TRUE,wMessage,"Tempo is set by the metronome. Click and type cmd-M to modify it.");
ResetMIDI(FALSE);
duration = (300L * Pclock) / Qclock / Time_res;
r = 3.999;
minkey = 48; maxkey = 72; stepkey = 1;
kx = (double)(maxkey - minkey) / stepkey;
x =  (double) (1 + ((unsigned) clock() % 998)) / 1000.;
key = minkey + stepkey * (int)(x * kx);
stop = FALSE;
while(TRUE) {
	PleaseWait();
	e.type = NORMAL_EVENT;
	e.time = Tcurr;
	e.status = NoteOn + channel;
	e.data1 = key;
	e.data2 = 60;
	DriverWrite(Tcurr * Time_res,0,&e);
	Tcurr += duration;
	e.type = NORMAL_EVENT;
	e.time = Tcurr;
	e.status = NoteOn + channel;
	e.data1 = key;
	e.data2 = 0;
	DriverWrite(Tcurr * Time_res,0,&e);
	x = (r * x * (1 - x));
	key = minkey + stepkey * (int)(x * kx);
	if(Button()) stop = TRUE;
	drivertime = GetDriverTime();
	while(Tcurr > drivertime) {
		/* Wait for the end of preceding play */
		PleaseWait();
		drivertime = GetDriverTime();
		if(Button()) stop = TRUE;
		}
	if(stop) break;
	}
while(Button());
ResetMIDI(FALSE);

// #ifndef __POWERPC
FlushEvents(mDownMask+mUpMask,0);
// #endif

HideWindow(Window[wInfo]);
return(OK);
#endif
}

#endif /* BP_CARBON_GUI */

SendToDriver(Milliseconds time,int nseq,int *p_rs,MIDI_Event *p_e)
{
long count = 12L;
int c0,c1,c2,status,chan;
byte midibyte;

if(Panic || EmergencyExit) return(ABORT);
if(p_e->type == RAW_EVENT || p_e->type == TWO_BYTE_EVENT) {
	if(DriverWrite(time,nseq,p_e) != noErr) return(ABORT);
	if(p_e->type == TWO_BYTE_EVENT) {
		midibyte = p_e->status;
		if(WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
		}
	midibyte = p_e->data2;
	if(WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
	*p_rs = 0;
	goto OUT;
	}
	
/* The event is NORMAL_EVENT type */

/* Don't use running status when capturing event stream, or
   when sending to any MIDI driver that does not communicate
   directly with a Serial port (eg. OMS, CoreMIDI, etc.) */
/* Currently, only the MacOS 9 "built-in" driver allows rs */
if(ItemCapture || !InBuiltDriverOn) *p_rs = 0;
status = ByteToInt(p_e->status);
chan = status % 16;
c0 = status - chan;

/* Store if volume */
if(MIDIfileOn && MIDIfileOpened) {
	if(c0 == NoteOn && CurrentVolume[chan+1] == -1)
		CurrentVolume[chan+1] = DeftVolume;
	if(c0 == ControlChange) {
		c1 = ByteToInt(p_e->data1);
		if(c1 == VolumeControl[chan+1]) {
			CurrentVolume[chan+1] = ByteToInt(p_e->data2);
			}
		}
	}
	
if(status != *p_rs || c0 == ChannelMode /* || c0 == ProgramChange */) {
	/* Send the full Midi event */
	/* Unexpectedly, D-50 seems to mess running status with ChannelMode */
	*p_rs = status;
	if(p_e->data1 > 127) {
		if(Beta) Println(wTrace,"Err. SendToDriver(). p_e->data1 > 127.");
		p_e->data1 = 127;
		}
	if(p_e->data2 > 127) {
		if(Beta) Println(wTrace,"Err. SendToDriver(). p_e->data2 > 127.");
		p_e->data2 = 127;
		}
	DriverWrite(time,nseq,p_e);
	midibyte = status;
	if(WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
	midibyte = p_e->data1;
	if(WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
	midibyte = p_e->data2;
	if(WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
	}
else {
	/* Skip the status byte, send only data ("running status") */
	/* This should probably only be used with direct Serial drivers */
	if(p_e->data1 > 127) {
		if(Beta) Println(wTrace,"Err. SendToDriver(). p_e->data1 > 127.");
		p_e->data1 = 127;
		}
	if(p_e->data2 > 127) {
		if(Beta) Println(wTrace,"Err. SendToDriver(). p_e->data2 > 127.");
		p_e->data2 = 127;
		}
	c1 = ByteToInt(p_e->data1);
	c2 = ByteToInt(p_e->data2);
	p_e->type = RAW_EVENT;
	p_e->data2 = c1;
	if(DriverWrite(time,nseq,p_e) != noErr) return(ABORT);
	
	midibyte = status;
	if(WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
	/* Normally, MIDI files accept running status, but our */
	/* procedure doesn't allow the accumulation of large streams */
	
	midibyte = c1;
	if(WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
		
	if(c0 != ChannelPressure && c0 != ProgramChange) {
		p_e->time = time / Time_res;
		p_e->type = RAW_EVENT;
		p_e->data2 = c2;
		if(DriverWrite(time,nseq,p_e) != noErr) return(ABORT);
		
		midibyte = c2;
		if(WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
		}
	else if(Beta) Alert1("Err. SendToDriver(). c0 == ChannelPressure");
	}
OUT:
return(OK);
}


AllNotesOffAllChannels(void)
{
int rs,key,channel;
long delay;
MIDI_Event e;

if(!IsMidiDriverOn()) {
	Alert1("Can't send 'AllNotesOff' because no MIDI output is active");
	return(ABORT);
	}
if(!OutMIDI) {
	Alert1("All Notes Off won't work since MIDI output is not active");	
	return(OK);
	}

#if WITH_REAL_TIME_MIDI
ShowMessage(TRUE,wMessage,"Sending AllNotesOffs and NoteOffs...");
delay = 20L;
if(NEWTIMER) {
	/* We can afford to mute the current output and send NoteOffs at a low level */
	SchedulerIsActive--;
	for(channel=0; channel < MAXCHAN; channel++) {
		PleaseWait();
		/* This is the Midi data to send:
			pkt.flags = 0;
			pkt.len = 3;
			pkt.data[0] = ControlChange + channel;
			pkt.data[1] = 123;
			pkt.data[2] = 0;
			WaitABit(delay); */
		/* $$$ write it later when NEWTIMER is 1 */
		}
	delay = 5L;
	for(channel=0; channel < MAXCHAN; channel++) {
		if(channel > 0 && CurrentMIDIprogram[channel+1] <= 0) continue;
		PleaseWait();
		for(key=0; key < 128; key++) {
			/* This is the Midi data to send:
				WaitABit(delay);
				pkt.flags = 0;
				pkt.len = 3;
				pkt.data[0] = NoteOn + channel;
				pkt.data[1] = key;
				pkt.data[2] = 0; */
			/* $$$ write it later when NEWTIMER is 1 */
			}
		}
	SchedulerIsActive++;
	WaitABit(2000L);
	}
else {
	rs = 0;
	for(channel=0; channel < MAXCHAN; channel++) {
		e.time = Tcurr;
		e.type = NORMAL_EVENT;
		e.status = ControlChange + channel;
		e.data1 = 123;
		e.data2 = 0;
		SendToDriver(Tcurr * Time_res,0,&rs,&e);
		Tcurr += 3L; /* 30ms is allowed for each AllNotesOff to take effect */
		}
	WaitForEmptyBuffer();
	
	/* Now we switch-off every note in case a NoteOn got stuck */
	for(channel=0; channel < MAXCHAN; channel++) {
		for(key=0; key < 128; key++) {
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = NoteOn + channel;
			e.data1 = key;
			e.data2 = 0;
			SendToDriver(Tcurr * Time_res,0,&rs,&e);
			}
		}
	}
ClearMessage();
#endif /* WITH_REAL_TIME_MIDI */
return(OK);
}


SetMIDIPrograms(void)
{
int i,p,rs;
MIDI_Event e;

for(i=1; i <= 16; i++) {
	p = CurrentMIDIprogram[i];
	if(p > 0 && p <= 128) {
		ChangedMIDIprogram = TRUE;
		e.time = Tcurr;
		e.type = TWO_BYTE_EVENT;
		e.status = ProgramChange + i - 1;
		e.data2 = p - 1;
		rs = 0;
#if WITH_REAL_TIME_MIDI
		if(IsMidiDriverOn() && !InitOn)
			SendToDriver(Tcurr * Time_res,0,&rs,&e);
#endif
		}
	}
return(OK);
}


/* ??? Not sure what the purpose of this functions is.  To prevent sending
   too many Midi messages ?  CoreMIDI driver sets MaxMIDIbytes = LONG_MAX,
   so not sure if this function is likely to ever do anything.
   -- akozar 20130830
 */
CheckMIDIbytes(int tell)
{
unsigned long drivertime;
long formertime,timeleft;
int rep,compiledmem;

if(!IsMidiDriverOn() || MIDIfileOn || !OutMIDI) return(OK);

if(Nbytes > (MaxMIDIbytes / 2) && Tbytes2 == ZERO) {
	HideWindow(Window[wInfo]); HideWindow(Window[wMessage]);
	Tbytes2 = Tcurr;
	}
if(Nbytes > MaxMIDIbytes) {
	HideWindow(Window[wInfo]); HideWindow(Window[wMessage]);
	drivertime = GetDriverTime();
	formertime = ZERO;
	while((timeleft = Tbytes2 - drivertime) > ZERO) {
		if((timeleft * Time_res / 1000L) != formertime && tell) {
			formertime = timeleft * Time_res / 1000L;
			sprintf(Message,"Idling (%ld sec)",(long)formertime + 1L);
			PleaseWait();
			ShowMessage(FALSE,wMessage,Message);
			}
#if BP_CARBON_GUI
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
		if((rep=MyButton(0)) != FAILED) {
			StopCount(0);
			SetButtons(TRUE);
			compiledmem = CompiledGr;
			if(rep == OK)
				while((rep = MainEvent()) != RESUME && rep != STOP && rep != EXIT);
			if(rep == RESUME) {
				rep = OK; EventState = NO;
				}
			if(rep != OK) return(rep);
			if(compiledmem && !CompiledGr) return(ABORT);
			if(LoadedIn && (!CompiledIn && (rep=CompileInteraction()) != OK))
				 return(rep);
			}
		rep = OK;
		if(EventState != NO) return(EventState);
#endif /* BP_CARBON_GUI */
		if((rep=ListenMIDI(0,0,0)) == ABORT || rep == ENDREPEAT
			|| rep == EXIT) return(rep);
		drivertime = GetDriverTime();
		}
	HideWindow(Window[wMessage]);
	Tbytes2 = ZERO; Nbytes = MaxMIDIbytes/2;
	}
return(OK);
}

int CaptureMidiEvent(Milliseconds time,int nseq,MIDI_Event *p_e)
{
	MIDIcode **ptr;
	Milliseconds currenttime;
	long size;

	size = (long) MyGetHandleSize((Handle)Stream.code);
	size = (size / sizeof(MIDIcode)) - 5L;
	if(Stream.i >= size) {
		ptr = Stream.code;
		if((ptr = (MIDIcode**)IncreaseSpace((Handle)ptr)) == NULL) return FAILED;
		Stream.code = ptr;
		}
	if(Stream.i == ZERO) {
		currenttime = ZERO;
		DataOrigin = time;
		}
	else currenttime = time - DataOrigin;
	switch(p_e->type) {
		case RAW_EVENT:
			(*Stream.code)[Stream.i].time = currenttime;
			(*Stream.code)[Stream.i].byte = ByteToInt(p_e->data2);
			(*Stream.code)[Stream.i].sequence = nseq;
			Stream.i++;
			break;
		case TWO_BYTE_EVENT:
			(*Stream.code)[Stream.i].time = currenttime;
			(*Stream.code)[Stream.i].byte = ByteToInt(p_e->status);
			(*Stream.code)[Stream.i].sequence = nseq;
			Stream.i++;
			(*Stream.code)[Stream.i].time = currenttime;
			(*Stream.code)[Stream.i].byte = ByteToInt(p_e->data2);
			(*Stream.code)[Stream.i].sequence = nseq;
			Stream.i++;
			break;
		case NORMAL_EVENT:
			(*Stream.code)[Stream.i].time = currenttime;
			(*Stream.code)[Stream.i].byte = ByteToInt(p_e->status);
			(*Stream.code)[Stream.i].sequence = nseq;
			Stream.i++;
			(*Stream.code)[Stream.i].time = currenttime;
			(*Stream.code)[Stream.i].byte = ByteToInt(p_e->data1);
			(*Stream.code)[Stream.i].sequence = nseq;
			Stream.i++;
			(*Stream.code)[Stream.i].time = currenttime;
			(*Stream.code)[Stream.i].byte = ByteToInt(p_e->data2);
			(*Stream.code)[Stream.i].sequence = nseq;
			Stream.i++;
	}
	
	return OK;
}

void RegisterProgramChange(MIDI_Event *p_e)
{
	int j, thisevent, channel, program;
	short itemtype;
	ControlHandle itemhandle;
	Rect r;
	
	// Register program change to the MIDI orchestra
	channel = ByteToInt(p_e->status) % 16;
	thisevent = ByteToInt(p_e->status) - channel;
	if(thisevent == ProgramChange) {
		program = ByteToInt(p_e->data2) + 1;
		if(CurrentMIDIprogram[channel+1] != program) {
#if BP_CARBON_GUI
			if(TestMIDIChannel == (channel+1) && CurrentMIDIprogram[TestMIDIChannel] > 0) {
				GetDialogItem(MIDIprogramPtr, (short)CurrentMIDIprogram[TestMIDIChannel],
							&itemtype, (Handle*)&itemhandle, &r);
				if(itemhandle != NULL) HiliteControl((ControlHandle) itemhandle,0);
				GetDialogItem(MIDIprogramPtr, (short)program, &itemtype, (Handle*)&itemhandle, &r);
				if(itemhandle != NULL) HiliteControl((ControlHandle) itemhandle,kControlButtonPart);
				WritePatchName();
				}
#endif /* BP_CARBON_GUI */
			CurrentMIDIprogram[channel+1] = program;
#if BP_CARBON_GUI
			for(j=0; j < 128; j++) {
				if((*p_GeneralMIDIpatchNdx)[j] == program) {
					sprintf(Message,"[%ld] %s",(long)program,*((*p_GeneralMIDIpatch)[j]));
					SetField(NULL,wMIDIorchestra,(channel+1),Message);
					break;
				}
			}
#endif /* BP_CARBON_GUI */
		}
	}
}
