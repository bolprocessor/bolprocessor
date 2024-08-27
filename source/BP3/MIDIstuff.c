/* MIDIstuff.c (BP3) */ 

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


#include "-BP2.h"
#include "-BP2decl.h"

int trace_midi_filter = 0;
int trace_driver = 0;

int read_midisetup() {
    char *itemType, *itemIndex, *itemNumber, *portName, *portComment, line[MAXLIN];
	char *key, *value;
    FILE *file;
	unsigned long long_value;
	int i, int_value, index;
	size_t len;
    int result = FALSE;
	MaxInputPorts = 0;
	MaxOutputPorts = 1;
	for(i = 0; i < MAXPORTS; i++) {
		strcpy(MIDIinputname[i],"");
		strcpy(MIDIoutputname[i],"");
		strcpy(OutputMIDIportComment[i],"");
		strcpy(InputMIDIportComment[i],"");
		MIDIoutput[i] = -1;  MIDIinput[i] = -1;
		}
    file = my_fopen(0,Midiportfilename,"r");
    if(file != NULL) {
        BPPrintMessage(odInfo,"Reading MIDI port settings: %s\n",Midiportfilename);
        while(fgets(line, sizeof(line), file) != NULL) {
			remove_carriage_returns(line);
		//	BPPrintMessage(odInfo,"%s\n",line);
            itemType = strtok(line, "\t");
            itemIndex = strtok(NULL, "\t");
            itemNumber = strtok(NULL, "\t");
            portName = strtok(NULL, "\t");
            portComment = strtok(NULL, "\n");
			strip_newline(itemType);
			strip_newline(itemIndex);
			strip_newline(itemNumber);
			strip_newline(portName);
			strip_newline(portComment);
			index = atoi(itemIndex);
			if(index >= MAXPORTS) {
				BPPrintMessage(odError,"=> ERROR: Incorrect index %d on this line: %s\n",index,line);
				break;
				}
            if(!portName) portName = "";
			if(portName && strlen(portName) >= (MAXNAME - 1)) {
				BPPrintMessage(odError,"=> ERROR: Name is longer than %d on this line: %s\n",MAXNAME,line);
				break;
				}
          //  BPPrintMessage(odInfo,"index = %d, itemType = %s, itemIndex = %s, itemNumber = %s, portName = %s, portComment = %s\n",index, itemType,itemIndex,itemNumber,portName,portComment);
            if(strcmp(itemType,"MIDIinput") == 0) {
				if(itemNumber && strlen(itemNumber) > 0) {
					MIDIinput[index] = atoi(itemNumber);
					if(strlen(portName) > 0) strcpy(MIDIinputname[index],portName);
					else strcpy(MIDIinputname[index],"???");
					if(strlen(portComment) > 0) strcpy(InputMIDIportComment[index],portComment);
					else strcpy(InputMIDIportComment[index],"");
					if((index + 1) > MaxInputPorts) MaxInputPorts = index + 1;
					}
				else {
					MIDIinput[index] = -1;
			//		BPPrintMessage(odInfo,"MIDIinput[%d] = -1\n",index);
					}
				}	
			if(strcmp(itemType,"MIDIoutput") == 0) {
				if(itemNumber && strlen(itemNumber) > 0) {
					MIDIoutput[index] = atoi(itemNumber);
					if(strlen(portName) > 0) strcpy(MIDIoutputname[index],portName);
					else strcpy(MIDIoutputname[index],"???");
					if(strlen(portComment) > 0) strcpy(OutputMIDIportComment[index],portComment);
					else strcpy(OutputMIDIportComment[index],"");
					if((index + 1) > MaxOutputPorts) MaxOutputPorts = index + 1;
					result = OK; // We need at least one MIDI output port
					}
				else MIDIoutput[index] = -1;
				}
			if(strcmp(itemType,"MIDIacceptFilter") == 0) {
        //        BPPrintMessage(odInfo,"MIDIacceptFilter = %s\n",itemNumber);
        		if(itemNumber && strlen(itemNumber) > 0) {
					long_value = 0L;
            		for(i = 0; i < 18; i++) {
						if(itemNumber[i] != '0' && itemNumber[i] != '1') {
							BPPrintMessage(odError,"Non-binary digit found at location %d in filter: “%s”\nLine: %s\n",i,itemNumber,line);
							return FALSE;
							}
                		long_value = (long_value * 2L) + (itemNumber[i] - '0');
            			}
					MIDIacceptFilter[index] = long_value;
					GetInputFilterWord(index);
					}
				}
			if(strcmp(itemType,"MIDIpassFilter") == 0) {
        		if(itemNumber && strlen(itemNumber) > 0) {
					long_value = 0L;
            		for(i = 0; i < 18; i++) {
						if(itemNumber[i] != '0' && itemNumber[i] != '1') {
							BPPrintMessage(odError,"Non-binary digit found at location %d in filter: “%s”\nLine: %s\n",i,itemNumber,line);
							return FALSE;
							}
                		long_value = (long_value * 2L) + (itemNumber[i] - '0');
            			}
					MIDIpassFilter[index] = long_value;
				//	BPPrintMessage(odInfo,"Lu: MIDIpassFilter[%d] = %ld = %s\n",index,MIDIpassFilter[index],itemNumber);
					GetOutputFilterWord(index);
					}
        		}
			if(strcmp(itemType,"MIDIchannelFilter") == 0) {
       			if(itemNumber && strlen(itemNumber) > 0) {
					strcpy(MIDIchannelFilter[index],itemNumber);
				//	BPPrintMessage(odInfo,"Lu: MIDIchannelFilter[%d] = %s\n",index,MIDIchannelFilter[index]);
					}
        		}
			if(strcmp(itemType,"MIDIoutFilter") == 0) {
            //  BPPrintMessage(odInfo,"MIDIoutFilter = %s\n",itemNumber);
				if(itemNumber && strlen(itemNumber) > 0) {
					strcpy(MIDIoutFilter[index],itemNumber);
			//		BPPrintMessage(odInfo,"Lu: MIDIoutFilter[%d] = %s\n",index,MIDIoutFilter[index]);
					}
				}
			}
        my_fclose(file);
		BPPrintMessage(odInfo,"🎹 Your real-time MIDI settings:\n");
		for(index = 0; index < MaxOutputPorts; index++) {
			if(strcmp(OutputMIDIportComment[index],"void") == 0) strcpy(line,"");
			else strcpy(line,OutputMIDIportComment[index]);
			BPPrintMessage(odInfo,"MIDI output = %d: “%s” - %s\n",MIDIoutput[index],MIDIoutputname[index],line);
			}
		for(index = 0; index < MaxInputPorts; index++) {
			if(strcmp(InputMIDIportComment[index],"void") == 0) strcpy(line,"");
			else strcpy(line,InputMIDIportComment[index]);
			BPPrintMessage(odInfo,"MIDI input = %d: “%s” - %s\n",MIDIinput[index],MIDIinputname[index],line);
			}
		BPPrintMessage(odInfo,"\n");
        }
    return(result);
    }

void strip_newline(char *str) {
    if(str) {
        size_t len = strlen(str);
        while(len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
            str[len - 1] = '\0';
            len--;
			}
		}
	}
	
void save_midisetup() {
    FILE* thefile;
	char* binaryString;
	int index;
    thefile = my_fopen(1,Midiportfilename,"w");
    if(thefile != NULL) {
        BPPrintMessage(odInfo,"\nMIDI settings saved to %s\n",Midiportfilename);
		for(index = 0; index < MaxOutputPorts; index++) {
			if(strlen(MIDIoutputname[index]) == 0) continue;
            if(strlen(OutputMIDIportComment[index]) == 0)
                strcpy(OutputMIDIportComment[index],"void");
			fprintf(thefile, "MIDIoutput\t%d\t%d\t%s\t%s\n",index,MIDIoutput[index],MIDIoutputname[index],OutputMIDIportComment[index]);
		//	BPPrintMessage(odInfo,"Ecrit: MIDIoutput[%d] = %d\t%s\t%s\n",index,MIDIoutput[index],MIDIoutputname[index],OutputMIDIportComment[index]);
		//	BPPrintMessage(odInfo,"Ecrit: MIDIoutFilter[%d] = %s\n",index,MIDIoutFilter[index]);
			fprintf(thefile, "MIDIoutFilter\t%d\t%s\n",index,MIDIoutFilter[index]);
		//	BPPrintMessage(odInfo,"Ecrit: MIDIchannelFilter[%d] = %s\n",index,MIDIchannelFilter[index]);
        	fprintf(thefile, "MIDIchannelFilter\t%d\t%s\n",index,MIDIchannelFilter[index]);
			}
		for(index = 0; index < MaxInputPorts; index++) {
			if(strlen(MIDIinputname[index]) == 0) continue;
            if(strlen(InputMIDIportComment[index]) == 0)
                strcpy(InputMIDIportComment[index],"void");
			fprintf(thefile, "MIDIinput\t%d\t%d\t%s\t%s\n",index,MIDIinput[index],MIDIinputname[index],InputMIDIportComment[index]);
			SetInputFilterWord(index);
			binaryString = longToBinary(18,(unsigned long)MIDIacceptFilter[index]);
        	fprintf(thefile, "MIDIacceptFilter\t%d\t%s\n",index,binaryString);
			free(binaryString);
			SetOutputFilterWord(index);
			binaryString = longToBinary(18,(unsigned long)MIDIpassFilter[index]);
        	fprintf(thefile, "MIDIpassFilter\t%d\t%s\n",index,binaryString);
			free(binaryString);
			}
        my_fclose(thefile);
        }
    }

char* longToBinary(int totalBits,unsigned long num) {
    char *binary = (char *)malloc(totalBits + 1); // Allocate memory for 18 bits plus null terminator
    unsigned long mask = 1UL << (totalBits - 1);
    for(int i = 0; i < totalBits; i++, mask >>= 1) {
        binary[i] = (num & mask) ? '1' : '0'; // Set each character in the array
    	}
    binary[totalBits] = '\0'; // Null-terminate the string
    return binary; // Return the binary string
	}

int MIDIflush() {
    unsigned long current_time,time_now;
    long i = 0;
    long time;
    unsigned char midiData[4];
    int dataSize = 3;
    int result,size, i_scale, blockkey, type;
    size = sizeof(MIDI_Event);
	current_time = getClockTime();
    current_time -= initTime;
    if(Panic) eventCount = 0L;
    if((result = stop(0,"MIDIflush")) != OK) {
        eventCount = 0L;
        return result;
        }
    while(i < eventCount) {
		if((result = MaybeWait(current_time)) != OK) return result;
        if((eventStack[i].time + TimeStopped) <= current_time) {
            midiData[0] = eventStack[i].status;
            midiData[1] = eventStack[i].data1;
            midiData[2] = eventStack[i].data2;
	/*		i_scale = eventStack[i].i_scale;
			blockkey = eventStack[i].blockkey; */
			type = eventStack[i].status & 0xF0;
			if(type != NoteOn && type != NoteOff) i_scale = blockkey = 0;
i_scale = blockkey = 0;
            time = (eventStack[i].time + TimeStopped);
        //	if(type == NoteOn || type == NoteOff) BPPrintMessage(odInfo,"§ type %d Note %d value %d time %ld\n",type,eventStack[i].data1,midiData[2],(long)time); 
		//	BPPrintMessage(odInfo,"§ type %d: status = %d data1 = %d data2 = %d, time %ld ms\n",type,midiData[0],midiData[1],midiData[2],(long)time/1000L); 
            sendMIDIEvent(i_scale,blockkey,midiData,dataSize,time);
            // Move remaining events forward
            memmove(&eventStack[i], &eventStack[i + 1], (eventCount - i - 1) * size);
            eventCount--;
            }
        else i++;
        }
	if((result = MaybeWait(current_time)) != OK) return result;
    return OK;
    }


