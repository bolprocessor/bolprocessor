/* MenuCommands.c (BP2 version CVS) */

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
#include "CarbonCompatUtil.h"

mLoadTimePattern(int wind)
{
int rep;

if(!OutMIDI) {
	Alert1("Cannot load time pattern because MIDI output is inactive");
	return(FAILED);
	}
#if WITH_REAL_TIME_MIDI
if(CheckEmergency() != OK) return(FAILED);
rep = NO;
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
if((LastEditWindow == wGrammar
		&& (rep=Answer("Insert pattern\nin current grammar",'Y')) == OK)
	|| (LastEditWindow == wAlphabet
		&& (rep=Answer("Insert pattern\nin current alphabet",'Y')) == OK)) {
	LoadTimePattern(LastEditWindow);
	}
if(rep == ABORT) return(OK);
if(rep == NO) Alert1("First select grammar or alphabet window");
return(OK);
#endif
}


mGetInfo(int wind)
{
int gap;
long pos;

if(wind < 0 || wind >= WMAX) return(OK);
if(Editable[wind] && !LockedWindow[wind]) {
	ShowLengthType(wind);
	}
if((*p_FileInfo[wind])[0] == '\0') return(OK);
MystrcpyHandleToString(MAXINFOLENGTH,0,Message,p_FileInfo[wind]);
FlashInfo(Message);
return(OK);
}


mFAQ(int wind)
{
ShowWindow(GetDialogWindow(FAQPtr));
SelectWindow(GetDialogWindow(FAQPtr));
Help = TRUE;
return(OK);
}


mShowMessages(int wind)
{
int j;

ClearWindow(NO,wNotice);
Print(wNotice,"LAST MESSAGES:\n");
if(Jmessage < MAXMESSAGE - 1) {
	for(j=Jmessage+1; j < MAXMESSAGE; j++) {
		if((*p_MessageMem[j])[0] != '\0') {
			PrintHandleln(wNotice,p_MessageMem[j]);
			}
		}
	}
for(j=0; j <= Jmessage; j++) {
	PrintHandleln(wNotice,p_MessageMem[j]);
	}
Print(wNotice,"\n");
/* SetSelect(ZERO,ZERO,TEH[wNotice]); */
ShowSelect(CENTRE,wNotice);
ShowWindow(Window[wNotice]);
BringToFront(Window[wNotice]);
ShowWindow(Window[wInfo]);
BringToFront(Window[wInfo]);
ShowWindow(Window[wMessage]);
BringToFront(Window[wMessage]);
return(OK);
}


/* About box strings */

/* Item #2 displays the version and compilation date like this:
       Bol Processor Carbon 2.9.6 beta (debug)
       ¥ Jun  1 2007 ¥
 */
static const char Carbon_Text[] = "Carbon ";
static const char Beta_Text[]   = " (debug)";  // "debug" since SHORT_VERSION may contain "beta"

static const unsigned char WASTE_Notice[]  = "\pWASTE text engine © 1993-1996 ¥ Marco Piovanelli";
static const unsigned char TE_Notice[]     = "\pUsing TextEdit for editing.";
static const unsigned char MLTE_Notice[]   = "\pUsing Multi-Lingual Text Engine for editing.";

static const unsigned char MWERKS_Notice[] = "\pPortions © Metrowerks, Corp.";

mAbout(int wind)
{
int r;
Str255 versionstr = "\p";
Str255 datestr = "\p";
ConstStr255Param textenginestr;
ConstStr255Param compilerstr;

// set variable strings in the About box indicating compile-time features
Message[0] = '\0';
if (strlen(Carbon_Text) + strlen(SHORT_VERSION) + strlen(Beta_Text) > MAXLIN) {
	if (Beta) Alert1("Err. mAbout(): version string too long for Message.");
	}
else {
#if TARGET_API_MAC_CARBON
	strcat(Message, Carbon_Text);
#endif
	strcat(Message, SHORT_VERSION);
	if (Beta) strcat(Message, Beta_Text);
	}
c2pstrcpy(versionstr, Message);

c2pstrcpy(datestr, __DATE__);

#if WASTE
  textenginestr = WASTE_Notice;
#elif USE_MLTE
  textenginestr = MLTE_Notice;
#else
  textenginestr = TE_Notice;
#endif

#ifdef __MWERKS__
  compilerstr = MWERKS_Notice;
#else
  compilerstr = "\p";
#endif

SetDefaultCursor();
ParamText(versionstr, datestr, textenginestr, compilerstr);
switch(Alert(AboutAlert,0L)) {
	case dAboutOK:
		r = OK;
		break;
	case dAboutAbort:
		if(wind < 0) r = RESUME;
		else r = OK;
		break;
	case dCredits:
		if (DisplayFile(wNotice,"Credits") != OK)
			DisplayFile(wNotice,"Credits.txt");
		if(wind < 0) r = RESUME;
		else r = OK;
		break;
	case bRegister:
		DisplayFile(wNotice,"License.txt");
	//	DisplayHelp("How do I register for BP2?");
	//	Register();
		if(wind < 0) r = RESUME;
		else r = OK;
		break;
	}
return(r);
}


m9pt(int wind)
{
return(SetFontSize(wind,9));
}


m10pt(int wind)
{
return(SetFontSize(wind,10));
}


m12pt(int wind)
{
return(SetFontSize(wind,12));
}


m14pt(int wind)
{
return(SetFontSize(wind,14));
}


mChangeColor(int wind)
{
ChangeColor();
return(OK);
}


mUseGraphicsColor(int wind)
{
UseGraphicsColor = 1 - UseGraphicsColor;
return(OK);
}


mUseTextColor(int wind)
{
UseTextColor = 1 - UseTextColor;
return(OK);
}


mMIDIorchestra(int wind)
{
ReadKeyBoardOn = FALSE; Jcontrol = -1;

if(mMIDIoutputcheck(wind) != OK) return(FAILED);

ShowWindow(GetDialogWindow(MIDIprogramPtr));
SelectWindow(GetDialogWindow(MIDIprogramPtr));
BPUpdateDialog(MIDIprogramPtr); /* Needed to make static text visible */

BPActivateWindow(SLOW,wMIDIorchestra);
return(OK);
}


mMIDIinputcheck(int wind)
{
TextOffset selbegin, selend;

if(!OutMIDI) {
	Alert1("MIDI input is inactive (check the 'Devices' menu)");
	return(FAILED);
	}

#if WITH_REAL_TIME_MIDI
if(Oms) FlashInfo("OMS MIDI driver is being used");
else if (InBuiltDriverOn) FlashInfo("OMS is inactive. In-built MIDI driver is being used");
// else FlashInfo("Null MIDI driver is being used");

#if BP_MACHO
  FlashInfo("CoreMIDI driver is being used");
  mShowCMSettings(wind);
#endif

Alert1("Notes played on external MIDI device will be shown in the 'Data' window. Otherwise select the proper input...");
mTypeNote(wData);
TextGetSelection(&selbegin, &selend, TEH[wData]);
SetSelect(selend, selend,TEH[wData]);
ShowSelect(CENTRE,wData);
#endif /* WITH_REAL_TIME_MIDI */

return(OK);
}


mMIDIoutputcheck(int wind)
{
if(!OutMIDI) {
	Alert1("MIDI output is inactive (check the 'Devices' menu)");
	return(FAILED);
	}

#if WITH_REAL_TIME_MIDI
if(Oms) FlashInfo("OMS MIDI driver is being used");
else if (InBuiltDriverOn) FlashInfo("OMS is inactive. In-built MIDI driver is being used");
// else FlashInfo("Null MIDI driver is being used");

ReadKeyBoardOn = FALSE; Jcontrol = -1;

HideWindow(Window[wMessage]);

ShowWindow(GetDialogWindow(SixteenPtr));
SelectWindow(GetDialogWindow(SixteenPtr));
BPUpdateDialog(SixteenPtr); /* Needed to make static text visible */
#endif

return(OK);
}


mExpandSelection(int wind)
{
if(SoundOn) return(FAILED);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
wind = LastEditWindow;
HideWindow(Window[wMessage]);
sprintf(Message,"\"%s\"",WindowName[wind]);
MystrcpyStringToTable(ScriptLine.arg,0,Message);
AppendScript(0);
return(ExpandSelection(wind));
}


mCaptureSelection(int wind)
{
int w;

if(SoundOn) return(FAILED);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
if(Answer("Capture text selection as a MIDI stream to paste it later to a sound-object prototype",'Y')
	!= YES) return(OK);
if(wind >= 0 && wind < WMAX && Editable[wind]) LastEditWindow = wind;
w = LastEditWindow;
switch(w) {
	case wData:
	case wAlphabet:
	case wGrammar:
	case wScript:
	case wScrap:
	case wTrace:
	case wStartString:
	case wGlossary:
		break;
	default:
		Alert1("You must select the item in the Data, Grammar, Alphabet, Script, Trace, Scrap or Glossary window");
		return(FAILED);
	}
return(TextToMIDIstream(w));
}


mShowPeriods(int wind)
{
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
wind = LastEditWindow;
if(wind != wData) {
	Alert1("Selection may be spaced only in data window");
	return(OK);
	}
return(ShowPeriods(wind));
}


mExecuteScript(int wind)
{
int rep;

if(SoundOn || ScriptExecOn) return(ABORT);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
EndWriteScript();
HideWindow(Window[wMessage]);
rep = RunScript(wScript,FALSE);
return(rep);
}


mTransliterateFile(int wind)
{
int rep;

if((ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn || CompileOn || GraphicOn
	|| PolyOn || ScriptExecOn)) return(FAILED);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
GetValues(TRUE);
if(IsEmpty(wGrammar)) {
	Alert1("Can't transliterate file because grammar is empty");
	return(FAILED);	
	}
if(CompileCheck() != OK) return(FAILED);
rep = TransliterateFile();
return(rep);
}


mCheckDeterminism(int wind)
{
int rep;

if((ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn || CompileOn || GraphicOn
	|| PolyOn || ScriptExecOn)) return(FAILED);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
if(IsEmpty(wGrammar)) {
	Alert1("Grammar is empty");
	return(FAILED);	
	}
if(CompileCheck() != OK) return(FAILED);
rep = CheckDeterminism(&Gram);
if(rep == OK) Alert1("All subgrammars in this project are deterministic");
return(rep);
}


mFrenchConvention(int wind)
{
if(NoteConvention != FRENCH) {
	NoteConvention = FRENCH;
	CompiledAl = CompiledGr = FALSE;
	AppendScript(1);
	Dirty[iSettings] = TRUE;
	UpdateInteraction();
	}
ShowMIDIkeyboard();
return(OK);
}


mEnglishConvention(int wind)
{
if(NoteConvention != ENGLISH) {
	NoteConvention = ENGLISH;
	CompiledAl = CompiledGr = FALSE;
	AppendScript(2);
	Dirty[iSettings] = TRUE;
	UpdateInteraction();
	}
ShowMIDIkeyboard();
return(OK);
}


mIndianConvention(int wind)
{
if(NoteConvention != INDIAN) {
	NoteConvention = INDIAN;
	CompiledAl = CompiledGr = FALSE;
	AppendScript(88);
	Dirty[iSettings] = TRUE;
	UpdateInteraction();
	}
ShowMIDIkeyboard();
return(OK);
}


mKeyConvention(int wind)
{
if(NoteConvention != KEYS) {
	NoteConvention = KEYS;
	CompiledAl = CompiledGr = FALSE;
	AppendScript(3);
	Dirty[iSettings] = TRUE;
	UpdateInteraction();
	}
HideWindow(GetDialogWindow(MIDIkeyboardPtr));
return(OK);
}


mAzerty(int wind)
{
if(KeyboardType != AZERTY) {
	KeyboardType = AZERTY;
	UpdateDirty(TRUE,wKeyboard);
	SetKeyboard();
	}
return(OK);
}


