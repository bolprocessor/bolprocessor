/* -BP2.h (BP2 version CVS) */

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

// --------------------------------------------------------------------
// ------  This is the common header used by Bol Processor BP2 --------
// --------------------------------------------------------------------

#ifndef _H_BP2
#define _H_BP2

#define SHORT_VERSION "2.999..."
#define IDSTRING ( "Version " SHORT_VERSION " (" __DATE__ ")" )
#define MAXVERSION 31

#ifndef __POWERPC
#define __POWERPC
#endif

// 1 for debugging memory, 0 otherwise
#ifndef BIGTEST
#define BIGTEST 0
#endif

// 1 to execute Beta tests for data validity, 0 otherwise
#ifndef COMPILING_BETA
#define COMPILING_BETA 0
#endif

// Select compilation options

// MACOS is currently unused 
#ifndef MACOS
#define MACOS 1
#endif

// 1 if building the OS X Carbon GUI, 0 otherwise
#ifndef BP_CARBON_GUI
#define BP_CARBON_GUI 1
#endif

// This allows the compiler to select low-level toolbox procedures.
#ifndef _FASTCODE
#define _FASTCODE 1
#endif

// Using CopyBits for off-screen graphics when NEWGRAPH is 1 (incomplete)
#ifndef NEWGRAF
#define NEWGRAF 0
#endif

#ifndef OBSOLETE
#define OBSOLETE 0
#endif

// 1 to use WASTE for text editing, 0 to use TextEdit or MLTE instead
#ifndef WASTE
#define WASTE 0
#endif

// 1 to use Multilingual Text Engine for text editing, 0 to use TextEdit or WASTE instead
#ifndef USE_MLTE
#define USE_MLTE 0
#endif

// if both WASTE and USE_MLTE are 0, then TextEdit is used
#if WASTE && USE_MLTE
#error Cannot use both WASTE and MLTE for text editing.
#endif

// See BP2 history regarding NEWTIMER
#ifndef NEWTIMER
#define NEWTIMER 0
#endif

// enable or disable built-in MIDI driver at compile time (not finished yet) - 010507 akozar
#ifndef USE_BUILT_IN_MIDI_DRIVER
#  if !TARGET_API_MAC_CARBON
#    define USE_BUILT_IN_MIDI_DRIVER 1
#  else
#    define USE_BUILT_IN_MIDI_DRIVER 0
#  endif
#endif

#if USE_BUILT_IN_MIDI_DRIVER
#  define WITH_REAL_TIME_MIDI 1
#  define WITH_REAL_TIME_SCHEDULER 1
#endif

//#include <ansi_prefix.mac.h>	// commented out - 010507 akozar

#if UseMIDIMgr
#  include <MIDI.h>
#endif

#ifdef __cplusplus
#  include <cstdio>
#  include <cstring>
#  include <cctype>
#  include <ctime>
#  include <cmath>
#  include <cstdlib>
#  include <cfloat>
#  include <climits>
#else
#  include <stdio.h>
#  include <math.h>
#  include <stdlib.h>
#  include <ctype.h>
#  include <string.h>
#  include <float.h>
#  include <time.h>
#  include <limits.h>
#endif

#if WASTE
#include "WASTEIntf.h"
#endif

#define _NOERRORCHECK_	/* Needed in <math.h> */
#define _NOSYNONYMS_

#include "midi1.h"

#if BP_CARBON_GUI
#  include "NavServWrapper.h"
#endif

#if !BP_CARBON_GUI
#  include "ConsoleMessages.h"
#endif

// Moved macros and enum down here to avoid potential problems with replacing names
// in any of the above headers -- 010807 akozar
// Note: the following definitions must not be given before loading ColorPicker.h because they mess up CMICCProfile.h
/*#if defined(__POWERPC) && !TARGET_API_MAC_CARBON
#define arrow Qd.arrow
#define screenBits Qd.screenBits
#define randSeed Qd.randSeed
#define dkGray Qd.dkGray
#define ltGray Qd.ltGray
#define gray Qd.gray
#define black Qd.black
#define white Qd.white
#define thePort Qd.thePort
#endif*/

#if !OLDROUTINENAMES
enum {
	inLabel					= 1,
	inMenu					= 2,
	inTriangle					= 4,
	inButton					= 10,
	inCheckBox					= 11,
	inUpButton					= 20,
	inDownButton				= 21,
	inPageUp					= 22,
	inPageDown					= 23,
	inThumb					= 129
};
#endif

// --------------------- General constants   --------------------------
#define MINUSPROC ((void*) -1L)
#define NULLSTR ((char*) 0)
#define NULLPROC ((IntProcPtr) -1L)
#define EMERGENCYMEMORYSIZE 200000L
#define TEXTEDIT_MAXCHARS 32000L
#define DRAG_EDGE 20

#define MAXDIAGRAM 1	/* number of diagrams displayed in graphic windows */
#define MAXPICT 2		/* number of pictures displayed in graphic windows */
#define MAXBUTT 32		/* number of buttons created in dialogs */
#define MAXMESSAGE 70	/* number of messages remembered */
#define MAXLIN 200		/* length of input line in any file */
#define HTMLTOKENLENGTH 80 /* estimated max length of html token */
#define MAXINFOLENGTH 50	/* length of date message */
#define MAXFIELDCONTENT 255	/* length of edit field in dialog window */
#define MAXNAME 31		/* length of file name */
#define MAXSTRINGCONSTANTS 256	/* string constants */
#define MAXNOTBPCASES 10	/* cases of non standard BP grammars */
#define MAXCOLOR 11		/* predefined colors */
#define MAXTICKS 3		/* ticks in time base */
#define MAXBEATS 40		/* beats in tick cycle */
#define TICKDUR 50L		/* default duration of ticks in the time-base (ms) */
#define TICKDELAY 500L	/* delay of ticks (ms) granting regularity */
#define MAXCHAN 16	/* number of MIDI-BP2 channels */
#define IPMAX 6	/* max number of additional parameters in Csound instrument */
#define MAXDATABASEFIELDS 60 /* number of fields in database */
#define MAXTIMESLICES 5000 /* max number of time slices by the OMS output time scheduling */
#define CLOCKRES 5L /* 5ms resolution of time scheduler */
#define CLOCKSIZE 256	/* must be a power of 2 */
#define CLOCKLOG 8		/* log base 2 of CLOCKSIZE */

// Maximum allowed memory for the phase diagram ?
// (or whatever is being checked in PolyMake())
#define kMaxPhaseDiagramSize (200*1024*1024)

// MIDI filter constants
#define FILTER_ALL_ON	(-1L)
#define FILTER_ALL_OFF	(0L)

// Constants for BP's special file and folder names

#define	kBPPrefsFolder	"\pBol Processor"
#define	kBPSeStartup	"\p-se.startup"
#define	kBPMdStartup	"\p-md.startup"
#define	kBPStartupScript	"\p+sc.startup"
#define	kBPShutdownScript	"\p+sc.shutdown"
#define	kBPTempFile		"\pBP2.temp"
#define	kBPTraceFile	"\pBP2.trace"

// Constants used in function returns

#define DONE 5
#define IMBEDDED 4
#define EMPTY 3
#define SINGLE 2
#define OK 1
#define YES 1
#ifndef TRUE
#define TRUE 1
#endif
#define FAILED 0
#define NO 0
#ifndef FALSE
#define FALSE 0
#endif
#define BACKTRACK -1
#define UNDO -2
#define ENDREPEAT -3
#define ABORT -4
#define CANCEL -4
#define QUICK -5
#define AGAIN -6
#define STOP -7
#define RESUME -8
#define FINISH -9
#define SLOW -10
#define EXIT -11

#define OFF 0
#define ON 1
#define LINE 2

#define RELATIVE 0
#define ABSOLUTE -1
#define IRRELEVANT -2
#define LINEAR 0

// Values for wait event types
#define KEYBOARDEVENT 0
#define APPLEVENT 1
#define STARTEVENT 2
#define STOPEVENT 3
#define CONTINUEEVENT 4

