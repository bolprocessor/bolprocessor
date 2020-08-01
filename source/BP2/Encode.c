/* Encode.c (BP2 version CVS) */

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

tokenbyte **Encode(int sequence,int notargument, int igram, int irul, char **pp1, char **pp2,
	p_context *p_pleftcontext, p_context *p_prightcontext, int *p_meta, int arg_nr,
	p_flaglist ***ph_flag,int quick,int *p_result)
// arg_nr = 0: item
// arg_nr = 1: left argument (grammar)
// arg_nr = 2: right argument (grammar)
// arg_nr = 4: left argument (glossary)
// arg_nr = 8: right argument (glossary)
{
tokenbyte **p_buff,**p_pi;
int ii,ig,ir,j,jj,n,l,ln,lmax,bound,leftside,rightcontext,neg,cv,needsK,needsflag;
long i,imax,k,siz,buffsize,y,u,v;
char c,d,**pp,*p,*q,*qmax,*r,line[MAXLIN],**p_x;
p_flaglist **nexth,**oldh;
KeyNumberMap map;
double x;

*p_result = OK;
if(p_Script == NULL && GetScriptSpace() != OK) return(NULL);
if(p_Flagname == NULL && GetFlagSpace() != OK) return(NULL);
if(p_Var == NULL && GetVariableSpace() != OK) return(NULL);	/* Added 8/3/98 */
for(i=0,p=(*pp1); p < (*pp2); i++,p++){};
pp = pp1;
imax = 4L * i + 6L;
buffsize = imax + 4L;
if((p_buff = (tokenbyte**) GiveSpace((Size) buffsize*sizeof(tokenbyte))) == NULL)
							return(NULL);
if((p_x = (char**) GiveSpace((Size)((BOLSIZE+2) * sizeof(char)))) == NULL) {
	return(NULL);
	}
leftside = TRUE;
rightcontext = bound = neg = FALSE;
i = 0;
if(arg_nr == 0) {
	for(j=0; j < WMAX; j++) {
		if(FilePrefix[j][0] == '\0') continue;
		q = &(FilePrefix[j][0]);
		if(Match(TRUE,pp1,&q,4)) goto FINISHED;
		}
	}
for(; (*pp) <= (*pp2);) {
	if(i % 25 == 0) PleaseWait();
#if BP_CARBON_GUI
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	if((SelectOn || CompileOn) && ((*p_result)=MyButton(2)) != FAILED) {
		if((*p_result) != OK || ((*p_result)=InterruptCompile()) != OK) goto ERR;
		PleaseWait();
		}
	(*p_result) = OK;
	if(EventState != NO) goto ERR;
#endif /* BP_CARBON_GUI */
	while(GetNilString(pp) == OK){}; /* Skip "lambda" */
	c = NextChar(pp);
	if(isdigit(c)) {
SEARCHNUMBER:
		if((k=FindNumber(pp)) != -1) {
			(*p_buff)[i++] = T1;
			(*p_buff)[i++] = (tokenbyte)((k - (k % TOKBASE)) / TOKBASE);
			(*p_buff)[i++] = T1;
			(*p_buff)[i++] = (tokenbyte)(k % TOKBASE);
			if(i > imax) {
				if(Beta) Alert1("i > imax. Err. Encode()");
				goto ERR;
				}
			c = NextChar(pp);
			continue;
			}
		}
	if(c == '*') {
		(*pp)++;
		if(!isdigit(**pp)) {
			(*pp)--;
			goto SEARCHHOMO;
			}
		(*p_buff)[i++] = T0;
		(*p_buff)[i++] = 21; /* '*' scale up */
		c = (**pp);
		if(c == '*') {
			(*p_buff)[--i] = 24; /* '**' scale down */
			i++;
			}
		c = NextChar(pp);	
		continue;
		}
		
NOTSCALE:
	if(c == 'Â') {
		(*pp)++;
		while(isspace(c=**pp) && (*pp) < (*pp2)) (*pp)++;
		continue;
		}
	if(c == '\n' || c == '\r' || c == '\0') break;
	if(c == '[') {
		if(arg_nr == 0 || arg_nr == 2 || arg_nr == 8) break;
		else {
			sprintf(Message,"Misplaced comment in left argument\n");
			Print(wTrace,Message);
			goto ERR;
			}
		}
	if(c == '>') {
		sprintf(Message,
			"Found illegal character '>'...\n");
		Print(wTrace,Message);
		goto ERR;
		}
	if(rightcontext) {	/* Remote right context has been read */
		sprintf(Message,
			"Can't make sense of expression between parentheses. May be found incorrect grammar procedure, performance control, or misplaced right context...\n");
		Print(wTrace,Message);
		goto ERR;
		}
	if(c == '-') goto SEARCHTERMINAL2;
	if(c == '_') {
		j = RESUME;
		if((j=GetPerformanceControl(pp,arg_nr,&n,quick,&u,&v,&map)) == ABORT) {
			goto ERR;
			}
		if(j != RESUME) {
			switch(j) {
				case 0:	/* _chan() */
				case 1:	/* _vel() */
					(*p_buff)[i++] = T10 + (tokenbyte) j; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 2:	/* _velstep */
				case 3:	/* _velcont */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte)(j - 2);
					break;
				case 4: /* _script() */
					(*p_buff)[i++] = T13; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 5:	/* _mod() */
					(*p_buff)[i++] = T14; (*p_buff)[i++] = (tokenbyte)n;
					break;
				case 6:	/* _modstep */
				case 7:	/* _modcont */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte)(j - 4);
					break;
				case 8:	/* _pitchbend */
					(*p_buff)[i++] = T15; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 9:	/* _pitchstep */
				case 10: /* _pitchcont */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte)(j - 5);
					break;
				case 11: /* _press */
					(*p_buff)[i++] = T16; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 12: /* _presstep */
				case 13: /* _presscont */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte)(j - 6);
					break;
				case 14: /* _switchon */
					(*p_buff)[i++] = T17; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 15: /* _switchoff */
					(*p_buff)[i++] = T18; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 16: /* _volume */
					(*p_buff)[i++] = T19; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 17: /* _volumestep */
				case 18: /* _volumecont */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte)(j - 9);
					break;
				case 19:	/* _legato() */
					(*p_buff)[i++] = T20; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 20:	/* _staccato() */
					(*p_buff)[i++] = T20; (*p_buff)[i++] = (tokenbyte)(- n);
					break;
				case 21: /* _articulstep */
				case 22: /* _articulcont */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte)(j - 11);
					break;
				case 23: /* _velfixed */
				case 24: /* _modfixed */
				case 25: /* _pitchfixed */
				case 26: /* _pressfixed */
				case 27: /* _volumefixed */
				case 28: /* _articulfixed */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte)(j - 11);
					break;
				case 29: /* _pitchrange */
					(*p_buff)[i++] = T21; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 30: /* _pitchrate */
					(*p_buff)[i++] = T22; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 31: /* _modrate */
					(*p_buff)[i++] = T23; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 32: /* _pressrate */
					(*p_buff)[i++] = T24; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 33: /* _transpose */
					(*p_buff)[i++] = T26; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 34: /* _volumerate */
					(*p_buff)[i++] = T27; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 35: /* _volumecontrol */
					(*p_buff)[i++] = T28; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 36: /* _pan */
					(*p_buff)[i++] = T29; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 37: /* _panstep */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte) 19;
					break;
				case 38: /* _pancont */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte) 20;
					break;
				case 40: /* _panrate */
					(*p_buff)[i++] = T30; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 41: /* _pancontrol */
					(*p_buff)[i++] = T31; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 42: /* _rest, i.e. 'É' */
					(*p_buff)[i++] = T0; (*p_buff)[i++] = 17;
					break;
				case 43: /* _ins() */
					(*p_buff)[i++] = T32; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 44: /* _value() */
					(*p_buff)[i++] = T35; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 45: /* _step() */
					(*p_buff)[i++] = T33; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 46: /* _cont() */
					(*p_buff)[i++] = T34; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 47: /* _fixed() */
					(*p_buff)[i++] = T36; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 48: /* _retro */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte) 21;
					if(arg_nr == 2) NotBPCase[7] = TRUE;
					break;
				case 49: /* _rndseq */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte) 22;
					if(arg_nr == 2) NotBPCase[7] = TRUE;
					break;
				case 50: /* _randomize */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte) 23;
					if(arg_nr < 4) WillRandomize = TRUE;
					break;
				case 51: /* _ordseq */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte) 24;
					break;
				case 52: /* _keymap */
					(*p_buff)[i++] = T37; (*p_buff)[i++] = (tokenbyte)(map.p1 + 128 * map.q1);
					(*p_buff)[i++] = T37; (*p_buff)[i++] = (tokenbyte)(map.p2 + 128 * map.q2);
					break;
				case 53: /* _mapfixed */
				case 54: /* _mapcont */
				case 55: /* _mapstep */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte)(j - 28);
					break;
				case 56:	/* _rndvel() */
					(*p_buff)[i++] = T38; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 57:	/* _rotate() */
					(*p_buff)[i++] = T39; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 58: /* _keyxpand() */
					(*p_buff)[i++] = T40; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 59: /* _rndtime() */
					(*p_buff)[i++] = T41; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 60: /* _srand() */
					(*p_buff)[i++] = T42; (*p_buff)[i++] = (tokenbyte) n;
					break;
				case 61: /* _tempo */
					(*p_buff)[i++] = T43; (*p_buff)[i++] = (tokenbyte) u;
					(*p_buff)[i++] = T43; (*p_buff)[i++] = (tokenbyte) v;
					break;
				case 62: /* _transposefixed */
				case 63: /* _transposecont */
				case 64: /* _transposestep */
					(*p_buff)[i++] = T12; (*p_buff)[i++] = (tokenbyte)(j - 34);
					break;
				}
			c = NextChar(pp);
			continue;
			}
		j = GetProcedure(igram,pp,arg_nr,&ig,&ir,&x,&y);
		switch(j) {
			case 13:
			case 14:
			case 15:
				if(ChangeMetronom(j,x) != OK) goto ERR;
				c = NextChar(pp);
				continue;
				break;
			}
		if(!notargument && j == ABORT) {
			goto ERR;
			}
		if(!notargument && j >= 0) {
			if(igram < 1 || Gram.p_subgram == NULL || igram > Gram.number_gram) {
				if(Beta) Alert1("Err. Encode(). igram < 1 || Gram.p_subgram == NULL || igram > Gram.number_gram");
				goto ERR;
				}
			if((ii=(*(Gram.p_subgram))[igram].type) == SUBtype || ii == SUB1type || ii == POSLONGtype) {
				sprintf(Message,
					"Can't accept rule procedure '_goto', etc., in SUB or SUB1 or POSLONG subgrammar\n");
				Print(wTrace,Message);
				goto ERR;
				}
			Gram.hasproc = TRUE;
			switch(j) {
				case 0:	/* _goto */
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].gotogram = ig;
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].gotorule = ir;
					break;
				case 1:	/* _failed */
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].failedgram = ig;
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].failedrule = ir;
					break;
				case 2:	/* _repeat */
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].repeat = ig;
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].repeatcontrol = ir;
					break;
				case 3:	/* _stop */
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].stop += arg_nr;
					break;
				case 4:	/* _print */
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].print += arg_nr;
					break;
				case 5:	/* _printOn */
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].printon += arg_nr;
					break;
				case 6:	/* _printOff */
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].printoff += arg_nr;
					break;
				case 7:	/* _stepOn */
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].stepon += arg_nr;
					break;
				case 8:	/* _stepOff */
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].stepoff += arg_nr;
					break;
				case 9:	/* _traceOn */
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].traceon += arg_nr;
					break;
				case 10:	/* _traceOff */
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].traceoff += arg_nr;
					break;
				case 11:	/* _destru */
					(*((*(Gram.p_subgram))[igram].p_rule))[irul].destru = TRUE;
					break;
				}
			c = NextChar(pp);
			continue;
			}
		goto SEARCHTERMINAL2;	/* Found "_" */
		}