mQwerty(int wind)
{
if(KeyboardType != QWERTY) {
	KeyboardType = QWERTY;
	UpdateDirty(TRUE,wKeyboard);
	SetKeyboard();
	}
return(OK);
}


mUseBullet(int wind)
{
UseBullet = 1 - UseBullet;
if(UseBullet) Code[7] = '¥';
else Code[7] = '.';
MaintainMenus();
return(OK);
}


mPianoRoll(int wind)
{
ShowPianoRoll = 1 - ShowPianoRoll;
ToldAboutPianoRoll = TRUE;
MaintainMenus();
return(OK);
}


mSplitTimeObjects(int wind)
{
SplitTimeObjects = 1 - SplitTimeObjects;
Dirty[iSettings] = TRUE;
MaintainMenus();
return(OK);
}


mSplitVariables(int wind)
{
SplitVariables = 1 - SplitVariables;
Dirty[iSettings] = TRUE;
MaintainMenus();
return(OK);
}


mText(int wind)
{
Token = 1 - Token;
Dirty[iSettings] = TRUE;
MaintainMenus();
return(OK);
}


mMIDI(int wind)
{
#if !WITH_REAL_TIME_MIDI
	Alert1("Real-time MIDI output is not available in this version of Bol Processor.");
#else
	OutMIDI = 1 - OutMIDI;
	if(OutMIDI) {
		AppendScript(20);
		ResetMIDI(FALSE);
		}
	else AppendScript(21);
	Dirty[iSettings] = TRUE;
	ReadKeyBoardOn = FALSE; Jcontrol = -1;
	HideWindow(Window[wMessage]);
	SetButtons(TRUE);
#endif
return(OK);
}


mCsound(int wind)
{
OutCsound = 1 - OutCsound;
if(OutCsound) {
	AppendScript(22);
	}
else {
	AppendScript(23);
	CloseCsScore();
	}
Dirty[iSettings] = TRUE;
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
SetButtons(TRUE);
return(OK);
}


mOMS(int wind)
{
	Alert1("The OMS driver is not available in this version of Bol Processor.");
	Oms = FALSE;
	return OK;
}


mMIDIfile(int wind)
{
WriteMIDIfile = 1 - WriteMIDIfile;
if(WriteMIDIfile) {
	AppendScript(184);
	}
else {
	CloseMIDIFile();
	AppendScript(185);
	}
Dirty[iSettings] = TRUE;
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
SetButtons(TRUE);
return(OK);
}


mCsoundInstrumentsSpecs(int wind)
{
BPActivateWindow(SLOW,wCsoundInstruments);
return(OK);
}


int mShowCMSettings(int w)
{
#if BP_MACHO
	DrawDialog(CMSettings);
	ShowWindow(GetDialogWindow(CMSettings));
	SelectWindow(GetDialogWindow(CMSettings));
#endif	
	return(OK);
}


int mOpenAudioMidiSetup(int w)
{
#if BP_MACHO
	// launch the Apple-supplied OS X "Audio Midi Setup" application
	StopWait();
	LaunchOSXApplication('AMDS');
#endif	
	return(OK);
}


mToken(int wind)
{
Token = 1 - Token;
Dirty[iSettings] = TRUE;
MaintainMenus();
return(OK);
}


mSmartCursor(int wind)
{
Alert1("'Smart cursor', a wonderful idea, will soon be implemented...");
return(OK);	/* $$$ */
SmartCursor = 1 - SmartCursor;
Dirty[iSettings] = TRUE;
if(SmartCursor) {
	if(CompiledAl && CompiledGr) {
		KillSubTree(PrefixTree); KillSubTree(SuffixTree);
		UpdateAutomata();
		}
	Alert1("'Smart cursor' is ON");
	}
return(OK);
}


mTypeNote(int wind)
{
if(!ReadKeyBoardOn) {
	if(!OutMIDI) {
		Alert1("Cannot type from MIDI because MIDI input/output is inactive");
		return(FAILED);
		}
#if WITH_REAL_TIME_MIDI
	if(!NoteOnIn) {
		Alert1("Can't type from MIDI because NoteOn's are not received. Check MIDI filter");
		mMIDIfilter(wind);
		return(FAILED);
		}
	FoundNote = FALSE; TickDone = FALSE;
	EmptyBeat = TRUE;
	ShowMessage(TRUE,wMessage,
		"Entering data from MIDI keyboard. Type cmd-J to return to text mode...");
	ResetMIDI(FALSE);
//	#ifndef __POWERPC
	FlushEvents(driverEvt,0);
//	#endif
	GetControlParameters();
	if(TransposeInput && LastEditWindow != wScript && TransposeValue != 0) {
		PrintHandle(LastEditWindow,(*p_PerformanceControl)[33]);
		sprintf(Message,"(%ld)",(long)-TransposeValue);
		Print(LastEditWindow,Message);
		EmptyBeat = FALSE;
		}
	ReadKeyBoardOn = TRUE;
#endif
	}
else {
	ReadKeyBoardOn = FALSE;
	HideWindow(Window[wMessage]);
	}
Jcontrol = -1;
BPActivateWindow(SLOW,wind);
return(OK);
}


mTuning(int wind)
{
ShowWindow(GetDialogWindow(TuningPtr));
SelectWindow(GetDialogWindow(TuningPtr));
SetTuning();
BPUpdateDialog(TuningPtr);
return(OK);
}


mDefaultPerformanceValues(int wind)
{
ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
SetDefaultPerformanceValues();
BPUpdateDialog(DefaultPerformanceValuesPtr);
return(OK);
}


mFileSavePreferences(int wind)
{
ShowWindow(GetDialogWindow(FileSavePreferencesPtr));
SelectWindow(GetDialogWindow(FileSavePreferencesPtr));
SetFileSavePreferences();
BPUpdateDialog(FileSavePreferencesPtr);
return(OK);
}


mDefaultStrikeMode(int wind)
{
ShowWindow(GetDialogWindow(StrikeModePtr));
SelectWindow(GetDialogWindow(StrikeModePtr));
SetDefaultStrikeMode();
BPUpdateDialog(StrikeModePtr);
return(OK);
}


mNewProject(int wind)
{
int rep;

if(CheckEmergency() != OK) return(FAILED);
if((rep=CheckSettings()) == ABORT) return(rep);
if(ResetProject(TRUE) != OK) return(FAILED);
if(!ScriptExecOn) {
	ShowWindow(Window[wAlphabet]);
	BPActivateWindow(SLOW,wGrammar);
	AppendScript(4);
	}
return(OK);
}


mLoadProject(int wind)
{
int w,rep;
FSSpec spec;
short refnum;
char *p,*q;
Str255 fn;
OSErr io;

if(CheckEmergency() != OK) return(FAILED);
if(ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn
	|| CompileOn || GraphicOn || PolyOn || LoadOn) return(FAILED);
if((rep=CheckSettings()) == ABORT) return(rep);
if(ResetProject(FALSE) != OK) return(ABORT);
HideWindow(Window[wInfo]);
ShowMessage(TRUE,wMessage,"Find grammar: -gr.<name>");

TRYLOAD:
if(OldFile(wGrammar,5,fn,&spec)) {
	if(IdentifyBPFileType(&spec) != wGrammar) {
		MyPtoCstr(MAXNAME,fn,LineBuff);
		sprintf(Message,"BP2 is not sure that '%s' is a grammar file. Do you want to load it anyway", LineBuff);
		rep = Answer(Message,'N');
		if(rep != YES) {
			if(rep == NO) ShowMessage(TRUE,wMessage,"Hint: You can use the 'Scrap' window to load any file...");
			goto TRYLOAD;
			}
		}
	if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {	/* Read grammar */
		if(LoadGrammar(&spec,refnum) != OK) goto ERR;
		sprintf(Message,"\"%s\"",FileName[wGrammar]);
		MystrcpyStringToTable(ScriptLine.arg,0,Message);
		AppendScript(5);
		if(LoadAlphabet(wGrammar,&spec) != OK) goto ERR;
		for(w=0; w < WMAX; w++) {
			if(w != wScript && Editable[w]) SetSelect(ZERO,ZERO,TEH[w]);
			}
		}
	else {
		MyPtoCstr(MAXNAME,fn,LineBuff);
		sprintf(Message,"Error opening %s",LineBuff);
		ShowMessage(TRUE,wMessage,Message);
		TellError(7,io);
		}
	}
else {
	HideWindow(Window[wMessage]);
	return(FAILED);
	}
HideWindow(Window[wMessage]);
Dirty[wData] = FALSE;
if(!ScriptExecOn) {
	BPActivateWindow(QUICK,wStartString);
	BPActivateWindow(QUICK,wGrammar);
	}
return(OK);

ERR:
return(FAILED);
}


mMakeGrammarFromTable(int wind)
{
int append;

if(CheckEmergency() != OK) return(FAILED);
if(ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn
	|| CompileOn || GraphicOn || PolyOn || LoadOn) return(FAILED);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
HideWindow(Window[wInfo]);
if(SaveCheck(wGrammar) == ABORT) return(ABORT);
if(SaveCheck(wAlphabet) == ABORT) return(ABORT);
if(ClearWindow(NO,wTrace) == ABORT) return(FAILED);
append = NO;
if(!IsEmpty(wGrammar)) {
	if((append=Answer("Append to existing grammar",'Y')) == ABORT) return(ABORT);
	if(append != YES) {
		if(ClearWindow(NO,wGrammar) != OK) return(FAILED);
		RemoveFirstLine(wData,FilePrefix[wGrammar]);
		ForgetFileName(wGrammar);
		}
	}
else ClearWindow(NO,wGrammar);
return(MakeGrammarFromTable(append));
}


mReceiveMIDI(int wind)
{
int r;
long count,i,im;
short refnum;
Str255 fn;
NSWReply reply;
Handle ptr;
OSErr err;

if(CheckEmergency() != OK) return(FAILED);
if(ComputeOn || PolyOn || SoundOn || SelectOn ||
	SetTimeOn || GraphicOn || PrintOn || ReadKeyBoardOn || HangOn || ScriptExecOn)
	return(FAILED);
r = OK;
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
err = NSWInitReply(&reply);
if(Answer("Load and save MIDI data from device",'Y') == YES) {
	if((r=LoadRawData(&im)) != OK) {
		r = FAILED; goto OUT;
		}
	if(im < ZERO) {
		Alert1("MIDI receive error: perhaps your machine is too slow for bulk transfer");
		r = FAILED; goto OUT;
		}
	sprintf(Message,"%ld bytes received. Writing them to text file.",(long)im);
	ShowMessage(TRUE,wMessage,Message);
	c2pstrcpy(fn, "Dump");
	if(NewFile(-1,1,fn,&reply)) {
		if((r=CreateFile(-1,-1,1,fn,&reply,&refnum)) != OK) goto OUT;
		WriteToFile(NO,MAC,"BP2 decimal MIDI dump",refnum);
		sprintf(LineBuff,"%ld",(long)im);
		WriteToFile(NO,MAC,LineBuff,refnum);
		for(i=ZERO; i < im; i++) {
			PleaseWait();
			sprintf(Message,"%ld\n",(long)ByteToInt((*p_Code)[i].byte));
			count = (long) strlen(Message);
			FSWrite(refnum,&count,Message);
			}
		WriteEnd(-1,refnum);
		GetFPos(refnum,&count);
		SetEOF(refnum,count);
		FlushFile(refnum);
		FSClose(refnum);
		reply.saveCompleted = true;
		}
			
OUT:	
	err = NSWCleanupReply(&reply);
	ptr = (Handle) p_Code;
	MyDisposeHandle(&ptr);
	p_Code = NULL;
	}
HideWindow(Window[wMessage]);
return(r);
}


