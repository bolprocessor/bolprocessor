/* Interface2.c  (BP2 version CVS) */
 
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

#if BP_CARBON_GUI

Ours(WindowPtr theWindow, WindowPtr thatWindow)
{
return((thatWindow != ZERO) && (theWindow == thatWindow));
}


CantOpen(void)
{
Rect r;
WindowPtr w;

SetRect(&r,152,60,366,132);
SetPortWindowPort((w = NewWindow(0L,&r,"\p",1,1,NULL,0,0L)));
TextFont(0);
MoveTo(4,40);
DrawString("\pCan't open resource file!");
#if TARGET_API_MAC_CARBON
  QDFlushPortBuffer(GetWindowPort(w), NULL);
#endif
while(!Button());
return(OK);
}


DoCommand(int w,long menuchoice)
{
int i,theitem,menuID,imenu,r,wind;
IntProcPtr DoIt;
Str255	name;
char line[256];

SetCursor(&WatchCursor);

HiliteMenu(0);
theitem = LoWord(menuchoice);
menuID = HiWord(menuchoice);
imenu = menuID - MenuIDoffset;
/* Except for 'scriptM', all menus have MAXMENUITEMS items */
if(imenu < 0 || imenu > MAXMENU || theitem < 1
			|| (imenu > 0 && imenu < scriptM && theitem >= MAXMENUITEMS))
	return(FAILED);
if(Help) {
	Help = FALSE;
	GetMenuItemText(myMenus[imenu],theitem,name);
	MyPtoCstr(255,name,line);
	for(i=0; i < strlen(line); i++)
		if(line[i] == '\r' || line[i] == '\n') line[i] = '\0';
	FilterHelpSelector(line);
	DisplayHelp(line);
	return(OK);
	}
switch(imenu) {
	case appleM:
		Option = FALSE;
		if(theitem == aboutCommand) return(mAbout(w));
#if !TARGET_API_MAC_CARBON
		GetMenuItemText(myMenus[appleM],theitem,name);
		OpenDeskAcc(name);
#endif
		SelectWindow(Window[w]);
		ResetTicks(FALSE,TRUE,ZERO,0);
		break;
	case scriptM:
		Option = FALSE;
		if(theitem == 1) return(mCheckScript(w));
		if(theitem < 2 || theitem >= MaxScriptInstructions+3) return(OK);
		GetMenuItemText(myMenus[scriptM],theitem,name);
		MyPtoCstr(255,name,line);
		if(line[0] == '\0') return(OK);
		if(line[0] == 'I' && line[1] == 'N' && line[2] == ' ')
			wind = wInteraction;
		else	if(theitem == 4) wind = wGlossary;
				else {
					if(w == LastEditWindow) wind = w;
					else wind = wScript;
					}
		if(wind == wScript || wind == wInteraction || wind == wGlossary)
			Println(wind,line);
		else Print(wind,line);
		ShowSelect(CENTRE,wind);
		break;
	default:
		if((imenu == actionM || imenu == windowM)
			&& (r=GetDialogValues(w)) != OK) return(r);
		DoIt = Menu[imenu][theitem];
		if(DoIt != NULL) {
			r = ((int (*)(int)) DoIt)(w);
			Option = Panic = FALSE;
			if(r == ABORT || r == RESUME || r == STOP || r == UNDO
				|| r == EXIT) return(r);
			}
	}
Option = FALSE;
return(OK);
}

#endif /* BP_CARBON_GUI */

#if BP_CARBON_GUI

ShowMessage(int store,int w,char *s)
{
int i;
Rect r;
GrafPtr saveport;

if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. ShowMessage()");
	return(OK);
	}
if(store) {
	Jmessage++; if(Jmessage >= MAXMESSAGE) Jmessage = 0;
	MystrcpyStringToHandle(&(p_MessageMem[Jmessage]),s);
	}
	
if(Beta && ScriptExecOn && TraceRefnum != -1 /* && Tracefile != (FILE*) NULL */)
/*	fprintf(Tracefile,"%s\n",s); */
	WriteToFile(NO,MAC,s,TraceRefnum);

ShowWindow(Window[w]);
if (RunningOnOSX) SendBehind(Window[w], FrontWindow()); // BringToFront() sometimes activates window on OS X
else BringToFront(Window[w]);
SetSelect(ZERO,GetTextLength(w),TEH[w]);
TextDelete(w);
PrintBehind(w,s);
TextUpdate(w);
GetPort(&saveport);
SetPortWindowPort(Window[w]);
GetWindowPortBounds(Window[w], &r);
InvalWindowRect(Window[w], &r);
QDFlushPortBuffer(GetWindowPort(Window[w]), NULL);
if(saveport != NULL) SetPort(saveport);

return(OK);
}


ClearMessage(void)
{
SelectBehind(ZERO,GetTextLength(wMessage),TEH[wMessage]);
TextDelete(wMessage);
TextUpdate(wMessage);
return(OK);
}


FlashInfo(char* s)
{
SelectBehind(ZERO,GetTextLength(wInfo),TEH[wInfo]);
TextDelete(wInfo);
PrintBehind(wInfo,s);
ShowWindow(Window[wInfo]);
if (RunningOnOSX) SendBehind(Window[wInfo], FrontWindow());
else BringToFront(Window[wInfo]);
TextUpdate(wInfo);
#if TARGET_API_MAC_CARBON
  QDFlushPortBuffer(GetWindowPort(Window[wInfo]), NULL);
#endif
return(OK);
}

#endif /* BP_CARBON_GUI */

SelectSomething(int w)
/* Select line on which cursor is flashing */
{
long startpos,endpos,length;
char c;
int j;

if(w < 0 || w >= WMAX || !Editable[w]) return(FAILED);
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
if(endpos <= startpos) return(FAILED);
SetSelect((long)startpos,(long)endpos,TEH[w]);
return(OK);
}


char Filter(char c)
{
switch(c) {
	case 'Ð':
	case 'Ñ':
		c = '-'; break;
	case 'Ô':
	case 'Õ':
		c = '\''; break;
	case 'Ò':
	case 'Ó':
		c = '"'; break;
	case 'á':
		c = '¥'; break;
	}
return(c);
}


ReadLine(int nocomment,int w,long *p_i,long im,char ***pp_line,int* p_gap)
/* Read unlimited line in TExt buffer */
{
int j,k,jm;
char c,oldc,**ptr;
long origin,end,length;

*p_gap = 0;
MyDisposeHandle((Handle*)pp_line);
if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. ReadLine(YES,). ");
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
	if((c == '\r' || c == '\n') && oldc != 'Â') break;
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
return(FAILED);
}


ReadLine1(int check,int w,long *p_i,long im,char *line,int size)
/* Read line in TExt buffer.  Old version: length is limited to 'size'. */
{
int j,k,l;
char c,oldc;

if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. ReadLine1(). ");
	return(OK);
	}
if(*p_i >= im) return(FAILED);
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
			sprintf(Message,"\nSelection too long, truncated: %s...\n",line);
			if(!ScriptExecOn) Alert1(Message);
			else PrintBehindln(wTrace,Message);
			return(FAILED);
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


