/*
 *	WASTE.h
 *
 *	C/C++ interface to the WASTE text engine
 *
 *	version 1.2a5 (January 1996)
 *
 *	Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 * 
 */

#ifndef _WASTE_
#define _WASTE_

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif

#ifndef __TYPES__
#include <Types.h>
#endif

#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif

#ifndef __QUICKDRAWTEXT__
#include <QuickdrawText.h>
#endif

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif

#ifndef __SCRIPT__
#include <Script.h>
#endif

#ifndef __TEXTUTILS__
#include <TextUtils.h>
#endif

#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif

#ifndef __DRAG__
#include "Drag.h"
#endif

#ifndef __LONGCOORDINATES__
#ifndef _LongCoords_
#include "LongCoords.h"
#endif
#endif

// if we're using a pre-2.1 version of the Universal Headers, define EventModifiers
#ifndef UNIVERSAL_INTERFACES_VERSION
typedef unsigned short EventModifiers;
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif

//	The macro WASTE_VERSION expands to the current version of WASTE,
//	expressed in standard NumVersion format (see Types.h)
//	The macro WASTE11 is obsolete but still supported for backward
//	compatibility

#define WASTE_VERSION	0x01204005
#define WASTE11

/*	result codes */

enum {
	weCantUndoErr			=	-10015,	/* undo buffer is clear (= errAECantUndo) */
	weEmptySelectionErr		=	-10013,	/* selection range is empty (= errAENoUserSelection) */
	weNotHandledErr			=	-1708,	/* please use default behavior (= errAEEventNotHandled) */
	weUnknownObjectTypeErr	=	-9478,	/* specified object type is not registered */
	weObjectNotFoundErr		=	-9477,	/* no object found at specified offset */
	weReadOnlyErr			=	-9476,	/* instance is read-only */
	weUndefinedSelectorErr	=	-50		/* unknown selector (= paramErr) */
};

/*	alignment styles */

enum {
	weFlushLeft 		=	-2,		/* flush left */
	weFlushRight		=	-1,		/* flush right */
	weFlushDefault		=	 0,		/* flush according to system direction */
	weCenter			=	 1,		/* centered */
	weJustify			=	 2		/* fully justified */
};

/*	values for the mode parameter in WESetStyle and WEContinuousStyle */

enum {
	weDoFont				=	0x0001,
	weDoFace				=	0x0002,
	weDoSize				=	0x0004,
	weDoColor				=	0x0008,
	weDoAll					=	weDoFont + weDoFace + weDoSize + weDoColor,
	weDoAddSize				=	0x0010,
	weDoToggleFace			=	0x0020,
	weDoReplaceFace			=	0x0040,
	weDoPreserveScript		=	0x0080,
	weDoExtractSubscript	=	0x0100,
	weDoFaceMask			=	0x0200
};

/*	values for the edge parameter in WEGetOffset etc. */

enum {
	kLeadingEdge = -1,		/* point is on the leading edge of a glyph */
	kTrailingEdge = 0,		/* point is on the trailing edge of a glyph */
	kObjectEdge = 2			/* point is in the middle of an embedded object */
};

/*	values for WEFeatureFlag feature parameter */

enum {
	weFAutoScroll		=	0,		/* automatically scroll the selection range into view */
	weFOutlineHilite	=	2,		/* frame selection when deactivated */
	weFReadOnly			=	5,		/* disallow modifications */
	weFUndo				=	6,		/* support WEUndo() */
	weFIntCutAndPaste	=	7,		/* use intelligent cut-and-paste rules */
	weFDragAndDrop		=	8,		/* support drag-and-drop text editing */
	weFInhibitRecal		=	9,		/* don't recalculate line starts and don't redraw text */
	weFUseTempMem		=	10,		/* use temporary memory for main data structures */
	weFDrawOffscreen	=	11,		/* draw text offscreen for smoother visual results */
	weFInhibitRedraw	=	12,		/* don't redraw text */
	weFMonoStyled		=	13,		/* disallow style changes */
	weFInhibitColor		=	31		/* draw in black & white only */
};

/*	values for WENew flags parameter */

