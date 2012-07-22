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
	WASTE Demo Project:
	Sample WASTE Object Handlers
	
	Copyright © 1993-1996 Marco Piovanelli
	All Rights Reserved
*/

#include "WEObjectHandlers.h"

#ifndef __ICONS__
#include <Icons.h>
#endif

#ifndef __SOUND__
#include <Sound.h>
#endif

/* PICTURES */

pascal OSErr HandleNewPicture(Point *defaultObjectSize, WEObjectReference objectRef)
{
	PicHandle thePicture;
	Rect frame;

	/* get handle to object data (in this case, a picture handle) */
	thePicture = (PicHandle) WEGetObjectDataHandle(objectRef);
	
	/* figure out the default object size by looking at the picFrame record */
	QDGetPictureBounds(thePicture, &frame);	// requires 10.3 or later - akozar 20120603
	OffsetRect(&frame, -frame.left, -frame.top);
	defaultObjectSize->v = frame.bottom;
	defaultObjectSize->h = frame.right;
	
	return noErr;
}

pascal OSErr HandleDisposePicture(WEObjectReference objectRef)
{
	PicHandle thePicture;

	/* get handle to object data (in this case, a picture handle) */
	thePicture = (PicHandle) WEGetObjectDataHandle(objectRef);
	
	/* kill the picture */
	KillPicture(thePicture);
	
	return MemError();
}

pascal OSErr HandleDrawPicture(const Rect *destRect, WEObjectReference objectRef)
{
	PicHandle thePicture;

	/* get handle to object data (in this case, a picture handle) */
	thePicture = (PicHandle) WEGetObjectDataHandle(objectRef);

	/* draw the picture */
	DrawPicture(thePicture, destRect);
	
	return noErr;
}


/* SOUND */

pascal OSErr HandleNewSound(Point *defaultObjectSize, WEObjectReference objectRef)
{
#pragma unused(objectRef)

	/* sounds are drawn as standard 32x32 icons */
	defaultObjectSize->v = 32;
	defaultObjectSize->h = 32;
	
	return noErr;
}

pascal OSErr HandleDrawSound(const Rect *destRect, WEObjectReference objectRef)
{
#pragma unused(objectRef)

	/* draw the sound icon */
	return PlotIconID(destRect, kAlignNone, kTransformNone, kSoundIconID);
}

pascal Boolean HandleClickSound(Point hitPt, EventModifiers modifiers,
					unsigned long clickTime, WEObjectReference objectRef)
{
#pragma unused(hitPt, clickTime)

	SndListHandle theSound;

	/* WASTE sets the low bit of modifiers on double (multiple) clicks */
	if (modifiers & 0x0001)
	{
	
		/* get a handle to the object data (in this case, a sound handle) */
		theSound = (SndListHandle) WEGetObjectDataHandle(objectRef);
		
		/* play the sound */
		SndPlay(NULL, theSound, false);
	
		/* return TRUE so WASTE knows we handled the click */
		return true;
	}
	else
	{
		/* not a double click: let WASTE handle the mouse-down */
		return false;
	}
}
