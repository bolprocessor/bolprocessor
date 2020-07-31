/* Graphic.c (BP2 version CVS) */

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

#ifndef SHOWEVERYTHING
#define SHOWEVERYTHING 0
#endif

DrawItem(int w,SoundObjectInstanceParameters **p_object,Milliseconds **p_t1,
	Milliseconds **p_t2,int kmax,long tmin,long tmax,
	unsigned long imax,int nmin,int nmax,unsigned long **p_imaxseq,int kmode,
	int interruptok,
	Milliseconds **p_delta)
{
int linenum,linemax,maxlines,hrect,htext,morespace,**p_morespace,
	nseq,leftoffset,topoffset,rep,maxslideh,maxslidev,linemin,tab,key,
	edge,foundone,overflow;
long pivloc,t1,tt1,t2,endxmax,endymax,endx,y,i,j,k,yruler,
	**p_endx,endy,**p_endy,**p_top,trbeg,trend;
Rect r, r2;
Str255 label;
char line[BOLSIZE+5],line2[BOLSIZE+1];
p_list **waitlist;
GrafPtr saveport;
WindowPtr window;
CGrafPtr port;
GDHandle gdh;

if(TempMemory) return(OK);
if(tmin == Infpos) {
	if(Beta) Alert1("Err. DrawObject(). tmin == Infpos");
	return(OK);
	}
GetPort(&saveport);
if(CheckLoadedPrototypes() != OK) return(OK);
window = Window[w];
PleaseWait();
rep = OK;
GraphicOn = TRUE; overflow = FALSE;
maxlines = (int) Maxevent + 1;

if((p_morespace = (int**) GiveSpace((Size)maxlines*sizeof(int))) == NULL)
	return(ABORT);
if((p_endx = (long**) GiveSpace((Size)maxlines*sizeof(long))) == NULL)
	return(ABORT);
if((p_endy = (long**) GiveSpace((Size)maxlines*sizeof(long))) == NULL)
	return(ABORT);
if((p_top = (long**) GiveSpace((Size)maxlines*sizeof(long))) == NULL)
	return(ABORT);
	
// Find size of picture
hrect = WindowTextSize[w] + 3; /* height of rectangles */
htext = WindowTextSize[w] + 2;
leftoffset = hrect - (tmin * GraphicScaleP) / GraphicScaleQ / 10;
topoffset = (3 * htext) + 8;
GetWindowPortBounds(window, &r);
r.bottom = r.top + topoffset + Maxevent * (hrect + htext);
endxmax = leftoffset + ((tmax - tmin) * GraphicScaleP) / GraphicScaleQ / 10
	+ BOLSIZE * CharWidth('w');
if(endxmax < 100) endxmax = 100;
r.right = r.left + endxmax;
GetWindowPortBounds(window, &r2);
if(r.right < r2.right) r.right = r2.right;

if(OpenGraphic(w,&r,NO,&port,&gdh) != OK) return(ABORT);

TextFont(kFontIDCourier);
TextSize(WindowTextSize[w]);

rep = DrawItemBackground(&r,imax,htext,hrect,leftoffset,interruptok,p_delta,&yruler,
	topoffset,&overflow);
if(rep != OK || overflow) goto ENDGRAPH;

// Now draw sound objects

linenum = 0; linemax = 1;
(*p_endx)[linenum] = endxmax = endymax = Infneg;
(*p_top)[linenum] = topoffset;
(*p_top)[linemax] = (*p_top)[linenum] + hrect + htext;
(*p_morespace)[linenum] = linemin = 0;
if(0 < Hmin[w]) Hmin[w] = 0;
if(0 < Vmin[w]) Vmin[w] = 0;
for(nseq = nmin; nseq <= nmax; nseq++) {
	foundone = FALSE;
	if(DoSystem() != OK) {
		rep = ABORT; goto ENDGRAPH;
		}
	for(i=1; i < (*p_imaxseq)[nseq] && i <= imax; i++) {
		if(GraphOverflow(p_Picture[0])) {
			overflow = TRUE;
			rep = OK;
			goto ENDGRAPH;
			}
		if((i % 10) == 0 && (rep=InterruptDraw(0,interruptok)) != OK) {
			if(TempMemory) rep = OK;
			goto ENDGRAPH;
			}
		k = (*((*p_Seq)[nseq]))[i];
		if(k < 0) if(Beta) Alert1("Err. 'k' in DrawItem(). ");
		if(k < 2) continue;	/* Reject '_' and '-' */
		if(kmode) {
			if(p_object == NULL) {
				if(Beta) Alert1("Err. DrawObject(). p_object == NULL");
				return(ABORT);
				}
			t1 = (*p_object)[k].starttime;
			t2 = (*p_object)[k].endtime;
			}
		else {
			t1 = (*p_t1)[i];
			t2 = (*p_t2)[i];
			}
		t1 =  (t1 * GraphicScaleP) / GraphicScaleQ / 10;
		t2 =  (t2 * GraphicScaleP) / GraphicScaleQ / 10;
		j = (*p_Instance)[k].object; /* Beware: j < 0 for out-time objects */
		trbeg = ((*p_Instance)[k].truncbeg * GraphicScaleP) / GraphicScaleQ / 10;
		tt1 = leftoffset + t1 - trbeg;
		trend = ((*p_Instance)[k].truncend * GraphicScaleP) / GraphicScaleQ / 10;
		if((*p_ObjectSpecs)[k] != NULL && (waitlist=WaitList(k)) != NULL) {
			PenNormal();	/* Draw synchronization tag */
			MoveTo(tt1,yruler+1);
			for(edge=3; edge > 0; edge--) {
				Line(edge,2*edge); Line(-2*edge,0); Line(edge,-2*edge);
				Line(0,1);
				}
			}
#if SHOWEVERYTHING
		if(j == 0) continue;
#else
		if(j == 0 || j == 1 || j == -1) continue;
#endif
		if(j > 0) {
			if(j >= Jbol && j < 16384) sprintf(line,"%s",*((*p_Patt)[j-Jbol]));
			else {
				if(j < 16364) sprintf(line,"%s",*((*p_Bol)[j]));
				else {
					key = j - 16384;
					
					if((*p_Instance)[k].lastistranspose)
						TransposeKey(&key,(*p_Instance)[k].transposition);
					key = ExpandKey(key,(*p_Instance)[k].xpandkey,(*p_Instance)[k].xpandval);
					if(!(*p_Instance)[k].lastistranspose)
						TransposeKey(&key,(*p_Instance)[k].transposition);
					
					key = MapThisKey(key,0.,(*p_Instance)[k].mapmode,
						&((*p_Instance)[k].map0),
						&((*p_Instance)[k].map1));
					PrintNote(key,0,-1,line);
					}
				}
			}
		else {
			j = -j;
			if(j < 16364) sprintf(line,"<<%s>>",*((*p_Bol)[j]));
				else {
					strcpy(line,"<<");
					key = j - 16384;
					
					if((*p_Instance)[k].lastistranspose)
						TransposeKey(&key,(*p_Instance)[k].transposition);
					key = ExpandKey(key,(*p_Instance)[k].xpandkey,(*p_Instance)[k].xpandval);
					if(!(*p_Instance)[k].lastistranspose)
						TransposeKey(&key,(*p_Instance)[k].transposition);
						
					key = MapThisKey(key,0.,(*p_Instance)[k].mapmode,
						&((*p_Instance)[k].map0),
						&((*p_Instance)[k].map1));
					PrintNote(key,0,-1,line2);
					strcat(line,line2);
					strcat(line,">>");
					}
			}
		if(SHOWEVERYTHING) {
			sprintf(line2," #%ld",(long)k);
			strcat(line,line2);
			}
		c2pstrcpy(label, line);
		tab = ((int) t2 - (int) t1 - StringWidth(label)) / 2;
		if(tab < 2) tt1 = (int) t1 + leftoffset + 1 + tab;
		if(tt1 < (*p_endx)[linenum]) {
			for(linenum=linemin; linenum < linemax; linenum++) {
				if(tt1 > (*p_endx)[linenum]) goto CONT;
				}
			linemax = linenum + 1;	/* here 'linenum' has been incremented */
			foundone = TRUE;
			if(linenum >= maxlines) {
				sprintf(Message,
					"Err. linenum = %ld  maxlines = %ld  DrawItem() ",
					(long)linenum,(long)maxlines);
				if(Beta) Alert1(Message);
				rep = ABORT;
				goto ENDGRAPH;
				}
			(*p_endx)[linenum] = Infneg;
			(*p_morespace)[linenum] = 0;
			}
CONT:
		morespace = (*p_morespace)[linenum];
		if(j < 16384) {
			if((*p_PivMode)[j] == ABSOLUTE)
				pivloc = (long) ((*p_PivPos)[j] * GraphicScaleP) / GraphicScaleQ / 10L;
			else
				pivloc = (long) ((*p_Instance)[k].dilationratio * (*p_PivPos)[j] * (*p_Dur)[j]
							* GraphicScaleP) / GraphicScaleQ / 1000L;
			}
		else pivloc = 0.;
		pivloc -= trbeg;
		if(DrawObject(j,label,(*p_Instance)[k].dilationratio,(*p_top)[linenum],hrect,htext,
				leftoffset,pivloc,t1,t2,trbeg,trend,&morespace,
				&endx,&endy,p_Picture[0]) == ABORT) {
			rep = OK;
			goto ENDGRAPH;
			}
		(*p_morespace)[linenum] = morespace;
		(*p_endx)[linenum] = endx;
		(*p_endy)[linenum] = endy;
		if(endx > Hmax[w]) Hmax[w] = endx;
		if(endy > Vmax[w]) Vmax[w] = endy;
		(*p_top)[linenum+1] = (*p_top)[linenum] + hrect
							+ htext * (1 + (*p_morespace)[linenum]);
		if((*p_endx)[linenum] > endxmax) endxmax = (*p_endx)[linenum];
		if((*p_endy)[linenum] > endymax) endymax = (*p_endy)[linenum];
		}
	linenum = linemin = linemax;
	if(!foundone) (*p_top)[linenum] = (*p_top)[linenum-1] + hrect
							+ htext * (1 + (*p_morespace)[linenum-1]);
	linemax += 1 /* foundone */;
	if(linenum >= maxlines) {
		sprintf(Message,
			"Err. linenum = %ld  maxlines = %ld  DrawItem() ",
			(long)linenum,(long)maxlines);
		if(Beta) Alert1(Message);
		rep = ABORT;
		goto ENDGRAPH;
		}
	(*p_endx)[linenum] = Infneg;
	(*p_morespace)[linenum] = 0;
	}

ENDGRAPH:
CloseGraphic(w,endxmax,endymax,overflow,&r,&port,gdh);

StopWait();

QUIT:
#if NEWGRAF
if(Offscreen) UnlockPixels(GetGWorldPixMap(gMainGWorld));
#endif
MyDisposeHandle((Handle*)&p_morespace);
MyDisposeHandle((Handle*)&p_top);
MyDisposeHandle((Handle*)&p_endx);
MyDisposeHandle((Handle*)&p_endy);
GraphicOn = FALSE;
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err DrawItem(). saveport == NULL");
return(rep);
}


