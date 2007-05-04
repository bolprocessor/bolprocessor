/*
 *	WASTEIntf.h
 *
 *	WASTE PROJECT
 *  Internal (private) interface
 *
 *  Copyright (c) 1993-1996 Marco Piovanelli
 *	All Rights Reserved
 *
 *  C port by Dan Crevier
 *
 */

// Modified by Bel on 30/10/00

//	Toolbox #includes

#if TARGET_API_MAC_CARBON

// #include <Carbon/Carbon.h>  // included correctly by CW headers

#else  /* non-Carbon includes */

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif

#ifndef __DRAG__
#include <Drag.h> // Added 30/10/00
#endif

#ifndef __TYPES__
#include <Types.h>
#endif

#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif

#ifndef __ERRORS__
#include <MacErrors.h>
#endif

#ifndef __GESTALT__
#include <Gestalt.h>
#endif

#ifndef __MEMORY__
#include <Memory.h>
#endif

#ifndef __FONTS__
#include <Fonts.h>
#endif

#ifndef __QUICKDRAWTEXT__
#include <QuickdrawText.h>
#endif

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif

#ifndef __QDOFFSCREEN__
#include <QDOffscreen.h>
#endif

#ifndef __PALETTES__
#include <Palettes.h>
#endif

#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif

#ifndef __SCRAP__
#include <Scrap.h>
#endif

#ifndef __SCRIPT__
#include <Script.h>
#endif

#ifndef __TOOLUTILS__
#include <ToolUtils.h>
#endif

#ifndef __TEXTUTILS__
#include <TextUtils.h>
#endif

#ifndef __LOWMEM__
#include <LowMem.h>
#endif

#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif

#ifndef __AEREGISTRY__
#include <AERegistry.h>
#endif

// #ifndef __DRAG__
// #include "Drag.h"
// #endif

#ifndef __TEXTSERVICES__
#include <TextServices.h>
#endif

#endif /* TARGET_API_MAC_CARBON */

//	ANSI #includes

#ifndef _LIMITS
#include <limits.h>
#endif

//	other #includes

#ifndef __LONGCOORDINATES__
#ifndef _LongCoords_
#include "LongCoords.h"
#endif
#endif

// MPW needs topLeft and botRight defined here
#ifndef topLeft
#define topLeft(r)              (((Point *) &(r))[0])
#endif

#ifndef botRight
#define botRight(r)             (((Point *) &(r))[1])
#endif

// pascal-like macros for testing, setting, clearing and inverting bits
#define BTST( FLAGS, BIT )	((FLAGS) &   (1L << (BIT)))
#define BSET( FLAGS, BIT )  ((FLAGS) |=  (1L << (BIT)))
#define BCLR( FLAGS, BIT )  ((FLAGS) &= ~(1L << (BIT)))
#define BCHG( FLAGS, BIT )  ((FLAGS) ^=  (1L << (BIT)))

// pascal-like macros for shifting bits
#define BSL( A, B )			(((long) (A)) << (B))
#define BSR( A, B )			(((long) (A)) >> (B))

// macros used in conjunction with the styleRunPosition parameter
#define IS_FIRST_RUN(RUN)	((RUN) <= leftStyleRun)
#define IS_LAST_RUN(RUN)	(! ((RUN) & 0x01))

// other macros
#define ABS(A) 				((A) > 0 ? (A) : -(A))
#define BLOCK_CLR(X)		_WEBlockClr(&(X), sizeof(X));

// set WASTE_DEBUG to 1 to perform various consistency checks;
// errors will be reported with DebugStr()
// WARNING: when WASTE_DEBUG is set, WEIdle() calls can be extremely time-consuming
#ifndef WASTE_DEBUG
#define WASTE_DEBUG				0
#endif

// set WASTE_OBJECTS to 0 if you don't need embedded objects
#ifndef WASTE_OBJECTS
#define	WASTE_OBJECTS			1
#endif

// set WASTE_USE_UPPS to 0 if you don't need UPPs
#ifndef WASTE_USE_UPPS
#define WASTE_USE_UPPS			TARGET_RT_MAC_CFM
#endif

// set WASTE_REDRAW_SPEED to 1 to use Mark Alldritt's optimizations of _WERedraw
#ifndef WASTE_REDRAW_SPEED
#define WASTE_REDRAW_SPEED		0
#endif

// set WASTE_PIN_SCROLL to 1 to use Chris Thomas's version of WEScroll(),
// which behaves like TEPinScroll()
#ifndef WASTE_PIN_SCROLL
#define WASTE_PIN_SCROLL		0
#endif

// set WASTE_NO_SYNCH to 1 if you don't want WASTE to synchronize keyboard and fonts
#ifndef WASTE_NO_SYNCH
#define WASTE_NO_SYNCH			0
#endif

// set WASTE_NO_RO_CARET to 1 to hide the caret in read-only mode
#ifndef WASTE_NO_RO_CARET
#define WASTE_NO_RO_CARET		0
#endif

// set WASTE_IC_SUPPORT to 1 to use Dan Crevier's
// support of URL cmd-clicking through Internet Config
#ifndef WASTE_IC_SUPPORT
#define WASTE_IC_SUPPORT		0
#endif

// If WASTE_OBJECTS_ARE_GLYPHS is set to 0 (default), WEGetOffset returns kObjectEdge in
// the edge parameter when thePoint is in the middle half of an object: as a result,
// clicking in the middle of an object immediately _selects_ the object, so that a second
// click immediately following triggers the 'clik' callback (this can be handy to make
// sound objects play when they are double-clicked, for example).

// If WASTE_OBJECTS_ARE_GLYPHS is set to 1, WEGetOffset treats embedded objects
// like ordinary glyphs and never returns kObjectEdge in the edge parameter:
// as a result, clicking in the middle of an object always positions the caret either
// to the left or to the right of the object.

#ifndef WASTE_OBJECTS_ARE_GLYPHS
#define WASTE_OBJECTS_ARE_GLYPHS	0
#endif

// set WASTE_RESOLVE_FONT_DESIGNATORS to false if you don't want WASTE to
// map font designators (the special IDs 0 and 1 that identify the system
// and application fonts, respectively) to the actual font IDs.

#ifndef WASTE_RESOLVE_FONT_DESIGNATORS
#define WASTE_RESOLVE_FONT_DESIGNATORS	1
#endif

// set WASTE_WECALTEXT_DOES_REDRAW to 1 if you want WECalText()
// to automatically redraw all the text (as in WASTE 1.1 final)

#ifndef WASTE_WECALTEXT_DOES_REDRAW
#define WASTE_WECALTEXT_DOES_REDRAW		0
#endif

// set WASTE_KURTHS_OPTION_ARROWS to 0 to use the old
// implementation of option + left/right arrow combinations, which is
// less conformant to Apple's HIGs than the current implementation,
// due to Stefan Kurth

#ifndef WASTE_KURTHS_OPTION_ARROWS
#define WASTE_KURTHS_OPTION_ARROWS		1
#endif

// macros for debugging

#if WASTE_DEBUG
#define WEASSERT(CONDITION, WARNING)	{ if (!(CONDITION)) DebugStr(WARNING); }
#else
#define WEASSERT(CONDITION, WARNING)
#endif

