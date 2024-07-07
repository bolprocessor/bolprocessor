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

#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"
#include "ConsoleMessages.h"

// indices to gOutDestinations[]
typedef enum {	
	odiDisplay	= 0,	// for results of produce items, expand selection, etc.
	odiMidiDump	= 1,	// for printing Midi messages as text
	odiCsScore	= 2,	// for writing Csound score
	odiTrace	= 3,	// for tracing processes (and step-by-step ?)
	odiInfo		= 4,	// informational messages
	odiWarning	= 5,	// warning messages
	odiError	= 6,	// error messages
	odiUserInt	= 7,	// interactive messages to which a response is expected
}	outdestidx_t;

#define	MAXOUTDEST	8

/* private globals */

static FILE*	gOutDestinations[MAXOUTDEST] = 
					{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static bp_message_callback_t	gMessageCallback = NULL;

void ConsoleMessagesInit() {
    gOutDestinations[odiDisplay] = stdout;
    gOutDestinations[odiMidiDump] = stdout;
    gOutDestinations[odiCsScore] = stdout;
    gOutDestinations[odiTrace] = stdout;
    gOutDestinations[odiInfo] = stdout;
    gOutDestinations[odiWarning] = stdout;
    gOutDestinations[odiError] = stdout;
    gOutDestinations[odiUserInt] = stdout;
    }

void SetOutputDestinations(int dest, FILE* file) {
	// set each destination in 'dest' to 'file'
	if (dest & odDisplay)	gOutDestinations[odiDisplay] = file;
	if (dest & odMidiDump)	gOutDestinations[odiMidiDump] = file;
	if (dest & odCsScore)	gOutDestinations[odiCsScore] = file;
	if (dest & odTrace)		gOutDestinations[odiTrace] = file;
	if (dest & odUserInt)	gOutDestinations[odiUserInt] = file;
	if (dest & odError)		gOutDestinations[odiError] = file;
	if (dest & odWarning)	gOutDestinations[odiWarning] = file;
	if (dest & odInfo)		gOutDestinations[odiInfo] = file;
    }


/* Functions for displaying messages and writing output in the console build */

int BPPrintMessage(int dest, const char *format, ...) {
	va_list	args;


    // Handle callback if set
    if (gMessageCallback != NULL) {
        va_start(args, format);
        gMessageCallback(NULL, dest, format, args);
        va_end(args);
    }

    // Handle standard destinations
    if (dest & odDisplay) {
        va_start(args, format);
        vfprintf(gOutDestinations[odiDisplay], format, args);
        va_end(args);
    }
    if (dest & odMidiDump) {
        va_start(args, format);
        vfprintf(gOutDestinations[odiMidiDump], format, args);
        va_end(args);
    }
    if (dest & odCsScore) {
        va_start(args, format);
        vfprintf(gOutDestinations[odiCsScore], format, args);
        va_end(args);
    }
    if (dest & odTrace) {
        va_start(args, format);
        vfprintf(gOutDestinations[odiTrace], format, args);
        va_end(args);
    }
    if (dest & odUserInt) {
        va_start(args, format);
        vfprintf(gOutDestinations[odiUserInt], format, args);
        va_end(args);
    }
    if (dest & odError) {
        va_start(args, format);
        vfprintf(gOutDestinations[odiError], format, args);
        va_end(args);
    }
    if (dest & odWarning && !PlayAllChunks && (!Improvize || ItemNumber < 1)) {
        va_start(args, format);
        vfprintf(gOutDestinations[odiWarning], format, args);
        va_end(args);
    }
    if (dest & odInfo && !PlayAllChunks && (!Improvize || ItemNumber < 1)) {
        va_start(args, format);
        vfprintf(gOutDestinations[odiInfo], format, args);
        va_end(args);
    }


//	else if ((dest & odInfo) && (!Improvize || ItemNumber < 1))		vfprintf(gOutDestinations[odiInfo], format, args); 
//	else if ((dest & odInfo) && !PlayAllChunks)		vfprintf(gOutDestinations[odiInfo], format, args);  2024-05-02
	return OK;
    }

int BPSetMessageCallback(bp_message_callback_t func)
{
	gMessageCallback = func;
	return OK;
}

int ShowMessage(int store, int w, char *message)
{
	if (w < 0 || w >= WMAX || !Editable[w]) {
		if (Beta) Alert1("=> Err. ShowMessage()");
		return(OK);
	}
	
	// save message for recall
	if (store) {
		Jmessage++;
		if(Jmessage >= MAXMESSAGE) Jmessage = 0;
		MystrcpyStringToHandle(&(p_MessageMem[Jmessage]), message);
	}
	
	BPPrintMessage(odInfo, "%s\n", message);
	return OK;
}

int ClearMessage(void)
{
	return OK;
}

int FlashInfo(char* message)
{
	BPPrintMessage(odInfo, "%s\n", message);
	return OK;
}

int Alert1(char message[])
{
	// Alerts could be warnings or errors ...
	BPPrintMessage(odError, "=> %s\n", message);
	return OK;
}

// Ask a question with a yes/no response
int Answer(char *what, char c)
{
	int reply;
	char line[MAXLIN];
		
	// for now, always return the default answer 
	if (TRUE || ScriptExecOn || (AEventOn && !OkWait)) {
		if (c == 'Y') reply = YES;
		else reply = NO;
		if (strlen(what) < MAXLIN - 5) 
			my_sprintf(line,"%s ? %c", what, c);
		else fprintf(stderr, "=> Err. Answer(): 'what' is too long: %s\n", what);
		if (ScriptExecOn) {
			if (wTrace != OutputWindow) PrintBehind(wTrace,line);
		}
		else ShowMessage(TRUE,wMessage,line);
		return reply;
	}
	// else get answer from user or host application callback
}

// Ask a question with a text response
// Answer is returned in value[], which must be able to hold 256 chars
int AnswerWith(char message[], char defaultvalue[], char value[])
{
	// for now, always return the default answer 
	strncpy(value, defaultvalue, 255);
	value[255] = '\0';
	BPPrintMessage(odUserInt,"%s ? %s\n", message, value);
	return OK;
}
