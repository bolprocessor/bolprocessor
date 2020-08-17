/* ConsoleMain.c (BP2 version CVS) */
/* August 7, 2013 */

/*  This file is a part of Bol Processor 2
Copyright (c) 1990-2000 by Bernard Bel, Jim Kippen and Srikumar K. Subramanian
Copyright (c) 2013, 2019, 2020 by Anthony Kozar
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "-BP2.h"
#include "-BP2main.h"
#include "ConsoleGlobals.h"
#include "ConsoleMessages.h"
#include "CTextHandles.h"

const size_t	READ_ENTIRE_FILE = 0;

// function prototypes
void ConsoleInit(BPConsoleOpts* opts);
void PrintVersion(void);
void PrintShortVersion(void);
void PrintInputFilenames(BPConsoleOpts* opts);
void PrintUsage(char* programName);
int ParsePreInitArgs(int argc, char* args[], BPConsoleOpts* opts);
int ParsePostInitArgs(int argc, char* args[], BPConsoleOpts* opts);
int ApplyArgs(BPConsoleOpts* opts);
const char* ActionTypeToStr(action_t action);
int LoadInputFiles(const char* pathnames[WMAX]);
int LoadFileToTextHandle(const char* pathname, TEHandle th);
int OpenAndReadFile(const char* pathname, char*** buffer);
int ReadNewHandleFromFile(FILE* fin, size_t numbytes, Handle* data);
int PrepareProdItemsDestination(BPConsoleOpts* opts);
int PrepareTraceDestination(BPConsoleOpts* opts);
void CloseOutputDestination(int dest, BPConsoleOpts* opts, outfileidx_t fileidx);
Boolean isInteger(const char* s);

// globals only for the console app
Boolean LoadedAlphabet = FALSE;
Boolean LoadedStartString = FALSE;
BPConsoleOpts gOptions;


int main (int argc, char* args[])
{
	int  result;
	
	ConsoleInit(&gOptions);
    ConsoleMessagesInit();
	result = ParsePreInitArgs(argc, args, &gOptions);
	if (result == EXIT)  return EXIT_SUCCESS;
	else if (result != OK)  return EXIT_FAILURE;
	
	if (gOptions.useStdErr)	{
		// split message output from "algorithmic output"
		SetOutputDestinations(odInfo|odWarning|odError|odUserInt, stderr);
	}

	if (Inits() != OK)	return EXIT_FAILURE;
	
	result = ParsePostInitArgs(argc, args, &gOptions);
	if (result != OK)  return EXIT_FAILURE;
	result = LoadInputFiles(gOptions.inputFilenames);
	if (result != OK)  return EXIT_FAILURE;
	// some command-line options are applied after loading the settings file
	result = ApplyArgs(&gOptions);
	if (result != OK)  return EXIT_FAILURE;

	
#if BIGTEST
	TraceMemory = TRUE;
#else
	TraceMemory = FALSE;
#endif

/* Some things that we might want to do later ...
	LoadSettings(TRUE,TRUE,TRUE,FALSE,&oms);

	RegisterMidiDrivers(oms);
	
#if WITH_REAL_TIME_MIDI
	if(SetDriver() != OK) goto END;
#endif
	
	LoadMidiDriverStartup();
	
	ResetTicks(TRUE,TRUE,ZERO,0);
*/

	MemoryUsedInit = MemoryUsed;
	InitOn = FALSE;
	BPPrintMessage(odInfo, "BP2 Console completed initialization.\n");
	SessionTime = clock();
	if (!gOptions.seedProvided) ReseedOrShuffle(NEWSEED);
	
	// load data
	if (!LoadedStartString)  CopyStringToTextHandle(TEH[wStartString], "S\n");
	// CopyStringToTextHandle(TEH[wGrammar], gr_Visser3);
	MemoryUsedInit = MemoryUsed;
	
	// configure output destinations
	result = PrepareProdItemsDestination(&gOptions);
	if (result == OK) result = PrepareTraceDestination(&gOptions);
	if (result == OK) {
		// perform the action specified on the command line
		switch (gOptions.action) {
			case compile:
				result = CompileCheck();
				if (Beta && result != OK)  BPPrintMessage(odError, "CompileCheck() returned %d\n", result);
				break;
			case produce:
				result = ProduceItems(wStartString,FALSE,FALSE,NULL);
				if (Beta && result != OK)  BPPrintMessage(odError, "ProduceItems() returned %d\n", result);
				break;
			case produce_items:
				break;
			case produce_all:
				AllItems = TRUE;
				result = ProduceItems(wStartString,FALSE,FALSE,NULL);
				if (Beta && result != OK)  BPPrintMessage(odError, "ProduceItems() returned %d\n", result);
				break;
			case play:
				break;
			case play_item:
				break;
			case play_all:
				break;
			case analyze:
				if(CompileCheck() == OK && ShowNotBP() == OK)	{
					// FIXME: Need to either set a selection or call SelectionToBuffer()
					// and AnalyzeBuffer() similarly to AnalyzeSelection().
					result = AnalyzeSelection(FALSE);
					if (Beta && result != OK)  BPPrintMessage(odError, "AnalyzeSelection() returned %d\n", result);
				}
				break;
			case expand:
				break;
			case show_beats:
				break;
			case templates:
				if(CompileCheck() == OK && ShowNotBP() == OK)	{
					result = ProduceItems(wStartString,FALSE,TRUE,NULL);
					if (Beta && result != OK)  BPPrintMessage(odError, "ProduceItems() returned %d\n", result);
				}
				break;
			case no_action:
				if (Beta)  BPPrintMessage(odError, "Err. main(): action == no_action\n");
				break;
			default:
				if (Beta)  BPPrintMessage(odError, "Err. main(): action == %d\n", gOptions.action);
				break;
		}
	}
	
	/* Cleanup ... */
	
	// deallocate any remaining space obtained since Inits()
	/* MyDisposeHandle((Handle*)&Stream.code);
	Stream.imax = ZERO;
	Stream.period = ZERO; */
	LoadedCsoundInstruments = TRUE;
	if (TraceMemory && Beta) {
		// reset everything and report memory usage & any leaked space
		if ((result = ResetProject(FALSE)) != OK)	BPPrintMessage(odError, "ResetProject() returned %d\n", result);
		BPPrintMessage(odInfo, "This session used %ld Kbytes maximum.  %ld handles created and released. [%ld bytes leaked]\n",
				(long) MaxMemoryUsed/1000L,(long)MaxHandles,
				(long) (MemoryUsed - MemoryUsedInit));
	}
	
	// close open files
	CloseOutputDestination(odDisplay, &gOptions, ofiProdItems);
	CloseOutputDestination(odTrace, &gOptions, ofiTraceFile);
	CloseMIDIFile();
	// CloseFileAndUpdateVolume(&TraceRefnum);
	// CloseFileAndUpdateVolume(&TempRefnum);
	CloseCsScore();
	
	// deallocate space obtained during Inits() (not strictly necessary)
	MyDisposeHandle((Handle*)&p_Oldvalue);
	ClearLockedSpace();

	// FIXME: CloseCurrentDriver should eventually work for all drivers - akozar
	// CloseCurrentDriver(FALSE);
	return EXIT_SUCCESS;
}

