/* Interaction.c (BP2 version CVS) */

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


ResetInteraction(void)
{
int i;

DeriveFurtherChan = ResetWeightChan = PlayChan = RepeatChan = EndRepeatChan
	= EverChan = QuitChan = UseEachSubChan = SynchronizeStartChan = SpeedChan
	= SetTimeChan = StriatedChan = NoConstraintChan = SkipChan = AgainChan
	= TclockChan = MuteOnChan = MuteOffChan = -1;
for(i=1; i <= MAXWAIT; i++) WaitChan[i] = WaitKey[i] = -1;
for(i=1; i < MAXPARAMCTRL; i++) ParamChan[i] = ParamControl[i] = ParamKey[i] = -1;
for(i=0; i < Maxinscript; i++) ((*p_INscript)[i]).chan = -1;
Jinscript = 0;
ParamControlChan = -1;
return(DoSystem());
}


CompileInteraction(void)
{
int r;

// This is a bit tricky: CompileInteraction() is invoked by BPActivateWindow()
// if CompiledIn is false. But RunScript() also invokes BPActivateWindow().
// Therefore, it is important to set CompiledIn to true until the end of the procedure.

ResetInteraction();
CompiledIn = TRUE;

CompileOn++;

r = RunScript(wInteraction,YES);

if(CompileOn) CompileOn--;

if(r == EXIT) return(r);
if(r == OK) SelectBehind(ZERO,ZERO,TEH[wInteraction]);
else {
	Print(wTrace,"\nStopped compiling interaction.\n");
	CompiledIn = FALSE;
	}
ShowSelect(CENTRE,wInteraction);
return(r);
}


PrintInteraction(int w)
{
int i;
char line[MAXLIN];


if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. PrintInteraction(). Incorrect window index");
	return(FAILED);
	}
SetSelect(ZERO,GetTextLength(w),TEH[w]);
TextDelete(w); SetVScroll(w);

PrintBehindln(w,*(p_ScriptLabelPart(110,0)));
switch(NoteConvention) {
	case ENGLISH: i = 2; break;
	case FRENCH: i = 1; break;
	case INDIAN: i = 88; break;
	case KEYS: i = 3; break;
	}
PrintBehindln(w,*(p_ScriptLabelPart(i,0)));

if(SynchronizeStartChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(97,0)));
	PrintBehind(w,line);
	PrintNote(SynchronizeStartKey,SynchronizeStartChan,w,line);
	PrintBehind(w,"\n");
	}
if(PlayChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(91,0)));
	PrintBehind(w,line);
	PrintNote(PlayKey,PlayChan,w,line);
	PrintBehind(w,"\n");
	}
if(MuteOnChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(188,0)));
	PrintBehind(w,line);
	PrintNote(MuteOnKey,MuteOnChan,w,line);
	PrintBehind(w,"\n");
	}
if(MuteOffChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(189,0)));
	PrintBehind(w,line);
	PrintNote(MuteOffKey,MuteOffChan,w,line);
	PrintBehind(w,"\n");
	}
if(RepeatChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(92,0)));
	PrintBehind(w,line);
	PrintNote(RepeatKey,PlayChan,w,line);
	PrintBehind(w,"\n");
	}
if(EndRepeatChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(93,0)));
	PrintBehind(w,line);
	PrintNote(EndRepeatKey,EndRepeatChan,w,line);
	PrintBehind(w,"\n");
	}
if(EverChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(94,0)));
	PrintBehind(w,line);
	PrintNote(EverKey,EverChan,w,line);
	PrintBehind(w,"\n");
	}
if(QuitChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(95,0)));
	PrintBehind(w,line);
	PrintNote(QuitKey,QuitChan,w,line);
	PrintBehind(w,"\n");
	}
if(DeriveFurtherChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(89,0)));
	PrintBehind(w,line);
	PrintNote(DeriveFurtherKey,DeriveFurtherChan,w,line);
	PrintBehind(w,"\n");
	}
if(ResetWeightChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(90,0)));
	PrintBehind(w,line);
	PrintNote(ResetWeightKey,ResetWeightChan,w,line);
	PrintBehind(w,"\n");
	}
if(UseEachSubChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(96,0)));
	PrintBehind(w,line);
	PrintNote(UseEachSubKey,UseEachSubChan,w,line);
	PrintBehind(w,"\n");
	}
if(SetTimeChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(99,0)));
	PrintBehind(w,line);
	PrintNote(SetTimeKey,SetTimeChan,w,line);
	PrintBehind(w,"\n");
	}
if(StriatedChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(100,0)));
	PrintBehind(w,line);
	PrintNote(StriatedKey,StriatedChan,w,line);
	PrintBehind(w,"\n");
	}
if(NoConstraintChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(101,0)));
	PrintBehind(w,line);
	PrintNote(NoConstraintKey,NoConstraintChan,w,line);
	PrintBehind(w,"\n");
	}
