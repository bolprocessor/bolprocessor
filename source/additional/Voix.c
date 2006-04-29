/* Voix.c */ 

#ifndef _H_BP2
#include "-BP2.h"
#endif
#include "-BPdecl.h"

int Threshold(char[],int,int);
long ReadTable(int,int***,long,int);
int DisplayTable(int,int**,int**,int,long,int,int,int,int);
long MakeEnvelope(int**,int**,int,long,int**,int**);
int MakeSonagram(int**,long,long****,int**,int*,int*,int*);
int GetWalsh(int**,int,int,int,int,long****,int,
										int**,int*);
int DrawSonagram(int,long****,int,int,int);
int Wal01(int,int,int,int*);
int Walsh(int,int,int,int*);
int Rad01(int,int,int);

#define LOGIM 5
	/* Log base 2 of number of samples in analysis window */
#define MINVALUE 10.
	/* Threshold value of signal for starting a sound file */
#define MINAMPLITUDE 5.
	/* Minimum amplitude of envelope in sound segment */
#define MINSEGMENT 0.05 /* Minimum duration of sound segment in sec. */
#define LOGNCROSSING 3
	/* Log base 2 of number of zero-crossings in analysis window */
#define NUMBERCOEFF 50
	/* Max number of Walsh coefficients (should be even) */

#define SOUNDSHIFT -210
#define SONAGRAMSHIFT -40
#define RECTHEIGHT 6	/* Height of rectangles in sonagram */
#define VSCALE 0.2
#define HSCALE 1.

#define FMIN 50		/* Min. frequency for envelope */
#define DMAX 0.04	/* Max. mean-square deviation for envelope */

#include "Voix.proto.h"

int NumberCoeff;

/* -----------------  READ SoundEdit FILE --------------------- */


mOpenSound(int wind)
{
int vref,refnum,rep;
int i,iorg,n,nmax,**p_sound,**p_xe,**p_ye,**p_yy,fith,fitv,sizeyy;
long size,jm,****p_a,**ptr;
double kk;
Str255 fn;

/* Test(); return(OK); */
rep = OK;
if(wind < 0 || wind >= WMAX) {
	ActivateWindow(QUICK,wGraphic);
	wind = wGraphic;
	}
ClearWindow(1,wind);
if((p_sound = (int**) GiveSpace((Size)SNDSIZE*sizeof(int))) == NULL)
	return(FAILED);
p_xe = p_ye = p_yy = NULL;
p_a = NULL;
fith = fitv = 0;
ShowMessage(wMessage,"Find SoundEdit file…");
ComputeOn = 1;
if(OldFile(10,fn,&vref)) {
	if(FSOpen(fn,vref,&refnum) == noErr) {
		do {
			size = ReadTable(refnum,&p_sound,(long)SNDSIZE,1);
			if((int) size == ABORT) {
				rep = FAILED; goto QUIT;
				}
			MyEmptyHandle(&p_xe);
			MyEmptyHandle(&p_ye);
			MyEmptyHandle(&p_yy);
			MyEmptyHandle(&p_a);
			if((p_xe = (int**) GiveSpace((Size)size*sizeof(int))) == NULL) {
				rep = FAILED; goto QUIT;
				}
			if((p_ye = (int**) GiveSpace((Size)size*sizeof(int))) == NULL) {
				rep = FAILED; goto QUIT;
				}
			if((p_yy = (int**) GiveSpace((Size)size*sizeof(int))) == NULL) {
				rep = FAILED; goto QUIT;
				}
			if((p_a = (long****) GiveSpace((Size)((size
				/ Power2(LOGIM)) + 1) * sizeof(long**),0)) == NULL) {
				rep = FAILED; goto QUIT;
				}
			iorg = 0;
	/*		size = 257L;
			kk = 6.283 / 40.;
			for(i=0; i < size; i++) {
				(*p_sound) [i] = (int) (100. * cos((double) kk * i));
				} */
			size = 800L;
			KillDiagrams(wGraphic);
			sprintf(Message,"Duration = %.4f sec",(float) size / FSWORK);
			ShowMessage(wMessage,Message);
			if((rep=MakeSonagram(p_sound,size,p_a,p_yy,&sizeyy,&nmax,&iorg))
					== ABORT) goto QUIT;
			}
		while(rep != OK);
		jm = MakeEnvelope(NULL,p_sound,iorg,size,p_xe,p_ye);
		DrawSonagram(wGraphic,p_a,Power2(LOGIM),size,nmax);
		for(n=0; n < nmax; n++) {
			ptr = (*p_a)[n];
			MyEmptyHandle(&ptr);
			}
		MyEmptyHandle(&p_a);
		DisplayTable(wGraphic,NULL,p_sound,iorg,size,0,fith,fitv,1);
		DisplayTable(wGraphic,p_xe,p_ye,iorg,size,jm,fith,fitv,1);
		DisplayTable(wGraphic,p_xe,p_ye,iorg,size,jm,fith,fitv,-1);
		DisplayTable(wGraphic,NULL,p_yy,iorg,(long)sizeyy,
						0,fith,fitv,1);
		MyEmptyHandle(&p_yy);
		MyEmptyHandle(&p_xe);
		MyEmptyHandle(&p_ye);
		goto QUIT;
		}
	else {
		Alert1("Error opening SoundEdit file…");
		}
	}
MyEmptyHandle(&p_sound);
ComputeOn = 0;
return(FAILED);
	
QUIT:
if(FSClose(refnum) != noErr) {
	Alert1("Error closing SoundEdit file…");
	MyEmptyHandle(&p_sound);
	rep = FAILED;
	}
FlushVol(0L,refnum);
MyEmptyHandle(&p_sound);
Nw = -1;
ActivateWindow(SLOW,wGraphic);
ComputeOn = 0;
return(rep);
}



