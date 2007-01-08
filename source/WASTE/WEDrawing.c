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
 *	WEDrawing.c
 *
 *	WASTE PROJECT
 *  Drawing routines and other basic support functions
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */

#include "WASTEIntf.h"

const Point kOneToOneScaling = { 1, 1 };

pascal long WEOffsetToLine (long offset, WEHandle hWE)
{
	// given a byte offset into the text, find the corresponding line index

	WEPtr pWE = *hWE;
	LineArrayPtr pLines = *pWE->hLines;
	long minIndex, maxIndex, index;

	// do a fast binary search through the line array
	minIndex = 0;
	maxIndex = pWE->nLines;

	while (minIndex < maxIndex)
	{
		index = (minIndex + maxIndex) >> 1;
		if (offset >= pLines[index].lineStart) 
		{
			if (offset < pLines[index + 1].lineStart) 
			{
				break;
			}
			else
			{
				minIndex = index + 1;
			}
		}
		else
		{
			maxIndex = index;
		}
	}

	return index;
}

pascal long _WEPixelToLine(long vOffset, WEHandle hWE)
{
	// given a vertical pixel offset in local coordinates,
	// find the corresponding line index

	WEPtr pWE = *hWE;
	LineArrayPtr pLines = *pWE->hLines;
	long minIndex, maxIndex, index;

	// do a fast binary search through the line array
	minIndex = 0;
	maxIndex = pWE->nLines;

	while (minIndex < maxIndex)
	{
		index = (minIndex + maxIndex) >> 1;
		if (vOffset >= pLines[index].lineOrigin) 
		{
			if (vOffset < pLines[index + 1].lineOrigin) 
			{
				break;
			}
			else
			{
				minIndex = index + 1;
			}
		}
		else
		{
			maxIndex = index;
		}
	}
	
	return index;
}

pascal long _WEOffsetToRun (long offset, WEHandle hWE)
{
	// given a byte offset into the text, find the corresponding style run index

	WEPtr pWE = *hWE;
	RunArrayPtr pRuns = *pWE->hRuns;
	long minIndex, maxIndex, index;

	// do a fast binary search through the style run array
	minIndex = 0;
	maxIndex = pWE->nRuns;

	while (minIndex < maxIndex)
	{
		index = (minIndex + maxIndex) >> 1;
		if (offset >= pRuns[index].runStart)
		{ 
			if (offset < pRuns[index + 1].runStart) 
			{
				break;
			}
			else
			{
				minIndex = index + 1;
			}
		}
		else
		{
			maxIndex = index;
		}
	}

	return index;
}

pascal void _WEGetIndStyle(long runIndex, WERunInfo *info, WEHandle hWE)
{
	WEPtr pWE = *hWE;	// assume WE record is already locked
	RunArrayElementPtr pRun;

	// get a pointer to the specified run array element
	pRun = *pWE->hRuns + runIndex;

	// fill in the runStart and runEnd fields from the style run array
	info->runStart = pRun[0].runStart;
	info->runEnd = pRun[1].runStart;

	// copy the style information from the appropriate entry in the style table
	info->runAttrs = (*pWE->hStyles)[pRun->styleIndex].info;
}

pascal void WEGetRunInfo(long offset, WERunInfo *info, WEHandle hWE)
{
	_WEGetIndStyle(_WEOffsetToRun(offset, hWE), info, hWE);
}

#if WASTE_OBJECTS

pascal OSErr WEGetSelectedObject(WEObjectDescHandle *hObjectDesc, WEHandle hWE)
{
	WEPtr pWE = *hWE;
	WERunInfo runInfo;

	// assume current selection is not an embedded object
	*hObjectDesc = NULL;

	// check selection range
	if (pWE->selEnd - pWE->selStart == 1)
	{
		// check run info
		WEGetRunInfo(pWE->selStart, &runInfo, hWE);
		if ((*hObjectDesc = runInfo.runAttrs.runStyle.tsObject) != NULL)
			return noErr;
	}
	return weObjectNotFoundErr;
} // WEGetSelectedObject