// Values fo file formats
#define MAC 0
#define DOS 1
#define UNIX 2

// More values for SynchroSignal
#define PLAYNOW 1
#define PLAYFOREVER 2

// Values for Nature_of_time
#define SMOOTH 0
#define STRIATED 1

// Values and indexes for pitch, modulation and pressure
// #define OFF 0 already defined
#define FIXED 1
#define CONTINUOUS 2
#define STEPWISE 3

#define IPITCHBEND 0
#define IVOLUME 1
#define IMODULATION 2
#define IPRESSURE 3
#define IPANORAMIC 4

// Values for MakeSound()
#define OBJECT 0
#define MODULATION 1
#define BENDER 2

// Values for Undo (LastAction)
#define TYPEWIND 1
#define TYPEDLG 2
#define CUTWIND 3
#define CUTDLG 4
#define PASTEWIND 5
#define PASTEDLG 6
#define COPY 7
#define DELETEWIND 8
#define DELETEDLG 9
#define SPACESELECTION 10

// Values for ShowSelect()
#define UP 0
#define CENTRE 1
#define DOWN 2

// Pitch parameter in Csound
#define OPPC 0
#define OPD 1
#define CPS 2

// File save preferences
#define ALLSAME 0
#define ALLSAMEPROMPT 1
#define NEWFILE 2

#define NOW 0
#define LATER 1

// Default values of MIDI parameters
#define DEFTVOLUME 90
#define DEFTVELOCITY 64
#define VOLUMECONTROL 7
#define SAMPLINGRATE 50
#define PANORAMICCONTROL 10
#define DEFTPANORAMIC 64
#define DEFTPITCHBEND 8191.5
#define DEFTPRESSURE 0
#define DEFTMODULATION 0

// Values for randomizing
#define NEWSEED -3
#define RANDOMIZE -2
#define NOSEED -1

// Csound parameter combination options
#define MULT 0
#define ADD 1

// Locations in phase diagram
#define INTIME 0
#define OUTTIME 1
#define ANYWHERE 2
#define BORDERLINE 3

// High level events
#define BP2Class 'Bel0'
#define PlayEventID 'play'
#define ScriptLineEventID 'scln'
#define LoadSettingsEventID 'sett'
#define NoteConventionEventID 'conv'
#define BeepID 'beep'
#define AbortID 'over'
#define PauseID 'paus'
#define AgainID 'more'
#define SkipID 'skip'
#define QuickID 'fast'
#define ResumeID 'cont'
#define ImprovizeID 'impr'
#define DoScriptID 'dosc'
#define NameID 'name'
#define GrammarID 'gram'
#define AlphabetID 'alph'
#define GlossaryID 'glos'
#define InteractionID 'inte'
#define DataID 'data'
#define ScriptID 'scri'
#define CsoundInstrID 'csin'

// Tokens
#define TEND (tokenbyte) -1
#define T0 (tokenbyte) 0
#define T1 (tokenbyte) 1
#define T2 (tokenbyte) 2
#define T3 (tokenbyte) 3
#define T4 (tokenbyte) 4
#define T5 (tokenbyte) 5
#define T6 (tokenbyte) 6
#define T7 (tokenbyte) 7
#define T8 (tokenbyte) 8
#define T9 (tokenbyte) 9
#define T10 (tokenbyte) 10
#define T11 (tokenbyte) 11
#define T12 (tokenbyte) 12
#define T13 (tokenbyte) 13
#define T14 (tokenbyte) 14
#define T15 (tokenbyte) 15
#define T16 (tokenbyte) 16
#define T17 (tokenbyte) 17
#define T18 (tokenbyte) 18
#define T19 (tokenbyte) 19
#define T20 (tokenbyte) 20
#define T21 (tokenbyte) 21
#define T22 (tokenbyte) 22
#define T23 (tokenbyte) 23
#define T24 (tokenbyte) 24
#define T25 (tokenbyte) 25
#define T26 (tokenbyte) 26
#define T27 (tokenbyte) 27
#define T28 (tokenbyte) 28
#define T29 (tokenbyte) 29
#define T30 (tokenbyte) 30
#define T31 (tokenbyte) 31
#define T32 (tokenbyte) 32
#define T33 (tokenbyte) 33
#define T34 (tokenbyte) 34
#define T35 (tokenbyte) 35
#define T36 (tokenbyte) 36
#define T37 (tokenbyte) 37
#define T38 (tokenbyte) 38
#define T39 (tokenbyte) 39
#define T40 (tokenbyte) 40
#define T41 (tokenbyte) 41
#define T42 (tokenbyte) 42
#define T43 (tokenbyte) 43

// Inference modes
#define ANAL 0	/* Modus tollens */
#define PROD 1	/* Modus ponens */
#define TEMP 2	/* Producing templates */

// Note conventions
#define MAXCONVENTIONS 4
#define ENGLISH 0
#define FRENCH 1
#define INDIAN 2
#define KEYS 3

#define NOTESIZE 9	/* Length of note/key name. Used by EnglishNotes(), FrenchNotes()É */
					/* É and p_NoteName and p_AltNoteName */

#define BOL 0
#define PATTERN 1

#define AZERTY 0
#define QWERTY 1

// Numbers
#define EPSILON 3L
#define ZERO 0L

// Predefined color indexes
#define VariableC 0
#define TerminalC 1
#define HomomorphismC 2
#define TimePatternC 3
#define FlagC 4
#define ControlC 5
#define TagC 6
#define SoundObjectC 7
#define StreakC 8
#define PivotC 9
#define NoteC 10

// Interface constants

#define SBARWIDTH 15	/* Width of menu bar, controller, etc. */
#define MINWINDOWHEIGHT 80
#define MINWINDOWWIDTH 80

#define MAXMENU 9		/* number of menus in menu bar */
#define MAXMENUITEMS 26 /* number of items in each menu */
#define MAXWIND 14	/* number of windows */
#define MAXDIAL 25	/* number of dialogs */
#define WMAX 39		/* number of windows = MAXDIAL + MAXWIND */

// Window ID's
#define WindowIDoffset 511
// Window[w] ID is (WindowIDoffset + w)

// Menus ID's
#define MenuIDoffset 128
// Other ID's are consecutive numbers, e.g. fileID = (fileM + MenuIDoffset)
// #define prefsID		136  /* submenu Preferences, not used */

// Cursor ID's
#define WheelID 1003 /* 4 consecutive cursors */
#define FeetID 1020 /* 8 consecutive cursors */
#define DiskID 1200 /* 2 consecutive cursors */
#define KeyboardID 129
#define QuestionID 128
#define XcrossID 131

// Strings ID's
#define DialogStringsBaseID 300
#define MiscStringsID 302
#define ScriptStringsID 303
#define GramProcedureStringsID 304
#define PerformanceControlStringsID 305
#define GeneralMIDIpatchesID 306
#define HTMLdiacriticalID 307

// Sound ID's
#define kpopID	128
#define kclocID 129

// Small icons
#define BP2smallIconID 128

// Dialog ID's
#define GreetingsID 128
#define FAQDialogID 129
#define SixteenDialogID 130
#define ObjectChartID 550
#define ResumeStopID 551
#define ReplaceCommandID 552
#define MIDIsettingsID 553
#define ResumeUndoStopID 554
#define PatternID 555
#define EnterID 556
#define MIDIkeyboardID 557
#define FileSavePreferencesID 558
#define StrikeModeID 559
#define TuningID 560
#define CsoundInstrMoreID 561
#define OMSinoutID 562
#define MIDIprogramID 563
#define DefaultID 564
#define EnterNameID 565
#define GrammarFromTableID 566
#define SelectFieldID 567

// Alert ID's
#define	WorkAlert		128
#define	StepAlert		142
#define 	OKAlert		143
#define	YesNo			144
#define	NoYes			145
#define	YesNoCancel		146
#define	NoYesCancel		147
#define AboutAlert		129
#define SaveAsAlert 		149
#define LoadNotesAlert		150
#define ColorAlert		151
#define DisplayItemAlert	152
#define PasteSelectionAlert	153