InterruptDraw(int n, int interruptok)
{
int rep,compiledmem;

rep = OK;
if(TempMemory || (interruptok && Button())) {
	rep = FAILED;
	goto QUIT;
	}
if(EventState != NO) {
	rep = EventState;
	}
	
QUIT:
return(rep);
}


DrawObject(int j, Str255 label, double beta,int top, int hrect, int htext, int leftoffset,
	long pivloc, long t1, long t2, long trbeg, long trend,
	int *p_morespace,
	long *p_endx, long *p_endy, PicHandle picture)
{
Pattern pat;
Rect r,r1,r2,r3;
int tab,rep;
Point pt;
long x;
double xx,preperiod,objectperiod;

if(TempMemory) return(OK);
if(Panic || (picture != NULL && GraphOverflow(picture))) return(ABORT);
r.top = top; r.left = (int)t1 + leftoffset;
r.bottom = r.top + hrect; r.right = (int)t2 + leftoffset;
PenNormal();

// Erase background
InsetRect(&r,-2,-2);
EraseRect(&r);
// FillRect(&r,GetQDGlobalsWhite(&pat));

// Now draw rectangle
InsetRect(&r,2,2);
if(UseGraphicsColor) {
	RGBForeColor(&Color[SoundObjectC]);
	PenPat(GetQDGlobalsLightGray(&pat));
	}
else RGBForeColor(&White);
PaintRect(&r);
if(j >= Jbol && j < 16384) PenPat(GetQDGlobalsGray(&pat));
else PenNormal();
if(j < 16384) {
	if(UseGraphicsColor) RGBForeColor(&Color[TerminalC]);
	else RGBForeColor(&Black);
	}
else {
	if(UseGraphicsColor) RGBForeColor(&Color[NoteC]);
	else RGBForeColor(&Black);
	}
FrameRect(&r);
PenNormal();
RGBForeColor(&Black);

// Draw gray rectangles indicating truncated parts
if(trbeg > 0L) {
	r1.top = top; r1.left = r.left - (int)(trbeg);
	r1.bottom = r1.top + hrect; r1.right = r.left;
	PenPat(GetQDGlobalsGray(&pat));
	InsetRect(&r1,-2,-2);
	EraseRect(&r1);
//	FillRect(&r1,GetQDGlobalsWhite(&pat));
	InsetRect(&r1,2,2);
	FrameRect(&r1);
	PenNormal();
	}
if(trend > 0L) {
	r2.top = top; r2.left = r.right;
	r2.bottom = r2.top + hrect; r2.right = r2.left + (int)trend;
	PenPat(GetQDGlobalsGray(&pat));
	FrameRect(&r2);
	InsetRect(&r2,-2,-2);
	EraseRect(&r2);
//	FillRect(&r2,GetQDGlobalsWhite(&pat));
	InsetRect(&r2,2,2);
	FrameRect(&r2);
	PenNormal();
	}

// Draw period(s)
GetPeriod(j,beta,&objectperiod,&preperiod);
if(objectperiod > EPSILON) {
	objectperiod = (objectperiod * (double) GraphicScaleP) / GraphicScaleQ / 10.;
	preperiod = (preperiod * (double) GraphicScaleP) / GraphicScaleQ / 10.;
	xx = r.left - trbeg + preperiod;
	PenPat(GetQDGlobalsGray(&pat));
	while(objectperiod > 3 && xx < (r.right - 1)) {
		MoveTo((long)xx,top+1);
		Line(0,hrect-2);
		xx += objectperiod;
		}
	PenNormal();
	}

if(j < Jbol && (*p_Tref)[j] > EPSILON) {
	// Erase background above pivot
	MoveTo(r.left + (int) pivloc - 2,r.top - 8);
	PenPat(GetQDGlobalsWhite(&pat)); PenSize(1,4);
	Line(4,0);
	PenNormal();
	
	// Now draw vertical line of pivot (if non relocatable)
	MoveTo(r.left + (int) pivloc,r.top - 7);
	PenSize(2,2); 
	if(UseGraphicsColor) RGBForeColor(&Color[PivotC]);
	else RGBForeColor(&Black);
	if(j > 16383 || (*p_OkRelocate)[j]) Move(0,5);
	else Line(0,5);
	
	// Now draw arrow of pivot
	Line(-2,-2); Line(4,0); Line(-2,2);
	PenNormal();
	
	// The following is a line connecting the pivot to the object if needed
	MoveTo(r.left + (int) pivloc,r.top);
	LineTo(r.left - trbeg,r.top);
	}

// Draw label
RGBForeColor(&Black);
tab = (r.right - r.left - StringWidth(label)) / 2;
if(tab > 1) {
	MoveTo(r.left + tab,r.bottom - 4);
	DrawString(label);
	*p_endx = r.right + trend;
	*p_endy = r.bottom;
	}
else {	/* Can't write label inside rectangle */
	if(t1 == t2) { /* Draw out-time object */
		PenSize(2,2);
		MoveTo(r.left,r.top); LineTo(r.left,r.bottom - 1);
		PenNormal();
		}
	MoveTo(r.left + tab,r.bottom + htext - 2);
	GetPen(&pt); r.left = pt.h - 1; r.bottom = pt.v + 1;
	r.right = r.left + StringWidth(label) + 2;
	r.top = r.bottom - htext + 1;
	EraseRect(&r);
//	FillRect(&r,GetQDGlobalsWhite(&pat));
	DrawString(label);
	GetPen(&pt);
	*p_endx = (long) pt.h + 3;
	*p_endy = (long) pt.v + 2;
	*p_morespace = TRUE;
	}
return(OK);
}


