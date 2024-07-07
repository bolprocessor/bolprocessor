/* SaveLoads3.c (BP2 version CVS) */ 

/*  This file is a part of Bol Processor
    Copyright (c) 1990-2000 by Bernard Bel, Jim Kippen and Srikumar K. Subramanian
    All rights reserved. 
    
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met: 
    
       Redistributions of source code must retain the above copyright notice, 
       this list of conditions and the following disclaimer. 
    
       Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution. 
    
       Neither the names of the Bol Processor authors nor the names of project
       contributors may be used to endorse or promote products derived from this
       software without specific prior written permission. 
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

#include <string.h>

#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"
#include "ConsoleMessages.h"

#if BP_CARBON_GUI_FORGET_THIS

int SaveAs(Str255 fn,FSSpec *p_spec,int w)
// The "save as..." command
// w is the index of the window whose text we'll save to a file
{
short refnum;
int i,n;
long count;
NSWReply reply;
OSErr err;

if(w < 0 || w >= WMAX || !Editable[w]) {
	BPPrintMessage(odError,"=> Err. SaveAs(). Incorrect window index '%d'\n",w);
	return(MISSED);
	}
err = NSWInitReply(&reply);
/* If the file name is empty, at least we insert its prefix */
if(fn[0] == 0) {
	if (GetDefaultFileName(w, Message) != OK) return(MISSED);
	c2pstrcpy(fn, Message);
}
reply.sfFile.vRefNum = TheVRefNum[w];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[w];
if(NewFile(w,gFileType[w],fn,&reply)) {
	i = CreateFile(w,w,gFileType[w],fn,&reply,&refnum);
	SetCursor(&WatchCursor);
	*p_spec = reply.sfFile;
	if(i == ABORT) {
		err = NSWCleanupReply(&reply);
		return(MISSED);
		}
	if(i == OK) {
		/* Update text length before saving */
		UpdateThisWindow(FALSE,Window[w]);
		WriteHeader(w,refnum,*p_spec);
		WriteFile(TRUE,MAC,refnum,w,GetTextLength(w));
		WriteEnd(w,refnum);
		GetFPos(refnum,&count);
		SetEOF(refnum,count);
		FlushFile(refnum);
		MyFSClose(w,refnum,p_spec);
		reply.saveCompleted = true;
		Dirty[w] = FALSE;
		CheckTextSize(w);
		err = NSWCleanupReply(&reply);
		return(OK);
		}
	else {
		MyPtoCstr(MAXNAME,fn,Message);
		BPPrintMessage(odError,"Can't create file '%s'\n",Message);
		}
	}
err = NSWCleanupReply(&reply);
return(MISSED);
}


int SaveFile(Str255 fn,FSSpec *p_spec,int w)
// Save the content of window index w to a file, the specs of it you think you know
{
short refnum;
int good,n;
long count,k;
// char line[MAXLIN];
OSErr io;

if(w < 0 || w >= WMAX || !Editable[w]) {
	BPPrintMessage(odError,"=> Err. SaveFile(). Incorrect window index '%d'\n",w);
	return(MISSED);
	}
SetCursor(&WatchCursor);
// spec should already be filled-in; fn (which should == spec.name) should therefore
// already be limited to length MAXNAME!  - akozar, 031907
// MyPtoCstr(MAXNAME,fn,line);	/* limit the length of filename */
// c2pstrcpy(p_spec->name, line);
good = ((io=MyOpen(p_spec,fsCurPerm,&refnum)) == noErr);
if(good) {
	UpdateThisWindow(FALSE,Window[w]);
	WriteHeader(w,refnum,*p_spec);
	WriteFile(TRUE,MAC,refnum,w,GetTextLength(w));
	WriteEnd(w,refnum);
	GetFPos(refnum,&count);
	SetEOF(refnum,count);
	FlushFile(refnum);
	MyFSClose(w,refnum,p_spec);
	Dirty[w] = FALSE;
	ShowLengthType(w);
	CheckTextSize(w);
	if(w == wGrammar) {
		c2pstrcpy(fn, FileName[wAlphabet]);
		if(Dirty[wAlphabet]
			&& ((GetAlphaName(wGrammar) == OK) || (GetAlphaName(wData) == OK))) {
			StopWait();
			if(Answer("Also save alphabet",'Y') == YES) {
				c2pstrcpy(p_spec->name, FileName[wAlphabet]);
				if(MyOpen(p_spec,fsCurPerm,&refnum) == noErr) {	// FIXME: alphabet may not be in same folder as grammar!
					UpdateThisWindow(FALSE,Window[wAlphabet]);
					WriteHeader(wAlphabet,refnum,*p_spec);
					WriteFile(TRUE,MAC,refnum,wAlphabet,GetTextLength(wAlphabet));
					WriteEnd(wAlphabet,refnum);
					GetFPos(refnum,&count);
					SetEOF(refnum,count);
					FlushFile(refnum);
					MyFSClose(wAlphabet,refnum,p_spec);
					Dirty[wAlphabet] = FALSE;
					my_sprintf(Message,"Also saved '%s'",FileName[wAlphabet]);
					ShowMessage(TRUE,wMessage,Message);
					CheckTextSize(wAlphabet);
					}
				else {
					BPPrintMessage(odError,"=> Error saving '%s'\n",FileName[wAlphabet]);
					}
				}
			Dirty[wAlphabet] = FALSE;
			}
		}
	return(OK);
	}
else {
	TellError(76,io);
	MyPtoCstr(MAXNAME,fn,LineBuff);
	my_sprintf(Message,"=> Error opening '%s'",LineBuff);
	ShowMessage(TRUE,wMessage,Message);
	return(MISSED);
	}
}

/* Returns in filename, the default save name for the specified window on this platform.
   filename should be long enough to hold the result (at least 32 on MacOS) */
int GetDefaultFileName(int w, char* filename)
{
	if(w < 0 || w >= WMAX) {
		BPPrintMessage(odError,"=> Err. GetDefaultFileName(). Incorrect window index '%d'\n",w);
		return(MISSED);
	}
	if (RunningOnOSX) {
		// suggest file extensions on OS X
		strcpy(filename, "untitled");
		strcat(filename, FileExtension[w]);
	}
	else {
		// suggest file prefixes on OS 9
		strcpy(filename, FilePrefix[w]);
		strcat(filename, "untitled");
	}
	
	return(OK);	
}

/* Returns in basename the current project's file name without any prefix or extension.
   The following filenames are checked in order: grammar, data, alphabet, settings.
   basename should be long enough to hold the result (at least MAXNAME+1) */
int GetProjectBaseName(char* basename)
{
	int  len, index;
	char workstr[MAXNAME+1];
	char *pre, *p_workstr;
	
	if      ((len = strlen(FileName[wGrammar])) > 0)   index = wGrammar;
	else if ((len = strlen(FileName[wData])) > 0)      index = wData;
	else if ((len = strlen(FileName[wAlphabet])) > 0)  index = wAlphabet;
	else if ((len = strlen(FileName[iSettings])) > 0)  index = iSettings;
	else return (MISSED);
	
	if(len > MAXNAME) {
		BPPrintMessage(odError,"=> Err. GetProjectBaseName(): len > MAXNAME\n");
		return (MISSED);
	}
	
	// copy everything but the extension for this file type
	if (MatchFileNameExtension(FileName[index], FileExtension[index])) {
		len -= strlen(FileExtension[index]);
		strncpy(workstr, FileName[index], len);
		workstr[len] = '\0'; // strncpy doesn't terminate the string
	}
	else  strcpy(workstr, FileName[index]);
	
	// now, copy everything but the prefix for this file type
	// (note, that we allow for both just in case ...)
	pre = FilePrefix[index];
	p_workstr = workstr; 
	len = strlen(pre);
	if (Match(TRUE, &p_workstr, &pre, len))
		strcpy(basename, &workstr[len]);
	else  strcpy(basename, workstr);
	
	return(OK);	
}

/* SelectCreatorAndFileType returns the default creator and file type codes
   for a Save file dialog */