//	if(isupper(c)) goto SEARCHCODE;
	if(islower(c)) goto SEARCHNOTE;
	if(c == '/') {		/* Look for /flag/ */
		q = *pp;
		c = *(++q);
		if(c == '/' && arg_nr == 0) { /* Remark starting with "//" */ 
			break;
			}
		if(!isdigit(c)) { /*  not a tempo marker */
			if(arg_nr == 0 || arg_nr > 2) {
				sprintf(Message,"Slash is used to denote flags in grammar rules only!\n");
				Print(wTrace,Message);
				goto ERR;
				}
			(*pp)++; NextChar(pp);
			l = 0;
			while(!MySpace(c=**pp) && c != '/' && c != '=' && c != '-' && c != '+'
					&& c != '<' && c != '\334' && c != '>' && c != '\335'
					&& c != '²' && c != '³' && c != '­') {
				(*p_x)[l++] = c;
				if(l >= BOLSIZE) {
					ShowError(4,igram,irul);
					sprintf(Message,"Max flag length: %ld chars!\n",(long)BOLSIZE);
					Print(wTrace,Message);
					goto ERR;
					}
				(*pp)++;
				if((*pp) > (*pp2)) {
					ShowError(11,igram,irul);
					goto ERR;
					}
				}
			(*p_x)[l++] = '\0';
			StripHandle(p_x);
			if((jj = CreateFlag(p_x)) == ABORT) {
				goto ERR;
				}
			if(MySpace(c)) c = NextChar(pp);
			needsK = needsflag = FALSE;
			if(c == '=' || c == '-' || c == '+' || c == '>' || c == '<' || c == '\334'
							 || c == '\335'	|| c == '²' || c == '³' || c == '­') {
				if(c == '=' && arg_nr != 1 && arg_nr != 2) {
					ShowError(50,igram,irul);
					goto ERR;
					}
				(*pp)++; NextChar(pp);
				if(c == '>' || c == '<' || c == '\334' || c == '\335' || c == '²'
						|| c == '³' || c == '­' || c == '=') {
					if(c != '='	&& arg_nr != 1) {
						ShowError(51,igram,irul);
						goto ERR;
						}
					if(**pp == 'K') {
						needsK = TRUE;
						(*pp)++; NextChar(pp);
						}
					j = 0;
					if(needsK && !isdigit(**pp)) (*p_x)[j++] = 'K';
					if(!isdigit(**pp)) {
						while(!MySpace(d=**pp) && d != '/') {
							(*p_x)[j++] = d;
							if(j >= BOLSIZE) {
								ShowError(4,igram,irul);
								sprintf(Message,"Max flag length: %ld chars!\n",(long)BOLSIZE);
								Print(wTrace,Message);
								goto ERR;
								}
							(*pp)++;
							if((*pp) > (*pp2)) {
								ShowError(11,igram,irul);
								goto ERR;
								}
							}
						(*p_x)[j++] = '\0';
						StripHandle(p_x);
						if((n = CreateFlag(p_x)) == ABORT) {
							goto ERR;
							}
						if(MySpace(d)) d = NextChar(pp);
						needsflag = TRUE;
						goto STOREFLAG;
						}
					}
				j = 0;
				MystrcpyHandleToString(MAXLIN,0,line,pp);
				if((n=GetInteger(YES,line,&j)) == INT_MAX) {
					ShowError(53,igram,irul);
					goto ERR;
					}
				(*pp) += j;
				if(NextChar(pp) != '/') {
					ShowError(52,igram,irul);
					goto ERR;
					}
				}
			/* Append new flag to list */
STOREFLAG:
			oldh = nexth = *(ph_flag);
			while(nexth != NULL) {
				oldh = nexth;
				nexth = (**nexth).p;
				}
 			if((nexth=(p_flaglist**) GiveSpace((Size)sizeof(p_flaglist))) == NULL)
				goto ERR;
			(**nexth).p = NULL;
			(**nexth).x = jj;
			(**nexth).refvalue = n;
			(**nexth).paramcontrol = 0;
			if(needsK) (**nexth).paramcontrol = 1;
			if(needsflag) (**nexth).paramcontrol = 2;
			if(arg_nr == 2) (**nexth).increment = 1;
			if(arg_nr == 1) (**nexth).increment = 0;
			if(c == '+') (**nexth).increment = n;
			if(c == '-') (**nexth).increment = - n;
			(**nexth).operator = OFF;
			if(c == '=') {
				if(arg_nr == 1)  (**nexth).operator = EQUAL;
				if(arg_nr == 2) (**nexth).operator = ASSIGN;
				}
			if(arg_nr == 1) {
				switch(c) {
					case '­':
						(**nexth).operator = DIF;
						break;
					case '<':
					case '\334':
						(**nexth).operator = INF;
						break;
					case '>':
					case '\335':
						(**nexth).operator = SUP;
						break;
					case '³':
						(**nexth).operator = SUPEQUAL;
						break;
					case '²':
						(**nexth).operator = INFEQUAL;
						break;
					}
				}
			if(*(ph_flag) == NULL) *(ph_flag) = nexth;
			else (**oldh).p = nexth;
			(*pp)++;
			c = NextChar(pp);
			continue;
			}
		}
	c = (**pp);
	if(c == '<' || c == '\334') {		/* Out-time object or <<Wx>> */
		(*pp)++;
		c = (**pp);
		if((c != '<' && c != '\334')) {
			Expect('<',"<",c);
			ShowError(41,igram,irul);
			goto ERR;
			}
		(*pp)++;
		if((c=(**pp)) == 'W') {	/* <<Wx>> */
			(*pp)++;
			l = 0;
			while((c=(**pp)) != '>' && c != '\335') {
				if(!isdigit(c)) {
					sprintf(Message,
				"Expecting integer in synchonization tag '<<Wx>>'\n");
					Print(wTrace,Message);
					ShowError(42,igram,irul);
					goto ERR;
					}
				l = 10 * l + (c - '0');
				(*pp)++;
				}
			if(l > MAXWAIT){
				sprintf(Message,
			"Tag '<<W%ld>>' inacceptable.  Not more than %ld tags allowed\n",
					(long)l,(long)MAXWAIT);
				Print(wTrace,Message);
				ShowError(42,igram,irul);
				
				goto ERR;
				}
			(*p_buff)[i++] = T8; (*p_buff)[i++] = l;
			(*pp)++;
			if((c=(**pp)) != '>' && c != '\335') {
				Expect('>',"synchronization tag",c);
				ShowError(41,igram,irul);
				goto ERR;
				}
			(*pp)++;
			c = NextChar(pp);
			continue;
			}
		l = 0;
		if(!OkBolChar(c=NextChar(pp))) {
			(*p_x)[l++] = c;
			(*p_x)[l] = '\0';
			sprintf(Message,
			"Terminal <<%s...>> starts with incorrect character '%c'\n",(*p_x),c);
			Print(wTrace,Message);
			ShowError(27,igram,irul);
			goto ERR;
			}
		while((c=NextChar(pp)) != '>' && c != '\335') {
			(*p_x)[l++] = c;
			if(!OkBolChar2(c) || c == '-') {
				(*p_x)[l] = '\0';
				sprintf(Message,
				"Terminal <<%s...>> contains incorrect character '%c'\n",(*p_x),c);
				Print(wTrace,Message);
				ShowError(27,igram,irul);
				goto ERR;
				}
			if(l >= BOLSIZE) {
				(*p_x)[l] = '\0';
				sprintf(Message,"Terminal <<%s...>> is too long. Max length: %ld chars.\n",
					(*p_x),(long)BOLSIZE);
				Print(wTrace,Message);
				ShowError(22,igram,irul);
				
				goto ERR;
				}
			(*pp)++;
			if((*pp) > (*pp2)) {
				ShowError(11,igram,irul);
				goto ERR;
				}
			}
end1:	(*pp)++;
		if((c=(**pp)) != '>' && c != '\335') {
			Expect('>',">",c);
			ShowError(41,igram,irul);
			goto ERR;
			}
		(*pp)++;
		(*p_x)[l++] = '\0';
		
		/* May be it's an out-time simple note */
		cv = NoteConvention;
		lmax = 0;
		for(j=0; j < 128; j++) {
			ln = (*(p_NoteLength[cv]))[j];
			if(Match(TRUE,p_x,(*(p_NoteName[cv]))[j],ln) && !isdigit((*p_x)[ln])) {
				lmax = ln; jj = j;
				qmax = q + ln;
				goto FOUNDNOTE1;
				}
			}
		for(j=0; j < 128; j++) {
			ln = (*(p_AltNoteLength[cv]))[j];
			if(Match(TRUE,p_x,(*(p_AltNoteName[cv]))[j],ln) && !isdigit((*p_x)[ln])) {
				lmax = ln; jj = j;
				qmax = q + ln;
				goto FOUNDNOTE1;
				}
			}
FOUNDNOTE1:
		if(lmax  > 0) {
			leftside = neg = FALSE;
			(*p_buff)[i++] = T7;
			jj += (C4key - 60);
			if(jj < 0 || jj > 127) {
				Alert1("Simple note is out of range. (May be check \"Tuning\")");
				ShowWindow(GetDialogWindow(TuningPtr));
				SelectWindow(GetDialogWindow(TuningPtr));
				goto ERR;
				}
			(*p_buff)[i++] = (tokenbyte) (jj + 16384);
			if(i > imax) {
				if(Beta) Alert1("i > imax. Err. Encode()");
				goto ERR;
				}
			c = NextChar(pp);
			continue;
			}
		
		/* It must be an out-time sound-object */
		if(l >= BOLSIZE) {
			ShowError(4,igram,irul);
			sprintf(Message,"Max length: %ld chars!\n",(long)BOLSIZE);
			Print(wTrace,Message);
			goto ERR;
			}
		if((jj=CreateBol(TRUE,FALSE,p_x,FALSE,TRUE,BOL)) < 0) {
			goto ERR;
			}
		if(jj >= Jbol) {
			if(Beta) Alert1("Err. Encode(). jj >= Jbol");
			
			goto ERR;
			}
		for(ii=0; ii < Jhomo; ii++) (*((*p_Image)[ii]))[jj] = (tokenbyte) jj;
		(*p_buff)[i++] = T7; (*p_buff)[i++] = (tokenbyte) jj;
		if(i > imax) {
			if(Beta) Alert1("i > imax. Err. Encode()");
			
			goto ERR;
			}
		leftside = neg = FALSE;
		c = NextChar(pp);
		continue;
		}

