/* SetObjectFeatures.c (BP2 version CVS) */ 

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


SetObjectParams(int isobject,int level,int nseq,short** p_articul,int k,int j,
	CurrentParameters *p_currentparameters,ContParameters **p_contparameters,
	Table **h_table)
{
ParameterSpecs **currentinstancevalues;
long size;
int i,ii,n,ip,ins,index,chan;
short mode;
double v0,v1;
Coordinates **ptr;

if(k < 2) {
	if(Beta) Alert1("Err. SetObjectParams(). k < 2");
	return(OK);
	}
if(nseq >= Maxconc) {
	if(Beta) Println(wTrace,"\nErr. SetObjectParams(). nseq >= Maxconc");
	return(OK);
	}

if(Beta && (*p_Instance)[k].contparameters.values != NULL)
	Println(wTrace,"Err. SetObjectParams(). (*p_Instance)[k].contparameters.values != NULL");

(*p_Instance)[k].contparameters.values = NULL;
(*p_Instance)[k].contparameters.number = 0;

if(level >= 0) {
	n = (*p_contparameters)[level].number;
	if(n <= IPANORAMIC) n = (IPANORAMIC + 1);
	if((currentinstancevalues=(ParameterSpecs**) GiveSpace((Size)(n) * sizeof(ParameterSpecs))) == NULL)
		return(ABORT);
	(*p_Instance)[k].contparameters.values = currentinstancevalues;
	(*p_Instance)[k].contparameters.number = n;
	
	if((*p_contparameters)[level].values == NULL) {
		if(Beta) Alert1("Err. SetObjectParams(). (*p_contparameters)[level].values == NULL");
		return(ABORT);
		}
	for(i=0; i < (*p_contparameters)[level].number; i++) {
		chan = (*((*p_contparameters)[level].values))[i].channel;
		if(i <= IPANORAMIC && !(*((*p_contparameters)[level].values))[i].known) {
			switch(i) {
				case IMODULATION:
					if(!ResetControllers && (*p_Oldvalue)[chan].modulation > -1)
						v0 = (*p_Oldvalue)[chan].modulation;
					else
						v0 = DEFTMODULATION;
					break;
				case IPITCHBEND:
					if(!ResetControllers && (*p_Oldvalue)[chan].pitchbend > -1)
						v0 = (*p_Oldvalue)[chan].pitchbend;
					else
						v0 = DEFTPITCHBEND;
					break;
				case IPRESSURE:
					if(!ResetControllers && (*p_Oldvalue)[chan].pressure > -1)
						v0 = (*p_Oldvalue)[chan].pressure;
					else
						v0 = DEFTPRESSURE;
					break;
				case IVOLUME:
					if(!ResetControllers && (*p_Oldvalue)[chan].volume > -1)
						v0 = (*p_Oldvalue)[chan].volume;
					else
						v0 = DeftVolume;
					break;
				case IPANORAMIC:
					if(!ResetControllers && (*p_Oldvalue)[chan].panoramic > -1)
						v0 = (*p_Oldvalue)[chan].panoramic;
					else
						v0 = DeftPanoramic;
					break;
				}
			(*((*p_contparameters)[level].values))[i].known = TRUE;
			v1 = v0;
			mode = FIXED;
			}
		else {
			v0 = (*((*p_contparameters)[level].values))[i].v0;
			v1 = (*((*p_contparameters)[level].values))[i].v1;
			mode = (*((*p_contparameters)[level].values))[i].mode;
			}
		(*currentinstancevalues)[i].v0 = v0;
		(*currentinstancevalues)[i].v1 = v1;
		(*currentinstancevalues)[i].mode = mode;
		(*currentinstancevalues)[i].control = (*((*p_contparameters)[level].values))[i].control;
		(*currentinstancevalues)[i].channel = chan;
		(*currentinstancevalues)[i].index = (*((*p_contparameters)[level].values))[i].index;
		(*currentinstancevalues)[i].imax = ZERO;
		(*currentinstancevalues)[i].point = NULL;
		if(i == IPANORAMIC && j < Jbol && !(*p_OkPan)[j]) continue;
		if(i == IVOLUME && j < Jbol && !(*p_OkVolume)[j]) continue;
		if(h_table == NULL) {
			if(Beta) Alert1("Err. SetObjectParams(). h_table == NULL");
			}
		
		if(isobject && (*h_table)[i].point != NULL) {
			if((*h_table)[i].offset == 0) {
				if((ptr=(Coordinates**)
					GiveSpace((Size)((*h_table)[i].imax) * sizeof(Coordinates))) == NULL)
					return(ABORT);
				(*currentinstancevalues)[i].point = ptr;
				(*currentinstancevalues)[i].imax = (*h_table)[i].imax;
				for(ii=0; ii < (*h_table)[i].imax; ii++) {
					(*((*currentinstancevalues)[i].point))[ii] = (*((*h_table)[i].point))[ii];
					}
				/* Here we set v1, at last */
				(*currentinstancevalues)[i].v1 = (*((*h_table)[i].point))[ii-1].value;
				}
			((*h_table)[i].offset)--;
			}
			
		if((*((*p_contparameters)[level].values))[i].v0
				== (*((*p_contparameters)[level].values))[i].v1
				&& (*h_table)[i].point == NULL)
			(*currentinstancevalues)[i].mode = FIXED;
			
		if((*((*p_contparameters)[level].values))[i].channel > 0)
			(*currentinstancevalues)[i].channel
				= (*((*p_contparameters)[level].values))[i].channel;
		else
			(*currentinstancevalues)[i].channel = p_currentparameters->currchan;
		}
	}

(*p_Instance)[k].nseq = nseq;
(*p_Instance)[k].velocity = p_currentparameters->currvel;
(*p_Instance)[k].rndvel = p_currentparameters->rndvel;
(*p_Instance)[k].velcontrol = p_currentparameters->velcontrol;
(*p_Instance)[k].randomtime = p_currentparameters->randomtime;
(*p_Instance)[k].seed = p_currentparameters->seed;

if(j < 1 || j >= Jbol || (*p_OkMap)[j]) {
	(*p_Instance)[k].map0 = p_currentparameters->map0;
	(*p_Instance)[k].map1 = p_currentparameters->map1;
	(*p_Instance)[k].mapmode = p_currentparameters->mapmode;
	(*p_Instance)[k].xpandkey = p_currentparameters->xpandkey;
	(*p_Instance)[k].xpandval = p_currentparameters->xpandval;
	}
else {
	(*p_Instance)[k].map0.p1 = (*p_Instance)[k].map1.p1 = -1;
	(*p_Instance)[k].xpandkey = -1;
	(*p_Instance)[k].xpandval = 0;
	}

(*p_Instance)[k].transposition = 0;
(*p_Instance)[k].lastistranspose = p_currentparameters->lastistranspose;
if(j < 1 || j >= Jbol) {
	(*p_Instance)[k].transposition = p_currentparameters->currtranspose;
	if(p_articul != NULL) {
		if(p_currentparameters->currarticul >= 0)
			(*p_articul)[k] = p_currentparameters->currarticul;
		else (*p_articul)[k] = 256 + p_currentparameters->currarticul;
		}
	}
else {
	if((*p_OkTransp)[j]) (*p_Instance)[k].transposition = p_currentparameters->currtranspose;
	if(p_articul != NULL) {
		if((*p_OkArticul)[j]) {
			if(p_currentparameters->currarticul >= 0)
				(*p_articul)[k] = p_currentparameters->currarticul;
			else (*p_articul)[k] = 256 + p_currentparameters->currarticul;
			}
		}
	}
if(j > 0 && j < Jbol) {
	if((*p_DefaultChannel)[j] > 0) {	/* Object has specific channel */
		/* Here we need (*p_DefaultChannel)[1] = 0. */
		(*p_Instance)[k].channel = (*p_DefaultChannel)[j];
		if((*p_Instance)[k].channel > MAXCHAN || (*p_Instance)[k].channel < 1) {
			sprintf(Message,"'%s' has channel %ld.  Should be 1..%ld\n",
				(*((*p_Bol)[j])),(long)(*p_DefaultChannel)[j],(long)MAXCHAN);
			Print(wTrace,Message);
			ShowError(32,0,0);
			return(FAILED);
			}
		}
	else {
		if((*p_DefaultChannel)[j] == 0)
			(*p_Instance)[k].channel = p_currentparameters->currchan;
		else (*p_Instance)[k].channel = 0;
		}
	if((*p_CsoundInstr)[j] > 0) {	/* Object has specific instrument */
		/* Here we need (*p_CsoundInstr)[1] = 0. */
		(*p_Instance)[k].instrument = (*p_CsoundInstr)[j];
		}
	else {
		if((*p_CsoundInstr)[j] == 0)
			(*p_Instance)[k].instrument = p_currentparameters->currinstr;
		else (*p_Instance)[k].instrument = 0;
		}
	}
else {
	(*p_Instance)[k].channel = p_currentparameters->currchan;
	(*p_Instance)[k].instrument = p_currentparameters->currinstr;
	}
	
return(OK);
}


