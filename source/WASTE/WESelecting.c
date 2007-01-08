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
 *	WESelecting.c
 *
 *	WASTE PROJECT
 *  Drawing Selections, Activating, Updating, Scrolling, etc.
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */


#include "WASTEIntf.h"

// values for _WEArrowOffset action parameter:

enum {

// plain arrow keys
	kGoLeft			=	0,
	kGoRight		=	1,
	kGoUp			=	2,
	kGoDown			=	3,

// modifiers
	kOption			=	4,
	kCommand		=	8,

// option + arrow combos
	kGoWordStart	=	kGoLeft + kOption,
	kGoWordEnd		=	kGoRight + kOption,
	kGoTextStart	=	kGoUp + kOption,
	kGoTextEnd		=	kGoDown + kOption,

// command + arrow combos
	kGoLineStart	=	kGoLeft + kCommand,
	kGoLineEnd		=	kGoRight + kCommand,
	kGoPageStart	=	kGoUp + kCommand,
	kGoPageEnd		=	kGoDown + kCommand
};

INLINE pascal void _WEClearHiliteBit(void)
{
	LMSetHiliteMode(LMGetHiliteMode() & 0x7F);
}

static Boolean SLPixelToChar(LinePtr pLine, const WERunAttributes *pAttrs,
		Ptr pSegment, long segmentStart, long segmentLength,
		JustStyleCode styleRunPosition, void *callbackData);

static Boolean SLPixelToChar(LinePtr pLine, const WERunAttributes *pAttrs,
		Ptr pSegment, long segmentStart, long segmentLength,
		JustStyleCode styleRunPosition, void *callbackData)
{
	struct SLPixelToCharData *p = (struct SLPixelToCharData *) callbackData;
	WEPtr pWE = *p->hWE;
	Fixed slop;
	short cType;
	Fixed oldWidth;
#if WASTE_OBJECTS
	Fixed objectWidth;
	Fixed subWidth;
#endif

	// if this is the first style run on the line, subtract pen indent from pixelWidth
	if (IS_FIRST_RUN(styleRunPosition))
	{
		p->pixelWidth -= BSL(_WECalcPenIndent(pLine->lineSlop, pWE->alignment), 16);
	}

	// if pixelWidth is gone negative already, the point is on the trailing edge of first glyph
	if (p->pixelWidth < 0)
	{
		p->offset = segmentStart;
		*p->edge = kTrailingEdge;
		return true;	// stop looping
	}
	
	oldWidth = p->pixelWidth;

#if WASTE_OBJECTS
	if (pAttrs->runStyle.tsObject != NULL)
	{

		// EMBEDDED OBJECT
		// calculate object width as Fixed
		objectWidth = BSL((*pAttrs->runStyle.tsObject)->objectSize.h, 16);

		// subtract object width from pixelWidth
		p->pixelWidth -= objectWidth;

#if WASTE_OBJECTS_ARE_GLYPHS

		// find out whether the point is in the leftmost half of the object,
		// in the rightmost half or past the object
		subWidth = objectWidth >> 1;	// divide by two
		if (p->pixelWidth + subWidth < 0)
		{
			p->offset = segmentStart;
			*p->edge = kLeadingEdge;		// point is in leftmost half of object
		}
		else
		{
			p->offset= segmentStart + 1;
			if (p->pixelWidth < 0)
				*p->edge = kTrailingEdge;	// point is in rightmost half of object
			else
				*p->edge = kLeadingEdge;	// point is past object
		}
#else

		// find out whether the point is in the leftmost quarter of the object,
		// in the middle half, in the rightmost quarter or past the object
		subWidth = objectWidth >> 2;	// divide by four
		if (p->pixelWidth + subWidth < 0)
		{
			p->offset = segmentStart;
			if (p->pixelWidth + objectWidth < subWidth)
				*p->edge = kLeadingEdge;		// point is in leftmost quarter of object
			else
				*p->edge = kObjectEdge;		// point is in middle half of object
		}
		else
		{
			p->offset = segmentStart + 1;
			if (p->pixelWidth < 0)
				*p->edge = kTrailingEdge;	// point is in rightmost quarter of object
			else
				*p->edge = kLeadingEdge;		// point is past object
		}
#endif // WASTE_OBJECTS_ARE_GLYPHS
	}
	else
#endif // WASTE_OBJECTS
	{
	
		// REGULAR TEXT
	
		// if this is the last segment on the line, strip the last blank character (if any),
		// unless it is the last non-CR character in the whole text stream
		if (IS_LAST_RUN(styleRunPosition))
		{
			if ((segmentStart + segmentLength < pWE->textLength) ||
				pSegment[segmentLength - 1] == kEOL)
			{
				cType = CallWECharTypeProc(pSegment, segmentLength - 1, FontScript(),
					 p->hWE, pWE->charTypeHook);
				if ((cType & (smcTypeMask + smcClassMask)) == smCharPunct + smPunctBlank)
				{
					segmentLength -= ((cType & smcDoubleMask) ? 2 : 1);
				}
			}
		}
	
		// calculate slop for this text segment (justified text only)
		if (pWE->alignment == weJustify) 
		{
			slop = FixMul(PortionLine(pSegment, segmentLength, styleRunPosition,
				kOneToOneScaling, kOneToOneScaling), pLine->lineJustAmount);
		}
		else
		{
			slop = 0;
		}
	
		// call PixelToChar hook for this segment
		p->offset = segmentStart + CallWEPixelToCharProc(pSegment,
				segmentLength, slop, &p->pixelWidth, p->edge, styleRunPosition, p->hPos,
				p->hWE, pWE->pixelToCharHook);
	}
	
	// increment hPos by change in pixelWidth
	p->hPos += (oldWidth - p->pixelWidth);

	// if pixelWidth has gone negative, we're finished; otherwise go to next run
	return (p->pixelWidth < 0);
}

