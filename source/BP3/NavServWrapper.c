/*	NavServWrapper.c

	A simple wrapper for Navigation Services and the Standard
	File Package that gives them a shared interface and uses
	Nav Services when available, or StdFile if not.
	
	Based on some sample Apple code (including SimpleText).
	
	Anthony Kozar
	November 27, 2006
 */

#if !TARGET_API_MAC_CARBON_FORGET_THIS
#  include	<StandardFile.h>
#endif

#include	"NavServWrapper.h"

/* Bol Processor functions used below --
   You can replace these with the standard MacOS calls or your
   application's equivalent if reusing this code. */
extern int **GiveSpace(Size);			// replace with NewHandle
extern int MyDisposeHandle(Handle*);	// DisposeHandle
extern int MyLock(int high, Handle h);	// HLock (remove param 'high')
extern int MyUnlock(Handle h);		// HUnlock
extern int DoEvent(EventRecord *p_event);

/* You should call this function if there is any chance that 
   your code will call NSWCleanupReply without first calling 
   NSWPutFile, NSWGetFile, etc. for this NSWReply record.  */
OSErr NSWInitReply(NSWReply* reply)
{
	reply->usedNavServices	= false;
	reply->needCleanup	= false;
	reply->saveCompleted	= false;
	reply->isHTML = false;
	reply->isText = false;

	return noErr;
}

/* Call this function before using an NSWOptions struct so 
   that you do not have to initialize every field. */
void	NSWOptionsInit(NSWOptions* opts)
{
	opts->prompt = NULL;
	opts->appName = NULL;
	opts->prefKey = 0;
	opts->menuItems = NULL;
	return;
}

static int NSW_have_checked_for_NavServ = false;
static int NSW_have_NavServ = false;

static int IsNavServAvailable()
{
	if (NSW_have_checked_for_NavServ)	return NSW_have_NavServ;
	else {
		long	result;
		
		/* need to check for Appearance Manager first */
		Gestalt(gestaltAppearanceAttr, &result);
		if (result & (1 << gestaltAppearanceExists))
			NSW_have_NavServ = NavServicesAvailable();
		else	NSW_have_NavServ = false;
		NSW_have_checked_for_NavServ = true;
	}
	
	return NSW_have_NavServ;
}


/*----------------------------------------------------------------------------
	CopyPascalString 
	
	Copy a pascal format string.
	From Norstad's Reusables, strutil.c
			
	Entry:	to = pointer to destination string.
			from = pointer to source string.
----------------------------------------------------------------------------*/

static void	CopyPascalString(StringPtr to, StringPtr from)
{
	BlockMoveData(from, to, *from+1);
}


/* You should provide this function in another file if you use
   the event filter procedure below. */
extern void HandleNavServUpdateEvent(WindowPtr, EventRecord*);

extern pascal void PutFileEventProc(NavEventCallbackMessage callBackSelector,
						NavCBRecPtr callBackParms,
						NavCallBackUserData callBackUD);

/* Simplistic event filter procedure - customized for Bol Processor. */
static pascal void SimpleNavServEventProc(NavEventCallbackMessage callBackSelector,
							NavCBRecPtr callBackParms,
							NavCallBackUserData callBackUD)
{
	NSWReply*		reply;
	//WindowPtr		window;
	NavMenuItemSpec*	item;
	
	// Be careful not to access fields in callBackParms before checking the
	// callBackSelector; They are not valid every time this function is called!
	
	switch (callBackSelector)
	{
		case kNavCBEvent:
   			switch (((callBackParms->eventData).eventDataParms).event->what)
			{
				case updateEvt:
					//window = (WindowPtr)callBackParms->eventData.eventDataParms.event->message;
					//HandleNavServUpdateEvent(window,
					//	(EventRecord*)callBackParms->eventData.eventDataParms.event);
					DoEvent((EventRecord*)callBackParms->eventData.eventDataParms.event);
					break;
				default:
					break;
			}
			break;
		case kNavCBPopupMenuSelect:
			// save the user's selection of our custom format options
			item = (NavMenuItemSpec*) callBackParms->eventData.eventDataParms.param;
			reply = (NSWReply*) callBackUD;
			// do your custom stuff here ...
			break;
		default:
			break;
	}
}

