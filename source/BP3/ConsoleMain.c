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

#include "-BP3.h"
#include "-BP3main.h"

#include "ConsoleGlobals.h"
#include "ConsoleMessages.h"

const size_t	READ_ENTIRE_FILE = 0;
char StopfileName[500] = {0};
char PanicfileName[500] = {0};
char PausefileName[500] = {0};
char ContinuefileName[500] = {0};

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
FILE * outPtr;
char imageFileName[500];
int N_image;
long MaxConsoleTime; // seconds: time allowed for console work
int NumberScales, MaxScales, DefaultScaleParam, ToldAboutScale; // Microtonal scales loaded from Csound instruments file
t_scale** Scale;
char LastSeen_scale[100]; // Last scale found during compilation of grammar
Handle mem_ptr[5000];
int i_ptr, hist_mem_ptr[5000], size_mem_ptr[5000];

int Find_leak = FALSE; // Flag to locate place where negative leak starts
int check_memory_use = FALSE;

int trace_scale = 0;
int trace_capture = 0;
int force_messages = 0;

int WarnedBlockKey,WarnedRangeKey;

int PrototypesLoaded = FALSE;

int main (int argc, char* args[]) {
	int  result,i,j,this_size;
	long forgotten_mem, memory_before;
	time_t current_time;

	MaxHandles = ZERO;
	MemoryUsed = 0;
	MemoryUsedInit = MemoryUsed;
	SkipFlag = FALSE;
	Interactive = FALSE;
	StopPlay = FALSE;
	PausePlay = FALSE;
	TraceMIDIinteraction = FALSE;
	TimeStopped = Oldtimestopped = 0L;
	MIDIsyncDelay = 380; // ms default value
	DisplayItems = FALSE;

	NoteOffInputFilter = NoteOnInputFilter = KeyPressureInputFilter = ControlTypeInputFilter = ProgramTypeInputFilter = ChannelPressureInputFilter = PitchBendInputFilter = SysExInputFilter = TimeCodeInputFilter = SongPosInputFilter = SongSelInputFilter = TuneTypeInputFilter = EndSysExInputFilter = ClockTypeInputFilter = StartTypeInputFilter = ContTypeInputFilter = ActiveSenseInputFilter = ResetInputFilter = 3;
	
	ConsoleInit(&gOptions);
    ConsoleMessagesInit();
	result = ParsePreInitArgs(argc, args, &gOptions);
	if(result != OK) goto CLEANUP;
	
	if(gOptions.useStdErr)	{
		// split message output from "algorithmic output"
		SetOutputDestinations(odInfo|odWarning|odError|odUserInt, stderr);
		BPPrintMessage(0,odInfo,"Splitting message output from algorithmic output\n");
		}

/*	struct stat buffer;
	if(stat("/Applications/XAMPP/xamppfiles/bin/httpd",&buffer) == 0) {
		// We restart Apache because XAMPP is the server and it needs to be cleanly restarted
		BPPrintMessage(0,odInfo,"We restart Apache because XAMPP is the server\n");
		fflush(stdout);
		fflush(stderr);
	//	int debug_status = system("sudo -l -U root > /tmp/sudo_debug.log 2>&1");
	// 	int restart_status = system("env sudo -u root /Applications/XAMPP/xamppfiles/xampp restartapache");
	//	int restart_status = system("bash -c 'sudo -u root /Applications/XAMPP/xamppfiles/xampp restartapache < /dev/null'");
	//	int restart_status = system("sudo -u root /Applications/XAMPP/xamppfiles/xampp restartapache > /tmp/sudo_output.log 2>&1");

		int restart_status = system("/usr/bin/sudo -u root /Applications/XAMPP/xamppfiles/xampp restartapache");
	//	int restart_status = system("sudo -u root /Applications/XAMPP/xamppfiles/bin/httpd -k restart");
	    if(restart_status != 0) {
			int actual_exit_code = WEXITSTATUS(restart_status);
			BPPrintMessage(0,odError, 
				"=> Failed to restart Apache in XAMPP. MIDI cannot be initialised. (Exit code: %d)\n",actual_exit_code);
			}
		sleep(5); // seconds
		} */


	if(Inits() != OK) goto CLEANUP;
	
	result = ParsePostInitArgs(argc, args, &gOptions);
	if(result != OK) return EXIT_SUCCESS;
	result = LoadInputFiles(gOptions.inputFilenames);
	if(result != OK) goto CLEANUP;
	// some command-line options are applied after loading the settings file
	result = ApplyArgs(&gOptions);
	if(result != OK) goto CLEANUP;
	

	TraceMemory = FALSE;

	MaxMIDIMessages = 1000L;  // May be increased if necesssary to deal with very large chunks of events in real time

	eventStack = (MIDI_Event*) malloc(MaxMIDIMessages * sizeof(MIDI_Event));
	if(eventStack == NULL) {
        // Memory allocation failed, handle it appropriately
        BPPrintMessage(0,odError,"=> Failed to allocate %ld cells for MIDI messages. Reduce ‘MaxMIDIMessages’ in ‘ConsoleMain.c’\n",MaxMIDIMessages);
		Panic = TRUE;
        goto CLEANUP;
    	}
	if(rtMIDI) BPPrintMessage(0,odInfo,"👉 Real-time events use a buffer of MaxMIDIMessages = %ld\n",(long)MaxMIDIMessages);
	
	eventCount = 0L;
	eventCountMax = MaxMIDIMessages - 50L;
	initTime = FirstEventTime = 0L; // millisconds

	InitOn = FALSE;
	time(&SessionStartTime);
	ProductionTime = ProductionStartTime = PhaseDiagramTime = TimeSettingTime = (time_t) 0L;
	time(&ProductionStartTime);
	BPPrintMessage(0,odInfo,"\nBP3 Console completed its initialization and will use:");
	BPPrintMessage(0,odInfo,"\n%s\n%s\n\n",gOptions.inputFilenames[wGrammar],gOptions.inputFilenames[wData]);
	
	CreateStopFile();
	SessionTime = clock();
	if(!gOptions.seedProvided) ReseedOrShuffle(NEWSEED);

	if(!LoadedStartString)  CopyStringToTextHandle(TEH[wStartString], "S\n");

	result = PrepareProdItemsDestination(&gOptions);
	if(result == OK) result = PrepareTraceDestination(&gOptions);
	if(result == OK) {
		// perform the action specified on the command line
		switch (gOptions.action) {
			case compile:
				result = CompileCheck();
				if(result != OK)  BPPrintMessage(0,odError,"=> CompileCheck() returned errors\n");
				break;
			case produce:
				result = ProduceItems(wStartString,FALSE,FALSE,NULL);
				break;
			case produce_items:
				break;
			case produce_all:
				AllItems = TRUE;
				time(&ProductionStartTime);
				result = ProduceItems(wStartString,FALSE,FALSE,NULL);
		//		if(result != OK)  BPPrintMessage(0,odError, "=> ProduceItems() returned errors\n");
				break;
			case play:
				BPPrintMessage(0,odInfo,"Playing single item...\n");
				PlaySelectionOn = TRUE;
				Improvize = FALSE;
                TextOffset start = 0;
                TextOffset end = GetTextLength(wData);
                SetSelect(start,end,TEH[wData]);
				result = PlaySelection(wData,0);
				if(result == OK) BPPrintMessage(0,odInfo,"\nErrors: 0\n");
				else if(Beta && result != OK && result != ABORT) BPPrintMessage(0,odError,"=> PlaySelection() returned errors\n");
				break;
			case play_item:
				 BPPrintMessage(0,odInfo,"Playing...\n");
				break;
			case play_all:
				BPPrintMessage(0,odInfo,"Playing item(s) or chunks…\n");
				PlaySelectionOn = PlayChunks = TRUE;
				Improvize = FALSE;
				result = PlaySelection(wData,1);
				PlayAllChunks = FALSE;
				if(result == OK) BPPrintMessage(0,odInfo,"\nErrors: 0\n");
				else if(Beta && result != OK && result != ABORT) BPPrintMessage(0,odError,"=> PlaySelection() returned errors\n");
				break;
			case analyze:
				if(CompileCheck() == OK && ShowNotBP() == OK)	{
					// FIXME: Need to either set a selection or call SelectionToBuffer()
					// and AnalyzeBuffer() similarly to AnalyzeSelection().
					result = AnalyzeSelection(FALSE);
					if(result != OK)  BPPrintMessage(0,odError,"=> AnalyzeSelection() returned errors\n");
					}
				break;
			case expand:
				BPPrintMessage(0,odInfo,"Expanding this item\n");
				Improvize = FALSE;
				result = ExpandSelection(wData);
				if(result == OK) BPPrintMessage(0,odInfo,"\nErrors: 0\n");
				else if(Beta && result != OK) BPPrintMessage(0,odError,"=> ExpandSelection() returned errors\n");
				break;
			case show_beats:
				break;
			case templates:
				if(CompileCheck() == OK && ShowNotBP() == OK)	{
					result = ProduceItems(wStartString,FALSE,TRUE,NULL);
			//		if(Beta && result != OK) BPPrintMessage(0,odError, "=> ProduceItems() returned errors\n");
				}
				break;
			case no_action:
				  BPPrintMessage(0,odError, "=> Err. main(): action == no_action\n");
				break;
			default:
				  BPPrintMessage(0,odError, "=> Err. main(): action == %d\n", gOptions.action);
				break;
			}
		}
CLEANUP:
	// deallocate any remaining space obtained since Inits()
	MyDisposeHandle((Handle*)&Stream.code);
	Stream.imax = ZERO;
	Stream.period = ZERO;
	if(imagePtr != NULL) {
		BPPrintMessage(0,odInfo, "(Last image) ");
		result = EndImageFile();
		}
	// Close open files
	CloseMIDIFile();
	// CloseFileAndUpdateVolume(&TraceRefnum);
	// CloseFileAndUpdateVolume(&TempRefnum);
	CloseCsScore();
	if(rtMIDI) {
		if(Panic) eventCount = 0L;
	//	MIDIflush(0);
		while(eventCount > 0L) {
			if(MIDIflush(0) != OK) break;  // Process MIDI events
			if((result = WaitABit(10)) != OK) break; // Sleep for 10 milliseconds
			}
		WaitABit(100); // Sleep for 100 milliseconds
		BPPrintMessage(0,odInfo,"Duration = %.3f seconds\n",(double)LastTime/1000.); // Date of the last MIDI event
		if(ResetNotes) {
			AllNotesOffPedalsOffAllChannels(TRUE);
			}
		WaitABit(100); // 100 milliseconds
		closeMIDISystem();
		WaitABit(100); // 100 milliseconds
		}
	CloseOutputDestination(odDisplay, &gOptions, ofiProdItems);
	CloseOutputDestination(odTrace, &gOptions, ofiTraceFile);
	Handle ptr = (Handle) p_Instance;
	MyDisposeHandle(&ptr);
	my_fclose(CapturePtr);
	p_Instance = NULL;
    if(result == OK) {
        time(&current_time);
        if(ProductionTime > 0) BPPrintMessage(0,odInfo, "Production time: %ld seconds\n",(long)ProductionTime);
        if(PhaseDiagramTime > 0) BPPrintMessage(0,odInfo, "Phase-diagram filling time: %ld seconds\n",(long)PhaseDiagramTime);
        if(TimeSettingTime > 0) BPPrintMessage(0,odInfo, "Time-setting time: %ld seconds\n",(long)TimeSettingTime);
        if(current_time > SessionStartTime && !Panic) BPPrintMessage(0,odInfo, "Total computation time: %ld seconds\n",(long)(current_time-SessionStartTime));
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
	//    BPPrintMessage(0,odInfo,"Creating 'done' file: %s\n",gOptions.outputFiles[ofiTraceFile].name);
		my_sprintf(Message,"%s",gOptions.outputFiles[ofiTraceFile].name);
		remove_spaces(Message,line);
		thefile = str_replace(".txt","",line);
		if(thefile != NULL) {
            // Allocate new space for modified thefile to include "_done"
            new_thefile = malloc(strlen(thefile) + strlen("_done") + 1);
            if(new_thefile != NULL) {
                strcpy(new_thefile,thefile);
                strcat(new_thefile,"_done");
          //      snprintf(StopfileName,sizeof(StopfileName),"%s",new_thefile);
            	}
			else {
				BPPrintMessage(0,odError,"=> Memory allocation failed for new_thefile in CreateDoneFile()\n");
				return;
				}
            }
		else {
			BPPrintMessage(0,odError, "=> Memory allocation failed for thefile in CreateDoneFile()\n");
			return;
			}
	//	BPPrintMessage(0,odInfo,"\n_____________________\n");
		ptr = my_fopen(1,new_thefile,"w");
		if(ptr != NULL) {
	    	BPPrintMessage(0,odInfo,"Created 'done' file: %s\n",new_thefile);
			fputs("bp completed work!\n",ptr);
			my_fclose(ptr);
			}
		else BPPrintMessage(0,odError,"=> Error creating 'done' file: %s\n",new_thefile);
        free(new_thefile);
		free(thefile);
		}
//	else BPPrintMessage(0,odError,"=> No path found in CreateDoneFile() [1]\n");
	return;
	}