IsEmpty(int w)
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
		/* line is "BP2 script" */
		}
	if(w == wGrammar) strcpy(line,"COMMENT:");
	len = strlen(line);
	while(ReadLine(YES,w,&pos,posmax,&p_line,&gap) == OK) {
		if((*p_line)[0] == '\0' || (*p_line)[0] == '\r') continue;
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

#if BP_CARBON_GUI

DoArrowKey(int w,char thechar,int shift,int option)
{
switch(thechar) {
	case '\34':	/* Left arrow */
		MoveWord(w,-1,shift,option);
		break;
	case '\35':	/* Right arrow */
		MoveWord(w,+1,shift,option);
		break;
	case '\36':	/* Up arrow */
		MoveParagraph(w,-1,shift);
		break;
	case '\37':	/* Down arrow */
		MoveParagraph(w,+1,shift);
		break;
	case '\f':	/* Page down */
		MovePage(w,+1);
		break;
	case '\v':	/* Page up */
		MovePage(w,-1);
		break;
	default: return(FAILED);
	}
return(OK);
}


TypeChar(int thechar,int shift)
{
int i,j,cutall,arrowkey;
char firstchar,diffchar;
long orgsel;
TextHandle th;
TextOffset selbegin, selend;

if(Nw < 0 || Nw >= WMAX) {
	if(Beta) Alert1("Err. TypeChar()");
	return(ABORT);
	}
if(!Editable[Nw] && !HasFields[Nw]) return(OK);
cutall = arrowkey = FALSE;
if(Editable[Nw]) {
	th = TEH[Nw];
	TextGetSelection(&selbegin, &selend, th);
	if(thechar == '\b' && GetTextLength(Nw) == selend && selbegin == 0) {
		if(SaveCheck(Nw) == ABORT) return(FAILED);
		cutall = TRUE;
		}
	switch(thechar) {
		case '\f':		/* Page down */
		case '\v':		/* Page up */
			shift = TRUE;
			/* no break */
		case '\34':
		case '\35':
		case '\36':
		case '\37':
			arrowkey = TRUE;
		}
	if(arrowkey && shift) {
		DoArrowKey(Nw,thechar,shift,FALSE);
		if(!WASTE && LastAction == TYPEWIND) LastAction = NO;
		return(OK);
		}
	else {
		if(!arrowkey && thechar != '\b' && (LastAction != TYPEWIND || UndoWindow != Nw)) {
			TextOffset dummy;
			if(!TextIsSelectionEmpty(th)) {
				TextCut(Nw);
				LastAction = CUTWIND;
				UndoWindow = Nw;
				TextGetSelection(&UndoPos, &dummy, th);
				}
			else {
				LastAction = TYPEWIND;
				UndoWindow = Nw;
				TextGetSelection(&UndoPos, &dummy, th);
				}
			}
		}
	firstchar = 'A'; diffchar = 'a' - 'A';
	if(Token && !ScriptExecOn && !VariableOn && (((i = (thechar - firstchar)) >= 0 && i < 26)
										|| (i >= diffchar && i < diffchar+26))) {
		j = i;
		if(j > 25) j -= (diffchar - 26);
		PrintHandle(Nw,((*p_Token)[Key(j,KeyboardType)]));
		if((*((*p_Token)[Key(j,KeyboardType)]))[0] == thechar && thechar <= 'Z'
				&& (*((*p_Token)[Key(j,KeyboardType)]))[1] == '\0') VariableOn = TRUE;
		else if(SplitTimeObjects) Print(Nw," ");
		}
	else {
		TextAutoView(FALSE,TRUE,th);
		DoKey(thechar,0,th);
		TextAutoView(FALSE,FALSE,th);
		if(cutall) ClearWindow(FALSE,Nw);
		if(thechar == 'Â') {
			thechar = '\r';
			DoKey(thechar,0,th);
			}
		}
	ShowSelect(CENTRE,Nw);
	if(Nw != wHelp) LastEditWindow = Nw;
	if(!SpaceOn && (MySpace(thechar) || thechar == '\r' || thechar == '\n')) {
		SpaceOn = TRUE;
		VariableOn = FALSE;
		}
	else SpaceOn = FALSE;
	}

if(!arrowkey) UpdateDirty(FALSE,Nw);
if(cutall) ForgetFileName(Nw);

return(OK);
}

/* ----------------------  DOTHINGS ---------------------------------- */
// Create and handle a dialog window in which buttons are created
// and labelled automatically


#define maxctrl 500
#define maxpage 25

DoThings(char ****p_thing, int jmin, int jmax, int** pp_index,int widmax, IntProcPtr p_proc,
	char failedstring[], int ctrltype)
{
DialogPtr p_dia;
//DialogRecord dr;
GrafPtr saveport;
Rect r, rtemp;
short part;
int theitem,i,im,j,jj,k,kmax,found,ibot,left,right,top,bottom,leftoffset,
	vpitch,morecoming,numberdrawn,page,j0[maxpage],result,start[maxpage];
ControlHandle h,h_ctrl[maxctrl];
EventRecord theEvent;
Str255 title;

GetPort(&saveport);
for(i=0; i < maxctrl; i++) h_ctrl[i] = NULL;
page = im = 0; j0[0] = jmin;
p_dia = GetNewDialog(ObjectChartID, NULL, (WindowPtr)-1L);
result = OK;
start[0] = 1;
vpitch = Buttonheight + 5;
ShowWindow(GetDialogWindow(p_dia)); SelectWindow(GetDialogWindow(p_dia));
SetPortDialogPort(p_dia);
TextFont(kFontIDCourier); TextSize(10);
r.top = 10; r.left = 5; r.bottom = 30; r.right = 74;	// std Cancel buttonwidth is 69 on OS X
h_ctrl[0] = NewControl(GetDialogWindow(p_dia),&r,"\p",1,0,0,1,pushButProc,0L);
top = 40;
left = 0;
GetPortBounds(GetDialogPort(p_dia), &rtemp);
bottom = rtemp.bottom - rtemp.top;
right = rtemp.right - rtemp.left;
/* right = 150; */
ibot = (bottom - top) / vpitch;
r.top = 10;
r.left = rtemp.right - rtemp.left - 60;
r.bottom = r.top + 20;
r.right = r.left + 50;
h_ctrl[1] = NewControl(GetDialogWindow(p_dia),&r,"\p==>",(Boolean)1,(short)0,(short)0,(short)1,
	(short)pushButProc,0L);
r.left = rtemp.right - rtemp.left - 120;
r.right = r.left + 50;
h_ctrl[2] = NewControl(GetDialogWindow(p_dia),&r,"\p<==",(Boolean)1,(short)0,(short)0,(short)1,
	(short)pushButProc,0L);
ibot = (bottom - top) / vpitch;
im = 2;

REDRAW:
StopWait();
ShowControl(h_ctrl[0]);
/* SetCTitle(h_ctrl[0],"\pCancel"); */
SetControlTitle(h_ctrl[0],"\pCancel");	/* Redraws the control */
HiliteControl(h_ctrl[0],0);
morecoming = TRUE;
j = j0[page];
leftoffset = 5 /* - Buttonheight - (widmax * Charstep) */;
for(i=0; ; i++,j++) {
	if(j >= jmax) {
		morecoming = FALSE;
		if(!start[page]) {
			for(i=i+1; i <= im; i++) {
				HiliteControl(h_ctrl[i],255); /* dimmed */
				HideControl(h_ctrl[i]);
				}
			}
		break;
		}
	jj = j; if(pp_index != NULL) jj = (*pp_index)[j];
	kmax = MyHandleLen((*p_thing)[jj]);
	if(kmax == 0) {
		title[0] = 0;
		}
	else {
		for(k=1; k <= widmax, k <= kmax; k++) {
			title[k] = (*((*p_thing)[jj]))[k-1];
			}
		title[0] = k - 1;	/* This is a Pascal string */
		}
	if(start[page]) {
		if(im >= maxctrl) {
			if(Beta) Alert1("Too many objects.  Increase MAXCTRL!");
			im--; result = ABORT; goto QUIT;
			}
		if((i % ibot) == 0 && i > 0) {
			leftoffset = leftoffset + Buttonheight + (widmax * Charstep) + 5;
			}
		r.top = top + (i % ibot) * vpitch;
		r.bottom = r.top + Buttonheight;
		r.left = left + leftoffset;
		r.right = r.left + Buttonheight + (widmax * Charstep);
		if(r.right > right) break;
		if((h_ctrl[++im] = NewControl(GetDialogWindow(p_dia),&r,title,1,0,0,1,(ctrltype + 8),0L))
				== NULL) {
			if(Beta) Alert1("Err1. DoThings()");
			result = ABORT; goto QUIT;
			}
		ShowControl(h_ctrl[im]);
		HiliteControl(h_ctrl[im],0);
		}
	else {
		if(i >= numberdrawn) break;
		if(h_ctrl[i+1] == NULL) {
			if(Beta) Alert1("Err2. DoThings()");
			result = ABORT; goto QUIT;
			}
		/* SetCTitle(h_ctrl[i+1],title); */
		SetControlTitle(h_ctrl[i+1],title);
		ShowControl(h_ctrl[i+1]);
		HiliteControl(h_ctrl[i+1],0);
		}
	}
if(start[0]) numberdrawn = im;
/* if(start[page]) {
	r.top = bottom + 10;
	r.left = p_dia->portRect.right - p_dia->portRect.left - 60;
	r.bottom = r.top + Buttonheight;
	r.right = r.left + 50;
	if(h_ctrl[1] == NULL) h_ctrl[1] = NewControl(p_dia,&r,
		"\p==>",1,0,0,1,pushButProc,0L);
	r.left = p_dia->portRect.right - p_dia->portRect.left - 120;
	r.right = r.left + 50;
	if(h_ctrl[2] == NULL) h_ctrl[2] = NewControl(p_dia,&r,
		"\p<==",1,0,0,1,pushButProc,0L);
	} */
if(morecoming) {
	ShowControl(h_ctrl[1]);	/* ==> */
/*	HiliteControl(h_ctrl[1],0); */
	}
else {
	HideControl(h_ctrl[1]); /* ==> */
	}
if(page == 0) {
	HideControl(h_ctrl[2]); /* <== */
	}
else {
	ShowControl(h_ctrl[2]);
/*	HiliteControl(h_ctrl[2],0); */
	}
SelectWindow(GetDialogWindow(p_dia)); 
/* DrawControls(p_dia); */
found = FALSE;
SetPortDialogPort(p_dia);

FlushEvents(everyEvent,0);

do {
	if(WaitNextEvent(mDownMask,&theEvent,60L,NULL)) {
		GlobalToLocal(&(theEvent.where));
		if(FindControl(theEvent.where, GetDialogWindow(p_dia), &h) != 0) {
			if (TrackControl(h,theEvent.where,NULL) != 0) {
			  for(i=0; i <= im; i++) {
				if(h == h_ctrl[i]) {
					found = TRUE;
					theitem = i;
					}
				}
			  }
			}
		else SysBeep(10);
		}
	}
while(!found);
if(theitem == 0) {
	result = ABORT; goto QUIT;
	}
if(theitem == 1) { /* Right arrow */
	if(page >= maxpage-1) {
		if(Beta) Alert1("Too many pages.  Contact Bel!");
		 result = ABORT; goto QUIT;
		}
	if(start[page]) {
		start[page+1] = 1;
		start[page] = 0;
		}
	j0[page+1] = j0[page] + numberdrawn - 2;
	page++;
	goto REDRAW;
	}
if(theitem == 2) { /* Left arrow */
	start[page--] = 0;
	goto REDRAW;
	}
ShowControl(h);
HiliteControl(h,0);
if(p_proc == MINUSPROC) {
	result = theitem - 3 + j0[page];
	goto QUIT;
	}
else {
	if((((int (*)(int)) p_proc)(theitem - 3 + j0[page])) == FAILED) Alert1(failedstring);
	else {
		result = theitem - 3 + j0[page];
		goto QUIT;
		}
	}
start[page] = 0;
SelectWindow(GetDialogWindow(p_dia)); 
goto REDRAW;

QUIT:
DisposeDialog(p_dia);
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err DoThings(). saveport == NULL");
return(result);
}


GetClickedWord(int w,char* line)
// Select epression between quotes, if any, or between spaces, or performance control, tool, procedureÉ
{
char c;
long i,iorg,iend,length,start1,start2,end1,end2;

TextGetSelection(&iorg, &iend, TEH[w]);
length = GetTextLength(w);

start1 = end1 = start2 = end2 = -1L;
for(i=iorg; ; i--) {
	c = GetTextChar(w,i);
	if(c == '_') {
		if(start1 == -1L) start1 = i;
		}
	if(!isalnum(c)) {
		if(start1 == -1L) start1 = i + 1;
		}
	if(c == '\"') {
		start2 = i + 1;
		break;
		}
	if(i == 0 || c == '\r') {
		if(start1 == -1L) start1 = i;
		break;
		}
	}
for(i=iend; ; i++) {
	c = GetTextChar(w,i);
	if(c == '(') {
		if(end1 == -1L) end1 = i + 1;
		}
	if(!isalnum(c)) {
		if(end1 == -1L) end1 = i;
		}
	if(c == '\"') {
		end2 = i;
		break;
		}
	if(i >= length-1 || c == '\r') {
		if(end1 == -1L) end1 = i + 1;
		break;
		}
	}
if(start2 > -1L && end2 > -1L && end2 > start2) {
	iorg = start2; iend = end2;
	}
else if(start1 > -1L && end1 > -1L && end1 > start1) {
	iorg = start1; iend = end1;
	}
else return(FAILED);
SetSelect(iorg,iend,TEH[w]);
for(i=iorg; i < iend; i++) line[i-iorg] = GetTextChar(w,i);
line[i-iorg] = '\0';
return(OK);
}


ShowHelpOnText(int w)
{
char line[MAXLIN];
int r;

if((r=GetClickedWord(w,line)) != OK) return(r);
Help = FALSE;
FilterHelpSelector(line);
DisplayHelp(line);
return(OK);
}

#endif /* BP_CARBON_GUI */

#if BP_CARBON_GUI

Answer(char *what,char c)
{
int r,rep,w,i;
Rect rect;
char line[MAXLIN];

if(ScriptExecOn || (AEventOn && !OkWait)) {
	if(c == 'Y') r = YES;
	else r = NO;
	sprintf(line,"%s ? %c\n",what,c);
	if(ScriptExecOn) {
		if(wTrace != OutputWindow) PrintBehind(wTrace,line);
		}
	else ShowMessage(TRUE,wMessage,line);
	return(r);
	}
StopWait();

if(CallUser(0) != OK) {
	if(c == 'Y') r = YES;
	else r = NO;
	return(r);
	}

strcpy(line,what);
ParamText(in_place_c2pstr(line),"\p ?","\p","\p");
if(ComputeOn || PlaySelectionOn) SndSetSysBeepState(sysBeepDisable);
if(c == 'Y') r = Alert(YesNoCancel,0L);
else  r = Alert(NoYesCancel,0L);
SndSetSysBeepState(sysBeepEnable);
#if 0
AlertOn++;
Interrupted = TRUE;
if(!EmergencyExit && !InputOn)
	for(i=0; i < 5; i++) MainEvent();
AlertOn--;
#endif
switch(r) {
	case dDefault:
		if(c == 'Y') rep = YES;
		else rep = NO;
		break;
	case dAltern:
		if(c == 'Y') rep = NO;
		else rep = YES;
		break;
	case dAbort:
		rep = ABORT;
		break;
	}
UpdateWindow(FALSE,Window[wGraphic]);
UpdateWindow(FALSE,Window[wPrototype1]);
return(rep);
}


AnswerWith(char message[],char defaultvalue[],char value[])
{
Rect r;
Handle itemhandle;
short item,itemtype;
int rep,eventfound;
Str255 t;
EventRecord event;
DialogPtr theDialog;
char c;

StopWait();
SetField(EnterPtr, wUnknown, fMessage, message);
SetField(EnterPtr, wUnknown, fValue, defaultvalue);
SelectField(EnterPtr, wUnknown, fValue, TRUE);

ShowWindow(GetDialogWindow(EnterPtr));
SelectWindow(GetDialogWindow(EnterPtr)); // was BringToFront() - akozar 052107
DrawDialog(EnterPtr);
// HiliteDefault(EnterPtr);
EnterOn = TRUE;
Interrupted = TRUE;
AlertOn++;
do {
	MaintainCursor();	// FIXME: this needs to be in an event filter procedure to work
	ModalDialog(NULL, &item);
	rep = RESUME;
	switch(item) {
		case dEnterCancel:
			rep = ABORT; break;
		case dEnterOK:
			rep = OK; break;
		}
	}
while (rep != OK && rep != ABORT);
EnterOn = FALSE;
AlertOn--;
GetDialogItem(EnterPtr,fValue,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(255,t,value);
HideWindow(GetDialogWindow(EnterPtr));
UpdateWindow(FALSE,Window[wGraphic]);
UpdateWindow(FALSE,Window[wPrototype1]);
return(rep);
}


Alert1(char s[])
{
char line[255];
int i;
Rect r;
GrafPtr saveport;
OSErr io;

if(!EmergencyExit && !InitOn && CallUser(0) != OK) return(OK);

SetDefaultCursor();
GetPort(&saveport);
if(!EmergencyExit && !InitOn && Nw >= 0 && Nw < WMAX) {
	SetPortWindowPort(Window[Nw]);
	GetWindowPortBounds(Window[Nw], &r);
	InvalWindowRect(Window[Nw], &r);
	}
/*r = screenBits.bounds;  // can't Carbonize since no window for screenbits -- seems unnecessary regardless - 011807 akozar
if(!EmergencyExit && !InitOn && Nw >= 0 && Nw < WMAX) InvalRect(&r);*/
if(saveport != NULL) SetPort(saveport);

strcpy(line,s);
ParamText(in_place_c2pstr(line),"\p","\p","\p");
if(ComputeOn || PlaySelectionOn) SndSetSysBeepState(sysBeepDisable);
GotAlert = TRUE;
Interrupted = TRUE;
if(Beta && ScriptExecOn)  ShowMessage(TRUE,wMessage,s); // so we don't interrupt scripts - 020907 akozar
else  NoteAlert(OKAlert,0L);
if(ComputeOn || PlaySelectionOn) SndSetSysBeepState(sysBeepEnable);
#if 0
AlertOn++;
if(!EmergencyExit && !InputOn && !AEventOn && !InitOn && !ItemCapture && !TickCapture
		&& EventState == NO) for(i=0; i < 5; i++) MainEvent();
AlertOn--;
#endif
if(!EmergencyExit && !InitOn) UpdateWindow(FALSE,Window[wGraphic]);
if(!EmergencyExit && !InitOn) UpdateWindow(FALSE,Window[wPrototype1]);
return(OK);
}

#endif /* BP_CARBON_GUI */

#if BP_CARBON_GUI

SetOptionMenu(int option)
{
if(option)
	sprintf(Message,"Open (any) file...");
else
	sprintf(Message,"Open file...");
c2pstrcpy(PascalLine, Message);
SetMenuItemText(myMenus[fileM],fmOpen,PascalLine);

if(option && CompiledGr && !IsEmpty(wGrammar))
	sprintf(Message,"Tokenized grammar");
else {
	strcpy(Message," ");	/* This avoids messing the menu as name starts with '-' */
	if(FileName[wGrammar][0] == '\0') strcpy(Message,WindowName[wGrammar]);
	else strcat(Message,FileName[wGrammar]);
	}
c2pstrcpy(PascalLine, Message);
SetMenuItemText(myMenus[windowM],grammarCommand,PascalLine);

if(option && CompiledAl && !IsEmpty(wAlphabet))
	sprintf(Message,"Tokenized alphabet");
else {
	strcpy(Message," ");	/* This avoids messing the menu as name starts with '-' */
	if(FileName[wAlphabet][0] == '\0') strcpy(Message,WindowName[wAlphabet]);
	else strcat(Message,FileName[wAlphabet]);
	}
c2pstrcpy(PascalLine, Message);
SetMenuItemText(myMenus[windowM],alphabetCommand,PascalLine);

if(option && CompiledGl && GlossGram.p_subgram != NULL)
	sprintf(Message,"Tokenized glossary");
else {
	strcpy(Message," ");	/* This avoids messing the menu as name starts with '-' */
	if(FileName[wGlossary][0] == '\0') strcpy(Message,WindowName[wGlossary]);
	else strcat(Message,FileName[wGlossary]);
	}
c2pstrcpy(PascalLine, Message);
SetMenuItemText(myMenus[windowM],glossaryCommand,PascalLine);

if (option)	SetMenuItemText(myMenus[windowM],miscsettingsCommand,"\pComputation & IO Settings");
else		SetMenuItemText(myMenus[windowM],miscsettingsCommand,"\pSettings");

// use Appearance Mgr features to set option-key shortcuts
if (HaveAppearanceManager) {
	SInt16 modifiers = (option ? kMenuOptionModifier : kMenuNoModifiers);
	
	SetMenuItemModifiers(myMenus[fileM], fmOpen, modifiers);
	SetMenuItemModifiers(myMenus[windowM], grammarCommand, modifiers);
	SetMenuItemModifiers(myMenus[windowM], alphabetCommand, modifiers);
	SetMenuItemModifiers(myMenus[windowM], miscsettingsCommand, modifiers);
	}

return(OK);
}

#endif /* BP_CARBON_GUI */

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


#if BP_CARBON_GUI

MoveLine(int w,int n,int select)
// Move cursor to beginning of line (n = -1) or end of line (n = +1)
{
char c,oldc;
long i,pos,posend,posmax;

if(w < 0 || w >= WMAX || !Editable[w]) return(FAILED);
if(n == 0) return(OK);
TextGetSelection(&pos, &posend, TEH[w]);
posmax = GetTextLength(w);
oldc = '\0';
if(n < 0) {
	i = pos - 1;
	if(i <= 0) i = 0;
	else {
		while(GetTextChar(w,i) != '\r') {
			if(i <= 0) break;
			else i--;
			}
		if(GetTextChar(w,i) == '\r') i++;
		}
	}
else {
	i = posend;
	while(GetTextChar(w,i) != '\r') {
		if(i >= posmax) break;
		else i++;
		}
/*	if((*((**(TEH[w])).hText))[i] == '\r') i++; */
	}
/* TEAutoView(FALSE,TEH[w]); */
if(!select) SetSelect(i,i,TEH[w]);
else  {
	if(i > pos) SetSelect(pos,i,TEH[w]);
	else SetSelect(i,posend,TEH[w]);
	}
ShowSelect(CENTRE,w);
return(OK);
}


MoveWord(int w,int n,int select,int option)
// Move cursor to previous word (n = -1) or next word (n = +1)
{
char c,oldc;
long i,pos,posend,posmax;

if(w < 0 || w >= WMAX || !Editable[w]) return(FAILED);
if(n == 0) return(OK);
TextGetSelection(&pos, &posend, TEH[w]);
posmax = GetTextLength(w);
if(n < 0) {
	i = pos - 1;
	if(i < 0) i = 0;
	else {
		if(option) {
			while(isspace(GetTextChar(w,i))) {
				if(i <= 0) break;
				else i--;
				}
			while(!isspace(GetTextChar(w,i))) {
				if(i <= 0) break;
				else i--;
				}
			if(isspace(GetTextChar(w,i))) i++;
			}
		}
	}
else {
	i = posend + 1;
	if(i >= posmax) i = posmax;
	else {
		if(option) {
			if(isspace(GetTextChar(w,i))) {
				while(isspace(GetTextChar(w,i))) {
					if(i >= posmax) break;
					else i++;
					}
				}
			else {
				while(!isspace(GetTextChar(w,i))) {
					if(i >= posmax) break;
					else i++;
					}
				while(isspace(GetTextChar(w,i))) {
					if(i >= posmax) break;
					else i++;
					}
				}
			}
		}
	}
/* TEAutoView(FALSE,TEH[w]); */
if(!select) SetSelect(i,i,TEH[w]);
else  {
	if(i > pos) SetSelect(pos,i,TEH[w]);
	else SetSelect(i,posend,TEH[w]);
	}
if(n < 0) ShowSelect(UP,w);
else ShowSelect(DOWN,w);
return(OK);
}


MovePage(int w,int n)
// Move cursor n pages down
{
long pos,posend,length,linelength,pagejump;

length = GetTextLength(w);
linelength = length / LinesInText(w);	/* This is an approximation */
pagejump = (linelength * linesInFolder[w] * 2L) / 3L;
TextGetSelection(&pos, &posend, TEH[w]);
pos += (n * pagejump);
if(pos < ZERO) pos = ZERO;
if(pos > length) pos = length;

SetSelect(pos,pos,TEH[w]);
if(n < 0) ShowSelect(UP,w);
else ShowSelect(DOWN,w);

return(OK);
}


MoveParagraph(int w,int n,int select)
// Move cursor n paragraphs down
{
char c,oldc;
long i,pos,posend,posmax;
int dir;

if(w < 0 || w >= WMAX || !Editable[w]) return(FAILED);
if(n == 0) return(OK);
dir = n;
TextGetSelection(&pos, &posend, TEH[w]);
i = pos;
posmax = GetTextLength(w);
oldc = '\0';
if(n < 0) {
	if(--i < 0) i = 0;
	else {
		while(GetTextChar(w,i) == '\r') i--;
		for(i=i; i > 0; i--) {
			c = GetTextChar(w,i);
			if(oldc == '\r' && c != 'Â') n++;
			if(n == 0) {
				i += 2;
				break;
				}
			oldc = c;
			}
		}
	}
else {
	for(i=posend; i <= posmax; i++) {
		c = GetTextChar(w,i);
		if(c == '\r' && oldc != 'Â') n--;
		if(n == 0) {
			i++;
			while(GetTextChar(w,i) == '\r') i++;
			break;
			}
		oldc = c;
		}
	}
/* TEAutoView(FALSE,TEH[w]); */
if(!select) SetSelect(i,i,TEH[w]);
else  {
	if(i > pos) SetSelect(pos,i,TEH[w]);
	else SetSelect(i,posend,TEH[w]);
	}
if(dir < 0) ShowSelect(UP,w);
else ShowSelect(DOWN,w);
return(OK);
}

#endif /* BP_CARBON_GUI */

SelectBehind(long pos1,long pos2,TextHandle teh)
/* Doesn't force selection to scroll */
{
#if !USE_MLTE
	long maxoffset;
	/* clamp range to text bounds (needed for WASTE since we are bypassing its checks) */
	maxoffset = GetTextHandleLength(teh);
	if (pos1 < ZERO) {
		if(Beta) Alert1("Err. SelectBehind(). pos1 < ZERO");
		pos1 = ZERO;
	}
	else if (pos1 > maxoffset) {
		if(Beta) Alert1("Err. SelectBehind(). pos1 > maxoffset");
		pos1 = maxoffset;
	}
	if (pos2 < ZERO) {
		if(Beta) Alert1("Err. SelectBehind(). pos2 < ZERO");
		pos2 = ZERO;
	}
	else if (pos2 > maxoffset) {
		if(Beta) Alert1("Err. SelectBehind(). pos2 > maxoffset");
		pos2 = maxoffset;
	}
	(*teh)->selStart = pos1; (*teh)->selEnd = pos2;
#else // FIXME: how do we do this with MLTE ?
if (Beta) printf("Err.  SelectBehind() not implemented for MLTE!\n");
#endif
return(OK);
}


TextDeleteBehind(int w)
{
Deactivate(TEH[w]);
TextDelete(w);
return(OK);
}

#if BP_CARBON_GUI

SetResumeStop(int force)
{
if((!AlertOn && !ButtonOn) || force) {
	if(!ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn &&
			!SetTimeOn && !GraphicOn && !PrintOn /* && !ReadKeyBoardOn */
			&& !HangOn && !ScriptExecOn && !WaitOn) {
		if(ResumeStopOn || force) {
			ResumeStopOn = FALSE;
			HideWindow(GetDialogWindow(ResumeUndoStopPtr));
			HideWindow(GetDialogWindow(ResumeStopPtr));
			}
		}
	else {
		if(!ResumeStopOn || force) {
			ResumeStopOn = TRUE;
			if(UndoFlag) {
				HideWindow(GetDialogWindow(ResumeStopPtr));
				ShowWindow(GetDialogWindow(ResumeUndoStopPtr));
				BringToFront(GetDialogWindow(ResumeUndoStopPtr));
				}
			else {
				HideWindow(GetDialogWindow(ResumeUndoStopPtr));
				ShowWindow(GetDialogWindow(ResumeStopPtr));
				BringToFront(GetDialogWindow(ResumeStopPtr));
				}
			}
		}
	}
return(OK);
}


ChangeNames(char **p_line)
// p_line contains a list of names that must be applied to the current files
// This is mainly used by Apple Events
{
int r,i,i0,im,j,w;
char c,line[MAXNAME+1],*p,*q;

if(p_line == NULL) {
	if(Beta) Alert1("Err. ChangeNames(). p_line == NULL");
	return(OK);
	}
if((*p_line)[0] == '\0') return(OK);

r = OK;
i = 0; im = MyHandleLen(p_line);
if(p_line == NULL) {
	if(Beta) Alert1("Err. ChangeNames(). p_line == NULL");
	return(OK);
	}
while(TRUE) {
	// names are separated by spaces or returns (which excludes setting a name containing a space)
	while(i < im && (isspace(c=(*p_line)[i]) || c == '\0')) i++;
	i0 = i;
	if(i >= im) break;
	j=0;
	while(i < im && !isspace(c=(*p_line)[i]) && c != '\0') {
		line[j] = c;
		i++; j++;
		if(j >= MAXNAME) {
			line[j] = '\0';
			sprintf(Message,"\nToo long name: %s... [max %ld chars]\n", line,(long)MAXNAME);
			Print(wTrace,Message);
			return(FAILED);
			}
		}
	line[j] = '\0';
	if(j == 0) continue;
	p = &line[0]; q = &(FilePrefix[wGrammar][0]);
	if(Match(TRUE,p_line,&q,4)) {
		w = wGrammar; goto FIX;
		}
	p = &line[0]; q = &(FilePrefix[wAlphabet][0]);
	if(Match(TRUE,p_line,&q,4)) {
		w = wAlphabet; goto FIX;
		}
	p = &line[0]; q = &(FilePrefix[wData][0]);
	if(Match(TRUE,p_line,&q,4)) {
		w = wData; goto FIX;
		}
	p = &line[0]; q = &(FilePrefix[wInteraction][0]);
	if(Match(TRUE,p_line,&q,4)) {
		w = wInteraction; goto FIX;
		}
	p = &line[0]; q = &(FilePrefix[wGlossary][0]);
	if(Match(TRUE,p_line,&q,4)) {
		w = wGlossary; goto FIX;
		}
	p = &line[0]; q = &(FilePrefix[iSettings][0]);
	if(Match(TRUE,p_line,&q,4)) {
		w = iSettings; goto FIX;
		}
	p = &line[0]; q = &(FilePrefix[wKeyboard][0]);
	if(Match(TRUE,p_line,&q,4)) {
		w = wKeyboard; goto FIX;
		}
	p = &line[0]; q = &(FilePrefix[wTimeBase][0]);
	if(Match(TRUE,p_line,&q,4)) {
		w = wTimeBase; goto FIX;
		}
	p = &line[0]; q = &(FilePrefix[wScript][0]);
	if(Match(TRUE,p_line,&q,4)) {
		w = wScript; goto FIX;
		}
	p = &line[0]; q = &(FilePrefix[iObjects][0]);
	if(Match(TRUE,p_line,&q,4)) {
		w = iObjects; goto FIX;
		}
	p = &line[0]; q = &(FilePrefix[wCsoundInstruments][0]);
	if(Match(TRUE,p_line,&q,4)) {
		w = wCsoundInstruments; goto FIX;
		}
	p = &line[0]; q = &(FilePrefix[wMIDIorchestra][0]);
	if(Match(TRUE,p_line,&q,4)) {
		w = wMIDIorchestra; goto FIX;
		}
	sprintf(Message,"\nIncorrect name: %s...\n",line);
	Print(wTrace,Message);
	r = FAILED;
	continue;
FIX:
	strcpy(FileName[w],line);
	SetName(w,TRUE,FALSE);
	}
return(r);
}

#endif /* BP_CARBON_GUI */

FindGoodIndex(int wind)
{
if(wind < 0 || wind >= WMAX) {
	if(Beta) Alert1("Err. FindGoodIndex(). Incorrect index");
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
		return(wCsoundInstruments);
		break;
	}
return(wind);
}

#if BP_CARBON_GUI

MyButton(int quick)
{
// quick = 0 --> process any event (slowest)
// quick = 1 --> process any event except activate. This notably includes update events.
// quick = 2 --> process only mouseclicks

EventRecord event;
int w,eventfound,theclick,what,mousedown,result,intext,hit;
short itemHit,itemtype;
long sleeptime;
WindowPtr whichwindow;
DialogPtr thedialog;
GrafPtr saveport;
OSErr memerr;

if(LoadOn) return(FAILED);

mousedown = Button();
if(mousedown) goto DOEVENT;

if(quick >= 2) return(FAILED);

DOEVENT:
MaintainMenus();
if ((ComputeOn || CompileOn) && !WaitOn)  sleeptime = 1L;
else  sleeptime = GetCaretTime();
eventfound = WaitNextEvent(everyEvent,&event, sleeptime, NULL);
what = event.what;
if(!eventfound || (what != mouseDown
		&& (what != keyDown || ((event.modifiers & cmdKey) == 0))
		&& what != updateEvt && what != activateEvt
		&& what != kHighLevelEvent)) return(FAILED);
if(quick == 1 && what == activateEvt) return(FAILED);
GetPort(&saveport);
if(what == mouseDown) {
	theclick = FindWindow(event.where,&whichwindow);
	for(w=0; w < WMAX; w++) {
		if(whichwindow == Window[w]) break;
		}	
	if(whichwindow != NULL) {
		switch(theclick) {
			case inDrag:
			case inGrow:
				BringToFront(whichwindow);
				break;
			case inContent:
				result = OK;
				intext = FALSE;
				if(w < WMAX && w != Nw && w != wScript) {
					BPActivateWindow(QUICK,w);
					}
				else {
					result = OK;
			/*		if(IsDialogEvent(&event)) result = DoDialog(&event); */
			/*		hit = DialogSelect(&event,&thedialog,&itemHit);
					if(hit && (w == wControlPannel)) { 
						switch(itemHit) {
							case bMute:
								if(!Oms && !NEWTIMER)
									Alert1("'Mute' only works when Opcode OMS is active");
								else {
									Mute = 1 - Mute;
									MaintainMenus();
									}
								result = DONE;
								break;
							}
						} */
					if(result != DONE) DoContent(whichwindow,&event,&intext);
					}
				result = intext;
				if(result == OK) PauseOn = TRUE;
				if(saveport != NULL) SetPort(saveport);
				else if(Beta) Alert1("Err MyButton(). saveport == NULL");
				return(result);	/* This causes interruption if result == TRUE */
				break;
			default: break;
			}
		}
	}
ButtonOn = TRUE;
result = DoEvent(&event);	/* This may send BP2 to background */
ButtonOn = FALSE;
if(EventState != EXIT && EventState != ABORT) EventState = NO;
else result = EventState;
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err MyButton(). saveport == NULL");
DoSystem();

/* We already processed the click, so we return FAILED */
if(result == OK) return(FAILED);
else return(result);
}


TellError(int thecase,OSErr oserr)
{
char line[MAXLIN];
int r;

r = OK;
if(oserr != noErr) {
	if(!InitOn) {
		sprintf(Message,"\nMac error %ld (case %ld)",(long)oserr,(long)thecase);
		if(TraceRefnum > -1) WriteToFile(NO,MAC,Message,TraceRefnum);
		if(TempRefnum > -1) WriteToFile(NO,MAC,Message,TempRefnum);
		}
	r = ABORT;
	switch(oserr) {
		case procNotFound: sprintf(line,"Process not found");
			break;
		case paramErr: sprintf(line,"paramErr: attempting to open folder instead of file, or 'inOutCount' was negative, or no default volume, or process serial number is invalid");
			break;
		case badUnitErr: sprintf(line,"'badUnitErr' error: refNum doesn't match unit table");
			break;
		case unitEmptyErr: sprintf(line,"'UnitEmptyErr' error: refNum specifies NIL handle in unit table");
			break;
		case notOpenErr: sprintf(line,"'notOpenErr' error: driver is closed");
			break;
		case readErr: sprintf(line,"'readErr' error: driver can't respond to Read");
			break;
		case posErr: sprintf(line,"'posErr' error: attempt to position before start of file");
			break;
		case bdNamErr: sprintf(line,"Incorrect file name");
			break;
		case fnfErr: sprintf(line,"File was not found");
			break;
		case fnOpnErr: sprintf(line,"File was not open");
			break;
		case ioErr: sprintf(line,"Input/output error");
			break;
		case eofErr: sprintf(line,"End of file error");
			break;
		case rfNumErr: sprintf(line,"Bad file reference number");
			break;
		case extFSErr: sprintf(line,"External file system");
			break;
		case nsvErr: sprintf(line,"No such volume");
			break;
		case opWrErr: sprintf(line,"File already open for writing");
			break;
		case tmfoErr: sprintf(line,"Too many files open");
			break;
		case dskFulErr:
			strcpy(line,"Disk full"); break;
		case fLckdErr:
			strcpy(line,"File is locked"); break;
		case vLckdErr:
			strcpy(line,"Volume is locked"); break;
		case wPrErr:
			strcpy(line,"Disk is write-protected"); break;
		case wrPermErr:
			strcpy(line,"Write permission denied"); break;
		case errAEDescNotFound: /* -1701 */
			strcpy(line,"Unexpected error: 'Apple Event descriptor not found'."); break;
		case errAECorruptData:	/* -1702 */
			strcpy(line,"Unexpected error: 'Data in an Apple Event could not be read'."); break;
		case errAENotAppleEvent:	/* -1707 */
			strcpy(line,"Unexpected error: 'Event is not Apple Event'."); break;
		case errAEReplyNotValid:	/* -1709 */
			strcpy(line,"Unexpected error: 'AEResetTimer was passed an invalid reply parameter'."); break;
		case errAEHandlerNotFound:	/* -1717 */
			strcpy(line,"Unexpected error: 'no handler in the dispatch tables fits the parameters to AEGetEventHandler or AEGetCoercionHandler'."); break;
		case errAEEventNotHandled:	/* -1708 */
			strcpy(line,"Unexpected error: 'Event was not handled by an Apple Event handler'."); break;
		case errAENewerVersion:
			strcpy(line,"Version of Apple Event Manager is too old. Event couldn't be handled."); break;
		case errAEParamMissed:	/* -1715 */
			strcpy(line,"Unexpected error: 'a required parameter was not accessed' in Apple Event handling."); break;
		case connectionInvalid:
			strcpy(line,"Apple Event error: invalid connection."); break;
		case errAETimeout:
			strcpy(line,"Apple Event error timed out."); break;
		case errAEUnknownAddressType:
			strcpy(line,"Unknown Apple Event address type."); break;
		case errAEUnknownSendMode:
			strcpy(line,"Unknown Apple Event sending mode."); break;
		case nilHandleErr:	/* -109 */
		/*	return(OK);  $$$ Discarding for the time being */
			strcpy(line,"Unexpected error: 'illegal operation on a NIL handle'"); break;
		case memWZErr:	/* -111 */
			strcpy(line,"Unexpected error: 'illegal operation on a free block'"); break;
		case memPurErr: strcpy(line,"Unexpected error: 'illegal operation on a locked block'"); break;
		case memFullErr:	/* -108 */
			EmergencyExit = TRUE;
			strcpy(line,"Unexpected 'memory full' error... You should resize 'BP2'."); break;
		case memAdrErr:	/* -110 */
			strcpy(line,"Unexpected error: 'memAdrErr'. Ignored..."); break;
		case memAZErr:	/* -113 */
			strcpy(line,"Unexpected error: 'memAZErr'. Ignored..."); break;
		case memPCErr:	/* -114 */
			strcpy(line,"Unexpected error: 'memPCErr'. Ignored..."); break;
		case memBCErr:	/* -115 */
			strcpy(line,"Unexpected error: 'memBCErr'. Ignored..."); break;
		case memSCErr:	/* -116 */
			strcpy(line,"Unexpected error: 'memSCErr'. Ignored..."); break;
		case memLockedErr:	/* -117 */
			strcpy(line,"Unexpected error: 'memLockedErr'. Ignored..."); break;
		case -919:
			strcpy(line,"Unexpected error: 'PPC record is invalid'"); break;
		default:
			sprintf(line,"Unknown OS error #%ld. Ignored...",(long)oserr);
			r = FAILED;
		}
	if(!InitOn) {
		if(TraceRefnum > -1) WriteToFile(NO,MAC,line,TraceRefnum);
		if(TempRefnum > -1) WriteToFile(NO,MAC,line,TempRefnum);
//		FlushVolume();
		if(Beta) Println(wTrace,line);
		ShowMessage(TRUE,wMessage,line);
		}
	}
return(r);
}

#endif /* BP_CARBON_GUI */

#if 0
GetString1(char *s)
{
char c;
int i = 0;  
EventRecord theEvent;
do {
	while(!WaitNextEvent(everyEvent,&theEvent,60L,NULL) || ((theEvent.what != keyDown)
		&& (theEvent.what != autoKey))) ;
  	c = (char) (theEvent.message & charCodeMask);
/*  	Print(wTrace,c); */
   	s[i++] = c;
	}
while(c != '\r' && i < 79); 
s[i] = '\0'; 
return(OK); 
}
#endif /* 0 */

/*
LockRule(igram,irul)
int igram,irul;
{
return(OK);
MyLock(FALSE,(Handle)(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftarg);
MyLock(FALSE,(Handle)(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightarg);
if((*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightflag
	!= NULL) MyLock(FALSE,(Handle)(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightflag);
if((*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftcontext
	!= NULL) {
	MyLock(FALSE,(Handle)(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftcontext);
	MyLock(FALSE,(Handle)(*((*((*(Gram.p_subgram))[igram].p_rule))[irul]
											.p_leftcontext))->p_arg);
	}
if((*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightcontext
	!= NULL) {
	MyLock(FALSE,(Handle)(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightcontext);
	MyLock(FALSE,(Handle)(*((*((*(Gram.p_subgram))[igram].p_rule))[irul]
											.p_rightcontext))->p_arg);
	}
return(OK);
}
*/

/*
UnlockRule(igram,irul)
int igram,irul;
{
MyUnlock((Handle)(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftarg);
MyUnlock((Handle)(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightarg);
if((*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightflag
	!= NULL) MyUnlock((Handle)(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightflag);
if((*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftcontext
	!= NULL) {
	MyUnlock((Handle)(*((*((*(Gram.p_subgram))[igram].p_rule))[irul]
												.p_leftcontext))->p_arg);
	MyUnlock((Handle)(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_leftcontext);
	}
if((*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightcontext
	!= NULL) {
	MyUnlock((Handle)(*((*((*(Gram.p_subgram))[igram].p_rule))[irul]
												.p_rightcontext))->p_arg);
	MyUnlock((Handle)(*((*(Gram.p_subgram))[igram].p_rule))[irul].p_rightcontext);
	}
return(OK);
}
*/

#if BP_CARBON_GUI

CheckSettings(void)
{
int rep;

rep = OK;
if(!ScriptExecOn && Dirty[iSettings]) {
	rep = Answer("Save settings",'Y');
	if(rep == ABORT) return(rep);
	if(rep == OK) {
		rep = mSaveSettingsAs(wUnknown);
		if(rep != OK) return(rep);
		}
	// Dirty[iSettings] = FALSE; // not true if user pressed 'No' - akozar 061107
	}
return(rep);
}


SaveCheck(int w)
{
int wmem,vref,rep,result,dirtymem;

result = OK;
dirtymem = Dirty[w];
wmem = w;
switch(w) {
	case wAlphabet:
		if(!Dirty[w] || ScriptExecOn || AEventOn) return(OK);
		GetMiName(); GetKbName(w); GetCsName(w);
		GetFileNameAndLoadIt(wMIDIorchestra,w,LoadMIDIorchestra);
		break;
	case wGrammar:
	case wData:
		if(!Dirty[w] || ScriptExecOn || AEventOn) return(OK);
		GetInName(w); GetGlName(w); GetCsName(w);
		GetFileNameAndLoadIt(wMIDIorchestra,w,LoadMIDIorchestra);
		break;
	case wPrototype1:
	case wPrototype2:
	case wPrototype3:
	case wPrototype4:
	case wPrototype5:
	case wPrototype6:
	case wPrototype8:
		dirtymem = (dirtymem || Dirty[iObjects]);
		Dirty[w] = FALSE;
		w = iObjects;
		if(iProto == 0 || Jbol < 3) dirtymem = FALSE;
		break;
	case wMetronom:
		return(OK); break;
	}
Dirty[w] = dirtymem;
if(!Dirty[w] || ScriptExecOn || AEventOn) return(OK);
if(NeedSave[w] && !IsEmpty(w)) {
	StopWait();
	if(FileName[w][0] == '\0')
		sprintf(Message,"Save changes for \"%s\"",DeftName[w]);
	else
		sprintf(Message,"Save changes for \"%s\"",FileName[w]);
	rep = Answer(Message,'Y');
	switch(rep) {
		case YES:
			result = mSaveFile(w);
			Dirty[w] = Dirty[wmem] = FALSE;
			break;
	 	case ABORT: {
	 		Dirty[wmem] = dirtymem;
	 		return(ABORT);
	 		}
	 	case NO: 
			Dirty[w] = Dirty[wmem] = FALSE;
			break;
	 	}
	}
return(result);
}

#endif /* BP_CARBON_GUI */

CompileCheck(void)
{
int r,compiledgl;

r = OK;
compiledgl = CompiledGl;
if(CheckEmergency() != OK) return(ABORT);
if(!CompiledGr && (AddBolsInGrammar() > BolsInGrammar)) CompiledAl = FALSE;
if(!CompiledGr || !CompiledAl) {
/*	KillSubTree(PrefixTree); KillSubTree(SuffixTree); $$$ */
	if((r=CompileGrammar(1)) != OK) {
		if(r == FAILED && CompiledGr && !CompiledAl) r = CompileAlphabet();
		if(CompiledGr && CompiledAl) r = OK;
		if(r != OK) return(r);
		}
	if(ResetWeights && (Varweight = ResetRuleWeights(0)) == ABORT) {
		if(Beta) Alert1("Can't fix bug in grammar code. Unexpected error");
		return(FAILED);
		}
	}
CompiledGl = compiledgl;
if(ObjectMode && ((r=CompileCsoundObjects()) != OK)) return(r);
#if BP_CARBON_GUI
if(LoadedIn && (!CompiledIn && (r=CompileInteraction()) != OK)) return(r);
#endif /* BP_CARBON_GUI */
if((r=UpdateGlossary()) != OK) return(r);
if(!CompiledPt) if((r=CompilePatterns()) != OK) return(r);
if(SmartCursor) r = UpdateAutomata();
return(r);
}

// -----------------------  FIND - REPLACE ------------------------

#if BP_CARBON_GUI

FindReplace(int all)
{
int i=0,done=FALSE,redo=FALSE,start=TRUE,rep;
Rect r;
GrafPtr saveport;

while(Button());
if(all) Deactivate(TEH[TargetWindow]);
do {
	if(GetNextOccurrence(&redo,start) != OK) {
		done = TRUE;
		break;
		}
	start = FALSE;
	if(all) {
		if(Button()) {
			if(Answer("Continue replacing",'N') != OK) break;
			else Deactivate(TEH[TargetWindow]);
			}
		Replace();
		i++;
		continue;
		}
	GetPort(&saveport);
	SetPortWindowPort(Window[TargetWindow]);
	GetWindowPortBounds(Window[TargetWindow], &r);
	InvalWindowRect(Window[TargetWindow], &r);
	BPActivateWindow(SLOW,TargetWindow);
	ShowSelect(CENTRE,TargetWindow);
	if(saveport != NULL) SetPort(saveport);
	else if(Beta) Alert1("Err FindReplace(). saveport == NULL");
	rep = GetReplaceCommand();
	Deactivate(TEH[TargetWindow]);
	switch(rep) {
		case dReplace:
			Replace();
			i++;
			break;
		case dStopReplace:
			Finding = (strlen(FindString) > 0);
			goto QUIT;
		case dDontChange:
			break;
		}
	}
while(!done);

QUIT:
Activate(TEH[TargetWindow]);
BPActivateWindow(SLOW,TargetWindow);
sprintf(Message,"%ld occurrence(s) found...",(long)i);
ShowMessage(TRUE,wMessage,Message);
return(OK);
}


GetNextOccurrence(int *p_redo,int start)
{
int i,j,k,w,jmax,rep;
long dummy,pos,posmax;

if((w = TargetWindow) < 0 || w >= WMAX || !Editable[w]) return(ABORT);
rep = FAILED;
TextGetSelection(&dummy, &pos, TEH[w]);
posmax = GetTextLength(w);
jmax = strlen(FindString); if(jmax == 0) return(ABORT);

START:
if(pos == ZERO) (*p_redo) = TRUE;
for(pos=pos; pos < posmax; pos++) {
	k = 0;
	if(MatchWords && pos > 0) {
		if(isalpha(GetTextChar(w,pos)) || isdigit(GetTextChar(w,pos))) goto NEXT;
		k = 1;
		}
	for(i=pos+k,j=0; i < posmax, j < jmax; i++,j++) {
		if(GetTextChar(w,i) != FindString[j]) {
			if(!IgnoreCase) goto NEXT;
			if(UpperCase(GetTextChar(w,i))
							!= UpperCase(FindString[j])) goto NEXT;
			}
		}
	if(MatchWords && ((pos+j+k) < posmax)) {
		if(isalpha(GetTextChar(w,pos+j+k))
			|| isdigit(GetTextChar(w,pos+j+k))) goto NEXT;
		}
	SelectBehind((long)(pos+k),(long)(pos+j+k),TEH[w]);
	return(OK);
NEXT:
	continue;
	}
if(!(*p_redo)) {
	if(Answer("Continue search from beginning",'N') != OK) return(ABORT);
	pos = ZERO;
	goto START;
	}
else {
	if(!ScriptExecOn) Alert1("End of document reached");
	return(ABORT);
	}
return(rep);
}


Replace(void)
{
long count;

// FIXME: should make temp copy of the deleted text in case insertion fails
TextDelete(TargetWindow);
count = (long) strlen(ReplaceString);
if (TextInsert(ReplaceString,count,TEH[TargetWindow]) != OK) {
	Alert1("The text replacement failed; probably because the window is too full.");
	return(FAILED);
	}
UpdateDirty(TRUE,TargetWindow);
LastEditWindow = TargetWindow;
return(OK);
}


GetReplaceCommand(void)
{
Rect r;
short theitem;
int found;
ControlHandle h;
EventRecord theEvent;
WindowPtr thewindow;
DialogPtr thedialog;
Point pt;

ShowWindow(GetDialogWindow(ReplaceCommandPtr));
SelectWindow(GetDialogWindow(ReplaceCommandPtr));
SetPortDialogPort(ReplaceCommandPtr);
StopWait();
found = FALSE;

// #ifndef __POWERPC
FlushEvents(everyEvent,0);
// #endif

/* DrawControls(ReplaceCommandPtr); */
BPUpdateDialog(ReplaceCommandPtr);

do {
	if(WaitNextEvent(everyEvent,&theEvent,60L,NULL)) {
		if((theEvent.what == keyDown)
			&& ((char)(theEvent.message & charCodeMask) == '\r')) {
			theitem = dDontChange;
			break;
			}
		if(theEvent.what != mouseDown) continue;
		found = TRUE;
		FindWindow(theEvent.where,&thewindow);
		GlobalToLocal(&(theEvent.where));
		if(FindControl(theEvent.where, GetDialogWindow(ReplaceCommandPtr), &h) != 0) {
			TrackControl(h,theEvent.where,MINUSPROC);
			LocalToGlobal(&(theEvent.where));
			thedialog = GetDialogFromWindow(thewindow);	// FIXME ? not sure what window(s) are being used here?
			DialogSelect(&theEvent,&thedialog,&theitem);
			}
		else {
			if(thewindow == GetDialogWindow(ReplaceCommandPtr)) {
				SysBeep(10);
				found = FALSE;
				}
			theitem = dStopReplace;
			}
		}
	}
while(!found);
HideWindow(GetDialogWindow(ReplaceCommandPtr));

// #ifndef __POWERPC
FlushEvents(everyEvent,0);
// #endif

return(theitem);
}


SetFindReplace(void)
{
Rect r;
ControlHandle itemhandle;
short itemtype;
char line[256];
long p,q;

SetField(NULL, wFindReplace, fFind, FindString);
SetField(NULL, wFindReplace, fReplace, ReplaceString);
GetDialogItem(gpDialogs[wFindReplace],dIgnoreCase,&itemtype,
	(Handle*)&itemhandle,&r);
SetControlValue(itemhandle,IgnoreCase);
GetDialogItem(gpDialogs[wFindReplace],dMatchWords,&itemtype,
	(Handle*)&itemhandle,&r);
SetControlValue(itemhandle,MatchWords);
return(OK);
}


GetFindReplace(void)
{
Rect r;
Handle itemhandle;
short itemtype;
int i,j,reply;
Str255 t;

/* if(!Dirty[wFindReplace]) return(OK); */
GetDialogItem(gpDialogs[wFindReplace],fFind,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,FindString);
ConvertSpecialChars(FindString);
Finding = TRUE; if(strlen(FindString) == 0) Finding = FALSE;
GetDialogItem(gpDialogs[wFindReplace],fReplace,&itemtype,&itemhandle,
	&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,ReplaceString);
ConvertSpecialChars(ReplaceString);
Dirty[wFindReplace] = FALSE;
return(OK);
}

#endif /* BP_CARBON_GUI */

// -----------------  DIALOG ROUTINES ---------------------------------

#if BP_CARBON_GUI

SetField(DialogPtr ptr,int w,int ifield,char* string)
{
OSErr err;
Rect r;
Handle itemhandle;
short itemtype;
Str255 line;
DialogPtr thedialog;
// DialogRecord dr;

if(ptr != NULL) {
	thedialog = ptr; w = -1;
	}
else {
	if(w < 0 || w >= WMAX || !HasFields[w]) {
		if(Beta) Alert1("Err. SetField(). Incorrect index");
		return(FAILED);
		}
	thedialog = gpDialogs[w];
	}

#if !USE_OLD_EDIT_TEXT
  // must get edit text handle this way when it is a real control
  err = GetDialogItemAsControl(thedialog, (short)ifield, (ControlRef*)&itemhandle);
  if (err != noErr) return(ABORT);
#else
GetDialogItem(thedialog,(short)ifield,&itemtype,&itemhandle,&r);
if(((itemtype & 127)  != editText && (itemtype & 127)  != statText)
		|| itemhandle == NULL) {
	if(Beta) {
		sprintf(Message,"Err SetField(%ld,%ld,%s)",(long)w,
			(long)ifield,(long)string);
		Alert1(Message);
		}
	return(ABORT);
	}
#endif
if (strlen(string) > 255) {
	if (Beta)  Alert1("Err SetField(): string is too long");
	return (ABORT);
	}
c2pstrcpy(line,string);
SetDialogItemText(itemhandle,line);
// SelectDialogItemText(thedialog,ifield,0,0); // don't change keyboard focus - akozar 052107
// if((itemtype & 127) == statText) TEDeactivate(GetDialogTextEditHandle(thedialog)); // we should not mess with Dialog Manager's state? - akozar 051707
return(DoSystem());
}


GetField(DialogPtr ptr,int forcenum,int w,int ifield,char *line,long *p_p,long *p_q)
{
Rect r;
Handle itemhandle;
short itemtype;
Str255 t;
int i,rep,neg;
double pp,qq;
DialogPtr thedialog;

if(ptr != NULL) {
	thedialog = ptr; w = -1;
	}
else {
	if(w < 0 || w >= WMAX || !HasFields[w]) {
		if(Beta) Alert1("Err. GetField(). Incorrect index");
		return(FAILED);
		}
	thedialog = gpDialogs[w];
	}
	
*p_p = ZERO; *p_q = 1L;
GetDialogItem(thedialog,(short)ifield,&itemtype,&itemhandle,&r);
if(((itemtype & 127) != editText && (itemtype & 127)  != statText)
		|| itemhandle == NULL) {
	if(Beta) {
		sprintf(Message,"Err GetField(%ld,%ld)",(long)w,(long)ifield);
		Alert1(Message);
		}
	return(FAILED);
	}
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,line);
Strip(line);
for(i=0; i < strlen(line); i++) {
	if(isspace(line[i])) line[i] = ' ';
	}
Strip(line);
if(forcenum && line[0] == '\0') return(FAILED);
if(Myatof(line,p_p,p_q) < Infneg) {
	if(forcenum && line[0] != '\0' && line[0] != '[') {
		ShowWindow(GetDialogWindow(thedialog));
		BringToFront(GetDialogWindow(thedialog));
		Alert1("Incorrect numerical value");
		SetField(thedialog,-1,ifield,"[?]");
		SelectField(thedialog,-1,ifield,TRUE);
		}
	return(FAILED);
	}
neg = 1;
if(*p_p < ZERO) {
	neg = - neg; *p_p = - *p_p;
	}
if(*p_q < ZERO) {
	neg = - neg; *p_q = - *p_q;
	}
pp = (double) *p_p;
qq = (double) *p_q;
rep = Simplify((double)INT_MAX,pp,qq,&pp,&qq);
*p_p = (long) pp;
*p_q = (long) qq;
if(neg < 0) *p_p = - *p_p;
return(rep);
}


GetValue(DialogPtr ptr,int w,int j,int field,double **p_v[],int i)
{
long p,q;
char line[MAXFIELDCONTENT];

if(GetField(ptr,TRUE,w,field,line,&p,&q) == OK) {
	(*(p_v[i]))[j] = ((double)p)/q;
	}
else {
	(*(p_v[i]))[j] = Infpos1;
/*	if(w >=0 && w < WMAX && line[0] != '\0' && line[0] != '[') {
		ShowWindow(Window[w]);
		BringToFront(Window[w]);
		SetField(ptr,w,field,"[?]");
		SelectField(ptr,w,field,TRUE);
		Alert1("Incorrect numerical value");
		}  */
	}
return(OK);
}


short GetCtrlValue(int w,int icontrol)
{
Rect r;
ControlHandle itemhandle;
short itemtype;
Str255 t;
long p,q;

if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. GetCtrlValue(). Incorrect index");
	return(0);
	}
GetDialogItem(gpDialogs[w],(short)icontrol,&itemtype,(Handle*)&itemhandle,&r);
if((((itemtype & 127) != (ctrlItem+radCtrl)) && ((itemtype & 127) != (ctrlItem+chkCtrl)))
	|| itemhandle == NULL) {
	if(Beta) {
		sprintf(Message,"Err GetCtrlValue(%ld,%ld)",(long)w,(long)icontrol);
		Alert1(Message);
		}
	return(0);
	}
return(GetControlValue(itemhandle));
}