/* This function is straight out of SimpleText */
static NavTypeListHandle NewNavTypeList(OSType creator, short numTypes, OSType typeList[])
{
	NavTypeListHandle open = NULL;
	
	if (numTypes > 0) {
		open = (NavTypeListHandle) GiveSpace(sizeof(NavTypeList) + numTypes*sizeof(OSType));
		if (open != NULL) {
			(*open)->componentSignature = creator;
			(*open)->osTypeCount        = numTypes;
			BlockMoveData(typeList, (*open)->osType, numTypes*sizeof(OSType));
		}
	}
	
	return open;
}

/* This is modified quite a bit from SimpleText */
OSErr NSWGetFile(NSWReply* reply, OSType creator, short numTypes, OSType typeList[], FSSpec* location, NSWOptions* opts)
{
	OSErr			err = noErr;
	NavDialogOptions	dialogOptions;
	NavTypeListHandle	openList;
	NavEventUPP		eventUPP;

	reply->needCleanup	= false;
	reply->saveCompleted	= false;
	
	/* Nav Serv is guaranteed to be available with Carbon, but we need
	  to test for its presence in non-Carbon environment. */
#if !TARGET_API_MAC_CARBON_FORGET_THIS
	if (!IsNavServAvailable())  {
		// use StdFilePackage
		reply->usedNavServices = false;
		StandardGetFile(NULL, numTypes, typeList, (StandardFileReply*)reply);
		reply->isStationery = ((reply->sfFlags & kIsStationery) != 0);
		return noErr;
	}
#endif
	
	reply->usedNavServices	= true;
	eventUPP = NewNavEventUPP(SimpleNavServEventProc);
	err = NavGetDefaultDialogOptions(&dialogOptions);
	
	if (err == noErr) {
		dialogOptions.preferenceKey = kDefaultPrefKey;
		if (opts) {
			if (opts->prompt)  CopyPascalString(dialogOptions.message, opts->prompt);
			if (opts->appName) CopyPascalString(dialogOptions.clientName, opts->appName);
			if (opts->prefKey) dialogOptions.preferenceKey = opts->prefKey;
			if (opts->menuItems) dialogOptions.popupExtension = opts->menuItems;
		}
	
		openList = NewNavTypeList(creator, numTypes, typeList);
		if (openList)  MyLock(FALSE, (Handle)openList);
		
		// FIXME: convert location parameter to AEDesc*
		// pass the entire reply as our callback user data
		err = NavGetFile(/* location */ NULL, &reply->navReply, &dialogOptions, eventUPP, NULL, NULL, openList, reply);
		DisposeNavEventUPP(eventUPP);
		reply->needCleanup = true;

		reply->sfGood = reply->navReply.validRecord;
		if (err == noErr && reply->navReply.validRecord)
		{
			AEKeyword	keyword;
			DescType	actualType;
			Size		actualSize;
			FSSpec	spec;
			FInfo		fileInfo;
			long		count;

			/* There may be multiple selected items, so we count them first */
			err = AECountItems(&(reply->navReply.selection),&count);
			if ( err == noErr && count == 1)
			{
				err = AEGetNthPtr(&(reply->navReply.selection), 1, typeFSS,
							&keyword, &actualType,
							&spec, sizeof(spec), &actualSize);
				if (err == noErr) {
					/* fill in the StandardFileReply with info */
					BlockMove(&spec, &(reply->sfFile), sizeof(FSSpec));
					reply->sfReplacing  = reply->navReply.replacing;
					reply->sfScript     = reply->navReply.keyScript;
					reply->isStationery = reply->navReply.isStationery;
					/* sfIsFolder & sfIsVolume are only used by StdFile in dlg hook */
					reply->sfIsFolder  = false;	/* can't be a folder or volume */
					reply->sfIsVolume  = false;
					/* get the Finder Info to determine other fields */
					err = FSpGetFInfo(&spec, &fileInfo);
					if (err == noErr) {
						reply->sfFlags = fileInfo.fdFlags;
						reply->sfType  = fileInfo.fdType;
					}
					else {  /* should probably return the error ... */
						reply->sfFlags = 0;
						reply->sfType = 0;
					}
				}
			}
			else if ( err == noErr && count > 1) {
				// Multiple files to open: could use AppleEvents or an array of FSSpecs
				// err = SendOpenAE(reply->navReply.selection);
				// OR
				// for (index=1; index <= count; index++)  // numbered from 1 to count
				SysBeep(30L);
				reply->sfGood = reply->navReply.validRecord = FALSE;
				err = -1;
			}
		}
		
		if (openList != NULL) {
			MyUnlock((Handle)openList);
			MyDisposeHandle((Handle*)&openList);
		}
	}
	
	return err;
}