// determine if inline functions are supported

#if defined(__cplusplus) || defined(__MWERKS__)
#define		INLINE	inline
#else
#define		INLINE
#endif

#ifndef UNIVERSAL_INTERFACES_VERSION
typedef unsigned short EventModifiers;	// defined in the Universal Headers v2.1 or later
#endif

// result codes
enum {
	weCantUndoErr				=	-10015,	// undo buffer is clear (= errAECantUndo)
	weEmptySelectionErr			=	-10013,	// selection range is empty (= errAENoUserSelection)
	weUnknownObjectTypeErr		=	-9478,	// specified object type is not registered
	weObjectNotFoundErr			=	-9477,	// no object found at specified offset
	weReadOnlyErr				=	-9476,	// instance is read-only
	weMonoStyledErr				=	-9475,	// instance is mono-styled
	weNotHandledErr				=	-1708,  // please use default handling (= errAEEventNotHandled)
	weUndefinedSelectorErr		=	-50,	// unknown selector
	weNoDragErr 				= 	128		// can be returned internally by _WEDrag
};

// values for WEInstallObjectHandler handlerSelector parameter
enum {
	weNewHandler 		= 	'new ',
	weDisposeHandler 	= 	'free',
	weDrawHandler 		= 	'draw',
	weClickHandler 		= 	'clik',
	weStreamHandler		=	'strm'
};

// action kinds
enum {
	weAKNone			=	0,		// null action
	weAKUnspecified		=	1,		// action of unspecified nature
	weAKTyping			=	2,		// some text has been typed in
	weAKCut				=	3,		// the selection range has been cut
	weAKPaste			=	4,		// something has been pasted
	weAKClear			=	5,		// the selection range has been deleted
	weAKDrag			=	6,		// drag and drop operation
	weAKSetStyle		=	7		// some style has been applied to a text range
};

// action flags
enum {
	weAFIsRedo			=	0x0001,	// action saves edit state prior to a WEUndo call
	weAFDontSaveText	=	0x0002, // don't save text
	weAFDontSaveStyles	=	0x0004,	// don't save styles
	weAFDontSaveSoup	=	0x0008	// don't save soup
};

// destination kinds for streaming objects
enum {
	weToScrap			=	0,
	weToDrag			=	1,
	weToSoup			=	2
};

// alignment styles
enum {
	weFlushLeft 		=	-2,		// flush left
	weFlushRight		=	-1,		// flush right
	weFlushDefault		=	 0,		// flush according to system direction
	weCenter			=	 1,		// centered
	weJustify			=	 2		// fully justified
};

// values for the edge parameter
enum {
	kLeadingEdge 		= 	-1,		// point is on the leading edge of a glyph
	kTrailingEdge 		= 	 0,		// point is on the trailing edge of a glyph
	kObjectEdge 		= 	 2		// point is inside an embedded object
};

// control character codes
enum {
	kObjectMarker 		=	0x01,
	kBackspace 			=	0x08,
	kTab 				=	0x09,
	kEOL 				=	0x0D,
	kArrowLeft 			=	0x1C,
	kArrowRight 		=	0x1D,
	kArrowUp 			=	0x1E,
	kArrowDown 			=	0x1F,
	kSpace 				=  	0x20,
	kForwardDelete 		=	0x7F
};

// bit equates for the tsFlags field of the WETextStyle record
enum {
	tsTSMHilite 		=	4,		// set if style run is part of active input area
	tsTSMSelected 		=	5,		// set for selected raw/converted text
	tsTSMConverted 		=	6,		// set for converted text, clear for raw text
	tsRightToLeft 		=	7		// set for right-to-left runs
};

// bit equates for the mode parameter in WESetStyle and WEContinuousStyle
enum {
	kModeFont 			=	0,		// change font
	kModeFace 			=	1,		// change Quickdraw styles
	kModeSize 			=	2,		// change point size
	kModeColor 			=	3,		// change color
	kModeAddSize 		=	4,		// add tsSize to existing size
	kModeToggleFace 	=	5,		// toggle continuous styles rather than setting them
	kModeReplaceFace 	=	6,		// tsFace replaces existing styles outright
	kModePreserveScript =	7,		// apply font to runs of same script only
	kModeExtractSubscript = 8,		// apply font to eligible subruns of different script
	kModeFaceMask 		= 	9,		// change Quickdraw styles; use mask
	kModeObject 		= 	14,		// change tsObject field
	kModeFlags 			= 	15		// change tsFlags field
};

// values for the mode parameter in WESetStyle and WEContinuousStyle
enum {
	weDoFont				=	1 << kModeFont,
	weDoFace				=	1 << kModeFace,
	weDoSize				=	1 << kModeSize,
	weDoColor				=	1 << kModeColor,
	weDoAll					=	weDoFont + weDoFace + weDoSize + weDoColor,
	weDoAddSize				=	1 << kModeAddSize,
	weDoToggleFace			=	1 << kModeToggleFace,
	weDoReplaceFace			=	1 << kModeReplaceFace,
	weDoPreserveScript		=	1 << kModePreserveScript,
	weDoSmartFont			=	weDoFont + weDoPreserveScript,
	weDoExtractSubscript	=	1 << kModeExtractSubscript,
	weDoFaceMask			=	1 << kModeFaceMask,
	weDoObject				=	1 << kModeObject,
	weDoFlags				=	1 << kModeFlags
};

// values for WEFeatureFlag action parameter
enum {
	weBitSet 			=	 1,		// enables the specified feature
	weBitClear 			=	 0,		// disables the specified feature
	weBitTest 			=	-1,		// returns the current setting of the specified feature
	weBitToggle 		=	-2		// toggles the specified feature
};

// bit equates for the flags field in the WE record
enum {
	weFHasColorQD 		=	31,		// Color Quickdraw is available
	weFHasDragManager 	=	30,		// the Drag Manager is available
	weFHasTextServices	=	29,		// the Text Services Manager is available
	weFNonRoman			=	28,		// at least one non-Roman script is enabled
	weFDoubleByte		=	27,		// a double-byte script is enabled
	weFBidirectional	=	26,		// a bidirectional script is enabled
	weFUseDualCaret		=	25,		// use dual caret at direction boundaries
	weFCaretVisible		=	24,		// the caret is currently visible
	weFMouseTracking	=	23,		// set internally during mouse tracking
	weFAnchorIsEnd		=	22,		// anchor offset is selEnd
	weFUseNullStyle		=	21,		// a null style is associated with the empty selection
	weFActive			=	20,		// we're active
	weFHilited			=	19,		// true if text pane is highlighted (for Drag & Drop)
	weFCanAcceptDrag	=	18,		// the drag in the text pane can be accepted
	weFDragCaretVisible	=	17,		// drag caret is currently visible
	weFDestRectChanged	=	16		// set if scroll callback needs to be called
};

