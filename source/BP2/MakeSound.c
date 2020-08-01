/* MakeSound.c (BP2 version CVS) */

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


MakeSound(tokenbyte ***pp_A,int *p_kmax,unsigned long imaxstreak,int maxnsequences,
	tokenbyte ***pp_b,long tmin,long tmax,int interruptok,int showpianoroll,
	Milliseconds **p_delta)
{
PerfParameters ****pp_currentparams,**ptrperf;
ContinuousControl **p_control;
ParameterStatus **params,**ptrs;
ParameterSpecs **currentinstancevalues;
ParameterStream **stream;

int w,y,ii,iii,j,jj,k,kcurrentinstance,n,noccurrence,s,in,itick,c,c0,c1,oldc1,c2,kfirstinstance,ch,
	alph,alphach,r,result,pos,instrument,nseq,key,sequence,control,doneobjects,trans,simplenote,
	rep,rep1,rep2,rep3,mustwait,waitcompletion,strike,iparam,chan,objectchannel,resetok,
	**p_inext,**p_inext1,**p_istartperiod,**p_iendperiod,**p_icycle,maxconc,hastabs,
	compiledmem,ifunc,interruptedonce,strikeagain,onoff,icont,chancont,minkey,maxkey,
	idummy,lsb,msb,rs,clickon[MAXTICKS],hidden[MAXTICKS],tickposition[MAXTICKS],
	localchan,localvelocity,outtime,foundfirsteventinperiod,maxparam,index,overflow,
	foundlasteventinperiod,foundfirstevent,cswrite,nextisobject,exclusive,themessage,
	okvolume,okpanoramic,okpitchbend,okpressure,okmodulation,hrect,htext,leftoffset,topoffset,
	contchan,volume,panoramic,pitchbend,modulation,pressure,**p_seqcont[MAXCHAN];
	
Milliseconds time,buffertime,torigin,t0,t1,t11,t2,t2obj,
	t2tick,t22,t3,date1,**p_t1,**p_t2cont[MAXCHAN],
	**p_nextd,computetime,currenttime,objectduration,objectstarttime;

Handle h;
char **p_keyon[MAXCHAN],**p_onoff,**p_active[MAXCHAN],line[4];
long timeleft,formertime,size,istreak,posmin,localperiod,endxmax,endymax,oldtcurr,
	i1,i2,oldi2,imap,gap,maxmapped,i,im,ievent,yruler;
unsigned long currswitchstate[MAXCHAN],oldtime,maxmidibytes5,drivertime;
unsigned int seed;
float howmuch;
double value,streakposition[MAXTICKS],tickdate[MAXTICKS],fstreak,alpha,beta,date,olddate,
	preroll,postroll,objectperiod,beforeperiod,firstcycleduration,**p_periodgap,p,q;
p_list **waitlist,**scriptlist;
MIDI_Event e;
MappedKey **p_currmapped;
CGrafPtr port;
GDHandle gdh;
Rect graphrect,labelrect;
GrafPtr saveport;
Str255 label;

w = wGraphic;
maxmidibytes5 = MaxMIDIbytes / 5L;
oldtcurr = Tcurr;

if(Panic || CheckEmergency() != OK) return(ABORT);
if(SoundOn) return(OK);

if(CompileRegressions() != OK) return(ABORT);

if(ConvertMIDItoCsound || ItemCapture) showpianoroll = FALSE;

cswrite = FALSE;
if(((OutCsound && (FileWriteMode == NOW || !OutMIDI))
	|| ConvertMIDItoCsound) && !ItemCapture && !showpianoroll) cswrite = TRUE;

MIDIfileOn = FALSE;
if(WriteMIDIfile && (FileWriteMode == NOW || !OutMIDI) && !ItemCapture && !showpianoroll)
	MIDIfileOn = TRUE;

if(FirstTime && (cswrite || (!OutMIDI && !MIDIfileOn) || showpianoroll)) return(OK);
	
interruptedonce = overflow = FALSE;
rs = 0;
resetok = TRUE;

#if BP_CARBON_GUI
// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
if((rep=MyButton(1)) != FAILED) {
	// any reason that InterruptSound() is not used here instead ?
	StopCount(0);
	Interrupted = TRUE; compiledmem = CompiledGr;
	SetButtons(TRUE);
	if(rep == OK) while((rep = MainEvent()) != RESUME && rep != STOP && rep != EXIT);
	if(rep == EXIT) return(rep);
	if((rep == STOP) || (compiledmem && !CompiledGr)) {
		return(ABORT);
		}
	if(LoadedIn && (!CompiledIn && (rep=CompileInteraction()) != OK)) return(rep);
	}
rep = OK;
if(EventState != NO) return(EventState);
#endif /* BP_CARBON_GUI */

Ke = log((double) SpeedRange) / 64.;
t0 = ZERO;
if(*p_kmax >= Maxevent) {
	if(Beta) Alert1("kmax >= Maxevent. Err. MakeSound()");
	return(ABORT);
	}

maxconc = maxnsequences;
if(Beta && maxconc > Maxconc) {
	Alert1("maxconc > Maxconc. Err. MakeSound()");
	}

if((p_control=(ContinuousControl**)GiveSpace(maxconc*sizeof(ContinuousControl))) == NULL)
	return(ABORT);
	
if((pp_currentparams=(PerfParameters****)GiveSpace(maxconc*sizeof(PerfParameters**)))
	== NULL) return(ABORT);

if((p_inext = (int**) GiveSpace((Size)Maxevent*sizeof(int))) == NULL) return(ABORT);
if((p_inext1 = (int**) GiveSpace((Size)Maxevent*sizeof(int))) == NULL) return(ABORT);
if((p_istartperiod = (int**) GiveSpace((Size)Maxevent*sizeof(int))) == NULL) return(ABORT);
if((p_iendperiod = (int**) GiveSpace((Size)Maxevent*sizeof(int))) == NULL) return(ABORT);
if((p_icycle = (int**) GiveSpace((Size)Maxevent*sizeof(int))) == NULL) return(ABORT);
if((p_periodgap = (double**) GiveSpace((Size)Maxevent*sizeof(double))) == NULL) return(ABORT);
if((p_nextd = (Milliseconds**) GiveSpace((Size)Maxevent*sizeof(Milliseconds))) == NULL) return(ABORT);
if((p_t1 = (Milliseconds**) GiveSpace((Size)Maxevent*sizeof(Milliseconds))) == NULL) return(ABORT);
if((p_onoff = (char**) GiveSpace((Size)Maxevent*sizeof(char))) == NULL) return(ABORT);

maxmapped = 32L;
p_currmapped = (MappedKey**)GiveSpace((Size)maxmapped*sizeof(MappedKey));
if(p_currmapped == NULL) return(ABORT);
for(imap=0; imap < maxmapped; imap++) (*p_currmapped)[imap].orgkey = -1;

maxparam = IPANORAMIC + 1;

for(ch=0; ch < MAXCHAN; ch++) {
	if((p_keyon[ch] = (char**) GiveSpace((Size)(MAXKEY+1)*sizeof(char)))
			== NULL) return(ABORT);
	for(k=0; k < MAXKEY; k++) (*p_keyon[ch])[k] = 0;
	if((p_seqcont[ch] = (int**) GiveSpace((Size)(maxparam)*sizeof(int)))
			== NULL) return(ABORT);
	if((p_t2cont[ch] = (Milliseconds**) GiveSpace((Size)(maxparam)*sizeof(Milliseconds)))
			== NULL) return(ABORT);
	if((p_active[ch] = (char**) GiveSpace((Size)(maxparam)*sizeof(char)))
			== NULL) return(ABORT);
	}
for(nseq=0; nseq < maxconc; nseq++) {
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
t11 = t22 = Infpos;
for(k=2; k <= (*p_kmax); k++) {
	j = (*p_Instance)[k].object;
	if(j == 0) continue;
	if(j < 0) j = -j;
	(*p_inext1)[k] = 0;
	(*p_onoff)[k] = FALSE;
	(*p_istartperiod)[k] = (*p_iendperiod)[k] = -1;
	foundfirsteventinperiod = foundlasteventinperiod = TRUE;
	alpha = (*p_Instance)[k].alpha;
	beta = (*p_Instance)[k].dilationratio;	/* alpha != beta if the sound-object is cyclic */
	if((*p_Instance)[k].ncycles < 2 && beta != alpha) {
		if(Beta) Alert1("Err. MakeSound(). beta != alpha");
		beta = (*p_Instance)[k].dilationratio = alpha;
		}
	if(j < 16384) {
		im = (*p_MIDIsize)[j];
		if(!((*p_Type)[j] & 1)) im = ZERO;
		if(cswrite && (*p_CsoundSize)[j] > ZERO && !ConvertMIDItoCsound)
			im = (*p_CsoundSize)[j];
		firstcycleduration = beta * (*p_Dur)[j];
		(*p_iendperiod)[k] = im - 1;
		if((*p_PreRollMode)[j] == ABSOLUTE) preroll = (*p_PreRoll)[j];
		else preroll = beta * (*p_PreRoll)[j];
		if(alpha > 1.) {
			if(GetPeriod(j,beta,&objectperiod,&beforeperiod) == OK) {
				foundfirsteventinperiod = FALSE;
				foundlasteventinperiod = FALSE;
				}
			}
		}
	else preroll = 0.;
	
	date = (*p_t1)[k] = - preroll;
	if((*p_Instance)[k].truncbeg < EPSILON) date1 = date;
	else date1 = (*p_Instance)[k].truncbeg;
	if(PlayFromInsertionPoint && k == 2
									&& (*p_Instance)[k].endtime > (*p_Tpict)[iProto]) {
		/* Playing single sound-object from insertion point */
		if((*p_Tpict)[iProto] > ((*p_Instance)[k].starttime + date))
			date1 = (*p_Tpict)[iProto] - (*p_Instance)[k].starttime;
		}
	foundfirstevent = FALSE;
	if(j > 1 && j < 16384) {	/* Sound-object or time pattern */
		/* Look for first event in object and for first and last events in its periodical part */
		if(Beta && j >= Jbol && Jbol < 2)
			Alert1("Err. MakeSound(). j >= Jbol && Jbol < 2");
		for(i=0; i < im; i++) {
			olddate = date;
			if(cswrite && (*p_CsoundSize)[j] > 0 && !ConvertMIDItoCsound)
				date += (Milliseconds) (beta * (*((*pp_CsoundTime)[j]))[i]);
			else
				date += (Milliseconds) (beta * (*((*pp_MIDIcode)[j]))[i].time);
			if(!foundfirstevent && date >= date1) {
				(*p_inext1)[k] = i;	/* Index of first message to be sent */
				(*p_t1)[k] = date - date1 - preroll;
				foundfirstevent = TRUE;
				if(foundlasteventinperiod) break;
				}
			if(!foundfirsteventinperiod && date >= beforeperiod) {
				(*p_istartperiod)[k] = i;
				(*p_periodgap)[k] = date - beforeperiod;
				foundfirsteventinperiod = TRUE;
				}
			if(!foundlasteventinperiod && (i == (im - 1) || date > firstcycleduration)) {
			/* We checked 'firstcycleduration' in case postroll was negative */
				if(cswrite && (*p_CsoundSize)[j] > 0 && !ConvertMIDItoCsound) {
					if(i < (im-1) && (*((*pp_CsoundTime)[j]))[i+1] == ZERO) {
						i++;
						while(i < (im-1) && (*((*pp_CsoundTime)[j]))[i] == ZERO) i++;
						if(i < (im-1)) i--;
						}
					}
				else {
					if(i < (im-1) && (*((*pp_MIDIcode)[j]))[i+1].time == ZERO) {
						i++;
						while(i < (im-1) && (*((*pp_MIDIcode)[j]))[i].time == ZERO) i++;
						if(i < (im-1)) i--;
						}
					}
				(*p_iendperiod)[k] = i;
				(*p_periodgap)[k] += (firstcycleduration - olddate);
				foundlasteventinperiod = TRUE;
				break;
				}
			}
		}
	else {	/* Simple note or silence */
		(*p_inext1)[k] = 0;
		}
	if(((*p_Instance)[k].starttime - preroll) < t11) {
		kfirstinstance = k;
		t11 = ((*p_Instance)[k].starttime - preroll);
		}
	}
for(k=2; k <= (*p_kmax); k++) {
	j = (*p_Instance)[k].object;
	if(j == 0) continue;
	if(j < 0) j = -j;
	if(j < 16384) {
		beta = (*p_Instance)[k].dilationratio;
		if((*p_PreRollMode)[j] == ABSOLUTE) preroll = (*p_PreRoll)[j];
		else preroll = beta * (*p_PreRoll)[j];
		}
	else preroll = 0.;
	if((k != kfirstinstance) && (((*p_Instance)[k].starttime - preroll) < t22)) {
		t22 = ((*p_Instance)[k].starttime - preroll);
		}
	}

SoundOn = TRUE;

if(showpianoroll) {
#if BP_CARBON_GUI
	// FIXME:  Would like to figure out how to move all of the piano roll code elsewhere ...
	if(ShowMessages || PlayPrototypeOn) ShowMessage(TRUE,wMessage,"Preparing piano roll...");
	GetPort(&saveport);
	minkey = 127; maxkey = 0;
	tmax = ZERO;
	for(k=2; k <= (*p_kmax); k++) {
		j = (*p_Instance)[k].object;
		if(j < 0) j = -j;
		if(j < 2) continue;
		if(j < 16384) {
			beta = (*p_Instance)[k].dilationratio;
			if((*p_PostRollMode)[j] == ABSOLUTE) postroll = (*p_PostRollMode)[j];
			else postroll = beta * (*p_PostRollMode)[j];
			}
		else postroll = 0.;
		if(tmax < ((*p_Instance)[k].endtime - postroll))
			tmax = ((*p_Instance)[k].endtime - postroll);
		trans = (*p_Instance)[k].transposition;
		if(j < 16384) {
			if(j < 2 || j >= Jbol) continue;
			for(i=0; i < (*p_MIDIsize)[j]; i++) {
				c0 = (*((*pp_MIDIcode)[j]))[i].byte;
				if(c0 < 128) continue;
				if(ChannelEvent(c0)) {
					localchan = c0 % 16;
					c0 -= localchan;
					if(c0 == NoteOn) {
						c1 = (*((*pp_MIDIcode)[j]))[++i].byte;		/* key number */
						if((*p_Instance)[k].lastistranspose) TransposeKey(&c1,trans);
						c1 = ExpandKey(c1,(*p_Instance)[k].xpandkey,(*p_Instance)[k].xpandval);
						if(!(*p_Instance)[k].lastistranspose) TransposeKey(&c1,trans);
						key = c1;
						if(key < minkey) minkey = key;
						if(key > maxkey) maxkey = key;
						}
					else i++;
					i++;
					}
				}
			}
		else {
			key = GoodKey(j);
			if((*p_Instance)[k].lastistranspose) TransposeKey(&key,trans);
			key = ExpandKey(key,(*p_Instance)[k].xpandkey,(*p_Instance)[k].xpandval);
			if(!(*p_Instance)[k].lastistranspose) TransposeKey(&key,trans);
		/*	key = GoodKey(j); */
			if(key < minkey) minkey = key;
			if(key > maxkey) maxkey = key;
			}
		}
	minkey = (minkey / 6) * 6;
	maxkey = 6 + ((maxkey / 6) * 6);
	if (minkey > maxkey)  minkey = maxkey = 60;
	hrect = 3;
	htext = WindowTextSize[w] + 2;
	leftoffset = 13 + StringWidth("\pmmm") - (tmin * GraphicScaleP) / GraphicScaleQ / 10;
	topoffset = (3 * htext) + 6;
	endxmax = leftoffset + 50 + ((tmax - tmin) * GraphicScaleP) / GraphicScaleQ / 10;
//	((*p_T)[imax] * GraphicScaleP) / GraphicScaleQ / 10 + leftoffset;
	if(endxmax < 100) endxmax = 100;
	endymax = topoffset + ((maxkey - minkey) * hrect) + 10;
	
	GetWindowPortBounds(Window[w], &graphrect);
	graphrect.bottom = graphrect.top + endymax;
	graphrect.right = graphrect.left + endxmax;
	
	if((result=OpenGraphic(w,&graphrect,NO,&port,&gdh)) != OK) goto GETOUT;
	
	if((result=DrawItemBackground(&graphrect,imaxstreak,htext,hrect,leftoffset,NO,
		p_delta,&yruler,topoffset,&overflow)) != OK || overflow) goto OUTGRAPHIC;
	
	topoffset += 3;
	PenSize(1,1);
	if(UseGraphicsColor) RGBForeColor(&NoteScaleColor1);
	else RGBForeColor(&Black);
	for(key=0; key < 128; key+=12) {	/* Draw horizontal line "C" */
		if(key < minkey || key > maxkey) continue;
		y = (maxkey - key) * hrect + topoffset + 1;
		MoveTo(leftoffset-2,y);
		LineTo(endxmax-27,y);
		sprintf(line,"c%ld",(long)((key - (key % 12))/12)-1L);
		c2pstrcpy(label, line);
		labelrect.top = y - 7;
		labelrect.left = endxmax - 25;
		labelrect.right = endxmax - 25 + StringWidth(label);
		labelrect.bottom = y + 6;
		EraseRect(&labelrect);
		MoveTo(endxmax-24,y + 3);
		DrawString(label);
		}
		
	if(UseGraphicsColor) RGBForeColor(&NoteScaleColor2);
	else RGBForeColor(&Black);
	for(key=6; key < 128; key+=12) {	/* Draw horizontal line "F#" */
		if(key < minkey || key > maxkey) continue;
		y = (maxkey - key) * hrect + topoffset + 1;
		MoveTo(leftoffset-2,y);
		LineTo(endxmax-27,y);
		sprintf(line,"f#%ld",(long)((key - (key % 12))/12)-1L);
		c2pstrcpy(label, line);
		labelrect.top = y - 7;
		labelrect.left = endxmax - 25;
		labelrect.right = endxmax - 25 + StringWidth(label);
		labelrect.bottom = y + 6;
		EraseRect(&labelrect);
		MoveTo(endxmax-24,y + 3);
		DrawString(label);
		}
	RGBForeColor(&Black);
	PenNormal();
		
	Hmin[w] = Hmax[w] = 0;
	Hzero[w] = Vzero[w] = 0;
	Vmin[w] = Vmax[w] = 0;
	PenNormal();
	RGBForeColor(&Black);
	PenSize(1,hrect);
#endif /* BP_CARBON_GUI */
	}

#if WITH_REAL_TIME_MIDI
if(!MIDIfileOn && !cswrite && OutMIDI && !showpianoroll) drivertime = GetDriverTime();
#endif
	
if(Improvize && !Interrupted && !FirstTime && !ItemCapture && !showpianoroll) {
	computetime = drivertime - ComputeStart;
	if(computetime > MaxComputeTime) MaxComputeTime = computetime;
	}
else computetime = ZERO;
if(!MIDIfileOn && !cswrite && OutMIDI && !ItemCapture && !FirstTime && !PlayPrototypeOn
		&& !showpianoroll) {
#if WITH_REAL_TIME_MIDI
	if(OkWait && SynchronizeStart) {
#if !BP_CARBON_GUI
		FlashInfo("Synchronized start is not currently enabled in non-Carbon builds!");
#else
		drivertime = GetDriverTime();
		if(ShowMessages && (Tcurr > drivertime))
			FlashInfo("Waiting until previous item is over...");
		result = WaitForEmptyBuffer();
		if(result != OK && result != RESUME && result != QUICK) goto OVER;
		if(result == RESUME) interruptedonce = TRUE;
		result = OK;
		if(SkipFlag) goto OVER;
			
		Nbytes = Tbytes2 = ZERO;
		if(PlayChan > 0) {
			FlashInfo("Waiting for MIDI sync code...");
			WhenItStarted = clock();
			do {
				// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
				if((rep=MyButton(1)) != FAILED) {
					// any reason that InterruptSound() is not used here instead ?
					StopCount(0);
					interruptedonce = TRUE;
					SetButtons(TRUE);
					compiledmem = CompiledGr;
					if(rep == OK)
						while((rep = MainEvent()) != RESUME && rep != STOP && rep != EXIT);
					if(rep == EXIT) {
						result = EXIT; goto OVER;
						}
					if(rep == STOP || (compiledmem && !CompiledGr)) {
						result = ABORT;
						goto OVER;
						}
					if(LoadedIn && (!CompiledIn && (result=CompileInteraction()) != OK))
						goto OVER;
					break;
					}
				rep = OK;
				if(EventState != NO) {
					result = EventState; goto OVER;
					}
				PleaseWait();
				if((result = ListenMIDI(0,0,0)) != OK && result != RESUME
					&& result != QUICK && result != ENDREPEAT) goto OVER;
				if(result == QUICK || result == ENDREPEAT) break;
				if(SkipFlag) goto OVER;
				}
			while((SynchroSignal == OFF) && SynchronizeStart);
			HideWindow(Window[wMessage]);
			LapWait += (clock() - WhenItStarted);
			if(ScriptExecOn && CountOn && WaitEndDate > ZERO) WaitEndDate += LapWait;
			}
		else {		/* No MIDI sync key */
			FlashInfo("Waiting for 'Start' or 'Continue' MIDI message. Click mouse to resume...");
#if BP_CARBON_GUI
			SetCursor(&KeyboardCursor);
#endif /* BP_CARBON_GUI */
			exclusive = FALSE;
			while(Button());
			while(!Button()) {
				if((result = ListenMIDI(0,0,0)) != OK && result != RESUME
					&& result != QUICK && result != ENDREPEAT) goto OVER;
				if(result == QUICK || result == ENDREPEAT) break;
				if(SkipFlag) goto OVER;
				if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) continue;
				if(e.type != RAW_EVENT) continue;
				themessage = ByteToInt(e.data2);
				if(!exclusive && themessage == Start) break;
				if(!exclusive && themessage == Continue) break;
				if(!exclusive && themessage == Stop) {
					result = ABORT; goto OVER;
					}
				if(themessage == SystemExclusive) exclusive = TRUE;
				if(themessage == EndSysEx) exclusive = FALSE;
				}
			HideWindow(Window[wInfo]);
			SetDefaultCursor();
			}
#endif /* BP_CARBON_GUI */
		}
#endif /* WITH_REAL_TIME_MIDI */
	}
if(cswrite || MIDIfileOn || ItemCapture) {
	sprintf(Message,"Writing %ld sound-objects",(long)(*p_kmax)-2L);
	FlashInfo(Message);
	}

START2:

if(cswrite) {
	if((result=CompileCsoundObjects()) != OK) goto OVER;
	if(Jinstr == 1 && (*p_CsInstrumentIndex)[0] == -1) {
		ShowMessage(TRUE,wMessage,"Couldn't find Csound instrument index. Index '1' will be assigned by default.");
		WaitABit(1000L);
		}
		
	if(!ConvertMIDItoCsound) {
		if((result=PrepareCsFile()) != OK) goto OVER;
		}
	Nplay = 1; FunctionTable = 100;
	}

if(MIDIfileOn) {
	if((result=PrepareMIDIFile()) != OK) goto OVER;
	Nplay = 1;
	}

mustwait = FALSE;
rs = 0;

for(nseq=0; nseq < maxconc; nseq++) {
	ptrperf = (*pp_currentparams)[nseq];
	for(key=0; key < MAXKEY; key++) {
		(*ptrperf)->level[key] = 0;
		(*ptrperf)->startparams[key] = NULL;
		}
	(*ptrperf)->transpose = 0;
	
	maxparam = (*ptrperf)->numberparams;
	for(i=0; i < maxparam; i++) (*((*ptrperf)->params))[i].active = FALSE;
	if(maxparam <= IPANORAMIC) {
		if(Beta) Alert1("Err. MakeSound(). Beta && maxparam <= IPANORAMIC");
		goto OVER;
		}
	
	for(i=0; i < (*p_control)[nseq].number; i++) {
		(*((*p_control)[nseq].param))[i].ibm = -1L;
		}
	}

// The following loop repeats Nplay performances of item.

for(noccurrence = 0; noccurrence < Nplay || SynchroSignal == PLAYFOREVER; noccurrence++) {
	result = OK;
	PleaseWait();
	if(SkipFlag) goto OVER;
#if BP_CARBON_GUI
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	if(!showpianoroll && (result=MyButton(1)) != FAILED) {
		interruptedonce = TRUE;
		if(result != OK || (result=InterruptSound()) != OK) goto OVER;
		}
	result = OK;
	if(EventState != NO) {
		result = EventState; goto OVER;
		}
#endif /* BP_CARBON_GUI */
	for(k=2; k <= (*p_kmax); k++) {
		(*p_inext)[k] = (*p_inext1)[k];
		(*p_onoff)[k] = FALSE;
		(*p_nextd)[k] = (*p_Instance)[k].starttime + (*p_t1)[k];
		}
	icont = -1; chancont = -1;
	for(ch=0; ch < MAXCHAN; ch++) {
		currswitchstate[ch] = ZERO;
		for(index=0; index <= IPANORAMIC; index++) {
			(*(p_t2cont[ch]))[index] = Infpos;
			(*(p_active[ch]))[index] = FALSE;
			}
		}
		
	LimTimeSet = LimCompute = FALSE; TimeMax = MAXTIME;
	t1 = t11; t2 = t2obj = t22; kcurrentinstance = kfirstinstance;
	t2tick = Infpos;
	instrument = -1;
	if(!MIDIfileOn && !cswrite && OutMIDI && mustwait && !ItemCapture && !showpianoroll) {
#if WITH_REAL_TIME_MIDI
		drivertime = GetDriverTime();
		if(ShowMessages
				&& (Tcurr > drivertime + ((SetUpTime + 600L) / Time_res)))
			FlashInfo("Waiting until previous item is over...");
		result = WaitForEmptyBuffer();
		HideWindow(Window[wInfo]);
#endif
		}
	else result = OK;
	if(result != OK && result != RESUME && result != QUICK) goto OVER;
	if(result == RESUME) interruptedonce = TRUE;
	result = OK;
		
	mustwait = FALSE;
	
	if(Improvize && (Nplay > 1 || SynchroSignal == PLAYFOREVER)) {
		sprintf(Message,"%ldth repetition...",(long)noccurrence+1L);
		ShowMessage(TRUE,wMessage,Message);
		}
		
	if((!Improvize || ItemNumber == ZERO || PlaySelectionOn || ItemCapture
			|| cswrite || MIDIfileOn || showpianoroll) && !CyclicPlay)
		Tcurr = ZERO;
	/* This value is used in ResetMIDIControllers() */
	
	if(!cswrite && !MIDIfileOn && ResetControllers && !ItemCapture && !CyclicPlay
		&& !showpianoroll && resetok) ResetMIDIControllers(NO,FALSE,YES);
	
	for(ch=0; ch < MAXCHAN; ch++) {
	/*	firstvolume[ch] = firstmodulation[ch] = firstpitchbend[ch] = firstpressure[ch]
			= firstpanoramic[ch] = FALSE;
		if(!cswrite && !MIDIfileOn && ResetControllers && !ItemCapture && !CyclicPlay
				&& !showpianoroll && resetok) {
			firstvolume[ch] = firstmodulation[ch] = firstpitchbend[ch] = firstpressure[ch]
				= firstpanoramic[ch] = TRUE;
			} */
		if(MIDIfileOpened || cswrite)
			for(k=0; k < MAXKEY; k++) (*p_keyon[ch])[k] = 0;
		}
	resetok = FALSE;
	
	if((MIDIfileOn || cswrite || showpianoroll || !Improvize
			|| ItemNumber == ZERO || PlaySelectionOn || ItemCapture) && !CyclicPlay) {
		if(!showpianoroll && !cswrite && !MIDIfileOn) SetDriverTime(Tcurr);
		if(!cswrite && !MIDIfileOn && !ItemCapture && !showpianoroll)
			Tcurr += (SetUpTime + 600L) / Time_res;
		currenttime = Tcurr * Time_res;
		Nbytes = 0; Tbytes2 = ZERO;
		}
	else {
		drivertime = GetDriverTime();
		if(drivertime > Tcurr) {
			/* Too late to start on time! */
			currenttime = drivertime * Time_res;
			Nbytes = 0; Tbytes2 = ZERO;
			}
		else currenttime = Tcurr * Time_res;
		}
	
	if(t1 < ZERO) t0 = currenttime - t1;
	else t0 = currenttime;
	
	torigin = t0 + t1;
	
	Tcurr = (t0 + t1) / Time_res;
	
	if(cswrite) {
TRYCSFILE:
		/* Tempo assignment */
		if(Pclock > 0.) 	{	/* Striated or measured smooth time */
			if(Simplify((double)INT_MAX,60L*Qclock,Pclock,&p,&q) != OK)
				Simplify((double)INT_MAX,Qclock,floor(Pclock/60L),&p,&q);
			sprintf(Message,"t %.3f %.3f",
				((double) torigin) * Qclock / ((double) Pclock) / 1000.,
				((double) p)/q);
			}
		else {	/* Unmeasured smooth time */
			sprintf(Message,"t %.3f 60",((double) torigin) / 1000.);
			}
		if(ConvertMIDItoCsound)
			Println(wPrototype7,Message);
		else {
			if(CsoundTrace) {
				SetSelect(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
				Println(wTrace,Message);
				}
			if(WriteToFile(NO,CsoundFileFormat,Message,CsRefNum) != OK) {
				sprintf(Message,"Couldn't write to file '%s'. May be it has been closed by another application",
					CsFileName);
				Alert1(Message);
				CloseCsScore();
				if((result=PrepareCsFile()) != OK) goto OVER;
				goto TRYCSFILE;
				}
			}
		}
	
	if(!MIDIfileOn && !cswrite && OutMIDI && PlayTicks && TickThere && !ItemCapture && !showpianoroll) {
#if WITH_REAL_TIME_MIDI
		for(itick=0; itick < MAXTICKS; itick++) clickon[itick] = hidden[itick] = FALSE;
		ResetTicksInItem(ZERO,tickposition,streakposition,tickdate,clickon,hidden,imaxstreak,
			&rs,p_keyon);
		istreak = fstreak = 1.;
		if(posmin == ZERO && (*p_T)[istreak] <= t1 && ObjScriptLine(kcurrentinstance) == NULL)
			InsertTickInItem(fstreak,clickon,hidden,tickdate,&rs,p_keyon,streakposition,t0,
				tickposition,imaxstreak);
#endif
		}
	
	ItemOutPutOn = TRUE;
	doneobjects = 0;
	
	while(kcurrentinstance > 0) {
		currentinstancevalues = (*p_Instance)[kcurrentinstance].contparameters.values;
		j = (*p_Instance)[kcurrentinstance].object;
		if(j == 0) {
			if(Beta) Alert1("Err. MakeSound(). j = 0");
			result = ABORT; goto OVER;
			}
		outtime = FALSE;
		if(j < 0) {
			j = -j; outtime = TRUE;
			}
		if((objectchannel = ChannelConvert((*p_Instance)[kcurrentinstance].channel)) < 0) {
			result = ABORT; goto OVER;
			}
		sequence = 0;  /* Normally not required */
		instrument = (*p_Instance)[kcurrentinstance].instrument;
		nseq = (*p_Instance)[kcurrentinstance].nseq;
		if(nseq < 0 || nseq >= maxconc) {
			if(Beta) Alert1("Err. MakeSound(). nseq < 0 || nseq >= maxconc");
			nseq = 0;
			}
		params = (*((*pp_currentparams)[nseq]))->params;
		if(j < 16384) {
			if(Beta && j >= Jbol && Jbol < 2) Alert1("Err. MakeSound(). j >= Jbol && Jbol < 2");
			if(j < Jbol) {
				switch((*p_StrikeAgain)[j]) {
					case -1:
						strikeagain = StrikeAgainDefault;
						break;
					case TRUE:
						strikeagain = TRUE;
						break;
					case FALSE:
						strikeagain = FALSE;
						break;
					}
				}
			else {
				strikeagain = StrikeAgainDefault;
				/* This is a time pattern! */
				}
			}
		else strikeagain = StrikeAgainDefault;
		localvelocity = ByteToInt((*p_Instance)[kcurrentinstance].velocity);
		trans = (*((*pp_currentparams)[nseq]))->transpose
			= (*p_Instance)[kcurrentinstance].transposition;
		(*((*pp_currentparams)[nseq]))->xpandkey = (*p_Instance)[kcurrentinstance].xpandkey;
		(*((*pp_currentparams)[nseq]))->xpandval = (*p_Instance)[kcurrentinstance].xpandval;
		(*((*pp_currentparams)[nseq]))->lastistranspose = (*p_Instance)[kcurrentinstance].lastistranspose;
		t3 = (*p_Instance)[kcurrentinstance].endtime;
		objectstarttime = (*p_Instance)[kcurrentinstance].starttime;
		objectduration = t3 - objectstarttime;
		ievent = (*p_inext)[kcurrentinstance];
		beta = (*p_Instance)[kcurrentinstance].dilationratio;
		if(j < 16384) {
			im = (*p_MIDIsize)[j];
			if(!((*p_Type)[j] & 1)) im = ZERO;
			if(cswrite && (*p_CsoundSize)[j] > ZERO && !ConvertMIDItoCsound)
				im = (*p_CsoundSize)[j];
			}
		else im = 6;	/* Simple note contains 6 MIDI bytes (NoteOn/NoteOff). */
		
		
// Initialise sound-object instance kcurrentinstance

		if(!(*p_onoff)[kcurrentinstance]) {
			howmuch = 0.;
#if BP_CARBON_GUI
			// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
			if(!showpianoroll && (result=MyButton(2)) != FAILED) {
				interruptedonce = TRUE;
				if(result != OK || (result=InterruptSound()) != OK) goto OVER;
				}
			else PleaseWait();
#endif /* BP_CARBON_GUI */
			result = OK;
			(*p_onoff)[kcurrentinstance] = TRUE;
			(*p_icycle)[kcurrentinstance] = 1;
			if(objectchannel > 0) chan = objectchannel - 1;
			else chan = 0;
			if(chan < 0) chan = 0;
			
			Tcurr = (t0 + t1) / Time_res;
			
			if(!outtime) {
				okvolume = okpanoramic = okpitchbend = okpressure = okmodulation = TRUE;
				/* Don't send control value to MIDI if it is being set by a stream */
				for(n=0; !cswrite && n < maxconc; n++) {
					stream = (*p_control)[n].param;
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
					
				volume = (int) VolumeStart(kcurrentinstance);
				if((control=(*((*p_Instance)[kcurrentinstance].contparameters.values))[IVOLUME].control)
					 > -1) volume = ParamValue[control];
				
				panoramic = (int) PanoramicStart(kcurrentinstance);
				if((control=(*((*p_Instance)[kcurrentinstance].contparameters.values))[IPANORAMIC].control)
					 > -1) panoramic = ParamValue[control];
				
				pitchbend = (int) PitchbendStart(kcurrentinstance);
				
				pressure = (int) PressureStart(kcurrentinstance);
				
				modulation = (int) ModulationStart(kcurrentinstance);
				
				if(okvolume && (volume != (*p_Oldvalue)[chan].volume /* || firstvolume[chan] */)
						&& (j >= Jbol || (*p_OkVolume)[j])) {
					(*p_Oldvalue)[chan].volume = volume;
					ChangedVolume[chan] = TRUE;
				//	firstvolume[chan] = FALSE;
					if(!cswrite && !showpianoroll) {
						e.time = Tcurr;
						e.type = NORMAL_EVENT;
						e.status = ControlChange + chan;
						e.data1 = VolumeControl[chan+1];
						e.data2 = volume;
						if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
						}
					}
				if(okpanoramic && (panoramic != (*p_Oldvalue)[chan].panoramic /* || firstpanoramic[chan] */)
						&& (j >= Jbol || (*p_OkPan)[j])) {
					(*p_Oldvalue)[chan].panoramic = panoramic;
					ChangedPanoramic[chan] = TRUE;
				//	firstpanoramic[chan] = FALSE;
					if(!cswrite && !showpianoroll) {
						e.time = Tcurr;
						e.type = NORMAL_EVENT;
						e.status = ControlChange + chan;
						e.data1 = PanoramicControl[chan+1];
						e.data2 = panoramic;
						if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
						}
					}
				if(okpitchbend && (pitchbend != (*p_Oldvalue)[chan].pitchbend /* || firstpitchbend[chan] */)) {
					(*p_Oldvalue)[chan].pitchbend = pitchbend;
					ChangedPitchbend[chan] = TRUE;
				//	firstpitchbend[chan] = FALSE;
					lsb = ((long)pitchbend) % 128;
					msb = (((long)pitchbend) - lsb) >> 7;
					if(!cswrite && !showpianoroll) {
						e.time = Tcurr;
						e.type = NORMAL_EVENT;
						e.status = PitchBend + chan;
						e.data1 = lsb;
						e.data2 = msb;
						if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
						}
					}
				if(okpressure && (pressure != (*p_Oldvalue)[chan].pressure /* || firstpressure[chan] */)) {
					(*p_Oldvalue)[chan].pressure = pressure;
					ChangedPressure[chan] = TRUE;
				//	firstpressure[chan] = FALSE;
					if(!cswrite && !showpianoroll) {
						e.time = Tcurr;
						e.type = TWO_BYTE_EVENT;
						e.status = ChannelPressure + chan;
						e.data2 = pressure;
						if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
						}
					}
				if(okmodulation && (modulation != (*p_Oldvalue)[chan].modulation /* || firstmodulation[chan] */)) {
					(*p_Oldvalue)[chan].modulation = modulation;
					ChangedModulation[chan] = TRUE;
				//	firstmodulation[chan] = FALSE;
					lsb = ((long)modulation) % 128;
					msb = (((long)modulation) - lsb) >> 7;
					if(!cswrite && !showpianoroll) {
						e.time = Tcurr;
						e.type = NORMAL_EVENT;
						e.status = ControlChange + chan;
						e.data1 = 1;
						e.data2 = msb;
						if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
						e.time = Tcurr;
						e.type = NORMAL_EVENT;
						e.status = ControlChange + chan;
						e.data1 = 33;
						e.data2 = lsb;
						if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
						}
					}
				}
			
			/* Recalculate t2 */
			
			t2obj = t2tick = Infpos;
			
			for(k=2; k <= (*p_kmax); k++) {
				if((*p_Instance)[k].object == 0) continue;
				if((*p_nextd)[k] < t2obj) {
					t2obj = (*p_nextd)[k];
					}
				}
			t2 = t2obj;
			icont = -1; chancont = -1;
			for(ch=0; ch < MAXCHAN; ch++) {
				for(index=0; index <= IPANORAMIC; index++) {
					if(!(*(p_active[ch]))[index]) continue;
					if(t2 >= (*(p_t2cont[ch]))[index]) {
						t2 = (*(p_t2cont[ch]))[index];
						icont = index;
						chancont = ch;
						}
					}
				}
			if(!MIDIfileOn && !cswrite && OutMIDI && PlayTicks && TickThere && !ItemCapture
					&& !showpianoroll) {
#if WITH_REAL_TIME_MIDI
				for(itick=0; itick < MAXTICKS; itick++) {
					if(tickdate[itick] < t2tick && tickposition[itick] != -1) {
						t2tick = tickdate[itick];
						fstreak = streakposition[itick];
						}
					}
#endif
				}
			if(t2 > t2tick && t2 != Infpos) t2 = t2tick;
		
			if(!cswrite && !MIDIfileOn && !showpianoroll && (result=CheckMIDIbytes(YES)) != OK
				&& result != RESUME) goto OVER;
			
			result = OK;
			
			/* Pass on parameters to pp_currentparams used by Csound */
			
			if((kcurrentinstance > 1) && (*p_Instance)[kcurrentinstance].contparameters.number > 0) {
				if(currentinstancevalues == NULL) {
					if(Beta) Alert1("Err. MakeSound(). currentinstancevalues == NULL");
					goto FORGETIT;
					}
				if((*((*pp_currentparams)[nseq]))->params == NULL) {
					if(Beta) Alert1("Err. MakeSound(). (*((*pp_currentparams)[nseq]))->params == NULL");
					maxparam = 10;
					if((ptrs=(ParameterStatus**) GiveSpace((Size)maxparam * sizeof(ParameterStatus))) == NULL)
						return(ABORT);
					(*((*pp_currentparams)[nseq]))->params = ptrs;
					for(index=0; index < maxparam; index++)
						(*params)[index].active = FALSE;
					}
				maxparam = MyGetHandleSize((Handle)(*((*pp_currentparams)[nseq]))->params)
					/ sizeof(ParameterStatus);
				params = (*((*pp_currentparams)[nseq]))->params;
				for(i=0; i < (*p_Instance)[kcurrentinstance].contparameters.number; i++) {
					index = (*currentinstancevalues)[i].index;
					if(index < 0) continue;
					if(index >= maxparam) {
						iparam = maxparam;
						maxparam = index + 1;
						h = (Handle) params;
						MySetHandleSize(&h,(Size)maxparam * sizeof(ParameterStatus));
						params = (*((*pp_currentparams)[nseq]))->params = (ParameterStatus**) h;
						for(iparam=iparam; iparam < maxparam; iparam++)
							(*params)[iparam].active = FALSE;
						}
					if(index >= (*((*pp_currentparams)[nseq]))->numberparams)
						(*((*pp_currentparams)[nseq]))->numberparams = index + 1;
						
					(*params)[index].startvalue = (*currentinstancevalues)[i].v0;
					(*params)[index].endvalue = (*currentinstancevalues)[i].v1;
					(*params)[index].imax = (*currentinstancevalues)[i].imax;
					(*params)[index].dur = objectduration;
					(*params)[index].starttime
							= t0 + (*p_Instance)[kcurrentinstance].starttime;
					(*params)[index].mode = (*currentinstancevalues)[i].mode;
					(*params)[index].point = (*currentinstancevalues)[i].point;
					if((index == IPANORAMIC && j < Jbol && !(*p_OkPan)[j])
								|| (index == IVOLUME && j < Jbol && !(*p_OkVolume)[j]))
						(*params)[index].active = FALSE;
					else
						(*params)[index].active = TRUE;
					}
				}
FORGETIT:
			if(kcurrentinstance > 1 && !showpianoroll) {
				Tcurr = (t0 + t1) / Time_res;
				rs = 0;
				/* Look at attached script line(s) */
				if((*p_ObjectSpecs)[kcurrentinstance] != NULL
						&& (scriptlist=ObjScriptLine(kcurrentinstance)) != NULL) {
				
					WhenItStarted = oldtime = clock();
					/* WhenItStarted may be further updated by WaitForEmptyBuffer()É */
					/* É if it is called by ExecuteScriptList() */
					
					if((result=ExecuteScriptList(scriptlist)) != OK && result != RESUME)
						goto OVER;
						
					if(!cswrite && !MIDIfileOn && !showpianoroll && (result=CheckMIDIbytes(YES)) != OK
						&& result != RESUME) goto OVER;
						
					LapWait += (clock() - WhenItStarted);
					if(ScriptExecOn && CountOn && WaitEndDate > ZERO)
						WaitEndDate += LapWait;
					
					/* This could be suppressed if a proper retiming during script execution occurs */
					/* (See -da.checkWaitUntilScript) */
					if((WhenItStarted != oldtime) && !MIDIfileOn && !cswrite && OutMIDI && !showpianoroll) {
#if WITH_REAL_TIME_MIDI
						SetDriverTime(Tcurr);
#endif
						}
					rs = 0;
					}
				
				/* Reset tick cycle if requested */
				Tcurr = (t0 + t1) / Time_res;
				if(!MIDIfileOn && !cswrite && OutMIDI && ResetTickInItemFlag && PlayTicks
						&& TickThere && !ItemCapture && !showpianoroll) {
#if WITH_REAL_TIME_MIDI
					ResetTicksInItem(t1,tickposition,streakposition,tickdate,clickon,
						hidden,imaxstreak,&rs,p_keyon);
					ResetTickInItemFlag = FALSE;
#endif
					}
					
#if BP_CARBON_GUI
				/* Look at synchro tags */
				if(!MIDIfileOn && !cswrite && OutMIDI && Interactive && !ItemCapture && !showpianoroll
						&& (*p_ObjectSpecs)[kcurrentinstance] != NULL
						&& (waitlist=WaitList(kcurrentinstance)) != NULL) {
#if WITH_REAL_TIME_MIDI
					WhenItStarted = clock();
					if((result=WaitForTags(waitlist)) != OK && result != RESUME)
						goto OVER;
					LapWait += (clock() - WhenItStarted);
					if(ScriptExecOn && CountOn && WaitEndDate > ZERO) WaitEndDate += LapWait;
					SetDriverTime(Tcurr);
#endif
					}
#endif /* BP_CARBON_GUI */
					
				if((*p_Instance)[kcurrentinstance].contparameters.number
						> (*p_control)[nseq].number) {
					(*p_control)[nseq].number = (*p_Instance)[kcurrentinstance].contparameters.number;
					h = (Handle) (*p_control)[nseq].param;
					MySetHandleSize((Handle*)&h,(*p_control)[nseq].number * sizeof(ParameterStream));
					(*p_control)[nseq].param = (ParameterStream**) h;
					}
				
				/* Look at continuous MIDI parameters */
				
				stream = (*p_control)[nseq].param;
				if(cswrite) goto SWITCHES;
				
				for(i=0; i < (*p_Instance)[kcurrentinstance].contparameters.number; i++) {
					index = (*currentinstancevalues)[i].index;
					if(index < 0) continue;
					if((index == IPANORAMIC && j < Jbol && !(*p_OkPan)[j])
							|| (index == IVOLUME && j < Jbol && !(*p_OkVolume)[j])) {
						(*stream)[index].ibm = -1L;
						continue;
						}
					if(index <= IPANORAMIC && (*currentinstancevalues)[i].mode == CONTINUOUS) {
						/* First terminate current control. May occur because of rounding errors */
						if((*stream)[index].ibm > -1L) {
							ch = (*stream)[index].channel;
							if((*(p_active[ch]))[index]) {
								if(SendControl(p_control,t0,ch,index,maxconc,cswrite,showpianoroll,&rs,
									p_active,p_t2cont,p_seqcont,p_Oldvalue,pp_currentparams) != OK) goto OVER;
								(*(p_active[ch]))[index] = FALSE;
								}
							(*stream)[index].ibm = -1L;
							}
						
						(*stream)[index].starttime
							= /* torigin + */ (*p_Instance)[kcurrentinstance].starttime;
						(*stream)[index].channel = ch = (*currentinstancevalues)[i].channel - 1;
						if(ch < 0 || ch >= MAXCHAN) {
							if(Beta) Alert1("Err. MakeSound(). ch < 0 || ch >= MAXCHAN");
							result = ABORT;
							goto OVER;
							}
						(*stream)[index].startvalue = (*currentinstancevalues)[i].v0;
						(*stream)[index].endvalue = (*currentinstancevalues)[i].v1;
						(*stream)[index].duration = objectduration;
						(*stream)[index].ibm = Findibm(index,(*stream)[index].duration,ch);
						(*stream)[index].ib = 0;
						(*stream)[index].imax = (*currentinstancevalues)[i].imax;
						if((*stream)[index].imax > ZERO) {
							(*stream)[index].point = (*currentinstancevalues)[i].point;
							}
						else (*stream)[index].point = NULL;
						if(!(*(p_active[ch]))[index] && (*stream)[index].ibm > ZERO) {
							(*(p_active[ch]))[index] = TRUE;
							(*(p_seqcont[ch]))[index] = nseq;
							if(SendControl(p_control,t0,ch,index,maxconc,cswrite,showpianoroll,&rs,
								p_active,p_t2cont,p_seqcont,p_Oldvalue,pp_currentparams) != OK) goto OVER;
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
				/* Look at switches */
				if((*p_ObjectSpecs)[kcurrentinstance] != NULL
						&& SwitchState(kcurrentinstance) != NULL) {
					Tcurr = (t0 + t1) / Time_res;
					for(ii=0; ii < MAXCHAN; ii++) {
						if(currswitchstate[ii] != (*(SwitchState(kcurrentinstance)))[ii]) {
							currswitchstate[ii] = (*(SwitchState(kcurrentinstance)))[ii];
							ChangedSwitch[ii] = TRUE;
							for(jj=0; jj < 32; jj++) {
								s = 127 * (currswitchstate[ii] & (1L << jj));
								if(s < 0 || s > 127) {
									if(Beta) Alert1("Err. MakeSound(). s < 0 || s > 127");
									s = 0;
									}
								rs = 0;
								if(!cswrite && !showpianoroll) {
									e.time = Tcurr;
									e.type = NORMAL_EVENT;
									e.status = ControlChange + ii;
									e.data1 = 64 + jj;
									e.data2 = s;
									if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
									}
								}
							}
						}
					}
				}
				
// Send dilation ratio to MIDI if applicable

			if(j < Jbol && (*p_AlphaCtrl)[j] && (*p_AlphaCtrlChan)[j] <= MAXCHAN
					&& (*p_AlphaCtrlNr)[j] < MAXPARAMCTRL) {
					
				alph = (int) (32. * log10(beta) + 64.);
				/* This converts beta to range [0.01,100] */
				
				if(alph < 0) alph = 0;
				if(alph > 127) alph = 127;
				alphach = (*p_AlphaCtrlChan)[j] - 1;
				if(alphach == 0) alphach = objectchannel;
				if(alphach < 0) alphach = 0;
				Tcurr =  (t0 + t1) / Time_res;
				if(!cswrite && !showpianoroll) {
					e.time = Tcurr;
					e.type = NORMAL_EVENT;
					e.status = ControlChange + alphach;
					e.data1 = ByteToInt((*p_AlphaCtrlNr)[j]);
					e.data2 = alph;
					if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
					}
				}

// Send initial messages if beginning of sound-object is truncated

			if(ievent > 0 && ievent < im && !PlayFromInsertionPoint
						&& !(cswrite && (*p_CsoundSize)[j] > 0 && !ConvertMIDItoCsound)) {
				Tcurr =  (t0 + t1) / Time_res;
				for(ii = 0; ii < ievent; ii++) {
					if(j < Jbol) {
						sequence = (*((*pp_MIDIcode)[j]))[ii].sequence;
						c0 = (*((*pp_MIDIcode)[j]))[ii].byte;
						simplenote = FALSE;
						}
					else {	/* Simple note */
						sequence = 0;
						if(ii == 0) c0 = NoteOn;
						else c0 = NoteOff;
						simplenote = TRUE;
						}
					/* Calculating MIDI channel */
					localchan = 0;
					if(ChannelEvent(c0) && j < Jbol) {
						localchan = c0 % 16;
						c0 -= localchan;
						}
					if(objectchannel > 0) localchan = objectchannel - 1;
					if(c0 >= 0) {
						if(c0 == NoteOn || c0 == NoteOff) {
							if(j < 16384) {
								sequence = (*((*pp_MIDIcode)[j]))[ii].sequence;
								c1 = (*((*pp_MIDIcode)[j]))[++ii].byte;		/* key number */
								c2 = (*((*pp_MIDIcode)[j]))[++ii].byte;		/* velocity */
								}
							else {	/* Simple note */
								sequence = 0;
								c1 = GoodKey(j);
								c2 = 127;
								ii += 2;
								}
			/*				if(!simplenote) { */
								if((*p_Instance)[kcurrentinstance].lastistranspose)
									TransposeKey(&c1,trans);
								c1 = ExpandKey(c1,(*p_Instance)[kcurrentinstance].xpandkey,
									(*p_Instance)[kcurrentinstance].xpandval);
								if(!(*p_Instance)[kcurrentinstance].lastistranspose)
									TransposeKey(&c1,trans);
			/*					} */
							if(c0 == NoteOff || c2 == 0) {
								if(j >= Jbol || (*p_OkMap)[j])
									c1 = RetrieveMappedKey(c1,kcurrentinstance,localchan,
										p_currmapped,maxmapped);
								onoff = ByteToInt((*p_keyon[localchan])[c1]);
								if(onoff > 0 || cswrite) {
									((*p_keyon[localchan])[c1])--;
									if((onoff == 1 || cswrite) && (j >= Jbol || !(*p_DiscardNoteOffs)[j]
											|| (*p_icycle)[kcurrentinstance] == (*p_Instance)[kcurrentinstance].ncycles)) {
										if(!cswrite && !showpianoroll) {
											e.time = Tcurr;
											e.type = NORMAL_EVENT;
											e.status = NoteOn + localchan;
											e.data1 = c1;
											e.data2 = 0;
											if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
											}
										if(cswrite)
											if((result=
		CscoreWrite(strikeagain,OFF,beta,(t0 + t1),-1,c1,c2,localchan,instrument,j,
			nseq,kcurrentinstance,pp_currentparams)) == ABORT) goto OVER;
										if(showpianoroll) {
#if BP_CARBON_GUI
											result = DrawPianoNote(c1,nseq,localchan,(t0 + t1),
												pp_currentparams,leftoffset,topoffset,hrect,
												minkey,maxkey,&graphrect,&overflow);
											if(result != OK || overflow) goto OVER;
#endif /* BP_CARBON_GUI */
											}
										}
									}
								}
							else {
								oldc1 = c1;
								if(j >= Jbol || (*p_OkMap)[j])
									c1 = MapThisKey(c1,howmuch,
										(*p_Instance)[kcurrentinstance].mapmode,
										&((*p_Instance)[kcurrentinstance].map0),
										&((*p_Instance)[kcurrentinstance].map1));
								onoff = ByteToInt((*p_keyon[localchan])[c1]);
								strike = TRUE;
								if(onoff > 0) {
									if(strikeagain) {
										/* First send NoteOff */
										if(!cswrite && !showpianoroll) {
											e.time = Tcurr;
											e.type = NORMAL_EVENT;
											e.status = NoteOn + localchan;
											e.data1 = c1;
											e.data2 = 0;
											if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
											}
										}
									else strike = FALSE;
									}
								((*p_keyon[localchan])[c1])++;
								if(strike || cswrite) {
									if(j >= Jbol || (*p_OkVelocity)[j])
										c2 = ClipVelocity(c2,localvelocity,
											(*p_Instance)[kcurrentinstance].velcontrol,
											(*p_Instance)[kcurrentinstance].rndvel);
									if(c1 != oldc1) {
										if((result=StoreMappedKey(oldc1,c1,kcurrentinstance,localchan,
											&p_currmapped,&maxmapped)) != OK) goto OVER;
										}
									if(!cswrite && !showpianoroll) {
										e.time = Tcurr;
										e.type = NORMAL_EVENT;
										e.status = c0 + localchan;
										e.data1 = c1;
										e.data2 = c2;
										if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
										}
									if(cswrite)
										if((result=
		CscoreWrite(strikeagain,ON,beta,(t0 + t1),-1,c1,c2,localchan,instrument,
			j,nseq,kcurrentinstance,pp_currentparams)) == ABORT) goto OVER;
									if(showpianoroll) {
#if BP_CARBON_GUI
										(*((*pp_currentparams)[nseq]))->starttime[c1] = (t0 + t1);
#endif /* BP_CARBON_GUI */
										}
									}
								}
							}
						else {
							if(ChannelEvent(c0)) {	/* Channel message */
								c0 = c0 + localchan;
								}
							if(ThreeByteEvent(c0)) {
								sequence = (*((*pp_MIDIcode)[j]))[ievent].sequence;
								c1 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
								c2 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
								if(!cswrite && !showpianoroll) {
									e.time = Tcurr;
									e.type = NORMAL_EVENT;
									e.status = c0;
									e.data1 = c1;
									e.data2 = c2;
									if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
									}
								t1 += (Milliseconds)(beta
									* ((*((*pp_MIDIcode)[j]))[ievent-1].time
										+ (*((*pp_MIDIcode)[j]))[ievent].time));
								}
							else {
								if(TwoByteEvent(c0)) {
									sequence = (*((*pp_MIDIcode)[j]))[ievent].sequence;
									c2 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
									if(!cswrite && !showpianoroll) {
										e.time = Tcurr;
										e.type = TWO_BYTE_EVENT;
										e.status = c0;
										e.data2 = c2;
										if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
										}
									t1 += (Milliseconds)(beta * (*((*pp_MIDIcode)[j]))[ievent].time);
									}
								else {
									if(c0 != TimingClock) {
									/* Suppress timing clock events internally used to create silences */
										rs = 0;
										if(!cswrite && !showpianoroll) {
											e.time = Tcurr;
											e.type = RAW_EVENT;
											e.data2 = c0;
											if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
											}
										}
									}
								}
							}
						}
					}
				}
			}
			
// Send messages of sound-object instance kcurrentinstance as long as possible

PLAYOBJECT:					
		while(t1 <= t2  && t1 <= t3  && ievent < im) {
			Tcurr =  (t0 + t1) / Time_res;
			
#if BP_CARBON_GUI
			// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
			// Does the call to MyButton() happen now that maxmidibytes5 = LONG_MAX / 5 ?
			if(Nbytes > maxmidibytes5 && !showpianoroll && (result=MyButton(1)) != FAILED) {
				interruptedonce = TRUE;
				if(result != OK || (result=InterruptSound()) != OK) goto OVER;
				}
#endif /* BP_CARBON_GUI */
			result = OK;
			
			if(objectduration > ZERO) howmuch = ((float)(t1 - objectstarttime)) / objectduration;
			else howmuch = 0.;
			
			/* Writing a Csound event taken from the Csound score of a sound-object */
			if(cswrite && j < Jbol && (*p_CsoundSize)[j] > 0 && !ConvertMIDItoCsound) {
				if((result=CscoreWrite(strikeagain,LINE,beta,(t0 + t1),ievent,0,0,0,0,j,
					nseq,kcurrentinstance,pp_currentparams)) == ABORT) goto OVER;
				goto NEWPERIOD;
				}
			
			if(j < Jbol) {
				sequence = (*((*pp_MIDIcode)[j]))[ievent].sequence;
				c0 = (*((*pp_MIDIcode)[j]))[ievent].byte;
				}
			else { /* Simple note or time pattern */
				if(ievent == 0) c0 = NoteOn;
				else c0 = NoteOff;
				sequence = 0;
				}
				
			/* Calculating MIDI channel */
			localchan = 0;
			if(ChannelEvent(c0) && j < 16384) {
				localchan = c0 % 16;
				c0 -= localchan;
				}
			if(objectchannel > 0) localchan = objectchannel - 1;
			
			if(c0 >= 0) {
				if(c0 == NoteOn  || c0 == NoteOff) {
					if(j < 16384) {
						sequence = (*((*pp_MIDIcode)[j]))[ievent].sequence;
						c1 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
						c2 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
						simplenote = FALSE;
						}
					else {	/* Simple note */
						sequence = 0;
						c1 = GoodKey(j);
						c2 = 127;
						ievent += 2;
						simplenote = TRUE;
						}
		/*			if(!simplenote) { */
						if((*p_Instance)[kcurrentinstance].lastistranspose)
							TransposeKey(&c1,trans);
						c1 = ExpandKey(c1,(*p_Instance)[kcurrentinstance].xpandkey,
							(*p_Instance)[kcurrentinstance].xpandval);
						if(!(*p_Instance)[kcurrentinstance].lastistranspose)
							TransposeKey(&c1,trans);
		/*				} */
					if(c0 == NoteOff || c2 == 0) {
						if(j >= Jbol || (*p_OkMap)[j])
							c1 = RetrieveMappedKey(c1,kcurrentinstance,localchan,p_currmapped,
								maxmapped);
						onoff = ByteToInt((*p_keyon[localchan])[c1]);
						if(onoff > 0 || cswrite) {
							((*p_keyon[localchan])[c1])--;
							if((onoff == 1 || cswrite) && (j >= Jbol || !(*p_DiscardNoteOffs)[j]
									|| (*p_icycle)[kcurrentinstance]
										== (*p_Instance)[kcurrentinstance].ncycles)) {
SENDNOTEOFF:
								if(!cswrite && !showpianoroll) {
									e.time = Tcurr;
									e.type = NORMAL_EVENT;
									e.status = NoteOn + localchan;
									e.data1 = c1;
									e.data2 = 0;
									if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
									}
								if(cswrite)
									if((result=
		CscoreWrite(strikeagain,OFF,beta,(t0 + t1),-1,c1,c2,localchan,instrument,
			j,nseq,kcurrentinstance,pp_currentparams)) == ABORT) goto OVER;
								if(showpianoroll) {
#if BP_CARBON_GUI
									result = DrawPianoNote(c1,nseq,localchan,(t0 + t1),
										pp_currentparams,leftoffset,topoffset,hrect,
										minkey,maxkey,&graphrect,&overflow);
									if(result != OK || overflow) goto OVER;
#endif /* BP_CARBON_GUI */
									}
								}
							}
						else {
							if((*p_istartperiod)[kcurrentinstance] > -1 && j < Jbol
								&& (*p_DiscardNoteOffs)[j]
									&& (*p_icycle)[kcurrentinstance]
										== (*p_Instance)[kcurrentinstance].ncycles) {
								/* Unbalanced NoteOn's/NoteOff's in cyclic object */
								goto SENDNOTEOFF;
								}
							}
						}
					else {
						oldc1 = c1;
						if(j >= Jbol || (*p_OkMap)[j])
							c1 = MapThisKey(c1,howmuch,(*p_Instance)[kcurrentinstance].mapmode,
								&((*p_Instance)[kcurrentinstance].map0),
								&((*p_Instance)[kcurrentinstance].map1));
						onoff = ByteToInt((*p_keyon[localchan])[c1]);
						strike = TRUE;
						if(onoff > 0) {
							if(strikeagain) {
								/* First send NoteOff */
								if(!cswrite && !showpianoroll) {
									e.time = Tcurr;
									e.type = NORMAL_EVENT;
									e.status = NoteOn + localchan;
									e.data1 = c1;
									e.data2 = 0;
									if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
									}
								}
							else strike = FALSE;
							}
						((*p_keyon[localchan])[c1])++;
						if(strike || cswrite) {
							if(j >= Jbol || (*p_OkVelocity)[j])
								c2 = ClipVelocity(c2,localvelocity,
									(*p_Instance)[kcurrentinstance].velcontrol,
									(*p_Instance)[kcurrentinstance].rndvel);
							if(c1 != oldc1) {
								if((result=StoreMappedKey(oldc1,c1,kcurrentinstance,localchan,
									&p_currmapped,&maxmapped)) != OK) goto OVER;
								}
							if(!cswrite && !showpianoroll) {
								e.time = Tcurr;
								e.type = NORMAL_EVENT;
								e.status = c0 + localchan;
								e.data1 = c1;
								e.data2 = c2;
								if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
								}
							if(cswrite)
								if((result=
		CscoreWrite(strikeagain,ON,beta,(t0 + t1),-1,c1,c2,localchan,instrument,
			j,nseq,kcurrentinstance,pp_currentparams)) == ABORT) goto OVER;
							if(showpianoroll) {
#if BP_CARBON_GUI
								(*((*pp_currentparams)[nseq]))->starttime[c1] = (t0 + t1);
#endif /* BP_CARBON_GUI */
								}
							}
						}
					/* Since ievent was incremented twice t1 should be updated accordingly */
					if(j < Jbol) {
						t1 += (Milliseconds)(beta
							* ((*((*pp_MIDIcode)[j]))[ievent-1].time
								+ (*((*pp_MIDIcode)[j]))[ievent].time));
						}
					Tcurr =  (t0 + t1) / Time_res;
					}
				else {
					if(ChannelEvent(c0)) {	/* Channel message */
						c0 = c0 + localchan;
						}
					if(ThreeByteEvent(c0)) {
						sequence = (*((*pp_MIDIcode)[j]))[ievent].sequence;
						c1 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
						c2 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
						if(!cswrite && !showpianoroll) {
							e.time = Tcurr;
							e.type = NORMAL_EVENT;
							e.status = c0;
							e.data1 = c1;
							e.data2 = c2;
							if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
							}
						t1 += (Milliseconds)(beta
							* ((*((*pp_MIDIcode)[j]))[ievent-1].time
								+ (*((*pp_MIDIcode)[j]))[ievent].time));
						}
					else {
						if(TwoByteEvent(c0)) {
							sequence = (*((*pp_MIDIcode)[j]))[ievent].sequence;
							c2 = (*((*pp_MIDIcode)[j]))[++ievent].byte;
							if(!cswrite && !showpianoroll) {
								e.time = Tcurr;
								e.type = TWO_BYTE_EVENT;
								e.status = c0;
								e.data2 = c2;
								if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
								}
							t1 += (Milliseconds)(beta * (*((*pp_MIDIcode)[j]))[ievent].time);
							}
						else {
							if(c0 != TimingClock) {
							/* Suppress timing clock events internally used to create silences */
								rs = 0;
								if(!cswrite && !showpianoroll) {
									e.time = Tcurr;
									e.type = RAW_EVENT;
									e.data2 = c0;
									if((result=SendToDriver((t0 + t1),nseq,&rs,&e)) != OK) goto OVER;
									}
								}
							}
						}
					}
				}
NEWPERIOD:
			if((*p_istartperiod)[kcurrentinstance] > -1 && ievent >= (*p_iendperiod)[kcurrentinstance]) {
				/* Cyclic object: start another period */
				t1 += (*p_periodgap)[kcurrentinstance];
				(*p_icycle)[kcurrentinstance]++;
				if(t1 <= t3 && (*p_icycle)[kcurrentinstance]
						<= (*p_Instance)[kcurrentinstance].ncycles) {
					ievent = (*p_istartperiod)[kcurrentinstance];
					continue;
					}
				else {
					ievent++;
					break;
					}
				}
			ievent++;
			if(ievent >= im) break;
			if(j < Jbol) {
				if(cswrite && (*p_CsoundSize)[j] > 0 && !ConvertMIDItoCsound)
					t1 += (Milliseconds)(beta * (*((*pp_CsoundTime)[j]))[ievent]);
				else t1 += (Milliseconds)(beta * (*((*pp_MIDIcode)[j]))[ievent].time);
				}
			else t1 += (Milliseconds)(beta * 1000.);
			}

// All messages have been sent. Let's see whether sound-object instance kcurrentinstance is over

		if(ievent < im) {
			(*p_inext)[kcurrentinstance] = ievent;
			if(t1 <= t3) (*p_nextd)[kcurrentinstance] = t1;
			else {
				(*p_nextd)[kcurrentinstance] = t3;	/* End of object is truncated */
				}
			}
		else {
			(*p_nextd)[kcurrentinstance] = Infpos;	/* Sound-object kcurrentinstance is OVER */
			doneobjects++;
			if(!MIDIfileOn && !cswrite && OutMIDI && !ItemCapture && !showpianoroll) {
#if WITH_REAL_TIME_MIDI
				if((result = ListenMIDI(0,0,0)) == ABORT || result == ENDREPEAT
					|| result == EXIT) goto OVER;
				if(result == AGAIN) {
					noccurrence--; /* Repeat once */
					sprintf(Message,"Current item will be played again...");
					ShowMessage(TRUE,wMessage,Message);
					}
#endif
				}
				
			else {
				if((MIDIfileOn || cswrite) && !showpianoroll && (!ItemCapture || ShowMessages)
						&& doneobjects > 10) {
					sprintf(Message,"%ld objects done out of %ld",
						(long)kcurrentinstance,(long)(*p_kmax));
					FlashInfo(Message);
					doneobjects = 0;
					}
					
				}
			}

		if(!cswrite && !showpianoroll && (result=CheckMIDIbytes(YES)) != OK
												&& result != RESUME) goto OVER;
#if BP_CARBON_GUI
		// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
		// Does the call to MyButton() happen now that maxmidibytes5 = LONG_MAX / 5 ?
		if(Nbytes > maxmidibytes5 && !showpianoroll && (result=MyButton(1)) != FAILED) {
			interruptedonce = TRUE;
			if(result != OK || (result=InterruptSound()) != OK) goto OVER;
			}
#endif /* BP_CARBON_GUI */
		result = OK;
		
		if(Beta && (chancont >= MAXCHAN || icont > IPANORAMIC)) {
			Alert1("Err. MakeSound(). chancont >= MAXCHAN || icont > IPANORAMIC");
			goto OVER;
			}
		if(cswrite || chancont < 0 || icont < 0 || !(*(p_active[chancont]))[icont])
			goto FINDNEXTEVENT;

SENDCONTROLMESSAGE:
// Send message of continuous parameter
		
		if(Beta && (chancont < 0 || icont < 0 || chancont >= MAXCHAN || icont > IPANORAMIC)) {
			Alert1("Err. MakeSound(). chancont < 0 || icont < 0 || chancont >= MAXCHAN || icont > IPANORAMIC");
			goto OVER;
			}
		if(Beta && (!(*(p_active[chancont]))[icont])) {
			Alert1("Err. MakeSound(). !(*(p_active[chancont]))[icont]");
			goto OVER;
			}
		if(t2 != Infpos && t2 == (*(p_t2cont[chancont]))[icont])
			if(SendControl(p_control,t0,chancont,icont,maxconc,
					cswrite,showpianoroll,&rs,p_active,p_t2cont,p_seqcont,p_Oldvalue,pp_currentparams) != OK)
				goto OVER;


FINDNEXTEVENT:
// Look for the next event: in a sound-object instance, a tick, or a continous parameter

		t2obj = t2tick = Infpos;
		kcurrentinstance = 0;
		
		for(k=2; k <= (*p_kmax); k++) {
			if((*p_Instance)[k].object == 0) continue;
			if((*p_nextd)[k] < t2obj) {
				kcurrentinstance = k;
				t2obj = (*p_nextd)[k];
				}
			}
			
		t2 = t2obj;
		nextisobject = TRUE;
		
		icont = -1; chancont = -1;
		for(ch=0; ch < MAXCHAN; ch++) {
			for(index=0; index <= IPANORAMIC; index++) {
				if(!(*(p_active[ch]))[index]) continue;
				if(t2 >= (*(p_t2cont[ch]))[index]) {
					t2 = (*(p_t2cont[ch]))[index];
					icont = index;
					chancont = ch;
					nextisobject = FALSE;
					}
				}
			}
		
		if(!MIDIfileOn && !cswrite && OutMIDI && PlayTicks && TickThere && !ItemCapture
				&& !showpianoroll) {
#if WITH_REAL_TIME_MIDI
			for(itick=0; itick < MAXTICKS; itick++) {
				if(tickdate[itick] < t2tick && tickposition[itick] != -1)  {
					t2tick = tickdate[itick];
					fstreak = streakposition[itick];
					}
				}
#endif
			}
			
		if(t2 > t2tick && t2 != Infpos) {
			t2 = t2tick; nextisobject = FALSE;
			}
			
		if(!nextisobject && t2 != Infpos) {
			// Tick performance is not with controls because objects have priority on it.
			// This allows HideTicks to be taken into account before the tick is played.
			if(t2 == t2tick) {
				InsertTickInItem(fstreak,clickon,hidden,tickdate,&rs,p_keyon,streakposition,
					t0,tickposition,imaxstreak);
				goto FINDNEXTEVENT;
				}
			goto SENDCONTROLMESSAGE;
			}
		else {
			/* This will take us back to the preceding object */
			if(kcurrentinstance > 1) t1 = (*p_nextd)[kcurrentinstance];
			}
		/* Here we go back to sound-object instance kcurrentinstance */
		}


// The item has been sent entirely.

	ItemOutPutOn = FALSE;
	
	/* Terminate ticks */
	if(!MIDIfileOn && !cswrite && OutMIDI && PlayTicks && TickThere && !ItemCapture
			&& !showpianoroll) {
#if WITH_REAL_TIME_MIDI
		for(itick=0; itick < MAXTICKS; itick++) {
			if(clickon[itick] && !hidden[itick] && tickdate[itick] < DBL_MAX) {
			
				/* Send NoteOff if allowed */
				c1 = TickKey[itick];
				localchan = TickChannel[itick] - 1;
				onoff = ByteToInt((*p_keyon[localchan])[c1]);
				if(onoff > 0) {
					((*p_keyon[localchan])[c1])--;
					if(onoff == 1) {
						e.time = (t0 + tickdate[itick]) / Time_res;
						e.type = NORMAL_EVENT;
						e.status = NoteOn + localchan;
						e.data1 = c1;
						e.data2 = 0;
						if((result=SendToDriver((t0 + tickdate[itick]),itick,&rs,&e)) != OK) goto OVER;
						}
					}
				}
			hidden[itick] = clickon[itick] = FALSE;
			}
#endif
		}
	currenttime = Tcurr * Time_res;
	mustwait = TRUE;
	}

// End of the Nplay performances

// Now, codes have been sent and may still being played by the MIDI
// driver.  Before proceeding further we might wait until the buffer is empty
// or not too much filled.

LastTcurr = Tcurr;

#if BP_CARBON_GUI
if(showpianoroll) goto OUTGRAPHIC;
#endif /* BP_CARBON_GUI */
	
buffertime = Infpos;
if(OutMIDI && !cswrite && !MIDIfileOn && !CyclicPlay && !ItemCapture && !showpianoroll
		&& (!Improvize || !ComputeWhilePlay)) {
#if WITH_REAL_TIME_MIDI
	waitcompletion = TRUE;
	if(Improvize) {
		buffertime = (MaxComputeTime * 3L) / 2L;
		if(ShowMessages) {
			sprintf(Message,"Maximum compute time: %.1f sec.",
									((double) (MaxComputeTime * Time_res)) / 1000.);
			ShowMessage(TRUE,wMessage,Message);
			}
		}
#else 
	waitcompletion = FALSE;
#endif
	}
else waitcompletion = FALSE;

SynchroSignal = OFF;

RunningStatus = 0;	/* This is used by ListenMIDI() */

if(showpianoroll || (!waitcompletion && !CyclicPlay)) {
	result = OK;
	goto OVER;
	}

result = OK;
if(buffertime < Infpos) result = WaitForLastSounds((long) buffertime);
else if(waitcompletion) result = WaitForEmptyBuffer();
if(result != OK) goto OVER;

#if BP_CARBON_GUI
HideWindow(Window[wMessage]);

QUESTION:

HideWindow(Window[wInfo]);
rep = OK; rep1 = rep2 = rep3 = NO;
if(!CyclicPlay && !ScriptExecOn) StopWait();
SndSetSysBeepState(sysBeepDisable);
ShowDuration(YES);
if(!FirstTime && !PlayPrototypeOn
	&& (CyclicPlay 
		|| (!Improvize && !ScriptExecOn && (DisplayItems || PlaySelectionOn) && OkWait
			&& !NoRepeat && (rep1=rep=Answer("Play again",'N')) != NO)
		|| (Improvize && interruptedonce
			&& !NoRepeat && (rep1=rep=Answer("Play again",'N')) != NO)
		|| ((WriteMIDIfile || OutCsound)
			&& (rep2=rep=Answer("Save to file",'Y')) != NO)
		|| (!Improvize && !ScriptExecOn && !DisplayItems && !PlaySelectionOn
			&& ((rep3=Alert(DisplayItemAlert,0L)) || TRUE)))) {
	if(rep == ABORT || rep3 == dCancelPlay) {
		result = ABORT; buffertime = ZERO; goto OVER;
		}
	if(rep3 == dProduceMore) {
		result = RESUME; goto OVER;
		}
	if(rep3 == dDisplayItem) {
		if(*pp_b == NULL) {
			if(Beta) Alert1("Err. MakeSound(). *pp_b == NULL");
			}
		else {
			BPActivateWindow(SLOW,wData);
			PrintArg(DisplayMode(pp_b,&ifunc,&hastabs),FALSE,TRUE,FALSE,ifunc,FALSE,stdout,wData,pp_Scrap,pp_b);
			DataEnd = GetTextLength(wData);
			SetSelect(DataOrigin,GetTextLength(wData)-1,TEH[wData]);
			ShowSelect(CENTRE,wData);
			Dirty[wData] = TRUE;
			}
		goto QUESTION;
		}
	for(k=2; k <= (*p_kmax); k++) {
		(*p_inext)[k] = (*p_inext1)[k];
		(*p_onoff)[k] = FALSE;
		(*p_nextd)[k] = (*p_Instance)[k].starttime + (*p_t1)[k];
		}
	if(rep1 == YES || rep3 == dPlayAgain) {
		while(Button());
		result = OK;
		ResetDriver();
		}
	if((WriteMIDIfile || OutCsound) && (rep2 == YES)) {
		if(WriteMIDIfile) MIDIfileOn = TRUE;
		if(OutCsound) cswrite = TRUE;
		}
	if(CyclicPlay) {
		result = WaitForLastSounds(200L);	/* Wait until 2 seconds left */
		if(result != OK) goto OVER;
		}
	if(!Newstatus || MIDIfileOn || cswrite) goto START2;
	else {
		Newstatus = FALSE;
		result = AGAIN;
		goto OVER;
		}
	}
if(rep == ABORT) result = ABORT;
#else
// seems likely if result == OK here, then MakeSound() will end properly in non-GUI build  
#endif /* BP_CARBON_GUI */

OVER:
HideWindow(Window[wMessage]);
HideWindow(Window[wInfo]);
if(result == ENDREPEAT) result = OK;
if(result == STOP) result = ABORT;

OUTGRAPHIC:
if(showpianoroll) {
#if BP_CARBON_GUI
	CloseGraphic(w,endxmax,endymax,overflow,&graphrect,&port,gdh);
	if(!overflow) DrawNoteScale(w,minkey,maxkey,hrect,leftoffset,topoffset);
#if NEWGRAF
	if(Offscreen) UnlockPixels(GetGWorldPixMap(gMainGWorld));
#endif
	if(saveport != NULL) SetPort(saveport);
	else if(Beta) Alert1("Err MakeSound(). saveport == NULL");
	if(ShowMessages) ClearMessage();
	goto GETOUT;
#endif /* BP_CARBON_GUI */
	}

if(!cswrite && !Panic && (result == RESUME || (!Improvize && !CyclicPlay))) { // FIXME ? Test OutMIDI==TRUE ?
#if WITH_REAL_TIME_MIDI	// FIXME: is this correct to not compile all of this?
	for(ch=0; ch < MAXCHAN; ch++) {
		rs = 0;
		/* Reset keys */
		for(k=0; k < MAXKEY; k++) {
			if((*p_keyon[ch])[k] > 0) {
				e.time = Tcurr;
				e.type = NORMAL_EVENT;
				e.status = NoteOn + ch;
				e.data1 = k;
				e.data2 = 0;
				if(SendToDriver(Tcurr * Time_res,0,&rs,&e) != OK) {
					result = ABORT;
					goto GETOUT;
					}
				}
			}
		/* Switch off switches */
		rs = 0;
		if(currswitchstate[ch] != ZERO) {
			for(jj=0; jj < 32; jj++) {
				if(currswitchstate[ch] & (1L << jj)) {
					e.time = Tcurr;
					e.type = NORMAL_EVENT;
					e.status = ControlChange + ch;
					e.data1 = 64 + jj;
					e.data2 = 0;
					if(SendToDriver(Tcurr * Time_res,0,&rs,&e) != OK) {
						result = ABORT;
						goto GETOUT;
						}
					}
				}
			}
	 	}
	while(Button());
	if(!MIDIfileOn && !cswrite && OutMIDI && !showpianoroll && result != ABORT && result != EXIT
		&& (r=WaitForLastSounds((long) buffertime)) != OK && r != ABORT) result = r;
	HideWindow(Window[wMessage]);
	if(!MIDIfileOn && !cswrite && OutMIDI && !showpianoroll && !ScriptExecOn
									&& (result == ABORT || result == EXIT))
		if((r=ResetMIDI(FALSE)) == EXIT) result = r;
#endif
	}

GETOUT:

if(EventState == AGAIN) result = AGAIN;

if(cswrite && result == OK) {
	if(!ConvertMIDItoCsound) {
		WriteToFile(NO,CsoundFileFormat,"s\n",CsRefNum);
		if(CsoundTrace) Println(wTrace,"s\n");
		}
	else {
		Println(wPrototype7,"e");
		ShowSelect(CENTRE,wPrototype7);
		}
	}

for(ch=0; ch < MAXCHAN; ch++) {
	h = (Handle) p_keyon[ch];
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

if(!MIDIfileOn && !cswrite && OutMIDI && !showpianoroll) {
#if WITH_REAL_TIME_MIDI
	drivertime = GetDriverTime();
	if(!FirstTime) ComputeStart = drivertime;
#endif
	}
	
MIDIfileOn = FALSE;

#if BP_CARBON_GUI
if(showpianoroll) Tcurr = oldtcurr;
#endif /* BP_CARBON_GUI */

if(cswrite && CsoundTrace) ShowSelect(CENTRE,wTrace);
Interrupted = FALSE;
#if BP_CARBON_GUI
SndSetSysBeepState(sysBeepEnable);
#endif
return(result);
}


ExecuteScriptList(p_list **scriptlist)
{
int x,r,idummy;
p_list **ptag;
long jdummy;

ptag = scriptlist;
r = OK;
ScriptExecOn++;
do {
	x = (**ptag).x;
	ptag = (**ptag).p;
	if((r=ExecScriptLine(NULL,wScript,FALSE,TRUE,(*p_Script)[x],jdummy,&jdummy,
			&idummy,&idummy)) != OK) goto OUT;
	}
while(ptag != NULL);

OUT:
if(ScriptExecOn > 0) ScriptExecOn--;
return(r);
}


ClipVelocity(int v,int localvelocity,int control,int rndvel)
{
int r;
float x;

if(control >= MAXPARAMCTRL) {
	sprintf(Message,"Err. ClipVelocity(). control = %ld",(long)control);
	if(Beta) Alert1(Message);
	control = -1;
	}
if(control > -1) v = ParamValue[control];
if(rndvel > 0) {
	if(rndvel > 127) r = ParamValue[rndvel-128];
	else r = rndvel;
	x = (2. * rand() * ((float)r)) / ((float)RAND_MAX);
	if(x > r) x = r - x;
	UsedRandom = TRUE;
	localvelocity += x;
	if(localvelocity < 1) localvelocity = 1;
	}
v = (v * localvelocity) / 127;
if(v > 127) v = 127;
return(v);
}


ChannelConvert(int ch)
{
int x;

if(ch < 0) ch += 256;
if(ch < 128) {
	if(ch > MAXCHAN) {
		sprintf(Message,"Trying to assign channel #%ld.\nValue should be 1..%ld",
			(long)ch,(long)MAXCHAN);
		Alert1(Message);
		return(ABORT);
		}
	return(ch);	/* Fixed channel */
	}
/* Channel is determined by Kx */
x = ch - 128;
if(x < 1 || x >= MAXPARAMCTRL) {
	sprintf(Message,"Trying to fix channel with incorrect K%ld.\nValue should be 1..%ld",
		(long)x,(long)MAXPARAMCTRL-1L);
	Alert1(Message);
	return(ABORT);
	}
ch = ParamValue[x];
if(ch < 1 || ch > MAXCHAN) {
	if(ParamControl[x] >= 0) {
		sprintf(Message,"Trying to assign channel #%ld by K%ld (MIDI controller #%ld).\nValue should be 1..%ld",
			(long)ch,(long)x,(long)ParamControl[x],(long)MAXCHAN);
		}
	else {
		if(ParamKey[x] >= 0) {
			sprintf(Message,"Trying to assign channel #%ld by K%ld (Key #%ld).\nValue should be 1..%ld",
				(long)ch,(long)x,(long)ParamKey[x],(long)MAXCHAN);
			}
		else {
		sprintf(Message,"Trying to assign channel #%ld by K%ld.\nValue should be 1..%ld",
			(long)ch,(long)x,(long)MAXCHAN);
			}
		}
	Alert1(Message);
	return(ABORT);
	}
return(ch);
}
								
								
double ContinuousParameter(Milliseconds t,int paramseq,ControlStream **p_stream)
{
double param;

param = (*p_stream)[paramseq].startvalue
		+ ((double)(*p_stream)[paramseq].difference * (t - (*p_stream)[paramseq].starttime))
			/ (*p_stream)[paramseq].dur;
return(param);
}


WaitForLastSounds(long buffertime)
// buffertime * Time_res is the remaining time after which we'll stop waiting.
{
int result,r,rep,compiledmem;
long timeleft,formertime;
unsigned long drivertime;

if(!OutMIDI || Panic) return(OK);

#if WITH_REAL_TIME_MIDI
result = OK;
formertime = ZERO;
drivertime = GetDriverTime();
WaitOn++;
while(Button() || (timeleft = (Tcurr - drivertime)) > buffertime) {
#if BP_CARBON_GUI	/* not sure about cutting out this code ... akozar 20130817 */
	if(MyButton(0)) {
		StopCount(0);
		SetButtons(TRUE);
		compiledmem = CompiledGr;
		while((rep = MainEvent()) != RESUME && rep != STOP && rep != EXIT && rep != ABORT);
		if(rep == EXIT || rep == ABORT) {
			result = rep;
			goto OVER;
			}
		if(rep == STOP || (compiledmem && !CompiledGr)) {
			ResetMIDI(!(Oms || NEWTIMER));	/* Added 8/3/98 */
			result = ABORT;
			goto OVER;
			}
		if(LoadedIn && (!CompiledIn && (result=CompileInteraction()) != OK))
			goto OVER;
		}
#endif /* BP_CARBON_GUI */
	if((timeleft * Time_res / 1000L) != formertime) {
		formertime = timeleft * Time_res / 1000L;
		sprintf(Message,"Remaining performance time: %ld sec...",
			(long)formertime + 1L);
		ShowMessage(FALSE,wMessage,Message);
		PleaseWait();
		}
#if BP_CARBON_GUI
	if(EventState != NO) {
		result = EventState;
		goto OVER;
		}
#endif /* BP_CARBON_GUI */
	if((result = ListenMIDI(0,0,0)) != OK && result != RESUME && result != QUICK
			&& result != ENDREPEAT) break;
	if(result == EXIT || result == ABORT) break;
	
	drivertime = GetDriverTime();
	}
#if BP_CARBON_GUI
if(EventState != NO) result = EventState;
#endif /* BP_CARBON_GUI */

OVER:
if(WaitOn > 0) WaitOn--;
else if(Beta) Alert1("Err. WaitForLastSounds(). WaitOn <= 0");
ClearMessage();
return(result);
#endif
}


WaitForEmptyBuffer(void)
{
long timeleft,formertime;
int result,rep,compiledmem;
unsigned long drivertime;

#if BP_CARBON_GUI
if(!OutMIDI || MIDIfileOn) {
	if(MIDIfileOn && (result=MyButton(0)) != FAILED) {
		return(Answer("Continue writing MIDI file",'Y'));
		}
	return(OK);
	}
#endif /* BP_CARBON_GUI */

#if WITH_REAL_TIME_MIDI
WhenItStarted += 1L;	/* Change it slightly so that BP2 remembers WaitForEmptyBuffer() has been called */
if(Nbytes == ZERO || Panic) return(OK);

result = OK;
formertime = ZERO;
drivertime = GetDriverTime();
WaitOn++;
while((timeleft=(Tcurr - drivertime)) >  (CLOCKRES * Oms) /* (10 * CLOCKRES * Oms) */) {
	if((timeleft * Time_res / 1000L) != formertime) {
		formertime = timeleft * Time_res / 1000L;
		sprintf(Message,"Waiting for end of play (%ld sec)",(long)formertime + 1L);
		ShowMessage(FALSE,wMessage,Message);
		PleaseWait();
		}
#if BP_CARBON_GUI
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	if((rep=MyButton(0)) != FAILED) {
		StopCount(0);
		SetButtons(TRUE);
		compiledmem = CompiledGr;
		if(rep == OK)
			while((rep = MainEvent()) != RESUME && rep != STOP && rep != EXIT && rep != ABORT);
		if(rep == EXIT || rep == ABORT) {
			result = rep; goto OVER;
			}
		if(rep == STOP || (compiledmem && !CompiledGr)) {
			result = ABORT;
			goto OVER;
			}
		if(LoadedIn && (!CompiledIn && (result=CompileInteraction()) != OK))
			goto OVER;
		}
	rep = OK;
	if(EventState != NO) {
		result = EventState; goto OVER;
		}
#endif /* BP_CARBON_GUI */
	if((result = ListenMIDI(0,0,0)) == ABORT
		|| result == EXIT || result == ENDREPEAT || result == QUICK) goto OVER;
#if BP_CARBON_GUI
	if(EventState != NO) {
		result = EventState; goto OVER;
		}
#endif /* BP_CARBON_GUI */
	drivertime = GetDriverTime();
	}
Nbytes = 0; Tbytes2 = ZERO;
	
OVER:
if(WaitOn > 0) WaitOn--;
HideWindow(Window[wMessage]);
WhenItStarted = clock();
return(result);
#endif /* WITH_REAL_TIME_MIDI */
}


#if BP_CARBON_GUI
InterruptSound(void)
{
int result,compiledmem;

StopCount(0);
SetButtons(TRUE);
compiledmem = CompiledGr;


if(MIDIfileOn && Answer("Continue writing MIDI file",'Y') != OK) return(ABORT);

while((result = MainEvent()) != RESUME && result != STOP && result != EXIT);

if(result == EXIT) return(EXIT);
if(result == STOP || (compiledmem && !CompiledGr)) return(ABORT);
if(LoadedIn && (!CompiledIn && (result=CompileInteraction()) != OK)) return(result);
return(OK);
}
#endif /* BP_CARBON_GUI */


long Findibm(int index,Milliseconds dur,int chan)
{
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


SendControl(ContinuousControl **p_control,Milliseconds t0,int chan,int iparam,int maxconc,
	int cswrite,int showpianoroll,int *p_rs,char ***p_active,Milliseconds ***p_t2cont,
	int ***p_seqcont,MIDIcontrolstatus **p_Oldvalue,PerfParameters ****pp_currentparams)
{
int seq,lsb,msb,nn,result;
double value,alpha;
MIDI_Event e;
Milliseconds time,t2;
ParameterStream **param;

result = OK;
if(iparam < 0 || iparam > IPANORAMIC) {
	if(Beta) Alert1("Err. SendControl(). iparam < 0 || iparam > IPANORAMIC");
	return(ABORT);
	}
if(chan < 0 || chan >= MAXCHAN) {
	if(Beta) Alert1("Err. SendControl(). chan < 0 || chan >= MAXCHAN");
	result = ABORT;
	goto OVER;
	}
seq = (*(p_seqcont[chan]))[iparam];

if(seq < 0 || seq >= maxconc) {
	if(Beta) Alert1("Err. SendControl(). seq < 0 || seq >= maxconc");
	return(ABORT);
	}
	
param = (*p_control)[seq].param;

if(chan != (*param)[iparam].channel) {
	if(Beta) Alert1("Err. SendControl(). chan != (*param)[iparam].channel");
	result = ABORT;
	goto OVER;
	}
if(!(*(p_active[chan]))[iparam]) {
	if(Beta) Alert1("Err. SendControl(). !(*(p_active[chan]))[iparam]");
	return(ABORT);
	}
if((*p_control)[seq].param == NULL) {
	if(Beta) Alert1("Err. SendControl(). (*p_control)[seq].param == NULL");
	return(ABORT);
	}
if((*param)[iparam].ibm <= ZERO) {
	if(Beta) Print(wTrace,"Err. SendControl(). (*param)[iparam].ibm <= ZERO\n");
/*	This case was found in item #26 of -da.checkControls.html when played after the
	two preceding ones */
	goto INCREMENT;
	}

alpha = ((double)(*param)[iparam].ib) / (*param)[iparam].ibm;

if((value = GetTableValue(alpha,(*param)[iparam].imax,(*param)[iparam].point,
		(*param)[iparam].startvalue,(*param)[iparam].endvalue)) == Infpos) {
	if(Beta) Alert1("Err. SendControl(). value == Infpos");
	return(ABORT);
	}
if(value < 0. && value > -0.1) value = 0.;
if(value > 16383. && value < 16384.) value = 16383.;

if(value < 0. || value > 16383.) {
	if(Beta) {
		sprintf(Message,"Err. SendControl(). value = %.2f\n",value);
		Print(wTrace,Message);
		}
	return(OK);
	}

time = t0 + (*param)[iparam].starttime
	+ ((long)(*param)[iparam].duration
	* (*param)[iparam].ib) / (*param)[iparam].ibm;

Tcurr =  time / Time_res;

lsb = ((long)value) % 128;
msb = (((long)value) - lsb) >> 7;

switch(iparam) {
	case IPITCHBEND:
		ChangedPitchbend[chan] = TRUE;
		(*p_Oldvalue)[chan].pitchbend = value;
		if(!cswrite && !showpianoroll) {
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = PitchBend + chan;
			e.data1 = lsb;
			e.data2 = msb;
			if((result=SendToDriver(time,seq,p_rs,&e)) != OK) goto OVER;
			}
		break;
	case IPRESSURE:
		ChangedPressure[chan] = TRUE;
		(*p_Oldvalue)[chan].pressure = value;
		if(!cswrite && !showpianoroll) {
			e.time = Tcurr;
			e.type = TWO_BYTE_EVENT;
			e.status = ChannelPressure + chan;
			e.data2 = value;
			if((result=SendToDriver(time,seq,p_rs,&e)) != OK) goto OVER;
			}
		break;
	case IMODULATION:
		ChangedModulation[chan] = TRUE;
		(*p_Oldvalue)[chan].modulation = value;
		if(!cswrite && !showpianoroll) {
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + chan;
			e.data1 = 1;
			e.data2 = msb;
			if((result=SendToDriver(time,seq,p_rs,&e)) != OK) goto OVER;
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + chan;
			e.data1 = 33;
			e.data2 = lsb;
			if((result=SendToDriver(time,seq,p_rs,&e)) != OK) goto OVER;
			}
		break;
	case IVOLUME:
		ChangedVolume[chan] = TRUE;
		(*p_Oldvalue)[chan].volume = value;
		if(!cswrite && !showpianoroll) {
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + chan;
			e.data1 = VolumeControl[chan+1];
			e.data2 = value;
			if((result=SendToDriver(time,seq,p_rs,&e)) != OK) goto OVER;
			}
		break;
	case IPANORAMIC:
		ChangedPanoramic[chan] = TRUE;
		(*p_Oldvalue)[chan].panoramic = value;
		if(!cswrite && !showpianoroll) {
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + chan;
			e.data1 = PanoramicControl[chan+1];
			e.data2 = value;
			if((result=SendToDriver(time,seq,p_rs,&e)) != OK) goto OVER;
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
	
	/* Try to activate the same parameter+channel in another sequence */
	for(nn=0; nn < maxconc; nn++) {
		if(nn == seq) continue;
		if((*((*p_control)[nn].param))[iparam].ibm <= ZERO) continue;
		if(chan == (*((*p_control)[nn].param))[iparam].channel) {
			if((*(p_active[chan]))[iparam]) {
				if(Beta) Alert1("Err. SendControl(). (*(p_active[chan]))[iparam]");
				return(ABORT);
				}
			
			/* Set the proper ib using value of t2 */
			if((*((*p_control)[nn].param))[iparam].duration <= ZERO) {
				if(Beta) Alert1("Err. SendControl(). (*((*p_control)[nn].param))[iparam].duration <= 0");
				(*((*p_control)[nn].param))[iparam].ib = ZERO;
				goto OVER;
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

OVER:
return(result);
}


double GetTableValue(double alpha,long imax,Coordinates** coords,double startvalue,
																		double endvalue)
{
double x,y,y1,y2;
long i,xmax,x1,x2;

if(alpha < 0.) {
	if(Beta && alpha < -0.01) Alert1("Err. GetTableValue(). alpha < -0.01");
	alpha = 0.;
	}
if(alpha > 1.) {
	if(Beta && alpha > 1.01) Alert1("Err. GetTableValue(). alpha > 1.01");
	alpha = 1.;
	}
if(imax == ZERO) {
	/* No table: just interpolate */
	y = startvalue + alpha * (endvalue - startvalue);
	return(y);
	}

if(imax < ZERO) {
	if(Beta) Alert1("Err. GetTableValue(). imax < ZERO || imax > 255");
	return(Infpos);
	}
if(coords == NULL) {
	if(Beta) Alert1("Err. GetTableValue(). coords == NULL");
	return(Infpos);
	}
if(Beta) {
	if(imax > MyGetHandleSize((Handle)coords) / sizeof(Coordinates)) {
		Alert1("Err. GetTableValue(). imax >= MyGetHandleSize((Handle)coords) / sizeof(Coordinates)");
		return(Infpos);
		}
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
		if(Beta) Alert1("Err. GetTableValue(). x1 >= x2");
		return(Infpos);
		}
	y = y1 + (y2 - y1) * (x - x1) / (x2 - x1);
	}
return(y);
}


GoodKey(int j)
{
int key;

key = j - 16384;
if(key < 0 || key > 127) {
	Println(wTrace,"MIDI key out of range");
	while(key < 0) key += 12;
	while(key > 127) key -= 12;
	}
return(key);
}


StoreMappedKey(int orgkey,int imagekey,int k,int channel,
	MappedKey ***pp_currmapped,long *p_maxmapped)
{
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


RetrieveMappedKey(int orgkey,int k,int channel,MappedKey **p_currmapped,long maxmapped)
{
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


MapThisKey(int key,float howmuch,char mapmode,KeyNumberMap *p_map0,KeyNumberMap *p_map1)
{
KeyNumberMap map;	/* Fixed 24/2/99 - was 'p_map' */
	
if(mapmode == OFF) return(key);
if(mapmode == FIXED || mapmode == STEPWISE) return(KeyImage(key,p_map0));
map.p1 = p_map0->p1 + howmuch * (p_map1->p1 - p_map0->p1);
map.q1 = p_map0->q1 + howmuch * (p_map1->q1 - p_map0->q1);
map.p2 = p_map0->p2 + howmuch * (p_map1->p2 - p_map0->p2);
map.q2 = p_map0->q2 + howmuch * (p_map1->q2 - p_map0->q2);
return(KeyImage(key,&map));
}


KeyImage(int key,KeyNumberMap *p_map)
{
int c;

if(key < p_map->p1 || key > p_map->p2) return(key);
if(p_map->p1 == p_map->p2) return(key);

c = p_map->q1
		+ Round(((double)(key - p_map->p1) * (p_map->q2 - p_map->q1))
									/ ((double)p_map->p2 - p_map->p1));
if(c < 0) {
	if(Beta) Println(wTrace,"\nErr. KeyImage(). c < 0");
	c = 0;
	}
if(c > 127) {
	if(Beta) Println(wTrace,"\nErr. KeyImage(). c > 127");
	c = 127;
	}
return(c);
}