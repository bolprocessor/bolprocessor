// -BP2decl.h (BP2 version CVS) 

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

#ifndef BP2_DECL_H
#define BP2_DECL_H

#if  defined(__POWERPC) && !TARGET_API_MAC_CARBON
extern QDGlobals Qd;
#endif

extern GWorldPtr gMainGWorld;
extern GDHandle gCurDev;
extern int Version;

#if BP_MACHO
// CoreMIDI globals (these are allocated in CoreMIDIdriver.c)
extern DialogPtr CMSettings;
#endif	

extern char UserName[MAXNAME],UserInstitution[MAXNAME];
extern ProcessSerialNumber PSN;					
extern long MemoryUsed,TempMemoryUsed,MemoryUsedInit,MaxMemoryUsed,
	MaxHandles,SessionTime;
extern int CheckMem;

#if BP_CARBON_GUI
extern IntProcPtr Menu[MAXMENU][MAXMENUITEMS];
extern ResID WindowMenuIcons[11];
extern ResID DeviceMenuIcons[12];
#endif /* BP_CARBON_GUI */

extern int EmergencyExit,TempMemory,AskedTempMemory,FixedMaxQuantization,
	TraceMemory,EventState,Beta;
extern int SetUpTime;
extern int LoadedScript;
extern int Jmessage;
// extern TEStyleRec** StyleHandle[WMAX];

extern long GramSelStart,GramSelEnd;

extern FSSpec **p_TempFSspec;

extern char **p_MessageMem[MAXMESSAGE],**p_HTMLchar1,**p_HTMLchar2,**p_HTMLdiacritical;
extern MIDIstream Stream;

extern RGBColor White,Black,Red,Green,Blue,Yellow,Magenta,Cyan,Brown,None,
	NoteScaleColor1,NoteScaleColor2;
extern RGBColor Color[MAXCOLOR],PianoColor[MAXCHAN],**p_ObjectColor;
extern int UseGraphicsColor,UseTextColor,ForceTextColor,ForceGraphicColor;

extern node PrefixTree,SuffixTree;
extern long SwitchState;
extern char EndStr[4];
extern char InitToken[6];
extern Rect PictFrame;
extern char Mode[MAXMODE][MODELENGTH];
extern char NilString[MAXNIL][NILLENGTH];
extern char Reality[2][6];
extern char Arrow[MAXARROW][ARROWLENGTH];
extern MIDIcontrolstatus **p_Oldvalue;
extern char ****p_GramProcedure,****p_PerformanceControl,****p_GeneralMIDIpatch,
	****p_Diacritical,****p_HTMLdiacrList;
extern long MaxProc,MaxPerformanceControl;
extern int **p_PerfCtrlNArg,**p_ProcNArg,**p_PerfCtrlNdx,**p_ProcNdx,**p_GeneralMIDIpatchNdx;
extern INscripttype** p_INscript;
extern char SubgramType[MAXTYPE][TYPELENGTH];
extern char Code[MAXCODE2];
extern char VersionName[MAXVERSION][VERSIONLENGTH];
extern char Arrowstring[4],GRAMstring[6];
extern char Portname[3][8];
extern char ConventionString[MAXCONVENTIONS][20];
extern char KeyString[5];
extern char Englishnote[12][3],Frenchnote[12][5],Indiannote[12][5];
extern char AltEnglishnote[12][3],AltFrenchnote[12][5],AltIndiannote[12][5];
extern short NameChoice[12];
extern char ****p_NoteName[MAXCONVENTIONS],****p_AltNoteName[MAXCONVENTIONS];
extern int **p_NoteLength[MAXCONVENTIONS],**p_AltNoteLength[MAXCONVENTIONS];
extern int InitThere,FirstTime,MaxScriptInstructions,KeyboardType,C4key,ProgNrFrom,TestMIDIChannel,
	CurrentMIDIprogram[MAXCHAN+1];
extern double A4freq;
extern char FindString[256],ReplaceString[256];
extern t_gram Gram,GlossGram;
extern int RunningStatus,StrikeAgainDefault;
extern int NoteOffPass,NoteOnPass,KeyPressurePass,ControlTypePass,ProgramTypePass,
	ChannelPressurePass,PitchBendPass,SysExPass,TimeCodePass,SongPosPass,SongSelPass,
	TuneTypePass,EndSysExPass,ClockTypePass,StartTypePass,ContTypePass,
	ActiveSensePass,ResetPass;
