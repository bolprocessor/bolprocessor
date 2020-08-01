/* MIDIloads.c (BP2 version CVS) */ 

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

#if 0	/* this is never called */
DecodeStampedMIDI(int w1, int w2)
{
char c;
unsigned long i=ZERO,im,j,kk,t,n;
int r=OK,space,start,k,buff[3],nitem,runningstatus;

SetCursor(&WatchCursor);
im = GetTextLength(w1);
j = GetTextLength(w2);
SetSelect(j,j,TEH[w2]);	/* Append output to text in window w2 */
BPActivateWindow(QUICK,w2);
for(i=ZERO,space=TRUE,start=TRUE,k=0,nitem=1,runningstatus=0,
		buff[0] = 0; i < im; i++) { 
	c = GetTextChar(w1,i);
	if(c == '\r' || c == '\n') {
		nitem++;
		sprintf(Message,"Attempting to read item #%ld",(long)nitem);
		ShowMessage(TRUE,wMessage,Message);
		Print(w2,"\n"); space = start = TRUE; k = 0;
		continue;
		}
	if(MySpace(c)) {
		space = TRUE;
		if(!start) {
			Eucl(n,256L,&t,&kk);
			if(kk > 127) {
				runningstatus = TRUE; buff[0] = (int) kk; k = 1;
				continue;
				}
			buff[k] = (int) kk; k++;
			if(k == 3) {
				k = runningstatus;
				if(buff[0] < 128) {
					Print(w2," Status? ");
					ShowMessage(TRUE,wMessage,"Incorrect status byte");
					}
				else ReadNoteOn(buff[0],buff[1],buff[2],w2);
				}
			}
		continue;
		}
	if(!isdigit(c)) {
		ShowMessage(TRUE,wMessage,"By-passing non numeric characters...");
		Print(w2," ? "); space = start = TRUE; k = 0; continue;
		}
	if(space) {
		n = c - '0';
		space = FALSE;
		}
	else {
		n = 10L * n + (long) (c - '0');
		start = FALSE;
		}
	}
END:
Print(w2,"\n");
return(r);
}
#endif

#if BP_CARBON_GUI

ReadNoteOn(int c0, int c1, int c2, int wind)
{
int notenum,octave,overflow;
// MIDI_Parameters parms;
unsigned long time;

if(wind < 0 || wind >= WMAX || !Editable[wind]) return(FAILED);
if(!IsMidiDriverOn()) {
	if(Beta) Alert1("Err. ReadNoteOn(). Driver is OFF");
	return(ABORT);
	}
if(NoteOn <= c0  &&  c0 < (NoteOn+16) && c2 > 0) {   /* NoteOn */
	if(PlayTicks && ReadKeyBoardOn && Jcontrol == -1) {
		PlayTick(TRUE);
		}
	if(SplitTimeObjects && !EmptyBeat) Print(wind," ");
/*	c1 -= (C4key - 60); */
	if(TransposeInput) c1 += TransposeValue;
	while(c1 < 0) c1 += 12;
	while(c1 > 127) c1 -= 12;
	PrintNote(c1,-1,wind,Message);
	FoundNote = TRUE;
	TickDone = EmptyBeat = FALSE;
	return(OK);
	}
else return(FAILED);
}

// I'm not sure whether we need LoadTimePattern() and LoadMIDIsyncOrKey()
// in ANSI console build, but seems unlikely.  - akozar

