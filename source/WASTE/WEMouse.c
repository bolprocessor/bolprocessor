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
 *	WEMouse.c
 *
 *	WASTE PROJECT
 *  Mouse Clicks and Support for Drag and Drop
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */


#include "WASTEIntf.h"

#ifndef __FOLDERS__
#include <Folders.h>
#endif

#if WASTE_IC_SUPPORT
#ifndef __ICTYPES__
#include "ICTypes.h"
#endif
#ifndef __ICAPI__
#include "ICAPI.h"
#endif
#endif

// WASTE_IC_SUPPORT

// static variables

static DragSendDataUPP _weFlavorSender = NULL;

pascal Boolean _WEIsOptionDrag(DragReference theDrag)
{
	short downModifiers, upModifiers;
	
	// get drag modifiers
	GetDragModifiers(theDrag, NULL, &downModifiers, &upModifiers);

	// return true if the option key was held down at the beginning and/or at the end
	return (((downModifiers | upModifiers) & optionKey) != 0);

} // _WEIsOptionDrag

pascal OSErr _WEGetFlavor(DragReference theDrag, ItemReference theItem,
				FlavorType requestedType, Handle hFlavor,
				WETranslateDragUPP translateDragHook)
{

	// get the requested flavor out of the specified drag reference and put it into
	// the given handle, if any -- if hFlavor is NULL, just check whether the specified flavor
	// is there or can be obtained by invoking a user-defined translation routine

	FlavorFlags theFlags;
	Size theSize;
	Boolean saveFlavorLock;
	OSErr err;

	// see if the drag item has the requested flavor type,
	// without forcing the actual data to be sent and/or translated
	err = GetFlavorFlags(theDrag, theItem, requestedType, &theFlags);
	if (err == badDragFlavorErr)
	{
		// requested flavor is not available: our client may try a custom translation
		// this is especially handy to translate HFS objects like TEXT and PICT files
		if (translateDragHook != NULL)
		{
			err = CallWETranslateDragProc(theDrag,
					theItem, requestedType, hFlavor, translateDragHook);
		}
	}
	else if (err == noErr)
	{
		
		// requested flavor is available: get it if hFlavor is not NULL
		if (hFlavor == NULL)
			return err;

		// get size of flavor data
		if ((err = GetFlavorDataSize(theDrag, theItem, requestedType, &theSize)) != noErr)
			return err;

		// resize the handle
		SetHandleSize(hFlavor, theSize);
		if ((err = MemError()) != noErr)
			return err;

		// get flavor data
		saveFlavorLock = _WESetHandleLock(hFlavor, true);
		err = GetFlavorData(theDrag, theItem, requestedType, *hFlavor, &theSize, 0);
		_WESetHandleLock(hFlavor, saveFlavorLock);
	}
	
	return err;
} // _WEGetFlavor

pascal OSErr _WEExtractFlavor(DragReference theDrag, ItemReference theItem,
						FlavorType theType, Handle *hFlavor,
						WETranslateDragUPP translateDragHook)
{
	OSErr err;

	// allocate a new handle
	err = _WEAllocate(0, kAllocTemp, hFlavor);
	if (err == noErr)
	{
		// put the requested flavor into this handle
		err = _WEGetFlavor(theDrag, theItem, theType, *hFlavor, translateDragHook);

		// if an error occurred, forget the handle
		if (err != noErr)
			_WEForgetHandle(hFlavor);
	}

	return err;
} // _WEExtractFlavor

pascal Boolean WECanAcceptDrag(DragReference theDrag, WEHandle hWE)
{
	WEPtr pWE;
	unsigned short numDragItems;
	unsigned short dragItemIndex;
	ItemReference theItem;
	Boolean saveWELock;
	OSErr err;
	Boolean retval = false;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// refuse all drags if the weFReadOnly feature is enabled
	if (BTST(pWE->features, weFReadOnly))
		goto cleanup;

	// count items in this theDrag
	if ((err = CountDragItems(theDrag, &numDragItems)) != noErr)
		goto cleanup;

	for (dragItemIndex = 1; dragItemIndex <= numDragItems; dragItemIndex++)
	{
		// get item reference number for current drag item
		if ((err = GetDragItemReferenceNumber(theDrag, dragItemIndex, &theItem)) != noErr)
			goto cleanup;

		// see if this drag item contains a text flavor
		err = _WEGetFlavor(theDrag, theItem, kTypeText, NULL, pWE->translateDragHook);

#if WASTE_OBJECTS
		if (err == badDragFlavorErr)
		{
			long objectIndex;
			FlavorType objectType;

			// see if this drag item contains a flavor matching one of the registered object types
			objectIndex = 0;
			while (_WEGetIndObjectType(objectIndex, &objectType, hWE) == noErr)
			{
				err = _WEGetFlavor(theDrag, theItem, objectType, NULL, pWE->translateDragHook);
				if (err != badDragFlavorErr)
					break;  // enclosing while
				objectIndex++;
			} // while
		}
#endif

		if (err != noErr)
			goto cleanup;
	} // for

	retval = true;

cleanup:

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	return retval;

} // WECanAcceptDrag

