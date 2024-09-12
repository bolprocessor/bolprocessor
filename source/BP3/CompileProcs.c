/* CompileProcs.c (BP3) */

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


int SkipRem(char **p) 	/* Skip comments between[] or C programming style */
{
char c,*ptr;
int level;

START:
c = NextChar(p);
if(c == '\0') return(OK);
level = 1; ptr = *p;
if(c == '[' || (c == '/' && ptr[1] == '*')) {
	if(c == '/') ptr++;
	while(TRUE) {
		if(CheckEnd(c)) return(MISSED);
		ptr++; c = *ptr;
		if(c == '[') level++;
		if(c == '/' && ptr[1] == '*') {
			level++; ptr++;
			}
		if(c == ']') level--;
		if(c == '*' && ptr[1] == '/') {
			level--; ptr++;
			}
		if(level == 0) {
			ptr++; *p = ptr;
			goto START;
			}
		}
	}
*p = ptr;
return(OK);
}


int FindLeftoffset(tokenbyte **p_left,tokenbyte **p_right,int *p_lenc)
{
int moresymbols;
tokenbyte m,p;
long i,j;

i = *p_lenc = 0;
if((*p_right)[i] == TEND && (*p_right)[i+1] == TEND) return(0);
for(i=*p_lenc=0; (*p_left)[i] == T0 && (*p_left)[i+1] == 2
				&& (*p_right)[i] == T0 && (*p_right)[i+1] == 2; (*p_lenc)++, i+=4){};
if((*p_left)[i] == TEND && (*p_left)[i+1] == TEND) return(-1);
for(i = 0; ((m=(*p_left)[i]) == (*p_right)[i])
	&& ((p=(*p_left)[i+1]) == (*p_right)[i+1])
	&& (m != TEND || p != TEND); i+=2){};

moresymbols = 0;
for(j=i;(*p_left)[j] != TEND || (*p_left)[j+1] != TEND; j+=4) {
	if((*p_right)[j] == TEND && (*p_right)[j+1] == TEND) break;
	if((*p_left)[j] == T0 && (*p_left)[j+1] == 2) {	/* Skip neg context */
		continue;
		}
	moresymbols = TRUE;
	break;
	}
if(!moresymbols) {
	return(0);
	}
else return(i - 4 * (*p_lenc));
}


int FindRightoffset(tokenbyte **p_left, tokenbyte **p_right, int *p_lenarg) {
	int i,ii,j,k,imax,jmax,lerc,moresymbols;
	i = 0;
	if((*p_right)[i] == TEND && (*p_right)[i+1] == TEND) return(0);
	for(i=0; (*p_left)[i] != TEND || (*p_left)[i+1] != TEND; i+=2){};
	*p_lenarg = i;
	imax = i - 2;
	for(j=0; (*p_right)[j] != TEND || (*p_right)[j+1] != TEND; j+=2){};
	jmax = j - 2;
	if(imax < 2 || jmax < 2) return(0); // Added 2024-07-03
	// BPPrintMessage(0,odInfo,"imax = %d jmax = %d\n",imax,jmax);
/*	for(i=(imax-2), lerc=0; (*p_left)[i] == T0 && (*p_left)[i+1] == 2
			&& (*p_right)[i] == T0 && (*p_right)[i+1] == 2; lerc++, i-=4){}; */
	for(i=(imax-2), j=(jmax-2), lerc=0; (*p_left)[i] == T0 && (*p_left)[i+1] == 2
			&& (*p_right)[j] == T0 && (*p_right)[j+1] == 2; lerc++, i-=2, j-=2){
	//	BPPrintMessage(0,odInfo,"[%d] %d %d = %d %d\n",i,(*p_left)[i],(*p_left)[i+1],(*p_right)[j],(*p_right)[j+1]);
			} // Fixed 2024-07-03
	for(i=imax,j=jmax,k=0;
		i >= 0 && j >= 0 && ((*p_left)[i] == (*p_right)[j])
			&& ((*p_left)[i+1] == (*p_right)[j+1]); k+=2, i-=2, j-=2){};
	moresymbols = 0;
	for(ii=i; ii >= 0; ii-=4, j-=4) {
		if(ii > 1 && (*p_left)[ii-2] == T0 && (*p_left)[ii-1] == 2) {
			/* Skip neg context */
			continue;
			}
		if(j >= 0) {
			moresymbols = TRUE;
			break;
			}
		}
	if(moresymbols) {
		return(k - 2 * lerc);
		}
	else return(2 * lerc);
	}


int CreateFlag(char **p_x) {
	int i,diff;
	char **ptr;
	Handle h;

	diff = TRUE;
	if(Jflag > 0) {
		for(i=1; i<= Jflag; i++)
			if((diff = MyHandlecmp((*p_Flagname)[i],p_x)) == 0) break;
		}
	if(diff) {
		if((++Jflag) >= MaxFlag) {
			h = (Handle) p_Flagname;
			if((h = IncreaseSpace(h)) == NULL) return(ABORT);
			p_Flagname = (char****) h;
			h = (Handle) p_Flag;
			if((h = IncreaseSpace(h)) == NULL) return(ABORT);
			p_Flag = (long**) h;
			MaxFlag = (MyGetHandleSize((Handle)p_Flagname) / sizeof(char**));
			for(i=Jflag; i < MaxFlag; i++) (*p_Flagname)[i] = NULL;
			}
		if((ptr = (char**) GiveSpace((Size)(MyHandleLen(p_x)+1))) == NULL) return(0);
		(*p_Flagname)[Jflag] = ptr;
		MystrcpyHandleToHandle(0,&((*p_Flagname)[Jflag]),p_x);
		i = Jflag;
		}
	return(i);
	}

int CreateEventScript(char *x,int quick) {
	int i,j,check,diff;
	char **ptr;
	long dummy;
	Handle h;

	if(p_Script == NULL) {
		if(Beta) Alert1("=> Err. Create EventScript()");
		return(ABORT);
		}
	diff = TRUE;
	if(TraceMIDIinteraction) BPPrintMessage(0,odInfo,"Creating script instruction during compilation from “%s”\n",x);
	if(!quick && Jscriptline > 0) {
	/* quick is TRUE in the second argument of glossary instructions */
		for(i=0; i < Jscriptline; i++) {
			if((diff = Mystrcmp((*p_Script)[i],x)) == 0) {
			//	diff = FALSE;
				BPPrintMessage(0,odInfo,"This script instruction already exists: “%s”\n",x);
				break;
				}
			}
		}
	if(diff) {
		if(++Jscriptline >= MaxScript) {
			h = (Handle) p_Script;
			if((h = IncreaseSpace(h)) == NULL) return(ABORT);
			p_Script = (char****) h;
			MaxScript = (MyGetHandleSize((Handle)p_Script) / sizeof(char**));
			for(i = Jscriptline; i < MaxScript; i++) (*p_Script)[i] = NULL;
			}
		if((*p_Script)[Jscriptline] != NULL) {
		//	BPPrintMessage(0,odError,"=> Err. CreateEventScript(). (*p_Script)[Jscriptline] != NULL\n");
			BPPrintMessage(0,odError,"=> Error creating a script\n");
			}
		if((ptr = (char**) GiveSpace((Size)(utf8_strsize(x)+1))) == NULL) return(ABORT);
		(*p_Script)[Jscriptline] = ptr;
		MystrcpyStringToTable(p_Script,Jscriptline,x);
		i = Jscriptline;
		check = quick; // If equal to zero it will create a script entry
		if(ExecScriptLine(NULL,-1,check,FALSE,ptr,dummy,&dummy,&j,&j) != OK) {
			my_sprintf(Message,"This script instruction is not valid: “%s”\n",x);
			Print(wTrace,Message);
			BPPrintMessage(0,odError,Message);
			Jscriptline--;
			h = (Handle) (*p_Script)[Jscriptline];
			MyDisposeHandle(&h);
			(*p_Script)[Jscriptline] = NULL;
			return(ABORT);
			}
		else {
			if(TraceMIDIinteraction) BPPrintMessage(0,odInfo,"Created script instruction: [%d] “%s”\n",Jscriptline,x);
			}
		}
	return(Jscriptline);
	}


long GetArgument(int mode,char **pp,int *p_inc,long *p_initparam,int *p_foundk,
	double *p_x,long *p_u,long *p_v)
