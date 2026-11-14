/* MakeSound.c (BP3) */

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

int trace_makesound = 0;
int trace_maps = 0;
int trace_missed_noteoff = 0;

int MakeSound(long *p_kmax,unsigned long imaxstreak,int maxnsequences,long tmin,long tmax,Milliseconds **p_delta) {

	// Performance parameters and continuous controls, for each sequence (nseq) of the polymetric structure
	PerfParameters ****pp_currentparams,**ptrperf;
	ContinuousControl **p_control;
	ParameterStatus **params;
	ParameterSpecs **currentinstancevalues;
	ParameterStream **stream;
	int nseq,maxconc,maxparam,index,iparam,icont,chancont;
	int **p_seqcont[MAXCHAN+1];
	Milliseconds **p_t2cont[MAXCHAN+1];
	char **p_active[MAXCHAN+1];

	// Sound-object instances: k is the instance, j its object, ievent the index of its next event
	int j,k,kcurrentinstance,kfirstinstance,oldk,doneobjects,nextisobject;
	long i,im,ievent;
	int **p_istartperiod,**p_iendperiod,**p_icycle;
	long **p_inext,**p_inext1;
	Milliseconds **p_t1,**p_nextd;
	double **p_periodgap;
	char **p_silence,**p_onoff;
	int time_pattern,simplenote,outtime,send_more,foundfirsteventinperiod,foundlasteventinperiod,foundfirstevent;
	double alpha,beta,date,olddate,preroll,postroll,objectperiod,cyclicafter,time_nextperiod_start,firstcycleduration;
	unsigned long t3,objectstarttime,objectduration;
	long this_date;

	// MIDI events and keys
	int c0,c1,oldc1,c2,ch,chan,key,sequence,trans,strike,strikeagain,onoff,rs,localchan,localvelocity,objectchannel,scale,blockkey;
	char **p_keychannel[MAXCHAN+1];		// Number of NoteOns of each key on each channel
	long **p_last_timeon[MAXCHAN+1];	// Date of the last NoteOn of each key on each channel
	unsigned int currswitchstate[MAXCHAN+1];
	unsigned long delta_timeon;
	long imap,maxmapped;
	MappedKey **p_currmapped;
	float howmuch;

	// Csound
	int instrument,instr_index;
	char **p_keyinstrument[MAXINS+1];	// Number of NoteOns of each key on each instrument
	Milliseconds end_this_event,cut_the_end,startsilence;

	// Time
	Milliseconds torigin,t0,t1,t11,t2,t2obj,t22,date1,computetime,currenttime,this_gap;
	long oldtcurr,max_endtime_event,max_endtime,add_time,expected_end;
	unsigned long oldtime;
	UInt64 drivertime;

	int occurrence,result,overflow,cswrite,showpianoroll;
	char this_note[20];
	Handle h;
	p_list **scriptlist;
	PianoRoll roll;

	memset(&roll,0,sizeof(roll));
	oldtcurr = Tcurr;
	time_pattern = send_more = FALSE;
	scale = -1;
	objectperiod = cyclicafter = 0.;
	if(OutCsound) {
		MIDIsetUpTime = 0;
	//	BPPrintMessage(1,odInfo,"@@@ CsoundSize[18] = %ld\n",(*p_CsoundSize)[18]);
		}

	if(trace_makesound) BPPrintMessage(1,odInfo, "\nRunning MakeSound() (Tcurr * Time_res) = %ld, tmin = %ld ms, tmax = %ld ms\n",(long) Tcurr * Time_res,tmin,tmax);


	// note 1
	if(Panic || CheckEmergency() != OK) return(ABORT);
	if(SoundOn) return(OK);
	if(NoTracePath) {
		ShowObjectGraph = ShowPianoRoll = ShowGraphic = FALSE;
		BPPrintMessage(0,odInfo,"👉 No graphic due to the absence of a specified trace path\n");
		}

	if(CompileRegressions() != OK) return(ABORT);

	showpianoroll = ShowPianoRoll;
	if(ConvertMIDItoCsound || ItemCapture) showpianoroll = FALSE;
	if(FALSE && showpianoroll) BPPrintMessage(0,odInfo,"Showing piano roll\n");

	cswrite = FALSE;
	if(((OutCsound && (FileWriteMode == NOW || !rtMIDI))
		|| ConvertMIDItoCsound) && !ItemCapture) cswrite = TRUE;

	MIDIfileOn = FALSE;
	if(WriteMIDIfile && (FileWriteMode == NOW || !rtMIDI) && !ItemCapture)
		MIDIfileOn = TRUE;

	if(trace_makesound) BPPrintMessage(0,odInfo,"cswrite = %d, OutCsound = %d, WriteMIDIfile = %d, MIDIfileOn = %d\n",cswrite,OutCsound,WriteMIDIfile,MIDIfileOn);

	if(!cswrite && !rtMIDI && !MIDIfileOn && !EventListOn && !ShowGraphic && !showpianoroll) {
	//	BPPrintMessage(0,odInfo, "=> Cancelling MakeSound()\n");
		return(OK);
		}
	if(WriteMIDIfile || OutCsound || EventListOn) {
		ItemNumber++;
	//  BPPrintMessage(1,odInfo,"@@ 👉 %ld !!!\n",(long)ItemNumber);
		if((MaxItemsProduce > 0) && ItemNumber > MaxItemsProduce) {
			if(ItemNumber == (MaxItemsProduce + 1L)) BPPrintMessage(1,odInfo,"👉 %ld items have been produced\n",(long)(ItemNumber - 1L));
			return(OK);
			}
		}
	overflow = FALSE;
	rs = 0;
	localchan = 0;
	// max_endtime_event = max_endtime = ZERO;

	if(MIDIfileOn) {
		BPPrintMessage(0,odInfo,"👉 A MIDI file has been created\n");
		}

	// Nplay = 1; // Used to try repeating real-time output or Csound score

	Ke = log((double) SpeedRange) / 64.;
	t0 = ZERO;
	if(*p_kmax >= MaxObjects) {
	//	BPPrintMessage(0,odError,"kmax >= MaxObjects. Err. MakeSound()");
		BPPrintMessage(0,odError,"=> kmax >= MaxObjects. Err. MakeSound()\n");
		return(ABORT);
		}

	maxconc = maxnsequences;
	if(Beta && maxconc > Maxconc) {
	//	BPPrintMessage(0,odError,"maxconc > Maxconc. Err. MakeSound()");
		BPPrintMessage(0,odError,"=> maxconc > Maxconc. Err. MakeSound()\n");
		}

	// note 2
	if((p_control=(ContinuousControl**)GiveSpace(maxconc*sizeof(ContinuousControl))) == NULL)
		return(ABORT);
		
	if((pp_currentparams=(PerfParameters****)GiveSpace(maxconc*sizeof(PerfParameters**)))
		== NULL) return(ABORT);

	if((p_inext = (long**) GiveSpace((Size)MaxObjects*sizeof(long))) == NULL) return(ABORT);
	if((p_inext1 = (long**) GiveSpace((Size)MaxObjects*sizeof(long))) == NULL) return(ABORT);
	if((p_istartperiod = (int**) GiveSpace((Size)MaxObjects*sizeof(int))) == NULL) return(ABORT);
	if((p_iendperiod = (int**) GiveSpace((Size)MaxObjects*sizeof(int))) == NULL) return(ABORT);
	if((p_icycle = (int**) GiveSpace((Size)MaxObjects*sizeof(int))) == NULL) return(ABORT);
	if((p_periodgap = (double**) GiveSpace((Size)MaxObjects*sizeof(double))) == NULL) return(ABORT);
	if((p_nextd = (Milliseconds**) GiveSpace((Size)MaxObjects*sizeof(Milliseconds))) == NULL) return(ABORT);
	if((p_t1 = (Milliseconds**) GiveSpace((Size)MaxObjects*sizeof(Milliseconds))) == NULL) return(ABORT);
	if((p_onoff = (char**) GiveSpace((Size)MaxObjects*sizeof(char))) == NULL) return(ABORT);
	if((p_silence = (char**) GiveSpace((Size)MaxObjects*sizeof(char))) == NULL) return(ABORT);

	maxmapped = 32L;
	p_currmapped = (MappedKey**)GiveSpace((Size)maxmapped*sizeof(MappedKey));
	if(p_currmapped == NULL) return(ABORT);
	for(imap=0; imap < maxmapped; imap++) (*p_currmapped)[imap].orgkey = -1;

	maxparam = IPANORAMIC + 1;
	// BPPrintMessage(0,odInfo,"In MakeSound.c maxparam = %ld\n",(long)maxparam);

	if(cswrite) {
		for(instr_index = 0; instr_index < Jinstr; instr_index++) {
			if((p_keyinstrument[instr_index] = (char**) GiveSpace((Size)(MAXKEY+1)*sizeof(char))) == NULL) return(ABORT);
			for(key = 0; key < MAXKEY; key++)
				(*p_keyinstrument[instr_index])[key] = 0;
			}
		}

	for(ch = 0; ch < MAXCHAN; ch++) {
		if((p_keychannel[ch] = (char**) GiveSpace((Size)(MAXKEY+1)*sizeof(char)))
				== NULL) return(ABORT);
		if((p_last_timeon[ch] = (long**) GiveSpace((Size)(MAXKEY+1)*sizeof(long)))
				== NULL) return(ABORT);
		for(key = 0; key < MAXKEY; key++) {
			(*p_keychannel[ch])[key] = 0;
			(*p_last_timeon[ch])[key] = 0L;
			}
		if((p_seqcont[ch] = (int**) GiveSpace((Size)(maxparam)*sizeof(int)))
				== NULL) return(ABORT);
		if((p_t2cont[ch] = (Milliseconds**) GiveSpace((Size)(maxparam)*sizeof(Milliseconds)))
				== NULL) return(ABORT);
		if((p_active[ch] = (char**) GiveSpace((Size)(maxparam)*sizeof(char)))
				== NULL) return(ABORT);
		}

	for(nseq = 0; nseq < maxconc; nseq++) {
		if((ptrperf=(PerfParameters**) GiveSpace(sizeof(PerfParameters)))
				== NULL) return(ABORT);
		(*pp_currentparams)[nseq] = ptrperf;
		
		(*ptrperf)->numberparams = maxparam;
		if((params=(ParameterStatus**)GiveSpace(maxparam * sizeof(ParameterStatus))) == NULL)
			return(ABORT);
		(*ptrperf)->params = params;
		
		(*p_control)[nseq].number = maxparam;
		if((stream=(ParameterStream**)GiveSpace(maxparam*sizeof(ParameterStream))) == NULL)
			return(ABORT);
		(*p_control)[nseq].param = stream;
		}

	result = OK;
	kfirstinstance = 0;
	howmuch = 0.;
	t11 = t22 = Infpos;
	// note 3
	for(k = 2; k <= (*p_kmax); k++) {
		(*p_inext1)[k] = (*p_inext)[k] = ZERO;
		time_pattern = send_more = FALSE;
		preroll = postroll = 0.;
		(*p_inext1)[k] = 0;
		objectperiod = 0.;
		j = (*p_Instance)[k].object;
		if(j == 0) continue;
		if(trace_makesound)	BPPrintMessage(0,odInfo,"Jbol = %ld, k = %ld -> j = %ld\n",(long)Jbol,(long)k,(long)j);
		if(j >= Jbol && j < 16384) { // Time pattern;
			time_pattern = TRUE;
			(*p_onoff)[k] = FALSE;
			(*p_silence)[k] = FALSE;
			(*p_istartperiod)[k] = (*p_iendperiod)[k] = -1;
			}
		if(j < 0) j = -j;
		alpha = (*p_Instance)[k].alpha;
		beta = (*p_Instance)[k].beta;	// alpha != beta if the sound-object is cyclic
		if(cswrite && trace_makesound && j < Jbol) {
			BPPrintMessage(0,odInfo,"j = %d\n",j);
			im = Find_im(cswrite,j);
			this_date = ZERO;
			for(ievent = 0; ievent < im; ievent++) {
				this_date += (*((*pp_CsoundTime)[j]))[ievent];
				BPPrintMessage(0,odInfo,"ievent = %ld / %ld --> %ld (%ld)\n",ievent,im,(*((*pp_CsoundTime)[j]))[ievent],this_date);
				}
			}
		(*p_onoff)[k] = FALSE;
		(*p_istartperiod)[k] = (*p_iendperiod)[k] = -1;
		if(j < Jbol) preroll = ObjectPreRoll(j,beta);
		if(((*p_Instance)[k].starttime - (*p_Instance)[k].truncbeg) < t11) {
			kfirstinstance = k;
			t11 = (*p_Instance)[k].starttime - (*p_Instance)[k].truncbeg;
			if(preroll < ZERO) t11 -= preroll;
			}
		foundfirsteventinperiod = foundlasteventinperiod = TRUE;
		if(j < 16384) {
			if(time_pattern) { // Time pattern
				im = ZERO;
				beta = 1;
				(*p_CsoundSize)[j] = (*p_MIDIsize)[j] = 0;
				}
			else {
				if((*p_Instance)[k].ncycles < 2 && beta != alpha && j > 1 && !(*p_FixScale)[j]) {
					beta = (*p_Instance)[k].beta = alpha;
					}
				im = Find_im(cswrite,j);
				if(trace_makesound) BPPrintMessage(1,odInfo,"im = %ld\n",im);
				firstcycleduration = beta * (*p_Dur)[j];
				(*p_iendperiod)[k] = im - 1;
				if(alpha > 1.) {
					if(GetPeriod(j,beta,&objectperiod,&cyclicafter) == OK) {
						if(trace_makesound) BPPrintMessage(0,odInfo,"@@ objectperiod = %.2f, cyclicafter = %.2f, beta = %.2f, alpha = %.2f\n",objectperiod,cyclicafter,beta,alpha);
						foundfirsteventinperiod = foundlasteventinperiod = FALSE;
						}
					}
				}
			}
		(*p_t1)[k] = ZERO;
	//	if(preroll < ZERO)
	//		(*p_t1)[k] -= preroll;
		date = (*p_t1)[k];
		date1 = date;
		// date1 = (*p_Instance)[k].truncbeg;
		if(trace_makesound) 
			BPPrintMessage(0,odInfo,"§§§ t1[%d] = %ld, truncbeg = %ld, date = %.2f, date1 = %ld\n",k,(*p_t1)[k],(*p_Instance)[k].truncbeg,date,date1);
		foundfirstevent = FALSE;
		if(j > 1 && j < 16384) {	// Sound-object or time pattern
			// Look for first event in object and for first and last events in its periodical part
			if(j >= Jbol && Jbol < 2) {
				my_sprintf(Message,"=> Err. MakeSound(). j >= Jbol && Jbol < 2\n");
				BPPrintMessage(0,odInfo,Message);
				}
			// note 4
			if(objectperiod > 0.) {
				time_nextperiod_start = (*p_Instance)[k].starttime - (*p_Instance)[k].truncbeg + cyclicafter + objectperiod;
				if(preroll < ZERO) time_nextperiod_start -= preroll;
				if(trace_makesound) 
					BPPrintMessage(0,odInfo,">time_nextperiod_start = %.2f ms for k = %d, starttime = %ld, cyclicafter = %.2f, objectperiod = %.2f, preroll = %.2f, postroll = %.2f\n",time_nextperiod_start,k,(*p_Instance)[k].starttime,cyclicafter,objectperiod,preroll,postroll);
				}
			for(i = 0; i < im; i++) {
				olddate = date;
				if(PlayCsoundLine(cswrite,j) || (*p_MIDIsize)[j] > 0)
					date += (Milliseconds) (beta * EventDelay(cswrite,j,i));
				else BPPrintMessage(0,odError,"=> Error MakeSound() MIDIsize = 0\n");
				if(trace_makesound) 
					BPPrintMessage(0,odInfo,"olddate = %ld, date = %.2f, date1 = %ld, i = %ld, im = %ld, foundfirstevent = %d\n",(long)olddate,date,date1,i,im,foundfirstevent);
				if(!foundfirstevent && date >= date1) {
					(*p_inext1)[k] = i;	// Index of first message to be sent
					(*p_t1)[k] = date - date1; // Probably useless
					foundfirstevent = TRUE;
					if(trace_makesound) BPPrintMessage(0,odInfo,"break: t1 = %ld, i = %ld\n",(*p_t1)[k],i);
					if(foundlasteventinperiod) break;
					}
				if(!foundfirsteventinperiod && date >= cyclicafter) {
					(*p_istartperiod)[k] = i;
					(*p_periodgap)[k] = date - cyclicafter;
					if(trace_makesound)
						BPPrintMessage(0,odInfo,"•• k = %d, i = %d, im = %ld, date = %.4f, cyclicafter = %.4f, istartperiod = %d, periodgap = %.4f\n",k,i,im,date,cyclicafter,(*p_istartperiod)[k],(*p_periodgap)[k]);
					foundfirsteventinperiod = TRUE;
					}
				if(!foundlasteventinperiod && !foundfirsteventinperiod && ((i == (im - 1)) || date > firstcycleduration)) { // Fixed 2026-08-05
					// We checked 'firstcycleduration' in case postroll was negative
					if(trace_makesound) BPPrintMessage(0,odInfo,"j = %d, CsoundSize = %ld\n",j,(*p_CsoundSize)[j]);
					// Include the following events that are simultaneous
					if(i < (im-1) && EventDelay(cswrite,j,i+1) == ZERO) {
						i++;
						while(i < (im-1) && EventDelay(cswrite,j,i) == ZERO) i++;
						if(i < (im-1)) i--;
						}
					(*p_iendperiod)[k] = i;
					(*p_periodgap)[k] += (firstcycleduration - olddate);
					if(trace_makesound)
						BPPrintMessage(0,odInfo,"••+ firstcycleduration = %.4f, date = %.4f, olddate = %.4f, periodgap = %.4f\n",firstcycleduration,date,olddate,(*p_periodgap)[k]);
			//		foundlasteventinperiod = TRUE;
					break;
					}
				}
			}
		if((k != kfirstinstance) && (((*p_Instance)[k].starttime - (*p_Instance)[k].truncbeg) < t22)) {
			t22 = (*p_Instance)[k].starttime - (*p_Instance)[k].truncbeg;
			if(preroll < 0.) t22 -= preroll;
			}
		}
	SoundOn = TRUE;

	// note 5
	// Prepare PIANOROLL, computing limits in time and pitch

	if(showpianoroll) {
		result = PreparePianoroll(*p_kmax,cswrite,&tmin,&tmax,imaxstreak,p_delta,&roll,&overflow);
		if(result != OK || overflow) goto OUTGRAPHIC;
		}

	// note 6
	if(!MIDIfileOn && !cswrite && !EventListOn && rtMIDI && !showpianoroll)
		drivertime = getClockTime();

	if(Improvize && !Interrupted && !FirstTime && !ItemCapture && !showpianoroll) {
		computetime = drivertime - ComputeStart;
		if(computetime > MaxComputeTime) MaxComputeTime = computetime;
		}
	else computetime = ZERO;
	if(cswrite || MIDIfileOn || EventListOn || ItemCapture) {
		BPPrintMessage(0,odInfo,"Writing %ld sound-objects\n",((*p_kmax)-2));
		}
	if(!cswrite && rtMIDI && A4freq != 440) TuneMIDIdevice(&rs);
	if(FALSE && !cswrite && MIDImicrotonality && rtMIDI) SetUpMPEreceiver(&rs);

	if(cswrite) {
		if(Jinstr == 1 && (*p_CsInstrumentIndex)[0] == -1) {
			BPPrintMessage(0,odInfo,"\nCouldn't find a Csound instrument index. Value '1' will be assigned by default\n\n");
			}
		if(!ConvertMIDItoCsound) {
			if((result=PrepareCsFile()) != OK) goto OVER;
			}
		Nplay = 1; FunctionTable = 100;
		}

	if(MIDIfileOn) {
		if(Create_set) {
			MakeMIDIFile(&(gOptions.outputFiles[ofiMidiFile]));
			}
		if((result=PrepareMIDIFile()) != OK) goto OVER;
		Nplay = 1;
		}

	if(A4freq != 440 && TraceMicrotonality) 
		BPPrintMessage(0,odInfo,"👉 The diapason (A4) is set to %.2f Hz. This multiplies the frequencies by %.2f\n",A4freq, (A4freq / 440.));

	rs = 0;

	// note 7
	for(nseq = 0; nseq < maxconc; nseq++) {
		ptrperf = (*pp_currentparams)[nseq];
		for(key = 0; key < MAXKEY; key++) {
			(*ptrperf)->level[key] = 0;
			(*ptrperf)->startparams[key] = NULL;
			}
		(*ptrperf)->transpose = 0;
		(*ptrperf)->paramsinstance = 0;

		maxparam = (*ptrperf)->numberparams;
	//	BPPrintMessage(0,odInfo,"\nMakeSound() nseq = %d number = %d maxparam = %d\n",nseq,(*p_control)[nseq].number,maxparam);
	//	BPPrintMessage(0,odInfo,"active = FALSE (3) with maxparam = %d\n",maxparam);
		for(i=0; i < maxparam; i++) (*((*ptrperf)->params))[i].active = FALSE;
		if(maxparam <= IPANORAMIC) {
			BPPrintMessage(0,odInfo,"=> Err. MakeSound(). maxparam (%d) <= IPANORAMIC\n",maxparam);
			goto OVER;
			}
		for(i = 0; i < (*p_control)[nseq].number; i++) {
			(*((*p_control)[nseq].param))[i].ibm = -1L;
			}
		}

	// ===========================
	// The following loop repeats Nplay performances of item
	// ===========================

	// note 8
	for(occurrence = 0; occurrence < Nplay || SynchroSignal == PLAYFOREVER; occurrence++) {
		result = OK;
		if((result=stop(1,"MakeSound")) != OK) goto OVER;
		if(SkipFlag) goto OVER;
		// note 9
		for(k = 2; k <= (*p_kmax); k++) {
			(*p_inext)[k] = (*p_inext1)[k];
			j = (*p_Instance)[k].object;
			this_gap = ZERO;
			preroll = postroll = 0.;
			beta = (*p_Instance)[k].beta;
			if(j > 0 && j < Jbol) {
				preroll = ObjectPreRoll(j,beta);
				postroll = ObjectPostRoll(j,beta);
				if((*p_Instance)[k].truncbeg > EPSILON) {
					if(trace_makesound) BPPrintMessage(1,odInfo,"Beginning truncated, k = %d, truncbeg = %ld, preroll = %.2f\n",k,(*p_Instance)[k].truncbeg, preroll);
					im = Find_im(cswrite,j);
					ievent = FirstEventAfterTruncation(k,j,cswrite,preroll,&this_gap);
					if(ievent < im) (*p_inext)[k] = ievent;
					}
				}
			(*p_onoff)[k] = FALSE;
			(*p_nextd)[k] = (*p_Instance)[k].starttime + this_gap;
			if(trace_makesound)
				BPPrintMessage(1,odInfo,"@k = %ld, starttime = %ld, endtime = %ld, this_gap = %ld,  t1 = %ld, nextd = %ld, inext[%d] = %d\n",(long)k,(long)(*p_Instance)[k].starttime,(long)(*p_Instance)[k].endtime,this_gap,(long)(*p_t1)[k],(long)(*p_nextd)[k],k,(*p_inext)[k]);
			}
		icont = -1; chancont = -1;
		for(ch = 0; ch < MAXCHAN; ch++) {
			currswitchstate[ch] = 0;
			for(index=0; index <= IPANORAMIC; index++) {
				(*(p_t2cont[ch]))[index] = Infpos;
				(*(p_active[ch]))[index] = FALSE;
				}
			}
		LimTimeSet = LimCompute = FALSE;
		TimeMax = MAXTIME;
		t1 = t11; t2 = t2obj = t22;
		kcurrentinstance = kfirstinstance;
		instrument = -1;
		result = OK;
		if(trace_makesound) BPPrintMessage(1,odInfo,"@@@ Starting t1 = %ld, t11 = %ld, t2 = %ld, t22 = %ld\n",t1,t11,t2,t22);
		
		if(Improvize && (Nplay > 1 || SynchroSignal == PLAYFOREVER)) {
			my_sprintf(Message,"%ldth repetition...",(long)occurrence+1L);
			ShowMessage(TRUE,wMessage,Message);
			}
		
		for(ch = 0; ch < MAXCHAN; ch++) {
			for(key = 0; key < MAXKEY; key++) {
				(*p_keychannel[ch])[key] = 0;
				(*p_last_timeon[ch])[key] = 0L;
				}
			}

		if(cswrite) {
			for(instr_index = 0; instr_index < Jinstr; instr_index++) {
				for(key = 0; key < MAXKEY; key++)
					(*p_keyinstrument[instr_index])[key] = 0;
				}
			}
		
		// note 10
		if((MIDIfileOn || cswrite || showpianoroll || rtMIDI || EventListOn
				|| ItemNumber == ZERO || PlaySelectionOn || ItemCapture) && !CyclicPlay) {
			if(!cswrite && !ItemCapture) {
				Tcurr += (MIDIsetUpTime) / Time_res;
				}
			currenttime = Tcurr * Time_res;
			Nbytes = 0; Tbytes2 = ZERO;
			}
		else {
			drivertime = getClockTime();
			if(drivertime > Tcurr) {
				// Too late to start on time!
				currenttime = drivertime * Time_res;
				Nbytes = 0; Tbytes2 = ZERO;
				}
			else currenttime = Tcurr * Time_res;
			}

		// BPPrintMessage(1,odInfo,"@@@ Tcurr = %ld, currenttime = %ld, drivertime = %ld, LastTime = %ld\n",Tcurr,currenttime,drivertime,LastTime);

		if(occurrence == 0 && Chunk_number > 1 && !cswrite && !Improvize && !PlayAllChunks && !AllItems && ChunkEndTime > ZERO) {
			// 2026-11-11 The chunks of an item played line by line start at the end of the previous chunk.
			// They restarted at MIDIsetUpTime when the pianoroll was displayed, because Tcurr is restored at the end of MakeSound():
			// real-time MIDI sent the next chunk at once, and MIDI files squeezed it. Without the pianoroll, the next chunk started
			// at the date of the last event of the previous one plus MIDIsetUpTime - fixed by Claude
			currenttime = ChunkEndTime;
			}

		if(t1 < ZERO) t0 = currenttime - t1;
		else t0 = currenttime;
		
		torigin = t0 + t1;
		
		roll.noteoffset = roll.leftoffset;
		if((Improvize || PlayAllChunks || AllItems) && (LastTime > ZERO)) {
			PianorollShift = LastTime;
		//	BPPrintMessage(1,odInfo,"(Tcurr * Time_res) = %ld ms, LastTime = %ld ms, currenttime = %ld ms\n",(long)Tcurr * Time_res,(long)LastTime,(long)currenttime);
			Tcurr = LastTime / Time_res; // 2024-05-02
			t0 = Tcurr * Time_res;
		//	BPPrintMessage(1,odInfo,"PianorollShift(1) = %ld, LastTime = %ld\n",(long)PianorollShift,(long)LastTime);
			}
		else {
			Tcurr = (t0 + t1) / Time_res;
			if(MIDIfileOn || rtMIDI || EventListOn) {
				PianorollShift = MIDIsetUpTime;
			//	BPPrintMessage(1,odInfo,"PianorollShift(2) = %ld, LastTime = %ld\n",(long)PianorollShift,(long)LastTime);
				}
			if(showpianoroll && t1 < ZERO)
				// All dates have been shifted by -t1 to avoid negative dates (pivot or pre-roll). The pianoroll's
				// 'leftoffset' already places date 'tmin' at the left margin, so notes must be shifted back.
				roll.noteoffset = roll.leftoffset + (t1 * GraphicScaleP) / GraphicScaleQ / 10;
			}
		
		if(occurrence == 0 && !cswrite) {
			// End of this chunk (dates start at t0)
			if(showpianoroll) ChunkEndTime = t0 + tmax; // 'tmax' already includes the Nplay repetitions
			else ChunkEndTime = t0 + tmax + (Nplay - 1) * (tmax - tmin);
			}
		if(rtMIDI && occurrence == 0) {
			// Expected date of the end of this item on the clock displayed by produce.php (see WriteTimeFile())
			// Several items (Improvize, all items, chunks): this is the end of the current item
			if(SynchroSignal == PLAYFOREVER) ExpectedEndTime = -1L;
			else if(ExpectedEndTime >= 0L) {
				expected_end = ChunkEndTime - MIDIsetUpTime;
				if(expected_end > ExpectedEndTime) ExpectedEndTime = expected_end;
				}
			}
		if(trace_makesound)
			BPPrintMessage(0,odInfo,"Tcurr = %ld, occurrence = %d, t0 = %ld (ms), t1 = %ld (ms), Time_res = %ld\n\n",(long)Tcurr,occurrence,(long)t0,(long)t1,(long)Time_res);
			
		if(cswrite && (result=WriteCsoundTempo(torigin)) != OK) goto OVER;
		
		ItemOutPutOn = TRUE;
		doneobjects = 0; // Probably useless
		time_nextperiod_start = -1.;
		(*p_silence)[kcurrentinstance] = FALSE;
		
		// =========================== //

		// note 11
		while(kcurrentinstance > 0) {
			if((result=stop(1,"MakeSound")) != OK) goto OVER;
			// note 12
			currentinstancevalues = (*p_Instance)[kcurrentinstance].contparameters.values;
			scale = (*p_Instance)[kcurrentinstance].scale;
			// BPPrintMessage(0,odInfo,"§§§ scale = %d\n",scale);
			blockkey = (*p_Instance)[kcurrentinstance].blockkey;
			j = (*p_Instance)[kcurrentinstance].object;
			if(j == 0) {
				BPPrintMessage(0,odError,"=> Error MakeSound(). j = 0\n");
				result = ABORT; goto OVER;
				}
			outtime = FALSE;
			if(j < 0) {
				j = -j; outtime = TRUE;
				}
			if((objectchannel = ChannelConvert((*p_Instance)[kcurrentinstance].channel)) < -1) {
				BPPrintMessage(0,odError,"=> Error MakeSound(), Instance[%d].channel = %d,objectchannel = %d\n",kcurrentinstance,(*p_Instance)[kcurrentinstance].channel,objectchannel);
				objectchannel = 0;
				}
		//	BPPrintMessage(0,odInfo,"§§§ imax[5] = %d\n",(*currentinstancevalues)[5].imax);
			if(trace_makesound) {
				BPPrintMessage(0,odInfo,"@@@ Instance[%d].channel = %d --> objectchannel = %d, kcurrentinstance = %d\n",kcurrentinstance,(*p_Instance)[kcurrentinstance].channel,objectchannel,kcurrentinstance);
				if(j < Jbol) BPPrintMessage(0,odInfo,"DefaultChannel[%d] = %d\n",j,(*p_DefaultChannel)[j]);
				}
			sequence = 0;  // Normally not required
			instrument = ThisInstrument(cswrite,kcurrentinstance,ievent,&instr_index);
		/*	if(trace_makesound) 
				BPPrintMessage(0,odInfo,"kcurrentinstance = %d, channel = %d instrument = %d scale = %d\n",kcurrentinstance,(*p_Instance)[kcurrentinstance].channel,(*p_Instance)[kcurrentinstance].instrument,scale); */
			nseq = (*p_Instance)[kcurrentinstance].nseq;
			if(nseq < 0 || nseq >= maxconc) {
				BPPrintMessage(0,odError,"=> Err. MakeSound(). nseq < 0 || nseq >= maxconc");
				nseq = 0;
				}
			params = (*((*pp_currentparams)[nseq]))->params;
			time_pattern = send_more = FALSE;
			alpha = (*p_Instance)[kcurrentinstance].alpha;
			beta = (*p_Instance)[kcurrentinstance].beta;
			preroll = postroll = 0.;

			// note 13
			if(j >= 16384) im = 6;
			// Simple note contains 6 MIDI bytes (NoteOn/NoteOff)
			if(j < 16384) {
				if(j >= Jbol && Jbol < 2) {
					BPPrintMessage(0,odError,"=> Err. MakeSound(). j >= Jbol && Jbol < 2");
					result = ABORT; goto OVER; // Fixed by BB 2021-02-26
					} // BPPrintMessage(0,odError,"=> Err. MakeSound(). j >= Jbol && Jbol < 2");
				if(j < Jbol) {
					if(trace_makesound) 
						BPPrintMessage(0,odInfo,"BEGIN kcurrentinstance = %d, j = %d, objectchannel = %d, Csound instrument mode = %d, silence = %d\n",kcurrentinstance,j,objectchannel,(*p_CsoundInstrumentMode)[j],(*p_silence)[kcurrentinstance]);
					strikeagain = StrikeAgainOf(j);
					im = Find_im(cswrite,j);
					if((*p_Instance)[kcurrentinstance].ncycles < 2 && beta != alpha && j > 1 && !(*p_FixScale)[j]) {
						beta = (*p_Instance)[kcurrentinstance].beta = alpha;
						}
					firstcycleduration = beta * (*p_Dur)[j];
					(*p_iendperiod)[kcurrentinstance] = im - 1;
					preroll = ObjectPreRoll(j,beta);
					postroll = ObjectPostRoll(j,beta);
					if(alpha > 1.) {
						if(GetPeriod(j,beta,&objectperiod,&cyclicafter) == OK) {
							if(trace_makesound) BPPrintMessage(0,odInfo,"@@@ objectperiod = %.2f, cyclicafter = %.2f, beta = %.2f, alpha = %.2f\n",objectperiod,cyclicafter,beta,alpha);
							foundfirsteventinperiod = foundlasteventinperiod = FALSE;
							}
						}
					if(objectperiod > 0. && time_nextperiod_start < 0) {
						time_nextperiod_start = (*p_Instance)[kcurrentinstance].starttime - (*p_Instance)[kcurrentinstance].truncbeg + cyclicafter + objectperiod;
						if(preroll < ZERO) time_nextperiod_start -= preroll;
						if(trace_makesound) 
							BPPrintMessage(0,odInfo,"@ time_nextperiod_start = %.2f ms for kcurrentinstance = %d\n",time_nextperiod_start,kcurrentinstance);
						}
					}
				else {
					strikeagain = StrikeAgainSettings;
					// This is a time pattern!
					}
				}
			else strikeagain = StrikeAgainSettings;
			if(trace_makesound) BPPrintMessage(0,odInfo,"strikeagain = %d \n",strikeagain);
			localvelocity = ByteToInt((*p_Instance)[kcurrentinstance].velocity);
			trans = (*((*pp_currentparams)[nseq]))->transpose
				= (*p_Instance)[kcurrentinstance].transposition;
			(*((*pp_currentparams)[nseq]))->xpandkey = (*p_Instance)[kcurrentinstance].xpandkey;
			(*((*pp_currentparams)[nseq]))->xpandval = (*p_Instance)[kcurrentinstance].xpandval;
			(*((*pp_currentparams)[nseq]))->transposefirst = (*p_Instance)[kcurrentinstance].transposefirst;
			// note 14
			t3 = (*p_Instance)[kcurrentinstance].endtime;
			if(j > 1 && j < Jbol) {
				if((*p_PostRoll)[j] > ZERO) t3 -= (*p_PostRoll)[j];
				}
			objectstarttime = (*p_Instance)[kcurrentinstance].starttime - (*p_Instance)[kcurrentinstance].truncbeg;
			if(preroll < ZERO) objectstarttime -= preroll;
			objectduration = t3 - objectstarttime;
			if(trace_makesound) BPPrintMessage(0,odError,"@kcurrentinstance = %d, objectstarttime = %ld, endtime = %ld, trucbeg = %ld\n",kcurrentinstance,objectstarttime,(*p_Instance)[kcurrentinstance].endtime,(*p_Instance)[kcurrentinstance].truncbeg);
			if(objectduration < 0 || objectduration > 500000) { // Revised 2024-06-21
				BPPrintMessage(0,odError,"=> Incorrect object duration for object #%d (j = %d) in chunk #%d, starting %ld ms ending %ld ms\n",kcurrentinstance,(*p_Instance)[kcurrentinstance].object,Chunk_number,objectstarttime,(long)t3);
			//	BPPrintMessage(0,odError,"=> startime = %ld, endtime = %ld\n",objectstarttime,(*p_Instance)[kcurrentinstance].endtime);
				t3 = (*p_Instance)[kcurrentinstance].endtime = objectstarttime;
				objectduration = ZERO;
				}
			// note 15
			// 2026-11-10: each Csound event follows the variations of parameters (volume, pitchbend, etc.) of its own object.
			// Parameters are stored per sequence, so an object starting later on the same sequence replaced them.
			// In "_volumecont _volume(30) a C4 ... _volume(127)", if 'a' lasts longer than its time slot, its events
			// after the start of 'C4' took the variation of 'C4'. They are now restored when the object resumes - fixed by Claude
			if(cswrite && (*p_onoff)[kcurrentinstance] && kcurrentinstance > 1
					&& (*((*pp_currentparams)[nseq]))->paramsinstance > 1
					&& (*((*pp_currentparams)[nseq]))->paramsinstance != kcurrentinstance
					&& (*p_Instance)[kcurrentinstance].contparameters.number > 0
					&& currentinstancevalues != NULL && params != NULL) {
				int numberparams = (*((*pp_currentparams)[nseq]))->numberparams;
				if((*p_Instance)[kcurrentinstance].contparameters.number < numberparams)
					numberparams = (*p_Instance)[kcurrentinstance].contparameters.number;
				SetObjectParameters(params,currentinstancevalues,numberparams,kcurrentinstance,j,t0,objectduration);
				(*((*pp_currentparams)[nseq]))->paramsinstance = kcurrentinstance;
				}

			ievent = (*p_inext)[kcurrentinstance];
			beta = (*p_Instance)[kcurrentinstance].beta;

			if(trace_makesound) BPPrintMessage(0,odInfo,"@@kcurrentinstance = %d, ievent = %d, objectstarttime = %ld endtime = %ld  objectduration = %ld\n",kcurrentinstance,ievent,objectstarttime,(long)t3,(long)objectduration);
			
			// Initialise sound-object instance kcurrentinstance

			// note 16
			if(!(*p_onoff)[kcurrentinstance]) {
				if(EventListOn) AddEventToList(kcurrentinstance);
			//	if(trace_makesound) BPPrintMessage(0,odInfo,"kcurrentinstance2 = %d\n",kcurrentinstance);
				howmuch = 0.;
				result = OK;
				(*p_onoff)[kcurrentinstance] = TRUE;
				(*p_icycle)[kcurrentinstance] = 1;
				time_nextperiod_start = -1.;
				(*p_silence)[kcurrentinstance] = FALSE;
		/*		if(objectchannel > 0) chan = objectchannel - 1;
				else chan = 0;
				if(chan < 0) chan = 0;
				if(cswrite) chan = 0; */
				c0 = 0;
				chan = MIDIchannel(cswrite,j,objectchannel,&c0);
				Tcurr = (t0 + t1) / Time_res;
				if(!outtime) {
					if((result=SendInitialControls(kcurrentinstance,j,chan,nseq,maxconc,p_control,p_active,cswrite,(t0 + t1),&rs)) != OK) goto OVER;
					}
				
				// Recalculate t2
				
				if(trace_makesound) BPPrintMessage(0,odInfo,"\nRecalculate t2\n");
				
				NextInstance(*p_kmax,p_nextd,NULL,0,&t2obj);
				t2 = t2obj;
				NextControlDate(p_active,p_t2cont,&t2,&icont,&chancont);
				if(trace_makesound)
					BPPrintMessage(0,odInfo,"Fixed t2 = %ld\n",(long)t2);

				result = OK;
				
				// note 17
				// Pass on parameters to pp_currentparams
			//	BPPrintMessage(0,odInfo,"k = %ld, contparameters.number = %d\n",(long)kcurrentinstance, (*p_Instance)[kcurrentinstance].contparameters.number);

				params = (*((*pp_currentparams)[nseq]))->params; 

				if((kcurrentinstance > 1) && (*p_Instance)[kcurrentinstance].contparameters.number > 0) {
					if(trace_makesound)
						BPPrintMessage(0,odInfo,"k = %ld > 1, contparameters.number = %d\n",(long)kcurrentinstance,(*p_Instance)[kcurrentinstance].contparameters.number);
					if(currentinstancevalues == NULL) {
						BPPrintMessage(0,odError,"=> Err. MakeSound(). currentinstancevalues == NULL");
						goto FORGETIT;
						}
					if(params == NULL) {
						// Never observed: params are created for every sequence at the start of MakeSound()
						BPPrintMessage(0,odError,"=> Err. MakeSound(). (*((*pp_currentparams)[nseq]))->params == NULL\n");
						result = ABORT; goto OVER;
						}
					// 2026-11-13 The parameters of the sequence are created with IPANORAMIC + 1 entries, so they may need to grow.
					// Those of the instance were created by SetObjectParams() with MaxParameters + IPANORAMIC + 1 entries
					// and were resized here to the same size, which did nothing. This is now removed - fixed by Claude
					int currparamnr = MyGetHandleSize((Handle)params) / sizeof(ParameterStatus);
					instrument = ThisInstrument(cswrite,kcurrentinstance,ievent,&instr_index);
					maxparam = MaxParameters + IPANORAMIC + 1;
					if(maxparam > currparamnr) {
						h = (Handle) params;
						if(MySetHandleSize(&h,(Size)maxparam * sizeof(ParameterStatus)) != OK) {
							result = ABORT; goto OVER;
							}
						(*((*pp_currentparams)[nseq]))->params = params = (ParameterStatus**) h;
						for(iparam = currparamnr; iparam < maxparam; iparam++) {
							(*params)[iparam].active = FALSE;
							(*params)[iparam].startvalue = 0.;
							(*params)[iparam].endvalue = 0.;
							(*params)[iparam].imax = ZERO; 
							(*params)[iparam].point = NULL;
							(*params)[iparam].starttime = ZERO;
							(*params)[iparam].dur = ZERO;
							}
						}
					(*((*pp_currentparams)[nseq]))->numberparams = maxparam;
					if((*p_Instance)[kcurrentinstance].contparameters.number != maxparam) {
						BPPrintMessage(0,odError,"=> Err. MakeSound(). Instance %d has %d parameters instead of %d\n",kcurrentinstance,(*p_Instance)[kcurrentinstance].contparameters.number,maxparam);
						result = ABORT; goto OVER;
						}
					SetObjectParameters(params,currentinstancevalues,maxparam,kcurrentinstance,j,t0,objectduration);
					(*((*pp_currentparams)[nseq]))->paramsinstance = kcurrentinstance;
					}
	FORGETIT:
				if(kcurrentinstance > 1) { 
					if(trace_makesound) 
						BPPrintMessage(0,odInfo,"The kcurrentinstance = %ld\n",(long)kcurrentinstance);
					Tcurr = (t0 + t1) / Time_res;
					rs = 0;
					// note 18
					// Look at attached script line(s)
					if((*p_ObjectSpecs)[kcurrentinstance] != NULL
							&& (scriptlist=ObjScriptLine(kcurrentinstance)) != NULL) {
					
						WhenItStarted = oldtime = clock();
						// WhenItStarted may be further updated if it is called by ExecuteScriptList()
						
						if((result=ExecuteScriptList(scriptlist)) != OK && result != RESUME)
							goto OVER;
							
						LapWait += (clock() - WhenItStarted);
						if(ScriptExecOn && CountOn && WaitEndDate > ZERO)
							WaitEndDate += LapWait;
						rs = 0;
						}
					
					// Reset tick cycle if requested
					Tcurr = (t0 + t1) / Time_res;
					if(trace_makesound) 
						BPPrintMessage(0,odInfo,"(*p_Instance)[kcurrentinstance].contparameters.number = %ld\n",(long)(*p_Instance)[kcurrentinstance].contparameters.number);	
					if((*p_Instance)[kcurrentinstance].contparameters.number
							> (*p_control)[nseq].number) {
						(*p_control)[nseq].number = (*p_Instance)[kcurrentinstance].contparameters.number;
						h = (Handle) (*p_control)[nseq].param;
						MySetHandleSize((Handle*)&h,(*p_control)[nseq].number * sizeof(ParameterStream));
						(*p_control)[nseq].param = (ParameterStream**) h;
						}
					
					// note 19
					// Look at continuous MIDI parameters
					
					stream = (*p_control)[nseq].param;
					if(trace_makesound)
						BPPrintMessage(0,odInfo,"\nMakeSound() kcurrentinstance = %d number = %d\n",kcurrentinstance,(*p_Instance)[kcurrentinstance].contparameters.number);
					if(PlayCsoundLine(cswrite,j)) goto SWITCHES;
					for(index = 0; index < (*p_Instance)[kcurrentinstance].contparameters.number; index++) {
				//		index = (*currentinstancevalues)[index].index;
						if(index < 0) continue;
						if((index == IPANORAMIC && j < Jbol && !(*p_OkPan)[j])
								|| (index == IVOLUME && j < Jbol && !(*p_OkVolume)[j])) {
							(*stream)[index].ibm = -1L;
							continue;
							}
						if(index <= IPANORAMIC && (*currentinstancevalues)[index].mode == CONTINUOUS) {
							// First terminate current control. May occur because of rounding errors
							if((*stream)[index].ibm > -1L) {
								ch = (*stream)[index].channel;
								if((*(p_active[ch]))[index]) {
									if((result=SendControl(p_control,t0,ch,index,maxconc,cswrite,showpianoroll,&rs,p_active,p_t2cont,p_seqcont,p_Oldvalue,pp_currentparams)) != OK) goto OVER;
									(*(p_active[ch]))[index] = FALSE;
									}
								(*stream)[index].ibm = -1L;
								}
							
							(*stream)[index].starttime
								= (*p_Instance)[kcurrentinstance].starttime - (*p_Instance)[kcurrentinstance].truncbeg;
							(*stream)[index].channel = ch = (*currentinstancevalues)[index].channel - 1;
							if(ch < 0 || ch >= MAXCHAN) {
							//	BPPrintMessage(0,odError,"=> Err. MakeSound(). ch < 0 || ch >= MAXCHAN");
								result = ABORT;
								goto OVER;
								}
							(*stream)[index].startvalue = (*currentinstancevalues)[index].v0;
							(*stream)[index].endvalue = (*currentinstancevalues)[index].v1;
							(*stream)[index].duration = objectduration;
							(*stream)[index].ibm = Findibm(index,(*stream)[index].duration,ch);
							(*stream)[index].ib = 0;
							(*stream)[index].imax = (*currentinstancevalues)[index].imax;
							// BPPrintMessage(0,odInfo,"## stream[%d].imax = %d\n",index,(*stream)[index].imax);
							if((*stream)[index].imax > ZERO) {
								(*stream)[index].point = (*currentinstancevalues)[index].point;
								}
							else (*stream)[index].point = NULL;
							if(!(*(p_active[ch]))[index] && (*stream)[index].ibm > ZERO) {
								(*(p_active[ch]))[index] = TRUE;
								(*(p_seqcont[ch]))[index] = nseq;
								if((result=SendControl(p_control,t0,ch,index,maxconc,cswrite,showpianoroll,&rs,p_active,p_t2cont,p_seqcont,p_Oldvalue,pp_currentparams)) != OK) goto OVER;
								if(t2 > (*(p_t2cont[ch]))[index]) {
									t2 = (*(p_t2cont[ch]))[index];
									icont = index;
									chancont = ch;
									}
								}
							}
						else (*stream)[index].ibm = -1L;
						}

	SWITCHES:
					// note 20
					// Look at switches (including hold pedal)
					if((result=SendSwitches(kcurrentinstance,currswitchstate,cswrite,(t0 + t1),nseq,&rs)) != OK) goto OVER;
					}
					
				// Send dilation ratio to MIDI if applicable
				if((result=SendDilationRatio(j,beta,objectchannel,cswrite,(t0 + t1),nseq,&rs)) != OK) goto OVER;
				}
		
	// Send messages of sound-object instance kcurrentinstance as long as possible

	// PLAYOBJECT
			if(trace_makesound) {
				if(j >= 16384) PrintThisNote(-1,j-16384,0,-1,this_note);
				else strcpy(this_note,"an object");
				BPPrintMessage(1,odInfo,"\n\nPLAYOBJECT: kcurrentinstance = %d, j = %d « %s », objectduration = %ld, t1 = %ld, t2 = %ld, t3 = %ld, ievent = %d, im = %d\n",kcurrentinstance,j,this_note,objectduration,(long)t1,(long)t2,(long)t3,ievent,im);
				}
			// note 21
			while(t1 <= t2  && t1 <= t3  && ievent < im) {
				if(j < Jbol && j > 1 && trace_makesound) BPPrintMessage(0,odInfo,"@@@ CyclicMode[%d] = %d, t1 = %ld, t2 = %ld, t3 = %ld, ievent = %ld, im = %ld\n",j,(*p_CyclicMode)[j],t1,t2,t3,ievent,im);
				if((result=stop(0,"MakeSound")) != OK) goto OVER;
				// note 22
				if(j < Jbol && j > 1 && t1 >= t3 && ((*p_CyclicMode)[j] != IRRELEVANT) && !(*p_ForceIntegerCycles)[j]) {
					if(trace_makesound) 
						BPPrintMessage(0,odInfo,"§§ t1 >= t3, t1 = %ld, t3 = %ld\n",(long)t1,(long)t3);
					if(ievent == (im - 1) && !EventListOn) {
						if(trace_makesound) BPPrintMessage(0,odInfo,"§§§ ievent = %ld, im = %ld\n",ievent,im);
					//	if(cswrite) localchan = 0;
						for(key = 0; key < MAXKEY; key++) {
							onoff = ByteToInt((*p_keychannel[localchan])[key]);
							if(onoff) {
								(*p_keychannel[localchan])[key]--;
								if(trace_makesound) BPPrintMessage(0,odInfo,"@ key = %d NoteOff\n",key);
								if(!cswrite) {
									if((result=SendNoteOffAndDraw(key,localchan,kcurrentinstance,scale,blockkey,(t0 + t1),nseq,&rs,showpianoroll,&roll,p_last_timeon[localchan],TRUE)) != OK) goto OVER;
									}
								else {
									instrument = ThisInstrument(cswrite,kcurrentinstance,ievent,&instr_index);
									if(trace_makesound)
										BPPrintMessage(0,odInfo,"Before CscoreWrite(1) kcurrentinstance = %ld j = %ld beta = %.2f t0 = %ld t1 = %ld c1 = %ld c2 = %ld, localchan = %d instrument = %d\n",(long)kcurrentinstance,(long)j,beta,(long)t0,(long)t1,(long)key,(long)c2,localchan,instrument);
									if((result =
			CscoreWrite(ZERO,&roll.graphrect,roll.noteoffset,roll.topoffset,roll.hrect,roll.minkey,roll.maxkey,strikeagain,OFF,beta,(t0 + t1),-1,key,0,localchan,instrument,
				j,nseq,kcurrentinstance,pp_currentparams,scale,blockkey)) == ABORT)
											goto OVER;
									}
								}
							else if(trace_missed_noteoff) BPPrintMessage(0,odInfo,"@ missed_noteoff key = %d, instrument = %d, kcurrentinstance = %d\n",key,instrument,kcurrentinstance);
							}
						}
					goto NEWPERIOD;
					}
				Tcurr =  (t0 + t1) / Time_res;
				if(trace_makesound) BPPrintMessage(0,odInfo,"Tcurr = %ld, j = %ld, t0 = %ld, t1 = %ld, Time_res = %ld\n",(long)Tcurr,(long)j,(long)t0,(long)t1,(long)Time_res);
				result = OK;
				if(objectduration > ZERO) howmuch = ((float)(t1 - objectstarttime)) / objectduration;
				else howmuch = 0.;
				if(trace_makesound) {
					BPPrintMessage(0,odInfo,"kcurrentinstance = %d, j = %d, ievent = %d, beta = %.2f, t0 = %ld, t1 = %ld, t2 = %ld\n",kcurrentinstance,j,ievent,beta,(long)t0,(long)t1,(long)t2);
					BPPrintMessage(0,odInfo,"@ objectduration = %ld, howmuch = %.3f\n",objectduration,howmuch);
					}
				// note 23
				if(PlayCsoundLine(cswrite,j)) { 
				// Writing a Csound event taken from the Csound score of a sound-object 
				//	localchan = 0;
					cut_the_end = ZERO;
					localchan = MIDIchannel(cswrite,j,objectchannel,&c0);
					if(!(*p_silence)[kcurrentinstance]) {
						instrument = ThisInstrument(cswrite,kcurrentinstance,ievent,&instr_index);
						if((*p_Instance)[kcurrentinstance].truncend > EPSILON) {
							startsilence = (*p_Instance)[kcurrentinstance].starttime - (*p_Instance)[kcurrentinstance].truncbeg + (beta * (*p_Dur)[j]) - (*p_Instance)[kcurrentinstance].truncend;
							end_this_event = (t0 + t1) + (*((*pp_CsoundScore)[j]))[ievent].duration * beta;
							if(end_this_event > startsilence)
								cut_the_end = end_this_event - startsilence;
							if(trace_makesound) BPPrintMessage(0,odInfo,"@) kcurrentinstance = %d j = %d, ievent = %ld, t1 = %ld, startsilence = %ld, end_this_event = %ld, cut_the_end = %ld, Dur= %ld, beta = %.2f, starttime = %ld, truncend = %ld\n",kcurrentinstance,j,ievent,(long)t1,startsilence,end_this_event,cut_the_end,(*p_Dur)[j],beta,(*p_Instance)[kcurrentinstance].starttime,(*p_Instance)[kcurrentinstance].truncend);
							}
						else startsilence = end_this_event = ZERO;
						if(trace_makesound) 
							BPPrintMessage(0,odInfo,"Before CscoreWrite(2) kcurrentinstance = %ld, j = %ld, beta = %.2f, t0 = %ld, t1 = %ld, t2 = %ld, t3 = %ld, c1 = %ld, c2 = %ld, localchan = %d instrument = %d\n",(long)kcurrentinstance,(long)j,beta,(long)t0,(long)t1,t2,t3,(long)c1,(long)c2,localchan,instrument);
						if((result=CscoreWrite(cut_the_end,&roll.graphrect,roll.noteoffset,roll.topoffset,roll.hrect,roll.minkey,roll.maxkey,strikeagain,LINE,beta,(t0 + t1),ievent,0,0,localchan,instrument,j,nseq,kcurrentinstance,pp_currentparams,scale,blockkey)) == ABORT) goto OVER;
						}
					goto NEWPERIOD;
					}
				// note 24
				time_pattern = FALSE;
				if(!PlayCsoundLine(cswrite,j)) {
					if(j < Jbol) {
						sequence = (*((*pp_MIDIcode)[j]))[ievent].sequence;
						c0 = (*((*pp_MIDIcode)[j]))[ievent].byte;
						c1 = (*((*pp_MIDIcode)[j]))[ievent+1].byte;
						}
					else { // Simple note or time pattern
						if(j < 16384) time_pattern = TRUE;
						if(ievent == 0) c0 = NoteOn;
						else c0 = NoteOff;
						sequence = 0;
						}
					}
				localchan = MIDIchannel(cswrite,j,objectchannel,&c0);
				if(trace_makesound)
					BPPrintMessage(0,odInfo,"Calculating MIDI channel j = %d, c0 = %d, ChannelEvent(c0) = %d, localchan = %d\n",j,c0,ChannelEvent(c0),localchan);
				
				// Inspect object
				if(c0 >= 0) {
					if(c0 == NoteOn  || c0 == NoteOff) {
						if(j < 16384) {
							if(j >= Jbol) { // Time pattern
								if(trace_makesound) BPPrintMessage(0,odError,"@@@ time pattern: j = %ld, t1 = %ld, t2 = %ld\n",(long)j,(long)t1,(long)t2);
								sequence = 0;
								c0 = 0;
								c1 = 0;
								c2 = 0;
								ievent += 2;
								}
							else if(!PlayCsoundLine(cswrite,j)) {
								sequence = (*((*pp_MIDIcode)[j]))[ievent].sequence;
								c1 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
								c2 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
								}
							simplenote = FALSE;
							}
						else {	// Simple note
							sequence = 0;
							c1 = GoodKey(j);
							c2 = 127;
							ievent += 2;
							simplenote = TRUE;
							}
						// note 25
						if(c1 > 0) {
			//			if(c1 > 0 && c0 != NoteOff && c2 != 0) {
							TransposeAndExpand(kcurrentinstance,&c1,trans);
				//			BPPrintMessage(0,odInfo,"@§@ howmuch = %2.f\n",howmuch);
			//				c1 = MapThisKey(c1,howmuch,(*p_Instance)[kcurrentinstance].mapmode,&((*p_Instance)[kcurrentinstance].map0),&((*p_Instance)[kcurrentinstance].map1));
							}
						// note 26
						if(c0 == NoteOff || c2 == 0) {
							if(j >= 16384 || (*p_OkMap)[j])
								c1 = RetrieveMappedKey(c1,kcurrentinstance,localchan,p_currmapped,maxmapped);
							onoff = ByteToInt((*p_keychannel[localchan])[c1]);
							if(trace_makesound) 
								BPPrintMessage(0,odInfo,"@§ onoff = %d\n",onoff);
							if((onoff > 0) && !time_pattern) {
								if(onoff > 0) ((*p_keychannel[localchan])[c1])--;
								if(trace_makesound)
									BPPrintMessage(1,odInfo,"@ keyoff %d = %d\n",c1,(*p_keychannel[localchan])[c1]);
								onoff = ByteToInt((*p_keychannel[localchan])[c1]);
								if((onoff == 0 || cswrite)) {
									if(TraceNoteOn) {
										PrintThisNote(-1,c1,0,-1,this_note);
										BPPrintMessage(1,odInfo,"•    NoteOff %s channel %d at %ld ms, t0 = %ld, t1 = %ld\n",this_note,(localchan + 1),(Tcurr * Time_res - MIDIsetUpTime),t0,t1);
										}
									if(!cswrite) {
										if((result=SendNoteOffAndDraw(c1,localchan,kcurrentinstance,scale,blockkey,(t0 + t1),nseq,&rs,showpianoroll,&roll,p_last_timeon[localchan],TRUE)) != OK) goto OVER;
										}
									else {
										instrument = ThisInstrument(cswrite,kcurrentinstance,ievent,&instr_index);
										if((*p_keyinstrument[instr_index])[c1] > 0) {
											onoff = TRUE;
											((*p_keyinstrument[instr_index])[c1])--;
											}
										else onoff = FALSE;
										if(onoff) {
											if(trace_makesound)
												BPPrintMessage(0,odInfo,"Before CscoreWrite(3) NoteOff kcurrentinstance = %ld j = %ld, ievent = %ld, beta = %.2f t0 = %ld t1 = %ld c1 = %ld c2 = %ld, localchan = %d instrument = %d\n",(long)kcurrentinstance,(long)j,ievent,beta,(long)t0,(long)t1,(long)c1,(long)c2,localchan,instrument);
											if((result =
				CscoreWrite(ZERO,&roll.graphrect,roll.noteoffset,roll.topoffset,roll.hrect,roll.minkey,roll.maxkey,strikeagain,OFF,beta,(t0 + t1),-1,c1,c2,localchan,instrument,
					j,nseq,kcurrentinstance,pp_currentparams,scale,blockkey)) == ABORT)
												goto OVER;
											}
										}
									}
								}
							else if(trace_missed_noteoff) BPPrintMessage(0,odInfo,"@ missed_noteoff2 key = %d, instrument = %d, kcurrentinstance = %d\n",key,instrument,kcurrentinstance);
							}
						else { // NoteOn
							// note 27
							oldc1 = c1;
							if(j >= 16384 || (*p_OkMap)[j]) {
								if(trace_makesound)
									BPPrintMessage(0,odInfo,"§§ c1 = %d, mapmode = %d\n",c1,(*p_Instance)[kcurrentinstance].mapmode);
								c1 = MapThisKey(c1,howmuch,(*p_Instance)[kcurrentinstance].mapmode,&((*p_Instance)[kcurrentinstance].map0),
									&((*p_Instance)[kcurrentinstance].map1));
								}
							if(trace_makesound)
								BPPrintMessage(0,odInfo,"§§§ c1 = %d, localchan = %d\n",c1,localchan);
							if(c1 > 127) {
								BPPrintMessage(0,odError,"=> Error MakeSound(). c1= %d\n",c1);
								c1 = 0;
								}
							onoff = ByteToInt((*p_keychannel[localchan])[c1]);
							if(trace_makesound)
								BPPrintMessage(1,odInfo,"\n** Tcurr = %ld NoteOn, localchan = %d, c0 = %d c1 = %d onoff = %d nseq = %d\n",(long)Tcurr,localchan,c0,c1,onoff,nseq);
							strike = TRUE;
							((*p_keychannel[localchan])[c1])++;
							if(trace_makesound)
								BPPrintMessage(1,odInfo,"@ keyon %d = %d\n",c1,(*p_keychannel[localchan])[c1]);
							// 2026-11-14: dates of NoteOns are the exact dates (t0 + t1) sent to the driver, not the clock time Tcurr * Time_res, which is rounded down to a multiple of Time_res.
							// Otherwise, with time patterns or a large Time_res, notes were drawn on the pianoroll before their start date.
							delta_timeon = labs((t0 + t1) - (*p_last_timeon[localchan])[c1]);
							if(TraceNoteOn && (*p_last_timeon[localchan])[c1] > 0L) {
								PrintThisNote(-1,c1,0,-1,this_note);
								BPPrintMessage(1,odInfo,"? %s channel %d at = %ld ms, last = %ld, delta = %ld > %ld ms ¿\n",this_note,(localchan + 1),(long)(Tcurr * Time_res - MIDIsetUpTime),(*p_last_timeon[localchan])[c1] - MIDIsetUpTime,delta_timeon,MaxDeltaTime);
								}
							// note 28
							// 2026-11-13: a NoteOn closer than MaxDeltaTime to the previous NoteOn of the same key is only skipped if
							// this key is still sounding (onoff > 0), so that both are merged. If the key had been released, the NoteOn was
							// skipped as well, and the note was lost, e.g. G4 and A4 trills in two parts of Beethoven's Grosse Fuge, m.10.
							// Since the skipped note was counted as sounding, the next NoteOn of this key sent a NoteOff for it,
							// which found no channel in MPE ("No channel available") - fixed by Claude
							if(cswrite || (delta_timeon > MaxDeltaTime) || onoff == 0) { // 2025-01-11
								if(!time_pattern && (onoff > 0)) {
									if(strikeagain) {
										// First send NoteOff
										if(TraceNoteOn) {
											PrintThisNote(-1,c1,0,-1,this_note);
											BPPrintMessage(1,odInfo,"@@ NoteOff %s channel %d at %ld ms\n",this_note,(localchan + 1),(Tcurr * Time_res - MIDIsetUpTime));
											}
										if(!cswrite) {
											if((result=SendNoteOffAndDraw(c1,localchan,kcurrentinstance,scale,blockkey,(t0 + t1),nseq,&rs,showpianoroll,&roll,p_last_timeon[localchan],FALSE)) != OK) goto OVER;
											}
										}
									else strike = FALSE;
									}
								// note 29
								if(strike && !time_pattern) {
									if(j >= Jbol || (*p_OkVelocity)[j])
										c2 = ClipVelocity(c2,localvelocity,
											(*p_Instance)[kcurrentinstance].velcontrol,
											(*p_Instance)[kcurrentinstance].rndvel);
									if(c1 != oldc1) {
										if((result=StoreMappedKey(oldc1,c1,kcurrentinstance,localchan,
											&p_currmapped,&maxmapped)) != OK) goto OVER;
										}
									if(!(*p_silence)[kcurrentinstance]) {
										// NoteOns are silenced in truncated part at the end
										if(TraceNoteOn)  {
											PrintThisNote(-1,c1,0,-1,this_note);
											BPPrintMessage(1,odInfo,"NoteOn %s channel %d at %ld ms\n",this_note,(localchan + 1),(Tcurr * Time_res - MIDIsetUpTime));
											}
										if(!cswrite) {
											if(trace_makesound) BPPrintMessage(0,odInfo,"*** Tcurr = %ld NoteOn c1 = %d onoff = %d nseq = %d strike\n",(long)Tcurr,c1,onoff,nseq);
											(*p_last_timeon[localchan])[c1] = t0 + t1;
											if((result=SendMIDIevent(NORMAL_EVENT,c0 + localchan,c1,c2,kcurrentinstance,scale,blockkey,(t0 + t1),nseq,&rs)) != OK) goto OVER;
											}
										else {
											instrument = ThisInstrument(cswrite,kcurrentinstance,ievent,&instr_index);
											if(trace_makesound)
												BPPrintMessage(0,odInfo,"Before CscoreWrite(4) NoteOn kcurrentinstance = %ld, j = %ld, ievent = %d, beta = %.2f t0 = %ld t1 = %ld c1 = %ld c2 = %ld, localchan = %d instrument = %d instr_index = %d\n",(long)kcurrentinstance,(long)j,ievent,beta,(long)t0,(long)t1,(long)c1,(long)c2,localchan,instrument,instr_index);
											((*p_keyinstrument[instr_index])[c1])++;
											if((result =
					CscoreWrite(ZERO,&roll.graphrect,roll.noteoffset,roll.topoffset,roll.hrect,roll.minkey,roll.maxkey,strikeagain,ON,beta,(t0 + t1),-1,c1,c2,localchan,instrument,
						j,nseq,kcurrentinstance,pp_currentparams,scale,blockkey)) == ABORT)
												goto OVER;
											}
										if(showpianoroll) {
											result = OK;
											if(!cswrite)  // 2026-03-31
												result = DrawPianoNote("midi",c1,localchan,(*p_last_timeon[localchan])[c1],(t0 + t1),roll.noteoffset,roll.topoffset,roll.hrect,roll.minkey,roll.maxkey,&roll.graphrect);
											if(result != OK) goto OVER;
											}
										(*((*pp_currentparams)[nseq]))->starttime[c1] = (t0 + t1) / 1000.;
										}
									}
								}
							}
						// note 30
						// Since ievent was incremented twice t1 should be updated accordingly
						if(j < Jbol && !PlayCsoundLine(cswrite,j)) {
							t1 += (Milliseconds)(beta
								* ((*((*pp_MIDIcode)[j]))[ievent-1].time
									+ (*((*pp_MIDIcode)[j]))[ievent].time));
							}
						if(trace_makesound) BPPrintMessage(1,odInfo,"New t1 = %ld, t3 = %ld\n",t1,t3);
						Tcurr = (t0 + t1) / Time_res;
						}
					else {
						// note 31
						if(ievent >= im) break;
						if(!cswrite && ChannelEvent(c0)) {	// Channel message
							c0 = c0 + localchan;
							}
					//	else localchan = 0;
						if(j < Jbol && !PlayCsoundLine(cswrite,j)) {
							if(ThreeByteEvent(c0)) {
								sequence = (*((*pp_MIDIcode)[j]))[ievent].sequence;
								c1 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
								c2 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
								if(!cswrite && (result=SendMIDIevent(NORMAL_EVENT,c0,c1,c2,kcurrentinstance,scale,blockkey,(t0 + t1),nseq,&rs)) != OK) goto OVER;
								t1 += (Milliseconds)(beta
									* ((*((*pp_MIDIcode)[j]))[ievent-1].time
										+ (*((*pp_MIDIcode)[j]))[ievent].time));
								}
							else {
								if(TwoByteEvent(c0)) {
								//	if(cswrite) continue;
									sequence = (*((*pp_MIDIcode)[j]))[ievent].sequence;
									c2 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
									if(!cswrite && (result=SendMIDIevent(TWO_BYTE_EVENT,c0,0,c2,0,0,0,(t0 + t1),nseq,&rs)) != OK) goto OVER;
									t1 += (Milliseconds)(beta * (*((*pp_MIDIcode)[j]))[ievent].time);
									}
								else {
									if(c0 != TimingClock) {
									// Suppress timing clock events internally used to create silences
										rs = 0;
										// 2026-11-13: 'status' was not set, so real-time MIDI sent the status of the previous event instead of c0 - fixed by Claude
										if(!cswrite && (result=SendMIDIevent(RAW_EVENT,c0,0,c0,0,0,0,(t0 + t1),nseq,&rs)) != OK) goto OVER;
										}
									}
								}
							}
						}
					}
NEWPERIOD:
				if(trace_makesound) {
					BPPrintMessage(0,odInfo,"\nNEWPERIOD:\n");
					BPPrintMessage(0,odInfo,"kcurrentinstance = %d, istartperiod = %d, ievent = %ld, im = %ld, iendperiod = %d, time_nextperiod_start = %.2f, objectperiod = %.2f, t1 = %ld, t3 = %ld, silence = %d\n",kcurrentinstance,(*p_istartperiod)[kcurrentinstance],ievent,im,(*p_iendperiod)[kcurrentinstance],time_nextperiod_start,objectperiod,t1,t3,(*p_silence)[kcurrentinstance]);
					}
				// note 32
				if((*p_istartperiod)[kcurrentinstance] > -1 && ievent >= (*p_iendperiod)[kcurrentinstance]) {
					// Cyclic object: start another period
					t1 = time_nextperiod_start;
					t1 += (*p_periodgap)[kcurrentinstance];
					time_nextperiod_start += objectperiod;
					if(trace_makesound) BPPrintMessage(0,odInfo,"@§§ t1 = %ld, time_nextperiod_start = %ld, periodgap = %ld\n",(long)t1,(long)time_nextperiod_start,(long)(*p_periodgap)[kcurrentinstance]);
					if(t1 >= t3) {
						if(trace_makesound) BPPrintMessage(0,odInfo,"§§§ t1 >= t3, t1 = %ld, t2 = %ld, t3 = %ld\n",(long)t1,(long)t2,(long)t3);
						ievent++;
						break;
						}
					if(trace_makesound)
						BPPrintMessage(0,odInfo,"••• periodgap = %ld ms, t1 = %ld, t2 = %ld, t3 = %ld\n",(long)(*p_periodgap)[kcurrentinstance],(long)t1,(long)t2,(long)t3);
					(*p_icycle)[kcurrentinstance]++;

					if(!send_more && j < Jbol && j > 1 && !(*p_ForceIntegerCycles)[j]) {
						// We'll send messages in the last (incomplete) cycle
						send_more = TRUE;
						if(trace_makesound)
							BPPrintMessage(0,odInfo,"send_more = %d\n",send_more);
						}
					if((t1 <= t3) && ((*p_icycle)[kcurrentinstance]
							<= (*p_Instance)[kcurrentinstance].ncycles || send_more)) {
						ievent = (*p_istartperiod)[kcurrentinstance];
						if(trace_makesound)
							BPPrintMessage(0,odInfo,"icycle = %d, ncycles = %d, ievent = %ld, t1 = %ld, t2 = %ld, t3 = %ld\n",(*p_icycle)[kcurrentinstance],(*p_Instance)[kcurrentinstance].ncycles,ievent,(long)t1,(long)t2,(long)t3);
						continue;
						}
					else {
						ievent++;
						if(trace_makesound)
							BPPrintMessage(0,odInfo,"ievent (new) = %ld, then break\n",ievent);
						break;
						}
					}
				// note 33
				ievent++;
				if(ievent >= im) break;
				if(j < Jbol) {
					if(trace_makesound)
						BPPrintMessage(0,odInfo,"•• t1 = %ld, ievent = %ld, im = %ld, beta = %.2f, j = %d, delay = %ld\n",(long)t1,ievent,im,beta,j,(long)EventDelay(cswrite,j,ievent));
					t1 += (Milliseconds)(beta * EventDelay(cswrite,j,ievent));
					}
				else t1 += (Milliseconds)(beta * 1000.);
				}

	// note 34
	// All messages have been sent. Let's see whether sound-object instance kcurrentinstance is over

			if(ievent < im) {
				(*p_inext)[kcurrentinstance] = ievent;
				if(t1 <= t3)
					(*p_nextd)[kcurrentinstance] = t1;
				else {
					(*p_nextd)[kcurrentinstance] = t3;	// End of object is truncated
					j = (*p_Instance)[kcurrentinstance].object;
					if(j < Jbol && j > 1 && (*p_Instance)[kcurrentinstance].truncend > EPSILON) {
						(*p_silence)[kcurrentinstance] = TRUE;
						if(trace_makesound)
							BPPrintMessage(0,odInfo,"### Following NoteOns will be silenced in the truncated part, t1 = %ld, t3 =  %ld\n",t1,t3);
						}
					}
				if(trace_makesound)
					BPPrintMessage(0,odInfo,"+++fix nextd, ievent = %ld, im = %ld, kcurrentinstance = %d, t1 = %ld, t3 = %ld, nextd = %ld\n",ievent,im,kcurrentinstance,(long)t1,(long)t3,(long)(*p_nextd)[kcurrentinstance]);
				}
			else {
				// Sound-object kcurrentinstance is OVER
				(*p_nextd)[kcurrentinstance] = Infpos;
				doneobjects++;
				if(MIDIfileOn || cswrite || !rtMIDI || ItemCapture || showpianoroll) {
					if(trace_makesound) {
						my_sprintf(Message,"%ld objects done out of %ld\n",(long)kcurrentinstance,(long)(*p_kmax));
						BPPrintMessage(1,odInfo,Message);
						}
					if((MIDIfileOn || cswrite) && (!ItemCapture || ShowMessages)
							&& doneobjects > 10) {
						doneobjects = 0;
						}
					}
				}
			result = OK;
			
			if(Beta && (chancont >= MAXCHAN || icont > IPANORAMIC)) {
			//	BPPrintMessage(0,odError,"=> Err. MakeSound(). chancont >= MAXCHAN || icont > IPANORAMIC");
				goto OVER;
				}
			if(cswrite || chancont < 0 || icont < 0 || !(*(p_active[chancont]))[icont])
				goto FINDNEXTEVENT;

	// note 35
SENDCONTROLMESSAGE:
			// Send message of continuous parameter
			if(chancont < 0 || icont < 0 || chancont >= MAXCHAN || icont > IPANORAMIC) {
		//		BPPrintMessage(0,odError,"=> Err. MakeSound(). chancont < 0 || icont < 0 || chancont >= MAXCHAN || icont > IPANORAMIC");
				goto OVER;
				}
			if(!(*(p_active[chancont]))[icont]) {
		//		BPPrintMessage(0,odError,"=> Err. MakeSound(). !(*(p_active[chancont]))[icont]");
				goto OVER;
				}
			if(t2 != Infpos && t2 == (*(p_t2cont[chancont]))[icont])
				if((result=SendControl(p_control,t0,chancont,icont,maxconc,cswrite,showpianoroll,&rs,p_active,p_t2cont,p_seqcont,p_Oldvalue,pp_currentparams)) != OK)
					goto OVER;


	// note 36
FINDNEXTEVENT:
			// Look for the next event: in a sound-object instance, a tick, or a continous parameter
			oldk = kcurrentinstance;
			kcurrentinstance = NextInstance(*p_kmax,p_nextd,p_silence,oldk,&t2obj);
			t2 = t2obj;
			nextisobject = !NextControlDate(p_active,p_t2cont,&t2,&icont,&chancont);
				
			if(!nextisobject && t2 != Infpos) goto SENDCONTROLMESSAGE; // The next event is a message of a continuous control
			else {
				// This will take us back to the preceding object
				if(kcurrentinstance > 1) {
					t1 = (*p_nextd)[kcurrentinstance];
					if(trace_makesound)
						BPPrintMessage(0,odInfo,"++ back to preceding object, kcurrentinstance = %d, t1 = %ld\n",kcurrentinstance,t1);
					}
				}
			// Here we go back to sound-object instance kcurrentinstance
			}

		// The item has been sent entirely.

		ItemOutPutOn = FALSE;
				
		currenttime = Tcurr * Time_res;
		}
	// note 37
	// End of the Nplay performances
	if(rtMIDI) { // 2024-06-17
		// Keep the driver alive to be able to handle MIDI input events
		if((result=SendMIDIevent(NORMAL_EVENT,ActiveSensing,0,0,0,0,0,(t0 + t1),0,&rs)) != OK) goto OVER;
		}

	// Now, codes have been sent and may still being played by the MIDI
	// driver.  Before proceeding further we might wait until the buffer is empty
	// or not too much filled.

	if(showpianoroll) goto OUTGRAPHIC;

	SynchroSignal = OFF;
	RunningStatus = 0;	/* This is used by ListenToEvents() */
	result = OK;

	// note 38
OVER:
	if(result == ENDREPEAT) result = OK;
	if(Panic || result == STOP) result = ABORT;
	if(ChangedGrammar) {
//	if(ChangedGrammar || ChangedSettings) {
		eventCount = 0L;
		result = OK;
		}
	if(result == ABORT) {
		BPPrintMessage(0,odError,"(Stopped playing)\n");
		goto GETOUT;
		}

OUTGRAPHIC:
	if(showpianoroll) goto GETOUT;

	if(!cswrite && !Panic && (result == RESUME || (!Improvize && !PlayAllChunks && !CyclicPlay))) {
		if(ReleaseKeysAndSwitches(p_keychannel,currswitchstate,&rs) != OK) {
			result = ABORT;
			goto GETOUT;
			}
		}

GETOUT:

	if(EventState == AGAIN) result = AGAIN;

	// note 39
	// Calculate duration of silence at the end in order to append it after the Csound score or MIDI stream
	// Added by BB 2021-02-06
	max_endtime_event = max_endtime = ZERO;
	for(k = 2; k <= (*p_kmax); k++) {
		j = (*p_Instance)[k].object;
		// BPPrintMessage(0,odInfo,"k = %ld, j =  %d, max_endtime = %ld\n",(long)k,j,(long)max_endtime);
		if(j > 1 && (*p_Instance)[k].endtime > max_endtime_event) max_endtime_event = (*p_Instance)[k].endtime;
		if((*p_Instance)[k].endtime > max_endtime) {
			max_endtime = (*p_Instance)[k].endtime;
			}
		}
	add_time = max_endtime - max_endtime_event;

	if(trace_makesound) 
		BPPrintMessage(0,odInfo,"max_endtime = %ld, max_endtime_event = %ld\n",(long)max_endtime,(long)max_endtime_event);

	if(add_time > ZERO  && (Improvize || PlayAllChunks)) {
		if((MIDIfileOn || rtMIDI) && !Create_set) {
			// Two NoteOffs (key 0) mark the silence
			if((result=SendMIDIevent(NORMAL_EVENT,NoteOn,0,0,0,scale,blockkey,(Tcurr * Time_res),nseq,&rs)) != OK) goto OVER2;
			if((result=SendMIDIevent(NORMAL_EVENT,NoteOn,0,0,0,scale,blockkey,((Tcurr * Time_res) + add_time),nseq,&rs)) != OK) goto OVER2;
			}
		if(cswrite) {
		//	BPPrintMessage(0,odInfo,"Added silence of %.3f sec at time = %.3f sec.\n",(add_time / 1000.),((LastTime + max_endtime_event) / 1000.));
			my_sprintf(Message,"i1 %.3f %.3f 0.00 0 0 ; silence",((LastTime + max_endtime_event) /1000.),add_time /1000.);
		//	my_sprintf(Message,"s %.3f ; silence",add_time /1000.); // Doesn't work: too long?
			WriteToFile(NO,CsoundFileFormat,Message,CsRefNum);
			if(((LastTime + max_endtime_event) + add_time) / 1000. > CsScoreEnd)
				CsScoreEnd = ((LastTime + max_endtime_event) + add_time) / 1000.;
			}
		}
OVER2:
	// note 40
	if(cswrite) LastTime += max_endtime;

#if CSOUND_SECTIONS
	if(cswrite && result == OK && !Improvize && !PlayAllChunks && !ConvertMIDItoCsound && !AllItems) // ConvertMIDItoCsound is always false in the console version
		WriteToFile(NO,CsoundFileFormat,"s",CsRefNum); // This line will automatically be deleted if this score belongs to a sound-object prototype — see function fix_csound_score() in prototype.php
#endif

	if(cswrite) {
		WriteToFile(NO,CsoundFileFormat,";",CsRefNum);
		for(instr_index = 0; instr_index < Jinstr; instr_index++) {
			h = (Handle) p_keyinstrument[instr_index];
			MyDisposeHandle((Handle*)&h);
			}
		}
	for(ch = 0; ch < MAXCHAN; ch++) {
		h = (Handle) p_keychannel[ch];
		MyDisposeHandle((Handle*)&h);
		h = (Handle) p_last_timeon[ch];
		MyDisposeHandle((Handle*)&h);
		h = (Handle) p_t2cont[ch];
		MyDisposeHandle((Handle*)&h);
		h = (Handle) p_seqcont[ch];
		MyDisposeHandle((Handle*)&h);
		h = (Handle) p_active[ch];
		MyDisposeHandle((Handle*)&h);
		}
	for(nseq=0; nseq < maxconc; nseq++) {
		h = (Handle) (*((*pp_currentparams)[nseq]))->params;
		MyDisposeHandle((Handle*)&h);
		h = (Handle) (*pp_currentparams)[nseq];
		MyDisposeHandle((Handle*)&h);
		h = (Handle) (*p_control)[nseq].param;
		MyDisposeHandle((Handle*)&h);
		}
	MyDisposeHandle((Handle*)&pp_currentparams);
	MyDisposeHandle((Handle*)&p_control);
	MyDisposeHandle((Handle*)&p_onoff);
	MyDisposeHandle((Handle*)&p_silence);
	MyDisposeHandle((Handle*)&p_inext);
	MyDisposeHandle((Handle*)&p_inext1);
	MyDisposeHandle((Handle*)&p_istartperiod);
	MyDisposeHandle((Handle*)&p_iendperiod);
	MyDisposeHandle((Handle*)&p_icycle);
	MyDisposeHandle((Handle*)&p_periodgap);
	MyDisposeHandle((Handle*)&p_nextd);
	MyDisposeHandle((Handle*)&p_t1);
	MyDisposeHandle((Handle*)&p_currmapped);
	SoundOn = FALSE;
	if(!MIDIfileOn && !cswrite && rtMIDI && !showpianoroll) {
		drivertime = getClockTime();
		if(!FirstTime) ComputeStart = drivertime;
		}
	if(showpianoroll) Tcurr = oldtcurr;
	Interrupted = FALSE;
	return(result);
	}

