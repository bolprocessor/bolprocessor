/* MissingMacTypes.h (BP2 version CVS) */

/*  This file is a part of Bol Processor 2
Copyright (c) 2013, 2019 by Anthony Kozar
All rights reserved. 

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer. 

Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution. 

Neither the names of the Bol Processor authors nor the names of project
contributors may be used to endorse or promote products derived from this
software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef BP_MISSINGMACTYPES_H
#define BP_MISSINGMACTYPES_H

/* This file contains type definitions from Mac OS that are used by BP2 */

#ifndef __MACTYPES__
typedef uint8_t		UInt8;
typedef int8_t		SInt8;
typedef uint16_t	UInt16;
typedef int16_t		SInt16;
typedef uint32_t	UInt32;
typedef int32_t		SInt32;
#endif

#ifndef __MACTYPES__
typedef long (*ProcPtr)();
#endif
typedef ProcPtr	ControlActionUPP, ModalFilterUPP;

typedef	int8_t	sbyte;

#ifndef __MACTYPES__
typedef sbyte*	Ptr;
typedef sbyte**	Handle;
typedef	size_t	Size;

// these are unsigned on Mac OS, but might be better replaced as "plain" char ??
typedef	uint8_t		Str255[256], Str63[64], Str31[32];
typedef uint8_t*	StringPtr;

typedef uint8_t		Boolean;
typedef int16_t		OSErr;
typedef int32_t		OSStatus;
typedef uint32_t	OSType;			// must be a 32-bit unsigned int
#endif

typedef OSType		AEEventClass, AEEventID;
typedef OSType		SFTypeList[4];
typedef int16_t		ResID;
typedef int16_t		ScriptCode;
typedef uint16_t	EventModifiers;

typedef int16_t		MenuID;
typedef uint16_t	MenuItemIndex;

typedef Handle	NavMenuItemSpecArrayHandle;
typedef Handle	PicHandle, PolyHandle, RgnHandle;
typedef Handle	MenuHandle;
typedef Handle	ControlHandle;

#ifndef __MACTYPES__
// FSSpec is not defined here to be compatible with Mac OS
// I am assuming that this struct will be replaced in BP3 at some point ...
typedef struct {
	int		vRefNum;
	int		parID;
	Str255	name;
} FSSpec, *FSSpecPtr;
#endif

typedef struct {
} NavReplyRecord;

typedef struct {
} NSWReply;

typedef struct {
} WindowRecord, *WindowPtr;

typedef struct {
} Dialog, *DialogPtr, *DialogRef;

typedef struct {
} EventRecord, *EventPtr;

#ifndef __MACTYPES__
typedef struct {
} AppleEvent;
#endif

typedef struct {
} QDGlobals, *GrafPtr, *CGrafPtr, *GWorldPtr, **GDHandle;

typedef struct {
} Cursor;

#ifndef __MACTYPES__
typedef struct {
} Point;

// fields are larger than in Mac OS (32-bit instead of 16-bit)
typedef struct Rect {
	int32_t top;
	int32_t left;
	int32_t bottom;
	int32_t right;
} Rect;
#endif

typedef struct RGBColor {
	uint16_t red;
	uint16_t green;
	uint16_t blue;
} RGBColor;

typedef struct {
} TextStyle;

#ifndef __MACTYPES__
typedef struct {
} ProcessSerialNumber;
#endif


/* Missing macros and constants from Mac OS --
   probably should replace/eliminate most of these.
 */
#define	pascal

#ifndef __MACTYPES__
enum { noErr = 0, eofErr, dskFulErr };

enum { fsCurPerm, fsRdPerm, fsFromStart };

enum { normal, bold, kFontIDCourier };

enum { mDownMask, mUpMask };
#endif

#endif /* BP_MISSINGMACTYPES_H */