void ConsoleInit(BPConsoleOpts* opts)
{
	int i;
	
	opts->action = no_action;
	opts->itemNumber = 0;
	opts->startString = NULL;
	opts->midiInSource = NULL;
	opts->midiOutDestination = NULL;
	
	for (i = 0; i < WMAX; i++)	{
		opts->inputFilenames[i] = NULL;
	}
	for (i = 0; i < MAXOUTFILES; i++) {
		opts->outputFiles[i].name = NULL;
		opts->outputFiles[i].fout = NULL;
		opts->outputFiles[i].isOpen = FALSE;
	}

	opts->useStdErr = FALSE;
	opts->useStartString = FALSE;
	opts->seedProvided = FALSE;
	opts->outOptsChanged = FALSE;
	opts->displayItems = NOCHANGE;
	opts->writeCsoundScore = NOCHANGE;
	opts->writeMidiFile = NOCHANGE;
	opts->useRealtimeMidi = NOCHANGE;
	opts->showProduction = NOCHANGE;
	opts->traceProduction = NOCHANGE;
	opts->noteConvention = NOCHANGE;
	opts->midiFileFormat = NOCHANGE;
	
	return;
}

void PrintVersion(void)
{
	BPPrintMessage(odInfo, "Bol Processor console app\n");
	BPPrintMessage(odInfo, "%s\n", IDSTRING);
	return;
}

void PrintShortVersion(void)
{
	BPPrintMessage(odInfo, "%s\n", SHORT_VERSION);
	return;
}