/* mode = 1 : rule weight (possibly Kx) */
/* mode = 2 : negative or positive (possibly Kx=y) */
/* mode = 3 : two integers */
/* mode = 4 : negative or positive integer */
/* mode = 5 : negative or positive integer representing 100 times the value */
/* mode = 6 : negative or positive floating-point value */
/* mode = 7 : negative or positive floating-point value or ratio */
{
int control,sign,dec,isnumber;
long i,n,num,den;
char c,d,s,**qq,*q,line[MAXLIN];
double u,v;

(*p_foundk) = FALSE;
(*p_inc) = 0;
if(mode < 1 || mode > 7) {
	if(Beta) Alert1("=> Err. GetArgument(). 'mode' out of range");
	return(INT_MAX);
	}
q = &d;
qq = &q; 
*qq = *pp;
(*qq)++;
while(MySpace(**qq)) (*qq)++;

if(mode == 6) {
	i = 0; isnumber = TRUE;
	while((c=(**qq)) != ',' && c != '\0' && c != ')') {
		if(MySpace(c)) goto NEXTCHAR;
		line[i++] = c;
		if(!isdigit(c) && c != '.' && c != '+' && c != '-') isnumber = FALSE;
		if(i >= MAXLIN) {
			Print(wTrace,"\nToo long argument in performance control or procedure");
			return(INT_MAX);
			}
NEXTCHAR:
		(*qq)++;
		}
	line[i] = '\0';
	if(isnumber) (*p_x) = Myatof(line,&num,&den);
	else (*p_x) = 0.;
	n = 1;	/* dummy value */
	goto SORTIR;
	}

if(mode == 7) {
	i = 0; isnumber = TRUE;
	while((c=(**qq)) != '\0' && c != ')') {
		if(MySpace(c)) goto NEXTCHAR2;
		line[i++] = c;
		if(!isdigit(c) && c != '.' && c != '+' && c != '-') isnumber = FALSE;
		if(i >= MAXLIN) {
			Print(wTrace,"\nToo long argument in performance control or procedure");
			return(INT_MAX);
			}
NEXTCHAR2:
		(*qq)++;
		}
	line[i] = '\0';
	sign = 1;
	if(isnumber) {
		(*p_x) = Myatof(line,p_u,p_v);
		u = (double) (*p_u);
		v = (double) (*p_v);
		}
	else {
		(*qq) = (*pp);
		(*qq)++;
		n = ZERO;
		while((c=(**qq)) != '/' && c != ')' && c != '\0') {
			if(MySpace(c)) goto NEXTCHAR3;
			if(c == '-') {
				if(sign == -1 || n > ZERO) return(INT_MAX);
				sign = -1;
				goto NEXTCHAR3;
				}
			if(!isdigit(c)) return(INT_MAX);
			n = (10L * n) + c - '0';
NEXTCHAR3:
			(*qq)++;
			}
		u = (double) n;
		(*qq)++;
		n = ZERO;
		while((c=(**qq)) != ')' && c != '\0') {
			if(MySpace(c)) goto NEXTCHAR4;
			if(!isdigit(c)) return(INT_MAX);
			n = (10L * n) + c - '0';
NEXTCHAR4:
			(*qq)++;
			}
		if(c != ')') return(INT_MAX);
		if(n <= ZERO) return(INT_MAX);
		v = (double) n;
		}
	Simplify((double)INT_MAX,u,v,&u,&v);
	(*p_u) = (long) (u * sign);
	(*p_v) = (long) v;
	n = 1;	/* dummy value */
	goto SORTIR;
	}

control = 0;
if((**qq) == 'K' && mode != 3 && mode != 4) {
	(*p_foundk) = TRUE;
	if(mode == 5) {
		Println(wTrace,"\n'_transpose(Kx)' is no longer a valid syntax...");
		}
	control = 1;
	*p_initparam = INT_MAX;
	(*qq)++;
	}
sign = 1;
if(mode == 2 || mode == 4 || mode == 5) {
	if((**qq) == '+') (*qq)++;
	else if((**qq) == '-') {
		(*qq)++; sign = -1;
		}
	while(MySpace(**qq)) (*qq)++;
	}
	
READ:
dec = -1;
for(n=ZERO; (c=(**qq)) != ')' && c != '>' && c != '-' && c != '+'
		&& c != '=' && c != ',' && !MySpace(c); (*qq)++) {
	if(!control && (mode == 1) && (c == -80)) {	/* -80 is '�' */
		n = INT_MIN;		/* Infinite weight */
		(*qq)++;
		while(MySpace(**qq)) (*qq)++;
		if((**qq) != '>') return(INT_MAX);
		goto SORTIR;
		}
	if(c == '.' && mode == 5) {
		dec = 0; continue;
		}
	if(!isdigit(c)) return(INT_MAX);
	n = (10L * n) + c - '0';
	if(dec > -1) dec++;
	}
// if(trace_scale) BPPrintMessage(0,odInfo,"GetArgument() n = %d\n",n);
if(control) n = - n - 1;
while(MySpace(**qq)) (*qq)++;
if((mode == 2 || mode == 3) && (**qq) == ')') {
	n = sign * n;
	goto SORTIR;
	}
if((mode == 4 || mode == 5) && (**qq) == ')') {
	*p_initparam = (long) sign * n;
	if(mode == 5) (*p_inc) = dec;
	goto SORTIR;
	}
if(mode == 3 && (**qq) == ',') {
	(*qq)++;
	(*p_inc) = sign * n;	/* In general, p_inc has another meaning here. */
	goto READ;
	}
if(mode == 1 && ((**qq) == '>')) goto SORTIR;

s = **qq;
if(s == '=') {
	(*qq)++;
	if(!control) return(INT_MAX);
	while(MySpace(**qq)) (*qq)++;
	sign = 1;
	for(i=ZERO; (((mode == 2 && (**qq) != ')')) || ((mode == 1 && (**qq) != '>' && (**qq) != '+'
			&& (**qq) != '-') && !MySpace(**qq))); (*qq)++) {
		c = **qq;
		if(c == '+') {
			if(sign == -1) return(INT_MAX); 
			else continue;
			}
		if(c == '-') {
			if(sign == -1) return(INT_MAX); 
			else {
				sign = -1;
				continue;
				}
			}
		if(!isdigit(c)) return(INT_MAX);
		i = (10L * i) + c - '0';
		}
	*p_initparam = (long) (i * sign);
	while(MySpace(**qq)) (*qq)++; s = **qq;
	if(mode == 2) {
		if((**qq) == ')') goto SORTIR;
		else return(INT_MAX);
		}
	if(mode == 1 && (**qq) == '>') goto SORTIR;
	(*qq)++;
	}
	
while(MySpace(**qq)) (*qq)++;

s = **qq;
if(s == '>') goto SORTIR;
if(s != '+' && s != '-') return(INT_MAX);
(*qq)++;
while(MySpace(**qq)) (*qq)++;
for(i=0; (**qq) != '>' && !MySpace(**qq); (*qq)++) {
	c = **qq;
	if(!isdigit(c)) return(INT_MAX);
	i = (10 * i) + c - '0';
	}
if(s == '-') i = -i;
(*p_inc) = i;
while((**qq) != '>') {
	if(!MySpace(**qq)) return(INT_MAX);
	(*qq)++;
	}

SORTIR:
(*qq)++;
*pp = *qq;
return(n);
}


int GetNilString(char **pp)
{
int i,d;
char *p;
for(d=0; d < MAXNIL && utf8_strsize(NilString[d]) > 0; d++) {
	p = *pp; i = 0;
	while(i < utf8_strsize(NilString[d]) && ((*p) == NilString[d][i])) {
		i++; p++;
		}
	if(i == utf8_strsize(NilString[d]) && (MySpace(*p) || (*p) == '\0')) {
		*pp = p;
		NotBPCase[0] = TRUE;
		return(OK);
		}
	}
return(MISSED);
}


int GetMode(char **pp, int type)
{
int i,j,mode;
char c,d,*p;

mode = 0;  /* default 'RND' */
for(j=0; j < MAXMODE && strlen(Mode[j]) > 0; j++) {
	p = *pp; i = 0;
	while(i < strlen(Mode[j])
		&& (((c=(*p)) == (d=Mode[j][i]))
		|| (UpperCase(c) == d))) {
		i++; p++;
		}
	if(i == strlen(Mode[j])) {
		*pp = p;
		mode = j;
		break;
		}
	}
if(type == ORDtype) {
	if(mode == j) return(mode);
	else return(1);	/* default 'LEFT' */
	}
if(type == LINtype || type == SUB1type || type == POSLONGtype) {
	if(mode == j && mode != 1) {
		my_sprintf(Message,"\n'%s' ignored in this rule.  Forced to 'LEFT' because of subgrammar type.",
			Mode[mode]);
		Print(wTrace,Message);
		return(-1);
		}
	mode = 1;
	}
return(mode);
}