SEARCHCONTEXT:	
	if(c == '(') {
		d = (*pp)[1];
		if((d != '=') && (d != ':')) {		/* Context */
			if(arg_nr != 1) {
				sprintf(Message,
					"Remote context should only be in left argument of grammar rule. May be misspelled '_goto','_failed','_chan'...\n");
				Print(wTrace,Message);
				goto ERR;
				}
			if(neg) {	/* erase preceding '#' */
				i--;i--;
				}
			if(leftside) {
				if(GetContext(igram,irul,pp,pp2,
					p_pleftcontext,p_meta) == FAILED) {
					sprintf(Message,
						"Error in left context!\n");
					Print(wTrace,Message);
					goto ERR;
					}
				leftside = 0;
				(**p_pleftcontext)->sign = 1 - neg;
				}
			else {
				if(GetContext(igram,irul,
					pp,pp2,p_prightcontext,p_meta) == FAILED) {
					sprintf(Message,
						"Error in right context!\n");
					Print(wTrace,Message);
					goto ERR;
					}
				rightcontext = TRUE;
				(**p_prightcontext)->sign = 1 - neg;
				}
			neg = FALSE;
			c = NextChar(pp);
			continue;
			}
		}
	if(c == '?') {
		(*pp)++;
		leftside = neg = FALSE;
		if((d = (**pp)) < '0' || d > '9') {
			(*pp)--;
			j = 1;
			goto OKCODE;
			}
		else {				/* found '?n' */
			n = 0;
			*p_meta = 1;
			(*p_buff)[i++] = T6;
			if(i > imax) {
				if(Beta) Alert1("i > imax. Err. Encode()");
				goto ERR;
				}
			while(d >= '0' && d <= '9') {
				n = (10 * n) + d - '0';
				if(n > MAXMETA) {
					sprintf(Message,"Maxi %ld wildcards!\n",(long)MAXMETA);
					Print(wTrace,Message);
					ShowError(28,igram,irul);
					
					goto ERR;
					}
				(*pp)++; d = (**pp);
				}
			(*p_buff)[i++] = (tokenbyte) n;
			if(i > imax) {
				if(Beta) Alert1("i > imax. Err. Encode()");
				
				goto ERR;
				}
			c = NextChar(pp);
			continue;
			}
		}
		
