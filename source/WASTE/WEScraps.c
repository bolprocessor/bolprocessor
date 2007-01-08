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
 *	WEScraps.c
 *
 *	WASTE PROJECT
 *  Routines for Manipulating Style Scraps and Object Soups
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */


#include "WASTEIntf.h"

pascal OSErr _WEPrependStyle(Handle hStyleScrap, const WERunInfo *info, long offsetDelta)
{
	// compare the stylistic attributes in info with the first element of the specified
	// style scrap: if they differ, prepend a new element to the style scrap.
	// in any case, advance all character offsets in the style scrap by offsetDelta

	TEStyleScrapPtr pScrap;
	TEStyleScrapElement element;
	short i;
	OSErr err;

	pScrap = * (TEStyleScrapHandle) hStyleScrap;

	// compare this style info with that stored in the first element of our private style scrap
	if (!_WEBlockCmp(&pScrap->scrpStyleTab[0].scrpTEAttrs, &info->runAttrs, sizeof(TERunAttributes)))
	{
		// insert a new style scrap element at the beginning of the style scrap
		element.scrpStartChar = 0;
		element.scrpTEAttrs = * (TERunAttributes *) &info->runAttrs;
		
		if ((err = _WEInsertBlock(hStyleScrap, &element, sizeof(element), sizeof(short))) != noErr)
			return err;

		pScrap = * (TEStyleScrapHandle) hStyleScrap;

		// increment element count
		pScrap->scrpNStyles++;

	} // if not _WEBlockCmp

	// update char offsets within the style scrap
	for ( i = pScrap->scrpNStyles - 1; i > 0; i-- )
		pScrap->scrpStyleTab[i].scrpStartChar += offsetDelta;

	return noErr;

} // _WEPrependStyle

pascal OSErr _WEAppendStyle(Handle hStyleScrap, const WERunInfo *info, long offset)
{

	// compare the stylistic attributes in info with the last element of the specified
	// style scrap: if they differ, append a new element to the style scrap.

	TEStyleScrapPtr pScrap;
	TEStyleScrapElement element;
	OSErr err;

	pScrap = * (TEStyleScrapHandle) hStyleScrap;
	// compare this style info with that stored in the first element of our private style scrap
	if (!_WEBlockCmp(&pScrap->scrpStyleTab[pScrap->scrpNStyles - 1].scrpTEAttrs,
		&info->runAttrs, sizeof(TERunAttributes)))
	{

		// create a new style scrap element
		element.scrpStartChar = offset;
		element.scrpTEAttrs = * (TERunAttributes *) &info->runAttrs;

		// append it at the end of the style scrap
		err = PtrAndHand(&element, hStyleScrap, sizeof(element));
		if ((err = MemError()) != noErr)
			return err;

		// increment scrap counter
		pScrap = * (TEStyleScrapHandle) hStyleScrap;
		pScrap->scrpNStyles++;

	} // if not _WEBlockCmp

	// return result code
	return noErr;
} // _WEAppendStyle

#if WASTE_OBJECTS

pascal OSErr _WEPrependObject(Handle hSoup, const WERunInfo *info, long offsetDelta)
{
	// if info describes an embedded object, prepend a new object descriptor,
	// complete with the associated object data, to the specified soup.
	// in any case, advance all character offsets in the soup by offsetDelta

	WEObjectDescHandle hObjectDesc = info->runAttrs.runStyle.tsObject;
	Handle hObjectData = NULL;
	FlavorType objectType;
	Boolean disposeData;
	unsigned long pSoup;
	WESoup soup;
	Size soupSize, objectDataSize, extraSize;
	OSErr err;

	// get size of existing soup
	soupSize = GetHandleSize(hSoup);

	if (hObjectDesc != NULL) 
	{
		// create a temporary handle for the streaming callback
		if ((err = _WEAllocate(0, kAllocTemp, &hObjectData)) != noErr)
			goto cleanup;
		
		// get the object type/data
		if ((err = _WEStreamObject(weToSoup, &objectType, &hObjectData, &disposeData, hObjectDesc)) != noErr)
			goto cleanup;

		// get size of object data
		objectDataSize = GetHandleSize(hObjectData);
		
		// extraSize is size of descriptor + size of object data
		extraSize = sizeof(soup) + objectDataSize;

		// fill in a soup item
		BLOCK_CLR(soup);
		soup.soupType = objectType;
		soup.soupSize = (*hObjectDesc)->objectSize;
		soup.soupDataSize = objectDataSize;

		// resize the soup
		SetHandleSize(hSoup, extraSize + soupSize);
		if ((err = MemError()) != noErr)
			goto cleanup;

		// move old contents forward
		pSoup = (unsigned long) *hSoup;
		BlockMoveData((const void *) pSoup, (void *) (pSoup + extraSize), soupSize);

		// insert the new object descriptor at the beginning
		BlockMoveData(&soup, (void *) pSoup, sizeof(soup));

		// copy the object data
		BlockMoveData(*hObjectData, (void *) (pSoup + sizeof(soup)), objectDataSize);
	}
	else
	{
		pSoup = (unsigned long) *hSoup;
		extraSize = 0;
	}

	// update char offsets within the soup
	while (soupSize > 0)
	{
		pSoup += extraSize;
		
		//	we have to use BlockMoveData to access descriptors within
		//	the soup, as they might be odd-aligned (duh!) and that would
		//	cause fatal address errors on 68000 CPUs
		
		BlockMoveData((const void *) pSoup, &soup, sizeof(soup));
		soup.soupOffset += offsetDelta;
		BlockMoveData(&soup, (void *) pSoup, sizeof(soup));
		extraSize = sizeof(soup) + soup.soupDataSize;
		soupSize -= extraSize;
	}

	err = noErr;

cleanup:
	if (disposeData)
		_WEForgetHandle(&hObjectData);

	return err;

} // _WEPrependObject

