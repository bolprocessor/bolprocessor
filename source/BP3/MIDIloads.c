/* MIDIloads.c (BP3) */ 

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
		my_sprintf(Message,"Attempting to read item #%ld",(long)nitem);
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
					ShowMessage(TRUE,wMessage,"=> Incorrect status byte");
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

#if BP_CARBON_GUI_FORGET_THIS

ReadNoteOn(int c0, int c1, int c2, int wind)
{

if(wind < 0 || wind >= WMAX || !Editable[wind]) return(MISSED);
if(!IsMidiDriverOn()) {
	if(Beta) Alert1("=> Err. ReadNoteOn(). Driver is OFF");
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
	PrintNote(-1,c1,-1,wind,Message);
	FoundNote = TRUE;
	TickDone = EmptyBeat = FALSE;
	return(OK);
	}
else return(MISSED);
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
	if(Beta) Alert1("=> Err. LoadTimePattern(). Driver is OFF");
	return(ABORT);
	}

#if !WITH_REAL_TIME_MIDI_FORGET_THIS
  return(ABORT);
#else
if(!rtMIDI) {
	rtMIDI = TRUE;
	
	}
if(ComputeOn || PolyOn || CompileOn || SoundOn || SelectOn ||
	SetTimeOn || GraphicOn || PrintOn || ReadKeyBoardOn || HangOn || ScriptExecOn)
	return(MISSED);
if(wind < 0 || wind >= WMAX || !Editable[wind]) return(MISSED);
EndWriteScript();
BPActivateWindow(SLOW,wind);

if((rep=CompilePatterns()) != OK) return(rep);
ipatt = Jpatt;
UpdateThisWindow(FALSE,Window[wind]); /* Update text length */
ShowSelect(CENTRE,wind);
pos = pos1 = ZERO; isthere = FALSE;
posmax = GetTextLength(wind);
while(posmax > 0 && GetTextChar(wind,posmax-1) == '\r') posmax--;
p_line = NULL;
while(ReadLine(YES,wind,&pos,posmax,&p_line,&gap) == OK) {
	if((*p_line)[0] == '\0') {
		pos1 = pos; continue;
		}
	if(Mystrcmp(p_line,"DATA:") == 0) goto SORTIR;
	if(Mystrcmp(p_line,"COMMENT:") == 0) goto SORTIR;
	if(Mystrcmp(p_line,"TIMEPATTERNS:") == 0) {
		isthere = TRUE; goto SORTIR;
		}
	pos1 = pos;
	}
pos1 = posmax + 1L;

SORTIR:
pos1 -= 1L;
SetSelect(pos1,pos1,TEH[wind]);
if(!isthere) Print(wind,"\nTIMEPATTERNS:\n------------");
pos2 = pos1 + 15L;
// HideWindow(Window[wMessage]);
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
	if((result=GetNextMIDIevent(&e,TRUE,FALSE)) == MISSED) continue;
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

if(FormatMIDIstream(p_b,imax,p_c,TRUE,im2,&nbytes,TRUE) != OK) return(MISSED);	
	
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
				// HideWindow(Window[wInfo]);
				ReadKeyBoardOn = FALSE;
				return(ABORT);
				}
			imax = (imax * 3L) / 2L;
			}
		}
	}

im = i;
// HideWindow(Window[wInfo]);
/* if(im < 2) {
	Alert1("No data received...");
	goto END;
	} */
for(i=0; i < 4; i++) MainEvent();
StopWait();
d = im - 1;
if (d < 1)  d = 1;
my_sprintf(Message, "%d", d);
SetField(PatternPtr, wUnknown, fPatternDuration, Message);
SelectField(PatternPtr, wUnknown, fPatternName, TRUE);

TRY:
ShowWindow(GetDialogWindow(PatternPtr)); 
SelectWindow(GetDialogWindow(PatternPtr)); // was BringToFront() - akozar 052107
GetDialogItem(PatternPtr,fPatternName,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXNAME,t,name);
rep = MISSED;
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
// HideWindow(GetDialogWindow(PatternPtr));
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
		Alert1("=> Pattern name is a variable and should start with uppercase character");
		goto TRY;
		}
	for(i=0; i < strlen(name); i++) {
		c = name[i];
		if(!isalnum(c)) {
			my_sprintf(Message,
		"=> Space or incorrect character in name.  Can't accept '%c'",c);
			Alert1(Message);
			goto TRY;
			}
		}
	}
for(i=0; i < strlen(LineBuff); i++) { // FIXME: This check seems unnecessary - akozar
	c = LineBuff[i];
	if(!isdigit(c)) {
		my_sprintf(Message,
		 "=> Unexpected character '%c'.\nThe symbolic duration must be a positive integer.",c);
		Alert1(Message);
		goto TRY;
		}
	}