enum {
	weDoAutoScroll		=	1L << weFAutoScroll,
	weDoOutlineHilite	=	1L << weFOutlineHilite,
	weDoReadOnly		=	1L << weFReadOnly,
	weDoUndo			=	1L << weFUndo,
	weDoIntCutAndPaste	=	1L << weFIntCutAndPaste,
	weDoDragAndDrop		=	1L << weFDragAndDrop,
	weDoInhibitRecal	=	1L << weFInhibitRecal,
	weDoUseTempMem		=	1L << weFUseTempMem,
	weDoDrawOffscreen	=	1L << weFDrawOffscreen,
	weDoInhibitRedraw	=	1L << weFInhibitRedraw,
	weDoMonoStyled		=	1L << weFMonoStyled,
	weDoInhibitColor	=	1L << weFInhibitColor
};

/*	values for WEFeatureFlag action parameter */

enum {
	weBitToggle = -2,	/* toggles the specified feature */
	weBitTest,			/* returns the current setting of the specified feature */
	weBitClear,			/* disables the specified feature */
	weBitSet			/* enables the specified feature */
};

/*	selectors for WEGetInfo and WESetInfo */

enum {
	weCharByteHook		=	'cbyt', /* CharByte hook */
	weCharToPixelHook	=	'c2p ',	/* CharToPixel hook */
	weCharTypeHook		=	'ctyp', /* CharType hook */
	weClickLoop			=	'clik',	/* click loop callback */
	weCurrentDrag		=	'drag',	/* drag currently being tracked from WEClick() */
	weDrawTextHook		=	'draw', /* text drawing hook */
	weLineBreakHook		=	'lbrk',	/* line breaking hook */
	wePixelToCharHook	=	'p2c ', /* PixelToChar hook */
	wePort				=	'port',	/* graphics port */
	weRefCon			=	'refc',	/* reference constant for use by application */
	weScrollProc		=	'scrl',	/* auto-scroll callback */
	weText				=	'text',	/* text handle */
	weTranslateDragHook =	'xdrg', /* drag translation callback */
	weTSMDocumentID		=	'tsmd',	/* Text Services Manager document ID */
	weTSMPreUpdate		=	'pre ',	/* Text Services Manager pre-update callback */
	weTSMPostUpdate		=	'post',	/* Text Services Manager post-update callback */
	weURLHint			=	'urlh',	/* URL hint string for Internet Config */
	weWordBreakHook		=	'wbrk'	/* word breaking hook */
};

/*	values for WEInstallObjectHandler handlerSelector parameter */

enum {
	weNewHandler		=	'new ',		/* new handler */
	weDisposeHandler	=	'free',		/* dispose handler */
	weDrawHandler		=	'draw',		/* draw handler */
	weClickHandler		=	'clik',		/* click handler */
	weStreamHandler		=	'strm'		/* stream handler */
};

/*	action kinds */

enum {
	weAKNone			=	0,		/* null action */
	weAKUnspecified		=	1,		/* action of unspecified nature */
	weAKTyping			=	2,		/* some text has been typed in */
	weAKCut				=	3,		/* the selection range has been cut */
	weAKPaste			=	4,		/* something has been pasted */
	weAKClear			=	5,		/* the selection range has been deleted */
	weAKDrag			=	6,		/* drag and drop operation */
	weAKSetStyle		=	7		/* some style has been applied to a text range */
};

/*	destination kinds for stream handler */

enum {
	weToScrap			=	0,
	weToDrag			=	1,
	weToSoup			=	2
};

typedef struct OpaqueWEReference *WEReference;
typedef struct OpaqueWEObjectReference *WEObjectReference;
typedef Handle WESoupHandle;
typedef short WEActionKind;
typedef char WEAlignment;
typedef unsigned short WEStyleMode;
// typedef char FourCharCode[4];
typedef FourCharCode WESelector;
typedef WEReference WEHandle;	/* obsolete, kept for backward compatibility */

typedef struct WERunInfo {
	long 				runStart;	/* byte offset to first character of style run */
	long 				runEnd;		/* byte offset past last character of style run */
	short 				runHeight;	/* line height (ascent + descent + leading) */
	short 				runAscent;	/* font ascent */
	TextStyle 			runStyle;	/* text attributes */
	WEObjectReference	runObject;	/* either NULL or reference to embedded object */
} WERunInfo;


/*	callback prototypes */

typedef pascal Boolean (*WEClickLoopProcPtr)(WEReference we);
typedef pascal void (*WEScrollProcPtr)(WEReference we);
typedef pascal void (*WETSMPreUpdateProcPtr)(WEReference we);
typedef pascal void (*WETSMPostUpdateProcPtr)(WEReference we,
		long fixLength, long inputAreaStart, long inputAreaEnd,
		long pinRangeStart, long pinRangeEnd);