long ReadTable(int refnum, int ***pp_table, long size, int trig)
{
register i,j,n;
char buffer[BUFSIZE];
long count,date=ZERO;
int io,newj,start,ratio,is,sum,**handle;

PleaseWait();
newj = size;
/* newsize = GetHandleSize(*pp_table); */
ratio = (int) FSORG / FSWORK;
if(ratio < 1) Alert1("‘FSWORK’ too high.");
sprintf(Message,"Original sampling %dHz, reduced to %dHz",FSORG,FSWORK);
ShowMessage(wMessage,Message);
j = 0;
start = !trig; /* trig=1 if table should start on threshold value */
do {
	count = (long) (BUFSIZE - (BUFSIZE % ratio));
	io = FSRead(refnum,&count,&buffer);
	if(start) {
		for(i=0,is=1,sum=0; i < count; i++) {
			if((n = buffer[i]) < 0) n = 256 + n;
			sum = sum + n;
			if(is++ >= ratio) {
				(**pp_table)[j++] = sum / ratio - 128;
				if(j > 2000) { /* $$$ */
					ShowMessage(wMessage,"Array is larger than 2000.  Truncating.");
					return(j);
					}
				if(j > 31988) {
					Alert1("Array is larger than 32000.  Truncating.");
					return(j);
					}
				if(j >= newj) {
					sprintf(Message,"%.4f sec. loaded…",
						(float) date / 1000L);
					ShowMessage(wMessage,Message);
					if(((*pp_table)
						= (int**) IncreaseSpace((Handle)(*pp_table)))
							== NULL) return(ABORT);
					newj = newj + (newj / 2);
					}
				sum = 0; is = 1;
				}
			}
		}
	else {
		if(Threshold(buffer,(int)count,40) == OK) {
			start = 1;
			sprintf(Message,"Loading from date %.4f sec.",
					(float) date / 1000L);
			ShowMessage(wMessage,Message);
			}
		}
	date = date + ((1000L * count) / FSORG);
	}
while (io == noErr);
END:
return(j);
}


Threshold(char x[], int size, int min)
{
register i,n;
for(i=0; i < size; i++) {
	n = x[i];
	if(n >= 0) n -= 128;
	else n += 128;
	if(abs(n) > min) return(OK);
	}
return(FAILED);
}