// bit equates for the features field in the WE record
enum {
	weFInhibitColor		=	31,		// draw in black & white only; ignore color
	weFMonoStyled		=	13,		// disallow style changes
	weFInhibitRedraw	=	12,		// don't redraw text
	weFDrawOffscreen	=	11,		// draw text offscreen for smoother visual results
	weFUseTempMem		=	10,		// use temporary memory for main data structures
	weFInhibitRecal		=	 9,		// don't recalculate line breaks and don't redraw text
	weFDragAndDrop		=	 8,		// support drag and drop
	weFIntCutAndPaste	=	 7,		// intelligent cut & paste
	weFUndoSupport		=	 6,		// support undo/redo
	weFReadOnly			=	 5,		// disallow editing
	weFOutlineHilite	=	 2,		// frame selection range when text pane is inactive
	weFAutoScroll		=	 0		// automatically scroll text when cursor is outside pane
};

// scrap types
enum {
	kTypeText			=	'TEXT',
	kTypeStyles 		=	'styl',
	kTypeSoup 			=	'SOUP'
};

// selectors for WEGetInfo/WESetInfo
enum {
	weCharByteHook		=	'cbyt',
	weCharTypeHook		=	'ctyp',
	weCharToPixelHook	=	'c2p ',
	weClickLoop			=	'clik',
	weCurrentDrag		=	'drag',
	weDrawTextHook		=	'draw',
	weLineArray			=	'line',
	weLineBreakHook		=	'lbrk',
	wePixelToCharHook	=	'p2c ',
	wePort				=	'port',
	weRefCon			=	'refc',
	weRunArray			=	'runa',
	weScrollProc		=	'scrl',
	weStyleTable		=	'styl',
	weText				=	'text',
	weTranslateDragHook	=	'xdrg',
	weTSMDocumentID		=	'tsmd',
	weTSMPostUpdate		=	'post',
	weTSMPreUpdate		=	'pre ',
	weURLHint			=	'urlh',
	weWordBreakHook		=	'wbrk'
};

// possible values returned by WEIntelligentPaste
enum {
	weDontAddSpaces			=	 0,
	weAddSpaceOnLeftSide	=	-1,
	weAddSpaceOnRightSide	=	 1
};

// values for WEAllocate allocFlags parameter
enum {
	kAllocClear			=	0x0001,		// clear handle after allocation
	kAllocTemp			=	0x0002		// use temporary memory if available
};

// other miscellaneous constants
enum {
	kCaretWidth = 1,			// width of the caret, in pixels
	kMinFontSize = 1,			// minimum valid font size
	kMaxFontSize = SHRT_MAX,	// maximum valid font size
	kInvalidOffset = -1, 		// used to flag an invalid or nonexistent offset
	kUnknownObjectType = -1,	// returned by _WELookupObjectType for unknown flavors
	kNullDrag = 0,				// null or nonexistent drag reference
	kTextMargin = 3,			// width of border area surrounding the text (in pixels)
	kMaxScrollDelta = 30,		// maximum scroll amount used by standard click loop
	kAutoScrollDelay = 10,		// delay before auto-scroll starts (in ticks)
	kDefaultObjectHeight = 32,	// default height for new objects
	kDefaultObjectWidth = 32	// default width for new objects
};

extern const Point kOneToOneScaling;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif

typedef unsigned short WEStyleMode;
typedef char WEAlignment;
typedef short WEActionKind;
typedef unsigned short WEActionFlags;

// forward declarations

typedef struct WERec *WEPtr, **WEHandle;
typedef struct WEObjectDesc *WEObjectDescPtr, **WEObjectDescHandle;

typedef struct WETextStyle {
	short tsFont;					// font family number
	Style tsFace;					// set of Quickdraw styles
	unsigned char tsFlags;			// flags (FOR INTERNAL USE ONLY)
	short tsSize;					// font size, in integer points
	RGBColor tsColor;				// absolute red-green-blue color
#if WASTE_OBJECTS
	WEObjectDescHandle tsObject;	// embedded object reference
#endif
} WETextStyle, *WETextStylePtr, **WETextStyleHandle;

typedef struct WERunAttributes {
	short runHeight;			// style run height (ascent + descent + leading)
	short runAscent;			// font ascent
	WETextStyle runStyle;		// text style
} WERunAttributes, *WERunAttributesPtr, **WERunAttributesHandle;

typedef struct WERunInfo {
	long runStart;				// byte offset to first character of this style run
	long runEnd;				// byte offset to first character of next style run
	WERunAttributes runAttrs;	// run attributes
} WERunInfo, *WERunInfoPtr, **WERunInfoHandle;

typedef struct QDEnvironment {
	GrafPtr envPort;
	PenState envPen;
	WETextStyle envStyle;
	short envMode;
} QDEnvironment;

typedef struct RunArrayElement {
	long runStart;				// offset to first character in style run
	long styleIndex;			// index into style table
} RunArrayElement, *RunArrayElementPtr, **RunArrayElementHandle;

typedef RunArrayElementPtr RunArrayPtr;
typedef RunArrayElementHandle RunArrayHandle;

typedef struct TERunAttributes {
	short runHeight;
	short runAscent;
	TextStyle runTEStyle;
} TERunAttributes, *TERunAttributesPtr;

typedef struct TEStyleScrapElement {
	long scrpStartChar;
	TERunAttributes scrpTEAttrs;
} TEStyleScrapElement, *TEStyleScrapElementPtr;

typedef struct TEStyleScrap {
	short scrpNStyles;
	TEStyleScrapElement scrpStyleTab[1];
} TEStyleScrap, *TEStyleScrapPtr, **TEStyleScrapHandle;

typedef struct StyleTableElement {
	long refCount;				// reference count
	WERunAttributes info;		// style information
} StyleTableElement, *StyleTableElementPtr, **StyleTableElementHandle;

typedef StyleTableElementPtr StyleTablePtr;
typedef StyleTableElementHandle StyleTableHandle;

typedef struct LineRec {
	long lineStart;				// byte offset to first character in line
	long lineOrigin;			// pixel offset from destRect.top
	short lineAscent;			// maximum font ascent for this line
	short lineSlop;				// extra pixels needed to fill up the line
	Fixed lineJustAmount;		// normalized slop value, used for justification
} LineRec, *LinePtr, **LineHandle;

typedef LinePtr LineArrayPtr;
typedef LineHandle LineArrayHandle;

typedef struct WEAction {
	WEHandle hOwner;			// handle to associated WE instance
	struct WEAction **hNext;	// used to keep a linked list of actions
	Handle hText;				// handle to saved text
	Handle hStyles;				// handle to saved styles
	Handle hSoup;				// handle to saved soup
	long delRangeStart;			// start of range to delete
	long delRangeLength;		// length of range to delete
	long insRangeLength;		// lenfth of range to insert
	long hiliteStart;			// start of range to hilite
	long hiliteEnd;				// end of range to hilite
	WEActionKind actionKind;	// identifies event that caused this action to be pushed
	WEActionFlags actionFlags;	// miscellaneous flags
} WEAction, *WEActionPtr, **WEActionHandle;

// A WESoup record is a static description of an object embedded in the text.
// The 'SOUP' data type is just a collection of WESoup records, each followed
// by the corresponding object data.
// This data type complements the standard TEXT/styl pair.

