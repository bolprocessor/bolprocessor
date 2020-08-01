/* Misc.c (BP2 version CVS) */

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


AppendStringList(char* line)
{
char** ptr;
int i;

if(line[0] == '\0') return(OK);
NrStrings++;
if(NrStrings >= SizeStringList) {
	p_StringList = (char****) IncreaseSpace((Handle) p_StringList);
	SizeStringList = (MyGetHandleSize((Handle)p_StringList) / sizeof(char**));
	for(i=NrStrings; i < SizeStringList; i++) (*p_StringList)[i] = NULL;
	}
if((ptr=(char**)GiveSpace((Size) (strlen(line) + 1) * sizeof(char))) == NULL)
	return(ABORT);
(*p_StringList)[NrStrings-1] = ptr;
MystrcpyStringToTable(p_StringList,NrStrings-1,line);
return(OK);
}


MemberStringList(char* line)
{
int i,r;

r = NO;
for(i=0; i < NrStrings; i++) {
	if(strcmp(line,StringList(i)) == 0) {
		r = YES; break;
		}
	}
return(r);
}


Expect(char c,char* string,char d)
{
char line[MAXLIN];

if(isspace(d))
	sprintf(line,"Expecting '%c' after '%s'. Found a space instead.\n",
		c,string);
else
	if(isgraph(d))
		sprintf(line,"Expecting '%c' after '%s'. Found '%c' instead.\n",
			c,string,d);
	else
		sprintf(line,"Expecting '%c' after '%s'. Found '%c' (ASCII %ld) instead.\n",
			c,string,d,(long)((256L + d) % 256L));
Print(wTrace,line);
return(OK);
}

#if BP_CARBON_GUI

void FilterHelpSelector(char* command)
/* The parameter to DisplayHelp() needs to be const, so
   this function is provided for making sure that a non-
   const string created at runtime and passed to 
   DisplayHelp() does not contain bad characters */
{
int i;
for(i=0; i < strlen(command); i++) {
	if(isspace(command[i])) command[i] = ' ';
	if(command[i] == '\21') command[i] = '\0'; /* Eliminate <cmd> */
	}
Strip(command);
return;
}

DisplayHelp(const char* command)
/* Display section of "BP2 help" relative to 'command' */
{
long pos,posline,posmax,pos1,pos2;
int i,length,rr1,rr2;
char **p_completeline,target[260],*q,**p_line;
GrafPtr saveport;

GetPort(&saveport);

PleaseWait();
Help = FALSE;
if(command[0] == '\0') return(FAILED);
pos = ZERO;
SetFPos(HelpRefnum,fsFromStart,pos);
sprintf(target,"### %s",command);
length = strlen(target);

/* First try to find target in the Help window... */

posline = ZERO;
posmax = GetTextLength(wHelp);
p_line = NULL;
ShowWindow(Window[wHelp]);
do {
	if(ReadLine(YES,wHelp,&pos,posmax,&p_line,&i) != OK) goto LOAD;
	if((*p_line)[0] == '\0') continue;
	q = &(target[0]);
	if(Match(FALSE,p_line,&q,length)) {
		MyDisposeHandle((Handle*)&p_line);
		SetSelect(posline,pos,TEH[wHelp]);
		goto OUT;
		}
	posline = pos;
	}
while(TRUE);

LOAD:
MyDisposeHandle((Handle*)&p_line);

/* Load target from help file */

ShowMessage(TRUE,wMessage,"Searching the 'BP2 help' data-base...");
FlashInfo(target);
pos1 = GetTextLength(wHelp);
SetSelect(pos1,pos1,TEH[wHelp]);
pos = pos2 = ZERO;
p_line = p_completeline = NULL;
LoadOn++;
do {
	if((rr1=ReadOne(FALSE,FALSE,TRUE,HelpRefnum,FALSE,&p_line,&p_completeline,&pos)) == FAILED) {
		goto BADLUCK;
		}
	if((*p_completeline)[0] != '#') goto GOAHEAD;
	PleaseWait();
	q = target;
	if(Match(FALSE,p_completeline,&q,length)) {
		do {
			MyLock(FALSE,(Handle)p_completeline);
			PrintBehindln(wHelp,(*p_completeline));
			MyUnlock((Handle)p_completeline);
			if(pos2 == ZERO) pos2 = GetTextLength(wHelp);
			if((rr2=ReadOne(FALSE,FALSE,TRUE,HelpRefnum,FALSE,&p_line,&p_completeline,&pos))
				== FAILED) break;
			if((*p_completeline)[0] == '#' && (*p_completeline)[1] == '#'
				&& (*p_completeline)[2] == '#') break;
			if(rr2 == STOP) break;
			}
		while(TRUE);
		break;
		}
	
GOAHEAD:
	if(rr1 == STOP) {
BADLUCK:
		PrintBehindln(wHelp,target);
		PrintBehindln(wHelp,"This item is not documented.");
		ShowSelect(CENTRE,wHelp);
		BPActivateWindow(QUICK,wHelp);
		pos2 = GetTextLength(wHelp);
		break;
		}
	}
while(TRUE);
MyDisposeHandle((Handle*)&p_line);
MyDisposeHandle((Handle*)&p_completeline);

LoadOn--;
HideWindow(Window[wMessage]);
HideWindow(Window[wInfo]);
SetSelect(pos1,pos2,TEH[wHelp]);

OUT:
ShowSelect(CENTRE,wHelp);
BPActivateWindow(QUICK,wHelp);
BringToFront(Window[wHelp]);
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err DisplayHelp(). saveport == NULL");
return(OK);
}


DisplayFile(int w,char* name)
/* Display a file contained in the same folder as BP2 */
{
int io;
FSSpec spec;
short refnum;

c2pstrcpy(spec.name, name);
spec.vRefNum = RefNumbp2;
spec.parID = ParIDbp2;
if((io=MyOpen(&spec,fsRdPerm,&refnum)) == noErr) {
	LoadOn++;
	ClearWindow(NO,w);
	ReadFile(w,refnum);
	FSClose(refnum);
	ShowWindow(Window[w]);
	SetSelect(ZERO,ZERO,TEH[w]);
	ShowSelect(UP,w);
	BPActivateWindow(QUICK,w);
	// BringToFront(Window[w]); // BPAct.Win.() makes it the front window - akozar
	LoadOn--;
	return(OK);
	}
else return(FAILED);
}

#endif /* BP_CARBON_GUI */

// FIXME: GetInteger() likely originally assumed that ints were 2 bytes.
// Change to int16_t Get2ByteInteger() ?? (needs careful checking!)
GetInteger(int test, const char* line, int* p_i)
{
long n;
int done,sign;
char c;

n = 0; done = FALSE; sign = 1;
if(test && *p_i >= strlen(line)) return(INT_MAX);
/* Sometimes reading special arrays that start with '\0' */
do {
	c = line[*p_i];
	if(!done && c == '-' && sign == 1) {
		sign = -1; (*p_i)++;
		continue;
		}
	if(c == '\0' || !isdigit(c)) break;
	n = 10L * n + (long) (c - '0'); done = TRUE;
	if(n > INT_MAX) {	// FIXME: can never be true when sizeof(long) == sizeof(int)
		sprintf(Message,"\nMaximum integer value: %ld.\n",(long)INT_MAX);
		Print(wTrace,Message);
		return(INT_MAX);
		}
	(*p_i)++;
	}
while(c != '\0');
if(!done) {
	if(!InitOn && !test) Print(wTrace,"\nNumber missing.\n");
	return(INT_MAX);
	}
return((int) n * sign);
}


// FIXME: GetHexa() likely originally assumed that ints were 2 bytes ?
int GetHexa(const char* line, int* p_i)
{
long n;
int done,j;
char c;

n = 0; done = FALSE;
if(*p_i >= strlen(line)) return(INT_MAX);
do {
	c = line[*p_i];
	if(!done && c == '-') {
		Print(wTrace,"\nHexadecimal number should not be negative");
		return(INT_MAX);
		}
	if(c == '\0') break;
	c = UpperCase(c);
	switch(c) {
		case 'A': j = 10; break;
		case 'B': j = 11; break;
		case 'C': j = 12; break;
		case 'D': j = 13; break;
		case 'E': j = 14; break;
		case 'F': j = 15; break;
		default:
			if(!isdigit(c)) goto END;
			j = c - '0';
			break;
		}
	n = 16L * n + (long) j; done = TRUE;
	if(n > INT_MAX) {	// FIXME: can never be true when sizeof(long) == sizeof(int)
		sprintf(Message,"\nMaximum integer value: %ld.\n",(long)INT_MAX);
		Print(wTrace,Message);
		return(INT_MAX);
		}
	(*p_i)++;
	}
while(c != '\0');
END:
if(!done) {
	Print(wTrace,"\nNumber missing.\n");
	return(INT_MAX);
	}
return((int) n);
}


// FIXME: GetLong() likely originally assumed that longs are 4 bytes.
// Change to int32_t Get4ByteInteger() ?? (needs careful checking!)
long GetLong(const char* line, int* p_i)
{
long n;
int done,sign;
char c;

n = 0; done = FALSE; sign = 1;
if(*p_i >= strlen(line)) return(Infpos);
do {
	c = line[*p_i];
	if(!done && c == '-' && sign == 1) {
		sign = -1; (*p_i)++;
		continue;
		}
	c = line[*p_i];
	if(c == '\0' || !isdigit(c)) break;
	n = 10L * n + (long) (c - '0'); done = TRUE;
	if(n >= Infpos) {	// FIXME: can never be true (Infpos == LONG_MAX)
		sprintf(Message,"\nMaximum value: %ld.\n",(long)Infpos-1);
		Print(wTrace,Message);
		return(Infpos);
		}
	(*p_i)++;
	}
while(c != '\0');
if(!done) {
	Print(wTrace,"\nNumber missing.\n");
	return(Infpos);
	}
return(n * sign);
}


