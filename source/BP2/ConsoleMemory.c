/* ConsoleMemory.c (BP2 version CVS) */
/* August 12, 2013 */

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

#include <stdlib.h>

#include "-BP2.h"
#include "-BP2decl.h"

/* ANSI replacements for memory allocation functions in BP2main.c */

/* s_handle_priv is a "private" structure to implement Handles in a similar 
   fashion to Mac OS' relocatable blocks.  The memory pointed to by our handle
   struct is never moved unless realloc() moves the block when BP calls
   MySetHandleSize().  The important feature of Mac OS Handles that we need
   to preserve is that they know their own size!  It doesn't seem necessary
   to maintain bits for locked, purgeable, etc. status.
 */

/* WARNING:  Do not attempt to access the internal members of this struct
   anywhere outside of this file.  They may change! */
typedef struct s_handle_priv
{
	void*	memblock;
	Size	size;
} s_handle_priv;

static Handle GiveSpaceInternal(Size size, Boolean clear);

// Allocate a new handle and a block of memory to return
inline
Handle GiveSpace(Size size)
{
	return GiveSpaceInternal(size, FALSE);
}

// Allocate a new handle and a block of memory initialized to zero
inline
Handle GiveZeroedSpace(Size size)
{
	return GiveSpaceInternal(size, TRUE);
}

static Handle GiveSpaceInternal(Size size, Boolean clear)
{
	s_handle_priv*	h;
		
	// allocate a private handle structure
	h = malloc(sizeof(s_handle_priv));
	if(h == NULL) {
		Alert1("BP2 ran out of memory before completing the current task."
			   "You may want to save your work before continuing.");
		return NULL;
	}
	
	// allocate the requested block
	if (clear)	h->memblock = calloc((size_t) 1, (size_t) size);
	else		h->memblock = malloc((size_t) size);
	if(h->memblock == NULL) {
		free(h);
		Alert1("BP2 ran out of memory before completing the current task."
			   "You may want to save your work before continuing.");
		return NULL;
	}
	else {
		h->size = size;
		// track memory usage
		MaxHandles++;
		MemoryUsed += (unsigned long) size;
		if(MemoryUsed > MaxMemoryUsed) {
			MaxMemoryUsed = MemoryUsed;
		}
	}
	return (Handle)h;
}

Size MyGetHandleSize(Handle h)
{
	if(h == NULL) return (Size) ZERO;
	else return ((s_handle_priv*)h)->size;
}

int MyDisposeHandle(Handle *p_h)
{
	if (p_h == NULL) {
		if(!EmergencyExit && Beta) Alert1("Err. MyDisposeHandle. p_h = NULL");
		return(ABORT);
	}
	if(*p_h != NULL) {
		s_handle_priv*	h = (s_handle_priv*) *p_h;
		if(h->size < (Size)1) {
			if(!EmergencyExit && Beta) Alert1("Err. MyDisposeHandle. size < 1");
			*p_h = NULL;
			return(ABORT);
		}
		MemoryUsed -= (unsigned long) h->size;
		free(h->memblock);
		free(h);
		// no way to check for errors ?
	}
	*p_h = NULL;
	return OK;
}

Handle IncreaseSpace(Handle h)
{
	Size oldsize, newsize;
	int rep;

	if(h == NULL) {
		if(Beta) Alert1("Err. IncreaseSpace(). h = NULL");
		return(NULL);
	}
	oldsize = MyGetHandleSize(h);
	// increase existing space by 50 percent
	// FIXME: how do we keep this calc from overflowing?
	newsize = (Size)2 + ((oldsize * (Size)3) / (Size)2);
	rep = MySetHandleSize(&h, newsize);
	if(rep != OK) return NULL;
	
	MemoryUsed += (newsize - oldsize);
	if(MemoryUsed > MaxMemoryUsed) {
		MaxMemoryUsed = MemoryUsed;
	}
	
	return(h);
}


int MySetHandleSize(Handle* p_h,Size size)
{
	s_handle_priv*	h;
	Size oldsize;
	
	if(p_h == NULL) {
		sprintf(Message,"Err. MySetHandleSize(). p_h == NULL");
		if(Beta) Alert1(Message);
		return(ABORT);
	}
	if(*p_h != NULL) {
		// if the handle exists, just resize its memory block
		h = (s_handle_priv*) *p_h;
		oldsize = h->size;
		if(Beta && !InitOn && oldsize < (Size)1) {
			sprintf(Message,"Err. MySetHandleSize(). oldsize = %ld", (long) oldsize);
			Alert1(Message);
		}
		h->memblock = realloc(h->memblock, size);
		if (h->memblock == NULL) {
			Alert1("BP2 ran out of memory before completing the current task."
				   "You may want to save your work before continuing.");
			return (ABORT);
		}
		else {
			if (size > oldsize)  MemoryUsed += (unsigned long)(size - oldsize);
			else  MemoryUsed -= (unsigned long)(oldsize - size);
			if(MemoryUsed > MaxMemoryUsed) {
				MaxMemoryUsed = MemoryUsed;
			}
		}
	}
	else {
		// handle was NULL, so just do a fresh alloc
		if((*p_h = GiveSpace(size)) == NULL) return(ABORT);
	}
	
	return(OK);
}


inline
int MyLock(int high,Handle h)
{
#if	COMPILING_BETA
	if(h == NULL) {
		if(Beta) Alert1("Attempted to lock NULL handle");
		return(ABORT);
	}
#else
	BP_NOT_USED(h);
#endif
	BP_NOT_USED(high);
	// do nothing
	return(OK);
}

inline
int MyUnlock(Handle h)
{
#if	COMPILING_BETA
	if(h == NULL) {
		if(Beta) Alert1("Attempted to unlock NULL handle");
		return(ABORT);
	}
#else
	BP_NOT_USED(h);
#endif
	// do nothing
	return(OK);
}