typedef struct WESoup {
	long soupOffset;			// insertion offset for this object
	FlavorType soupType;		// 4-letter tag identifying object type
	long soupReserved1;			// reserved for future use; set to zero
	Size soupDataSize;			// size of object data following this record
	Point soupSize;				// object height and width, in pixels
	long soupReserved2;			// reserved for future use; set to zero
} WESoup, *WESoupPtr, **WESoupHandle;

// callback prototypes

typedef pascal Boolean (*WEClickLoopProcPtr)(WEHandle hWE);
typedef pascal void (*WEScrollProcPtr)(WEHandle hWE);
typedef pascal void (*WETSMPreUpdateProcPtr)(WEHandle hWE);
typedef pascal void (*WETSMPostUpdateProcPtr)(WEHandle hWE,
		long fixLength, long inputAreaStart, long inputAreaEnd,
		long pinRangeStart, long pinRangeEnd);
typedef pascal OSErr (*WETranslateDragProcPtr)(DragReference theDrag,
		ItemReference theItem, FlavorType requestedType, Handle putDataHere);
typedef pascal void (*WEDrawTextProcPtr)(Ptr pText, long textLength, Fixed slop,
		JustStyleCode styleRunPosition, WEHandle hWE);
typedef pascal long (*WEPixelToCharProcPtr)(Ptr pText, long textLength, Fixed slop,
		Fixed *pixelWidth, char *edge, JustStyleCode styleRunPosition, Fixed hPos, WEHandle hWE);
typedef pascal short (*WECharToPixelProcPtr)(Ptr pText, long textLength, Fixed slop,
		long offset, short direction, JustStyleCode styleRunPosition, long hPos, WEHandle hWE);
typedef pascal StyledLineBreakCode (*WELineBreakProcPtr)(Ptr pText, long textLength,
		long textStart, long textEnd, Fixed *textWidth, long *textOffset, WEHandle hWE);
typedef pascal void (*WEWordBreakProcPtr)(Ptr pText, short textLength, short offset,
		char edge, OffsetTable breakOffsets, ScriptCode script, WEHandle hWE);
typedef pascal short (*WECharByteProcPtr)(Ptr pText, short textOffset, ScriptCode script,
		WEHandle hWE);
typedef pascal short (*WECharTypeProcPtr)(Ptr pText, short textOffset, ScriptCode script, WEHandle hWE);

typedef pascal OSErr (*WENewObjectProcPtr)(Point *defaultObjectSize,
		WEObjectDescHandle hObjectDesc);
typedef pascal OSErr (*WEDisposeObjectProcPtr)(WEObjectDescHandle hObjectDesc);
typedef pascal OSErr (*WEDrawObjectProcPtr)(const Rect *destRect,
		WEObjectDescHandle hObjectDesc);
typedef pascal Boolean (*WEClickObjectProcPtr)(Point hitPt, EventModifiers modifiers, unsigned long clickTime,
		WEObjectDescHandle hObjectDesc);
typedef pascal OSErr (*WEStreamObjectProcPtr)(short destKind, FlavorType *theType, Handle putDataHere, WEObjectDescHandle hObjectDesc);


// UPP proc info

enum {
	uppWEClickLoopProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWEScrollProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWETSMPreUpdateProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWETSMPostUpdateProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
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
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
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
		| STACK_ROUTINE_PARAMETER(8,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
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
		| STACK_ROUTINE_PARAMETER(8,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
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
		| STACK_ROUTINE_PARAMETER(7,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWEWordBreakProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(short /*textLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(short /*offset*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(char /*edge*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(OffsetTable * /*breakOffsets*/)))
		| STACK_ROUTINE_PARAMETER(6,SIZE_CODE(sizeof(ScriptCode /*script*/)))
		| STACK_ROUTINE_PARAMETER(7,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWECharByteProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(short )))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(short /*textOffset*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(ScriptCode /*script*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWECharTypeProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(short )))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(short /*textOffset*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(ScriptCode /*script*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};


enum {
	uppWENewObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Point * /*defaultObjectSize*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(WEObjectDescHandle /*hObjectDesc*/)))
};
enum {
	uppWEDisposeObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEObjectDescHandle /*hObjectDesc*/)))
};
enum {
	uppWEDrawObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(const Rect * /*destRect*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(WEObjectDescHandle /*hObjectDesc*/)))
};
enum {
	uppWEClickObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Point /*hitPt*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(short /*modifiers*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(unsigned long /*clickTime*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(WEObjectDescHandle /*hObjectDesc*/)))
};
enum {
	uppWEStreamObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short /*destKind*/)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(FlavorType * /*theType*/)))
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Handle /*putDataHere*/)))
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(WEObjectDescHandle /*hObjectDesc*/)))
};


/*	UPPs, NewÅProc macros & CallÅProc macros */

/*
	NOTE:
    For compatibility with the Pascal version, CallÅProc macros take the form:

		CallFooProc(..., userRoutine)

	instead of:

		CallFooProc(userRoutine, ...)

*/

#if WASTE_USE_UPPS

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

#define CallWEClickLoopProc(hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWEClickLoopProcInfo, (hWE))
#define CallWEScrollProc(hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWEScrollProcInfo, (hWE))
#define CallWETSMPreUpdateProc(hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWETSMPreUpdateProcInfo, (hWE))
#define CallWETSMPostUpdateProc(hWE, fixLength, inputAreaStart, inputAreaEnd, pinRangeStart, pinRangeEnd, userRoutine) \
	CallUniversalProc((userRoutine), uppWETSMPostUpdateProcInfo, (hWE), (fixLength), (inputAreaStart), (inputAreaEnd), (pinRangeStart), (pinRangeEnd))
#define CallWETranslateDragProc(theDrag, theItem, requestedType, putDataHere, userRoutine) \
	CallUniversalProc((userRoutine), uppWETranslateDragProcInfo, (theDrag), (theItem), (requestedType), (putDataHere))
#define CallWEDrawTextProc(pText, textLength, slop, styleRunPosition, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWEDrawTextProcInfo, (pText), (textLength), (slop), (styleRunPosition), (hWE))
#define CallWEPixelToCharProc(pText, textLength, slop, pixelWidth, edge, styleRunPosition, hPos, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWEPixelToCharProcInfo, (pText), (textLength), (slop), (pixelWidth), (edge), (styleRunPosition), (hPos), (hWE))
#define CallWECharToPixelProc(pText, textLength, slop, offset, direction, styleRunPosition, hPos, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWECharToPixelProcInfo, (pText), (textLength), (slop), (offset), (direction), (styleRunPosition), (hPos), (hWE))
#define CallWELineBreakProc(pText, textLength, textStart, textEnd, textWidth, textOffset, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWELineBreakProcInfo, (pText), (textLength), (textStart), (textEnd), (textWidth), (textOffset), (hWE))
#define CallWEWordBreakProc(pText, textLength, offset, edge, breakOffsets, script, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWEWordBreakProcInfo, (pText), (textLength), (offset), (edge), (breakOffsets), (script), (hWE))
#define CallWECharByteProc(pText, textOffset, script, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWECharByteProcInfo, (pText), (textOffset), (script), (hWE))
#define CallWECharTypeProc(pText, textOffset, script, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWECharTypeProcInfo, (pText), (textOffset), (script), (hWE))
#define CallWENewObjectProc(defaultObjectSize, hObjectDesc, userRoutine) \
	CallUniversalProc((userRoutine), uppWENewObjectProcInfo, (defaultObjectSize), (hObjectDesc))
