/* CsoundMaths.c (BP3) */ 

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


#ifndef _H_BP3
#include "-BP3.h"
#endif

#include "-BP3decl.h"

int trace_csound_maths = 0;
int trace_write_score = 0;

int CompileRegressions(void)
{
int j,rep;

if(CompiledRegressions) return(OK);
for(j=0; j < Jinstr; j++) {
	if((rep=GetRegressions(j)) != OK) {
		BPPrintMessage(0,odError,"=> Problem in GetRegressions()\n");
		return(rep);
		}
	}
CompiledRegressions = YES;
return(OK);
}


int GetRegressions(int j)
{
int result;
double a,b,c;
regression r;

// Calculate quadratic regressions

if(trace_csound_maths) BPPrintMessage(0,odInfo,"Compiling regressions for Csound instrument %d\n",j);

r = (*p_CsInstrument)[j].rPitchBend;
if((result=Findabc(p_CsPitchBend,j,&r)) == ABORT) {
	BPPrintMessage(0,odError,"=> Incorrect mapping for pitchbend");
	goto ERR;
	}
(*p_CsInstrument)[j].rPitchBend = r;
if(result == MISSED)
	(*p_CsInstrument)[j].rPitchBend.islogx = (*p_CsInstrument)[j].rPitchBend.islogy = FALSE;

r = (*p_CsInstrument)[j].rVolume;
if((result=Findabc(p_CsVolume,j,&r)) == ABORT) {
	BPPrintMessage(0,odError,"=> Incorrect mapping for volume");
	goto ERR;
	}
(*p_CsInstrument)[j].rVolume = r;
if(result == MISSED)
	(*p_CsInstrument)[j].rVolume.islogx = (*p_CsInstrument)[j].rVolume.islogy = FALSE;

r = (*p_CsInstrument)[j].rPressure;
if((result=Findabc(p_CsPressure,j,&r)) == ABORT) {
	BPPrintMessage(0,odError,"=> Incorrect mapping for pressure");
	goto ERR;
	}
(*p_CsInstrument)[j].rPressure = r;
if(result == MISSED)
	(*p_CsInstrument)[j].rPressure.islogx = (*p_CsInstrument)[j].rPressure.islogy = FALSE;

r = (*p_CsInstrument)[j].rModulation;
if((result=Findabc(p_CsModulation,j,&r)) == ABORT) {
	BPPrintMessage(0,odError,"=> Incorrect mapping for modulation");
	goto ERR;
	}
(*p_CsInstrument)[j].rModulation = r;
if(result == MISSED)
	(*p_CsInstrument)[j].rModulation.islogx = (*p_CsInstrument)[j].rModulation.islogy = FALSE;

r = (*p_CsInstrument)[j].rPanoramic;
if((result=Findabc(p_CsPanoramic,j,&r)) == ABORT) {
	BPPrintMessage(0,odError,"=> Incorrect mapping for panoramic");
	goto ERR;
	}
(*p_CsInstrument)[j].rPanoramic = r;
if(result == MISSED)
	(*p_CsInstrument)[j].rPanoramic.islogx = (*p_CsInstrument)[j].rPanoramic.islogy = FALSE;

#if BP_CARBON_GUI_FORGET_THIS
SetCsoundLogButtons(j);
#endif /* BP_CARBON_GUI_FORGET_THIS */

return(OK);

ERR:
iCsoundInstrument = j;
BPActivateWindow(SLOW,wCsoundResources);
return(ABORT);
}


