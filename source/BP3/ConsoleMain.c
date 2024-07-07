/* ConsoleMain.c (BP3) */
/* August 7, 2013 */

/*  This file is a part of Bol Processor
Copyright (c) 1990-2000 by Bernard Bel, Jim Kippen and Srikumar K. Subramanian
Copyright (c) 2013, 2019, 2020 by Anthony Kozar
All rights reserved. 

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer. 

Redistributions in binary form must }roduce the above copyright notice,
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

/* #include <stdio.h>
#include <stdlib.h>
#include <string.h> */

#include "-BP2.h"
#include "-BP2main.h"

#include "ConsoleGlobals.h"
#include "ConsoleMessages.h"

const size_t	READ_ENTIRE_FILE = 0;
char StopfileName[500] = {0};
char PanicfileName[500] = {0};

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
int OpenAndReadFile(const char*,char**);
int PrepareProdItemsDestination(BPConsoleOpts* opts);
int PrepareTraceDestination(BPConsoleOpts* opts);
void CloseOutputDestination(int dest, BPConsoleOpts* opts, outfileidx_t fileidx);
int isInteger(const char* s);

// globals only for the console app
int LoadedAlphabet  = FALSE;
int LoadedStartString = FALSE;
BPConsoleOpts gOptions;
FILE * imagePtr;
char imageFileName[500];
int N_image;
long MaxConsoleTime; // seconds: time allowed for console work
int NumberScales, MaxScales, DefaultScale, ToldAboutScale; // Microtonal scales loaded from Csound instruments file
t_scale** Scale;
char LastSeen_scale[100]; // Last scale found during compilation of grammar
Handle mem_ptr[5000];
int i_ptr, hist_mem_ptr[5000], size_mem_ptr[5000];

int Find_leak = FALSE; // Flag to locate place where negative leak starts
int check_memory_use = FALSE;

int trace_scale = 0;

int WarnedBlockKey,WarnedRangeKey;

int PrototypesLoaded = FALSE;

