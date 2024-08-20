/* Graphic.c (BP3) */

/*  This file is a part of Bol Processor 
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

#define SHOWEVERYTHING 0

extern FILE * imagePtr;

char graphic_scheme[] = "canvas";
int resize = 4; // Rescaling the image to get high resolution
long max_coordinate = 16383; // Nobody knows why this fits! Added by BB 2021-03-23 
int Lastx2[MAXKEY];

// Flags for debugging:
int trace_graphic = 0;
int try_pivots = 0;
int try_separate_labels = 0;
int try_synchro_tag = 0;
int trace_draw_piano_note = 0;

int DrawItem(int w,SoundObjectInstanceParameters **p_object,Milliseconds **p_t1,
	Milliseconds **p_t2,long kmax,long tmin,long tmax,
	unsigned long imax,int nmin,int nmax,unsigned long **p_imaxseq,int kmode,
	int interruptok,
	Milliseconds **p_delta) {
	int linenum,old_linenum,linemax,maxlines,hrect,htext,morespace,**p_morespace,
		nseq,leftoffset,topoffset,rep,maxslideh,maxslidev,linemin,tab,key,
		edge,foundone,overflow,xc,scale,i_scale,result,moved_up;
	long pivloc,t1,tt1,t2,endxmax,endymax,endx,y,i,j,k,yruler,
		**p_endx,endy,**p_endy,**p_top,trbeg,trend;
	Rect r, r2;
	char label[BOLSIZE+5];
	char line[BOLSIZE+5],line2[BOLSIZE+1],someline[200];
	char* thisline;
	unsigned char c;
	p_list **waitlist;

    if(!ShowGraphic) return OK;
	if(!ShowObjectGraph) return(OK);
	// BPPrintMessage(odInfo,"Creating image %d based on objects\n",N_image);

	if(tmin == Infpos) {
		BPPrintMessage(odError,"=> Err. DrawObject(). tmin == Infpos\n");
		return(OK);
		}
	if(CheckLoadedPrototypes() != OK) {
		BPPrintMessage(odInfo,"No sound-object prototypes have been loaded. Graphic is cancelled.\n");
		return(OK);
		}
	// BPPrintMessage(odInfo,"Drawing graphics in ‘objects’ mode...\n");
	if(trace_graphic) BPPrintMessage(odInfo,"Jbol = %d Jpatt = %d\n",Jbol,Jpatt);

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
	// BPPrintMessage(odInfo,"hrect = %d, tmin = %ld, leftoffset = %d\n",hrect,(long)tmin,leftoffset);
	if(leftoffset < 20) {
		if(leftoffset < -10) BPPrintMessage(odError,"=> Fixed leftoffset = %d\n",leftoffset);
		leftoffset = 20; // Added BB 2021-03-13
		}
	topoffset = (4 * htext) + 8;
	r.top = 0;
	r.bottom = r.top + topoffset + Maxevent * (hrect + htext);
	r.left = 0;
	endxmax = leftoffset + ((tmax - tmin) * GraphicScaleP) / GraphicScaleQ / 10
		+ BOLSIZE * 10;
	if(trace_graphic) BPPrintMessage(odInfo,"GraphicScaleP = %d GraphicScaleQ = %d tmin = %d tmax =%d endxmax = %d\n",GraphicScaleP,GraphicScaleQ,tmin,tmax,endxmax);
	if(endxmax < 100) endxmax = 100;

	if(WidthMax < 32000) WidthMax = 2 * endxmax + 40;
	//	BPPrintMessage(odInfo,"WidthMax (2) = %ld\n",WidthMax);
	if(WidthMax > 32000) {
		BPPrintMessage(odInfo,"\nImage width %ld was too large: it has been cropped to 32000\n",(long)WidthMax);
		WidthMax = endxmax = 32000;
		}
	r.right = r.left + endxmax;

	rep = DrawItemBackground(&r,imax,htext,hrect,leftoffset,interruptok,p_delta,&yruler,
		topoffset,&overflow,"objects");
	if(rep != OK || overflow) goto ENDGRAPH;

	// Now draw sound objects

	linenum = linemin = 0; linemax = 1;
	(*p_endx)[linenum] = (*p_endx)[linemax] = endx = endxmax = endymax = 0;
	(*p_top)[linenum] = topoffset;
	(*p_morespace)[linenum] = (*p_morespace)[linemax] = 0;
	// if(0 < Hmin[w]) Hmin[w] = 0;
	// if(0 < Vmin[w]) Vmin[w] = 0;
	Hmin[w] = 0;
	Vmin[w] = 0;
	if(trace_graphic) BPPrintMessage(odInfo,"\ntopoffset = %ld nmax = %d maxlines = %d\n",(long)topoffset,nmax,maxlines);
	for(nseq = nmin; nseq <= nmax; nseq++) {
		if(trace_graphic) BPPrintMessage(odInfo,"\nnseq = %d\n",nseq);
		foundone = FALSE;
	//	for(i=1; i < (*p_imaxseq)[nseq] && i <= imax; i++) {
		for(i=ZERO; i < (*p_imaxseq)[nseq] && i <= imax; i++) { // Fixed by BB 2021-03-20
			k = (*((*p_Seq)[nseq]))[i];
			if(trace_graphic) {
				if(k > 0) BPPrintMessage(odInfo,"k = %d \n",k);
			//	else BPPrintMessage(odInfo,"_");
				}
			if(k < 0) BPPrintMessage(odError,"=> Err. 'k' in DrawItem(): nseq = %ld, i = %ld, k = %ld\n",(long)nseq,(long)i,(long)k);
			if(k < 2) continue;	/* Reject '_' and '-' */
			if(kmode) {
				if(trace_graphic) BPPrintMessage(odInfo,"kmode = TRUE\n");
				if(p_object == NULL) {
					BPPrintMessage(odError,"=> Err. DrawObject(). p_object == NULL\n");
					return(ABORT);
					}
				t1 = (*p_object)[k].starttime;
				t2 = (*p_object)[k].endtime;
				}
			else {
				if(trace_graphic) BPPrintMessage(odInfo,"kmode = FALSE\n");
				t1 = (*p_t1)[i];
				t2 = (*p_t2)[i];
				}
			if(trace_graphic) BPPrintMessage(odInfo,"GraphicScaleP = %ld GraphicScaleQ = %ld, t1 = %ld t2 = %ld\n",GraphicScaleP,GraphicScaleQ,(long)t1,(long)t2);
			t1 =  (t1 * GraphicScaleP) / GraphicScaleQ / 10;
			t2 =  (t2 * GraphicScaleP) / GraphicScaleQ / 10;
			j = (*p_Instance)[k].object; /* Beware: j < 0 for out-time objects */
			if(trace_graphic) BPPrintMessage(odInfo,"j = %ld t1 = %d t2 = %d endx = %ld\n",(long)j,t1,t2,(long)endx);
			trbeg = ((*p_Instance)[k].truncbeg * GraphicScaleP) / GraphicScaleQ / 10;
			tt1 = leftoffset + t1 - trbeg;
			trend = ((*p_Instance)[k].truncend * GraphicScaleP) / GraphicScaleQ / 10;
			if(try_synchro_tag || ((*p_ObjectSpecs)[k] != NULL && (waitlist=WaitList(k)) != NULL)) {
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
			if(OutCsound || MIDImicrotonality) {
				scale = (*p_Instance)[k].scale;
				if(scale < 0) i_scale = -1;
				else if(scale == 0) {
					i_scale = DefaultScale;
					if(trace_scale) BPPrintMessage(odInfo,"Default scale will be used\n");
					}
				else {
					if(trace_graphic) BPPrintMessage(odInfo,"j = %d k = %d scale = %d NumberScales = %d\n",j,k,scale,NumberScales);
					for(i_scale = 1; i_scale <= NumberScales; i_scale++) {
					// ‘scale’ is not the index of the scale. It is the index of its name in StringConstant
						if(trace_graphic) BPPrintMessage(odInfo,"scale = %d  i_scale = %d label = %s StringConstant = %s\n",scale,i_scale,(*Scale)[i_scale].label,(*p_StringConstant)[scale]);
						result = MyHandlecmp((*p_StringConstant)[scale],(*Scale)[i_scale].label);
						if(trace_graphic) BPPrintMessage(odInfo,"i_scale = %d (*p_StringConstant)[scale] = %s result = %d\n",i_scale,*((*p_StringConstant)[scale]),result);
						if(result == 0) break;
						}
					if(i_scale > NumberScales) i_scale = -1;
					}
				}
			else i_scale = -1;
			if(j > 0) {
				if(j >= Jbol && j < 16384) my_sprintf(line,"%s",*((*p_Patt)[j-Jbol]));
				else {
					if(j < 16364) {
						my_sprintf(line,"%s",*((*p_Bol)[j]));
						if(trace_graphic) BPPrintMessage(odInfo,"(*p_Bol)[%ld] = %s\n",(long)j,line);
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
						PrintNote(i_scale,key,0,-1,line);
						if(trace_scale) BPPrintMessage(odInfo,"DrawItem i_scale = %d\n",i_scale);
						}
					}
				}
			else {
				j = -j;
				if(j < 16364) my_sprintf(line,"<<%s>>",*((*p_Bol)[j]));
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
						PrintNote(i_scale,key,0,-1,line2);
						strcat(line,line2);
						strcat(line,">>");
						}
				}
			if(SHOWEVERYTHING) {
				my_sprintf(line2," #%ld",(long)k);
				strcat(line,line2);
				}
			for(xc = 0; xc < strlen(line); xc++) {
				c = (unsigned char) line[xc];
				if(c == '"') label[xc] = '-';
				else label[xc] = line[xc];
				}
			label[xc] = '\0';
			tab = ((int) t2 - (int) t1 - strlen(line)) / 2;
			if(tab < 2) tt1 = (int) t1 + leftoffset + 1 + tab;
			if(trace_graphic) BPPrintMessage(odInfo,"t1 = %ld t2 = %ld tt1= %ld (*p_endx)[%ld] = %ld\n",(long)t1,(long)t2,(long)tt1,(long)linenum,(long)(*p_endx)[linenum]);
		//	if(endx < t2) endx = t2;
			old_linenum = linenum;
		//	if(tt1 < (*p_endx)[linenum]) { // Fixed by BB 2021-01-30 = Now we try to push up every object
				for(linenum=linemin; linenum <= linemax; linenum++) { 
					if(trace_graphic) BPPrintMessage(odInfo,"+ linenum = %ld linemax = %ld (*p_morespace)[linenum] = %ld, (*p_endx)[linenum] = %ld\n",(long)linenum,(long)linemax,(long)(*p_morespace)[linenum],(long)(*p_endx)[linenum]);
			//		if(tt1 > (*p_endx)[linenum]) goto CONT;
					if(tt1 >= (*p_endx)[linenum]) goto CONT; // Fixed by BB 2021-02-03
					}
				linemax = linenum; // Fixed by BB 2021-01-30
				(*p_endx)[linemax] = 0; // Fixed by BB 2021-01-30
				(*p_morespace)[linenum] = 0;
			//	linemax = linenum + 1;
				foundone = TRUE;
				if(trace_graphic) BPPrintMessage(odInfo,"New linenum = %ld\n",(long)linenum);
				if(linenum >= maxlines) {
					BPPrintMessage(odError,"=> Err. (1) linenum = %ld  maxlines = %ld  DrawItem()\n",
						(long)linenum,(long)maxlines);
					rep = ABORT;
					goto ENDGRAPH;
					}
				endx = (*p_endx)[linenum] = 0;
		//		}
	CONT:
			if(old_linenum != linenum) {
		//	if(linenum < linemax) {
				if(trace_graphic) BPPrintMessage(odInfo,"Moved from line %d to line %d\n",old_linenum,linenum);
				moved_up =  TRUE;
				}
			else moved_up = FALSE;
			if(linenum > 0) {
				(*p_top)[linenum] = (*p_top)[linenum-1] + hrect + htext * (1 + (*p_morespace)[linenum-1]); // Fixed by BB 2021-01-30
			//	if(!moved_up) (*p_morespace)[linenum] = (*p_morespace)[linenum-1];  // Fixed by BB 2021-01-31
			//	if(!moved_up) (*p_morespace)[linenum] = 0;  // Fixed by BB 2021-01-31
				}
			morespace = (*p_morespace)[linenum];
			if(j < 16384) {
				if((*p_PivMode)[j] == ABSOLU)
					pivloc = (long) ((*p_PivPos)[j] * GraphicScaleP) / GraphicScaleQ / 10L;
				else
					pivloc = (long) ((*p_Instance)[k].dilationratio * (*p_PivPos)[j] * (*p_Dur)[j]
								* GraphicScaleP) / GraphicScaleQ / 1000L;
				}
			else pivloc = 0.;
			pivloc -= trbeg;
			if((*p_top)[linenum] <= 0) {
				BPPrintMessage(odError,"=> Error Graphic linenum = %ld top <= 0\n",(long)linenum);
				(*p_top)[linenum] = 0;
				}
			if(morespace < 0) {
				BPPrintMessage(odError,"=> Error Graphic morespace < 0\n");
				morespace = 0;
				}
			
			if(trace_graphic) BPPrintMessage(odInfo,"\nRunning DrawObject(%s) for t1 = %ld t2= %ld linenum = %ld, endx = %ld morespace = %ld, top = %ld\n",label,(long)t1,(long)t2,(long)linenum,(long)endx,(long)morespace,(long)(*p_top)[linenum]);
					
			if(DrawObject(j,label,moved_up,(*p_Instance)[k].dilationratio,(*p_top)[linenum],hrect,htext,
					leftoffset,pivloc,t1,t2,trbeg,trend,&morespace,
					&endx,&endy) == ABORT) {
				rep = OK;
				goto ENDGRAPH;
				}
			if(trace_graphic) BPPrintMessage(odInfo,"endx = %ld linenum = %ld\n",(long)endx,(long)linenum);
			(*p_morespace)[linenum] = morespace;
			(*p_endx)[linenum] = endx;
			(*p_endy)[linenum] = endy;
			if(endx > Hmax[w]) Hmax[w] = endx;
			if(endy > Vmax[w]) Vmax[w] = endy;
			if((*p_endx)[linenum] > endxmax) endxmax = (*p_endx)[linenum];
			if((*p_endy)[linenum] > endymax) endymax = (*p_endy)[linenum];
			
			if(trace_graphic) BPPrintMessage(odInfo,"linenum = %ld, linemax = %ld, morespace[%ld] = %ld\n",(long)linenum,(long)linemax,(long)linenum,(long)(*p_morespace)[linenum]);
			}
		if(foundone) {
			linemax += 1;
			(*p_morespace)[linemax] = 0; // Fixed by BB 2021-01-31
			(*p_endx)[linemax] = 0; // Fixed by BB 2021-01-31
			}
		if(trace_graphic) BPPrintMessage(odInfo,"linenum = %ld, linemax = %ld, (*p_top)[linenum] = %ld\n",(long)linenum,(long)linemax,(long)(*p_top)[linenum]);
		if(linenum >= maxlines) {
			BPPrintMessage(odError,"=> Err. (2) linenum = %ld  maxlines = %ld in DrawItem()\n",(long)linenum,(long)maxlines);
			rep = ABORT;
			goto ENDGRAPH;
			}
	//	endx = (*p_endx)[linenum] = 0; // Fixed by BB 2021-01-30
	//	(*p_morespace)[linenum] = 0; // Fixed by BB 2021-01-31
		if(imagePtr != NULL) fflush(imagePtr);
		}

	ENDGRAPH:

	QUIT:
	if(WidthMax < 32000) WidthMax = 2 * endxmax + 40;
	// BPPrintMessage(odInfo,"WidthMax (1) = %ld\n",WidthMax);
	if(WidthMax > 32000) {
		BPPrintMessage(odInfo,"\nImage width %d was too large: it has been cropped to 32000\n",WidthMax);
		WidthMax = endxmax = 32000;
		}

	// BPPrintMessage(odInfo,"endymax = %ld\n",endymax);
	HeightMax = 2 * endymax + 20;
	// BPPrintMessage(odInfo,"HeightMax (1) = %ld\n",HeightMax);
	if(HeightMax > 32000) {
		BPPrintMessage(odInfo,"\nImage height %d was too large: it has been cropped to 32767\n",HeightMax);
		HeightMax = endymax = 32000;
		}

	MyDisposeHandle((Handle*)&p_morespace);
	MyDisposeHandle((Handle*)&p_top);
	MyDisposeHandle((Handle*)&p_endx);
	MyDisposeHandle((Handle*)&p_endy);
	GraphicOn = FALSE;
	return OK;
