/* SaveLoads1.c (BP3) */ 

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

int trace_load_settings = 0;
int trace_load_prototypes = 0;
int trace_load_csound_resources = 0;

#if BP_CARBON_GUI_FORGET_THIS
#include "CarbonCompatUtil.h"

#define OKOMS 1

LoadWeights(void)
{
int vref,i,iv,j,r,igram,irul,w;
FSSpec spec;
short refnum;
long pos;
long compiledate;
char line[MAXNAME+1];
char *p,*q;
OSErr io;
char **p_completeline,**p_line;

if(ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn || CompileOn || GraphicOn
	|| PolyOn) return(RESUME);
if(CompileCheck() != OK) return(MISSED);
LoadOn++;
ShowMessage(TRUE,wMessage,"Locate '-wg.' file...");
p_line = p_completeline = NULL;
if(OldFile(-1,12,PascalLine,&spec)) {
	MyPtoCstr(255,PascalLine,LineBuff);
	if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
		pos = ZERO;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
		if(CheckVersion(&iv,p_line,LineBuff) != OK) goto NOERR;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
		if(ReadLong(refnum,&compiledate,&pos) == MISSED) goto ERR;
		if(compiledate != CompileDate) {
			r = Answer(
	"Grammar may have changed. Non-matching grammars may yield unpredictable results. Load anyway",'N');
			if(r != OK) goto NOERR;
			}
		SetCursor(&WatchCursor);
		for(igram=1; igram <= Gram.number_gram; igram++) {
			for(irul=1; irul <= (*(Gram.p_subgram))[igram].number_rule; irul++) {
				if(ReadInteger(refnum,&w,&pos) == MISSED) goto ERR;
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].weight = w;
				}
			}
		AdjustWeights();
		p2cstrcpy(FileName[iWeights],PascalLine);
		goto NOERR;
ERR:
		Alert1("Can't read weight file...");
NOERR:
		if(FSClose(refnum) == noErr) ;
		}
	else TellError(32,io);
	MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
	}
else {
	HideWindow(Window[wMessage]);
	LoadOn--;
	return(MISSED);
	}
LoadOn--;
if(Beta && LoadOn > 0) {
	Alert1("=> Err. LoadOn > 0 ");
	LoadOn = 0;
	}
HideWindow(Window[wMessage]);
if(!IsEmpty(wInfo)) BringToFront(Window[wInfo]);
return(OK);
}

SaveWeights(void)
{
int i,igram,irul,w;
short refnum;
long count;
Str255 fn;
NSWReply reply;
OSErr err;

if(ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn || CompileOn || GraphicOn
	|| PolyOn) return(RESUME);
if(CompileCheck() != OK) return(MISSED);
err = NSWInitReply(&reply);
ShowMessage(TRUE,wMessage,"Creating weight file...");
if(FileName[iWeights][0] != '\0')
	strcpy(Message,FileName[iWeights]);
else if (GetDefaultFileName(iWeights, Message) != OK) return(MISSED);
c2pstrcpy(fn, Message);
reply.sfFile.vRefNum = TheVRefNum[iSettings];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[iSettings];
if(NewFile(-1,12,fn,&reply)) {
	i = CreateFile(-1,-1,12,fn,&reply,&refnum);
	if(i == ABORT) {
		err = NSWCleanupReply(&reply);
		return(MISSED);
		}
	if(i == OK) {
		SetCursor(&WatchCursor);
		WriteHeader(iWeights,refnum,reply.sfFile);
		sprintf(LineBuff,"%ld",(long)CompileDate);
		WriteToFile(NO,MAC,LineBuff,refnum);
		for(igram=1; igram <= Gram.number_gram; igram++) {
			for(irul=1; irul <= (*(Gram.p_subgram))[igram].number_rule; irul++) {
				w = (*((*(Gram.p_subgram))[igram].p_rule))[irul].weight;
				sprintf(LineBuff,"%ld",(long)w);
				WriteToFile(NO,MAC,LineBuff,(long)refnum);
				}
			}
		WriteEnd(-1,refnum);
		GetFPos(refnum,&count);
		SetEOF(refnum,count);
		FlushFile(refnum);
		FSClose(refnum);
		reply.saveCompleted = true;
		err = NSWCleanupReply(&reply);
		p2cstrcpy(FileName[iWeights],fn);
		return(OK);
		}
	else {
		MyPtoCstr(MAXNAME,fn,LineBuff);
		sprintf(Message,"=> Error creating '%s'",LineBuff);
		Alert1(Message);
		}
	}
err = NSWCleanupReply(&reply);
return(MISSED);
}


LoadKeyboard(short refnum)
{
int i,io,iv,imax,j,result;
char **ptr;
long pos;
char **p_line,**p_completeline;

pos = ZERO; Dirty[wKeyboard] = FALSE;
p_line = p_completeline = NULL;
LoadOn++;

if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
if(CheckVersion(&iv,p_line,FileName[wKeyboard]) != OK) goto ERR;
if(ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
GetDateSaved(p_completeline,&(p_FileInfo[wKeyboard]));
if(ReadInteger(refnum,&imax,&pos) == MISSED) goto ERR;
if(imax > 52) {
	Alert1("This version of BP2 can't read selected '-kb.' file");
	goto ERR;
	}
for(i=0; i < imax; i++) {
	if(ReadOne(FALSE,TRUE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED
			|| (*p_completeline)[0] == '\0')
		goto ERR;
	ptr = (*p_Token)[i];
	MyDisposeHandle((Handle*)&ptr);
	(*p_Token)[i] = NULL;
	if((ptr = (char**) GiveSpace((Size)MyHandleLen(p_completeline)+1)) == NULL) {
		LoadOn--; return(ABORT);
		}
	(*p_Token)[i] = ptr;
	MystrcpyHandleToHandle(0,&((*p_Token)[i]),p_completeline);
	}
if(ReadInteger(refnum,&j,&pos) == MISSED) goto ERR;
KeyboardType = j;
result = OK;
goto QUIT;

ERR:
GetKeyboard();
result = MISSED;
sprintf(Message,"=> Error reading '%s' keyboard file...",FileName[wKeyboard]);
Alert1(Message);
FileName[wKeyboard][0] = '\0';

QUIT:
MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
SetKeyboard();
if(Token) AppendScript(66); else AppendScript(67);
if(FSClose(refnum) != noErr) {
	sprintf(Message,"=> Error closing '%s' keyboard file...",FileName[wKeyboard]);
	Alert1(Message);
	result = MISSED;
	}
if(result == OK) {
	SetName(wKeyboard,TRUE,TRUE);
	Created[wKeyboard] = TRUE;
	}
else	Created[wKeyboard] = FALSE;
Dirty[wKeyboard] = FALSE;
HideWindow(Window[wMessage]);
LoadOn--;
return(result);
}


SaveKeyboard(FSSpec *p_spec)
{
int i,good;
short refnum;
Str255 fn;
long count;
NSWReply reply;
OSErr err;

err = NSWInitReply(&reply);
GetKeyboard();
if(FileName[wKeyboard][0] != '\0') strcpy(Message,FileName[wKeyboard]);
else if (GetDefaultFileName(wKeyboard, Message) != OK) return(MISSED);
c2pstrcpy(fn, Message);
p_spec->vRefNum = TheVRefNum[wKeyboard];
p_spec->parID = WindowParID[wKeyboard];
CopyPString(fn,p_spec->name);
good = NO;
if(Created[wKeyboard]) good = (MyOpen(p_spec,fsCurPerm,&refnum) == noErr);
if(good) goto WRITE;
reply.sfFile.vRefNum = TheVRefNum[wKeyboard];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[wKeyboard];
if(NewFile(-1,gFileType[wKeyboard],fn,&reply)) {
	i = CreateFile(wKeyboard,-1,gFileType[wKeyboard],fn,&reply,&refnum);
	(*p_spec) = reply.sfFile;
	if(i == ABORT) {
		err = NSWCleanupReply(&reply);
		return(MISSED);
		}
	if(i == OK) {
WRITE:
		SetCursor(&WatchCursor);
		WriteHeader(wKeyboard,refnum,*p_spec);
		WriteToFile(NO,MAC,"52",refnum);
		for(i=0; i < 52; i++) {
			sprintf(LineBuff,"%s",*((*p_Token)[i]));
			WriteToFile(YES,MAC,LineBuff,refnum);
			}
		sprintf(LineBuff,"%ld",(long)KeyboardType);
		WriteToFile(NO,MAC,LineBuff,refnum);
		WriteToFile(NO,MAC,"1\n1\n1\n1\n0\n0\n0\n0\n\0",refnum);
		WriteEnd(wKeyboard,refnum);
		GetFPos(refnum,&count);
		SetEOF(refnum,count);
		FlushFile(refnum);
		MyFSClose(wKeyboard,refnum,p_spec);
		reply.saveCompleted = true;
		p2cstrcpy(FileName[wKeyboard],p_spec->name);
		TheVRefNum[wKeyboard] = p_spec->vRefNum;
		WindowParID[wKeyboard] = p_spec->parID;
		SetName(wKeyboard,TRUE,TRUE);
		Created[wKeyboard] = TRUE;
		Dirty[wKeyboard] = FALSE;
		err = NSWCleanupReply(&reply);
		return(OK);
		}
	else {
		MyPtoCstr(MAXNAME,fn,LineBuff);
		sprintf(Message,"=> Error creating '%s'",LineBuff);
		Alert1(Message);
		}
	}
err = NSWCleanupReply(&reply);
return(MISSED);
}


LoadTimeBase(short refnum)
{
int i,io,iv,imax,j,result,y,maxticks,maxbeats,arg;
char **ptr;
long pos,x;
char **p_line,**p_completeline;

WaitForLastTicks();
LoadOn++;
pos = ZERO; Dirty[wTimeBase] = FALSE;
p_line = p_completeline = NULL;
if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
if(CheckVersion(&iv,p_line,FileName[wTimeBase]) != OK) goto ERR;
if(ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
GetDateSaved(p_completeline,&(p_FileInfo[wTimeBase]));
if(ReadInteger(refnum,&y,&pos) == MISSED) goto ERR; maxticks = y;
if(ReadInteger(refnum,&y,&pos) == MISSED) goto ERR; maxbeats = y;
if(maxticks > MAXTICKS) {
	Alert1("This version of BP2 can't read selected '-tb.' file");
	goto ERR;
	}
if(maxbeats > MAXBEATS) {
	Alert1("This version of BP2 can't read selected '-tb.' file");
	goto ERR;
	}
TickThere = FALSE;
for(i=0; i < maxticks; i++) {
	if(ReadInteger(refnum,&y,&pos) == MISSED) goto ERR; /* type */
	if(ReadInteger(refnum,&y,&pos) == MISSED) goto ERR; arg = y; /* nr of arguments */
	if(ReadInteger(refnum,&y,&pos) == MISSED) goto ERR; TickKey[i] = y;
	if(ReadInteger(refnum,&y,&pos) == MISSED) goto ERR; TickChannel[i] = y;
	if(ReadInteger(refnum,&y,&pos) == MISSED) goto ERR; TickVelocity[i] = y;
	if(ReadInteger(refnum,&y,&pos) == MISSED) goto ERR; TickCycle[i] = y;
	Ptick[i] = Qtick[i] = 1L; MuteTick[i] = FALSE;
	if(arg > 4) {
		if(ReadLong(refnum,&x,&pos) == MISSED) goto ERR; Ptick[i] = x;
		if(ReadLong(refnum,&x,&pos) == MISSED) goto ERR; Qtick[i] = x;
		}
	if(arg > 6) {
		if(ReadInteger(refnum,&y,&pos) == MISSED) goto ERR; TickDuration[i] = y;
		}
	for(j=0; j < maxbeats; j++) {
		if(ReadLong(refnum,&x,&pos) == MISSED) goto ERR;
		ThisTick[i][j] = x;
		if(x % 2) TickThere = TRUE;
		}
	}
result = OK;
if(ReadOne(FALSE,TRUE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
if(iv > 9) {
/*	MyLock(FALSE,(Handle)p_completeline);
	result = SetField(NULL,wTimeBase,fTimeBaseComment,*p_completeline);
	MyUnlock((Handle)p_completeline); */
	}
// else SetField(NULL,wTimeBase,fTimeBaseComment,"[Comment on time base]");
goto QUIT;

ERR:
GetTimeBase(); GetTickParameters();
result = MISSED;
sprintf(Message,"=> Error reading '%s' time base file...",FileName[wTimeBase]);
Alert1(Message);
FileName[wTimeBase][0] = '\0';

QUIT:
MyDisposeHandle((Handle*)&p_line);
MyDisposeHandle((Handle*)&p_completeline);
for(i=0; i < MAXTICKS; i++) SetTickParameters(i+1,MAXBEATS);
SetTickParameters(0,MAXBEATS);
if(FSClose(refnum) != noErr) {
	sprintf(Message,"=> Error closing '%s' time base file...",FileName[wTimeBase]);
	Alert1(Message);
	result = MISSED;
	}
if(result == OK) {
	SetName(wTimeBase,TRUE,TRUE);
	Created[wTimeBase] = TRUE;
	}
else	Created[wTimeBase] = FALSE;
HideWindow(Window[wMessage]);
ResetTickFlag = TRUE;
Dirty[wTimeBase] = FALSE;
LoadOn--;
return(result);
}


SaveTimeBase(FSSpec *p_spec)
{
int i,j,good;
short refnum;
Str255 fn;
long x,count,p,q;
NSWReply reply;
char line[MAXFIELDCONTENT];
OSErr err;

err = NSWInitReply(&reply);
GetTimeBase(); GetTickParameters();
if(FileName[wTimeBase][0] != '\0') strcpy(Message,FileName[wTimeBase]);
else if (GetDefaultFileName(wTimeBase, Message) != OK) return(MISSED);
c2pstrcpy(fn, Message);
p_spec->vRefNum = TheVRefNum[wTimeBase];
p_spec->parID = WindowParID[wTimeBase];
CopyPString(fn,p_spec->name);
good = NO;
if(Created[wTimeBase]) good = (MyOpen(p_spec,fsCurPerm,&refnum) == noErr);
if(good) goto WRITE;
reply.sfFile.vRefNum = TheVRefNum[wTimeBase];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[wTimeBase];
if(NewFile(-1,gFileType[wTimeBase],fn,&reply)) {
	i = CreateFile(wTimeBase,-1,gFileType[wTimeBase],fn,&reply,&refnum);
	*p_spec = reply.sfFile;
	if(i == ABORT) {
		err = NSWCleanupReply(&reply);
		return(MISSED);
		}
	if(i == OK) {
WRITE:
		SetCursor(&WatchCursor);
		WriteHeader(wTimeBase,refnum,*p_spec);
		sprintf(LineBuff,"%ld",(long)MAXTICKS); WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)MAXBEATS); WriteToFile(NO,MAC,LineBuff,refnum);
		for(i=0; i < MAXTICKS; i++) {
			sprintf(LineBuff,"1\n7");	/* '1' is the type and '4' the number of parameters */
			WriteToFile(NO,MAC,LineBuff,refnum);
			sprintf(LineBuff,"%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld",
				(long)TickKey[i],(long)TickChannel[i],
				(long)TickVelocity[i],(long)TickCycle[i],(long)Ptick[i],
				(long)Qtick[i],(long)TickDuration[i]);
			WriteToFile(NO,MAC,LineBuff,refnum);
			for(j=0; j < MAXBEATS; j++) {
				sprintf(LineBuff,"%ld",(long)ThisTick[i][j]);
				WriteToFile(NO,MAC,LineBuff,refnum);
				}
			}
		GetField(NULL,FALSE,wTimeBase,fTimeBaseComment,line,&p,&q);
		WriteToFile(YES,MAC,line,refnum);	/* For comment */
		WriteEnd(wTimeBase,refnum);
		GetFPos(refnum,&count);
		SetEOF(refnum,count);
		FlushFile(refnum);
		MyFSClose(wTimeBase,refnum,p_spec);
		reply.saveCompleted = true;
		p2cstrcpy(FileName[wTimeBase],p_spec->name);
		TheVRefNum[wTimeBase] = p_spec->vRefNum;
		WindowParID[wTimeBase] = p_spec->parID;
		SetName(wTimeBase,TRUE,TRUE);
		Created[wTimeBase] = TRUE;
		Dirty[wTimeBase] = FALSE;
		err = NSWCleanupReply(&reply);
		return(OK);
		}
	else {
		MyPtoCstr(MAXNAME,fn,LineBuff);
		sprintf(Message,"=> Error creating '%s'",LineBuff);
		Alert1(Message);
		}
	}
err = NSWCleanupReply(&reply);
return(MISSED);
}


SaveCsoundInstruments(FSSpec* p_spec)
// Obsolete
{
int i,j,good,ishtml;
short refnum;
Str255 fn;
long x,count,p,q;
NSWReply reply;
char line[MAXFIELDCONTENT];
OSErr err;

err = NSWInitReply(&reply);
if(FileName[wCsoundResources][0] != '\0') strcpy(Message,FileName[wCsoundResources]);
else if (GetDefaultFileName(wCsoundResources, Message) != OK) return(MISSED);
c2pstrcpy(fn, Message);
p_spec->vRefNum = TheVRefNum[wCsoundResources];
p_spec->parID = WindowParID[wCsoundResources];
CopyPString(fn,p_spec->name);
good = NO;
if(Created[wCsoundResources]) good = (MyOpen(p_spec,fsCurPerm,&refnum) == noErr);
if(good) goto WRITE;
reply.sfFile.vRefNum = TheVRefNum[wCsoundResources];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[wCsoundResources];
if(NewFile(-1,gFileType[wCsoundResources],fn,&reply)) {
	i = CreateFile(wCsoundResources,-1,gFileType[wCsoundResources],fn,&reply,&refnum);
	*p_spec = reply.sfFile;
	if(i == ABORT) {
		err = NSWCleanupReply(&reply);
		return(MISSED);
		}
	if(i == OK) {
WRITE:
		SaveOn++;
		WriteHeader(wCsoundResources,refnum,*p_spec);
		sprintf(line,"%ld",(long)MAXCHAN);
		WriteToFile(NO,MAC,line,refnum);
		for(i=1; i <= MAXCHAN; i++) {
			sprintf(line,"%ld",(long)WhichCsoundInstrument[i]);
			WriteToFile(NO,MAC,line,refnum);
			}
		WriteToFile(YES,MAC,CsoundOrchestraName,refnum);
		sprintf(line,"%ld",(long)Jinstr);
		WriteToFile(NO,MAC,line,refnum);
		for(j=0; j < Jinstr; j++) {
			PleaseWait();
			MystrcpyHandleToString(MAXFIELDCONTENT,0,line,(*pp_CsInstrumentName)[j]);
			ShowMessage(TRUE,wMessage,line);
			WriteToFile(YES,MAC,line,refnum);
			MystrcpyHandleToString(MAXFIELDCONTENT,0,line,
				(*pp_CsInstrumentComment)[j]);
			WriteToFile(YES,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].iargmax);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrumentIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsDilationRatioIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsAttackVelocityIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsReleaseVelocityIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsPitchIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsPitchFormat)[j]);
			WriteToFile(NO,MAC,line,refnum);
			
			sprintf(line,"%.4f",(*p_CsInstrument)[j].pitchbendrange);
			WriteToFile(NO,MAC,line,refnum);
			
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].rPitchBend.islogx);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].rPitchBend.islogy);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].rVolume.islogx);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].rVolume.islogy);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].rPressure.islogx);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].rPressure.islogy);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].rModulation.islogx);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].rModulation.islogy);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].rPanoramic.islogx);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].rPanoramic.islogy);
			WriteToFile(NO,MAC,line,refnum);
			
			sprintf(line,"%ld",(long)(*p_CsPitchBendStartIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsVolumeStartIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsPressureStartIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsModulationStartIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsPanoramicStartIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsPitchBendEndIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsVolumeEndIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsPressureEndIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsModulationEndIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsPanoramicEndIndex)[j]);
			WriteToFile(NO,MAC,line,refnum);
			
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].pitchbendtable);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].volumetable);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].pressuretable);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].modulationtable);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].panoramictable);
			WriteToFile(NO,MAC,line,refnum);
			
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].pitchbendGEN);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].volumeGEN);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].pressureGEN);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].modulationGEN);
			WriteToFile(NO,MAC,line,refnum);
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].panoramicGEN);
			WriteToFile(NO,MAC,line,refnum);
			
			for(i=0; i < 6; i++) {
				WriteFloatToLine(line,(*(p_CsPitchBend[i]))[j]);
				WriteToFile(NO,MAC,line,refnum);
				WriteFloatToLine(line,(*(p_CsVolume[i]))[j]);
				WriteToFile(NO,MAC,line,refnum);
				WriteFloatToLine(line,(*(p_CsPressure[i]))[j]);
				WriteToFile(NO,MAC,line,refnum);
				WriteFloatToLine(line,(*(p_CsModulation[i]))[j]);
				WriteToFile(NO,MAC,line,refnum);
				WriteFloatToLine(line,(*(p_CsPanoramic[i]))[j]);
				WriteToFile(NO,MAC,line,refnum);
				}
			
			sprintf(line,"%ld",(long)(*p_CsInstrument)[j].ipmax);
			WriteToFile(NO,MAC,line,refnum);
			for(i=0; i < (*p_CsInstrument)[j].ipmax; i++) {
				PleaseWait();
				if((*p_CsInstrument)[j].paramlist == NULL) {
					if(Beta) Alert1("=> Err. SaveCsoundInstruments(). (*p_CsInstrument)[j].paramlist == NULL");
					break;
					}
				MystrcpyHandleToString(MAXFIELDCONTENT,0,line,
					(*((*p_CsInstrument)[j].paramlist))[i].name);
				WriteToFile(YES,MAC,line,refnum);
				MystrcpyHandleToString(MAXFIELDCONTENT,0,line,
					(*((*p_CsInstrument)[j].paramlist))[i].comment);
				WriteToFile(YES,MAC,line,refnum);
				sprintf(line,"%ld\n%ld",
					(long)(*((*p_CsInstrument)[j].paramlist))[i].startindex,
					(long)(*((*p_CsInstrument)[j].paramlist))[i].endindex);
				WriteToFile(NO,MAC,line,refnum);
				if(Version > 12) {
					sprintf(line,"%ld",
						(long)(*((*p_CsInstrument)[j].paramlist))[i].table);
					WriteToFile(NO,MAC,line,refnum);
					sprintf(line,"%.3f",
						(*((*p_CsInstrument)[j].paramlist))[i].defaultvalue);
					WriteToFile(NO,MAC,line,refnum);
					sprintf(line,"%ld",
						(long)(*((*p_CsInstrument)[j].paramlist))[i].GENtype);
					WriteToFile(NO,MAC,line,refnum);
					sprintf(line,"%ld",
						(long)(*((*p_CsInstrument)[j].paramlist))[i].combinationtype);
					WriteToFile(NO,MAC,line,refnum);
					}
				}
			}
		WriteToFile(NO,MAC,"_begin tables",refnum);
		UpdateWindow(FALSE,Window[wCsoundTables]); /* Update text length */
		ShowSelect(CENTRE,wCsoundTables);
		ishtml = IsHTML[wCsoundTables];
		IsHTML[wCsoundTables] = TRUE;
		NoReturnWriteToFile("<HTML>",refnum);
		WriteFile(NO,MAC,refnum,wCsoundTables,GetTextLength(wCsoundTables));
		NoReturnWriteToFile("</HTML>",refnum);
		IsHTML[wCsoundTables] = ishtml;
		WriteEnd(-1,refnum);
		GetFPos(refnum,&count);
		SetEOF(refnum,count);
		FlushFile(refnum);
		MyFSClose(wCsoundResources,refnum,p_spec);
		reply.saveCompleted = true;
		p2cstrcpy(FileName[wCsoundResources],p_spec->name);
		TheVRefNum[wCsoundResources] = p_spec->vRefNum;
		WindowParID[wCsoundResources] = p_spec->parID;
		SetName(wCsoundResources,TRUE,TRUE);
		Created[wCsoundResources] = TRUE;
		Dirty[wCsoundResources] = FALSE;
		ClearMessage();
		if(SaveOn > 0) SaveOn--;
		err = NSWCleanupReply(&reply);
		return(OK);
		}
	else {
		MyPtoCstr(MAXNAME,fn,LineBuff);
		sprintf(Message,"=> Error creating '%s'",LineBuff);
		Alert1(Message);
		}
	}