typedef pascal OSErr (*WETranslateDragProcPtr)(DragReference theDrag,
		ItemReference theItem, FlavorType requestedType, Handle putDataHere);
typedef pascal void (*WEDrawTextProcPtr)(Ptr pText, long textLength, Fixed slop,
		JustStyleCode styleRunPosition, WEReference we);
typedef pascal long (*WEPixelToCharProcPtr)(Ptr pText, long textLength, Fixed slop,
		Fixed *pixelWidth, char *edge, JustStyleCode styleRunPosition, Fixed hPos, WEReference we);
typedef pascal short (*WECharToPixelProcPtr)(Ptr pText, long textLength, Fixed slop,
		long offset, short direction, JustStyleCode styleRunPosition, long hPos, WEReference we);
typedef pascal StyledLineBreakCode (*WELineBreakProcPtr)(Ptr pText, long textLength,
		long textStart, long textEnd, Fixed *textWidth, long *textOffset, WEReference we);
typedef pascal void (*WEWordBreakProcPtr)(Ptr pText, short textLength, short offset,
		char edge, OffsetTable breakOffsets, ScriptCode script, WEReference we);
typedef pascal short (*WECharByteProcPtr)(Ptr pText, short textOffset, ScriptCode script,
		WEReference we);
typedef pascal short (*WECharTypeProcPtr)(Ptr pText, short textOffset, ScriptCode script, WEReference we);
typedef pascal OSErr (*WENewObjectProcPtr)(Point *defaultObjectSize,
		WEObjectReference obj);
typedef pascal OSErr (*WEDisposeObjectProcPtr)(WEObjectReference obj);
typedef pascal OSErr (*WEDrawObjectProcPtr)(const Rect *destRect,
		WEObjectReference obj);
typedef pascal Boolean (*WEClickObjectProcPtr)(Point hitPt, EventModifiers modifiers, unsigned long clickTime,
		WEObjectReference obj);
typedef pascal OSErr (*WEStreamObjectProcPtr)(short destKind, FlavorType *theType,
		Handle putDataHere, WEObjectReference obj);


/*	UPP proc info */

enum {
	uppWEClickLoopProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEReference /*we*/)))
};
enum {
	uppWEScrollProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEReference /*we*/)))
};
enum {
	uppWETSMPreUpdateProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEReference /*we*/)))
};
enum {
	uppWETSMPostUpdateProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEReference /*we*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(long /*fixLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(long /*inputAreaStart*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(long /*inputAreaEnd*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(long /*pinRangeStart*/)))
		| STACK_ROUTINE_PARAMETER(6,SIZE_CODE(sizeof(long /*pinRangeEnd*/)))
};
enum {
	uppWETranslateDragProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(DragReference /*theDrag*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(ItemReference /*theItem*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(FlavorType /*requestedType*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(Handle /*putDataHere*/)))
};
enum {
	uppWEDrawTextProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(long /*textLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(Fixed /*slop*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(JustStyleCode /*styleRunPosition*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(WEReference /*we*/)))
};
enum {
	uppWEPixelToCharProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(long)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(long /*textLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(Fixed /*slop*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(Fixed * /*pixelWidth*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(char * /*edge*/)))
		| STACK_ROUTINE_PARAMETER(6,SIZE_CODE(sizeof(JustStyleCode /*styleRunPosition*/)))
		| STACK_ROUTINE_PARAMETER(7,SIZE_CODE(sizeof(Fixed /*hPos*/)))
		| STACK_ROUTINE_PARAMETER(8,SIZE_CODE(sizeof(WEReference /*we*/)))
};
enum {
	uppWECharToPixelProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(long /*textLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(Fixed /*slop*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(long /*offset*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(short /*direction*/)))
		| STACK_ROUTINE_PARAMETER(6,SIZE_CODE(sizeof(JustStyleCode /*styleRunPosition*/)))
		| STACK_ROUTINE_PARAMETER(7,SIZE_CODE(sizeof(long /*hPos*/)))
		| STACK_ROUTINE_PARAMETER(8,SIZE_CODE(sizeof(WEReference /*we*/)))
};
enum {
	uppWELineBreakProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(StyledLineBreakCode )))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(long /*textLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(long /*textStart*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(long /*textEnd*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(Fixed * /*textWidth*/)))
		| STACK_ROUTINE_PARAMETER(6,SIZE_CODE(sizeof(long * /*textOffset*/)))
		| STACK_ROUTINE_PARAMETER(7,SIZE_CODE(sizeof(WEReference /*we*/)))
};
enum {
	uppWEWordBreakProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(short /*textLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(short /*offset*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(char /*edge*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(OffsetTable * /*breakOffsets*/)))
		| STACK_ROUTINE_PARAMETER(6,SIZE_CODE(sizeof(ScriptCode /*script*/)))
		| STACK_ROUTINE_PARAMETER(7,SIZE_CODE(sizeof(WEReference /*we*/)))
};
enum {
	uppWECharByteProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(short )))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(short /*textOffset*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(ScriptCode /*script*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(WEReference /*we*/)))
};
enum {
	uppWECharTypeProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(short )))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(short /*textOffset*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(ScriptCode /*script*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(WEReference /*we*/)))
};