AttachObjectLists(int k,int nseq,p_list ****p_waitlist,p_list ****p_scriptlist,
	int* p_newswitch,unsigned long* currswitchstate)
{
objectspecs** pto;
long** pts;
int i;


if(nseq >= Maxconc) {
	if(Beta) Println(wTrace,"\nErr. AttachObjectLists(). nseq >= Maxconc");
	return(OK);
	}
if((*p_waitlist)[nseq] == NULL && (*p_scriptlist)[nseq] == NULL
											&& !(*p_newswitch)) return(OK);
if(k < 2) {
	if(Beta) Alert1("Err. AttachObjectLists()");
	return(ABORT);
	}
if((*p_ObjectSpecs)[k] == NULL) {
	if((pto = (objectspecs**) GiveSpace((Size)sizeof(objectspecs))) == NULL) return(ABORT);
	(*p_ObjectSpecs)[k] = pto;
	SwitchState(k) = NULL;
	}
WaitList(k) = (*p_waitlist)[nseq];
ObjScriptLine(k) = (*p_scriptlist)[nseq];
if(*p_newswitch) {
	if((pts = (long**) GiveSpace((Size) MAXCHAN * sizeof(long))) == NULL) return(ABORT);
	SwitchState(k) = pts;
	for(i=0; i < MAXCHAN; i++) (*(SwitchState(k)))[i] = currswitchstate[i];
	}
(*p_waitlist)[nseq] = NULL;
(*p_scriptlist)[nseq] = NULL;
*p_newswitch = FALSE;
return(OK);
}


SetVariation(tokenbyte targettoken,CurrentParameters **p_deftcurrentparameters,
	CurrentParameters *p_currentparameters,
	ContParameters **p_contparameters,int levelorg,int index,
	unsigned long id,tokenbyte **p_buff,double orgspeed,double orgscale,
	float *p_endvalue,
	KeyNumberMap *p_mapendvalue,float *p_maxbeats,Table **h_table)
// This procedure does two things:
// 1) Given a target token representing a stepwise/continuous parameter assignment,
//    or the 'index' of a continuous parameter, search the next target token and
//    return the number of beats between the two target tokens and the end value.
//    If the parameter is continuous, calculate the increment from start to end value.
// 2) Build a table 'h_table' containing at least three values if these relate to
//    the same sound-object or simple note.  The table will be attached to the
//    object in SetObjectParams(). An offset value is returned so that attachment
//    will occur on the proper object.

// This procedure is similar to GetSymbolicDuration() concerning object concatenation Ô&Õ

