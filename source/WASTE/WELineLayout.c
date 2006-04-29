/*
 *	WELineLayout.c
 *
 *	WASTE PROJECT
 *  Line Layout, Getting and Setting Variables, etc.
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */


#include "WASTEIntf.h"

pascal void WEStopInlineSession(WEHandle hWE)
{
	WEPtr pWE = *hWE;

// call FixTSMDocument() only if the inlint input area is actually "open"
	if ((pWE->tsmAreaStart != kInvalidOffset) && (pWE->tsmReference != NULL))
		FixTSMDocument(pWE->tsmReference);
}

INLINE pascal void _WERemoveLine(long lineIndex, WEPtr pWE)
{
	// remove the line
	_WERemoveBlock((Handle) pWE->hLines, sizeof(LineRec), lineIndex * sizeof(LineRec));

	// decrement line count
	pWE->nLines--;
}

pascal OSErr _WEInsertLine(long lineIndex, const LineRec *theLine, WEPtr pWE)
{
	// insert the specified element in the line array
	OSErr err;

	// do the insertion
	if ((err = _WEInsertBlock((Handle) pWE->hLines, theLine, sizeof(LineRec), lineIndex * sizeof(LineRec))) != noErr)
		return err;

	// increment line count
	pWE->nLines++;
	return noErr;
}

pascal void _WEBumpOrigin(long lineIndex, long deltaOrigin, WEPtr pWE)
{
	LineRec *pLine = *pWE->hLines + lineIndex;
	long nLines = pWE->nLines;

	// loop through the line run array adjusting the lineOrigin fields
	for ( ; lineIndex <= nLines; lineIndex++ )
	{
		pLine->lineOrigin += deltaOrigin;
		pLine++;
	}
}

pascal long _WEFindLineBreak(long lineStart, WEHandle hWE)
{
	// Find where to break the line beginning at lineStart
	// the current graphics port must be already set up correctly

	WEPtr pWE = *hWE;	// assume WE record is already locked
	Ptr pText;
	long offset, breakOffset;
	long textLength;
	long remainingLength;
	long segmentStart, segmentEnd;
	long runIndex;
	WERunInfo runInfo;
	Fixed pixelWidth;
	ScriptCode script, previousScript;
	Boolean isBreak = false;

	offset = lineStart;
	pText = *pWE->hText + offset;
	remainingLength = pWE->textLength - offset;

	// find the style run index corresponding to the first segment on this line
	runIndex = _WEOffsetToRun(offset, hWE);

	// initialize pixelWidth to the width of the destination rectangle, as a Fixed quantity
	pixelWidth = BSL((pWE->destRect.right - pWE->destRect.left), 16);

	// STYLE SEGMENT LOOP
	do
	{

		// get style run information for the current style run
		_WEGetIndStyle(runIndex, &runInfo, hWE);
		runIndex++;

		// set text attributes in the graphics port
		TextFont(runInfo.runAttrs.runStyle.tsFont);
		TextFace(runInfo.runAttrs.runStyle.tsFace);
		TextSize(runInfo.runAttrs.runStyle.tsSize);

		// if we're handling multiscript text, keep track of script boundaries
		if (BTST(pWE->flags, weFNonRoman))
		{ 
			// what is the script for this segment?
			script = FontToScript(runInfo.runAttrs.runStyle.tsFont);

			// have we crossed a script run boundary in the middle of a line?
			if ((runInfo.runStart > offset) && (script != previousScript))
			{
				// leave behind the all previous segments on this line
				offset = runInfo.runStart;
				pText = *pWE->hText + offset;
				remainingLength = pWE->textLength - offset;
			}
			previousScript = script;
		} // if non-Roman

		// we'll pass textLength as the second parameter to the line break hook
		// although this parameter is declared as a long, StyledLineBreak uses only
		// the low word, so make sure it doesn't trespass the 32,767 byte threshold!
		textLength = _WEPinInRange(remainingLength, 0, SHRT_MAX);

		// calculate segmentStart and segmentEnd relative to offset
		segmentStart = _WEPinInRange(runInfo.runStart - offset, 0, textLength);
		segmentEnd = _WEPinInRange(runInfo.runEnd - offset, 0, textLength);

		// set breakOffset to a non-zero value for the first script run on the line,
		// set it to zero for all subsequent script runs
		breakOffset = (offset == lineStart);

#if WASTE_OBJECTS
		if (runInfo.runAttrs.runStyle.tsObject != NULL)
		{
			// EMBEDDED OBJECT
			// subtract object width from pixelWidth
			pixelWidth -= BSL((*runInfo.runAttrs.runStyle.tsObject)->objectSize.h, 16);
			
			// stop looping if pixelWidth has gone negative
			isBreak = (pixelWidth < 0);
			breakOffset = isBreak ? segmentStart : segmentEnd;
		}
		else
#endif
		{
			// REGULAR TEXT
			isBreak = (CallWELineBreakProc(pText, textLength, segmentStart, segmentEnd,
				&pixelWidth, &breakOffset, hWE, pWE->lineBreakHook) != smBreakOverflow);
		}

		// break the line anyway when we reach the end of the text
		if (segmentEnd >= remainingLength)
			isBreak = true;

	} while (!isBreak);

	// return the offset from lineStart to the break point
	return (offset - lineStart) + breakOffset;
}