// Icon ID's
#define BP2iconID 128
#define EditObjectsIconID 257

// PICT ID's
#define MIDIKeyboardPictID 128
#define GreetingsPictID 129

// Window indexes
#define wUnknown -1
#define wGrammar 0
#define wAlphabet 1
#define wStartString 2
#define wMessage 3
#define wGraphic 4
#define wTrace 5
#define wInfo 6
#define wData 7
#define wScript 8
#define wScrap 9
#define wHelp 10
#define wNotice 11
#define wGlossary 12
#define wInteraction 13

// Dialog indexes
#define wRandomSequence 14
#define wTimeAccuracy 15
#define wTimeBase 16
#define wBufferSize 17
#define wFindReplace 18
#define wGraphicSettings 19
#define wControlPannel 20
#define wKeyboard 21
#define wScriptDialog 22
#define wMetronom 23
#define wSettingsTop 24
#define wSettingsBottom 25
#define wPrototype1 26
#define wPrototype2 27
#define wPrototype3 28
#define wPrototype4 29
#define wPrototype5 30
#define wPrototype6 31
#define wPrototype7 32
#define wFilter 33
#define wTickDialog 34
#define wCsoundInstruments 35
#define wPrototype8 36
#define wCsoundTables 37
#define wMIDIorchestra 38

// DITL indexes
#define GreetingsDitlID 128

// Additional indexes used in file types (see gFileType[] in BP2main.h)
#define iObjects 26
#define iWeights 14
#define iDecisions 15	// added but not really in use, yet - akozar 031407
#define iMidiDriver 17
#define iMIDIfile 24
#define iSettings 25

// BP File type indexes (these are the values stored in gFileType)
typedef enum {
	ftiAny = 0,
	ftiText = 1,
	ftiKeyboard = 2,
	ftiObjects = 3,
	ftiDecisions = 4,
	ftiGrammar = 5,
	ftiAlphabet = 6,
	ftiData = 7,
	ftiInteraction = 8,
	ftiSettings = 9,
	ftiAIFC = 10,
	ftiMidi = 11,
	ftiWeights = 12,
	ftiScript = 13,
	ftiGlossary = 14,
	ftiTimeBase = 15,
	ftiCsoundInstruments = 16,
	ftiMIDIorchestra = 17,
	ftiHTML = 18,
	ftiMidiDriver = 19
} FileTypeIndex;

#define MAXFILETYPEINDEX	20

// Menu indexes
#define appleM			0
#define fileM			1
#define editM			2
#define searchM			3
#define actionM			4
#define layoutM			5
#define miscM			6
#define deviceM			7
#define windowM			8
#define scriptM			9

// Apple menu indexes
#define aboutCommand	1

// File menu command indexes
#define fmNewProject  1
#define fmLoadProject 2
// -------------------- 3
#define fmOpen		4
#define fmClearWindow	5
#define fmGoAway		6
#define fmSave		7
#define fmSaveAs		8
#define fmRevert		9
#define fmGetInfo		10
// -------------------- 11
#define fmLoadSettings	12
#define fmSaveSettings	13
#define fmSaveSettingsAs 14
#define fmSaveStartup	15
// -------------------- 16
#define fmMakeGrammarFromTable 17
#define fmReceiveMIDI	18
#define fmSendMIDI	19
#define fmLoadPattern	20
// -------------------- 21
#define fmPageSetUp	22
#define fmPrint		23
// -------------------- 24
#define fmQuit		25

// Edit menu command indexes
#define undoCommand 	1
#define cutCommand		3
#define copyCommand		4
#define pasteCommand	5
#define clearCommand	6
#define selectallCommand 8
// --------------------------	9
#define textCommand		10
#define tokenCommand		11
// --------------------------	12
#define typenoteCommand		13
// --------------------------	14
#define pickperfctrlCommand	15
#define pickgramprocCommand	16

// Search menu command indexes
#define findCommand			1
#define enterfindCommand	2
#define findagainCommand	3
#define listvariablesCommand 5
#define listterminalsCommand 6
#define listreservedCommand 7
#define balanceCommand 8

// Action menu command indexes
#define compileCommand		1
#define produceCommand		2
#define checkdeterminismCommand		3
#define templatesCommand	4
#define analyzeCommand		5
#define playCommand			6
#define expandCommand		7
#define captureCommand 8
#define showPeriodsCommand 9
#define executescriptCommand 10
#define transliterateCommand 11
// --------------------------		12
#define pauseCommand			13
#define resumeCommand			14
#define stopCommand			15
// --------------------------		16
#define resetsessiontimeCommand	17
#define tellsessiontime			18
// --------------------------		19
#define helpCommand			20

// Misc menu command indexes
#define metronomCommand				1
#define computationSettingsCommand		2
#define inoutSettingsCommand			3
#define randomizeCommand			4
#define accuracyCommand				5
#define buffersizeCommand 			6
#define graphicsettingsCommand		7
#define defaultPerformanceValuesCommand	8
#define defaultStrikeModeCommand		9
#define defaultFileSavePreferencesCommand	10
#define tuningCommand				11
// --------------------------			12
#define frenchconventionCommand		13
#define englishconventionCommand		14
#define indianconventionCommand		15
#define keyconventionCommand			16
// --------------------------			17
#define pianorollCommand			18

// Layout menu command indexes
#define f9Command	1
#define f10Command	2
#define f12Command	3
#define f14Command	4
#define changeColorCommand 6
#define graphicsColorCommand 7
#define textColorCommand 8
#define azertyCommand 10
#define qwertyCommand 11
#define bulletCommand 13
// --------------------------	 14
#define splitTimeObjectCommand 15
#define splitVariableCommand	 16

// Devices menu command indexes
#define outMIDICommand 1
#define outMIDIfileCommand 2
#define outCsoundCommand 3
#define outOMSCommand 4
// --------------------------		5
#define checkMIDIinputcommand 6
#define checkMIDIoutputcommand 7
#define midifilterCommand		8
// --------------------------		9
#define MIDIorchestracommand		10
#define CsoundInstrumentSpecsCommand 11
// --------------------------		12
#define OMSinoutCommand			13
#define OMSmidiCommand			14
#define OMSstudioCommand		15
// --------------------------		16
#define modemportCommand		17
#define printerportCommand		18

#if BP_MACHO
// alternate Device menu indices for CoreMIDI
#define CMsettingsCommand		13
#define OpenAudioMidiSetupCommand	14
#endif

// Window menu command indexes
#define grammarCommand		1
#define alphabetCommand		2
#define dataCommand		3
#define miscsettingsCommand	4
#define prototypeCommand	5
#define timebaseCommand		6
#define interactionCommand	7
#define scriptCommand		8
#define glossaryCommand		9
#define keyboardCommand		10
// -------------------------- 11
#define controlpannelCommand	12
// -------------------------- 13
#define graphicCommand		14
#define traceCommand		15
#define startstringCommand	16
#define scrapCommand		17
#define noticeCommand		18
// -------------------------- 19
#define FAQCommand		20
// -------------------------- 21
#define showmessagesCommand	22

// AboutAlert indexes
#define dAboutOK		1
#define dAboutAbort		6
#define dCredits 10
#define bRegister 11

// WorkAlert indexes
#define dLoadProject 1
#define dData 3
#define dGrammars 4
#define dScripts 5
#define dSoundObjects 6
#define dInteraction 8
#define dGlossary 9
#define dTimeBase 10
#define dFAQ 11
#define dQuitWork 12
#define dCsoundInstruments 13
#define dAlphabets 14
#define dRegister 24

// YesNoCancel and NoYesCancel alert indexes
#define dDefault	1
#define dAltern		2
#define dAbort		3