// =========================================================
// Subroutines of MakeSound()
// =========================================================

// note 41
int SendMIDIevent(int type,int status,int data1,int data2,int kinstance,int scale,int blockkey,Milliseconds date,int nseq,int *p_rs) {
	// Send an event to the MIDI driver (real-time MIDI, MIDI file, event list) at 'date' (ms).
	// 'type' is NORMAL_EVENT (3 bytes), TWO_BYTE_EVENT (status + data2) or RAW_EVENT (a single byte in 'status' and 'data2').
	// 'kinstance', 'scale' and 'blockkey' are only needed for NoteOn/NoteOff (microtonality).
	MIDI_Event e;

	e.time = Tcurr;	// Not used by SendToDriver(), which takes 'date'
	e.type = type;
	e.status = status;
	e.data1 = data1;
	e.data2 = data2;
	return SendToDriver(kinstance,scale,blockkey,date,nseq,p_rs,&e);
	}

int SendControlChange(int chan,int control,int value,Milliseconds date,int nseq,int *p_rs) {
	// Send value 'value' of controller 'control' on MIDI channel 'chan' (0..15)
	return SendMIDIevent(NORMAL_EVENT,ControlChange + chan,control,value,0,0,0,date,nseq,p_rs);
	}

int SendNoteOffAndDraw(int key,int chan,int kinstance,int scale,int blockkey,Milliseconds date,int nseq,int *p_rs,int showpianoroll,PianoRoll *p_roll,long **p_timeon,int forgettimeon) {
	// Send a NoteOff of 'key' on MIDI channel 'chan' at 'date', and draw the note on the pianoroll from the date
	// of its NoteOn, '(*p_timeon)[key]'. If 'forgettimeon', this date is then reset.
	int result;

	if((result=SendMIDIevent(NORMAL_EVENT,NoteOff + chan,key,0,kinstance,scale,blockkey,date,nseq,p_rs)) != OK) return(result);
	if(!showpianoroll) return(OK);
	if(trace_makesound)
		BPPrintMessage(1,odInfo,"* DrawPianoNote() key = %d start = %ld end = %ld nseq = %d\n",key,(*p_timeon)[key],(long)date,nseq);
	result = DrawPianoNote("midi",key,chan,(*p_timeon)[key],date,p_roll->noteoffset,p_roll->topoffset,p_roll->hrect,p_roll->minkey,p_roll->maxkey,&(p_roll->graphrect));
	if(result != OK) return(result);
	if(forgettimeon) (*p_timeon)[key] = 0L;
	return(OK);
	}

