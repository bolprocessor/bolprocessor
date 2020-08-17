/* Inits.c (BP2 version CVS) */

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

#if !BP_CARBON_GUI
#include "StringLists.h"
#endif


Inits(void)
{
int i,j,ch;
OSStatus io;
char **ptr;
Rect r;
long handlerRefcon;
#if BP_CARBON_GUI
WDPBRec pb;
AEEventHandlerUPP handler;
ProcessInfoRec info;
#endif /* BP_CARBON_GUI */
FSSpec spec;
long t;

static char HTMLlatin[] /* Starting with "&#32" up to "&#255" */
	= {' ','!','\"','#','$','%','&','\'','(',')','*','+',',','-','.','/','0','1','2','3',
	'4','5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H',
	'I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']',
	'\0','_','\0',
	'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t',
	'u','v','w','x','y','z','{','|','}','~','\0','•','™','≠','∞','≥','∑','∫','Ω',
	'√','≈','…','—','‘','Ÿ','⁄','∂','∆','Œ','‚','„','„','‰','','ˆ','˜',
	'˘','˙','˚','˛','ˇ','ı','ƒ','\0','¡','¢','£','€','¥','|','§','¨','©','ª','«',
	'¬','\0','®','\0','°','±','”','“','\0','µ','\0','·','˛','’','º','»','π','∏','≤','¿',
	'À','Á','Â','Ã','Ä','Å','Æ','Ç','È','É','Ê','Ë','Ì','Í','Î','Ï','\0','Ñ','Ò','Ó','Ô',
	'Õ','Ö','◊','Ø','Ù','Ú','Û','Ü','Y','\0','ß','à','á','â','ã','ä','å','æ','ç','è','é','ê',
	'ë','ì','í','î','ï','\0','ñ','ò','ó','ô','õ','ö','÷','ø','ù','ú','û','ü','y',
	'\0','ÿ'};

#if BP_CARBON_GUI
InitCursor();
#endif /* BP_CARBON_GUI */

#if USE_MLTE
io = TXNInitTextension(NULL, 0, 0);
if (io != noErr) {
	ParamText("\pBP could not initialize the Multilingual Text Engine library.  Quitting ...",
	          "\p", "\p", "\p");
	StopAlert(OKAlert,0L);
	return EXIT;
}
#endif

ForceTextColor = ForceGraphicColor = 0;

#if BP_CARBON_GUI
if(!GoodMachine()) return(ABORT);
FlushEvents(everyEvent,0);
#endif /* BP_CARBON_GUI */

////////////////////////////////////
////   Is this a beta version?  ////
#if	COMPILING_BETA
      Beta = YES;
#else
	Beta = NO;
#endif
////////////////////////////////////

// In this part we systematically initialise ALL global variables

Time_res = 10L; /* Time resolution for MIDI messages */
Quantization = 10L;
#if WITH_REAL_TIME_SCHEDULER
  TotalTicks = ZERO;
#endif

InBuiltDriverOn = FALSE;

Nw = 0;
 
InitOn = NoCursor = NotSaidKpress = TRUE;
ReceivedOpenAppEvent = FALSE;
CheckMem = TRUE; EmergencyExit = TempMemory = AskedTempMemory
	= FixedMaxQuantization = FALSE;
TempMemoryUsed = ZERO;
EventState = NO;
MemoryUsed = TempMemoryUsed = MaxMemoryUsed = ZERO;
SetTimeOn = ComputeOn = PolyOn = CompileOn = SoundOn = SelectOn = ButtonOn = ExpandOn
	= PrintOn = ClickRuleOn = GraphicOn = CompleteDecisions = LoadOn = SaveOn = MIDIfileOn
	= ReadKeyBoardOn = AlertOn = AllOn = HangOn = ScriptRecOn = PlayPrototypeOn
	= PlaySelectionOn = SelectPictureOn = TypeScript = InputOn = EnterOn = AEventOn
	= PauseOn = WaitOn = ItemOutPutOn = ItemCapture = TickCapture = TickCaptureStarted
	= AskedAboutCsound = MustChangeInput = ToldSkipped = ShownBufferSize = FALSE;
Option = TickDone = FoundNote = GotAlert = UsedRandom = SaidTooComplex = FALSE;
POLYconvert = OkShowExpand = FALSE;
NewOrchestra = TRUE;
Ratio = 0.;  Prod = 1.;
TimeMax = MAXTIME; Nalpha = 100L; SpeedRange = 6.;
CorrectionFactor = 1.;
UserName[0] = UserInstitution[0] = '\0';
// Tracefile = NULL;

Oms = FALSE;

#if WITH_REAL_TIME_SCHEDULER
// Space for time scheduler
if((p_Clock = (Slice***) NewHandle((Size)CLOCKSIZE * sizeof(Slice*))) == NULL)
	return(ABORT);
MyLock(TRUE,(Handle)p_Clock);
Clock = *p_Clock;
for(t=0; t < CLOCKSIZE; t++) Clock[t] = NULL;	/* No event in the queue */

if((p_AllSlices = (Slice**) NewHandle((Size)MAXTIMESLICES * sizeof(Slice))) == NULL)
	return(ABORT);
MyLock(TRUE,(Handle)p_AllSlices);
SlicePool = NULL;
for(i=ZERO; i < MAXTIMESLICES; i++) {
	(*p_AllSlices)[i].next = SlicePool;
	SlicePool = &((*p_AllSlices)[i]);
	}
#endif

for(i=0; i < MAXMESSAGE; i++) {
	ptr = (char**) GiveSpace((Size)(MAXLIN * sizeof(char)));
	if(ptr == NULL) return(ABORT);
	p_MessageMem[i] = ptr;
	(*p_MessageMem[i])[0] = '\0';
	}
Jmessage = 0;

if((p_TempFSspec=(FSSpec**) GiveSpace((Size)(WMAX * sizeof(FSSpec)))) == NULL)
	return(ABORT);

for(i=0; i < WMAX; i++) {
	ptr = (char**) GiveSpace((Size)(MAXINFOLENGTH * sizeof(char)));
	if(ptr == NULL) return(ABORT);
	p_FileInfo[i] = ptr;
	(*p_FileInfo[i])[0] = '\0';
	(*p_TempFSspec)[i].name[0] = 0;
	}

if(NEWTIMER) MaxMIDIbytes = MAXTIMESLICES - 50;
else MaxMIDIbytes = ZERO;

KeyboardType = QWERTY;
C4key = 60;  A4freq = 440.;
ProgNrFrom = 1;	/* This has changed with version 2.7.3 */
TestMIDIChannel = 1;
for(i=0; i <= MAXCHAN; i++) {
	CurrentMIDIprogram[i] = 0;
	}
ChangedMIDIprogram = FALSE;

NumInstalledDrivers = 0;
InstalledDrivers = NULL;
InstalledMenuItems = NULL;

Nbytes = Tbytes2 = ZERO;

MIDIinputFilter = MIDIinputFilterstartup = FILTER_ALL_ON;
MIDIoutputFilter = MIDIoutputFilterstartup = FILTER_ALL_OFF; // turn off Midi-thru by default - 061307 akozar
ResetMIDIFilter();

QuantizeOK = TRUE;
LapWait = ZERO;
PrefixTree.p = SuffixTree.p = NULL;
PrefixTree.accept = SuffixTree.accept = FALSE;
SmartCursor = Mute = Panic = ClockOverFlow = SchedulerIsActive = FALSE;
/*AlertMute = FALSE;*/

#if WITH_REAL_TIME_SCHEDULER
OKsend = FALSE;
#endif

// Limits of speed and scale values
TokenLimit = (((double)TOKBASE) * ((double)TOKBASE)) - 1.;
InvTokenLimit = 1. / TokenLimit;
MaxTempo = 100000.;
MaxFrac = 1000000.;
InvMaxTempo = 1. / MaxTempo;
//

Stream.code = NULL;
Stream.imax = ZERO; Stream.cyclic = FALSE; Stream.period = ZERO;

#if BP_CARBON_GUI
if(GetResource('MENU',MenuIDoffset) == NULL) {
	SysBeep(20);
	CantOpen();
	return(ABORT);
	}
GetResource('ICON',EditObjectsIconID);
GetResource('ICON',BP2iconID);
GetResource('PICT',MIDIKeyboardPictID);
GetResource('PICT',GreetingsPictID);
GetResource('DITL',GreetingsDitlID);
if(SetUpCursors() != OK) {
	SysBeep(20);
	CantOpen();
	return(ABORT);
	}
SetUpMenus();
InitColors();
#endif /* BP_CARBON_GUI */

if(LoadStrings() != OK) return(ABORT);

// Replaced NewHandle/DisposeHandle with GiveSpace/MyDisposeHandle in this section - akozar 20130910
if((p_Diacritical = (char****) GiveSpace((Size)(MAXHTMLDIACR) * sizeof(char**)))
	== NULL) return(ABORT);
MyLock(TRUE,(Handle)p_Diacritical);

if((p_HTMLdiacritical = (char**) GiveSpace((Size)(MAXHTMLDIACR) * sizeof(char)))
	== NULL) return(ABORT);
MyLock(TRUE,(Handle)p_HTMLdiacritical);

if((p_HTMLchar1 = (char**) GiveSpace((Size)(MAXHTMLDIACR) * sizeof(char)))
	== NULL) return(ABORT);
if((p_HTMLchar2 = (char**) GiveSpace((Size)256 * sizeof(char)))
	== NULL) return(ABORT);
	
for(i=0; i < MAXHTMLDIACR; i++) {
	if((ptr = (char**) GiveSpace((Size)((1 + MyHandleLen((*p_HTMLdiacrList)[i+i])) * sizeof(char))))
		== NULL) return(ABORT);
	(*p_Diacritical)[i] = ptr;
	MystrcpyHandleToHandle(0,&((*p_Diacritical)[i]),(*p_HTMLdiacrList)[i+i]);
	MyLock(TRUE,(Handle)(*p_Diacritical)[i]);
	(*p_HTMLchar1)[i] = (*((*p_HTMLdiacrList)[i+i+1]))[0];
	}
for(i=0; i < MAXHTMLDIACR; i++) {
	(*p_HTMLdiacritical)[i] = (*((*p_HTMLdiacrList)[i+i+1]))[0];
	}
MyDisposeHandle((Handle*)&p_HTMLdiacrList);

for(i=0; i < 32; i++) (*p_HTMLchar2)[i] = '\0';
for(i=32; i < 256; i++) (*p_HTMLchar2)[i] = HTMLlatin[i-32];

if(MakeWindows() != OK) return(ABORT);
// SetDialogFont(systemFont);
if(InitButtons() != OK) return(ABORT);
if(Beta) FlashInfo("This is a beta version for evaluation...");

#if NEWGRAF
if(!HasGWorlds()) {
	Alert1("Deep GWorlds not supported by this machine!");
	return(ABORT);
	}
