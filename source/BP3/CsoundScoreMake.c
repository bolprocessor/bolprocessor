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


#ifndef _H_BP3
#include "-BP3.h"
#endif

#include "-BP3decl.h"
 
int trace_cs_scoremake = 0;

int CscoreWrite(Milliseconds cut_the_end,Rect* p_graphrect,int leftoffset,int topoffset,int hrect,int minkey,int maxkey,int strikeagain,int onoffline,double beta,Milliseconds time_ms,int iline,int key,int velocity,int chan,int instrument,int j,int nseq,int kcurrentinstance,PerfParameters ****pp_currentparams,int scale,int blockkey) {
	// j is the sound-object prototype
	// onoffline = LINE  -- copy Csound score line from object-prototype
	// onoffline = ON -- start a Csound process as a NoteOn has been encountered
	// onoffline = OFF -- terminate Csound process as a NoteOff has been encountered, and
	//    write Csound score line

	int i,jj,k,c,ins,index,paramindex,paramnameindex,this_instrument,iarg,ip,ipitch,iargmax,octave,changedpitch,overflow,comeback,pitchclass,result,maxparam,itable,pitch_format,i_scale,mode,cents2;
	char line[MAXLIN],line2[MAXLIN];
	long imax,pivloc,trbeg,starttime;
	double time,x,xx,cents,deltakey,dur,**scorearg,alpha1,alpha2,startvalue,endvalue,ratio,oldtime_on;
	PerfParameters **perf;
	ParameterStatus **params,**paramscopy;
	Handle h;
	CsoundParam **instrparamlist; 
	Milliseconds timeon,timeoff;

	scorearg = NULL;
	params = NULL;
	
	result = ABORT; 
	cents2 = 0;

	if(chan < 0 || chan >= MAXCHAN) {
		BPPrintMessage(0,odError,"=> Err. CscoreWrite(ZERO,). chan < 0 || chan >= MAXCHAN");
		chan = 0;
		}
		
	perf = (*pp_currentparams)[nseq]; 

	if(trace_cs_scoremake)
		BPPrintMessage(0,odInfo,"@@@ instrument = %d, chan = %d\n",instrument,chan);

	mode = -1;
	if(iline >= 0 && j > 0 && j < Jbol) {
		mode = (*p_CsoundInstrumentMode)[j];
		if(mode > 0) instrument = (*p_CsoundInstr)[j];
		else if(mode == LOCAL_CS) instrument = (*((*pp_CsoundScore)[j]))[iline].instrument;
		else if(instrument <= 0) {
			if(WhichCsoundInstrument[chan+1] > 0) instrument = WhichCsoundInstrument[chan+1];
			else instrument = 1;
			}
		}

	if(trace_cs_scoremake)
		BPPrintMessage(0,odInfo,"\nRunning CscoreWrite for iline = %d channel = %d instrument = %d k = %d, j = %d, onoffline = %d, mode = %d\n",iline,chan,instrument,kcurrentinstance,j,onoffline,mode);

	if(onoffline == LINE) {
		if(j >= Jbol) {
			my_sprintf(Message,"=> Err. CscoreWrite(ZERO,). j >= Jbol, j = %ld\n",(long)j);
			BPPrintMessage(0,odInfo,Message);
			goto SORTIR;
			} 
		if(iline < 0 || (iline >= (*p_CsoundSize)[j])) {
			my_sprintf(Message,"=> Err. CscoreWrite(ZERO,). iline < 0 || iline >= (*p_CsoundSize)[j] iline = %ld\n",(long)iline);
			BPPrintMessage(0,odInfo,Message);
			goto SORTIR;
			}
		dur = (*((*pp_CsoundScore)[j]))[iline].duration * beta - (double) cut_the_end;
		if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"CsoundScore[%d][%d] duration = %.2f\n",j,iline,dur);
		key = 0;
		}
	else {
		if(key < 0 || key >= MAXKEY) {
			my_sprintf(Message,"=> Err. CscoreWrite(ZERO,). Incorrect key = %ld\n",(long)key);
			BPPrintMessage(0,odInfo,Message);
			goto SORTIR;
			}
		}

	if(Jinstr < 2) {
		ins = 0;
		if((*p_CsInstrumentIndex)[ins] == -1) index = 1;
		else index = (*p_CsInstrumentIndex)[ins];
		}
	else {
		if(onoffline == LINE) {
			index = instrument;
			}
		else {
			if(instrument > 0) index = instrument;
			else if(j < Jbol && (*p_CsoundInstrumentMode)[j] >= 0) index = (*p_CsoundInstrumentMode)[j];
			else if(WhichCsoundInstrument[chan+1] >= 0) index = WhichCsoundInstrument[chan+1];
			else index = 0;
			}
		for(ins = 0; ins < Jinstr; ins++) {
			if((*p_CsInstrumentIndex)[ins] == index) break;
			}
		if(index == 0 || ins >= Jinstr) {
			ins = 0; index = 1;
			}
		}

	instrparamlist = (*p_CsInstrument)[ins].paramlist;

	time = ((double) time_ms) / 1000.;

	if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"Pclock = %ld Qclock = %ld, t = %ld, time = %.3f, onoffline = %d, ins = %d, index = instrument = %d\n",(long)Pclock,(long)Qclock,(long)time_ms,time,onoffline,ins,index);

	comeback = FALSE;
	maxparam = (*p_Instance)[kcurrentinstance].contparameters.number;
	(*((*pp_currentparams)[nseq]))->numberparams = maxparam;

	if(onoffline == ON) {
		if((*perf)->level[key] > 0) {
		// Here we're in trouble because of two consecutive NoteOn's
		// The solution will be to use the 'sequence' information
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
		(*perf)->beta[key] = beta;
		
		if((paramscopy = (ParameterStatus**)
			GiveSpace((Size)(maxparam * sizeof(ParameterStatus)))) == NULL) {
				BPPrintMessage(0,odError,"Err. GiveSpace in CscoreWrite(ZERO,). maxparam = %ld\n",(long)maxparam);
				goto SORTIR;
				}
		for(i = 0; i < maxparam; i++) (*paramscopy)[i] = (*((*perf)->params))[i];
		(*perf)->startparams[key] = paramscopy;
		result = OK;
	//	BPPrintMessage(0,odInfo,"Start CscoreWrite(ZERO,). maxparam = %ld\n",(long)maxparam);
		goto SORTIR;
		}

	if(onoffline == OFF && (*perf)->level[key] < 0) {
		BPPrintMessage(0,odError,"=> Err. CscoreWrite(ZERO,). (*perf)->level[key] < 0 : %ld for key = %ld\n",(long)(*perf)->level[key],(long)key);
		// (*perf)->level[key] = 1;
		result = OK; // $$$ TEMP
		goto SORTIR;
		}

	if(onoffline != LINE && (--((*perf)->level[key])) > 0) {
		result = OK;
		goto SORTIR;
		}

	SETOFF:
	// Prepare Csound score line

	if(onoffline != LINE) {
		params = (*perf)->startparams[key];
	//	BPPrintMessage(0,odInfo,"§§§ !LINE params[5].imax = %d, key = %d, maxparam = %d\n",(*params)[5].imax,key,maxparam);
		}
	else {
		params = (*perf)->params;
	//	BPPrintMessage(0,odInfo,"§§§ LINE params[5].imax = %d\n",(*params)[5].imax);
		}

	if(params == NULL) {
	//	BPPrintMessage(0,odError,"=> Err. CscoreWrite(ZERO,). params == NULL\n");
		result = OK;
		goto SORTIR;
		}

	iargmax = (*p_CsInstrument)[ins].iargmax;
	if(iargmax < 4) {
		BPPrintMessage(0,odError,"=> Err. CscoreWrite(ZERO,). iargmax < 4");
		iargmax = 4;
		}

	if((scorearg=(double**) GiveSpace((Size)((iargmax + 1) * sizeof(double)))) == NULL)
		goto SORTIR;
		
	for(iarg = 0; iarg <= iargmax; iarg++) (*scorearg)[iarg] = 0.;

	if(onoffline == LINE) {
		if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"@@+ iline = %d\n",iline);
		if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"@@@ ins = %d, iargmax = %d, nbparameters = %d\n",ins,iargmax,(*((*pp_CsoundScore)[j]))[iline].nbparameters);
		}

	ratio = 0.001;
			
	if(onoffline != LINE) {
		(*scorearg)[2] = (*perf)->starttime[key];
		(*scorearg)[3] = time - (*scorearg)[2];
		if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"onoffline != LINE, key = %d, starttime = %.3f, time = %ld, scorearg[2] = %.3f, scorearg[3] = %.3f, iargmax = %d\n",key,(*perf)->starttime[key],(long)time,(*scorearg)[2],(*scorearg)[3],iargmax);
		}
	else {
		(*scorearg)[2] = time;
		(*scorearg)[3] = dur * ratio;
		if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"onoffline == LINE, scorearg[2] = %.3f, scorearg[3] = %.3f, number arguments = %d, iargmax = %d\n",(long)(*scorearg)[2],(long)(*scorearg)[3],(*((*pp_CsoundScore)[j]))[iline].nbparameters,iargmax);
		for(iarg=4; iarg < (4+(*((*pp_CsoundScore)[j]))[iline].nbparameters) && iarg <= iargmax; iarg++) {
			(*scorearg)[iarg] = (*((*((*pp_CsoundScore)[j]))[iline].h_param))[iarg-4];
		//	if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"iarg = %d, scorearg = %.2f\n",iarg,(*scorearg)[iarg]);
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
				//	BPPrintMessage(0,odInfo,"### deltakey = %.2f, x = %.2f\n",deltakey,x);
					break;
				}
		//	key = MyInt(deltakey);
			key = (int)lround(deltakey);
			deltakey -= key;
			cents2 = (int) (100. * deltakey);
			if(j < Jbol) {
				if((*p_OkTransp)[j] && (*perf)->transposefirst)
					TransposeKey(&key,(*perf)->transpose);
				if((*p_OkMap)[j])
					key = ExpandKey(key,(*perf)->xpandkey,(*perf)->xpandval);
				if((*p_OkTransp)[j] && (!(*perf)->transposefirst))
					TransposeKey(&key,(*perf)->transpose);
				if((*p_OkMap)[j])
					key = MapThisKey(key,0.,(*p_Instance)[kcurrentinstance].mapmode,
								&((*p_Instance)[kcurrentinstance].map0),
								&((*p_Instance)[kcurrentinstance].map1));

				}
			}
		else deltakey = 0.;
		pitchclass = modulo(key,12);
		octave = (key - pitchclass) / 12 + 3;
		if(trace_cs_scoremake && (*params)[IPITCHBEND].active)
			BPPrintMessage(0,odInfo,"• (*params)[IPITCHBEND] is active, ins = %d range = %d, index = %d\n",(int)ins,(int)(*p_CsInstrument)[ins].pitchbendrange,(*p_CsPitchBendStartIndex)[ins]);
	//	if((*p_CsPitchBendStartIndex)[ins] != -1 && (*params)[IPITCHBEND].active && (*p_CsInstrument)[ins].pitchbendrange > 0.) { // 2025-10-27
		if((*p_CsInstrument)[ins].pitchbendrange > 0. && (*params)[IPITCHBEND].active) { 
			startvalue = (*params)[IPITCHBEND].startvalue;
			endvalue = (*params)[IPITCHBEND].endvalue;
			imax = (*params)[IPITCHBEND].imax;
			if(trace_cs_scoremake) 
				BPPrintMessage(0,odInfo,"• pitchbend startvalue = %d, endvalue = %d\n",(int)startvalue,(int)endvalue);
			if((*params)[IPITCHBEND].mode != FIXMAPMODE) {
				if((*params)[IPITCHBEND].dur <= 0.) {
					BPPrintMessage(0,odError,"=> Err. CsScoreWrite(). (*params)[IPITCHBEND].dur <= 0");
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
			if((*params)[IPITCHBEND].mode != FIXMAPMODE) {
				if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"(pitchbend mode is not FIXMAPMODE)\n");
				if((*params)[IPITCHBEND].dur <= 0.) {
					BPPrintMessage(0,odError,"=> Err. CsScoreWrite(). (*params)[IPITCHBEND].dur <= 0");
					goto SORTIR;
					}
				alpha1 = (((*scorearg)[2] / ratio) - (*params)[IPITCHBEND].starttime)
					/ (*params)[IPITCHBEND].dur;
				alpha2 = ((((*scorearg)[2] + (*scorearg)[3]) / ratio) - (*params)[IPITCHBEND].starttime)
					/ (*params)[IPITCHBEND].dur;
				}
			else {
				alpha1 = alpha2 = 0.;
				if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"(pitchbend mode is FIXMAPMODE)\n");
				}
		/*	if(alpha1 <= -0.01 || alpha1 >= 1.01 || alpha2 <= -0.01 || alpha2 >= 1.01 || alpha2 < alpha1)
				alpha1 = alpha2 = -1.; */
			if(alpha1 > -0.1 && alpha1 < 1.1) {
				x = GetTableValue(alpha1,imax,(*params)[IPITCHBEND].point,startvalue,endvalue);
				}
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
	if(iarg > 0) {	// (*p_CsPitchBendStartIndex)[ins] was also > 0, therefore alpha2 is known
		if((*params)[IPITCHBEND].active && (*params)[IPITCHBEND].mode == CONTINUOUS) {
			if(alpha2 > -0.1 && alpha2 < 1.1) {
				x = GetTableValue(alpha2,imax,(*params)[IPITCHBEND].point,startvalue,endvalue);
				if(onoffline == LINE) x = (*scorearg)[iarg] + x - DEFTPITCHBEND;
				x = Remap(x,ins,IPITCHBEND,&overflow);
				if(overflow) goto SORTIR;
				}
			}
		(*scorearg)[iarg] = x;
		}

	iarg = (*p_CsVolumeStartIndex)[ins];
//	BPPrintMessage(0,odInfo,"&&& iarg = %d, ins = %d, active = %d\n",iarg,ins,(*params)[IVOLUME].active);
	if(iarg > 0) {
		if((*params)[IVOLUME].active) {
			startvalue = (*params)[IVOLUME].startvalue;
			endvalue = (*params)[IVOLUME].endvalue;
			imax = (*params)[IVOLUME].imax;
			if((*params)[IVOLUME].mode != FIXMAPMODE) {
				alpha1 = (((*scorearg)[2] / ratio) - (*params)[IVOLUME].starttime)
					/ (*params)[IVOLUME].dur;
				alpha2 = ((((*scorearg)[2] + (*scorearg)[3]) / ratio) - (*params)[IVOLUME].starttime)
					/ (*params)[IVOLUME].dur;
				}
			else alpha1 = alpha2 = 0.;
		/*	if(alpha1 <= -0.01 || alpha1 >= 1.01 || alpha2 <= -0.01 || alpha2 >= 1.01 || alpha2 < alpha1)
				alpha1 = alpha2 = -1.; */
			if(alpha1 > -0.1 && alpha1 < 1.1)
				x = GetTableValue(alpha1,imax,(*params)[IVOLUME].point,startvalue,endvalue);
			else x = startvalue;
		//	BPPrintMessage(0,odInfo,"--1 x = %.2f @ %.3f, alpha1 = %.2f, alpha2 = %.2f, startvalue = %.2f, endvalue = %.2f\n",x,alpha1,alpha2,startvalue,endvalue);
			}
		else x = DeftVolume;
	//	BPPrintMessage(0,odInfo,"++1 x = %.2f @ %.3f, DeftVolume = %d, alpha1 = %.2f, alpha2 = %.2f\n",x,(*scorearg)[2],DeftVolume,alpha1,alpha2);
		if(onoffline == LINE) x = (*scorearg)[iarg] * x / DeftVolume;
		x = Remap(x,ins,IVOLUME,&overflow);
	//	BPPrintMessage(0,odInfo,"--- x = %.2f @ %.3f\n",x,(*scorearg)[2]);
		if(overflow) goto SORTIR;
		// BPPrintMessage(0,odInfo,"++2 x = %.2f @ %.3f\n",x,(*scorearg)[2]);
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
			if(alpha2 > -0.1 && alpha2 < 1.1) {
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
			if((*params)[IPRESSURE].mode != FIXMAPMODE) {
				alpha1 = (((*scorearg)[2] / ratio) - (*params)[IPRESSURE].starttime)
					/ (*params)[IPRESSURE].dur;
				alpha2 = ((((*scorearg)[2] + (*scorearg)[3]) / ratio) - (*params)[IPRESSURE].starttime)
					/ (*params)[IPRESSURE].dur;
				}
			else alpha1 = alpha2 = 0.;
		/*	if(alpha1 <= -0.01 || alpha1 >= 1.01 || alpha2 <= -0.01 || alpha2 >= 1.01 || alpha2 < alpha1)
				alpha1 = alpha2 = -1.; */
			if(alpha1 > -0.1 && alpha1 < 1.1)
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
					(*params)[IPRESSURE].point,ins,IPRESSURE,-1,(*p_CsPressureStartIndex)[ins],(*p_CsPressureEndIndex)[ins]) == OK)
				(*scorearg)[itable] = FunctionTable;
			else
				(*scorearg)[itable] = 0;
			}
		(*scorearg)[iarg] = x;
		}
		
	iarg = (*p_CsPressureEndIndex)[ins];
	if(iarg > 0) {
		if((*params)[IPRESSURE].active && (*params)[IPRESSURE].mode == CONTINUOUS) {
			if(alpha2 > -0.1 && alpha2 < 1.1) {
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
			if((*params)[IMODULATION].mode != FIXMAPMODE) {
				alpha1 = (((*scorearg)[2] / ratio) - (*params)[IMODULATION].starttime)
					/ (*params)[IMODULATION].dur;
				alpha2 = ((((*scorearg)[2] + (*scorearg)[3]) / ratio) - (*params)[IMODULATION].starttime)
					/ (*params)[IMODULATION].dur;
				}
			else alpha1 = alpha2 = 0.;
		/*	if(alpha1 <= -0.01 || alpha1 >= 1.01 || alpha2 <= -0.01 || alpha2 >= 1.01 || alpha2 < alpha1)
				alpha1 = alpha2 = -1.; */
			if(alpha1 > -0.1 && alpha1 < 1.1)
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
					(*params)[IMODULATION].point,ins,IMODULATION,-1,(*p_CsModulationStartIndex)[ins],(*p_CsModulationEndIndex)[ins]) == OK)
				(*scorearg)[itable] = FunctionTable;
			else
				(*scorearg)[itable] = 0;
			}
		(*scorearg)[iarg] = x;
		}
		
	iarg = (*p_CsModulationEndIndex)[ins];
	if(iarg > 0) {
		if((*params)[IMODULATION].active && (*params)[IMODULATION].mode == CONTINUOUS) {
			if(alpha2 > -0.1 && alpha2 < 1.1) {
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
			if((*params)[IPANORAMIC].mode != FIXMAPMODE) {
				alpha1 = (((*scorearg)[2] / ratio) - (*params)[IPANORAMIC].starttime)
					/ (*params)[IPANORAMIC].dur;
				alpha2 = ((((*scorearg)[2] + (*scorearg)[3]) / ratio) - (*params)[IPANORAMIC].starttime)
					/ (*params)[IPANORAMIC].dur;
				}
			else alpha1 = alpha2 = 0.;
	/*		if(alpha1 <= -0.01 || alpha1 >= 1.01 || alpha2 <= -0.01 || alpha2 >= 1.01 || alpha2 < alpha1)
				alpha1 = alpha2 = -1.; */
			if(alpha1 > -0.1 && alpha1 < 1.0)
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
					(*params)[IPANORAMIC].point,ins,IPANORAMIC,-1,(*p_CsPanoramicStartIndex)[ins],(*p_CsPanoramicEndIndex)[ins]) == OK)
				(*scorearg)[itable] = FunctionTable;
			else
				(*scorearg)[itable] = 0;
			}
		(*scorearg)[iarg] = x;
		}
		
	iarg = (*p_CsPanoramicEndIndex)[ins];
	if(iarg > 0) {
		if((*params)[IPANORAMIC].active && (*params)[IPANORAMIC].mode == CONTINUOUS) {
			if(alpha2 > -0.1 && alpha2 < 1.1) {
				x = GetTableValue(alpha2,imax,(*params)[IPANORAMIC].point,
					startvalue,endvalue);
				if(onoffline == LINE) x = (*scorearg)[iarg] + x - DeftPanoramic;
				x = Remap(x,ins,IPANORAMIC,&overflow);
				if(overflow) goto SORTIR;
				}
			}
		(*scorearg)[iarg] = x;
		}

	if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"@@ ipmax = %d, numberparams = %d\n",(*p_CsInstrument)[ins].ipmax,(*perf)->numberparams);

	if((*p_CsInstrument)[ins].ipmax > 0 && (*perf)->numberparams > 0) {
		if((*perf)->params == NULL) {
			BPPrintMessage(0,odError,"=> Err. CscoreWrite(ZERO,). (*perf)->params == NULL");
			goto WRITECSCORELINE;
			}
		if(instrparamlist == NULL) {
			BPPrintMessage(0,odError,"=> Err. CscoreWrite(ZERO,). instrparamlist == NULL");
			goto WRITECSCORELINE;
			}

	//	for(i = 0; i < (*p_CsInstrument)[ins].ipmax; i++) {
		for(i = 0; i < MaxParameters; i++) {
			// Scan all additional parameters
			paramindex = i + 1 + IPANORAMIC;
			ip = ParamIndexToIp[paramindex];
			paramnameindex = ParamIndexToParamNameIndex(paramindex);

			this_instrument = ParamNameIndexToInstrument[paramnameindex];
			if(InstrumentIndex(this_instrument) != ins) continue;
			iarg = (*instrparamlist)[ip].startindex;
			if(iarg < 0) continue;
			if(trace_cs_scoremake)
				BPPrintMessage(0,odInfo,"&& i = %d, paramindex = %d, ip = %d, ipmax = %d, iarg = %d, nseq = %d, kcurrentinstance = %d, key = %d, ins = %d,  name = “%s”, numberparams = %d, paramnameindex = %d\n",i,paramindex,ip,(*p_CsInstrument)[ins].ipmax,iarg,nseq,kcurrentinstance,(j - 16384),ins,*((*p_StringConstant)[paramnameindex]),(*perf)->numberparams,paramnameindex);
			//	BPPrintMessage(0,odInfo,"&& i = %d, paramindex = %d, ip = %d, ipmax = %d, iarg = %d, nseq = %d, kcurrentinstance = %d, key = %d, ins = %d, numberparams = %d\n",i,paramindex,ip,(*p_CsInstrument)[ins].ipmax,iarg,nseq,kcurrentinstance,(j - 16384),ins,(*perf)->numberparams);
			// BPPrintMessage(0,odInfo,"paramindex = %d, (*perf)->numberparams = %d\n",paramindex,(*perf)->numberparams);
			if(paramindex >= (*perf)->numberparams) continue;
		//	BPPrintMessage(0,odInfo,"@ (*params)[%d].active = %d\n",paramindex,(int)(*params)[paramindex].active);
			if((*params)[paramindex].active) {
				startvalue = (*params)[paramindex].startvalue;
				endvalue = (*params)[paramindex].endvalue;
				imax = (*params)[paramindex].imax;
				if((*params)[paramindex].mode != FIXMAPMODE) {
					alpha1 = (((*scorearg)[2] / ratio) - (*params)[paramindex].starttime)
						/ (*params)[paramindex].dur;
					alpha2 = ((((*scorearg)[2] + (*scorearg)[3]) / ratio)
						- (*params)[paramindex].starttime) / (*params)[paramindex].dur;
					}
				else alpha1 = alpha2 = 0.;
		/*		if(alpha1 <= -0.01 || alpha1 >= 1.01 || alpha2 <= -0.01 || alpha2 >= 1.01 || alpha2 < alpha1)
					alpha1 = alpha2 = -1.; */
				if(alpha1 > -0.1 && alpha1 < 1.1)
					x = GetTableValue(alpha1,imax,(*params)[paramindex].point,startvalue,
						endvalue);
				else x = startvalue;
				if(trace_cs_scoremake) 
					BPPrintMessage(0,odInfo,"startvalue = %.2f, endvalue = %.2f, imax = %ld, mode = %d, alpha1 = %.2f, alpha2 = %.2f, x = %.2f, ins = %d\n",(*params)[paramindex].startvalue,(*params)[paramindex].endvalue,imax,(*params)[paramindex].mode,alpha1,alpha2,x,ins);
				}
			else x = (*instrparamlist)[ip].defaultvalue;
			if(onoffline == LINE) {
				switch((*instrparamlist)[ip].combinationtype) {
					case MULT:
						if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"MULT iarg = %d, x = %.3f, (*scorearg)[iarg] = %.3f, (*instrparamlist)[ip].defaultvalue = %.3f\n",iarg,x,(*scorearg)[iarg],(*instrparamlist)[ip].defaultvalue);
						x = x * (*scorearg)[iarg] / (*instrparamlist)[ip].defaultvalue;
						break;
					case ADD:
						if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"ADD paramindex = %d, iarg = %d, x = %.3f, (*scorearg)[iarg] = %.3f, (*instrparamlist)[ip].defaultvalue = %.3f\n",paramindex,iarg,x,(*scorearg)[iarg],(*instrparamlist)[ip].defaultvalue);
						x = x + (*scorearg)[iarg] - (*instrparamlist)[ip].defaultvalue;
						break;
					default:
						break;
					}
				}
			if((*params)[paramindex].active && imax > ZERO
					&& (*params)[paramindex].mode == CONTINUOUS
					&& (itable=(*instrparamlist)[ip].table) > -1) {
			//	BPPrintMessage(0,odInfo,"@@@ Calling MakeCsoundFunctionTable() for paramindex = %d, ip =  %d\n",paramindex,ip);
				if(MakeCsoundFunctionTable(onoffline,scorearg,alpha1,alpha2,imax,
						(*params)[paramindex].point,ins,paramindex,ip,(*instrparamlist)[ip].startindex,(*instrparamlist)[ip].endindex) == OK)
					(*scorearg)[itable] = FunctionTable;
				else
					(*scorearg)[itable] = 0;
				}
			(*scorearg)[iarg] = x;
			
			iarg = (*instrparamlist)[ip].endindex;
			if(iarg < 0) continue;

			if((*params)[paramindex].active) {
				if(alpha2 > -0.1 && alpha2 < 1.1)
					x = GetTableValue(alpha2,imax,(*params)[paramindex].point,startvalue,
						endvalue);
				else x = endvalue;
				if(onoffline == LINE) {
					switch((*instrparamlist)[ip].combinationtype) {
						case MULT:
							if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"MULT iarg = %ld, x = %.3f, (*scorearg)[iarg] = %.3f, (*instrparamlist)[ip].defaultvalue = %.3f\n",(long)iarg,x,(*scorearg)[iarg],(*instrparamlist)[ip].defaultvalue);
							x = x * (*scorearg)[iarg] / (*instrparamlist)[ip].defaultvalue;
							break;
						case ADD:
							if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"ADD paramindex = %d, iarg = %ld, x = %.3f, (*scorearg)[iarg] = %.3f, (*instrparamlist)[ip].defaultvalue = %.3f\n",paramindex,(long)iarg,x,(*scorearg)[iarg],(*instrparamlist)[ip].defaultvalue);
							x = x + (*scorearg)[iarg] - (*instrparamlist)[ip].defaultvalue;
							break;
						default:
							break;
						}
					}
				}
			else x = (*instrparamlist)[ip].defaultvalue;
			(*scorearg)[iarg] = x;
			}
		}

	iarg = (*p_CsAttackVelocityIndex)[ins];
	if(iarg > 0 && onoffline != LINE) (*scorearg)[iarg] = (*perf)->velocity[key];
		
	iarg = (*p_CsReleaseVelocityIndex)[ins];
	if(iarg > 0 && onoffline != LINE) (*scorearg)[iarg] = velocity;

	iarg = (*p_CsDilationRatioIndex)[ins];
	if(iarg > 0) {
		if(onoffline == LINE) (*scorearg)[iarg] = beta;
		else (*scorearg)[iarg] = (*perf)->beta[key];
		}
		
