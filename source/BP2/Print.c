/* Print.c (BP2 version CVS) */

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

/* There is no way to compile this file with OPAQUE_TOOLBOX_STRUCTS set to 1 */
/* This file is usable in non-Carbon builds and will be replaced for Carbon builds */
#if  !defined(TARGET_API_MAC_CARBON)
#  include	"MacHeaders.h"
// #  include	"MacHeadersTransitional.h"
#endif

#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"
	
#define topMargin 20
#define leftMargin 20
#define rightMargin 30
#define bottomMargin 20


DoPageSetUp(void)
{
SetCursor(&Qd.arrow);
PrOpen();
// if(PrError() != noErr) return(FAILED);
CheckPrintHandle();
if(PrStlDialog(hPrint)) ;
PrClose();
return(OK);
}


PrintTextDocument(int w)
{
int linebase,lineheight,uplimit,bottomlimit,leftlimit,rightlimit,
	page,line,maxcharsinline,maxlinesinpage,rep,poslastspace,poslastbreak,
	toolong;
Rect printrect;
char **htext,c,oldc;
long i,im,posstart,posend;
FontInfo info;
TPPrPort printport;
/* GrafPtr saveport; */

htext = WindowTextHandle(TEH[w]);
posstart = (long)(**(TEH[w])).selStart;
posend = (long)(**(TEH[w])).selEnd;
rep = NO;
if((posend - posstart) > 10L) {
	rep = Answer("Print only selection",'N');
	if(rep == ABORT) return(OK);
	}
if(rep == NO) {
	posstart = ZERO;
	posend = GetTextLength(w);
	}
printport = PrOpenDoc(hPrint,0L,0L);
SetPort(&printport->gPort);

TextFont(kFontIDCourier);
TextSize(WindowTextSize[w]);

printrect = (**hPrint).prInfo.rPage;
GetFontInfo(&info);
lineheight = info.leading+info.ascent+info.descent;
uplimit = printrect.top + topMargin;
bottomlimit = printrect.bottom - bottomMargin;
leftlimit = printrect.left + leftMargin;
rightlimit = printrect.right - rightMargin;
MyLock(TRUE,(Handle)htext);
maxcharsinline = (rightlimit - leftlimit) / TextWidth("m",0,1);
maxlinesinpage = (bottomlimit - uplimit) / lineheight;
page = 0;

NEWPAGE:
line = 0; page++;
PrOpenPage(printport,0L);
// if(PrError() != noErr) goto OUT;
linebase = uplimit + lineheight;
MoveTo(leftlimit,linebase);
if(WindowName[w][0] != '\0')
	sprintf(Message,"%s window / '%s' / page %ld",WindowName[w],
		FileName[w],(long)page);
else
	sprintf(Message,"%s window / page %ld",WindowName[w],(long)page);
DrawText(Message,0,strlen(Message));
linebase += lineheight;
MoveTo(leftlimit,linebase);
MystrcpyHandleToString(MAXINFOLENGTH,0,Message,p_FileInfo[w]);
DrawText(Message,0,strlen(Message));
linebase += lineheight;
line += 3;

NEWLINE:
linebase += lineheight;
MoveTo(leftlimit,linebase);
poslastspace = poslastbreak = -1;
if(posstart > posend) goto OUT;
oldc = '\0'; toolong = FALSE;
for(i=posstart; ; i++) {
	if(i >= posend) break;
	c = (*htext)[i];
	if(c == '\r' || oldc == 'Â') {
		break;
		}
	if(isspace(c)) poslastspace = i;
	if(CanBreak(c,oldc)) poslastbreak = i;
	if((i-posstart) > maxcharsinline) {
		toolong = TRUE; break;
		}
	oldc = c;
	}
im = i;
if(toolong && poslastspace > (posstart + 3 * im) / 4) {
	toolong = FALSE; im = poslastspace;
	}
if(toolong && poslastbreak > (posstart + 3 * im) / 4) {
	toolong = FALSE; im = poslastbreak;
	}
if(im > posstart) DrawText(*htext,(short)posstart,(short)(im-posstart));
if(toolong || im == poslastbreak) posstart = im;
else posstart = im + 1;

line++;
if(line >= maxlinesinpage) {
	PrClosePage(printport);
	goto NEWPAGE;
	}
goto NEWLINE;

OUT:
PrClosePage(printport);
MyUnlock((Handle)htext);
PrCloseDoc(printport);
/* SetPort(saveport); */
return(OK);
}


