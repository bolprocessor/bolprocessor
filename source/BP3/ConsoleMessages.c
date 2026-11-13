/*  ConsoleMessages.c (BP3) */

/*  This file is a part of Bol Processor
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

#include <stdarg.h>

#ifndef _H_BP3
#include "-BP3.h"
#endif

#include "-BP3decl.h"

typedef enum {	
	odiDisplay	= 0,	// for results of produce items, expand selection, etc.
	odiMidiDump	= 1,	// for printing Midi messages as text
	odiCsScore	= 2,	// for writing Csound score
	odiTrace	= 3,	// for tracing processes (and step-by-step ?)
	odiInfo		= 4,	// informational messages
	odiWarning	= 5,	// warning messages
	odiError	= 6,	// error messages
	odiUserInt	= 7,	// interactive messages to which a response is expected
	odiWeights	= 8,	// interactive messages to which a response is expected
	odiMIDIfile	= 9,	// MIDI file: only used to count its size
	odiEventList	= 10,	// event list file: only used to count its size
	}	outdestidx_t;

#define	MAXOUTDEST	15

/* private globals */

static FILE*	gOutDestinations[MAXOUTDEST] = 
					{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static bp_message_callback_t	gMessageCallback = NULL;

// 2026-11-13 Size of each output file, to stop the work before it fills the disk
static long long	gBytesWritten[MAXOUTDEST] = { 0 };
static int			gSizeExceeded[MAXOUTDEST] = { 0 };
static int			gForceMessage = FALSE; // TRUE to display a message beyond MAXMESSAGES

void ConsoleMessagesInit() {
#ifdef __BP3_WASM__
	FILE* dest = NULL;
#else
	FILE* dest = stdout;
#endif
	gOutDestinations[odiDisplay] = dest;
	gOutDestinations[odiMidiDump] = dest;
	gOutDestinations[odiCsScore] = dest;
	gOutDestinations[odiTrace] = dest;
	gOutDestinations[odiInfo] = dest;
	gOutDestinations[odiWarning] = dest;
	gOutDestinations[odiError] = dest;
	gOutDestinations[odiUserInt] = dest;
	gOutDestinations[odiWeights] = dest;
	}

void SetOutputDestinations(int dest, FILE* file) {
	// set each destination in 'dest' to 'file'
	if(dest & odDisplay)	gOutDestinations[odiDisplay] = file;
	if(dest & odMidiDump)	gOutDestinations[odiMidiDump] = file;
	if(dest & odCsScore)	gOutDestinations[odiCsScore] = file;
	if(dest & odTrace)		gOutDestinations[odiTrace] = file;
	if(dest & odWeights)	gOutDestinations[odiWeights] = file;
	if(dest & odUserInt)	gOutDestinations[odiUserInt] = file;
	if(dest & odError)		gOutDestinations[odiError] = file;
	if(dest & odWarning)	gOutDestinations[odiWarning] = file;
	if(dest & odInfo)		gOutDestinations[odiInfo] = file;
	}

static int OutputSizeIndex(int dest) {
	switch(dest) {
		case odDisplay:		return odiDisplay;
		case odMidiDump:	return odiMidiDump;
		case odCsScore:		return odiCsScore;
		case odTrace:		return odiTrace;
		case odWeights:		return odiWeights;
		case odMIDIfile:	return odiMIDIfile;
		case odEventList:	return odiEventList;
		}
	return -1;
	}

static const char* OutputSizeFileName(int idx) {
	const char* name = NULL;
	switch(idx) {
		case odiDisplay:	name = gOptions.outputFiles[ofiProdItems].name; break;
		case odiCsScore:	name = gOptions.outputFiles[ofiCsScore].name; break;
		case odiTrace:		name = gOptions.outputFiles[ofiTraceFile].name; break;
		case odiWeights:	name = gOptions.outputFiles[ofiWeightsFile].name; break;
		case odiMIDIfile:	name = gOptions.outputFiles[ofiMidiFile].name; break;
		case odiEventList:	name = gOptions.outputFiles[ofiEventListfile].name; break;
		}
	if(name == NULL || name[0] == '\0' || strcmp(name,"-") == 0) name = "standard output";
	return name;
	}

void ResetOutputSize(int dest) {
	// Call it when a new file is created for 'dest' (odCsScore, odMIDIfile, odEventList...). 2026-11-13
	int bit, idx;
	for(bit = 1; bit <= odEventList; bit <<= 1) {
		if(!(dest & bit)) continue;
		idx = OutputSizeIndex(bit);
		if(idx < 0) continue;
		gBytesWritten[idx] = 0;
		gSizeExceeded[idx] = FALSE;
		}
	}

int CheckOutputSize(int dest, size_t numbytes) {
	// 'dest' is a single output file (odCsScore, odMIDIfile, odEventList...).
	// Returns OK and counts 'numbytes' if they can be written without the file exceeding MaxOutputFileSize.
	// Otherwise returns MISSED: nothing more should be written to this file.
	// The first time, it displays a warning and stops the work, like the time limit. 2026-11-13
	int idx;
	idx = OutputSizeIndex(dest);
	if(idx < 0) return OK;
	if(gSizeExceeded[idx]) return MISSED;
	if(gBytesWritten[idx] + (long long) numbytes <= MaxOutputFileSize) {
		gBytesWritten[idx] += (long long) numbytes;
		return OK;
		}
	gSizeExceeded[idx] = TRUE;
	Improvize = PlayAllChunks = FALSE;
	EmergencyExit = TRUE;
	gForceMessage = TRUE;
	BPPrintMessage(0,odError,"\n=> File ‘%s’ has reached the maximum size of %lld MB. Work has been terminated and this file is incomplete.\n➡ Produce fewer or shorter items\n\n",OutputSizeFileName(idx),MaxOutputFileSize / 1000000LL);
	gForceMessage = FALSE;
	return MISSED;
	}

static void PrintToOutputFile(int dest, int idx, const char *format, va_list args) {
	// Writes to a file destination, unless this makes the file exceed MaxOutputFileSize. 2026-11-13
	va_list args2;
	int length;
	if(gOutDestinations[idx] == NULL || gSizeExceeded[idx]) return;
	va_copy(args2, args);
	length = vsnprintf(NULL, 0, format, args2);
	va_end(args2);
	if(length < 0) return;
	if(CheckOutputSize(dest, (size_t) length) != OK) return;
	vfprintf(gOutDestinations[idx], format, args);
	}


// Functions for displaying messages and writing output in the console build

int BPPrintMessage(int force,int dest, const char *format, ...) {
	va_list	args;
	int result, messages;

	// 2026-11-13 MAXMESSAGES only limits messages. Output files (Csound score, data, trace...) are limited by MaxOutputFileSize
	messages = dest & (odInfo | odWarning | odError | odUserInt);
	if(messages && (MAXMESSAGES > 0) && !gForceMessage) {
		if(NumberMessages == MAXMESSAGES) {
			NumberMessages++;
			BPPrintMessage(1,odInfo,"👉 End of displaying messages (max %d)\n",MAXMESSAGES);
			dest &= ~messages;
			}
		else if(NumberMessages > (MAXMESSAGES + 1)) dest &= ~messages; // 2026-03-24
		if(dest == 0) return OK;
		}

	// Handle callback if set
   if(gMessageCallback != NULL) {
	   va_start(args, format);
	   gMessageCallback(NULL, dest, format, args);
	   va_end(args);
	   }

	// Handle standard destinations
	if(dest & odDisplay) {
		va_start(args, format);
		PrintToOutputFile(odDisplay, odiDisplay, format, args);
		va_end(args);
		}
	if(dest & odMidiDump) {
		va_start(args, format);
		PrintToOutputFile(odMidiDump, odiMidiDump, format, args);
		va_end(args);
		}
	if(dest & odCsScore) {
		va_start(args, format);
		PrintToOutputFile(odCsScore, odiCsScore, format, args);
		va_end(args);
		}
	if(dest & odTrace) {
		va_start(args, format);
		PrintToOutputFile(odTrace, odiTrace, format, args);
		va_end(args);
		}
	if(dest & odWeights) {
		va_start(args, format);
		PrintToOutputFile(odWeights, odiWeights, format, args);
		va_end(args);
		}
	if(dest & odUserInt) {
		NumberMessages++; // 2026-11-13
		va_start(args, format);
		vfprintf(gOutDestinations[odiUserInt], format, args);
		va_end(args);
		}
	if(dest & odError) {
		va_start(args, format);
	//    vfprintf(gOutDestinations[odiError], format, args);
#ifndef __BP3_WASM__
		NumberMessages++; // 2026-11-13 Error messages are also limited by MAXMESSAGES
		vfprintf(stdout, format, args);
#endif
		va_end(args);
		}
	if((dest & odWarning) && !PlayAllChunks && (!Improvize || ItemNumber < 1)) {
		NumberMessages++; // 2026-11-13
		va_start(args, format);
		vfprintf(gOutDestinations[odiWarning], format, args);
		va_end(args);
		}
	if((dest & odInfo) && (((!Improvize || ItemNumber < 1) && !HideMessages && !PlayAllChunks) || force)) {
		va_start(args, format);
   //     fprintf(stderr, "ok %s\n",format);
   //     vfprintf(gOutDestinations[odiInfo], format, args);
#ifndef __BP3_WASM__
		NumberMessages++; // 2026-03-24
		result = vfprintf(stdout, format, args);
		if(result < 0) fprintf(stderr, "Error occurred during writing to stdout.\n");
#endif
		va_end(args);
		}
	return OK;
	}

// The following are obsolete in BP3

int BPSetMessageCallback(bp_message_callback_t func) {
	gMessageCallback = func;
	return OK;
	}

int ShowMessage(int store, int w, char *message) {
	// This is obsolete. Generally replaced by BPPrint()
	if(w < 0 || w >= WMAX || !Editable[w]) {
		BPPrintMessage(0,odError,"=> Err. ShowMessage()");
		return(OK);
		}
	// Save message for recall (currently not used)
	if(store) {
		Jmessage++;
		if(Jmessage >= MAXMESSAGE) Jmessage = 0;
		MystrcpyStringToHandle(&(p_MessageMem[Jmessage]), message);
		}
    
	BPPrintMessage(0,odInfo, "%s\n", message);
	return OK;
	}

int ClearMessage(void) {
	return OK;
	}

int FlashInfo(char* message) {
	BPPrintMessage(0,odInfo,"%s\n",message);
	return OK;
	}

int Alert1(char message[]) {
	// Alerts could be warnings or errors ...
	BPPrintMessage(0,odError, "=> %s\n", message);
	return OK;
	}

// Ask a question with a yes/no response
int Answer(char *what, char c) {
	int reply;
	char line[MAXLIN];
		
	// for now, always return the default answer 
	if(TRUE || ScriptExecOn || (AEventOn && !OkWait)) {
		if(c == 'Y') reply = YES;
		else reply = NO;
		if(strlen(what) < MAXLIN - 5) 
			my_sprintf(line,"%s ? %c", what, c);
		else fprintf(stderr, "=> Err. Answer(): 'what' is too long: %s\n", what);
		if(ScriptExecOn) {
			if(wTrace != OutputWindow) PrintBehind(wTrace,line);
			}
		else ShowMessage(TRUE,wMessage,line);
		return reply;
		}
	// else get answer from user or host application callback
	}

// Ask a question with a text response
// Answer is returned in value[], which must be able to hold 256 chars
int AnswerWith(char message[], char defaultvalue[], char value[]) {
	// for now, always return the default answer 
	strncpy(value, defaultvalue, 255);
	value[255] = '\0';
	BPPrintMessage(0,odUserInt,"%s ? %s\n", message, value);
	return OK;
	}
