/* ConsoleMain.c (BP3) */
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
char StopfileName[500];

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
FILE * imagePtr;
char imageFileName[500];
int N_image;
long MaxConsoleTime; // seconds: time allowed for console work
int CsoundPianoRollNoteShift;
int NumberScales, MaxScales, DefaultScale, ToldAboutScale; // Microtonal scales loaded from Csound instruments file
t_scale** Scale;
char LastSeen_scale[100]; // Last scale found during compilation of grammar
Handle mem_ptr[5000];
int i_ptr, hist_mem_ptr[5000], size_mem_ptr[5000];

Boolean Find_leak = FALSE; // Flag to locate place where negative leak starts
Boolean check_memory_use = FALSE;

int trace_scale = 0;

int WarnedBlockKey,WarnedRangeKey;

Boolean PrototypesLoaded = FALSE;

MIDI_Event eventStack[MAXMIDIMESSAGES];
long eventCount = 0L;
long eventCountMax = MAXMIDIMESSAGES - 2L;
long initTime = 0L;

int main (int argc, char* args[])
{
	int  result,i,j,this_size,improvize_mem;
	long forgotten_mem, memory_before;
	time_t current_time;
		
//	MemoryUsedInit = MemoryUsed = 0;

	MaxHandles = ZERO;
	MemoryUsed = 0;
	MemoryUsedInit = MemoryUsed;
	SkipFlag = FALSE;
	
//	system("sync");
	ConsoleInit(&gOptions);
//	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (1) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
    ConsoleMessagesInit();
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (2) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	result = ParsePreInitArgs(argc, args, &gOptions);
	if (result == EXIT)  return EXIT_SUCCESS;
	else if (result != OK)  return EXIT_FAILURE;
	
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (3) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	
	if (gOptions.useStdErr)	{
		// split message output from "algorithmic output"
		SetOutputDestinations(odInfo|odWarning|odError|odUserInt, stderr);
	}
	
	if(check_memory_use) BPPrintMessage(odInfo,"Memory before Inits() = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	
	memory_before = MemoryUsed;
	if (Inits() != OK)	return EXIT_FAILURE;
	MemoryUsed = memory_before;
	
	
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (5) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
//	MemoryUsedInit = MemoryUsed;
	
	result = ParsePostInitArgs(argc, args, &gOptions);
	if (result != OK)  return EXIT_FAILURE;
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (6) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);

	result = LoadInputFiles(gOptions.inputFilenames);
	if (result != OK)  return EXIT_FAILURE;
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (7) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	// some command-line options are applied after loading the settings file
	result = ApplyArgs(&gOptions);
	if (result != OK)  return EXIT_FAILURE;

	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (8) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	
#if BIGTEST
	TraceMemory = TRUE;
#else
	TraceMemory = FALSE;
#endif

/* Some things that we might want to do later ...
	
	ResetTicks(TRUE,TRUE,ZERO,0);
*/

	InitOn = FALSE;
	time(&SessionStartTime);
	ProductionTime = ProductionStartTime = PhaseDiagramTime = TimeSettingTime = (time_t) 0L;
	time(&ProductionStartTime);
	BPPrintMessage(odInfo,"\nBP3 Console completed initialization and will use:");
	BPPrintMessage(odInfo,"\n%s\n%s\n\n",gOptions.inputFilenames[wGrammar],gOptions.inputFilenames[wData]);
	
	CreateStopFile();

	SessionTime = clock();
	if (!gOptions.seedProvided) ReseedOrShuffle(NEWSEED);
	
	// load data
	if (!LoadedStartString)  CopyStringToTextHandle(TEH[wStartString], "S\n");
		
//	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (9) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
//	BPPrintMessage(odInfo,"\nMemory used so far = %ld bytes\n",(long)MemoryUsed);

	// The following is a global record of the status of handles created and disposed
	// hist_mem_ptr[] is 1 after a GiveSpace() and 2 after a DisposHandle()
	if(check_memory_use) {
		i_ptr = 0;
		BPPrintMessage(odInfo,"\nChecking memory use starts here with MemoryUsed = %ld and i_ptr = %d\n",(long)MemoryUsed,i_ptr);
		for(i=0; i < 5000; i++) {
			hist_mem_ptr[i] = size_mem_ptr[i] = 0;
			mem_ptr[i] = NULL;
			}
		}
	
	// configure output destinations
	result = PrepareProdItemsDestination(&gOptions);
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (10) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	if (result == OK) result = PrepareTraceDestination(&gOptions);
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (11) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
		
	if (result == OK) {
		// perform the action specified on the command line
		switch (gOptions.action) {
			case compile:
				result = CompileCheck();
				if (Beta && result != OK)  BPPrintMessage(odError,"=> CompileCheck() returned errors\n");
				break;
			case produce:
				if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed start ProduceItems = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
				improvize_mem = Improvize;
				result = ProduceItems(wStartString,FALSE,FALSE,NULL);
			//	if (Beta && result != OK && !improvize_mem)  BPPrintMessage(odError, "=> ProduceItems() returned errors\n");
				break;
			case produce_items:
				break;
			case produce_all:
				AllItems = TRUE;
				time(&ProductionStartTime);
				result = ProduceItems(wStartString,FALSE,FALSE,NULL);
		//		if (Beta && result != OK)  BPPrintMessage(odError, "=> ProduceItems() returned errors\n");
				break;
			case play:
				BPPrintMessage(odInfo,"Playing single item…\n");
				PlaySelectionOn = TRUE;
				Improvize = FALSE;
				result = PlaySelection(wData,0);
				if(result == OK) BPPrintMessage(odInfo,"\nErrors: 0\n");
				else if(Beta && result != OK && result != ABORT) BPPrintMessage(odError,"=> PlaySelection() returned errors\n");
				break;
			case play_item:
				 BPPrintMessage(odInfo,"Playing...\n");
				break;
			case play_all:
				BPPrintMessage(odInfo,"Playing item(s) or chunks…\n");
				PlaySelectionOn = PlayChunks = TRUE;
				Improvize = FALSE;
				result = PlaySelection(wData,1);
				PlayAll = FALSE;
				if(result == OK) BPPrintMessage(odInfo,"\nErrors: 0\n");
				else if(Beta && result != OK && result != ABORT) BPPrintMessage(odError,"=> PlaySelection() returned errors\n");
				break;
			case analyze:
				if(CompileCheck() == OK && ShowNotBP() == OK)	{
					// FIXME: Need to either set a selection or call SelectionToBuffer()
					// and AnalyzeBuffer() similarly to AnalyzeSelection().
					result = AnalyzeSelection(FALSE);
					if (Beta && result != OK)  BPPrintMessage(odError,"=> AnalyzeSelection() returned errors\n");
					}
				break;
			case expand:
				BPPrintMessage(odInfo,"Expanding this item\n");
				Improvize = FALSE;
				result = ExpandSelection(wData);
				if(result == OK) BPPrintMessage(odInfo,"\nErrors: 0\n");
				else if(Beta && result != OK) BPPrintMessage(odError,"=> ExpandSelection() returned errors\n");
				break;
			case show_beats:
				break;
			case templates:
				if(CompileCheck() == OK && ShowNotBP() == OK)	{
					result = ProduceItems(wStartString,FALSE,TRUE,NULL);
			//		if (Beta && result != OK) BPPrintMessage(odError, "=> ProduceItems() returned errors\n");
				}
				break;
			case no_action:
				if (Beta)  BPPrintMessage(odError, "=> Err. main(): action == no_action\n");
				break;
			default:
				if (Beta)  BPPrintMessage(odError, "=> Err. main(): action == %d\n", gOptions.action);
				break;
		}
	}
	
	/* Cleanup ... */
	
	// deallocate any remaining space obtained since Inits()
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (12) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	MyDisposeHandle((Handle*)&Stream.code);
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (13) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	Stream.imax = ZERO;
	Stream.period = ZERO;
	EndImageFile();
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (14) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	if(check_memory_use && (MemoryUsed < MemoryUsedInit)) {
		BPPrintMessage(odInfo,"WARNING! MemoryUsed = %ld < MemoryUsedInit = %ld in %s/%s\n",(long)MemoryUsed,(long)MemoryUsedInit,__FILE__,__FUNCTION__);
		}
	if(TraceMemory && Beta) {
		// reset everything and report memory usage & any leaked space
		if((result = ResetProject(FALSE)) != OK)
			BPPrintMessage(odError, "=> ResetProject() returned errors\n");
		if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (21) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
		// ClearObjectSpace();
		if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (23) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	/*	BPPrintMessage(odInfo, "\nThis session used %ld bytes overall.  %ld handles created and released. [%ld bytes leaked]\n",
			(long) MaxMemoryUsed,(long)MaxHandles,
			(long) (MemoryUsed - MemoryUsedInit)); */
		}
	if(check_memory_use) {
		j = forgotten_mem = 0;
		for(i = 0; i < 5000; i++) {
			if(hist_mem_ptr[i] == 1) { // Handles that we forgot to dispose of
				j++;
				forgotten_mem += size_mem_ptr[i];
				this_size = (int) MyGetHandleSize((Handle)(mem_ptr[i])); // Checking again
				BPPrintMessage(odInfo,"Leaking handle %d containing %ld bytes = %ld, hist_mem = %d\n",(long)i,(long)size_mem_ptr[i],(long)this_size,hist_mem_ptr[i]);
				}
			}
		BPPrintMessage(odInfo, "Uncleared %ld handles for %ld bytes\n",(long)j,(long)forgotten_mem);
		}
	
	// close open files
	CloseOutputDestination(odDisplay, &gOptions, ofiProdItems);
	CloseMIDIFile();
	// CloseFileAndUpdateVolume(&TraceRefnum);
	// CloseFileAndUpdateVolume(&TempRefnum);
	CloseCsScore();
	CloseOutputDestination(odTrace, &gOptions, ofiTraceFile);
	if(InBuiltDriverOn) {
		if(Panic) eventCount = 0L;
		while(eventCount > 0L) {
			MIDIflush();  // Process MIDI events
			WaitABit(1000); // Sleep for 1 millisecond
		//	break;
			}
		AllNotesOffAllChannels();
		closeMIDISystem();
		}
	
	// Create "done.txt" file
	CreateDoneFile();
	time(&current_time);
	if(ProductionTime > 0) BPPrintMessage(odInfo, "Production time: %ld seconds\n",(long)ProductionTime);
	if(PhaseDiagramTime > 0) BPPrintMessage(odInfo, "Phase-diagram filling time: %ld seconds\n",(long)PhaseDiagramTime);
	if(TimeSettingTime > 0) BPPrintMessage(odInfo, "Time-setting time: %ld seconds\n",(long)TimeSettingTime);
	if(current_time > SessionStartTime) BPPrintMessage(odInfo, "Total computation time: %ld seconds\n",(long)(current_time-SessionStartTime));
	
	// deallocate space obtained during Inits() (not strictly necessary)
/*	MyDisposeHandle((Handle*)&p_Oldvalue); */
//	ClearLockedSpace();

	// FIXME: CloseCurrentDriver should eventually work for all drivers - akozar
	// CloseCurrentDriver(FALSE);
	return EXIT_SUCCESS;
}