int MaybeWait(unsigned long current_time) {
	unsigned long time,time_now,i;
	int result;
	if(FirstNoteOn) time = 0L;
	else time = current_time;
	check_stop_instructions(time); // This may set StopPlay to TRUE
	time_now = getClockTime(); // microseconds
	i = 0L;
	while(StopPlay) { // The proper input MIDI event will end this loop, setting StopPlay to FALSE
		if((result = stop(1,"Waiting loop")) != OK) return result;
		WaitABit(5); // milliseconds
		i++;
		if(i == 100L && ResetNotes) AllNotesOffPedalsOffAllChannels();
		}
	TimeStopped += (getClockTime() - time_now);
	if((TimeStopped / 10000L) != (Oldtimestopped / 10000L)) {
		if(TraceMIDIinteraction) BPPrintMessage(odInfo,"TimeStopped = %d ms\n",(int) TimeStopped / 1000L);
		}
	Oldtimestopped = TimeStopped;
	return OK;
	}


int HandleInputEvent(const MIDIPacket* packet,MIDI_Event* e,int index) {
	int x0,x1,x2,filter,c,c0,c1,c2,channel,i,j,jj,r,idummy,eventfound;
	long jdummy;
	unsigned long time_now,thisscripttime;
	char **p_line;

	x0 = x1 = x2 = 0;
	filter = x0 + x1 + x2;  // Will be used later

	STARTCHECK:
	if(packet == NULL) return OK;
	// if(!AcceptEvent(ByteToInt(packet->data[0]),index)) return OK;
	// This is redundant because acceptance has already be checked at the input
	if (packet->length > 0) {
		e->type = packet->data[0];  // Assuming data[0] is the status byte
	//	e->time = packet->timestamp;
		}
	if (packet->length > 1)
		e->data1 = packet->data[1];  // Assuming data[1] is the first data byte
	else e->data1 = 0;  // No data available
	
	if (packet->length > 2)
		e->data2 = packet->data[2];  // Assuming data[2] is the second data byte
	else
		e->data2 = 0;  // No data available
	c0 = e->type;
	channel = (c0 % 16) + 1;
	c = c0 - channel + 1;
	if(packet->length > 1 && !ThreeByteChannelEvent(c)) {  // REVISE THIS!
		RunningStatus = 0;
		if(c0 < 128) return(OK);
		if(c0 == SystemExclusive) {
			do {
				e->time = 0;
				e->type = RAW_EVENT;
				e->data2 = c0;
				c0 = e->data2;
				}
			while(c0 != EndSysEx && e->type != NULL_EVENT);
			e->time = 0;
			e->type = RAW_EVENT;
			e->data2 = c0;
			return(OK);
			}
		if(c0 == SongPosition) {
			for(i=0; i < 2; i++) {
				e->time = 0;
				e->type = RAW_EVENT;
				e->data2 = c0;
				c0 = e->data2;
				}
			e->time = 0;
			e->type = RAW_EVENT;
			e->data2 = c0;
			return(OK);
			}
		if(c0 == SongSelect) {
			e->time = 0;
			e->type = RAW_EVENT;
			e->data2 = c0;
			c0 = e->data2;
			e->time = 0;
			e->type = RAW_EVENT;
			e->data2 = c0;
			return(OK);
			}
		if(c == ProgramChange) {
			e->time = 0;
			e->type = RAW_EVENT;
			e->data2 = c0;
			my_sprintf(Message,"%ld",(long)(e->data2) + ProgNrFrom);
			if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
			e->time = 0;
			e->type = RAW_EVENT;
			my_sprintf(Message,"%ld",(long)((c0 % 16) + 1));
			if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,1,Message);
		/*	if(!ScriptExecOn) AppendScript(71); */
			return(OK);
			}
		if(c == ChannelPressure) {
			e->type = RAW_EVENT;
			c1 = e->data2;
			e->time = 0;
			e->type = TWO_BYTE_EVENT;
			e->status = c;
			if(Jcontrol < 0) {
			/*	my_sprintf(Message,"Pressure = %ld channel %ld",(long)c1,
					(long)(c0 - c + 1));
				if(Interactive && ShowMessages) ShowMessage(TRUE,wMessage,Message); */
				if(Interactive) {
					(*p_Oldvalue)[c0-c].pressure = c1;
					ChangedPressure[c0-c] = TRUE;
					}
				}
			return(OK);
			}
		e->time = 0;
		e->type = RAW_EVENT;
		e->data2 = c0;
		return(OK);
		}
	switch(c0) {
		case Start:
		case Continue:
		case Stop:
			StopWaiting(c0,'\0');
			return(OK);
			break;
		}
	RunningStatus = c0;
	c1 = e->data1;
	c2 = e->data2;

	INTERPRET:
	c = c0 - c0 % 16;
	if(filter && c2 != 0 && (x0 == 0 || x0 == c0) && (x1 == 0 || x1 == c1)
				&& (x2 == 0 || x2 == c2)) return(RESUME);

	if((Interactive || ScriptRecOn) && c == ChannelMode && c1 > 121) {
		strcpy(LineBuff,"");
		my_sprintf(Message,"%ld",(long)(c0 - c + 1));
		if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
		switch(c1) {
			case 122:
				if(c2 == 0) {	/* Local control off */
					AppendScript(76); my_sprintf(LineBuff,"Local control off channel %ld",
						(long)(c0-c+1));
					break;
					}
				if(c2 == 127) {	/* Local control on */
					AppendScript(77); my_sprintf(LineBuff,"Local control on channel %ld",
						(long)(c0-c+1));
					break;
					}
				break;
			case 123: /* if(c2 == 0) AppendScript(78) */ break;	/* All notes off */
			case 124: if(c2 == 0) {	/* Omni mode off */
				AppendScript(79); my_sprintf(LineBuff,"Omni mode off channel %ld",(long)(c0-c+1));
				break;
				}
			case 125: if(c2 == 0) {	/* Omni mode on */
				AppendScript(80); my_sprintf(LineBuff,"Omni mode on channel %ld",(long)(c0-c+1));
				break;
				}
			case 127: if(c2 == 0) {	/* Poly mode on */
				AppendScript(82); my_sprintf(LineBuff,"Poly mode on channel %ld",(long)(c0-c+1));
				break;
				}
			case 126:	/* Mono mode on, c2 channels */
				my_sprintf(LineBuff,"Mono mode on (%ld channels) channel %ld",(long)c2,
					(long)(c0-c+1));
				my_sprintf(Message,"%ld",(long)(c0 - c + 1));
				if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,1,Message);
				my_sprintf(Message,"%ld",(long)c2);
				if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
				AppendScript(81);
				break;
			}
		if(LineBuff[0] != '\0' && ShowMessages) ShowMessage(TRUE,wMessage,LineBuff);
		return(OK);
		}
	if(Interactive || ScriptRecOn || ReadKeyBoardOn) {
		if(c == PitchBend) {
			if(Jcontrol == -1) {
			/*	my_sprintf(Message,"Pitchbend = %ld channel %ld",(long) c1 + 128L*c2,
					(long)(c0 - c + 1));
				if(ShowMessages) ShowMessage(TRUE,wMessage,Message); */
				if(Interactive) {
					(*p_Oldvalue)[c0-c].pitchbend = c1 + (128L * c2);
					ChangedPitchbend[c0-c] = TRUE;
					}
				}
			return(OK);
			}
		if(c == ControlChange) {
			if(c1 == VolumeControl[c0-c+1]) {	/* Volume */
				if(Jcontrol == -1) {
			/*		my_sprintf(Message,"Volume = %ld channel %ld",(long)c2,
						(long)(c0 - c + 1));
					if(ShowMessages) ShowMessage(TRUE,wMessage,Message); */
					if(Interactive) {
						(*p_Oldvalue)[c0-c].volume = c2;
						ChangedVolume[c0-c] = TRUE;
						}
					}
				return(OK);
				}
			else
			if(c1 == PanoramicControl[c0-c+1]) {	/* Panoramic */
				if(Jcontrol == -1) {
			/*		my_sprintf(Message,"Panoramic = %ld channel %ld",(long)c2,
						(long)(c0 - c + 1));
					if(ShowMessages) ShowMessage(TRUE,wMessage,Message); */
					if(Interactive) {
						(*p_Oldvalue)[c0-c].panoramic = c2;
						ChangedPanoramic[c0-c] = TRUE;
						}
					}
				return(OK);
				}
			else
			if(c1 == 1) {	/* Modulation MSB */
				OldModulation = (long) 128L * c2;
			/*	my_sprintf(Message,"Modulation = %ld channel %ld",(long)OldModulation,
					(long)(c0 - c + 1));
				if(ShowMessages && Jcontrol == -1) ShowMessage(TRUE,wMessage,Message); */
				return(OK);
				}
			else
			if(c1 == 33) {	/* Modulation LSB */
				if(Jcontrol == -1) {
			/*		my_sprintf(Message,"Modulation = %ld channel %ld",(long)(c2+OldModulation),
						(long)(c0 - c + 1));
					if(ShowMessages) ShowMessage(TRUE,wMessage,Message); */
					if(Interactive) {
						(*p_Oldvalue)[c0-c].modulation = c2 + OldModulation;
						ChangedModulation[c0-c] = TRUE;
						}
					}
				return(OK);
				}
			else
			if(c1 >= 64 && c1 <= 95) {
				if(!ScriptExecOn) {
					my_sprintf(Message,"%ld",(long)(c0-c+1));
					MystrcpyStringToTable(ScriptLine.arg,1,Message);
					}
				if(c2 == 0) {
					my_sprintf(LineBuff,"Switch %ld OFF channel %ld",(long)c1,(long)(c0-c+1));
					my_sprintf(Message,"%ld",(long)c1);
					if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
					AppendScript(87);
					}
				else {
					my_sprintf(LineBuff,"Switch %ld ON channel %ld",(long)c1,(long)(c0-c+1));
					my_sprintf(Message,"%ld",(long)c1);
					if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
					AppendScript(86);
					}
			//	if(ShowMessages && Jcontrol == -1) ShowMessage(TRUE,wMessage,LineBuff);
				return(OK);
				}
			}
		}
	if(Interactive && (Ctrl_adjust(e,c0,c1,c2) == OK)) {
/*		if(e->type == NULL_EVENT) return(OK); // Suppressed 2024-06-19
		RunningStatus = 0;
		c0 = e->data2;
		goto STARTCHECK; */
		}
