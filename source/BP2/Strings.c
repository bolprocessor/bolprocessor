/* Strings.c  (BP2 version CVS) */
 
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


void CopyPString(const Str255 src,Str255 dest)
/* copies a pascal string from src to dest */
{
int i,len;

i = 0;
len = dest[i++] = *src++;
while (--len >= 0) dest[i++] = *src++;
}


/* Copies a pascal string to a C string, truncating if necessary.
   WARNING: this function also converts whitespace characters! */
MyPtoCstr(int max,Str255 s,char *t)
{
int i,len;
char c;

len = s[0];
if(len > max) {
	if(Beta) Alert1("Err. MyPtoCstr(). len > max");
	len = max;
	}
for(i=0; i < len; i++) {
	c = s[i+1];
	if(MySpace(c)) c = ' ';
	t[i] = c;
	}
t[len] = '\0';
return(OK);
}


StringPtr in_place_c2pstr(char* s)
{
	unsigned int len = 0;
	char  cur, next;
	char* p = s;
	
	cur = next = *s;
	while (cur != '\0')	{
		next = *(++p);
		*p = cur;
		cur = next;
		++len;
	}
	s[0] = (unsigned char)((len>255) ? 255 : len);
	
	return (StringPtr)(s);
}


Pstrcmp(Str255 s1,Str255 s2)
{
int i,n1,n2;
n1 = s1[0]; n2 = s2[0];
if(n1 != n2) return(-1);
for(i=1; i <= n1; i++) {
	if(ByteToInt(s1[i]) != ByteToInt(s2[i])) return(-1);
	}
return(0);
}


MystrcpyStringToTable(char ****p_t,int j,char *s)
{
/* register */ int i;
int imax;

if(p_t == NULL) {
	if(Beta) Alert1("Err. MystrcpyStringToTable(). p_t = NULL");
	return(FAILED);
	}
imax = (MyGetHandleSize((*p_t)[j]) / sizeof(char)) - 1;
i = -1;
do {
	i++;
	(*((*p_t)[j]))[i] = s[i];
	}
while(s[i] != '\0' && i < imax);
if(s[i] != '\0') {
	(*((*p_t)[j]))[i] = '\0';
	if(Beta && s[i] != '\0') Alert1("Err. MystrcpyStringToTable(). s[i] incomplete");
	}
return(OK);
}


MystrcpyTableToString(int imax,char *s,char ****p_t,int j)
{
/* register */ int i;

if(p_t == NULL) {
	if(Beta) Alert1("Err. MystrcpyTableToString(). p_t = NULL");
	return(FAILED);
	}
i = -1;
do	{
	i++; s[i] = (*((*p_t)[j]))[i];
	}
while(s[i] != '\0' && i < imax-1);
s[i] = '\0';	/* The content might get truncated */
return(OK);
}


MystrcpyStringToHandle(char ***pp_t,char *s)
{
long i,imt,ims;

ims = (long) strlen(s) + 1L;
if(*pp_t == NULL) {
	if((*pp_t = (char**) GiveSpace((Size)ims * sizeof(char))) == NULL)
		return(ABORT);
	}
else {
	imt = (long) MyGetHandleSize((Handle)*pp_t) / sizeof(char);
	if(imt < ims) {
		if(MySetHandleSize((Handle*)pp_t,(Size)ims * sizeof(char)) != OK) return(ABORT);
		}
	}
i = ZERO;
do (**pp_t)[i] = s[i];
while(s[i++] != '\0');
return(OK);
}


MystrcpyHandleToString(int imax,int offset,char *s,char **p_t)
{
long i;
char c;

if(p_t == NULL) {
	s[0] = '\0';
	return(OK);
	}
i = 0;
while((c = (*p_t)[offset++]) != '\0' && i < imax)
	s[i++] = c;
if (i >= imax) i = imax - 1;
s[i] = '\0';	/* The content might get truncated */

return(OK);
}


