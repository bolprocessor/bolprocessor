/* Interface2.c  (BP2 version 2.9.4) */
 
#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"


Ours(WindowPtr theWindow, WindowPtr thatWindow)
{
return((thatWindow != ZERO) && (theWindow == thatWindow));
}


CantOpen(void)
{
Rect r;
WindowPtr w;

SetRect(&r,152,60,366,132);
SetPort((w = NewWindow(0L,&r,"\p",1,1,NULL,0,0L)));
TextFont(0);
MoveTo(4,40);
DrawString("\pCan't open resource file!");
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
	DisplayHelp(line);
	return(OK);
	}
switch(imenu) {
	case appleM:
		Option = FALSE;
		if(theitem == aboutCommand) return(mAbout(w));
		GetMenuItemText(myMenus[appleM],theitem,name);
		OpenDeskAcc(name);
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
BringToFront(Window[w]);
SetSelect(ZERO,GetTextLength(w),TEH[w]);
TextDelete(w);
PrintBehind(w,s);
TextUpdate(w);
/* GetPort(&saveport);
SetPort(Window[w]); */
r = Window[w]->portRect;
InvalRect(&r);
/* if(saveport != NULL) SetPort(saveport); */
SystemTask();	/* Allows redrawing control strip */
return(OK);
}


ClearMessage(void)
{
SelectBehind(ZERO,GetTextLength(wMessage),TEH[wMessage]);
TextDelete(wMessage);
TextUpdate(wMessage);
/* SystemTask();	Allows redrawing control strip */
return(OK);
}


FlashInfo(char* s)
{
SelectBehind(ZERO,GetTextLength(wInfo),TEH[wInfo]);
TextDelete(wInfo);
PrintBehind(wInfo,s);
ShowWindow(Window[wInfo]);
SelectWindow(Window[wInfo]);
TextUpdate(wInfo);
return(OK);
}