LoadTimePattern(int wind)
{
EventRecord event;
MIDI_Event e;
/* MIDI_Parameters parms;
long count = 12L */
long pos,posmax,pos1,pos2,imax;
double p,q;
Milliseconds period,**p_t,date;
MIDIcode **p_b,**p_c;
long i,im,im2,ibyte,nbytes;
int ipatt,c0,c1,c2,rep,d,isthere,gap,result;
double ratio;
Rect r;
Handle itemhandle;
short item,itemtype;
char c,**p_line,name[MAXFIELDCONTENT];
Str255 t;
Handle ptr;
unsigned long drivertime;

p_b = p_c = NULL;
if(!IsMidiDriverOn()) {
	if(Beta) Alert1("Err. LoadTimePattern(). Driver is OFF");
	return(ABORT);
	}

#if !WITH_REAL_TIME_MIDI
  return(ABORT);
#else
if(!OutMIDI) {
	OutMIDI = TRUE;
	SetButtons(TRUE);
	}
if(ComputeOn || PolyOn || CompileOn || SoundOn || SelectOn ||
	SetTimeOn || GraphicOn || PrintOn || ReadKeyBoardOn || HangOn || ScriptExecOn)
	return(FAILED);
if(wind < 0 || wind >= WMAX || !Editable[wind]) return(FAILED);
EndWriteScript();
BPActivateWindow(SLOW,wind);

if((rep=CompilePatterns()) != OK) return(rep);
ipatt = Jpatt;
UpdateWindow(FALSE,Window[wind]); /* Update text length */
ShowSelect(CENTRE,wind);
pos = pos1 = ZERO; isthere = FALSE;
posmax = GetTextLength(wind);
while(posmax > 0 && GetTextChar(wind,posmax-1) == '\r') posmax--;
p_line = NULL;
while(ReadLine(YES,wind,&pos,posmax,&p_line,&gap) == OK) {
	if((*p_line)[0] == '\0') {
		pos1 = pos; continue;
		}
	if(Mystrcmp(p_line,"DATA:") == 0) goto OUT;
	if(Mystrcmp(p_line,"COMMENT:") == 0) goto OUT;
	if(Mystrcmp(p_line,"TIMEPATTERNS:") == 0) {
		isthere = TRUE; goto OUT;
		}
	pos1 = pos;
	}
pos1 = posmax + 1L;

OUT:
pos1 -= 1L;
SetSelect(pos1,pos1,TEH[wind]);
if(!isthere) Print(wind,"\nTIMEPATTERNS:\n------------");
pos2 = pos1 + 15L;
HideWindow(Window[wMessage]);
ResetMIDI(FALSE);
FlushEvents(everyEvent,0);
imax = 200L;
if((p_b = (MIDIcode**) GiveSpace((Size)imax * sizeof(MIDIcode))) == NULL)
	return(ABORT);
Alert1("Click 'OK'. Then play notes on MIDI keyboard. Click mouse to terminate");
ReadKeyBoardOn = TRUE; MaintainCursor();
FlashInfo("Play notes.  Click mouse to terminate.");

while(Button());
i = ZERO;
while(!Button()) {
	if((result=GetNextMIDIevent(&e,TRUE,FALSE)) == FAILED) continue;
	if(result == ABORT) break;
	drivertime = GetDriverTime();
	(*p_b)[i].byte = ByteToInt(e.data2);
	(*p_b)[i].time = drivertime;
	if(++i >= imax) {
		if(ThreeOverTwo(&imax) != OK) goto END;
		if((p_b = (MIDIcode**) IncreaseSpace((Handle)p_b)) == NULL)
			return(ABORT);
		}
	}

ReadKeyBoardOn = FALSE;

im2 = 2L * imax;
if((p_c = (MIDIcode**) GiveSpace((Size) im2 * sizeof(MIDIcode))) == NULL)
	return(ABORT);

if(FormatMIDIstream(p_b,imax,p_c,TRUE,im2,&nbytes,TRUE) != OK) return(FAILED);	
	
if((p_t = (Milliseconds**) GiveSpace((Size)im2 * sizeof(Milliseconds))) == NULL)
	return(ABORT);

for(ibyte=i=ZERO; ibyte < nbytes; ibyte++) {
	c0 = (*p_c)[ibyte].byte;
	date = (*p_c)[ibyte].time;
	if(c0 < NoteOn || c0 >= (NoteOn+16)) continue;
	ibyte += 2;
	c2 = (*p_c)[ibyte].byte;
	if(c2 > 0) { 	/* NoteOn */
		if(i == 0 || date > (*p_t)[i-1]) (*p_t)[i++] = date;
		if(i >= imax) {
			if((p_t = (Milliseconds**) IncreaseSpace((Handle)p_t)) == NULL) {
				MyDisposeHandle((Handle*)&p_line);
				HideWindow(Window[wInfo]);
				ReadKeyBoardOn = FALSE;
				return(ABORT);
				}
			imax = (imax * 3L) / 2L;
			}
		}
	}

im = i;
HideWindow(Window[wInfo]);
/* $$$$ if(im < 2) {
	Alert1("No data received...");
	goto END;
	} */
for(i=0; i < 4; i++) MainEvent();
StopWait();
d = im - 1;
if (d < 1)  d = 1;
sprintf(Message, "%d", d);
SetField(PatternPtr, wUnknown, fPatternDuration, Message);
SelectField(PatternPtr, wUnknown, fPatternName, TRUE);

TRY:
ShowWindow(GetDialogWindow(PatternPtr)); 
SelectWindow(GetDialogWindow(PatternPtr)); // was BringToFront() - akozar 052107
GetDialogItem(PatternPtr,fPatternName,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXNAME,t,name);
rep = FAILED;
while(TRUE) {
	MaintainCursor();
	ModalDialog((ModalFilterUPP) 0L,&item);
	switch(item) {
		case dPatternCancel:
			rep = ABORT; break;
		case dPatternOK:
			rep = OK; break;
		case fPatternName:
			break;
		case fPatternDuration:
			break;
		}
	if(rep == OK || rep == ABORT) break;
	}
HideWindow(GetDialogWindow(PatternPtr));
if(rep == ABORT) goto END;
// reading the text boxes was moved from the switch cases above
// for efficiency and to prevent bug where LineBuff is not changed - akozar 052107
GetDialogItem(PatternPtr,fPatternName,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXNAME,t,name);
GetDialogItem(PatternPtr,fPatternDuration,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXNAME,t,LineBuff);
d = (int) atol(LineBuff);	/* Don't use atoi() because int's could be 4 bytes */
if(strlen(name) > 0) {
	if(!isupper(name[0])) {
		Alert1("Pattern name is a variable and should start with uppercase character");
		goto TRY;
		}
	for(i=0; i < strlen(name); i++) {
		c = name[i];
		if(!isalnum(c)) {
			sprintf(Message,
		"Space or incorrect character in name.  Can't accept '%c'",c);
			Alert1(Message);
			goto TRY;
			}
		}
	}
for(i=0; i < strlen(LineBuff); i++) { // FIXME: This check seems unnecessary - akozar
	c = LineBuff[i];
	if(!isdigit(c)) {
		sprintf(Message,
		 "Unexpected character '%c'.\nThe symbolic duration must be a positive integer.",c);
		Alert1(Message);
		goto TRY;
		}
	}
if(d <= 0) {
	Alert1("The symbolic duration must be a positive integer.");
	goto TRY;
	}
	
period = ((*p_t)[im-1] - (*p_t)[0]) / d;
SetSelect(pos2,pos2,TEH[wind]);
for(i=1; i < im; i++) {
	if(Simplify((double)INT_MAX,(double)((*p_t)[i] - (*p_t)[i-1]),(double)period,&p,&q) != OK)
		goto END;
	ipatt++;
	sprintf(Message,"t%ld = %u/%u  ",(long)ipatt,(unsigned long)p,
		(unsigned long)q);
	Print(wind,Message); UpdateDirty(TRUE,wAlphabet);
	if((i % 5) == 0) {
		Print(wind,"\n"); isthere = TRUE;
		}
	else isthere = FALSE;
	}
if(!isthere) Print(wind,"\n");
if(strlen(name) > 0) {
	SetSelect(pos1,pos1,TEH[wind]);
	sprintf(Message,"\n%s -->",name);
	Print(wind,Message);
	ipatt = Jpatt;
	for(i=1; i < im; i++) {
		ipatt++;
		sprintf(Message," t%ld",(long)ipatt);
		Print(wind,Message);
		}
	}

END:
ShowSelect(CENTRE,wind);
MyDisposeHandle((Handle*)&p_t);
MyDisposeHandle((Handle*)&p_line);
MyDisposeHandle((Handle*)&p_b);
MyDisposeHandle((Handle*)&p_c);
return(OK);
#endif
}