// I wonder how many times I debugged this procedure!
{
int level,seq,**p_seq,**p_chan,**p_instr,ibeats,foundtimeobject,foundsecondvalue,
	paramvalueindex,oldpitchbendrange[MAXCHAN+1],foundconcatenation,oldm,oldp,
	foundendconcatenation,levelmem,seqmem,foundtarget,imap,
	paramnameindex,maketable,tableisbeingbuilt,tablemade,iobjmem,
	okincrease,objectsfound,nonemptyobject,chan,instr,targettokenfoundafterobject,
	forgetmakingtable,followedwithgap,result,chanorg,instrorg;
KeyNumberMap vmap;
unsigned long i,tablesize;
char **p_notinthisfield,notinthisfield,foundclosingbracket;
Coordinates **ptr;
double v,vv,oldv,x,orgtranspose,startvalue,tempo,speed,scale,s;
tokenbyte m,p;

/* The following ones must be double because they may be incremented with values < 1 when sequence is too fast */
double prodtempo,maxbeats,itargettoken,itable;

Table t;
Handle h;

result = OK;
if((*p_buff)[id] == TEND && (*p_buff)[id+1] == TEND) return(OK);

if((p_seq=(int**) GiveSpace((Size) Maxlevel * sizeof(int))) == NULL) return(ABORT);
if((p_chan=(int**) GiveSpace((Size) Maxlevel * sizeof(int))) == NULL) return(ABORT);
if((p_instr=(int**) GiveSpace((Size) Maxlevel * sizeof(int))) == NULL) return(ABORT);
if((p_notinthisfield=(char**) GiveSpace((Size) Maxlevel * sizeof(char))) == NULL)
	return(ABORT);

for(i=1; i <= MAXCHAN; i++) oldpitchbendrange[i] = PitchbendRange[i];

level = levelmem = 0;
seq = seqmem = (*p_seq)[level] = 0;

speed = orgspeed;
scale = orgscale;
if(scale != 0.) {
	tempo = speed / scale;
	prodtempo = (Prod / tempo);
	}
else tempo = prodtempo = 0.;

ibeats = itable = maxbeats = 0.;
objectsfound = targettokenfoundafterobject = 0;
foundtimeobject = foundsecondvalue = maketable = tableisbeingbuilt = tablemade
	= forgetmakingtable
	= followedwithgap = foundconcatenation = foundendconcatenation = foundtarget
	= notinthisfield = foundclosingbracket = FALSE;
okincrease = TRUE;

if(targettoken == -1) targettoken = IndexToToken(index);
/* This will be required for FindValue() */

if(index > -1) {
	maketable = TRUE;
	startvalue = (*p_endvalue) = (*((*p_contparameters)[levelorg].values))[index].v0;
	if((*((*p_contparameters)[levelorg].values))[index].mode == FIXED)
		goto RECORDVALUES;
	}
else {
	switch(targettoken)	{
		case T11:	/* _vel() */
			startvalue = (*p_endvalue) = p_currentparameters->currvel;
			if(p_currentparameters->velmode == FIXED) goto END;
			break;
		case T20:	/* _legato() or _staccato() */
			startvalue = (*p_endvalue) = p_currentparameters->currarticul;
			break;
		case T26:	/* _transpose */
			startvalue = (*p_endvalue) = p_currentparameters->currtranspose;
			break;
		case T37:	/* _keymap() */
			*p_mapendvalue = p_currentparameters->map0;
			if(p_currentparameters->mapmode == FIXED
				|| p_currentparameters->mapmode == OFF) goto END;
			break;
		}
	}
	
oldv = startvalue;
if(levelorg >= 0) orgtranspose = (*p_deftcurrentparameters)[levelorg].currtranspose;
t.point = NULL;
t.imax = t.offset = tablesize = ZERO;
itargettoken = 0.;
iobjmem = oldm = oldp = -1;

chan = chanorg = p_currentparameters->currchan;
instr = instrorg = p_currentparameters->currinstr;

for(i=id+2L; ; i+=2L) {
	m = (*p_buff)[i]; p = (*p_buff)[i+1];
	if(m == TEND && p == TEND) break;
	if(m == T0) {
		switch(p) {
			case 11:	/* '/' speed up */
				speed = GetScalingValue(p_buff,i);
				if(scale != 0.) {
					tempo = speed / scale;
					prodtempo = (Prod / tempo);
					}
				else tempo = prodtempo = 0.;
				i += 4;
				break;
			case 25:	/* '\' speed down */
				speed = 1. / (GetScalingValue(p_buff,i));
				if(scale != 0.) {
					tempo = speed / scale;
					prodtempo = (Prod / tempo);
					}
				else tempo = prodtempo = 0.;
				i += 4;
				break;
			case 21:	/* '*' scale up */
				scale = GetScalingValue(p_buff,i);
				if(scale != 0.) {
					tempo = speed / scale;
					prodtempo = (Prod / tempo);
					}
				else tempo = prodtempo = 0.;
				i += 4;
				continue;
				break;
			case 24:	/* '**' scale down */
				s = GetScalingValue(p_buff,i);
				scale = 1. / s;
				if(scale != 0.) {
					tempo = speed / scale;
					prodtempo = (Prod / tempo);
					}
				else tempo = prodtempo = 0.;
				i += 4;
				continue;
				break;
			case 12:	/* '{' */
			case 22:
				if(seq == seqmem && objectsfound > 0 && !foundconcatenation) {
					okincrease = FALSE;
					}
				if(level >= 0) {
					(*p_seq)[level] = seq;
					(*p_notinthisfield)[level] = notinthisfield;
					if(seq == seqmem) {
						(*p_chan)[level] = chan;
						(*p_instr)[level] = instr;
						}
					}
				level++;
				if(level >= Maxlevel) {
					if(Beta) Println(wTrace,"\nErr. SetVariation(). level >= Maxlevel");
					goto ENDLOOP;
					}
				seq = 0;
				break;
			case 13:	/* '}' */
			case 23:
				if(seq == seqmem && objectsfound > 0 && !foundconcatenation)
					okincrease = FALSE;
				if(level == levelmem && foundconcatenation) foundclosingbracket = TRUE;
				level--;
				if(level >= 0) {
					seq = (*p_seq)[level];
					notinthisfield = (*p_notinthisfield)[level];
					if(seq == seqmem) {
						chan = (*p_chan)[level];
						instr = (*p_instr)[level];
						}
					}
				else {
					seq = 0;
					chan = chanorg;
					instr = instrorg;
					goto ENDLOOP;
					}
				continue;
				break;
			case 14:	/* ',' */
				if(seq == seqmem && objectsfound > 0 && !foundconcatenation) {
					okincrease = FALSE;
					}
				if(foundconcatenation && !foundclosingbracket) notinthisfield = TRUE;
				seq++;
				break;
			case 18:	/* '&' following terminal */
				if(level == levelmem && seq == seqmem) {
					foundconcatenation = TRUE;
					notinthisfield = foundclosingbracket = FALSE;
					}
				break;
			case 19:	/* '&' preceding terminal */
				if(!foundconcatenation) continue;
				if(!notinthisfield) foundendconcatenation = TRUE;
				break;
			}
		continue;
		}
	
	if(m == T3 || m == T25 || m == T9) {
		/* Sound-object, silence, prolongation, simple note, time pattern */
		if(foundendconcatenation && foundconcatenation
				&& (m == T25 || (m == T3 && p > 0))) {
			foundendconcatenation = FALSE;
			if(m == oldm && p == oldp) {
				foundconcatenation = FALSE;
				m = T3; p = 0;
				levelmem = level;
				seqmem = seq;
				}
			}
			
		if(seq != seqmem) continue;
		
		if((m == T25 || m == T9 || (m == T3 && p > 0)) && level <= 0) {
			nonemptyobject = TRUE;
			objectsfound++;
			if(!foundconcatenation && objectsfound == 1) {
				oldm = m; oldp = p;
				}
			}
		else {
			/* When level > 0 all objects are accounted as empty */
			nonemptyobject = FALSE;
			if(targettokenfoundafterobject > 0) followedwithgap = TRUE;
			}
		if(!foundsecondvalue) maxbeats += prodtempo;
		if(nonemptyobject && (iobjmem == -1 || !tablemade)) iobjmem = itargettoken;
		itargettoken += prodtempo;
		if(maketable && !tableisbeingbuilt && !tablemade && !forgetmakingtable
				&& nonemptyobject) {
			tableisbeingbuilt = TRUE;
			itable = 0.;
			}
		if(!foundtimeobject) {	/* Found first sound-object or prolongation or silence */
			foundtimeobject = TRUE;
			maxbeats = prodtempo;
			goto ENOUGH;
			}
		if(nonemptyobject && !tablemade && !forgetmakingtable && (objectsfound > 1)) {
			/* Found following sound-object or silence */
			okincrease = FALSE;
			if(tableisbeingbuilt && t.point != NULL) {
				if(targettokenfoundafterobject > 0 && followedwithgap) {
					(*(t.point))[t.imax].i = itable;
					/* The last value is not yet known */
					tableisbeingbuilt = FALSE;
					tablemade = TRUE;
					(t.imax)++;
					}
				else {
					/* No table can be built */
					forgetmakingtable = TRUE;
					tableisbeingbuilt = FALSE;
					}
				}
			else {
				if(maketable) {
					tableisbeingbuilt = TRUE;
					itable = 0.;
					(t.offset)++;
					}
				}
			}
ENOUGH:
		itable += prodtempo;
		if(nonemptyobject) {
			targettokenfoundafterobject = 0;
			followedwithgap = FALSE;
			}
		continue;
		}
		
	if((level > levelmem || seq != seqmem) && !(foundconcatenation && !notinthisfield))
		continue;
		
	if((level == levelmem || (foundconcatenation && !notinthisfield))
			&& chan == chanorg
			&& (m == targettoken || (index >= 0 && TokenToIndex(m,index) == index))) {
		if(m == T35) { 	/* _value() */
			paramnameindex = p % 256;
			if(index != FindParameterIndex(p_contparameters,levelorg,paramnameindex))
				continue;
			}
			
		v = FindValue(m,p,chan);
		if(v == Infpos) {
	//		if(Beta) Alert1("Err. SetVariation(). v == Infpos");
			result = ABORT;
			break;
			}
		if(targettoken == T26) {	/* _transpose */
			v += orgtranspose;	/* $$ added 27/1/99 and fixed 28/1/99 */
			}
		if(targettoken == T37) {	/* _value */
			vmap.p1 = p % 128;
			vmap.q1 = (p - (p % 128)) / 128;
			i += 2L;
			m = (*p_buff)[i]; p = (*p_buff)[i+1];
			if(m != T37) {
				if(Beta) Alert1("Err. SetVariation(). m != T37");
				break;
				}
			vmap.p2 = p % 128;
			vmap.q2 = (p - (p % 128)) / 128;
			}
		foundtarget = TRUE;
		if(objectsfound > 0) targettokenfoundafterobject++;
		if(!foundsecondvalue) {
			if(maxbeats > 0.) {
				foundsecondvalue = TRUE;
				if(targettoken != T37) *p_endvalue = v;
				else *p_mapendvalue = vmap;
				if(!maketable) break;
				}
			else startvalue = v;
			}
		
		if(targettoken == T11 || targettoken == T20 || targettoken == T37) goto END;
		/* _vel(), articulation and _keymap()  are not concerned with tables */
		
	/*	if(index < 0) {
			if(Beta) Alert1("Err. SetVariation(). index < 0");
			goto END;
			} */
		
		if(!maketable || !tableisbeingbuilt) goto MORE;
		if(t.point == NULL) {
			tablesize = 10L;
			t.imax = 1L;
			if((ptr = (Coordinates**) GiveSpace((Size)(tablesize * sizeof(Coordinates)))) == NULL)
				return(ABORT);
			t.point = ptr;
			(*(t.point))[ZERO].i = ZERO;
			/* The value of this first point is not yet known */
			}
		(*(t.point))[t.imax].i = itable;
		(*(t.point))[t.imax].value = v;
		(t.imax)++;
		if(t.imax >= (tablesize - 1L)) {
			ptr = t.point;
			if((ptr = (Coordinates**) IncreaseSpace((Handle)ptr)) == NULL)
				return(ABORT);
			t.point = ptr;
			tablesize = MyGetHandleSize((Handle)ptr) / sizeof(Coordinates);
			}
MORE:
		if(t.point != NULL && iobjmem >= 0) {
			if(itargettoken > 0.)
				vv = oldv + ((v - oldv) * iobjmem) / itargettoken;
			else vv = oldv;
			if(tablemade) {
				(*(t.point))[t.imax-1L].value = vv;
				break;
				}
			else (*(t.point))[ZERO].value = vv;
			}
		oldv = v;
		iobjmem = -1;
		itargettoken = 0.;
		continue;
		}
	
	if(m == T21) {		/* _pitchrange() */
		if(p > 0) PitchbendRange[chan] = p;
		else
			if(Beta) Alert1("Err. SetVariation(). p <= 0");
		continue;
		}
	
	if(m == T10) {		/* _chan() */
		x = FindValue(m,p,chan);
		if(chan == (int) x) continue;
		
		/* A channel change in the sequence should be the end of the variation */
		/* É of a continuous MIDI parameter */
		if(objectsfound > 0 && index >= 0 && index <= IPANORAMIC) okincrease = FALSE;
		
		chan = (int) x;
		if(maxbeats == 0.) chanorg = chan;
		continue;
		}
	if(m == T32) {		/* _ins() */
		x = FindValue(m,p,chan);
		if(instr == (int) x) continue;
		instr = (int) x;
		if(maxbeats == 0.) instrorg = instr;
		continue;
		}
	if(m == T7) {	/* Out-time object */
		if(objectsfound > 0) okincrease = FALSE;
		continue;
		}
	}
	
ENDLOOP:
if(maketable && tableisbeingbuilt && !tablemade && t.point != NULL
		&& t.imax > ZERO && targettokenfoundafterobject > 0 && followedwithgap
		&& result == OK) {
	if((*(t.point))[t.imax-1L].i != itable) {
		(*(t.point))[t.imax].i = itable;
		(*(t.point))[t.imax].value = oldv;
		(t.imax)++;
		}
	iobjmem = 0;
	tablemade = TRUE;
	}
if(!foundsecondvalue || result != OK) maxbeats = 0.;

(*p_maxbeats) = maxbeats;

if(index < 0 || result != OK) goto END;

h = (Handle)(*h_table)[index].point;
MyDisposeHandle(&h);

(*h_table)[index].point = NULL;
(*h_table)[index].imax = 0;

if(!foundtarget) goto STORECHAN;

if(maketable && t.point != NULL) {
	if(tablemade && result == OK) {
		if(iobjmem == -1) (*(t.point))[t.imax-1L].value = oldv;
		}
	else {
		ptr = t.point;
		MyDisposeHandle((Handle*)&ptr);
		t.point = NULL;
		t.imax = ZERO;
		}
	(*h_table)[index] = t;
	}
else {
	(*h_table)[index].point = NULL;
	(*h_table)[index].imax = ZERO;
	}

RECORDVALUES:
if((*p_contparameters)[levelorg].values == NULL) {
	if(Beta) Alert1("Err. SetVariation(). (*p_contparameters)[levelorg].values == NULL");
	goto END;
	}
(*((*p_contparameters)[levelorg].values))[index].maxbeats = (long) maxbeats;
(*((*p_contparameters)[levelorg].values))[index].ibeats = 0;
(*((*p_contparameters)[levelorg].values))[index].start
	= (*((*p_contparameters)[levelorg].values))[index].v0 = startvalue;
(*((*p_contparameters)[levelorg].values))[index].known = TRUE;
/* (*((*p_contparameters)[levelorg].values))[index].instrument = instrorg; */
if((maxbeats == 0.) || (*((*p_contparameters)[levelorg].values))[index].mode == FIXED) {
	(*((*p_contparameters)[levelorg].values))[index].maxbeats = 1;
	(*((*p_contparameters)[levelorg].values))[index].increment = 0;
	(*((*p_contparameters)[levelorg].values))[index].v1 = startvalue;
	}
else {
	(*((*p_contparameters)[levelorg].values))[index].increment
		= (*p_endvalue) - startvalue;
	(*((*p_contparameters)[levelorg].values))[index].v1 = (*p_endvalue);
	/* Normally this is useless as the v1 value will be calculated again later */
	}
				
STORECHAN:
if(index >= 0 && (*p_contparameters)[levelorg].values != NULL) {
	(*((*p_contparameters)[levelorg].values))[index].channel = chanorg;
	}

END:
for(i=1; i <= MAXCHAN; i++) PitchbendRange[i] = oldpitchbendrange[i];
MyDisposeHandle((Handle*)&p_seq);
MyDisposeHandle((Handle*)&p_chan);
MyDisposeHandle((Handle*)&p_instr);
MyDisposeHandle((Handle*)&p_notinthisfield);
return(result);
}


