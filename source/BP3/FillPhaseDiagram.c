/* FillPhaseDiagram.c (BP3) */

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

int trace_diagram = 0;
int trace_toofast = 0;
int trace_overstrike = 0;
int new_thing = 1; // This change should be confirmed (BB 2022-02-17)

int FillPhaseDiagram(tokenbyte ***pp_buff,long* p_numberobjects,unsigned long *p_maxseq,
	int* p_nmax,unsigned long **p_imaxseq,
	double maxseqapprox,int *p_bigitem,short **p_articul)
{
unsigned long id,iseq,ip,ip_next,iplot,**p_maxcol,classofinext,currswitchstate[MAXCHAN+1],
	tstart,imax;
float maxbeats,ibeatsvel,maxbeatsvel,**p_deftmaxbeatsvel,ibeatsarticul,maxbeatsarticul,
	**p_deftibeatsarticul,**p_deftmaxbeatsarticul,**p_deftibeatsvel,
	**p_deftibeatsmap,**p_deftmaxbeatsmap,ibeatsmap,maxbeatsmap,
	value,orgval,newval,endvel,endarticul,endval,
	starttranspose,transposeincrement,endtranspose,ibeatstranspose,maxbeatstranspose,
	**p_deftmaxbeatstranspose,**p_deftibeatstranspose,**p_deftstarttranspose,**p_defttransposeincrement;
double objectduration,**p_im,**p_origin,scale,inext,
	tempo,tempomax,max_tempo_in_skipped_object,prodtempo,speed,s,numberzeros,
	**p_currobject,**p_objectsfound,objectsfound,part_of_ip;
short rndvel,velcontrol,**p_deftrndvel,**p_deftvelcontrol,**p_deftstartvel,
	velincrement,**p_deftvelincrement,**p_deftarticulincrement,**p_deftstartarticul;
int i,j,nseq,oldnseq,nseqmem,newswitch,v,ch,gotnewline,foundobject,
	failed,paramnameindex,paramvalueindex,maxparam,newxpandval,newkeyval,
	**p_deftxpandval,**p_deftxpandkey,number_skipped,suggested_quantization,
	r,rest,oldm,oldp,**p_seq,**p_deftnseq,startvel,articulincrement,
	startarticul,istop,result,level,nseqplot,instrument,channel,a,b;
long k,kobj;
Handle h;
char  line[MAXLIN],toofast,skipzeros,foundendconcatenation,just_done,
	iscontinuous,isMIDIcontinuous,overstrike,tie;

tokenbyte m,p;
Table **h_table;
CurrentParameters currentparameters,**p_deftcurrentparameters;
ContParameters **p_contparameters,**p_deftcontparameters;
KeyNumberMap startmap,mapendvalue,mapincrement,**p_deftmapincrement,**p_deftstartmap;
p_list ****p_waitlist,****p_scriptlist,**tag,**ptag;


if(CheckEmergency() != OK) return(ABORT);
if((result=stop(1,"FillPhaseDiagram")) != OK) return(result);
if(trace_diagram) BPPrintMessage(odInfo,"Started filling phase diagram\n");

AllSolTimeSet = StackFlag = (*p_bigitem) = ToldSkipped = FALSE;

tstart = ZERO;
// if(IsMidiDriverOn()) tstart = GetDriverTime(); Fixed by BB 2021-03-27

just_done = FALSE;
number_skipped = 0;
max_tempo_in_skipped_object = 0.;
maxseqapprox = ((ceil(maxseqapprox) / Kpress) + 6.) * 1.01;	/* This is an approximation. */
if(ShowMessages || Maxevent > 500L) {
	if(Kpress > 1.) {
		ShowWindow(Window[wTimeAccuracy]);
		if(Kpress < ULONG_MAX)
			BPPrintMessage(odInfo,"Creating phase diagram with compression rate = %u\n",
				(unsigned long)Kpress);
		else
			BPPrintMessage(odInfo,"Creating phase diagram with compression rate = %.0f\n",
				Kpress);
		}
	else
		BPPrintMessage(odInfo,"Creating phase diagram. (No compression)\n");
//	BPPrintMessage(odInfo,Message);
	}

if(p_NumberConstant == NULL) maxparam = 0;
else maxparam = (MyGetHandleSize((Handle)p_NumberConstant) / sizeof(double));

if((h_table = (Table**) GiveSpace((Size)MAXSTRINGCONSTANTS*sizeof(Table))) == NULL)
	return(ABORT);
for(i=0; i < MAXSTRINGCONSTANTS; i++) {
	(*h_table)[i].point = NULL;
	(*h_table)[i].imax = (*h_table)[i].offset = 0;
	}

if((p_T = (Milliseconds**) GiveSpace((Size) (maxseqapprox+1.)*sizeof(Milliseconds))) == NULL)
	return(ABORT);
if((p_im = (double**) GiveSpace((Size)Maxconc*sizeof(double))) == NULL)
	return(ABORT);
if((p_maxcol = (unsigned long**) GiveSpace((Size)Maxconc*sizeof(unsigned long))) == NULL)
	return(ABORT);
if((p_waitlist = (p_list****) GiveSpace((Size)Maxconc*sizeof(p_list**))) == NULL)
	return(ABORT);
if((p_scriptlist = (p_list****) GiveSpace((Size)Maxconc*sizeof(p_list**))) == NULL)
	return(ABORT);	
if((p_origin = (double**) GiveSpace((Size)Maxlevel*sizeof(double))) == NULL)
	return(ABORT);
if((p_objectsfound = (double**) GiveSpace((Size)Maxlevel*sizeof(double))) == NULL)
	return(ABORT);
if((p_seq = (int**) GiveSpace((Size)Maxlevel*sizeof(int))) == NULL)
	return(ABORT);
if((p_deftnseq = (int**) GiveSpace((Size)Maxlevel*sizeof(int))) == NULL)
	return(ABORT);
if((p_currobject = (double**) GiveSpace((Size)Maxlevel*sizeof(double))) == NULL)
	return(ABORT);
if((p_deftcurrentparameters = (CurrentParameters**)
			GiveSpace((Size)Maxlevel*sizeof(CurrentParameters))) == NULL)
	return(ABORT);

if((p_deftvelincrement = (short**) GiveSpace((Size)Maxlevel*sizeof(short))) == NULL)
	return(ABORT);
if((p_deftstartvel = (short**) GiveSpace((Size)Maxlevel*sizeof(short))) == NULL)
	return(ABORT);
if((p_deftrndvel = (short**) GiveSpace((Size)Maxlevel*sizeof(short))) == NULL)
	return(ABORT);
if((p_deftvelcontrol = (short**) GiveSpace((Size)Maxlevel*sizeof(short))) == NULL)
	return(ABORT);
if((p_deftibeatsvel = (float**) GiveSpace((Size)Maxlevel*sizeof(float))) == NULL)
	return(ABORT);
if((p_deftmaxbeatsvel = (float**) GiveSpace((Size)Maxlevel*sizeof(float))) == NULL)
	return(ABORT);
	
if((p_deftarticulincrement = (short**) GiveSpace((Size)Maxlevel*sizeof(short))) == NULL)
	return(ABORT);
if((p_deftstartarticul = (short**) GiveSpace((Size)Maxlevel*sizeof(short))) == NULL)
	return(ABORT);
if((p_deftibeatsarticul = (float**) GiveSpace((Size)Maxlevel*sizeof(float))) == NULL)
	return(ABORT);
if((p_deftmaxbeatsarticul = (float**) GiveSpace((Size)Maxlevel*sizeof(float))) == NULL)
	return(ABORT);

if((p_deftmapincrement = (KeyNumberMap**) GiveSpace((Size)Maxlevel*sizeof(KeyNumberMap))) == NULL)
	return(ABORT);
if((p_deftstartmap = (KeyNumberMap**) GiveSpace((Size)Maxlevel*sizeof(KeyNumberMap))) == NULL)
	return(ABORT);
if((p_deftibeatsmap = (float**) GiveSpace((Size)Maxlevel*sizeof(float))) == NULL)
	return(ABORT);
if((p_deftmaxbeatsmap = (float**) GiveSpace((Size)Maxlevel*sizeof(float))) == NULL)
	return(ABORT);
	
if((p_defttransposeincrement = (float**) GiveSpace((Size)Maxlevel*sizeof(float))) == NULL)
	return(ABORT);
if((p_deftstarttranspose = (float**) GiveSpace((Size)Maxlevel*sizeof(float))) == NULL)
	return(ABORT);
if((p_deftibeatstranspose = (float**) GiveSpace((Size)Maxlevel*sizeof(float))) == NULL)
	return(ABORT);
if((p_deftmaxbeatstranspose = (float**) GiveSpace((Size)Maxlevel*sizeof(float))) == NULL)
	return(ABORT);
	
if((p_deftxpandval = (int**) GiveSpace((Size)Maxlevel*sizeof(int))) == NULL)
	return(ABORT);
if((p_deftxpandkey = (int**) GiveSpace((Size)Maxlevel*sizeof(int))) == NULL)
	return(ABORT);
	
if((p_deftcontparameters = (ContParameters**) GiveSpace((Size)Maxlevel*sizeof(ContParameters))) == NULL)
	return(ABORT);
if((p_contparameters = (ContParameters**) GiveSpace((Size)Maxlevel*sizeof(ContParameters))) == NULL)
	return(ABORT);

for(i=0; i < Maxlevel; i++) {
	(*p_contparameters)[i].values = NULL;
	(*p_contparameters)[i].number = 0;
	(*p_deftcontparameters)[i].values = NULL;
	(*p_deftcontparameters)[i].number = 0;
	
	/* Create minimum storage */
	FindParameterIndex(p_contparameters,i,-1);
	FindParameterIndex(p_deftcontparameters,i,-1);
	}

(*p_nmax) = 0;
// BPPrintMessage(odError,"Minconc = %ld, Maxconc = %ld\n",Minconc,Maxconc);
// for(nseq=0; nseq <= Minconc; nseq++) {
for(nseq=0; nseq < Maxconc; nseq++) { // Replaced Minconc with Maxconc by BB 2021-02-26 
	if(MakeNewLineInPhaseTable(nseq,p_nmax,p_im,maxseqapprox,p_maxcol) != OK) {
		BPPrintMessage(odError,"=> Unexpectedly, cannot fill phase diagram\n");
		return(ABORT);
		}
	}

for(nseq=0; nseq < Maxconc; nseq++) {
	(*p_waitlist)[nseq] = (*p_scriptlist)[nseq] = NULL;
	}
(*p_deftnseq)[0] = nseq = 0;

if(trace_diagram)
	BPPrintMessage(odInfo,"Maxevent = %d\n",Maxevent);

for(k=0; k < Maxevent; k++) {
	(*p_Instance)[k].object = 0;
	(*p_ObjectSpecs)[k] = NULL;
	(*p_Instance)[k].channel = 0;
	(*p_Instance)[k].scale = DefaultScale;
	(*p_Instance)[k].blockkey = BlockScaleOnKey;
	(*p_Instance)[k].transposition = 0;
	(*p_Instance)[k].xpandkey = -1;
	(*p_Instance)[k].xpandval = 0;
	(*p_Instance)[k].randomtime = 0;
	(*p_Instance)[k].seed = NOSEED;
	(*p_Instance)[k].lastistranspose = TRUE;
	(*p_Instance)[k].ncycles = 0;
	(*p_Instance)[k].alpha = (*p_Instance)[k].dilationratio = 0.;
	(*p_Instance)[k].velocity = DeftVelocity;
	(*p_Instance)[k].instrument = (*p_Instance)[k].nseq = 0;
	(*p_articul)[k] = 0;
	}

failed = TRUE;
result = ABORT;
if(DisplayTimeSet && !ScriptExecOn && !Improvize) {
	if((AllSolTimeSet=Answer("Display all canonic solutions",'Y')) == ABORT) {
		goto ENDDIAGRAM;
		}
	StackFlag = AllSolTimeSet || DisplayTimeSet;
	}
if(!StepTimeSet) {
/*	LimTimeSet = TRUE; */
/*	if(!WriteMIDIfile && !Improvize && !OutCsound) {
		sprintf(Message,
		"Current time limit is: %4.2f sec. Stop at limit",
			(double) TimeMax/1000.);
		if((LimTimeSet = Answer(Message,'N')) == ABORT)
			goto ENDDIAGRAM;
		} */
	}


//  -------------  Fill phase diagram ------------------

	
	/* kobj = 0		empty object or prolongational gap "_"		*/
	/* kobj = 1		silence "-"							*/
	/* kobj > 1		non-empty time-object					*/


tempomax = Prod / Kpress;
scale = 1.;
speed = 1.;
toofast = foundobject = FALSE;
if(scale != 0.) {
	tempo = speed / scale;
	prodtempo = (Prod / tempo);
	}
else {
	tempo = prodtempo = 0.;
	toofast = TRUE;
	}
kobj = (*p_Instance)[1].object = 1;
oldp = -1; overstrike = FALSE;


(*p_currobject)[0] = (*p_objectsfound)[0] = 0.;
(*p_seq)[0] = level = 0;


for(i=0; i <= MAXCHAN; i++) {
	PitchbendRange[i] = DeftPitchbendRange;
	VolumeControl[i] = VolumeController;
	PressRate[i] = PitchbendRate[i] = ModulationRate[i] = VolumeRate[i] = PanoramicRate[i]
		= SamplingRate;
	PanoramicControl[i] = PanoramicController;
	}
	
startarticul = (*p_deftstartarticul)[0] = currentparameters.currarticul = 0;
startvel = (*p_deftstartvel)[0] = currentparameters.currvel = DeftVelocity;

velcontrol = (*p_deftvelcontrol)[0] = currentparameters.velcontrol = -1;

rndvel = (*p_deftrndvel)[0] = currentparameters.rndvel = 0;

startmap.p1 = startmap.q1 = 0;
startmap.p2 = startmap.q2 = 127;

currentparameters.currchan = 1;
currentparameters.scale = DefaultScale;
currentparameters.blockkey = BlockScaleOnKey;
currentparameters.currinstr = 0;

currentparameters.currtranspose = starttranspose = (*p_deftstarttranspose)[0] = 0.;
currentparameters.lastistranspose = TRUE;
currentparameters.transposemode = FIXED;

currentparameters.articulmode = currentparameters.velmode = FIXED;
currentparameters.mapmode = OFF;

currentparameters.xpandkey = (*p_deftxpandkey)[0]  = -1;
currentparameters.xpandval = (*p_deftxpandval)[0]  = 0;
orgval = 1.;

currentparameters.randomtime = 0;
currentparameters.seed = NOSEED;

(*p_deftcurrentparameters)[0] = currentparameters;

velincrement = (*p_deftvelincrement)[0]
	 = articulincrement = (*p_deftarticulincrement)[0] = 0;

transposeincrement = (*p_defttransposeincrement)[0] = 0.;

maxbeatsvel = (*p_deftmaxbeatsvel)[0]
	= maxbeatsarticul = (*p_deftmaxbeatsarticul)[0]
	= maxbeatsmap = (*p_deftmaxbeatsmap)[0]
	= maxbeatstranspose = (*p_deftmaxbeatstranspose)[0]
	= 1L;
	 
mapincrement.p1 = (*p_deftmapincrement)[0].p1
= mapincrement.q1 = (*p_deftmapincrement)[0].q1
= mapincrement.p2 = (*p_deftmapincrement)[0].p2
= mapincrement.q2 = (*p_deftmapincrement)[0].q2 = 0;

(*p_origin)[level] = (*p_im)[level];

foundendconcatenation = skipzeros = FALSE;

ibeatsvel = ibeatsarticul = ibeatsmap = ibeatstranspose = 0.;

for(i=0; i <= MAXCHAN; i++) currswitchstate[i] = 0L;
newswitch = TRUE;	/* This will reset all switches in the beginning of the item */

nseqplot = Minconc + 1;
iplot = ZERO;

instrument = channel = 0;
part_of_ip = Kpress;

for(id=istop=ZERO; ;id+=2,istop++) {
/*	if(istop == 20) istop = 0;
	if((istop == 0 && Button()) && (r=InterruptTimeSet(FALSE,&tstart)) != OK) {
		result = r;
		goto ENDDIAGRAM;
		} */
	m = (tokenbyte) (**pp_buff)[id];
	p = (tokenbyte) (**pp_buff)[id+1];
	if(m == TEND && p == TEND) break;
	if((result=stop(0,"FillPhaseDiagram")) != OK) return(result);
	if(trace_diagram)
		BPPrintMessage(odInfo,"FillPhaseDiagram() m = %d p = %d level = %ld nseq = %ld id = %ld\n",m,p,(long)level,(long)nseq,(long)id);
/*	if(m == T10) {	// Channel assignment _chan() // Fixed by BB 2021-02-15
		channel = (int) FindValue(m,p,0);
		if(trace_diagram)
			BPPrintMessage(odInfo,"\nFillPhaseDiagram() channel = %d\n",channel);
		goto NEXTTOKEN;
		} */
/*	if(m == T32) { // Instrument assignment _ins() // Fixed by BB 2021-02-15
		instrument = (int) FindValue(m,p,0);
		if(trace_diagram) BPPrintMessage(odInfo,"\nFillPhaseDiagram() instrument = %d\n",instrument);
		goto NEXTTOKEN;
		} */
	if(m != T3 || p != 0) part_of_ip = Kpress; // Added by BB 2021-03-25
	if(m == T33 || m == T34) {	/* _step() or _cont() */
		paramnameindex = p;
		i = FindParameterIndex(p_contparameters,level,paramnameindex);
		if(trace_diagram) BPPrintMessage(odInfo, "_step() or _cont() m = %d paramnameindex = %d i = %d\n",m,paramnameindex,i);
		if(i >= 0) {
			UpdateParameter(i,p_contparameters,level,ZERO);
			if(m == T33) (*((*p_contparameters)[level].values))[i].mode = STEPWISE;
			else (*((*p_contparameters)[level].values))[i].mode = CONTINUOUS;
			if(SetVariation(-1,p_deftcurrentparameters,&currentparameters,p_contparameters,level,i,
					id,*pp_buff,speed,scale,&endval,&mapendvalue,&maxbeats,h_table) != OK)
				goto ENDDIAGRAM;
			}
		goto NEXTTOKEN;
		}
	if(m == T36) {	/* _fixed() */
		paramnameindex = p;
		i = FindParameterIndex(p_contparameters,level,paramnameindex);
		if(i >= 0) {
			UpdateParameter(i,p_contparameters,level,ZERO);
			(*((*p_contparameters)[level].values))[i].ibeats = 0;
			(*((*p_contparameters)[level].values))[i].maxbeats = 1;
			(*((*p_contparameters)[level].values))[i].increment = 0;
			(*((*p_contparameters)[level].values))[i].mode = FIXED;
			}
		goto NEXTTOKEN;
		}
	if(m == T12) {
		switch(p) {
			case 0: /* _velcont */
			case 1: /* _velstep */
				startvel = startvel + (velincrement * ibeatsvel / maxbeatsvel);
				currentparameters.currvel = startvel;
				if(p == 0) currentparameters.velmode = STEPWISE;
				else currentparameters.velmode = CONTINUOUS;	/* not implemented */
			if(SetVariation(T11,p_deftcurrentparameters,&currentparameters,
			p_contparameters,level,-1,id,*pp_buff,speed,scale,&endvel,&mapendvalue,
			&maxbeatsvel,h_table) != OK)
					goto ENDDIAGRAM;
				ibeatsvel = 0.;
				if(maxbeatsvel == 0.) {
					maxbeatsvel = 1.; velincrement = 0;
					}
				else velincrement = endvel - startvel;
				break;
			case 2:	/* _modstep */
			case 3: /* _modcont */
				i = IMODULATION;
				UpdateParameter(i,p_contparameters,level,ZERO);
				if(p == 2) (*((*p_contparameters)[level].values))[i].mode = STEPWISE;
				else (*((*p_contparameters)[level].values))[i].mode = CONTINUOUS;
				if(SetVariation(-1,p_deftcurrentparameters,&currentparameters,p_contparameters,level,i,id,
						*pp_buff,speed,scale,&endval,&mapendvalue,&maxbeats,h_table) != OK)
					goto ENDDIAGRAM;
				break;
			case 4:	/* _pitchstep */
			case 5:	/* _pitchcont */
				i = IPITCHBEND;
				UpdateParameter(i,p_contparameters,level,ZERO);
				if(p == 4) (*((*p_contparameters)[level].values))[i].mode = STEPWISE;
				else (*((*p_contparameters)[level].values))[i].mode = CONTINUOUS;
				if(SetVariation(-1,p_deftcurrentparameters,&currentparameters,p_contparameters,level,i,id,
						*pp_buff,speed,scale,&endval,&mapendvalue,&maxbeats,h_table) != OK)
					goto ENDDIAGRAM;
				break;
			case 6:	/* _presstep */
			case 7:	/* _presscont */
				i = IPRESSURE;
				UpdateParameter(i,p_contparameters,level,ZERO);
				if(p == 6) (*((*p_contparameters)[level].values))[i].mode = STEPWISE;
				else (*((*p_contparameters)[level].values))[i].mode = CONTINUOUS;
				if(SetVariation(-1,p_deftcurrentparameters,&currentparameters,p_contparameters,level,i,id,
					*pp_buff,speed,scale,&endval,&mapendvalue,&maxbeats,h_table) != OK)
						goto ENDDIAGRAM;
				break;
			case 8: /* _volumestep */
			case 9: /* _volumecont */
				i = IVOLUME;
				UpdateParameter(i,p_contparameters,level,ZERO);
				if(p == 8) (*((*p_contparameters)[level].values))[i].mode = STEPWISE;
				else (*((*p_contparameters)[level].values))[i].mode = CONTINUOUS;
				if(SetVariation(-1,p_deftcurrentparameters,&currentparameters,p_contparameters,level,i,id,
					*pp_buff,speed,scale,&endval,&mapendvalue,&maxbeats,h_table) != OK)
						goto ENDDIAGRAM;
				break;
			case 10: /* _articulstep */
			case 11: /* _articulcont */
				startarticul = startarticul + (articulincrement * ibeatsarticul / maxbeatsarticul);
				currentparameters.currarticul = startarticul;
				if(p == 10) currentparameters.articulmode = STEPWISE;
				else currentparameters.articulmode = CONTINUOUS;	/* not implemented */
				if(SetVariation(T20,p_deftcurrentparameters,&currentparameters,p_contparameters,level,-1,id,
						*pp_buff,speed,scale,&endarticul,&mapendvalue,&maxbeatsarticul,h_table) != OK)
					goto ENDDIAGRAM;
				
				ibeatsarticul = 0;
				if(maxbeatsarticul == 0) {
					maxbeatsarticul = 1; articulincrement = 0;
					}
				else articulincrement = endarticul - startarticul;
				
				break;
			case 12: /* _velfixed */
				currentparameters.velmode = FIXED;
				startvel = startvel + (velincrement * ibeatsvel / maxbeatsvel);
				currentparameters.currvel = startvel;
				ibeatsvel = 0.; maxbeatsvel = 1.; velincrement = 0.;
				break;
			case 13: /* _modfixed */
				i = IMODULATION;
				UpdateParameter(i,p_contparameters,level,ZERO);
				(*((*p_contparameters)[level].values))[i].ibeats = 0;
				(*((*p_contparameters)[level].values))[i].maxbeats = 1;
				(*((*p_contparameters)[level].values))[i].increment = 0;
				(*((*p_contparameters)[level].values))[i].mode = FIXED;
				break;
			case 14: /* _pitchfixed */
				i = IPITCHBEND;
				UpdateParameter(i,p_contparameters,level,ZERO);
				(*((*p_contparameters)[level].values))[i].ibeats = 0;
				(*((*p_contparameters)[level].values))[i].maxbeats = 1;
				(*((*p_contparameters)[level].values))[i].increment = 0;
				(*((*p_contparameters)[level].values))[i].mode = FIXED;
				break;
			case 15: /* _pressfixed */
				i = IPRESSURE;
				UpdateParameter(i,p_contparameters,level,ZERO);
				(*((*p_contparameters)[level].values))[i].ibeats = 0;
				(*((*p_contparameters)[level].values))[i].maxbeats = 1;
				(*((*p_contparameters)[level].values))[i].increment = 0;
				(*((*p_contparameters)[level].values))[i].mode = FIXED;
				break;
			case 16: /* _volumefixed */
				i = IVOLUME;
				UpdateParameter(i,p_contparameters,level,ZERO);
				(*((*p_contparameters)[level].values))[i].ibeats = 0;
				(*((*p_contparameters)[level].values))[i].maxbeats = 1;
				(*((*p_contparameters)[level].values))[i].increment = 0;
				(*((*p_contparameters)[level].values))[i].mode = FIXED;
				break;
			case 17: /* _articulfixed */
				currentparameters.articulmode = FIXED;
				startarticul = startarticul + (articulincrement * ibeatsarticul / maxbeatsarticul);
				currentparameters.currarticul = startarticul;
				ibeatsarticul = 0; maxbeatsarticul = 1; articulincrement = 0;
				break;
			case 18: /* _panfixed */
				i = IPANORAMIC;
				UpdateParameter(i,p_contparameters,level,ZERO);
				(*((*p_contparameters)[level].values))[i].ibeats = 0;
				(*((*p_contparameters)[level].values))[i].maxbeats = 1;
				(*((*p_contparameters)[level].values))[i].increment = 0;
				(*((*p_contparameters)[level].values))[i].mode = FIXED;
				break;
			case 19: /* _panstep */
			case 20: /* _pancont */
				i = IPANORAMIC;
				UpdateParameter(i,p_contparameters,level,ZERO);
				if(p == 19) (*((*p_contparameters)[level].values))[i].mode = STEPWISE;
				else (*((*p_contparameters)[level].values))[i].mode = CONTINUOUS;
				if(SetVariation(-1,p_deftcurrentparameters,&currentparameters,p_contparameters,level,i,id,
						*pp_buff,speed,scale,&endval,&mapendvalue,&maxbeats,h_table) != OK)
					goto ENDDIAGRAM;
				break;
			case 25: /* _mapfixed */
				currentparameters.mapmode = FIXED;
				startmap.p1 = startmap.p1 + (mapincrement.p1 * ibeatsmap / maxbeatsmap);
				startmap.q1 = startmap.q1 + (mapincrement.q1 * ibeatsmap / maxbeatsmap);
				startmap.p2 = startmap.p2 + (mapincrement.p2 * ibeatsmap / maxbeatsmap);
				startmap.q2 = startmap.q2 + (mapincrement.q2 * ibeatsmap / maxbeatsmap);
				currentparameters.map0 = startmap;
				mapincrement.p1 = mapincrement.q1 = mapincrement.p2 = mapincrement.q2 = 0;
				ibeatsmap = 0; maxbeatsmap = 1;
				if(startmap.p1 == startmap.q1 && startmap.p2 == startmap.q2) {
					startmap.p1 = startmap.q1 = 0;
					startmap.p2 = startmap.q2 = 127;
					currentparameters.mapmode = OFF;
					}
				break;
			case 23: /* _randomize */
				currentparameters.seed = RANDOMIZE;
				break;
			case 26: /* _mapcont */
			case 27: /* _mapstep */
				startmap.p1 = startmap.p1 + (mapincrement.p1 * ibeatsmap / maxbeatsmap);
				startmap.q1 = startmap.q1 + (mapincrement.q1 * ibeatsmap / maxbeatsmap);
				startmap.p2 = startmap.p2 + (mapincrement.p2 * ibeatsmap / maxbeatsmap);
				startmap.q2 = startmap.q2 + (mapincrement.q2 * ibeatsmap / maxbeatsmap);
				currentparameters.map0 = startmap;
				ibeatsmap = 0.; maxbeatsmap = 1.;
				mapincrement.p1 = mapincrement.q1 = mapincrement.p2 = mapincrement.q2 = 0;
				if(p == 27) currentparameters.mapmode = STEPWISE;
				else currentparameters.mapmode = CONTINUOUS;
				if(SetVariation(T37,p_deftcurrentparameters,&currentparameters,p_contparameters,level,-1,id,
						*pp_buff,speed,scale,&endval,&mapendvalue,&maxbeatsmap,h_table) != OK)
					goto ENDDIAGRAM;
				
				ibeatsmap = 0.;
				if(maxbeatsmap == 0.) {
					maxbeatsmap = 1.;
					mapincrement.p1 = mapincrement.q1 = mapincrement.p2 = mapincrement.q2 = 0;
					}
				else {
					mapincrement.p1 = mapendvalue.p1 - startmap.p1;
					mapincrement.q1 = mapendvalue.q1 - startmap.q1;
					mapincrement.p2 = mapendvalue.p2 - startmap.p2;
					mapincrement.q2 = mapendvalue.q2 - startmap.q2;
					}
				break;
			case 28: /* _transposefixed */
				currentparameters.transposemode = FIXED;
				starttranspose = starttranspose + (transposeincrement * ibeatstranspose / maxbeatstranspose);
				currentparameters.currtranspose = starttranspose;
				ibeatstranspose = 0.; maxbeatstranspose = 1.; transposeincrement = 0.;
				break;
			case 29: /* _transposecont */
			case 30: /* _transposestep */
				starttranspose = starttranspose + (transposeincrement * ibeatstranspose / maxbeatstranspose);
				currentparameters.currtranspose = starttranspose;
				if(p == 30) currentparameters.transposemode = STEPWISE;
				else currentparameters.transposemode = CONTINUOUS;	/* not implemented */
				if(SetVariation(T26,p_deftcurrentparameters,&currentparameters,p_contparameters,level,-1,id,
						*pp_buff,speed,scale,&endtranspose,&mapendvalue,
						&maxbeatstranspose,h_table) != OK)
					goto ENDDIAGRAM;
				ibeatstranspose = 0.;
				if(maxbeatstranspose == 0.) {
					maxbeatstranspose = 1.; transposeincrement = 0;
					}
				else transposeincrement = endtranspose - starttranspose;
				break;
			}
		goto NEXTTOKEN;
		}
	if((m == T3 && p < Jbol) || m == T25	/* Sound-object or simple note or silence */
			|| (m == T9 && p < Jpatt)) {	/* Time pattern */
		if(trace_diagram) {
			BPPrintMessage(odInfo,"m = %d p = %d id = %ld",m,p,(long)id);
			if(m == T3 && p > 1) BPPrintMessage(odInfo," (*p_MIDIsize)[p] = %ld (*p_CsoundSize)[p] = %ld\n",(*p_MIDIsize)[p],(*p_CsoundSize)[p]);
			else BPPrintMessage(odInfo,"\n");
			}
		if(m == T3 && p > 1 && (*p_MIDIsize)[p] == ZERO && (*p_CsoundSize)[p] == ZERO) {
			m = T3;
			p = 1;
			if(trace_diagram) BPPrintMessage(odError,"=> m = %d p = %d, MIDIsize = CsoundSize = 0\n",m,p);
			}
		toofast = (tempo > tempomax || tempo == 0.);
		just_done = FALSE;
		if(!new_thing || m != T3 || p != 0 || !toofast || skipzeros || (m == T3 && p == 0 && toofast && part_of_ip >= Kpress)) { // Added by BB 2021-03-25
			((*p_im)[nseq]) += Kpress;
		//	BPPrintMessage(odInfo,"+ m = %d p = %d kobj = %d (*p_im)[%d] = %.1f (*p_im)[nseq]/Kpress = %.1f toofast = %d\n",m,p,kobj,nseq,(double)((*p_im)[nseq]),(double)((*p_im)[nseq]/Kpress),(int)toofast);
			ip = Class((*p_im)[nseq]);
			just_done = TRUE;
			}
		if(Beta && (ip > maxseqapprox)) {
			BPPrintMessage(odError,"\n=> Error nseq = %ld Class((*p_im)[nseq]) = %ld maxseqapprox = %ld\n",
				(long)nseq,ip,(long)maxseqapprox);
			goto ENDDIAGRAM;
			}
			
		overstrike = FALSE;
		nseqmem = nseq;
		tie = FALSE;
		if(m == T25) {
			tie = (*(p_Tie_note[channel]))[p]; // Added by BB 2021-02-07
			if(trace_diagram)
				BPPrintMessage(odInfo,"Tie_note m = %d p = %d tie =  %d\n",m,p,tie);
			}
		else if(m == T3) {
			tie = (*(p_Tie_event[instrument]))[p]; // Added by BB 2021-02-07
			if(trace_diagram) BPPrintMessage(odInfo,"Tie_event m = %d p = %d tie =  %d\n",m,p,tie);
			}
		else tie = FALSE;
		if(foundendconcatenation && tie) {
			if(trace_diagram)
				BPPrintMessage(odInfo,"With foundendconcatenation kobj = %d, m = %d p = %d\n",(int)kobj,m,p);
			if(m == T25) (*(p_Tie_note[channel]))[p] = FALSE;
			else if(m == T3) (*(p_Tie_event[instrument]))[p] = FALSE;
			
			if(Plot(INTIME,&nseqplot,&iplot,&overstrike,FALSE,p_nmax,p_maxcol,p_im,p_Seq,
				&nseq,maxseqapprox,ip,1) != OK) goto ENDDIAGRAM;
			oldm = m; oldp = p;
			(*p_maxcol)[nseq] = ip;
			}
		else {
			objectduration = 0.;
			if(tie && (m == T25 || m == T9 || p > 0)) {
			//	BPPrintMessage(odInfo,"\nCase 1 tie = %d\n",tie);
				// BPPrintMessage(odInfo,"GetSymbolicDuration() with tie, nseq = %ld level = %ld m = %ld p = %ld speed = %.2f scale = %.2f id = %ld kobj = %d\n",(long)nseq,(long)level,(long)m,(long)p,speed,scale,id,(int)kobj);
				objectduration = GetSymbolicDuration(NO,*pp_buff,m,p,id,speed,scale,channel,instrument,foundendconcatenation,level);
				// BPPrintMessage(odInfo,"End GetSymbolicDuration() objectduration = %.2f\n",objectduration);
				}
			iscontinuous = isMIDIcontinuous = FALSE;
			if(p > 0) {
				oldm = m; oldp = p;
				skipzeros = FALSE;
				if((*p_contparameters)[level].number > 0) {
					for(i=0; i < (*p_contparameters)[level].number; i++) {
						if((*((*p_contparameters)[level].values))[i].active) {
							iscontinuous = TRUE;
							if(i <= IPANORAMIC) isMIDIcontinuous = TRUE;
							break;
							}
						}
					if(iscontinuous) {
						for(i=0; i < (*p_contparameters)[level].number; i++)
							UpdateParameter(i,p_contparameters,level,objectduration);
						}
					}
				}
		//	if(m == T25 || m == T9 || p > 1 || objectduration > 1. || isMIDIcontinuous) { // Fixed by BB 2021-04-01
			if((m == T25 || m == T9 || p > 1 || objectduration > 1. || isMIDIcontinuous) && !foundendconcatenation) {
				/* Non-empty sound-object or time pattern or simple note */
				// p > 1 implies that silences won't be played as objects
				skipzeros = FALSE;
				kobj++; (*p_numberobjects) = kobj;
				ShowProgress(kobj);
				ip = Class((*p_im)[nseq]);
				// BPPrintMessage(odInfo,"@ nseq = %ld, kobj = %ld m = %d p = %d ip = %ld\n",(long)nseq,(long)kobj,m,p,(long)ip);
				if(AttachObjectLists(kobj,nseq,p_waitlist,p_scriptlist,&newswitch,currswitchstate)
					== ABORT) goto ENDDIAGRAM;
				if(Plot(INTIME,&nseqplot,&iplot,&overstrike,FALSE,p_nmax,p_maxcol,p_im,
					p_Seq,&nseq,maxseqapprox,ip,kobj) != OK) goto ENDDIAGRAM;
				//	BPPrintMessage(odInfo,"Plot(4) nseq = %ld, kobj = %ld tie = %d foundendconcatenation = %d m = %d p = %d ip = %ld\n",(long)nseq,(long)kobj,(int)tie,(int)foundendconcatenation,m,p,(long)ip);
				if(overstrike) {
					kobj--;
					(*p_numberobjects) = kobj;
					if(m != T3 || p != 1) number_skipped++;
					}
				(*p_maxcol)[nseq] = ip;
				
				switch(m) {
					case T3:
						if(p > 1) foundobject = TRUE; /* Sound object */ 
						break;
					case T9: /* Time pattern */
						p += Jbol;
						break;
					case T25: /* Simple note */
						p += 16384;
						break;
					}
				if(!overstrike) (*p_Instance)[kobj].object = p;
					
				currentparameters.currvel = startvel + (velincrement * ibeatsvel
					/ maxbeatsvel);
				currentparameters.currarticul = startarticul
					+ (articulincrement * ibeatsarticul / maxbeatsarticul);
				currentparameters.currtranspose = starttranspose
					+ (transposeincrement * ibeatstranspose / maxbeatstranspose);
					
				currentparameters.map0.p1 = startmap.p1
					+ (mapincrement.p1 * ibeatsmap / maxbeatsmap);
				currentparameters.map0.q1 = startmap.q1
					+ (mapincrement.q1 * ibeatsmap / maxbeatsmap);
				currentparameters.map0.p2 = startmap.p2
					+ (mapincrement.p2 * ibeatsmap / maxbeatsmap);
				currentparameters.map0.q2 = startmap.q2
					+ (mapincrement.q2 * ibeatsmap / maxbeatsmap);
				
				if(currentparameters.mapmode == CONTINUOUS) {
					ibeatsmap += objectduration;
					currentparameters.map1.p1 = startmap.p1
						+ (mapincrement.p1 * ibeatsmap / maxbeatsmap);
					currentparameters.map1.q1 = startmap.q1
						+ (mapincrement.q1 * ibeatsmap / maxbeatsmap);
					currentparameters.map1.p2 = startmap.p2
						+ (mapincrement.p2 * ibeatsmap / maxbeatsmap);
					currentparameters.map1.q2 = startmap.q2
						+ (mapincrement.q2 * ibeatsmap / maxbeatsmap);
					ibeatsmap -= objectduration;
					}
				else currentparameters.map1 = currentparameters.map0;
				
				if(!overstrike) {
					if(SetObjectParams(TRUE,level,nseq,p_articul,kobj,p,&currentparameters,p_contparameters,
							h_table) != OK)
						goto ENDDIAGRAM;
					}
				for(i=0; i < (*p_contparameters)[level].number; i++) {
					if((*((*p_contparameters)[level].values))[i].active) {
						if((*((*p_contparameters)[level].values))[i].increment == 0)
							(*((*p_contparameters)[level].values))[i].active = FALSE;
						}
					}
				// BPPrintMessage(odInfo,"Sequence toofast = %d overstrike = %d\n",toofast,overstrike);
				if(toofast && !overstrike) {		/* Sequence is too fast, we'll store proper duration nevertheless */
					/* We'll get the symbolic duration excluding '&' concatenation */
					objectduration
						= GetSymbolicDuration(YES,*pp_buff,m,p,id,speed,
							scale,channel,instrument,foundendconcatenation,level);
					numberzeros = objectduration - prodtempo; // Added by BB 2021-03-25
			/*		if(numberzeros >= 1.)
						BPPrintMessage(odInfo,"PutZeros(1) id = %ld m = %d p = %d toofast = %d objectduration = %.1f (*p_im)[nseq]/Kpress = %.1f kobj = %d maxseqapprox = %.0f numberzeros = %.0f numberzeros/Kpress = %.0f\n",id,m,p,(int)toofast,objectduration,(*p_im)[nseq]/Kpress,kobj,maxseqapprox,numberzeros,(numberzeros/Kpress)); */
					// BPPrintMessage(odInfo,"@@ toofast GetSymbolicDuration() nseq = %ld level = %ld m = %ld p = %ld numberzeros = %.2f speed = %.2f scale = %.2f id = %ld\n",(long)nseq,(long)level,(long)m,(long)p,numberzeros,speed,scale,id);
					if(PutZeros(toofast,p_im,p_maxcol,nseq,maxseqapprox,numberzeros,p_nmax,0) != OK)
						goto ENDDIAGRAM;
					skipzeros = TRUE;
					ibeatsvel += objectduration;
					ibeatsarticul += objectduration;
					ibeatsmap += objectduration;
					ibeatstranspose += objectduration;
					(*p_currobject)[level] += objectduration;
					for(i=0; i < (*p_contparameters)[level].number; i++)
						IncrementParameter(i,p_contparameters,level,objectduration);
					(*p_im)[nseq] -= (Kpress - 1.);
					(*p_maxcol)[nseq] = Class((*p_im)[nseq]);
				//	if(speed/scale > max_tempo_in_skipped_object) max_tempo_in_skipped_object = speed/scale;
					if(tempo != speed/scale) BPPrintMessage(odError,"=> ERROR tempo = %.3f speed/scale = %.3f\n",(double)tempo,(double)speed/scale);
					if(tempo > max_tempo_in_skipped_object) max_tempo_in_skipped_object = tempo; // Fixed by BB 2021-03-26
					if(trace_toofast) BPPrintMessage(odInfo,"---> toofast m = %d p = %d speed = %.0f scale = %.0f objectduration = %.0f Prod = %.0f = tempomax = %.0f prodtempo = %.0f, im[%d] = %.0f, maxcol[nseq] = %ld\n",m,p,speed,scale,objectduration,Prod,tempomax,prodtempo,nseq,(*p_im)[nseq],(long)(*p_maxcol)[nseq]);
					goto NEXTTOKEN;
					}
				}
			else {
				ip = Class((*p_im)[nseq]);
				if((p == 0 && !skipzeros) || p == 1) {	/* Empty object or silence */
				//	BPPrintMessage(odInfo,"--> toofast = %d m = %d p = %d, Kpress = %.0f speed = %.0f scale = %.0f Prod = %.0f tempomax = %.0f tempo = %.0f prodtempo = %.0f, (*p_im)[%d] = %.0f, maxcol[nseq] = %ld part_of_ip = %.0f\n",(int)toofast,m,p,(double)Kpress,(double)speed,(double)scale,(double)Prod,(double)tempomax,(double)tempo,(double)prodtempo,(int)nseq,(double)(*p_im)[nseq],(long)(*p_maxcol)[nseq],(double)part_of_ip);
					if(!new_thing || m != T3 || p != 0 || !toofast || just_done) { // Added by BB 2021-03-27
						if(Plot(INTIME,&nseqplot,&iplot,&overstrike,FALSE,p_nmax,p_maxcol,p_im,p_Seq,
							&nseq,maxseqapprox,ip,p) != OK) goto ENDDIAGRAM;
						// BPPrintMessage(odInfo,"Plot(1) id = %ld m = %d p = %d ip = %.0f iplot = %.0f skipzeros = %d toofast = %d overstrike = %d (*p_im)[%d] = %.1f (*p_im)[nseq]/Kpress = %.1f kobj = %d maxseqapprox = %.0f Prod = %.0f tempo = %.0f nseq = %d nseqplot = %d (*p_maxcol)[nseq] = %.0f part_of_ip = %.0f\n",id,m,p,(double)ip,(double)iplot,(int)skipzeros,(int)toofast,overstrike,nseq,(*p_im)[nseq],(*p_im)[nseq]/Kpress,kobj,maxseqapprox,(double)Prod,(double)tempo,nseq,nseqplot,(double)ip,(double)part_of_ip);
						part_of_ip = 0.;
						(*p_maxcol)[nseq] = ip;
						}
					if(m == T3 && p == 0 && toofast) { // Fixed by BB 2021-03-28
						// Check this process in -da.checkPoly
						part_of_ip += Kpress * tempomax / tempo;
						if(!just_done) (*p_im)[nseq] += (prodtempo - 1); // Fixed by BB 2021-03-28
					//	(*p_im)[nseq] += prodtempo;
						ip = Class((*p_im)[nseq]); // Added by BB 2021-03-27
						}
					else part_of_ip = Kpress;
					}
				else part_of_ip = Kpress;
				if(p == 1 && ((*p_waitlist)[nseq] != NULL || (*p_scriptlist)[nseq] != NULL || newswitch || isMIDIcontinuous)) {
					/* Silence becomes a specific object because it has attached info */
					kobj++;
					(*p_numberobjects) = kobj;
					ShowProgress(kobj);
					if(AttachObjectLists(kobj,nseq,p_waitlist,p_scriptlist,&newswitch,currswitchstate)
						== ABORT) goto ENDDIAGRAM;
						
					if(Plot(INTIME,&nseqplot,&iplot,&overstrike,FALSE,p_nmax,p_maxcol,p_im,p_Seq,
						&nseq,maxseqapprox,ip,kobj)!= OK) goto ENDDIAGRAM;
					(*p_maxcol)[nseq] = ip;
					if(overstrike) {
						kobj--;
						(*p_numberobjects) = kobj;
						}
					else {
						(*p_Instance)[kobj].object = p;
						if(SetObjectParams(TRUE,level,nseq,p_articul,kobj,p,&currentparameters,
							p_contparameters,h_table) != OK) goto ENDDIAGRAM;
						}
					for(i=0; i < (*p_contparameters)[level].number; i++) {
						if((*((*p_contparameters)[level].values))[i].active) {
							if((*((*p_contparameters)[level].values))[i].increment == 0)
								(*((*p_contparameters)[level].values))[i].active = FALSE;
							}
						}
					}
				}
			}
		foundendconcatenation = FALSE;
		if(!new_thing || m != T3 || p != 0 || !toofast || skipzeros || just_done) { // Added by BB 2021-03-26
			numberzeros = prodtempo - 1.;
			if(skipzeros) numberzeros = -1.;
			if(PutZeros(toofast,p_im,p_maxcol,nseq,maxseqapprox,numberzeros,p_nmax,kobj) != OK) goto ENDDIAGRAM;
		/*	if(numberzeros >= 1. && PutZeros(toofast,p_im,p_maxcol,nseq,maxseqapprox,numberzeros,p_nmax,kobj) != OK)
				goto ENDDIAGRAM; */
		//	if(numberzeros >= 1.) BPPrintMessage(odInfo,"PutZeros(2) id = %ld m = %d p = %d prodtempo = %.0f toofast = %d (*p_im)[%d] = %.1f (*p_maxcol)[nseq] = %.1f (*p_im)[nseq]/Kpress = %.1f kobj = %d maxseqapprox = %.0f Prod = %.0f tempo = %.0f numberzeros = %.0f numberzeros/Kpress = %.0f\n",(long)id,m,p,(double)prodtempo,(int)toofast,(int)nseq,(double)(*p_im)[nseq],(double)(*p_maxcol)[nseq],(double)(*p_im)[nseq]/Kpress,(int)kobj,(double)maxseqapprox,(double)Prod,(double)tempo,(double)numberzeros,(double)(numberzeros/Kpress));
			}
		just_done = FALSE;
		
		if(m != T3 && m != T9 && m != T25) {
			BPPrintMessage(odError,"=> Err. FillPhaseDiagram(). m != T3 && m != T9 && m != T25\n");
			goto NEXTTOKEN;
			}
		numberzeros += 1.; // This is the total duration: zeros + note
		if(numberzeros < 1.) {
			/* Too fast sequence */
			if(nseq < Maxconc) {
				(*p_im)[nseq] -= Kpress;
				(*p_maxcol)[nseq] = Class((*p_im)[nseq]);
				}
			/* Plot() will attempt to overstrike the current object, but seeing it, it will jump to another value of nseq */
			goto NEXTTOKEN;
			}
		ibeatsvel += numberzeros;
		ibeatsarticul += numberzeros;
		ibeatsmap += numberzeros;
		ibeatstranspose += numberzeros;
		(*p_currobject)[level] += numberzeros;
		for(i=0; i < (*p_contparameters)[level].number; i++)
			IncrementParameter(i,p_contparameters,level,numberzeros);
		goto NEXTTOKEN;
		}
	if(m == T7 && ((p > 16383) || (p < Jbol && ((*p_MIDIsize)[p] > ZERO
			|| (*p_CsoundSize)[p] > ZERO)))) {
		/* Out-time object or out-time simple note */
		skipzeros = FALSE;
		if(p > 16383 || (p > 1 && ((*p_MIDIsize)[p] > ZERO || (*p_CsoundSize)[p] > ZERO))) {
			/* Sound-object */
			kobj++;
			(*p_numberobjects) = kobj;
			ShowProgress(kobj);
			(*p_Instance)[kobj].object = - p;
			if(AttachObjectLists(kobj,nseq,p_waitlist,p_scriptlist,&newswitch,
				currswitchstate) == ABORT) goto ENDDIAGRAM;
			ip = Class((*p_im)[nseq]);
			if(Plot(OUTTIME,&nseqplot,&iplot,&overstrike,FALSE,p_nmax,p_maxcol,p_im,p_Seq,
					&nseq,maxseqapprox,ip,kobj) != OK) {
				goto ENDDIAGRAM;
				}
			if(overstrike) {
				kobj--;
				(*p_numberobjects) = kobj;
				}
			if((*p_contparameters)[level].number > 0) {
				for(i=0; i < (*p_contparameters)[level].number; i++) {
					UpdateParameter(i,p_contparameters,level,ZERO);
					} 
				}
			currentparameters.currvel = startvel + (velincrement * ibeatsvel / maxbeatsvel);
			currentparameters.currarticul = startarticul + (articulincrement * ibeatsarticul / maxbeatsarticul);
			currentparameters.currtranspose = starttranspose + (transposeincrement * ibeatstranspose / maxbeatstranspose);
			
			currentparameters.map0.p1 = startmap.p1
				+ (mapincrement.p1 * ibeatsmap / maxbeatsmap);
			currentparameters.map0.q1 = startmap.q1
				+ (mapincrement.q1 * ibeatsmap / maxbeatsmap);
			currentparameters.map0.p2 = startmap.p2
				+ (mapincrement.p2 * ibeatsmap / maxbeatsmap);
			currentparameters.map0.q2 = startmap.q2
				+ (mapincrement.q2 * ibeatsmap / maxbeatsmap);
			currentparameters.map1 = currentparameters.map0;
			
			if(!overstrike) {
				if(SetObjectParams(TRUE,level,(nseqplot-1),p_articul,kobj,p,
						&currentparameters,p_contparameters,h_table) != OK)
					goto ENDDIAGRAM;
				}
			for(i=0; i < (*p_contparameters)[level].number; i++) {
				if((*((*p_contparameters)[level].values))[i].active) {
					if((*((*p_contparameters)[level].values))[i].increment == 0)
						(*((*p_contparameters)[level].values))[i].active = FALSE;
					}
				}
			}

DONEOUTTIMEOBJECT:
//		nseq = nseqmem;
		oldp = -1;
		goto NEXTTOKEN;
		}
//	if(trace_diagram) BPPrintMessage(odInfo,"FillPhaseDiagram() m = %d p = %d level = %ld\n",m,p,(long)level);
	switch(m) {
		case T0:
			switch(p) {
				case 12:			/* '{' */
				case 22:
				//	BPPrintMessage(odInfo,"\nFillPhaseDiagram() { m = %d p = %d level = %ld\n",m,p,(long)level);
					skipzeros = FALSE;
					for(i=0; i < (*p_contparameters)[level].number; i++) {
						UpdateParameter(i,p_contparameters,level,ZERO);
						}
					if(CopyContinuousParameters(p_contparameters,level,p_contparameters,level+1)
						!= OK) goto ENDDIAGRAM;
						
					level++;
					if(level >= Maxlevel) {
						if(Beta) Alert1("=> Err. FillPhaseDiagram(). level >= Maxlevel");
						else Alert1("=> Unexpectedly the phase diagram can't be created. You should send this project to the designers");
						goto ENDDIAGRAM;
						}
						
					if(CopyContinuousParameters(p_contparameters,level,p_deftcontparameters,level)
						!= OK) goto ENDDIAGRAM;
					
					(*p_deftvelincrement)[level] = velincrement;
					(*p_deftstartvel)[level] = startvel;
					(*p_deftrndvel)[level] = rndvel;
					(*p_deftvelcontrol)[level] = velcontrol;
					(*p_deftibeatsvel)[level] = ibeatsvel;
					(*p_deftmaxbeatsvel)[level] = maxbeatsvel;
					
					(*p_deftarticulincrement)[level] = articulincrement;
					(*p_deftstartarticul)[level] = startarticul;
					(*p_deftibeatsarticul)[level] = ibeatsarticul;
					(*p_deftmaxbeatsarticul)[level] = maxbeatsarticul;
					
					(*p_deftmapincrement)[level] = mapincrement;
					(*p_deftstartmap)[level] = startmap;
					(*p_deftibeatsmap)[level] = ibeatsmap;
					(*p_deftmaxbeatsmap)[level] = maxbeatsmap;
					
					(*p_defttransposeincrement)[level] = transposeincrement;
					(*p_deftstarttranspose)[level] = starttranspose;
					(*p_deftibeatstranspose)[level] = ibeatstranspose;
					(*p_deftmaxbeatstranspose)[level] = maxbeatstranspose;
					
					(*p_deftxpandval)[level] = currentparameters.xpandval;
					(*p_deftxpandkey)[level] = currentparameters.xpandkey;
					
					if((*p_deftxpandkey)[level] > -1)
						orgval = (*p_NumberConstant)[(*p_deftxpandval)[level]];
					else orgval = 1.;
					
					(*p_deftnseq)[level] = nseq;
					
					for(i=0; i < (*p_contparameters)[level].number; i++) {
						if((*((*p_contparameters)[level].values))[i].active) {
							if(SetVariation(-1,p_deftcurrentparameters,&currentparameters,p_contparameters,
									level,i,id,*pp_buff,speed,scale,&endval,&mapendvalue,
									&maxbeats,h_table) != OK)
								goto ENDDIAGRAM;
							}
						}
					
					(*p_deftcurrentparameters)[level] = currentparameters;
					
					(*p_currobject)[level] = (*p_objectsfound)[level] = 0.;
					(*p_seq)[level] = 0;
					
					(*p_origin)[level] = (*p_im)[nseq];
					(*p_im)[nseq] = (*p_origin)[level];
					oldp = -1;
					goto NEXTTOKEN;
					break;
				
				case 13:					/* '}' */
				case 23:
					skipzeros = FALSE;
					if(level >= Maxlevel) {
						if(Beta) Alert1("=> Err. FillPhaseDiagram(). level >= Maxlevel");
						else Alert1("=> Unexpectedly the phase diagram can't be created. You should send this project to the designers");
						goto ENDDIAGRAM;
						}
					inext = (*p_im)[nseq];
					classofinext = Class(inext);
					
					nseq = (*p_deftnseq)[level] - 1;
					/* (*p_maxcol)[nseq] must be checked because of concatenated time-objects */
					while((++nseq) <= (*p_nmax) && (*p_maxcol)[nseq] > classofinext);
					if(nseq >= Minconc) {
						BPPrintMessage(odError,"=> Formula too complex (case 1)\n");
						Panic = TRUE;
						if(Beta) goto ENDDIAGRAM;
						else goto NEWSEQUENCE;
						}
					if(nseq > (*p_nmax)) {
						BPPrintMessage(odError,"=> Err. FillPhaseDiagram(). nseq > (*p_nmax) after '}'\n");
						if((gotnewline=MakeNewLineInPhaseTable(nseq,p_nmax,p_im,maxseqapprox,p_maxcol))
								!= OK) {
							if(gotnewline == ABORT) goto ENDDIAGRAM;
							goto NEXTTOKEN;
							}
						}
					(*p_im)[nseq] = inext;
					
					if((*p_seq)[level] == 0)
						(*p_objectsfound)[level] = (*p_currobject)[level];
					objectsfound = (*p_objectsfound)[level];
					(*p_currobject)[level-1] += objectsfound;
					
					currentparameters = (*p_deftcurrentparameters)[level];
					
					velincrement = (*p_deftvelincrement)[level];
					startvel = (*p_deftstartvel)[level];
					rndvel = (*p_deftrndvel)[level];
					velcontrol = (*p_deftvelcontrol)[level];
					ibeatsvel = (*p_deftibeatsvel)[level] + objectsfound;
					maxbeatsvel = (*p_deftmaxbeatsvel)[level];
					
					articulincrement = (*p_deftarticulincrement)[level];
					startarticul = (*p_deftstartarticul)[level];
					ibeatsarticul = (*p_deftibeatsarticul)[level] + objectsfound;
					maxbeatsarticul = (*p_deftmaxbeatsarticul)[level];
					
					mapincrement = (*p_deftmapincrement)[level];
					startmap = (*p_deftstartmap)[level];
					ibeatsmap = (*p_deftibeatsmap)[level] + objectsfound;
					maxbeatsmap = (*p_deftmaxbeatsmap)[level];
					
					transposeincrement = (*p_defttransposeincrement)[level];
					starttranspose = (*p_deftstarttranspose)[level];
					ibeatstranspose = (*p_deftibeatstranspose)[level] + objectsfound;
					maxbeatstranspose = (*p_deftmaxbeatstranspose)[level];
					
			/*		currentparameters.xpandval = (*p_deftxpandval)[level];
					currentparameters.xpandkey = (*p_deftxpandkey)[level]; */
					
					level--;
					
					if((*p_deftxpandkey)[level] > -1)
						orgval = (*p_NumberConstant)[(*p_deftxpandval)[level]];
					else orgval = 1.;
					
					for(i=0; i < (*p_contparameters)[level].number; i++)
						IncrementParameter(i,p_contparameters,level,objectsfound);
					
					oldp = -1;
					goto NEXTTOKEN;
					break;
					
				case 14:				/* ',' */
			//		BPPrintMessage(odInfo,"\nFillPhaseDiagram() , m = %d p = %d level = %ld\n",m,p,(long)level);
					skipzeros = FALSE;
					inext = (*p_origin)[level];
					classofinext = Class(inext);
					if(trace_diagram) BPPrintMessage(odInfo,"nseq = %ld level = %ld Minconc = %ld inext = %.0f classofinext = %ld\n",(long)nseq,(long)level,(long)Minconc,(double)inext,(long)classofinext);
					
					/* (*p_maxcol)[nseq] must be checked because of concatenated time-objects */
					if(trace_diagram) BPPrintMessage(odInfo,"@@ (*p_maxcol)[%ld + 1] = %ld\n",(long)nseq,(long)(*p_maxcol)[nseq+1]);
					while((++nseq) <= (*p_nmax) && (*p_maxcol)[nseq] > classofinext);
					if(nseq >= Minconc) {
						Panic = TRUE;
						if(Beta) {
							BPPrintMessage(odError,"=> Formula too complex (case 2)\n"); // BB 2021-01-29
							BPPrintMessage(odError,"nseq = %ld Minconc = %ld classofinext = %ld inext = %.0f\n",(long)nseq,(long)Minconc,(long)classofinext,inext);
							goto ENDDIAGRAM;
							}
						goto NEWSEQUENCE;
						}
					if(nseq > (*p_nmax)) {
						if(Beta) Println(wTrace,"\n=> Error 3 FillPhaseDiagram(). nseq > (*p_nmax) after ','");
						if((gotnewline=MakeNewLineInPhaseTable(nseq,p_nmax,p_im,maxseqapprox,p_maxcol))
								!= OK) {
							if(gotnewline == ABORT) goto ENDDIAGRAM;
							goto NEXTTOKEN;
							}
						}
NEWSEQUENCE:
					(*p_im)[nseq] = inext;
					
					currentparameters = (*p_deftcurrentparameters)[level];
					
					CopyContinuousParameters(p_deftcontparameters,level,p_contparameters,level);
					
					velincrement = (*p_deftvelincrement)[level];
					startvel = (*p_deftstartvel)[level];
					rndvel = (*p_deftrndvel)[level];
					velcontrol = (*p_deftvelcontrol)[level];
					ibeatsvel = (*p_deftibeatsvel)[level];
					maxbeatsvel = (*p_deftmaxbeatsvel)[level];
					
					articulincrement = (*p_deftarticulincrement)[level];
					startarticul = (*p_deftstartarticul)[level];
					ibeatsarticul = (*p_deftibeatsarticul)[level];
					maxbeatsarticul = (*p_deftmaxbeatsarticul)[level];
					
					mapincrement = (*p_deftmapincrement)[level];
					startmap = (*p_deftstartmap)[level];
					ibeatsmap = (*p_deftibeatsmap)[level];
					maxbeatsmap = (*p_deftmaxbeatsmap)[level];
					
					transposeincrement = (*p_defttransposeincrement)[level];
					starttranspose = (*p_deftstarttranspose)[level];
					ibeatstranspose = (*p_deftibeatstranspose)[level];
					maxbeatstranspose = (*p_deftmaxbeatstranspose)[level];

			/*		currentparameters.xpandval = (*p_deftxpandval)[level];
					currentparameters.xpandkey = (*p_deftxpandkey)[level]; */
					
			/*		if((*p_deftxpandkey)[level] > -1)
						orgval = (*p_NumberConstant)[(*p_deftxpandval)[level]];
					else orgval = 1.; */
					
					for(i=0; i < (*p_contparameters)[level].number; i++) {
						if((*((*p_contparameters)[level].values))[i].active) {
							UpdateParameter(i,p_contparameters,level,ZERO);
							if(SetVariation(-1,p_deftcurrentparameters,&currentparameters,p_contparameters,
									level,i,id,*pp_buff,speed,scale,&endval,&mapendvalue,
									&maxbeats,h_table) != OK)
								goto ENDDIAGRAM;
							}
						}
					
					if((*p_seq)[level] == 0) (*p_objectsfound)[level] = (*p_currobject)[level];
					
					(*p_seq)[level]++;
					(*p_currobject)[level] = 0.;
					
					oldp = -1;
					goto NEXTTOKEN;
					break;
					
				case 11:				/* '/' speed up */
					skipzeros = FALSE;
					speed = GetScalingValue((*pp_buff),id);
					if(speed < 1.) goto ENDDIAGRAM;
					if(scale != 0.) {
						tempo = speed / scale;
						prodtempo = (Prod / tempo);
						}
					else tempo = prodtempo = 0.;
					toofast = (tempo > tempomax || tempo == 0.);
					id += 4;
					oldp = -1;
					goto NEXTTOKEN;
					break;
					
				case 25:				/* '\' speed down */
					skipzeros = FALSE;
					speed = GetScalingValue((*pp_buff),id);
					if(speed < 1.) goto ENDDIAGRAM;
					speed = 1. / speed;
					if(scale != 0.) {
						tempo = speed / scale;
						prodtempo = (Prod / tempo);
						}
					else tempo = prodtempo = 0.;
					toofast = (tempo > tempomax || tempo == 0.);
					id += 4;
					oldp = -1;
					goto NEXTTOKEN;
					break;
					
				case 21:				/* '*' scale up */
					skipzeros = FALSE;
					scale = GetScalingValue((*pp_buff),id);
					if(scale != 0.) {
						tempo = speed / scale;
						prodtempo = (Prod / tempo);
						}
					else tempo = prodtempo = 0.;
					toofast = (tempo > tempomax || tempo == 0.);
					id += 4;
					oldp = -1;
					goto NEXTTOKEN;
					break;
					
				case 24:				/* '**' scale down */
					skipzeros = FALSE;
					s = GetScalingValue((*pp_buff),id);
					if(s < 1.) goto ENDDIAGRAM;
					scale = 1. / s;
					if(scale < InvMaxTempo) scale = 0.;
					if(scale != 0.) {
						tempo = speed / scale;
						prodtempo = (Prod / tempo);
						}
					else tempo = prodtempo = 0.;
					toofast = (tempo > tempomax || tempo == 0.);
					id += 4;
					oldp = -1;
					goto NEXTTOKEN;
					break;
					
				case 19:			/* '&' preceding terminal symbol */
	//				if(level >= Maxlevel) goto NEXTTOKEN;
					skipzeros = FALSE;
					if(trace_diagram)
						BPPrintMessage(odInfo,"'&' preceding id = %ld nseq = %ld level = %ld\n",(long)id,(long)nseq,(long)level);
					foundendconcatenation = TRUE;
					goto NEXTTOKEN;
					break;
					
				case 18:				/* '&' following terminal or simple note */
					if(foundendconcatenation) {
						BPPrintMessage(odInfo,"foundendconcatenation -> goto NEXTTOKEN\n");
						goto NEXTTOKEN;
						}
					skipzeros = FALSE;
					inext = (*p_im)[nseq];
					classofinext = Class(inext);
					
					if((oldm != T3 && oldm != T25) || oldp < 1 || id < 2L) {
						sprintf(Message,"=> Concatenation '&' should follow a sound-object or simple note. One of them is misplaced");
						if(!ScriptExecOn) Alert1(Message);
						else Println(wTrace,Message);
						goto NEXTTOKEN;
						}
					// BPPrintMessage(odInfo,"\nGetSymbolicDuration() '&' following terminal nseq = %ld level = %ld m = %ld p = %ld speed = %.2f scale = %.2f id = %ld kobj = %d\n",(long)nseq,(long)level,(long)oldm,(long)oldp,speed,scale,id,(int)kobj);
					numberzeros
						= GetSymbolicDuration(NO,*pp_buff,oldm,oldp,id-2L,speed,
							scale,channel,instrument,foundendconcatenation,level) - prodtempo;
					if(trace_diagram) 
						BPPrintMessage(odInfo,"End GetSymbolicDuration() numberzeros = %.2f\n\n",numberzeros);
					if(numberzeros < 0.) numberzeros = 0.;
							
					if(PutZeros(toofast,p_im,p_maxcol,nseq,maxseqapprox,numberzeros,p_nmax,0) != OK) goto ENDDIAGRAM;
					
					if(oldm == T25) {
						(*(p_Tie_note[channel]))[oldp] = TRUE; // Added by BB 2021-02-07 
						// BPPrintMessage(odInfo,"Tied note p = %d\n",(int)oldp);
						}
					else if(oldm == T3) (*(p_Tie_event[instrument]))[oldp] = TRUE; // Added by BB 2021-02-07
				
					while((++nseq) <= (*p_nmax) && (*p_maxcol)[nseq] > classofinext);
					if(nseq >= Minconc) {
						Panic = TRUE;
						if(Beta) {
							Alert1("=> Err. FillPhaseDiagram(). nseq >= Minconc after '&'");
							goto ENDDIAGRAM;
							}
						goto NEWSEQUENCE;
						}
					if(nseq > (*p_nmax)) {
						if((gotnewline=MakeNewLineInPhaseTable(nseq,p_nmax,p_im,
								maxseqapprox,p_maxcol)) != OK) {
							if(gotnewline == ABORT) goto ENDDIAGRAM;
							goto NEXTTOKEN;
							}
						}
					(*p_im)[nseq] = inext;
					oldp = -1;
					goto NEXTTOKEN;
					break;
				}
			break;
		case T8:	/* Synchronization tag */
	//		if(nseq >= Maxconc) goto NEXTTOKEN;
//			if(level >= Maxlevel) goto NEXTTOKEN;
			if((tag=(p_list**) GiveSpace(sizeof(p_list))) == NULL) goto ENDDIAGRAM;
			(**tag).x = p; (**tag).p = NULL;
			if((*p_waitlist)[nseq] == NULL) (*p_waitlist)[nseq] = tag;
			else {
				ptag = (*p_waitlist)[nseq];
				while((**ptag).p != NULL) ptag = (**ptag).p;
				(**ptag).p = tag;
				}
			break;
		case T10:	/* Channel assignment _chan() */
			currentparameters.currchan = value = FindValue(m,p,currentparameters.currchan);
			if(value == Infpos) goto ENDDIAGRAM;
			channel = roundf(value);
			break;
		case T32:	/* Instrument assignment _ins() */
			currentparameters.currinstr = value = FindValue(m,p,currentparameters.currchan);
			if(value == Infpos) goto ENDDIAGRAM;
			instrument = roundf(value);
			break;
		case T44:	/* _scale() */
			currentparameters.scale = p % MAXSTRINGCONSTANTS;
			if(trace_scale) BPPrintMessage(odInfo,"FillPhaseDiagram() _scale() value = %ld\n",(long)value);
			if(currentparameters.scale > -1)
				newkeyval = (p - currentparameters.scale) / MAXSTRINGCONSTANTS;
			else newkeyval = BlockScaleOnKey;
			if(trace_scale) BPPrintMessage(odInfo,"newkeyval = %ld currentparameters.scale = %d\n",(long)newkeyval,currentparameters.scale);
			if(newkeyval < 0 || newkeyval > 127) {
				if(Beta) Println(wTrace,"=> Err. FillPhaseDiagram(). newblockkey < 0 || newblockkey > 127");
				currentparameters.blockkey = BlockScaleOnKey;
				}
			else if(currentparameters.scale > -1) {
				newval = (*p_NumberConstant)[newkeyval];
				if(newval < 0 || newval > 127) {	 
					BPPrintMessage(odError,"\n=> Error on block key in \"_scale()\" statement. It should be in range [0..127], or a note in your convention. Its default value will be used: %d\n",BlockScaleOnKey);
					newval = BlockScaleOnKey;
					}
				if(trace_scale) BPPrintMessage(odInfo,"blockkey = %ld\n",(long)newval);
				currentparameters.blockkey = newval;
				}
			else currentparameters.blockkey = BlockScaleOnKey;
			break;
		case T21:	/* _pitchrange() */
			PitchbendRange[currentparameters.currchan] = p;
			break;
		case T22:	/* _pitchrate() */
			PitchbendRate[currentparameters.currchan] = p;
			break;
		case T23:	/* _modrate() */
			ModulationRate[currentparameters.currchan] = p;
			break;
		case T24:	/* _pressrate() */
			PressRate[currentparameters.currchan] = p;
			break;
		case T27:	/* _volumerate() */
			VolumeRate[currentparameters.currchan] = p;
			break;
		case T30:	/* _panrate() */
			PanoramicRate[currentparameters.currchan] = p;
			break;
		case T28:	/* _volumecontrol() */
			VolumeControl[currentparameters.currchan] = value
				= FindValue(m,p,currentparameters.currchan);
			if(value == Infpos) goto ENDDIAGRAM;
			break;
		case T31:	/* _pancontrol() */
			PanoramicControl[currentparameters.currchan] = value
				= FindValue(m,p,currentparameters.currchan);
			if(value == Infpos) goto ENDDIAGRAM;
			break;
		case T14:	/* Modulation assignment _mod() */
	//		if(level >= Maxlevel) goto NEXTTOKEN;
			i = IMODULATION;
			i = FindParameterIndex(p_contparameters,level,i);
			if(i >= 0) {
				value = FindValue(m,p,currentparameters.currchan);
				if(value == Infpos) goto ENDDIAGRAM;
				if(AssignValue(i,value,0,level,p_numberobjects,p_deftcurrentparameters,&currentparameters,
					p_contparameters,id,pp_buff,tempo,scale,h_table) != OK)
						goto ENDDIAGRAM;
				}
			break;
		case T35:	/* _value() */
	//		if(level >= Maxlevel) goto NEXTTOKEN;
			paramnameindex = (p % 256);
			paramvalueindex = (p - paramnameindex) / 256;
			i = FindParameterIndex(p_contparameters,level,paramnameindex);
			if(i >= 0) {
				if(Beta) {
					if(paramvalueindex >= (MyGetHandleSize((Handle)p_NumberConstant)
						/ sizeof(double))) {
						Alert1("=> Err FillPhaseDiagram(). paramvalueindex overflow");
						goto ENDDIAGRAM;
						}
					}
				value = FindValue(m,p,currentparameters.currchan);
				if(trace_diagram) BPPrintMessage(odInfo, "_value() paramnameindex = %d i = %d value = %.3f\n",paramnameindex,i,value);
				if(AssignValue(i,value,0,level,p_numberobjects,p_deftcurrentparameters,&currentparameters,
					p_contparameters,id,pp_buff,tempo,scale,h_table) != OK)
						goto ENDDIAGRAM;
				}
			break;
		case T15:	/* Pitchbend assignment _pitchbend() */
	//		if(level >= Maxlevel) goto NEXTTOKEN;
			i = IPITCHBEND;
			i = FindParameterIndex(p_contparameters,level,i);
			if(i >= 0) {
				value = FindValue(m,p,currentparameters.currchan);
				if(value == Infpos) goto ENDDIAGRAM;
				if(AssignValue(i,value,0,level,p_numberobjects,p_deftcurrentparameters,&currentparameters,
					p_contparameters,id,pp_buff,tempo,scale,h_table) != OK)
						goto ENDDIAGRAM;
				}
			break;
		case T16:	/* Pressure assignment _press() */
	//		if(level >= Maxlevel) goto NEXTTOKEN;
			i = IPRESSURE;
			i = FindParameterIndex(p_contparameters,level,i);
			if(i >= 0) {
				value = FindValue(m,p,currentparameters.currchan);
				if(value == Infpos) goto ENDDIAGRAM;
				if(AssignValue(i,value,0,level,p_numberobjects,p_deftcurrentparameters,&currentparameters,
					p_contparameters,id,pp_buff,tempo,scale,h_table) != OK)
						goto ENDDIAGRAM;
				}
			break;
		case T19:	/* Volume assignment _volume() */
	//		if(level >= Maxlevel) goto NEXTTOKEN;
			i = IVOLUME;
			i = FindParameterIndex(p_contparameters,level,i);
			if(i >= 0) {
				value = FindValue(m,p,currentparameters.currchan);
				if(value == Infpos) goto ENDDIAGRAM;
				if(AssignValue(i,value,p,level,p_numberobjects,p_deftcurrentparameters,&currentparameters,
					p_contparameters,id,pp_buff,tempo,scale,h_table) != OK)
						goto ENDDIAGRAM;
				}
			break;
		case T29:	/* Panoramic assignment _pan() */
	//		if(level >= Maxlevel) goto NEXTTOKEN;
			i = IPANORAMIC;
			i = FindParameterIndex(p_contparameters,level,i);
			if(i >= 0) {
				value = FindValue(m,p,currentparameters.currchan);
				if(value == Infpos) goto ENDDIAGRAM;
				if(AssignValue(i,value,p,level,p_numberobjects,p_deftcurrentparameters,&currentparameters,
					p_contparameters,id,pp_buff,tempo,scale,h_table) != OK)
						goto ENDDIAGRAM;
				}
			break;
		case T26:	/* Transpose assignment _transpose() */
	//		if(level >= Maxlevel) goto NEXTTOKEN;
			value = FindValue(m,p,currentparameters.currchan);
			if(value == Infpos) goto ENDDIAGRAM;
			
			currentparameters.currtranspose = starttranspose
				= (*p_deftcurrentparameters)[level].currtranspose + value;
			currentparameters.lastistranspose = TRUE;
			
			if(SetVariation(m,p_deftcurrentparameters,&currentparameters,p_contparameters,level,-1,id,*pp_buff,
					speed,scale,&endtranspose,&mapendvalue,&maxbeatstranspose,h_table) != OK)
				goto ENDDIAGRAM;
			ibeatstranspose = 0;
			if(maxbeatstranspose == 0 || currentparameters.transposemode == FIXED) {
				maxbeatstranspose = 1; transposeincrement = 0;
				}
			else transposeincrement = endtranspose - starttranspose;
			break;
		case T40:	/* _keyxpand */
//			if(level >= Maxlevel) goto NEXTTOKEN;
		//	currentparameters.xpandkey = p % 256;
			currentparameters.xpandkey = p % MAXSTRINGCONSTANTS;
			if(currentparameters.xpandkey < 0) {
				if(Beta) Println(wTrace,"=> Err. FillPhaseDiagram(). xpandkey < 0");
				currentparameters.xpandkey = -1;
				}
			if(currentparameters.xpandkey > -1)
				newxpandval = (p - currentparameters.xpandkey) / MAXSTRINGCONSTANTS;
			else {
				newxpandval = 0;
//				currentparameters.xpandkey = 0; 
				}
			if(newxpandval < 0 || newxpandval >= maxparam) {
				if(Beta) Println(wTrace,"=> Err. FillPhaseDiagram(). newxpandval < 0 || newxpandval >= maxparam");
				newxpandval = 0;
				}
			if(currentparameters.xpandkey > -1
					&& currentparameters.xpandkey == (*p_deftcurrentparameters)[level].xpandkey) {
				newval = (*p_NumberConstant)[newxpandval];
				sprintf(line,"%.4f",orgval * newval);
				if((r=FixNumberConstant(line)) < 0) {
					result = ABORT;
					goto ENDDIAGRAM;
					}
				currentparameters.xpandval = r;
				}
			else currentparameters.xpandval = newxpandval;
			currentparameters.lastistranspose = FALSE;
			break;
		case T41:	/* Time fluctuation assignment _rndtime() */
			currentparameters.randomtime = p;
			break;
		case T42:	/* Reseed and reset random sequence _srand() */
			currentparameters.seed = p;
			break;
		case T11:	/* Velocity assignment _vel() */
	//		if(level >= Maxlevel) goto NEXTTOKEN;
			startvel = FindValue(m,p,currentparameters.currchan);
			if(startvel == Infpos) goto ENDDIAGRAM;
			currentparameters.currvel = startvel;
			if(p > 127) /* Velocity is assigned by controller Kx */
				velcontrol = currentparameters.velcontrol = p - 128;
			else velcontrol = currentparameters.velcontrol = -1;
			if(SetVariation(m,p_deftcurrentparameters,&currentparameters,p_contparameters,level,-1,id,
					*pp_buff,speed,scale,&endvel,&mapendvalue,&maxbeatsvel,h_table) != OK)
				goto ENDDIAGRAM;
			ibeatsvel = 0.;
			if(maxbeatsvel == 0. || currentparameters.velmode == FIXED) {
				maxbeatsvel = 1.; velincrement = 0.;
				}
			else velincrement = endvel - startvel;
			break;
		case T20:	/* Articulation assignment _legato() or _staccato() */
	//		if(level >= Maxlevel) goto NEXTTOKEN;
			startarticul = FindValue(m,p,currentparameters.currchan);
			if(startarticul == Infpos) goto ENDDIAGRAM;
			currentparameters.currarticul = startarticul;
			if(SetVariation(m,p_deftcurrentparameters,&currentparameters,p_contparameters,level,-1,id,*pp_buff,
					speed,scale,&endarticul,&mapendvalue,&maxbeatsarticul,h_table) != OK)
				goto ENDDIAGRAM;
			ibeatsarticul = 0;
			if(maxbeatsarticul == 0 || currentparameters.articulmode == FIXED) {
				maxbeatsarticul = 1; articulincrement = 0;
				}
			else articulincrement = endarticul - startarticul;
			break;
		case T13:	/* Scriptline assignment */
	//		if(level >= Maxlevel) goto NEXTTOKEN;
	//		if(nseq >= Maxconc) goto NEXTTOKEN;
			if((tag=(p_list**) GiveSpace(sizeof(p_list))) == NULL) goto ENDDIAGRAM;
			(**tag).x = p; (**tag).p = NULL;
			if((*p_scriptlist)[nseq] == NULL) (*p_scriptlist)[nseq] = tag;
			else {
				ptag = (*p_scriptlist)[nseq];
				while((**ptag).p != NULL) {
					ptag = (**ptag).p;
					}
				(**ptag).p = tag;
				}
			break;
		case T37:	/* Key map assignment _keymap() */
//			if(level >= Maxlevel) goto NEXTTOKEN;
			startmap.p1 = p % 128;
			startmap.q1 = (p - (p % 128)) / 128;
			if(startmap.p1 == Infpos) goto ENDDIAGRAM;
			id += 2;
			m = (tokenbyte) (**pp_buff)[id];
			p = (tokenbyte) (**pp_buff)[id+1];
			startmap.p2 = p % 128;
			startmap.q2 = (p - (p % 128)) / 128;
			currentparameters.map0 = startmap;
			if(currentparameters.mapmode == OFF) currentparameters.mapmode = FIXED;
			if(SetVariation(T37,p_deftcurrentparameters,&currentparameters,p_contparameters,level,-1,id,*pp_buff,
					speed,scale,&endval,&mapendvalue,&maxbeatsmap,h_table) != OK)
				goto ENDDIAGRAM;
			ibeatsmap = 0;
			if(maxbeatsmap == 0 || currentparameters.mapmode == FIXED) {
				maxbeatsmap = 1;
				mapincrement.p1 = mapincrement.q1 = mapincrement.p2 = mapincrement.q2 = 0;
				if(startmap.p1 == startmap.q1 && startmap.p2 == startmap.q2) {
					currentparameters.mapmode = OFF;
					startmap.p1 = startmap.q1 = 0;
					startmap.p2 = startmap.q2 = 127;
					}
				}
			else {
				mapincrement.p1 = mapendvalue.p1 - startmap.p1;
				mapincrement.q1 = mapendvalue.q1 - startmap.q1;
				mapincrement.p2 = mapendvalue.p2 - startmap.p2;
				mapincrement.q2 = mapendvalue.q2 - startmap.q2;
				}
			break;
		case T38:	/* Random velocity assignment _rndvel() */
			currentparameters.rndvel = p;
			break;
		case T17:	/* _switchon() */
	//		if(level >= Maxlevel) goto NEXTTOKEN;
			v = p % 128;
			if(v < 64 || v > 95)  {
				if(Beta) Alert1("=> Err. FillPhaseDiagram(). v < 64 || v > 95");
				goto ENDDIAGRAM;
				}
			ch = (p - v) / 128 - 1;
			if(ch < 0 || ch > 15) {
				if(Beta) Alert1("=> Err. FillPhaseDiagram(). ch < 0 || ch > 15");
				goto ENDDIAGRAM;
				}
			currswitchstate[ch] = currswitchstate[ch] | (1L << (v - 64));
			newswitch = TRUE;
			break;
		case T18:	/* _switchoff() */
	//		if(level >= Maxlevel) goto NEXTTOKEN;
			v = p % 128;
			if(v < 64 || v > 95)  {
				if(Beta) Alert1("=> Err. FillPhaseDiagram(). v < 64 || v > 95");
				goto ENDDIAGRAM;
				}
			ch = (p - v) / 128 - 1;
			if(ch < 0 || ch > 15) {
				if(Beta) Alert1("=> Err. FillPhaseDiagram(). ch < 0 || ch > 15");
				goto ENDDIAGRAM;
				}
			currswitchstate[ch] = currswitchstate[ch] & ~(1L << (v - 64));
			newswitch = TRUE;
			break;
		default:
			oldp = -1;
		}
NEXTTOKEN:
	id = id;
	}

if(DoSystem() != OK) return(ABORT);

// Mark exact ends of sequences and item

imax = ZERO;
for(nseq=nseqmem=0; nseq <= (*p_nmax); nseq++) {
	(*p_maxcol)[nseq]++;
	ip = (*p_maxcol)[nseq];
	// if(trace_diagram) BPPrintMessage(odInfo,"@ nseq = %ld maxcol[nseq] = %ld\n",nseq,(*p_maxcol)[nseq]);
	oldnseq = nseq;
	if(Plot(INTIME,&nseqplot,&iplot,&overstrike,FALSE,p_nmax,p_maxcol,p_im,p_Seq,&nseq,
		maxseqapprox,ip,1) != OK) goto ENDDIAGRAM;
	/* This shortens objects that would be too long because of roundings when Kpress > 1 */
	/* Note than an object that was just at the end (in too fast tempo) hasn't been overstriken */
	nseq = oldnseq; // nseq might have been changed by Plot() // Added by BB 2021-03-22
	if((*p_maxcol)[nseq] > imax) {
		imax = (*p_maxcol)[nseq];
		nseqmem = nseq;
		}
	}

LOOKATEND:
k = (*((*p_Seq)[nseqmem]))[imax];
if(k < 0 || k > kobj) {
	if(Beta) Println(wTrace,"=> Err. FillPhaseDiagram(). k < 0 || k > kobj");
	k = 0;
	}
j = (*p_Instance)[k].object;
if(trace_diagram) BPPrintMessage(odInfo,"LOOKATEND nseqmem = %ld imax = %ld k = %d j = %d\n",nseqmem,imax,k,j);
if(j < 0) {
	/* Oops! we are outside the table */
	if(Beta) Println(wTrace,"=> Err. FillPhaseDiagram(). LOOKATEND: j < 0");
	imax--;
	if(imax > ZERO) goto LOOKATEND;
	}

CorrectionFactor = 1.;

if(imax > 0.) { 
	if(imax > 1.) {
		CorrectionFactor = (((Ratio * Pduration) / Qduration) / (Kpress * (imax - 1.)));
	/* This compensates errors due to overflow in calculating Prod and Ratio */
		if(CorrectionFactor < 0.95 || CorrectionFactor > 1.05) {
			BPPrintMessage(odError,"=> Correction factor = %.3f (imax = %ld nseqmem = %ld  nseqmax = %ld) - probable error",CorrectionFactor,(long)imax,(long)nseqmem,(long)(*p_nmax));
			if(!PlayChunks) BPPrintMessage(odError,"\n");
			else BPPrintMessage(odError," in chunk #%d\n\n",Chunk_number);
			}
		}
	if((trace_diagram || trace_toofast) && (CorrectionFactor != 1.0)) {
		BPPrintMessage(odInfo,"Correction factor = %.3f Ratio = %.0f Pduration = %ld Qduration = %ld Kpress = %ld imax = %ld\n",(float)CorrectionFactor,(float)Ratio,(long)Pduration,(long)Qduration,(long)Kpress,(long)imax);
		}
	if(number_skipped > 0) { // Added by BB 2021-02-01
		suggested_quantization = (int)(Quantization * (tempomax + 1) / max_tempo_in_skipped_object * CorrectionFactor);
		if(suggested_quantization > 10)
			suggested_quantization = suggested_quantization - modulo(suggested_quantization,((int)Time_res / 2));
		BPPrintMessage(odError,"=> %d objects/notes have been skipped. Quantization should be less than %d ms to avoid it\n",number_skipped,suggested_quantization);
		}
	/* Let's put an out-time silence at the end of the item, and attach to it the ultimate control parameter values */
	nseq = Minconc;
	ip = imax;
	kobj++;
	(*p_numberobjects) = kobj;
	if(kobj >= Maxevent) {
	 	if(Beta) Println(wTrace,"\nErr. FillPhaseDiagram(). kobj >= Maxevent. (1)");
	 	kobj--; (*p_numberobjects) = kobj;
	 	goto LASTOBJECTDONE;
	 	}
	(*p_Instance)[kobj].object = -1;
	if(AttachObjectLists(kobj,nseq,p_waitlist,p_scriptlist,&newswitch,currswitchstate)
		== ABORT) goto ENDDIAGRAM;
	if(Plot(BORDERLINE,&nseqplot,&ip,&overstrike,TRUE,p_nmax,p_maxcol,p_im,p_Seq,&nseq,
		maxseqapprox,ip,kobj) != OK) goto ENDDIAGRAM;
	(*p_maxcol)[nseq]++;
	if(!overstrike && SetObjectParams(FALSE,-1,Minconc,p_articul,kobj,1,&currentparameters,
			p_contparameters,h_table) != OK) goto ENDDIAGRAM;
	}


// Complete last column of diagram

LASTOBJECTDONE:
(*p_maxseq) = ZERO;

for(nseq=0; nseq <= (*p_nmax); nseq++) {
	if((*p_maxcol)[nseq] < 1L) (*p_maxcol)[nseq] = 1L;
	ip = (*p_maxcol)[nseq];
	if((*p_waitlist)[nseq] != NULL || (*p_scriptlist)[nseq] != NULL || newswitch) {
		/* Append <<->> */
		kobj++; (*p_numberobjects) = kobj;
		if(kobj >= Maxevent) {
		 	if(Beta) Alert1("=> Err. FillPhaseDiagram(). kobj >= Maxevent. (2)");
		 	failed = TRUE; goto ENDDIAGRAM;
		 	}
		(*p_Instance)[kobj].object = -1;
		if(AttachObjectLists(kobj,nseq,p_waitlist,p_scriptlist,&newswitch,currswitchstate)
			== ABORT) goto ENDDIAGRAM;
		newswitch = FALSE;
		nseqmem = nseq;
		if(Plot(ANYWHERE,&nseqplot,&ip,&overstrike,TRUE,p_nmax,p_maxcol,p_im,p_Seq,
			&nseq,maxseqapprox,ip,kobj)!= OK) goto ENDDIAGRAM;
		if(!overstrike && SetObjectParams(FALSE,-1,nseq,p_articul,kobj,1,&currentparameters,
			p_contparameters,h_table) != OK) goto ENDDIAGRAM;
		nseq = nseqmem;
		}
	k = (*((*p_Seq)[nseq]))[ip];
	if(k > 1) (*p_maxcol)[nseq]++;
	(*p_imaxseq)[nseq] = ip = (*p_maxcol)[nseq];
	if(ip > (*p_maxseq)) (*p_maxseq) = ip;
	
	/* Mark the end of the sequence */
	if(Plot(INTIME,&nseqplot,&ip,&overstrike,TRUE,p_nmax,p_maxcol,p_im,p_Seq,
		&nseq,maxseqapprox,ip,-1) != OK) goto ENDDIAGRAM;
	rest = FALSE;
//	for(iseq=1L; iseq <= (*p_imaxseq)[nseq]; iseq++) { Fixed by BB 2021-03-20
	for(iseq=ZERO; iseq <= (*p_imaxseq)[nseq]; iseq++) {
		k = (*((*p_Seq)[nseq]))[iseq];
		if(k == 0) continue;
		if(k == 1) {
			if(rest == FALSE && nseq < Minconc) rest = TRUE;
			else {	/* Replace trailing silences with empty objects */
				if(iseq > 1L) (*((*p_Seq)[nseq]))[iseq] = 0;
				}
			}
		else rest = FALSE;
		}
	}
	
failed = FALSE;

ENDDIAGRAM:

if(ShowMessages || Maxevent > 500) HideWindow(Window[wInfo]);

for(i=0; i < MAXSTRINGCONSTANTS; i++) {
	h = (Handle)(*h_table)[i].point;
	MyDisposeHandle(&h);
	}
MyDisposeHandle((Handle*)&h_table);
MyDisposeHandle((Handle*)&p_origin);
MyDisposeHandle((Handle*)&p_waitlist);
MyDisposeHandle((Handle*)&p_scriptlist);
MyDisposeHandle((Handle*)&p_im);
MyDisposeHandle((Handle*)&p_maxcol);
MyDisposeHandle((Handle*)&p_objectsfound);
MyDisposeHandle((Handle*)&p_seq);
MyDisposeHandle((Handle*)&p_deftnseq);
MyDisposeHandle((Handle*)&p_currobject);

MyDisposeHandle((Handle*)&p_deftcurrentparameters);

MyDisposeHandle((Handle*)&p_deftvelincrement);
MyDisposeHandle((Handle*)&p_deftstartvel);
MyDisposeHandle((Handle*)&p_deftrndvel);
MyDisposeHandle((Handle*)&p_deftvelcontrol);
MyDisposeHandle((Handle*)&p_deftibeatsvel);
MyDisposeHandle((Handle*)&p_deftmaxbeatsvel);

MyDisposeHandle((Handle*)&p_deftarticulincrement);
MyDisposeHandle((Handle*)&p_deftstartarticul);
MyDisposeHandle((Handle*)&p_deftibeatsarticul);
MyDisposeHandle((Handle*)&p_deftmaxbeatsarticul);

MyDisposeHandle((Handle*)&p_deftmapincrement);
MyDisposeHandle((Handle*)&p_deftstartmap);
MyDisposeHandle((Handle*)&p_deftibeatsmap);
MyDisposeHandle((Handle*)&p_deftmaxbeatsmap);

MyDisposeHandle((Handle*)&p_defttransposeincrement);
MyDisposeHandle((Handle*)&p_deftstarttranspose);
MyDisposeHandle((Handle*)&p_deftibeatstranspose);
MyDisposeHandle((Handle*)&p_deftmaxbeatstranspose);

MyDisposeHandle((Handle*)&p_deftxpandval);
MyDisposeHandle((Handle*)&p_deftxpandkey);

for(level=0; level < (MyGetHandleSize((Handle)p_deftcontparameters) / sizeof(ContParameters));
		level++) {
	h = (Handle)(*p_deftcontparameters)[level].values;
	MyDisposeHandle(&h);
	}
MyDisposeHandle((Handle*)&p_deftcontparameters);

for(level=0; level < (MyGetHandleSize((Handle)p_contparameters) / sizeof(ContParameters)); level++) {
	h = (Handle)(*p_contparameters)[level].values;
	MyDisposeHandle(&h);
	}
MyDisposeHandle((Handle*)&p_contparameters);

if(failed) return(result);

(*p_maxseq)++; /* Now we got the exact value. */
Maxevent = (*p_numberobjects) + 1; 	/* Now we got the exact value. */

if(Maxevent > 5000) (*p_bigitem) = TRUE;

// Sizing down handles to save space

MySetHandleSize((Handle*)&p_Instance,(Size)Maxevent*sizeof(SoundObjectInstanceParameters));
MySetHandleSize((Handle*)&p_ObjectSpecs,(Size)Maxevent*sizeof(objectspecs**));
MySetHandleSize((Handle*)&p_articul,(Size)Maxevent*sizeof(short));
MySetHandleSize((Handle*)&p_T,(Size) (*p_maxseq+2)*sizeof(Milliseconds));

if(DoSystem() != OK) return(ABORT);

if(!foundobject && ShowGraphic && !ShowObjectGraph && !ShowPianoRoll && !ScriptExecOn && Jbol < 3
		&& Jpatt == 0 && ((*p_numberobjects) > 30) && !ToldAboutPianoRoll) {
	ToldAboutPianoRoll = TRUE;
	ShowPianoRoll = TRUE;
	BPPrintMessage(odInfo,"This project doesn't require sound-objects. We'll use piano roll display\n");
	}

if(ShowGraphic && !ShowObjectGraph && !ShowPianoRoll)
	ShowObjectGraph = TRUE;
	
if(trace_diagram) BPPrintMessage(odInfo, "Finished filling phase diagram\n");
return(OK);
}


