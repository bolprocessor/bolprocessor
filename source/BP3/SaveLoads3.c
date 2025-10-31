/* SaveLoads3.c (BP3 version CVS) */ 

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

#ifndef _H_BP3
#include "-BP3.h"
#endif

#include "-BP3decl.h"
// #include "ConsoleMessages.h"

// FIXME: name should be const, but Match needs to take const char * instead of char**
int FindMatchingFileNamePrefix(/*const*/ char* name)
{
	int	w;
	char* pre;
	
	for (w = 0; w < WMAX; w++) {
		if(FilePrefix[w][0] == '\0') continue;
		// prefixes are case sensitive
		pre = FilePrefix[w];
		if(Match(TRUE, &name, &pre, strlen(pre))) return w;
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
	if(nameExt == NULL)  return wUnknown;
	
	len = strlen(nameExt);
	for (w = 0; w < WMAX; w++) {
		if(FileExtension[w][0] == '\0') continue;
		ext = FileExtension[w];
		// extensions must be the same length
		if(len != strlen(ext))  continue;
		// extensions are case insensitive
		if(Match(FALSE, &nameExt, &ext, strlen(ext))) return w;
	}
	
	return wUnknown;
}

// FIXME: name and ext should be const, but Match needs to take const char * instead of char**
int MatchFileNameExtension(/*const*/ char* name, /*const*/ char* ext)
{
	char* nameExt;
	
	// find the last '.' in name, if any
	nameExt = strrchr(name, '.');
	if(nameExt == NULL)  return FALSE;
	
	// extensions must be the same length
	if(strlen(nameExt) != strlen(ext))  return FALSE;
	
	// extensions are case insensitive
	return Match(FALSE, &nameExt, &ext, strlen(ext));
}

// FIXME: name should be const, but functions above need to be fixed first
int IdentifyBPFileTypeByName(/*const*/ char* name)
{
	int	wfindex;	// window/file index (result)

	// try the filename's prefix, if any
	wfindex = FindMatchingFileNamePrefix(name);
	if(wfindex != wUnknown)  return wfindex;
	
	// next try the filename's extension, if any
	wfindex = FindMatchingFileNameExtension(name);
	if(wfindex != wUnknown)  return wfindex;
	
	// also try the other extensions that we recognize
	if(MatchFileNameExtension(name, ".mid") ||
	    MatchFileNameExtension(name, ".midi"))
		return iMIDIfile;
	if(MatchFileNameExtension(name, ".trace"))  return wTrace;
	if(MatchFileNameExtension(name, ".txt") ||
	    MatchFileNameExtension(name, ".sco") ||
	    MatchFileNameExtension(name, ".htm") ||
	    MatchFileNameExtension(name, ".html"))
		return wScrap;

	return wUnknown;
}


void strip_trailing_spaces(char *str) {
    if(str == NULL) return;
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        len--;
		}
		str[len] = '\0';
	}


void remove_double_slash_prefix(char *str) {
    if(strncmp(str, "//", 2) == 0) {
        // Shift everything to the left over the "//"
        memmove(str, str + 2, strlen(str) - 1);
		}
	}


int ReadOne(int bindlines,int careforhtml,int nocomment,FILE* fin,int strip,char ***pp_line,
	char ***pp_completeline,long *p_pos) {
// Read a line in the file and save it to text handle 'pp_completeline'
// If the line starts with "//", discard it
// bindlines is now irrelevant
	int i, html;
	char *buffer;
	long size, count;
	char line[3000];
	buffer = NULL;

	MyDisposeHandle((Handle*)pp_line);
	MyDisposeHandle((Handle*)pp_completeline);
	size = 3000;
	if((*pp_line = (char**) GiveSpace((Size)size * sizeof(char))) == NULL) return(ABORT);
	if((*pp_completeline = (char**) GiveSpace((Size)size * sizeof(char))) == NULL) return(ABORT);

	// BPPrintMessage(0,odError,"pos1 = %ld\n",*p_pos);
	if(fseek(fin, *p_pos, SEEK_SET) != 0) {
		BPPrintMessage(0,odError, "Error reading from file\n");
	//	perror("fseek failed");
		MyDisposeHandle((Handle*)pp_line);
		MyDisposeHandle((Handle*)pp_completeline);
		return MISSED;
		}
		// *p_pos = ftell(fin);
		// BPPrintMessage(0,odError,"pos2 = %ld\n",*p_pos);
	if(fgets(line, sizeof(line),fin) != NULL) {
		if(ferror(fin)) {
			BPPrintMessage(0,odError, "Error reading from file.\n");
			clearerr(fin);  // Clear the error indicator for the stream
			MyDisposeHandle((Handle*)pp_line);
			MyDisposeHandle((Handle*)pp_completeline);
			return MISSED;
			}
		remove_final_linefeed(line);
		*p_pos = ftell(fin);
		// BPPrintMessage(0,odError,"pos3 = %ld\n",*p_pos);
		size_t lineSize = utf8_strsize(line);
		char* newBuffer = realloc(buffer,lineSize + 2); // +1 for '\n' and +1 for '\0'
		if(newBuffer == NULL) {
			BPPrintMessage(0,odError, "=> Err. ReadOne(). newBuffer == NULL\n");
			return MISSED;
			}
		buffer =  newBuffer;
		memcpy(buffer, line, lineSize);
		buffer[lineSize] = '\0';
		if(lineSize == 0) {
			char *emptyStr = strdup(""); // Handle empty file case
			if(emptyStr != NULL) {
				free(buffer);
				buffer = emptyStr;
				}
			else {
				BPPrintMessage(0,odError, "Memory allocation failed for empty string\n");
				free(buffer);
				return MISSED;
				}
			}
		remove_carriage_returns(line);
		// BPPrintMessage(0,odInfo,"thisline = %s\n",buffer);
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
		BPPrintMessage(0,odError, "=> Error while writing to file.\n");
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
	BPPrintMessage(0,odError,"=> Err. WriteToFile(). refnum == -1 line = \"%s\"\n",line);
	return(MISSED);
	}
p_line = NULL;
if((res = MystrcpyStringToHandle(&p_line, line)) != OK) return res;

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
	BPPrintMessage(0,(int)refnum, "%s\n", *p_line);
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
	BPPrintMessage(0,odError,"=> Err. NoReturnWriteToFile(). refnum == -1");
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
	BPPrintMessage(0,(int)refnum, line);
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
	if(WindowFullAlertLevel[w] < 1) {	// this test cannot be in the previous 'if'
		if(FileName[w][0] != '\0')
			my_sprintf(Message,"Window '%s' is almost full",FileName[w]);
		else
			my_sprintf(Message,"Window '%s' is almost full",WindowName[w]);
		BPPrintMessage(0,odError,"%s",Message);
		WindowFullAlertLevel[w] = 1;	// 1 means we've warned about being almost full
		}
	}