pascal void _WEUpdateDragCaret(long offset, WEHandle hWE)
{
	WEPtr pWE = *hWE;	// assume WE record is already locked
	unsigned long currentTime;

	// get current time
	currentTime = TickCount();

	if (offset == pWE->dragCaretOffset)
	{

		// drag caret offset didn't change; blink the caret
		if ((currentTime > pWE->caretTime + GetCaretTime()) && (offset != kInvalidOffset))
		{
			_WEDrawCaret(pWE->dragCaretOffset, hWE);
			BCHG(pWE->flags, weFDragCaretVisible);	// invert flag
			pWE->caretTime = currentTime;
		}
	}
	else
	{

		// drag caret offset did change
		// hide old caret, if it's showing
		if (BTST(pWE->flags, weFDragCaretVisible))
			_WEDrawCaret(pWE->dragCaretOffset, hWE);

		// show new caret (unless offset is kInvalidOffset)
		if (offset != kInvalidOffset)
		{
			_WEDrawCaret(offset, hWE);
			BSET(pWE->flags, weFDragCaretVisible);
			pWE->caretTime = currentTime;
		}
		else
		{
			BCLR(pWE->flags, weFDragCaretVisible);
		}
	
		// remember drag caret offset
		pWE->dragCaretOffset = offset;
	}
} // _WEUpdateDragCaret

pascal OSErr WETrackDrag(DragTrackingMessage theMessage, DragReference theDrag, WEHandle hWE)
{
	WEPtr pWE;
	DragAttributes attributes;
	Point mouse;
	RgnHandle tmpRgn;
	LongPt thePoint;
	long offset;
	unsigned long currentTime;
	char edge;
	Boolean saveWELock;
	OSErr err;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// dispatch on theMessage
	switch(theMessage)
	{
		case kDragTrackingEnterWindow:
			// determine whether we can accept this drag
			if (WECanAcceptDrag(theDrag, hWE))
				BSET(pWE->flags, weFCanAcceptDrag);
			else
				BCLR(pWE->flags, weFCanAcceptDrag);

			// reset clickTime
			pWE->clickTime = 0;
			break;

		case kDragTrackingInWindow:
			if (BTST(pWE->flags, weFCanAcceptDrag))
			{

				// get drag attributes
				if ((err = GetDragAttributes(theDrag, &attributes)) != noErr)
					goto cleanup;

				// get current mouse location in local coordinates
				if ((err = GetDragMouse(theDrag, &mouse, NULL)) != noErr)
					goto cleanup;
				GlobalToLocal(&mouse);

				if (PtInRgn(mouse, pWE->viewRgn))
				{
					// mouse is in text area
					// hilite the text rectangle, if we haven't already
					// and if the drag has left sender window since drag tracking started
					if ((!BTST(pWE->flags, weFHilited)) &&
						(attributes & kDragHasLeftSenderWindow) != 0)
					{
						tmpRgn = NewRgn();
						CopyRgn(pWE->viewRgn, tmpRgn);
						InsetRgn(tmpRgn, -kTextMargin, -kTextMargin);
						ShowDragHilite(theDrag, tmpRgn, true);
						DisposeRgn(tmpRgn);
						BSET(pWE->flags, weFHilited);
					}

					// hide the caret
					if (BTST(pWE->flags, weFCaretVisible))
						_WEBlinkCaret(hWE);

					// get text offset corresponding to mouse location
					WEPointToLongPoint(mouse, &thePoint);
					offset = WEGetOffset(&thePoint, &edge, hWE);

					// if offset is within the original selection range, don't display drag feedback
					if (theDrag == pWE->currentDrag)
						if (_WEOffsetInRange(offset, edge, pWE->selStart, pWE->selEnd))
							offset = kInvalidOffset;

					// provide a drag feedback in the form of a blinking caret
					_WEUpdateDragCaret(offset, hWE);

					// clear clickTime
					pWE->clickTime = 0;
				}
				else
				{

					// mouse is outside text area
					// dehilite the text rectangle, if it's hilited
					if (BTST(pWE->flags, weFHilited))
					{
						HideDragHilite(theDrag);
						BCLR(pWE->flags, weFHilited);
					}

					// hide the drag caret, if it's showing
					_WEUpdateDragCaret(kInvalidOffset, hWE);

					// if the mouse has been remaining outside the view region for 10 ticks or more
					// and this drag was created by this WE instance, call the click loop routine
					if (theDrag == pWE->currentDrag)
					{
						currentTime = TickCount();
						if (pWE->clickTime == 0)
							pWE->clickTime = currentTime;
						else if (currentTime > pWE->clickTime + kAutoScrollDelay)
							if (pWE->clickLoop != NULL)
								CallWEClickLoopProc(hWE, pWE->clickLoop);
					}
				}
			}
			break; // case dragTrackingInWindow

		case kDragTrackingLeaveWindow:

			// drag has left this window
			// dehilite the text area if necessary
			if (BTST(pWE->flags, weFHilited))
			{
				HideDragHilite(theDrag);
				BCLR(pWE->flags, weFHilited);
			}

			// hide the drag caret, if it's showing
			_WEUpdateDragCaret(kInvalidOffset, hWE);

			break;

		default:
			;
	} // case theMessage

	// clear result code
	err = noErr;

cleanup:
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return result code
	return err;

} // WETrackDrag