Fix(int nseq,Milliseconds **p_time1,Milliseconds **p_time2,int nature_time)
{
int j,k;
long i,inext;
double local_period;
Milliseconds t1,t2;
unsigned int seed;


if(nseq >= Maxconc) {
	if(Beta) Println(wTrace,"\nErr. Fix(). nseq >= Maxconc");
	return(OK);
	}
i = 1; local_period = 0.;
while(TRUE) {
	k = (*((*p_Seq)[nseq]))[i];
	if(k < 0) break;
	inext = i;
	while((*((*p_Seq)[nseq]))[++inext] == 0);
	if(k > 1) {					/* Ignoring silences "-" except if specs attached */
		if(k >= Maxevent) {
			if(Beta) Alert1("Err. Fix(). k >= Maxevent");
			return(ABORT);
			}
		j = (*p_Instance)[k].object;
		if(j > 0) {
			if(j < 16384) {
				if((*p_PivMode)[j] == RELATIVE)
					t1 = (*p_T)[i]
						- (Milliseconds) ((*p_Instance)[k].dilationratio * (*p_Dur)[j]
						* (*p_PivPos)[j] / 100.);
				else
					t1 = (*p_T)[i] - (*p_PivPos)[j];
				RandomTime(&t1,(*p_Instance)[k].randomtime);
				(*p_time1)[i] = t1;
				if(PlayFromInsertionPoint) t1 = (*p_time1)[i] = (*p_T)[i];
				t2 = (*p_time2)[i] = t1
					+ (Milliseconds)((*p_Instance)[k].alpha * (*p_Dur)[j]);
				}
			else {	/* Simple note */
				t1 = (*p_T)[i];
				RandomTime(&t1,(*p_Instance)[k].randomtime);
				(*p_time1)[i] = t1;
				t2 = (*p_time2)[i] = t1 + (*p_Instance)[k].alpha * 1000L;
				}
			}
		else {
			/* Out-time sound-object */
			j = -j;
			t1 = t2 = (*p_time1)[i] = (*p_time2)[i] = (*p_T)[i];
			}
		}
	else {
		t1 = t2 = (*p_time1)[i] = (*p_time2)[i] = (*p_T)[i];
		if(nseq == 0 && nature_time == SMOOTH) {
			t2 = (*p_time2)[i] = (*p_time1)[i] + (inext - i) * local_period;
			}
		}
	if(nseq == 0 && nature_time == SMOOTH) {
		local_period = (double) (t2 - t1) / (inext - i);
		i = inext;
		}
	else i = inext;
	}
return(OK);
}