else if(GWorldInit() != OK) return(ABORT);
Offscreen = TRUE;
#else
Offscreen = FALSE;
#endif
Nw = -1; Ndiagram = Npicture = 0;
LastEditWindow = OutputWindow = wData;
LastComputeWindow = wGrammar;
LastAction = NO;
Ke = log((double) 2.) / 64.;
strcpy(Message,"\0");
PictFrame.top = topDrawPrototype;
PictFrame.left = leftDrawPrototype;
PictFrame.bottom = bottomDrawPrototype;
PictFrame.right = rightDrawPrototype;
NoteScalePicture = NULL;
p_Tpict = NULL; Hpos = -1;
for(i=0; i < 4; i++) EndStr[i] = (char) 255;
Jbol = Jfunc = iProto = Jpatt = Jvar = Jflag = Jhomo = N_err = BolsInGrammar
	= ScriptExecOn = 0;
Jcontrol = -1;
for(i=0; i < MAXPARAMCTRL; i++) ParamControl[i] = ParamKey[i] = -1;
MaxRul = MaxGram = 0;
Gram.trueBP = Gram.hasTEMP = Gram.hasproc = FALSE;
pp_MIDIcode = NULL;
pp_CsoundTime = NULL;
p_Code = NULL;
pp_Comment = pp_CsoundScoreText = NULL;
p_CsoundSize = NULL;
pp_CsoundScore = NULL;
PointCsound = PointMIDI = FALSE;
CsFileName[0] = MIDIfileName[0] = CsoundOrchestraName[0] = '\0';
MIDIfadeOut = 2.;
MIDIfileOpened = FALSE;
MIDIfileReply = CsFileReply = NULL;
if((p_Oldvalue = (MIDIcontrolstatus**)
	GiveSpace((Size)MAXCHAN*sizeof(MIDIcontrolstatus))) == NULL) return(ABORT);
	
for(ch=0; ch < MAXCHAN; ch++) {
	(*p_Oldvalue)[ch].volume = -1;
	(*p_Oldvalue)[ch].panoramic = -1;
	(*p_Oldvalue)[ch].pressure = -1;
	(*p_Oldvalue)[ch].pitchbend = -1;
	(*p_Oldvalue)[ch].modulation = -1;
	}

// Variables for Csound instruments
p_CsInstrument = NULL;
p_CsInstrumentIndex = p_CsDilationRatioIndex = p_CsAttackVelocityIndex = p_CsReleaseVelocityIndex
	= p_CsPitchIndex = p_CsPitchBendStartIndex
	= p_CsVolumeStartIndex = p_CsPressureStartIndex = p_CsModulationStartIndex
	= p_CsPanoramicStartIndex = p_CsPitchBendEndIndex = p_CsVolumeEndIndex
	= p_CsPressureEndIndex = p_CsModulationEndIndex = p_CsPanoramicEndIndex
	= NULL;
p_CsPitchFormat = NULL;
pp_CsInstrumentName = pp_CsInstrumentComment = p_StringConstant = NULL;
p_NumberConstant = NULL;
#if BP_CARBON_GUI
FileSaveMode = ALLSAMEPROMPT;
FileWriteMode = LATER;
#else
FileSaveMode = ALLSAME;
FileWriteMode = NOW;
#endif /* BP_CARBON_GUI */
ConvertMIDItoCsound = FALSE;
MIDIfileType = 1;
CsoundFileFormat = MAC;

for(i=0; i < 6; i++) {
	p_CsPitchBend[i] = p_CsVolume[i] = p_CsPressure[i] = p_CsModulation[i]
		= p_CsPanoramic[i] = NULL;
	}
	
NoAlphabet = TRUE;
UseGraphicsColor = UseTextColor = TRUE;

StartFromOne = TRUE;
SetUpTime = 100L;	/* 100 ms */
NewEnvironment = NewColors = Help = FALSE;
for(i=0; i < WMAX; i++) ChangedCoordinates[i] = Dirty[i] = FALSE;
ObjectMode = ObjectTry = Final = LoadedScript = FALSE;
LoadedIn = LoadedGl = FALSE;
TransposeInput = FALSE; TransposeValue = 0;
CompiledGr = CompiledAl = CompiledPt = CompiledIn = CompiledGl = CompiledCsObjects
	= CompiledRegressions = NotFoundMetronom = NotFoundNatureTime = FALSE;
Pclock = Qclock = 1.;
Nature_of_time = STRIATED;
Pduration = 0.;  Qduration = 1.;
UseBufferLimit = FirstTime = FALSE;
OkWait = TRUE;
Nplay = 1;	/* Number of times each item is played */
SynchroSignal = OFF;
MaxDeriv = MAXDERIV;

Infneg = LONG_MIN;
Veryneg = Infneg + 1;
Infpos = - Veryneg;
Infpos1 = Infpos + 1.;

InsertGramRuleNumbers = InsertGramCorrections = InsertNewBols = (BP_CARBON_GUI ? TRUE : FALSE);
SplitTimeObjects = TRUE;	/* Terminal symbols separated by spaces */
SplitVariables = FALSE;	/* SplitVariables <=> variables displayed between '||' */
Token = SpaceOn = FALSE;	/* Typing text, not tokens */
FileName[iObjects][0] = '\0';
BufferSize = DeftBufferSize = MAXDISPL;
GraphicScaleP = GraphicScaleQ = 1L;
MatchWords = Finding = FALSE; IgnoreCase = TRUE;
strcpy(FindString,"\0"); strcpy(ReplaceString,"\0");
p_Image = p_NoteImage = NULL; p_Homo = NULL; p_Initbuff = NULL;
p_Bol = p_Patt = p_Var = p_Flagname = NULL;
p_Ppatt = p_Qpatt = NULL;
MaxScript = MAXEVENTSCRIPT; Jscriptline = 0;
p_Script = NULL;
InitThere = 0; JustCompiled = FALSE;
MIDIbytestate = MIDIfileTrackNumber = 0;
MidiLen_pos = ZERO;
MIDItracklength = Midi_msg = OldMIDIfileTime = ZERO;
LoadedCsoundInstruments = FALSE;

#if BP_CARBON_GUI
// Allow BP2 to respond to Apple Events coming from remote machines

io = AESetInteractionAllowed(kAEInteractWithAll);

// Installing Apple Event handlers. Don't forget to register in GoodEvent()
// (We don't worry about saving UPPs since we will never dispose of them).

handler = NewAEEventHandlerUPP(MyHandleOAPP);
io = AEInstallEventHandler(kCoreEventClass,kAEOpenApplication,handler,0,FALSE);

handler = NewAEEventHandlerUPP(MyHandleODOC);
io = AEInstallEventHandler(kCoreEventClass,kAEOpenDocuments,handler,0,FALSE);

handler = NewAEEventHandlerUPP(MyHandleODOC);
io = AEInstallEventHandler(kCoreEventClass,kAEPrintDocuments,handler,0,FALSE);	/* Print is identified by handler */

/*  = NewAEEventHandlerUPP(MyHandlePDOC);
io = AEInstallEventHandler(kCoreEventClass,kAEPrintDocuments,handler,0,FALSE); */

handler = NewAEEventHandlerUPP(MyHandleQUIT);
io = AEInstallEventHandler(kCoreEventClass,kAEQuitApplication,handler,0,FALSE);

#if !TARGET_API_MAC_CARBON	/* Edition Manager not in Carbon */
  handler = NewAEEventHandlerUPP(MyHandleSectionReadEvent);
  io = AEInstallEventHandler(sectionEventMsgClass,sectionReadMsgID,handler,0,FALSE);

  handler = NewAEEventHandlerUPP(MyHandleSectionWriteEvent);
  io = AEInstallEventHandler(sectionEventMsgClass,sectionWriteMsgID,handler,0,FALSE);

  handler = NewAEEventHandlerUPP(MyHandleSectionScrollEvent);
  io = AEInstallEventHandler(sectionEventMsgClass,sectionScrollMsgID,handler,0,FALSE);
#endif

handler = NewAEEventHandlerUPP(RemoteUseText);
io = AEInstallEventHandler(BP2Class,PlayEventID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteUseText);
io = AEInstallEventHandler(BP2Class,NameID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteUseText);
io = AEInstallEventHandler(BP2Class,GrammarID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteUseText);
io = AEInstallEventHandler(BP2Class,AlphabetID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteUseText);
io = AEInstallEventHandler(BP2Class,GlossaryID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteUseText);
io = AEInstallEventHandler(BP2Class,InteractionID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteUseText);
io = AEInstallEventHandler(BP2Class,DataID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteLoadCsoundInstruments);
io = AEInstallEventHandler(BP2Class,CsoundInstrID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteUseText);
io = AEInstallEventHandler(BP2Class,ScriptID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteControl);
io = AEInstallEventHandler(BP2Class,ImprovizeID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteControl);
io = AEInstallEventHandler(BP2Class,DoScriptID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteDoScriptLine);
io = AEInstallEventHandler(BP2Class,ScriptLineEventID,handler,0,FALSE);


handler = NewAEEventHandlerUPP(RemoteLoadSettings);
io = AEInstallEventHandler(BP2Class,LoadSettingsEventID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteSetConvention);
io = AEInstallEventHandler(BP2Class,NoteConventionEventID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteControl);
io = AEInstallEventHandler(BP2Class,BeepID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteControl);
io = AEInstallEventHandler(BP2Class,AbortID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteControl);
io = AEInstallEventHandler(BP2Class,AgainID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteControl);
io = AEInstallEventHandler(BP2Class,PauseID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteControl);
io = AEInstallEventHandler(BP2Class,QuickID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteControl);
io = AEInstallEventHandler(BP2Class,SkipID,handler,0,FALSE);

handler = NewAEEventHandlerUPP(RemoteControl);
io = AEInstallEventHandler(BP2Class,ResumeID,handler,0,FALSE);

/* io = AESetInteractionAllowed(kAEInteractWithAll); */  // duplicate call

/* Allocate scroll bar action UPPs once at init time to avoid leaks */
vScrollUPP = NewControlActionUPP(vScrollProc);
hScrollUPP = NewControlActionUPP(hScrollProc);
#endif /* BP_CARBON_GUI */

Maxitems = ZERO;
p_Flag = NULL;
p_MemGram = p_MemRul = p_VarStatus = NULL;
p_MemPos = p_LastStackIndex = NULL;
p_ItemStart = p_ItemEnd = NULL;
pp_Scrap = &p_Scrap;

for(i=0; i < MAXPICT; i++) {
	p_Picture[i] = NULL;
	PictureWindow[i] = -1;
	}

for(i=0; i < MAXDIAGRAM; i++) p_Diagram[i] = NULL;
p_StringList = NULL; pp_StringList = &p_StringList; NrStrings = 0;
if(ResetScriptQueue() != OK) return(ABORT);
p_InitScriptLine = NULL;
Maxinscript = 8; Jinscript = 0;
if((p_INscript=(INscripttype**) GiveSpace((Size) Maxinscript * sizeof(INscripttype)))
	== NULL) return(ABORT);
for(i=0; i < Maxinscript; i++) ((*p_INscript)[i]).chan = -1;

#if BP_CARBON_GUI
ResetPianoRollColors();
ShowPianoRoll = ToldAboutPianoRoll = FALSE;
ShowObjectGraph = TRUE;
#else
ShowPianoRoll = FALSE;
ToldAboutPianoRoll = TRUE;
ShowObjectGraph = FALSE;
#endif /* BP_CARBON_GUI */