pascal long WEFindNextObject(long offset, WEObjectDescHandle *hObjectDesc, WEHandle hWE)
{
	WEPtr pWE = *hWE;
	StyleTablePtr pStyles = *pWE->hStyles;
	WEObjectDescHandle obj;
	RunArrayElementPtr pRun;
	
	*hObjectDesc = NULL;

	// do nothing if offset is already at the end of the text
	if (offset >= pWE->textLength - 1)
		return kInvalidOffset;

	// get a pointer to the run array element immediately following offset
	pRun = *pWE->hRuns + _WEOffsetToRun(offset + 1, hWE);

	// perform a linear scan of the run array looking for a run whose
	// corresponding style table entry points to an embedded object;
	// the search will stop anyway because the last run array element has styleIndex = -1
	while (pRun->styleIndex >= 0)
	{
		if ((obj = pStyles[pRun->styleIndex].info.runStyle.tsObject) != NULL)
		{
			*hObjectDesc = obj;
			return pRun->runStart;
		}
		pRun++;
	}

	return kInvalidOffset;

} // WEFindNextObject

#endif  // WASTE_OBJECTS

pascal void _WEContinuousStyleRange(long rangeStart, long rangeEnd, WEStyleMode *mode,
		WETextStyle *ts, WEHandle hWE)
{
	// find out which style attributes are continous over the specified text range
	// on entry, *mode specifies which attributes are to be checked
	// on exit, *mode specifies the continuous attributes, also copied to ts

	WEPtr pWE = *hWE;
	WEStyleMode outMode;
	long runIndex;
	WERunInfo runInfo;

	// get style attributes to check (masking out private and unused bits)
	outMode = *mode & weDoAll;

	// get style info at the beginning of the specified range
	runIndex = _WEOffsetToRun(rangeStart, hWE);
	_WEGetIndStyle(runIndex, &runInfo, hWE);

	// copy the specified fields to ts
	_WECopyStyle(&runInfo.runAttrs.runStyle, (WETextStyle *) ts, normal, outMode | weDoReplaceFace);

	// loop through style runs across the current selection range
	// if we determine that all specified attributes are discontinuous, we exit prematurely
	do
	{
		_WEGetIndStyle(runIndex, &runInfo, hWE);

		// determine which attributes have changed, if any
		if (outMode & weDoFont)
		{ 
			if (runInfo.runAttrs.runStyle.tsFont != ts->tsFont)
			{
				outMode &= (~weDoFont);
			}
		}
		if (outMode & weDoFace)
		{ 
			if (runInfo.runAttrs.runStyle.tsFace != ts->tsFace)
			{ 
				ts->tsFace &= runInfo.runAttrs.runStyle.tsFace;
				if (ts->tsFace == 0)
				{
					outMode &= (~weDoFace);
				}
			}
		}
		if (outMode & weDoSize)
		{ 
			if (runInfo.runAttrs.runStyle.tsSize != ts->tsSize) 
			{
				outMode &= (~weDoSize);
			}
		}
		if (outMode & weDoColor)
		{ 
			if (!_WEBlockCmp(&runInfo.runAttrs.runStyle.tsColor, &ts->tsColor, sizeof(RGBColor)))
			{
				outMode &= (~weDoColor);
			}
		}

		runIndex++;
	} while ((outMode != 0) && (runInfo.runEnd < rangeEnd));

	*mode = outMode;
}