// note 42
int SendInitialControls(int kinstance,int j,int chan,int nseq,int maxconc,ContinuousControl **p_control,char ***p_active,int cswrite,Milliseconds date,int *p_rs) {
	// At the start of instance 'kinstance' of object j, send on MIDI channel 'chan' its volume, panoramic, pitchbend,
	// pressure and modulation if they differ from the current values of the channel. A parameter that is being varied
	// by a stream (continuous control) on this channel is not sent. With 'cswrite', only the current values are updated.
	int n,ch,okvolume,okpanoramic,okpitchbend,okpressure,okmodulation,volume,panoramic,pressure,modulation,control,lsb,msb,result;
	double pitchbend;
	ParameterStream **stream;

	okvolume = okpanoramic = okpitchbend = okpressure = okmodulation = TRUE;
	for(n = 0; !cswrite && n < maxconc; n++) {
		stream = (*p_control)[n].param;
		if(stream == NULL) {
			BPPrintMessage(0,odError,"=> Error stream = NULL, n = %ld\n",(long)n);
			return(ABORT); // Fixed by BB 2021-02-25
			}
		ch = (*stream)[IVOLUME].channel;
		if(chan == ch && (*(p_active[ch]))[IVOLUME]) okvolume = FALSE;
		ch = (*stream)[IPANORAMIC].channel;
		if(chan == ch && (*(p_active[ch]))[IPANORAMIC]) okpanoramic = FALSE;
		ch = (*stream)[IPITCHBEND].channel;
		if(chan == ch && (*(p_active[ch]))[IPITCHBEND]) okpitchbend = FALSE;
		ch = (*stream)[IPRESSURE].channel;
		if(chan == ch && (*(p_active[ch]))[IPRESSURE]) okpressure = FALSE;
		ch = (*stream)[IMODULATION].channel;
		if(chan == ch && (*(p_active[ch]))[IMODULATION]) okmodulation = FALSE;
		}
	volume = (int) VolumeStart(kinstance);
	if((control=(*((*p_Instance)[kinstance].contparameters.values))[IVOLUME].control) > -1)
		volume = ParamValue[control];
	panoramic = (int) PanoramicStart(kinstance);
	if((control=(*((*p_Instance)[kinstance].contparameters.values))[IPANORAMIC].control) > -1)
		panoramic = ParamValue[control];
	pitchbend = PitchbendStart(kinstance);
	pressure = (int) PressureStart(kinstance);
	modulation = (int) ModulationStart(kinstance);

	if(okvolume && (volume != (*p_Oldvalue)[chan].volume) && (j >= 16384 || (*p_OkVolume)[j])) {
		(*p_Oldvalue)[chan].volume = volume;
		ChangedVolume[chan] = TRUE;
		// Since the value is 0..127 we send only the MSB
		if(!cswrite && (result=SendControlChange(chan,VolumeControl[chan+1],volume,date,nseq,p_rs)) != OK) return(result);
		}
	if(okpanoramic && panoramic != (*p_Oldvalue)[chan].panoramic && (j >= 16384 || (*p_OkPan)[j])) {
		(*p_Oldvalue)[chan].panoramic = panoramic;
		ChangedPanoramic[chan] = TRUE;
		// Since the value is 0..127 we send only the MSB
		if(!cswrite && (result=SendControlChange(chan,PanoramicControl[chan+1],panoramic,date,nseq,p_rs)) != OK) return(result);
		}
	if(okpitchbend && pitchbend != (*p_Oldvalue)[chan].pitchbend && pitchbend >= 0.) {
		(*p_Oldvalue)[chan].pitchbend = pitchbend;
		ChangedPitchbend[chan] = TRUE;
		lsb = ((long)pitchbend) % 128;
		msb = (((long)pitchbend) - lsb) >> 7;
		if(!cswrite && (result=SendMIDIevent(NORMAL_EVENT,PitchBend + chan,lsb,msb,0,0,0,date,nseq,p_rs)) != OK) return(result);
		}
	if(okpressure && pressure != (*p_Oldvalue)[chan].pressure) {
		(*p_Oldvalue)[chan].pressure = pressure;
		ChangedPressure[chan] = TRUE;
		if(!cswrite && (result=SendMIDIevent(TWO_BYTE_EVENT,ChannelPressure + chan,0,pressure,0,0,0,date,nseq,p_rs)) != OK) return(result);
		}
	if(okmodulation && modulation != (*p_Oldvalue)[chan].modulation) {
		(*p_Oldvalue)[chan].modulation = modulation;
		ChangedModulation[chan] = TRUE;
		lsb = ((long)modulation) % 128;
		msb = (((long)modulation) - lsb) >> 7;
		if(!cswrite) {
			if((result=SendControlChange(chan,1,msb,date,nseq,p_rs)) != OK) return(result);
			if((result=SendControlChange(chan,33,lsb,date,nseq,p_rs)) != OK) return(result);
			}
		}
	return(OK);
	}