DrawGraph(int w, PolyHandle p_graph)
{
GrafPtr saveport;

if(TempMemory) return(OK);
if(p_graph == NULL) {
	if(Beta) Alert1("Err. DrawGraph()");
	return(FAILED);
	}
GetPort(&saveport);
SetPortWindowPort(Window[w]);
FramePoly(p_graph);
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err DrawGraph(). saveport == NULL");
StopWait();
return(OK);
}


KillDiagrams(int w)
{
int n;

if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. KillDiagrams. w < 0 || w >= WMAX");
	return(OK);
	}
for(n=Ndiagram-1; n >= 0; n--) {
	if(n < 0 || n >= MAXDIAGRAM) {
		if(Beta) Alert1("Err. KillDiagrams. n < 0 || n >= MAXDIAGRAM");
		Ndiagram = 0;
		break;
		}
	if(DiagramWindow[n] == w && p_Diagram[n] != NULL) {
		KillPoly(p_Diagram[n]);
		p_Diagram[n] = NULL;
		if(n == Ndiagram-1) Ndiagram--;
		}
	}
if(w == wGraphic && NoteScalePicture != NULL) {
	KillPicture(NoteScalePicture);
	NoteScalePicture = NULL;
	}
if(!Offscreen) {
	for(n=Npicture-1; n >= 0; n--) {
		if(n < 0 || n >= MAXPICT) {
			if(Beta) Alert1("Err. KillDiagrams. n < 0 || n >= MAXPICT");
			Npicture = 0;
			break;
			}
		if(PictureWindow[n] == w && p_Picture[n] != NULL) {
			KillPicture(p_Picture[n]);
			p_Picture[n] = NULL;
			PictureWindow[n] = -1;
			if(n == Npicture-1) Npicture--;
			}
		}
	}

Hmin[w] = INT_MAX; Hmax[w] = - INT_MAX;
Hzero[w] = Vzero[w] = 0;
Vmin[w] = INT_MAX; Vmax[w] = - INT_MAX;
return(OK);
}


GraphOverflow(PicHandle picture)
{
Rect r;
int rep;

if(TempMemory || EmergencyExit /* || !ShowGraphic */) return(TRUE); // BB 20070529
if(picture == NULL) return(FALSE);
QDGetPictureBounds(picture, &r);	// requires 10.3 or later - akozar 20120603
if (EmptyRect(&r))  {
	if (Beta) Alert1("Err. GraphOverflow(): picFrame is empty");
	return(TRUE);
	}
if (QDError() == insufficientStackErr) {
	rep = Answer("Picture is too large. Disable graphics",'Y');
	if(rep == YES) {
		ShowGraphic = FALSE;
		SetButtons(TRUE);
		}
	return(TRUE);
	}
return(FALSE);
}


DrawSequence(int nseq,SoundObjectInstanceParameters **p_object,Milliseconds **p_t1,
	Milliseconds **p_t2,int kmax,unsigned long imax,unsigned long **p_imaxseq,
	int kmode,long **p_ddelta0,long **p_ddelta1,long **p_ddelta2)
{
int j,k,result;
Milliseconds t,tmin,tmax,**p_delta;
unsigned long i,im;

if(TempMemory || !ShowGraphic) return(OK);
tmin = Infpos; tmax = Infneg;
im = imax;
if(im >  (*p_imaxseq)[nseq]) im =  (*p_imaxseq)[nseq];
if((p_delta=(Milliseconds**) GiveSpace((Size) (im+1) * sizeof(Milliseconds))) == NULL)
	return(ABORT);
for(i=1L; i <= im; i++) {
	k = (*((*p_Seq)[nseq]))[i];
	(*p_delta)[i] = (*p_ddelta0)[i] + (*p_ddelta1)[i] + (*p_ddelta2)[i];
	if(k < 2) continue;
	if(kmode) {
		if(p_object == NULL) {
			if(Beta) Alert1("Err. DrawSequence(). p_object == NULL");
			return(ABORT);
			}
		if((t = (*p_object)[k].starttime - (*p_Instance)[k].truncbeg) < tmin) tmin = t;
		if((t = (*p_object)[k].endtime + (*p_Instance)[k].truncend) > tmax) tmax = t;
		}
	else {
		if((t = (*p_t1)[i] - (*p_Instance)[k].truncbeg) < tmin) tmin = t;
		if((t = (*p_t2)[i] + (*p_Instance)[k].truncend) > tmax) tmax = t;
		}
	}
result = OK;
result = DrawItem(wGraphic,p_object,p_t1,p_t2,kmax,tmin,tmax,im,nseq,nseq,p_imaxseq,
	kmode,FALSE,p_delta);
MyDisposeHandle((Handle*)&p_delta);
return(result);
}


