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
	oldSize = InlineGetHandleSize(h);
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
	newSize = InlineGetHandleSize(h) - blockSize;
	WEASSERT((offset >= 0) && (offset <= newSize), "\p_WERemoveBlock: bad offset");

// compact the handle (this should never fail)
	BlockMoveData( *h + offset + blockSize, *h + offset, newSize - offset );
	SetHandleSize(h, newSize);
	WEASSERT(MemError() == noErr, "\p_WERemoveSlot: failed to shorten handle");
}
