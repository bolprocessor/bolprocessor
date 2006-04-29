/*
 *	WELowLevelEditing.c
 *
 *	WASTE PROJECT
 *  Low-Level Editing Routines
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */


#include "WASTEIntf.h"

pascal Boolean _WEIsWordRange(long rangeStart, long rangeEnd, WEHandle hWE)
{
	// _WEIsWordRange returns true if the specified range is a word range,
	// i.e. if it would be possible to select it by double-clicking and (optionally) dragging.

	long wordStart, wordEnd;

	// determine if rangeStart is at the beginning of a word
	WEFindWord(rangeStart, kLeadingEdge, &wordStart, &wordEnd, hWE);
	if (rangeStart == wordStart) 
	{

		// determine if rangeEnd is at the end of a word
		WEFindWord(rangeEnd, kTrailingEdge, &wordStart, &wordEnd, hWE);
		return (rangeEnd == wordEnd);
	}
	return false;
} // _WEIsWordRange

pascal Boolean _WEIsPunct(long offset, WEHandle hWE)
{
	short cType;
	
	cType = WECharType(offset, hWE);
	if ((cType & smcTypeMask) == smCharPunct) 
	{
		cType &= smcClassMask;
		if ((cType == smPunctNormal) || (cType == smPunctBlank)) 
			return true;
	}
	return false;
}  // _WEIsPunct

pascal void _WEIntelligentCut(long *rangeStart, long *rangeEnd, WEHandle hWE)
{

	// _WEIntelligentCut is called by other WASTE routines to determine the actual
	// range to be deleted when weFIntCutAndPaste is enabled.
	// On entry, rangeStart and rangeEnd specify the selection range visible to the user.
	// On exit, rangeStart and rangeEnd specify the actual range to be removed.

	// do nothing if the intelligent cut-and-paste feature is disabled
	if (!BTST((*hWE)->features, weFIntCutAndPaste)) 
		return;

	// intelling cut-&-paste rules should be applied only to word ranges...
	if (!_WEIsWordRange(*rangeStart, *rangeEnd, hWE))
		return;

	// ...without punctuation characters at the beginning or end
	if (_WEIsPunct(*rangeStart, hWE)) 
		return;
	if (_WEIsPunct(*rangeEnd - 1, hWE)) 
		return;

	// if the character preceding the selection range is a space, discard it
	if (WEGetChar(*rangeStart - 1, hWE) == kSpace) 
		(*rangeStart)--;
	// else, if the character following the selection range is a space, discard it
	else if (WEGetChar(*rangeEnd, hWE) == kSpace)
		(*rangeEnd)++;

} // _WEIntelligentCut

pascal short _WEIntelligentPaste(long rangeStart, long rangeEnd, WEHandle hWE)
{
	short retval;
	
	// _WEIntelligentPaste is called by other WASTE routines to determine whether
	// an additional space character should be added (before or after) after inserting
	// new text (usually from the Clipboard or from a drag).

	retval = weDontAddSpaces;

	// do nothing unless the intelligent cut-and-paste feature is enabled
	if (!BTST((*hWE)->features, weFIntCutAndPaste)) 
		return retval;

	// extra spaces will be added only if the pasted text looks like a word range,
	// without punctuation characters at the beginning or at the end
	if (_WEIsPunct(rangeStart, hWE))
		return retval;
	if (_WEIsPunct(rangeEnd - 1, hWE))
		return retval;

	// if the character on the left of the pasted text is a punctuation character
	// and the character on the right isn't,  add a space on the right, and vice versa
	if (_WEIsPunct(rangeStart - 1, hWE))
	{
		if (!_WEIsPunct(rangeEnd, hWE))
		{
			retval = weAddSpaceOnRightSide;
		}
	}
	else if (_WEIsPunct(rangeEnd, hWE))
	{
		retval = weAddSpaceOnLeftSide;
	}

	return retval;
} // _WEIntelligentPaste

pascal OSErr _WEInsertRun(long runIndex, long offset, long styleIndex, WEPtr pWE)
{

	// Insert a new element in the style run array, at the specified runIndex position.
	// The new element consists of the pair <offset, styleIndex>.

	RunArrayElement element;
	OSErr err;

	// prepare the element record to be inserted in the array
	element.runStart = offset;
	element.styleIndex = styleIndex;

	// do the insertion
	if ((err = _WEInsertBlock((Handle) pWE->hRuns, (const void *) &element, sizeof(element), (runIndex + 1) * sizeof(element))) != noErr)
		return err;

	// increment style run count
	pWE->nRuns++;

	// increment the reference count field of the style table element
	// referenced by the newly inserted style run
	(*pWE->hStyles)[styleIndex].refCount++;

	return noErr;
} // _WEInsertRun

