/*	NavServWrapper.c

	A simple wrapper for Navigation Services and the Standard
	File Package that gives them a shared interface and uses
	Nav Services when available, or StdFile if not.
	
	Based on some sample Apple code.
	
	Anthony Kozar
	November 27, 2006
 */

#include	"NavServWrapper.h"

/* Call this function before using an NSWOptions struct so 
   that you do not have to initialize every field. */
void	NSWOptionsInit(NSWOptions* opts)
{
	opts->prompt = NULL;
	opts->appName = NULL;
	opts->prefKey = 0;
	return;
}

static Boolean NSW_have_checked_for_NavServ = false;
static Boolean NSW_have_NavServ = false;

static Boolean IsNavServAvailable()
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

/* Very simplistic event filter procedure for Nav Services. */
static pascal void SimpleNavServEventProc(NavEventCallbackMessage callBackSelector,
							NavCBRecPtr callBackParms,
							NavCallBackUserData callBackUD)
{
	/* FIXME: this function is crashing on OS X */
	/*WindowPtr window = (WindowPtr)callBackParms->eventData.eventDataParms.event->message;
	switch (callBackSelector)
	{
		case kNavCBEvent:
			switch (((callBackParms->eventData).eventDataParms).event->what)
			{
				case updateEvt:
					HandleNavServUpdateEvent(window,
						(EventRecord*)callBackParms->eventData.eventDataParms.event);
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}*/
}

OSErr NSWPutFile(NSWReply* reply, OSType creator, OSType fileType, const StringPtr defaultName, NSWOptions* opts)
{
	OSErr			err = noErr;
	NavDialogOptions	dialogOptions;
	NavEventUPP		eventProc;
	
	if (!IsNavServAvailable())  return -1;	/* FIXME: use StdFile instead */
	
	reply->usedNavServices	= true;
	reply->needCleanup	= false;
	reply->saveCompleted	= false;
	
	eventProc = NewNavEventUPP(SimpleNavServEventProc);
	err = NavGetDefaultDialogOptions(&dialogOptions);
	dialogOptions.preferenceKey = kDefaultPrefKey;

	// user might want to translate the saved doc into another format
	// dialogOptions.dialogOptionFlags -= kNavDontAddTranslateItems;

	if (err == noErr)
	{
		CopyPascalString(dialogOptions.savedFileName, defaultName);
		if (opts) {
			if (opts->prompt)  CopyPascalString(dialogOptions.message, opts->prompt);
			if (opts->appName) CopyPascalString(dialogOptions.clientName, opts->appName);
			if (opts->prefKey) dialogOptions.preferenceKey = opts->prefKey;
		}
		
		err = NavPutFile(NULL, &reply->navReply, &dialogOptions, eventProc, fileType, creator, NULL);
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
				reply->sfIsFolder  = false;	/* can't be a folder or volume */
				reply->sfIsVolume  = false;
				/* FIXME: need to set sfReply.sfFlags & sfType ?? */
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
	OSErr	err;
	
	if (reply->saveCompleted) {
		// Always call NavCompleteSave() to complete saves
		err = NavCompleteSave(&reply->navReply, kNavTranslateInPlace);
	}
	if (reply->needCleanup)  NavDisposeReply(&reply->navReply);

	return err;
}