int SkipGramProc(char **pp)
{
int i,j,found;
long y;
char *p;
double x;

found = FALSE;
if(NextChar(pp) != '_') return(MISSED);
while((j=GetProcedure(-1,pp,0,&i,&i,&x,&y)) != RESUME) {
	if(j >= 0) found = TRUE;
	if(j == ABORT) return(j);
	}
p = *pp;
while(MySpace(*p)) p++;
*pp = p;
if(found && (*p == '\0' || *p == '[')) return(OK);
return(MISSED);
}


int GetProcedure(int igram,char **pp,int arg_nr,int *p_igram,int *p_irul,double *p_x,
	long *p_y) {
/* arg_nr = 0 : searching for procedure on top of a subgrammar */

int i,im,j,jproc,k,incweight,length,foundk;
long initparam,u,v;
char c,d,*p,*q,*ptr;

if(arg_nr > 2) {
	Print(wTrace,"\nCan't accept rule procedure in glossary.");
	return(ABORT);
	}
if((**pp) == '\0') return(RESUME);
jproc = -1; im = 0;
*p_igram = *p_irul = 0;
for(j=0; j < MaxProc; j++) {
	p = *pp; i = 0; length = MyHandleLen((*p_GramProcedure)[j]);
	while(MySpace(*p)) p++;
	while(i < length && (((c=(*p)) == (d=(*((*p_GramProcedure)[j]))[i]))
				|| (d == UpperCase(c)) || (c == UpperCase(d)))) {
		i++; p++;
		}
	if(i == length && i > im) {
		jproc = j; im = i; q = p;
		}
	}
if(jproc == -1) return(RESUME);
p = q;

switch(jproc) {
	case 0:	/* _goto */
	case 1:	/* _failed */
	case 2:	/* _repeat */
		NotBPCase[7] = TRUE;
		/* no break */
	case 16:	/* _srand */
		if(jproc != 16 && arg_nr != 2) {
			my_sprintf(Message,"\n'%s' may only occur be in the right argument or a rule.",
				*((*p_GramProcedure)[jproc]));
			Print(wTrace,Message);
			return(ABORT);
			}
		if((*p) != '(') {
			Expect('(',*((*p_GramProcedure)[jproc]),*p);
			return(ABORT);
			}
		if(jproc == 16) {
			if(arg_nr != 0) return(jproc);	/* This won't be used. _srand() actually belongs to the left argument */
			if((k=GetArgument(4,&p,&incweight,&initparam,&foundk,p_x,&u,&v)) == INT_MAX) {
				Print(wTrace,"\nIncorrect or missing argument after '_srand'");
				return(ABORT);
				}
			if(initparam < 0 || initparam > 32767) {
				my_sprintf(Message,"\nIncorrect argument in '_srand(%ld)'. Range is 0..32767",
					(long)initparam);
				Print(wTrace,Message);
				return(ABORT);
				}
			(*p_y) = initparam;
			*pp = p;
			return(jproc);
			break;
			}
		if(jproc == 2) {	/* _repeat */
			if((k=GetArgument(2,&p,&incweight,&initparam,&foundk,p_x,&u,&v)) == INT_MAX) {
				Print(wTrace,"\nIncorrect or missing argument after '_repeat'");
				return(ABORT);
				}
			else {
				if(foundk) {
					i = - k - 1;
					if(i < 1 || i >= MAXPARAMCTRL) {
						my_sprintf(Message,"\n'<K%ld>' not accepted. Range [1,%ld]\n",(long)i,(long)MAXPARAMCTRL-1);
						Print(wTrace,Message);
						return(ABORT);
						}
					*p_irul = i;
					if(initparam == INT_MAX) {
						k = ParamInit[i];
						}
					else {	/* Found '<Kx=y>' */
						k = 1;
						if(ParamInit[i] == INT_MAX) ParamInit[i] = ParamValue[i] = initparam;
						else {
							if(ParamInit[i] != initparam) {
								my_sprintf(Message,
									"\nInitial value of '<K%ld>' already set to %ld...",
										(long)i,(long)ParamInit[i]);
								Print(wTrace,Message);
								return(ABORT);
								}
							}
						}
					}
				}
			}
		else {
			p++; k = 0;
			while((c=(*p)) != ',') {
				c -= '0';
				if(c >= 0 && c <= 9) k = 10 * k + c;
				else {
					Print(wTrace,"\nInvalid grammar index after '_goto' or '_failed'");
					return(ABORT);
					}
				p++;
				}
			}
		*p_igram = k; p++;
		if(jproc == 2) {	/* _repeat */
			*pp = p; return(jproc);
			}
		k = 0;
		while((c=(*p)) != ')') {
			c -= '0';
			if(c >= 0 && c <= 9) k = 10 * k + c;
			else {
				Print(wTrace,"\nInvalid rule index after '_goto' or '_failed'");
				return(ABORT);
				}
			p++;
			}
		*p_irul = k; p++;
		*pp = p;
		return(jproc);
		break;
	case 11:	/* _destru */
		NotBPCase[7] = TRUE;
		if(arg_nr == 1) {
			my_sprintf(Message,"\n'%s' should be on top of a subgrammar or in the right argument of a rule.",
				*((*p_GramProcedure)[jproc]));
			Print(wTrace,Message);
			return(ABORT);
			}
	case 12:	/* _randomize */
		if(arg_nr > 0) return(RESUME);
	case 3:		/* _stop */
	case 4:		/* _print */
	case 5:		/* _printOn */
	case 6:		/* _printOff */
	case 7:		/* _stepOn */
	case 8:		/* _stepOff */
	case 9:		/* _traceOn */
	case 10:	/* _traceOff */
	case 14:	/* _striated */
	case 15:	/* _smooth */
		break;
	case 13:	/* _mm */
		if((igram > 1 || arg_nr > 0) && (jproc == 14 || jproc == 15 || jproc == 13)) {
			my_sprintf(Message,"\n'%s' should always be on top of the first subgrammar.",
				*((*p_GramProcedure)[jproc]));
			Println(wTrace,Message);
			}
		if((*p) != '(') {
			Expect('(',*((*p_GramProcedure)[jproc]),*p);
			return(ABORT);
			}
		if((k=GetArgument(6,&p,&incweight,&initparam,&foundk,p_x,&u,&v)) == INT_MAX) {
			Print(wTrace,"\nIncorrect or missing argument after '_mm'");
			return(ABORT);
			}
		p--;
		if((*p) != ')') {
			Expect(')',*((*p_GramProcedure)[jproc]),*p);
			return(ABORT);
			}
		p++;
		break;
	}
*pp = p;
return(jproc);
}