Calculate_alpha(int nseq,int nmax,long maxseq,unsigned long imaxseq,int nature_time,
	char** p_marked)
{
int j,k,kprev,unfinished,gotcurrenttime,ncycles;
long i,inext,inextm,imax,iprev,jprev;
double d,alpha,beta,r,sigmaridi,clockperiod;
Milliseconds To,ton,toff,currenttime,dur;


if(nseq >= Maxconc) {
	if(Beta) Println(wTrace,"\nErr. Calculate_alpha(). nseq >= Maxconc");
	return(OK);
	}
(*p_Instance)[0].alpha = (*p_Instance)[1].alpha = 0.;	/* '_' and '-' */
if(nature_time == SMOOTH) clockperiod = ((double) Pclock) * 1000. / Qclock;
i = 0;
while((k=(*((*p_Seq)[nseq]))[++i]) < 1) {
	if(k == -1) return(OK);
	}
if(nature_time == STRIATED || nseq == 0) {
	while(TRUE) {
		k = (*((*p_Seq)[nseq]))[i]; if(k == -1) break;
		inext = i; while((*((*p_Seq)[nseq]))[++inext] == 0);
		if(k < 2) {	/* Also reject silences */
			i = inext;
			continue;
			}
		if(k >= Maxevent) {
			if(Beta) Alert1("Err. Calculate_alpha(). k >= Maxevent");
			return(ABORT);
			}
		j = (*p_Instance)[k].object;
		ncycles = 1;
		if(j <= 0) {	/* Out-time object $$$ added '=' on 18/8/98 */
			beta = alpha = 0.; goto OKALPHA1;
			}
		d = (double) (inext - i) * Kpress / Ratio; /* Symbolic duration */
		if(nature_time == SMOOTH) {
			if(Qclock < 1L) {
				if(Beta) Alert1("Err. Calculate_alpha(). Qclock < 1. ");
				return(ABORT);
				}
			if(Pclock > 0.) { 				/* Measured smooth time */
				if(j > 16383 || (*p_Tref)[j] > EPSILON) {
					/* Striated object or simple note */
					if(j < 16384) alpha = (double) d * clockperiod / (*p_Tref)[j];
					else	/* Simple note */
						alpha = (double) d * clockperiod / 1000L;
					}
				else {
					if((*p_Dur)[j] > EPSILON) {
						alpha = (double) d * clockperiod / (*p_Dur)[j];
						}
					else alpha = d;
					}
				}
			else {		/* Pclock = ZERO; non-measured smooth time */
				alpha = d;
				}
			}
		else {					/* Striated time or nseq > 0 */
			if(d > 0.) {
				if(j > 16383 || (*p_Tref)[j] > EPSILON) { /* Striated object or simple note */
					if(j < 16384)
						alpha = ((double)(*p_T)[inext] - (*p_T)[i]) / (*p_Tref)[j];
					else 	/* Simple note */
						alpha = ((double)(*p_T)[inext] - (*p_T)[i]) / 1000L;
					}
				else {			/* Smooth object */
					if((*p_Dur)[j] > EPSILON)
						alpha = ((double)(*p_T)[inext] - (*p_T)[i]) / (*p_Dur)[j];
					else alpha = 0.;
					}
				}
			else {
				alpha = 0.;
				}
			}
		
		beta = alpha;
		if(j > 16383) goto OKALPHA1;
		if((*p_FixScale)[j] && (*p_PeriodMode)[j] == IRRELEVANT) {
			alpha = beta = 1.; goto OKALPHA1;
			}
		if(FixDilationRatioInCyclicObject(j,d,&alpha,&beta,&ncycles) == OK) goto OKALPHA1;
		if((*p_OkExpand)[j] && alpha >= 1.) goto OKALPHA1;
		if((*p_OkCompress)[j] && alpha <= 1.) goto OKALPHA1;
		if(!(*p_FixScale)[j] && !(*p_OkExpand)[j] && !(*p_OkCompress)[j]) {
			if(alpha > (*p_AlphaMax)[j]) beta = alpha = (*p_AlphaMax)[j];
			if(alpha < (*p_AlphaMin)[j]) beta = alpha = (*p_AlphaMin)[j];
			goto OKALPHA1;
			}
		beta = alpha = 1.;
		
OKALPHA1:
		if(alpha < 0.) alpha = 0.;
		if(beta < 0.) beta = 0.;
		(*p_Instance)[k].alpha = alpha;
		if(ForceRatio >= 0.) (*p_Instance)[k].alpha = beta = ForceRatio;
		(*p_Instance)[k].dilationratio = beta;
		(*p_Instance)[k].ncycles = ncycles;
		i = inext;
		}
	return(OK);
	}
	
// Now, case nseq > 0 in SMOOTH time

imax = imaxseq;
if(nseq < nmax && (imax < maxseq)) {
	imax = maxseq;
	/* We will complete streak markings beyond the end ofÉ */
	/* É the sequence, up to end of the phase diagram. */
	/* This is only needed if there are more sequences. */
	}
iprev = i; jprev = -1;
while(TRUE) {
	if(iprev >= imax) break;
	inextm = iprev;
FINDNEXTMARKED:
	while(!(*p_marked)[++inextm]) {
		if(inextm >= imax) return(OK);
		}
	To = (*p_T)[inextm] - (*p_T)[iprev];
	if(To <= ZERO) {	/* May be ZERO because of roundings */
		goto FINDNEXTMARKED;
		}
	sigmaridi = 0.;
	for(i=iprev; i < inextm;) {			/* Calculate sigmaridi */
		if(i < imaxseq) {
			k = (*((*p_Seq)[nseq]))[i];
			/* Here we will also consider silences "-" and empty objects "_" */
			inext = i; while((*((*p_Seq)[nseq]))[++inext] == 0);
			}
		else {
			k = 1;	/* Beyond end of sequence we create silences. */
			inext = i + 1;
			}
		if(inext > inextm) inext = inextm;
		/* In this case alpha will be too small on current object */
		
		if(k >= Maxevent) {
			if(Beta) Alert1("Err. Calculate_alpha(). k >= Maxevent (2)");
			return(ABORT);
			}
		j = (*p_Instance)[k].object;
		if(k == 0) j = jprev;
		if(j <= 0) {		/* Out-time object $$$ added '=' on 18/8/98 */
			i = inext; continue;
			}
		d = (double) (inext - i) * Kpress / Ratio;	/* Symbolic duration */
		if(j < 16384) {
			dur = (*p_Dur)[j];
			if((*p_Tref)[j] > EPSILON && dur > EPSILON) {
				/* Striated object */
				r = ((double) dur) / (*p_Tref)[j];
				}
			else r = 1.;
			}
		else {	/* Simple note */
			dur = 1000L; r = 1.;
			}
		sigmaridi += r * d;
		i = inext;
		}
	unfinished = FALSE;
	if(sigmaridi <= 0.) {	/* May happen if only out-time objects */
		iprev = inextm;
		continue;
		}
	currenttime = (*p_T)[iprev];
	for(i=iprev; i < inextm;) {			/* Now calculate alpha's */
		if(i < imaxseq) {
			k = (*((*p_Seq)[nseq]))[i];
			inext = i; while((*((*p_Seq)[nseq]))[++inext] == 0);
			}
		else {
			k = 1;	/* Beyond end of sequence we create silences. */
			inext = i + 1;
			}
		if(inext > inextm) {
			if(inext <= imaxseq) unfinished = TRUE;
			inext = inextm;
			}
		j = (*p_Instance)[k].object;
		if(k == 0) j = jprev;
		if(j <= 0) {			/* Out-time object $$$ Added '=' on 18/8/98 */
			(*p_Instance)[k].alpha = 0.;
			i = inext;
			continue;
			}
		d = (double) (inext - i) * Kpress / Ratio;
		if(j < 16384) {
			dur = (*p_Dur)[j];
			if((*p_Tref)[j] > EPSILON && dur > EPSILON) {
				/* Striated object */
				r = ((double) dur) / (*p_Tref)[j];
				}
			else r = 1.;
			}
		else {	/* Simple note */
			dur = 1000L; r = 1.;
			}
		alpha = To * r * d / dur / sigmaridi;
		if(!unfinished && inext == inextm) {	/* Last object in section */
			toff = (*p_T)[inext];	/* This will compensate cumulated roundings */
			alpha = ((double)(toff - currenttime)) / dur;
			}
		gotcurrenttime = FALSE;
		if(k == 0 && !unfinished) {
			currenttime = (*p_T)[iprev] + (Milliseconds)(alpha * dur);
			/* Must calculate it now because alpha is going to change */
			
			gotcurrenttime = TRUE;
			toff = currenttime;
			alpha = ((double)(toff - ton)) / dur;
			k = kprev;
			if(k == 1 && !(*p_marked)[inext]) {
				/* alpha is meaningless for silences */
				(*p_T)[inext] = toff;
				(*p_marked)[inext] = TRUE;
				}
			}
		if(unfinished && k != 0) {
			ton = currenttime;
			/* Here alpha was incorrect but will be recalculated */
			jprev = j; kprev = k;
			}
		if(k == 1 && !(*p_marked)[inext]) {
			/* This is a "short" silence beginning in the same section */
			(*p_T)[inext] = currenttime + alpha * dur;
			(*p_marked)[inext] = TRUE;
			}
		if(alpha < 0.) {	/* Should no longer happen since To > ZERO */
			sprintf(Message,
			"Err. Calculate_alpha(). alpha=%f",alpha);
			if(Beta) Alert1(Message);
			alpha = 0.;
			}
		if(!gotcurrenttime) currenttime += (Milliseconds) (alpha * dur);
		
		beta = alpha; ncycles = 1;
		if(j > 16383) goto OKALPHA2;
		if((*p_FixScale)[j] && (*p_PeriodMode)[j] == IRRELEVANT) {
			alpha = beta = 1.; goto OKALPHA2;
			}
		if(FixDilationRatioInCyclicObject(j,d,&alpha,&beta,&ncycles) == OK) goto OKALPHA2;
		if((*p_OkExpand)[j] && alpha >= 1.) goto OKALPHA2;
		if((*p_OkCompress)[j] && alpha <= 1.) goto OKALPHA2;
		if(!(*p_FixScale)[j] && !(*p_OkExpand)[j] && !(*p_OkCompress)[j]) {
			if(alpha > (*p_AlphaMax)[j]) beta = alpha = (*p_AlphaMax)[j];
			if(alpha < (*p_AlphaMin)[j]) beta = alpha = (*p_AlphaMin)[j];
			goto OKALPHA2;
			}
		beta = alpha = 1.;
		
OKALPHA2:
		if(alpha < 0.) alpha = 0.;
		if(beta < 0.) beta = 0.;
		(*p_Instance)[k].alpha = alpha;
		if(ForceRatio >= 0.) (*p_Instance)[k].alpha = beta = ForceRatio;
		(*p_Instance)[k].dilationratio = beta;
		(*p_Instance)[k].ncycles = ncycles;
		i = inext;
		}
	iprev = inextm;
	}
return(OK);
}


