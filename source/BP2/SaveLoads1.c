/* SaveLoads1.c (BP2 version 2.9.4) */ 

#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"

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
if(CompileCheck() != OK) return(FAILED);
LoadOn++;
ShowMessage(TRUE,wMessage,"Locate ‘-wg.’ file…");
p_line = p_completeline = NULL;
if(OldFile(-1,12,PascalLine,&spec)) {
	MyPtoCstr(255,PascalLine,LineBuff);
	if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
		pos = ZERO;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
		if(CheckVersion(&iv,p_line,LineBuff) != OK) goto NOERR;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
		if(ReadLong(refnum,&compiledate,&pos) == FAILED) goto ERR;
		if(compiledate != CompileDate) {
			r = Answer(
	"Grammar may have changed. Non-matching grammars may yield unpredictable results. Load anyway",'N');
			if(r != OK) goto NOERR;
			}
		SetCursor(&WatchCursor);
		for(igram=1; igram <= Gram.number_gram; igram++) {
			for(irul=1; irul <= (*(Gram.p_subgram))[igram].number_rule; irul++) {
				if(ReadInteger(refnum,&w,&pos) == FAILED) goto ERR;
				(*((*(Gram.p_subgram))[igram].p_rule))[irul].weight = w;
				}
			}
		AdjustWeights();
		MyPtoCstr(MAXNAME,PascalLine,FileName[iWeights]);
		goto NOERR;
ERR:
		Alert1("Can't read weight file…");
NOERR:
		if(FSClose(refnum) == noErr) ;
		}
	else TellError(32,io);
	MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
	}
else {
	HideWindow(Window[wMessage]);
	LoadOn--;
	return(FAILED);
	}
LoadOn--;
if(Beta && LoadOn > 0) {
	Alert1("Err. LoadOn > 0 ");
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
StandardFileReply reply;

if(ComputeOn || SetTimeOn || PrintOn || SoundOn || SelectOn || CompileOn || GraphicOn
	|| PolyOn) return(RESUME);
if(CompileCheck() != OK) return(FAILED);
ShowMessage(TRUE,wMessage,"Creating weight file…");
if(FileName[iWeights][0] != '\0')
	strcpy(Message,FileName[iWeights]);
else
	strcpy(Message,FilePrefix[iWeights]);
pStrCopy((char*)c2pstr(Message),fn);
reply.sfFile.vRefNum = TheVRefNum[iSettings];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[iSettings];
if(NewFile(fn,&reply)) {
	i = CreateFile(-1,-1,12,fn,&reply,&refnum);
	if(i == ABORT) return(FAILED);
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
		MyPtoCstr(MAXNAME,fn,FileName[iWeights]);
		return(OK);
		}
	else {
		MyPtoCstr(MAXNAME,fn,LineBuff);
		sprintf(Message,"Error creating ‘%s’",LineBuff);
		Alert1(Message);
		}
	}
return(FAILED);
}