int GetPerformanceControl(char **pp,int arg_nr,int *p_n,int quick,long *p_u,long *p_v,
	KeyNumberMap *p_map) {
	int i,im,j,jinstr,p,length,chan,foundk,cntl,result,i_scale,j_scale,
	key,this_key,note_class,numnotes,l,numgrades,basekey,notenum, octave,pitchclass;
	long k,initparam;
	char c,d,*ptr,*ptr2,*p_line,*q,line[MAXLIN];
	double x;

	jinstr = -1; im = 0;
	if(trace_scale)
		BPPrintMessage(0,odInfo,"Start GetPerformanceControl()\n");
	for(j=0; j < MaxPerformanceControl; j++) {
	//	BPPrintMessage(0,odInfo,"%s\n",(*(*p_PerformanceControl)[j]));
		ptr = *pp; i = 0; length = MyHandleLen((*p_PerformanceControl)[j]);
		while(MySpace(*ptr)) ptr++;
		while(i < length && (((c=(*ptr)) == (d=(*((*p_PerformanceControl)[j]))[i]))
				|| (d == UpperCase(c)) || (c == UpperCase(d)))) {
			i++; ptr++;
			}
		if(i == length && i > im) {
			jinstr = j; im = i; q = ptr;
			}
		}
	if(jinstr == -1) return(RESUME);
	// BPPrintMessage(0,odInfo,"GetPerformanceControl() j = %d MaxPerformanceControl = %d\n",jinstr,MaxPerformanceControl);
	if(arg_nr == 1 || arg_nr == 4) {
		my_sprintf(Message,"\n'%s' should not appear in the left argument of a rule",
			*((*p_PerformanceControl)[jinstr]));
		Print(wTrace,Message);
		return(ABORT);
		}
	ptr = q;
	switch(jinstr) {
		case 0:	/* _chan() */
		case 1:	/* _vel() */
		case 5:	/* _mod() */
		case 11: /* _press() */
		case 16: /* _volume() */
		case 19: /* _legato() */
		case 20: /* _staccato() */
		case 29: /* _pitchrange() */
		case 30: /* _pitchrate() */
		case 31: /* _modrate() */
		case 32: /* _pressrate() */
		case 34: /* _volumerate() */
		case 35: /* _volumecontrol() */
		case 36: /* _pan() */
		case 40: /* _panrate() */
		case 41: /* _pancontrol() */
		case 56: /* _rndvel() */
		case 57: /* _rotate() */
		case 59: /* _rndtime() */
		case 60: /* _srand() */
		case 66: /* _capture() */
			goto GETARG;
			break;
		case 8:	/* _pitchbend() */
			k = 4;
			goto GETARGN;
			break;
		case 33: /* _transpose() */
			k = 5;
			goto GETARGN;
			break;
		case 2:	/* _velstep */
		case 3:	/* _velcont */
		case 6:	/* _modstep */
		case 7:	/* _modcont */
		case 9:	/* _pitchstep */
		case 10: /* _pitchcont */
		case 12: /* _presstep */
		case 13: /* _presscont */
		case 17: /* _volumestep */
		case 18: /* _volumecont */
		case 21: /* _articulstep */
		case 22: /* _articulcont */
		case 23: /* _velfixed */
		case 24: /* _modfixed */
		case 25: /* _pitchfixed */
		case 26: /* _pressfixed */
		case 27: /* _volumefixed */
		case 28: /* _articulfixed */
		case 37: /* _panstep */
		case 38: /* _pancont */
		case 39: /* _panfixed */
		case 42: /* _rest */
		case 48: /* _retro */
		case 49: /* _rndseq */
		case 50: /* _randomize */
		case 51: /* _ordseq */
		case 53: /* _mapfixed */
		case 54: /* _mapcont */
		case 55: /* _mapstep */
		case 62: /* _transposefixed */
		case 63: /* _transposecont */
		case 64: /* _transposestep */
			*pp = ptr;
			if((*ptr) == '(') {
				my_sprintf(Message,"\nShould not find '(' after '%s'",*((*p_PerformanceControl)[jinstr]));
				Print(wTrace,Message);
				return(ABORT);
				}
			return(jinstr);
			break;
		case 4:	/* _script() */
		case 43: /* _ins() */
		case 45: /* _step() */
		case 46: /* _cont() */
		case 47: /* _fixed() */
			goto GETSTRING;
			break;
		case 14: /* _switchon() */
		case 15: /* _switchoff() */
			goto GET2ARGS;
			break;
		case 44: /* _value() */
		case 58: /* _keyxpand() */
		case 65: /* _scale() */
			goto GET2CONSTANTS;
			break;
		case 52: /* _keymap() */
			goto GET4ARGS;
			break;
		case 61: /* _tempo() */
			goto GETRATIO;
			break;
		default:
			my_sprintf(Message,"=> Err. GetPerformanceControl(). Unknown instruction");
			if(Beta) Alert1(Message);
			return(ABORT);
		}

	GETRATIO:
	if((*ptr) != '(') {
		Expect('(',*((*p_PerformanceControl)[jinstr]),*ptr);
		return(ABORT);
		}
	if((k=GetArgument(7,&ptr,&p,&initparam,&foundk,&x,p_u,p_v)) == INT_MAX) {
		my_sprintf(Message,"\nIncorrect or missing argument after '%s'",
			*((*p_PerformanceControl)[jinstr]));
		Print(wTrace,Message);
		return(ABORT);
		}
	else {
		if((*p_u) <= ZERO){
			my_sprintf(Message,"\nArgument in '%s' must be strictly positive",
				*((*p_PerformanceControl)[jinstr]));
			Print(wTrace,Message);
			return(ABORT);
			}
		}
	*pp = ptr;
	return(jinstr);

	GET4ARGS:
	if((*ptr) != '(') {
		Expect('(',*((*p_PerformanceControl)[jinstr]),*ptr);
		return(ABORT);
		}
	ptr++;
	for(i=0; i < 4; i++) {
		j = 0;
		j = GetInteger(YES,ptr,&j);
		if(j == INT_MAX) {
			if(GetNote(ptr,&j,&chan,YES) != OK) {
				Print(wTrace,"\nIncorrect argument in _keymap(). Key numbers or note names expected...");
				return(ABORT);
				}
			}
		else {
			if(j < 0 || j > 127) {
				my_sprintf(Message,"\nIncorrect argument in _keymap(). Key number must be in range 0..127. Can't accept %ld...",
					(long) j);
				Print(wTrace,Message);
				return(ABORT);
				}
			}
		switch(i) {
			case 0: p_map->p1 = j; break;
			case 1: p_map->q1 = j; break;
			case 2: p_map->p2 = j; break;
			case 3: p_map->q2 = j; break;
			}
		while((c=(*ptr)) != ',' && c != ')' && c != '\0') ptr++;
		if(c == '\0') {
			Print(wTrace,"\nIncorrect separator between arguments in _keymap()...");
			return(ABORT);
			}
		if(i == 3 && c != ')') {
			Expect(')',*((*p_PerformanceControl)[jinstr]),c);
			return(ABORT);
			}
		if(i < 3 && c == ')') {
			Expect(',',*((*p_PerformanceControl)[jinstr]),c);
			return(ABORT);
			}
		ptr++;
		}
	if(p_map->p1 > p_map->p2) {
		my_sprintf(Message,"\nIn _keymap(p1,q1,p2,q2), p2 must be greater than p1. Can't accept _keymap(%ld,%ld,%ld,%ld)",
			(long)p_map->p1,(long)p_map->q1,(long)p_map->p2,(long)p_map->q2);
		Print(wTrace,Message);
		return(ABORT);
		}
	*pp = ptr;
	return(jinstr);

	GET2ARGS:
	if(trace_scale) BPPrintMessage(0,odInfo,"GET2ARGS\n");
	if((*ptr) != '(') {
		Expect('(',*((*p_PerformanceControl)[jinstr]),*ptr);
		return(ABORT);
		}
	if((k=GetArgument(3,&ptr,&p,&initparam,&foundk,&x,p_u,p_v)) == INT_MAX) {
		my_sprintf(Message,"\nIncorrect or missing argument after '%s'",
			*((*p_PerformanceControl)[jinstr]));
		Print(wTrace,Message);
		return(ABORT);
		}
	if(trace_scale) BPPrintMessage(0,odInfo,"k = %d p = %d initparam = %d foundk = %d x = %ld u = %ld v = %ld\n",k,p,initparam,foundk,(long)x,(long)*p_u,(long)*p_v);
	/* if(jinstr == 65) { // _scale()
		if(p < 0 || p > NumberScales) { 
			BPPrintMessage(0,odError,"\nScale number %d is out of range (maximum %d). No Csound score produced\n\n",p,NumberScales);
			return(ABORT);
			}
		if(k < 0 || k > 127) {
			BPPrintMessage(0,odError,"\nKey #%d is out of range [0..127]. No Csound score produced\n\n",k);
			return(ABORT);
			}
		} */
	if(jinstr == 14 || jinstr == 15) {
		if(p < 64 || p > 95) {
			my_sprintf(Message,"\nFirst argument of '%s' is switch number, range 64..95. Can't accept %ld",
				*((*p_PerformanceControl)[jinstr]),(long)p);
			Print(wTrace,Message);
			return(ABORT);
			}
		if(k < 1 || k > MAXCHAN) {
			my_sprintf(Message,"\nSecond argument of '%s' is channel number, range 1..%ld. Can't accept %ld",
				*((*p_PerformanceControl)[jinstr]),(long)MAXCHAN,(long)k);
			Print(wTrace,Message);
			return(ABORT);
			}
		*p_n = 128 * k + p;
		}
	*pp = ptr;
	return(jinstr);

	GETARG:
	if((*ptr) != '(') {
		Expect('(',*((*p_PerformanceControl)[jinstr]),*ptr);
		return(ABORT);
		}
	p = initparam = 0;
	if((k=GetArgument(2,&ptr,&p,&initparam,&foundk,&x,p_u,p_v)) == INT_MAX || p != 0) {
		my_sprintf(Message,"\nIncorrect or missing argument after '%s'",
			*((*p_PerformanceControl)[jinstr]));
		Print(wTrace,Message);
		return(ABORT);
		}
	else {
		if(foundk) {	/* Found '<Kx>' or '<Kx=y>' */
			i = - k - 1;
			switch(jinstr) {
				case 5: /* _mod() */
				case 29: /* _pitchrange() */ 
				case 30: /* _pitchrate() */
				case 31: /* _modrate() */
				case 32: /* _pressrate() */
				case 34: /* _volumerate() */
				case 40: /* _panrate() */
					my_sprintf(Message,
						"\nIncorrect argument in %s. Integer value required (0..16383)",
							*((*p_PerformanceControl)[jinstr]));
					Print(wTrace,Message);
					return(ABORT);
					break;
				case 66: /* _capture() */
					my_sprintf(Message,
						"\nIncorrect argument in %s. Integer value required (0..128)",
							*((*p_PerformanceControl)[jinstr]));
					Print(wTrace,Message);
					return(ABORT);
					break;
				case 59: /* _rndtime() */ 
				case 60: /* _srand() */
					my_sprintf(Message,
						"\nIncorrect argument in %s. Integer value required (0..32767)",
							*((*p_PerformanceControl)[jinstr]));
					Print(wTrace,Message);
					return(ABORT);
				}
			if(i < 1 || i >= MAXPARAMCTRL) {
				my_sprintf(Message,"\n<K%ld> not accepted.  Range [1,%ld]",(long)i,
					(long)MAXPARAMCTRL-1);
				Print(wTrace,Message);
				return(ABORT);
				}
			k = 128 + i;
			if(initparam != INT_MAX) {	/* Found '<Kx=y>' */
				if(jinstr == 0 && (initparam < 1 || initparam > MAXCHAN)) {
					my_sprintf(Message,
						"\nMIDI channel range is 1..%ld. Can't accept '_chan(K%ld=%ld)'",
							(long)MAXCHAN,(long)i,(long)initparam);
					Print(wTrace,Message);
					return(ABORT);
					}
				if(jinstr == 1 && (initparam < 0 || initparam > 127)) {
					my_sprintf(Message,
						"\nVelocity range is 0..127. Can't accept '_vel(K%ld=%ld)'",
							(long)i,(long)initparam);
					Print(wTrace,Message);
					return(ABORT);
					}
				if(jinstr == 56 && (initparam < 0 || initparam > 127)) {
					my_sprintf(Message,
						"\nRandom assignment to velocity range is 0..127. Can't accept '_rndvel(K%ld=%ld)'",
							(long)i,(long)initparam);
					Print(wTrace,Message);
					return(ABORT);
					}
				if(jinstr == 57 && (initparam < -128 || initparam > 127)) {
					my_sprintf(Message,
						"\nRotation range is -128..127. Can't accept '_rotate(K%ld=%ld)'",
							(long)i,(long)initparam);
					Print(wTrace,Message);
					return(ABORT);
					}
				if(jinstr == 16 && (initparam < 0 || initparam > 127)) {
					my_sprintf(Message,
						"\nVolume range is 0..127. Can't accept '_volume(K%ld=%ld)'",
							(long)i,(long)initparam);
					Print(wTrace,Message);
					return(ABORT);
					}
				if(jinstr == 36 && (initparam < 0 || initparam > 127)) {
					my_sprintf(Message,
						"\nPanoramic range is 0..127. Can't accept '_pan(K%ld=%ld)'",
							(long)i,(long)initparam);
					Print(wTrace,Message);
					return(ABORT);
					}
				if(jinstr == 19 && initparam < 0) {
					my_sprintf(Message,
						"\nLegato must be positive. Can't accept '_legato(K%ld=%ld)'",
							(long)i,(long)initparam);
					Print(wTrace,Message);
					return(ABORT);
					}
				if(jinstr == 20 && (initparam < 0 || initparam > 100)) {
					my_sprintf(Message,
						"\nStaccato range is 0..100. Can't accept '_staccato(K%ld=%ld)'",
							(long)i,(long)initparam);
					Print(wTrace,Message);
					return(ABORT);
					}
				if(jinstr == 43 && initparam < 1) {
					my_sprintf(Message,
						"\nCsound instrument index must be strictly positive. Can't accept '_ins(K%ld=%ld)'",
							(long)i,(long)initparam);
					Print(wTrace,Message);
					return(ABORT);
					}
				if(arg_nr == 0)	/* Playing selection */
					ParamValue[i] = initparam;
				else {	/* Compiling grammar */
					if(ParamInit[i] == INT_MAX) ParamInit[i] = ParamValue[i] = initparam;
					else {
						if(ParamInit[i] != initparam) {
							my_sprintf(Message,
								"\nInitial value of '<K%ld>' already set to %ld...",
									(long)i,(long)ParamInit[i]);
							Print(wTrace,Message);
							return(ABORT);
							}
						}
					}
				}
			}
		else {
			strcpy(Message,""); // Fixed by BB 2021-02-15
			if(jinstr == 0 && (k < 1 || k > MAXCHAN)) {
				my_sprintf(Message,
					"\nMIDI channel range is 1..%ld. Can't accept '_chan(%ld)'",
						(long)MAXCHAN,(long)k);
				}
			if(jinstr == 56 && (k < 0 || k > 127)) {
				my_sprintf(Message,
					"\nRandom assignment to velocity range is 0..127. Can't accept '_rndvel(%ld)'",
					(long)k);
				}
			if(jinstr == 57 && (k < -128 || k > 127)) {
				my_sprintf(Message,
					"\nRotation range is -128..127. Can't accept '_rotate(%ld)'",
					(long)k);
				}
			if(jinstr == 1 && (k < 0 || k > 127)) {
				my_sprintf(Message,
					"\nVelocity range is 0..127. Can't accept '_vel(%ld)'",(long)k);
				}
			if(jinstr == 5 && (k < 0 || k > 16383)) {
				my_sprintf(Message,
					"\nModulation range is 0..16383. Can't accept '_mod(%ld)'",(long)k);
				}
			if(jinstr == 11 && (k < 0 || k > 127)) {
				my_sprintf(Message,
					"\nPressure range is 0..127. Can't accept '_press(%ld)'",(long)k);
				}
			if(jinstr == 16 && (k < 0 || k > 127)) {
				my_sprintf(Message,
					"\nVolume range is 0..127. Can't accept '_volume(%ld)'",(long)k);
				}
			if(jinstr == 36 && (k < 0 || k > 127)) {
				my_sprintf(Message,
					"\nPanoramic range is 0..127. Can't accept '_pan(%ld)'",(long)k);
				}
			if(jinstr == 19 && k < 0) {
				my_sprintf(Message,
					"\nLegato must be positive. Can't accept '_legato(%ld)'",(long)k);
				}
			if(jinstr == 20 && (k < 0 || k > 100)) {
				my_sprintf(Message,
					"\nStaccato range is 0..100. Can't accept '_staccato(%ld)'",(long)k);
				}
			if(jinstr == 29 && (k < 0 || k > 16383)) {
				my_sprintf(Message,
					"\nPitchbend range is 0..16383 cents. Can't accept '_pitchrange(%ld)'",(long)k);
				}
			if(jinstr == 30 && (k < 0 || k > 1000)) {
				my_sprintf(Message,
					"\nPitchbend rate range is 0..1000 samples/sec. Can't accept '_pitchrate(%ld)'",
						(long)k);
				}
			if(jinstr == 31 && (k < 0 || k > 1000)) {
				my_sprintf(Message,
					"\nModulation rate range is 0..1000 samples/sec. Can't accept '_modrate(%ld)'",
						(long)k);
				}
			if(jinstr == 32 && (k < 0 || k > 1000)) {
				my_sprintf(Message,
					"\nPressure rate range is 0..1000 samples/sec. Can't accept '_pressrate(%ld)'",
						(long)k);
				}
			if(jinstr == 34 && (k < 0 || k > 1000)) {
				my_sprintf(Message,
					"\nVolume rate range is 0..1000 samples/sec. Can't accept '_volumerate(%ld)'",
						(long)k);
				}
			if(jinstr == 40 && (k < 0 || k > 1000)) {
				my_sprintf(Message,
					"\nPanoramic rate range is 0..1000 samples/sec. Can't accept '_panrate(%ld)'",
						(long)k);
				}
			if(jinstr == 35 && (k < 0 || k > 127)) {
				my_sprintf(Message,
					"\nVolume controller is 0..127. Can't accept '_volumecontrol(%ld)'",(long)k);
				}
			if(jinstr == 41 && (k < 0 || k > 127)) {
				my_sprintf(Message,
					"\nPanoramic controller is 0..127. (Generally 10) Can't accept '_pancontrol(%ld)'",
						(long)k);
				}
			if(jinstr == 43 && k < 1) {
				my_sprintf(Message,
					"\nCsound instrument index must be strictly positive. Can't accept '_ins(%ld)'",
						(long)k);
				}
			if(jinstr == 59 && (k < 0 || k > 32767)) {
				my_sprintf(Message,
					"\nRandom timing range is 0..32767 milliseconds. Can't accept '_rndtime(%ld)'",
						(long)k);
				}
			if(jinstr == 66 && (k < 0 || k > 127)) {
				my_sprintf(Message,
					"\nCapture stream is 0..127. Can't accept '_capture(%ld)'",
						(long)k);
				}
			if(jinstr == 60 && (k < 0 || k > 32767)) {
				my_sprintf(Message,
					"\nRandom seed range is 0..32767. Can't accept '_srand(%ld)'",(long)k);
				}
			if(Message[0] != '\0') {
				Print(wTrace,Message);
				return(ABORT);
				}
			}
		}
	*p_n = k;
	*pp = ptr;
	return(jinstr);

	GETARGN:
	if((*ptr) != '(') {
		Expect('(',*((*p_PerformanceControl)[jinstr]),*ptr);
		return(ABORT);
		}
	p = initparam = 0;
	if((k=GetArgument(k,&ptr,&p,&initparam,&foundk,&x,p_u,p_v)) == INT_MAX) {
		my_sprintf(Message,"\nIncorrect or missing argument after '%s'",
			*((*p_PerformanceControl)[jinstr]));
		Print(wTrace,Message);
		return(ABORT);
		}
	else {
		Message[0] = '\0';
		if(jinstr == 8 && (initparam < -16384 || initparam > 16383)) {
			my_sprintf(Message,
				"\nPitchbender range is -16384..+16383. Can't accept '_pitchbend(%ld)'",(long)initparam);
			}
		if(jinstr == 33) {
			x = initparam;
			while((p--) > 0) x = x / 10.;
			initparam = 100 * x;
			if(initparam < -12800 || initparam > 12700) {
				my_sprintf(Message,
					"\nTransposition range is -128..127 semitones. Can't accept '_transpose(%.2f)'",
					(((float)initparam)/100.));
				}
			}
		if(Message[0] != '\0') {
			Print(wTrace,Message);
			return(ABORT);
			}
		}
	*p_n = initparam;
	*pp = ptr;
	return(jinstr);

	GETSTRING:
	if((*ptr) != '(') {
		Expect('(',*((*p_PerformanceControl)[jinstr]),*ptr);
		return(ABORT);
		}
	ptr++; i = 0;
	while((c = *ptr) != ')' && c != '\0') {
		line[i++] = c;
		if(i >= MAXLIN) {
			Print(wTrace,"Too long argument in performance control\n");
			return(ABORT);
			}
		ptr++;
		}
	line[i] = '\0';
	Strip(line);
	if(c != ')') {
		my_sprintf(Message,"Missing ')' after '%s'",*((*p_PerformanceControl)[jinstr]));
		Print(wTrace,Message);
		return(ABORT);
		}
	switch(jinstr) {
		case 4:
	//		BPPrintMessage(0,odInfo,"scriptline = “%s”\n",line);
			quick = TRUE;
			*p_n = CreateEventScript(line,quick);
			if(*p_n < 0) return(*p_n);
			break;
		case 43:
		//	BPPrintMessage(0,odError,"line = %s\n",line);
			*p_n = FindCsoundInstrument(line);
			if(*p_n < 0) return(*p_n);
	//		*p_n -= 1;
			break;
		case 45:
		case 46:
		case 47:
			*p_n = FixStringConstant(line);
			if(*p_n < 0) return(*p_n);
			break;
		}
	ptr++;
	*pp = ptr;
	return(jinstr);

	GET2CONSTANTS:

	if((*ptr) != '(') {
		Expect('(',*((*p_PerformanceControl)[jinstr]),*ptr);
		return(ABORT);
		}
	ptr++; i = 0;
	while((c = *ptr) != ',' && c != '\0') {
		if(c == ')') break;
		line[i++] = c;
		if(i >= MAXLIN) {
			Print(wTrace,"Too long argument in performance control\n");
			return(ABORT);
			}
		ptr++;
		}
	line[i] = '\0';

	switch(jinstr) {
		case 44:	/* _value() */
			if((k=FixStringConstant(line)) < 0) return(k);
			break;
		case 65:	/* _scale() */
			if(trace_scale) BPPrintMessage(0,odInfo,"_scale() found in compilation\n");
			if(!OutCsound && (rtMIDI || WriteMIDIfile)) { 
				if(!MIDImicrotonality) BPPrintMessage(0,odInfo,"👉 Microtonality mimics MIDI Polyphonic Expression\n");
				MIDImicrotonality = TRUE;
				}
			if(strcmp(line,"0") == 0 || (!OutCsound && !rtMIDI && !WriteMIDIfile)) k = 0; 
			else {
				k = FixStringConstant(line);
				if(k < 0) return(k);
				i_scale = FindScale(k);
		/*		for(i_scale = 1; i_scale <= NumberScales; i_scale++) {
					result = MyHandlecmp((*p_StringConstant)[k],(*Scale)[i_scale].label);
					if(result == 0) break;
					} */
				if(i_scale > NumberScales) {
					my_sprintf(Message,"Instruction \"_scale(%s,...)\" is illicit as this name is unknown\n",*((*p_StringConstant)[k]));
					Print(wTrace,Message);
					return(ABORT);
					}
				else {
					numgrades = (*Scale)[i_scale].numgrades;
					numnotes = (*Scale)[i_scale].numnotes;
					basekey = (*Scale)[i_scale].basekey;
					int baseoctave = (*Scale)[i_scale].baseoctave;
					strcpy(LastSeen_scale,line);
					if(TraceMicrotonality)
						BPPrintMessage(0,odInfo,"Recording the note names of scale %d '%s' with %d grades and base key = %d\n",i_scale,line,numgrades,basekey);
					j_scale = i_scale + 3;
					for(key = 0; key < MAXKEY; key++) {
						this_key = modulo(key - basekey, numnotes);
						int keyclass = (*((*Scale)[i_scale].keyclass))[this_key];
						octave = baseoctave + floor((((double)key - basekey)) / numnotes);
						if(trace_scale) 
							BPPrintMessage(0,odInfo,"key = %d, keyclass = %d, octave = %d keyclass = %d\n",key,keyclass,octave,keyclass);
						switch(octave) {
							case -1:
								my_sprintf(Message,"%s00",*((*((*Scale)[i_scale].notenames))[keyclass]));
								break;
							default:
								my_sprintf(Message,"%s%ld",*((*((*Scale)[i_scale].notenames))[keyclass]),(long)octave);
								break;
							}
						if(trace_scale)
							BPPrintMessage(0,odInfo,"i_scale = %d, key = %d, '%s'\n",i_scale,key,Message);
						MystrcpyStringToTable(p_NoteName[j_scale],key,Message);
						MystrcpyStringToTable(p_AltNoteName[j_scale],key,Message);
						(*(p_NoteLength[j_scale]))[key] = (*(p_AltNoteLength[j_scale]))[key] = strlen(Message);
						}
					}
				}
			break;
		case 58:	/* _keyxpand() */
			Strip(line);
			ptr2 = ptr;
			if(line[0] == 'K') {
				for(i=1; i <= strlen(line); i++) line[i-1] = line[i];
				cntl = atol(line); 	/* Don't use atoi() because integers are 4 bytes */
				if(cntl < 1 || cntl >= MAXPARAMCTRL) {
					my_sprintf(Message,"'K%ld' not accepted. Range [1,%ld]",(long)cntl,
						(long)MAXPARAMCTRL-1);
					Print(wTrace,Message);
					return(ABORT);
					}
				k = cntl + 128;
				if((ptr=strstr(line,"=")) != NULLSTR) {
					ptr++;
					initparam = (int) atol(ptr);
					if(arg_nr == 0)	/* Playing selection */
						ParamValue[cntl] = initparam;
					else {	/* Compiling grammar */
						if(ParamInit[cntl] == INT_MAX) ParamInit[cntl] = ParamValue[cntl]
							= initparam;
						else {
							if(ParamInit[cntl] != initparam) {
								my_sprintf(Message,
									"Initial value of '<K%ld>' already set to %ld...",
										(long)cntl,(long)ParamInit[cntl]);
								Print(wTrace,Message);
								return(ABORT);
								}
							}
						}
					}
				}
			else {
				j = 0;
				ptr = line;
				j = GetInteger(YES,ptr,&j);
				if(j == INT_MAX) {
					if(GetNote(ptr,&j,&chan,YES) != OK) {
						Print(wTrace,
							"Incorrect first argument in _keyxpand(). Key number or note name expected...");
						return(ABORT);
						}
					}
				else {
					if(j < 0 || j > 127) {
						my_sprintf(Message,"Incorrect argument in _keyxpand(). Key number must be in range 0..127. Can't accept %ld...",
							(long) j);
						Print(wTrace,Message);
						return(ABORT);
						}
					}
				k = j;
				}
			ptr = ptr2;
			break;
		}

	if(c != ',') {
		switch(jinstr) {
			case 44:	/* _value() */
				Print(wTrace,"Missing ',' in '_value()'\n");
				break;
			case 65:	/* _scale() */
				Print(wTrace,"Missing ',' in '_scale()'\n");
				break;
			case 58:	/* _keyxpand() */
				Print(wTrace,"Missing ',' in '_keyxpand()'\n");
				break;
			}
		return(ABORT);
		}
		
	ptr++; i = 0;
	while((c = *ptr) != ')' && c != '\0') {
		line[i++] = c;
		if(i >= MAXLIN) {
			Print(wTrace,"Too long argument in tool or performance control\n");
			return(ABORT);
			}
		ptr++;
		}
	line[i] = '\0';
	if(c != ')') {
		Print(wTrace,"Missing ')' in '_value()'\n");
		return(ABORT);
		}
	if(trace_scale) BPPrintMessage(0,odInfo,"GET2CONSTANTS line = '%s'\n",line);

	Strip(line);
	if(isalpha(line[0])) {
		// Try to interpret non-numeric value as a note. Used mainly for block key in _scale()
	//	BPPrintMessage(0,odInfo,"Found non-numeric value: %s NoteConvention = %d\n",line,NoteConvention);
		p_line = line;
		if(strlen(LastSeen_scale) > 0) {
			if(trace_scale) BPPrintMessage(0,odInfo,"Looking for block key '%s' with LastSeen_scale = %s NumberScales = %d\n",line,LastSeen_scale,NumberScales);
			for(i_scale = 1; i_scale <= NumberScales; i_scale++) {
				if(trace_scale) BPPrintMessage(0,odInfo,"i_scale = %d (*Scale)[i_scale].label = %s\n",i_scale,*((*Scale)[i_scale].label));
				result = strcmp(LastSeen_scale,*((*Scale)[i_scale].label));
				if(result == 0) break;
				}
			if(i_scale > NumberScales) i_scale = -1;
			else {
				if(trace_scale) BPPrintMessage(0,odInfo,"Found: i_scale = %d\n",i_scale);
				j_scale = i_scale + 3;
				for(key=0; key < 128; key++) {
					l = (*(p_NoteLength[j_scale]))[key];
			//		if(trace_scale) BPPrintMessage(0,odInfo,"key =%d (*(p_NoteName[j_scale]))[key] = %s\n",key,*((*(p_NoteName[j_scale]))[key]));
					if(l > 0 && Match(TRUE,&p_line,(*(p_NoteName[j_scale]))[key],l)
							&& !isdigit(p_line[l])) {
						my_sprintf(line,"%d",key);
						if(trace_scale) BPPrintMessage(0,odInfo,"Found block key '%d' in current scale, j_scale = %d LastSeen_scale = %s\n",key,j_scale,LastSeen_scale);
						goto FOUNDKEY;
						}
					}
				}
			}
		if(NoteConvention <= KEYS) {
			for(key = 0; key < 128; key++) {
				l = (*(p_NoteLength[NoteConvention]))[key];
				if(l > 0 && Match(TRUE,&p_line,(*(p_NoteName[NoteConvention]))[key],l)
				&& !isdigit(line[l])) {
					my_sprintf(line,"%d",key);
					goto FOUNDKEY;
					}
				}
			for(key = 0; key < 128; key++) {
				l = (*(p_NoteLength[NoteConvention]))[key];
				if(l > 0 && Match(TRUE,&p_line,(*(p_AltNoteName[NoteConvention]))[key],l)
				&& !isdigit(line[l])) {
					my_sprintf(line,"%d",key);
					goto FOUNDKEY;
					}
				}
			}
		else {
			for(j_scale = 4; j_scale < MAXCONVENTIONS; j_scale++) {
				for(key=0; key < MAXKEY; key++) {
					l = (*(p_NoteLength[j_scale]))[key];
					if(l > 0 && Match(TRUE,&p_line,(*(p_NoteName[j_scale]))[key],l)
							&& !isdigit(p_line[l])) {
						my_sprintf(line,"%d",key);
						if(trace_scale) BPPrintMessage(0,odInfo,"Found block key '%d' in other scale, j_scale = %d LastSeen_scale = %s\n",key,j_scale,LastSeen_scale);
						goto FOUNDKEY;
						}
					}
				}
			}
		}
	FOUNDKEY:
	if((p=FixNumberConstant(line)) < 0) return(p);

	*p_n = MAXSTRINGCONSTANTS * p + k;
	ptr++;
	*pp = ptr;
	return(jinstr);
	}


	int GetSubgramType(char **pp)
	{
	int i,jtype;
	char c,d,*p;
	for(jtype = 0; jtype < MAXTYPE && strlen(SubgramType[jtype]) > 0; jtype++) {
		p = *pp; i = 0;
		while(MySpace(*p)) p++;
		while(i < strlen(SubgramType[jtype]) &&
			(((c=(*p)) == (d=SubgramType[jtype][i]))
				|| (UpperCase(c) == d))) {
			i++; p++;
			}
		if((i == strlen(SubgramType[jtype]) && ((*p) == '\0' || MySpace(*p) || (*p) == '['))) {
			*pp = p;
			if(jtype == SUBtype || jtype == SUB1type || jtype == POSLONGtype) NotBPCase[2] = TRUE;
			return(jtype);
			}
		}
	return(-1);
	}


