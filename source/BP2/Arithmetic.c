/* Arithmetic.c (BP2 version CVS) */ 

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



Equal(double accuracy,double p,double q,double r,double s,int* p_overflow)
{
int overflow;
double D,x,y,dif;


(*p_overflow) = FALSE;
if(p == 0. && r == 0.) return(YES);
if(q == 0. || s == 0.) {
	(*p_overflow) = TRUE;
	return(ABORT);
	}

D = LCM(q,s,&overflow);
if(D < 1. || D >= ULONG_MAX || overflow) {
	x = p / q;
	y = r / s;
	dif = fabs(x - y);
	(*p_overflow) = TRUE;
	if((dif / fabs(x)) < accuracy) return(YES);
	else return(NO);
	}
if(((D/q) * p) == ((D/s) * r)) return(YES);
return(NO);
}


Add(double p,double q,double r,double s,
	double *p_P,double *p_Q,int* p_overflow)
{
double qq,ss;
unsigned long E;
int overflow;
double D,N,x,y,z;

(*p_overflow) = FALSE;

if(r == 0.) {
	*p_P = p;
	*p_Q = q;
	return(OK);
	}
if(q == 0. || s == 0.) return(FAILED);

qq = q; ss = s;
while((D = LCM(qq,ss,&overflow)) < 1. || D >= ULONG_MAX || overflow) {
	(*p_overflow) = TRUE;
	qq = qq / 2.;
	ss = ss / 2.;
	if(D < 1. || qq == 0. || ss == 0.) {
		x = p / q;
		y = r / s;
		z = x + y;
		return(MakeRatio((double)ULONG_MAX,z,p_P,p_Q));
		}
	}
N = ((D/qq) * p) + ((D/ss) * r);

if(D >= ULONG_MAX || (E=GCD(N,D)) < 1L) E = 1L;
(*p_P) = N / E;
(*p_Q) = D / E;
return(OK);
}


Substract(double p,double q,double r,double s,double *p_P,double *p_Q,
	int *p_sign,int* p_overflow)
{
double qq,ss;
int overflow;
double D,N,n1,n2,x,y,z;
unsigned long E;

(*p_overflow) = FALSE;

*p_sign = 1;
if(r == ZERO) {
	(*p_P) = p; (*p_Q) = q;
	return(OK);
	}

qq = q; ss = s;
while((D = LCM(qq,ss,&overflow)) < 1. || D >= ULONG_MAX || overflow) {
	(*p_overflow) = TRUE;
	qq = qq / 2.;
	ss = ss / 2.;
	if(D < 1. || qq == ZERO || ss == ZERO) {
		x = p / q;
		y = r / s;
		z = x - y;
		return(MakeRatio((double)ULONG_MAX,z,p_P,p_Q));
		}
	}
n1 = (D/qq) * p;
n2 = (D/ss) * r;
if(n1 > n2) N = n1 - n2;
else {
	(*p_sign) = -1;
	N = n2 - n1;
	}

if(D >= ULONG_MAX || (E=GCD(N,D)) < 1L) E = 1L;
*p_P = N / E;
*p_Q = D / E;
return(OK);
}


Eucl(unsigned long a,unsigned long b,unsigned long *p_q,unsigned long *p_r)
{

if(b == ZERO) return(-1);

(*p_r) = a % b;
(*p_q) = (a - *p_r) / b;
return(0);
}


Simplify(double limit,double p,double q,double *p_p,double *p_q)
/* Fixed 24 May 2007 by BB */
{
unsigned long gcd;
int neg,result;
double x,y;

result = OK;
if(p < 0.) {
	neg = TRUE;
	p = - p;
	}
else neg = FALSE;

if(p == 0.) {
ISZERO:
	(*p_p) = 0.;
	(*p_q) = 1.;
	return(result);
	}
	
if(q == 0.) {
	result = FAILED;
	goto OUT;
	}

if(p < 1.) {
	q = Round(1000. * q / p); 
	p = 1000.;
	goto INTEGERRATIO;
	}
if(q < 1.) {
	p = Round(1000. * p / q);
	q = 1000.;
	goto INTEGERRATIO;
	}
	
y = modf(p,&x);
if(fabs(y-x) > 0.001) {
	p = Round(1000. * p);
	q = 1000. * q;
	}
	
y = modf(q,&x);
if(fabs(y-x) > 0.001) {
	p = 1000. * p;
	q = Round(1000. * q);
	}

INTEGERRATIO:
if((gcd=GCD(p,q)) > 1.) {
	p = p / gcd;
	q = q / gcd;
	}
	
while(p > limit || q > limit) {
	p = Round(p / 10.);
	q = Round(q / 10.);
	if((gcd=GCD(p,q)) > 1.) {
		p = p / gcd;
		q = q / gcd;
		}
	}
	
if(q < 1.) result = FAILED;
if(p < 1.) goto ISZERO;

OUT:
(*p_p) = p;
(*p_q) = q;
if(neg) (*p_p) = - (*p_p);
return(result);
}


double LCM(double p,double q,int *p_overflow)
{
double r;
unsigned long gcd;

(*p_overflow) = FALSE;
if(p == q) return(p);
if(p < 1. || q  < 1.) {
	(*p_overflow) = TRUE;
	return(ZERO);
	}
if((gcd=GCD(p,q)) == ZERO)  {
	TellComplex();
	(*p_overflow) = TRUE;
/*	return(p * q); */
	if(p > q) return(p);
	else return(q);
	}
r = (p / gcd) * q;
return(r);
}