#define CallWEDisposeObjectProc(hObjectDesc, userRoutine) \
	CallUniversalProc((userRoutine), uppWEDisposeObjectProcInfo, (hObjectDesc))
#define CallWEDrawObjectProc(destRect, hObjectDesc, userRoutine) \
	CallUniversalProc((userRoutine), uppWEDrawObjectProcInfo, (destRect), (hObjectDesc))
#define CallWEClickObjectProc(hitPt, modifiers, clickTime, hObjectDesc, userRoutine) \
	CallUniversalProc((userRoutine), uppWEClickObjectProcInfo, (hitPt), (modifiers), (clickTime), (hObjectDesc))
#define CallWEStreamObjectProc(destKind, theType, putDataHere, hObjectDesc, userRoutine) \
	CallUniversalProc((userRoutine), uppWEStreamObjectProcInfo, (destKind), (theType), (putDataHere), (hObjectDesc))

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

#define CallWEClickLoopProc(hWE, userRoutine) \
	(*(userRoutine))((hWE))
#define CallWEScrollProc(hWE, userRoutine) \
	(*(userRoutine))((hWE))
#define CallWETSMPreUpdateProc(hWE, userRoutine) \
	(*(userRoutine))((hWE))
#define CallWETSMPostUpdateProc(hWE, fixLength, inputAreaStart, inputAreaEnd, pinRangeStart, pinRangeEnd, userRoutine) \
	(*(userRoutine))((hWE), (fixLength), (inputAreaStart), (inputAreaEnd), (pinRangeStart), (pinRangeEnd))
#define CallWETranslateDragProc(theDrag, theItem, requestedType, putDataHere, userRoutine) \
	(*(userRoutine))((theDrag), (theItem), (requestedType), (putDataHere))
#define CallWEDrawTextProc(pText, textLength, slop, styleRunPosition, hWE, userRoutine) \
	(*(userRoutine))((pText), (textLength), (slop), (styleRunPosition), (hWE))
#define CallWEPixelToCharProc(pText, textLength, slop, pixelWidth, edge, styleRunPosition, hPos, hWE, userRoutine) \
	(*(userRoutine))((pText), (textLength), (slop), (pixelWidth), (edge), (styleRunPosition), (hPos), (hWE))
#define CallWECharToPixelProc(pText, textLength, slop, offset, direction, styleRunPosition, hPos, hWE, userRoutine) \
	(*(userRoutine))((pText), (textLength), (slop), (offset), (direction), (styleRunPosition), (hPos), (hWE))
#define CallWELineBreakProc(pText, textLength, textStart, textEnd, textWidth, textOffset, hWE, userRoutine) \
	(*(userRoutine))((pText), (textLength), (textStart), (textEnd), (textWidth), (textOffset), (hWE))
#define CallWEWordBreakProc(pText, textLength, offset, edge, breakOffsets, script, hWE, userRoutine) \
	(*(userRoutine))((pText), (textLength), (offset), (edge), (breakOffsets), (script), (hWE))
#define CallWECharByteProc(pText, textOffset, script, hWE, userRoutine) \
	(*(userRoutine))((pText), (textOffset), (script), (hWE))
#define CallWECharTypeProc(pText, textOffset, script, hWE, userRoutine) \
	(*(userRoutine))((pText), (textOffset), (script), (hWE))
#define CallWENewObjectProc(defaultObjectSize, hObjectDesc, userRoutine) \
	(*(userRoutine))((defaultObjectSize), (hObjectDesc))
#define CallWEDisposeObjectProc(hObjectDesc, userRoutine) \
	(*(userRoutine))((hObjectDesc))
#define CallWEDrawObjectProc(destRect, hObjectDesc, userRoutine) \
	(*(userRoutine))((destRect), (hObjectDesc))
#define CallWEClickObjectProc(hitPt, modifiers, clickTime, hObjectDesc, userRoutine) \
	(*(userRoutine))((hitPt), (modifiers), (clickTime), (hObjectDesc))
#define CallWEStreamObjectProc(destKind, theType, putDataHere, hObjectDesc, userRoutine) \
	(*(userRoutine))((destKind), (theType), (putDataHere), (hObjectDesc))

#endif

typedef Boolean (*SegmentLoopProcPtr)
				(LinePtr pLine,
				 const WERunAttributes *pAttrs,
				 Ptr pSegment,
				 long segmentStart,
				 long segmentLength,
				 JustStyleCode styleRunPosition,
				 void *callbackData);

// typedef char FourCharCode[4];
typedef FourCharCode WESelector;

typedef struct WEFieldDescriptor {
	short fOffset;
	short fLength;
} WEFieldDescriptor;

typedef struct WELookupTable {
	WESelector selector;
	WEFieldDescriptor desc;
} WELookupTable;

typedef struct WEOHTableElement {
	FlavorType objectType;
	WENewObjectUPP newHandler;
	WEDisposeObjectUPP freeHandler;
	WEDrawObjectUPP drawHandler;
	WEClickObjectUPP clickHandler;
	WEStreamObjectUPP streamHandler;
} WEOHTableElement, *WEOHTablePtr, **WEOHTableHandle;

typedef struct WEObjectDesc {
	FlavorType objectType;				// 4-letter tag identifying object type
	Handle objectDataHandle;			// handle to object data
	Point objectSize;					// object height and width, in pixels
	WEOHTableHandle objectTable;		// handle to object handler table
	long objectIndex;					// index into object handler table
	WEHandle objectOwner;				// handle to owner WE instance
	long objectRefCon;					// free for use by object handlers
} WEObjectDesc;

