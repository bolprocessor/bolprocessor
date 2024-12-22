/* CsoundScoreMake.c (BP3) */ 

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
 
int trace_cs_scoremake = 0;

int CscoreWrite(Rect* p_graphrect,int leftoffset,int topoffset,int hrect,int minkey,int maxkey,int strikeagain,int onoffline,double dilationratio,Milliseconds time_ms,int iline,
	int key,int velocity,int chan,int instrument,int j,int nseq,int kcurrentinstance,
	PerfParameters ****pp_currentparams,int scale,int blockkey)
{
// j is the sound-object prototype
// onoffline = LINE  -- copy Csound score line from object-prototype
// onoffline = ON -- start a Csound process as a NoteOn has been encountered
// onoffline = OFF -- terminate Csound process as a NoteOff has been encountered, and
//    write Csound score line

int i,jj,k,c,ins,index,paramnameindex,iarg,ip,ipitch,iargmax,octave,changedpitch,overflow,comeback,
	pitchclass,result,maxparam,itable,pitch_format,i_scale;
char line[MAXLIN],line2[MAXLIN];
long imax,pivloc,trbeg,starttime;
double time,x,xx,cents,deltakey,dur,**scorearg,alpha1,alpha2,startvalue,
endvalue,ratio,oldtime_on;
PerfParameters **perf;
ParameterStatus **params,**paramscopy;
Handle h;
CsoundParam **instrparamlist; 
Milliseconds timeon,timeoff;

scorearg = NULL;
params = NULL;
 
result = ABORT; 

if(chan < 0 || chan >= MAXCHAN) {
	if(Beta) Alert1("=> Err. CscoreWrite(). chan < 0 || chan >= MAXCHAN");
	chan = 0;
	}
	
perf = (*pp_currentparams)[nseq]; 

if(trace_cs_scoremake)
	BPPrintMessage(0,odInfo,"\nRunning CscoreWrite for iline = %d channel = %d instrument = %d k = %d\n",iline,chan,instrument,kcurrentinstance);

if(onoffline == LINE) {
	if(j >= Jbol) {
		my_sprintf(Message,"=> Err. CscoreWrite(). j >= Jbol, j = %ld\n",(long)j);
		BPPrintMessage(0,odInfo,Message);
		goto SORTIR;
		} 
	if(iline < 0 || (iline >= (*p_CsoundSize)[j])) {
		my_sprintf(Message,"=> Err. CscoreWrite(). iline < 0 || iline >= (*p_CsoundSize)[j] iline = %ld\n",(long)iline);
		BPPrintMessage(0,odInfo,Message);
		goto SORTIR;
		}
	dur = (*((*pp_CsoundScore)[j]))[iline].duration * dilationratio;
	key = 0;
	}
else {
	if(key < 0 || key >= MAXKEY) {
		my_sprintf(Message,"=> Err. CscoreWrite(). Incorrect key = %ld\n",(long)key);
		BPPrintMessage(0,odInfo,Message);
		goto SORTIR;
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

// if(Pclock > 0.)	/* Striated or measured smooth time */ // Fixed by BB 30 0ct 2020 ???
	// time = ((double) t) * Qclock / ((double) Pclock) / 1000.;
// else 
	// time = ((double) t) / 1000.;

time = ((double) time_ms) / 1000.;	 // Fixed by BB 2022-02-10

if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"Pclock = %ld Qclock = %ld, t = %ld, time = %.3f, onoffline = %d\n",(long)Pclock,(long)Qclock,(long)time_ms,time,onoffline);

comeback = FALSE;
maxparam = (*p_Instance)[kcurrentinstance].contparameters.number; // Fixed by BB 2024-07-05

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
			goto SORTIR;
			}
		}

SETON:
	(*perf)->level[key]++;
	if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"key = %d level = %d\n",key,(*perf)->level[key]);
	(*perf)->starttime[key] = time;
	(*perf)->velocity[key] = velocity;
	(*perf)->dilationratio[key] = dilationratio;
	