unsigned long GCD(double p,double q)
{
unsigned long pp,qq,qmem;

if(p < 1. || q < 1. || p >= ULONG_MAX || q >= ULONG_MAX) return(ZERO);
pp = (unsigned long) p;
qq = (unsigned long) q;
if(p < 1.) return(qq);
if(q < 1.) return(pp);
while(qq != ZERO) {
	qmem = qq;
	qq = pp % qq;
	pp = qmem;
	}
return(pp);
}


unsigned long LCMofTable(unsigned long p[],int imax,int *p_overflow)
{
int i;
double x,xold;
unsigned long result;

x = 1.;
*p_overflow = FALSE;
for(i=0; i < imax; i++) {
	xold = x;
	x = LCM(x,(double)p[i],p_overflow);
	if(x < 1. || x >= ULONG_MAX || (*p_overflow)) {
		x = xold;
		goto OUT;
		}
	}
OUT:
result = (unsigned long) x;
return(result);
}


long MyInt(double x)
{
long i;

i = (long) (1.00001 * x);
return(i);
}


double Myatof(char *s,long *p_p,long *p_q)
{
int i,j,dot=FALSE,sign;
char c;
long p,q,maxp;
double x;

sign = 1; i = 0;
j = strlen(s);
p = *p_p = ZERO; q = *p_q = 1L;
maxp = Infpos;

while(MySpace(s[i])) i++;
if(s[i] == '-') {
	sign = -1; i++;
	}
if(s[i] == '+') {
	if(sign == -1) {
		q = ZERO; goto END;
		}
	i++;
	}
for(i=i; i < j; i++) {
	c = s[i];
	if(isdigit(c)) {
		p = 10L * p + (long)(c - '0');
		if(dot) q = 10L * q;
		if(p > maxp || p < ZERO) {
			if(!dot) q = ZERO;	/* Overflow */
			sprintf(Message,"Number '%s' creates overflow. Max value is %ld...",
				s,(long)Infpos);
			Println(wTrace,Message);
			goto END;
			}
		continue;
		}
	if(c == '.' || c == ',') {
		if(dot) {
			sprintf(Message,"Two decimal points or commas. Can't accept '%s' as a number...",
				s);
			Println(wTrace,Message);
			q = ZERO;	/* Error in string */
			break;
			}
		else dot = TRUE;
		continue;
		}
	q = ZERO;	/* Error in string */
	break;
	}

END:
if(q > ZERO) {
	x = sign * (((double) p) / q);
	*p_p = sign * p; *p_q = q;
	}
else {
	x = (double) Infneg - 1;	/* Indicates error */
	*p_p = ZERO; *p_q = 1L;
	}
return(x);
}


FloatToNiceRatio(char *s,unsigned long *p_p,unsigned long *p_q)
{
double x,pp,qq;
long p,q;

if((x = Myatof(s,&p,&q)) < Infneg) return(FAILED);
*p_p = (unsigned long) p;
*p_q = (unsigned long) q;
if(x < 0.0001) {
	*p_p = ZERO; *p_q = 1L;
	return(OK);
	}
pp = (double) *p_p;
qq = (double) *p_q;
Simplify((double)ULONG_MAX,pp,qq,&pp,&qq);
(*p_p) = (unsigned long) pp;
(*p_q) = (unsigned long) qq;
return(OK);
}


MakeRatio(double limit,double x,double *p_p,double *p_q)
{
int sign,rounded,result;
double p,q,qq,epsilon,dif,xx,xxx;

result = OK;

if(x >= 0.) sign = 1;
else sign = -1;

if(limit >= 100.) epsilon = 100. / limit;
else epsilon = 1.;

xx = fabs(x);
if(xx < (epsilon / 10.)) {
	(*p_p) = 0.;
	(*p_q) = 1.;
	return(result);
	}

// mantissa = frexp(xx,&exponent);

q = 1.;
while(xx > 10.) {
	xx = 0.1 * xx;
	q = 0.1 * q;
	}
while(xx < 1.) {
	xx = 10. * xx;
	q = 10. * q;
	}

while(TRUE) {
	dif = fabs(xx - Round(xx));
	rounded = FALSE;
	if(dif != 0. && dif < epsilon) {
		xx = Round(xx);
		rounded = TRUE;
		}
	xxx = 10. * xx;
	qq = 10. * q;
	if(xxx >= limit || qq >= limit) break;
	xx = xxx;
	q = qq;
	}
if(!rounded) p = Round(xx);
else p = xx;
q = Round(q);

result = Simplify(limit,p,q,&p,&q);
(*p_p) = sign * p;
(*p_q) = q;
if(result != OK) goto ERR;
if(q < 1.) result = FAILED;
if(result == OK) return(result);

ERR:
if(Beta) Println(wTrace,"\nCalculation overflow...");
TellComplex();
return(result);
}


double Round(double x)
{
double fract,ip;

fract = modf(x,&ip);
if(x >= 0.) {
	if(fract < 0.5) return(ip);
	else return(ip + 1.);
	}
else {
	if(fract > -0.5) return(ip);
	else return(ip - 1.);
	}

/*
x1 = floor(x);
x2 = ceil(x);
if((x - x1) > (x2 - x)) return(x2);
else return(x1); */
}