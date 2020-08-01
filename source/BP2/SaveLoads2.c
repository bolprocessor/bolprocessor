/* SaveLoads2.c (BP2 version CVS) */ 

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
#include "CarbonCompatUtil.h"

LoadGrammar(FSSpec *p_spec,short refnum)
{
int r,oms;

if((r=ClearWindow(FALSE,wGrammar)) != OK) return(r);
if(ReadFile(wGrammar,refnum) == OK) {
	TheVRefNum[wKeyboard] = TheVRefNum[iObjects] = TheVRefNum[wGrammar]
	= TheVRefNum[wInteraction] = TheVRefNum[wGlossary] = TheVRefNum[iSettings]
	= TheVRefNum[wAlphabet] = TheVRefNum[wTimeBase]
	= TheVRefNum[wCsoundInstruments] = TheVRefNum[wMIDIorchestra] = p_spec->vRefNum;
	WindowParID[wKeyboard] = WindowParID[iObjects] = WindowParID[wGrammar]
	= WindowParID[wInteraction] = WindowParID[wGlossary] = WindowParID[iSettings]
	= WindowParID[wAlphabet] = WindowParID[wTimeBase]
	= WindowParID[wCsoundInstruments] = WindowParID[wMIDIorchestra] = p_spec->parID;
	p2cstrcpy(FileName[wGrammar],p_spec->name);
	SetName(wGrammar,TRUE,TRUE); /* suppressed 26/2/99 */
	CompiledPt = Dirty[wGrammar] = FALSE;
	FSClose(refnum);
	}
else {
	sprintf(Message,"Can't read '%s'... (no data)",FileName[wGrammar]);
	Alert1(Message);
	FSClose(refnum);
	ForgetFileName(wGrammar);
	return(FAILED);
	}
GetHeader(wGrammar);
CompiledAl = FALSE;	/* Because grammar may contain new terminal symbols */
if(GetSeName(wGrammar) == OK) r = LoadSettings(TRUE,TRUE,FALSE,FALSE,&oms);
LoadLinkedMidiDriverSettings(wGrammar);
GetFileNameAndLoadIt(wMIDIorchestra,wGrammar,LoadMIDIorchestra);
if(GetInName(wGrammar) == OK) LoadInteraction(TRUE,FALSE);
if(GetGlName(wGrammar) == OK) CompiledGl = LoadedGl = FALSE;
GetAlphaName(wGrammar);
GetTimeBaseName(wGrammar);
ShowSelect(CENTRE,wGrammar);
Activate(TEH[wGrammar]);	/* Needed to speed up if window was resized. */
Created[wGrammar] = TRUE;
return(r);
}