int Findabc(double ***p_xy,int j,regression *p_r)
/* Determine a quadratic regression of coordinates (x1,y1),(x2,y2),(x3,y3) */
{
double x1,x2,x3,y1,y2,y22,y3,D,Da,Db,Dc,xmin,xmax,ymax,m;

double infinite_number = 2147483647;
// 2^31 - 1 (Mersenne) Used in the PHP interface and in BP2.9.8
// We use this number in replacement of Infpos

p_r->a = 0.;  p_r->b = 1.;  p_r->c = 0.;
p_r->isquadra12 = p_r->isquadra23 = TRUE;
p_r->x2 = p_r->y2 = 0.;
p_r->crescent = TRUE;

if((*(p_xy[0]))[j] < infinite_number) x1 /* = p_r->x1 */ = (*(p_xy[0]))[j];
else return(MISSED);
if((*(p_xy[1]))[j] < infinite_number) x2 = p_r->x2 = (*(p_xy[1]))[j];
else return(MISSED);
if((*(p_xy[2]))[j] < infinite_number) x3 /* = p_r->x3 */ = (*(p_xy[2]))[j];
else return(MISSED);
if((*(p_xy[3]))[j] < infinite_number) y1 = (*(p_xy[3]))[j];
else return(MISSED);
if((*(p_xy[4]))[j] < infinite_number) y2 = p_r->y2 = (*(p_xy[4]))[j];
else return(MISSED);
if((*(p_xy[5]))[j] < infinite_number) y3 = (*(p_xy[5]))[j];
else return(MISSED);

// BPPrintMessage(0,odError,"Infpos = %ld\n",Infpos);
if(trace_csound_maths) BPPrintMessage(0,odError,"x1 = %.3f y1 = %.3f x2 = %.3f y2 = %.3f x3 = %.3f y3 = %.3f \n",x1,y1,x2,y2,x3,y3);

if(y1 < y2) p_r->crescent = TRUE;
else p_r->crescent = FALSE;
if((x2 <= x1) || (x3 <= x2)) goto NOTMONOTONOUS;
if((y2 - y1) * (y2 - y3) >= 0.) goto NOTMONOTONOUS;

if(p_r->islogx && !p_r->islogy) {
	xmax = 0.;
	if(fabs(x1) > xmax) xmax = fabs(x1);
	if(fabs(x2) > xmax) xmax = fabs(x2);
	if(fabs(x3) > xmax) xmax = fabs(x3);
	p_r->scalex = 6. / xmax;
	x1 = /* p_r->x1 = */ exp(x1 * p_r->scalex);
	x2 = p_r->x2 = exp(x2 * p_r->scalex);
	x3 = /* p_r->x3 = */ exp(x3 * p_r->scalex);
	}
if(p_r->islogy && !p_r->islogx) {
	ymax = 0.;
	if(fabs(y1) > ymax) ymax = fabs(y1);
	if(fabs(y2) > ymax) ymax = fabs(y2);
	if(fabs(y3) > ymax) ymax = fabs(y3);
	p_r->scaley = 6. / ymax;
	y1 = exp(y1 * p_r->scaley);
	y2 = p_r->y2 = exp(y2 * p_r->scaley);
	y3 = exp(y3 * p_r->scaley);
	}
y22 = y1 + (x2 - x1) * ((y3 - y1)/(x3 - x1));
if(fabs((y22 - y2) / (y3 - y1)) < 0.01) {
	goto NOTQUADRA;
	}
D = x1 * x1 * (x2 - x3) + x2 * x2 * (x3 - x1) + x3 * x3 * (x1 - x2);
if(fabs(D) < 0.001) goto NOTQUADRA;
Da = y1 * (x2 - x3) + y2 * (x3 - x1) + y3 * (x1 - x2);
Db = x1 * x1 * (y2 - y3) + x2 * x2 * (y3 - y1) + x3 * x3 * (y1 - y2);
Dc = x1 * x1 * (x2 * y3 - x3 * y2) + x2 * x2 * (x3 * y1 - x1 * y3)
	+ x3 * x3 * (x1 * y2 - x2 * y1);

p_r->a = Da / D;  p_r->b = Db / D;  p_r->c = Dc / D;

// Check that the regression is monotonous

xmin = - p_r->b / 2. / p_r->a;
if((xmin - x1) * (xmin - x2) < 0.) goto NOTQUADRA12;
if((xmin - x2) * (xmin - x3) < 0.) goto NOTQUADRA23;

return(OK);

NOTMONOTONOUS:
ShowMessage(TRUE,wMessage,"Can't determine mapping because functions are not monotonous, or x values are not crescent");
return(OK);

NOTQUADRA12:
p_r->isquadra12 = FALSE;
m = (y2 - y1) / (x2 - x1);
D = 2. * x2 * (x2 - x3) - (x2 * x2 - x3 * x3);
if(fabs(D) < 0.001) goto NOTQUADRA;
Da = m * (x2 - x3) - (y2 - y3);
Db = 2. * x2 * (y2 - y3) - m * (x2 * x2 - x3 * x3);
Dc = 2. * x2 * (x2 * y3 - x3 * y2) - (x2 * x2 * y3 - x3 * x3 * y2)
	+ m * (x2 * x2 * x3 - x3 * x3 * x2);
p_r->a = Da / D;  p_r->b = Db / D;  p_r->c = Dc / D;
goto FINDLINEAR;

NOTQUADRA23:
p_r->isquadra23 = FALSE;
m = (y3 - y2) / (x3 - x2);
D = 2. * x2 * (x2 - x1) - (x2 * x2 - x1 * x1);
if(fabs(D) < 0.001) goto NOTQUADRA;
Da = m * (x2 - x1) - (y2 - y1);
Db = 2. * x2 * (y2 - y1) - m * (x2 * x2 - x1 * x1);
Dc = 2. * x2 * (x2 * y1 - x1 * y2) - (x2 * x2 * y1 - x1 * x1 * y2)
	+ m * (x2 * x2 * x1 - x1 * x1 * x2);
p_r->a = Da / D;  p_r->b = Db / D;  p_r->c = Dc / D;
goto FINDLINEAR;

NOTQUADRA:
p_r->isquadra12 = p_r->isquadra23 = FALSE;

FINDLINEAR:
if(x1 < x2) {
	p_r->a12 = (y2 - y1) / (x2 - x1);
	p_r->b12 = y1 - (p_r->a12 * x1);
	p_r->a23 = (y3 - y2) / (x3 - x2);
	p_r->b23 = y2 - (p_r->a23 * x2);
	}
else {
	p_r->a23 = (y2 - y1) / (x2 - x1);
	p_r->b23 = y1 - (p_r->a23 * x1);
	p_r->a12 = (y3 - y2) / (x3 - x2);
	p_r->b12 = y2 - (p_r->a12 * x2);
	}
return(OK);
}