pascal OSErr WEReceiveDrag(DragReference theDrag, WEHandle hWE)
{
	WEPtr pWE;
	WEActionHandle hAction;
	Handle hText = NULL;
	Handle hStyles = NULL;
#if WASTE_OBJECTS
	Handle hSoup = NULL;
#endif
	Handle hObjectData = NULL;
	Point mouse;
	LongPt dropLocation;
	long insertionOffset;
	long insertionLength;
	long sourceStart;
	long sourceEnd;
	long destStart;
	long destEnd;
	long delta;
	unsigned short dragItemIndex;
	unsigned short numDragItems;
	ItemReference theItem;
	GrafPtr savePort;
	short intPasteAction;
	char dropEdge;
	char space = kSpace;
	Boolean isMove = false;
	Boolean isBackwards;
	Boolean saveWELock;
	OSErr err;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// set up the port
	GetPort(&savePort);
	SetPort(pWE->port);

	// stop any ongoing inline input session
	WEStopInlineSession(hWE);

	// hide the drag caret
	_WEUpdateDragCaret(kInvalidOffset, hWE);

	// refuse this drag if it doesn't taste good
	err = badDragFlavorErr;
	if (!WECanAcceptDrag(theDrag, hWE))
		goto cleanup;

	// get drop location in local coordinates
	if ((err = GetDragMouse(theDrag, &mouse, NULL)) != noErr)
		goto cleanup;
	GlobalToLocal(&mouse);

	// for the drag to be accepted, the drop location must be within the view region
	err = dragNotAcceptedErr;
	if (!PtInRgn(mouse, pWE->viewRgn))
		goto cleanup;

	// get drop offset into the text
	WEPointToLongPoint(mouse, &dropLocation);
	insertionOffset = WEGetOffset(&dropLocation, &dropEdge, hWE);
	// destStart/destEnd define the range to highlight at the end of the drag
	destStart = insertionOffset;

	// drag originated from this same window?
	if (theDrag == pWE->currentDrag)
	{

		// sourceStart/sourceEnd define the range to delete at the end of the move
		sourceStart = pWE->selStart;
		sourceEnd = pWE->selEnd;

		// remember text length before insertion
		delta = pWE->textLength;

		// if insertion offset is within the original selection range, abort the drag
		// (*err = dragNotAcceptedErr;*)
		if (_WEOffsetInRange(insertionOffset, dropEdge, sourceStart, sourceEnd))
			goto cleanup;

		// if the drag originated from this window, a move,
		// rather than a copy, should be performed
		// Exception: the option key may be held down at mouse-down
		// or mouse-up time to force a copy operation.

		isMove = !_WEIsOptionDrag(theDrag);
		isBackwards = (insertionOffset <= sourceStart);
	} // if intra-window drag

	// clear null style
	BCLR(pWE->flags, weFUseNullStyle);

	// hide selection highlighting
	_WEHiliteRange(pWE->selStart, pWE->selEnd, hWE);

	// increment modification count
	pWE->modCount++;

	// if undo support is enabled, create a new action so we'll be able to undo the insertion
	if (BTST(pWE->features, weFUndoSupport))
	{
		WEClearUndo(hWE);
		if (_WENewAction(insertionOffset, insertionOffset, 0, weAKDrag, 0, hWE, &hAction) == noErr)
		{
			_WEPushAction(hAction);
		}
	}

	// count items in this drag
	if ((err = CountDragItems(theDrag, &numDragItems)) != noErr)
		goto cleanup;

	for (dragItemIndex = 1; dragItemIndex<=numDragItems; dragItemIndex++)
	{
		// get item reference number for current drag item
		if ((err = GetDragItemReferenceNumber(theDrag, dragItemIndex, &theItem)) != noErr)
			goto cleanup;

		// see if this drag item contains a text flavor
		err = _WEExtractFlavor(theDrag, theItem, kTypeText, &hText, pWE->translateDragHook);
		if (err == noErr)
		{
			if (!BTST(pWE->features, weFMonoStyled))
			{
				// extract accompanying styles and soup, if any
				err = _WEExtractFlavor(theDrag, theItem, kTypeStyles, &hStyles, pWE->translateDragHook);
				if ((err != noErr) && (err != badDragFlavorErr))
					goto cleanup;
#if WASTE_OBJECTS
				err = _WEExtractFlavor(theDrag, theItem, kTypeSoup, &hSoup, pWE->translateDragHook);
				if ((err != noErr) && (err != badDragFlavorErr))
					goto cleanup;
#endif
			}

			// any extra space added because of intelligent cut-and-paste rules will use the
			// style attributes set at the insertion point
			if (dragItemIndex == 1)
			{
				pWE->selStart = insertionOffset;
				pWE->selEnd = insertionOffset;
				_WESynchNullStyle(hWE);
			}

			// get text length
			insertionLength = GetHandleSize(hText);
			destEnd = insertionOffset + insertionLength;

			// insert the new text at the insertion point
			HLock(hText);
			err = _WEInsertText(insertionOffset, *hText, insertionLength, hWE);
			_WEForgetHandle(&hText);
			if (err != noErr)
				goto cleanup;

			// adjust deletion range length in undo buffer
			_WEAdjustUndoRange(insertionLength, hWE);

			// apply the accompanying styles, if any
			if (hStyles != NULL)
			{
				if ((err = _WEApplyStyleScrap(insertionOffset, destEnd,
							(StScrpHandle) hStyles, hWE)) != noErr)
					goto cleanup;
				_WEForgetHandle(&hStyles);
			}

#if WASTE_OBJECTS
			// apply the accompanying soup, if any
			if (hSoup != NULL)
			{
				if ((err = _WEApplySoup(insertionOffset, hSoup, hWE)) != noErr)
					goto cleanup;
				_WEForgetHandle(&hSoup);
			}
#endif

			// determine whether an extra space should be added before or after the inserted text
			intPasteAction = _WEIntelligentPaste(insertionOffset, destEnd, hWE);

			// add the extra space, if necessary
			if (intPasteAction != weDontAddSpaces)
			{
				if (intPasteAction == weAddSpaceOnLeftSide)
				{
					if ((err = _WEInsertText(insertionOffset, &space, sizeof(space), hWE)) != noErr)
						goto cleanup;

					destEnd++;

					// if an extra space is inserted in front of all dropped items,
					// don't count it when eventually highlighting the destination range
					if (dragItemIndex == 1)
						destStart++;
				}
				else
				{
					if ((err = _WEInsertText(destEnd, &space, sizeof(space), hWE)) != noErr)
						goto cleanup;
				}

				insertionLength++;
				_WEAdjustUndoRange(1, hWE);
			} // if extra space

		}

#if WASTE_OBJECTS
		else if (err == badDragFlavorErr)
		{
			long objectIndex;
			FlavorType objectType;
			Point objectSize;
			short saveUndoSupport;
			short saveInhibitRecal;
			
			objectSize.v = 0;
			objectSize.h = 0;

			// no text flavor: there must be a flavor matching one of the registered object types
			objectIndex = 0;
			while (_WEGetIndObjectType(objectIndex, &objectType, hWE) == noErr)
			{
				err = _WEExtractFlavor(theDrag, theItem, objectType, &hObjectData, pWE->translateDragHook);
				if (err == noErr)
					break; // enclosing while
				if (err != badDragFlavorErr)
					goto cleanup;
				objectIndex++;
			} // while
			if (err != noErr)
				goto cleanup;

			// set insertion point on first iteration (*after* extracting flavors, in case we are
			// doing an intra-window move, otherwise our send proc would be confused)
			if (dragItemIndex == 1)
			{
				pWE->selStart = insertionOffset;
				pWE->selEnd = insertionOffset;
			}

			// insert the object, but without touching undo or redrawing the text
			saveUndoSupport = WEFeatureFlag(weFUndoSupport, weBitClear, hWE);
			saveInhibitRecal = WEFeatureFlag(weFInhibitRecal, weBitSet, hWE);
			err = WEInsertObject(objectType, hObjectData, objectSize, hWE);
			WEFeatureFlag(weFUndoSupport, saveUndoSupport, hWE);
			WEFeatureFlag(weFInhibitRecal, saveInhibitRecal, hWE);
			if (err != noErr)
				goto cleanup;

			insertionLength = 1;
			destEnd = insertionOffset + 1;
			pWE->modCount--; // compensate for increment made by WEInsertObject
			_WEAdjustUndoRange(1, hWE);
		}
#endif	// WASTE_OBJECTS

		else
			goto cleanup;

		// advance insertion offset for subsequent drag items, if any
		insertionOffset += insertionLength;

	} // for

	if (isMove)
	{
		// adjust source range
		if (isBackwards)
		{
			delta -= pWE->textLength;
			sourceStart -= delta;
			sourceEnd -= delta;
		}

		// extend range according to intelligent cut-and-paste rules
		_WEIntelligentCut(&sourceStart, &sourceEnd, hWE);

		// if undo support is enabled, create a new action so we'll be able to undo the deletion
		if (BTST(pWE->features, weFUndoSupport))
		{
			if (_WENewAction(sourceStart, sourceEnd, 0, weAKDrag, 0, hWE, &hAction) == noErr)
			{
				_WEPushAction(hAction);
			}
		}

		// delete source range
		delta = pWE->textLength;
		if ((err = _WEDeleteRange(sourceStart, sourceEnd, hWE)) != noErr)
			goto cleanup;

		// adjust destination range
		if (!isBackwards)
		{
			delta -= pWE->textLength;
			destStart -= delta;
			destEnd -= delta;
		}

	} // if isMove

	// select the range encompassing all items dropped
	pWE->selStart = destStart;
	pWE->selEnd = destEnd;

	// redraw the text
	if (isMove)
		if (sourceStart < destStart)
			err = _WERedraw(sourceStart, destEnd, hWE);
		else
			err = _WERedraw(destStart, sourceEnd, hWE);
	else
		err = _WERedraw(destStart, destEnd, hWE);

cleanup:
	// dispose of temporary handles
	_WEForgetHandle(&hText);
	_WEForgetHandle(&hStyles);
#if WASTE_OBJECTS
	_WEForgetHandle(&hSoup);
#endif

	// restore the port
	SetPort(savePort);

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return result code
	return err;

} // WEReceiveDrag