int Plot(char where,int *p_nseqplot,unsigned long *p_iplot,char *p_overstrike,int force,
	int *p_nmax,unsigned long **p_maxcol,double **p_im,
	long ****p_seq,int *p_nseq,double maxseq,unsigned long iplot,int newk)
{
int oldk,gotnewline,nseq;

(*p_overstrike) = FALSE;

switch(where) {
	case INTIME: // Normal mode
		oldk = (*((*p_seq)[*p_nseq]))[iplot];
	//	if(iplot < 50 && *p_nseq > 2 && *p_nseq < 10 && newk == 0)
	//		BPPrintMessage(odInfo,"Plot(2) nseq = %d, iplot = %ld, oldk = %d, newk = %d\n",*p_nseq,(long)iplot, oldk, newk);
		if(!force && oldk > 1) {
			/* This may happen due to roundings after arithmetic overflows */
		//	BPPrintMessage(odInfo,"Plot(2) PLOTOUTSIDE nseq = %d, iplot = %ld, oldk = %d, newk = %d\n",*p_nseq,(long)iplot, oldk, newk);
			goto PLOTOUTSIDE;
			}
		// if(force && oldk > 1 && newk != 1) { 
		if(force && oldk > 1 && newk > 1) { // Fixed by BB 2021-01-25
			/* When newk == 1 with force, table contains arbitrary numbers, so oldk is irrelevant */
			BPPrintMessage(odError,"=> Error Plot(): overwrote object #%d\n",oldk);
			(*p_Instance)[oldk].object = 0;
			}
	//	BPPrintMessage(odInfo,"Plot(3) INTIME nseq = %d, iplot = %ld, oldk = %d, newk = %d\n",*p_nseq,(long)iplot,oldk,newk);
		(*((*p_seq)[*p_nseq]))[iplot] = newk;
		break;
	case OUTTIME: // Out-time object or simple note
PLOTOUTSIDE:
		if(iplot > (*p_iplot)) {
			(*p_iplot) = iplot;
			(*p_nseqplot) = Minconc + 1;
			}
		if(newk == 0 || newk == 1) {
//			(*p_overstrike) = TRUE;
			return(OK);
			}
		if((*p_nseqplot) >= Maxconc) {
NEWCOLUMN:
			(*p_nseqplot) = Minconc + 1;
			(*p_iplot)++;
			}
//		if((*p_iplot) >= maxseq || ((((*p_iplot) - iplot)) > (200L / Quantization) && !force)) {
		if((*p_iplot) >= maxseq || ((((*p_iplot) - iplot) > (200L / Quantization)) && !force)) { // Fixed by BB 2021-03-22, up to 2000L on 2021-03-25
			(*p_overstrike) = TRUE;
			if(trace_overstrike) BPPrintMessage(odError,"overstrike1 (*p_iplot) = %ld maxseq = %ld iplot = %ld force = %d\n",(long)(*p_iplot),(long)maxseq,(long)iplot,(int)force);
			TellSkipped();
			return(OK);
			}
		if((*p_nseqplot) > (*p_nmax)) {
		//	BPPrintMessage(odInfo,"MakeNewLineInPhaseTable(1)\n");
			if((gotnewline=MakeNewLineInPhaseTable((*p_nseqplot),p_nmax,p_im,maxseq,
					p_maxcol)) != OK) {
				if(gotnewline == ABORT) return(ABORT);
				goto NEWCOLUMN;
				}
			}
		oldk = (*((*p_seq)[*p_nseqplot]))[*p_iplot];
		(*((*p_seq)[*p_nseqplot]))[*p_iplot] = newk;
		if((*p_iplot) > (*p_maxcol)[*p_nseqplot])
			(*p_maxcol)[*p_nseqplot] = (*p_iplot);
		(*p_nseqplot)++;
		break;
	case ANYWHERE: // Append <<->> anywhere on the phase diagram
		if(newk == 0 || newk == 1) {
			return(OK);
			}
		for(nseq=0; nseq < Maxconc; nseq++) {
			if(nseq > (*p_nmax)) {
				if((*p_ObjectSpecs)[newk] == NULL && ObjScriptLine(newk) == NULL) {
					(*p_overstrike) = TRUE;
					if(trace_overstrike) BPPrintMessage(odError,"overstrike2 nseq = %ld\n",(long)nseq);
					return(OK);
					}
		//		BPPrintMessage(odInfo,"MakeNewLineInPhaseTable(2)\n");
				if((gotnewline=MakeNewLineInPhaseTable(nseq,p_nmax,p_im,maxseq,p_maxcol))
						!= OK) {
					if(gotnewline == ABORT) return(ABORT);
					force = TRUE;
					goto NEWCOLUMN;
					}
				}
			oldk = (*((*p_seq)[nseq]))[iplot];
			if(oldk > 1) continue;
			(*((*p_seq)[nseq]))[iplot] = newk;
			(*p_nseq) = nseq;
			if(iplot > (*p_maxcol)[nseq]) (*p_maxcol)[nseq] = iplot;
			return(OK);
			}
		(*p_overstrike) = TRUE;
		if(trace_overstrike) BPPrintMessage(odError,"overstrike3\n");
		break;
	case BORDERLINE: // At the end of the item
		if(newk == 0 || newk == 1) {
//			(*p_overstrike) = TRUE;
			return(OK);
			}
		nseq = Minconc;
		oldk = (*((*p_seq)[nseq]))[iplot];
		if(oldk > 1) {
			force = TRUE;
			if((*p_ObjectSpecs)[newk] == NULL && ObjScriptLine(newk) == NULL) {
//				(*p_Instance)[newk].object = 0;
				(*p_overstrike) = TRUE;
				if(trace_overstrike) BPPrintMessage(odError,"overstrike4\n");
				return(OK);
				}
			else goto PLOTOUTSIDE;
			}
		(*((*p_seq)[nseq]))[iplot] = newk;
		if(iplot > (*p_maxcol)[nseq]) (*p_maxcol)[nseq] = iplot;
		break;
	}
return(OK);
}


