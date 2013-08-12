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

#include "-BP2.h"

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


/* These are Mac OS X Carbon calls that could have useful replacements in
   the console/library build or that are too numerous in the BP2 source code
   to conditionalize with the preprocessor. */

void SysBeep(short duration)
{
	// FIXME: should replace printf with a messaging API
	printf("\007Beep!\n");
}