void SelectCreatorAndFileType(int type, OSType* thecreator, OSType* thetype)
{
	*thecreator = 'Bel0';
	switch(type) {
		case ftiAny:
		case ftiText:
			*thetype = 'TEXT';
			break;
		case ftiKeyboard:
			*thetype = 'BP02';	/* -kb keyboard file */
			break;
		case ftiObjects:
			*thetype = 'BP03';	/* -mi MIDI object file */
			break;
		case ftiDecisions:
			*thetype = 'BP04';	/* decision file */
			break;
		case ftiGrammar:
			*thetype = 'BP05';	/* -gr grammar file */
			break;
		case ftiAlphabet:
			*thetype = 'BP06';	/* -ho alphabet file */
			break;
		case ftiData:
			*thetype = 'TEXT';	/* -da data file.  Suppressed 'BP07' */
			break;
		case ftiInteraction:
			*thetype = 'BP08';	/* -in interactive code file */
			break;
		case ftiSettings:
			*thetype = 'BP09';	/* -se settings file */
			break;
		case ftiAIFC:
			*thetype = 'AIFC';	/* AIFF compressed file */
			break;
		case ftiMidi:
			*thetype = 'Midi';	/* MIDI file */
			break;
		case ftiWeights:
			*thetype = 'BP10';	/* -wg weight file */
			break;
		case ftiScript:
			*thetype = 'BP11';	/* BP script file */
			break;
		case ftiGlossary:
			*thetype = 'BP12';	/* glossary -gl file */
			break;
		case ftiTimeBase:
			*thetype = 'BP13';	/* time base -tb file */
			break;
		case ftiCsoundInstruments:
			*thetype = 'BP14';	/* Csound instruments -cs file */
			break;
		case ftiMIDIorchestra:
			*thetype = 'BP15';	/* MIDI orchestra -or file */
			break;
		case ftiHTML:
			*thetype = 'TEXT';
			if (RunningOnOSX)	*thecreator = '\0\0\0\0';	/* use user's default browser? */
			else			*thecreator = 'MOSS';		/* Netscape creator */
			break;
		case ftiMidiDriver:
			*thetype = 'BP16';	/* MIDI driver settings (-md) file */
			break;
		}
	return;
}

/* FillTypeList makes a list of file type codes for an Open file dialog */
void FillTypeList(int type, SFTypeList typelist, int* numtypes)
{
	/* Modified 013107 by akozar: 'MOSS' is a creator not a file 
	   type.  Use 'TEXT' instead.  'text' is not used.  */
switch(type) {
	case ftiAny:				/* open any file */
		*numtypes = -1;
		break;
	case ftiText:
		typelist[0] = 'TEXT';
		//typelist[1] = 'MOSS';	/* Netscape file */
		//typelist[2] = 'text';
		*numtypes = 1;
		break;
	case ftiKeyboard:
		typelist[0] = 'BP02';	/* -kb file */
		*numtypes = 1;
		break;
	case ftiObjects:
		typelist[0] = 'BP03';	/* -mi file */
		*numtypes = 1;
		break;
	case ftiDecisions:
		typelist[0] = 'BP04';	/* decision file */
		*numtypes = 1;
		break;
	case ftiGrammar:
		typelist[0] = 'BP05';	/* grammar -gr file */
		typelist[1] = 'TEXT';
		*numtypes = 2;
		break;
	case ftiAlphabet:
		typelist[0] = 'BP06';	/* alphabet -ho file */
		typelist[1] = 'TEXT';
		*numtypes = 2;
		break;
	case ftiData:
		typelist[0] = 'TEXT';
		typelist[1] = 'BP07';	/* data -da file */
		//typelist[2] = 'MOSS';	/* Netscape file */
		//typelist[3] = 'text';
		*numtypes = 2;
		break;
	case ftiInteraction:
		typelist[0] = 'BP08';	/* interactive -in file */
		typelist[1] = 'TEXT';
		*numtypes = 2;
		break;
	case ftiSettings:
		typelist[0] = 'BP09';	/* settings -se file */
		*numtypes = 1;
		break;
	case ftiAIFC:
		/* First two of these also presumably creator codes */
		//typelist[0] = 'FSSD';	/* SoundEdit file */
		//typelist[1] = 'jB1 ';	/* SoundEdit Pro file */
		typelist[0] = 'AIFF';	/* AIFF file */
		typelist[1] = 'AIFC';	/* AIFF compressed file */
		*numtypes = 2;
		break;
	case ftiMidi:
		typelist[0] = 'Midi';	/* MIDI file */
		*numtypes = 1;
		break;
	case ftiWeights:
		typelist[0] = 'BP10';	/* weights -wg file */
		*numtypes = 1;
		break;
	case ftiScript:
		typelist[0] = 'BP11';	/* script +sc file */
		typelist[1] = 'TEXT';
		*numtypes = 2;
		break;
	case ftiGlossary:
		typelist[0] = 'BP12';	/* glossary -gl file */
		typelist[1] = 'TEXT';
		*numtypes = 2;
		break;
	case ftiTimeBase:
		typelist[0] = 'BP13';	/* time base -tb file */
		*numtypes = 1;
		break;
	case ftiCsoundInstruments:
		typelist[0] = 'BP14';	/* Csound instruments -cs file */
		*numtypes = 1;
		break;
	case ftiMIDIorchestra:
		typelist[0] = 'BP15';	/* MIDI orchestra -or file */
		typelist[1] = 'TEXT';	// added 060807 because "Text file" is now an option in Save dialog
		*numtypes = 2;
		break;
	case ftiHTML:
		if (Beta) Alert1("=> Err. FillTypeList(): type 18 not allowed.");
		break;
	case ftiMidiDriver:
		typelist[0] = 'BP16';	/* MIDI driver settings (-md) file */
		*numtypes = 1;
		break;
	}

	return;
}

// GCC does not like const values as global array sizes
// const int	MAXFORMATNAMES = 20;
// const int	MFNLEN = 30;
#define	MAXFORMATNAMES	(20)
#define	MFNLEN	(30)

const char  FormatNames[MAXFORMATNAMES][MFNLEN] = {
		"BP2 text file",	 			/* 'TEXT' */
		"BP2 text file",				/* 'TEXT' */
		"BP2 keyboard",				/* 'BP02' */
		"BP2 sound-object prototypes",	/* 'BP03' */
		"BP2 decisions",				/* 'BP04' */
		"BP2 grammar",				/* 'BP05' */
		"BP2 alphabet",				/* 'BP06' */
		"BP2 data",					/* 'TEXT', 'BP07' */
		"BP2 interaction",			/* 'BP08' */
		"BP2 settings",				/* 'BP09' */
		"AIFF-C sound file",			/* 'AIFC' */
		"BP2 MIDI file",				/* 'Midi' */
		"BP2 weights",				/* 'BP10' */
		"BP2 script",				/* 'BP11' */
		"BP2 glossary",				/* 'BP12' */
		"BP2 time-base",				/* 'BP13' */
		"BP2 Csound instruments",		/* 'BP14' */
		"BP2 MIDI orchestra",			/* 'BP15' */
		"Standard HTML file",			/* 'TEXT', creator depends on OS or browser */
		"BP2 MIDI driver settings"		/* 'BP16' */
		};
		
// const int	HFLEN = 8;
#define	HFLEN	(8)
const	char	HTMLFormat[HFLEN] = " (HTML)";	/* this is appended to primary name */

int MakeFormatMenuItems(int type, NavMenuItemSpecArrayHandle* p_handle)
{
	NavMenuItemSpec* items;
	int i, numitems, typelist[4];
	char HTMLname[MFNLEN+HFLEN];
	
	if (type < 0 || type >= MAXFORMATNAMES)  type = ftiText;
	
	// MIDI orchestra does not support HTML
	if (type == ftiMIDIorchestra) {
		typelist[0] = type;				// BP2 native format
		typelist[1] = ftiText;				// BP2 plain text
		numitems = 2;
	}
	else {
		typelist[0] = type;				// BP2 native format
		typelist[1] = type;				// BP2 native format with HTML encoding
		if (type == ftiAny || type == ftiText) {	// (skip adding plain text again)
			typelist[2] = ftiHTML;			// HTML
			numitems = 3;
		}
		else {
			typelist[2] = ftiText;			// BP2 plain text
			typelist[3] = ftiHTML;			// HTML
			numitems = 4;
		}
	}
	
	/* resize before dereferencing */
	if (MySetHandleSize((Handle*)p_handle, numitems*sizeof(NavMenuItemSpec)) != OK)
		return (ABORT);
	items = **p_handle;
	
	/* This loop assumes that if numitems is greater than 2, then the item with index
	   1 is "native HTML" format, otherwise all items are "as advertised". */
	for (i = 0; i < numitems; ++i) {
		items[i].version = kNavMenuItemSpecVersion;
		SelectCreatorAndFileType(typelist[i], &(items[i].menuCreator), &(items[i].menuType));
		if (i == 1 && numitems > 2) {
			strcpy(HTMLname, FormatNames[typelist[i]]);
			strcat(HTMLname, HTMLFormat);
			c2pstrcpy(items[i].menuItemName, HTMLname);
		}
		else  c2pstrcpy(items[i].menuItemName, FormatNames[typelist[i]]);
	}
	
	return(OK);
	
}

