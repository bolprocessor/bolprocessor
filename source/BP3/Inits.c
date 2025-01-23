/* Inits.c (BP3) */

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


#ifndef _H_BP3
#include "-BP3.h"
#endif

#include "-BP3decl.h"

// REVISED VERSION 2024-12-28 using "console_strings.json" instead of "StringLists.h"

char StringsJason[] = "console_strings.json";
char ScriptCommand[MAX_STRINGLISTS_NUMBER][MAX_STRINGLISTS_LEN];
char GramProcedure[MAX_STRINGLISTS_NUMBER][MAX_STRINGLISTS_LEN];
char PerformanceControl[MAX_STRINGLISTS_NUMBER][MAX_STRINGLISTS_LEN];
char HTMLdiacritical[MAX_STRINGLISTS_NUMBER][MAX_STRINGLISTS_LEN];

int trace_scriptcommands = 0;

int Inits(void) {
	int i,j,ch;
	OSStatus io;
	char **ptr;
	Rect r;
	long handlerRefcon;
	// FSSpec spec;
	long t;
	long LastTime = ZERO;
	long PianorollShift = ZERO;

	Beta = NO;

	Time_res = 10L; /* Time resolution for MIDI messages */
	// Quantization = 10L;

	Nw = 0;

	rtMIDI = OutCsound = WriteMIDIfile =  Capture0n = FALSE;
	CaptureSource = -1;
	CapturePtr = NULL;
	
	InitOn = NoCursor = NotSaidKpress = TRUE;
	ReceivedOpenAppEvent = FALSE;
	SaidChannel = 0;
	CheckMem = TRUE; EmergencyExit = FixedMaxQuantization = FALSE;
	EventState = NO;
	SetTimeOn = ComputeOn = PolyOn = CompileOn = SoundOn = SelectOn = ButtonOn = ExpandOn
		= PrintOn = ClickRuleOn = GraphicOn = CompleteDecisions = LoadOn = SaveOn = MIDIfileOn
		= ReadKeyBoardOn = AlertOn = HangOn = ScriptRecOn = PlayPrototypeOn
		= PlaySelectionOn = PlayChunks = PlayAllChunks = UseEachSub = SelectPictureOn = TypeScript = InputOn = EnterOn = AEventOn = HideMessages
		= PauseOn = WaitOn = ItemOutPutOn = ItemCapture = TickCapture = TickCaptureStarted
		= AskedAboutCsound = MustChangeInput = ToldSkipped = ShownBufferSize = FALSE;
	Option = TickDone = FoundNote = GotAlert = UsedRandom = SaidTooComplex = FALSE;
	POLYconvert = OkShowExpand = FALSE;
	NewOrchestra = TRUE;
	ItemNumber = ZERO;
	MaxItemsProduce = 20;
	AssignedTempoCsoundFile = FALSE;
	MaxConsoleTime = 0; // No limit
	Ratio = 0.;  Prod = 1.;
	N_image = 0; imagePtr = outPtr = NULL;
	strcpy(OutFileName,"");
	NumberScales = 0;
	DefaultScaleParam = -1; MaxScales = 2; Scale = NULL;
	ToldAboutScale = FALSE;
	WarnedBlockKey = WarnedRangeKey  = FALSE;
	TimeMax = MAXTIME;
	Nalpha = 100L; SpeedRange = 6.;
	CorrectionFactor = 1.;
	Chunk_number = 0;
	NextStop = 0L;

	Oms = FALSE;

	for(i=0; i < MAXMESSAGE; i++) {
		ptr = (char**) GiveSpace((Size)(MAXLIN * sizeof(char)));
		if(ptr == NULL) return(ABORT);
		p_MessageMem[i] = ptr;
		(*p_MessageMem[i])[0] = '\0';
		}
	Jmessage = 0;

	for(i=0; i < WMAX; i++) {
		ptr = (char**) GiveSpace((Size)(MAXINFOLENGTH * sizeof(char)));
		if(ptr == NULL) return(ABORT);
		p_FileInfo[i] = ptr;
		(*p_FileInfo[i])[0] = '\0';
		}

	MaxMIDIbytes = ZERO;

	KeyboardType = QWERTY;
	C4key = 60;  A4freq = 440.;
	DefaultBlockKey =  60; // Block frequency on C4 wheen changing scales
	ProgNrFrom = 1;	/* This has changed with version 2.7.3 */
	TestMIDIChannel = 1;
	for(i=0; i <= MAXCHAN; i++) {
		CurrentMIDIprogram[i] = 0;
		}
	ChangedMIDIprogram = FALSE;

	Nbytes = Tbytes2 = ZERO;

	MIDIinputFilterstartup = FILTER_ALL_ON;
	MIDIoutputFilterstartup = FILTER_ALL_OFF;
	for(i = 0; i < MAXPORTS; i++) {
		MIDIacceptFilter[i] = FILTER_ALL_ON;
		MIDIpassFilter[i] = FILTER_ALL_OFF;
		my_sprintf(MIDIchannelFilter[i],"%s","1111111111111111");
		my_sprintf(MIDIpartFilter[i],"%s","111111111111111111111111111111");
	//	BPPrintMessage(0,odInfo,"Init: MIDIchannelFilter[%d] = %s\n",i,MIDIchannelFilter[i]);
		}
	ResetMIDIFilter();

	LapWait = ZERO;
	PrefixTree.p = SuffixTree.p = NULL;
	PrefixTree.accept = SuffixTree.accept = FALSE;
	SmartCursor = Mute = Panic = ClockOverFlow = SchedulerIsActive = FALSE;
	WarnedBasedKey = FALSE;

	// Limits of speed and scale values
	TokenLimit = (((double)TOKBASE) * ((double)TOKBASE)) - 1.;
	InvTokenLimit = 1. / TokenLimit;
	MaxTempo = 100000.;
	MaxFrac = 1000000.;
	InvMaxTempo = 1. / MaxTempo;
	//

	Stream.code = NULL;
	Stream.imax = ZERO; Stream.cyclic = FALSE; Stream.period = ZERO;

	for(ch = 0; ch < MAXCHAN; ch++) {
		MPEnote[ch] = 0;
		MPEold_note[ch] = MPEnew_note[ch] = -1;
		MPEscale[ch] = -1;
		MPEpitch[ch] = -1;
		}

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

	if(MakeWindows() != OK) return(ABORT);

	FirstNoteOn = TRUE;
	OutBPdata = FALSE;
	ObjectMode = ObjectTry = Improvize = StepProduce = TraceMicrotonality
		= PlanProduce = DisplayProduce = UseEachSub
		= TraceProduce = DisplayTimeSet = StepTimeSet = TraceTimeSet = TraceNoteOn = ResetNotes
		= ShowGraphic = ComputeWhilePlay = NeverResetWeights = FALSE;
	SynchronizeStart = CyclicPlay = NoConstraint = AllItems
	 = CsoundTrace = WillRandomize = FALSE;
	ResetWeights = ResetFlags = ResetControllers = ShowMessages
		= AllowRandomize = TRUE;
	NoteConvention = ENGLISH;
	Offscreen = FALSE;
	Nw = -1; Ndiagram = Npicture = 0;
	LastEditWindow = OutputWindow = wData;
	LastComputeWindow = wGrammar;
	LastAction = NO;
	Ke = log((double) 2.) / 64.;
	strcpy(Message,"");
	/* PictFrame.top = topDrawPrototype;
	PictFrame.left = leftDrawPrototype;
	PictFrame.bottom = bottomDrawPrototype;
	PictFrame.right = rightDrawPrototype; */
	// NoteScalePicture = NULL;
	p_Tpict = NULL; Hpos = -1;
	Jbol = Jfunc = iProto = Jpatt = Jvar = Jflag = Jhomo = N_err = BolsInGrammar
		= ScriptExecOn = 0;
	Jcontrol = -1;
	for(i=0; i < MAXPARAMCTRL; i++) ParamControl[i] = ParamKey[i] = ParamChan[i] = -1;
	MaxRul = MaxGram = 0;
	Gram.trueBP = Gram.hasTEMP = Gram.hasproc = FALSE;
	pp_MIDIcode = NULL;
	pp_CsoundTime = NULL;
	p_Code = NULL;
	p_Type = NULL;
	pp_Comment = pp_CsoundScoreText = NULL;
	p_CsoundSize = NULL;
	pp_CsoundScore = NULL;
	p_CsoundTables = NULL;
	MaxCsoundTables = 0;
	NumberCharsTrace = NumberCharsData = NumberInferences =  0L;
	ClockInitCapture = -1L;
	PointCsound = PointMIDI = FALSE;
	CsFileName[0] = MIDIfileName[0] = CsoundOrchestraName[0] = '\0';
	EndFadeOut = 2.; AdvanceTime = 0.0;
	MIDIfileOpened = FALSE;
	// MIDIfileReply = CsFileReply = NULL;
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
	FileSaveMode = ALLSAME;
	FileWriteMode = NOW;
	ConvertMIDItoCsound = FALSE;
	MIDIfileType = 1;
	CsoundFileFormat = MAC;

	for(i=0; i < 6; i++) {
		p_CsPitchBend[i] = p_CsVolume[i] = p_CsPressure[i] = p_CsModulation[i]
			= p_CsPanoramic[i] = NULL;
		}
		
	NoAlphabet = TRUE;
	StartFromOne = TRUE;
	MIDIsetUpTime = 700L;	/* ms */
	NewEnvironment = NewColors = Help = FALSE;
	for(i=0; i < WMAX; i++) ChangedCoordinates[i] = Dirty[i] = FALSE;
	ObjectMode = ObjectTry = Final = LoadedScript = FALSE;
	LoadedIn = LoadedGl = FALSE;
	TransposeInput = FALSE; TransposeValue = 0;
	CompiledGr = CompiledAl = CompiledPt = CompiledIn = CompiledGl = CompiledCsObjects
		= CompiledRegressions = NotFoundMetronom = NotFoundNatureTime = ToldPitchbend = ToldStop = 0;
	Pclock = Qclock = 1.;
	Nature_of_time = STRIATED;
	Pduration = 0.;  Qduration = 1.;
	UseBufferLimit = FirstTime = WaitForSpace = WaitForEver = FALSE;
	OkWait = TRUE;
	Nplay = 1;	/* Number of times each item is played */
	SynchroSignal = OFF;
	MaxDeriv = MAXDERIV;

	Infneg = LONG_MIN;
	Veryneg = Infneg + 1;
	Infpos = - Veryneg;
	Infpos1 = Infpos + 1.;
	// BPPrintMessage(0,odError,"Infneg = %ld\n",Infneg);
	InsertGramRuleNumbers = InsertGramCorrections = InsertNewBols = (BP_CARBON_GUI_FORGET_THIS ? TRUE : FALSE);
	SplitTimeObjects = TRUE;	/* Terminal symbols separated by spaces */
	SplitVariables = FALSE;	/* SplitVariables <=> variables displayed between '||' */
	Token = SpaceOn = FALSE;	/* Typing text, not tokens */
	FileName[iObjects][0] = '\0';
	BufferSize = DeftBufferSize = MAXDISPL;
	GraphicScaleP = GraphicScaleQ = 1L;
	MatchWords = Finding = FALSE; IgnoreCase = TRUE;
	strcpy(FindString,""); strcpy(ReplaceString,""); // Fixed by BB 2022-02-20
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
	NumEventsWritten = 0L;
	p_Flag = NULL;
	p_MemGram = p_MemRul = p_VarStatus = NULL;
	p_MemPos = p_LastStackIndex = NULL;
	p_ItemStart = p_ItemEnd = NULL;
	pp_Scrap = &p_Scrap;
	p_StringList = NULL; pp_StringList = &p_StringList; NrStrings = 0;
	if(ResetScriptQueue() != OK) return(ABORT);
	p_InitScriptLine = NULL;

	Maxinscript = 8; Jinscript = 0;
	if((p_INscript=(INscripttype**) GiveSpace((Size) Maxinscript * sizeof(INscripttype)))
		== NULL) return(ABORT);
	for(i=0; i < Maxinscript; i++) ((*p_INscript)[i]).chan = -1;

	Maxoutscript = 8; Joutscript = 0;
	if((p_OUTscript=(OUTscripttype**) GiveSpace((Size) Maxoutscript * sizeof(OUTscripttype)))
		== NULL) return(ABORT);
	for(i=0; i < Maxoutscript; i++) ((*p_OUTscript)[i]).chan = -1;

	ShowPianoRoll = ToldAboutPianoRoll = FALSE;
	ShowObjectGraph = TRUE;

	p_Instance = NULL;
	p_ObjectSpecs = NULL;

	p_Seq = NULL;
	MaxVar = MaxFlag = 0;
	Gram.p_subgram = GlossGram.p_subgram = NULL;
	Gram.number_gram = GlossGram.number_gram = 0;
	RunningStatus = 0; NoRepeat = FALSE;
	ScriptSyncKey = ScriptSyncChan = -1;
	StrikeAgainDefault = TRUE;
	StopPauseContinue = TRUE;
	MIDImicrotonality = FALSE;
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
	if(SetNoteNames() != OK) return(ABORT);
	NeedAlphabet = FALSE;
	for(i=0; i < MAXCHAN; i++) {
		ChangedVolume[i] = ChangedPanoramic[i] = ChangedModulation[i] = ChangedPitchbend[i] = ChangedSwitch[i]
			= ChangedPressure[i] = FALSE;
		WhichCsoundInstrument[i+1] = -1;
		}
	PrototypeTickChannel = 1; PrototypeTickKey = 84;
	TickKey[0] = 96;
	TickKey[1] = 84;
	TickKey[2] = 72;
	PrototypeTickVelocity = 64;
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
		}
	iTick = jTick = -1;
	strcpy(Message,WindowName[wCsoundTables]);

	// MaxHandles = ZERO;
	PedalOrigin = -1;
	Nalpha = 100L;
	Jinstr = 0;
	NeedZouleb = 0;
	UseBullet = TRUE;
	// Code[7] = '�';
	Code[7] = '.'; // Fixed by BB 2022-02-18
	if(ResizeCsoundInstrumentsSpace(1) != OK) return(ABORT);
	iCsoundInstrument = 0;
	ResetCsoundInstrument(iCsoundInstrument,YES,NO);
	for(i=1; i <= MAXCHAN; i++) WhichCsoundInstrument[i] = -1; // FIXME: this is done above too?
	return(OK);
	}


