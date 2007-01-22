/*	CarbonCompatUtil.c

	Utility functions for maintaining code that is compatible with
	both Carbon and "Classic" Mac APIs.
	
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

OSErr	CCUZeroScrap()
{
#if !TARGET_API_MAC_CARBON
	return ZeroScrap();
#else
	return ClearCurrentScrap();
#endif
}

long	CCUGetScrap(Handle destination, ScrapFlavorType flavor, SInt32* offset)
{
#if !TARGET_API_MAC_CARBON
	return GetScrap(destination, flavor, offset);
#else
	/* 	check that Handle is not NULL ?
		Call	GetCurrentScrap(&theScrap)
			GetScrapFlavorFlags() to see if type available,
			GetScrapFlavorSize() to find out the size,
			allocate a Handle ? large enough to hold data,
			HLock(buffer),
			GetScrapFlavorData(theScrap, flavor, size, *buffer);
	 */
	return noErr;
#endif 
}

OSErr	CCUPutScrap(SInt32 size, ScrapFlavorType flavor, const void* data)
{
#if !TARGET_API_MAC_CARBON
	return PutScrap(size, flavor, data);
#else
	return noErr;
#endif
}