double XtoY(double xin,regression *p_r,int *p_overflow,int ins)
{
double x,y,xmin;

*p_overflow = FALSE;
if(p_r->islogx && !p_r->islogy) x = exp(xin * p_r->scalex);
else x = xin;

if(xin < p_r->x2) {
	if(p_r->isquadra12) {
FINDQUADRA:
		xmin = - p_r->b / 2. / p_r->a;
		if((xmin - p_r->x2) * (xmin - xin) < 0.) goto DO_OVERFLOW;
		y = (((p_r->a * x) + p_r->b) * x) + p_r->c;
		}
	else y = (p_r->a12 * x) + p_r->b12;
	}
else {
	if(p_r->isquadra23) goto FINDQUADRA;
	else y = (p_r->a23 * x) + p_r->b23;
	}

if(p_r->islogy && !p_r->islogx) {
	if(y <= 0.) goto DO_OVERFLOW;
	y = log(y) / p_r->scaley;
	}
return(y);

DO_OVERFLOW:
my_sprintf(Message,
	"Parameter value '%.2f' is out of range...\nCheck parameter mappings in Csound instrument %ld",
	xin,(long)(*p_CsInstrumentIndex)[ins]);
BPPrintMessage(0,odError,"%s",Message);
iCsoundInstrument = ins;
#if BP_CARBON_GUI_FORGET_THIS
SetCsoundInstrument(iCsoundInstrument,-1);
#endif /* BP_CARBON_GUI_FORGET_THIS */
*p_overflow = TRUE;
return(0.);
}