pascal long WEGetOffset(const LongPt *thePoint, char *edge, WEHandle hWE)
{
	// given a long point in local coordinates,
	// find the text offset corresponding to the nearest glyph

	WEPtr pWE;
	long lineIndex;
	Fixed pixelWidth;
	Boolean saveWELock;
	struct SLPixelToCharData callbackData;
	LongPt tempPoint = *thePoint; // so we don't change original point
	long offset;
	char theEdge;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// offset thePoint so that it is relative to the top left corner of the destination rectangle
	tempPoint.v -= pWE->destRect.top;
	tempPoint.h -= pWE->destRect.left;

	// if the point is above the destination rect, return zero
	if (tempPoint.v < 0) 
	{
		offset = 0;
		theEdge = kTrailingEdge;
	}
	else
	{
		// if the point is below the last line, return last char offset
		if (tempPoint.v >= WEGetHeight(0, LONG_MAX, hWE)) 
		{
			offset = pWE->textLength;
			theEdge = kLeadingEdge;
		}
		else
		{
			// find the line index corresponding to the vertical pixel offset
			lineIndex = _WEPixelToLine(tempPoint.v, hWE);

			// express the horizontal pixel offset as a Fixed value
			pixelWidth = BSL(tempPoint.h, 16);

			// walk through the segments on this line calling PixelToChar
			callbackData.hWE = hWE;
			callbackData.hPos = 0;
			callbackData.pixelWidth = pixelWidth;
			callbackData.edge = &theEdge;
			callbackData.offset = 0;
			_WESegmentLoop(lineIndex, lineIndex, SLPixelToChar, (void *)&callbackData, hWE);
			offset = callbackData.offset;
		}
	}
	
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return offset/edge pair
	if (edge != NULL)
		*edge = theEdge;
	return offset;
}

static Boolean SLCharToPixel(LinePtr pLine, const WERunAttributes *pAttrs,
		Ptr pSegment, long segmentStart, long segmentLength,
		JustStyleCode styleRunPosition, void *callbackData);

static Boolean SLCharToPixel(LinePtr pLine, const WERunAttributes *pAttrs,
		Ptr pSegment, long segmentStart, long segmentLength,
		JustStyleCode styleRunPosition, void *callbackData)
{
	struct SLCharToPixelData *p = (struct SLCharToPixelData *) callbackData;
	WEPtr pWE = *p->hWE;
	Fixed slop;
	short width;
	Boolean isInSegment;

	// is offset within this segment?
	isInSegment = (p->offset < segmentStart + segmentLength);

	// if this is the first style run on the line, add pen indent to thePoint.h
	if (IS_FIRST_RUN(styleRunPosition))
	{
		p->thePoint->h += _WECalcPenIndent(pLine->lineSlop, pWE->alignment);
	}

#if WASTE_OBJECTS
	if (pAttrs->runStyle.tsObject != NULL)
	{
	
		// EMBEDDED OBJECT
		width = isInSegment ? 0 : (*(pAttrs->runStyle.tsObject))->objectSize.h;
	}
	else
#endif
	{
		// REGULAR TEXT
		// calculate slop for this text segment (justified text only)
		if (pWE->alignment == weJustify) 
		{
			slop = FixMul(PortionLine(pSegment, segmentLength, styleRunPosition,
				kOneToOneScaling, kOneToOneScaling), pLine->lineJustAmount);
		}
		else
		{
			slop = 0;
		}
		
		// call CharToPixel to get width of segment up to specified offset
		width = CallWECharToPixelProc(pSegment, segmentLength,
			slop, p->offset - segmentStart, hilite, styleRunPosition, p->thePoint->h,
			p->hWE, pWE->charToPixelHook);
	}
	
	// advance thePoint.h by the width of this segment
	p->thePoint->h += width;

	// drop out of loop when we reach offset
	return isInSegment;
}

pascal void WEGetPoint(long offset, LongPt *thePoint, short *lineHeight, WEHandle hWE)
{
	// given a byte offset into the text, find the corresponding glyph position
	// this routine is useful for highlighting the text and for positioning the caret

	WEPtr pWE;
	LineRec *pLine;
	long lineIndex;
	Boolean saveWELock;
	struct SLCharToPixelData callbackData;	
	
	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// the base point is the top left corner of the destination rectangle
	*thePoint = * (LongPt *) &pWE->destRect;

	// first of all find the line on which the glyph lies
	lineIndex = WEOffsetToLine(offset, hWE);

	// calculate the vertical coordinate and the line height
	pLine = *pWE->hLines + lineIndex;
	thePoint->v += pLine->lineOrigin;
	*lineHeight = pLine[1].lineOrigin - pLine[0].lineOrigin;

	if ((offset == pWE->textLength) && (WEGetChar(offset - 1, hWE) == kEOL)) 
	{
		// SPECIAL CASE: if offset is past the last character and
		// the last character is a carriage return, return a point below the last line

		thePoint->v += *lineHeight;
		thePoint->h += _WECalcPenIndent(pWE->destRect.right - pWE->destRect.left, pWE->alignment);
	}
	else
	{
		callbackData.hWE = hWE;
		callbackData.offset = offset;
		callbackData.thePoint = thePoint;
		// to get the horizontal coordinate, walk through the style runs on this line
		_WESegmentLoop(lineIndex, lineIndex, SLCharToPixel, (void *)&callbackData, hWE);
	}

	// pin the horizontal coordinate to the destination rectangle
	thePoint->h = _WEPinInRange(thePoint->h, pWE->destRect.left, pWE->destRect.right);

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);
}

pascal void WEFindLine(long offset, char edge, long *lineStart, long *lineEnd, WEHandle hWE)
{
#pragma unused(edge)
	WEPtr pWE = *hWE;
	LineRec *pLine;
	
	pLine = *pWE->hLines + WEOffsetToLine(offset, hWE);
	*lineStart = pLine[0].lineStart;
	*lineEnd = pLine[1].lineStart;
}

