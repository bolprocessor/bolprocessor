/* Script.c (BP3) */

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

#include "-BP2.h"
#include "-BP2decl.h"

/* int RunScript(int w,int quick)
{
long pos,posline,posmax,posdir;
char **p_line,**p_keyon[MAXCHAN+1];
int i,j,r,changed,gap,dirtymem,wmem,noteconventionmem;
Handle ptr;

if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("=> Err. RunScript(). Not editable window");
	return(MISSED);
	}
while(Button());
r = OK;
p_line = NULL;
noteconventionmem = NoteConvention;
dirtymem = Dirty[iSettings];
for(i=1; i <= MAXCHAN; i++) {
	if((p_keyon[i] = (char**) GiveSpace((Size)(MAXKEY+1)*sizeof(char)))
			== NULL) return(ABORT);
	for(j=0; j < MAXKEY; j++) (*p_keyon[i])[j] = 0;
	}
if(ScriptExecOn) goto QUIT;
StartScript();
if(w == wInteraction || w == wGlossary) goto DOIT;
OutputWindow = wData;
StepScript = FALSE;
ScriptW = 0;
// Created[w] = FALSE;  // suppressed 041207 akozar
if(quick) goto DOIT;
BPActivateWindow(SLOW,wScriptDialog);
BPActivateWindow(SLOW,w);
if(SaveCheck(w) == ABORT) goto QUIT1;
if(FileName[w][0] == '\0') {
	Alert1("=> You should first load a script or save the current one");
	goto QUIT1;
	}

DOIT:
if(w != wInteraction && w != wGlossary && (r=ClearWindow(TRUE,wTrace)) != OK) goto QUIT1;
else SelectBehind(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
TextGetSelection(&posline, &posmax, TEH[w]);
pos = posline;
if(w == wScript) {
	if(ResetScriptQueue() != OK) {
		r = ABORT; goto QUIT1;
		}
	if(AppendStringList(FileName[wScript]) != OK) goto QUIT1;
	if((posmax > pos) && !quick) {
		ShowSelect(CENTRE,w);
		// my_sprintf(Message,"Execute script '%s'",FileName[w]);  // suppressed 041007, akozar
		// if(Answer(Message,'Y') != OK) goto QUIT1;
		if((r=Answer("Execute only the current selection",'Y')) == ABORT) goto QUIT1;
		if(r == YES) {
			for(i=pos; i > 0; i--) {
				if(GetTextChar(wScript,i) == '\r') {
					i++; break;
					}
				}
			for(j=posmax; j < GetTextLength(wScript); j++) {
				if(GetTextChar(wScript,j) == '\r') break;
				}
			if(i != pos || j != posmax) SetSelect(i,j,TEH[wScript]);
			pos = posline = i;
			posmax = j;
			goto START;
			}
		}
	}
pos = posline = ZERO; posmax = GetTextLength(w);

START:
if(Beta && w == wScript) {
	OpenTrace();
	}
r = OK;
if(w != wScript) goto HERE;
my_sprintf(Message,"Running script '%s'\nExecute step by step",FileName[w]);
if(!quick && (r=Answer(Message,'N')) == OK) StepScript = TRUE;
if(r == ABORT) goto QUIT1;
ScriptExecOn = 1; OkWait = OK;
MoreTime = FALSE; Maxitems = ZERO;
HideTicks = FALSE;
CurrentDir = WindowParID[wScript];
CurrentVref = TheVRefNum[wScript];
CurrentChannel = 1;	// Used for program changes, by default.
ForceTextColor = ForceGraphicColor = 0;
WaitEndDate = ZERO;
posdir = -1L;
CompiledGl = FALSE;

HERE:
p_line = NULL;
while(ReadLine(YES,w,&pos,posmax,&p_line,&gap) == OK) {
	PleaseWait();
	if(w == wScript) {
		SetSelect(posline+gap,pos,TEH[w]); Activate(TEH[w]);
		TextUpdate(w);
		ShowSelect(CENTRE,w);
		}
	else SelectBehind(posline+gap,pos,TEH[w]);
	if((*p_line)[0] == '\0') continue;
	if((r=MyButton(0)) != MISSED || StepScript) {
		wmem = Nw;
		BPActivateWindow(SLOW,w);
		if((StepScript || r == OK) && ((r=InterruptScript()) != OK && r != STOP))
			goto END;
		BPActivateWindow(SLOW,wmem);
		if(r == STOP || r == EXIT) {
			mStop(ScriptW);
			if(!ScriptExecOn) break;
			}
		}
	r = OK;
	if(EventState != NO) {
		r = EventState; goto END;
		}
	r = OK; changed = NO;
	if((r=ExecScriptLine(p_keyon,w,FALSE,FALSE,p_line,posline+gap,&posdir,&changed,&i))
			== ABORT) break;
	if(r == EXIT) break;
	if(r == FINISH) {
		EndScript();
		r = OK;
		break;
		}
	if(r == STOP) {
		if((r=InterruptScript()) != OK && r != STOP) goto END;
		if(r == STOP) mStop(ScriptW);
		r = OK;
		}
	if(!ScriptExecOn && w == wScript) break;
	if(r != OK) {
		if(w == wInteraction || w == wGlossary) break;
		}
	posline = pos;
	if(w == wScript) Deactivate(TEH[w]);
	}

END:
MyDisposeHandle((Handle*)&p_line);

QUIT1:
if(w == wScript && r != EXIT) EndScript();

QUIT:
NoteConvention = noteconventionmem;
Dirty[iSettings] = dirtymem;
HideTicks = FALSE;
for(i=1; i <= MAXCHAN; i++) {
	ptr = (Handle) p_keyon[i];
	MyDisposeHandle(&ptr);
	p_keyon[i] = NULL;
	}
if(w == wScript) {
	Maxitems = ZERO; OkWait = OK;
	ForceTextColor = ForceGraphicColor = 0;
	OutputWindow = wData;
	BPActivateWindow(SLOW,w);
	ShowSelect(CENTRE,w);
	Activate(TEH[w]);
	}
return(r);
} */


