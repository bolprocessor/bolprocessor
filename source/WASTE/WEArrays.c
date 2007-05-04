/* This is defined by both Carbon and non-Carbon prefix headers */
#if  !defined(TARGET_API_MAC_CARBON)
   /* so if it is not defined yet, there is no prefix file, 
      and we are compiling the "Transitional" build. */
   /* Use MacHeaders.h until ready to convert this file.
      Then change to MacHeadersTransitional.h. */
// #  include	"MacHeaders.h"
#  include	"MacHeadersTransitional.h"
#endif

/*
 *	WEArrays.c
 *
 *	WASTE PROJECT
 *  Utilities for handling handle-based dynamic arrays
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */

#include "WASTEIntf.h"

pascal OSErr _WEInsertBlock(Handle h, const void *blockPtr, long blockSize, long offset)
{
	long oldSize;
	OSErr err;

// get handle size
	oldSize = GetHandleSize(h);
	WEASSERT((offset >= 0) && (offset <= oldSize), "\p_WEInsertBlock: bad offset");

// make room for the block to be inserted
	SetHandleSize(h, oldSize + blockSize);
	if ((err = MemError()) != noErr)
		return err;
	BlockMoveData( *h + offset, *h + offset + blockSize, oldSize - offset );

// insert block
	BlockMoveData( blockPtr, *h + offset, blockSize);
	
	return noErr;
}

pascal void _WERemoveBlock(Handle h, long blockSize, long offset)
{
	long newSize;

// get handle size minus a "slot"
	newSize = GetHandleSize(h) - blockSize;
	WEASSERT((offset >= 0) && (offset <= newSize), "\p_WERemoveBlock: bad offset");

// compact the handle (this should never fail)
	BlockMoveData( *h + offset + blockSize, *h + offset, newSize - offset );
	SetHandleSize(h, newSize);
	WEASSERT(MemError() == noErr, "\p_WERemoveSlot: failed to shorten handle");
}
