/* Interface2.c  (BP3) */
 
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


#ifndef _H_BP3 
#include "-BP3.h"
#endif

#include "-BP3decl.h"

int check_no_copy = 0;

int SelectSomething(int w)
/* Select line on which cursor is flashing */
{
long startpos,endpos,length;
char c;
int j;

if(w < 0 || w >= WMAX || !Editable[w]) return(MISSED);
if(w != wStartString) return(OK);
TextGetSelection(&startpos,&endpos,TEH[w]);
length = GetTextLength(w);
if(startpos > length - 2) startpos = length - 2;
if(endpos > length - 1) endpos = length - 1;
for(j=startpos; j >= 0; j--) {
	c = GetTextChar(w,j);
	if(c == '\r') break;
	}
startpos = j + 1;
for(j=endpos; j < length; j++) {
	c = GetTextChar(w,j);
	if(c == '\r') break;
	}
endpos = j;
while(MySpace(GetTextChar(w,startpos)) && startpos < endpos) startpos++;
if(endpos <= startpos) return(MISSED);
SetSelect((long)startpos,(long)endpos,TEH[w]);
return(OK);
}


char Filter(char c)
{
return(c);
}


int ReadLine(int nocomment,int w,long *p_i,long im,char ***pp_line,int* p_gap)
/* Read unlimited line in TExt buffer */
{
int j,k,jm;
char c,oldc,**ptr;
long origin,end,length;

*p_gap = 0;
MyDisposeHandle((Handle*)pp_line);
if(w < 0 || w >= WMAX || !Editable[w]) {
	BPPrintMessage(0,odError,"=> Err. ReadLine(YES,). ");
	return(ABORT);
	}
if(*p_i >= im) goto BAD;
origin = *p_i;

/* Suppress blanks in beginning */
if(nocomment) {
	while(MySpace(c=GetTextChar(w,origin)) || c == '\n') {
		origin++; (*p_i)++; (*p_gap)++;
		}
	}
end = origin; oldc = '\0';
while(TRUE) {
	c = GetTextChar(w,end);
	if(nocomment && c == '*' && oldc == '/') {
		/* Skip C-type remark */
		oldc = '\0'; end++;
		while(TRUE) {
			c = GetTextChar(w,end);
			if(end >= im) {
				c = '\r';
				break;
				}
			if(c == '/' && oldc == '*') {
				end++;
				c = GetTextChar(w,end);
				break;
				}
			oldc = c;
			end++;
			}
		}
	if((c == '\r' || c == '\n') /* && oldc != '�' */) break;
	if(c == '\0' || end >= im) break;
	oldc = c;
NEXTCHAR:
	end++;
	}
if(nocomment && GetTextChar(w,origin) == '/'
				&& GetTextChar(w,origin+1) == '/') {
	/* Ignore lines starting with // */
	if((ptr = (char**) GiveSpace((Size)(2 * sizeof(char)))) == NULL) return(ABORT);
	*pp_line = ptr; (**pp_line)[0] = '\0';
	*p_i = end + 1; return(OK);
	}
if(origin >= end) {
	if((ptr = (char**) GiveSpace((Size)(2 * sizeof(char)))) == NULL) return(ABORT);
	*pp_line = ptr; (**pp_line)[0] = '\0';
	(*p_i)++; return(OK);
	}
length = end - origin + 4L;
if((ptr = (char**) GiveSpace((Size)(length * sizeof(char)))) == NULL) return(ABORT);
*pp_line = ptr;
if(ReadToBuff(nocomment,FALSE,w,&origin,end,pp_line) != OK) goto BAD;
*p_i = origin;

/* Suppress trailing blanks */
jm = MyHandleLen(*pp_line) - 1;
for(j=jm; j > 0; j--) {
	if(MySpace((**pp_line)[j])) (**pp_line)[j] = '\0';
	else break;
	}
return(OK);

BAD:
MyDisposeHandle((Handle*)pp_line);
return(MISSED);
}