MystrcpyHandleToHandle(int offset,char ***pp_s,char **p_t)
// Copy t to s
{
long i;
Size ims,imt;

if(*pp_s != NULL) ims = (long) MyGetHandleSize((Handle)*pp_s);
else {
	ims = ZERO;
	if(Beta) Alert1("Err. MystrcpyHandleToHandle(). ims = ZERO");
	}
imt = ZERO;
if(p_t == NULL) {
	if(Beta) Alert1("Err. MystrcpyHandleToHandle(). p_t = NULL");
	if(*pp_s != NULL && ims > ZERO) (**pp_s)[0] = '\0';
	// FIXME ? what if *pp_s == NULL ? Will caller expect a valid handle?
	goto OUT;
	}
else {
	while((*p_t)[offset+imt] != '\0') imt++;	/* fixed 11/3/99 */
	}
if(imt == ZERO) {
	if(*pp_s != NULL && ims > ZERO) (**pp_s)[0] = '\0';
	// FIXME ? what if *pp_s == NULL ? Will caller expect a valid handle?
	goto OUT;
	}

imt = (imt + 1L) * sizeof(char);
if(imt > ims) {
	if(MySetHandleSize((Handle*)pp_s,imt) != OK) return(ABORT);
	}
i = ZERO;
do {
	(**pp_s)[i++] = (*p_t)[offset];
	}
while((*p_t)[offset++] != '\0');

OUT:
return(OK);
}


GetTextHandle(char ***pp_h,int w)
{
long i,length;

if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. GetTextHandle(). Invalid window index");
	return(FAILED);
	}
length = GetTextLength(w);
if(MySetHandleSize((Handle*)pp_h,(Size)(1L + length) * sizeof(char)) != OK)
	return(ABORT);
	
/* #if WASTE
h = WEGetText(TEH[w]);
(*h)[length] = '\0';
MystrcpyHandleToHandle(0,pp_h,h);
#else */
for(i= 0; i < length; i++) {
	(**pp_h)[i] = GetTextChar(w,i);
	}
(**pp_h)[i] = '\0';
// #endif

StripHandle(*pp_h);
return(OK);
}


Mystrcmp(char **p_t,char *s)
{
long i;

i = ZERO;
if(p_t == NULL) {
	if(Beta) Alert1("Err. Mystrcmp(). p_t = NULL");
	return(1);
	}
do 
	if((*p_t)[i] != s[i]) return(1);
while(s[i++] != '\0');
return(0);
}


MyHandlecmp(char **p_t,char **p_s)
{
long i;

i = ZERO;
if(p_t == NULL || p_s == NULL) {
	if(Beta) Alert1("Err. MyHandlecmp(). p_t = NULL || p_s = NULL");
	return(0);
	}
do 
	if((*p_t)[i] != (*p_s)[i]) return(1);
while((*p_s)[i++] != '\0');
return(0);
}


MyHandleLen(char **p_t)
{
long i,im;

i = ZERO;
if(p_t == NULL) {
	if(Beta) Alert1("Err. MyHandleLen(). p_t = NULL");
	return(0);
	}
im = MyGetHandleSize((Handle)p_t);
im = (im / sizeof(char)) - 1L;
while((*p_t)[i] != '\0') {
	i++;
	if(i >= im) break;
	}
if((*p_t)[i] != '\0') {
	if(Beta) Alert1("Err. MyHandleLen(). (*p_t)[i] != nullchar");
	}
return(i);
}


Strip(char *word)
// Eliminate leading and trailing blanks
{
int i,j;

if(word[0] == '\0') return(OK);
j = 0; while(isspace(word[j])) j++;
if(j > 0) {
	for(i=j; i <= strlen(word); i++) {
		word[i-j] = word[i];
		}
	}
i = strlen(word) - 1;
while(isspace(word[i])) {
	word[i] = '\0'; i--;
	}
return(OK);
}