SaveObjectPrototypes(FSSpec *p_spec)
{
int pivbeg,pivend,pivbegon,pivendoff,pivcent,pivcentonoff,pivspec,
	okrescale,good;
long i,j;
short refnum;
Str255 fn;
long count,p,q;
NSWReply reply;
char line[MAXLIN];
Milliseconds t;
OSErr err;

err = NSWInitReply(&reply);
if(FileName[iObjects][0] != '\0') strcpy(Message,FileName[iObjects]);
else if (GetDefaultFileName(iObjects, Message) != OK) return(FAILED);
c2pstrcpy(fn, Message);

if(Created[iObjects]) good = (MyOpen(p_spec,fsCurPerm,&refnum) == noErr);
else good = NO;
if(good) goto WRITE;
if(NewFile(-1,gFileType[iObjects],fn,&reply)) {
	i = CreateFile(iObjects,-1,gFileType[iObjects],fn,&reply,&refnum);
	(*p_spec) = reply.sfFile;
	if(i == ABORT) {
		err = NSWCleanupReply(&reply);
		return(FAILED);
		}
	if(i == OK) {
WRITE:
		SaveOn++;
		WriteHeader(iObjects,refnum,*p_spec);
		sprintf(line,"%ld",(long)PrototypeTickKey);
		WriteToFile(NO,MAC,line,refnum);
		sprintf(line,"%ld",(long)PrototypeTickChannel);
		WriteToFile(NO,MAC,line,refnum);
		sprintf(line,"%ld",(long)PrototypeTickVelocity);
		WriteToFile(NO,MAC,line,refnum);
		WriteToFile(NO,MAC,FileName[wCsoundInstruments],refnum);
		sprintf(line,"%ld",(long)Jbol);
		WriteToFile(NO,MAC,line,refnum);
		for(i=2; i < Jbol; i++) {
			PleaseWait();
			if((*p_MIDIsize)[i] == ZERO && (*pp_CsoundScoreText)[i] == NULL) continue;
			CheckConsistency(i,TRUE);
			sprintf(line,"%s",*((*p_Bol)[i]));
			WriteToFile(YES,MAC,line,refnum);
			ShowMessage(TRUE,wMessage,line);
			sprintf(line,"%ld",(long)(*p_Type)[i]);
			WriteToFile(NO,MAC,line,refnum);
			(*p_Resolution)[i] = 1;	/* Previously = Time_res (changed 1/10/96) */
			sprintf(line,"%ld",(long)(*p_Resolution)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_DefaultChannel)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)((*p_Tref)[i] / (*p_Resolution)[i]));
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%.4f",(long)(*p_Quan)[i]);
			WriteToFile(NO,MAC,line,refnum);
			pivbeg = pivend = pivbegon = pivendoff = pivcent = pivcentonoff = pivspec
				= FALSE;
			switch((*p_PivType)[i]) {
				case 1: pivbeg = TRUE; break;
				case 2: pivend = TRUE; break;
				case 3: pivbegon = TRUE; break;
				case 4: pivendoff = TRUE; break;
				case 5: pivcent = TRUE; break;
				case 6: pivcentonoff = TRUE; break;
				case 7: pivspec = TRUE;
				}
			if((*p_OkExpand)[i] && (*p_OkCompress)[i])
				okrescale = TRUE;
			else okrescale = FALSE;
			j = 0;
			line[j++] = pivbeg + '0';
			line[j++] = pivend + '0';
			line[j++] = pivbegon + '0';
			line[j++] = pivendoff + '0';
			line[j++] = pivcent + '0';
			line[j++] = pivcentonoff + '0';
			line[j++] = okrescale + '0';
			line[j++] = (*p_FixScale)[i] + '0';
			line[j++] = (*p_OkExpand)[i] + '0';
			line[j++] = (*p_OkCompress)[i] + '0';
			line[j++] = (*p_OkRelocate)[i] + '0';
			line[j++] = (*p_BreakTempo)[i] + '0';
			line[j++] = (*p_ContBeg)[i] + '0';
			line[j++] = (*p_ContEnd)[i] + '0';
			line[j++] = (*p_CoverBeg)[i] + '0';
			line[j++] = (*p_CoverEnd)[i] + '0';
			line[j++] = (*p_TruncBeg)[i] + '0';
			line[j++] = (*p_TruncEnd)[i] + '0';
			line[j++] = pivspec + '0';
			line[j++] = (*p_AlphaCtrl)[i] + '0';
			line[j++] = '\0';
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_RescaleMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%.4f",(*p_AlphaMin)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%.4f",(*p_AlphaMax)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_DelayMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_MaxDelay)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_ForwardMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_MaxForward)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"1"); /* Not used */
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"1"); /* Not used */
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_ContBegMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_MaxBegGap)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_ContEndMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_MaxEndGap)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CoverBegMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_MaxCoverBeg)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CoverEndMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_MaxCoverEnd)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_TruncBegMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_MaxTruncBeg)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_TruncEndMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_MaxTruncEnd)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_PivMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%.4f",(*p_PivPos)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)ByteToInt((*p_AlphaCtrlNr)[i]));
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_AlphaCtrlChan)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_OkTransp)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_OkArticul)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_OkVolume)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_OkPan)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_OkMap)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_OkVelocity)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_PreRoll)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_PostRoll)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_PreRollMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_PostRollMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_PeriodMode)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%.4f",(*p_BeforePeriod)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_ForceIntegerPeriod)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_DiscardNoteOffs)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_StrikeAgain)[i]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsoundAssignedInstr)[i]);
			WriteToFile(NO,MAC,line,refnum);
			
			sprintf(line,"%ld",(long)(*p_CsoundInstr)[i]);
			WriteToFile(NO,MAC,line,refnum);
			
			sprintf(line,"%ld",(long)(*p_Tpict)[i]);
			WriteToFile(NO,MAC,line,refnum);
			
			
			sprintf(line,"%ld",(long)(*p_ObjectColor)[i].red);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_ObjectColor)[i].green);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_ObjectColor)[i].blue);
			WriteToFile(NO,MAC,line,refnum);
			
			WriteToFile(NO,MAC,"_beginCsoundScore_",refnum);
			if((*pp_CsoundScoreText)[i] != NULL) {
				MyLock(FALSE,(Handle)(*pp_CsoundScoreText)[i]);
				WriteToFile(YES,MAC,*((*pp_CsoundScoreText)[i]),refnum);
				MyUnlock((Handle)(*pp_CsoundScoreText)[i]);
				}
			WriteToFile(NO,MAC,"_endCsoundScore_",refnum);
			sprintf(line,"%ld",(long)(*p_MIDIsize)[i]);
			WriteToFile(NO,MAC,line,refnum);
			for(j=ZERO,t=ZERO; j < (*p_MIDIsize)[i]; j++) {
				t += (*((*pp_MIDIcode)[i]))[j].time;
				sprintf(line,"%ld",(long)(*((*pp_MIDIcode)[i]))[j].byte + (256L * t));
				WriteToFile(NO,MAC,line,refnum);
				}
			for(j=ZERO; j < (*p_MIDIsize)[i]; j++) {
				sprintf(line,"%ld",(long)(*((*pp_MIDIcode)[i]))[j].sequence);
				WriteToFile(NO,MAC,line,refnum);
				}
			if((*pp_Comment)[i] == NULL) {
				WriteToFile(NO,MAC,"[Comment on this prototype]",refnum);
				}
			else {
				MystrcpyHandleToString(MAXLIN,0,line,(*pp_Comment)[i]);
				WriteToFile(YES,MAC,line,refnum);
				}
			}
		WriteToFile(NO,MAC,"DATA:",refnum);
		GetField(NULL,FALSE,wPrototype1,fPrototypeFileComment,line,&p,&q);
		WriteToFile(YES,MAC,line,refnum);
		WriteToFile(NO,MAC,"_endSoundObjectFile_",refnum);
		WriteEnd(iObjects,refnum);
		GetFPos(refnum,&count);
		SetEOF(refnum,count);
		FlushFile(refnum);
		MyFSClose(iObjects,refnum,p_spec);
		reply.saveCompleted = true;
		p2cstrcpy(FileName[iObjects],p_spec->name);
		TheVRefNum[iObjects] = p_spec->vRefNum;
		WindowParID[iObjects] = p_spec->parID;
		SetName(iObjects,TRUE,TRUE);
		Created[iObjects] = TRUE;
		Dirty[iObjects] = FALSE;
		HideWindow(Window[wMessage]);
		if(SaveOn > 0) SaveOn--;
		err = NSWCleanupReply(&reply);
		return(OK);
		}
	else {
		MyPtoCstr(MAXNAME,fn,line);
		sprintf(Message,"Error creating '%s'",line);
		Alert1(Message);
		Created[iObjects] = FALSE;
		}
	}