/* InitWriteScript(void)
{
long i;

CountOn = FALSE;
#if WITH_REAL_TIME_MIDI_FORGET_THIS
  ResetMIDI(TRUE);
#endif
BPActivateWindow(SLOW,wScript);
SwitchOn(NULL,wScriptDialog,bRecordScript);

i = GetTextLength(wScript) - 1L;
while(isspace(GetTextChar(wScript,i))) i--;
SetSelect(i+1,GetTextLength(wScript),TEH[wScript]);
TextDelete(wScript);
if(i > 0) Print(wScript,"\n");
ShowSelect(CENTRE,wScript);
CurrentVref = TheVRefNum[wScript];
CurrentDir = WindowParID[wScript];
return(OK);
} 


EndWriteScript(void)
{
ScriptRecOn = FALSE;
SwitchOff(NULL,wScriptDialog,bRecordScript);
return(OK);
} */


/* AppendScript(int i)
// Write script command i.  If command has arguments these should be provided in global variable Script
{
int j;

StopCount(i);
if(ScriptRecOn) {
	SelectBehind(GetTextLength(wScript),GetTextLength(wScript),TEH[wScript]);
	for(j=0; j < ScriptNrLabel(i); j++) {
		PrintBehind(wScript,*(p_ScriptLabelPart(i,j)));
		if(j < ScriptNrArg(i)) {
			PrintBehind(wScript," ");
			PrintBehind(wScript,*((*(ScriptLine.arg))[j]));
			PrintBehind(wScript," ");
			}
		}
	PrintBehind(wScript,"\n");
	ShowSelect(CENTRE,wScript);
	}
return(OK);
} */


int StartCount(void)
{
if(CountOn) return(OK);
WaitStartDate = clock();
CountOn = TRUE; LapWait = ZERO;
return(OK);
}


int StopCount(int i)
{
if(!CountOn) {
	if(TypeScript && i != 16 && ScriptRecOn) {
		TypeScript = FALSE;
		PrintBehind(wScript,"\n");
		}
	return(OK);
	}
/* my_sprintf(Message,"%ld",(long) clock() - WaitStartDate - LapWait);
MystrcpyStringToTable(ScriptLine.arg,0,Message); */
my_sprintf(Message,"%ld",(long)ItemNumber-1L);
MystrcpyStringToTable(ScriptLine.arg,0,Message);
CountOn = FALSE;
/* AppendScript(18); */
return(OK);
}


/* StartScript(void)
{
EndWriteScript();
HidePannel(wScriptDialog,bExecScript);
HidePannel(wScriptDialog,bLoadScript);
HidePannel(wScriptDialog,bClearScript);
HidePannel(wScriptDialog,bWaitForSyncScript);
HidePannel(wScriptDialog,bRecordScript);
SwitchOn(NULL,wScriptDialog,bExecScript);
return(OK);
} */


