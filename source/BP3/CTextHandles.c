/*  CTextHandles.c (BP3) */

/*  This file is a part of Bol Processor
    Copyright (c) 1990-2000 by Bernard Bel, Jim Kippen and Srikumar K. Subramanian
	Copyright (c) 2013, 2020 by Anthony Kozar
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


#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"
#include "ConsoleMessages.h"

/*	NewTextHandle()
	Creates and inits a new TEHandle (TERec**)
	TextHandle is a synonym for TEHandle (defined in -BP2.h)
 */
TEHandle NewTextHandle(void) {
	TEHandle th;
	TERec* tp;
	
	// allocate a handle to a TERec
	th = (TEHandle) GiveSpace(sizeof(TERec));
	if (th == NULL) return NULL;
	tp = *th;
	
	// allocate empty text buffer
	tp->hText = (Handle2) GiveSpace((Size)1);
	if (tp->hText == NULL) {
		MyDisposeHandle((Handle*)&th);
		return NULL;
		}
	else (*tp->hText)[0] = '\0';
	
	// initialize other fields
	tp->selStart = 0;
	tp->selEnd = 0;
	tp->length = 0;
	
	return th;	
	}

/*	CopyStringToTextHandle()
	
	Copies str to the internal text buffer of th.
	Maintains the correct buffer size and other TEHandle fields.
 */
int CopyStringToTextHandle(TEHandle th, const char* str) {
	int	res;
	size_t len;
	TERec* tp;
	char* dest;
	
	// resize text buffer and copy str
	len = strlen(str);
	tp = *th;
	res = MySetHandleSize((Handle*)&(tp->hText), len + (Size)1);	// +1 for null termination
	if (res != OK)	return res;
	dest = (char*) *tp->hText;
	strcpy(dest, str);
	
	// adjust other "instance variables"
	tp->length = (int32_t) len;
	if (tp->selStart > tp->length)	tp->selStart = tp->length;
	if (tp->selEnd > tp->length)	tp->selEnd = tp->length;
	return OK;
	}


int CopyHandleToTextHandle(TEHandle handle_copy,char** handle_source) {
	 // Validate pointers
    if (handle_copy == NULL || *handle_copy == NULL) {
        BPPrintMessage(0,odError,"=> CopyHandleToTextHandle error: Source handle is NULL\n");
        return MISSED; 
    	}
    if (handle_source == NULL) {
        BPPrintMessage(0,odError,"=> CopyHandleToTextHandle error: Destination handle is NULL\n");
        return MISSED;
    	}

    TERec *rec = *handle_copy;

    // Check if the text handle is already allocated or not
    if (*handle_source != NULL) {
        BPPrintMessage(0,odError,"=> CopyHandleToTextHandle error: *handle_source != NULL\n");
        free(*handle_source); // Prevent memory leaks by freeing existing memory
   		}

    // Allocate memory for the text in the destination handle
    *handle_source = malloc((rec->length + 1) * sizeof(char)); // +1 for null terminator
    if (*handle_source == NULL) {
        BPPrintMessage(0,odError,"=> CopyHandleToTextHandle error: Memory allocation failed\n");
        return MISSED;
    	}

    // Check if the text buffer in TEHandle is valid
    if (rec->hText == NULL || *(rec->hText) == NULL) {
        free(*handle_source);
        *handle_source = NULL;
        BPPrintMessage(0,odError,"=> CopyHandleToTextHandle error: Text buffer in source handle is NULL\n");
        return MISSED;
    	}

    // Copy the text from the source to the destination
	strcpy(*handle_source, (const char *)*(rec->hText));

    return OK; // Success	
	}



/* The functions below are skeleton reimplementations and stubs for some
   of the text window functionality in BP2's Carbon GUI.  These are needed
   while the computational units still directly reference the "windowed"
   text buffers, but it is hoped that all of these references to "virtual
   windows" can be removed in the future! -- akozar, 20130826
 */

int TextIsSelectionEmpty(TextHandle th)
{
	return (((*th)->selEnd - (*th)->selStart) == ZERO);
}