enum {
	uppWENewObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Point * /*defaultObjectSize*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(WEObjectReference /*obj*/)))
};
enum {
	uppWEDisposeObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEObjectReference /*obj*/)))
};
enum {
	uppWEDrawObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(const Rect * /*destRect*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(WEObjectReference /*obj*/)))
};
enum {
	uppWEClickObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Point /*hitPt*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(EventModifiers /*modifiers*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(unsigned long /*clickTime*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(WEObjectReference /*obj*/)))
};
enum {
	uppWEStreamObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short /*destKind*/)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(FlavorType * /*theType*/)))
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Handle /*putDataHere*/)))
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(WEObjectReference /*obj*/)))
};


/*	UPPs, NewÅProc macros & CallÅProc macros */

/*
	NOTE:
    For compatibility with the Pascal version, CallÅProc macros take the form:

		CallFooProc(..., userRoutine)

	instead of:

		CallFooProc(userRoutine, ...)

*/

#if TARGET_RT_MAC_CFM

typedef UniversalProcPtr WEClickLoopUPP;
typedef UniversalProcPtr WEScrollUPP;
typedef UniversalProcPtr WETSMPreUpdateUPP;
typedef UniversalProcPtr WETSMPostUpdateUPP;
typedef UniversalProcPtr WETranslateDragUPP;
typedef UniversalProcPtr WEDrawTextUPP;
typedef UniversalProcPtr WEPixelToCharUPP;
typedef UniversalProcPtr WECharToPixelUPP;
typedef UniversalProcPtr WELineBreakUPP;
typedef UniversalProcPtr WEWordBreakUPP;
typedef UniversalProcPtr WECharByteUPP;
typedef UniversalProcPtr WECharTypeUPP;
typedef UniversalProcPtr WENewObjectUPP;
typedef UniversalProcPtr WEDisposeObjectUPP;
typedef UniversalProcPtr WEDrawObjectUPP;
typedef UniversalProcPtr WEClickObjectUPP;
typedef UniversalProcPtr WEStreamObjectUPP;

#define NewWEClickLoopProc(userRoutine) \
	(WEClickLoopUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEClickLoopProcInfo, GetCurrentArchitecture())
#define NewWEScrollProc(userRoutine) \
	(WEScrollUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEScrollProcInfo, GetCurrentArchitecture())
#define NewWETSMPreUpdateProc(userRoutine) \
	(WETSMPreUpdateUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWETSMPreUpdateProcInfo, GetCurrentArchitecture())
#define NewWETSMPostUpdateProc(userRoutine) \
	(WETSMPostUpdateUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWETSMPostUpdateProcInfo, GetCurrentArchitecture())
#define NewWETranslateDragProc(userRoutine) \
	(WETranslateDragUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWETranslateDragProcInfo, GetCurrentArchitecture())
#define NewWEDrawTextProc(userRoutine) \
	(WEDrawTextUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEDrawTextProcInfo, GetCurrentArchitecture())
#define NewWEPixelToCharProc(userRoutine) \
	(WEPixelToCharUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEPixelToCharProcInfo, GetCurrentArchitecture())
#define NewWECharToPixelProc(userRoutine) \
	(WECharToPixelUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWECharToPixelProcInfo, GetCurrentArchitecture())
#define NewWELineBreakProc(userRoutine) \
	(WELineBreakUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWELineBreakProcInfo, GetCurrentArchitecture())
#define NewWEWordBreakProc(userRoutine) \
	(WEWordBreakUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEWordBreakProcInfo, GetCurrentArchitecture())
#define NewWECharByteProc(userRoutine) \
	(WECharByteUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWECharByteProcInfo, GetCurrentArchitecture())
