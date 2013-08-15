
/*
 *	WEHighLevelEditing.c
 *
 *	WASTE PROJECT
 *  High-Level Editing Routines
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */


#include "WASTEIntf.h"
#include "CarbonCompatUtil.h"

typedef struct DoubleByte {
	char firstByte;
	char secondByte;
} DoubleByte;

pascal void _WEPushAction(WEActionHandle hAction)
{
	WEPtr pWE = *((*hAction)->hOwner);
	WEActionHandle hLast;

	// find the last action in the given stack
	for ( hLast = hAction; (*hLast)->hNext != NULL; hLast = (*hLast)->hNext )
		;

	// prepend hAction in front of the action stack
	(*hLast)->hNext = pWE->hActionStack;
	pWE->hActionStack = hAction;
}

pascal OSErr _WENewAction(long rangeStart, long rangeEnd, long newTextLength,
							WEActionKind actionKind, WEActionFlags actionFlags,
							WEHandle hWE, WEActionHandle *hAction)
{
	WEActionPtr pAction;
	OSErr err;

	// allocate a new action record
	if ((err = _WEAllocate(sizeof(WEAction), kAllocClear, (Handle *)hAction)) != noErr) 
		goto cleanup1;

	// lock it down
	HLock((Handle) *hAction);
	pAction = **hAction;

	// fill in the fields
	pAction->hOwner = hWE;
	pAction->delRangeStart = rangeStart;
	pAction->delRangeLength = newTextLength;
	pAction->insRangeLength = rangeEnd - rangeStart;
	pAction->actionKind = actionKind;
	pAction->actionFlags = actionFlags;

	// remember selection range
	WEGetSelection(&pAction->hiliteStart, &pAction->hiliteEnd, hWE);

	// allocate a handle to hold the text to be saved, unless otherwise specified
	if ((actionFlags & weAFDontSaveText) == 0) 
	{
		if ((err = _WEAllocate(0, kAllocTemp, &pAction->hText)) != noErr) 
			goto cleanup1;
	}

	// allocate a handle to hold the styles to be saved, unless otherwise specified
	if ((actionFlags & weAFDontSaveStyles) == 0) 
	{
		if ((err = _WEAllocate(0, kAllocTemp, &pAction->hStyles)) != noErr) 
			goto cleanup1;
	}

#if WASTE_OBJECTS
	// allocate a handle to hold the "soup" to be saved, unless otherwise specified
	if ((actionFlags & weAFDontSaveSoup) == 0) 
	{
		if ((err = _WEAllocate(0, kAllocTemp, &pAction->hSoup)) != noErr) 
			goto cleanup1;
	}
#endif
	
	// make a copy of text range
	if ((err = WECopyRange(rangeStart, rangeEnd, pAction->hText,
			(Handle) pAction->hStyles, pAction->hSoup, hWE)) != noErr)
		goto cleanup1;

	// unlock action record
	HUnlock((Handle) *hAction);

	// skip clean-up section
	goto cleanup0;

cleanup1:
	// clean up
	_WEForgetHandle(&pAction->hText);
	_WEForgetHandle(&pAction->hStyles);
#if WASTE_OBJECTS
	_WEForgetHandle(&pAction->hSoup);
#endif
	_WEForgetHandle((Handle *)hAction);

cleanup0:
	// return result code
	return err;

}

pascal void _WEDisposeAction(WEActionHandle hAction)
{
	WEActionPtr pAction;
	WEActionHandle hNext;

	for ( ; hAction != NULL; hAction = hNext )
	{
		// lock the action record
		HLock((Handle) hAction);
		pAction = *hAction;
		hNext = pAction->hNext;

		// throw away text, styles and soup
		_WEForgetHandle(&pAction->hText);
		_WEForgetHandle(&pAction->hStyles);
#if WASTE_OBJECTS
		_WEForgetHandle(&pAction->hSoup);
#endif

		// throw away the action record itself
		DisposeHandle((Handle) hAction);
	}
}

pascal void _WEForgetAction(WEActionHandle *hAction)
{
	WEActionHandle theAction;

	theAction = *hAction;
	if (theAction != NULL) 
	{
		*hAction = NULL;
		_WEDisposeAction(theAction);
	}
}