const char gOptionList[] = 
	"OPTIONS (General):\n"
	"  -h or --help     print this help information\n"
	"  --version        print the program name, version #, and date compiled\n"
	"  --short-version  print just the version number\n"
	"\n"
	"ACTIONS:  Specify which operation to perform.  They are case-insensitive.\n"
	"  produce          produce one item from the grammar\n"
	"  produce-items N  produce N items from the grammar\n"
	"  produce-all      produce all items from the grammar\n"
    "\n"
	"  play             play the first item in the input data file\n"
	"  play-item N      play the Nth item in the input data file\n"
	"  play-all         play all items in the input data file\n"
	"  analyze-item N   analyze the Nth item's derivation using the grammar\n"
	"  expand-item N    expand the Nth item to a complete polymetric expression\n"
	"  show-beats N     print the Nth item using periods to show the beats\n"
	"\n"
	"  compile          check the syntax of input files and report errors\n"
	"  templates        produce templates from the grammar\n" 
	"\n"
	"FILE-TYPES: Input files are automatically recognized if they use BP's naming\n"
	"            conventions (either prefixes or extensions).  Otherwise, specify\n"
	"            the type of input files with the following markers.\n"
	"\n"
	"  These file types can currently be loaded and used:\n"
	"\n"
	"  -da fname        load data file 'fname'\n"
	"  -gl fname        load glossary file 'fname'\n"
	"  -gr fname        load grammar file 'fname'\n"
	"  -ho fname        load alphabet file 'fname'\n"
	"  -se fname        load settings file 'fname'\n"
	"\n"
	"  These file-type markers currently are recognized but ignored:\n"
	"      -cs  -in  -kb  -md  -mi  -or  -tb  -tr  -wg  +sc \n"
	"\n"
/*	"  -cs fname        load Csound instrument definitions file 'fname'\n"
	"  -de fname        load decisions file 'fname'\n"
	"  -in fname        load interaction file 'fname'\n"
	"  -kb fname        load keyboard file 'fname'\n"
	"  -md fname        load MIDI driver settings file 'fname'\n"
	"  -mi fname        load sound-object prototypes file 'fname'\n"
	"  -or fname        load MIDI orchestra file 'fname'\n"
	"  -tb fname        load time base file 'fname'\n"
	"  -wg fname        load weights file 'fname'\n"
	"  +sc fname        load script file 'fname'\n"
 */
	"OPTIONS (Output):\n"
	"  -D or --display        print produced items to standard output (default)\n"
	"  -d or --no-display     don't print produced items to standard output\n"
	"  -o outfile             write produced items to file 'outfile'\n"
	"  -e or --use-stderr     print messages to standard error instead of standard output\n"
	"  --traceout tracefile   write compilation & trace output to file 'tracefile'\n"
	"\n"
	"  --csoundout outfile    write Csound score to file 'outfile' ('-' for stdout)\n"
	"  --midiout outfile      write Midi score to file 'outfile' ('-' for stdout)\n"
	"  --midiformat num       use Midi file format 0, 1, or 2 (default is 1)\n"
	"  --rtmidi destination   play real-time Midi on 'destination'\n"
	"\n"
	"OPTIONS (Computation):\n"
	"  -s or --start string   use 'string' as the start string (default is \"S\")\n"
	"  -S startfile           read the start string from file 'startfile'\n"
	"  --seed num             seeds the random number generator with the integer 'num'\n"
	"  --show-production      outputs the work string at each step of producing items\n"
	"  --trace-production     outputs the work string & selected rule at each step of production\n"
	"\n"
	"OPTIONS (Musical):\n"
	"  --english              specifies that the input files use English note conventions\n"
	"  --french               specifies that the input files use French note conventions\n"
	"  --indian               specifies that the input files use Indian note conventions\n"
	"  --keys                 specifies that the input files use Midi note numbers\n"
	"\n";

void PrintUsage(char* programName)
{
	PrintVersion();
	BPPrintMessage(odInfo, "\nUsage:  %s action [options] { [file-type] inputfile }+\n\n", programName);
	BPPrintMessage(odInfo, gOptionList);
	return;
}

/*	ParsePreInitArgs()
	
	Parses "early" command-line arguments such as --help and --version that cause
	BP to exit without performing other actions and for which we do not want 
	Inits() to run.
	
	Returns EXIT if program should exit when no error occured, ABORT if an error
	occured, or OK if program should continue.
 */