/* returns TRUE if this window can be saved in multiple formats */
int CanSaveMultipleFormats(int w)
{
	if (w == wHelp || w == wNotice || w == wPrototype7 || w == wCsoundTables)
		return (FALSE);
	else if (w >= 0 && w < WMAX && Editable[w] || w == wMIDIorchestra)
		return (TRUE);
	else  return (FALSE);
}

/* NewFile()
   type is the file type constant (see SelectCreatorAndFileType() above).
   
   w is the window index used in determining the possible save formats 
   (see CreateFile() below); While w is the expected index for editable 
   text documents, document types that use dialogs (e.g. Time-base) 
   usually pass -1 (there are exceptions) and document types without a 
   dedicated window pass -1 as well. */
int NewFile(int w, int type, Str255 fn, NSWReply *p_reply)
// Check whether the file we're creating is a new one, and get its specs in a reply record
{
	NavMenuItemSpecArrayHandle formatItems = NULL;
	NSWOptions opts;
	OSType creator, filetype;
	OSErr io;

	if(CallUser(1) != OK) return(MISSED);

	SetDefaultCursor();
	/* spec = p_reply->sfFile; */

	/* Let's first recall the default folder in which this file had been opened, to neutralize the effect of DefaultFolder */
	/* Helas, this does not neutralize DefaultFolder! */
	/*if(fn[0] > 0) {
		CopyPString(fn,p_reply->sfFile.name);
		CopyPString(fn,spec.name);
		io = FSpOpenDF(&spec,fsRdPerm,&refnum);
		if(io == noErr) FSClose(refnum); 
		} */
	NSWOptionsInit(&opts);
	opts.appName = "\pBol Processor";
	opts.prompt  = "\pSave file as:";
	if (CanSaveMultipleFormats(w)) {
		formatItems = (NavMenuItemSpecArrayHandle) GiveSpace(1);
		if (MakeFormatMenuItems(type, &formatItems) != OK) {
			MyDisposeHandle((Handle*)&formatItems);
			return(MISSED);
		}
		opts.menuItems = formatItems;
		creator  = kNavGenericSignature;
		filetype = kNavGenericSignature;
	}
	else SelectCreatorAndFileType(type, &creator, &filetype);
	AlertOn = TRUE; // prevent ResumeStop from showing
	io = NSWPutFile(p_reply, creator, filetype, fn, &opts);
	AlertOn = FALSE;
	if (formatItems) MyDisposeHandle((Handle*)&formatItems);
	if (io != noErr) {
		return(MISSED);
	}
	if(p_reply->sfGood) {
		CopyPString(p_reply->sfFile.name,fn);
		return(OK);
		}
	else return(MISSED);
}


int OldFile(int w,int type,Str255 fn,FSSpec *p_spec)
// Select a file you want to open
// w is the index of the text window to which this file will be saved
// p_spec doesn't matter on entry.  It returns the "file specs" record used afterwards
{
	OSErr		err, err2;
	NSWReply	reply;
	NSWOptions	opts;
	SFTypeList	typelist;
	int		numtypes;

	if(CallUser(1) != OK) return(MISSED);

	if(w < -1 || w >= WMAX) {
		if(Beta) Alert1("=> Err. OldFile(). Incorrect window index");
		return(MISSED);
		}

	SetDefaultCursor();
	FillTypeList(type, typelist, &numtypes);
	err = NSWInitReply(&reply);
	NSWOptionsInit(&opts);
	opts.appName = "\pBol Processor";
	opts.prompt  = "\pSelect a file:"; // could customize this by type
	AlertOn = TRUE; // prevent ResumeStop from showing
	err = NSWGetFile(&reply, 'Bel0', numtypes, typelist, NULL, &opts);
	AlertOn = FALSE;
	err2 = NSWCleanupReply(&reply);
	if  (err != noErr)  return (MISSED);
	if(reply.sfGood) {
		(*p_spec) = reply.sfFile;
		if(w >= 0 && w < WMAX) {
			if(reply.sfType == 'TEXT' && Editable[w] && numtypes > -1)
				IsText[w] = TRUE;
			else  IsText[w] = FALSE;
		}
		CopyPString(p_spec->name,fn);
		RecordVrefInScript(p_spec);	/* For script in learning mode */
		return(YES);
	}
	else return(NO);
}

FileTypeIndex MapFileTypeCodeToFileTypeIndex(OSType type)
{
	switch(type) {
		case 'AIFC':	return ftiAIFC; break;
		case 'AIFF':	return ftiAIFC; break;
		case 'BP02':	return ftiKeyboard; break;
		case 'BP03':	return ftiObjects; break;
		case 'BP04':	return ftiDecisions; break;
		case 'BP05':	return ftiGrammar; break;
		case 'BP06':	return ftiAlphabet; break;
		case 'BP07':	return ftiData; break;
		case 'BP08':	return ftiInteraction; break;
		case 'BP09':	return ftiSettings; break;
		case 'BP10':	return ftiWeights; break;
		case 'BP11':	return ftiScript; break;
		case 'BP12':	return ftiGlossary; break;
		case 'BP13':	return ftiTimeBase; break;
		case 'BP14':	return ftiCsoundInstruments; break;
		case 'BP15':	return ftiMIDIorchestra; break;
		case 'BP16':	return ftiMidiDriver; break;
		case 'Midi':	return ftiMidi; break;
		case 'TEXT':	return ftiText; break; // this is correct; do not change to ftiData
		default:		return ftiAny; break;
		}
	
	return ftiAny;
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

// FIXME: name should be const, but Match needs to take const char * instead of char**
int FindMatchingFileNamePrefix(/*const*/ char* name)
{
	int	w;
	char* pre;
	
	for (w = 0; w < WMAX; w++) {
		if (FilePrefix[w][0] == '\0') continue;
		// prefixes are case sensitive
		pre = FilePrefix[w];
		if (Match(TRUE, &name, &pre, strlen(pre))) return w;
	}
	
	return wUnknown;
}

// FIXME: name should be const, but Match needs to take const char * instead of char**
int FindMatchingFileNameExtension(/*const*/ char* name)
{
	int	w; 
	size_t len;
	char  *nameExt, *ext;
	
	// find the last '.' in name, if any
	nameExt = strrchr(name, '.');
	if (nameExt == NULL)  return wUnknown;
	
	len = strlen(nameExt);
	for (w = 0; w < WMAX; w++) {
		if (FileExtension[w][0] == '\0') continue;
		ext = FileExtension[w];
		// extensions must be the same length
		if (len != strlen(ext))  continue;
		// extensions are case insensitive
		if (Match(FALSE, &nameExt, &ext, strlen(ext))) return w;
	}
	
	return wUnknown;
}

// FIXME: name and ext should be const, but Match needs to take const char * instead of char**
int MatchFileNameExtension(/*const*/ char* name, /*const*/ char* ext)
{
	char* nameExt;
	
	// find the last '.' in name, if any
	nameExt = strrchr(name, '.');
	if (nameExt == NULL)  return FALSE;
	
	// extensions must be the same length
	if (strlen(nameExt) != strlen(ext))  return FALSE;
	
	// extensions are case insensitive
	return Match(FALSE, &nameExt, &ext, strlen(ext));
}

// FIXME: name should be const, but functions above need to be fixed first
int IdentifyBPFileTypeByName(/*const*/ char* name)
{
	int	wfindex;	// window/file index (result)

	// try the filename's prefix, if any
	wfindex = FindMatchingFileNamePrefix(name);
	if (wfindex != wUnknown)  return wfindex;
	
	// next try the filename's extension, if any
	wfindex = FindMatchingFileNameExtension(name);
	if (wfindex != wUnknown)  return wfindex;
	
	// also try the other extensions that we recognize
	if (MatchFileNameExtension(name, ".mid") ||
	    MatchFileNameExtension(name, ".midi"))
		return iMIDIfile;
	if (MatchFileNameExtension(name, ".trace"))  return wTrace;
	if (MatchFileNameExtension(name, ".txt") ||
	    MatchFileNameExtension(name, ".sco") ||
	    MatchFileNameExtension(name, ".htm") ||
	    MatchFileNameExtension(name, ".html"))
		return wScrap;

	return wUnknown;
}


void strip_trailing_spaces(char *str) {
    if (str == NULL) return;
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        len--;
    }
    str[len] = '\0';
}

