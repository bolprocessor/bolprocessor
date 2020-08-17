/* -BP2main.h  (BP2 version CVS) */

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

#ifndef BP2_MAIN_H
#define BP2_MAIN_H

#if  defined(__POWERPC) && !TARGET_API_MAC_CARBON
QDGlobals Qd;
#endif

GWorldPtr gMainGWorld;
GDHandle gCurDev;
int Version = 30; /* 2.999... */

ProcessSerialNumber PSN;
char UserName[MAXNAME],UserInstitution[MAXNAME];
long MemoryUsed,TempMemoryUsed,MemoryUsedInit,MaxMemoryUsed,MaxHandles,
	SessionTime;
int CheckMem;

#if BP_CARBON_GUI
IntProcPtr Menu[][MAXMENUITEMS] =
	{{NULLPROC,mAbout,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,
		NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,
		NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC},
	{NULLPROC,mNewProject,mLoadProject,NULLPROC,mOpenFile,mClearWindow,mGoAway,mSaveFile,mSaveAs,mRevert,
		mGetInfo,NULLPROC,mLoadSettings,mSaveSettings,mSaveSettingsAs,mSaveStartup,NULLPROC,mMakeGrammarFromTable,
		mReceiveMIDI,mSendMIDI,mLoadTimePattern,NULLPROC,mPageSetup,mPrint,NULLPROC,mQuit},
	{NULLPROC,mUndo,NULLPROC,mCut,mCopy,mPaste,mClear,NULLPROC,mSelectAll,NULLPROC,mText,mToken,NULLPROC,mTypeNote,
		NULLPROC,mPickPerformanceControl,mPickGrammarProcedure,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,
		NULLPROC,NULLPROC,NULLPROC,NULLPROC},
	{NULLPROC,mFind,mEnterFind,mFindAgain,NULLPROC,mCheckVariables,mListTerminals,mListReserved,
		mBalance,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,
		NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC},
	{NULLPROC,mCompile,mProduce,mCheckDeterminism,mTemplates,mAnalyze,mPlaySelectionOrPrototype,mExpandSelection,
		mCaptureSelection,mShowPeriods,mExecuteScript,mTransliterateFile,NULLPROC,mPause,mResume,mStop,
		NULLPROC,mResetSessionTime,mTellSessionTime,NULLPROC,mHelp,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC},
	{NULLPROC,m9pt,m10pt,m12pt,m14pt,NULLPROC,mChangeColor,mUseGraphicsColor,mUseTextColor,
		NULLPROC,mAzerty,mQwerty,NULLPROC,mUseBullet,NULLPROC,mSplitTimeObjects,mSplitVariables,
		NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC},
	{NULLPROC,mMetronom,mComputationSettings,mInputOutputSettings,mRandomSequence,mTimeAccuracy,mBufferSize,
		mGraphicSettings,mDefaultPerformanceValues,mDefaultStrikeMode,mFileSavePreferences,mTuning,NULLPROC,
		mFrenchConvention,mEnglishConvention,mIndianConvention,mKeyConvention,NULLPROC,mPianoRoll,NULLPROC,
		NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC},
	{NULLPROC,mMIDI,mMIDIfile,mCsound,NULLPROC,NULLPROC,mMIDIinputcheck,mMIDIoutputcheck,mMIDIfilter,NULLPROC,
		mMIDIorchestra,mCsoundInstrumentsSpecs,NULLPROC,mShowCMSettings,mOpenAudioMidiSetup,NULLPROC,NULLPROC,
		NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC,NULLPROC},
	{NULLPROC,mGrammar,mAlphabet,mData,mMiscSettings,mObjectPrototypes,mTimeBase,mInteraction,mScript,mGlossary,
		mKeyboard,NULLPROC,mControlPannel,NULLPROC,mGraphic,mTrace,mStartString,mScrap,
		mNotice,NULLPROC,mFAQ,NULLPROC,mShowMessages,NULLPROC,NULLPROC,NULLPROC}};

ResID  WindowMenuIcons[11] = { 10, 129, 130, 132, 133, 131, 140, 139, 137, 138, 136 };
ResID  DeviceMenuIcons[12] = { 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 144, 142 };
#endif /* BP_CARBON_GUI */