int GetArg(char **pp, char **qq1, char **qq2, char **qq3, char **qq4)
// Get pointers delimitating the two arguments of a rule:
// *qq1 is the beginning of the first argument
// *qq2 is the end of the first argument
// *qq3 is the beginning of the second argument
// *qq4 is the end of the second argument
{
int operator,i2,i3;
char x[ARROWLENGTH+1];

*qq1 = *pp;
for(operator=0; operator < MAXARROW && strlen(Arrow[operator]) > 0; operator++) {
	strcpy(x,Arrow[operator]);
	strcat(x," ");		/* This helps discriminating --> from '-->' */
	if((i2 = FindPattern(qq1,x,&i3)) > 0) {
		*qq2 = &((*qq1)[i2-1]);
		*qq3 = &((*qq1)[i3]);
		*qq4 = *qq3;
		*qq4 = GetEnd(qq4);
		if(operator == 0 || operator == 2) NotBPCase[3] = FALSE; /* '<->', '<--' */
		return(operator);
		}
	}
for(operator=0; operator < MAXARROW && strlen(Arrow[operator]) > 0; operator++) {
	strcpy(x,Arrow[operator]);	/* This search is only useful if |arg2| = 0 */
	if((i2 = FindPattern(qq1,x,&i3)) > 0) {
		*qq2 = &((*qq1)[i2-1]);
		*qq3 = &((*qq1)[i3]);
		*qq4 = *qq3;
		*qq4 = GetEnd(qq4);
		NotBPCase[0] = TRUE;	/* Erase rule */
		return(operator);
		}
	}
return(-1);
}