mSendMIDI(int wind)
// $$$ needs to be revised
{
int r,k,sysex,send,nbytes,maxbytes;
long pos,im,i,timeleft,formertime,tbytes2;
short refnum;
FSSpec spec;
MIDI_Event e;
// long count = 12L;
// MIDI_Parameters parms;
Handle ptr;
char **p_line,**p_completeline;
unsigned long drivertime;

if(CheckEmergency() != OK) return(FAILED);
if(!IsMidiDriverOn()) {
	if(Beta) Alert1("Err. mSendMIDI(). Driver is OFF");
	return(ABORT);
	}

#if WITH_REAL_TIME_MIDI
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
p_line = p_completeline = NULL;
if(OldFile(-1,1,PascalLine,&spec)) {
	if(MyOpen(&spec,fsCurPerm,&refnum) == noErr) {
		pos = ZERO;
		if(ReadOne(FALSE,FALSE,TRUE,refnum,FALSE,&p_line,&p_completeline,&pos) == FAILED) goto ERR1;
		if(Mystrcmp(p_line,"BP2 decimal MIDI dump") != 0) {
			Alert1("This file does not contain appropriate (decimal) code");
			goto ERR1;
			}
		if(ReadLong(refnum,&im,&pos) == FAILED) goto ERR1;
		if((p_Code = (MIDIcode**) GiveSpace((Size)im * sizeof(MIDIcode))) == NULL)
			return(FAILED);
		sysex = Answer("Is it system exclusive data",'Y');
		if(sysex == ABORT) goto ERR2;
		Alert1("Set MIDI device ready to 'receive' mode and click 'OK'");
		sprintf(Message,"Reading %ld bytes from file...",(long)im);
		ShowMessage(TRUE,wMessage,Message);
		SetCursor(&WatchCursor);
		while(Button()){};
		for(i=0; i < im; i++) {
			PleaseWait();
			if(ReadInteger(refnum,&k,&pos) == FAILED) goto ERR1;
			(*p_Code)[i].time = i / Time_res;
			(*p_Code)[i].byte = k;
			(*p_Code)[i].sequence = 0;
			if(Button() && Answer("Continue reading",'Y') != OK) break;
			/* This timing is only needed for the control of overflow when sending codes */
			}
		FSClose(refnum);
		sprintf(Message,"Sending %ld bytes to MIDI device.",(long)im);
		ShowMessage(TRUE,wMessage,Message);
		ResetMIDI(FALSE);
		if(sysex) send = FALSE; else send = TRUE;
		maxbytes = MIDI_FIFO_MAX - 50;
		nbytes = 0; tbytes2 = ZERO;
		for(i=0; i < im; i++) {
			if(Button() && Answer("Continue sending",'Y') != OK) break;
			PleaseWait();
			e.type = RAW_EVENT;
			e.time = Tcurr = (*p_Code)[i].time;
			e.data2 = ByteToInt((*p_Code)[i].byte);
			if(!send  && (e.data2 == SystemExclusive)) send = TRUE;
			if(send) {
				DriverWrite(Tcurr * Time_res,0,&e);
				nbytes++;
				}
			if(nbytes > maxbytes/2 && tbytes2 == ZERO) tbytes2 = Tcurr;
			if(nbytes > maxbytes) {
				drivertime = GetDriverTime();
				formertime = ZERO;
				while((timeleft = tbytes2 - drivertime) > ZERO) {
					if((timeleft * Time_res / 1000L) != formertime) {
						formertime = timeleft * Time_res / 1000L;
						sprintf(Message,"Idling (%ld sec)",
							(long)formertime + 1L);
						ShowMessage(FALSE,wMessage,Message);
						}
					drivertime = GetDriverTime();
					}
				HideWindow(Window[wInfo]);
				tbytes2 = ZERO; nbytes = maxbytes/2;
				}
			}
		ptr = (Handle) p_Code;
		MyDisposeHandle(&ptr);
		p_Code = NULL;
		while(Tcurr > drivertime  + (SetUpTime / Time_res)) {
			drivertime = GetDriverTime();
			PleaseWait();
			}
		sprintf(Message,"%ld bytes have been sent. Check result on MIDI device",
			(long)im);
		Alert1(Message);
		goto OUT;
		}
	}
else  return (ABORT);

ERR1:
FSClose(refnum);
Alert1("Couldn't read enough bytes from this file. Check its format");

ERR2:
MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
ptr = (Handle) p_Code;
MyDisposeHandle(&ptr);
p_Code = NULL;
r = FAILED;

OUT:
HideWindow(Window[wMessage]);
return(r);
#endif
}


mOpenFile(int w)
{
int anyfile,r,type,result,clear,oldoutmidi,oms,i,badname,dindex;
FSSpec spec;
short refnum;
char *p,*q;
Str255 fn;
OSErr io;

w = FindGoodIndex(w);
if(CheckEmergency() != OK) return(FAILED);
oldoutmidi = OutMIDI;
badname = FALSE;
if(w == wControlPannel || (!Editable[w] && !HasFields[w])) {
	w = LastEditWindow;
	BPActivateWindow(QUICK,w);
	}
if(LockedWindow[w]) return(FAILED);
if(w == wScript) EndWriteScript();
if(w == wScriptDialog) {
	w = wScript; EndWriteScript();
	BPActivateWindow(SLOW,w);
	}
if((w == wGrammar || w == iObjects || w == wGlossary || w == wInteraction
		|| w == wAlphabet || w == wPrototype7 || w == wMIDIorchestra)
		&& (ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn
		|| CompileOn || GraphicOn || PolyOn)) {
	Alert1("Can't change this file because a task using this file is being executed...");
	return(FAILED);
	}
r = OK;
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
HideWindow(Window[wInfo]);
if(SaveCheck(w) == ABORT) return(ABORT);
switch(w) {
	case wInteraction:
		if((r=ClearWindow(FALSE,w)) != OK) return(r);
		ForgetFileName(w);
#if WITH_REAL_TIME_MIDI // FIXME: can we load an interaction file without RT MIDI? What sd we do here? - akozar
		Interactive = OutMIDI = TRUE; SetButtons(TRUE);
		if(!oldoutmidi) ResetMIDI(FALSE);
#endif
		LoadedIn = CompiledIn = anyfile = FALSE;
		if(Option /* && (r = Answer("Open any file type",'N')) == YES */) anyfile = TRUE;
		if(r == ABORT) return(FAILED);
		r = LoadInteraction(anyfile,TRUE);
		if(r == OK && ScriptRecOn) {
			sprintf(Message,"\"%s\"",FileName[wInteraction]);
			MystrcpyStringToTable(ScriptLine.arg,0,Message);
			if(FileName[wInteraction][0] != '\0') AppendScript(24);
			}
		return(r);
		break;
	case wGlossary:
		if((r=ClearWindow(FALSE,w)) != OK) return(r);
		ForgetFileName(w);
		LoadedGl = CompiledGl = anyfile = FALSE;
		if(Option /* && (r = Answer("Open any file type",'N')) == YES */) anyfile = TRUE;
		if(r == ABORT) return(FAILED);
		r = LoadGlossary(anyfile,TRUE);
		if(r == OK && ScriptRecOn) {
			sprintf(Message,"\"%s\"",FileName[wGlossary]);
			MystrcpyStringToTable(ScriptLine.arg,0,Message);
			if(FileName[wGlossary][0] != '\0') AppendScript(24);
			}
		return(r);
		break;
	case iObjects:
		if(IsEmpty(wAlphabet)) {
			if(Answer("Your alphabet is empty. Loading prototypes will generate a new alphabet.\nFirst load an alphabet file",
				'N') == OK) mOpenFile(wAlphabet);
			}
		else {
			if(Answer("Your alphabet is not empty. Loading prototypes will generate new symbols.\nFirst clear current alphabet and grammar",
				'Y') == OK) {
				if(ResetProject(FALSE) != OK) return(ABORT);
				}
			}
		if((r=ClearWindow(FALSE,w)) != OK) return(r);
		ForgetFileName(w);
		if(CompileCheck() != OK) return(STOP);
		FileName[iObjects][0] = '\0';
		if((r=LoadObjectPrototypes(YES,YES)) != OK) {
			ObjectMode = ObjectTry = FALSE;
			FileName[iObjects][0] = '\0';
			SetName(iObjects,TRUE,TRUE);
			Dirty[iObjects] = Created[iObjects] = FALSE;
			iProto = 0;
			}
		else {
			sprintf(Message,"\"%s\"",FileName[iObjects]);
			MystrcpyStringToTable(ScriptLine.arg,0,Message);
			if(FileName[iObjects][0] != '\0') AppendScript(24);
			if(iProto >= Jbol) iProto = 2;
			SetPrototype(iProto);
			SetCsoundScore(iProto);
			BPActivateWindow(SLOW,wPrototype1);
			ObjectTry = ObjectMode = TRUE;
	/*		CompileObjectScore(iProto,&longerCsound); */
			StopWait();
			}
		return(r);
		break;
	}
clear = FALSE; r = OK;
if(Editable[w] && !IsEmpty(w) && w != wGrammar) {
	sprintf(Message,"Clear '%s' window",WindowName[w]);
	if((r=Answer(Message,'Y')) == YES) {
		clear = TRUE;
		sprintf(Message,"\"%s\"",WindowName[w]);
		MystrcpyStringToTable(ScriptLine.arg,0,Message);
		if(w != wScript) AppendScript(25);
		}
	else Created[w] = FALSE;
	}
if(r == ABORT) return(FAILED);
if(Editable[w] && IsEmpty(w)) clear = TRUE;
anyfile = FALSE; r = OK;
if(w == wTrace || (/*w != wData &&*/ w != wScrap
	&& Option /* && (r = Answer("Open any file type",'N')) == YES */)) anyfile = TRUE;
if(r == ABORT) return(FAILED);

TRYLOAD:
LastAction = NO;
if(DocumentTypeName[w][0] != '\0' && w != wTrace) {
	sprintf(Message,"Select a %s file...",DocumentTypeName[w]);
	ShowMessage(TRUE,wMessage,Message);
	}
type = gFileType[w];
if(anyfile) type = ftiAny;
result = FAILED;
if(OldFile(w,type,fn,&spec)) {
	if(gFileType[w] != ftiAny && gFileType[w] != ftiText && IdentifyBPFileType(&spec) != w) {
		//anyfile = TRUE;
		p2cstrcpy(LineBuff,fn);
		sprintf(Message,"BP2 is not sure that '%s' is a(n) %s file. Do you want to load it anyway", LineBuff, 
			DocumentTypeName[w]);
		r = Answer(Message,'N');
		if(r != YES) {
			if(r == NO) ShowMessage(TRUE,wMessage,"Hint: You can use the 'Scrap' window to load any file...");
			return(ABORT);
			}
		// check if file has prefix or extension of a BP
		// document type that we are NOT loading
		dindex = FindMatchingFileNamePrefix(LineBuff);
		if (dindex != w && dindex != wUnknown)  badname = TRUE;
		dindex = FindMatchingFileNameExtension(LineBuff);
		if (dindex != w && dindex != wUnknown)  badname = TRUE;
		}
	if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
		if(clear) {
			ClearWindow(FALSE,w);
			ForgetFileName(w);
			}
		else if(Editable[w]) SetSelect(GetTextLength(w),GetTextLength(w),TEH[w]);
		/* Let's not change the filename on the user, but mark it as
		   Weird below so that they are prompted to choose a name again 
		   when saving.  Besides, this constructed name was getting 
		   overwritten below if clear == TRUE.  -- akozar 031507 */
		/* if(badname) {
			p2cstrcpy(LineBuff,spec.name);
			for(i=0; i < strlen(FilePrefix[w]); i++)
				FileName[w][i] = FilePrefix[w][i];
			for(i=strlen(FilePrefix[w]); i < strlen(LineBuff); i++)
				FileName[w][i] = LineBuff[i];
			}
		else */
		p2cstrcpy(FileName[w],spec.name);
		TheVRefNum[w] = spec.vRefNum;
		WindowParID[w] = spec.parID;
		if(anyfile || badname) Weird[w] = TRUE;
		else Weird[w] = FALSE;
		if(w == wKeyboard) {
			r = LoadKeyboard(refnum);
			if(r == OK && ScriptRecOn) {
				sprintf(Message,"\"%s\"",FileName[w]);
				MystrcpyStringToTable(ScriptLine.arg,0,Message);
				if(FileName[w][0] != '\0') AppendScript(24);
				}
			Token = TRUE; MaintainMenus();
			return(r);
			}
		if(w == wTimeBase) {
			r = LoadTimeBase(refnum);
			if(r == OK && ScriptRecOn) {
				sprintf(Message,"\"%s\"",FileName[w]);
				MystrcpyStringToTable(ScriptLine.arg,0,Message);
				if(FileName[w][0] != '\0') AppendScript(24);
				}
			MaintainMenus();
			return(r);
			}
		if(w == wCsoundInstruments) {
			r = LoadCsoundInstruments(refnum,FALSE);
			if(r == OK) SetName(w,TRUE,TRUE);
			if(r == OK && ScriptRecOn) {
				sprintf(Message,"\"%s\"",FileName[w]);
				MystrcpyStringToTable(ScriptLine.arg,0,Message);
				if(FileName[w][0] != '\0') AppendScript(24);
				}
			MaintainMenus();
			return(r);
			}
		if(w == wMIDIorchestra) {
			r = LoadMIDIorchestra(refnum,FALSE);
			if(r == OK) SetName(w,TRUE,TRUE);
			if(r == OK && ScriptRecOn) {
				sprintf(Message,"\"%s\"",FileName[w]);
				MystrcpyStringToTable(ScriptLine.arg,0,Message);
				if(FileName[w][0] != '\0') AppendScript(24);
				}
			MaintainMenus();
			return(r);
			}
		if((w == wGrammar) && (LoadGrammar(&spec,refnum) == OK)) {
			sprintf(Message,"\"%s\"",FileName[wGrammar]);
			MystrcpyStringToTable(ScriptLine.arg,0,Message);
			if(FileName[wGrammar][0] != '\0') AppendScript(24);
			LoadAlphabet(wGrammar,&spec);
			sprintf(Message,"\"%s\"",FileName[wAlphabet]);
			MystrcpyStringToTable(ScriptLine.arg,0,Message);
			if(FileName[wAlphabet][0] != '\0') AppendScript(24);
			BPActivateWindow(QUICK,wStartString);
			result = OK;
			}
		else {  // FIXME: tries to load grammar again if first time failed ?
			if(ReadFile(w,refnum) == OK) {
				if(!WASTE) CCUTEToScrap();	// WHY?
				if(w != wScrap) GetHeader(w);
				if(clear) {
					/* p2cstrcpy(FileName[w],spec.name); */
					SetName(w,TRUE,TRUE);
					}
				if(w == wGrammar || w == wAlphabet || w == wData) {
					TheVRefNum[wGrammar] = TheVRefNum[wInteraction]
					= TheVRefNum[wGlossary] = TheVRefNum[wTimeBase]
					= TheVRefNum[iSettings] = TheVRefNum[wAlphabet] = TheVRefNum[iObjects]
					= TheVRefNum[wKeyboard] = TheVRefNum[wCsoundInstruments]
					= TheVRefNum[wMIDIorchestra] = spec.vRefNum;
					WindowParID[iObjects] = WindowParID[wGrammar] = WindowParID[wKeyboard]
					= WindowParID[wInteraction] = WindowParID[wGlossary]
					= WindowParID[iSettings] = WindowParID[wTimeBase]
					= WindowParID[wAlphabet] = WindowParID[wCsoundInstruments]
					= WindowParID[wMIDIorchestra] = spec.parID;
					}
				result = OK;
				sprintf(Message,"\"%s\"",FileName[w]);
				MystrcpyStringToTable(ScriptLine.arg,0,Message);
				if(w != wScrap && FileName[w][0] != '\0') AppendScript(24);
				}
			else {
				sprintf(Message,"Can't read '%s'... (no data)",FileName[w]);
				Alert1(Message);
				result = FAILED;
				}
			if(FSClose(refnum) != noErr) {
				if(Beta) Alert1("Error closing file...");
				}
			if(result == OK) {
				if(Editable[w]) SetSelect(ZERO,ZERO,TEH[w]);
				BPActivateWindow(SLOW,w);
				if(w == wAlphabet) {
					GetMiName(); GetKbName(w);
					GetCsName(w);
					GetFileNameAndLoadIt(wMIDIorchestra,w,LoadMIDIorchestra);
					}
				if(w == wData) {
					if(GetSeName(w) == OK)
						result = LoadSettings(TRUE,TRUE,FALSE,FALSE,&oms);
					LoadLinkedMidiDriverSettings(w);
					GetTimeBaseName(w);
					GetGlName(w);
					GetInName(w);
					GetCsName(w);
					GetFileNameAndLoadIt(wMIDIorchestra,w,LoadMIDIorchestra);
					if(LoadInteraction(TRUE,FALSE) != OK) return(OK);
					if(LoadAlphabet(wData,&spec) != OK) return(OK);
					if(result == OK && SmartCursor) result = CompileAlphabet();
					}
				if(w == wScript) BPActivateWindow(SLOW,wScriptDialog);
				}
			}
		}
	else {
		sprintf(Message,"Unexpected error opening '%s'",LineBuff);
		ShowMessage(TRUE,wMessage,Message);
		TellError(8,io);
		}
	}