SEARCHNOTE:
	/* Look for simple note in current convention */
	lmax = 0;
	for(j=0; j < 128; j++) {
		q = *pp; l = (*(p_NoteLength[NoteConvention]))[j];
		if(Match(TRUE,&q,(*(p_NoteName[NoteConvention]))[j],l)
				&& !isdigit(q[l])) {
			lmax = l; jj = j;
			qmax = q + l;
			goto FOUNDNOTE2;
			}
		}
	for(j=0; j < 128; j++) {
		q = *pp; l = (*(p_AltNoteLength[NoteConvention]))[j];
		if(Match(TRUE,&q,(*(p_AltNoteName[NoteConvention]))[j],l)
				&& !isdigit(q[l])) {
			lmax = l; jj = j;
			qmax = q + l;
			goto FOUNDNOTE2;
			}
		}
	if(lmax  > 0) {
FOUNDNOTE2:
		leftside = neg = FALSE;
		(*p_buff)[i++] = T25;
		jj += (C4key - 60);
		if(jj < 0 || jj > 127) {
			Alert1("Simple note is out of range. (May be check \"Tuning\")");
			ShowWindow(GetDialogWindow(TuningPtr));
			SelectWindow(GetDialogWindow(TuningPtr));
			goto ERR;
			}
		(*p_buff)[i++] = (tokenbyte) jj;
		*pp = qmax;
		if(i > imax) {
			if(Beta) Alert1("i > imax. Err. Encode()");
			goto ERR;
			}
		if((**pp) == '&') bound = TRUE;
		c = NextChar(pp);
		continue;
		}

SEARCHTERMINAL:
	if(OkBolChar(c)) {
		
SEARCHTERMINAL2:
		lmax = 0;		/* Look for terminal symbol */
		for(j=0; j < Jbol; j++) {
			q = *pp;
			l = MyHandleLen((*p_Bol)[j]);
			if(l == 0) {
				if(Beta) Alert1("Err. Encode(). l == 0");
				break;
				}
			if(Match(TRUE,&q,(*p_Bol)[j],l) && l > lmax) {
				lmax = l; jj = j;
				qmax = q + l;
				}
			}
		if(lmax  > 0) {
			leftside = neg = FALSE;
			(*p_buff)[i++] = T3; (*p_buff)[i++] = (tokenbyte) jj;
			(*pp) = qmax;
			if(i > imax) {
				if(Beta) Alert1("i > imax. Err. Encode()");
				goto ERR;
				}
			if((**pp) == '&') bound = TRUE;
			c = NextChar(pp);
			continue;
			}
		}
		
SEARCHTIMEPATTERN:
	lmax = 0;	/* Look for time pattern */
	for(j=0; j < Jpatt; j++) {
		q = *pp;
		l = MyHandleLen((*p_Patt)[j]);
		if(l == 0) {
			if(Beta) Alert1("Err. Encode(). l == 0 in SEARCHTIMEPATTERN");
			break;
			}
		if(Match(TRUE,&q,(*p_Patt)[j],l) && l > lmax) {
			lmax = l; jj = j;
			qmax = q + l;
			}
		}
	if(lmax  > 0) {
		leftside = neg = FALSE;
		(*p_buff)[i++] = T9; (*p_buff)[i++] = (tokenbyte) jj;
		*pp = qmax;
		if(i > imax) {
			if(Beta) Alert1("i > imax. Err. Encode()");
			goto ERR;
			}
		if((**pp) == '&') bound = TRUE;
		c = NextChar(pp);
		continue;
		}
	