err = NSWCleanupReply(&reply);
return(FAILED);
}


LoadObjectPrototypes(int checkversion,int tryname)
{
char c,date[80],*newp;
MIDIcode **p_b;
char **p_line,**p_completeline,line[MAXLIN];
int i,iv,j,jj,co,rep,okt1,diff,stop,maxsounds,s,objecttype,oldjbol,notsaid,
	pivbeg,pivend,pivbegon,pivendoff,pivcent,pivcentonoff,pivspec,newbols,okrescale,
	compilemem,newinstruments,type,dirtymem,longerCsound;
long t,t1,t2,tm,d,kres;
long pos,imax;
long k,kk;
double r;
short refnum,refnum2;
FSSpec spec;
Str63 filename;
Handle h,ptr;

if(SaveCheck(iObjects) == ABORT) return(FAILED);
rep = FAILED;
newinstruments = CompiledCsObjects = FALSE;
pos = 0L;
if(!tryname) FileName[iObjects][0] = '\0';
p_line = p_completeline = NULL;
c2pstrcpy(PascalLine, DeftName[iObjects]);
SetWTitle(Window[iObjects], PascalLine);
if(!tryname) GetMiName();
c2pstrcpy(PascalLine,FileName[iObjects]);
CopyPString(PascalLine, spec.name);
spec.vRefNum = TheVRefNum[iObjects];
spec.parID = WindowParID[iObjects];
if(MyOpen(&spec,fsCurPerm,&refnum) != noErr) {	// FIXME: don't use MyOpen if no FileName[iObjects] ??
FIND:
	if(!tryname) return(FAILED);
	if(FileName[iObjects][0] == '\0') {
//		if(OkWait || !ScriptExecOn)
//			Alert1("The note convention might be incorrect because I am looking for an unspecified '-mi' sound-object prototype file");
		sprintf(Message,"Find '-mi' sound-object prototype file...");
		}
	else
	sprintf(Message,"Find '%s' or other sound-object prototype file",FileName[iObjects]);
	ShowMessage(TRUE,wMessage,Message);
	if(Option) r = YES; /* Answer("Open old sound-object prototype file (text created by HyperMIDI)",'N'); */
	else r = NO;
	if(r == ABORT) return(r);
	type = gFileType[iObjects]; if(r == YES) type = ftiAny;
	if(!OldFile(iObjects,type,PascalLine,&spec)) return(ABORT);
	if(FileName[iObjects][0] == '\0') {
		p2cstrcpy(FileName[iObjects],PascalLine);
		// TellOthersMyName(iObjects); // this is taken care of by SetName() at end - akozar 050707
		}
	c2pstrcpy(filename,FileName[iObjects]);
	if(Pstrcmp(PascalLine, filename) != 0) {
		rep = Answer("Changing sound-object file name",'Y');
		switch(rep) {
			case NO:
				goto FIND;
			case YES:
				p2cstrcpy(FileName[iObjects],PascalLine);
				// TellOthersMyName(iObjects); // this is taken care of by SetName() at end - akozar 050707
				break;
			case ABORT:
				HideWindow(Window[wMessage]);
				return(ABORT);
			}
		}
	// c2pstrcpy(spec.name, FileName[iObjects]);
	if(MyOpen(&spec,fsCurPerm,&refnum) != noErr) return(ABORT);
	}
TheVRefNum[wCsoundInstruments] = TheVRefNum[iObjects] = spec.vRefNum;
WindowParID[wCsoundInstruments] = WindowParID[iObjects] = spec.parID;
HideWindow(Window[wMessage]);
rep = ABORT;
j = -1;

LoadOn++;

if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
if(CheckVersion(&iv,p_line,FileName[iObjects]) != OK) goto ERR;

sprintf(Message,"Loading %s...",FileName[iObjects]);
ShowMessage(TRUE,wMessage,Message);

if(iv == 0) {
	FSClose(refnum);
	HideWindow(Window[wMessage]);
	pos = 0L;
	MyOpen(&spec,fsCurPerm,&refnum);
	}
if(iv > 2) {
	ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos);
	GetDateSaved(p_completeline,&(p_FileInfo[iObjects]));
	}