pascal void _WESynchNullStyle(WEHandle hWE)
{
	// This routine fills the nullStyle field of the WE record with valid information
	// and makes sure that the null style font belongs to the keyboard script.

	WEPtr pWE = *hWE;	// assume WE record is already locked
	long runIndex;
	WERunInfo runInfo;
#if !WASTE_NO_SYNCH
	ScriptCode keyboardScript;
	short fontID;
#endif

	// find the run index of the style run preceding the insertion point
	runIndex = _WEOffsetToRun(pWE->selStart - 1, hWE);

	// if the nullStyle record is marked as invalid, fill it with the style attributes
	// associated with the character preceding the insertion point, and mark it as valid
	if (!BTST(pWE->flags, weFUseNullStyle)) 
	{
		_WEGetIndStyle(runIndex, &runInfo, hWE);
		pWE->nullStyle = runInfo.runAttrs;
		BSET(pWE->flags, weFUseNullStyle);
	}

#if !WASTE_NO_SYNCH
	// if only the Roman script is installed, we're finished
	if (!BTST(pWE->flags, weFNonRoman)) 
	{
		return;
	}

	// *** FONT / KEYBOARD SYNCHRONIZATION ***
	// get the keyboard script
	keyboardScript = GetScriptManagerVariable(smKeyScript);

	// find out what font will be used for the next character typed
	fontID = pWE->nullStyle.runStyle.tsFont;

	// do nothing if the font script is the same as the keyboard script
	if (FontToScript(fontID) == keyboardScript) return; 

	// scan style runs starting from the insertion point backwards,
	// looking for the first font belonging to the keyboard script
	do
	{
		_WEGetIndStyle(runIndex, &runInfo, hWE);
		fontID = runInfo.runAttrs.runStyle.tsFont;
		if (FontToScript(fontID) == keyboardScript) break;
		runIndex--;
	} while (runIndex>=0);
	
	// if no font was ever used for the keyboard script, default to the
	// application font for the script
	if (runIndex < 0) 
	{
		fontID = GetScriptVariable(keyboardScript, smScriptAppFond);
	}
	
	// change the font in the null style record
	pWE->nullStyle.runStyle.tsFont = fontID;
#endif
}

pascal Boolean WEContinuousStyle(WEStyleMode *mode, TextStyle *ts, WEHandle hWE)
{
	// find out which style attributes are continous over the selection range
	// on entry, the mode bitmap specifies which attributes are to be checked
	// on exit, the mode bitmap specifies the continuous attributes, also copied to ts
	// return true if all specified attributes are continuous

	WEPtr pWE;
	WEStyleMode oldMode;
	Boolean continuousStyle;
	Boolean saveWELock;

	// lock the WE record
	
	pWE = *hWE;
	saveWELock = _WESetHandleLock((Handle) hWE, true);

	// mask out private and unused bits in mode so we don't run the risk of overwriting
	// memory past the ts record (which is defined as TextStyle in the public interfaces)
	*mode &= weDoAll;

	// two rather different paths are taken depending on whether
	// the selection range is empty or not
	if (pWE->selStart == pWE->selEnd) 
	{
		// if the selection range is empty, always return true and set ts
		// from the nullStyle record, after having validated it
		continuousStyle = true;
		_WESynchNullStyle(hWE);
		_WECopyStyle(&pWE->nullStyle.runStyle, (WETextStyle *) ts, normal, *mode | weDoReplaceFace);
	}
	else
	{
		// otherwise get the continuous style attributes over the selection range
		oldMode = *mode;
		_WEContinuousStyleRange(pWE->selStart, pWE->selEnd, mode, (WETextStyle *) ts, hWE);

		// return true if mode hasn't changed
		continuousStyle = (oldMode == *mode);
	}

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);	

	return continuousStyle;
}

