/* ConsoleMain.c (BP2 version CVS) */
/* August 7, 2013 */

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

#include <stdio.h>

#include "-BP2.h"
#include "-BP2main.h"

// globals only for the console app
Boolean LoadedAlphabet = FALSE;

int main (int argc, char* args[])
{
	int  result;
	long leak;
	
	if(Inits() != OK)	return EXIT_FAILURE;
	
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

	MemoryUsedInit = MemoryUsed + leak;
	InitOn = FALSE;
	printf("BP2 Console completed initialization.\n");
	SessionTime = clock();

	/* This is where we ought to do something ... */
	
	/* Cleanup ... */
	
	/* MyDisposeHandle((Handle*)&Stream.code);
	Stream.imax = ZERO;
	Stream.period = ZERO; */
	LoadedCsoundInstruments = TRUE;
	if (TraceMemory && Beta) {
		if ((result = ResetProject(FALSE)) != OK)	printf("ResetProject() returned %d\n", result);
		printf("This session used %ld Kbytes maximum.  %ld handles created and released. [%ld bytes leaked]",
				(long) MaxMemoryUsed/1000L,(long)MaxHandles,
				(long) (MemoryUsed - MemoryUsedInit));
	}
	
	return EXIT_SUCCESS;
}