//	BPPrintMessage(odInfo,"Handling? c0 = %d, c1 = %d, c2 = %d, Interactive = %d\n",c0,c1,c2,Interactive);
	if(Interactive && (c2 > 0)) {
		if(EndRepeatChan > 0 && c1 == EndRepeatKey && c0 == (NoteOn+EndRepeatChan-1)) {
			Nplay = 1; SynchroSignal = OFF;
			ShowMessage(TRUE,wMessage,"Stop repeating!");
			return(ENDREPEAT);
			}
		else {
		if(MuteOnChan > 0 && c1 == MuteOnKey && c0 == (NoteOn+MuteOnChan-1)) {
			Mute = TRUE;
			ShowMessage(TRUE,wMessage,"Received MIDI message telling to mute output");
			FlashInfo("MUTE is ON...   cmd-space will turn if off");
			return(OK);
			}
		else {
		if(MuteOffChan > 0 && c1 == MuteOffKey && c0 == (NoteOn+MuteOffChan-1)) {
			Mute = FALSE;
			ShowMessage(TRUE,wMessage,"Received MIDI message telling to stop muting output");
			// HideWindow(Window[wInfo]);
			return(OK);
			}
		else {
		if(QuitChan > 0 && c1 == QuitKey && c0 == (NoteOn+QuitChan-1)) {
			SkipFlag = FALSE;
			ShowMessage(TRUE,wMessage,"Abort!");
			return(ABORT);
			}
		else {
		if(PlayChan > 0 && c1 == PlayKey && c0 == (NoteOn+PlayChan-1)) {
			SynchroSignal = PLAYNOW;
			ShowMessage(TRUE,wMessage,"Play now!");
			return(QUICK);
			/* Used to interrupt TimeSet() and play immediately */
			}
		else {
		if(EverChan > 0 && c1 == EverKey && c0 == (NoteOn + EverChan - 1)) {
			SynchroSignal = PLAYFOREVER;
			ClearMessage();
			Print(wMessage,"Play forever! (");
			PrintNote(-1,EndRepeatKey,EndRepeatChan,wMessage,Message);
			Print(wMessage," will stop)");
			return(OK);
			}
		else {
		if(RepeatChan > 0 && c1 == RepeatKey && c0 == (NoteOn + RepeatChan -1)) {
			Nplay = c2;
			SynchroSignal = OFF;
			ClearMessage();
			my_sprintf(Message,"Playing %ld times... (",(long)Nplay);
			Print(wMessage,Message);
			PrintNote(-1,EndRepeatKey,EndRepeatChan,wMessage,Message);
			Print(wMessage," will stop)");
			return(OK);
			}
		else {
		if(AgainChan > 0 && c1 == AgainKey && c0 == (NoteOn+AgainChan-1)) {
			ShowMessage(TRUE,wMessage,"Play again!");
			return(AGAIN);
			}
		else {
		if(DeriveFurtherChan > 0 && c1 == DeriveFurtherKey && c0 == (NoteOn+
				DeriveFurtherChan-1)) {
			DeriveFurther = 1 - DeriveFurther;
			if(DeriveFurther)
				ShowMessage(TRUE,wMessage,"Derive further flag ON");
			else
				ShowMessage(TRUE,wMessage,"Derive further flag OFF");
			return(OK);
			}
		else {
		if(Jinscript > 0) {
			if(FirstNoteOn) {
				if(TraceMIDIinteraction) BPPrintMessage(odError,"time_now = 0L in HandleInputEvent()\n");
				time_now = 0L;
				}
			else time_now = getClockTime() - initTime; // microseconds
	//		if(TraceMIDIinteraction) BPPrintMessage(odInfo,"Received NoteOn key = %d channel %d date %ld ms, checking %d script(s)\n",c1,channel,time_now / 1000L,Jinscript);
			// Find the next expected NoteOn
			for(j = 1; j <= Jinscript; j++) {
				if(((*p_INscript)[j]).chan == -1) { // This is a deactivated instruction
			//		if(j == Jinscript) Jinscript = 0; // No need to try later
					continue;
					}
				thisscripttime = ((*p_INscript)[j]).time + TimeStopped;
		//		thisscripttime = ((*p_INscript)[j]).time + Oldtimestopped;
				// We won't verify that velocity (c2) is greater than zero, because a NoteOn with velocity zero can be used as a soundless instruction
				if(channel == ((*p_INscript)[j]).chan && c1 == ((*p_INscript)[j]).key && time_now > thisscripttime) {
					if(TraceMIDIinteraction)
						BPPrintMessage(odInfo, "[%d] Good NoteOn key = %d, time_now = %ld ms, thisscripttime = %ld ms\n", j, c1, time_now / 1000L, thisscripttime / 1000L);
				//	my_sprintf(Message,"On key = %d at date %ld ms",c1,time_now/1000L);
				//	Notify(Message,0);
					strcpy(Message,"");
					StopPlay = FALSE;
				//	Oldtimestopped = TimeStopped; // 2024-07-03
					TimeStopped +=  1000 * MIDIsyncDelay; // Necessary to restore the timing of the next events
					((*p_INscript)[j]).chan = -1; // This input event is now deactivated
					for(jj = 1; jj <= Jinscript; jj++) { //  Now we deactivate all input events at the same date
						if(((*p_INscript)[j]).time >= ((*p_INscript)[jj]).time && ((*p_INscript)[jj]).chan != -1) {
							((*p_INscript)[jj]).chan = -1;
						//	my_sprintf(Message,"Canceled waiting for NoteOn key = %d at date %ld ms",((*p_INscript)[jj]).key,((*p_INscript)[jj]).time / 1000L);
						//	Notify(Message,0);
							strcpy(Message,"");
							}
						}
					return OK;
					}
				}
			}
		}}}}}}}}}
		if(ParamControlChan > 0) {
			for(i=1; i < MAXPARAMCTRL; i++) {
				if(ParamChan[i] != -1 && ParamKey[i] == c1
									&& c0 == (NoteOn + ParamChan[i] - 1)) {
				/* IN Param key "Kx" = velocity "note" channel "1..16" */
					my_sprintf(Message,"Setting K%d = %d (key %d channel %d)",i,c2,c1,ParamChan[i]);
					Notify(Message,0);
					strcpy(Message,"");
					ParamValue[i] = c2;
					return(OK);
					}
				}
			}
		r = ChangeStatus(c0,c1,c2);
		}
	return(r);
	}

int check_stop_instructions(unsigned long time) {
	int j, instr, mssg;
	unsigned long thisscripttime;
	unsigned char midiData[4];
	for(j=1; j <= Jinscript; j++) {
//		BPPrintMessage(odInfo,"((*p_INscript)[%d]).chan = %d\n",j,((*p_INscript)[j]).chan);
		if(((*p_INscript)[j]).chan == -1) continue;
		instr = ((*p_INscript)[j]).scriptline;
		switch(instr) {
			case 46: break; // Wait for space
			case 97: break; // Wait for a note
			case 67: break; //  Start
			case 66: break; // wait for Continue
			case 128: break; // wait for Stop
			default: continue;
			break;
			}
		thisscripttime = ((*p_INscript)[j]).time + TimeStopped;
		if(thisscripttime > time) continue;
		StopPlay = TRUE;
		if(time == 0L) initTime = (UInt64) getClockTime();
		if(TraceMIDIinteraction) BPPrintMessage(odInfo,"Stopped sound at time %ld ms <= %ld ms as per instruction %d\n",(long)thisscripttime / 1000L,(long)time / 1000L,instr);
		switch(instr) {
			case 46: // Wait for Space
				WaitForSpace = TRUE;
				strcpy(Message, "Waiting for a spacebar hit");
				break;
			case 97: // Wait for note
				my_sprintf(Message,"Waiting for note key %d at date %ld ms",((*p_INscript)[j]).key,(long)thisscripttime / 1000L);
				break;
			case 67: // Wait for Start
				strcpy(Message, "Waiting for MIDI Start (250)"); break;
			case 66: // wait for Continue
				strcpy(Message, "Waiting for MIDI Continue (251)"); break;
			case 128: // wait for Stop
				strcpy(Message, "Waiting for MIDI Stop (252)"); break;
			default:
				strcpy(Message,"");
				break;
			}
		Notify(Message,0); // It should be 1 but the "OK" button is problematic on Windows
		strcpy(Message,"");
		}
	for(j=1; j <= Joutscript; j++) {
		if(((*p_OUTscript)[j]).chan == -1) continue;
		instr = ((*p_OUTscript)[j]).scriptline;
		switch(instr) {
			case 17: break; // Hold
			case 190: mssg = Start; break; // MIDI Start
			case 191: mssg = Continue; break; // MIDI Continue
			case 192: mssg = Stop; break; // MIDI Stop
			default: continue;
			break;
			}
		thisscripttime = ((*p_OUTscript)[j]).time + TimeStopped;
	//	BPPrintMessage(odInfo,"thisscripttime = %ld, time = %ld\n",thisscripttime,time);
		if(thisscripttime > time) continue;
		TimeStopped += ((*p_OUTscript)[j]).duration; // For this reason duration is '0' with Start, Continue and Stop
		(*p_OUTscript)[j].chan = -1; // Now this script can be deleted
		if(instr == 17) {
			if(TraceMIDIinteraction) BPPrintMessage(odInfo,"Holding from time %ld ms during %ld ms as per instruction %d\n",(long)thisscripttime / 1000L,((*p_OUTscript)[j]).duration / 1000L,instr);
			my_sprintf(Message,"Holding during %ld ms",((*p_OUTscript)[j]).duration / 1000L);
			Notify(Message,0);
			strcpy(Message,"");
			}
		else {
			if(TraceMIDIinteraction) BPPrintMessage(odInfo,"Sending MIDI message %d date %ld ms as per instruction %d\n",mssg,(long)thisscripttime / 1000L,instr);
			midiData[0] = mssg;
			my_sprintf(Message,"Sending MIDI instruction (%d) at date %ld ms",mssg,(long)thisscripttime / 1000L);
			Notify(Message,0);
			strcpy(Message,"");
			sendMIDIEvent(0,0,midiData,1,thisscripttime);
			}
		}
	return OK;
	}

int Ctrl_adjust(MIDI_Event *p_e,int c0,int c1,int c2) {
	int speed_change,i,j,r,c11;
	long count = 12L,oldn,dt;
	r = MISSED;
//	if(TraceMIDIinteraction) BPPrintMessage(odError,"(controller %d) ParamControlChan = %d\n",c1,ParamControlChan);
	if(ParamControlChan > 0) {
		for(i=1; i < MAXPARAMCTRL; i++) {
			if(ParamChan[i] != -1
				&& ParamControl[i] == c1 && c0 == (ControlChange + ParamChan[i] - 1)) {
			/* Control parameter _Kx_ from #_0..127_ channel _1..16_ */
				ParamControlChan = ParamChan[i];
				ParamValue[i] = c2;
				my_sprintf(Message,"Setting K%d = %d (controller %d channel %d)",i,c2,c1,ParamChan[i]);
				Notify(Message,0);
				strcpy(Message,"");
				return OK;
				}
			}
		}
	if(SpeedChan > 0 && c0 == ControlChange+SpeedChan-1 && c1 == SpeedCtrl) {
		while(TRUE) {
		//	if(GetNextMIDIevent(p_e,FALSE,FALSE) != OK) break;
			if(p_e->data2 > 127 || p_e->type == NULL_EVENT) break;
			c11 = p_e->data2;
			p_e->type = RAW_EVENT;
		//	if(GetNextMIDIevent(p_e,FALSE,FALSE) != OK) break;
			if(p_e->data2 > 127 || p_e->type == NULL_EVENT) break;
			c1 = c11;
			c2 = p_e->data2;
			}
		Newstatus = TRUE;
		if(PedalOrigin == -1) {
			PedalOrigin = c2;
			OldPclock = Pclock;
			OldQclock = Qclock;
			Nalpha = 100L;
			}
		else {
			PedalPosition = c2;
			if(c2 == PedalOrigin) return(OK);
			SetTempo(); SetTimeBase();
	/*		ShowWindow(Window[wMetronom]);
			BringToFront(Window[wMetronom]); */
			}
		r = OK;
		}
	return(r);
	}