int InitWait(long maxticks)
{
if(!ScriptExecOn || CountOn) return(OK);
StartCount(); LapWait = ZERO;
WaitEndDate = WaitStartDate + maxticks;
return(OK);
}


int CheckEndOfWait(void)
{
if(!ScriptExecOn || !CountOn) return(OK);
if(WaitEndDate > ZERO && clock() > WaitEndDate) return(ABORT);
/* clock() is the same as TickCount() */
return(OK);
}


/* int WaitForSyncScript(void)
{
if(!ScriptRecOn) {
	Alert1("'REC' should be on"); return(OK);
	}
if(Answer("Define MIDI synchronisation code or key stroke",'Y') != YES) return(OK);
SwitchOn(NULL,wScriptDialog,bWaitForSyncScript);
// LoadMIDIsyncOrKey();
SwitchOff(NULL,wScriptDialog,bWaitForSyncScript);
return(OK);
} */


/* int SyncWait(void)
{
int r;

HangOn = TRUE;
while((r=MainEvent()) != RESUME && r != EXIT);
HangOn = FALSE;
if(r == EXIT) return(r);
return(OK);
} */


int ExecScriptLine(char*** p_keyon,int w,int check,int nocomment,char **p_line,long posline,
	long* p_posdir,int* p_changed,int* p_keep) {
	// MenuHandle themenu;
	Handle h;
	int i,istart,j,r,quote;
	short itemnumber,iItemCnt;
	Str255 itemstring;
	long menuchoice;
	short menuID;
	char b,c,*p,*q,**p_arg,newarg;

	if(p_line == NULL) {
		if(Beta) Alert1("=> ERROR: ExecScriptLine(). p_line = NULL");
		return(OK);
		}
	if((*p_line)[0] == '\0') return(OK);
//	BPPrintMessage(0,odError,"Match? line = “%s”\n",*p_line);
	istart = 0; p_arg = NULL;
	/* Skip BP style remark */
	if((*p_line)[istart] == '[') {
		while((*p_line)[istart] != ']' && (*p_line)[istart] != '\0') istart++;
		istart++;
		while(MySpace((*p_line)[istart])) istart++;
		}

	/* Suppress AppleScript style remark: Should use Arrow[1] */
	quote = OFF;
	for(i=istart; i < MyHandleLen(p_line)-2; i++) {
		c = (*p_line)[i];
		if(c == '"') quote = 1 - quote;
		if(!quote && c == '-' && (*p_line)[i+1] == '-' && (*p_line)[i+2] != '>') {
			(*p_line)[i] = '\0';
			break;
			}
		}

	/* Remove trailing spaces */
	i = MyHandleLen(p_line) - 1;
	while(i >= istart && MySpace((*p_line)[i])) (*p_line)[i--] = '\0';

	if((*p_line)[istart] == '\0') return(OK);

	if((p_arg = (char**) GiveSpace(MyGetHandleSize((Handle)p_line))) == NULL)
		return(ABORT);
	MystrcpyHandleToHandle(istart,&p_arg,p_line);

	/* Discard glossary line with "-->" */

	MyLock(FALSE,(Handle)p_arg);
	if(w == wGlossary && strstr(*p_arg,Arrow[1]) != NULLSTR) {
		MyUnlock((Handle)p_arg);
		r = OK; goto QUIT;
		}
	MyUnlock((Handle)p_arg);

	/* Now, try to match script line with script command */

	MATCH:
	ScriptExecOn++;
	// BPPrintMessage(0,odError,"MaxScriptInstructions = %d\n",MaxScriptInstructions);
	for(i=0; i < MaxScriptInstructions; i++) {
		MystrcpyHandleToString(MAXLIN,0,Message,p_ScriptLabelPart(i,0));
		j = strlen(Message);
	//	BPPrintMessage(0,odInfo,"Match%d #%d [%d] %s = etc. ? this line = %s\n",check,i,(*h_ScriptIndex)[i],Message,*p_arg);
		if(j == 0) {
			if(Beta) Println(wTrace,"=> Err. ExecScriptLine(): strlen = 0");
			continue;
			}
		q = &(Message[0]);
		if(Match(FALSE,p_arg,&q,j)) {
		//	if(check || 1) {
				if(TraceMIDIinteraction) BPPrintMessage(0,odInfo,"Script command matches #%d [%d] (out of %d) “%s”\n",i,(*h_ScriptIndex)[i],MaxScriptInstructions,Message);
				c = (*p_arg)[j];
				if(c != '\0' && !isspace(c) && !isdigit(c) && !ispunct((*p_arg)[j-1])) continue;
				r = GetScriptArguments(i,p_arg,istart);
				newarg = FALSE;
				if(r == OK) r = DoScript(i,p_keyon,w,check,(*h_ScriptIndex)[i],p_posdir,p_changed,&newarg,FALSE);
				else {
					BPPrintMessage(0,odError,"=> Error GetScriptArguments()\n");
					goto ERR;
					}
		/*		if(newarg && check) {
					// Only for 'Open', 'Run script', etc.
					my_sprintf(Message,"%s \"%s\"",*(p_ScriptLabelPart(i,0)),
														*((*(ScriptLine.arg))[0]));
					MystrcpyStringToHandle(&p_line,Message);
					*p_changed = TRUE; *p_keep = FALSE;
					} */
				/* Reset the sizes of argument handles */
			//	if(TraceMIDIinteraction) BPPrintMessage(0,odError,"Done setting script #%d (out of %ld) [%d] with %d args\n",i,(long)MaxScriptInstructions,(*h_ScriptIndex)[i],ScriptNrArg(i));
				if(check) {
					for(j=0; j < ScriptNrArg(i); j++) {
						h = (Handle) (*(ScriptLine.arg))[j];
						if((r = MySetHandleSize((Handle*)&h,(Size) MAXLIN * sizeof(char))) != OK) goto ERR;
						(*(ScriptLine.arg))[j] = h;
						}
					}
			//	if(TraceMIDIinteraction) BPPrintMessage(0,odError,"Done setting script #%d [%d], r = %d\n",i,(*h_ScriptIndex)[i],(int)r);
				if(r != OK) goto ERR;
		//		}
		//	else r = OK;
			goto QUIT;	
			}
		}
	r = MISSED;

	ERR:
	my_sprintf(Message,">>> Script aborted on: %s\n",*p_line);
	Print(wTrace,Message);
	if(r != EXIT) r = ABORT;
		
	QUIT:
	MyDisposeHandle((Handle*)&p_arg);
	EndScript();
	return(r);
	}


