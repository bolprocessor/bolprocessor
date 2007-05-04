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
 *	WEBirthDeath.c
 *
 *	WASTE PROJECT
 *  Creation and Destruction, Standard Procs, etc.
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */


#include "WASTEIntf.h"

#if TARGET_RT_MAC_CFM
#ifndef __CODEFRAGMENTS__
#include <CodeFragments.h>
#endif
#endif

// static variables

static WEDrawTextUPP		_weStdDrawTextProc = NULL;
static WEPixelToCharUPP		_weStdPixelToCharProc = NULL;
static WECharToPixelUPP		_weStdCharToPixelProc = NULL;
static WELineBreakUPP		_weStdLineBreakProc = NULL;
static WEWordBreakUPP		_weStdWordBreakProc = NULL;
static WECharByteUPP		_weStdCharByteProc = NULL;
static WECharTypeUPP		_weStdCharTypeProc = NULL;
static WEClickLoopUPP		_weStdClickLoopProc = NULL;

pascal void _WEStdDrawText(Ptr pText, long textLength, Fixed slop, 
				JustStyleCode styleRunPosition, WEHandle hWE)
{
#pragma unused(hWE)
	DrawJustified(pText, textLength, slop, styleRunPosition,
		  kOneToOneScaling, kOneToOneScaling);
} // _WEStdDrawText

pascal long _WEStdPixelToChar(Ptr pText, long textLength, Fixed slop,
				Fixed *width, char *edge, JustStyleCode styleRunPosition,
				Fixed hPos, WEHandle hWE)
{
#pragma unused(hPos, hWE)
	Fixed lastWidth;
	long retVal;
	Boolean theEdge = *edge;

	lastWidth = *width;
	retVal = PixelToChar(pText, textLength, slop, lastWidth, &theEdge,
		width, styleRunPosition, kOneToOneScaling, kOneToOneScaling);
	*edge = theEdge;

	// round width to nearest integer value
	// (this is supposed to fix an incompatibility with the WorldScript Power Adapter)
	*width = (*width + 0x00008000) & 0xFFFF0000;

	return retVal;
} // _WEStdPixelToChar

pascal short _WEStdCharToPixel(Ptr pText, long textLength, Fixed slop,
				long offset, short direction, JustStyleCode styleRunPosition,
				long hPos, WEHandle hWE)
{
#pragma unused(hPos, hWE)
	return CharToPixel(pText, textLength, slop, offset, direction,
			styleRunPosition, kOneToOneScaling, kOneToOneScaling);
} // _WEStdCharToPixel

pascal StyledLineBreakCode _WEStdLineBreak(Ptr pText, long textLength,
				long textStart, long textEnd, Fixed *textWidth,
				long *textOffset, WEHandle hWE)
{
#pragma unused(hWE)
	return StyledLineBreak(pText, textLength, textStart, textEnd, 0, textWidth,
				textOffset);
} // _WEStdLineBreak

pascal void _WEStdWordBreak(Ptr pText, short textLength, short offset,
				char edge, OffsetTable breakOffsets, ScriptCode script,
				WEHandle hWE)
{
#pragma unused(hWE)
	FindWordBreaks(pText, textLength, offset, (Boolean)edge, NULL, breakOffsets,
		script);
} // _WEStdWordBreak

pascal short _WEStdCharByte(Ptr pText, short textOffset, ScriptCode script,
				WEHandle hWE)
{
#pragma unused(hWE)
	return CharacterByteType(pText, textOffset, script);
} // _WEStdCharByte

pascal short _WEStdCharType(Ptr pText, short textOffset, ScriptCode script,
				WEHandle hWE)
{
#pragma unused(hWE)
	return CharacterType(pText, textOffset, script);
} // _WEStdCharType

pascal short _WEScriptToFont(ScriptCode script)
{
	// given an explicit script code, return the first font ID in the corresponding range
	// for an explanation of the formula given below, see IM: Text, page B-8

	if (script == smRoman)
		return 2;
	else if ((script > smRoman) && (script <= smUninterp))
		return (0x3E00 + 0x200 * script);
	else
		return systemFont;	// unknown script code (?)
} // _WEScriptToFont