// note 43
int SendSwitches(int kinstance,unsigned int *currswitchstate,int cswrite,Milliseconds date,int nseq,int *p_rs) {
	// Send the switches (controllers 64 to 69, including the hold pedal) of instance 'kinstance' on channels where they
	// have changed. 'currswitchstate[ch]' holds the current state of channel ch, one bit per switch.
	// If 'PedalReleaseDefault', switches that remain on are released and pressed again.
	int ch,jj,s,result;

	if((*p_ObjectSpecs)[kinstance] == NULL || SwitchState(kinstance) == NULL) return(OK);
	Tcurr = date / Time_res;
	for(ch = 0; ch < MAXCHAN; ch++) {
		if(currswitchstate[ch] != (*(SwitchState(kinstance)))[ch]) {
			currswitchstate[ch] = (*(SwitchState(kinstance)))[ch];
			ChangedSwitch[ch] = TRUE;
			for(jj = 0; jj < 6; jj++) {
				s = ((currswitchstate[ch] >> jj) & 1) * 127; // Fixed 2026-01-10
				if(s < 0 || s > 127) {
					BPPrintMessage(0,odError,"=> Err. 1 MakeSound(). channel %d, jj = %d, currswitchstate = %d, s = %d\n",ch,jj,currswitchstate[ch],s);
					s = 0;
					}
				*p_rs = 0;
				if(!cswrite && (result=SendControlChange(ch,64 + jj,s,date,nseq,p_rs)) != OK) return(result);
				}
			}
		else if(!cswrite && PedalReleaseDefault && (*(SwitchState(kinstance)))[ch] > 0) {
			// Action off and on every switch if it was on
			for(jj = 0; jj < 6; jj++) {
				s = ((currswitchstate[ch] >> jj) & 1) * 127; // Fixed 2026-01-10
				if(s < 0 || s > 127) {
					BPPrintMessage(0,odError,"=> Err. 2 MakeSound(). channel %d, jj = %d, currswitchstate = %d, s = %d\n",ch,jj,currswitchstate[ch],s);
					s = 0;
					}
				if(s > 0) {
					*p_rs = 0;
					if((result=SendControlChange(ch,64 + jj,0,date,nseq,p_rs)) != OK) return(result);
					*p_rs = 0;
					if((result=SendControlChange(ch,64 + jj,s,date,nseq,p_rs)) != OK) return(result);
					}
				}
			}
		}
	return(OK);
	}