pascal OSErr _WEDoAction(WEActionHandle hAction)
{
	WEActionHandle hRedoAction;
	WEActionPtr pAction;
	WEHandle hWE;
	WEPtr pWE;
	long offset, delOffset, insOffset;
	long redrawStart, redrawEnd;
	Boolean saveActionLock, saveWELock, saveTextLock;
	OSErr err;

	// sanity check: make sure hAction isn't NULL
	if (hAction == NULL) 
	{
		return nilHandleErr;
	}
	
	// get handle to associated WE instance
	hWE = (*hAction)->hOwner;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// return an error code if this instance is read-only
	err = weReadOnlyErr;
	if (BTST(pWE->features, weFReadOnly))
		goto cleanup;

	// stop any ongoing inline input session
	WEStopInlineSession(hWE);

	// hide selection highlighting and the caret
	_WEHiliteRange(pWE->selStart, pWE->selEnd, hWE);
	if (BTST(pWE->flags, weFCaretVisible))
		_WEBlinkCaret(hWE);

	redrawStart = LONG_MAX;
	redrawEnd = 0;

	for ( ; hAction != NULL; hAction = (*hAction)->hNext )
	{
		// lock the action record
		saveActionLock = _WESetHandleLock((Handle) hAction, true);
		pAction = *hAction;
		offset = pAction->delRangeStart;
		delOffset = offset + pAction->delRangeLength;
		insOffset = offset + pAction->insRangeLength;

		// if undo support is enabled, save the range to be affected by this action
		if (BTST(pWE->features, weFUndoSupport)) 
		{
			if (_WENewAction(offset, delOffset, pAction->insRangeLength, pAction->actionKind,
				(pAction->actionFlags ^ weAFIsRedo), hWE, &hRedoAction) == noErr) 
			{
				_WEPushAction(hRedoAction);
			}
		}

		if (pAction->hText != NULL) 
		{
			// delete the range to replace
			if ((err = _WEDeleteRange(offset, delOffset, hWE)) != noErr) 
				goto cleanup;

			// insert the saved text
			saveTextLock = _WESetHandleLock(pAction->hText, true);
			err = _WEInsertText(offset, *pAction->hText, pAction->insRangeLength, hWE);
			_WESetHandleLock(pAction->hText, saveTextLock);
			if (err != noErr) 
				goto cleanup;
		}

		// apply the saved styles, if any
		if (pAction->hStyles != NULL) 
		{
			if ((err = _WEApplyStyleScrap(offset, insOffset, (StScrpHandle)pAction->hStyles, hWE)) != noErr) 
				goto cleanup;
		}

#if WASTE_OBJECTS
		// the same goes for the soup
		if (pAction->hSoup != NULL) 
		{
			if ((err = _WEApplySoup(offset, pAction->hSoup, hWE)) != noErr) 
				goto cleanup;
		}
#endif

		// adjust redraw range
		if (offset < redrawStart) 
			redrawStart = offset;
		if (insOffset > redrawEnd) 
			redrawEnd = insOffset;

		// unlock action record
		_WESetHandleLock((Handle) hAction, saveActionLock);

	} // for

	// restore the original selection range
	pWE->selStart = pAction->hiliteStart;
	pWE->selEnd = pAction->hiliteEnd;

	// redraw the text
	if ((err = _WERedraw(redrawStart, redrawEnd, hWE)) != noErr) 
		goto cleanup;

	// clear result code
	err = noErr;

cleanup:
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return result code
	return err;
}

pascal OSErr WEUndo(WEHandle hWE)
{
	WEPtr pWE;
	WEActionHandle hAction;
	Boolean saveWELock;
	OSErr err;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;
	
	// "detach" the action stack from the WE instance
	hAction = pWE->hActionStack;
	pWE->hActionStack = NULL;

	if (hAction != NULL)
	{

		// undoing a change _decrements_ the modification count;
		// redoing the change increments it again
		if (((*hAction)->actionFlags & weAFIsRedo) != 0)
			pWE->modCount++;
		else
			pWE->modCount--;

		// perform the action...
		err = _WEDoAction(hAction);

		// ...and throw it away
		_WEDisposeAction(hAction);
	}
	else
	{
		// return an error code if the undo buffer is empty
		err = weCantUndoErr;
	}

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	return err;
}

pascal void WEClearUndo(WEHandle hWE)
{
	// dispose of the action chain associated with the given WE instance
	_WEForgetAction(&(*hWE)->hActionStack);
}

pascal WEActionKind WEGetUndoInfo(Boolean *redoFlag, WEHandle hWE)
{
	WEActionHandle hAction;
	WEActionKind theKind = weAKNone;
	Boolean theFlag = false;
	
	if ((hAction = (*hWE)->hActionStack) != NULL)
	{
		theKind = (*hAction)->actionKind;
		theFlag = (((*hAction)->actionFlags & weAFIsRedo) != 0);
	}
	
	if (redoFlag != NULL)
		*redoFlag = theFlag;
	return theKind;
}

pascal unsigned long WEGetModCount(WEHandle hWE)
{
	return (*hWE)->modCount;
}

pascal void WEResetModCount(WEHandle hWE)
{
	(*hWE)->modCount = 0;
	WEClearUndo(hWE);
}