#if !SystemSevenFiveOrLater

pascal void _WEOldWordBreak(Ptr pText, short textLength, short offset,
				char edge, OffsetTable breakOffsets, ScriptCode script,
				WEHandle hWE)
{
	GrafPtr savePort, tempPort;
	short saveFont;

	// the old (now obsolete) FindWord routine gets an implicit script parameter through
	// the current graphics port txFont field, so first of all we must have a valid port
	GetPort(&savePort);
	tempPort = (*hWE)->port;
	SetPort(tempPort);

	// then set the txFont field to a font number in the specified script range
	saveFont = GetPortTextFont(tempPort);
	TextFont(_WEScriptToFont(script));

	// call _FindWord
	FindWord(pText, textLength, offset, (Boolean)edge, NULL, breakOffsets);

	// restore font and port
	TextFont(saveFont);
	SetPort(savePort);

} // _WEOldWordBreak

pascal short _WEOldCharByte(Ptr pText, short textOffset, ScriptCode script,
				WEHandle hWE)
{
	GrafPtr savePort, tempPort;
	short saveFont;
	short retVal;

	// the old (now obsolete) CharByte routine gets an implicit script parameter through
	// the current graphics port txFont field, so first of all we must have a valid port
	GetPort(&savePort);
	tempPort = (*hWE)->port;
	SetPort(tempPort);

	// then set the txFont field to a font number in the specified script range
	saveFont = GetPortTextFont(tempPort);
	TextFont(_WEScriptToFont(script));

	// call _CharByte
	retVal = CharByte(pText, textOffset);

	// restore font and port
	TextFont(saveFont);
	SetPort(savePort);

	return retVal;
} // _WEOldCharByte

pascal short _WEOldCharType(Ptr pText, short textOffset, ScriptCode script,
				WEHandle hWE)
{
	GrafPtr savePort, tempPort;
	short saveFont;
	short retVal;

	// the old (now obsolete) CharType routine gets an implicit script parameter through
	// the current graphics port txFont field, so first of all we must have a valid port
	GetPort(&savePort);
	tempPort = (*hWE)->port;
	SetPort(tempPort);

	// then set the txFont field to a font number in the specified script range
	saveFont = GetPortTextFont(tempPort);
	TextFont(_WEScriptToFont(script));

	// call _CharType
	retVal = CharType(pText, textOffset);

	// restore font and port
	TextFont(saveFont);
	SetPort(savePort);

	return retVal;

} // _WEOldCharType

#endif