int NumberWildCards(tokenbyte **p_Q)
{
int i,n;
for(i=0,n=0; (*p_Q)[i] != TEND || (*p_Q)[i+1] != TEND; i+=2)
	if((*p_Q)[i] == T0 && (*p_Q)[i+1] == 1) n++;
return(n);
}


int CompilePatterns(void)
{
int rep,oldjpatt;

CompileOn++;
rep = MISSED; CompiledPt = FALSE;
oldjpatt = Jpatt;
// ShowMessage(TRUE,wMessage,"Looking for time patterns...");
if((rep=ReleasePatternSpace()) != OK) goto ERR;
if((rep=GetPatterns(wAlphabet,TRUE)) != OK){	/* Just counting patterns */
	Jpatt = 0; goto ERR;
	}
if((rep=GetPatterns(wGrammar,TRUE)) != OK){
	Jpatt = 0; goto ERR;
	}
if(Jpatt == 0) {
	goto SORTIR;
	}
if((rep=GetPatternSpace()) != OK) {
	ReleasePatternSpace();
	goto ERR;
	}
Jpatt = 0;
if((rep=GetPatterns(wAlphabet,FALSE)) != OK) {	/* Now creating patterns */
	ReleasePatternSpace();
	goto ERR;
	}
if((rep=GetPatterns(wGrammar,FALSE)) != OK) {
	ReleasePatternSpace();
	goto ERR;
	}
if(Jpatt > 0 && Nature_of_time != SMOOTH) {
	if(!ScriptExecOn) {
		Alert1("Since time-patterns are used you might want to use smooth time");
//		BPActivateWindow(SLOW,wMetronom);
		}
	else {
		PrintBehindln(wTrace,"Since time-patterns were found, time was set to 'smooth'.");
		Nature_of_time = SMOOTH;
		}
	}

SORTIR:
CompiledPt = TRUE;
// HideWindow(Window[wMessage]);
if(CompileOn) CompileOn--;
return(OK);

ERR:
ShowMessage(TRUE,wMessage,"Can't compile patterns");
if(CompileOn) CompileOn--;
return(rep);
}