// Color alert indexes
#define dOK 1
#define dVariable 2
#define fVariable 3
#define dTerminal 4
#define fTerminal 5
#define dHomomorphism 6
#define fHomomorphism 7
#define dTimePattern 8
#define fTimePattern 9
#define dFlag 10
#define fFlag 11
#define dControl 12
#define fControl 13
#define dTag 14
#define fTag 15
#define dSoundObject 16
#define dStreak 17
#define dPivot 18
#define dNote 22
#define fNote 23

// EnterName dialog indexes
#define dNameOK 1
#define fUserName 3
#define fInstitution 4

// Control pannel dialog button indexes
#define dProduceItems 1
#define dDeriveFurther 2
#define dLoadDecisions 3
#define dSaveDecisions 4
#define dRepeatComputation 5
#define dShowGramWeights 6
#define dMIDIcheck 7
#define dPlaySelection 8
#define dExpandSelection 9
#define dTemplates 10
#define dAnalyze 11
#define dLoadWeights 12
#define dLearnWeights 13
#define dSaveWeights 14
#define dSetWeights 15
#define bMIDIpanic 16
#define dShowPeriods 17
#define dTransposeInput 18
#define fTransposeInput 19
#define bCaptureSelection 21
#define bResetControllers 22
#define bMute 23

// Tuning dialog indexes
#define bOKtuning 1
#define fC4key 2
#define fA4freq 3

// Default dialog indexes
#define fDeftVolume 3
#define fDeftVelocity 4
#define fDeftPanoramic 7
#define fPanoramicController 8
#define fVolumeController 10
#define fSamplingRate 12

// Tempo dialog indexes
#define fTempo 1
#define dSmooth 2
#define dStriated 3

// Randomize dialog indexes
#define dReset 1
#define dNewSeed 2
#define fSeed 3

// TimeAccuracy dialog indexes
#define fQuantize 1
#define fTimeRes 7
#define fSetUpTime 10
#define dOff 2
#define dOn 3

// ResumeStop & ResumeUndoStop dialog indexes
#define dResume 1
#define dStop 2
#define dUndo 3

// SixteenDialog indexes
#define bOKSixteen 1
#define button1 2
#define bChangePatch 19

// Pattern dialog indexes
#define dPatternOK 1
#define dPatternCancel 7
#define fPatternName 3
#define fPatternDuration 5

// Enter dialog indexes
#define dEnterOK 1
#define dEnterCancel 2
#define fValue 3
#define fMessage 4

// ReplaceCommand dialog indexes
#define dDontChange 1
#define dReplace 2
#define dStopReplace 3

// Keyboard dialog indexes
#define dOK 1
#define dLoad 2
#define dSave 3
#define dToken 108
#define dResetkeys 109
#define fA 30
#define fa 82

// Tick dialog indexes
#define dTickOn 3
#define dTickOff 4
#define dDefaultTick 5
#define dSpecialTick 6
#define fThisTickChannel 7
#define fThisTickKey 8
#define fThisTickVelocity 9
#define dRecordThisTick 13

// TimeBase dialog indexes
#define fQ 1
#define fP 2
#define dPlayTicks 5
#define dResetCycle 171
#define dNewTimeBase 175
#define dLoadTimeBase 176
#define dSaveTimeBase 177
#define dSaveAsTimeBase 193
#define fTimeBaseComment 203
#define dCaptureTicks 204
// The following 3 times with period = 3
	#define fTickDuration 195
// The following 3 times with period = 1
	#define dClearCycle 172
// The following 3 times with period = 5
	#define dMute 178
    #define fPratio 179
    #define fQratio 180
// The following repeated 3 times with period = 55
	#define fTickKey 7
	#define fTickChannel 9
	#define dRecordTick 10
	#define fTickVelocity 12
	// The following 40 times with period = 1
		#define dPlayBeat 13
	#define fTickCycle 59

// BufferSize dialog indexes
#define fBufferSize		1
#define dNoSizeLimit	2
#define dYesSizeLimit	3
#define fDeftBufferSize 5

// FindReplace dialog indexes
#define dFind		1
#define fFind		2
#define fReplace	3
#define dCancel		6
#define dReplaceAll	7
#define dIgnoreCase	8
#define dMatchWords	9

// MIDIsettings dialog indexes
#define dOKMIDIsettings 1
#define dResetDefault 2
#define dCancelMIDIsettings 3
#define fSizeMIDIbuffer 6
#define fStorageTime 9
#define dPrinter 11
#define dModem 12
#define fFrequency 13

// GraphicSettings dialog indexes
#define fGraphicScale 1
#define dZero 4
#define dOne 5

// MIDI program dialog indexes
#define bMIDItest 131
#define fMIDIchannel 130
#define fPatchName 133
#define bMIDIprogramList 135
#define bShowMIDIorchestra 136
#define bMIDIprogramHelp 137
#define bMIDIprogramAllNotesOff 138

// MIDI orchestra dialog indexes
#define bMIDIorchestraSave 17
#define bMIDIorchestraLoad 18
#define bMIDIorchestraNew 19

// Prototype1 dialog indexes
#define bOKprototype1 1
#define fProtoName 2
#define bMIDIsequence 3
#define bSampledSound 4
#define bCsoundInstrument 5
#define bEditRecord 6
#define bResetPrototype 7
#define bCopyFrom1 8
#define bPlayObjectAlone 9
#define bPlayExpression 10
#define fExpression 11
#define fMetronomTry 13
#define bStriatedTimeTry 14
#define bShowGraphicTry 16
#define bIgnorePropertiesTry 17
#define bWithParameters 18
#define fDilationRatio 20
#define pPicture 21
#define bContinuityPreRoll 22
#define bDurationPivot 23
#define bCoverTruncate 24
#define bLeftArrowPrototype 25
#define bRightArrowPrototype 26
#define bGoToPrototype 27
#define bLoadPrototypeFile 29
#define bSavePrototypeFile 30
#define fPrototypeFileComment 31
#define fPrototypeComment 32
#define bPeriodCyclicObject 33
#define bCreateObject 35
#define bPasteSelection 36
#define bUndoPasteSelection 37
#define bCapture 38
#define fDuration 39
#define bCsoundWindow 41
#define bConvertToCsound 42
#define bChannelInstrument 43
#define fInstrumentFileName 44
#define bChangeInstrumentFile 46
#define bImportMIDIfile 47
#define bShowPianoRoll 48

// Picture for sound-object prototype
#define topDrawPrototype 120
#define leftDrawPrototype 344
#define bottomDrawPrototype 380
#define rightDrawPrototype 631

// Prototype2 dialog indexes
#define bOKprototype2 1
#define bOKrescale 3
#define bNeverRescale 4
#define bDilationRatioRange 5
#define fMinDilationRatio 6
#define fMaxDilationRatio 8 
#define bSendAlpha 10
#define fSendAlphaControl 11
#define fSendAlphaChannel 13
#define bPivBeg 15
#define bPivMiddle 16
#define bPivEnd 17
#define bPivFirstNoteOn 18
#define bPivMiddleNoteOnOff 19
#define bPivLastNoteOff 20
#define bSetPivotms 21
#define fSetPivotms 22
#define bSetPivotPC 26
#define fSetPivotPC 25
#define bNeverRelocate 28
#define bAllowDelayms 29
#define fAllowDelayms 30
#define bAllowDelayPC 32
#define fAllowDelayPC 33
#define bAllowFwdms 35
#define fAllowFwdms 36
#define bAllowFwdPC 38
#define fAllowFwdPC 39
#define bRelocateAtWill 42
#define bExpandAtWill 43
#define bCompressAtWill 44
#define bCopyFrom2 45