pascal OSErr _WESendFlavor(FlavorType requestedType, void *dragSendRefCon, WEHandle hWE, DragReference theDrag)
{
#pragma unused(dragSendRefCon)
	
#if !GENERATINGCFM
	long saveA5 = SetCurrentA5();	// this fixes a conflict with HoverBar
									// (well, probably a bug in the Drag Manager)
#endif

	WEPtr pWE = *hWE;
	long selStart = pWE->selStart;
	long selEnd = pWE->selEnd;
	Handle hItem = NULL;
	Boolean disposeItem = true;		// dispose of item when done
#if WASTE_OBJECTS
	WEObjectDescHandle hObjectDesc;
#endif
	OSErr err;

	// allocate a temporary handle to hold a copy of the requested flavor
	if ((err = _WEAllocate(0, kAllocTemp, &hItem)) != noErr)
		goto cleanup;

#if WASTE_OBJECTS
	// see if the selection contains an embedded object whose type matches the flavortype
	if (WEGetSelectedObject(&hObjectDesc, hWE) == noErr)
	{
		FlavorType theType;

		if ((err = _WEStreamObject(weToDrag, &theType, &hItem, &disposeItem, hObjectDesc)) != noErr)
			goto cleanup;
		
		// make sure theType matches the requested type
		err = badDragFlavorErr;
		if (theType != requestedType)
			goto cleanup;
	}
	else
#endif
	{
		// identify the requested flavor type as either 'TEXT', 'styl' or 'SOUP'
		if (requestedType == kTypeText)
			err = WECopyRange(selStart, selEnd, hItem, NULL, NULL, hWE);
		else if (requestedType == kTypeStyles)
			err = WECopyRange(selStart, selEnd, NULL, hItem, NULL, hWE);
#if WASTE_OBJECTS
		else if (requestedType == kTypeSoup)
			err = WECopyRange(selStart, selEnd, NULL, NULL, hItem,hWE);
#endif
		else
			err = badDragFlavorErr;

		if (err != noErr)
			goto cleanup;

	}

	// set the drag flavor data
	HLock(hItem);
	err = SetDragItemFlavorData(theDrag, (ItemReference) hWE, requestedType, *hItem,
			GetHandleSize(hItem), 0);
	HUnlock(hItem);

cleanup:
	if (disposeItem)
		_WEForgetHandle(&hItem);

#if !GENERATINGCFM
	SetA5(saveA5);
#endif

	// return result code
	return err;

} // _WESendFlavor