PrintTextWindow(int w)
{
TPPrPort	printport;
GrafPtr		saveport;
TPrStatus	prStatus;
int			copies;

GetPort(&saveport);

PrOpen();
// if(PrError() != noErr) return(FAILED);
CheckPrintHandle();
StopWait();
if(PrJobDialog(hPrint) != 0) {
	SetCursor(&WatchCursor);
	for(copies=HowMany(); copies > 0; copies--) {
		PrintTextDocument(w);
		// Handle print spooler
		if((*hPrint)->prJob.bJDocLoop == bSpoolLoop) {
			PrPicFile(hPrint,(TPPrPort)nil,nil,nil,&prStatus);
			if(PrError() != noErr) break;
			}
		}
	}
PrClose();
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err PrintTextWindow(). saveport == NULL");
return(OK);
}


HowMany()
{
return(((**hPrint).prJob.bJDocLoop
				== bDraftLoop) ? (**hPrint).prJob.iCopies : 1);
}


PrintGraphicWindow(PicHandle whichPic,Rect *p_destrect)
{
GrafPtr	saveport;
TPrStatus prStatus;
TPPrPort printport;
OSErr err;
int copies,bottomlimit,rightlimit;
Rect printrect;

GetPort(&saveport);
PrOpen();
// if(PrError() != noErr) return(FAILED);
CheckPrintHandle();
ClipRect(p_destrect);

StopWait();
if(PrJobDialog(hPrint)) {
	SetCursor(&WatchCursor);
	printport = PrOpenDoc(hPrint,(TPPrPort)0L,(Ptr) NULL);
	printrect = (**hPrint).prInfo.rPage;
	bottomlimit = printrect.bottom - bottomMargin;
	rightlimit = printrect.right - rightMargin;
	if(p_destrect->bottom > bottomlimit) p_destrect->bottom = bottomlimit;
	if(p_destrect->right > rightlimit) p_destrect->right = rightlimit;
	SetPort(&printport->gPort);
	for(copies=HowMany(); copies > 0; copies--) {
		PrOpenPage(printport,(Rect*) NULL);
		// ---------
		DrawPicture(whichPic,p_destrect);
		// ---------
		PrClosePage(printport);
		if((*hPrint)->prJob.bJDocLoop == bSpoolLoop) {
			PrPicFile(hPrint,(TPPrPort)nil,nil,nil,&prStatus);
			if(PrError() != noErr) break;
			}
		}
	PrCloseDoc(printport);
	}
PrClose();
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err PrintGraphicWindow(). saveport == NULL");
return(OK);
}


CanBreak(char c,char oldc)
{
switch(c) {
	case '[':
	case '+':
		return(YES); break;
	case '<':
	case '-':
	case '/':
		if(oldc != c) return(YES);
		break;
	case '(':
		if(oldc != '_') return(YES);
		break;
	case '_':
		if(!isalnum(oldc)) return(YES);
	}
switch(oldc) {
	case ')':
	case ']':
	case '>':
		return(YES);
	}
if(isdigit(c) && oldc != '/' && !isalnum(oldc)) return(YES);
return(NO);
}


CheckPrintHandle(void)
{
if(hPrint == NULL) hPrint = (TPrint**) NewHandle(sizeof(TPrint));
PrintDefault(hPrint);
return(OK);
}