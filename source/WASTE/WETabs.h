/*
 *	WETabs.h
 *
 *	Routines for installing and removing tab hooks
 *
 */

#ifndef _WASTE_
#include "WASTE.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

pascal OSErr WEInstallTabHooks( WEReference we );
pascal OSErr WERemoveTabHooks( WEReference we );
pascal Boolean WEIsTabHooks( WEReference we );

#ifdef __cplusplus
}
#endif