void CreateStopFile(void) {
	char line[MAXLIN];
	char* thefile;
	char* new_thefile;
	my_sprintf(PausefileName,"%s","../temp_bolprocessor/messages/_pause");
	my_sprintf(ContinuefileName,"%s","../temp_bolprocessor/messages/_continue");
	// We may also need to read the "panic" file which is not specific to the project
	my_sprintf(PanicfileName,"%s","../temp_bolprocessor/messages/_panic");
	BPPrintMessage(0,odInfo,"Created path to expected '_panic' file: %s\n",PanicfileName);
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
			else BPPrintMessage(0,odError, "=> Memory allocation failed for new_thefile in CreateStopFile()\n");
        	free(thefile);
	    	BPPrintMessage(0,odInfo,"Created path to expected '_stop' file: %s\n",StopfileName);
            }
		else {
	    	BPPrintMessage(0,odInfo,"=> Memory allocation failed for thefile in CreateStopFile()\n");
			return;
			}
		}
	else 
	    BPPrintMessage(0,odInfo,"=> Cannot create path to the expected '_stop' file because no Trace path has been provided\n");
	return;
	}

FILE* CreateCaptureFile(FILE* oldptr) {
	FILE * thisfile;
	FILE * ptr;
	char line[MAXLIN];
	char* thefile;
	char* new_thefile;
	int length;
	if(oldptr != NULL) return oldptr;
	if(gOptions.outputFiles[ofiTraceFile].name != NULL) {
		my_sprintf(Message,"%s",gOptions.outputFiles[ofiTraceFile].name);
		remove_spaces(Message,line);
		thefile = str_replace(".txt","",line);
		if(thefile != NULL) {
            // Allocate new space for modified thefile to include "_capture"
            new_thefile = malloc(strlen(thefile) + strlen("_capture") + 1);
            if(new_thefile != NULL) {
                strcpy(new_thefile,thefile);
                strcat(new_thefile,"_capture");
            	}
			else {
				BPPrintMessage(0,odError,"=> Memory allocation failed for new_thefile in CreateCaptureFile()\n");
				return NULL;
				}
            }
		else {
			BPPrintMessage(0,odError, "=> Memory allocation failed for thefile in CreateCaptureFile()\n");
			return NULL;
			}
		ptr = my_fopen(1,new_thefile,"w");
		free(thefile);
		if(ptr != NULL) {
	    	BPPrintMessage(0,odInfo,"Creating 'capture' file: %s\n",new_thefile);
			fprintf(ptr, "time\tnote\tevent\tsize\tsource\tpart\tstatus\tdata1\tdata2\tchannel\tcents correction\tmethod\n");
        	free(new_thefile);
			return ptr;
			}
		}
//	else BPPrintMessage(0,odError,"=> No path found in CreateCaptureFile() [2]\n");
	return NULL;
	}