int GetPatterns(int w, int justcount)
{
long pos,posmax;
int done,result,gap;
char **p_line,line[MAXLIN];

done = FALSE;
pos = ZERO; result = MISSED;
posmax = GetTextLength(w);
p_line = NULL;
while(ReadLine(YES,w,&pos,posmax,&p_line,&gap) == OK) {
	if((*p_line)[0] == '\0') continue;
	if(Mystrcmp(p_line,"TEMPLATES:") == 0) {
		do {
			if(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) != OK) return(result);
			if((*p_line)[0] == '\0') continue;
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		continue;
		}
	if(!done && (Mystrcmp(p_line,"TIMEPATTERNS:") == 0)) {
		done = TRUE; continue;
		}
	if(!done) continue;
	if((*p_line)[0] == '-' && (*p_line)[1] == '-') break;
	MystrcpyHandleToString(MAXLIN,0,line,p_line);
	if((result=ReadPatterns(p_line,justcount)) != OK) {
		Println(wTrace,line);
		goto QUIT;
		}
	}
result = OK;

QUIT:
MyDisposeHandle((Handle*)&p_line);
if(CheckEmergency() != OK) result = ABORT;
return(result);
}


int ReadPatterns(char **p_line,int justcount)
{
char **p_y,c,line2[MAXLIN];
int r,i,j,k,kk,l,length,z=0;
long n;
tokenbyte jj;

if((p_y = (char**) GiveSpace((Size)((BOLSIZE+1) * sizeof(char)))) == NULL) {
	return(ABORT);
	}
if(p_line == NULL) {
	if(Beta) Alert1("=> Err. ReadPatterns(). p_line == NULL");
	r = OK;
	goto QUIT;
	}
l = MyHandleLen(p_line)-1;
r = OK;
for(i=0; i < l;) {
	while(MySpace((*p_line)[i]) && i < l) i++;
	if((*p_line)[i] == '[') {
		while((*p_line)[++i] != ']') if(i >= l) goto QUIT;
		i++; if(i >= l) goto QUIT;
		}
	if((length=GetBol(p_line,&i)) > BOLSIZE) {
		my_sprintf(Message,"\nMaximum length of pattern name: %ld chars.\n",(long)BOLSIZE);
		Print(wTrace,Message);
		ShowError(46,0,0);
		N_err++;
		r = MISSED;
		goto QUIT;
		}
	if(length == -1) {
		ShowError(46,0,0);
		N_err++;
		r = MISSED;
		goto QUIT;
		}
	j = i + length;
	(*p_line)[j++] = '\0';
	for(kk=0; (i+kk) < j; kk++) {
		c = (*p_line)[i + kk];
		(*p_y)[kk] = c;
		}
	jj = CreateBol(TRUE,FALSE,p_y,justcount,FALSE,PATTERN);
	if(jj < 0 || jj >= Jpatt) {
		N_err++;
		r = ABORT;
		goto QUIT;
		}
	while((MySpace((*p_line)[j]) || (*p_line)[j] == '=') && j < l) j++;
	if(j >= l) {
		my_sprintf(Message,"\nIncomplete time pattern: ratio missing.\n");
		Print(wTrace,Message);
		ShowError(47,0,0);
		N_err++;
		r = MISSED;
		goto QUIT;
		}
	if(j >= MAXLIN) {
		Print(wTrace,"\nToo long line in TIME PATTERNS.\n");
		N_err++;
		r = MISSED;
		goto QUIT;
		}
	for(k=0; k < MAXLIN; k++) {
		line2[k] = (*p_line)[k+j];
		if(line2[k] == '\0') break;
		}
	z = 0;
	if((n=GetLong(line2,&z)) == INT_MAX) {
		ShowError(47,0,0);
		N_err++;
		r = MISSED;
		goto QUIT;
		}
	j += z;
	if(!justcount) (*p_Ppatt)[Jpatt-1] = n;
	while(MySpace(c=(*p_line)[j]) && j < l) j++;
	if(j >= l) {
		my_sprintf(Message,
			"\nIncomplete time pattern: ratio denominator missing.\n");
		Print(wTrace,Message);
		ShowError(47,0,0);
		N_err++;
		r = MISSED;
		goto QUIT;
		}
	if(c != '/') {
		my_sprintf(Message,"\n'/' expected.\n");
		Print(wTrace,Message);
		ShowError(47,0,0);
		N_err++;
		r = MISSED;
		goto QUIT;
		}
	j++;
	while(MySpace(c=(*p_line)[j]) && j < l) j++;
	if(j > l) {
		my_sprintf(Message,
			"\nIncomplete time pattern: ratio denominator missing.\n");
		Print(wTrace,Message);
		ShowError(47,0,0);
		N_err++;
		r = MISSED;
		goto QUIT;
		}
	if(j >= MAXLIN) {
		Print(wTrace,"\nToo long line in TIME PATTERNS.\n");
		N_err++;
		r = MISSED;
		goto QUIT;
		}
	for(k=0; k < MAXLIN; k++) {
		line2[k] = (*p_line)[k+j];
		if(line2[k] == '\0') break;
		}
	z = 0;
	if((n=GetLong(line2,&z)) == INT_MAX) {
		ShowError(47,0,0);
		r = MISSED;
		N_err++;
		goto QUIT;
		}
	j += z;
	if(!justcount) (*p_Qpatt)[Jpatt-1] = n;
	i = j;
	}

QUIT:
MyDisposeHandle((Handle*)&p_y);
return(r);
}


#if BP_CARBON_GUI_FORGET_THIS
int InterruptCompile(void)
{
int r,dirtygrammar,dirtyalphabet;

Interrupted = TRUE;
dirtygrammar = Dirty[wGrammar]; dirtyalphabet = Dirty[wAlphabet];
Dirty[wGrammar] = Dirty[wAlphabet] = FALSE;
while((r = MainEvent()) != RESUME && r != STOP && r != EXIT){};
if(r == STOP) r = ABORT;
if(r == RESUME) {
	r = OK; EventState = NO;
	}
if((r != ABORT && r != EXIT) && (Dirty[wGrammar] || Dirty[wAlphabet])) {
	Alert1("Grammar or alphabet changed. Must recompile");
	return(ABORT);
	}
Dirty[wGrammar] = dirtygrammar; Dirty[wAlphabet] = dirtyalphabet;
return(r);
}
#endif /* BP_CARBON_GUI_FORGET_THIS */