/* This function is significantly modified from sample code in the Nav Services documentation */
OSErr NSWPutFile(NSWReply* reply, OSType creator, OSType fileType, const StringPtr defaultName, NSWOptions* opts)
{
	OSErr			err = noErr;
	NavDialogOptions	dialogOptions;
	NavEventUPP		eventProc;
	
	reply->needCleanup	= false;
	reply->saveCompleted	= false;
	
	/* Nav Serv is guaranteed to be available with Carbon, but we need
	  to test for its presence in non-Carbon environment. */
#if !TARGET_API_MAC_CARBON_FORGET_THIS
	if (!IsNavServAvailable())  {
		StringPtr	prompt;
		// use StdFilePackage
		reply->usedNavServices = false;
		if (opts && opts->prompt)  prompt = opts->prompt;
		else                       prompt = "\pSave file as:";
		StandardPutFile(prompt, defaultName, (StandardFileReply*)reply);
		reply->isStationery = false;
		return noErr;
	}
#endif
		
	reply->usedNavServices	= true;
	eventProc = NewNavEventUPP(PutFileEventProc);
	err = NavGetDefaultDialogOptions(&dialogOptions);
	dialogOptions.preferenceKey = kDefaultPrefKey;

	// worrying about translation is too much ******* work!
	dialogOptions.dialogOptionFlags |= kNavDontAddTranslateItems;

	if (err == noErr)
	{
		CopyPascalString(dialogOptions.savedFileName, defaultName);
		if (opts) {
			if (opts->prompt)  CopyPascalString(dialogOptions.message, opts->prompt);
			if (opts->appName) CopyPascalString(dialogOptions.clientName, opts->appName);
			if (opts->prefKey) dialogOptions.preferenceKey = opts->prefKey;
			if (opts->menuItems) dialogOptions.popupExtension = opts->menuItems;
		}
		
		// pass the entire reply as our callback user data
		err = NavPutFile(NULL, &reply->navReply, &dialogOptions, eventProc, fileType, creator, reply);
		DisposeNavEventUPP(eventProc);
		reply->needCleanup = true;

		reply->sfGood = reply->navReply.validRecord;
		if (err == noErr && reply->navReply.validRecord)
		{
			AEKeyword theKeyword;
			DescType  actualType;
			Size      actualSize;
			FSSpec    spec;
			
			err = AEGetNthPtr(&(reply->navReply.selection), 1, typeFSS,
						&theKeyword, &actualType,
						&spec, sizeof(spec), &actualSize);
			if (err == noErr) {
				/* fill in the StandardFileReply with info */
				BlockMove(&spec, &(reply->sfFile), sizeof(FSSpec));
				reply->sfReplacing = reply->navReply.replacing;
				reply->sfScript    = reply->navReply.keyScript;
				/* sfIsFolder & sfIsVolume are only used by StdFile in dlg hook */
				reply->sfIsFolder  = false;	/* can't be a folder or volume */
				reply->sfIsVolume  = false;
				/* sfReply.sfFlags & sfType are set to 0 by StandardPutFile */
				reply->sfFlags = 0;
				reply->sfType = 0;
				reply->isStationery = reply->navReply.isStationery;
			}
		}
			
	}
	return err;
}

/* You must call this function if reply->needCleanup is true.
   If reply was used with NSWPutFile() and you successfully
   finished saving the file, set reply->saveCompleted to true
   before calling this function. */
OSErr NSWCleanupReply(NSWReply* reply)
{
	OSErr	err = noErr;
	
	if (!IsNavServAvailable()) return noErr;
	
	if (reply->usedNavServices && reply->saveCompleted) {
		// Always call NavCompleteSave() to complete saves
		err = NavCompleteSave(&reply->navReply, kNavTranslateCopy);
	}
	if (reply->needCleanup)  NavDisposeReply(&reply->navReply);

	return err;
}