void remove_double_slash_prefix(char *str) {
    if (strncmp(str, "//", 2) == 0) {
        // Shift everything to the left over the "//"
        memmove(str, str + 2, strlen(str) - 1);
    }
}

int ReadOne(int bindlines,int careforhtml,int nocomment,FILE* fin,int strip,char ***pp_line,
	char ***pp_completeline,long *p_pos)
// Read a line in the file and save it to text handle 'pp_completeline'
// If the line starts with "//", discard it
// bindlines is now irrelevant
{
int i, html;
char *buffer;
long size, count;
char line[1000];
buffer = NULL;

MyDisposeHandle((Handle*)pp_line);
MyDisposeHandle((Handle*)pp_completeline);
size = 1000;
if((*pp_line = (char**) GiveSpace((Size)size * sizeof(char))) == NULL) return(ABORT);
if((*pp_completeline = (char**) GiveSpace((Size)size * sizeof(char))) == NULL) return(ABORT);

// BPPrintMessage(odError,"pos1 = %ld\n",*p_pos);
if(fseek(fin, *p_pos, SEEK_SET) != 0) {
    perror("fseek failed");
    // Handle error or exit
    }
	// *p_pos = ftell(fin);
    // BPPrintMessage(odError,"pos2 = %ld\n",*p_pos);
if(fgets(line, sizeof(line),fin) != NULL) {
    if(ferror(fin)) {
        fprintf(stderr, "Error reading from file.\n");
        clearerr(fin);  // Clear the error indicator for the stream
        }
    remove_final_linefeed(line);
	*p_pos = ftell(fin);
    // BPPrintMessage(odError,"pos3 = %ld\n",*p_pos);
	size_t lineSize = utf8_strsize(line);
	char* newBuffer = realloc(buffer,lineSize + 2); // +1 for '\n' and +1 for '\0'
	if(newBuffer == NULL) {
		BPPrintMessage(odError, "=> Err. ReadOne(). newBuffer == NULL\n");
		return MISSED;
		}
	buffer =  newBuffer;
	memcpy(buffer, line, lineSize);
	buffer[lineSize] = '\0';
	if (lineSize == 0) {
        char *emptyStr = strdup(""); // Handle empty file case
        if (emptyStr != NULL) {
            free(buffer);
            buffer = emptyStr;
       		}
		else {
            BPPrintMessage(odError, "Memory allocation failed for empty string\n");
            free(buffer);
            return MISSED;
        	}
		}
	remove_carriage_returns(line);
	// BPPrintMessage(odInfo,"thisline = %s\n",buffer);
	MystrcpyStringToHandle(pp_completeline,buffer);
	if(strip) strip_trailing_spaces(buffer);
	if(careforhtml) {
		count = 1L + MyHandleLen(*pp_completeline);
		html = TRUE;
		CheckHTML(FALSE,0,*pp_completeline,&count,&html);
		}
	if(nocomment) remove_double_slash_prefix(buffer);
	MystrcpyStringToHandle(pp_line,buffer);
    free(buffer);
	return OK;
	}
return STOP;
}

/*
discount = 0; firsttime = TRUE;

RESTART:
imax = count = MAXLIN;
count = fread(*p_buffer,sizeof(char),count,fin);
oldcount = count;

CleanLF(p_buffer,&count,&dos);
// Here we cleaned the extra LF of DOS files

discount = oldcount - count;
// if(discount > 0 && firsttime) *p_pos += 1;  2024-06-24
firsttime = FALSE;
// if (oldcount < MAXLIN) {
if(count == 0) {
	// at end of file
	rep = STOP;
	}
else rep = OK;
is = 0;
if(offset == 0) {
	while(MySpace((*p_buffer)[is]) && (*p_buffer)[is] != '\r') is++;
	if((*p_buffer)[is] == '\0' || (nocomment && (*p_buffer)[is] == '/'
			&& (*p_buffer)[is+1] == '/')) {
		empty = TRUE;
		}
	oldc = '\0';
	}
if(!strip) is = 0;
for(i=is; i < count; i++) {
	c = (*p_buffer)[i];
	j = i - is + offset;
	
//	if(((oldc != '�' || !bindlines) && (c == '\n' || c == '\r')) || c == '\0' || j >= (size-discount-1)) {
	if(c == '\n' || c == '\r' || c == '\0' || j >= (size-discount-1)) { // Fixed by BB 2021-02-14
		(*p_pos) += (i + 1);
		fseek(fin,*p_pos,SEEK_SET);
		if(j >= (size-discount-1)) {
			(**pp_line)[j] = c;
			(**pp_completeline)[j] = c;
			size += (MAXLIN - discount);
			if(MySetHandleSize((Handle*)pp_line,size * sizeof(char)) != OK) return(ABORT);
			if(MySetHandleSize((Handle*)pp_completeline,size * sizeof(char)) != OK) return(ABORT);
			offset += i - is + 1; 
			oldc = c;
			goto RESTART;
			}
		(**pp_line)[j] = '\0';
		(**pp_completeline)[j] = '\0';
		rep = OK;
		goto SORTIR;
		}
	oldc = c;
	if(strip && (c == '\r')) is++; // Fixed by BB 2022-02-18
	else {
		(**pp_line)[j] = c;
		(**pp_completeline)[j] = c;
		}
	}
if(rep == STOP) {
	(**pp_line)[i-is+offset] = '\0';
	(**pp_completeline)[i-is+offset] = '\0';
	}

SORTIR:
MyDisposeHandle((Handle*)&p_buffer);

if(empty) (**pp_line)[0] = '\0';
jm = MyHandleLen(*pp_line) - 1;
for(j=jm; j > 0; j--) {
	if(MySpace((**pp_line)[j])) {
		(**pp_line)[j] = '\0';
		}
	else break;
	}
	
if(careforhtml) {
	count = 1L + MyHandleLen(*pp_completeline);
	html = TRUE;
	CheckHTML(FALSE,0,*pp_completeline,&count,&html);
	}

return(rep);
} */


int ReadInteger(FILE* fin,int* p_i,long* p_pos)
// Read an integer value
{
int rep,i;
char c;
char **p_line,**p_completeline;

p_line = p_completeline = NULL;
if((rep = ReadOne(FALSE,FALSE,TRUE,fin,TRUE,&p_line,&p_completeline,p_pos)) == MISSED) goto QUIT;
if(MyHandleLen(p_line) == 0) {
	rep = MISSED; goto QUIT;
	}
i = 0;
while(MySpace(c=(*p_line)[i])) i++;
if(c != '-' && c != '+' && !isdigit(c)) {
	rep = MISSED; goto QUIT;
	}
MyLock(FALSE,(Handle)p_line);
*p_i = (int) atol(*p_line);	/* Don't use atoi() because int's are 4 bytes */
MyUnlock((Handle)p_line);

QUIT:
MyDisposeHandle((Handle*)&p_line);
MyDisposeHandle((Handle*)&p_completeline);
return(rep);
}


int ReadLong(FILE* fin,long* p_i,long* p_pos)
{
int rep,i;
char c;
char **p_line,**p_completeline;

p_line = p_completeline = NULL;
if((rep = ReadOne(FALSE,FALSE,TRUE,fin,TRUE,&p_line,&p_completeline,p_pos)) == MISSED) goto QUIT;
if(MyHandleLen(p_line) == 0) return(MISSED);
i = 0; while(MySpace(c=(*p_line)[i])) i++;
if(c != '-' && c != '+' && !isdigit(c)) {
	rep = MISSED; goto QUIT;
	}
MyLock(FALSE,(Handle)p_line);
*p_i = atol(*p_line);
MyUnlock((Handle)p_line);

QUIT:
MyDisposeHandle((Handle*)&p_line);
MyDisposeHandle((Handle*)&p_completeline);
return(OK);
}