pascal void _WEAdjustUndoRange(long moreBytes, WEHandle hWE)
{
	WEActionHandle hAction;

	if ((hAction = (*hWE)->hActionStack) != NULL) 
		(*hAction)->delRangeLength += moreBytes;
}

pascal OSErr _WETypeChar(char theByte, WEHandle hWE)
{
	WEPtr pWE;
	DoubleByte db;
	long offset, endOffset, charLength;
	OSErr err;

	pWE = *hWE;					// the WE record must be already locked
	charLength = 1;				// assume 1-byte character by default
	db.firstByte = theByte;
	offset = pWE->selStart;

	// delete current selection, if any
	if ((err = _WEDeleteRange(offset, pWE->selEnd, hWE)) != noErr) 
		goto cleanup2;
	
	pWE->selEnd = offset; // needed in case we take a premature exit

	// make sure the font script is synchronized with the keyboard script
	_WESynchNullStyle(hWE);

	if (BTST(pWE->flags, weFDoubleByte))
	{

		// special processing for double-byte characters
		if (pWE->firstByte != 0) 
		{

			// if this byte is the second half of a double-byte character,
			// insert the two bytes at the same time (flush the double-byte cache)
			db.firstByte = pWE->firstByte;
			db.secondByte = theByte;
			charLength = 2;
			pWE->firstByte = 0;
		}
		else
		{

			// if theByte is the first half of a double-byte character, just cache it and exit
			if (CallWECharByteProc(&theByte, 0, FontToScript(pWE->nullStyle.runStyle.tsFont),
				hWE, pWE->charByteHook) == smFirstByte)
			{
				pWE->firstByte = theByte;
				return noErr;
			}
		}

	} // if double-byte script installed

	// insert the new character into the text
	if ((err = _WEInsertText(offset, (Ptr) &db, charLength, hWE)) != noErr) 
		goto cleanup2;

	// adjust undo buffer for the new character
	_WEAdjustUndoRange(charLength, hWE);

	// invalid the null style
	BCLR(pWE->flags, weFUseNullStyle);

	// move the insertion point after the new character
	endOffset = offset + charLength;
	pWE->selStart = endOffset;
	pWE->selEnd = endOffset;

	// redraw the text
	if ((err = _WERedraw(offset, endOffset, hWE)) != noErr) 
		goto cleanup2;

cleanup1:
	// clear result code
	err = noErr;

cleanup2:
	// return result code
	return err;

}

pascal OSErr _WEBackspace(WEHandle hWE)
{
	// this routine is called by WEKey to handle the backspace key

	WEPtr pWE = *hWE;	// assume WE record is already locked
	WEActionPtr pAction;
	long rangeStart, rangeEnd, charLength;
	WERunInfo runInfo;
	DoubleByte db;
	Boolean saveActionLock;
	OSErr err;

	// calculate the text range to delete
	// if the selection is non-empty, delete that
	rangeStart = pWE->selStart;
	rangeEnd = pWE->selEnd;
	if (rangeStart == rangeEnd) 
	{

		// otherwise the selection is an insertion point
		// do nothing if insertion point is at the beginning of the text
		if (rangeStart == 0)
			return noErr; 

		// determine the byte-type of the character preceding the insertion point
		charLength = (WECharByte(rangeStart - 1, hWE) == smSingleByte) ? 1 : 2;
		rangeStart -= charLength;

		if (pWE->hActionStack != NULL) 
		{
			// UNDO SUPPORT FOR BACKSPACES

			// lock the action record
			saveActionLock = _WESetHandleLock((Handle) pWE->hActionStack, true);
			pAction = *pWE->hActionStack;

			// backspaces over the newly entered text aren't a problem
			if (pAction->delRangeLength > 0) 
				pAction->delRangeLength -= charLength;
			else
			{

				// the hard part comes when backspacing past the new text because
				// the user is about to delete a character not included in the block we saved
				db.firstByte = WEGetChar(rangeStart, hWE);
				if (charLength == 2)
					db.secondByte = WEGetChar(rangeStart + 1, hWE);

				// prepend the character to be deleted to the beginning of our saved text handle
				if ((err = _WEInsertBlock(pAction->hText, &db, charLength, 0)) != noErr)
					return err;

				// adjust internal counters
				pAction->insRangeLength += charLength;
				pAction->delRangeStart -= charLength;

				// get style run info associated with the about-to-be-deleted character
				WEGetRunInfo(rangeStart, &runInfo, hWE);

				// prepend a new style element to our style scrap, if necessary
				if ((err = _WEPrependStyle(pAction->hStyles, &runInfo, charLength)) != noErr)
					return err; 

#if WASTE_OBJECTS
				// do the same with our object "soup"
				if ((err = _WEPrependObject(pAction->hSoup, &runInfo, charLength)) != noErr)
					return err;
#endif
			
			} // if deleting old text

			// unlock the action record
			_WESetHandleLock((Handle) pWE->hActionStack, saveActionLock);

		} // if undo support is enabled
	} // if selection is empty

	if ((err = _WEDeleteRange(rangeStart, rangeEnd, hWE)) != noErr)
		return err;

	// keep track of current selection range
	pWE->selStart = rangeStart;
	pWE->selEnd = rangeStart;

	// redraw the text
	err = _WERedraw(rangeStart, rangeStart, hWE);
	
	return err;
}