DrawPrototype(int j,int w,Rect *p_frame)
{
Rect r,r1;
int jj,x,xmin,xmax,oldx,y,rep,htext,topoffset;
GrafPtr saveport;
long i,t,tmin,tmax,pivpos,maxbeggap,maxendgap;
double p,k,preroll,postroll,preperiod,objectperiod,scale;
Str255 label;
char line[BOLSIZE+5];
Milliseconds maxcover1,maxcover2,maxgap1,maxgap2,maxtrunc1,maxtrunc2,dur;
short oldfont,oldsize;
Pattern pat;

if(TempMemory) return(OK);
if(w < 0 || w >= WMAX) return(FAILED);
if(!GrafWindow[w]) {
	if(Beta) Alert1("Err. DrawPrototype(). !GrafWindow[w]");
	return(FAILED);
	}
if(j < 2 || j >= Jbol) return(OK);
rep = OK;

GetPort(&saveport);
SetPortWindowPort(Window[w]);
KillDiagrams(w);
if(p_Picture[1] != NULL) {
	KillPicture(p_Picture[1]);
	p_Picture[1] = NULL;
	}

r = (*p_frame);
ClipRect(&r);

p_Picture[1] = OpenPicture(&r);
PictureWindow[1] = w;
PictRect[1] = r;

oldsize = GetPortTextSize(GetWindowPort(Window[w]));
oldfont = GetPortTextFont(GetWindowPort(Window[w]));
TextFont(kFontIDCourier); TextSize(WindowTextSize[w]);
PenNormal();

EraseRect(&r);
// FillRect(&r,GetQDGlobalsWhite(&pat));
RGBForeColor(&Black);
PenSize(2,2);
FrameRect(&r);
PenNormal();
htext = WindowTextSize[w] + 2;
if((*p_MIDIsize)[j] <= ZERO && (*p_CsoundSize)[j] <= ZERO) {
	sprintf(Message,"This sound-object is empty");
	c2pstrcpy(label, Message);
	MoveTo((p_frame->left + p_frame->right - StringWidth(label))/2,
		(p_frame->top + p_frame->bottom - htext)/2);
	DrawString(label);
	if(Beta) {
		PenSize(3,3);
		for(i=0; i < MAXCHAN; i++) {
			RGBForeColor(&(PianoColor[i]));
			FrameRect(&r);
			InsetRect(&r,3,3);
			}
		}
	RGBForeColor(&Black);
	goto QUIT;
	}
else {
	InsetRect(&r,2,2);
	RGBForeColor(&White);
	PenMode(patCopy);
	PaintRect(&r);
	RGBForeColor(&Black);
	}
topoffset = 6 * htext;

GetPrePostRoll(j,&preroll,&postroll);

// Calculate leftmost date 'tmin'
if((*p_PivMode)[j] == RELATIVE) pivpos = ((*p_PivPos)[j] * (*p_Dur)[j]) / 100L;
else pivpos = (*p_PivPos)[j];
if(preroll < 0.) tmin = 0;
else tmin = - preroll;
if(pivpos < tmin) tmin = pivpos;
maxbeggap = -1L;
if((*p_ContBeg)[j]) {
	if((*p_ContBegMode)[j] == RELATIVE)
		maxbeggap = ((*p_MaxBegGap)[j] * (*p_Dur)[j]) / 100L;
	else maxbeggap = (*p_MaxBegGap)[j];
	}
if(maxbeggap > ZERO && maxbeggap < (*p_Dur)[j] && -maxbeggap < tmin)
	tmin = -maxbeggap;

// Calculate rightmost date 'tmax'
if(postroll >= ZERO) tmax = (*p_Dur)[j];
else tmax = (*p_Dur)[j] - postroll;
if(pivpos > tmax) tmax = pivpos;
maxendgap = -1L;
if((*p_ContEnd)[j]) {
	if((*p_ContEndMode)[j] == RELATIVE)
		maxendgap = ((*p_MaxEndGap)[j] * (*p_Dur)[j]) / 100L;
	else maxendgap = (*p_MaxEndGap)[j];
	}
if((maxendgap > ZERO) && maxendgap < (*p_Dur)[j] && ((*p_Dur)[j] + maxendgap) > tmax)
	tmax = (*p_Dur)[j] + maxendgap;
if((tmax - tmin) < ZERO) {
	if(Beta) Alert1("Err1. DrawPrototype()");
	rep = ABORT; goto QUIT;
	}
xmin = p_frame->left + 3 * htext;
xmax = p_frame->right - 3 * htext;

if((tmax - tmin) > EPSILON) {
	scale = ((double)(xmax - xmin)) / (tmax - tmin);
	}
else scale = 0.;

PenNormal();
RGBForeColor(&Black);

// Draw time ruler
y = r.top + htext + 2;
MoveTo(xmin,y); LineTo(xmax,y);
PenNormal();
p = 0.1;
if(scale > 0.001) {
	while((scale * p) < ((xmax - xmin) / 20.)) p = 10. * p;
	for(i=jj=0; ; i++,jj++) {
		x = xmin + (int)(i * scale * p);
		if(x > xmax) break;
		switch(jj) {
			case 0:
			case 10:
				jj = 0; PenSize(2,1);
				MoveTo(x,y); LineTo(x,y+6);
				PenNormal();
				break;
			case 5:
				MoveTo(x,y); LineTo(x,y+5);
				break;
			default:
				MoveTo(x,y); LineTo(x,y+3);
			}
		}
	y = r.top + htext + 1;
	sprintf(line,"0");
	c2pstrcpy(label, line);
	MoveTo(xmin - StringWidth(label)/2,y); DrawString(label);
	k = 20.; while((k * scale * p) > (xmax - xmin)) k = k / 2.;
	for(i=1; ; i++) {
		sprintf(line,"%.2fs",((double)i) * (k * p) / 1000.);
		c2pstrcpy(label, line);
		x = xmin + k * i * scale * p - StringWidth(label)/2;
		if(x > xmax) break;
		MoveTo(x,y);
		DrawString(label);
		}
	}
dur = (*p_Dur)[j];

RGBForeColor(&Black);

// Draw covered parts
maxcover1 = maxcover2 = dur;
if(!(*p_CoverBeg)[j]) {
	if((*p_CoverBegMode)[j] == ABSOLUTE) maxcover1 = (*p_MaxCoverBeg)[j];
	else maxcover1 = (dur * (*p_MaxCoverBeg)[j]) / 100.;
	}
if(!(*p_CoverEnd)[j]) {
	if((*p_CoverEndMode)[j] == ABSOLUTE) maxcover2 = (*p_MaxCoverEnd)[j];
	else maxcover2 = (dur * (*p_MaxCoverEnd)[j]) / 100.;
	}
r.top = p_frame->top + 2 * htext + 5;
r.bottom = r.top + htext;
r.left = p_frame->left + 2;
if(((*p_CoverBeg)[j] && (*p_CoverEnd)[j]) || (maxcover1 == dur && maxcover2 == dur)) {
	RGBForeColor(&Black); PenNormal();
	sprintf(line,"This object may be entirely covered");
	c2pstrcpy(label, line);
	MoveTo((xmin + xmax - StringWidth(label))/2,r.top + htext - 2);
	DrawString(label);
	}
else {
	if(maxcover1+maxcover2 > dur) {
		if(maxcover1 < dur) maxcover2 = dur - maxcover1;
		else maxcover1 = dur - maxcover2;
		}
	r.right = xmin + scale * (maxcover1 - tmin);
	if(maxcover1 < INT_MAX && maxcover1 > 0 && r.right > r.left) {
		RGBForeColor(&Color[SoundObjectC]); PenSize(2,2);
		FrameRect(&r);
		PenNormal(); RGBForeColor(&Black);
		MoveTo(r.left + 2,r.top + htext - 3);
		DrawString("\pCoverBeg");
		}
	else {
		MoveTo(r.left + 2,r.top + htext - 3);
		DrawString("\p#CoverBeg");
		}
	r.left = xmin + scale * (dur - tmin - maxcover2);
	r.right = p_frame->right - 2;
	if(maxcover2 < INT_MAX && maxcover2 > 0 && r.right > r.left) {
		RGBForeColor(&Color[SoundObjectC]); PenSize(2,2);
		FrameRect(&r);
		PenNormal(); RGBForeColor(&Black);
		MoveTo(p_frame->right - StringWidth("\pCoverEnd") - 2,r.top + htext - 3);
		DrawString("\pCoverEnd");
		}
	else {
		MoveTo(p_frame->right - StringWidth("\p#CoverEnd") - 2,r.top + htext - 3);
		DrawString("\p#CoverEnd");
		}
	}

// Draw continuity
maxgap1 = maxgap2 = INT_MAX;
if((*p_ContBeg)[j]) {
	if((*p_ContBegMode)[j] == ABSOLUTE) maxgap1 = (*p_MaxBegGap)[j];
	else maxgap1 = (dur * (*p_MaxBegGap)[j]) / 100.;
	}
if((*p_ContEnd)[j]) {
	if((*p_ContEndMode)[j] == ABSOLUTE) maxgap2 = (*p_MaxEndGap)[j];
	else maxgap2 = (dur * (*p_MaxEndGap)[j]) / 100.;
	}
r.top = p_frame->top + topoffset + 1;
r.bottom = r.top + htext - 2;
r.left = p_frame->left + 2;
r.right = xmin + scale * (- tmin - maxgap1);
if(maxgap1 < INT_MAX && r.right > r.left) {
	RGBForeColor(&Color[SoundObjectC]);
	PenPat(GetQDGlobalsGray(&pat));
	PenMode(patOr);
	PaintRect(&r);
	RGBForeColor(&Black);
	PenNormal();
	MoveTo(p_frame->left + 3,r.bottom + htext + 1);
	DrawString("\pContBeg");
	}
else {
	MoveTo(p_frame->left + 3,r.bottom + htext + 1);
	DrawString("\p#ContBeg");
	}
r.left = xmin + scale * (dur - tmin + maxgap2);
r.right = p_frame->right - 2;
if(maxgap2 < INT_MAX && r.right > r.left) {
	RGBForeColor(&Color[SoundObjectC]);
	PenPat(GetQDGlobalsGray(&pat));
	PenMode(patOr);
	PaintRect(&r);
	RGBForeColor(&Black);
	PenNormal();
	MoveTo(p_frame->right - StringWidth("\pContEnd") - 3,r.bottom + htext + 1);
	DrawString("\pContEnd");
	}
else {
	MoveTo(p_frame->right - StringWidth("\p#ContEnd") - 3,r.bottom + htext + 1);
	DrawString("\p#ContEnd");
	}

// Draw truncated parts
PenNormal();
RGBForeColor(&Blue);
maxtrunc1 = maxtrunc2 = dur;
if(!(*p_TruncBeg)[j]) {
	if((*p_TruncBegMode)[j] == ABSOLUTE) maxtrunc1 = (*p_MaxTruncBeg)[j];
	else maxtrunc1 = (dur * (*p_MaxTruncBeg)[j]) / 100.;
	}

if(!(*p_TruncEnd)[j]) {
	if((*p_TruncEndMode)[j] == ABSOLUTE) maxtrunc2 = (*p_MaxTruncEnd)[j];
	else maxtrunc2 = (dur * (*p_MaxTruncEnd)[j]) / 100.;
	}

y = p_frame->top + topoffset + 4 * htext;
if(maxtrunc1 > 0 && maxtrunc1 <= dur) {
	x = xmin + scale * (- tmin);
	MoveTo(x,y); LineTo(x,y - htext);
	x = xmin + scale * (- tmin + maxtrunc1);
	MoveTo(x,y); LineTo(x,y - htext);
	PenSize(2,2);
	y = p_frame->top + topoffset + 4 * htext - 1;
	x = xmin + scale * (- tmin);
	MoveTo(x,y); 
	LineTo(xmin + scale * (- tmin + maxtrunc1) - 1,y);
	PenNormal(); RGBForeColor(&Black);
	y += htext;
	MoveTo(xmin + scale * (- tmin) + 1,y);
	DrawString("\pTruncBeg");
	}
else {
	MoveTo(p_frame->left + 3,p_frame->top + topoffset + 5 * htext - 1);
	RGBForeColor(&Black); DrawString("\p#TruncBeg");
	}
y = p_frame->top + topoffset + 6 * htext;
if(maxtrunc2 > 0 && maxtrunc2 <= dur) {
	RGBForeColor(&Blue);
	x = xmin + scale * (dur - tmin);
	MoveTo(x,y); LineTo(x,y - htext);
	x = xmin + scale * (dur - tmin - maxtrunc2);
	MoveTo(x,y); LineTo(x,y - htext);
	PenSize(2,2);
	y = p_frame->top + topoffset + 6 * htext - 1;
	x = xmin + scale * (dur - tmin) - 1;
	MoveTo(x,y); 
	LineTo(xmin + scale * (dur - tmin - maxtrunc2),y);
	PenNormal(); RGBForeColor(&Black);
	y += htext;
	MoveTo(xmin + scale * (dur - tmin) - StringWidth("\pTruncEnd"),y);
	DrawString("\pTruncEnd");
	}
else {
	if(maxtrunc1 > 0 && maxtrunc1 <= dur) {
		y = p_frame->top + topoffset + 7 * htext - 1;
		MoveTo(p_frame->right - StringWidth("\p#TruncEnd") - 3,y);
		}
	else {
		y = p_frame->top + topoffset + 5 * htext - 1;
		MoveTo(p_frame->right - StringWidth("\p#TruncEnd") - 3,y);
		}
	RGBForeColor(&Black); DrawString("\p#TruncEnd");
	}

y += htext + htext;
x = p_frame->right - StringWidth("\p#BreakTempo") - 3;
MoveTo(x,y);
if((*p_BreakTempo)[j]) DrawString("\pBreakTempo");
else DrawString("\p#BreakTempo");

y += htext + htext;
x = p_frame->left + 4;
MoveTo(x,y);
if((*p_FixScale)[j]) DrawString("\pNever rescale");
else {
	if((*p_OkExpand)[j] && (*p_OkCompress)[j]) DrawString("\pRescale at will");
	else {
		if(!(*p_OkExpand)[j] && !(*p_OkCompress)[j]) DrawString("\pRescale within limits");
		else {
			if((*p_OkExpand)[j]) DrawString("\pExpand at will");
			else DrawString("\pDo not expand");
			y += htext + 2;
			x = p_frame->left + 4;
			MoveTo(x,y);
			if((*p_OkCompress)[j]) DrawString("\pCompress at will");
			else DrawString("\pDo not compress");
			}
		}
	}

r.top = p_frame->top + topoffset;
r.left = xmin + scale * (-tmin);
r.bottom = r.top + htext;
r.right = r.left + scale * dur;
if(r.right == r.left) r.right += 1;

// Draw period if any
GetPeriod(j,1.,&objectperiod,&preperiod);
if(objectperiod > EPSILON) {
	y = r.top - htext;
	x = r.left + (scale * preperiod);
	PenSize(1,2);
	RGBForeColor(&Cyan);
	MoveTo(x,y);
	LineTo(r.right,y);
	PenNormal();
	MoveTo(x,y);
	Line(0,htext);
	MoveTo(x+2,y);
	Line(0,htext);
	MoveTo(r.right,y);
	Line(0,htext);
	MoveTo(r.right-2,y);
	Line(0,htext);
	}

// Draw rectangle of object
RGBForeColor(&Color[SoundObjectC]);
PenPat(GetQDGlobalsGray(&pat));
PenMode(patOr);
PaintRect(&r);
PenPat(GetQDGlobalsBlack(&pat));
RGBForeColor(&Color[TerminalC]);
FrameRect(&r);
PenNormal();

// Show Csound messages

RGBForeColor(&Red);
t = - preroll;
oldx = -1;
for(i=0; i < (*p_CsoundSize)[j]; i++) {
	t += (*((*pp_CsoundTime)[j]))[i];
	x = r.left + scale * t;
	if(x > (oldx + 1)) {
		MoveTo(x,r.top-1); LineTo(x,r.bottom);
		oldx = x;
		}
	}
RGBForeColor(&Black);

// Show MIDI messages

RGBForeColor(&Black);
t = - preroll;
oldx = -1;
for(i=0; i < (*p_MIDIsize)[j]; i++) {
	t += (*((*pp_MIDIcode)[j]))[i].time;
	x = r.left + scale * t;
	if(x > (oldx + 1)) {
		MoveTo(x,r.top+1); LineTo(x,r.bottom-2);
		oldx = x;
		}
	}

// Draw label
sprintf(Message,"%s",*((*p_Bol)[j]));
c2pstrcpy(label, Message);
r1.left = r.left + (r.right - r.left - StringWidth(label)) / 2 - 1;
r1.right = r.right - (r.right - r.left - StringWidth(label)) / 2 - 1;
r1.top = r.top + 2;
r1.bottom = r.bottom - 2;
EraseRect(&r1);
// FillRect(&r1,GetQDGlobalsWhite(&pat));
MoveTo(r.left + (r.right - r.left - StringWidth(label)) / 2,r.bottom - 3);
DrawString(label);
	
if((*p_Tref)[j] > EPSILON) {
	// Draw vertical line of pivot (if non relocatable)
	MoveTo(r.left + (int) (scale*pivpos),r.top - 7);
	PenSize(2,2); 
	RGBForeColor(&Color[PivotC]);
	if(j > 16383 || (*p_OkRelocate)[j]) Move(0,5);
	else Line(0,5);
	
	// Draw arrow of pivot
	Line(-2,-2); Line(4,0); Line(-2,2);
	PenNormal();
	
	// The following is a line connecting the pivot to the object if needed
	MoveTo(r.left + (int) (scale*pivpos),r.top);
	LineTo(r.left,r.top);
	}
else {
	MoveTo(r.left,r.top - 1);
	DrawString("\pSmooth object: no pivot");
	}

// Draw vertical red line
if(xmax > xmin) {
	if((*p_Tpict)[iProto] == Infneg) {
		if(Hpos == -1) Hpos = xmin + scale * (- tmin);
		(*p_Tpict)[iProto] = tmin + ((Hpos - xmin) * (tmax - tmin))/ (xmax - xmin);
		}
	else Hpos = (((*p_Tpict)[iProto] - tmin) * scale) + xmin;
	if((*p_Tpict)[iProto] != ZERO) {
		MoveTo(Hpos,p_frame->top+1);
		RGBForeColor(&Red);
		LineTo(Hpos,p_frame->bottom - htext - htext);
		RGBForeColor(&Black);
		sprintf(Message,"%ld ms",(long)(*p_Tpict)[iProto]);
		c2pstrcpy(label, Message);
		if(Hpos + (StringWidth(label) / 2) > (p_frame->right - 2))
			x = p_frame->right - 2 - StringWidth(label);
		else x = Hpos - (StringWidth(label) / 2);
		if(x < (p_frame->left + 2)) x = p_frame->left + 2;
		MoveTo(x,p_frame->bottom - htext);
		DrawString(label);
		}
	}
RGBForeColor(&Black);

// Csound instrument status
if((*p_Type)[iProto] & 4) {
	if((*p_CsoundInstr)[iProto] > 0)
	   sprintf(Message,"Force to Csound instrument %ld",(long)(*p_CsoundInstr)[iProto]);
	else if((*p_CsoundInstr)[iProto] == 0) sprintf(Message,"Force to current Csound instrument");
		else sprintf(Message,"Never change Csound instruments");
	c2pstrcpy(label, Message);
	x = p_frame->right - 4 - StringWidth(label);
	MoveTo(x,p_frame->bottom - (4 * htext));
	DrawString(label);
	}
else {
	if((*p_CsoundAssignedInstr)[iProto] >= 1) {
		sprintf(Message,"Send MIDI messages to Csound instrument %ld",
			(long)(*p_CsoundAssignedInstr)[iProto]);
		c2pstrcpy(label, Message);
		x = p_frame->right - 4 - StringWidth(label);
		MoveTo(x,p_frame->bottom - (4 * htext));
		DrawString(label);
		}
	}

// MIDI channel status
if((*p_DefaultChannel)[iProto] > 0)
   sprintf(Message,"Force to MIDI channel %ld",(long)(*p_DefaultChannel)[iProto]);
else if((*p_DefaultChannel)[iProto] == 0) sprintf(Message,"Force to current MIDI channel");
	else sprintf(Message,"Never change MIDI channels");
c2pstrcpy(label, Message);
x = p_frame->right - 4 - StringWidth(label);
MoveTo(x,p_frame->bottom - (3 * htext));
DrawString(label);

// Object type
if(!((*p_Type)[iProto] & 1) && (*p_MIDIsize)[iProto] > ZERO) {
	RGBForeColor(&Red);
	sprintf(Message,"MIDI sequence is INACTIVE");
	c2pstrcpy(label, Message);
	x = p_frame->right - 4 - StringWidth(label);
	MoveTo(x,p_frame->bottom - htext - htext);
	DrawString(label);
	RGBForeColor(&Black);
	}

QUIT:
r = (*p_frame);
TextFont(oldfont); TextSize(oldsize);
ClosePicture();
DrawPicture(p_Picture[1],&r);
ValidWindowRect(Window[w], &r);
if(Npicture < 2) Npicture = 2;

GetWindowPortBounds(Window[w], &r);
ClipRect(&r);
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err DrawPrototype(). saveport == NULL");
return(rep);
}