int TextGetSelection(TextOffset* start,TextOffset* end, TextHandle th)
{
	if (th == NULL)  return (MISSED);
	
	*start = (*th)->selStart;
	*end   = (*th)->selEnd;
	return OK;
}

int SetSelect(TextOffset start,TextOffset end, TextHandle th)
{
	long maxoffset;

	/* clamp range to text bounds */
	maxoffset = GetTextHandleLength(th);
	if (start < ZERO) {
		if(Beta) Alert1("=> Err. SetSelect(). start < ZERO");
		start = ZERO;
	}
	else if (start > maxoffset) {
		if(Beta) Alert1("=> Err. SetSelect(). start > maxoffset");
		start = maxoffset;
	}
	if (end < ZERO) {
		if(Beta) Alert1("=> Err. SetSelect(). end < ZERO");
		end = ZERO;
	}
	else if (end > maxoffset) {
		if(Beta) Alert1("=> Err. SetSelect(). end > maxoffset");
		end = maxoffset;
	}

	(*th)->selStart = start;
	(*th)->selEnd = end;
	return OK;
}

long GetTextLength(int w)
{
	if(w < 0 || w >= WMAX || !Editable[w]) {
		if(Beta) Alert1("=> Err. GetTextLength(). Incorrect w");
		return(ZERO);
	}
	return GetTextHandleLength(TEH[w]);
}

long GetTextHandleLength(TextHandle th)
{
	if (th == NULL) {
		if(Beta) Alert1("=> Err. GetTextHandleLength(). NULL handle");
		return(ZERO);
	}
	return((*th)->length);
}

char GetTextChar(int w,long pos)
{
	if(w < 0 || w >= WMAX || !Editable[w]) {
		if(Beta) Alert1("=> Err. GetTextChar(). Incorrect w");
		return '\0';
	}
	if (pos < 0 || pos > (*TEH[w])->length) {
		BPPrintMessage(0,odError,"=> Err. GetTextChar in %s: 'pos' (%ld) is out of range (max %ld)\n",WindowName[w],(long)pos,(long)(*TEH[w])->length);
		return '\0';
	}
	else if (pos == (*TEH[w])->length) {
		// length does not include the null char
		return '\0';
	}
	else  return (*(*TEH[w])->hText)[pos];
}

// Returns handle of the actual text buffer in a TextHandle
char** WindowTextHandle(TextHandle th)
{
	return (char**)((*th)->hText);
}

int Activate(TextHandle th)
{
	BP_NOT_USED(th);
	return OK;
}

int Deactivate(TextHandle th)
{
	BP_NOT_USED(th);
	return OK;
}

/* int DoKey(char c,EventModifiers modifiers,TextHandle th)
{
	char cbuffer[2];
	
	BP_NOT_USED(modifiers);
	if (c == '\0' || c == '\b')	{
		// report attempts to output null or backspace
		fprintf(stderr, "Ignoring DoKey(0x%X)\n", c);
	}
	else {
		cbuffer[0] = c;
		cbuffer[1] = '\0';
		TextInsert(cbuffer, 1L, th);
	}
	return OK;
} */

int TextDelete(int w)
{
	fprintf(stderr, "Ignoring TextDelete() in %s\n", WindowName[w]);
	return OK;
}

int TextInsert(char *s,long length,TextHandle th)
{
	int w, od;
	
	BP_NOT_USED(length);
	BP_NOT_USED(th);
	// find window index
	for (w = 0; w < WMAX; ++w) {
		if (th == TEH[w])  break;
	}
	switch (w)	{
		case wGrammar:	od = odTrace; break;
		case wData:		od = odDisplay; break;
		case wTrace:	od = odTrace; break;
		default:		od = odInfo; break;
	}
	if (od == odInfo) {
		BPPrintMessage(0,od, "TextInsert(%s): %s\n", (w < WMAX) ? WindowName[w] : "", s);
	}
	else {
		BPPrintMessage(0,od, "%s", s);
	}
	
	return OK;
}