double YtoX(double yin,regression *p_r,int *p_overflow,int ins)
{
double x,y,xmin,delta;

*p_overflow = FALSE;
if(p_r->islogy && !p_r->islogx) y = exp(yin * p_r->scaley);
else y = yin;

if((p_r->crescent && yin <= p_r->y2) || (!p_r->crescent && yin > p_r->y2)) {
	if(p_r->isquadra12) {
FINDQUADRA:
		if(p_r->a == 0.) return(yin);
		xmin = - p_r->b / 2. / p_r->a;
		delta = (p_r->b * p_r->b) - 4.* p_r->a * (p_r->c - y);
		if(delta < 0.) {
			BPPrintMessage(0,odError,"=> Err. YtoX(). delta < 0");
			goto DO_OVERFLOW;
			}
		x = (- p_r->b - sqrt(delta)) / 2. / p_r->a;
		if((xmin - p_r->x2) * (xmin - x) < 0.) {
			x = (- p_r->b + sqrt(delta)) / 2. / p_r->a;
			if((xmin - p_r->x2) * (xmin - x) < 0.) {
				BPPrintMessage(0,odError,"=> Err. YtoX(). ((xmin - p_r->x2) * (xmin - x) < 0.)"); 
				goto DO_OVERFLOW;
				}
			}
		}
	else {
		if(p_r->a12 == 0.) {
			BPPrintMessage(0,odError,"=> Err. YtoX(). p_r->a12 == 0");
			goto DO_OVERFLOW;
			}
		x = (y - p_r->b12) / p_r->a12;
		}
	}
else {
	if(p_r->isquadra23) goto FINDQUADRA;
	else {
		if(p_r->a23 == 0.) {
			BPPrintMessage(0,odError,"=> Err. YtoX(). p_r->a23 == 0");
			goto DO_OVERFLOW;
			}
		x = (y - p_r->b23) / p_r->a23;
		}
	}

if(p_r->islogx && !p_r->islogy) {
	if(x <= 0.) {
		BPPrintMessage(0,odError,"Overflow in parameter mapping (log of a negative number)");
		goto DO_OVERFLOW;
		}
	x = log(x) / p_r->scalex;
	}
return(x);

DO_OVERFLOW:
my_sprintf(Message,
	"Parameter value '%.2f' does not match specified range: check parameter mappings in Csound instrument %ld",
	yin,(long)(*p_CsInstrumentIndex)[ins]);
iCsoundInstrument = ins;
#if BP_CARBON_GUI_FORGET_THIS
SetCsoundInstrument(iCsoundInstrument,-1);
#endif /* BP_CARBON_GUI_FORGET_THIS */
BPPrintMessage(0,odError,"%s",Message);
*p_overflow = TRUE;
return(0.);
}


double Remap(double x,int ins,int i,int *p_overflow)
{

*p_overflow = FALSE;
switch(i) {
	case IPITCHBEND: return(XtoY(x,&((*p_CsInstrument)[ins].rPitchBend),p_overflow,ins));
		break;
	case IVOLUME: return(XtoY(x,&((*p_CsInstrument)[ins].rVolume),p_overflow,ins));
		break;
	case IMODULATION: return(XtoY(x,&((*p_CsInstrument)[ins].rModulation),p_overflow,ins));
		break;
	case IPRESSURE: return(XtoY(x,&((*p_CsInstrument)[ins].rPressure),p_overflow,ins));
		break;
	case IPANORAMIC: return(XtoY(x,&((*p_CsInstrument)[ins].rPanoramic),p_overflow,ins));
		break;
	}
return(x);
}