pascal ScriptCode _WEGetContext(long offset, long *contextStart, long *contextEnd,
						WEHandle hWE)
{
	// This function finds a range of characters ("context"), all belonging to the same script
	// and centered around the specified offset.
	// The function result is the ID of a font belonging to this script.
	// Ideally, the context should consist of a whole script run, but in practice the returned
	// context can be narrower, for performance and other reasons (see below)

	long index, saveIndex, saveRunEnd;
	WERunInfo runInfo;
	ScriptCode script1, script2;
	short retval;
	
	if (BTST((*hWE)->flags, weFNonRoman))
	{ 
		// if more than one script is installed, limit the search of script run boundaries
		// to a single line, for speed's sake
		WEFindLine(offset, kLeadingEdge, contextStart, contextEnd, hWE);

		// find the style run the specified offset is in
		index = _WEOffsetToRun(offset, hWE);
		_WEGetIndStyle(index, &runInfo, hWE);

		// find the script code associated with this style run
		script1 = FontToScript(runInfo.runAttrs.runStyle.tsFont);

		// the script code is returned as function result
		retval = script1;

		// save index and runInfo.runEnd for the second while loop
		saveIndex = index;
		saveRunEnd = runInfo.runEnd;

		// walk backwards across style runs preceding offset, looking for a script run boundary
		while (runInfo.runStart > *contextStart)
		{
			index--;
			_WEGetIndStyle(index, &runInfo, hWE);
			script2 = FontToScript(runInfo.runAttrs.runStyle.tsFont);
			if (script1 != script2) 
			{
				*contextStart = runInfo.runEnd;
				break;
			}
		}

		// restore index and runInfo.runEnd
		index = saveIndex;
		runInfo.runEnd = saveRunEnd;

		// walk forward across style runs following offset, looking for a script run boundary
		while (runInfo.runEnd < *contextEnd)
		{
			index++;
			_WEGetIndStyle(index, &runInfo, hWE);
			script2 = FontToScript(runInfo.runAttrs.runStyle.tsFont);
			if (script1 != script2) 
			{
				*contextEnd = runInfo.runStart;
				break;
			}
		}
	}
	else
	{
		// only the Roman script is enabled: the whole text constitutes one script run
		retval = smRoman;
		*contextStart = 0;
		*contextEnd = (*hWE)->textLength;
	}

	// make sure the range identified by contextStart/contextEnd is contained within
	// the 32K byte range centered around the specified offset
	// the reason for this is that many Script Manager routines (e.g. FindWord and CharByte)
	// only accept 16-bit offsets, rather than 32-bit offsets

	*contextStart = _WEPinInRange(*contextStart, offset - (SHRT_MAX / 2), offset);
	*contextEnd = _WEPinInRange(*contextEnd, offset, offset + (SHRT_MAX / 2));

	return retval;
}

pascal ScriptCode _WEGetRestrictedContext(long offset, long *contextStart, long *contextEnd,
						WEHandle hWE)
{
	// This function finds a range of characters ("context"), all belonging to the same script
	// and centered around the specified offset.
	// This function returns a script run subrange and is more efficient than
	// _WEGetContext because it doesn't try to find the script boundaries accurately.

	WERunInfo runInfo;

	// just find the style run the specified offset is in
	WEGetRunInfo(offset, &runInfo, hWE);
	*contextStart = runInfo.runStart;
	*contextEnd = runInfo.runEnd;

	// make sure the range identified by contextStart/contextEnd is contained within
	// the 32K byte range centered around the specified offset
	// the reason for this is that many Script Manager routines (e.g. FindWord and CharByte)
	// only accept 16-bit offsets, rather than 32-bit offsets

	*contextStart = _WEPinInRange(*contextStart, offset - (SHRT_MAX / 2), offset);
	*contextEnd = _WEPinInRange(*contextEnd, offset, offset + (SHRT_MAX / 2));

	return FontToScript(runInfo.runAttrs.runStyle.tsFont);
} // _WEGetRestrictedContext

pascal void WEFindWord(long offset, char edge, long *wordStart, long *wordEnd, WEHandle hWE)
{
	WEPtr pWE;
	ScriptCode script;
	long contextStart, contextEnd;
	Handle hText;
	OffsetTable wordBreaks;
	Boolean saveTextLock, saveWELock;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// find a script context containing the specified offset
	// (words cannot straddle script boundaries)
	script = _WEGetContext(offset, &contextStart, &contextEnd, hWE);

	// lock the text
	hText = pWE->hText;
	saveTextLock = _WESetHandleLock(hText, true);

	// call the word break hook
	CallWEWordBreakProc(*hText + contextStart, contextEnd - contextStart,
		offset - contextStart, edge, wordBreaks, script, hWE, pWE->wordBreakHook);

	// unlock the text
	_WESetHandleLock(hText, saveTextLock);

	// calculate wordStart and wordEnd relative to the beginning of the text
	*wordStart = contextStart + wordBreaks[0].offFirst;
	*wordEnd = contextStart + wordBreaks[0].offSecond;
}

pascal short WECharByte(long offset, WEHandle hWE)
{
	WEPtr pWE;
	ScriptCode script;
	long contextStart, contextEnd;
	short retval;
	Boolean saveWELock, saveTextLock;
	Handle hText;

	retval = smSingleByte;
	pWE = *hWE;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// do nothing unless there is at least one double-byte script system installed
	// and make sure offset is within allowed bounds
	if (BTST(pWE->flags, weFDoubleByte))
	{
		if ((offset >= 0) && (offset < pWE->textLength))
		{

			// find a script context containing the specified offset
			script = _WEGetRestrictedContext(offset, &contextStart, &contextEnd, hWE);

			// lock the text
			hText = pWE->hText;
			saveTextLock = _WESetHandleLock(hText, true);

			// pass the CharByte hook a pointer to the beginning of the style run
			retval = CallWECharByteProc(*hText + contextStart,
				offset - contextStart, script, hWE, pWE->charByteHook);

			// unlock the text
			_WESetHandleLock(hText, saveTextLock);
		}
	}
	
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	return retval;

} // WECharByte

pascal short WECharType(long offset, WEHandle hWE)
{
	WEPtr pWE;
	ScriptCode script;
	long contextStart, contextEnd;
	Handle hText;
	Boolean saveWELock, saveTextLock;
	short retval;
	
	retval = 0;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// make sure offset is within allowed bounds
	if ((offset >= 0) && (offset < pWE->textLength))
	{

		// find a script context containing the specified offset
		script = _WEGetRestrictedContext(offset, &contextStart, &contextEnd, hWE);

		// lock the text
		hText = pWE->hText;
		saveTextLock = _WESetHandleLock(hText, true);

		// pass the CharType hook a pointer to the beginning of the style run
		retval = CallWECharTypeProc(*hText + contextStart,
			offset - contextStart, script, hWE, pWE->charTypeHook);

		// unlock the text
		_WESetHandleLock(hText, saveTextLock);
	}

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	return retval;
} // WECharType


