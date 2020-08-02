/* DisplayArg.c (BP2 version CVS) */

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


PrintArg(int datamode,int istemplate,int ret,char showtempo,int ifunc,int nocode,FILE *f,int wind,
	tokenbyte ***pp_b, tokenbyte ***pp_a)

/* datamode = TRUE: space according to metre, FALSE: no space */
/* nocode = FALSE: copy pp_a to file f, if not NULL, or window 'wind' */
/* nocode = TRUE and istemplate = FALSE: copy pp_a without structure to array pp_bÉ */
/* istemplate = TRUE: match istemplate in pp_a against item in pp_bÉ */
/* É and replace '_' in istemplate with terminals in pp_b. */
/* showtempo: add prolongational gaps using "Prod" */
/* ret = TRUE: print a 'return' at the end */
/* ifunc = TRUE: interpret grammar.  Jfunc is the index of '-->' */

{
int itab,h,nhomo,pos,levpar,homoname[MAXLEVEL],r,wasactive,
	copylevel,depth[MAXLEVEL],setting_section,setting_section2,tab[MAXTAB],
	section,sp,print_periods,firstslash,newline,newsection;
unsigned long i,ia,ib,maxib;
double tempo,tempo2,firsttempo,prodtempo,scale,firstscale,s,speed;
TextHandle th;
GrafPtr saveport;
Rect vr;
tokenbyte m,p;
PrintargType printarg;

if(PrintOn) return(RESUME);
if(nocode) wind = -1;
newline = newsection = FALSE;
wasactive = TRUE;

if(wind >= 0 && wind < WMAX && Editable[wind]) {
	th = TEH[wind];
#if BP_CARBON_GUI
	GetPort(&saveport);
	SetPortWindowPort(Window[wind]);
	TextSize(WindowTextSize[wind]);
	Reformat(wind,-1,WindowTextSize[wind],(int)normal,&Black,NO,NO);
#endif /* BP_CARBON_GUI */
#if WASTE
	wasactive = WEIsActive(th);
	Deactivate(th);
	WEFeatureFlag(weFInhibitRecal,weBitSet,th);
#endif
	}
else th = NULL;

if(*pp_b != NULL) maxib = MyGetHandleSize((Handle)*pp_b) / sizeof(tokenbyte);
else maxib = ZERO;

pos = levpar = copylevel = itab = h = ib = section = sp = nhomo = 0;
print_periods = 0;
ia = ZERO; r = OK;
setting_section = firstslash = TRUE;
for(i=ZERO; i < MAXTAB; i++) tab[i] = 0;
tempo = tempo2 = scale = speed = 1.;
firstscale = 0.;

if(istemplate) {
	nocode = TRUE;
	/* Skip section markers in the beginning, match only speeds */
	for(ia=ZERO; ;ia+=2L) {	/* Scan the template */
		m = (**pp_a)[ia]; p = (**pp_a)[ia+1];
		if(m == TEND && p == TEND) break;
		if(m == T1) continue;
		if(m != T0) break;
		if(p == 3) continue;	/* '+' */
		if(p == 11) {		/* '/' */
			speed = GetScalingValue((*pp_a),ia);
			if(scale != 0.) {
				tempo = speed / scale;
				}
			else tempo = 0.;
			ia += 4L;
			continue;
			}
		if(p == 25) {		/* '\' */
			speed = 1. / (GetScalingValue((*pp_a),ia));
			if(scale != 0.) {
				tempo = speed / scale;
				}
			else tempo = 0.;
			ia += 4L;
			continue;
			}
		if(p == 11) {		/* '/' speed up */
			speed = 1. / (GetScalingValue((*pp_a),ia));
			if(scale != 0.) {
				tempo = speed / scale;
				}
			else tempo = 0.;
			ia += 4L;
			continue;
			}
		if(p == 21) {		/* '*' scale up */
			scale = GetScalingValue((*pp_a),ia);
			if(scale != 0.) {
				tempo = speed / scale;
				}
			else tempo = 0.;
			ia += 4L;
			continue;
			}
		if(p == 24) {		/* '**' scale down */
			scale = 1. / GetScalingValue((*pp_a),ia);
			if(scale != 0.) {
				tempo = speed / scale;
				}
			else tempo = 0.;
			ia += 4L;
			continue;
			}
		break;
		}
	scale = 1.;
	for(ib=ZERO;;ib+=2) {	/* Scan the item */
		m = (**pp_b)[ib]; p = (**pp_b)[ib+1];
		if(m == TEND && p == TEND) break;
		if(m == T1) continue;
		if(m != T0) break;
		if(p == 3) continue;	/* '+' */
		if(p == 11) {		/* '/' */
			speed = GetScalingValue((*pp_b),ib);
			if(scale != 0.) {
				tempo2 = speed / scale;
				}
			else tempo2 = 0.;
			ib += 4;
			continue;
			}
		if(p == 25) {		/* '\' */
			speed = 1. / (GetScalingValue((*pp_b),ib));
			if(scale != 0.) {
				tempo2 = speed / scale;
				}
			else tempo2 = 0.;
			ib += 4;
			continue;
			}
		if(p == 21) {		/* '*' scale up */
			scale = GetScalingValue((*pp_b),ib);
			if(scale != 0.) {
				tempo2 = speed / scale;
				}
			else tempo2 = 0.;
			ib += 4L;
			continue;
			}
		if(p == 24) {		/* '**' scale down */
			scale = 1. / GetScalingValue((*pp_b),ib);
			if(scale != 0.) {
				tempo2 = speed / scale;
				}
			else tempo2 = 0.;
			ib += 4L;
			continue;
			}
		break;
		}
	if(tempo != tempo2) {
		r = FAILED; goto QUIT;
		}
	}
if((datamode || showtempo) && !istemplate) {
	// If tempo is greater than 1 and there is no fractional gap we will print periods
	// print_periods = 0 don't print
	// print_periods = 1 print periods
	// print_periods = 2 don't print
	// print_periods = 3 print also section markers 'Â'
	
	setting_section2 = TRUE;
	if(datamode) print_periods = 2;
	firsttempo = 0.;
	scale = 1.;
	for(i=ZERO; ;i+=2L) {
		m = (int)(**pp_a)[i]; p = (int)(**pp_a)[i+1];
		if(m == TEND && p == TEND) break;
		if(setting_section2 && m == T1) continue;
		if(m == T1 && (**pp_a)[i+2] == T1 && (**pp_a)[i+4] == T0 && (**pp_a)[i+5] == 11) {
			/* Found fractional gap */
			print_periods = 0;
			goto READY;
			}
		if(m == T0) {
			switch(p) {
				case 3:		/* '+' */
					continue;
					break;
				case 11:	/* '/' speed up */
					speed = GetScalingValue((*pp_a),i);
					if(scale != 0.) {
						tempo = speed / scale;
						}
					else tempo = 0.;
					if(firsttempo == 0. && tempo != 0.) firsttempo = tempo;
					i += 4L;
					continue;
					break;
				case 25:	/* '\' speed down */
					speed = 1. / (GetScalingValue((*pp_a),i));
					if(scale != 0.) {
						tempo = speed / scale;
						}
					else tempo = 0.;
					if(firsttempo == 0. && tempo != 0.) firsttempo = tempo;
					i += 4L;
					continue;
					break;
				case 21:	/* '*' scale up */
					s = GetScalingValue((*pp_a),i);
					scale = s;
					i += 4L;
					break;
				case 24:	/* '**' scale down */
					s = GetScalingValue((*pp_a),i);
					scale = 1. / s;
					i += 4L;
					break;
				case 7:		/* '¥' */
				case 14:	/* ',' */
					print_periods = 0;
					goto READY;
				}
			}
		setting_section2 = FALSE;
		if(m == T3 || m == T25 || m == T9) {
			if(speed > 1.) tempo2 = speed;
			if(firstscale == 0. && scale != 0.) firstscale = scale;
			}
		}
	if(datamode && tempo2 > 1.) print_periods++;
	else print_periods = 0;
	}
	
READY:
if(f == NULL) {
	Alert1("Can't write to disk.\n(incorrect file pointer)");
	r = FAILED; goto OUT2;
	}

if(firstscale == 0.) firstscale = 1.;
if(Prod < 1.) Prod = 1.;
if(showtempo || print_periods > 0) {
	prodtempo = Prod;
	}
else prodtempo = 0.;
speed = 1.;
scale = 1.;

if((*pp_a) == NULL && !nocode) {
	r = Display('\0',nhomo,levpar,homoname,depth,&maxib,pp_a,&ia,istemplate,
		(tokenbyte)0,(tokenbyte)0,0,pp_b,&ib,f,th," << Not encoded >> ",NULL,-1);
	}
else {
	PrintOn = TRUE;
	printarg.datamode = datamode;
	printarg.firstscale = firstscale;
	printarg.istemplate = istemplate;
	printarg.wind = wind;
	printarg.prodtempo = prodtempo;
	printarg.ifunc = ifunc;
	printarg.nocode = nocode;
	printarg.ia = ia;
	printarg.levpar = 1;
	printarg.nhomo = nhomo;
	r = PrintArgSub(&printarg,&maxib,th,f,pp_b,pp_a,
		&ib,&itab,&speed,&pos,homoname,depth,copylevel,tab,setting_section,
		1,&h,&section,sp,print_periods,&firstslash,&newline,&newsection,
		&scale);
	}

OUT2:
if(nocode) {
	if(!istemplate) {
		(**pp_b)[ib++] = TEND; (**pp_b)[ib++] = TEND;
		}
	else {
		while((**pp_b)[ib] == T3 && (**pp_b)[ib+1] == 1) ib += 2;	/* Skip trailing silences */
		if(r == OK && ((**pp_b)[ib] != TEND || (**pp_b)[ib+1] != TEND)) {
			r = FAILED; goto QUIT;
			}
		}
	}
else {
	if(th != NULL) {
#if WASTE
		WEFeatureFlag(weFInhibitRecal,weBitClear,th);
		if(wasactive) Activate(th);
#endif
		if(r == OK && print_periods && pos > 0 && pos < (Prod/firstscale)) {
			for(i=pos; i < (Prod/firstscale); i++) {
				if(OutChar(f,th,'-') != OK) {
					r = ABORT; goto QUIT;
					}
				}
			}
		if(ret) DoKey('\n',0,th);
		// else  DoKey('\0',0,th);	/* Forces text to be drawn and vertical scroll to be fixed */
		}
	else if(ret) OutChar(f,th,'\n');
	}

QUIT:
PrintOn = FALSE;
if(th != NULL) {
/*	ShowSelect(CENTRE,wind); */
	CheckTextSize(wind);
#if BP_CARBON_GUI
	if(saveport != NULL) SetPort(saveport);
	else if(Beta) Alert1("Err PrintArg(). saveport == NULL");
#endif /* BP_CARBON_GUI */
	}
return(r);
}