pascal void _WECalcHeights(long rangeStart, long rangeEnd, short *lineAscent, short *lineDescent,
		WEHandle hWE)
{
	// Find the maximum ascent and descent values between rangeStart and rangeEnd
	// the WE record must be already locked
	// the current graphics port must be already set up correctly

	long runIndex;
	WERunInfo runInfo;
	short runAscent, runDescent;

	*lineAscent = 1;
	*lineDescent = 0;

	// find the style run index corresponding to the first segment on this line
	runIndex = _WEOffsetToRun(rangeStart, hWE);

	// STYLE SEGMENT LOOP
	do
	{
		// get style run information for the current style run
		_WEGetIndStyle(runIndex, &runInfo, hWE);
		runIndex++;

		// calculate ascent and descent (actually, descent + leading) values for this style run

#if WASTE_OBJECTS
		if (runInfo.runAttrs.runStyle.tsObject != NULL)
		{
			// EMBEDDED OBJECT
			runAscent = (*runInfo.runAttrs.runStyle.tsObject)->objectSize.v;
			runDescent = 0;
		}
		else
#endif
		{
			// REGULAR TEXT
			runAscent = runInfo.runAttrs.runAscent;
			runDescent = runInfo.runAttrs.runHeight - runAscent;
		}
		
		// save the maximum values in lineAscent and lineDescent
		if (runAscent > *lineAscent) 
			*lineAscent = runAscent;

		if (runDescent > *lineDescent) 
			*lineDescent = runDescent;
		
		// keep looping until we reach rangeEnd
	} while (runInfo.runEnd < rangeEnd);
}