pascal void _WEDrawCaret(long offset, WEHandle hWE)
{
	WEPtr pWE = *hWE;	// assume WE record is already locked
	LongPt thePoint;
	Rect caretRect;
	short caretHeight;
	GrafPtr savePort;
	RgnHandle saveClip;

	// find the caret position using WEGetPoint
	WEGetPoint(offset, &thePoint, &caretHeight, hWE);
	WELongPointToPoint(&thePoint, (Point *) &caretRect.top);
	if (caretRect.left > pWE->destRect.left) 
	{
		caretRect.left--;
	}
	
	// calculate caret rectangle
	caretRect.bottom = caretRect.top + caretHeight;
	caretRect.right = caretRect.left + kCaretWidth;

	// set up the port
	GetPort(&savePort);
	SetPort(pWE->port);

	// clip to the view region
	saveClip = NewRgn();
	GetClip(saveClip);
	SetClip(pWE->viewRgn);

	// draw the caret
	InvertRect(&caretRect);

	// restore the clip region
	SetClip(saveClip);
	DisposeRgn(saveClip);

	// restore the port
	SetPort(savePort);
}

pascal void _WEBlinkCaret(WEHandle hWE)
{
	WEPtr pWE = *hWE;	// assume WE record is already locked

	// do nothing if we're not active
	if (!BTST(pWE->flags, weFActive))
		return;

#if WASTE_NO_RO_CARET
	if (BTST(pWE->features, weFReadOnly) && !BTST(pWE->flags, weFCaretVisible))
		return;
#endif

	// redraw the caret, in XOR mode
	_WEDrawCaret(pWE->selStart, hWE);

	// keep track of the current caret visibility status
	BCHG(pWE->flags, weFCaretVisible);	// invert flag

	// update caretTime
	pWE->caretTime = TickCount();

} // _WEBlinkCaret

pascal RgnHandle WEGetHiliteRgn(long rangeStart, long rangeEnd, WEHandle hWE)
{
	// returns the hilite region corresponding to the specified range
	// the caller is responsible for disposing of the returned region
	// when it's finished with it

	WEPtr pWE;
	RgnHandle hiliteRgn;
	LongRect selRect;
	LongPt firstPoint, lastPoint;
	short firstLineHeight, lastLineHeight;
	Rect r;
	GrafPtr savePort;
	Boolean saveWELock;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// set up the port
	GetPort(&savePort);
	SetPort(pWE->port);

	// make sure rangeStart comes before rangeEnd
	_WEReorder(&rangeStart, &rangeEnd);

	// calculate pixel location corresponding to rangeStart
	WEGetPoint(rangeStart, &firstPoint, &firstLineHeight, hWE);

	// calculate pixel location corresponding to rangeEnd
	WEGetPoint(rangeEnd, &lastPoint, &lastLineHeight, hWE);

	// open a region: rects to be hilited will be accumulated in this
	OpenRgn();

	if (firstPoint.v == lastPoint.v) 
	{
		// selection range encompasses only one line
		WESetLongRect(&selRect, firstPoint.h, firstPoint.v, lastPoint.h, lastPoint.v + lastLineHeight);
		WELongRectToRect(&selRect, &r);
		FrameRect(&r);
	}
	else
	{
		// selection range encompasses more than one line
		// hilite the first line
		WESetLongRect(&selRect, firstPoint.h, firstPoint.v, pWE->destRect.right, firstPoint.v + firstLineHeight);
		WELongRectToRect(&selRect, &r);
		FrameRect(&r);

		// any lines between the first and the last one?
		if (firstPoint.v + firstLineHeight < lastPoint.v) 
		{
			// hilite all the lines in-between
			WESetLongRect(&selRect, pWE->destRect.left, firstPoint.v + firstLineHeight, pWE->destRect.right, lastPoint.v);
			WELongRectToRect(&selRect, &r);
			FrameRect(&r);
		}

		// hilite the last line
		WESetLongRect(&selRect, pWE->destRect.left, lastPoint.v, lastPoint.h, lastPoint.v + lastLineHeight);
		WELongRectToRect(&selRect, &r);
		FrameRect(&r);
	}

	// copy the accumulated region into a new region
	hiliteRgn = NewRgn();
	CloseRgn(hiliteRgn);

	// restrict this region to the view region
	SectRgn(hiliteRgn, pWE->viewRgn, hiliteRgn);

	// restore the port
	SetPort(savePort);

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	// return the hilite region
	return hiliteRgn;
}

pascal void _WEHiliteRange(long rangeStart, long rangeEnd, WEHandle hWE)
{
	WEPtr pWE;
	RgnHandle saveClip, auxRgn, hiliteRgn;
	PenState savePen;
	GrafPtr savePort;

	// the WE record must be already locked
	pWE = *hWE;

	// do nothing if the specified range is empty
	if (rangeStart == rangeEnd) 
	{
		return;
	}

	// set up the port
	GetPort(&savePort);
	SetPort(pWE->port);

	// create auxiliary regions
	saveClip = NewRgn();
	auxRgn = NewRgn();

	// restrict the clip region to the view rectangle
	GetClip(saveClip);
	SectRgn(saveClip, pWE->viewRgn, auxRgn);
	SetClip(auxRgn);

	// get the hilite region corresponding to the specified range
	hiliteRgn = WEGetHiliteRgn(rangeStart, rangeEnd, hWE);

	// hilite the region or frame it, depending on the setting of the active flag
	if (BTST(pWE->flags, weFActive))
	{
		_WEClearHiliteBit();
		InvertRgn(hiliteRgn);
	}
	else if (BTST(pWE->features, weFOutlineHilite)) 
	{
		GetPenState(&savePen);
		PenNormal();
		PenMode(patXor);
		_WEClearHiliteBit();
		FrameRgn(hiliteRgn);
		SetPenState(&savePen);
	}

	// restore the clip region
	SetClip(saveClip);

	// dispose of all regions
	DisposeRgn(saveClip);
	DisposeRgn(auxRgn);
	DisposeRgn(hiliteRgn);

	// restore the port
	SetPort(savePort);
}