PrintArgSub(PrintargType *p_printarg,unsigned long *p_maxib,TextHandle th,
	FILE *f,tokenbyte ***pp_b,tokenbyte ***pp_a,unsigned long *p_ib,
	int *p_itab,double *p_speed,int *p_pos,int *homoname,
	int *depth,
	int copylevel,int *tab,int setting_section,int beat,
	int *p_h,int *p_section,int sp,int print_periods,int *p_firstslash,int *p_newline,
	int *p_newsection,double *p_scale)
{
tokenbyte m,p,mm,pp;
int h,j,k,r,newtempo,forceshowtempo,key,new_depth[MAXLEVEL],compiledmem,maxpoly,level,
	datamode,istemplate,wind,ifunc,nocode,levpar,nhomo;
unsigned long i,ii,ia,numberprolongations,oldnumberprolongations;
char c,line[MAXLIN],**p_sequence;
TextOffset dummy;
long posmem,n;
double tempo,x,y,scale,speed,s,perioddivision,firstscale,prodtempo;
Handle ptr;

datamode = p_printarg->datamode;
firstscale = p_printarg->firstscale;
istemplate = p_printarg->istemplate;
wind = p_printarg->wind;
prodtempo = p_printarg->prodtempo;
ifunc = p_printarg->ifunc;
nocode = p_printarg->nocode;
ia = p_printarg->ia;
levpar = p_printarg->levpar;
nhomo = p_printarg->nhomo;


r = OK;
newtempo = forceshowtempo = FALSE;
level = 0;
maxpoly = 32;
speed = 1.;
numberprolongations = oldnumberprolongations = perioddivision = 0;

if(prodtempo > 0.) {
	if((*p_speed) != 0.) {
		prodtempo = (Prod / (*p_speed));
		numberprolongations = Prod * (*p_scale) / speed / firstscale;
		}
	}
else prodtempo = 0.;

if(nocode) {
	if((p_sequence = (char**) GiveSpace((Size)(maxpoly * sizeof(char)))) == NULL) {
		return(ABORT);
		}
	(*p_sequence)[level] = FALSE;
	}
else p_sequence = NULL;

for(i=ia; ; i+=2L) {
	m = (int)(**pp_a)[i]; p = (int)(**pp_a)[i+1];
	if(m == TEND && p == TEND) break;
#if BP_CARBON_GUI
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	if(!nocode && ((i % 10L) == 0)) {
		if((r=MyButton(2)) != FAILED) {
			StopCount(0);
			if(th != NULL) {
	#if WASTE
				WEFeatureFlag(weFInhibitRecal,weBitClear,th);
	#endif
				AdjustTextInWindow(wind);
				TextGetSelection(&dummy, &posmem, th);
				}
			Interrupted = TRUE; SetButtons(TRUE);
			compiledmem = CompiledGr;
			if(r == OK) while((r = MainEvent()) != RESUME && r != STOP && r != EXIT);
			if(r == EXIT) goto OUT;
			if(th != NULL) {
				SetSelect(posmem,posmem,th);
	#if WASTE
				WEFeatureFlag(weFInhibitRecal,weBitSet,th);
	#endif
				}
			if(r == STOP || (compiledmem && !CompiledGr)) {
				if(th != NULL) {
					Reformat(wind,(int) kFontIDCourier,WindowTextSize[wind],(int) normal,&Black,TRUE,NO);
					Print(wind,"\n");
					}
				r = ABORT; goto OUT;
				}
			}
		else if(WASTE) PleaseWait();
		r = OK;
		}
	if(EventState != NO) {
		r = EventState; goto OUT;
		}
#endif /* BP_CARBON_GUI */
	if(m == T3 || m == T25) {
		if((r=CheckPeriodOrLine(print_periods,p_newline,p_newsection,f,th,&beat,numberprolongations,
				&sp)) != OK) goto OUT;
		}
	if(m == T0 && setting_section && (p == 3 || p == 11)) {	/* '+'  or initial '/' */
		if(datamode) {
			if((*p_itab) >= MAXTAB) {
				Alert1("Too many tab sections: check '.+.+./.' in begining");
				r = ABORT; goto OUT;
				}
			tab[(*p_itab)++] = n;
			for(j=(*p_itab); j < MAXTAB; j++) tab[j] = 0;
			}
		if(((!datamode || setting_section) || nocode || prodtempo == 0.) && p != 11) {
			if((r=Display(Code[p],nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					m,p,nocode,pp_b,p_ib,f,th,"%c",NULL,-1)) != OK) {
				goto OUT;
				}
			sp = 0;
			}
		if(p == 11) setting_section = FALSE;
		if(setting_section) continue;
		}
	if(m == T0 && p > 10) {
		if(istemplate) {
			switch(p) {
				case 11: /* '/' speed up */
					mm = (**pp_b)[*p_ib];
					pp = (**pp_b)[*p_ib+1];
					if(mm == T0 && pp == 21) { /* '*' scale up */
						s = GetScalingValue((*pp_b),*p_ib);
						if(s == 1.) {	/* Skip '*1' */
							(*p_ib) += 6L;
							}
						}
					break;
				}
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,
					p,nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
				goto OUT;
				}
			continue;
			}
		switch(p) {
			case 11: /* '/' speed up */
				speed = GetScalingValue((*pp_a),i);
				if((*p_scale) != 0.) {
					tempo = speed / (*p_scale);
					}
				else tempo = prodtempo = 0.;
				i += 4;
				if(prodtempo > 0.) {
					prodtempo = Prod / tempo;
					oldnumberprolongations = numberprolongations;
					numberprolongations = Prod * (*p_scale) / speed / firstscale;
					}
				if((*p_speed) != speed || oldnumberprolongations != numberprolongations
						|| prodtempo == 0. || istemplate || forceshowtempo
						|| perioddivision != (Prod/firstscale)) {
					newtempo = TRUE;
					(*p_speed) = speed;
					}
				(*p_firstslash) = FALSE;
				continue;
				break;
			case 25:	/* '\' speed down */
				speed = 1. / (GetScalingValue((*pp_a),i));
				if((*p_scale) != 0.) {
					tempo = speed / (*p_scale);
					}
				else tempo = prodtempo = 0.;
				i += 4;
				if(prodtempo > 0.) {
					prodtempo = Prod / tempo;
				//	numberprolongations = (Ratio * firsttempo) / speed;
					oldnumberprolongations = numberprolongations;
					numberprolongations = Prod * (*p_scale) / speed / firstscale;
					}
				if((*p_speed) != speed || oldnumberprolongations != numberprolongations
						|| prodtempo == 0. || istemplate || forceshowtempo) {
					newtempo = TRUE;
					(*p_speed) = speed;
					}
				(*p_firstslash) = FALSE;
				continue;
				break;
			case 21:	/* '*' scale up */
				scale = GetScalingValue((*pp_a),i);
				if(scale != 0.) {
					tempo = speed / scale;
					}
				else tempo = prodtempo = 0.;
				i += 4;
				if(prodtempo > 0.) {
					prodtempo = Prod / tempo;
					oldnumberprolongations = numberprolongations;
					numberprolongations = Prod * scale / speed / firstscale;
					}
				if((*p_scale) != scale || oldnumberprolongations != numberprolongations
						|| prodtempo == 0. || istemplate) {
					(*p_scale) = scale;
					}
				(*p_firstslash) = FALSE;
				continue;
				break;
			case 24:	/* '**' scale down */
				s = GetScalingValue((*pp_a),i);
				scale = 1. / s;
				if(scale != 0.) {
					tempo = speed / scale;
					}
				else tempo = prodtempo = 0.;
				i += 4;
				if(prodtempo > 0.) {
					prodtempo = Prod / tempo;
					oldnumberprolongations = numberprolongations;
					numberprolongations = Prod * scale / speed / firstscale;
					}
				if((*p_scale) != scale || oldnumberprolongations != numberprolongations
						|| prodtempo == 0. || istemplate) {
					(*p_scale) = scale;
					}
				(*p_firstslash) = FALSE;
				continue;
				break;
			}
		}
	if(newtempo) {
		if(nocode) {
			if((*p_scale) >= 1. || (*p_scale) == 0.) {
				y = modf(((*p_scale) / (double)TOKBASE),&x);
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T0,
						(tokenbyte)21,nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
					goto OUT;
					}
				}
			else {
				y = modf((1. / (*p_scale) / (double)TOKBASE),&x);
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T0,
						(tokenbyte)24,nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
					goto OUT;
					}
				}
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T1,
					(tokenbyte) x,nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
				goto OUT;
				}
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T1,
					(tokenbyte) (y * TOKBASE),nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
				goto OUT;
				}
			if((*p_speed) >= 1. || (*p_speed) == 0.) {
				y = modf(((*p_speed) / (double)TOKBASE),&x);
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T0,
						(tokenbyte)11,nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
					goto OUT;
					}
				}
			else {
				y = modf((1. / (*p_speed) / (double)TOKBASE),&x);
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T0,
						(tokenbyte)25,nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
					goto OUT;
					}
				}
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T1,
					(tokenbyte) x,nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
				goto OUT;
				}
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T1,
					(tokenbyte) (y * TOKBASE),nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
				goto OUT;
				}
			}
		else {
			if(datamode)  {
				if((oldnumberprolongations == numberprolongations
					|| (print_periods && beat > 1))
					&& perioddivision == (Prod/firstscale)) goto DONESPEED;
				oldnumberprolongations = numberprolongations;
				perioddivision = (Prod/firstscale);
				if(perioddivision < 2.) goto DONESPEED;
				if(!setting_section) {
					if(sp > 0 && OutChar(f,th,' ') != OK) {
						r = ABORT; goto OUT;
						}
					}
				else {
		/*			if(OutChar(f,th,'*') != OK) {
						r = ABORT; goto OUT;
						}
					if(OutChar(f,th,'1') != OK) {
						r = ABORT; goto OUT;
						} */
					}
				if(OutChar(f,th,'/') != OK) {
					r = ABORT; goto OUT;
					}
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
					(tokenbyte)0,nocode,pp_b,p_ib,f,th,"%ld",NULL,(long)perioddivision)) != OK) {
					goto OUT;
					}
				sp = 1;
				goto DONESPEED;
				}
			if(sp > 0 && OutChar(f,th,' ') != OK) {
				r = ABORT; goto OUT;
				}
			scale = (*p_scale);
			if(scale < InvMaxTempo) scale = 0.;
			if(scale > 1. || scale == 0. || (forceshowtempo && scale == 1.)) {
				if(OutChar(f,th,'*') != OK) {
					r = ABORT; goto OUT;
					}
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
					(tokenbyte)0,nocode,pp_b,p_ib,f,th,"%ld",NULL,(long)scale)) != OK) {
					goto OUT;
					}
				}
			else if(scale < 1.) {
				if(OutChar(f,th,'*') != OK) {
					r = ABORT; goto OUT;
					}
				if(OutChar(f,th,'*') != OK) {
					r = ABORT; goto OUT;
					}
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
					(tokenbyte)0,nocode,pp_b,p_ib,f,th,"%ld",NULL,(long)(1./scale))) != OK) {
					goto OUT;
					}
				}
			if((*p_speed) >= 1.) {
				if(OutChar(f,th,'/') != OK) {
					r = ABORT; goto OUT;
					}
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
					(tokenbyte)0,nocode,pp_b,p_ib,f,th,"%ld",NULL,(long)(*p_speed))) != OK) {
					goto OUT;
					}
				}
			else  {
				if(OutChar(f,th,'\\') != OK) {
					r = ABORT; goto OUT;
					}
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
					(tokenbyte)0,nocode,pp_b,p_ib,f,th,"%ld",NULL,(long)(1./(*p_speed)))) != OK) {
					goto OUT;
					}
				}
			if(OutChar(f,th,' ') != OK) {
				r = ABORT; goto OUT;
				}
			}
		sp = 1;
		