// FIXME: figure out how to completely remove TempMemory, AskedTempMemory, TempMemoryUsed (& FixedMaxQuantization?)
int EmergencyExit,TempMemory,AskedTempMemory,FixedMaxQuantization,TraceMemory,
	EventState,Beta;
int SetUpTime;  /* Time needed to set-up driver (ms) */
int LoadedScript;
int Jmessage;
// TEStyleRec** StyleHandle[WMAX];

long GramSelStart,GramSelEnd;

FSSpec **p_TempFSspec;

char **p_MessageMem[MAXMESSAGE],**p_HTMLchar1,**p_HTMLchar2,**p_HTMLdiacritical;
MIDIstream Stream;

RGBColor White,Black,Red,Green,Blue,Yellow,Magenta,Cyan,Brown,None,NoteScaleColor1,NoteScaleColor2;
RGBColor Color[MAXCOLOR],PianoColor[MAXCHAN],**p_ObjectColor;
int UseGraphicsColor,UseTextColor,ForceTextColor,ForceGraphicColor;

node PrefixTree,SuffixTree;
long SwitchState;
char EndStr[4];
char InitToken[] = "INIT:";
Rect PictFrame;
char Mode[][MODELENGTH] = {"RND","LEFT","RIGHT"};
char NilString[][NILLENGTH] = {"nil","lambda","empty","null"};
char Reality[][6] = {"false","true"};
char Arrow[][ARROWLENGTH] = {"<->","-->","<--"};
MIDIcontrolstatus **p_Oldvalue;
char ****p_GramProcedure,****p_PerformanceControl,****p_GeneralMIDIpatch,
	****p_Diacritical,****p_HTMLdiacrList;
long MaxProc,MaxPerformanceControl;
INscripttype** p_INscript;
int **p_PerfCtrlNdx,**p_GeneralMIDIpatchNdx,**p_PerfCtrlNArg,**p_ProcNdx,**p_ProcNArg;
char SubgramType[][TYPELENGTH] = {"RND","ORD","LIN","SUB","SUB1","TEM","POSLONG"};
char Code[] = {'\0','?','#','+',':',';','=','¥','(',')','S','/','{','}',',','<',
	'>','É','&','&','!','*','{','}'};
char VersionName[][VERSIONLENGTH] = {"-","V.2.1","V.2.2","V.2.3","V.2.4","V.2.5",
	"V.2.5.1","V.2.5.2","V.2.6","BP2.6.1","BP2.6.2","BP2.6.3","BP2.7","BP2.7.1","BP2.7.2",
	"BP2.7.3","BP2.7.4","BP2.8.0","BP2.8.1","BP2.9.0","BP2.9.1","BP2.9.2","BP2.9.3","BP2.9.4",
	"BP2.9.5", "BP2.9.6beta", "BP2.9.6", "BP2.9.7beta", "BP2.9.8", "BP2.9.9", "BP2.999..."};