err = NSWCleanupReply(&reply);
return(MISSED);
}


SaveSettings(int startup,int openexisting,Str255 fn,FSSpec* p_spec)
{
short refnum;
int i,io,imax,j,rep,w,good,ishtml,result;
NSWReply reply;
long count,a,b;
char line[64];
GrafPtr saveport;
Rect r;
Point p,q;
OSErr err;

if(ScriptExecOn) return(OK);
err = NSWInitReply(&reply);
if(fn[0] == 0) {
	if (GetDefaultFileName(iSettings, line) != OK) return(MISSED);
	c2pstrcpy(fn, line);
	CopyPString(fn,p_spec->name);
	}
good = NO;
if(openexisting) good = (MyOpen(p_spec,fsCurPerm,&refnum) == noErr);
if(startup && !good) {
	Alert1("Could not open startup settings file -se.startup.");
	return(MISSED);
	}
p2cstrcpy(line,p_spec->name);
if(good) goto WRITE;
reply.sfFile.vRefNum = TheVRefNum[iSettings];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[iSettings];
result = MISSED;

if(NewFile(iSettings,gFileType[iSettings],fn,&reply)) {
	io = CreateFile(iSettings,iSettings,gFileType[iSettings],fn,&reply,&refnum);
	*p_spec = reply.sfFile;
	if(io == ABORT) {
		err = NSWCleanupReply(&reply);
		return(MISSED);
		}
	p2cstrcpy(line,fn);
	if(io == OK) {
WRITE:
		SaveOn++;
		sprintf(Message,"Saving '%s'...",line);
		PleaseWait();
		ShowMessage(TRUE,wMessage,Message);
		WriteHeader(iSettings,refnum,*p_spec);
		sprintf(LineBuff,"%ld",(long)Port);
		WriteToFile(NO,MAC,LineBuff,refnum);
		
		WriteToFile(NO,MAC," ",refnum);
		
		sprintf(LineBuff,"%ld\n%ld\n%ld",(long)Quantization,(long)Time_res,(long)SetUpTime);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)QuantizeOK); WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)Nature_of_time); WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%.0f",(double)Pclock); WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%.0f",(double)Qclock); WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)Jbutt); WriteToFile(NO,MAC,LineBuff,refnum);
		for(i=0; i < Jbutt; i++) {	/* Store radio button settings */
			PleaseWait();
			sprintf(LineBuff,"%ld",(long)GetControlValue(Hbutt[i]));
			WriteToFile(NO,MAC,LineBuff,refnum);
			}
		sprintf(LineBuff,"%ld\n%ld\n%ld\n%ld\n%ld",(long)SplitTimeObjects,(long)SplitVariables,
			(long)UseTextColor,(long)DeftBufferSize,(long)UseGraphicsColor);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)UseBufferLimit); WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)TimeMax); WriteToFile(NO,MAC,LineBuff,refnum);
		GetSeed();
		sprintf(LineBuff,"%.0f\n%ld",(double)Seed,(long)Token);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld\n%ld\n%ld",(long)NoteConvention,(long)StartFromOne,
			(long)SmartCursor);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld\n%ld",(long)GraphicScaleP,(long)GraphicScaleQ);
		WriteToFile(NO,MAC,LineBuff,refnum);

		// old settings for OMS
		sprintf(LineBuff,"<no input device>");
		WriteToFile(NO,MAC,LineBuff,refnum);
		MoveDisk();
		sprintf(LineBuff,"<no output device>");
		WriteToFile(NO,MAC,LineBuff,refnum);
			
		sprintf(LineBuff,"%ld",(long)UseBullet); WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)PlayTicks); WriteToFile(NO,MAC,LineBuff,refnum);
		
		sprintf(LineBuff,"%ld\n%ld",(long)FileSaveMode,(long)FileWriteMode);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld\n%ld\n%ld\n%.2f",(long)MIDIfileType,(long)CsoundFileFormat,
			(long)ProgNrFrom,EndFadeOut);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)C4key);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%.4f",A4freq);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)StrikeAgainDefault);
		WriteToFile(NO,MAC,LineBuff,refnum);
		
		sprintf(LineBuff,"%ld\n%ld\n%ld\n%ld\n%ld\n%ld",(long)DeftVolume,(long)VolumeController,
			(long)DeftVelocity,(long)DeftPanoramic,(long)PanoramicController,(long)SamplingRate);
		WriteToFile(NO,MAC,LineBuff,refnum);
			
		sprintf(LineBuff,"%ld",(long)WMAX); WriteToFile(NO,MAC,LineBuff,refnum);
		for(w=0; w < WMAX; w++) {
			sprintf(LineBuff,"%ld",(long)WindowTextSize[w]);
			WriteToFile(NO,MAC,LineBuff,refnum);
			}
		sprintf(LineBuff,"%ld",(long)MIDIoutputFilter); WriteToFile(NO,MAC,LineBuff,refnum);
		for(i=0; i < 12; i++) {
			sprintf(LineBuff,"%ld",(long)NameChoice[i]); WriteToFile(NO,MAC,LineBuff,refnum);
			}
		sprintf(LineBuff,"%ld",(long)MIDIinputFilter); WriteToFile(NO,MAC,LineBuff,refnum);
		
		if(Version > 19) {
			sprintf(LineBuff,"%ld",(long)ShowObjectGraph);
			WriteToFile(NO,MAC,LineBuff,refnum);
			sprintf(LineBuff,"%ld",(long)ShowPianoRoll);
			WriteToFile(NO,MAC,LineBuff,refnum);
			sprintf(LineBuff,"%ld",(long)MAXCHAN);
			WriteToFile(NO,MAC,LineBuff,refnum);
			for(i=0; i < MAXCHAN; i++) {
				PleaseWait();
				sprintf(LineBuff,"%ld",(long)PianoColor[i].red);
				WriteToFile(NO,MAC,LineBuff,refnum);
				sprintf(LineBuff,"%ld",(long)PianoColor[i].green);
				WriteToFile(NO,MAC,LineBuff,refnum);
				sprintf(LineBuff,"%ld",(long)PianoColor[i].blue);
				WriteToFile(NO,MAC,LineBuff,refnum);
				}
			}
			
		sprintf(LineBuff,"%ld",(long)(NewEnvironment && !FreezeWindows));
			WriteToFile(NO,MAC,LineBuff,refnum);
		if(NewEnvironment && !FreezeWindows) {
			sprintf(LineBuff,"%ld",(long)WMAX); WriteToFile(NO,MAC,LineBuff,refnum);
			GetPort(&saveport);
			for(w=0; w < WMAX; w++) {
				PleaseWait();
				MoveDisk();
				sprintf(LineBuff,"%ld",(long)ChangedCoordinates[w]);
				SetPortWindowPort(Window[w]);
				WriteToFile(NO,MAC,LineBuff,refnum);
				if(ChangedCoordinates[w]) {
					GetWindowPortBounds(Window[w], &r);
					p = topLeft(r);
					LocalToGlobal(&p);
					q = botRight(r);
					LocalToGlobal(&q);
					sprintf(LineBuff,"%ld",(long)(p.v));	/* top */
					WriteToFile(NO,MAC,LineBuff,refnum);
					sprintf(LineBuff,"%ld",(long)(p.h));	/* left */
					WriteToFile(NO,MAC,LineBuff,refnum);
					sprintf(LineBuff,"%ld",(long)(q.v));	/* bottom */
					WriteToFile(NO,MAC,LineBuff,refnum);
					sprintf(LineBuff,"%ld",(long)(q.h));	/* right */
					WriteToFile(NO,MAC,LineBuff,refnum);
					}
				}
			if(saveport != NULL) SetPort(saveport);
			else if(Beta) Alert1("=> Err SaveSettings(). saveport == NULL");
			}
		imax = MAXCOLOR; if(!NewColors) imax = 0;
		sprintf(LineBuff,"%ld",(long)imax); WriteToFile(NO,MAC,LineBuff,refnum);
		if(imax > 0) {
			for(i=0; i < imax; i++) {
				PleaseWait();
				sprintf(LineBuff,"%ld",(long)(Color[i].red)); WriteToFile(NO,MAC,LineBuff,refnum);
				sprintf(LineBuff,"%ld",(long)(Color[i].green)); WriteToFile(NO,MAC,LineBuff,refnum);
				sprintf(LineBuff,"%ld",(long)(Color[i].blue)); WriteToFile(NO,MAC,LineBuff,refnum);
				}
			}
