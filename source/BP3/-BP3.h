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

// --------------------------------------------------------------------
// ------  This is the common header used by Bol Processor BP3 --------
// --------------------------------------------------------------------

#ifndef _H_BP3
#define _H_BP3

#define SHORT_VERSION "3.2.0"
#define IDSTRING ( "Version " SHORT_VERSION " (" __DATE__ " - " __TIME__ ")")
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

// Select compilation options (ALL OBSOLETE)
// The only compilation options are the ones selecting code for MacOS, Windows and Linux.

// 1 if building the OS X Carbon GUI, 0 otherwise
// Needs to be removed
#ifndef BP_CARBON_GUI_FORGET_THIS
#define BP_CARBON_GUI_FORGET_THIS 0
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
#  include "cJSON.h"
#endif

#include <sys/stat.h>
#include <dirent.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef	int8_t	sbyte;

typedef	uint8_t		Str255[256], Str63[64], Str31[32];
typedef uint8_t*	StringPtr;

typedef int16_t		OSErr;
typedef int32_t		OSStatus;
typedef uint32_t	OSType, AEEventClass, AEEventID;	// must be a 32-bit unsigned int
typedef OSType		SFTypeList[4];
typedef int16_t		ResID;
typedef int16_t		ScriptCode;
typedef uint16_t	EventModifiers;

typedef int16_t		MenuID;
typedef uint16_t	MenuItemIndex;

typedef char** Handle2;

typedef struct TERec {
	int32_t		selStart;		// start of selection (range [0,length])
	int32_t		selEnd;			// end of selection (range [0,length])
	Handle2		hText;			// text buffer
	int32_t		length;			// length of text in buffer (not buffer size)
} TERec, **TEHandle;

TEHandle NewTextHandle(void);
int CopyStringToTextHandle(TEHandle th, const char* str);

typedef struct {
	long time;
	unsigned char type,status,data1,data2;
	int instance,scale;
	} MIDI_Event, *MIDI_EventPtr;

#define _NOERRORCHECK_	/* Needed in <math.h> */
#define _NOSYNONYMS_

// #include "midi1.h"

/* Valid MIDI_Event types */ 
#define RAW_EVENT 0		/* data only in low order byte */
#define NORMAL_EVENT 1	/* data packed into three low order bytes */
#define NULL_EVENT 2	/* no data */
#define TWO_BYTE_EVENT 3	

/* Add extensions for channel definitions as part of the type byte. */
#define PORTA 0
#define PORTB 32
#define PORTB_COMPLEMENT 0xffdf /* long complement of PORTB */

#define IN 0
#define OUT 1

/* Control/Status codes 
 * These codes are common for both A & B ports */
#define CLOCKTIME_CODE 0		/* control/status */
#define CHANNELIZE_CODE 2		/* control only */
#define CLEAR_SCHEDULER_CODE 3	/* control only */


#if BP_CARBON_GUI_FORGET_THIS
#include "NavServWrapper.h"
#endif

#if !BP_CARBON_GUI_FORGET_THIS
#include "ConsoleMessages.h"
#endif

#if defined(_WIN64)
	#include <windows.h>
	#include <mmsystem.h>
	#include <conio.h> 
	#pragma comment(lib, "winmm.lib")
    #define noErr 0
#elif defined(__APPLE__)
    #include <CoreMIDI/CoreMIDI.h>
    #include <mach/mach_time.h>
	#include <unistd.h>
	#include <termios.h>
#elif defined(__linux__)
    #include <alsa/asoundlib.h>
	#include <unistd.h>
	#include <termios.h>
    #include <time.h>
#endif

// Moved macros and enum down here to avoid potential problems with replacing names
// in any of the above headers -- 010807 akozar
// Note: the following definitions must not be given before loading ColorPicker.h because they mess up CMICCProfile.h
/*#if defined(__POWERPC) && !TARGET_API_MAC_CARBON_FORGET_THIS
#define arrow Qd.arrow
#define screenBits Qd.screenBits
#define randSeed Qd.randSeed
#define dkGray Qd.dkGray
#define ltGray Qd.ltGray
#define gray Qd.gray
#define black Qd.black
#define white Qd.white
#define thePort Qd.thePort
#endif

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
#endif */

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
#define MAXLIN 400		/* length of input line in any file */
#define MAXPORTS 32		/* number of input/output MIDI ports */
#define MAXCLIENTS 512 /* number of MIDI clients (in Linux) */
#define HTMLTOKENLENGTH 80 /* estimated max length of html token */
#define MAXINFOLENGTH 50	/* length of date message */
#define MAXFIELDCONTENT 255	/* length of edit field in dialog window */
#define MAXNAME 200		/* length of file name */
#define MAXBOL 500		/* number of sound-objects */
#define MAXSTRINGCONSTANTS 256	/* string constants */
#define MAXNOTBPCASES 10	/* cases of non standard BP grammars */
#define MAXCOLOR 11		/* predefined colors */
#define MAXTICKS 3		/* ticks in time base */
#define MAXBEATS 40		/* beats in tick cycle */
#define MAXCHAN 16	/* number of MIDI-BP3 channels */
#define MAXPARTS 30	/* number of parts in a score */
#define MAXINSTRUMENTS 512	/* number of Csound instruments refered to by tied notes */
// #define IPMAX 20	/* max number of additional parameters in Csound instrument */
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