int stop(int now,char* where) {
	FILE * ptr;
    int r;
	if(Panic || EmergencyExit) return ABORT;
    if((r = ListenToEvents()) != OK) return r;
	unsigned long current_time = getClockTime(); // microseconds
	if(rtMIDI && !now && (current_time < NextStop)) return(OK); // We only check _stop and _panic every 500 ms
	NextStop = current_time + 500000L; // microseconds
    if(strlen(StopfileName) == 0) return OK;
	ptr = my_fopen(0,StopfileName,"r");
	if(ptr) {
		Improvize = PlayAllChunks = FALSE;
		my_sprintf(Message,"Found 'stop' file (during “%s”): %s\n",where,StopfileName);
        Notify(Message,0);
        strcpy(Message,"");
		Panic = EmergencyExit = TRUE;
		my_fclose(ptr);
		my_fclose(CapturePtr);
		return ABORT;
		}
    if(strlen(ContinuefileName) > 0) {
		ptr = my_fopen(0,ContinuefileName,"r");
		if(ptr) {
			my_fclose(ptr);
			PausePlay = FALSE;
			remove(PausefileName);
			remove(ContinuefileName);
			return OK;
			}
		}
    if(strlen(PausefileName) > 0) {
		ptr = my_fopen(0,PausefileName,"r");
		if(ptr) {
			my_fclose(ptr);
			PausePlay = TRUE;
		//	my_fclose(CapturePtr);
			return OK; // The pause file is not deleted immediately
			}
		}
    if(strlen(PanicfileName) > 0) {
		ptr = my_fopen(0,PanicfileName,"r");
		if(ptr) {
			Improvize = PlayAllChunks = FALSE;
			BPPrintMessage(0,odError,"👉 Found 'panic' file: %s\n",PanicfileName);
			my_fclose(ptr);
			Panic = EmergencyExit = TRUE;
			my_fclose(CapturePtr);
			return ABORT;
			}
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
			BPPrintMessage(0,odError, "=> Error #%d flushing image content\n,result");
			my_fclose(imagePtr);
			return;
			}
		result = EndImageFile();
		if(result != OK) return;
		}
	imageHits = 0;
	N_image++;
