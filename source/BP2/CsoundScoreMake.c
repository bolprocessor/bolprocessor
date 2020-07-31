/* CsoundScoreMake.c (BP2 version CVS) */ 

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


CscoreWrite(int strikeagain,int onoffline,double dilationratio,Milliseconds t,int iline,
	int key,int velocity,int chan,int instrument,int j,int nseq,int kcurrentinstance,
	PerfParameters ****pp_currentparams)
{
// j is the sound-object prototype
// onoffline = LINE  -- copy Csound score line from object-prototype
// onoffline = ON -- start a Csound process as a NoteOn has been encountered
// onoffline = OFF -- terminate Csound process as a NoteOff has been encountered, and
//    write Csound score line

int i,c,ins,index,paramnameindex,iarg,ip,ipitch,iargmax,octave,changedpitch,overflow,comeback,
	pitchclass,result,maxparam,itable;
char line[MAXLIN];
long imax;
double time,x,xx,cents,deltakey,dur,**scorearg,alpha1,alpha2,startvalue,endvalue,ratio;
PerfParameters **perf;
ParameterStatus **params,**paramscopy;
Handle h;
CsoundParam **instrparamlist;

scorearg = NULL;
params = NULL;

result = ABORT;

if(chan < 0 || chan >= MAXCHAN) {
	if(Beta) Alert1("Err. CscoreWrite(). chan < 0 || chan >= MAXCHAN");
	chan = 0;
	}
	
perf = (*pp_currentparams)[nseq];

if(onoffline == LINE) {
	if(j >= Jbol) {
		if(Beta) Alert1("Err. CscoreWrite(). j >= Jbol");
		goto OUT;
		} 
	if(iline < 0 || (iline >= (*p_CsoundSize)[j])) {
		if(Beta) Alert1("Err. CscoreWrite(). iline < 0 || iline >= (*p_CsoundSize)[j]");
		goto OUT;
		}
	dur = (*((*pp_CsoundScore)[j]))[iline].duration * dilationratio;
	key = 0;
	}
else {
	if(key < 0 || key >= MAXKEY) {
		if(Beta) Alert1("Err. CscoreWrite(). Incorrect key");
		goto OUT;
		}
	}

if(Jinstr == 1) {
	ins = 0;
	if((*p_CsInstrumentIndex)[ins] == -1) index = 1;
	else index = (*p_CsInstrumentIndex)[ins];
	}
else {
	if(onoffline == LINE) {
		index = instrument;
		if((*p_CsoundInstr)[j] > 0) index = (*p_CsoundInstr)[j];
		if((*p_CsoundInstr)[j] < 0) index = (*((*pp_CsoundScore)[j]))[iline].instrument;
		}
	else {
		if(instrument > 0) index = instrument;
		else if(j < Jbol && (*p_CsoundAssignedInstr)[j] >= 0) index = (*p_CsoundAssignedInstr)[j];
			else if(WhichCsoundInstrument[chan+1] >= 0)
								index = WhichCsoundInstrument[chan+1];
				else index = 0;
		}
	for(ins=0; ins < Jinstr; ins++) {
		if((*p_CsInstrumentIndex)[ins] == index) break;
		}
	if(index == 0 || ins >= Jinstr) {
		ins = 0; index = 1;
		}
	}
instrparamlist = (*p_CsInstrument)[ins].paramlist;

if(Pclock > 0.)	/* Striated or measured smooth time */
	time = ((double) t) * Qclock / ((double) Pclock) / 1000.;
else
	time = ((double) t) / 1000.;
	
comeback = FALSE;

if(onoffline == ON) {
	if((*perf)->level[key] > 0) {
	/* Here we're in trouble because of two consecutive NoteOn's */
	/* The solution will be to use the 'sequence' information */
		if(strikeagain) {
			comeback = TRUE;
			onoffline = OFF;
			goto SETOFF;
			}
		else {
			(*perf)->level[key]++;
			result = OK;
			goto OUT;
			}
		}
		
SETON:
	(*perf)->level[key]++;
	(*perf)->starttime[key] = time;
	(*perf)->velocity[key] = velocity;
	(*perf)->dilationratio[key] = dilationratio;
	
	maxparam = (*((*pp_currentparams)[nseq]))->numberparams;
	if((paramscopy = (ParameterStatus**)
		GiveSpace((Size)(maxparam * sizeof(ParameterStatus)))) == NULL) goto OUT;
	for(i=0; i < maxparam; i++) (*paramscopy)[i] = (*((*perf)->params))[i];
	(*perf)->startparams[key] = paramscopy;
	result = OK;
	goto OUT;
	}

if(onoffline == OFF && (*perf)->level[key] < 1) {
	if(Beta) Alert1("Err. CscoreWrite(). (*perf)->level[key] < 1");
	goto OUT;
	}

if(onoffline != LINE && (--((*perf)->level[key])) > 0) {
	result = OK;
	goto OUT;
	}

SETOFF:

// Prepare Csound score line

if(onoffline != LINE)
	params = (*perf)->startparams[key];
else
	params = (*perf)->params;

if(params == NULL) {
	if(Beta) Alert1("Err. CscoreWrite(). params == NULL");
	goto OUT;
	}

iargmax = (*p_CsInstrument)[ins].iargmax;
if(iargmax < 4) {
	if(Beta) Alert1("Err. CscoreWrite(). iargmax < 4");
	iargmax = 4;
	}

if((scorearg=(double**) GiveSpace((Size)((iargmax + 1) * sizeof(double)))) == NULL)
	goto OUT;
	
for(iarg=0; iarg <= iargmax; iarg++) (*scorearg)[iarg] = 0.;

if(Pclock > 0.)  /* Striated or measured smooth time */
	ratio = Qclock / ((double) Pclock) / 1000.;
else
	ratio = 0.001;
		
if(onoffline != LINE) {
	(*scorearg)[2] = (*perf)->starttime[key];
	(*scorearg)[3] = time - (*perf)->starttime[key];
	}
else {
	(*scorearg)[2] = time;
	(*scorearg)[3] = dur * ratio;
	for(iarg=4; iarg < (4+(*((*pp_CsoundScore)[j]))[iline].nbparameters); iarg++) {
		(*scorearg)[iarg] = (*((*((*pp_CsoundScore)[j]))[iline].h_param))[iarg-4];
		}
	}

iarg = ipitch = (*p_CsPitchIndex)[ins];
pitchclass = -1;

if(iarg > 0) {
	if(onoffline == LINE) {
		x = (*scorearg)[iarg];
		switch((*p_CsPitchFormat)[ins]) {
			case OPPC:
				octave = MyInt(x);
				pitchclass = MyInt(100. * (x - octave));
				deltakey = 12. * (octave - 3.) + pitchclass;
				break;
			case OPD:
				deltakey = (x - 3.) * 12.;
				break;
			case CPS:
				deltakey = (C4key + 9.) + 12. * (log(x/A4freq) / log(2.));
				break;
			}
		key = MyInt(deltakey);
		deltakey -= key;
		if(j < Jbol) {
			if((*p_OkTransp)[j] && (*perf)->lastistranspose)
				TransposeKey(&key,(*perf)->transpose);
			if((*p_OkMap)[j])
				key = ExpandKey(key,(*perf)->xpandkey,(*perf)->xpandval);
			if((*p_OkTransp)[j] && (!(*perf)->lastistranspose))
				TransposeKey(&key,(*perf)->transpose);
			}
		}
	else deltakey = 0.;
	pitchclass = key % 12;
	octave = (key - pitchclass) / 12 + 3;
	if((*p_CsPitchBendStartIndex)[ins] == -1
			&& (*params)[IPITCHBEND].active && (*p_CsInstrument)[ins].pitchbendrange > 0.) {
		startvalue = (*params)[IPITCHBEND].startvalue;
		endvalue = (*params)[IPITCHBEND].endvalue;
		imax = (*params)[IPITCHBEND].imax;
		if((*params)[IPITCHBEND].mode != FIXED) {
			if((*params)[IPITCHBEND].dur <= 0.) {
				if(Beta) Alert1("Err. CsScoreWrite(). (*params)[IPITCHBEND].dur <= 0");
				goto OUT;
				}
			alpha1 = (((*scorearg)[2] / ratio) - (*params)[IPITCHBEND].starttime)
				/ (*params)[IPITCHBEND].dur;
			}
		else alpha1 = 0.;
		if(alpha1 > -0.01 && alpha1 < 1.01)
			x = GetTableValue(alpha1,imax,(*params)[IPITCHBEND].point,startvalue,endvalue);
		else x = startvalue;
		cents = (x - DEFTPITCHBEND) * (*p_CsInstrument)[ins].pitchbendrange / DEFTPITCHBEND;
		}
	else cents = 0.;
	switch((*p_CsPitchFormat)[ins]) {
		case OPPC:
			x = octave + ((double) pitchclass) / 100.;
			break;
		case OPD:
			x = ((double)key + deltakey) / 12. + 3.;
			break;
		case CPS:
			x = A4freq * exp((((double)key + deltakey) - ((double)C4key + 9.))
				/ 12. * log(2.));
			x = x * exp((cents / 1200.) * log(2.));
			break;
		}
	(*scorearg)[iarg] = x;
	}


iarg = (*p_CsPitchBendStartIndex)[ins];
if(iarg > 0) {
	if((*params)[IPITCHBEND].active) {
		startvalue = (*params)[IPITCHBEND].startvalue;
		endvalue = (*params)[IPITCHBEND].endvalue;
		imax = (*params)[IPITCHBEND].imax;
		if((*params)[IPITCHBEND].mode != FIXED) {
			if((*params)[IPITCHBEND].dur <= 0.) {
				if(Beta) Alert1("Err. CsScoreWrite(). (*params)[IPITCHBEND].dur <= 0");
				goto OUT;
				}
			alpha1 = (((*scorearg)[2] / ratio) - (*params)[IPITCHBEND].starttime)
				/ (*params)[IPITCHBEND].dur;
			alpha2 = ((((*scorearg)[2] + (*scorearg)[3]) / ratio) - (*params)[IPITCHBEND].starttime)
				/ (*params)[IPITCHBEND].dur;
			}
		else alpha1 = alpha2 = 0.;
		if(alpha1 <= -0.01 || alpha1 >= 1.01 || alpha2 <= -0.01 || alpha2 >= 1.01 || alpha2 < alpha1)
			alpha1 = alpha2 = -1.;
		if(alpha1 > -0.01 && alpha1 < 1.01)
			x = GetTableValue(alpha1,imax,(*params)[IPITCHBEND].point,startvalue,endvalue);
		else x = startvalue;
		}
	else x = DEFTPITCHBEND;
	if(onoffline == LINE) x = (*scorearg)[iarg] + x - DEFTPITCHBEND;
	x = Remap(x,ins,IPITCHBEND,&overflow);
	if(overflow) goto OUT;
	
	if((*params)[IPITCHBEND].active && imax > ZERO && (*params)[IPITCHBEND].mode == CONTINUOUS
			&& (itable=(*p_CsInstrument)[ins].pitchbendtable) > -1) {
		if(MakeCsoundFunctionTable(onoffline,scorearg,alpha1,alpha2,imax,
				(*params)[IPITCHBEND].point,ins,IPITCHBEND,-1,
				(*p_CsPitchBendStartIndex)[ins],(*p_CsPitchBendEndIndex)[ins]) == OK)
			(*scorearg)[itable] = FunctionTable;
		else
			(*scorearg)[itable] = 0;
		}
	(*scorearg)[iarg] = x;
	}
	
iarg = (*p_CsPitchBendEndIndex)[ins];
if(iarg > 0) {	/* (*p_CsPitchBendStartIndex)[ins] was also > 0, therefore alpha2 is known */
	if((*params)[IPITCHBEND].active && (*params)[IPITCHBEND].mode == CONTINUOUS) {
		if(alpha2 > -0.01 && alpha2 < 1.01) {
			x = GetTableValue(alpha2,imax,(*params)[IPITCHBEND].point,startvalue,endvalue);
			if(onoffline == LINE) x = (*scorearg)[iarg] + x - DEFTPITCHBEND;
			x = Remap(x,ins,IPITCHBEND,&overflow);
			if(overflow) goto OUT;
			}
		}
	(*scorearg)[iarg] = x;
	}


iarg = (*p_CsVolumeStartIndex)[ins];
if(iarg > 0) {
	if((*params)[IVOLUME].active) {
		startvalue = (*params)[IVOLUME].startvalue;
		endvalue = (*params)[IVOLUME].endvalue;
		imax = (*params)[IVOLUME].imax;
		if((*params)[IVOLUME].mode != FIXED) {
			alpha1 = (((*scorearg)[2] / ratio) - (*params)[IVOLUME].starttime)
				/ (*params)[IVOLUME].dur;
			alpha2 = ((((*scorearg)[2] + (*scorearg)[3]) / ratio) - (*params)[IVOLUME].starttime)
				/ (*params)[IVOLUME].dur;
			}
		else alpha1 = alpha2 = 0.;
		if(alpha1 <= -0.01 || alpha1 >= 1.01 || alpha2 <= -0.01 || alpha2 >= 1.01 || alpha2 < alpha1)
			alpha1 = alpha2 = -1.;
		if(alpha1 > -0.01 && alpha1 < 1.01)
			x = GetTableValue(alpha1,imax,(*params)[IVOLUME].point,startvalue,endvalue);
		else x = startvalue;
		}
	else x = DeftVolume;
	if(onoffline == LINE) x = (*scorearg)[iarg] * x / DeftVolume;
	x = Remap(x,ins,IVOLUME,&overflow);
	if(overflow) goto OUT;
	if((*params)[IVOLUME].active && imax > ZERO && (*params)[IVOLUME].mode == CONTINUOUS
			&& (itable=(*p_CsInstrument)[ins].volumetable) > -1) {
		if(MakeCsoundFunctionTable(onoffline,scorearg,alpha1,alpha2,imax,
				(*params)[IVOLUME].point,ins,IVOLUME,-1,(*p_CsVolumeStartIndex)[ins],
				(*p_CsVolumeEndIndex)[ins]) == OK)
			(*scorearg)[itable] = FunctionTable;
		else
			(*scorearg)[itable] = 0;
		}
	(*scorearg)[iarg] = x;
	}
	
iarg = (*p_CsVolumeEndIndex)[ins];
if(iarg > 0) {
	if((*params)[IVOLUME].active && (*params)[IVOLUME].mode == CONTINUOUS) {
		if(alpha2 > -0.01 && alpha2 < 1.01) {
			x = GetTableValue(alpha2,imax,(*params)[IVOLUME].point,startvalue,endvalue);
			if(onoffline == LINE) x = (*scorearg)[iarg] * x / DeftVolume;
			x = Remap(x,ins,IVOLUME,&overflow);
			if(overflow) goto OUT;
			}
		}
	(*scorearg)[iarg] = x;
	}


iarg = (*p_CsPressureStartIndex)[ins];
if(iarg > 0) {
	if((*params)[IPRESSURE].active) {
		startvalue = (*params)[IPRESSURE].startvalue;
		endvalue = (*params)[IPRESSURE].endvalue;
		imax = (*params)[IPRESSURE].imax;
		if((*params)[IPRESSURE].mode != FIXED) {
			alpha1 = (((*scorearg)[2] / ratio) - (*params)[IPRESSURE].starttime)
				/ (*params)[IPRESSURE].dur;
			alpha2 = ((((*scorearg)[2] + (*scorearg)[3]) / ratio) - (*params)[IPRESSURE].starttime)
				/ (*params)[IPRESSURE].dur;
			}
		else alpha1 = alpha2 = 0.;
		if(alpha1 <= -0.01 || alpha1 >= 1.01 || alpha2 <= -0.01 || alpha2 >= 1.01 || alpha2 < alpha1)
			alpha1 = alpha2 = -1.;
		if(alpha1 > -0.01 && alpha1 < 1.01)
			x = GetTableValue(alpha1,imax,(*params)[IPRESSURE].point,startvalue,endvalue);
		else x = startvalue;
		}
	else x = DEFTPRESSURE;
	if(onoffline == LINE) x = (*scorearg)[iarg] + x - DEFTPRESSURE;
	x = Remap(x,ins,IPRESSURE,&overflow);
	if(overflow) goto OUT;
	if((*params)[IPRESSURE].active && imax > ZERO && (*params)[IPRESSURE].mode == CONTINUOUS
			&& (itable=(*p_CsInstrument)[ins].pressuretable) > -1) {
		if(MakeCsoundFunctionTable(onoffline,scorearg,alpha1,alpha2,imax,
				(*params)[IPRESSURE].point,ins,IPRESSURE,-1,(*p_CsPressureStartIndex)[ins],
				(*p_CsPressureEndIndex)[ins]) == OK)
			(*scorearg)[itable] = FunctionTable;
		else
			(*scorearg)[itable] = 0;
		}
	(*scorearg)[iarg] = x;
	}
	
iarg = (*p_CsPressureEndIndex)[ins];
if(iarg > 0) {
	if((*params)[IPRESSURE].active && (*params)[IPRESSURE].mode == CONTINUOUS) {
		if(alpha2 > -0.01 && alpha2 < 1.01) {
			x = GetTableValue(alpha2,imax,(*params)[IPRESSURE].point,startvalue,endvalue);
			if(onoffline == LINE) x = (*scorearg)[iarg] + x - DEFTPRESSURE;
			x = Remap(x,ins,IPRESSURE,&overflow);
			if(overflow) goto OUT;
			}
		}
	(*scorearg)[iarg] = x;
	}


iarg = (*p_CsModulationStartIndex)[ins];
if(iarg > 0) {
	if((*params)[IMODULATION].active) {
		startvalue = (*params)[IMODULATION].startvalue;
		endvalue = (*params)[IMODULATION].endvalue;
		imax = (*params)[IMODULATION].imax;
		if((*params)[IMODULATION].mode != FIXED) {
			alpha1 = (((*scorearg)[2] / ratio) - (*params)[IMODULATION].starttime)
				/ (*params)[IMODULATION].dur;
			alpha2 = ((((*scorearg)[2] + (*scorearg)[3]) / ratio) - (*params)[IMODULATION].starttime)
				/ (*params)[IMODULATION].dur;
			}
		else alpha1 = alpha2 = 0.;
		if(alpha1 <= -0.01 || alpha1 >= 1.01 || alpha2 <= -0.01 || alpha2 >= 1.01 || alpha2 < alpha1)
			alpha1 = alpha2 = -1.;
		if(alpha1 > -0.01 && alpha1 < 1.01)
			x = GetTableValue(alpha1,imax,(*params)[IMODULATION].point,startvalue,endvalue);
		else x = startvalue;
		}
	else x = DEFTMODULATION;
	if(onoffline == LINE) x = (*scorearg)[iarg] + x - DEFTMODULATION;
	x = Remap(x,ins,IMODULATION,&overflow);
	if(overflow) goto OUT;
	if((*params)[IMODULATION].active && imax > ZERO && (*params)[IMODULATION].mode == CONTINUOUS
			&& (itable=(*p_CsInstrument)[ins].modulationtable) > -1) {
		if(MakeCsoundFunctionTable(onoffline,scorearg,alpha1,alpha2,imax,
				(*params)[IMODULATION].point,ins,IMODULATION,-1,(*p_CsModulationStartIndex)[ins],
				(*p_CsModulationEndIndex)[ins]) == OK)
			(*scorearg)[itable] = FunctionTable;
		else
			(*scorearg)[itable] = 0;
		}
	(*scorearg)[iarg] = x;
	}
	
iarg = (*p_CsModulationEndIndex)[ins];
if(iarg > 0) {
	if((*params)[IMODULATION].active && (*params)[IMODULATION].mode == CONTINUOUS) {
		if(alpha2 > -0.01 && alpha2 < 1.01) {
			x = GetTableValue(alpha2,imax,(*params)[IMODULATION].point,
				startvalue,endvalue);
			if(onoffline == LINE) x = (*scorearg)[iarg] + x - DEFTMODULATION;
			x = Remap(x,ins,IMODULATION,&overflow);
			if(overflow) goto OUT;
			}
		}
	(*scorearg)[iarg] = x;
	}


iarg = (*p_CsPanoramicStartIndex)[ins];
if(iarg > 0) {
	if((*params)[IPANORAMIC].active) {
		startvalue = (*params)[IPANORAMIC].startvalue;
		endvalue = (*params)[IPANORAMIC].endvalue;
		imax = (*params)[IPANORAMIC].imax;
		if((*params)[IPANORAMIC].mode != FIXED) {
			alpha1 = (((*scorearg)[2] / ratio) - (*params)[IPANORAMIC].starttime)
				/ (*params)[IPANORAMIC].dur;
			alpha2 = ((((*scorearg)[2] + (*scorearg)[3]) / ratio) - (*params)[IPANORAMIC].starttime)
				/ (*params)[IPANORAMIC].dur;
			}
		else alpha1 = alpha2 = 0.;
		if(alpha1 <= -0.01 || alpha1 >= 1.01 || alpha2 <= -0.01 || alpha2 >= 1.01 || alpha2 < alpha1)
			alpha1 = alpha2 = -1.;
		if(alpha1 > -0.01 && alpha1 < 1.01)
			x = GetTableValue(alpha1,imax,(*params)[IPANORAMIC].point,startvalue,endvalue);
		else x = startvalue;
		}
	else x = DeftPanoramic;
	if(onoffline == LINE) x = (*scorearg)[iarg] + x - DeftPanoramic;
	x = Remap(x,ins,IPANORAMIC,&overflow);
	if(overflow) goto OUT;
	if((*params)[IPANORAMIC].active && imax > ZERO && (*params)[IPANORAMIC].mode == CONTINUOUS
			&& (itable=(*p_CsInstrument)[ins].panoramictable) > -1) {
		if(MakeCsoundFunctionTable(onoffline,scorearg,alpha1,alpha2,imax,
				(*params)[IPANORAMIC].point,ins,IPANORAMIC,-1,(*p_CsPanoramicStartIndex)[ins],
				(*p_CsPanoramicEndIndex)[ins]) == OK)
			(*scorearg)[itable] = FunctionTable;
		else
			(*scorearg)[itable] = 0;
		}
	(*scorearg)[iarg] = x;
	}
	
iarg = (*p_CsPanoramicEndIndex)[ins];
if(iarg > 0) {
	if((*params)[IPANORAMIC].active && (*params)[IPANORAMIC].mode == CONTINUOUS) {
		if(alpha2 > -0.01 && alpha2 < 1.01) {
			x = GetTableValue(alpha2,imax,(*params)[IPANORAMIC].point,
				startvalue,endvalue);
			if(onoffline == LINE) x = (*scorearg)[iarg] + x - DeftPanoramic;
			x = Remap(x,ins,IPANORAMIC,&overflow);
			if(overflow) goto OUT;
			}
		}
	(*scorearg)[iarg] = x;
	}

if((*p_CsInstrument)[ins].ipmax > 0 && (*perf)->numberparams > 0) {
	if((*perf)->params == NULL) {
		if(Beta) Alert1("Err. CscoreWrite(). (*perf)->params == NULL");
		goto WRITECSCORELINE;
		}
	if(instrparamlist == NULL) {
		if(Beta) Alert1("Err. CscoreWrite(). instrparamlist == NULL");
		goto WRITECSCORELINE;
		}
	for(i=0; i < (*p_CsInstrument)[ins].ipmax; i++) {
		iarg = (*instrparamlist)[i].startindex;
		if(iarg < 0) continue;
		
		paramnameindex = (*instrparamlist)[i].nameindex;
		if(paramnameindex < 0) continue;
		if(paramnameindex >= (*perf)->numberparams) continue;
		if((*params)[paramnameindex].active) {
			startvalue = (*params)[paramnameindex].startvalue;
			endvalue = (*params)[paramnameindex].endvalue;
			imax = (*params)[paramnameindex].imax;
			if((*params)[paramnameindex].mode != FIXED) {
				alpha1 = (((*scorearg)[2] / ratio) - (*params)[paramnameindex].starttime)
					/ (*params)[paramnameindex].dur;
				alpha2 = ((((*scorearg)[2] + (*scorearg)[3]) / ratio)
					- (*params)[paramnameindex].starttime) / (*params)[paramnameindex].dur;
				}
			else alpha1 = alpha2 = 0.;
			if(alpha1 <= -0.01 || alpha1 >= 1.01 || alpha2 <= -0.01 || alpha2 >= 1.01 || alpha2 < alpha1)
				alpha1 = alpha2 = -1.;
			if(alpha1 > -0.01 && alpha1 < 1.01)
				x = GetTableValue(alpha1,imax,(*params)[paramnameindex].point,startvalue,
					endvalue);
			else x = startvalue;
			}
		else x = (*instrparamlist)[i].defaultvalue;
		if(onoffline == LINE) {
			switch((*instrparamlist)[i].combinationtype) {
				case MULT:
					x = (*scorearg)[iarg] * x / (*instrparamlist)[i].defaultvalue;
					break;
				case ADD:
					x = (*scorearg)[iarg] + x - (*instrparamlist)[i].defaultvalue;
					break;
				default:
					break;
				}
			}
		if((*params)[paramnameindex].active && imax > ZERO
				&& (*params)[paramnameindex].mode == CONTINUOUS
				&& (itable=(*instrparamlist)[i].table) > -1) {
			if(MakeCsoundFunctionTable(onoffline,scorearg,alpha1,alpha2,imax,
					(*params)[paramnameindex].point,ins,paramnameindex,i,
					(*instrparamlist)[i].startindex,(*instrparamlist)[i].endindex) == OK)
				(*scorearg)[itable] = FunctionTable;
			else
				(*scorearg)[itable] = 0;
			}
		(*scorearg)[iarg] = x;
		
		iarg = (*instrparamlist)[i].endindex;
		if(iarg < 0) continue;
		if((*params)[paramnameindex].active && (*params)[paramnameindex].mode == CONTINUOUS) {
			if(alpha2 > -0.01 && alpha2 < 1.01) {
				x = GetTableValue(alpha2,imax,(*params)[paramnameindex].point,
					startvalue,endvalue);
				if(onoffline == LINE) {
					switch((*instrparamlist)[i].combinationtype) {
						case MULT:
							x = (*scorearg)[iarg] * x / (*instrparamlist)[i].defaultvalue;
							break;
						case ADD:
							x = (*scorearg)[iarg] + x - (*instrparamlist)[i].defaultvalue;
							break;
						default:
							break;
						}
					}
				}
			}
		(*scorearg)[iarg] = x;
		}
	}

iarg = (*p_CsAttackVelocityIndex)[ins];
if(iarg > 0 && onoffline != LINE) (*scorearg)[iarg] = (*perf)->velocity[key];
	
iarg = (*p_CsReleaseVelocityIndex)[ins];
if(iarg > 0 && onoffline != LINE) (*scorearg)[iarg] = velocity;

iarg = (*p_CsDilationRatioIndex)[ins];
if(iarg > 0) {
	if(onoffline == LINE) (*scorearg)[iarg] = dilationratio;
	else (*scorearg)[iarg] = (*perf)->dilationratio[key];
	}

// Write Csound event

WRITECSCORELINE:

sprintf(line,"i%ld ",(long)index);
if(!ConvertMIDItoCsound) NoReturnWriteToFile(line,CsRefNum);
strcpy(Message,line);

for(iarg=2; iarg <= iargmax; iarg++) {
	if(iarg != ipitch) {
		if(fabs((*scorearg)[iarg]) < 0.0001) (*scorearg)[iarg] = 0.;
		sprintf(line,"%.3f ",(*scorearg)[iarg]);
		}
	else
		sprintf(line,"%.2f ",(*scorearg)[iarg]);
	if(!ConvertMIDItoCsound) NoReturnWriteToFile(line,CsRefNum);
	strcat(Message,line);
	}

if(pitchclass >= 0) {
	strcpy(line,"; ");
	PrintNote(key,0,-1,LineBuff);
	strcat(line,LineBuff);
	strcat(Message,line);
	}
else line[0] = '\0';

if(!ConvertMIDItoCsound) {
	if(CsoundTrace) Println(wTrace,Message);
	else if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
		else PleaseWait();
	}
else PleaseWait();

if(ConvertMIDItoCsound) Println(wPrototype7,Message);
else WriteToFile(NO,CsoundFileFormat,line,CsRefNum);
result = OK;

OUT:
MyDisposeHandle((Handle*)&scorearg);
if(result == OK && onoffline == OFF) {
	MyDisposeHandle((Handle*)&params);	/* This had been created while onoffline == ON */
	}

if(result == OK && comeback) {
	comeback = FALSE;
	goto SETON;
	}
return(result);
}

