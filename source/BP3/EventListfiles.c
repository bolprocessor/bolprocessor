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
	WriteToEventListFile("event,item,k,id proto,label,start time,end time,MIDI channel,Csound instrument,random time,velocity,random velocity,trunc beg,trunc end,dilation ratio beta (actual),dilation ratio alpha (declared),cyclic,cyclic after (ms or percent),force integer number of cycles,articul,preroll (ms),postroll (ms),transposition,transpose first,expand value,expand key,keymap mode,keymap0 p1,keymap0 q1,keymap0 p2,keymap0 q2,keymap1 p1,keymap1 q1,keymap1 p2,keymap1 q2,volume start,volume end,volume channel,volume mode,modulation start,modulation end,modulation channel,modulation mode,panoramic start,panoramic end,panoramic channel,panoramic mode,pressure start,pressure end,pressure channel, pressure mode,pitchbend start,pitchbend end,pitchbend channel,pitchbend mode,tonal scale,block key");
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
	int j,id_proto,articul,trans,cyclic_after,blockkey,keymap0_p1,keymap0_q1,keymap0_p2,keymap0_q2,keymap1_p1,keymap1_q1,keymap1_p2,keymap1_q2,cyclic,sound_object;
	short xpandval,xpandkey;
	char line[MAXLIN],label[MAXNAME],scalename[MAXNAME],keymapmode[MAXNAME],channel_txt[MAXNAME],instrument_txt[MAXNAME],pitchbendmode_txt[15],pressuremode_txt[15],panoramicmode_txt[15],volumemode_txt[15],modulationmode_txt[15],pitchbendstart_txt[15],pitchbendend_txt[15],pitchbendchannel_txt[15],xpandkey_txt[15],blockkey_txt[10];
	double beta,preroll,postroll,expand;
	int transposefirst,forceintegercycles,i_scale;
	int localchan,instrument;

    if(EventListPtr == NULL) {
        BPPrintMessage(0,odError,"=> Could not add event to list\n");
        return MISSED;
    	}
	j = (*p_Instance)[k].object;
	if(j == -1) return(OK);
	if(j == 0) return(OK);
	if(j < 16384 && j >= Jbol) return(OK); // Time-pattern

	// BPPrintMessage(0,odError,"k = %d, j = %d\n",k,j);
	
	keymap0_p1 = keymap0_q1 = keymap0_p2 = keymap0_q2 = 0;
	keymap1_p1 = keymap1_q1 = keymap1_p2 = keymap1_q2 = 0;
	cyclic_after = forceintegercycles = cyclic = 0;
	localchan = (*p_Instance)[k].channel;
	instrument = (*p_Instance)[k].instrument;
	int volumestart = VolumeStart(k);
	int volumeend = VolumeEnd(k);
	int volumechannel = VolumeChannel(k);
	int volumemode = VolumeMode(k);
	switch(volumemode) {
		case FIXMAPMODE:
			strcpy(volumemode_txt,"FIXED");
		break;
		case STEPWISE:
			strcpy(volumemode_txt,"STEPWISE");
		break;
		case CONTINUOUS:
			strcpy(volumemode_txt,"CONTINUOUS");
		break;
		default:
			strcpy(volumemode_txt,"");
		break;
		}

	int modulationstart = ModulationStart(k);
	int modulationend = ModulationEnd(k);
	int modulationchannel = ModulationChannel(k);
	int modulationmode = ModulationMode(k);
	switch(modulationmode) {
		case FIXMAPMODE:
			strcpy(modulationmode_txt,"FIXED");
		break;
		case STEPWISE:
			strcpy(modulationmode_txt,"STEPWISE");
		break;
		case CONTINUOUS:
			strcpy(modulationmode_txt,"CONTINUOUS");
		break;
		default:
			strcpy(modulationmode_txt,"");
		break;
		}
	int panoramicstart = PanoramicStart(k);
	int panoramicend = PanoramicEnd(k);
	int panoramicchannel = PanoramicChannel(k);
	int panoramicmode = PanoramicMode(k);
	switch(panoramicmode) {
		case FIXMAPMODE:
			strcpy(panoramicmode_txt,"FIXED");
		break;
		case STEPWISE:
			strcpy(panoramicmode_txt,"STEPWISE");
		break;
		case CONTINUOUS:
			strcpy(panoramicmode_txt,"CONTINUOUS");
		break;
		default:
			strcpy(panoramicmode_txt,"");
		break;
		}
	int pressurestart = PressureStart(k);
	int pressureend = PressureEnd(k);
	int pressurechannel = PressureChannel(k);
	int pressuremode = PressureMode(k);
	switch(pressuremode) {
		case FIXMAPMODE:
			strcpy(pressuremode_txt,"FIXED");
		break;
		case STEPWISE:
			strcpy(pressuremode_txt,"STEPWISE");
		break;
		case CONTINUOUS:
			strcpy(pressuremode_txt,"CONTINUOUS");
		break;
		default:
			strcpy(pressuremode_txt,"");
		break;
		}
	int pitchbendstart = PitchbendStart(k);
	int pitchbendend = PitchbendEnd(k);
	int pitchbendchannel = PitchbendChannel(k);
	my_sprintf(pitchbendchannel_txt,"%d",pitchbendchannel);
	int pitchbendmode = PitchbendMode(k);
	my_sprintf(pitchbendstart_txt,"%d",pitchbendstart);
	my_sprintf(pitchbendend_txt,"%d",pitchbendend);
	switch(pitchbendmode) {
		case FIXMAPMODE:
			strcpy(pitchbendmode_txt,"FIXED");
		break;
		case STEPWISE:
			strcpy(pitchbendmode_txt,"STEPWISE");
		break;
		case CONTINUOUS:
			strcpy(pitchbendmode_txt,"CONTINUOUS");
		break;
		default:
			strcpy(pitchbendmode_txt,"");
		break;
		}

	int scale = (*p_Instance)[k].scale;
	if(scale > 0) my_sprintf(scalename,"%s",*((*p_StringConstant)[scale]));
	else strcpy(scalename,"");
	// BPPrintMessage(0,odInfo,"@@@ Scale = %s\n",*((*p_StringConstant)[scale]));
	if(strlen(scalename) > 0) blockkey = (*p_Instance)[k].blockkey;
	else blockkey = -1;
	if(blockkey <  0) strcpy(blockkey_txt,"");
	else my_sprintf(blockkey_txt,"%d",blockkey);
	preroll = postroll = id_proto = 0;
	beta = (*p_Instance)[k].beta;
	preroll = postroll = 0.;
	sound_object = FALSE;
	if(j < 16384) {
		if(j < 0) {
			j = -j;
			if(j >= 16384) {
				if(MIDImicrotonality) {
					scale = (*p_Instance)[k].scale;
					if(scale < 0) i_scale = -1;
					else if(scale == 0) i_scale = 0;
					else i_scale = FindScale(scale);
					}
				else i_scale = -1;
				PrintThisNote(i_scale,j-16384,0,-1,line);
				my_sprintf(label,"<<%s>>",line);
				}
			else if(j < Jbol) {
				my_sprintf(label,"<<%s>>",*((*p_Bol)[j]));
				sound_object = TRUE;
				}
			else strcpy(label,"???");
			}
		else if(j == 1) my_sprintf(label,"-");
		else if(j < Jbol) {
			my_sprintf(label,"%s",*((*p_Bol)[j]));
			sound_object = TRUE;
			}
		else strcpy(label,"???");
		}
	else {
		if(MIDImicrotonality) {
			scale = (*p_Instance)[k].scale;
			if(scale < 0) i_scale = -1;
			else if(scale == 0) i_scale = 0;
			else i_scale = FindScale(scale);
			}
		else i_scale = -1;
		PrintThisNote(i_scale,j-16384,0,-1,label);
	//	BPPrintMessage(0,odInfo,"@@@ scale = %d, i_scale = %d, label = %s\n",scale,i_scale,label);
		}
	if(sound_object) {
		strcpy(pitchbendmode_txt,"");
		strcpy(pitchbendstart_txt,"");
		strcpy(pitchbendend_txt,"");
		strcpy(pitchbendchannel_txt,"");
		if((*p_CyclicMode)[j] != IRRELEVANT) {
			cyclic = 1;
			forceintegercycles = (*p_ForceIntegerCycles)[j];
			if((*p_CyclicMode)[j] == FIXVALUE) cyclic_after = (int)(*p_CyclicAfter)[j];
			if((*p_CyclicMode)[j] == PERCENT) cyclic_after = (int) ((double)(*p_CyclicAfter)[j] * (*p_Dur)[j]) / 100.;
			}
		id_proto = j;
		if((*p_PreRollMode)[j] == FIXVALUE) preroll = (*p_PreRoll)[j];
		else preroll = beta * (*p_PreRoll)[j];
		if((*p_PostRollMode)[j] == FIXVALUE) postroll = (*p_PostRoll)[j];
		else postroll = beta * (*p_PostRoll)[j];
		if(beta == 0.) preroll = postroll = 0.;
		if((*p_DefaultChannel)[j] != 0) localchan = (*p_DefaultChannel)[j];
		if((*p_CsoundInstrumentMode)[j] != 0) instrument = (*p_CsoundInstrumentMode)[j];
		}
	if(p_Articul != NULL) articul =	(*p_Articul)[k];
	else articul = 0;
	trans = (*p_Instance)[k].transposition / 100;
	transposefirst = (int) (*p_Instance)[k].transposefirst;
	xpandval = (*p_Instance)[k].xpandval;
	if(xpandval > 0) {
		expand = (*p_NumberConstant)[xpandval];
		xpandkey = (*p_Instance)[k].xpandkey;
		my_sprintf(xpandkey_txt,"%d",xpandkey);
		}
	else {
		expand = 0;
		strcpy(xpandkey_txt,"");
		}
	if(localchan < 0) strcpy(channel_txt,"LOCAL_CH"); 
	else if(localchan == 0) strcpy(channel_txt,"GLOBAL_CH");
	else my_sprintf(channel_txt,"%d",localchan);

	if(instrument < 0) strcpy(instrument_txt,"LOCAL_CS"); 
	else if(instrument == 0) strcpy(instrument_txt,"GLOBAL_CS");
	else my_sprintf(instrument_txt,"%d",instrument);

	int velocity = ByteToInt((*p_Instance)[k].velocity);
	int rndvel = (*p_Instance)[k].rndvel;
	int randomtime = (*p_Instance)[k].randomtime;
	shift = PianorollShift - MIDIsetUpTime;
	starttime = (*p_Instance)[k].starttime + shift;
	endtime = (*p_Instance)[k].endtime  + shift;
	char mapmode = (*p_Instance)[k].mapmode;
	if(mapmode == CONTINUOUS) strcpy(keymapmode,"CONT");
	else if(mapmode == STEPWISE) strcpy(keymapmode,"STEP");
	else if(mapmode == FIXMAPMODE) strcpy(keymapmode,"FIXMAPMODE");
	else strcpy(keymapmode,"OFF");
	keymap0_p1 = (*p_Instance)[k].map0.p1;
	keymap0_q1 = (*p_Instance)[k].map0.q1;
	keymap0_p2 = (*p_Instance)[k].map0.p2;
	keymap0_q2 = (*p_Instance)[k].map0.q2;
	keymap1_p1 = (*p_Instance)[k].map1.p1;
	keymap1_q1 = (*p_Instance)[k].map1.q1;
	keymap1_p2 = (*p_Instance)[k].map1.p2;
	keymap1_q2 = (*p_Instance)[k].map1.q2;
	if(j == 1 && starttime == endtime && strcmp(keymapmode,"OFF") == 0) return(OK);

	EventNumber++;
	my_sprintf(line,"#%ld,%ld,(%d),%d,%s,%ld,%ld,%s,%s,%d,%d,%d,%ld,%ld,%.4f,%.4f,%d,%d,%d,%d,%.4f,%.4f,%d,%d,%.4f,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,%d,%d,%d,%s,%d,%d,%d,%s,%d,%d,%d,%s,%s,%s,%s,%s,%s,%s",EventNumber,ItemNumber,k,id_proto,label,starttime,endtime,channel_txt,instrument_txt,randomtime,velocity,rndvel,(*p_Instance)[k].truncbeg,(*p_Instance)[k].truncend,beta,(*p_Instance)[k].alpha,cyclic,cyclic_after,forceintegercycles,articul,preroll,postroll,trans,transposefirst,expand,xpandkey_txt,keymapmode,keymap0_p1,keymap0_q1,keymap0_p2,keymap0_q2,keymap1_p1,keymap1_q1,keymap1_p2,keymap1_q2,volumestart,volumeend,volumechannel,volumemode_txt,modulationstart,modulationend,modulationchannel,modulationmode_txt,panoramicstart,panoramicend,panoramicchannel,panoramicmode_txt,pressurestart,pressureend,pressurechannel,pressuremode_txt,pitchbendstart_txt,pitchbendend_txt,pitchbendchannel_txt,pitchbendmode_txt,scalename,blockkey_txt);
	if(WriteToEventListFile(line) != OK) return MISSED;
	return(OK);
	}