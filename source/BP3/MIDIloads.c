/* MIDIloads.c (BP3) */ 

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

int PrintNote(int i_scale,int key,int channel,int wind,char* line) {
	// wind is not used
	int pitchclass, octave;
	char channelstring[20], jscale;
	if(key < 0) {
		strcpy(line,"<void>");
		return(OK);
		}
	channelstring[0] = '\0';
	if(channel > 0) my_sprintf(channelstring," channel %ld",(long)channel);
	// BPPrintMessage(0,odInfo,"i_scale = %d key =  %d NumberScales = %d NoteConvention = %d\n",i_scale,key,NumberScales,NoteConvention);

	if(i_scale > NumberScales) {
		BPPrintMessage(0,odError,"=> Error: i_scale (%ld) > NumberScales (%d)\n",(long)i_scale,NumberScales);
		return(OK);
		}

	if(i_scale > 0) {
		int keyclass;
		int basekey = (*Scale)[i_scale].basekey;
		int numgrades = (*Scale)[i_scale].numgrades;
		int numnotes = (*Scale)[i_scale].numnotes;
		int baseoctave = (*Scale)[i_scale].baseoctave;
		if(numgrades <= 12) {
			keyclass = modulo(key - basekey, 12);
			octave = baseoctave + floor(((double)key - basekey) / 12);
			}
		else {
			int i_note = modulo(key - basekey, numnotes);
			keyclass = (*((*Scale)[i_scale].keyclass))[i_note];
			octave = baseoctave + floor((((double)key - basekey)) / numnotes);
			}
		my_sprintf(line,"%s%d%s",*((*(*Scale)[i_scale].notenames)[keyclass]),octave,channelstring);
	//	my_sprintf(line,"%s%s",*((*(p_NoteName[jscale]))[key]),channelstring);
		}	
	else {	
	/*	key -= (C4key - 60);
		pitchclass = modulo(key,12); */
		pitchclass = modulo((key - C4key),12);
		octave = (key - pitchclass) / 12;
		if(NameChoice[pitchclass] == 1 && pitchclass == 0) octave--;
		if(NameChoice[pitchclass] == 1 && pitchclass == 11) octave++;
		switch(NoteConvention) {
			case FRENCH:
				octave -= 2;
				switch(octave) {
					case -2:
						if(NameChoice[pitchclass] == 0)
							my_sprintf(line,"%s000%s",Frenchnote[pitchclass],channelstring);
						else
							my_sprintf(line,"%s000%s",AltFrenchnote[pitchclass],channelstring);
						break;
					case -1:
						if(NameChoice[pitchclass] == 0)
							my_sprintf(line,"%s00%s",Frenchnote[pitchclass],channelstring);
						else
							my_sprintf(line,"%s00%s",AltFrenchnote[pitchclass],channelstring);
						break;
					default:
						if(NameChoice[pitchclass] == 0)
							my_sprintf(line,"%s%ld%s",Frenchnote[pitchclass],(long)octave,channelstring);
						else
							my_sprintf(line,"%s%ld%s",AltFrenchnote[pitchclass],(long)octave,channelstring);
						break;
					}
				break;
			case ENGLISH:
				octave--;
				switch(octave) {
					case -1:
						if(NameChoice[pitchclass] == 0)
							my_sprintf(line,"%s00%s",Englishnote[pitchclass],channelstring);
						else
							my_sprintf(line,"%s00%s",AltEnglishnote[pitchclass],channelstring);
						break;
					default:
						if(NameChoice[pitchclass] == 0)
							my_sprintf(line,"%s%ld%s",Englishnote[pitchclass],(long)octave,channelstring);
						else
							my_sprintf(line,"%s%ld%s",AltEnglishnote[pitchclass],(long)octave,channelstring);
						break;
					}
				break;
			case INDIAN:
				octave--;
				switch(octave) {
					case -1:
						if(NameChoice[pitchclass] == 0)
							my_sprintf(line,"%s00%s",Indiannote[pitchclass],channelstring);
						else
							my_sprintf(line,"%s00%s",AltIndiannote[pitchclass],channelstring);
						break;
					default:
						if(NameChoice[pitchclass] == 0)
							my_sprintf(line,"%s%ld%s",Indiannote[pitchclass],(long)octave,channelstring);
						else
							my_sprintf(line,"%s%ld%s",AltIndiannote[pitchclass],(long)octave,channelstring);
						break;
					}
				break;
			default: // This includes 'KEYS'
				my_sprintf(line,"%s%ld%s",KeyString,(long)key,channelstring);
				break;
			}
		}
	trim_digits_after_key_hash(line); // Remove the octave number after key#xx
	return(OK);
	}