DisplayTable(int w, int **p_x, int **p_y, int iorg, long im, int jm, int fith, int fitv, int dir)
{
int i,j,n,x,x0,x1,xmem,y,ymem,y0,ymin,ymax,vmin,vmax,rep,stop;
int newh,newv;
Rect r;
float hscale,vscale;

if(im == 0) return(OK);
if((p_x != NULL) && (jm == 0)) return(OK);
sprintf(Message,"Starting at date %.4f sec.",(float) iorg / FSWORK);
ShowMessage(wMessage,Message);
PleaseWait();
SetPort(Window[w]);
r.top = Window[w]->portRect.top;
r.left = Window[w]->portRect.left;
r.bottom = Window[w]->portRect.bottom - SBARWIDTH - 1;
r.right = Window[w]->portRect.right - SBARWIDTH - 1;
hscale = HSCALE;
vscale = VSCALE;
y0 = ((r.bottom + r.top) / 2) + Vzero[w];
/* If not rescaled, zero is in the middle */
if(fith) {
	if(p_x == NULL) hscale = (float) (r.right - r.left) / im;
	else hscale = (float) (r.right - r.left)
					/ (1 + (*p_x)[jm-1] - (*p_x)[0]);
	sprintf(Message,"Vscale = %.4f  Hscale = %.4f",vscale,hscale);
	ShowMessage(wMessage,Message);
	}
if(fitv) {
	for(i=iorg,ymin=32767,ymax=-32768,j=0; i < im; i++,j++) {
		if(p_x == NULL) {
			if(!fith && (i > (r.right - r.left))) break;
			y = dir * (*p_y)[i];
			}
		else {
			if(j >= jm) break;
			if(!fith && ((*p_x)[j] > (r.right - r.left))) break;
			y = dir * (*p_y)[j];
			}
		if(y > ymax) ymax = y;
		if(y < ymin) ymin = y;
		}
	if((ymax - ymin) >= 1) {
		vscale = (float) (r.bottom - r.top) / (ymax - ymin);
		}
	sprintf(Message,"Vscale = %.4f  Hscale = %.4f",vscale,hscale);
	ShowMessage(wMessage,Message);
	}
for(n=0; n < Ndiagram; n++) {
	if(p_Diagram[n] == NULL) break;
	}
if(n >= Ndiagram) {
	if(Ndiagram < MAXDIAGRAM) {
		Ndiagram++;
		}
	else {
		if(Answer("Too many diagrams.  Erase one of them",'N') != 'Y')
			return(ABORT);
		SetPort(Window[w]);
		n = 0;
		KillPoly(p_Diagram[n]);
		p_Diagram[n] = NULL;
		}
	}
p_Diagram[n] = OpenPoly();
DiagramWindow[n] = w;
PenNormal();
vscale = vscale * dir;
if(p_x != NULL) {
/*	PenPat(gray); */
	x = r.left +  Hzero[w] + (*p_x)[0] * hscale - iorg;
	y = vmin = vmax = (int) (y0 - (*p_y)[0] * vscale);
	}
else {
	x = r.left +  Hzero[w];
	y = vmin = vmax = (int) (y0 - (*p_y)[iorg] * vscale);
	}
MoveTo(x,y);
xmem = 0; ymem = y; stop = 0;
for(i=(iorg+1),j=0;i < im;i++,j++) {
	if(p_x == NULL) {
		y = (int)(y0 - (*p_y)[i] * vscale);
		x = r.left +  Hzero[w] + (i - iorg) * hscale;
		}
	else {
		if(j >= jm) break;
		y = (int)(y0 - (*p_y)[j] * vscale);
		x = r.left +  Hzero[w] + (*p_x)[j] * hscale - iorg;
		}
	if(y > vmax) vmax = y;
	if(y < vmin) vmin = y;
	if(x >= 30000) {		/* 3000 $$$ */
		ShowMessage(wMessage,"Can't display entirely…");
		break;
		}
	if((x - xmem) == 0) {
		stop = 1;
		ymem = y;
		continue;
		}
	else {
		if(stop) {
			LineTo(xmem,ymem);
			stop = 0;
			}
		}
	xmem = x; ymem = y;
	if(Button()) {
		Nw = -1;
		FlushEvents(everyEvent,0);
		while((rep = MainEvent()) != RESUME && rep != STOP);
		if(rep == STOP) {
			KillPoly(p_Diagram[Ndiagram-1]);
			Ndiagram--;
			return(rep);
			}
		Nw = -1;
		ActivateWindow(SLOW,w);
		}
	LineTo(x,y);
	}
ClosePoly();
/* DrawControls(Window[w]); */
/*
if(Hmin[w] != 0 && Hmin[w] != MAXINT) OffsetPoly(p_Diagram[n],-Hmin[w],0);
if(Vmin[w] != 0 && Vmin[w] != MAXINT) OffsetPoly(p_Diagram[n],0,-Vmin[w]);
*/
newh = newv = 0;
if(0 < Hmin[w]) {
	Hmin[w] = 0;
	newh = 1;
	}
if(x > Hmax[w]) Hmax[w] = x;
if(vmin < Vmin[w]) {
	Vmin[w] = vmin;
	newv = 1;
	}
if(vmax > Vmax[w]) Vmax[w] = vmax;
if(OKvScroll[w]) SetCtlValue(vScroll[w],0);
if(OKhScroll[w]) SetCtlValue(hScroll[w],0);
SlideH[w] = SlideV[w] = 0;
SetMaxControlValues(w,r);
if(newh) OffsetGraphs(w,Hmin[w] - 1,0);
if(newv) OffsetGraphs(w,0,Vmin[w] - 1);
ClipRect(&r);
SetCursor(&arrow);
return(OK);
}


