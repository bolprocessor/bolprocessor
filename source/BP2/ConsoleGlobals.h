/*  ConsoleGlobals.h (BP2 version CVS) */

/*  This file is a part of Bol Processor 2
    Copyright (c) 1990-2000 by Bernard Bel, Jim Kippen and Srikumar K. Subramanian
	Copyright (c) 2020 by Anthony Kozar
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

#ifndef BP_CONSOLEGLOBALS_H
#define BP_CONSOLEGLOBALS_H

// indices to gOptions.outputFiles[]
typedef enum {
	ofiProdItems	= 0,	// output file for produced items (-o option)
	ofiMidiFile		= 1,	// output Std Midi score file (--midiout option)
	ofiCsScore		= 2,	// output Csound score file (--csoundout option)
	ofiTraceFile	= 3,	// output file for tracing processes (no option yet)
}	outfileidx_t;

// actions that can be specified on the command line
typedef enum {
	no_action = 0, compile, produce, produce_items, produce_all, play, play_item,
	play_all, analyze, expand, show_beats, templates
} action_t;

// Values for CLOption type are TRUE, FALSE, and NOCHANGE
typedef int CLOption;
#define NOCHANGE		-1

#define MAXOUTFILES		4

typedef struct OutFileInfo {
	const char	*name;
	FILE		*fout;
	Boolean		isOpen;
} OutFileInfo;

typedef struct BPConsoleOpts {
	action_t	action;
	int			itemNumber;
	const char	*startString;
	const char	*midiInSource;
	const char	*midiOutDestination;
	const char	*inputFilenames[WMAX];
	OutFileInfo	outputFiles[MAXOUTFILES];
	Boolean		useStdErr;
	Boolean		useStartString;
	Boolean		seedProvided;
	Boolean		outOptsChanged;
	CLOption	displayItems;
	CLOption	writeCsoundScore;
	CLOption	writeMidiFile;
	CLOption	useRealtimeMidi;
	CLOption	showProduction;
	CLOption	traceProduction;
	int			noteConvention;		// NOCHANGE is an option too
	int			midiFileFormat;		// NOCHANGE is an option too
	unsigned	seed;
} BPConsoleOpts;

extern BPConsoleOpts gOptions;

FILE* OpenOutputFile(OutFileInfo* finfo, const char* mode);
void CloseOutputFile(OutFileInfo* finfo);

#endif /* BP_CONSOLEGLOBALS_H */