p_Instance = NULL;
p_ObjectSpecs = NULL;

p_Seq = NULL;

if(MakeSoundObjectSpace() != OK) return(ABORT);

MaxVar = MaxFlag = 0;
Gram.p_subgram = GlossGram.p_subgram = NULL;
Gram.number_gram = GlossGram.number_gram = 0;
RunningStatus = 0; NoRepeat = FALSE;
ScriptSyncKey = ScriptSyncChan = -1;
StrikeAgainDefault = TRUE;
Jwheel = Jfeet = Jdisk = 0;
EmptyBeat = TRUE;

DeftPitchbendRange = 0;
DeftVolume = DEFTVOLUME;
DeftVelocity = DEFTVELOCITY;
DeftPanoramic = DEFTPANORAMIC;
PanoramicController = PANORAMICCONTROL;
VolumeController = VOLUMECONTROL;
SamplingRate = SAMPLINGRATE;
for(i=0; i <= MAXCHAN; i++) {
	CurrentVolume[i] = -1;
	PitchbendRange[i] = DeftPitchbendRange;
	VolumeControl[i] = VolumeController;
	PressRate[i] = PitchbendRate[i] = ModulationRate[i] = VolumeRate[i] = PanoramicRate[i]
		= SamplingRate;
	PanoramicControl[i] = PanoramicController;
	}
ForceRatio = -1.; PlayFromInsertionPoint = FALSE;

#if BP_CARBON_GUI
if((p_Token = (char****) GiveSpace((Size) 52 * sizeof(char**))) == NULL) return(ABORT);
for(i=0; i < 52; i++) (*p_Token)[i] = NULL;
ResetKeyboard(TRUE);

// Find current directory and volume
/* FIXME ? I think these values are always the same as GetCurrentProcess()
   returns below.  So, could remove this call and replace ParID/RefNumStartup
   with ParID/RefNumbp2 throughout entire program. - akozar */
pb.ioCompletion = NULL;
pb.ioNamePtr = NULL; // (StringPtr) DeftVolName;
io = PBHGetVol(&pb,(Boolean)FALSE);
LastDir = ParIDstartup = pb.ioWDDirID;
LastVref = RefNumStartUp = pb.ioWDVRefNum;

// Find the folder and volume of BP2 so that -se.startup and BP2 help may be located
io = GetCurrentProcess(&PSN);
info.processName = NULL;
info.processAppSpec = &spec;
info.processInfoLength = sizeof(ProcessInfoRec);
io = GetProcessInformation(&PSN,&info);
ParIDbp2 = info.processAppSpec->parID;
RefNumbp2 = info.processAppSpec->vRefNum;
#endif /* BP_CARBON_GUI */

OpenMIDIfilePtr = NULL;
HelpRefnum = TempRefnum = TraceRefnum = -1;
CsRefNum = -1; CsScoreOpened = MIDIfileTrackEmpty = FALSE;
for(i=0; i < WMAX; i++) {
	WindowParID[i] = ParIDstartup;
	TheVRefNum[i] = RefNumStartUp;
	}
FileName[wScript][0] = '\0';
CurrentChannel = 1;	/* Used for program changes,by default. */
Seed = 1;
ResetRandom();
#if BP_CARBON_GUI
SetSeed();
if(ResetInteraction() != OK) return(ABORT);
if(SetFindReplace() != OK) return(ABORT);
if(ResetPannel() != OK) return(ABORT);
#endif /* BP_CARBON_GUI */
if(SetNoteNames() != OK) return(ABORT);
NeedAlphabet = FALSE;
for(i=0; i < MAXCHAN; i++) {
	ChangedVolume[i] = ChangedPanoramic[i] = ChangedModulation[i] = ChangedPitchbend[i] = ChangedSwitch[i]
		= ChangedPressure[i] = FALSE;
	WhichCsoundInstrument[i+1] = -1;
	}

#if BP_CARBON_GUI
SwitchOn(NULL,wPrototype5,bPlayPrototypeTicks);
#endif /* BP_CARBON_GUI */
PrototypeTickChannel = 1; PrototypeTickKey = 84;
TickKey[0] = 96;
TickKey[1] = 84;
TickKey[2] = 72;
PrototypeTickVelocity = 64;
TickThere = PlayTicks = HideTicks = FALSE;
for(i=0; i < MAXTICKS; i++) {
	TickChannel[i] = 1;
	TickVelocity[i] = 64;
	TickCycle[i] = 4;
	if(Quantization < 50) TickDuration[i] = 50;
	else  TickDuration[i] = Quantization;
	NextTickDate[i] = ZERO;
	for(j=0; j < MAXBEATS; j++) ThisTick[i][j] = ZERO;
	ThisTick[0][0] = ThisTick[1][1] = ThisTick[1][2] = ThisTick[1][3] = 1L;
	Ptick[i] = Qtick[i] = 1L;
	MuteTick[i] = FALSE;
	SetTickParameters(i+1,MAXBEATS);
	}
SetTickParameters(0,MAXBEATS);
#if BP_CARBON_GUI
SetField(NULL,wTimeBase,fTimeBaseComment,"[Comment on time base]");
#endif /* BP_CARBON_GUI */
iTick = jTick = -1;
ResetTickFlag = TRUE; ResetTickInItemFlag = FALSE;
strcpy(Message,WindowName[wCsoundTables]);
#if BP_CARBON_GUI
SetWTitle(Window[wCsoundTables],in_place_c2pstr(Message));
#endif /* BP_CARBON_GUI */

MaxHandles = ZERO;
PedalOrigin = -1;
Nalpha = 100L;
Jinstr = 0;
NeedZouleb = 0;
UseBullet = TRUE;  Code[7] = '•';
if(ResizeCsoundInstrumentsSpace(1) != OK) return(ABORT);
iCsoundInstrument = 0;
ResetCsoundInstrument(iCsoundInstrument,YES,NO);
for(i=1; i <= MAXCHAN; i++) WhichCsoundInstrument[i] = -1; // FIXME: this is done above too?
#if BP_CARBON_GUI
SetCsoundInstrument(iCsoundInstrument,-1);
#endif /* BP_CARBON_GUI */
/* ClearWindow(TRUE,wCsoundInstruments); */
// ErrorSound(MySoundProc);
#if BP_CARBON_GUI
{int CheckDate();
if(!CheckDate()) return(ABORT);}
#endif /* BP_CARBON_GUI */
return(OK);
}


SetNoteNames(void)
{
int i,j,notenum,octave;
char **ptr;

for(i=0; i < MAXCONVENTIONS; i++) {
	if((p_NoteName[i] = (char****) GiveSpace((Size) 128 * sizeof(char**)))
		== NULL) return(ABORT);
	if((p_AltNoteName[i] = (char****) GiveSpace((Size) 128 * sizeof(char**)))
		== NULL) return(ABORT);
	if((p_NoteLength[i] = (int**) GiveSpace((Size) 128 * sizeof(int)))
		== NULL) return(ABORT);
	if((p_AltNoteLength[i] = (int**) GiveSpace((Size) 128 * sizeof(int)))
		== NULL) return(ABORT);
	for(j=0; j < 128; j++) {
		if((ptr=(char**) GiveSpace((Size) NOTESIZE * sizeof(char))) == NULL) return(ABORT);
		(*(p_NoteName[i]))[j] = ptr;
		if((ptr=(char**) GiveSpace((Size) NOTESIZE * sizeof(char))) == NULL) return(ABORT);
		(*(p_AltNoteName[i]))[j] = ptr;
		notenum = j % 12;
		octave = (j - notenum) / 12;
		switch(i) {
			case FRENCH:
				octave -= 2;
				switch(octave) {
					case -2:
						sprintf(Message,"%s000",Frenchnote[notenum]); break;
					case -1:
						sprintf(Message,"%s00",Frenchnote[notenum]); break;
					default:
						sprintf(Message,"%s%ld",Frenchnote[notenum],(long)octave);
						break;
					}
				break;
			case ENGLISH:
				octave--;
				switch(octave) {
					case -1:
						sprintf(Message,"%s00",Englishnote[notenum]); break;
					default:
						sprintf(Message,"%s%ld",Englishnote[notenum],(long)octave);
						break;
					}
				break;
			case INDIAN:
				octave--;
				switch(octave) {
					case -1:
						sprintf(Message,"%s00",Indiannote[notenum]); break;
					default:
						sprintf(Message,"%s%ld",Indiannote[notenum],(long)octave);
						break;
					}
				break;
			case KEYS:
				sprintf(Message,"%s%ld",KeyString,(long)j);
				break;
			}
		MystrcpyStringToTable(p_NoteName[i],j,Message);
		(*(p_NoteLength[i]))[j] = MyHandleLen((*(p_NoteName[i]))[j]);
		octave = (j - notenum) / 12;
		switch(i) {
			case FRENCH:
				octave -= 2;
				switch(octave) {
					case -2:
						sprintf(Message,"%s000",AltFrenchnote[notenum]); break;
					case -1:
						sprintf(Message,"%s00",AltFrenchnote[notenum]); break;
					default:
						sprintf(Message,"%s%ld",AltFrenchnote[notenum],(long)octave);
						break;
					}
				break;
			case ENGLISH:
				octave--;
				switch(octave) {
					case -1:
						sprintf(Message,"%s00",AltEnglishnote[notenum]); break;
					default:
						sprintf(Message,"%s%ld",AltEnglishnote[notenum],(long)octave);
						break;
					}
				break;
				break;
			case INDIAN:
				octave--;
				switch(octave) {
					case -1:
						sprintf(Message,"%s00",AltIndiannote[notenum]); break;
					default:
						sprintf(Message,"%s%ld",AltIndiannote[notenum],(long)octave);
						break;
					}
				break;
			case KEYS:
				sprintf(Message,"%s%ld",KeyString,(long)j);
				break;
			}
		MystrcpyStringToTable(p_AltNoteName[i],j,Message);
		(*(p_AltNoteLength[i]))[j] = MyHandleLen((*(p_AltNoteName[i]))[j]);
		}
	}
for(i=0; i < 12; i++) NameChoice[i] = 0;
#if BP_CARBON_GUI
return(SetNameChoice());
#else
return(OK);
#endif /* BP_CARBON_GUI */
}


Ctrlinit(void)
{
int i,igram,irul,j,k;
/* Print(wTrace,"\nCtrl values:\n"); */

if(Gram.p_subgram == NULL || Gram.number_gram < 1) return(OK);
for(i=1; i < MAXPARAMCTRL; i++) {
	k = ParamValue[i] = ParamInit[i];
	if(k == INT_MAX) continue;
	// FIXME ? Can we make this more efficient by resetting all ParamValue[],
	// then only looping over the grammar rules once, and if ctrl > 0 or
	// repeatcontrol > 0, then set w/weight or repeat to the appropriate 
	// ParamValue[ctrl or repeatcontrol] (if not INT_MAX)?
	for(igram=1; igram <= Gram.number_gram; igram++) {
		for(irul=0; irul <= (*(Gram.p_subgram))[igram]
										.number_rule; irul++) {
			if((*((*(Gram.p_subgram))[igram].p_rule))[irul].ctrl == i) {
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].w
					= (*((*(Gram.p_subgram))[igram].p_rule))[irul].weight = k;
				}
			if((*((*(Gram.p_subgram))[igram].p_rule))[irul].repeatcontrol == i) {
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].repeat = k;
				}
			}
		}