pascal Boolean WEDraggedToTrash(DragReference theDrag)
{

	// return true if the drop location of the specified drag is the trash

	const short bDirectoryAttr = 4;

	AEDesc dropLocation, coercedDropLocation;
	CInfoPBRec pb;
	FSSpecPtr pSpec;
	short trashVRefNum;
	long trashDirID;
	Boolean draggedToTrash;

	draggedToTrash = false;
	dropLocation.dataHandle = NULL;
	coercedDropLocation.dataHandle = NULL;

	// get drop location
	if (GetDropLocation(theDrag, &dropLocation) != noErr)
		goto cleanup;

	// do nothing if dropLocation is a null descriptor
	if (dropLocation.descriptorType == typeNull)
		goto cleanup;

	// try to coerce the descriptor to a file system specification record
	if (AECoerceDesc(&dropLocation, typeFSS, &coercedDropLocation) != noErr)
		goto cleanup;

	// lock the data handle of the coerced descriptor
	HLock(coercedDropLocation.dataHandle);
	pSpec = *(FSSpecHandle)coercedDropLocation.dataHandle;

	// determine the directory ID of the drop location (assuming it's a folder!)
	BLOCK_CLR(pb);
	pb.hFileInfo.ioVRefNum = pSpec->vRefNum;
	pb.hFileInfo.ioDirID = pSpec->parID;
	pb.hFileInfo.ioNamePtr = pSpec->name;
	if (PBGetCatInfoSync(&pb) != noErr)
		goto cleanup;

	// make sure the specified file system object is really a directory
	if (!BTST(pb.hFileInfo.ioFlAttrib, bDirectoryAttr))
		goto cleanup;

	// find the directory ID of the trash folder
	if (FindFolder(pSpec->vRefNum, kTrashFolderType, kDontCreateFolder, &trashVRefNum, &trashDirID)
		!= noErr)
		goto cleanup;

	// compare the two directory IDs: if they're the same, the drop location is the trash
	if (pb.dirInfo.ioDrDirID == trashDirID)
		draggedToTrash = true;

cleanup:
	// clean up
	AEDisposeDesc(&dropLocation);
	AEDisposeDesc(&coercedDropLocation);

	return draggedToTrash;
} // WEDraggedToTrash