typedef struct	WERec {
	GrafPtr port;							// graphics port text is drawn into
	Handle hText;							// handle to the text
	LineArrayHandle hLines;					// handle to the line array
	StyleTableHandle hStyles;				// handle to the style table
	RunArrayHandle hRuns;					// handle to the style run array
	long textLength;						// length of text
	long nLines;							// number of lines
	long nStyles;							// number of unique styles in the style table
	long nRuns;								// number of style runs
	LongRect viewRect;						// view rectangle, all drawing is clipped to this
	LongRect destRect;						// destination rectangle
	long selStart;							// start of selection range
	long selEnd;							// end of selection range
	unsigned long flags;					// 32 bits of miscellaneous flags (private)
	unsigned long features;					// 32 bit of feature flags (public)
	unsigned long caretTime;				// time of most recent caret drawing, in ticks
	unsigned long clickTime;				// time of most recent click, in ticks
	long clickLoc;							// byte offset of most recent click
	long anchorStart;						// start offset of anchor word/line
	long anchorEnd;							// end offset of anchor word/line
	char firstByte;							// first byte of a double-byte character
	char clickEdge;							// edge of character hit by most recent click
	unsigned char clickCount;				// multiple click count
	WEAlignment alignment;					// alignment style
	GrafPtr offscreenPort;					// offscreen graphics world
	RgnHandle viewRgn;						// handle to the view region
	void *refCon;							// reference value for client use
	TSMDocumentID tsmReference;				// reference value for the text services manager
	long tsmAreaStart;						// start of active input area (for TSM)
	long tsmAreaEnd;						// end of active input area
	DragReference currentDrag;				// reference of drag being tracked by _WEDrag
	long dragCaretOffset;					// offset to caret displayed during a drag
	WEActionHandle hActionStack;			// action stack for undo
	unsigned long modCount;					// modification count
	WEClickLoopUPP clickLoop;				// click loop callback
	WEScrollUPP scrollProc;					// scroll callback
	WETSMPreUpdateUPP tsmPreUpdate;			// TSM pre-update callback
	WETSMPostUpdateUPP tsmPostUpdate;		// TSM post-update callback
	WETranslateDragUPP translateDragHook;	// drag translation hook
	WEDrawTextUPP drawTextHook;				// hook for drawing text
	WEPixelToCharUPP pixelToCharHook;		// hook for hit-testing
	WECharToPixelUPP charToPixelHook;		// hook for locating glyph position
	WELineBreakUPP lineBreakHook;			// hook for finding line breaks
	WEWordBreakUPP wordBreakHook;			// hook for finding word breaks
	WECharByteUPP charByteHook;				// hook for finding character byte type
	WECharTypeUPP charTypeHook;				// hook for finding character type
	WERunAttributes nullStyle;				// style for null selection
#if WASTE_IC_SUPPORT
	StringHandle hURLHint;					// hint string for slack URLs
#endif
#if WASTE_OBJECTS
	WEOHTableHandle hObjectHandlerTable;	// handle to object handler table for this instance
#endif
} WERec;

struct SLDrawData {
	WEHandle hWE;
	Rect bounds;
	Rect lineRect;				// rect enclosing current line mf
	Rect drawRect;				// visible portion of line rect mf
	PixMapHandle offscreenPixels; // mf
	GrafPtr screenPort;			// DWC 5/18/95
	GDHandle screenDevice;
	Boolean usingColor;			// true if drawing in color
	Boolean usingOffscreen;		// true if an offscreen graphics world has been set up
	Boolean drawingOffscreen;	// true if actually drawing to an offscreen buffer
	Boolean doErase;
};

struct SLCalcSlopData {
	WEPtr pWE;
	short lineWidth;
	short totalSlop;
	Fixed totalProportion;
};

struct SLPixelToCharData {
	WEHandle hWE;
	Fixed hPos;
	Fixed pixelWidth;
	long offset;
	char *edge;
};

struct SLCharToPixelData {
	WEHandle hWE;
	long offset;
	LongPt *thePoint;
};


#if defined(powerc) || defined (__powerc)
#pragma options align=reset
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Routines in WEUtilities.c
pascal void _WEForgetHandle(Handle *h);
pascal Boolean _WESetHandleLock(Handle h, Boolean lock);
pascal void _WEBlockClr(void *block, long blockSize);
pascal Boolean _WEBlockCmp(const void *block1, const void *block2, long blockSize);
pascal void _WEReorder(long *a, long *b);
pascal OSErr _WEAllocate(long blockSize, short allocFlags, Handle *h);

// in WEArrays.c
pascal OSErr _WEInsertBlock(Handle h, const void *blockPtr, long blockSize, long offset);
pascal void _WERemoveBlock(Handle h, long blockSize, long offset);

// in WEBirthDeath.c
pascal OSErr _WERegisterWithTSM(WEHandle hWE);
pascal void _WESetStandardHooks(WEHandle hWE);
pascal OSErr WENew(const LongRect *destRect, const LongRect *viewRect, unsigned long features, WEHandle *hWE);
pascal void _WEResetStyleTable(WEHandle hWE);
pascal void WEDispose(WEHandle hWE);
pascal OSErr WEUseText(Handle text, WEHandle hWE);
pascal void _WEStdDrawText(Ptr pText, long textLength, Fixed slop,
				JustStyleCode styleRunPosition, WEHandle hWE);
pascal long _WEStdPixelToChar(Ptr pText, long textLength, Fixed slop,
				Fixed *width, char *edge, JustStyleCode styleRunPosition,
				Fixed hPos, WEHandle hWE);
pascal short _WEStdCharToPixel(Ptr pText, long textLength, Fixed slop,
				long offset, short direction, JustStyleCode styleRunPosition,
				long hPos, WEHandle hWE);
pascal StyledLineBreakCode _WEStdLineBreak(Ptr pText, long textLength,
				long textStart, long textEnd, Fixed *textWidth,
				long *textOffset, WEHandle hWE);
pascal void _WEStdWordBreak(Ptr pText, short textLength, short offset,
				char edge, OffsetTable breakOffsets, ScriptCode script,
				WEHandle hWE);
pascal short _WEStdCharByte(Ptr pText, short textLength, ScriptCode script,
				WEHandle hWE);
pascal short _WEStdCharType(Ptr pText, short textLength, ScriptCode script,
				WEHandle hWE);
pascal short _WEScriptToFont(ScriptCode script);
pascal void _WEOldWordBreak(Ptr pText, short textLength, short offset,
				char edge, OffsetTable breakOffsets, ScriptCode script,
				WEHandle hWE);
pascal short _WEOldCharByte(Ptr pText, short textLength, ScriptCode script,
				WEHandle hWE);
pascal short _WEOldCharType(Ptr pText, short textLength, ScriptCode script,
				WEHandle hWE);
pascal Boolean _WEStdClickLoop(WEHandle hWE);

// in WEDebug.c
pascal void _WESanityCheck(WEHandle hWE);

// in WEInlineInput.c
pascal OSErr _WEHiliteRangeArray(TextRangeArrayHandle hTray, WEHandle hWE);
pascal OSErr _WEHandleUpdateActiveInputArea(const AppleEvent *ae, AppleEvent *reply, long handlerRefCon);
pascal OSErr _WEHandlePositionToOffset(const AppleEvent *ae, AppleEvent *reply, long handlerRefCon);
pascal OSErr _WEHandleOffsetToPosition(const AppleEvent *ae, AppleEvent *reply, long handlerRefCon);
pascal OSErr WEInstallTSMHandlers(void);
pascal OSErr WERemoveTSMHandlers(void);

// in WELongCoords.c
pascal long _WEPinInRange(long value, long rangeStart, long rangeEnd);
pascal void WELongPointToPoint(const LongPt *lp, Point *p);
pascal void WEPointToLongPoint(Point p, LongPt *lp);
pascal void WESetLongRect(LongRect *lr, long left, long top, long right, long bottom);
pascal void WELongRectToRect(const LongRect *lr, Rect *r);
pascal void WERectToLongRect(const Rect *r, LongRect *lr);
pascal void WEOffsetLongRect(LongRect *lr, long hOffset, long vOffset);
pascal Boolean WELongPointInLongRect(const LongPt *lp, const LongRect *lr);