newbols = FALSE;
if(iv > 4) {
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	PrototypeTickKey = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	PrototypeTickChannel = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	PrototypeTickVelocity = s;
	if(iv > 11) {
		ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos);
		if((*p_completeline)[0] != '\0') {
			MystrcpyHandleToString(MAXNAME,0,line,p_completeline);
			if(FileName[wCsoundInstruments][0] == '\0'
					|| strcmp(line,FileName[wCsoundInstruments]) != 0) {
				if(SaveCheck(wCsoundInstruments) == ABORT) goto MAXSOUNDS;
				strcpy(FileName[wCsoundInstruments],line);
				type = gFileType[wCsoundInstruments];
				c2pstrcpy(spec.name, line);
				spec.vRefNum = TheVRefNum[wCsoundInstruments];
				spec.parID = WindowParID[wCsoundInstruments];
				if(MyOpen(&spec,fsCurPerm,&refnum2) != noErr) {
					if(CheckFileName(wCsoundInstruments,FileName[wCsoundInstruments],
						&spec,&refnum2,type,TRUE) != OK) goto MAXSOUNDS;
					}
				rep = LoadCsoundInstruments(refnum2,FALSE);
				if(rep == OK) {
					dirtymem = Dirty[wAlphabet];
					SetName(wCsoundInstruments,TRUE,FALSE);
					PutFirstLine(wAlphabet,FileName[wCsoundInstruments],FilePrefix[wCsoundInstruments]);
					Dirty[wAlphabet] = dirtymem;
					newinstruments = TRUE;
					}
				}
			}
		}
		
MAXSOUNDS:
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	maxsounds = s;
	if(CheckTerminalSpace() != OK) goto ERR;
	oldjbol = Jbol;
	Jbol += (maxsounds - 2);
	if(ResizeObjectSpace(YES,Jbol + Jpatt,0) != OK) goto ERR;
	if(CheckTerminalSpace() != OK) goto ERR;
	Jbol = oldjbol; newbols = TRUE;
	}
else {
	if(checkversion && (Jbol < 3 || p_Bol == NULL)) {
		Alert1("This is an old type of object prototype file. To load it you must first load or create the corresponding alphabet");
		FileName[iObjects][0] = '\0';	// FIXME: this wasn't enough when TellOthersMyName() was called above; is it OK now ?
		goto OUT;	// changed from goto ERR - akozar 050707
		}
	if(ResizeObjectSpace(YES,Jbol + Jpatt,0) != OK) goto ERR;
	}
NumberTables = 0;
rep = notsaid = OK;

// Be careful: the following loop will read data forever if the file
// was not properly closed...

NEXTBOL:
PleaseWait(); j = -1;
if(Button() && (Answer("Stop reading sound-object file",'N') == YES)) {
	rep = ABORT;
	goto ERR;
	}