pascal void _WESegmentLoop(long firstLine, long lastLine, SegmentLoopProcPtr callback, void *callbackData,
		WEHandle hWE)
{
	// For each style segment on every line in the specified range, set up
	// text attributes in the port and call the callback.
	// the WE record must be already locked

	WEPtr pWE = *hWE;
	LinePtr pLine;
	Ptr pText;
	long lineIndex;
	long runIndex, previousRunIndex;
	long lineStart, lineEnd, segmentStart, segmentEnd;
	JustStyleCode styleRunPosition;
	WERunInfo runInfo;
	Boolean saveLineLock;
	Boolean saveTextLock;
	QDEnvironment saveEnvironment;

	// save the Quickdraw environment
	_WESaveQDEnvironment(pWE->port, BTST(pWE->flags, weFHasColorQD) ? true : false, &saveEnvironment);

	// make sure firstLine and lastLine are within the allowed range
	lineIndex = pWE->nLines - 1;
	firstLine = _WEPinInRange(firstLine, 0, lineIndex);
	lastLine = _WEPinInRange(lastLine, 0, lineIndex);

	// lock the line array
	saveLineLock = _WESetHandleLock((Handle) pWE->hLines, true);
	pLine = *pWE->hLines + firstLine;

	// lock the text
	saveTextLock = _WESetHandleLock(pWE->hText, true);
	pText = *pWE->hText;

	// find the style run index corresponding to the beginning of the first line
	runIndex = _WEOffsetToRun(pLine->lineStart, hWE);
	previousRunIndex = -1;

	// loop thru the specified lines
	for ( lineIndex = firstLine; lineIndex <= lastLine; lineIndex++ )
	{
		// get line start and line end
		lineStart = pLine[0].lineStart;
		lineEnd = pLine[1].lineStart;

		// loop thru each style run on this line
		do
		{
			// get style run information for the current style run
			_WEGetIndStyle(runIndex, &runInfo, hWE);

			if (previousRunIndex != runIndex) 
			{
				// set new text attributes
				TextFont(runInfo.runAttrs.runStyle.tsFont);
				TextFace(runInfo.runAttrs.runStyle.tsFace);
				TextSize(runInfo.runAttrs.runStyle.tsSize);

				// remember previous run index
				previousRunIndex = runIndex;
			}

			// determine the relative position of this style run on the line
			styleRunPosition = 0;					// onlyStyleRun

			if (runInfo.runStart <= lineStart)
			{ 
				segmentStart = lineStart;
			}
			else
			{
				styleRunPosition += 2;				// rightStyleRun or middleStyleRun
				segmentStart = runInfo.runStart;
			}

			if (runInfo.runEnd >= lineEnd)
			{
						segmentEnd = lineEnd;
			}
			else
			{
				styleRunPosition += 1;				// leftStyleRun or middleStyleRun
				segmentEnd = runInfo.runEnd;
			}

			// do the callback
			if ((callback)(pLine, &runInfo.runAttrs, pText + segmentStart,
					segmentStart, segmentEnd - segmentStart, styleRunPosition, callbackData))
			{
				break;
			}

			// advance style run index, unless this style run goes on to the next line
			if (runInfo.runEnd <= lineEnd)
			{ 
				runIndex++;
			}
		} while (runInfo.runEnd < lineEnd);
		
		pLine++;

	}  // for

	// unlock the text
	_WESetHandleLock(pWE->hText, saveTextLock);	

	// unlock the line array
	_WESetHandleLock((Handle) pWE->hLines, saveLineLock);

	// restore the Quickdraw environment
	_WERestoreQDEnvironment(&saveEnvironment);
}

pascal void _WEDrawTSMHilite(Rect *segmentRect, short tsFlags)
{
	short underlineHeight;
	RGBColor background, foreground, saveForeground;
	Boolean isColorPort;
	Boolean usingTrueGray;

	isColorPort = (((CGrafPtr)(qd.thePort))->portVersion < 0);
	usingTrueGray = false;

	// by default, the pen pattern is solid
	PenPat(&qd.black);

	// if we're drawing in color, set the foreground color
	if (isColorPort) 
	{
		// save foreground color
		GetForeColor(&saveForeground);

		// by default, the foreground color is black
		foreground.red = 0;
		foreground.green = 0;
		foreground.blue = 0;

		// if we're underlining raw (unconverted) text, see if a "true gray" is available
		if (!BTST(tsFlags, tsTSMConverted)) 
		{
			GetBackColor(&background);
			usingTrueGray = GetGray(GetGDevice(), &background, &foreground);
		} // if raw text

		// set the foreground color
		RGBForeColor(&foreground);
	} // if color graf port

	// if we're underlining raw (unconverted) text and no true gray is available,
	// simulate gray with a 50% pattern
	if (!BTST(tsFlags, tsTSMConverted)) 
	{
		if (!usingTrueGray)
		{ 
			PenPat(&qd.gray);
		}
	}
	// use a 2-pixel tall underline if text is "selected", else use a 1-pixel tall underline
	underlineHeight = BTST(tsFlags, tsTSMSelected) ? 2 : 1;
	
	// segmentRect becomes the rectangle to paint
	InsetRect(segmentRect, 1, 0);
	segmentRect->top = segmentRect->bottom - underlineHeight;

	// draw the underline
	PaintRect(segmentRect);

	// restore the foreground color
	if (isColorPort) 
	{
		RGBForeColor(&saveForeground);
	}
}

static Boolean SLDraw (LinePtr pLine, const WERunAttributes *pAttrs,
		Ptr pSegment, long segmentStart, long segmentLength,
		JustStyleCode styleRunPosition, void *callbackData);