int ParsePreInitArgs(int argc, char* args[], BPConsoleOpts* opts)
{
	int argn = 1;
	
	if (argc == 1) {
		PrintUsage(args[0]);
		BPPrintMessage(odError, "Not enough arguments...\n\n");
		return ABORT;
	}
	else if (argc < 1) {
		// can this ever happen?
		BPPrintMessage(odError, "Error in ParsePreInitArgs(): argc is %d\n", argc);
		return ABORT;
	}
	
	while (argn < argc) {
		if (strcmp(args[argn], "-h") == 0 || strcmp(args[argn], "--help") == 0)	{
			PrintUsage(args[0]);
			return EXIT;
		}
		if (strcmp(args[argn], "--version") == 0)	{
			PrintVersion();
			return EXIT;
		}
		if (strcmp(args[argn], "--short-version") == 0)	{
			PrintShortVersion();
			return EXIT;
		}
		
		// need to parse these args before Init() so that init messages are redirected
		if (strcmp(args[argn], "-e") == 0 || strcmp(args[argn], "--use-stderr") == 0)	{
			opts->useStdErr = TRUE;
		}
		++argn;
	}
	
	return OK;
}

/*	ParsePostInitArgs()
	
	Parses all command-line arguments, saves global options,
	action and input/output details (but does not perform actions).
	
	Returns ABORT if an error occured or OK if program should continue.
 */