SEARCHHOMO:
	lmax = 0;
	for(j=0; j < Jhomo; j++) {
		q = *pp; l = MyHandleLen((*p_Homo)[j]);
		if(l == 0) {
			if(Beta) Alert1("Err. Encode(). l == 0 in SEARCHHOMO");
			break;
			}
		r = *((*p_Homo)[j]);
		if(Match(TRUE,&q,&r,l) && l > lmax) {
			lmax = l; jj = j;
			qmax = q + l;
			}
		}
	if(lmax > 0) {				/* Found homomorphism */
		leftside = neg = FALSE;
		(*p_buff)[i++] = 5;
		(*p_buff)[i++] = (tokenbyte) jj;
		*pp = qmax;
		if(i > imax) {
			if(Beta) Alert1("i > imax. Err. Encode()");
			goto ERR;
			}
		c = NextChar(pp);
		continue;
		}
	
SEARCHCODE:
	neg = FALSE;
	if((j=FindCode(c)) != -1) {	/* found special char */
		if(j == 10) { 		/* 'S' */
			(*pp)++;
			if(OkChar(**pp) && (*pp) <= (*pp2)) {
				(*pp)--;
				goto SEARCHVAR;
				}
			else (*pp)--;
			}
		if(j == 18) {		/* '&' */
			if(!bound) j = 19; /* precedes terminal or variable */
			else bound = FALSE;
			}
		if(j == 2) {		/* '#' */
			neg = TRUE;
			}
		if(j == 7) {		/* '¥' */
//			NotBPCase[9] = TRUE;
			}
OKCODE:	
		(*p_buff)[i++] = T0;
		(*p_buff)[i++] = (tokenbyte) j;
		if(i > imax) {
			if(Beta) Alert1("i > imax. Err. Encode()");
			goto ERR;
			}
		(*pp)++;
		c = NextChar(pp);
		continue;
		}
	if(c == '\'') {			/* terminal between single quotes */
		(*pp)++;
		(*p_x)[0] = '\''; l = 0;
		while((c=(**pp)) != '\'') {
			if(l >= BOLSIZE-4) {
				(*p_x)[++l] = '\0';
				sprintf(Message,
				"Terminal %s...' is too long. Max length: %ld chars.\n",(*p_x),(long)BOLSIZE);
				Print(wTrace,Message);
				ShowError(22,igram,irul);
				
				goto ERR;
				}
			(*p_x)[++l] = c;
			(*pp)++;
			if((*pp) > (*pp2)) {
				(*p_x)[l] = '\0';
				sprintf(Message,"Missing single quote for terminal %s...\n",(*p_x));
				Print(wTrace,Message);
				ShowError(11,igram,irul);
				
				goto ERR;
				}
			}
		(*p_x)[++l] = '\''; (*p_x)[++l] = '\0';
		(*pp)++;
		if((jj=CreateBol(TRUE,FALSE,p_x,FALSE,TRUE,BOL)) < 0) {
			goto ERR;
			}
		if(jj >= Jbol) {
			if(Beta) Alert1("Err. Encode(). (2) jj >= Jbol");
			goto ERR;
			}
		for(ii=0; ii < Jhomo; ii++) (*((*p_Image)[ii]))[jj] = (tokenbyte) jj;
		(*p_buff)[i++] = T3; (*p_buff)[i++] = (tokenbyte) jj;
		if(i > imax) {
			if(Beta) Alert1("i > imax. Err. Encode()");
			
			goto ERR;
			}
		if((**pp) == '&') bound = TRUE;
		leftside = neg = FALSE;
		c = NextChar(pp);
		continue;
		}

SEARCHVAR:

	if(!OkChar(c) && c != '|') {
		ShowError(55,igram,irul);
		sprintf(Message,"Can't accept '%c'\n",c);
		Print(wTrace,Message);
		goto ERR;
		}
	if((j=GetVar(pp,pp2)) < 0) goto ERR;
	(*p_buff)[i++] = T4;
	(*p_buff)[i++] = (tokenbyte) j;
	if(j > 0) (*p_VarStatus)[j] = (*p_VarStatus)[j] | arg_nr;	/* j > 0 added 8/3/98 */
	if(i > imax) {
		if(Beta) Alert1("i > imax. Err. Encode()");
		goto ERR;
		}
	leftside = 0;
	if((**pp) == '&') bound = TRUE;
	if(MySpace(**pp)) bound = FALSE;
	c = NextChar(pp);
	}
	
FINISHED:

(*p_buff)[i++] = TEND; (*p_buff)[i] = TEND;
MyDisposeHandle((Handle*)&p_x);
if((i+1) > imax) {
	if(Beta) Alert1("i > imax. Err. Encode()");
	
	goto ERR;
	}
imax = (int) LengthOf(&p_buff);	// OPTIMIZE: can't we just do imax = i - (1 or 2)?
if(j=Recode(notargument,&imax,&p_buff)) {
	ShowError(j,igram,irul);
	DoSystem();
	goto ERR;
	}
if((p_pi = (tokenbyte**) GiveSpace((Size)(imax+2)*sizeof(tokenbyte))) == NULL)
	goto ERR;
siz = imax + 2L;	// FIXME: siz is never used
// OPTIMIZE: why make a copy of p_buff just to return the copy and dispose of p_buff ??
// Just resizing p_buff should be faster.
if(CopyBuf(&p_buff,&p_pi) == ABORT) p_pi = NULL;
MyDisposeHandle((Handle*)&p_buff);
if(DoSystem() != OK) p_pi = NULL;	// FIXME ? why fail just because DoSystem() does ??
if(p_pi == NULL) return(p_pi); 	// FIXME
return(p_pi);

ERR:
MyDisposeHandle((Handle*)&p_x);
MyDisposeHandle((Handle*)&p_buff);
ShowSelect(CENTRE,wTrace);
return(NULL);
}