int ChangeStatus(int c0,int c1,int c2) {
	long newP,newQ;
	if(ResetWeightChan > 0 && c1 == ResetWeightKey
			&& c0 == (NoteOn + ResetWeightChan - 1)) {
		ResetWeights = 1 - ResetWeights;
		if(ResetWeights) NeverResetWeights = FALSE;
		if(!Improvize) 
		my_sprintf(Message,"Reset weights: %s",Reality[ResetWeights]);
		ShowMessage(TRUE,wMessage,Message);
		}
	else {
		if(UseEachSubChan > 0 && SUBthere && c1 == UseEachSubKey
			&& c0 == (NoteOn + UseEachSubChan - 1)) {
			UseEachSub = 1 - UseEachSub; if(!Improvize) 
			my_sprintf(Message,"Play each substitution: %s",Reality[UseEachSub]);
			ShowMessage(TRUE,wMessage,Message);
			}
		else {
			if(SynchronizeStartChan > 0 && c1 == SynchronizeStartKey
				&& c0 == (NoteOn + SynchronizeStartChan - 1)) {
				SynchronizeStart = 1 - SynchronizeStart; if(!Improvize) 
				my_sprintf(Message,"Synchronize start: %s",Reality[SynchronizeStart]);
				ShowMessage(TRUE,wMessage,Message);
				}
			else {
				if(c1 == SetTimeKey && SetTimeChan > 0
					&& Improvize && c2 > 0
						&& c0 == (NoteOn + SetTimeChan - 1)) {
					TimeMax = (long) c2 * 472;
					/* UseTimeLimit = */ LimCompute = LimTimeSet = TRUE;
					my_sprintf(Message,
						"Max computation time: %4.2f s.",(double) TimeMax/1000.);
					ShowMessage(TRUE,wMessage,Message);
					}
				else {
					if(StriatedChan > 0 && c1 == StriatedKey
								&& c0 == (NoteOn + StriatedChan - 1)) {
						Nature_of_time = 1 - Nature_of_time;
						my_sprintf(Message,
							"Striated time: %s",Reality[Nature_of_time]);
						ShowMessage(TRUE,wMessage,Message);
						SetTempo(); SetTimeBase();
						Newstatus = TRUE;
						}
					else {
						if(NoConstraintChan > 0 && c1 == NoConstraintKey
									&& c0 == (NoteOn + NoConstraintChan - 1)) {
							NoConstraint = 1 - NoConstraint;
							if(!Improvize) 
							my_sprintf(Message,
								"Ignore constraints: %s",Reality[NoConstraint]);
							ShowMessage(TRUE,wMessage,Message);
							Newstatus = TRUE;
							}
						else {
							if(TclockChan > 0 && c0 == (NoteOn + TclockChan - 1)
									&& ((c1 >= MinTclockKey && c1 <= MaxTclockKey)
									|| (c1 <= MinTclockKey && c1 >= MaxTclockKey))) {
								Pclock = 100L * MinPclock +
									(long) ((100L * (c1 - MinTclockKey) * (MaxPclock - MinPclock)) /
									(MaxTclockKey - MinTclockKey));
								Qclock = 100L * MinQclock +
									(long) ((100L * (c1 - MinTclockKey) * (MaxQclock - MinQclock)) /
									(MaxTclockKey - MinTclockKey));
								Simplify((double)INT_MAX,Pclock,Qclock,&Pclock,&Qclock);
								Nalpha = 100L;
								PedalOrigin = -1;
								SetTempo(); SetTimeBase();
						/*		ShowWindow(Window[wMetronom]);
								BringToFront(Window[wMetronom]); */
								Newstatus = TRUE;
								}
							else {
								if(Improvize && SkipChan > 0 && c1 == SkipKey
											&& c0 == (NoteOn + SkipChan - 1)) {
									SkipFlag = TRUE;
									ShowMessage(TRUE,wMessage,"Skip next item");
									}
								}
							}
						}
					}
				}
			}
		}
	return(OK);
	}


int SetInputFilterWord(int i) {
	if(EndSysExIn[i]) SysExIn[i] = TRUE;
	if(SysExIn[i]) EndSysExIn[i] = TRUE;
/*	if(StartTypeIn[i]) ContTypeIn[i] = TRUE;
	if(ContTypeIn[i]) StartTypeIn[i] = TRUE; */
	if(NoteOnIn[i]) NoteOffIn[i] = TRUE;
	if(NoteOffIn[i]) NoteOnIn[i] = TRUE;
	MIDIacceptFilter[i] =
		ResetIn[i] + 2L * (ActiveSenseIn[i] + 2L * (ContTypeIn[i] + 2L * (StartTypeIn[i] + 2L * (ClockTypeIn[i]
		+ 2L * (EndSysExIn[i] + 2L * (TuneTypeIn[i] + 2L * (SongSelIn[i] + 2L * (SongPosIn[i]
		+ 2L * (TimeCodeIn[i] + 2L * (SysExIn[i] + 2L * (PitchBendIn[i] + 2L * (ChannelPressureIn[i]
		+ 2L * (ProgramTypeIn[i] + 2L * (ControlTypeIn[i] + 2L * (KeyPressureIn[i] + 2L * (NoteOnIn[i]
		+ 2L * NoteOffIn[i]))))))))))))))));
	return(OK);
	}


int SetOutputFilterWord(int i) {
	if(SetInputFilterWord(i) != OK) return(ABORT);
	if(EndSysExPass[i]) SysExPass[i] = TRUE;
	if(SysExPass[i]) EndSysExPass[i] = TRUE;
/*	if(StartTypePass[i]) ContTypePass[i] = TRUE;
	if(ContTypePass[i]) StartTypePass[i] = TRUE; */
	if(NoteOnPass[i]) NoteOffPass[i] = TRUE;
	if(NoteOffPass[i]) NoteOnPass[i] = TRUE;
	MIDIpassFilter[i] =
		ResetPass[i] + 2L * (ActiveSensePass[i] + 2L * (ContTypePass[i] + 2L * (StartTypePass[i] + 2L * (ClockTypePass[i]
		+ 2L * (EndSysExPass[i] + 2L * (TuneTypePass[i] + 2L * (SongSelPass[i] + 2L * (SongPosPass[i]
		+ 2L * (TimeCodePass[i] + 2L * (SysExPass[i] + 2L * (PitchBendPass[i] + 2L * (ChannelPressurePass[i]
		+ 2L * (ProgramTypePass[i] + 2L * (ControlTypePass[i] + 2L * (KeyPressurePass[i] + 2L * (NoteOnPass[i]
		+ 2L * NoteOffPass[i]))))))))))))))));
	/* To pass an event you should enable the driver to receive it... */
	MIDIacceptFilter[i] = MIDIacceptFilter[i] | MIDIpassFilter[i];
	GetInputFilterWord(i);
//	BPPrintMessage(odInfo,"@@@ NoteOnPass = %d\n",NoteOnPass);
	return(OK);
	}


int GetInputFilterWord(int i) {
	long n = 1L;
    ResetIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    ActiveSenseIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    ContTypeIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    StartTypeIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    ClockTypeIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    EndSysExIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    TuneTypeIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    SongSelIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    SongPosIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    TimeCodeIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    SysExIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    PitchBendIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    ChannelPressureIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    ProgramTypeIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    ControlTypeIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    KeyPressureIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    NoteOnIn[i] = (MIDIacceptFilter[i] & n) != 0; n <<= 1;
    NoteOffIn[i] = (MIDIacceptFilter[i] & n) != 0; // Last use of n
/*	BPPrintMessage(odInfo,"@@@ MIDIacceptFilter[%d] = %ld\n",i,MIDIacceptFilter[i]);
	BPPrintMessage(odInfo,"@@@ NoteOnIn[%d] = %d\n",i,NoteOnIn[i]); */
	return(OK);
	}


int GetOutputFilterWord(int i) {
	long n = 1L;
 	ResetPass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    ActiveSensePass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    ContTypePass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    StartTypePass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    ClockTypePass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    EndSysExPass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    TuneTypePass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    SongSelPass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    SongPosPass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    TimeCodePass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    SysExPass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    PitchBendPass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    ChannelPressurePass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    ProgramTypePass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    ControlTypePass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    KeyPressurePass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    NoteOnPass[i] = (MIDIpassFilter[i] & n) != 0; n <<= 1;
    NoteOffPass[i] = (MIDIpassFilter[i] & n) != 0; // Last use of n
/*	char* binaryStr = printBinary18(MIDIpassFilter[i],18);
	BPPrintMessage(odInfo,"MIDIpassFilter[%d] = %s = %ld\n",i,binaryStr,MIDIpassFilter[i]);
	BPPrintMessage(odInfo,"NoteOnPass[%d] = %d\n",i,NoteOnPass[i]);
	free(binaryStr); */
	return(OK);
	}

int ResetMIDIFilter(void) {
	int i;
	for(i= 0; i < MaxInputPorts; i++) {
		MIDIacceptFilter[i] = MIDIinputFilterstartup;
		MIDIpassFilter[i] = MIDIoutputFilterstartup;
		GetInputFilterWord(i);
		GetOutputFilterWord(i);
		SetOutputFilterWord(i);	/* Verifies consistency */
		}
	if(TraceMIDIinteraction) BPPrintMessage(odInfo,"MIDI filter has been reset\n");
	return(OK);
	}

char* printBinary18(long num, int n) { // Used for checking MIDI filters: display "num" as a binary number of n digits
    char* binaryString = malloc(n+1); // Allocate memory for 18 characters plus a null terminator
    if(binaryString == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL; // Return NULL if memory allocation fails
   		}
    unsigned long mask = 1UL << (n-1); // Start with the mask at the highest bit of the n bits
    for(int i = 0; i < n; i++) {
        binaryString[i] = (num & mask) ? '1' : '0'; // Set each character in the array
        mask >>= 1; // Shift the mask right
    	}
    binaryString[n] = '\0'; // Null-terminate the string
    return binaryString; // Return the binary string
	}


int TwoByteEvent(int c) {
	int c0;
	if(c < NoteOff) return(NO);
	if(c == SongSelect) return(YES);
	c0 = c - c % 16;
	if(c0 == ProgramChange || c0 == ChannelPressure) return(YES);
	return(NO);
	}


int ThreeByteEvent(int c) {
	int c0;
	if(c < NoteOff) return(NO);
	if(c == SongPosition) return(YES);
	c0 = c - c % 16;
	if(ThreeByteChannelEvent(c0)) return(YES);
	return(NO);
	}


int ThreeByteChannelEvent(int c) {
	if(c < NoteOff) return(NO);
	if(c == ProgramChange || c == ChannelPressure) return(NO);
	if(c > PitchBend) return(NO);
	return(YES);
	}

int ChannelEvent(int c) {
	int c0;
	if(c < NoteOff) return(NO);
	c0 = c - c % 16;
	if(c0 < SystemExclusive) return(YES);
	return(NO);
	}

