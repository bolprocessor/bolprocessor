/*	CarbonCompatUtil.c

	Utility functions for maintaining code that is compatible with
	both Carbon and InterfaceLib APIs.
	
	Anthony Kozar
	January 22, 2007
 */

/* This is defined by both Carbon and non-Carbon prefix headers */
#if  !defined(TARGET_API_MAC_CARBON)
   /* so if it is not defined yet, there is no prefix file, 
      and we are compiling the "Transitional" build. */
   /* Use MacHeaders.h until ready to convert this file.
      Then change to MacHeadersTransitional.h. */
// #  include	"MacHeaders.h"
#  include	"MacHeadersTransitional.h"
#endif

#include "CarbonCompatUtil.h"

/*	CCUZeroScrap()

	This is a Carbon-compliant replacement for ZeroScrap().
 */
OSErr	CCUZeroScrap()
{
#if !TARGET_API_MAC_CARBON
	return ZeroScrap();
#else
	return ClearCurrentScrap();
#endif
}

/*	CCUGetScrap()

	This is a Carbon-compliant replacement for GetScrap().
	Note that the offset return value is meaningless when this function
	is compiled with Carbon.  It may not be meaningful with InterfaceLib
	either if the Translation Manager is active.  So, don't use it :)
	Also note, that your handle will not be unlocked before resizing it,
	so if you pass a locked handle, this function may fail.
 */
long	CCUGetScrap(Handle destination, ScrapFlavorType flavor, SInt32* offset)
{
#if !TARGET_API_MAC_CARBON
	return GetScrap(destination, flavor, offset);
#else
	OSStatus	err;
	ScrapRef	theScrap;
	Size		flavorSize;
	char		hflags;
	
	*offset = 0;
	err = GetCurrentScrap(&theScrap);
	if (err != noErr) return err;
	err = GetScrapFlavorSize(theScrap, flavor, &flavorSize);
	if (err != noErr || flavorSize < 1) return err;
	
	/* If passed a null destination handle, just return the size */
	if (destination == NULL) return flavorSize;
	else {
		hflags = HGetState(destination);		// save state
		SetHandleSize(destination, flavorSize);
		if ((err = MemError()) != noErr) return err;
		HLock(destination);
		err = GetScrapFlavorData(theScrap, flavor, &flavorSize, *destination);
		if (err != noErr) return err;
		HSetState(destination, hflags);
		return flavorSize;
	}
#endif 
}

/*	CCUPutScrap()

	This is a Carbon-compliant replacement for PutScrap().
	If your data is in a handle, you should probably lock it before
	calling this function.
 */
OSErr	CCUPutScrap(SInt32 size, ScrapFlavorType flavor, const void* data)
{
#if !TARGET_API_MAC_CARBON
	return PutScrap(size, flavor, data);
#else
	OSStatus	err;
	ScrapRef	theScrap;
	
	err = GetCurrentScrap(&theScrap);
	if (err != noErr) return err;
	err = PutScrapFlavor(theScrap, flavor, kScrapFlavorMaskNone, (Size)size, data);
	return err;
#endif
}
