/* Csound.c (BP3) */ 

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

int trace_csound = 0;

#if BP_CARBON_GUI_FORGET_THIS

SetCsoundInstrument(int j,int w)
/* If w > 0 this procedure also displays instrument specs to window w */
{
char line[MAXFIELDCONTENT];
int i,ip,channel;

if(j < 0 || j >= Jinstr) {
	BPPrintMessage(0,odError,"=> Err. SetCsoundInstrument(). Incorrect index");
	return(MISSED);
	}
if((*p_CsInstrumentIndex)[j] > -1) {
	my_sprintf(line,"%ld",(long)(*p_CsInstrumentIndex)[j]);
	SetField(NULL,wCsoundResources,fCsoundInstrumentIndex,line);
	}
else {
	SetField(NULL,wCsoundResources,fCsoundInstrumentIndex,"[?]");
	SelectField(NULL,wCsoundResources,fCsoundInstrumentIndex,TRUE);
	}

if((*((*pp_CsInstrumentName)[j]))[0] != '\0')
	MystrcpyHandleToString(MAXFIELDCONTENT,0,line,(*pp_CsInstrumentName)[j]);
else strcpy(line,"[New instrument]");
SetField(NULL,wCsoundResources,fCsoundInstrumentName,line);

if(w > 0) {
	if((*((*pp_CsInstrumentName)[j]))[0] == '\0') strcpy(line,"[no name]");
	if((*p_CsInstrumentIndex)[j] > -1)
		my_sprintf(Message,"--- Instrument #%ld = %s ---\n",(long)(*p_CsInstrumentIndex)[j],line);
	else
		my_sprintf(Message,"--- Instrument [no index] = %s ---\n",line);
	Println(w,Message);
	}
	
if((*pp_CsInstrumentComment)[j] != NULL && (*((*pp_CsInstrumentComment)[j]))[0] != '\0') {
	MystrcpyHandleToString(MAXFIELDCONTENT,0,line,(*pp_CsInstrumentComment)[j]);
	SetField(NULL,wCsoundResources,fCsoundInstrumentComment,line);
	if(w > 0) {
		my_sprintf(Message,"Comment: %s",line);
		Println(w,Message);
		}
	}
else SetField(NULL,wCsoundResources,fCsoundInstrumentComment,"[Comment on this instrument]");
	
if(CsoundOrchestraName[0] != '\0')
	SetField(NULL,wCsoundResources,fCsoundOrchestra,CsoundOrchestraName);
else
	SetField(NULL,wCsoundResources,fCsoundOrchestra,"BP2test.orc");

if(w > 0) {
	if(CsoundOrchestraName[0] == '\0') my_sprintf(Message,"Csound orchestra file = [unspecified]");
	else my_sprintf(Message,"Csound orchestra file = %s",CsoundOrchestraName);
	Println(w,Message);
	}

for(channel=1; channel <= MAXCHAN; channel++) {
	if(WhichCsoundInstrument[channel] == (*p_CsInstrumentIndex)[j]
			&& WhichCsoundInstrument[channel] > 0) {
		my_sprintf(line,"%ld",(long)channel);
		SetField(NULL,wCsoundResources,fCsoundInstrumentChannel,line);
		if(w > 0) {
			my_sprintf(Message,"Assigned by default to MIDI channel %ld",(long)channel);
			Println(w,Message);
			}
		goto MORE;
		}
	}
SetField(NULL,wCsoundResources,fCsoundInstrumentChannel,"\0");

MORE:
if((*p_CsInstrument)[j].iargmax > -1)
	my_sprintf(line,"%ld",(long)(*p_CsInstrument)[j].iargmax);
else strcpy(line,"[?]");
SetField(NULL,wCsoundResources,fNumberParameters,line);

if(w > 0) {
	PleaseWait();
	my_sprintf(Message,"%s arguments defined for this instrument\n",line);
	Println(w,Message);
	}

if((*p_CsDilationRatioIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsDilationRatioIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fDilationRatioIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Dilation ratio argument = %s",line);
	Println(w,Message);
	}

if((*p_CsAttackVelocityIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsAttackVelocityIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fAttackVelocityIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Attack velocity argument = %s",line);
	Println(w,Message);
	}

if((*p_CsReleaseVelocityIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsReleaseVelocityIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fReleaseVelocityIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Release velocity argument = %s\n",line);
	Println(w,Message);
	}

if((*p_CsPitchIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsPitchIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fPitchIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Pitch argument = %s",line);
	Println(w,Message);
	}

switch((*p_CsPitchFormat)[j]) {
	case OPPC:
		SwitchOn(NULL,wCsoundResources,bOctavePitchClass);
		SwitchOff(NULL,wCsoundResources,bOctaveDecimal);
		SwitchOff(NULL,wCsoundResources,bHz);
		strcpy(line,"octave point pitch-class (OPPC)");
		break;
	case OPD:
		SwitchOff(NULL,wCsoundResources,bOctavePitchClass);
		SwitchOn(NULL,wCsoundResources,bOctaveDecimal);
		SwitchOff(NULL,wCsoundResources,bHz);
		strcpy(line,"octave point decimal (OPD)");
		break;
	case CPS:
		SwitchOff(NULL,wCsoundResources,bOctavePitchClass);
		SwitchOff(NULL,wCsoundResources,bOctaveDecimal);
		SwitchOn(NULL,wCsoundResources,bHz);
		strcpy(line,"cycles per second (CPS)");
		break;
	}
	
if(w > 0) {
	my_sprintf(Message,"Pitch format = %s\n",line);
	Println(w,Message);
	}

if((*p_CsPitchBendStartIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsPitchBendStartIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fPitchBendIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Pitchbend start argument = %s",line);
	Println(w,Message);
	}

if((*p_CsPitchBendEndIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsPitchBendEndIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fPitchBendEndIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Pitchbend end argument = %s",line);
	Println(w,Message);
	}

if((*p_CsInstrument)[j].pitchbendtable > 0) my_sprintf(line,"%ld",(long)(*p_CsInstrument)[j].pitchbendtable);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fPitchBendTable,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Pitchbend table argument = %s",line);
	Println(w,Message);
	}

my_sprintf(line,"0%ld",(long)(*p_CsInstrument)[j].pitchbendGEN);
SetField(NULL,wCsoundResources,fPitchBendGEN,line);

if(w > 0) {
	my_sprintf(Message,"Pitchbend table GEN%s",line);
	Println(w,Message);
	}

if((*p_CsInstrument)[j].pitchbendrange > -1.) {
	if((*p_CsInstrument)[j].pitchbendrange == (double)((long)(*p_CsInstrument)[j].pitchbendrange))
	my_sprintf(line,"%ld",(long)(*p_CsInstrument)[j].pitchbendrange);
	else WriteFloatToLine(line,(*p_CsInstrument)[j].pitchbendrange);
	}
else line[0] = '\0';
SetField(NULL,wCsoundResources,fPitchBendRange,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Pitchbend range = %s cents\n",line);
	Println(w,Message);
	}

if((*p_CsVolumeStartIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsVolumeStartIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fVolumeIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Volume start argument = %s",line);
	Println(w,Message);
	}
	
if((*p_CsVolumeEndIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsVolumeEndIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fVolumeEndIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Volume end argument = %s",line);
	Println(w,Message);
	}

if((*p_CsInstrument)[j].volumetable > 0) my_sprintf(line,"%ld",(long)(*p_CsInstrument)[j].volumetable);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fVolumeTable,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Volume table argument = %s",line);
	Println(w,Message);
	}

my_sprintf(line,"0%ld",(long)(*p_CsInstrument)[j].volumeGEN);
SetField(NULL,wCsoundResources,fVolumeGEN,line);

if(w > 0) {
	my_sprintf(Message,"Volume table GEN%s\n",line);
	Println(w,Message);
	}

if((*p_CsPressureStartIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsPressureStartIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fPressureIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Pressure start argument = %s",line);
	Println(w,Message);
	}

if((*p_CsPressureEndIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsPressureEndIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fPressureEndIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Pressure end argument = %s",line);
	Println(w,Message);
	}
	
if((*p_CsInstrument)[j].pressuretable > 0) my_sprintf(line,"%ld",(long)(*p_CsInstrument)[j].pressuretable);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fPressureTable,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Pressure table argument = %s",line);
	Println(w,Message);
	}
	
my_sprintf(line,"0%ld",(long)(*p_CsInstrument)[j].pressureGEN);
SetField(NULL,wCsoundResources,fPressureGEN,line);

if(w > 0) {
	my_sprintf(Message,"Pressure table GEN%s\n",line);
	Println(w,Message);
	}
	
if((*p_CsModulationStartIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsModulationStartIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fModulationIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Modulation start argument = %s",line);
	Println(w,Message);
	}

if((*p_CsModulationEndIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsModulationEndIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fModulationEndIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Modulation end argument = %s",line);
	Println(w,Message);
	}
	
if((*p_CsInstrument)[j].modulationtable > 0) my_sprintf(line,"%ld",(long)(*p_CsInstrument)[j].modulationtable);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fModulationTable,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Modulation table argument = %s",line);
	Println(w,Message);
	}
	
my_sprintf(line,"0%ld",(long)(*p_CsInstrument)[j].modulationGEN);
SetField(NULL,wCsoundResources,fModulationGEN,line);

if(w > 0) {
	my_sprintf(Message,"Modulation table GEN%s\n",line);
	Println(w,Message);
	}

if((*p_CsPanoramicStartIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsPanoramicStartIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fPanoramicIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Panoramic start argument = %s",line);
	Println(w,Message);
	}

if((*p_CsPanoramicEndIndex)[j] > -1) my_sprintf(line,"%ld",(long)(*p_CsPanoramicEndIndex)[j]);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fPanoramicEndIndex,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Panoramic end argument = %s",line);
	Println(w,Message);
	}
	
if((*p_CsInstrument)[j].panoramictable > 0) my_sprintf(line,"%ld",(long)(*p_CsInstrument)[j].panoramictable);
else line[0] = '\0';
SetField(NULL,wCsoundResources,fPanoramicTable,line);

if(w > 0) {
	if(line[0] == '\0') strcpy(line,"[unspecified]");
	my_sprintf(Message,"Panoramic table argument = %s",line);
	Println(w,Message);
	}
	
my_sprintf(line,"0%ld",(long)(*p_CsInstrument)[j].panoramicGEN);
SetField(NULL,wCsoundResources,fPanoramicGEN,line);

if(w > 0) {
	my_sprintf(Message,"Panoramic table GEN%s\n",line);
	Println(w,Message);
	}

if(w > 0) {
	PleaseWait();
	if((*p_CsInstrument)[j].rPitchBend.islogx && !(*p_CsInstrument)[j].rPitchBend.islogy)
		Println(w,"Pitchbend mapping (log to lin):");
	else {
		if(!(*p_CsInstrument)[j].rPitchBend.islogx && (*p_CsInstrument)[j].rPitchBend.islogy)
			Println(w,"Pitchbend mapping (lin to log):");
		else Println(w,"Pitchbend mapping (linear):");
		}
	for(i=0; i < 3; i++) {
		if((*(p_CsPitchBend[i]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsPitchBend[i]))[j]);
		else strcpy(line,"[?]");
		Print(w,line);
		if((*(p_CsPitchBend[i+3]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsPitchBend[i+3]))[j]);
		else strcpy(line,"[?]");
		my_sprintf(Message," <-> %s",line);
		Println(w,Message);
		}
	
	if((*p_CsInstrument)[j].rVolume.islogx && !(*p_CsInstrument)[j].rVolume.islogy)
		Println(w,"\nVolume mapping (log to lin):");
	else {
		if(!(*p_CsInstrument)[j].rVolume.islogx && (*p_CsInstrument)[j].rVolume.islogy)
			Println(w,"\nVolume mapping (lin to log):");
		else Println(w,"\nVolume mapping (linear):");
		}
	for(i=0; i < 3; i++) {
		if((*(p_CsVolume[i]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsVolume[i]))[j]);
		else strcpy(line,"[?]");
		Print(w,line);
		if((*(p_CsVolume[i+3]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsVolume[i+3]))[j]);
		else strcpy(line,"[?]");
		my_sprintf(Message," <-> %s",line);
		Println(w,Message);
		}
	
	if((*p_CsInstrument)[j].rPressure.islogx && !(*p_CsInstrument)[j].rPressure.islogy)
		Println(w,"\nPressure mapping (log to lin):");
	else {
		if(!(*p_CsInstrument)[j].rPressure.islogx && (*p_CsInstrument)[j].rPressure.islogy)
			Println(w,"\nPressure mapping (lin to log):");
		else Println(w,"\nPressure mapping (linear):");
		}
	for(i=0; i < 3; i++) {
		if((*(p_CsPressure[i]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsPressure[i]))[j]);
		else strcpy(line,"[?]");
		Print(w,line);
		if((*(p_CsPressure[i+3]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsPressure[i+3]))[j]);
		else strcpy(line,"[?]");
		my_sprintf(Message," <-> %s",line);
		Println(w,Message);
		}
	
	if((*p_CsInstrument)[j].rModulation.islogx && !(*p_CsInstrument)[j].rModulation.islogy)
		Println(w,"\nModulation mapping (log to lin):");
	else {
		if(!(*p_CsInstrument)[j].rModulation.islogx && (*p_CsInstrument)[j].rModulation.islogy)
			Println(w,"\nModulation mapping (lin to log):");
		else Println(w,"\nModulation mapping (linear):");
		}
	for(i=0; i < 3; i++) {
		if((*(p_CsModulation[i]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsModulation[i]))[j]);
		else strcpy(line,"[?]");
		Print(w,line);
		if((*(p_CsModulation[i+3]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsModulation[i+3]))[j]);
		else strcpy(line,"[?]");
		my_sprintf(Message," <-> %s",line);
		Println(w,Message);
		}
	
	if((*p_CsInstrument)[j].rPanoramic.islogx && !(*p_CsInstrument)[j].rPanoramic.islogy)
		Println(w,"\nPanoramic mapping (log to lin):");
	else {
		if(!(*p_CsInstrument)[j].rPanoramic.islogx && (*p_CsInstrument)[j].rPanoramic.islogy)
			Println(w,"\nPanoramic mapping (lin to log):");
		else Println(w,"\nPanoramic mapping (linear):");
		}
	for(i=0; i < 3; i++) {
		PleaseWait();
		if((*(p_CsPanoramic[i]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsPanoramic[i]))[j]);
		else strcpy(line,"[?]");
		Print(w,line);
		if((*(p_CsPanoramic[i+3]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsPanoramic[i+3]))[j]);
		else strcpy(line,"[?]");
		my_sprintf(Message," <-> %s",line);
		Println(w,Message);
		}
	}
else {
	SetCsoundLogButtons(j);
	for(i=0; i < 6; i++) {
		if((*(p_CsPitchBend[i]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsPitchBend[i]))[j]);
		else line[0] = '\0';
		SetField(NULL,wCsoundResources,fPitchBendIndex+1+i,line);
		
		if((*(p_CsVolume[i]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsVolume[i]))[j]);
		else line[0] = '\0';
		SetField(NULL,wCsoundResources,fVolumeIndex+1+i,line);
		
		if((*(p_CsPressure[i]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsPressure[i]))[j]);
		else line[0] = '\0';
		SetField(NULL,wCsoundResources,fPressureIndex+1+i,line);
		
		if((*(p_CsModulation[i]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsModulation[i]))[j]);
		else line[0] = '\0';
		SetField(NULL,wCsoundResources,fModulationIndex+1+i,line);
		
		if((*(p_CsPanoramic[i]))[j] < Infpos)
			WriteFloatToLine(line,(*(p_CsPanoramic[i]))[j]);
		else line[0] = '\0';
		SetField(NULL,wCsoundResources,fPanoramicIndex+1+i,line);
		}
	}
	
if((*p_CsInstrument)[j].ipmax > IPMAX) {
	BPPrintMessage(0,odError,"=> Err. SetCsoundInstrument(). (*p_CsInstrument)[j].ipmax > IPMAX");
	(*p_CsInstrument)[j].ipmax = 0;
	}

SetCsoundMoreParametersWindow(j,w);
return(OK);
}


SetCsoundMoreParametersWindow(int j,int w)
{
int ip;
char line[MAXFIELDCONTENT];

CsoundParam **paramlist;

paramlist = (*p_CsInstrument)[j].paramlist;

for(ip=0; ip < IPMAX; ip++) {
	if((*p_CsInstrument)[j].paramlist == NULL || ip >= (*p_CsInstrument)[j].ipmax) {
		if(w <= 0) {
			SetField(CsoundInstrMorePtr,-1,fMoreStartIndex + (7*ip),"\0");
			SetField(CsoundInstrMorePtr,-1,fMoreEndIndex + (7*ip),"\0");
			SetField(CsoundInstrMorePtr,-1,fMoreTable + ip,"\0");
			SetField(CsoundInstrMorePtr,-1,fMoreName + (7*ip),"\0");
			SetField(CsoundInstrMorePtr,-1,fMoreComment + (7*ip),"[Comment]");
			SetField(CsoundInstrMorePtr,-1,fDefaultCsoundParameterValue + (4*ip),"0.00");
			SetField(CsoundInstrMorePtr,-1,fCsoundParameterGenType + (4*ip),"07");
			SwitchOn(CsoundInstrMorePtr,-1,bADDval + (2*ip));
			SwitchOff(CsoundInstrMorePtr,-1,bMULTval + (2*ip));
			}
		continue;
		}
	MystrcpyHandleToString(MAXFIELDCONTENT,0,line,(*paramlist)[ip].name);
	SetField(CsoundInstrMorePtr,-1,fMoreName + (7*ip),line);
	
	if(w > 0) {
		PleaseWait();
		if(line[0] != '\0') my_sprintf(Message,"\nParameter « %s »",line);
		else continue;
		}
	
	if((*paramlist)[ip].startindex > -1) {
		my_sprintf(line,"%ld",(long)(*paramlist)[ip].startindex);
		if(w > 0) {
			strcat(Message," start argument ");
			strcat(Message,line);
			}
		}
	else line[0] = '\0';
	SetField(CsoundInstrMorePtr,-1,fMoreStartIndex + (7*ip),line);
	
	if((*paramlist)[ip].endindex > -1) {
		my_sprintf(line,"%ld",(long)(*paramlist)[ip].endindex);
		if(w > 0) {
			strcat(Message,", end argument ");
			strcat(Message,line);
			}
		}
	else line[0] = '\0';
	SetField(CsoundInstrMorePtr,-1,fMoreEndIndex + (7*ip),line);
	
	if((*paramlist)[ip].table > 0) {
		my_sprintf(line,"%ld",(long)(*paramlist)[ip].table);
		if(w > 0) {
			strcat(Message,", table argument ");
			strcat(Message,line);
			}
		}
	else line[0] = '\0';
	SetField(CsoundInstrMorePtr,-1,fMoreTable + ip,line);
		
	my_sprintf(line,"0%ld",(long)(*paramlist)[ip].GENtype);
	SetField(CsoundInstrMorePtr,-1,fCsoundParameterGenType + (4*ip),line);
	if(w > 0) {
		strcat(Message,", GEN");
		strcat(Message,line);
		Println(w,Message);
		}
	
	my_sprintf(line,"%.3f",(*paramlist)[ip].defaultvalue);
	SetField(CsoundInstrMorePtr,-1,fDefaultCsoundParameterValue + (4*ip),line);
	if(w > 0) {
		my_sprintf(Message,"Default value = %s",line);
		Println(w,Message);
		}
	
	switch((*paramlist)[ip].combinationtype) {
		case ADD:
			SwitchOn(CsoundInstrMorePtr,-1,bADDval + (2*ip));
			SwitchOff(CsoundInstrMorePtr,-1,bMULTval + (2*ip));
			if(w > 0) strcpy(line,"additive");
			break;
		case MULT:
			SwitchOn(CsoundInstrMorePtr,-1,bMULTval + (2*ip));
			SwitchOff(CsoundInstrMorePtr,-1,bADDval + (2*ip));
			if(w > 0) strcpy(line,"multiplicative");
			break;
		default:
			BPPrintMessage(0,odError,"=> Err. SetCsoundInstrument(). Incorrect (*paramlist)[ip].combinationtype");
			break;
		}
	if(w > 0) {
		my_sprintf(Message,"Combination type = %s",line);
		Println(w,Message);
		}
	
	MystrcpyHandleToString(MAXFIELDCONTENT,0,line,(*paramlist)[ip].comment);
	SetField(CsoundInstrMorePtr,-1,fMoreComment + (7*ip),line);
	
	if(w > 0 && line[0] != '\0') {
		my_sprintf(Message,"Comment: %s",line);
		Println(w,Message);
		}
	}
if(w > 0) Println(w,"\n----------------------------------");
return(OK);
}

CopyCsoundInstrument(int i,int j)
{
int ip;
CsoundParam **ptr3;
char **ptr;

if(j < 0 || j >= Jinstr || i < 0 || i >= Jinstr) {
	BPPrintMessage(0,odError,"=> Err. CopyCsoundInstrument(). Incorrect index");
	return(MISSED);
	}
(*p_CsDilationRatioIndex)[j] = (*p_CsDilationRatioIndex)[i];
(*p_CsAttackVelocityIndex)[j] = (*p_CsAttackVelocityIndex)[i];
(*p_CsReleaseVelocityIndex)[j] = (*p_CsReleaseVelocityIndex)[i];
(*p_CsPitchBendStartIndex)[j] = (*p_CsPitchBendStartIndex)[i];
(*p_CsVolumeStartIndex)[j] = (*p_CsVolumeStartIndex)[i];
(*p_CsPressureStartIndex)[j] = (*p_CsPressureStartIndex)[i];
(*p_CsModulationStartIndex)[j] = (*p_CsModulationStartIndex)[i];
(*p_CsPanoramicStartIndex)[j] = (*p_CsPanoramicStartIndex)[i];
(*p_CsPitchBendEndIndex)[j] = (*p_CsPitchBendEndIndex)[i];
(*p_CsVolumeEndIndex)[j] = (*p_CsVolumeEndIndex)[i];
(*p_CsPressureEndIndex)[j] = (*p_CsPressureEndIndex)[i];
(*p_CsModulationEndIndex)[j] = (*p_CsModulationEndIndex)[i];
(*p_CsPanoramicEndIndex)[j] = (*p_CsPanoramicEndIndex)[i];

(*p_CsPitchIndex)[j] = (*p_CsPitchIndex)[i];
(*p_CsPitchFormat)[j] = (*p_CsPitchFormat)[i];
(*p_CsDilationRatioIndex)[j] = (*p_CsDilationRatioIndex)[i];
(*p_CsDilationRatioIndex)[j] = (*p_CsDilationRatioIndex)[i];
(*p_CsDilationRatioIndex)[j] = (*p_CsDilationRatioIndex)[i];

for(ip=0; ip < 6; ip++) {
	(*(p_CsPitchBend[ip]))[j] = (*(p_CsPitchBend[ip]))[i];
	(*(p_CsVolume[ip]))[j] = (*(p_CsVolume[ip]))[i];
	(*(p_CsPressure[ip]))[j] = (*(p_CsPressure[ip]))[i];
	(*(p_CsModulation[ip]))[j] = (*(p_CsModulation[ip]))[i];
	(*(p_CsPanoramic[ip]))[j] = (*(p_CsPanoramic[ip]))[i];
	}

if((*p_CsInstrument)[j].paramlist != NULL) {
	for(ip=0; ip < (*p_CsInstrument)[j].ipmax; ip++) {
		ptr = (*((*p_CsInstrument)[j].paramlist))[ip].name;
		MyDisposeHandle((Handle*)&ptr);
		ptr = (*((*p_CsInstrument)[j].paramlist))[ip].comment;
		MyDisposeHandle((Handle*)&ptr);
		}
	ptr3 = (*p_CsInstrument)[j].paramlist;
	MyDisposeHandle((Handle*)&ptr3);
	(*p_CsInstrument)[j].paramlist = NULL;
	}
(*p_CsInstrument)[j].ipmax = 0;

(*p_CsInstrument)[j] = (*p_CsInstrument)[i];

if((*p_CsInstrument)[i].ipmax > 0) {
	if((ptr3=(CsoundParam**) GiveSpace((Size)(IPMAX * sizeof(CsoundParam)))) == NULL)
		return(ABORT);
	(*p_CsInstrument)[j].paramlist = ptr3;
	(*p_CsInstrument)[j].ipmax = IPMAX;
	for(ip=0; ip < (*p_CsInstrument)[i].ipmax; ip++) {
		(*((*p_CsInstrument)[j].paramlist))[ip] = (*((*p_CsInstrument)[i].paramlist))[ip];
		if((*((*p_CsInstrument)[i].paramlist))[ip].name != NULL) {
			if((ptr=(char**) GiveSpace(sizeof((*((*p_CsInstrument)[i].paramlist))[ip].name)))
				== NULL) return(ABORT);
			MystrcpyHandleToHandle(0,&ptr,(*((*p_CsInstrument)[i].paramlist))[ip].name);
			(*((*p_CsInstrument)[j].paramlist))[ip].name = ptr;
			}
		
		if((*((*p_CsInstrument)[i].paramlist))[ip].comment != NULL) {
			if((ptr=(char**) GiveSpace(sizeof((*((*p_CsInstrument)[i].paramlist))[ip].comment)))
				== NULL) return(ABORT);
			MystrcpyHandleToHandle(0,&ptr,(*((*p_CsInstrument)[i].paramlist))[ip].comment);
			(*((*p_CsInstrument)[j].paramlist))[ip].comment = ptr;
			}
		}
	for(ip=(*p_CsInstrument)[i].ipmax; ip < IPMAX; ip++) {
		// This may happen with future versions of BP3 when IPMAX gets larger
		(*((*p_CsInstrument)[j].paramlist))[ip].name = NULL;
		(*((*p_CsInstrument)[j].paramlist))[ip].comment = NULL;
		if(ResetMoreParameter(j,ip) != OK) return(ABORT);
		}
	}
return(OK);
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

/* New argument "newinstr" should be YES when the instrument space
   is newly allocated and uninitialized.  This is the case when called from 
   ResizeCsoundInstrumentsSpace().  When "all" is NO, then the instrument
   index, name, and comment are preserved.  Changes by BB. */
int ResetCsoundInstrument(int j,int all, int newinstr)
{
Handle ptr;
int i,channel;

if(!newinstr) {
	// Jinstr has not been adjusted yet when resizing
	if(j < 0 || j >= Jinstr) { 
		BPPrintMessage(0,odError,"=> Err. ResetCsoundInstrument(). Incorrect index = %d\n",j);
		return(MISSED);
		}
	for(channel=1; channel <= MAXCHAN; channel++) {
		if(WhichCsoundInstrument[channel] == (*p_CsInstrumentIndex)[j]) {
			WhichCsoundInstrument[channel] = -1;
			break;
			}
		}
	}

if(all || newinstr) (*p_CsInstrumentIndex)[j] = -1;
(*p_CsDilationRatioIndex)[j] = (*p_CsAttackVelocityIndex)[j]
	= (*p_CsReleaseVelocityIndex)[j]
	= (*p_CsPressureStartIndex)[j] = (*p_CsModulationStartIndex)[j]
	= (*p_CsPanoramicStartIndex)[j]
	= (*p_CsPressureEndIndex)[j] = (*p_CsModulationEndIndex)[j]
	= (*p_CsPanoramicEndIndex)[j] = -1;
	
(*p_CsPitchIndex)[j] = 4;
(*p_CsPitchFormat)[j] = OPPC;

(*p_CsVolumeStartIndex)[j] = 5;
(*p_CsVolumeEndIndex)[j] = 6;
(*p_CsInstrument)[j].volumetable = 7;

(*p_CsPitchBendStartIndex)[j] = 8;
(*p_CsPitchBendEndIndex)[j] = 9;
(*p_CsInstrument)[j].pitchbendtable = 10;

(*p_CsInstrument)[j].rPitchBend.islogx = (*p_CsInstrument)[j].rPitchBend.islogy = TRUE;

(*p_CsInstrument)[j].rVolume.islogx = (*p_CsInstrument)[j].rVolume.islogy
	= (*p_CsInstrument)[j].rPressure.islogx = (*p_CsInstrument)[j].rPressure.islogy
	= (*p_CsInstrument)[j].rModulation.islogx = (*p_CsInstrument)[j].rModulation.islogy
	= (*p_CsInstrument)[j].rPanoramic.islogx = (*p_CsInstrument)[j].rPanoramic.islogy = FALSE;

(*p_CsInstrument)[j].pitchbendrange = 200; // 2025-01-19

(*p_CsInstrument)[j].pressuretable = (*p_CsInstrument)[j].modulationtable
	= (*p_CsInstrument)[j].panoramictable = -1;

(*p_CsInstrument)[j].pitchbendGEN = (*p_CsInstrument)[j].volumeGEN
	= (*p_CsInstrument)[j].pressureGEN = (*p_CsInstrument)[j].modulationGEN
	= (*p_CsInstrument)[j].panoramicGEN = 7;

(*p_CsInstrument)[j].iargmax = 10;	/* Three Arguments are compulsory */

if(!newinstr) {
	for(i=0; i < (*p_CsInstrument)[j].ipmax; i++) {
		if((*p_CsInstrument)[j].paramlist == NULL) {
			BPPrintMessage(0,odError,"=> Err. ResetCsoundInstrument(). (*p_CsInstrument)[j].paramlist == NULL\n",j);
			continue;
			}
		ptr = (Handle) (*((*p_CsInstrument)[j].paramlist))[i].name;
		MyDisposeHandle(&ptr);
		ptr = (Handle) (*((*p_CsInstrument)[j].paramlist))[i].comment;
		MyDisposeHandle(&ptr);
		}
	ptr = (Handle) (*p_CsInstrument)[j].paramlist;
	MyDisposeHandle(&ptr);
	}
	
(*p_CsInstrument)[j].paramlist = NULL;
(*p_CsInstrument)[j].ipmax = 0;

if(all) {
	ptr = (Handle)(*pp_CsInstrumentName)[j];
	MySetHandleSize(&ptr,(Size)2L * sizeof(char));
	(*pp_CsInstrumentName)[j] = (char**) ptr;
//	(*((*pp_CsInstrumentName)[j]))[0] = '\0';
	strcpy((*((*pp_CsInstrumentName)[j])),""); // Fixed by BB 2021-02-14
	
	ptr = (Handle)(*pp_CsInstrumentComment)[j];
	MySetHandleSize(&ptr,(Size)2L * sizeof(char));
	(*pp_CsInstrumentComment)[j] = (char**) ptr;
//	(*((*pp_CsInstrumentComment)[j]))[0] = '\0';
	strcpy((*((*pp_CsInstrumentComment)[j])),""); // Fixed by BB 2021-02-14
	}

(*(p_CsPitchBend[0]))[j] = 0.; (*(p_CsPitchBend[1]))[j] = DEFTPITCHBEND;
(*(p_CsPitchBend[2]))[j] = 16383.;
(*(p_CsVolume[0]))[j] = 0.; (*(p_CsVolume[1]))[j] = 64.;
(*(p_CsVolume[2]))[j] = 127.;
(*(p_CsPressure[0]))[j] = 0.; (*(p_CsPressure[1]))[j] = 64.;
(*(p_CsPressure[2]))[j] = 127.;
(*(p_CsModulation[0]))[j] = 0.; (*(p_CsModulation[1]))[j] = 8192.;
(*(p_CsModulation[2]))[j] = 16383.;
(*(p_CsPanoramic[0]))[j] = 0.; (*(p_CsPanoramic[1]))[j] = 64.;
(*(p_CsPanoramic[2]))[j] = 127.;

(*(p_CsPitchBend[3]))[j] = -200.; (*(p_CsPitchBend[4]))[j] = 0.;
(*(p_CsPitchBend[5]))[j] = 200.;
(*(p_CsVolume[3]))[j] = Infpos1; (*(p_CsVolume[4]))[j] = Infpos1;
(*(p_CsVolume[5]))[j] = Infpos1;
(*(p_CsPressure[3]))[j] = Infpos1; (*(p_CsPressure[4]))[j] = Infpos1;
(*(p_CsPressure[5]))[j] = Infpos1;
(*(p_CsModulation[3]))[j] = Infpos1; (*(p_CsModulation[4]))[j] = Infpos1;
(*(p_CsModulation[5]))[j] = Infpos1;
(*(p_CsPanoramic[3]))[j] = Infpos1; (*(p_CsPanoramic[4]))[j] = Infpos1;
(*(p_CsPanoramic[5]))[j] = Infpos1;
CompiledRegressions = FALSE;
return(OK);
}

int FixCsoundScoreName(char* line)
{
return(OK);	// Csound doesn't care! -- akozar 031907
/* if(strcmp(strstr(line,".sco"),".sco") == 0) return(OK);
line[MAXNAME - 1 - strlen(".sco")] = '\0';
strcat(line,".sco");
my_sprintf(Message,"Csound score file was renamed « %s »",line);
ShowMessage(TRUE,wMessage,Message);
return(MISSED);
*/
}


int CompileCsoundObjects(void)
// Tokenize Csound scores in sound-object prototypes
{
int j,rep,longerCsound,maxsounds;

if(CompiledCsObjects || !ObjectMode) return(OK);
if(Jbol < 2) return(OK);

rep = OK;
CompileOn++;
maxsounds = MyGetHandleSize((Handle)p_Type) / sizeof(char);
if(trace_csound) BPPrintMessage(0,odInfo,"Running CompileCsoundObjects() for maxsounds = %ld\n",(long)maxsounds);

for(j=2; j < maxsounds; j++) {
	if(trace_csound) BPPrintMessage(0,odInfo,"(*p_Type)[%d] = %d\n",j,(*p_Type)[j]);
	if((*p_Type)[j] == 0) continue;
	if(!((*p_Type)[j] & 4)) {  // The third bit is not 1
		if((*pp_CsoundScoreText)[j] == NULL) {
			if(trace_csound) BPPrintMessage(0,odInfo, "No Csound score in %d\n",j);
			continue;
			}
		(*p_Type)[j] |= 4; // Set the third bit to 1
		}
	if((rep=CompileObjectScore(j,&longerCsound)) != OK) {
		if(rep == ABORT) goto SORTIR;
		iProto = j;
#if BP_CARBON_GUI_FORGET_THIS
		SetPrototype(iProto);
		SetCsoundScore(iProto);
		ShowWindow(Window[wPrototype1]);
		BringToFront(Window[wPrototype1]);
		BPActivateWindow(SLOW,wPrototype7);
#endif /* BP_CARBON_GUI_FORGET_THIS */
		CompiledCsObjects = 0;
		rep = MISSED;
		goto SORTIR;
		}
	}
CompiledCsObjects = 1;

SORTIR:
if(CompileOn) CompileOn--;
return(rep);
}


int CompileObjectScore(int j,int *p_longerCsound) {
	char c,**p_line,line[MAXLIN];
	int i,ii,ipos,im,i0,i1,ievent,ip,ins,l,nparam,maxparam,maxevents,foundevent,result,
		istempo,overflow,finished,index,html;
	long p,q,count;;
	double param,tempo,dur,**h;
	Handle ptr,ptr2;
	CsoundParam **paramlist;

	if(j < 2 || j >= Jbol) return(OK);

	if(trace_csound) BPPrintMessage(0,odInfo,"CompileObjectScore(%d)\n",j);
	p_line = (*pp_CsoundScoreText)[j];
	if(p_line == NULL) {
		if(trace_csound) BPPrintMessage(0,odInfo, "=> Warning CompileObjectScore(%d). p_line == NULL\n",j);
		return OK;
		}
	if(trace_csound) BPPrintMessage(0,odInfo, "Compiling object score line[%d] = %s\n",j,(*p_line));

	if(strlen(*p_line) == 0 || strcmp((*p_line),"<HTML></HTML>") == 0) {
	/*	ptr = (Handle) (*pp_CsoundScore)[j];
		if(ptr != NULL) {
			if(trace_csound) BPPrintMessage(0,odError, "=> Error (*pp_CsoundScore)[%d] != NULL, (*p_CsoundSize)[%d] = %d\n",j,j,(*p_CsoundSize)[j]);
			for(i=0; i < (*p_CsoundSize)[j]; i++) {
				ptr2 = (Handle) (*((*pp_CsoundScore)[j]))[i].h_param;
				MyDisposeHandle(&ptr2);
				(*((*pp_CsoundScore)[j]))[i].h_param = NULL;
				}
			MyDisposeHandle(&ptr);
			(*pp_CsoundScore)[j] = NULL;
			} */
		(*p_CsoundSize)[j] = ZERO;
		if((*p_Type)[j] & 4) {
			(*p_Type)[j] &= (255-4);
			}
		return(OK);
		}

	if((result=CompileRegressions()) != OK) return(result);

	// if(trace_csound) BPPrintMessage(0,odError,"Compiling ObjectScore(%d) ? CompiledCsoundScore = %d\n",j,(int)(*p_CompiledCsoundScore)[j]);

	if((*p_CompiledCsoundScore)[j]) {
		if(trace_csound) BPPrintMessage(0,odError,"ObjectScore « %s » is already compiled\n",*((*p_Bol)[j]));
		return(OK);
		}

	if(trace_csound) BPPrintMessage(0,odInfo, "Compiling Csound score in object « %s »\n",*((*p_Bol)[j]));

	maxevents = 12;

	ptr = (Handle) (*pp_CsoundScore)[j];
	if(ptr != NULL) {
		for(i=0; i < (*p_CsoundSize)[j]; i++) {
			ptr2 = (Handle) (*((*pp_CsoundScore)[j]))[i].h_param;
			MyDisposeHandle(&ptr2);
			(*((*pp_CsoundScore)[j]))[i].h_param = NULL;
			}
		MySetHandleSize((Handle*)&ptr,(Size)maxevents * sizeof(CsoundLine));
		}
	else {
		if((ptr=(Handle) GiveSpace((Size)(maxevents * sizeof(CsoundLine)))) == NULL)
			return(ABORT);
		}

	(*pp_CsoundScore)[j] = (CsoundLine**) ptr;

	for(i=0; i < maxevents; i++) {
		(*((*pp_CsoundScore)[j]))[i].h_param = NULL;
		}
		
	ptr = (Handle) (*pp_CsoundTime)[j];
	MyDisposeHandle(&ptr);
	if((ptr=(Handle) GiveSpace((Size)(maxevents * sizeof(Milliseconds)))) == NULL)
			return(ABORT);
	(*pp_CsoundTime)[j] = (Milliseconds**) ptr;
	(*p_CsoundSize)[j] = ZERO;

	ievent = 0; result = OK; finished = FALSE; h = NULL;

	CompileOn++;

	// BPPrintMessage(0,odError,"Now compiling ObjectScore(%d)\n",j);
	count = 1L + MyHandleLen(p_line);
	html = TRUE;
	CheckHTML(TRUE,0,p_line,&count,&html);
	ipos = 0; im = MyHandleLen(p_line);
	(*p_CsoundTempo)[j] = tempo = 60.;

	l = strlen((*p_line)); 
	if(l == 0) return(OK);
	if(trace_csound) BPPrintMessage(0,odInfo,"Compiling Csound score (length %d):\n%s\n",l,(*p_line));

	while(TRUE) {
		result = OK;
		while(ipos < im && (isspace(c=(*p_line)[ipos]) || c == '\0')) ipos++;
		i0 = ipos;
		if(ipos >= im) break;
		if(c == '/' && ipos < (im-1) && (*p_line)[ipos+1] == '/') {	/* Skip C-like remarks */
			while(ipos < im && (*p_line)[ipos] != '\r' && (*p_line)[ipos] != '\n' && (*p_line)[ipos] != '\0')
				ipos++;
			continue;
			}
		while(ipos < im && (c=(*p_line)[ipos]) != '\r' && c != '\n' && c != '\0')
			ipos++;
		i1 = ipos;
		istempo = FALSE;
		c = (*p_line)[i0];
		if(c == '_') goto NEXTLINE;
		if(c != 'i' && c != 't' && c != 'f' && c != ';' && c != 'e') {
			BPPrintMessage(0,odInfo,"\n=> Csound score line must start with 'i', 'f', 't' or a semi-colon. Can't accept '%c' in the score of object « %s ».\nPart of this score will be ignored:\n%s\n",c,*((*p_Bol)[j]),(*p_line));
			result = OK; goto SORTIR;
			}
		foundevent = FALSE;
		
		(*((*pp_CsoundScore)[j]))[ievent].nbparameters = 0;
		nparam = -1;
		maxparam = 1;
		if((h=(double**) GiveSpace((Size)(maxparam * sizeof(double)))) == NULL) {
			result = ABORT; goto SORTIR;	/* Will be resized later */
			}
		i0++;
		ip = 1;
		if(c == ';') goto NEXTLINE;
		if(finished) {
			BPPrintMessage(0,odError,"=> Unwanted events have been found beyond end of score");
			result = MISSED; goto SORTIR;
			}
		if(c == 't') istempo = TRUE;
		if(c == 'e') finished = TRUE;
		if(c == 'f') {
			BPPrintMessage(0,odInfo,"\n=> Opcode 'f' (table definition) is not supported in sound-object scores. The incorrect line has been ignored in object « %s »:\n%s\n",*((*p_Bol)[j]),(*p_line));
			goto NEXTLINE;
			}

	NEWPARAMETER:
		PleaseWait();
		while(i0 < i1 && (isspace(c=(*p_line)[i0]) || c == '\0')) i0++;
		for(ii=i0; ii < i1; ii++) {
			c = (*p_line)[ii];
			if(isspace(c) || c == ';') break;
			line[ii-i0] = c;
			}
		line[ii-i0] = '\0';
		i0 = ii;
		Strip(line);
		if((c=line[0]) == '\0' || c == ';') goto NEXTLINE;
		param = Myatof(line,&p,&q);
		if(istempo) {
			if(ip == 1 && param != 0) {
				my_sprintf(Message,"=> BP3 can't compile this Csound score: argument following 't' should be 0. Can't accept %.2f",
					param);
				BPPrintMessage(0,odError,"%s",Message);
				result = MISSED;
				goto SORTIR;
				}
			if(ip > 2) {
				my_sprintf(Message,"=> BP3 can't compile this Csound score: more than %ld arguments following 't'",
					(long)ip-1L);
				BPPrintMessage(0,odError,"%s",Message);
				result = MISSED;
				goto SORTIR;
				}
			if(ip == 2) {
				if(param <= 0.) {
					my_sprintf(Message,"=> Incorrect Csound score: 2nd argument following 't' should be positive. Can't accept %.2f",
						param);
					BPPrintMessage(0,odError,"%s",Message);
					result = MISSED;
					goto SORTIR;
					}
				tempo = (*p_CsoundTempo)[j] = param;
				}
			}
		else {
			foundevent = TRUE;
			switch(ip) {
				case 1:
					if((*p_CsoundInstr)[j] > 0) param = (*p_CsoundInstr)[j];
					for(ins=0; ins < Jinstr; ins++) {
						if((*p_CsInstrumentIndex)[ins] == param) break;
						}
					if(ins >= Jinstr || param != (*p_CsInstrumentIndex)[ins]) {
						if(Jinstr > 1 || ((*p_CsInstrumentIndex)[0] > 0
								&& param != (*p_CsInstrumentIndex)[0])) {
							my_sprintf(Message,
								"=> Can't compile Csound score because instrument %ld is not defined. You may modify or load the '-cs' file",
								(long)param);
							BPPrintMessage(0,odError,"%s",Message);
							if((*p_CsoundInstr)[j] > 0) {
	#if BP_CARBON_GUI_FORGET_THIS
								ShowWindow(Window[wPrototype1]);
								BringToFront(Window[wPrototype1]);
								BPActivateWindow(SLOW,wPrototype8);
								SelectField(NULL,wPrototype8,fForceToInstrument,TRUE);
	#endif /* BP_CARBON_GUI_FORGET_THIS */
								my_sprintf(Message,"=> Perhaps the problem is that you forced this sound-object to use instrument %ld",
									(long)param);
								BPPrintMessage(0,odError,"%s",Message);
								}
							result = MISSED;
							goto SORTIR;
							}
						ins = 0;
						}
					(*((*pp_CsoundScore)[j]))[ievent].instrument = param;
					
					paramlist = (*p_CsInstrument)[ins].paramlist;
					
					/* Check the number of arguments in this instrument */
					maxparam = (*p_CsInstrument)[ins].iargmax - 3;
					if(MySetHandleSize((Handle*)&h,(Size)maxparam * sizeof(double)) != OK) {
						result = ABORT; goto SORTIR;
						}
					(*((*pp_CsoundScore)[j]))[ievent].nbparameters = maxparam;
					
					/* Set all arguments to default because the score line might be incomplete */
					for(i=4; i <= (*p_CsInstrument)[ins].iargmax; i++) {
						(*h)[i-4] = 0.;
						if(i == (*p_CsPitchBendStartIndex)[ins] || i == (*p_CsPitchBendEndIndex)[ins]) {
							(*h)[i-4] = DEFTPITCHBEND; continue;
							}
						if(i == (*p_CsVolumeStartIndex)[ins] || i == (*p_CsVolumeEndIndex)[ins]) {
							(*h)[i-4] = DeftVolume; continue;
							}
						if(i == (*p_CsPressureStartIndex)[ins] || i == (*p_CsPressureEndIndex)[ins]) {
							(*h)[i-4] = DEFTPRESSURE; continue;
							}
						if(i == (*p_CsModulationStartIndex)[ins] || i == (*p_CsModulationEndIndex)[ins]) {
							(*h)[i-4] = DEFTMODULATION; continue;
							}
						if(i == (*p_CsPanoramicStartIndex)[ins] || i == (*p_CsPanoramicEndIndex)[ins]) {
							(*h)[i-4] = DeftPanoramic; continue;
							}
						}
					for(i=0; i < (*p_CsInstrument)[ins].ipmax; i++) {
						if((index=(*paramlist)[i].startindex) > 0) {
							if(index < 4 || index >= (maxparam+4)) {
								BPPrintMessage(0,odError,"=> Err. CompileObjectScore(). index < 4 || index >= maxparam");
								}
							else (*h)[index-4] = (*paramlist)[i].defaultvalue;
							}
						if((index=(*paramlist)[i].endindex) > 0) {
							if(index < 4 || index >= (maxparam+4)) {
								BPPrintMessage(0,odError,"=> Err. CompileObjectScore(). index < 4 || index >= maxparam");
								}
							else (*h)[index-4] = (*paramlist)[i].defaultvalue;
							}
						}
					break;
				case 2:
					param = (param * 60000.) / tempo;
					(*((*pp_CsoundTime)[j]))[ievent] = param;
					break;
				case 3:
					if(param < 0) {
						// FIXME ? any way to allow neg. dur? This has multiple uses with Csound -- akozar
						BPPrintMessage(0,odError,"=> Can't compile Csound score because a negative duration was found.\n(3d argument)");
						result = MISSED; goto SORTIR;
						}
					param = (param * 60000.) / tempo;
					(*((*pp_CsoundScore)[j]))[ievent].duration = param;
					break;
				default:
					/* Modify param according to instrument mapping */
					if(ins < 0 || ins >= Jinstr) {
						BPPrintMessage(0,odError,"=> Err. CompileObjectScore(). ins < 0 || ins >= Jinstr");
						if(CompileOn) CompileOn--;
						return(ABORT);
						}
					if(ip == (*p_CsPitchBendStartIndex)[ins] || ip == (*p_CsPitchBendEndIndex)[ins]) {
						param = YtoX(param,&((*p_CsInstrument)[ins].rPitchBend),&overflow,
							(*((*pp_CsoundScore)[j]))[ievent].instrument);
						if(overflow) {
							result = ABORT; goto SORTIR;
							}
						}
					if(ip == (*p_CsVolumeStartIndex)[ins] || ip == (*p_CsVolumeEndIndex)[ins]) {
						param = YtoX(param,&((*p_CsInstrument)[ins].rVolume),&overflow,
							(*((*pp_CsoundScore)[j]))[ievent].instrument);
						if(overflow) {
							result = ABORT; goto SORTIR;
							}
						}
					if(ip == (*p_CsPressureStartIndex)[ins] || ip == (*p_CsPressureEndIndex)[ins]) {
						param = YtoX(param,&((*p_CsInstrument)[ins].rPressure),&overflow,
							(*((*pp_CsoundScore)[j]))[ievent].instrument);
						if(overflow) {
							result = ABORT; goto SORTIR;
							}
						}
					if(ip == (*p_CsModulationStartIndex)[ins] || ip == (*p_CsModulationEndIndex)[ins]) {
						param = YtoX(param,&((*p_CsInstrument)[ins].rModulation),&overflow,
							(*((*pp_CsoundScore)[j]))[ievent].instrument);
						if(overflow) {
							result = ABORT; goto SORTIR;
							}
						}
					if(ip == (*p_CsPanoramicStartIndex)[ins] || ip == (*p_CsPanoramicEndIndex)[ins]) {
						param = YtoX(param,&((*p_CsInstrument)[ins].rPanoramic),&overflow,
							(*((*pp_CsoundScore)[j]))[ievent].instrument);
						if(overflow) {
							result = ABORT; goto SORTIR;
							}
						}
					if(++nparam >= maxparam) {
						if((*p_CsInstrumentIndex)[ins] > 0)
							my_sprintf(Message,
								"=> Csound instrument %ld accepts %ld arguments but the score is supplying more",
								(long)(*p_CsInstrumentIndex)[ins],(long)(maxparam+3));
						else
							my_sprintf(Message,
								"=> Default Csound instrument requires %ld arguments but the score is supplying more",
								(long)(maxparam+3));
						BPPrintMessage(0,odError,"%s",Message);
						result = ABORT;
						OutCsound = FALSE;
						goto SORTIR;
						}
					(*h)[nparam] = param;
					break;
				}
			}
		ip++;
		if(i0 < i1) goto NEWPARAMETER;
		
	NEXTLINE:
		if(foundevent) {
			if(ip < 4) {
				if((*p_CsInstrumentIndex)[ins] > 0)
					my_sprintf(Message,
						"=> Csound instrument %ld requires at least 3 arguments whereas the score is supplying %ld ones",
						(long)(*p_CsInstrumentIndex)[ins],(long)(ip-1));
				else
					my_sprintf(Message,
						"=> Default Csound instrument requires at least 3 arguments whereas the score is supplying %ld ones",
						(long)(ip-1));
				BPPrintMessage(0,odError,"%s",Message);
				if(Jinstr < 2)
					BPPrintMessage(0,odError,"=> You probably forgot to create or load a '-cs' instrument file");
				if((*p_CsoundInstr)[j] > 0) {
	#if BP_CARBON_GUI_FORGET_THIS
					ShowWindow(Window[wPrototype1]);
					BringToFront(Window[wPrototype1]);
					BPActivateWindow(SLOW,wPrototype8);
					SelectField(NULL,wPrototype8,fForceToInstrument,TRUE);
	#endif /* BP_CARBON_GUI_FORGET_THIS */
					my_sprintf(Message,"=> Perhaps the problem is that this sound-object is instructed to use instrument %ld",
						(long)param);
					BPPrintMessage(0,odError,"%s",Message);
					}
				MyDisposeHandle((Handle*)&h);
				result = MISSED;
				OutCsound = FALSE;
				goto SORTIR;
				}
				
			(*((*pp_CsoundScore)[j]))[ievent].h_param = h;
			h = NULL;
			
			ievent++;
			if(ievent >= maxevents) {
				ptr = (Handle) (*pp_CsoundScore)[j];
				if((ptr = IncreaseSpace((Handle)ptr)) == NULL) {
					result = ABORT; goto SORTIR;
					}
				(*pp_CsoundScore)[j] = (CsoundLine**) ptr;
				ptr = (Handle) (*pp_CsoundTime)[j];
				if((ptr = IncreaseSpace((Handle)ptr)) == NULL) {
					result = ABORT; goto SORTIR;
					}
				(*pp_CsoundTime)[j] = (Milliseconds**) ptr;
				maxevents = (3L * maxevents) / 2L;
				}
			}
		else MyDisposeHandle((Handle*)&h);
		ipos++;
		}

	SORTIR:
	if(CompileOn) CompileOn--;
	// HideWindow(Window[wMessage]);
	MyDisposeHandle((Handle*)&h); /* Useful when result != OK */

	if(result != OK) ievent = 0;
	(*p_CsoundSize)[j] = ievent;
	if(ievent > 0) {
		ptr = (Handle) (*pp_CsoundScore)[j];
		MySetHandleSize((Handle*)&ptr,(Size) ievent * sizeof(CsoundLine));
		(*pp_CsoundScore)[j] = (CsoundLine**) ptr;
		
		ptr = (Handle) (*pp_CsoundTime)[j];
		MySetHandleSize((Handle*)&ptr,(Size) ievent * sizeof(Milliseconds));
		(*pp_CsoundTime)[j] = (Milliseconds**) ptr;
		
		SortCsoundDates(ZERO,j);
		/* This is important for the correct scheduling of events in MakeSound() */
		}

	PointCsound = TRUE;
	// BPPrintMessage(0,odError,"CompiledObjectScore(%d)\n",j);

	if(PointToDuration(NULL,pp_CsoundTime,p_CsoundSize,j) != OK) return(ABORT); 

	SetPrototypeDuration(j,p_longerCsound);

	if(iProto == 0 && Jbol > 2) iProto = 2;
	(*p_CompiledCsoundScore)[j] = 1;
	if(result == OK) BPPrintMessage(0,odError,"Csound score of object « %s » successfully compiled\n",*((*p_Bol)[j]));
	else BPPrintMessage(0,odError,"Csound score of object « %s » not successfully compiled\n",*((*p_Bol)[j]));
	return(result);
	}


int DeCompileObjectScore(int j)
{
long ievent;
int k,ip,rep,ins,longerCsound,overflow,result,instrumentindex;
double param;

if(j < 2 || j >= Jbol) {
	BPPrintMessage(0,odError,"=> Err. DeCompileObjectScore(). j < 2 || j >= Jbol");
	return(MISSED);
	}
result = OK;
if((*p_CsoundSize)[j] < 1L) return(OK);
if(CompileRegressions() != OK) return(MISSED);
// if((rep=ClearWindow(NO,wPrototype7)) != OK) return(rep);

CompileOn++;

if(ShowMessages && !LoadOn)
	ShowMessage(TRUE,wMessage,"The Csound score of this object is being updated...");

if(DurationToPoint(NULL,pp_CsoundTime,p_CsoundSize,j) != OK) {
	if(CompileOn) CompileOn--;
	return(ABORT);
	}
#if BP_CARBON_GUI_FORGET_THIS
if(j != iProto) SetPrototype(iProto=j);
#endif /* BP_CARBON_GUI_FORGET_THIS */
PrintBehind(wPrototype7,"t 0 ");
WriteFloatToLine(LineBuff,(double)(*p_CsoundTempo)[j]);
PrintBehindln(wPrototype7,LineBuff);
for(ievent=ZERO; ievent < (*p_CsoundSize)[j]; ievent++) {
	PleaseWait();
#if BP_CARBON_GUI_FORGET_THIS
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	if((result=MyButton(1)) != MISSED) {
		if(result != OK || (result=InterruptCompileCscore()) != OK) goto SORTIR;
		}
#endif /* BP_CARBON_GUI_FORGET_THIS */
	result = OK;
	instrumentindex = (*((*pp_CsoundScore)[j]))[ievent].instrument;
	for(ins=0; ins < Jinstr; ins++) {
		if((*p_CsInstrumentIndex)[ins] == instrumentindex) break;
		}
	if(ins >= Jinstr) {
		if(instrumentindex > 1) {
			BPPrintMessage(0,odError,"=> Can't decompile this Csound score. We need a '-cs' file describing Csound instruments");
			PointToDuration(NULL,pp_CsoundTime,p_CsoundSize,j);
			BPActivateWindow(SLOW,wCsoundResources);
			DurationToPoint(NULL,pp_CsoundTime,p_CsoundSize,j);
			result = ABORT;
			goto SORTIR;
			}
		ins = 0; instrumentindex = 1;
		}
	my_sprintf(LineBuff,"i%ld ",(long)instrumentindex);
	PrintBehind(wPrototype7,LineBuff);
	WriteFloatToLine(LineBuff,(double)(*((*pp_CsoundTime)[j]))[ievent]
											* (*p_CsoundTempo)[j] / 60000.);
	PrintBehind(wPrototype7,LineBuff); PrintBehind(wPrototype7," ");
	WriteFloatToLine(LineBuff,(double)(*((*pp_CsoundScore)[j]))[ievent].duration
											* (*p_CsoundTempo)[j] / 60000.);
	PrintBehind(wPrototype7,LineBuff); PrintBehind(wPrototype7," ");
	if((*((*pp_CsoundScore)[j]))[ievent].h_param != NULL) {
		for(k=0; k < (*((*pp_CsoundScore)[j]))[ievent].nbparameters; k++) {
			param = (*(*((*pp_CsoundScore)[j]))[ievent].h_param)[k];
			ip = k + 4;
			if(ip == (*p_CsPitchBendStartIndex)[ins] || ip == (*p_CsPitchBendEndIndex)[ins]) {
				param = XtoY(param,&((*p_CsInstrument)[ins].rPitchBend),&overflow,
					instrumentindex);
				if(overflow) {
					result = ABORT; goto SORTIR;
					}
				}
			if(ip == (*p_CsVolumeStartIndex)[ins] || ip == (*p_CsVolumeEndIndex)[ins]) {
				param = XtoY(param,&((*p_CsInstrument)[ins].rVolume),&overflow,
					instrumentindex);
				if(overflow) {
					result = ABORT; goto SORTIR;
					}
				}
			if(ip == (*p_CsPressureStartIndex)[ins] || ip == (*p_CsPressureEndIndex)[ins]) {
				param = XtoY(param,&((*p_CsInstrument)[ins].rPressure),&overflow,
					instrumentindex);
				if(overflow) {
					result = ABORT; goto SORTIR;
					}
				}
			if(ip == (*p_CsModulationStartIndex)[ins] || ip == (*p_CsModulationEndIndex)[ins]) {
				param = XtoY(param,&((*p_CsInstrument)[ins].rModulation),&overflow,
					instrumentindex);
				if(overflow) {
					result = ABORT; goto SORTIR;
					}
				}
			if(ip == (*p_CsPanoramicStartIndex)[ins] || ip == (*p_CsPanoramicEndIndex)[ins]) {
				param = XtoY(param,&((*p_CsInstrument)[ins].rPanoramic),&overflow,
					instrumentindex);
				if(overflow) {
					result = ABORT; goto SORTIR;
					}
				}
			WriteFloatToLine(LineBuff,param);
			PrintBehind(wPrototype7,LineBuff); PrintBehind(wPrototype7," ");
			}
		}
	PrintBehind(wPrototype7,"\n");
	}
PrintBehind(wPrototype7,"\ne\n");

SORTIR:

if(PointToDuration(NULL,pp_CsoundTime,p_CsoundSize,j) != OK) result = ABORT;
if(result == OK) result = CompileObjectScore(j,&longerCsound);

if(CompileOn) CompileOn--;

return(result);
}

int FindCsoundInstrument(char* line) {
	int i,j,isnumber;

	Strip(line);
	if(line[0] == '\0') {
		isnumber = FALSE; goto ERR;
		}
	isnumber = TRUE;
	for(i=0; i < strlen(line); i++) {
		if(!isdigit(line[i])) {
			isnumber = FALSE; break;
			}
		}
	if(isnumber) {
		i = (int) atol(line);
		for(j=0; j < Jinstr; j++) {
	//		BPPrintMessage(0,odInfo,"InstrumentIndex = %d, j = %d\n",(*p_CsInstrumentIndex)[j],j);
			if(i == (*p_CsInstrumentIndex)[j]) break;
			}
		if(j < Jinstr) return(i);
		goto ERR;
		}
	for(j=0; j < Jinstr; j++) {
	//	BPPrintMessage(0,odInfo,"InstrumentName = %s, j = %d\n",(*(*pp_CsInstrumentName)[j]),j);
		if(Mystrcmp((*pp_CsInstrumentName)[j],line) == 0) break;
		}
	if(j < Jinstr) return((*p_CsInstrumentIndex)[j]);

	ERR:
	if(Jinstr < 2) {
		Print(wTrace,"\n=> You probably forgot to create or load a '-cs' instrument file\n");
		}
	if(isnumber) my_sprintf(Message,"=> Instrument %ld was not found in the '-cs' instrument file\n",
		(long) i);
	else my_sprintf(Message,"=> Instrument \"%s\" was not found in the '-cs' instrument file\n",
		line);
	BPPrintMessage(0,odError,Message);
	return(ABORT);
	}


int ResetMoreParameter(int j,int ip) {
	char **ptr;
	Handle h;
	CsoundParam **paramlist;

	paramlist = (*p_CsInstrument)[j].paramlist;

	if(paramlist == NULL) {
		BPPrintMessage(0,odError,"=> Err. ResetMoreParameter(). paramlist == NULL");
		return(OK);
		}
	h = (Handle) (*paramlist)[ip].name;
	MyDisposeHandle(&h);
	(*paramlist)[ip].name = NULL;
	h = (Handle) (*paramlist)[ip].comment;
	MyDisposeHandle(&h);
	(*paramlist)[ip].comment = NULL;

	(*paramlist)[ip].startindex
		= (*paramlist)[ip].endindex
		= (*paramlist)[ip].nameindex
		= (*paramlist)[ip].table = -1;

	(*paramlist)[ip].defaultvalue = 0.;
	(*paramlist)[ip].GENtype = 7;
	(*paramlist)[ip].combinationtype = ADD;
	return(OK);
	}