int AcceptEvent(int c, int i) {
	int c0;
	switch(c) {
		case SystemExclusive:
			if(SysExIn[i]) return(YES);
			break;
		case SongPosition:
			if(SongPosIn[i]) return(YES);
			break;
		case SongSelect:
			if(SongSelIn[i]) return(YES);
			break;
		case TuneRequest:
			if(TuneTypeIn[i]) return(YES);
			break;
		case EndSysEx:
			if(EndSysExIn[i]) return(YES);
			break;
		case TimingClock:
			if(ClockTypeIn[i]) return(YES);
			break;
		case Start:
		case Stop:
			if(StartTypeIn[i]) return(YES);
			break;
		case Continue:
			if(ContTypeIn[i]) return(YES);
			break;
		case ActiveSensing:
			if(ActiveSenseIn[i]) return(YES);
			break;
		case SystemReset:
			if(ResetIn[i]) return(YES);
			break;
		}
	c0 = c - (c % 16);
	switch(c0) {
		case NoteOff:
			if(NoteOffIn[i]) return(YES);
			break;
		case NoteOn:
			if(NoteOnIn[i]) return(YES);
			break;
		case KeyPressure:
			if(KeyPressureIn[i]) return(YES);
			break;
		case ControlChange:
			if(ControlTypeIn[i]) return(YES);
			break;
		case ProgramChange:
			if(ProgramTypeIn[i]) return(YES);
			break;
		case ChannelPressure:
			if(ChannelPressureIn[i]) return(YES);
			break;
		case PitchBend:
			if(PitchBendIn[i]) return(YES);
			break;
		}
	return(NO);
	}


int PassInEvent(int c, int i) {
	int c0;

	switch(c) {
		case SystemExclusive:
			if(SysExPass[i]) return(YES);
			break;
		case SongPosition:
			if(SongPosPass[i]) return(YES);
			break;
		case SongSelect:
			if(SongSelPass[i]) return(YES);
			break;
		case TuneRequest:
			if(TuneTypePass[i]) return(YES);
			break;
		case EndSysEx:
			if(EndSysExPass[i]) return(YES);
			break;
		case TimingClock:
			if(ClockTypePass[i]) return(YES);
			break;
		case Start:
		case Stop:
			if(StartTypePass[i]) return(YES);
			break;
		case Continue:
			if(ContTypePass[i]) return(YES);
			break;
		case ActiveSensing:
			if(ActiveSensePass[i]) return(YES);
			break;
		case SystemReset:
			if(ResetPass[i]) return(YES);
			break;
		}
	c0 = c - (c % 16);
	switch(c0) {
		case NoteOff:
			if(NoteOffPass[i]) return(YES);
			break;
		case NoteOn:
			if(NoteOnPass[i]) return(YES);
			break;
		case KeyPressure:
			if(KeyPressurePass[i]) return(YES);
			break;
		case ControlChange:
			if(ControlTypePass[i]) return(YES);
			break;
		case ProgramChange:
			if(ProgramTypePass[i]) return(YES);
			break;
		case ChannelPressure:
			if(ChannelPressurePass[i]) return(YES);
			break;
		case PitchBend:
			if(PitchBendPass[i]) return(YES);
			break;
		}
	return(NO);
	}


int PassOutEvent(int c, int i) {
	int c0;
	switch(c) {
		case SystemExclusive:
			if(MIDIoutFilter[i][7] == '0') return(NO);
			break;
		case SongPosition:
			if(MIDIoutFilter[i][9] == '0') return(NO);
			break;
		case SongSelect:
			if(MIDIoutFilter[i][10] == '0') return(NO);
			break;
		case TuneRequest:
			if(MIDIoutFilter[i][11] == '0') return(NO);
			break;
		case EndSysEx:
			if(MIDIoutFilter[i][12] == '0') return(NO);
			break;
		case TimingClock:
			if(MIDIoutFilter[i][13] == '0') return(NO);
			break;
		case Start:
			if(MIDIoutFilter[i][14] == '0') return(NO);
			break;
		case Continue:
			if(MIDIoutFilter[i][15] == '0') return(NO);
			break;
		case Stop:
			if(MIDIoutFilter[i][14] == '0') return(NO);
			break;
		case ActiveSensing:
			if(MIDIoutFilter[i][16] == '0') return(NO);
			break;
		case SystemReset:
			if(MIDIoutFilter[i][17] == '0') return(NO);
			break;
		}
	c0 = c - (c % 16);
	switch(c0) {
		case NoteOff:
			if(MIDIoutFilter[i][0] == '0') return(NO);
			break;
		case NoteOn:
			if(MIDIoutFilter[i][1] == '0') return(NO);
			break;
		case KeyPressure:
			if(MIDIoutFilter[i][2] == '0') return(NO);
			break;
		case ControlChange:
			if(MIDIoutFilter[i][3] == '0') return(NO);
			break;
		case ProgramChange:
			if(MIDIoutFilter[i][4] == '0') return(NO);
			break;
		case ChannelPressure:
			if(MIDIoutFilter[i][5] == '0') return(NO);
			break;
		case PitchBend:
			if(MIDIoutFilter[i][6] == '0') return(NO);
			break;
		}
	return(YES);
	}

int SendMIDIstream(int check,char** p_line,int hexa)
{
int i,r;
long n,time;
MIDI_Event e;

if((*p_line)[0] == '\0') return(MISSED);
i = 0; r = MISSED;

#if WITH_REAL_TIME_MIDI_FORGET_THIS
MyLock(FALSE,(Handle)p_line);
time = ZERO;
do {
	while(MySpace((*p_line)[i])) i++;
	if((*p_line)[i] == '\0') break;
	if(!hexa && (n = GetInteger(YES,*p_line,&i)) == INT_MAX) break;
	if(hexa && (n = GetHexa(*p_line,&i)) == INT_MAX) break;
	if(n > 255 || n < 0) {
		my_sprintf(Message,"\nCan't send %ld as MIDI data.\n",(long)n);
		Print(wTrace,Message);
		 goto QUIT;
		}
	i++;
	e->time = time;
	e->type = RAW_EVENT;
	e->data2 = n;
	// DriverWrite(time,0,&e);
	}
while(TRUE);
r = OK;

QUIT:
MyUnlock((Handle)p_line);
#endif

return(r);
}


int MIDItoPrototype(int zerostart,int filter,int j,MIDIcode **p_b,long imax)
// Store MIDI codes to prototype j
{
long im2,nbytes;
MIDIcode **ptr1;
Handle ptr;
double preroll,postroll;

im2 = 2L * imax;
if((ptr1 = (MIDIcode**) GiveSpace((Size) im2 * sizeof(MIDIcode))) == NULL)
	return(ABORT);

if(FormatMIDIstream(p_b,imax,ptr1,zerostart,im2,&nbytes,filter) != OK) return(MISSED);
// BPPrintMessage(odInfo, "j = %d nbytes = %d\n",j,nbytes);

ptr = (Handle)(*pp_MIDIcode)[j];
if(MyDisposeHandle(&ptr) != OK) return(ABORT);
(*pp_MIDIcode)[j] = NULL;

(*pp_MIDIcode)[j] = ptr1;

GetPrePostRoll(j,&preroll,&postroll);
if(nbytes > 0) {
	(*p_MIDIsize)[j] = nbytes;
	(*p_Type)[j] |= 1;
	(*p_Dur)[j] = ((*((*pp_MIDIcode)[j]))[nbytes-1].time) - preroll + postroll;
	ptr1 = (*pp_MIDIcode)[j];
	MySetHandleSize((Handle*)&ptr1,(Size) nbytes * sizeof(MIDIcode));
	(*pp_MIDIcode)[j]  = ptr1;
	PointMIDI = TRUE;
	if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
	}
else {
	PointMIDI = FALSE;
	(*p_MIDIsize)[j] = ZERO;
	(*p_Dur)[j] = ZERO;
	}
return(OK);
}


int FormatMIDIstream(MIDIcode **p_b,long imax,MIDIcode **p_c,int zerostart,
	long im2,long *p_nbytes,int filter)
{
long i,ii,time,t0,this_byte;
int b,br,rc,which_control,value1,value2,foundNoteOn,status;

// Make sure dates are increasing, starting from 0

/* if(trace_midi_filter) {
	for(i=0; i < imax; i++) {
		BPPrintMessage(odInfo,"%d\n",(*p_b)[i].byte);
		}
	} */
	
time = (*p_b)[0].time;
if(zerostart) t0 = time;
else t0 = ZERO;
for(i=0; i < imax; i++) {
	(*p_c)[i].sequence = (*p_b)[i].sequence;
	if((*p_b)[i].time < time) (*p_b)[i].time = time - t0;
	else {
		time = (*p_b)[i].time;
		(*p_b)[i].time -= t0;
		}
	}
ii = ZERO;
i = -1; br = 0;

// Restore missing status bytes
// It's a headache!
// It doesn't follow running status...
// And it's probably useless because the MIDI sequences we import from the interface
// and the ones produced by BP do not have running status.
// The problem is ControlChange which is a 3-byte event only if C1 > 64, otherwise a 4-byte event

// filter = FALSE;

if(trace_midi_filter) BPPrintMessage(odInfo,"\n");

foundNoteOn = FALSE;

NEXTBYTE:
if(ii >= im2) {
	if(Beta) Alert1("=> Err. FormatMIDIstream(). ii >= im2");
	return(MISSED);
	}
i++; if(i >= imax) goto QUIT;
b = ByteToInt((*p_b)[i].byte);
time = (*p_b)[i].time;

if(b == TimingClock) goto NEXTBYTE;
if(b == SystemExclusive) {
	br = 0;
	while(((b = (*p_b)[i].byte) != EndSysEx) && i < imax - 1) {
	/*	(*p_c)[ii].time = time;
		(*p_c)[ii].byte = ByteToInt((*p_b)[i].byte);
		(*p_c)[ii++].sequence = (*p_b)[i].sequence;
		if(ii >= im2) {
			if(Beta) Alert1("=> Err. FormatMIDIstream(). ii >= im2");
			return(MISSED);
			} */
		i++;
		}
/*	(*p_c)[ii].time = time;
	(*p_c)[ii].byte = EndSysEx;
	(*p_c)[ii++].sequence = 0; */
	goto NEXTBYTE;
	}

if(!filter) {
	(*p_c)[ii].time = time;
	(*p_c)[ii].byte = b;
	(*p_c)[ii++].sequence = (*p_b)[i].sequence;
	goto NEXTBYTE;
	}

if(b < NoteOff || br == PitchBend || br == ProgramChange || br == ChannelPressure) {
	if(br == 0) goto NEXTBYTE;	/* happens in beginning */
	(*p_c)[ii].time = time;
	this_byte = (*p_c)[ii].byte = br + rc;
	(*p_c)[ii++].sequence = 0;
	if(ii >= im2) {
		if(Beta) Alert1("=> Err. FormatMIDIstream(). ii >= im2");
		return(MISSED);
		}
	value1 = (*p_c)[ii].byte = ByteToInt((*p_b)[i].byte);
	(*p_c)[ii++].sequence = (*p_b)[i].sequence;
	if(ii >= im2) {
		if(Beta) Alert1("=> Err. FormatMIDIstream(). ii >= im2");
		return(MISSED);
		}
	if(br == ProgramChange || br == ChannelPressure) {
		if(trace_midi_filter) BPPrintMessage(odInfo,"ChannelPressure or ProgramChange channel %d value = %d i = %ld\n",(rc + 1),value1,(long)i);
		br = 0;
		goto NEXTBYTE;
		}
	i++; if(i >= imax) goto QUIT;
	(*p_c)[ii].time = time;
	value2 = (*p_c)[ii].byte = ByteToInt((*p_b)[i].byte);
	(*p_c)[ii++].sequence = (*p_b)[i].sequence;
	if(trace_midi_filter) BPPrintMessage(odInfo,"This byte %d value1 = %d value2 = %d i = %ld\n",this_byte,value1,value2,(long)i);
	if(br == PitchBend) br = 0; 
	goto NEXTBYTE;
	}
if(b >= SystemExclusive) goto NEXTBYTE;
rc = b % 16;
b -= rc;
if(b == ControlChange) {
	which_control = ByteToInt((*p_b)[i+1].byte);
	(*p_c)[ii].time = time;
	(*p_c)[ii].byte = b + rc;
	(*p_c)[ii++].sequence = (*p_b)[i].sequence;
	i++; if(i >= imax) goto QUIT;
	(*p_c)[ii].time = time;
	which_control = (*p_c)[ii].byte = ByteToInt((*p_b)[i].byte);
	(*p_c)[ii++].sequence = (*p_b)[i].sequence;
	i++; if(i >= imax) goto QUIT;
	(*p_c)[ii].time = time;
	(*p_c)[ii].byte = ByteToInt((*p_b)[i].byte);
	(*p_c)[ii++].sequence = (*p_b)[i].sequence;
	if(trace_midi_filter) BPPrintMessage(odInfo,"Param #%d channel %d value = %d i = %d\n",which_control,(rc + 1),(*p_b)[i].sequence,i);
	goto NEXTBYTE;
	}
	
br = b; // The following bytes will be treated as following a running status
if(ThreeByteChannelEvent(b) || b == ProgramChange || b == ChannelPressure || b == PitchBend) {
	goto NEXTBYTE;
	}
br = 0;	/* b doesn't have a value that may be taken for running status */
goto NEXTBYTE;

QUIT:
*p_nbytes = ii;

if(trace_midi_filter) {
	for(i=0; i < ii; i++) {
		BPPrintMessage(odInfo,"%d %d (%ld)\n",(*p_c)[i].byte,(*p_c)[i].sequence,(long)(*p_c)[i].time);
		}
	}
return(OK);
}