// in WEDrawing.c
pascal long WEOffsetToLine (long offset, WEHandle hWE);
pascal long _WEPixelToLine(long vOffset, WEHandle hWE);
pascal long _WEOffsetToRun (long offset, WEHandle hWE);
pascal void _WEGetIndStyle(long runIndex, WERunInfo *info, WEHandle hWE);
pascal void WEGetRunInfo(long offset, WERunInfo *info, WEHandle hWE);
pascal OSErr WEGetSelectedObject(WEObjectDescHandle *hObjectDesc, WEHandle hWE);
pascal long WEFindNextObject(long offset, WEObjectDescHandle *hObjectDesc, WEHandle hWE);
pascal void _WEContinuousStyleRange(long rangeStart, long rangeEnd,
		WEStyleMode *mode, WETextStyle *ts, WEHandle hWE);
pascal void _WESynchNullStyle(WEHandle hWE);
pascal Boolean WEContinuousStyle(WEStyleMode *mode, TextStyle *ts, WEHandle hWE);
pascal void _WESegmentLoop(long firstLine, long lastLine, SegmentLoopProcPtr callback, void *callbackData, WEHandle hWE);
pascal void _WEDrawTSMHilite(Rect *segmentRect, short tsFlags);
pascal void _WEDrawLines (long firstLine, long lastLine, Boolean doErase, WEHandle hWE);
pascal short _WECalcPenIndent(short slop, WEAlignment alignment);
pascal void _WESaveQDEnvironment(GrafPtr port, Boolean saveColor, QDEnvironment *theEnvironment);
pascal void _WERestoreQDEnvironment(const QDEnvironment *theEnvironment);
pascal void _WEFillFontInfo (GrafPtr port, WERunAttributes *targetStyle);
pascal void _WECopyStyle (const WETextStyle *sourceStyle, WETextStyle *targetStyle,
		Style offStyles, WEStyleMode mode);
pascal Boolean _WEOffsetInRange(long offset, char edge, long rangeStart, long rangeEnd);

// WELineLayout.c
pascal void WEStopInlineSession(WEHandle hWE);
pascal void _WERemoveLine(long lineIndex, WEPtr pWE);
pascal OSErr _WEInsertLine(long lineIndex, const LineRec *theLine, WEPtr pWE);
pascal void _WEBumpOrigin(long lineIndex, long deltaOrigin, WEPtr pWE);
pascal long _WEFindLineBreak(long lineStart, WEHandle hWE);
pascal void _WECalcHeights(long rangeStart, long rangeEnd, short *lineAscent, short *lineDescent,
		WEHandle hWE);
pascal OSErr _WERecalBreaks(long *startLine, long *endLine, WEHandle hWE);
pascal void _WERecalSlops(long firstLine, long lastLine, WEHandle hWE);
pascal OSErr WECalText(WEHandle hWE);
pascal WEAlignment WEGetAlignment(WEHandle hWE);
pascal void WEGetSelection(long *selStart, long *selEnd, WEHandle hWE);
pascal void WESetDestRect(const LongRect *destRect, WEHandle hWE);
pascal void WEGetDestRect(LongRect *destRect, WEHandle hWE);
pascal void WESetViewRect(const LongRect *viewRect, WEHandle hWE);
pascal void WEGetViewRect(LongRect *viewRect, WEHandle hWE);
pascal long WEGetTextLength(WEHandle hWE);
pascal long WECountLines(WEHandle hWE);
pascal long WEGetHeight(long startLine, long endLine, WEHandle hWE);
pascal void WEGetLineRange(long lineNo, long *lineStart, long *lineEnd, WEHandle hWE);
pascal Handle WEGetText(WEHandle hWE);
pascal short WEGetChar(long offset, WEHandle hWE);
pascal short WEFeatureFlag(short feature, short action, WEHandle hWE);

// WESelecting.c
pascal void _WEClearHiliteBit(void);
pascal long WEGetOffset(const LongPt *thePoint, char *edge, WEHandle hWE);
pascal void WEGetPoint(long offset, LongPt *thePoint, short *lineHeight, WEHandle hWE);
pascal void WEFindLine(long offset, char edge, long *lineStart, long *lineEnd, WEHandle hWE);
pascal short _WEGetContext(long offset, long *contextStart, long *contextEnd,
						WEHandle hWE);
pascal short _WEGetRestrictedContext(long offset, long *contextStart, long *contextEnd,
						WEHandle hWE);
pascal void WEFindWord(long offset, char edge, long *wordStart, long *wordEnd, WEHandle hWE);
pascal short WECharByte(long offset, WEHandle hWE);
pascal short WECharType(long offset, WEHandle hWE);
pascal void _WEDrawCaret(long offset, WEHandle hWE);
pascal void _WEBlinkCaret(WEHandle hWE);
pascal RgnHandle WEGetHiliteRgn(long rangeStart, long rangeEnd, WEHandle hWE);
pascal void _WEHiliteRange(long rangeStart, long rangeEnd, WEHandle hWE);
pascal void WESetSelection(long selStart, long selEnd, WEHandle hWE);
pascal void WESetAlignment(WEAlignment alignment, WEHandle hWE);
pascal long _WEArrowOffset(short action, long offset, WEHandle hWE);
pascal void _WEDoArrowKey (short, EventModifiers modifiers, WEHandle hWE);
pascal Boolean WEAdjustCursor(Point mouseLoc, RgnHandle mouseRgn, WEHandle hWE);
pascal void WEIdle(unsigned long *maxSleep, WEHandle hWE);
pascal void WEUpdate(RgnHandle updateRgn, WEHandle hWE);
pascal void WEDeactivate(WEHandle hWE);
pascal void WEActivate(WEHandle hWE);
pascal Boolean WEIsActive(WEHandle hWE);
pascal void WEScroll(long hOffset, long vOffset, WEHandle hWE);
pascal Boolean _WEScrollIntoView (long offset, WEHandle hWE);
pascal void WESelView(WEHandle hWE);

// WELowLevelEditing.c
pascal Boolean _WEIsWordRange(long rangeStart, long rangeEnd, WEHandle hWE);
pascal Boolean _WEIsPunct(long offset, WEHandle hWE);
pascal void _WEIntelligentCut(long *rangeStart, long *rangeEnd, WEHandle hWE);
pascal short _WEIntelligentPaste(long rangeStart, long rangeEnd, WEHandle hWE);
pascal OSErr _WEInsertRun(long runIndex, long offset, long styleIndex, WEPtr pWE);
pascal void _WERemoveRun(long runIndex, WEPtr pWE);
pascal void _WEChangeRun(long runIndex, long newStyleIndex, Boolean keepOld, WEPtr pWE);
pascal OSErr _WENewStyle(const WETextStyle *ts, long *styleIndex, WEPtr pWE);
pascal OSErr _WERedraw(long rangeStart, long rangeEnd, WEHandle hWE);
pascal OSErr _WESetStyleRange(long rangeStart, long rangeEnd, WEStyleMode mode,
				const WETextStyle *ts, WEHandle hWE);