SelectSomething(int w)
/* Select line on which cursor is flashing */
{
int startpos,endpos,length;
char c;
int j;

if(w < 0 || w >= WMAX || !Editable[w]) return(FAILED);
if(w != wStartString) return(OK);
startpos = (**(TEH[w])).selStart;
endpos = (**(TEH[w])).selEnd;
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
	case '–':
	case '—':
		c = '-'; break;
	case '‘':
	case '’':
		c = '\''; break;
	case '“':
	case '”':
		c = '"'; break;
	case '·':
		c = '•'; break;
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
	if(c == '\n') {
		if(Beta) Alert1("Err. ReadLine(). c == '\n'");
		goto NEXTCHAR;
		}
	if(c == '\r' && oldc != '¬') break;
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
/* Read line in TExt buffer.  Old version: length is limited to ‘size’. */
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
			sprintf(Message,"\rSelection too long, truncated: %s…\r",line);
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

if(Nw < 0 || Nw >= WMAX) {
	if(Beta) Alert1("Err. TypeChar()");
	return(ABORT);
	}
if(!Editable[Nw] && !HasFields[Nw]) return(OK);
cutall = arrowkey = FALSE;
if(Editable[Nw]) {
	th = TEH[Nw];
	if(thechar == '\b' && GetTextLength(Nw) == (*th)->selEnd
												&& (*th)->selStart == 0) {
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
			if((*th)->selEnd > (*th)->selStart) {
				TextCut(Nw);
				LastAction = CUTWIND;
				UndoWindow = Nw;
				UndoPos = (*th)->selStart;
				}
			else {
				LastAction = TYPEWIND;
				UndoWindow = Nw;
				UndoPos = (*th)->selStart;
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
		if(thechar == '¬') {
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
DialogRecord dr;
GrafPtr saveport;
Rect r;
int theitem,i,im,j,jj,k,kmax,found,ibot,left,right,top,bottom,leftoffset,
	vpitch,morecoming,numberdrawn,page,j0[maxpage],result,start[maxpage];
ControlHandle h,h_ctrl[maxctrl];
EventRecord theEvent;
Str255 title;

GetPort(&saveport);
for(i=0; i < maxctrl; i++) h_ctrl[i] = NULL;
page = im = 0; j0[0] = jmin;
p_dia = GetNewDialog(ObjectChartID,(Ptr)&dr,(WindowPtr)-1L);
result = OK;
start[0] = 1;
vpitch = Buttonheight + 5;
ShowWindow(p_dia); SelectWindow(p_dia);
SetPort(p_dia);
TextFont(kFontIDCourier); TextSize(10);
r.top = 10; r.left = 5; r.bottom = 30; r.right = 55;
h_ctrl[0] = NewControl(p_dia,&r,"\p",1,0,0,1,pushButProc,0L);
top = 40;
left = 0;
bottom = p_dia->portRect.bottom - p_dia->portRect.top;
right = p_dia->portRect.right - p_dia->portRect.left;
/* right = 150; */
ibot = (bottom - top) / vpitch;
r.top = 10;
r.left = p_dia->portRect.right - p_dia->portRect.left - 60;
r.bottom = r.top + Buttonheight;
r.right = r.left + 50;
h_ctrl[1] = NewControl((WindowPtr)p_dia,&r,"\p==>",(Boolean)1,(short)0,(short)0,(short)1,
	(short)pushButProc,0L);
r.left = p_dia->portRect.right - p_dia->portRect.left - 120;
r.right = r.left + 50;
h_ctrl[2] = NewControl((WindowPtr)p_dia,&r,"\p<==",(Boolean)1,(short)0,(short)0,(short)1,
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
		if((h_ctrl[++im] = NewControl(p_dia,&r,title,1,0,0,1,(ctrltype + 8),0L))
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
SelectWindow(p_dia); 
/* DrawControls(p_dia); */
found = FALSE;
SetPort(p_dia);

FlushEvents(everyEvent,0);

do {
	if(GetNextEvent(mDownMask,&theEvent)) {
		GlobalToLocal(&(theEvent.where));
		if(FindControl(theEvent.where,p_dia,&h) != 0) {
			TrackControl(h,theEvent.where,(ControlActionUPP)MINUSPROC);
			for(i=0; i <= im; i++) {
				if(h == h_ctrl[i]) {
					found = TRUE;
					theitem = i;
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
SelectWindow(p_dia); 
goto REDRAW;

QUIT:
CloseDialog(p_dia);
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err DoThings(). saveport == NULL");
return(result);
}


GetClickedWord(int w,char* line)
// Select epression between quotes, if any, or between spaces, or performance control, tool, procedure…
{
char c;
long i,iorg,iend,length,start1,start2,end1,end2;

iorg = (**(TEH[w])).selStart;
iend = (**(TEH[w])).selEnd;
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
DisplayHelp(line);
return(OK);
}


Answer(char *what,char c)
{
int r,rep,w,i;
Rect rect;
char line[MAXLIN];

if(ScriptExecOn || (AEventOn && !OkWait)) {
	if(c == 'Y') r = YES;
	else r = NO;
	sprintf(line,"%s ? %c\r",what,c);
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
ParamText(c2pstr(line),"\p ?","\p","\p");
if(ComputeOn || PlaySelectionOn) SndSetSysBeepState(sysBeepDisable);
if(c == 'Y') r = Alert(YesNoCancel,0L);
else  r = Alert(NoYesCancel,0L);
SndSetSysBeepState(sysBeepEnable);
AlertOn++;
Interrupted = TRUE;
if(!EmergencyExit && !InputOn)
	for(i=0; i < 5; i++) MainEvent();
AlertOn--;
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
strcpy(LineBuff,message);
GetDialogItem(EnterPtr,fMessage,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,c2pstr(LineBuff));
TESetSelect(ZERO,ZERO,EnterDR.textH);
strcpy(LineBuff,defaultvalue);
GetDialogItem(EnterPtr,fValue,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,c2pstr(LineBuff));
TESetSelect(ZERO,63L,EnterDR.textH);
TEActivate(EnterDR.textH);

ShowWindow(EnterPtr);
BringToFront(EnterPtr);
DrawDialog(EnterPtr);
HiliteDefault(EnterPtr);
EnterOn = TRUE;
Interrupted = TRUE;
AlertOn++;
while(TRUE) {
	eventfound = GetNextEvent(everyEvent,&event);
	MaintainCursor();
	TEIdle(EnterDR.textH);
	/* Apparently needed although Think Reference says IsDialogEvent() blinks the caret */
	if(!IsDialogEvent(&event)) continue;
	if(!eventfound) continue;
	if(event.what == keyDown) {
		c = (char)(event.message & charCodeMask);
		if(c == '\r' || c == '\3') {	/* Return or Enter */
			rep = OK; break;
			}
		}
	if(!DialogSelect(&event,&theDialog,&item) || theDialog != EnterPtr) continue;
	GetDialogItem(theDialog,item,&itemtype,(Handle*)&itemhandle,&r);
	if(itemtype == editText) continue;
	if(itemtype != 4) {
		SysBeep(10); continue;
		}
	rep = RESUME;
	switch(item) {
		case dEnterCancel:
			rep = ABORT; break;
		case dEnterOK:
			rep = OK; break;
		}
	if(rep == OK || rep == ABORT) break;
	}
EnterOn = FALSE;
AlertOn--;
GetDialogItem(EnterPtr,fValue,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(255,t,value);
HideWindow(EnterPtr);
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

SetCursor(&arrow);
GetPort(&saveport);
if(!EmergencyExit && !InitOn && Nw >= 0 && Nw < WMAX) {
	SetPort(Window[Nw]);
	r = Window[Nw]->portRect;
	InvalRect(&r);
	}
r = screenBits.bounds;
if(!EmergencyExit && !InitOn && Nw >= 0 && Nw < WMAX) InvalRect(&r);
if(saveport != NULL) SetPort(saveport);

strcpy(line,s);
ParamText(c2pstr(line),"\p","\p","\p");
if(ComputeOn || PlaySelectionOn) SndSetSysBeepState(sysBeepDisable);
GotAlert = TRUE;
Interrupted = TRUE;
NoteAlert(OKAlert,0L);
if(ComputeOn || PlaySelectionOn) SndSetSysBeepState(sysBeepEnable);
AlertOn++;
if(!EmergencyExit && !InputOn && !AEventOn && !InitOn && !ItemCapture && !TickCapture
		&& EventState == NO) for(i=0; i < 5; i++) MainEvent();
AlertOn--;
if(!EmergencyExit && !InitOn) UpdateWindow(FALSE,Window[wGraphic]);
if(!EmergencyExit && !InitOn) UpdateWindow(FALSE,Window[wPrototype1]);
return(OK);
}


SetOptionMenu(int option)
{
if(option)
	sprintf(Message,"Open (any) file");
else
	sprintf(Message,"Open file");
pStrCopy((char*)c2pstr(Message),PascalLine);
SetMenuItemText(myMenus[fileM],fmOpen,PascalLine);
if(option && CompiledGr && !IsEmpty(wGrammar))
	sprintf(Message,"Tokenized grammar");
else {
	strcpy(Message," ");	/* This avoids messing the menu as name starts with '-' */
	if(FileName[wGrammar][0] == '\0') strcpy(Message,WindowName[wGrammar]);
	else strcat(Message,FileName[wGrammar]);
	}
pStrCopy((char*)c2pstr(Message),PascalLine);
SetMenuItemText(myMenus[windowM],grammarCommand,PascalLine);
if(option && CompiledAl && !IsEmpty(wAlphabet))
	sprintf(Message,"Tokenized alphabet");
else {
	strcpy(Message," ");	/* This avoids messing the menu as name starts with '-' */
	if(FileName[wAlphabet][0] == '\0') strcpy(Message,WindowName[wAlphabet]);
	else strcat(Message,FileName[wAlphabet]);
	}
pStrCopy((char*)c2pstr(Message),PascalLine);
SetMenuItemText(myMenus[windowM],alphabetCommand,PascalLine);
return(OK);
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



MoveLine(int w,int n,int select)
// Move cursor to beginning of line (n = -1) or end of line (n = +1)
{
char c,oldc;
long i,pos,posend,posmax;

if(w < 0 || w >= WMAX || !Editable[w]) return(FAILED);
if(n == 0) return(OK);
pos = (*(TEH[w]))->selStart;
posend = (*(TEH[w]))->selEnd;
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
pos = (*(TEH[w]))->selStart;
posend = (*(TEH[w]))->selEnd;
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
long pos,length,linelength,pagejump;

length = GetTextLength(w);
linelength = length / LinesInText(w);	/* This is an approximation */
pagejump = (linelength * linesInFolder[w] * 2L) / 3L;
pos = (*(TEH[w]))->selStart;
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
i = pos = (*(TEH[w]))->selStart;
posend = (*(TEH[w]))->selEnd;
posmax = GetTextLength(w);
oldc = '\0';
if(n < 0) {
	if(--i < 0) i = 0;
	else {
		while(GetTextChar(w,i) == '\r') i--;
		for(i=i; i > 0; i--) {
			c = GetTextChar(w,i);
			if(oldc == '\r' && c != '¬') n++;
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
		if(c == '\r' && oldc != '¬') n--;
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


SelectBehind(long pos1,long pos2,TextHandle teh)
/* Doesn't force selection to scroll */
{
(*teh)->selStart = pos1; (*teh)->selEnd = pos2;
return(OK);
}


TextDeleteBehind(int w)
{
Deactivate(TEH[w]);
TextDelete(w);
return(OK);
}


SetResumeStop(int force)
{
if((!AlertOn && !ButtonOn) || force) {
	if(!ComputeOn && !PolyOn && !CompileOn && !SoundOn && !SelectOn &&
			!SetTimeOn && !GraphicOn && !PrintOn /* && !ReadKeyBoardOn */
			&& !HangOn && !ScriptExecOn && !WaitOn) {
		if(ResumeStopOn || force) {
			ResumeStopOn = FALSE;
			HideWindow(ResumeUndoStopPtr);
			HideWindow(ResumeStopPtr);
			}
		}
	else {
		if(!ResumeStopOn || force) {
			ResumeStopOn = TRUE;
			if(UndoFlag) {
				HideWindow(ResumeStopPtr);
				ShowWindow(ResumeUndoStopPtr);
				BringToFront(ResumeUndoStopPtr);
				}
			else {
				HideWindow(ResumeUndoStopPtr);
				ShowWindow(ResumeStopPtr);
				BringToFront(ResumeStopPtr);
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
	while(i < im && (isspace(c=(*p_line)[i]) || c == '\0')) i++;
	i0 = i;
	if(i >= im) break;
	j=0;
	while(i < im && !isspace(c=(*p_line)[i]) && c != '\0') {
		line[j] = c;
		i++; j++;
		if(j >= MAXNAME) {
			line[j] = '\0';
			sprintf(Message,"\rToo long name: %s… [max %ld chars]\r",
				(long)line,(long)MAXNAME);
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
	sprintf(Message,"\rIncorrect name: %s…\r",line);
	Print(wTrace,Message);
	r = FAILED;
	continue;
FIX:
	strcpy(FileName[w],line);
	SetName(w,TRUE,FALSE);
	}
return(r);
}


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


MyButton(int quick)
{
// quick = 0 --> process any event (slowest)
// quick = 1 --> process any event except activate. This notably includes update events.
// quick = 2 --> process only mouseclicks

EventRecord event;
int w,eventfound,theclick,what,mousedown,result,intext,hit;
short itemHit,itemtype;
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
eventfound = GetNextEvent(everyEvent,&event);
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
					ActivateWindow(QUICK,w);
					}
				else {
					result = OK;
			/*		if(IsDialogEvent(&event)) result = DoDialog(&event); */
			/*		hit = DialogSelect(&event,&thedialog,&itemHit);
					if(hit && (w == wControlPannel)) { 
						switch(itemHit) {
							case bMute:
								if(!Oms && !NEWTIMER)
									Alert1("‘Mute’ only works when Opcode OMS is active");
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
		sprintf(Message,"\nMemory error %ld (case %ld)",(long)oserr,(long)thecase);
		if(TraceRefnum > -1) WriteToFile(NO,MAC,Message,TraceRefnum);
		if(TempRefnum > -1) WriteToFile(NO,MAC,Message,TempRefnum);
		}
	r = ABORT;
	switch(oserr) {
		case procNotFound: sprintf(line,"Process not found");
			break;
		case paramErr: sprintf(line,"paramErr: attempting to open folder instead of file, or ‘inOutCount’ was negative, or no default volume, or process serial number is invalid");
			break;
		case badUnitErr: sprintf(line,"‘badUnitErr’ error: refNum doesn't match unit table");
			break;
		case unitEmptyErr: sprintf(line,"‘UnitEmptyErr’ error: refNum specifies NIL handle in unit table");
			break;
		case notOpenErr: sprintf(line,"‘notOpenErr’ error: driver is closed");
			break;
		case readErr: sprintf(line,"‘readErr’ error: driver can't respond to Read");
			break;
		case posErr: sprintf(line,"‘posErr’ error: attempt to position before start of file");
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
			strcpy(line,"Unexpected error: ‘Apple Event descriptor not found’."); break;
		case errAECorruptData:	/* -1702 */
			strcpy(line,"Unexpected error: ‘Data in an Apple Event could not be read’."); break;
		case errAENotAppleEvent:	/* -1707 */
			strcpy(line,"Unexpected error: ‘Event is not Apple Event’."); break;
		case errAEReplyNotValid:	/* -1709 */
			strcpy(line,"Unexpected error: ‘AEResetTimer was passed an invalid reply parameter’."); break;
		case errAEHandlerNotFound:	/* -1717 */
			strcpy(line,"Unexpected error: ‘no handler in the dispatch tables fits the parameters to AEGetEventHandler or AEGetCoercionHandler’."); break;
		case errAEEventNotHandled:	/* -1708 */
			strcpy(line,"Unexpected error: ‘Event was not handled by an Apple Event handler’."); break;
		case errAENewerVersion:
			strcpy(line,"Version of Apple Event Manager is too old. Event couldn't be handled."); break;
		case errAEParamMissed:	/* -1715 */
			strcpy(line,"Unexpected error: ‘a required parameter was not accessed’ in Apple Event handling."); break;
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
			strcpy(line,"Unexpected error: ‘illegal operation on a NIL handle’"); break;
		case memWZErr:	/* -111 */
			strcpy(line,"Unexpected error: ‘illegal operation on a free block’"); break;
		case memPurErr: strcpy(line,"Unexpected error: ‘illegal operation on a locked block’"); break;
		case memFullErr:	/* -108 */
			EmergencyExit = TRUE;
			strcpy(line,"Unexpected ‘memory full’ error… You should resize ‘BP2’."); break;
		case memAdrErr:	/* -110 */
			strcpy(line,"Unexpected error: ‘memAdrErr’. Ignored…"); break;
		case memAZErr:	/* -113 */
			strcpy(line,"Unexpected error: ‘memAZErr’. Ignored…"); break;
		case memPCErr:	/* -114 */
			strcpy(line,"Unexpected error: ‘memPCErr’. Ignored…"); break;
		case memBCErr:	/* -115 */
			strcpy(line,"Unexpected error: ‘memBCErr’. Ignored…"); break;
		case memSCErr:	/* -116 */
			strcpy(line,"Unexpected error: ‘memSCErr’. Ignored…"); break;
		case memLockedErr:	/* -117 */
			strcpy(line,"Unexpected error: ‘memLockedErr’. Ignored…"); break;
		case -919:
			strcpy(line,"Unexpected error: ‘PPC record is invalid’"); break;
		default:
			sprintf(line,"Unknown OS error #%ld. Ignored…",(long)oserr);
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


GetString1(char *s)
{
char c;
int i = 0;  
EventRecord theEvent;
do {
	while(!GetNextEvent(everyEvent,&theEvent) || ((theEvent.what != keyDown)
		&& (theEvent.what != autoKey))) ;
  	c = (char) (theEvent.message & charCodeMask);
/*  	Print(wTrace,c); */
   	s[i++] = c;
	}
while(c != '\r' && i < 79); 
s[i] = '\0'; 
return(OK); 
}

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


CheckSettings(void)
{
int rep;

rep = OK;
if(!ScriptExecOn && Dirty[iSettings]) {
	rep = Answer("Save settings",'Y');
	if(rep == ABORT) return(rep);
	if(rep == OK) {
		rep = mSaveSettings(0);
		if(rep != OK) return(rep);
		}
	Dirty[iSettings] = FALSE;
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
		sprintf(Message,"Save changes for “%s”",DeftName[w]);
	else
		sprintf(Message,"Save changes for “%s”",FileName[w]);
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


CompileCheck(void)
{
int r,compiledgl;

r = OK;
compiledgl = CompiledGl;
if(CheckMemory() != OK) return(ABORT);
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
if(LoadedIn && (!CompiledIn && (r=CompileInteraction()) != OK)) return(r);
if((r=UpdateGlossary()) != OK) return(r);
if(!CompiledPt) if((r=CompilePatterns()) != OK) return(r);
if(SmartCursor) r = UpdateAutomata();
return(r);
}

// -----------------------  FIND - REPLACE ------------------------

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
	SetPort(Window[TargetWindow]);
	r = (*(Window[TargetWindow])).portRect;
	InvalRect(&r);
	ActivateWindow(SLOW,TargetWindow);
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
ActivateWindow(SLOW,TargetWindow);
sprintf(Message,"%ld occurrence(s) found…",(long)i);
ShowMessage(TRUE,wMessage,Message);
return(OK);
}


GetNextOccurrence(int *p_redo,int start)
{
int i,j,k,w,jmax,rep;
long pos,posmax;

if((w = TargetWindow) < 0 || w >= WMAX || !Editable[w]) return(ABORT);
rep = FAILED;
pos = (int) (**(TEH[w])).selEnd;
posmax = (int) GetTextLength(w);
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

TextDelete(TargetWindow);
count = (long) strlen(ReplaceString);
TextInsert(ReplaceString,count,TEH[TargetWindow]);
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
Point pt;

ShowWindow(ReplaceCommandPtr);
SelectWindow(ReplaceCommandPtr);
SetPort(ReplaceCommandPtr);
StopWait();
found = FALSE;

// #ifndef __POWERPC
FlushEvents(everyEvent,0);
// #endif

/* DrawControls(ReplaceCommandPtr); */
UpdateDialog(ReplaceCommandPtr,ReplaceCommandPtr->visRgn);
do {
	if(GetNextEvent(everyEvent,&theEvent)) {
		if((theEvent.what == keyDown)
			&& ((char)(theEvent.message & charCodeMask) == '\r')) {
			theitem = dDontChange;
			break;
			}
		if(theEvent.what != mouseDown) continue;
		found = TRUE;
		FindWindow(theEvent.where,&thewindow);
		GlobalToLocal(&(theEvent.where));
		if(FindControl(theEvent.where,ReplaceCommandPtr,&h) != 0) {
			TrackControl(h,theEvent.where,MINUSPROC);
			LocalToGlobal(&(theEvent.where));
			DialogSelect(&theEvent,&thewindow,&theitem);
			}
		else {
			if(thewindow == ReplaceCommandPtr) {
				SysBeep(10);
				found = FALSE;
				}
			theitem = dStopReplace;
			}
		}
	}
while(!found);
HideWindow(ReplaceCommandPtr);

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

sprintf(line,"%s",FindString);
GetDialogItem((DialogPtr)&DRecord[wFindReplace],fFind,&itemtype,
	(Handle*)&itemhandle,&r);
SetDialogItemText((Handle)itemhandle,c2pstr(line));
TESetSelect(ZERO,63L,DRecord[wFindReplace].textH);
sprintf(line,"%s",ReplaceString);
GetDialogItem((DialogPtr)&DRecord[wFindReplace],fReplace,&itemtype,
	(Handle*)&itemhandle,&r);
SetDialogItemText((Handle)itemhandle,c2pstr(line));
TESetSelect(ZERO,63L,DRecord[wFindReplace].textH);
GetDialogItem((DialogPtr)&DRecord[wFindReplace],dIgnoreCase,&itemtype,
	(Handle*)&itemhandle,&r);
/* SetCtlValue(itemhandle,IgnoreCase); */
SetControlValue(itemhandle,IgnoreCase);
GetDialogItem((DialogPtr)&DRecord[wFindReplace],dMatchWords,&itemtype,
	(Handle*)&itemhandle,&r);
/* SetCtlValue(itemhandle,MatchWords); */
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
GetDialogItem((DialogPtr)&DRecord[wFindReplace],fFind,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,FindString);
ConvertSpecialChars(FindString);
Finding = TRUE; if(strlen(FindString) == 0) Finding = FALSE;
GetDialogItem((DialogPtr)&DRecord[wFindReplace],fReplace,&itemtype,&itemhandle,
	&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,ReplaceString);
ConvertSpecialChars(ReplaceString);
Dirty[wFindReplace] = FALSE;
return(OK);
}

// -----------------  DIALOG ROUTINES ---------------------------------

SetField(DialogPtr ptr,int w,int ifield,char* string)
{
Rect r;
Handle itemhandle;
short itemtype;
char line[MAXFIELDCONTENT];
DialogPtr thedialog;
DialogRecord dr;

if(ptr != NULL) {
	thedialog = ptr; w = -1;
	}
else {
	if(w < 0 || w >= WMAX || !HasFields[w]) {
		if(Beta) Alert1("Err. SetField(). Incorrect index");
		return(FAILED);
		}
	thedialog = (DialogPtr) &DRecord[w];
	}
dr =  *((DialogRecord*)thedialog);

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
strcpy(line,string);
SetDialogItemText(itemhandle,c2pstr(line));
SelectDialogItemText(thedialog,ifield,0,0);
if((itemtype & 127) == statText) TEDeactivate(dr.textH);
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
	thedialog = (DialogPtr) &DRecord[w];
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
		ShowWindow(thedialog);
		BringToFront(thedialog);
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
GetDialogItem((DialogPtr)&DRecord[w],(short)icontrol,&itemtype,(Handle*)&itemhandle,&r);
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
GetDialogItem((DialogPtr)&DRecord[w],(short)icontrol,&itemtype,(Handle*)&itemhandle,&r);
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
	thedialog = (DialogPtr) &DRecord[w];
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
	thedialog = (DialogPtr) &DRecord[w];
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
GetDialogItem((DialogPtr)&DRecord[w],(short)i,&itemtype,&itemhandle,&r);
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
GetDialogItem((DialogPtr)&DRecord[w],(short)i,&itemtype,&itemhandle,&r);
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
	thedialog = (DialogPtr) &DRecord[w];
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
GrafPtr saveport;

if(w < 0 || w >= WMAX || !Editable[w] || !IsDialog[w] || WASTE) return(OK);
GetPort(&saveport);
SetPort(Window[w]);
PenNormal();
if(active) {
	Activate(TEH[w]);
	if(HasFields[w]) TEDeactivate(DRecord[w].textH);
	}
else {
	Deactivate(TEH[w]);
	if(HasFields[w]) TEActivate(DRecord[w].textH);
	PenPat(&gray);
	}
RGBForeColor(&Black);
r = LongRectToRect((**(TEH[w])).viewRect);
InsetRect(&r,-1,-1);
FrameRect(&r);
PenNormal();

if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err OutlineTextInDialog(). saveport == NULL");
return(OK);
}


DoSystem(void)
{
OSErr oserr;
GrafPtr saveport;
OSErr err;
int result;

if(Panic || EmergencyExit) return(OK);
result = OK;
GetPort(&saveport);
SystemTask();
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err DoSystem(). saveport == NULL");
err = MemError();
if(err != noErr) {
	if(Beta) TellError(57,err);
	result = FAILED;
	}
if(!SoundOn) return(PlayTick(FALSE));
return(result);
}