int MakeCsoundFunctionTable(int onoffline,double **scorearg,double alpha1,double alpha2,long imax,
	Coordinates** coords,int ins,int paramnameindex,int ip,int iarg0,int iarg1)
{
int r,result,overflow,gentype,usescorevalues;
long i,n,lasti,newx,sum;
double x,y,iscale,oldx,xmax,v0,v1;
char line[MAXLIN],line2[MAXLIN];
Handle h;
XYgraph subtable;

if(scorearg == NULL) {
	BPPrintMessage(0,odError,"=> Err. MakeCsoundFunctionTable(). scorearg == NULL");
	return(MISSED);
	}
if(alpha1 < 0. || alpha2 > 1. || (alpha1 >= alpha2)) return(MISSED);

r = ABORT;
subtable.imax = ZERO;
subtable.point = NULL;
result = GetPartOfTable(&subtable,alpha1,alpha2,imax,coords);
if(result == ABORT) return(result);
if(subtable.imax <= ZERO) {
//	BPPrintMessage(0,odError,"=> Err. MakeCsoundFunctionTable(). subtable.imax <= ZERO");
	r = MISSED;
	goto SORTIR;
	}
if(result == OK) {
	usescorevalues = onoffline;
	if(usescorevalues) {
		if(iarg0 > 0) v0 = (*scorearg)[iarg0];
		else usescorevalues = FALSE;
		if(iarg1 > 0) v1 = (*scorearg)[iarg1];
		else usescorevalues = FALSE;
		}
	lasti = subtable.imax - 1L;
	xmax = (*(subtable.point))[lasti].x;
	x = log(xmax) / log(2.);
	if(x == (long)x) n = x;
	else n = 1L + (long)x;
	n = 1L << n;
	if(n < 256L) n = 256L;	/* This provides a minimum accuracy */
	iscale = ((double) n) / xmax;
	y = (*(subtable.point))[ZERO].value;
	oldx = 0.;
	if(usescorevalues) y = CombineScoreValues(y,oldx,xmax,v0,v1,ins,paramnameindex,ip);
	y = Remap(y,ins,paramnameindex,&overflow);
	if(overflow) goto SORTIR;
	FunctionTable++;
	gentype = GetGENtype(ins,ip,paramnameindex);
	my_sprintf(line,"f%ld %.3f %ld -%ld %.3f",(long)FunctionTable,(*scorearg)[2],
		(long)n,(long)gentype,y);
	newx = sum = 0;
	oldx = 0.;
	for(i=1L; i <= lasti; i++) {
		if(i == lasti)
			newx = n - sum;	/* This compensates rounding errors */
		else 
			newx = MyInt(((*(subtable.point))[i].x - oldx) * iscale);
		oldx = (*(subtable.point))[i].x;
		sum += newx;
		my_sprintf(line2," %ld",(long)newx);
		strcat(line,line2);
		y = (*(subtable.point))[i].value;
		if(i == lasti && trace_write_score) BPPrintMessage(0,odInfo,"y(1) = %.3f for i = %ld\n",y,(long)i);
		if(usescorevalues) y = CombineScoreValues(y,oldx,xmax,v0,v1,ins,paramnameindex,ip);
		if(i == lasti && trace_write_score) BPPrintMessage(0,odInfo,"y(2) = %.3f for i = %ld\n",y,(long)i);
		y = Remap(y,ins,paramnameindex,&overflow);
		if(i == lasti && trace_write_score) BPPrintMessage(0,odInfo,"y(3) = %.3f for i = %ld\n",y,(long)i);
		if(overflow) goto SORTIR;
		if(i == lasti && trace_write_score) BPPrintMessage(0,odInfo,"y(4) = %.3f for i = %ld\n",y,(long)i);
		my_sprintf(line2," %.3f",y);
		strcat(line,line2);
		}
	if(CsoundTrace) ShowMessage(TRUE,wMessage,line);
	if(trace_write_score) BPPrintMessage(0,odInfo,"%s\n",line);
	if(ConvertMIDItoCsound) Println(wPrototype7,line);
	else WriteToFile(NO,CsoundFileFormat,line,CsRefNum);
	}
r = OK;

SORTIR:
h = (Handle) subtable.point;
MyDisposeHandle(&h);
return(r);
}