pascal OSErr _WEDrag(Point mouseLoc, EventModifiers modifiers, unsigned long clickTime, WEHandle hWE)
{
	WEPtr pWE = *hWE;
	DragReference theDrag = kNullDrag;
	RgnHandle dragRgn = NULL;
	RgnHandle tmpRgn = NULL;
	EventRecord theEvent;
	Rect dragBounds;
	Point portDelta;
	GrafPtr savePort;
	OSErr err;
#if WASTE_OBJECTS
	WEObjectDescHandle hObjectDesc;
#endif

	// set up the port
	GetPort(&savePort);
	SetPort(pWE->port);
	
	// turn the cursor into an arrow
	SetCursor(&qd.arrow);

	// fabricate an EventRecord for TrackDrag
	theEvent.what = mouseDown;
	theEvent.message = 0;
	theEvent.when = clickTime;
	theEvent.where = mouseLoc;
	LocalToGlobal(&theEvent.where);
	theEvent.modifiers = modifiers;

	// before seeing the dotted outline, the user must move the mouse a certain
	// distance away from the initial mouse location; this allows a short click in the selection
	// area to set the insertion point instead of starting a drag-and-drop sequence
	err = weNoDragErr;
	if (!WaitMouseMoved(theEvent.where))
		goto cleanup;

	// create a drag object
	if ((err = NewDrag(&theDrag)) != noErr)
		goto cleanup;

	WEASSERT(theDrag != kNullDrag, "\pZero is a valid drag reference (??)");

#if WASTE_OBJECTS
	// if the selection range consists of an embedded object,
	// then use its object type as flavor type
	if (WEGetSelectedObject(&hObjectDesc, hWE) == noErr)
	{
		FlavorType theType;
		Handle theData = NULL;
		Boolean disposeData;
		
		if ((err = _WEStreamObject(weToDrag, &theType, &theData, &disposeData, hObjectDesc)) != noErr)
			goto cleanup;

		if ((err = AddDragItemFlavor(theDrag, (ItemReference) hWE, theType, NULL, 0, 0)) != noErr)
			goto cleanup;
	}
	else
#endif
	{
	
		// add a 'TEXT' flavor to the drag
		if ((err = AddDragItemFlavor(theDrag, (ItemReference)hWE, kTypeText, NULL, 0, 0)) != noErr)
			goto cleanup;

		// add a 'styl' flavor to the drag
		if ((err = AddDragItemFlavor(theDrag, (ItemReference)hWE, kTypeStyles, NULL, 0, 0)) != noErr)
			goto cleanup;

#if WASTE_OBJECTS
		// add a 'SOUP' flavor to the drag
		if ((err = AddDragItemFlavor(theDrag, (ItemReference)hWE, kTypeSoup, NULL, 0, 0)) != noErr)
			goto cleanup;
#endif

	}

	// since we didn't provide the flavor data for any of the above flavors,
	// we need supply a data send callback
	if (_weFlavorSender == NULL)
		_weFlavorSender = NewDragSendDataProc(_WESendFlavor);

	if ((err = SetDragSendProc(theDrag, _weFlavorSender, 0)) != noErr)
		goto cleanup;

	// get hilite region
	dragRgn = WEGetHiliteRgn(pWE->selStart, pWE->selEnd, hWE);

	// we need just the outline of this region
	tmpRgn = NewRgn();
	CopyRgn(dragRgn, tmpRgn);
	InsetRgn(tmpRgn, 1, 1);
	DiffRgn(dragRgn, tmpRgn, dragRgn);
	DisposeRgn(tmpRgn);

	// and we need it in global coordinates
	portDelta.v = 0;
	portDelta.h = 0;
	LocalToGlobal(&portDelta);
	OffsetRgn(dragRgn, portDelta.h, portDelta.v);

	// set the bounds of the drag
	dragBounds = (*dragRgn)->rgnBBox;
	if ((err = SetDragItemBounds(theDrag, (ItemReference)hWE, &dragBounds)) != noErr)
		goto cleanup;

	// stash drag reference in currentDrag so WETrackDrag and WEReceiveDrag
	// can tell whether a given drag originated from this WE instance
	pWE->currentDrag = theDrag;

	// track the drag
	err = TrackDrag(theDrag, &theEvent, dragRgn);
	pWE->currentDrag = kNullDrag;
	if (err != noErr)
		goto cleanup;

	// if the selection was dragged to the trash and the option key wasn't held down
	// and if the instance is editable, delete the selection
	if (!BTST(pWE->features, weFReadOnly))
	{
		if (WEDraggedToTrash(theDrag))
		{
			if (!_WEIsOptionDrag(theDrag))
			{
				if ((err = WEDelete(hWE)) != noErr)
					goto cleanup;
			}
		}
	}

	// clear result code
	err = noErr;

cleanup:
	// dispose of the drag
	if (theDrag != (DragReference) kNullDrag)
	{
		DisposeDrag(theDrag);
	}

	// dispose of the drag region
	if (dragRgn != NULL)
		DisposeRgn(dragRgn);

	// restore the port
	SetPort(savePort);

	// return result code
	return err;
} // _WEDrag