pascal OSErr _WEForwardDelete(WEHandle hWE)
{
	
	// this routine is called by WEKey to handle the forward delete key

	WEPtr pWE = *hWE;	// assume WE record is already locked
	WEActionPtr pAction;
	long rangeStart, rangeEnd, charLength;
	WERunInfo runInfo;
	DoubleByte db;
	Boolean saveActionLock;
	OSErr err;

	// calculate the text range to delete
	// if the selection is non-empty, delete that
	rangeStart = pWE->selStart;
	rangeEnd = pWE->selEnd;
	if (rangeStart == rangeEnd)
	{
		
		// otherwise the selection is an insertion point
		// do nothing if insertion point is at the end of the text
		if (rangeStart == pWE->textLength)
			return noErr;

		// determine the byte-type of the character following the insertion point
		charLength = (WECharByte(rangeStart, hWE) == smSingleByte) ? 1 : 2;
		rangeEnd = rangeStart + charLength;

		if (pWE->hActionStack != NULL)
		{

			// UNDO SUPPORT FOR FORWARD DELETE

			// lock the action record
			saveActionLock = _WESetHandleLock((Handle) pWE->hActionStack, true);
			pAction = *pWE->hActionStack;

			// make a copy of the character about to be deleted
			db.firstByte = WEGetChar(rangeStart, hWE);
			if (charLength == 2)
				db.secondByte = WEGetChar(rangeStart + 1, hWE);

			// append it to the end of our saved text handle
			PtrAndHand(&db, pAction->hText, charLength);
			if ((err = MemError()) != noErr)
				return err;

			// get style run info associated with the about-to-be-deleted character
			WEGetRunInfo(rangeStart, &runInfo, hWE);

			// append a new style element to our style scrap, if necessary
			if ((err = _WEAppendStyle(pAction->hStyles, &runInfo, pAction->insRangeLength)) != noErr)
				return err;

#if WASTE_OBJECTS
			// do the same with our object soup
			if ((err = _WEAppendObject(pAction->hSoup, &runInfo, pAction->insRangeLength)) != noErr)
				return err;
#endif

			// adjust internal counters
			pAction->insRangeLength += charLength;

			// unlock the action record
			_WESetHandleLock((Handle) pWE->hActionStack, saveActionLock);

		} // if undo support is enabled
	} // if selection is empty

	if ((err = _WEDeleteRange(rangeStart, rangeEnd, hWE)) != noErr)
		return err;

	// keep track of current selection range
	pWE->selStart = rangeStart;
	pWE->selEnd = rangeStart;

	// redraw the text
	err = _WERedraw(rangeStart, rangeStart, hWE);

	return err;
}

pascal Boolean WEIsTyping(WEHandle hWE)
{
	// return true if we're tracking a typing sequence in the specified WE instance

	WEPtr pWE = *hWE;
	WEActionPtr pAction;

	// there must be an undo buffer
	if (pWE->hActionStack == NULL)
		return false;
	
	pAction = *pWE->hActionStack;

	// the action kind must be "typing" and the redo flag must be clear
	if ((pAction->actionKind != weAKTyping) || ((pAction->actionFlags & weAFIsRedo) != 0))
		return false;

	// finally, the selection range mustn't have moved since the last WEKey
	if ((pWE->selStart != pWE->selEnd) ||
		(pWE->selStart != pAction->delRangeStart + pAction->delRangeLength))
		return false;
	
	return true;
}

pascal void WEKey(short key, EventModifiers modifiers, WEHandle hWE)
{
	WEPtr pWE;
	WEActionHandle hAction;
	Boolean saveWELock;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// hide the caret if it's showing
	if (BTST(pWE->flags, weFCaretVisible))
		_WEBlinkCaret(hWE);

	// hide the cursor (it will show again as soon as it's moved)
	ObscureCursor();

	// dispatch on key class (arrow keys, printable characters, backspace)
	if ((key >= kArrowLeft) && (key <= kArrowDown))
	{
		_WEDoArrowKey(key, modifiers, hWE);
	}
	else
	{

		// non-arrow keys modify the text, so make sure editing is allowed
		if (!BTST(pWE->features, weFReadOnly))
		{
			// are we tracking a typing sequence?
			if (!WEIsTyping(hWE))
			{
				// nope;  start a new one
				// increment modification count
				pWE->modCount++;

				// if undo support is enabled, create a new action to keep track of typing
				if (BTST(pWE->features, weFUndoSupport))
				{
					WEClearUndo(hWE);
					if (_WENewAction(pWE->selStart, pWE->selEnd, 0, weAKTyping, 0, hWE, &hAction) == noErr)
					{
						_WEPushAction(hAction);
					}
				}
			} // if WEIsTyping

			if (key == kBackspace)
			{
				_WEBackspace(hWE);
			}
			else if (key == kForwardDelete)
			{
				_WEForwardDelete(hWE);
			}
			else
			{
				_WETypeChar(key, hWE);
			}
		} // if not read-only
	}

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);
}