static Boolean SLDraw (LinePtr pLine, const WERunAttributes *pAttrs,
		Ptr pSegment, long segmentStart, long segmentLength,
		JustStyleCode styleRunPosition, void *callbackData)
{
#pragma unused(segmentStart)
	struct SLDrawData *p = (struct SLDrawData *) callbackData;
	WEPtr pWE = *p->hWE;
	Fixed slop;
	Rect segmentRect;
	RGBColor theColorBlack;

	// is this the first segment on this line?
	if (IS_FIRST_RUN(styleRunPosition))
	{
		// calculate the line rectangle (the rectangle which completely encloses the current line)
		(p->lineRect).left = pWE->destRect.left;
		(p->lineRect).right = pWE->destRect.right;
		(p->lineRect).top = pWE->destRect.top + pLine[0].lineOrigin;
		(p->lineRect).bottom = pWE->destRect.top + pLine[1].lineOrigin;

		// calculate the visible portion of this rectangle
		// we do this by intersecting the line rectangle with the view rectangle
		p->drawRect = (*pWE->viewRgn)->rgnBBox;
		if (SectRect(&(p->lineRect), &(p->drawRect), &(p->drawRect)))
		{ 
			;
		}
		if (p->usingOffscreen) 
		{
			// calculate the boundary rectangle for the offscreen buffer
			// this is simply drawRect converted to global coordinates
			p->bounds = p->drawRect;
			LocalToGlobal((Point *) &p->bounds.top);
			LocalToGlobal((Point *) &p->bounds.bottom);

			// update the offscreen graphics world for the new bounds (this could fail)
			p->drawingOffscreen = false;
			if (UpdateGWorld((GWorldPtr *) (&pWE->offscreenPort), 0, &p->bounds, (CTabHandle) NULL,
				(GDHandle) NULL, (GWorldFlags) 0) >= 0)
			{
				// NOTE: when running on a 68000 machine with the original Quickdraw,
				// a GWorld is just an extended GrafPort, and GetGWorldPixMap actually
				// returns a handle to a _copy_ of the GrafPort portBits (a BitMap, not a PixMap).
				// An important side-effect of this is that when we call SetOrigin,
				// only the original portBits is offset, not the copy.
				// get the pixel map associated with the offscreen graphics world
				p->offscreenPixels = GetGWorldPixMap((GWorldPtr)(pWE->offscreenPort));

				// lock it down
				if (LockPixels(p->offscreenPixels)) 
				{
					// offscreen pixel buffer allocation was successful
					p->drawingOffscreen = true;

					// switch graphics world
					SetGWorld((GWorldPtr)(pWE->offscreenPort), NULL);

					// synchronize the coordinate system of the offscreen port with that of the screen port
					SetOrigin(p->drawRect.left, p->drawRect.top);

					// reset the offscreen clip region
					ClipRect(&p->drawRect);
				}
			} // if pixel buffer allocation was successful
		} // if usingOffscreen

		// if doErase is true, erase the drawable area before drawing text
		if (p->doErase) 
		{
			EraseRect(&p->drawRect);
		}
		// position the pen
		MoveTo(p->lineRect.left + _WECalcPenIndent(pLine->lineSlop, pWE->alignment),
			p->lineRect.top + pLine->lineAscent);
	} // if first segment on line

	// if drawingOffscreen, switch thePort to the offscreen port
	// and synchronize text attributes
	if (p->drawingOffscreen) 
	{
		SetPort(pWE->offscreenPort);
		TextFont(pAttrs->runStyle.tsFont);
		TextFace(pAttrs->runStyle.tsFace);
		TextSize(pAttrs->runStyle.tsSize);
	} // if drawingOffscreen

	// get horizontal coordinate of the pen before drawing the segment
	GetPen((Point *)&segmentRect.top);

	// set the foreground color
	if (p->usingColor)
	{
		RGBForeColor(&pAttrs->runStyle.tsColor);
	}

#if WASTE_OBJECTS
	if (pAttrs->runStyle.tsObject != NULL)
	{
		_WEDrawObject(pAttrs->runStyle.tsObject);
	}
	else
#endif
	{
		slop = 0;

		// calculate the "slop" (extra space) for this text segment (justified text only)
		if (pWE->alignment == weJustify) 
		{
			// if this is the last segment on the line, strip trailing spaces
			if (IS_LAST_RUN(styleRunPosition))
			{
				segmentLength = VisibleLength(pSegment, segmentLength);
			}
			// calculate how much extra space is to be applied to this text segment
			slop = FixMul(PortionLine(pSegment, segmentLength, styleRunPosition, 
				kOneToOneScaling, kOneToOneScaling), pLine->lineJustAmount);

		} // if alignment = weJustify

		// draw the segment
		CallWEDrawTextProc(pSegment, segmentLength, slop, styleRunPosition, p->hWE, pWE->drawTextHook);
	}
	
	// get horizontal coordinate of the pen after drawing the segment
	GetPen((Point *)&segmentRect.bottom);
	segmentRect.bottom = p->lineRect.bottom;

	// if this segment is in the TSM area, underline it in the appropriate way
	if (BTST(pAttrs->runStyle.tsFlags, tsTSMHilite)) 
	{
		_WEDrawTSMHilite(&segmentRect, pAttrs->runStyle.tsFlags);
	}
	if (p->drawingOffscreen) 
	{
		if (IS_LAST_RUN(styleRunPosition)) 
		{
			// after drawing offscreen the last segment,
			// prepare to copy the offscreen buffer to video RAM

			// first set the graphics world to the screen port
			SetGWorld((CGrafPtr)p->screenPort, p->screenDevice);

			// before calling CopyBits, set the foreground color to black to avoid colorization (color only)
			if (p->usingColor) 
			{
				theColorBlack.red = 0;
				theColorBlack.green = 0;
				theColorBlack.blue = 0;
				RGBForeColor(&theColorBlack);
			}
			
			// copy the offscreen image of the [visible portion of the] line to the screen
			CopyBits(&pWE->offscreenPort->portBits, &p->screenPort->portBits, &p->drawRect,
					&p->drawRect, srcCopy, (RgnHandle)NULL);

			// restore the original offscreen coordinate system and unlock the pixel image
			SetPort(pWE->offscreenPort);
			SetOrigin(0, 0);
			if (p->usingColor)
				RGBForeColor(&theColorBlack);	// this fixes a bug in Style 1.3
			UnlockPixels(p->offscreenPixels);

		} // if last segment

		// restore the screen port for _WESegmentLoop
		SetPort(p->screenPort);
	} // if drawingOffscreen

	return false;	// keep looping
}