LoadMIDIsyncOrKey(void)
{
EventRecord event;
MIDI_Event e;
int i,j,r,c0,c1,c2,rep,channel,eventfound;
char thechar;

BPActivateWindow(SLOW,wScript);
SetCursor(&WatchCursor);
#if WITH_REAL_TIME_MIDI
  ResetMIDI(FALSE);
#endif

FlushEvents(everyEvent,0);

FlashInfo("Waiting for keystroke or MIDI code. (Click to exit)");
i = 0;
ReadKeyBoardOn = TRUE;
while(!Button()) {
	MaintainCursor();
#if WITH_REAL_TIME_MIDI
	if((r=GetNextMIDIevent(&e,FALSE,FALSE)) == FAILED) goto EVENT;
	if(r == ABORT) break;
	c0 = e.data2;
	if(c0 < NoteOn || c0 >= (NoteOn+16)) goto EVENT;
	if((r=GetNextMIDIevent(&e,FALSE,FALSE)) == FAILED) goto EVENT;
	if(r == ABORT) break;
	c1 = e.data2;
	if((r=GetNextMIDIevent(&e,FALSE,FALSE)) == FAILED) goto EVENT;
	if(r == ABORT) break;
	c2 = e.data2;
	if(c2 > 0) { 	/* NoteOn */
		switch(NoteConvention) {
			case ENGLISH: j = 2; break;
			case FRENCH: j = 1; break;
			case INDIAN: j = 88; break;
			case KEYS: j = 3; break;
			}
		PrintBehindln(wScript,*(p_ScriptLabelPart(j,0)));
		channel = c0 - NoteOn + 1;
		PrintNote(c1,channel,-1,Message);
		MystrcpyStringToTable(ScriptLine.arg,0,Message);
		AppendScript(14);
		break;
		}
#endif
EVENT:
	eventfound = WaitNextEvent(everyEvent,&event,30L,NULL);
	if(eventfound && (event.what == keyDown)) {
		thechar = event.message & charCodeMask;
		if(thechar == ' ') {
			MystrcpyStringToTable(ScriptLine.arg,0,"space");
			AppendScript(14);
			break;
			}
		if((event.modifiers & cmdKey) != 0)
			strcpy(Message,"cmd-");
		else
			Message[0] = '\0';
		MystrcpyStringToTable(ScriptLine.arg,0,Message);
		Message[0] = thechar;
		Message[1] = '\0';
		strcat((*((*(ScriptLine.arg))[0])),Message);
		AppendScript(14);
		break;
		}
	}
HideWindow(Window[wInfo]);
ReadKeyBoardOn = FALSE;

FlushEvents(mDownMask+mUpMask,0);

return(OK);
}

