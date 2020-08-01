/* AppleEvents.c (BP2 version CVS) */

/*  This file is a part of Bol Processor 2
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


#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"


DoHighLevelEvent(EventRecord *p_event)
{
OSErr io;

PrintEvent(p_event, "DoHighLevelEvent()", NULL);
if(GoodEvent(p_event)) {
	AEventOn++;
	io = AEProcessAppleEvent(p_event);
	if(EventState == EXIT) return(OK);
	if(AEventOn > 0) AEventOn--;
	// if(io != noErr) TellError(38,io); // it's ok if the event was not handled - akozar
	}
// else return(HandleMySpecialHLEvent(p_event));
return(OK);
}


GoodEvent(EventRecord *p_event)
{
/* must process all events whether we recognize them or not. This is necessary
   for scripting additions to work and notably Default Folder - akozar */
/* switch(p_event->message) {
	case BP2Class:
	case kCoreEventClass:
#if !TARGET_API_MAC_CARBON	/* Edition Manager not in Carbon *\/
	case sectionEventMsgClass:
#endif
		break;
	default: return(NO);
	}
switch((*((AEEventID*) (&(p_event->where))))) {
	case kAEOpenDocuments:
	case kAEPrintDocuments:
	case kAEQuitApplication:
#if !TARGET_API_MAC_CARBON	/* Edition Manager not in Carbon *\/
	case sectionReadMsgID:
	case sectionWriteMsgID:
	case sectionScrollMsgID:
#endif
	case PlayEventID:
	case ScriptLineEventID:
	case LoadSettingsEventID:
	case NoteConventionEventID:
	case BeepID:
	case AbortID:
	case PauseID:
	case AgainID:
	case SkipID:
	case QuickID:
	case ResumeID:
	case ImprovizeID:
	case DoScriptID:
	case NameID:
	case GrammarID:
	case AlphabetID:
	case GlossaryID:
	case InteractionID:
	case DataID:
	case ScriptID:
	case CsoundInstrID:
		break;
	case kAEOpenApplication:
		if(InitOn || !ReceivedOpenAppEvent) {
			ReceivedOpenAppEvent = TRUE;
			return(NO);
			}
		break;
	default: return(NO);
	}
 */
return(YES);
}

#if 0
HandleMySpecialHLEvent(EventRecord *p_event)
{

return(OK);
}
#endif


OSErr MyGotRequiredParams(const AppleEvent *p_event)
{
DescType returnedType;
OSErr io;
Size size;

size = ZERO;
io = AEGetAttributePtr(p_event,keyMissedKeywordAttr,typeWildCard,&returnedType,
    NULL,0,&size);
if(io == errAEDescNotFound) return(noErr);
if(io == noErr) return(errAEParamMissed);
else return(io);
}