#define NewWECharTypeProc(userRoutine) \
	(WECharTypeUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWECharTypeProcInfo, GetCurrentArchitecture())
#define NewWENewObjectProc(userRoutine) \
	(WENewObjectUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWENewObjectProcInfo, GetCurrentArchitecture())
#define NewWEDisposeObjectProc(userRoutine) \
	(WEDisposeObjectUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEDisposeObjectProcInfo, GetCurrentArchitecture())
#define NewWEDrawObjectProc(userRoutine) \
	(WEDrawObjectUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEDrawObjectProcInfo, GetCurrentArchitecture())
#define NewWEClickObjectProc(userRoutine) \
	(WEClickObjectUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEClickObjectProcInfo, GetCurrentArchitecture())
#define NewWEStreamObjectProc(userRoutine) \
	(WEStreamObjectUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEStreamObjectProcInfo, GetCurrentArchitecture())

#define CallWEClickLoopProc(we, userRoutine) \
	CallUniversalProc((userRoutine), uppWEClickLoopProcInfo, (we))
#define CallWEScrollProc(we, userRoutine) \
	CallUniversalProc((userRoutine), uppWEScrollProcInfo, (we))
#define CallWETSMPreUpdateProc(we, userRoutine) \
	CallUniversalProc((userRoutine), uppWETSMPreUpdateProcInfo, (we))
#define CallWETSMPostUpdateProc(we, fixLength, inputAreaStart, inputAreaEnd, pinRangeStart, pinRangeEnd, userRoutine) \
	CallUniversalProc((userRoutine), uppWETSMPostUpdateProcInfo, (we), (fixLength), (inputAreaStart), (inputAreaEnd), (pinRangeStart), (pinRangeEnd))
#define CallWETranslateDragProc(theDrag, theItem, requestedType, putDataHere, userRoutine) \
	CallUniversalProc((userRoutine), uppWETranslateDragProcInfo, (theDrag), (theItem), (requestedType), (putDataHere))
#define CallWEDrawTextProc(pText, textLength, slop, styleRunPosition, we, userRoutine) \
	CallUniversalProc((userRoutine), uppWEDrawTextProcInfo, (pText), (textLength), (slop), (styleRunPosition), (we))
#define CallWEPixelToCharProc(pText, textLength, slop, pixelWidth, edge, styleRunPosition, hPos, we, userRoutine) \
	CallUniversalProc((userRoutine), uppWEPixelToCharProcInfo, (pText), (textLength), (slop), (pixelWidth), (edge), (styleRunPosition), (hPos), (we))
#define CallWECharToPixelProc(pText, textLength, slop, offset, direction, styleRunPosition, hPos, we, userRoutine) \
	CallUniversalProc((userRoutine), uppWECharToPixelProcInfo, (pText), (textLength), (slop), (offset), (direction), (styleRunPosition), (hPos), (we))
#define CallWELineBreakProc(pText, textLength, textStart, textEnd, textWidth, textOffset, we, userRoutine) \
	CallUniversalProc((userRoutine), uppWELineBreakProcInfo, (pText), (textLength), (textStart), (textEnd), (textWidth), (textOffset), (we))
#define CallWEWordBreakProc(pText, textLength, offset, edge, breakOffsets, script, we, userRoutine) \
	CallUniversalProc((userRoutine), uppWEWordBreakProcInfo, (pText), (textLength), (offset), (edge), (breakOffsets), (script), (we))
#define CallWECharByteProc(pText, textOffset, script, we, userRoutine) \
	CallUniversalProc((userRoutine), uppWECharByteProcInfo, (pText), (textOffset), (script), (we))
#define CallWECharTypeProc(pText, textOffset, script, we, userRoutine) \
	CallUniversalProc((userRoutine), uppWECharTypeProcInfo, (pText), (textOffset), (script), (we))
#define CallWENewObjectProc(defaultObjectSize, obj, userRoutine) \
	CallUniversalProc((userRoutine), uppWENewObjectProcInfo, (defaultObjectSize), (obj))
#define CallWEDisposeObjectProc(obj, userRoutine) \
	CallUniversalProc((userRoutine), uppWEDisposeObjectProcInfo, (obj))
#define CallWEDrawObjectProc(destRect, obj, userRoutine) \
	CallUniversalProc((userRoutine), uppWEDrawObjectProcInfo, (destRect), (obj))
#define CallWEClickObjectProc(hitPt, modifiers, clickTime, obj, userRoutine) \
	CallUniversalProc((userRoutine), uppWEClickObjectProcInfo, (hitPt), (modifiers), (clickTime), (obj))