OpenGraphic(int w,Rect *p_r,int showpen,CGrafPtr *p_port,GDHandle *p_gdh)
{
GWorldFlags flags;

if(Offscreen) {
#if NEWGRAF
	Rect rtemp;
	GetGWorld(p_port,p_gdh);
	flags = UpdateGWorld(&gMainGWorld,16,p_r,0,NULL,clipPix);
	if(!LockPixels(GetGWorldPixMap(gMainGWorld))) {
		Alert1("LockPixels Failed... ");
		return(ABORT);
		}
	SetGWorld(gMainGWorld,nil);
	SetPort(gMainGWorld);
	EraseRect(GetPortBounds(gMainGWorld, &rtemp));
#endif
	}
else {
	SetPortWindowPort(Window[w]);
	ClearWindow(TRUE,w);
	if(p_Picture[0] != NULL) {
		if(Beta) {
			Alert1("Err. DrawItem(). p_Picture[0] != NULL");
			}
		KillPicture(p_Picture[0]);
		p_Picture[0] = NULL;
		}
	p_Picture[0] = OpenPicture(p_r);
	if(Npicture < 1) Npicture = 1;
	PictureWindow[0] = w;
	PictRect[0] = (*p_r);
	}
if(showpen) ShowPen();
ClipRect(p_r);
return(OK);
}