unsigned long Class(double i)
{
unsigned long result;

if(i < 0.) {
	/* Happens in beginning of table */
	return(ZERO);
	}
if(Kpress < 1.) {
	if(Beta) Println(wTrace,"=> Err. Class(). Kpress < 1.");
	Kpress = 1.;
	}
// if(Kpress < 2. || i < 1.) return((unsigned long)i);
if(Kpress < 2. || i < 0.) return((unsigned long)i); // Fixed by BB 2021-03-20
// result = 1L + ((unsigned long)((ceil(i) - 1.) / Kpress));
result = 1L + ((unsigned long)(floor(i) / Kpress)); // Fixed by BB 2021-03-22
// if(i < 2.) BPPrintMessage(odInfo,"Class(%.4f) = %ld\n",i,result);
return(result);
}


int FindParameterIndex(ContParameters **p_param,int level,int paramnameindex)
{
int i,j,maxnumber;
Handle h;
ContParameterSpecs **ptr;

if(level >= Maxlevel) {
	if(Beta) Alert1("=> Err. FindParameterIndex(). level >= Maxlevel");
	return(ABORT);
	}
if((*p_param)[level].values == NULL) {
	maxnumber = IPANORAMIC + 1;
	if((ptr=(ContParameterSpecs**)
		GiveSpace((Size)maxnumber * sizeof(ContParameterSpecs))) == NULL) return(ABORT);
	(*p_param)[level].values = ptr;
	for(j=0; j < maxnumber; j++) {
		(*((*p_param)[level].values))[j].index = -1;
		(*((*p_param)[level].values))[j].mode = FIXED;
		(*((*p_param)[level].values))[j].increment = 0.;
		(*((*p_param)[level].values))[j].maxbeats = 1;
		(*((*p_param)[level].values))[j].active = FALSE;
		(*((*p_param)[level].values))[j].known = FALSE;
		(*((*p_param)[level].values))[j].control = -1;
		(*((*p_param)[level].values))[j].start
			= (*((*p_param)[level].values))[j].v0
			= (*((*p_param)[level].values))[j].v1
			= 0.;
		(*((*p_param)[level].values))[j].channel = 0;
		(*((*p_param)[level].values))[j].scale = DefaultScale;
		(*((*p_param)[level].values))[j].blockkey = BlockScaleOnKey;
		(*((*p_param)[level].values))[j].imax = 0;
		(*((*p_param)[level].values))[j].ibeats = 0;
		(*((*p_param)[level].values))[j].point = NULL;
		}
	(*((*p_param)[level].values))[IPITCHBEND].start
		= (*((*p_param)[level].values))[IPITCHBEND].v0
		= (*((*p_param)[level].values))[IPITCHBEND].v1
		= DEFTPITCHBEND;
	(*((*p_param)[level].values))[IVOLUME].start
		= (*((*p_param)[level].values))[IVOLUME].v0
		= (*((*p_param)[level].values))[IVOLUME].v1
		= DeftVolume;
	(*((*p_param)[level].values))[IPANORAMIC].start
		= (*((*p_param)[level].values))[IPANORAMIC].v0
		= (*((*p_param)[level].values))[IPANORAMIC].v1
		= DeftPanoramic;
	(*((*p_param)[level].values))[IPRESSURE].start
		= (*((*p_param)[level].values))[IPRESSURE].v0
		= (*((*p_param)[level].values))[IPRESSURE].v1
		= DEFTPRESSURE;
	(*((*p_param)[level].values))[IMODULATION].start
		= (*((*p_param)[level].values))[IMODULATION].v0
		= (*((*p_param)[level].values))[IMODULATION].v1
		= DEFTMODULATION;
	
	(*((*p_param)[level].values))[IVOLUME].index = IVOLUME;
	(*((*p_param)[level].values))[IPITCHBEND].index = IPITCHBEND;
	(*((*p_param)[level].values))[IPANORAMIC].index = IPANORAMIC;
	(*((*p_param)[level].values))[IPRESSURE].index = IPRESSURE;
	(*((*p_param)[level].values))[IMODULATION].index = IMODULATION;
	(*p_param)[level].number = IPANORAMIC + 1;
	}
	
if(paramnameindex < 0) return(-1);

maxnumber = MyGetHandleSize((Handle)(*p_param)[level].values) / sizeof(ContParameterSpecs);
for(i=0; i < maxnumber; i++) {
	if((*((*p_param)[level].values))[i].index == -1) {
		(*((*p_param)[level].values))[i].index = paramnameindex;
		break;
		}
	if((*((*p_param)[level].values))[i].index == paramnameindex) break;
	}
if(i >= maxnumber) {
	h = (Handle) ((*p_param)[level].values);
	MySetHandleSize(&h,(Size)(i+2) * sizeof(ContParameterSpecs));
	/* We give space for 2 more parameters to save time */
	(*p_param)[level].values = (ContParameterSpecs**) h;
	maxnumber = MyGetHandleSize(h) / sizeof(ContParameterSpecs);
	for(j=i; j < maxnumber; j++) {
		(*((*p_param)[level].values))[j].index = -1;
		(*((*p_param)[level].values))[j].mode = FIXED;
		(*((*p_param)[level].values))[j].increment = 0.;
		(*((*p_param)[level].values))[j].maxbeats = 1;
		(*((*p_param)[level].values))[j].active = FALSE;
		(*((*p_param)[level].values))[j].known = FALSE;
	// (*(*p_CsInstrument)[j].paramlist)[ip].defaultvalue ???
		(*((*p_param)[level].values))[j].start
			= (*((*p_param)[level].values))[j].v0
			= (*((*p_param)[level].values))[j].v1
			= 0.;
		(*((*p_param)[level].values))[j].channel = 0;
		(*((*p_param)[level].values))[j].scale = DefaultScale;
		(*((*p_param)[level].values))[j].blockkey = BlockScaleOnKey;
		(*((*p_param)[level].values))[j].imax = 0;
		(*((*p_param)[level].values))[j].ibeats = 0;
		(*((*p_param)[level].values))[j].point = NULL;
		}
	(*((*p_param)[level].values))[i].index = paramnameindex;
	}
if(i >= (*p_param)[level].number) (*p_param)[level].number = i + 1;
return(i);
}