pascal Boolean _WEStdClickLoop(WEHandle hWE)
{
	WEPtr pWE = *hWE;		// assume WE record is already locked
	Point mouseLoc;
	long currentOffset;
	long maxOffset;
	long vDelta = 0;
	long hDelta = 0;
	
	// do nothing if auto-scroll is disabled or if we're inactive
	if (!BTST(pWE->features, weFAutoScroll) || !BTST(pWE->flags, weFActive))
		return true;
	
	// get current mouse location, in local coords
	// we can safely assume the graphics port is set up correctly
	GetMouse(&mouseLoc);
	
	// HANDLE VERTICAL AUTOSCROLL
	currentOffset = pWE->viewRect.top - pWE->destRect.top;
	maxOffset = (pWE->destRect.bottom - pWE->destRect.top) - (pWE->viewRect.bottom - pWE->viewRect.top);
	
	// is the mouse below the view rect?
	if (mouseLoc.v > pWE->viewRect.bottom)
	{
		// is there anything hidden below the view rect?
		if (currentOffset < maxOffset)
		{
			// then scroll down: calculate the scroll delta
			vDelta = pWE->viewRect.bottom - mouseLoc.v;

			// pin the new vertical offset to the bottom of the dest rectangle
			if (vDelta < (currentOffset - maxOffset))
				vDelta = currentOffset - maxOffset;
			
			// never scroll by more than kMaxScrollDelta pixels
			if (vDelta < -kMaxScrollDelta)
				vDelta = -kMaxScrollDelta;
		}
	}

	// is the mouse above the view rect?
	else if (mouseLoc.v < pWE->viewRect.top)
	{
		// is there anything hidden above the view rect?
		if (currentOffset > 0)
		{
			// then scroll up: calculate the scroll delta
			vDelta = pWE->viewRect.top - mouseLoc.v;

			// pin the new vertical offset to the top of the dest rectangle
			if (vDelta > currentOffset)
				vDelta = currentOffset;

			// never scroll by more than kMaxScrollDelta pixels
			if (vDelta > kMaxScrollDelta)
				vDelta = kMaxScrollDelta;
		}
	}
	
	// HANDLE HORIZONTAL AUTOSCROLL
	currentOffset = pWE->viewRect.left - pWE->destRect.left;
	maxOffset = (pWE->destRect.right - pWE->destRect.left) - (pWE->viewRect.right - pWE->viewRect.left);
	
	// is the mouse to the right of the view rect?
	if (mouseLoc.h > pWE->viewRect.right)
	{
		// is there anything hidden to the right of the view rect?
		if (currentOffset < maxOffset)
		{
			// then scroll right: calculate the scroll delta
			hDelta = pWE->viewRect.right - mouseLoc.h;

			// pin the new vertical offset to the rightmost edge
			// of the dest rectangle
			if (hDelta < (currentOffset - maxOffset))
				hDelta = currentOffset - maxOffset;
			
			// never scroll by more than kMaxScrollDelta pixels
			if (hDelta < -kMaxScrollDelta)
				hDelta = -kMaxScrollDelta;
		}
	}

	// is the mouse to the left of the view rect?
	else if (mouseLoc.h < pWE->viewRect.left)
	{
		// is there anything hidden to the left of the view rect?
		if (currentOffset > 0)
		{
			// then scroll up: calculate the scroll delta
			hDelta = pWE->viewRect.left - mouseLoc.h;

			// pin the new horizontal offset to the leftmost edge
			// of the dest rectangle
			if (hDelta > currentOffset)
				hDelta = currentOffset;

			// never scroll by more than kMaxScrollDelta pixels
			if (hDelta > kMaxScrollDelta)
				hDelta = kMaxScrollDelta;
		}
	}
	
	if ((vDelta != 0) || (hDelta != 0))
	{
		// do the scroll
		WEScroll(hDelta, vDelta, hWE);
		
		// notify our client we have scrolled the text
		if (pWE->scrollProc != NULL) 
		{
			CallWEScrollProc(hWE, pWE->scrollProc);
		}
	}

	return true;
}

pascal OSErr _WERegisterWithTSM(WEHandle hWE)
{
	WEPtr pWE = *hWE;	// assume WE record is already locked
	InterfaceTypeList typeList;
	OSErr err;

	// do nothing if the Text Services Manager isn't available
	if (BTST(pWE->flags, weFHasTextServices))
	{
		typeList[0] = kTextService;
		if ((err = NewTSMDocument(1, typeList, &pWE->tsmReference, (long)hWE)) != noErr)
		{
			// we don't consider it an error if our client application isn't TSM-aware
			if (err != tsmNeverRegisteredErr)
			{
				goto cleanup;
			}
		}
	}

	// clear result code
	err = noErr;

cleanup:
	// return result code
	return err;
}