pascal void _WERemoveRun(long runIndex, WEPtr pWE)
{
	StyleTableElementPtr pStyle;
	
	// get a pointer to the style table element referenced by the style run
	pStyle = *pWE->hStyles + (*pWE->hRuns)[runIndex].styleIndex;

	// decrement the reference count field of the style table element
	// referenced by the style run to be removed
	pStyle->refCount--;

#if WASTE_OBJECTS
	// dispose of the embedded object (if any)
	if (pStyle->info.runStyle.tsObject != NULL)
		_WEFreeObject(pStyle->info.runStyle.tsObject);
#endif
	
	// remove a "slot" from the run array
	_WERemoveBlock((Handle) pWE->hRuns, sizeof(RunArrayElement), runIndex * sizeof(RunArrayElement));

	// decrement style run count
	pWE->nRuns--;
	
} // _WERemoveRun

pascal void _WEChangeRun(long runIndex, long newStyleIndex, Boolean keepOld, WEPtr pWE)
{
	// change the styleIndex field of the specified element of the style run array

	long oldStyleIndex;
	StyleTableElementPtr oldStyle, newStyle;

	// do the change
	oldStyleIndex = (*pWE->hRuns)[runIndex].styleIndex;
	(*pWE->hRuns)[runIndex].styleIndex = newStyleIndex;
	
	// get pointers to old and new style table elements
	oldStyle = *pWE->hStyles + oldStyleIndex;
	newStyle = *pWE->hStyles + newStyleIndex;
	
	// increment the reference count field of the new style table element
	newStyle->refCount++;
	
	// decrement the reference count field of the old style table element
	oldStyle->refCount--;

#if WASTE_OBJECTS
	// dispose of the embedded object (if any) unless it is again referenced in the new style
	if (!keepOld)
	{
		WEObjectDescHandle oldObject = oldStyle->info.runStyle.tsObject;
		
		if ((oldObject != NULL) && (oldObject != newStyle->info.runStyle.tsObject))
			_WEFreeObject(oldObject);
	}
#endif

} // _WEChangeRun

pascal OSErr _WENewStyle(const WETextStyle *ts, long *styleIndex, WEPtr pWE)
{
	// given the specified WETextStyle record, find the corresponding entry
	// in the style table (create a new entry if necessary), and return its index

	StyleTableElementPtr pElement;
	StyleTableElement element;
	long index;
	long unusedIndex;
	OSErr err;

	// see if the given style already exists in the style table
	// while scanning the table, also remember the position of the first unused style, if any
	unusedIndex = -1;
	pElement = *pWE->hStyles;
	for ( index = 0 ; index < pWE->nStyles ; index++ )
	{
		// check for entries which aren't referenced and can be recycled
		if (pElement->refCount == 0)
			unusedIndex = index;

		// perform a bitwise comparison between the current element and the specified style
		// (actually, we ignore metrics information)
		else if (_WEBlockCmp(&pElement->info.runStyle, ts, sizeof(WETextStyle)))
		{
			// found: style already present
			*styleIndex = index;
			return noErr;
		}
		pElement++;
	} // for

	// the specified style doesn't exist in the style table
	// since this is a new entry, we have to calculate font metrics information
	element.info.runStyle = *ts;
	_WEFillFontInfo(pWE->port, &element.info);
	
	// set the high bit of tsFlags if attributes specify a right-to-left run
	if (BTST(pWE->flags, weFBidirectional))
	{
		if (GetScriptVariable(FontToScript(element.info.runStyle.tsFont), smScriptRight) != 0)
		{
			BSET(element.info.runStyle.tsFlags, tsRightToLeft);
		}
	}
	
	// see if we can recycle an unused entry
	if (unusedIndex >= 0) 
	{
		index = unusedIndex;
		(*pWE->hStyles)[index].info = element.info;
	}
	else
	{
		// no reusable entry: we have to append a new element to the table
		element.refCount = 0;
		if ((err = _WEInsertBlock((Handle) pWE->hStyles, &element, sizeof(element), index * sizeof(element))) != noErr)
			return err;

		// update style count in the WE record
		pWE->nStyles++;
	}
	
	// return the index to the new element
	*styleIndex = index;
	return noErr;
}