int GetGENtype(int ins,int ip,int paramnameindex)
{
if(paramnameindex <= IPANORAMIC) {
	switch(paramnameindex) {
		case IPITCHBEND:
			return((*p_CsInstrument)[ins].pitchbendGEN);
		case IVOLUME:
			return((*p_CsInstrument)[ins].volumeGEN);
		case IPRESSURE:
			return((*p_CsInstrument)[ins].pressureGEN);
		case IMODULATION:
			return((*p_CsInstrument)[ins].modulationGEN);
		case IPANORAMIC:
			return((*p_CsInstrument)[ins].panoramicGEN);
		default:
			return(7);
			break;
		}
	}
if(ip >= 0) {
	if(ins < 0 || ins >= Jinstr) {
		BPPrintMessage(0,odError,"=> Err. GetGENtype(). ins < 0 || ins >= Jinstr");
		return(7);
		}
/*	if(ip >= IPMAX) {
		BPPrintMessage(0,odError,"=> Err. GetGENtype(). ip >= IPMAX");
		return(7);
		} */
	if((*p_CsInstrument)[ins].paramlist == NULL) {
		BPPrintMessage(0,odError,"=> Err. GetGENtype(). (*p_CsInstrument)[ins].paramlist == NULL");
		return(7);
		}
	if(ip >= (*p_CsInstrument)[ins].ipmax) {
		BPPrintMessage(0,odError,"=> Err. GetGENtype(). ip >= (*p_CsInstrument)[ins].ipmax");
		return(7);
		}
	return((*((*p_CsInstrument)[ins].paramlist))[ip].GENtype);
	}
return(7);
}


