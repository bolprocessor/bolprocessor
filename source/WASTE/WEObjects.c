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
 *	WEObjects.c
 *
 *	WASTE PROJECT
 *  Embedded Objects
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */


#include "WASTEIntf.h"

#if WASTE_OBJECTS

// static variables

static WEOHTableHandle _weGlobalObjectHandlerTable;

pascal FlavorType WEGetObjectType(WEObjectDescHandle hObjectDesc)
{
	return (*hObjectDesc)->objectType;
} // WEGetObjectType

pascal Handle WEGetObjectDataHandle(WEObjectDescHandle hObjectDesc)
{
	return (*hObjectDesc)->objectDataHandle;
} // WEGetObjectDataHandle

pascal Point WEGetObjectSize(WEObjectDescHandle hObjectDesc)
{
	return (*hObjectDesc)->objectSize;
} // WEGetObjectSize

pascal WEHandle WEGetObjectOwner(WEObjectDescHandle hObjectDesc)
{
	return (*hObjectDesc)->objectOwner;
} // WEGetObjectOwner

pascal long WEGetObjectRefCon(WEObjectDescHandle hObjectDesc)
{
	return (*hObjectDesc)->objectRefCon;
} // WEGetObjectRefCon

pascal void WESetObjectRefCon(WEObjectDescHandle hObjectDesc, long refCon)
{
	(*hObjectDesc)->objectRefCon = refCon;
} // WESetObjectRefCon

pascal long _WELookupObjectType(FlavorType objectType, WEOHTableHandle hTable)
{
	// look for a WEOHTableElement record for the specified object kind
	// in the given object handler table

	long nEntries, index;
	WEOHTablePtr pTable;

	// do nothing if the Object Handler Table has not been inited yet
	if (hTable == NULL)
		return kUnknownObjectType;

	// calculate entry count
	nEntries = GetHandleSize((Handle) hTable) / sizeof(WEOHTableElement);

	// scan the Object Handler Table looking for a type match
	pTable = *hTable;
	for ( index = 0; index < nEntries; index++ )
	{
		if (pTable->objectType == objectType) 
			return index;
		pTable++;
	}
	
	return kUnknownObjectType;
} // _WELookupObjectType

pascal OSErr _WEGetIndObjectType(long index, FlavorType *objectType, WEHandle hWE)
{
	WEOHTableHandle hTable;
	long nEntries;

	*objectType = 0L;

	// index must be non-negative
	if (index < 0)
		return weUnknownObjectTypeErr;
	
	// calculate number of entries in the instance-specific handler table
	nEntries = 0;
	hTable = (*hWE)->hObjectHandlerTable;
	if (hTable != NULL)
		nEntries = GetHandleSize((Handle) hTable) / sizeof(WEOHTableElement);

	// low indices refer to the instance-specific handler table
	if (index < nEntries)
	{
		*objectType = (*hTable)[index].objectType;
		return noErr;
	}
	// indices above that refer to the global handler table
	index -= nEntries;

	// calculate number of entries in the global handler table
	nEntries = 0;
	hTable = _weGlobalObjectHandlerTable;
	if (hTable != NULL)
		nEntries = GetHandleSize((Handle) hTable) / sizeof(WEOHTableElement);

	// return an error code if index is too large
	if (index >= nEntries)
		return weUnknownObjectTypeErr;

	*objectType = (*hTable)[index].objectType;
	
	return noErr;
} // _WEGetIndObjectType