/*	sprintf(Message,"K%ld = %ld ",(long)i,(long)k);
	Print(wTrace,Message); */
	}
return(OK);
}

MakeWindows(void)
{
int i,id,im,ibot,itemtype,j,k,km,w,top,left,bottom,right,leftoffset,
	widmax,type,proc,bad,x0,y0, oldheight;
Handle h_res;
Rect r, rw;
Str255 title;
long rc;
GrafPtr saveport;
OSErr err;

#if BP_CARBON_GUI
DialogPtr* miscdialogs[] = { &ResumeStopPtr,&ResumeUndoStopPtr,&MIDIkeyboardPtr,
	&PatternPtr,&ReplaceCommandPtr,&EnterPtr,&FAQPtr,&SixteenPtr,&FileSavePreferencesPtr,
	&StrikeModePtr,&TuningPtr,&DefaultPerformanceValuesPtr,&CsoundInstrMorePtr,&MIDIprogramPtr };
Boolean miscdlgThemed[] = { 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0 };

ReplaceCommandPtr = GetNewDialog(ReplaceCommandID,NULL,0L); // could use kLastWindowOfClass instead of 0L
ResumeStopPtr = GetNewDialog(ResumeStopID,NULL,0L);
ResumeUndoStopPtr = GetNewDialog(ResumeUndoStopID,NULL,0L);
MIDIkeyboardPtr = GetNewDialog(MIDIkeyboardID,NULL,0L);
FileSavePreferencesPtr = GetNewDialog(FileSavePreferencesID,NULL,0L);
PatternPtr = GetNewDialog(PatternID,NULL,0L);
EnterPtr = GetNewDialog(EnterID,NULL,0L);
GreetingsPtr = GetNewDialog(GreetingsID,NULL,0L);
DrawDialog(GreetingsPtr);
#if TARGET_API_MAC_CARBON
  QDFlushPortBuffer(GetDialogPort(GreetingsPtr), NULL);
#endif
FAQPtr = GetNewDialog(FAQDialogID,NULL,0L);
SixteenPtr = GetNewDialog(SixteenDialogID,NULL,0L);
StrikeModePtr = GetNewDialog(StrikeModeID,NULL,0L);
TuningPtr = GetNewDialog(TuningID,NULL,0L);
DefaultPerformanceValuesPtr = GetNewDialog(DefaultID,NULL,0L);
CsoundInstrMorePtr = GetNewDialog(CsoundInstrMoreID,NULL,0L);
OMSinoutPtr = GetNewDialog(OMSinoutID,NULL,0L); // FIXME: should remove all OMS code - 20130826 akozar
MIDIprogramPtr = GetNewDialog(MIDIprogramID,NULL,0L);

for (i = 0; i < 14; ++i)  BPSetDialogAppearance(*(miscdialogs[i]), miscdlgThemed[i]);

#if BP_MACHO
  err = CreateCMSettings();
#endif

bad = FALSE;
#endif /* BP_CARBON_GUI */

/* This initialization needs to occur even in non-GUI builds ?? */
ResumeStopOn = FALSE;
Jbutt = 0;
for(w=0; w < WMAX; w++) {
	Window[w] = NULL;
	CurrentColor[w] = Black;
	IsHTML[w] = FALSE;
	WindowFullAlertLevel[w] = 0;
#if !BP_CARBON_GUI
	gpDialogs[w] = NULL;
	Weird[w] = FALSE;
	SetUpWindow(w);		// this is to create text buffers
#endif
	}
IsHTML[wCsoundTables] = TRUE;

#if BP_CARBON_GUI
for(w=0; w < MAXWIND; w++) {
	PleaseWait();
	if((Window[w] = GetNewCWindow(WindowIDoffset+w, NULL, 0L)) == NULL) {
		sprintf(Message,"Can't load resource window ID#%ld",
			(long)WindowIDoffset+w);
		EmergencyExit = TRUE;
		ParamText(in_place_c2pstr(Message),"\p","\p","\p");
		NoteAlert(OKAlert,0L);
		bad = TRUE;
		}
	// gpDialogs[w] = Window[w];  /* should probably not duplicate WindowPtrs as DialogPtrs, but may be neccessary for now -- 010907 akozar */
	/* By removing the above line, we invalidate any use of gpDialogs[w] where w < MAXWIND.
	   I am not sure if this will cause problems (even crashes), but it would also be an error
	   to try GetDialogFromWindow() on a non-Dialog WindowPtr.  011307 akozar */
	gpDialogs[w] = NULL; // FIXME ??
	if(!bad) SetUpWindow(w);
	GetWindowPortBounds(Window[w], &r);
	Weird[w] = FALSE;
	err = InvalWindowRect(Window[w], &r);
	if (err != noErr) Alert1("Err MakeWindows().  InvalWindowRect returned non-zero.");
	}

GetWindowPortBounds(Window[wMessage], &r);
AdjustWindow(FALSE,wMessage,r.top,r.left,r.bottom,r.right);
GetWindowPortBounds(Window[wInfo], &r);
AdjustWindow(FALSE,wInfo,r.top,r.left,r.bottom,r.right);
#endif /* BP_CARBON_GUI */

#if BP_CARBON_GUI
FlashInfo("Bol Processor Mac OS X (Carbon) GUI");
#else
FlashInfo("Bol Processor console app");
#endif

sprintf(Message,"%s",IDSTRING);
ShowMessage(TRUE,wMessage,Message);

#if BP_CARBON_GUI
SelectWindow(GetDialogWindow(GreetingsPtr));
SetPortDialogPort(GreetingsPtr);
TextSize(10); TextFont(kFontIDCourier);
RGBForeColor(&Red);
x0 = 253; y0 = 18;
CopyPString("\pInternational",title);
MoveTo(x0 - StringWidth(title)/2,y0);
DrawString(title);
y0 += 11;
CopyPString("\paward",title);
MoveTo(x0 - StringWidth(title)/2,y0);
DrawString(title);
y0 += 11;
CopyPString("\pBourges",title);
MoveTo(x0 - StringWidth(title)/2,y0);
DrawString(title);
y0 += 11;
CopyPString("\p1997",title);
MoveTo(x0 - StringWidth(title)/2,y0);
DrawString(title);
#if TARGET_API_MAC_CARBON
  QDFlushPortBuffer(GetDialogPort(GreetingsPtr), NULL);
#endif
	
for(w=MAXWIND; w < WMAX; w++) {
	PleaseWait();
	if((gpDialogs[w] = GetNewDialog(WindowIDoffset+w,NULL,0L)) == NULL) {
		sprintf(Message,"Can't load dialog window ID#%ld",
			(long)WindowIDoffset+w);
		EmergencyExit = TRUE;
		ParamText(in_place_c2pstr(Message),"\p","\p","\p");
		NoteAlert(OKAlert,0L);
		bad = TRUE;
		}
	if(bad) continue;
	Window[w] = GetDialogWindow(gpDialogs[w]);  /* should probably not duplicate DialogPtrs as WindowPtrs, but may be neccessary for now -- 010907 akozar */
	SetPortWindowPort(Window[w]);
	BPSetDialogAppearance(gpDialogs[w], WindowUsesThemeBkgd[w]);
	rc = GetWRefCon(GetDialogWindow(gpDialogs[w]));
	if(rc != 0L) {
		TextSize(WindowTextSize[w]);
		type = (int) (rc % 4L);
		switch(type) {
			case 1: proc = radioButProc; break;
			case 2: proc = pushButProc; break;
			case 3: proc = checkBoxProc; break;
			default:
				sprintf(Message,"Incorrect button type in window %ld",(long)w);
				EmergencyExit = TRUE;
				ParamText(in_place_c2pstr(Message),"\p","\p","\p");
				NoteAlert(OKAlert,0L);
				return(FAILED);
			}
		id = (int)((((rc % 256L) - type) / 4L) + DialogStringsBaseID); /* string list ID */
		h_res = GetResource('STR#',id);
		if((i=ResError()) != noErr) {
			sprintf(Message,
		"Error %ld loading resource string list for window %ld",(long)i,(long)w);
			EmergencyExit = TRUE;
			ParamText(in_place_c2pstr(Message),"\p","\p","\p");
			NoteAlert(OKAlert,0L);
			return(FAILED);
			}
		im = **((short**)h_res); // (*h_res)[1];
		if(im < 1) {
			sprintf(Message,
					"Error in resource string list for window %ld",(long)w);
			EmergencyExit = TRUE;
			ParamText(in_place_c2pstr(Message),"\p","\p","\p");
			NoteAlert(OKAlert,0L);
			return(FAILED);
			}
		if (RunningOnOSX)  { oldheight = Buttonheight; Buttonheight = 18; }
		GetPortBounds(GetDialogPort(gpDialogs[w]), &rw);
		top = (int)(((rc - (rc % 536870912L)) / 536870912L) * 64)+4;
		if(top == 0) top = rw.top;
		left = (int)((((rc - (rc % 67108864L)) / 67108864L) % 8) * 64)+4;
		if(left == 0) left = rw.left;
		bottom = (int)(((rc - (rc % 131072L)) / 131072L) % 512);
		if(bottom == 0) bottom = rw.bottom;
		right = (int)(((rc - (rc % 256L)) / 256L) % 512);
		if(right == 0) right = rw.right;
		ibot = (bottom - top) / Buttonheight;
		j = 2;
		widmax = 0;
		leftoffset = -Buttonheight;
		for(i=0; i < im; i++) {
			PleaseWait();
			km = (*h_res)[j]; /* length of P-string */
			if(km == 0) {
				sprintf(Message,
					"Error in resource string list for window %ld",(long)w);
				EmergencyExit = TRUE;
				ParamText(in_place_c2pstr(Message),"\p","\p","\p");
				NoteAlert(OKAlert,0L);
				return(FAILED);
				}
			for(k=0; k <= km; j++,k++) {
				title[k] = (*h_res)[j];
				}
			if((i % ibot) == 0) {
				leftoffset += Buttonheight + (widmax * Charstep);
				widmax = km;
				}
			else if(widmax < km) widmax = km;
			r.top = top + (i % ibot) * Buttonheight;
			r.left = left + leftoffset;
			r.bottom = r.top + Buttonheight;
			r.right = r.left + Buttonheight + (widmax * Charstep); 
			if(r.right > right) {
				sprintf(Message,
					"Can't put more than %ld buttons on window %ld",
						(long)i+1L,(long)w);
				EmergencyExit = TRUE;
				ParamText(in_place_c2pstr(Message),"\p","\p","\p");
				NoteAlert(OKAlert,0L);
				return(FAILED);
				}
			if((Jbutt+1) >= MAXBUTT) {
				EmergencyExit = TRUE;
				ParamText("\pIncrease MAXBUTT: too many buttons","\p","\p","\p");
				NoteAlert(OKAlert,0L);
				return(FAILED);
				}
			Hbutt[Jbutt++] = NewControl(Window[w],&r,title,(Boolean)1,
				(short)0,(short)0,(short)1,(short)(proc + 8),0L);
			}
		if (RunningOnOSX)  { Buttonheight = oldheight; }
		ReleaseResource(h_res);
		}
	SetUpWindow(w);
	}
if(bad) return(FAILED);
ClearWindow(TRUE,wMIDIorchestra);
#else
Jbutt = 28;  // number of buttons must not be less than that found within settings files
#endif /* BP_CARBON_GUI */

return(OK);
}