int ParsePostInitArgs(int argc, char* args[], BPConsoleOpts* opts)
{
	int argn = 1, arglen, w;
	Boolean argDone;
	action_t action = no_action;

	while (argn < argc) {
		/* check if it is an input file */
		argDone = FALSE;
		if (args[argn][0] == '-' || args[argn][0] == '+') {
			// check for matching file prefix
			arglen = strlen(args[argn]);
			// FIXME: Need to ignore any path components before the filename when matching prefix
			for (w = 0; w < WMAX; w++) {
				// This comparison assumes all prefixes are 3 chars long (not including the '.')
				if (strncmp(args[argn], FilePrefix[w], 3) == 0) {
					if (arglen == 3) {
						// argument is just the file prefix (eg. "-gr"),
						// so look at the next argument for the file name
						if (++argn < argc)  {
							opts->inputFilenames[w] = args[argn];
							argDone = TRUE;
						}
						else {
							BPPrintMessage(odError, "\nMissing filename after %s\n\n", args[argn-1]);
							return ABORT;
						}
					}
					else if (arglen > 4 && args[argn][3] == '.') {
						//  argument is a complete file name (with 1+ chars after '.'), so just save it
						opts->inputFilenames[w] = args[argn];
						argDone = TRUE;
					}
					// else, check for other options below
					break;
				}
			}
		}
		// check for matching file extension
		else if ((w = FindMatchingFileNameExtension(args[argn])) != wUnknown) {
			opts->inputFilenames[w] = args[argn];
			argDone = TRUE;
		}
		
		if (!argDone) {
			/* check if it is an option */
			if (args[argn][0] == '-') {
				if (strcmp(args[argn], "-D") == 0 || strcmp(args[argn], "--display") == 0)	{
					opts->displayItems = TRUE;
					opts->outOptsChanged = TRUE;
				}
				else if (strcmp(args[argn], "-d") == 0 || strcmp(args[argn], "--no-display") == 0)	{
					opts->displayItems = FALSE;
					opts->outOptsChanged = TRUE;
				}
				else if (strcmp(args[argn], "-o") == 0)	{
					// look at the next argument for the output file name
					if (++argn < argc)  {
						opts->displayItems = TRUE;
						opts->outputFiles[ofiProdItems].name = args[argn];
						opts->outOptsChanged = TRUE;
					}
					else {
						BPPrintMessage(odError, "\nMissing filename after %s\n\n", args[argn-1]);
						return ABORT;
					}
				}
				else if (strcmp(args[argn], "-e") == 0 || strcmp(args[argn], "--use-stderr") == 0)	{
					// need to parse these args a second time to avoid "unknown option" error
				}
				else if (strcmp(args[argn], "--traceout") == 0)	{
					// look at the next argument for the output file name
					if (++argn < argc)  {
						opts->outputFiles[ofiTraceFile].name = args[argn];
					}
					else {
						BPPrintMessage(odError, "\nMissing filename after %s\n\n", args[argn-1]);
						return ABORT;
					}
				}
				else if (strcmp(args[argn], "--csoundout") == 0)	{
					// look at the next argument for the output file name
					if (++argn < argc)  {
						opts->writeCsoundScore = TRUE;
						opts->outputFiles[ofiCsScore].name = args[argn];
						opts->outOptsChanged = TRUE;
					}
					else {
						BPPrintMessage(odError, "\nMissing filename after %s\n\n", args[argn-1]);
						return ABORT;
					}
				}
				else if (strcmp(args[argn], "--midiout") == 0)	{
					// look at the next argument for the output file name
					if (++argn < argc)  {
						opts->writeMidiFile = TRUE;
						opts->outputFiles[ofiMidiFile].name = args[argn];
						opts->outOptsChanged = TRUE;
					}
					else {
						BPPrintMessage(odError, "\nMissing filename after %s\n\n", args[argn-1]);
						return ABORT;
					}
				}
				else if (strcmp(args[argn], "--rtmidi") == 0)	{
					opts->useRealtimeMidi = TRUE;
					opts->outOptsChanged = TRUE;
				}
				else if (strcmp(args[argn], "--english") == 0)	{
					opts->noteConvention = ENGLISH;
				}
				else if (strcmp(args[argn], "--french") == 0)	{
					opts->noteConvention = FRENCH;
				}
				else if (strcmp(args[argn], "--indian") == 0)	{
					opts->noteConvention = INDIAN;
				}
				else if (strcmp(args[argn], "--keys") == 0)	{
					opts->noteConvention = KEYS;
				}
				else if (strcmp(args[argn], "--midiformat") == 0)	{
					// look at the next argument for an integer
					if (++argn < argc && isInteger(args[argn]))  {
						opts->midiFileFormat = atoi(args[argn]);
						if (opts->midiFileFormat < 0 || opts->midiFileFormat > 2) {
							BPPrintMessage(odError, "\n--midiformat must be 0, 1, or 2\n\n");
							return ABORT;
						}
					}
					else {
						BPPrintMessage(odError, "\nMissing number after --midiformat\n\n");
						return ABORT;
					}
				}
				else if (strcmp(args[argn], "--seed") == 0)	{
					// look at the next argument for an integer seed
					if (++argn < argc && isInteger(args[argn]))  {
						opts->seed = (unsigned int) atoi(args[argn]);
						opts->seedProvided = TRUE;
					}
					else {
						BPPrintMessage(odError, "\nMissing number after --seed\n\n");
						return ABORT;
					}
				}
				else if (strcmp(args[argn], "--show-production") == 0)	{
					opts->showProduction = TRUE;
				}
				else if (strcmp(args[argn], "--trace-production") == 0)	{
					opts->traceProduction = TRUE;
				}
				/* else if (strcmp(args[argn], "--step-production") == 0)	{
					DisplayProduce = TRUE;
					StepProduce = TRUE;
				}
				else if (strcmp(args[argn], "--step-subgrammars") == 0)	{
					DisplayProduce = TRUE;
					StepGrammars = TRUE;
				}
				else if (strcmp(args[argn], "--choose-rules") == 0)	{
					PlanProduce = TRUE;
					DisplayProduce = TRUE;
					StepProduce = TRUE;
					TraceProduce = TRUE;
				} */
				else {
					BPPrintMessage(odError, "\nUnknown option '%s'\n", args[argn]);
					BPPrintMessage(odError, "Use '%s --help' to see help information.\n\n", args[0]);
					return ABORT;
				}
			}
			else {
				/* check if it is an action */
				// FIXME? help says that actions are case-insensitive
				if (strcmp(args[argn], "compile") == 0)	{
					action = compile;
				}
				else if (strcmp(args[argn], "produce") == 0)	{
					action = produce;
				}
				else if (strcmp(args[argn], "produce-items") == 0)	{
					action = produce_items;
					// FIXME: look for the item number in next arg
				}
				else if (strcmp(args[argn], "produce-all") == 0)	{
					action = produce_all;
				}
				else if (strcmp(args[argn], "play") == 0)	{
					action = play;
				}
				else if (strcmp(args[argn], "play-item") == 0)	{
					action = play_item;
					// FIXME: look for the item number in next arg
				}
				else if (strcmp(args[argn], "play-all") == 0)	{
					action = play_all;
				}
				else if (strcmp(args[argn], "analyze-item") == 0)	{
					action = analyze;
					// FIXME: look for the item number in next arg
				}
				else if (strcmp(args[argn], "expand-item") == 0)	{
					action = expand;
					// FIXME: look for the item number in next arg
				}
				else if (strcmp(args[argn], "show-beats") == 0)	{
					action = show_beats;
					// FIXME: look for the item number in next arg
				}
				else if (strcmp(args[argn], "templates") == 0)	{
					action = templates;
				}
				else {
					BPPrintMessage(odError, "\nUnknown action '%s'\n", args[argn]);
					BPPrintMessage(odError, "If '%s' is an input file, indicate the file type (eg. -gr %s).\n", args[argn], args[argn]);
					BPPrintMessage(odError, "Use '%s --help' to see help information.\n\n", args[0]);
					return ABORT;
				}
				
				// more than one action is not allowed
				if (action != no_action && opts->action != no_action)	{
					BPPrintMessage(odError, "\nOnly one action is allowed but two were given: '%s' & '%s'\n\n",
								   ActionTypeToStr(opts->action), ActionTypeToStr(action));
					return ABORT;
				}
				
				opts->action = action;
			}
		}
		++argn;
	}
	
	// an action is required
	if (opts->action == no_action)	{
		BPPrintMessage(odError, "\nMissing required action command in arguments.\n");
		BPPrintMessage(odError, "Use '%s --help' to see help information.\n\n", args[0]);
		return ABORT;
	}
	
	return OK;
}

