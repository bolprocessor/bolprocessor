/* HTML.c (BP2 version CVS) */ 

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

/* This is defined by both Carbon and non-Carbon prefix headers */
#if  !defined(TARGET_API_MAC_CARBON)
   /* so if it is not defined yet, there is no prefix file, 
      and we are compiling the "Transitional" build. */
   /* Use MacHeaders.h until ready to convert this file.
      Then change to MacHeadersTransitional.h. */
// #  include	"MacHeaders.h"
#  include	"MacHeadersTransitional.h"
#endif

#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"


CheckHTML(int w,char** p_buffer,long* p_count,int* p_html)
// Check whether buffer contains HTML code, and converts if necessary
{
/* register */ int i,j,k,itoken;
int foundslash,imax,n,html;
char c,line[HTMLTOKENLENGTH];

imax = (*p_count) - 2;
if(*p_html) goto FOUND;
for(i=0; i < imax;) {
	j = i + 1;
	c = (*p_buffer)[i];
	if(c == '&') {
		if((*p_buffer)[j] == '#') {
			j++;
			while(TRUE) {
				if(j >= (*p_count) || (j > (i+5))) break;
				c = (*p_buffer)[j];
				if(c == ';' && (j > (i+2))) goto FOUND;
				if(!isdigit(c)) break;
				j++;
				}
			goto NEXTTOKEN;
			}
		while(TRUE) {
			if(j >= *p_count) break;
			c = (*p_buffer)[j];
			if(c == ';' && j > (i+1)) goto FOUND;
			if(!isalpha(c)) break;
			j++;
			}
		goto NEXTTOKEN;
		}
	if(c == '<') {
		foundslash = FALSE;
		k = 0;
		while(TRUE) {
			if(j >= *p_count) break;
			c = (*p_buffer)[j];
			if(c == '>') {
				if(foundslash && j > (i+2)) goto FOUND;
				line[k] = '\0';
				if(strcmp(line,"HTML") == 0) goto FOUND;
				if(strcmp(line,"BR") == 0) goto FOUND;
				goto NEXTTOKEN;
				}
			if(!foundslash) {
				if(c == '/') {
					foundslash = TRUE;
					j++;
					continue;
					}
				if(k < HTMLTOKENLENGTH) {
					line[k] = UpperCase(c);
					k++;
					}
				else break;
				}
			if(!isalpha(c)) break;
			j++;
			}
		}
NEXTTOKEN:
	i = j;
	}
return(OK);	/* This is not a HTML file */

FOUND:
/* Transcode from HTML to plain text in Mac character set */

if(w == wScrap && !(*p_html)) {
	if(Answer("Interpret HTML",'Y') != YES) return(OK);
	}
*p_html = html = TRUE;
for(i=j=0; ;i++,j++) {
	if(j >= *p_count) break;
	c = (*p_buffer)[j];
	if(html && (c == '\r' || c == '\n')) {
		if(c == '\r' && j > 0 && (*p_buffer)[j-1] != '>') (*p_buffer)[i++] = ' ';
		goto BAD;
		}
	if(c == '<') {
FOUNDTOKEN:
		/* Discard tag */
		j++; k = 0;
		while(TRUE) {
			if(j >= *p_count) goto BAD;
			c = (*p_buffer)[j];
			if(c == '>') {
				if(k < HTMLTOKENLENGTH) line[k] = '\0';
				if(strcmp(line,"/HTML") == 0) {
					html = FALSE;
					goto BAD;
					}
				if(!html) {
					if(strcmp(line,"HTML") == 0) {
						html = TRUE;
						goto BAD;
						}
					else {
						(*p_buffer)[i++] = '<';
						for(k=0; k < strlen(line); k++) {
							(*p_buffer)[i++] = line[k];
							}
						(*p_buffer)[i] = '>';
						goto GOOD;
						}
					}
				if(strcmp(line,"TITLE") == 0) {
					/* Skip the title */
					while(TRUE) {
						j++;
						if(j >= *p_count) break;
						c = (*p_buffer)[j];
						if(c == '<') goto FOUNDTOKEN;
						}
					break;
					}
				if(strcmp(line,"BR") == 0
						|| strcmp(line,"/UL") == 0
						|| strcmp(line,"LI") == 0) {
					(*p_buffer)[i] = '\r';
					goto GOOD;
					}
				if(strcmp(line,"P") == 0
						|| strcmp(line,"HR") == 0) {
					(*p_buffer)[i++] = '\r';
					(*p_buffer)[i] = '\r';
					goto GOOD;
					}
				if(line[0] == '/' && line[1] == 'H') {
					for(k=2; k < strlen(line); k++) {
						if(!isdigit(line[k])) goto BAD;
						}
					(*p_buffer)[i] = '\r';
					goto GOOD;
					}
				goto BAD;	/* Ignore this token */
				} 
			if(k < HTMLTOKENLENGTH) line[k] = UpperCase(c);
			j++; k++;
			}
		}
	if(!html) {
		(*p_buffer)[i] = c;
		goto GOOD;
		}
	if(c == '&') {
		j++; k = 0;
		if((*p_buffer)[j] == '#') {
			j++; n = 0;
			while(TRUE) {
				if(j >= (*p_count)) break;
				c = (*p_buffer)[j];
				if(c == ';') {
					if(n >= 32 && n < 256) {
						(*p_buffer)[i] = (*p_HTMLchar2)[n];
						goto GOOD;
						}
					break;
					}
				if(isdigit(c) && n < 256) n = (10 * n) + (c - '0');
				else n = 256;
				j++;
				}
			goto BAD; 	/* Token wasn't recognized */
			}
		while(TRUE) {
			if(j >= *p_count) goto BAD;
			c = (*p_buffer)[j];
			if(c == ';') {
				if(k < HTMLTOKENLENGTH) line[k] = '\0';
				else line[0] = '\0';
				for(itoken=0; itoken < MAXHTMLDIACR; itoken++) {
					if(Mystrcmp((*p_Diacritical)[itoken],line) == 0) {
						(*p_buffer)[i] = (*p_HTMLchar1)[itoken];
						goto GOOD;
						}
					}
				goto BAD; 	/* Token wasn't recognized */
				}
			if(k < HTMLTOKENLENGTH) line[k] = c;
			k++; j++;
			}
		}
	(*p_buffer)[i] = (*p_buffer)[j];
GOOD:
	continue;
BAD:
	i--;
	}
(*p_count) = i;
return(OK);
}