pascal void WESetSelection(long selStart, long selEnd, WEHandle hWE)
{
	WEPtr pWE;
	long oldSelStart, oldSelEnd;
	Boolean saveWELock;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;
	
	// range-check parameters
	selStart = _WEPinInRange(selStart, 0, pWE->textLength);
	selEnd = _WEPinInRange(selEnd, 0, pWE->textLength);

	// set the weFAnchorIsEnd bit if selStart > selEnd,  reorder the endpoints
	if (selStart > selEnd) 
	{
		BSET(pWE->flags, weFAnchorIsEnd);
		_WEReorder(&selStart, &selEnd);
	}
	else
	{
		BCLR(pWE->flags, weFAnchorIsEnd);
	}

	// get old selection range
	oldSelStart = pWE->selStart;
	oldSelEnd = pWE->selEnd;

	// selection changed?
	if ((oldSelStart != selStart) || (oldSelEnd != selEnd))
	{
		// invalid the null style
		BCLR(pWE->flags, weFUseNullStyle);

		// hide the caret if it's showing
		if (BTST(pWE->flags, weFCaretVisible))
		{
			_WEBlinkCaret(hWE);
		}

		// set new selection range
		pWE->selStart = selStart;
		pWE->selEnd = selEnd;

		// skip this section if redrawing has been inhibited
		if (!BTST(pWE->features, weFInhibitRecal))
		{
			// if we're active, invert the exclusive-OR between the old range and the new range.
			// if we're inactive, this optimization can't be used because of outline highlighting.
			if (BTST(pWE->flags, weFActive))
			{ 
				_WEReorder(&oldSelStart, &selStart);
				_WEReorder(&oldSelEnd, &selEnd);
				_WEReorder(&oldSelEnd, &selStart);
			}
	
			_WEHiliteRange(oldSelStart, oldSelEnd, hWE);
			_WEHiliteRange(selStart, selEnd, hWE);
	
			if (!BTST(pWE->flags, weFMouseTracking))
			{
				// redraw the caret immediately, if the selection range is empty
				if (pWE->selStart == pWE->selEnd)
				{ 
					_WEBlinkCaret(hWE);
				}

				// clear clickCount, unless we're tracking the mouse
				pWE->clickCount = 0;
	
				// scroll the selection into view, unless we're tracking the mouse
				WESelView(hWE);
	
			}
		} // if redrawing not inhibited
	} // if selection changed
	
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);
}

pascal void WESetAlignment(WEAlignment alignment, WEHandle hWE)
{
	WEPtr pWE;
	WEAlignment oldAlignment;
	Boolean saveWELock;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;
	oldAlignment = pWE->alignment;

	if ((alignment >= weFlushLeft) && (alignment <= weJustify) && (alignment != oldAlignment))
	{
		// hide the caret if it's showing
		if (BTST(pWE->flags, weFCaretVisible))
			_WEBlinkCaret(hWE);

		// change the alignment
		pWE->alignment = alignment;

		// if the text was left-aligned, then we haven't been bothering till now,
		// so we have to recalc the whole document
		if (oldAlignment == weFlushLeft)
			_WERecalSlops(0, pWE->nLines - 1, hWE);

		// redraw the view rectangle
		WEUpdate(NULL, hWE);
	}
	
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);
}

pascal long _WEArrowOffset(short action, long offset, WEHandle hWE)
{
	// given an action code (corresponding to a modifiers + arrow key combo)
	// and an offset into the text, find the offset of the new caret position

	LongPt thePoint;
	long textLength, rangeStart, rangeEnd;
	short lineHeight;
	char edge;
#if WASTE_KURTHS_OPTION_ARROWS
	short cType;
#endif

	textLength = (*hWE)->textLength;
	switch (action)
	{
		case kGoLeft: 
			if (offset > 0)
			{
				offset--;
				if (WECharByte(offset, hWE) != smSingleByte) 
				{
					offset--;
				}
			}
			break;
			
		case kGoRight: 
			if (offset < textLength)
			{
				if (WECharByte(offset, hWE) != smSingleByte) 
				{
					offset++;
				}
				offset++;
			}
			break;

		case kGoUp: 
			WEGetPoint(offset, &thePoint, &lineHeight, hWE);
			thePoint.v--;
			offset = WEGetOffset(&thePoint, &edge, hWE);
			break;

		case kGoDown: 
			WEGetPoint(offset, &thePoint, &lineHeight, hWE);
			thePoint.v += lineHeight;
			offset = WEGetOffset(&thePoint, &edge, hWE);
			break;
			
		case kGoWordStart:
#if WASTE_KURTHS_OPTION_ARROWS
			// loop "forever" (until we break out of it)
			while (true)
			{
				WEFindWord(offset, kTrailingEdge, &rangeStart, &rangeEnd, hWE);
				offset = rangeStart;

				// If the found range is empty, get outta here.  (Most
				// likely this means that we have reached the beginning
				// of the text.)
				if (rangeStart == rangeEnd)
					break;

				cType = WECharType(rangeStart, hWE);

				// If the char is punctuation (other than a number),
				// it's not really a word, so keep looping.  Otherwise
				// we're done.
				if (((cType & smcTypeMask) != smCharPunct) ||
					((cType & smcClassMask) == smPunctNumber))
				{
					break;
				}
			}
#else
			WEFindWord(offset, kTrailingEdge, &rangeStart, &rangeEnd, hWE);
			offset = rangeStart;
#endif // WASTE_KURTHS_OPTION_ARROWS
			break;
			
		case kGoWordEnd:
#if WASTE_KURTHS_OPTION_ARROWS
			// loop "forever" (until we break out of it)
			while (true)
			{
				WEFindWord(offset, kLeadingEdge, &rangeStart, &rangeEnd, hWE);
				offset = rangeEnd;

				// If the found range is empty, get outta here.
				// (Most likely this means that we have reached
				// the end of the text.)
				if (rangeStart == rangeEnd)
					break;

				// `rangeEnd - 1' may point in the middle of a two-byte
				// character; that's ok, CharType can deal with that.
				cType = WECharType(rangeEnd - 1, hWE);

				// If the char is punctuation (other than a number),
				// it's not really a word, so keep looping.  Otherwise
				// we're done.
				if (((cType & smcTypeMask) != smCharPunct) || 
					((cType & smcClassMask) == smPunctNumber))
				{
					break;
				}
			}
#else
			WEFindWord(offset, kLeadingEdge, &rangeStart, &rangeEnd, hWE);
			offset = rangeEnd;
#endif // WASTE_KURTHS_OPTION_ARROWS
			break;
			
		case kGoTextStart: 
			offset = 0;
			break;

		case kGoTextEnd: 
			offset = textLength;
			break;

		case kGoLineStart: 
			WEFindLine(offset, kLeadingEdge, &rangeStart, &rangeEnd, hWE);
			offset = rangeStart;
			break;

		case kGoLineEnd: 
			WEFindLine(offset, kTrailingEdge, &rangeStart, &rangeEnd, hWE);
			offset = rangeEnd;
			if (offset < textLength) 
			{
				offset--;
				if (WECharByte(offset, hWE) != smSingleByte) 
				{
					offset--;
				}
			}
			break;
			
		default:
			break;
	}

	return offset;
}