/*	ApplyArgs()
	
	Applies command-line options to global options after the settings
	file (if any) is loaded.  This allows options in the settings file
	to be overridden on the command line.
	
	Returns ABORT if an error occured or OK if program should continue.
 */
int ApplyArgs(BPConsoleOpts* opts)
{
	// If any of the score output or performance options were specified,
	// then ignore all of those options from the settings file!
	if (opts->outOptsChanged) {
		DisplayItems = OutCsound = WriteMIDIfile = OutMIDI = FALSE;
	}
	
	// apply options that were explicitly given on the command line
	if (opts->displayItems != NOCHANGE)		DisplayItems = opts->displayItems;
	if (opts->writeCsoundScore != NOCHANGE)	OutCsound = opts->writeCsoundScore;
	if (opts->writeMidiFile != NOCHANGE)	WriteMIDIfile = opts->writeMidiFile;
	if (opts->useRealtimeMidi != NOCHANGE)	OutMIDI = opts->useRealtimeMidi;
	if (opts->traceProduction != NOCHANGE)	{
		DisplayProduce = opts->traceProduction;
		TraceProduce = opts->traceProduction;
	}
	// showProduction could be enabled after traceProduction is disabled
	if (opts->showProduction != NOCHANGE)	DisplayProduce = opts->showProduction;
	if (opts->noteConvention != NOCHANGE)	NoteConvention = opts->noteConvention;
	if (opts->midiFileFormat != NOCHANGE)	MIDIfileType = opts->midiFileFormat;
	if (opts->seedProvided)	{
		Seed = opts->seed;
		ResetRandom();
		BPPrintMessage(odInfo, "Setting seed = %u\n", Seed);
	}
	
	return OK;
}

const char* ActionTypeToStr(action_t action)
{
	switch (action) {
		case no_action:		return "none";
		case compile:		return "compile";
		case produce:		return "produce";
		case produce_items:	return "produce-items";
		case produce_all:	return "produce-all";
		case play:			return "play";
		case play_item:		return "play-item";
		case play_all:		return "play-all";
		case analyze:		return "analyze-item";
		case expand:		return "expand-item";
		case show_beats:	return "show-beats";
		case templates:		return "templates";
		default:
			if (Beta)  BPPrintMessage(odError, "Err. ActionTypeToStr(): action == %d\n", action);
	}
	
	return "";
}

void PrintInputFilenames(BPConsoleOpts* opts)
{
	int w;
	
	for (w = 0; w < WMAX; w++) {
		if (opts->inputFilenames[w] != NULL)
			BPPrintMessage(odInfo, "opts->inputFilenames[%s] = %s\n", WindowName[w], opts->inputFilenames[w]);
	}
	return;
}

/*	LoadInputFiles()
	
	Calls LoadFileToTextHandle() for each file in pathnames and copies the contents
	to the corresponding TextHandle in TEH[].  pathnames must be an array of 
	WMAX file/path names with file types that match the window indices in -BP2.h.
	
	Returns FAILED if an error occured or OK if successful.
 */