// Prototype3 dialog indexes
#define bOKprototype3 1
#define bNeverCoverBeg 2
#define bCoverBegAtWill 3
#define bCoverBegLessThanms 4
#define fCoverBegLessThanms 5 
#define bCoverBegLessThanPC 6
#define fCoverBegLessThanPC 7 
#define bNeverCoverEnd 8
#define bCoverEndAtWill 9
#define bCoverEndLessThanms 10
#define fCoverEndLessThanms 11 
#define bCoverEndLessThanPC 12
#define fCoverEndLessThanPC 13 
#define bNeverTruncBeg 14
#define bTruncBegAtWill 15
#define bTruncBegLessThanms 16
#define fTruncBegLessThanms 17 
#define bTruncBegLessThanPC 18
#define fTruncBegLessThanPC 19 
#define bNeverTruncEnd 20
#define bTruncEndAtWill 21
#define bTruncEndLessThanms 22
#define fTruncEndLessThanms 23 
#define bTruncEndLessThanPC 24
#define fTruncEndLessThanPC 25 
#define bNeverBreakTempo 26
#define bBreakTempoAtWill 27
// #define bBreakTempoLessThanms 28
// #define fBreakTempoLessThanms 29
// #define bBreakTempoLessThanPC 30
// #define fBreakTempoLessThanPC 31
#define bCopyFrom3 41

// Prototype4 dialog indexes
#define bOKprototype4 1
#define bDontForceContBeg 2
#define bForceContBeg 3
#define bAllowGapBegms 4
#define fAllowGapBegms 5 
#define bAllowGapBegPC 6
#define fAllowGapBegPC 7 
#define bDontForceContEnd 8
#define bForceContEnd 9
#define bAllowGapEndms 10
#define fAllowGapEndms 11 
#define bAllowGapEndPC 12
#define fAllowGapEndPC 13 

#define bCopyFrom4 20
#define bPreRollms 21
#define bPreRollPC 23
#define bPostRollms 27
#define bPostRollPC 29
#define fPreRollms 22
#define fPreRollPC 24
#define fPostRollms 28
#define fPostRollPC 30

// Prototype 5 dialog indexes
#define bOKprototype5 1
#define bPlayPrototype 2
#define bRecordPrototype 3
#define bSuppressKeyPressure 4
#define bStriatedObject 5
#define bSuppressTrailingSilences 46
#define fTref 6
#define bAdjustDurationms 8
#define bAdjustDBeats 36
#define fDurationAdjustms 9
#define fDurationAdjustbeats 10
#define bAdjustVelocities 13
#define fMinVelocity 17
#define fMaxVelocity 16
#define bQuantizeNoteOn 18
#define fQuantizeFractionBeat 20
#define bExpandDurations 22
#define fExpandDurationsFractionBeat 24
#define bMakeMonodic 26
#define bAppendAllNotesOff 27
#define bSuppressAllNotesOff 28
#define bInsertSilence 29
#define bAppendSilence 30
#define fInsertSilence 31
#define fAppendSilence 32
#define bCopyFrom5 35
#define bImportPrototype 37
#define bPlayPrototypeTicks 38
#define bRecordPrototypeTick 39
#define fPrototypeTickKey 40
#define fPrototypeTickChannel 42
#define fPrototypeTickVelocity 47
#define bSuppressChannelPressure 43
#define bSuppressPitchBend 44
#define bShowCodes 45
#define fCurrentBeats 49
#define bClearStreamInPrototype 51

// Prototype 6 dialog indexes
#define bOKprototype6 1
#define bBeforePeriodms 4
#define bBeforePeriodPC 7
#define bIrrelevantPeriod 3
#define fBeforePeriodms 5
#define fBeforePeriodPC 8
#define bForceIntegerPeriod 11
#define bStrikeAgain 13
#define bDontStrikeAgain 15
#define bStrikeDefault 16
#define bDiscardNoteOffs 17
#define bShowDefaultStrikeMode 18

// Prototype 7 dialog indexes
#define bOKprototype7 1
#define bImportScore 2
#define bExportScore 3

// Prototype 8 dialog indexes
#define bOKprototype8 1
#define bDontChangeChannel 2
#define bForceToChannel 3
#define fForceToChannel 4
#define bForceCurrChannel 6
#define bAcceptArticulation 8
#define bAcceptTransposition 9
#define bAcceptVolume 10
#define bAcceptPanoramic 11
#define bCopyFrom8 12
#define bDontChangeInstrument 13
#define bForceToInstrument 14
#define fForceToInstrument 15
#define bForceCurrInstrument 16
#define fAssignInstrument 20
#define bAcceptMap 22
#define bAcceptVelocity 23

// CsoundInstruments dialog indexes
#define bOKCsoundInstruments 1
#define fCsoundInstrumentIndex 3
#define fCsoundInstrumentName 5
#define fCsoundInstrumentComment 6
#define bLoadCsoundInstruments 7
#define bSaveAsCsoundInstruments 8
#define bNewCsoundInstrument 9
#define bPrecedingCsoundInstrument 10
#define bNextCsoundInstrument 11
#define bGotoCsoundInstrument 12
#define fCsoundInstrumentChannel 14
#define fPitchIndex 16
#define bOctavePitchClass 17
#define bOctaveDecimal 18
#define bHz 19
#define fPitchBendIndex 21
#define fVolumeIndex 31
#define fPressureIndex 41
#define fModulationIndex 51
#define fPanoramicIndex 65
#define fAttackVelocityIndex 61
#define fReleaseVelocityIndex 63
#define fDilationRatioIndex 76
#define bSaveCsoundInstruments 74
#define bResetCsoundInstrument 77
#define bCsoundInstrumentHelp 78
#define fPitchBendEndIndex 79
#define fVolumeEndIndex 80
#define fPressureEndIndex 81
#define fModulationEndIndex 82
#define fPanoramicEndIndex 83
#define fPitchBendRange 85
#define fNumberParameters 87
#define bCopyCsoundInstrumentFrom 88
#define fCsoundOrchestra 89
#define bCheckCsoundOrchestra 90
#define bq1 91
#define bq2 92
#define bq3 93
#define bq4 94
#define bq5 95
#define bMore 96
#define bNewCsoundInstrumentFile 97
#define bCsoundTables 98
#define fPitchBendTable 99
#define fVolumeTable 100
#define fPressureTable 101
#define fModulationTable 102
#define fPanoramicTable 103
#define bDisplayAsText 114
#define bExportAllInstruments 115

// More Csound parameters dialog indexes
#define fMoreName 2
#define fMoreStartIndex 3
#define fMoreEndIndex 4
#define fMoreComment 5
/* 5 more follow by increments of 7 */

#define fMoreTable 44
/* 5 more follow by increments of 1 */

#define bMULTval 80
#define bADDval 81
/* 5 more follow by increments of 3 */

#define fDefaultCsoundParameterValue 57
#define fCsoundParameterGenType 59
/* 5 more follow by increments of 4 */

#define fPitchBendGEN 105
#define fVolumeGEN 107
#define fPressureGEN 109
#define fModulationGEN 111
#define fPanoramicGEN 113

// CsoundTables dialog indexes
#define bOKCsoundTables 1

// MIDI filter dialog indexes
#define bNoteOnOffIn 1
#define bKeyPressureIn 2
#define bControlIn 3
#define bProgramChangeIn 4
#define bChannelPressureIn 5
#define bPitchBendIn 6
#define bSysExIn 7
#define bTimeCodeIn 8
#define bSongPositionIn 9
#define bSongSelectIn 10
#define bTuneRequestIn 11
#define bTimingClockIn 12
#define bStartStopIn 13
#define bActiveSensingIn 14
#define bSystemResetIn 15

#define bNoteOnOffPass 16
#define bKeyPressurePass 17
#define bControlPass 18
#define bProgramChangePass 19
#define bChannelPressurePass 20
#define bPitchBendPass 21
#define bSysExPass 22
#define bTimeCodePass 23
#define bSongPositionPass 24
#define bSongSelectPass 25
#define bTuneRequestPass 26
#define bTimingClockPass 27
#define bStartStopPass 28
#define bActiveSensingPass 29
#define bSystemResetPass 30

#define bFilterReset 48
#define bFilterReceiveSetAll	49
#define bFilterReceiveClear	50
#define bFilterTransmitSetAll	51
#define bFilterTransmitClear	52