#if BP_CARBON_GUI_FORGET_THIS

int SelectControlArgument(int w,char* line)
{
int i,ii,j,ok,n;
long iorg,iend,length;
char c;

TextGetSelection(&iorg, &iend, TEH[w]);
length = GetTextLength(w);
if(line[0] == '\0') return(MISSED);

/* Suppress bracket */
line[strlen(line)-1] = '\0';
Jcontrol = -1;
for(j=0; j < MaxPerformanceControl; j++) {
	if(Mystrcmp((*p_PerformanceControl)[j],line) == 0) {
		Jcontrol = j; break;
		}
	}
ok = YES;
switch(Jcontrol) {
	case 0:	/* _chan */
		ShowMessage(TRUE,wMessage,"Waiting for MIDI message indicating channel...");
		break;
	case 1:	/* _vel */
		ShowMessage(TRUE,wMessage,"Waiting for NoteOn message indicating velocity...");
		break;
	case 5:	/* _mod */
		ShowMessage(TRUE,wMessage,"Move the modulation bender...");
		break;
	case 8:	/* _pitchbend */
		ShowMessage(TRUE,wMessage,"Move the pitch bender...");
		break;
	case 11:	/* _press */
		ShowMessage(TRUE,wMessage,"Play a note and adjust pressure...");
		break;
	case 16:	/* _volume */
		ShowMessage(TRUE,wMessage,"Move the volume pedal...");
		break;
	case 35:	/* _volumecontrol */
		ShowMessage(TRUE,wMessage,"Move the controller you are using for volume...");
		break;
	case 36:	/* _pan */
		ShowMessage(TRUE,wMessage,"Move the panoramic controller...");
		break;
	case 41:	/* _pancontrol */
		ShowMessage(TRUE,wMessage,"Move the controller you are using for panoramic...");
		break;
	default:
		ok = NO; break;
	}
if(!ok) return(MISSED);

/* Select argument */
i = iend;
while(GetTextChar(w,i) != ')') {
	i++;
	if(i >= length) return(MISSED);
	}
SetSelect(iend,i,TEH[w]);
BPActivateWindow(SLOW,w);
if(Jcontrol != 8 && Jcontrol != 16 && Jcontrol != 36) return(OK);

/* Look for particular settings on same line */
i = iend;
while(GetTextChar(w,i) != '\r') {
	i--; if(i < 0) break;
	}
i++;
PitchbendRange[0] = DeftPitchbendRange;
VolumeControl[0] = VolumeController;
PanoramicControl[0] = PanoramicController;
for(i=i; i < iend; i++) {
	if((c=GetTextChar(w,i)) != '_') continue;
	iorg = i;
	line[i-iorg] = c;
	for(i=iorg+1;;i++) {
		c = GetTextChar(w,i);
		if(c == '(' || !isalnum(c) || i >= iend) break;
		line[i-iorg] = c;
		}
	line[i-iorg] = '\0';
	if(c != '(') continue;
	for(j=0; j < MaxPerformanceControl; j++) {
		if(Mystrcmp((*p_PerformanceControl)[j],line) == 0) {
			break;
			}
		}
	if(j == MaxPerformanceControl) continue;
	iorg = i + 1;
	/* Copy numeric argument */
	for(i=iorg;;i++) {
		c = GetTextChar(w,i);
		if(!isdigit(c)) break;
		if(c == ')' || i >= iend) break;
		line[i-iorg] = c;
		}
	if(i >= iend) break;
	line[i-iorg] = '\0'; ii = 0;
	if((n=GetInteger(YES,line,&ii)) == INT_MAX) continue;
	switch(j) {
		case 29:	/* _pitchrange */
			if(Jcontrol == 8) {
				if(n < 0 || n > 16383) {
					my_sprintf(Message,
						"Range for _pitchrange(x) is 0..16383. Can't accept %ld",(long)n);
					Alert1(Message);
					return(MISSED);
					}
				PitchbendRange[0] = n;	/* 0 is special index */
				}
			break;
		case 35:	/* _volumecontrol */
			if(Jcontrol == 16) {
				if(n < 0 || n > 127) {
					my_sprintf(Message,
						"Range for _volumecontrol(x) is 0..127. Can't accept %ld",(long)n);
					Alert1(Message);
					return(MISSED);
					}
				VolumeControl[0] = n;	/* 0 is special index */
				}
			break;
		case 41:	/* _pancontrol */
			if(Jcontrol == 36) {
				if(n < 0 || n > 127) {
					my_sprintf(Message,
						"Range for _pancontrol(x) is 0..127. Can't accept %ld",(long)n);
					Alert1(Message);
					return(MISSED);
					}
				PanoramicControl[0] = n;	/* 0 is special index */
				}
			break;
		default: continue;
		}
	}
return(OK);
}


int ReadMIDIparameter(int c0, int c1, int c2, int wind)
// Get parameter of incoming MIDI message and put it as an argument
// of performance control.
// Argument is already selected.
{
int x,chan,c;
long origin,end;

if(Jcontrol == -1) return(MISSED);
chan = c0 % 16;
c = c0 - chan;
switch(Jcontrol) {
	case 0:	/* _chan */
		x = chan + 1; break;
	case 1:	/* _vel */
		if(c == NoteOn && c2 > 0) x = c2;
		else return(MISSED);
		break;
	case 5:	/* _mod */
		if(c == ControlChange && (c1 == 33 || c1 == 1)) {
			if(c1 == 33) x = c2 + OldModulation;
		/* Here c2 may be greater than 256 because it contains MSB and LSB. */
			else {
				x = OldModulation = 128 * c2;
				}
			}
		else return(MISSED);
		break;
	case 8:	/* pitchbend = (int) PitchbendStart(kcurrentinstance); */
		if(c == PitchBend) x = c1 + (128L * c2);
		else return(MISSED);
		if(PitchbendRange[0] > 0) {
			x =  PitchbendRange[0] * ((double) x - DEFTPITCHBEND) / ((double) DEFTPITCHBEND);
			}
		break;
	case 11:	/* _press */
		if(c == ChannelPressure) x = c1;
		else return(MISSED);
		break;
	case 16:	/* _volume */
		if(c == ControlChange && c1 == VolumeControl[0]) x = c2;
		else return(MISSED);
		break;
	case 35:	/* _volumecontrol */
		if(c == ControlChange) x = c1;
		else return(MISSED);
		break;
	case 36:	/* _pan */
		if(c == ControlChange && c1 == PanoramicControl[0]) x = c2;
		else return(MISSED);
		break;
	case 41:	/* _pancontrol */
		if(c == ControlChange) x = c1;
		else return(MISSED);
		break;
	default: return(MISSED);
	}
my_sprintf(Message,"%ld",(long)x);
TextDelete(LastEditWindow);
#if USE_MLTE_FORGET_THIS
// FIXME ? Is there a reason the Print() call is inbetween getting origin & end below ?
TextGetSelection(&origin, &end, TEH[LastEditWindow]);
Print(LastEditWindow,Message);
#else
origin = (**(TEH[LastEditWindow])).selStart;
Print(LastEditWindow,Message);
end = (**(TEH[LastEditWindow])).selEnd;
#endif
SetSelect(origin,end,TEH[LastEditWindow]);
return(OK);
}


int SetFilterDialog(void)
// Set buttons in MIDI filter dialog
{
SwitchOnOff(NULL,wFilter,bNoteOnOffIn,NoteOffIn);
SwitchOnOff(NULL,wFilter,bKeyPressureIn,KeyPressureIn);
SwitchOnOff(NULL,wFilter,bControlIn,ControlTypeIn);
SwitchOnOff(NULL,wFilter,bProgramChangeIn,ProgramTypeIn);
SwitchOnOff(NULL,wFilter,bChannelPressureIn,ChannelPressureIn);
SwitchOnOff(NULL,wFilter,bPitchBendIn,PitchBendIn);
SwitchOnOff(NULL,wFilter,bSysExIn,SysExIn);
SwitchOnOff(NULL,wFilter,bTimeCodeIn,TimeCodeIn);
SwitchOnOff(NULL,wFilter,bSongPositionIn,SongPosIn);
SwitchOnOff(NULL,wFilter,bSongSelectIn,SongSelIn);
SwitchOnOff(NULL,wFilter,bTuneRequestIn,TuneTypeIn);
SwitchOnOff(NULL,wFilter,bTimingClockIn,ClockTypeIn);
SwitchOnOff(NULL,wFilter,bStartStopIn,StartTypeIn);
SwitchOnOff(NULL,wFilter,bActiveSensingIn,ActiveSenseIn);
SwitchOnOff(NULL,wFilter,bSystemResetIn,ResetIn);
SwitchOnOff(NULL,wFilter,bNoteOnOffPass,NoteOffPass);
SwitchOnOff(NULL,wFilter,bKeyPressurePass,KeyPressurePass);
SwitchOnOff(NULL,wFilter,bControlPass,ControlTypePass);
SwitchOnOff(NULL,wFilter,bProgramChangePass,ProgramTypePass);
SwitchOnOff(NULL,wFilter,bChannelPressurePass,ChannelPressurePass);
SwitchOnOff(NULL,wFilter,bPitchBendPass,PitchBendPass);
SwitchOnOff(NULL,wFilter,bSysExPass,SysExPass);
SwitchOnOff(NULL,wFilter,bTimeCodePass,TimeCodePass);
SwitchOnOff(NULL,wFilter,bSongPositionPass,SongPosPass);
SwitchOnOff(NULL,wFilter,bSongSelectPass,SongSelPass);
SwitchOnOff(NULL,wFilter,bTuneRequestPass,TuneTypePass);
SwitchOnOff(NULL,wFilter,bTimingClockPass,ClockTypePass);
SwitchOnOff(NULL,wFilter,bStartStopPass,StartTypePass);
SwitchOnOff(NULL,wFilter,bActiveSensingPass,ActiveSensePass);
SwitchOnOff(NULL,wFilter,bSystemResetPass,ResetPass);
return(OK);
}