GetContext(int igram, int irul, char **pp, char **ppmax, p_context *p_ppc, int *p_meta)
{
int levpar,result,flagindex;
char c,c1,*p1,**pp1,c2,*p2,**pp2;
p_context ppx,ppy,ptr;
tokenbyte **p;

if((ptr = (p_context) GiveSpace((Size)sizeof(t_context))) == NULL) return(ABORT);
flagindex = 0;
*p_ppc = ptr;
p1 = &c1; p2 = &c2;
pp1 = &p1; pp2 = &p2;
++(*pp);		/* skip first '(' */
*pp1 = *pp;
for(levpar = 1; (*pp) <= (*ppmax); (*pp)++) {
	c = NextChar(pp);
	if(c == '\n' || c == '[') return(FAILED);
	if(c == '(') {
		levpar++;
		continue;
		}
	if(c == ')') {
		if(--levpar == 0) break;
		}
	}
/* printf("levpar=%ld",(long)levpar); Pause(0); */
if(levpar != 0) {
	sprintf(Message,"Incorrect bracketting.\n");
	Print(wTrace,Message);
	return(FAILED);
	}
*pp2 = (*pp);
**pp2 = ' '; /* Erase ')' */
ppx = NULL; ppy = NULL;
result = OK;
if((p = Encode(FALSE,TRUE,igram,irul,pp1,pp2,&ppx,&ppy,p_meta,0,NULL,FALSE,&result))
		== NULL) {
	(*(*p_ppc))->p_arg = NULL;
	if(result < 0) return(result);
	sprintf(Message,"Can't encode remote context!");
	Print(wTrace,Message);
	if(EmergencyExit) return(ABORT);
	return(FAILED);
	}
(*(*p_ppc))->p_arg = p;
if((ppx != NULL) || (ppy != NULL)) {
	sprintf(Message,"Can't have multilayered remote context!");
	Print(wTrace,Message);
	return(FAILED);
	}
++(*pp); NextChar(pp);
return(OK);
}



long FindNumber(char **pp)
{
long n;
char c;

n = ZERO;
if((c=**pp) < '0' || c > '9') return(-1L);
while((c=**pp) >= '0' && c <= '9') {
	n = (10L * n) + (long)(c - '0');
	(*pp)++;
	}
return(n);
}


GetVar(char **pp, char **ppmax)
{
int i,im,j,l,bracket;
char c,*porg,*q,**ptr,line[MAXLIN];
Size oldsize;

bracket = FALSE;
porg = *pp;
if(**pp == '|') {
	(*pp)++; bracket = TRUE;
	}
if(p_Var != NULL && Jvar > 0) {
	for(j=1; j <= Jvar; j++) { 
		l = MyHandleLen((*p_Var)[j]);
		if(l == 0) {
			if(Beta) Alert1("Err. GetVar(). l == 0");
			break;
			}
		q = *pp;
		if(Match(TRUE,&q,(*p_Var)[j],l)) {
			q += l;
			if((*q == '|' && bracket) || (!bracket && !OkChar(*q))) {
				if(bracket) q++;
				*pp = q;
				return(j);
				}
			}
		}
	}
c = **pp;
if(!isupper(c) && !bracket) {
	for(i=0,(*pp)=porg; (*pp) <= (*ppmax) && !MySpace(**pp) && i < MAXLIN; (*pp)++,i++)
					line[i] = **pp;
	line[i] = '\0'; im = i;
	sprintf(Message,
	"\nVariable must start with uppercase character or '|'. Can't make sense of \"%s\"",
		line);
	Print(wTrace,Message);
	if(OkBolChar(c)) {
		sprintf(Message,
			"\nMay be unknown terminal symbol, time-pattern or incorrect note convention?");
		Print(wTrace,Message);
		}
	return(ABORT);
	}
if(MaxVar != 0) {
	/* The contrary may happen if encoding with no compiled grammar and no compiled glossary */
	if(++Jvar >= MaxVar) {
		if((p_Var = (char****) IncreaseSpace((Handle)p_Var)) == NULL)
										return(ABORT);
		if((p_VarStatus = (int**) IncreaseSpace((Handle)p_VarStatus)) == NULL)
										return(ABORT);
		for(i=Jvar; i < (MaxVar=(MyGetHandleSize((Handle)p_Var) / sizeof(char**))); i++) {
			(*p_Var)[i] = NULL; (*p_VarStatus)[i] = 0;
			}
		}
	ptr = (char**) GiveSpace((Size)MAXLIN * sizeof(char));
	if(((*p_Var)[Jvar] = ptr) == NULL) return(ABORT);
	}
for(i=0; (*pp) <= (*ppmax)+1; (*pp)++,i++) {
	c = **pp;
	if(MaxVar > 0 && i >= MAXLIN) {
		(*((*p_Var)[Jvar]))[i-1] = '\0';
		sprintf(Message,"Max %ld chars in variable! Can't accept \"%s...\"",
			(long)MAXLIN-1L,(*((*p_Var)[Jvar])));
		Print(wTrace,Message);
		ptr = (*p_Var)[Jvar];
		MyDisposeHandle((Handle*)&ptr);
		(*p_Var)[Jvar] = NULL;
		Jvar--;
		return(ABORT);
		}
	if((c == '|' && bracket) || (!bracket && (!OkChar(c))) || ((*pp) > (*ppmax))) {
			if(MaxVar > 0 && i == 0) {
				Print(wTrace,
				"Variable name can't be empty. (May be you forgot a space before '-->')");
				ptr = (*p_Var)[Jvar];
				MyDisposeHandle((Handle*)&ptr);
				(*p_Var)[Jvar] = NULL;
				Jvar--;
				return(ABORT);
				}
			if(!bracket) {
				(*pp)--;
				while((c=(**pp)) == '_') {
					(*pp)--; i--;
					}
				(*pp)++;
				}
			if(MaxVar > 0) {
				(*((*p_Var)[Jvar]))[i] = '\0';
				ptr = (*p_Var)[Jvar];
				MySetHandleSize((Handle*)&ptr,(Size)(i+1)*sizeof(char));
				(*p_Var)[Jvar] = ptr;
				for(i=0; i < MAXMODE; i++) {
					if(Mystrcmp((*p_Var)[Jvar],Mode[i]) == 0) {
						sprintf(Message,"Misplaced '%s': it should be placed immediately after the weight",
							Mode[i]);
						Print(wTrace,Message);
						ptr = (*p_Var)[Jvar];
						MyDisposeHandle((Handle*)&ptr);
						(*p_Var)[Jvar] = NULL;
						Jvar--;
						return(ABORT);
						}
					}
				}
			if(bracket) (*pp)++;
			return(Jvar);
			}
	if(MaxVar > 0) (*((*p_Var)[Jvar]))[i] = c;
	}
if(Beta) Alert1("Err2. GetVar()");
return(ABORT);
}


FindCode(char x)
{
int j;
for(j=0; j < MAXCODE; j++)
	if(Code[j] == x) {
		switch(j) {
			case 12:
			case 13:
			case 14:
			case 17:
			case 18:
			case 19:
//			case 21:
				NotBPCase[4] = TRUE;
				break;
			}
		return(j);
		}
if(x == '.' || x == '¥') return(7);
if(x == '*') return(21);
if(x == '\\') return(25);
return(-1);
}