#if BP_CARBON_GUI

PrepareCsFile(void)
{
int rep;

if(!CsScoreOpened) {
	if((rep=GetCsoundScoreName()) == OK) 
		return(MakeCsFile(CsFileName));
	if(rep == FAILED) return(FAILED);
	}
switch(FileSaveMode) {
	case ALLSAME:
		return(OK);
		break;
	case ALLSAMEPROMPT:
		sprintf(Message,"Current Csound score file is '%s'. Change it",CsFileName);
		rep = Answer(Message,'N');
		if(rep == ABORT) return(rep);
		if(rep == NO) return(OK);
		/* no break */
	case NEWFILE:
		CloseCsScore();
		return(MakeCsFile(NULL));
		break;
	}
return(OK);
}


MakeCsFile(const char* line)
{
short refnum;
int rep,vref,ishtml;
FSSpec spec;
// FInfo fndrinfo;
OSErr io;
long length;

rep = FAILED;

// need to save the NSWReply record until we close the file
CsFileReply = (NSWReply**)GiveSpace(sizeof(NSWReply));
MyLock(FALSE, (Handle)CsFileReply);
io = NSWInitReply(*CsFileReply);

ShowMessage(TRUE,wMessage,"Create new Csound score file...");
if(line == NULL || line[0] == '\0') {
	// make a new filename based on the project's name
	if (GetProjectBaseName(Message) == OK) {
		// truncate the base filename, leaving enough room for the extension
		if (strlen(Message) > (MAXNAME-4))  Message[MAXNAME-4] = '\0';
		strcat(Message, ".sco");
		}
	else strcpy(Message, "untitled.sco");
	}
else strcpy(Message,line);
c2pstrcpy(PascalLine,Message);
if(NewFile(-1,1,PascalLine,*CsFileReply)) {
	rep = CreateFile(-1,-1,1,PascalLine,*CsFileReply,&refnum);
	if(rep == OK) {
		CsRefNum = refnum;
		p2cstrcpy(CsFileName,PascalLine);
		if(FixCsoundScoreName(CsFileName) == FAILED) {
			spec = (*CsFileReply)->sfFile;
			c2pstrcpy(PascalLine, CsFileName);
			io = FSpRename(&spec,PascalLine);
			if(io != noErr && Beta) {
				Alert1("Err. MakeCsFile(). Can't rename");
				}
			}
		CsScoreOpened = YES;
		if(WriteToFile(NO,CsoundFileFormat,"; Csound score",CsRefNum) != OK) {
			Alert1("Can't write to Csound score file. Unknown error");
			CloseCsScore();
			return(ABORT);
			}
/*		WriteToFile(NO,CsoundFileFormat,"\0",CsRefNum); */
		UpdateWindow(FALSE,Window[wCsoundTables]); /* Update text length */
		ShowSelect(CENTRE,wCsoundTables);
		length = GetTextLength(wCsoundTables);
		ishtml = IsHTML[wCsoundTables];
		IsHTML[wCsoundTables] = FALSE;
		WriteFile(TRUE,CsoundFileFormat,CsRefNum,wCsoundTables,length);
		IsHTML[wCsoundTables] = ishtml;
		WriteToFile(NO,CsoundFileFormat,"\0",CsRefNum);
		SetField(FileSavePreferencesPtr,-1,fCsoundFileName,CsFileName);
		sprintf(Message,"\"%s\"",CsFileName);
		MystrcpyStringToTable(ScriptLine.arg,0,Message);
		AppendScript(180);
		}
	}
if (CsFileReply) MyUnlock((Handle)CsFileReply);
ClearMessage();
return(rep);
}


CloseCsScore(void)
{
long count;
char line[MAXLIN];
OSErr err;

if(!CsScoreOpened) return(OK);

WriteToFile(NO,CsoundFileFormat,"e",CsRefNum);	/* 'e' terminates a Csound score */
Date(line);
sprintf(Message,"; this score was created by Bol Processor BP2 (version %s) on %s",
	VersionName[Version],line);
WriteToFile(NO,CsoundFileFormat,Message,CsRefNum);
GetFPos(CsRefNum,&count);
SetEOF(CsRefNum,count);
FlushFile(CsRefNum);
FSClose(CsRefNum);
CsScoreOpened = FALSE;
if (CsFileReply) {
	MyLock(FALSE, (Handle)CsFileReply);
	(*CsFileReply)->saveCompleted = true;
	err = NSWCleanupReply(*CsFileReply);
	MyDisposeHandle((Handle*)&CsFileReply);
}
sprintf(Message,"Closed Csound score file '%s'",CsFileName);
ShowMessage(TRUE,wMessage,Message);
CsFileName[0] = '\0';
SetField(FileSavePreferencesPtr,-1,fCsoundFileName,CsFileName);
return(OK);
}

#endif /* BP_CARBON_GUI */