int ReadUnsignedLong(FILE* fin,unsigned long* p_i,long* p_pos)
{
int rep,i;
char c,*end;
char **p_line,**p_completeline;
long x;

p_line = p_completeline = NULL;
if((rep = ReadOne(FALSE,FALSE,TRUE,fin,TRUE,&p_line,&p_completeline,p_pos)) == MISSED) goto QUIT;
if(MyHandleLen(p_line) == 0) return(MISSED);
i = 0; while(MySpace(c=(*p_line)[i])) i++;
if(c != '-' && c != '+' && !isdigit(c)) {
	my_sprintf(Message,"\nUnexpected characters in integer: %s",(*p_line));
	Println(wTrace,Message);
	rep = MISSED;
	goto QUIT;
	}
MyLock(FALSE,(Handle)p_line);

(*p_i) = strtoul((*p_line),&end,0);

/* x = atol(*p_line);
if(x < ZERO) {
	my_sprintf(Message,"\nUnexpected negative integer: %ld",x);
	Println(wTrace,Message);
	rep = MISSED;
	goto QUIT;
	}
*p_i = (unsigned long) x; */
MyUnlock((Handle)p_line);

QUIT:
MyDisposeHandle((Handle*)&p_line);
MyDisposeHandle((Handle*)&p_completeline);
return(OK);
}


int ReadFloat(FILE* fin,double* p_i,long* p_pos)
{
int rep,i;
long p,q;
char c;
char **p_line,**p_completeline;

p_line = p_completeline = NULL;
if((rep = ReadOne(FALSE,FALSE,TRUE,fin,TRUE,&p_line,&p_completeline,p_pos)) == MISSED)  {
	rep = MISSED; goto QUIT;
	}
if(MyHandleLen(p_line) == 0)  {
	rep = MISSED; goto QUIT;
	}
i = 0; while(MySpace(c=(*p_line)[i])) i++;
if(c != '-' && c != '+' && !isdigit(c)) {
	rep = MISSED; goto QUIT;
	}
MyLock(FALSE,(Handle)p_line);
*p_i = Myatof(*p_line,&p,&q);
MyUnlock((Handle)p_line);
if((*p_i) < Infneg) {
	rep = MISSED; goto QUIT;
	}

QUIT:
MyDisposeHandle((Handle*)&p_line);
MyDisposeHandle((Handle*)&p_completeline);
return(rep);
}


int NewWriteToFile(char* line,FILE* fout) {
// We should use this procedure in replacement of WriteToFile()
// Writes the line and a return to the file

	int res;
	OSErr io;
	size_t written, numbytes;
	char* line2;

	numbytes = strlen(line);
	my_sprintf(line2,"%s\n",line);
	written = fwrite(line2, (size_t)1, (size_t) numbytes, fout);
	if(written < numbytes)	{
		BPPrintMessage(odError, "=> Error while writing to file.\n");
		return ABORT;
		}
	return(OK);
	}


int WriteToFile(int careforhtml,int format,char* line,short refnum)
// Writes the line and a return to the file
// Obsolete because of using FSWrite, should be replaced with NewWriteToFile() as fwrite() is already used for writing MIDI files. The only problem is to use a file pointer instead of refnum.
{
int res;
long count;
OSErr io;
char **p_line;

if(refnum == -1) {
	if(Beta) BPPrintMessage(odError,"=> Err. WriteToFile(). refnum == -1 line = \"%s\"\n",line);
	return(MISSED);
	}
p_line = NULL;
if ((res = MystrcpyStringToHandle(&p_line, line)) != OK) return res;

#if BP_CARBON_GUI_FORGET_THIS
count = (long) MyHandleLen(p_line);
MyLock(FALSE,(Handle)p_line);
io = FSWrite(refnum,&count,*p_line);
MyUnlock((Handle)p_line);
MyDisposeHandle((Handle*)&p_line);

count = 1L;
if(io == noErr) {
	switch(format) {
		case MAC:
			io = FSWrite(refnum,&count,"\r");
			break;
		case DOS:
			count = 2L;
			io = FSWrite(refnum,&count,"\r\n");
			break;
		case UNIX:
			io = FSWrite(refnum,&count,"\n");
			break;
		}
	}
if(io != noErr) {
	TellError(80,io);
	return(ABORT);
	}
#else
	// A temporary console version of file output until I decide how to rewrite
	// the interface to support both Mac and ANSI file refs. - akozar 20130903
	// 'refnum' is (ab)used to specify the output destination (see ConsoleMessages.h)

	BP_NOT_USED(format);	// need to rethink whether this option sd be used

	// I think '\n' will always print as the native line ending using fprintf
	BPPrintMessage((int)refnum, "%s\n", *p_line);
	MyDisposeHandle((Handle*)&p_line);
#endif /* BP_CARBON_GUI_FORGET_THIS */

return(OK);
}


int NoReturnWriteToFile(char* line,short refnum)
// Writes the line and no return to the file
{
long count;
OSErr io;

if(refnum == -1) {
	if(Beta) Alert1("=> Err. NoReturnWriteToFile(). refnum == -1");
	return(MISSED);
	}

#if BP_CARBON_GUI_FORGET_THIS
count = (long) strlen(line);
io = FSWrite(refnum,&count,line);
if(io != noErr) {
	TellError(81,io);
	 return(ABORT);
	}
#else
	// A temporary console version of file output until I decide how to rewrite
	// the interface to support both Mac and ANSI file refs. - akozar 20130903
	// 'refnum' is (ab)used to specify the output destination (see ConsoleMessages.h)
	BPPrintMessage((int)refnum, line);
#endif /* BP_CARBON_GUI_FORGET_THIS */

return(OK);
}

#if BP_CARBON_GUI_FORGET_THIS

/* FIXME ? CheckTextSize() is generally called after TextInsert() to see if the
   TextEdit buffer was overrun.  Should modify this (if we want to keep TE) so 
   that the check can occur before the TextInsert(). - akozar 20130903 */
int CheckTextSize(int w)
{
long n;

if(WASTE_FORGET_THIS || w < 0 || w >= WMAX || !Editable[w]) return(OK);
n = GetTextLength(w);
if(n > (TEXTEDIT_MAXCHARS - 100)) {
	if (WindowFullAlertLevel[w] < 1) {	// this test cannot be in the previous 'if'
		if(FileName[w][0] != '\0')
			my_sprintf(Message,"Window '%s' is almost full",FileName[w]);
		else
			my_sprintf(Message,"Window '%s' is almost full",WindowName[w]);
		Alert1(Message);
		WindowFullAlertLevel[w] = 1;	// 1 means we've warned about being almost full
		}
	}
else if (n < 0) {
	my_sprintf(Message, "Text has overflowed the '%s' window! Save your work and quit...", WindowName[w]);
	Alert1(Message);
	EmergencyExit = TRUE;
	WindowFullAlertLevel[w] = 3;	// 3 means overflowed
	return(MISSED);
	}
else WindowFullAlertLevel[w] = 0;	// reset if in normal bounds

return(OK);
}