void CreateDoneFile(void) {
	FILE * thisfile;
	FILE * ptr;
	char line[200];
	char* thefile;
	int length;
	if(gOptions.outputFiles[ofiTraceFile].name != NULL) {
		sprintf(Message,"%s",gOptions.outputFiles[ofiTraceFile].name);
		remove_spaces(Message,line);
		thefile = str_replace(line,".txt","_done.txt"); // Defined in Misc.c
	/*	length = strlen(line2); // This is the code we used before implementing str_replace()
		memset(line1,'\0',sizeof(line1));
		strncpy(line1,line2,length - 4);
		strcat(line1,"_done.txt");
		remove_spaces(line1,line2); */
	    BPPrintMessage(odInfo,"Created 'done' file: %s",thefile);
		BPPrintMessage(odInfo,"\n_____________________\n");
		ptr = fopen(thefile,"w");
		fputs("bp completed work!\n",ptr);
		fclose(ptr);
		free(thefile);
		}
	return;
	}

void CreateStopFile(void) {
		sprintf(StopfileName,"%s","../temp_bolprocessor/messages/_stop");
	    BPPrintMessage(odInfo,"Created path to expected 'stop' file: %s\n",StopfileName);
	return;
	}

int stop() {
	FILE * ptr;
	ptr = fopen(StopfileName,"r");
	if(ptr) {
		Improvize = FALSE;
		BPPrintMessage(odInfo,"Found 'stop' file after producing %ld items: %s\n",(long)ItemNumber,StopfileName);
		fclose(ptr);
		Panic = EmergencyExit = TRUE;
		return ABORT;
		}
	return OK;
	}