// MakeGrammarFromTable dialog indexes
#define bMakeGrammarFromTableProceed 1
#define bMakeGrammarFromTableCancel 2
#define bMakeGrammarFromTableLoad 3
#define bMakeGrammarFromTableSave 4
#define fMakeGrammarFromTableSettingsName 5
#define fMakeGrammarFromTableSettingsComment 8
#define bMakeGrammarFromTableSetDefault 9
#define fMakeGrammarFromTableLeftArgCol 13
#define fMakeGrammarFromTableRightArgCol 14
#define fMakeGrammarFromTableFirstRow 16
#define fMakeGrammarFromTableLastRow 18
#define bMakeGrammarFromTableORD 20
#define bMakeGrammarFromTableRND 21
#define bMakeGrammarFromTableLIN 22
#define bMakeGrammarFromTableSUB 23
#define bMakeGrammarFromTableSUB1 24
#define bMakeGrammarFromTableLeftIsASCII 27
#define bMakeGrammarFromTableLeftIsText 28
#define bMakeGrammarFromTableLeftIsNum 29
#define bMakeGrammarFromTableRightIsASCII 31
#define bMakeGrammarFromTableRightIsText 32
#define bMakeGrammarFromTableRightIsNum 33
#define bMakeGrammarFromTableAppend 34
#define bMakeGrammarFromTableQuotes 35
#define bMakeGrammarFromTableLR 38
#define bMakeGrammarFromTableLRRL 39
#define bMakeGrammarFromTableRL 40
#define fMakeGrammarFromTableLeftPrefix 42
#define fMakeGrammarFromTableRightPrefix 44
#define bMakeGrammarFromTableLeftRemoveSpaces 45
#define bMakeGrammarFromTableRightRemoveSpaces 46
#define bMakeGrammarFromTableShowNext 47
#define fMakeGrammarFromTableCommentCol 49
#define bMakeGrammarFromTablePOSLONG 50

// SelectField dialog indexes
#define bSelectFieldProceed 1
#define bSelectFieldCancel 2
#define bSelectFieldShowNextRecord 3
#define bSelectFieldHelp 59
#define fSelectFieldNumber 6
   // 6 to 54, increment 4
#define fSubGramStart 7
   // 7 to 55, increment 4
#define fSubGramEnd 9
   // 9 to 57, increment 4
#define fSaveToFieldNumber 61
   // 61 to 73, increment 1

// Step alert button indexes
#define aContinue 1
#define aStep 2
#define aUndo 3
#define aAbort 4

// SaveAs alert button indexes
#define dBP2format 1
#define dPlainText 2
#define dhtml 3
#define dhtmlText 5

// LoadNotes alert button indexes
#define dMIDIdevice		2
#define dFile			3

// DisplayItemAlert button indexes
#define dProduceMore 1
#define dPlayAgain 2
#define dDisplayItem 3
#define dCancelPlay 4

// PasteSelectionAlert button indexes
// These values are now also used as parameters to PasteStreamToPrototype()
// bAskPasteAction is not a button index but indicates to ask user for action.
#define bAskPasteAction -1
#define bCancelPasteSelection 1
#define bInsertBefore 2
#define bAppend 3
#define bInsertAtInsertPoint 4
#define bReplaceFromInsertpoint 5
#define bDeleteReplace 6
#define bHelpPasteSelection 7
#define bMergeFromInsertPoint 9

// Settings dialog button indexes
#define bImprovize 0
#define bCyclicPlay 1
#define bUseEachSub 2
#define bAllItems 3
#define bDisplayProduce 4
#define bStepProduce 5
#define bStepGrammars 6
#define bTraceProduce 7
#define bPlanProduce 8
#define bDisplayItems 9
#define bShowGraphic 10
#define bAllowRandomize 11
#define bDisplayTimeSet 12
#define bStepTimeSet 13
#define bTraceTimeSet 14
#define bCsoundTrace 15

#define bMIDI 16
#define bSynchronizeStart 17
#define bComputeWhilePlay 18
#define bInteractive 19
#define bResetWeights 20
#define bResetFlags 21
#define bResetControllers 22
#define bNoConstraint 23
#define bWriteMIDIfile 24
#define bShowMessages 25
#define bCsound 26
#define bOMS 27

// Script dialog item buttons
#define bExecScript 1
#define bSaveScript 2
#define bLoadScript 3
#define bClearScript 4
#define bRecordScript 5
#define bWaitForSyncScript 6
#define bCheckScriptSyntax 7

// MIDIkeyboard dialog items
#define dOKkeyboard 26
// Other buttons are numbered 1 to 24, from 'do' down to 'si' twice.

// FileSavePreferences dialog items
#define bOKFileSavePreferences 1
#define bSaveAllToSame 2
#define fCsoundFileName 3
#define bChangeCsFile 4
#define bSaveAllToSameWithPrompt 5
#define bEachToNew 6
#define bWriteNow 7
#define bWriteLater 8
#define fMIDIFileName 9
#define bChangeMIDIfile 12
#define bType0 14
#define bType1 15
#define bType2 16
#define bTypeMac 18
#define bTypeDos 19
#define bTypeUnix 20
#define fFadeOut 22

// StrikeMode dialog items
#define bOKSaveStrikeMode 1
#define bDefaultStrikeAgain 2
#define bDefaultDontStrikeAgain 4



// -----------  BP2 constants  -------------------------------

#define MAXMIDI 1000	/* length of line encoding MIDI event */
#define BOLSIZE 30		/* length of terminal name */
#define MAXFLAG 8		/* default number of flags */
#define MAXEVENTSCRIPT 8	/* default number of event script lines */
#define FIELDSIZE 100	/* standard size of field in poly structure */
#define MAXVAR 30		/* initial number of variables in grammar or script */
#define HOMOSIZE 20		/* length of homomorphism identifier */
#define MAXDISPL 1000	/* minimum size of display */
#define MAXLEVEL 128	/* parentheses level */
#define MAXDERIV 200 	/* number of derivation steps */
#define MAXTAB 20		/* number of tab sections for display */
#define MAXMETA 32		/* number of metavariables in a rule */
#define MAXMETA2 65		/* 2 * MAXMETA + 1 */
#define MAXPARAMCTRL 128	/* number of MIDI controllers + 1 */
#define MAXWAIT 64		/* number of synchronization tags */
#define MAXTIME 59944L	/* max computation time (milliseconds) */

#define MAXMODE 3		/* number of derivation modes: LEFT, etc. */
#define MODELENGTH 6
#define MAXNIL 4		/* number of nil string representations */
#define NILLENGTH 7
#define MODELENGTH 6

#define MAXTYPE 7		/* number of subgram types: RND, etc. */
#define TYPELENGTH 8 	/* length of subgram type */
#define RNDtype 0
#define ORDtype 1
#define LINtype 2
#define SUBtype 3
#define SUB1type 4
#define TEMtype 5
#define POSLONGtype 6

#define MAXHTMLDIACR 62	/* number of discritical HTML tags */
#define INSTRLENGTH 17
#define DESTROYproc 0
#define MAXARROW 3		/* number of rule operators: '-->', etc. */
#define ARROWLENGTH 4
#define MAXCODE 21		/* number of special characters: ()+;=:/ etc. accessible to users */
#define MAXCODE2 24		/* number of special characters: ()+;=:/ etc. used by BP2 */
#define VERSIONLENGTH 15
#define MAXKEY 128		/* number of keys in MIDI standard */

// ----------------  MIDI Status bytes -----------------

#define NoteOff 128
#define NoteOn 144
#define KeyPressure 160
#define ControlChange 176 /* Controllers 0 to 121 */
#define ChannelMode 176
#define ProgramChange 192
#define ChannelPressure 208
#define PitchBend 224
#define SystemExclusive 240
#define SongPosition 242
#define SongSelect 243
#define TuneRequest 246
#define EndSysEx 247
#define TimingClock 248
#define Start 250
#define Continue 251
#define Stop 252
#define ActiveSensing 254
#define SystemReset 255

// --------------  Types -----------------------------

