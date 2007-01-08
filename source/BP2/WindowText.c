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
#  include	"MacHeaders.h"
// #  include	"MacHeadersTransitional.h"
#endif

#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"


SetSelect(long start,long end, TextHandle th)
{
#if WASTE
WESetSelection(start,end,th);
#else
TESetSelect(start,end,th);
#endif
return(OK);
}


Activate(TextHandle th)
{
#if WASTE
if(!WEIsActive(th)) WEActivate(th);
#else
TEActivate(th);
#endif
return(OK);
}


Deactivate(TextHandle th)
{
#if WASTE
if(WEIsActive(th)) WEDeactivate(th);
#else
TEDeactivate(th);
#endif
return(OK);
}


Idle(TextHandle th)
{
#if WASTE
WEIdle(NULL,th);
#else
TEIdle(th);
#endif
return(OK);
}


CalText(TextHandle th)
{
#if WASTE
WECalText(th);
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
#else
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
#if WASTE
return(WEGetTextLength(TEH[w]));
#else
return((*(TEH[w]))->teLength);
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
#else
TEDelete(TEH[w]);
#endif
return(OK);
}


TextInsert(char *s,long length,TextHandle th)
{
#if WASTE
WEInsert((Ptr)s,length,(StScrpHandle)NULL,NULL,th);
#else
TEInsert(s,length,th);
#endif
return(OK);
}


TextUpdate(int w)
{
GrafPtr saveport;

if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) Alert1("Err. TextUpdate(). Incorrect w");
	return(ZERO);
	}
GetPort(&saveport);
SetPort(Window[w]);
#if WASTE
EraseRgn((*TEH[w])->viewRgn);
WEUpdate((*TEH[w])->viewRgn,TEH[w]);
#else
TEUpdate(&(Window[w]->portRect),TEH[w]);
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
#else
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
#else
TEAutoView(scroll,th);
#endif
return(OK);
}


TextSetStyle(short mode,TextStyle *p_newStyle,Boolean redraw,TextHandle th)
{
#if WASTE
WESetStyle((WEStyleMode)mode,(const TextStyle*)p_newStyle,th);
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
#else
return((*((**(TEH[w])).hText))[pos]);
#endif
return(OK);
}


TextDispose(TextHandle th)
{
#if WASTE
WEDispose(th);
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
#else
TEGetStyle(0,p_thestyle,p_lineheight,p_ascent,th);
#endif
return(OK);
}


char** WindowTextHandle(TextHandle th)
{
#if WASTE
return((char**) WEGetText(th));
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
#else
return((*(TEH[w]))->lineStarts[line]);
#endif
}