int UpdateParameter(int i,ContParameters **p_contparameters,int level,long duration)
{
int imax;
double start;
unsigned long ibeats,maxbeats;

if(level >= Maxlevel) {
	if(Beta) Println(wTrace,"=> Err. UpdateParameter(). level >= Maxlevel");
	return(OK);
	}
if((*p_contparameters)[level].values == NULL) {
	if(Beta) Println(wTrace,"=> Err. UpdateParameter(). (*p_contparameters)[level].values == NULL");
	return(OK);
	}
if(i < 0) {
	if(Beta) Println(wTrace,"=> Err. UpdateParameter(). i < 0");
	return(OK);
	}
if(Beta) {
	imax = MyGetHandleSize((Handle)((*p_contparameters)[level].values)) / sizeof(ContParameterSpecs);
	if(i < 0 || i >= imax) {
		Println(wTrace,"=> Err. UpdateParameter(). i < 0 || i >= imax");
		return(ABORT);
		}
	}
start = (*((*p_contparameters)[level].values))[i].start;

// BPPrintMessage(odError,"contparameter #%d level = %d active = %d start = %.3f\n",i,level,(*((*p_contparameters)[level].values))[i].active,(*((*p_contparameters)[level].values))[i].start);

if((*((*p_contparameters)[level].values))[i].active) {
	ibeats = (*((*p_contparameters)[level].values))[i].ibeats;
	maxbeats = (*((*p_contparameters)[level].values))[i].maxbeats;
	if((*((*p_contparameters)[level].values))[i].point != NULL
			&& (ibeats + duration) > maxbeats) {
		if(Beta) Println(wTrace,"=> Err. UpdateParameter(). ibeats + duration > maxbeats");
		(*((*p_contparameters)[level].values))[i].active = FALSE;
		return(OK);
		}
	(*((*p_contparameters)[level].values))[i].v0
		= start + (((*((*p_contparameters)[level].values))[i].increment * ibeats) / maxbeats);
	(*((*p_contparameters)[level].values))[i].v1
		= start
		+ (((*((*p_contparameters)[level].values))[i].increment * (ibeats + duration)) / maxbeats);
	//  If the variation is tabulated this v1 may be wrong when (ibeats + duration) > maxbeats, but it will later
	// be set properly by SetObjectParam() using the last value in the table...
	}
else
	(*((*p_contparameters)[level].values))[i].v0
		= (*((*p_contparameters)[level].values))[i].v1 = start;
(*((*p_contparameters)[level].values))[i].scale = DefaultScale;
(*((*p_contparameters)[level].values))[i].blockkey = BlockScaleOnKey;
return(OK);
}