ToggleButton(int w,int icontrol)
{
Rect r;
ControlHandle itemhandle;
short itemtype;

if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. ToggleButton(). Incorrect index");
	return(FAILED);
	}
GetDialogItem(gpDialogs[w],(short)icontrol,&itemtype,(Handle*)&itemhandle,&r);
if((((itemtype & 127) != (ctrlItem+radCtrl)) && ((itemtype & 127) != (ctrlItem+chkCtrl)))
	|| itemhandle == NULL) {
	if(Beta)  {
		sprintf(Message,"Err ToggleButton(%ld,%ld)",(long)w,(long)icontrol);
		Alert1(Message);
		}
	return(FAILED);
	}
SetControlValue(itemhandle,1 - (GetControlValue(itemhandle) > 0));
return(DoSystem());
}


SwitchOnOff(DialogPtr ptr,int w,int i,int state)
{
if(state == TRUE) return(SwitchOn(ptr,w,i));
else return(SwitchOff(ptr,w,i));
}


SwitchOn(DialogPtr ptr,int w,int i)
{
Rect r;
short itemtype;
Handle itemhandle;
DialogPtr thedialog;

if(ptr != NULL) {
	thedialog = ptr; w = -1;
	}
else {
	if(w < 0 || w >= WMAX) {
		if(Beta) Alert1("Err. SwitchOn(). Incorrect index");
		return(FAILED);
		}
	thedialog = gpDialogs[w];
	}
GetDialogItem(thedialog,(short)i,&itemtype,&itemhandle,&r);
itemtype = (itemtype & 127) - ctrlItem;
if(itemtype != radCtrl && itemtype != chkCtrl && itemtype != btnCtrl
		|| itemhandle == NULL) {
	if(Beta) {
		sprintf(Message,"Err SwitchOn(NULL,%ld,%ld)",(long)w,(long)i);
		Alert1(Message);
		}
	return(ABORT);
	}
if(itemtype == btnCtrl)
	HiliteControl((ControlHandle) itemhandle,kControlButtonPart);
else
	SetControlValue((ControlHandle) itemhandle,1);
return(OK);
}