LoadKeyboard(short refnum)
{
int i,io,iv,imax,j,type,result;
char **ptr;
long pos;
char **p_line,**p_completeline;

pos = ZERO; Dirty[wKeyboard] = FALSE;
p_line = p_completeline = NULL;
LoadOn++;

if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
if(CheckVersion(&iv,p_line,FileName[wKeyboard]) != OK) goto ERR;
if(ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
GetDateSaved(p_completeline,&(p_FileInfo[wKeyboard]));
if(ReadInteger(refnum,&imax,&pos) == FAILED) goto ERR;
if(imax > 52) {
	Alert1("This version of BP2 can't read selected ‘-kb.’ file");
	goto ERR;
	}
for(i=0; i < imax; i++) {
	if(ReadOne(FALSE,TRUE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED
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
if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
KeyboardType = j;
result = OK;
goto QUIT;

ERR:
GetKeyboard();
result = FAILED;
sprintf(Message,"Error reading ‘%s’ keyboard file…",FileName[wKeyboard]);
Alert1(Message);
FileName[wKeyboard][0] = '\0';

QUIT:
MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
SetKeyboard();
if(Token) AppendScript(66); else AppendScript(67);
if(FSClose(refnum) != noErr) {
	sprintf(Message,"Error closing ‘%s’ keyboard file…",FileName[wKeyboard]);
	Alert1(Message);
	result = FAILED;
	}
if(result == OK) {
	SetName(wKeyboard,TRUE,TRUE);
	Dirty[wKeyboard] = Created[wKeyboard] = FALSE;
	}
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
StandardFileReply reply;

GetKeyboard();
if(FileName[wKeyboard][0] != '\0') strcpy(Message,FileName[wKeyboard]);
else strcpy(Message,FilePrefix[wKeyboard]);
pStrCopy((char*)c2pstr(Message),fn);
p_spec->vRefNum = TheVRefNum[wKeyboard];
p_spec->parID = WindowParID[wKeyboard];
pStrCopy((char*)fn,p_spec->name);
good = NO;
if(Created[wKeyboard]) good = (MyOpen(p_spec,fsCurPerm,&refnum) == noErr);
if(good) goto WRITE;
reply.sfFile.vRefNum = TheVRefNum[wKeyboard];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[wKeyboard];
if(NewFile(fn,&reply)) {
	i = CreateFile(wKeyboard,-1,gFileType[wKeyboard],fn,&reply,&refnum);
	(*p_spec) = reply.sfFile;
	if(i == ABORT) return(FAILED);
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
		WriteToFile(NO,MAC,"1\r1\r1\r1\r0\r0\r0\r0\r\0",refnum);
		WriteEnd(wKeyboard,refnum);
		GetFPos(refnum,&count);
		SetEOF(refnum,count);
		FlushFile(refnum);
		MyFSClose(wKeyboard,refnum,p_spec);
		MyPtoCstr(MAXNAME,p_spec->name,FileName[wKeyboard]);
		TheVRefNum[wKeyboard] = p_spec->vRefNum;
		WindowParID[wKeyboard] = p_spec->parID;
		SetName(wKeyboard,TRUE,TRUE);
		Created[wKeyboard] = TRUE;
		Dirty[wKeyboard] = FALSE;
		return(OK);
		}
	else {
		MyPtoCstr(MAXNAME,fn,LineBuff);
		sprintf(Message,"Error creating ‘%s’",LineBuff);
		Alert1(Message);
		}
	}
return(FAILED);
}


LoadTimeBase(short refnum)
{
int i,io,iv,imax,j,type,result,y,maxticks,maxbeats,arg;
char **ptr;
long pos,x;
char **p_line,**p_completeline;

WaitForLastTicks();
LoadOn++;
pos = ZERO; Dirty[wTimeBase] = FALSE;
p_line = p_completeline = NULL;
if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
if(CheckVersion(&iv,p_line,FileName[wTimeBase]) != OK) goto ERR;
if(ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
GetDateSaved(p_completeline,&(p_FileInfo[wTimeBase]));
if(ReadInteger(refnum,&y,&pos) == FAILED) goto ERR; maxticks = y;
if(ReadInteger(refnum,&y,&pos) == FAILED) goto ERR; maxbeats = y;
if(maxticks > MAXTICKS) {
	Alert1("This version of BP2 can't read selected ‘-kb.’ file");
	goto ERR;
	}
if(maxbeats > MAXBEATS) {
	Alert1("This version of BP2 can't read selected ‘-kb.’ file");
	goto ERR;
	}
TickThere = FALSE;
for(i=0; i < maxticks; i++) {
	if(ReadInteger(refnum,&y,&pos) == FAILED) goto ERR; /* type */
	if(ReadInteger(refnum,&y,&pos) == FAILED) goto ERR; arg = y; /* nr of arguments */
	if(ReadInteger(refnum,&y,&pos) == FAILED) goto ERR; TickKey[i] = y;
	if(ReadInteger(refnum,&y,&pos) == FAILED) goto ERR; TickChannel[i] = y;
	if(ReadInteger(refnum,&y,&pos) == FAILED) goto ERR; TickVelocity[i] = y;
	if(ReadInteger(refnum,&y,&pos) == FAILED) goto ERR; TickCycle[i] = y;
	Ptick[i] = Qtick[i] = 1L; MuteTick[i] = FALSE;
	if(arg > 4) {
		if(ReadLong(refnum,&x,&pos) == FAILED) goto ERR; Ptick[i] = x;
		if(ReadLong(refnum,&x,&pos) == FAILED) goto ERR; Qtick[i] = x;
		}
	if(arg > 6) {
		if(ReadInteger(refnum,&y,&pos) == FAILED) goto ERR; TickDuration[i] = y;
		}
	for(j=0; j < maxbeats; j++) {
		if(ReadLong(refnum,&x,&pos) == FAILED) goto ERR;
		ThisTick[i][j] = x;
		if(x % 2) TickThere = TRUE;
		}
	}
result = OK;
if(ReadOne(FALSE,TRUE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
if(iv > 9) {
	MyLock(FALSE,(Handle)p_completeline);
	result = SetField(NULL,wTimeBase,fTimeBaseComment,*p_completeline);
	MyUnlock((Handle)p_completeline);
	}
else SetField(NULL,wTimeBase,fTimeBaseComment,"[Comment on time base]");
goto QUIT;

ERR:
GetTimeBase(); GetTickParameters();
result = FAILED;
sprintf(Message,"Error reading ‘%s’ time base file…",FileName[wTimeBase]);
Alert1(Message);
FileName[wTimeBase][0] = '\0';

QUIT:
MyDisposeHandle((Handle*)&p_line);
MyDisposeHandle((Handle*)&p_completeline);
for(i=0; i < MAXTICKS; i++) SetTickParameters(i+1,MAXBEATS);
SetTickParameters(0,MAXBEATS);
if(FSClose(refnum) != noErr) {
	sprintf(Message,"Error closing ‘%s’ time base file…",FileName[wTimeBase]);
	Alert1(Message);
	result = FAILED;
	}
if(result == OK) {
	SetName(wTimeBase,TRUE,TRUE);
	Dirty[wTimeBase] = Created[wTimeBase] = FALSE;
	}
HideWindow(Window[wMessage]);
ResetTickFlag = TRUE;
Dirty[wTimeBase] = Created[wTimeBase] = FALSE;
LoadOn--;
return(result);
}


SaveTimeBase(FSSpec *p_spec)
{
int i,j,good;
short refnum;
Str255 fn;
long x,count,p,q;
StandardFileReply reply;
char line[MAXFIELDCONTENT];

GetTimeBase(); GetTickParameters();
if(FileName[wTimeBase][0] != '\0') strcpy(Message,FileName[wTimeBase]);
else strcpy(Message,FilePrefix[wTimeBase]);
pStrCopy((char*)c2pstr(Message),fn);
p_spec->vRefNum = TheVRefNum[wTimeBase];
p_spec->parID = WindowParID[wTimeBase];
pStrCopy((char*)fn,p_spec->name);
good = NO;
if(Created[wTimeBase]) good = (MyOpen(p_spec,fsCurPerm,&refnum) == noErr);
if(good) goto WRITE;
reply.sfFile.vRefNum = TheVRefNum[wTimeBase];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[wTimeBase];
if(NewFile(fn,&reply)) {
	i = CreateFile(wTimeBase,-1,gFileType[wTimeBase],fn,&reply,&refnum);
	*p_spec = reply.sfFile;
	if(i == ABORT) return(FAILED);
	if(i == OK) {
WRITE:
		SetCursor(&WatchCursor);
		WriteHeader(wTimeBase,refnum,*p_spec);
		sprintf(LineBuff,"%ld",(long)MAXTICKS); WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)MAXBEATS); WriteToFile(NO,MAC,LineBuff,refnum);
		for(i=0; i < MAXTICKS; i++) {
			sprintf(LineBuff,"1\r7");	/* '1' is the type and '4' the number of parameters */
			WriteToFile(NO,MAC,LineBuff,refnum);
			sprintf(LineBuff,"%ld\r%ld\r%ld\r%ld\r%ld\r%ld\r%ld",
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
		MyPtoCstr(MAXNAME,p_spec->name,FileName[wTimeBase]);
		TheVRefNum[wTimeBase] = p_spec->vRefNum;
		WindowParID[wTimeBase] = p_spec->parID;
		SetName(wTimeBase,TRUE,TRUE);
		Created[wTimeBase] = TRUE;
		Dirty[wTimeBase] = FALSE;
		return(OK);
		}
	else {
		MyPtoCstr(MAXNAME,fn,LineBuff);
		sprintf(Message,"Error creating ‘%s’",LineBuff);
		Alert1(Message);
		}
	}
return(FAILED);
}


LoadCsoundInstruments(short refnum,int manual)
{
int i,io,iv,ip,jmax,j,type,result,y,maxticks,maxbeats,arg,length;
char **ptr;
Handle **ptr2;
CsoundParam **ptr3;
long pos,x;
char **p_line,**p_completeline;
double r;

iCsoundInstrument = 0;
LoadOn++;
pos = ZERO; Dirty[wCsoundInstruments] = CompiledRegressions = CompiledCsObjects = FALSE;
p_line = p_completeline = NULL;
if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
if(CheckVersion(&iv,p_line,FileName[wCsoundInstruments]) != OK) goto ERR;
if(ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
GetDateSaved(p_completeline,&(p_FileInfo[wCsoundInstruments]));
if(ReadInteger(refnum,&jmax,&pos) == FAILED) goto ERR;
if(jmax < 16 || jmax > MAXCHAN) {
	Alert1("This file is empty or in an unknown format");
	goto QUIT;
	}
for(j=1; j <= jmax; j++) {
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	WhichCsoundInstrument[j] = i;
	}
if(iv > 11) {
	if(ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
	MystrcpyHandleToString(MAXNAME,0,CsoundOrchestraName,p_completeline);
	}
else CsoundOrchestraName[0] = '\0';
if(ReadInteger(refnum,&jmax,&pos) == FAILED) goto ERR;
if(jmax <= 0) {
	Alert1("This file is empty or in an unknown format");
	goto QUIT;
	}
if((result=ResizeCsoundInstrumentsSpace(jmax)) != OK) goto ERR;
result = FAILED;
for(j=0; j < jmax; j++) {
	PleaseWait();
	if(ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
	ptr = (*pp_CsInstrumentName)[j];
	if((*p_completeline)[0] != '\0') {
		MystrcpyHandleToString(MAXLIN,0,LineBuff,p_completeline);
		if(ShowMessages) ShowMessage(TRUE,wMessage,LineBuff);
		if(MySetHandleSize((Handle*)&ptr,
			(1L + MyHandleLen(p_completeline)) * sizeof(char)) != OK)
				goto ERR;
		MystrcpyHandleToHandle(0,&ptr,p_completeline);
		(*pp_CsInstrumentName)[j] = ptr;
		}
	else (*((*pp_CsInstrumentName)[j]))[0] = '\0';
	
	if(ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
	ptr = (*pp_CsInstrumentComment)[j];
	if((*p_completeline)[0] != '\0') {
		if(MySetHandleSize((Handle*)&ptr,
			(1L + MyHandleLen(p_completeline)) * sizeof(char)) != OK)
				goto ERR;
		MystrcpyHandleToHandle(0,&ptr,p_completeline);
		(*pp_CsInstrumentComment)[j] = ptr;
		}
	else (*((*pp_CsInstrumentComment)[j]))[0] = '\0';
	
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].iargmax = i;
	
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrumentIndex)[j] = i;
	
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsDilationRatioIndex)[j] = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsAttackVelocityIndex)[j] = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsReleaseVelocityIndex)[j] = i;
	
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsPitchIndex)[j] = i;
	
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsPitchFormat)[j] = i;
	
	if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].pitchbendrange = r;
	
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].rPitchBend.islogx = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].rPitchBend.islogy = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].rVolume.islogx = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].rVolume.islogy = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].rPressure.islogx = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].rPressure.islogy = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].rModulation.islogx = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].rModulation.islogy = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].rPanoramic.islogx = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].rPanoramic.islogy = i;
	
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsPitchBendStartIndex)[j] = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsVolumeStartIndex)[j] = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsPressureStartIndex)[j] = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsModulationStartIndex)[j] = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsPanoramicStartIndex)[j] = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsPitchBendEndIndex)[j] = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsVolumeEndIndex)[j] = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsPressureEndIndex)[j] = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsModulationEndIndex)[j] = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsPanoramicEndIndex)[j] = i;

	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].pitchbendtable = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].volumetable = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].pressuretable = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].modulationtable = i;
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	(*p_CsInstrument)[j].panoramictable = i;
	
	if(iv > 13) {
		if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
		(*p_CsInstrument)[j].pitchbendGEN = i;
		if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
		(*p_CsInstrument)[j].volumeGEN = i;
		if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
		(*p_CsInstrument)[j].pressureGEN = i;
		if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
		(*p_CsInstrument)[j].modulationGEN = i;
		if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
		(*p_CsInstrument)[j].panoramicGEN = i;
		}
	else {
		(*p_CsInstrument)[j].pitchbendGEN = (*p_CsInstrument)[j].volumeGEN
			= (*p_CsInstrument)[j].pressureGEN = (*p_CsInstrument)[j].modulationGEN
			= (*p_CsInstrument)[j].panoramicGEN = 7;
		}

	for(i=0; i < 6; i++) {
		if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
		(*(p_CsPitchBend[i]))[j] = r;
		if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
		(*(p_CsVolume[i]))[j] = r;
		if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
		(*(p_CsPressure[i]))[j] = r;
		if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
		(*(p_CsModulation[i]))[j] = r;
		if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
		(*(p_CsPanoramic[i]))[j] = r;
		}
		
	for(ip=0; ip < (*p_CsInstrument)[j].ipmax; ip++) {
		if((*p_CsInstrument)[j].paramlist == NULL) {
			if(Beta) Alert1("Err. LoadCsoundInstruments(). (*p_CsInstrument)[j].paramlist == NULL");
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
	
	if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
	if(i < 1) continue;
	if(i > IPMAX) {
		if(Beta) Alert1("Err. LoadCsoundInstruments(). i > IPMAX");
		Alert1("This ‘-cs’ file was created by a newer version of BP2. Some parameters may be ignored");
		i = IPMAX;
		}
	if((ptr3=(CsoundParam**) GiveSpace((Size)(IPMAX * sizeof(CsoundParam)))) == NULL)
		goto ERR;
	(*p_CsInstrument)[j].paramlist = ptr3;
	(*p_CsInstrument)[j].ipmax = i;
	for(ip=0; ip < IPMAX; ip++) {
		(*((*p_CsInstrument)[j].paramlist))[ip].name = NULL;
		(*((*p_CsInstrument)[j].paramlist))[ip].comment = NULL;
		ResetMoreParameter(j,ip);
		}
	
	for(ip=0; ip < (*p_CsInstrument)[j].ipmax; ip++) {
		if(ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
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
/*		else (*((*((*p_CsInstrument)[j].paramlist))[ip].name))[0] = '\0'; $$$ Fixed 7/3/98 */
		
		if(ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
		length = MyHandleLen(p_completeline);
		if(length > 0) {
			if((ptr=(char**) GiveSpace((Size)((1L + length)
				* sizeof(char)))) == NULL) goto ERR;
			MystrcpyHandleToHandle(0,&ptr,p_completeline);
			(*((*p_CsInstrument)[j].paramlist))[ip].comment = ptr;
			}
		
		if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
		(*((*p_CsInstrument)[j].paramlist))[ip].startindex = i;
		if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
		(*((*p_CsInstrument)[j].paramlist))[ip].endindex = i;
		if(iv > 12) {
			if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
			(*((*p_CsInstrument)[j].paramlist))[ip].table = i;
			if(ReadFloat(refnum,&r,&pos) == FAILED) goto ERR;
			(*((*p_CsInstrument)[j].paramlist))[ip].defaultvalue = r;
			if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
			(*((*p_CsInstrument)[j].paramlist))[ip].GENtype = i;
			if(ReadInteger(refnum,&i,&pos) == FAILED) goto ERR;
			(*((*p_CsInstrument)[j].paramlist))[ip].combinationtype = i;
			if(i == MULT && fabs(r) < 0.01) {
				(*((*p_CsInstrument)[j].paramlist))[ip].defaultvalue = 1.;
				sprintf(Message,"In instrument %ld a default parameter value ‘%.3f’ was replaced with ‘1’ because its combination mode is multiplicative",
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
if(ReadOne(FALSE,FALSE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto QUIT;
if(Mystrcmp(p_line,"_begin tables") == 0) {
	ClearWindow(NO,wCsoundTables);
	ReadFile(wCsoundTables,refnum);
	Dirty[wCsoundTables] = FALSE;
	}
else ClearWindow(YES,wCsoundTables);
goto QUIT;

ERR:
sprintf(Message,"Error reading ‘%s’ Csound instrument file…",FileName[wCsoundInstruments]);
Alert1(Message);
FileName[wCsoundInstruments][0] = '\0';

QUIT:
result = OK;
MyDisposeHandle((Handle*)&p_line);
MyDisposeHandle((Handle*)&p_completeline);
if(FSClose(refnum) != noErr) {
	sprintf(Message,"Error closing ‘%s’ Csound instrument file…",FileName[wCsoundInstruments]);
	Alert1(Message);
	result = FAILED;
	}
if(result == OK) {
	Dirty[wCsoundInstruments] = Created[wCsoundInstruments] = FALSE;
	LoadedCsoundInstruments = TRUE;
/*	No SetName(wCsoundInstruments,TRUE,TRUE); */
	}
HideWindow(Window[wMessage]);
Dirty[wCsoundInstruments] = Created[wCsoundInstruments] = FALSE;
if(iCsoundInstrument >= Jinstr) iCsoundInstrument = 0;
SetCsoundInstrument(iCsoundInstrument,-1);
LoadOn--;
return(result);
}


SaveCsoundInstruments(FSSpec* p_spec)
{
int i,j,good,ishtml;
short refnum;
Str255 fn;
long x,count,p,q;
StandardFileReply reply;
char line[MAXFIELDCONTENT];

if(FileName[wCsoundInstruments][0] != '\0') strcpy(Message,FileName[wCsoundInstruments]);
else strcpy(Message,FilePrefix[wCsoundInstruments]);
pStrCopy((char*)c2pstr(Message),fn);
p_spec->vRefNum = TheVRefNum[wCsoundInstruments];
p_spec->parID = WindowParID[wCsoundInstruments];
pStrCopy((char*)fn,p_spec->name);
good = NO;
if(Created[wCsoundInstruments]) good = (MyOpen(p_spec,fsCurPerm,&refnum) == noErr);
if(good) goto WRITE;
reply.sfFile.vRefNum = TheVRefNum[wCsoundInstruments];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[wCsoundInstruments];
if(NewFile(fn,&reply)) {
	i = CreateFile(wCsoundInstruments,-1,gFileType[wCsoundInstruments],fn,&reply,&refnum);
	*p_spec = reply.sfFile;
	if(i == ABORT) return(FAILED);
	if(i == OK) {
WRITE:
		SaveOn++;
		WriteHeader(wCsoundInstruments,refnum,*p_spec);
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
					if(Beta) Alert1("Err. SaveCsoundInstruments(). (*p_CsInstrument)[j].paramlist == NULL");
					break;
					}
				MystrcpyHandleToString(MAXFIELDCONTENT,0,line,
					(*((*p_CsInstrument)[j].paramlist))[i].name);
				WriteToFile(YES,MAC,line,refnum);
				MystrcpyHandleToString(MAXFIELDCONTENT,0,line,
					(*((*p_CsInstrument)[j].paramlist))[i].comment);
				WriteToFile(YES,MAC,line,refnum);
				sprintf(line,"%ld\r%ld",
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
		MyFSClose(wCsoundInstruments,refnum,p_spec);
		MyPtoCstr(MAXNAME,p_spec->name,FileName[wCsoundInstruments]);
		TheVRefNum[wCsoundInstruments] = p_spec->vRefNum;
		WindowParID[wCsoundInstruments] = p_spec->parID;
		SetName(wCsoundInstruments,TRUE,TRUE);
		Created[wCsoundInstruments] = TRUE;
		Dirty[wCsoundInstruments] = FALSE;
		ClearMessage();
		if(SaveOn > 0) SaveOn--;
		return(OK);
		}
	else {
		MyPtoCstr(MAXNAME,fn,LineBuff);
		sprintf(Message,"Error creating ‘%s’",LineBuff);
		Alert1(Message);
		}
	}
return(FAILED);
}


SaveSettings(int startup,int now,Str255 fn,FSSpec* p_spec)
{
short refnum;
int i,io,imax,j,rep,w,good,ishtml,result;
StandardFileReply reply;
long count,a,b;
char line[MAXNAME+1];
GrafPtr saveport;
Rect r;
Point p,q;

if(ScriptExecOn) return(OK);
p_spec->vRefNum = TheVRefNum[iSettings];
p_spec->parID = WindowParID[iSettings];
strcpy(line,FilePrefix[iSettings]);
if(fn[0] == 0) pStrCopy((char*)c2pstr(line),fn);
pStrCopy((char*)fn,p_spec->name);
good = NO;
if(now) good = (MyOpen(p_spec,fsCurPerm,&refnum) == noErr);
MyPtoCstr(MAXNAME,p_spec->name,line);
if(good) goto WRITE;
reply.sfFile.vRefNum = TheVRefNum[iSettings];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[iSettings];
result = FAILED;

if(NewFile(fn,&reply)) {
	io = CreateFile(iSettings,iSettings,gFileType[iSettings],fn,&reply,&refnum);
	*p_spec = reply.sfFile;
	if(io == ABORT) return(FAILED);
	MyPtoCstr(MAXNAME,fn,line);
	if(io == OK) {
WRITE:
		SaveOn++;
		sprintf(Message,"Saving ‘%s’...",line);
		PleaseWait();
		ShowMessage(TRUE,wMessage,Message);
		WriteHeader(iSettings,refnum,*p_spec);
		sprintf(LineBuff,"%ld",(long)Port);
		WriteToFile(NO,MAC,LineBuff,refnum);
		
		WriteToFile(NO,MAC," ",refnum);
		
		sprintf(LineBuff,"%ld\r%ld\r%ld",(long)Quantization,(long)Time_res,(long)SetUpTime);
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
		sprintf(LineBuff,"%ld\r%ld\r%ld\r%ld\r%ld",(long)SplitTimeObjects,(long)SplitVariables,
			(long)UseTextColor,(long)DeftBufferSize,(long)UseGraphicsColor);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)UseBufferLimit); WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)TimeMax); WriteToFile(NO,MAC,LineBuff,refnum);
		GetSeed();
		sprintf(LineBuff,"%.0f\r%ld",(double)Seed,(long)Token);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld\r%ld\r%ld",(long)NoteConvention,(long)StartFromOne,
			(long)SmartCursor);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld\r%ld",(long)GraphicScaleP,(long)GraphicScaleQ);
		WriteToFile(NO,MAC,LineBuff,refnum);
		
		if(Oms && OMSinputName[0] != '\0' && OMSinputName[0] != '<') {
			if(gChosenInputIDbydefault > 0)
				sprintf(LineBuff,"%ld %s",(long)gChosenInputIDbydefault,OMSinputName);
			else
				sprintf(LineBuff,"%ld %s",(long)gChosenInputID,OMSinputName);
			}
		else sprintf(LineBuff,"<no input device>");
		WriteToFile(NO,MAC,LineBuff,refnum);
		MoveDisk();
		if(Oms && OMSoutputName[0] != '\0')
			sprintf(LineBuff,"%ld %s",(long)gChosenOutputID,OMSoutputName);
		else sprintf(LineBuff,"<no output device>");
		WriteToFile(NO,MAC,LineBuff,refnum);
			
		sprintf(LineBuff,"%ld",(long)UseBullet); WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)PlayTicks); WriteToFile(NO,MAC,LineBuff,refnum);
		
		sprintf(LineBuff,"%ld\r%ld",(long)FileSaveMode,(long)FileWriteMode);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld\r%ld\r%ld\r%.2f",(long)MIDIfileType,(long)CsoundFileFormat,
			(long)ProgNrFrom,MIDIfadeOut);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)C4key);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%.4f",A4freq);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)StrikeAgainDefault);
		WriteToFile(NO,MAC,LineBuff,refnum);
		
		sprintf(LineBuff,"%ld\r%ld\r%ld\r%ld\r%ld\r%ld",(long)DeftVolume,(long)VolumeController,
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
				SetPort(Window[w]);
				WriteToFile(NO,MAC,LineBuff,refnum);
				if(ChangedCoordinates[w]) {
					r = Window[w]->portRect;
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
			else if(Beta) Alert1("Err SaveSettings(). saveport == NULL");
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
		Dirty[iSettings] = FALSE;
		if(!startup) {
			Created[iSettings] = TRUE;
			MyPtoCstr(MAXNAME,fn,FileName[iSettings]);
			SetName(iSettings,TRUE,TRUE);
			TheVRefNum[iSettings] = p_spec->vRefNum;
			WindowParID[iSettings] = p_spec->parID;
			}
		result = OK;
		if(SaveOn > 0) SaveOn--;
		}
	else {
		MyPtoCstr(MAXNAME,fn,line);
		sprintf(Message,"Error creating ‘%s’",line);
		Alert1(Message);
		result = FAILED;
		}
	}
HideWindow(Window[wMessage]);
return(result);
}


LoadSettings(int anyfile,int changewindows,int startup,int manual,int *p_oms)
{
int i,ii,imax,j,jmax,io,rep,result,iv,s,type,top,left,bottom,right,w,wmax,connectionok,
	maxticks,maxbeats,oldoutmidi,oldoutcsound,oldwritemidifile,toldoms,tried,activemem;
FSSpec spec;
short refnum,oldomsinput,oldomsoutput,newomsinput,newomsoutput;
char filename[MAXNAME+1],connectionname[MAXNAME+1],oldinputname[MAXNAME+1];
long pos,k;
unsigned long kk;
FontInfo font;
double x;
Rect r;
char **p_line,**p_completeline;
OMSNodeInfoListH info;
OMSNodeInfoList node;

result = connectionok = OK;
oldoutmidi = OutMIDI;
p_line = p_completeline = NULL;
if((rep=ClearWindow(FALSE,wStartString)) != OK) return(rep);
strcpy(Message,FileName[iSettings]);
if(startup) {
	sprintf(Message,"%sstartup",FilePrefix[iSettings]);
	spec.vRefNum = RefNumbp2;
	spec.parID = ParIDbp2;
	}
else {
	spec.vRefNum = TheVRefNum[iSettings];
	spec.parID = WindowParID[iSettings];
	}
strcpy(LineBuff,Message);
strcpy(filename,LineBuff);
type = gFileType[iSettings];
if(anyfile) type = 0;
pStrCopy((char*)c2pstr(Message),spec.name);
if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
	rep = FAILED;
	if(startup || (rep=CheckFileName(iSettings,LineBuff,&spec,&refnum,gFileType[iSettings],TRUE)) != OK) {
		sprintf(Message,"Can't find ‘%s’ setting file...",LineBuff);
		ShowMessage(TRUE,wMessage,Message);
		return(rep);
		}
	else strcpy(filename,FileName[iSettings]);
	}
if(!startup) HideWindow(Window[wMessage]);
PleaseWait();
pos = ZERO; Dirty[iSettings] = Created[iSettings] = FALSE;

LoadOn++;

if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
if(CheckVersion(&iv,p_line,filename) != OK) {
	result = FAILED;
	goto QUIT;
	}
if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;

if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
if(startup) Port = j;
switch(Port) {
	case 1:
		Portbit = PORTA; break;
	case 2:
		Portbit = PORTB; break;
	}

if(ReadOne(FALSE,FALSE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;	/* Not used */
if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR; Quantization = k;
if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR; Time_res = k;
if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR; SetUpTime = j;
if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR; QuantizeOK = j;
SetTimeAccuracy();
Dirty[wTimeAccuracy] = FALSE;
NotSaidKpress = TRUE;

if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR; Nature_of_time = j;
if(ReadUnsignedLong(refnum,&kk,&pos) == FAILED) goto ERR; Pclock = (double)kk;
if(ReadUnsignedLong(refnum,&kk,&pos) == FAILED) goto ERR; Qclock = (double)kk;
SetTempo(); SetTimeBase(); Dirty[wMetronom] = Dirty[wTimeBase] = FALSE;

if(ReadInteger(refnum,&jmax,&pos) == FAILED) goto ERR;
if(jmax > Jbutt) {
	sprintf(Message,"Err. settings file.  jmax = %ld. ",(long)jmax);
	if(Beta) Alert1(Message);
	goto QUIT;
	}
	
oldwritemidifile = WriteMIDIfile;
oldoutcsound = OutCsound;

if(ReadInteger(refnum,&Improvize,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&CyclicPlay,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&UseEachSub,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&AllItems,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&DisplayProduce,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&StepProduce,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&StepGrammars,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&TraceProduce,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&PlanProduce,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&DisplayItems,&pos) == FAILED) goto ERR; 
if(ReadInteger(refnum,&ShowGraphic,&pos) == FAILED) goto ERR; 
if(ReadInteger(refnum,&AllowRandomize,&pos) == FAILED) goto ERR;
if(iv < 15) AllowRandomize = TRUE;
if(ReadInteger(refnum,&DisplayTimeSet,&pos) == FAILED) goto ERR; 
if(ReadInteger(refnum,&StepTimeSet,&pos) == FAILED) goto ERR; 
if(ReadInteger(refnum,&TraceTimeSet,&pos) == FAILED) goto ERR; 
if(jmax > 27) ReadInteger(refnum,&CsoundTrace,&pos);
else CsoundTrace = FALSE;
if(ReadInteger(refnum,&OutMIDI,&pos) == FAILED) goto ERR; 
if(ReadInteger(refnum,&SynchronizeStart,&pos) == FAILED) goto ERR; 
if(ReadInteger(refnum,&ComputeWhilePlay,&pos) == FAILED) goto ERR; 
if(ReadInteger(refnum,&Interactive,&pos) == FAILED) goto ERR; 
if(jmax > 19) ReadInteger(refnum,&ResetWeights,&pos);
else ResetWeights = FALSE;
NeverResetWeights = FALSE;
if(jmax > 20) ReadInteger(refnum,&ResetFlags,&pos);
else ResetFlags = FALSE;
if(jmax > 21) ReadInteger(refnum,&ResetControllers,&pos);
else ResetControllers = FALSE; 
if(jmax > 22) ReadInteger(refnum,&NoConstraint,&pos);
else NoConstraint = FALSE;
if(jmax > 23) ReadInteger(refnum,&WriteMIDIfile,&pos);
else WriteMIDIfile = FALSE; 
if(jmax > 24) ReadInteger(refnum,&ShowMessages,&pos); 
if(jmax > 25) ReadInteger(refnum,&OutCsound,&pos);
else OutCsound = FALSE;
if(jmax > 26) ReadInteger(refnum,p_oms,&pos);
else *p_oms = FALSE;

SetButtons(TRUE);

if(oldoutcsound && !OutCsound && !startup) CloseCsScore();
if(oldwritemidifile && !WriteMIDIfile && !startup) CloseMIDIFile();
if(OutMIDI && !oldoutmidi && !InitOn && !startup) ResetMIDI(FALSE);

if(ReadInteger(refnum,&SplitTimeObjects,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&SplitVariables,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
UseTextColor = (j > 0);
if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
DeftBufferSize = BufferSize = k;
if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
UseGraphicsColor = (j > 0);
if(ForceTextColor == 1) UseTextColor = TRUE;
if(ForceTextColor == -1) UseTextColor = FALSE;
if(ForceGraphicColor == 1) UseGraphicsColor = TRUE;
if(ForceGraphicColor == -1) UseGraphicsColor = FALSE;
if(ReadInteger(refnum,&UseBufferLimit,&pos) == FAILED) goto ERR;
SetBufferSize();
if(ReadLong(refnum,&TimeMax,&pos) == FAILED) goto ERR;

if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
Seed = (unsigned) (k % 32768L);
SetSeed(); ResetRandom();

if(ReadInteger(refnum,&Token,&pos) == FAILED) goto ERR;
if(Token > 0) Token = TRUE;
else Token = FALSE;
if(ReadInteger(refnum,&NoteConvention,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&StartFromOne,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
SmartCursor = (j == 1);
if(ReadInteger(refnum,&GraphicScaleP,&pos) == FAILED) goto ERR;
if(ReadInteger(refnum,&GraphicScaleQ,&pos) == FAILED) goto ERR;
SetGraphicSettings();

/* Find OMS default input device, and connect it if OMS is active */
if(ReadOne(FALSE,FALSE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
if((*p_oms) && iv > 14 && OKOMS) {
	oldomsinput = gChosenInputID;
	MystrcpyHandleToString(MAXNAME,0,connectionname,p_line);
	if(Beta) {
		sprintf(Message,"MIDI input found in settings file: %s",connectionname);
		ShowMessage(YES,wMessage,Message);
		}
	newomsinput = GetIDandName(connectionname);
	if((strcmp(OMSinputName,connectionname) != 0
			|| (connectionname[0] == '\0' && newomsinput != 0)) && connectionname[0] != '<') {
		strcpy(oldinputname,OMSinputName);
		if(newomsinput != 0) {
			if(Oms) {
				OpenOrCloseConnection(startup,FALSE);
				OMSinputName[0] = '\0';
				}
			gChosenInputID = newomsinput;
			}
		tried = toldoms = FALSE;
		
	TRYOPEN:
		if(Oms) {
			if(newomsinput == 0) {
				tried = TRUE; 
				gChosenInputID = newomsinput = FindOMSdevice(YES,connectionname);
				}
			if(gChosenInputID > 0 && gInputMenu != NULL) {
				SetOMSDeviceMenuSelection(gInputMenu,0,gChosenInputID,"\p",TRUE);
				strcpy(OMSinputName,connectionname);
				}
			connectionok = OpenOrCloseConnection(startup,TRUE);
			if(!connectionok) {
				connectionok = TRUE;
				if(!tried) {
					newomsinput = 0;
					goto TRYOPEN;
					}
				else {
					if(oldomsinput > 0) {
						gChosenInputID = oldomsinput;
						if(gInputMenu != NULL)
							SetOMSDeviceMenuSelection(gInputMenu,0,gChosenInputID,"\p",TRUE);
						if(OpenOrCloseConnection(startup,TRUE) != OK) {
							connectionok = FALSE;
							}
						else strcpy(OMSinputName,oldinputname);
						}
					}
				}
			}
		else {
			if(connectionname[0] != '\0' || startup) strcpy(OMSinputName,connectionname);
			if(!startup) {
				toldoms = TRUE;
				if(!ScriptExecOn)
					Alert1("Settings indicate that the intput was routed via OMS. You should install and activate it…");
				}
			}
		}
	}
	
/* Find OMS default output device, and connect it if OMS is active */
if(iv > 5) {
	if(ReadOne(FALSE,FALSE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
	if((*p_oms) && iv > 14 && OKOMS) {
		oldomsoutput = gChosenOutputID;
		MystrcpyHandleToString(MAXNAME,0,connectionname,p_line);
		if(Beta) {
			sprintf(Message,"MIDI output found in settings file: %s",connectionname);
			ShowMessage(YES,wMessage,Message);
			}
		newomsoutput = GetIDandName(connectionname);
		if((strcmp(OMSoutputName,connectionname) != 0
				|| (connectionname[0] == '\0' && newomsoutput != 0)) && connectionname[0] != '<') {
			if(Oms) {
				activemem = SchedulerIsActive;
				SchedulerIsActive = FALSE;
				gChosenOutputID = newomsoutput;
				gOutNodeRefNum = OMSUniqueIDToRefNum(gChosenOutputID);
				if(gOutNodeRefNum == OMSInvalidRefNum) {
					gChosenOutputID = FindOMSdevice(NO,connectionname);
					gOutNodeRefNum = OMSUniqueIDToRefNum(gChosenOutputID);
					}
				if(gOutNodeRefNum == OMSInvalidRefNum) {
					if(oldomsoutput > 0) {
						// Try to reconnect previous device
						gChosenOutputID = oldomsoutput;
						gOutNodeRefNum = OMSUniqueIDToRefNum(gChosenOutputID);
						if(gOutNodeRefNum == OMSInvalidRefNum) {
							// If it doesn't work, select the first device available
					/*		info = OMSGetNodeInfo(omsIncludeReal+omsIncludeOutputs);
							if(info != NULL) {
								node = (*info)[0];
								if(node.numNodes > 0) {
									gChosenOutputID = node.info[0].uniqueID;
									if(gOutputMenu != NULL)
										SetOMSDeviceMenuSelection(gOutputMenu,0,gChosenOutputID,"\p",TRUE);
									gOutNodeRefNum = OMSUniqueIDToRefNum(gChosenOutputID);
									MyPtoCstr(255,node.info[0].name,OMSoutputName);
									}
								OMSDisposeHandle(info);
								} */
							connectionok = FALSE;
							gChosenOutputID = 0;
							sprintf(Message,
								"BP2 was unable to reconnect output port ‘%s’. Check MIDI output and save project settings…",
								connectionname);
							if(!ScriptExecOn) Alert1(Message);
							else Println(wTrace,Message);
							}
						}
					}
				else {
					strcpy(OMSoutputName,connectionname);
					io = OMSAddPort('Bel0',gChosenOutputID,omsPortTypeOutput,NULL,0L,&gOutputPortRefNum);
					if(io != noErr) {
						if(io == 4) {
							sprintf(Message,"The ‘%s’ MIDI output port was already open…",
								connectionname);
							ShowMessage(TRUE,wMessage,Message);
							}
						else {
							if(Beta) TellError(33,io);
							SetOMSDeviceMenuSelection(gOutputMenu,0,gChosenOutputID,"\p",FALSE);
							connectionok = FALSE;
							gChosenOutputID = 0;
							OMSoutputName[0] = '\0';
							sprintf(Message,
								"BP2 was unable to open output port ‘%s’ for this project. You may need to run OMS setup",
								connectionname);
							if(!ScriptExecOn) Alert1(Message);
							else Println(wTrace,Message);
							}
						}
					else {
						sprintf(Message,"Opened MIDI output ‘%s’…",connectionname);
						ShowMessage(TRUE,wMessage,Message);
						}
					if(gOutputMenu != NULL && gChosenOutputID != 0)
						SetOMSDeviceMenuSelection(gOutputMenu,0,gChosenOutputID,"\p",TRUE);
					}
				SchedulerIsActive = activemem;
				}
			else {
				if(connectionname[0] != '\0') strcpy(OMSoutputName,connectionname);
				if(newomsoutput > 0) gChosenOutputID = newomsoutput;
				if(!startup && !toldoms && !ScriptExecOn)
					Alert1("Settings indicate that the output was routed via OMS. You should install and activate it…");
				}
			}
		}
	}

if(iv > 11) {
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	if(iv > 15) UseBullet = j;
	else UseBullet = TRUE;
	if(UseBullet) Code[7] = '•';
	else Code[7] = '.';
	}

PlayTicks = FALSE;
SwitchOff(NULL,wTimeBase,dPlayTicks);
ResetTickFlag = TRUE;

if(iv > 7) {
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	PlayTicks = j;
	if(PlayTicks && !InitOn && !startup) {
		ResetMIDI(FALSE);
		SwitchOn(NULL,wTimeBase,dPlayTicks);
		}
	}
if(iv > 10) {
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	FileSaveMode = j;
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	FileWriteMode = j;
	}
else {
	FileSaveMode = ALLSAMEPROMPT;
	FileWriteMode = LATER;
	}
if(iv > 11) {
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	MIDIfileType = j;
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	CsoundFileFormat = j;
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	ProgNrFrom = j;
	if(ProgNrFrom == 0) {
/*		if(Beta) Alert1("Old program numbers"); */
		ProgNrFrom = 1;
		}
	if(ReadFloat(refnum,&x,&pos) == FAILED) goto ERR;
	if(iv > 19) MIDIfadeOut = x;
	else MIDIfadeOut = 2.;
	sprintf(Message,"%.2f",MIDIfadeOut);
	SetField(FileSavePreferencesPtr,-1,fFadeOut,Message);
	
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	if(j > 1 && j < 128) C4key = j;
	else C4key = 60;
	ReadFloat(refnum,&x,&pos);
	if(x > 1.) A4freq = x;
	else A4freq = 440.;
	
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	StrikeAgainDefault = j;
	}
else {
	MIDIfileType = 0;
	CsoundFileFormat = MAC;
	StrikeAgainDefault = TRUE;
	C4key = 48;	/* Here we compensate bad convention on old projects */
	A4freq = 220.;	/* ditto */
	}
if(iv > 15) {
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	DeftVolume = j;
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	VolumeController = j;
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	DeftVelocity = j;
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	DeftPanoramic = j;
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	PanoramicController = j;
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	SamplingRate = j;
	}
else {
	DeftVolume = DEFTVOLUME;
	VolumeController = VOLUMECONTROL;
	DeftVelocity = DEFTVELOCITY;
	DeftPanoramic = DEFTPANORAMIC;
	PanoramicController = PANORAMICCONTROL;
	SamplingRate = SAMPLINGRATE;
	}
SetFileSavePreferences();
SetDefaultPerformanceValues();
SetTuning();
SetDefaultStrikeMode();

if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
wmax = j;
if(wmax > 0) {
	for(w=0; w < wmax; w++) {
		PleaseWait();
		if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
		if(changewindows && WindowTextSize[w] != j) SetFontSize(w,j);
		}
	MaintainMenus();
	}
	
ResetMIDIFilter();

if(iv > 4) {
	if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
	MIDIoutputFilter = k;
	if(startup) MIDIoutputFilterstartup = MIDIoutputFilter;
	GetOutputFilterWord();
	for(i=0; i < 12; i++) {
		if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
		NameChoice[i] = j;
		}
	}
if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
if(k != 0L) {
	MIDIinputFilter = k;
	if(startup) MIDIinputFilterstartup = MIDIinputFilter;
	GetInputFilterWord();
	if(!ScriptExecOn && !NoteOnIn) {
		Alert1("Reception of NoteOn's is disabled. Most MIDI data received by BP2 will be meaningless");
		}
	}
SetFilterDialog();

if(iv > 19) {
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	ShowObjectGraph = j;
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	ShowPianoRoll = j;
	if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
	for(i=0; i < j; i++) {
		if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
		// PianoColor[i].red = k;
		if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
		// PianoColor[i].green = k;
		if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
		// PianoColor[i].blue = k;
		}
	}
else {
	ResetPianoRollColors();
	ShowObjectGraph = TRUE;
	ShowPianoRoll = FALSE;
	}
				
if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
NewEnvironment = j;
if(NewEnvironment) {
	if(ReadInteger(refnum,&wmax,&pos) == FAILED) goto ERR;
	for(w=0; w < wmax; w++) {
		if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
		ChangedCoordinates[w] = j;
		if(j) {
			if(ReadInteger(refnum,&top,&pos) == FAILED) goto ERR;
			if(ReadInteger(refnum,&left,&pos) == FAILED) goto ERR;
			if(ReadInteger(refnum,&bottom,&pos) == FAILED) goto ERR;
			if(ReadInteger(refnum,&right,&pos) == FAILED) goto ERR;
			}
		if(changewindows) AdjustWindow(j,w,top,left,bottom,right);
		}
	}
if(FreezeWindows) NewEnvironment = FALSE;
if(iv > 4) {
	if(ReadInteger(refnum,&imax,&pos) == FAILED) goto ERR;
	if(imax > 0) {
		NewColors = TRUE;
		for(i=0; i < imax; i++) {
			if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
			Color[i].red = (unsigned) j;
			if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
			Color[i].green = (unsigned) j;
			if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
			Color[i].blue = (unsigned) j;
			}
		}
	}
if(ReadOne(FALSE,FALSE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
if(Mystrcmp(p_line,"STARTSTRING:") != 0) {
	sprintf(Message,"Incorrect end in ‘%s’ settings file. May be bad version?",
			filename);
	if(Beta) Alert1(Message);
	goto QUIT;
	}
ReadFile(wStartString,refnum);
ShowSelect(CENTRE,wStartString);
Dirty[wStartString] = FALSE;
goto QUIT;

ERR:
result = FAILED;
sprintf(Message,"Error reading ‘%s’ settings file…",filename);
Alert1(Message);

QUIT:
MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
if(FSClose(refnum) != noErr) {
	sprintf(Message,"Error closing ‘%s’ settings file…",filename);
	result = FAILED;
	}
if(result == OK) {
	SetName(iSettings,TRUE,manual && !startup);
	if(!startup) {
		TheVRefNum[iSettings] = spec.vRefNum;
		WindowParID[iSettings] = spec.parID;
		}
	else {
		RefNumbp2 = spec.vRefNum;
		ParIDbp2 = spec.parID;
		}
	}
else Dirty[iSettings] = FALSE;
if(!startup) HideWindow(Window[wMessage]);
SetCursor(&arrow);
LoadOn--;
if(!connectionok) return(ABORT);
return(result);
}


SaveDecisions(void)
{
int i,ishtml;
StandardFileReply reply;
short refnum;
long count;

ShowMessage(TRUE,wMessage,"Creating decision file…");
PascalLine[0] = 0;
if(NewFile(PascalLine,&reply)) {
	i = CreateFile(-1,-1,4,PascalLine,&reply,&refnum);
	if(i == ABORT) return(FAILED);
	if(i == OK) {
		SetCursor(&WatchCursor);
		if(!ComputeOn) WriteToFile(NO,MAC,"END",refnum);
		WriteHeader(-1,refnum,reply.sfFile);
		sprintf(LineBuff,"%ld",(long)CompileDate);
		WriteToFile(NO,MAC,LineBuff,refnum);
		sprintf(LineBuff,"%ld",(long)ProduceStackDepth);
		WriteToFile(NO,MAC,LineBuff,refnum);
		for(i=0; i < ProduceStackDepth; i++) {
			sprintf(LineBuff,"%ld\r%ld\r%ld",
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
	/*	Dirty[wInteraction] = FALSE; */
		return(OK);
		}
	else {
		MyPtoCstr(MAXNAME,PascalLine,LineBuff);
		sprintf(Message,"Error creating ‘%s’",LineBuff);
		Alert1(Message);
		}
	}
return(FAILED);
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

ShowMessage(TRUE,wMessage,"Locate decision file…");
p_line = p_completeline = NULL;
if(OldFile(-1,4,PascalLine,&spec)) {
	MyPtoCstr(255,PascalLine,LineBuff);
	if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
		pos = ZERO;
		CompleteDecisions = FALSE;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
		if(Mystrcmp(p_line,"END") == 0) {
			CompleteDecisions = TRUE;
			if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
			}
		if(CheckVersion(&iv,p_line,LineBuff) != OK) goto ERR;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
		if(ReadLong(refnum,&compiledate,&pos) == FAILED)
			goto ERR;
		if(!loadgrammar && (compiledate != CompileDate)) {
			if((r = Answer(
	"Grammar may have changed.\rLoad old version from decision file",'Y'))
														== OK) {
				if(ResetProject(FALSE) != OK) goto NOERR;
				loadgrammar = YES;
				}
			if(r == NO) Alert1
		("Non-matching grammars may yield unpredictable results. (Including bomb)");
			if(r == ABORT) goto NOERR;
			}
		if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
		ProduceStackDepth = k;
		if(ProduceStackDepth > MAXDERIV) {
			maxderiv = ProduceStackDepth;
			ReleaseProduceStackSpace();
			}
		else maxderiv = MAXDERIV;
		if(MakeComputeSpace(maxderiv) != OK) goto ERR;
		for(i=0; i < ProduceStackDepth; i++) {
			PleaseWait();
			if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
			(*p_MemGram)[i] = j;
			if(ReadInteger(refnum,&j,&pos) == FAILED) goto ERR;
			(*p_MemRul)[i] = j;
			if(ReadLong(refnum,&k,&pos) == FAILED) goto ERR;
			(*p_MemPos)[i] = k;
			}
		if(loadgrammar) {
			if((r=LoadGrammar(&spec,refnum)) != OK) goto ERR;
			MyPtoCstr(MAXNAME,spec.name,FileName[wGrammar]);
			SetName(wGrammar,TRUE,TRUE);
			if((r=LoadAlphabet(wGrammar,&spec)) != OK) goto ERR;
			if(CompileGrammar(TRUE) != OK) goto ERR;
			CompileDate = compiledate; /* compile time is the old one */
			}
		goto NOERR;
ERR:
		Alert1("Can't read decision file…");
NOERR:
		if(FSClose(refnum) == noErr) ;
		}
	else {
		TellError(34,io);
		}
	MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
	}
else {
	Alert1("Error reading decision file…");
	HideWindow(Window[wMessage]);
	return(FAILED);
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
result = FAILED;
p_line = p_completeline = NULL;
strcpy(Message,FileName[wInteraction]);
strcpy(line,Message);
type = gFileType[wInteraction];
if(anyfile) type = 0;
spec.vRefNum = TheVRefNum[wInteraction];
spec.parID = WindowParID[wInteraction];
pStrCopy((char*)c2pstr(Message),spec.name);
SetSelect(ZERO,GetTextLength(wInteraction),TEH[wInteraction]);
TextDelete(wInteraction);
if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
	if(CheckFileName(wInteraction,line,&spec,&refnum,gFileType[wInteraction],TRUE) != OK) {
		Interactive = FALSE;
		SetButtons(TRUE);
		UpdateDirty(TRUE,iSettings);
		return(FAILED);
		}
	}
sprintf(Message,"Loading %s…",FileName[wInteraction]);
ShowMessage(TRUE,wMessage,Message);
pos = 0L;
LoadOn++;

html = FALSE;
 
READMORE:
if(ReadOne(FALSE,html,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
if((*p_line)[0] == '<') {
	html = TRUE; goto READMORE;
	}
if(CheckVersion(&iv,p_completeline,FileName[wInteraction]) != OK) goto ERR;
ReadOne(FALSE,TRUE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos);
GetDateSaved(p_completeline,&(p_FileInfo[wInteraction]));
result = OK;
if(ReadInteger(refnum,&s,&pos) != FAILED) {
	oldversion = TRUE;
	DeriveFurtherKey = s;
	}
else {	/* This is a text version (new format) */
	oldversion = FALSE; pos = ZERO;
	SetFPos(refnum,fsFromStart,pos);
	if(ReadFile(wInteraction,refnum) == OK) {
		if(!WASTE) {
			ZeroScrap(); TEToScrap();
			}
		/* The following is only useful to erase date and version */
		GetHeader(wInteraction);
		goto QUIT;
		}
	else {
		result = FAILED; goto QUIT;
		}
	}

/* Continue to read the old format… */
MuteOnChan = MuteOffChan = -1;

if(ReadInteger(refnum,&s,&pos) != FAILED)
DeriveFurtherChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
ResetWeightKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
ResetWeightChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
PlayKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
PlayChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
RepeatKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
RepeatChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
EndRepeatKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
EndRepeatChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
EverKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
EverChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
QuitKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
QuitChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
UseEachSubKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
UseEachSubChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
SynchronizeStartKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
SynchronizeStartChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
SpeedCtrl = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
SpeedChan = s; else goto ERR;
if(ReadFloat(refnum,&r,&pos) != FAILED)
SpeedRange = r; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
SetTimeKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
SetTimeChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
StriatedKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
StriatedChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
NoConstraintKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
NoConstraintChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
SkipKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
SkipChan = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
AgainKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
AgainChan = s; else goto ERR;

if(iv > 3) {
	if(ReadInteger(refnum,&s,&pos) != FAILED)
	maxwait = s; else goto ERR;
	}
else maxwait = 8;
for(i=1; i <= maxwait; i++) {
	if((iv > 3) && (ReadInteger(refnum,&s,&pos) == FAILED))
			goto ERR;	/* type 0 = MIDi keyboard, 1 = … */
	if(iv > 3 && s != 0) {
		sprintf(Message, "Unknown synchro tag type #%ld in %s",(long)s,
			FileName[wInteraction]);
		Alert1(Message);
		goto ERR;
		}
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	WaitKey[i] = s;
	if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR;
	WaitChan[i] = s;
	}
if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR; /* Unused */
if(ReadInteger(refnum,&s,&pos) == FAILED) goto ERR; /* Unused */
if(ReadInteger(refnum,&s,&pos) != FAILED)
MinTclockKey = s; else goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED)
MaxTclockKey = s; else goto ERR;
if(MinTclockKey != -1 && MinTclockKey == MaxTclockKey) {
	Alert1("Lower and higher tempo-adjustment keys should be different");
	goto ERR;
	}
if(iv < 4) {
	if(ReadOne(FALSE,FALSE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) != FAILED) {
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
	if(ReadOne(FALSE,FALSE,TRUE,refnum,TRUE,&p_line,&p_completeline,&pos) != FAILED) {
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
	if(ReadUnsignedLong(refnum,&kk,&pos) != FAILED) MinPclock = (double) kk;
	else goto ERR;
	if(MinPclock < 1.) {
		Alert1("Err: MinPclock < 1 in LoadInteraction().  Incorrect ‘-in’ file");
		goto ERR;
		}
	if(ReadUnsignedLong(refnum,&kk,&pos) != FAILED) MaxPclock = kk;
	else goto ERR;
	if(MaxPclock < 1) {
		Alert1("Err: MaxPclock < 1 in LoadInteraction().  Incorrect ‘-in’ file");
		goto ERR;
		}
	if(ReadUnsignedLong(refnum,&kk,&pos) != FAILED) MinQclock = kk;
	else goto ERR;
	if(ReadUnsignedLong(refnum,&kk,&pos) != FAILED) MaxQclock = kk;
	else goto ERR;
	}
if(MinPclock != ZERO && Simplify((double)INT_MAX,MinQclock,MinPclock,&MinQclock,&MinPclock) != OK) goto ERR;
if(MaxPclock != ZERO && Simplify((double)INT_MAX,MaxQclock,MaxPclock,&MaxQclock,&MaxPclock) != OK) goto ERR;
if(ReadInteger(refnum,&s,&pos) != FAILED) TclockChan = s;
else goto ERR;

if(iv > 3) {
	if(ReadInteger(refnum,&s,&pos) != FAILED)
	maxctrl = s; else goto ERR;
	}
else maxctrl = 16;

for(i=1; i <= maxctrl; i++) {
	if(ReadInteger(refnum,&s,&pos) != FAILED)
	ParamControl[i] = s; else goto ERR;
	}
for(i=maxctrl+1; i < MAXPARAMCTRL; i++) ParamControl[i] = -1;
for(i=0; i < MAXPARAMCTRL; i++) ParamKey[i] = -1;
ParamControlChan = 1;
if(ReadInteger(refnum,&s,&pos) != FAILED)
ParamControlChan = s; else goto ERR;
for(i=1; i < MAXPARAMCTRL; i++) {
	ParamChan[i] = ParamControlChan;	/* For compatibility with old files */
	}
goto QUIT;

ERR:
result = FAILED;
Alert1("Error reading interactive code file…");
ForgetFileName(wInteraction); /* 1/3/97 */
Interactive = FALSE; SetButtons(TRUE);

QUIT:
MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
if(FSClose(refnum) != noErr) {
	if(Beta) Alert1("Error closing interactive code file…");
	}
HideWindow(Window[wMessage]);
if(result == OK) {
	LoadedIn = TRUE;
	if(oldversion) {
		CompiledIn = TRUE; PrintInteraction(wInteraction);
		}
	else {
		if(CompileInteraction() != OK) {
			LoadedIn = FALSE; result = FAILED;
			}
		}
	SetName(wInteraction,TRUE,TRUE);
	}
Dirty[wInteraction] = Created[wInteraction] = FALSE;
HideWindow(Window[wMessage]);
LoadOn--;
return(result);
}


SaveMIDIorchestra(void)
{
int rep,i,j,type,result;
short refnum;
long count;
Str255 fn;
StandardFileReply reply;

ShowMessage(TRUE,wMessage,"Saving MIDI orchestra file…");
if(FileName[wMIDIorchestra][0] == '\0') strcpy(Message,"-or.");
else strcpy(Message,FileName[wMIDIorchestra]);
pStrCopy((char*)c2pstr(Message),fn);
type = gFileType[wMIDIorchestra];

result = FAILED;
rep = Answer("Export as text file",'N');
if(rep == ABORT) goto OUT;
if(rep == OK) type = 1;
reply.sfFile.vRefNum = TheVRefNum[wMIDIorchestra];	/* Added 30/3/98 */
reply.sfFile.parID = WindowParID[wMIDIorchestra];
if(NewFile(fn,&reply)) {
	rep = CreateFile(wMIDIorchestra,-1,type,fn,&reply,&refnum);
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
		MyPtoCstr(MAXNAME,reply.sfFile.name,FileName[wMIDIorchestra]);
		TheVRefNum[wMIDIorchestra] = reply.sfFile.vRefNum;
		WindowParID[wMIDIorchestra] = reply.sfFile.parID;
		SetName(wMIDIorchestra,TRUE,TRUE);
		Dirty[wMIDIorchestra] = FALSE;
		result = OK;
		if(type == 1)
			Alert1("To open this file, click the ‘Load’ button with the ‘option’ key down");
		}
	else {
		MyPtoCstr(MAXNAME,fn,LineBuff);
		sprintf(Message,"Error creating ‘%s’",LineBuff);
		Alert1(Message);
		}
	}
OUT:
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


LoadOn++;
if(!(InBuiltDriverOn || Oms)) {
	Alert1("Can't load MIDI orchestra because neither OMS nor the in-built MIDI driver are active");
	return(FAILED);
	}
p_line = p_completeline = NULL;
SetField(MIDIprogramPtr,-1,fPatchName," ");
if(TestMIDIChannel > 0 && TestMIDIChannel <= MAXCHAN) {
	if(CurrentMIDIprogram[TestMIDIChannel] > 0) {
		GetDialogItem((DialogPtr)MIDIprogramPtr,(short)CurrentMIDIprogram[TestMIDIChannel],
			&itemtype,(Handle*)&itemhandle,&therect);
		HiliteControl((ControlHandle)itemhandle,0);
		}
	GetDialogItem((DialogPtr)SixteenPtr,(short)button1 + TestMIDIChannel - 1,&itemtype,
		(Handle*)&itemhandle,&therect);
	HiliteControl((ControlHandle) itemhandle,0);
	}
	
if(!manual) goto READIT;

ShowMessage(TRUE,wMessage,"Locate MIDI orchestra file…");
type = gFileType[wMIDIorchestra];
if(Option && Answer("Import any type of file",'Y') == OK) type = 0;
if(OldFile(-1,type,PascalLine,&spec)) {
	MyPtoCstr(255,PascalLine,FileName[wMIDIorchestra]);
	if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
	
READIT:
		sprintf(Message,"Loading ‘%s’ orchestra file…",FileName[wMIDIorchestra]);
		ShowMessage(TRUE,wMessage,Message);
		pos = ZERO;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
		if(CheckVersion(&iv,p_line,FileName[wMIDIorchestra]) != OK) goto NOERR;
		if(ReadOne(FALSE,FALSE,FALSE,refnum,TRUE,&p_line,&p_completeline,&pos) == FAILED) goto ERR;
		SetCursor(&WatchCursor);	
		if(ReadLong(refnum,&imax,&pos) == FAILED) goto ERR;
		if(imax < 16 || imax > MAXCHAN) {
			if(Beta) Alert1("Err. LoadMIDIorchestra(). imax < 16 || imax > MAXCHAN");
			imax = MAXCHAN;
			}
		for(i=1; i <= 16; i++) {
			SetField(NULL,wMIDIorchestra,i,"[unknown instrument]");
			CurrentMIDIprogram[i] = 0;
			}
		for(i=1; i <= imax && i <= 16; i++) {
			if(ReadInteger(refnum,&w,&pos) == FAILED) goto ERR;
			CurrentMIDIprogram[i] = w;
			ChangedMIDIprogram = TRUE;
			if(w > 0 && w <= 128) {
				e.time = Tcurr;
				e.type = TWO_BYTE_EVENT;
				e.status = ProgramChange + i - 1;
				e.data2 = w - 1;
				rs = 0;
				if((InBuiltDriverOn || Oms) && !InitOn)
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
				GetDialogItem((DialogPtr)MIDIprogramPtr,(short)CurrentMIDIprogram[TestMIDIChannel],
					&itemtype,(Handle*)&itemhandle,&therect);
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
			GetDialogItem((DialogPtr)SixteenPtr,(short)button1 + TestMIDIChannel - 1,&itemtype,
				(Handle*)&itemhandle,&therect);
			HiliteControl((ControlHandle) itemhandle,kControlButtonPart);
			}
		goto NOERR;
ERR:
		Alert1("Can't read MIDI orchestra file…");
NOERR:
		if(FSClose(refnum) == noErr) ;
		if(manual) {
			TheVRefNum[wMIDIorchestra] = spec.vRefNum;
			WindowParID[wMIDIorchestra] = spec.parID;
			}
		Dirty[wMIDIorchestra] = FALSE;
		NewOrchestra = TRUE;
		}
	else TellError(35,io);
	MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
	}
else {
	HideWindow(Window[wMessage]);
	LoadOn--;
	return(FAILED);
	}
LoadOn--;
HideWindow(Window[wMessage]);
return(OK);
}