LASTPART:
		WriteToFile(NO,MAC,"STARTSTRING:",refnum);
		UpdateWindow(FALSE,Window[wStartString]); /* Update text length */
		ShowSelect(CENTRE,wStartString);
		ishtml = IsHTML[wStartString];
		IsHTML[wStartString] = TRUE;
		NoReturnWriteToFile("<HTML>",refnum);
		WriteFile(NO,MAC,refnum,wStartString,GetTextLength(wStartString));
		NoReturnWriteToFile("</HTML>",refnum);
		IsHTML[wStartString] = ishtml;
		WriteEnd(-1,refnum);
		GetFPos(refnum,&count);
		SetEOF(refnum,count);
		FlushFile(refnum);
		MyFSClose(iSettings,refnum,p_spec);
		reply.saveCompleted = true;
		Dirty[iSettings] = FALSE;
		if(!startup) {
			Created[iSettings] = TRUE;
			p2cstrcpy(FileName[iSettings],fn);
			SetName(iSettings,TRUE,TRUE);
			TheVRefNum[iSettings] = p_spec->vRefNum;
			WindowParID[iSettings] = p_spec->parID;
			}
		result = OK;
		if(SaveOn > 0) SaveOn--;
		}
	else {
		MyPtoCstr(MAXNAME,fn,line);
		sprintf(Message,"=> Error creating '%s'",line);
		Alert1(Message);
		result = MISSED;
		}
	}
err = NSWCleanupReply(&reply);
HideWindow(Window[wMessage]);
return(result);
}


/* FindBPPrefsFolder() gets an FSSpec for the folder named "Bol Processor"
   in the system (or user) preferences folder.  If this folder does not
   exist, it is created.  */
OSErr	FindBPPrefsFolder(FSSpecPtr location)
{
	OSErr	err;
	short	systemVRefNum;
	long	prefDirID;
	long	bpDirID;
	
	err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, 
			&systemVRefNum, &prefDirID);
	if (err != noErr) return err;
	
	err = FSMakeFSSpec(systemVRefNum, prefDirID, kBPPrefsFolder, location);
	if (err == fnfErr) {
		// need to create the "Bol Processor" folder in the Preferences folder
		err = FSpDirCreate(location, smSystemScript, &bpDirID);
		if (err != noErr) return err;
		location->parID = bpDirID;
		err = FSMakeFSSpec(systemVRefNum, prefDirID, kBPPrefsFolder, location);
	}
	else if (err != noErr) return err;
	else {
		Boolean targetIsFolder;
		Boolean wasAliased;

		err = ResolveAliasFile(location, true, &targetIsFolder, &wasAliased);
		if (err != noErr) return err;
		if (!targetIsFolder) return paramErr; // "Bol Processor" is a file, not a folder
	}

	return noErr;
}

/* FindFileInPrefsFolder() gets an FSSpec for the file named filename that
   should be in the "Bol Processor" sub-folder of the system (or user) preferences 
   folder.  If this file does not exist, fnfErr is returned and location is valid (?).
   filename may be a relative pathname up to 255 chars long. 
   FSSpec returned may not be in prefs folder if the file was aliased. */
OSErr	FindFileInPrefsFolder(FSSpecPtr location, StringPtr filename)
{
	OSErr	 err;
	short	 prefVRefNum;
	long	 bpDirID;
	
	err = FindBPPrefsFolder(location);
	if (err != noErr)	return err;
	
	err = GetFolderID(location, &bpDirID);
	if (err != noErr)	return err;
	prefVRefNum = location->vRefNum;
	err = FSMakeFSSpec(prefVRefNum, bpDirID, filename, location);
	if (err != noErr) return err;
	else	{
		Boolean targetIsFolder;
		Boolean wasAliased;

		err = ResolveAliasFile(location, true, &targetIsFolder, &wasAliased);
		if (targetIsFolder) return paramErr; // there is a folder where the file should be
	}

	return noErr;
}

/* GetFolderID() gets the folder ID for a directory specified via a full FSSpec. */
OSErr	GetFolderID(const FSSpecPtr loc, long* dirID)
{
	OSErr	err;
	CInfoPBRec	catinfo;
	Str255	name;
	
	CopyPString(loc->name, name);
	catinfo.dirInfo.ioCompletion = NULL;
	catinfo.dirInfo.ioNamePtr = name;
	catinfo.dirInfo.ioVRefNum = loc->vRefNum;
	catinfo.dirInfo.ioFDirIndex = 0;	// we want info about the named folder
	catinfo.dirInfo.ioDrDirID = loc->parID;
	
	err = PBGetCatInfo(&catinfo, false);
	*dirID = catinfo.dirInfo.ioDrDirID;
	return err;
}


/* Copy -se.startup from BP application bundle to "Bol Processor" subfolder of the
   user's preferences folder if it does not exist there already - akozar 040607 
   (If called on OS 9, this would copy from BP app's folder to system prefs) */
int CopyStartupSettings()
{
	OSErr  err;
	long	 prefDirID;
	FSSpec existing, copy;
	
	// locate the file in BP's bundle
	err = FSMakeFSSpec(RefNumbp2, ParIDbp2, kBPSeStartup, &existing);
	if (err != noErr) return (MISSED);
	
	// make an FSSpec for the copy to be made in prefs folder
	err = FindBPPrefsFolder(&copy);
	if (err != noErr)	return err;
	err = GetFolderID(&copy, &prefDirID);
	if (err != noErr)	return err;
	err = FSMakeFSSpec(copy.vRefNum, prefDirID, kBPSeStartup, &copy);

	if (err == noErr)  return (OK);	// already exists
	if (err != fnfErr) return (MISSED);
	
	err = CopyFile(&existing, &copy);
	if (err != noErr) return (MISSED);
	return (OK);
}

void GetStartupSettingsSpec(FSSpecPtr spec)
{
	OSErr err;
	Boolean haveStartupSpec = FALSE;
	
	// on OS X, check the user's preferences folder first
	if (RunningOnOSX) {
		if (FindFileInPrefsFolder(spec, kBPSeStartup) == noErr)
			haveStartupSpec = TRUE;
		else {  
			if (CopyStartupSettings() == OK &&
			    FindFileInPrefsFolder(spec, kBPSeStartup) == noErr)
				haveStartupSpec = TRUE;
			}
		}
	if (!haveStartupSpec) {
		// use the BP2 application folder copy if needed
		err = FSMakeFSSpec(RefNumbp2, ParIDbp2, kBPSeStartup, spec);
		// if err, fill in spec ourselves (will probably fail to open, 
		// but we need to avoid returning completely random values)
		if (err != noErr && err != fnfErr) {
			spec->vRefNum = RefNumbp2;
			spec->parID = ParIDbp2;
			CopyPString(kBPSeStartup, spec->name);
			}
		}

	return;
}

#endif /* BP_CARBON_GUI_FORGET_THIS */