// Decide between pointers and handles.
// I always notate "p_X" the handle of table X
// #if MACOS
//	#define h_(x) *p_(x)
// #else
//	#define h_(x)  (x)
//	#define ((t)**)GiveSpace((x))  ((t)*)NewPtr((x))
//	#define MyDisposeHandle((handle*)&p_(x)) DisposPointer((x))
// #endif

#if USE_MLTE
typedef struct {
	TXNObject	textobj;
	TXNFrameID	id;
	Rect		viewRect;
} OurMLTERecord;
#endif

#if WASTE
typedef WEHandle TextHandle;
typedef long TextOffset;
#elif USE_MLTE
typedef OurMLTERecord** TextHandle;
typedef long TextOffset;	// should be TXNOffset (unsigned long), but there are many incompatible assumptions of long - akozar
#else
typedef TEHandle TextHandle;
typedef long TextOffset;	// should be short, but there are many assumptions of long - akozar
#endif

#if WITH_REAL_TIME_SCHEDULER
// Types for time scheduler
typedef OMSAPI(void) (*voidOMSdoPacket)(OMSMIDIPacket*,short,short);
typedef struct Slice {
	voidOMSdoPacket routine;	// routine to be called at time n
	OMSMIDIPacket pkt;			// parameter for routine
	short dat1,dat2;			// parameters for routine
	long loopstep;				// number of clock cycles from the preceding event on
								// the same tick.
	struct Slice *next;			// next timeslice
	} Slice;

// end of types for time scheduler
#endif

struct s_chunck {
	unsigned long origin,end;
	};
typedef struct s_chunck ChunkPointer;

typedef int (*IntProcPtr)(int);
typedef int (*Int2ProcPtr)(short,int);

typedef uint32_t dword;
typedef unsigned char byte;

typedef int tokenbyte;
#define TOKBASE ((unsigned long)(((unsigned long)INT_MAX) + 1L))

typedef long Milliseconds;

typedef struct {
	int datamode;
	double firstscale;
	int istemplate;
	int wind;
	double prodtempo;
	int ifunc;
	int nocode;
	unsigned long ia;
	int levpar;
	int nhomo;
	} PrintargType;

typedef char solset[5];

struct s_context {
	int sign;
	tokenbyte **p_arg;
	};
typedef struct s_context t_context;
typedef struct s_context **p_context;

struct s_list {
	int x;
	int n;
	struct s_list **p;
	};
typedef struct s_list p_list;

struct s_flaglist {
	int x;
	int increment;
	int refvalue;
	int refflag;
	char paramcontrol;
	int operator;
	struct s_flaglist **p;
	};
typedef struct s_flaglist p_flaglist;

struct s_list2 {
	int n;
	int side;
	int i;
	struct s_list2 **p;
	};
typedef struct s_list2 p_list2;

struct s_rule {
	p_flaglist **p_leftflag;	/* list of flags attached to left argument */
	p_flaglist **p_rightflag;	/* list of flags attached to right argument */
	short mode;
	int operator;
	int weight;			/* original weight */
	int incweight;		/* weight increment */
	int w;				/* current weight during computation */
	int ctrl;			/* weight controller, e.g. <K1>,<K2>, etc. */
	char ismeta;			/* '1' if contains metavariables '?x' */
	int leftoffset;		/* length of left (proximate) context (in tokenbytes) */
	int rightoffset;	/* length of right (proximate) context (in tokenbytes) */
	int leftnegcontext;	/* lenght of left negative context common to both arguments (in symbols, i.e. 4 tokenbytes per symbol) */
	tokenbyte **p_leftarg,**p_rightarg;
	p_context p_leftcontext,p_rightcontext;
	int oldgramindex,oldrulindex;
	int gotogram,gotorule;
	int failedgram,failedrule;
	int repeat,repeatcontrol;
	char stop,print,printon,printoff,
		stepon,stepoff,traceon,traceoff,destru;
	};
typedef struct s_rule t_rule;

struct s_subgram {
	int number_rule,type,oldindex;
	short seed;
	t_rule **p_rule;
	char stop,print,printon,printoff,
		stepon,stepoff,traceon,traceoff,destru,randomize;
	};
typedef struct s_subgram t_subgram;

struct s_gram {
	int number_gram;
	int trueBP;
	int hasTEMP;
	int hasproc;
	t_subgram **p_subgram;
	};
typedef struct s_gram t_gram;

typedef struct {
	long clockTime;
	short extra[8];
	} MIDI_Parameters,*MIDI_ParametersPtr;
	
typedef struct {
	int nrlabel;
	int nrarg;
	char ****label;
	char ****arg;
	} scriptcommandtype;

typedef struct {
	int nrlabel;
	int nrarg;
	char ****label;
	char ****arg;
	long **intarg;
	unsigned long **ularg;
	double **floatarg;
	} fullscriptcommandtype;

typedef struct {
	p_list** waitlist;
	p_list** scriptline;
	long** switchstate;
	} objectspecs;
	
#define WaitList(i) (*((*p_ObjectSpecs)[(i)]))->waitlist
#define ObjScriptLine(i) (*((*p_ObjectSpecs)[(i)]))->scriptline
#define SwitchState(i) (*((*p_ObjectSpecs)[(i)]))->switchstate

#define PitchbendStart(i) (*((*p_Instance)[i].contparameters.values))[IPITCHBEND].v0
#define PitchbendEnd(i) (*((*p_Instance)[i].contparameters.values))[IPITCHBEND].v1
#define PitchbendChannel(i) (*((*p_Instance)[i].contparameters.values))[IPITCHBEND].channel
#define PitchbendMode(i) (*((*p_Instance)[i].contparameters.values))[IPITCHBEND].mode

#define PressureStart(i) (*((*p_Instance)[i].contparameters.values))[IPRESSURE].v0
#define PressureEnd(i) (*((*p_Instance)[i].contparameters.values))[IPRESSURE].v1
#define PressureChannel(i) (*((*p_Instance)[i].contparameters.values))[IPRESSURE].channel
#define PressureMode(i) (*((*p_Instance)[i].contparameters.values))[IPRESSURE].mode

#define ModulationStart(i) (*((*p_Instance)[i].contparameters.values))[IMODULATION].v0
#define ModulationEnd(i) (*((*p_Instance)[i].contparameters.values))[IMODULATION].v1
#define ModulationChannel(i) (*((*p_Instance)[i].contparameters.values))[IMODULATION].channel
#define ModulationMode(i) (*((*p_Instance)[i].contparameters.values))[IMODULATION].mode

#define VolumeStart(i) (*((*p_Instance)[i].contparameters.values))[IVOLUME].v0
#define VolumeEnd(i) (*((*p_Instance)[i].contparameters.values))[IVOLUME].v1
#define VolumeChannel(i) (*((*p_Instance)[i].contparameters.values))[IVOLUME].channel
#define VolumeMode(i) (*((*p_Instance)[i].contparameters.values))[IVOLUME].mode

#define PanoramicStart(i) (*((*p_Instance)[i].contparameters.values))[IPANORAMIC].v0
#define PanoramicEnd(i) (*((*p_Instance)[i].contparameters.values))[IPANORAMIC].v1
#define PanoramicChannel(i) (*((*p_Instance)[i].contparameters.values))[IPANORAMIC].channel
#define PanoramicMode(i) (*((*p_Instance)[i].contparameters.values))[IPANORAMIC].mode

struct s_INscripttype {	/* Script line executed by NoteOn */
	int key;
	int chan;
	int scriptline;
	};
typedef struct s_INscripttype INscripttype;


struct s_express {	/* Expressions used in scripts */
	short type;		/* See infra */
	char **name;
	struct s_express **leftarg;
	struct s_express **rightarg;
	int ivariable;
	short valuetype;	/* See infra */
	short instantiated;
	long intvalue;
	double floatvalue;
	};
typedef struct s_express expression;

struct s_regression {
	char isquadra12,isquadra23;
	double a,b,c;
	double a12,b12,a23,b23;
	double scalex,scaley;
	double x2,y2;
	char islogx;
	char islogy;
	char crescent;
	};
typedef struct s_regression regression;