// FIXME: GetUnsigned() likely originally assumed that unsigned ints were 2 bytes.
// Change to uint16_t Get2ByteUnsigned() ?? (needs careful checking!)
unsigned GetUnsigned(const char* line, int* p_i)
{
long n;
int done;
char c;

n = 0; done = FALSE;
if(*p_i >= strlen(line)) return(INT_MAX * 2L);	// FIXME: integer overflow (*2UL or UINT16_MAX ?)
do {
	c = line[*p_i];
	if(!done && c == '-') {
		Print(wTrace,"\nUnsigned integer should not be negative");
		return(INT_MAX * 2L);
		}
	c = line[*p_i];
	if(c == '\0' || !isdigit(c)) break;
	n = 10L * n + (long) (c - '0'); done = TRUE;
	if(n >= INT_MAX * 2L) {	// FIXME: use wider integers and proper max value
		sprintf(Message,"\nMaximum unsigned value: %ld.\n",
			(long) INT_MAX * 2L - 1L);	// FIXME (UINT16_MAX = INT16_MAX*2 + 1)
		Print(wTrace,Message);
		return(INT_MAX * 2L);
		}
	(*p_i)++;
	}
while(c != '\0');
if(!done) {
	Print(wTrace,"\nNumber missing.\n");
	return(INT_MAX * 2L);
	}
return((unsigned) n);
}


// FIXME ? Use strtod() instead ?
double GetDouble(const char* line, int* p_i)
{
double n,sign;
int done,signfound;
long dec;
char c;

n = 0.; done = signfound = FALSE; sign = 1.;
dec = ZERO;
if(*p_i >= strlen(line)) return(-1.);
do {
	c = line[*p_i];
	if(c == '+') {
		if(!done && sign == 1 && !signfound) {
			(*p_i)++; signfound = TRUE; continue;
			}
		else {
			Print(wTrace,"\nIncorrect sign in float number.\n");
			return(-1.);
			}
		}
	if(c == '-') {
		if(!done && sign == 1 && !signfound) {
			(*p_i)++; signfound = TRUE; sign = -1.; continue;
			}
		else {
			Print(wTrace,"\nIncorrect sign in float number.\n");
			return(-1.);
			}
		}
	if(c == '.') {
		if(dec == ZERO) {
			dec = 10L; (*p_i)++;
			continue;
			}
		else {
			Print(wTrace,"\nSeveral decimal points in float number.\n");
			return(-1.);
			}
		}
	if(c == '\0' || !isdigit(c)) break;
	done = TRUE;
	if(dec == ZERO) n = 10.0 * n + (double) (c - '0');
	else  {
		n = n + (((double) (c - '0')) / dec);
		dec = 10 * dec;
		}
	(*p_i)++;
	}
while(c != '\0');
if(!done) {
	Print(wTrace,"\nFloat number incorrect or missing.\n");
	return(-1.);
	}
return(n * sign);
}

#if BP_CARBON_GUI

ShowMIDIkeyboard(void)
{
short i,itemtype;
Rect r;
ControlHandle itemhandle;
Str255 textStr;
GrafPtr saveport, dport;

for(i=1; i <= 24; i++) {
	GetDialogItem(MIDIkeyboardPtr,i,&itemtype,(Handle*) &itemhandle,&r);
	switch(i) {
		case 15:
		case 20:
		case 22:
		HideControl(itemhandle);
		continue;
		}
	switch(NoteConvention) {
		case FRENCH:
			if(i < 13)
				strcpy(Message,Frenchnote[i-1]);
			else
				strcpy(Message,AltFrenchnote[i-13]);
			break;
		case ENGLISH:
			if(i < 13)
				strcpy(Message,Englishnote[i-1]);
			else
				strcpy(Message,AltEnglishnote[i-13]);
			break;
		case INDIAN:
			if(i < 13)
				strcpy(Message,Indiannote[i-1]);
			else
				strcpy(Message,AltIndiannote[i-13]);
			break;
		
		}
	c2pstrcpy(textStr, Message);
	SetControlTitle(itemhandle,textStr);
	}
SetNameChoice();
ShowWindow(GetDialogWindow(MIDIkeyboardPtr));
SelectWindow(GetDialogWindow(MIDIkeyboardPtr));
BPUpdateDialog(MIDIkeyboardPtr); /* Needed to show static text! */
return(DoSystem());
}


SetNameChoice(void)
{
short i,itemtype;
Rect r;
ControlHandle itemhandle;

for(i=1; i <= 12; i++) {
	if(NameChoice[i-1] == 0) {
		GetDialogItem(MIDIkeyboardPtr,i,&itemtype,(Handle*) &itemhandle,&r);
		SetControlValue(itemhandle,1);
		GetDialogItem(MIDIkeyboardPtr,i+12,&itemtype,(Handle*) &itemhandle,&r);
		SetControlValue(itemhandle,0);
		}
	else {
		GetDialogItem(MIDIkeyboardPtr,i,&itemtype,(Handle*) &itemhandle,&r);
		SetControlValue(itemhandle,0);
		GetDialogItem(MIDIkeyboardPtr,i+12,&itemtype,(Handle*) &itemhandle,&r);
		SetControlValue(itemhandle,1);
		}
	}
return(DoSystem());
}

#endif /* BP_CARBON_GUI */

MySpace(char c)
{
if(c == '\r' || c == '\n' || c == '\0') return(NO);
if(isspace(c)) return(YES);
// -54 = 0xCA (202) which is a non-breaking space (in Mac Roman)
// '\20' = 0x10 (16) which is control-P and ???
if(c == (char) -54 || c == '\20') return(YES);	/* option space, ctrl-p */
return(NO);
}




ByteToInt(char x)
{
int i;

i = x;
if(i < 0) i += 256;
return(i);
}


MoveDown(tokenbyte ***pp_buff, long *p_i, long *p_k, long *p_imax)
{
long j;
j = *p_i;
while((*p_k) < (*p_imax)+2) (**pp_buff)[j++] = (**pp_buff)[(*p_k)++];
*p_imax = j - 2L;
return(OK);
}

#if BP_CARBON_GUI

GetValues(int force)
{
int oldoutmidi,oldwritemidifile,oldoutcsound,oms;

oldoutmidi = OutMIDI;
if(force || Nw == wSettingsTop || Nw == wSettingsBottom) {
	OutMIDI = GetControlValue(Hbutt[bMIDI]);
	Improvize = GetControlValue(Hbutt[bImprovize]);
	StepProduce = GetControlValue(Hbutt[bStepProduce]);
	StepGrammars = GetControlValue(Hbutt[bStepGrammars]);
	PlanProduce = GetControlValue(Hbutt[bPlanProduce]);
	DisplayProduce = GetControlValue(Hbutt[bDisplayProduce]);
	UseEachSub = GetControlValue(Hbutt[bUseEachSub]);
	AllItems = GetControlValue(Hbutt[bAllItems]);
	TraceProduce = GetControlValue(Hbutt[bTraceProduce]);
	DisplayTimeSet = GetControlValue(Hbutt[bDisplayTimeSet]);
	StepTimeSet = GetControlValue(Hbutt[bStepTimeSet]);
	TraceTimeSet = GetControlValue(Hbutt[bTraceTimeSet]);
	AllowRandomize = GetControlValue(Hbutt[bAllowRandomize]);
	CyclicPlay = GetControlValue(Hbutt[bCyclicPlay]);
	SynchronizeStart = GetControlValue(Hbutt[bSynchronizeStart]);
	DisplayItems = GetControlValue(Hbutt[bDisplayItems]);
	ResetWeights = GetControlValue(Hbutt[bResetWeights]);
	if(ResetWeights) NeverResetWeights = FALSE;
	ResetFlags = GetControlValue(Hbutt[bResetFlags]);
	ResetControllers = GetControlValue(Hbutt[bResetControllers]);
	NoConstraint = GetControlValue(Hbutt[bNoConstraint]);
	ShowGraphic = GetControlValue(Hbutt[bShowGraphic]);
	
	oldwritemidifile = WriteMIDIfile;
	WriteMIDIfile = GetControlValue(Hbutt[bWriteMIDIfile]);
	if(oldwritemidifile && !WriteMIDIfile) CloseMIDIFile();
	
	ComputeWhilePlay = GetControlValue(Hbutt[bComputeWhilePlay]);
	Interactive = GetControlValue(Hbutt[bInteractive]);
	ShowMessages = GetControlValue(Hbutt[bShowMessages]);
	
	oldoutcsound = OutCsound;
	OutCsound = GetControlValue(Hbutt[bCsound]);
	if(oldoutcsound && !OutCsound) CloseCsScore();
	
	CsoundTrace = GetControlValue(Hbutt[bCsoundTrace]);
	oms = GetControlValue(Hbutt[bOMS]);
	if(oms != Oms) mOMS(0);
	
	SetButtons(force);
	}
return(OK);
}

#endif /* BP_CARBON_GUI */

