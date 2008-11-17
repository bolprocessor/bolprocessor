/* WindowText.c (BP2 version CVS) */

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
#include "CarbonCompatUtil.h"

#if WASTE
LongRect TextGetViewRect(TextHandle th)
#else
Rect     TextGetViewRect(TextHandle th)
#endif
{
#if !USE_MLTE
	return (*th)->viewRect;
#else
	Rect vr;
	TXNGetViewRect((*th)->textobj, &vr);
	return vr; // (*th)->viewRect;
#endif
}


Boolean TextIsSelectionEmpty(TextHandle th)
{
#if USE_MLTE
	return TXNIsSelectionEmpty((*th)->textobj);
#else
	return (((*th)->selEnd - (*th)->selStart) == ZERO);
#endif	
}


int TextGetSelection(TextOffset* start,TextOffset* end, TextHandle th)
{
	if (th == NULL)  return (FAILED);
	
#if WASTE
	// WEGetSelection(start,end,th);
	// most of the code does this the same as TextEdit,
	// so I don't want to make any changes at this time - akozar 032807
	*start = (*th)->selStart;
	*end   = (*th)->selEnd;
#elif USE_MLTE
	{ TXNOffset s, e;
	TXNGetSelection((*th)->textobj, &s, &e);
	*start = s; *end = e;
	}
#else
	*start = (*th)->selStart;
	*end   = (*th)->selEnd;
#endif

return(OK);
}


SetSelect(TextOffset start,TextOffset end, TextHandle th)
{
	long maxoffset;

#if !WASTE	
	/* clamp range to text bounds (WASTE does these checks) */
	maxoffset = GetTextHandleLength(th);
	if (start < ZERO) {
		if(Beta) Alert1("Err. SetSelect(). start < ZERO");
		start = ZERO;
	}
	else if (start > maxoffset) {
		if(Beta) Alert1("Err. SetSelect(). start > maxoffset");
		start = maxoffset;
	}
	if (end < ZERO) {
		if(Beta) Alert1("Err. SetSelect(). end < ZERO");
		end = ZERO;
	}
	else if (end > maxoffset) {
		if(Beta) Alert1("Err. SetSelect(). end > maxoffset");
		end = maxoffset;
	}
#endif

#if WASTE
WESetSelection(start,end,th);
#elif USE_MLTE
TXNSetSelection((*th)->textobj, start, end);
#else
TESetSelect(start,end,th);
#endif
return(OK);
}


Activate(TextHandle th)
{
#if WASTE
if(!WEIsActive(th)) WEActivate(th);
#elif USE_MLTE
TXNFocus((*th)->textobj, TRUE);
TXNActivate((*th)->textobj, (*th)->id, TRUE); // returns an OSStatus that we do not check
#else
TEActivate(th);
#endif
return(OK);
}


Deactivate(TextHandle th)
{
#if WASTE
if(WEIsActive(th)) WEDeactivate(th);
#elif USE_MLTE
TXNFocus((*th)->textobj, FALSE);
TXNActivate((*th)->textobj, (*th)->id, FALSE); // returns an OSStatus that we do not check
#else
TEDeactivate(th);
#endif
return(OK);
}


Idle(TextHandle th)
{
#if WASTE
WEIdle(NULL,th);
#elif USE_MLTE
TXNIdle((*th)->textobj);
#else
TEIdle(th);
#endif
return(OK);
}


CalText(TextHandle th)
{
#if WASTE
WECalText(th);
#elif USE_MLTE
PrintCall("CalText()", NULL);
TXNRecalcTextLayout((*th)->textobj);
// TXNForceUpdate((*th)->textobj);
#else
TECalText(th);
#endif
return(OK);
}