#if WITH_REAL_TIME_MIDI_FORGET_THIS
if(Mute) {
	Alert1("The 'Mute' button was checked on the control pannel...");
	Mute = FALSE;
	// HideWindow(Window[wInfo]);
	MaintainMenus();
	BPActivateWindow(SLOW,wControlPannel);
	return(MISSED);
	}
if(SoundOn) {
	Alert1("BP3 is already sending messages to the MIDI output...");
	return(MISSED);
	}
ResetMIDIControllers(YES,YES,YES);

my_sprintf(Message,"Playing test on MIDI channel %ld... (Click to stop)",(long)(channel+1));
FlashInfo(Message);
ShowMessage(TRUE,wMessage,"Tempo is set by the metronome. Click and type cmd-M to modify it.");
ResetMIDI(FALSE);
duration = (300L * Pclock) / Qclock / Time_res;
r = 3.999;
minkey = 48; maxkey = 72; stepkey = 1;
kx = (double)(maxkey - minkey) / stepkey;
x =  (double) (1 + ((unsigned) clock() % 998)) / 1000.;
key = minkey + stepkey * (int)(x * kx);
stop = FALSE;
while(TRUE) {
	PleaseWait();
	e.type = NORMAL_EVENT;
	e.time = Tcurr;
	e.status = NoteOn + channel;
	e.data1 = key;
	e.data2 = 60;
	// DriverWrite(Tcurr * Time_res,0,&e);
	Tcurr += duration;
	e.type = NORMAL_EVENT;
	e.time = Tcurr;
	e.status = NoteOn + channel;
	e.data1 = key;
	e.data2 = 0;
	// DriverWrite(Tcurr * Time_res,0,&e);
	x = (r * x * (1 - x));
	key = minkey + stepkey * (int)(x * kx);
	if(Button()) stop = TRUE;
	drivertime = GetDriverTime();
	while(Tcurr > drivertime) {
		/* Wait for the end of preceding play */
		PleaseWait();
		drivertime = GetDriverTime();
		if(Button()) stop = TRUE;
		}
	if(stop) break;
	}
while(Button());
ResetMIDI(FALSE);

// #ifndef __POWERPC
FlushEvents(mDownMask+mUpMask,0);
// #endif

// HideWindow(Window[wInfo]);
return(OK);
#endif
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

int FindScale(int scale) {
	int i_scale,result;
	if(scale < 0) return(-1);
	if(scale == 0) i_scale = 0; // Use equal-tempered scale
	// ‘scale’ is not the index of the scale. It is the index of its name in StringConstant
	else for(i_scale = 1; i_scale <= NumberScales; i_scale++) {
		result = MyHandlecmp((*p_StringConstant)[scale],(*Scale)[i_scale].label);
		if(result == 0) break;
		}
	return i_scale;
	}

int AssignUniqueChannel(int status,int note,int value,int i_scale) {
    int ch;
	if(i_scale < 0) i_scale = 0;
	if(i_scale >= MAXCONVENTIONS) {
		BPPrintMessage(odError,"=> i_scale >= MAXCONVENTIONS (100)\n");
		i_scale = 0;
		}
    for(ch = 1; ch < 16; ch++) {
        if(MPEnote[ch] == note)  {
            if((status == NoteOff || value == 0) && MPEscale[ch] == i_scale) {
				MPEnote[ch] = 0;
				MPEscale[ch] = 0;
            	return ch;
				}
			if(status == NoteOn && value > 0 && MPEscale[ch] == i_scale) return ch;
            }
        }
    for(ch = 1; ch < 16; ch++) {
        if(MPEnote[ch] == 0 && MPEscale[ch] == 0) {
            if(status == NoteOn && value > 0) {
                MPEnote[ch] = note;
				MPEscale[ch] = i_scale;
                return ch;
                }
            }
        }
    return(-1);
    }

