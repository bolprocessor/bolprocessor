/* ConsoleStubs.c (BP2 version CVS) */
/* August 11, 2013 */

/*  This file is a part of Bol Processor 2
Copyright (c) 1990-2000 by Bernard Bel, Jim Kippen and Srikumar K. Subramanian
Copyright (c) 2013 by Anthony Kozar
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

#include <string.h>

#include "-BP2.h"
#include "-BP2decl.h"
#include "ConsoleMessages.h"

/* Stubs to replace missing functions from BP2 Carbon GUI and from Mac OS X Carbon libraries */

/* These are BP2 functions -- some of these may end up utilizing callbacks
   set by host application using the client API ... */

int PleaseWait(void)
{
	return(OK);
}

int StopWait(void)
{
	return(OK);
}

void SetDefaultCursor(void)
{	
	return;
}

int BPActivateWindow(int mode,int newNw)
{
	BP_NOT_USED(mode);
	BP_NOT_USED(newNw);
	return OK;
}

int AppendScript(int command)
{
	BP_NOT_USED(command);
	return OK;
}

int StartCount(void)
{
	return OK;
}

int StopCount(int i)
{
	BP_NOT_USED(i);
	return OK;
}

int CheckLoadedPrototypes(void)
{
	if (NeedAlphabet && !ObjectMode && !ObjectTry && (OutMIDI || OutCsound || WriteMIDIfile)) {
		ObjectTry = TRUE;
		BPPrintMessage(odWarning, "Loading object prototypes is not yet possible in the "
			"console version, so MIDI and Csound output may not work correctly.");
		return FAILED;
	}
	return OK; // ??
}

extern Boolean LoadedAlphabet;

int LoadAlphabet(int w, FSSpec *p_spec)
{
	BP_NOT_USED(p_spec);

	// If an alphabet was loaded due to a command-line argument, or none is referenced
	// in "window" w, the data file, or the grammar file, then everything's fine.
	if (LoadedAlphabet) return OK;
	if (w == -1 && GetAlphaName(wData) != OK && GetAlphaName(wGrammar) != OK)
		return OK;
	else if (GetAlphaName(w) != OK) return OK;
	
	// Otherwise, if a -ho reference was found somewhere, we should load it.
	// FIXME: for now, we just warn the user!
	BPPrintMessage(odWarning, "Ignoring alphabet file specified in data or grammar file!");
	BPPrintMessage(odWarning, "(You can specify an alphabet file as a command-line argument).");
	return FAILED;
}	

int LoadGlossary(int anyfile,int manual)
{
	BP_NOT_USED(anyfile);
	BP_NOT_USED(manual);

	// LoadGlossary() is called from UpdateGlossary() only if !LoadedGl and 
	// either the data or grammar references a -gl file.
	// Console build sets LoadedGl if a glossary was loaded due to a command-line
	// argument, so this test is superfluous unless something changes ...
	if (LoadedGl) return OK;
	else {
		// Otherwise, if a -gl reference was found somewhere, we should load it.
		// FIXME: for now, we just warn the user!
		BPPrintMessage(odWarning, "Ignoring glossary file specified in data or grammar file!");
		BPPrintMessage(odWarning, "(You can specify a glossary file as a command-line argument).");
		return FAILED;
	}	
}

int LoadSettings(int anyfile,int changewindows,int startup,int manual,int *p_oms)
{
	BP_NOT_USED(anyfile);
	BP_NOT_USED(changewindows);
	BP_NOT_USED(startup);
	BP_NOT_USED(manual);
	BP_NOT_USED(p_oms);
	// For now, this is only called from ResetProject() shortly before exiting.
	// Keeping that call b/c might eventually make ResetProject() an API function.
	return OK;
}

int ClearWindow(int reset,int w)
{
	BP_NOT_USED(reset);
	BP_NOT_USED(w);
	// do we need some of the logic from ClearWindow() in Interface1.c ?
	return OK;
}

int ShowSelect(int dir,int w)
{
	BP_NOT_USED(dir);
	BP_NOT_USED(w);
	return OK;
}

int Reformat(int w,int font,int size,int face,RGBColor* p_color,int manual,int force)
{
	BP_NOT_USED(w);
	BP_NOT_USED(font);
	BP_NOT_USED(size);
	BP_NOT_USED(face);
	BP_NOT_USED(p_color);
	BP_NOT_USED(manual);
	BP_NOT_USED(force);
	return OK;
}

int InterruptCompute(int igram,t_gram *p_gram,int repeat,int grtype,int mode)
{
	BP_NOT_USED(igram);
	BP_NOT_USED(p_gram);
	BP_NOT_USED(repeat);
	BP_NOT_USED(grtype);
	BP_NOT_USED(mode);
	
	if (StepProduce || StepGrammars) {
		BPPrintMessage(odWarning, "Step-by-step production and step subgrammars options "
			"do not work yet in the console version.");
	}
	BPPrintMessage(odWarning, "Continuing from InterruptCompute()...");
	return OK;
}

/* Console version of GetMIDIfileName() just assumes that MIDIfileName 
   has been set by a command-line argument */
int GetMIDIfileName(void)
{
	if (strlen(MIDIfileName) != 0) return OK;
	else return FAILED;
}

int PrepareMIDIFile(void)
{
	BPPrintMessage(odWarning, "Midi file output does not work yet in the console version.");
	return OK;
}

int ResetMIDIfile(void)
{
	return OK;
}

int WriteMIDIbyte(Milliseconds time,byte midi_byte)
{
	BP_NOT_USED(time);
	BP_NOT_USED(midi_byte);
	return OK;
}

/* Console version of GetCsoundScoreName() just assumes that CsFileName 
   has been set by a command-line argument */
int GetCsoundScoreName(void)
{
	if (strlen(CsFileName) != 0) return OK;
	else return FAILED;
}

int MakeCsFile(char* pathname)
{	
	BP_NOT_USED(pathname);
	// FIXME ? ignoring pathname and assuming odiCsScore has been set
	ShowMessage(TRUE,wMessage,"Create new Csound score file…");
	CsScoreOpened = YES;
	if(WriteToFile(NO,CsoundFileFormat,"; Csound score",CsRefNum) != OK) {
		Alert1("Can't write to Csound score file. Unknown error");
		CloseCsScore();
		return(ABORT);
	}
	
	// FIXME: should write Csound tables here 
	// WriteFile(TRUE,CsoundFileFormat,CsRefNum,wCsoundTables,length);
	return OK;
}

int CheckTextSize(int w)
{
	BP_NOT_USED(w);
	return OK;
}


/* These are Mac OS X Carbon calls that could have useful replacements in
   the console/library build or that are too numerous in the BP2 source code
   to conditionalize with the preprocessor. */

void SysBeep(short duration)
{
	BP_NOT_USED(duration);
	// FIXME: should probably only send \007 to a terminal ?
	BPPrintMessage(odWarning, "\007Beep!\n");
	return;
}

Boolean Button(void)
{
	// See docs-developer/Uses-of-Button.txt for a discussion of this function
	// and why this return value may not be OK in the future! 
	return FALSE;
}