#define CallWEStreamObjectProc(destKind, theType, putDataHere, obj, userRoutine) \
	CallUniversalProc((userRoutine), uppWEStreamObjectProcInfo, (destKind), (theType), (putDataHere), (obj))

#else

typedef WEClickLoopProcPtr WEClickLoopUPP;
typedef WEScrollProcPtr WEScrollUPP;
typedef WETSMPreUpdateProcPtr WETSMPreUpdateUPP;
typedef WETSMPostUpdateProcPtr WETSMPostUpdateUPP;
typedef WETranslateDragProcPtr WETranslateDragUPP;
typedef WEDrawTextProcPtr WEDrawTextUPP;
typedef WEPixelToCharProcPtr WEPixelToCharUPP;
typedef WECharToPixelProcPtr WECharToPixelUPP;
typedef WELineBreakProcPtr WELineBreakUPP;
typedef WEWordBreakProcPtr WEWordBreakUPP;
typedef WECharByteProcPtr WECharByteUPP;
typedef WECharTypeProcPtr WECharTypeUPP;
typedef WENewObjectProcPtr WENewObjectUPP;
typedef WEDisposeObjectProcPtr WEDisposeObjectUPP;
typedef WEDrawObjectProcPtr WEDrawObjectUPP;
typedef WEClickObjectProcPtr WEClickObjectUPP;
typedef WEStreamObjectProcPtr WEStreamObjectUPP;

#define NewWEClickLoopProc(userRoutine) ((WEClickLoopUPP) (userRoutine))
#define NewWEScrollProc(userRoutine) ((WEScrollUPP) (userRoutine))
#define NewWETSMPreUpdateProc(userRoutine) ((WETSMPreUpdateUPP) (userRoutine))
#define NewWETSMPostUpdateProc(userRoutine) ((WETSMPostUpdateUPP) (userRoutine))
#define NewWETranslateDragProc(userRoutine) ((WETranslateDragUPP) (userRoutine))
#define NewWEDrawTextProc(userRoutine) ((WEDrawTextUPP) (userRoutine))
#define NewWEPixelToCharProc(userRoutine) ((WEPixelToCharUPP) (userRoutine))
#define NewWECharToPixelProc(userRoutine) ((WECharToPixelUPP) (userRoutine))
#define NewWELineBreakProc(userRoutine) ((WELineBreakUPP) (userRoutine))
#define NewWEWordBreakProc(userRoutine) ((WEWordBreakUPP) (userRoutine))
#define NewWECharByteProc(userRoutine) ((WECharByteUPP) (userRoutine))
#define NewWECharTypeProc(userRoutine) ((WECharTypeUPP) (userRoutine))
#define NewWENewObjectProc(userRoutine) ((WENewObjectUPP) (userRoutine))
#define NewWEDisposeObjectProc(userRoutine) ((WEDisposeObjectUPP) (userRoutine))
#define NewWEDrawObjectProc(userRoutine) ((WEDrawObjectUPP) (userRoutine))
#define NewWEClickObjectProc(userRoutine) ((WEClickObjectUPP) (userRoutine))
#define NewWEStreamObjectProc(userRoutine) ((WEStreamObjectUPP) (userRoutine))

#define CallWEClickLoopProc(we, userRoutine) \
	(*(userRoutine))((we))
#define CallWEScrollProc(we, userRoutine) \
	(*(userRoutine))((we))
#define CallWETSMPreUpdateProc(we, userRoutine) \
	(*(userRoutine))((we))
#define CallWETSMPostUpdateProc(we, fixLength, inputAreaStart, inputAreaEnd, pinRangeStart, pinRangeEnd, userRoutine) \
	(*(userRoutine))((we), (fixLength), (inputAreaStart), (inputAreaEnd), (pinRangeStart), (pinRangeEnd))
#define CallWETranslateDragProc(theDrag, theItem, requestedType, putDataHere, userRoutine) \
	(*(userRoutine))((theDrag), (theItem), (requestedType), (putDataHere))
#define CallWEDrawTextProc(pText, textLength, slop, styleRunPosition, we, userRoutine) \
	(*(userRoutine))((pText), (textLength), (slop), (styleRunPosition), (we))
#define CallWEPixelToCharProc(pText, textLength, slop, pixelWidth, edge, styleRunPosition, hPos, we, userRoutine) \
	(*(userRoutine))((pText), (textLength), (slop), (pixelWidth), (edge), (styleRunPosition), (hPos), (we))