pascal OSErr _WERedraw(long rangeStart, long rangeEnd, WEHandle hWE)
{
	WEPtr pWE = *hWE;		// assume WE record is already locked
	LineArrayPtr pLines;
	long startLine, endLine;
	long oldTextHeight, newTextHeight;
	LongRect r;
	Rect viewRect, updateRect;
	RgnHandle saveClip;
	GrafPtr savePort;
	OSErr err;
#if WASTE_REDRAW_SPEED
	LongRect scrollRect;
	RgnHandle updateRgn,
              utilRgn;
#endif

	// do nothing if recalculation has been inhibited
	if (!BTST(pWE->features, weFInhibitRecal)) 
	{
		// hide the caret
#if WASTE_REDRAW_SPEED
		BCLR(pWE->flags, weFCaretVisible);
#else
		if (BTST(pWE->flags, weFCaretVisible))
		{ 
			_WEBlinkCaret(hWE);
		}
#endif

		// remember total text height
		oldTextHeight = pWE->destRect.bottom - pWE->destRect.top;

		// find line range affected by modification
		startLine = WEOffsetToLine(rangeStart, hWE);
		endLine = WEOffsetToLine(rangeEnd, hWE);

		// recalculate line breaks starting from startLine
		if ((err = _WERecalBreaks(&startLine, &endLine, hWE)) != noErr) 
		{
			goto cleanup;
		}

		// recalculate slops
		_WERecalSlops(startLine, endLine, hWE);
		
		// do nothing if redrawing has been inhibited
		if (!BTST(pWE->features, weFInhibitRedraw))
		{
	
			// calculate new total text height
			newTextHeight = pWE->destRect.bottom - pWE->destRect.top;
	
			// calculate the rectangle to redraw (in long coordinates)
			r.left = -SHRT_MAX;
			r.right = SHRT_MAX;
			pLines = *pWE->hLines;
			r.top = pLines[startLine].lineOrigin;
	
#if WASTE_REDRAW_SPEED
			// if total text height hasn't changed, it's enough to redraw lines up to endLine
			// otherwise we must redraw all lines from startLine on
	
			if (endLine < pWE->nLines - 1)
				 r.bottom = pLines[endLine + 1].lineOrigin;
			else
				r.bottom = newTextHeight;
			WEOffsetLongRect(&r, 0, pWE->destRect.top);
	
			if (newTextHeight == oldTextHeight)
				WELongRectToRect(&r, &updateRect);
			else
			{
				/*      Instead of scrolling the lines below the deleted text up by redrawing them,
				 *      use scroll bits to move the displayed text up.
				 */
	
				scrollRect = pWE->viewRect;
				if (newTextHeight > oldTextHeight)
					scrollRect.top = pLines[startLine + 1].lineOrigin + pWE->destRect.top;
				else
					scrollRect.top = pLines[startLine].lineOrigin + pWE->destRect.top;
				WELongRectToRect(&scrollRect, &updateRect);
				updateRgn = NewRgn();
				ScrollRect(&updateRect, 0, newTextHeight - oldTextHeight, updateRgn);
	
				//      Redraw the exposed region (caused by a scroll up)      
	
				WELongRectToRect(&r, &updateRect);
				utilRgn = NewRgn();
				RectRgn(utilRgn, &updateRect);
				DiffRgn(updateRgn, utilRgn, updateRgn);
				DisposeRgn(utilRgn);
				WEUpdate(updateRgn, hWE);
				DisposeRgn(updateRgn);
			}
#else
			// if total text height hasn't changed, it's enough to redraw lines up to endLine
			// otherwise we must redraw all lines from startLine on
			if ((newTextHeight == oldTextHeight) && (endLine < pWE->nLines - 1)) 
			{
				r.bottom = pLines[endLine + 1].lineOrigin;
			}
			else if (newTextHeight < oldTextHeight) 
			{
				r.bottom = oldTextHeight;
			}
			else
			{
				r.bottom = newTextHeight;
			}
			
			WEOffsetLongRect(&r, 0, pWE->destRect.top);
	
			// calculate the intersection between this rectangle and the view rectangle
			WELongRectToRect(&r, &updateRect);
#endif
			WELongRectToRect(&pWE->viewRect, &viewRect);
	
			if (SectRect(&updateRect, &viewRect, &updateRect)) 
			{
				// set up the port and the clip region
				GetPort(&savePort);
				SetPort(pWE->port);
	
				// set the clip region to updateRect
				saveClip = NewRgn();
				GetClip(saveClip);
				ClipRect(&updateRect);
	
				// we only really need to redraw the visible lines
				startLine = _WEPixelToLine(updateRect.top - pWE->destRect.top, hWE);
				endLine = _WEPixelToLine(updateRect.bottom - pWE->destRect.top - 1, hWE);
	
				// redraw the lines (pass true in the doErase parameter)
				_WEDrawLines(startLine, endLine, true, hWE);
	
				// erase the portion of the update rectangle below the last line (if any)
				pLines = *pWE->hLines;
				updateRect.top = pWE->destRect.top + pLines[endLine + 1].lineOrigin;
				if (updateRect.top < updateRect.bottom) 
				{
					EraseRect(&updateRect);
				}
	
				// restore the clip region
				SetClip(saveClip);
				DisposeRgn(saveClip);
	
				// restore the port
				SetPort(savePort);
	
				// redraw the caret or the selection range
				if (pWE->selStart < pWE->selEnd) 
				{
					_WEHiliteRange(pWE->selStart, pWE->selEnd, hWE);
				}
				else
				{
					_WEBlinkCaret(hWE);
				}
			} // if SectRect
	
			// scroll the selection range into view
			WESelView(hWE);

		} // if redraw not inhibited
	} // if recal not inhibited

	// clear result code
	err = noErr;

cleanup:
	// return result code
	return err;
} // _WERedraw