pascal void _WESetStandardHooks(WEHandle hWE)
{
	WEPtr pWE;

	// the first time we're called, create routine descriptors
	if (_weStdDrawTextProc == NULL)
	{
		_weStdDrawTextProc = NewWEDrawTextProc(_WEStdDrawText);
		_weStdPixelToCharProc = NewWEPixelToCharProc(_WEStdPixelToChar);
		_weStdCharToPixelProc = NewWECharToPixelProc(_WEStdCharToPixel);
		_weStdLineBreakProc = NewWELineBreakProc(_WEStdLineBreak);

#if !SystemSevenFiveOrLater

		if (GetScriptManagerVariable(smVersion) < 0x0710)
		{
			// pre-7.1 version of the Script Manager: must use old hooks
			_weStdWordBreakProc = NewWEWordBreakProc(_WEOldWordBreak);
			_weStdCharByteProc = NewWECharByteProc(_WEOldCharByte);
			_weStdCharTypeProc = NewWECharTypeProc(_WEOldCharType);
		}
		else
#endif
		{
			// Script Manager version 7.1 or newer
			_weStdWordBreakProc = NewWEWordBreakProc(_WEStdWordBreak);
			_weStdCharByteProc = NewWECharByteProc(_WEStdCharByte);
			_weStdCharTypeProc = NewWECharTypeProc(_WEStdCharType);
		}

		_weStdClickLoopProc = NewWEClickLoopProc(_WEStdClickLoop);

	} // if called for the first time

	// replace null hook fields with the addresses of the standard hooks

	pWE = *hWE;

	// replace null hook fields with the addresses of the standard hooks

	if (pWE->drawTextHook == NULL)
		pWE->drawTextHook = _weStdDrawTextProc;

	if (pWE->pixelToCharHook == NULL)
		pWE->pixelToCharHook = _weStdPixelToCharProc;

	if (pWE->charToPixelHook == NULL)
		pWE->charToPixelHook = _weStdCharToPixelProc;

	if (pWE->lineBreakHook == NULL)
		pWE->lineBreakHook = _weStdLineBreakProc;

	if (pWE->wordBreakHook == NULL)
		pWE->wordBreakHook = _weStdWordBreakProc;
	
	if (pWE->charByteHook == NULL)
		pWE->charByteHook = _weStdCharByteProc;

	if (pWE->charTypeHook == NULL)
		pWE->charTypeHook = _weStdCharTypeProc;
	
	if (pWE->clickLoop == NULL)
		pWE->clickLoop = _weStdClickLoopProc;

} // _WESetStandardHooks