// note 44
int SendDilationRatio(int j,double beta,int objectchannel,int cswrite,Milliseconds date,int nseq,int *p_rs) {
	// If the prototype of sound-object j asks for it, send its dilation ratio 'beta' to a MIDI controller
	int alph,alphach;

	if(j >= Jbol || j <= 1 || !(*p_AlphaCtrl)[j] || (*p_AlphaCtrlChan)[j] > MAXCHAN || (*p_AlphaCtrlNr)[j] >= MAXPARAMCTRL)
		return(OK);
	if(trace_makesound) BPPrintMessage(0,odInfo,"Send dilation ratio to MIDI\n");
	alph = (int) (32. * log10(beta) + 64.);	// This converts beta in range [0.01,100] to [0,127]
	if(alph < 0) alph = 0;
	if(alph > 127) alph = 127;
	alphach = (*p_AlphaCtrlChan)[j] - 1;
	if(alphach == 0) alphach = objectchannel;
	if(alphach < 0) alphach = 0;
	Tcurr = date / Time_res;
	if(cswrite) return(OK);
	return SendControlChange(alphach,(*p_AlphaCtrlNr)[j],alph,date,nseq,p_rs); // Maybe WRONG if AlphaCtrlNr < 64
	}

// note 43
int ReleaseKeysAndSwitches(char ***p_keychannel,unsigned int *currswitchstate,int *p_rs) {
	// At the end of the item, send a NoteOff (NoteOn with velocity 0) for every key still sounding,
	// and switch off the switches that are on
	int ch,key,jj;

	for(ch = 0; ch < MAXCHAN; ch++) {
		*p_rs = 0;
		for(key = 0; key < MAXKEY; key++) {
			if((*p_keychannel[ch])[key] > 0) {
				if(SendMIDIevent(NORMAL_EVENT,NoteOn + ch,key,0,0,0,0,Tcurr * Time_res,0,p_rs) != OK) return(ABORT);
				}
			}
		*p_rs = 0;
		if(currswitchstate[ch] != 0) {
			for(jj = 0; jj < 6; jj++) {
				if(currswitchstate[ch] & (1L << jj)) {
					if(SendControlChange(ch,64 + jj,0,Tcurr * Time_res,0,p_rs) != OK) return(ABORT);
					}
				}
			}
		}
	return(OK);
	}