long MakeEnvelope(int **p_x, int **p_y, int iorg, long im, int **p_xe, int **p_ye)
/* Input is (x,y), output is (xe,ye) */
/* If p_x is NULL then 'x' is stepping by one unit */
{
register i,j,k,n;
int x,y,x0,y0,xn,yn,ymin,ymax,Sx2,Sxy,Sy2,dx,dy;
float decay,peak,ryx,d;
Rect r,dot;

PleaseWait();
if(p_x != NULL) (*p_xe)[0] = (*p_x)[iorg]; else (*p_xe)[0] = iorg;
(*p_ye)[0] = abs((*p_y)[iorg]);
decay = 1. - ((float) FMIN / FSWORK);
peak = 0.;
if(i >= im) return(0L);
for(i=iorg,j=0; i < im; i++) {
	peak = peak * decay;
	if((y0 = abs((*p_y)[i])) <= peak) continue;
	peak = y0;
	if(p_x != NULL) x0 = (*p_x)[i]; else x0 = i;
	Sx2 = Sxy = Sy2 = 0;
	for(k=1,n=0; (i+k) < im; k++) {
		peak = peak * decay;
		yn = abs((*p_y)[i+k]);
		if(yn <= peak) continue;
		n++;
		peak = (float) yn;
		if(p_x != NULL) xn = (*p_x)[i+k]; else xn = i + k;
		dx = xn - x0; dy = yn - y0;
		Sx2 = Sx2 + dx * dx;
		Sxy = Sxy + dx * dy;
		Sy2 = Sy2 + dy * dy;
		if(n == 1) continue; /* Not necessary, but speeds up… */
		ryx = (float) dy / dx;
		d = (ryx * ryx * Sx2 - 2. * ryx * Sxy + Sy2) / n;
		if(d > DMAX) {
			(*p_xe)[++j] = xn; (*p_ye)[j] = yn;
			i = i + k;
			break;
			}
		}
	}
if(p_x != NULL) (*p_xe)[++j] = (*p_x)[im-1]; else (*p_xe)[++j] = im - 1;
(*p_ye)[j] = abs((*p_y)[im-1]);
return((long)(j+1));
}