int LoadCsoundInstruments(int checkversion,int tryname) 
{
int i,io,iv,ip,jmax,j,result,y,maxticks,maxbeats,arg,length,i_table,ipmax;
char **ptr, line[MAXLIN], note_names[MAXLIN], key_numbers[MAXLIN], fractions[MAXLIN], baseoctave_string[10];
Handle **ptr2;
CsoundParam **ptr3;
long pos,x;
char **p_line,**p_completeline;
double r;
FILE* csfile;

if(trace_load_csound_resources) BPPrintMessage(odInfo, "LoadCsoundInstruments(%d,%d)\n",checkversion,tryname);

if(LoadedCsoundInstruments) return(OK);

iCsoundInstrument = 0;
LoadOn++;
pos = ZERO;
Dirty[wCsoundResources] = CompiledRegressions = CompiledCsObjects = FALSE;
p_line = p_completeline = NULL;

if(trace_load_csound_resources) BPPrintMessage(odInfo,"Opening Csound resource file %s\n",FileName[wCsoundResources]);

// Check whether file is being saved
sprintf(line,"%s_lock",FileName[wCsoundResources]);
csfile = fopen(line,"r");
if(csfile != NULL) {
	BPPrintMessage(odError,"\n=> As the Csound resource file was locked I waited for 3 seconds...\n");
//	sleep(2);
	delay(3); // Fixed by BB 2022-02-20
	}

csfile = fopen(FileName[wCsoundResources],"r");
if(csfile == NULL) {
	BPPrintMessage(odError,"=> Could not find and open Csound resource file %s\n",FileName[wCsoundResources]);
	goto ERR;
	}

if(ReadOne(FALSE,FALSE,FALSE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
BPPrintMessage(odInfo,"Loading: %s...\n",FileName[wCsoundResources]);

if(trace_load_csound_resources) BPPrintMessage(odInfo, "Line = %s\n",*p_line);
if(CheckVersion(&iv,p_line,FileName[wCsoundResources]) != OK) goto ERR;
if(ReadOne(FALSE,TRUE,FALSE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
// GetDateSaved(p_completeline,&(p_FileInfo[wCsoundResources]));
if(ReadInteger(csfile,&jmax,&pos) == MISSED) goto ERR;
if(jmax < 16 || jmax > MAXCHAN) {
	BPPrintMessage(odError,"=> This file is empty or in an unknown format\n");
	goto QUIT;
	}
for(j=1; j <= jmax; j++) {
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	WhichCsoundInstrument[j] = i;
	}
if(iv > 11) {
	if(ReadOne(FALSE,TRUE,FALSE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	MystrcpyHandleToString(MAXNAME,0,CsoundOrchestraName,p_completeline);
	}
else CsoundOrchestraName[0] = '\0';
if(trace_load_csound_resources) BPPrintMessage(odInfo,"CsoundOrchestraName = %s\n",CsoundOrchestraName);
if(ReadInteger(csfile,&jmax,&pos) == MISSED) goto ERR;
if(trace_load_csound_resources) BPPrintMessage(odInfo,"jmax = %d\n",jmax);
if(jmax < 0) {
	BPPrintMessage(odError,"=> This file is empty or in an unknown format\n");
	goto QUIT;
	}
if(jmax > 0 && (result=ResizeCsoundInstrumentsSpace(jmax)) != OK) goto ERR;
result = MISSED;

if(trace_load_csound_resources) BPPrintMessage(odInfo,"Jinstr = %d\n",Jinstr);

for(j=0; j < jmax; j++) {
	ResetCsoundInstrument(j,YES,YES);
	if(trace_load_csound_resources) BPPrintMessage(odInfo,"j = %d\n",j);
	if(ReadOne(FALSE,TRUE,FALSE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
//	ptr = (*pp_CsInstrumentName)[j];
	ptr = NULL; // Fixed by BB 2021-02-14
	if((*p_completeline)[0] != '\0') {
		MystrcpyHandleToString(MAXLIN,0,LineBuff,p_completeline);
	//	if(ShowMessages) ShowMessage(TRUE,wMessage,LineBuff);
		if(MySetHandleSize((Handle*)&ptr,(1L + MyHandleLen(p_completeline)) * sizeof(char)) != OK)
				goto ERR;
		MystrcpyHandleToHandle(0,&ptr,p_completeline);
		(*pp_CsInstrumentName)[j] = ptr;
		}
	else strcpy((*((*pp_CsInstrumentName)[j])),""); // (*((*pp_CsInstrumentName)[j]))[0] = '\0'; Fixed by BB 2021-02-14
	
	if(trace_load_csound_resources) BPPrintMessage(odInfo,"Loading Csound instrument %d = \"%s\" out of %d\n",(j+1),(*((*pp_CsInstrumentName)[j])),jmax);
//	p_line = p_completeline = NULL; 
	if(ReadOne(FALSE,TRUE,FALSE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
//	ptr = (*pp_CsInstrumentComment)[j];
	ptr = NULL; // Fixed by BB 2021-02-14
	if((*p_completeline)[0] != '\0') {
		if(MySetHandleSize((Handle*)&ptr,(1L + MyHandleLen(p_completeline)) * sizeof(char)) != OK)
			goto ERR;
		MystrcpyHandleToHandle(0,&ptr,p_completeline);
		(*pp_CsInstrumentComment)[j] = ptr;
		if(trace_load_csound_resources) BPPrintMessage(odInfo,"Comment: %s\n",(*p_completeline));
		}
	else strcpy((*((*pp_CsInstrumentComment)[j])),""); // (*((*pp_CsInstrumentComment)[j]))[0] = '\0'; Fixed by BB 2021-02-14
	
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].iargmax = i;
	if(trace_load_csound_resources) BPPrintMessage(odInfo,"iargmax = %d\n",i);
	
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrumentIndex)[j] = i;
	if(trace_load_csound_resources) BPPrintMessage(odInfo,"CsInstrumentIndex = %d\n",i);
	
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsDilationRatioIndex)[j] = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsAttackVelocityIndex)[j] = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsReleaseVelocityIndex)[j] = i;
	
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsPitchIndex)[j] = i;
	
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsPitchFormat)[j] = i;
	
	if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
	if(r == -1.) r = (float) DeftPitchbendRange;
	(*p_CsInstrument)[j].pitchbendrange = r;
	if(trace_load_csound_resources) BPPrintMessage(odInfo,"pitchbendrange = %.3f\n",r);
	
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].rPitchBend.islogx = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].rPitchBend.islogy = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].rVolume.islogx = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].rVolume.islogy = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].rPressure.islogx = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].rPressure.islogy = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].rModulation.islogx = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].rModulation.islogy = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].rPanoramic.islogx = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].rPanoramic.islogy = i;
	
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsPitchBendStartIndex)[j] = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsVolumeStartIndex)[j] = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsPressureStartIndex)[j] = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsModulationStartIndex)[j] = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsPanoramicStartIndex)[j] = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsPitchBendEndIndex)[j] = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsVolumeEndIndex)[j] = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsPressureEndIndex)[j] = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsModulationEndIndex)[j] = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsPanoramicEndIndex)[j] = i;
	if(trace_load_csound_resources) BPPrintMessage(odInfo,"PanoramicEndIndex = %d\n",i);

	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].pitchbendtable = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].volumetable = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].pressuretable = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].modulationtable = i;
	if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
	(*p_CsInstrument)[j].panoramictable = i;
	if(trace_load_csound_resources) BPPrintMessage(odInfo,"panoramictable = %d\n",i);
	
	if(iv > 13) {
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].pitchbendGEN = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].volumeGEN = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].pressureGEN = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].modulationGEN = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].panoramicGEN = i;
		if(trace_load_csound_resources) BPPrintMessage(odInfo,"panoramicGEN = %d\n",i);
		}
	else {
		(*p_CsInstrument)[j].pitchbendGEN = (*p_CsInstrument)[j].volumeGEN
			= (*p_CsInstrument)[j].pressureGEN = (*p_CsInstrument)[j].modulationGEN
			= (*p_CsInstrument)[j].panoramicGEN = 7;
		}

	for(i=0; i < 6; i++) {
		if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
		(*(p_CsPitchBend[i]))[j] = r;
		if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
		(*(p_CsVolume[i]))[j] = r;
		if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
		(*(p_CsPressure[i]))[j] = r;
		if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
		(*(p_CsModulation[i]))[j] = r;
		if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
		(*(p_CsPanoramic[i]))[j] = r;
		}
	
	if(trace_load_csound_resources) BPPrintMessage(odInfo,"ipmax = %d j = %d\n",(*p_CsInstrument)[j].ipmax,j);
	
	for(ip=0; ip < (*p_CsInstrument)[j].ipmax; ip++) {
		if((*p_CsInstrument)[j].paramlist == NULL) {
			Alert1("=> Err. LoadCsoundInstruments(). (*p_CsInstrument)[j].paramlist == NULL");
			break;
			}
		ptr = (*((*p_CsInstrument)[j].paramlist))[ip].name;
		MyDisposeHandle((Handle*)&ptr);
		(*((*p_CsInstrument)[j].paramlist))[ip].name = NULL;
		ptr = (*((*p_CsInstrument)[j].paramlist))[ip].comment;
		MyDisposeHandle((Handle*)&ptr);
		(*((*p_CsInstrument)[j].paramlist))[ip].comment = NULL;
		}
	ptr3 = (*p_CsInstrument)[j].paramlist;
	MyDisposeHandle((Handle*)&ptr3);
	(*p_CsInstrument)[j].paramlist = NULL;
	(*p_CsInstrument)[j].ipmax = 0;
	
	if(ReadInteger(csfile,&ipmax,&pos) == MISSED) goto ERR;
	if(trace_load_csound_resources) BPPrintMessage(odInfo,"Read ipmax again = %d\n",ipmax);
	if(ipmax < 1) continue;
	if((ptr3=(CsoundParam**) GiveSpace((Size)(ipmax * sizeof(CsoundParam)))) == NULL)
		goto ERR;
	(*p_CsInstrument)[j].paramlist = ptr3;
	(*p_CsInstrument)[j].ipmax = ipmax;
	for(ip=0; ip < ipmax; ip++) {
		(*((*p_CsInstrument)[j].paramlist))[ip].name = NULL;
		(*((*p_CsInstrument)[j].paramlist))[ip].comment = NULL;
		ResetMoreParameter(j,ip);
		}
	
	for(ip=0; ip < (*p_CsInstrument)[j].ipmax; ip++) {
		if(ReadOne(FALSE,TRUE,FALSE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
		length = MyHandleLen(p_completeline);
		if(length > 0) {
			if((ptr=(char**) GiveSpace((Size)((1L + length)
				* sizeof(char)))) == NULL) goto ERR;
			MystrcpyHandleToHandle(0,&ptr,p_completeline);
			(*((*p_CsInstrument)[j].paramlist))[ip].name = ptr;
			MyLock(FALSE,(Handle)p_completeline);
			i = FixStringConstant(*p_completeline);
			MyUnlock((Handle)p_completeline);
			if(i >= 0) (*((*p_CsInstrument)[j].paramlist))[ip].nameindex = i;
			}
		
		if(trace_load_csound_resources) BPPrintMessage(odInfo,"-> Parameter: \"%s\"\n",*p_completeline);
		strcpy(line,"");
		fscanf(csfile, "%[^\n]",line); // Necessary to read a line that might be empty
		if(strlen(line) > 0) {
			MystrcpyStringToHandle(&ptr,line);
			(*((*p_CsInstrument)[j].paramlist))[ip].comment = ptr;
			pos += strlen(line);
			}
		if(trace_load_csound_resources) BPPrintMessage(odInfo, "Comment: %s\n",line);
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*((*p_CsInstrument)[j].paramlist))[ip].startindex = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*((*p_CsInstrument)[j].paramlist))[ip].endindex = i;
		if(iv > 12) {
			if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
			(*((*p_CsInstrument)[j].paramlist))[ip].table = i;
			if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
			(*((*p_CsInstrument)[j].paramlist))[ip].defaultvalue = r;
			if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
			(*((*p_CsInstrument)[j].paramlist))[ip].GENtype = i;
			if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
			(*((*p_CsInstrument)[j].paramlist))[ip].combinationtype = i;
			if(i == MULT && fabs(r) < 0.01) {
				(*((*p_CsInstrument)[j].paramlist))[ip].defaultvalue = 1.;
				sprintf(Message,"In instrument %ld a default parameter value '%.3f' was replaced with '1' because its combination mode is multiplicative",
					(long)(*p_CsInstrumentIndex)[j],r);
				Alert1(Message);
				}
			}
		else {
			(*((*p_CsInstrument)[j].paramlist))[ip].table = -1;
			(*((*p_CsInstrument)[j].paramlist))[ip].defaultvalue = 0.;
			(*((*p_CsInstrument)[j].paramlist))[ip].GENtype = 7;
			(*((*p_CsInstrument)[j].paramlist))[ip].combinationtype = ADD;
			}
		}
	}