CloseGraphic(int w,long endxmax,long endymax,int overflow,Rect *p_r,CGrafPtr *p_port,
	GDHandle gdh)
{
int rep,endxmin,endymin,newh,newv;
Rect rclip;
	
endxmin = endymin = 0;

RGBForeColor(&Black);
PenSize(1,1);
PenNormal();

GetWindowPortBounds(Window[w], &rclip);
if(OKhScroll[w]) rclip.bottom -= (SBARWIDTH + 1);
if(OKvScroll[w]) rclip.right -= (SBARWIDTH + 1);
ClipRect(&rclip);

ShowWindow(Window[w]);
SelectWindow(Window[w]);

if(!Offscreen) {
	ClosePicture();
	if(!overflow) {
		DrawPicture(p_Picture[0],p_r);
		if(GraphOverflow(p_Picture[0])) rep = OK;
		}
	else {
		if(Npicture > 0 && p_Picture[Npicture-1] != NULL) {
			KillPicture(p_Picture[Npicture-1]);
			p_Picture[Npicture-1] = NULL;
			PictureWindow[Npicture-1] = -1;
			if(Npicture == 1) Npicture--;
			}
		}
	}
else {
#if NEWGRAF
	if(!overflow) {	// FIXME ? Should we call LockPixels before CopyBits?
		Rect rtemp;
		RgnHandle cliprgn;
		SetGWorld((*p_port),gdh);
		GetPortBounds(gMainGWorld, &rtemp);
		cliprgn = NewRgn();	// FIXME: should check return value; is it OK to move memory here?
		GetPortClipRegion(GetWindowPort(Window[w]), cliprgn);
		CopyBits((BitMap*)*GetGWorldPixMap(gMainGWorld),             // was (BitMap*)*gMainGWorld->portPixMap,
			   (BitMap*)*GetPortPixMap(GetWindowPort(Window[w])),  // was (BitMap*)&(((CGrafPtr)Window[w])->portPixMap),
			   &rtemp,                                             // was &gMainGWorld->portRect,
			   &rtemp,                                             // was &gMainGWorld->portRect,
			   srcCopy,
			   cliprgn);                                           // was Window[w]->clipRgn
		DisposeRgn(cliprgn);
	}
#endif
}

newh = newv = FALSE;
if(endxmin < Hmin[w]) {
	Hmin[w] = 0;
	newh = TRUE;
	}
if(endxmax > Hmax[w]) Hmax[w] = endxmax;
if(endymin < Vmin[w]) {
	Vmin[w] = 0;
	newv = TRUE;
	}
if(endymax > Vmax[w]) Vmax[w] = endymax;

if(OKvScroll[w]) {
	SetControlValue(vScroll[w],0);
	ShowControl(vScroll[w]);
	}
if(OKhScroll[w]) {
	SetControlValue(hScroll[w],0);
	ShowControl(hScroll[w]);
	}

SlideH[w] = SlideV[w] = 0;
SetMaxControlValues(w,rclip);
if(newh) OffsetGraphs(w,Hmin[w] - rclip.left - 1,0);
if(newv) OffsetGraphs(w,0,Vmin[w] - rclip.top - 1);

GetWindowPortBounds(Window[w], &rclip);
ClipRect(&rclip);
DrawControls(Window[w]);
ValidWindowRect(Window[w], &rclip);
return(OK);
}