//	maxparam = (*((*pp_currentparams)[nseq]))->numberparams; 
//	maxparam = (*p_Instance)[kcurrentinstance].contparameters.number; // Fixed by BB 2024-07-05
	
	if((paramscopy = (ParameterStatus**)
		GiveSpace((Size)(maxparam * sizeof(ParameterStatus)))) == NULL) {
			BPPrintMessage(0,odError,"Err. GiveSpace in CscoreWrite(). maxparam = %ld\n",(long)maxparam);
			goto SORTIR;
			}
	for(i=0; i < maxparam; i++) (*paramscopy)[i] = (*((*perf)->params))[i];
	(*perf)->startparams[key] = paramscopy;
	result = OK;
//	BPPrintMessage(0,odInfo,"Start CscoreWrite(). maxparam = %ld\n",(long)maxparam);
	goto SORTIR;
	}

if(onoffline == OFF && (*perf)->level[key] < 1) {
	BPPrintMessage(0,odError,"=> Err. CscoreWrite(). (*perf)->level[key] < 1 : %ld for key = %ld\n",(long)(*perf)->level[key],(long)key);
	result = OK; // $$$ TEMP
	goto SORTIR;
	}

if(onoffline != LINE && (--((*perf)->level[key])) > 0) {
	result = OK;
	goto SORTIR;
	}

SETOFF:
// Prepare Csound score line

if(onoffline != LINE)
	params = (*perf)->startparams[key];
else
	params = (*perf)->params;

if(params == NULL) {
	if(Beta) Alert1("=> Err. CscoreWrite(). params == NULL");
	goto SORTIR;
	}

iargmax = (*p_CsInstrument)[ins].iargmax;
if(iargmax < 4) {
	if(Beta) Alert1("=> Err. CscoreWrite(). iargmax < 4");
	iargmax = 4;
	}

if((scorearg=(double**) GiveSpace((Size)((iargmax + 1) * sizeof(double)))) == NULL)
	goto SORTIR;
	
for(iarg=0; iarg <= iargmax; iarg++) (*scorearg)[iarg] = 0.;

// if(Pclock > 0.)  /* Striated or measured smooth time */ 
//	ratio = Qclock / ((double) Pclock) / 1000.;
// else
//	ratio = 0.001;

ratio = 0.001; // Fixed by BB 2022-02-10
		
if(onoffline != LINE) {
//	(*scorearg)[2] = (*perf)->starttime[key] * Qclock / ((double) Pclock);
	(*scorearg)[2] = (*perf)->starttime[key]; // Fixed by BB 2022-02-10
	(*scorearg)[3] = time - (*scorearg)[2];
	if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"onoffline != LINE, key = %d, starttime = %.3f, time = %ld, scorearg[2] = %.3f, scorearg[3] = %.3f\n",key,(*perf)->starttime[key],(long)time,(*scorearg)[2],(*scorearg)[3]);
	}