SetButtons(int force)
{
  /* Might prefer #if BP_CARBON_GUI here b/c changing settings in non-GUI build 
     might be undesirable (better to just report incompatible options and exit?)
	 -- akozar 20130813
   */
if(AllItems || Improvize) CyclicPlay = FALSE;
if(AllItems) Improvize = FALSE;
if(CyclicPlay || AllItems || OutCsound || WriteMIDIfile) ComputeWhilePlay = FALSE;
if(!InitOn && !IsMidiDriverOn()) OutMIDI = FALSE; // added 012307 - akozar
if((AllItems || Improvize) && !OutMIDI && !OutCsound && !WriteMIDIfile) DisplayItems = TRUE;
if(StepTimeSet) TraceTimeSet = TRUE;
if(TraceTimeSet) DisplayTimeSet = TRUE;
if(PlanProduce) TraceProduce = StepProduce = TRUE;
if(TraceProduce || StepProduce || StepGrammars) DisplayProduce = TRUE;
#if BP_CARBON_GUI
if(force || (Nw == wSettingsTop) || (Nw == wSettingsBottom)) {
	ChangeControlValue(force,Hbutt[bMIDI],OutMIDI);
	ChangeControlValue(force,Hbutt[bImprovize],Improvize);
	ChangeControlValue(force,Hbutt[bStepProduce],StepProduce);
	ChangeControlValue(force,Hbutt[bStepGrammars],StepGrammars);
	ChangeControlValue(force,Hbutt[bPlanProduce],PlanProduce);
	ChangeControlValue(force,Hbutt[bDisplayProduce],DisplayProduce);
	ChangeControlValue(force,Hbutt[bUseEachSub],UseEachSub);
	ChangeControlValue(force,Hbutt[bAllItems],AllItems);
	ChangeControlValue(force,Hbutt[bTraceProduce],TraceProduce);
	ChangeControlValue(force,Hbutt[bDisplayTimeSet],DisplayTimeSet);
	ChangeControlValue(force,Hbutt[bStepTimeSet],StepTimeSet);
	ChangeControlValue(force,Hbutt[bTraceTimeSet],TraceTimeSet);
	ChangeControlValue(force,Hbutt[bAllowRandomize],AllowRandomize);
	ChangeControlValue(force,Hbutt[bCyclicPlay],CyclicPlay);
	ChangeControlValue(force,Hbutt[bSynchronizeStart],SynchronizeStart);
	ChangeControlValue(force,Hbutt[bDisplayItems],DisplayItems);
	ChangeControlValue(force,Hbutt[bResetWeights],ResetWeights);
	ChangeControlValue(force,Hbutt[bResetFlags],ResetFlags);
	ChangeControlValue(force,Hbutt[bResetControllers],ResetControllers);
	ChangeControlValue(force,Hbutt[bNoConstraint],NoConstraint);
	ChangeControlValue(force,Hbutt[bShowGraphic],ShowGraphic);
	ChangeControlValue(force,Hbutt[bComputeWhilePlay],ComputeWhilePlay);
	ChangeControlValue(force,Hbutt[bWriteMIDIfile],WriteMIDIfile);
	ChangeControlValue(force,Hbutt[bInteractive],Interactive);
	ChangeControlValue(force,Hbutt[bShowMessages],ShowMessages);
	ChangeControlValue(force,Hbutt[bCsound],OutCsound);
	ChangeControlValue(force,Hbutt[bCsoundTrace],CsoundTrace);
	ChangeControlValue(force,Hbutt[bOMS],Oms);
	}
#endif /* BP_CARBON_GUI */
return(OK);
}


#if BP_CARBON_GUI

ChangeControlValue(int force,ControlHandle hbutt,int ib)
{
if(ib != GetControlValue(hbutt)) {
	if(!force && MustBeSaved(ib)) UpdateDirty(TRUE,iSettings);
	SetControlValue(hbutt,ib);
	}
return(OK);
}

#endif /* BP_CARBON_GUI */


ConvertSpecialChars(char* line) {
int i,j;
char c;

for(i=j=0;; i++) {
	if(line[i+j] == '\r') {
		line[i+j] = '\0';
		break;
		}
	if(line[i+j] == '^') {
		j++;
		switch(line[i+j]) {
			case 'n': c = 'Â'; break;
			case 'r':
			case 'p': c = '\r'; break;
			case 't': c = '\t'; break;
			default: continue;
			}
		line[i] = c;
		if(c == 'Â') {
			line[++i] = '\r';
			j--;
			}
		continue;
		}
	line[i] = line[i+j];
	if(line[i] == '\0') break;
	}
return(OK);
}

#if BP_CARBON_GUI

GetThisTick(void)
{
char line[MAXFIELDCONTENT];
long p,q,s,v,c,k;
short itemtype;
int jj;
Handle itemhandle;
Rect r;

s = (long) GetCtrlValue(wTickDialog,dTickOn);
if(GetCtrlValue(wTickDialog,dSpecialTick)) {
	GetField(NULL,TRUE,wTickDialog,fThisTickVelocity,line,&p,&q);
	v =  p/q;
	if(v < 0 || v > 127) {
		sprintf(line,"Velocity range is 0..127\nCan't accept %ld",(long)v);
		Alert1(line);
		SetField(NULL,wTickDialog,fThisTickVelocity,"[?]");
		SelectField(NULL,wTickDialog,fThisTickVelocity,TRUE);
		return(FAILED);
		}
	GetField(NULL,TRUE,wTickDialog,fThisTickChannel,line,&p,&q);
	c =  p/q;
	if(c < 1 || c > 16) {
		sprintf(line,"Channel range is 1..16\nCan't accept %ld",(long)c);
		Alert1(line);
		SetField(NULL,wTickDialog,fThisTickChannel,"[?]");
		SelectField(NULL,wTickDialog,fThisTickChannel,TRUE);
		return(FAILED);
		}
	GetField(NULL,TRUE,wTickDialog,fThisTickKey,line,&p,&q);
	k =  p/q;
	if(k < 0 || k > 127) {
		sprintf(line,"Key range is 0..127\nCan't accept %ld",(long)k);
		Alert1(line);
		SetField(NULL,wTickDialog,fThisTickKey,"[?]");
		SelectField(NULL,wTickDialog,fThisTickKey,TRUE);
		return(FAILED);
		}
	ThisTick[iTick][jTick] = s + 2L * (v + 128L * ((c - 1L) + 128L * k));
	}
else ThisTick[iTick][jTick] = s;
jj = dPlayBeat + 55*iTick + jTick;
GetDialogItem(gpDialogs[wTimeBase],(short)jj,&itemtype,&itemhandle,&r);
/* HiliteControl((ControlHandle) itemhandle,11); */
if(s) {
	SwitchOn(NULL,wTimeBase,jj);
	TickThere = TRUE;
	}
else SwitchOff(NULL,wTimeBase,jj);
return(OK);
}


SetThisTick(void)
{
char line[MAXFIELDCONTENT];
int vel,ch,key;

if(iTick < 0 || jTick < 0) {
	if(Beta) Alert1("Err. SetThisTick()");
	return(FAILED);
	}
if(ThisTick[iTick][jTick] % 2L) {
	SwitchOn(NULL,wTickDialog,dTickOn);
	SwitchOff(NULL,wTickDialog,dTickOff);
	}
else {
	SwitchOn(NULL,wTickDialog,dTickOff);
	SwitchOff(NULL,wTickDialog,dTickOn);
	}
if(ThisTick[iTick][jTick] > 1) {
	SwitchOn(NULL,wTickDialog,dSpecialTick);
	SwitchOff(NULL,wTickDialog,dDefaultTick);
	ShowDialogItem(gpDialogs[wTickDialog],fThisTickVelocity);
	ShowDialogItem(gpDialogs[wTickDialog],fThisTickChannel);
	ShowDialogItem(gpDialogs[wTickDialog],fThisTickKey);
	}
else {
	SwitchOn(NULL,wTickDialog,dDefaultTick);
	SwitchOff(NULL,wTickDialog,dSpecialTick);
	HideDialogItem(gpDialogs[wTickDialog],fThisTickVelocity);
	HideDialogItem(gpDialogs[wTickDialog],fThisTickChannel);
	HideDialogItem(gpDialogs[wTickDialog],fThisTickKey);
	}
vel = TickVelocity[iTick];
ch = TickChannel[iTick];
key = TickKey[iTick];
FindTickValues(ThisTick[iTick][jTick],&vel,&ch,&key);
sprintf(line,"%ld",(long)vel);
SetField(NULL,wTickDialog,fThisTickVelocity,line);
sprintf(line,"%ld",(long)ch);
SetField(NULL,wTickDialog,fThisTickChannel,line);
sprintf(line,"%ld",(long)key);
SetField(NULL,wTickDialog,fThisTickKey,line);
return(OK);
}

#endif /* BP_CARBON_GUI */

/* Retrieve the name of a linked file of type doc from window w.
   doc is the "document index" of the name that is being looked for.
   filename should be storage with at least MAXNAME space. */
GetLinkedFileName(int w, int doc, char* filename)
{
long pos,posmax;
char *p,*q,line[MAXLIN];

if (filename == NULL) {
	if(Beta) Alert1("Err. GetLinkedFileName(). filename == NULL.");
	return(FAILED);
	}
if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. GetLinkedFileName(). Bad window index.");
	return(FAILED);
	}
if(doc < 0 || doc >= WMAX || FilePrefix[doc][0] == '\0') {
	if(Beta) Alert1("Err. GetLinkedFileName(). Bad document index.");
	return(FAILED);
	}
pos = ZERO;
posmax = GetTextLength(w);
do {
	if(ReadLine1(FALSE,w,&pos,posmax,line,MAXLIN) != OK) return(FAILED);
	if(line[0] == '\0') continue;
	if(line[0] == '-' && line[1] == '-') return(FAILED);
	p = line; q = FilePrefix[doc];
	if(Match(TRUE,&p,&q,4) && line[4] != '<' && line[4] != '\334') {
		Strip(line); // does this make sense?
		if (line[4] == ':')    // real filename does not begin with prefix
			p = &(line[5]);  // so, skip the prefix in this line
		else  p = line;
		if (strlen(p) > MAXNAME) continue;
		strcpy(filename,p);
		return(OK);
		}
	}
while(TRUE);
}