DoKey(char c,EventModifiers modifiers,TextHandle th)
{
#if WASTE
if(!WEIsActive(th)) WEActivate(th);
WEKey((short)c,modifiers,th);
#elif USE_MLTE
/*  {	EventRecord e;
  	e.what = keyDown;
  	e.
TXNKeyDown((*th)->textobj, const EventRecord * iEvent);*/
#else
// check length of text edit buffer first
long len = (long) (*(th))->teLength;
// if full, only allow backspace, delete, and arrow keys
if ((len >= TEXTEDIT_MAXCHARS || len < 0) && c != '\b' && c != 0x7F
    && c != '\34' && c != '\35' && c != '\36' && c != '\37') {
  	TextFullError(th, 1);
	return(FAILED);
	}
TEKey(c,th);
#endif
return(OK);
}


long GetTextLength(int w)
{
	if(w < 0 || w >= WMAX || !Editable[w]) {
		if(Beta) Alert1("Err. GetTextLength(). Incorrect w");
		return(ZERO);
		}
	return GetTextHandleLength(TEH[w]);
}

long GetTextHandleLength(TextHandle th)
{
	if (th == NULL) {
		if(Beta) Alert1("Err. GetTextHandleLength(). NULL handle");
		return(ZERO);
	}
	
#if WASTE
return(WEGetTextLength(th));
#elif USE_MLTE
// TXNDataSize returns a byte count which we assume is the number
// of characters since we are using the Mac Roman encoding.
return (long) TXNDataSize((*th)->textobj);
#else
return((*th)->teLength);
#endif
}


TextDelete(int w)
{
if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. TextDelete(). Incorrect w");
	return(ZERO);
	}
#if WASTE
WEDelete(TEH[w]);
#elif USE_MLTE
TXNClear((*(TEH[w]))->textobj);
/*{ TXNOffset start, end;
  char s[1] = "";
  TXNGetSelection((*(TEH[w]))->textobj, &start, &end);
  TXNSetData((*(TEH[w]))->textobj, kTXNTextData, s, 0, start, end);
}*/
#else
TEDelete(TEH[w]);
#endif
return(OK);
}


TextInsert(char *s,long length,TextHandle th)
{
#if WASTE
WEInsert((Ptr)s,length,(StScrpHandle)NULL,NULL,th);
#elif USE_MLTE
{ TXNOffset start, end;
  TXNGetSelection((*th)->textobj, &start, &end);
  TXNSetData((*th)->textobj, kTXNTextData, s, length, start, end);
}
#else
{ long textlen;

  /* first check that there is enough room in the TextEdit handle */
  textlen = (long) (*(th))->teLength;
  if (textlen < 0 || (length+textlen) > TEXTEDIT_MAXCHARS) {
  	TextFullError(th, 0);
	return(FAILED);
	}
  }
TEInsert(s,length,th);
#endif
return(OK);
}


/* Controls reporting of errors if using TextEdit and the window is full */
int TextFullError(TextHandle th, int messageNum)
{
	int w;
	long len;
	
#if !WASTE && !USE_MLTE
	// get a window index from the TextHandle
	for (w = 0; w < WMAX; ++w)  if (th == TEH[w]) break;
	if (w < 0 || w >= WMAX || !Editable[w])  return (FAILED);
	if (WindowFullAlertLevel[w] < 2) {
		switch (messageNum) {
			case 1:  // for DoKey()
				len = (long) (*(th))->teLength;
				sprintf(Message, "Text window is full! (It has %d characters). "
					  "No more text can be typed.", len);
				break;
			case 0:  // for TextInsert()
			default:
	  			sprintf(Message, "BP2 cannot insert text into window '%s' because it is too full."
					  " (%d characters max)", (FileName[w][0] ? FileName[w] : WindowName[w]),
					  TEXTEDIT_MAXCHARS);
				break;
		}
		Alert1(Message);
		WindowFullAlertLevel[w] = 2;	// 2 means we've warned about being full
	}
#endif	

	return(OK);
}


TextUpdate(int w)
{
GrafPtr saveport;
Rect r;

if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. TextUpdate(). Incorrect w");
	return(ZERO);
	}