DONESPEED:
		newtempo = forceshowtempo = FALSE;
		}
	if(m == T25) {	/* Simple note */
		if(!nocode && sp != 4 && sp && (sp < 3 || (datamode && *(p_speed) < 2.)
			|| SplitTimeObjects)) if(Space(f,th,&sp) != OK) {
					r = ABORT; goto OUT;
					}
		p += 16384;
		goto TERMINAL;
		}
	if(m == T3 && p < Jbol && p_Bol != NULL) {				/* Terminal */
		if(!nocode && sp != 4 && sp && (sp < 3 || SplitTimeObjects))
			if(Space(f,th,&sp) != OK) {
				r = ABORT; goto OUT;
				}
TERMINAL:
		setting_section = FALSE;
		for(n=nhomo; n >= 1; n--) {
			h = homoname[n];
			if(h >= Jhomo) {
				if(Beta) {
					Alert1("Err. PrintArgSub(). h >= Jhomo");
					r = ABORT; goto OUT;
					}
				}
			if(depth[n] < levpar) p = Image(h,p);
			}
		if(p >= 16384) m = T25;
		else m = T3;
		if(!nocode && UseTextColor) {
			if(p < 2) Reformat(wind,-1,-1,-1,&Black,NO,NO);	/* '-' or '_' */
			else	if(p < 16384) Reformat(wind,-1,-1,-1,&Color[TerminalC],NO,NO);
					else Reformat(wind,-1,-1,-1,&Color[NoteC],NO,NO);
			}
		if(ifunc && !nocode && p < Jbol && (*((*p_Bol)[p]))[0] == '\'') {
			for(j=1; (*((*p_Bol)[p]))[j] != '\''; j++) {
				if(OutChar(f,th,(*((*p_Bol)[p]))[j]) != OK) {
					r = ABORT; goto OUT;
					}
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			}
		else {
			if(p < 16384) {
				if(!nocode && (*((*p_Bol)[p]))[0] == '\'' && OutChar(f,th,' ') != OK) {
					r = ABORT; goto OUT;
					}
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,nocode,
						pp_b,p_ib,f,th,"%s",(*p_Bol)[p],-1)) != OK) {
					goto OUT;
					}
				}
			else {
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p-16384,
						nocode,pp_b,p_ib,f,th,"\0",NULL,-1)) != OK) {
					goto OUT;
					}
				}
			if(nocode) continue;
			