if(ReadOne(FALSE,TRUE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) {
	if(iv > 3) {
		sprintf(Message,"Unexpected end of '%s' file...  May be old version?",
			FileName[iObjects]);
		Println(wTrace,Message);
		}
	goto ERR;
	}
if(p_completeline == NULL) {
	if(Beta) Alert1("Err. LoadObjectPrototypes(). p_completeline == NULL");
	goto OUT;
	}
if(MyHandleLen(p_completeline) < 1) goto OUT;
if(Mystrcmp(p_completeline,"DATA:") == 0) {
	if(iv > 9) {
		if(ReadOne(FALSE,TRUE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
		if((*p_completeline)[0] == '\0')
			MystrcpyStringToHandle(&p_completeline,"[Comment on prototype file]");
		MystrcpyHandleToString(MAXFIELDCONTENT,0,line,p_completeline);
		rep = SetField(NULL,wPrototype1,fPrototypeFileComment,line);
		}
	else SetField(NULL,wPrototype1,fPrototypeFileComment,"[Comment on prototype file]");
	goto OUT;
	}
if(Mystrcmp(p_completeline,"_endSoundObjectFile_") == 0) goto OUT;
if((iv > 3) && (Mystrcmp(p_completeline,"TABLES:") == 0)) {
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto OUT;
	if(s > 0 && 0) {
		NumberTables = s;
		for(i=1; i <= s; i++) {
			/*  Read table i */
			}
		}
	goto OUT;
	}
if(iv > 4 && newbols) {
	oldjbol = Jbol;
	if((jj=CreateBol(FALSE,FALSE,p_completeline,FALSE,FALSE,BOL)) < 0) goto ERR;
	if(jj >= Jbol) {
		if(Beta) Alert1("Err. LoadObjectPrototypes(). jj >= Jbol");
		goto ERR;
		}
	if(Jbol > oldjbol) {
		compilemem = CompiledGr;
		if(notsaid) {
			ShowMessage(TRUE,wMessage,"New terminal symbols have been appended to alphabet.");
			SetSelect(GetTextLength(wAlphabet),GetTextLength(wAlphabet),
				TEH[wAlphabet]);
			PrintBehind(wAlphabet,"\n");
			}
		notsaid = FALSE;
		MystrcpyHandleToString(MAXLIN,0,Message,p_completeline);
		strcat(Message," ");
		PrintBehind(wAlphabet,Message);
		CompiledAl = TRUE; /* Dirty[wAlphabet] = FALSE; */
		CompiledGr = compilemem;
		}
	}
diff = TRUE;
for(j=0; j < Jbol; j++) {
	if((diff = MyHandlecmp((*p_Bol)[j],p_completeline)) == 0) break;
	}
if(j < 2) diff = TRUE;	/* May happen in old files */
// Beware that diff=TRUE  ==> j = Jbol, which is larger than allowed.

if(iv > 3) { 
	if(ReadInteger(refnum,&objecttype,&pos) == FAILED) goto ERR;
	if(!diff) (*p_Type)[j] = objecttype;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_Resolution)[j] = s;
	}
else if(!diff) {
	(*p_Type)[j] = 1;
	(*p_Resolution)[j] = 1;
	}
if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
if(!diff) (*p_DefaultChannel)[j] = s;
if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
if(!diff) (*p_Tref)[j] = ((long) k * (*p_Resolution)[j]);
if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
if(!diff) (*p_Quan)[j] = r;
if(ReadOne(FALSE,FALSE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
if(!diff) {
	i = 0;
	pivbeg = (*p_line)[i++]-'0';
	pivend = (*p_line)[i++]-'0';
	pivbegon = (*p_line)[i++]-'0';
	pivendoff = (*p_line)[i++]-'0';
	pivcent = (*p_line)[i++]-'0';
	pivcentonoff = (*p_line)[i++]-'0';
	okrescale = (*p_line)[i++]-'0';
	(*p_FixScale)[j] = (*p_line)[i++]-'0';
	(*p_OkExpand)[j] = (*p_line)[i++]-'0';
	(*p_OkCompress)[j] = (*p_line)[i++]-'0';
	(*p_OkRelocate)[j] = (*p_line)[i++]-'0';
	(*p_BreakTempo)[j] = (*p_line)[i++]-'0';
	(*p_ContBeg)[j] = (*p_line)[i++]-'0';
	(*p_ContEnd)[j] = (*p_line)[i++]-'0';
	(*p_CoverBeg)[j] = (*p_line)[i++]-'0';
	(*p_CoverEnd)[j] = (*p_line)[i++]-'0';
	(*p_TruncBeg)[j] = (*p_line)[i++]-'0';
	(*p_TruncEnd)[j] = (*p_line)[i++]-'0';
	pivspec = (*p_line)[i++]-'0';
	(*p_PivType)[j] = pivbeg + 2 * pivend + 3 * pivbegon + 4 * pivendoff
		+ 5 * pivcent + 6 * pivcentonoff + 7 * pivspec;
	(*p_AlphaCtrl)[j] = (*p_line)[i++]-'0';
	}

if(iv > 3) {
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR; /* rescalemode */
	if(!diff) (*p_RescaleMode)[j] = s;
	if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
	if(!diff) (*p_AlphaMin)[j] = r;
	if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
	if(!diff) (*p_AlphaMax)[j] = r;
	if(!diff && iv < 5) {	/* Fixing a bug in default values */
		(*p_AlphaMax)[j] = 10L;
		}
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR; /* delaymode */
	if(!diff) (*p_DelayMode)[j] = s;
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	if(!diff) (*p_MaxDelay)[j] = k;
	if(iv > 4) {
		if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR; /* forwardmode */
		if(!diff) (*p_ForwardMode)[j] = s;
		}
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	if(!diff) (*p_MaxForward)[j] = k;
	if(!diff && iv < 5) {	/* Fixing a bug in default values */
		(*p_MaxDelay)[j] = (*p_MaxForward)[j] = ZERO;
		}
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR; /* brktempomode */
	if(!diff) (*p_BreakTempoMode)[j] = s;
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
/*	if(!diff) (*p_MaxBreakTempo)[j] = k;
	if(!diff && iv < 5) {
		(*p_MaxBreakTempo)[j] = ZERO;
		} */
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR; /* contbegmode */
	if(!diff) (*p_ContBegMode)[j] = s;
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	if(!diff) (*p_MaxBegGap)[j] = k;
	if(!diff && iv < 5) {	/* Fixing a bug in default values */
		(*p_MaxBegGap)[j] = ZERO;
		}
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR; /* contendmode */
	if(!diff) (*p_ContEndMode)[j] = s;
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	if(!diff) (*p_MaxEndGap)[j] = k;
	if(!diff && iv < 5) {	/* Fixing a bug in default values */
		(*p_MaxEndGap)[j] = ZERO;
		}
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR; /* coverbegmode */
	if(!diff) (*p_CoverBegMode)[j] = s;
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	if(!diff) (*p_MaxCoverBeg)[j] = k;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR; /* coverendmode */
	if(!diff) (*p_CoverEndMode)[j] = s;
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	if(!diff) (*p_MaxCoverEnd)[j] = k;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR; /* truncbegmode */
	if(!diff) (*p_TruncBegMode)[j] = s;
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	if(!diff) (*p_MaxTruncBeg)[j] = k;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR; /* truncendmode */
	if(!diff) (*p_TruncEndMode)[j] = s;
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	if(!diff) (*p_MaxTruncEnd)[j] = k;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_PivMode)[j] = s;
	}
else {
	if(!diff) {
		(*p_RescaleMode)[j] = LINEAR;
		(*p_AlphaMin)[j] = 0; (*p_AlphaMax)[j] = 100.;
		(*p_DelayMode)[j] = (*p_ForwardMode)[j] = (*p_BreakTempoMode)[j]
			= (*p_ContBegMode)[j] = (*p_ContEndMode)[j] = ABSOLUTE;
		(*p_CoverBegMode)[j] = (*p_CoverEndMode)[j] = (*p_TruncBegMode)[j]
			= (*p_TruncEndMode)[j] = RELATIVE;
		(*p_PivMode)[j] = ABSOLUTE;
		(*p_MaxDelay)[j] = (*p_MaxForward)[j] = Infpos;
		(*p_MaxBegGap)[j] = (*p_MaxEndGap)[j] = Infpos;
		(*p_MaxCoverBeg)[j] = (*p_MaxCoverEnd)[j] = 100L;
		(*p_MaxTruncBeg)[j] = (*p_MaxTruncEnd)[j] = ZERO;
		}
	}
if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
if(!diff) (*p_PivPos)[j] = r;
if(iv > 1) {
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_AlphaCtrlNr)[j] = s;
	}
else if(!diff) (*p_AlphaCtrlNr)[j] = 255;
if(iv > 4) {
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_AlphaCtrlChan)[j] = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_OkTransp)[j] = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_OkArticul)[j] = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_OkVolume)[j] = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_OkPan)[j] = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_OkMap)[j] = s;
	if(iv >= 18) {
		if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
		if(!diff) (*p_OkVelocity)[j] = s;
		}
	else (*p_OkMap)[j] = (*p_OkVelocity)[j] = TRUE;
	}