/*
// Constants for BP's special file and folder names

#define	kBPPrefsFolder	"\pBol Processor"
#define	kBPSeStartup	"\p-se.startup"
#define	kBPMdStartup	"\p-md.startup"
#define	kBPStartupScript	"\p+sc.startup"
#define	kBPShutdownScript	"\p+sc.shutdown"
#define	kBPTempFile		"\pBP2.temp"
#define	kBPTraceFile	"\pBP2.trace" */

// Constants used in function returns

#define DONE 5
#define IMBEDDED 4
#define EMPTY 3
#define SINGLE 2
#define OK 1
#define YES 1
#define TRUE 1
#define MISSED 0
#define NO 0
#define FALSE 0
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

#define RELATIF 0
#define ABSOLU -1
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
#define IGNORER 3

// File save preferences
#define ALLSAME 0
#define ALLSAMEPROMPT 1
#define NEWFILE 2

#define NOW 0
#define LATER 1

#if defined(__linux__)
#define noErr 0
#endif

// Default values of MIDI parameters
#define DEFTVOLUME 90
#define DEFTVELOCITY 64
#define VOLUMECONTROL 7
#define SAMPLINGRATE 50
#define PANORAMICCONTROL 10
#define DEFTPANORAMIC 64
#define DEFTPITCHBEND 8191.5 
// #define DEFTPITCHBEND 8191 // Fixed by BB 2022-02-20
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
#define T32 (tokenbyte) 32 // _ins()
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
#define T44 (tokenbyte) 44 // _scale()
#define T45 (tokenbyte) 45 // _capture()
#define T46 (tokenbyte) 46 // _part()
#define MAXTOKENBYTE 47

// Inference modes
#define ANAL 0	/* Modus tollens */
#define PROD 1	/* Modus ponens */
#define TEMP 2	/* Producing templates */

// Note conventions
#define MAXCONVENTIONS 100
#define ENGLISH 0
#define FRENCH 1
#define INDIAN 2
#define KEYS 3
// #define CUSTOM 4

#define NOTESIZE 20	/* Length of note/key name. Used by EnglishNotes(), FrenchNotes() */
					/*  and p_NoteName and p_AltNoteName */

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

/*
// Cursor ID's
#define WheelID 1003 // 4 consecutive cursors
#define FeetID 1020 // 8 consecutive cursors
#define DiskID 1200 // 2 consecutive cursors
#define KeyboardID 129
#define QuestionID 128
#define XcrossID 131
*/

// Strings ID's
#define DialogStringsBaseID 300
#define MiscStringsID 302
#define ScriptStringsID 303
#define GramProcedureStringsID 304
#define PerformanceControlStringsID 305
#define GeneralMIDIpatchesID 306
#define HTMLdiacriticalID 307


#define	MAX_STRINGLISTS_LEN	110
#define	MAX_STRINGLISTS_NUMBER	300
/* #define NUM_SCRIPT_CMD_STRS	135
#define NUM_GRAM_PROC_STRS	17
#define NUM_PERF_CONTROL_STRS	68
#define NUM_HTML_DIACRITICAL_STRS	124 */

// Sound ID's
#define kpopID	128
#define kclocID 129

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
#define OKAlert		143
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
#define wCsoundResources 35
#define wPrototype8 36
#define wCsoundTables 37
#define wTonality 38
// #define wMIDIorchestra 38

// DITL indexes
#define GreetingsDitlID 128

// Additional indexes used in file types (see gFileType[] in BP3main.h)
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
#define MAX_BUFFER_SIZE 1024

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



// -----------  BP3 constants  -------------------------------