pascal void _WEDoArrowKey (short wearrow, EventModifiers modifiers, WEHandle hWE)
{
	// this routine is called by WEKey to handle arrow keys

	WEPtr pWE = *hWE;	// assume the WE record is already locked
	short action;
	long selStart, selEnd;
	long caretLoc, anchor;

	// calculate the "action" parameter for _WEArrowOffset from arrow and modifiers
	action = wearrow - kArrowLeft;			// possible range: 0..3
	if (modifiers & optionKey)
	{
		action += kOption;
	}
	if (modifiers & cmdKey)
	{
		action += kCommand;
	}
	
	// get selection range
	selStart = pWE->selStart;
	selEnd = pWE->selEnd;

	if ((modifiers & shiftKey) == 0) 
	{
		// if selection range isn't empty, collapse it to one of the endpoints
		if (selStart < selEnd) 
		{
			if ((wearrow == kArrowLeft) || (wearrow == kArrowUp)) 
			{
				caretLoc = selStart;
			}
			else
			{
				caretLoc = selEnd;
			}
		}
		else
		{
			// otherwise move the insertion point
			caretLoc = _WEArrowOffset(action, selStart, hWE);
		}
		
		// set anchor to caretLoc, so new selection will be empty
		anchor = caretLoc;
	}
	else
	{
		// shift key was held down: extend the selection rather than replacing it
		// find out which selection boundary is the anchor and which is the free endpoint
		if (BTST(pWE->flags, weFAnchorIsEnd)) 
		{
			anchor = selEnd;
			caretLoc = selStart;
		}
		else
		{
			anchor = selStart;
			caretLoc = selEnd;
		}
		
		// move the free endpoint
		caretLoc = _WEArrowOffset(action, caretLoc, hWE);
	}

	// select the new selection
	WESetSelection(anchor, caretLoc, hWE);
}

pascal Boolean WEAdjustCursor(Point mouseLoc, RgnHandle mouseRgn, WEHandle hWE)
{
	// Call WEAdjustCursor to set the cursor shape when the mouse is in the view rectangle.
	// MouseRgn should be either a valid region handle or NULL.
	// If mouseRgn is supplied (i.e., if it's not NULL), it is intersected with a region
	// in global coordinates within which the cursor is to retain its shape.
	// WEAdjustCursor returns true if the cursor has been set.
	// Your application should set the cursor only if WEAdjustCursor returns false.

	WEPtr pWE;
	RgnHandle auxRgn, hiliteRgn;
	enum { kIBeam, kArrow} cursorType;
	Point portDelta;
	GrafPtr savePort;
	Boolean saveWELock;
	Boolean adjustCursor;

	adjustCursor = false;
	cursorType = kIBeam;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// set up the port
	GetPort(&savePort);
	SetPort(pWE->port);

	// calculate delta between the local coordinate system and the global one
	portDelta.v = 0;
	portDelta.h = 0;
	LocalToGlobal(&portDelta);

	// calculate the visible portion of the view rectangle, in global coordinates
	auxRgn = NewRgn();
	CopyRgn(pWE->viewRgn, auxRgn);
	SectRgn(auxRgn, pWE->port->visRgn, auxRgn);
	OffsetRgn(auxRgn, portDelta.h, portDelta.v);

	if (PtInRgn(mouseLoc, auxRgn)) 
	{
		// mouse is within view rectangle: it's up to us to set the cursor
		adjustCursor = true;

		// if drag-and-drop is enabled, see if the mouse is within current selection
		if (BTST(pWE->flags, weFHasDragManager) && BTST(pWE->features, weFDragAndDrop))
		{
			if (pWE->selStart < pWE->selEnd)
			{
			
				// get current hilite region in global coordinates
				hiliteRgn = WEGetHiliteRgn(pWE->selStart, pWE->selEnd, hWE);
				OffsetRgn(hiliteRgn, portDelta.h, portDelta.v);

				// if mouse is within selection, set cursor to an arrow, else to an I-beam
				// (actually, we still use an I-beam if less than DoubleTime ticks have elapsed
				// since the last mouse click, so that the cursor doesn't turn into an arrow while
				// triple-clicking + dragging a range of lines)

				if (PtInRgn(mouseLoc, hiliteRgn) && ((TickCount() > pWE->clickTime + GetDblTime()) ||
					(pWE->clickEdge == kObjectEdge)))
				{
					cursorType = kArrow;				// use arrow cursor
					CopyRgn(hiliteRgn, auxRgn);
				}
				else
				{
					DiffRgn(auxRgn, hiliteRgn, auxRgn);
				}
				
				// dispose of the hilite region
				DisposeRgn(hiliteRgn);

			} // if drag-and-drop is enabled
		}
		
		// set the cursor
		if (cursorType == kIBeam)
			SetCursor(*GetCursor(iBeamCursor));
		else
			SetCursor(&qd.arrow);

		// set mouseRgn, if provided
		if (mouseRgn != NULL) 
		{
			SectRgn(mouseRgn, auxRgn, mouseRgn);
		}
	}
	else
	{
		// mouse is outside view rectangle: don't set the cursor; subtract viewRgn from mouseRgn
		if (mouseRgn != NULL) 
		{
			DiffRgn(mouseRgn, auxRgn, mouseRgn);
		}
	}
	// dispose of the temporary region
	DisposeRgn(auxRgn);

	// restore the port
	SetPort(savePort);

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);
	
	return adjustCursor;
}