Recode(int notargument,long *p_imax,tokenbyte ***pp_buff)
{
int j,imaster,nbmaster;
long i,orgmaster[MAXLEVEL],endmaster[MAXLEVEL];


i = ZERO;
/* levpar = */ imaster = /* nhomo = */ 0;
while(i < (*p_imax)-1) {
	if((**pp_buff)[i] == T0) {
		i++;
		if((**pp_buff)[i] == 8) {		/* '(' */
			i++; /* levpar++;  */
			if((**pp_buff)[i] != T0) return(18);
			i++;
			if((**pp_buff)[i] == 6) {	/* '=' */
				(**pp_buff)[i-1] = 2;
				(**pp_buff)[i] = 0;
				imaster++;
				}
			else {
				if((**pp_buff)[i] != 4) return(18);
										/* only ':' should follow */
				(**pp_buff)[i-1] = 2;
				(**pp_buff)[i] = 1;
				}
			}
	/*	else
			if((**pp_buff)[i] == 9) {
				if(levpar > 0) levpar--;
				for(h=nhomo; h >= 1; h--)
					if(depth[h] >= levpar) nhomo--;
				} */
		i++;
		}
	else {
	/*	if((**pp_buff)[i] == T5) {
			j = (int)(**pp_buff)[i+1];
			nhomo++;
			if(nhomo >= MAXLEVEL) {
				if(Beta) Alert1("Increase 'MAXLEVEL'.  Err Recode()");
				return(20);
				}
			depth[nhomo] = levpar;
			homoname[nhomo] = j; 
			} */
		i+=2;
		}

	}
// OPTIMIZE: does the rest of this need to be done if imaster == 0 ?
nbmaster = 0;
// FIXME ? FindMaster() currently always returns 0; should it return -1 if LastSymbol() returns 0 ?
if(FindMaster(pp_buff,orgmaster,endmaster,&nbmaster,p_imax) == -1) return(16);
/* Print(wTrace,"\nList of masters:\n");
if(nbmaster == 0) Print(wTrace,"none\n");
else
	for(j=1; j<= nbmaster; j++) {
		sprintf(Message,"\n<%ld> ",(long)j);
		Print(wTrace,Message);
		for(i=orgmaster[j]; i <= endmaster[j]; i=i+2) {
			sprintf(Message," %ld.%ld",(long)(**pp_buff)[i],(long)(**pp_buff)[i+1]);
			Print(wTrace,Message);
			}
		}
*/

if((j=BindSlaves(pp_buff,orgmaster,endmaster,&nbmaster,p_imax))
	!= 0) return(j);
return(0);
}


FindMaster(tokenbyte ***pp_buff, long orgmaster[], long endmaster[], int *p_nbmaster, long *p_imax)
{
long i;
int j;

for(i=0; i < (*p_imax)-1; i+=2) {
	if((**pp_buff)[i] == T0 && (**pp_buff)[i+1] == 8) {		/* '(' */
		if((**pp_buff)[i+2] == T2 && (**pp_buff)[i+3] == 0) { /* '=' */
			i += 4;
			if((j=LastSymbol(pp_buff,i,p_imax)) == 0) continue;
			(*p_nbmaster)++;
			orgmaster[*p_nbmaster] = i;
			endmaster[*p_nbmaster] = j;
			}
		}
	}
return(0);
}


LastSymbol(tokenbyte ***pp_buff, long iorg, long *p_imax)
{
int levpar;
long i;

for(i=iorg,levpar=1; i < (*p_imax)-1; i+=2) {
	if((**pp_buff)[i] == T0 && (**pp_buff)[i+1] == 9) {	/* ')' */
		levpar--;
		if(levpar == 0) return(i-2);
		}
	else
		if((**pp_buff)[i] == T0 && (**pp_buff)[i+1] == 8) levpar++; 	/* '(' */
	}
return(0);
}


BindSlaves(tokenbyte ***pp_buff,long *orgmaster,long *endmaster,int *p_nbmaster,
	long *p_imax)
{
tokenbyte **p_a;
long i,ii,j,k;
int h,levpar,mark,islave;
long length[MAXLEVEL];
int nhomo,homoname[MAXLEVEL],depth[MAXLEVEL];

/* First pick-up ref numbers in master table */

if(*pp_buff == NULL) {
	if(Beta) Alert1("Err. BindSlaves(). *pp_buff = NULL");
	return(ABORT);
	}
if((p_a=(tokenbyte**) GiveSpace((Size) MyGetHandleSize((Handle)*pp_buff))) == NULL)
		return(ABORT);
	
if(CopyBuf(pp_buff,&p_a) == ABORT) return(ABORT);

for(i=levpar=nhomo=islave=0; i < (*p_imax)-1; i+=2) {
	if((**pp_buff)[i] == T0 && (**pp_buff)[i+1] == 8) {
		levpar++;
		}
	if((**pp_buff)[i] == T0 && (**pp_buff)[i+1] == 9) {	/* ')' */
		if(levpar > 0) levpar--;
		for(h=nhomo; h >= 1; h--)
			if(depth[h] >= levpar) nhomo--;
		}
	if((**pp_buff)[i] == T5) {
		nhomo++;
		if(nhomo >= MAXLEVEL) {
			if(Beta) Alert1("Increase 'MAXLEVEL'.  Err Recode()");
			MyDisposeHandle((Handle*)&p_a);
			return(20);	// should this be error 25 or 40 ? - akozar
			}
		depth[nhomo] = levpar;
		homoname[nhomo] = (int)(**pp_buff)[i+1];
		}
	if((**pp_buff)[i] == T2 && (**pp_buff)[i+1] == 1) {
								/* temp ':', this is a slave */
		if((j=Reference(pp_buff,orgmaster,endmaster,p_nbmaster,p_imax,i+2,
			nhomo,levpar,depth,homoname)) == -1) {
			MyDisposeHandle((Handle*)&p_a);
			return(17);
			}
		(*p_a)[i+1] = (tokenbyte) j;
		length[islave++] = endmaster[j] - orgmaster[j] + 2;
		if(islave >= MAXLEVEL) {
			if(Beta) Alert1("Increase 'MAXLEVEL'.  Err BindSlaves(). ");
			MyDisposeHandle((Handle*)&p_a);
			return(40);
			}
		continue;
		}
	}
if(CopyBuf(&p_a,pp_buff) == ABORT) return(17);  // should this return ABORT as above? - akozar
MyDisposeHandle((Handle*)&p_a);

/* Erase content of slavesÉ */
/* É especially copies of 'ZERO' markers that are irrelevant */

// OPTIMIZE: I think we can skip the rest of this if islave == 0
i = 0; islave = 0;
while(i < (*p_imax)-1)	{
	if((**pp_buff)[i] == T2 && (**pp_buff)[i+1] != 0) {	/* found slave */
		for(j=i+2; j < i+2+length[islave]; j=j+2) {
			(**pp_buff)[j] = TEND; (**pp_buff)[j+1] = TEND;
			}
		i = j;
		islave++;
		}
	i += 2;
	}
	
/* Count ZERO markers and write true slave markers */
for(i=0; i < (*p_imax)-1; i+=2) {
	if((**pp_buff)[i] != T2 || (j=(**pp_buff)[i+1]) == 0) continue;
													/* look for slave */
	for(ii=orgmaster[j],mark=1; ii < (i-2); ii=ii+2)
		if((**pp_buff)[ii] == T2 && (**pp_buff)[ii+1] == 0) /* ZERO marker */
			mark++;
	(**pp_buff)[i+1] = (tokenbyte) mark;
	}

/* Now delete all '0 0's in slaves */
i = 0; islave = 0;
while(i < (*p_imax)-1)	{
	if((**pp_buff)[i] == T2 && (**pp_buff)[i+1] != 0) {	/* found slave */
		j = i + 2; k = j + length[islave];
		// OPTIMIZE: save & use p_a above and copy back to pp_buff here instead, skipping
		// slave contents, so we only need to move the rest of the buffer once ??
		// See longer explanation in BP2-info4-Grammar-functions.txt - akozar
		MoveDown(pp_buff,&j,&k,p_imax);
		islave++; i=j;
		}
	i += 2;
	}
return(0);
}


