/* This is defined by both Carbon and non-Carbon prefix headers */
#if  !defined(TARGET_API_MAC_CARBON)
   /* so if it is not defined yet, there is no prefix file, 
      and we are compiling the "Transitional" build. */
   /* Use MacHeaders.h until ready to convert this file.
      Then change to MacHeadersTransitional.h. */
#  include	"MacHeaders.h"
// #  include	"MacHeadersTransitional.h"
#endif

/*
 *	WEUtilities.c
 *
 *	WASTE PROJECT
 *  Miscellaneous General-Purpose Utilities
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */


#include "WASTEIntf.h"

pascal Boolean _WEBlockCmp(const void *block1, const void *block2, register long blockSize)
{
	register const char *p1 = (const char *) block1;
	register const char *p2 = (const char *) block2;

	while ( --blockSize >= 0 )
		if ( *p1++ != *p2++ )
			return false;
	
	return true;
}

pascal void _WEBlockClr(void *block, register long blockSize)
{
	register char *p = (char *) block;

	while ( --blockSize >= 0 )
		*p++ = 0;
}

pascal void _WEForgetHandle(Handle *h)
{
	Handle theHandle;
	
	if ((theHandle = *h) != NULL)
	{
		*h = NULL;
		DisposeHandle(theHandle);
	}
}

pascal Boolean _WESetHandleLock(Handle h, Boolean lock)
{
	Boolean oldLock = (HGetState(h) & (1 << 7)) != 0;
	
	if (lock != oldLock)
		if (lock)
			HLock(h);
		else
			HUnlock(h);
			
	return oldLock;
}

pascal void _WEReorder(long *a, long *b)
{
	if (*a > *b)
	{
		long temp = *a;
		*a = *b;
		*b = temp;
	}
}

pascal OSErr _WEAllocate(long blockSize, short allocFlags, Handle *h)
{
	// Allocate a new relocatable block.
	// AllocFlags may specify whether the block should be cleared and whether
	// temporary memory should be used.

	Handle theHandle = NULL;
	OSErr err;

	// if kAllocTemp is specified, try tapping temporary memory
	if (allocFlags & kAllocTemp)
	{
		theHandle = TempNewHandle(blockSize, &err);
	}

	// if kAllocTemp isn't specified, or TempNewHandle failed, try with current heap
	if (theHandle == NULL) 
	{
		theHandle = NewHandle(blockSize);
		err = MemError();
	}
	
	// if kAllocClear is specified, zero the block
	if ((allocFlags & kAllocClear) && (theHandle != NULL))
	{
		_WEBlockClr(*theHandle, blockSize);
	}

	*h = theHandle;	
	return err;
}