GetAlphaName(int w)
{
if (GetLinkedFileName(w,wAlphabet,FileName[wAlphabet]) == OK) {
		NoAlphabet = FALSE;
		return(OK);
		}
else return(FAILED);
}

GetMiName(void)
{
char name[MAXNAME];

if (GetLinkedFileName(wAlphabet,iObjects,name) == OK) {
	if(strcmp(FileName[iObjects],name) != 0) {
		strcpy(FileName[iObjects],name);
		ObjectMode = ObjectTry = FALSE;
		}
	return(OK);
	}
else return(FAILED);
}

GetInName(int w)
{
char name[MAXNAME];

if (GetLinkedFileName(w,wInteraction,name) == OK) {
	if(strcmp(FileName[wInteraction],name) != 0) {
		strcpy(FileName[wInteraction],name);
		LoadedIn = CompiledIn = FALSE;
		}
	return(OK);
	}
else return(FAILED);
}

GetGlName(int w)
{
char name[MAXNAME];

if (GetLinkedFileName(w,wGlossary,name) == OK) {
	if(strcmp(FileName[wGlossary],name) != 0) {
		strcpy(FileName[wGlossary],name);
		LoadedGl = CompiledGl = FALSE;
		}
	return(OK);
	}
else return(FAILED);
}

GetSeName(int w)
{
char name[MAXNAME];

if (GetLinkedFileName(w,iSettings,name) == OK) {
	if(strcmp(FileName[iSettings],name) != 0 || Dirty[iSettings]) {
		strcpy(FileName[iSettings],name);
		Created[iSettings] = FALSE;
#if BP_CARBON_GUI
		TellOthersMyName(iSettings);
#endif /* BP_CARBON_GUI */
		return(OK);
		}
	else return(FAILED); 	/* Name is unchanged: no need to reload */
	}
else return(FAILED);
}

#if BP_CARBON_GUI

GetKbName(int w)
{
int type,result;
FSSpec spec;
short refnum;
char name[MAXNAME];

result = FAILED;
if (GetLinkedFileName(w,wKeyboard,name) == OK) {
	if(strcmp(FileName[wKeyboard],name) != 0) {
		strcpy(FileName[wKeyboard],name);
		if(Token == FALSE && !ScriptExecOn) {
			if(Answer("Alphabet file indicated keyboard encoding.\nType tokens instead of normal text",
				'N') == OK) Token = TRUE;
			}
		type = gFileType[wKeyboard];
		c2pstrcpy(spec.name, name);
		spec.vRefNum = TheVRefNum[wKeyboard];
		spec.parID = WindowParID[wKeyboard];
		if (MyOpen(&spec,fsCurPerm,&refnum) == noErr ||
		    CheckFileName(wKeyboard,FileName[wKeyboard],&spec,&refnum,type,TRUE) == OK) {
			result = LoadKeyboard(refnum);
			}
		}
	else {
		if(Token && FileName[wKeyboard][0] == '\0') {
			if(!ScriptExecOn) Alert1("You can't use tokens ('Misc' menu) unless you define '-kb.' file in alphabet");
			Token = FALSE;
			result = ABORT;
			}
		}
}
if(Token && LoadOn && FileName[wKeyboard][0] == '\0') {
	Token = FALSE; MaintainMenus();
	}
return(result);
}

GetFileNameAndLoadIt(int wfile,int w,Int2ProcPtr loadit)
{
int r,type;
FSSpec spec;
short refnum;
char name[MAXNAME];

if(wfile < 0 || wfile >= WMAX) {
	if(Beta) Alert1("Err. GetFileNameAndLoadIt().(wfile < 0 || wfile >= WMAX");
	return(FAILED);
	}

if (GetLinkedFileName(w,wfile,name) == OK) {
		if(strcmp(FileName[wfile],name) != 0) {
			strcpy(FileName[wfile],name);
			type = gFileType[wfile];
			c2pstrcpy(spec.name, name);
			spec.vRefNum = TheVRefNum[wfile];
			spec.parID = WindowParID[wfile];
			if(MyOpen(&spec,fsCurPerm,&refnum) != noErr) {
				if(CheckFileName(wfile,FileName[wfile],&spec,&refnum,type,TRUE)
					!= OK) return(ABORT);
				}
			r = ((int (*)(short,int)) loadit)(refnum,FALSE);
			if(r == OK) SetName(wfile,TRUE,FALSE);
			return(r);
			}
		else return(FAILED); // is this correct? -- akozar
		}
else return(FAILED);
}

GetCsName(int w)
{
int r,type;
FSSpec spec;
short refnum;
char name[MAXNAME];

if (GetLinkedFileName(w,wCsoundInstruments,name) == OK) {
	if(strcmp(FileName[wCsoundInstruments],name) != 0) {
		strcpy(FileName[wCsoundInstruments],name);
		type = gFileType[wCsoundInstruments];
		c2pstrcpy(spec.name, name);
		spec.vRefNum = TheVRefNum[wCsoundInstruments];
		spec.parID = WindowParID[wCsoundInstruments];
		if(MyOpen(&spec,fsCurPerm,&refnum) != noErr) {
			if(CheckFileName(wCsoundInstruments,FileName[wCsoundInstruments],&spec,&refnum,type,TRUE)
				!= OK) return(FAILED);
			}
		r = LoadCsoundInstruments(refnum,FALSE);
		if(r == OK) SetName(wCsoundInstruments,TRUE,FALSE);
		return(r);
		}
	else return(FAILED); // is this right? -- akozar
	}
else return(FAILED);
}

GetTimeBaseName(int w)
{
int type;
FSSpec spec;
short refnum;
char name[MAXNAME];

if (GetLinkedFileName(w,wTimeBase,name) == OK) {
	if(strcmp(FileName[wTimeBase],name) != 0) {
		strcpy(FileName[wTimeBase],name);
		type = gFileType[wTimeBase];
		c2pstrcpy(spec.name, name);
		spec.vRefNum = TheVRefNum[wTimeBase];
		spec.parID = WindowParID[wTimeBase];
		if(MyOpen(&spec,fsCurPerm,&refnum) != noErr) {
			if(CheckFileName(wTimeBase,FileName[wTimeBase],&spec,&refnum,type,TRUE)
				!= OK) return(FAILED);
			}
		LoadTimeBase(refnum);
		return(OK);
		}
	else return(FAILED);  // is this correct? -- akozar
	}
else return(FAILED);
}

#endif /* BP_CARBON_GUI */

ChangeMetronom(int j,double x)
{
int striated;
double p,q,newp,newq;

striated = (Nature_of_time == STRIATED);
newp = Pclock; newq = Qclock;

switch(j) {
	case 13:
		MakeRatio((double)ULONG_MAX,x,&p,&q);
		if(p < 0.) {
			sprintf(Message,"Metronome cannot be set to negative value. '%.4f' not accepted",
				x);
			Alert1(Message);
			return(ABORT);
			}
		if(p == ZERO) {
			newp = 0.; newq = 1.;
			}
		else {
			if(Simplify((double)INT_MAX,p,60.*q,&newq,&newp) != OK) 
				Simplify((double)INT_MAX,floor(p/60.),q,&newq,&newp);
			}
		break;
	case 14:
		if(Pclock < 1. && !NotFoundMetronom) {
			Alert1("Setting time to 'striated' is inconsistent with having no clock");
			striated = FALSE;
			goto MAKECHANGE;
			}
		striated = TRUE;
		break;
	case 15:
		striated = FALSE;
		break;
	}
if((striated && Nature_of_time != STRIATED)
		|| (!striated && Nature_of_time == STRIATED)
		|| newp != Pclock || newq != Qclock) {
MAKECHANGE:
	Pclock = newp;
	Qclock = newq;
	if(striated) Nature_of_time = STRIATED;
	else Nature_of_time = SMOOTH;
	SetTempo();
	SetTimeBase();
#if BP_CARBON_GUI
	ShowWindow(Window[wMetronom]);
	BringToFront(Window[wMetronom]);
	UpdateDirty(TRUE,iSettings);
#endif /* BP_CARBON_GUI */
	SetTickParameters(0,MAXBEATS);
	ResetTickFlag = TRUE;
	}
return(OK);
}


SetTempo(void)
{
Rect r;
ControlHandle itemhandle;
short itemtype;
int speed_change;
double p,q;
char line[MAXFIELDCONTENT];
double speedratio;

if(PedalOrigin != -1) {
	speed_change = PedalPosition - PedalOrigin;
	speedratio = exp((double)((double) speed_change * Ke));
	if(speedratio > 20.) speedratio = 20.;
	if(speedratio < 0.05) speedratio = 0.05;
	Nalpha = (long) (speedratio * 100L);
	Simplify((double)INT_MAX,100. * OldPclock,Nalpha * OldQclock,&Pclock,&Qclock);
	Nalpha = 100L;
	}
if(Pclock == 0.) {
	strcpy(line,"[no clock]");
	Nature_of_time = SMOOTH;
	}
else {
	if(Simplify((double)INT_MAX,(double)60L*Qclock,Pclock,&p,&q) != OK)
		Simplify((double)INT_MAX,Qclock,floor((double)Pclock/60.),&p,&q);
	sprintf(line,"%.4f", ((double)p)/q);
	}

#if BP_CARBON_GUI
SetField(NULL, wMetronom, fTempo, line);

/* Set smooth or striated */
if(Nature_of_time == STRIATED) {
	GetDialogItem(gpDialogs[wMetronom],dSmooth,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,0);
	GetDialogItem(gpDialogs[wMetronom],dStriated,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,1);
	}
else {
	GetDialogItem(gpDialogs[wMetronom],dSmooth,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,1);
	GetDialogItem(gpDialogs[wMetronom],dStriated,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,0);
	}
#endif /* BP_CARBON_GUI */
return(OK);
}