pascal OSErr _WEAppendObject(Handle hSoup, const WERunInfo *info, long offset)
{

	// if info describes an embedded object, append a new object descriptor,
	// complete with the associated object data, to the specified soup.

	WEObjectDescHandle hObjectDesc = info->runAttrs.runStyle.tsObject;
	Handle hObjectData = NULL;
	FlavorType objectType;
	WESoup soup;
	Boolean saveDataLock;
	Boolean disposeData;
	OSErr err;

	if (hObjectDesc != NULL)
	{
		// create a temporary handle for the streaming callback
		if ((err = _WEAllocate(0, kAllocTemp, &hObjectData)) != noErr)
			goto cleanup;
		
		// get the object type/data
		if ((err = _WEStreamObject(weToSoup, &objectType, &hObjectData, &disposeData, hObjectDesc)) != noErr)
			goto cleanup;

		// fill in a soup item
		BLOCK_CLR(soup);
		soup.soupOffset = offset;
		soup.soupType = objectType;
		soup.soupSize = (*hObjectDesc)->objectSize;
		soup.soupDataSize = GetHandleSize(hObjectData);

		// append it to the soup handle
		PtrAndHand(&soup, hSoup, sizeof(soup));
		if ((err = MemError()) != noErr)
			goto cleanup;

		// append the actual object data to the soup handle
		saveDataLock = _WESetHandleLock(hObjectData, true);
		PtrAndHand(*hObjectData, hSoup, soup.soupDataSize);
		err = MemError();
		_WESetHandleLock(hObjectData, saveDataLock);
		if (err != noErr)
			goto cleanup;
	} // if object reference is not NULL

	err = noErr;

cleanup:
	if (disposeData)
		_WEForgetHandle(&hObjectData);

	return err;

} // _WEAppendObject

#endif

pascal OSErr WECopyRange(long rangeStart, long rangeEnd, Handle hText,
					Handle hStyles, Handle hSoup, WEHandle hWE)
{

	// Make a copy of the specified range of text: store the characters in hText
	// and the associated style scrap in hStyles.  The handles are resized as necessary.
	// Specify NULL in hText or hStyles if you don't want the corresponding info returned.

	WEPtr pWE;
	TEStyleScrapElementPtr pElement;
	long rangeLength;
	long firstRun, nRuns, i;
	long startChar;
	WERunInfo info;
	Boolean saveWELock;
	OSErr err;

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// range-check parameters and reorder them if necessary
	rangeStart = _WEPinInRange(rangeStart, 0, pWE->textLength);
	rangeEnd = _WEPinInRange(rangeEnd, 0, pWE->textLength);
	_WEReorder(&rangeStart, &rangeEnd);
	rangeLength = rangeEnd - rangeStart;

	if (hText != NULL) 
	{
		// resize the given handle
		SetHandleSize(hText, rangeLength);
		if ((err = MemError()) != noErr) 
			goto cleanup;

		// copy the text range
		BlockMoveData((Ptr) *pWE->hText + rangeStart, (Ptr) *hText, rangeLength);
	}

	// make the soup handle zero-length
	if (hSoup != NULL)
	{
		SetHandleSize(hSoup, 0);
		if ((err = MemError()) != noErr)
			goto cleanup;
	}

	if ((hStyles != NULL) || (hSoup != NULL))
	{
		// count how many style runs there are in the selection range
		firstRun = _WEOffsetToRun(rangeStart, hWE);
		nRuns = _WEOffsetToRun(rangeEnd - 1, hWE) - firstRun + 1;

		if (hStyles != NULL)
		{
			// resize the given style scrap handle and lock it in high heap
			SetHandleSize(hStyles, (nRuns * sizeof(ScrpSTElement)) + sizeof(short));
			if ((err = MemError()) != noErr)
				goto cleanup;
			HLockHi(hStyles);
			
			// fill in the style count in the style scrap
			// *** POTENTIAL PROBLEM: if nRuns > 32767, scrpNStyles will be invalid ***
			WEASSERT(nRuns <= SHRT_MAX, "\pToo many styles");
			(* (TEStyleScrapHandle) hStyles)->scrpNStyles = nRuns;
		}
		
		pElement = & ((* (TEStyleScrapHandle) hStyles)->scrpStyleTab[0]);
		// loop through every style run in the selection range
		for ( i = 0; i < nRuns; i++ )
		{
			_WEGetIndStyle(firstRun + i, &info, hWE);
			
			// calculate the start character for this style run, relative to the beginning of the range
			startChar = info.runStart - rangeStart;
			if (startChar < 0)
			{
				startChar = 0;
#if WASTE_OBJECTS
				info.runAttrs.runStyle.tsObject = NULL;
#endif
			}
			if (hStyles != NULL)
			{
				info.runAttrs.runStyle.tsFlags = 0; // don't export internal flags
				pElement->scrpStartChar = startChar;
				pElement->scrpTEAttrs = * (TERunAttributes *) &info.runAttrs;
				pElement++;
			}

#if WASTE_OBJECTS
			if (hSoup != NULL)
			{
				// if this style run references an embedded object, append it to the "soup"
				if (info.runAttrs.runStyle.tsObject != NULL)
				{
					if ((err = _WEAppendObject(hSoup, &info, startChar)) != noErr)
						goto cleanup;
				}
			}
#endif

		}
	}
	// clear result code
	err = noErr;

cleanup:
	
	// unlock the style scrap handle
	if (hStyles != NULL)
		HUnlock(hStyles);

	// unlock the WE record
	_WESetHandleLock((Handle) hWE, saveWELock);
	// return result code
	return err;
}

