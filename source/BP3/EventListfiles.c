/* EventListfiles.c (BP3) */

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


int MakeEventListFile(OutFileInfo* finfo) {
	int result;
	FILE *fout;
	result = OK;
	char thename[MAXNAME];
	if(EventListPtr != NULL) CloseEventListFile();
	fout = OpenOutputFile(finfo,"wb");
	if(!fout) {
		BPPrintMessage(0,odError, "=> Could not create event list file %s\n", finfo->name);
		return MISSED;
		}
	else EventListPtr = fout;
	BPPrintMessage(0,odInfo,"👉 An event list file has been created\n");
	WriteToEventListFile("event,item,k,id proto,label,start time,end time,trunc beg,trunc end,dilation ratio beta,dilation ratio alpha,cycles,cyclic after,force integer number of cycles,articul,preroll,postroll,transpos,transpose first,expand value,expand key,volume start,volume end,volume channel,volume mode (2 = continuous),modulation start,modulation end,modulation channel,modulation mode,panoramic start,panoramic end,panoramic channel,panoramic mode,pressure start,pressure end,pressure channel, pressure mode,pitchbend start,pitchbend end,pitchbend channel,pitchbend mode");
	return result;
	}

int CloseEventListFile(void) {
	if(EventListPtr == NULL) return(OK);
    fflush(EventListPtr);
	if(gOptions.outputFiles[ofiEventListfile].isOpen) {
		fflush(gOptions.outputFiles[ofiEventListfile].fout);
		CloseOutputFile(&(gOptions.outputFiles[ofiEventListfile]));
		my_sprintf(Message,"Closing event list file %s",gOptions.outputFiles[ofiEventListfile].name);
		ShowMessage(TRUE,wMessage,Message);
		}
	EventListPtr =  NULL;
	return(OK);
	}

int WriteToEventListFile(const char *line) {
    if (EventListPtr == NULL || line == NULL) {
        return MISSED;
    	}
    if (fputs(line, EventListPtr) == EOF || fputs("\r\n", EventListPtr) == EOF) {
        BPPrintMessage(0,odError,"=> Could not write to event list file\n");
        return MISSED;
    	}
    fflush(EventListPtr);
    return OK;
	}

int AddEventToList(int k) {
	long starttime,endtime,shift;
	int j,id_proto,articul,trans,cyclic_after;
	short xpandval,xpandkey;
	char line[MAXLIN],label[MAXNAME];
	double alpha,dilationratio,preroll,postroll,expand;
	int transposefirst,forceintegercycles;

    if(EventListPtr == NULL) {
        BPPrintMessage(0,odError, "=> Could not add event to list\n");
        return MISSED;
    	}
	j = (*p_Instance)[k].object;
	if(j == -1) return(OK);
	if(j == 0) return(OK);
	if(j < 16384 && j >= Jbol) return(OK); // Time-pattern
	cyclic_after = forceintegercycles = 0;
	int volumestart = VolumeStart(k);
	int volumeend = VolumeEnd(k);
	int volumechannel = VolumeChannel(k);
	int volumemode = VolumeMode(k);

	int modulationstart = ModulationStart(k);
	int modulationend = ModulationEnd(k);
	int modulationchannel = ModulationChannel(k);
	int modulationmode = ModulationMode(k);

	int panoramicstart = PanoramicStart(k);
	int panoramicend = PanoramicEnd(k);
	int panoramicchannel = PanoramicChannel(k);
	int panoramicmode = PanoramicMode(k);

	int pressurestart = PressureStart(k);
	int pressureend = PressureEnd(k);
	int pressurechannel = PressureChannel(k);
	int pressuremode = PressureMode(k);

	int pitchbendstart = PitchbendStart(k);
	int pitchbendend = PitchbendEnd(k);
	int pitchbendchannel = PitchbendChannel(k);
	int pitchbendmode = PitchbendMode(k);

	if(j < 16384) {
		if(j < 0) {
			j = -j;
			my_sprintf(label,"<<%s>>",*((*p_Bol)[j]));
			}
		else if(j == 1) my_sprintf(label,"-");
		else {
			my_sprintf(label,"%s",*((*p_Bol)[j]));
			if((*p_Instance)[k].ncycles > 1) {
				if((*p_CyclicMode)[j] == ABSOLU) cyclic_after = (int)(*p_CyclicAfter)[j];
				else
					cyclic_after = (int) ((double)(*p_CyclicAfter)[j] * (*p_Dur)[j]) / 100.;
				forceintegercycles = (*p_ForceIntegerCycles)[j];
				}
			}
		id_proto = j;
		dilationratio = (*p_Instance)[k].dilationratio;
		if((*p_PreRollMode)[j] == ABSOLU) preroll = (*p_PreRoll)[j];
		else preroll = dilationratio * (*p_PreRoll)[j];
		if((*p_PostRollMode)[j] == ABSOLU) postroll = (*p_PostRoll)[j];
		else postroll = dilationratio * (*p_PostRoll)[j];
		}
	else {
		id_proto = 0;
		preroll = postroll = 0.;
		dilationratio = (*p_Instance)[k].dilationratio;
		my_sprintf(label,"%s",*((*(p_NoteName[NoteConvention]))[j-16384]));
		}
	if(p_Articul != NULL) articul =	(*p_Articul)[k];
	else articul = 0;
	trans = (*p_Instance)[k].transposition / 100;
	transposefirst = (int) (*p_Instance)[k].transposefirst;
	xpandval = (*p_Instance)[k].xpandval;
	if(xpandval > 0) {
		expand = (*p_NumberConstant)[xpandval];
		xpandkey = (*p_Instance)[k].xpandkey;
		}
	else {
		expand = 0;
		xpandkey = -1;
		}
	shift = PianorollShift - MIDIsetUpTime;
	starttime = (*p_Instance)[k].starttime + shift;
	endtime = (*p_Instance)[k].endtime  + shift;
	EventNumber++;
	my_sprintf(line,"#%ld,%ld,(%d),%d,%s,%ld,%ld,%ld,%ld,%.4f,%.4f,%d,%d,%d,%d,%.4f,%.4f,%d,%d,%.4f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",EventNumber,ItemNumber,k,id_proto,label,starttime,endtime,(*p_Instance)[k].truncbeg,(*p_Instance)[k].truncend,dilationratio,(*p_Instance)[k].alpha,(*p_Instance)[k].ncycles,cyclic_after,forceintegercycles,articul,preroll,postroll,trans,transposefirst,expand,xpandkey,volumestart,volumeend,volumechannel,volumemode,modulationstart,modulationend,modulationchannel,modulationmode,panoramicstart,panoramicend,panoramicchannel,panoramicmode,pressurestart,pressureend,pressurechannel,pressuremode,pitchbendstart,pitchbendend,pitchbendchannel,pitchbendmode);
	if(WriteToEventListFile(line) != OK) return MISSED;
	return(OK);
	}

	/*
		Milliseconds starttime,endtime;
	short object,nseq,ncycles,seed;
	Milliseconds truncbeg,truncend;
	char velocity,channel;
	int scale,blockkey;
	int capture;
	short rndvel,velcontrol,randomtime;
	short xpandkey,xpandval;
	short transposition,instrument,part;
	double alpha,dilationratio;
	Parameters contparameters;
	KeyNumberMap map0,map1;
	char mapmode,transposefirst; */