pascal OSErr _WENewObject(FlavorType objectType, Handle objectDataHandle, WEHandle hWE,
					WEObjectDescHandle *hObjectDesc)
{
	WEObjectDescPtr pDesc;
	WEOHTableHandle hTable;
	long index;
	OSErr err;
	
	*hObjectDesc = NULL;

	// first look up the specified object type in the instance-specific handler table
	hTable = (*hWE)->hObjectHandlerTable;
	if ((index = _WELookupObjectType(objectType, hTable)) == kUnknownObjectType)
	{
		// no match: try with the global handler table
		hTable = _weGlobalObjectHandlerTable;
		if ((index = _WELookupObjectType(objectType, hTable)) == kUnknownObjectType)
			hTable = NULL;
	}
	
	// create a new relocatable block to hold the object descriptor
	if ((err = _WEAllocate(sizeof(WEObjectDesc), kAllocClear, (Handle *)hObjectDesc)) != noErr)
		return err;

	// lock it down
	HLock((Handle) *hObjectDesc);
	pDesc = (WEObjectDescPtr) **hObjectDesc;

	// fill in the object descriptor
	pDesc->objectType = objectType;
	pDesc->objectDataHandle = objectDataHandle;
	pDesc->objectSize.v = kDefaultObjectHeight;
	pDesc->objectSize.h = kDefaultObjectWidth;
	pDesc->objectTable = hTable;
	pDesc->objectIndex = index;
	pDesc->objectOwner = hWE;

	if (hTable != NULL) 
	{
		WENewObjectUPP newHandler = (*hTable)[index].newHandler;

		// call the new handler, if any
		if (newHandler != NULL) 
		{
			if ((err = CallWENewObjectProc(&pDesc->objectSize, *hObjectDesc, newHandler)) != noErr)
			{
				_WEForgetHandle((Handle *) hObjectDesc);
				return err;
			}
		}
	}
	
	// unlock the object descriptor
	HUnlock((Handle) *hObjectDesc);

	// clear result code
	return noErr;
} // _WENewObject

pascal OSErr _WEFreeObject(WEObjectDescHandle hObjectDesc)
{
	WEObjectDescPtr pDesc;
	OSErr err = noErr;

	// sanity check: do nothing if we have a null descriptor handle
	if (hObjectDesc == NULL) 
		return nilHandleErr;

	// lock the descriptor record
	HLock((Handle)hObjectDesc);
	pDesc = *hObjectDesc;

	if (pDesc->objectTable != NULL) 
	{
		WEDisposeObjectUPP freeHandler = (*pDesc->objectTable)[pDesc->objectIndex].freeHandler;

		// call the dispose handler, if any
		if (freeHandler != NULL) 
		{
			err = CallWEDisposeObjectProc(hObjectDesc, freeHandler);
			pDesc->objectDataHandle = NULL;
		}
	}
	
	// if object kind is unknown or there's no custom dispose handler, use DisposeHandle
	_WEForgetHandle(&pDesc->objectDataHandle);

	// finally, dispose of the object descriptor itself
	DisposeHandle((Handle)hObjectDesc);

	return err;
} // _WEFreeObject

pascal OSErr _WEDrawObject(WEObjectDescHandle hObjectDesc)
{
	WEObjectDescPtr pDesc;
	Rect destRect;
	PenState state;
	Boolean saveDescLock;
	OSErr err = noErr;
	
	// lock the object descriptor
	saveDescLock = _WESetHandleLock((Handle)hObjectDesc, true);
	pDesc = *hObjectDesc;

	// get current pen state
	// state.pnLoc has already been set to the bottom left of the rectangle to draw
	GetPenState(&state);

	// calculate the new pen position
	state.pnLoc.h += pDesc->objectSize.h;

	// calculate the destination rectangle
	*(long *)(&topLeft(destRect)) = DeltaPoint(state.pnLoc, pDesc->objectSize);
	botRight(destRect) = state.pnLoc;

	if (pDesc->objectTable != NULL) 
	{
		WEDrawObjectUPP drawHandler = (*pDesc->objectTable)[pDesc->objectIndex].drawHandler;

		// call the drawing handler, if any
		if (drawHandler != NULL) 
			err = CallWEDrawObjectProc(&destRect, hObjectDesc, drawHandler);
	}
	else
	{
		// if this object kind was not registered, draw an empty frame
		PenNormal();
		FrameRect(&destRect);
	}
	
	// restore original pen state, advancing the pen position by the object width
	SetPenState(&state);

	// unlock the object descriptor
	_WESetHandleLock((Handle)hObjectDesc, saveDescLock);

	return err;
} // _WEDrawObject