if(SkipChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(102,0)));
	PrintBehind(w,line);
	PrintNote(SkipKey,SkipChan,w,line);
	PrintBehind(w,"\n");
	}
if(AgainChan != -1) {
	sprintf(line,"%s ",*(p_ScriptLabelPart(103,0)));
	PrintBehind(w,line);
	PrintNote(AgainKey,AgainChan,w,line);
	PrintBehind(w,"\n");
	}
if(SpeedChan != -1) {
	sprintf(line,"%s %ld %s %ld %s %f",*(p_ScriptLabelPart(98,0)),(long)SpeedCtrl,
		*(p_ScriptLabelPart(98,1)),(long)SpeedChan,*(p_ScriptLabelPart(98,2)),SpeedRange);
	PrintBehindln(w,line);
	}
if(TclockChan != -1) { 
	sprintf(line,"%s %ld %s %ld %s ",*(p_ScriptLabelPart(106,0)),(long)MinQclock,
		*(p_ScriptLabelPart(106,1)),(long)MinPclock,*(p_ScriptLabelPart(106,2)));
	PrintBehind(w,line);
	PrintNote(MinTclockKey,TclockChan,-1,line);
	i = 0; while(!MySpace(line[i])) {
		Message[i] = line[i]; i++;
		}
	Message[i] = '\0';
	PrintBehind(w,Message);
	sprintf(line," %s %ld %s %ld %s ",*(p_ScriptLabelPart(106,3)),(long)MaxQclock,
		*(p_ScriptLabelPart(106,4)),(long)MaxPclock,*(p_ScriptLabelPart(106,5)));
	PrintBehind(w,line);
	PrintNote(MaxTclockKey,TclockChan,w,line);
	PrintBehind(w,"\n");
	}
for(i=1; i <= MAXWAIT; i++) {
	if(WaitChan[i] != -1 && WaitKey[i] != -1) {
		sprintf(line,"%s W%ld %s ",*(p_ScriptLabelPart(104,0)),(long)i,
			*(p_ScriptLabelPart(104,1)));
		PrintBehind(w,line);
		PrintNote(WaitKey[i],WaitChan[i],w,line);
		PrintBehind(w,"\n");
		}
	}
for(i=1; i < MAXPARAMCTRL; i++) {
	if(ParamChan[i] != -1 && ParamControl[i] != -1) {
		sprintf(line,"%s K%ld %s%ld %s %ld",*(p_ScriptLabelPart(105,0)),(long)i,
			*(p_ScriptLabelPart(105,1)),
			(long)ParamControl[i],*(p_ScriptLabelPart(105,2)),(long)ParamChan[i]);
		PrintBehindln(w,line);
		}
	if(ParamChan[i] != -1 && ParamKey[i] != -1) {
		sprintf(line,"%s K%ld %s ",*(p_ScriptLabelPart(15,0)),(long)i,
			*(p_ScriptLabelPart(15,1)));
		PrintBehind(w,line);
		PrintNote(ParamKey[i],ParamChan[i],w,line);
		PrintBehind(w,"\n");
		}
	}
for(i=1; i <= Jinscript; i++) {
	if(((*p_INscript)[i]).chan != -1) {
		sprintf(line,"%s ",*(p_ScriptLabelPart(161,0)));
		PrintBehind(w,line);
		PrintNote(((*p_INscript)[i]).key,((*p_INscript)[i]).chan,w,line);
		MystrcpyTableToString(MAXLIN,Message,p_Script,((*p_INscript)[i]).scriptline);
		sprintf(line," %s %s\n",*(p_ScriptLabelPart(161,2)),Message);
		PrintBehind(w,line);
		}
	}
SelectBehind(ZERO,ZERO,TEH[w]);
ShowSelect(CENTRE,w);
Dirty[wInteraction] = FALSE;
return(OK);
}


UpdateInteraction(void)
{
if(!LoadedIn) return(OK);
if(!CompiledIn) CompileInteraction();
if(!CompiledIn) return(FAILED);
PrintInteraction(wInteraction);
return(OK);
}

// ----------------------- VARIOUS "WAIT" PROCEDURES -----------------------