// note 44
void TuneMIDIdevice(int *p_rs) {
	// Set the diapason (A4freq) of the MIDI device on all channels
	int chan,semitones;
	double cents;

	cents = 1200 * (log(A4freq/440.) / log(2.));
	BPPrintMessage(0,odInfo,"👉 Trying to tune the MIDI device to %.2f Hz, i.e. %d cents from 440 Hz\n",A4freq,(int)cents);
	semitones = (int)(cents / 100);		// Calculate the number of semitones
	cents -= semitones * 100;			// Adjust cents to be within one semitone
	if(cents < 0) {
		semitones -= 1;		// Move down one more semitone
		cents += 100;		// Adjust cents to be positive within this lower semitone
		}
	if(semitones != 0) BPPrintMessage(0,odInfo,"... which means %d semitone(s) lower, then up +%d cents\n",(-semitones),(int)cents);
	for(chan = 0; chan < MAXCHAN; chan++) {
		SendControlChange(chan,101,0,0,0,p_rs);	// RPN MSB, set to 0
		SendControlChange(chan,100,2,0,0,p_rs);	// RPN LSB, set to 2 for A4 tuning
		// Set A4 tuning
		SendControlChange(chan,6,(semitones >= 0) ? semitones : (127 + semitones + 1),0,0,p_rs);	// Data Entry MSB
		SendControlChange(chan,38,(int) cents,0,0,p_rs);	// Data Entry LSB
		}
	}

// note 44
void SetUpMPEreceiver(int *p_rs) {
	// Not used (see MakeSound()). Set up a MIDI receiver for MPE microtonality.
	int chan;
	Milliseconds time;

	time = 0;
	// Set receiver to Mode 3 ("Poly)
	time += 10;
	SendControlChange(0,126,0,time,0,p_rs);

	// MPE Configuration, see page 20 of MPE specs
	// This doesn't seem to work on PianoTeq STAGE
	BPPrintMessage(0,odInfo,"👉 Trying to set up MPE with a lower zone using member channels 2-16\n");
	BPPrintMessage(0,odInfo,"... This is probably not necessary\n");
	SendControlChange(0,101,0,time,0,p_rs);
	SendControlChange(0,100,6,time,0,p_rs);
	SendControlChange(0,6,15,time,0,p_rs);

	// Not sure this is necessary. It creates unwanted noises when improvising -gr.Mozart
	time += 10;
	// Set PitchBend sensitivity to ± 2 semitones on all member channels
	BPPrintMessage(0,odInfo,"👉 Setting pitchbend sensitivity to ± 2 semitones on channels 2-16\n");
	for(chan = 2; chan < MAXCHAN; chan++) {
		SendControlChange(chan,101,0,time,0,p_rs);
		SendControlChange(chan,100,0,time,0,p_rs);
		SendControlChange(chan,6,2,time,0,p_rs);	// Data Entry MSB: 2 semitones
	/*	SendControlChange(chan,38,0,time,0,p_rs);	// Data Entry LSB: 0 cents */
		}
	WaitABit(10); // Sleep for 10 milliseconds
	}