pascal void _WEDrawLines (long firstLine, long lastLine, Boolean doErase, WEHandle hWE)
{
	// draw the specified range of lines
	// we can safely assume that the WE record is already locked
	// and the port is already set the pWE->port

	WEPtr pWE = *hWE;					// assume WE record is locked
	Rect bounds;						// bounds of the offscreen buffer, in global coordinates
	Boolean usingColor;					// true if we're drawing in color
	Boolean usingOffscreen;				// true if we're using an offscreen port
	Boolean drawingOffscreen;			// true if actually drawing to an offscreen buffer
	struct SLDrawData callbackData;
	GDHandle screenDevice;
	GrafPtr screenPort;

	usingOffscreen = false;
	drawingOffscreen = false;
	usingColor = (BTST(pWE->flags, weFHasColorQD) &&
				 !BTST(pWE->features, weFInhibitColor)) ? true : false;
		
	// do nothing if our graphics port is not visible
	if (EmptyRgn(pWE->port->visRgn))
	{
		return;
	}

	// save graphics world
	GetGWorld((GWorldPtr *)&screenPort, &screenDevice);

	// If doErase is true, we're drawing over old text, so we must erase each line
	// before redrawing it.  But if the weFDrawOffscreen feature is enabled, we draw
	// the entire line offscreen and  we copy the image right over the old line,
	// without erasing it, thus achieving a very smooth drawing effect.

	if ((doErase) && BTST(pWE->features, weFDrawOffscreen)) 
	{
		// has an offscreen world already been allocated?
		if (pWE->offscreenPort == NULL) 
		{
			GWorldPtr aGWorld; // mf
			// nope,  create one; its bounds are set initially to an arbitrary rectangle
			SetRect(&bounds, 0, 0, 1, 1);
			if (NewGWorld(&aGWorld, 0, &bounds, NULL, NULL,
				pixPurge + noNewDevice + useTempMem) != noErr)
			{ ; }
			(pWE->offscreenPort)=(GrafPtr)aGWorld; // mf
		}
		usingOffscreen = (pWE->offscreenPort != NULL);
	}

	callbackData.hWE = hWE;
	callbackData.bounds = bounds;
	callbackData.usingColor = usingColor;
	callbackData.usingOffscreen = usingOffscreen;
	callbackData.drawingOffscreen = drawingOffscreen;
	callbackData.doErase = doErase;
	SetRect(&callbackData.lineRect,0,0,0,0);				// mf
	SetRect(&callbackData.drawRect,0,0,0,0);				// mf
	callbackData.offscreenPixels = NULL;
	callbackData.screenPort = screenPort;
	callbackData.screenDevice = screenDevice;

	_WESegmentLoop(firstLine, lastLine, SLDraw, (void *) &callbackData, hWE);

	// restore graphics world
	SetGWorld((GWorldPtr)screenPort, screenDevice);

}