extern int NoteOffIn,NoteOnIn,KeyPressureIn,ControlTypeIn,ProgramTypeIn,
	ChannelPressureIn,PitchBendIn,SysExIn,TimeCodeIn,SongPosIn,SongSelIn,
	TuneTypeIn,EndSysExIn,ClockTypeIn,StartTypeIn,ContTypeIn,
	ActiveSenseIn,ResetIn;
extern scriptcommandtype **h_Script;
extern fullscriptcommandtype ScriptLine;
extern int **h_ScriptIndex;
extern char *****pp_StringList,****p_StringList;
extern int iString,NrStrings,SizeStringList;
extern char ****p_Bol,****p_Patt,****p_Flagname,****p_Script,
	****p_Var,****p_Homo,****p_Token;
extern tokenbyte ***pp_Scrap,**p_Scrap,**p_Initbuff;
extern char **p_InitScriptLine;
extern unsigned Seed,UsedRandom;
extern short CurrentVref,LastVref;
extern int ScriptW,CurrentChannel;
extern long CurrentDir,LastDir,OldModulation;
extern int PitchbendRange[MAXCHAN+1],DeftPitchbendRange,PitchbendRate[MAXCHAN+1],ModulationRate[MAXCHAN+1],PressRate[MAXCHAN+1],
	VolumeRate[MAXCHAN+1],VolumeControl[MAXCHAN+1],PanoramicRate[MAXCHAN+1],PanoramicControl[MAXCHAN+1],
	DeftVolume,DeftVelocity,DeftPanoramic,PanoramicController,VolumeController,SamplingRate;
extern char ChangedVolume[MAXCHAN],ChangedPanoramic[MAXCHAN],ChangedModulation[MAXCHAN],ChangedPitchbend[MAXCHAN],ChangedSwitch[MAXCHAN],
	ChangedPressure[MAXCHAN],WhichCsoundInstrument[MAXCHAN+1];
extern int Jbol,Jinstr,Jfunc,Jinscript,Maxinscript,iProto,Jcontrol,Jpatt,Jvar,Jflag,
	MaxVar,Jhomo,**p_VarStatus,iCsoundInstrument;
extern int FreezeWindows,StartFromOne,OutMIDI,OutCsound,MustChangeInput,
	SmartCursor,NotSaidKpress,**p_MemGram,**p_MemRul;
extern long ProduceStackDepth,ProduceStackIndex,DisplayStackIndex,
	**p_LastStackIndex,**p_MemPos;
extern long **p_ItemStart,**p_ItemEnd,CompileDate,ComputeStart,MaxComputeTime,
	ComputeTime,Maxitems,ItemNumber,Interrupted,MaxDeriv,Sel1,Sel2;
extern int Nplay,SynchroSignal,QuantizeOK,IgnoreCase,
	MatchWords,TargetWindow,GraphicScaleP,GraphicScaleQ;
extern long Time_res,Quantization;
extern volatile unsigned long TimeSlice;

#if WITH_REAL_TIME_SCHEDULER
extern Slice **Clock,***p_Clock,**p_AllSlices,*Slices,*SlicePool;
extern volatile unsigned long TotalTicks;
extern volatile char OKsend;
#endif
extern volatile char ClockOverFlow,SchedulerIsActive;
extern char Mute,Panic;

/* extern char AlertMute; */

// Variables for Csound instruments
extern CsoundInstrument **p_CsInstrument;
extern int **p_CsInstrumentIndex,**p_CsDilationRatioIndex,**p_CsAttackVelocityIndex,
	**p_CsReleaseVelocityIndex,**p_CsPitchIndex,**p_CsPitchBendStartIndex,
	**p_CsVolumeStartIndex,**p_CsPressureStartIndex,**p_CsModulationStartIndex,
	**p_CsPanoramicStartIndex,**p_CsPitchBendEndIndex,**p_CsVolumeEndIndex,
	**p_CsPressureEndIndex,**p_CsModulationEndIndex,**p_CsPanoramicEndIndex;