FixDilationRatioInCyclicObject(int j,double d,double *p_alpha,double *p_dilationratio,
	int *p_ncycles)
/* Examine objects with periodical part and see whether that part could be repeated */
/* If so, fix the number of repetitions and the actual dilation ratio,É */
/* É and change alpha if necessary */
{
int n1,n2,limit;
double objectperiod,beforeperiod,ncycles,pivpos;

*p_dilationratio = *p_alpha;
if(j > 16383) return(OK);

// We're only concerned about cyclic objects with dilation ratio > 1.
if((*p_alpha) <= 1. || (*p_PeriodMode)[j] == IRRELEVANT) return(FAILED);
if(GetPeriod(j,1.,&objectperiod,&beforeperiod) == FAILED) return(FAILED);

if(!(*p_FixScale)[j] && !(*p_OkExpand)[j] && !(*p_OkCompress)[j])
	limit = TRUE;		/* dilation ratio has a specified upper limit */
else limit = FALSE;		/* it hasn't */

if((*p_PivMode)[j] == RELATIVE)
	pivpos = (*p_Dur)[j] * (*p_PivPos)[j] / 100.;
else
	pivpos = (*p_PivPos)[j];
if(PlayFromInsertionPoint) pivpos = 0.;
	
// First consider objects that can't be stretched

if(!limit && (!(*p_OkExpand)[j] || (*p_PeriodMode)[j] == ABSOLUTE)) {
	*p_dilationratio = 1.;
FINDCYCLES:
	ncycles = (double)(((*p_alpha) * (*p_Dur)[j] / (*p_dilationratio))
		- beforeperiod + pivpos) / objectperiod;
	if((*p_ForceIntegerPeriod)[j]) {
		if((ncycles - (int)ncycles) > 0.5) ncycles = ceil(ncycles);
		else ncycles = floor(ncycles);
		}
	goto OUT;
	}

// Now, objects that can be stretched
	
ncycles = d;

// Adjust dilation ratio so that the duration is correct

*p_dilationratio = (*p_alpha) * (*p_Dur)[j] / (beforeperiod - pivpos + (ncycles * objectperiod));

// If beta is too large, fix limit value and increase ncycles

if((!limit && !(*p_OkExpand)[j] && *p_dilationratio > 1.)
		|| (limit && *p_dilationratio > (*p_AlphaMax)[j])) {
	if(limit) *p_dilationratio = (*p_AlphaMax)[j];
	else *p_dilationratio = 1.;
	goto FINDCYCLES;
	}

OUT:
if((*p_Dur)[j] > EPSILON) {
	*p_alpha = *p_dilationratio * (beforeperiod + (ncycles * objectperiod)) / (*p_Dur)[j];
	*p_ncycles = (int) ncycles;
	}
else {
	*p_alpha = 0.;
	*p_ncycles = 0;
	}
return(OK);
}


