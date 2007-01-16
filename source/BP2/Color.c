/* Color.c (BP2 version CVS) */

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

/* hasMode = HasDepth(gdh,depth,whichFlags,flags); */

ResetPianoRollColors(void)
{
PianoColor[0].red = 0.55 * 0xFFFF;	/* Dirt (dark brown) */
PianoColor[0].green = 0.38 * 0xFFFF;
PianoColor[0].blue = 0.21 * 0xFFFF;

PianoColor[1] = Red;

PianoColor[2].red = 0xFFFF;	/* Orange */
PianoColor[2].green = 0.69 * 0xFFFF;
PianoColor[2].blue = 0.09 * 0xFFFF;

PianoColor[3] = Yellow;

PianoColor[4].red = 0.38 * 0xFFFF;	/*Ocean green */
PianoColor[4].green = 0.84 * 0xFFFF;
PianoColor[4].blue = 0.67 * 0xFFFF;

PianoColor[5] = Blue;
PianoColor[6] = Magenta;

PianoColor[7].red = 0.57 * 0xFFFF;	/* Light grey */
PianoColor[7].green = 0.57 * 0xFFFF;
PianoColor[7].blue = 0.57 * 0xFFFF;

PianoColor[8].red = 0.84 * 0xFFFF;	/* Marble-ish */
PianoColor[8].green = 0.93 * 0xFFFF;
PianoColor[8].blue = 0.92 * 0xFFFF;

PianoColor[9].red = 0.33 * 0xFFFF;	/* Dark grey */
PianoColor[9].green = 0.33 * 0xFFFF;
PianoColor[9].blue = 0.33 * 0xFFFF;

PianoColor[10].red = 0.75 * 0xFFFF;	/* Tan (light brown) */
PianoColor[10].green = 0.52 * 0xFFFF;
PianoColor[10].blue = 0.29 * 0xFFFF;

PianoColor[11].red = 0xFFFF;	/* Light red */
PianoColor[11].green = 0.46 * 0xFFFF;
PianoColor[11].blue = 0.36 * 0xFFFF;

PianoColor[12].red = 0.96 * 0xFFFF; /* Light orange */
PianoColor[12].green = 0.79 * 0xFFFF;
PianoColor[12].blue = 0.55 * 0xFFFF;

PianoColor[13].red = 0xFFFF;	/* Light yellow */
PianoColor[13].green = 0xFFFF;
PianoColor[13].blue = 0.61 * 0xFFFF;

PianoColor[14] = Green;

PianoColor[15].red = 0.36 * 0xFFFF;	/* Light blue */
PianoColor[15].green = 0.78 * 0xFFFF;
PianoColor[15].blue = 0xFFFF;

return(OK);
}


InitColors(void)
{
RGBColor c;

White.red = White.green = White.blue = 0xFFFF;

Black.red = Black.green = Black.blue = 0;
CopyColor(&Black,NoteC);

Red.red = 0xFFFF;
Red.green = 0;
Red.blue = 0;

Green.red = 0;
Green.green = 0xFFFF;
Green.blue = 0;

Blue.red = 0;
Blue.green = 0;
Blue.blue = 0xFFFF;

Yellow.red = 0xFFFF;
Yellow.green = 0xFFFF;
Yellow.blue = 0;

Magenta.red = 0xFFFF;
Magenta.green = 0;
Magenta.blue = 0xFFFF;

Cyan.red = 0;
Cyan.green = 0xFFFF;
Cyan.blue = 0xFFFF;

c.red = 238 * 256;
c.green = 0;
c.blue = 0;
CopyColor(&c,VariableC);
CopyColor(&c,PivotC);

Brown.red = 121 * 256;
Brown.green = 74 * 256;
Brown.blue = 0;
CopyColor(&Brown,TerminalC);

c.red = 0;
c.green = 0;
c.blue = 191 * 256;
CopyColor(&c,HomomorphismC);

c.red = 0;
c.green = 0;
c.blue = 30000;
CopyColor(&c,StreakC);

c.red = 0;
c.green = 218 * 256;
c.blue = 0;
CopyColor(&c,TimePatternC);

c.red = 16 * 256;
c.green = 96 * 256;
c.blue = 127 * 256;
CopyColor(&c,FlagC);

c.red = 156 * 256;
c.green = 0;
c.blue = 81 * 256;
CopyColor(&c,ControlC);

c.red = 199 * 256;
c.green = 184 * 256;
c.blue = 0;
CopyColor(&c,TagC);

CopyColor(&Yellow,SoundObjectC);

NoteScaleColor1.red = 0;
NoteScaleColor1.green = 30000;
NoteScaleColor1.blue = 30000;

NoteScaleColor2.red = 50000;
NoteScaleColor2.green = 50000;
NoteScaleColor2.blue = 0;

NewColors = FALSE;
return(OK);
}