int LoadInputFiles(const char* pathnames[WMAX])
{
	int w, result;
	
	for (w = 0; w < WMAX; w++) {
		if (pathnames[w] != NULL) {
			switch(w) {
				case wGrammar:
				case wAlphabet:
				case wStartString:
				case wData:
				case wGlossary:
					BPPrintMessage(odInfo, "Reading %s file %s...\n", DocumentTypeName[w], pathnames[w]);
					result = LoadFileToTextHandle(pathnames[w], TEH[w]);
					if (result != OK)  return result;
					switch(w) {
						case wAlphabet:			LoadedAlphabet = TRUE; break;
						case wStartString:		LoadedStartString = TRUE; break;
						case wGlossary:			LoadedGl = TRUE; break;
					}
					break;
				case iSettings:
					BPPrintMessage(odInfo, "Reading settings file %s...\n", pathnames[w]);
					result = LoadSettings(pathnames[w], FALSE);
					if (result != OK)  return result;
					break;
				default:
					BPPrintMessage(odWarning, "Ignoring %.3s %s (%s files are currently unsupported)\n", FilePrefix[w], pathnames[w], DocumentTypeName[w]);
					break;
			}
		}
	}
	return OK;
}

/*	LoadFileToTextHandle()

	Reads in the entire contents of the file at 'pathname' and copies
	it to the existing TEHandle th. 
	
	Returns OK on success, ABORT if the parameters from the caller
	are bad, or FAILED if there was an error.
 */
int LoadFileToTextHandle(const char* pathname, TEHandle th)
{
	int result;
	char **filecontents = NULL;

	if (pathname == NULL) {
		if (Beta)  BPPrintMessage(odError, "Err. LoadFileToTextHandle(): pathname == NULL\n");
		return ABORT;
	}
	if (th == NULL) {
		if (Beta)  BPPrintMessage(odError, "Err. LoadFileToTextHandle(): th == NULL\n");
		return ABORT;
	}

	result = OpenAndReadFile(pathname, &filecontents);
	if (result != OK)  return result;
	// FIXME: Need to convert line endings and HTML -- do it here ?
	// CleanLF(filecontents,&count,&dos);
	// CheckHTML(w,filecontents,&count,&html);
	result = CopyStringToTextHandle(th, *filecontents);
	MyDisposeHandle((Handle*) &filecontents);
	return result;
}

/*	OpenAndReadFile()

	Opens the file at 'pathname' and reads the entire contents into
	a new Handle and returns a pointer to that Handle in 'buffer'. 
	
	Returns OK on success, ABORT if the parameters from the caller
	are bad, or FAILED if there was an error.
 */
int OpenAndReadFile(const char* pathname, char*** buffer)
{
	FILE *fin;
	Handle data;
	int result;
	
	if (pathname == NULL) {
		if (Beta)  BPPrintMessage(odError, "Err. LoadFileToTextHandle(): pathname == NULL\n");
		return ABORT;
	}
	if (pathname[0] == '\0') {
		if (Beta)  BPPrintMessage(odError, "Err. LoadFileToTextHandle(): pathname is empty\n");
		return ABORT;
	}
	if (buffer == NULL) {
		if (Beta)  BPPrintMessage(odError, "Err. LoadFileToTextHandle(): buffer == NULL\n");
		return ABORT;
	}

	// open the file for reading
	fin = fopen(pathname, "r");
	if (!fin) {
		BPPrintMessage(odError, "Could not open file %s\n", pathname);
		return FAILED;
	}
	
	// read the entire file
	result = ReadNewHandleFromFile(fin, READ_ENTIRE_FILE, &data);
	if (result != OK) {
		*buffer = NULL;
		CloseFile(fin);
		return result;
	}
	else *buffer = (char**) data;
	
	CloseFile(fin);
	return OK;
}

/*	ReadNewHandleFromFile()

	Allocates a new Handle and reads data from fin.  If numbytes is
	READ_ENTIRE_FILE, then the file pointer is reset to the beginning
	and the entire file is read; otherwise, ReadNewHandleFromFile reads
	numbytes from the file pointer's current location.
	
	The file data is returned in 'data' and the function returns OK
	on success or FAILED if there was an error.
 */