SetLimits(int nseq,Milliseconds** p_maxcoverbeg,Milliseconds** p_maxcoverend,
	Milliseconds** p_maxgapbeg,Milliseconds** p_maxgapend,Milliseconds** p_maxtruncbeg,
	Milliseconds** p_maxtruncend)
{
int i,j,k;		
Milliseconds maxcover1,maxcover2,maxgap1,maxgap2,maxtrunc1,maxtrunc2,dur;

if(nseq >= Maxconc) {
	if(Beta) Println(wTrace,"\nErr. SetLimits(). nseq >= Maxconc");
	return(OK);
	}		
for(i=1;; i++) {
	k = (*((*p_Seq)[nseq]))[i];
	if(k == -1) break;
	if(k < 1) continue;
	if(k >= Maxevent) {
		if(Beta) Alert1("Err. SetLimits(). k >= Maxevent");
		return(ABORT);
		}
	j = (*p_Instance)[k].object;
	if(j <= 0) {		/* Out-time object $$$ Added '=' on 18/8/98 */
		j = -j; dur = ZERO;
		}
	else {
		if(j < 16384) dur = (*p_Instance)[k].dilationratio * (*p_Dur)[j];
		else dur = (*p_Instance)[k].dilationratio * 1000L;	/* Simple note */
		}

	maxcover1 = maxcover2 = Infpos;
	if(j > 1 && j < 16384 && !(*p_CoverBeg)[j]) {
		if((*p_CoverBegMode)[j] == ABSOLUTE) maxcover1 = (*p_MaxCoverBeg)[j];
		else maxcover1 = (dur * (*p_MaxCoverBeg)[j]) / 100.;
		}
	
	if(j > 1 && j < 16384 && !(*p_CoverEnd)[j]) {
		if((*p_CoverEndMode)[j] == ABSOLUTE) maxcover2 = (*p_MaxCoverEnd)[j];
		else maxcover2 = (dur * (*p_MaxCoverEnd)[j]) / 100.;
		} 
	
	if((maxcover1 + maxcover2) >= dur && maxcover1 != Infpos && maxcover2 != Infpos)
		(*p_maxcoverbeg)[i] = (*p_maxcoverend)[i] = Infpos;
	else {
		(*p_maxcoverbeg)[i] = maxcover1;
		(*p_maxcoverend)[i] = maxcover2;
		}
	
	maxgap1 = maxgap2 = Infpos;
	if(j > 1 && j < 16384 && (*p_ContBeg)[j]) {
		if((*p_ContBegMode)[j] == ABSOLUTE) maxgap1 = (*p_MaxBegGap)[j];
		else maxgap1 = (dur * (*p_MaxBegGap)[j]) / 100.;
		}
	(*p_maxgapbeg)[i] = maxgap1;
	
	if(j > 1 && j < 16384 && (*p_ContEnd)[j]) {
		if((*p_ContEndMode)[j] == ABSOLUTE) maxgap2 = (*p_MaxEndGap)[j];
		else maxgap2 = (dur * (*p_MaxEndGap)[j]) / 100.;
		}
	(*p_maxgapend)[i] = maxgap2;
	
	maxtrunc1 = maxtrunc2 = dur;
	if(j > 1 && j < 16384 && !(*p_TruncBeg)[j]) {
		if((*p_TruncBegMode)[j] == ABSOLUTE) maxtrunc1 = (*p_MaxTruncBeg)[j];
		else maxtrunc1 = (dur * (*p_MaxTruncBeg)[j]) / 100.;
		}
	if(maxtrunc1 < dur) (*p_maxtruncbeg)[i] = maxtrunc1;
	else (*p_maxtruncbeg)[i] = dur;
	
	if(j > 1 && j < 16384 && !(*p_TruncEnd)[j]) {
		if((*p_TruncEndMode)[j] == ABSOLUTE) maxtrunc2 = (*p_MaxTruncEnd)[j];
		else maxtrunc2 = (dur * (*p_MaxTruncEnd)[j]) / 100.;
		}
	if(maxtrunc2 < dur) (*p_maxtruncend)[i] = maxtrunc2;
	else (*p_maxtruncend)[i] = dur;
	}
return(OK);
}


AssignValue(int index,double value,int p,int level,int* p_kmx,
	CurrentParameters **p_deftcurrentparameters,
	CurrentParameters *p_currentparameters,ContParameters **p_contparameters,
	long id,tokenbyte ***pp_buff,double tempo,double scale,
	Table **h_table)
{
float endval,maxbeats;
KeyNumberMap mapendvalue;

if((*p_contparameters)[level].values == NULL) {
	if(Beta) Alert1("Err. AssignValue(). (*p_contparameters)[level].values == NULL");
	return(ABORT);
	}
(*((*p_contparameters)[level].values))[index].v0
	= (*((*p_contparameters)[level].values))[index].start
	= value;
(*((*p_contparameters)[level].values))[index].active = TRUE;
(*((*p_contparameters)[level].values))[index].known = TRUE;

if(p < 128)
	(*((*p_contparameters)[level].values))[index].control = -1;
else
	(*((*p_contparameters)[level].values))[index].control = p - 128;
	
if(SetVariation(-1,p_deftcurrentparameters,p_currentparameters,p_contparameters,level,index,id,*pp_buff,
	tempo,scale,&endval,&mapendvalue,&maxbeats,h_table) != OK) return(ABORT);

return(OK);
}


TokenToIndex(tokenbyte m,int index)
{
switch(m) {
	case T14:	/* _mod() */
		return(IMODULATION); break;
	case T15:	/* _pitchbend() */
		return(IPITCHBEND); break;
	case T16:	/* _press() */
		return(IPRESSURE); break;
	case T19:	/* _volume() */
		return(IVOLUME); break;
	case T29:	/* _pan() */
		return(IPANORAMIC); break;
	case T35:	/* _value() */
		return(index); break;
	}
return(-1);
}


IndexToToken(int index)
{
if(index < 0) return(-1);
switch(index) {
	case IMODULATION:	/* _mod() */
		return(T14); break;
	case IPITCHBEND:	/* _pitchbend() */
		return(T15); break;
	case IPRESSURE:	/* _press() */
		return(T16); break;
	case IVOLUME:	/* _volume() */
		return(T19); break;
	case IPANORAMIC:	/* _pan() */
		return(T29); break;
	default:	/* _value() */
		return(T35); break;
	}
}


double FindValue(tokenbyte m,tokenbyte p,int chan)
{
double x,xx;
int value,paramvalueindex,paramnameindex;

if(m == T37) return(0.);

paramnameindex = -1;
switch(m) {
	case T10:
	case T11:
	case T16:
	case T19:
	case T20:
	case T26:
	case T29:
	case T32:
	case T38:
	case T39:
		if(p >= 128 && m != T26) {
			value = ParamValue[p-128];
			if(value == INT_MAX) {
				if(Beta) Alert1("Err. FindValue(). value == INT_MAX");
				value = 127;
				}
			return((double) value);
			}
		else return((double) p);
		break;
	}
if(m == T35) {	/* _value() */
	paramnameindex = (p % 256);
	paramvalueindex = (p - paramnameindex) / 256;
	x = (*p_NumberConstant)[paramvalueindex];
	}
else x = (double) p;
if(m == T15 || paramnameindex == IPITCHBEND) {
	xx = x;
	if(PitchbendRange[chan] > 0)
		x = DEFTPITCHBEND + ((double) x * DEFTPITCHBEND / (double) PitchbendRange[chan]);
	if(x < 0 || x > 16383) {
		if(PitchbendRange[chan] > 0)
			sprintf(Message,"Pitchbend value (%ld cents) on channel %ld out of range (-%ld..%ld cents)",
				(long)xx,(long)chan,(long)PitchbendRange[chan],(long)PitchbendRange[chan]);
		else
			sprintf(Message,"Pitchbend value (%ld) on channel %ld out of range (0..16383)",
				(long)xx,(long)chan);
		Alert1(Message);
		return(Infpos);
		}
	}
return(x);
}