pascal Boolean _WEClickObject(Point hitPt, EventModifiers modifiers, unsigned long clickTime,
								WEObjectDescHandle hObjectDesc)
{
	WEObjectDescPtr pDesc;
	Boolean saveDescLock;
	Boolean clickHandled = false;

	// lock the object descriptor
	saveDescLock = _WESetHandleLock((Handle)hObjectDesc, true);
	pDesc = *hObjectDesc;

	if (pDesc->objectTable != NULL)
	{
		WEClickObjectUPP clickHandler = (*pDesc->objectTable)[pDesc->objectIndex].clickHandler;

		// call the click handler, if any
		if (clickHandler != NULL)
			clickHandled = CallWEClickObjectProc(hitPt, modifiers, clickTime, hObjectDesc, clickHandler);
	}

	// unlock the object descriptor
	_WESetHandleLock((Handle)hObjectDesc, saveDescLock);
	
	return clickHandled;
} // _WEClickObject

pascal OSErr _WEStreamObject(short destKind, FlavorType *theType,
				Handle *theData, Boolean *canDisposeData, WEObjectDescHandle hObjectDesc)
{
	WEObjectDescPtr pDesc;
	Boolean saveDescLock;
	OSErr err = weNotHandledErr;
	
	*canDisposeData = true;

	// lock the object descriptor
	saveDescLock = _WESetHandleLock((Handle)hObjectDesc, true);
	pDesc = *hObjectDesc;

	if (pDesc->objectTable != NULL)
	{
		WEStreamObjectUPP streamHandler = (*pDesc->objectTable)[pDesc->objectIndex].streamHandler;

		// call the stream handler, if any
		if (streamHandler != NULL)
			err = CallWEStreamObjectProc(destKind, theType, *theData, hObjectDesc, streamHandler);
	}
	
	if (err == weNotHandledErr)
	{
		// default streaming behavior
		_WEForgetHandle(theData);
		*canDisposeData = false;
		*theType = pDesc->objectType;
		*theData = pDesc->objectDataHandle;
		err = noErr;
	}
	
	// unlock the object descriptor
	_WESetHandleLock((Handle) hObjectDesc, saveDescLock);
	
	return err;
} // _WEStreamObject

pascal OSErr WEInstallObjectHandler(FlavorType objectType, WESelector handlerSelector,
				UniversalProcPtr handler, WEHandle hWE)
{
	WEOHTableHandle hTable;
	long index;
	WEOHTableElement element;
	OSErr err;

	// if hWE is NULL, install the handler in the global handler table,
	// otherwise install the handler in the instance-specific handler table
	if (hWE == NULL)
		hTable = _weGlobalObjectHandlerTable;
	else
		hTable = (*hWE)->hObjectHandlerTable;

	// create the handler table, if it doesn't exist
	if (hTable == NULL) 
	{
		hTable = (WEOHTableHandle) NewHandle(0);
		if ((err = MemError()) != noErr) 
			return err;
		if (hWE == NULL)
			_weGlobalObjectHandlerTable = hTable;
		else
			(*hWE)->hObjectHandlerTable = hTable;
	}

	// look for the entry corresponding to the specified object type
	if ((index = _WELookupObjectType(objectType, hTable)) == kUnknownObjectType) 
	{
		BLOCK_CLR(element);
		element.objectType = objectType;

		// previously unknown object type: append a new entry at the end of the handler table
		index = GetHandleSize((Handle) hTable) / sizeof(WEOHTableElement);
		if ((err = _WEInsertBlock((Handle) hTable, (const void *) &element, sizeof(element), index * sizeof(element))) != noErr)
			return err;
	}
	
	// install the handler
	return _WESetHandler(handlerSelector, (long *) &handler, (void *) (*hTable + index));

} // WEInstallObjectHandler

#endif  // WASTE_OBJECTS