PRINTPROLONGATIONS:
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			if(print_periods && (++(*p_pos)) >= perioddivision) {
				(*p_pos) = 0;
				(*p_newsection) = TRUE;
				if(tab[*p_section] > 0 && ((++(*p_h)) >= tab[*p_section])) {
					(*p_h) = 0; (*p_newline) = TRUE;
					(*p_newsection) = FALSE;
					if(tab[++(*p_section)] == 0) (*p_section) = 0;
					}
				}
			if(numberprolongations > 0) {
				for(j=1; j < numberprolongations; j++) {
					if((r=CheckPeriodOrLine(print_periods,p_newline,p_newsection,f,th,&beat,
							numberprolongations,&sp)) != OK) goto OUT;
					
					if(!nocode && sp) if(Space(f,th,&sp) != OK) {
						r = ABORT; goto OUT;
						}
					if((r=Display('_',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T3,
							(tokenbyte)0,nocode,pp_b,p_ib,f,th,"%c",NULL,-1)) != OK) {
						goto OUT;
						}
					if(SplitTimeObjects) sp = 1;
					if(print_periods && (++(*p_pos)) >= perioddivision) {
						(*p_pos) = 0;
						(*p_newsection) = TRUE;
						if(tab[*p_section] && (++(*p_h) >= tab[*p_section])) {
							(*p_h) = 0; (*p_newline) = TRUE;
							(*p_newsection) = FALSE;
							if(!tab[++*p_section]) (*p_section) = 0;
							}
						}
					}
				}
			}
		if(sp == 4 || ifunc) {
			if(OutChar(f,th,' ') != OK) {
				r = ABORT; goto OUT;
				}
			}
		sp = 3;
		continue;
		}
		
	if(m == T1) {		/* Digit */
		i += 2L;
		(*p_firstslash) = FALSE;
		n = ((long) (TOKBASE * p)) + (**pp_a)[i+1];
		if(!setting_section && (**pp_a)[i+2] == T0 && (**pp_a)[i+3] == 11
				&& (**pp_a)[i+4] == T1 && (**pp_a)[i+6] == T1) {
			/* Fractional gap */
			if(nocode) {
				for(ii=i-2L; ii < i+7L; ii+=2L) {
					if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,
							istemplate,(**pp_a)[ii],(**pp_a)[ii+1],nocode,pp_b,
							p_ib,f,th,"",NULL,-1)) != OK) {
						goto OUT;
						}
					}
				}
			else {
				if(OutChar(f,th,' ') != OK) {
					r = ABORT; goto OUT;
					}
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
						nocode,pp_b,p_ib,f,th,"%ld",NULL,(long)n)) != OK) {
					goto OUT;
					}
				if(OutChar(f,th,'/') != OK) {
					r = ABORT; goto OUT;
					}
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
						m,p,nocode,pp_b,p_ib,f,th,"%ld",NULL,(long)
						(((long)TOKBASE * (**pp_a)[i+5]) + (**pp_a)[i+7]))) != OK) {
					goto OUT;
					}
				if(OutChar(f,th,' ') != OK) {
					r = ABORT; goto OUT;
					}
				sp = 0;
				}
			i += 6L;
			continue;
			}
		if(!nocode && sp >= 2) if(Space(f,th,&sp) != OK) {
			r = ABORT; goto OUT;
			}
		if(nocode) {
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T1,
					p,nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
				goto OUT;
				}
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T1,
					(**pp_a)[i+1],nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
				goto OUT;
				}
			}
		else {
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
					nocode,pp_b,p_ib,f,th,"%ld",NULL,n)) != OK) {
				goto OUT;
				}
			if(Space(f,th,&sp) != OK) {
				r = ABORT; goto OUT;
				}
			}
		if(!setting_section) sp = 2;
		continue;
		}
	setting_section = FALSE;
		
	if(m == T12) {	/* Performance Control without argument: _velcont, etc. */
		if(!nocode && sp != 4) if(Space(f,th,&sp) != OK) {
			r = ABORT; goto OUT;
			}
		if(nocode) {
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
					nocode,pp_b,p_ib,f,th,"%c",NULL,-1)) != OK) {
				goto OUT;
				}
			}
		else {
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			switch(p) {
				case 0:	/* _velcont */
				case 1:	/* _velstep */
					p += 2; break;
				case 2:	/* _modstep */
				case 3:	/* _modcont */
					p += 4; break;
				case 4:	/* _pitchstep */
				case 5:	/* _pitchcont */
					p += 5; break;
				case 6:	/* _presstep */
				case 7:	/* _presscont */
					p += 6; break;
				case 8: /* _volumestep */
				case 9: /* _volumecont */
					p += 9; break;
				case 10: /* _articulstep */
				case 11: /* _articulcont */
				case 12: /* _velfixed */
				case 13: /* _modfixed */
				case 14: /* _pitchfixed */
				case 15: /* _pressfixed */
				case 16: /* _volumefixed */
				case 17: /* _articulfixed */
					p += 11; break;
				case 18: /* _panfixed */
					p += 21; break;
				case 19: /* _panstep */
				case 20: /* _pancont */
					p += 18; break;
				case 21: /* _retro */
				case 22: /* _rndseq */
				case 23: /* _randomize */
				case 24: /* _ordseq */
					p += 27; break;
				case 25: /* _mapfixed */
				case 26: /* _mapcont */
				case 27: /* _mapstep */
					p += 28; break;
				case 28: /* _transposefixed */
				case 29: /* _transposecont */
				case 30: /* _transposestep */
					p += 34; break;
				}
			sprintf(line,"%s ",*((*p_PerformanceControl)[p]));
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
				(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			sp = 0;
			}
		continue;
		}
		
	if(m == T43) {	/* Performance Control '_tempo()' */
		if(!nocode && sp != 4) if(Space(f,th,&sp) != OK) {
			r = ABORT; goto OUT;
			}
		if(nocode) {
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
					nocode,pp_b,p_ib,f,th,"%c",NULL,-1)) != OK) {
				goto OUT;
				}
			}
		else {
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			sprintf(line,"%s(",*((*p_PerformanceControl)[61]));
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			sprintf(line,"%ld",(long)p);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
					(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,"/",NULL,-1)) != OK) {
				goto OUT;
				}
			i += 2L;
			m = (**pp_a)[i];
			p = (**pp_a)[i+1];
			if(m != T43) {
				if(Beta) Alert1("Err. PrintArgSub().  Second T43 is missing");
				i -= 2L;
				continue;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			sprintf(line,"%ld",(long)p);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
					(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,")",NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			sp = 2;
			}
		continue;
		}
		
	if(m == T37) {	/* Performance Control '_keymap()' */
		if(!nocode && sp != 4) if(Space(f,th,&sp) != OK) {
			r = ABORT; goto OUT;
			}
		if(nocode) {
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
					nocode,pp_b,p_ib,f,th,"%c",NULL,-1)) != OK) {
				goto OUT;
				}
			}
		else {
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			sprintf(line,"%s(",*((*p_PerformanceControl)[52]));
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			PrintNote(p % 128,-1,-1,line);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,",",NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			PrintNote((p - (p % 128)) / 128,-1,-1,line);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,",",NULL,-1)) != OK) {
				goto OUT;
				}
				
			i += 2L;
			m = (**pp_a)[i];
			p = (**pp_a)[i+1];
			
			if(m != T37) {
				if(Beta) Alert1("Err. PrintArgSub().  Second T37 is missing");
				i -= 2L;
				continue;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			PrintNote(p % 128,-1,-1,line);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,",",NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			PrintNote((p - (p % 128)) / 128,-1,-1,line);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,")",NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			sp = 2;
			}
		continue;
		}
	
		
	if(m == T40) {	/*Tool '_keyxpand()' */
		if(!nocode && sp != 4) if(Space(f,th,&sp) != OK) {
			r = ABORT; goto OUT;
			}
		if(nocode) {
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
					nocode,pp_b,p_ib,f,th,"%c",NULL,-1)) != OK) {
				goto OUT;
				}
			}
		else {
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			sprintf(line,"%s(",*((*p_PerformanceControl)[58]));
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			key = p % 256;
			if(key > 127) {
				sprintf(line,"K%ld=%ld",(long)key-128,(long)ParamValue[key-128]);
				}
			else PrintNote(key,-1,-1,line);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,",",NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			goto SHOWNUMBER;
			}
		continue;
		}
		
	if(m == T13) {	/* Performance Control '_script()' */
		if(!nocode && sp != 4) if(Space(f,th,&sp) != OK) {
			r = ABORT; goto OUT;
			}
		if(nocode) {
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
					nocode,pp_b,p_ib,f,th,"%c",NULL,-1)) != OK) {
				goto OUT;
				}
			}
		else {
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			sprintf(line,"%s(",*((*p_PerformanceControl)[4]));
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			if(p_Script == NULL || p >= Jscriptline || p < 0) {
				if(Beta)
					Alert1("Err. PrintArgSub(). Event script line not found");
				r = ABORT; goto OUT;
				}
			sprintf(line,"%s",*((*p_Script)[p]));
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
					(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,")",NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			sp = 2;
			}
		continue;
		}
		
	if(ifunc && m == T0 && p == 5) { /* interpreting grammar: ';' becomes '\r' */
		if(OutChar(f,th,'\n') != OK) {
					r = ABORT; goto OUT;
					}
		sp = 0;
		continue;
		}
		
	if(ifunc && m == T3 && p == Jfunc) {	/* interpreting grammar */
		if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
				(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th," --> ",NULL,-1)) != OK) {
			goto OUT;
			}
		sp = 0;
		continue;
		}
		
	if(m == T4 && p <= Jvar) {							/* Variable */
		if(p_Var == NULL) continue;	/* Happens when there is no grammar and no glossary */
		if(!nocode) {
			if(sp != 4 && Space(f,th,&sp) != OK) {
				r = ABORT; goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Color[VariableC],NO,NO);
			}
		if(SplitVariables) {
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
					nocode,pp_b,p_ib,f,th,"|%s|",(*p_Var)[p],-1)) != OK) {
				goto OUT;
				}
			if(sp != 4) sp = 0;
			}
		else {
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
					nocode,pp_b,p_ib,f,th,"%s",(*p_Var)[p],-1)) != OK) {
				goto OUT;
				}
			if(sp != 4) sp = 2;
			}
		if(th != NULL) Reformat(wind,-1,-1,-1,&Black,NO,NO);
		if(sp == 4) {
			if(OutChar(f,th,' ') != OK) {
					r = ABORT; goto OUT;
					}
			sp = 0;
			}
		continue;
		}
		
	if(m == T0) {
		if(p == 10 && th != NULL) Reformat(wind,-1,-1,(int) bold,&None,NO,NO);	/* 'S' */
		if(p == 12) {	/* '{' */
			level++;
			if(level >= maxpoly) {
				maxpoly = (maxpoly * 3) / 2;
				if(nocode) {
					ptr = (Handle) p_sequence;
					if((ptr = IncreaseSpace(ptr)) == NULL) return(ABORT);
					p_sequence = (char**) ptr;
					}
				}
			}
		if(!datamode && (p == 12 || p == 13 || p == 14 || p == 7)) /* '{', '}', comma, '¥' */
			forceshowtempo = TRUE;
		if(p == 22 || p == 23) continue;	/* Temporary bracket '|' */
		if(nocode) {
			if(p == 12) {	/* '{' */
				(*p_sequence)[level] = FALSE;
				}
			if(p == 7 || (p > 11 && p < 20)) {		/* '¥', '{', '}', comma */
				if((*p_sequence)[level]) {
					if(p == 13 || p == 14) {				/* '}', comma */
						(*p_sequence)[level] = FALSE;
						/* Insert closing '|' for bracketing sequential expression */
						if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T0,
								(tokenbyte)23,nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
							goto OUT;
							}
						}
					}
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,nocode,
						pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
					goto OUT;
					}
				if((p == 12 || p == 14) && !(*p_sequence)[level]
						&& SequenceField(pp_a,i+2)) {			/* '{', comma */
					(*p_sequence)[level] = TRUE;
					/* Insert opening '|' for bracketing sequential expression */
					if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T0,
							(tokenbyte)22,nocode,pp_b,p_ib,f,th,"",NULL,-1)) != OK) {
						goto OUT;
						}
					}
				}
			}
		else {
			if(!datamode || ((p > 11 || p == 7) && (print_periods < 3 || p == 18 || p == 19))) {
				if((p != 1 || sp != 4) && p != 9 && p != 8 && p != 18 && p != 7
						&& (p < 12 || p > 14))
					if(Space(f,th,&sp) != OK) {
						r = ABORT; goto OUT;
						}
				if(p == 17) {
					sprintf(line,"%s",*((*p_PerformanceControl)[42]));
					if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
							(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
						goto OUT;
						}
					}
				else {
					if(p < MAXCODE2 && Display(Code[p],nhomo,levpar,homoname,depth,
						p_maxib,pp_a,&i,istemplate,m,p,nocode,pp_b,p_ib,f,th,"%c",NULL,-1)
								== ABORT) {
						goto OUT;
						}
					}
				sp = 2;
				switch(p) {
					case 2:
						sp = 4; break;
					case 3:
					case 7:
					case 8:
					case 12:
					case 14:
					case 19:
					case 22:
					case 23:
						sp = 0; break;
					case 13:
					case 17:
					case 18:
						sp = 1; break;
					}
				}
			if(p == 7 || p == 14) {		/* '¥' or ',' */
				(*p_pos) = 0;	/* Restart sequence */
				(*p_newsection) = FALSE;
				}
			}
		if(p == 13 && level > 0) {
			level--;
			}
		if(p == 10 && th != NULL) Reformat(wind,-1,-1,(int) normal,&None,NO,NO);	/* 'S' */
		if(p == 8 && (++levpar) >= MAXLEVEL) {	/* '(' */
			ShowError(40,0,0);
			r = ABORT; goto OUT;
			}	
		if(p == 9) {										/* ')' */
			if(levpar == 1 && copylevel) {	/* End of copying master */
				r = OK; goto OUT;
				}
			if(levpar > 0) levpar--;
			for(n=nhomo; n >= 1; n--) if(depth[n] >= levpar) nhomo--;
			}
		continue;
		}
		
	if(m == T2) {
		if(!nocode && !datamode) {
			sp = 2;
			if(p == 0) {
				if(OutChar(f,th,'=') != OK) {
					r = ABORT; goto OUT;
					} /* ZERO marker  */
				}
			else {
				if(OutChar(f,th,':') != OK) {
					r = ABORT; goto OUT;
					} /* slave marker */
				}
			}
		if(p > 0) {							/* slave marker */
			if((ia=SearchOrigin(pp_a,i,p)) == -1) {
				r = ABORT; goto OUT;
				}
			for(j=1; j <= nhomo; j++)
				new_depth[j] = depth[j] - levpar;
			copylevel++;
			
			p_printarg->datamode = datamode;
			p_printarg->firstscale = firstscale;
			p_printarg->istemplate = istemplate;
			p_printarg->wind = wind;
			p_printarg->prodtempo = prodtempo;
			p_printarg->ifunc = ifunc;
			p_printarg->nocode = nocode;
			p_printarg->ia = ia;
			p_printarg->levpar = 1;
			p_printarg->nhomo = nhomo;
			
			if((r=PrintArgSub(p_printarg,p_maxib,th,f,pp_b,pp_a,p_ib,p_itab,
						p_speed,p_pos,homoname,
						new_depth,copylevel,tab,setting_section,beat,p_h,
						p_section,sp,print_periods,p_firstslash,p_newline,p_newsection,
						p_scale)) != OK) {
					goto OUT;
					}
			copylevel--;
			if(levpar > 0) levpar--;
			for(n=nhomo; n >= 1; n--)
				if(depth[n] >= levpar) nhomo--;
			i++; i++;
			}
		continue;
		}
		
	if(m == T9 && p < Jpatt) {				/* Time pattern */
		if(!nocode && sp != 4 && sp) if(Space(f,th,&sp) != OK) {
					r = ABORT; goto OUT;
					}
		if(th != NULL) Reformat(wind,-1,-1,-1,&Color[TimePatternC],NO,NO);
		if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
				nocode,pp_b,p_ib,f,th,"%s",(*p_Patt)[p],-1)) != OK) {
			goto OUT;
			}
		if(th != NULL) Reformat(wind,-1,-1,-1,&Black,NO,NO);
		if(sp == 4 || ifunc) {
			if(OutChar(f,th,' ') != OK) {
					r = ABORT; goto OUT;
					}
			}
		sp = 3;
		goto PRINTPROLONGATIONS;
		}
		
	if(m == T8 && p <= MAXWAIT) {	/* Synchronization tag */
		if(!nocode) {
			if(Space(f,th,&sp) != OK) {
					r = ABORT; goto OUT;
					}
			sp = 1;
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
					nocode,pp_b,p_ib,f,th,"<<",NULL,-1)) != OK) {
				{
					goto OUT;
					}
				}
			Reformat(wind,-1,-1,-1,&Color[TagC],NO,NO);
			}
		if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
				nocode,pp_b,p_ib,f,th,"W%ld",NULL,p)) != OK) {
			{
					goto OUT;
					}
			}
		if(!nocode) {
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
				nocode,pp_b,p_ib,f,th,">>",NULL,-1)) != OK) {
				{
					goto OUT;
					}
				}
			}
		continue;
		}
		
	if(m == T7 && (p < Jbol || p > 16383)) {	/* Out-time object or out-time simple note */
		if(!nocode) {
			if(Space(f,th,&sp) != OK) {
					r = ABORT; goto OUT;
					}
			sp = 1;
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
					(tokenbyte)0,nocode,pp_b,p_ib,f,th,"<<",NULL,-1)) != OK) {
				goto OUT;
				}
			Reformat(wind,-1,-1,-1,&Color[TerminalC],NO,NO);
			}
		if(p < 16384) {
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
					nocode,pp_b,p_ib,f,th,"%s",(*p_Bol)[p],-1)) != OK) {
				goto OUT;
				}
			}
		else {	/* Simple note */
			if(nocode) Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
				nocode,pp_b,p_ib,f,th,"%s",NULL,-1);
			else {
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,T25,p-16384,
						nocode,pp_b,p_ib,f,th,"%s",NULL,-1)) != OK) {
					goto OUT;
					}
				}
			}
		if(!nocode) {
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
					(tokenbyte)0,nocode,pp_b,p_ib,f,th,">>",NULL,-1)) != OK) {
				goto OUT;
				}
			}
		continue;
		}
		
	if(m == T5) {				/* Homomorphism */
		if(th != NULL) Reformat(wind,-1,-1,-1,&Color[HomomorphismC],NO,NO);
		if(!nocode && !datamode && Jhomo > 0) {
			if(Space(f,th,&sp) != OK) {
				r = ABORT; goto OUT;
				}
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
					nocode,pp_b,p_ib,f,th,"%s",(*p_Homo)[p],-1)) != OK) {
				goto OUT;
				}
			sp = 1;
			}
		if(th != NULL) Reformat(wind,-1,-1,-1,&Black,NO,NO);
		if(p >= 0) {
			nhomo++;
			depth[nhomo] = levpar;
			homoname[nhomo] = p;
			}
		continue;
		}
	
	if(m == T10 || m == T11 || (m >= T14 && m <= T24) || (m >= T26 && m <= T36) || m == T38
			|| m == T39 || m == T41 || m == T42) {
		// _chan(), _vel(), _mod(), _pitchbend(), _press(), _switchon(,), _switchoff(,)
		// _volume(), _pan(),_legato(), _staccato(),_pitchrange(),_pitchrate(),_modrate(),
		// _pressrate(),_transpose(),_volumerate(),_volumecontrol(),_panrate(),_pancontrol()
		// _ins(), _value(), _step(), _cont(), _rndvel(), _rotate(), _rndtime() _srand()
		if(!nocode && sp != 4) if(Space(f,th,&sp) != OK) {
			r = ABORT; goto OUT;
			}
		if(nocode) {
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
					nocode,pp_b,p_ib,f,th,"%c",NULL,-1)) != OK) {
				goto OUT;
				}
			}
		else {
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			switch(m) {
				case T10:
				case T11:
					ii = m - 10; break;
				case T14:
					ii = 5; break;
				case T15:
					ii = 8; break;
				case T16:
					ii = 11; break;
				case T17:
					ii = 14; break;
				case T18:
					ii = 15; break;
				case T19:
					ii = 16; break;
				case T20:
					ii = 19;
					if(p < 0) {
						ii = 20; p = - p;
						}
					break;
				case T21:
					ii = 29; break;
				case T22:
					ii = 30; break;
				case T23:
					ii = 31; break;
				case T24:
					ii = 32; break;
				case T26:
					ii = 33; break;
				case T27:
					ii = 34; break;
				case T28:
					ii = 35; break;
				case T29:
					ii = 36; break;
				case T30:
					ii = 40; break;
				case T31:
					ii = 41; break;
				case T32:
					ii = 43; break;
				case T33:
					ii = 45; break;
				case T34:
					ii = 46; break;
				case T35:
					ii = 44; break;
				case T36:
					ii = 47; break;
				case T38:
					ii = 56; break;
				case T39:
					ii = 57; break;
				case T41:
					ii = 59; break;
				case T42:
					ii = 60; break;
				}
			sprintf(line,"%s(",*((*p_PerformanceControl)[ii]));
			if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
					(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
				goto OUT;
				}
			if(p >= 128 && AcceptControl(m)) {
				Reformat(wind,-1,-1,-1,&Color[ControlC],NO,NO);
				sprintf(line,"K%ld",(long)(p-128L));
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
						(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
					goto OUT;
					}
				if(ParamValue[p-128] != INT_MAX) {
					Reformat(wind,-1,-1,-1,&Black,NO,NO);
					sprintf(line,"=%ld",(long)ParamValue[p-128]);
					if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,
							(tokenbyte)0,(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
						goto OUT;
						}
					}
				}
			else {
				Reformat(wind,-1,-1,-1,&Black,NO,NO);
				switch(m) {
					case T17:
					case T18:
						sprintf(line,"%ld,%ld",(long)(p%128),(long)((p-p%128)/128));
						break;
					case T26:
						if(((p / 100) * 100) != p)
							sprintf(line,"%.2f",((double) p) / 100.);
						else
							sprintf(line,"%ld",(long) p / 100);
						break;
					case T32:
						for(ii=0; ii < Jinstr; ii++) {
							if((*p_CsInstrumentIndex)[ii] == p) break;
							}
						if(ii < Jinstr && (*pp_CsInstrumentName)[ii] != NULL
									&& (*((*pp_CsInstrumentName)[ii]))[0] != '\0') {
							MystrcpyHandleToString(MAXLIN,0,line,(*pp_CsInstrumentName)[ii]);
							}
						else sprintf(line,"%ld",(long)p);
						break;
					case T33:
					case T34:
					case T36:
						if(p_StringConstant == NULL) {
							if(Beta) Alert1("Err PrintArgSub(). p_StringConstant == NULL");
							sprintf(line,"???");
							}
						else MystrcpyHandleToString(MAXLIN,0,line,(*p_StringConstant)[p]);
						break;
					case T35:
						ii = p % 256;
						if(p_StringConstant == NULL) {
							if(Beta) Alert1("Err PrintArgSub(). p_StringConstant == NULL");
							sprintf(line,"???");
							}
						else MystrcpyHandleToString(MAXLIN,0,line,(*p_StringConstant)[ii]);
						break;
					default:
						sprintf(line,"%ld",(long)p);
						break;
					}
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
						(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
					goto OUT;
					}
				}
			Reformat(wind,-1,-1,-1,&Blue,NO,NO);
			if(m != T35) {
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
						(tokenbyte)0,nocode,pp_b,p_ib,f,th,")",NULL,-1)) != OK) {
					goto OUT;
					}
				}
			else {
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
						(tokenbyte)0,nocode,pp_b,p_ib,f,th,",",NULL,-1)) != OK) {
					goto OUT;
					}
				}
			Reformat(wind,-1,-1,-1,&Black,NO,NO);
			if(m == T35) {
SHOWNUMBER:
				ii = (p - (p % 256)) / 256;
				if(p_NumberConstant == NULL) {
					if(Beta) Alert1("Err PrintArgSub(). p_NumberConstant == NULL");
					sprintf(line,"???");
					}
				else sprintf(line,"%.4f",(*p_NumberConstant)[ii]);
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
						(tokenbyte)0,nocode,pp_b,p_ib,f,th,line,NULL,-1)) != OK) {
					goto OUT;
					}
				Reformat(wind,-1,-1,-1,&Blue,NO,NO);
				if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,(tokenbyte)0,
						(tokenbyte)0,nocode,pp_b,p_ib,f,th,")",NULL,-1)) != OK) {
					goto OUT;
					}
				Reformat(wind,-1,-1,-1,&Black,NO,NO);
				}
			sp = 1;
			}
		continue;
		}
		
	if(!nocode && m == T6) {
		if(Space(f,th,&sp) != OK) {
			r = ABORT; goto OUT;
			}
		Reformat(wind,-1,-1,-1,&Magenta,NO,NO);
		if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
				nocode,pp_b,p_ib,f,th,"?%ld",NULL,p)) != OK) {
			goto OUT;
			}
		Reformat(wind,-1,-1,-1,&Black,NO,NO);
		sp = 1;
		}
	else {	/* Unknown token */
		if(!nocode) if(Space(f,th,&sp) != OK) {
			r = ABORT; goto OUT;
			}
		if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
				nocode,pp_b,p_ib,f,th,"%ld.",NULL,m)) != OK) {
			goto OUT;
			}
		if((r=Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
				nocode,pp_b,p_ib,f,th,"%ld ",NULL,p)) != OK) {
			goto OUT;
			}
		Display('\0',nhomo,levpar,homoname,depth,p_maxib,pp_a,&i,istemplate,m,p,
			nocode,pp_b,p_ib,f,th," ???",NULL,-1);
		break;
		}
	}
	