pascal OSErr WEInsert(Ptr textPtr, long textLength, StScrpHandle hStyles, Handle hSoup, WEHandle hWE)
{
#if !WASTE_OBJECTS
#pragma unused(hSoup)
#endif
	WEPtr pWE;
	long offset, endOffset;
	WEActionHandle hAction;
	short intPasteAction;
	Boolean saveWELock;
	char space = kSpace;
	OSErr err;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;
	offset = pWE->selStart;

	// return an error code if this instance is read-only
	err = weReadOnlyErr;
	if (BTST(pWE->features, weFReadOnly))
		goto cleanup;

	// stop any ongoing inline input session
	WEStopInlineSession(hWE);

	// increment modification count
	pWE->modCount++;

	// if undo support is enabled, save current selection range
	if (BTST(pWE->features, weFUndoSupport)) 
	{
		WEClearUndo(hWE);
		if (_WENewAction(offset, pWE->selEnd, textLength, weAKUnspecified, 0, hWE, &hAction) == noErr)
		{
			_WEPushAction(hAction);
		}
	}

	// delete current selection
	if ((err = _WEDeleteRange(offset, pWE->selEnd, hWE)) != noErr) 
		goto cleanup;

	// insert the new text at the insertion point
	if ((err = _WEInsertText(offset, textPtr, textLength, hWE)) != noErr) 
		goto cleanup;
	endOffset = offset + textLength;

	if (hStyles != NULL) 
	{

		// if a style scrap was supplied, apply it to the newly inserted text
		if ((err = _WEApplyStyleScrap(offset, endOffset, hStyles, hWE)) != noErr) 
			goto cleanup;
	}

#if WASTE_OBJECTS
	if (hSoup != NULL)
	{
		// if an object soup was supplied, apply it to the newly inserted text
		if ((err = _WEApplySoup(offset, hSoup, hWE)) != noErr) 
			goto cleanup;
	}
#endif

	// determine whether an extra space should be added before or after the inserted text
	intPasteAction = _WEIntelligentPaste(offset, endOffset, hWE);

	// add the extra space, if necessary
	if (intPasteAction != weDontAddSpaces) 
	{
		if (intPasteAction == weAddSpaceOnLeftSide)
		{
			err = _WEInsertText(offset, &space, sizeof(space), hWE);
		}
		else
		{
			err = _WEInsertText(endOffset, &space, sizeof(space), hWE);
		}
		if (err != noErr)
			goto cleanup;
		endOffset++;

		// adjust undo buffer (if any) for the extra space
		_WEAdjustUndoRange(sizeof(space), hWE);
	}

	// invalid the null style
	BCLR(pWE->flags, weFUseNullStyle);

	// move the insertion point at the end of the inserted text
	pWE->selStart = endOffset;
	pWE->selEnd = endOffset;

	// redraw the text
	if ((err = _WERedraw(offset, endOffset, hWE)) != noErr) 
		goto cleanup;

	// clear result code
	err = noErr;

cleanup:
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return result code
	return err;
}

#if WASTE_OBJECTS