pascal void WEIdle(unsigned long *maxSleep, WEHandle hWE)
{
	WEPtr pWE;
	unsigned long currentTime, blinkTime, sleep;
	Boolean saveWELock;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

#if WASTE_DEBUG
		_WESanityCheck(hWE);
#endif

	// the caret blinks only if we're active and the selection point is empty
	if (BTST(pWE->flags, weFActive) && (pWE->selStart == pWE->selEnd)) 
	{
		// get current time
		currentTime = TickCount();
		
		// calculate when the caret should be blinked again
		blinkTime = pWE->caretTime + GetCaretTime();

		if (currentTime < blinkTime)
		{
			sleep = blinkTime - currentTime;
		}
		else
		{
			_WEBlinkCaret(hWE);
			sleep = GetCaretTime();
		}
	}
	else
	{
		// if we don't need to blink the caret, we can sleep forever
		sleep = LONG_MAX;
	}
	
	// return sleepTime to the caller if maxSleep isn't NULL
	if (maxSleep != NULL)
	{
		*maxSleep = sleep;
	}
	
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);
}

pascal void WEUpdate(RgnHandle updateRgn, WEHandle hWE)
{
	WEPtr pWE;
	LongRect updateRect;
	Rect r;
	RgnHandle saveClip, auxRgn;
	GrafPtr savePort;
	Boolean saveWELock;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// set up the port
	GetPort(&savePort);
	SetPort(pWE->port);

	// save the clip region
	saveClip = NewRgn();
	GetClip(saveClip);

	// clip to the insersection between updateRgn and the view rectangle
	// (updateRgn may be NULL; in this case, just clip to the view rectangle)
	auxRgn = NewRgn();
	if (updateRgn != NULL) 
	{
		SectRgn(updateRgn, pWE->viewRgn, auxRgn);
	}
	else
	{
		CopyRgn(pWE->viewRgn, auxRgn);
	}
	SetClip(auxRgn);

	if (!EmptyRgn(auxRgn))
	{
		// calculate the rectangle to update
		r = (*auxRgn)->rgnBBox;
		WERectToLongRect(&r, &updateRect);

		// find out which lines need to be redrawn and draw them
		// if updateRgn is NULL, erase each line rectangle before redrawing
		_WEDrawLines( _WEPixelToLine(updateRect.top - pWE->destRect.top, hWE),
					  _WEPixelToLine((updateRect.bottom - 1) - pWE->destRect.top, hWE),
					  (updateRgn == NULL), hWE);

		// erase the portion of the update rectangle below the last line (if any)
		updateRect.top = pWE->destRect.top + (*pWE->hLines)[pWE->nLines].lineOrigin;
		if (updateRect.top < updateRect.bottom) 
		{
			WELongRectToRect(&updateRect, &r);
			EraseRect(&r);
		}

		// hilite the selection range or draw the caret (only if active)
		if (pWE->selStart < pWE->selEnd)
		{
			_WEHiliteRange(pWE->selStart, pWE->selEnd, hWE);
		}
		else if (BTST(pWE->flags, weFCaretVisible)) 
		{
			_WEBlinkCaret(hWE);
			BSET(pWE->flags, weFCaretVisible);
		}
	}

	DisposeRgn(auxRgn);

	// restore the clip region
	SetClip(saveClip);
	DisposeRgn(saveClip);

	// restore the port
	SetPort(savePort);

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);
}

pascal void WEDeactivate(WEHandle hWE)
{
	WEPtr pWE;
	Boolean saveWELock;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// do nothing if we are already inactive
	if (BTST(pWE->flags, weFActive))
	{

		// hide the selection range or the caret
		_WEHiliteRange(pWE->selStart, pWE->selEnd, hWE);
		if (BTST(pWE->flags, weFCaretVisible))
			_WEBlinkCaret(hWE);

		// clear the active flag
		BCLR(pWE->flags, weFActive);

		// frame the selection
		_WEHiliteRange(pWE->selStart, pWE->selEnd, hWE);

		// dispose of the offscreen graphics world, if any
		if (pWE->offscreenPort != NULL)
		{
			DisposeGWorld((GWorldPtr)(pWE->offscreenPort));
			pWE->offscreenPort = NULL;
		}

		// notify Text Services
		if (pWE->tsmReference != NULL)
			DeactivateTSMDocument(pWE->tsmReference);
	}
	
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);
}

pascal void WEActivate(WEHandle hWE)
{
	WEPtr pWE;
	Boolean saveWELock;

	if (WEIsActive(hWE)) return;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// remove the selection frame
	_WEHiliteRange(pWE->selStart, pWE->selEnd, hWE);

	// set the active flag
	BSET(pWE->flags, weFActive);

	// show the selection range
	_WEHiliteRange(pWE->selStart, pWE->selEnd, hWE);

	// notify Text Services
	if (pWE->tsmReference != NULL) 
	{
		ActivateTSMDocument(pWE->tsmReference);
	}
	
	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);
}

pascal Boolean WEIsActive(WEHandle hWE)
{
	// return true iff the specified WE instance is currently active
	return BTST((*hWE)->flags, weFActive) ? true : false;
}