int GetNote(char* line,int* p_thekey,int* p_channel,int ignorechannel)
{
char *p,*q,line2[MAXLIN];
int i,j,pitchclass,octave,l;

i = j = 0; while(MySpace(line[i])) i++;
strcpy(line2,line);
if(NoteConvention == KEYS) {
	while(line[i] != '\0' && line[i] == KeyString[j]) {
		i++; j++;
		}
	if(KeyString[j] != '\0') return(MISSED);
	*p_thekey = GetInteger(YES,line2,&i);
	if(*p_thekey == INT_MAX) return(MISSED);
	}
else {
	for(pitchclass=0; pitchclass < 12; pitchclass++) {
		p = &line2[i];
		switch(NoteConvention) {
			case FRENCH: {
				q = &Frenchnote[pitchclass][0]; l = strlen(Frenchnote[pitchclass]);
				if(l > 0 && Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				p = &line2[i];
				q = &AltFrenchnote[pitchclass][0]; l = strlen(AltFrenchnote[pitchclass]);
				if(l > 0 && Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				break;
				}
			case ENGLISH: {
				q = &Englishnote[pitchclass][0]; l = strlen(Englishnote[pitchclass]);
				if(l > 0 && Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				p = &line2[i];
				q = &AltEnglishnote[pitchclass][0]; l = strlen(AltEnglishnote[pitchclass]);
				if(l > 0 && Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				break;
				}
			case INDIAN:
				q = &Indiannote[pitchclass][0]; l = strlen(Indiannote[pitchclass]);
				if(l > 0 && Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				p = &line2[i];
				q = &AltIndiannote[pitchclass][0]; l = strlen(AltIndiannote[pitchclass]);
				if(l > 0 && Match(TRUE,&p,&q,l) && isdigit(p[l])) goto CONT;
				break;
			}
		}
	return(MISSED);
CONT:
	while(!isdigit(line[i]) && line[i] != '\0') i++;
	if(NoteConvention == FRENCH) {
		if(line[i] == '0' && line[i+1] == '0' && line[i+2] == '0') {
			octave = 0; i += 3;
			goto CONT2;
			}
		if(line[i] == '0' && line[i+1] == '0') {
			octave = 1; i += 2;
			goto CONT2;
			}
		}
	if(NoteConvention == ENGLISH || NoteConvention == INDIAN) {
		if(line[i] == '0' && line[i+1] == '0') {
			octave = 0; i += 2;
			goto CONT2;
			}
		}
	if((octave=GetInteger(YES,line2,&i)) == INT_MAX) return(MISSED);
	if(NoteConvention == FRENCH) octave += 2;
	if(NoteConvention == ENGLISH || NoteConvention == INDIAN) octave++;
	
CONT2:
	*p_thekey = 12 * octave + pitchclass;
	*p_thekey += (C4key - 60);
//	BPPrintMessage(0,odInfo,"\nkey = %d Englishnote[pitchclass] = %s AltEnglishnote[pitchclass] = %s pitchclass = %d octave = %d NameChoice[pitchclass] = %d\n",*p_thekey,Englishnote[pitchclass],pitchclass,octave,NameChoice[pitchclass]);
	}
if(ignorechannel) return(OK);
while(MySpace(line[i])) i++;
strcpy(Message,"channel");
p = &line2[i]; q = &(Message[0]);
if(!Match(FALSE,&p,&q,strlen(Message))) return(MISSED);
while(!isdigit(line[i]) && line[i] != '\0') i++;
if((*p_channel=GetInteger(YES,line2,&i)) == INT_MAX) return(MISSED);
return(OK);
}