#if 0
HiliteDefault(DialogPtr dp)
{
short itemtype;
ControlHandle itemhandle;
Rect r;
UserItemUPP procForBorderUserItem;
GrafPtr	saveport;

/* procForBorderUserItem = NewUserItemProc(DrawButtonBorder); */

return(OK); /* $$$ */
if(dp == NULL || !IsWindowVisible(GetDialogWindow(dp))) return(FAILED);
GetDialogItem(dp,1,&itemtype,(Handle*)&itemhandle,&r);
itemtype = (itemtype & 127) - ctrlItem;
if(itemhandle != NULL && itemtype == btnCtrl) {
	GetPort(&saveport);
	SetPortDialogPort(dp);
	PenSize(3,3);
	InsetRect(&r,-2,-2);
	FrameRoundRect(&r,16,16);
/*	SetDialogItem(thewindow,1,itemtype,(Handle)procForBorderUserItem,&r); */
	if(saveport != NULL) SetPort(saveport);
	else if(Beta) Alert1("Err HiliteDefault(). saveport == NULL");
	}
return(OK);
}
#endif


#if !BP_CARBON_GUI
/* Non-Carbon version just creates TextHandles to use */
SetUpWindow(int w)
{	
	if(w < 0 || w >= WMAX) {
		Alert1("Internal problem in setting up text buffers!");
		return(ABORT);
	}
	
	if(Editable[w]) {
		TEH[w] = NewTextHandle();
		if (TEH[w] == NULL) return(ABORT);
	}
	else TEH[w] = NULL;
	Dirty[w] = FALSE;
	return(OK);
}

#else
SetUpWindow(int w)
{
Rect destRect,viewRect;
Rect scrollrect,r;
FontInfo myInfo;
int height,i,itemType;
long scrapoffset,n;
OSErr err;
#if WASTE
LongRect dr,vr;
#else
Rect dr,vr;
#endif

/* SetCursor(&WatchCursor); */
if(w < 0 || w >= WMAX) {
	Alert1("Internal problem in setting up windows. Restart your Mac!");
	return(ABORT);
	}
SetPortWindowPort(Window[w]);
if(Editable[w]) { TextFont(kFontIDCourier); TextSize(WindowTextSize[w]); }
// SetDialogFont(systemFont);  // too late for this; font will be systemFont anyways - akozar
Charstep = 7; /* StringWidth("\pm"); */
Nw = w;
GetWindowPortBounds(Window[w], &viewRect);
if (!USE_MLTE) {
if(OKvScroll[w]  || OKhScroll[w]) {
	viewRect.right = viewRect.right - SBARWIDTH;
	if (!RunningOnOSX || OKhScroll[w])  viewRect.bottom = viewRect.bottom - SBARWIDTH;
	GetWindowPortBounds(Window[w], &r);
	}
if(OKvScroll[w]) {
	scrollrect.left = r.right - SBARWIDTH;
	scrollrect.right = r.right + 1;
	scrollrect.bottom = r.bottom - (SBARWIDTH - 1) - Freebottom[w];
	scrollrect.top = r.top - 1;
	vScroll[w] = NewControl(Window[w],&scrollrect,"\p",(Boolean)1,(short)0,
		(short)0,(short)0,(short)scrollBarProc,0L);
	Vmin[w] = INT_MAX; Vmax[w] = - INT_MAX;
	Vzero[w] = 0;
	}
if(OKhScroll[w]) {
	scrollrect.left = r.left - 1;
	scrollrect.right = r.right - (SBARWIDTH - 1);
	scrollrect.bottom = r.bottom - Freebottom[w];
	scrollrect.top = r.bottom - SBARWIDTH - Freebottom[w];
	hScroll[w] = NewControl(Window[w],&scrollrect,"\p",(Boolean)1,(short)0,
		(short)0,(short)0,(short)scrollBarProc,0L);
	Hmin[w] = INT_MAX; Hmax[w] = - INT_MAX;
	Hzero[w] = 0;
	}
  }
viewRect.bottom -= Freebottom[w];
destRect = viewRect;
if(Editable[w]) {
#if WASTE
	dr.top = destRect.top;
	dr.left = destRect.left;
	dr.bottom = destRect.bottom;
	dr.right = destRect.right;
	vr.top = viewRect.top;
	vr.left = viewRect.left;
	vr.bottom = viewRect.bottom;
	vr.right = viewRect.right;
	err = WENew((const LongRect*)&dr,(const LongRect*)&vr,/* weFUseTempMem */ 0,
		&TEH[w]);
	if(err != noErr) {
		TellError(36,err); return(ABORT);
		}
	WEFeatureFlag(weFUndoSupport,weBitSet,TEH[w]);
	WEFeatureFlag(weFIntCutAndPaste,weBitClear,TEH[w]);
	WEFeatureFlag(weFDrawOffscreen,weBitSet,TEH[w]);
	WEFeatureFlag(weFInhibitRecal,weBitClear,TEH[w]);
	WEFeatureFlag(weFAutoScroll,weBitClear,TEH[w]);
	if(w != wInfo)
		WESetAlignment(weFlushLeft,TEH[w]);
	else
		WESetAlignment(weFlushRight,TEH[w]);
#elif USE_MLTE
	if (CreateMLTEObject(w, &viewRect) != OK) return (FAILED);
#else
	TEH[w] = TEStyleNew(&destRect,&viewRect);
	// StyleHandle[w] = TEGetStyleHandle(TEH[w]);
#endif
	CalText(TEH[w]);
	ForgetFileName(w);
	if(w != wInfo)
		Reformat(w,(int)kFontIDCourier,WindowTextSize[w],(int)normal,&Black,TRUE,TRUE);
	else
		Reformat(w,(int)kFontIDCourier,WindowTextSize[w],(int)normal,&Blue,TRUE,TRUE);
	}
else TEH[w] = NULL;
Dirty[w] = FALSE;
SetViewRect(w);
#if !WASTE
if(Editable[w]) PrintBehind(w," ");	/* Needed to record size into windowscrap */
#endif
return(OK);
}
#endif /* BP_CARBON_GUI */


#if USE_MLTE
int CreateMLTEObject(int w, Rect* frame)
{
	OSStatus err;
	TXNObject textObject;
	TXNFrameID id;
	TXNFrameOptions frameOptions = 0;

	// use Ptrs so we don't have to worry about locking, etc.
	TEH[w] = (OurMLTERecord**) NewHandle(sizeof(OurMLTERecord));
	if (MemError() != noErr)  return (FAILED);
	
	(*TEH[w])->textobj = NULL;
	(*TEH[w])->id = 0;
	(*TEH[w])->viewRect = *frame;
	
	if (OKgrow[w])     frameOptions |= kTXNDrawGrowIconMask;
	if (OKvScroll[w])  frameOptions |= kTXNWantVScrollBarMask;
	if (OKhScroll[w])  frameOptions |= kTXNWantHScrollBarMask;
	
	frameOptions |= kTXNAlwaysWrapAtViewEdgeMask;
	
	// we use MacOS text encoding, not kTXNSystemDefaultEncoding which defaults to Unicode
	// pass window index as the refcon
	err =TXNNewObject(NULL, Window[w], frame, frameOptions, kTXNTextEditStyleFrameType,	
				kTXNTextFile, kTXNMacOSEncoding, &textObject, &id, (TXNObjectRefcon)w);
	if (err != noErr)  return (FAILED);
	
	(*TEH[w])->textobj = textObject;
	(*TEH[w])->id = id;

	{ // this block hopefully turns off auto-handling of keyboard events
	TXNControlTag  iControlTags [] = {kTXNUseCarbonEvents};
	TXNControlData iControlData [1];
	TXNCarbonEventInfo carbonEventInfo;
	
	carbonEventInfo.useCarbonEvents = false;
	carbonEventInfo.filler = 0;
	carbonEventInfo.flags = 0;
	carbonEventInfo.fDictionary = NULL;
	
	iControlData [0 ].uValue = (UInt32)&carbonEventInfo;
	err =TXNSetTXNObjectControls(textObject, false, 1, iControlTags, iControlData);
	}
	
	return (OK);
}
#endif


LoadStrings(void)
{
long max;

p_GramProcedure = p_PerformanceControl = p_GeneralMIDIpatch = p_HTMLdiacrList = NULL;
p_ProcNdx = p_ProcNArg = p_PerfCtrlNdx = p_GeneralMIDIpatchNdx = p_PerfCtrlNArg = NULL;
MaxProc = MaxPerformanceControl = ZERO;

if(LoadStringResource(&p_GramProcedure,&p_ProcNdx,&p_ProcNArg,GramProcedureStringsID,
	&MaxProc,YES) != OK) return(ABORT);
MyLock(TRUE,(Handle)p_GramProcedure);
	
if(LoadStringResource(&p_PerformanceControl,&p_PerfCtrlNdx,&p_PerfCtrlNArg,
	PerformanceControlStringsID,&MaxPerformanceControl,YES) != OK) return(ABORT);
MyLock(TRUE,(Handle)p_PerformanceControl);
	
if(LoadStringResource(&p_GeneralMIDIpatch,&p_GeneralMIDIpatchNdx,NULL,
	GeneralMIDIpatchesID,&max,YES) != OK) return(ABORT);
MyLock(TRUE,(Handle)p_GeneralMIDIpatch);
	
if(LoadStringResource(&p_HTMLdiacrList,NULL,NULL,
	HTMLdiacriticalID,&max,YES) != OK) return(ABORT);
	
if(LoadScriptCommands(ScriptStringsID) != OK) return(ABORT);
return(OK);
}