pascal void WEScroll(long hOffset, long vOffset, WEHandle hWE)
{
	WEPtr pWE;
	Rect viewRect;
	GrafPtr savePort;
	Boolean hideOutline, saveWELock;

	// do nothing if both scroll offsets are zero
	if ((hOffset == 0) && (vOffset == 0))
		return;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;
	
	// set up the port
	GetPort(&savePort);
	SetPort(pWE->port);

	// get view rect in short coordinates
	viewRect = (*pWE->viewRgn)->rgnBBox;

	// hide the caret if it's showing
	if (BTST(pWE->flags, weFCaretVisible))
	{
		_WEBlinkCaret(hWE);
	}
	
#if WASTE_PIN_SCROLL
	// CKT Sep 12 94 Begin - Added PinScroll behavior
	if(vOffset > 0){
		// if top of the destRect would be moved below top of the viewRect
		if(pWE->destRect.top + vOffset > pWE->viewRect.top){
			vOffset += -((pWE->destRect.top + vOffset) - pWE->viewRect.top);
		}
	}else if(vOffset < 0){
		// if bottom of the destRect would be moved above bottom of the viewRect
		if(pWE->destRect.bottom + vOffset < pWE->viewRect.bottom){
			vOffset +=  -((pWE->destRect.bottom + vOffset) - pWE->viewRect.bottom);
		}
	}
	// CKT Sep 12 94 End
#endif

	// if we're inactive and outline highlighting is on, we have to temporarily
	// hide the selection outline while scrolling to avoid a cosmetic bug
	hideOutline = false;
	if (!BTST(pWE->flags, weFActive))
		if (BTST(pWE->features, weFOutlineHilite))
		{
			hideOutline = true;
			_WEHiliteRange(pWE->selStart, pWE->selEnd, hWE);
			BCLR(pWE->features, weFOutlineHilite);
		}

	// offset the destination rectangle by the specified amount
	WEOffsetLongRect(&pWE->destRect, hOffset, vOffset);

	// scroll the view rectangle
	// we use ScrollRect unless the whole text is to be redrawn anyway
	// notice that both ScrollRect and DragPreScroll take short (16-bit)
	// offset parameters, while WEScroll deals with long (32-bit) quantities
	if ((ABS(hOffset) < (viewRect.right - viewRect.left)) && (ABS(vOffset) < (viewRect.bottom - viewRect.top)))
	{
		RgnHandle updateRgn = NewRgn();

		// if we're currently tracking a drag, notify the Drag Manager we're about to scroll
		if (pWE->currentDrag != (DragReference) kNullDrag)
			DragPreScroll(pWE->currentDrag, (short) hOffset, (short) vOffset);

		// ScrollRect will set updateRgn to the region to redraw
		ScrollRect(&viewRect, (short) hOffset, (short) vOffset, updateRgn);

		if (pWE->currentDrag != (DragReference) kNullDrag)
			DragPostScroll(pWE->currentDrag);
		
		// redraw the exposed region
		WEUpdate(updateRgn, hWE);
		DisposeRgn(updateRgn);
	}
	else
	{
		// redraw the whole text
		WEUpdate(NULL, hWE);
	}

	// redraw the selection outline, if hidden
	if (hideOutline)
	{
		BSET(pWE->features, weFOutlineHilite);
		_WEHiliteRange(pWE->selStart, pWE->selEnd, hWE);
	}

	// restore the port
	SetPort(savePort);

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);
}

pascal Boolean _WEScrollIntoView (long offset, WEHandle hWE)
{
	WEPtr pWE = *hWE;
	LongPt thePoint;
	short lineHeight;
	long hScroll, vScroll, temp;
	Boolean retval;

	// do nothing if automatic scrolling is disabled
	if (!BTST(pWE->features, weFAutoScroll)) 
	{
		return false;
	}

	// find the selection point
	WEGetPoint(offset, &thePoint, &lineHeight, hWE);

	// assume no scrolling is needed
	retval = false;
	vScroll = 0;
	hScroll = 0;

	// determine if we need to scroll vertically
	if ((thePoint.v < pWE->viewRect.top) || 
		(thePoint.v + lineHeight >= pWE->viewRect.bottom))
	{
		// calculate the amount of vertical scrolling needed to center the selection into view
		vScroll = ((pWE->viewRect.top + pWE->viewRect.bottom) >> 1) -
					(thePoint.v + (lineHeight >> 1));

		// we'd like to superimpose the bottom margins of the dest/view rects, if possible
		temp = pWE->viewRect.bottom - pWE->destRect.bottom;
		if (temp > vScroll) 
		{
			vScroll = temp;
		}
		// but we also have to make sure the dest top isn't scrolled below the view top
		temp = pWE->viewRect.top - pWE->destRect.top;
		if (temp < vScroll) 
		{
			vScroll = temp;
		}
	}
	
	// determine if we need to scroll horizontally
	if ((thePoint.h - 1 < pWE->viewRect.left) || (thePoint.h >= pWE->viewRect.right))
	{ 
		// calculate the amount of horizontal scrolling needed to center the selection into view
		hScroll = ((pWE->viewRect.left + pWE->viewRect.right) >> 1) - thePoint.h;

		// we'd like to superimpose the right margins of the dest/view rects, if possible
		temp = pWE->viewRect.right - pWE->destRect.right;
		if (temp > hScroll) 
		{
			hScroll = temp;
		}
		
		// but we also have to make sure the dest left isn't scrolled to the right of the view left
		temp = pWE->viewRect.left - pWE->destRect.left;
		if (temp < hScroll) 
		{
			hScroll = temp;
		}
	}
	
	// scroll the text if necessary
	if ((vScroll != 0) || (hScroll != 0)) 
	{
		retval = true;
		WEScroll(hScroll, vScroll, hWE);
		BSET(pWE->flags, weFDestRectChanged);
	}

	// notify our client of changes to the destination rectangle
	if (BTST(pWE->flags, weFDestRectChanged))
	{
		if (pWE->scrollProc != NULL) 
		{
			CallWEScrollProc(hWE, pWE->scrollProc);
		}
		BCLR(pWE->flags, weFDestRectChanged);
	}

	// call the scroll callback, if any
	return retval;
}

pascal void WESelView(WEHandle hWE)
{
	WEPtr pWE;
	Boolean saveWELock;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// scroll the free endpoint of the selection into view
	_WEScrollIntoView(BTST(pWE->flags, weFAnchorIsEnd) ? pWE->selStart : pWE->selEnd, hWE);

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);
}