MakeSonagram(int **p_y, long size, long ****p_a, int **p_yy, int *p_sizeyy, int *p_nmax, int *p_iorg)
{
register i;
int iorg,im,y0,yi,n,rep,
	sign,soundthere,zerocrossing,maxcrossing;
float peak,decay,date;
long **ptr;

PleaseWait();
/* maxcrossing = (int) pow((double) 2.,(double) LOGNCROSSING); */
(*p_sizeyy) = 0;
im = Power2(LOGIM);
NumberCoeff = NUMBERCOEFF;
if(NumberCoeff > im) NumberCoeff = im;

decay = 1. - ((float) FMIN / FSWORK);
peak = 0.;
sign = 1;
soundthere = zerocrossing = (*p_nmax) = 0;
for(i=0,iorg=0,n=0,(*p_iorg)=0; i < size; i++) {
	if(Button()) {
		while((rep = MainEvent()) != RESUME && rep != STOP);
		if(rep == STOP) return(ABORT);
		}
	peak = peak * decay;
	yi = (*p_y)[i];
	y0 = abs(yi);
	if((peak < MINAMPLITUDE) && (y0 < MINAMPLITUDE)) {
		if(soundthere) {
			date = (float)(i - (*p_iorg)) / FSWORK;
			if(date < MINSEGMENT) {
				sprintf(Message,"Skipping segment of duration %.4f sec.",
					date);
				ShowMessage(wMessage,Message);
				for(n=0; n < (*p_nmax); n++) {
					ptr = (*p_a)[n];
					MyEmptyHandle(&ptr);
					}
				n = (*p_nmax) = 0;
				}
			else {
				ShowMessage(wMessage,"Reached end of sound segment.");
				(*p_nmax) = n;
				return(OK);
				}
			}
		soundthere = zerocrossing = 0;
		continue;
		}
	else {
		if(soundthere == 0) {
			soundthere = 1;
			iorg = *p_iorg = i;
			sprintf(Message,"Analyzing from date %.4f sec.",
					(float) i/ FSWORK);
			ShowMessage(wMessage,Message);
			zerocrossing = 0;
			}
		}
	if((sign * yi) < 0) {
		sign = - sign;
		zerocrossing++;
		}
	if((i - iorg) == im) {
		ptr = (long**) GiveSpace((Size)NumberCoeff*sizeof(long));
		(*p_a)[n] = ptr;
		if((*p_a)[n] == NULL) return(ABORT);
		if(GetWalsh(p_y,iorg,i,LOGIM,zerocrossing,p_a,n,p_yy,p_sizeyy)
			== ABORT) return(ABORT);
		n++;
		zerocrossing = 0;
		iorg = i;
		}
	if(y0 <= peak) continue;
	peak = y0;
	}
(*p_nmax) = n;
if(n == 0) return(FAILED);
return(OK);
}


DrawSonagram(int w, long ****p_a, int im, int imax, int nmax)
{
int i,n,p,rep,color,newh,newv,di;
Rect r,block;
int vmin,vmax,hmin,hmax;
float amplitude,phase,a1,a2;

rep = OK;
if(nmax == 0) return(rep);
SetPort(Window[w]);
if(p_Picture[0] != NULL) {
	KillPicture(p_Picture[0]);
	p_Picture[0] = NULL;
	}
Npicture = 1;
TextFont(22);	/* Courier */
r.top = Window[w]->portRect.top;
r.left  = Window[w]->portRect.left;
r.bottom = Window[w]->portRect.bottom - SBARWIDTH - 1;
r.right = Window[w]->portRect.right - SBARWIDTH - 1;
PictureWindow[0] = w;
PictRect[0] = r;
ClipRect(&r);
p_Picture[0] = OpenPicture(&r);
/* $$$ ShowPen(); */
for(n=0,i=0; n < nmax; n++,i+=im) {
	for(p=1; p < (NumberCoeff - 1); p++,p++) {
		Walsh(p,0,LOGIM,&di);	/* Computing only di */
		block.left = r.left + Hzero[w] + i;
		block.right = block.left + im;
		block.bottom = r.top + Vzero[w] - SONAGRAMSHIFT
			- ((p - 1) / 2) * RECTHEIGHT;
		block.top = block.bottom - RECTHEIGHT;
		a1 = ((float) (*((*p_a)[n]))[p]) / im;
		a2 = ((float) (*((*p_a)[n]))[p+1]) / im;
		amplitude = (float) sqrt((double)(a1 * a1 + a2 * a2));
		if(amplitude > 0.000001) {
			phase = (float) acos((double) (a2 / amplitude));
			if(a1 < 0.) phase = - phase;
			if(di > 0) phase = - phase;
			}
		else phase = 0.;
/*		color = Rescale(amplitude,0.,50.,0,2 * im);  */
		color = Rescale(phase,-3.1416,3.1416,-im,im);
		DrawBlock(w,color,block);
		if(Button()) {
			while((rep = MainEvent()) != RESUME && rep != STOP);
			if(rep == STOP) {
				rep = ABORT;
				goto QUIT;
				}
			rep = OK;
			}
		}
	}
hmin = r.left + Hzero[w];
hmax = r.left + Hzero[w] + i;
vmin = r.top + Vzero[w] - SONAGRAMSHIFT - (NumberCoeff/2 * RECTHEIGHT);
vmax = r.top + Vzero[w] - SONAGRAMSHIFT;
ClosePicture();
r.bottom = Window[w]->portRect.bottom - SBARWIDTH - 1;
r.right = Window[w]->portRect.right - SBARWIDTH - 1;
newh = newv = 0;
if(hmin < Hmin[w]) {
	Hmin[w] = hmin;
	newh = 1;
	}
if(hmax > Hmax[w]) Hmax[w] = hmax;
if(vmin < Vmin[w]) {
	Vmin[w] = vmin;
	newv = 1;
	}
if(vmax > Vmax[w]) Vmax[w] = vmax;
if(OKvScroll[w]) SetCtlValue(vScroll[w],0);
if(OKhScroll[w]) SetCtlValue(hScroll[w],0);
SlideH[w] = SlideV[w] = 0;
SetMaxControlValues(w,r);
if(newh) OffsetGraphs(w,Hmin[w] - r.left - 1,0);
if(newv) OffsetGraphs(w,0,Vmin[w] - r.top - 1);
ClipRect(&r);
EraseRect(&r);
SetCursor(&arrow);

QUIT:
return(rep);
}