OSErr MyOpen(FSSpec *p_spec,char perm,short *p_refnum)
// Open a file via its chain of aliases if necessary
{
OSErr io;
int targetIsFolder,wasAliased;

(*p_refnum) = -1;	// initialize fRefNum

io = ResolveAliasFile(p_spec,TRUE,&targetIsFolder,&wasAliased);
if (io != noErr) return (io);

if(targetIsFolder)
	io = paramErr;	// cannot open a folder
else
	if(io == noErr)	io = FSpOpenDF(p_spec,perm,p_refnum);
if(io == opWrErr)
	/* Oops! The file is already opened in write mode. Just reposition to the beginning */
	io = SetFPos(*p_refnum,fsFromStart,ZERO);
if(io == noErr) {
	LastVref = p_spec->vRefNum;
	LastDir = p_spec->parID;
	}
return(io);
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

int CleanLF(char** p_buffer,long* p_count,int* p_dos)
// Remove line feeds from buffer and transcode high ASCII so that
// DOS files may be read
// This should be rewritten using a simple preg_replace()!
{
int i,j;
char c;

if(!*p_dos) {
	if((*p_buffer)[0] == '\n') *p_dos = TRUE;
	else {
		for(i=0; i < ((*p_count) - 1); i++) {
			if((*p_buffer)[i] == '\r') {
				if((*p_buffer)[i+1] == '\n') {
					*p_dos = TRUE; break;
					}
				else {
		//			BPPrintMessage(odInfo,"NOT DOS\n");
					return(OK);	/* Not a DOS file */
					}
				}
			}
		}
	}
if(!*p_dos) {
//	BPPrintMessage(odInfo,"NOT DOS\n");
	return(OK);
	}

for(i=j=0; ; i++) {
	if(i >= *p_count) break;
	while((c=(*p_buffer)[i+j]) == '\n' && (i == 0 || (*p_buffer)[i+j-1] == '\r')) {
	//	BPPrintMessage(odInfo,"i = %d, j = %d, count = %d\n",i,j,*p_count);
		j++; (*p_count)--;
		}
	// DOStoMac(&c); Fixed by BB 2022-02-18
	(*p_buffer)[i] = c;
	}
return(OK);
}

#if BP_CARBON_GUI_FORGET_THIS

int OpenHelp(void)
{
OSErr io;
int type,r;
FSSpec spec;
char line[MAXLIN];

if(HelpRefnum != -1) return(OK);	/* already open */
strcpy(line, "BP2 help");
c2pstrcpy(spec.name, line);
spec.vRefNum = RefNumbp2;
spec.parID = ParIDbp2;
type = gFileType[wHelp];
if((io=MyOpen(&spec,fsRdPerm,&HelpRefnum)) != noErr) {
	if((r=CheckFileName(wHelp,line,&spec,&HelpRefnum,type,TRUE)) != OK) {
		HelpRefnum = -1;
		return(r);
		}
	/*else {  // suppressed since can impair finding other files - akozar 040907
		RefNumbp2 = spec.vRefNum;
		ParIDbp2 = spec.parID;
		}*/
	}
return(OK);
}


/* Tries to create a file in the temporary directory returned by FindFolder() or
   in the BP2 application folder if FindFolder() fails.  You only need to provide
   a pascal string filename and space for spec and filerefnum which are returned 
   to the caller */
int CreateTemporaryFile(FSSpecPtr spec, short *filerefnum, StringPtr filename, int deleteIfExists)
{
OSErr err;
int rep, namecount;
NSWReply reply;
short refnum, vrefnum;
long parid;

rep = OK;
err = NSWInitReply(&reply);


// find the temporary folder on the System disk
err = FindFolder(kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &vrefnum, &parid);
if(err != noErr) {
	// try to use the application's folder (FIXME ? not a great choice on OS X)
	vrefnum = RefNumbp2;
	parid = ParIDbp2;
	}
	
err = FSMakeFSSpec(vrefnum, parid, filename, &reply.sfFile);

if (err == noErr && deleteIfExists)	{	// file exists
	err = FSpDelete(&reply.sfFile);	// try to delete it
	if (err != noErr) {
		if(Beta) {
			p2cstrcpy(LineBuff, filename);
			my_sprintf(Message, "Can't delete temporary file '%s'", LineBuff);
			Alert1(Message);
			}
		}
	else err = fnfErr;
	}
if (rep == OK && err == fnfErr) {		// FSSpec is good
	reply.sfReplacing = FALSE;
	CopyPString(filename,PascalLine);
	rep = CreateFile(wUnknown,wUnknown,ftiText,PascalLine,&reply,&refnum);
	if(rep == OK) {
		*filerefnum = refnum;
		*spec = reply.sfFile;
		}
	else {
		if(Beta) {
			p2cstrcpy(LineBuff, filename);
			my_sprintf(Message, "Can't create temporary file '%s'", LineBuff);
			Alert1(Message);
			}
		return(ABORT);
		}
	}
else rep = ABORT;
return(rep);
}


int OpenTemp(void)
{
if(TempRefnum != -1) {
	if(Beta) Alert1("=> Err. OpenTemp(). TempRefnum != -1");
	return(OK);
	}
return CreateTemporaryFile(&TempSpec, &TempRefnum, kBPTempFile, TRUE);
}


int OpenTrace(void)
{
FSSpec tracespec;	// not saved

if(TraceRefnum != -1) {
	if(Beta) Alert1("=> Err. OpenTrace(). TraceRefnum != -1");
	return(OK);
	}
return CreateTemporaryFile(&tracespec, &TraceRefnum, kBPTraceFile, TRUE);
}


OSErr CloseAndDeleteTemp()
{
	OSErr io;

	io = noErr;
	if(TempRefnum != -1) {
		io = CloseMe(&TempRefnum);
		io = FSpDelete(&TempSpec);
		FlushVol(NULL, TempSpec.vRefNum);
		if(io != noErr && Beta) {
			TellError(11,io);
			Alert1("=> Err. deleting 'BP2.temp'");
		}
	}

	return io;
}

/* Closes an open file that has been written to, flushing the volume.
   Requires only the file refnum. */
OSErr CloseFileAndUpdateVolume(short *p_refnum)
{
	OSErr err;
	short vrefnum;

	if(*p_refnum != -1) {
		err = GetVRefNum(*p_refnum, &vrefnum);
		if (err == noErr) {
			err = CloseMe(p_refnum);
			FlushVol(NULL, vrefnum);
		}
	}	
	return err;
}

/* Closes an open file using the file refnum.
   Call this function for a file that was opened for reading only.
   Use CloseFileAndUpdateVolume() for files opened for writing. */
OSErr CloseMe(short *p_refnum)
{
OSErr io;
	
io = noErr;
if(*p_refnum != -1) {
	io = FSClose(*p_refnum);
	if(io != noErr && Beta) {
		TellError(82,io);
		Alert1("Er. CloseMe()");
		}
	}
*p_refnum = -1;
return(io);
}


int CheckFileName(int w,char *filename,FSSpec *p_spec,short *p_refnum,int type,int openreally)
// The file couldn't be opened.  Try to find its actual name and location
// If openreally is false it means we're just checking, not opening
{
char line2[64],line3[MAXLIN];
int rep,io, memexec;
Str255 fn;

/* Usually, type = gFileType[w] */

FIND:
if(filename[0] != '\0') {
	if(DocumentTypeName[w][0] != '\0' && w != wTrace)
		my_sprintf(line3,"Locate '%s' or other %s file",filename,DocumentTypeName[w]);
	else
		my_sprintf(line3,"Locate '%s'",filename);
	}
else {
	if(DocumentTypeName[w][0] != '\0' && w != wTrace)
		my_sprintf(line3,"Select a(n) %s file",DocumentTypeName[w]);
	else
		my_sprintf(line3,"Select a file");
	}

ShowMessage(TRUE,wMessage,line3);
if(AEventOn && CallUser(1) != OK) return(ABORT);

TRYOPEN:
if(!OldFile(w,type,fn,p_spec)) {
	// HideWindow(Window[wMessage]);
	return(MISSED);
	}
p2cstrcpy(line2,fn);
if(gFileType[w] != ftiAny && gFileType[w] != ftiText && IdentifyBPFileType(p_spec) != w) {
	my_sprintf(Message,"BP2 is not sure that '%s' is a(n) %s file. Do you want to load it anyway",
		line2, DocumentTypeName[w]);
	memexec = ScriptExecOn; ScriptExecOn = 0;
	rep = Answer(Message,'Y'); // default to 'Y' in case script is running
	ScriptExecOn = memexec;
	if (rep == NO) goto TRYOPEN;
	else if (rep == ABORT) return(ABORT);
	}
	
// Strip(filename);
if(filename[0] != '\0') {
	if(strcmp(filename,line2) != 0) {
		// Don't ask since the user already chose the file - akozar, 031907
		/* rep = Answer("Changing file",'N');
		switch(rep) {
			case NO:
				goto FIND;
				break;
			case YES: */
				strcpy(filename,line2);
				if(openreally) {
					strcpy(FileName[w],filename);
					TellOthersMyName(w);
					}
				/* break;
			case ABORT:
				// HideWindow(Window[wMessage]);
				return(MISSED);
			} */
		}
	}
else {
	strcpy(filename,line2);
	if(openreally) {
		strcpy(FileName[w],filename);
/*		TellOthersMyName(w); */
		}
	}
InputOn++;
// HideWindow(Window[wMessage]);
// c2pstrcpy(p_spec->name, line2);
// FIXME ? if (!openreally), should we be calling MyOpen().  If so, how does the
//         file get closed later ?  - akozar
if((io=MyOpen(p_spec,fsCurPerm,p_refnum)) != noErr && io != opWrErr) {
	my_sprintf(Message,"Can't open '%s'",filename);
	Alert1(Message);
	TellError(83,io);
	InputOn--;
	return(ABORT);
	}
if(io == opWrErr) {
	io = SetFPos(*p_refnum,fsFromStart,ZERO);
	if(io != noErr) {
		my_sprintf(Message,"Can't reopen '%s'",filename);
		Alert1(Message);
		TellError(84,io);
		InputOn--;
		return(ABORT);
		}
	}
if(openreally) {
	SetName(w,FALSE,TRUE);
	TheVRefNum[w] = p_spec->vRefNum;
	WindowParID[w] = p_spec->parID;
	}
InputOn--;
return(OK);
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

#if 0
/* FIXME ? Shouldn't we be flushing the vRefNum of the file that was written ?? - akozar */
FlushVolume()
{
IOParam pb;
OSErr io;
int async;

async = FALSE;
/* MacOS bombs if async is true! */

pb.ioCompletion = NULL;
pb.ioNamePtr = NIL;
pb.ioVRefNum = 0;
io = PBFlushVol((ParmBlkPtr)&pb,async);
return(io == noErr);
}
#endif


#if BP_CARBON_GUI_FORGET_THIS
int FlushFile(short refnum)
{
IOParam pb;
OSErr io;
int async;

pb.ioCompletion = NULL;
async = FALSE;
/* MacOS bombs if async is true! */

pb.ioRefNum = refnum;
io = PBFlushFile((ParmBlkPtr)&pb,async);
if(io != noErr) TellError(85,io);
return(io == noErr);
}
#endif /* BP_CARBON_GUI_FORGET_THIS */


int GetThisVersion(int w)
{
int i,j,diff,r,fileversion;
long pos,posho,posmax;
char c,*p,*q,**p_line,version[VERSIONLENGTH];

pos = ZERO;
p_line = NULL;
if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("=> Err. GetThisVersion(). Incorrect window index");
	return(MISSED);
	}
posmax = GetTextLength(w);
r = MISSED;

REDO:
if(ReadLine(NO,w,&pos,posmax,&p_line,&j) != OK) goto SORTIR;
if((*p_line)[0] == '\0') goto REDO;
FindVersion(p_line,version);
diff = TRUE;
for(fileversion = 0; fileversion < MAXVERSION; fileversion++)
	if((diff = strcmp(version,VersionName[fileversion])) == 0) break;
if(diff) {
	fileversion = 2;
	pos = ZERO;
	}
if(fileversion > Version) {
	// It would be unusual for VersionName[fileversion] to exist if fileversion > Version
	my_sprintf(Message,
		"Can't use file version %s\nbecause 'BP2' version is %s.\n",
		VersionName[fileversion],VersionName[Version]);
	if(!ScriptExecOn) Alert1(Message);
	else PrintBehind(wTrace,Message);
	goto SORTIR;
	}
if(fileversion >= 3) {
	/* Delete info and date line */
REDO2:
	if(ReadLine(NO,w,&pos,posmax,&p_line,&j) != OK) goto SORTIR;
	if((*p_line)[0] == '\0') goto REDO2;
	}
SetSelect(ZERO,pos,TEH[w]);
TextDelete(w);
r = OK;

SORTIR:
MyDisposeHandle((Handle*)&p_line);
return(r);
}


int CheckVersion(int *p_iv, char **p_line, const char name[])
{
int diff,rep,iv;
char version[VERSIONLENGTH];

(*p_iv) = 0;
diff = 1;
if(p_line == NULL || (*p_line)[0] == '\0') {
	if(Beta) {
	//	Alert1("=> Err. CheckVersion(). p_line == NULL || (*p_line)[0] == '\0'");
		BPPrintMessage(odError,"=> Error loading this file: version cannot be found. Did you save it?\n");
		}
	return(MISSED);
	}
FindVersion(p_line,version);
for(iv=0; iv < MAXVERSION; iv++)
	if((diff = strcmp(version,VersionName[iv])) == 0) break;
if(iv > Version && name[0] != '\0') {
	BPPrintMessage(odError,"=> File '%s' was created with a version of BP2 more recent than %s\n",name,VersionName[Version]);
/*	my_sprintf(Message,
		"File '%s' was created with a version of BP2 more recent than %s. Try to read it anyway (risky)",
			name,VersionName[Version]);
	rep = Answer(Message,'N'); */
	rep = NO;
	if(rep != YES) goto ERR;
	iv = Version;
	}
(*p_iv) = iv;
return(OK);

ERR:
return(MISSED);
}


int GetFileDate(int w,char ***pp_result)
{
int i,diff,gap,result;
long pos,posmax;
char *p,*q,**p_line;

if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("=> Err. GetFileDate(). w < 0 || w >= WMAX || !Editable[w]");
	return(OK);
	}
pos = ZERO; p_line = NULL;
posmax = GetTextLength(w);
(**pp_result)[0] = '\0';
if(ReadLine(NO,w,&pos,posmax,&p_line,&gap) != OK) return(OK);
pos = ZERO;
while((result=ReadLine(NO,w,&pos,posmax,&p_line,&gap)) == OK) {
	if(GetDateSaved(p_line,pp_result) == OK) break;
	}
MyDisposeHandle((Handle*)&p_line);
return(OK);
}