Reference(tokenbyte ***pp_buff, long orgmaster[], long endmaster[], int *p_nbmaster,
	long *p_imax, long iorg, int nhomo, int levpar, int depth[], int homoname[])
{
long i,j;
int k,n,dif,h,p1,q1,p2,q2;

for(k=1; k <= (*p_nbmaster); k++) {
	dif = 0;
	for(i=iorg,j=orgmaster[k]; j <= endmaster[k]; i+=2,j+=2) {
		p1 = (int)(**pp_buff)[i]; q1 = (int)(**pp_buff)[i+1];
		p2 = (int)(**pp_buff)[j]; q2 = (int)(**pp_buff)[j+1];
		if(p1 == T25 || p2 == T25) {	/* Fixed 12/7/98 */
			if(p1 == T25) q1 += 16384;
			if(p2 == T25) q2 += 16384;
			p1 = p2 = T3;
			if(nhomo > 0) goto TERMINAL;
			}
		if(p2 == T3 && q2 > 1 && nhomo > 0) {
			/* Apply homorphisms to terminal */
TERMINAL:
			for(n=nhomo; n >= 1; n--) {
				if(depth[n] < levpar) {
					h = homoname[n];
					if(h >= Jhomo) {
						if(Beta) Alert1("Err. Reference(). h >= Jhomo");
						return(-1);
						}
					q2 = Image(h,q2);
					}
				}
			}
		if(p1 != p2 || q1 != q2) {
			dif = TRUE; 
			break;
			}
		}
	if(!dif && (j == (endmaster[k] + 2)) && (**pp_buff)[j] == T0
			&& (**pp_buff)[j+1] == 9) 		/* Next symbol is ')' */
		return(k);
	}
return(-1);
}


tokenbyte Image(int h,int j)
{
tokenbyte jj;

if(j >= 16384) {
	jj = j - 16384;
	if(jj > 127) {
		if(Beta) Alert1("Err. Image(). jj > 127");
		return(j);
		}
	jj = (*((*p_NoteImage)[h]))[jj];
	}
else {
	if(j < 0 || j >= Jbol) {
		if(Beta) Alert1("Err. Image(). j < 0 || j >= Jbol");
		return(j);
		}
	jj = (*((*p_Image)[h]))[j];
	}
return(jj);
}


FoundPeriod(tokenbyte ***pp_a)
{
long i;
tokenbyte m,p;

if(pp_a == NULL || *pp_a == NULL || **pp_a == NULL) return(FALSE);
for(i=0; ; i+=2) {
	m = (int)(**pp_a)[i]; p = (int)(**pp_a)[i+1];
	if(m == TEND && p == TEND) break;
	if(m == T0 && p == 7) {		/* '¥' */
		return(TRUE);
		}
	}
return(FALSE);
}


ShowError(int i,int igram,int irul)
{
char t[255];
static int fatal[] = {0,1,1,0,0,0,1,0,0,0,0,0,0,1,
	0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0};
static char* err[] = {"",
"too many grammars", 		/* 1 */
"too many rules",			/* 2 */
"rule syntax",				/* 3 */
"flag name too long",		/* 4 */
"rule incomplete",			/* 5 */
"file not found",			/* 6 */
"subgrammar type",		/* 7 */
"incorrect expression or bad derivation",			/* 8 */
"error in remote context",	/* 9 */
"unterminated comment: can't find ']' nor '*/'",		/* 10 */
"incorrect terminal",		/* 11 */
"variable name too long",	/* 12 */
"too many variables",		/* 13 */
"argument too large: increase 'FIELDSIZE'!",	/* 14 */
"argument syntax",			/* 15 */
"bad slave marker",			/* 16 */
"slave has no reference (may be incorrect homomorphism in alphabet)",	/* 17 */
"'=' or ':' marker must follow opening bracket",	/* 18 */
"too many terminal symbols",						/* 19 */
"incorrect weight",							/* 20 */
"can't read this file",						/* 21 */
"terminal name too long",					/* 22 */
"variable name can't be empty (could be missing space before '-->')",
																/* 23 */
"incorrect homomorphism",					/* 24 */
"too many homomorphisms",					/* 25 */
"symbol too long",							/* 26 */
"terminal symbol contains unwanted character", /* 27 */
"'n' is too high in '?n' metavariable",		/* 28 */
"more '?' in right argument",				/* 29 */
"can't write this file",					/* 30 */
"can't interpret polyphonic structure",		/* 31 */
"can't create MIDI data",					/* 32 */
"error in arithmetic",						/* 33 */
"too long sequence",						/* 34 */
"subgrammar type should be one single word:'ORD','LEFT','LIN','SUB'", /* 35 */
"too many sound-objects",							/* 36 */
"can't set time",									/* 37 */
"item ignored...",									/* 38 */
"'{' and '}' not balanced",							/* 39 */
"too many parenthesis levels: increase 'MAXLEVEL'",	/* 40 */
"weight misplaced or incorrect <<out-time object>>",		/* 41 */
"incorrect tag",									/* 42 */
"number should be less than 256",					/* 43 */
"procedure should be single word, e.g. '_destru'",	/* 44 */
"only one flag allowed in left argument",			/* 45 */
"incorrect time pattern name(s)",						/* 46 */
"incorrect time value in pattern(s)",					/* 47 */
"same pattern(s) defined twice",						/* 48 */
"/flags/ not allowed in SUB or SUB1 or POSLONG subgrammar or glossary",	/* 49 */
"/flag = .../ assignment or comparison should occur only in argument of rule", /* 50 */
"/flag/ incrementation or comparison should occur only in left argument of rule",	/* 51 */
"Missing slash after /flag/",	/* 52 */
"Incorrect integer value for /flag/ assignment or incrementation",	/* 53 */
"Reserved words 'lambda', 'nil', 'empty' and 'null' can't be redefined as terminals", /* 54 */
"Incorrect character in argument" /* 55 */
	};

if(i < 0) return(TRUE);
sprintf(Message,"*** Error code %ld: %s",(long)i,err[i]);
if(igram != 0) {
	sprintf(t," in gram#%ld rule %ld\n",(long)igram,(long)irul);
	}
else {
	strcpy(t,"\n");
	}
strcat(Message,t);
BPActivateWindow(SLOW,wTrace);
Print(wTrace,Message);
ShowSelect(CENTRE,wTrace);
return(fatal[i]);
}