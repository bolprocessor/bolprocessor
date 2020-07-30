/*  ConsoleMessages.h (BP2 version CVS) */

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

#ifndef BP_CONSOLEMESSAGES_H
#define BP_CONSOLEMESSAGES_H

// Output destinations / messages types (these may be summed)

#define odDisplay	1		// for results of produce items, expand selection, etc.
#define odMidiDump	2		// for printing Midi messages as text
#define odCsScore	4		// for writing Csound score
#define odTrace		8		// for tracing processes (and step-by-step ?)
#define odInfo		16		// informational messages
#define odWarning	32		// warning messages
#define odError		64		// error messages
#define odUserInt	128		// interactive messages to which a response is expected

typedef	int (*bp_message_callback_t)(void* bp, int dest, const char *format, va_list parms);

void ConsoleMessagesInit(void);
void SetOutputDestinations(int dest, FILE* file);

int BPPrintMessage(int dest, const char *format, ...);
int BPSetMessageCallback(bp_message_callback_t func);


#endif /* BP_CONSOLEMESSAGES_H */