int IncrementParameter(int i,ContParameters **p_contparameters,int level,double addbeats)
{
int imax;
unsigned long ibeats;

if(Beta) {
	if((*p_contparameters)[level].values == NULL) {
		Println(wTrace,"=> Err. IncrementParameter(). (*p_contparameters)[level].values == NULL");
		return(ABORT);
		}
	imax = MyGetHandleSize((Handle)((*p_contparameters)[level].values)) / sizeof(ContParameterSpecs);
	if(i < 0 || i >= imax) {
		Println(wTrace,"=> Err. IncrementParameter(). i < 0 || i >= imax");
		return(ABORT);
		}
	}
if(level >= Maxlevel) return(ABORT);

if(!(*((*p_contparameters)[level].values))[i].active) return(OK);

ibeats = (*((*p_contparameters)[level].values))[i].ibeats + addbeats;
if(ibeats >= (*((*p_contparameters)[level].values))[i].maxbeats) {
	(*((*p_contparameters)[level].values))[i].active = FALSE;
	(*((*p_contparameters)[level].values))[i].v1 = (*((*p_contparameters)[level].values))[i].v0;
	(*((*p_contparameters)[level].values))[i].increment = 0.;
	}
else (*((*p_contparameters)[level].values))[i].ibeats = ibeats;
return(OK);
}