LoadStringResource(char***** pp_str,int ***pp_ndx,int ***pp_narg,int id,long *p_max,
	int lock)
{
int i,im,j,j0,k,km;
char **ptr;
const char *buffer;
Handle h_res;

/* FIXME:  I would like to eventually convert the strings with multiple values in 
   StringLists.h to appropriate data structures and then all of this code could be
   dispensed with and accesses to all of these handles to arrays of string handles
   and ints can be replaced with straightforward references to the constant data.
   But for now, this will be easier (and less error-prone).
   -- akozar  20130908
 */

#if !BP_CARBON_GUI
const char (*strarray)[MAX_STRINGLISTS_STR_LEN];

// use the resource id to select the corresponding array
switch(id)
{
	case GramProcedureStringsID:
		strarray = GramProcedure;
		im = NUM_GRAM_PROC_STRS;
		break;
	case PerformanceControlStringsID:
		strarray = PerformanceControl;
		im = NUM_PERF_CONTROL_STRS;
		break;
	case GeneralMIDIpatchesID:
		strarray = GeneralMidiPatchName;
		im = NUM_GEN_MIDI_PATCH_STRS;
		break;
	case HTMLdiacriticalID:
		strarray = HTMLdiacritical;
		im = NUM_HTML_DIACRITICAL_STRS;
		break;
	default:
		if (Beta) fprintf(stderr, "Warning! Bad STR# id in LoadStringResource().\n");
		return FAILED;
		break;
}

// h_res must be non-NULL when it is "locked" below to avoid an error return code
h_res = (Handle) &buffer;	// WARNING! Dummy value -- don't use!
#else
h_res = GetResource('STR#',id);
if((i=ResError()) != noErr) {
	sprintf(Message,"Error %ld loading resource string list ID %ld",(long)i,(long)id);
	ParamText(in_place_c2pstr(Message),"\p","\p","\p");
	NoteAlert(OKAlert,0L);
	EmergencyExit = TRUE;
	return(FAILED);
	}
// resource begins with a two-byte integer which is the number of strings
im = **((short**)h_res);
buffer = (char*) *h_res;
#endif /* BP_CARBON_GUI */

*p_max = im;
	
if((*pp_str = (char****) GiveSpace((Size)im * sizeof(char**))) == NULL)
	return(ABORT);
if(pp_ndx != NULL) {
	if((*pp_ndx = (int**) GiveSpace((Size)im * sizeof(int))) == NULL)
		return(ABORT);
	}
if(pp_narg != NULL) {
	if((*pp_narg = (int**) GiveSpace((Size)im * sizeof(int))) == NULL)
		return(ABORT);
	}

/* In Carbon, the strings start at the 3rd byte (buffer[2]) and follow one after another
   without any padding (they are Pascal strings, so they begin with length bytes).
   Variable j keeps track of the current offset in buffer. */
for(i=0,j=1; i < im; i++) {
#if !BP_CARBON_GUI
	// In console build, we set buffer to each consecutive C string in strarray and
	// j to the non-existent "length byte" at position -1 (it will be incremented before use).
	buffer = strarray[i];
	j = -1;
	km = strlen(buffer);
#else
	km = (*h_res)[++j]; /* length of P-string */
#endif /* BP_CARBON_GUI */
	j0 = j;
	if(km == 0) goto ERR;
	if(pp_ndx != NULL) {
		j++;
		MyLock(FALSE,(Handle)h_res);
		k = GetInteger(NO,buffer,&j);
		if(k == INT_MAX) goto ERR;
		(**pp_ndx)[i] = k;
		MyUnlock((Handle)h_res);
		}
	if(pp_narg != NULL) {
		j++;
		MyLock(FALSE,(Handle)h_res);
		k = GetInteger(NO,buffer,&j);
		if(k == INT_MAX) goto ERR;
		(**pp_narg)[i] = k;
		MyUnlock((Handle)h_res);
		}
	km -= j - j0;
	
	ptr = (char**) GiveSpace((Size)(km+1) * sizeof(char));
	if(((**pp_str)[i] = ptr) == NULL) return(ABORT);
	if(lock) MyLock(TRUE,(Handle)ptr);
	for(k=0; k < km; k++) {
		(*((**pp_str)[i]))[k] = buffer[++j];
		}
	(*((**pp_str)[i]))[k] = '\0';
	}
#if BP_CARBON_GUI
ReleaseResource(h_res);
#endif /* BP_CARBON_GUI */
return(OK);

ERR:
sprintf(Message,"Error loading %ldth string in resource list ID %ld",
	(long)i,(long)id);
#if !BP_CARBON_GUI
fprintf(stderr, "%s\n", Message);
#else
ParamText(in_place_c2pstr(Message),"\p","\p","\p");
NoteAlert(OKAlert,0L);
#endif /* BP_CARBON_GUI */
EmergencyExit = TRUE;
return(FAILED);
}


LoadScriptCommands(int id)
{
int i,im,ilabel,iarg,j,k,km,kk,n,nmax,ic;
char c,**ptr;
Handle h_res;

/* FIXME ?  The list of script commands is only used in a few isolated places
   in code which is included in the console build.  It is possible though that
   by not initializing this list, those places will crash.  The most likely to
   cause problems are in CompileGlossary() and IsEmpty() which use the macro
   p_ScriptLabelPart().  (DisplayGrammar() does also but it currently is not 
   called anywhere).
 */

#if BP_CARBON_GUI
h_res = GetResource('STR#',id);
if((i=ResError()) != noErr) {
	sprintf(Message,"Error %ld loading resource string list ID %ld",(long) i,
		(long)id);
ERR1:
	ParamText(in_place_c2pstr(Message),"\p","\p","\p");
	NoteAlert(OKAlert,0L);
	EmergencyExit = TRUE;
	return(FAILED);
	}
j = 1; nmax = 0;
im = **((short**)h_res); // (*h_res)[j]; if(im < 0) im += 256;
MaxScriptInstructions = im;
if(im < 1 || im > 255) {
	sprintf(Message,"Error im=%ld loading resource string list ID %ld",
		(long)im,(long)id);
	goto ERR1;
	}
if((h_Script = (scriptcommandtype**) GiveSpace((Size)im * sizeof(scriptcommandtype))) == NULL)
	goto ERR2;
if((h_ScriptIndex = (int**) GiveSpace((Size)im * sizeof(int))) == NULL)
	goto ERR2;
for(i=0; i < im; i++) {
	(*h_ScriptIndex)[i] = i;
	ScriptLabel(i) = ScriptArg(i) = NULL;
	ScriptNrLabel(i) = ScriptNrArg(i) = 0;
	}
for(i=0; i < im; i++) {
	PleaseWait();
	km = (*h_res)[++j]; /* length of P-string */
	if(km == 0) goto ERR;
	j++; km += j;
	MyLock(FALSE,(Handle)h_res);
	k = GetInteger(NO,(char*)*h_res,&j);
	if(k == INT_MAX) goto ERR;
	if(k >= MaxScriptInstructions) goto ERR;
	(*h_ScriptIndex)[i] = ic = k;
	MyUnlock((Handle)h_res);
	
#if BP_CARBON_GUI
	/* Insert script command into script menu */
	for(k=j+1; k < km; k++) PascalLine[k-j] = (*h_res)[k];
	PascalLine[0] = km - j - 1;
	AppendMenu(myMenus[scriptM],PascalLine);
#endif /* BP_CARBON_GUI */
	
	ilabel = iarg = n = 0;
	for(k=j; k < km; k++) {
		if((*h_res)[k] == '«') n++;
		}
	if(n > nmax) nmax = n;
	if((ScriptLabel(ic) = (char****) GiveSpace((Size)(n+1) * sizeof(char**)))
		== NULL) goto ERR2;
	if(n > 0 && ((ScriptArg(ic) = (char****) GiveSpace((Size)(n) * sizeof(char**)))
		== NULL)) goto ERR2;
	
NEWLABELPART:
	while((c=(*h_res)[j]) == ' ' || c == '»') j++;
	if(j >= km) {
		j = km - 1; continue;
		}
	k = j; while((c=(*h_res)[k]) != '«' && k < km) k++;
	k--; while((c=(*h_res)[k]) == ' ') k--; k++;
	ptr = (char**) GiveSpace((Size)(k-j+2) * sizeof(char));
	if((p_ScriptLabelPart(ic,ilabel) = ptr) == NULL) goto ERR2;
	k -= j;
	for(kk=0; kk < k; j++,kk++) {
		c = (*h_res)[j];
		(*(p_ScriptLabelPart(ic,ilabel)))[kk] = Filter(c);
		}
	(*(p_ScriptLabelPart(ic,ilabel)))[kk] = '\0';
	ScriptNrLabel(ic) = ++ilabel;

NEWARGPART:	
	while((c=(*h_res)[j]) == ' ' || c == '«') j++;
	if(j >= km) {
		j = km - 1; continue;
		}
	k = j; while((c=(*h_res)[k]) != '»' && k < km) k++;
	ptr = (char**) GiveSpace((Size)(k-j+1) * sizeof(char));
	if((p_ScriptArgPart(ic,iarg) = ptr) == NULL) goto ERR2;
	k -= j;
	for(kk=0; kk < k; j++,kk++) {
		c = (*h_res)[j];
		(*(p_ScriptArgPart(ic,iarg)))[kk] = Filter(c);
		}
	(*(p_ScriptArgPart(ic,iarg)))[kk] = '\0';
	ScriptNrArg(ic) = ++iarg;
	goto NEWLABELPART;
	}
ReleaseResource(h_res);
nmax++;
#else
nmax = 8;	// this is the value BP 2.9.8 calculates
#endif /* BP_CARBON_GUI */

// need to allocate this space in console build as many functions try to append script lines
if((ScriptLine.label=(char****) GiveSpace((Size)(nmax) * sizeof(char**))) == NULL)
	goto ERR2;
if((ScriptLine.arg=(char****) GiveSpace((Size)(nmax) * sizeof(char**))) == NULL)
	goto ERR2;
if((ScriptLine.intarg=(long**) GiveSpace((Size)(nmax) * sizeof(long))) == NULL)
	goto ERR2;
if((ScriptLine.floatarg=(double**) GiveSpace((Size)(nmax) * sizeof(double))) == NULL)
	goto ERR2;
if((ScriptLine.ularg=(unsigned long**) GiveSpace((Size)(nmax) * sizeof(unsigned long))) == NULL)
	goto ERR2;
for(i=0; i < nmax; i++) {
	if((ptr=(char**) GiveSpace((Size)MAXLIN * sizeof(char))) == NULL) goto ERR2;
	(*(ScriptLine.label))[i] = ptr;
	if((ptr=(char**) GiveSpace((Size)MAXLIN * sizeof(char))) == NULL) goto ERR2;
	(*(ScriptLine.arg))[i] = ptr;
	}
return(OK);

ERR:
sprintf(Message,"Error loading %ldth string in resource list ID %ld",
	(long)i,(long)id);
ERR3:
#if !BP_CARBON_GUI
fprintf(stderr, "%s\n", Message);
#else
ParamText(in_place_c2pstr(Message),"\p","\p","\p");
NoteAlert(OKAlert,0L);
#endif /* BP_CARBON_GUI */
EmergencyExit = TRUE;
return(FAILED);

ERR2:
sprintf(Message,"Error string resource list ID %ld. Insufficient memory",(long)id);
goto ERR3;
}


#if BP_CARBON_GUI
SetUpCursors(void)
{
CursHandle	hCurs;
int i;

hCurs = GetCursor(iBeamCursor);
EditCursor = **hCurs;

hCurs = GetCursor(watchCursor);
WatchCursor = **hCurs;

hCurs = GetCursor(crossCursor);
CrossCursor = **hCurs;

if((hCurs=GetCursor(KeyboardID)) == NULL)  return(ABORT);
KeyboardCursor = **hCurs;

if((hCurs=GetCursor(QuestionID)) == NULL) return(ABORT);
HelpCursor = **hCurs;

for(i=0; i < 4; i++) {
	if((hCurs = GetCursor(WheelID+i)) == NULL) return(ABORT);
	WheelCursor[i] = **hCurs;
	}
for(i=0; i < 8; i++) {
	if((hCurs = GetCursor(FeetID+i)) == NULL) return(ABORT);
	FootCursor[i] = **hCurs;
	}
for(i=0; i < 2; i++) {
	if((hCurs = GetCursor(DiskID+i)) == NULL) return(ABORT);
	DiskCursor[i] = **hCurs;
	}
NoCursor = FALSE;
return(OK);
}
#endif /* BP_CARBON_GUI */


