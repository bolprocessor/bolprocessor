/* ConsoleMemory.c (BP3) */
/* August 12, 2013 */

/*  This file is a part of Bol Processor
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

/* ANSI replacements for memory allocation functions in BP3 */

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

static Handle GiveSpaceInternal(Size size, int clear);

// Allocate a new handle and a block of memory to return
inline
Handle GiveSpace(Size size) {
	return GiveSpaceInternal(size, FALSE);
	}


// Allocate a new handle and a block of memory initialized to zero
inline
Handle GiveZeroedSpace(Size size) {
	return GiveSpaceInternal(size, TRUE);
	}


static Handle GiveSpaceInternal(Size size, int clear) {
	s_handle_priv*	h;
	if(size <= 0L) {
		BPPrintMessage(wInfo,"=> Err. GiveSpaceInternal() size <= 0\n");
		return NULL;
		}
	// allocate a private handle structure
	h = malloc(sizeof(s_handle_priv));
	if(h == NULL) {
		BPPrintMessage(odError,"=> Out of memory in GiveSpaceInternal(), h == NULL\n");
		Panic = TRUE;
		return NULL;
		}
	// allocate the requested block
	if (clear)	h->memblock = calloc((size_t) 1, (size_t) size);
	else		h->memblock = malloc((size_t) size);
	if(h->memblock == NULL) {
		free(h);
		BPPrintMessage(odError,"=> Out of memory in GiveSpaceInternal(), h->memblock == NULL\n");
		Panic = TRUE;
		return NULL;
		}
	else {
		h->size = size;
		// track memory usage
		MaxHandles++;
		if(i_ptr < 5000) {
			mem_ptr[i_ptr] = (Handle)h;
			hist_mem_ptr[i_ptr] = 1;
			size_mem_ptr[i_ptr] = (int) size;
			}
		i_ptr++;
		MemoryUsed += (long) size;
		if(MemoryUsed > MaxMemoryUsed) MaxMemoryUsed = MemoryUsed;
		}
	return (Handle)h;
	}


Size MyGetHandleSize(Handle h) {
	if(h == NULL) return (Size) ZERO;
	if(((s_handle_priv*)h)->memblock == NULL) {
        BPPrintMessage(odError,"=> Err. MyGetHandleSize(). h->memblock == NULL\n");
        return (Size) ZERO;
    	}
	return ((s_handle_priv*)h)->size;
	}

int MyDisposeHandle(Handle *p_h) {
	int i;
	if(Panic) return ABORT;
	if (p_h == NULL || *p_h == NULL) {
	//	BPPrintMessage(odError,"=> Err. MyDisposeHandle. p_h or *p_h == NULL\n");
		*p_h = NULL;
		return OK;
		}
	s_handle_priv*	h = (s_handle_priv*) *p_h;
	if(h->memblock == NULL) {
		BPPrintMessage(odError,"=> Err. MyDisposeHandle(). h->memblock == NULL\n");
		return(ABORT);
		}
/*	for(i = 0; i < 5000; i++) { // 2024-05-20
		if(mem_ptr[i] == (Handle)(*p_h)) {
			hist_mem_ptr[i] = 2;
			}
		} */
	if(h->size < 1) {
		BPPrintMessage(odError,"=> Err. MyDisposeHandle(). size < 1\n");
		*p_h = NULL;
		return(ABORT);
		}
	MemoryUsed -= (long) h->size;
	free(h->memblock);
	free(h);
/*	if(check_memory_use && MemoryUsed < MemoryUsedInit) {
		BPPrintMessage(odInfo,"=> WARNING! MemoryUsed (%ld) < MemoryUsedInit (%ld) in %s/%s\n",(long)MemoryUsed,(long)MemoryUsedInit,__FILE__,__FUNCTION__);
		} */
	*p_h = NULL;
	return OK;
	}


Handle IncreaseSpace(Handle h) {
	Size oldsize, newsize;
	int rep;
	if(h == NULL) {
		BPPrintMessage(odError,"=> Err. IncreaseSpace(). h == NULL\n");
		return(NULL);
		}
	oldsize = MyGetHandleSize(h);
	// increase existing space by 50 percent
	// FIXME: how do we keep this calc from overflowing?
	newsize = (Size)2 + ((oldsize * (Size)3) / (Size)2);
	rep = MySetHandleSize(&h, newsize);
	if(rep != OK) return NULL;
	return(h);
	}


int MySetHandleSize(Handle* p_h, Size size) {
	s_handle_priv*	h;
	Size oldsize;
	int i;
//	BPPrintMessage(odInfo,"size = %ld\n",(long) size);
	if(size <= 0L) {
		BPPrintMessage(odError,"=> Err. MySetHandleSize() size <= 0\n");
		return ABORT;
		}
	if(p_h == NULL) {
		BPPrintMessage(odError,"=> Err. MySetHandleSize(). p_h == NULL");
		Panic = TRUE;
		return(ABORT);
		}
	if(*p_h != NULL) {
		// if the handle exists, just resize its memory block
		h = (s_handle_priv*) *p_h;
		if(h->memblock == NULL) {
			BPPrintMessage(odError,"=> Error(1) MySetHandleSize(). h->memblock == NULL\n");
			return ABORT;
			}
		oldsize = h->size;
		if(!InitOn && oldsize < (Size)1) {
			BPPrintMessage(odError,"=> Err. MySetHandleSize(). oldsize = %ld (1)\n", (long) oldsize);
			return ABORT;
			}
		void* new_mem = realloc(h->memblock, size);
		if (new_mem == NULL) {
			BPPrintMessage(odError,"=> Error(2) MySetHandleSize(). new_mem == NULL\n");
			return ABORT;
			}
		h->memblock = new_mem;
		h->size = size;
	/*	if (size > oldsize) */  MemoryUsed += (long)(size - oldsize);
	/*	else  MemoryUsed -= (unsigned long)(oldsize - size); */
		if(MemoryUsed > MaxMemoryUsed) {
			MaxMemoryUsed = MemoryUsed;
			}
		for(i = 0; i < 5000; i++) {
			if(mem_ptr[i] == (Handle)h) {
				size_mem_ptr[i] = (int) size;
				break;
				}
			}
		}
	else {
		// handle was NULL, so just do a fresh alloc
	//	BPPrintMessage(odError,"=> Warning: MySetHandleSize(%ld), handle was NULL\n",(long)size);
		if((*p_h = GiveSpace(size)) == NULL) {
			BPPrintMessage(odError,"=> Err. MySetHandleSize(%ld), handle == NULL\n",(long)size);
			return(ABORT);
			}
		}
//	BPPrintMessage(odInfo,"Done size = %ld\n",(long) size);
	return OK;
	}

inline int MyLock(int high,Handle h) {
	BP_NOT_USED(h);
	BP_NOT_USED(high);
	return(OK);
	}

inline int MyUnlock(Handle h) {
	BP_NOT_USED(h);
	return(OK);
	}