HideWindow(Window[wMessage]);
if(Editable[w]) SetSelect(ZERO,ZERO,TEH[w]);
if(result == OK) {
	result = BPActivateWindow(SLOW,w);
	/*if((w == wScript || w == wData || w == wGrammar) && !anyfile)*/
	// These windows do not change their name, so always prompt to save - akozar
	if (w != wTrace && w != wScrap && w != wNotice && w != wHelp && w != wStartString)
		Created[w] = TRUE;
	}
return(result);
}


mClearWindow(int wind)
{
int r;

wind = FindGoodIndex(wind);
if(wind < 0 || wind >= WMAX) return(FAILED);
if((wind == wGrammar || wind == wData || wind == wAlphabet)
	&& (SetTimeOn || PrintOn || SoundOn || SelectOn
	|| CompileOn || GraphicOn || PolyOn)) return(FAILED);
if((r=ClearWindow(FALSE,wind)) != OK) return(r);
ForgetFileName(wind);
if(!ScriptExecOn) BPActivateWindow(SLOW,wind);
sprintf(Message,"\"%s\"",WindowName[wind]);
MystrcpyStringToTable(ScriptLine.arg,0,Message);
if(wind != wScript) AppendScript(25);
return(OK);
}


mGoAway(int wind)
{
return(GoAway(wind));
}


mSelectAll(int wind)
{
long posmax;
TEHandle h;

if(wind < 0 || wind >= WMAX) return(FAILED);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
if(Editable[wind]) {
	posmax = GetTextLength(wind);
	SetSelect(ZERO,posmax,TEH[wind]);
	}
else {
	if(IsDialog[wind] && HasFields[wind]) {	// IsDialog just to be sure - 011207 akozar
		h = GetDialogTextEditHandle(gpDialogs[wind]);
		TESetSelect(ZERO,32767,h);
		}
	else return(FAILED);
	}
sprintf(Message,"\"%s\"",WindowName[wind]);
MystrcpyStringToTable(ScriptLine.arg,0,Message);
AppendScript(6);
return(OK);
}


mSaveFile(int w)
{
int w1,rep,longerCsound;
Str255 fn;
FSSpec spec;

if(w < 0 || w >= WMAX) return(FAILED);
w1 = FindGoodIndex(w);
if(w == wPrototype7) {
	if(CompileObjectScore(iProto,&longerCsound) != OK) return(FAILED);
	w1 = iObjects;
	}
SetCursor(&WatchCursor);
spec.vRefNum = TheVRefNum[w1];
spec.parID = WindowParID[w1];
c2pstrcpy(spec.name, FileName[w1]);
rep = OK;
c2pstrcpy(fn, FileName[w1]);
if(!Created[w1] || Weird[w1]) {
	if(!Editable[w1]) return(mSaveAs(w1));
	if((rep=SaveAs(fn,&spec,w1)) == OK) {
		p2cstrcpy(FileName[w1],spec.name);
		TheVRefNum[w1] = spec.vRefNum;
		WindowParID[w1] = spec.parID;
		SetName(w1,TRUE,TRUE);
		Created[w1] = TRUE;
		}
	}
else {
	// spec.vRefNum = TheVRefNum[w1];
	// spec.parID = WindowParID[w1];
	// c2pstrcpy(spec.name, FileName[w1]);
	sprintf(Message,"Saving '%s'...",FileName[w1]);
	ShowMessage(TRUE,wMessage,Message);
	switch(w1) {
		case iObjects:
			rep = SaveObjectPrototypes(&spec); break;
		case wKeyboard:
			rep = SaveKeyboard(&spec); break;
		case wTimeBase:
			rep = SaveTimeBase(&spec); break;
		case wCsoundInstruments:
			rep = SaveCsoundInstruments(&spec); break;
		case iSettings:
			rep = mSaveSettings(w1); break;
		case wMIDIorchestra:
			rep = SaveMIDIorchestra(FALSE); break;
		default:
			rep = SaveFile(fn,&spec,w1); break;
		}
	}
// HideWindow(Window[wMessage]); // was hiding error messages
/* BPActivateWindow(SLOW,w); */
if (rep == OK) {
	sprintf(Message,"Successfully saved '%s'", FileName[w1]);
	ShowMessage(TRUE,wMessage,Message);
	}
return(rep);
}


mSaveAs(int w)
{
int r;
Str255 fn;
FSSpec spec;

if(w < 0 || w >= WMAX) return(FAILED);
w = FindGoodIndex(w);
c2pstrcpy(fn, FileName[w]);
spec.vRefNum = TheVRefNum[w];
spec.parID = WindowParID[w];
c2pstrcpy(spec.name, FileName[w]);

if(!Editable[w]) Created[w] = FALSE;

switch(w) {
	case iObjects:
		r = SaveObjectPrototypes(&spec); return(r);
		break;
	case wKeyboard:
		r = SaveKeyboard(&spec); return(r);
		break;
	case wTimeBase:
		r = SaveTimeBase(&spec); return(r);
		break;
	case wCsoundInstruments:
		r = SaveCsoundInstruments(&spec); return(r);
		break;
	case iSettings:
		r = mSaveSettingsAs(w); return(r);
		break;
	case wMIDIorchestra:
		r = SaveMIDIorchestra(TRUE); return(r);
		break;
	}
if(!Editable[w]) return(FAILED);
if(SaveAs(fn,&spec,w) == OK) {
	p2cstrcpy(FileName[w],spec.name);
	TheVRefNum[w] = spec.vRefNum;
	WindowParID[w] = spec.parID;
	SetName(w,TRUE,TRUE);
	Created[w] = TRUE;
	}
BPActivateWindow(SLOW,w);
return(OK);
}


mLoadSettings(int wind)
{
int rep,oms,anyfile;

if(CheckEmergency() != OK) return(FAILED);
if(Dirty[iSettings] && Created[iSettings] && FileName[iSettings][0] != 0) {
	sprintf(Message, "Save changes in current settings to file '%s'", FileName[iSettings]);
	if((rep=Answer(Message, 'Y')) == OK) mSaveSettings(wind);
	if(rep == ABORT) return(FAILED);
	// Dirty[iSettings] = FALSE; // not true yet - akozar 061107
	}
FileName[iSettings][0] = '\0';
anyfile = FALSE;
rep = OK;
if(Option /* && (rep=Answer("Open any file type",'N')) == YES */) anyfile = TRUE;
if(rep == ABORT) return(OK);
LoadSettings(anyfile,TRUE,FALSE,TRUE,&oms);
sprintf(Message,"\"%s\"",FileName[iSettings]);
MystrcpyStringToTable(ScriptLine.arg,0,Message);
AppendScript(26);
BPActivateWindow(SLOW,wind);
return(OK);
}