#if WASTE_IC_SUPPORT

pascal void _WEResolveURL(EventModifiers modifiers, long urlStart, long urlEnd, WEHandle hWE)
{
	WEPtr pWE = *hWE;			// assume WE record is already locked
	Str255 hint;
	FourCharCode signature;
	ProcessSerialNumber psn;
	ProcessInfoRec info;
	ICInstance inst;
	ICError err;
	long junkLong;
	Handle hURL = NULL;
	Boolean saveTextLock;

	// get the hint string IC will use to parse slack URLs
	hint[0] = 0;
	if (pWE->hURLHint != NULL)
	{
		BlockMoveData(*pWE->hURLHint, hint, StrLength(*pWE->hURLHint) + 1);
	}

	// find out the signature of the current process
	signature = '\?\?\?\?';
	psn.highLongOfPSN = 0;
	psn.lowLongOfPSN = 0;
	if (GetCurrentProcess(&psn) == noErr)
	{
		BLOCK_CLR(info);
		info.processInfoLength = sizeof(info);
		if (GetProcessInformation(&psn, &info) == noErr)
			signature = info.processSignature;
	}

	if (ICStart(&inst, signature) == noErr)
	{
		if (ICFindConfigFile(inst, 0, NULL) == noErr)
		{
			saveTextLock = _WESetHandleLock(pWE->hText, true);
			
			// cmd + option click just highlights the URL,
			// without actually resolving it
			if (modifiers & optionKey)
			{
				hURL = NewHandle(0);
				err = ICParseURL(inst, hint, *pWE->hText, WEGetTextLength(hWE), &urlStart, &urlEnd, hURL);
				_WEForgetHandle(&hURL);
			}
			else
			{
				err = ICLaunchURL(inst, hint, *pWE->hText, WEGetTextLength(hWE), &urlStart, &urlEnd);
			}
			_WESetHandleLock(pWE->hText, saveTextLock);
			WESetSelection(urlStart, urlEnd, hWE);

			// flash selection if successful (unless option key was down)
			if ((err == noErr) && !(modifiers & optionKey))
			{
				Delay(5, &junkLong);
				WEDeactivate(hWE);
				Delay(5, &junkLong);
				WEActivate(hWE);
			}
		}
		ICStop(inst);
	}			
}

#endif	// WASTE_IC_SUPPORT