DrawItemBackground(Rect *p_r,unsigned long imax,int htext,int hrect,int leftoffset,
	int interruptok,Milliseconds **p_delta,long *p_yruler,int topoffset,int *p_overflow)
{
int result;
double x,xscale,p,rr,period;
long i,j,k,t1,t2,y,tmem1,tmem2,xmax;
char line[BOLSIZE+5],showsmalldivisions;
Str255 label;
Pattern pat;

result = OK;
RGBForeColor(&Black);
PenSize(1,1);
PenNormal();

// Draw scale ruler
x = 5. * ((double) GraphicScaleQ) / GraphicScaleP;	/* Duration on 500 pixels */
xscale = 1.;
if(x >= 10.) {
	while(x > 10.) {
		x = x / 10.; xscale = xscale * 10.;
		}
	}
else {
	while(x < 1.) {
		x = x * 10.; xscale = xscale / 10.;
		}
	}
xmax = p_r->right;

if(Nature_of_time == SMOOTH)
	while(imax > 1L && (*p_T)[imax] == ZERO) imax--;
t2 = ((double)(*p_T)[imax] * GraphicScaleP) / GraphicScaleQ / 10. + leftoffset;
// if(t2 < xmax) xmax = t2;
y = (*p_yruler) = p_r->top + htext + 2;
MoveTo(leftoffset,y); LineTo(xmax,y);
p = 50. / x;
for(i=j=0; ; i++,j++) {
	if(GraphOverflow(p_Picture[0])) {
		(*p_overflow) = TRUE;
		return(OK);
		}
	if((i % 10) == 0 && (result=InterruptDraw(0,interruptok)) != OK) {
		return(result);
		}
	t1 = leftoffset + (int) Round(i * p);
	if(t1 > xmax) break;
	switch(j) {
		case 0:
		case 10:
			j = 0; PenSize(2,1);
			MoveTo(t1,y); LineTo(t1,y+6);
			PenNormal();
			break;
		case 5:
			MoveTo(t1,y); LineTo(t1,y+5);
			break;
		default:
			MoveTo(t1,y); LineTo(t1,y+3);
		}
	}
y = p_r->top + htext + 1;
sprintf(line,"0");
c2pstrcpy(label, line);
MoveTo(leftoffset - StringWidth(label)/2,y); DrawString(label);
k = 10; if(k * p > 400) k = 1;
tmem2 = - Infpos;
for(i = 1; ; i++) {
	if(GraphOverflow(p_Picture[0])) {
		(*p_overflow) = TRUE;
		return(OK);
		}
	if((i % 10) == 0 && (result=InterruptDraw(0,interruptok)) != OK) {
		return(result);
		}
	if(k == 10) sprintf(line,"%.2fs",(double)i * xscale * k / 10.);
	if(k == 1) sprintf(line,"%.3fs",(double)i * xscale * k / 10.);
	c2pstrcpy(label, line);
	t1 = leftoffset + Round(k * i * p) - StringWidth(label)/2;
	t2 = t1 + StringWidth(label);
	if(t2 > xmax) break;
	if(t1 > tmem2) {
		MoveTo(t1,y);
		DrawString(label);
		tmem2 = t2 + StringWidth("\pmm");
		}
	}

// Draw time streaks

PenSize(1,1);
if(PlayPrototypeOn) goto ENDSTREAKS;
if(UseGraphicsColor) RGBForeColor(&Color[StreakC]);
else  RGBForeColor(&Black);
y = p_r->top + topoffset - hrect;

period = Kpress * Pclock * 1000. / Qclock / Ratio;
period = (period * GraphicScaleP) / GraphicScaleQ / 10;
if(period < 3) showsmalldivisions = FALSE;
else showsmalldivisions = TRUE;

tmem1 = tmem2 = - Infpos;
p = ((double) GraphicScaleP) / (GraphicScaleQ * 10.);
for(i=1L,rr=Ratio,k=0; i <= imax; i++,rr+=Kpress) {
	if(GraphOverflow(p_Picture[0])) {
		(*p_overflow) = TRUE;
		return(OK);
		}
	if((i % 10) == 0 && (result=InterruptDraw(0,interruptok)) != OK) {
		return(result);
		}
	PenPat(GetQDGlobalsGray(&pat));
	t1 = (*p_T)[i] / CorrectionFactor;
	if(p_delta != NULL) t1 += (*p_delta)[i];
	t1 = leftoffset + Round(t1 * p);
	if(rr >= Ratio) {
		rr -= Ratio;
		if(t1 > tmem1) {
			PenPat(GetQDGlobalsBlack(&pat));
			sprintf(line,"%ld",(long)(k + StartFromOne));
			c2pstrcpy(label, line);
			t2 = t1 + StringWidth(label)/2;
			if(t2 >= xmax) break;
			MoveTo(t1 - StringWidth(label)/2,y - 1);
			DrawString(label);
			tmem1 = t1 + StringWidth(label) + 4;
			}
		tmem2 = -Infpos;
		k++;
		}
	if((t1 > tmem2 && showsmalldivisions) || tmem2 == -Infpos) {
		MoveTo(t1,y);
		LineTo(t1,p_r->bottom);
		tmem2 = t1;
		}
	}

ENDSTREAKS:
PenNormal(); RGBForeColor(&Black);

return(result);
}