DOStoMac(char* p_c)
{
switch(*p_c) {
	case 'ã': *p_c = '‹'; break;
	case 'ç': *p_c = ''; break;
	case 'Ž': *p_c = '€'; break;
	case '': *p_c = ''; break;
	case '€': *p_c = '‚'; break;
	case '': *p_c = 'ƒ'; break;
	case '¥': *p_c = '„'; break;
	case '™': *p_c = '…'; break;
	case 'š': *p_c = '†'; break;
	case '„': *p_c = 'Š'; break;
	case ' ': *p_c = '‡'; break;
	case '…': *p_c = 'ˆ'; break;
	case 'ƒ': *p_c = '‰'; break;
	case '†': *p_c = 'Œ'; break;
	case '˜': *p_c = 'Ø'; break;
	case '‡': *p_c = ''; break;
	case '‚': *p_c = 'Ž'; break;
	case 'Š': *p_c = ''; break;
	case 'ˆ': *p_c = ''; break;
	case '‰': *p_c = '‘'; break;
	case '¡': *p_c = '’'; break;
	case '': *p_c = '“'; break;
	case 'Œ': *p_c = '”'; break;
	case '‹': *p_c = '•'; break;
	case '¤': *p_c = '–'; break;
	case '”': *p_c = 'š'; break;
	case '¢': *p_c = '—'; break;
	case '•': *p_c = '˜'; break;
	case '“': *p_c = '™'; break;
	case '£': *p_c = 'œ'; break;
	case '—': *p_c = ''; break;
	case '–': *p_c = 'ž'; break;
	case '': *p_c = 'Ÿ'; break;
	case 'Å': *p_c = ' '; break;
	case 'ø': *p_c = '¡'; break;
	case '›': *p_c = '¢'; break;
	case 'œ': *p_c = '£'; break;
	case '': *p_c = '¤'; break;
	case 'þ': *p_c = '¥'; break;
	case 'ï': *p_c = '¦'; break;
	case 'á': *p_c = '§'; break;
	case 'é': *p_c = '¨'; break;
	case '°': *p_c = 'ª'; break;
	case 'Ø': *p_c = '­'; break;
	case '’': *p_c = '®'; break;
	case 'í': *p_c = '¯'; break;
	case 'ì': *p_c = '°'; break;
	case 'ñ': *p_c = '±'; break;
	case 'ó': *p_c = '²'; break;
	case 'ò': *p_c = '³'; break;
	case '': *p_c = '´'; break;
	case 'æ': *p_c = 'µ'; break;
	case 'å': *p_c = '¶'; break;
	case 'ä': *p_c = '·'; break;
	case 'à': *p_c = '¹'; break;
//	case 'ã': *p_c = '¸'; break;
	case 'ô': *p_c = 'º'; break;
	case '¦': *p_c = '»'; break;
	case '§': *p_c = '¼'; break;
	case 'ê': *p_c = '½'; break;
	case '‘': *p_c = '¾'; break;
	case '¨': *p_c = 'À'; break;
	case '­': *p_c = 'Á'; break;
	case '"': *p_c = '"'; break;  /* 'Ò' */
	case '®': *p_c = 'Ç'; break;
	case '¯': *p_c = 'È'; break;
	case 'Ä': *p_c = 'Ñ'; break;
	case '`': *p_c = 'Ô'; break;
	case 'û': *p_c = 'Ã'; break;
	case 'ö': *p_c = 'Ö'; break;
	case 'ð': *p_c = '×'; break;
	case 'ª': *p_c = 'Â'; break;
	case 'Ÿ': *p_c = 'Ä'; break;
	case '÷': *p_c = 'Å'; break;
	case '': *p_c = 'Æ'; break;
	case '\n': *p_c = '\r'; break;
	}
return(OK);
}