BPPrintMessage(odInfo, "All instruments have been loaded\n");
if(ReadOne(FALSE,FALSE,TRUE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto QUIT;

if(Mystrcmp(p_line,"_begin tables") == 0) {
	i_table = 0;
	strcpy(line,""); strcpy(note_names,""); strcpy(key_numbers,""); strcpy(baseoctave_string,""); strcpy(fractions,"");
	while(TRUE) {
		if(ReadOne(FALSE,FALSE,TRUE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto QUIT;
		Strip(*p_line);
		if(strlen(*p_line) == 0) goto QUIT; // Required because 'pos' is not incremented when reading an empty line
		if(Mystrcmp(p_line,"_end tables") == 0) break;
		if(trace_load_csound_resources) BPPrintMessage(odInfo, "table line = [%s]\n",*p_line);
		if(i_table >= MaxCsoundTables) {
			p_CsoundTables = (char****) IncreaseSpace((Handle)p_CsoundTables);
			MaxCsoundTables = (MyGetHandleSize((Handle)p_CsoundTables) / sizeof(char**));
			for(i = i_table; i < MaxCsoundTables; i++) (*p_CsoundTables)[i] = NULL;
			}
		if((*p_line)[0] == '"') { // This line contains the name of the next scale
			MystrcpyHandleToString((strlen(*p_line) - 2),1,line,p_line);
			continue;
			}
		if((*p_line)[0] == '<') continue; // Ignore comments
		if((*p_line)[0] == '[') { // fractions
			MystrcpyHandleToString(strlen(*p_line),0,fractions,p_line);
			continue;
			}
		if((*p_line)[0] == 'c') continue; // Ignore comma
		if((*p_line)[0] == 's') continue; // Ignore series
		if((*p_line)[0] == '|') { // baseoctave
			MystrcpyHandleToString(strlen(*p_line)-2,1,baseoctave_string,p_line);
			continue;
			}
		if((*p_line)[0] == '/') { // This line contains note names for this scale
			MystrcpyHandleToString(strlen(*p_line),0,note_names,p_line);
			continue;
			}
		if((*p_line)[0] == 'k') { // This line contains key numbers for this scale
			MystrcpyHandleToString(strlen(*p_line),0,key_numbers,p_line);
			continue;
			}
		length = MyHandleLen(p_completeline);
		if(length > 0) {
			result = CreateMicrotonalScale(*p_line,line,note_names,key_numbers,fractions,baseoctave_string);
			if(result == EXIT) {
				if((ptr=(char**) GiveSpace((Size)((1L + length)
					* sizeof(char)))) == NULL) goto ERR;
				MystrcpyHandleToHandle(0,&ptr,p_completeline);
				(*p_CsoundTables)[i_table] = ptr;
				i_table++;
				result = OK;
				}
			if(result != OK) goto ERR;
			strcpy(line,""); strcpy(note_names,""); strcpy(key_numbers,""); strcpy(baseoctave_string,"");
			}
		}
	}
goto QUIT;

ERR:
BPPrintMessage(odError,"=> Error reading '%s' Csound resource file...\n",FileName[wCsoundResources]);
// FileName[wCsoundResources][0] = '\0';

QUIT:
MyDisposeHandle((Handle*)&p_line);
MyDisposeHandle((Handle*)&p_completeline);
CloseFile(csfile);
if(result == OK) {
	Created[wCsoundResources] = TRUE;
	LoadedCsoundInstruments = TRUE;
	if(FALSE && NumberScales == 1) {
		BPPrintMessage(odInfo, "\nThis microtonal scale will be used for Csound scores in replacement of the equal-tempered 12-tone scale\nPitch will be adjusted to the diapason\n");
		DefaultScale = -1;
		}
	else DefaultScale = 0; // Don't use scales until the _scale() instruction has been found
	}
else {
	Created[wCsoundResources] = FALSE;
	EmergencyExit = TRUE;
	}

// Dirty[wCsoundResources] = FALSE;
if(iCsoundInstrument >= Jinstr) iCsoundInstrument = 0;
// SetCsoundInstrument(iCsoundInstrument,-1);
LoadOn--;
return(result);
}


int LoadSettings(const char *filename, int startup)
{
int i,j,jmax,rep,result,iv,w,wmax,oldoutmidi,oldoutcsound,oldwritemidifile;
FILE* sefile;
long pos,k;
unsigned long kk;
double x;
char **p_line,**p_completeline;

if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed start LoadSettings = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);

result = OK;
oldoutmidi = OutMIDI;
p_line = p_completeline = NULL;
if(startup) {
	// FIXME: set filename = location of a startup settings file and continue? We'll see later (BB)
	return OK;
	}
else {
	// filename cannot be NULL or empty
	if (filename == NULL || filename[0] == '\0') {
		BPPrintMessage(odError, "=> Err. LoadSettings(): filename was NULL or empty\n");
		return MISSED;
		}
	}

// open the file for reading
sefile = fopen(filename, "r");
if (sefile == NULL) {
	BPPrintMessage(odError, "=> Could not open settings file %s\n", filename);
	return MISSED;
}

pos = ZERO; Dirty[iSettings] = Created[iSettings] = FALSE;

LoadOn++;

if(ReadOne(FALSE,FALSE,FALSE,sefile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
if(CheckVersion(&iv,p_line,filename) != OK) {
	result = MISSED;
	goto QUIT;
	}
	
if(trace_load_settings) BPPrintMessage(odError, "Settings file version %d\n",iv);
if(ReadOne(FALSE,FALSE,FALSE,sefile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;

if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;	// serial port used by old built-in Midi driver
// if(startup) Port = j;

if(ReadOne(FALSE,FALSE,TRUE,sefile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;	// Not used but should be kept for consistency
if(ReadLong(sefile,&k,&pos) == MISSED) goto ERR; Quantization = k;
if(ReadLong(sefile,&k,&pos) == MISSED) goto ERR; Time_res = k;
if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR; SetUpTime = j;
if(trace_load_settings) BPPrintMessage(odError, "SetUpTime = %d\n",SetUpTime);
if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR; QuantizeOK = j;
if(trace_load_settings) BPPrintMessage(odError, "QuantizeOK = %d\n",QuantizeOK);
#if BP_CARBON_GUI_FORGET_THIS
SetTimeAccuracy(); // We'll see later what to do with Time_res
Dirty[wTimeAccuracy] = FALSE;
#endif /* BP_CARBON_GUI_FORGET_THIS */
NotSaidKpress = TRUE;

if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR; Nature_of_time = j;
if(ReadUnsignedLong(sefile,&kk,&pos) == MISSED) goto ERR; Pclock = (double)kk;
if(ReadUnsignedLong(sefile,&kk,&pos) == MISSED) goto ERR; Qclock = (double)kk;
SetTempo(); SetTimeBase(); Dirty[wMetronom] = Dirty[wTimeBase] = FALSE;

if(ReadInteger(sefile,&jmax,&pos) == MISSED) goto ERR;
if(jmax != Jbutt) {
	sprintf(Message,"\nError in settings file:  jmax = %d instead of %d\n",jmax,Jbutt);
	if(Beta) Alert1(Message);
	goto ERR;
	}
	
oldwritemidifile = WriteMIDIfile;
oldoutcsound = OutCsound;

if(ReadInteger(sefile,&Improvize,&pos) == MISSED) goto ERR;
if(trace_load_settings) BPPrintMessage(odError, "Improvize = %d\n",Improvize);
if(PlaySelectionOn) Improvize = 0;
if(ReadInteger(sefile,&CyclicPlay,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&UseEachSub,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&AllItems,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&DisplayProduce,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&StepProduce,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&StepGrammars,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&TraceProduce,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&PlanProduce,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&DisplayItems,&pos) == MISSED) goto ERR; 
if(ReadInteger(sefile,&ShowGraphic,&pos) == MISSED) goto ERR;
// if(ShowGraphic) BPPrintMessage(odInfo,"Graphics activated\n");
if(ReadInteger(sefile,&AllowRandomize,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&DisplayTimeSet,&pos) == MISSED) goto ERR; 
if(ReadInteger(sefile,&StepTimeSet,&pos) == MISSED) goto ERR; 
if(ReadInteger(sefile,&TraceTimeSet,&pos) == MISSED) goto ERR; 
if(jmax > 27) ReadInteger(sefile,&CsoundTrace,&pos);
else CsoundTrace = FALSE;
if(ReadInteger(sefile,&OutMIDI,&pos) == MISSED) goto ERR; 
if(ReadInteger(sefile,&SynchronizeStart,&pos) == MISSED) goto ERR; 
if(ReadInteger(sefile,&ComputeWhilePlay,&pos) == MISSED) goto ERR; 
if(ReadInteger(sefile,&Interactive,&pos) == MISSED) goto ERR; 
if(jmax > 19) ReadInteger(sefile,&ResetWeights,&pos);
else ResetWeights = FALSE;
NeverResetWeights = FALSE;
if(jmax > 20) ReadInteger(sefile,&ResetFlags,&pos);
else ResetFlags = FALSE;
if(jmax > 21) ReadInteger(sefile,&ResetControllers,&pos);
else ResetControllers = FALSE; 
if(jmax > 22) ReadInteger(sefile,&NoConstraint,&pos);
else NoConstraint = FALSE;
if(jmax > 23) ReadInteger(sefile,&WriteMIDIfile,&pos);
else WriteMIDIfile = FALSE;
WriteMIDIfile = FALSE; // This must be set by the command line
if(jmax > 24) ReadInteger(sefile,&ShowMessages,&pos); 
if(jmax > 25) ReadInteger(sefile,&OutCsound,&pos);
else OutCsound = FALSE;
if(jmax > 26) ReadInteger(sefile,&j,&pos); // used to read p_oms
Oms = FALSE;	// OMS is no more

/* Silently reset this flag if real-time Midi is not available.
   Note that this does not mark the settings file as Dirty either.
   -- 012307 akozar */
#if !WITH_REAL_TIME_MIDI_FORGET_THIS
	OutMIDI = FALSE;
//	Improvize = FALSE;
#endif

// if(!OutMIDI) Improvize = FALSE;

SetButtons(TRUE);

#if BP_CARBON_GUI_FORGET_THIS
if(oldoutcsound && !OutCsound && !startup) CloseCsScore();
if(oldwritemidifile && !WriteMIDIfile && !startup) CloseMIDIFile();
if(OutMIDI && !oldoutmidi && !InitOn && !startup) ResetMIDI(FALSE);
#endif /* BP_CARBON_GUI_FORGET_THIS */

if(ReadInteger(sefile,&SplitTimeObjects,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&SplitVariables,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
UseTextColor = (j > 0);
if(ReadLong(sefile,&k,&pos) == MISSED) goto ERR;
if(k < 100) k = 1000;
DeftBufferSize = BufferSize = k;
// BPPrintMessage(odInfo, "Default buffer size = %ld symbols\n", DeftBufferSize);
if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
UseGraphicsColor = (j > 0);
if(ForceTextColor == 1) UseTextColor = TRUE;
if(ForceTextColor == -1) UseTextColor = FALSE;
if(ForceGraphicColor == 1) UseGraphicsColor = TRUE;
if(ForceGraphicColor == -1) UseGraphicsColor = FALSE;
if(ReadInteger(sefile,&UseBufferLimit,&pos) == MISSED) goto ERR;
UseBufferLimit = FALSE;

#if BP_CARBON_GUI_FORGET_THIS
SetBufferSize();
#endif /* BP_CARBON_GUI_FORGET_THIS */

if(ReadLong(sefile,&MaxConsoleTime,&pos) == MISSED) goto ERR; // Previously it was TimeMax
if(trace_load_settings) BPPrintMessage(odInfo, "MaxConsoleTime = %ld\n",(long)MaxConsoleTime);

if(ReadLong(sefile,&k,&pos) == MISSED) goto ERR;
Seed = (unsigned) (k % 32768L);
if(Seed > 0) {
	if(!PlaySelectionOn) BPPrintMessage(odInfo, "Resetting random seed to %u as per settings\n", Seed);
	ResetRandom();
	}
else {
	if(!PlaySelectionOn) BPPrintMessage(odInfo, "No new random seed as per settings\n");
	Randomize();
	}

if(ReadInteger(sefile,&Token,&pos) == MISSED) goto ERR;
if(Token > 0) Token = TRUE;
else Token = FALSE;
if(ReadInteger(sefile,&NoteConvention,&pos) == MISSED) goto ERR;
if(NoteConvention > 4) {
	BPPrintMessage(odInfo, "\n=> ERROR NoteConvention = %d\n",NoteConvention);
	goto ERR;
	}
if(ReadInteger(sefile,&StartFromOne,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
SmartCursor = (j == 1);
if(ReadInteger(sefile,&GraphicScaleP,&pos) == MISSED) goto ERR;
if(ReadInteger(sefile,&GraphicScaleQ,&pos) == MISSED) goto ERR;

#if BP_CARBON_GUI_FORGET_THIS
SetGraphicSettings();
#endif /* BP_CARBON_GUI_FORGET_THIS */

/* Read OMS default input device, and ignore it */
if(ReadOne(FALSE,FALSE,TRUE,sefile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
/* Read OMS default output device, and ignore it */
if(iv > 5) {
	if(ReadOne(FALSE,FALSE,TRUE,sefile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	}

if(iv > 11) {
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	if(iv > 15) UseBullet = j;
	else UseBullet = TRUE;
	UseBullet = FALSE; // FIXED by BB 2020-10-22
//	if(UseBullet) Code[7] = '�'; Requires UTF8 format BB 2022-02-17
//	else
	Code[7] = '.';
	}

PlayTicks = FALSE;
ResetTickFlag = TRUE;

if(iv > 7) {
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	PlayTicks = j;
/*	if(PlayTicks && !InitOn && !startup) {
		ResetMIDI(FALSE); 
		} */
	}
if(iv > 10) {
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	FileSaveMode = ALLSAME;  // was = j;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	FileWriteMode = NOW;     // was = j;
	}
else {
	FileSaveMode = ALLSAME;  // was = ALLSAMEPROMPT;
	FileWriteMode = NOW;     // was = LATER;
	}
if(iv > 11) {
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	MIDIfileType = j;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	CsoundFileFormat = j;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	ProgNrFrom = j;
	if(ProgNrFrom == 0) {
/*		if(Beta) Alert1("Old program numbers"); */
		ProgNrFrom = 1;
		}
	if(ReadFloat(sefile,&x,&pos) == MISSED) goto ERR;
	if(iv > 19) EndFadeOut = x;
	else EndFadeOut = 2.;
	sprintf(Message,"EndFadeOut = %.2f sec\n",EndFadeOut);
//	BPPrintMessage(odInfo,Message);
#if BP_CARBON_GUI_FORGET_THIS
	sprintf(Message,"%.2f",EndFadeOut);
	SetField(FileSavePreferencesPtr,-1,fFadeOut,Message);
#endif /* BP_CARBON_GUI_FORGET_THIS */
	
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	if(j > 1 && j < 128) C4key = j;
	else C4key = 60;
	ReadFloat(sefile,&x,&pos);
	if(x > 1.) A4freq = x;
	else A4freq = 440.;
	
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	StrikeAgainDefault = j;
	}
else {
	MIDIfileType = 1;
	CsoundFileFormat = UNIX;
	StrikeAgainDefault = TRUE;
	// C4key = 48;	/* Here we compensate bad convention on old projects */
	// A4freq = 220.;	/* ditto */
	C4key = 60;
	A4freq = 440.0;
	}
if(iv > 15) {
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	DeftVolume = j;
//	BPPrintMessage(odInfo,"Default volume = %d\n",j);
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	VolumeController = j;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	DeftVelocity = j;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	DeftPanoramic = j;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	PanoramicController = j;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	SamplingRate = j;
//	BPPrintMessage(odInfo,"Sampling rate = %d\n",j);
	}
else {
	DeftVolume = DEFTVOLUME;
	VolumeController = VOLUMECONTROL;
	DeftVelocity = DEFTVELOCITY;
	DeftPanoramic = DEFTPANORAMIC;
	PanoramicController = PANORAMICCONTROL;
	SamplingRate = SAMPLINGRATE;
	}
#if BP_CARBON_GUI_FORGET_THIS
SetFileSavePreferences();
SetDefaultPerformanceValues();
SetTuning();
SetDefaultStrikeMode();
#endif /* BP_CARBON_GUI_FORGET_THIS */

// This block reads in font sizes for Carbon GUI text windows
if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
wmax = j;
// BPPrintMessage(odInfo,"Number of windows = %d\n",j);
if(wmax > 0) {
	for(w=0; w < (wmax - 1); w++) {
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		}
	}
if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
if(j <= 10 || j > 127) BlockScaleOnKey = 60;
else BlockScaleOnKey = j;

ResetMIDIFilter();

if(iv > 4) {
	if(ReadLong(sefile,&k,&pos) == MISSED) goto ERR;
	MIDIoutputFilter = k;
	if(startup) MIDIoutputFilterstartup = MIDIoutputFilter;
	GetOutputFilterWord();
	for(i=0; i < 12; i++) {
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		NameChoice[i] = j;
	//	BPPrintMessage(odInfo,"NameChoice[%d] = %d\n",i,j);
		}
	}
if(ReadLong(sefile,&k,&pos) == MISSED) goto ERR;
if(k != 0L) {
	MIDIinputFilter = k;
//	BPPrintMessage(odInfo,"MIDIinputFilter = %d\n",k);
	if(startup) MIDIinputFilterstartup = MIDIinputFilter;
	GetInputFilterWord();
	if(!ScriptExecOn && !NoteOnIn) {
		Alert1("Reception of NoteOn's is disabled. Most MIDI data received by BP2 will be meaningless");
		}
	}
#if BP_CARBON_GUI_FORGET_THIS
SetFilterDialog();
#endif /* BP_CARBON_GUI_FORGET_THIS */

if(iv > 19) {
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	ShowObjectGraph = j;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	ShowPianoRoll = j;
	/**** THIS IS WHERE THE SETTINGS FILE ENDS NOW IN BP3 ****/
	/* Removed code for reading piano roll colors */
	}
else {
	ShowObjectGraph = TRUE;
	ShowPianoRoll = FALSE;
	}

if(ShowObjectGraph || ShowPianoRoll) ShowGraphic = TRUE;
if(!ShowGraphic) ShowObjectGraph = ShowPianoRoll = FALSE;
if(ShowPianoRoll) BPPrintMessage(odInfo,"Pianoroll graphics will be displayed\n");
if(ShowObjectGraph) BPPrintMessage(odInfo,"Object graphics will be displayed\n");
BPPrintMessage(odInfo,"Metronome will be %.3f beats/mn by default (as per settings)\n",(Qclock * 60.)/Pclock); 
if(Nature_of_time == STRIATED) BPPrintMessage(odInfo,"Time is STRIATED\n");
else BPPrintMessage(odInfo,"Time is SMOOTH (no metronome)\n");

if(PlaySelectionOn) Improvize = 0;			
/* Removed code for reading "NewEnvironment", window coordinates & text colors */

goto QUIT;

ERR:
result = MISSED;
sprintf(Message,"=> Error reading '%s' settings file...",filename);
Alert1(Message);

QUIT:
MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
CloseFile(sefile);

LoadOn--;

if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed end LoadSettings = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);
return(result);
}

#if BP_CARBON_GUI_FORGET_THIS

SaveDecisions(void)
{
int i,ishtml;
NSWReply reply;
short refnum;
long count;
OSErr err;

err = NSWInitReply(&reply);
ShowMessage(TRUE,wMessage,"Creating decision file...");
PascalLine[0] = 0;
if(NewFile(-1,4,PascalLine,&reply)) {
	i = CreateFile(-1,-1,4,PascalLine,&reply,&refnum);
	if(i == ABORT) {
		err = NSWCleanupReply(&reply);
		return(MISSED);
		}
	if(i == OK) {
		SetCursor(&WatchCursor);
		if(!ComputeOn) WriteToFile(NO,MAC,"END",refnum);
		WriteHeader(-1,refnum,reply.sfFile);
		sprintf(LineBuff,"%ld",(long)CompileDate);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)ProduceStackDepth);
		WriteToFile(NO,MAC,LineBuff,refnum);
		for(i=0; i < ProduceStackDepth; i++) {
			sprintf(LineBuff,"%ld\n%ld\n%ld",
				(long)(*p_MemGram)[i],(long)(*p_MemRul)[i],(long)(*p_MemPos)[i]);
			WriteToFile(NO,MAC,LineBuff,refnum);
			}
		ishtml = IsHTML[wGrammar];
		IsHTML[wGrammar] = FALSE;
		WriteHeader(wGrammar,refnum,reply.sfFile);
		WriteFile(TRUE,MAC,refnum,wGrammar,GetTextLength(wGrammar));
		IsHTML[wGrammar] = ishtml;
		WriteEnd(-1,refnum);
		GetFPos(refnum,&count);
		SetEOF(refnum,count);
		FlushFile(refnum);
		FSClose(refnum);
		reply.saveCompleted = true;
		err = NSWCleanupReply(&reply);
		return(OK);
		}
	else {
		MyPtoCstr(MAXNAME,PascalLine,LineBuff);
		sprintf(Message,"=> Error creating '%s'",LineBuff);
		Alert1(Message);
		}
	}
err = NSWCleanupReply(&reply);
return(MISSED);
}


LoadDecisions(int loadgrammar)
{
int i,iv,j,maxderiv,r;
FSSpec spec;
short refnum;
long pos,k;
long compiledate;
char *p,*q;
OSErr io;
char **p_line,**p_completeline;

ShowMessage(TRUE,wMessage,"Locate decision file...");
p_line = p_completeline = NULL;
if(OldFile(-1,4,PascalLine,&spec)) {
	p2cstrcpy(LineBuff,PascalLine);
	if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
		pos = ZERO;
		CompleteDecisions = FALSE;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
		if(Mystrcmp(p_line,"END") == 0) {
			CompleteDecisions = TRUE;
			if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
			}
		if(CheckVersion(&iv,p_line,LineBuff) != OK) goto ERR;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
		if(ReadLong(refnum,&compiledate,&pos) == MISSED)
			goto ERR;
		if(!loadgrammar && (compiledate != CompileDate)) {
			if((r = Answer(
	"Grammar may have changed.\nLoad old version from decision file",'Y'))
														== OK) {
				if(ResetProject(FALSE) != OK) goto NOERR;
				loadgrammar = YES;
				}
			if(r == NO) Alert1
		("Non-matching grammars may yield unpredictable results. (Including bomb)");
			if(r == ABORT) goto NOERR;
			}
		if(ReadLong(refnum,&k,&pos) == MISSED) goto ERR;
		ProduceStackDepth = k;
		if(ProduceStackDepth > MAXDERIV) {
			maxderiv = ProduceStackDepth;
			ReleaseProduceStackSpace();
			}
		else maxderiv = MAXDERIV;
		if(MakeComputeSpace(maxderiv) != OK) goto ERR;
		for(i=0; i < ProduceStackDepth; i++) {
			PleaseWait();
			if(ReadInteger(refnum,&j,&pos) == MISSED) goto ERR;
			(*p_MemGram)[i] = j;
			if(ReadInteger(refnum,&j,&pos) == MISSED) goto ERR;
			(*p_MemRul)[i] = j;
			if(ReadLong(refnum,&k,&pos) == MISSED) goto ERR;
			(*p_MemPos)[i] = k;
			}
		if(loadgrammar) {
			if((r=LoadGrammar(&spec,refnum)) != OK) goto ERR;
			p2cstrcpy(FileName[wGrammar],spec.name);
			SetName(wGrammar,TRUE,TRUE);
			if((r=LoadAlphabet(wGrammar,&spec)) != OK) goto ERR;
			if(CompileGrammar(TRUE) != OK) goto ERR;
			CompileDate = compiledate; /* compile time is the old one */
			}
		goto NOERR;
ERR:
		Alert1("Can't read decision file...");
NOERR:
		if(FSClose(refnum) == noErr) ;
		}
	else {
		TellError(34,io);
		}
	MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
	}
else {
	/* Alert1("=> Error reading decision file..."); */  // user may have cancelled
	HideWindow(Window[wMessage]);
	return(MISSED);
	}
HideWindow(Window[wMessage]);
if(!IsEmpty(wInfo)) BringToFront(Window[wInfo]);
return(OK);
}


LoadInteraction(int anyfile, int manual)
{
int i,io,maxctrl,maxwait,rep,result,iv,s,type,oldversion,html;
FSSpec spec;
short refnum;
char line[MAXNAME+1],line2[MAXNAME+1],date[MAXNAME+1];
char **p_line,**p_completeline;
long pos;
unsigned long p,q,kk;
long k;
double r;

if(!Interactive) return(OK);
if(!ScriptExecOn) ShowWindow(Window[wInteraction]);
if(LoadedIn) {
	if(!CompiledIn) return(CompileInteraction());
	else return(OK);
	}
result = MISSED;
p_line = p_completeline = NULL;
strcpy(Message,FileName[wInteraction]);
strcpy(line,Message);
type = gFileType[wInteraction];
if(anyfile) type = ftiAny;
spec.vRefNum = TheVRefNum[wInteraction];
spec.parID = WindowParID[wInteraction];
c2pstrcpy(spec.name, Message);
SetSelect(ZERO,GetTextLength(wInteraction),TEH[wInteraction]);
TextDelete(wInteraction);
if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
	if(CheckFileName(wInteraction,line,&spec,&refnum,type,TRUE) != OK) {
		Interactive = FALSE;
		SetButtons(TRUE);
		UpdateDirty(TRUE,iSettings);
		return(MISSED);
		}
	}
sprintf(Message,"Loading %s...",FileName[wInteraction]);
ShowMessage(TRUE,wMessage,Message);
pos = 0L;
LoadOn++;

html = FALSE;
 
READMORE:
if(ReadOne(FALSE,html,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
if((*p_line)[0] == '<') {
	html = TRUE; goto READMORE;
	}
if(CheckVersion(&iv,p_completeline,FileName[wInteraction]) != OK) goto ERR;
ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos);
GetDateSaved(p_completeline,&(p_FileInfo[wInteraction]));
result = OK;
if(ReadInteger(refnum,&s,&pos) != MISSED) {
	oldversion = TRUE;
	DeriveFurtherKey = s;
	}
else {	/* This is a text version (new format) */
	oldversion = FALSE; pos = ZERO;
	SetFPos(refnum,fsFromStart,pos);
	if(ReadFile(wInteraction,refnum) == OK) {
		if(!WASTE) {
			CCUTEToScrap();	// WHY?
			}
		/* The following is only useful to erase date and version */
		GetHeader(wInteraction);
		goto QUIT;
		}
	else {
		result = MISSED; goto QUIT;
		}
	}

/* Continue to read the old format... */
MuteOnChan = MuteOffChan = -1;

if(ReadInteger(refnum,&s,&pos) != MISSED)
DeriveFurtherChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
ResetWeightKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
ResetWeightChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
PlayKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
PlayChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
RepeatKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
RepeatChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
EndRepeatKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
EndRepeatChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
EverKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
EverChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
QuitKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
QuitChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
UseEachSubKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
UseEachSubChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
SynchronizeStartKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
SynchronizeStartChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
SpeedCtrl = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
SpeedChan = s; else goto ERR;
if(ReadFloat(refnum,&r,&pos) != MISSED)
SpeedRange = r; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
SetTimeKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
SetTimeChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
StriatedKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
StriatedChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
NoConstraintKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
NoConstraintChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
SkipKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
SkipChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
AgainKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
AgainChan = s; else goto ERR;

if(iv > 3) {
	if(ReadInteger(refnum,&s,&pos) != MISSED)
	maxwait = s; else goto ERR;
	}
else maxwait = 8;
for(i=1; i <= maxwait; i++) {
	if((iv > 3) && (ReadInteger(refnum,&s,&pos) == MISSED))
			goto ERR;	/* type 0 = MIDi keyboard, 1 = ... */
	if(iv > 3 && s != 0) {
		sprintf(Message, "Unknown synchro tag type #%ld in %s",(long)s,
			FileName[wInteraction]);
		Alert1(Message);
		goto ERR;
		}
	if(ReadInteger(refnum,&s,&pos) == MISSED) goto ERR;
	WaitKey[i] = s;
	if(ReadInteger(refnum,&s,&pos) == MISSED) goto ERR;
	WaitChan[i] = s;
	}
if(ReadInteger(refnum,&s,&pos) == MISSED) goto ERR; /* Unused */
if(ReadInteger(refnum,&s,&pos) == MISSED) goto ERR; /* Unused */
if(ReadInteger(refnum,&s,&pos) != MISSED)
MinTclockKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED)
MaxTclockKey = s; else goto ERR;
if(MinTclockKey != -1 && MinTclockKey == MaxTclockKey) {
	Alert1("=> Lower and higher tempo-adjustment keys should be different");
	goto ERR;
	}
if(iv < 4) {
	if(ReadOne(FALSE,FALSE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) != MISSED) {
		MystrcpyHandleToString(MAXNAME,0,line,p_line);
		if(FloatToNiceRatio(line,&p,&q) != OK) goto ERR;
		if(p == ZERO) {
			MaxPclock= ZERO; MaxQclock = 1L;
			}
		else {
			if((Simplify((double)INT_MAX,(double)p,(double)60L*q,&MaxQclock,&MaxPclock) != OK)
				&& (Simplify((double)INT_MAX,floor((double)p/60L),(double)q,&MaxQclock,&MaxPclock) != OK))
					goto ERR;
			}
		}
	else goto ERR;
	if(ReadOne(FALSE,FALSE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) != MISSED) {
		MystrcpyHandleToString(MAXNAME,0,line,p_line);
		if(FloatToNiceRatio(line,&p,&q) != OK) goto ERR;
		if(p == ZERO) {
			MinPclock = ZERO; MinQclock = 1L;
			}
		else {
			if((Simplify((double)INT_MAX,(double)p,(double)60.*q,&MinQclock,&MinPclock) != OK)
				&& (Simplify((double)INT_MAX,floor((double)p/60.),q,&MinQclock,&MinPclock) != OK)) goto ERR;
			}
		}
	else goto ERR;
	}
else {
	if(ReadUnsignedLong(refnum,&kk,&pos) != MISSED) MinPclock = (double) kk;
	else goto ERR;
	if(MinPclock < 1.) {
		Alert1("=> Err: MinPclock < 1 in LoadInteraction().  Incorrect '-in' file");
		goto ERR;
		}
	if(ReadUnsignedLong(refnum,&kk,&pos) != MISSED) MaxPclock = kk;
	else goto ERR;
	if(MaxPclock < 1) {
		Alert1("=> Err: MaxPclock < 1 in LoadInteraction().  Incorrect '-in' file");
		goto ERR;
		}
	if(ReadUnsignedLong(refnum,&kk,&pos) != MISSED) MinQclock = kk;
	else goto ERR;
	if(ReadUnsignedLong(refnum,&kk,&pos) != MISSED) MaxQclock = kk;
	else goto ERR;
	}
if(MinPclock != ZERO && Simplify((double)INT_MAX,MinQclock,MinPclock,&MinQclock,&MinPclock) != OK) goto ERR;
if(MaxPclock != ZERO && Simplify((double)INT_MAX,MaxQclock,MaxPclock,&MaxQclock,&MaxPclock) != OK) goto ERR;
if(ReadInteger(refnum,&s,&pos) != MISSED) TclockChan = s;
else goto ERR;

if(iv > 3) {
	if(ReadInteger(refnum,&s,&pos) != MISSED)
	maxctrl = s; else goto ERR;
	}
else maxctrl = 16;

for(i=1; i <= maxctrl; i++) {
	if(ReadInteger(refnum,&s,&pos) != MISSED)
	ParamControl[i] = s; else goto ERR;
	}
for(i=maxctrl+1; i < MAXPARAMCTRL; i++) ParamControl[i] = -1;
for(i=0; i < MAXPARAMCTRL; i++) ParamKey[i] = -1;
ParamControlChan = 1;
if(ReadInteger(refnum,&s,&pos) != MISSED)
ParamControlChan = s; else goto ERR;
for(i=1; i < MAXPARAMCTRL; i++) {
	ParamChan[i] = ParamControlChan;	/* For compatibility with old files */
	}
goto QUIT;

ERR:
result = MISSED;
Alert1("=> Error reading interactive code file...");
ForgetFileName(wInteraction); /* 1/3/97 */
Interactive = FALSE; SetButtons(TRUE);

QUIT:
MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
if(FSClose(refnum) != noErr) {
	if(Beta) Alert1("=> Error closing interactive code file...");
	}
HideWindow(Window[wMessage]);
if(result == OK) {
	LoadedIn = TRUE;
	if(oldversion) {
		CompiledIn = TRUE; PrintInteraction(wInteraction);
		}
	else {
		if(CompileInteraction() != OK) {
			LoadedIn = FALSE; result = MISSED;
			}
		}
	SetName(wInteraction,TRUE,TRUE);
	Created[wInteraction] = TRUE;
	}
else	Created[wInteraction] = FALSE;
Dirty[wInteraction] = FALSE;
HideWindow(Window[wMessage]);
LoadOn--;
return(result);
}


SaveMIDIorchestra(Boolean doSaveAs)
{
int rep,i,j,type,result;
short refnum;
long count;
Str255 fn;
NSWReply reply;
OSErr err;

err = NSWInitReply(&reply);
result = MISSED;
if(Created[wMIDIorchestra] && !doSaveAs) {  // try the existing file first
	c2pstrcpy(fn, FileName[wMIDIorchestra]);
	err = FSMakeFSSpec(TheVRefNum[wMIDIorchestra], WindowParID[wMIDIorchestra], fn, &(reply.sfFile));
	if (err == noErr) {
		err = MyOpen(&(reply.sfFile), fsCurPerm, &refnum);
		if (err == noErr) goto WRITE;
		}
	}
// else do a "Save As"
ShowMessage(TRUE,wMessage,"Saving MIDI orchestra file...");
if(FileName[wMIDIorchestra][0] == '\0') GetDefaultFileName(wMIDIorchestra, Message);
else strcpy(Message,FileName[wMIDIorchestra]);
c2pstrcpy(fn, Message);
type = gFileType[wMIDIorchestra];

/* rep = Answer("Export as text file",'N');
if(rep == ABORT) goto OUT;
if(rep == OK) type = ftiText; */
reply.sfFile.vRefNum = TheVRefNum[wMIDIorchestra];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[wMIDIorchestra];
if(NewFile(wMIDIorchestra,type,fn,&reply)) {
	rep = CreateFile(wMIDIorchestra,wMIDIorchestra,type,fn,&reply,&refnum);
	if(rep == ABORT) goto OUT;
	if(rep == OK) {

WRITE:
		SetCursor(&WatchCursor);
		WriteHeader(wMIDIorchestra,refnum,reply.sfFile);
		sprintf(LineBuff,"%ld",(long)MAXCHAN);
		WriteToFile(NO,MAC,LineBuff,refnum);
		for(i=1; i <= MAXCHAN; i++) {
			for(j=0; j < 128; j++) {
				if((*p_GeneralMIDIpatchNdx)[j] == i) break;
				}
			sprintf(LineBuff,"%ld %s",(long)CurrentMIDIprogram[i],
				*((*p_GeneralMIDIpatch)[j]));
			WriteToFile(NO,MAC,LineBuff,refnum);
			}
		WriteEnd(wMIDIorchestra,refnum);
		GetFPos(refnum,&count);
		SetEOF(refnum,count);
		FlushFile(refnum);
		MyFSClose(wMIDIorchestra,refnum,&(reply.sfFile));
		reply.saveCompleted = true;
		p2cstrcpy(FileName[wMIDIorchestra],reply.sfFile.name);
		TheVRefNum[wMIDIorchestra] = reply.sfFile.vRefNum;
		WindowParID[wMIDIorchestra] = reply.sfFile.parID;
		SetName(wMIDIorchestra,TRUE,TRUE);
		Dirty[wMIDIorchestra] = FALSE;
		Created[wMIDIorchestra] = TRUE;
		result = OK;
		/* if(type == 1)
			Alert1("To open this file, click the 'Load' button with the 'option' key down"); */
		}
	else {
		MyPtoCstr(MAXNAME,fn,LineBuff);
		sprintf(Message,"=> Error creating '%s'",LineBuff);
		Alert1(Message);
		}
	}
OUT:
err = NSWCleanupReply(&reply);
ClearMessage();
return(result);
}


LoadMIDIorchestra(short refnum,int manual)
{
int vref,i,iv,j,r,w,rs,type;
FSSpec spec;
short itemtype;
long pos,imax;
OSErr io;
char **p_completeline,**p_line;
ControlHandle itemhandle;
Rect therect;
MIDI_Event e;


/* Removed this check since it is possible to load an orchestra without a driver;
   The code below checks for an active driver before sending program changes - 020807 akozar */
/* if(!IsMidiDriverOn()) {
	Alert1("Can't load MIDI orchestra because neither OMS nor the in-built MIDI driver are active");
	return(MISSED);
	} */
LoadOn++;
p_line = p_completeline = NULL;
SetField(MIDIprogramPtr,-1,fPatchName," ");
if(TestMIDIChannel > 0 && TestMIDIChannel <= MAXCHAN) {
	if(CurrentMIDIprogram[TestMIDIChannel] > 0) {
		GetDialogItem(MIDIprogramPtr, (short)CurrentMIDIprogram[TestMIDIChannel],
					&itemtype, (Handle*)&itemhandle, &therect);
		HiliteControl((ControlHandle)itemhandle,0);
		}
	GetDialogItem(SixteenPtr, (short)button1 + TestMIDIChannel - 1, &itemtype,
				(Handle*)&itemhandle, &therect);
	HiliteControl((ControlHandle) itemhandle,0);
	}
	
if(!manual) goto READIT;

ShowMessage(TRUE,wMessage,"Locate MIDI orchestra file...");
type = gFileType[wMIDIorchestra];
if(Option /* && Answer("Import any type of file",'Y') == OK */) type = ftiAny;
if(OldFile(-1,type,PascalLine,&spec)) {
	p2cstrcpy(FileName[wMIDIorchestra],PascalLine);
	if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
	
READIT:
		sprintf(Message,"Loading '%s' orchestra file...",FileName[wMIDIorchestra]);
		ShowMessage(TRUE,wMessage,Message);
		pos = ZERO;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
		if(CheckVersion(&iv,p_line,FileName[wMIDIorchestra]) != OK) goto NOERR;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
		SetCursor(&WatchCursor);	
		if(ReadLong(refnum,&imax,&pos) == MISSED) goto ERR;
		if(imax < 16 || imax > MAXCHAN) {
			if(Beta) Alert1("=> Err. LoadMIDIorchestra(). imax < 16 || imax > MAXCHAN");
			imax = MAXCHAN;
			}
		for(i=1; i <= 16; i++) {
			SetField(NULL,wMIDIorchestra,i,"[unknown instrument]");
			CurrentMIDIprogram[i] = 0;
			}
		for(i=1; i <= imax && i <= 16; i++) {
			if(ReadInteger(refnum,&w,&pos) == MISSED) goto ERR;
			CurrentMIDIprogram[i] = w;
			ChangedMIDIprogram = TRUE;
			if(w > 0 && w <= 128) {
				e.time = Tcurr;
				e.type = TWO_BYTE_EVENT;
				e.status = ProgramChange + i - 1;
				e.data2 = w - 1;
				rs = 0;
				if(IsMidiDriverOn() && !InitOn)
					SendToDriver(Tcurr * Time_res,0,&rs,&e);
				for(j=0; j < 128; j++) {
					if((*p_GeneralMIDIpatchNdx)[j] == w) {
						sprintf(Message,"[%ld] %s",(long)w,*((*p_GeneralMIDIpatch)[j]));
						SetField(NULL,wMIDIorchestra,i,Message);
						break;
						}
					}
				}
			}
		if(TestMIDIChannel > 0 && TestMIDIChannel <= MAXCHAN) {
			if(CurrentMIDIprogram[TestMIDIChannel] > 0) {
				GetDialogItem(MIDIprogramPtr, (short)CurrentMIDIprogram[TestMIDIChannel],
							&itemtype, (Handle*)&itemhandle, &therect);
				HiliteControl((ControlHandle) itemhandle,kControlButtonPart);
				for(j=0; j < 128; j++) {
					if((*p_GeneralMIDIpatchNdx)[j] == CurrentMIDIprogram[TestMIDIChannel]) {
						sprintf(Message,"[%ld] %s",(long)CurrentMIDIprogram[TestMIDIChannel],
							*((*p_GeneralMIDIpatch)[j]));
						SetField(MIDIprogramPtr,-1,fPatchName,Message);
						break;
						}
					}
				}
			GetDialogItem(SixteenPtr, (short)button1 + TestMIDIChannel - 1, &itemtype,
						(Handle*)&itemhandle, &therect);
			HiliteControl((ControlHandle) itemhandle,kControlButtonPart);
			}
		goto NOERR;
ERR:
		Alert1("Can't read MIDI orchestra file...");
NOERR:
		if(FSClose(refnum) == noErr) ;
		if(manual) {
			TheVRefNum[wMIDIorchestra] = spec.vRefNum;
			WindowParID[wMIDIorchestra] = spec.parID;
			}
		Dirty[wMIDIorchestra] = FALSE;
		Created[wMIDIorchestra] = TRUE;
		NewOrchestra = TRUE;
		}
	else TellError(35,io);
	MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
	}
else {
	HideWindow(Window[wMessage]);
	LoadOn--;
	return(MISSED);
	}
LoadOn--;
HideWindow(Window[wMessage]);
return(OK);
}


static OSErr GetMidiDriverStartupSpec(FSSpecPtr spec, Boolean saving)
{
	OSErr err;
	Boolean haveStartupSpec = FALSE;
	
	// on OS X, check the user's preferences folder first
	if (RunningOnOSX) {
		if ((err = FindFileInPrefsFolder(spec, kBPMdStartup)) == noErr)
			haveStartupSpec = TRUE;
		else if (saving)  return err;  // stop here if getting spec to save
	}
	if (!haveStartupSpec) {
		// look in the BP2 application folder on OS 9 or if not found yet
		err = FSMakeFSSpec(RefNumbp2, ParIDbp2, kBPMdStartup, spec);
	}

	return err;
}

static void RememberMdFile(FSSpecPtr spec)
{
	p2cstrcpy(FileName[iMidiDriver], spec->name);
	TellOthersMyName(iMidiDriver);
	TheVRefNum[iMidiDriver] = spec->vRefNum;
	WindowParID[iMidiDriver] = spec->parID;
	return;
}

int LoadMidiDriverStartup()
{
	OSErr	err;
	int	result;
	short	refnum;
	FSSpec mdStartup;
	
	err = GetMidiDriverStartupSpec(&mdStartup, FALSE);
	if (err != noErr)  return (MISSED);	// the file is optional, so don't report to user
	
	err = MyOpen(&mdStartup, fsRdPerm, &refnum);
	if (err != noErr)  {	// do report the failure to open an existing file
		ShowMessage(TRUE, wMessage, "=> Error opening '-md.startup' file.");
		return (MISSED);
	}
	
	result = ReadMidiDriverSettings(refnum, &mdStartup);
	if (result != OK) ShowMessage(TRUE, wMessage, "=> Error reading '-md.startup' file.");
	return result; 
}

/* Retrieves the name (and vRefNum/parID) of an -md file from window w
   and then tries to load a file with those specs. */
int LoadLinkedMidiDriverSettings(int w)
{
	OSErr  err;
	char   name[MAXNAME];
	FSSpec mdfile;
	
	if (GetLinkedFileName(w, iMidiDriver, name) == OK) {
		err = FSMakeFSSpec(TheVRefNum[w], WindowParID[w], in_place_c2pstr(name), &mdfile);
		if (err != noErr) return (MISSED);
		return LoadMidiDriverSettings(&mdfile);
	}
	
	return (MISSED);
}

/* OpenMidiDriverSettings() displays a standard Open file dialog to get
   a FSSpec for an -md file before calling Load... */ 
int OpenMidiDriverSettings()
{
	int		result;
	FSSpec	mdfile;
	
	sprintf(Message, "Select a %s file...", DocumentTypeName[iMidiDriver]);
	ShowMessage(TRUE,wMessage,Message);
	result = OldFile(wUnknown, ftiMidiDriver, PascalLine, &mdfile);
	HideWindow(Window[wMessage]);
	if (result == OK) result = LoadMidiDriverSettings(&mdfile);
	
	return result;
}

/* LoadMidiDriverSettings() opens and loads the Midi Driver settings file
   specified by mdfile. */ 
int LoadMidiDriverSettings(FSSpec* mdfile)
{
	OSErr		err;
	int		result;
	short		refnum;
	char		cname[64];	// size of FSSpec.name
	
	err = MyOpen(mdfile, fsRdPerm, &refnum);
	if (err != noErr)  {
		p2cstrcpy(cname, mdfile->name);
		sprintf(Message,"=> Error opening the file '%s'.", cname);
		Alert1(Message);
		return (MISSED);
	}
	
	result = ReadMidiDriverSettings(refnum, mdfile);
	if (result == OK) RememberMdFile(mdfile);
	else {
		p2cstrcpy(cname, mdfile->name);
		sprintf(Message,"=> Error reading the file '%s'.", cname);
		Alert1(Message);
		return (MISSED);
	}
	
	return result; 
}

extern int ReadCoreMIDISettings(short refnum, long* pos);

/* Read the current Midi driver's settings from an already open file */
int ReadMidiDriverSettings(short refnum, FSSpec* spec)
{
	int  iv, result;
	long pos;
	char **p_line,**p_completeline;
	// char cname[64]; // size of FSSpec.name
	
	p_line = p_completeline = NULL;
	pos = ZERO;
	PleaseWait();
	LoadOn++;
	
	result = MISSED;
	// read BP version comment
	if (ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	// for now, CoreMIDI driver checks its own version number
	// p2cstrcpy(cname, spec->name);
	// if (CheckVersion(&iv, p_line, cname) != OK) goto ERR;
	// read "File saved as ..." & date comment
	if (ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;

	// read the name of the driver
	if (ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	// call the driver's own read settings function
#if BP_MACHO
	if (p_completeline != NULL && strcmp(*p_completeline, "CoreMIDI") == 0) {
		result = ReadCoreMIDISettings(refnum, &pos);
	}
#endif
	
ERR:
	MyDisposeHandle((Handle*)&p_line);
	MyDisposeHandle((Handle*)&p_completeline);
	if (FSClose(refnum) != noErr && Beta) {
		Alert1("=> Error closing Midi driver settings file.");
	}
	LoadOn--;
	StopWait();
	
	return result;
}

int SaveMidiDriverStartup()
{
	OSErr	err;
	int	result;
	short	refnum;
	OSType thecreator, thetype;
	FSSpec mdStartup;
	
	if (Answer("Save the current Midi driver settings as your startup settings", 'Y')
		!= YES)  return (MISSED);
	
	err = GetMidiDriverStartupSpec(&mdStartup, TRUE);
	if (err == fnfErr) {
		SelectCreatorAndFileType(ftiMidiDriver, &thecreator, &thetype);
		err = FSpCreate(&mdStartup, thecreator, thetype, smSystemScript);
	}
	if (err != noErr)  return (MISSED);
	
	err = MyOpen(&mdStartup, fsRdWrPerm, &refnum);
	if (err != noErr)  return (MISSED);

	result = WriteMidiDriverSettings(refnum, &mdStartup);
	if (result == OK) {
		ShowMessage(TRUE, wMessage, 
			(RunningOnOSX ? "Saved '-md.startup' to your Library/Preferences/Bol Processor/ folder."
			              : "Saved '-md.startup' to the BP2 application folder."));
	}
	return result; 
}

/* Display a Save file dialog and save the current Midi driver's settings */
int SaveMidiDriverSettings()
{
	OSErr		err;
	short		refnum;
	int		io;
	NSWReply	reply;
	char		defaultname[64];
	Str255	fn;
	
	err = NSWInitReply(&reply);
	
	// suggest last saved filename if we have one
	if (FileName[iMidiDriver][0] != '\0')  c2pstrcpy(fn, FileName[iMidiDriver]);
	else if (GetDefaultFileName(iMidiDriver, defaultname) == OK) {
		c2pstrcpy(fn, defaultname);
	}
	else return(MISSED);
	
	if (io = NewFile(iMidiDriver,gFileType[iMidiDriver],fn,&reply)) {
		io = CreateFile(iMidiDriver,iMidiDriver,gFileType[iMidiDriver],fn,&reply,&refnum);
		if (io == OK) {
			io = WriteMidiDriverSettings(refnum, &(reply.sfFile));
			if (io == OK) {
				reply.saveCompleted = true;
				RememberMdFile(&(reply.sfFile));
				sprintf(Message,"Successfully saved '%s'", FileName[iMidiDriver]);
				ShowMessage(TRUE,wMessage,Message);
			}
			else {
				p2cstrcpy(defaultname, fn);
				sprintf(Message, "=> Error writing file '%s'", defaultname);
				Alert1(Message);
				HideWindow(Window[wMessage]);
			}
		}
		else {
			MyPtoCstr(MAXNAME,fn,defaultname);
			sprintf(Message, "=> Error creating file '%s'", defaultname);
			Alert1(Message);
		}
	}		
	// no error message if NewFile() fails (probably user cancelled)
	
	err = NSWCleanupReply(&reply);
	return io;
}

extern int WriteCoreMIDISettings(short refnum);

/* Write the current Midi driver's settings to an already open file */
int WriteMidiDriverSettings(short refnum, FSSpec* spec)
{
	char	fname[64];
	long	flength;
	
	SaveOn++;
	p2cstrcpy(fname, spec->name);
	sprintf(Message,"Saving '%s'...", fname);
	ShowMessage(TRUE,wMessage,Message);
	PleaseWait();
	
	// write the file, starting with the standard BP2 header
	WriteHeader(wUnknown, refnum, *spec); // use wUnknown instead of iMidiDriver until we fix WindowName[] issues
	// write the name of the driver
	WriteToFile(NO,MAC,"CoreMIDI",refnum);
	// call the driver's own save settings function
#if BP_MACHO
	WriteCoreMIDISettings(refnum);
#endif
	// this doesn't actually do anything ...
	WriteEnd(wUnknown, refnum);
	
	GetFPos(refnum,&flength);
	SetEOF(refnum,flength);
	FlushFile(refnum);
	MyFSClose(iMidiDriver, refnum, spec);
	SaveOn--;
	
	return OK;
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

int LoadObjectPrototypes(int checkversion,int tryname)
{
char c,date[80],*newp,*name_of_file = NULL, *final_name = NULL;
MIDIcode **p_b;
char **p_line,**p_completeline,line[MAXLIN],line2[MAXLIN];
int i,iv,j,jj,co,rep,okt1,diff,stop,maxsounds,s,objecttype,oldjbol,notsaid,
	pivbeg,pivend,pivbegon,pivendoff,pivcent,pivcentonoff,pivspec,newbols,okrescale,
	compilemem,newinstruments,type,dirtymem,longerCsound,result;
long t,t1,t2,tm,d,kres;
long pos,imax;
long k,kk;
double r;
short refnum,refnum2;
FSSpec spec;
Str63 filename;
Handle h,ptr;
FILE* mifile;

CompileAlphabet();

if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed start LoadObjectPrototypes = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);

rep = MISSED;
newinstruments = CompiledCsObjects = FALSE;
pos = 0L;
line2[0] = '\0';
// if(!tryname) FileName[iObjects][0] = '\0';
p_line = p_completeline = NULL;

LoadOn++;

mifile = fopen(FileName[iObjects], "r");
if (mifile == NULL) {
	BPPrintMessage(odError, "=> Could not find and open prototypes file %s\n",FileName[iObjects]);
//	return MISSED;
	return ABORT; // Fixed by BB 2022-02-18
	}

if(ReadOne(FALSE,FALSE,FALSE,mifile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
sprintf(Message,"Loading %s...",FileName[iObjects]);
ShowMessage(TRUE,wMessage,Message);
if(trace_load_prototypes) BPPrintMessage(odInfo, "Line = %s\n",*p_line);
if(CheckVersion(&iv,p_line,FileName[iObjects]) != OK) goto ERR;
if(trace_load_prototypes) BPPrintMessage(odInfo, "Version = %d\n",iv);

if(iv > 2) {
	ReadOne(FALSE,TRUE,FALSE,mifile,TRUE,&p_line,&p_completeline,&pos);
//	GetDateSaved(p_completeline,&(p_FileInfo[iObjects]));
	}
newbols = FALSE;

if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
PrototypeTickKey = s;
if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
PrototypeTickChannel = s;
if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
PrototypeTickVelocity = s;

fscanf(mifile, "%[^\n]",line2); // Necessary to read a line that might be empty

if(trace_load_prototypes) BPPrintMessage(odInfo, "Line = %s\n",line2);
if(strlen(line2) > 5) { // Fixed by BB 2022-02-18
	BPPrintMessage(odInfo,"Trying to load Csound resource %s\n",line2);
	// Note that line2 contains the path (csound_resources)
	sprintf(Message,"../%s",line2);
	strcpy(FileName[wCsoundResources],Message);
	if((result = LoadCsoundInstruments(0,1)) != OK) return(result);
	pos += strlen(line2);
	}
		
MAXSOUNDS:
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	maxsounds = s;
	if(trace_load_prototypes) BPPrintMessage(odInfo, "maxsounds = %d Jbol = %d\n",s,Jbol);
//	if(CheckTerminalSpace() != OK) goto ERR;
	oldjbol = Jbol;
	Jbol += maxsounds;
	if(ResizeObjectSpace(YES,Jbol + Jpatt,0) != OK) goto ERR;
//	if(CheckTerminalSpace() != OK) goto ERR;
	Jbol = oldjbol; newbols = TRUE;

NumberTables = 0;
rep = notsaid = OK;

// Be careful: the following loop will read data forever if the file
// was not properly closed...

NEXTBOL:
PleaseWait(); j = -1;
if(ReadOne(FALSE,TRUE,TRUE,mifile,TRUE,&p_line,&p_completeline,&pos) == MISSED) {
	sprintf(Message,"Unexpected end of '%s' file...  May be old version?",
		FileName[iObjects]);
	BPPrintMessage(odInfo,Message);
	goto ERR;
	}
if(trace_load_prototypes) BPPrintMessage(odInfo, "line = %s\n",*p_line);

if(p_completeline == NULL) {
	if(Beta) Alert1("=> Err. LoadObjectPrototypes(). p_completeline == NULL");
	goto OUT;
	}
if(MyHandleLen(p_completeline) < 1) goto OUT;
if(Mystrcmp(p_completeline,"DATA:") == 0) {
	if(ReadOne(FALSE,TRUE,TRUE,mifile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	if((*p_completeline)[0] == '\0')
		MystrcpyStringToHandle(&p_completeline,"[Comment on prototype file]");
	MystrcpyHandleToString(MAXFIELDCONTENT,0,line,p_completeline);
	if(trace_load_prototypes) BPPrintMessage(odInfo, "General comment = %s\n",line);
	//	rep = SetField(NULL,wPrototype1,fPrototypeFileComment,line);
//	else SetField(NULL,wPrototype1,fPrototypeFileComment,"[Comment on prototype file]");
	goto OUT;
	}
if(Mystrcmp(p_completeline,"_endSoundObjectFile_") == 0) goto OUT;
if((iv > 3) && (Mystrcmp(p_completeline,"TABLES:") == 0)) {
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto OUT;
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
	if(Jbol == 0) {
		Jbol = 2;
		GetAlphabetSpace();
		}
//	p_Bol = (char****) GiveSpace((Size)(Jbol) * sizeof(char**));
	if((jj=CreateBol(0,0,p_completeline,0,0,BOL)) < 0) goto ERR;
	if(jj >= Jbol) {
		if(Beta) Alert1("=> Err. LoadObjectPrototypes(). jj >= Jbol");
		goto ERR;
		}
	j = jj;
	if(trace_load_prototypes) BPPrintMessage(odInfo, "Creating sound-object for j = %d named %s\n",j,*p_completeline);
	if(Jbol > oldjbol) {
		compilemem = CompiledGr;
		if(notsaid) {
			ShowMessage(TRUE,wMessage,"New terminal symbols have been appended to alphabet.");
	/*		SetSelect(GetTextLength(wAlphabet),GetTextLength(wAlphabet),
				TEH[wAlphabet]);
			PrintBehind(wAlphabet,"\n"); */
			}
		notsaid = FALSE;
		MystrcpyHandleToString(MAXLIN,0,Message,p_completeline);
		strcat(Message," ");
	//	PrintBehind(wAlphabet,Message);
		CompiledAl = TRUE; /* Dirty[wAlphabet] = FALSE; */
		CompiledGr = compilemem;
		}
	}

diff = FALSE; // Old variable for backward  compatibility. It should be suppressed.

if(trace_load_prototypes) BPPrintMessage(odInfo, "Final Jbol = %d\n",Jbol);
// if(iv > 3) { 
	if(ReadInteger(mifile,&objecttype,&pos) == MISSED) goto ERR;
	if(trace_load_prototypes) BPPrintMessage(odInfo, "object type = %d\n",objecttype);
	(*p_Type)[j] = objecttype;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	(*p_Resolution)[j] = s;
	if(trace_load_prototypes) BPPrintMessage(odInfo, "(*p_Resolution)[%d] = %d\n",j,(*p_Resolution)[j]);
//	}
/* else if(!diff) {
	(*p_Type)[j] = 1;
	(*p_Resolution)[j] = 1; 
	} */
if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
(*p_DefaultChannel)[j] = s;
if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
(*p_Tref)[j] = ((long) k * (*p_Resolution)[j]);
if(ReadFloat(mifile,&r,&pos) == MISSED) goto ERR;
(*p_Quan)[j] = r;
if(ReadOne(FALSE,FALSE,TRUE,mifile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
if(trace_load_prototypes) BPPrintMessage(odInfo,"line3 = %s\n",*p_line);

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

	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR; /* rescalemode */
	if(!diff) (*p_RescaleMode)[j] = s;
	if(ReadFloat(mifile,&r,&pos) == MISSED) goto ERR;
	if(!diff) (*p_AlphaMin)[j] = r;
	if(ReadFloat(mifile,&r,&pos) == MISSED) goto ERR;
	if(!diff) (*p_AlphaMax)[j] = r;
	if(!diff && iv < 5) {	/* Fixing a bug in default values */
		(*p_AlphaMax)[j] = 10L;
		}
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR; /* delaymode */
	if(!diff) (*p_DelayMode)[j] = s;
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	if(!diff) (*p_MaxDelay)[j] = k;
	if(iv > 4) {
		if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR; /* forwardmode */
		if(!diff) (*p_ForwardMode)[j] = s;
		}
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	if(!diff) (*p_MaxForward)[j] = k;
	if(!diff && iv < 5) {	/* Fixing a bug in default values */
		(*p_MaxDelay)[j] = (*p_MaxForward)[j] = ZERO;
		}
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR; /* brktempomode */
	if(!diff) (*p_BreakTempoMode)[j] = s;
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
/*	if(!diff) (*p_MaxBreakTempo)[j] = k;
	if(!diff && iv < 5) {
		(*p_MaxBreakTempo)[j] = ZERO;
		} */
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR; /* contbegmode */
	if(!diff) (*p_ContBegMode)[j] = s;
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	if(!diff) (*p_MaxBegGap)[j] = k;
	if(!diff && iv < 5) {	/* Fixing a bug in default values */
		(*p_MaxBegGap)[j] = ZERO;
		}
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR; /* contendmode */
	if(!diff) (*p_ContEndMode)[j] = s;
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	if(!diff) (*p_MaxEndGap)[j] = k;
	if(!diff && iv < 5) {	/* Fixing a bug in default values */
		(*p_MaxEndGap)[j] = ZERO;
		}
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR; /* coverbegmode */
	if(!diff) (*p_CoverBegMode)[j] = s;
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	if(!diff) (*p_MaxCoverBeg)[j] = k;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR; /* coverendmode */
	if(!diff) (*p_CoverEndMode)[j] = s;
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	if(!diff) (*p_MaxCoverEnd)[j] = k;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR; /* truncbegmode */
	if(!diff) (*p_TruncBegMode)[j] = s;
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	if(!diff) (*p_MaxTruncBeg)[j] = k;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR; /* truncendmode */
	if(!diff) (*p_TruncEndMode)[j] = s;
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	if(!diff) (*p_MaxTruncEnd)[j] = k;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_PivMode)[j] = s;
	if(ReadFloat(mifile,&r,&pos) == MISSED) goto ERR;
		(*p_PivPos)[j] = r;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_AlphaCtrlNr)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_AlphaCtrlChan)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_OkTransp)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_OkArticul)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_OkVolume)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_OkPan)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_OkMap)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
		(*p_OkVelocity)[j] = s;
	if(ReadFloat(mifile,&r,&pos) == MISSED) goto ERR;
	if(!diff) (*p_PreRoll)[j] = r;
	if(ReadFloat(mifile,&r,&pos) == MISSED) goto ERR;
	if(!diff) (*p_PostRoll)[j] = r;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_PreRollMode)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_PostRollMode)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_PeriodMode)[j] = s;
	if(ReadFloat(mifile,&r,&pos) == MISSED) goto ERR;
	if(!diff) (*p_BeforePeriod)[j] = r;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_ForceIntegerPeriod)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_DiscardNoteOffs)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_StrikeAgain)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
	if(!diff) (*p_CsoundAssignedInstr)[j] = s;
	if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
		(*p_CsoundInstr)[j] = s;
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	if(!diff) (*p_Tpict)[j] = k;
if(iv > 21) { // These are no longer used
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	if(!diff) (*p_ObjectColor)[j].red = k;
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	if(!diff) (*p_ObjectColor)[j].green = k;
	if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	if(!diff) (*p_ObjectColor)[j].blue = k;
	if(trace_load_prototypes) BPPrintMessage(odInfo, "(*p_ObjectColor)[j].blue = %ld\n",k);
	}
else {
	if(!diff) {
		(*p_ObjectColor)[j].red = (*p_ObjectColor)[j].green = (*p_ObjectColor)[j].blue = -1L;
		}
	}

(*pp_CsoundTime)[j] = NULL;

	/* Read pp_CsoundScoreText */
if(ReadOne(FALSE,FALSE,TRUE,mifile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
StripHandle(p_line);
if(trace_load_prototypes) BPPrintMessage(odInfo, "line2 = %s\n",*p_line);

if(Mystrcmp(p_line,"_beginCsoundScore_") != 0) goto ERR;

if(ReadOne(FALSE,FALSE,TRUE,mifile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;

if(trace_load_prototypes) BPPrintMessage(odInfo, "line3 = %s\n",*p_line);

if((ptr = (Handle) GiveSpace(MyGetHandleSize((Handle)p_completeline))) == NULL) goto ERR;
    (*pp_CsoundScoreText)[j] = (char**) ptr;
MystrcpyHandleToHandle(0,&((*pp_CsoundScoreText)[j]),p_completeline);

if((rep=CompileObjectScore(j,&longerCsound)) != OK) {
	OutCsound = FALSE;
	goto ERR;
	}
Dirty[iObjects] = Dirty[wPrototype7] = FALSE;
if(trace_load_prototypes) BPPrintMessage(odInfo,"Compiled Csound score\n");

if(ReadOne(FALSE,TRUE,TRUE,mifile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
if(trace_load_prototypes) BPPrintMessage(odInfo, "line4 = %s\n",*p_line);
if(Mystrcmp(p_completeline,"_endCsoundScore_") == 0) goto READSIZE;
else goto ERR;

READSIZE:
if(ReadInteger(mifile,&s,&pos) == MISSED) goto ERR;
if(trace_load_prototypes) BPPrintMessage(odInfo, "Size of MIDI code = %d\n",s);
imax = s;
	(*p_PasteDone)[j] = FALSE;
	(*p_MIDIsize)[j] = (*p_Ifrom)[j] = ZERO;
	(*pp_MIDIcode)[j] = NULL;
if(imax > 0) {
	if((p_b = (MIDIcode**) GiveSpace((Size)sizeof(MIDIcode)*(imax+1))) == NULL) goto ERR;
	rep = OK;
	}
if((*p_CsoundSize)[j] == 0) (*p_Dur)[j] = t1 = t2 = ZERO;
if(imax > 0) {
	for(i=0,tm=ZERO,okt1=FALSE; i < imax; i++) {
		if(ReadLong(mifile,&k,&pos) == MISSED) goto ERR;
	//	if(trace_load_prototypes) BPPrintMessage(odInfo, "k = %ld\n",k);
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
	imax = i;
	if(trace_load_prototypes) BPPrintMessage(odInfo, "imax = %d\n",imax);

	if(MIDItoPrototype(FALSE,TRUE,j,p_b,imax) != OK) goto ERR;
	if(MyDisposeHandle((Handle*)&p_b) != OK) goto ERR;
	}

if(!diff && CheckConsistency(j,TRUE) != OK) goto ERR;
if(trace_load_prototypes) BPPrintMessage(odInfo, "CheckConsistency is OK for j = %d\n",j);
if(iv > 9) {
	if(ReadOne(FALSE,TRUE,TRUE,mifile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	if(p_completeline == NULL) {
		if(Beta) Alert1("=> Err. LoadObjectPrototypes(). p_completeline == NULL");
		goto ERR;
		}
	if(!diff) {
		s = MyHandleLen(p_completeline);
		if(s > MAXFIELDCONTENT) {
			s = MAXFIELDCONTENT;
			(*p_completeline)[s] = '\0';
			}
		if((h = (Handle) GiveSpace((Size)(1+s) * sizeof(char))) == NULL) goto ERR;
		(*pp_Comment)[j] = (char**) h;
		MystrcpyHandleToHandle(0,&((*pp_Comment)[j]),p_completeline);
		if(trace_load_prototypes) BPPrintMessage(odInfo, "comment for %d = %s\n\n",j,*p_completeline);
		}
	}
goto NEXTBOL;

ERR:
if(!diff) {
	if(j > 1 && j < Jbol) ResetPrototype(j);
	}
if(CheckEmergency() == OK) {
	rep = MISSED;
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
CloseFile(mifile);
// if(CheckEmergency() != OK) return(ABORT);
// HideWindow(Window[wMessage]);

ObjectMode = ObjectTry = TRUE;
if(check_memory_use) BPPrintMessage(odInfo,"MemoryUsed end LoadObjectPrototypes = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);

// SetButtons(TRUE);
if(rep == OK) {
//	SetName(iObjects,YES,TRUE);
	if(newbols) {
		ResizeObjectSpace(NO,Jbol + Jpatt,0);
		// BPPrintMessage(odInfo,"Resizing object space for Jbol = %d and Jpatt = %ld\n",Jbol,Jpatt);
		}
	}
PrototypesLoaded = TRUE;
return(rep);
}

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
  
    // Storing start time
    clock_t start_time = clock();
  
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds);
}