double GetSymbolicDuration(int ignoreconcat,int precise,tokenbyte **p_buff,
	tokenbyte oldm,tokenbyte oldp,long id,double orgspeed,double orgscale,
	int levelorg)
{
// This procedure is similar to SetVariation() concerning object concatenation Ô&Õ

// $$$ suppress 'precise'

unsigned long i;
tokenbyte m,p;
int foundconcatenation,foundendconcatenation,foundclosingbracket,seq,level,**p_seq;
char **p_notinthisfield,notinthisfield;
double tempo,tempomax,prodtempo,objectduration,orgobjectduration,speed,
	s,scale;

if(oldm != T3 && oldm != T25 && oldm != T9) {
	if(Beta) {
		sprintf(Message,"Err. GetSymbolicDuration(). oldm = %ld",(long)oldm);
		Println(wTrace,Message);
		}
	return(0);
	}
	
if((p_seq=(int**) GiveSpace((Size) Maxlevel * sizeof(int))) == NULL) return(ABORT);
if((p_notinthisfield=(char**) GiveSpace((Size) Maxlevel * sizeof(char))) == NULL)
	return(ABORT);

level = 0;
seq = (*p_seq)[level] = 0;

if(orgspeed == ZERO) {
	if(Beta) Println(wTrace,"Err. GetSymbolicDuration(). orgspeed == ZERO");
	objectduration = 0.;
	goto OUT;
	}
tempomax = Prod / Kpress;
scale = orgscale;
speed = orgspeed;

if(scale != 0.) tempo = speed / scale;
else tempo = 0.;

if((!precise && (tempo > tempomax)) || tempo == 0.) prodtempo = 0.;
else prodtempo = (Prod / tempo);

objectduration = prodtempo;
orgobjectduration = 0.;

foundconcatenation = foundendconcatenation = notinthisfield = foundclosingbracket = FALSE;

for(i=id+2L; ; i+=2) {
	m = (*p_buff)[i]; p = (*p_buff)[i+1];
	if(m == TEND && p == TEND) break;
	if(m == T0) {
		switch(p) {
			case 11:	/* '/' speed up */
				speed = GetScalingValue(p_buff,i);
				if(scale != 0.) {
					tempo = speed / scale;
					prodtempo = (Prod / tempo);
					}
				else tempo = 0.;
				if((!precise && (tempo > tempomax)) || tempo == 0.) prodtempo = 0.;
				else prodtempo = Prod / tempo;
				i += 4;
				continue;
				break;
			case 25:	/* '\' speed down */
				speed = 1. / (GetScalingValue(p_buff,i));
				if(scale != 0.) {
					tempo = speed / scale;
					prodtempo = (Prod / tempo);
					}
				else tempo = 0.;
				if((!precise && (tempo > tempomax)) || tempo == 0.) prodtempo = 0.;
				else prodtempo = (Prod / tempo);
				i += 4;
				continue;
				break;
			case 21:	/* '*' scale up */
				scale = GetScalingValue(p_buff,i);
				if(scale != 0.) {
					tempo = speed / scale;
					prodtempo = (Prod / tempo);
					}
				else tempo = 0.;
				if((!precise && (tempo > tempomax)) || tempo == 0.) prodtempo = 0.;
				else prodtempo = (Prod / tempo);
				i += 4;
				continue;
				break;
			case 24:	/* '**' scale down */
				s = GetScalingValue(p_buff,i);
				scale = 1. / s;
				if(scale != 0.) {
					tempo = speed / scale;
					prodtempo = (Prod / tempo);
					}
				else tempo = 0.;
				if((!precise && (tempo > tempomax)) || tempo == 0.) prodtempo = 0.;
				else prodtempo = (Prod / tempo);
				i += 4;
				continue;
				break;
			case 12:	/* '{' */
			case 22:
				if(!foundconcatenation || ignoreconcat) goto OUT;
				if(level >= 0) {
					(*p_seq)[level] = seq;
					(*p_notinthisfield)[level] = notinthisfield;
					}
				level++;
				if(level >= Maxlevel) {
					if(Beta) Println(wTrace,"\nErr. GetSymbolicDuration(). level >= Maxlevel");
					return(0);
					}
				seq = 0;
				if(!notinthisfield) {
					orgobjectduration = objectduration;
					}
				break;
			case 13:	/* '}' */
			case 23:
				if(!foundconcatenation || ignoreconcat) goto OUT;
				if(level == 0) foundclosingbracket = TRUE;
				level--;
				if(level >= 0) {
					seq = (*p_seq)[level];
					notinthisfield = (*p_notinthisfield)[level];
					}
				else {
					seq = 0;
					notinthisfield = FALSE;
					}
				break;
			case 14:	/* ',' */
				if(!foundconcatenation || ignoreconcat) goto OUT;
				if(!foundclosingbracket) notinthisfield = TRUE;
				if(!notinthisfield) {
					objectduration = orgobjectduration;
					}
				seq++;
				break;
			case 7:		/* '.' */
				if(!foundconcatenation || ignoreconcat) goto OUT;
				break;
			case 18:	/* '&' following terminal */
				if(ignoreconcat) goto OUT;
				if(level == 0 && seq == 0) {
					foundconcatenation = TRUE;
					notinthisfield = foundclosingbracket = FALSE;
					}
				break;
			case 19:	/* '&' preceding terminal */
				if(!foundconcatenation || ignoreconcat) goto OUT;
				if(!notinthisfield)
					foundendconcatenation = TRUE;
				break;
			}
		continue;
		}
	if(m == T7 || m == T4) continue;	/* out-time object or variable */
	if(m == T9) {	/* time pattern */
		if(!notinthisfield) objectduration += prodtempo;
		continue;
		}
	if(m == T3 && p == 0) {
		if(!notinthisfield) objectduration += prodtempo;
		continue;
		}
	if(m == T3 || m == T25) {
		if(!foundconcatenation || ignoreconcat) goto OUT;
		if(foundendconcatenation) {
			foundendconcatenation = FALSE;
			if(m == oldm && p == oldp) {
				foundconcatenation = FALSE;
				level = 0;
				seq = (*p_seq)[level] = 0;
				}
			}
		if(!notinthisfield) objectduration += prodtempo;
		continue;
		}
	}

OUT:
if(foundconcatenation && !ignoreconcat) {
	if(ShowMessages) Println(wTrace,"An unbound concatenation '&' was ignored");
	objectduration = 0.;
	}
MyDisposeHandle((Handle*)&p_seq);
MyDisposeHandle((Handle*)&p_notinthisfield);
return(objectduration);
}


RandomTime(Milliseconds *p_t1,short randomtime)
{
float x;

if(randomtime == 0) return(OK);
x = (2. * rand() * ((float)randomtime)) / ((float)RAND_MAX);
UsedRandom = TRUE;
if(x > randomtime) x = randomtime - x;
(*p_t1) += x;
if((*p_t1) < ZERO) (*p_t1) = ZERO;
return(OK);
}