pascal OSErr WECopy(WEHandle hWE)
{
	// Copy the selection range to the desk scrap

	WEPtr pWE;
	AEDesc d[3] = { { kTypeText, NULL }, { kTypeStyles, NULL }, { kTypeSoup, NULL } };
	Handle hItem;
	Size itemSize;
	short i, numTypes;
	Boolean saveWELock;
	Boolean saveDataLock;
	Boolean disposeData = true;
	OSErr err;
#if WASTE_OBJECTS
	WEObjectDescHandle hObjectDesc = NULL;
#endif

	// lock the WE record
	saveWELock = _WESetHandleLock((Handle) hWE, true);
	pWE = *hWE;

	// return weEmptySelectionErr if the selection range is empty
	if (pWE->selStart == pWE->selEnd)
	{
		err = weEmptySelectionErr;
		goto cleanup;
	}
	
	// clear the desk scrap
	if ((err = ZeroScrap()) != noErr)
		goto cleanup;

#if WASTE_OBJECTS
	numTypes = (BTST(pWE->features, weFMonoStyled) || (WEGetSelectedObject(&hObjectDesc, hWE) == noErr)) ? 1 : 3;
#else
	numTypes = BTST(pWE->features, weFMonoStyled) ? 1 : 2;
#endif

	// allocate some temporary handles
	for ( i = 0; i < numTypes; i++ )
	{
		if ((err = _WEAllocate(0, kAllocTemp, &d[i].dataHandle)) != noErr)
			goto cleanup;
	}

#if WASTE_OBJECTS
	if (hObjectDesc != NULL)
	{
		if ((err = _WEStreamObject(weToScrap, &d[0].descriptorType, &d[0].dataHandle,
					&disposeData, hObjectDesc)) != noErr)
			goto cleanup;
	}
	else
#endif
	{
		// make a copy of the selection text and styles and create an object "soup"
		if ((err = WECopyRange(pWE->selStart, pWE->selEnd,
			d[0].dataHandle, d[1].dataHandle, d[2].dataHandle, hWE)) != noErr)
			goto cleanup;
	}
	
	// copy the items to the desk scrap
	for ( i = 0; i < numTypes; i++ )
	{
		hItem = d[i].dataHandle;
		itemSize = GetHandleSize(hItem);
		if (itemSize > 0)
		{
			saveDataLock = _WESetHandleLock(hItem, true);
			err = PutScrap(itemSize, d[i].descriptorType, *hItem);
			_WESetHandleLock(hItem, saveDataLock);
			if (err != noErr)
				goto cleanup;
		}
	}

	// clear result code
	err = noErr;

cleanup:
	// clean up
	if (disposeData)
	{
		_WEForgetHandle(&d[0].dataHandle);
		_WEForgetHandle(&d[1].dataHandle);
		_WEForgetHandle(&d[2].dataHandle);
	}
	_WESetHandleLock((Handle) hWE, saveWELock);
	return err;
}