int InterruptScript(void)
{
int r;

Interrupted = TRUE;
ResumeStopOn = TRUE;
/* ShowWindow(GetDialogWindow(ResumeStopPtr));
BringToFront(GetDialogWindow(ResumeStopPtr)); */
// while((r = MainEvent()) != RESUME && r != STOP && r != ABORT && r != EXIT);
if(r == RESUME) {
	r = OK; EventState = NO;
	}
return(r);
}


/* ScriptKeyStroke(int w,EventRecord *p_event)
{
char thechar;

if(ScriptRecOn && ((p_event->modifiers & cmdKey) == 0)) {
	if(w < 0 || w >= WMAX || !Editable[w] || w == wScript) return(MISSED);
	thechar = ((char)(p_event->message & charCodeMask));
	if(thechar == '\b') {	// Delete key
		if(TypeScript) {
			TypeScript = FALSE;
			PrintBehind(wScript,"\n");
			}
		else {
			RecordEditWindow(w);
			}
		MystrcpyStringToTable(ScriptLine.arg,0,"1");
		// AppendScript(19);
		}
	else {
		if(thechar == '\r') {
			if(TypeScript) PrintBehind(wScript,"\n");
			else RecordEditWindow(w);
			TypeScript = FALSE;
			MystrcpyStringToTable(ScriptLine.arg,0,"<return>");
			// if(Nw != wScript) AppendScript(16);
			return(OK);
			}
		if(!TypeScript) {
			RecordEditWindow(w);
			PrintBehind(wScript,*(p_ScriptLabelPart(16,0)));
			PrintBehind(wScript," ");
			TypeScript = TRUE;
			}
		Message[0] = thechar;
		Message[1] = '\0';
		PrintBehind(wScript,Message);
		}
	}
return(OK);
} */