pascal void WEClick(Point mouseLoc, EventModifiers modifiers, unsigned long clickTime, WEHandle hWE)
{
	WEPtr pWE;
	LongPt thePoint;
	long offset, anchor;
	long rangeStart, rangeEnd;
	char edge;
	Boolean isMultipleClick;
	Boolean saveWELock;
#if WASTE_OBJECTS
	WEObjectDescHandle hObjectDesc;
#endif
#if WASTE_IC_SUPPORT
	long urlStart, urlEnd;
#endif

	// stop any ongoing inline input session
	WEStopInlineSession(hWE);

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

#if WASTE_IC_SUPPORT
	// remember the selection range before the click
	urlStart = pWE->selStart;
	urlEnd = pWE->selEnd;
#endif

	// hide the caret if it's showing
	if (BTST(pWE->flags, weFCaretVisible))
		_WEBlinkCaret(hWE);

	// find click offset
	WEPointToLongPoint(mouseLoc, &thePoint);
	offset = WEGetOffset(&thePoint, &edge, hWE);

	// determine whether this click is part of a sequence
	// a single click inside an object selects it, so it's like a double click in a word
	isMultipleClick = ((clickTime < pWE->clickTime + GetDblTime()) && (offset == pWE->clickLoc));

	// remember click time, click offset and edge value
	pWE->clickTime = clickTime;
	pWE->clickLoc = offset;
	pWE->clickEdge = edge;

#if WASTE_OBJECTS
	// when selected, embedded objects can intercept mouse clicks
	if (WEGetSelectedObject(&hObjectDesc, hWE) == noErr)
		if (_WEOffsetInRange(offset, edge, pWE->selStart, pWE->selEnd))
			if (_WEClickObject(mouseLoc, modifiers + isMultipleClick, clickTime, hObjectDesc))
					goto cleanup;
#endif

	if ((modifiers & shiftKey) == 0)
	{

		// is this click part of a sequence or is it a single click?
		if (isMultipleClick)
		{
			pWE->clickCount++;

			// a double (triple) click creates an anchor-word (anchor-line)
			if (pWE->clickCount > 1)
				WEFindLine(offset, edge, &pWE->anchorStart, &pWE->anchorEnd, hWE);
			else
				WEFindWord(offset, edge, &pWE->anchorStart, &pWE->anchorEnd, hWE);

			offset = pWE->anchorStart;
		}
		else
		{
			// single-click
			// if the Drag Manager is available and the click went in the selection range,
			// this click may be the beginning of a drag gesture
			if (BTST(pWE->flags, weFHasDragManager) && BTST(pWE->features, weFDragAndDrop))
				if (_WEOffsetInRange(offset, edge, pWE->selStart, pWE->selEnd))
					if (_WEDrag(mouseLoc, modifiers, clickTime, hWE) != weNoDragErr)
						goto cleanup;

			pWE->clickCount = 0;
			anchor = offset;
		}
	}
	else
	{
	
		// if the shift key was down, use the old anchor offset found with the previous click
		anchor = BTST(pWE->flags, weFAnchorIsEnd) ? pWE->selEnd : pWE->selStart;
	}

	// set the weFMouseTracking bit while we track the mouse
	BSET(pWE->flags, weFMouseTracking);

	// MOUSE TRACKING LOOP
	do
	{

		// get text offset corresponding to mouse position
		WEPointToLongPoint(mouseLoc, &thePoint);
		offset = WEGetOffset(&thePoint, &edge, hWE);

		// if we're selecting words or lines, pin offset to a word or line boundary
		if (pWE->clickCount > 0)
		{
			if (pWE->clickCount > 1)
				WEFindLine(offset, edge, &rangeStart, &rangeEnd, hWE);
			else
				WEFindWord(offset, edge, &rangeStart, &rangeEnd, hWE);

			// choose the word/line boundary and the anchor that are farthest away from each other
			if (offset > pWE->anchorStart)
			{
				anchor = pWE->anchorStart;
				offset = rangeEnd;
			}
			else
			{
				offset = rangeStart;
				anchor = pWE->anchorEnd;
			}
		}
		else
		{
			// if the point is in the middle of an object, the selection should include it
			if (edge == kObjectEdge)
				offset++;
		}

		// set the selection range from anchor point to current offset
		WESetSelection(anchor, offset, hWE);

		// call the click loop callback, if any
		if (pWE->clickLoop != NULL)
			if (!CallWEClickLoopProc(hWE, pWE->clickLoop))
				break;

		// update mouse position
		GetMouse(&mouseLoc);

	} while(WaitMouseUp());

	// clear the weFMouseTracking bit
	BCLR(pWE->flags, weFMouseTracking);

	// redraw the caret immediately if the selection range is empty
	if (anchor == offset)
		_WEBlinkCaret(hWE);

#if WASTE_IC_SUPPORT
	if (modifiers & cmdKey)
	{
		// command+clicking a URL tries to resolve it
		// we normally ask IC to parse the text surrounding the clicked point,
		// but if a selection already existed prior to the click, we pass
		// that to IC rather than forcing a re-parse
		if ((anchor != offset) || (anchor < urlStart) || (anchor > urlEnd))
		{
			urlStart = anchor;
			urlEnd = offset;
		}
		_WEResolveURL(modifiers, urlStart, urlEnd, hWE);
	}
#endif

cleanup:
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

} // WEClick