pascal OSErr WEInsertObject(FlavorType objectType, Handle objectDataHandle, Point objectSize, WEHandle hWE)
{
	WEPtr pWE;
	WEActionHandle hAction;
	long offset, endOffset;
	WETextStyle ts;
	Boolean saveWELock;
	char marker = kObjectMarker;
	OSErr err;

	BLOCK_CLR(ts);

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;
	offset = pWE->selStart;

	// return an error code if this instance is read-only
	err = weReadOnlyErr;
	if (BTST(pWE->features, weFReadOnly))
		goto cleanup;

	// stop any ongoing inline input session
	WEStopInlineSession(hWE);

	// call the 'new' handler to initialize private object storage (if any)
	// and to calculate the default size for this object

	if ((err = _WENewObject(objectType, objectDataHandle, hWE, &ts.tsObject)) != noErr) 
		goto cleanup;

	// use the specified object size, unless it is (0, 0), in which case keep the default size
	if (*((long *) &objectSize) != 0) 
		(*ts.tsObject)->objectSize = objectSize;

	// increment modification count
	pWE->modCount++;

	// if undo support is enabled, save current selection range
	if (BTST(pWE->features, weFUndoSupport)) 
	{
		WEClearUndo(hWE);
		if (_WENewAction(offset, pWE->selEnd, 1, weAKUnspecified, 0, hWE, &hAction) == noErr)
		{
			_WEPushAction(hAction);
		}
	}

	// delete current selection
	if ((err = _WEDeleteRange(offset, pWE->selEnd, hWE)) != noErr) 
		goto cleanup;

	// insert a kObjectMarker character at the insertion point
	if ((err = _WEInsertText(offset, &marker, sizeof(marker), hWE)) != noErr)
		goto cleanup;

	// move the insertion point after the inserted text
	endOffset = offset + 1;
	pWE->selStart = endOffset;
	pWE->selEnd = endOffset;

	// record a reference to the object descriptor in the style table
	err = _WESetStyleRange(offset, endOffset, weDoObject, &ts, hWE);
	ts.tsObject = NULL;
	if (err != noErr) 
		goto cleanup;

	// invalid the null style
	BCLR(pWE->flags, weFUseNullStyle);

	// redraw the text
	if ((err = _WERedraw(offset, endOffset, hWE)) != noErr) 
		goto cleanup;

	// clear result code
	err = noErr;

cleanup:
	// clean up
	_WEForgetHandle((Handle *) &ts.tsObject);

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return result code
	return err;
}

#endif

pascal OSErr WEDelete(WEHandle hWE)
{
	WEPtr pWE;
	WEActionHandle hAction;
	long rangeStart, rangeEnd;
	Boolean saveWELock;
	OSErr err;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// return an error code if this instance is read-only
	err = weReadOnlyErr;
	if (BTST(pWE->features, weFReadOnly))
		goto cleanup;

	// stop any ongoing inline input session
	WEStopInlineSession(hWE);

	// get current selection range
	rangeStart = pWE->selStart;
	rangeEnd = pWE->selEnd;

	// do nothing if the selection range is empty
	if (rangeStart < rangeEnd) 
	{

		// increment modification count
		pWE->modCount++;

		// range extension for intelligent cut-and-paste
		_WEIntelligentCut(&rangeStart, &rangeEnd, hWE);

		// if undo support is enabled, save the range to be deleted
		if (BTST(pWE->features, weFUndoSupport)) 
		{
			WEClearUndo(hWE);
			if (_WENewAction(rangeStart, rangeEnd, 0, weAKClear, 0, hWE, &hAction) == noErr)
			{
				_WEPushAction(hAction);
			}
		}

		// delete the selection range
		if ((err = _WEDeleteRange(rangeStart, rangeEnd, hWE)) != noErr) 
			goto cleanup;

		// reset the selection range
		pWE->selStart = rangeStart;
		pWE->selEnd = rangeStart;

		// redraw the text
		if ((err = _WERedraw(rangeStart, rangeStart, hWE)) != noErr) 
			goto cleanup;

	} // if non-empty selection

	// clear result code
	err = noErr;

cleanup:
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return result code
	return err;
}

pascal OSErr WECut(WEHandle hWE)
{
	OSErr err;

	// first copy...
	if ((err = WECopy(hWE)) != noErr)
		goto cleanup;

	// ... then delete
	if ((err = WEDelete(hWE)) != noErr)
		goto cleanup;

	// change the action kind of the most recent action, if any
	if ((*hWE)->hActionStack != NULL)
	{
		(*(*hWE)->hActionStack)->actionKind = weAKCut;
	}

cleanup:
	// return result code
	return err;
}  // WECut

pascal Boolean WECanPaste(WEHandle hWE)
{
	long scrapOffset;
#if WASTE_OBJECTS
	FlavorType objectType;
	long index;
#endif

	if (!BTST((*hWE)->features, weFReadOnly))
	{
		// return true if the desk scrap contains a text flavor
		if (CCUGetScrap(NULL, kTypeText, &scrapOffset) > 0)
			return true;

#if WASTE_OBJECTS
		// see if the desk scrap contains a flavor matching one of the registered object types
		index = 0;
		while (_WEGetIndObjectType(index, &objectType, hWE) == noErr)
		{
			if (CCUGetScrap(NULL, objectType, &scrapOffset) > 0)
				return true;
			index++;
		} // while
#endif
	} // if not read-only
	return false;
}