else {
	(*scorearg)[2] = time;
	(*scorearg)[3] = dur * ratio;
	if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"onoffline == LINE, scorearg[2] = %ld, scorearg[3] = %ld\n",(long)(*scorearg)[2],(long)(*scorearg)[3]);
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
				deltakey = C4key - 60 + 12. * (octave - 3.) + pitchclass;
				break;
			case OPD:
				deltakey = C4key - 60 + (x - 3.) * 12.;
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
	pitchclass = modulo(key,12);
	octave = (key - pitchclass) / 12 + 3;
	if((*p_CsPitchBendStartIndex)[ins] == -1
			&& (*params)[IPITCHBEND].active && (*p_CsInstrument)[ins].pitchbendrange > 0.) {
		startvalue = (*params)[IPITCHBEND].startvalue;
		endvalue = (*params)[IPITCHBEND].endvalue;
		imax = (*params)[IPITCHBEND].imax;
		if((*params)[IPITCHBEND].mode != FIXED) {
			if((*params)[IPITCHBEND].dur <= 0.) {
				if(Beta) Alert1("=> Err. CsScoreWrite(). (*params)[IPITCHBEND].dur <= 0");
				goto SORTIR;
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
	pitch_format = (*p_CsPitchFormat)[ins];
	// BPPrintMessage(0,odInfo,"The A4freq = %.3f\n",A4freq);
	if((pitch_format == OPPC || pitch_format == OPD) && (A4freq != 440. || C4key != 60))
		pitch_format = CPS;
	if(scale <= 0) i_scale = 0;
	if(NumberScales > 0 && scale >= 0) {
		// BPPrintMessage(0,odInfo,"The scale = %d\n",scale);
		if(scale == -1) {
			i_scale = 1;
			if(trace_scale) BPPrintMessage(0,odInfo,"Default scale will be used\n");
			}
		else {
			i_scale = FindScale(scale);
	/*		MystrcpyHandleToString(MAXLIN,0,Message,(*p_StringConstant)[scale]);
			if(trace_scale) BPPrintMessage(0,odInfo,"scale = %d => \"%s\"\n",scale,Message);
			for(i_scale = 1; i_scale <= NumberScales; i_scale++) {
			// ‘scale’ is not the index of the scale. It is the index of its name in StringConstant
				result = MyHandlecmp((*p_StringConstant)[scale],(*Scale)[i_scale].label);
				if(result == 0) break;
				} */
			}
		if(i_scale > 0 && i_scale <= NumberScales) { 
		//	BPPrintMessage(0,odInfo,"blockkey = %d\n",blockkey);
			x = GetPitchWithScale(i_scale,kcurrentinstance,key,cents,blockkey);
			if(x == Infpos) return(ABORT);
			pitch_format = IGNORER;
			}
		}
	switch(pitch_format) {
		case OPPC:
			x = octave + ((double) pitchclass) / 100.;
			break;
		case OPD:
			x = ((double)key + deltakey) / 12. + 3.;
			break;
		case CPS:
			x = A4freq * exp((((double)key + deltakey) - ((double)C4key + 9.)) / 12. * log(2.));
			x = x * exp((cents / 1200.) * log(2.));
			break;
		case IGNORER: break;
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
				if(Beta) Alert1("=> Err. CsScoreWrite(). (*params)[IPITCHBEND].dur <= 0");
				goto SORTIR;
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
	if(overflow) goto SORTIR;
	
	if((*params)[IPITCHBEND].active && imax > ZERO && (*params)[IPITCHBEND].mode == CONTINUOUS
			&& (itable=(*p_CsInstrument)[ins].pitchbendtable) > -1) {
		// BPPrintMessage(0,odInfo,"Calling MakeCsoundFunctionTable() for IPITCHBEND = %ld\n");
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
			if(overflow) goto SORTIR;
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
	if(overflow) goto SORTIR;
	if((*params)[IVOLUME].active && imax > ZERO && (*params)[IVOLUME].mode == CONTINUOUS
			&& (itable=(*p_CsInstrument)[ins].volumetable) > -1) {
		// BPPrintMessage(0,odInfo,"Calling MakeCsoundFunctionTable() for IVOLUME = %ld\n");
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
			if(overflow) goto SORTIR;
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
	if(overflow) goto SORTIR;
	if((*params)[IPRESSURE].active && imax > ZERO && (*params)[IPRESSURE].mode == CONTINUOUS
			&& (itable=(*p_CsInstrument)[ins].pressuretable) > -1) {
		// BPPrintMessage(0,odInfo,"Calling MakeCsoundFunctionTable() for IPRESSURE = %ld\n");
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
			if(overflow) goto SORTIR;
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
	if(overflow) goto SORTIR;
	if((*params)[IMODULATION].active && imax > ZERO && (*params)[IMODULATION].mode == CONTINUOUS
			&& (itable=(*p_CsInstrument)[ins].modulationtable) > -1) {
		// BPPrintMessage(0,odInfo,"Calling MakeCsoundFunctionTable() for IMODULATION = %ld\n");
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
			if(overflow) goto SORTIR;
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
	if(overflow) goto SORTIR;
	if((*params)[IPANORAMIC].active && imax > ZERO && (*params)[IPANORAMIC].mode == CONTINUOUS
			&& (itable=(*p_CsInstrument)[ins].panoramictable) > -1) {
		//	BPPrintMessage(0,odInfo,"Calling MakeCsoundFunctionTable() for IPANORAMIC = %ld\n");
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
			if(overflow) goto SORTIR;
			}
		}
	(*scorearg)[iarg] = x;
	}

if((*p_CsInstrument)[ins].ipmax > 0 && (*perf)->numberparams > 0) {
	if((*perf)->params == NULL) {
		if(Beta) Alert1("=> Err. CscoreWrite(). (*perf)->params == NULL");
		goto WRITECSCORELINE;
		}
	if(instrparamlist == NULL) {
		if(Beta) Alert1("=> Err. CscoreWrite(). instrparamlist == NULL");
		goto WRITECSCORELINE;
		}
	for(i=0; i < (*p_CsInstrument)[ins].ipmax; i++) {
		iarg = (*instrparamlist)[i].startindex;
		if(iarg < 0) continue;
		
		paramnameindex = (*instrparamlist)[i].nameindex;
		if(paramnameindex < 0) continue;
		if(paramnameindex >= (*perf)->numberparams) continue;
		BPPrintMessage(0,odInfo,"paramnameindex = %d, (*perf)->numberparams = %d\n",(int)paramnameindex,(*perf)->numberparams);
		BPPrintMessage(0,odInfo,"(*params)[paramnameindex].active = %d\n",(int)(*params)[paramnameindex].active);
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
			//		BPPrintMessage(0,odInfo,"MULT iarg = %ld, x = %.3f, (*scorearg)[iarg] = %.3f, (*instrparamlist)[i].defaultvalue = %.3f\n",(long)iarg,x,(*scorearg)[iarg],(*instrparamlist)[i].defaultvalue);
					x = (*scorearg)[iarg] * x / (*instrparamlist)[i].defaultvalue;
					break;
				case ADD:
			//		BPPrintMessage(0,odInfo,"ADD iarg = %ld, x = %.3f, (*scorearg)[iarg] = %.3f, (*instrparamlist)[i].defaultvalue = %.3f\n",(long)iarg,x,(*scorearg)[iarg],(*instrparamlist)[i].defaultvalue);
					x = (*scorearg)[iarg] + x - (*instrparamlist)[i].defaultvalue;
					break;
				default:
					break;
				}
			}
		if((*params)[paramnameindex].active && imax > ZERO
				&& (*params)[paramnameindex].mode == CONTINUOUS
				&& (itable=(*instrparamlist)[i].table) > -1) {
		//	BPPrintMessage(0,odInfo,"Calling MakeCsoundFunctionTable() for paramnameindex = %ld\n",(long)paramnameindex);
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
					//		BPPrintMessage(0,odInfo,"MULT iarg = %ld, (*scorearg)[iarg] = %.3f, (*instrparamlist)[i].defaultvalue = %.3f\n",(long)iarg,(*scorearg)[iarg],(*instrparamlist)[i].defaultvalue);
							x = (*scorearg)[iarg] * x / (*instrparamlist)[i].defaultvalue;
					//	BPPrintMessage(0,odInfo,"MULT \n");
							break;
						case ADD:
					//		BPPrintMessage(0,odInfo,"ADD iarg = %ld, (*scorearg)[iarg] = %.3f, (*instrparamlist)[i].defaultvalue = %.3f\n",(long)iarg,(*scorearg)[iarg],(*instrparamlist)[i].defaultvalue);
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

// First send this note to pianoroll
if(ShowPianoRoll) {
	timeon = (Milliseconds) 1000 * (*scorearg)[2];
	timeoff = (Milliseconds) 1000 * ((*scorearg)[2] + (*scorearg)[3]); // Fixed by BB 2022-02-10
		
	if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"key = %d chan = %d timeon = %ld timeoff = %ld minkey = %d maxkey = %d\n",key,chan,(long)timeon,(long)timeoff,minkey,maxkey);
	DrawPianoNote("csound",key,chan,timeon,timeoff,leftoffset,topoffset,hrect,minkey,maxkey,p_graphrect);
//	BPPrintMessage(0,odInfo," key #%d draw_line(%d,%d) p_r->left = %d\n",key,timeon,timeoff,p_graphrect->left);
	}

if(!OutCsound) {
	result = OK;
	if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"going out because !OutCsound)\n");
	goto SORTIR;
	}

// Now to the score
my_sprintf(line,"i%ld ",(long)index);
if(!ConvertMIDItoCsound) NoReturnWriteToFile(line,CsRefNum);
strcpy(Message,line);
my_sprintf(line2,"iarg = 1 line = %s\n",line);
if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"- %s",line2);

for(iarg=2; iarg <= iargmax; iarg++) {
	if(iarg != ipitch || pitch_format == IGNORER || pitch_format == CPS) {
		if(fabs((*scorearg)[iarg]) < 0.0001) (*scorearg)[iarg] = 0.;
		my_sprintf(line,"%.3f ",(*scorearg)[iarg]);
		}
	else
		my_sprintf(line,"%.2f ",(*scorearg)[iarg]);
	if(!ConvertMIDItoCsound) NoReturnWriteToFile(line,CsRefNum);
	strcat(Message,line);
	
	my_sprintf(line2,"iarg = %ld -> %s\n",(long)iarg,line);
	if(trace_cs_scoremake) BPPrintMessage(0,odInfo,line2);
	}

if(pitchclass >= 0) {
	strcpy(line,"; ");
	PrintThisNote(i_scale,key,0,-1,LineBuff);
	strcat(line,LineBuff);
	int cent_value = (int) cents;
	if(cent_value != 0) {
		if(cent_value > 0) my_sprintf(LineBuff," + %d cents",cent_value);
		if(cent_value < 0) my_sprintf(LineBuff," - %d cents",cent_value);
		strcat(line,LineBuff);
		}
	strcat(Message,line);
	}
else line[0] = '\0';

if(!ConvertMIDItoCsound) {
	if(CsoundTrace) ShowMessage(TRUE,wMessage,Message);
//	else if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
//		else PleaseWait();
	}
// else PleaseWait();


if(ConvertMIDItoCsound) Println(wPrototype7,Message);
else WriteToFile(NO,CsoundFileFormat,line,CsRefNum);
result = OK;

if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"line = %s\n",line);

strcpy(Message,"");


SORTIR:
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


/* int PrepareCsFile(void)
{
int rep;

 if(!CsScoreOpened) {
	if((rep=GetCsoundScoreName()) == OK) 
		return(MakeCsFile(CsFileName));
	if(rep == MISSED) return(MISSED);
	}
switch(FileSaveMode) {
	case ALLSAME:
		return(OK);
		break;
	case ALLSAMEPROMPT:
		my_sprintf(Message,"Current Csound score file is '%s'. Change it",CsFileName);
		rep = Answer(Message,'N');
		if(rep == ABORT) return(rep);
		if(rep == NO) return(OK);
	case NEWFILE:
		CloseCsScore();
	//	return(MakeCsFile(NULL));
		break;
	}
return(OK);
} */


/* int MakeCsFile(const char* line)
{
short refnum;
int rep,vref,ishtml;
FSSpec spec;
// FInfo fndrinfo;
OSErr io;
long length;

rep = MISSED;

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
		if(FixCsoundScoreName(CsFileName) == MISSED) {
			spec = (*CsFileReply)->sfFile;
			c2pstrcpy(PascalLine, CsFileName);
			io = FSpRename(&spec,PascalLine);
			if(io != noErr && Beta) {
				Alert1("=> Err. MakeCsFile(). Can't rename");
				}
			}
		CsScoreOpened = YES;
		if(WriteToFile(NO,CsoundFileFormat,"; Csound score",CsRefNum) != OK) {
			Alert1("Can't write to Csound score file. Unknown error");
			CloseCsScore();
			return(ABORT);
			}
		}
	}
if (CsFileReply) MyUnlock((Handle)CsFileReply);
ClearMessage();
return(rep);
} */


/* CloseCsScore(void)
{
long count;
char line[MAXLIN];
OSErr err;
long timeout;

if(!CsScoreOpened) return(OK);
if(EndFadeOut > 0.) {
	my_sprintf(line,"e %.3f",EndFadeOut);
	}
else strcpy(line,"e");
WriteToFile(NO,CsoundFileFormat,line,CsRefNum);	// 'e' terminates a Csound score 
Date(line);
my_sprintf(Message,"; this score was created by Bol Processor BP2 (version %s) on %s",
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
my_sprintf(Message,"Closing Csound score file '%s'",CsFileName);
ShowMessage(TRUE,wMessage,Message);
// CsFileName[0] = '\0';
strcpy(CsFileName,""); // Fixed by BB 2021-02-14
SetField(FileSavePreferencesPtr,-1,fCsoundFileName,CsFileName);
return(OK);
} */