mSaveSettings(int wind)
{
	int result;
	Str255 fn;
	FSSpec spec;
	
	result = FAILED;
	if (Created[iSettings] && FileName[iSettings][0] != 0) {
		c2pstrcpy(fn, FileName[iSettings]);
		spec.vRefNum = TheVRefNum[iSettings];
		spec.parID = WindowParID[iSettings];
		c2pstrcpy(spec.name, FileName[iSettings]);
		result = SaveSettings(NO,YES,fn,&spec);
	}
	else return mSaveSettingsAs(wind);
	
	BPActivateWindow(SLOW,wind);
	return (result);
}


mSaveSettingsAs(int wind)
{
int rep;
Str255 fn;
FSSpec spec;

if(Dirty[wData]) GetSeName(wData);
if(Dirty[wGrammar]) GetSeName(wGrammar);
c2pstrcpy(fn, FileName[iSettings]);
spec.vRefNum = TheVRefNum[iSettings];
spec.parID = WindowParID[iSettings];
c2pstrcpy(spec.name, FileName[iSettings]);
rep = SaveSettings(NO,NO,fn,&spec);
BPActivateWindow(SLOW,wind);
return(OK);
}


mSaveStartup(int wind)
{
int rep;
FSSpec spec;

if(Answer("Save current settings as startup",'Y') != OK) return(FAILED);
GetStartupSettingsSpec(&spec);
SaveSettings(YES,YES,spec.name,&spec);
return(OK);
}


mSaveDecisions(int wind)
{
if(ProduceStackDepth == 0) {
	Alert1("No item has been produced");
	}
else {
	if(ProduceStackDepth == -1) {
		Alert1("Can't save... Decisions are lost!");
		}
	else SaveDecisions();
	}
BPActivateWindow(SLOW,wind);
return(OK);
}


mRevert(int wind)
{
int rep,longerCsound;
TextOffset selbegin, selend;
short refnum;
FSSpec spec;
OSErr io;

wind = FindGoodIndex(wind);
if(wind == wPrototype7) wind = iObjects;
if(wind < 0 || wind >= WMAX || LockedWindow[wind] || FileName[wind][0] == '\0')
	return(FAILED);
sprintf(Message,"Revert to last saved version\nof %s",FileName[wind]);
rep = Answer(Message,'N');
switch (rep) {
	case YES:
		Dirty[wind] = FALSE;
		if(Editable[wind]) {
			TextGetSelection(&selbegin, &selend, TEH[wind]);
			SetSelect(ZERO,GetTextLength(wind),TEH[wind]);
			TextDelete(wind);
			}
		spec.vRefNum = TheVRefNum[wind];
		spec.parID = WindowParID[wind];
		c2pstrcpy(spec.name, FileName[wind]);
		switch(wind) {
			case wGlossary:
				LoadedGl = CompiledGl = FALSE;
				LoadGlossary(TRUE,FALSE);
				goto OUT;
				break;
			case wInteraction:
				LoadedIn = CompiledIn = FALSE;
				LoadInteraction(FALSE,TRUE);
				goto OUT;
				break;
			case wMIDIorchestra:
				if(MyOpen(&spec,fsCurPerm,&refnum) == noErr)
					LoadMIDIorchestra(refnum,FALSE);
				goto OUT;
				break;
			case iObjects:
				if(LoadObjectPrototypes(YES,YES) != OK) {
					ObjectMode = ObjectTry = FALSE;
					FileName[iObjects][0] = '\0';
					SetName(iObjects,TRUE,TRUE);
					Dirty[iObjects] = FALSE;
					iProto = 0;
					}
				else {
					SetCsoundScore(iProto);
					SetPrototype(iProto);
					BPActivateWindow(SLOW,wPrototype1);
					ObjectTry = ObjectMode = TRUE;
					CompileObjectScore(iProto,&longerCsound);
					StopWait();
					}
				return(OK);
				break;
			}
		c2pstrcpy(spec.name, FileName[wind]);
		if(FileName[wind][0] != '\0') {
			if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
				if(!Editable[wind]) {
					if(wind == wTimeBase) LoadTimeBase(refnum);
					if(wind == wCsoundInstruments) {
						return(LoadCsoundInstruments(refnum,TRUE));
						}
					if(wind == wKeyboard) LoadKeyboard(refnum);
					}
				else {
					if(ReadFile(wind,refnum) == OK) {
						UpdateDirty(TRUE,wind);
						Dirty[wind] = FALSE;
#if WASTE
						WEResetModCount(TEH[wind]);
#endif
						}
					else {
						sprintf(Message,"Can't read '%s'... (no data)",FileName[wind]);
						Alert1(Message);
						FSClose(refnum);
						return(FAILED);
						}
					}
				if(FSClose(refnum) == noErr);
				}
			else TellError(9,io);
			}
		UpdateWindow(FALSE,Window[wind]);
		GetHeader(wind);
		if(wind == wGrammar || wind == wData) {
			GetSeName(wind);
			LoadLinkedMidiDriverSettings(wind);
			GetTimeBaseName(wind);
			GetAlphaName(wind);
			GetCsName(wind);
			GetFileNameAndLoadIt(wMIDIorchestra,wind,LoadMIDIorchestra);
			}
		if(wind == wAlphabet) {
			GetMiName();
			GetKbName(wind);
			GetCsName(wind);
			GetFileNameAndLoadIt(wMIDIorchestra,wind,LoadMIDIorchestra);
			}
		break;
	case NO:
 	case ABORT:
 		return(FAILED);
 		break;
 	}

OUT:
if(Editable[wind]) {
	SetSelect(selbegin,selbegin,TEH[wind]);
	ShowSelect(CENTRE,wind);
	}
return(OK);
}


mPageSetup(int wind)
{
#if !TARGET_API_MAC_CARBON
	DoPageSetUp();
#else
	Alert1("Bol Processor Carbon is not able to print yet.  Try opening your documents in a text editor.");
#endif
return(OK);
}


mPrint(int wind)
{
int n;
Rect r;

#if TARGET_API_MAC_CARBON
	Alert1("Bol Processor Carbon is not able to print yet.  Try opening your documents in a text editor.");
#else
if(wind < 0 || wind >= WMAX || (!Editable[wind] && !GrafWindow[wind])) return(FAILED);
sprintf(Message,"Printing '%s' window...",WindowName[wind]);
ShowMessage(TRUE,wMessage,Message);

WaitForLastTicks();
WaitForEmptyBuffer();
WaitABit(500L);

if(wind == wGraphic) {
	if(!Offscreen) {
		for(n=0; n < Npicture; n++) {
			GetWindowPortBounds(Window[wGraphic], &r);
			if(OKhScroll[wind]) r.bottom = r.bottom - SBARWIDTH - 1;
			if(OKvScroll[wind]) r.right = r.right - SBARWIDTH - 1;
			PrintGraphicWindow(p_Picture[n],&r);
			}
		if(NoteScalePicture != NULL) PrintGraphicWindow(NoteScalePicture,&NoteScaleRect);
		}
	else Alert1("This version can't print pictures");
	}
else {
	wind = LastEditWindow;
	PrintTextWindow(wind);
	}
HideWindow(Window[wMessage]);

sprintf(Message,"\"%s\"",WindowName[wind]);
MystrcpyStringToTable(ScriptLine.arg,0,Message);
AppendScript(7);
#endif

return(OK);
}


mQuit(int wind)
{
int r,w,s;

if(ScriptExecOn && ResumeStopOn) {
	s = ScriptExecOn;
	ScriptExecOn = FALSE;
	r = Answer("Abort script and quit",'N');
	ScriptExecOn = s;
	if(r != OK) return(FAILED);
	else ScriptExecOn = FALSE;
	}
ResumeStopOn = FALSE;
HideWindow(GetDialogWindow(ResumeStopPtr));
HideWindow(GetDialogWindow(ResumeUndoStopPtr));
if((r=CheckSettings()) == ABORT) return(r);
AppendScript(8);
for(w=0; w < WMAX; w++) {
	PleaseWait();
	if((r=SaveCheck(w)) == ABORT) return(r);
	}
return(EXIT);
}


mUndo(int wind)
{
#if !USE_MLTE  /* FIXME:  use MLTE's built-in Undo */
if(UndoFlag) {
	HideWindow(GetDialogWindow(ResumeUndoStopPtr)); ResumeStopOn = FALSE;
	return(UNDO);
	}
TextAutoView(FALSE,TRUE,TEH[UndoWindow]);
switch(LastAction) {
	case TYPEWIND:
#if WASTE
		WEUndo(TEH[UndoWindow]);
		ShowSelect(CENTRE,UndoWindow);
		break;
#endif
	case TYPEDLG:
		if(LastAction == TYPEWIND) {
			SetSelect(UndoPos,(*TEH[UndoWindow])->selEnd,TEH[UndoWindow]);
			TextCut(UndoWindow);
			LastAction = CUTWIND;
			UpdateDirty(TRUE,UndoWindow);
			Activate(TEH[UndoWindow]);
			ShowSelect(CENTRE,UndoWindow);
			BPActivateWindow(SLOW,UndoWindow);
			}
		else LastAction = NO;
		break;
	case COPY:
#if WASTE
		WEUndo(TEH[UndoWindow]);
		ShowSelect(CENTRE,UndoWindow);
#endif
		break;
	case CUTWIND:
#if WASTE
		WEUndo(TEH[UndoWindow]);
		ShowSelect(CENTRE,UndoWindow);
#else
		SetSelect(UndoPos,UndoPos,TEH[UndoWindow]);
		UndoPos = (**(TEH[UndoWindow])).selStart;
		TextPaste(UndoWindow);
		LastAction = PASTEWIND;
		UpdateDirty(TRUE,UndoWindow);
		Activate(TEH[UndoWindow]);
		ShowSelect(CENTRE,UndoWindow);
		BPActivateWindow(SLOW,UndoWindow);
#endif
		break;
	case CUTDLG: break;
	case PASTEWIND:
#if WASTE
		WEUndo(TEH[UndoWindow]);
		ShowSelect(CENTRE,UndoWindow);
#else
		SetSelect(UndoPos,(*TEH[UndoWindow])->selEnd,TEH[UndoWindow]);
		TextCut(UndoWindow);
		LastAction = CUTWIND;
		UpdateDirty(TRUE,UndoWindow);
		Activate(TEH[UndoWindow]);
		ShowSelect(CENTRE,UndoWindow);
		BPActivateWindow(SLOW,UndoWindow);
#endif
		break;
	case PASTEDLG:
		break;
	case DELETEWIND:
#if WASTE
		WEUndo(TEH[UndoWindow]);
		ShowSelect(CENTRE,UndoWindow);
#else
		SetSelect(UndoPos,UndoPos,TEH[UndoWindow]);
		UndoPos = (**(TEH[UndoWindow])).selStart;
		TextPaste(UndoWindow);
		LastAction = PASTEWIND;
		UpdateDirty(TRUE,UndoWindow);
		Activate(TEH[UndoWindow]);
		ShowSelect(CENTRE,UndoWindow);
		BPActivateWindow(SLOW,UndoWindow);
#endif
		break;
	case SPACESELECTION:
		TextDelete(UndoWindow);
		SetSelect(UndoPos,UndoPos,TEH[UndoWindow]);
		UndoPos = (**(TEH[UndoWindow])).selStart;
		TextPaste(UndoWindow);
		LastAction = NO;
		UpdateDirty(TRUE,UndoWindow);
		Activate(TEH[UndoWindow]);
		ShowSelect(CENTRE,UndoWindow);
		BPActivateWindow(SLOW,UndoWindow);
		break;
	case DELETEDLG:
		break;
	case NO:
/* #if WASTE
		WEUndo(TEH[UndoWindow]);
		ShowSelect(CENTRE,UndoWindow);
#endif */
		break;
	}
TextAutoView(FALSE,FALSE,TEH[UndoWindow]);
#endif

return(OK);
}