#if BP_CARBON_GUI

GetTempo(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char s[255];
Str255 t;
unsigned long p,q;
double oldp,oldq;

if(!Dirty[wMetronom]) return(OK);
GetDialogItem(gpDialogs[wMetronom],fTempo,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,s);
Dirty[wMetronom] = FALSE;
oldp = Pclock; oldq = Qclock;
if(FloatToNiceRatio(s,&p,&q) != OK) return(FAILED);
if(p == ZERO) {
	Pclock = ZERO; Qclock = 1L;
	SetTempo();
	SetTimeBase();
	return(OK);
	}
if(Simplify((double)INT_MAX,(double)p,(double)60L*q,&Qclock,&Pclock) != OK)
	Simplify((double)INT_MAX,floor((double)p/60L),(double)q,&Qclock,&Pclock);
if(oldp != Pclock || oldq != Qclock) {
	SetTickParameters(0,MAXBEATS);
	ResetTickFlag = TRUE;
	}
SetTempo();
SetTimeBase();
SetGrammarTempo();
return(OK);
}

#endif /* BP_CARBON_GUI */

SetGrammarTempo(void)
// Here we only erase the line containing "_mm()" and tell BP2 that the grammar is not compiled
// so that a fresh line is  inserted during the compilation
{
int w,j,gap,rep;
char *q,**p_line,line1[MAXLIN],line2[MAXLIN],line3[MAXLIN];
long pos,posline,posmax;

if(IsEmpty(wGrammar) || !Dirty[wMetronom]) return(OK);
w = wGrammar;
posmax = GetTextLength(w);
p_line = NULL;
pos = posline = ZERO;
rep = OK;
MystrcpyHandleToString(MAXLIN,0,line1,(*p_GramProcedure)[13]);	/* _mm() */
MystrcpyHandleToString(MAXLIN,0,line2,(*p_GramProcedure)[14]);	/* _striated */
MystrcpyHandleToString(MAXLIN,0,line3,(*p_GramProcedure)[15]);	/* _smooth() */
while(ReadLine(YES,w,&pos,posmax,&p_line,&gap) == OK) {
	if((*p_line)[0] == '\0' || (*p_line)[0] == '\r') continue;
	for(j=0; j < WMAX; j++) {
		if(FilePrefix[j][0] == '\0') continue;
		q = &(FilePrefix[j][0]);
		if(Match(TRUE,p_line,&q,4)) goto NEXTLINE;
		}
	if(Mystrcmp(p_line,"DATA:") == 0) break;
	if(Mystrcmp(p_line,"COMMENT:") == 0) break;
	if(Mystrcmp(p_line,"TIMEPATTERNS:") == 0) {
		do {
			if(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) != OK) goto END;
			if((*p_line)[0] == '\0') continue;
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		goto NEXTLINE;
		}
	q = GRAMstring;
	if(Match(FALSE,p_line,&q,strlen(GRAMstring))) break;
	MyLock(TRUE,(Handle)p_line);
	if(strstr(*p_line,line1) != NULLSTR) {	/* _mm() */
FOUNDIT:
		SelectBehind(posline,pos,TEH[w]);
		TextDelete(w);
		CompiledGr = FALSE;
		MyUnlock((Handle)p_line);
		break;
		}
	if(strstr(*p_line,line2) != NULLSTR) goto FOUNDIT;
	if(strstr(*p_line,line3) != NULLSTR) goto FOUNDIT;
	MyUnlock((Handle)p_line);
NEXTLINE:
	posline = pos;
	}
	
END:
MyDisposeHandle((Handle*)&p_line);
return(OK);
}

#if BP_CARBON_GUI

SetBufferSize(void)
{
Rect r;
ControlHandle itemhandle;
short itemtype;
char line[MAXFIELDCONTENT];

sprintf(line,"%ld",(long)BufferSize / 2L - 1L);
SetField(NULL, wBufferSize, fBufferSize, line);
sprintf(line,"%ld",(long)DeftBufferSize / 2L - 1L);
SetField(NULL, wBufferSize, fDeftBufferSize, line);
if(UseBufferLimit) {
	GetDialogItem(gpDialogs[wBufferSize],dNoSizeLimit,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,0);
	GetDialogItem(gpDialogs[wBufferSize],dYesSizeLimit,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,1);
	}
else {
	GetDialogItem(gpDialogs[wBufferSize],dNoSizeLimit,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,1);
	GetDialogItem(gpDialogs[wBufferSize],dYesSizeLimit,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,0);
	}
return(OK);
}


GetBufferSize(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char s[MAXFIELDCONTENT];
Str255 t;
long x;

if(!Dirty[wBufferSize]) return(OK);
InputOn++;
GetDialogItem(gpDialogs[wBufferSize],fBufferSize,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,s);
x = atol(s);
if(x < 2L) {
	Alert1("Minimum initial buffer size: 2 symbols");
	x = 2L;
	}
if(x > 100000L) {
	Alert1("Maximum initial buffer size: 100,000 symbols. (It may be expanded during computation)");
	x = 100000L;
	}
BufferSize = 2L * (x + 1L);
GetDialogItem(gpDialogs[wBufferSize],fDeftBufferSize,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,s);
x = atol(s);
if(x < 2L) {
	Alert1("Minimum initial buffer size: 2 symbols");
	x = 2L;
	}
if(x > 100000L) {
	Alert1("Maximum initial buffer size: 100,000 symbols. (It may be expanded during computation)");
	x = 100000L;
	}
DeftBufferSize = 2L * (x + 1L);
UpdateDirty(TRUE,iSettings);
InputOn--;
return(OK);
}


SetGraphicSettings(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char line[MAXFIELDCONTENT];

WriteFloatToLine(line,(double) (GraphicScaleQ * 5.) / (double) GraphicScaleP);
SetField(NULL, wGraphicSettings, fGraphicScale, line);
if(StartFromOne) {
	GetDialogItem(gpDialogs[wGraphicSettings],dZero,&itemtype,&itemhandle,&r);
	SetControlValue((ControlHandle) itemhandle,0);
	GetDialogItem(gpDialogs[wGraphicSettings],dOne,&itemtype,&itemhandle,&r);
	SetControlValue((ControlHandle) itemhandle,1);
	}
else {
	GetDialogItem(gpDialogs[wGraphicSettings],dZero,&itemtype,&itemhandle,&r);
	SetControlValue((ControlHandle) itemhandle,1);
	GetDialogItem(gpDialogs[wGraphicSettings],dOne,&itemtype,&itemhandle,&r);
	SetControlValue((ControlHandle) itemhandle,0);
	}
return(OK);
}


GetGraphicSettings(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char s[MAXFIELDCONTENT];
Str255 t;
unsigned long p,q;
double pp,qq;
int rep;

InputOn++;
rep = FAILED;
GetDialogItem(gpDialogs[wGraphicSettings],fGraphicScale,&itemtype,
	&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,s);
if((FloatToNiceRatio(s,&p,&q) != OK) || (p == ZERO)
		|| (Simplify((double)INT_MAX,(double)5. * q,(double)p,&pp,&qq) != OK)) {
	Alert1("Scale out of range");
	goto OUT;
	}
while((pp > INT_MAX) || (qq > INT_MAX)) {
	pp = (pp / 2.);
	qq = (qq / 2.);
	}
if(GraphicScaleP != pp || GraphicScaleQ != qq) {
	Dirty[wGraphicSettings] = Dirty[iSettings] = TRUE;
	}
GraphicScaleP = (int) pp;
GraphicScaleQ = (int) qq;
/* sprintf(Message,"Scale = %ld / %ld",(long)GraphicScaleP,(long)GraphicScaleQ);
ShowMessage(TRUE,wMessage,Message); */
rep = OK;

OUT:
InputOn--;
return(rep);
}