else if(n < 0) {
	my_sprintf(Message, "Text has overflowed the '%s' window! Save your work and quit...", WindowName[w]);
	BPPrintMessage(0,odError,"%s",Message);
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
if(io != noErr) return (io);

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

int CleanLF(char** p_buffer,long* p_count,int* p_dos) {
// Remove line feeds from buffer and transcode high ASCII so that
// DOS files may be read
// This should be rewritten using a simple preg_replace()!
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
			//			BPPrintMessage(0,odInfo,"NOT DOS\n");
						return(OK);	/* Not a DOS file */
						}
					}
				}
			}
		}
	if(!*p_dos) {
	//	BPPrintMessage(0,odInfo,"NOT DOS\n");
		return(OK);
		}

	for(i=j=0; ; i++) {
		if(i >= *p_count) break;
		while((c=(*p_buffer)[i+j]) == '\n' && (i == 0 || (*p_buffer)[i+j-1] == '\r')) {
		//	BPPrintMessage(0,odInfo,"i = %d, j = %d, count = %d\n",i,j,*p_count);
			j++; (*p_count)--;
			}
		// DOStoMac(&c); Fixed by BB 2022-02-18
		(*p_buffer)[i] = c;
		}
	return(OK);
	}

int GetThisVersion(int w) {
	int i,j,diff,r,fileversion;
	long pos,posho,posmax;
	char c,*p,*q,**p_line,version[VERSIONLENGTH];

	pos = ZERO;
	p_line = NULL;
	if(w < 0 || w >= WMAX || !Editable[w]) {
		BPPrintMessage(0,odError,"=> Err. GetThisVersion(). Incorrect window index");
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
		if(!ScriptExecOn) BPPrintMessage(0,odError,"%s",Message);
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


int CheckVersion(int *p_iv, char** p_line, const char name[]) {
	int diff,rep,iv;
	char version[VERSIONLENGTH];

	(*p_iv) = 0;
	diff = 1;
	if(p_line == NULL || strlen((*p_line)) < 5) {
		BPPrintMessage(0,odError,"=> Error loading this file: version cannot be found. Did you save it?\n");
		return(MISSED);
		}
	FindVersion(p_line,version);
	for(iv=0; iv < MAXVERSION; iv++)
		if((diff = strcmp(version,VersionName[iv])) == 0) break;
	if(iv > Version && name[0] != '\0') {
		BPPrintMessage(0,odError,"=> File '%s' was created with a version of BP3 more recent\n",name);
		rep = NO;
		if(rep != YES) goto ERR;
		iv = Version;
		}
	(*p_iv) = iv;
	return(OK);

	ERR:
	return(MISSED);
	}


int GetFileDate(int w,char ***pp_result) {
	int i,diff,gap,result;
	long pos,posmax;
	char *p,*q,**p_line;

	if(w < 0 || w >= WMAX || !Editable[w]) {
		BPPrintMessage(0,odError,"=> Err. GetFileDate(). w < 0 || w >= WMAX || !Editable[w]");
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


int GetDateSaved(char **p_line,char ***pp_result) {
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

int FindVersion(char **p_line,char* version) {
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