mCut(int wind)
{
if(wind < 0 || wind >= WMAX || (!Editable[wind] && !HasFields[wind])) return(FAILED);
if(RecordEditWindow(wind) && wind != wScript) AppendScript(61);
if(Editable[wind]) {
	TextOffset dummy;
	TextAutoView(FALSE,TRUE,TEH[wind]);
	TextCut(wind);
	TextAutoView(FALSE,FALSE,TEH[wind]);
	LastAction = CUTWIND;
	UndoWindow = wind;
	TextGetSelection(&UndoPos, &dummy, TEH[wind]);
	if (Beta)  CheckScrapContents();
	/*if(!WASTE) {
		CCUZeroScrap(); TEToScrap();	// not necessary for multistyled TE - 030607 akozar
		}					// and does not work correctly on OS X
	if (Beta)  CheckScrapContents();*/
	UpdateDirty(FALSE,wind);
	ShowSelect(CENTRE,wind);
	if(IsEmpty(wind)) {
		ForgetFileName(wind);
		}
	}
else if(IsDialog[wind]) {	// IsDialog just to be sure - 011207 akozar
	DialogCut(GetDialogFromWindow(FrontWindow()));
	LastAction = CUTDLG;
	if(FrontWindow() == Window[wind]) {
		UndoWindow = wind;
		GetDialogValues(wind);
		}
	CCUTEToScrap();
	UpdateDirty(TRUE,wind);
	}
return(OK);
}


mCopy(int wind)
{

if(wind < 0 || wind >= WMAX || (!Editable[wind] && !HasFields[wind])) return(FAILED);
if(RecordEditWindow(wind)) AppendScript(62);
if(Editable[wind]) {
	TextCopy(wind);
	if(LastAction == 0) LastAction = COPY;
	LastComputeWindow = wind;
	if (Beta)  CheckScrapContents();
	/*if(!WASTE) {
		CCUZeroScrap(); TEToScrap();
		}
	if (Beta)  CheckScrapContents();*/
	}
else if(IsDialog[wind]) {	// IsDialog just to be sure - 011207 akozar
	DialogCopy(GetDialogFromWindow(FrontWindow()));
	CCUTEToScrap();
	}
// if(WASTE) TEFromScrap();
return(OK);
}


mPaste(int wind)
{

/* if(wind == wPrototype1 && SelectPictureOn && LastAction == COPY) {
	if(Stream.imax > ZERO) return(PasteStreamToPrototype(iProto));
	if((rep=TextToMIDIstream(LastEditWindow)) != OK) return(rep);
	if(Stream.imax > ZERO) return(PasteStreamToPrototype(iProto));
	} */
if(wind < 0 || wind >= WMAX
	|| ((!Editable[wind] || LockedWindow[wind]) && !HasFields[wind])) return(FAILED);
if(RecordEditWindow(wind) && wind != wScript) AppendScript(63);
if(Editable[wind]) {
	TextOffset dummy;
	if (Beta)  CheckScrapContents();
	TextGetSelection(&UndoPos, &dummy, TEH[wind]);
	TextAutoView(FALSE,TRUE,TEH[wind]);
	TextPaste(wind);
	TextAutoView(FALSE,FALSE,TEH[wind]);
	LastAction = PASTEWIND;
	UndoWindow = wind;
	CheckTextSize(wind);
	UpdateDirty(FALSE,wind);
	ShowSelect(CENTRE,wind);
	}
else if(IsDialog[wind]) {	// IsDialog just to be sure - 011207 akozar
	DialogPaste(GetDialogFromWindow(FrontWindow()));
	LastAction = PASTEDLG;
	if(FrontWindow() == Window[wind]) {
		UndoWindow = wind;
		GetDialogValues(wind);
		}
	UpdateDirty(TRUE,wind);
	}
return(OK);
}


mPickPerformanceControl(int wind)
{
int i,w,r;

r = DoThings(p_PerformanceControl,0,MaxPerformanceControl,p_PerfCtrlNdx,16,MINUSPROC,"\0",
	(int) pushButProc);
if(r >= 0 && r < MaxPerformanceControl) {
	if(wind >= 0 && wind < WMAX && Editable[wind] && !LockedWindow[wind]) w = wind;
	else w = LastEditWindow;
	r = (*p_PerfCtrlNdx)[r];
	Print(w," ");
	PrintHandle(w,(*p_PerformanceControl)[r]);
	if((*p_PerfCtrlNArg)[r] > 0) {
		Print(w,"(¥");
		for(i=0; i < (*p_PerfCtrlNArg)[r]-1; i++) Print(w,",¥");
		Print(w,")");
		}
	UpdateDirty(YES,w);
	}
return(OK);
}


mPickGrammarProcedure(int wind)
{
int i,r;

r = DoThings(p_GramProcedure,0,MaxProc,p_ProcNdx,16,MINUSPROC,"\0",(int) pushButProc);
if(r >= 0 && r < MaxProc) {
	r = (*p_ProcNdx)[r];
	Print(wGrammar," ");
	PrintHandle(wGrammar,(*p_GramProcedure)[r]);
	if((*p_ProcNArg)[r] > 0) {
		Print(wGrammar,"(¥");
		for(i=0; i < (*p_ProcNArg)[r]-1; i++) Print(wGrammar,",¥");
		Print(wGrammar,")");
		}
	CompiledGr = FALSE;
	}
return(OK);
}


mClear(int wind)
{
if(wind < 0 || wind >= WMAX
	|| (!Editable[wind] && !HasFields[wind])) return(FAILED);
if(RecordEditWindow(wind) && wind != wScript) AppendScript(64);
if(Editable[wind]) {
	TextDelete(wind);
	UpdateDirty(FALSE,wind);
	}
else if(IsDialog[wind]) {	// IsDialog just to be sure - 011207 akozar
	DialogDelete(gpDialogs[wind]);
	UpdateDirty(TRUE,wind);
	}
return(OK);
}


mGrammar(int wind)
{
int r;

r = OK;
if(Option /* && (r=Answer("Display tokenized grammar",'Y')) == YES */) {
	if(!CompiledGr) r = CompileGrammar(1);
	if((CompiledGr || N_err > 0) && Gram.number_gram > 0) {
		DisplayGrammar(&Gram,wTrace,FALSE,TRUE,TRUE);
		return(OK);
		}
	else Alert1("No token created in grammar");
	}
AppendScript(27);
BPActivateWindow(SLOW,wGrammar);
if(ScriptExecOn) ScriptW = wGrammar;
return(OK);
}


mAlphabet(int wind)
{
if(Option /* && Answer("Display tokenized alphabet",'Y') == YES */) {
	if(CompiledAl || CompileAlphabet() == OK) {
		ShowAlphabet();
		return(OK);
		}
	else Alert1("No token created in alphabet");
	}
AppendScript(28); 
BPActivateWindow(SLOW,wAlphabet);
if(ScriptExecOn) ScriptW = wAlphabet;
return(OK);
}


mData(int wind)
{
AppendScript(29); 
BPActivateWindow(SLOW,wData);
if(ScriptExecOn) ScriptW = wData;
return(OK);
}


mMiscSettings(int wind)
{
AppendScript(30);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);

/* only display Top & Bottom settings when option key down on OS X */
if (!Option) {
ShowWindow(GetDialogWindow(StrikeModePtr));
BringToFront(GetDialogWindow(StrikeModePtr));
SetDefaultStrikeMode();
BPUpdateDialog(StrikeModePtr);

ShowWindow(GetDialogWindow(FileSavePreferencesPtr));
BringToFront(GetDialogWindow(FileSavePreferencesPtr));
SetFileSavePreferences();
BPUpdateDialog(FileSavePreferencesPtr);

ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
BringToFront(GetDialogWindow(DefaultPerformanceValuesPtr));
SetDefaultPerformanceValues();
BPUpdateDialog(DefaultPerformanceValuesPtr);

ShowWindow(GetDialogWindow(TuningPtr));
BringToFront(GetDialogWindow(TuningPtr));
SetTuning();
BPUpdateDialog(TuningPtr);
}

BPActivateWindow(SLOW,wSettingsTop);
BPActivateWindow(SLOW,wSettingsBottom);
if (!Option) BPActivateWindow(SLOW,wTimeAccuracy);
return(OK);
}


mInteraction(int wind)
{
AppendScript(31); 
BPActivateWindow(SLOW,wInteraction);
if(ScriptExecOn) ScriptW = wInteraction;
return(OK);
}


mGlossary(int wind)
{
int r;

if(CompiledGl && GlossGram.p_subgram != NULL) {
	if(Option /* && (r=Answer("Display tokenized glossary",'Y')) == YES */) {
		DisplayGrammar(&GlossGram,wTrace,FALSE,FALSE,FALSE);
		return(OK);
		}
	}
AppendScript(143); 
BPActivateWindow(SLOW,wGlossary);
if(ScriptExecOn) ScriptW = wGlossary;
return(OK);
}


mStartString(int wind)
{
AppendScript(32); 
BPActivateWindow(SLOW,wStartString);
if(ScriptExecOn) ScriptW = wStartString;
return(OK);
}


mTrace(int wind)
{
AppendScript(34); 
BPActivateWindow(SLOW,wTrace);
if(ScriptExecOn) ScriptW = wTrace;
return(OK);
}


mControlPannel(int wind)
{
AppendScript(36);
BPActivateWindow(SLOW, wControlPannel);
return(OK);
}


mKeyboard(int wind)
{
AppendScript(38); 
BPActivateWindow(SLOW,wKeyboard);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
return(OK);
}


mScrap(int wind)
{
AppendScript(108); 
BPActivateWindow(SLOW,wScrap);
if(ScriptExecOn) ScriptW = wScrap;
return(OK);
}


mNotice(int wind)
{
AppendScript(109); 
BPActivateWindow(SLOW,wNotice);
return(OK);
}


mGraphic(int wind)
{
int found=FALSE;

AppendScript(35);
found = TRUE;
if(found) BPActivateWindow(SLOW,wGraphic);
else Alert1("First load or produce items...");
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
return(OK);
}


mScript(int wind)
{
AppendScript(37); 
BPActivateWindow(SLOW,wScriptDialog);
BPActivateWindow(SLOW,wScript);
return(OK);
}


mFind(int wind)
{
Rect r;
WindowPtr wp;

if(wind < 0 || wind >= WMAX || !Editable[wind]) {
	Alert1("You can search only text window...");
	return(OK);
	}
TargetWindow = LastEditWindow = wind;
BPActivateWindow(QUICK,wFindReplace);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
SelectField(NULL,wFindReplace,fFind,TRUE);

// We need the following so that typing will directly be taken as a dialog
// event belonging to this window $$$
SetPortDialogPort(gpDialogs[wFindReplace]);
wp = GetDialogWindow(gpDialogs[wFindReplace]);
GetWindowPortBounds(wp, &r);
InvalWindowRect(wp, &r);
return(OK);
}