WaitForNoteOn(int channel,int thekey)
{
int statusbyte,notenum,octave,c0,c1,c2,r;
MIDI_Event e;
long count = 12L;

if(!OkWait) return(OK);

FlushEvents(everyEvent,0);

if(channel > 0 && thekey > -1) statusbyte = NoteOn + channel - 1;
else statusbyte = thekey = 0;

while(Button());
if(statusbyte == 0) FlashInfo("Waiting for click...");
else {
	FlashInfo("Waiting for ");
	PrintNote(thekey,channel,wInfo,Message);
	Print(wInfo," (or click)");
	}

SetCursor(&KeyboardCursor);
do {
	if((r=GetNextMIDIevent(&e,TRUE,FALSE)) == FAILED) continue;
	if(r == ABORT) goto END;
	c0 = e.data2;
	if(c0 < NoteOn || c0 >= (NoteOn+16)) continue;
	if((r=GetNextMIDIevent(&e,TRUE,FALSE)) == FAILED) continue;
	if(r == ABORT) goto END;
	c1 = e.data2;
	if((r=GetNextMIDIevent(&e,TRUE,FALSE)) == FAILED) continue;
	if(r == ABORT) goto END;
	c2 = e.data2;
	if(QuitChan > 0 && c1 == QuitKey && c0 == (NoteOn+QuitChan-1) && c2 > 0) {
		SkipFlag = FALSE;
		ShowMessage(TRUE,wMessage,"Abort!");
		r = ABORT; goto END;
		}
	if(statusbyte == 0) continue;
	if(statusbyte != c0) continue;
	if(thekey != c1) continue;
	if(c2 == 0) continue;
	break;
	}
while(!Button());
HideWindow(Window[wInfo]);
while(Button());
StopWait();

END:
r = RESUME;
FlushEvents(everyEvent,0);
return(r);
}


WaitForTags(p_list **waitlist)
{
int x,statusbyte,thekey,notenum,octave,c0,c1,r;
p_list **ptag;
MIDI_Event e;
long count = 12L;

if(!OkWait) return(OK);
ptag = waitlist;
RunningStatus = 0;
do {
	x = (**ptag).x;
	ptag = (**ptag).p;
	thekey = WaitKey[x];
	if((r=WaitForNoteOn(WaitChan[x],thekey)) != OK && r != RESUME) return(r);
	}
while(ptag != NULL);
while(Button());
RunningStatus = 0;
return(OK);
}


WaitKeyStrokeOrAppleEvent(char thechar,int command,int eventtype,AEEventClass theclass,
	AEEventID theID,char* classname,char* IDname)
{
EventRecord event;
int eventfound,exclusive,themessage;
AEEventID id;
MIDI_Event e;

if(eventtype == KEYBOARDEVENT && thechar == '\0') return(FAILED);
if(!OkWait) return(OK);

while(Button());

switch(eventtype) {
	case APPLEVENT:
		sprintf(Message,"Waiting for Apple Event class '%s' ID '%s'. Click mouse to cancel",
			classname,IDname);
		SetCursor(&WatchCursor);
		break;
	case STARTEVENT:
		sprintf(Message,"Waiting for 'Start' MIDI message. Click mouse to cancel");
		SetCursor(&KeyboardCursor);
		break;
	case STOPEVENT:
		sprintf(Message,"Waiting for 'Stop' MIDI message. Click mouse to cancel");
		SetCursor(&KeyboardCursor);
		break;
	case CONTINUEEVENT:
		sprintf(Message,"Waiting for 'Continue' MIDI message. Click mouse to cancel");
		SetCursor(&KeyboardCursor);
		break;
	case KEYBOARDEVENT:
		FlushEvents(everyEvent,0);
		if(!command)
			sprintf(Message,"Waiting for '%c' or click",thechar);
		else
			sprintf(Message,"Waiting for 'cmd-%c' or click",thechar);
		SetCursor(&WatchCursor);
		break;
	}
FlashInfo(Message);

exclusive = FALSE;
while(TRUE) {
	if(Button()) break;
	switch(eventtype) {
		case STARTEVENT:
		case STOPEVENT:
		case CONTINUEEVENT:
			if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) continue;
			if(e.type != RAW_EVENT) continue;
			themessage = ByteToInt(e.data2);
			if(!exclusive && themessage == Start && eventtype == STARTEVENT) goto GOTIT;
			if(!exclusive && themessage == Stop && eventtype == STOPEVENT) goto GOTIT;
			if(!exclusive && themessage == Continue && eventtype == CONTINUEEVENT) goto GOTIT;
			if(themessage == SystemExclusive) exclusive = TRUE;
			if(themessage == EndSysEx) exclusive = FALSE;
			break;
		case APPLEVENT:
		case KEYBOARDEVENT:
			eventfound = WaitNextEvent(everyEvent,&event,30L,NULL);
			if(!eventfound) continue;
			if(eventtype == APPLEVENT) {
				if(event.what == kHighLevelEvent && event.message == theclass) {
					id = (*((AEEventID*) (&(event.where))));
					if(id == theID) goto GOTIT;
					}
				}
			else {
				if(event.what == keyDown) {
					if(thechar == (char) (event.message & charCodeMask)) {
						if((!command && ((event.modifiers & cmdKey) == 0))
								|| (command && ((event.modifiers & cmdKey) != 0)))
							goto GOTIT;
						}
					}
				}
			break;
		}
	}

GOTIT:
HideWindow(Window[wInfo]);

FlushEvents(everyEvent,0);

return(OK);
}