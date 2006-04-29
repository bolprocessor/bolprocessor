/*
 *	LongCoords.h
 *
 *	C/C++ interface to the WASTE text engine:
 *	Long Coordinates
 *
 *	version 1.0
 *
 *	Copyright (c) 1993-1994 Marco Piovanelli
 *	All Rights Reserved
 * 
 */

#ifndef __LONGCOORDINATES__ 
#ifndef _LongCoords_
#define _LongCoords_

typedef struct LongPt {
	long v;
	long h;
} LongPt;

typedef struct LongRect {
	long top;
	long left;
	long bottom;
	long right;
} LongRect;

#endif
#endif