pascal OSErr WECalText(WEHandle hWE)
{
	Boolean saveWELock;
	OSErr err;

	// lock WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);

#if WASTE_WECALTEXT_DOES_REDRAW

	// recalculate line breaks & slops and redraw the text
	err = _WERedraw(0, LONG_MAX, hWE);

#else
	
	{
		long startLine = 0;
		long endLine = LONG_MAX;

		// recalculate line breaks & slops without redrawing anything
		if ((err = _WERecalBreaks(&startLine, &endLine, hWE)) == noErr)
			_WERecalSlops(startLine, endLine, hWE);
	}

#endif

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return result code
	return err;
}

pascal OSErr _WESetStyleRange(long rangeStart, long rangeEnd, WEStyleMode mode, const WETextStyle *ts, WEHandle hWE)
{
	// alter the style attributes of the specified text range according to ts and mode

	WEPtr pWE = *hWE;					// assume WE record is already locked
	RunArrayHandle hRuns = pWE->hRuns;
	long offset;
	long runIndex;
	long oldStyleIndex, newStyleIndex;
	WERunInfo runInfo;
	Style continuousStyles;
	OSErr err;

    WEASSERT(rangeStart < rangeEnd, "\pBad style range");

	// if mode contains weDoToggleFace, we need to determine which Quickdraw styles
	// are continuous over the specified text range: those styles must be turned off
	if (BTST(mode, kModeToggleFace)) 
	{
		WEStyleMode temp = weDoFace;
		_WEContinuousStyleRange(rangeStart, rangeEnd, &temp, &runInfo.runAttrs.runStyle, hWE);
		continuousStyles = runInfo.runAttrs.runStyle.tsFace;
	}
	else
	{
		continuousStyles = normal;
	}

	// find the index to the first style run in the specified range
	offset = rangeStart;
	runIndex = _WEOffsetToRun(offset, hWE);

	// run thru all the style runs that encompass the selection range
	do
	{
		// find style index for this run and retrieve corresponding style attributes
		oldStyleIndex = (*hRuns)[runIndex].styleIndex;
		_WEGetIndStyle(runIndex, &runInfo, hWE);

		// _WEGetIndStyle returns textLength + 1 in runInfo.runEnd for the last style run:
		// correct this anomaly (which is useful for other purposes, anyway)
		if (runInfo.runEnd > pWE->textLength) 
		{
			runInfo.runEnd = pWE->textLength;
		}
		
		// apply changes to existing style attributes as requested
		_WECopyStyle(ts, &runInfo.runAttrs.runStyle, continuousStyles, mode);

		// get a style index for the new text style
		if ((err = _WENewStyle(&runInfo.runAttrs.runStyle, &newStyleIndex, pWE)) != noErr)
			goto cleanup;
		
		// if offset falls on a style boundary and this style run has become identical
		// to the previous one, merge the two runs together
		if ((offset == runInfo.runStart) && (runIndex > 0) &&
			((*hRuns)[runIndex - 1].styleIndex == newStyleIndex))
		{
			_WERemoveRun(runIndex, pWE);
			runIndex--;
		}

		// style index changed?
		if (oldStyleIndex != newStyleIndex) 
		{
			// if offset is in the middle of a style run, insert a new style run in the run array
			if (offset > runInfo.runStart) 
			{
				if ((err = _WEInsertRun(runIndex, offset, newStyleIndex, pWE)) != noErr) 
				{
					goto cleanup;
				}
				runIndex++;
			}
			else
			{
				// otherwise just change the styleIndex field of the current style run element
				_WEChangeRun(runIndex, newStyleIndex, (rangeEnd < runInfo.runEnd), pWE);
			}

			// if specified range ends in the middle of a style run, insert yet another element
			if (rangeEnd < runInfo.runEnd) 
			{
				if ((err = _WEInsertRun(runIndex, rangeEnd, oldStyleIndex, pWE)) != noErr) 
				{
					goto cleanup;
				}
			}
		} // if oldStyle != newStyle

		// go to next style run
		runIndex++;
		offset = runInfo.runEnd;

	} while (offset < rangeEnd);

	// if the last style run ends exactly at the end of the specified range,
	// see if we can merge it with the following style run
	if ((offset == rangeEnd) && (runIndex < pWE->nRuns) && 
		((*hRuns)[runIndex].styleIndex == newStyleIndex)) 
	{
		_WERemoveRun(runIndex, pWE);
	}

	// clear result code
	err = noErr;

cleanup:
	// return result code
	return err;
} // _WESetStyleRange

