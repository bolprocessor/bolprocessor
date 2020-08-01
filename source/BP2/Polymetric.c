/* Polymetric.c (BP2 version CVS) */

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


PolyMake(tokenbyte ***pp_a,double *p_maxseq,int notrailing)
{
tokenbyte m,p,**p_b,**ptr;
char fixtempo,useful,toocomplex,alreadychangedquantize;
int k,krep,rep,level,r,**p_nseq,**p_nseqmax,maxlevel,needalphabet,foundinit,overflow,toofast,
	numberouttimeinseq,longestseqouttime,numbertoofast,longestnumbertoofast,morelines;
double P,Q,tempo,tempomax,prodtempo,fmaxseq,nsymb,lcm,kpress,thelimit,speed,scale,s,
	limit1,limit2,imax,x,firstscale,scalespeed,maxscalespeed;
unsigned long i,maxid,pos_init,gcd,numberprolongations;
long newquantize,newquantize2,totalbytes,a,b;

if(CheckEmergency() != OK) return(ABORT);

r = FAILED;
Pduration = 0.;
Qduration = 1.;
POLYconvert = OkShowExpand = FALSE;
Ratio = Prod = 1.;
Kpress = 1.;
Maxevent = 3L;
(*p_maxseq) = 10.;

p_b = NULL;
p_nseq = p_nseqmax = NULL;
if(*pp_a == NULL) {
	if(Beta) Alert1("Err. PolyMake(). *pp_a = NULL");
	return(ABORT);
	}
	
maxid = (MyGetHandleSize((Handle)*pp_a) / sizeof(tokenbyte));
if(maxid <= 2) {
	return(FAILED);
	}
if(maxid < 30) maxid = 30;
maxid += 20;
if((p_b = (tokenbyte**) GiveSpace((Size)(maxid * sizeof(tokenbyte)))) == NULL)
	return(ABORT);
if(MySetHandleSize((Handle*)pp_a,(Size)(maxid * sizeof(tokenbyte))) != OK)
	return(ABORT);

PolyOn = TRUE;

if(ShowMessages) ShowMessage(TRUE,wMessage,"Interpreting structure...");

if(PrintArg(FALSE,FALSE,FALSE,FALSE,FALSE,TRUE,stdout,wData,&p_b,pp_a) != OK) goto QUIT;
	/* stdout and TEH are useless */
	/* item is now in B[] without structure */

//  Size of p_b may have been increased.  Force *pp_a to have the same size
maxid = (MyGetHandleSize((Handle)p_b) / sizeof(tokenbyte));
if(MySetHandleSize((Handle*)pp_a,(Size)(maxid * sizeof(tokenbyte))) != OK)
	return(ABORT);
maxid -= 16;
	
// Calculate sizes of arrays and check {} balance

maxlevel = 2;	/* Otherwise *3/2 will not change it. */
NeedZouleb = 0;
for(i=ZERO,level=0; (*p_b)[i] != TEND || (*p_b)[i+1] != TEND; i+=2L) {
	m = (*p_b)[i]; p = (*p_b)[i+1];
	if((m == T12 && (p == 21 || p == 22 || p == 24)) || m == T39)
		NeedZouleb++;
	if(m == T0 && (p == 12 || p == 22)) level++;
	if(m == T0 && (p == 13 || p == 23)) {
		if(level > maxlevel) maxlevel = level;
		level--;
		}
	}
if(level != 0) {	/* '{' and '}' not balanced */
	sprintf(Message,"Incorrect polymetric expression(s): '{' and '}' are not balanced. Can't proceed further...");
	if(ScriptExecOn) Println(wTrace,Message);
	else Alert1(Message);
	goto QUIT;
	}

pos_init = ZERO;
level = 0;
if(NeedZouleb > 0) {
	if(ShowMessages) ShowMessage(TRUE,wMessage,"Applying serial tools...");
	do {
		r = Zouleb(&p_b,&level,&pos_init,FALSE,FALSE,0,FALSE,FALSE,NOSEED);
		if(r != OK) goto QUIT;
		}
	while(level >= 0);
	maxlevel++;	/* A pair of brackets {} may have been created around a sound-object at the deepest level */
	if(ShowMessages) HideWindow(Window[wMessage]);
	}

if(Beta && NeedZouleb > 0) {
	sprintf(Message,"NeedZouleb = %ld in polymetric expression",(long)NeedZouleb);
	Println(wTrace,Message);
	}

if(ShowMessages) ShowMessage(TRUE,wMessage,"Expanding polymetric expression...");

needalphabet = FALSE;

for(i=ZERO,level=0; ; i+=2L) {
	m = (*p_b)[i]; p = (*p_b)[i+1];
	if(m == TEND && p == TEND) break;
	if(m == T3) {
		if(p > 1) needalphabet = TRUE; 
		continue;
		}
	if(m == T7 && p < 16384) needalphabet = TRUE;
	if(m != T0) continue;
	if(p == 12 || p == 22) {	/* '{' */
		level++;
		if(level > maxlevel) {
			if(Beta) Alert1("Err. PolyMake(). level > maxlevel");
			r = ABORT; goto QUIT;
			}
		continue;
		}
	if(p == 13 || p == 23) {	/* '}' */
		if(level < 1) {
			if(Beta) Alert1("Err. PolyMake(). {} not balanced");
			r = ABORT; goto QUIT;
			}
		level--;
		continue;
		}
	}
Maxconc += 1;
if(needalphabet && !ObjectMode) {
	if(!NeedAlphabet) {
		NeedAlphabet = TRUE;
		ObjectMode = ObjectTry = FALSE;
		}
	}

// Copy and complete init section
pos_init = ZERO;
foundinit = FALSE;
for(i=ZERO; ; i+=2L) {
	m = (*p_b)[i];
	p = (*p_b)[i+1];
	if(m == TEND && p == TEND) break;
	(**pp_a)[i] = (tokenbyte) m;
	(**pp_a)[i+1] = (tokenbyte) p;
	if(m == T1) continue;
	if(m == T0 && p == 3) {	/* '+' */
		foundinit = TRUE;
		continue;
		}
	if(m == T0 && p == 11) { /* '/' */
		pos_init = i;
		break;
		}
	if(foundinit) {
		/* Insert '/1' speed marker because none has been found. Otherwise, the initialÉ */
		/* É section numbers might be wrongly interpreted */
		pos_init = i;
		(**pp_a)[i] = T0; (**pp_a)[i+1] = 11; /* '/' */
		i += 2;
		(**pp_a)[i] = T1; (**pp_a)[i+1] = 0;
		i += 2;
		(**pp_a)[i] = T1; (**pp_a)[i+1] = 1;
		}
	break;
	}
fixtempo = useful = FALSE;
P = 0L;  Q = 1L;

//////////////////////////////////
if((r=PolyExpand(p_b,pp_a,pos_init,&maxid,&pos_init,&P,&Q,1.,&fixtempo,&useful,1.,notrailing))
	!= OK && r != SINGLE && r != IMBEDDED) goto QUIT;
//////////////////////////////////

r = OK;
scale = speed = tempo = 1.;
firstscale = 0.;
level = 0;

// Calculate firstscale

for(i=ZERO; ; i+=2L) {
	m = (**pp_a)[i]; p = (**pp_a)[i+1];
	if(m == TEND && p == TEND) break;
	if(m == T3 || m == T25 || m == T9) {
		if(firstscale == 0.) {
			firstscale = scale;
			break;
			}
		}
	if(m == T0) {
		switch(p) {
			case 21:	/* '*' scale up */
				scale = GetScalingValue((*pp_a),i);
				i += 4L;
				break;
			case 24:	/* '**' scale down */
				scale = GetScalingValue((*pp_a),i);
				scale = 1. / scale;
				i += 4L;
				break;
			}
		}
	}

// Calculate Prod

if(firstscale == 0.) firstscale = 1.;
Prod = LCM(firstscale,Q,&overflow);
toocomplex = FALSE;
scale = speed = tempo = 1.;
level = 0;
maxscalespeed = 0.;

for(i=ZERO; ; i+=2L) {
	m = (**pp_a)[i]; p = (**pp_a)[i+1];
	if(m == TEND && p == TEND) break;
	if(m == T0) {
		switch(p) {
			case 11: /* '/' speed up */
				speed = firstscale * GetScalingValue((*pp_a),i);
				scalespeed = scale / speed;
				if(maxscalespeed < scalespeed) maxscalespeed = scalespeed;
				if(!toocomplex) {
					gcd = GCD(speed,scale);
					if(gcd < 1.) gcd = 1.;
					tempo = speed / gcd;
					lcm = LCM(Prod,tempo,&overflow);
					if(lcm > 0.) Prod = lcm;
					else {
						TellComplex();
						toocomplex = TRUE;
						}
					}
				i += 4L;
				break;
			case 25:	/* '\' speed down */
				speed = firstscale / GetScalingValue((*pp_a),i);
				scalespeed = scale / speed;
				if(maxscalespeed < scalespeed) maxscalespeed = scalespeed;
				if(!toocomplex) {
					if(scale != 0.) {
						tempo = speed / scale;
						}
					else tempo = 0.;
					if(tempo > 1.) {
						lcm = LCM(Prod,tempo,&overflow);
						if(lcm > 0.) Prod = lcm;
						else {
							TellComplex();
							toocomplex = TRUE;
							}
						}
					}
				i += 4L;
				break;
			case 21:	/* '*' scale up */
				scale = GetScalingValue((*pp_a),i);
				scalespeed = scale / speed;
				if(maxscalespeed < scalespeed) maxscalespeed = scalespeed;
				i += 4L;
				break;
			case 24:	/* '**' scale down */
				scale = GetScalingValue((*pp_a),i);
				scale = 1. / scale;
				scalespeed = scale / speed;
				if(maxscalespeed < scalespeed) maxscalespeed = scalespeed;
				i += 4L;
				break;
			}
		}
	}

PRODOK:

numberprolongations = (Prod * maxscalespeed) / firstscale;
POLYconvert = TRUE;
Ratio = Prod;
Pduration = P;
Qduration = Q;

	/* if(ShowMessages)  {
	if(Qduration > 1.)
		sprintf(Message,"Duration = %.0f/%.0f time units",Pduration,Qduration);
	else sprintf(Message,"Duration = %.0f time units",Pduration);
	ShowMessage(TRUE,wMessage,Message);
	} */

alreadychangedquantize = FALSE;

// Calculate compression rate Kpress for quantization
FINDCOMPRESSION:
Kpress = 1.;
if(Pclock > 0.) {
	kpress = 1. + (((double)Quantization) * Qclock * Ratio) / Pclock / 1000.;
	kpress = floor(1.00001 * kpress);
	if(QuantizeOK && kpress > 1.) {
		if(kpress > 1000000.) {
			Ratio = Ratio * 1000000. / kpress;
			kpress = 1000000.;
			TellComplex();
			}
		else {
			s = (LCM(Ratio,kpress,&overflow) / Ratio);	/* 21/6/98 This solved a major accuracy problem! */
			if(s > 1. && s < 10. && Ratio < 1000000.) Ratio = Round(s * Ratio);
			}
		s = Round(Ratio / kpress);
		if(s > 10.) Ratio = kpress * s;
		Prod = Ratio;
		}
	if(QuantizeOK) Kpress = kpress;
	else {
		if((kpress >= 4. && NotSaidKpress) || kpress >= 100.) {
			NotSaidKpress = FALSE;
			BPActivateWindow(SLOW,wTimeAccuracy);
			if(kpress < 100.)
				sprintf(Message,"This item is quite complex. Quantization is recommended (compression rate > %u). Set it to %ldms",
					(unsigned long)kpress,Quantization);
			else
				sprintf(Message,"This item is too complex. Quantization is unavoidable. Set it to %ldms",
					Quantization);
			r = Answer(Message,'Y');
			if(r == CANCEL) {
				NotSaidKpress = TRUE;
				rep = Answer("Do you really want to abort this job",'N');
				if(rep == YES) goto QUIT;
				else goto FINDCOMPRESSION;
				}
			if(r == YES) {
				QuantizeOK = TRUE;
#if BP_CARBON_GUI
				UpdateDirty(TRUE,iSettings);
				SetTimeAccuracy();
				BPUpdateDialog(gpDialogs[wTimeAccuracy]);
#endif /* BP_CARBON_GUI */
				goto FINDCOMPRESSION;
				}
			r = OK;
			}
		}
	}


// Calculate Maxevent, (*p_maxseq) and the final value of Maxconc

if(Beta && p_nseq != NULL) {
	Alert1("Err. PolyMake(). p_nseq != NULL");
	}
if(Beta && p_nseqmax != NULL) {
	Alert1("Err. PolyMake(). p_nseqmax != NULL");
	}
if((p_nseq = (int**) GiveSpace((Size)((maxlevel+1)*sizeof(int)))) == NULL) {
	r = ABORT; goto QUIT;
	}
if((p_nseqmax = (int**) GiveSpace((Size)((maxlevel+1)*sizeof(int)))) == NULL) {
	r = ABORT; goto QUIT;
	}
(*p_nseq)[0] = (*p_nseqmax)[0] = 0;
for(level=0; level <= maxlevel; level++) (*p_nseqmax)[level] = 0;
level = 0;
nsymb = ZERO;
fmaxseq = (*p_maxseq) = 0.;
Maxevent = 3L;
tempomax = Prod / Kpress;
Minconc = 0;
Maxlevel = 1;
longestseqouttime = numberouttimeinseq
	= numbertoofast = longestnumbertoofast = morelines = 0;
toofast = FALSE;
speed = scale = Prod / firstscale;
tempo = speed / scale;
prodtempo = Prod / tempo;
toofast = (tempo > tempomax);

for(i=ZERO; ; i+=2L) {
	m = (tokenbyte)(**pp_a)[i];
	p = (tokenbyte)(**pp_a)[i+1];
	if(m == TEND && p == TEND) break;
	if(m == T7) numberouttimeinseq++;
	else {
		if(numberouttimeinseq > 0) {
			if(m == T25 || m == T3 || (m == T0 && p == 14)) {
				if(numberouttimeinseq > longestseqouttime)
					longestseqouttime = numberouttimeinseq;
				numberouttimeinseq = 0;
				}
			}
		}
	if(m == T3 || m == T9 || m == T25) {	/* sound-object, time pattern, simple note */
		if(toofast) numbertoofast++;
		else {
			if(numbertoofast > 0) {
				if(numbertoofast > longestnumbertoofast)
					longestnumbertoofast = numbertoofast;
				numbertoofast = 0;
				}
			}
		fmaxseq += prodtempo;
		if(p >= 1 || m == T9 || m == T25) {	/* Even silence may become an event if precededÉ */
								/* É by _pitchbend() for instance. */
			Maxevent++;
			if(toofast) nsymb += prodtempo;
			else nsymb += 1.;
			}
		continue;
		}
	else {
		if(numbertoofast > 0) {
			if((!toofast) || (m == T0 && (p == 13 || p == 14 || p == 23))) {
				if(numbertoofast > longestnumbertoofast)
					longestnumbertoofast = numbertoofast;
				numbertoofast = 0;
				}
			}
		}
	switch(m) {
		case T7: /* out-time object */
		case T8: /* synchronization tag */
		case T13: /* script line */
		case T14: /* _mod() */
		case T15: /* _pitchbend() */
		case T16: /* _press() */
		case T17: /* _switchon() */
		case T18: /* _switchoff() */
		case T19: /* _volume() */
		case T20: /* _legato() */
		case T26: /* _tranpose) */
		case T29: /* _pan() */
		case T35: /* _value() */
		case T37: /* _keymap() */
			fmaxseq += 1.;
			Maxevent++;
			nsymb += 1.;
			continue;
			break;
		}
	if(m == T0) {
		switch(p) {
			case 12:	/* '{' */
			case 22:
				if(++level > maxlevel) {
					if(Beta) Alert1("Err. PolyMake(): level > maxlevel");
					r = ABORT; goto QUIT;
					}
				(*p_nseqmax)[level] = (*p_nseq)[level] = (*p_nseq)[level-1];
				if(level > Maxlevel) Maxlevel = level;
				continue;
				break;
				
			case 13:	/* '}' */
			case 23:
				if(level < 1) {
					if(Beta) Alert1("Err. PolyMake(): level < 1");
					r = ABORT; goto QUIT;
					}
				if((*p_nseqmax)[level] > (*p_nseqmax)[level-1])
					(*p_nseqmax)[level-1] = (*p_nseqmax)[level];
				level--;
				continue;
				break;
			
			case 18:	/* '&' following terminal symbol */
				morelines++;
				continue;
				break;
					
			case 14:	/* ',' */
				(*p_nseqmax)[level]++;
				(*p_nseq)[level] = (*p_nseqmax)[level];
				if((*p_nseq)[level] > Minconc) Minconc = (*p_nseq)[level];
				nsymb += 1.;
				continue;
				break;
				
			case 11:	/* '/' speed up */
				speed = GetScalingValue((*pp_a),i);
				if(scale != 0.) {
					tempo = speed / scale;
					prodtempo = Prod / tempo;
					}
				else tempo = 0.;
				toofast = (tempo > tempomax || tempo == 0.);
				i += 4;
				continue;
				break;
				
			case 25:	/* '\' speed down */
				if(Beta) FlashInfo("Speed down");
				speed = GetScalingValue((*pp_a),i);
				if(speed < 1.) {
					r = ABORT;
					goto QUIT;
					}
				speed = 1. / speed;
				if(scale != 0.) {
					tempo = speed / scale;
					prodtempo = Prod / tempo;
					}
				else tempo = 0.;
				toofast = (tempo > tempomax || tempo == 0.);
				i += 4;
				continue;
				break;
	
			case 21:		/* '*' scale up */
				scale = GetScalingValue((*pp_a),i);
				if(scale != 0.) {
					tempo = speed / scale;
					prodtempo = Prod / tempo;
					}
				else tempo = 0.;
				toofast = (tempo > tempomax || tempo == 0.);
				i += 4;
				continue;
				break;
				
			case 24:		/* '**' scale down */
				scale = GetScalingValue((*pp_a),i);
				scale = 1. / scale;
				if(scale < InvMaxTempo) scale = 0.;
				if(scale != 0.) {
					tempo = speed / scale;
					prodtempo = Prod / tempo;
					}
				else tempo = 0.;
				toofast = (tempo > tempomax || tempo == 0.);
				i += 4;
				continue;
				break;
			}
		}
	}
if((*p_nseqmax)[0] > Minconc) Minconc = (*p_nseq)[0];
Minconc++;
if(numberouttimeinseq > longestseqouttime) longestseqouttime = numberouttimeinseq;
if(numbertoofast > longestnumbertoofast) longestnumbertoofast = numbertoofast;
imax = fmaxseq / Kpress;
thelimit = ULONG_MAX - 10.;

if(imax >= thelimit) {
TOOBIG:
	if(Pclock < 1.) {
		BPActivateWindow(SLOW,wTimeAccuracy);
		BPActivateWindow(SLOW,wMetronom);
		if(!ScriptExecOn) Alert1("Quantization is required but you must set a metronom value");
		else {
			Pclock = Qclock = 1.;
			goto SETMETRONOM;
			}
		
ENTERMETRONOM:
		rep = AnswerWith("Set metronome to...","60",Message);
		if(rep == ABORT) {
			rep = Answer("Do you really want to abort this job",'N');
			if(rep == YES) {
				r = ABORT;
				goto QUIT;
				}
			goto ENTERMETRONOM;
			}
		if(Myatof(Message,&a,&b) < 0.) {
			Alert1("Incorrect value");
			goto ENTERMETRONOM;
			}
		if(Simplify((double)INT_MAX,(double)a,(double)60L*b,&Qclock,&Pclock) != OK)
			Simplify((double)INT_MAX,floor((double)a/60L),(double)b,&Qclock,&Pclock);

SETMETRONOM:
		SetTickParameters(0,MAXBEATS);
		ResetTickFlag = TRUE;
		SetTempo();
		SetTimeBase();
		SetGrammarTempo();
		}
	if(!QuantizeOK) {
		BPActivateWindow(SLOW,wTimeAccuracy);
		rep = Answer("Set 'Quantize' to true (no other way!)",'Y');
		if(rep != YES && Answer("Do you really want to abort this job",'Y') == YES) {
			r = ABORT;
			goto QUIT;
			}
		newquantize = Quantization;
		QuantizeOK = TRUE;
		goto SETQUANTIZE;
		}
	else if(Pclock > 0.) {
		x = (((double)Quantization) * (imax + 100L) / thelimit);
		x = 10L * (long) ceil(((double) x) / 10.);
		if(x < Quantization) {
			Alert1("Polymetric expansion encountered an unpredicted overflow. Sorry, can't proceed further");
			r = ABORT;
			goto QUIT;
			}
		if(x > 2000.) {
			if(Quantization > 100L) {
				rep = Answer("This item would require a large quantization (greater than 2000ms). Use temporary memory instead",
					'Y');
				if(rep == YES) {
					FixedMaxQuantization = TRUE;
					AskedTempMemory = TRUE;
					goto FORGETIT;
					}
				else {
					if(x < 32767.) newquantize = (long) x;
					else newquantize = 2000L;
					goto CHANGEQUANTIZE;
					}
				}
			else {
				rep = Answer("Increasing quantization may not be sufficient to reduce memory requirement. Try it anyway",
					'Y');
				newquantize = 2000L;
				if(rep == YES) {
					goto CHANGEQUANTIZE;
					}
				if(rep == CANCEL) goto WANTABORT;
				AskedTempMemory = TRUE;
				FixedMaxQuantization = TRUE;
				goto FORGETIT;
				}
			}
		newquantize = (long) x;
		BPActivateWindow(SLOW,wTimeAccuracy);
		sprintf(Message,"Quantization of %ldms may be increased to reduce memory requirement (%ldms will work)",
			(long) Quantization,(long) newquantize);
		if(!ScriptExecOn && !alreadychangedquantize) Alert1(Message);
		else goto SETQUANTIZE;
		
CHANGEQUANTIZE:
		sprintf(Message,"%ld",(long)newquantize);
		rep = AnswerWith("Set quantization to...",Message,Message);
		if(rep == ABORT) {

WANTABORT:
			rep = Answer("Do you want to abort this job (answer 'no' to get another option)",'Y');
			if(rep == YES) {
				r = ABORT;
				goto QUIT;
				}
			else {
				rep = Answer("Allow BP2 to use additional memory",'Y');
				if(rep == YES) {
					AskedTempMemory = TRUE;
					goto FORGETIT;
					}
				if(rep == CANCEL) {
					r = ABORT;
					goto QUIT;
					}
				goto CHANGEQUANTIZE;
				}
			}
		newquantize2 = atol(Message);
		if(newquantize2 <= Quantization) {
			rep = Answer("Are you sure you don't want to increase the current quantization",'N');
			if(rep != YES) goto CHANGEQUANTIZE;
			}
		if(newquantize2 < newquantize) FixedMaxQuantization = TRUE;
		alreadychangedquantize = TRUE;
		newquantize = newquantize2;
		
SETQUANTIZE:
		Quantization = newquantize;
#if BP_CARBON_GUI
		UpdateDirty(TRUE,iSettings);
		SetTimeAccuracy();
		BPActivateWindow(SLOW,wTimeAccuracy);
#endif /* BP_CARBON_GUI */
		MyDisposeHandle((Handle*)&p_nseq);
		MyDisposeHandle((Handle*)&p_nseqmax);
		goto FINDCOMPRESSION;
		}
	}

Minconc += morelines;
Maxlevel++;
Maxconc = Minconc + 1 + longestseqouttime + longestnumbertoofast;

CHECKSIZE:
// FIXME: This whole section needs reconsideration on OS X
/* Maximum allowed memory for the phase diagram (?) is set by kMaxPhaseDiagramSize (currently
   200 MB). That is pretty arbitrary but it is 10x the 20MB limit that MaxMem() was returning. 
   A better solution is difficult since having no upper limit can lead to extreme thrashing,
   and limiting the size to the computer's RAM does not prevent some virtual memory from 
   being used (which the user might be OK with).
   
   Would it be good to allow the user to decide an upper limit ?
   
   Also, note that BP2 chooses a compression rate for quantization above (at label 
   FINDCOMPRESSION, about line 327) before it even gets to this calculation.
      -- akozar 20130904
 */
if(!TempMemory && !AskedTempMemory && !FixedMaxQuantization
		&& (imax > 10000. || (imax * Maxconc) > 20000.)) {
	// contigbytes = MaxMem(&grow);  // MaxMem() is no longer useful (always 20 MB on OS X)
	// totalbytes = contigbytes + grow;
	totalbytes = kMaxPhaseDiagramSize;
	limit1 = totalbytes / 70L;
	limit2 = (40L * limit1) / Maxconc;
	if(limit1 < limit2) thelimit = limit1;
	else {
		thelimit = limit2;
		if(Maxconc > (Minconc + 1 + longestseqouttime)) {
			Maxconc--;
			goto CHECKSIZE;
			}
		}
	if(imax > thelimit) {
		if(!QuantizeOK || Pclock < 1.) {
			rep = Answer("Item is large. Use quantization",'Y');
			if(rep == ABORT && Answer("Do you really want to abort this job",'N') == YES) {
				r = ABORT;
				goto QUIT;
				}
			if(rep == YES) goto TOOBIG;
			else goto FORGETIT;
			}
	ASK:
		if(!alreadychangedquantize)
			rep = Answer("Item is large. Change quantization",'Y');
		else {
			rep = Answer("Quantization value must be reduced again, as the estimation proved wrong. Fix",'Y');
			if(rep == YES) goto TOOBIG;
			}
		if(rep == CANCEL) {
			rep = Answer("Do you really want to abort this job",'N');
			if(rep == YES) {
				r = ABORT;
				goto QUIT;
				}
			rep = CANCEL;
			}
		if(rep == CANCEL) goto ASK;
		if(rep == YES) goto TOOBIG;
		}
	}

FORGETIT:
// Maxconc += Maxconc;
// Maxevent += ((2 * Maxconc) + 1);
Maxevent += Maxconc + 1;
// Maxconc += 4;
(*p_maxseq) = fmaxseq + 1.;
/* Takes care of newswitch at the end of diagram */

if(ShowMessages) ShowDuration(YES);

if(Maxevent >= INT_MAX) {	// FIXME ? This comparison is never true with sizeof(long) == sizeof(int)
	sprintf(Message,"Can't create %ld sound-objects. Limit: %ld",(long)Maxevent,(long)INT_MAX);
	Alert1(Message);
	r = FAILED;
	goto QUIT;
	}
	
OkShowExpand = TRUE;	/* OK to display prolongational gaps "_" */
if(nsymb > 500L || numberprolongations > 100 || ((Prod / firstscale) > 100)) {
	if(ShowMessages) {
		if(nsymb > 500L) {
			sprintf(Message,"Expanded polymetric expression would contain %ld symbols...",(long)nsymb);
			FlashInfo(Message);
			}
		if(ExpandOn) ShowMessage(TRUE,wMessage,"Expanded expression is too large for display");
		}
	OkShowExpand = FALSE;
	}
r = OK;

QUIT:
MyDisposeHandle((Handle*)&p_b);
MyDisposeHandle((Handle*)&p_nseq);
MyDisposeHandle((Handle*)&p_nseqmax);
PolyOn = FALSE;

if(r == IMBEDDED) r = OK;
return(r);
}