pascal OSErr MyHandleODOC(const AppleEvent *p_event, AppleEvent *p_reply,long handlerRefcon)
{
FSSpec spec;
AEDescList	docList;
OSErr err,io;
char *p,*q;
int wind,print,longerCsound,oms,failedonce;
long index,itemsInList;
short refnum;
Size actualSize;
AEKeyword keywd;
DescType returnedType;
char name[64],loaded[WMAX];	// FSSpec.name is a Str63
AEEventID theID;

// check whether the Apple Event requested to open or to print
err = AEGetAttributePtr(p_event,keyEventIDAttr,typeWildCard,&returnedType,&theID,
	sizeof(AEEventID),&actualSize);
if(theID == kAEPrintDocuments) print = TRUE;
else print = FALSE;

/* sprintf(Message,"Open, print option = %ld\n",(long)print);
PrintBehind(wTrace,Message); */

// get the direct parameter--a descriptor list--and put it into a docList
err = AEGetParamDesc(p_event,keyDirectObject,typeAEList,&docList);
if(err) return err;

// check for missing parameters
err = MyGotRequiredParams(p_event);
if(err) goto OUT;

// count the number of descriptor records in the list
err = AECountItems(&docList,&itemsInList);

sprintf(Message,"Opening %ld file(s)...",(long)itemsInList);
/* PrintBehindln(wTrace,Message); */
if(ShowMessages) ShowMessage(TRUE,wMessage,Message);

for(wind=0; wind < WMAX; wind++) loaded[wind] = FALSE;

// Now get each descriptor record from the list, coerce the returned
// data to an FSSpec record, and open the associated file
for(index=1,failedonce=loaded[iSettings]=FALSE; index <= itemsInList;) {
	err = AEGetNthPtr(&docList,index,typeFSS,&keywd,&returnedType,(Ptr)&spec,
		sizeof(spec),&actualSize);
	if(err) goto OUT;
	p2cstrcpy(name, spec.name);	// shouldn't use MyPtoCstr(), it converts whitespace characters
	Strip(name);  			// FIXME:  filenames can begin or end with spaces! (but currently cause problems for BP2)
	if(name[0] == '\0') goto NEWINDEX;
	/* if(name[0] != '-'&& name[0] != '+') {	
		sprintf(Message,"File '%s' doesn't have a name that BP2 would recognize...\n",name);
		Print(wTrace,Message);
		DisplayHelp("Types-creators");
		failedonce = TRUE;
		goto NEWINDEX;
		} */
	
	wind = IdentifyBPFileType(&spec);
	if(wind != wUnknown) {
		// don't reload an already open document (FIXME: should compare FSSpecs, not just names)
		if(wind != iMidiDriver && strcmp(FileName[wind],name) == 0) {
			if(print && Editable[wind]) mPrint(wind);
			goto NEWINDEX;
			}
		if(loaded[wind]) {
			sprintf(Message,"BP2 can't open several files of the same type. '%s' was ignored...",name);
			Println(wTrace,Message);
			failedonce = TRUE;
			goto NEWINDEX;
			}
		switch(wind) {
			case wGrammar:
			case wAlphabet:
			case iSettings:
			case wInteraction:
			case wData:
			case wTrace:
			case wScrap:	// added akozar 031407
			case wScript:
			case wGlossary:
			case iObjects:
			case wKeyboard:
			case wTimeBase:
			case wCsoundInstruments:
			case wMIDIorchestra:
			case iMidiDriver:
				break;
			default:
				sprintf(Message,"BP2 can't open file '%s' from the Finder...",name);
				Println(wTrace,Message);
				failedonce = TRUE;
				goto NEWINDEX;
			break;
			}
		if(ClearWindow(FALSE,wind) != OK) goto NEWINDEX;
		sprintf(Message,"Opening '%s'...",name);
		ShowMessage(TRUE,wMessage,Message);
		if (wind != iMidiDriver) { // don't change stored FSSpec for iMidiDriver until the file is successfully opened
						   // eventually, I would like for all loads to work this way -- akozar
			TheVRefNum[wind] = spec.vRefNum;
			WindowParID[wind] = spec.parID;
			strcpy(FileName[wind],name);
			}
		loaded[wind] = TRUE;
		if(Editable[wind]) LastEditWindow = wind;
		if(wind == iSettings) {
			LoadSettings(TRUE,TRUE,FALSE,FALSE,&oms);
			TellOthersMyName(iSettings);
			Created[iSettings] = TRUE; // 020907 akozar
			}
		else if(wind == iMidiDriver) {
			LoadMidiDriverSettings(&spec);  // this handles all necessary changes to globals
			// Created[iMidiDriver] = TRUE; // not needed as we don't ever Save to the stored FSSpec
			}
		else if(wind == wGlossary) {
			LoadGlossary(FALSE,FALSE);
			BPActivateWindow(SLOW,wind);
			if(print) mPrint(wind);
			Created[wGlossary] = TRUE; // 020907 akozar
			}
		else if(wind == wInteraction) {
			Interactive = TRUE; SetButtons(TRUE);
			LoadInteraction(FALSE,FALSE);
			BPActivateWindow(SLOW,wind);
			if(print) mPrint(wind);
			Created[wInteraction] = TRUE; // 020907 akozar
			}
		else if(wind == iObjects) {
			if(CompileCheck() != OK) break;
			if(LoadObjectPrototypes(YES,YES) != OK) {
				ObjectMode = ObjectTry = FALSE;
				FileName[iObjects][0] = '\0';
				SetName(iObjects,TRUE,TRUE);
				Dirty[iObjects] = Created[iObjects] = FALSE;
				iProto = 0;
				}
			else {
				NeedAlphabet = TRUE;    // Fixed by BB 28 May 2007
				iProto = 2;
				BPActivateWindow(SLOW,wPrototype1);
				SetPrototype(iProto);
				SetCsoundScore(iProto);
				CompileObjectScore(iProto,&longerCsound);
				Created[iObjects] = TRUE; // 020907 akozar
				}
			}
		else if((wind == wKeyboard || wind == wTimeBase || wind == wCsoundInstruments
				|| wind == wMIDIorchestra) && MyOpen(&spec,fsCurPerm,&refnum) == noErr) {
			switch(wind) {
				case wKeyboard:
					LoadKeyboard(refnum);
					Token = TRUE; MaintainMenus();
					mKeyboard(wind);
					break;
				case wTimeBase:
					LoadTimeBase(refnum);
					mTimeBase(wind);
					break;
				case wCsoundInstruments:
					LoadCsoundInstruments(refnum,FALSE);
					mCsoundInstrumentsSpecs(wind);
					break;
				case wMIDIorchestra:
					LoadMIDIorchestra(refnum,FALSE);
					
				//	ShowWindow(MIDIprogramPtr);
				//	SelectWindow(MIDIprogramPtr);
				//	BPUpdateDialog(MIDIprogramPtr); /* Needed to make static text visible */
					
					HideWindow(Window[wMessage]);
					
					ShowWindow(GetDialogWindow(SixteenPtr));
					SelectWindow(GetDialogWindow(SixteenPtr));
					BPUpdateDialog(SixteenPtr); /* Needed to make static text visible */
					BPActivateWindow(SLOW,wind);
					break;
				}
			Created[wind] = TRUE;  // changed from false - 020907 akozar
			SetName(wind,TRUE,TRUE);
			}
		else if(((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) && 
			  (ReadFile(wind,refnum) == OK && (FSClose(refnum) == noErr))) {
			ShowWindow(Window[wind]);
			Created[wind] = TRUE;  // changed from false - 020907 akozar
			SetName(wind,FALSE,TRUE);
			UpdateDirty(TRUE,wind);
			Dirty[wind] = FALSE;
#if WASTE
			WEResetModCount(TEH[wind]);
#endif
			GetHeader(wind);
			if(wind == wScript) {
				LoadedScript = TRUE;
				}
			if(wind == wAlphabet) {
				TheVRefNum[iObjects] = TheVRefNum[wKeyboard]
					= TheVRefNum[wCsoundInstruments]
					= TheVRefNum[wMIDIorchestra] = spec.vRefNum;
				WindowParID[iObjects] = WindowParID[wKeyboard]
					= WindowParID[wCsoundInstruments]
					= WindowParID[wMIDIorchestra] = spec.parID;
				GetMiName(); GetKbName(wAlphabet);
				GetCsName(wAlphabet);
				GetFileNameAndLoadIt(wMIDIorchestra,wind,LoadMIDIorchestra);
				}
			if(wind == wGrammar) {
				TheVRefNum[wCsoundInstruments] = TheVRefNum[iObjects] = TheVRefNum[wKeyboard]
					= TheVRefNum[iWeights] = TheVRefNum[wInteraction]
					= TheVRefNum[wGlossary] = TheVRefNum[iSettings]
					= TheVRefNum[wAlphabet] = TheVRefNum[wTimeBase]
					= TheVRefNum[wMIDIorchestra] = spec.vRefNum;
				WindowParID[wCsoundInstruments] = WindowParID[iObjects] = WindowParID[wKeyboard]
					= WindowParID[iWeights] = WindowParID[wInteraction]
					= WindowParID[wGlossary] = WindowParID[iSettings]
					= WindowParID[wAlphabet] = WindowParID[wTimeBase]
					= WindowParID[wMIDIorchestra] = spec.parID;
				if(loaded[iSettings]) TellOthersMyName(iSettings);
				else if(GetSeName(wGrammar) == OK) LoadSettings(TRUE,TRUE,FALSE,FALSE,&oms);
				if(loaded[iMidiDriver]) TellOthersMyName(iMidiDriver);
				else LoadLinkedMidiDriverSettings(wGrammar);
				GetTimeBaseName(wGrammar);
				GetCsName(wGrammar);
				GetFileNameAndLoadIt(wMIDIorchestra,wind,LoadMIDIorchestra);
				LoadAlphabet(wGrammar,&spec);
				}
			if(wind == wData) {
				TheVRefNum[iObjects] = TheVRefNum[wKeyboard]
					= TheVRefNum[wInteraction] = TheVRefNum[wGlossary]
					= TheVRefNum[iSettings] = TheVRefNum[wAlphabet]
					= TheVRefNum[wTimeBase] = TheVRefNum[wCsoundInstruments]
					= TheVRefNum[wMIDIorchestra] = spec.vRefNum;
				WindowParID[iObjects] = WindowParID[wKeyboard]
					= WindowParID[wInteraction] = WindowParID[wGlossary]
					= WindowParID[iSettings] = WindowParID[wAlphabet]
					= WindowParID[wTimeBase] = WindowParID[wCsoundInstruments]
					= WindowParID[wMIDIorchestra] = spec.parID;
				if(loaded[iSettings]) TellOthersMyName(iSettings);
				else if(GetSeName(wData) == OK) LoadSettings(TRUE,TRUE,FALSE,FALSE,&oms);
				if(loaded[iMidiDriver]) TellOthersMyName(iMidiDriver);
				else LoadLinkedMidiDriverSettings(wData);
				GetTimeBaseName(wData);
				GetCsName(wData);
				GetFileNameAndLoadIt(wMIDIorchestra,wind,LoadMIDIorchestra);
				LoadAlphabet(wData,&spec);
				}
			BPActivateWindow(SLOW,wind);
			if(print) mPrint(wind);
			}
		else {
			TellError(39,io);
			sprintf(Message,"BP2 was unable to open '%s'... [Error code %ld]\n",
				name,(long)io);
			Println(wTrace,Message);
			failedonce = TRUE;
			FileName[wind][0] = '\0';
			}
		Dirty[wind] = FALSE;
		}
	else {
		sprintf(Message,"Dragging '%s' to BP2 had no effect...\n",name);
		ShowMessage(TRUE,wMessage,Message);
		// failedonce = TRUE;
		}
NEWINDEX:
	index++;
	}
OUT:
if(failedonce) {
	BPActivateWindow(SLOW,wTrace);
	ShowSelect(CENTRE,wTrace);
	}
err = AEDisposeDesc(&docList);
return err;
}


pascal OSErr MyHandleOAPP(const AppleEvent *p_event, AppleEvent *p_reply,long handlerRefcon)
{
ReceivedOpenAppEvent = TRUE;
return(noErr);
}



pascal OSErr MyHandleQUIT(const AppleEvent *p_event, AppleEvent *p_reply,long handlerRefcon)
{
EventState = EXIT;
ShowMessage(TRUE,wMessage,"Received Apple Event of class 'aevt' ID 'quit'. Quitting BP2...");
return(noErr);
}


pascal OSErr MyHandleSectionReadEvent(const AppleEvent *p_event,AppleEvent *p_reply,long handlerRefcon)
{
return(noErr);
}


pascal OSErr MyHandleSectionWriteEvent(const AppleEvent *p_event,AppleEvent *p_reply,long handlerRefcon)
{
return(noErr);
}


pascal OSErr MyHandleSectionScrollEvent(const AppleEvent *p_event,AppleEvent *p_reply,long handlerRefcon)
{
return(noErr);
}


pascal OSErr RemoteDoScriptLine(const AppleEvent *p_event,AppleEvent *p_reply,
	long handlerRefcon)
{
OSErr io;
char c,**h_text;
long count,posdir,posline;
int rep,changed,keep;
Size size;
DescType returnedType;
AEKeyword keywd;
AEDescList docList;

posline = ZERO; posdir = -1L; changed = NO; keep = TRUE;
io = AEGetParamDesc(p_event,keyDirectObject,typeAEList,&docList);
if(io != noErr) return(io);
io = MyGotRequiredParams(p_event);
if(io != noErr) {
	TellError(40,io);
	return(AEDisposeDesc(&docList));
	}
io = AECountItems(&docList,&count);
if(io != noErr) {
	TellError(41,io);
	return(AEDisposeDesc(&docList));
	}
if((h_text = (char**) GiveSpace((Size)(32768 * sizeof(char)))) == NULL) {
	return(AEDisposeDesc(&docList));
	}
/* count = 1 in this type of message */
/*	io = AEGetNthDesc(&docList,i,desiredType,&keywd,&desc); */
MyLock(FALSE,(Handle)h_text);
io = AEGetNthPtr(&docList,1L,'TEXT',&keywd,&returnedType,(Ptr)*h_text,32768,&size);
MyUnlock((Handle)h_text);
if(io != noErr) {
	Println(wTrace,"\nData of 'scln' Apple Event is missing. Check client application!");
	goto OUT;
	}
io = AEDisposeDesc(&docList);
if(io != noErr) goto OUT;

if(size > 0) {
	/* Beware that this handle does not terminate with '\0' */
	(*h_text)[size] = '\0';
	ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'scln' along with script line");
	ScriptExecOn++;
	rep = ExecScriptLine(NULL,wScript,FALSE,FALSE,h_text,posline,&posdir,&changed,&keep);
	EndScript();
	if(rep == EXIT) io = dsForcedQuit;
	}

OUT:
MyDisposeHandle((Handle*)&h_text);
return(io);
}


pascal OSErr RemoteLoadSettings(const AppleEvent *p_event,AppleEvent *p_reply,long handlerRefcon)
{
OSErr io;
char **h_text;
long i,x,count;
int startup,oms;
Size size;
DescType returnedType;
AEKeyword keywd;
AEDescList docList;

io = AEGetParamDesc(p_event,keyDirectObject,typeAEList,&docList);
if(io != noErr) return(io);
io = MyGotRequiredParams(p_event);
if(io != noErr) {
	TellError(42,io);
	return(AEDisposeDesc(&docList));
	}
io = AECountItems(&docList,&count);
if(io != noErr) {
	TellError(43,io);
	return(AEDisposeDesc(&docList));
	}
if((h_text = (char**) GiveSpace((Size)(32768 * sizeof(char)))) == NULL) {
	return(AEDisposeDesc(&docList));
	}
MyLock(FALSE,(Handle)h_text);
io = AEGetNthPtr(&docList,1L,'TEXT',&keywd,&returnedType,(Ptr)*h_text,32768,&size);
MyUnlock((Handle)h_text);
if(io != noErr) {
	Alert1("Data of 'sett' Apple Event is missing. Check client application!");
	goto OUT;
	}
io = AEDisposeDesc(&docList);
if(io != noErr) goto OUT;

if(size > 0) {
	/* Beware that this handle does not terminate with '\0' */
	for(i=0; i < size; i++) Message[i] = (*h_text)[i];
	Message[size] = '\0';
	Strip(Message);
	if(Message[0] != '\0' && strcmp(Message,FileName[iSettings]) != 0) {
		sprintf(LineBuff,"%sstartup",FilePrefix[iSettings]);
		if(strcmp(LineBuff,Message) == 0) startup = TRUE;
		else {
			startup = FALSE;
			strcpy(FileName[iSettings],Message);
			}
		LoadSettings(TRUE,YES,startup,NO,&oms);
		}
	}

OUT:
MyDisposeHandle((Handle*)&h_text);
return(io);
}


pascal OSErr RemoteLoadCsoundInstruments(const AppleEvent *p_event,AppleEvent *p_reply,long handlerRefcon)
{
OSErr io;
char **h_text;
long i,count;
int type;
Size size;
DescType returnedType;
AEKeyword keywd;
AEDescList docList;
short refnum;
FSSpec spec;

io = AEGetParamDesc(p_event,keyDirectObject,typeAEList,&docList);
if(io != noErr) return(io);
io = MyGotRequiredParams(p_event);
if(io != noErr) {
	TellError(44,io);
	return(AEDisposeDesc(&docList));
	}
io = AECountItems(&docList,&count);
if(io != noErr) {
	TellError(45,io);
	return(AEDisposeDesc(&docList));
	}
if((h_text = (char**) GiveSpace((Size)(32768 * sizeof(char)))) == NULL) {
	return(AEDisposeDesc(&docList));
	}
MyLock(FALSE,(Handle)h_text);
io = AEGetNthPtr(&docList,1L,'TEXT',&keywd,&returnedType,(Ptr)*h_text,32768,&size);
MyUnlock((Handle)h_text);
if(io != noErr) {
	Alert1("Data of 'csin' Apple Event is missing. Check client application!");
	goto OUT;
	}
io = AEDisposeDesc(&docList);
if(io != noErr) goto OUT;

if(size > 0) {
	/* Beware that this handle does not terminate with '\0' */
	for(i=0; i < size; i++) Message[i] = (*h_text)[i];
	Message[size] = '\0';
	Strip(Message);
	if(Message[0] != '\0' && strcmp(Message,FileName[wCsoundInstruments]) != 0) {
		strcpy(FileName[wCsoundInstruments],Message);
		type = gFileType[wCsoundInstruments];
		c2pstrcpy(spec.name,Message);
		spec.vRefNum = TheVRefNum[wCsoundInstruments];
		spec.parID = WindowParID[wCsoundInstruments];
		if(MyOpen(&spec,fsCurPerm,&refnum) != noErr) {
			if(CheckFileName(wCsoundInstruments,FileName[wCsoundInstruments],&spec,
					&refnum,type,TRUE) != OK) {
				io = fnfErr;
				goto OUT;
				}
			}
		if(LoadCsoundInstruments(refnum,FALSE) == OK) {
			io = noErr;
			SetName(wCsoundInstruments,TRUE,FALSE);
			}
		else io = fnfErr;
		}
	}

OUT:
MyDisposeHandle((Handle*)&h_text);
return(io);
}


pascal OSErr RemoteSetConvention(const AppleEvent *p_event,AppleEvent *p_reply,long handlerRefcon)
{
OSErr io;
char **h_text;
long i,count;
Size size;
DescType returnedType;
AEKeyword keywd;
AEDescList docList;

io = AEGetParamDesc(p_event,keyDirectObject,typeAEList,&docList);
if(io != noErr) return(io);
io = MyGotRequiredParams(p_event);
if(io != noErr) {
	TellError(46,io);
	return(AEDisposeDesc(&docList));
	}
io = AECountItems(&docList,&count);
if(io != noErr) {
	TellError(47,io);
	return(AEDisposeDesc(&docList));
	}
if((h_text = (char**) GiveSpace((Size)(32768 * sizeof(char)))) == NULL) {
	return(AEDisposeDesc(&docList));
	}
/* count = 1 in this type of message */
MyLock(FALSE,(Handle)h_text);
io = AEGetNthPtr(&docList,1L,'TEXT',&keywd,&returnedType,(Ptr)*h_text,32768,&size);
MyUnlock((Handle)h_text);
if(io != noErr) {
	Alert1("Data of 'conv' Apple Event is missing. Check client application!");
	goto OUT;
	}
io = AEDisposeDesc(&docList);
if(io != noErr) goto OUT;

if(size > 0) {
	/* Beware that this handle does not terminate with '\0' */
	for(i=0; i < size; i++) Message[i] = (*h_text)[i];
	Message[size] = '\0';
	Strip(Message); UpperCaseString(Message);
	if(Message[0] != '\0') {
		for(i=0; i < MAXCONVENTIONS; i++) {
			if(strcmp(Message,ConventionString[i]) == 0) {
				NoteConvention = i;
				sprintf(Message,"Received Apple Event class 'Bel0' ID 'conv'. Changed convention to %s",
					ConventionString[i]);
				ShowMessage(TRUE,wMessage,Message);
				break;
				}
			}
		if(i >= MAXCONVENTIONS) {
			sprintf(LineBuff,"\nBP2 received a 'conv' Apple Event from a distant client, but could not interpret its data '%s' as a note convention",
				Message);
			Println(wTrace,LineBuff);
			Println(wTrace,"Note conventions recognized by this version are:");
			for(i=0; i < MAXCONVENTIONS; i++) Println(wTrace,ConventionString[i]);
			Print(wTrace,"\n");
			}
		}
	}

OUT:
MyDisposeHandle((Handle*)&h_text);
return(io);
}


pascal OSErr RemoteControl(const AppleEvent *p_event,AppleEvent *p_reply,long handlerRefcon)
{
OSErr io;
Size actualSize;
DescType returnedType;
AEEventID theID;
int r,displayitems,cyclicplay,improvize;

if(EventState == EXIT) return(noErr);

// check the ID of Apple Event
io = AEGetAttributePtr(p_event,keyEventIDAttr,typeWildCard,&returnedType,&theID,
	sizeof(AEEventID),&actualSize);
switch(theID) {
	case BeepID:
		ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'beep'");
		SysBeep(10); break;
	case AbortID:
		if(!ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn &&
			!SetTimeOn && !GraphicOn && !PrintOn /* && !ReadKeyBoardOn */
			&& !HangOn && !ScriptExecOn) break;
		ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'over'. Aborting...");
		EventState = ABORT;
		mStop(0);
		break;
	case PauseID:
		EventState = NO;
		ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'paus', meaning 'Pause'");
		mPause(0);
		break;
	case AgainID:
		if(!ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn &&
			!SetTimeOn && !GraphicOn && !PrintOn /* && !ReadKeyBoardOn */
			&& !HangOn && !ScriptExecOn) break;
		ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'more', meaning 'Play again'");
		EventState = AGAIN;
		break;
	case SkipID:
		if(!ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn &&
			!SetTimeOn && !GraphicOn && !PrintOn /* && !ReadKeyBoardOn */
			&& !HangOn && !ScriptExecOn) break;
		ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'skip'. Skipping next item...");
		SkipFlag = TRUE; break;
	case QuickID:
		if(!ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn &&
			!SetTimeOn && !GraphicOn && !PrintOn /* && !ReadKeyBoardOn */
			&& !HangOn && !ScriptExecOn) break;
		ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'fast', meaning 'Quick!'");
		EventState = QUICK; break;
	case ResumeID:
		if(!ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn &&
			!SetTimeOn && !GraphicOn && !PrintOn /* && !ReadKeyBoardOn */
			&& !HangOn && !ScriptExecOn) break;
		if(!PauseOn) break;
		ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'cont'. Resuming...");
		mResume(0);
		EventState = RESUME;
		break;
	case ImprovizeID:
		ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'impr'. Improvizing...");
		ItemNumber = ZERO;
		Maxitems = ZERO;
		ReadKeyBoardOn = FALSE; Jcontrol = -1;
		if(OutMIDI && Interactive && !LoadedIn) {
			if(GetInName(wData) != OK) GetInName(wGrammar);
			if(LoadInteraction(TRUE,FALSE) != OK) return(OK);
			}
		if(CompileCheck() != OK) return(FAILED);
		cyclicplay = CyclicPlay; CyclicPlay = FALSE;
		improvize = Improvize; Improvize = TRUE; /* SetButtons(TRUE); */
	/*	MIDI = TRUE; */
		displayitems = DisplayItems; DisplayItems = FALSE;
		SetButtons(YES);
		GetValues(TRUE);
		Ctrlinit();
		HideWindow(Window[wMessage]);
		r = ProduceItems(wStartString,FALSE,FALSE,NULL);
		CyclicPlay = cyclicplay; Improvize = improvize;
		DisplayItems = displayitems;
		SetButtons(YES);
		if(r != OK) EventState = r;
		else EventState = NO;
		break;
	case DoScriptID:
		ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'dosc'. Executing script...");
		r = RunScript(wScript,YES);
		if(r != OK) EventState = r;
		else EventState = NO;
		break;
	}
return(noErr);
}


pascal OSErr RemoteUseText(const AppleEvent *p_event,AppleEvent *p_reply,long handlerRefcon)
{
OSErr io;
char **h_text;
long w,count;
int r,oms;
Size size;
AEKeyword keywd;
AEDescList docList;
Size actualSize;
DescType returnedType;
AEEventID theID;


if(EventState == EXIT) return(noErr);

// check the ID of Apple Event
io = AEGetAttributePtr(p_event,keyEventIDAttr,typeWildCard,&returnedType,&theID,
	sizeof(AEEventID),&actualSize);

h_text = NULL;
io = AEGetParamDesc(p_event,keyDirectObject,typeAEList,&docList);
if(io != noErr) return(io);
io = MyGotRequiredParams(p_event);
if(io != noErr) {
	TellError(48,io);
	return(AEDisposeDesc(&docList));
	}
io = AECountItems(&docList,&count);
if(io != noErr) {
	TellError(49,io);
	return(AEDisposeDesc(&docList));
	}
if((h_text = (char**) GiveSpace((Size)(32768 * sizeof(char)))) == NULL) {
	return(AEDisposeDesc(&docList));
	}
MyLock(FALSE,(Handle)h_text);
io = AEGetNthPtr(&docList,1L,'TEXT',&keywd,&returnedType,(Ptr)*h_text,32768,&size);
MyUnlock((Handle)h_text);
if(io != noErr) {
	Println(wTrace,"\nData of Apple Event is missing. Check client application!");
	goto OUT;
	}
io = AEDisposeDesc(&docList);
if(io != noErr) goto OUT;
if(size > 0) {
	/* Beware, this handle does not terminate with '\0' */
	(*h_text)[size] = '\0'; /* Now it does */
	switch(theID) {
		case PlayEventID:
			ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'play'. Playing text score...");
			r = PlayHandle(h_text,NO);
			if(r != OK) EventState = r;
			else EventState = NO;
			break;
		case NameID:
			ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'name'. Changing names...");
			r = ChangeNames(h_text);
			if(r != OK) EventState = r;
			else EventState = NO;
			break;
		case GrammarID:
			ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'gram'. Loading grammar...");
			w = wGrammar;
			goto PASTE;
			break;
		case AlphabetID:
			ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'alph'. Loading alphabet...");
			w = wAlphabet;
			goto PASTE;
			break;
		case GlossaryID:
			ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'glos'. Loading glossary...");
			w = wGlossary;
			goto PASTE;
			break;
		case InteractionID:
			ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'inte'. Loading interaction...");
			w = wInteraction;
			goto PASTE;
			break;
		case DataID:
			ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'data'. Loading data...");
			w = wData;
			goto PASTE;
			break;
		case ScriptID:
			ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'scri'. Loading script...");
			w = wScript;
			goto PASTE;
			break;
		case CsoundInstrID:
			ShowMessage(TRUE,wMessage,"Received Apple Event class 'Bel0' ID 'csin'. Loading Csound instrument file...");
			w = wScript;
			goto PASTE;
			break;
		}
	}
goto OUT;

PASTE:
if(ClearWindow(NO,w) == OK) {
	ForgetFileName(w);
	PrintHandle(w,h_text);
	Dirty[w] = FALSE;
	GetHeader(w);
	switch(w) {
		case wGrammar:
			CompiledAl = FALSE;	/* Because grammar may contain new terminal symbols */
			if(GetSeName(w) == OK) LoadSettings(TRUE,TRUE,FALSE,FALSE,&oms);
			LoadLinkedMidiDriverSettings(w);
			if(GetInName(w) == OK) LoadInteraction(TRUE,FALSE);
			if(GetGlName(w) == OK) CompiledGl = LoadedGl = FALSE;
			GetAlphaName(w);
			GetTimeBaseName(w);
			GetCsName(w);
			GetFileNameAndLoadIt(wMIDIorchestra,w,LoadMIDIorchestra);
			break;
		case wAlphabet:
			CompiledAl = CompiledAl = FALSE;
			GetMiName(); GetKbName(wAlphabet);
			GetCsName(wAlphabet);
			GetFileNameAndLoadIt(wMIDIorchestra,w,LoadMIDIorchestra);
			break;
		case wGlossary:
			CompiledGl = FALSE;
			break;
		case wInteraction:
			CompiledIn = FALSE; LoadedIn = TRUE;
			break;
		case wData:
			if(GetSeName(w) == OK) LoadSettings(TRUE,TRUE,FALSE,FALSE,&oms);
			LoadLinkedMidiDriverSettings(w);
			if(GetInName(w) == OK) LoadInteraction(TRUE,FALSE);
			if(GetGlName(w) == OK) CompiledGl = LoadedGl = FALSE;
			GetCsName(wData);
			GetFileNameAndLoadIt(wMIDIorchestra,w,LoadMIDIorchestra);
			break;
		}
	}

OUT:
MyDisposeHandle((Handle*)&h_text);
return(io);
}

