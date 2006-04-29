/*
 *	WETabHooks.h
 *
 *	WASTE Tabs Private Interface
 *
 */

#include "WASTEIntf.h"

#ifdef __cplusplus
extern "C" {
#endif

pascal void 				_WETabDrawText
			(Ptr, long, Fixed, JustStyleCode, WEHandle);

pascal long 				_WETabPixelToChar
			(Ptr, long, Fixed, Fixed *, char *, JustStyleCode, Fixed, WEHandle);

pascal short 				_WETabCharToPixel
			(Ptr, long, Fixed, long, short, JustStyleCode, long, WEHandle);

pascal StyledLineBreakCode	_WETabLineBreak
			(Ptr, long, long, long, Fixed *, long *, WEHandle);

#ifdef __cplusplus
}
#endif