void CreateImageFile(void)
{
	FILE * thisfile; 
	char* someline;
	char line1[200], line2[200], line3[200];
	size_t length = 0;
	ssize_t number;
	char cwd[PATH_MAX];
	
	if(imagePtr != NULL) {
		EndImageFile();
		N_image++;
		}
	if(gOptions.outputFiles[ofiTraceFile].name == NULL) {
		BPPrintMessage(odInfo,"=> Cannot create image file because no path is specified and trace mode is not active\n");
		ShowGraphic = ShowPianoRoll = ShowObjectGraph = FALSE;
		return;
		}
	sprintf(line3,"%s",gOptions.outputFiles[ofiTraceFile].name);
	remove_spaces(line3,line2);
//	BPPrintMessage(odInfo,"\n\nline3 = %s\n\n",line3);
	length = strlen(line2);
	memset(line1,'\0',sizeof(line1));
	strncpy(line1,line2,length - 4);
//	BPPrintMessage(odInfo,"\n\nline1 = %s\n\n",line1);
	sprintf(line2,"_image_%03ld_temp.html",(long)N_image);
	if(!PlaySelectionOn && strcmp(gOptions.inputFilenames[wGrammar],"") != 0) { // fixed by BB 2022-02-17
		GetFileName(line3,gOptions.inputFilenames[wGrammar]);
		sprintf(Message,"_%s",line3);
		remove_spaces(Message,line3);
		strcat(line1,line3);
		}
	if(PlaySelectionOn && strcmp(gOptions.inputFilenames[wData],"") != 0) {
		GetFileName(line3,gOptions.inputFilenames[wData]);
		sprintf(Message,"_%s",line3);
		remove_spaces(Message,line3);
		strcat(line1,line3);
		}
	strcat(line1,line2);
	remove_spaces(line1,line2);
    BPPrintMessage(odInfo,"Creating image file: ");
	BPPrintMessage(odInfo,line2);
	BPPrintMessage(odInfo,"\n");
	imagePtr = fopen(line2,"w");
	strcpy(imageFileName,line2);
/*	if(getcwd(cwd,sizeof(cwd))) { // Needs to be revised, if necessary
		BPPrintMessage(odInfo,"\nCurrent working directory = %s\n",cwd);
		}
	else BPPrintMessage(odError,"\n=> Current working directory not found\n",cwd); */
	thisfile = fopen("CANVAS_header.txt","r");
	if(thisfile == NULL) {
		BPPrintMessage(odInfo,"‘CANVAS_header.txt’ is missing!\n");
		fclose(imagePtr);
		}
	else {
		someline = (char *) malloc(100);
		while((number = getline(&someline,&length,thisfile)) != -1) {
			fputs(someline,imagePtr);
			}
		fclose(thisfile);
		fputs("\n",imagePtr);
		free(someline);
		}
	return;
}