pascal OSErr WEPaste(WEHandle hWE)
{
	Handle hItem = NULL;
	Handle hStyles = NULL;
	Handle hSoup = NULL;
	long scrapOffset;
#if WASTE_OBJECTS
	FlavorType objectType;
	long index;
#endif
	OSErr err;

	// allocate a handle to hold a scrap item
	if ((err = _WEAllocate(0, kAllocTemp, &hItem)) != noErr) 
		goto cleanup;

	// look for a text flavor
	if (CCUGetScrap(hItem, kTypeText, &scrapOffset) <= 0) 
	{

#if WASTE_OBJECTS
		// no text: look for a flavor matching one of the registered object types
		index = 0;
		while (_WEGetIndObjectType(index, &objectType, hWE) == noErr)
		{
			if (CCUGetScrap(hItem, objectType, &scrapOffset) > 0) 
			{
				Point objectSize;
				* (long *) &objectSize = 0;
			
				// found a registered type: create a new object out of the tagged data
				err = WEInsertObject(objectType, hItem, objectSize, hWE);

				// if successful, set hItem to NULL so clean-up section won't kill the object data
				if (err == noErr) 
					hItem = NULL;
				goto cleanup;
			}

			// try with next flavor
			index++;
		} // while
#endif

		// nothing pasteable: return an error code
		err = noTypeErr;
		goto cleanup;
	}

#if 0
	// ignore styles when pasting! -- akozar, 201308014
	if (!BTST((*hWE)->features, weFMonoStyled))
	{
		// allocate a handle to hold the style scrap, if any
		if ((err = _WEAllocate(0, kAllocTemp, &hStyles)) != noErr) 
			goto cleanup;
	
		// look for a 'styl' item accompanying the text
		if (CCUGetScrap(hStyles, kTypeStyles, &scrapOffset) <= 0) 
			// forget the handle if nothing was found or an error occurred
			_WEForgetHandle(&hStyles);

#if WASTE_OBJECTS
		// allocate a handle to hold the soup, if any
		if ((err = _WEAllocate(0, kAllocTemp, &hSoup)) != noErr) 
			goto cleanup;
	
		// look for a 'SOUP' item accompanying the text
		if (CCUGetScrap(hSoup, kTypeSoup, &scrapOffset) <= 0) 
			// forget the handle if nothing was found or an error occurred
			_WEForgetHandle(&hSoup);
#endif
	} // if not mono-styled
#endif

	// lock down the text
	HLock(hItem);

	// insert the text
	err = WEInsert(*hItem, GetHandleSize(hItem), (StScrpHandle) hStyles, hSoup, hWE);

cleanup:
	// if successful, change the action kind of the most recent action, if any
	if ((err == noErr) && ((*hWE)->hActionStack != NULL))
	{
		(*(*hWE)->hActionStack)->actionKind = weAKPaste;
	}

	// clean up
	_WEForgetHandle(&hItem);
	_WEForgetHandle(&hStyles);
#if WASTE_OBJECTS
	_WEForgetHandle(&hSoup);
#endif

	// return result code
	return err;
}

pascal OSErr _WESmartSetFont(WEStyleMode mode, const TextStyle *ts, WEHandle hWE)
{
	WEPtr pWE = *hWE;	// assume WE record is already locked
	ScriptCode script;
	long runIndex;
	long rangeStart, rangeEnd;
	WERunInfo runInfo;
	GrafPtr savePort;
	short saveFont;
	OSErr err;
	
	// set up the graphics port
	GetPort(&savePort);
	SetPort(pWE->port);
	saveFont = GetPortTextFont(pWE->port);

	// get the script corresponding to the font we're applying
	script = FontToScript(ts->tsFont);

	// walk through the style runs encompassing the selection range
	runIndex = _WEOffsetToRun(pWE->selStart, hWE);
	do {
		_WEGetIndStyle(runIndex, &runInfo, hWE);
		
		if (pWE->selStart > runInfo.runStart)
			rangeStart = pWE->selStart;
		else
			rangeStart = runInfo.runStart;
		
		if (pWE->selEnd < runInfo.runEnd)
			rangeEnd = pWE->selEnd;
		else
			rangeEnd = runInfo.runEnd;

		// does this style run belong to the same script we're applying?
		if (FontToScript(runInfo.runAttrs.runStyle.tsFont) == script)
		{
			if ((err = _WESetStyleRange(rangeStart, rangeEnd, weDoFont, (WETextStyle *) ts, hWE)) != noErr)
				goto cleanup;
			runIndex = _WEOffsetToRun(runInfo.runEnd, hWE);
		}
		else if ((mode & weDoExtractSubscript) != 0)
		{
			long runLength;
			long subrunLength;
			ScriptRunStatus runStatus;
			
			// FindScriptRun takes an implicit parameter through the txFont field of thePort
			TextFont(runInfo.runAttrs.runStyle.tsFont);
			
			runLength = rangeEnd - rangeStart;
			while (runLength > 0)
			{
				// lock text handle
				Boolean saveTextLock = _WESetHandleLock(pWE->hText, true);
	
				// look for blocks of subscript text
				runStatus = FindScriptRun(*pWE->hText + rangeStart, runLength, &subrunLength);
	
				// unlock text handle
				_WESetHandleLock(pWE->hText, saveTextLock);
	
				if (runStatus.script == script)
				{
					// "extract" subscript text
					if ((err = _WESetStyleRange(rangeStart, rangeStart + subrunLength, weDoFont, (WETextStyle *) ts, hWE)) != noErr)
						goto cleanup;
				}
				rangeStart += subrunLength;
				runLength -= subrunLength;
			}
			runIndex = _WEOffsetToRun(runInfo.runEnd, hWE);
		}
		else
			runIndex++;

	} while (runInfo.runEnd < pWE->selEnd);

cleanup:
	// restore the port
	TextFont(saveFont);
	SetPort(savePort);
	
	// return result code
	return err;
}