int GetDateSaved(char **p_line,char ***pp_result)
{
char c,*p,*q;
int i0,offset;

i0 = strlen(DateMark);
MyLock(FALSE,(Handle)p_line);
p = strstr(*p_line,DateMark); q = DateMark;
if(p != NULLSTR && Match(FALSE,&p,&q,i0)) {
	if(p != (*p_line)) {
		p--;
		c = *p;
		*p = '\0';
		offset = 1 + strlen(*p_line);
		}
	else offset = 0;
	MystrcpyHandleToHandle(offset,pp_result,p_line);
	if(offset > 0) *p = c;	/* fixed 11/3/99 */
	MyUnlock((Handle)p_line);
	return(OK);
	}
MyUnlock((Handle)p_line);
return(MISSED);
}

#if BP_CARBON_GUI_FORGET_THIS

int WriteHeader(int w,short refnum,FSSpec spec)
{
char line[MAXLIN],name[64],**p_line;
long count;

if(w >= WMAX || (w >= 0 && !Editable[w] && !HasFields[w] && w != iSettings)) {
	if(Beta) Alert1("=> Err. WriteHeader(). w >= WMAX || (!Editable[w] && !HasFields[w])");
	}
if(refnum == -1) {
	if(Beta) Alert1("=> Err. WriteHeader(). refnum == -1");
	return(MISSED);
	}
MyPtoCstr(MAXNAME,spec.name,name);
if(w >= 0 && IsHTML[w]) {
	my_sprintf(line,"<HTML><HEAD><TITLE>%s</TITLE>",name);
	WriteToFile(NO,DOS,line,refnum);
	my_sprintf(line,"<META HTTP-EQUIV=\"content-type\" CONTENT=\"text/html;charset=iso-8859-1\">");
	WriteToFile(NO,DOS,line,refnum);
	my_sprintf(line,"<META NAME=\"generator\" CONTENT=\"Bol Processor BP2\">");
	WriteToFile(NO,DOS,line,refnum);
	my_sprintf(line,"<META NAME=\"keywords\" CONTENT=\"computer music, Bol Processor, BP2\">");
	WriteToFile(NO,DOS,line,refnum);
	my_sprintf(line,"</HEAD><BODY BGCOLOR=\"White\">");
	WriteToFile(NO,DOS,line,refnum);
	}
switch(w) {
	case wScrap:
	case wHelp:
	case wNotice:
		return(OK);
		break;
	}
if((p_line = (char**) GiveSpace((Size)(MAXLIN * sizeof(char)))) == NULL)
	return(ABORT);
my_sprintf(line,"// Bol Processor version %s",VersionName[Version]);
if(w >= 0 && IsHTML[w]) {
	strcat(line,"<BR>");
	WriteToFile(NO,DOS,line,refnum);
	}
else WriteToFile(NO,MAC,line,refnum);
Date(line);

if(w >= 0)
	my_sprintf(Message,"// %s file saved as '%s'. %s",WindowName[w],name,line);
else
	my_sprintf(Message,"// File saved as '%s'. %s",name,line);
	
if(w >= 0 && Editable[w] && IsHTML[w]) {
	MystrcpyStringToHandle(&p_line,Message);
	MacToHTML(YES,&p_line,NO);
	MystrcpyHandleToString(MAXLIN,0,Message,p_line);
	strcat(Message,"<BR>");
	WriteToFile(NO,DOS,Message,refnum);
	}
else WriteToFile(NO,MAC,Message,refnum);

MyDisposeHandle((Handle*)&p_line);

return(OK);
}


