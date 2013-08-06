
/*
 *	WESelectors.c
 *
 *	WASTE PROJECT
 *  Routines for manipulating selector lookup tables
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */


#include "WASTEIntf.h"

// MPW can't handle FIELD_DESC stuff (duh!)

#ifdef applec

// this is **ugly** and will break if we change the declaration of WERec

static WELookupTable _weMainSelectorTable[] = {
{ weRefCon,     		0x0074, 0x0004 },
{ wePort,				0x0000, 0x0004 },
{ weText,				0x0004, 0x0004 },
{ weCharToPixelHook,	0x00B0, 0x0004 },
{ weCharByteHook,		0x00BC, 0x0004 },
{ weCharTypeHook,		0x00C0, 0x0004 },
{ weClickLoop,			0x0094, 0x0004 },
{ weCurrentDrag,		0x0084, 0x0004 },
{ weDrawTextHook,		0x00A8, 0x0004 },
{ weLineBreakHook,		0x00B4, 0x0004 },
{ weLineArray,			0x0008, 0x0004 },
{ wePixelToCharHook,	0x00AC, 0x0004 },
{ weTSMPostUpdate,		0x00A0, 0x0004 },
{ weTSMPreUpdate,		0x009C, 0x0004 },
{ weRunArray,			0x0010, 0x0004 },
{ weScrollProc,			0x0098, 0x0004 },
{ weStyleTable,			0x000C, 0x0004 },
{ weTSMDocumentID,		0x0078, 0x0004 },
{ weWordBreakHook,		0x00B8, 0x0004 },
{ weTranslateDragHook,	0x00A4, 0x0004 },
#if WASTE_IC_SUPPORT
{ weURLHint,			0x00D8, 0x0004 },
#endif
{ 0, 					0, 0 }};

#if WASTE_OBJECTS

static WELookupTable _weObjectHandlerSelectorTable[] = {
{ weClickHandler,		0x0010, 0x0004 },
{ weDrawHandler,		0x000C, 0x0004 },
{ weDisposeHandler,		0x0008, 0x0004 },
{ weNewHandler,			0x0004, 0x0004 },
{ weStreamHandler,      0x0014, 0x0004 },
{ 0, 					0, 0 }};

#endif  // WASTE_OBJECTS

#else

// proper way of defining selector tables

#define FIELD_OFFSET(FIELD, STRUCT)		(short) &((STRUCT *) 0L)->FIELD
#define FIELD_SIZE(FIELD, STRUCT)		(short) sizeof(((STRUCT *) 0L)->FIELD)
#define FIELD_DESC(FIELD, STRUCT)		{ FIELD_OFFSET(FIELD, STRUCT), FIELD_SIZE(FIELD, STRUCT) }

static WELookupTable _weMainSelectorTable[] = {
{ weRefCon,     		FIELD_DESC(refCon,            WERec) },
{ wePort,				FIELD_DESC(port,              WERec) },
{ weText,				FIELD_DESC(hText,             WERec) },
{ weCharToPixelHook,	FIELD_DESC(charToPixelHook,   WERec) },
{ weCharByteHook,		FIELD_DESC(charByteHook,      WERec) },
{ weCharTypeHook,		FIELD_DESC(charTypeHook,      WERec) },
{ weClickLoop,			FIELD_DESC(clickLoop,         WERec) },
{ weCurrentDrag,		FIELD_DESC(currentDrag,       WERec) },
{ weDrawTextHook,		FIELD_DESC(drawTextHook,      WERec) },
{ weLineBreakHook,		FIELD_DESC(lineBreakHook,     WERec) },
{ weLineArray,			FIELD_DESC(hLines,            WERec) },
{ wePixelToCharHook,	FIELD_DESC(pixelToCharHook,   WERec) },
{ weTSMPostUpdate,		FIELD_DESC(tsmPostUpdate,     WERec) },
{ weTSMPreUpdate,		FIELD_DESC(tsmPreUpdate,      WERec) },
{ weRunArray,			FIELD_DESC(hRuns,             WERec) },
{ weScrollProc,			FIELD_DESC(scrollProc,        WERec) },
{ weStyleTable,			FIELD_DESC(hStyles,           WERec) },
{ weTSMDocumentID,		FIELD_DESC(tsmReference,      WERec) },
{ weWordBreakHook,		FIELD_DESC(wordBreakHook,     WERec) },
{ weTranslateDragHook,	FIELD_DESC(translateDragHook, WERec) },
#if WASTE_IC_SUPPORT
{ weURLHint,			FIELD_DESC(hURLHint,          WERec) },
#endif
{ 0, 					0, 0 }};

#if WASTE_OBJECTS

static WELookupTable _weObjectHandlerSelectorTable[] = {
{ weClickHandler,		FIELD_DESC(clickHandler,      WEOHTableElement) },
{ weDrawHandler,		FIELD_DESC(drawHandler,       WEOHTableElement) },
{ weDisposeHandler,		FIELD_DESC(freeHandler,       WEOHTableElement) },
{ weNewHandler,			FIELD_DESC(newHandler,        WEOHTableElement) },
{ weStreamHandler,      FIELD_DESC(streamHandler,     WEOHTableElement) },
{ 0, 					0, 0 }};

#endif	// WASTE_OBJECTS

#endif	// !applec

pascal void _WELookupSelector(const WELookupTable *table, WESelector selector, WEFieldDescriptor *desc)
{
	for ( ; table->selector != selector ; table++ )
		if ( * (long *) &(table->desc) == 0L )
			break;
	
	*desc = table->desc;
}

pascal OSErr _WEGetField(const WELookupTable *table, WESelector selector, long *info, void *structure)
{
	WEFieldDescriptor desc;
	
	_WELookupSelector(table, selector, &desc);
	
	if (desc.fLength == 0)
		return weUndefinedSelectorErr;
	
	*info = * (long *) ((long) structure + desc.fOffset);
	return noErr;
}

pascal OSErr _WESetField(const WELookupTable *table, WESelector selector, long *info, void *structure)
{
	WEFieldDescriptor desc;
	
	_WELookupSelector(table, selector, &desc);
	
	if (desc.fLength == 0)
		return weUndefinedSelectorErr;
	
	* (long *) ((long) structure + desc.fOffset) = *info;
	return noErr;
}

pascal OSErr WEGetInfo(WESelector selector, void *info, WEHandle hWE)
{
	return _WEGetField(_weMainSelectorTable, selector, (long *)info, (void *)*hWE);
} // WEGetInfo

pascal OSErr WESetInfo(WESelector selector, const void *info, WEHandle hWE)
{
	OSErr err;
	
	err = _WESetField(_weMainSelectorTable, selector, (long *)info, (void *)*hWE);
	
	// the hook fields can never be NULL, so replace any NULL field with the default address
	_WESetStandardHooks(hWE);
	
	return err;
} // WESetInfo

#if WASTE_OBJECTS
// for some reason _WESetField doesn't work from other files (?)
pascal OSErr _WESetHandler(WESelector selector, long *info, void *structure)
{
	return _WESetField(_weObjectHandlerSelectorTable, selector, info, structure);
}
#endif