mEnterFind(int wind)
{
Handle myHandle;
long scrapOffset, rc;
TextOffset selbegin, selend;

if(wind < 0 || wind >= WMAX || !Editable[wind]) {
	Alert1("You can search only text window...");
	return(OK);
	}
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
TargetWindow = LastEditWindow = wind;
/* make a temporary copy of the scrap */
if(!WASTE) {
	CCUTEToScrap();	/* save current text edit scrap */
	}
myHandle = (Handle) GiveSpace(1);
rc = CCUGetScrap(myHandle,'TEXT',&scrapOffset);
if(rc > 0) {
	MemoryUsed += (unsigned long)(rc - 1);
	// adding a null char is not such a good idea - 012907 akozar
	/* MySetHandleSize(&myHandle,rc+1);
	MyLock(FALSE,myHandle);
	(*myHandle)[rc] = 0;
	MyUnlock(myHandle); */
	}
TextCopy(wind);
// if(WASTE) TEFromScrap();
TextGetSelection(&selbegin, &selend, TEH[wind]);
SetSelect(selbegin,selbegin,TEH[wind]);
BPActivateWindow(SLOW,wFindReplace);
SelectField(NULL,wFindReplace,fFind,TRUE);
DialogPaste(gpDialogs[wFindReplace]);
CCUZeroScrap();
if(rc > 0) {
	MyLock(FALSE,myHandle);
	CCUPutScrap(rc,'TEXT',*myHandle); // was rc+1 with above null added
	MyUnlock(myHandle);
	}
MyDisposeHandle((Handle*)&myHandle);
Dirty[wFindReplace] = TRUE;
GetFindReplace();
return(OK);
}


mFindAgain(int wind)
{
if(wind < 0 || wind >= WMAX || !Editable[wind]) {
	Alert1("You can search only text window...");
	return(OK);
	}
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
TargetWindow = wind;
FindReplace(FALSE);
BPActivateWindow(SLOW,wind);
return(OK);
}


mCheckVariables(int wind)
{
int j,s,unreachable,undefined;

if(CompileCheck() != OK) return(OK);
if(p_Var == NULL) {
	Alert1("No variables found");
	return(OK);
	}
ClearWindow(NO,wNotice);
Print(wNotice,"VARIABLES USED IN GRAMMAR:\n");
undefined = unreachable = 0;
for(j=1; j <= Jvar; j++) { 
	s = (*p_VarStatus)[j];
	sprintf(Message,"%s ",*((*p_Var)[j]));
	if(s & (1+2)) Print(wNotice,Message);
	if((s & 1) && !(s & 2)) unreachable++;
	if((s & 2) && !(s & 1) && !(s & 4))
		undefined++;
	}
Print(wNotice,"\n");
if(undefined) {
	Print(wNotice,"\nUNDEFINED VARIABLES:\n");
	for(j=1; j <= Jvar; j++) {
		s = (*p_VarStatus)[j];
		if((s & 2) && !(s & 1) && !(s & 4)) { 
			sprintf(Message,"%s ",*((*p_Var)[j]));
			Print(wNotice,Message);
			}
		}
	Print(wNotice,"\n");
	}
if(unreachable) {
	Print(wNotice,"\nUNREACHABLE VARIABLES:\n");
	for(j=1; j <= Jvar; j++) {
		s = (*p_VarStatus)[j];
		if((s & 1) && !(s & 2))  {
			sprintf(Message,"%s ",*((*p_Var)[j]));
			Print(wNotice,Message);
			}
		}
	}
SetSelect(ZERO,ZERO,TEH[wNotice]);
ShowSelect(CENTRE,wNotice);
return(OK);
}


mListReserved(int wind)
{
int i,j,ii;

ClearWindow(NO,wNotice);
Print(wNotice,"RESERVED WORDS:\n\n");
Println(wNotice,"Miscellaneous operators and markers (see doc):");
for(i=0; i < (MAXCODE-2); i++) {
	sprintf(Message,"%c ",Code[i]);
	Print(wNotice,Message);
	}
Print(wNotice,"* \\");
Print(wNotice,"\n\nNote conventions used in Apple Event 'conv' (not case sensitive): ");
for(i=0; i < MAXCONVENTIONS-1; i++) {
	sprintf(Message,"%s, ",ConventionString[i]);
	Print(wNotice,Message);
	}
sprintf(Message,"%s\n\n",ConventionString[MAXCONVENTIONS-1]);
Print(wNotice,Message);
sprintf(Message,"'%s' for initialisation line on top of grammar\n\n",InitToken);
Print(wNotice,Message);
Print(wNotice,"Subgrammar types: ");
for(i=0; i < MAXTYPE; i++) {
	sprintf(Message,"%s ",SubgramType[i]);
	Print(wNotice,Message);
	}
Print(wNotice,"\n\n");
Print(wNotice,"Rule operators: ");
for(i=0; i < MAXARROW; i++) {
	sprintf(Message,"%s ",Arrow[i]);
	Print(wNotice,Message);
	}
Print(wNotice,"\n\n");
Print(wNotice,"Rule derive modes: ");
for(i=0; i < MAXMODE; i++) {
	sprintf(Message,"%s ",Mode[i]);
	Print(wNotice,Message);
	}
Print(wNotice,"\n\n");
Print(wNotice,"Grammar procedures (not case sensitive): ");
for(j=0; j < MaxProc; j++) {
	i = (*p_ProcNdx)[j];
	sprintf(Message,"%s",*((*p_GramProcedure)[i]));
	Print(wNotice,Message);
	if((*p_ProcNArg)[i] > 0) {
		Print(wNotice,"(¥");
		for(ii=0; ii < (*p_ProcNArg)[i]-1; ii++) Print(wNotice,",¥");
		Print(wNotice,") ");
		}
	else Print(wNotice," ");
	}
Print(wNotice,"\n\n");
Print(wNotice,"Performance control (not case sensitive): ");
for(j=0; j < MaxPerformanceControl; j++) {
	i = (*p_PerfCtrlNdx)[j];
	sprintf(Message,"%s",*((*p_PerformanceControl)[i]));
	Print(wNotice,Message);
	if((*p_PerfCtrlNArg)[i] > 0) {
		Print(wNotice,"(¥");
		for(ii=0; ii < (*p_PerfCtrlNArg)[i]-1; ii++) Print(wNotice,",¥");
		Print(wNotice,") ");
		}
	else Print(wNotice," ");
	}
Print(wNotice,"\n\n");
Print(wNotice,"Null string (equivalent symbols): ");
for(i=0; i < MAXNIL; i++) {
	sprintf(Message,"%s ",NilString[i]);
	Print(wNotice,Message);
	}
Print(wNotice,"\n\n");
Println(wNotice,"Templates in a grammar start on a 'TEMPLATES:' header");
Println(wNotice,"Grammars may terminate on a 'COMMENTS:' or 'DATA:' header");
SetSelect(ZERO,ZERO,TEH[wNotice]);
ShowSelect(CENTRE,wNotice);
return(OK);
}


mListTerminals(int wind)
{
int j;

if(!CompiledAl  || (!CompiledGr && (AddBolsInGrammar() > BolsInGrammar))) {
	CompiledAl = FALSE;
	if(CompileAlphabet() != OK) return(OK);
	}
if(Jbol < 3) {
	Alert1("No terminal symbols found (other than simple notes)");
	return(OK);
	}
ClearWindow(NO,wNotice);
Print(wNotice,"\nTERMINAL SYMBOLS (ALPHABET):\n");
for(j=2; j < Jbol; j++) {
	sprintf(Message,"%s ",*((*p_Bol)[j]));
	Print(wNotice,Message);
	}
SetSelect(ZERO,ZERO,TEH[wNotice]);
ShowSelect(CENTRE,wNotice);
return(OK);
}


mBalance(int w)
{
TextOffset selbegin, selend;
long i,iorg,startpos,endpos,length;
int levelbracket,levelsquare,levelcurled,
	isbracket,issquare,iscurled;
char c;

if(w < 0 || w >= WMAX || !Editable[w]) return(OK);
TextGetSelection(&selbegin, &selend, TEH[w]);
iorg = selbegin - 1;
length = GetTextLength(w);
levelbracket = levelsquare = levelcurled = 0;
isbracket = issquare = iscurled = FALSE;
for(i=iorg; i >= 0; i--) {
	c = GetTextChar(w,i);
	if(c == ')') levelbracket++;
	if(c == '}') levelcurled++;
	if(c == ']') levelsquare++;
	if(c == '(') {
		if(levelbracket == 0) {
			isbracket = TRUE; break;
			}
		levelbracket--;
		}
	if(c == '{')  {
		if(levelcurled == 0) {
			iscurled = TRUE; break;
			}
		levelcurled--;
		}
	if(c == '[')  {
		if(levelsquare == 0) {
			issquare = TRUE; break;
			}
		levelsquare--;
		}
	}
if(isbracket || issquare || iscurled) {
	startpos = i;
	}
else {
	startpos = 0;
	}

levelbracket = levelsquare = levelcurled = 0;
for(i=startpos; i < length; i++) {
	c = GetTextChar(w,i);
	if(c == '(') levelbracket++;
	if(c == '{') levelcurled++;
	if(c == '[') levelsquare++;
	if(c == ')') {
		if(isbracket && levelbracket == 1) break;
		levelbracket--;
		}
	if(c == '}')  {
		if(iscurled && levelcurled == 1) break;
		levelcurled--;
		}
	if(c == ']')  {
		if(issquare && levelsquare == 1) break;
		levelsquare--;
		}
	}
if(i == length) {
	if(levelbracket == 0 && levelsquare == 0 && levelcurled == 0) {
		endpos = length - 1;
		}
	else {
		SysBeep(10); return(OK);
		}
	}
else {
	endpos = i + 1;
	}
SetSelect((long)startpos,(long)endpos,TEH[w]);
return(OK);
}


mCompile(int wind)
{
AppendScript(9);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
if(CompileCheck() != OK) SysBeep(10);
else BPActivateWindow(SLOW,wind);
return(OK);
}


mProduce(int wind)
{
int r;

if(!ScriptExecOn && (ResetScriptQueue() != OK)) return(FAILED);
if((ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn || CompileOn || GraphicOn
	|| PolyOn)) return(FAILED);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
GetValues(TRUE);
if(OutMIDI && Interactive && !LoadedIn) {
	if(GetInName(wData) != OK) GetInName(wGrammar);
	if(LoadInteraction(TRUE,FALSE) != OK) return(OK);
	}
if(!ScriptExecOn && IsEmpty(wGrammar)) {
	Alert1("Can't produce items because grammar is empty");
	return(FAILED);	
	}
if(CompileCheck() != OK) return(FAILED);

if(WillRandomize) {
//	ReseedOrShuffle(RANDOMIZE);
	if(!AllowRandomize) {
		Alert1("Since '_randomize' was found, button 'Allow randomize' has been checked");
		AllowRandomize = TRUE; SetButtons(TRUE);
		BPActivateWindow(QUICK,wSettingsTop);
		}
	}
else {
	if(!AllowRandomize) ResetRandom();
	else {
		if(UsedRandom && !ScriptExecOn && !AllItems && !AEventOn) {
			UsedRandom = FALSE;
			switch(Improvize) {
				case TRUE:
					if((r=Answer("Reset random sequence",'Y')) == OK)
						ResetRandom();
					break;
				case FALSE:
					if((r=Answer("Reset random sequence",'N')) == OK)
						ResetRandom();
				}
			}
		}
	}
if(r == ABORT) return(FAILED);
Ctrlinit();

DOIT:
if(ScriptExecOn == 0 && AEventOn == 0) OkWait = OK;
SwitchOn(NULL,wControlPannel,dProduceItems);
r = ProduceItems(wStartString,FALSE,FALSE,NULL);
SwitchOff(NULL,wControlPannel,dProduceItems);
if(r == RESUME) return(mResume(wind));
else return(r);
}


mTemplates(int wind)
{
int result;
	
if(ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn || CompileOn || GraphicOn
	|| PolyOn) return(RESUME);
if(CompileCheck() != OK || ShowNotBP() != OK) return(OK);
SwitchOn(NULL,wControlPannel,dTemplates);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
AppendScript(11);
result = ProduceItems(wStartString,FALSE,TRUE,NULL);
SwitchOff(NULL,wControlPannel,dTemplates);
return(result);
}