int SetNoteNames(void) {
	int i,j,notenum,octave;
	char **ptr;

	// MoreConvention = MAXCONVENTIONS;
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
							my_sprintf(Message,"%s000",Frenchnote[notenum]); break;
						case -1:
							my_sprintf(Message,"%s00",Frenchnote[notenum]); break;
						default:
							my_sprintf(Message,"%s%ld",Frenchnote[notenum],(long)octave);
							break;
						}
					break;
				case ENGLISH:
					octave--;
					switch(octave) {
						case -1:
							my_sprintf(Message,"%s00",Englishnote[notenum]); break;
						default:
							my_sprintf(Message,"%s%ld",Englishnote[notenum],(long)octave);
							break;
						}
					break;
				case INDIAN:
					octave--;
					switch(octave) {
						case -1:
							my_sprintf(Message,"%s00",Indiannote[notenum]); break;
						default:
							my_sprintf(Message,"%s%ld",Indiannote[notenum],(long)octave);
							break;
						}
					break;
				default:
			//	case KEYS:
			//	case CUSTOM:
					my_sprintf(Message,"%s%ld",KeyString,(long)j);
					break;
				}
		//	BPPrintMessage(0,odInfo,"key = %d notenum = %d octave = %d  name = %s\n",j,notenum,octave,Message);
			MystrcpyStringToTable(p_NoteName[i],j,Message);
			(*(p_NoteLength[i]))[j] = MyHandleLen((*(p_NoteName[i]))[j]);
			octave = (j - notenum) / 12;
			if(notenum == 0) octave--;
			if(notenum == 11) octave++;
			switch(i) {
				case FRENCH:
					octave -= 2;
					switch(octave) {
						case -2:
							my_sprintf(Message,"%s000",AltFrenchnote[notenum]); break;
						case -1:
							my_sprintf(Message,"%s00",AltFrenchnote[notenum]); break;
						default:
							my_sprintf(Message,"%s%ld",AltFrenchnote[notenum],(long)octave);
							break;
						}
					break;
				case ENGLISH:
					octave--;
					switch(octave) {
						case -1:
							my_sprintf(Message,"%s00",AltEnglishnote[notenum]); break;
						default:
							my_sprintf(Message,"%s%ld",AltEnglishnote[notenum],(long)octave);
							break;
						}
					break;
					break;
				case INDIAN:
					octave--;
					switch(octave) {
						case -1:
							my_sprintf(Message,"%s00",AltIndiannote[notenum]); break;
						default:
							my_sprintf(Message,"%s%ld",AltIndiannote[notenum],(long)octave);
							break;
						}
					break;
				default:
			//	case KEYS:
			//	case CUSTOM:
					my_sprintf(Message,"%s%ld",KeyString,(long)j);
					break;
				}
		//	BPPrintMessage(0,odInfo,"ALT key = %d notenum = %d octave = %d  name = %s\n",j,notenum,octave,Message);
			MystrcpyStringToTable(p_AltNoteName[i],j,Message);
			(*(p_AltNoteLength[i]))[j] = MyHandleLen((*(p_AltNoteName[i]))[j]);
			}
		}
	for(i=0; i < 12; i++) NameChoice[i] = 0;
	return(OK);
	}