DrawPianoNote(int key,int nseq,int chan,Milliseconds timeoff,
	PerfParameters ****pp_currentparams,
	int leftoffset,int topoffset,int hrect,int minkey,int maxkey,Rect *p_r,int *p_overflow)
{
Milliseconds timeon;
long x,y;
Rect r;

if(*p_overflow) return(OK);
if(key < 0 || key > 127) {
	if(Beta) Alert1("Err. DrawPianoNote(). key < 0 || key > 127");
	return(OK);
	}
if(chan < 0 || chan >= MAXCHAN) {
	if(Beta) Alert1("Err. DrawPianoNote(). chan < 0 || chan >= MAXCHAN");
	return(OK);
	}
timeon = (*((*pp_currentparams)[nseq]))->starttime[key];
timeon = Round(((double)timeon * GraphicScaleP) / GraphicScaleQ / 10.);
timeoff = Round(((double)timeoff * GraphicScaleP) / GraphicScaleQ / 10.);

x = p_r->left + leftoffset + timeon;
y = (maxkey - key) * hrect + topoffset;

r.top = y;
r.bottom = y + hrect;
r.left = x - 1;
r.right = x;
EraseRect(&r);

MoveTo(x,y);
if(UseGraphicsColor) RGBForeColor(&(PianoColor[chan]));
Line(timeoff-timeon,0);
if(GraphOverflow(p_Picture[0])) {
	(*p_overflow) = TRUE; return(ABORT);
	}
return(OK);
}


DrawNoteScale(int w,int minkey,int maxkey,int hrect,int leftoffset,int topoffset)
{
int y,key;
Str255 label;
Rect therect,cliprect;
RgnHandle cliprgn;

// was cliprect = (*(Window[w]->clipRgn))->rgnBBox;
cliprgn = NewRgn();	// FIXME: should check return value; is it OK to move memory here?
GetPortClipRegion(GetWindowPort(Window[w]), cliprgn);
GetRegionBounds(cliprgn, &cliprect);
DisposeRgn(cliprgn);

therect.top = topoffset - 2 - hrect;
therect.left = 0;
therect.right = leftoffset - 2;
therect.bottom = (maxkey - minkey + 1) * hrect + topoffset + 2;

ClipRect(&therect);

NoteScalePicture = OpenPicture(&therect);
NoteScaleRect = therect;

PenSize(1,1);

if(UseGraphicsColor) RGBForeColor(&NoteScaleColor1);
else RGBForeColor(&Black);

EraseRect(&therect);
	
for(key=0; key < 128; key+=12) {
	if(key < minkey || key > maxkey) continue;
	y = (maxkey - key) * hrect + topoffset + 1;
	sprintf(Message,"c%ld",(long)((key - (key % 12))/12)-1L);
	c2pstrcpy(label, Message);
	MoveTo(2,y + 3);
	DrawString(label);
	}

if(UseGraphicsColor) RGBForeColor(&NoteScaleColor2);
else RGBForeColor(&Black);
for(key=6; key < 128; key+=12) {
	if(key < minkey || key > maxkey) continue;
	y = (maxkey - key) * hrect + topoffset + 1;
	sprintf(Message,"f#%ld",(long)((key - (key % 12))/12)-1L);
	c2pstrcpy(label, Message);
	MoveTo(2,y + 3);
	DrawString(label);
	}
RGBForeColor(&Black);
PenNormal();
ClosePicture();
DrawPicture(NoteScalePicture,&NoteScaleRect);

ClipRect(&cliprect);
return(OK);
}