pascal OSErr WENew(const LongRect *destRect, const LongRect *viewRect, unsigned long features, WEHandle *hWE)
{
	WEPtr pWE = NULL;
	short allocFlags = kAllocClear;
	long response;
	Rect r;
	OSErr err;

	// allocate the WE record
	if ((err = _WEAllocate(sizeof(WERec), allocFlags, (Handle *)hWE)) != noErr) 
	{
		goto cleanup;
	}
	
	// lock it down
	HLock((Handle)*hWE);
	pWE = **hWE;

	// get active port
	GetPort(&pWE->port);

	// determine whether temporary memory should be used for data structures
	if (BTST(features, weFUseTempMem))
	{ 
		allocFlags += kAllocTemp;
	}
	
	// allocate the text handle (initially empty)
	if ((err = _WEAllocate(0, allocFlags, (Handle *)&pWE->hText)) != noErr) 
	{
		goto cleanup;
	}

	// allocate the line array
	if ((err = _WEAllocate(2 * sizeof(LineRec), allocFlags, (Handle *)&pWE->hLines)) != noErr) 
	{
		goto cleanup;
	}

	// allocate the style table
	if ((err = _WEAllocate(sizeof(StyleTableElement), allocFlags, (Handle *)&pWE->hStyles)) != noErr) 
	{
		goto cleanup;
	}
	
	// allocate the run array
	if ((err = _WEAllocate(2 * sizeof(RunArrayElement), allocFlags, (Handle *)&pWE->hRuns)) != noErr) 
	{
		goto cleanup;
	}
	
	// check for the presence of various system software features
	// determine whether Color Quickdraw is available
	if ((Gestalt(gestaltQuickdrawVersion, &response) == noErr) && (response >= gestalt8BitQD))
	{
		BSET(pWE->flags, weFHasColorQD);
	}
	
	// determine whether the Drag Manager is available
	if ((Gestalt(gestaltDragMgrAttr, &response) == noErr) && BTST(response, gestaltDragMgrPresent))
	{
#if TARGET_RT_MAC_CFM
		if ((unsigned long) NewDrag != kUnresolvedCFragSymbolAddress)
#endif
			BSET(pWE->flags, weFHasDragManager);
	}

	// determine whether the Text Services manager is available
	if (Gestalt(gestaltTSMgrVersion, &response) == noErr)
	{ 
		BSET(pWE->flags, weFHasTextServices);
	}
	
	// determine if there are any non-Roman scripts enabled
	if (GetScriptManagerVariable(smEnabled) > 1) 
	{
		BSET(pWE->flags, weFNonRoman);
		
		// determine whether a double-byte script is installed
		if (GetScriptManagerVariable(smDoubleByte) != 0)
		{
#if TARGET_CPU_68K
			BSET(pWE->flags, weFDoubleByte);	// the WorldScript Power Adapter breaks this :-(
#else
			ScriptCode script;
			for ( script = smRoman; script <= smKlingon; script++ )
			{
				if (GetScriptVariable(script, smEnabled) &&
					((GetScriptVariable(script, smScriptFlags) & smsfSingByte) == 0))
				{
					BSET(pWE->flags, weFDoubleByte);
					break;
				}
			}
#endif
		}
		
		// determine whether a bidirectional script is installed
		if (GetScriptManagerVariable(smBidirect) != 0)
		{
			BSET(pWE->flags, weFBidirectional);
			
			// should we use a dual caret?
			if ((GetScriptManagerVariable(smGenFlags) & smfDualCaret) != 0)
			{
				BSET(pWE->flags, weFUseDualCaret);
			}
		}
	}
	
	// initialize miscellaneous fields of the WE record
	pWE->nLines = 1;
	pWE->nStyles = 1;
	pWE->nRuns = 1;
	pWE->viewRect = *viewRect;
	pWE->destRect = *destRect;
	pWE->features = features;
	pWE->tsmAreaStart = kInvalidOffset;
	pWE->tsmAreaEnd = kInvalidOffset;
	pWE->dragCaretOffset = kInvalidOffset;

	// initialize hook fields with the addresses of the standard hooks
	_WESetStandardHooks(*hWE);

	// create a region to hold the view rectangle
	pWE->viewRgn = NewRgn();
	WELongRectToRect(viewRect, &r);
	RectRgn(pWE->viewRgn, &r);

	// initialize the style run array
	(*pWE->hRuns)[1].runStart = 1;
	(*pWE->hRuns)[1].styleIndex = -1;

	// initialize the style table
	(*pWE->hStyles)[0].refCount = 1;

	// copy text attributes from the active graphics port
	(*pWE->hStyles)[0].info.runStyle.tsFont = GetPortTextFont(pWE->port);
	(*pWE->hStyles)[0].info.runStyle.tsSize = GetPortTextSize(pWE->port);
	(*pWE->hStyles)[0].info.runStyle.tsFace = GetPortTextFace(pWE->port);
	if (BTST(pWE->flags, weFHasColorQD))
	{ 
		GetForeColor(&(*pWE->hStyles)[0].info.runStyle.tsColor);
	}
	_WEFillFontInfo(pWE->port, &(*pWE->hStyles)[0].info);

	// initialize the line array
	if ((err = WECalText(*hWE)) != noErr) 
	{
		goto cleanup;
	}
	
	// register with the Text Services Manager
	if ((err = _WERegisterWithTSM(*hWE)) != noErr) 
	{
		goto cleanup;
	}

	// unlock the WE record
	HUnlock((Handle)*hWE);

	// skip clean-up section
	return noErr;

cleanup:
	// clean up
	if (pWE != NULL) 
	{
		_WEForgetHandle((Handle *) &pWE->hText);
		_WEForgetHandle((Handle *) &pWE->hLines);
		_WEForgetHandle((Handle *) &pWE->hStyles);
		_WEForgetHandle((Handle *) &pWE->hRuns);
		if (pWE->viewRgn != NULL) 
		{
			DisposeRgn(pWE->viewRgn);
		}
	}
	_WEForgetHandle((Handle *)hWE);

	return err;
}

pascal void _WEResetStyleTable(WEHandle hWE)
{
	WEPtr pWE = *hWE;	// assume WE record is already locked
	long index;
	StyleTableElement *pTable;
	Boolean saveTableLock;
	
	// sanity check
	if (pWE->hStyles == NULL)
		return;

	// lock the style table
	saveTableLock = _WESetHandleLock((Handle) pWE->hStyles, true);
	pTable = *pWE->hStyles;

	// walk the style table, disposing of all embedded objects referenced there
	index = pWE->nStyles;
	while ( --index >= 0 )
	{

#if WASTE_OBJECTS
		if ((pTable->refCount > 0) && (pTable->info.runStyle.tsObject != NULL))
		{
			_WEFreeObject(pTable->info.runStyle.tsObject);
		}
#endif

		pTable->refCount = 0;
		pTable++;
	};
	
	// unlock the style table
	_WESetHandleLock((Handle) pWE->hStyles, saveTableLock);
}