#define CallWECharToPixelProc(pText, textLength, slop, offset, direction, styleRunPosition, hPos, we, userRoutine) \
	(*(userRoutine))((pText), (textLength), (slop), (offset), (direction), (styleRunPosition), (hPos), (we))
#define CallWELineBreakProc(pText, textLength, textStart, textEnd, textWidth, textOffset, we, userRoutine) \
	(*(userRoutine))((pText), (textLength), (textStart), (textEnd), (textWidth), (textOffset), (we))
#define CallWEWordBreakProc(pText, textLength, offset, edge, breakOffsets, script, we, userRoutine) \
	(*(userRoutine))((pText), (textLength), (offset), (edge), (breakOffsets), (script), (we))
#define CallWECharByteProc(pText, textOffset, script, we, userRoutine) \
	(*(userRoutine))((pText), (textOffset), (script), (we))
#define CallWECharTypeProc(pText, textOffset, script, we, userRoutine) \
	(*(userRoutine))((pText), (textOffset), (script), (we))
#define CallWENewObjectProc(defaultObjectSize, obj, userRoutine) \
	(*(userRoutine))((defaultObjectSize), (obj))
#define CallWEDisposeObjectProc(obj, userRoutine) \
	(*(userRoutine))((obj))
#define CallWEDrawObjectProc(destRect, obj, userRoutine) \
	(*(userRoutine))((destRect), (obj))
#define CallWEClickObjectProc(hitPt, modifiers, clickTime, obj, userRoutine) \
	(*(userRoutine))((hitPt), (modifiers), (clickTime), (obj))
#define CallWEStreamObjectProc(destKind, theType, putDataHere, obj, userRoutine) \
	(*(userRoutine))((destKind), (theType), (putDataHere), (obj))

#endif


/*	WASTE public calls */

