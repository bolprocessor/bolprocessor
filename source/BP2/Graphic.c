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

extern FILE * imagePtr;

char graphic_scheme[] = "canvas";
int resize = 4; // Rescaling the image to get high resolution
int Lastx2[MAXKEY];

// Flags for debugging:
int try_pivots = 0;
int try_separate_labels = 0;
int try_synchro_tag = 0;
int show_more_details = 0;

int DrawItem(int w,SoundObjectInstanceParameters **p_object,Milliseconds **p_t1,
	Milliseconds **p_t2,int kmax,long tmin,long tmax,
	unsigned long imax,int nmin,int nmax,unsigned long **p_imaxseq,int kmode,
	int interruptok,
	Milliseconds **p_delta)
{
int linenum,linemax,maxlines,hrect,htext,morespace,**p_morespace,
	nseq,leftoffset,topoffset,rep,maxslideh,maxslidev,linemin,tab,key,
	edge,foundone,overflow,xc;
long pivloc,t1,tt1,t2,endxmax,endymax,endx,y,i,j,k,yruler,
	**p_endx,endy,**p_endy,**p_top,trbeg,trend;
Rect r, r2;
char label[BOLSIZE+5];
char line[BOLSIZE+5],line2[BOLSIZE+1],someline[200];
char* thisline;
unsigned char c;
p_list **waitlist;

if(!ShowObjectGraph || ShowPianoRoll) return(OK);

if(tmin == Infpos) {
	BPPrintMessage(odInfo,"Err. DrawObject(). tmin == Infpos\n");
	return(OK);
	}
if(CheckLoadedPrototypes() != OK) {
	BPPrintMessage(odInfo,"No sound-object prototypes have been loaded. Graphic is cancelled.\n");
	return(OK);
	}
BPPrintMessage(odInfo,"==> Yes, drawing graphics in ‘objects’ mode...\n");
if(show_more_details) BPPrintMessage(odInfo,"Jbol = %d Jpatt = %d\n",Jbol,Jpatt);

rep = OK;
GraphicOn = TRUE; overflow = FALSE;
maxlines = (int) Maxevent + 1;
Hzero[w] = Vzero[w] = 0;
Vmin[w] = INT_MAX; Vmax[w] = - 1;

if((p_morespace = (int**) GiveSpace((Size)maxlines*sizeof(int))) == NULL)
	return(ABORT);
if((p_endx = (long**) GiveSpace((Size)maxlines*sizeof(long))) == NULL)
	return(ABORT);
if((p_endy = (long**) GiveSpace((Size)maxlines*sizeof(long))) == NULL)
	return(ABORT);
if((p_top = (long**) GiveSpace((Size)maxlines*sizeof(long))) == NULL)
	return(ABORT);
	
hrect = 13;
htext = 12;
leftoffset = hrect - (tmin * GraphicScaleP) / GraphicScaleQ / 10;
topoffset = (4 * htext) + 8;
r.top = 0;
r.bottom = r.top + topoffset + Maxevent * (hrect + htext);
r.left = 0;
/* endxmax = leftoffset + ((tmax - tmin) * GraphicScaleP) / GraphicScaleQ / 10
	+ BOLSIZE * CharWidth('w'); */
endxmax = leftoffset + ((tmax - tmin) * GraphicScaleP) / GraphicScaleQ / 10
	+ BOLSIZE * 10;
if(show_more_details) BPPrintMessage(odInfo,"GraphicScaleP = %d GraphicScaleQ = %d tmin = %d tmax =%d endxmax = %d\n",GraphicScaleP,GraphicScaleQ,tmin,tmax,endxmax);
if(endxmax < 100) endxmax = 100;
r.right = r.left + endxmax;

// endxmax = leftoffset + 50 + ((tmax - tmin) * GraphicScaleP) / GraphicScaleQ / 10;
// WidthMax = (resize * endxmax) + 20;
WidthMax = 2 * endxmax + 20;
sprintf(Message,"WidthMax (2) = %ld\n",WidthMax);
BPPrintMessage(odInfo,Message);
// fputs(Message,imagePtr);

rep = DrawItemBackground(&r,imax,htext,hrect,leftoffset,interruptok,p_delta,&yruler,
	topoffset,&overflow);
if(rep != OK || overflow) goto ENDGRAPH;

// Now draw sound objects

linenum = 0; linemax = 1;
(*p_endx)[linenum] = endx = endxmax = endymax = 0;
(*p_top)[linenum] = topoffset;
(*p_top)[linemax] = (*p_top)[linenum] + hrect + htext;
(*p_morespace)[linenum] = linemin = 0;
// if(0 < Hmin[w]) Hmin[w] = 0;
// if(0 < Vmin[w]) Vmin[w] = 0;
Hmin[w] = 0;
Vmin[w] = 0;;
for(nseq = nmin; nseq <= nmax; nseq++) {
	if(show_more_details) BPPrintMessage(odInfo,"\nnseq = %d\n",nseq);
	foundone = FALSE;
	for(i=1; i < (*p_imaxseq)[nseq] && i <= imax; i++) {
		k = (*((*p_Seq)[nseq]))[i];
		if(show_more_details) BPPrintMessage(odInfo,"\nk = %d\n",k);
		if(k < 0) BPPrintMessage(odInfo,"Err. 'k' in DrawItem().\n");
		if(k < 2) continue;	/* Reject '_' and '-' */
		if(kmode) {
			if(show_more_details) BPPrintMessage(odInfo,"kmode = TRUE\n");
			if(p_object == NULL) {
				BPPrintMessage(odInfo,"Err. DrawObject(). p_object == NULL\n");
				return(ABORT);
				}
			t1 = (*p_object)[k].starttime;
			t2 = (*p_object)[k].endtime;
			}
		else {
			if(show_more_details) BPPrintMessage(odInfo,"kmode = FALSE\n");
			t1 = (*p_t1)[i];
			t2 = (*p_t2)[i];
			}
		t1 =  (t1 * GraphicScaleP) / GraphicScaleQ / 10;
		t2 =  (t2 * GraphicScaleP) / GraphicScaleQ / 10;
		j = (*p_Instance)[k].object; /* Beware: j < 0 for out-time objects */
		if(show_more_details) BPPrintMessage(odInfo,"j = %d t1 = %d t2 = %d endx =  %d\n",j,t1,t2,endx);
		trbeg = ((*p_Instance)[k].truncbeg * GraphicScaleP) / GraphicScaleQ / 10;
		tt1 = leftoffset + t1 - trbeg;
		trend = ((*p_Instance)[k].truncend * GraphicScaleP) / GraphicScaleQ / 10;
		if(try_synchro_tag || ((*p_ObjectSpecs)[k] != NULL && (waitlist=WaitList(k)) != NULL)) {
			// Draw synchronization tag
			y = yruler + 8;
			stroke_style("green");
			pen_size(8,0);
			for(edge=3; edge > 0; edge--) {
				draw_line(tt1,y,tt1 + edge,y + (2* edge),"");
				draw_line(tt1,y,tt1 - (2* edge),y,"");
				draw_line(tt1,y,tt1 + edge,y - (2* edge),"");
				}
			stroke_style("black");
			pen_size(1,0);
			}
#if SHOWEVERYTHING
		if(j == 0) continue;
#else
		if(j == 0 || j == 1 || j == -1) continue;
#endif
		if(show_more_details) BPPrintMessage(odInfo,"j = %d\n",j);
		if(j > 0) {
			if(j >= Jbol && j < 16384) sprintf(line,"%s",*((*p_Patt)[j-Jbol]));
			else {
				if(j < 16364) {
					sprintf(line,"%s",*((*p_Bol)[j]));
					if(show_more_details) BPPrintMessage(odInfo,"(*p_Bol)[%ld] = %s\n",(long)j,line);
					}
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
	//	strcpy(label,line);
		for(xc = 0; xc < strlen(line); xc++) {
			c = (unsigned char) line[xc];
			if(c == '"') label[xc] = "-"; // Quotes need to be replaced as they would jam fill_text()
			else label[xc] = line[xc];
			}
		label[xc] = '\0';
		tab = ((int) t2 - (int) t1 - strlen(line)) / 2;
		if(tab < 2) tt1 = (int) t1 + leftoffset + 1 + tab;
		sprintf(Message,"t1 = %ld tt1= %ld endx = %ld\n",(long)t1,(long)tt1,(long) (*p_endx)[linenum]);
		if(show_more_details) BPPrintMessage(odInfo,Message);
	//	if(endx < t2) endx = t2;
		if(tt1 < (*p_endx)[linenum]) {
			for(linenum=linemin; linenum < linemax; linenum++) {
				if(tt1 > (*p_endx)[linenum]) goto CONT;
				}
			linemax = linenum + 1;	/* here 'linenum' has been incremented */
			foundone = TRUE;
			sprintf(Message,"=> New linenum = %ld\n",(long)linenum);
			if(show_more_details) BPPrintMessage(odInfo,Message);
			if(linenum >= maxlines) {
				sprintf(Message,
					"Err. linenum = %ld  maxlines = %ld  DrawItem()\n",
					(long)linenum,(long)maxlines);
		//		if(Beta) Alert1(Message);
				BPPrintMessage(odInfo,Message);
				rep = ABORT;
				goto ENDGRAPH;
				}
			endx = (*p_endx)[linenum] = 0;
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
	
		sprintf(Message,"Running DrawObject(%s) for linenum = %ld, endx = %ld and top = %ld\n",label,(long)linenum,(long)endx,(long)(*p_top)[linenum]);
		if(show_more_details) BPPrintMessage(odInfo,Message);
				
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
		
		sprintf(Message,"linenum = %ld, morespace[%ld] = %ld\n",(long)linenum,(long)linenum,(long)(*p_morespace)[linenum]);
		if(show_more_details) BPPrintMessage(odInfo,Message);
		}
//	linenum = linemin = linemax;
	linenum = linemax;
	if(!foundone) (*p_top)[linenum] = (*p_top)[linenum-1] + hrect
							+ htext * (1 + (*p_morespace)[linenum-1]);
	linemax += 1 /* foundone */;
	if(linenum >= maxlines) {
		sprintf(Message,
			"Err. linenum = %ld  maxlines = %ld in DrawItem()\n",
			(long)linenum,(long)maxlines);
	//	if(Beta) Alert1(Message);
		BPPrintMessage(odInfo,Message);
		rep = ABORT;
		goto ENDGRAPH;
		}
	endx = (*p_endx)[linenum] = 0;
	(*p_morespace)[linenum] = 0;
	}

ENDGRAPH:

QUIT:
// WidthMax = (resize * endxmax) + 20;
WidthMax = 2 * endxmax + 20;
sprintf(Message,"WidthMax (1) = %ld\n",WidthMax);
BPPrintMessage(odInfo,Message);
// fputs(Message,imagePtr);

HeightMax = 2 * endymax + 20;
sprintf(Message,"HeightMax (1) = %ld\n",HeightMax);
BPPrintMessage(odInfo,Message);

MyDisposeHandle((Handle*)&p_morespace);
MyDisposeHandle((Handle*)&p_top);
MyDisposeHandle((Handle*)&p_endx);
MyDisposeHandle((Handle*)&p_endy);
GraphicOn = FALSE;

EndImageFile();
return(rep);
}


int InterruptDraw(int n, int interruptok)
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


int DrawObject(int j, char *label, double beta,int top, int hrect, int htext, int leftoffset,
	long pivloc, long t1, long t2, long trbeg, long trend, int *p_morespace, long *p_endx, long *p_endy, PicHandle picture)
{
// Pattern pat;
Rect r,r1,r2,r3;
int tab,rep,x_startpivot,y_startpivot;
Point pt;
long x;
double xx,preperiod,objectperiod;

r.top = top;
r.left = (int)t1 + leftoffset;
r.right = (int)t2 + leftoffset;
r.bottom = r.top + hrect;
sprintf(Message,"j = %ld, leftoffset = %ld,  t1 = %ld, t2 = %ld, endx = %ld\n",(long)j,leftoffset,t1,t2,(*p_endx));
if(show_more_details) BPPrintMessage(odInfo,Message);

// Erase background 
r2 = r;
resize_rect(&r2,+2,+2);
erase_rect(&r2);

// Draw box
pen_size(4,0);
stroke_style("black");
stroke_rect(&r);

sprintf(Message,"j = %ld, r.left = %ld, r.right = %ld, r.top = %ld, r.bottom = %ld\n",(long)j,(long)r.left,(long)r.right,(long)r.top,(long)r.bottom);
if(show_more_details) BPPrintMessage(odInfo,Message);
r2 = r;
resize_rect(&r2,-1,-1);
sprintf(Message,"j = %ld, r2.left = %ld, r2.right = %ld, r2.top = %ld, r2.bottom = %ld\n",(long)j,(long)r2.left,(long)r2.right,(long)r2.top,(long)r2.bottom);
if(show_more_details) BPPrintMessage(odInfo,Message);
if(j >= Jbol && j < 16384) { // Time pattern
	fill_rect(&r2,"LightCyan");
	}
else fill_rect(&r2,"Cornsilk");

// Draw gray rectangles indicating truncated parts
if(trbeg > 0L) {
	r1.top = top;
	r1.left = r.left - (int)(trbeg);
	r1.bottom = r1.top + hrect;
	r1.right = r.left;
	resize_rect(&r1,-2,-2);
	erase_rect(&r1);
	resize_rect(&r1,2,2);
	stroke_style("grey");
	stroke_rect(&r1);
	stroke_style("black");
	}
if(trend > 0L) {
	r2.top = top; r2.left = r.right;
	r2.bottom = r2.top + hrect;
	r2.right = r2.left + (int)trend;
	resize_rect(&r2,-2,-2);
	erase_rect(&r2);
	resize_rect(&r2,2,2);
	stroke_style("grey");
	stroke_rect(&r2);
	stroke_style("black");
	}

// Draw period(s)
GetPeriod(j,beta,&objectperiod,&preperiod);
if(objectperiod > EPSILON) {
	objectperiod = (objectperiod * (double) GraphicScaleP) / GraphicScaleQ / 10.;
	preperiod = (preperiod * (double) GraphicScaleP) / GraphicScaleQ / 10.;
	xx = r.left - trbeg + preperiod;
	stroke_style("grey");
	while(objectperiod > 3 && xx < (r.right - 1)) {
		draw_line(xx,top+1,xx,(top+1+hrect-2),"");
		xx += objectperiod;
		}
	stroke_style("black");
	}

// Draw pivot
if(try_pivots || (j < Jbol && (*p_Tref)[j] > EPSILON)) {
	x_startpivot = r.left + (int) pivloc;
	
	// Erase background above pivot
	pen_size(25,0); 
	stroke_style("white");
	y_startpivot = r.top - 8;
	draw_line((x_startpivot - 2),y_startpivot,(x_startpivot + 2),y_startpivot,"");
	
	// Now draw vertical line of pivot (if non relocatable)
	y_startpivot = r.top - 7;
	pen_size(6,0); 
	stroke_style("red");
	if(j > 16383 || (*p_OkRelocate)[j]) {
		draw_line((x_startpivot-2),(y_startpivot+3),(x_startpivot+2),(y_startpivot+3),"round");
		}
	else {
		draw_line(x_startpivot,y_startpivot,x_startpivot,(y_startpivot + 5),"");
		sprintf(Message,"Pivot x = %ld, y = %ld\n",(long)x_startpivot,(long)y_startpivot);
		if(show_more_details) BPPrintMessage(odInfo,Message);
		}
	
	// Now draw arrow of pivot
	y_startpivot += 5;
	draw_line(x_startpivot,y_startpivot,(x_startpivot-2),(y_startpivot - 2),"");
	draw_line(x_startpivot,y_startpivot,(x_startpivot+2),(y_startpivot - 2),"");
	pen_size(1,0);
	
	// The following is a line connecting the pivot to the object if needed
	y_startpivot = r.top;
	draw_line(x_startpivot,y_startpivot,(r.left - trbeg),y_startpivot,"");
	stroke_style("black");
	}

// Draw label

tab = (r.right - r.left - 6 * strlen(label)) / 2;
*p_endx = t2 + leftoffset + trend;
if(!try_separate_labels &&  tab > 1) {
	fill_text(label,(r.left + tab),r.bottom - 2);
	// *p_endx = t2 + leftoffset + trend;
	*p_endy = r.bottom;
	}
else {	/* Can't write label inside rectangle */
	if(t1 == t2) { /* Draw out-time object */
		pen_size(10,0);
		draw_line(r.left,r.top,r.left,(r.bottom + 1),"round");
		pen_size(1,0);
		}
//	tab = (r.right - r.left) / 2;
/*	r.left = r.left + tab - 3 * strlen(label);
	r.right = r.left + 3 * strlen(label); */
	
	r.right += 6 * strlen(label);
	r.top = r.bottom + 1;
	r.bottom = r.top + htext;
	
	erase_rect(&r);
	fill_text(label,r.left,r.bottom);
	if(*p_endx < (r.right + 1)) *p_endx = r.right + 1;
	// *p_endx = t2 + leftoffset + trend;
	*p_endy = r.bottom + 10;
	*p_morespace += 1; // Instead of 1 because we used an extra line for labels
	}
return(OK);
}


int DrawGraph(int w, PolyHandle p_graph)
{
// GrafPtr saveport;

// if(TempMemory) return(OK);
if(p_graph == NULL) {
//	if(Beta) Alert1("Err. DrawGraph()");
	return(FAILED);
	}
/* GetPort(&saveport);
SetPortWindowPort(Window[w]);
FramePoly(p_graph); */
/* if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err DrawGraph(). saveport == NULL"); */
// StopWait();
return(OK);
}


int KillDiagrams(int w)
{
int n;

w = wGraphic;
/* if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("Err. KillDiagrams. w < 0 || w >= WMAX");
	return(OK);
	} */
for(n=Ndiagram-1; n >= 0; n--) {
	if(n < 0 || n >= MAXDIAGRAM) {
	//	if(Beta) Alert1("Err. KillDiagrams. n < 0 || n >= MAXDIAGRAM");
		Ndiagram = 0;
		break;
		}
	if(DiagramWindow[n] == w && p_Diagram[n] != NULL) {
	//	KillPoly(p_Diagram[n]);
		p_Diagram[n] = NULL;
		if(n == Ndiagram-1) Ndiagram--;
		}
	}
if(w == wGraphic && NoteScalePicture != NULL) {
	// KillPicture(NoteScalePicture);
	NoteScalePicture = NULL;
	}
if(!Offscreen) {
	for(n=Npicture-1; n >= 0; n--) {
		if(n < 0 || n >= MAXPICT) {
		//	if(Beta) Alert1("Err. KillDiagrams. n < 0 || n >= MAXPICT");
			Npicture = 0;
			break;
			}
		if(PictureWindow[n] == w && p_Picture[n] != NULL) {
		//	KillPicture(p_Picture[n]);
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


int GraphOverflow(PicHandle picture)
{
return(OK);
/* Rect r;
int rep;

if(TempMemory || EmergencyExit) return(TRUE); // BB 20070529
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
return(FALSE); */
}


int DrawSequence(int nseq,SoundObjectInstanceParameters **p_object,Milliseconds **p_t1,
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


int DrawPrototype(int j,int w,Rect *p_frame)
{
Rect r,r1;
int jj,x,xmin,xmax,oldx,y,rep,htext,topoffset;
// GrafPtr saveport;
long i,t,tmin,tmax,pivpos,maxbeggap,maxendgap;
double p,k,preroll,postroll,preperiod,objectperiod,scale;
Str255 label;
char line[BOLSIZE+5];
Milliseconds maxcover1,maxcover2,maxgap1,maxgap2,maxtrunc1,maxtrunc2,dur;
short oldfont,oldsize;
// Pattern pat;

// if(TempMemory) return(OK);
if(w < 0 || w >= WMAX) return(FAILED);
if(!GrafWindow[w]) {
//	if(Beta) Alert1("Err. DrawPrototype(). !GrafWindow[w]");
	return(FAILED);
	}
if(j < 2 || j >= Jbol) return(OK);
rep = OK;

// GetPort(&saveport);
// SetPortWindowPort(Window[w]);
// KillDiagrams(w);
if(p_Picture[1] != NULL) {
//	KillPicture(p_Picture[1]);
	p_Picture[1] = NULL;
	}

/* r = (*p_frame);
ClipRect(&r); */

// p_Picture[1] = OpenPicture(&r);
PictureWindow[1] = w;
PictRect[1] = r;

/* oldsize = GetPortTextSize(GetWindowPort(Window[w]));
oldfont = GetPortTextFont(GetWindowPort(Window[w]));
TextFont(kFontIDCourier); TextSize(WindowTextSize[w]);
PenNormal();

erase_rect(&r); */
// FillRect(&r,GetQDGlobalsWhite(&pat));
/* stroke_style(&Black);
pen_size(2,2);
FrameRect(&r);
PenNormal(); */
// htext = WindowTextSize[w] + 2;
htext = 12 + 2;
if((*p_MIDIsize)[j] <= ZERO && (*p_CsoundSize)[j] <= ZERO) {
	sprintf(Message,"Sound-object is empty");
//	c2pstrcpy(label, Message);
/*	move_to((p_frame->left + p_frame->right - strlen(label))/2,
		(p_frame->top + p_frame->bottom - htext)/2);
	fill_text(label); */
/*	if(Beta) {
		pen_size(3,3);
		for(i=0; i < MAXCHAN; i++) {
			stroke_style(&(PianoColor[i]));
			FrameRect(&r);
			resize_rect(&r,3,3);
			}
		} */
//	stroke_style(&Black);
	goto QUIT;
	}
else {
/*	resize_rect(&r,2,2);
	stroke_style(&White);
	PenMode(patCopy);
	PaintRect(&r);
	stroke_style(&Black); */
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
//	if(Beta) Alert1("Err1. DrawPrototype()");
	rep = ABORT; goto QUIT;
	}
xmin = p_frame->left + 3 * htext;
xmax = p_frame->right - 3 * htext;

if((tmax - tmin) > EPSILON) {
	scale = ((double)(xmax - xmin)) / (tmax - tmin);
	}
else scale = 0.;

/* PenNormal();
stroke_style(&Black); */

// Draw time ruler
y = r.top + htext + 2;
/* move_to(xmin,y); line_to(xmax,y);
PenNormal(); */
p = 0.1;
if(scale > 0.001) {
	while((scale * p) < ((xmax - xmin) / 20.)) p = 10. * p;
	for(i=jj=0; ; i++,jj++) {
		x = xmin + (int)(i * scale * p);
		if(x > xmax) break;
		switch(jj) {
			case 0:
			case 10:
				jj = 0; /* pen_size(2,1);
				move_to(x,y); line_to(x,y+6);
				PenNormal(); */
				break;
			case 5:
			//	move_to(x,y); line_to(x,y+5);
				break;
			default:
			//	move_to(x,y); line_to(x,y+3);
				break;
			}
		}
	y = r.top + htext + 1;
	sprintf(line,"0");
//	c2pstrcpy(label, line);
//	move_to(xmin - strlen(label)/2,y); fill_text(label);
	k = 20.; while((k * scale * p) > (xmax - xmin)) k = k / 2.;
	for(i=1; ; i++) {
		sprintf(line,"%.2fs",((double)i) * (k * p) / 1000.);
		// c2pstrcpy(label, line);
		x = xmin + k * i * scale * p - strlen(line)/2;
		if(x > xmax) break;
	//	move_to(x,y);
	//	fill_text(label);
		}
	}
dur = (*p_Dur)[j];

// stroke_style(&Black);

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
//	stroke_style(&Black); PenNormal();
	sprintf(line,"This object may be entirely covered");
// c2pstrcpy(label, line);
//	move_to((xmin + xmax - strlen(line))/2,r.top + htext - 2);
//	fill_text(label);
	}
else {
	if(maxcover1+maxcover2 > dur) {
		if(maxcover1 < dur) maxcover2 = dur - maxcover1;
		else maxcover1 = dur - maxcover2;
		}
	r.right = xmin + scale * (maxcover1 - tmin);
	if(maxcover1 < INT_MAX && maxcover1 > 0 && r.right > r.left) {
	//	stroke_style(&Color[SoundObjectC]); pen_size(2,2);
	//	FrameRect(&r);
	//	PenNormal(); stroke_style(&Black);
	//	move_to(r.left + 2,r.top + htext - 3);
	//	fill_text("\pCoverBeg");
		}
	else {
	//	move_to(r.left + 2,r.top + htext - 3);
	//	fill_text("\p#CoverBeg");
		}
	r.left = xmin + scale * (dur - tmin - maxcover2);
	r.right = p_frame->right - 2;
	if(maxcover2 < INT_MAX && maxcover2 > 0 && r.right > r.left) {
	/*	stroke_style(&Color[SoundObjectC]); pen_size(2,2);
		FrameRect(&r);
		PenNormal(); stroke_style(&Black);
		move_to(p_frame->right - strlen("CoverEnd") - 2,r.top + htext - 3);
		fill_text("\pCoverEnd"); */
		}
	else {
	/*	move_to(p_frame->right - strlen("#CoverEnd") - 2,r.top + htext - 3);
		fill_text("\p#CoverEnd"); */
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
/*	stroke_style(&Color[SoundObjectC]);
	PenPat(GetQDGlobalsGray(&pat));
	PenMode(patOr);
	PaintRect(&r);
	stroke_style(&Black);
	PenNormal();
	move_to(p_frame->left + 3,r.bottom + htext + 1);
	fill_text("\pContBeg"); */
	}
else {
/*	move_to(p_frame->left + 3,r.bottom + htext + 1);
	fill_text("\p#ContBeg"); */
	}
r.left = xmin + scale * (dur - tmin + maxgap2);
r.right = p_frame->right - 2;
if(maxgap2 < INT_MAX && r.right > r.left) {
/*	stroke_style(&Color[SoundObjectC]);
	PenPat(GetQDGlobalsGray(&pat));
	PenMode(patOr);
	PaintRect(&r);
	stroke_style(&Black);
	PenNormal();
	move_to(p_frame->right - strlen("ContEnd") - 3,r.bottom + htext + 1);
	fill_text("\pContEnd"); */
	}
else {
/*	move_to(p_frame->right - strlen("#ContEnd") - 3,r.bottom + htext + 1);
	fill_text("\p#ContEnd"); */
	}

// Draw truncated parts
/* PenNormal();
stroke_style(&Blue); */
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
//	move_to(x,y); line_to(x,y - htext);
	x = xmin + scale * (- tmin + maxtrunc1);
/*	move_to(x,y); line_to(x,y - htext);
	pen_size(2,2); */
	y = p_frame->top + topoffset + 4 * htext - 1;
	x = xmin + scale * (- tmin);
/*	move_to(x,y); 
	line_to(xmin + scale * (- tmin + maxtrunc1) - 1,y);
	PenNormal(); stroke_style(&Black); */
	y += htext;
/*	move_to(xmin + scale * (- tmin) + 1,y);
	fill_text("\pTruncBeg"); */
	}
else {
/*	move_to(p_frame->left + 3,p_frame->top + topoffset + 5 * htext - 1);
	stroke_style(&Black); fill_text("\p#TruncBeg"); */
	}
y = p_frame->top + topoffset + 6 * htext;
if(maxtrunc2 > 0 && maxtrunc2 <= dur) {
//	stroke_style(&Blue);
	x = xmin + scale * (dur - tmin);
//	move_to(x,y); line_to(x,y - htext);
	x = xmin + scale * (dur - tmin - maxtrunc2);
/*	move_to(x,y); line_to(x,y - htext);
	pen_size(2,2); */
	y = p_frame->top + topoffset + 6 * htext - 1;
	x = xmin + scale * (dur - tmin) - 1;
/*	move_to(x,y); 
	line_to(xmin + scale * (dur - tmin - maxtrunc2),y);
	PenNormal(); stroke_style(&Black); */
	y += htext;
/*	move_to(xmin + scale * (dur - tmin) - strlen("TruncEnd"),y);
	fill_text("\pTruncEnd"); */
	}
else {
	if(maxtrunc1 > 0 && maxtrunc1 <= dur) {
		y = p_frame->top + topoffset + 7 * htext - 1;
	//	move_to(p_frame->right - strlen("#TruncEnd") - 3,y);
		}
	else {
		y = p_frame->top + topoffset + 5 * htext - 1;
	//	move_to(p_frame->right - strlen("#TruncEnd") - 3,y);
		}
//	stroke_style(&Black); fill_text("\p#TruncEnd");
	}

y += htext + htext;
x = p_frame->right - strlen("BreakTempo") - 3;
/* move_to(x,y);
if((*p_BreakTempo)[j]) fill_text("\pBreakTempo");
else fill_text("\p#BreakTempo"); */

y += htext + htext;
x = p_frame->left + 4;
// move_to(x,y);
if((*p_FixScale)[j]) {} // fill_text("\pNever rescale");
else {
	if((*p_OkExpand)[j] && (*p_OkCompress)[j]) {} //  fill_text("\pRescale at will");
	else {
		if(!(*p_OkExpand)[j] && !(*p_OkCompress)[j]) {} //  fill_text("\pRescale within limits");
		else {
			if((*p_OkExpand)[j]) {} //  fill_text("\pExpand at will");
			else  {} // fill_text("\pDo not expand");
			y += htext + 2;
			x = p_frame->left + 4;
		//	move_to(x,y);
		/*	if((*p_OkCompress)[j]) fill_text("\pCompress at will");
			else fill_text("\pDo not compress"); */
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
/*	pen_size(1,2);
	stroke_style(&Cyan);
	move_to(x,y);
	line_to(r.right,y);
	PenNormal();
	move_to(x,y);
	Line(0,htext);
	move_to(x+2,y);
	Line(0,htext);
	move_to(r.right,y);
	Line(0,htext);
	move_to(r.right-2,y);
	Line(0,htext); */
	}

// Draw rectangle of object
/* stroke_style(&Color[SoundObjectC]);
PenPat(GetQDGlobalsGray(&pat));
PenMode(patOr);
PaintRect(&r);
PenPat(GetQDGlobalsBlack(&pat));
stroke_style(&Color[TerminalC]);
FrameRect(&r);
PenNormal(); */

// Show Csound messages

// stroke_style(&Red);
t = - preroll;
oldx = -1;
for(i=0; i < (*p_CsoundSize)[j]; i++) {
	t += (*((*pp_CsoundTime)[j]))[i];
	x = r.left + scale * t;
	if(x > (oldx + 1)) {
	//	move_to(x,r.top-1); line_to(x,r.bottom);
		oldx = x;
		}
	}
// stroke_style(&Black);

// Show MIDI messages

// stroke_style(&Black);
t = - preroll;
oldx = -1;
for(i=0; i < (*p_MIDIsize)[j]; i++) {
	t += (*((*pp_MIDIcode)[j]))[i].time;
	x = r.left + scale * t;
	if(x > (oldx + 1)) {
	//	move_to(x,r.top+1); line_to(x,r.bottom-2);
		oldx = x;
		}
	}

// Draw label
sprintf(Message,"%s",*((*p_Bol)[j]));
// c2pstrcpy(label, Message);
r1.left = r.left + (r.right - r.left - strlen(line)) / 2 - 1;
r1.right = r.right - (r.right - r.left - strlen(line)) / 2 - 1;
r1.top = r.top + 2;
r1.bottom = r.bottom - 2;
// erase_rect(&r1);
// FillRect(&r1,GetQDGlobalsWhite(&pat));
/* move_to(r.left + (r.right - r.left - strlen(label)) / 2,r.bottom - 3);
fill_text(label); */
	
if((*p_Tref)[j] > EPSILON) {
	// Draw vertical line of pivot (if non relocatable)
/*	move_to(r.left + (int) (scale*pivpos),r.top - 7);
	pen_size(2,2); 
	stroke_style(&Color[PivotC]);
	if(j > 16383 || (*p_OkRelocate)[j]) Move(0,5);
	else Line(0,5); */
	
	// Draw arrow of pivot
/*	Line(-2,-2); Line(4,0); Line(-2,2);
	PenNormal(); */
	
	// The following is a line connecting the pivot to the object if needed
/*	move_to(r.left + (int) (scale*pivpos),r.top);
	line_to(r.left,r.top); */
	}
else {
/*	move_to(r.left,r.top - 1);
	fill_text("\pSmooth object: no pivot"); */
	}

// Draw vertical red line
if(xmax > xmin) {
	if((*p_Tpict)[iProto] == Infneg) {
		if(Hpos == -1) Hpos = xmin + scale * (- tmin);
		(*p_Tpict)[iProto] = tmin + ((Hpos - xmin) * (tmax - tmin))/ (xmax - xmin);
		}
	else Hpos = (((*p_Tpict)[iProto] - tmin) * scale) + xmin;
	if((*p_Tpict)[iProto] != ZERO) {
	/*	move_to(Hpos,p_frame->top+1);
		stroke_style(&Red);
		line_to(Hpos,p_frame->bottom - htext - htext);
		stroke_style(&Black); */
		sprintf(Message,"%ld ms",(long)(*p_Tpict)[iProto]);
	//	c2pstrcpy(label, Message);
		if(Hpos + (strlen(line) / 2) > (p_frame->right - 2))
			x = p_frame->right - 2 - strlen(line);
		else x = Hpos - (strlen(line) / 2);
		if(x < (p_frame->left + 2)) x = p_frame->left + 2;
	/*	move_to(x,p_frame->bottom - htext);
		fill_text(label); */
		}
	}
// stroke_style(&Black);

// Csound instrument status
if((*p_Type)[iProto] & 4) {
	if((*p_CsoundInstr)[iProto] > 0)
	   sprintf(Message,"Force to Csound instrument %ld",(long)(*p_CsoundInstr)[iProto]);
	else if((*p_CsoundInstr)[iProto] == 0) sprintf(Message,"Force to current Csound instrument");
		else sprintf(Message,"Never change Csound instruments");
//	c2pstrcpy(label, Message);
	x = p_frame->right - 4 - strlen(line);
/*	move_to(x,p_frame->bottom - (4 * htext));
	fill_text(label); */
	}
else {
	if((*p_CsoundAssignedInstr)[iProto] >= 1) {
		sprintf(Message,"Send MIDI messages to Csound instrument %ld",
			(long)(*p_CsoundAssignedInstr)[iProto]);
	//	c2pstrcpy(label, Message);
		x = p_frame->right - 4 - strlen(line);
	/*	move_to(x,p_frame->bottom - (4 * htext));
		fill_text(label); */
		}
	}

// MIDI channel status
if((*p_DefaultChannel)[iProto] > 0)
   sprintf(Message,"Force to MIDI channel %ld",(long)(*p_DefaultChannel)[iProto]);
else if((*p_DefaultChannel)[iProto] == 0) sprintf(Message,"Force to current MIDI channel");
	else sprintf(Message,"Never change MIDI channels");
// c2pstrcpy(label, Message);
x = p_frame->right - 4 - strlen(line);
/* move_to(x,p_frame->bottom - (3 * htext));
fill_text(label); */

// Object type
if(!((*p_Type)[iProto] & 1) && (*p_MIDIsize)[iProto] > ZERO) {
//	stroke_style(&Red);
	sprintf(Message,"MIDI sequence is INACTIVE");
//	c2pstrcpy(label, Message);
	x = p_frame->right - 4 - strlen(line);
/*	move_to(x,p_frame->bottom - htext - htext);
	fill_text(label);
	stroke_style(&Black); */
	}

QUIT:
r = (*p_frame);
/* TextFont(oldfont); TextSize(oldsize);
ClosePicture();
DrawPicture(p_Picture[1],&r);
ValidWindowRect(Window[w], &r); */
if(Npicture < 2) Npicture = 2;

/* GetWindowPortBounds(Window[w], &r);
ClipRect(&r);
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err DrawPrototype(). saveport == NULL"); */
return(rep);
}


/*  int OpenGraphic(int w,Rect *p_r,int showpen,CGrafPtr *p_port,GDHandle *p_gdh)
{
// GWorldFlags flags;

if(Offscreen) {

	Rect rtemp;
	GetGWorld(p_port,p_gdh);
	flags = UpdateGWorld(&gMainGWorld,16,p_r,0,NULL,clipPix);
	if(!LockPixels(GetGWorldPixMap(gMainGWorld))) {
		Alert1("LockPixels Failed... ");
		return(ABORT);
		}
	SetGWorld(gMainGWorld,nil);
	SetPort(gMainGWorld);
	erase_rect("canvas"GetPortBounds(gMainGWorld, &rtemp));

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
} */


/*  int CloseGraphic(int w,long endxmax,long endymax,int overflow,Rect *p_r,CGrafPtr *p_port,
	GDHandle gdh)
{
int rep,endxmin,endymin,newh,newv;
Rect rclip;
	
endxmin = endymin = 0;

stroke_style(&Black);
pen_size(1,0);
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
	} */

/* newh = newv = FALSE;
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
} */


int DrawItemBackground(Rect *p_r,unsigned long imax,int htext,int hrect,int leftoffset,
	int interruptok,Milliseconds **p_delta,long *p_yruler,int topoffset,int *p_overflow)
{
int result;
double x,xscale,p,rr,period;
long i,j,k,t1,t2,y,tmem1,tmem2,tmem3,xmax,ymax,y_curr;
char line[BOLSIZE+5],showsmalldivisions,line_image[200];
Str255 label;

result = OK;
pen_size(1,0);
text_style(htext,"arial");


ymax = p_r->bottom;
if(TRUE || ShowPianoRoll) {
	HeightMax = 2 * ymax + 20;
	sprintf(Message,"HeightMax (2) = %ld\n",HeightMax);
	BPPrintMessage(odInfo,Message);
	}

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

sprintf(Message,"DrawItemBackground() imax= %ld, xmax = %ld\n",(long)imax,(long)xmax);
if(show_more_details) BPPrintMessage(odInfo,Message);

if(Nature_of_time == SMOOTH)
	while(imax > 1L && (*p_T)[imax] == ZERO) imax--;
t2 = ((double)(*p_T)[imax] * GraphicScaleP) / GraphicScaleQ / 10. + leftoffset;

y = (*p_yruler) = p_r->top + htext + 6;
pen_size(1,0);
draw_line(leftoffset,y,xmax,y,"");
p = 50. / x;
for(i=j=0; ; i++,j++) {
	t1 = leftoffset + (int) Round(i * p);
	if(t1 > xmax) break;
	switch(j) {
		case 0:
		case 10:
			j = 0;
			pen_size(4,0);
			draw_line(t1,y,t1,y+6,"");
			break;
		case 5:
			pen_size(1,0);
			draw_line(t1,y,t1,y+5,"");
			break;
		default:
			pen_size(1,0);
			draw_line(t1,y,t1,y+3,"");
			break;
		}
	}
y = p_r->top + htext + 1;
sprintf(line,"0");
fill_text(line,leftoffset - 6,y);
k = 10; if(k * p > 400) k = 1;
tmem2 = - Infpos;
for(i = 1; ; i++) {
	if(k == 10) sprintf(line,"%.2fs",(double)i * xscale * k / 10.);
	if(k == 1) sprintf(line,"%.3fs",(double)i * xscale * k / 10.);
	t1 = leftoffset + Round(k * i * p) - 10;
	t2 = t1 + 12;
	if(t2 > xmax) break;
	if(t1 > tmem2) {
		fill_text(line,t1,y);
	//	tmem2 = t2 + strlen("mm");
		tmem2 = t2 + 48;
		}
	}

// Draw time streaks

if(PlayPrototypeOn) goto ENDSTREAKS;
y = p_r->top + topoffset - hrect;
xmax -= 40;

period = Kpress * Pclock * 1000. / Qclock / Ratio;
period = (period * GraphicScaleP) / GraphicScaleQ / 10;
if(period < 3) showsmalldivisions = FALSE;
else showsmalldivisions = TRUE;

tmem1 = tmem2 = tmem3 = - Infpos;
p = ((double) GraphicScaleP) / (GraphicScaleQ * 10.);
stroke_style("blue");
fill_style("blue");

// Subdivisional time streaks
for(i=1L,rr=Ratio,k=0; i <= imax; i++, rr += Kpress) {
	t1 = (*p_T)[i] / CorrectionFactor;
	if(p_delta != NULL) t1 += (*p_delta)[i];
	t1 = leftoffset + Round(t1 * p);
	if((t1 > tmem1 && showsmalldivisions && t1 < xmax) || tmem1 == -Infpos) {
		pen_size(1,0);
		draw_line(t1,y,t1,ymax,"");
		tmem1 = t1 + 20;
		}
	}

// We draw horizontal white lines to break time subdivisional time streaks
pen_size(1,0);
stroke_style("white");
for(y_curr = y; y_curr < ymax; y_curr  += 6) {
	draw_line(leftoffset,y_curr,xmax,y_curr,"");
	}

// Major time streaks
pen_size(1,0);
stroke_style("blue");
text_style(10,"arial");
for(i=1L,rr=Ratio,k=0; i <= imax; i++, rr += Kpress) {
	t1 = (*p_T)[i] / CorrectionFactor;
	if(p_delta != NULL) t1 += (*p_delta)[i];
	t1 = leftoffset + Round(t1 * p);
	if(rr >= Ratio) {
		rr -= Ratio;
		if(t1 > tmem2  && t1 < xmax) {
			sprintf(line,"%ld",(long)(k + StartFromOne));
			t2 = t1 + 12;
			if(t1 > tmem3) {
				fill_text(line,t1 - 5,y - 4);
				tmem3 = t1 + 7 * strlen(line);
				}
			pen_size(4,0);
			draw_line(t1,y,t1,ymax,"");
			tmem2 = t1 + 20;
			}
		k++;
		}
	}
text_style(htext,"arial");
stroke_style("black");

ENDSTREAKS:
return(result);
}


int DrawPianoNote(int key,int nseq,int chan,Milliseconds timeoff,
	PerfParameters ****pp_currentparams,
	int leftoffset,int topoffset,int hrect,int minkey,int maxkey,Rect *p_r,int *p_overflow)
{
Milliseconds timeon;
int x1, x2, y;
Rect r;
char* word;
int length;

if(key < 0 || key > 127) {
	return(OK);
	}
if(chan < 0 || chan >= MAXCHAN) {
	return(OK);
	}
timeon = (*((*pp_currentparams)[nseq]))->starttime[key] * 1000.;
timeon = Round(((double)timeon * GraphicScaleP) / GraphicScaleQ / 10.);
timeoff = Round(((double)timeoff * GraphicScaleP) / GraphicScaleQ / 10.);

x1 = p_r->left + leftoffset + timeon;y = (maxkey - key - 1) * hrect + topoffset;
x2 = x1 + timeoff - timeon;
if((x1 - Lastx2[key]) < 4) {
	x1 += 2; // Separate identical notes when replayed
	}
length = x2 - x1;
if(length < 8) {
	x2 += 2; // For out-time objects
	x1 -= 2;
	}
Lastx2[key] = x2;
pen_size(8,0);
stroke_style("brown");
draw_line(x1,y,x2,y,"");
return(OK);
}


int DrawNoteScale(Rect* p_r,int w,int minkey,int maxkey,int hrect,int leftoffset,int topoffset)
{
int y,key,xmin,xmax;
char line[20];

pen_size(2,0);
xmin = p_r->left + 41;
xmax = p_r->right - 28;
stroke_style("rgb(0,117,117)");
fill_style("black");
text_style(11,"arial");
for(key=0; key < 128; key+=12) {
	if(key < minkey || key > maxkey) continue;
	y = (maxkey - key) * hrect + topoffset;
	// sprintf(line,"C%ld",(long)((key - (key % 12))/12)-1L);
	PrintNote(key,0,-1,line);
	fill_text(line,2,y + 3);
	fill_text(line,xmax+ 7,y + 3);
	draw_line(xmin,y,xmax,y,"");
	}
stroke_style("rgb(186,186,0)");
fill_style("rgb(186,186,0)");
for(key=6; key < 128; key+=12) {
	if(key < minkey || key > maxkey) continue;
	y = (maxkey - key) * hrect + topoffset;
	// sprintf(line,"F#%ld",(long)((key - (key % 12))/12)-1L);
	PrintNote(key,0,-1,line);
	fill_text(line,2,y + 3);
	fill_text(line,xmax + 7,y + 3);
	draw_line(xmin,y,xmax,y,"");
	}
text_style(12,"arial");
stroke_style("black");
fill_style("black");
return(OK);
}

// ------- GRAPHIC CONVERSION -------

void begin_path() {
	char line[100];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		sprintf(line,"ctx.beginPath();\n");
		fputs(line,imagePtr);
		}
	}

void end_path() {
	char line[100];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		sprintf(line,"ctx.stroke();\n");
		fputs(line,imagePtr);
		}
	}
	
void move_to(int x,int y) {
	char line[100];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		sprintf(line,"ctx.moveTo(%ld,%ld);\n",(long)resize * x,(long)resize * y);
		fputs(line,imagePtr);
		}
	}

void line_to(int x,int y) {
	char line[100];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		sprintf(line,"ctx.lineTo(%ld,%ld);\n",(long)resize * x,(long)resize * y);
		fputs(line,imagePtr);
		}
	}

void draw_line(int x1,int y1,int x2,int y2,char* style) {
	char line[200];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		if(strcmp(style,"round") == 0)
			sprintf(line,"ctx.beginPath();\nctx.lineCap = \"round\";\nctx.moveTo(%ld,%ld);\nctx.lineTo(%ld,%ld);\nctx.stroke();\n",(long)resize * x1,(long)resize * y1,(long)resize * x2,(long)resize * y2);
		else
			sprintf(line,"ctx.beginPath();\nctx.moveTo(%ld,%ld);\nctx.lineTo(%ld,%ld);\nctx.stroke();\n",(long)resize * x1,(long)resize * y1,(long)resize * x2,(long)resize * y2);
		fputs(line,imagePtr);
		}
	}
	
void pen_size(int x,int y) {
	char line[100];
	float ratio;
	if(strcmp(graphic_scheme,"canvas") == 0) {
		ratio = (float) x + (0.1 * (float) y);
		sprintf(line,"ctx.lineWidth = %ld;\n",(long)ratio);
		fputs(line,imagePtr);
		}
	}

void stroke_style(char* color) {
	char line[100];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		sprintf(line,"ctx.strokeStyle = '%s';\n",color);
		fputs(line,imagePtr);
		}
	}
	
void fill_style(char* color) {
	char line[100];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		sprintf(line,"ctx.fillStyle = '%s';\n",color);
		fputs(line,imagePtr);
		}
	}

void stroke_text(char* txt,int x,int y) {
	char line[500];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		sprintf(line,"ctx.strokeText('%s',%ld,%ld);\n",txt,(long)resize * x,(long)resize * y);
		fputs(line,imagePtr);
		}
	}

void fill_text(char* txt,int x,int y) {
	char line[500];
	if(show_more_details) BPPrintMessage(odInfo,"text = %s\n",txt);
	if(strcmp(graphic_scheme,"canvas") == 0) {
		sprintf(line,"ctx.fillText(\"%s\",%ld,%ld);\n",txt,(long)resize * x,(long)resize * y);
		fputs(line,imagePtr);
		}
	}
	
void stroke_rect(Rect* p_r) {
	char line[100];
	int x1,x2,y1,y2,w,h;
	if(strcmp(graphic_scheme,"canvas") == 0) {
		x1 = resize * p_r->left;
		x2 = resize * p_r->right;
		y1 = resize * p_r->bottom;
		y2 = resize * p_r->top;
		w = x2 - x1;
		h = y2 - y1;
		sprintf(line,"ctx.strokeRect(%ld,%ld,%ld,%ld);\n",(long)x1,(long)y1,(long)w,(long)h);
		fputs(line,imagePtr);
		}
	}
		
void fill_rect(Rect* p_r,char* color) {
	char line[100];
	int x1,x2,y1,y2,w,h;
	if(strcmp(graphic_scheme,"canvas") == 0) {
		x1 = resize * p_r->left;
		x2 = resize * p_r->right;
		y1 = resize * p_r->bottom;
		y2 = resize * p_r->top;
		w = x2 - x1;
		h = y2 - y1;
		sprintf(line,"ctx.fillStyle = '%s';\n",color);
		fputs(line,imagePtr);
		sprintf(line,"ctx.fillRect(%ld,%ld,%ld,%ld);\n",(long)x1,(long)y1,(long)w,(long)h);
		fputs(line,imagePtr);
		sprintf(line,"ctx.fillStyle = 'black';\n");
		fputs(line,imagePtr);
		}
	}

void text_style(int size,char* font) {
	char line[100];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		sprintf(line,"ctx.font = \"%ldpx %s\";\n",(long)(resize *size),font);
		fputs(line,imagePtr);
		}
	}

void erase_rect(Rect* p_r) {
	char line[200];
	int x1,x2,y1,y2;
	if(strcmp(graphic_scheme,"canvas") == 0) {
		x1 = resize * p_r->left;
		x2 = resize * p_r->right;
		y1 = resize * p_r->bottom;
		y2 = resize * p_r->top;
		sprintf(line,"ctx.fillStyle = 'white';\nctx.fillRect(%ld,%ld,%ld,%ld);\nctx.fillStyle = 'black';\n",(long)x1,(long)y1,(long)(x2 - x1),(long)(y2 - y1));
		fputs(line,imagePtr);
		}
	}

void resize_rect(Rect* p_r,int w,int h) {
	p_r->left = p_r->left - w;
	p_r->right = p_r->right + w;
	p_r->top = p_r->top - h;
	p_r->bottom = p_r->bottom + h;
	}