OUT:
if(nocode) MyDisposeHandle((Handle*)&p_sequence);
return(r);
}


long SearchOrigin(tokenbyte ***pp_a,long ia,tokenbyte p)
/* Find beginning of master referenced by n */
{
long i;
tokenbyte q;

for(i=ia-6,q=0; i > 0; i-=2) {
	if((**pp_a)[i] == T2 && (**pp_a)[i+1] == 0 && ++q == p) return(i+2);
	}
ShowError(17,0,0);
return(-1);
}


Space(FILE *f,TextHandle th,int *p_sp)
{
if(*p_sp) {
	if(OutChar(f,th,' ') != OK) return(ABORT);
	*p_sp = 0;
	}
return(OK);
}


PrintPeriod(FILE* f,TextHandle th)
{
if(UseBullet) {
	if(SplitTimeObjects) {
		 if(OutChar(f,th,' ') != OK) return(ABORT);
		 if(OutChar(f,th,'¥') != OK) return(ABORT);
		 if(OutChar(f,th,' ') != OK) return(ABORT);
		}
	else {
		 if(OutChar(f,th,'¥') != OK) return(ABORT);
		}
	}
else {
	 if(OutChar(f,th,'.') != OK) return(ABORT);
	}
return(OK);
}


OutChar(FILE* f,TextHandle th,char c)
{
char s[2];

s[0] = c;
s[1] = '\0';
if(f == stdout) {
	if(th == NULL) {
		if(Beta) Alert1("Err. OutChar()");
		return(ABORT);
		}
	else /* DoKey(c,0,th); */ TextInsert(s,1L,th);
	}
else {
	if(f != NULL) fprintf(f,"%s",s);
	}
return(OK);
}