pascal OSErr _WEApplyStyleScrap(long rangeStart, long rangeEnd, StScrpHandle styleScrap, WEHandle hWE);
pascal OSErr _WEApplySoup(long offset, Handle hSoup, WEHandle hWE);
pascal void _WEBumpRunStart(long runIndex, long deltaRunStart, WEPtr pWE);
pascal void _WERemoveRunRange(long rangeStart, long rangeEnd, WEHandle hWE);
pascal void _WEBumpLineStart(long lineIndex, long deltaLineStart, WEPtr pWE);
pascal void _WERemoveLineRange(long rangeStart, long rangeEnd, WEHandle hWE);
pascal OSErr _WEDeleteRange(long rangeStart, long rangeEnd, WEHandle hWE);
pascal OSErr _WEInsertText(long offset, Ptr textPtr, long textLength, WEHandle hWE);

// WEHighLevelEditing.c
pascal void _WEPushAction(WEActionHandle hAction);
pascal OSErr _WENewAction(long rangeStart, long rangeEnd, long newTextLength,
							WEActionKind actionKind, WEActionFlags actionFlags,
							WEHandle hWE, WEActionHandle *hAction);
pascal void _WEDisposeAction(WEActionHandle hAction);
pascal void _WEForgetAction(WEActionHandle *hAction);
pascal OSErr _WEDoAction(WEActionHandle hAction);
pascal OSErr WEUndo(WEHandle hWE);
pascal void WEClearUndo(WEHandle hWE);
pascal WEActionKind WEGetUndoInfo(Boolean *redoFlag, WEHandle hWE);
pascal unsigned long WEGetModCount(WEHandle hWE);
pascal void WEResetModCount(WEHandle hWE);
pascal void _WEAdjustUndoRange(long moreBytes, WEHandle hWE);
pascal OSErr _WETypeChar(char theByte, WEHandle hWE);
pascal OSErr _WEBackspace(WEHandle hWE);
pascal OSErr _WEForwardDelete(WEHandle hWE);
pascal Boolean WEIsTyping(WEHandle hWE);
pascal void WEKey(short key, EventModifiers modifiers, WEHandle hWE);
pascal OSErr WEInsert(Ptr textPtr, long textLength, StScrpHandle hStyles, Handle hSoup, WEHandle hWE);
pascal OSErr WEInsertObject(FlavorType objectType, Handle objectDataHandle, Point objectSize, WEHandle hWE);
pascal OSErr WEDelete(WEHandle hWE);
pascal OSErr WECut(WEHandle hWE);
pascal Boolean WECanPaste(WEHandle hWE);
pascal OSErr WEPaste(WEHandle hWE);
pascal OSErr _WESmartSetFont(WEStyleMode mode, const TextStyle *ts, WEHandle hWE);
pascal OSErr WESetStyle(WEStyleMode mode, const TextStyle *ts, WEHandle hWE);
pascal OSErr WEUseStyleScrap(StScrpHandle hStyles, WEHandle hWE);
pascal OSErr WEUseSoup(Handle hSoup, WEHandle hWE);

// WEMouse.c
pascal Boolean _WEIsOptionDrag(DragReference theDrag);
pascal OSErr _WEGetFlavor(DragReference theDrag, ItemReference theItem,
				FlavorType requestedType, Handle hFlavor,
				WETranslateDragUPP translateDragHook);
pascal OSErr _WEExtractFlavor(DragReference theDrag, ItemReference theItem,
						FlavorType theType, Handle *hFlavor,
						WETranslateDragUPP translateDragHook);
pascal Boolean WECanAcceptDrag(DragReference theDrag, WEHandle hWE);
pascal void _WEUpdateDragCaret(long offset, WEHandle hWE);
pascal OSErr WETrackDrag(DragTrackingMessage theMessage,DragReference theDrag,
									WEHandle hWE);
pascal OSErr WEReceiveDrag(DragReference theDrag, WEHandle hWE);
pascal OSErr _WESendFlavor(FlavorType requestedType, void *dragSendRefCon,
									unsigned long hWE, DragReference theDrag); // hWE was type WEHandle - 050107 akozar
pascal Boolean WEDraggedToTrash(DragReference theDrag);
pascal OSErr _WEDrag(Point mouseLoc, EventModifiers modifiers, unsigned long clickTime, WEHandle hWE);
pascal void _WEResolveURL(EventModifiers modifiers, long urlStart, long urlEnd, WEHandle hWE);
pascal void WEClick(Point mouseLoc, EventModifiers modifiers, unsigned long clickTime, WEHandle hWE);

// WEObjects.c
pascal FlavorType WEGetObjectType(WEObjectDescHandle hObjectDesc);
pascal Handle WEGetObjectDataHandle(WEObjectDescHandle hObjectDesc);
pascal Point WEGetObjectSize(WEObjectDescHandle hObjectDesc);
pascal WEHandle WEGetObjectOwner(WEObjectDescHandle hObjectDesc);
pascal long WEGetObjectRefCon(WEObjectDescHandle hObjectDesc);
pascal void WESetObjectRefCon(WEObjectDescHandle hObjectDesc, long refCon);
pascal long _WELookupObjectType(FlavorType objectType, WEOHTableHandle hTable);
pascal OSErr _WEGetIndObjectType(long index, FlavorType *objectType, WEHandle hWE);
pascal OSErr _WENewObject(FlavorType objectType, Handle objectDataHandle, WEHandle hWE,
					WEObjectDescHandle *hObjectDesc);
pascal OSErr _WEFreeObject(WEObjectDescHandle hObjectDesc);
pascal OSErr _WEDrawObject(WEObjectDescHandle hObjectDesc);
pascal Boolean _WEClickObject(Point hitPt, EventModifiers modifiers, unsigned long clickTime,
								WEObjectDescHandle hObjectDesc);
pascal OSErr _WEStreamObject(short destKind, FlavorType *theType, Handle *theData,
				Boolean *canDisposeData, WEObjectDescHandle hObjectDesc);
pascal OSErr WEInstallObjectHandler(FlavorType objectType, WESelector handlerSelector, UniversalProcPtr handler,
				WEHandle hWE);

// WEScraps.c
pascal OSErr _WEPrependStyle(Handle hStyleScrap, const WERunInfo *info, long offsetDelta);
pascal OSErr _WEAppendStyle(Handle hStyleScrap, const WERunInfo *info, long offset);
pascal OSErr _WEPrependObject(Handle hSoup, const WERunInfo *info, long offsetDelta);
pascal OSErr _WEAppendObject(Handle hSoup, const WERunInfo *info, long offset);
pascal OSErr WECopyRange(long rangeStart, long rangeEnd, Handle hText, Handle
					hStyles, Handle hSoup, WEHandle hWE);
pascal OSErr WECopy(WEHandle hWE);


// WESelectors.c
pascal void _WELookupSelector(const WELookupTable *table, WESelector selector, WEFieldDescriptor *desc);
pascal OSErr _WEGetField(const WELookupTable *table, WESelector selector, long *info, void *structure);
pascal OSErr _WESetField(const WELookupTable *table, WESelector selector, long *info, void *structure);
pascal OSErr WEGetInfo(WESelector selector, void *info, WEHandle hWE);
pascal OSErr WESetInfo(WESelector selector, const void *info, WEHandle hWE);
pascal OSErr _WESetHandler(WESelector selector, long *info, void *structure);

#ifdef __cplusplus
}
#endif
