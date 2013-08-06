/*
 *	WETabs.c
 *
 *	Routines for installing and removing tab hooks
 *
 */

#include "WETabs.h"

/* prototypes for the hook routines defined in WETabHooks.c or WETabHooks32.c */

#ifdef __cplusplus
extern "C" {
#endif

extern pascal void _WETabDrawText(Ptr, long, Fixed, JustStyleCode, WEReference);
extern pascal long _WETabPixelToChar(Ptr, long, Fixed,
					Fixed *, char *, JustStyleCode, Fixed, WEReference);
extern pascal short _WETabCharToPixel(Ptr, long, Fixed, long, short,
					JustStyleCode, long, WEReference);
extern pascal StyledLineBreakCode _WETabLineBreak(Ptr, long, long, long,
					Fixed *, long *, WEReference);

#ifdef __cplusplus
}
#endif

/* static UPP's */
static WEDrawTextUPP		_weTabDrawTextProc = NULL;
static WEPixelToCharUPP		_weTabPixelToCharProc = NULL;
static WECharToPixelUPP		_weTabCharToPixelProc = NULL;
static WELineBreakUPP		_weTabLineBreakProc = NULL;

pascal OSErr WEInstallTabHooks(WEReference we)
{
	OSErr err;

	/* if first time, create routine descriptors */
	if (_weTabDrawTextProc == NULL)
	{
		_weTabDrawTextProc = NewWEDrawTextProc(_WETabDrawText);
		_weTabPixelToCharProc = NewWEPixelToCharProc(_WETabPixelToChar);
		_weTabCharToPixelProc = NewWECharToPixelProc(_WETabCharToPixel);
		_weTabLineBreakProc = NewWELineBreakProc(_WETabLineBreak);
	}
	
	if ((err = WESetInfo( weDrawTextHook, &_weTabDrawTextProc, we )) != noErr)
		goto cleanup;
	if ((err = WESetInfo( wePixelToCharHook, &_weTabPixelToCharProc, we )) != noErr)
		goto cleanup;
	if ((err = WESetInfo( weCharToPixelHook, &_weTabCharToPixelProc, we )) != noErr)
		goto cleanup;
	if ((err = WESetInfo( weLineBreakHook, &_weTabLineBreakProc, we )) != noErr)
		goto cleanup;

cleanup:
	return err;
}

pascal OSErr WERemoveTabHooks( WEReference we )
{
	UniversalProcPtr hook = NULL;
	OSErr err;
	
	if ((err = WESetInfo( weDrawTextHook, &hook, we )) != noErr)
		goto cleanup;
	if ((err = WESetInfo( wePixelToCharHook, &hook, we )) != noErr)
		goto cleanup;	
	if ((err = WESetInfo( weCharToPixelHook, &hook, we )) != noErr)
		goto cleanup;
	if ((err = WESetInfo( weLineBreakHook, &hook, we )) != noErr)
		goto cleanup;

cleanup:
	return err;
}

pascal Boolean WEIsTabHooks( WEReference we )
{
	WEDrawTextUPP hook = NULL;
		
	/* return true if our tab hooks are installed */
	
	return 	( _weTabDrawTextProc != NULL ) &&
			( WEGetInfo( weDrawTextHook, &hook, we ) == noErr) &&
			( _weTabDrawTextProc == hook );
}
