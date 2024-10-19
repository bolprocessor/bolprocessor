/* Polymetric.c (BP3) */

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

int trace_polymake = 0;

int PolyMake(tokenbyte ***pp_a,double *p_maxseq,int notrailing) {
	tokenbyte m,p,**p_b,**ptr;
	char fixtempo,useful,toocomplex,alreadychangedquantize;
	int k,krep,rep,level,r,**p_nseq,**p_nseqmax,maxlevel,needalphabet,foundinit,overflow,toofast,
		numberouttimeinseq,longestseqouttime,numbertoofast,longestnumbertoofast,morelines,max_quantization;
	double P,Q,tempo,tempomax,prodtempo,fmaxseq,nsymb,lcm,kpress,thelimit,speed,scaling,s,
		limit1,limit2,imax,x,firstscaling,scalespeed,maxscalespeed;
	unsigned long i,maxid,pos_init,gcd,numberprolongations;
	long newquantize,newquantize2,totalbytes,a,b;

	if(CheckEmergency() != OK) return(ABORT);

	r = MISSED;
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
		if(Beta) Alert1("=> Err. PolyMake(). *pp_a = NULL");
		return(ABORT);
		}
		
	maxid = (MyGetHandleSize((Handle)*pp_a) / sizeof(tokenbyte));
	if(maxid <= 2) {
		return(MISSED);
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
		BPPrintMessage(0,odError,"=> Incorrect polymetric expression(s): '{' and '}' are not balanced. Can't proceed further...\n");
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
	//	if(trace_polymake) // HideWindow(Window[wMessage]);
		}

	if(Beta && NeedZouleb > 0) {
		my_sprintf(Message,"NeedZouleb = %ld in polymetric expression",(long)NeedZouleb);
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
				if(Beta) Alert1("=> Err. PolyMake(). level > maxlevel");
				r = ABORT; goto QUIT;
				}
			continue;
			}
		if(p == 13 || p == 23) {	/* '}' */
			if(level < 1) {
				if(Beta) Alert1("=> Err. PolyMake(). {} not balanced");
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
			/* Insert '/1' speed marker because none has been found. Otherwise, the initial... */
			/* ... section numbers might be wrongly interpreted */
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
	r = PolyExpand(p_b,pp_a,pos_init,&maxid,&pos_init,&P,&Q,1.,&fixtempo,&useful,1.,notrailing);
	if(r != OK && r != SINGLE && r != IMBEDDED) goto QUIT;
	//////////////////////////////////

	if(trace_polymake) BPPrintMessage(0,odInfo,"After PolyExpand() maxid = %ld pos_init = %ld P = %ld Q = %ld fixtempo = %ld useful = %ld\n",(long)maxid,(long)pos_init,(long)P,(long)Q,(long)fixtempo,(long)useful);

	r = OK;
	scaling = speed = tempo = 1.;
	firstscaling = 0.;
	level = 0;

	// Calculate first scaling

	for(i=ZERO; ; i+=2L) {
		m = (**pp_a)[i]; p = (**pp_a)[i+1];
		if(m == TEND && p == TEND) break;
		if(m == T3 || m == T25 || m == T9) {
			if(firstscaling == 0.) {
				firstscaling = scaling;
				break;
				}
			}
		if(m == T0) {
			switch(p) {
				case 21:	/* '*' scaling up */
					scaling = GetScalingValue((*pp_a),i);
					i += 4L;
					break;
				case 24:	/* '**' scaling down */
					scaling = GetScalingValue((*pp_a),i);
					scaling = 1. / scaling;
					i += 4L;
					break;
				}
			}
		}

	// Calculate Prod

	if(firstscaling == 0.) firstscaling = 1.;
	Prod = LCM(firstscaling,Q,&overflow);
	toocomplex = FALSE;
	scaling = speed = tempo = 1.;
	level = 0;
	maxscalespeed = 0.;

	for(i=ZERO; ; i+=2L) {
		m = (**pp_a)[i]; p = (**pp_a)[i+1];
		if(m == TEND && p == TEND) break;
		if(m == T0) {
			switch(p) {
				case 11: /* '/' speed up */
					speed = firstscaling * GetScalingValue((*pp_a),i);
					scalespeed = scaling / speed;
					if(maxscalespeed < scalespeed) maxscalespeed = scalespeed;
					if(!toocomplex) {
						gcd = GCD(speed,scaling);
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
					speed = firstscaling / GetScalingValue((*pp_a),i);
					scalespeed = scaling / speed;
					if(maxscalespeed < scalespeed) maxscalespeed = scalespeed;
					if(!toocomplex) {
						if(scaling != 0.) {
							tempo = speed / scaling;
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
				case 21:	/* '*' scaling up */
					scaling = GetScalingValue((*pp_a),i);
					scalespeed = scaling / speed;
					if(maxscalespeed < scalespeed) maxscalespeed = scalespeed;
					i += 4L;
					break;
				case 24:	/* '**' scaling down */
					scaling = GetScalingValue((*pp_a),i);
					scaling = 1. / scaling;
					scalespeed = scaling / speed;
					if(maxscalespeed < scalespeed) maxscalespeed = scalespeed;
					i += 4L;
					break;
				}
			}
		}

	PRODOK:

	numberprolongations = (Prod * maxscalespeed) / firstscaling;
	POLYconvert = TRUE;
	Ratio = Prod;
	Pduration = P;
	Qduration = Q;

	if(trace_polymake)  {
		if(Qduration > 1.)
			BPPrintMessage(0,odInfo,"Duration = %.0f/%.0f time units, Ratio = %.0f\n",Pduration,Qduration,(double)Ratio);
		else BPPrintMessage(0,odInfo,"Duration = %.0f time units, Ratio = %.0f\n",Pduration,(double)Ratio);
		}

	alreadychangedquantize = FALSE;

	// Calculate compression rate Kpress for quantization
	FINDCOMPRESSION:
	// BPPrintMessage(0,odInfo,"FINDCOMPRESSION\n");
	Kpress = 1.;
	if(Pclock > 0.) {
		kpress = 1. + (((double)Quantization) * Qclock * Ratio) / Pclock / 1000.;
		kpress = floor(1.00001 * kpress);
		if(QuantizeOK && kpress > 1.) {
			s = LCM(Ratio,kpress,&overflow) / Ratio;
			if(s > 1. && s < 10. && Ratio < 1000000.) Ratio = Round(s * Ratio);
			s = Round(Ratio / kpress);
			if(s > 10.) Ratio = kpress * s;
			Prod = Ratio;
			}
		if(QuantizeOK) {
			Kpress = kpress;
			BPPrintMessage(0,odInfo,"Using quantization = %ld ms with compression rate = %.0f\n",(long)Quantization,(double)Kpress);
			}
		else {
			if((kpress >= 4. && NotSaidKpress) || kpress >= 100.) {
				NotSaidKpress = FALSE;
				BPPrintMessage(0,odInfo,"Forcing quantization to %ld ms\n",Quantization);
				QuantizeOK = TRUE;
				goto FINDCOMPRESSION;
				}
			}
		}

	if(Pclock > 0.) {
	//	BPPrintMessage(0,odInfo,"maxscalespeed = %ld\n",(long)maxscalespeed);
	//	max_quantization = (int) (1000. * (Pclock/Qclock) * (Kpress - 1) / Kpress / maxscalespeed);
		// Added by BB 2021-01-31
	/*	if(max_quantization < Quantization)
			BPPrintMessage(0,odError,"=> To avoid skipping objects/notes, quantization should be less than %d ms\n",max_quantization); */
		}

	// Calculate Maxevent, (*p_maxseq) and the final value of Maxconc

	if(Beta && p_nseq != NULL) {
		Alert1("=> Err. PolyMake(). p_nseq != NULL");
		}
	if(Beta && p_nseqmax != NULL) {
		Alert1("=> Err. PolyMake(). p_nseqmax != NULL");
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
	speed = scaling = Prod / firstscaling;
	tempo = speed / scaling;
	prodtempo = Prod / tempo;
	// toofast = (tempo > tempomax);
	toofast = (tempo > tempomax || tempo == 0.); // Fixed by BB 2021-03-26

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
			if(toofast) {
				numbertoofast++;
				}
			else {
				if(numbertoofast > 0) {
					if(numbertoofast > longestnumbertoofast)
						longestnumbertoofast = numbertoofast;
					numbertoofast = 0;
					}
				}
			fmaxseq += prodtempo;
			if(p >= 1 || m == T9 || m == T25) {	/* Even silence may become an event if preceded... */
									/* ... by _pitchbend() for instance. */
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
			
		//	case T10: // _chan() // Added by BB 2021-01-25
		//	case T11: // _vel() // Added by BB 2021-01-25
			
			case T13: /* script line */
			case T14: /* _mod() */
			case T15: /* _pitchbend() */
			case T16: /* _press() */
			case T17: /* _switchon() */
			case T18: /* _switchoff() */
			case T19: /* _volume() */
			case T20: /* _legato() */
			
			case T21: /* _pitchrange() */ 
			case T22: /* _pitchrate() */
			case T23: /* _modrate() */
			case T24: /* _pressrate() */
			
			case T26: /* _transpose() */
			
			case T27: /* _volumerate() */
			case T28: /* _volumecontrol() */
			
			case T29: /* _pan() */
			
			case T30: /* _panrate() */
			case T31: /* _pancontrol() */
			case T32: /* _ins() */
			case T33: /* _step() */
			case T34: /* _cont() */
			
			case T35: /* _value() */
			
			case T36: /* _fixed() */
			
			case T37: /* _keymap() */
			
			case T38: /* _rndvel() */
			case T39: /* _rotate() */
			case T40: /* _keyxpand() */
			case T41: /* _rndtime() */
			case T42: /* _srand() */
			case T43: /* _tempo() */
			case T44: /* _scale() */
			case T45: /* _capture() */
			case T46: /* _part() */
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
						if(Beta) Alert1("=> Err. PolyMake(): level > maxlevel");
						r = ABORT; goto QUIT;
						}
					(*p_nseqmax)[level] = (*p_nseq)[level] = (*p_nseq)[level-1];
					if(level > Maxlevel) Maxlevel = level;
					morelines++;  // Fixed by BB 2021-01-30 - probably not optimal
					continue;
					break;
					
				case 13:	/* '}' */
				case 23:
					if(level < 1) {
						if(Beta) Alert1("=> Err. PolyMake(): level < 1");
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
				//	(*p_nseqmax)[level] += 2;
				//	(*p_nseq)[level] = (*p_nseqmax)[level];
					(*p_nseq)[level] = (*p_nseqmax)[level] + 1; // Fixed by BB 2021-01-29
					if((*p_nseq)[level] > Minconc) Minconc = (*p_nseq)[level];
					nsymb += 1.;
					continue;
					break;
					
				case 11:	/* '/' speed up */
					speed = GetScalingValue((*pp_a),i);
					if(scaling != 0.) {
						tempo = speed / scaling;
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
					if(scaling != 0.) {
						tempo = speed / scaling;
						prodtempo = Prod / tempo;
						}
					else tempo = 0.;
					toofast = (tempo > tempomax || tempo == 0.);
					i += 4;
					continue;
					break;
		
				case 21:		/* '*' scaling up */
					scaling = GetScalingValue((*pp_a),i);
					if(scaling != 0.) {
						tempo = speed / scaling;
						prodtempo = Prod / tempo;
						}
					else tempo = 0.;
					toofast = (tempo > tempomax || tempo == 0.);
					i += 4;
					continue;
					break;
					
				case 24:		/* '**' scaling down */
					scaling = GetScalingValue((*pp_a),i);
					scaling = 1. / scaling;
					if(scaling < InvMaxTempo) scaling = 0.;
					if(scaling != 0.) {
						tempo = speed / scaling;
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
	if((*p_nseqmax)[0] > Minconc) {
		BPPrintMessage(0,odError,"=> (*p_nseqmax)[0] = %ld Minconc = %ld\n",(long)(*p_nseqmax)[0],(long)Minconc);
		Minconc = (*p_nseq)[0];
		}
	Minconc++;
	if(numberouttimeinseq > longestseqouttime) longestseqouttime = numberouttimeinseq;
	if(numbertoofast > longestnumbertoofast) longestnumbertoofast = numbertoofast;
	fmaxseq += longestseqouttime + longestnumbertoofast; // Added by BB 2021-03-24
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
				Alert1("=> Incorrect value");
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
				BPPrintMessage(0,odError,"=> Polymetric expansion encountered an unpredicted overflow. Sorry, can't proceed further\n");
				r = ABORT;
				goto QUIT;
				}
			if(x > 2000.) {
				if(FALSE && (Quantization > 100L)) {
					rep = Answer("This item would require a large quantization (greater than 2000 ms). Use temporary memory instead",
						'Y');
					if(rep == YES) {
						FixedMaxQuantization = TRUE;
					//	AskedTempMemory = TRUE;
						goto FORGETIT;
						}
					else {
						if(x < 32767.) newquantize = (long) x;
						else newquantize = 2000L;
						goto CHANGEQUANTIZE;
						}
					}
				else {
					BPPrintMessage(0,odError,"=> Increasing quantization may not be sufficient to reduce memory requirement.\n");
					rep = CANCEL; // Fixed by BB 2021-01-30
				//	rep = Answer("Increasing quantization may not be sufficient to reduce memory requirement. Try it anyway",'Y');
					newquantize = 2000L;
					if(rep == YES) {
						goto CHANGEQUANTIZE;
						}
				//	if(rep == CANCEL) goto WANTABORT;
				//	AskedTempMemory = TRUE;
					FixedMaxQuantization = TRUE;
					goto FORGETIT;
					}
				}
			newquantize = (long) x;
		//	BPActivateWindow(SLOW,wTimeAccuracy);
			if(!ScriptExecOn && !alreadychangedquantize) BPPrintMessage(0,odError,"Quantization of %ld ms will be increased to reduce memory requirement (%ld ms might work)\n",
				(long) Quantization,(long) newquantize);
			else goto SETQUANTIZE;
		//	newquantize = 20L; // $$$
			
	CHANGEQUANTIZE:
		//	my_sprintf(Message,"%ld",(long)newquantize);
		//	rep = AnswerWith("Setting quantization to...",Message,Message);
		//	BPPrintMessage(0,odError,"Setting quantization to %ld ms\n",(long) newquantize);
			
	/*		if(rep == ABORT) { // Fixed by BB 2021-02-25

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
			
			newquantize = newquantize2; */
			alreadychangedquantize = TRUE;
			
	SETQUANTIZE:
			Quantization = newquantize;
	#if BP_CARBON_GUI_FORGET_THIS
			UpdateDirty(TRUE,iSettings);
			SetTimeAccuracy();
			BPActivateWindow(SLOW,wTimeAccuracy);
	#endif /* BP_CARBON_GUI_FORGET_THIS */
			MyDisposeHandle((Handle*)&p_nseq);
			MyDisposeHandle((Handle*)&p_nseqmax);
			goto FINDCOMPRESSION;
			}
		}
	// BPPrintMessage(0,odError,"@ morelines = %ld Minconc = %ld\n",(long)morelines,(long)Minconc);
	Minconc += morelines;
	// Minconc += Maxlevel;
	Maxlevel++;
	Maxconc = Minconc + 1 + longestseqouttime + longestnumbertoofast;

	// BPPrintMessage(0,odInfo,"Minconc = %ld, longestseqouttime = %ld, longestnumbertoofast = %ld, Maxconc = %ld\n",(long)Minconc,(long)longestseqouttime,(long)longestnumbertoofast,(long)Maxconc);
	BPPrintMessage(0,odInfo,"Phase diagram contains %ld lines",(long)Maxconc);
	if(longestnumbertoofast > 0) BPPrintMessage(0,odInfo,", longest stream faster than quantization = %ld notes or sound-objects",(long)longestnumbertoofast);
	BPPrintMessage(0,odInfo,"\n");

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

	FORGETIT:
	// Maxconc += Maxconc;
	Maxevent += ((2 * Maxconc) + 1);
	// Maxevent += Maxconc + 1;
	// Maxconc += 4;
	(*p_maxseq) = fmaxseq + 1.;
	/* Takes care of newswitch at the end of diagram */

	if(ShowMessages) ShowDuration(YES);
		
	OkShowExpand = TRUE;	/* OK to display prolongational gaps "_" */
	if(nsymb > 15000L || numberprolongations > 2000 || ((Prod / firstscaling) > 200)) {
		if(ShowMessages) {
			if(nsymb > 15000L) {
				BPPrintMessage(0,odInfo,"Expanded polymetric expression would contain %.0f symbols\n",nsymb);
				}
			if(ExpandOn) ShowMessage(TRUE,wMessage,"Expanded expression is too large for display");
			}
		OkShowExpand = FALSE;
		}
	r = OK;

	QUIT:
	// BPPrintMessage(0,odInfo,"@@ Maxconc = %ld\n",(long)Maxconc);
	MyDisposeHandle((Handle*)&p_b);
	MyDisposeHandle((Handle*)&p_nseq);
	MyDisposeHandle((Handle*)&p_nseqmax);
	PolyOn = FALSE;

	if(r == IMBEDDED) r = OK;
	return(r);
	}


int PolyExpand(tokenbyte **p_b,tokenbyte ***pp_a,unsigned long idorg,unsigned long *p_maxid,
	unsigned long *p_pos,double *p_P,double *p_Q,double oldspeed,char *p_fixtempo,
	char *p_onefielduseful,double oldscaling,int notrailing)
{
tokenbyte m,p,****pp_c,**p_e,**ptr;
char xf,useful,**ptr_fixtempo,**p_firstistempo,**p_empty,**p_useful,
	foundtokens,toobigitem,truebracket,space,forceshowtempo;
int gr,k,kk,kmax,a,a0,a1,result,r,tempomark,fixlength,dirtymem,sign,isequal,
	singlegap,newg,newh,just_fill_gap,restart,comma,period,overflow,imbedded,
	compiledmem,firstistempo,**p_vargap;
double L,M,lcm,Q,**p_p,**p_q,**p_pp,**p_r,pmax,qmax,**p_pgap,**p_qgap,xp,xq,ss,x,y,t,
	speed,speedbeforegap,scalebeforegap,s,mm,scalegap,scaleup,scaledown,rescale,
	mgap,approxduration,rescalesubstructure,scaling,prevscale,prevspeed;
long level;
unsigned long i,j,jmax,gcd,g,h,lastbyte,oldpos,ic,id,**p_maxic,useless,ptempo,qtempo;

if(trace_polymake) {
	my_sprintf(Message,"Expanding polymetric expression [position %lu]...",(*p_pos));
	ShowMessage(TRUE,wMessage,Message);
	}
result = ABORT; compiledmem = CompiledGr;

if((r=stop(1,"PolyExpand")) != OK) return(r);

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
		case 7:		/* period */
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
if((p_e = (tokenbyte**) GiveSpace((Size) FIELDSIZE * sizeof(tokenbyte))) == NULL) goto SORTIR;
jmax = FIELDSIZE - 16L;

if((ptr_fixtempo = (char**) GiveSpace((Size)sizeof(char) * k)) == NULL) 
	goto SORTIR;
if((p_vargap = (int**) GiveSpace((Size)sizeof(int) * k)) == NULL) 
	goto SORTIR;
if((p_firstistempo = (char**) GiveSpace((Size)sizeof(char) * k)) == NULL) 
	goto SORTIR;
if((p_empty = (char**) GiveSpace((Size)sizeof(char) * k)) == NULL) 
	goto SORTIR;
if((p_useful = (char**) GiveSpace((Size)sizeof(char) * k)) == NULL) 
	goto SORTIR;
if((p_p = (double**) GiveSpace((Size)sizeof(double) * k)) == NULL) 
	goto SORTIR;
if((p_q = (double**) GiveSpace((Size)sizeof(double) * k)) == NULL) 
	goto SORTIR;
if((p_pp = (double**) GiveSpace((Size)sizeof(double) * k)) == NULL) 
	goto SORTIR;
if((p_r = (double**) GiveSpace((Size)sizeof(double) * k)) == NULL) 
	goto SORTIR;
if((p_pgap = (double**) GiveSpace((Size)sizeof(double) * k)) == NULL) 
	goto SORTIR;
if((p_qgap = (double**) GiveSpace((Size)sizeof(double) * k)) == NULL) 
	goto SORTIR;
	
(*p_pgap)[0] = 0.; (*p_qgap)[0]= 1.;
oldpos = (*p_pos);
restart = FALSE;
if(period && comma) {
	BPPrintMessage(0,odError,"=> Error in polymetric expression.\nThe same expression contains both a bullet and a comma...\n");
	result = ABORT;
	goto SORTIR;
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

prevscale = scaling = oldscaling;
prevspeed = speed = oldspeed;
if(speed > TokenLimit || (1./speed) > TokenLimit) {
	BPPrintMessage(0,odError,"=> Unexpected overflow in polymetric formula (case 14). You may send this item to the designers...\n");
	result = ABORT; goto SORTIR;
	}

just_fill_gap = FALSE;
		
for(i = (*p_pos); (m = (*p_b)[i]) != TEND || (*p_b)[i+1] != TEND; i += 2L) {
	p = (*p_b)[i+1];
	if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
		result = ABORT; goto SORTIR;
		}
 	if(m == T0 && p == 21) {  /* '*' scaling up */
 		scaling = 0.;
 		do {
 			i += 2;
 			m = (*p_b)[i];
 			p = (*p_b)[i+1];
 			if(m == T1) scaling = (TOKBASE * scaling) + p;
			}
		while(m == T1);
		i -= 2;
		continue;
 		}
 	if(m == T0 && p == 24) {  /* '**' scaling down */
 		s = 0.;
 		do {
 			i += 2;
 			m = (*p_b)[i];
 			p = (*p_b)[i+1];
 			if(m == T1) s = (TOKBASE * s) + p;
			}
		while(m == T1);
		if(s != 0.) scaling = 1. / s;
		else if(Beta) Alert1("=> Err. PolyExpand() after '**'. s = 0.");
		i -= 2;
		continue;
 		}
 	if(m == T0 && p == 25) {  /*  speed down */
 		s = 0.;
 		do {
 			i += 2;
 			m = (*p_b)[i];
 			p = (*p_b)[i+1];
 			if(m == T1) s = (TOKBASE * s) + p;
			}
		while(m == T1);
		if(s != 0.) speed = 1. / s;
		else if(Beta) Alert1("=> Err. PolyExpand() after '\\'. s = 0.");
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
			scalebeforegap = scaling;
			speedbeforegap = speed;
			if(g > 1000.) {
				if(MakeRatio(1000.,(((double)g) / ((double)h)),&x,&y) != OK) {
					result = ABORT; goto SORTIR;
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
			
		if(speed > MaxTempo || scaling > MaxTempo) {
			MakeRatio(MaxTempo,(scaling/speed),&xq,&xp);
			speed = xp;
			scaling = xq;
			}
		else Simplify(MaxTempo,scaling,speed,&xq,&xp);
		if(xq < InvMaxTempo) xq = 0.;
		speed = xp;
		scaling = xq;
		isequal = Equal(0.005,scaling,speed,prevscale,prevspeed,&overflow);
		if(isequal == ABORT) {
			if(Beta) {
				BPPrintMessage(0,odError,"=> Err. PolyExpand(). isequal == ABORT\n");
				result = ABORT; goto SORTIR;
				}
			isequal = FALSE;
			}
		if(isequal == FALSE) {
			prevscale = scaling;
			prevspeed = speed;
			if(scaling >= 1. || scaling == 0.) {
				y = modf((scaling/(double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 21;	/* '*' scaling up */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) scaling - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			else {
				y = modf(((1./scaling)/(double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 24;	/* '**' scaling down */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) (1./scaling) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
				result = ABORT; goto SORTIR;
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
				(*((*pp_c)[a]))[ic++] = 25;	/*  speed down */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) (1./speed) - (((tokenbyte) x) * TOKBASE);  /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
				result = ABORT; goto SORTIR;
				}
			}
		h = ZERO;
		}
	if(newg) {
		newg = FALSE;
		(*p_empty)[a] = FALSE;
		if(g > ZERO) {
			if(Add((*p_p)[a],(*p_q)[a],(double) g * scaling,speed,&xp,&xq,
					&overflow) != OK) {
				result = ABORT; goto SORTIR;
				}
			if(overflow) TellComplex();
			
			if(trace_polymake) BPPrintMessage(0,odInfo,"MaxFrac = %.0f, xp = %.0f xq = %.0f\n",MaxFrac,xp,xq);
			if(xp > MaxFrac || xq > MaxFrac) {
				MakeRatio(MaxFrac,(xp/xq),&xp,&xq);
				TellComplex();
				}
			else Simplify(MaxFrac,xp,xq,&xp,&xq);
			
			(*p_p)[a] = xp; (*p_q)[a] = xq;
			if(trace_polymake) BPPrintMessage(0,odInfo,"xp = %.0f xq = %.0f\n",xp,xq);
			
			/* Replace number with '-' */
			(*((*pp_c)[a]))[ic++] = T3;
			(*((*pp_c)[a]))[ic++] = 1;	/* '-' */
			if(period) (*p_useful)[a] = TRUE;
			g--;
			while(g > ZERO) {
				if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
					result = ABORT; goto SORTIR;
					}
				(*((*pp_c)[a]))[ic++] = T3;
				(*((*pp_c)[a]))[ic++] = 0;	/* '_' */
				g--;
				}
			}
		/* g = ZERO now */
		if(singlegap) {
			singlegap = FALSE;
			isequal = Equal(0.005,scaling,speed,scalebeforegap,speedbeforegap,&overflow);
			if(isequal == ABORT) {
				if(Beta) {
					BPPrintMessage(0,odError,"=> Err. PolyExpand(). isequal == ABORT\n");
					result = ABORT; goto SORTIR;
					}
				isequal = FALSE;
				}
			if(isequal == FALSE) {
				scaling = scalebeforegap;
				speed = speedbeforegap;
				prevscale = scaling;
				prevspeed = speed;
				if(scaling >= 1. || scaling == 0.) {
					y = modf((scaling/(double)TOKBASE),&x);
					(*((*pp_c)[a]))[ic++] = T0;
					(*((*pp_c)[a]))[ic++] = 21;	/* '*' scaling up */
					(*((*pp_c)[a]))[ic++] = T1;
					(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
					(*((*pp_c)[a]))[ic++] = T1;
					(*((*pp_c)[a]))[ic++] = (tokenbyte) scaling - (((tokenbyte) x) * TOKBASE);  /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
					}
				else {
					y = modf(((1./scaling)/(double)TOKBASE),&x);
					(*((*pp_c)[a]))[ic++] = T0;
					(*((*pp_c)[a]))[ic++] = 24;	/* '**' scaling down */
					(*((*pp_c)[a]))[ic++] = T1;
					(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
					(*((*pp_c)[a]))[ic++] = T1;
					(*((*pp_c)[a]))[ic++] = (tokenbyte) (1./scaling) - (((tokenbyte) x) * TOKBASE);  /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
					}
				if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
					result = ABORT; goto SORTIR;
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
					result = ABORT; goto SORTIR;
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
			if(Beta) Alert1("=> Err. PolyMake(). m != T43");
			i -= 2L;
			continue;
			}
		qtempo = (*p_b)[i+1L];
		if(firstistempo) {
			(*p_firstistempo)[a] = TRUE;
			firstistempo = FALSE;
			}
		prevspeed = speed;
		prevscale = scaling;
		speed = oldspeed * ptempo;
		scaling = oldscaling * qtempo;
		if(speed > MaxTempo || scaling > MaxTempo) {
			MakeRatio(MaxTempo,(scaling/speed),&xq,&xp);
			speed = xp;
			scaling = xq;
			TellComplex();
			}
		else Simplify(MaxTempo,scaling,speed,&xq,&xp);
		if(xq < InvMaxTempo) xq = 0.;
		speed = xp;
		scaling = xq;
		
		isequal = Equal(0.005,scaling,speed,prevscale,prevspeed,&overflow);
		if(isequal == ABORT) {
			if(Beta) {
				Alert1("=> Err. PolyExpand(). isequal == ABORT");
				result = ABORT; goto SORTIR;
				}
			isequal = FALSE;
			}
		if(isequal == FALSE) {
			prevscale = scaling;
			prevspeed = speed;
			if(scaling >= 1. || scaling == 0.) {
				y = modf((scaling / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 21;	/* '*' scaling up */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) scaling - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			else {
				y = modf(((1. / scaling) / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 24;	/* '**' scaling down */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) (1./scaling) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
				result = ABORT; goto SORTIR;
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
				(*((*pp_c)[a]))[ic++] = 25; /*  speed down */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte)  (1./speed) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
				result = ABORT; goto SORTIR;
				}
			}
		continue;
		}
	if(m == T0 && p == 17) {					/* Undetermined rest "_rest" */
		firstistempo = (*p_firstistempo)[a] = FALSE;
		/* This field can't be used to determine the duration */
		if(restart) {	/* Scanning structure second time...*/
			newh = newg = TRUE;
			xp = speed * ((*p_pgap)[a]);
			xq = (*p_qgap)[a] * (scaling);
			if(xp > ULONG_MAX || xq > ULONG_MAX)  {
				if(MakeRatio(ULONG_MAX,(xq/xp),&xq,&xp) != OK) {
					result = ABORT; goto SORTIR;
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
		r = PolyExpand(p_b,&p_e,ZERO,&jmax,&i,&xp,&xq,speed,&xf,&useful,scaling,notrailing);
		///////////////////////////////////
		
		if(r != OK && r != SINGLE && r != EMPTY && r != IMBEDDED) {
			result = r;
			goto SORTIR;
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
					if(Beta) Alert1("=> Err. PolyExpand() j > jmax");
					result = ABORT; goto SORTIR;
					}
				if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
					result = ABORT; goto SORTIR;
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
			if(scaling >= 1. || scaling == 0.) {
				y = modf((scaling / (double)TOKBASE),&x); 
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 21;	/* '*' scaling up */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte)  scaling - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			else {
				y = modf(((1. / scaling) / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T0;
				(*((*pp_c)[a]))[ic++] = 24;	/* '**' scaling down */
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte)  (1./scaling) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			prevscale = scaling;
			if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
				result = ABORT; goto SORTIR;
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
				(*((*pp_c)[a]))[ic++] = 25;	/* speed down */
				y = modf((1. / speed / (double)TOKBASE),&x);
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
				(*((*pp_c)[a]))[ic++] = T1;
				(*((*pp_c)[a]))[ic++] = (tokenbyte)  (1./speed) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
				}
			prevspeed = speed;
			if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
				result = ABORT; goto SORTIR;
				}
			}
		continue;
		}
	if(m == T0 && (p == 13 || p == 23)) {		/* '}' or '|' */
		firstistempo = FALSE;
		(*((*pp_c)[a]))[ic++] = TEND;
		(*((*pp_c)[a]))[ic++] = TEND;
		if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
			result = ABORT; goto SORTIR;
			}
		(*p_pos) = i;
		if(foundtokens) imbedded = FALSE;
		if(trace_polymake) BPPrintMessage(0,odInfo,"} foundtokens = %ld ic = %ld (*p_useful)[a] = %ld\n",(long)foundtokens,(long)ic,(long)(*p_useful)[a]);
		goto END;
		}
	if(m == T0 && (p == 14 || p == 7)) {		/* Either ',' or period */
		(*((*pp_c)[a]))[ic++] = TEND;
		(*((*pp_c)[a]))[ic++] = TEND;
		if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
			result = ABORT; goto SORTIR;
			}
		foundtokens = TRUE;
		
		a++;	/* Next field */
		if(a >= k) {
			my_sprintf(Message,"=> Err in PolyExpand() k = %ld",(long)k);
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
		scaling = oldscaling;
		speed = oldspeed;
		prevscale = scaling;
		prevspeed = speed;
		if(scaling >= 1. || scaling == 0.) {
			y = modf((scaling / (double)TOKBASE),&x); 
			(*((*pp_c)[a]))[ic++] = T0;
			(*((*pp_c)[a]))[ic++] = 21;	/* '*' scaling up */
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte)  scaling - (((tokenbyte) x) * TOKBASE);
			}
		else {
			y = modf(((1. / scaling) / (double)TOKBASE),&x);
			(*((*pp_c)[a]))[ic++] = T0;
			(*((*pp_c)[a]))[ic++] = 24;	/* '**' scaling down */
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte)  (1./scaling) - (((tokenbyte) x) * TOKBASE);
			}
		if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
			result = ABORT; goto SORTIR;
			}
		if(speed >= 1.) {
			(*((*pp_c)[a]))[ic++] = T0;
			(*((*pp_c)[a]))[ic++] = 11;	/* '/' speed up */
			y = modf((speed / (double)TOKBASE),&x);
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte) speed - (((tokenbyte) x) * TOKBASE);
			}
		else {
			(*((*pp_c)[a]))[ic++] = T0;
			(*((*pp_c)[a]))[ic++] = 25;	/* speed down */
			y = modf((1. / speed / (double)TOKBASE),&x);
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte) x;
			(*((*pp_c)[a]))[ic++] = T1;
			(*((*pp_c)[a]))[ic++] = (tokenbyte) (1./speed) - (((tokenbyte) x) * TOKBASE);
			}
		if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
			result = ABORT; goto SORTIR;
			}
		continue;
		}
	if(period) {
		if(m == T3 || m == T25) (*p_useful)[a] = foundtokens = TRUE;
		}
	else {	/* comma */
	//	if((m == T3 && p > 1) || m == T25 || (m >= T7 && m <= T43))
		if((m == T3 && p >= 1) || m == T25 || (m >= T7 && m <= T43) || m == T1) // Fixed by BB 2021-01-28
			(*p_useful)[a] = foundtokens = TRUE;
		}
	if(m == T3 || m == T7 || m == T8 || m == T9 || m == T25) {
		/* Terminal or prolongation or out-time object or synchronization tag... */
		/* ... or time pattern or simple note */
		firstistempo = FALSE;
		if(m == T3 || m == T9 || m == T25) {
			if(Add((*p_p)[a],(*p_q)[a],scaling,speed,&xp,&xq,&overflow) != OK){
				result = ABORT; goto SORTIR;
				}
			if(overflow) TellComplex();
			if(trace_polymake) BPPrintMessage(0,odInfo,"MaxFrac = %.0f, m = %d p = %d, xp = %.0f xq = %.0f\n",MaxFrac,m,p,xp,xq);
			if(xp > MaxFrac || xq > MaxFrac) {
				MakeRatio(MaxFrac,(xp/xq),&xp,&xq);
				TellComplex();
				}
			else Simplify(MaxFrac,xp,xq,&xp,&xq);;
			if(trace_polymake) BPPrintMessage(0,odInfo,"xp = %.0f xq = %.0f\n",xp,xq);
			
			(*p_p)[a] = xp; (*p_q)[a] = xq;
			}
		(*((*pp_c)[a]))[ic++] = (tokenbyte) m;
		(*((*pp_c)[a]))[ic++] = (tokenbyte) p;
		if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
			result = ABORT; goto SORTIR;
			}
		(*p_empty)[a] = FALSE;
		}
	else {
		if((m == T4 || m == T6 || (m == T0 && p == 1))) {
		/*	if(ShowMessages && (m != T4 || MaxVar > 0) && (ShowGraphic || !Improvize)) {
				switch(m) {  // Suppressed 2024-06-19
					case T4:
						my_sprintf(Message,
							"Variable '%s' found and ignored (field level %ld)",*((*p_Var)[p]),
								(long)a);
						break;
					case T6:
						my_sprintf(Message,
							"Unreplaced wild card '?%ld' found and ignored (field level %ld)",
								(long)p,(long)a);
						break;
					case T0:
						my_sprintf(Message,
							"Unreplaced wild card '?' found and ignored (field level %ld)",
								(long)a);
						break;
					}
				ShowMessage(TRUE,wMessage,Message);
				} */
			}
		else {
			if((m >= T10 && m <= T24) || (m >= T26 && m < MAXTOKENBYTE) || (m == T0
															&& (p == 18 || p == 19))) {
			/* '&' or tool or performance control */
				(*((*pp_c)[a]))[ic++] = (tokenbyte) m;
				(*((*pp_c)[a]))[ic++] = (tokenbyte) p;
				(*p_empty)[a] = FALSE;
				foundtokens = TRUE;
				if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
					result = ABORT; goto SORTIR;
					}
				}
			}
		}
	}
(*((*pp_c)[a]))[ic++] = TEND;
(*((*pp_c)[a]))[ic++] = TEND;
if(Check_ic(ic,p_maxic,a,pp_c) != OK) {
	result = ABORT; goto SORTIR;
	}

END:
if((a+1) != k) {
	my_sprintf(Message,"=> Err. PolyExpand(). a+1=%ld k=%ld",
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
		a0 = a;		/* The duration field is the leftmost one... */
					/* ... satisfying non-empty conditions */
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
			result = MISSED; goto SORTIR;
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
			result = MISSED; goto SORTIR;
			}
		if(fixlength && (isequal != TRUE)) {
			if(comma) my_sprintf(Message,"Conflicting field duration (field %ld)...\n",
				(long)(a+1L));
			else my_sprintf(Message,"Conflicting beat duration (beat %ld)...\n",
				(long)(a+1L));
			Print(wTrace,Message);
			result = MISSED; goto SORTIR;
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
	goto SORTIR;
	}
	
for(a=0; a < k; a++) {	/* Calculate undetermined rests */
	if((*p_vargap)[a] > 0) {
		restart = TRUE;
		Substract(pmax,qmax,(*p_p)[a],(*p_q)[a],&xp,&xq,&sign,&overflow);
		if(overflow) TellComplex();
		if(sign < 0 || xp < 1.) {
			if((*ptr_fixtempo)[a0]) {
				if(comma)
					my_sprintf(Message,"Not enough time for undetermined rest (field %ld)\n",
						(long)(a+1L));
				else
					my_sprintf(Message,"Not enough time for undetermined rest (beat %ld)\n",
						(long)(a+1L));
				Print(wTrace,Message);
				result = MISSED; goto SORTIR;
				}
			else {
				gcd = GCD((*p_p)[a],(double)(*p_vargap)[a]);
				if(gcd < 1L) gcd = 1L;
				(*p_p)[a] = (*p_p)[a] / gcd;
				(*p_q)[a] = (*p_q)[a] * ((*p_vargap)[a] / gcd);
				if((lcm=LCM(qmax,(*p_q)[a],&overflow)) < 1.) {
					Println(wTrace,"Unexpected overflow in polymetric formula (case 2). You may send this item to the designers...");
					result = ABORT; goto SORTIR;
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
		result = ABORT; goto SORTIR;
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
		result = ABORT; goto SORTIR;
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
			goto SORTIR;
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

scaling = prevscale = oldscaling;
speed = prevspeed = oldspeed;
kmax = 0;

for(a=0; a < k; a++) {
	if((*p_empty)[a]) continue;
	if(comma && !(*p_useful)[a]) continue;
	if(comma && a > a1) {
		(**pp_a)[id++] = T0;
		(**pp_a)[id++] = 14; /* ',' */
		if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto SORTIR;
		}
	scaling = rescale * oldscaling;
	speed = oldspeed * (*p_r)[a];
	if(speed > MaxTempo || scaling > MaxTempo) {
		MakeRatio(MaxTempo,(scaling/speed),&scaling,&speed);
		TellComplex();
		}
	else Simplify(MaxTempo,scaling,speed,&scaling,&speed);
	if(scaling < InvMaxTempo) scaling = 0.;
	if(comma || a == 0 || scaling != prevscale || speed != prevspeed) {
		if(scaling >= 1. || scaling == 0.) {
			y = modf((scaling / (double)TOKBASE),&x);
			(**pp_a)[id++] = T0;
			(**pp_a)[id++] = 21;	/* '*' scaling up */
			(**pp_a)[id++] = T1;
			(**pp_a)[id++] = (tokenbyte) x;
			(**pp_a)[id++] = T1;
			(**pp_a)[id++] = (tokenbyte)  scaling - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
			}
		else {
			y = modf(((1. / scaling) / (double)TOKBASE),&x);
			(**pp_a)[id++] = T0;
			(**pp_a)[id++] = 24;	/* '**' scaling down */
			(**pp_a)[id++] = T1;
			(**pp_a)[id++] = (tokenbyte) x;
			(**pp_a)[id++] = T1;
			(**pp_a)[id++] = (tokenbyte)  (1./scaling) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
			}
		if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto SORTIR;
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
		if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto SORTIR;
		}
	
	prevscale = scaling;
	prevspeed = speed;
	space = forceshowtempo = FALSE;
	
	for(ic=ZERO; ; ic+=2L) {
		if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto SORTIR;
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
				case 21:	/* '*' scaling up */
					s = GetScalingValue(((*pp_c)[a]),ic);
					scaling = rescale * s;
					ic += 4L;
					break;
				case 24:	/* '**' scaling down */
					s = GetScalingValue(((*pp_c)[a]),ic);
					scaling = rescale / s;
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
					xq = scaling;
					if(speed > MaxTempo || scaling > MaxTempo) {
						MakeRatio(MaxTempo,(scaling/speed),&xq,&xp);
						TellComplex();
						}
					else Simplify(MaxTempo,xq,xp,&xq,&xp);
					if(xq < InvMaxTempo) xq = 0.;
					isequal = Equal(0.005,xq,xp,prevscale,prevspeed,&overflow);
					if(isequal == ABORT) {
						if(Beta) {
							Alert1("=> Err. PolyExpand(). isequal == ABORT");
							result = ABORT; goto SORTIR;
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
							(**pp_a)[id++] = 21;	/* '*' scaling up */
							(**pp_a)[id++] = T1;
							(**pp_a)[id++] = (tokenbyte) x;
							(**pp_a)[id++] = T1;
							(**pp_a)[id++] = (tokenbyte)  xq - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
							}
						else  {
							y = modf((1. / xq / (double)TOKBASE),&x);
							(**pp_a)[id++] = T0;
							(**pp_a)[id++] = 24;	/* '**' scaling down */
							(**pp_a)[id++] = T1;
							(**pp_a)[id++] = (tokenbyte) x;
							(**pp_a)[id++] = T1;
							(**pp_a)[id++] = (tokenbyte)  (1. / xq) - (((tokenbyte) x) * TOKBASE); /* instead of (y * TOKBASE), fixed by BB 21 May 2007 */
							}
						if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto SORTIR;
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
						if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto SORTIR;
						}
					ic += 4L;
					break;
				}
			}
		else {
COPYIT:
			(**pp_a)[id++] = (tokenbyte) m;
			(**pp_a)[id++] = (tokenbyte) p;
			if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto SORTIR;
			}
		}
	kmax++;
	}

(**pp_a)[id++] = TEND; (**pp_a)[id++] = TEND;
if((result=CheckSize(id,p_maxid,pp_a)) != OK) goto SORTIR;

if(Beta && level != ZERO) {
	Alert1("=> Err. PolyExpand(). level != ZERO (end)");
	}

result = OK;
if(kmax == 1) result = SINGLE; /* Indicates that structure had one single field */

SORTIR:

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


int Check_ic(unsigned long ic,unsigned long **p_maxic,int a,tokenbyte ****pp_c)
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


int TellComplex(void)
{
if(!SaidTooComplex) {
	SaidTooComplex = TRUE;
//	BPPrintMessage(0,odInfo,"Formula is complex. Roundings are performed... (ULONG_MAX = %ul)\n",ULONG_MAX);
	BPPrintMessage(0,odInfo,"Formula is complex. Roundings are performed\n");
	}
return(OK);
}


int CheckSize(unsigned long i,unsigned long *p_maxi,tokenbyte ***pp_a)
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