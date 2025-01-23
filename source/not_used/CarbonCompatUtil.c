/*	CarbonCompatUtil.c

	Utility functions for maintaining code that is compatible with
	both Carbon and InterfaceLib APIs.
	
	Anthony Kozar
	January 22, 2007
 */


#include "CarbonCompatUtil.h"

/*	CCUZeroScrap()

	This is a Carbon-compliant replacement for ZeroScrap().
 */
OSErr	CCUZeroScrap()
{
#if !TARGET_API_MAC_CARBON_FORGET_THIS
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
#if !TARGET_API_MAC_CARBON_FORGET_THIS
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
#if !TARGET_API_MAC_CARBON_FORGET_THIS
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


/*	CCUTEToScrap()

	Handles clearing the desk scrap and transfering the private
	TextEdit scrap to the desk scrap.  Works around an (undocumented?)
	change in behavior on OS X.  Substitute your own global for 
	RunningOnOSX if reusing this code.
 */
void	CCUTEToScrap()
{
	OSErr err;
	extern Boolean RunningOnOSX;
	
	if (!RunningOnOSX) {
		err = CCUZeroScrap();
		TEToScrap();
	}
	// calling TEToScrap() on OS X does not work 
	// and seems to be unnecessary
	
	return;
}

/*	CheckScrapContents()

	This is a debugging function.  The contents of the global scrap
	are retrieved but nothing is done with them.  You can call this
	function in your debug build and set a breakpoint here ...
 */
void	CheckScrapContents()
{
	char** contents;
	SInt32 offset;
	long size;
	
	contents = NULL;
	/* get the size first */
	size = CCUGetScrap(NULL, 'TEXT', &offset);
	if (size > 0) {
		contents = NewHandle(size);
		if (MemError() == noErr)
			size = CCUGetScrap(contents, 'TEXT', &offset);
	}
	// set breakpoint on the next line
	if (contents)  DisposeHandle(contents);

	return;
}