int GetPartOfTable(XYgraph *p_subtable,double alpha1,double alpha2,long imax,Coordinates** coords)
{
Handle h;
long i,j,xmax,i1,i2,x1,x2;
double ix1,ix2,y1,y2,startvalue,endvalue;

if(alpha1 < 0.) {
	if(Beta && alpha1 < -0.01) BPPrintMessage(0,odError,"=> Err. GetPartOfTable(). alpha1 < -0.01");
	alpha1 = 0.;
	}
if(alpha1 > 1.) {
	if(Beta && alpha1 > 1.01) BPPrintMessage(0,odError,"=> Err. GetPartOfTable(). alpha1 > 1.01");
	alpha1 = 1.;
	}
if(alpha2 < 0.) {
	if(Beta && alpha2 < -0.01) BPPrintMessage(0,odError,"=> Err. GetPartOfTable(). alpha2 < -0.01");
	alpha2 = 0.;
	}
if(alpha2 > 1.) {
	if(Beta && alpha2 > 1.01) BPPrintMessage(0,odError,"=> Err. GetPartOfTable(). alpha2 > 1.01");
	alpha2 = 1.;
	}
if(alpha1 >= alpha2) {
	BPPrintMessage(0,odError,"=> Err. GetPartOfTable(). alpha1 >= alpha2");
	return(MISSED);
	}

h = (Handle) p_subtable->point;
MyDisposeHandle(&h);
p_subtable->point = NULL;
p_subtable->imax = ZERO;

if(imax <= ZERO) {
	BPPrintMessage(0,odError,"=> Err. GetPartOfTable(). imax <= ZERO");
	return(MISSED);
	}
if(coords == NULL) {
	BPPrintMessage(0,odError,"=> Err. GetPartOfTable(). coords == NULL");
	return(MISSED);
	}
{
	if(imax > MyGetHandleSize((Handle)coords) / sizeof(Coordinates)) {
		BPPrintMessage(0,odError,"=> Err. GetPartOfTable(). imax >= MyGetHandleSize((Handle)coords) / sizeof(Coordinates)");
		return(MISSED);
		}
	}
xmax = (*(coords))[imax-1L].i;

ix1 = alpha1 * (double) xmax;
i = 0; while((*(coords))[i].i < ix1) i++;
i1 = i;

if(trace_write_score) BPPrintMessage(0,odInfo,"i1 = %ld\n",i1);

if(i1 > ZERO) {
	x2 = (*(coords))[i1].i;
	x1 = (*(coords))[i1-1L].i; 
	y2 = (*(coords))[i1].value;
	y1 = (*(coords))[i1-1L].value;
	if(x1 >= x2) {
		BPPrintMessage(0,odError,"=> Err. GetPartOfTable(). x1 >= x2");
		return(MISSED);
		}
	startvalue = y1 + (y2 - y1) * (ix1 - x1) / (x2 - x1);
	if(trace_write_score) BPPrintMessage(0,odInfo,"i1 = %ld, x1 = %.3f, y1 = %.3f, x2 = %.3f, y2 = %.3f, ix1 = %.3f, startvalue = %.3f\n",i1,x1,y1,x2,y2,ix1,startvalue);
	}
else {
	startvalue = (*(coords))[ZERO].value;
	if(trace_write_score) BPPrintMessage(0,odInfo,"i1 = ZERO startvalue = %.3f\n",startvalue);
	}

ix2 = alpha2 * (double) xmax;
i = 0; while((*(coords))[i].i < ix2) i++;
i2 = i;

if(trace_write_score) BPPrintMessage(0,odInfo,"i2 = %ld\n",i2);

if(i2 > ZERO) {
	x2 = (*(coords))[i2].i;
	x1 = (*(coords))[i2-1L].i; 
	y2 = (*(coords))[i2].value;
	if(y2 < Infneg || y2 > Infpos) {
		y2 = 0.; // Dirty fix by BB, 10 Nov. 2020 after seeing rare problem occurring in -gr.vina3
		BPPrintMessage(0,odError,"=> Error in GetPartOfTable(). y2 = %.3f, x1 = %.3f, x2 = .%3f; i2 = %ld (fixed)\n",y2,x1,x2,(long)i2);
		}
	y1 = (*(coords))[i2-1L].value;
	if(y1 < Infneg || y1 > Infpos) {
		y1 = 0.; // Dirty fix by BB, 10 Nov. 2020 after seeing rare problem occurring in -gr.vina3
		BPPrintMessage(0,odError,"=> Error in GetPartOfTable(). y1 = %.3f, x1 = %.3f, x2 = .%3f; i2 = %ld (fixed)\n",y1,x1,x2,(long)i2);
		}
	if(x1 >= x2) {
		BPPrintMessage(0,odError,"=> Err. GetPartOfTable(). x1 >= x2");
		BPPrintMessage(0,odError,"=> Error in GetPartOfTable(). x1 >= x2\n");
		return(MISSED);
		}
	endvalue = y1 + (y2 - y1) * (ix2 - x1) / (x2 - x1);
	if(trace_write_score) BPPrintMessage(0,odInfo,"i2 = %ld, x1 = %.3f, y1 = %.3f, x2 = %.3f, y2 = %.3f, ix2 = %.3f, endvalue = %.3f\n",i2,x1,y1,x2,y2,ix2,endvalue);
	}
else {
	endvalue = (*(coords))[ZERO].value;
	if(trace_write_score) BPPrintMessage(0,odInfo,"i2 = ZERO endvalue = %.3f\n",endvalue);
	}
p_subtable->imax = i2 - i1 + 3L;
if(trace_write_score) BPPrintMessage(0,odInfo,"p_subtable i1 = %ld i2 = %ld imax = %ld\n",(long)i1,(long)i2,(long)imax);
if((p_subtable->point = (XYCoordinates**) GiveSpace((Size)((imax + 1) * sizeof(XYCoordinates)))) == NULL)
	return(ABORT);

j = ZERO;
(*(p_subtable->point))[j].value = startvalue;
(*(p_subtable->point))[j++].x = 0.;

if(startvalue != (*(coords))[i1].value) {
	(*(p_subtable->point))[j].value = (*(coords))[i1].value;
	(*(p_subtable->point))[j++].x = (*(coords))[i1].i;
	}

for(i=i1+1L; i <= i2; i++) {
	(*(p_subtable->point))[j].value = (*(coords))[i].value;
	(*(p_subtable->point))[j++].x = (*(coords))[i].i;
	}

(*(p_subtable->point))[j-1].value = endvalue;
// BPPrintMessage(0,odInfo,"(*(p_subtable->point))[j-1].value = %.3f\n",endvalue);
(*(p_subtable->point))[j-1].x = ix2;	/* May not be integer value */

// if(trace_write_score) BPPrintMessage(0,odInfo,"startvalue = %.3f endvalue = %.3f\n",startvalue,endvalue);

if(j < 3L) return(MISSED);

for(i=1L; i < j; i++) (*(p_subtable->point))[i].x -= ix1;

p_subtable->imax = j;

return(OK);
}