int MakeNewLineInPhaseTable(int nseq,int *p_nmax,double **p_im,double maxseq,
	unsigned long **p_maxcol)
{
long **ptr;
unsigned long iseq,iplot;
char overstrike;
int nseqplot;

overstrike = FALSE; // Fixed by BB 2021-01-31
if(nseq >= Maxconc) {
	BPPrintMessage(odError,"=> Error MakeNewLineInPhaseTable() nseq >= Maxconc\n");
	BPPrintMessage(odInfo,"nseq = %ld, nmax = %d, maxseq = %.0f, Maxconc = %ld\n",(long)nseq,*p_nmax,maxseq,(long)Maxconc);
//	TellSkipped();
	return(ABORT);
	}
ptr = (long**) GiveSpace((Size)(maxseq+1.) * sizeof(long));
if(((*p_Seq)[nseq]=ptr) == NULL) {
	BPPrintMessage(odError,"=> Memory size error MakeNewLineInPhaseTable()\n");
	return(ABORT);
	}

(*p_im)[nseq] = Round(1. - Kpress);
(*p_maxcol)[nseq] = Class((*p_im)[nseq]);

if((*p_nmax) < nseq) (*p_nmax) = nseq;

(*((*p_Seq)[nseq]))[0] = 0;
// BPPrintMessage(odError,"MakeNewLineInPhaseTable nseq = %ld, nmax = %ld (*p_im)[nseq] = %.0f (*p_maxcol)[nseq] = %ld\n",(long)nseq,(long)(*p_nmax),(double)(*p_im)[nseq],(long)(*p_maxcol)[nseq]);
for(iseq=ZERO; iseq <= maxseq; iseq++)
	Plot(INTIME,&nseqplot,&iplot,&overstrike,TRUE,p_nmax,p_maxcol,p_im,p_Seq,&nseq,maxseq,iseq,1); // Confirmed by BB 2021-03-20
//	Plot(INTIME,&nseqplot,&iplot,&overstrike,TRUE,p_nmax,p_maxcol,p_im,p_Seq,&nseq,maxseq,iseq,0);
return(OK);
}