pascal OSErr _WERecalBreaks(long *startLine, long *endLine, WEHandle hWE)
{
	// Recalculates line breaks, line heights and ascents for all the text or for a portion of it.
	// On entry, startLine and endLine define a range of lines to recalculate.
	// On exit, startLine to endLine defines the range of lines actually recalculated
	// the WE record must already be locked

	WEPtr pWE = *hWE;
	LinePtr pLine;
	LineRec lineInfo, oldLineInfo;
	long lineIndex;
	long recalThreshold;
	long lineOffset;
	short lineAscent, lineDescent;
	long textHeight;
	Boolean saveTextLock;
	QDEnvironment saveEnvironment;
	OSErr err = noErr;

	// lock the text
	saveTextLock = _WESetHandleLock(pWE->hText, true);

	// find the character offset that must be necessarily reached before we can
	// even consider the possibility of stopping the recalculation process
	// this offset, recalThreshold, is the last character on endLine _before_ recalculation
	lineIndex = _WEPinInRange(*endLine, 0, pWE->nLines - 1);
	recalThreshold = (*pWE->hLines)[lineIndex + 1].lineStart;

	// we start recalculating line breaks from the line actually _preceding_ startLine,
	// since editing startLine may cause part of its text to fit on the preceding line
	lineIndex = _WEPinInRange(*startLine - 1, 0, pWE->nLines - 1);

	// find where in the text recalculation should begin
	lineInfo = (*pWE->hLines)[lineIndex];

	// save the Quickdraw environment
	_WESaveQDEnvironment(pWE->port, false, &saveEnvironment);

	// MAIN LINE BREAKING LOOP
	do
	{
		// find where to break the current line
		lineOffset = _WEFindLineBreak(lineInfo.lineStart, hWE);

		// make sure we advance at least by one character (unless we reached the end of text)
		if ((lineOffset <= 0) && (lineInfo.lineStart < pWE->textLength))
			lineOffset = 1;
		
		// calculate ascent and descent values for this line
		_WECalcHeights(lineInfo.lineStart, lineInfo.lineStart + lineOffset, &lineAscent, &lineDescent, hWE);

		// save the maximum line ascent for this line in the line array
		pLine = *pWE->hLines + lineIndex;
		pLine->lineAscent = lineAscent;

		// increment counters (go to the next line array entry)
		lineIndex++;
		lineInfo.lineStart += lineOffset;
		lineInfo.lineOrigin += (lineAscent + lineDescent);
		pLine++;
		
		// compare the newly calculated line start with the old value
		// if the new line start comes before the old line start, insert a new element
		oldLineInfo = *pLine;
		if ((lineIndex > pWE->nLines) || (lineInfo.lineStart < oldLineInfo.lineStart)) 
		{
			if ((err = _WEInsertLine(lineIndex, &lineInfo, pWE)) != noErr)
				goto cleanup;
		}
		else
		{
			// overwrite the old element
			pLine->lineStart = lineInfo.lineStart;
			pLine->lineOrigin = lineInfo.lineOrigin;

			// remove all further elements which have a lineStart field
			// less than or equal to the current one
			while((lineIndex < pWE->nLines) && (lineInfo.lineStart >= (pLine + 1)->lineStart))
				_WERemoveLine(lineIndex + 1, pWE);

			// if the new line start is the same as the old one...
			if (lineInfo.lineStart == oldLineInfo.lineStart) 
			{
				// ...and recalThreshold has been reached, we can stop recalculating line breaks
				if (lineInfo.lineStart >= recalThreshold) 
				{
					// although line breaks need not be changed from lineIndex on,
					// the lineOrigin fields may need to be changed
					if (lineInfo.lineOrigin != oldLineInfo.lineOrigin) 
						_WEBumpOrigin(lineIndex + 1, lineInfo.lineOrigin - oldLineInfo.lineOrigin, pWE);
					
					// exit from the line breaking loop
					goto cleanup;
				}
			}
			else
			{
				// otherwise, the new line start comes after the old line start...
				// if the current line is the one preceding startLine, warn our caller about this
				if ((lineIndex > 0) && (lineIndex == *startLine)) 
					*startLine = lineIndex - 1;
			}
		}
	} while(lineInfo.lineStart < pWE->textLength);
	
cleanup:
	// calculate total text height
	textHeight = WEGetHeight(0, pWE->nLines, hWE);

	// quirk: if the last character in the text is a carriage return, the caret appears
	// below the last line, so in this case we need to add the extra height to textHeight
	if (WEGetChar(pWE->textLength - 1, hWE) == kEOL)
		textHeight += WEGetHeight(pWE->nLines - 1, pWE->nLines, hWE);
	
	// if total text height has changed, remember to call the scroll callback later
	if (textHeight != pWE->destRect.bottom - pWE->destRect.top)
		BSET(pWE->flags, weFDestRectChanged);
	
	// set destRect.bottom to destRect.top + total text height
	pWE->destRect.bottom = pWE->destRect.top + textHeight;

	// return through endLine the index of the last line affected by recalculation
	*endLine = lineIndex - 1;

	// make sure startLine isn't greater than endLine
	if (*startLine > *endLine) 
		*startLine = *endLine;
	
	// unlock the text
	_WESetHandleLock(pWE->hText, saveTextLock);

	// restore the Quickdraw environment
	_WERestoreQDEnvironment(&saveEnvironment);

	// return result code
	return err;
}

static Boolean SLCalcSlop(LinePtr pLine, const WERunAttributes *pAttrs,
		Ptr pSegment, long segmentStart, long segmentLength,
		JustStyleCode styleRunPosition, void *callbackData);

static Boolean SLCalcSlop(LinePtr pLine, const WERunAttributes *pAttrs,
		Ptr pSegment, long segmentStart, long segmentLength,
		JustStyleCode styleRunPosition, void *callbackData)
{
	struct SLCalcSlopData *p = (struct SLCalcSlopData *) callbackData;
	short segmentWidth;
	Fixed segmentProportion;
	Boolean isEndOfLine;

	// see if this text segment ends with a carriage return, or if we've reached the
	// end of the text (in which case we don't want any justification to take place)
	isEndOfLine = (segmentStart + segmentLength >= p->pWE->textLength) ||
					( *((Ptr)pSegment + segmentLength - 1) == kEOL);

	// if this is the first segment on the line, reset line totals
	if (IS_FIRST_RUN(styleRunPosition)) 
	{
		p->totalSlop = p->lineWidth;
		p->totalProportion = 0;
	}

#if WASTE_OBJECTS
	if (pAttrs->runStyle.tsObject != NULL)
	{
		// EMBEDDED OBJECT
		// segment width is just object width; no extra space can be applied for justification
		segmentWidth = (*pAttrs->runStyle.tsObject)->objectSize.h;
		segmentProportion = 0;
	}
	else
#endif
	{
		// REGULAR TEXT

		// if this is the last segment on the line, strip trailing spaces
		if (IS_LAST_RUN(styleRunPosition))
		{
			segmentLength = VisibleLength(pSegment, segmentLength);
		}
		// measure this segment
		segmentWidth = TextWidth(pSegment, 0, segmentLength);

		// calculate the proportion of extra space to apply to this text segment
		segmentProportion = PortionLine(pSegment, segmentLength,
							styleRunPosition, kOneToOneScaling, kOneToOneScaling);
	}
	
	// keep track of line totals
	p->totalSlop -= segmentWidth;
	p->totalProportion += segmentProportion;

	// if this is the last segment on the line, save values in the line array
	if (IS_LAST_RUN(styleRunPosition))
	{
		// make sure slop is non-negative
		if (p->totalSlop < 0)
			p->totalSlop = 0;
		pLine->lineSlop = p->totalSlop;

		if (isEndOfLine) 
		{
			pLine->lineJustAmount = 0;
		}
		else
		{
			pLine->lineJustAmount = FixDiv(BSL(p->totalSlop, 16), p->totalProportion);
		}
	}
	return false;	// keep looping
}