pascal void WEDispose(WEHandle hWE)
{
	WEPtr pWE;

	// sanity check: make sure WE isn't NULL
	if (hWE == NULL)
		return;

	// lock the WE record
	HLock((Handle) hWE);
	pWE = *hWE;

	// clear the Undo buffer
	WEClearUndo(hWE);

	// unregister with the Text Services Manager
	if (pWE->tsmReference != NULL)
	{
		DeleteTSMDocument(pWE->tsmReference);			
		pWE->tsmReference = NULL;
	}

	// dispose of the offscreen graphics world
	if (pWE->offscreenPort != NULL) 
	{
		DisposeGWorld((GWorldPtr)pWE->offscreenPort);
		pWE->offscreenPort = NULL;
	}

#if WASTE_OBJECTS
	// release all embedded objects we know about
	_WEResetStyleTable(hWE);

	// dispose instance-specific object handler table
	_WEForgetHandle((Handle *) &pWE->hObjectHandlerTable);
#endif
	
	// dispose of auxiliary data structures
	_WEForgetHandle((Handle *) &pWE->hText);
	_WEForgetHandle((Handle *) &pWE->hLines);
	_WEForgetHandle((Handle *) &pWE->hStyles);
	_WEForgetHandle((Handle *) &pWE->hRuns);
	DisposeRgn(pWE->viewRgn);

	// dispose of the WE record
	DisposeHandle((Handle) hWE);
}

pascal OSErr WEUseText(Handle text, WEHandle hWE)
{
	WEPtr pWE;
	RunArrayPtr pRuns;
	LinePtr pLines;
	long textLength;
	Boolean saveWELock;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;
	
	// stop any ongoing inline session
	WEStopInlineSession(hWE);
	
	// reset modification count and clear undo buffer, if any
	WEResetModCount(hWE);

	// reset the style table, disposing of all embedded objects it references
	_WEResetStyleTable(hWE);
	
	// install the text
	_WEForgetHandle(&pWE->hText);
	pWE->hText = text;
	textLength = GetHandleSize(text);
	pWE->textLength = textLength;
	
	// reset the run array
	SetHandleSize((Handle) pWE->hRuns, 2 * sizeof(RunArrayElement));
	pWE->nRuns = 1;
	pRuns = *pWE->hRuns;
	pRuns[1].runStart = textLength + 1;
	pRuns[1].styleIndex = -1;
	
	// fix the refCount of the style table entry referenced by the only run
	(*pWE->hStyles)[pRuns[0].styleIndex].refCount = 1;

	// reset the line array
	SetHandleSize((Handle) pWE->hLines, 2 * sizeof(LineRec));
	pWE->nLines = 1;
	pLines = *pWE->hLines;
	_WEBlockClr(pLines, 2 * sizeof(LineRec));
	pLines[1].lineStart = textLength;

	// reset several fields of the WE record
	pWE->selStart = 0;
	pWE->selEnd = 0;
	pWE->firstByte = 0;
	pWE->clickCount = 0;
	pWE->tsmAreaStart = kInvalidOffset;
	pWE->tsmAreaEnd = kInvalidOffset;
	pWE->dragCaretOffset = kInvalidOffset;
	
	// recalculate and redraw everything
	// err = _WERedraw(0, LONG_MAX, hWE);

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);

	return noErr;
}

pascal short WEFeatureFlag(short feature, short action, WEHandle hWE)
{
	WEPtr pWE = *hWE;
	short status;

	// get current status of the specified feature
	status = BTST(pWE->features, feature) ? weBitSet : weBitClear;

	// if action is weBitToggle, invert flag
	if (action == weBitToggle)
		action = 1 - status;

	// reset flag according to action
	if (action == weBitClear)
	{ 
		BCLR(pWE->features, feature);
	}
	else if (action == weBitSet)
	{ 
		BSET(pWE->features, feature);
	}
	
	// return old status
	return status;
}