SwitchOff(DialogPtr ptr,int w,int i)
{
Rect r;
short itemtype;
Handle itemhandle;
DialogPtr thedialog;

if(ptr != NULL) {
	thedialog = ptr; w = -1;
	}
else {
	if(w < 0 || w >= WMAX) {
		if(Beta) Alert1("Err. SwitchOff(). Incorrect index");
		return(FAILED);
		}
	thedialog = gpDialogs[w];
	}
GetDialogItem(thedialog,(short)i,&itemtype,&itemhandle,&r);
itemtype = (itemtype & 127) - ctrlItem;
if(itemtype != radCtrl && itemtype != chkCtrl && itemtype != btnCtrl
		|| itemhandle == NULL) {
	if(Beta) {
		sprintf(Message,"Err SwitchOff(%ld,%ld)",(long)w,(long)i);
		Alert1(Message);
		}
	return(ABORT);
	}
if(itemtype == btnCtrl)
	HiliteControl((ControlHandle) itemhandle,0);
else
	SetControlValue((ControlHandle) itemhandle,0);
return(OK);
}


ShowPannel(int w, int i)
{
Rect r;
short itemtype;
Handle itemhandle;

if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. ShowPannel(). Incorrect index");
	return(FAILED);
	}
GetDialogItem(gpDialogs[w],(short)i,&itemtype,&itemhandle,&r);
itemtype = (itemtype & 127) - ctrlItem;
if(itemtype != radCtrl && itemtype != chkCtrl && itemtype != btnCtrl
		|| itemhandle == NULL) {
	if(Beta) {
		sprintf(Message,"Err ShowPannel(%ld,%ld)",(long)w,(long)i);
		Alert1(Message);
		}
	return(ABORT);
	}