double CombineScoreValues(double y,double x,double xmax,double v0,double v1,int ins,
	int paramnameindex,int ip)
{
int combinationtype;
double yinterp,ydefault,ynew;

if(paramnameindex <= IPANORAMIC) {
	switch(paramnameindex) {
		case IPITCHBEND:
			combinationtype = ADD;
			ydefault = DEFTPITCHBEND;
			break;
		case IPRESSURE:
			combinationtype = ADD;
			ydefault = DEFTPRESSURE;
			break;
		case IPANORAMIC:
			combinationtype = ADD;
			ydefault = DeftPanoramic;
			break;
		case IMODULATION:
			combinationtype = ADD;
			ydefault = DEFTMODULATION;
			break;
		case IVOLUME:
			combinationtype = MULT;
			ydefault = DeftVolume;
			break;
		}
	goto FINDIT;
	}
if(ip >= 0) {
	if(ins < 0 || ins >= Jinstr) {
		BPPrintMessage(0,odError,"=> Err. CombineScoreValues(). ins < 0 || ins >= Jinstr");
		return(y);
		}
/*	if(ip >= IPMAX) {
		BPPrintMessage(0,odError,"=> Err. CombineScoreValues(). ip >= IPMAX");
		return(y);
		} */
	if((*p_CsInstrument)[ins].paramlist == NULL) {
		BPPrintMessage(0,odError,"=> Err. CombineScoreValues(). (*p_CsInstrument)[ins].paramlist == NULL");
		return(y);
		}
	if(ip >= (*p_CsInstrument)[ins].ipmax) {
		BPPrintMessage(0,odError,"=> Err. CombineScoreValues(). ip >= (*p_CsInstrument)[ins].ipmax");
		return(y);
		}
	combinationtype = (*((*p_CsInstrument)[ins].paramlist))[ip].combinationtype;
	ydefault = (*((*p_CsInstrument)[ins].paramlist))[ip].defaultvalue;
	}
else {
	BPPrintMessage(0,odError,"=> Err. CombineScoreValues(). ip < 0");
	return(y);
	}
	
FINDIT:
if(combinationtype != ADD && combinationtype != MULT) {
	BPPrintMessage(0,odError,"=> Err. CombineScoreValues(). combinationtype != ADD && combinationtype != MULT");
	return(y);
	}
if(x == 0.) yinterp = v0;
else if(x == xmax) yinterp = v1;
else {
	if(xmax == 0.) {
		BPPrintMessage(0,odError,"=> Err. CombineScoreValues(). xmax == 0");
		yinterp = v0;
		}
	else yinterp = v0 + (x * (v1 - v0)) / xmax;
	}
if(combinationtype == ADD)
	ynew = y + yinterp - ydefault;
else {
	if(ydefault == 0.) {
		BPPrintMessage(0,odError,"=> Err. CombineScoreValues(). ydefault == 0");
		return(y);
		}
	ynew = y * yinterp / ydefault;
	}
return(ynew);
}