Rescale(float x, float xmin, float xmax, int ymin, int ymax)
{
int y;
y = ymin + ((x - xmin) / (xmax - xmin) * (ymax - ymin));
if(y < ymin) y = ymin;
if(y > ymax) y = ymax;
return(y);
}


DrawBlock(int w, int color, Rect block)
{
Rect r;
int x,dx,x0,y0,x1,y1,step;
r = (*(Window[w]->clipRgn))->rgnBBox;
ClipRect(&block);
PenPat(gray);
x0 = block.left; y0 = block.bottom;
x1 = block.right; y1 = block.top;
MoveTo(x0,y0); LineTo(x0,y1);
LineTo(x1,y1); LineTo(x1,y0);
PenNormal();
if(color == 0) return(OK);
step = 2 * (1 + ((x1 - x0) / abs(color)));
dx = y1 - y0;
if(color > 0) {
	for(x = (x0 + dx); x < x1; x+=step) {
		MoveTo(x,y0);
		LineTo(x - dx,y1);
		}
	}
else {
	for(x = x0; x < (x1 - dx); x+=step) {
		MoveTo(x,y0);
		LineTo(x + dx,y1);
		}
	}
/*
if(color > (x1 - x0)) {
	step = 2 * (1 + ((x1 - x0) / (color - (x1 - x0))));
	for(x = x0; x < (x1 - dx); x+=step) {
		MoveTo(x,y0);
		LineTo(x + dx,y1);
		}
	}
*/
ClipRect(&r);
return(OK);
}


GetWalsh(int **p_y, int iorg, int iend, int N, int zerocrossing, long ****p_a, int n, int **p_yy, int *p_sizeyy)
{
int i,im,p,r,di,wa;
long y;
float a;

im = iend - iorg;
for(p=0; p < NumberCoeff; p++) {
	(*((*p_a)[n]))[p] = 0L;
	}
for(i=iorg; i < iend; i++) {
	y = (*p_y)[i];
	if(Button()) {
		while((r = MainEvent()) != RESUME && r != STOP);
		if(r == STOP) return(ABORT);
		}
	for(p=0; p < NumberCoeff; p++) {
		(*((*p_a)[n]))[p] += y * Walsh(p,i-iorg,N,&di);
		}
	}
/*
for(p=0; p < NumberCoeff; p++) {
	sprintf(Message,"%ld ",(*((*p_a)[n]))[p]); 
	Print(wData,Message);
	}
Print(wData,"\r\r");
ShowSelect(wData);
*/
for(i=iorg; i < iend; i++) {
	y = 0L;
	for(p=0; p < NumberCoeff; p++) {
		if(Button()) {
			while((r = MainEvent()) != RESUME && r != STOP);
			if(r == STOP) return(ABORT);
			}
		y += (*((*p_a)[n]))[p] * Walsh(p,i-iorg,N,&di);
		}
/*	if(((y >> N) < -200L) || ((y >> N) > 200L)) {
		ShowMessage(wMessage,"Overflow.  Click button.");
		while(!Button());
		} */
	(*p_yy)[i] =  (int) (y >> N) + SOUNDSHIFT;  /*  = y / (2 exp N) */
	(*p_sizeyy)++;
	}
return(OK);
}