pascal OSErr WESetStyle(WEStyleMode mode, const TextStyle *ts, WEHandle hWE)
{
	WEPtr pWE;
	WEActionHandle hAction;
	ScriptCode fontScript;
	Boolean saveWELock;
	OSErr err;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// return an error code if this instance is read-only
	err = weReadOnlyErr;
	if (BTST(pWE->features, weFReadOnly))
		goto cleanup;

	// stop any ongoing inline input session
	WEStopInlineSession(hWE);

	if (pWE->selStart == pWE->selEnd) 
	{

		// NULL SELECTION
		// first make sure the nullStyle field contains valid information
		_WESynchNullStyle(hWE);

		// apply style changes to the nullStyle record
		_WECopyStyle((WETextStyle *) ts, &pWE->nullStyle.runStyle, pWE->nullStyle.runStyle.tsFace, mode);

		// if the font was altered, synchronize the keyboard script
		if (BTST(pWE->flags, weFNonRoman) && (mode & weDoFont))
		{
			fontScript = FontToScript(pWE->nullStyle.runStyle.tsFont);
			if (fontScript != GetScriptManagerVariable(smKeyScript))
			{
				KeyScript(fontScript);
			}
		}
	}
	else
	{
		// NON-EMPTY SELECTION

		// increment modification count
		pWE->modCount++;

		// if undo support is enabled, save the styles of the text range to be affected
		if (BTST(pWE->features, weFUndoSupport)) 
		{
			WEClearUndo(hWE);
			if (_WENewAction(pWE->selStart, pWE->selEnd, pWE->selEnd - pWE->selStart, weAKSetStyle,
				weAFDontSaveText + weAFDontSaveSoup, hWE, &hAction) == noErr)
			{
				_WEPushAction(hAction);
			}
		}

		// check for "smart" font modes
		if (BTST(pWE->flags, weFNonRoman) && ((mode & weDoSmartFont) == weDoSmartFont))
		{
			if ((err = _WESmartSetFont(mode, ts, hWE)) != noErr)
				goto cleanup;
			mode &= ~weDoFont;
		}

		// set the style of the selection range
		if ((err = _WESetStyleRange(pWE->selStart, pWE->selEnd, mode, (WETextStyle *) ts, hWE)) != noErr)
			goto cleanup;

		// and redraw the text
		if ((err = _WERedraw(pWE->selStart, pWE->selEnd, hWE)) != noErr) 
			goto cleanup;
	}

	// clear the result code
	err = noErr;

cleanup:
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return result code
	return err;
}

pascal OSErr WEUseStyleScrap(StScrpHandle hStyles, WEHandle hWE)
{
	WEPtr pWE;
	Boolean saveWELock;
	OSErr err;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// return an error code if this instance is read-only
	err = weReadOnlyErr;
	if (BTST(pWE->features, weFReadOnly))
		goto cleanup;

	// apply the style scrap to the selection range
	if ((err = _WEApplyStyleScrap(pWE->selStart, pWE->selEnd, hStyles, hWE)) != noErr) 
		goto cleanup;

	// redraw the text
	err = _WERedraw(pWE->selStart, pWE->selEnd, hWE);

cleanup:
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return result code
	return err;
}

#if WASTE_OBJECTS

pascal OSErr WEUseSoup(Handle hSoup, WEHandle hWE)
{
	WEPtr pWE;
	Boolean saveWELock;
	OSErr err;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// return an error code if this instance is read-only
	err = weReadOnlyErr;
	if (BTST(pWE->features, weFReadOnly))
		goto cleanup;

	// apply the soup starting from selStart
	if ((err = _WEApplySoup(pWE->selStart, hSoup, hWE)) != noErr) 
		goto cleanup;

	// redraw the text
	err = _WERedraw(pWE->selStart, pWE->selEnd, hWE);

cleanup:
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return result code
	return err;
}

#endif  // WASTE_OBJECTS
