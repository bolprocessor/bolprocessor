/*	CarbonCompatUtil.h    */

#ifndef ANTHONY_CARBON_COMPAT_UTIL_H
#define ANTHONY_CARBON_COMPAT_UTIL_H

/* [Carbon] Scrap Manager replacements */
OSErr	CCUZeroScrap();
long	CCUGetScrap(Handle destination, ScrapFlavorType flavor, SInt32* offset);
OSErr	CCUPutScrap(SInt32 size, ScrapFlavorType flavor, const void* data);


#endif