else {
	if(!diff) {
		(*p_AlphaCtrlChan)[j] = 255;
		(*p_OkTransp)[j] = TRUE;
		(*p_OkArticul)[j] = TRUE;
		(*p_OkVolume)[j] = TRUE;
		(*p_OkPan)[j] = TRUE;
		(*p_OkMap)[j] = TRUE;
		(*p_OkVelocity)[j] = TRUE;
		}
	}
if(iv > 9) {
	if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
	if(!diff) (*p_PreRoll)[j] = r;
	if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
	if(!diff) (*p_PostRoll)[j] = r;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_PreRollMode)[j] = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_PostRollMode)[j] = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_PeriodMode)[j] = s;
	if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
	if(!diff) (*p_BeforePeriod)[j] = r;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_ForceIntegerPeriod)[j] = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_DiscardNoteOffs)[j] = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_StrikeAgain)[j] = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	if(!diff) (*p_CsoundAssignedInstr)[j] = s;
	if(iv > 10) {
		if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
		if(!diff) (*p_CsoundInstr)[j] = s;
		}
	else {
		if(!diff) (*p_CsoundInstr)[j] = 0;
		}
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	if(!diff) (*p_Tpict)[j] = k;
	}
else {
	if(!diff) {
		(*p_PreRoll)[j] = (*p_PostRoll)[j] = ZERO;
		(*p_Tpict)[j] = ZERO;
		(*p_PreRollMode)[j] = (*p_PostRollMode)[j] = RELATIVE;
		(*p_PeriodMode)[j] = IRRELEVANT;
		(*p_BeforePeriod)[j] = (*p_CsoundInstr)[j] = 0.;
		(*p_ForceIntegerPeriod)[j] = (*p_DiscardNoteOffs)[j] = FALSE;
		(*p_StrikeAgain)[j] = (*p_CsoundAssignedInstr)[j] = -1;
		}
	}
if(iv > 21) {
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	if(!diff) (*p_ObjectColor)[j].red = k;
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	if(!diff) (*p_ObjectColor)[j].green = k;
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	if(!diff) (*p_ObjectColor)[j].blue = k;
	}
else {
	if(!diff) {
		(*p_ObjectColor)[j].red = (*p_ObjectColor)[j].green = (*p_ObjectColor)[j].blue = -1L;
		}
	}
if(!diff) {
	ptr = (Handle) (*pp_CsoundScoreText)[j];
	if(MyDisposeHandle(&ptr) != OK) goto ERR;
	(*pp_CsoundScoreText)[j] = NULL;
	(*p_CompiledCsoundScore)[j] = TRUE;
	(*pp_CsoundTime)[j] = NULL;
	}