#define MAXMIDI 1000	/* length of line encoding MIDI event */
#define BOLSIZE 30		/* length of terminal name */
#define MAXFLAG 8		/* default number of flags */
#define MAXEVENTSCRIPT 8	/* default number of event script lines */
#define FIELDSIZE 100	/* standard size of field in poly structure */
#define MAXVAR 30		/* initial number of variables in grammar or script */
#define HOMOSIZE 20		/* length of homomorphism identifier */
#define MAXDISPL 1000	/* minimum size of display */
#define MAXLEVEL 128	/* parentheses level */
#define MAXDERIV 10000 	/* number of derivation steps */
#define MAXTAB 20		/* number of tab sections for display */
#define MAXMETA 32		/* number of metavariables in a rule */
#define MAXMETA2 65		/* 2 * MAXMETA + 1 */
#define MAXPARAMCTRL 128	/* number of MIDI controllers + 1 */
#define MAXWAIT 64		/* number of synchronization tags */
#define MAXTIME 59944L	/* max computation time (milliseconds) */
#define MAXCHARTRACE 2000000
#define MAXCHARDATA 2000000

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
#define MAXCODE2 24		/* number of special characters: ()+;=:/ etc. used by BP3 */
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

// Define platform-specific types
#if defined(_WIN64) || defined(__linux__)
	typedef uint64_t UInt64;
	typedef size_t Size;
	typedef struct Rect {
		int top;
		int left;
		int bottom;
		int right;
		} Rect;
	typedef struct {
		unsigned char* data;  // MIDI data bytes
		int length;           // Number of bytes in the data array
		unsigned long timestamp;  // Timestamp (optional, can be adapted to your needs)
		} MIDIPacket;
	typedef char** Handle;
#endif

typedef TEHandle TextHandle;
typedef long TextOffset;	// should be short, but there are many assumptions of long - akozar

struct s_chunck {
	unsigned long origin,end;
	};
typedef struct s_chunck ChunkPointer;

typedef int (*IntProcPtr)(int);
typedef int (*Int2ProcPtr)(int,int);

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
	int mode;
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

struct s_scale {
	int index;
	int** keys;
	int** keyclass;
	char** label;
	char**** notenames;
	int numgrades,numnotes,basekey,baseoctave;
	double basefreq,interval;
	double** tuningratio;
	short** deviation;
	short** blockkey_shift;
	};
typedef struct s_scale t_scale;

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

struct s_INscripttype {	/* Script line executed by MIDI input event */
	int key;
	int chan;
	int scriptline;
	unsigned long time;
	unsigned long duration;
	};
typedef struct s_INscripttype INscripttype;

struct s_OUTscripttype {
	int key;
	int chan;
	int scriptline;
	unsigned long time;
	unsigned long duration;
	};
typedef struct s_OUTscripttype OUTscripttype;


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
	int currvel,rndvel,velcontrol,currarticul,currchan,scale,blockkey,currinstr,currpart,capture;
	float currtranspose;
	short xpandkey,xpandval,randomtime,seed;
	char velmode,articulmode,mapmode,transposemode,lastistranspose;
	KeyNumberMap map0,map1;
	};
typedef struct s_CurrentParameters CurrentParameters;

struct s_ParameterSpecs {
	double v0,v1;
	short index,mode,channel,control;
	int scale,blockkey;
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
	int scale,blockkey;
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
	int scale,blockkey;
	int capture;
	short rndvel,velcontrol,randomtime;
	short xpandkey,xpandval;
	short transposition,instrument,part;
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

/* // Values for node.accept
#define PROC 1
#define CTRL 2
#define TERM 3
#define VAR 4
#define PAT 5 */
	
// ---------  Macros ------------------------

// #define topLeft(r)		(((Point *) &(r))[0])
// #define botRight(r)		(((Point *) &(r))[1])
#define ScriptLabel(k)  ((*h_Script)[(k)]).label
#define ScriptArg(k)  ((*h_Script)[(k)]).arg
#define p_ScriptLabelPart(k,i) (*(ScriptLabel(k)))[(i)]
#define p_ScriptArgPart(k,i) (*(ScriptArg(k)))[(i)]
#define ScriptNrLabel(k)  ((*h_Script)[(k)]).nrlabel
#define ScriptNrArg(k)  ((*h_Script)[(k)]).nrarg
#define StringList(i) *((*p_StringList)[(i)])

/*
// Structures for dynamically registering devices & MIDI drivers

// signature for custom menu item enable procedures
typedef OSStatus (*MenuEnableProcPtr)(MenuItemIndex , int* , int*  );

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
typedef struct DynamicMenuItem DynamicMenuItem; */

typedef struct {
} NavReplyRecord;

typedef struct {
} NSWReply;

typedef struct {
} WindowRecord, *WindowPtr;

typedef struct {
} Dialog, *DialogPtr, *DialogRef;

typedef struct {
} EventRecord, *EventPtr;

typedef struct {
} AppleEvent;

typedef struct {
} QDGlobals, *GrafPtr, *CGrafPtr, *GWorldPtr, **GDHandle, Cursor;


#include "-BP3.proto.h"

// This macro is used to avoid "unused parameter" warnings
#define BP_NOT_USED(x) (void)(x)

#endif /* _H_BP3 */