if(d <= 0) {
	Alert1("=> The symbolic duration must be a positive integer.");
	goto TRY;
	}
	
period = ((*p_t)[im-1] - (*p_t)[0]) / d;
SetSelect(pos2,pos2,TEH[wind]);
for(i=1; i < im; i++) {
	if(Simplify((double)INT_MAX,(double)((*p_t)[i] - (*p_t)[i-1]),(double)period,&p,&q) != OK)
		goto END;
	ipatt++;
	my_sprintf(Message,"t%ld = %u/%u  ",(long)ipatt,(unsigned long)p,
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
	my_sprintf(Message,"\n%s -->",name);
	Print(wind,Message);
	ipatt = Jpatt;
	for(i=1; i < im; i++) {
		ipatt++;
		my_sprintf(Message," t%ld",(long)ipatt);
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
#if WITH_REAL_TIME_MIDI_FORGET_THIS
  ResetMIDI(FALSE);
#endif

FlushEvents(everyEvent,0);

FlashInfo("Waiting for keystroke or MIDI code. (Click to exit)");
i = 0;
ReadKeyBoardOn = TRUE;
while(!Button()) {
	MaintainCursor();
#if WITH_REAL_TIME_MIDI_FORGET_THIS
	if((r=GetNextMIDIevent(&e,FALSE,FALSE)) == MISSED) goto EVENT;
	if(r == ABORT) break;
	c0 = e.data2;
	if(c0 < NoteOn || c0 >= (NoteOn+16)) goto EVENT;
	if((r=GetNextMIDIevent(&e,FALSE,FALSE)) == MISSED) goto EVENT;
	if(r == ABORT) break;
	c1 = e.data2;
	if((r=GetNextMIDIevent(&e,FALSE,FALSE)) == MISSED) goto EVENT;
	if(r == ABORT) break;
	c2 = e.data2;
	if(c2 > 0) { 	/* NoteOn */
		switch(NoteConvention) {
			case ENGLISH: j = 2; break;
			case FRENCH: j = 1; break;
			case INDIAN: j = 88; break;
			default: j = 3; break;
			}
		PrintBehindln(wScript,*(p_ScriptLabelPart(j,0)));
		channel = c0 - NoteOn + 1;
		PrintNote(-1,c1,channel,-1,Message);
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
// HideWindow(Window[wInfo]);
ReadKeyBoardOn = FALSE;

FlushEvents(mDownMask+mUpMask,0);

return(OK);
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

int PrintNote(int i_scale,int key,int channel,int wind,char* line)
{
int pitchclass, octave;
char channelstring[11], jscale;

if(key < 0) {
	strcpy(line,"<void>");
	return(OK);
	}
key -= (C4key - 60);
pitchclass = modulo(key,12);
octave = (key - pitchclass) / 12;
channelstring[0] = '\0';
if(channel > 0) my_sprintf(channelstring," channel %ld",(long)channel);
if(NameChoice[pitchclass] == 1 && pitchclass == 0) octave--;
if(NameChoice[pitchclass] == 1 && pitchclass == 11) octave++;
// BPPrintMessage(odInfo,"i_scale = %d key =  %d NumberScales = %d NoteConvention = %d\n",i_scale,key,NumberScales,NoteConvention);

if(i_scale > NumberScales) {
	BPPrintMessage(odError,"=> Error: i_scale (%ld) > NumberScales (%d)\n",(long)i_scale,NumberScales);
	return(OK);
	}

jscale = i_scale + 3;
if(jscale > 3) {
//	jscale = i_scale + 3;
	my_sprintf(line,"%s%s",*((*(p_NoteName[jscale]))[key]),channelstring);
	}	
else {	
	switch(NoteConvention) {
		case FRENCH:
			octave -= 2;
			switch(octave) {
				case -2:
					if(NameChoice[pitchclass] == 0)
						my_sprintf(line,"%s000%s",Frenchnote[pitchclass],channelstring);
					else
						my_sprintf(line,"%s000%s",AltFrenchnote[pitchclass],channelstring);
					break;
				case -1:
					if(NameChoice[pitchclass] == 0)
						my_sprintf(line,"%s00%s",Frenchnote[pitchclass],channelstring);
					else
						my_sprintf(line,"%s00%s",AltFrenchnote[pitchclass],channelstring);
					break;
				default:
					if(NameChoice[pitchclass] == 0)
						my_sprintf(line,"%s%ld%s",Frenchnote[pitchclass],(long)octave,channelstring);
					else
						my_sprintf(line,"%s%ld%s",AltFrenchnote[pitchclass],(long)octave,channelstring);
					break;
				}
			break;
		case ENGLISH:
			octave--;
			switch(octave) {
				case -1:
					if(NameChoice[pitchclass] == 0)
						my_sprintf(line,"%s00%s",Englishnote[pitchclass],channelstring);
					else
						my_sprintf(line,"%s00%s",AltEnglishnote[pitchclass],channelstring);
					break;
				default:
					if(NameChoice[pitchclass] == 0)
						my_sprintf(line,"%s%ld%s",Englishnote[pitchclass],(long)octave,channelstring);
					else
						my_sprintf(line,"%s%ld%s",AltEnglishnote[pitchclass],(long)octave,channelstring);
					break;
				}
			break;
		case INDIAN:
			octave--;
			switch(octave) {
				case -1:
					if(NameChoice[pitchclass] == 0)
						my_sprintf(line,"%s00%s",Indiannote[pitchclass],channelstring);
					else
						my_sprintf(line,"%s00%s",AltIndiannote[pitchclass],channelstring);
					break;
				default:
					if(NameChoice[pitchclass] == 0)
						my_sprintf(line,"%s%ld%s",Indiannote[pitchclass],(long)octave,channelstring);
					else
						my_sprintf(line,"%s%ld%s",AltIndiannote[pitchclass],(long)octave,channelstring);
					break;
				}
			break;
		default: // This includes 'KEYS'
			my_sprintf(line,"%s%ld%s",KeyString,(long)key,channelstring);
			break;
		}
	}
#if BP_CARBON_GUI_FORGET_THIS
if(wind >= 0) {
	PrintBehind(wind,line);
	UpdateDirty(TRUE,wind);
	}
#endif /* BP_CARBON_GUI_FORGET_THIS */
return(OK);
}


int GetNote(char* line,int* p_thekey,int* p_channel,int ignorechannel)
{
char *p,*q,line2[MAXLIN];
int i,j,pitchclass,octave,l;

i = j = 0; while(MySpace(line[i])) i++;
strcpy(line2,line);
if(NoteConvention == KEYS) {
	while(line[i] != '\0' && line[i] == KeyString[j]) {
		i++; j++;
		}
	if(KeyString[j] != '\0') return(MISSED);
	*p_thekey = GetInteger(YES,line2,&i);
	if(*p_thekey == INT_MAX) return(MISSED);
	}
else {
	for(pitchclass=0; pitchclass < 12; pitchclass++) {
		p = &line2[i];
		switch(NoteConvention) {
			case FRENCH: {
				q = &Frenchnote[pitchclass][0]; l = strlen(Frenchnote[pitchclass]);
				if(l > 0 && Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				p = &line2[i];
				q = &AltFrenchnote[pitchclass][0]; l = strlen(AltFrenchnote[pitchclass]);
				if(l > 0 && Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				break;
				}
			case ENGLISH: {
				q = &Englishnote[pitchclass][0]; l = strlen(Englishnote[pitchclass]);
				if(l > 0 && Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				p = &line2[i];
				q = &AltEnglishnote[pitchclass][0]; l = strlen(AltEnglishnote[pitchclass]);
				if(l > 0 && Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				break;
				}
			case INDIAN:
				q = &Indiannote[pitchclass][0]; l = strlen(Indiannote[pitchclass]);
				if(l > 0 && Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				p = &line2[i];
				q = &AltIndiannote[pitchclass][0]; l = strlen(AltIndiannote[pitchclass]);
				if(l > 0 && Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				break;
			}
		}
	return(MISSED);
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
	if((octave=GetInteger(YES,line2,&i)) == INT_MAX) return(MISSED);
	if(NoteConvention == FRENCH) octave += 2;
	if(NoteConvention == ENGLISH || NoteConvention == INDIAN) octave++;
	
CONT2:
	*p_thekey = 12 * octave + pitchclass;
	*p_thekey += (C4key - 60);
//	BPPrintMessage(odInfo,"\nkey = %d Englishnote[pitchclass] = %s AltEnglishnote[pitchclass] = %s pitchclass = %d octave = %d NameChoice[pitchclass] = %d\n",*p_thekey,Englishnote[pitchclass],pitchclass,octave,NameChoice[pitchclass]);
	}
if(ignorechannel) return(OK);
while(MySpace(line[i])) i++;
strcpy(Message,"channel");
p = &line2[i]; q = &(Message[0]);
if(!Match(FALSE,&p,&q,strlen(Message))) return(MISSED);
while(!isdigit(line[i]) && line[i] != '\0') i++;
if((*p_channel=GetInteger(YES,line2,&i)) == INT_MAX) return(MISSED);
return(OK);
}