int ReadLine1(int check,int w,long *p_i,long im,char *line,int size)
/* Read line in TExt buffer.  Old version: length is limited to 'size'. */
{
int j,k,l;
char c,oldc;

if(w < 0 || w >= WMAX || !Editable[w]) {
	BPPrintMessage(0,odError,"=> Err. ReadLine1(). ");
	return(OK);
	}
if(*p_i >= im) return(MISSED);
oldc = '\0'; k = 0;
for(j=*p_i; j < im; j++) {
	c = GetTextChar(w,j);
	if(c == '\n') continue;
	if(oldc == '/' && c == '*') {
		/* Skip C-type remark */
		oldc = '\0'; j++; k--;
		while(TRUE) {
			c = GetTextChar(w,j);
			if(j >= im) {
				c = '\r';
				break;
				}
			if(c == '/' && oldc == '*') {
				j++;
				c = GetTextChar(w,j);
				break;
				}
			oldc = c;
			j++;
			}
		}
	if((c == '\n') || (c == '\r') || (c == '\0')) break;
	c = Filter(c);
	line[k++] = c;
	if(k >= (size - 2)) {
		line[k] = '\0';
		*p_i = ++j;
		if(check) {
			my_sprintf(Message,"\nSelection too long, truncated: %s...\n",line);
			if(!ScriptExecOn) BPPrintMessage(0,odError,"%s",Message);
			else PrintBehindln(wTrace,Message);
			return(MISSED);
			}
		else return(OK);
		}
	oldc = c;
	}
line[k] = '\0';
*p_i = ++j;

CUT:
l = strlen(line);
if(l > 0 && MySpace(c=line[l-1])) {
	line[l-1] = '\0';
	goto CUT;
	}
return(OK);
}


int IsEmpty(int w)
{
int j,len,gap,rep;
char c,*q,**p_line,line[MAXLIN];
long i,pos,posmax;

if(w >= 0 && w < WMAX && Editable[w]) {
	posmax = GetTextLength(w);
	if(w == wPrototype7) {
		for(i=0; i < posmax; i++) {
			if(!isspace(GetTextChar(w,i))) return(FALSE);
			}
		return(TRUE);
		}
	
	p_line = NULL;
	pos = ZERO; rep = TRUE;
	
	if(w == wScript || w == wGlossary || w == wInteraction) {
		MystrcpyHandleToString(MAXLIN,0,line,p_ScriptLabelPart(110,0));
		/* line is "BP3 script" */
		}
	if(w == wGrammar) strcpy(line,"COMMENT:");
	len = strlen(line);
	while(ReadLine(YES,w,&pos,posmax,&p_line,&gap) == OK) {
	//	BPPrintMessage(0,odInfo,"line = %s\n",*p_line);
		if((*p_line)[0] == '\0' || (*p_line)[0] == '\r' || (*p_line)[0] == '\n') continue;
		for(j=0; j < WMAX; j++) {
			if(FilePrefix[j][0] == '\0') continue;
			q = &(FilePrefix[j][0]);
			if(Match(TRUE,p_line,&q,4)) goto NEXTLINE;
			}
		if(w == wScript || w == wGlossary || w == wInteraction) {
			/* Discard line */
			q = line;
			if(Match(FALSE,p_line,&q,len)) goto NEXTLINE;
			}
		if(w == wGrammar) {
			q = line;
			if(Match(FALSE,p_line,&q,len)) break;
			}
		rep = FALSE;
		break;
NEXTLINE: ;
		}
	MyDisposeHandle((Handle*)&p_line);
	return(rep);
	}
else return(FALSE);
}

int GetInitialRemark(char** p_line,char* remark)
{
int i,j;
char c;

i = j = 0; remark[j] = '\0';
while((c=(*p_line)[i++]) != '[') {
	if(c == '\0') return(OK);
	}
while((c=(*p_line)[i++]) != ']') {
	remark[j++] = c;
	if(j >= MAXLIN) break;
	if(c == '\0') return(OK);
	}
remark[j] = '\0';
return(OK);
}