HiliteControl((ControlHandle) itemhandle,0);
return(OK);
}


HidePannel(int w, int i)
{
Rect r;
short itemtype;
Handle itemhandle;

if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. HidePannel(). Incorrect index");
	return(FAILED);
	}
GetDialogItem(gpDialogs[w],(short)i,&itemtype,&itemhandle,&r);
itemtype = (itemtype & 127) - ctrlItem;
if(itemtype != radCtrl && itemtype != chkCtrl && itemtype != btnCtrl
		|| itemhandle == NULL) {
	if(Beta) {
		sprintf(Message,"Err HidePannel(%ld,%ld)",(long)w,(long)i);
		Alert1(Message);
		}
	return(ABORT);
	}
HiliteControl((ControlHandle) itemhandle,255);
return(OK);
}


SelectField(DialogPtr ptr,int w,int ifield,int all)
{
DialogPtr thedialog;

if(ptr != NULL) {
	thedialog = ptr; w = -1;
	}
else {
	if(w < 0 || w >= WMAX || !HasFields[w]) {
		if(Beta) Alert1("Err. SelectField(). Incorrect index");
		return(FAILED);
		}
	thedialog = gpDialogs[w];
	}
if(all) SelectDialogItemText(thedialog,ifield,0,32767);
else  SelectDialogItemText(thedialog,ifield,0,0);
return(OK);
}