extern char ****pp_CsInstrumentName,****pp_CsInstrumentComment,**p_CsPitchFormat,
	****p_StringConstant;
extern double **p_CsPitchBend[6],**p_CsVolume[6],**p_CsPressure[6],**p_CsModulation[6],
	**p_CsPanoramic[6],**p_NumberConstant;


extern int FileSaveMode,FileWriteMode,MIDIfileType,CsoundFileFormat;

extern SoundObjectInstanceParameters **p_Instance;
extern objectspecs ****p_ObjectSpecs;
extern int WaitKey[MAXWAIT+1],WaitChan[MAXWAIT+1];
extern long **p_Flag,BufferSize,DeftBufferSize,Maxevent;
extern FILE* OpenMIDIfilePtr;
extern short HelpRefnum,TempRefnum,TraceRefnum,CsRefNum,CsScoreOpened,
	MIDIfileOpened,MIDIfileTrackEmpty;
extern int MIDIbytestate,MIDIfileTrackNumber,StepScript,TypeScript,MoreTime;
extern long MidiLen_pos,**p_Tref,**p_Tpict;
extern dword MIDItracklength,Midi_msg;
extern Milliseconds OldMIDIfileTime;
extern unsigned long LapWait,WhenItStarted;
extern int ****p_Image,****p_NoteImage,MaxGram,MaxRul,SplitTimeObjects,SplitVariables,Token,
	SpaceOn,
	VariableOn,N_err,nstore,NumberTables,OkShowExpand,Improvize,ComputeWhilePlay,Varweight,
	TransposeInput,TransposeValue,
	Flagthere,ResetDone,BolsInGrammar,NoAlphabet,**p_Ifrom,PointCsound,PointMIDI,
	**p_Resolution,
	**p_CsoundInstr,**p_CsoundAssignedInstr;
extern MIDIcode ****pp_MIDIcode;
extern float **p_CsoundTempo;
extern Milliseconds ****pp_CsoundTime;
extern long **p_CsoundSize,**p_MIDIsize;
extern char **p_AlphaCtrlNr,**p_AlphaCtrlChan,**p_DefaultChannel,**p_PasteDone,
	**p_AlphaCtrl,**p_Type,**p_FixScale,**p_OkExpand,**p_OkCompress,**p_OkRelocate,
	**p_BreakTempo,**p_DiscardNoteOffs,
	**p_OkTransp,**p_OkArticul,**p_OkVolume,**p_OkPan,**p_OkMap,**p_OkVelocity,
	**p_ContBeg,**p_ContEnd,**p_CoverBeg,**p_CoverEnd,**p_TruncBeg,**p_TruncEnd,
	**p_PivType,**p_PivMode,**p_RescaleMode,**p_DelayMode,**p_ForwardMode,
	**p_BreakTempoMode,**p_ContBegMode,**p_ContEndMode,**p_CoverBegMode,**p_CoverEndMode,
	**p_TruncBegMode,**p_TruncEndMode,**p_PreRollMode,**p_PostRollMode,
	**p_PeriodMode,**p_ForceIntegerPeriod,**p_StrikeAgain,**p_CompiledCsoundScore,
	****pp_Comment,****pp_CsoundScoreText;
extern CsoundLine ****pp_CsoundScore;
extern double **p_AlphaMin,**p_AlphaMax,**p_Quan,ForceRatio,Infpos1;
extern float **p_PivPos,**p_BeforePeriod;
extern long **p_Dur,**p_MaxDelay,**p_MaxForward,**p_PreRoll,**p_PostRoll,
	**p_MaxBegGap,**p_MaxEndGap,
	**p_MaxCoverBeg,**p_MaxCoverEnd,**p_MaxTruncBeg,**p_MaxTruncEnd;
extern short Minconc,Maxconc,Maxlevel,MaxFlag,MaxScript,PlayFromInsertionPoint,
	Jscriptline,Final,POLYconvert,
	KeepFlag,CoverOK,DiscontinuityOK,AllSolTimeSet,
	LimCompute,LimTimeSet,SkipFlag,StackFlag,Port,Portbit,****p_Seq,
	MIDI;