// note 45
int WriteCsoundTempo(Milliseconds torigin) {
	// Write the tempo statement "t 0 60" at the start of the Csound score, 'torigin' being the date (ms) of the start of the item
	double p,q;
	int result;

	while(TRUE) {
		if(Pclock > 0.) {	// Striated or measured smooth time
			if(Simplify((double)INT_MAX,60L*Qclock,Pclock,&p,&q) != OK)
				Simplify((double)INT_MAX,Qclock,floor(Pclock/60L),&p,&q);
			my_sprintf(Message,"t %.3f 60",((double) torigin) * Qclock / ((double) Pclock) / 1000.);
			}
		else {	// Unmeasured smooth time
			my_sprintf(Message,"t %.3f 60",((double) torigin) / 1000.);
			}
		if(ConvertMIDItoCsound) {
			Println(wPrototype7,Message);
			return(OK);
			}
		if(CsoundTrace) ShowMessage(TRUE,wMessage,Message);
		// With several items, only the first one gets a tempo statement
		// Without sections (CSOUND_SECTIONS = 0), the score gets a single tempo statement
		if((!CSOUND_SECTIONS || Improvize || PlayAllChunks || AllItems) && AssignedTempoCsoundFile) return(OK); // Fixed !PlayAllChunks 2024-05-20
		AssignedTempoCsoundFile = TRUE;
		if(WriteToFile(NO,CsoundFileFormat,Message,CsRefNum) == OK) return(OK);
		BPPrintMessage(0,odError,"=> Couldn't write to the Csound score. Maybe it has been closed by another application\n");
		CloseCsScore();
		if((result=PrepareCsFile()) != OK) return(result);
		}
	}

// note 46
double ObjectPreRoll(int j,double beta) {
	// Pre-roll (ms) of sound-object j played with dilation ratio 'beta'
	if(beta == 0.) return(0.);
	if((*p_PreRollMode)[j] == FIXVALUE) return((double) (*p_PreRoll)[j]);
	return(beta * (*p_PreRoll)[j]);
	}

double ObjectPostRoll(int j,double beta) {
	// Post-roll (ms) of sound-object j played with dilation ratio 'beta'
	if(beta == 0.) return(0.);
	if((*p_PostRollMode)[j] == FIXVALUE) return((double) (*p_PostRoll)[j]);
	return(beta * (*p_PostRoll)[j]);
	}

double ObjectPivot(int j,double beta) {
	// Location (ms) of the pivot of sound-object j played with dilation ratio 'beta'
	if(beta == 0.) return(0.);
	if((*p_PivMode)[j] == FIXVALUE) return((double) (*p_PivPos)[j]);
	return(beta * (*p_PivPos)[j]);
	}

// note 46
int StrikeAgainOf(int j) {
	// Whether a NoteOn on a key that is already sounding sends a NoteOff first: setting of sound-object j, or else of the project
	switch((*p_StrikeAgain)[j]) {
		case TRUE: return(TRUE);
		case FALSE: return(FALSE);
		}
	return(StrikeAgainSettings);
	}

// note 46
Milliseconds EventDelay(int cswrite,int j,long i) {
	// Delay (ms, before dilation) between events i-1 and i of sound-object j: in its Csound score if it is played, else in its MIDI codes
	if(PlayCsoundLine(cswrite,j)) return((*((*pp_CsoundTime)[j]))[i]);
	return((*((*pp_MIDIcode)[j]))[i].time);
	}

// note 47
long FirstEventAfterTruncation(int k,int j,int cswrite,double preroll,Milliseconds *p_gap) {
	// The beginning of instance k of sound-object j is truncated. Return the index of its first NoteOn at or after
	// the truncation, and in '*p_gap' the time (ms) between them. Return the number of events if there is none.
	long im,ievent;
	int c0,c1,c2;
	Milliseconds this_date;
	char label_note[15];

	im = Find_im(cswrite,j);
	this_date = ZERO;
	if(preroll < ZERO) this_date = - preroll;
	c0 = NoteOn;
	c1 = 0;
	for(ievent = 0; ievent < im; ievent++) {
		if(!PlayCsoundLine(cswrite,j)) {
			c0 = (*((*pp_MIDIcode)[j]))[ievent].byte;
			c0 = c0 - c0 % 16;
			if(ievent < (im-2)) {
				c1 = (*((*pp_MIDIcode)[j]))[ievent+1].byte;
				c2 = (*((*pp_MIDIcode)[j]))[ievent+2].byte;
				}
			else c2 = 0;
			if(trace_makesound) {
				if(c0 == NoteOn) PrintThisNote(-1,c1,0,-1,label_note);
				else strcpy(label_note,"");
				BPPrintMessage(1,odInfo,"[%d] date %ld => %d %d %d « %s » time = %ld\n",ievent,this_date,c0,c1,c2,label_note,(*((*pp_MIDIcode)[j]))[ievent].time);
				}
			if(c2 == 0) c0 = 0; // NoteOn with zero velocity is actually a NoteOff
			}
		if(c0 == NoteOn && this_date >= (*p_Instance)[k].truncbeg) {
			*p_gap = this_date - (*p_Instance)[k].truncbeg;
			if(trace_makesound) BPPrintMessage(1,odInfo,"@@ this_date = %ld, ievent = %ld, truncbeg of this instance = %ld, this_gap = %ld, k = %d\n",this_date,ievent,(*p_Instance)[k].truncbeg,*p_gap,k);
			return(ievent);
			}
		this_date += EventDelay(cswrite,j,ievent);
		}
	return(im);
	}

// note 48
int NextInstance(long kmax,Milliseconds **p_nextd,char **p_silence,int oldk,Milliseconds *p_t2obj) {
	// Return the sound-object instance whose next event comes first (0 if none), and in '*p_t2obj' the date of this event.
	// If 'p_silence' is not NULL, instances found on the way, other than 'oldk', are no longer silenced.
	int k,knext;

	knext = 0;
	*p_t2obj = Infpos;
	for(k = 2; k <= kmax; k++) {
		if((*p_Instance)[k].object == 0) continue;
		if((*p_nextd)[k] < *p_t2obj) {
			knext = k;
			if(p_silence != NULL && k != oldk) (*p_silence)[k] = FALSE;
			*p_t2obj = (*p_nextd)[k];
			}
		if(trace_makesound)
			BPPrintMessage(0,odInfo,"+nextd[%d] = %ld, t2obj = %ld, object = %d\n",k,(*p_nextd)[k],*p_t2obj,(*p_Instance)[k].object);
		}
	return(knext);
	}

// note 48
int NextControlDate(char ***p_active,Milliseconds ***p_t2cont,Milliseconds *p_t2,int *p_icont,int *p_chancont) {
	// Look for an active continuous control whose next message comes at or before '*p_t2'. If there is one, return TRUE,
	// with its date in '*p_t2', its parameter in '*p_icont' and its channel in '*p_chancont' (otherwise both are -1).
	int ch,index,found;

	found = FALSE;
	*p_icont = *p_chancont = -1;
	for(ch = 0; ch < MAXCHAN; ch++) {
		for(index = 0; index <= IPANORAMIC; index++) {
			if(!(*(p_active[ch]))[index]) continue;
			if(*p_t2 >= (*(p_t2cont[ch]))[index]) {
				*p_t2 = (*(p_t2cont[ch]))[index];
				*p_icont = index;
				*p_chancont = ch;
				found = TRUE;
				}
			}
		}
	return(found);
	}

// note 18
int ExecuteScriptList(p_list **scriptlist) {
	int x,r,idummy,check;
	p_list **ptag;
	long jdummy;

	ptag = scriptlist;
	r = OK;
	check = 0; // This will create a script entry
	do {
		x = (**ptag).x;
		ptag = (**ptag).p;
	//	BPPrintMessage(0,odInfo,"ExecScriptLine in ExecuteScriptList\n");
		if((r=ExecScriptLine(NULL,wScript,check,TRUE,(*p_Script)[x],jdummy,&jdummy,
				&idummy,&idummy)) != OK) goto SORTIR;
		}
	while(ptag != NULL);

	SORTIR:
	return(r);
	}

// note 29
int ClipVelocity(int v,int localvelocity,int control,int rndvel) {
	int r;
	float x;

	if(control >= MAXPARAMCTRL) {
		my_sprintf(Message,"=> Err. ClipVelocity(). control = %ld",(long)control);
	//	BPPrintMessage(0,odError,"%s",Message);
		control = -1;
		}
	// BPPrintMessage(0,odInfo,"@@@ v = %d, localvelocity = %d, control = %d, rndvel = %d\n",v,localvelocity,control,rndvel);
	if(control > -1) v = ParamValue[control];
	if(rndvel > 0) {
		if(rndvel > 127) r = ParamValue[rndvel-128];
		else r = rndvel;
		x = (2. * bp3_rand() * ((float)r)) / ((float)BP3_RAND_MAX);
		if(x > r) x = r - x;
		UsedRandom = TRUE;
		localvelocity += x;
		if(localvelocity < 1) localvelocity = 1;
		}
	v = (v * localvelocity) / 127;
	if(v > 127) v = 127;
	return(v);
	}


// note 49
int ChannelConvert(int ch) {
	int x;

	if(ch < 0) ch += 256;
	if(ch < 128) {
		if(ch > MAXCHAN) {
			my_sprintf(Message,"=> Trying to assign channel #%ld.\nValue should be 1..%ld",
				(long)ch,(long)MAXCHAN);
		//	BPPrintMessage(0,odError,"%s",Message);
			return(ABORT);
			}
		return(ch);	/* Fixed channel */
		}
	/* Channel is determined by Kx */
	x = ch - 128;
	if(x < 1 || x >= MAXPARAMCTRL) {
		my_sprintf(Message,"=> Trying to fix channel with incorrect K%ld.\nValue should be 1..%ld",
			(long)x,(long)MAXPARAMCTRL-1L);
		// BPPrintMessage(0,odError,"%s",Message);
		return(ABORT);
		}
	ch = ParamValue[x];
	if(ch < 1 || ch > MAXCHAN) {
		if(ParamControl[x] >= 0) {
			my_sprintf(Message,"=> Trying to assign channel #%ld by K%ld (MIDI controller #%ld).\nValue should be 1..%ld",
				(long)ch,(long)x,(long)ParamControl[x],(long)MAXCHAN);
			}
		else {
			if(ParamKey[x] >= 0) {
				my_sprintf(Message,"=> Trying to assign channel #%ld by K%ld (Key #%ld).\nValue should be 1..%ld",
					(long)ch,(long)x,(long)ParamKey[x],(long)MAXCHAN);
				}
			else {
				my_sprintf(Message,"=> Trying to assign channel #%ld by K%ld.\nValue should be 1..%ld",
					(long)ch,(long)x,(long)MAXCHAN);
				}
			}
	//	BPPrintMessage(0,odError,"%s",Message);
		return(ABORT);
		}
	return(ch);
	}
								
								
double ContinuousParameter(Milliseconds time_ms,int paramseq,ControlStream **p_stream) {
	double param;

	param = (*p_stream)[paramseq].startvalue
			+ ((double)(*p_stream)[paramseq].difference * (time_ms - (*p_stream)[paramseq].starttime))
				/ (*p_stream)[paramseq].dur;
	return(param);
	}


// note 50
long Findibm(int index,Milliseconds dur,int chan) {
	int rate;
	long x;

	switch(index) {
		case IMODULATION:
			rate = ModulationRate[chan+1]; break;
		case IPITCHBEND:
			rate = PitchbendRate[chan+1]; break;
		case IPRESSURE:
			rate = PressRate[chan+1]; break;
		case IVOLUME:
			rate = VolumeRate[chan+1]; break;
		case IPANORAMIC:
			rate = PanoramicRate[chan+1]; break;
		default:
			return(1L);
		}
	x = ((unsigned long)(dur * rate)) / 1000L;
	return(x);
	}


// note 50
int SendControl(ContinuousControl **p_control,Milliseconds t0,int chan,int iparam,int maxconc,
	int cswrite,int showpianoroll,int *p_rs,char ***p_active,Milliseconds ***p_t2cont,
	int ***p_seqcont,MIDIcontrolstatus **p_Oldvalue,PerfParameters ****pp_currentparams) {
	int seq,lsb,msb,nn,result;
	double value,alpha;
	MIDI_Event e;
	Milliseconds time,t2;
	ParameterStream **param;

	result = OK;
	if(iparam < 0 || iparam > IPANORAMIC) {
	//	BPPrintMessage(0,odError,"=> Err. SendControl(). iparam < 0 || iparam > IPANORAMIC");
		return(ABORT);
		}
	if(chan < 0 || chan >= MAXCHAN) {
		// BPPrintMessage(0,odError,"=> Err. SendControl(). chan < 0 || chan >= MAXCHAN");
		result = ABORT;
		goto OVER3;
		}
	seq = (*(p_seqcont[chan]))[iparam];

	if(seq < 0 || seq >= maxconc) {
		// BPPrintMessage(0,odError,"=> Err. SendControl(). seq < 0 || seq >= maxconc");
		return(ABORT);
		}
		
	param = (*p_control)[seq].param;

	if(chan != (*param)[iparam].channel) {
		// BPPrintMessage(0,odError,"=> Err. SendControl(). chan != (*param)[iparam].channel");
		result = ABORT;
		goto OVER3;
		}
	if(!(*(p_active[chan]))[iparam]) {
		// BPPrintMessage(0,odError,"=> Err. SendControl(). !(*(p_active[chan]))[iparam]");
		return(ABORT);
		}
	if((*p_control)[seq].param == NULL) {
		// BPPrintMessage(0,odError,"=> Err. SendControl(). (*p_control)[seq].param == NULL");
		return(ABORT);
		}
	if((*param)[iparam].ibm <= ZERO) {
		Print(wTrace,"=> Err. SendControl(). (*param)[iparam].ibm <= ZERO\n");
	/*	This case was found in item #26 of -da.checkControls.html when played after the
		two preceding ones */
		goto INCREMENT;
		}

	alpha = ((double)(*param)[iparam].ib) / (*param)[iparam].ibm;

	if((value = GetTableValue(alpha,(*param)[iparam].imax,(*param)[iparam].point,
			(*param)[iparam].startvalue,(*param)[iparam].endvalue)) == Infpos) {
		BPPrintMessage(0,odError,"=> Err. SendControl(). value == Infpos\n");
		return(ABORT);
		}
	if(value < 0. && value > -0.1) value = 0.;
	if(value > 16383. && value < 16384.) value = 16383.;

	if(value < 0. || value > 16383.) {
		my_sprintf(Message,"=> Err. SendControl(). value = %.3f\n",value);
		Print(wTrace,Message);
		BPPrintMessage(0,odError,"=> Err. SendControl(). value = %.3f\n",value);
	return(ABORT);
		}

	time = t0 + (*param)[iparam].starttime
		+ ((long)(*param)[iparam].duration
		* (*param)[iparam].ib) / (*param)[iparam].ibm;

	Tcurr =  time / Time_res;

	// BPPrintMessage(0,odInfo,"SendControl(). value = %.3f Tcurr = %ld\n",value,(long)Tcurr);

	lsb = ((long)value) % 128;
	msb = (((long)value) - lsb) >> 7;

	switch(iparam) {
		case IPITCHBEND:
			ChangedPitchbend[chan] = TRUE;
			(*p_Oldvalue)[chan].pitchbend = value;
			if(!cswrite) {
				e.time = Tcurr;
				e.type = NORMAL_EVENT;
				e.status = PitchBend + chan;
				e.data1 = lsb;
				e.data2 = msb;
				if((result=SendToDriver(0,0,0,time,seq,p_rs,&e)) != OK) goto OVER3;
				}
			break;
		case IPRESSURE:
			ChangedPressure[chan] = TRUE;
			(*p_Oldvalue)[chan].pressure = value;
			if(!cswrite) {
				e.time = Tcurr;
				e.type = TWO_BYTE_EVENT;
				e.status = ChannelPressure + chan;
				e.data1 = 0;
				e.data2 = value;
		//		BPPrintMessage(1,odInfo,"@@ Sending pressure = %d time = %ld\n",value,(long)Tcurr);
				if((result=SendToDriver(0,0,0,time,seq,p_rs,&e)) != OK) goto OVER3;
				}
			break;
		case IMODULATION:
			ChangedModulation[chan] = TRUE;
			(*p_Oldvalue)[chan].modulation = value;
			if(!cswrite) {
				e.time = Tcurr;
				e.type = NORMAL_EVENT;
				e.status = ControlChange + chan;
				e.data1 = 1;
				e.data2 = msb;
				if((result=SendToDriver(0,0,0,time,seq,p_rs,&e)) != OK) goto OVER3;
				e.time = Tcurr;
				e.type = NORMAL_EVENT;
				e.status = ControlChange + chan;
				e.data1 = 33;
				e.data2 = lsb;
				if((result=SendToDriver(0,0,0,time,seq,p_rs,&e)) != OK) goto OVER3;
				}
			break;
		case IVOLUME: // Later consider that it also may have a LSB
			ChangedVolume[chan] = TRUE;
			(*p_Oldvalue)[chan].volume = value;
			if(!cswrite) {
				e.time = Tcurr;
				e.type = NORMAL_EVENT;
				e.status = ControlChange + chan;
				e.data1 = VolumeControl[chan+1];
				e.data2 = value;
				if((result=SendToDriver(0,0,0,time,seq,p_rs,&e)) != OK) goto OVER3;
				}
			break;
		case IPANORAMIC: // Later consider that it also may have a LSB
			ChangedPanoramic[chan] = TRUE;
			(*p_Oldvalue)[chan].panoramic = value;
			if(!cswrite) {
				e.time = Tcurr;
				e.type = NORMAL_EVENT;
				e.status = ControlChange + chan;
				e.data1 = PanoramicControl[chan+1];
				e.data2 = value;
				if((result=SendToDriver(0,0,0,time,seq,p_rs,&e)) != OK) goto OVER3;
				}
			break;
		}

	(*param)[iparam].ib++;
	(*(p_t2cont[chan]))[iparam] = t2 = (*param)[iparam].starttime
		+ ((*param)[iparam].duration * (*param)[iparam].ib)
		/ (*param)[iparam].ibm;

	INCREMENT:
	if((*param)[iparam].ib > (*param)[iparam].ibm) {
		(*param)[iparam].ibm = -1L;
		(*(p_active[chan]))[iparam] = FALSE;
		(*(p_t2cont[chan]))[iparam] = Infpos;
	//	BPPrintMessage(0,odInfo,"@&&  chan = %d, iparam = %d\n",chan,iparam);
		
		// Try to activate the same parameter+channel in another sequence
		for(nn = 0; nn < maxconc; nn++) {
			if(nn == seq) continue;
			if((*((*p_control)[nn].param))[iparam].ibm <= ZERO) continue;
			if(chan == (*((*p_control)[nn].param))[iparam].channel) {
				if((*(p_active[chan]))[iparam]) {
					// BPPrintMessage(0,odError,"=> Err. SendControl(). (*(p_active[chan]))[iparam]");
					return(ABORT);
					}
				
				/* Set the proper ib using value of t2 */
				if((*((*p_control)[nn].param))[iparam].duration <= ZERO) {
					// BPPrintMessage(0,odError,"=> Err. SendControl(). (*((*p_control)[nn].param))[iparam].duration <= 0");
					(*((*p_control)[nn].param))[iparam].ib = ZERO;
					goto OVER3;
					}
				(*((*p_control)[nn].param))[iparam].ib
					= (t2 - (*((*p_control)[nn].param))[iparam].starttime)
						* (*((*p_control)[nn].param))[iparam].ibm
						/ (*((*p_control)[nn].param))[iparam].duration;
				if((*((*p_control)[nn].param))[iparam].ib
						<= (*((*p_control)[nn].param))[iparam].ibm) {
					(*(p_t2cont[chan]))[iparam] = t2;
					(*(p_seqcont[chan]))[iparam] = nn;
					(*(p_active[chan]))[iparam] = TRUE;
					}
				else {
					(*((*p_control)[nn].param))[iparam].ibm = -1L;
		/*			(*(p_active[chan]))[iparam] = FALSE; */
					}
				break;
				}
			}
		}

OVER3:
	return(result);
	}