//	rep = EndImageFile();
//	return(rep);
	}


int DrawObject(int j, char *label, int moved_up, double beta,int top, int hrect, int htext, int leftoffset,
	long pivloc, long t1, long t2, long trbeg, long trend, int *p_morespace, long *p_endx, long *p_endy) {
	// Pattern pat;
	Rect r,r1,r2,r3;
	int tab,rep,x_startpivot,y_startpivot;
	long x;
	double xx,preperiod,objectperiod;

	r.top = top;
	r.left = (int)t1 + leftoffset;
	r.right = (int)t2 + leftoffset;
	r.bottom = r.top + hrect;
	if(trace_graphic) BPPrintMessage(odInfo,"j = %ld, leftoffset = %d,  t1 = %ld, t2 = %ld, endx = %ld\n",(long)j,leftoffset,t1,t2,(*p_endx));

	// Erase background 
	r2 = r;
	resize_rect(&r2,+2,+2);
	erase_rect(&r2);

	// Draw box
	pen_size(4,0);
	stroke_style("black");
	stroke_rect(&r);

	my_sprintf(Message,"j = %ld, r.left = %ld, r.right = %ld, r.top = %ld, r.bottom = %ld\n",(long)j,(long)r.left,(long)r.right,(long)r.top,(long)r.bottom);
	if(trace_graphic) BPPrintMessage(odInfo,Message);
	r2 = r;
	resize_rect(&r2,-1,-1);
	my_sprintf(Message,"j = %ld, r2.left = %ld, r2.right = %ld, r2.top = %ld, r2.bottom = %ld\n",(long)j,(long)r2.left,(long)r2.right,(long)r2.top,(long)r2.bottom);
	if(trace_graphic) BPPrintMessage(odInfo,Message);
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
			my_sprintf(Message,"Pivot x = %ld, y = %ld\n",(long)x_startpivot,(long)y_startpivot);
			if(trace_graphic) BPPrintMessage(odInfo,Message);
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

	tab = (r.right - r.left - 8 * strlen(label)) / 2;
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
		if(moved_up) { // Fixed by BB 2021-01-31
			r.left = r.right + 2; // Write label on the right side
			r.right = r.left + (8 * strlen(label));
			r.bottom -= 1;
		//	r.bottom = r.top + htext;
			*p_endy = r.bottom + 2;
			}
		else {
		//	r.right += 6 * strlen(label); // Write label under the object
			r.right = r.left + (8 * strlen(label));
			r.top = r.bottom + 1;
			r.bottom = r.top + htext + 1;
			*p_endy = r.bottom + 12;
			*p_morespace = 1; // Fixed by BB 2021-01-31
			r.left -= 1;
			}
		erase_rect(&r);
		fill_text(label,r.left + 1,r.bottom - 1);
		if(*p_endx < (r.right + 1)) *p_endx = r.right + 1;
	//	*p_morespace += 1;
		}
	return(OK);
	}