extern int CyclicPlay,AllowRandomize,WillRandomize,StepProduce,StepGrammars,DisplayProduce,UseTimeLimit,
	DisplayTimeSet,StepTimeSet,TraceProduce,UseEachSub,PlanProduce,TraceTimeSet;
extern int InsertGramRuleNumbers, InsertGramCorrections, InsertNewBols;
extern unsigned long Ptick[MAXTICKS],Qtick[MAXTICKS],GrandCycle[MAXTICKS],
	PosGrandCycle[MAXTICKS],GrandPeriod,BeatGrandCycle,BeatPosGrandCycle,
	LastCommonDate[MAXTICKS],LastCommonBeatDate;
extern double Pclock,Qclock,OldPclock,OldQclock,MinPclock,MaxPclock,
	MinQclock,MaxQclock,Prod,Ratio,Kpress,Pduration,Qduration;
extern long Infpos,Infneg,Veryneg,MaxMIDIbytes,
	TimeMax,Nalpha,Nbytes,Tbytes2,MIDIinputFilter,MIDIoutputFilter,MIDIinputFilterstartup,
	MIDIoutputFilterstartup,Ts,DataEnd;
extern Milliseconds **p_T;
extern long **p_Ppatt,**p_Qpatt;
extern MIDIcode **p_Code;

extern double SpeedRange,Ke,CorrectionFactor;

extern int DeriveFurther,DeriveFurtherKey,DeriveFurtherChan,ResetWeights,NeverResetWeights,
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
extern TextOffset UndoPos;
extern long CountOn;
extern unsigned long WaitStartDate,WaitEndDate;
extern char Message[MAXLIN],TheName[MAXNAME];
extern char DateMark[6];
extern PolyHandle p_Diagram[MAXDIAGRAM];
extern int Ndiagram,ObjectMode,ObjectTry;
extern int DiagramWindow[MAXDIAGRAM];
extern int Finding,Hpos;
extern PicHandle NoteScalePicture,p_Picture[MAXPICT];
extern Rect NoteScaleRect,PictRect[MAXPICT];
extern int Npicture,Offscreen;
extern int PictureWindow[MAXPICT];
extern int Charstep,UndoFlag;
extern char LineBuff[MAXLIN];
extern Str255 PascalLine;

extern long Tcurr,LastTcurr,DataOrigin;
extern unsigned long NextTickDate[MAXTICKS],NextBeatDate;

extern int Nw,LastEditWindow,LastComputeWindow,OutputWindow,ResumeStopOn,
	ResetTickFlag,ResetTickInItemFlag;
extern WindowPtr Window[WMAX];
extern DialogPtr ResumeStopPtr,ResumeUndoStopPtr,MIDIkeyboardPtr;
extern DialogPtr PatternPtr,ReplaceCommandPtr,EnterPtr,GreetingsPtr,FAQPtr,SixteenPtr,FileSavePreferencesPtr,
	StrikeModePtr,TuningPtr,DefaultPerformanceValuesPtr,CsoundInstrMorePtr,OMSinoutPtr,MIDIprogramPtr;
extern DialogPtr gpDialogs[WMAX];
/*extern DialogRecord EnterDR,ReplaceCommandDR,TuningDR,DefaultPerformanceValuesDR,CsoundInstrMoreDR,
	MIDIprogramDR,PatternDR,FileSavePreferencesDR; */

extern TextHandle TEH[WMAX];

extern int linesInFolder[WMAX];
extern Boolean WindowFullAlertLevel[WMAX];
extern MenuHandle myMenus[MAXMENU+1];
extern int Buttonheight;
extern int NewEnvironment,NewColors,ShowPianoRoll,ToldAboutPianoRoll,ShowObjectGraph,
	Help,ChangedCoordinates[WMAX];