StripHandle(char **p_line)
// Eliminate leading and trailing blanks
{
int i,im,j;

if(p_line == NULL) {
	if(Beta) Alert1("Err. StripHandle(). p_line = NULL");
	return(OK);
	}
if((*p_line)[0] == '\0') return(OK);
j = 0; while(isspace((*p_line)[j])) j++;
if(j > 0) {
	if((im=MyHandleLen(p_line)) == 0) {
		if(Beta) Alert1("Err. StripHandle(). MyHandleLen(p_line) == 0");
		return(ABORT);
		}
	for(i=j; i <= im; i++) {
		(*p_line)[i-j] = (*p_line)[i];
		}
	}
i = MyHandleLen(p_line) - 1;
while(isspace((*p_line)[i])) {
	(*p_line)[i] = '\0'; i--;
	}
return(OK);
}


/* FIXME ? This is not "portable". Should we also convert diacriticals?
   If not, then we should just use toupper() instead. */
UpperCase(char c)
{
if(!isalpha(c)) return(c);
if(c > 'Z') return(c - ('a' - 'A'));
else return(c);
}


UpperCaseString(char* line)
{
int i;

if(line[0] == '\0') return(OK);
for(i=0; i < strlen(line); i++) line[i] = UpperCase(line[i]);
return(OK);
}


NextChar(char **pp)
{
while(MySpace(**pp) && **pp != '\r' && **pp != '\n') (*pp)++;
return(**pp);
}


CheckEnd(char c)
{
if(c == '\0') return(TRUE);
return(FALSE);
}


char *GetEnd(char** pp)
{
while((**pp) != '[' && (**pp) != '\0' && (**pp) != '\n') {
/*	printf("%c",**pp); */
	(*pp)++;
	}
return(*pp);
}


FindPattern(char **p_thetarget,char thepattern[],int *p_end)
{
char **pp_t,*p_t,*s;
int i,l;

l = strlen(thepattern);
pp_t = &p_t;
*pp_t = thepattern;
for(i=0; ; i++) {
	if((*p_thetarget)[i] == '\0') break;
	s = &((*p_thetarget)[i]);
	if(Match(TRUE,&s,pp_t,l)) {
		*p_end = i + l;
		return(i);
		}
	}
return(-1);
}


Match(int casesensitive,char** p_s,char** p_t,int length)
// Is (*p_t) a prefix of (*p_s) ?
// Beware that *p_s is no longer modified since 22/6/95
{
/* register */ int i;
char c,d;

if(p_s == NULL || p_t == NULL) {
	if(Beta) Alert1("Err. Match(). p_s == NULL || p_t == NULL");
	return(NO);
	}
/* if(MyHandleLen(p_s) < length) return(NO); */
i = 0;
while((((c=(*p_s)[i]) == (d=((*p_t)[i]))) || (!casesensitive && ((UpperCase(c) == d)
		|| (c == UpperCase(d))))) && (c != '\0') && (i < length)) i++;
if(((*p_t)[i]) == '\0' && i == length) return(YES);
return(NO);
}


WriteFloatToLine(char *line,double x)
{
if(x >= Infpos) sprintf(line,"%ld",(long)Infpos);
else {
	if(x == ((long) x)) sprintf(line,"%ld",(long)x);
	else sprintf(line,"%.2f",x);
	}
return(OK);
}


#if BP_CARBON_GUI
TooLongFileName(char* line,DialogPtr ptr,int w,int field)
{
DialogPtr thedialog;

if(ptr != NULL) {
	thedialog = ptr; w = -1;
	}
else {
	if(w < 0 || w >= WMAX || !HasFields[w]) {
		if(Beta) Alert1("Err. TooLongFileName(). Incorrect index");
		return(FAILED);
		}
	thedialog = gpDialogs[w];
	}
// Strip(line); // don't strip filenames - akozar
if(strlen(line) > MAXNAME) {
	line[MAXNAME-1] = 'É';
	line[MAXNAME] = '\0';
	sprintf(Message,"File name is too long. Truncating to '%s'",line);
	Alert1(Message);
	BPActivateWindow(QUICK,w);
	SetField(thedialog,w,field,line);
	SelectField(thedialog,w,field,TRUE);
	return(TRUE);
	}
return(FALSE);
}
#endif /* BP_CARBON_GUI */