pascal OSErr _WEApplyStyleScrap(long rangeStart, long rangeEnd, StScrpHandle styleScrap, WEHandle hWE)
{
	// apply the given style scrap to the specified text range

	WEPtr pWE = *hWE;	// assume WE record is already locked
	TEStyleScrapElement *pElement;
	long runStart, runEnd;
	short index, lastElement;
	WETextStyle ts;
	OSErr err;

	// loop through each element of the style scrap
	lastElement = (*styleScrap)->scrpNStyles - 1;
	for(index = 0; index <= lastElement; index++)
	{
		// get a pointer to the current scrap element
		pElement = (TEStyleScrapElement *) ((*styleScrap)->scrpStyleTab + index);

		// calculate text run to which this element is to be applied
		runStart = rangeStart + pElement->scrpStartChar;
		if (index < lastElement) 
		{
			runEnd = rangeStart + pElement[1].scrpStartChar;
		}
		else
		{
			runEnd = rangeEnd;
		}

		// perform some range checking
		if (runEnd > rangeEnd) 
		{
			runEnd = rangeEnd;
		}
		if (runStart >= runEnd) 
		{
			continue;
		}

		// copy style to a local variable in case memory moves
		* (TextStyle *) &ts = pElement->scrpTEAttrs.runTEStyle;

		// apply the specified style to the range
		if ((err = _WESetStyleRange(runStart, runEnd, weDoAll + weDoReplaceFace, &ts, hWE)) != noErr)
		{
			return err;
		}
	}
	return noErr;
} // _WEApplyStyleScrap

#if WASTE_OBJECTS