struct s_CsoundLine {
	int instrument;
	double duration;
	int nbparameters;
	double **h_param;
	};
typedef struct s_CsoundLine CsoundLine;

struct s_CsoundParam {
	char **name,**comment,combinationtype;
	int startindex,endindex,table,GENtype;
	int nameindex;
	double defaultvalue;
	};
typedef struct s_CsoundParam CsoundParam;

struct s_CsoundInstrument {
	int iargmax;	/* Total number of arguments */
	int ipmax;	/* Number of untyped parameters */
	CsoundParam** paramlist;
	regression rPitchBend;
	regression rVolume;
	regression rModulation;
	regression rPressure;
	regression rPanoramic;
	double pitchbendrange;
	int pitchbendtable,volumetable,pressuretable,modulationtable,panoramictable;
	int pitchbendGEN,volumeGEN,pressureGEN,modulationGEN,panoramicGEN;
	};
typedef struct s_CsoundInstrument CsoundInstrument;

	
typedef struct {
	long i;
	double value;
	} Coordinates;

typedef struct {
	double x;
	double value;
	} XYCoordinates;

struct s_ParameterStatus {
	double startvalue,endvalue;
	short active,mode;
	long imax;
	Coordinates **point;
	Milliseconds starttime,dur;
	};
typedef struct s_ParameterStatus ParameterStatus;
	

struct s_PerfParameters {
	int numberparams;
	ParameterStatus **params,**startparams[MAXKEY];
	short xpandkey,xpandval;
	char lastistranspose;
	int transpose;
	int level[MAXKEY];
	int velocity[MAXKEY];
	float dilationratio[MAXKEY];
	double starttime[MAXKEY];
	double startpitchbend[MAXKEY];
	int startvolume[MAXKEY];
	long startmodulation[MAXKEY];
	long startpressure[MAXKEY];
	int startpanoramic[MAXKEY];
	};
typedef struct s_PerfParameters PerfParameters;

typedef struct {
	char pressure,volume,panoramic;
	int pitchbend,modulation;
	} MIDIcontrolstatus;

typedef struct {
	int p1,p2,q1,q2;
	} KeyNumberMap;

struct s_CurrentParameters {
	int currvel,rndvel,velcontrol,currarticul,currchan,currinstr;
	float currtranspose;
	short xpandkey,xpandval,randomtime,seed;
	char velmode,articulmode,mapmode,transposemode,lastistranspose;
	KeyNumberMap map0,map1;
	};
typedef struct s_CurrentParameters CurrentParameters;

struct s_ParameterSpecs {
	double v0,v1;
	short index,mode,channel,control;
	long imax;
	Coordinates **point;
	};
typedef struct s_ParameterSpecs ParameterSpecs;

struct s_ContParameterSpecs {
	double start,v0,v1;
	short index,mode,channel,control;
	char active,known;
	float ibeats,maxbeats,increment;
	long imax;
	Coordinates **point;
	};
typedef struct s_ContParameterSpecs ContParameterSpecs;


struct s_ContParameters {
	int number;
	ContParameterSpecs **values;
	};
typedef struct s_ContParameters ContParameters;

struct s_Parameters {
	int number;
	ParameterSpecs **values;
	};
typedef struct s_Parameters Parameters;


struct s_ControlStream {
	double startvalue,difference;
	Milliseconds starttime,dur;
	long ib,ibm;
	short channel;
	};
typedef struct s_ControlStream ControlStream;


struct s_ParameterStream {
	double startvalue,endvalue;
	Milliseconds starttime,duration;
	long ib,ibm;
	short channel;
	long imax;
	Coordinates **point;
	};
typedef struct s_ParameterStream ParameterStream;


struct s_ContinuousControl {
	int number;
	ParameterStream **param;
	};
typedef struct s_ContinuousControl ContinuousControl;


struct s_SoundObjectInstanceParameters {
	Milliseconds starttime,endtime;
	short object,nseq,ncycles,seed;
	Milliseconds truncbeg,truncend;
	char velocity,channel;
	short rndvel,velcontrol,randomtime;
	short xpandkey,xpandval;
	short transposition,instrument;
	double alpha,dilationratio;
	Parameters contparameters;
	KeyNumberMap map0,map1;
	char mapmode,lastistranspose;
	};
typedef struct s_SoundObjectInstanceParameters
	SoundObjectInstanceParameters;


typedef struct {
	long imax;
	int offset;
	Coordinates **point;
	} Table;

typedef struct {
	long imax;
	XYCoordinates **point;
	} XYgraph;
	
typedef struct {
	Milliseconds time;
	int byte;
	int sequence;
	} MIDIcode;

typedef struct {
	long i;
	long imax;
	long pclock;
	long qclock;
	unsigned long tempo;
	Milliseconds period;
	MIDIcode** code;
	char cyclic;
	} MIDIstream;

typedef struct {
	int orgkey,imagekey;
	int k,channel;
	} MappedKey;

typedef struct {
	int field,savefield,subgramstart,subgramend;
	} FieldProcess;
			
// Expression types
#define CONSTANT 0
#define VARIABLE 1
#define PLUS 2
#define MINUS 3
#define MULTIPLY 4
#define DIVIDE 5
#define MOD 6
#define EQUAL 7
#define INF 8
#define SUP 9
#define INFEQUAL 10
#define SUPEQUAL 11
#define DIF 12
#define ASSIGN 13

// Value types
#define CHAR 0
#define INT 1
#define FLOAT 2
#define NOTE 3

// Structures for tree automata

struct s_node {
	short accept;
	struct s_arc **p;
	};
typedef struct s_node node;

struct s_arc {
	char x;
	struct s_arc **next;
	struct s_node **p;
	};
typedef struct s_arc arc;

// Values for node.accept
#define PROC 1
#define CTRL 2
#define TERM 3
#define VAR 4
#define PAT 5
	
// ---------  Macros ------------------------

#define topLeft(r)		(((Point *) &(r))[0])
#define botRight(r)		(((Point *) &(r))[1])
#define ScriptLabel(k)  ((*h_Script)[(k)]).label
#define ScriptArg(k)  ((*h_Script)[(k)]).arg
#define p_ScriptLabelPart(k,i) (*(ScriptLabel(k)))[(i)]
#define p_ScriptArgPart(k,i) (*(ScriptArg(k)))[(i)]
#define ScriptNrLabel(k)  ((*h_Script)[(k)]).nrlabel
#define ScriptNrArg(k)  ((*h_Script)[(k)]).nrarg
#define StringList(i) *((*p_StringList)[(i)])


// Structures for dynamically registering devices & MIDI drivers

// signature for custom menu item enable procedures
typedef OSStatus (*MenuEnableProcPtr)(MenuItemIndex /*index*/, int* /*enable*/, int* /*check*/ );

struct BPMidiDriver {
	struct BPMidiDriver** next;
	UInt32		id;			// a unique ID for the driver
	char			name[64];		// driver name
	int			initOK;		// driver installed without any errors
	DialogPtr		settingsDialog;	// the driver's modeless settings dialog
	MenuItemIndex	firstMItem;		// the first menu item registered
	MenuItemIndex	lastMItem;		// the last menu item registered
	// add function pointers for driver calls here later
};
typedef struct BPMidiDriver BPMidiDriver;

struct DynamicMenuItem {
	struct DynamicMenuItem** next;
	struct BPMidiDriver**    owner;	// driver which owns this item
	UInt32		id;			// a unique ID for this menu item
	MenuID		menu;			// menu to which the item belongs
	MenuItemIndex	index;		// item's position in menu
	char			name[64];		// menu item's text
	MenuEnableProcPtr	enableProc;		// called by MaintainMenus() to decide enabled state
	IntProcPtr		commandProc;	// function to call when selected
	// char*		helpText;		// ??
};
typedef struct DynamicMenuItem DynamicMenuItem;


#include "-BP2.proto.h"

// This macro is used to avoid "unused parameter" warnings
#define BP_NOT_USED(x) (void)(x)

#endif /* _H_BP2 */