Display(char thechar,int nhomo,int levpar,int* homoname,int* depth,unsigned long *p_maxib,
	tokenbyte ***pp_a,
	unsigned long *p_i,int istemplate,tokenbyte m,tokenbyte p,int nocode,
	tokenbyte ***pp_b,unsigned long *p_ib,FILE *f,TextHandle th,char *format,char **p_smthing,
	long integ)
{
char s[255];
tokenbyte mm,pp,**ptr;
int n,h,px,py;

if(nocode) {
	if(!istemplate) {
		if((*p_ib) > ((*p_maxib)-16L)) {
			ptr = *pp_b;
			(*p_maxib) = ((*p_maxib) * 3L) / 2L;
			if((ptr = (tokenbyte**) IncreaseSpace((Handle)ptr)) != NULL) {
				(*pp_b) = ptr;
				}
			else {
				(**pp_b)[(*p_ib)++] = TEND; (**pp_b)[(*p_ib)++] = TEND;
				return(FAILED);
				}
			}
		(**pp_b)[(*p_ib)++] = m;
		(**pp_b)[(*p_ib)++] = p;
		}
	else {
		/* pp_a is istemplate, pp_b is item without structure */
POSITION:
		if((**pp_b)[*p_ib] == T0 && (**pp_b)[(*p_ib)+1] == 7) {
			(*p_ib) += 2;		/* Skip '¥' */
			goto POSITION;
			}
		if((**pp_b)[*p_ib] == TEND && (**pp_b)[(*p_ib)+1] == TEND) return(FAILED);
		if(m == T3 && p == 0) {	/* '_' */
			if((**pp_b)[*p_ib] != T3 && (**pp_b)[*p_ib] != T25) return(FAILED);
			mm = (**pp_b)[*p_ib];
			pp = (**pp_b)[(*p_ib)+1];
			if(mm == T25) pp += 16384;
			if(nhomo > 0 && levpar > 0) {
				/* Search for original terminal symbol */
				for(px=2; px < Jbol; px++) {
					py = px;
					for(n=nhomo; n >= 1; n--) {
						h = homoname[n];
						if(depth[n] < levpar) {
							py = Image(h,py);
							}
						}
					if(pp == py) {
						pp = px;
						mm = T3;
						if(pp >= 16384) {
							mm = T25;
							pp -= 16384;
							}
						goto FOUNDIT;
						}
					}
				}
FOUNDIT:
			if(mm == T25 && pp >= 16384) pp -= 16384;
			(**pp_a)[*p_i] = (tokenbyte) mm;
			(**pp_a)[(*p_i)+1] = (tokenbyte) pp;
			(*p_ib) += 2;
			return(OK);
			}
		if(m == T1 || m == T0 || m == T5 || m == T3 || m == T25 || m == T43) {
		/* m = T3 or T25 while checking slave bracket */
			if(m != (**pp_b)[*p_ib] || p != (**pp_b)[(*p_ib)+1]) return(FAILED);
			(*p_ib) += 2;
			}
		}
	}
else {
	if(m == T25) {
		PrintNote(p,0,-1,s);
		goto PRINT;
		}
	if(integ == -1) {
		if(p_smthing != NULL) {
			sprintf(s,format,*p_smthing);
			}
		else {
			if(thechar != '\0') {
				s[0] = thechar; s[1] = '\0';
				}
			else sprintf(s,format);
			}
		}
	else {
		sprintf(s,format,(long)integ);
		}
PRINT:
	if(f == stdout) {
		if(th == NULL) {
			if(Beta) Alert1("Err. Display");
			return(ABORT);
			}
		TextInsert(s,(long)strlen(s),th);
		}
	else fprintf(f,s);
	}
return(OK);
}