int main (int argc, char* args[])
{
	int  result,i,j,this_size,improvize_mem;
	long forgotten_mem, memory_before;
	time_t current_time;

	MaxHandles = ZERO;
	MemoryUsed = 0;
	MemoryUsedInit = MemoryUsed;
	SkipFlag = FALSE;
	Interactive = FALSE;
	StopPlay = FALSE;
	TraceMIDIinteraction = FALSE;
	TimeStopped = Oldtimestopped = 0L;
	MIDIsyncDelay = 380; // ms default value
	DisplayItems = FALSE;

	NoteOffInputFilter = NoteOnInputFilter = KeyPressureInputFilter = ControlTypeInputFilter = ProgramTypeInputFilter = ChannelPressureInputFilter = PitchBendInputFilter = SysExInputFilter = TimeCodeInputFilter = SongPosInputFilter = SongSelInputFilter = TuneTypeInputFilter = EndSysExInputFilter = ClockTypeInputFilter = StartTypeInputFilter = ContTypeInputFilter = ActiveSenseInputFilter = ResetInputFilter = 3;
	
	ConsoleInit(&gOptions);
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (1) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
    ConsoleMessagesInit();
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (2) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	result = ParsePreInitArgs(argc, args, &gOptions);
	if (result != OK) goto CLEANUP;
	
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (3) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	
	if (gOptions.useStdErr)	{
		// split message output from "algorithmic output"
		SetOutputDestinations(odInfo|odWarning|odError|odUserInt, stderr);
		}
	
	if(check_memory_use) BPPrintMessage(odInfo,"Memory before Inits() = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	
	memory_before = MemoryUsed;
	MemoryUsed = memory_before;
	if (Inits() != OK) goto CLEANUP;
	MemoryUsed = memory_before;
	
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (5) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	MemoryUsedInit = MemoryUsed;
	
	result = ParsePostInitArgs(argc, args, &gOptions);
//    BPPrintMessage(odInfo,"So far it works\n");
	if (result != OK) goto CLEANUP;
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (6) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	result = LoadInputFiles(gOptions.inputFilenames);
	if (result != OK) goto CLEANUP;
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (7) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	// some command-line options are applied after loading the settings file
	result = ApplyArgs(&gOptions);
	if (result != OK) goto CLEANUP;

	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (8) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	
#if BIGTEST
	TraceMemory = TRUE;
#else
	TraceMemory = FALSE;
#endif

	MaxMIDIMessages = 1000L;  // May be increased if necesssary to deal with very large chunks of events in real time

	eventStack = (MIDI_Event*) malloc(MaxMIDIMessages * sizeof(MIDI_Event));
	if(eventStack == NULL) {
        // Memory allocation failed, handle it appropriately
        BPPrintMessage(odError,"=> Failed to allocate %ld cells for MIDI messages. Reduce ‘MaxMIDIMessages’ in ‘ConsoleMain.c’\n",MaxMIDIMessages);
		Panic = TRUE;
        goto CLEANUP;
    	}
	if(rtMIDI) BPPrintMessage(odInfo,"Real-time events use a buffer of MaxMIDIMessages = %ld\n",(long)MaxMIDIMessages);
	
	eventCount = 0L;
	eventCountMax = MaxMIDIMessages - 4L;
	initTime = 0L; // millisconds

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
		
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (9) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
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
				BPPrintMessage(odInfo,"Playing single item...\n");
				PlaySelectionOn = TRUE;
				Improvize = FALSE;
                TextOffset start = 0;
                TextOffset end = GetTextLength(wData);
                SetSelect(start,end,TEH[wData]);
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
				PlayAllChunks = FALSE;
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
	
CLEANUP:
	// deallocate any remaining space obtained since Inits()
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (12) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	MyDisposeHandle((Handle*)&Stream.code);
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (13) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	Stream.imax = ZERO;
	Stream.period = ZERO;
	if(imagePtr != NULL) {
		BPPrintMessage(odInfo, "(Last image) ");
		result = EndImageFile();
		}
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (14) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	if(check_memory_use && (MemoryUsed < MemoryUsedInit)) {
		BPPrintMessage(odInfo,"WARNING! MemoryUsed = %ld < MemoryUsedInit = %ld in %s/%s\n",(long)MemoryUsed,(long)MemoryUsedInit,__FILE__,__FUNCTION__);
		}
	if(TraceMemory && Beta && !Panic) {
		// reset everything and }ort memory usage & any leaked space
		if((result = ResetProject(FALSE)) != OK)
			BPPrintMessage(odError, "=> ResetProject() returned errors\n");
		if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (21) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
		// ClearObjectSpace();
		if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed (23) = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	/*	BPPrintMessage(odInfo, "\nThis session used %ld bytes overall.  %ld handles created and released. [%ld bytes leaked]\n",
			(long) MaxMemoryUsed,(long)MaxHandles,
			(long) (MemoryUsed - MemoryUsedInit)); */
		}
	if(check_memory_use && !Panic) {
		j = forgotten_mem = 0;
		for(i = 0; i < 5000; i++) {
			if(hist_mem_ptr[i] == 1) { // Handles that we forgot to dispose of
				j++;
				forgotten_mem += size_mem_ptr[i];
				this_size = (int) MyGetHandleSize((Handle)(mem_ptr[i])); // Checking again
				BPPrintMessage(odInfo,"Leaking handle %d containing %ld bytes = %ld, hist_mem = %d\n",(long)i,(long)size_mem_ptr[i],(long)this_size,hist_mem_ptr[i]);
				}
			}
		BPPrintMessage(odInfo, "=> Uncleared %ld handles for %ld bytes\n",(long)j,(long)forgotten_mem);
		}
	
	// close open files
	CloseOutputDestination(odDisplay, &gOptions, ofiProdItems);
	CloseMIDIFile();
	// CloseFileAndUpdateVolume(&TraceRefnum);
	// CloseFileAndUpdateVolume(&TempRefnum);
	CloseCsScore();
	CloseOutputDestination(odTrace, &gOptions, ofiTraceFile);
	if(rtMIDI) {
		if(Panic) eventCount = 0L;
		while(eventCount > 0L) {
			if(MIDIflush() != OK) break;  // Process MIDI events
			if((result = WaitABit(10)) != OK) break; // Sleep for 10 milliseconds
			}
		WaitABit(100); // Sleep for 100 milliseconds
		if(ResetNotes) AllNotesOffPedalsOffAllChannels();
		BPPrintMessage(odInfo,"Duration = %.3f seconds\n",(double)LastTime/1000.); // Date of the last MIDI event
		closeMIDISystem();
		WaitABit(100); // 100 milliseconds
		}
    if(result == OK) {
        time(&current_time);
        if(ProductionTime > 0) BPPrintMessage(odInfo, "Production time: %ld seconds\n",(long)ProductionTime);
        if(PhaseDiagramTime > 0) BPPrintMessage(odInfo, "Phase-diagram filling time: %ld seconds\n",(long)PhaseDiagramTime);
        if(TimeSettingTime > 0) BPPrintMessage(odInfo, "Time-setting time: %ld seconds\n",(long)TimeSettingTime);
        if(current_time > SessionStartTime && !Panic) BPPrintMessage(odInfo, "Total computation time: %ld seconds\n",(long)(current_time-SessionStartTime));
        }
	CreateDoneFile();
	free(eventStack);
	return EXIT_SUCCESS;
	}

void CreateDoneFile(void) {
	FILE * thisfile;
	FILE * ptr;
	char line[MAXLIN];
	char* thefile;
	char* new_thefile;
	int length;
	if(gOptions.outputFiles[ofiTraceFile].name != NULL) {
		my_sprintf(Message,"%s",gOptions.outputFiles[ofiTraceFile].name);
		remove_spaces(Message,line);
		thefile = str_replace(".txt","",line);
		if(thefile != NULL) {
            // Allocate new space for modified thefile to include "_done"
            new_thefile = malloc(strlen(thefile) + strlen("_done") + 1);
            if(new_thefile != NULL) {
                strcpy(new_thefile,thefile);
                strcat(new_thefile,"_done");
                snprintf(StopfileName,sizeof(StopfileName),"%s",new_thefile);
            	}
			else {
				BPPrintMessage(odError,"=> Memory allocation failed for new_thefile in CreateDoneFile()\n");
				return;
				}
            }
		else {
			BPPrintMessage(odError, "=> Memory allocation failed for thefile in CreateDoneFile()\n");
			return;
			}
	    BPPrintMessage(odInfo,"Created 'done' file: %s",new_thefile);
	//	BPPrintMessage(odInfo,"\n_____________________\n");
		ptr = my_fopen(1,new_thefile,"w");
		fputs("bp completed work!\n",ptr);
		my_fclose(ptr);
        free(new_thefile);
		free(thefile);
		}
	return;
	}

void CreateStopFile(void) {
	char line[MAXLIN];
	char* thefile;
	char* new_thefile;
	// We may also need to read the "panic" file which is not specific to the project
	my_sprintf(PanicfileName,"%s","../temp_bolprocessor/messages/_panic");
	BPPrintMessage(odInfo,"Created path to expected '_panic' file: %s\n",PanicfileName);
	if(gOptions.outputFiles[ofiTraceFile].name != NULL) {
		my_sprintf(Message,"%s",gOptions.outputFiles[ofiTraceFile].name);
		remove_spaces(Message,line);
		thefile = str_replace(".txt","",line);
		if(thefile != NULL) {
            // Allocate new space for modified thefile to include "_stop"
            new_thefile = malloc(strlen(thefile) + strlen("_stop") + 1);
            if(new_thefile != NULL) {
                strcpy(new_thefile,thefile);
                strcat(new_thefile,"_stop");
                snprintf(StopfileName,sizeof(StopfileName),"%s",new_thefile);
                free(new_thefile);
            	}
			else BPPrintMessage(odError, "=> Memory allocation failed for new_thefile in CreateStopFile()\n");
        	free(thefile);
	    	BPPrintMessage(odInfo,"Created path to expected '_stop' file: %s\n",StopfileName);
            }
		else {
	    	BPPrintMessage(odInfo,"=> Memory allocation failed for thefile in CreateStopFile()\n");
			return;
			}
		}
	else 
	    BPPrintMessage(odInfo,"=> Cannot create path to the expected '_stop' file because no Trace path has been provided\n");
	return;
	}

int stop(int now,char* where) {
	FILE * ptr;
    int r;
	if(Panic || EmergencyExit) return ABORT;
    if((r = ListenMIDI(0,0,0)) != OK) return r;
	unsigned long current_time = getClockTime(); // microseconds
	if(!now && (current_time < NextStop)) return(OK); // We only check _stop and _panic every 500 ms
	NextStop = current_time + 500000L; // microseconds
    if(strlen(StopfileName) == 0) return OK;
	ptr = my_fopen(0,StopfileName,"r");
	if(ptr) {
		Improvize = PlayAllChunks = FALSE;
		my_sprintf(Message,"Found 'stop' file (during “%s”): %s\n",where,StopfileName);
        Notify(Message,0);
        strcpy(Message,"");
		my_fclose(ptr);
		Panic = EmergencyExit = TRUE;
		return ABORT;
		}
    if(strlen(PanicfileName) == 0) return OK;
	ptr = my_fopen(0,PanicfileName,"r");
	if(ptr) {
		Improvize = PlayAllChunks = FALSE;
		BPPrintMessage(odError,"Found 'panic' file: %s\n",PanicfileName);
		my_fclose(ptr);
		Panic = EmergencyExit = TRUE;
        my_fclose(ptr);
		return ABORT;
		}
	return OK;
	}


void CreateImageFile(double time) {
	FILE* thisfile; 
	char* someline;
    char* thefile;
	char* new_thefile;
	char line1[MAXLIN], line2[MAXLIN], line3[MAXLIN];
	size_t length = 0;
	ssize_t number;
	char cwd[4096];
	
    if(!ShowGraphic) return;
	if(imagePtr != NULL) {
		int result = fflush(imagePtr);
		if(result != 0) {
			BPPrintMessage(odError, "=> Error #%d flushing image content\n,result");
			my_fclose(imagePtr);
			return;
			}
		result = EndImageFile();
		if(result != OK) return;
		}
	imageHits = 0;
	N_image++;
//	BPPrintMessage(odInfo,"N_image = %d\n",N_image);
	if(gOptions.outputFiles[ofiTraceFile].name == NULL) {
		BPPrintMessage(odInfo,"=> Cannot create image file because no path is specified and trace mode is not active\n");
		ShowGraphic = ShowPianoRoll = ShowObjectGraph = FALSE;
		return;
		}
    my_sprintf(line1,"%s",gOptions.outputFiles[ofiTraceFile].name);
	remove_spaces(line1,line2);
	thefile = str_replace(".txt","",line2);
	if(thefile == NULL) {
		BPPrintMessage(odError, "=> Error CreateImageFile(). thefile == NULL\n,result");
		return;
		}
	if(time >= 0.)
		my_sprintf(line2,"_image_%03ld-%.2f_temp.html",(long)N_image,(double)time);
	else
		my_sprintf(line2,"_image_%03ld_temp.html",(long)N_image);
	new_thefile = malloc(strlen(thefile) + strlen(line2) + 1);
	strcpy(new_thefile,thefile);
	strcat(new_thefile,line2);
    BPPrintMessage(odInfo,"Creating image #%d: %s\n",N_image,new_thefile);
	imagePtr = my_fopen(1,new_thefile,"w");
	strcpy(imageFileName,new_thefile);
    getcwd(cwd,sizeof(cwd));
    convert_path(cwd);
    if(strlen(cwd) > 259) BPPrintMessage(odError,"=> Warning: this path might be too long: %s\n",cwd);
  //  BPPrintMessage(odInfo,"cwd = %s\n",cwd);
    my_sprintf(line1,"%s/CANVAS_header.txt",cwd);
   // BPPrintMessage(odInfo,"Reading %s\n",line1);
	thisfile = my_fopen(1,line1,"r");
	if(thisfile == NULL) {
		BPPrintMessage(odError,"=> %s is missing!\n",line1);
		my_fclose(imagePtr);
        imagePtr = NULL;
		}
	else {
		someline = (char*) malloc(MAXLIN);
		while(fgets(someline,sizeof(someline),thisfile) != NULL) {
			remove_carriage_returns(someline);
		//	BPPrintMessage(odInfo,"someline = %s\n",someline);
			fputs(someline,imagePtr);
			}
		my_fclose(thisfile);
		fputs("\n",imagePtr);
		free(someline);
		}
	return;
	}

int EndImageFile(void) {
	FILE* thisfile;
	char pick_a_line[MAXLIN];
    char cwd[4096],line[MAXLIN];
	char *final_name = NULL, *someline = NULL, *anotherline = NULL;
	size_t length;
	ssize_t number;
	int result;
	if(imagePtr == NULL) {
		BPPrintMessage(odError,"=> Error closing an image that is already closed\n");
		return ABORT;
		}
	if(imageHits < 1) {
        BPPrintMessage(odError, "Closing an empty image: %s\n",imageFileName);
		my_fclose(imagePtr);
		remove(imageFileName);
        return OK;
        }
	result = fflush(imagePtr);
    if(result != 0) {
        BPPrintMessage(odError, "=> Error #%d flushing image content\n,result");
		my_fclose(imagePtr);
        return ABORT;
        }
	imageHits = 0;
    getcwd(cwd,sizeof(cwd));
    convert_path(cwd);
    my_sprintf(line,"%s/CANVAS_footer.txt",cwd);
	thisfile = my_fopen(1,line,"r");
	if(thisfile == NULL) {
        BPPrintMessage(odInfo,"%s is missing!\n",line);
        }
	else {
		while(fgets(pick_a_line,sizeof(pick_a_line),thisfile) != NULL) {
			remove_carriage_returns(pick_a_line);
        	fputs(pick_a_line,imagePtr);
			}
		my_fclose(thisfile);
		}
	if(imagePtr != NULL) my_fclose(imagePtr);
	imagePtr = NULL;
	// BPPrintMessage(odInfo,"Closing temporary image file\n");
	if(ShowGraphic) {
		final_name = str_replace("_temp","",imageFileName);
		remove_spaces(final_name,final_name);
		if(TraceMIDIinteraction) BPPrintMessage(odInfo,"\n");
		BPPrintMessage(odInfo,"Finalizing image #%d: %s\n",N_image,final_name);
		imagePtr = my_fopen(1,final_name,"w");
		thisfile = my_fopen(1,imageFileName,"r");
		free(final_name);
		while(fgets(pick_a_line, 200, thisfile) != NULL) {
		//	BPPrintMessage(odInfo,"pick_a_line: %s",pick_a_line);
			remove_carriage_returns(pick_a_line);
			if(strstr(pick_a_line,"THE_TITLE") != NULLSTR) {
				if(!PlaySelectionOn) someline = str_replace("THE_TITLE",gOptions.inputFilenames[wGrammar],pick_a_line);
				else someline = str_replace("THE_TITLE",gOptions.inputFilenames[wData],pick_a_line);
			//	BPPrintMessage(odInfo,"someline: %s",someline);
				fputs(someline,imagePtr);
				fflush(imagePtr);
				free(someline);
				}
	      	else if(strstr(pick_a_line,"THE_WIDTH") != NULLSTR) {
				my_sprintf(line,"%ld",WidthMax);
				someline = str_replace("THE_WIDTH",line,pick_a_line);
			//	BPPrintMessage(odInfo,"someline: %s",someline);
				fputs(someline,imagePtr);
				fflush(imagePtr);
				free(someline);
				}
	        else if(strstr(pick_a_line,"THE_HEIGHT") != NULLSTR) {
				my_sprintf(line,"%ld",HeightMax);
				someline = str_replace("THE_HEIGHT",line,pick_a_line);
				fputs(someline,imagePtr);
				fflush(imagePtr);
				free(someline);
				}
	        else fputs(pick_a_line,imagePtr);
			}
		my_fclose(thisfile);
		my_fclose(imagePtr);
		}
	remove(imageFileName);
	imagePtr = NULL;
	BPPrintMessage(odInfo,"Removed %s\n",imageFileName);
	return OK;
    }


void ConsoleInit(BPConsoleOpts* opts) // OBSOLETE
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
	"  compile          check the syntax of input files and }ort errors\n"
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
	"  -al fname        load alphabet file 'fname'\n"
	"  -se fname        load settings file 'fname'\n"
	"  -so fname        load sound-object prototypes file 'fname'\n"
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
		BPPrintMessage(odError,"=> Not enough arguments...\n\n");
		return ABORT;
	}
	else if (argc < 1) {
		// can this ever happen?
		BPPrintMessage(odError, "=> Error in ParsePreInitArgs(): argc is %d\n", argc);
		return ABORT;
	}
	
	while(argn < argc) {
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

/*	Parses all command-line arguments, saves global options,
	action and input/output details (but does not perform actions).
	
	Returns ABORT if an error occured or OK if program should continue.
 */
int ParsePostInitArgs(int argc, char* args[], BPConsoleOpts* opts)
{
	int argn = 1, arglen, w, resultinit, r;
	int argDone;
	action_t action = no_action;
	char* thepath;
	char* new_thepath;

	while(argn < argc) {
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
				if (strcmp(args[argn], "-d") == 0 || strcmp(args[argn], "-D") == 0 || strcmp(args[argn], "--display") == 0)	{ // Modified 2024-06-29
					opts->displayItems = TRUE;
					opts->outOptsChanged = FALSE;
					}
		//		else if (strcmp(args[argn], "-d") == 0 || strcmp(args[argn], "--no-display") == 0)	{
				else if (strcmp(args[argn], "--no-display") == 0)	{
					opts->displayItems = FALSE;
					opts->outOptsChanged = FALSE;
					}
				else  if (strcmp(args[argn], "-o") == 0)	{
					// look at the next argument for the output file name
					if (++argn < argc)  {
						opts->displayItems = TRUE;
						opts->outputFiles[ofiProdItems].name = args[argn];
						opts->outOptsChanged = TRUE;
						OutBPdata = TRUE;
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
	
	if (opts->action == no_action)	{
		BPPrintMessage(odError, "\n=> Missing required action command in arguments.\n");
		BPPrintMessage(odError, "Use '%s --help' to see help information.\n\n", args[0]);
		return ABORT;
		}
	if(opts->useRealtimeMidi == TRUE) {
	// BPPrintMessage(odInfo,"opts->outputFiles[ofiTraceFile].name = %s\n",opts->outputFiles[ofiTraceFile].name);
		thepath = str_replace(".txt","",opts->outputFiles[ofiTraceFile].name);
		if(thepath == NULL) {
			BPPrintMessage(odError, "=> Error ParsePostInitArgs(). thepath == NULL\n,result");
			return ABORT;
			}
		new_thepath = malloc(strlen(thepath) + strlen("_midiport") + 1);
		if(new_thepath == NULL) {
			BPPrintMessage(odError, "=> Error ParsePostInitArgs(). new_thepath == NULL\n,result");
			return ABORT;
			}
		strcpy(new_thepath,thepath);
		free(thepath);
		strcat(new_thepath,"_midiport");
		strcpy(Midiportfilename,new_thepath);
        BPPrintMessage(odInfo,"Midiportfilename = %s\n",Midiportfilename);
		WaitABit(100L); // 100 ms
		resultinit = initializeMIDISystem();
		if(new_thepath != NULL) free(new_thepath);
		if(resultinit != OK) {
			Panic = 1;
			return ABORT;
			}
		WaitABit(100L); // 100 ms
		rtMIDI = TRUE;
		if(ResetNotes) AllNotesOffPedalsOffAllChannels();
		WaitABit(100L); // 100 ms
		if((r = MIDIflush()) != OK) return r;
		Notify("Real-time MIDI started",0);
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
		OutCsound = WriteMIDIfile = rtMIDI = FALSE;
	}
	
	// apply options that were explicitly given on the command line
	if (opts->displayItems != NOCHANGE)		DisplayItems = opts->displayItems;
	if (opts->writeCsoundScore != NOCHANGE)	OutCsound = opts->writeCsoundScore;
	if (opts->writeMidiFile != NOCHANGE)	WriteMIDIfile = opts->writeMidiFile;
	if (opts->useRealtimeMidi != NOCHANGE)	rtMIDI = opts->useRealtimeMidi;
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
			BPPrintMessage(odInfo, "Not using a random seed: shuffling the cards\n");
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

/*	Calls LoadFileToTextHandle() for each file in pathnames and copies the contents
	to the corresponding TextHandle in TEH[].  pathnames must be an array of 
	WMAX file/path names with file types that match the window indices in -BP2.h.
	
	Returns MISSED if an error occured or OK if successful. */
int LoadInputFiles(const char* pathnames[WMAX]) {
	int w, result;
	for(w = 0; w < WMAX; w++) {
		// The order of reading these files is important. For instance, iObjects should occur after wCsoundResources
		if(pathnames[w] != NULL) {
		//	BPPrintMessage(odInfo,"pathnames[%d] = %s\n",w,pathnames[w]);
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
					if(result != OK)  {
						Notify("You first need to save the Grammar or Data",1);
						BPPrintMessage(odError,"\n");
						return result;
						}
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
					BPPrintMessage(odInfo, "Reading sound-object prototypes file: %s\n", pathnames[w]);
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

int LoadFileToTextHandle(const char* pathname,TEHandle th) {
	int result;
	char* filecontents;
	filecontents = NULL;
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed start LoadFileToTextHandle = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	if (pathname == NULL) {
		if (Beta)  BPPrintMessage(odError, "=> Err. LoadFileToTextHandle(): pathname == NULL\n");
		return ABORT;
		}
	if(th == NULL) {
		if (Beta)  BPPrintMessage(odError, "=> Err. LoadFileToTextHandle(): th == NULL\n");
		return ABORT;
		}
	result = OpenAndReadFile(pathname,&filecontents);
//	BPPrintMessage(odInfo,"%s\n",filecontents);
	if(result != OK)  return result;
	result = CopyStringToTextHandle(th,filecontents);
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed before end LoadFileToTextHandle = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	free(filecontents);
	filecontents = NULL;
	if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed end LoadFileToTextHandle = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	return result;	
	}


int OpenAndReadFile(const char* pathname,char** buffer) { // Rewritten 2024-06-25
	FILE *fin;
	int result;
    char line[MAXLIN];
	long pos;
	
	if (pathname == NULL) {
		if (Beta)  BPPrintMessage(odError, "=> Err. LoadFileToTextHandle(): pathname == NULL\n");
		return MISSED;
		}
	if (pathname[0] == '\0') {
		if (Beta)  BPPrintMessage(odError, "=> Err. LoadFileToTextHandle(): pathname is empty\n");
		return MISSED;
		}
	fin = my_fopen(1,pathname, "r");
	if(!fin) {
		BPPrintMessage(odError, "=> Could not open file for reading: %s\n", pathname);
		return MISSED;
		}
//	BPPrintMessage(odInfo, "Opened file: %s\n", pathname);

	*buffer = NULL;
    size_t bufferLength = 0;
    int i = 0;
	pos = ZERO;;
    while(fgets(line, sizeof(line),fin) != NULL) {
		remove_carriage_returns(line);
        if(strlen(line) == 0) continue;
        i++;
  //      BPPrintMessage(odInfo, "line[%d] (%d) = %s\n",i,utf8_strsize(line),line);
		size_t lineSize = utf8_strsize(line);
        char* newBuffer = realloc(*buffer, bufferLength + lineSize + 2); // +1 for '\n' and +1 for '\0'
		if(newBuffer == NULL) {
			// Handle allocation failure; clean up and exit
			BPPrintMessage(odError, "=> Failed to allocate memory for buffer\n");
			return MISSED;
			}
		*buffer =  newBuffer;
        memcpy(*buffer + bufferLength, line, lineSize);
        bufferLength += lineSize;
		}
	if (bufferLength > 0)
		(*buffer)[bufferLength] = '\0'; // Null-terminate the block
	else *buffer = strdup(""); // Handle empty file case
//	BPPrintMessage(odInfo,"OK reading\n");
	my_fclose(fin);
	return OK;
	}

/*	ReadNewHandleFromFile()  OBSOLETE

	Allocates a new Handle and reads data from fin.  If numbytes is
	READ_ENTIRE_FILE, then the file pointer is reset to the beginning
	and the entire file is read; otherwise, ReadNewHandleFromFile reads
	numbytes from the file pointer's current location.
	
	The file data is returned in 'data' and the function returns OK
	on success or MISSED if there was an error.

int ReadNewHandleFromFile(FILE* fin, size_t numbytes, Handle* data) {
    char* buffer;
    size_t bsize;
    long pos;

    if (fin == NULL) {
        if (Beta) BPPrintMessage(odError, "=> Err. ReadNewHandleFromFile(): fin == NULL\n");
        return ABORT;
    }
    if (data == NULL) {
        if (Beta) BPPrintMessage(odError, "=> Err. ReadNewHandleFromFile(): data is NULL\n");
        return ABORT;
    }

    if (numbytes == READ_ENTIRE_FILE) {
        BPPrintMessage(odInfo, "Reading entire file\n");
        // find the length of the file
        if (fseek(fin, 0L, SEEK_END) == 0) {
            pos = ftell(fin);
            if (pos == -1) {
                BPPrintMessage(odError, "=> Error finding file length (input file may be empty)\n");
                return MISSED;
            }
            numbytes = (size_t)pos; // numbytes is file length
            fseek(fin, 0L, SEEK_SET);
            if (fseek(fin, 0L, SEEK_SET) != 0) {  // rewind to beginning
                BPPrintMessage(odError, "=> Error rewinding the file.\n");
                return MISSED;
            }
        } else {
            BPPrintMessage(odError, "=> Error seeking to the end of input file\n");
            return MISSED;
        }
    }

    // allocate space for data plus a null char
    BPPrintMessage(odInfo,"Reading %d bytes\n",(int)numbytes);
    bsize = numbytes + 1;
    buffer = (char*)malloc(bsize);
    if (buffer == NULL) {
        BPPrintMessage(odError, "=> Error allocating memory\n");
        return MISSED;
    }

    // read from the file
    size_t bytesRead = fread(buffer, 1, numbytes, fin);

    // terminate the string and return Handle
    buffer[numbytes] = '\0';
    *data = (Handle)buffer;
    BPPrintMessage(odInfo, "Read entire file:\n%s\n",*data);
    return OK;
} */


FILE* my_fopen(int check, const char* path, const char* mode) {
	char thismode[3];
	if(strcmp(mode,"r") == 0) strcpy(thismode,"rb");
	else strcpy(thismode,mode);
    char convertedPath[4096];  // Buffer to store the converted path
    FILE* file;
	if(path == NULL) {
        BPPrintMessage(odError, "=> Err. in my_fopen(). path == NULL\n");
        return NULL;
		}
    strcpy(convertedPath,path);  // Copy the original path to the buffer
    convert_path(convertedPath);  // Change backslashes to normal
	file = fopen(convertedPath,thismode);
    if(!file) {
		if(check) BPPrintMessage(odError,"=> Failed to open: %s in '%s' mode\n",convertedPath,thismode);
		}
    return file;  // Return the file pointer 
    }

int my_fclose(FILE *file) {
	int result;
    if (file == NULL) {
        BPPrintMessage(odError, "=> Attempt to close a NULL file pointer\n");
        return ABORT;
        }
	result = fflush(file);
    if(result != 0) {
        BPPrintMessage(odError, "=> Error #%d flushing a file\n, result");
		fclose(file);
		file = NULL;
        return ABORT;
        }
	result = fclose(file);
	file = NULL;
    if(result != 0) {
        BPPrintMessage(odError, "=> Error #%d closing a file\n, result");
        return ABORT;
        }
    return OK;
    }

/*	OpenOutputFile()
 
	Open the file controlled by an OutFileInfo struct.
 
	finfo->name should be set to the pathname.
	Returns the FILE pointer referenced by finfo->fout (NULL if failed).
 */
FILE* OpenOutputFile(OutFileInfo* finfo, const char* mode)
{
	finfo->fout = my_fopen(1,finfo->name, mode);
	if (finfo->fout != NULL)  finfo->isOpen = TRUE;
	
	return finfo->fout;
}

/*	CloseOutputFile()
 
	Closes the file controlled by an OutFileInfo struct.
 */
void CloseOutputFile(OutFileInfo* finfo) {
	if(finfo->isOpen && finfo->fout != NULL) {
		my_fclose(finfo->fout);
		finfo->fout = NULL;
		finfo->isOpen = FALSE;
		}
	return;
	}

/*	CloseOutputDestination()
 
	Cleans up an output destination and closes any file associated with it.
 */
void CloseOutputDestination(int dest, BPConsoleOpts* opts, outfileidx_t fileidx) {
	if (opts->outputFiles[fileidx].isOpen)	{
		SetOutputDestinations(dest, NULL);
		CloseOutputFile(&(opts->outputFiles[fileidx]));
		BPPrintMessage(odInfo, "Closed file: %s\n", opts->outputFiles[fileidx].name);
		}
	return;
	}

int PrepareProdItemsDestination(BPConsoleOpts* opts) {
	FILE *fout;
	// prepare output file if requested
	if (opts->displayItems && opts->outputFiles[ofiProdItems].name != NULL)	{
		BPPrintMessage(odInfo, "Opening output file %s\n", opts->outputFiles[ofiProdItems].name);
		fout = OpenOutputFile(&(opts->outputFiles[ofiProdItems]), "w");
		if (!fout) {
			BPPrintMessage(odError, "=> Could not open file for output %s\n", opts->outputFiles[ofiProdItems].name);
			return MISSED;
		    }
		SetOutputDestinations(odDisplay, fout);	
	    }
	return OK;
    }

int PrepareTraceDestination(BPConsoleOpts* opts) {
	FILE *fout;
	// prepare trace output file if requested
	if(opts->outputFiles[ofiTraceFile].name != NULL) {
		fout = OpenOutputFile(&(opts->outputFiles[ofiTraceFile]), "w");
		if(!fout) {
			BPPrintMessage(odError, "=> Could not create trace file %s\n", opts->outputFiles[ofiTraceFile].name);
			return MISSED;
		    }
        else BPPrintMessage(odInfo, "Creating trace file: %s\n", opts->outputFiles[ofiTraceFile].name);
		SetOutputDestinations(odTrace, fout);
	    }
    return OK;
    }

/* Utility functions */

/* Returns TRUE if string s is an integer, otherwise FALSE. */
int isInteger(const char* s) {
	int i = 0;
	if (s[i] != '-' && s[i] != '+' && !isdigit(s[i])) return FALSE;
	while(s[++i] != '\0') {
		if (!isdigit(s[i])) return FALSE;
    	}
	return TRUE;
    }
