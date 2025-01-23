/*  This file is a part of Bol Processor
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

/* This file contains type definitions from Mac OS that are used by BP3 */

typedef	int8_t	sbyte;

typedef	uint8_t		Str255[256], Str63[64], Str31[32];
typedef uint8_t*	StringPtr;

typedef int16_t		OSErr;
typedef int32_t		OSStatus;
typedef uint32_t	OSType, AEEventClass, AEEventID;	// must be a 32-bit unsigned int
typedef OSType		SFTypeList[4];
typedef int16_t		ResID;
typedef int16_t		ScriptCode;
typedef uint16_t	EventModifiers;

typedef int16_t		MenuID;
typedef uint16_t	MenuItemIndex;




/* Missing macros and constants from Mac OS --
   probably should replace/eliminate most of these.
 */
/*
#define	pascal

// enum { noErr = 0, eofErr, dskFulErr };
enum { eofErr, dskFulErr };

enum { fsCurPerm, fsRdPerm, fsFromStart };

// enum { normal, bold, kFontIDCourier };
enum { kFontIDCourier };

enum { mDownMask, mUpMask }; */

#endif /* BP_MISSINGMACTYPES_H */