char Arrowstring[] = "-->",GRAMstring[] = "gram#";
char Portname[][8] = {"\0","Modem","Printer"};
char ConventionString[][20] = {"ENGLISH","FRENCH","INDIAN","KEYS"};
char KeyString[] = "key#";
char Englishnote[][3] =
	{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
char Frenchnote[][5] =
	{"do","do#","re","re#","mi","fa","fa#","sol","sol#","la","la#","si"};
char Indiannote[][5] =
	{"sa","rek","re","gak","ga","ma","ma#","pa","dhak","dha","nik","ni"};
char AltEnglishnote[][3] =
	{"B#","Db","D","Eb","Fb","E#","Gb","G","Ab","A","Bb","Cb"};
char AltFrenchnote[][5] =
	{"si#","reb","re","mib","fab","mi#","solb","sol","lab","la","sib","dob"};
char AltIndiannote[][5] =
	{"ni#","sa#","re","re#","mak","ga#","pak","pa","pa#","dha","dha#","sak"};
short NameChoice[12];
char ****p_NoteName[MAXCONVENTIONS],****p_AltNoteName[MAXCONVENTIONS];
int **p_NoteLength[MAXCONVENTIONS],**p_AltNoteLength[MAXCONVENTIONS];
int InitThere,FirstTime,MaxScriptInstructions,KeyboardType,C4key,ProgNrFrom,TestMIDIChannel,
	CurrentMIDIprogram[MAXCHAN+1];
double A4freq;
char FindString[256],ReplaceString[256];
int RunningStatus,StrikeAgainDefault;
int NoteOffPass,NoteOnPass,KeyPressurePass,ControlTypePass,ProgramTypePass,
	ChannelPressurePass,PitchBendPass,SysExPass,TimeCodePass,SongPosPass,SongSelPass,
	TuneTypePass,EndSysExPass,ClockTypePass,StartTypePass,ContTypePass,
	ActiveSensePass,ResetPass;
int NoteOffIn,NoteOnIn,KeyPressureIn,ControlTypeIn,ProgramTypeIn,
	ChannelPressureIn,PitchBendIn,SysExIn,TimeCodeIn,SongPosIn,SongSelIn,
	TuneTypeIn,EndSysExIn,ClockTypeIn,StartTypeIn,ContTypeIn,
	ActiveSenseIn,ResetIn;
t_gram Gram,GlossGram;
scriptcommandtype **h_Script;
fullscriptcommandtype ScriptLine;
int **h_ScriptIndex;
char *****pp_StringList,****p_StringList;
int iString,NrStrings,SizeStringList;
char ****p_Bol,****p_Patt,****p_Flagname,****p_Script,****p_Var,****p_Homo,****p_Token;
tokenbyte ***pp_Scrap,**p_Scrap,**p_Initbuff;
char **p_InitScriptLine;
unsigned Seed,UsedRandom;
short CurrentVref,LastVref;
int ScriptW,CurrentChannel;
long CurrentDir,LastDir,OldModulation;
int PitchbendRange[MAXCHAN+1],DeftPitchbendRange,PitchbendRate[MAXCHAN+1],ModulationRate[MAXCHAN+1],PressRate[MAXCHAN+1],
	VolumeRate[MAXCHAN+1],VolumeControl[MAXCHAN+1],PanoramicRate[MAXCHAN+1],PanoramicControl[MAXCHAN+1],
	DeftVolume,DeftVelocity,DeftPanoramic,PanoramicController,VolumeController,SamplingRate;
char ChangedVolume[MAXCHAN],ChangedPanoramic[MAXCHAN],ChangedModulation[MAXCHAN],ChangedPitchbend[MAXCHAN],ChangedSwitch[MAXCHAN],
	ChangedPressure[MAXCHAN],WhichCsoundInstrument[MAXCHAN+1];
int Jbol,Jfunc,Jinstr,Jinscript,Maxinscript,iProto,Jcontrol,Jpatt,Jvar,Jflag,MaxVar,
	Jhomo,**p_VarStatus,iCsoundInstrument;
int FreezeWindows,StartFromOne,OutMIDI,OutCsound,MustChangeInput,SmartCursor,NotSaidKpress,
	**p_MemGram,**p_MemRul;
long ProduceStackDepth,ProduceStackIndex,DisplayStackIndex,
	**p_LastStackIndex,**p_MemPos;
long **p_ItemStart,**p_ItemEnd,CompileDate,ComputeStart,MaxComputeTime,
	ComputeTime,ItemNumber,Maxitems,Interrupted,MaxDeriv,Sel1,Sel2;
int Nplay,SynchroSignal,QuantizeOK,IgnoreCase,MatchWords,
	TargetWindow,GraphicScaleP,GraphicScaleQ;
long Time_res,Quantization;
volatile unsigned long TimeSlice;

#if WITH_REAL_TIME_SCHEDULER
Slice **Clock,***p_Clock,**p_AllSlices,*Slices,*SlicePool;
volatile unsigned long TotalTicks;
volatile char OKsend;
#endif
// these are part of the Scheduler but we need them for now - akozar
volatile char ClockOverFlow,SchedulerIsActive;
char Mute,Panic;

/* char AlertMute; */   // obsolete; only used by ErrorSound callback - 011707 - akozar

int WaitKey[MAXWAIT+1],WaitChan[MAXWAIT+1];

// Variables for Csound instruments
CsoundInstrument **p_CsInstrument;
int **p_CsInstrumentIndex,**p_CsDilationRatioIndex,**p_CsAttackVelocityIndex,
	**p_CsReleaseVelocityIndex,**p_CsPitchIndex,**p_CsPitchBendStartIndex,
	**p_CsVolumeStartIndex,**p_CsPressureStartIndex,**p_CsModulationStartIndex,
	**p_CsPanoramicStartIndex,**p_CsPitchBendEndIndex,**p_CsVolumeEndIndex,
	**p_CsPressureEndIndex,**p_CsModulationEndIndex,**p_CsPanoramicEndIndex;
char ****pp_CsInstrumentName,****pp_CsInstrumentComment,**p_CsPitchFormat,****p_StringConstant;
double **p_CsPitchBend[6],**p_CsVolume[6],**p_CsPressure[6],**p_CsModulation[6],
	**p_CsPanoramic[6],**p_NumberConstant;


int FileSaveMode,FileWriteMode,MIDIfileType,CsoundFileFormat;

SoundObjectInstanceParameters **p_Instance;
objectspecs ****p_ObjectSpecs;
long **p_Flag,BufferSize,DeftBufferSize,Maxevent;
FILE* OpenMIDIfilePtr;
short HelpRefnum,TempRefnum,TraceRefnum,CsRefNum,CsScoreOpened,MIDIfileOpened,
	MIDIfileTrackEmpty;
int MIDIbytestate,MIDIfileTrackNumber,StepScript,TypeScript,MoreTime;
long MidiLen_pos,**p_Tref,**p_Tpict;
dword MIDItracklength,Midi_msg;
Milliseconds OldMIDIfileTime;
unsigned long LapWait,WhenItStarted;
int ****p_Image,****p_NoteImage,MaxGram,MaxRul,SplitTimeObjects,SplitVariables,Token,SpaceOn,
	VariableOn,N_err,nstore,NumberTables,OkShowExpand,Improvize,ComputeWhilePlay,TransposeInput,
	TransposeValue,Varweight,Flagthere,ResetDone,BolsInGrammar,NoAlphabet,PointCsound,PointMIDI,
	**p_Ifrom,**p_Resolution,**p_CsoundInstr,**p_CsoundAssignedInstr;
MIDIcode ****pp_MIDIcode;
float **p_CsoundTempo;
Milliseconds ****pp_CsoundTime;
long **p_CsoundSize,**p_MIDIsize;
char **p_DefaultChannel,**p_AlphaCtrlNr,**p_AlphaCtrlChan,**p_AlphaCtrl,**p_Type,**p_PasteDone,
	**p_FixScale,**p_OkExpand,**p_OkCompress,**p_OkRelocate,**p_BreakTempo,**p_DiscardNoteOffs,
	**p_OkTransp,**p_OkArticul,**p_OkVolume,**p_OkPan,**p_OkMap,**p_OkVelocity,
	**p_ContBeg,**p_ContEnd,
	**p_CoverBeg,**p_CoverEnd,**p_TruncBeg,**p_TruncEnd,**p_PivType,**p_PivMode,
	**p_RescaleMode,**p_DelayMode,**p_ForwardMode,
	**p_BreakTempoMode,**p_ContBegMode,**p_ContEndMode,**p_CoverBegMode,**p_CoverEndMode,
	**p_TruncBegMode,**p_TruncEndMode,**p_PreRollMode,**p_PostRollMode,**p_PeriodMode,
	**p_ForceIntegerPeriod,**p_StrikeAgain,**p_CompiledCsoundScore,
	****pp_Comment,****pp_CsoundScoreText;
CsoundLine ****pp_CsoundScore;
double **p_AlphaMin,**p_AlphaMax,**p_Quan,ForceRatio,Infpos1;
float **p_PivPos,**p_BeforePeriod;
long **p_Dur,**p_MaxDelay,**p_MaxForward,**p_MaxBegGap,**p_MaxEndGap,
	**p_MaxCoverBeg,**p_MaxCoverEnd,**p_MaxTruncBeg,**p_MaxTruncEnd,**p_PreRoll,**p_PostRoll;
short Minconc,Maxconc,Maxlevel,MaxFlag,MaxScript,Jscriptline,Final,POLYconvert,
	KeepFlag,CoverOK,DiscontinuityOK,AllSolTimeSet,
	LimCompute,LimTimeSet,SkipFlag,StackFlag,PlayFromInsertionPoint,
	Port,Portbit,****p_Seq,MIDI;
int CyclicPlay,AllowRandomize,WillRandomize,StepProduce,StepGrammars,DisplayProduce,TraceProduce,UseTimeLimit,
	DisplayTimeSet,StepTimeSet,UseEachSub,PlanProduce,TraceTimeSet;
int InsertGramRuleNumbers, InsertGramCorrections, InsertNewBols;
unsigned long Ptick[MAXTICKS],Qtick[MAXTICKS],GrandCycle[MAXTICKS],
	PosGrandCycle[MAXTICKS],GrandPeriod,BeatGrandCycle,BeatPosGrandCycle,
	LastCommonDate[MAXTICKS],LastCommonBeatDate;
double Pclock,Qclock,OldPclock,OldQclock,MinPclock,MaxPclock,MinQclock,MaxQclock,
	Prod,Ratio,Kpress,Pduration,Qduration;
long Infpos,Infneg,Veryneg,MaxMIDIbytes,
	TimeMax,Nalpha,Nbytes,Tbytes2,MIDIinputFilter,MIDIoutputFilter,
	MIDIinputFilterstartup,
	MIDIoutputFilterstartup,Ts,DataEnd;
Milliseconds **p_T;
long **p_Ppatt,**p_Qpatt;
MIDIcode **p_Code;
double SpeedRange,Ke,CorrectionFactor;
int DeriveFurther,DeriveFurtherKey,DeriveFurtherChan,ResetWeights,NeverResetWeights,
	ResetWeightKey,ResetWeightChan,ResetFlags,ResetControllers,AllItems,JustCompiled,
	PlayKey,PlayChan,RepeatKey,RepeatChan,EndRepeatKey,EndRepeatChan,IgnoreUndefinedVariables,
	EverKey,EverChan,QuitKey,QuitChan,MuteOnKey,MuteOnChan,MuteOffKey,MuteOffChan,
	UseEachSubKey,UseEachSubChan,
	SetTimeKey,SetTimeChan,StriatedKey,StriatedChan,
	MinTclockKey,MaxTclockKey,TclockChan,SkipKey,SkipChan,NoConstraint,
	NoConstraintKey,NoConstraintChan,AgainKey,AgainChan,
	SynchronizeStart,SynchronizeStartKey,SynchronizeStartChan,SpeedCtrl,SpeedChan,
	WriteMIDIfile,CsoundTrace,ShownBufferSize,
	ParamControl[MAXPARAMCTRL],ParamKey[MAXPARAMCTRL],ParamChan[MAXPARAMCTRL],
	ParamValue[MAXPARAMCTRL],ParamInit[MAXPARAMCTRL],SaidTooComplex,ToldSkipped,
	ParamControlChan,Newstatus,ShowMessages,ScriptSyncKey,ScriptSyncChan,
	SUBthere,DisplayItems,ShowGraphic,NoteConvention,FunctionTable,
	ScriptRecOn,ScriptExecOn,DefaultVolume,LastAction,UndoWindow,
	PedalOrigin,PedalPosition,HangOn,NoRepeat,ConvertMIDItoCsound;
TextOffset UndoPos;
long CountOn;
unsigned long WaitStartDate,WaitEndDate;
char Message[MAXLIN],TheName[MAXNAME];
char DateMark[] = "Date:";
PolyHandle p_Diagram[MAXDIAGRAM];
int Ndiagram,ObjectMode,ObjectTry;
int DiagramWindow[MAXDIAGRAM];
int Finding,Hpos;
PicHandle NoteScalePicture,p_Picture[MAXPICT];
Rect NoteScaleRect,PictRect[MAXPICT];
int Npicture,Offscreen;
int PictureWindow[MAXPICT];
int Charstep,UndoFlag;
char LineBuff[MAXLIN];
Str255 PascalLine;
long DataOrigin,Tcurr,LastTcurr;
unsigned long NextTickDate[MAXTICKS],NextBeatDate;

int Nw,LastEditWindow,LastComputeWindow,OutputWindow,ResumeStopOn,ResetTickFlag,ResetTickInItemFlag;
WindowPtr Window[WMAX];
DialogPtr ResumeStopPtr,ResumeUndoStopPtr,MIDIkeyboardPtr;
DialogPtr PatternPtr,ReplaceCommandPtr,EnterPtr,GreetingsPtr,FAQPtr,SixteenPtr,FileSavePreferencesPtr,
	StrikeModePtr,TuningPtr,DefaultPerformanceValuesPtr,CsoundInstrMorePtr,OMSinoutPtr,MIDIprogramPtr;
DialogPtr gpDialogs[WMAX];
/* DialogRecord EnterDR,ReplaceCommandDR,TuningDR,DefaultPerformanceValuesDR,CsoundInstrMoreDR,PatternDR,
	MIDIprogramDR,FileSavePreferencesDR; */

TextHandle TEH[WMAX];

int linesInFolder[WMAX];
Boolean WindowFullAlertLevel[WMAX];	// used in TextEdit build to track which alerts have been shown
MenuHandle myMenus[MAXMENU+1];
int Buttonheight = 14;
int NewEnvironment,NewColors,ShowPianoRoll,ToldAboutPianoRoll,ShowObjectGraph,Help,ChangedCoordinates[WMAX];
RGBColor CurrentColor[WMAX];
ControlHandle vScroll[WMAX],hScroll[WMAX];
int SlideH[WMAX],SlideV[WMAX],Hmin[WMAX],Hmax[WMAX],Hzero[WMAX],
	Vmin[WMAX],Vmax[WMAX],Vzero[WMAX];
short IsHTML[WMAX],IsText[WMAX],Weird[WMAX];
char **p_FileInfo[WMAX];

// FileTypeIndexToWindowFileIndex does the reverse of gFileType below
int	FileTypeIndexToWindowFileIndex[MAXFILETYPEINDEX] = {wUnknown, wUnknown, wKeyboard, iObjects, 
		iDecisions, wGrammar, wAlphabet, wData, wInteraction, iSettings, wUnknown, iMIDIfile, 
		iWeights, wScript, wGlossary, wTimeBase, wCsoundInstruments, wMIDIorchestra, wUnknown,
		iMidiDriver};
		
// In this section every array should contain WMAX values

int WindowTextSize[] = {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
									10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};
short LockedWindow[] =	{0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
short GrafWindow[] =	{0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0};
short OKvScroll[] =	{1,1,1,0,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0};
short OKhScroll[] =	{0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
short OKgrow[] =		{1,1,1,0,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
short Adjustable[] =	{1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
short NeedSave[] =	{1,1,1,0,0,1,0,1,1,1,0,0,1,1,0,0,1,0,0,0,0,1,0,0,1,0,1,1,1,1,1,1,0,0,0,1,1,1,1};
int Freebottom[] =	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,20,0};
short Editable[] =	{1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0};
short HasFields[] =  	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,0,0,1,1,1,1,1,1,0,0,1,1,1,0,1}; // Note: HasFields[n] true also assumes IsDialog[n] true
short IsDialog[] =  	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
Boolean WindowUsesThemeBkgd[] = 
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,1};

	// gFileType maps window/file index to a FileTypeIndex
short gFileType[] =   {5,6,1,1,1,1,1,7,13,0,1,1,14,8,1,1,15,19,1,1,1,2,15,15,11,9,3,1,1,1,1,1,1,1,1,16,1,1,17};
	// FilePrefix, FileExtension, and DocumentTypeName map window/file index to their appropriate strings
char FilePrefix[][5] = {"-gr.","-ho.","\0","\0","\0","-tr.","\0","-da.",
	"+sc.","\0","\0","\0","-gl.","-in.","-wg.","\0","-tb.","-md.","\0","\0","\0","-kb.",
	"\0","\0","\0","-se.","-mi.","\0","\0","\0","\0","\0","\0","\0","\0","-cs.","\0","\0","-or."};
char FileExtension[][6] = {".bpgr",".bpho","\0","\0","\0",".bptr","\0",".bpda",
	".bpsc","\0","\0","\0",".bpgl",".bpin",".bpwg","\0",".bptb",".bpmd","\0","\0","\0",".bpkb",
	"\0","\0","\0",".bpse",".bpmi","\0","\0","\0","\0","\0","\0","\0","\0",".bpcs","\0","\0",".bpor"};
char DocumentTypeName[][21] = {"grammar","alphabet","\0","\0","\0","trace","\0","data",
	"script","\0","\0","\0","glossary","interaction","weights","decisions","time-base","MIDI driver settings",
	"\0","\0","\0","keyboard","\0","\0","MIDI","settings","sound-objects","\0","\0","\0","\0","\0","\0","\0","\0",
	"csound instruments","\0","\0","MIDI orchestra"};
	/* See window and dialog indexes in BP2.h */
char DeftName[][MAXNAME] = {"New Grammar","New Alphabet",
	"Start string","Message","Graphic","Trace","Info",
	"New Data","New script","Scrap","Help","Notice","New glossary","New interaction",
	"Random","Time accuracy","Time base","Buffer","Find","Graphic settings","Control",
	"New Keyboard","Script","Metronom","\0","\0","New prototypes",
	"\0","\0","\0","\0","Period","Csound object prototype","MIDI filter","Tick settings",
	"Csound instruments","\0","\0","MIDI orchestra"};
	/* See window and dialog indexes in -BP2.h */
char WindowName[][MAXNAME] = {"Grammar","Alphabet","Start string",
	"Message","Graphic","Trace","Info",
	"Data","Script","Scrap","Help","Notice","Glossary","Interaction",
	"Weight","Time accuracy","Time base","Buffer","Find-replace",
	"Graphic settings","Control pannel","Keyboard","Script dialog","Tempo",
	"Computation settings","General settings","Object Prototypes",
	"Prototypes (2)","Prototypes (3)","Prototypes (4)","Prototypes (5)","Prototypes (6)",
	"Prototypes (7)","MIDI filter","Tick settings","Csound instruments","Prototypes (8)",
	"Csound tables","MIDI orchestra"};

// End of section

char FileName[WMAX][MAXNAME+1],CsFileName[MAXNAME+1],MIDIfileName[MAXNAME+1],
	CsoundOrchestraName[MAXNAME+1];
FSSpec MIDIfileSpec;
float MIDIfadeOut;
int CurrentVolume[MAXCHAN+1];
long WindowParID[WMAX],ParIDstartup,ParIDbp2;
// Str255 DeftVolName;
Cursor EditCursor,WatchCursor,CrossCursor,HelpCursor,KeyboardCursor,WheelCursor[4],
	FootCursor[8],DiskCursor[2];
int Jwheel,Jfeet,Jdisk;
int Jbutt; /* Number of buttons created in dialogs */
ControlHandle Hbutt[MAXBUTT];
int Dirty[WMAX],CompiledGr,CompiledPt,CompiledAl,CompiledIn,CompiledGl,CompiledCsObjects,
	CompiledRegressions,LoadedIn,LoadedGl,LoadedCsoundInstruments,Interactive,
	NotFoundMetronom,NotFoundNatureTime;
int NotBPCase[MAXNOTBPCASES];
int Created[WMAX];
short TheVRefNum[WMAX],RefNumStartUp,RefNumbp2;
int Nature_of_time,UseBufferLimit,NeedAlphabet,CompleteDecisions,OkWait;
int PrototypeTickChannel,PrototypeTickKey,PrototypeTickVelocity;
int TickChannel[MAXTICKS],TickKey[MAXTICKS],TickVelocity[MAXTICKS],MuteTick[MAXTICKS],
	TickDuration[MAXTICKS],
	TickCycle[MAXTICKS],TickPosition[MAXTICKS],iTick,jTick,HideTicks,PlayTicks,UseBullet,NeedZouleb;
long ThisTick[MAXTICKS][MAXBEATS];
int InitOn,SetTimeOn,ComputeOn,PolyOn,SoundOn,SelectOn,PrintOn,InputOn,ClickRuleOn,InBuiltDriverOn,
	EnterOn,Option,CompileOn,GraphicOn,ReadKeyBoardOn,ButtonOn,LoadOn,SaveOn,PauseOn,AlertOn,AllOn,
	PlaySelectionOn,PlayPrototypeOn,AEventOn,ExpandOn,SelectPictureOn,EmptyBeat,TickDone,TickThere,
	FoundNote,NoCursor,MIDIfileOn,WaitOn,ReceivedOpenAppEvent,Oms,GotAlert,NewOrchestra,ChangedMIDIprogram,
	ItemOutPutOn,ItemCapture,TickCapture,TickCaptureStarted,AskedAboutCsound;
double MaxTempo,InvMaxTempo,TokenLimit,InvTokenLimit;
double MaxFrac;
Boolean HaveAppearanceManager;
Boolean RunningOnOSX;
ControlActionUPP vScrollUPP,hScrollUPP;
NSWReply** MIDIfileReply;
NSWReply** CsFileReply;
FSSpec TempSpec;
UInt32 NumInstalledDrivers;
BPMidiDriver** InstalledDrivers;
DynamicMenuItem** InstalledMenuItems;

#endif /* BP2_MAIN_H */