Walsh(int p, int i, int N, int *p_di)
{
int w;
return((Wal01(p,i,N,p_di) << 1) - 1);	/* yields a correct ‘di’ */
/*
if(i > p) return((Wal01(p,i,N,p_di) << 1) - 1);
return((Wal01(i,p,N,p_di) << 1) - 1); Faster, but…
										 … ‘di’ will be incorrect. */
}


Wal01(int p, int i, int N, int *p_di)
{
int k,pow,result;
if(p++ == 0) return(1);		/* Beware that p has been incremented ! */
for(k=0,pow=1; k <= N; k++) {
	if(p <= pow) goto EXIT;
	pow = pow << 1;
	}
Alert1("Overflow in Wal01().  Call Bel!");
EXIT:
if(p == pow) {
	*p_di = - (1 << (N - k - 1));
	return(Rad01(k-1,i,N));
	}
result = 1 - (Rad01(k-1,i,N) ^ Wal01(pow-p,i,N,p_di));
if(((pow >> 1) == --p) && (p != 1)) *p_di = - *p_di;
return(result);
}


Rad01(int n, int i, int N)
{
return(1 - ((i >> (N - n - 1)) & 1));
/* return(1 - ((int)(i / Power2(N - n - 1)) & 1)); */
}



Power2(int x)
{
int n=1;
if(x == 0) return(1);
if(x > 15) {
	Alert1("Overflow in Power2(x). Call Bel!");
	return(1);
	}
if(x < 0) {
	Alert1("x < 0 in Power2(x). Call Bel!");
	return(1);
	}
return(n << x);
}



Test(void)
{
int i,im,p,N,w,di;
PleaseWait();
for(N = 1; N < 6; N++) {
	im = Power2(N);
	Print(wData,"\r");
	for(p=0; p < im; p++) {
		for(i=0; i < im; i++) {
			w = Walsh(p,i,N,&di);
			sprintf(Message,"%d",(w+1)/2);
			Print(wData,Message);
			}
		sprintf(Message," p = %d  \r",p);
		Print(wData,Message);
		ShowSelect(wData);
		}
	}
}




Test1(void)
{
int i,i1,im,p,N,w,di;
PleaseWait();
for(N = 1; N < 8; N++) {
	im = Power2(N);
	Print(wData,"\r");
	for(p=1; p < im-1; p++,p++) {
		sprintf(Message,"%d ",p);
		Print(wData,Message);
		ShowSelect(wData);
		Walsh(p,im-1,N,&di);  /* Calculating di.  i can't be < im/2 */
/*		sprintf(Message," di = %d\r",di);
		Print(wData,Message);
		for(i=0; i < im; i++) {
			i1 = Mymod((i-di),im);
			sprintf(Message,"%d",Wal01(p,i1,N,&di));
			Print(wData,Message);
			}
		Print(wData,"\r");
		for(i=0; i < im; i++) {
			sprintf(Message,"%d",Wal01(p+1,i,N,&di));
			Print(wData,Message);
			}
		Print(wData,"\r\r"); */
		for(i=0; i < im; i++) {
			i1 = Mymod((i-di),im);
			if(Walsh(p,i1,N,&di) != Walsh(p+1,i,N,&di)) {
				Alert1("Error");
				break;
				}
			} 
		}
	}
}


Mymod(int a, int b)
{
while(a < 0) a += b;
return(a - b * ((int) (a / b)));
}