extern RGBColor CurrentColor[WMAX];
extern ControlHandle vScroll[WMAX],hScroll[WMAX];
extern int SlideH[WMAX],SlideV[WMAX];
extern int WindowTextSize[WMAX];
extern short GrafWindow[WMAX];
extern short LockedWindow[WMAX];
extern short OKvScroll[WMAX],OKhScroll[WMAX];
extern short OKgrow[WMAX];
extern short Adjustable[WMAX];
extern short NeedSave[WMAX];
extern int Freebottom[WMAX];
extern short Editable[WMAX],IsHTML[WMAX],IsText[WMAX],Weird[WMAX];
extern short HasFields[WMAX];
extern short IsDialog[WMAX];
extern Boolean WindowUsesThemeBkgd[WMAX];
extern int FileTypeIndexToWindowFileIndex[MAXFILETYPEINDEX];
extern short gFileType[WMAX];
extern char **p_FileInfo[WMAX];
extern Cursor EditCursor,WatchCursor,CrossCursor,HelpCursor,KeyboardCursor,
	WheelCursor[4],FootCursor[8],DiskCursor[2];
extern int Jwheel,Jfeet,Jdisk;
extern int Jbutt;
extern ControlHandle Hbutt[MAXBUTT];
extern int Dirty[WMAX],CompiledGr,CompiledPt,CompiledAl,CompiledIn,CompiledGl,
	CompiledCsObjects,CompiledRegressions,LoadedIn,LoadedGl,LoadedCsoundInstruments,
	Interactive,NotFoundMetronom,NotFoundNatureTime;
extern int NotBPCase[MAXNOTBPCASES];
extern int Hmin[WMAX],Hmax[WMAX],Hzero[WMAX],Vmin[WMAX],Vmax[WMAX],Vzero[WMAX];
extern int Created[WMAX];
extern char FileName[WMAX][MAXNAME+1],CsFileName[MAXNAME+1],MIDIfileName[MAXNAME+1],
	CsoundOrchestraName[MAXNAME+1];
extern FSSpec MIDIfileSpec;
extern float MIDIfadeOut;
extern int CurrentVolume[MAXCHAN+1];
extern char FilePrefix[WMAX][5];
extern char FileExtension[WMAX][6];
extern char DocumentTypeName[WMAX][21];
extern char DeftName[WMAX][MAXNAME];
extern char WindowName[WMAX][MAXNAME];
extern long WindowParID[WMAX],ParIDstartup,ParIDbp2;
// extern Str255 DeftVolName;
extern short TheVRefNum[WMAX],RefNumStartUp,RefNumbp2;

extern int Nature_of_time,UseBufferLimit,NeedAlphabet,CompleteDecisions,OkWait;
extern int PrototypeTickChannel,PrototypeTickKey,PrototypeTickVelocity;
extern int TickChannel[MAXTICKS],TickKey[MAXTICKS],TickVelocity[MAXTICKS],MuteTick[MAXTICKS],
	TickDuration[MAXTICKS],
	TickCycle[MAXTICKS],TickPosition[MAXTICKS],iTick,jTick,
	HideTicks,PlayTicks,UseBullet,NeedZouleb;
extern long ThisTick[MAXTICKS][MAXBEATS];
extern int InitOn,SetTimeOn,ComputeOn,PolyOn,CompileOn,SoundOn,SelectOn,PrintOn,InputOn,
	ClickRuleOn,GraphicOn,ReadKeyBoardOn,ButtonOn,PauseOn,AlertOn,AllOn,PlaySelectionOn,
	PlayPrototypeOn,MIDIfileOn,WaitOn,ExpandOn,ReceivedOpenAppEvent,Oms,GotAlert,NewOrchestra,ChangedMIDIprogram,
	AEventOn,LoadOn,SaveOn,SelectPictureOn,InBuiltDriverOn,EnterOn,Option,EmptyBeat,TickDone,TickThere,
	FoundNote,NoCursor,ItemOutPutOn,ItemCapture,TickCapture,TickCaptureStarted,AskedAboutCsound;

extern double MaxTempo,InvMaxTempo,TokenLimit,InvTokenLimit;
extern double MaxFrac;
extern Boolean HaveAppearanceManager;
extern Boolean RunningOnOSX;
extern ControlActionUPP vScrollUPP,hScrollUPP;
extern NSWReply** MIDIfileReply;
extern NSWReply** CsFileReply;
extern FSSpec TempSpec;
extern UInt32 NumInstalledDrivers;
extern BPMidiDriver** InstalledDrivers;
extern DynamicMenuItem** InstalledMenuItems;

#endif /* BP2_DECL_H */
