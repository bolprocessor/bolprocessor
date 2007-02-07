/*	NavServWrapper.h

	A simple wrapper for Navigation Services and the Standard
	File Package that gives them a shared interface and uses
	Nav Services when available, or StdFile if not.
	
	Portions based on some sample Apple code.
	
	Anthony Kozar
	November 27, 2006
 */

#ifndef	ANTHONY_NAVSERV_WRAPPER_H
#define	ANTHONY_NAVSERV_WRAPPER_H

#if !TARGET_API_MAC_CARBON
#  include	<Navigation.h>
#  include	<Gestalt.h>
#else
#  include	<Carbon.h>
#endif

#define	kDefaultPrefKey	1

/* For backwards-compatibility with StdFilePkg */
#if TARGET_API_MAC_CARBON
  typedef unsigned long  SFTypeList[4]; 
#endif

#pragma options align=mac68k
/* Our own file dialog reply record.
   Has fields with the same names as StandardFileReply and it is
   castable to StandardFileReply for convenience in updating existing code. */
typedef struct NSWReply {
	/* These first 10 fields are identical to StandardFileReply */
	Boolean		sfGood;		/*TRUE if user did not cancel*/
	Boolean		sfReplacing;	/*TRUE if replacing file with same name*/
	OSType		sfType;		/*file type*/
	FSSpec		sfFile;		/*selected file, folder, or volume*/
	ScriptCode		sfScript;		/*script of file, folder, or volume name*/
	short			sfFlags;		/*Finder flags of selected item*/
	Boolean		sfIsFolder;		/*selected item is a folder*/
	Boolean		sfIsVolume;		/*selected item is a volume*/
	long			sfReserved1;	/*reserved*/
	short			sfReserved2;	/*reserved*/

	/* our extra fields for combatibility with Nav Services */
	NavReplyRecord	navReply;
	Boolean		isStationery;	/* a copy of the field returned by NavServ */
	Boolean		usedNavServices;	/* TRUE if NavServ used --> navReply is valid */
	Boolean		needCleanup;	/* need to call NSWCleanupReply before disposing */
	Boolean		saveCompleted;	/* you should set this TRUE when applicable */
	
	/* custom fields for Bol Processor */
	Boolean		isHTML;
	Boolean		isText;
} NSWReply;
#pragma options align=reset

/* Optional parameters struct -- pass NULL if you don't need any of these */
typedef struct NSWOptions {
	StringPtr	prompt;	/* dialog box prompt */
	StringPtr	appName;	/* application name */
	UInt32	prefKey;	/* preference key for Nav Services */
	NavMenuItemSpecArrayHandle	menuItems;	/* items for Show/Format menu */
} NSWOptions;


OSErr NSWInitReply(NSWReply* reply);
OSErr NSWCleanupReply(NSWReply* reply);
void	NSWOptionsInit(NSWOptions* opts);

OSErr NSWPutFile(NSWReply* reply, OSType creator, OSType fileType, 
			const StringPtr defaultName, NSWOptions* opts);
			


#endif