if(iv > 9) {
	/* Read pp_CsoundScoreText */
	if(!diff) ClearWindow(NO,wPrototype7);
	if(ReadOne(FALSE,FALSE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
	StripHandle(p_line);
	if(Mystrcmp(p_line,"_beginCsoundScore_") != 0) goto ERR;
	
NEXTCSOUNDSCORELINE:
	if(ReadOne(FALSE,TRUE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
	if(Mystrcmp(p_completeline,"_endCsoundScore_") == 0) {
		if(!diff) {
			if((rep=GetCsoundScore(j)) != OK) goto OUT;
			if((rep=CompileObjectScore(j,&longerCsound)) != OK) goto OUT;
			}
		goto READSIZE;
		}
	if(!diff) {
		PrintHandleBehind(wPrototype7,p_completeline);
		PrintBehind(wPrototype7,"\n");
		Dirty[iObjects] = Dirty[wPrototype7] = FALSE;
		CompiledCsObjects = (*p_CompiledCsoundScore)[j] = FALSE;
		}
	goto NEXTCSOUNDSCORELINE;
	}

READSIZE:
if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
imax = s;
if(!diff) {
	(*p_PasteDone)[j] = FALSE;
	(*p_MIDIsize)[j] = (*p_Ifrom)[j] = ZERO;
	(*pp_MIDIcode)[j] = NULL;
	}
if(imax > 0 && !diff) {
	if((p_b = (MIDIcode**) GiveSpace((Size)sizeof(MIDIcode)*(imax+1))) == NULL) goto ERR;
	rep = OK;
	}
if(!diff && (*p_CsoundSize)[j] == 0) (*p_Dur)[j] = t1 = t2 = ZERO;
if(imax > 0) {
	for(i=0,tm=ZERO,okt1=FALSE; i < imax; i++) {
		if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
		if(Eucl(k,256L,(unsigned long*)&t,(unsigned long*)&kk) != 0) goto ERR;
		if(!diff) (*p_b)[i].byte = (int) kk;
		if(143L < kk  &&  kk < 160L) {   /* NoteOn or NoteOff */
			t2 = t;
			if(!okt1) {
				okt1 = TRUE;
				t1 = t;
				}
			}
		if(!diff) {
			(*p_b)[i].time = (Milliseconds) t;
			}
		}

	if(iv > 11) {		
		for(i=0; i < imax; i++) {
			if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
			if(!diff) {
				(*p_b)[i].sequence = s;
				}
			}
		}
	else for(i=0; i < imax; i++) (*p_b)[i].sequence = 0;

	if(!diff) {
		if(MIDItoPrototype(FALSE,TRUE,j,p_b,imax) != OK) goto ERR;
		if(MyDisposeHandle((Handle*)&p_b) != OK) goto ERR;
		}
	}

if(!diff && CheckConsistency(j,TRUE) != OK) goto ERR;
if(iv > 9) {
	if(ReadOne(FALSE,TRUE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
	if(p_completeline == NULL) {
		if(Beta) Alert1("Err. LoadObjectPrototypes(). p_completeline == NULL");
		goto ERR;
		}
	if(!diff) {
		s = MyHandleLen(p_completeline);
		if(s > MAXFIELDCONTENT) {
			s = MAXFIELDCONTENT;
			(*p_completeline)[s] = '\0';
			}
		if((h = (Handle) GiveSpace((Size)(1+s) * sizeof(char))) == NULL) goto ERR;
		(*pp_Comment)[j] = h;
		MystrcpyHandleToHandle(0,&((*pp_Comment)[j]),p_completeline);
		}
	}
goto NEXTBOL;

ERR:
if(!diff) {
	if(j > 1 && j < Jbol) ResetPrototype(j);
	}
if(CheckEmergency() == OK) {
	rep = FAILED;
	sprintf(Message,"File '%s' may be corrupted or in some unknown format",
		FileName[iObjects]);
	Alert1(Message);
	if(j > 1 && j < Jbol) {
		sprintf(Message,"An error occured while reading '%s'",*((*p_Bol)[j]));
		Println(wTrace,Message);
		}
	Println(wTrace,
		"You may read the file to the 'Scrap' window and try to fix inconsistencies.");
	}
else rep = ABORT;

OUT:
LoadOn--;
MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
FSClose(refnum);
if(CheckEmergency() != OK) return(ABORT);
HideWindow(Window[wMessage]);

ObjectMode = ObjectTry = TRUE;
SetButtons(TRUE);
if(rep == OK) {
	SetName(iObjects,YES,TRUE);
	if(newbols) ResizeObjectSpace(NO,Jbol + Jpatt,0);
	}

return(rep);
}


LoadGlossary(int anyfile,int manual)
{
int i,io,maxctrl,maxwait,rep,result,iv,s,type;
FSSpec spec;
short refnum;
char line[MAXNAME+1],line2[MAXNAME+1],date[MAXNAME+1];
// char **p_line,**p_completeline;
long pos,p,q;
long k;
double r;

if(LoadedGl) return(OK);

if(!ScriptExecOn) ShowWindow(Window[wGlossary]);
result = FAILED;
// p_line = p_completeline = NULL;
type = gFileType[wGlossary];
if(anyfile) type = ftiAny;
spec.vRefNum = TheVRefNum[wGlossary];
spec.parID = WindowParID[wGlossary];
c2pstrcpy(spec.name, FileName[wGlossary]);
strcpy(line, FileName[wGlossary]);
SetSelect(ZERO,GetTextLength(wGlossary),TEH[wGlossary]);
TextDelete(wGlossary); CompiledGl = FALSE;
if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
	if(CheckFileName(wGlossary,line,&spec,&refnum,type,TRUE) != OK) {
		return(FAILED);
		}
	}
sprintf(Message,"Loading %s...",FileName[wGlossary]);
ShowMessage(TRUE,wMessage,Message);
pos = 0L; LoadOn++;

/* READMORE:
if(ReadOne(FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
if((*p_line)[0] == '<') goto READMORE;
if(CheckVersion(&iv,p_line,FileName[wGlossary]) != OK) goto ERR;
ReadOne(TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos);
GetDateSaved(p_completeline,&(p_FileInfo[wGlossary])); */
result = OK;
pos = ZERO;
SetFPos(refnum,fsFromStart,pos);
if(ReadFile(wGlossary,refnum) == OK) {
	if(!WASTE) {
		CCUTEToScrap();	// WHY?
		}
	/* The following is only useful to erase date and version */
	GetHeader(wGlossary);
	SetName(wGlossary,TRUE,TRUE);
	goto QUIT;
	}
else {
	result = FAILED; goto QUIT;
	}

ERR:
result = FAILED;
Alert1("Error reading glossary file...");
ForgetFileName(wGlossary); /* 1/3/97 */

QUIT:
// MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
if(FSClose(refnum) != noErr) {
	if(Beta) Alert1("Error closing glossary code file...");
	}
HideWindow(Window[wMessage]);
if(result == OK) {
	SetSelect(ZERO,ZERO,TEH[wGlossary]);
	if(manual) BPActivateWindow(SLOW,wGlossary);
	else UpdateWindow(FALSE,Window[wGlossary]);
	LoadedGl = TRUE;
	Created[wGlossary] = TRUE;
	}
else	Created[wGlossary] = FALSE;
Dirty[wGlossary] = FALSE;
HideWindow(Window[wMessage]);
LoadOn--;
return(result);
}


LoadAlphabet(int w,FSSpec *p_spec)
{
short refnum;
FSSpec spec;
int io,rep,result;
char line2[64];


if(w == -1 && GetAlphaName(wData) != OK && GetAlphaName(wGrammar) != OK)
	return(OK);
else if(GetAlphaName(w) != OK) return(OK);
if((rep=ClearWindow(FALSE,wAlphabet)) != OK) return(rep);
if(!ScriptExecOn) ShowWindow(Window[wAlphabet]);
c2pstrcpy(spec.name, FileName[wAlphabet]);
strcpy(LineBuff, FileName[wAlphabet]);
if(p_spec != NULL) {
	spec.vRefNum = p_spec->vRefNum;
	spec.parID = p_spec->parID;
	}
else {
	spec.vRefNum = TheVRefNum[wAlphabet];
	spec.parID = WindowParID[wAlphabet];
	}
if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
FIND:
	sprintf(Message,"Find '%s' or other alphabet file",LineBuff);
	ShowMessage(TRUE,wMessage,Message);
	if(!OldFile(wAlphabet,6,PascalLine,&spec)) {
		HideWindow(Window[wMessage]);
		ForgetFileName(wAlphabet);
		return(FAILED);
		}
	p2cstrcpy(line2,PascalLine);
	if(strcmp(LineBuff,line2) != 0) {
		rep = Answer("Changing alphabet file",'Y');
		switch(rep) {
			case NO:
				goto FIND;
				break;
			case YES:
				p2cstrcpy(FileName[wAlphabet],PascalLine);
				TellOthersMyName(wAlphabet);
				Dirty[wGrammar] = Dirty[wData] = TRUE;
				break;
			case ABORT:
				HideWindow(Window[wMessage]);
				return(ABORT);
			}
		}
	if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
		sprintf(Message,"Can't open '%s'",FileName[wAlphabet]);
		ShowMessage(TRUE,wMessage,Message);
		ForgetFileName(wAlphabet);
		TellError(58,io);
		return(ABORT);
		}
	}
HideWindow(Window[wMessage]); LoadOn++;								
if(ReadFile(wAlphabet,refnum) == OK) {
	p2cstrcpy(FileName[wAlphabet],spec.name);
	SetName(wAlphabet,TRUE,TRUE);
	TheVRefNum[wKeyboard] = TheVRefNum[iObjects] = TheVRefNum[wAlphabet]
		= TheVRefNum[wCsoundInstruments] = spec.vRefNum;
	WindowParID[wKeyboard] = WindowParID[iObjects] = WindowParID[wAlphabet]
		= WindowParID[wCsoundInstruments] = spec.parID;
	result = OK;		
	GetHeader(wAlphabet);
	GetMiName(); GetKbName(wAlphabet); GetCsName(wAlphabet);
	GetFileNameAndLoadIt(wMIDIorchestra,wAlphabet,LoadMIDIorchestra);
	Created[wAlphabet] = TRUE;
	}
else {
	sprintf(Message,"Can't read '%s'... (no data)",FileName[wAlphabet]);
	Alert1(Message);
	ForgetFileName(wAlphabet);
	result = FAILED;
	}
if(FSClose(refnum) != noErr) {
	if(Beta) Alert1("Error closing alphabet file...");
	}
ShowSelect(CENTRE,wAlphabet);
LoadOn--;
return(result);
}