#ifdef __cplusplus
extern "C" {
#endif

/*	creation and destruction */

pascal OSErr WENew(const LongRect *destRect, const LongRect *viewRect, unsigned long flags, WEReference *we);
pascal void WEDispose(WEReference we);

/*	getting variables */

pascal Handle WEGetText(WEReference we);
pascal short WEGetChar(long offset, WEReference we);
pascal long WEGetTextLength(WEReference we);
pascal long WECountLines(WEReference we);
pascal long WEGetHeight(long startLine, long endLine, WEReference we);
pascal void WEGetSelection(long *selStart, long *selEnd, WEReference we);
pascal void WEGetDestRect(LongRect *destRect, WEReference we);
pascal void WEGetViewRect(LongRect *viewRect, WEReference we);
pascal Boolean WEIsActive(WEReference we);
pascal long WEOffsetToLine (long offset, WEReference we);
pascal void WEGetLineRange(long lineNo, long *lineStart, long *lineEnd, WEReference we);

/*	setting variables */

pascal void WESetSelection(long selStart, long selEnd, WEReference we);
pascal void WESetDestRect(const LongRect *destRect, WEReference we);
pascal void WESetViewRect(const LongRect *viewRect, WEReference we);

/*	accessing style run information */

pascal Boolean WEContinuousStyle(WEStyleMode *mode, TextStyle *ts, WEReference we);
pascal void WEGetRunInfo(long offset, WERunInfo *runInfo, WEReference we);

/*	converting byte offsets to screen position and vice versa */

pascal long WEGetOffset(const LongPt *thePoint, char *edge, WEReference we);
pascal void WEGetPoint(long offset, LongPt *thePoint, short *lineHeight, WEReference we);

/*	finding words and lines */

pascal void WEFindWord(long offset, char edge, long *wordStart, long *wordEnd, WEReference we);
pascal void WEFindLine(long offset, char edge, long *lineStart, long *lineEnd, WEReference we);

/*	making a copy of a text range */

pascal OSErr WECopyRange(long rangeStart, long rangeEnd, Handle hText, StScrpHandle hStyles, WESoupHandle hSoup, WEReference we);

/*	getting and setting the alignment style */

pascal WEAlignment WEGetAlignment(WEReference we);
pascal void WESetAlignment(WEAlignment alignment, WEReference we);

/*	recalculating line breaks, drawing and scrolling */

pascal OSErr WECalText(WEReference we);
pascal void WEUpdate(RgnHandle updateRgn, WEReference we);
pascal void WEScroll(long hOffset, long vOffset, WEReference we);
pascal void WESelView(WEReference we);

/*	handling activate / deactivate events */

pascal void WEActivate(WEReference we);
pascal void WEDeactivate(WEReference we);

/* 	handling key-down events */

pascal void WEKey(short key, EventModifiers modifiers, WEReference we);

/*	handling mouse-down events and mouse tracking */

pascal void WEClick(Point hitPt, EventModifiers modifiers, unsigned long clickTime, WEReference we);

/*	adjusting the cursor shape */

pascal Boolean WEAdjustCursor(Point mouseLoc, RgnHandle mouseRgn, WEReference we);

/*	blinking the caret */

pascal void WEIdle(unsigned long *maxSleep, WEReference we);

/*	modifying the text and the styles */

pascal OSErr WEInsert(const void *pText, long textLength, StScrpHandle hStyles, WESoupHandle hSoup, WEReference we);
pascal OSErr WEDelete(WEReference we);
pascal OSErr WESetStyle(WEStyleMode mode, const TextStyle *ts, WEReference we);
pascal OSErr WEUseStyleScrap(StScrpHandle hStyles, WEReference we);
pascal OSErr WEUseText(Handle hText, WEReference we);

/*	undo */

pascal OSErr WEUndo(WEReference we);
pascal void WEClearUndo(WEReference we);
pascal WEActionKind WEGetUndoInfo(Boolean *redoFlag, WEReference we);
pascal Boolean WEIsTyping(WEReference we);

/*	keeping track of changes */

pascal unsigned long WEGetModCount(WEReference we);
pascal void WEResetModCount(WEReference we);

/*	embedded objects */

pascal OSErr WEInstallObjectHandler(FlavorType objectType, WESelector handlerSelector, UniversalProcPtr handler, WEReference we);
pascal OSErr WEInsertObject(FlavorType objectType, Handle objectDataHandle, Point objectSize, WEReference we);
pascal OSErr WEGetSelectedObject(WEObjectReference *obj, WEReference we);
pascal long WEFindNextObject(long offset, WEObjectReference *obj, WEReference we);
pascal OSErr WEUseSoup(WESoupHandle hSoup, WEReference we);

/*	accessing embedded object attributes */

pascal FlavorType WEGetObjectType(WEObjectReference obj);
pascal Handle WEGetObjectDataHandle(WEObjectReference obj);
pascal Point WEGetObjectSize(WEObjectReference obj);
pascal WEReference WEGetObjectOwner(WEObjectReference obj);
pascal long WEGetObjectRefCon(WEObjectReference obj);
pascal void WESetObjectRefCon(WEObjectReference obj, long refCon);

/*	clipboard operations */

pascal OSErr WECut(WEReference we);
pascal OSErr WECopy(WEReference we);
pascal OSErr WEPaste(WEReference we);
pascal Boolean WECanPaste(WEReference we);

/*	Drag Manager support */

pascal RgnHandle WEGetHiliteRgn(long rangeStart, long rangeEnd, WEReference we);
pascal OSErr WETrackDrag(DragTrackingMessage message, DragReference drag, WEReference we);
pascal OSErr WEReceiveDrag(DragReference drag, WEReference we);
pascal Boolean WECanAcceptDrag(DragReference drag, WEReference we);
pascal Boolean WEDraggedToTrash(DragReference drag);

/*	Script Manager utilities */

pascal short WECharByte(long offset, WEReference we);
pascal short WECharType(long offset, WEReference we);

/*	Text Services Manager support */

pascal OSErr WEInstallTSMHandlers(void);
pascal OSErr WERemoveTSMHandlers(void);
pascal void WEStopInlineSession(WEReference we);

/*	additional features */

pascal short WEFeatureFlag(short feature, short action, WEReference we);
pascal OSErr WEGetInfo(WESelector selector, void *info, WEReference we);
pascal OSErr WESetInfo(WESelector selector, const void *info, WEReference we);

/*	long coordinate utilities */

pascal void WELongPointToPoint(const LongPt *lp, Point *p);
pascal void WEPointToLongPoint(Point p, LongPt *lp);
pascal void WESetLongRect(LongRect *lr, long left, long top, long right, long bottom);
pascal void WELongRectToRect(const LongRect *lr, Rect *r);
pascal void WERectToLongRect(const Rect *r, LongRect *lr);
pascal void WEOffsetLongRect(LongRect *lr, long hOffset, long vOffset);
pascal Boolean WELongPointInLongRect(const LongPt *lp, const LongRect *lr);

#ifdef __cplusplus
}
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=reset
#endif

#endif