#endif /* BP_CARBON_GUI */

PrintNote(int key,int channel,int wind,char* line)
{
int notenum,octave;
char channelstring[11];

if(key < 0) {
	if(wind >= 0) PrintBehind(wind,"<void>");
	strcpy(line,"<void>");
	return(OK);
	}
key -= (C4key - 60);
notenum = key % 12;
octave = (key - notenum) / 12;
channelstring[0] = '\0';
if(channel > 0) sprintf(channelstring," channel %ld",(long)channel);
switch(NoteConvention) {
	case FRENCH:
		octave -= 2;
		switch(octave) {
			case -2:
				if(NameChoice[notenum] == 0)
					sprintf(line,"%s000%s",Frenchnote[notenum],channelstring);
				else
					sprintf(line,"%s000%s",AltFrenchnote[notenum],channelstring);
				break;
			case -1:
				if(NameChoice[notenum] == 0)
					sprintf(line,"%s00%s",Frenchnote[notenum],channelstring);
				else
					sprintf(line,"%s00%s",AltFrenchnote[notenum],channelstring);
				break;
			default:
				if(NameChoice[notenum] == 0)
					sprintf(line,"%s%ld%s",Frenchnote[notenum],(long)octave,channelstring);
				else
					sprintf(line,"%s%ld%s",AltFrenchnote[notenum],(long)octave,channelstring);
				break;
			}
		break;
	case ENGLISH:
		octave--;
		switch(octave) {
			case -1:
				if(NameChoice[notenum] == 0)
					sprintf(line,"%s00%s",Englishnote[notenum],channelstring);
				else
					sprintf(line,"%s00%s",AltEnglishnote[notenum],channelstring);
				break;
			default:
				if(NameChoice[notenum] == 0)
					sprintf(line,"%s%ld%s",Englishnote[notenum],(long)octave,channelstring);
				else
					sprintf(line,"%s%ld%s",AltEnglishnote[notenum],(long)octave,channelstring);
				break;
			}
		break;
	case INDIAN:
		octave--;
		switch(octave) {
			case -1:
				if(NameChoice[notenum] == 0)
					sprintf(line,"%s00%s",Indiannote[notenum],channelstring);
				else
					sprintf(line,"%s00%s",AltIndiannote[notenum],channelstring);
				break;
			default:
				if(NameChoice[notenum] == 0)
					sprintf(line,"%s%ld%s",Indiannote[notenum],(long)octave,channelstring);
				else
					sprintf(line,"%s%ld%s",AltIndiannote[notenum],(long)octave,channelstring);
				break;
			}
		break;
	case KEYS:
		sprintf(line,"%s%ld%s",KeyString,(long)key,channelstring);
		break;
	}
if(wind >= 0) {
	PrintBehind(wind,line);
#if BP_CARBON_GUI
	UpdateDirty(TRUE,wind);
#endif /* BP_CARBON_GUI */
	}
return(OK);
}