void EndImageFile(void)
{
	FILE * thisfile;
	char *pick_a_line = NULL;
	char *final_name = NULL, *someline = NULL, *anotherline = NULL;
	size_t length;
	ssize_t number;
	
	if(imagePtr == NULL) return;
	someline = (char *) malloc(200);
	thisfile = fopen("CANVAS_footer.txt","r");
	if(thisfile == NULL) BPPrintMessage(odInfo,"‘CANVAS_footer.txt’ is missing!\n");
	else {
		while((number = getline(&pick_a_line,&length,thisfile)) != -1) {
        	fputs(pick_a_line,imagePtr);
			}
		fclose(thisfile);
		}
	free(pick_a_line);
  	pick_a_line = NULL;
	fclose(imagePtr);
	imagePtr = NULL;
	// BPPrintMessage(odInfo,"Closing temporary image file\n");
	if(ShowGraphic) {
		final_name = repl_str(imageFileName,"_temp","");
		remove_spaces(final_name,final_name);
		BPPrintMessage(odInfo,"Finalized image file to %s\n",final_name);
		imagePtr = fopen(final_name,"w");
		thisfile = fopen(imageFileName,"r");
		free(final_name);
		while((number = getline(&pick_a_line,&length,thisfile)) != -1) {
			if(strstr(pick_a_line,"THE_TITLE") != NULLSTR) {
				// remove_final_linefeed(pick_a_line,Message);
				if(!PlaySelectionOn) someline = repl_str(pick_a_line,"THE_TITLE",gOptions.inputFilenames[wGrammar]);
				else someline = repl_str(pick_a_line,"THE_TITLE",gOptions.inputFilenames[wData]);
				anotherline = recode_tags(pick_a_line);
				remove_final_linefeed(anotherline,anotherline);
			//	BPPrintMessage(odInfo,"Found: %s\n",anotherline);
				fputs(someline,imagePtr);
				someline = recode_tags(someline);
				remove_final_linefeed(someline,someline);
			//	BPPrintMessage(odInfo,"Replaced with: %s\n",someline);
				}
	      else if(strstr(pick_a_line,"THE_WIDTH") != NULLSTR) {
			//	remove_final_linefeed(pick_a_line,Message);
				sprintf(Message,"%ld",WidthMax);
				someline = repl_str(pick_a_line,"THE_WIDTH",Message);
				fputs(someline,imagePtr);
				anotherline = recode_tags(pick_a_line);
				remove_final_linefeed(anotherline,anotherline);
			//	BPPrintMessage(odInfo,"Found: %s\n",anotherline);
				someline = recode_tags(someline);
				remove_final_linefeed(someline,someline);
			//	BPPrintMessage(odInfo,"Replaced with: %s\n",someline);
				}
	        else if(strstr(pick_a_line,"THE_HEIGHT") != NULLSTR) {
			//	remove_final_linefeed(pick_a_line,Message);
				sprintf(Message,"%ld",HeightMax);
				someline = repl_str(pick_a_line,"THE_HEIGHT",Message);
				fputs(someline,imagePtr);
				anotherline = recode_tags(pick_a_line);
				remove_final_linefeed(anotherline,anotherline);
			//	BPPrintMessage(odInfo,"Found: %s\n",anotherline);
				someline = recode_tags(someline);
				remove_final_linefeed(someline,someline);
			//	BPPrintMessage(odInfo,"Replaced with: %s\n",someline);
				}
	        else fputs(pick_a_line,imagePtr);
	     //   free(pick_a_line);
			}
		fclose(thisfile);
		fclose(imagePtr);
		free(someline);
		imagePtr = NULL;
		}
	remove(imageFileName);
	BPPrintMessage(odInfo,"Removed: %s\n",imageFileName);
	return;
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
	"  play             play item in the input data file\n"
	"  play-item N      play the Nth item in the input data file\n"
	"  play-all         play all items in the input data file\n"
	"  analyze-item N   analyze the Nth item's derivation using the grammar\n"
	"  expand           expand item in the input data file to a complete polymetric expression\n"
	"  show-beats N     display the Nth item using periods to show the beats\n"
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
	"  -mi fname        load sound-object prototypes file 'fname'\n"
	"\n"
	"  These file-type markers currently are recognized but ignored:\n"
	"      -cs  -in  -kb  -md  -mi  -or  -tb  -tr  -wg  +sc \n"
	"\n"
/*	"  -cs fname        load Csound instrument definitions file 'fname'\n"
	"  -de fname        load decisions file 'fname'\n"
	"  -in fname        load interaction file 'fname'\n"
	"  -kb fname        load keyboard file 'fname'\n"
	"  -md fname        load MIDI driver settings file 'fname'\n"
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
	"  --french               specifies that the input files use Italian/Spanish/French note conventions\n"
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
		BPPrintMessage(odError, "=> Not enough arguments...\n\n");
		return ABORT;
	}
	else if (argc < 1) {
		// can this ever happen?
		BPPrintMessage(odError, "=> Error in ParsePreInitArgs(): argc is %d\n", argc);
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
	int argn = 1, arglen, w, resultinit;
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
							BPPrintMessage(odError, "\n=> Missing filename after %s\n\n", args[argn-1]);
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
						BPPrintMessage(odError, "\n=> Missing filename after %s\n\n", args[argn-1]);
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
						BPPrintMessage(odError, "\n=> Missing filename after %s\n\n", args[argn-1]);
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
						BPPrintMessage(odError, "\n=> Missing filename after %s\n\n", args[argn-1]);
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
						BPPrintMessage(odError, "\n=> Missing filename after %s\n\n", args[argn-1]);
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
						opts->midiFileFormat = (int) atol(args[argn]);
						if (opts->midiFileFormat < 0 || opts->midiFileFormat > 2) {
							BPPrintMessage(odError, "\n=> midiformat must be 0, 1, or 2\n\n");
							return ABORT;
						}
					}
					else {
						BPPrintMessage(odError, "\n=> Missing number after --midiformat\n\n");
						return ABORT;
					}
				}
				else if (strcmp(args[argn], "--seed") == 0)	{
					// look at the next argument for an integer seed
					if (++argn < argc && isInteger(args[argn]))  {
						opts->seed = (unsigned int) atol(args[argn]);
						opts->seedProvided = TRUE;
					}
					else {
						BPPrintMessage(odError, "\n=> Missing number after --seed\n\n");
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
					BPPrintMessage(odError, "\n=> Unknown option '%s'\n", args[argn]);
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
				else if (strcmp(args[argn], "expand") == 0)	{
					action = expand;
				}
				else if (strcmp(args[argn], "show-beats") == 0)	{
					action = show_beats;
					// FIXME: look for the item number in next arg
					}
				else if (strcmp(args[argn], "templates") == 0)	{
					action = templates;
					}
				else {
					BPPrintMessage(odError, "\n=> Unknown action '%s'\n", args[argn]);
					BPPrintMessage(odError, "If '%s' is an input file, indicate the file type (eg. -gr %s).\n", args[argn], args[argn]);
					BPPrintMessage(odError, "Use '%s --help' to see help information.\n\n", args[0]);
					return ABORT;
					}
				
				// more than one action is not allowed
				if (action != no_action && opts->action != no_action)	{
					BPPrintMessage(odError, "\n=> Only one action is allowed but two were given: '%s' & '%s'\n\n",ActionTypeToStr(opts->action), ActionTypeToStr(action));
					return ABORT;
					}
				
				opts->action = action;
				}
			}
		++argn;
		}
	
	// an action is required
	if (opts->action == no_action)	{
		BPPrintMessage(odError, "\n=> Missing required action command in arguments.\n");
		BPPrintMessage(odError, "Use '%s --help' to see help information.\n\n", args[0]);
		return ABORT;
		}
	if(opts->useRealtimeMidi == TRUE) {
		resultinit = initializeMIDISystem();
		if(resultinit != 0) return ABORT;
		InBuiltDriverOn = TRUE;
		OutMIDI = TRUE;
		initTime = getClockTime();
	/*	Byte midiData[4];
		int dataSize;
		dataSize = 3;
		midiData[0] = 0x90;
		midiData[1] = 60;
		midiData[2] = 80;
		sendMIDIEvent(midiData,dataSize);
		WaitABit(500000);
		midiData[0] = 0x80;
		midiData[1] = 60;
		midiData[2] = 0;
		sendMIDIEvent(midiData,dataSize);
		WaitABit(1000000);

		eventCount = 0L;
		int localchan = 1;
		eventStack[eventCount].time = (UInt64) 0;
        eventStack[eventCount].type = NORMAL_EVENT;
        eventStack[eventCount].status = NoteOn + localchan;
        eventStack[eventCount].data1 = 72; 
        eventStack[eventCount].data2 = 90;
        eventCount++;
		eventStack[eventCount].time = (UInt64) 1000000; // microseconds
        eventStack[eventCount].type = NORMAL_EVENT;
        eventStack[eventCount].status = NoteOff + localchan;
        eventStack[eventCount].data1 = 72; 
        eventStack[eventCount].data2 = 0;
        eventCount++;
		while(eventCount > 0L) {
			MIDIflush();  // Process MIDI events
			WaitABit(1000); // Sleep for 1 millisecond
			} */
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
		if(Seed > 0) {
			BPPrintMessage(odInfo, "Resetting random seed to %u as per command line\n", Seed);
			ResetRandom();
			}
		else {
			BPPrintMessage(odInfo, "No new random seed as per command line\n");
			Randomize();
			}
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
		case expand:	return "expand";
		case show_beats:	return "show-beats";
		case templates:		return "templates";
		default:
			if (Beta)  BPPrintMessage(odError, "=> Err. ActionTypeToStr(): action == %d\n", action);
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


void GetFileName(char* name,const char* path) { // Added by BB 4 Nov 2020
	char c;
	int i,j;
//	BPPrintMessage(odInfo, "\npath = %s\n",path);
	for(i = strlen(path) - 1; i >= 0; i--) {
		c = path[i];
		if(c == '/') {
			i++; break;
			}
		}
	for(i = i, j = 0; i < strlen(path); i++) {
		c = path[i];
		if(c == ' ') c = '_';
		name[j++] = c;
		}
	name[j] = '\0';
	return;
	}

/*	LoadInputFiles()
	
	Calls LoadFileToTextHandle() for each file in pathnames and copies the contents
	to the corresponding TextHandle in TEH[].  pathnames must be an array of 
	WMAX file/path names with file types that match the window indices in -BP2.h.
	
	Returns MISSED if an error occured or OK if successful.
 */
int LoadInputFiles(const char* pathnames[WMAX])
{
	int w, result;
	
	for(w = 0; w < WMAX; w++) {
		// The order of reading these files is important. For instance, iObjects should occur after wCsoundResources
		if(pathnames[w] != NULL) {
			switch(w) {
				case wGrammar:
				case wAlphabet:
				case wStartString:
				case wData:
				case wGlossary:
					BPPrintMessage(odInfo, "Reading %s file: %s\n", DocumentTypeName[w], pathnames[w]);
					if(check_memory_use) BPPrintMessage(odInfo,"Before Reading %s file MemoryUsed = %ld\n",DocumentTypeName[w],(long)MemoryUsed);
					result = LoadFileToTextHandle(pathnames[w], TEH[w]);
					if(check_memory_use) BPPrintMessage(odInfo,"After Reading %s file MemoryUsed = %ld\n",DocumentTypeName[w],(long)MemoryUsed);
					if (result != OK)  return result;
					switch(w) {
						case wAlphabet:			LoadedAlphabet = TRUE; break;
						case wStartString:		LoadedStartString = TRUE; break;
						case wGlossary:			LoadedGl = TRUE; break;
					}
					break;
				case wCsoundResources: 
				//	BPPrintMessage(odInfo, ": %s\n", pathnames[w]);
					strcpy(FileName[wCsoundResources],pathnames[w]);
					result = LoadCsoundInstruments(0,1);
					if(result != OK)  return result;
					break;
				case iSettings:
					BPPrintMessage(odInfo, "Reading settings file: %s\n", pathnames[w]);
					result = LoadSettings(pathnames[w], FALSE);
					if (result != OK)  return result;
					break;
				case iObjects:
					BPPrintMessage(odInfo, "Reading object prototypes file: %s\n", pathnames[w]);
					strcpy(FileName[iObjects],pathnames[w]);
					result = LoadObjectPrototypes(0,1);
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
	are bad, or MISSED if there was an error.
 */
int LoadFileToTextHandle(const char* pathname, TEHandle th)
{
	int result;
	char **filecontents = NULL;

	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed start LoadFileToTextHandle = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	if (pathname == NULL) {
		if (Beta)  BPPrintMessage(odError, "=> Err. LoadFileToTextHandle(): pathname == NULL\n");
		return ABORT;
	}
	if (th == NULL) {
		if (Beta)  BPPrintMessage(odError, "=> Err. LoadFileToTextHandle(): th == NULL\n");
		return ABORT;
	}

	result = OpenAndReadFile(pathname, &filecontents);
	if (result != OK)  return result;
	// FIXME: Need to convert line endings and HTML -- do it here ?
	// CleanLF(filecontents,&count,&dos);
	// CheckHTML(FALSE,w,filecontents,&count,&html);
	result = CopyStringToTextHandle(th, *filecontents);
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed before end LoadFileToTextHandle = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	MyDisposeHandle((Handle*) &filecontents);
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed end LoadFileToTextHandle = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	return result;
}

/*	OpenAndReadFile()

	Opens the file at 'pathname' and reads the entire contents into
	a new Handle and returns a pointer to that Handle in 'buffer'. 
	
	Returns OK on success, ABORT if the parameters from the caller
	are bad, or MISSED if there was an error.
 */
int OpenAndReadFile(const char* pathname, char*** buffer)
{
	FILE *fin;
	Handle data;
	int result;
	
	if (pathname == NULL) {
		if (Beta)  BPPrintMessage(odError, "=> Err. LoadFileToTextHandle(): pathname == NULL\n");
		return ABORT;
	}
	if (pathname[0] == '\0') {
		if (Beta)  BPPrintMessage(odError, "=> Err. LoadFileToTextHandle(): pathname is empty\n");
		return ABORT;
	}
	if (buffer == NULL) {
		if (Beta)  BPPrintMessage(odError, "=> Err. LoadFileToTextHandle(): buffer == NULL\n");
		return ABORT;
	}

	// open the file for reading
	fin = fopen(pathname, "r");
	if (!fin) {
		BPPrintMessage(odError, "=> Could not open file %s\n", pathname);
		return MISSED;
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
	on success or MISSED if there was an error.
 */
int ReadNewHandleFromFile(FILE* fin, size_t numbytes, Handle* data)
{
	Handle	buffer;
	size_t	bsize;
	long	pos;
	
	if (fin == NULL) {
		if (Beta)  BPPrintMessage(odError, "=> Err. ReadNewHandleFromFile(): fin == NULL\n");
		return ABORT;
	}
	if (data == NULL) {
		if (Beta)  BPPrintMessage(odError, "=> Err. ReadNewHandleFromFile(): data == NULL\n");
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
				BPPrintMessage(odError, "=> Error finding file length (input file may be empty).\n");
				return MISSED;
			}
		}
		else {
			BPPrintMessage(odError, "=> Error seeking to the end of input file.\n");
			return MISSED;
		}
	}
	// else use numbytes and fin pointer as is
	
	// allocate space for data plus a null char
	bsize = numbytes + 1;
	buffer = GiveSpace(bsize);
	if (buffer == NULL) return MISSED;
	
	// read from the file
	if (fread(*buffer, 1, numbytes, fin) != numbytes) {
		BPPrintMessage(odError, "=> Error reading data from file.\n");
		MyDisposeHandle(&buffer);
		return MISSED;
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
	//	if (Beta)  BPPrintMessage(odError, "=> Err. CloseFile(): file == NULL\n");
		return;
	}
	result = fclose(file);
	if (Beta && result != 0) {
		BPPrintMessage(odError, "=> Err. CloseFile(): fclose() error #%d\n",result);
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
		BPPrintMessage(odInfo, "Closed output file: %s\n", opts->outputFiles[fileidx].name);
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
			BPPrintMessage(odError, "=> Could not open file %s\n", opts->outputFiles[ofiProdItems].name);
			return MISSED;
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
		BPPrintMessage(odInfo, "Opening trace file: %s\n", opts->outputFiles[ofiTraceFile].name);
		fout = OpenOutputFile(&(opts->outputFiles[ofiTraceFile]), "w");
		if (!fout) {
			BPPrintMessage(odError, "=> Could not open file %s\n", opts->outputFiles[ofiTraceFile].name);
			return MISSED;
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