int SelectBehind(long pos1,long pos2,TextHandle teh) {
/* Doesn't force selection to scroll */
	long maxoffset;
	maxoffset = GetTextHandleLength(teh);
	if(pos1 < ZERO) {
		BPPrintMessage(0,odError,"=> Err. SelectBehind(). pos1 < ZERO");
		pos1 = ZERO;
    	}
	else if(pos1 > maxoffset) {
		BPPrintMessage(0,odError,"=> Err. SelectBehind(). pos1 > maxoffset");
		pos1 = maxoffset;
	    }
	if(pos2 < ZERO) {
		BPPrintMessage(0,odError,"=> Err. SelectBehind(). pos2 < ZERO");
		pos2 = ZERO;
	    }
	else if(pos2 > maxoffset) {
		BPPrintMessage(0,odError,"=> Err. SelectBehind(). pos2 > maxoffset");
		pos2 = maxoffset;
	    }
	(*teh)->selStart = pos1; (*teh)->selEnd = pos2;
    return(OK);
    }


int TextDeleteBehind(int w)
{
/* Deactivate(TEH[w]);
TextDelete(w);  */
return(OK);
}

int FindGoodIndex(int wind)
{
if(wind < 0 || wind >= WMAX) {
	BPPrintMessage(0,odError,"=> Err. FindGoodIndex(). Incorrect index");
	return(LastEditWindow);
	}
switch(wind) {
	case wPrototype1:
	case wPrototype2:
	case wPrototype3:
	case wPrototype4:
	case wPrototype5:
	case wPrototype6:
	case wPrototype8:
		return(iObjects);
		break;
	case wCsoundTables:
		return(wCsoundResources);
		break;
	}
return(wind);
}

int TellError(int thecase,OSErr oserr)
{
char line[MAXLIN];
int r;

r = OK;
if(oserr != noErr) {
	if(!InitOn) {
		my_sprintf(Message,"\nMac error %ld (case %ld)",(long)oserr,(long)thecase);
		if(TraceRefnum > -1) WriteToFile(NO,MAC,Message,TraceRefnum);
		if(TempRefnum > -1) WriteToFile(NO,MAC,Message,TempRefnum);
		}
	r = ABORT;
	return(r);
	}
return(r);
}

int CompileCheck(void) {
	t_gram* p_gram;
	int r;
	r = OK;
	if(CheckEmergency() != OK) return(ABORT);
	if(check_no_copy) p_gram = &Gram;
	else p_gram = &Gram_compile;
	if(!CompiledGr && (AddBolsInGrammar() > BolsInGrammar)) CompiledAl = FALSE;
	if(!CompiledGr || !CompiledAl) {
		if((r=CompileGrammar(FirstGrammar,p_gram)) != OK) {
			if(r == MISSED && CompiledGr && !CompiledAl) r = CompileAlphabet();
			if(CompiledGr && CompiledAl) r = OK;
			if(r != OK) {
				if(FirstGrammar) BPPrintMessage(0,odError,"=> Problem compiling grammar and/or alphabet\n");
				return(r);
				}
			}
		if((Varweight = ResetRuleWeights(p_gram,0)) == ABORT) {
	//	if(FirstGrammar && ResetWeights && (Varweight = ResetRuleWeights(0)) == ABORT) {
			Print(wTrace,"Can't fix bug in grammar code. Unexpected error\n");
			return(MISSED);
			}
		}
	if(ObjectMode && ((r=CompileCsoundObjects()) != OK)) return(r);
	if(!CompiledPt) if((r=CompilePatterns()) != OK) return(r);
	if(r == OK) {
	//	BPPrintMessage(1,odInfo,"FirstGrammar = %d\n",FirstGrammar);
		if(!check_no_copy) CopyGramcompileToGram(FirstGrammar);
		}
	return(r);
	}