pascal OSErr _WEApplySoup(long offset, Handle hSoup, WEHandle hWE)
{
	WESoup soup;
	unsigned long pSoup, pSoupEnd;
	WETextStyle ts;
	Handle hObjectData;
	long objectOffset;
	Boolean saveWELock;
	OSErr err;

	BLOCK_CLR(ts);
	hObjectData = NULL;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);

	// lock the soup in high heap
	HLockHi(hSoup);
	pSoup = (unsigned long) *hSoup;
	pSoupEnd = pSoup + GetHandleSize(hSoup);

	// loop through each object descriptor in the soup
	while (pSoup < pSoupEnd)
	{
		// Object descriptors may be aligned to odd addresses (duh!)
		// this isn't a problem on 68020+ and PPC CPUs, but causes
		// a fatal address error on the 68000.  To avoid this, we
		// copy the descriptor to the stack with BlockMoveData()
		// before trying to access its fields.
		BlockMoveData((const void *) pSoup, &soup, sizeof(soup));
		
		// if soupDataSize is negative, this soup is a special type that we won't handle here
		if (soup.soupDataSize < 0)
			continue;

		// create a new relocatable block the hold the object data
		if ((err = _WEAllocate(soup.soupDataSize, kAllocTemp, &hObjectData)) != noErr)
			goto cleanup;

		// copy the object data to this block
		BlockMoveData((const void *) (pSoup + sizeof(soup)), *hObjectData, soup.soupDataSize);

		// create a new object out of the tagged data
		if ((err = _WENewObject(soup.soupType, hObjectData, hWE, &ts.tsObject)) != noErr) 
			goto cleanup;

		// if there was no new handler for this object, use the object size stored in the soup
		if ((*ts.tsObject)->objectTable == NULL)
			(*ts.tsObject)->objectSize = soup.soupSize;

		// record a reference to the object descriptor in the style table
		objectOffset = soup.soupOffset + offset;
		err = _WESetStyleRange(objectOffset, objectOffset + 1, weDoObject, &ts, hWE);
		hObjectData = NULL;
		ts.tsObject = NULL;
		if (err != noErr) 
			goto cleanup;

		// advance soup pointer
		pSoup += sizeof(soup) + soup.soupDataSize;

	} // while

	// clear result code
	err = noErr;

cleanup:
	// clean up
	HUnlock(hSoup);
	_WEForgetHandle((Handle *) &ts.tsObject);
	_WEForgetHandle(&hObjectData);

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return result code
	return err;

} // _WEApplySoup

#endif	// WASTE_OBJECTS

pascal void _WEBumpRunStart(long runIndex, long deltaRunStart, WEPtr pWE)
{
	// add deltaLineStart to the lineStart field of all line records
	// starting from lineIndex

	RunArrayElementPtr pRun = *pWE->hRuns + runIndex;
	long nRuns = pWE->nRuns;

	// loop through the style run array adjusting the runStart fields
	for ( ; runIndex <= nRuns; runIndex++ )
	{
		pRun->runStart += deltaRunStart;
		pRun++;
	}
} // _WEBumpRunStart

pascal void _WERemoveRunRange(long rangeStart, long rangeEnd, WEHandle hWE)
{
	// the range of text between rangeStart and rangeEnd is being deleted
	// update the style run array (and the style table) accordingly

	WEPtr pWE = *hWE;	// assume WE record is already locked
	RunArrayElementPtr pRun;
	long startRun, endRun;

	// find the index to the first and last style runs in the specified range
	startRun = _WEOffsetToRun(rangeStart, hWE);
	endRun = _WEOffsetToRun(rangeEnd, hWE) - 1;

	// remove all style runs between startRun and endRun
	for ( ; endRun > startRun; endRun-- )
		_WERemoveRun(endRun, pWE);
	
	// move back all subsequent style runs
	_WEBumpRunStart(startRun + 1, rangeStart - rangeEnd, pWE);

	if ((endRun == startRun) && (endRun < pWE->nRuns - 1))
	{
		pRun = *pWE->hRuns + endRun;
		pRun[1].runStart = rangeStart;
	}

	// remove the first style run if is has become zero length
	pRun = *pWE->hRuns + startRun;
	if (pRun[0].runStart == pRun[1].runStart) 
	{
		_WERemoveRun(startRun, pWE);
		startRun--;
	}

	// merge the first and last runs if they have the same style index
	if (startRun >= 0) 
	{
		pRun = *pWE->hRuns + startRun;
		if (pRun[0].styleIndex == pRun[1].styleIndex) 
		{
			_WERemoveRun(startRun + 1, pWE);
		}
	}
} // _WERemoveRunRange

pascal void _WEBumpLineStart(long lineIndex, long deltaLineStart, WEPtr pWE)
{
	// add deltaLineStart to the lineStart field of all line records
	// starting from lineIndex

	LineRec *pLine = *pWE->hLines + lineIndex;
	long nLines = pWE->nLines;

	// loop through the line array adjusting the lineStart fields
	for ( ; lineIndex <= nLines; lineIndex++ )
	{
		pLine->lineStart += deltaLineStart;
		pLine++;
	}
} // _WEBumpLineStart