PolyExpand(tokenbyte **p_b,tokenbyte ***pp_a,unsigned long idorg,unsigned long *p_maxid,
	unsigned long *p_pos,double *p_P,double *p_Q,double oldspeed,char *p_fixtempo,
	char *p_onefielduseful,double oldscale,int notrailing)
{
tokenbyte m,p,****pp_c,**p_e,**ptr;
char xf,useful,**ptr_fixtempo,**p_firstistempo,**p_empty,**p_useful,
	foundtokens,toobigitem,truebracket,space,forceshowtempo;
int gr,k,kk,kmax,a,a0,a1,result,r,tempomark,fixlength,dirtymem,sign,isequal,
	singlegap,newg,newh,just_fill_gap,restart,comma,period,overflow,imbedded,
	compiledmem,firstistempo,**p_vargap;
double L,M,lcm,Q,**p_p,**p_q,**p_pp,**p_r,pmax,qmax,**p_pgap,**p_qgap,xp,xq,ss,x,y,t,
	speed,speedbeforegap,scalebeforegap,s,mm,scalegap,scaleup,scaledown,rescale,
	mgap,approxduration,rescalesubstructure,scale,prevscale,prevspeed;
long level;
unsigned long i,j,jmax,gcd,g,h,lastbyte,oldpos,ic,id,**p_maxic,useless,ptempo,qtempo;

if(ShowMessages && Beta && 0) {
	sprintf(Message,"Expanding polymetric expression [position %ul]...",(*p_pos));
	ShowMessage(TRUE,wMessage,Message);
	}
PleaseWait();
result = ABORT; compiledmem = CompiledGr;

imbedded = TRUE;

/* Calculate k, the number of arguments */
k = 1; level = ZERO;
comma = period = FALSE;
for(i=(*p_pos);
	(level >= ZERO) && ((m = (*p_b)[i]) != TEND || (*p_b)[i+1] != TEND); i += 2L) {
	if(m != T0) continue;
	p = (*p_b)[i+1];
	switch(p) {
		case 22:
		case 12:	/* '{' */
			level++;
			break;
		case 23:
		case 13:	/* '}' */
			level--;
			break;
		case 7:		/* '¥' */
			if(level == ZERO) {
				period = TRUE; k++;
				}
			break;
		case 14:	/* ',' */
			if(level == ZERO) {
				comma = TRUE; k++;
				}
			break;
		default: break;
		}
	}
		
if((pp_c = (tokenbyte****) GiveSpace((Size)k * sizeof(tokenbyte**))) == NULL) return(ABORT);
if((p_maxic = (unsigned long**) GiveSpace((Size)k * sizeof(unsigned long))) == NULL)
	 return(ABORT);
for(a = 0; a < k; a++) {
	(*p_maxic)[a] = FIELDSIZE - 16L;
	ptr = (tokenbyte**) GiveSpace((Size) FIELDSIZE * sizeof(tokenbyte));
	if(ptr == NULL) return(ABORT);
	(*pp_c)[a] = ptr;
	}
if((p_e = (tokenbyte**) GiveSpace((Size) FIELDSIZE * sizeof(tokenbyte))) == NULL) goto OUT;
jmax = FIELDSIZE - 16L;

if((ptr_fixtempo = (char**) GiveSpace((Size)sizeof(char) * k)) == NULL) {
	goto OUT;
	}
if((p_vargap = (int**) GiveSpace((Size)sizeof(int) * k)) == NULL) {
	goto OUT;
	}
if((p_firstistempo = (char**) GiveSpace((Size)sizeof(char) * k)) == NULL) {
	goto OUT;
	}
if((p_empty = (char**) GiveSpace((Size)sizeof(char) * k)) == NULL) {
	goto OUT;
	}
if((p_useful = (char**) GiveSpace((Size)sizeof(char) * k)) == NULL) {
	goto OUT;
	}

if((p_p = (double**) GiveSpace((Size)sizeof(double) * k)) == NULL) {
	goto OUT;
	}
if((p_q = (double**) GiveSpace((Size)sizeof(double) * k)) == NULL) {
	goto OUT;
	}
if((p_pp = (double**) GiveSpace((Size)sizeof(double) * k)) == NULL) {
	goto OUT;
	}
if((p_r = (double**) GiveSpace((Size)sizeof(double) * k)) == NULL) {
	goto OUT;
	}
if((p_pgap = (double**) GiveSpace((Size)sizeof(double) * k)) == NULL) {
	goto OUT;
	}
if((p_qgap = (double**) GiveSpace((Size)sizeof(double) * k)) == NULL) {
	goto OUT;
	}

(*p_pgap)[0] = 0.; (*p_qgap)[0]= 1.;
oldpos = (*p_pos);
restart = FALSE;
if(period && comma) {
	sprintf(Message,"Error in polymetric expression.\nThe same expression contains both '¥' and a comma...");
	if(ScriptExecOn) Println(wTrace,Message);
	else Alert1(Message);
	result = ABORT;
	goto OUT;
	}

START:
result = OK;
a = 0;
(*p_pos) = oldpos;
(*p_p)[0] = 0L; (*p_q)[0] = 1L;
firstistempo = TRUE;
(*p_firstistempo)[0] = foundtokens = FALSE;
scalegap = 1.;

ic = ZERO;
(*p_vargap)[0] = 0;
newg = newh = singlegap = tempomark = FALSE;
(*p_empty)[0] = TRUE; (*p_useful)[0] = FALSE;
g = h = ZERO;
(*ptr_fixtempo)[0] = FALSE;
ptempo = qtempo = 1L;

prevscale = scale = oldscale;
prevspeed = speed = oldspeed;
if(speed > TokenLimit || (1./speed) > TokenLimit) {
	Alert1("Unexpected overflow in polymetric formula (case 14). You may send this item to the designers...");
	result = ABORT; goto OUT;
	}

just_fill_gap = FALSE;
		
for(i = (*p_pos); (m = (*p_b)[i]) != TEND || (*p_b)[i+1] != TEND; i+=2L) {
#if BP_CARBON_GUI
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	// This block is very similar to the middle of InterruptTimeSet() and to code in
	// SetTimeObjects().  Could we refactor the shared code into a function ?? 
	// (or move this if block into a function InterruptExpand()) ?? -- akozar, 20130830
	if((i % 100L) == ZERO && (result=MyButton(1)) != FAILED) {
		StopCount(0);
		SetButtons(TRUE);
		Interrupted = TRUE;
		dirtymem = Dirty[wAlphabet]; Dirty[wAlphabet] = FALSE;
		compiledmem = CompiledGr;
		if(result == OK)
			while((result = MainEvent()) != RESUME && result != STOP && result != EXIT);
		if(result == EXIT) goto OUT;
		if(Nw >= 0 && Editable[Nw]) {
			lastbyte =  GetTextLength(Nw);
			SetSelect(lastbyte,lastbyte,TEH[Nw]);
			}
		if(Dirty[wAlphabet]) {
			Alert1("Alphabet changed. Must recompile...");
			result = ABORT; goto OUT;
			}
		Dirty[wAlphabet] = dirtymem;
		if(result == STOP || (compiledmem && !CompiledGr)) {
			result = ABORT; goto OUT;
			}
		if(LoadedIn && (!CompiledIn && (result=CompileInteraction()) != OK))
			goto OUT;
		if(OutMIDI && Dirty[wTimeAccuracy]) {
#if WITH_REAL_TIME_MIDI
			result = ResetMIDI(FALSE);
			if(result == ABORT || result == EXIT) goto OUT;
			if((result=CheckSettings()) == ABORT) goto OUT;
			else {
				Dirty[wTimeAccuracy] = FALSE; result = AGAIN;
				}
#endif
			goto OUT;
			}
		}
	result = OK;
	if(EventState != NO) {
		result = EventState; goto OUT;
		}
	if(Panic) {
		result = ABORT; goto OUT;
		}
#endif /* BP_CARBON_GUI */
	p = (*p_b)[i+1];
	if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
		result = ABORT; goto OUT;
		}
 	if(m == T0 && p == 21) {  /* '*' scale up */
 		scale = 0.;
 		do {
 			i += 2;
 			m = (*p_b)[i];
 			p = (*p_b)[i+1];
 			if(m == T1) scale = (TOKBASE * scale) + p;
			}
		while(m == T1);
		i -= 2;
		continue;
 		}
 	if(m == T0 && p == 24) {  /* '**' scale down */
 		s = 0.;
 		do {
 			i += 2;
 			m = (*p_b)[i];
 			p = (*p_b)[i+1];
 			if(m == T1) s = (TOKBASE * s) + p;
			}
		while(m == T1);
		if(s != 0.) scale = 1. / s;
		else if(Beta) Alert1("Err. PolyExpand() after '**'. s = 0.");
		i -= 2;
		continue;
 		}
 	if(m == T0 && p == 25) {  /* '\' speed down */
 		s = 0.;
 		do {
 			i += 2;
 			m = (*p_b)[i];
 			p = (*p_b)[i+1];
 			if(m == T1) s = (TOKBASE * s) + p;
			}
		while(m == T1);
		if(s != 0.) speed = 1. / s;
		else if(Beta) Alert1("Err. PolyExpand() after '\\'. s = 0.");
		i -= 2;
		continue;
 		}
 	if(m == T0 && p == 11 && !tempomark) {	/* '/' speed up */
 		tempomark = TRUE;
 		foundtokens = TRUE;
 		firstistempo = (*p_firstistempo)[a] = FALSE;
 		continue;
 		}
 	if(m == T1) {							/* digit */
 		firstistempo = FALSE;
 		foundtokens = TRUE;
 		if(tempomark) {						/* changing tempo */
			h = (TOKBASE * h) + p;
			if(!newh) {
				newh = TRUE;
				continue;
				}
			else {
				tempomark = FALSE; 		/* two bytes have been read */
				/* process 'k' (and perhaps 'g') */
				}
			}
		else {								/* fractional silence */
			g = (TOKBASE * g) + p;
			if(!newg) {
				newg = TRUE;
				continue;		/* read next byte */
				}
			/* two bytes have been read */
			if((*p_b)[i+2] == T0 && (*p_b)[i+3] == 11) {	/* '/' */
				continue;
				}
			/* process number 'g': it is a silence. */	
			}
		}
	tempomark = FALSE;
	
FIXTEMP:
	scalegap = 1.;
	if(newh) {
		newh = FALSE;
		if(newg) {
			singlegap = TRUE;
			scalebeforegap = scale;
			speedbeforegap = speed;
			if(g > 1000.) {
				if(MakeRatio(1000.,(((double)g) / ((double)h)),&x,&y) != OK) {
					result = ABORT; goto OUT;
					}
				g = (unsigned long) x;
				h = (unsigned long) y;
				}
			speed = speed * h; /* only temporary and relative change of speed */
			}
		else {
			(*ptr_fixtempo)[a] = TRUE;
			(*p_fixtempo) = TRUE;
			speed = h; /* absolute value */
			}
			
		if(speed > MaxTempo || scale > MaxTempo) {
			MakeRatio(MaxTempo,(scale/speed),&xq,&xp);
			speed = xp;
			scale = xq;
			}
		else Simplify(MaxTempo,scale,speed,&xq,&xp);
		if(xq < InvMaxTempo) xq = 0.;
		speed = xp;
		scale = xq;
		isequal = Equal(0.005,scale,speed,prevscale,prevspeed,&overflow);
		if(isequal == ABORT) {
			if(Beta) {
				Alert1("Err. PolyExpand(). isequal == ABORT");
				result = ABORT; goto OUT;
				}
			isequal = FALSE;
			}
		if(isequal == FALSE) {
			prevscale = scale;
			prevspeed = speed;
			if(scale >= 1. || scale == 0.) {
				y = modf((scale/(double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 21;	/* '*' scale up */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) scale - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			else {
				y = modf(((1./scale)/(double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 24;	/* '**' scale down */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) (1./scale) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
				result = ABORT; goto OUT;
				}
			if(speed >= 1.) {
				y = modf((speed / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 11;	/* '/' speed up */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) speed - (((tokenbyte) x) * TOKBASE);  /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			else  {
				y = modf((1. / speed / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 25;	/* '\' speed down */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) (1./speed) - (((tokenbyte) x) * TOKBASE);  /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
				result = ABORT; goto OUT;
				}
			}
		h = ZERO;
		}
	if(newg) {
		newg = FALSE;
		(*p_empty)[a] = FALSE;
		if(g > ZERO) {
			if(Add((*p_p)[a],(*p_q)[a],(double) g * scale,speed,&xp,&xq,
					&overflow) != OK) {
				result = ABORT; goto OUT;
				}
			if(overflow) TellComplex();
			if(xp > MaxFrac || xq > MaxFrac) {
				MakeRatio(MaxFrac,(xp/xq),&xp,&xq);
				TellComplex();
				}
			else Simplify(MaxFrac,xp,xq,&xp,&xq);
			
			(*p_p)[a] = xp; (*p_q)[a] = xq;
			
			/* Replace number with '-' */
			(*((*pp_c)[a]))[ic++] = T3;
			(*((*pp_c)[a]))[ic++] = 1;	/* '-' */
			if(period) (*p_useful)[a] = TRUE;
			g--;
			while(g > ZERO) {
				if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
					result = ABORT; goto OUT;
					}
				(*((*pp_c)[a]))[ic++] = T3;
				(*((*pp_c)[a]))[ic++] = 0;	/* '_' */
				g--;
				}
			}
		/* g = ZERO now */
		if(singlegap) {
			singlegap = FALSE;
			isequal = Equal(0.005,scale,speed,scalebeforegap,speedbeforegap,&overflow);
			if(isequal == ABORT) {
				if(Beta) {
					Alert1("Err. PolyExpand(). isequal == ABORT");
					result = ABORT; goto OUT;
					}
				isequal = FALSE;
				}
			if(isequal == FALSE) {
				scale = scalebeforegap;
				speed = speedbeforegap;
				prevscale = scale;
				prevspeed = speed;
				if(scale >= 1. || scale == 0.) {
					y = modf((scale/(double)TOKBASE),&x);
					(*((*pp_c)[a]))[ic++] = T0;
					(*((*pp_c)[a]))[ic++] = 21;	/* '*' scale up */
					(*((*pp_c)[a]))[ic++] = T1;
					(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
					(*((*pp_c)[a]))[ic++] = T1;
					(*((*pp_c)[a]))[ic++] = (tokenbyte) scale - (((tokenbyte) x) * TOKBASE);  /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
					}
				else {
					y = modf(((1./scale)/(double)TOKBASE),&x);
					(*((*pp_c)[a]))[ic++] = T0;
					(*((*pp_c)[a]))[ic++] = 24;	/* '**' scale down */
					(*((*pp_c)[a]))[ic++] = T1;
					(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
					(*((*pp_c)[a]))[ic++] = T1;
					(*((*pp_c)[a]))[ic++] = (tokenbyte) (1./scale) - (((tokenbyte) x) * TOKBASE);  /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
					}
				if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
					result = ABORT; goto OUT;
					}
				if(speed >= 1.) {
					y = modf((speed / (double)TOKBASE),&x);
					(*((*pp_c)[a]))[ic++] = T0;
					(*((*pp_c)[a]))[ic++] = 11;	/* '/' speed up */
					(*((*pp_c)[a]))[ic++] = T1;
					(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
					(*((*pp_c)[a]))[ic++] = T1;
					(*((*pp_c)[a]))[ic++] = (tokenbyte) speed - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
					}
				else  {
					y = modf((1. / speed / (double)TOKBASE),&x);
					(*((*pp_c)[a]))[ic++] = T0;
					(*((*pp_c)[a]))[ic++] = 25;	/* '\' speed down */
					(*((*pp_c)[a]))[ic++] = T1;
					(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
					(*((*pp_c)[a]))[ic++] = T1;
					(*((*pp_c)[a]))[ic++] = (tokenbyte) (1./speed) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
					}
				if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
					result = ABORT; goto OUT;
					}
				}
			}
		}
	if(just_fill_gap) {
		just_fill_gap = FALSE;
		continue;
		}
	if(m == T43) {	/* _tempo() */
		foundtokens = TRUE;
		ptempo = p;
		i += 2L;
		m = (*p_b)[i];
		if(m != T43) {
			if(Beta) Alert1("Err. PolyMake(). m != T43");
			i -= 2L;
			continue;
			}
		qtempo = (*p_b)[i+1L];
		if(firstistempo) {
			(*p_firstistempo)[a] = TRUE;
			firstistempo = FALSE;
			}
		prevspeed = speed;
		prevscale = scale;
	/*	speed = speed * ptempo;
		gcd = GCD(speed,qtempo);
		if(gcd > 1L) {
			speed = speed / gcd;
			qtempo = qtempo / gcd;
			}
		scale = qtempo * scale; */
		speed = oldspeed * ptempo;
		scale = oldscale * qtempo;
		if(speed > MaxTempo || scale > MaxTempo) {
			MakeRatio(MaxTempo,(scale/speed),&xq,&xp);
			speed = xp;
			scale = xq;
			TellComplex();
			}
		else Simplify(MaxTempo,scale,speed,&xq,&xp);
		if(xq < InvMaxTempo) xq = 0.;
		speed = xp;
		scale = xq;
		
		isequal = Equal(0.005,scale,speed,prevscale,prevspeed,&overflow);
		if(isequal == ABORT) {
			if(Beta) {
				Alert1("Err. PolyExpand(). isequal == ABORT");
				result = ABORT; goto OUT;
				}
			isequal = FALSE;
			}
		if(isequal == FALSE) {
			prevscale = scale;
			prevspeed = speed;
			if(scale >= 1. || scale == 0.) {
				y = modf((scale / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 21;	/* '*' scale up */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) scale - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			else {
				y = modf(((1. / scale) / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 24;	/* '**' scale down */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) (1./scale) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
				result = ABORT; goto OUT;
				}
			if(speed >= 1.) {
				y = modf((speed / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 11; /* '/' speed up */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte)  speed - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			else {
				y = modf((1. / speed / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 25; /* '\' speed down */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte)  (1./speed) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
				result = ABORT; goto OUT;
				}
			}
		continue;
		}
	if(m == T0 && p == 17) {					/* Undetermined rest 'É' */
		firstistempo = (*p_firstistempo)[a] = FALSE;
		/* This field can't be used to determine the duration */
		if(restart) {	/* Scanning structure second time:É*/
			newh = newg = TRUE;
			xp = speed * ((*p_pgap)[a]);
			xq = (*p_qgap)[a] * (scale);
			if(xp > ULONG_MAX || xq > ULONG_MAX)  {
				if(MakeRatio(ULONG_MAX,(xq/xp),&xq,&xp) != OK) {
					result = ABORT; goto OUT;
					}
				TellComplex();
				}
			g = xp;
			h = xq;
			just_fill_gap = TRUE;
			goto FIXTEMP;
			}
		else (*p_vargap)[a]++;
		continue;
		}
	if(m == T0 && (p == 12 || p == 22)) { 			/* '{' or '|' */
		if(p == 12) truebracket = TRUE;
		else truebracket = FALSE;
		firstistempo = FALSE;
		i += 2L;
		xp = (*p_p)[a];
		xq = (*p_q)[a];
		xf = (*ptr_fixtempo)[a];
		useful = (*p_useful)[a];
		
		//// Recursive call ///////////////////
		r = PolyExpand(p_b,&p_e,ZERO,&jmax,&i,&xp,&xq,speed,&xf,&useful,scale,notrailing);
		///////////////////////////////////
		
		if(r != OK && r != SINGLE && r != EMPTY && r != IMBEDDED) {
			result = r;
			goto OUT;
			}
		
		if(r != EMPTY) {
			(*p_empty)[a] = FALSE;
			(*ptr_fixtempo)[a] = xf;
			(*p_useful)[a] = useful;
			(*p_p)[a] = xp;
			(*p_q)[a] = xq;
			
			if(r != IMBEDDED && truebracket) {
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 12;	/* '{' */
				}
			for(j=ZERO; (*p_e)[j] != TEND || (*p_e)[j+1] != TEND; j += 2L) {
				if(j > jmax) {
					if(Beta) Alert1("Err. PolyExpand() j > jmax");
					result = ABORT; goto OUT;
					}
				if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
					result = ABORT; goto OUT;
					}
				m = (*p_e)[j];
				p = (*p_e)[j+1];
				(*((*pp_c)[a]))[ic++] = m;
				(*((*pp_c)[a]))[ic++] = p;
				}
			/* Now, substructure has been copied. */
			
			if(r != IMBEDDED) {
				if(truebracket) {
					(*((*pp_c)[a]))[ic++] = T0;
					(*((*pp_c)[a]))[ic++] = 13;	/* '}' */
					}
				else imbedded = FALSE;
				}
			if(scale >= 1. || scale == 0.) {
				y = modf((scale / (double)TOKBASE),&x); 
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 21;	/* '*' scale up */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte)  scale - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			else {
				y = modf(((1. / scale) / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 24;	/* '**' scale down */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte)  (1./scale) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			prevscale = scale;
			if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
				result = ABORT; goto OUT;
				}
			if(speed >= 1.) {
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 11;	/* '/' speed up */
				y = modf((speed / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte)  speed - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			else {
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 25;	/* '\' speed down */
				y = modf((1. / speed / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte)  (1./speed) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			prevspeed = speed;
			if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
				result = ABORT; goto OUT;
				}
			}
		continue;
		}
	if(m == T0 && (p == 13 || p == 23)) {		/* '}' or '|' */
		firstistempo = FALSE;
		(*((*pp_c)[a]))[ic++] = TEND;
		(*((*pp_c)[a]))[ic++] = TEND;
		if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
			result = ABORT; goto OUT;
			}
		(*p_pos) = i;
		if(foundtokens) imbedded = FALSE;
		goto END;
		}
	if(m == T0 && (p == 14 || p == 7)) {		/* Either ',' or '¥' */
		(*((*pp_c)[a]))[ic++] = TEND;
		(*((*pp_c)[a]))[ic++] = TEND;
		if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
			result = ABORT; goto OUT;
			}
		foundtokens = TRUE;
		
		a++;	/* Next field */
		if(a >= k) {
			sprintf(Message,"Err in PolyExpand() k = %ld",(long)k);
			if(Beta) Alert1(Message);
			break;
			}
			
		(*p_p)[a] = 0.;
		(*p_q)[a] = 1.;
		(*p_vargap)[a] = 0;
		if(!restart) {
			(*p_pgap)[a] = 0.; (*p_qgap)[a] = 1.;
			firstistempo = TRUE;
			(*p_firstistempo)[a] = FALSE;
			}
		ic = 0;
		(*ptr_fixtempo)[a] = FALSE;
		(*p_empty)[a] = TRUE;
		(*p_useful)[a] = FALSE;
		scale = oldscale;
		speed = oldspeed;
		prevscale = scale;
		prevspeed = speed;
		if(scale >= 1. || scale == 0.) {
			y = modf((scale / (double)TOKBASE),&x); 
			(*((*pp_c)[a]))[ic++] = T0;
			(*((*pp_c)[a]))[ic++] = 21;	/* '*' scale up */
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte)  scale - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
			}
		else {
			y = modf(((1. / scale) / (double)TOKBASE),&x);
			(*((*pp_c)[a]))[ic++] = T0;
			(*((*pp_c)[a]))[ic++] = 24;	/* '**' scale down */
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte)  (1./scale) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
			}
		if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
			result = ABORT; goto OUT;
			}
		if(speed >= 1.) {
			(*((*pp_c)[a]))[ic++] = T0;
			(*((*pp_c)[a]))[ic++] = 11;	/* '/' speed up */
			y = modf((speed / (double)TOKBASE),&x);
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte) speed - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
			}
		else {
			(*((*pp_c)[a]))[ic++] = T0;
			(*((*pp_c)[a]))[ic++] = 25;	/* '\' speed down */
			y = modf((1. / speed / (double)TOKBASE),&x);
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte) (1./speed) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
			}
		if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
			result = ABORT; goto OUT;
			}
		continue;
		}
	if(period) {
		if(m == T3 || m == T25) (*p_useful)[a] = foundtokens = TRUE;
		}
	else {	/* comma */
		if((m == T3 && p > 1) || m == T25 || (m >= T7 && m <= T43))
			(*p_useful)[a] = foundtokens = TRUE;
		}
	if(m == T3 || m == T7 || m == T8 || m == T9 || m == T25) {
		/* Terminal or prolongation or out-time object or synchronization tagÉ */
		/* É or time pattern or simple note */
		firstistempo = FALSE;
		if(m == T3 || m == T9 || m == T25) {
			if(Add((*p_p)[a],(*p_q)[a],scale,speed,&xp,&xq,&overflow) != OK){
				result = ABORT; goto OUT;
				}
			if(overflow) TellComplex();
			if(xp > MaxFrac || xq > MaxFrac) {
				MakeRatio(MaxFrac,(xp/xq),&xp,&xq);
				TellComplex();
				}
			else Simplify(MaxFrac,xp,xq,&xp,&xq);
			
			(*p_p)[a] = xp; (*p_q)[a] = xq;
			}
		(*((*pp_c)[a]))[ic++] = (tokenbyte) m;
		(*((*pp_c)[a]))[ic++] = (tokenbyte) p;
		if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
			result = ABORT; goto OUT;
			}
		(*p_empty)[a] = FALSE;
		}
	else {
		if((m == T4 || m == T6 || (m == T0 && p == 1))) {
			if(ShowMessages && (m != T4 || MaxVar > 0) && (ShowGraphic || !Improvize)) {
				switch(m) {
					case T4:
						sprintf(Message,
							"Variable '%s' was found (field level %ld)",*((*p_Var)[p]),
								(long)a);
						break;
					case T6:
						sprintf(Message,
							"Unreplaced wild card '?%ld' was found (field level %ld)",
								(long)p,(long)a);
						break;
					case T0:
						sprintf(Message,
							"Unreplaced wild card '?' was found (field level %ld)",
								(long)a);
						break;
					}
				ShowMessage(TRUE,wMessage,Message);
				}
			}
		else {
			if((m >= T10 && m <= T24) || (m >= T26 && m <= T43) || (m == T0
															&& (p == 18 || p == 19))) {
			/* '&' or tool or performance control */
				(*((*pp_c)[a]))[ic++] = (tokenbyte) m;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) p;
				(*p_empty)[a] = FALSE;
				foundtokens = TRUE;
				if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
					result = ABORT; goto OUT;
					}
				}
			}
		}
	}
(*((*pp_c)[a]))[ic++] = TEND;
(*((*pp_c)[a]))[ic++] = TEND;
if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
	result = ABORT; goto OUT;
	}

END:
if((a+1) != k) {
	sprintf(Message,"Err. PolyExpand(). a+1=%ld k=%ld",
		(long)(a+1L),(long)k);
	if(Beta) Alert1(Message);
	}
fixlength = FALSE;
restart = firstistempo = toobigitem = FALSE;
pmax = 0.; qmax = 1.;
a1 = a0 = -1;
L = 1.;
result = EMPTY;

for(a=kk=0; a < k; a++) {
	if(!(*p_empty)[a]) result = OK;
	if((*p_useful)[a]) {
		(*p_onefielduseful) = TRUE;
		kk++;	/* Only used in sequences */
		if(a1 == -1) a1 = a;
		}
	if((*p_vargap)[a] || (*p_p)[a] == ZERO) continue;
	if(a0 == -1) {
		a0 = a;		/* The duration field is the leftmost oneÉ */
					/* É satisfying non-empty conditions */
		pmax = (*p_p)[a];
		qmax = (*p_q)[a];
		}
	if(!fixlength && !firstistempo && (*p_firstistempo)[a] && (*p_useful)[a]
			&& !(*ptr_fixtempo)[a]) {
		a0 = a;
		pmax = (*p_p)[a];
		qmax = (*p_q)[a];
		firstistempo = TRUE;
		}
	if(!toobigitem) {
		if((L=LCM(L,(*p_p)[a],&overflow)) < 1.) {
			Println(wTrace,"Unexpected overflow in polymetric formula (case 1). You may send this item to the designers...");
			result = FAILED; goto OUT;
			}
		if(overflow) {
			TellComplex();
			toobigitem = TRUE;
			}
		}
	if((*ptr_fixtempo)[a]) {
		isequal = Equal(0.01,(*p_p)[a],(*p_q)[a],pmax,qmax,&overflow);
		if(overflow) TellComplex();
		if(isequal == ABORT) {
			Println(wTrace,"Unexpected overflow in polymetric formula (case 5). You may send this item to the designers...");
			result = FAILED; goto OUT;
			}
		if(fixlength && (isequal != TRUE)) {
			if(comma) sprintf(Message,"Conflicting field duration (field %ld)...\n",
				(long)(a+1L));
			else sprintf(Message,"Conflicting beat duration (beat %ld)...\n",
				(long)(a+1L));
			Print(wTrace,Message);
			result = FAILED; goto OUT;
			}
		fixlength = TRUE;
		pmax = (*p_p)[a];
		qmax = (*p_q)[a];
		a0 = a;	/* Fixed length field: must be the duration one. */
		(*p_fixtempo) = TRUE;
		}
	}
	
if(a0 == -1) {
	a0 = 0;
	(*p_r)[a0] = 1.;
	}
	
if(result == EMPTY) {
	(**pp_a)[0] = TEND;
	(**pp_a)[1] = TEND;
	goto OUT;
	}
	
for(a=0; a < k; a++) {	/* Calculate undetermined rests */
	if((*p_vargap)[a] > 0) {
		restart = TRUE;
		Substract(pmax,qmax,(*p_p)[a],(*p_q)[a],&xp,&xq,&sign,&overflow);
		if(overflow) TellComplex();
		if(sign < 0 || xp < 1.) {
			if((*ptr_fixtempo)[a0]) {
				if(comma)
					sprintf(Message,"Not enough time for undetermined rest (field %ld)\n",
						(long)(a+1L));
				else
					sprintf(Message,"Not enough time for undetermined rest (beat %ld)\n",
						(long)(a+1L));
				Print(wTrace,Message);
				result = FAILED; goto OUT;
				}
			else {
				gcd = GCD((*p_p)[a],(double)(*p_vargap)[a]);
				if(gcd < 1L) gcd = 1L;
				(*p_p)[a] = (*p_p)[a] / gcd;
				(*p_q)[a] = (*p_q)[a] * ((*p_vargap)[a] / gcd);
				if((lcm=LCM(qmax,(*p_q)[a],&overflow)) < 1.) {
					Println(wTrace,"Unexpected overflow in polymetric formula (case 2). You may send this item to the designers...");
					result = ABORT; goto OUT;
					}
				if(overflow) TellComplex();
				
				pmax = pmax * (lcm / qmax);
				qmax = lcm;
				(*p_p)[a] = (*p_p)[a] * (lcm / (*p_q)[a]);
				
				y = modf(((*p_p)[a] / pmax),&mgap);
				
				(*p_pgap)[a] = ((mgap + 1.) * pmax - (*p_p)[a]) / (*p_vargap)[a];
				(*p_qgap)[a] = lcm;
				}
			}
		else {
			gcd = GCD(xp,(double)(*p_vargap)[a]);
			if(gcd < 1L) gcd = 1L;
			(*p_pgap)[a] = xp / gcd;
			(*p_qgap)[a] = xq * (*p_vargap)[a] / gcd;
			}
		}
	}

if(restart) goto START;		/* Now rests are known */

/* Calculate duration of structure */

if(comma) {
	if(Add((*p_P),(*p_Q),pmax,qmax,&xp,&xq,&overflow) != OK) {
		Println(wTrace,"Unexpected overflow in polymetric formula (case 7). You may send this item to the designers...");
		result = ABORT; goto OUT;
		}
	if(overflow) TellComplex();
	if(xp > MaxFrac || xq > MaxFrac) {
		MakeRatio(MaxFrac,(xp/xq),&xp,&xq);
		TellComplex();
		}
	else Simplify(MaxFrac,xp,xq,&xp,&xq);
	(*p_P) = xp;
	(*p_Q) = xq;
	}
else {
	if(Add((*p_P),(*p_Q),((double) kk * pmax),qmax,&xp,&xq,&overflow)
			!= OK) {
		Println(wTrace,"Unexpected overflow in polymetric formula (case 8). You may send this item to the designers...");
		result = ABORT; goto OUT;
		}
	if(overflow) TellComplex();
	if(xp > MaxFrac || xq > MaxFrac) {
		MakeRatio(MaxFrac,(xp/xq),&xp,&xq);
		TellComplex();
		}
	else Simplify(MaxFrac,xp,xq,&xp,&xq);
	(*p_P) = xp;
	(*p_Q) = xq;
	}

if(!toobigitem) {
	M = 1.;
	for(a=0; a < k; a++) {
		if((*p_empty)[a]) continue;
		if((*p_p)[a] == 0.) continue;
		(*p_pp)[a] = L / (*p_p)[a];
		if((M = LCM(M,(*p_q)[a] * (*p_pp)[a],&overflow)) < 1.) {
			Println(wTrace,"Unexpected overflow in polymetric formula (case 10). You may send this item to the designers...");
			result = ABORT;
			goto OUT;
			}
		if(overflow) {
			TellComplex();
			toobigitem = TRUE;
			break;
			}
		}
	}
approxduration = pmax / qmax;

for(a=0; a < k; a++) {
	if((*p_empty)[a]) continue;
	(*p_r)[a] = 1.;
	if((*p_p)[a] == 0) continue;
	if(!toobigitem) (*p_r)[a] = M / ((*p_q)[a] * ((*p_pp)[a]));
	else (*p_r)[a] = ((*p_p)[a] / (*p_q)[a]) / approxduration;
	}

ss = 1.;

scaledown = scaleup = 1.;

xp = (*p_r)[a0];

rescale = xp;

//  Copy structure to *pp_a

result = OK;
level = ZERO;
id = idorg;

scale = prevscale = oldscale;
speed = prevspeed = oldspeed;
kmax = 0;

for(a=0; a < k; a++) {
	if((*p_empty)[a]) continue;
	if(comma && !(*p_useful)[a]) continue;
	if(comma && a > a1) {
		(**pp_a)[id++] = T0;
		(**pp_a)[id++] = 14; /* ',' */
		if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto OUT;
		}
	scale = rescale * oldscale;
	speed = oldspeed * (*p_r)[a];
	if(speed > MaxTempo || scale > MaxTempo) {
		MakeRatio(MaxTempo,(scale/speed),&scale,&speed);
		TellComplex();
		}
	else Simplify(MaxTempo,scale,speed,&scale,&speed);
	if(scale < InvMaxTempo) scale = 0.;
	if(comma || a == 0 || scale != prevscale || speed != prevspeed) {
		if(scale >= 1. || scale == 0.) {
			y = modf((scale / (double)TOKBASE),&x);
			(**pp_a)[id++] = T0;
			(**pp_a)[id++] = 21;	/* '*' scale up */
			(**pp_a)[id++] = T1;
			(**pp_a)[id++] = (tokenbyte) x;
			(**pp_a)[id++] = T1;
			(**pp_a)[id++] = (tokenbyte)  scale - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
			}
		else {
			y = modf(((1. / scale) / (double)TOKBASE),&x);
			(**pp_a)[id++] = T0;
			(**pp_a)[id++] = 24;	/* '**' scale down */
			(**pp_a)[id++] = T1;
			(**pp_a)[id++] = (tokenbyte) x;
			(**pp_a)[id++] = T1;
			(**pp_a)[id++] = (tokenbyte)  (1./scale) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
			}
		if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto OUT;
		if(speed >= 1.) {
			y = modf((speed / (double)TOKBASE),&x);
			(**pp_a)[id++] = T0;
			(**pp_a)[id++] = 11;	/* '/' speed up */
			(**pp_a)[id++] = T1;
			(**pp_a)[id++] = (tokenbyte) x;
			(**pp_a)[id++] = T1;
			(**pp_a)[id++] = (tokenbyte)  speed - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
			}
		else  {
			y = modf((1. / speed / (double)TOKBASE),&x);
			(**pp_a)[id++] = T0;
			(**pp_a)[id++] = 25;	/* '\' speed down */
			(**pp_a)[id++] = T1;
			(**pp_a)[id++] = (tokenbyte) x;
			(**pp_a)[id++] = T1;
			(**pp_a)[id++] = (tokenbyte)  (1./speed) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
			}
		if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto OUT;
		}
	
	prevscale = scale;
	prevspeed = speed;
	space = forceshowtempo = FALSE;
	
	for(ic=ZERO; ; ic+=2L) {
		if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto OUT;
		m = (*((*pp_c)[a]))[ic];
		p = (*((*pp_c)[a]))[ic+1];
		if(m == T3 && p == 1) {
			/* Replace strings of '-' with trailing '_' otherwise FillPhaseDiagram() may not place them... */
			/* ... properly when toofast is true */
			if(space && notrailing) p = 0;
			else space = TRUE;
			goto COPYIT;
			}
		space = FALSE;
		if(m == TEND && p == TEND) break;
		if(m == T0) {
			switch(p) {
				case 12: /* '{' */
					level++;
					forceshowtempo = TRUE;
					goto COPYIT;
					break;
				case 13: /* '}' */
					level--;
					forceshowtempo = TRUE;
					goto COPYIT;
					break;
				case 14: /* ',' */
					forceshowtempo = TRUE;
					goto COPYIT;
					break;
				case 22:	/* Forget '|' */
				case 23:
					break;
				case 21:	/* '*' scale up */
					s = GetScalingValue(((*pp_c)[a]),ic);
					scale = rescale * s;
					ic += 4L;
					break;
				case 24:	/* '**' scale down */
					s = GetScalingValue(((*pp_c)[a]),ic);
					scale = rescale / s;
					ic += 4L;
					break;
				case 11:	/* '/' speed up */
					speed = GetScalingValue(((*pp_c)[a]),ic);
					speed = speed * ((*p_r)[a]);
					break;
				case 25:	/* '\' speed down */
					speed = GetScalingValue(((*pp_c)[a]),ic);
					speed = 1. / speed;
					speed = speed * ((*p_r)[a]);
					break;
				default:
					goto COPYIT;
				}
			switch(p) {
				case 11:	/* '/' speed up */
				case 25:	/* '\' speed down */
					xp = speed;
					xq = scale;
					if(speed > MaxTempo || scale > MaxTempo) {
						MakeRatio(MaxTempo,(scale/speed),&xq,&xp);
						TellComplex();
						}
					else Simplify(MaxTempo,xq,xp,&xq,&xp);
					if(xq < InvMaxTempo) xq = 0.;
					isequal = Equal(0.005,xq,xp,prevscale,prevspeed,&overflow);
					if(isequal == ABORT) {
						if(Beta) {
							Alert1("Err. PolyExpand(). isequal == ABORT");
							result = ABORT; goto OUT;
							}
						isequal = FALSE;
						}
					if(isequal == FALSE || forceshowtempo) {
						forceshowtempo = FALSE;
						prevscale = xq;
						prevspeed = xp;
						if(xq >= 1. || xq == 0.) {
							y = modf((xq / (double)TOKBASE),&x);
							(**pp_a)[id++] = T0;
							(**pp_a)[id++] = 21;	/* '*' scale up */
							(**pp_a)[id++] = T1;
							(**pp_a)[id++] = (tokenbyte) x;
							(**pp_a)[id++] = T1;
							(**pp_a)[id++] = (tokenbyte)  xq - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
							}
						else  {
							y = modf((1. / xq / (double)TOKBASE),&x);
							(**pp_a)[id++] = T0;
							(**pp_a)[id++] = 24;	/* '**' scale down */
							(**pp_a)[id++] = T1;
							(**pp_a)[id++] = (tokenbyte) x;
							(**pp_a)[id++] = T1;
							(**pp_a)[id++] = (tokenbyte)  (1. / xq) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
							}
						if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto OUT;
						if(xp >= 1.) {
							y = modf((xp / (double)TOKBASE),&x);
							(**pp_a)[id++] = T0;
							(**pp_a)[id++] = 11;	/* '/' speed up */
							(**pp_a)[id++] = T1;
							(**pp_a)[id++] = (tokenbyte) x;
							(**pp_a)[id++] = T1;
							(**pp_a)[id++] = (tokenbyte)  xp - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
							}
						else  {
							y = modf((1. / xp / (double)TOKBASE),&x);
							(**pp_a)[id++] = T0;
							(**pp_a)[id++] = 25;	/* '\' speed down */
							(**pp_a)[id++] = T1;
							(**pp_a)[id++] = (tokenbyte) x;
							(**pp_a)[id++] = T1;
							(**pp_a)[id++] = (tokenbyte)  (1./xp) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
							}
						if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto OUT;
						}
					ic += 4L;
					break;
				}
			}
		else {
COPYIT:
			(**pp_a)[id++] = (tokenbyte) m;
			(**pp_a)[id++] = (tokenbyte) p;
			if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto OUT;
			}
		}
	kmax++;
	}

(**pp_a)[id++] = TEND; (**pp_a)[id++] = TEND;
if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto OUT;

if(Beta && level != ZERO) {
	Alert1("Err. PolyExpand(). level != ZERO (end)");
	}

result = OK;
if(kmax == 1) result = SINGLE; /* Indicates that structure had one single field */

OUT:

MyDisposeHandle((Handle*)&ptr_fixtempo); MyDisposeHandle((Handle*)&p_vargap);
MyDisposeHandle((Handle*)&p_empty); MyDisposeHandle((Handle*)&p_useful);
MyDisposeHandle((Handle*)&p_firstistempo);
MyDisposeHandle((Handle*)&p_p); MyDisposeHandle((Handle*)&p_q);
MyDisposeHandle((Handle*)&p_pp); MyDisposeHandle((Handle*)&p_r);
MyDisposeHandle((Handle*)&p_pgap); MyDisposeHandle((Handle*)&p_qgap);
MyDisposeHandle((Handle*)&p_e);
for(a=0; a < k; a++) {
	ptr = (*pp_c)[a];
	MyDisposeHandle((Handle*)&ptr);
	(*pp_c)[a] = NULL;
	}
MyDisposeHandle((Handle*)&pp_c);
MyDisposeHandle((Handle*)&p_maxic);
if(CheckEmergency() != OK) result = ABORT;
if((result == OK || result == SINGLE) && imbedded) result = IMBEDDED;
return(result);
}


Check_ic(unsigned long ic,unsigned long **p_maxic,int a,tokenbyte ****pp_c)
{
tokenbyte **ptr;
unsigned long size;

if(ic > (*p_maxic)[a]) {
	size = (*p_maxic)[a];
	(*p_maxic)[a] = (size * 3L) / 2L;
	ptr = (*pp_c)[a];
	if((ptr=(tokenbyte**) IncreaseSpace((Handle)ptr)) == NULL) {
		return(ABORT);
		}
	(*pp_c)[a] = ptr;
	}
return(OK);
}


TellComplex(void)
{
if(!SaidTooComplex) {
	SaidTooComplex = TRUE;
	FlashInfo("Formula is too complex. Roundings are performed...");
	if(ShowMessages)
		ShowMessage(TRUE,wMessage,"Formula is too complex. Roundings are performed...");
	}
return(OK);
}


CheckSize(unsigned long i,unsigned long *p_maxi,tokenbyte ***pp_a)
{
tokenbyte **ptr;

if(i > (*p_maxi)) {
	ptr = (*pp_a);
	if((ptr=(tokenbyte**) IncreaseSpace((Handle)ptr)) == NULL) {
		return(ABORT);
		}
	(*pp_a) = ptr;
	(*p_maxi) = ((*p_maxi) * 3L) / 2L;
	}
return(OK);
}