ShowCodes(int j)
{
int w,ch;
long i;

if(j < 2 || j >= Jbol) return(FAILED);
ClearWindow(NO,wNotice);
if((*p_MIDIsize)[j] > ZERO) {
	if((*((*pp_MIDIcode)[j]))[0].time > ZERO) {
		sprintf(Message,"Silence=%ldms ",(long)(*((*pp_MIDIcode)[j]))[0].time);
		PrintBehind(wNotice,Message);
		}
	for(i=0; i < (*p_MIDIsize)[j]; i++) {
		w = (*((*pp_MIDIcode)[j]))[i].byte;
		if(ChannelEvent(w)) {
			ch = w % 16;
			w -= ch;
			ch++;
			}
		if(w == NoteOn && (*((*pp_MIDIcode)[j]))[i+2].byte > 0) {
			sprintf(Message,"NoteOn#%ld[%ld] ",(long)(*((*pp_MIDIcode)[j]))[i+1].byte,(long)ch);
			PrintBehind(wNotice,Message); i+= 2;
			}
		else {
			if(w == NoteOff || (w == NoteOn && (*((*pp_MIDIcode)[j]))[i+2].byte == 0)) {
				sprintf(Message,"NoteOff#%ld[%ld] ",(long)(*((*pp_MIDIcode)[j]))[i+1].byte,(long)ch);
				PrintBehind(wNotice,Message);
				i+= 2;
				}
			else {
				if(w > 127) {
					switch(w) {
						case ChannelPressure:
							sprintf(Message,"ChPress=%ld[%ld] ",
								(long)(*((*pp_MIDIcode)[j]))[i+1].byte,(long)ch);
							PrintBehind(wNotice,Message);
							i += 1; break;
						case PitchBend:
							sprintf(Message,"Bend=%ld[%ld] ",(long)((*((*pp_MIDIcode)[j]))[i+1].byte
								+ 128 * (*((*pp_MIDIcode)[j]))[i+2].byte,(long)ch));
							PrintBehind(wNotice,Message);
							i += 2; break;
						case ProgramChange:
							sprintf(Message,"Progr#%ld[%ld] ",
								(long)(*((*pp_MIDIcode)[j]))[i+1].byte + ProgNrFrom,(long)ch);
							PrintBehind(wNotice,Message);
							i += 1;
							break;
						case TimingClock:
							sprintf(Message,"Silence %ldms ",(long)(*((*pp_MIDIcode)[j]))[i].time);
							if(i == (*p_MIDIsize)[j]-1 && i > 0) PrintBehind(wNotice,Message);
							break;
						case ControlChange:
							if((*((*pp_MIDIcode)[j]))[i+1].byte == 123
													&& (*((*pp_MIDIcode)[j]))[i+2].byte == 0) {
								sprintf(Message,"AllNotesOff[%ld] ",(long)ch);
								PrintBehind(wNotice,Message);
								i += 2; break;
								}
							if((*((*pp_MIDIcode)[j]))[i+1].byte == 1 && (*((*pp_MIDIcode)[j]))[i+4].byte == 33) {
								sprintf(Message,"Mod=%ld[%ld] ",(long)(128L * (*((*pp_MIDIcode)[j]))[i+2].byte
									+ (*((*pp_MIDIcode)[j]))[i+5].byte,(long)ch));
								PrintBehind(wNotice,Message);
								i += 5; break;
								}
							else {
								sprintf(Message,"Ctrl#%ld=%ld[%ld] ",
									(long)(*((*pp_MIDIcode)[j]))[i+1].byte,
									(long)(*((*pp_MIDIcode)[j]))[i+2].byte,(long)ch);
								PrintBehind(wNotice,Message);
								i += 2; break;
								}
							/* Here we fall into default */
						default:
							sprintf(Message,"<%ld> ",(long)w);
							PrintBehind(wNotice,Message);
							break;																 
						}
					}
				else {
					sprintf(Message,"%ld ",(long)w);
					PrintBehind(wNotice,Message);
					}
				}
			}
		}
	Print(wNotice,"\n");
	}
else Print(wNotice,"This object is empty");
SetSelect(ZERO,ZERO,TEH[wNotice]);
ShowSelect(CENTRE,wNotice);
return(OK);
}