int CopyContinuousParameters(ContParameters **p_a,int na,ContParameters **p_b,int nb)
{
// Makes a copy of the record except 'imax' and 'point', the pointer to a table

int i,j;

if(na >= Maxlevel) return(OK);

if((*p_a)[na].values == NULL) {
	if(Beta) Alert1("=> Err. CopyContinuousParameters(). (*p_a)[na].values) == NULL");
	return(ABORT);
	}
for(i=0; i < (*p_a)[na].number; i++) {
	j = FindParameterIndex(p_b,nb,(*((*p_a)[na].values))[i].index);
	if(j < 0) return(OK);
	/* Creates storage if needed */
	
	(*((*p_b)[nb].values))[i].index = (*((*p_a)[na].values))[i].index;
	(*((*p_b)[nb].values))[i].channel = (*((*p_a)[na].values))[i].channel;
	(*((*p_b)[nb].values))[i].scale = (*((*p_a)[na].values))[i].scale;
	(*((*p_b)[nb].values))[i].blockkey = (*((*p_a)[na].values))[i].blockkey;
	(*((*p_b)[nb].values))[i].mode = (*((*p_a)[na].values))[i].mode;
	(*((*p_b)[nb].values))[i].active = (*((*p_a)[na].values))[i].active;
	(*((*p_b)[nb].values))[i].known = (*((*p_a)[na].values))[i].known;
	(*((*p_b)[nb].values))[i].control = (*((*p_a)[na].values))[i].control;
	(*((*p_b)[nb].values))[i].start = (*((*p_a)[na].values))[i].start;
	(*((*p_b)[nb].values))[i].increment = (*((*p_a)[na].values))[i].increment;
	(*((*p_b)[nb].values))[i].ibeats = (*((*p_a)[na].values))[i].ibeats;
	(*((*p_b)[nb].values))[i].maxbeats = (*((*p_a)[na].values))[i].maxbeats;
	(*((*p_b)[nb].values))[i].v0 = (*((*p_a)[na].values))[i].v0;
	(*((*p_b)[nb].values))[i].v1 = (*((*p_a)[na].values))[i].v1;
	}
(*p_b)[nb].number = (*p_a)[na].number;

return(OK);
}