int SendToDriver(int kcurrentinstance,int scale,int blockkey,Milliseconds time,int nseq,int *p_rs,MIDI_Event *p_e) {
	// nseq is useless
	long count = 12L;
	int c0,c1,c2,status,chan,result,type,i_scale,note,i_note,keyclass,basekeyclass,octave,value,sensitivity,correction;
	unsigned long done;
	byte midibyte;
	MIDI_Event pb_event;
    unsigned int pitchBendValue, pitchbend_master;
    unsigned char pitchBendLSB, pitchBendMSB;
	char this_key[100];

	LastTime = time;
	if(Panic || EmergencyExit) return(ABORT);
	if(!MIDIfileOn && !rtMIDI) return(OK);
	status = ByteToInt(p_e->status);
	note = ByteToInt(p_e->data1);
	value = ByteToInt(p_e->data2);
	type = status & 0xF0;
	if(type != NoteOn && type != NoteOff) i_scale = blockkey = 0;
	if(rtMIDI) {
        done = 0L;
	//	trace_driver = TRUE;
    	if(trace_driver) 
			BPPrintMessage(odInfo,"Sending MIDI event to stack, date = %ld ms,\tstatus = %ld,\tdata1 = %ld,\tdata2 = %ld\n",(long)time,(long)p_e->status,(long)p_e->data1,(long)p_e->data2);
		MIDIflush();
        while(eventCount > eventCountMax) {
			// The stack is full
            WaitABit(1); // Sleep for 1 millisecond
            if(MIDIflush() != OK) break;
        //  if(done++ == 0L) BPPrintMessage(odInfo,"Reached the limit of the buffer...\n");
            }
		}
	if(MIDImicrotonality && (type == NoteOn || type == NoteOff)) {
		i_scale = FindScale(scale);
	//	BPPrintMessage(odInfo,"i_scale = %d\n",i_scale);
		int check_corrections = FALSE;
		if((type == NoteOn || type == NoteOff) && i_scale <= NumberScales && i_scale > 0) {
			chan = AssignUniqueChannel(type,note,value,i_scale);
			if(chan > 0) p_e->status = type + chan;
			if(type == NoteOn && value > 0 && chan > 0) {
				if(blockkey < 0) blockkey = DefaultBlockKey;
				correction = (*(*Scale)[i_scale].deviation)[note] - (*(*Scale)[i_scale].deviation)[blockkey];
				if(TraceMicrotonality) {
					int basekey = (*Scale)[i_scale].basekey;
					int numgrades = (*Scale)[i_scale].numgrades;
					int numnotes = (*Scale)[i_scale].numnotes;
					int baseoctave = (*Scale)[i_scale].baseoctave;
					double interval = (*Scale)[i_scale].interval;
					if(numgrades <= 12) { 
						i_note = 0;
						keyclass = modulo(note - basekey, 12);
						basekeyclass = (*((*Scale)[i_scale].keyclass))[modulo(basekey, 12)];
						octave = baseoctave + floor((double) (note - basekey)  / 12.);
						}
					else {
						i_note = modulo(note - basekey, numnotes);
						keyclass = (*((*Scale)[i_scale].keyclass))[i_note];
						basekeyclass = (*((*Scale)[i_scale].keyclass))[modulo(basekey, numnotes)];
						octave = baseoctave + floor((double) (note - basekey) / numnotes);
						}
					double pitch_ratio = (*((*Scale)[i_scale].tuningratio))[keyclass];
					double n = log(pitch_ratio * pow(interval,octave - baseoctave)) / log(interval);
					double frequency = (*Scale)[NumberScales].basefreq  * pow(interval,n);
					frequency = frequency * pow(interval, ((double) correction / 600. / interval));
					frequency = frequency * (*((*Scale)[i_scale].tuningratio))[basekeyclass];
					frequency = frequency * A4freq / 440.;

					if(check_corrections) BPPrintMessage(odInfo,"i_note = %d, keyclass = %d, numnotes = %d, pitch_ratio = %.3f, basekey = %d, basekeyclass = %d, block key = %d, octave = %d\n",i_note,keyclass,numnotes,pitch_ratio,basekey,basekeyclass,blockkey,octave);

					my_sprintf(this_key,"%s%d",*((*(*Scale)[i_scale].notenames)[keyclass]),octave);
					trim_digits_after_key_hash(this_key); // Remove the octave number after key#xx
					BPPrintMessage(odInfo,"§ key %d: \"%s\" chan %d",note,this_key,(chan+1));
					BPPrintMessage(odInfo," scale #%d, block key %d, corr %d cents, freq %.3f Hz",i_scale,blockkey,correction,frequency);
					if(basekey != 60) BPPrintMessage(odInfo," (base key %d in scale)",basekey);
					BPPrintMessage(odInfo,"\n");
					}
				// With a pitch bend sensitivity of 2 semitones, the entire pitch bend range (14-bit) will correspond to ± 2 semitones.
				// The 14-bit range is 16384 values (from 0 to 16383), with 8192 being the center (no pitch bend).
				// Therefore, 2 semitones = 200 cents corresponds to 8192 units, and 1 cent is 8192 / 200 units
				sensitivity = 2; // semitones
				pitchbend_master = (int) PitchbendStart(kcurrentinstance);
				if(pitchbend_master > 0 && pitchbend_master < 16384) pitchbend_master -= DEFTPITCHBEND;
				else pitchbend_master = 0;
				if(pitchbend_master != 0  && TraceMicrotonality) BPPrintMessage(odInfo,"--> with additional pitchbendvalue %d\n",pitchbend_master);
				if(correction < -200 || correction >= 200) {
					if(ToldPitchbend++ < 4) BPPrintMessage(odError,"=> Microtonality pitchbender out of range ± 200 cents: %d  cents note %d\n",correction,note);
					correction = 0;
					}
				if(correction != 0) {
					pitchBendValue = pitchbend_master + DEFTPITCHBEND + (int)(correction * (0.01 * DEFTPITCHBEND / sensitivity));
					if(pitchBendValue < 0) {
						if(ToldPitchbend++ < 4) BPPrintMessage(odError,"=> Pitchbender out of range has been set to 0\n");
						pitchBendValue = 0;
						}
					if(pitchBendValue > 16383) {
						if(ToldPitchbend++ < 4) BPPrintMessage(odError,"=> Pitchbender out of range has been set to 16383\n");
						pitchBendValue = 16383;
						}
					pitchBendLSB = pitchBendValue & 0x7F; // Lower 7 bits
					pitchBendMSB = (pitchBendValue >> 7) & 0x7F; // Upper 7 bits
					pb_event.status = PitchBend + chan;
					pb_event.data1 = pitchBendLSB;  // Pitch Bend LSB
					pb_event.data2 = pitchBendMSB;  // Pitch Bend MSB
				//  BPPrintMessage(odInfo,"• pitchBendValue channel %d: %d = %d %d %d\n",chan,pitchBendValue,(int)pb_event.status,(int)pb_event.data1,(int)pb_event.data2);
					if(rtMIDI) {
						eventStack[eventCount] = pb_event;
						eventStack[eventCount].time = 1000 * time;
						eventCount++;
						}
					else {  // Send pitchbend to MIDI file
						midibyte = pb_event.status;
						if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
						midibyte = pb_event.data1;
						if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
						midibyte = pb_event.data2;
						if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
						if(TraceMIDIinteraction) BPPrintMessage(odInfo,"Sending pitchbend to MIDI file: %d %d %d\n",pb_event.status,pb_event.data1,pb_event.data2);
						status = type + chan;
						}
					}
				}
			}
		}
	if(rtMIDI) {
		// Sending to the event stack
		eventStack[eventCount] = *p_e;
		eventStack[eventCount].time = 1000 * time;
		eventCount++;
		if((type == NoteOn) && FirstNoteOn) {
			FirstNoteOn = FALSE;
			initTime = (UInt64) getClockTime();
			if(TraceMIDIinteraction) BPPrintMessage(odInfo,"\nFirst NoteOn at %ld ms\n",(long)time);
			} 
		return(OK);
		}

	// The following is for MIDI files
	if(p_e->type == RAW_EVENT || p_e->type == TWO_BYTE_EVENT) {
		if(p_e->type == TWO_BYTE_EVENT) {
			midibyte = p_e->status;
			if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
			}
		midibyte = p_e->data2;
		if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
		*p_rs = 0;
		goto SORTIR;
		}
	// The event is NORMAL_EVENT type
	/* Don't use running status when capturing event stream, or
	when sending to any MIDI driver that does not communicate
	directly with a Serial port (eg. OMS, CoreMIDI, etc.) */
	if(ItemCapture) *p_rs = 0;
	chan = status % 16;
	c0 = status - chan;
	if(trace_driver) BPPrintMessage(odInfo,"++ SendToDriver(kcurrentinstance,scale,blockkey,) time = %ld c0 = %d\tc1 = %d\tc2 = %d\n",(long)time,c0,ByteToInt(p_e->data1),ByteToInt(p_e->data2));
	/* Store if volume */
	if(MIDIfileOn && MIDIfileOpened) {
		if(c0 == NoteOn && CurrentVolume[chan+1] == -1)
			CurrentVolume[chan+1] = DeftVolume;
		if(c0 == ControlChange) {
			c1 = ByteToInt(p_e->data1);
			if(c1 == VolumeControl[chan+1]) {
				CurrentVolume[chan+1] = ByteToInt(p_e->data2);
				}
			}
		}
	if(status != *p_rs || c0 == ChannelMode /* || c0 == ProgramChange */) {
		/* Send the full Midi event */
		*p_rs = status;
		if(p_e->data1 > 127) {
			if(Beta) BPPrintMessage(odError,"=> Err. SendToDriver(kcurrentinstance,scale,blockkey,). p_e->data1 > 127.");
			p_e->data1 = 127;
			}
		if(p_e->data2 > 127) {
			if(Beta) BPPrintMessage(odError,"=> Err. SendToDriver(kcurrentinstance,scale,blockkey,). p_e->data2 > 127.");
			p_e->data2 = 127;
			}
		midibyte = status;
		if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
		midibyte = p_e->data1;
		if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
		midibyte = p_e->data2;
		if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
		// if(trace_driver) BPPrintMessage(odInfo,"Full event status = %d c1 = %d c2= %d time = %ld\n",status,ByteToInt(p_e->data1),ByteToInt(p_e->data2),(long)time);
		}
	else {
		// Skip the status byte, send only data ("running status")
		// This should probably only be used with direct Serial drivers
		if(p_e->data1 > 127) {
			if(Beta) BPPrintMessage(odError,"=> Err. SendToDriver(kcurrentinstance,scale,blockkey,). p_e->data1 > 127.");
			p_e->data1 = 127;
			}
		if(p_e->data2 > 127) {
			if(Beta) BPPrintMessage(odError,"=> Err. SendToDriver(kcurrentinstance,scale,blockkey,). p_e->data2 > 127.");
			p_e->data2 = 127;
			}
		c1 = ByteToInt(p_e->data1);
		c2 = ByteToInt(p_e->data2);
		p_e->type = RAW_EVENT;
		p_e->data2 = c1;
		midibyte = status;
		if(WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
		// Normally, MIDI files accept running status, but our procedure doesn't allow the accumulation of large streams.
		midibyte = c1;
		if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
		if(c0 != ChannelPressure && c0 != ProgramChange) {
			p_e->time = time / Time_res; // ???
			p_e->type = RAW_EVENT;
			p_e->data2 = c2;
			midibyte = c2;
			if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
			}
		else if(Beta) BPPrintMessage(odError,"=> Err. SendToDriver(kcurrentinstance,scale,blockkey,). c0 == ChannelPressure");
		}
	SORTIR:
	return(OK);
	}


int AllNotesOffPedalsOffAllChannels(void) {
	int rs,key,channel;
	unsigned char midiData[4];
	int dataSize = 3;
	if(!rtMIDI) {
		BPPrintMessage(odError,"=> All Notes Off won't work since MIDI output is not active");	
		return(OK);
		}
	if(TraceMIDIinteraction) BPPrintMessage(odInfo,"Sending AllNotesOff and resetting controls on all channels\n");
	/* We can afford to mute the current output and send NoteOffs at a low level */
	// Mute++;
	for(channel=0; channel < MAXCHAN; channel++) {
		WaitABit(10); // Wait for 10 ms
		midiData[0] = ControlChange + channel;
		midiData[1] = 123; // All Notes Off
		midiData[2] = 0;
		sendMIDIEvent(0,0,midiData,dataSize,0); // Sending immediately
		midiData[0] = ControlChange + channel;
		midiData[1] = 64; // Pedal Off
		midiData[2] = 0;
		sendMIDIEvent(0,0,midiData,dataSize,0); // Sending immediately
		}
	// Mute--;
	WaitABit(10);
	return(OK);
	}


int SetMIDIPrograms(void) {
	int i,p,rs;
	MIDI_Event e;

	for(i=1; i <= 16; i++) {
		p = CurrentMIDIprogram[i];
		if(p > 0 && p <= 128) {
			ChangedMIDIprogram = TRUE;
			e.time = Tcurr;
			e.type = TWO_BYTE_EVENT;
			e.status = ProgramChange + i - 1;
			e.data2 = p - 1;
			rs = 0;
	#if WITH_REAL_TIME_MIDI_FORGET_THIS
			if(IsMidiDriverOn() && !InitOn)
				SendToDriver(kcurrentinstance,scale,blockkey,Tcurr * Time_res,0,&rs,&e);
	#endif
			}
		}
	return(OK);
	}


/* ??? Not sure what the purpose of this functions is.  To prevent sending
   too many Midi messages ?  CoreMIDI driver sets MaxMIDIbytes = LONG_MAX,
   so not sure if this function is likely to ever do anything.
   -- akozar 20130830
   Apparently a reminiscence of the OMS system -- Bernard 2024-06-19
 */
/* int CheckMIDIbytes(int tell)
{
unsigned long drivertime;
long formertime,timeleft;
int rep,compiledmem;

return OK; // 2024-06-19

if(Nbytes > (MaxMIDIbytes / 2) && Tbytes2 == ZERO) {
	// HideWindow(Window[wInfo]); // HideWindow(Window[wMessage]);
	Tbytes2 = Tcurr;
	}
if(Nbytes > MaxMIDIbytes) {
	// HideWindow(Window[wInfo]); // HideWindow(Window[wMessage]);
	// drivertime = GetDriverTime();
	formertime = ZERO;
	while((timeleft = Tbytes2 - drivertime) > ZERO) {
		if((timeleft * Time_res / 1000L) != formertime && tell) {
			formertime = timeleft * Time_res / 1000L;
			my_sprintf(Message,"Idling (%ld sec)",(long)formertime + 1L);
			PleaseWait();
			ShowMessage(FALSE,wMessage,Message);
			}
		if((rep=ListenToEvents()) == ABORT || rep == ENDREPEAT
			|| rep == EXIT) return(rep);
//		drivertime = GetDriverTime();
		}
	// HideWindow(Window[wMessage]);
	Tbytes2 = ZERO; Nbytes = MaxMIDIbytes/2;
	}
return(OK);
} */

int CaptureMidiEvent(Milliseconds time,int nseq,MIDI_Event *p_e)
{
	MIDIcode **ptr;
	Milliseconds currenttime;
	long size;

	size = (long) MyGetHandleSize((Handle)Stream.code);
	size = (size / sizeof(MIDIcode)) - 5L;
	if(Stream.i >= size) {
		ptr = Stream.code;
		if((ptr = (MIDIcode**)IncreaseSpace((Handle)ptr)) == NULL) return MISSED;
		Stream.code = ptr;
		}
	if(Stream.i == ZERO) {
		currenttime = ZERO;
		DataOrigin = time;
		}
	else currenttime = time - DataOrigin;
	switch(p_e->type) {
		case RAW_EVENT:
			(*Stream.code)[Stream.i].time = currenttime;
			(*Stream.code)[Stream.i].byte = ByteToInt(p_e->data2);
			(*Stream.code)[Stream.i].sequence = nseq;
			Stream.i++;
			break;
		case TWO_BYTE_EVENT:
			(*Stream.code)[Stream.i].time = currenttime;
			(*Stream.code)[Stream.i].byte = ByteToInt(p_e->status);
			(*Stream.code)[Stream.i].sequence = nseq;
			Stream.i++;
			(*Stream.code)[Stream.i].time = currenttime;
			(*Stream.code)[Stream.i].byte = ByteToInt(p_e->data2);
			(*Stream.code)[Stream.i].sequence = nseq;
			Stream.i++;
			break;
		case NORMAL_EVENT:
			(*Stream.code)[Stream.i].time = currenttime;
			(*Stream.code)[Stream.i].byte = ByteToInt(p_e->status);
			(*Stream.code)[Stream.i].sequence = nseq;
			Stream.i++;
			(*Stream.code)[Stream.i].time = currenttime;
			(*Stream.code)[Stream.i].byte = ByteToInt(p_e->data1);
			(*Stream.code)[Stream.i].sequence = nseq;
			Stream.i++;
			(*Stream.code)[Stream.i].time = currenttime;
			(*Stream.code)[Stream.i].byte = ByteToInt(p_e->data2);
			(*Stream.code)[Stream.i].sequence = nseq;
			Stream.i++;
	}
	
	return OK;
}

void RegisterProgramChange(MIDI_Event *p_e)
{
	int j, thisevent, channel, program;
	short itemtype;
//	ControlHandle itemhandle;
	Rect r;
	
	// Register program change to the MIDI orchestra
	channel = ByteToInt(p_e->status) % 16;
	thisevent = ByteToInt(p_e->status) - channel;
	if(thisevent == ProgramChange) {
		program = ByteToInt(p_e->data2) + 1;
		if(CurrentMIDIprogram[channel+1] != program) {
			CurrentMIDIprogram[channel+1] = program;
		}
	}
}