/* int KillDiagrams(int w){
// Currently not used
	int n;
	w = wGraphic;
	for(n=Ndiagram-1; n >= 0; n--) {
		if(n < 0 || n >= MAXDIAGRAM) {
		//	if(Beta) Alert1("=> Err. KillDiagrams. n < 0 || n >= MAXDIAGRAM");
			Ndiagram = 0;
			break;
			}
		if(DiagramWindow[n] == w && p_Diagram[n] != NULL) {
		//	KillPoly(p_Diagram[n]);
			p_Diagram[n] = NULL;
			if(n == Ndiagram-1) Ndiagram--;
			}
		}
	if(!Offscreen) {
		for(n=Npicture-1; n >= 0; n--) {
			if(n < 0 || n >= MAXPICT) {
			//	if(Beta) Alert1("=> Err. KillDiagrams. n < 0 || n >= MAXPICT");
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
	} */


int DrawSequence(int nseq,SoundObjectInstanceParameters **p_object,Milliseconds **p_t1,
	Milliseconds **p_t2,long kmax,unsigned long imax,unsigned long **p_imaxseq,
	int kmode,long **p_ddelta0,long **p_ddelta1,long **p_ddelta2) {
	int j,result;
	Milliseconds t,tmin,tmax,**p_delta;
	unsigned long i,im,k;

	BPPrintMessage(odInfo,"Drawing sequence\n");
	tmin = Infpos; tmax = Infneg;
	im = imax;
	if(im >  (*p_imaxseq)[nseq]) im =  (*p_imaxseq)[nseq];
	if((p_delta=(Milliseconds**) GiveSpace((Size) (im+1) * sizeof(Milliseconds))) == NULL)
		return(ABORT);
	// for(i=1L; i <= im; i++) {
	for(i=ZERO; i <= im; i++) { // Fixed by BB 2021-03-22
		k = (*((*p_Seq)[nseq]))[i];
		(*p_delta)[i] = (*p_ddelta0)[i] + (*p_ddelta1)[i] + (*p_ddelta2)[i];
		if(k < 2) continue;
		if(kmode) {
			if(p_object == NULL) {
				BPPrintMessage(odError,"=> Err. DrawSequence(). p_object == NULL\n");
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


int DrawPrototype(int j,int w,Rect *p_frame) { // THIS IS NOT (YET?) USED because prototypes are drawn on the PHP interface
	Rect r,r1;
	int jj,x,xmin,xmax,oldx,y,rep,htext,topoffset;
	// GrafPtr saveport;
	long i,t,tmin,tmax,pivpos,maxbeggap,maxendgap;
	double p,k,preroll,postroll,preperiod,objectperiod,grscale;
	Str255 label;
	char line[BOLSIZE+10];
	Milliseconds maxcover1,maxcover2,maxgap1,maxgap2,maxtrunc1,maxtrunc2,dur;
	short oldfont,oldsize;
	// Pattern pat;

	if(w < 0 || w >= WMAX) return(MISSED);
	if(!GrafWindow[w]) {
	//	if(Beta) Alert1("=> Err. DrawPrototype(). !GrafWindow[w]");
		return(MISSED);
		}
	if(j < 2 || j >= Jbol) return(OK);
	rep = OK;

	// GetPort(&saveport);
	// SetPortWindowPort(Window[w]);
	// KillDiagrams(w);
	/* if(p_Picture[1] != NULL) {
	//	KillPicture(p_Picture[1]);
		p_Picture[1] = NULL;
		} */
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
		my_sprintf(Message,"Sound-object is empty");
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
	if((*p_PivMode)[j] == RELATIF) pivpos = ((*p_PivPos)[j] * (*p_Dur)[j]) / 100L;
	else pivpos = (*p_PivPos)[j];
	if(preroll < 0.) tmin = 0;
	else tmin = - preroll;
	if(pivpos < tmin) tmin = pivpos;
	maxbeggap = -1L;
	if((*p_ContBeg)[j]) {
		if((*p_ContBegMode)[j] == RELATIF)
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
		if((*p_ContEndMode)[j] == RELATIF)
			maxendgap = ((*p_MaxEndGap)[j] * (*p_Dur)[j]) / 100L;
		else maxendgap = (*p_MaxEndGap)[j];
		}
	if((maxendgap > ZERO) && maxendgap < (*p_Dur)[j] && ((*p_Dur)[j] + maxendgap) > tmax)
		tmax = (*p_Dur)[j] + maxendgap;
	if((tmax - tmin) < ZERO) {
	//	if(Beta) Alert1("=> Err1. DrawPrototype()");
		rep = ABORT; goto QUIT;
		}
	xmin = p_frame->left + 3 * htext;
	xmax = p_frame->right - 3 * htext;
	if((tmax - tmin) > EPSILON) {
		grscale = ((double)(xmax - xmin)) / (tmax - tmin);
		}
	else grscale = 0.;

	/* PenNormal();
	stroke_style(&Black); */

	// Draw time ruler
	y = r.top + htext + 2;
	/* move_to(xmin,y); line_to(xmax,y);
	PenNormal(); */
	p = 0.1;
	if(grscale > 0.001) {
		while((grscale * p) < ((xmax - xmin) / 20.)) p = 10. * p;
		for(i=jj=0; ; i++,jj++) {
			x = xmin + (int)(i * grscale * p);
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
		my_sprintf(line,"0");
	//	c2pstrcpy(label, line);
	//	move_to(xmin - strlen(label)/2,y); fill_text(label);
		k = 20.; while((k * grscale * p) > (xmax - xmin)) k = k / 2.;
		for(i=1; ; i++) {
			my_sprintf(line,"%.2fs",((double)i) * (k * p) / 1000.);
			// c2pstrcpy(label, line);
			x = xmin + k * i * grscale * p - strlen(line)/2;
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
		if((*p_CoverBegMode)[j] == ABSOLU) maxcover1 = (*p_MaxCoverBeg)[j];
		else maxcover1 = (dur * (*p_MaxCoverBeg)[j]) / 100.;
		}
	if(!(*p_CoverEnd)[j]) {
		if((*p_CoverEndMode)[j] == ABSOLU) maxcover2 = (*p_MaxCoverEnd)[j];
		else maxcover2 = (dur * (*p_MaxCoverEnd)[j]) / 100.;
		}
	r.top = p_frame->top + 2 * htext + 5;
	r.bottom = r.top + htext;
	r.left = p_frame->left + 2;
	if(((*p_CoverBeg)[j] && (*p_CoverEnd)[j]) || (maxcover1 == dur && maxcover2 == dur)) {
		my_sprintf(line,"This object may be entirely covered");
		}
	else {
		if(maxcover1+maxcover2 > dur) {
			if(maxcover1 < dur) maxcover2 = dur - maxcover1;
			else maxcover1 = dur - maxcover2;
			}
		r.right = xmin + grscale * (maxcover1 - tmin);
		r.left = xmin + grscale * (dur - tmin - maxcover2);
		r.right = p_frame->right - 2;
		}
	// Draw continuity
	maxgap1 = maxgap2 = INT_MAX;
	if((*p_ContBeg)[j]) {
		if((*p_ContBegMode)[j] == ABSOLU) maxgap1 = (*p_MaxBegGap)[j];
		else maxgap1 = (dur * (*p_MaxBegGap)[j]) / 100.;
		}
	if((*p_ContEnd)[j]) {
		if((*p_ContEndMode)[j] == ABSOLU) maxgap2 = (*p_MaxEndGap)[j];
		else maxgap2 = (dur * (*p_MaxEndGap)[j]) / 100.;
		}
	r.top = p_frame->top + topoffset + 1;
	r.bottom = r.top + htext - 2;
	r.left = p_frame->left + 2;
	r.right = xmin + grscale * (- tmin - maxgap1);
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
	r.left = xmin + grscale * (dur - tmin + maxgap2);
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
		if((*p_TruncBegMode)[j] == ABSOLU) maxtrunc1 = (*p_MaxTruncBeg)[j];
		else maxtrunc1 = (dur * (*p_MaxTruncBeg)[j]) / 100.;
		}

	if(!(*p_TruncEnd)[j]) {
		if((*p_TruncEndMode)[j] == ABSOLU) maxtrunc2 = (*p_MaxTruncEnd)[j];
		else maxtrunc2 = (dur * (*p_MaxTruncEnd)[j]) / 100.;
		}

	y = p_frame->top + topoffset + 4 * htext;
	if(maxtrunc1 > 0 && maxtrunc1 <= dur) {
		x = xmin + grscale * (- tmin);
		x = xmin + grscale * (- tmin + maxtrunc1);
		y = p_frame->top + topoffset + 4 * htext - 1;
		x = xmin + grscale * (- tmin);
		y += htext;
		}
	y = p_frame->top + topoffset + 6 * htext;
	if(maxtrunc2 > 0 && maxtrunc2 <= dur) {
		x = xmin + grscale * (dur - tmin);
		x = xmin + grscale * (dur - tmin - maxtrunc2);
		y = p_frame->top + topoffset + 6 * htext - 1;
		x = xmin + grscale * (dur - tmin) - 1;
		y += htext;
	//	fill_text("TruncEnd");
		}
	else {
		if(maxtrunc1 > 0 && maxtrunc1 <= dur) {
			y = p_frame->top + topoffset + 7 * htext - 1;
			}
		else {
			y = p_frame->top + topoffset + 5 * htext - 1;
			}
		}

	y += htext + htext;
	x = p_frame->right - strlen("BreakTempo") - 3;

	y += htext + htext;
	x = p_frame->left + 4;
	if((*p_FixScale)[j]) {} // fill_text("\pNever rescale");
	else {
		if((*p_OkExpand)[j] && (*p_OkCompress)[j]) {} //  fill_text("\pRescale at will");
		else {
			if(!(*p_OkExpand)[j] && !(*p_OkCompress)[j]) {} //  fill_text("\pRescale within limits");
			else {
				y += htext + 2;
				x = p_frame->left + 4;
				}
			}
		}

	r.top = p_frame->top + topoffset;
	r.left = xmin + grscale * (-tmin);
	r.bottom = r.top + htext;
	r.right = r.left + grscale * dur;
	if(r.right == r.left) r.right += 1;

	// Draw period if any
	GetPeriod(j,1.,&objectperiod,&preperiod);
	if(objectperiod > EPSILON) {
		y = r.top - htext;
		x = r.left + (grscale * preperiod);
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
		x = r.left + grscale * t;
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
		x = r.left + grscale * t;
		if(x > (oldx + 1)) {
		//	move_to(x,r.top+1); line_to(x,r.bottom-2);
			oldx = x;
			}
		}
	// Draw label
	my_sprintf(Message,"%s",*((*p_Bol)[j]));
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
	/*	move_to(r.left + (int) (grscale*pivpos),r.top - 7);
		pen_size(2,2); 
		stroke_style(&Color[PivotC]);
		if(j > 16383 || (*p_OkRelocate)[j]) Move(0,5);
		else Line(0,5); */	
		// Draw arrow of pivot
	/*	Line(-2,-2); Line(4,0); Line(-2,2);
		PenNormal(); */
		
		// The following is a line connecting the pivot to the object if needed
	/*	move_to(r.left + (int) (grscale*pivpos),r.top);
		line_to(r.left,r.top); */
		}
	else {
	/*	move_to(r.left,r.top - 1);
		fill_text("\pSmooth object: no pivot"); */
		}
	// Draw vertical red line
	if(xmax > xmin) {
		if((*p_Tpict)[iProto] == Infneg) {
			if(Hpos == -1) Hpos = xmin + grscale * (- tmin);
			(*p_Tpict)[iProto] = tmin + ((Hpos - xmin) * (tmax - tmin))/ (xmax - xmin);
			}
		else Hpos = (((*p_Tpict)[iProto] - tmin) * grscale) + xmin;
		if((*p_Tpict)[iProto] != ZERO) {
		/*	move_to(Hpos,p_frame->top+1);
			stroke_style(&Red);
			line_to(Hpos,p_frame->bottom - htext - htext);
			stroke_style(&Black); */
			my_sprintf(Message,"%ld ms",(long)(*p_Tpict)[iProto]);
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
		my_sprintf(Message,"Force to Csound instrument %ld",(long)(*p_CsoundInstr)[iProto]);
		else if((*p_CsoundInstr)[iProto] == 0) my_sprintf(Message,"Force to current Csound instrument");
			else my_sprintf(Message,"Never change Csound instruments");
	//	c2pstrcpy(label, Message);
		x = p_frame->right - 4 - strlen(line);
	/*	move_to(x,p_frame->bottom - (4 * htext));
		fill_text(label); */
		}
	else {
		if((*p_CsoundAssignedInstr)[iProto] >= 1) {
			my_sprintf(Message,"Send MIDI messages to Csound instrument %ld",
				(long)(*p_CsoundAssignedInstr)[iProto]);
		//	c2pstrcpy(label, Message);
			x = p_frame->right - 4 - strlen(line);
		/*	move_to(x,p_frame->bottom - (4 * htext));
			fill_text(label); */
			}
		}
	// MIDI channel status
	if((*p_DefaultChannel)[iProto] > 0)
	my_sprintf(Message,"Force to MIDI channel %ld",(long)(*p_DefaultChannel)[iProto]);
	else if((*p_DefaultChannel)[iProto] == 0) my_sprintf(Message,"Force to current MIDI channel");
		else my_sprintf(Message,"Never change MIDI channels");
	// c2pstrcpy(label, Message);
	x = p_frame->right - 4 - strlen(line);
	/* move_to(x,p_frame->bottom - (3 * htext));
	fill_text(label); */

	// Object type
	if(!((*p_Type)[iProto] & 1) && (*p_MIDIsize)[iProto] > ZERO) {
	//	stroke_style(&Red);
		my_sprintf(Message,"MIDI sequence is INACTIVE");
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
	else if(Beta) Alert1("=> Err DrawPrototype(). saveport == NULL"); */
	return(rep);
	}


int DrawItemBackground(Rect *p_r,unsigned long imax,int htext,int hrect,int leftoffset,
	int interruptok,Milliseconds **p_delta,long *p_yruler,int topoffset,int *p_overflow,char* type) {
	int result;
	double x,xscale,p,rr,period,shift;
	long i,j,k,t1,t2,y,tmem1,tmem2,tmem3,xmax,ymax,y_curr;
	char line[BOLSIZE+5],showsmalldivisions,line_image[200];
	Str255 label;

    if(!ShowGraphic) return OK;
	shift = 0.;
	if(strcmp(type,"pianoroll") == 0) shift = 0.; // Later we'll take care of this  2024-05-10
	else {
		if(Improvize || PlayAllChunks) shift = (double) PianorollShift;
		}
	// if(imagePtr == NULL) {
	//	N_image++;
		if(strcmp(type,"pianoroll") != 0 && (Improvize || PlayAllChunks) && (ShowPianoRoll))
	//	if(strcmp(type,"pianoroll") != 0 && (Improvize || PlayAllChunks) && (ShowPianoRoll || !rtMIDI))
		// ShowPianoRoll, because if no pianoroll has been drawn, the value of shift is incorrect.
			CreateImageFile(shift/1000.);
		else CreateImageFile(-1.);  // Later we can use it
        if(Panic) return ABORT;
	//	}
	result = OK;
	pen_size(1,0);
	text_style(htext,"arial");
	ymax = p_r->bottom;
	if(ymax > 16300) ymax = p_r->bottom = 16300;
	if(HeightMax < 32000) HeightMax = 2 * ymax + 20;
	if(HeightMax > 32000) {
		BPPrintMessage(odInfo,"\nImage height %d was too large: it has been cropped to 32000\n",HeightMax);
		HeightMax = ymax = 32000;
		ymax = (HeightMax - 20) / 2;
		p_r->bottom = ymax;
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
	if(xmax < 40 || xmax > max_coordinate) xmax = max_coordinate;
	if(leftoffset < 0) {
		xmax -= (hrect * leftoffset);
		}

	if(trace_graphic) 
		BPPrintMessage(odInfo,"DrawItemBackground() imax = %ld, xmax = %ld\n",(long)imax,(long)xmax);

	if(Nature_of_time == SMOOTH) while(imax > 1L && (*p_T)[imax] == ZERO) imax--;
	t2 = ((double)(*p_T)[imax] * GraphicScaleP) / GraphicScaleQ / 10. + leftoffset;

	y = (*p_yruler) = p_r->top + htext + 6;
	pen_size(1,0);
	draw_line(leftoffset,y,xmax,y,"");
	if(trace_graphic)
		BPPrintMessage(odInfo,"leftoffset = %d, y = %d, xmax = %d\n",leftoffset,y,xmax);

	p = 50. / x;
	for(i = j = 0; ; i++,j++) {
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
	if(shift == 0.) my_sprintf(line,"0");
	else my_sprintf(line,"%.2fs",shift / 1000.);
	fill_text(line,leftoffset - 6,y);
	k = 10; if((k * p) > 400) k = 1;
	tmem2 = - Infpos;
	for(i = 1; ; i++) {
		if(k == 10) my_sprintf(line,"%.2fs",(double)i * xscale * k / 10. + shift / 1000.);
		if(k == 1) my_sprintf(line,"%.3fs",(double)i * xscale * k / 10. + shift / 1000.);
		t1 = leftoffset + Round(k * i * p) - 10;
		t2 = t1 + 12;
		if(t2 >= xmax) break;
		if(t1 > tmem2) {
			fill_text(line,t1,y);
			tmem2 = t2 + 7 * strlen(line);
			}
		}

	// Draw time streaks
	if(PlayPrototypeOn) goto ENDSTREAKS;
	y = p_r->top + topoffset - hrect;
	xmax -= 40;

	period = Kpress * Pclock * 1000. / Qclock / Ratio;
	period = (period * GraphicScaleP) / GraphicScaleQ / 10;
	if(period < 5) showsmalldivisions = FALSE;
	else showsmalldivisions = TRUE;

	tmem1 = tmem2 = tmem3 = - Infpos;
	p = ((double) GraphicScaleP) / (GraphicScaleQ * 10.);
	stroke_style("blue");
	fill_style("blue");

	// Subdivisional time streaks
	// showsmalldivisions = TRUE;
	for(i=1L,rr=Ratio,k=0; i < imax; i++, rr += Kpress) {
		t1 = (*p_T)[i] / CorrectionFactor;
		// BPPrintMessage(odInfo, " %ld",(long) t1);
		if(p_delta != NULL) t1 += (*p_delta)[i];
		t1 = leftoffset + Round(t1 * p);
		if((t1 > tmem1 && showsmalldivisions && t1 < xmax) || tmem1 == -Infpos) {
			pen_size(1,0);
			draw_line(t1,y,t1,ymax,"");
			tmem1 = t1 + 20;
			tmem1 = t1 + 4; // 2022-02-17
			}
		}
	// BPPrintMessage(odInfo, "\n\n");

	// Draw horizontal white lines to break time subdivisional time streaks
	pen_size(3,0);
	stroke_style("white");
	for(y_curr = y; y_curr < ymax; y_curr += 6) {
		draw_line(leftoffset,y_curr,xmax,y_curr,"");
		}

	// Major time streaks
	pen_size(1,0);
	stroke_style("blue");
	text_style(10,"arial");
	for(i=1L,rr=Ratio,k=0; i < imax; i++, rr += Kpress) {
		t1 = (*p_T)[i] / CorrectionFactor;
		if(p_delta != NULL) t1 += (*p_delta)[i];
		t1 = leftoffset + Round(t1 * p);
		if(rr >= Ratio) {
			rr -= Ratio;
			if(t1 > tmem2  && t1 < xmax) {
				my_sprintf(line,"%ld",(long)(k + StartFromOne));
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


int DrawPianoNote(char* type,int key,int chan, Milliseconds timeon, Milliseconds timeoff,
	int leftoffset,int topoffset,int hrect,int minkey,int maxkey,Rect *p_r) {
	int x1, x2, y;
	Rect r;
	char* word;
	int length;

	if(key < 0 || key > 127) {
		BPPrintMessage(odError,"=> Err. DrawPianoNote() key = %d\n",key);
		return(OK);
		}
	if(chan < 0 || chan >= MAXCHAN) {
		BPPrintMessage(odError,"=> Err. DrawPianoNote() chan = %d\n",chan);
		return(OK);
		}
	// if(stop(0,"DrawPianoNote") != OK) return(ABORT);
	if(rtMIDI || WriteMIDIfile || OutCsound || OutBPdata) {
		timeon -= PianorollShift;
		timeoff -= PianorollShift;	
		}
	else if(DisplayItems) {
		timeon -= 600L;
		timeoff -= 600L;
		}
	timeon = Round(((double)timeon * GraphicScaleP) / GraphicScaleQ / 10.);
	timeoff = Round(((double)timeoff * GraphicScaleP) / GraphicScaleQ / 10.);
	x1 = p_r->left + leftoffset + timeon;
	y = (maxkey - key - 1) * hrect + topoffset;
	x2 = x1 + timeoff - timeon;
	if((x1 - Lastx2[key]) < 4) {
		x1 += 2; // Separate identical notes when replayed
		}
	length = x2 - x1;
	if(length < 8) {
		x2 += 2; // For out-time objects
		if(strcmp(type,"midi") == 0) x1 -= 1;
		}
	Lastx2[key] = x2;
	pen_size(8,0);
	if(strcmp(type,"midi") == 0) stroke_style("brown");
	else if(strcmp(type,"csound") == 0) {
		stroke_style("green");
	//	return(OK);
		}
	else stroke_style("black");
	draw_line(x1,y,x2,y,"");
	if(trace_draw_piano_note) BPPrintMessage(odInfo,"Note #%d x1 = %d, x2 = %d,  %s\n",key,(x1 - 42) * 5,(x2 - 42) * 5,type);
	imageHits++;
	return(OK);
	}


int DrawNoteScale(Rect* p_r,int w,int minkey,int maxkey,int hrect,int leftoffset,int topoffset) {
	int y,key,xmin,xmax;
	char line[20];
	pen_size(2,0);
	xmin = p_r->left + 41;
	xmax = p_r->right - 28;
	if(leftoffset < 0) xmax -= (hrect * (leftoffset - 1));
	if(NoteConvention > 3) {
		stroke_style("rgb(186,186,0)");
		fill_style("rgb(186,186,0)");
		for(key=6; key < 128; key+=2) {
			if(key < minkey || key > maxkey) continue;
			y = (maxkey - key) * hrect + topoffset;
			PrintNote(-1,key,0,-1,line);
			draw_line(xmin,y,xmax,y,"");
			}
		}
	else {
		stroke_style("rgb(0,117,117)");
		fill_style("black");
		text_style(11,"arial");
		for(key=0; key < 128; key+=12) {
			if(key < minkey || key > maxkey) continue;
			y = (maxkey - key) * hrect + topoffset;
			PrintNote(-1,key,0,-1,line);
			fill_text(line,5,y + 3);
			fill_text(line,xmax+ 5,y + 3);
			draw_line(xmin,y,xmax,y,"");
			}
		stroke_style("rgb(186,186,0)");
		fill_style("rgb(186,186,0)");
		for(key=6; key < 128; key+=12) {
			if(key < minkey || key > maxkey) continue;
			y = (maxkey - key) * hrect + topoffset;
			PrintNote(-1,key,0,-1,line);
			fill_text(line,5,y + 3);
			fill_text(line,xmax + 5,y + 3);
			draw_line(xmin,y,xmax,y,"");
			}
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
		my_sprintf(line,"ctx.beginPath();\n");
		fputs(line,imagePtr);
		imageHits++;
		}
	}

void end_path() {
	char line[100];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		my_sprintf(line,"ctx.stroke();\n");
		fputs(line,imagePtr);
		imageHits++;
		}
	}
	
void move_to(int x,int y) {
	char line[100];
	if(x >= 0 && x <= max_coordinate && y >= 0 && y <= max_coordinate) {
		if(strcmp(graphic_scheme,"canvas") == 0) {
			my_sprintf(line,"ctx.moveTo(%ld,%ld);\n",(long)resize * x,(long)resize * y);
			fputs(line,imagePtr);
			imageHits++;
			}
		}
	}

void line_to(int x,int y) {
	char line[100];
	if(x < 0 || x > max_coordinate) x = max_coordinate;
	if(y < 0 || y > max_coordinate) y = max_coordinate;
	if(x >= 0 && x <= max_coordinate && y >= 0 && y <= max_coordinate) {
		if(strcmp(graphic_scheme,"canvas") == 0) {
			my_sprintf(line,"ctx.lineTo(%ld,%ld);\n",(long)resize * x,(long)resize * y);
			fputs(line,imagePtr);
			imageHits++;
			}
		}
	}

void draw_line(int x1,int y1,int x2,int y2,char* style) { 
	char line[200];
	if((x2 < 0 || x2 > max_coordinate) && y1 == y2) x2 = max_coordinate;
	if((y2 < 0 || y2 > max_coordinate) && x1 == x2) y2 = max_coordinate;
	if(x1 >= 0 && x1 <= max_coordinate && y1 >= 0 && y1 <= max_coordinate && x2 >= 0 && x2 <= max_coordinate && y2 >= 0 && y2 <= max_coordinate) {
		if(strcmp(graphic_scheme,"canvas") == 0) {
			if(strcmp(style,"round") == 0)
				my_sprintf(line,"ctx.beginPath();\nctx.lineCap = \"round\";\nctx.moveTo(%ld,%ld);\nctx.lineTo(%ld,%ld);\nctx.stroke();\n",(long)resize * x1,(long)resize * y1,(long)resize * x2,(long)resize * y2);
			else
				my_sprintf(line,"ctx.beginPath();\nctx.moveTo(%ld,%ld);\nctx.lineTo(%ld,%ld);\nctx.stroke();\n",(long)resize * x1,(long)resize * y1,(long)resize * x2,(long)resize * y2);
			fputs(line,imagePtr);
			imageHits++;
			}
		}
	}
	
void pen_size(int x,int y) {
	char line[100];
	float ratio;
	if(strcmp(graphic_scheme,"canvas") == 0) {
		ratio = (float) x + (0.1 * (float) y);
		my_sprintf(line,"ctx.lineWidth = %ld;\n",(long)ratio);
		fputs(line,imagePtr);
		}
	}

void stroke_style(char* color) {
	char line[100];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		my_sprintf(line,"ctx.strokeStyle = '%s';\n",color);
		fputs(line,imagePtr);
		imageHits++;
		}
	}
	
void fill_style(char* color) {
	char line[100];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		my_sprintf(line,"ctx.fillStyle = '%s';\n",color);
		fputs(line,imagePtr);
		imageHits++;
		}
	}

void stroke_text(char* txt,int x,int y) {
	char line[500];
	if(x >= 0 && x <= max_coordinate && y >= 0 && y <= max_coordinate) {
		if(strcmp(graphic_scheme,"canvas") == 0) {
			my_sprintf(line,"ctx.strokeText('%s',%ld,%ld);\n",txt,(long)resize * x,(long)resize * y);
			fputs(line,imagePtr);
			imageHits++;
			}
		}
	}

void fill_text(char* txt,int x,int y) {
	char line[500];
	if(x >= 0 && x <= max_coordinate && y >= 0 && y <= max_coordinate) {
		if(trace_graphic) BPPrintMessage(odInfo,"text = %s\n",txt);
		if(strcmp(graphic_scheme,"canvas") == 0) {
			my_sprintf(line,"ctx.fillText(\"%s\",%ld,%ld);\n",txt,(long)resize * x,(long)resize * y);
			fputs(line,imagePtr);
			imageHits++;
			}
		}
	}
	
void stroke_rect(Rect* p_r) {
	char line[100];
	int x1,x2,y1,y2,w,h;
	if(p_r->left <= max_coordinate && p_r->right <= max_coordinate && p_r->top <= max_coordinate && p_r->bottom <= max_coordinate) {
		if(strcmp(graphic_scheme,"canvas") == 0) {
			x1 = resize * p_r->left;
			x2 = resize * p_r->right;
			y1 = resize * p_r->top;
			y2 = resize * p_r->bottom;
			w = x2 - x1;
			h = y2 - y1;
			if(x1 >= 0 && y1 >= 0) {
				my_sprintf(line,"ctx.strokeRect(%ld,%ld,%ld,%ld);\n",(long)x1,(long)y1,(long)w,(long)h);
				fputs(line,imagePtr);
				imageHits++;
				}
			}
		}
	}
		
void fill_rect(Rect* p_r,char* color) {
	char line[100];
	int x1,x2,y1,y2,w,h;
	if(p_r->left <= max_coordinate && p_r->right <= max_coordinate && p_r->top <= max_coordinate && p_r->bottom <= max_coordinate) {
		if(strcmp(graphic_scheme,"canvas") == 0) {
			x1 = resize * p_r->left;
			x2 = resize * p_r->right;
			y1 = resize * p_r->top;
			y2 = resize * p_r->bottom;
			w = x2 - x1;
			h = y2 - y1;
			if(x1 >= 0 && y1 >= 0) {
				my_sprintf(line,"ctx.fillStyle = '%s';\n",color);
				fputs(line,imagePtr);
				my_sprintf(line,"ctx.fillRect(%ld,%ld,%ld,%ld);\n",(long)x1,(long)y1,(long)w,(long)h);
				fputs(line,imagePtr);
				my_sprintf(line,"ctx.fillStyle = 'black';\n");
				fputs(line,imagePtr);
				imageHits++;
				}
			}
		}
	}

void text_style(int size,char* font) {
	char line[100];
	if(strcmp(graphic_scheme,"canvas") == 0) {
		my_sprintf(line,"ctx.font = \"%ldpx %s\";\n",(long)(resize *size),font);
		fputs(line,imagePtr);
		}
	}

void erase_rect(Rect* p_r) {
	char line[200];
	int x1,x2,y1,y2;
	if(p_r->left <= max_coordinate && p_r->right <= max_coordinate && p_r->top <= max_coordinate && p_r->bottom <= max_coordinate) {
		if(strcmp(graphic_scheme,"canvas") == 0) {
			x1 = resize * p_r->left;
			x2 = resize * p_r->right;
			y1 = resize * p_r->top;
			y2 = resize * p_r->bottom;
			if(x1 >= 0 && x2 >= 0 && y1 >= 0 && y2 >= 0) {
				my_sprintf(line,"ctx.fillStyle = 'white';\nctx.fillRect(%ld,%ld,%ld,%ld);\nctx.fillStyle = 'black';\n",(long)x1,(long)y1,(long)(x2 - x1),(long)(y2 - y1));
				fputs(line,imagePtr);
				}
			}
		}
	}

void resize_rect(Rect* p_r,int w,int h) {
	if((p_r->right - p_r->left) > (-2 * w)) {
		p_r->left = p_r->left - w;
		p_r->right = p_r->right + w;
		}
	if((p_r->bottom - p_r->top) > (-2 * h)) {
		p_r->top = p_r->top - h;
		p_r->bottom = p_r->bottom + h;
		}
	}