int Ctrlinit(void) {
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
				//	BPPrintMessage(0,odInfo,"Ctrlinit(%d). igram = %d irul = %d weight = %ld\n",i,igram,irul,(long)k);
					}
				if((*((*(Gram.p_subgram))[igram].p_rule))[irul].repeatcontrol == i) {
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].repeat = k;
					}
				}
			}
	/*	my_sprintf(Message,"K%ld = %ld ",(long)i,(long)k);
		Print(wTrace,Message); */
		}
	return(OK);
	}


int MakeWindows(void)
{
int i,id,im,ibot,itemtype,j,k,km,w,top,left,bottom,right,leftoffset,
	widmax,type,proc,bad,x0,y0, oldheight;
Handle h_res;
Rect r, rw;
Str255 title;
long rc;
GrafPtr saveport;
OSErr err;


/* This initialization needs to occur even in non-GUI builds ?? */
ResumeStopOn = FALSE;
Jbutt = 0;
for(w=0; w < WMAX; w++) {
	IsHTML[w] = FALSE;
	WindowFullAlertLevel[w] = 0;
	Weird[w] = FALSE;
	SetUpWindow(w);		// this is to create text buffers
	}
IsHTML[wCsoundTables] = TRUE;

#if BP_CARBON_GUI_FORGET_THIS
for(w=0; w < MAXWIND; w++) {
	PleaseWait();
	if((Window[w] = GetNewCWindow(WindowIDoffset+w, NULL, 0L)) == NULL) {
		my_sprintf(Message,"Can't load resource window ID#%ld",
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
	if(err != noErr) BPPrintMessage(0,odError,"=> Err MakeWindows().  InvalWindowRect returned non-zero.");
	}

GetWindowPortBounds(Window[wMessage], &r);
AdjustWindow(FALSE,wMessage,r.top,r.left,r.bottom,r.right);
GetWindowPortBounds(Window[wInfo], &r);
AdjustWindow(FALSE,wInfo,r.top,r.left,r.bottom,r.right);
#endif /* BP_CARBON_GUI_FORGET_THIS */

#if BP_CARBON_GUI_FORGET_THIS
FlashInfo("Bol Processor Mac OS X (Carbon) GUI");
#else
FlashInfo("Bol Processor console app");
#endif

my_sprintf(Message,"%s",IDSTRING);
ShowMessage(TRUE,wMessage,Message);

Jbutt = 28;  // number of buttons must not be less than that found within settings files

return(OK);
}

/* Non-Carbon version just creates TextHandles to use */
int SetUpWindow(int w)
{	
	if(w < 0 || w >= WMAX) {
		BPPrintMessage(0,odError,"Internal problem in setting up text buffers!");
		return(ABORT);
	}
	
	if(Editable[w]) {
		TEH[w] = NewTextHandle();
		if(TEH[w] == NULL) return(ABORT);
	}
	else TEH[w] = NULL;
	Dirty[w] = FALSE;
	return(OK);
}

int LoadStrings(void) {
	long max;
	int i;

	p_GramProcedure = p_PerformanceControl = p_GeneralMIDIpatch = p_HTMLdiacrList = NULL;
	p_ProcNdx = p_ProcNArg = p_PerfCtrlNdx = p_GeneralMIDIpatchNdx = p_PerfCtrlNArg = NULL;
	MaxProc = MaxPerformanceControl = ZERO;

	if(LoadStringResource(&p_GramProcedure,&p_ProcNdx,&p_ProcNArg,GramProcedureStringsID,
		&MaxProc,YES) != OK) return(ABORT);
	if(LoadStringResource(&p_PerformanceControl,&p_PerfCtrlNdx,&p_PerfCtrlNArg,
		PerformanceControlStringsID,&MaxPerformanceControl,YES) != OK) return(ABORT);
	if(LoadStringResource(&p_HTMLdiacrList,NULL,NULL,
		HTMLdiacriticalID,&max,YES) != OK) return(ABORT);
	if(LoadScriptCommands() != OK) return(ABORT);
	return(OK);
	}

int LoadStringResource(char***** pp_str,int ***pp_ndx,int ***pp_narg,int id,long *p_max,
	int lock) {
	int i,im,j,j0,k,km,result;
	char **ptr;
	char buffer[MAX_STRINGLISTS_LEN];
	Handle h_res;

	char strarray[MAX_STRINGLISTS_NUMBER][MAX_STRINGLISTS_LEN];
	switch(id) {
		case GramProcedureStringsID:
			if((result = ReadJason(&im,"GramProcedure",strarray)) != OK) return result;
			break;
		case PerformanceControlStringsID:
			if((result = ReadJason(&im,"PerformanceControl",strarray)) != OK) return result;
			break;
		case HTMLdiacriticalID:
			if((result = ReadJason(&im,"HTMLdiacritical",strarray)) != OK) return result;
			break;
		default:
			 fprintf(stderr, "=> Warning! Bad STR# id in LoadStringResource().\n");
			return MISSED;
			break;
		}
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
	for(i=0,j=1; i < im; i++) {
		// In console build, we set buffer to each consecutive C string in strarray and
		// j to the non-existent "length byte" at position -1 (it will be incremented before use)
		strcpy(buffer,strarray[i]);
		buffer[MAX_STRINGLISTS_LEN - 1] = '\0';
		j = -1;
		km = strlen(buffer);
		j0 = j;
		if(km == 0) goto ERR;
		if(pp_ndx != NULL) {
			j++;
			k = GetInteger(NO,buffer,&j);
			if(k == INT_MAX) goto ERR;
			(**pp_ndx)[i] = k;
			}
		if(pp_narg != NULL) {
			j++;
			k = GetInteger(NO,buffer,&j);
			if(k == INT_MAX) goto ERR;
			(**pp_narg)[i] = k;
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
	return(OK);

	ERR:
	BPPrintMessage(0,odError,"=> Error loading %ldth string in resource list ID %ld",
		(long)i,(long)id);
	Panic = TRUE;
	return(ABORT);
	}

int ReadJason(int* p_im, char* key, char the_array[][MAX_STRINGLISTS_LEN]) {
	int i = 0;
	char *json_data = read_file(StringsJason);
    if(!json_data) {
		BPPrintMessage(0,odError,"=> Could not open file “php/%s”\n",StringsJason);
		Panic = TRUE;
		return ABORT;
		}
	cJSON *json = cJSON_Parse(json_data);
    if(!json) {
        BPPrintMessage(0,odError,"=> Could not parse “php/%s”: %s\n",StringsJason,cJSON_GetErrorPtr());
		free(json_data);
		Panic = TRUE;
        return ABORT;
    	}
	cJSON *scriptCommandArray = cJSON_GetObjectItem(json,key);
	if(!cJSON_IsArray(scriptCommandArray)) {
        BPPrintMessage(0,odError,"=> “%s” was not found in “php/%s”\n",key,StringsJason);
		cJSON_Delete(json);
		free(json_data);
		Panic = TRUE;
		return ABORT;
		}
    cJSON *item;
    cJSON_ArrayForEach(item,scriptCommandArray) {
        if(i >= MAX_STRINGLISTS_NUMBER) {
			BPPrintMessage(0,odError,"=> Too many “%s” in “php/%s”\n",key,StringsJason);
            break;
        	}
        if(cJSON_IsString(item)) {
            strncpy(the_array[i],item->valuestring,MAX_STRINGLISTS_LEN - 1);
            the_array[i][MAX_STRINGLISTS_LEN -1] = '\0';
            i++;
			}
		}
	*p_im = i;
    cJSON_Delete(json);
    free(json_data);
	return OK;
	}


int LoadScriptCommands() {
	int i,im,result,ilabel,iarg,j,k,script_length,kk,nargs,nmax_args,index,kjn;
	char c,**ptr;

	if((result = ReadJason(&im,"ScriptCommand",ScriptCommand)) != OK) return result;
	MaxScriptInstructions = 0; 
	if((h_Script = (scriptcommandtype**) GiveSpace((Size)im * sizeof(scriptcommandtype))) == NULL)
		goto ERR2;
	if((h_ScriptIndex = (int**) GiveSpace((Size)im * sizeof(int))) == NULL)
		goto ERR2;
	for(i=0; i < im; i++) {
		ScriptLabel(i) = ScriptArg(i) = NULL;
		ScriptNrLabel(i) = ScriptNrArg(i) = 0;
		}
	nmax_args = 0;
	for(i=0; i < im; i++) {
		script_length = strlen(ScriptCommand[i]);
		if(script_length == 0) {
			BPPrintMessage(0,odError,"Empty script command has been found\n");
			goto OVER;
			}
		MaxScriptInstructions++;
		j = ilabel = iarg = nargs = 0;
		for(k=0; k < script_length; k++) {
			if(ScriptCommand[i][k] == '_') nargs++;
			}
		nargs = nargs / 2; // Number of arguments is half the number of '_'
		if(nargs > nmax_args) nmax_args = nargs;
		if(trace_scriptcommands) BPPrintMessage(0,odInfo,"\nScriptCommand[%d] = %s (%d args)\n",i,ScriptCommand[i],nargs);
		ScriptNrArg(i) = 0; // 2024-07-06
		if((ScriptLabel(i) = (char****) GiveSpace((Size)(nargs+1) * sizeof(char**)))
			== NULL) goto ERR2;
		if(nargs > 0 && ((ScriptArg(i) = (char****) GiveSpace((Size)(nargs) * sizeof(char**))) == NULL)) goto ERR2;
	//	if((ScriptArg(i) = (char****) GiveSpace((Size)(nargs) * sizeof(char**))) == NULL) goto ERR2;
		index = 0;
		while((c=ScriptCommand[i][j]) != ' ') {
			if(c >= '0' && c <= '9')
				index = (10 * index) + c - '0';
			else {
				BPPrintMessage(0,odError,"Incorrect index in script command [%d] = %s\n",i,ScriptCommand[i]);
				return ABORT;
				}
			j++;
			}
		(*h_ScriptIndex)[i] = index;
		if(trace_scriptcommands) BPPrintMessage(0,odInfo,"index = %d\n",(*h_ScriptIndex)[i]);

	NEWLABELPART:
		while((c=ScriptCommand[i][j]) == ' ' || c == '_') j++;
		if(j >= script_length) {
			continue; // To next script
			}
		k = j; while(k < script_length && (c=ScriptCommand[i][k]) != '_') k++;
		k--; while((c=ScriptCommand[i][k]) == ' ') k--; k++;
		kjn = k - j + 2;
		if(trace_scriptcommands) BPPrintMessage(0,odInfo,"size of next label = %d\n",kjn);
		if(kjn < 1) {
			BPPrintMessage(0,odError,"Error in script label: kjn = %d\n",kjn);
			goto ERR2;
			}
		ptr = (char**) GiveSpace((Size)(kjn + 1) * sizeof(char));
		if((p_ScriptLabelPart(i,ilabel) = ptr) == NULL) goto ERR2;
		k -= j;
		for(kk=0; kk < k; j++,kk++) {
			c = ScriptCommand[i][j];
			(*(p_ScriptLabelPart(i,ilabel)))[kk] = c;
			}
		(*(p_ScriptLabelPart(i,ilabel)))[kk] = '\0';
		if(trace_scriptcommands) BPPrintMessage(0,odInfo,"ScriptLabel[%d] = %s\n",ilabel,(*(p_ScriptLabelPart(i,ilabel))));
		ScriptNrLabel(i) = ++ilabel;

	NEWARGPART:
		while((c=ScriptCommand[i][j]) == ' ' || c == '_') j++;
		if(j >= script_length) {
			continue;
			}
		k = j; while(k < script_length && (c=ScriptCommand[i][k]) != '_') k++;
		kjn = k - j + 2;
		if(trace_scriptcommands) BPPrintMessage(0,odInfo,"size of next arg = %d\n",kjn);
		if(kjn < 1) {
			BPPrintMessage(0,odError,"Error in script arg: kjn = %d\n",kjn);
			goto ERR2;
			}
		ptr = (char**) GiveSpace((Size)(kjn) * sizeof(char));
		if((p_ScriptArgPart(i,iarg) = ptr) == NULL) goto ERR2;
		k -= j;
		for(kk=0; kk < k; j++,kk++) {
			c = ScriptCommand[i][j];
			(*(p_ScriptArgPart(i,iarg)))[kk] = c;
			}
		(*(p_ScriptArgPart(i,iarg)))[kk] = '\0';
		if(trace_scriptcommands) BPPrintMessage(0,odInfo,"ScriptArg[%d] = %s\n",iarg,(*(p_ScriptArgPart(i,iarg))));
		ScriptNrArg(i) = ++iarg;
		goto NEWLABELPART;
		}

	OVER:
	nmax_args++;
	if((ScriptLine.label=(char****) GiveSpace((Size)(nmax_args) * sizeof(char**))) == NULL)
		goto ERR2;
	if((ScriptLine.arg=(char****) GiveSpace((Size)(nmax_args) * sizeof(char**))) == NULL)
		goto ERR2;
	if((ScriptLine.intarg=(long**) GiveSpace((Size)(nmax_args) * sizeof(long))) == NULL)
		goto ERR2;
	if((ScriptLine.floatarg=(double**) GiveSpace((Size)(nmax_args) * sizeof(double))) == NULL)
		goto ERR2;
	if((ScriptLine.ularg=(unsigned long**) GiveSpace((Size)(nmax_args) * sizeof(unsigned long))) == NULL)
		goto ERR2;
	for(i=0; i < nmax_args; i++) {
		if((ptr=(char**) GiveSpace((Size)MAXLIN * sizeof(char))) == NULL) goto ERR2;
		(*(ScriptLine.label))[i] = ptr;
		if((ptr=(char**) GiveSpace((Size)MAXLIN * sizeof(char))) == NULL) goto ERR2;
		(*(ScriptLine.arg))[i] = ptr;
		}
	if(trace_scriptcommands) BPPrintMessage(0,odInfo,"\nAll %d script instructions have been loaded\n\n",im);
	// EmergencyExit = TRUE;
	return(OK);
	ERR:
	BPPrintMessage(0,odError,"=> Error empty line in ScriptCommand %d (StringLists.h)\n",i);
	ERR3:
	EmergencyExit = TRUE;
	return(MISSED);
	ERR2:
	BPPrintMessage(0,odError,"=> Error making space for label of script. Insufficient memory\n");
	goto ERR3;
	}

/* int InitButtons(void) {
	FirstNoteOn = TRUE;
	OutBPdata = FALSE;
	ObjectMode = ObjectTry = Improvize = StepProduce = TraceMicrotonality
		= PlanProduce = DisplayProduce = UseEachSub
		= TraceProduce = DisplayTimeSet = StepTimeSet = TraceTimeSet = ResetNotes
		= ShowGraphic = ComputeWhilePlay = NeverResetWeights = FALSE;
	SynchronizeStart = CyclicPlay = NoConstraint = AllItems
		= WriteMIDIfile = CsoundTrace = WillRandomize = FALSE;
	ResetWeights = ResetFlags = ResetControllers = ShowMessages
		= AllowRandomize = TRUE;
	NoteConvention = ENGLISH;
	return(OK);
	} */