CopyColor(RGBColor *color,int i)
{
Color[i].red = color->red;
Color[i].green = color->green;
Color[i].blue = color->blue;
return(OK);
}


Reformat(int w,int font,int size,int face,RGBColor* p_color,int manual,int force)
{
TextStyle thestyle;
short lineheight,ascent;
Rect r;
GrafPtr saveport;

if(w < 0 || w >= WMAX) return(FAILED);
if((!UseTextColor || ((DisplayProduce || TraceProduce) && ComputeOn))
	&& !force) return(OK);
if(!Editable[w]) {
	if(Beta) Alert1("Err. Reformat(). ");
	return(FAILED);
	}
/* if((CurrentColor[w].red == p_color->red) && (CurrentColor[w].green == p_color->green)
	&& (CurrentColor[w].blue == p_color->blue) && font < 0 && size < 0 && face < 0)
		return(OK); */
GetTextStyle(&thestyle,&lineheight,&ascent,TEH[w]);
if(size > 0) {
	thestyle.tsSize = (short) size;
	TextSetStyle(doSize,&thestyle,manual,TEH[w]);
	}
if(!UseTextColor && !force) return(OK);
if(font >= 0) {
	thestyle.tsFont = (short) font;
	TextSetStyle(doFont,&thestyle,manual,TEH[w]);
	}
if(face >= 0) {
	thestyle.tsFace = (Style) face;
	TextSetStyle(doFace,&thestyle,manual,TEH[w]);
	}
if(p_color != &None) {
	CurrentColor[w].red = thestyle.tsColor.red = p_color->red;
	CurrentColor[w].green = thestyle.tsColor.green = p_color->green;
	CurrentColor[w].blue = thestyle.tsColor.blue = p_color->blue;
	TextSetStyle(doColor,&thestyle,manual,TEH[w]);
	}
if(!manual) return(OK);
r = LongRectToRect((**(TEH[w])).viewRect);
GetPort(&saveport);
SetPortWindowPort(Window[w]);
InvalRect(&r);
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err Reformat(). saveport == NULL");
return(OK);
}


SetFontSize(int w,int size)
{
long origin,end;
GrafPtr saveport;

if(w < 0 || w >= WMAX) return(FAILED);
WindowTextSize[w] = size;
if(!Editable[w]) return(OK);
origin = (**(TEH[w])).selStart; end = (**(TEH[w])).selEnd;
Deactivate(TEH[w]);
SetSelect(ZERO,GetTextLength(w),TEH[w]);
Reformat(w,-1,size,-1,&None,TRUE,TRUE);
SetSelect(origin,end,TEH[w]);
GetPort(&saveport);
SetPortWindowPort(Window[w]);
TextSize(size);
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err SetFontSize(). saveport == NULL");
if((FrontWindow() == Window[w] || Nw == w) && !LockedWindow[w]) Activate(TEH[w]);
return(OK);
}


ChangeColor(void)
{
int i,rep;
Point where;
RGBColor incolor,outcolor;
Str255 message;

while(TRUE) {
	SetPt(&where,0,0);	/* Also try -1,-1 */
	StopWait();
	rep = Alert(ColorAlert,0L);
	switch(rep) {
		case dOK:
			return(OK);
		case dVariable:
		case fVariable:
			strcpy(Message,"Color for variables");
			i = VariableC; break;
		case dTerminal:
		case fTerminal:
			strcpy(Message,"Color for terminal symbols");
			i = TerminalC; break;
		case dNote:
		case fNote:
			strcpy(Message,"Color for simple notes");
			i = NoteC; break;
		case dHomomorphism:
		case fHomomorphism:
			strcpy(Message,"Color for homomorphism");
			i = HomomorphismC; break;
		case dTimePattern:
		case fTimePattern:
			strcpy(Message,"Color for time patterns");
			i = TimePatternC; break;
		case dFlag:
		case fFlag:
			strcpy(Message,"Color for flags");
			i = FlagC; break;
		case dControl:
		case fControl:
			strcpy(Message,"Color for controls");
			i = ControlC; break;
		case dTag:
		case fTag:
			strcpy(Message,"Color for sync tags");
			i = TagC; break;
		case dSoundObject:
			strcpy(Message,"Color for sound-objects (graphic)");
			i = SoundObjectC; break;
		case dStreak:
			strcpy(Message,"Color for streaks (graphic)");
			i = StreakC; break;
		case dPivot:
			strcpy(Message,"Color for time pivots (graphic)");
			i = PivotC; break;
		}
	incolor = Color[i];
	if(GetColor(where,in_place_c2pstr(Message),&incolor,&outcolor)) {
		CopyColor(&outcolor,i);
		NewColors = TRUE;
		if(Version > 4) Dirty[iSettings] = TRUE;
		}
	}
}