WRITECSCORELINE:

	// First send this note to pianoroll
	if(ShowPianoRoll) {
		timeon = (Milliseconds) 1000 * (*scorearg)[2];
		timeoff = (Milliseconds) 1000 * ((*scorearg)[2] + (*scorearg)[3]);
			
		if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"key = %d, chan = %d, timeon = %ld, timeoff = %ld, minkey = %d maxkey = %d\n",key,chan,(long)timeon,(long)timeoff,minkey,maxkey);
		DrawPianoNote("csound",key,chan,timeon,timeoff,leftoffset,topoffset,hrect,minkey,maxkey,p_graphrect);
	//	BPPrintMessage(1,odInfo," key #%d draw_line(%d,%d) p_r->left = %d\n",key,timeon,timeoff,p_graphrect->left);
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
		int cent_value = ((int) cents) + cents2;
		if(cent_value != 0) {
			if(cent_value > 0) my_sprintf(LineBuff," + %d cents",cent_value);
			if(cent_value < 0) my_sprintf(LineBuff," %d cents",cent_value);
			strcat(line,LineBuff);
			}
		strcat(Message,line);
		}
	else line[0] = '\0';
	if(ConvertMIDItoCsound) Println(wPrototype7,Message);
	else WriteToFile(NO,CsoundFileFormat,line,CsRefNum);
	result = OK;
	if(trace_cs_scoremake) BPPrintMessage(0,odInfo,"line = %s\n",line);
	strcpy(Message,"");
SORTIR:
	MyDisposeHandle((Handle*)&scorearg);
	if(result == OK && onoffline == OFF) {
		MyDisposeHandle((Handle*)&params);	// This had been created while onoffline == ON
		}

	if(result == OK && comeback) {
		comeback = FALSE;
		goto SETON;
		}
	return(result);
	}

int ParamIndexToParamNameIndex(int paramindex) {
	int paramnameindex,maxparam;
	maxparam = (MyGetHandleSize((Handle)p_StringConstant) / sizeof(char**));
	for(paramnameindex = 0; paramnameindex < maxparam; paramnameindex++) {
		if(paramindex == ParamNameIndexToParameter[paramnameindex]) return paramnameindex;
		}
	return(-1);
	}