/* RecordEditWindow(int w)
{
TextOffset selbegin, selend;

if(!ScriptRecOn) return(OK);
if(w < 0 || w >= WMAX || !Editable[w] || w == wScript) {
	return(MISSED);
	}
my_sprintf(Message,"\"%s\"",WindowName[w]);
MystrcpyStringToTable(ScriptLine.arg,0,Message);
AppendScript(42);
TextGetSelection(&selbegin, &selend, TEH[w]);
my_sprintf(Message,"%ld",(long)selbegin);
MystrcpyStringToTable(ScriptLine.arg,0,Message);
AppendScript(59);
my_sprintf(Message,"%ld",(long)selend);
MystrcpyStringToTable(ScriptLine.arg,0,Message);
AppendScript(60);
ShowSelect(CENTRE,wScript);
return(OK);
} */


/* Returns an error code if anything goes wrong other than not
   finding or executing the script.  If the script was found
   and successfully executed, then returns TRUE in scriptCompleted.
   filename may be a relative pathname up to 255 chars long. */

/* int RunScriptInPrefsOrAppFolder(StringPtr filename, int* scriptCompleted)
{
	OSErr  err;
	FSSpec scriptloc;
	char   cname[64];		// size of FSSpec.name
	int    rep, changed;
	short  oldvrefnum;
	long   oldparid;

	*scriptCompleted = FALSE;
	rep = OK;
	
	// Look first in the "Bol Processor" subfolder of the user (or system on OS 9)
	   preferences folder.  If not found there, try the BP2 application folder
	err = FindFileInPrefsFolder(&scriptloc, filename);
	if (err != noErr) {
		err = FSMakeFSSpec(RefNumbp2, ParIDbp2, filename, &scriptloc);
	}
	if (err == noErr) {
		// above func calls may have modified the name, so use scriptloc.name
		p2cstrcpy(cname, scriptloc.name);
		if ((rep = AppendStringList(cname)) == OK) {
			oldparid = WindowParID[wScript];
			oldvrefnum = TheVRefNum[wScript];
			
			CurrentDir = WindowParID[wScript] = scriptloc.parID;
			CurrentVref = TheVRefNum[wScript] = scriptloc.vRefNum;
					
			if (RunScriptOnDisk(FALSE, cname, &changed) == OK) *scriptCompleted = TRUE;
			
			WindowParID[wScript] = oldparid;
			TheVRefNum[wScript] = oldvrefnum;
			
			rep = ResetScriptQueue();
		}
	}
	
	return rep;
} */


