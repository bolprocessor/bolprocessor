/* MissingMacTypes.h (BP2 version CVS) */

/*  This file is a part of Bol Processor 2
Copyright (c) 2013 by Anthony Kozar
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

/* This file contains type definitions from Mac OS that are used by BP2 */

// FIXME: these integer types should be defined using Std. C header types
typedef unsigned char	UInt8;
typedef signed char		SInt8;
typedef unsigned short	UInt16;
typedef signed short	SInt16;
typedef unsigned int	UInt32;
typedef signed int		SInt32;

typedef long (*ProcPtr)();
typedef ProcPtr	ControlActionUPP, ModalFilterUPP;

typedef char*	Ptr;
typedef char**	Handle;
typedef	size_t	Size;

// these are unsigned on Mac OS, but might be better replaced as "plain" char ??
typedef	unsigned char	Str255[256], Str63[64], Str31[32];
typedef unsigned char*	StringPtr;

typedef UInt8	Boolean;
typedef SInt16	OSErr;
typedef SInt32	OSStatus;
typedef UInt32	OSType, AEEventClass, AEEventID;	// must be a 32-bit unsigned int
typedef OSType	SFTypeList[4];
typedef SInt16	ResID;
typedef SInt16	ScriptCode;
typedef UInt16	EventModifiers;

typedef SInt16	MenuID;
typedef UInt16	MenuItemIndex;

typedef Handle	NavMenuItemSpecArrayHandle;
typedef Handle	TEHandle;
typedef Handle	PicHandle, PolyHandle, RgnHandle;
typedef Handle	MenuHandle;
typedef Handle	ControlHandle;

typedef struct {
} FSSpec, *FSSpecPtr;

typedef struct {
} NavReplyRecord;

typedef struct {
} WindowRecord, *WindowPtr;

typedef struct {
} Dialog, *DialogPtr, *DialogRef;

typedef struct {
} EventRecord, *EventPtr;

typedef struct {
} AppleEvent;

typedef struct {
} QDGlobals, Rect, Point, RGBColor, *GrafPtr, *CGrafPtr, *GWorldPtr, **GDHandle, Cursor;

typedef struct {
} TextStyle;

typedef struct {
} ProcessSerialNumber;


/* Missing macros and constants from Mac OS */
#define	pascal

enum { noErr = 0, eofErr, dskFulErr };