SetTimeAccuracy(void)
{
Rect r;
ControlHandle itemhandle;
short itemtype;
char line[MAXFIELDCONTENT];

sprintf(line,"%ld",(long)Time_res);
SetField(NULL, wTimeAccuracy, fTimeRes, line);
sprintf(line,"%ld",(long)Quantization);
SetField(NULL, wTimeAccuracy, fQuantize, line);
sprintf(line,"%ld",(long)SetUpTime);
SetField(NULL, wTimeAccuracy, fSetUpTime, line);
if(QuantizeOK) {
	GetDialogItem(gpDialogs[wTimeAccuracy],dOff,&itemtype,
		(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,0);
	GetDialogItem(gpDialogs[wTimeAccuracy],dOn,&itemtype,
		(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,1);
	if(Pclock < 0.9 && !LoadOn) {
		Alert1("Quantization requires a metronom value. It has been set to mm = 60");
		Pclock = Qclock = 1000.;
		SetTempo();
		BPActivateWindow(SLOW,wMetronom);
		}
	}
else {
	GetDialogItem(gpDialogs[wTimeAccuracy],dOff,&itemtype,
		(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,1);
	GetDialogItem(gpDialogs[wTimeAccuracy],dOn,&itemtype,
		(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,0);
	}
return(OK);
}


GetTimeAccuracy(void)
{
Rect r;
int i;
Handle itemhandle;
long k;
short itemtype;
char line[MAXFIELDCONTENT];
Str255 t;

InputOn++;
GetDialogItem(gpDialogs[wTimeAccuracy],fSetUpTime,&itemtype,
	&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,line);
i = (int) atol(line); 	/* Don't use atoi() because int's are 4 bytes */
if(i < 0 || i > 2000) {
	Alert1("Range of set-up time: 0 - 2000ms");
	if(i > 2000) i = 2000;
	else i = 0;
	}
SetUpTime = i;
GetDialogItem(gpDialogs[wTimeAccuracy],fTimeRes,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,line);
k = atol(line);
if(k < 1L) {
	Alert1("Minimum time resolution: 1ms");
	Time_res = 1L;
	SetTimeAccuracy();
	InputOn--;
	return(FAILED);
	}
if(k != Time_res) 
	Interrupted = Dirty[wTimeAccuracy] = Dirty[iSettings] = TRUE;
Time_res = k;
GetDialogItem(gpDialogs[wTimeAccuracy],fQuantize,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,line);
k = atol(line);
if(k < Time_res) {
	sprintf(Message,"Minimum quantization: %ldms",(long)Time_res);
	Alert1(Message);
	Quantization = Time_res;
	SetTimeAccuracy();
	InputOn--;
	return(FAILED);
	}
if(k != Quantization) {
	Dirty[wTimeAccuracy] = Dirty[iSettings] = TRUE;
	FixedMaxQuantization = AskedTempMemory = FALSE;
	}
Quantization = k;
SetGraphicSettings();
InputOn--;
return(OK);
}


SetKeyboard(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char line[MAXFIELDCONTENT];
int i,j;

for(i=0; i < 52; i++) {
	if(i < 26) j = i + fA;
	else j = i - 26 + fa;
	if((*p_Token)[Key(i,KeyboardType)] != NULL) {
		MystrcpyTableToString(MAXFIELDCONTENT,line,p_Token,Key(i,KeyboardType));
		SetField(NULL, wKeyboard, j, line);
		}
	}
return(OK);
}


Key(int i,int keyboardtype)
{
int j;

switch(keyboardtype) {
	case AZERTY: break;
	case QWERTY:
		j = 0; if(i > 25) {
			j = 26; i -= 26;
			}
		switch(i) {
			case 0: i = 16; break;	/* A */
			case 1: i = i; break;
			case 2: i = i; break;
			case 3: i = i; break;
			case 4: i = i; break;
			case 5: i = i; break;
			case 6: i = i; break;
			case 7: i = i; break;
			case 8: i = i; break;
			case 9: i = i; break;
			case 10: i = i; break;
			case 11: i = i; break;
			case 12: i = i; break;
			case 13: i = i; break;
			case 14: i = i; break;
			case 15: i = i; break;
			case 16: i = 0; break;	/* Q */
			case 17: i = i; break;
			case 18: i = i; break;
			case 19: i = i; break;
			case 20: i = i; break;
			case 21: i = i; break;
			case 22: i = 25; break;	/* W */
			case 23: i = i; break;
			case 24: i = i; break;
			case 25: i = 22; break;	/* Z */
			}
		i += j;
	}
return(i);
}


ResetKeyboard(int quick)
{
Rect r;
Handle itemhandle;
short itemtype;
char line[2];
int i,j,c,rep;

if(!quick && (rep=Answer("Suppress all tokens",'N')) != OK) return(rep);

DOIT:
if(SaveCheck(wKeyboard) == ABORT) return(ABORT);
line[1] = '\0';
for(i=0; i < 52; i++) {
	if(i <= 25) c = i + 'A';
	else c = '.';
	if(i < 26) j = i + fA;
	else j = i - 26 + fa;
	line[0] = c;
	SetField(NULL, wKeyboard, j, line);
	}
Dirty[wKeyboard] = FALSE;
Token = SpaceOn = FALSE;
SwitchOff(NULL,wKeyboard,dToken);
ForgetFileName(wKeyboard);
GetKeyboard();
return(OK);
}


GetKeyboard(void)
{
Rect r;
Handle itemhandle;
short itemtype;
int i,j;
char line[MAXFIELDCONTENT],**ptr;
Str255 t;

for(i=0; i < 52; i++) {
	if(i < 26) j = i + fA;
	else j = i - 26 + fa;
	GetDialogItem(gpDialogs[wKeyboard],j,&itemtype,&itemhandle,&r);
	GetDialogItemText(itemhandle,t);
	MyPtoCstr(MAXFIELDCONTENT,t,line);
	ptr = (*p_Token)[Key(i,KeyboardType)];
	MyDisposeHandle((Handle*)&ptr);
	if((ptr = (char**) GiveSpace((Size)strlen(line)+1)) == NULL) return(ABORT);
	(*p_Token)[Key(i,KeyboardType)] = ptr;
	MystrcpyStringToTable(p_Token,Key(i,KeyboardType),line);
	}
return(OK);
}


SetDefaultStrikeMode(void)
{
if(StrikeAgainDefault) {
	SwitchOn(StrikeModePtr,-1,bDefaultStrikeAgain);
	SwitchOff(StrikeModePtr,-1,bDefaultDontStrikeAgain);
	}
else {
	SwitchOn(StrikeModePtr,-1,bDefaultDontStrikeAgain);
	SwitchOff(StrikeModePtr,-1,bDefaultStrikeAgain);
	}
return(OK);
}


SetFileSavePreferences(void)
{
char line[MAXFIELDCONTENT];
	
switch(FileSaveMode) {
	case ALLSAME:
		SwitchOn(FileSavePreferencesPtr,-1,bSaveAllToSame);
		SwitchOff(FileSavePreferencesPtr,-1,bSaveAllToSameWithPrompt);
		SwitchOff(FileSavePreferencesPtr,-1,bEachToNew);
		break;
	case ALLSAMEPROMPT:
		SwitchOn(FileSavePreferencesPtr,-1,bSaveAllToSameWithPrompt);
		SwitchOff(FileSavePreferencesPtr,-1,bSaveAllToSame);
		SwitchOff(FileSavePreferencesPtr,-1,bEachToNew);
		break;
	case NEWFILE:
		SwitchOn(FileSavePreferencesPtr,-1,bEachToNew);
		SwitchOff(FileSavePreferencesPtr,-1,bSaveAllToSame);
		SwitchOff(FileSavePreferencesPtr,-1,bSaveAllToSameWithPrompt);
		break;
	}
if(FileWriteMode == NOW) {
	SwitchOn(FileSavePreferencesPtr,-1,bWriteNow);
	SwitchOff(FileSavePreferencesPtr,-1,bWriteLater);
	}
else {
	SwitchOn(FileSavePreferencesPtr,-1,bWriteLater);
	SwitchOff(FileSavePreferencesPtr,-1,bWriteNow);
	}
switch(MIDIfileType) {
	case 0:
		SwitchOn(FileSavePreferencesPtr,-1,bType0);
		SwitchOff(FileSavePreferencesPtr,-1,bType1);
		SwitchOff(FileSavePreferencesPtr,-1,bType2);
		break;
	case 1:
		SwitchOn(FileSavePreferencesPtr,-1,bType1);
		SwitchOff(FileSavePreferencesPtr,-1,bType0);
		SwitchOff(FileSavePreferencesPtr,-1,bType2);
		break;
	case 2:
		SwitchOn(FileSavePreferencesPtr,-1,bType2);
		SwitchOff(FileSavePreferencesPtr,-1,bType1);
		SwitchOff(FileSavePreferencesPtr,-1,bType0);
		break;
	}
switch(CsoundFileFormat) {
	case MAC:
		SwitchOn(FileSavePreferencesPtr,-1,bTypeMac);
		SwitchOff(FileSavePreferencesPtr,-1,bTypeDos);
		SwitchOff(FileSavePreferencesPtr,-1,bTypeUnix);
		break;
	case DOS:
		SwitchOn(FileSavePreferencesPtr,-1,bTypeDos);
		SwitchOff(FileSavePreferencesPtr,-1,bTypeMac);
		SwitchOff(FileSavePreferencesPtr,-1,bTypeUnix);
		break;
	case UNIX:
		SwitchOn(FileSavePreferencesPtr,-1,bTypeUnix);
		SwitchOff(FileSavePreferencesPtr,-1,bTypeDos);
		SwitchOff(FileSavePreferencesPtr,-1,bTypeMac);
		break;
	}
if(OutCsound)
	SetField(FileSavePreferencesPtr,-1,fCsoundFileName,CsFileName);
if(WriteMIDIfile)
	SetField(FileSavePreferencesPtr,-1,fMIDIFileName,MIDIfileName);
sprintf(line,"%.2f",MIDIfadeOut);
SetField(FileSavePreferencesPtr,-1,fFadeOut,line);
return(OK);
}


GetFileSavePreferences(void)
{
int result;
long p,q;
char line[MAXFIELDCONTENT];

if(FileSaveMode == ALLSAME || FileSaveMode == ALLSAMEPROMPT) {
	GetCsoundScoreName();
	GetMIDIfileName();
	}
if(GetField(FileSavePreferencesPtr,YES,-1,fFadeOut,LineBuff,&p,&q) != OK) {
	p = 3L; q = 1L;
	}

FADEOUTVALUE:
MIDIfadeOut = ((float)p) / q;
if(MIDIfadeOut < 0. || MIDIfadeOut > 100.) {
	ShowWindow(GetDialogWindow(FileSavePreferencesPtr));
	BringToFront(GetDialogWindow(FileSavePreferencesPtr));
	BPUpdateDialog(FileSavePreferencesPtr);
	SelectField(FileSavePreferencesPtr,-1,fFadeOut,TRUE);
	Alert1("Range for MIDI fade out is 0..100 seconds");
	result = AnswerWith("Set fade out to...","0.00",line);
	if(result != OK) goto ERR;
	else {
		Myatof(line,&p,&q);
		MIDIfadeOut = ((float)p) / q;
		sprintf(line,"%.2f",MIDIfadeOut);
		SetField(FileSavePreferencesPtr,-1,fFadeOut,line);
		BPUpdateDialog(FileSavePreferencesPtr);
		}
	goto FADEOUTVALUE;
	}
return(OK);

ERR:
SetField(FileSavePreferencesPtr,-1,fFadeOut,"[?]");
BPUpdateDialog(FileSavePreferencesPtr);
SelectField(FileSavePreferencesPtr,-1,fFadeOut,TRUE);
return(FAILED);
}


GetControlParameters(void)
{
char line[MAXFIELDCONTENT];
long p,q;
int oldtransposevalue;

oldtransposevalue = TransposeValue;
GetField(NULL,TRUE,wControlPannel,fTransposeInput,line,&p,&q);
TransposeValue = (unsigned) p/q;
if(TransposeValue != oldtransposevalue && TransposeValue != 0
	&& TransposeInput && ReadKeyBoardOn && Jcontrol == -1 && LastEditWindow != wScript) {
	if(!EmptyBeat) Print(LastEditWindow," ");
	PrintHandle(LastEditWindow,(*p_PerformanceControl)[33]);
	sprintf(Message,"(%ld)",(long)-TransposeValue);
	Print(LastEditWindow,Message);
	EmptyBeat = FALSE;
	}
return(OK);
}


GetTuning(void)
{
char line[MAXFIELDCONTENT];
long p,q;
int i,oldC4key;
double x;

oldC4key = C4key;
GetField(TuningPtr,TRUE,-1,fC4key,line,&p,&q);
i = p / q;
if(i < 2 || i > 127) {
	sprintf(Message,"Key for C4 should be in range 2..127 (typ. 60). Can't accept %ld",(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(TuningPtr));
	SelectWindow(GetDialogWindow(TuningPtr));
	SetField(TuningPtr,-1,fC4key,"[?]");
	return(FAILED);
	}
C4key = i;
if(C4key != oldC4key) CompiledGr = CompiledGl = FALSE;

GetField(TuningPtr,TRUE,-1,fA4freq,line,&p,&q);
x = ((double) p) / q;
if(x < 25. || x > 2000.) {
	sprintf(Message,"Frequency for A4 should be in range 25..2000 (typ. 440). Can't accept %.2f",x);
	Alert1(Message);
	ShowWindow(GetDialogWindow(TuningPtr));
	SelectWindow(GetDialogWindow(TuningPtr));
	SetField(TuningPtr,-1,fA4freq,"[?]");
	return(FAILED);
	}
A4freq = x;
return(OK);
}


SetTuning(void)
{
sprintf(Message,"%ld",(long)C4key);
SetField(TuningPtr,-1,fC4key,Message);
sprintf(Message,"%.4f",A4freq);
SetField(TuningPtr,-1,fA4freq,Message);
return(OK);
}


GetDefaultPerformanceValues(void)
{
char line[MAXFIELDCONTENT];
long p,q;
int i;

GetField(DefaultPerformanceValuesPtr,TRUE,-1,fDeftVolume,line,&p,&q);
i = p / q;
if(i < 1 || i > 127) {
	sprintf(Message,"Default volume should be in range 1..127 (typ. 90). Can't accept %ld",
		(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SetField(DefaultPerformanceValuesPtr,-1,fDeftVolume,"90");
	return(FAILED);
	}
DeftVolume = i;

GetField(DefaultPerformanceValuesPtr,TRUE,-1,fDeftVelocity,line,&p,&q);
i = p / q;
if(i < 1 || i > 127) {
	sprintf(Message,"Default velocity should be in range 1..127 (typ. 64). Can't accept %ld",
		(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SetField(DefaultPerformanceValuesPtr,-1,fDeftVelocity,"64");
	return(FAILED);
	}
DeftVelocity = i;

GetField(DefaultPerformanceValuesPtr,TRUE,-1,fDeftPanoramic,line,&p,&q);
i = p / q;
if(i < 0 || i > 127) {
	sprintf(Message,"Default panoramic should be in range 0..127 (typ. 64). Can't accept %ld",
		(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SetField(DefaultPerformanceValuesPtr,-1,fDeftPanoramic,"64");
	return(FAILED);
	}
DeftPanoramic = i;

GetField(DefaultPerformanceValuesPtr,TRUE,-1,fPanoramicController,line,&p,&q);
i = p / q;
if(i < 0 || i > 127) {
	sprintf(Message,"Panoramic control index should be in range 0..127 (typ. 10). Can't accept %ld",
		(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SetField(DefaultPerformanceValuesPtr,-1,fPanoramicController,"10");
	return(FAILED);
	}
PanoramicController = i;

GetField(DefaultPerformanceValuesPtr,TRUE,-1,fVolumeController,line,&p,&q);
i = p / q;
if(i < 0 || i > 127) {
	sprintf(Message,"Volume control index should be in range 0..127 (typ. 7). Can't accept %ld",
		(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SetField(DefaultPerformanceValuesPtr,-1,fVolumeController,"7");
	return(FAILED);
	}
VolumeController = i;

GetField(DefaultPerformanceValuesPtr,TRUE,-1,fSamplingRate,line,&p,&q);
i = p / q;
if(i < 1 || i > 500) {
	sprintf(Message,"Default sample rate should be in range 1..500 (typ. 50). Can't accept %ld",
		(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SetField(DefaultPerformanceValuesPtr,-1,fSamplingRate,"50");
	return(FAILED);
	}
SamplingRate = i;

return(OK);
}


SetDefaultPerformanceValues(void)
{
sprintf(Message,"%ld",(long)DeftVolume);
SetField(DefaultPerformanceValuesPtr,-1,fDeftVolume,Message);

sprintf(Message,"%ld",(long)DeftVelocity);
SetField(DefaultPerformanceValuesPtr,-1,fDeftVelocity,Message);

sprintf(Message,"%ld",(long)DeftPanoramic);
SetField(DefaultPerformanceValuesPtr,-1,fDeftPanoramic,Message);

sprintf(Message,"%ld",(long)PanoramicController);
SetField(DefaultPerformanceValuesPtr,-1,fPanoramicController,Message);

sprintf(Message,"%ld",(long)VolumeController);
SetField(DefaultPerformanceValuesPtr,-1,fVolumeController,Message);

sprintf(Message,"%ld",(long)SamplingRate);
SetField(DefaultPerformanceValuesPtr,-1,fSamplingRate,Message);
return(OK);
}

#endif /* BP_CARBON_GUI */

#if 0
pascal void DrawButtonBorder(DialogPtr thedialog)
{
Handle thecontrol;
short type;
Rect r;

GetDialogItem(thedialog,1,&type,&thecontrol,&r);
PenSize(3,3);
InsetRect(&r,-2,-2);
FrameRoundRect(&r,16,16);
}
#endif


/* Pause() and GetCap() are used to get user input while stepping thru timesetting.
   Will need to provide an alternative for user-interactivity OR create a "stepping
   API" for the timesetting code if we want to retain step functionality. */

// NOTE: the return value of this function is not checked in several places ...
Pause(void)
{
char c;

c = 0;
StopWait();
#if BP_CARBON_GUI
switch(Alert(StepAlert,0L)) {
	case aContinue: return(' ');
	case aStep: return('S');
	case aUndo: return('U');
	case aAbort: return('Q');
	}
#else
// FIXME: do we need a console equivalent of this function ??
#endif
return(' ');
}


char GetCap(void)
{
EventRecord theEvent;
int r,compiledmem,dirtymem;

ShowSelect(CENTRE,wTrace);
BPActivateWindow(SLOW,wTrace);
ShowMessage(TRUE,wMessage,"Type answer!");

#if BP_CARBON_GUI
while(!WaitNextEvent(everyEvent,&theEvent,3L,NULL) || ((theEvent.what != keyDown)
		&& (theEvent.what != autoKey))) {
	ListenMIDI(0,0,0);
	if(Button()) {
		compiledmem = CompiledGr;
		dirtymem = Dirty[wAlphabet]; Dirty[wAlphabet] = FALSE;
		while((r = MainEvent()) != RESUME && r != STOP && r != EXIT){};
		if(r == EXIT) r = STOP;
		if(Dirty[wAlphabet]) {
			Alert1("Alphabet changed. Must recompile...");
			return('Q');
			}
		Dirty[wAlphabet] = dirtymem;
		if(compiledmem && !CompiledGr) {
			Alert1("Grammar changed. Must recompile...");
			return('Q');
			}
		if(r == STOP) return('Q');
		ShowMessage(TRUE,wMessage,"Type answer!");
		}
	}
return(UpperCase((char) (theEvent.message & charCodeMask)));
#else
// FIXME: do we need a console equivalent of this function ??
return ('Q');
#endif
}


Date(char line[])
{
unsigned long datetime;
char dd[MAXNAME],tt[MAXNAME];

#if BP_CARBON_GUI
Str255 pascalline;
Handle i1h;		        /* handle to an Intl1 or Intl0 Rec  */

/* GetIntlResource() seems to always return the same handles with Carbon,
   so I do not think that they need to be deallocated - akozar */
i1h = GetIntlResource(1); /* Note: does not return a resource handle on Carbon */
GetDateTime(&datetime);	  /* See DateTimeUtils.h */
if (i1h != NULL) {
	DateString(datetime,abbrevDate,pascalline, i1h);
	MyPtoCstr(MAXNAME,pascalline,dd);
	}
else  dd[0] = '\0';
i1h = GetIntlResource(0);
if (i1h != NULL) {
	TimeString(datetime,FALSE,pascalline, i1h);
	MyPtoCstr(MAXNAME,pascalline,tt);
	}
else  tt[0] = '\0';
#else
time_t curtime;
struct tm *loctime;

time(&curtime);
loctime = localtime(&curtime);
// format the date
if (strftime(dd, MAXNAME, "%a, %b %d, %Y", loctime) == 0) {
	if (Beta) Alert1("Err. Date(): strftime() results were too long for dd.");
	dd[0] = '\0';
	}
// format the time
if (strftime(tt, MAXNAME, "%I:%M %p", loctime) == 0) {
	if (Beta) Alert1("Err. Date(): strftime() results were too long for tt.");
	tt[0] = '\0';
	}
#endif

sprintf(line,"%s %s -- %s",DateMark,dd,tt);
return(OK);
}


FixStringConstant(char* line)
{
int i,j,maxparam,found;
Handle h;
char ****pp_h,**ptr;

Strip(line);
if(line[0] == '\0') goto ERR;

found = FALSE;

if(p_StringConstant == NULL) maxparam = 0;
else maxparam = (MyGetHandleSize((Handle)p_StringConstant) / sizeof(char**));

if(maxparam == 0) {
	if((pp_h = (char****) GiveSpace((Size)(10) * sizeof(char**))) == NULL) return(ABORT);
	p_StringConstant = pp_h;
	maxparam = 10;
	
	if((ptr = (char**) GiveSpace((Size)(strlen("pitchbend")+1))) == NULL) return(ABORT);
	(*p_StringConstant)[IPITCHBEND] = ptr;
	MystrcpyStringToTable(p_StringConstant,IPITCHBEND,"pitchbend");
	if((ptr = (char**) GiveSpace((Size)(strlen("volume")+1))) == NULL) return(ABORT);
	(*p_StringConstant)[IVOLUME] = ptr;
	MystrcpyStringToTable(p_StringConstant,IVOLUME,"volume");
	if((ptr = (char**) GiveSpace((Size)(strlen("modulation")+1))) == NULL) return(ABORT);
	(*p_StringConstant)[IMODULATION] = ptr;
	MystrcpyStringToTable(p_StringConstant,IMODULATION,"modulation");
	if((ptr = (char**) GiveSpace((Size)(strlen("pressure")+1))) == NULL) return(ABORT);
	(*p_StringConstant)[IPRESSURE] = ptr;
	MystrcpyStringToTable(p_StringConstant,IPRESSURE,"pressure");
	if((ptr = (char**) GiveSpace((Size)(strlen("panoramic")+1))) == NULL) return(ABORT);
	(*p_StringConstant)[IPANORAMIC] = ptr;
	MystrcpyStringToTable(p_StringConstant,IPANORAMIC,"panoramic");
	
	for(i=5; i < maxparam; i++) (*p_StringConstant)[i] = NULL;
	}

for(j=0; j < maxparam; j++) {
	if((*p_StringConstant)[j] == NULL) break;
	if(Mystrcmp((*p_StringConstant)[j],line) == 0) {
		found = TRUE;
		break;
		}
	}
if(found) return(j);

if(j >= MAXSTRINGCONSTANTS) {
	sprintf(Message,
			"Too many identifiers found (max %ld)\nCan't store '%s'\n",
				(long)MAXSTRINGCONSTANTS,line);
	Alert1(Message);
	return(ABORT);
	}

if(j >= maxparam) {
	h = (Handle) p_StringConstant;
	if((h = IncreaseSpace(h)) == NULL) return(ABORT);
	p_StringConstant = (char****) h;
	maxparam = (MyGetHandleSize((Handle)p_StringConstant) / sizeof(char**));
	for(i=j; i < maxparam; i++) (*p_StringConstant)[i] = NULL;
	}
	
if((ptr = (char**) GiveSpace((Size)(strlen(line)+1))) == NULL) return(ABORT);
(*p_StringConstant)[j] = ptr;
MystrcpyStringToTable(p_StringConstant,j,line);
return(j);

ERR:
Alert1("Missing parameter name");
return(ABORT);
}


FixNumberConstant(char* line)
{
int i,j,maxparam;
Handle h;
double x;
long p,q;

Strip(line);
if(line[0] == '\0') goto ERR;

x = Myatof(line,&p,&q);

if(p_NumberConstant == NULL) maxparam = 0;
else maxparam = (MyGetHandleSize((Handle)p_NumberConstant) / sizeof(double));

for(j=ZERO; j < maxparam; j++) {
	if((*p_NumberConstant)[j] == x) break;
	if((*p_NumberConstant)[j] == 0. && j > 0) {
		(*p_NumberConstant)[j] = x;
		break;
		}
	}
if(j < maxparam) return(j);
if(j >= 256) {
	sprintf(Message,
		"Too many numeric constants found (max 256)\nCan't store '%s'\n",line);
	Alert1(Message);
	return(ABORT);
	}

if(maxparam == 0) {
	maxparam = 10;
	if((p_NumberConstant=(double**) GiveSpace((Size)(maxparam) * sizeof(double))) == NULL)
		return(ABORT);
	for(i=0; i < maxparam; i++) (*p_NumberConstant)[i] = 0.;
	j = 1;	/*  (*p_NumberConstant)[0] must be 0. */
	}
else {
	h = (Handle) p_NumberConstant;
	if((h = IncreaseSpace(h)) == NULL) return(ABORT);
	p_NumberConstant = (double**) h;
	}
(*p_NumberConstant)[j] = x;
return(j);

ERR:
Alert1("Missing value");
return(ABORT);
}


WaitABit(long thedelay)
// Wait for thedelay milliseconds
{
int i;
unsigned long endtime;

i = 0;
if(Oms || NEWTIMER) {
#if WITH_REAL_TIME_SCHEDULER
	endtime = TotalTicks + (thedelay / CLOCKRES);
	while(TotalTicks < endtime) {
		if(i++ > 50) {
			PleaseWait(); i = 0;
			}
		}
#endif
	}
else {
	endtime = clock() + ((thedelay * 6L) / 100L);
	while(clock() < endtime) {
		if(i++ > 50) {
			PleaseWait(); i = 0;
			}
		}
	}
return(OK);
}


NeedGlossary(tokenbyte ***pp_X)
{
/* register */ int  i;
tokenbyte m,p;

for(i=0; ((m=(**pp_X)[i]) != TEND) || ((**pp_X)[i+1] != TEND); i+=2) {
	if(m != T4) continue;
	p = (**pp_X)[i+1];
	if(p > Jvar || p_VarStatus == NULL) {
		if(Beta) Alert1("Err. NeedGlossary(). p > Jvar || p_VarStatus == NULL");
		return(NO);
		}
	if((*p_VarStatus)[p] & 4) return(YES);
	}
return(NO);
}

#if 0
pascal void MySoundProc(short sndNum)
/* sndNum will range from 0 to 3 */
{
SndChannelPtr myChan = 0L;
SndListHandle mySound;
OSErr err;

if(sndNum == 0 || AlertMute) return;
mySound = (SndListHandle) GetResource(soundListRsrc,kpopID);
err = SndNewChannel( &myChan, 0, 0, 0L );
HLock((Handle) mySound );
err = SndPlay(myChan,mySound,FALSE);
HUnlock((Handle) mySound);
err = SndDisposeChannel(myChan,FALSE);
}
#endif

// ------------------------  Random numbers -------------------------

#if BP_CARBON_GUI

SetSeed(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char line[MAXFIELDCONTENT];

sprintf(line,"%.0f",(double) Seed);
SetField(NULL, wRandomSequence, fSeed, line);
return(OK);
}


GetSeed(void)
{
char line[MAXFIELDCONTENT];
long p,q,newseed;

GetField(NULL,TRUE,wRandomSequence,fSeed,line,&p,&q);
newseed = p / q;
if(newseed < 0 || newseed > 32767) {
	Alert1("Random seed must be in range [0..32767]");
	ShowWindow(GetDialogWindow(gpDialogs[wRandomSequence]));
	BringToFront(GetDialogWindow(gpDialogs[wRandomSequence]));
	BPUpdateDialog(gpDialogs[wRandomSequence]);
	SelectField(NULL,wRandomSequence,fSeed,TRUE);
	return(FAILED);
	}
Seed = (unsigned int) newseed;
return(OK);
}

#endif /* BP_CARBON_GUI */

ResetRandom(void)
{
srand(Seed);
UsedRandom = FALSE;
AppendScript(55);
return(OK);
}


Randomize(void)
{

ReseedOrShuffle(NEWSEED);
sprintf(Message,"%.0f",(double)Seed);
MystrcpyStringToTable(ScriptLine.arg,0,Message);
AppendScript(57);
return(OK);
}


ReseedOrShuffle(int what)
{
unsigned int seed;
int randomnumber;

switch(what) {
	case NOSEED:
		break;
	case NEWSEED:
	case RANDOMIZE:
		seed = (unsigned int) time(NULL);
		srand(seed);
		// FIXME ? Why seed a second time (with a restricted range for the seed too) ?
		randomnumber = rand();
		seed = (unsigned int) (randomnumber % 32768);
		BPPrintMessage(odInfo, "Random seed = %u\n", seed);
		srand(seed);
		UsedRandom = TRUE;
		if(what == NEWSEED) {
			Seed = seed;
			UsedRandom = FALSE;
#if BP_CARBON_GUI
			SetSeed();
			if(ShowMessages) {
				ShowWindow(GetDialogWindow(gpDialogs[wRandomSequence]));
				BringToFront(GetDialogWindow(gpDialogs[wRandomSequence]));
				BPUpdateDialog(gpDialogs[wRandomSequence]);
				}
#endif /* BP_CARBON_GUI */
			}
		break;
	default:
		seed = (unsigned int) ((Seed + what) % 32768);
		srand(seed);
		UsedRandom = TRUE;
		break;
	}
return(OK);
}


double GetScalingValue(tokenbyte **p_a,unsigned long i)
{
tokenbyte m,p;
double value;

m = (*p_a)[i+3L];
p = (*p_a)[i+5L];
if(m < 0 || p < 0) {
	if(Beta) Alert1("Err. GetScalingValue(). m < 0 || p < 0");
	return(1.);
	}
value = ((double)TOKBASE * m) + p;
return(value);
}