InitButtons(void)
{
#if WITH_REAL_TIME_MIDI && BP_CARBON_GUI
  OutMIDI = TRUE;
#else
  OutMIDI = FALSE;
#endif
ObjectMode = ObjectTry = Improvize = StepProduce = StepGrammars
	= PlanProduce = DisplayProduce = UseEachSub
	= TraceProduce = DisplayTimeSet = StepTimeSet = TraceTimeSet
	= ShowGraphic = ComputeWhilePlay = NeverResetWeights = FALSE;
SynchronizeStart = CyclicPlay = NoConstraint = AllItems
	= WriteMIDIfile = Interactive = OutCsound = CsoundTrace = WillRandomize = FALSE;
ResetWeights = ResetFlags = ResetControllers = DisplayItems = ShowMessages
	= AllowRandomize = TRUE;
NoteConvention = ENGLISH;
SetButtons(TRUE);
return(OK);
}


#if BP_CARBON_GUI

ResetPannel(void)
{
HidePannel(wControlPannel,dDeriveFurther);
HidePannel(wControlPannel,dSaveDecisions);
HidePannel(wControlPannel,dShowGramWeights);
HidePannel(wControlPannel,dAnalyze);
/* HidePannel(wControlPannel,dLoadWeights);
HidePannel(wControlPannel,dLearnWeights);
HidePannel(wControlPannel,dSaveWeights);
HidePannel(wControlPannel,dSetWeights); */
ShowPannel(wControlPannel,dTemplates);
return(OK);
}


GoodMachine(void)
{
char *processor[] = {
	"mc68000",
	"mc68010",
	"mc68020",
	"mc68030",
	"mc68040"
	};
char *fpu[] = {
	"<none>",
	"mc68881",
	"mc68882",
	"mc68040 built-in"
	};
long gestaltAnswer;
OSErr gestaltErr;
short depth;

/* Determine whether we can use Gestalt or not,and if so,what version */
gestaltErr = Gestalt(gestaltVersion,&gestaltAnswer);

if(!gestaltErr) {
	/* Check the processor type */
	Gestalt('cput',&gestaltAnswer);
#ifdef __POWERPC
	if(gestaltAnswer < 0x101) {
		ParamText("\pThis version of BP2 runs only on a PowerMac. Use the 68k version instead.", "\p", "\p", "\p");
		StopAlert(OKAlert, NULL);
		return(FAILED);
		}
#endif
	if(gestaltAnswer < 0x002) {
		sprintf(Message,"BP2 requires at least a 68020 processor.\nThis machine has a %s processor",
			processor[gestaltAnswer]);
		ParamText(in_place_c2pstr(Message), "\p", "\p", "\p");
		StopAlert(OKAlert, NULL);
		return(FAILED);
		}
	
	/* Determine the coprocessor type -- useless */
	Gestalt(gestaltFPUType,&gestaltAnswer);
	
	/* Determine system version */
	Gestalt(gestaltSystemVersion,&gestaltAnswer);
	if(gestaltAnswer < 0x00000701) {
		ParamText("\pSystem version is too old. BP2 requires at least MacOS version 7.1", "\p", "\p", "\p");
		StopAlert(OKAlert, NULL);
		return(FAILED);
		}
	/* Check color possibility */
	Gestalt(gestaltQuickdrawVersion,&gestaltAnswer);
	if(gestaltAnswer < 0x100
			|| ((depth=GetDepth(GetMainDevice())) < 4)) {
		/* Color QuickDraw not there or pixel depth insufficient */
		/* HasDepth() might be a better solution $$$ */
		ForceTextColor = ForceGraphicColor = -1;
		}
	
	/* Check for Appearance Manager */
	Gestalt(gestaltAppearanceAttr, &gestaltAnswer);
	if (gestaltAnswer & (1 << gestaltAppearanceExists))  HaveAppearanceManager = TRUE;
	else  HaveAppearanceManager = FALSE;

	/* Check for running on MacOS X */
	Gestalt(gestaltMenuMgrAttr,&gestaltAnswer);
	if(gestaltAnswer & gestaltMenuMgrAquaLayoutMask)  RunningOnOSX = TRUE;
	else  RunningOnOSX = FALSE;
	
	return(OK);
	}
ParamText("\pSystem version is too old. BP2 requires at least System 7.1", "\p", "\p", "\p");
StopAlert(OKAlert, NULL);
return(FAILED);
}


/* The first element of iconlist should be the number of sequential items
   in the menu to set icons for, and the rest of the array should be the
   resource IDs for the icon families of the first, second, etc. menu items. 
   You can set an array element to 0 to skip that menu item. */
int BPSetMenuItemIcons(MenuHandle menu, ResID iconIDs[])
{
	Handle iconsuite;
	OSErr  err;
	int    i;

	// SetMenuItemIconHandle() only available with Appearance Mgr
	if (!HaveAppearanceManager)  return (FAILED);

	for (i = 1; i <= iconIDs[0]; ++i) {
		if (iconIDs[i] > 0) {
			err = GetIconSuite(&iconsuite, iconIDs[i], kSelectorAllSmallData);
			if (err == noErr)
				err =  SetMenuItemIconHandle(menu, i, kMenuIconSuiteType, iconsuite);
		}
	}
	
	return (OK);
}


SetUpMenus(void)
{
int	i;

ClearMenuBar();
myMenus[appleM] = GetMenu(MenuIDoffset);
InsertMenu(myMenus[appleM],0) ;
#if !TARGET_API_MAC_CARBON
  AppendResMenu(myMenus[appleM],'DRVR');
#endif

for (i = fileM; i <= MAXMENU; i++) {
	/* Also loading 'Script' menu */
	myMenus[i] = GetMenu(MenuIDoffset + i);
	InsertMenu(myMenus[i],0);
	}
/* On OS X, remove the Quit item and separator from the File menu */ 
#if TARGET_API_MAC_CARBON
  if (RunningOnOSX && myMenus[fileM] != NULL) {
  	DeleteMenuItem(myMenus[fileM], fmQuit);
  	DeleteMenuItem(myMenus[fileM], fmQuit - 1);
  	}
#endif

if (!HaveAppearanceManager) {
	sprintf(Message,"Find again %c-option A",(char) commandMark);
	c2pstrcpy(PascalLine, Message);
	SetMenuItemText(myMenus[searchM],findagainCommand,PascalLine);

	sprintf(Message,"Enter and find %c-option E",(char) commandMark);
	c2pstrcpy(PascalLine, Message);
	SetMenuItemText(myMenus[searchM],enterfindCommand,PascalLine);
	}
else {  // use Appearance Mgr features to set option-key shortcuts
	SetItemCmd(myMenus[searchM],enterfindCommand,'E');
	SetMenuItemModifiers(myMenus[searchM],enterfindCommand,kMenuOptionModifier);
	SetItemCmd(myMenus[searchM],findagainCommand,'A');
	SetMenuItemModifiers(myMenus[searchM],findagainCommand,kMenuOptionModifier);
	}
EnableMenuItem(myMenus[searchM],enterfindCommand);

if (!HaveAppearanceManager) {
	sprintf(Message,"Type tokens [toggle] %c-option T",(char) commandMark);
	c2pstrcpy(PascalLine, Message);
	SetMenuItemText(myMenus[editM],tokenCommand,PascalLine);
	}
else {  // use Appearance Mgr features to set option-key shortcuts
	SetItemCmd(myMenus[editM],tokenCommand,'T');
	SetMenuItemModifiers(myMenus[editM],tokenCommand,kMenuOptionModifier);
	}
EnableMenuItem(myMenus[editM],tokenCommand);

/*sprintf(Message,"Help                       %c-?",(char) commandMark);
c2pstrcpy(PascalLine, Message);
SetMenuItemText(myMenus[actionM],helpCommand,PascalLine);
EnableMenuItem(myMenus[actionM],helpCommand);*/

BPSetMenuItemIcons(myMenus[windowM], WindowMenuIcons);
BPSetMenuItemIcons(myMenus[deviceM], DeviceMenuIcons);
DrawMenuBar();
return(OK);
}


AdjustWindow(int newplace,int w,int top,int left,int bottom,int right)
{
int vdrag,hdrag,wresize,hresize,height,width,screenwidth,screenheight,d;
Rect r; Point p,q;
GrafPtr saveport;
BitMap  screenBits;

if(w < 0 || w >= WMAX
		|| (!Adjustable[w] && w != wMessage && w != wInfo)) return(OK);
if(ScriptExecOn || InitOn) PleaseWait();

GetWindowPortBounds(Window[w], &r);

GetPort(&saveport);
SetPortWindowPort(Window[w]);

p = topLeft(r); LocalToGlobal(&p);
q = botRight(r); LocalToGlobal(&q);

/* First readjust coordinates so that window will not disappear */
GetQDGlobalsScreenBits(&screenBits);
screenwidth = screenBits.bounds.right - screenBits.bounds.left;
screenheight = screenBits.bounds.bottom - screenBits.bounds.top;

if(w == wMessage || w == wInfo) {
	height = 12;
	if(w == wMessage) bottom = screenheight;
	else bottom = screenheight - height - 1;
	top = bottom - height;
	right = screenwidth;
	if(w == wMessage) left = 0;
	else left = screenwidth / 3;
	}
else if(!newplace) {
	top = p.v; left = p.h; bottom = q.v; right = q.h;
	}

if(!OKgrow[w] && w != wMessage && w != wInfo) {
	d = (q.h - p.h) - (right - left);
	right += d;
	d = (q.v - p.v) - (bottom - top);
	bottom += d;
	}
	
d = bottom - screenheight;
if(d > 0) {
	bottom = screenheight;
	top = top - d;
	if(top < (2 * GetMBarHeight() + 4)) top = (2 * GetMBarHeight() + 4);
	}
	
d = right - screenwidth;
if(d > 0) {
	right = screenwidth;
	left = left - d;
	if(left < 0) left = 0;
	}

vdrag = top - p.v;	/* top */
hdrag = left - p.h;	/* left */
wresize = (right - left) - (q.h - p.v);
hresize = (bottom - top) - (q.v - p.h);

if(Editable[w] && !LockedWindow[w]) Deactivate(TEH[w]);

if((vdrag != 0) || (hdrag != 0)) {
	MoveWindow(Window[w],left,top,FALSE);	/* Don't activate */
	}
if((OKgrow[w] || w == wMessage || w == wInfo) && ((wresize != 0) || (hresize != 0))) {
	SizeWindow(Window[w],right - left,bottom - top,TRUE);	/* Update */
	AdjustWindowContents(w);
	}
if(w == wScript && ScriptExecOn) {
	BPActivateWindow(SLOW,w);
	Activate(TEH[w]);
	}
	
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err AdjustWindow(). saveport == NULL");
return(OK);
}


Boolean HasGWorlds(void)
{
long qdResponse,mask;
OSErr err;

err = Gestalt(gestaltQuickdrawFeatures,&qdResponse );

if(err != noErr) {
	Alert1("Error calling Gestalt");
	return false;
	}

mask = 1 << gestaltHasDeepGWorlds;
	
if(qdResponse & mask)
	return true;
else
	return false;
}