MacToHTML(int makedos,char*** pp_line,int makesingleline)
{
int itoken;
long i,j,j0,jmax,k,r;
char c,**p_line2,line[4];
static char* rest = "_rest";
static char* br = "<BR>";
static char* html = "<HTML>";
static char* slashhtml = "</HTML>";
Size s;

r = OK;
if(*pp_line == NULL || (**pp_line)[0] == '\0') return(OK);
s = MyGetHandleSize(*pp_line);
if(s < 20L) s = 20L;
if((p_line2 = (char**) GiveSpace(s)) == NULL) return(ABORT);
j = ZERO;
jmax = (s / sizeof(char)) - 10L;

if(makesingleline) for(k=0; k < strlen(html); k++) (*p_line2)[j++] = html[k];

for(i=j0=ZERO; ; i++) {
	c = (**pp_line)[i];
	if(c == '\0') break;
	if(!makesingleline && isspace(c) && (j - j0) > 80L) {
	/* Slice to 80 char lines */
		(*p_line2)[j++] = '\r';
		if(makedos) (*p_line2)[j++] = '\n'; /* DOS compatible */
		j0 = j;
		continue;	/* The space was discarded and replaced with CR */
		}
	if(j >= jmax) {
		if((p_line2 = (char**) IncreaseSpace((Handle)p_line2)) == NULL) {
			r = ABORT; goto OUT;
			}
		jmax = (((jmax + 10L) * 3L) / 2L) - 10L;
		}
	if(GoodHTMLchar(c)) {
		(*p_line2)[j++] = c;
		continue;
		}
	if(c == 'É') {
		for(k=0; k < strlen(rest); k++) (*p_line2)[j++] = rest[k];
		continue;
		}
	if(isspace(c)) {
		switch(c) {
			case ' ':
			case '\t':
				(*p_line2)[j++] = ' ';
				continue;
				break;
			default:
				for(k=0; k < strlen(br); k++) (*p_line2)[j++] = br[k];
				continue;
				break;
			}
		}
	(*p_line2)[j++] = '&';
	for(itoken=0; itoken < MAXHTMLDIACR; itoken++) {
		if((*p_HTMLchar1)[itoken] == c) {
			for(k=0; k < MyHandleLen((*p_Diacritical)[itoken]); k++) {
				(*p_line2)[j++] = (*((*p_Diacritical)[itoken]))[k];
				if(j >= jmax) {
					if((p_line2 = (char**) IncreaseSpace((Handle)p_line2)) == NULL) {
						r = ABORT; goto OUT;
						}
					jmax = (((jmax + 10L) * 3L) / 2L) - 10L;
					}
				}
			goto ENDTOKEN;
			}
		}
	(*p_line2)[j++] = '#';
	for(itoken=32; itoken < 256; itoken++) {
		if((*p_HTMLchar2)[itoken] == c) {
			sprintf(line,"%ld",(long)itoken);
			for(k=0; k < strlen(line); k++) {
				(*p_line2)[j++] = line[k];
				}
			goto ENDTOKEN;
			}
		}
ENDTOKEN: 
	(*p_line2)[j++] = ';';
	}

if(makesingleline)
	for(k=0; k < strlen(slashhtml); k++) (*p_line2)[j++] = slashhtml[k];
(*p_line2)[j] = '\0';

s = MyGetHandleSize(p_line2);
if((r=MySetHandleSize((Handle*)pp_line,(Size)s)) != OK) return(r);
if((r=MystrcpyHandleToHandle(0,pp_line,p_line2)) != OK) return(r);

OUT:
MyDisposeHandle((Handle*)&p_line2);
return(r);
}


GoodHTMLchar(char c)
{
if(isalnum(c)) return(YES);
if(ispunct(c)) {
	switch(c) {
		case '&':
		case '<':
		case '>':
		case '\"':
			return(NO);
		default:
			return(YES);
		}
	}
return(NO);
}


NeedsHTMLConversion(char **p_line)
{
long i;
char c;

if(p_line == NULL) return(NO);

i = 0;
while((c=(*p_line)[i++]) != '\0') {
	if(!GoodHTMLchar(c) && c != '\r') return(YES);
	}
return(NO);
}