mAnalyze(int wind)
{
int result;

if(ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn || CompileOn || GraphicOn
	|| PolyOn) return(RESUME);
if(CompileCheck() != OK) return(FAILED);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
HideWindow(Window[wMessage]);
if(ShowNotBP() != OK) return(OK);
if(wTrace != wind) SetSelect(GetTextLength(wTrace),GetTextLength(wTrace),
	TEH[wTrace]);
if(RecordEditWindow(wind)) {
	sprintf(Message,"\"%s\"",WindowName[wind]);
	MystrcpyStringToTable(ScriptLine.arg,0,Message);
	AppendScript(12);
	}
result = AnalyzeSelection(FALSE);
return(result);
}

/* int TestIt(void); $$$ */

mPlaySelectionOrPrototype(int wind)
{
	// play the current sound-object if one of the prototype windows is active
	if((Nw >= wPrototype1 && Nw <= wPrototype7) || Nw == wPrototype8) {
		if(iProto > 1 && iProto < Jbol) {
			if((*p_MIDIsize)[iProto] > ZERO) return(PlayPrototype(iProto));
			else if((*p_CsoundSize)[iProto] > ZERO) DeCompileObjectScore(iProto);
		}
	}
	// otherwise, play the text selection in the last edit window
	else return mPlaySelect(wind);

	return (OK);
}

mPlaySelect(int wind)
{
int r;
long origin,end;

if(CheckEmergency() != OK) return(FAILED);
if(ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn || CompileOn || GraphicOn
	|| PolyOn) return(FAILED);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
if(wind < 0 || wind >= WMAX || !Editable[wind]) wind = LastEditWindow;
HideWindow(Window[wMessage]);
TextGetSelection(&origin, &end, TEH[wind]);
if(Nw == wScript) goto DOIT;
if(RecordEditWindow(wind)) {
	sprintf(Message,"\"%s\"",WindowName[wind]);
	MystrcpyStringToTable(ScriptLine.arg,0,Message);
	AppendScript(13);
	}
if(OutMIDI && Interactive && !LoadedIn) {
	if(GetInName(wData) != OK) GetInName(wGrammar);
	if(LoadInteraction(TRUE,FALSE) != OK) return(OK);
	}
SetSelect(origin,end,TEH[wind]);

DOIT:
if(ScriptExecOn == 0) OkWait = OK;
r = PlaySelection(wind);
if(r == OK) BPActivateWindow(SLOW,wind);
return(r);
}


mRandomSequence(int wind)
{
SetSeed();
BPActivateWindow(SLOW,wRandomSequence);
return(OK);
}


mTimeBase(int wind)
{
BPActivateWindow(SLOW,wMetronom);
BPActivateWindow(SLOW,wTimeBase);
AppendScript(173);
return(OK);
}


mMetronom(int wind)
{
BPActivateWindow(SLOW,wMetronom);
AppendScript(179);
return(OK);
}


int mComputationSettings(int wind)
{
BPActivateWindow(SLOW,wSettingsTop);
// AppendScript(??);
return(OK);
}


int mInputOutputSettings(int wind)
{
BPActivateWindow(SLOW,wSettingsBottom);
// AppendScript(??);
return(OK);
}


mTimeAccuracy(int wind)
{
BPActivateWindow(SLOW,wTimeAccuracy);
/* BuildAutomaton(); */
return(OK);
}


mBufferSize(int wind)
{
BPActivateWindow(SLOW,wBufferSize);
return(OK);
}


mGraphicSettings(int wind)
{
BPActivateWindow(SLOW,wGraphicSettings);
return(OK);
}


mObjectPrototypes(int wind)
{
int r,longerCsound;
long p,q;
char line[MAXFIELDCONTENT];

if(CompileCheck() != OK) return(OK);
if(!NeedAlphabet) {
	ObjectMode = ObjectTry = FALSE;
	if(Jbol > 2) NeedAlphabet = TRUE;	/* Added if(Jbol > 2) on 31/3/98 */
	}
if(!ObjectMode && !ObjectTry && NeedAlphabet && LoadObjectPrototypes(YES,NO) != OK) {
	if((r=Answer("Load an existing '-mi' sound-object prototype file",'N')) == YES) {
		if((r=CheckPrototypes()) != OK) return(r);
		}
	else {
		if(r != NO) return(r);
		if((r=CheckTerminalSpace()) != OK) return(r);
		if((r=ResizeObjectSpace(YES,Jbol + Jpatt,0)) != OK) return(r);
		ObjectMode = ObjectTry = NeedAlphabet = TRUE;
		}
	}
if(Jbol < 3) iProto = 0;
if(iProto >= Jbol) iProto = Jbol - 1;
GetField(NULL,TRUE,wMetronom,fTempo,line,&p,&q);
sprintf(line,"%.1f", ((double)p)/q);
SetField(NULL,wPrototype1,fMetronomTry,line);
sprintf(line,"100");
SetField(NULL,wPrototype1,fDilationRatio,line);
if(Nature_of_time == STRIATED)
	SwitchOn(NULL,wPrototype1,bStriatedTimeTry);
else
	SwitchOff(NULL,wPrototype1,bStriatedTimeTry);
SwitchOn(NULL,wPrototype1,bShowGraphicTry);
SwitchOff(NULL,wPrototype1,bIgnorePropertiesTry);
SwitchOff(NULL,wPrototype1,bWithParameters);
SetPrototype(iProto);
SetCsoundScore(iProto);
AppendScript(33);
BPActivateWindow(AGAIN,wPrototype1);	/* AGAIN avoids recursive call of mObjectPrototypes() */
DrawPrototype(iProto,wPrototype1,&PictFrame);
CompileObjectScore(iProto,&longerCsound);
StopWait();
return(OK);
}


mPause(wind)
{
int r,oldbuttonon;
unsigned long datemem;

if(!ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn &&
	!SetTimeOn && !GraphicOn && !PrintOn /* && !ReadKeyBoardOn */
	&& !HangOn && !ScriptExecOn) return(OK);
if(PauseOn) return(OK);
oldbuttonon = ButtonOn;
/* ButtonOn = FALSE;	This will allow ResumeStop window to show up */
SetButtons(TRUE);
Interrupted = PauseOn = TRUE;
datemem = CompileDate; UndoFlag = FALSE;
SetResumeStop(TRUE);
while((r = MainEvent()) != RESUME && r != STOP && r != ABORT && r != EXIT
											&& !(StepProduce && r == UNDO)){};
/* ButtonOn = oldbuttonon; */
if(r == ABORT || r == EXIT) EventState = r;
if(datemem != CompileDate) {
	ShowMessage(TRUE,wMessage,"Grammar changed or recompiled. Must abort...");
	EventState = ABORT;
	}
if(r == STOP) EventState = ABORT;
if(r == RESUME) {
	EventState = NO;
	r = OK;
	}
return(r);
}


mResume(int wind)
{
StartCount();
HideWindow(GetDialogWindow(ResumeStopPtr));
HideWindow(GetDialogWindow(ResumeUndoStopPtr));
ResumeStopOn = FALSE;
GetDialogValues(Nw);
SetTempo();
SetTimeAccuracy();
SetCursor(&WatchCursor);
PauseOn = FALSE;
return(RESUME);
}


mStop(int wind)
{
int s,r,result;

StopCount(0);
HideWindow(GetDialogWindow(ResumeStopPtr));
HideWindow(GetDialogWindow(ResumeUndoStopPtr));
ResumeStopOn = FALSE;
HideWindow(Window[wMessage]);

r = OK;
result = STOP;
if(!WaitOn) r = WaitForLastSounds(ZERO);
if(r == EXIT) result = r;

if(EventState == ABORT) ScriptExecOn = 0;
else {
	if((s=ScriptExecOn) > 0) {
		ScriptExecOn = 0;	/* Needed for manual answer to question */
		if(Answer("Continue executing script",'Y') == OK) ScriptExecOn = s;
		}
	}
GetDialogValues(Nw);
SetTempo();
SetTimeAccuracy();
SetCursor(&WatchCursor);
PauseOn = FALSE;
return(result);
}


mHelp(int wind)
{
if(OpenHelp() != OK) return(OK);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
Help = TRUE;
SetCursor(&HelpCursor);
return(OK);
}


mResetSessionTime(int wind)
{
SessionTime = clock();
AppendScript(174);
return(OK);
}


mTellSessionTime(int wind)
{
int w;

sprintf(Message,"Elapsed time in this session: %.0f seconds",
	(double)(clock() - SessionTime) / 60.);
if(ScriptExecOn) {
	w = OutputWindow;
	if(w < 0 || w >= WMAX || !Editable[w]) w = wTrace;
	PrintBehind(w,"\n");
	PrintBehindln(w,Message);
	ShowSelect(CENTRE,w);
	}
else Alert1(Message);
AppendScript(175);
return(OK);
}


mCheckScript(int wind)
{
int i,w,r,changed,vrefnummem;
long parIDmem;
FSSpec spec;
short refnum;
char line[MAXNAME];
OSErr io;

if(ScriptExecOn || ResetScriptQueue() != OK) goto END;
EndWriteScript();
BPActivateWindow(wScript,SLOW);
ReadKeyBoardOn = FALSE; Jcontrol = -1;
if(Dirty[wScript]) {
	// if(FileName[wScript][0] != '\0') Created[wScript] = TRUE;  // suppressed 041207 akozar
	if(mSaveFile(wScript) != OK) goto END;
	}
if(FileName[wScript][0] == '\0') {
	Alert1("No script has been loaded");
	return(OK);
	}
HideWindow(Window[wMessage]);
AppendStringList(FileName[wScript]);
vrefnummem = TheVRefNum[wScript];
parIDmem = WindowParID[wScript];
i = 0; changed = FALSE;

SwitchOn(NULL,wScriptDialog,bCheckScriptSyntax);
while(i < NrStrings) {
	CurrentDir = WindowParID[wScript];
	CurrentVref = TheVRefNum[wScript];
	for(w=0; w < WMAX; w++) {
		WindowParID[w] = CurrentDir;
		TheVRefNum[w] = CurrentVref;
		}
	MystrcpyTableToString(MAXNAME,line,p_StringList,i);
	if((r=RunScriptOnDisk(TRUE,line,&changed)) != OK) {
		if(r == ABORT) break;
		}
	i++;
	}
if(r != ABORT) {
	spec.vRefNum = TheVRefNum[wScript] = vrefnummem;
	spec.parID = WindowParID[wScript] = parIDmem;
	}
if(r != ABORT && changed) {
	/* Reload new version */
	SetSelect(ZERO,GetTextLength(wScript),TEH[wScript]);
	TextDelete(wScript);
	c2pstrcpy(spec.name, FileName[wScript]);
	if(FileName[wScript][0] != '\0') {
		if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
			if(ReadFile(wScript,refnum)) {
				GetHeader(wScript);
				Created[wScript] = TRUE;
				SetName(wScript,TRUE,TRUE);
				}
			else {
				sprintf(Message,"Can't read '%s'...",FileName[wScript]);
				Alert1(Message);
				}
			if(FSClose(refnum) == noErr);
			}
		else {
			Alert1("Unknown error: unable to reload the script...");
			if(Beta) TellError(10,io);
			ClearWindow(FALSE,wScript);
			ForgetFileName(wScript);
			}
		}
	BPActivateWindow(SLOW,wScript);
	Alert1("Script syntax is now OK. Changes have been recorded");
	goto END;
	}
if(r != OK) {
	Alert1("Syntax errors have been reported in the 'Trace' window");
	BPActivateWindow(SLOW,wTrace);
	ShowSelect(CENTRE,wTrace);
	}
else {
	Alert1("Script syntax is OK.\n(Smooth execution is not certified)");
	BPActivateWindow(SLOW,wScript);
	}

END:
SwitchOff(NULL,wScriptDialog,bCheckScriptSyntax);
return(OK);
}


mMIDIfilter(int wind)
{

BPActivateWindow(SLOW,wFilter);
return(OK);
}