GetPort(&saveport);
SetPortWindowPort(Window[w]);
#if WASTE
EraseRgn((*TEH[w])->viewRgn);
WEUpdate((*TEH[w])->viewRgn,TEH[w]);
#elif USE_MLTE
PrintCall("TextUpdate()", Window[w]);
TXNUpdate((*TEH[w])->textobj);
#else
TEUpdate(GetWindowPortBounds(Window[w], &r),TEH[w]);
#endif
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err. TextUpdate(). saveport == NULL");
return(OK);
}


TextCut(int w)
{
if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. TextCut(). Incorrect w");
	return(ZERO);
	}
#if WASTE
WECut(TEH[w]);
TEFromScrap(); /* Necessary to pass on the content to dialog field */
#elif USE_MLTE
TXNCut((*TEH[w])->textobj);
#else
TECut(TEH[w]);
#endif
return(OK);
}


TextPaste(int w)
{
if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. TextPaste(). Incorrect w");
	return(ZERO);
	}
#if WASTE
WEPaste(TEH[w]);
#elif USE_MLTE
TXNPaste((*TEH[w])->textobj);
#else
{ SInt32 offset;
  long size, textlen;

  /* first check that the clipboard contents are not too large */
  size = CCUGetScrap(NULL, 'TEXT', &offset);
  textlen = GetTextLength(w);
  if (textlen < 0 || (size+textlen) > TEXTEDIT_MAXCHARS) {
  	// we want to display an error each time 'Paste' fails (unlike DoKey and TextInsert)
  	sprintf(Message, "The contents of the clipboard are too large to paste into window '%s'! (%d "
  	        "characters max)", (FileName[w][0] ? FileName[w] : WindowName[w]), TEXTEDIT_MAXCHARS);
	Alert1(Message);
	return(FAILED);
	}
  }
TEStylePaste(TEH[w]);
#endif
return(OK);
}


TextCopy(int w)
{
if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. TextCopy(). Incorrect w");
	return(ZERO);
	}
#if WASTE
WECopy(TEH[w]);
TEFromScrap(); /* Necessary to pass on the content to dialog field */
#elif USE_MLTE
TXNCopy((*TEH[w])->textobj);
#else
TECopy(TEH[w]);
#endif
return(OK);
}


TextAutoView(int force,int scroll,TextHandle th)
{
#if WASTE
if(!force) return(OK);
if(scroll) WEFeatureFlag(weFAutoScroll,weBitSet,th);
else  WEFeatureFlag(weFAutoScroll,weBitClear,th);
#elif USE_MLTE

#else
TEAutoView(scroll,th);
#endif
return(OK);
}


TextSetStyle(short mode,TextStyle *p_newStyle,Boolean redraw,TextHandle th)
{
#if WASTE
WESetStyle((WEStyleMode)mode,(const TextStyle*)p_newStyle,th);
#elif USE_MLTE

#else
TESetStyle(mode,p_newStyle,redraw,th);
#endif
return(OK);
}


TextScroll(long h,long v,TextHandle th)
{
#if WASTE
LongRect r;

WEScroll(h,v,th);
#elif USE_MLTE

#else
TEScroll(h,v,th);
#endif
return(OK);
}


long LinesInText(int w)
{
if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. LinesInText(). Incorrect w");
	return(ZERO);
	}
#if WASTE
return(WECountLines(TEH[w]));
#elif USE_MLTE
  {	OSStatus err;
	ItemCount count;
	err = TXNGetLineCount((*TEH[w])->textobj, &count);
	return (long)count;
  }
#else
return((**(TEH[w])).nLines);
#endif
}


TextClick(int w,EventRecord *p_event)
{
if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. TextClick(). Incorrect w");
	return(ZERO);
	}