GWorldInit(void)
{
OSErr	err;
Rect	r;
WindowPtr	window;	

window = Window[wGraphic];
GetWindowPortBounds(window, &r);
OffsetRect(&r,-r.left,-r.top);

err = NewGWorld(&gMainGWorld,16,&r,nil,nil,pixPurge);

if(err != noErr) {
	Alert1("Error calling NewGWorld");
	return(ABORT);
	}
return(OK);
}


short GetDepth(GDHandle gdevice)
{
short depth = 0;
if(gdevice && (*gdevice)->gdPMap) depth = (*((*gdevice)->gdPMap))->pixelSize;
return(depth);
}

#endif /* BP_CARBON_GUI */

#if 0
CheckRegistration(void)
{
int result,iv;
short type,refnum;
OSErr io;
FSSpec spec;
unsigned long today,secs;
FInfo fndrinfo;
NSWReply reply;
char **p_line,**p_completeline;
long pos;

p_line = p_completeline = NULL;
c2pstrcpy(spec.name, "_bp2_key");
spec.vRefNum = RefNumbp2;
spec.parID = ParIDbp2;
io = FSpGetFInfo(&spec,&fndrinfo);
if(io == noErr) {
	if(!(fndrinfo.fdFlags & fInvisible)) {
		fndrinfo.fdFlags |= fInvisible;
		io = FSpSetFInfo(&spec,&fndrinfo);
		io = FSDelete("\p_bp2_startdate",0);
		}
	MyOpen(&spec,fsRdPerm,&refnum);
	pos = ZERO;
	if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ENTERNAME;
	if(CheckVersion(&iv,p_line,"'\0'") != OK) goto ENTERNAME;
	if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ENTERNAME;
	if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ENTERNAME;
	MystrcpyHandleToString(MAXNAME,0,UserName,p_completeline);
	Strip(UserName);
	if(UserName[0] == '\0' || strcmp(UserName,"<unknown>") == 0) goto ENTERNAME;
	if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ENTERNAME;
	MystrcpyHandleToString(MAXNAME,0,UserInstitution,p_completeline);
	CloseMe(&refnum);
	sprintf(Message,"Bonjour %s!",UserName);
	FlashInfo(Message);
	goto OUT;
	
ENTERNAME:
	CloseMe(&refnum);
	if(MakeNewKeyFile(TRUE) == OK) io = FSDelete("\p_bp2_startdate",0);
	}
else {
	GetDateTime(&today);
	c2pstrcpy(spec.name, "_bp2_startdate");
	spec.vRefNum = RefNumbp2;
	spec.parID = ParIDbp2;
	type = 0;
	io = FSpGetFInfo(&spec,&fndrinfo);
	if(io == noErr) {
		MyOpen(&spec,fsRdPerm,&refnum);
		pos = ZERO;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
		if(CheckVersion(&iv,p_line,"'\0'") != OK) goto ERR;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
		if(ReadUnsignedLong(refnum,&secs,&pos) == FAILED) goto ERR;
		CloseMe(&refnum);
//		if(Beta) goto MAKE;
		if((today-secs) > (86400L * 30L)) {
			Alert1("This copy of BP2 has been used for more than 30 days.\nIt's a good idea to click the 'Register' button...");
			goto REGISTERED;
			}
		goto OUT;
ERR:
		CloseMe(&refnum);
		goto MAKE;
		}
	else {
MAKE:
		io = FSDelete("\p_bp2_startdate",0);
		err = NSWInitReply(&reply);	
		reply.sfFile.vRefNum = RefNumbp2;
		reply.sfFile.parID = ParIDbp2;
		reply.sfReplacing = FALSE;
		CopyPString("\p_bp2_startdate",PascalLine);
		CopyPString(PascalLine,reply.sfFile.name);
		result = CreateFile(-1,-1,1,PascalLine,&reply,&refnum);
		if(result != OK) {
			Alert1("Unexpected problem creating the registration file.  Is the hard disk full?\nContact the authors");
			goto OUT;
			}
		WriteHeader(-1,refnum,reply.sfFile);
		sprintf(Message,"%.0f",(double)today);
		WriteToFile(NO,MAC,Message,refnum);
		CloseMe(&refnum);
		spec = reply.sfFile;
		io = FSpGetFInfo(&spec,&fndrinfo);
		fndrinfo.fdFlags |= fInvisible;
		io = FSpSetFInfo(&spec,&fndrinfo);
		Alert1("This is a fresh copy of BP2.\nIt's a good idea to click the 'Register' button...");
REGISTERED:
		Alert1("If you already registered, contact <bel@kagi.com> to get an activation key");
		}
	}
OUT:
MyDisposeHandle((Handle*)&p_line);
MyDisposeHandle((Handle*)&p_completeline);
return(OK);
}


MakeNewKeyFile(int formyself)
{
int result;
short type,refnum;
OSErr io;
FSSpec spec;
FInfo fndrinfo;
NSWReply reply;
char line[MAXFIELDCONTENT];
Rect r;
Handle itemhandle;
short item,itemtype;
long pos;
Str255 t;
DialogPtr enternameptr;

enternameptr = GetNewDialog(EnterNameID,0L,0L);

GetDialogItem(enternameptr,fUserName,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,"\p");
GetDialogItem(enternameptr,fInstitution,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,"\p");

TRYENTER:
ShowWindow(enternameptr);
SelectWindow(enternameptr);
result = FAILED;
while(TRUE) {
	MaintainCursor();
	ModalDialog((ModalFilterUPP) 0L,&item);
	switch(item) {
		case dNameOK:
			result = OK; break;
		case fUserName:
			break;
		case fInstitution:
			break;
		}
	if(result == OK) break;
	}

GetDialogItem(enternameptr,fUserName,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,line);
Strip(line);
if(formyself && line[0] == '\0') {
	Alert1("You must enter a user name");
	goto TRYENTER;
	}
if(strlen(line) >= MAXNAME) {
	sprintf(Message,"Name can't be more than %ld chars",(long)(MAXNAME-1L));
	Alert1(Message);
	goto TRYENTER;
	}
strcpy(UserName,line);
if(UserName[0] == '\0') sprintf(UserName,"<unknown>");

GetDialogItem(enternameptr,fInstitution,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,line);
Strip(line);
if(strlen(line) >= MAXNAME) {
	sprintf(Message,"Institution can't be more than %ld chars",(long)(MAXNAME-1L));
	Alert1(Message);
	goto TRYENTER;
	}
strcpy(UserInstitution,line);
if(UserInstitution[0] == '\0') sprintf(UserInstitution,"<unknown>");

if(formyself) {
	sprintf(Message,"Your name is: %s",UserName);
	if(Answer(Message,'Y') != YES) goto TRYENTER;
	sprintf(Message,"Your institution is: %s",UserInstitution);
	if(Answer(Message,'Y') != YES) goto TRYENTER;
	}

DisposeDialog(enternameptr);

CopyPString("\p_bp2_key",PascalLine);
if(formyself || Answer("Delete the current (invisible) activation key",'N') == YES) {
	io = FSDelete(PascalLine,0);
	}
if(formyself) {
	reply.sfFile.vRefNum = RefNumbp2;
	reply.sfFile.parID = ParIDbp2;
	reply.sfReplacing = FALSE;
	CopyPString(PascalLine,reply.sfFile.name);
	}
else {
	result = NewFile(-1,1,PascalLine,&reply);
	if(result != OK) return(FAILED);
	}
result = CreateFile(-1,-1,1,PascalLine,&reply,&refnum);
if(result != OK) {
	Alert1("Unexpected problem recording your identity. Is the hard disk full?\nContact the authors");
	return(ABORT);
	}
WriteHeader(-1,refnum,reply.sfFile);
WriteToFile(NO,MAC,UserName,refnum);
WriteToFile(NO,MAC,UserInstitution,refnum);
CloseMe(&refnum);
if(formyself) {
	spec = reply.sfFile;
	io = FSpGetFInfo(&spec,&fndrinfo);
	fndrinfo.fdFlags |= fInvisible;
	io = FSpSetFInfo(&spec,&fndrinfo);
	}
return(OK);
}


Y2K(void)
{
short y2krefnum;
unsigned long today,secs;
DateTimeRec dtrp;
OSErr io;
FSSpec spec;
FInfo fndrinfo;
int type;
	
dtrp.year = 2000;
dtrp.month = 1;
dtrp.day = 1;
dtrp.hour = dtrp.minute = dtrp.second = 0; 
/* dtrp.year = 1998;
dtrp.month = 9;
dtrp.day = 22; */
DateToSeconds(&dtrp,&secs);
GetDateTime(&today);
// spec.vRefNum = RefNumbp2;
// spec.parID = ParIDbp2;
type = 0;
io = FSMakeFSSpec(RefNumbp2, ParIDbp2, "\py2k", &spec);
io = FSpGetFInfo(&spec,&fndrinfo);
if(io == noErr) {
	if(!(fndrinfo.fdFlags & fInvisible)) {
		fndrinfo.fdFlags |= fInvisible;
		io = FSpSetFInfo(&spec,&fndrinfo);
		}
	if(today > secs) {
		if(((today-secs) < (86400L * 70L)) && (io=MyOpen(&spec,fsRdPerm,&y2krefnum)) == noErr) {
			Alert1("Welcome to the third millennium!\n(Any idea whether this software is y2k compliant?)");
			while(Button());
			BPActivateWindow(SLOW,wData);
			BPActivateWindow(SLOW,wGrammar);
			Println(wNotice,"\n\nPhew! BP2 survived the shock...\n");
			Println(wNotice,"You'd better check your bank account, retirement planning, etc.\n");
			Println(wNotice,"It's an auspicious day to click the 'Register' button if you never tried it and the bank hasn't screwed up your account ;-)\n\n");
			Println(wNotice,"With you, forever,\n\n");
			Println(wNotice,"*    *   *  * * Bol Processor folks * *  *   *    *\n\n");
			Println(wNotice,"             (Click mouse to continue)");
			while(!Button());
			FSClose(y2krefnum);
			io = FSDelete(&spec);
			}
		}
	}
else {
	if(today < secs) {
		Alert1("You should get the y2k certificate for this program.\nContact <bel@kagi.com>");
		}
	}
return(OK);
}

#endif // 0

#if BP_CARBON_GUI
int CheckDate()
{
	unsigned long today,secs;
	DateTimeRec dtrp;
	int reply = YES;
			
	dtrp.year = 1984;
	dtrp.month = 1;
	dtrp.day = 24;
	dtrp.hour = dtrp.minute = dtrp.second = 0; 
	/*dtrp.year = 2007;
	dtrp.month = 1;
	dtrp.day = 19;*/
	DateToSeconds(&dtrp,&secs);
	GetDateTime(&today);
	if(today < secs) {
		reply = Answer("BP2 requires a 68020 processor and at least System 4 to run. "
				   "This isn't 1984 anymore!!\nContinue anyways?", 'Y');
	}
	if (reply == YES)  return(OK);
	else return(FAILED);
}
#endif /* BP_CARBON_GUI */