AcceptControl(tokenbyte m)
{
switch(m) {
	case T15:
	case T14:
	case T17:
	case T16:
	case T18:
	case T20:
	case T21:
	case T22:
	case T23:
	case T24:
	case T26:
	case T27:
	case T28:
	case T30:
	case T32:
	case T33:
	case T34:
	case T35:
	case T36:
	case T41:
	case T42:
		return(NO);
		break;
	}
return(YES);
}


CheckPeriodOrLine(int print_periods,int *p_newline,int *p_newsection,FILE *f,
	TextHandle th,int *p_beat,unsigned long numberprolongations,int *p_sp)
{
int r;

r = OK;
if(*p_newline) {
	if(print_periods > 2) {
		(*p_beat)++;
		if((r=PrintPeriod(f,th)) != OK) goto OUT;
		if(OutChar(f,th,'Â') != OK || OutChar(f,th,'\n') != OK) {
			r = ABORT; goto OUT;
			}
		(*p_sp) = 0;
		}
	(*p_newline) = FALSE;
	}
if(*p_newsection) {
	if(print_periods) {
		(*p_beat)++;
		if((r=PrintPeriod(f,th)) != OK) goto OUT;
		(*p_sp) = 0;
		}
	else if(numberprolongations > 1L) (*p_sp) = 1;
	(*p_newsection) = FALSE;
	}
	
OUT:
return(r);
}