pascal void _WERecalSlops(long firstLine, long lastLine, WEHandle hWE)
{
	// Calculates the lineSlop and lineJustAmount fields
	// of the line array for the specified lines

	WEPtr pWE = *hWE;
	struct SLCalcSlopData callbackData;

	// we only need to bother if the user isn't using left justification
	if (pWE->alignment == weFlushLeft)
		return;

	// calculate slop and normalized slop proportion for all lines
	callbackData.pWE = pWE;
	callbackData.lineWidth = pWE->destRect.right - pWE->destRect.left;
	_WESegmentLoop(firstLine, lastLine, SLCalcSlop, (void *) &callbackData, hWE);
}

pascal WEAlignment WEGetAlignment(WEHandle hWE)
{
	return (*hWE)->alignment;
}

pascal void WEGetSelection(long *selStart, long *selEnd, WEHandle hWE)
{
	WEPtr pWE = *hWE;

	*selStart = pWE->selStart;
	*selEnd = pWE->selEnd;
}

pascal void WESetDestRect(const LongRect *destRect, WEHandle hWE)
{
	(*hWE)->destRect = *destRect;
}

pascal void WEGetDestRect(LongRect *destRect, WEHandle hWE)
{
	*destRect = (*hWE)->destRect;
}

pascal void WESetViewRect(const LongRect *viewRect, WEHandle hWE)
{
	WEPtr pWE = *hWE;
	Rect r;
	
	pWE->viewRect = *viewRect;

	// keep the viewRgn in sync with the view rectangle
	WELongRectToRect(viewRect, &r);
	RectRgn(pWE->viewRgn, &r);
}

pascal void WEGetViewRect(LongRect *viewRect, WEHandle hWE)
{
	*viewRect = (*hWE)->viewRect;
}

pascal long WEGetTextLength(WEHandle hWE)
{
	return (*hWE)->textLength;
}

pascal long WECountLines(WEHandle hWE)
{
	return (*hWE)->nLines;
}

pascal long WEGetHeight(long startLine, long endLine, WEHandle hWE)
{
	WEPtr pWE = *hWE;
	LineArrayPtr pLines = *pWE->hLines;
	long nLines = pWE->nLines;

	startLine = _WEPinInRange(startLine, 0, nLines);
	endLine = _WEPinInRange(endLine, 0, nLines);
	_WEReorder(&startLine, &endLine);
	return pLines[endLine].lineOrigin - pLines[startLine].lineOrigin;
}

pascal void WEGetLineRange(long lineNo, long *lineStart, long *lineEnd, WEHandle hWE)
{
	WEPtr pWE = *hWE;
	LineRec *pLine;
	
	lineNo = _WEPinInRange(lineNo, 0, pWE->nLines - 1);
	pLine = *pWE->hLines + lineNo;
	if (lineStart != NULL)
		*lineStart = pLine[0].lineStart;
	if (lineEnd != NULL)
		*lineEnd = pLine[1].lineStart;
}

pascal Handle WEGetText(WEHandle hWE)
{
	return (*hWE)->hText;
}

pascal short WEGetChar(long offset, WEHandle hWE)
{
	WEPtr pWE = *hWE;

	// sanity check: make sure offset is withing allowed bounds
	if ((offset < 0) || (offset >= pWE->textLength))
	{
		return 0;
	}

	// get the specified character (actually, byte)
	return * (unsigned char *) (*pWE->hText + offset);
}