ShowLengthType(int w)
{
long n;
char line[MAXLIN];

n = GetTextLength(w);
sprintf(line,"%ld chars.",(long)n);
if(IsHTML[w]) {
	strcat(line,"  File format: HTML");
	if(IsText[w])
		strcat(line," (text)");
	else
		strcat(line," (BP2)");
	}
else
	if(Weird[w]) strcat(line,"  File format: unknown");
	else if(IsText[w]) strcat(line,"  File format: plain text");
ShowMessage(TRUE,wMessage,line);
return(OK);
}


OutlineTextInDialog(int w,int active)
{
Rect r;
Pattern pat;
GrafPtr saveport;

if(w < 0 || w >= WMAX || !Editable[w] || !IsDialog[w] || WASTE) return(OK);
GetPort(&saveport);
SetPortWindowPort(Window[w]);
PenNormal();
if(active) {
	Activate(TEH[w]);
	}
else {
	Deactivate(TEH[w]);
	PenPat(GetQDGlobalsGray(&pat));
	}
RGBForeColor(&Black);
r = LongRectToRect(TextGetViewRect(TEH[w]));
InsetRect(&r,-1,-1);
FrameRect(&r);
PenNormal();

if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err OutlineTextInDialog(). saveport == NULL");
return(OK);
}

#endif /* BP_CARBON_GUI */