pascal short _WECalcPenIndent(short slop, WEAlignment alignment)
{
	short retval = 0;				// left aligned or justified

	// if alignment is weFlushDefault, align according SysDirection
	if (((alignment == weFlushDefault) && (GetSysDirection() != 0)) ||
		 (alignment == weFlushRight))
	{
		retval = slop;				// right aligned
	}
	else if (alignment == weCenter)
	{
		retval = slop / 2;			// centered
	}

	return retval;
}

pascal void _WESaveQDEnvironment(GrafPtr port, Boolean saveColor, QDEnvironment *theEnvironment)
{
	GetPort(&theEnvironment->envPort);
	SetPort(port);
	GetPenState(&theEnvironment->envPen);
	PenNormal();
	theEnvironment->envStyle.tsFont = port->txFont;
	theEnvironment->envStyle.tsFace = port->txFace;
	theEnvironment->envStyle.tsFlags = saveColor;		// remember if color was saved
	theEnvironment->envStyle.tsSize = port->txSize;
	if (saveColor) 
	{
		GetForeColor(&theEnvironment->envStyle.tsColor);
	}
	theEnvironment->envMode = port->txMode;
	TextMode(srcOr);
}

pascal void _WERestoreQDEnvironment(const QDEnvironment *theEnvironment)
{
	SetPenState(&theEnvironment->envPen);
	TextFont(theEnvironment->envStyle.tsFont);
	TextFace(theEnvironment->envStyle.tsFace);
	TextSize(theEnvironment->envStyle.tsSize);
	TextMode(theEnvironment->envMode);
	if (theEnvironment->envStyle.tsFlags) 
	{
		RGBForeColor(&theEnvironment->envStyle.tsColor);
	}
	SetPort(theEnvironment->envPort);
}

pascal void _WEFillFontInfo (GrafPtr port, WERunAttributes *targetStyle)
{
	// given a WERunAttributes record, fill in the runHeight, runAscent fields etc.
	FontInfo fInfo;
	QDEnvironment saveEnvironment;

	_WESaveQDEnvironment(port, false, &saveEnvironment);

	// we don't want a zero font size; although Quickdraw accepts zero to mean
	// the default font size, it can cause trouble to us when we do calculations
	if (targetStyle->runStyle.tsSize == 0) 
		targetStyle->runStyle.tsSize = 12;

	// set the text attributes
	TextFont(targetStyle->runStyle.tsFont);
	TextSize(targetStyle->runStyle.tsSize);
	TextFace(targetStyle->runStyle.tsFace);
	GetFontInfo(&fInfo);
	targetStyle->runHeight = fInfo.ascent + fInfo.descent + fInfo.leading;
	targetStyle->runAscent = fInfo.ascent;

	_WERestoreQDEnvironment(&saveEnvironment);
}