GetNote(char* line,int* p_thekey,int* p_channel,int ignorechannel)
{
char *p,*q,line2[MAXLIN];
int i,j,notenum,octave,l;

i = j = 0; while(MySpace(line[i])) i++;
strcpy(line2,line);
if(NoteConvention == KEYS) {
	while(line[i] != '\0' && line[i] == KeyString[j]) {
		i++; j++;
		}
	if(KeyString[j] != '\0') return(FAILED);
	*p_thekey = GetInteger(YES,line2,&i);
	if(*p_thekey == INT_MAX) return(FAILED);
	}
else {
	for(notenum=0; notenum < 12; notenum++) {
		p = &line2[i];
		switch(NoteConvention) {
			case FRENCH: {
				q = &Frenchnote[notenum][0]; l = strlen(Frenchnote[notenum]);
				if(Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				p = &line2[i];
				q = &AltFrenchnote[notenum][0]; l = strlen(AltFrenchnote[notenum]);
				if(Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				break;
				}
			case ENGLISH: {
				q = &Englishnote[notenum][0]; l = strlen(Englishnote[notenum]);
				if(Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				p = &line2[i];
				q = &AltEnglishnote[notenum][0]; l = strlen(AltEnglishnote[notenum]);
				if(Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				break;
				}
			case INDIAN:
				q = &Indiannote[notenum][0]; l = strlen(Indiannote[notenum]);
				if(Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				p = &line2[i];
				q = &AltIndiannote[notenum][0]; l = strlen(AltIndiannote[notenum]);
				if(Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				break;
			}
		}
	return(FAILED);
CONT:
	while(!isdigit(line[i]) && line[i] != '\0') i++;
	if(NoteConvention == FRENCH) {
		if(line[i] == '0' && line[i+1] == '0' && line[i+2] == '0') {
			octave = 0; i += 3;
			goto CONT2;
			}
		if(line[i] == '0' && line[i+1] == '0') {
			octave = 1; i += 2;
			goto CONT2;
			}
		}
	if(NoteConvention == ENGLISH || NoteConvention == INDIAN) {
		if(line[i] == '0' && line[i+1] == '0') {
			octave = 0; i += 2;
			goto CONT2;
			}
		}
	if((octave=GetInteger(YES,line2,&i)) == INT_MAX) return(FAILED);
	if(NoteConvention == FRENCH) octave += 2;
	if(NoteConvention == ENGLISH || NoteConvention == INDIAN) octave++;
CONT2:
	*p_thekey = 12 * octave + notenum;
	*p_thekey += (C4key - 60);
	}
if(ignorechannel) return(OK);
while(MySpace(line[i])) i++;
strcpy(Message,"channel");
p = &line2[i]; q = &(Message[0]);
if(!Match(FALSE,&p,&q,strlen(Message))) return(FAILED);
while(!isdigit(line[i]) && line[i] != '\0') i++;
if((*p_channel=GetInteger(YES,line2,&i)) == INT_MAX) return(FAILED);
return(OK);
}

#if BP_CARBON_GUI
// I'm not sure whether we need LoadRawData()
// in ANSI console build, but seems unlikely.  - akozar

LoadRawData(long *p_im)
{
EventRecord event;
int eventfound,error;
MIDI_Event e;
long i,imax,oldfilter;
Milliseconds period;
int r;
char c;
Str255 t;

#if !WITH_REAL_TIME_MIDI
  return(FAILED);
#else
r = FAILED; error = NO;
oldfilter = MIDIinputFilter;
SetCursor(&WatchCursor);
if(ComputeOn || PolyOn || CompileOn || SoundOn || SelectOn || SetTimeOn || GraphicOn
	|| PrintOn || ReadKeyBoardOn || HangOn || ScriptExecOn) return(FAILED);
imax = 120000L; 
if((p_Code = (MIDIcode**) GiveSpace((Size)imax * sizeof(MIDIcode))) == NULL)
	return(ABORT);
HideWindow(Window[wMessage]);
FlashInfo("Receiving MIDI data. Click mouse when completed.");
MIDIinputFilter = 0xffffffffL;
ResetMIDI(FALSE);

FlushEvents(everyEvent,0);

while(Button());
i = ZERO;
while(!Button()) {
	if((r=GetNextMIDIevent(&e,TRUE,FALSE)) == FAILED) continue;
	if(r == ABORT) break;
	(*p_Code)[i].byte = ByteToInt(e.data2);
	(*p_Code)[i].sequence = 0;
	if(++i >= imax) {
		if(ThreeOverTwo(&imax) != OK) goto END;
		if((p_Code = (MIDIcode**) IncreaseSpace((Handle)p_Code)) == NULL)
			return(ABORT);
		}
	}
	
FlushEvents(mDownMask+mUpMask,0);

(*p_im) = i;
HideWindow(Window[wMessage]);
if((*p_im) < 2L) {
	Alert1("No data received..."); goto END;
	}
r = OK;

END:
HideWindow(Window[wInfo]);
MIDIinputFilter = oldfilter;
StopWait();
return(r);
#endif
}

#endif /* BP_CARBON_GUI */

#if 0	/* this is never called */
TranslateMIDIdata(int w,long im)
{
int b,br,channel,key,vel;
long i;

i = br = -1;

NEXTBYTE:
i++; if(i >= im) goto QUIT;
b = ByteToInt((*p_Code)[i].byte);
if(b < 128) {
	if(br == 0) goto NEXTBYTE;	/* happens in beginning */
	if(br == NoteOn || br == NoteOff) {
		key = b;
		i++; if(i >= im) goto QUIT;
		vel = ByteToInt((*p_Code)[i].byte) % 256;
		if(br == NoteOn) {
			if(vel > 0)
				sprintf(Message,"NoteOn channel %ld %ld %ld ",
					(long)channel,(long)key,(long)vel);
			else
				sprintf(Message,"NoteOff channel %ld %ld 127 ",
					(long)channel,(long)key);
			}
		else sprintf(Message,"NoteOff channel %ld %ld %ld ",
			(long)channel,(long)key,(long)vel);
		Print(w,Message);
		goto NEXTBYTE;
		}
	/* $$$ if br == ... */
	}
if(b == SystemExclusive) {
	br = 0;
	while(((b = ByteToInt((*p_Code)[i].byte)) != EndSysEx) && i < im) i++;
	if(i == im) goto QUIT;
	}
if(b >= SystemExclusive) {
	switch(b) {
		case Start:
			Print(w,"MIDI_Start "); break;
		case Continue:
			Print(w,"MIDI_Continue "); break;
		case Stop:
			Print(w,"MIDI_Stop "); break;
		}
	goto NEXTBYTE;
	}
channel = b % 16;
b -= channel; br = b; channel++;
if(b == NoteOn) {
	i++; if(i >= im) goto QUIT;
	key = ByteToInt((*p_Code)[i].byte);
	i++; if(i >= im) goto QUIT;
	vel = ByteToInt((*p_Code)[i].byte);
	if(vel > 0)
		sprintf(Message,"NoteOn channel %ld %ld %ld ",
			(long)channel,(long)key,(long)vel);
	else
		sprintf(Message,"NoteOff channel %ld %ld 127 ",
			(long)channel,(long)key);
	Print(w,Message);
	goto NEXTBYTE;
	}
if(b == NoteOff) {
	i++; if(i >= im) goto QUIT;
	key = ByteToInt((*p_Code)[i].byte);
	i++; if(i >= im) goto QUIT;
	vel = ByteToInt((*p_Code)[i].byte);
	sprintf(Message,"NoteOff channel %ld %ld %ld ",
		(long)channel,(long)key,(long)vel);
	Print(w,Message);
	goto NEXTBYTE;
	}
goto NEXTBYTE;
	
QUIT:
return(OK);
}
#endif

#if BP_CARBON_GUI
// I'm not sure whether we need LoadMIDIprototype()
// in ANSI console build, but seems unlikely.  - akozar

LoadMIDIprototype(int j,long isize) 
{
long i,imax;
MIDI_Event e;
/* long count = 12L,i,imax;
MIDI_Parameters parms; */
int key,r,dur,channel,velocity,error,rs;
long p,q;
char line[MAXFIELDCONTENT];
unsigned long drivertime;

if(SoundOn || CheckEmergency() != OK) return(FAILED);
if(!IsMidiDriverOn()) { // added this check - 012307 akozar
	if(Beta) Alert1("Err. LoadMIDIprototype(). Driver is OFF");
	return(FAILED);
	}

#if !WITH_REAL_TIME_MIDI
  return(FAILED);
#else
if(!OutMIDI) {
	OutMIDI = TRUE;
	SetButtons(TRUE);
	}
i = ZERO;
imax = isize;
if(!GetCtrlValue(wPrototype5,bPlayPrototypeTicks)) {
	/* Don't play ticks */
	dur = 1000; key = -1;
	goto DOIT;
	}
key = PrototypeTickKey;
channel = PrototypeTickChannel - 1;
velocity = PrototypeTickVelocity;
if(GetField(NULL,TRUE,wPrototype5,fTref,line,&p,&q) != OK) return(FAILED);
dur = p/q;
if(dur < EPSILON) {
	sprintf(Message,"You can't play ticks because Tref is only %ldms",(long)dur);
	Alert1(Message);
	key = -1; dur = 1000;
	goto DOIT;
	}
if(j > 2 && j < Jbol && (*p_Tref)[j] != dur) {
	if(Answer(
		"You want to play ticks although this object is not striated. Proceed anyway?",
		'Y') != OK) return(FAILED);
	}
dur = dur / Time_res / 2;

DOIT:
SetCursor(&WatchCursor);
FlashInfo("Receiving MIDI data. Click mouse when completed.");
ResetMIDI(FALSE);

FlushEvents(everyEvent,0);

error = NO;
rs = 0; /* Running status */
while(TRUE) {
	if(key >= 0) {
		e.type = NORMAL_EVENT;
		e.time = Tcurr;
		e.status = NoteOn + channel;
		e.data1 = key;
		e.data2 = velocity;
		SendToDriver(Tcurr * Time_res,0,&rs,&e);
		Tcurr += dur;
		e.type = NORMAL_EVENT;
		e.time = Tcurr;
		e.status = NoteOn + channel;
		e.data1 = key;
		e.data2 = 0;
		SendToDriver(Tcurr * Time_res,0,&rs,&e);
		}
	drivertime = GetDriverTime();
	while((Tcurr > drivertime) && !Button()) {
		drivertime = GetDriverTime();
		if((r=GetNextMIDIevent(&e,FALSE,FALSE)) == FALSE) continue;
		if(r == ABORT) break;
		(*p_Code)[i].time = drivertime * Time_res;	/* milliseconds */
		(*p_Code)[i].byte = ByteToInt(e.data2);
		(*p_Code)[i].sequence = 0;
		if(++i >= imax) {
			if(ThreeOverTwo(&imax) != OK) return(FAILED);
			if((p_Code = (MIDIcode**) IncreaseSpace((Handle)p_Code)) == NULL) return(ABORT);
			}
		}
	Tcurr += dur;
	if(error || Button()) break;
	}
if(error) imax = -1L;
else {
	imax = i;
	if(MySetHandleSize((Handle*)&p_Code,(Size)(i+1) * sizeof(MIDIcode)) != OK) return(ABORT);
	}

while(Button());

FlushEvents(mDownMask+mUpMask,0);

ResetMIDI(TRUE);
HideWindow(Window[wInfo]);
StopWait();
if(imax < 2L) {
	Alert1("No data received...");
	return(FAILED);
	}
return(MIDItoPrototype(TRUE,TRUE,j,p_Code,imax));
#endif
}

#endif /* BP_CARBON_GUI */