//	BPPrintMessage(0,odInfo,"N_image = %d\n",N_image);
	if(gOptions.outputFiles[ofiTraceFile].name == NULL) {
		BPPrintMessage(0,odInfo,"=> Cannot create image file because no path is specified and trace mode is not active\n");
		ShowGraphic = ShowPianoRoll = ShowObjectGraph = FALSE;
		return;
		}
    my_sprintf(line1,"%s",gOptions.outputFiles[ofiTraceFile].name);
	remove_spaces(line1,line2);
	thefile = str_replace(".txt","",line2);
	if(thefile == NULL) {
		BPPrintMessage(0,odError, "=> Error CreateImageFile(). thefile == NULL\n,result");
		return;
		}
	if(time >= 0.)
		my_sprintf(line2,"_image_%03ld-%.2f_temp.html",(long)N_image,(double)time);
	else
		my_sprintf(line2,"_image_%03ld_temp.html",(long)N_image);
	new_thefile = malloc(strlen(thefile) + strlen(line2) + 1);
	strcpy(new_thefile,thefile);
	strcat(new_thefile,line2);
    BPPrintMessage(0,odInfo,"Creating image #%d: %s\n",N_image,new_thefile);
	imagePtr = my_fopen(1,new_thefile,"w");
	strcpy(imageFileName,new_thefile);
    getcwd(cwd,sizeof(cwd));
    convert_path(cwd);
    if(strlen(cwd) > 259) BPPrintMessage(0,odError,"=> Warning: this path might be too long: %s\n",cwd);
  //  BPPrintMessage(0,odInfo,"cwd = %s\n",cwd);
    my_sprintf(line1,"%s/CANVAS_header.txt",cwd);
   // BPPrintMessage(0,odInfo,"Reading %s\n",line1);
	thisfile = my_fopen(1,line1,"r");
	if(thisfile == NULL) {
		BPPrintMessage(0,odError,"=> %s is missing!\n",line1);
		my_fclose(imagePtr);
        imagePtr = NULL;
		}
	else {
		someline = (char*) malloc(MAXLIN);
		while(fgets(someline,sizeof(someline),thisfile) != NULL) {
			remove_carriage_returns(someline);
		//	BPPrintMessage(0,odInfo,"someline = %s\n",someline);
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
	char pick_a_line[400];
    char cwd[4096],line[400];
	char *final_name = NULL, *someline = NULL, *anotherline = NULL;
	size_t length;
	ssize_t number;
	int result;
	if(imagePtr == NULL) {
		BPPrintMessage(0,odError,"=> Error closing an image that is already closed\n");
		return ABORT;
		}
	if(imageHits < 1) {
        BPPrintMessage(0,odError, "Closing an empty image: %s\n",imageFileName);
		my_fclose(imagePtr);
		remove(imageFileName);
        return OK;
        }
	result = fflush(imagePtr);
    if(result != 0) {
        BPPrintMessage(0,odError, "=> Error #%d flushing image content\n,result");
		my_fclose(imagePtr);
        return ABORT;
        }
	imageHits = 0;
    getcwd(cwd,sizeof(cwd));
    convert_path(cwd);
    my_sprintf(line,"%s/CANVAS_footer.txt",cwd);
	thisfile = my_fopen(1,line,"r");
	if(thisfile == NULL) {
        BPPrintMessage(0,odInfo,"%s is missing!\n",line);
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
	BPPrintMessage(0,odInfo,"Closing temporary image file\n");
	if(ShowGraphic) {
		final_name = str_replace("_temp","",imageFileName);
		remove_spaces(final_name,final_name);
		if(TraceMIDIinteraction) BPPrintMessage(0,odInfo,"\n");
		BPPrintMessage(0,odInfo,"Finalizing image #%d: %s\n",N_image,final_name);
		imagePtr = my_fopen(1,final_name,"wb");
		thisfile = my_fopen(1,imageFileName,"rb");
		free(final_name);
		while(fgets(pick_a_line,sizeof(pick_a_line),thisfile) != NULL) {
	//		BPPrintMessage(0,odInfo,"pick_a_line: %s",pick_a_line);
			remove_carriage_returns(pick_a_line);
			if(strstr(pick_a_line,"THE_TITLE") != NULLSTR) {
				if(!PlaySelectionOn) someline = str_replace("THE_TITLE",gOptions.inputFilenames[wGrammar],pick_a_line);
				else someline = str_replace("THE_TITLE",gOptions.inputFilenames[wData],pick_a_line);
			//	BPPrintMessage(0,odInfo,"someline: %s",someline);
				fputs(someline,imagePtr);
				fflush(imagePtr);
				free(someline);
				}
	      	else if(strstr(pick_a_line,"THE_WIDTH") != NULLSTR) {
				my_sprintf(line,"%ld",WidthMax);
				someline = str_replace("THE_WIDTH",line,pick_a_line);
			//	BPPrintMessage(0,odInfo,"someline: %s",someline);
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
	        else {
				if(strlen(pick_a_line ) == 0) {
					BPPrintMessage(0,odError,"=> Err trying to write empty line to: %s\n",imageFileName);
					return ABORT;
					}
				if(imagePtr == NULL) {
					BPPrintMessage(0,odError,"=> Err trying to write to empty pointer in: %s\n",imageFileName);
					return ABORT;
					}
				fputs(pick_a_line,imagePtr);
				}
			}
		my_fclose(thisfile);
	/*	result = chmod(imageFileName,0777);
	    if(result != 0) {
			BPPrintMessage(0,odError,"=> Err chmod() after closing %s\n",imageFileName);
			} */
		my_fclose(imagePtr);
	/*	result = chmod(final_name,0777);
	    if(result != 0) {
			BPPrintMessage(0,odError,"=> Err chmod() after closing %s\n",final_name);
			} */
		}
	remove(imageFileName);
	imagePtr = NULL;
	BPPrintMessage(0,odInfo,"Removed %s\n",imageFileName);
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
	BPPrintMessage(0,odInfo, "Bol Processor console app\n");
	BPPrintMessage(0,odInfo, "%s\n", IDSTRING);
	return;
}

void PrintShortVersion(void)
{
	BPPrintMessage(0,odInfo, "%s\n", SHORT_VERSION);
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
	"  -to fname        load tonality file 'fname'\n"
	"  -al fname        load alphabet file 'fname'\n"
	"  -se fname        load settings file 'fname'\n"
	"  -so fname        load sound-object prototypes file 'fname'\n"
	"  -cs fname        load Csound instrument definitions file 'fname'\n"
	"\n"
	"  These file-type markers currently are recognized but ignored:\n"
	"      -in  -kb  -md  -mi  -tb  -tr  -wg  +sc \n"
	"\n"
/*	"  -de fname        load decisions file 'fname'\n"
	"  -in fname        load interaction file 'fname'\n"
	"  -kb fname        load keyboard file 'fname'\n"
	"  -md fname        load MIDI driver settings file 'fname'\n"
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
	BPPrintMessage(0,odInfo, "\nUsage:  %s action [options] { [file-type] inputfile }+\n\n", programName);
	BPPrintMessage(0,odInfo, gOptionList);
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
	
	if(argc == 1) {
		PrintUsage(args[0]);
		BPPrintMessage(0,odError,"=> Not enough arguments...\n\n");
		return ABORT;
	}
	else if(argc < 1) {
		// can this ever happen?
		BPPrintMessage(0,odError, "=> Error in ParsePreInitArgs(): argc is %d\n", argc);
		return ABORT;
	}
	
	while(argn < argc) {
		if(strcmp(args[argn], "-h") == 0 || strcmp(args[argn], "--help") == 0)	{
			PrintUsage(args[0]);
			return EXIT;
		}
		if(strcmp(args[argn], "--version") == 0)	{
			PrintVersion();
			return EXIT;
		}
		if(strcmp(args[argn], "--short-version") == 0)	{
			PrintShortVersion();
			return EXIT;
		}
		
		// need to parse these args before Init() so that init messages are redirected
		if(strcmp(args[argn], "-e") == 0 || strcmp(args[argn], "--use-stderr") == 0)	{
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
		if(args[argn][0] == '-' || args[argn][0] == '+') {
			// check for matching file prefix
			arglen = strlen(args[argn]);
			// FIXME: Need to ignore any path components before the filename when matching prefix
			for (w = 0; w < WMAX; w++) {
				// This comparison assumes all prefixes are 3 chars long (not including the '.')
				if(strncmp(args[argn], FilePrefix[w], 3) == 0) {
					if(arglen == 3) {
						// argument is just the file prefix (eg. "-gr"),
						// so look at the next argument for the file name
						if(++argn < argc)  {
							opts->inputFilenames[w] = args[argn];
							argDone = TRUE;
						}
						else {
							BPPrintMessage(0,odError, "\n=> Missing filename after %s\n\n", args[argn-1]);
							return ABORT;
						}
					}
					else if(arglen > 4 && args[argn][3] == '.') {
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
		else if((w = FindMatchingFileNameExtension(args[argn])) != wUnknown) {
			opts->inputFilenames[w] = args[argn];
			argDone = TRUE;
			}
		
		if(!argDone) {
			/* check if it is an option */
			if(args[argn][0] == '-') {
				if(strcmp(args[argn], "-d") == 0 || strcmp(args[argn], "-D") == 0 || strcmp(args[argn], "--display") == 0)	{ // Modified 2024-06-29
					opts->displayItems = TRUE;
					opts->outOptsChanged = FALSE;
					}
		//		else if(strcmp(args[argn], "-d") == 0 || strcmp(args[argn], "--no-display") == 0)	{
				else if(strcmp(args[argn], "--no-display") == 0)	{
					opts->displayItems = FALSE;
					opts->outOptsChanged = FALSE;
					}
				else  if(strcmp(args[argn], "-o") == 0)	{
					// look at the next argument for the output file name
					if(++argn < argc)  {
						opts->displayItems = TRUE;
						opts->outputFiles[ofiProdItems].name = args[argn];
						opts->outOptsChanged = TRUE;
						OutBPdata = TRUE;
						}
					else {
						BPPrintMessage(0,odError, "\n=> Missing filename after %s\n\n", args[argn-1]);
						return ABORT;
						}
					}
				else if(strcmp(args[argn], "-e") == 0 || strcmp(args[argn], "--use-stderr") == 0)	{
					// need to parse these args a second time to avoid "unknown option" error
					}
				else if(strcmp(args[argn], "--traceout") == 0)	{
					// look at the next argument for the output file name
					if(++argn < argc)  {
						opts->outputFiles[ofiTraceFile].name = args[argn];
						}
					else {
						BPPrintMessage(0,odError, "\n=> Missing filename after %s\n\n", args[argn-1]);
						return ABORT;
						}
					}
				else if(strcmp(args[argn], "--csoundout") == 0)	{
					// look at the next argument for the output file name
					if(++argn < argc)  {
						opts->writeCsoundScore = TRUE;
						opts->outputFiles[ofiCsScore].name = args[argn];
						opts->outOptsChanged = TRUE;
						}
					else {
						BPPrintMessage(0,odError, "\n=> Missing filename after %s\n\n", args[argn-1]);
						return ABORT;
						}
					}
				else if(strcmp(args[argn], "--midiout") == 0)	{
					// look at the next argument for the output file name
					if(++argn < argc)  {
						opts->writeMidiFile = TRUE;
						opts->outputFiles[ofiMidiFile].name = args[argn];
						opts->outOptsChanged = TRUE;
						}
					else {
						BPPrintMessage(0,odError, "\n=> Missing filename after %s\n\n", args[argn-1]);
						return ABORT;
						}
					}
				else if(strcmp(args[argn], "--rtmidi") == 0)	{
					opts->useRealtimeMidi = TRUE;
					opts->outOptsChanged = TRUE;
					}
				else if(strcmp(args[argn], "--english") == 0)	{
					opts->noteConvention = ENGLISH;
					}
				else if(strcmp(args[argn], "--french") == 0)	{
					opts->noteConvention = FRENCH;
					}
				else if(strcmp(args[argn], "--indian") == 0)	{
					opts->noteConvention = INDIAN;
					}
				else if(strcmp(args[argn], "--keys") == 0)	{
					opts->noteConvention = KEYS;
					}
				else if(strcmp(args[argn], "--midiformat") == 0)	{
					// look at the next argument for an integer
					if(++argn < argc && isInteger(args[argn]))  {
						opts->midiFileFormat = (int) atol(args[argn]);
						if(opts->midiFileFormat < 0 || opts->midiFileFormat > 2) {
							BPPrintMessage(0,odError, "\n=> midiformat must be 0, 1, or 2\n\n");
							return ABORT;
						}
					}
					else {
						BPPrintMessage(0,odError, "\n=> Missing number after --midiformat\n\n");
						return ABORT;
					}
				}
				else if(strcmp(args[argn], "--seed") == 0)	{
					// look at the next argument for an integer seed
					if(++argn < argc && isInteger(args[argn]))  {
						opts->seed = (unsigned int) atol(args[argn]);
						opts->seedProvided = TRUE;
					}
					else {
						BPPrintMessage(0,odError, "\n=> Missing number after --seed\n\n");
						return ABORT;
					}
				}
				else if(strcmp(args[argn], "--show-production") == 0)	{
					opts->showProduction = TRUE;
				}
				else if(strcmp(args[argn], "--trace-production") == 0)	{
					opts->traceProduction = TRUE;
				}
				/* else if(strcmp(args[argn], "--step-production") == 0)	{
					DisplayProduce = TRUE;
					StepProduce = TRUE;
				}
				else if(strcmp(args[argn], "--step-subgrammars") == 0)	{
					DisplayProduce = TRUE;
					StepGrammars = TRUE;
				}
				else if(strcmp(args[argn], "--choose-rules") == 0)	{
					PlanProduce = TRUE;
					DisplayProduce = TRUE;
					StepProduce = TRUE;
					TraceProduce = TRUE;
				} */
				else {
					BPPrintMessage(0,odError, "\n=> Unknown option '%s'\n", args[argn]);
					BPPrintMessage(0,odError, "Use '%s --help' to see help information.\n\n", args[0]);
					return ABORT;
					}
				}
			else {
				/* check if it is an action */
				// FIXME? help says that actions are case-insensitive
				if(strcmp(args[argn], "compile") == 0)	{
					action = compile;
					}
				else if(strcmp(args[argn], "produce") == 0)	{
					action = produce;
					}
				else if(strcmp(args[argn], "produce-items") == 0)	{
					action = produce_items;
					// FIXME: look for the item number in next arg
					}
				else if(strcmp(args[argn], "produce-all") == 0)	{
					action = produce_all;
					}
				else if(strcmp(args[argn], "play") == 0)	{
					action = play;
					}
				else if(strcmp(args[argn], "play-item") == 0)	{
					action = play_item;
					// FIXME: look for the item number in next arg
					}
				else if(strcmp(args[argn], "play-all") == 0)	{
					action = play_all;
					}
				else if(strcmp(args[argn], "analyze-item") == 0)	{
					action = analyze;
					// FIXME: look for the item number in next arg
				}
				else if(strcmp(args[argn], "expand") == 0)	{
					action = expand;
				}
				else if(strcmp(args[argn], "show-beats") == 0)	{
					action = show_beats;
					// FIXME: look for the item number in next arg
					}
				else if(strcmp(args[argn], "templates") == 0)	{
					action = templates;
					}
				else {
					BPPrintMessage(0,odError, "\n=> Unknown action '%s'\n", args[argn]);
					BPPrintMessage(0,odError, "If '%s' is an input file, indicate the file type (eg. -gr %s).\n", args[argn], args[argn]);
					BPPrintMessage(0,odError, "Use '%s --help' to see help information.\n\n", args[0]);
					return ABORT;
					}
				
				// more than one action is not allowed
				if(action != no_action && opts->action != no_action)	{
					BPPrintMessage(0,odError, "\n=> Only one action is allowed but two were given: '%s' & '%s'\n\n",ActionTypeToStr(opts->action), ActionTypeToStr(action));
					return ABORT;
					}
				
				opts->action = action;
				}
			}
		++argn;
		}
	
	if(opts->action == no_action)	{
		BPPrintMessage(0,odError, "\n=> Missing required action command in arguments.\n");
		BPPrintMessage(0,odError, "Use '%s --help' to see help information.\n\n", args[0]);
		return ABORT;
		}
	if(opts->useRealtimeMidi == TRUE) {
	// BPPrintMessage(0,odInfo,"opts->outputFiles[ofiTraceFile].name = %s\n",opts->outputFiles[ofiTraceFile].name);
		if(opts->outputFiles[ofiTraceFile].name == NULL) {
			BPPrintMessage(0,odError, "=> The path to the trace file is missing\n");
			return ABORT;
			}
		if(strlen(opts->outputFiles[ofiTraceFile].name) == 0) {
			BPPrintMessage(0,odError, "=> The path to the trace file is missing\n");
			return ABORT;
			}
		thepath = str_replace(".txt","",opts->outputFiles[ofiTraceFile].name);
		if(thepath == NULL) {
			BPPrintMessage(0,odError, "=> Error ParsePostInitArgs(). thepath == NULL\n,result");
			return ABORT;
			}
		new_thepath = malloc(strlen(thepath) + strlen("_midiport") + 1);
		if(new_thepath == NULL) {
			BPPrintMessage(0,odError, "=> Error ParsePostInitArgs(). new_thepath == NULL\n,result");
			return ABORT;
			}
		strcpy(new_thepath,thepath);
		free(thepath);
		strcat(new_thepath,"_midiport");
		strcpy(Midiportfilename,new_thepath);
     //   BPPrintMessage(0,odInfo,"Midiportfilename = %s\n",Midiportfilename);
		rtMIDI = TRUE;
		resultinit = initializeMIDISystem();
		if(new_thepath != NULL) free(new_thepath);
		if(resultinit != OK) {
			Panic = 1;
			rtMIDI = FALSE;
			return ABORT;
			}
		if(rtMIDI) {
			if((r = MIDIflush(0)) != OK) return r;
			Notify("Real-time MIDI started",0);
			}
		else Notify("Real-time MIDI failed to start",1);
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
	if(opts->outOptsChanged) {
		OutCsound = WriteMIDIfile = rtMIDI = FALSE;
	}
	
	// apply options that were explicitly given on the command line
	if(opts->displayItems != NOCHANGE)		DisplayItems = opts->displayItems;
	if(opts->writeCsoundScore != NOCHANGE)	OutCsound = opts->writeCsoundScore;
	if(opts->writeMidiFile != NOCHANGE)	WriteMIDIfile = opts->writeMidiFile;
	if(opts->useRealtimeMidi != NOCHANGE)	rtMIDI = opts->useRealtimeMidi;
	if(opts->traceProduction != NOCHANGE)	{
		DisplayProduce = opts->traceProduction;
		TraceProduce = opts->traceProduction;
	}
	// showProduction could be enabled after traceProduction is disabled
	if(opts->showProduction != NOCHANGE)	DisplayProduce = opts->showProduction;
	if(opts->noteConvention != NOCHANGE)	NoteConvention = opts->noteConvention;
	if(opts->midiFileFormat != NOCHANGE)	MIDIfileType = opts->midiFileFormat;
	if(opts->seedProvided)	{
		Seed = opts->seed;
		if(Seed > 0) {
			BPPrintMessage(0,odInfo, "Random seed = %u as per command line\n", Seed);
			ResetRandom();
			}
		else {
			BPPrintMessage(0,odInfo, "Not using a random seed: shuffling the cards\n");
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
			  BPPrintMessage(0,odError, "=> Err. ActionTypeToStr(): action == %d\n", action);
	}
	
	return "";
}


void PrintInputFilenames(BPConsoleOpts* opts)
{
	int w;
	for (w = 0; w < WMAX; w++) {
		if(opts->inputFilenames[w] != NULL)
			BPPrintMessage(0,odInfo, "opts->inputFilenames[%s] = %s\n", WindowName[w], opts->inputFilenames[w]);
	}
	return;
}


void GetFileName(char* name,const char* path) { // Added by BB 4 Nov 2020
	char c;
	int i,j;
//	BPPrintMessage(0,odInfo, "\npath = %s\n",path);
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
	WMAX file/path names with file types that match the window indices in -BP3.h.
	
	Returns MISSED if an error occured or OK if successful. */
int LoadInputFiles(const char* pathnames[WMAX]) {
	int w, result;
	for(w = 0; w < WMAX; w++) {
		// The order of reading these files is important. For instance, iObjects should occur after wCsoundResources
		if(pathnames[w] != NULL) {
		//	BPPrintMessage(0,odInfo,"pathnames[%d] = %s\n",w,pathnames[w]);
			switch(w) {
				case wGrammar:
				case wAlphabet:
				case wStartString:
				case wData:
				case wGlossary:
					BPPrintMessage(0,odInfo, "Reading %s file: %s\n", DocumentTypeName[w], pathnames[w]);
					result = LoadFileToTextHandle(pathnames[w], TEH[w]);
					if(result != OK)  {
						BPPrintMessage(0,odError,"=> You first need to save the Grammar or Data\n");
						return result;
						}
					switch(w) {
						case wAlphabet:			LoadedAlphabet = TRUE; break;
						case wStartString:		LoadedStartString = TRUE; break;
						case wGlossary:			LoadedGl = TRUE; break;
						}
					break;
				case wCsoundResources:
					strcpy(FileName[wCsoundResources],pathnames[w]);
					result = LoadCsoundInstruments(0,1);
					if(result != OK)  return result;
					break;
				case wTonality:
					strcpy(FileName[wTonality],pathnames[w]);
					result = LoadTonality();
					if(result != OK)  return result;
					break;
				case iSettings:
					BPPrintMessage(0,odInfo, "Reading settings file: %s\n", pathnames[w]);
					result = LoadSettings(pathnames[w], FALSE);
					if(result != OK)  return result;
					break;
				case iObjects:
					BPPrintMessage(0,odInfo, "Reading sound-object prototypes file: %s\n", pathnames[w]);
					strcpy(FileName[iObjects],pathnames[w]);
					result = LoadObjectPrototypes(0,1);
					if(result != OK)  return result;
					break;
				default:
					BPPrintMessage(0,odWarning, "Ignoring %.3s %s (%s files are currently unsupported)\n", FilePrefix[w], pathnames[w], DocumentTypeName[w]);
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
	if(check_memory_use) BPPrintMessage(0,odInfo,"MemoryUsed start LoadFileToTextHandle = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	if(pathname == NULL) {
		  BPPrintMessage(0,odError, "=> Err. LoadFileToTextHandle(): pathname == NULL\n");
		return ABORT;
		}
	if(th == NULL) {
		  BPPrintMessage(0,odError, "=> Err. LoadFileToTextHandle(): th == NULL\n");
		return ABORT;
		}
	result = OpenAndReadFile(pathname,&filecontents);
//	BPPrintMessage(0,odInfo,"%s\n",filecontents);
	if(result != OK)  return result;
	result = CopyStringToTextHandle(th,filecontents);
	if(check_memory_use) BPPrintMessage(0,odInfo,"MemoryUsed before end LoadFileToTextHandle = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	free(filecontents);
	filecontents = NULL;
	if(check_memory_use) BPPrintMessage(0,odInfo,"MemoryUsed end LoadFileToTextHandle = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
	return result;	
	}


int OpenAndReadFile(const char* pathname,char** buffer) { // Rewritten 2024-06-25
	FILE *fin;
	int result;
    char line[MAXLIN];
	long pos;
	
	if(pathname == NULL) {
		  BPPrintMessage(0,odError, "=> Err. LoadFileToTextHandle(): pathname == NULL\n");
		return MISSED;
		}
	if(pathname[0] == '\0') {
		  BPPrintMessage(0,odError, "=> Err. LoadFileToTextHandle(): pathname is empty\n");
		return MISSED;
		}
	fin = my_fopen(1,pathname, "r");
	if(!fin) {
		BPPrintMessage(0,odError, "=> Could not open file for reading: %s\n", pathname);
		return MISSED;
		}
//	BPPrintMessage(0,odInfo, "Opened file: %s\n", pathname);

	*buffer = NULL;
    size_t bufferLength = 0;
    int i = 0;
	pos = ZERO;;
    while(fgets(line, sizeof(line),fin) != NULL) {
		remove_carriage_returns(line);
        if(strlen(line) == 0) continue;
        i++;
  //      BPPrintMessage(0,odInfo, "line[%d] (%d) = %s\n",i,utf8_strsize(line),line);
		size_t lineSize = utf8_strsize(line);
        char* newBuffer = realloc(*buffer, bufferLength + lineSize + 2); // +1 for '\n' and +1 for '\0'
		if(newBuffer == NULL) {
			// Handle allocation failure; clean up and exit
			BPPrintMessage(0,odError, "=> Failed to allocate memory for buffer\n");
			return MISSED;
			}
		*buffer =  newBuffer;
        memcpy(*buffer + bufferLength, line, lineSize);
        bufferLength += lineSize;
		}
	if(bufferLength > 0)
		(*buffer)[bufferLength] = '\0'; // Null-terminate the block
	else *buffer = strdup(""); // Handle empty file case
//	BPPrintMessage(0,odInfo,"OK reading\n");
	my_fclose(fin);
	return OK;
	}


FILE* my_fopen(int check, const char* path, const char* mode) {
	char thismode[3];
	if(strcmp(mode,"r") == 0) strcpy(thismode,"rb");
	else strcpy(thismode,mode);
    char convertedPath[4096];  // Buffer to store the converted path
    FILE* file;
	if(path == NULL) {
        BPPrintMessage(0,odError, "=> Err. in my_fopen(). path == NULL\n");
        return NULL;
		}
    strcpy(convertedPath,path);  // Copy the original path to the buffer
    convert_path(convertedPath);  // Change backslashes to normal
	struct stat file_stat;
	file = fopen(convertedPath,thismode);
    if(file == NULL) {
		if(check) BPPrintMessage(0,odError, "=> Failed to open: %s in '%s' mode. Error: %s\n",
                   convertedPath, thismode, strerror(errno));
		}
	else if(strcmp(mode,"w") == 0 || strcmp(mode,"wb") == 0) {
        if(stat(convertedPath, &file_stat) == 0) {
            if((file_stat.st_mode & 0777) != 0777) {
				int result = chmod(convertedPath,0777);
				if(result != 0)
					BPPrintMessage(0,odError,"=> Error chmod 777 after opening %s. %s\n",convertedPath, strerror(errno));
				}
			}
		}
    return file;  // Return the file pointer 
    }

int my_fclose(FILE *file) {
	int result;
    if(file == NULL) {
   //     BPPrintMessage(0,odError, "=> Attempt to close a NULL file pointer\n");
        return OK;
        }
	result = fflush(file);
    if(result != 0) {
     //   BPPrintMessage(0,odError, "=> Error #%d flushing a file\n, result");
		fclose(file);
		file = NULL;
        return ABORT;
        }
	result = fclose(file);
    if(result != 0 && !Panic) {
     //   BPPrintMessage(0,odError, "=> Error #%d closing a file\n, result");
        return ABORT;
        }
	file = NULL;
    return OK;
    }


FILE* OpenOutputFile(OutFileInfo* finfo, const char* mode) {
	// Opens the file controlled by an OutFileInfo struct.
	finfo->fout = my_fopen(1,finfo->name, mode);
	if(finfo->fout != NULL)  finfo->isOpen = TRUE;
	return finfo->fout;
	}


void CloseOutputFile(OutFileInfo* finfo) {
	// Closes the file controlled by an OutFileInfo struct.
	int result;
	if(finfo->isOpen && finfo->fout != NULL) {
		result = my_fclose(finfo->fout);
		finfo->fout = NULL;
		finfo->isOpen = FALSE;
		}
/*	result = chmod(finfo->name,0777);
	if(result != 0) {
		BPPrintMessage(0,odError,"=> Err chmod() after closing %s\n",finfo->name);
		return;
		} */
	return;
	}

void CloseOutputDestination(int dest, BPConsoleOpts* opts, outfileidx_t fileidx) {
	// Cleans up an output destination and closes any file associated with it.
	if(opts->outputFiles[fileidx].isOpen)	{
		BPPrintMessage(0,odInfo, "Closing file: %s\n", opts->outputFiles[fileidx].name);
		CloseOutputFile(&(opts->outputFiles[fileidx]));
		SetOutputDestinations(dest, NULL);
		}
	return;
	}

int PrepareProdItemsDestination(BPConsoleOpts* opts) {
//	FILE *fout;
	// prepare output file if requested
	if(opts->displayItems && opts->outputFiles[ofiProdItems].name != NULL)	{
		BPPrintMessage(0,odInfo, "Opening output file %s\n", opts->outputFiles[ofiProdItems].name);
		strcpy(OutFileName,opts->outputFiles[ofiProdItems].name);
		outPtr = OpenOutputFile(&(opts->outputFiles[ofiProdItems]),"w");
		if(!outPtr) {
			BPPrintMessage(0,odError, "=> Could not open file for output %s\n", opts->outputFiles[ofiProdItems].name);
			return MISSED;
		    }
		SetOutputDestinations(odDisplay,outPtr);	
	    }
	return OK;
    }

int PrepareTraceDestination(BPConsoleOpts* opts) {
	FILE *fout;
	// prepare trace output file if requested
	if(opts->outputFiles[ofiTraceFile].name != NULL) {
		fout = OpenOutputFile(&(opts->outputFiles[ofiTraceFile]), "w");
		if(!fout) {
			BPPrintMessage(0,odError, "=> Could not create trace file %s\n", opts->outputFiles[ofiTraceFile].name);
			return MISSED;
		    }
		SetOutputDestinations(odTrace,fout);
        BPPrintMessage(0,odInfo, "Creating trace file: %s\n", opts->outputFiles[ofiTraceFile].name);
	    }
    return OK;
    }

/* Utility functions */

/* Returns TRUE if string s is an integer, otherwise FALSE. */
int isInteger(const char* s) {
	int i = 0;
	if(s[i] != '-' && s[i] != '+' && !isdigit(s[i])) return FALSE;
	while(s[++i] != '\0') {
		if(!isdigit(s[i])) return FALSE;
    	}
	return TRUE;
    }

void StopWaiting(int key,char ch) {
	unsigned long time_now,thisscripttime;
	int j;
	if(FirstNoteOn) {
		if(TraceMIDIinteraction) BPPrintMessage(0,odInfo,"time_now = 0L in HandleInputEvent()\n");
		time_now = 0L;
		}
	else time_now = getClockTime() - initTime; // microseconds
	if(TraceMIDIinteraction) BPPrintMessage(0,odInfo,"Received MIDI event %d date %ld ms, checking %d script(s)\n",key,time_now / 1000L,Jinscript);
	for(j=1; j <= Jinscript; j++) {
		if(((*p_INscript)[j]).chan == -1) { // This is a deactivated instruction
	//		if(j == Jinscript) Jinscript = 0; // No need to try this later
			continue;
			}
		if(TraceMIDIinteraction && key > 0) BPPrintMessage(0,odInfo,"[%d] Instruction %d time_now = %ld ms, waiting for MIDI event %d, this script date = %ld ms\n",j,((*p_INscript)[Jinscript]).scriptline, time_now / 1000L, ((*p_INscript)[j]).key, (((*p_INscript)[j]).time + TimeStopped)/1000L);
		if(((*p_INscript)[j]).key != key) continue;
		if(key == 0 && (((*p_INscript)[j]).scriptline != 46 || ch != ' ')) continue;  // Wait for Space
		if(key == Start && ((*p_INscript)[j]).scriptline != 67) continue;
		if(key == Continue && ((*p_INscript)[j]).scriptline != 66) continue;
		if(key == Stop && ((*p_INscript)[j]).scriptline != 128) continue;
		thisscripttime = ((*p_INscript)[j]).time + TimeStopped;
		if(key == ((*p_INscript)[j]).key && time_now >= thisscripttime) {
			if(TraceMIDIinteraction) BPPrintMessage(0,odInfo,"[%d] Good real-time message %d, time_now = %ld ms\n",j,key,time_now/1000L);
			StopPlay = FALSE;
			TimeStopped +=  1000 * MIDIsyncDelay; // Necessary to restore the timing of the next events
			((*p_INscript)[j]).chan = -1; // This input script is now deactivated
			return;
			}
		else continue;
		}
	return;
	}

#if !defined(_WIN64)
struct termios orig_termios;
void disable_raw_mode() {
    // Restore the original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
	}

void enable_raw_mode() {
    struct termios raw;
    // Get the current terminal settings
    tcgetattr(STDIN_FILENO, &orig_termios);
    // Register the disable_raw_mode to run when the program exits
    atexit(disable_raw_mode);
    // Start with the original settings
    raw = orig_termios;
    // Input modes: no break, no CR to NL, no parity check, no strip char, no start/stop output control
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // Output modes: disable post-processing
    raw.c_oflag &= ~(OPOST);
    // Control modes: set 8-bit chars
    raw.c_cflag |= (CS8);
    // Local modes: echoing off, canonical mode off, no signal chars, no extended functions
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    // Control characters: set the read timeout to return immediately
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    // Apply the new terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
	}
#endif