pascal void _WECopyStyle(const WETextStyle *sourceStyle, WETextStyle *targetStyle,
				Style offStyles, WEStyleMode mode)
{
	// Copy some or all of the attributes composing sourceStyle to targetStyle.
	// The mode parameter determines which attributes are to be copied and how.
	// If mode contains weDoToggleFace,  offStyles indicates which
	// Quickdraw styles are to be turned off.

	// COPY FONT
	if (mode & weDoFont)
	{ 
			targetStyle->tsFont = sourceStyle->tsFont;
#if WASTE_RESOLVE_FONT_DESIGNATORS
			if (targetStyle->tsFont == systemFont)
				targetStyle->tsFont = GetSysFont();
			if (targetStyle->tsFont == applFont)
				targetStyle->tsFont = GetAppFont();
#endif
	}
	
	// COPY SIZE
	if (mode & (weDoSize | weDoAddSize))
	{
		// copy size to a long variable to avoid integer overflows when doing additions
		long longSize = sourceStyle->tsSize;

		// zero really means 12
		if (longSize == 0)
			longSize = 12;

		// if kModeAddSize is set, the source size is added to the target size,
		// otherwise the source size replaces the target size outright
		if (mode & weDoAddSize) 
		{
			longSize += targetStyle->tsSize;
		}
		// range-check the resulting size
		longSize = _WEPinInRange(longSize, kMinFontSize, kMaxFontSize);
		targetStyle->tsSize = longSize;
	} // if alter size
	
	// COPY FACE
	if (mode & (weDoFace | weDoFaceMask))
	{
		Style sourceFace = sourceStyle->tsFace;
		Style targetFace = targetStyle->tsFace;
		
		if (mode & weDoFaceMask)
		{
			// USE MASK
			// if kModeFaceMask is set, copy the Quickdraw styles (tsFace field)
			// using the tsFlags field as a mask specifying which bits in the tsFace
			// field are to be copied.
			Style sourceMask = sourceStyle->tsFlags;
			targetFace = (sourceFace & sourceMask) | (targetFace & (~sourceMask));
		}
		else {
			// IGNORE MASK
			// sourceFace replaces targetFace outright if one or both of these conditions hold:
			// 1. sourceFace is zero (= empty set = plain text)
			// 2. the kModeReplaceFace bit is set
	
			if ((sourceFace == normal) || (mode & weDoReplaceFace))
			{
				targetFace = sourceFace;
			}
			else {
				// Otherwise sourceFace is interpreted as a bitmap indicating
				// which styles are to be altered -- all other styles are left intact.
				// What exactly happens to the styles indicated in sourceFace
				// depends on whether the kModeToggleFace bit is set or clear.
		
				// if kModeToggleFace is set, turn a style off if it's set in offStyles,
				// else turn it on
				if (mode & weDoToggleFace) 
				{
					targetFace = (sourceFace ^ offStyles) | (targetFace & (~sourceFace));
				}
				else
				{
					// if kModeToggleFace is clear, turn on the styles specified in sourceStyle
					targetFace |= sourceFace;
				}
			}
		}

		// the condense and extend attributes are mutually exclusive: if one is set
		// in sourceFace, remove it from targetFace
		if (sourceFace & condense)
		{
			targetFace &= (~extend);
		}
		if (sourceFace & extend)
		{
			targetFace &= (~condense);
		}
		
		targetStyle->tsFace = targetFace;
	}

	// COPY COLOR
	if (mode & weDoColor)
	{
		targetStyle->tsColor = sourceStyle->tsColor;
	}

#if WASTE_OBJECTS
	// if kModeObject is set, copy object descriptor
	if (mode & weDoObject)
	{
		targetStyle->tsObject = sourceStyle->tsObject;
	}
#endif

	// always clear targetStyle->tsFlags by default
	targetStyle->tsFlags = 0;

	// if kModeFlags is set, copy the tsFlags field
	if (mode & weDoFlags)
	{ 
		targetStyle->tsFlags = sourceStyle->tsFlags;
	}
}

pascal Boolean _WEOffsetInRange(long offset, char edge, long rangeStart, long rangeEnd)
{
	// return true if the position specified by the pair < offset, edge >
	// is within the specified range

	// if edge is kTrailingEdge, offset really refers to the preceding character
	if (edge == kTrailingEdge) 
	{
		offset--;
	}
	// return true iff offset is within the specified range
	return ((offset >= rangeStart) && (offset < rangeEnd));
}