// note 50
double GetTableValue(double alpha,long imax,Coordinates** coords,double startvalue,double endvalue) {
	double x,y,y1,y2;
	long i,xmax,x1,x2;

	if(alpha < 0.) {
		// if(Beta && alpha < -0.01) BPPrintMessage(0,odError,"=> Err. GetTableValue(). alpha < -0.01");
		alpha = 0.;
		}
	if(alpha > 1.) {
		// if(Beta && alpha > 1.01) BPPrintMessage(0,odError,"=> Err. GetTableValue(). alpha > 1.01");
		alpha = 1.;
		}
	if(imax == ZERO) {
		/* No table: just interpolate */
		y = startvalue + alpha * (endvalue - startvalue);
		return(y);
		}

	if(imax < ZERO) {
		// BPPrintMessage(0,odError,"=> Err. GetTableValue(). imax < ZERO || imax > 255");
		return(0.);
		}
	if(coords == NULL) {
		// BPPrintMessage(0,odError,"=> Err. GetTableValue(). coords == NULL");
		return(0.);
		}
	if(imax > MyGetHandleSize((Handle)coords) / sizeof(Coordinates)) {
	//	BPPrintMessage(0,odError,"=> Err. GetTableValue(). imax >= MyGetHandleSize((Handle)coords) / sizeof(Coordinates)");
		return(0.);
		}
	xmax = (*(coords))[imax-1L].i;
	x = alpha * (double) xmax;
	i = 0; while((*(coords))[i].i < x) i++;
	if(i == 0) y = (*(coords))[ZERO].value;
	else {
		x2 = (*(coords))[i].i;
		x1 = (*(coords))[i-1L].i; 
		y2 = (*(coords))[i].value;
		y1 = (*(coords))[i-1L].value;
		if(x1 >= x2) {
		//	BPPrintMessage(0,odError,"=> Err. GetTableValue(). x1 >= x2");
			return(Infpos);
			}
		y = y1 + (y2 - y1) * (x - x1) / (x2 - x1);
		}
	return(y);
	}


// note 25
int GoodKey(int j) {
	int key;
	key = j - 16384;
	if(key < 0 || key > 127) {
		BPPrintMessage(0,odError, "=> A MIDI key is out of range: key = %ld\n",(long)key);
		while(key < 0) key += 12;
		while(key > 127) key -= 12;
		}
	return(key);
	}


// note 51
int StoreMappedKey(int orgkey,int imagekey,int k,int channel,
	MappedKey ***pp_currmapped,long *p_maxmapped) {
	long imap;							
		
	for(imap=0; imap < (*p_maxmapped); imap++) {
		if((**pp_currmapped)[imap].orgkey == -1) break;
		}
	if(imap >= *p_maxmapped) {
		(*p_maxmapped) = ((*p_maxmapped) * 3L) / 2L;
		if((*pp_currmapped=(MappedKey**) IncreaseSpace((Handle)*pp_currmapped))
				== NULL) return(ABORT);
		}
	(**pp_currmapped)[imap].orgkey = orgkey;
	(**pp_currmapped)[imap].imagekey = imagekey;
	(**pp_currmapped)[imap].k = k;
	(**pp_currmapped)[imap].channel = channel;
	return(OK);
	}


// note 51
int RetrieveMappedKey(int orgkey,int k,int channel,MappedKey **p_currmapped,long maxmapped) {
	long imap;
	int key;

	for(imap=0; imap < maxmapped; imap++) {
		if((*p_currmapped)[imap].orgkey != orgkey) continue;
		if((*p_currmapped)[imap].k != k) continue;
		if((*p_currmapped)[imap].channel != channel) continue;
		key = (*p_currmapped)[imap].imagekey;
		(*p_currmapped)[imap].orgkey = -1;
		return(key);
		}
	return(orgkey);
	}


// note 51
int MapThisKey(int key,float howmuch,char mapmode,KeyNumberMap *p_map0,KeyNumberMap *p_map1) {
	KeyNumberMap map;	/* Fixed 24/2/99 - was 'p_map' */
	
	if(trace_maps) {
		if(howmuch < 0.000001) BPPrintMessage(0,odInfo,"\n=====\n");
		else BPPrintMessage(0,odInfo,"\n•\n");
		}
	if(mapmode == OFF) return(key);
	if(mapmode == FIXMAPMODE || mapmode == STEPWISE) {
		if(trace_maps) {
			BPPrintMessage(0,odInfo,"map0 p1 = %d, q1 = %d, p2 = %d, q2 = %d \n",p_map0->p1,p_map0->q1,p_map0->p2,p_map0->q2);
			BPPrintMessage(0,odInfo,"map1 p1 = %d, q1 = %d, p2 = %d, q2 = %d \n",p_map1->p1,p_map1->q1,p_map1->p2,p_map1->q2);
			BPPrintMessage(0,odInfo,"STEPWISE p1 = %d, q1 = %d, p2 = %d, q2 = %d\n",p_map0->p1,p_map0->q1,p_map0->p2,p_map0->q2);
			}
		return(KeyImage(key,p_map0));
		}
	map.p1 = p_map0->p1 + howmuch * (p_map1->p1 - p_map0->p1);
	map.q1 = p_map0->q1 + howmuch * (p_map1->q1 - p_map0->q1);
	map.p2 = p_map0->p2 + howmuch * (p_map1->p2 - p_map0->p2);
	map.q2 = p_map0->q2 + howmuch * (p_map1->q2 - p_map0->q2);
	if(trace_maps) {
		BPPrintMessage(0,odInfo,"map0 p1 = %d, q1 = %d, p2 = %d, q2 = %d \n",p_map0->p1,p_map0->q1,p_map0->p2,p_map0->q2);
		BPPrintMessage(0,odInfo,"map1 p1 = %d, q1 = %d, p2 = %d, q2 = %d \n",p_map1->p1,p_map1->q1,p_map1->p2,p_map1->q2);
		BPPrintMessage(0,odInfo,"CONTINUOUS howmuch = %.3f p1 = %d, q1 = %d, p2 = %d, q2 = %d\n",howmuch,map.p1,map.q1,map.p2,map.q2);
		}
	return(KeyImage(key,&map));
	}


int KeyImage(int key,KeyNumberMap *p_map) {
	int c;

	if(key < p_map->p1 || key > p_map->p2) return(key);
	if(p_map->p1 == p_map->p2) return(key);

	c = p_map->q1
			+ Round(((double)(key - p_map->p1) * (p_map->q2 - p_map->q1))
										/ ((double)p_map->p2 - p_map->p1));
	if(c < 0) {
		BPPrintMessage(0,odError,"Err. KeyImage(). c < 0\n");
		c = 0;
		}
	if(c > 127) {
		BPPrintMessage(0,odError,"Err. KeyImage(). c > 127\n");
		c = 127;
		}
	return(c);
	}

// note 49
int PlayCsoundLine(int cswrite,int j) {
	if(cswrite && j < Jbol && j > 0 && (*p_CsoundSize)[j] > ZERO && !ConvertMIDItoCsound) return(OK);
	return(MISSED);
	}

int PlayMIDIcode(int j) {
	if(j > 0 && j < Jbol && ((*p_Type)[j] & 1)) return(OK);
	return(MISSED);
	}

// note 49
int MIDIchannel(int cswrite,int j,int objectchannel,int *p_c0) {
	int localchan;
	if(ChannelEvent(*p_c0) && j < Jbol) {
		localchan = (*p_c0) % 16;
		(*p_c0) -= localchan;
		if(cswrite) return 0;
		if((*p_DefaultChannel)[j] > 0) return (*p_DefaultChannel)[j];
		if((*p_DefaultChannel)[j] < 0) {
			return localchan;
			}
		}
	// if(cswrite) return 0;
	localchan = 0;
	if(objectchannel > 0) localchan = objectchannel - 1; // This object has a specific channel
	if(localchan < 0) localchan = 0;
	return localchan;
	}

// note 49
int ThisInstrument(int cswrite,int k, int iline, int *p_index) {
	int instrument,mode,j;

	j = (*p_Instance)[k].object;
	if(j < 0) j = -j;
	instrument = (*p_Instance)[k].instrument;
	*p_index = InstrumentIndex(instrument);
	if(EventListOn && instrument > 0 && instrument < MAXINSTRUMENTS && !Exported_instrument[instrument]) ExportJsonCsoundInstrument(instrument);
	if(j < 2 || j >= Jbol) return instrument;
	if(!((*p_Type)[j] & 4)) return instrument;
	mode = (*p_CsoundInstrumentMode)[j];
	if(mode == GLOBAL_CS) return instrument;
	if(mode > 0) return (*p_CsoundInstr)[j];
	if(mode == LOCAL_CS) {
		if(PlayCsoundLine(cswrite,j) && iline >= 0 && iline < (*p_CsoundSize)[j]) {
		// Checking 'iline' is not really necessary, just prevent a crash!
			instrument = (*((*pp_CsoundScore)[j]))[iline].instrument;
			*p_index = InstrumentIndex(instrument);
			if(EventListOn && instrument > 0 && instrument < MAXINSTRUMENTS && !Exported_instrument[instrument]) ExportJsonCsoundInstrument(instrument);
			}
		return instrument;
		}
	BPPrintMessage(0,odError,"=> Error CsoundInstrumentMode = %d, j = %d\n",mode,j);
	return instrument;
	}

int InstrumentIndex(int instrument) {
	int j;
	for(j = 0; j < Jinstr; j++) 
		if((*p_CsInstrumentIndex)[j] == instrument) break;
	if(j < Jinstr) return(j);
	else return(0);
	}

// note 25
int TransposeAndExpand(int k,int *p_c1,int trans) {
	int oldc1,j;
	j = (*p_Instance)[k].object;
	if(j < 0) j = -j;
	if(j < 2) {
		BPPrintMessage(0,odError,"=> Err. TransposeAndExpand j = %d\n",j);
		return(OK);
		}
	oldc1 = (*p_c1);
	if((*p_Instance)[k].transposefirst) TransposeKey(p_c1,trans);
	*p_c1 = ExpandKey(*p_c1,(*p_Instance)[k].xpandkey,(*p_Instance)[k].xpandval);
	if(!(*p_Instance)[k].transposefirst) TransposeKey(p_c1,trans);
	if(*p_c1 < 0 || *p_c1 >= MAXKEY) {
		BPPrintMessage(0,odError,"=> Key out of range: %d\n",*p_c1);
		(*p_c1) = oldc1;
		}
	// Trace("TransposeAndExpand");
	return(OK);
	}

// note 49
long Find_im(int cswrite,int j) {
	long im;
	im = ZERO;
	if(j < 0) j = -j;
	if(j < 2 || j >= Jbol) return im;
	if(PlayMIDIcode(j)) im = (*p_MIDIsize)[j];
	if(PlayCsoundLine(cswrite,j)) im = (*p_CsoundSize)[j];
	return im;
	}
// note 17
void SetObjectParameters(ParameterStatus **params,ParameterSpecs **currentinstancevalues,int maxparam,int kcurrentinstance,int j,Milliseconds t0,unsigned long objectduration) {
	// Copy the variations of parameters of instance 'kcurrentinstance' to 'params', used by CscoreWrite()
	// Moved out of MakeSound() so that they can be restored when the object resumes (2026-11-10) - fixed by Claude
	int index;
	for(index = 0; index < maxparam; index++) {
		(*params)[index].startvalue = (*currentinstancevalues)[index].v0;
		(*params)[index].endvalue = (*currentinstancevalues)[index].v1;
		(*params)[index].imax = (*currentinstancevalues)[index].imax;
		(*params)[index].dur = objectduration;
		(*params)[index].starttime = t0 + (*p_Instance)[kcurrentinstance].starttime;
		(*params)[index].mode = (*currentinstancevalues)[index].mode;
		(*params)[index].point = (*currentinstancevalues)[index].point;
		if((index == IPANORAMIC && j < Jbol && !(*p_OkPan)[j]) || (index == IVOLUME && j < Jbol && !(*p_OkVolume)[j]))
			(*params)[index].active = FALSE;
		// 2026-11-10: an additional parameter without a value is inactive, so that CscoreWrite() uses its default value - fixed by Claude
		else if(index > IPANORAMIC && !(*currentinstancevalues)[index].known)
			(*params)[index].active = FALSE;
		// 2026-11-08: continuous variations of MIDI parameters (pitchbend, volume, modulation, pressure, panoramic) apply
		// to the channel on which they were set, as in real-time MIDI and MIDI files (see 'stream' in MakeSound()). In
		// "_chan(4) _pitchbend(+200) _pitchcont do4 _chan(2) sol4 ..." sol4 received the pitchbend of channel 4 in the
		// Csound score. A fixed value applies to the channel of the object, as in MIDI, e.g. "_volume(80) _chan(4) do4" - fixed by Claude
		else if(index <= IPANORAMIC && (*currentinstancevalues)[index].mode == CONTINUOUS
				&& (*currentinstancevalues)[index].channel > 0
				&& (*p_Instance)[kcurrentinstance].channel > 0
				&& (*currentinstancevalues)[index].channel != (*p_Instance)[kcurrentinstance].channel)
			(*params)[index].active = FALSE;
		else
			(*params)[index].active = TRUE;
		}
	}