int WriteEnd(int w,short refnum)
{
char line[MAXLIN],name[MAXNAME+1],**p_line;
long count;

if(refnum == -1) {
	if(Beta) Alert1("=> Err. WriteEnd(). refnum == -1");
	return(MISSED);
	}
if(w >= 0 && IsHTML[w]) {
	WriteToFile(NO,DOS,"\r\n<HR>\r\n</BODY>\r\n</HTML>",refnum); // FIXME? Will this write CR-CR-LF on Windows?
	}
else NoReturnWriteToFile("\0",refnum);
return(OK);
}


int GetHeader(int w)
{
if(!Editable[w]) return(OK);
switch(w) {
	case wScrap:
	case wHelp:
	case wNotice:
		break;
	case wStartString:
	case wGrammar:
	case wAlphabet:
	case wScript:
	case wInteraction:
	case wGlossary:
	case wData:
	case wPrototype7:
	case wTrace:
		GetFileDate(w,&(p_FileInfo[w]));
		GetThisVersion(w);
		break;
	default:
		return(OK);
	}
UpdateDirty(TRUE,w);
Dirty[w] /* = Created[w] */ = FALSE;
#if WASTE_FORGET_THIS
WEResetModCount(TEH[w]);
#endif
return(OK);
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

int FindVersion(char **p_line,char* version)
{
char c,*p;
int i;

if(p_line == NULL || (*p_line)[0] == '\0') return(MISSED);
StripHandle(p_line);
MyLock(FALSE,(Handle)p_line);
p = strstr((*p_line),"version");
if(p == NULLSTR) p = (*p_line);
else {
	p += strlen("version");
	while(MySpace(c=(*p))) p++;
	}
i = 0;
while(!isspace(c=(*p))) {	/* Fixed 24/2/99 */
	if(i >= (VERSIONLENGTH-1)) break;
	version[i] = c; p++; i++;
	}
version[i] = '\0';
MyUnlock((Handle)p_line);
Strip(version);
return(OK);
}

#if BP_CARBON_GUI_FORGET_THIS

OSErr MyFSClose(int w,short refnum,FSSpec *p_spec)
{
OSErr io;
FSSpec spec;

io = FSClose(refnum);
if(io == noErr && w >= 0 && w < WMAX) {
	spec = (*p_TempFSspec)[w];
	if(spec.name[0] != 0) {
		io = FSpExchangeFiles(&spec,p_spec);
		if(io == noErr) io = FSpDelete(&spec);
		}
	(*p_TempFSspec)[w].name[0] = 0;
	io = FlushVol(NULL, p_spec->vRefNum);
	}
if(io != noErr) TellError(86,io);
return(io);
}

/* PutFile event filter procedure - customized for Bol Processor. */
pascal void PutFileEventProc(NavEventCallbackMessage callBackSelector,
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
			{ char str[100];
			
			// save the user's selection of our custom format options
			item = (NavMenuItemSpec*) callBackParms->eventData.eventDataParms.param;
			reply = (NSWReply*) callBackUD;
			p2cstrcpy(str, item->menuItemName);
			if (strcmp(str, FormatNames[18]) == 0) {		// HTML
				reply->isHTML = TRUE; reply->isText = TRUE;
			}
			else if (strcmp(str, FormatNames[1]) == 0) {	// plain text
				reply->isHTML = FALSE; reply->isText = TRUE;
			}
			else if (strstr(str, HTMLFormat) != NULL)	{	// BP2 HTML
				reply->isHTML = TRUE; reply->isText = FALSE;
			}
			else {							// BP2 native
				reply->isHTML = FALSE; reply->isText = FALSE;
			}
			}
			break;
		default:
			break;
	}
}

/*----------------------------------------------------------------------------

	The next two functions (CopyOneFork & CopyFile) are by John Norstad
	and are freely reusable and redistributable with his permission.
	(I have modified them a little to not rely on his other utility files).
	
	fileutil.c   (from Norstad's Reusables)
	
	Copyright � 1994-1995, Northwestern University.

----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
	CopyOneFork
	
	Copy one fork of a file.
	
	Entry:	source = pointer to source file spec.
			dest = pointer to destination file spec.
			resourceFork = true to copy resource fork, false to copy
				data fork.
	
	Exit:	function result = error code.
----------------------------------------------------------------------------*/

static OSErr CopyOneFork (FSSpec *source, FSSpec *dest, int resourceFork)
{
	const Size bufferSize = 1024;
	short sourceRefNum = 0;
	short destRefNum = 0;
	long fileSize, len;
	Ptr buf;
	FInfo fInfo;
	OSErr err = noErr;
	
	/* Open source fork. */
	
	if (resourceFork) {
		err = FSpOpenRF(source, fsRdPerm, &sourceRefNum);
	} else {
		err = FSpOpenDF(source, fsRdPerm, &sourceRefNum);
	}
	if (err == fnfErr) return noErr;
	if (err != noErr) goto exit;
	err = GetEOF(sourceRefNum, &fileSize);
	if (err != noErr) goto exit;
	
	/* Open destination fork. Create the fork if it is missing. */
	
	if (resourceFork) {
		err = FSpOpenRF(dest, fsRdWrPerm, &destRefNum);
	} else {
		err = FSpOpenDF(dest, fsRdWrPerm, &destRefNum);
	}
	if (err == fnfErr) {
		err = FSpGetFInfo(source, &fInfo);
		if (err != noErr) goto exit;
		if (resourceFork) {
			FSpCreateResFile(dest, fInfo.fdCreator, fInfo.fdType, smSystemScript);
			err = ResError();
		} else {
			err = FSpCreate(dest, fInfo.fdCreator, fInfo.fdType, smSystemScript);
		}
		if (err != noErr) goto exit;
		if (resourceFork) {
			err = FSpOpenRF(dest, fsRdWrPerm, &destRefNum);
		} else {
			err = FSpOpenDF(dest, fsRdWrPerm, &destRefNum);
		}
	}
	if (err != noErr) goto exit;
	err = SetFPos(destRefNum, fsFromStart, 0);
	if (err != noErr) goto exit;
	
	/* Copy the source fork to the destination fork. */
	buf = NewPtr(bufferSize);
	err = MemError();
	if (err != noErr) goto exit;
	while(fileSize > 0) {
		len = fileSize > bufferSize ? bufferSize : fileSize;
		err = FSRead(sourceRefNum, &len, buf);
		if (err != noErr) goto exit;
		err = FSWrite(destRefNum, &len, buf);
		if (err != noErr) goto exit;
		fileSize -= len;
	}
	
exit:

	if (sourceRefNum != 0) err = FSClose(sourceRefNum);
	if (destRefNum != 0) {
		FSClose(destRefNum);
		err = FlushVol(NULL, dest->vRefNum);
	}
	if (buf != nil) DisposePtr(buf);
	return err;
}



/*----------------------------------------------------------------------------
	CopyFile
	
	Make a copy of a file (both forks).
	
	Entry:	source = pointer to source file spec.
			dest = pointer to destination file spec.
	
	Exit:	function result = error code.
----------------------------------------------------------------------------*/

OSErr CopyFile (FSSpec *source, FSSpec *dest)
{
	OSErr err = noErr;

	err = CopyOneFork(source, dest, true);
	if (err != noErr) return err;
	return CopyOneFork(source, dest, false);
}

#endif /* BP_CARBON_GUI_FORGET_THIS */