int ReadNewHandleFromFile(FILE* fin, size_t numbytes, Handle* data)
{
	Handle	buffer;
	size_t	bsize;
	long	pos;
	
	if (fin == NULL) {
		if (Beta)  BPPrintMessage(odError, "Err. ReadNewHandleFromFile(): fin == NULL\n");
		return ABORT;
	}
	if (data == NULL) {
		if (Beta)  BPPrintMessage(odError, "Err. ReadNewHandleFromFile(): data == NULL\n");
		return ABORT;
	}
	
	if (numbytes == READ_ENTIRE_FILE) {
		// find the length of the file
		if (fseek(fin, 0L, SEEK_END) == 0) {
			pos = ftell(fin); 
			if (pos > 0) {
				numbytes = (size_t)pos;			// numbytes is file length
				fseek(fin, 0L, SEEK_SET);		// rewind to beginning
			}
			else {
				BPPrintMessage(odError, "Error finding file length (input file may be empty).\n");
				return FAILED;
			}
		}
		else {
			BPPrintMessage(odError, "Error seeking to the end of input file.\n");
			return FAILED;
		}
	}
	// else use numbytes and fin pointer as is
	
	// allocate space for data plus a null char
	bsize = numbytes + 1;
	buffer = GiveSpace(bsize);
	if (buffer == NULL) return FAILED;
	
	// read from the file
	if (fread(*buffer, 1, numbytes, fin) != numbytes) {
		BPPrintMessage(odError, "Error reading data from file.\n");
		MyDisposeHandle(&buffer);
		return FAILED;
	}
	
	// terminate the string and return Handle
	((char*)*buffer)[numbytes] = '\0';
	*data = buffer;
	return OK;
}

/*	CloseFile()
 
	Generic wrapper for closing files opened with C library.
 */
void CloseFile(FILE* file)
{
	int result;
	
	if (file == NULL) {
		if (Beta)  BPPrintMessage(odError, "Err. CloseFile(): file == NULL\n");
		return;
	}
	result = fclose(file);
	if (Beta && result != 0) {
		BPPrintMessage(odError, "Err. CloseFile(): fclose() returned an error.\n");
	}
	
	return;
}

/*	OpenOutputFile()
 
	Open the file controlled by an OutFileInfo struct.
 
	finfo->name should be set to the pathname.
	Returns the FILE pointer referenced by finfo->fout (NULL if failed).
 */
FILE* OpenOutputFile(OutFileInfo* finfo, const char* mode)
{
	finfo->fout = fopen(finfo->name, mode);
	if (finfo->fout != NULL)  finfo->isOpen = TRUE;
	
	return finfo->fout;
}

/*	CloseOutputFile()
 
	Closes the file controlled by an OutFileInfo struct.
 */
void CloseOutputFile(OutFileInfo* finfo)
{
	if (finfo->isOpen && finfo->fout != NULL) {
		CloseFile(finfo->fout);
		finfo->fout = NULL;
		finfo->isOpen = FALSE;
	}
	
	return;
}

/*	CloseOutputDestination()
 
	Cleans up an output destination and closes any file associated with it.
 */
void CloseOutputDestination(int dest, BPConsoleOpts* opts, outfileidx_t fileidx)
{
	if (opts->outputFiles[fileidx].isOpen)	{
		SetOutputDestinations(dest, NULL);
		CloseOutputFile(&(opts->outputFiles[fileidx]));
		BPPrintMessage(odInfo, "Closed output file %s\n", opts->outputFiles[fileidx].name);
	}
	
	return;
}

int PrepareProdItemsDestination(BPConsoleOpts* opts)
{
	FILE *fout;
	
	// prepare output file if requested
	if (opts->displayItems && opts->outputFiles[ofiProdItems].name != NULL)	{
		BPPrintMessage(odInfo, "Opening output file %s\n", opts->outputFiles[ofiProdItems].name);
		fout = OpenOutputFile(&(opts->outputFiles[ofiProdItems]), "w");
		if (!fout) {
			BPPrintMessage(odError, "Could not open file %s\n", opts->outputFiles[ofiProdItems].name);
			return FAILED;
		}
		SetOutputDestinations(odDisplay, fout);
		
	}
	
	return OK;
}

int PrepareTraceDestination(BPConsoleOpts* opts)
{
	FILE *fout;
	
	// prepare trace output file if requested
	if (opts->outputFiles[ofiTraceFile].name != NULL)	{
		BPPrintMessage(odInfo, "Opening trace file %s\n", opts->outputFiles[ofiTraceFile].name);
		fout = OpenOutputFile(&(opts->outputFiles[ofiTraceFile]), "w");
		if (!fout) {
			BPPrintMessage(odError, "Could not open file %s\n", opts->outputFiles[ofiTraceFile].name);
			return FAILED;
		}
		SetOutputDestinations(odTrace, fout);
		
	}
	
	return OK;
}

/* Utility functions */

/* Returns TRUE if string s is an integer, otherwise FALSE. */
Boolean isInteger(const char* s)
{
	int i = 0;
	
	if (s[i] != '-' && s[i] != '+' && !isdigit(s[i])) return FALSE;
	while (s[++i] != '\0') {
		if (!isdigit(s[i])) return FALSE;
	}
	
	return TRUE;
}