int PutZeros(char toofast,double **p_im,unsigned long **p_maxcol,int nseq,double maxseq,
	double numberzeros,int *p_nmax,int kobj)
	// kobj only for checking. Value is wrong if object is a silence or empty
{
unsigned long ip,iplot,ipnew,k,kmax;
char overstrike;
double i,i2,j;
int nseqplot,oldnseq,new_method;

overstrike = FALSE; // Fixed by BB 2021-01-31
if(nseq >= Maxconc) {
	if(Beta) Println(wTrace,"=> Err. PutZeros(). nseq >= Maxconc");
	TellSkipped();
	return(MISSED);
	}
if(numberzeros > 0.) {
	i = i2 = (*p_im)[nseq];
	ip = Class(i);
	kmax = Class(numberzeros);
//	if(kobj == 4) BPPrintMessage(odInfo,"+++ kobj = %d nseq = %d i = %.2f Class(i) = %ld kmax = %ld toofast = %d\n",kobj,nseq,i,(long)ip,kmax,(int)toofast); 
	
	new_method = 1;
	if(new_method) { // Created by BB 2021-03-22
		for(k = 1; k <= kmax; k++) {
			ipnew = ip + k;
			if(toofast) continue;
			if(ipnew > maxseq) {
				if(ipnew < (1.005 * (maxseq + 1.))) {
					BPPrintMessage(odInfo,"Adjusted ipnew (%ld) to %.0f (object #%d)\n",ipnew,maxseq,kobj);
					ipnew = maxseq;
					}
				else {
					BPPrintMessage(odError,"=> Err. PutZeros() new method. nseq = %ld ipnew = %u  maxseq = %.1f\n",(long)nseq,(long)ipnew,maxseq);
					return(ABORT);
					}
				}
			oldnseq = nseq;
			Plot(INTIME,&nseqplot,&iplot,&overstrike,FALSE,p_nmax,p_maxcol,p_im,p_Seq,
				&nseq,maxseq,ipnew,0);
			nseq = oldnseq;
			}
	/*	i += (1 + numberzeros - Kpress);
		(*p_im)[nseq] = i;
		(*p_maxcol)[nseq] = Class((*p_im)[nseq]);
		for(j=Kpress; j <= numberzeros; j += Kpress) { i2 += Kpress; }
		if(kobj < 20 && kobj > 0) BPPrintMessage(odInfo,"kobj = %d delta_im1 = %.3f\n",kobj,(*p_im)[nseq]-(i2 + 1. + numberzeros - j)); */
		for(j=Kpress; j <= numberzeros; j += Kpress) { i += Kpress; }
	 	(*p_im)[nseq] = i + 1. + numberzeros - j;
		(*p_maxcol)[nseq] = Class((*p_im)[nseq]);
		}
	else { // Creates rounding errors
		for(j=Kpress; j <= numberzeros; j += Kpress) {
			i += Kpress;
			ip = Class(i);
			if(toofast) continue;
			if(ip > maxseq) {
				if(ip < (1.005 * (maxseq + 1.))) {
					ip = maxseq;
					}
				else {
					BPPrintMessage(odError,"=> Err. PutZeros() old method. nseq=%ld (*p_im)[nseq]=%.1f ip=%u  maxseq=%.1f\n",(long)nseq,i,ip,maxseq);
					return(ABORT);
					}
				}
			oldnseq = nseq;
			Plot(INTIME,&nseqplot,&iplot,&overstrike,FALSE,p_nmax,p_maxcol,p_im,p_Seq,
				&nseq,maxseq,ip,0);
			nseq = oldnseq;
			}
	 	(*p_im)[nseq] = i + 1. + numberzeros - j;
		(*p_maxcol)[nseq] = Class((*p_im)[nseq]);
		}
	/* This final value, when incremented with Kpress, is the position of the next streak to */
	/* ... place objects on. */
	}
return(OK);
}


int ShowProgress(int k)
{
if(!ShowMessages || ((k % 50) != 0)) return(OK);
PleaseWait();
sprintf(Message,"Time-objects created: %ld.  Estimated: %ld",(long)k,(long)(Maxevent-1));
// FlashInfo(Message);
return(OK);
}


int TellSkipped(void)
{
if(!ToldSkipped) {
	ToldSkipped = TRUE;
//	BPPrintMessage(odError,"=> Some objects have been skipped. Quantization should be reduced to avoid it.\n");
	}
return(OK);
}