#if WASTE
WEClick(p_event->where,p_event->modifiers,p_event->when,TEH[w]);
#elif USE_MLTE
PrintEvent(p_event, "TextClick()", Window[w]);
LocalToGlobal(&p_event->where);
TXNClick((*TEH[w])->textobj, p_event);
#else
TEClick(p_event->where,(p_event->modifiers & shiftKey) != 0,TEH[w]);
#endif
return(OK);
}


char GetTextChar(int w,long pos)
{
if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. GetTextChar(). Incorrect w");
	return(ZERO);
	}
#if WASTE
return((char)WEGetChar(pos,TEH[w]));
#elif USE_MLTE

#else
return((*((**(TEH[w])).hText))[pos]);
#endif
return(OK);
}


TextDispose(TextHandle th)
{
#if WASTE
WEDispose(th);
#elif USE_MLTE

#else
TEDispose(th);
#endif
return(OK);
}


SetTextViewRect(Rect *p_r,TextHandle th)
{
#if WASTE
LongRect r;
r.top = p_r->top;
r.left = p_r->left;
r.bottom = p_r->bottom;
r.right = p_r->right;
WESetViewRect((const LongRect*) &r,th);
#elif USE_MLTE
//TXNSetFrameBounds((*th)->textobj, p_r->top, p_r->left, 
//			p_r->bottom, p_r->right, (*th)->id);
TXNResizeFrame((*th)->textobj, (p_r->right - p_r->left), 
		   (p_r->bottom - p_r->top), (*th)->id);
#else
(*th)->viewRect = *p_r;
#endif
return(OK);
}


SetTextDestRect(Rect *p_r,TextHandle th)
{
#if WASTE
LongRect r;
r.top = p_r->top;
r.left = p_r->left;
r.bottom = p_r->bottom;
r.right = p_r->right;
WESetDestRect((const LongRect*) &r,th);
#elif USE_MLTE
//TXNSetFrameBounds ((*th)->textobj, p_r->top, p_r->left, 
//			 p_r->bottom, p_r->right, (*th)->id);
TXNResizeFrame((*th)->textobj, (p_r->right - p_r->left), 
		   (p_r->bottom - p_r->top), (*th)->id);
#else
(*th)->destRect = *p_r;
#endif
return(OK);
}


GetTextStyle(TextStyle *p_thestyle,short *p_lineheight,short *p_ascent,TextHandle th)
{
#if WASTE
WERunInfo runinfo;

WEGetRunInfo(ZERO,&runinfo,th);
p_thestyle->tsFont = runinfo.runAttrs.runStyle.tsFont;
p_thestyle->tsFace = runinfo.runAttrs.runStyle.tsFace;
p_thestyle->tsSize = runinfo.runAttrs.runStyle.tsSize;
p_thestyle->tsColor = runinfo.runAttrs.runStyle.tsColor;
*p_lineheight = runinfo.runAttrs.runHeight;
*p_ascent = runinfo.runAttrs.runAscent;
#elif USE_MLTE

#else
TEGetStyle(0,p_thestyle,p_lineheight,p_ascent,th);
#endif
return(OK);
}


char** WindowTextHandle(TextHandle th)
{
#if WASTE
return((char**) WEGetText(th));
#elif USE_MLTE

#else
return((char**) (*th)->hText);
#endif
}


#if WASTE
Rect LongRectToRect(LongRect r)
{
Rect r1;

r1.top = r.top;
r1.left = r.left;
r1.bottom = r.bottom;
r1.right = r.right;
return(r1);
}
#else
Rect LongRectToRect(Rect r)
{
return(r);
}
#endif


long LineStartPos(int line,int lineoffset,int w)
{
#if WASTE
LongPt p;
char edge;
long pos;

if(line < 1) return(ZERO);
p.h = 5L;
p.v = 4L + (LineHeight(w) * ((float)(line - lineoffset) + 0.5));
pos = WEGetOffset((const LongPt*)&p,&edge,TEH[w]);
return(pos);
#elif USE_MLTE

#else
return((*(TEH[w]))->lineStarts[line]);
#endif
}