pascal void _WERemoveLineRange(long rangeStart, long rangeEnd, WEHandle hWE)
{
	// the range of text between rangeStart and rangeEnd is being deleted
	// update the line array accordingly

	WEPtr pWE = *hWE;	// assume WE record is already locked
	long startLine, nLines;

	// remove all line records between rangeStart and rangeEnd
	startLine = WEOffsetToLine(rangeStart, hWE) + 1;
	nLines = WEOffsetToLine(rangeEnd, hWE) - startLine + 1;
	
	_WERemoveBlock((Handle) pWE->hLines, nLines * sizeof(LineRec), startLine * sizeof(LineRec));
	pWE->nLines -= nLines;

	// update the lineStart field of all the line records that follow
	_WEBumpLineStart(startLine, rangeStart - rangeEnd, pWE);
}

pascal OSErr _WEDeleteRange(long rangeStart, long rangeEnd, WEHandle hWE)
{
	// used internally to delete a text range
	WEPtr pWE = *hWE;	// assume WE record is already locked
	WERunInfo runInfo;
	OSErr err;

	if (rangeEnd > pWE->textLength)
		rangeEnd = pWE->textLength;
	
	// do nothing if the specified range is empty
	if (rangeStart == rangeEnd) 
		goto cleanup1;
	
	// save the first style in the specified range in nullStyle
	WEGetRunInfo(rangeStart, &runInfo, hWE);
	pWE->nullStyle = runInfo.runAttrs;
	BSET(pWE->flags, weFUseNullStyle);

#if WASTE_OBJECTS
	// special case: if we're deleting up to the end of the text, see whether
	// there's an embedded object at the very end and remove it
	if (rangeEnd == pWE->textLength) 
	{
		WEGetRunInfo(rangeEnd - 1, &runInfo, hWE);
		if (runInfo.runAttrs.runStyle.tsObject != NULL) 
		{
			runInfo.runAttrs.runStyle.tsObject = NULL;
			if ((err = _WESetStyleRange(rangeEnd - 1, rangeEnd, weDoObject, &runInfo.runAttrs.runStyle, hWE)) != noErr) 
				goto cleanup2;
		}
	}
#endif

	// remove all line records between rangeStart and rangeEnd
	_WERemoveLineRange(rangeStart, rangeEnd, hWE);
	
	// remove all style runs between rangeStart and rangeEnd
	_WERemoveRunRange(rangeStart, rangeEnd, hWE);
	
	// remove the text
	_WERemoveBlock(pWE->hText, rangeEnd - rangeStart, rangeStart);

	// update textLength field
	pWE->textLength -= (rangeEnd - rangeStart);
	
	// we modified the text, so the anchor range (if any) is no longer valid
	pWE->clickCount = 0;

cleanup1:
	// clear result code
	err = noErr;

cleanup2:
	// return result code
	return err;
} // _WEDeleteRange

pascal OSErr _WEInsertText(long offset, Ptr textPtr, long textLength, WEHandle hWE)
{
	WEPtr pWE = *hWE;	// assume WE record is already locked
	WEStyleMode mode;
	OSErr err;

	// do nothing if textLength is zero or negative
	if (textLength <= 0)
		return noErr;

	// insert the text
	if ((err = _WEInsertBlock(pWE->hText, textPtr, textLength, offset)) != noErr)
		return err;
	
	// update the lineStart fields of all lines following the insertion point
	_WEBumpLineStart(WEOffsetToLine(offset, hWE) + 1, textLength, pWE);

	// update the runStart fields of all style runs following the insertion point
	_WEBumpRunStart(_WEOffsetToRun(offset - 1, hWE) + 1, textLength, pWE);

	// update the textLength field
	pWE->textLength += textLength;

	// we modified the text, so the anchor range (if any) is no longer valid
	pWE->clickCount = 0;

	// make sure the newly inserted text doesn't reference any embedded object
#if WASTE_OBJECTS
	pWE->nullStyle.runStyle.tsObject = NULL;
#endif
	mode = weDoObject;

	// if there is a valid null style, apply it to the newly inserted text
	if (BTST(pWE->flags, weFUseNullStyle))
	{
		mode += (weDoAll + weDoReplaceFace);
	}

	if ((err = _WESetStyleRange(offset, offset + textLength, mode, &pWE->nullStyle.runStyle, hWE)) != noErr)
		return err;

	return noErr;

} // _WEInsertText