/* int RunScriptOnDisk(int check,char* filename,int *p_changed)
{
int i,iv,io,r,rr,result,type,startup,shutdown,good,changed,keep,dirtymem,wmem;
FSSpec spec;
short refnum;
long pos,posdir,posline,count;
char **p_line,**p_completeline;


result = OK; startup = shutdown = FALSE;
p_line = p_completeline = NULL;
// Strip(filename);
if(!MemberStringList(filename)) AppendStringList(filename);
if(strcmp(filename,"+sc.startup") == 0) startup = TRUE;
if(strcmp(filename,"+sc.shutdown") == 0) shutdown = TRUE;
strcpy(LineBuff, filename);
type = gFileType[wScript];
c2pstrcpy(spec.name, filename);
spec.vRefNum = TheVRefNum[wScript];
spec.parID = WindowParID[wScript];
if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
	spec.vRefNum = TheVRefNum[wScript] = CurrentVref;
	spec.parID = WindowParID[wScript] = CurrentDir;
	if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
		if(startup || shutdown) return(MISSED);
		if(CheckFileName(wScript,LineBuff,&spec,&refnum,type,TRUE) != OK) return(ABORT);
		strcpy(filename,LineBuff);
		}
	TheVRefNum[wScript] = spec.vRefNum;
	WindowParID[wScript] = spec.parID;
	}
// HideWindow(Window[wMessage]);
if(ScriptExecOn == 0) {
	dirtymem = Dirty[iSettings];
	Maxitems = ZERO; OkWait = OK;
	}
if(check && ClearWindow(FALSE,wTrace) != OK) {
	my_sprintf(Message,"\n\nInterrupted script '%s'\n\n",filename);
	Print(wTrace,Message);
	return(ABORT);
	}
 ScriptExecOn++;
WaitEndDate = ZERO; good = NO; changed = NO;
PleaseWait();
pos = posline = ZERO;
MoreTime = FALSE; posdir = -1L;
CurrentDir = spec.parID;
CurrentVref = spec.vRefNum;
if(ReadOne(TRUE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto END;
if(CheckVersion(&iv,p_line,filename) != OK) goto END;
if(ReadOne(TRUE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto END;
if(!check) my_sprintf(Message,"Running script '%s'",filename);
else my_sprintf(Message,"Checking script '%s'",filename);
ShowMessage(TRUE,wMessage,Message);
good = YES;
StartScript();
while((rr=ReadOne(TRUE,FALSE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos)) != MISSED) {
	PleaseWait();
	if(!check && StepScript) {
		MyLock(FALSE,(Handle)p_line);
		FlashInfo(*p_line);
		MyUnlock((Handle)p_line);
		my_sprintf(Message,"Running script '%s'",filename);
		ShowMessage(TRUE,wMessage,Message);
		}
	if((r=MyButton(0)) != MISSED || (!check && StepScript)) {
		wmem = Nw;
		if(!check && StepScript) r = OK;
		if(r != STOP && ((r != OK) || ((r=InterruptScript()) != OK && r != STOP))) {
			result = r; break;
			}
		BPActivateWindow(SLOW,wmem);
		if(r == STOP) {
			mStop(ScriptW);
			if(ScriptExecOn == 0) break;
			}
		}
	if(EventState != NO) {
		result = EventState; break;
		}
	r = OK;
	keep = TRUE;
	if((r=ExecScriptLine(NULL,wScript,check,FALSE,p_line,posline,&posdir,&changed,&keep))
			== ABORT) {
		my_sprintf(Message,"\nInterrupted script '%s'\n\n",filename);
		Print(wTrace,Message);
		good = NO;
		break;
		}
	if(!check && r == EXIT) {
		result = EXIT; good = YES;
		break;
		}
	if(ScriptExecOn == 0) {
		result = ABORT;
		break;
		}
	if(r != OK) {
		good = NO;
		if(!check) {
			MyLock(FALSE,(Handle)p_line);
			my_sprintf(Message,">>> Error in script '%s': %s\n",filename,*p_line);
			MyUnlock((Handle)p_line);
			Print(wTrace,Message);
			ShowSelect(CENTRE,wScript);
			result = ABORT;
			break;
			}
		}
	if(r == OK && check) {
		MyLock(FALSE,(Handle)p_line);
		MyLock(FALSE,(Handle)p_completeline);
		if(!keep) PrintBehindln(wTrace,*p_line);
		else PrintBehindln(wTrace,*p_completeline);
		MyUnlock((Handle)p_line);
		MyUnlock((Handle)p_completeline);
		}
	posline = pos;
	if(rr == STOP) break;
	}
// HideWindow(Window[wInfo]);

END:
if(check && good && changed) {
	UpdateThisWindow(FALSE,Window[wTrace]);
	ShowSelect(CENTRE,wTrace);
	SetFPos(refnum,fsFromStart,ZERO);
	WriteHeader(wScript,refnum,spec);
	WriteFile(TRUE,MAC,refnum,wTrace,GetTextLength(wTrace));
	WriteEnd(wScript,refnum);
	GetFPos(refnum,&count);
	SetEOF(refnum,count);
	FlushFile(refnum);
	result = OK;
	}
if(check && !good ) {
	ClearWindow(FALSE,wScript);
	strcpy(FileName[wScript],filename);
	SetFPos(refnum,fsFromStart,ZERO);
	if(ReadFile(wScript,refnum)) {
		GetHeader(wScript);
		Created[wScript] = TRUE;
		SetName(wScript,TRUE,TRUE);
		}
	else {
		my_sprintf(Message,"Can't read '%s'...",FileName[wScript]);
		Alert1(Message);
		}
	ShowSelect(CENTRE,wTrace);
	changed = FALSE;
	result = ABORT;
	}
if(FSClose(refnum) != noErr) {
	my_sprintf(Message,"=> Error closing '%s' script file...",filename);
	Alert1(Message);
	}
EndScript();
if(check && good) {
	Dirty[wTrace] = FALSE;
	ClearWindow(FALSE,wTrace);
	ShowMessage(TRUE,wMessage,"=> Errors: 0");
	}
MyDisposeHandle((Handle*)&p_line);
MyDisposeHandle((Handle*)&p_completeline);
if(!check || !good) // HideWindow(Window[wMessage]);
if(ScriptExecOn == 0) {
	Dirty[iSettings] = dirtymem;
	OutputWindow = wData;
	Maxitems = ZERO; OkWait = OK;
	}
if(changed) *p_changed = TRUE;
return(result);
}
*/