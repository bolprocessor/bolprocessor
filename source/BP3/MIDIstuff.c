/* MIDIstuff.c (BP3) */ 

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


#include "-BP2.h"
#include "-BP2decl.h"

int trace_midi_filter = 0;
int trace_driver = 0;

int read_midisetup() {
    char *itemType, *itemIndex, *itemNumber, *portName, line[MAXLIN];
	char *key, *value;
    FILE *file;
	unsigned long long_value;
	int i, int_value, index;
	size_t len;
    int result = FALSE;
	MaxInputPorts = MaxOutputPorts = 0;
	for(i = 0; i < MAXPORTS; i++) {
		strcpy(InputMIDIportName[i],"");
		strcpy(OutputMIDIportName[i],"");
		MIDIoutput[i] = 0;  MIDIinput[i] = 1;
		}
    file = fopen(Midiportfilename,"r");
    if(file != NULL) {
        BPPrintMessage(odInfo,"Reading the content of %s\n",Midiportfilename);
        while(fgets(line, sizeof(line), file) != NULL) {
		//	BPPrintMessage(odInfo,"%s\n",line);
            itemType = strtok(line, "\t");
            itemIndex = strtok(NULL, "\t");
            itemNumber = strtok(NULL, "\t");
            portName = strtok(NULL, "\n");
			strip_newline(itemType);
			strip_newline(itemIndex);
			strip_newline(itemNumber);
			strip_newline(portName);
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
            if(strcmp(itemType,"MIDIinput") == 0) {
				if(itemNumber && strlen(itemNumber) > 0) {
					MIDIinput[index] = atoi(itemNumber);
					if(strlen(portName) > 0) strcpy(InputMIDIportName[index],portName);
					else strcpy(InputMIDIportName[index],"???");
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
					if(strlen(portName) > 0) strcpy(OutputMIDIportName[index],portName);
					else strcpy(OutputMIDIportName[index],"???");
					if((index + 1) > MaxOutputPorts) MaxOutputPorts = index + 1;
					result = OK; // We need at least one MIDI output port
					}
				else MIDIoutput[index] = -1;
				}
			if(strcmp(itemType,"MIDIinputFilter") == 0) {
        		if(itemNumber && strlen(itemNumber) > 0) {
					long_value = 0L;
            		for(i = 0; i < 18; i++) {
						if(itemNumber[i] != '0' && itemNumber[i] != '1') {
							BPPrintMessage(odError,"Non-binary digit found at location %d in filter: “%s”\nLine: %s\n",i,itemNumber,line);
							return FALSE;
							}
                		long_value = (long_value * 2L) + (itemNumber[i] - '0');
            			}
					MIDIinputFilter[index] = long_value;
					GetInputFilterWord(index);
					}
				}
			if(strcmp(itemType,"MIDIoutputFilter") == 0) {
        		if(itemNumber && strlen(itemNumber) > 0) {
					long_value = 0L;
            		for(i = 0; i < 18; i++) {
						if(itemNumber[i] != '0' && itemNumber[i] != '1') {
							BPPrintMessage(odError,"Non-binary digit found at location %d in filter: “%s”\nLine: %s\n",i,itemNumber,line);
							return FALSE;
							}
                		long_value = (long_value * 2L) + (itemNumber[i] - '0');
            			}
					MIDIoutputFilter[index] = long_value;
				//	BPPrintMessage(odInfo,"Lu: MIDIoutputFilter[%d] = %ld = %s\n",index,MIDIoutputFilter[index],itemNumber);
					GetOutputFilterWord(index);
					}
        		}
			}
        fclose(file);
		BPPrintMessage(odInfo,"Your MIDI settings:\n");
		for(index = 0; index < MaxOutputPorts; index++) {
			BPPrintMessage(odInfo,"MIDI output [%d] = %d: “%s”\n",index,MIDIoutput[index],OutputMIDIportName[index]);
			}
		for(index = 0; index < MaxInputPorts; index++) {
			BPPrintMessage(odInfo,"MIDI input [%d] = %d: “%s”\n",index,MIDIinput[index],InputMIDIportName[index]);
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
    thefile = fopen(Midiportfilename,"w");
    if(thefile != NULL) {
        BPPrintMessage(odInfo,"MIDI settings saved to %s\n",Midiportfilename);
		for(index = 0; index < MaxOutputPorts; index++) {
			if(strlen(OutputMIDIportName[index]) == 0) continue;
			fprintf(thefile, "MIDIoutput\t%d\t%d\t%s\n",index,MIDIoutput[index],OutputMIDIportName[index]);
			}
		for(index = 0; index < MaxInputPorts; index++) {
			if(strlen(InputMIDIportName[index]) == 0) continue;
			fprintf(thefile, "MIDIinput\t%d\t%d\t%s\n",index,MIDIinput[index],InputMIDIportName[index]);
			SetInputFilterWord(index);
			binaryString = longToBinary((unsigned long)MIDIinputFilter[index]);
        	fprintf(thefile, "MIDIinputFilter\t%d\t%s\n",index,binaryString);
			free(binaryString);
			SetOutputFilterWord(index);
			binaryString = longToBinary((unsigned long)MIDIoutputFilter[index]);
        	fprintf(thefile, "MIDIoutputFilter\t%d\t%s\n",index,binaryString);
			free(binaryString);
			}
        fclose(thefile);
        }
    }

char* longToBinary(unsigned long num) {
    int totalBits = 18; // The total number of bits in the output
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
    int result,size;
    size = sizeof(MIDI_Event);
	
	// oldtimestopped = 0L;
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
            time = eventStack[i].time + TimeStopped;
            sendMIDIEvent(midiData,dataSize,time);
            // Move remaining events forward
            memmove(&eventStack[i], &eventStack[i + 1], (eventCount - i - 1) * size);
            eventCount--;
            }
        else i++;
        }
    return OK;
    }

int MaybeWait(unsigned long current_time) {
	unsigned long time,time_now;
	int result,i;
	if(FirstMIDIevent) time = 0L;
	else time = current_time;
	check_stop_instructions(time); // This may set StopPlay to TRUE
	time_now = getClockTime(); // microseconds
	i = 0;
	while(StopPlay) { // The proper input MIDI event will end this loop, setting StopPlay to FALSE
		if((result = stop(1,"Waiting loop")) != OK) return result;
		WaitABit(5); // milliseconds
		i++;
		if(i == 100) AllNotesOffPedalsOffAllChannels();
		}
	TimeStopped += (getClockTime() - time_now);
	if((TimeStopped / 10000L) != (Oldtimestopped / 10000L)) {
		if(TraceMIDIinput) BPPrintMessage(odInfo,"TimeStopped = %ul ms\n",TimeStopped / 1000L);
		}
	Oldtimestopped = TimeStopped;
	return OK;
	}


int ListenMIDI(int x0, int x1, int x2) {
	int r = OK;
	if(EmergencyExit || Panic) return(ABORT); // 2024-05-03
	if((r=stop(0,"ListenMIDI")) != OK) return r;
	return(r);
	}


int HandleInputEvent(const MIDIPacket* packet,MIDI_Event* e,int index) {
	int x0,x1,x2,filter,c,c0,c1,c2,channel,i,j,r,idummy,eventfound;
	long jdummy;
	unsigned long time_now,thisscripttime;
	char **p_line;

	x0 = x1 = x2 = 0;
	filter = x0 + x1 + x2;  // Will be used later

	STARTCHECK:
	if(packet == NULL) return OK;
	// if(!AcceptEvent(ByteToInt(packet->data[0]),index)) return OK;
	if (packet->length > 0) {
		e->type = packet->data[0];  // Assuming data[0] is the status byte
		e->time = packet->timeStamp;
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
	if(!ThreeByteChannelEvent(c)) {  // REVISE THIS!
		RunningStatus = 0;
		if(c0 < 128) return(OK);
		if(c0 == SystemExclusive) {
			do {
				e->time = 0;
				e->type = RAW_EVENT;
				e->data2 = c0;
		/*		if(!Oms && SysExPass) DriverWrite(ZERO,0,&e);
				if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) break; */
				c0 = e->data2;
				}
			while(c0 != EndSysEx && e->type != NULL_EVENT);
			e->time = 0;
			e->type = RAW_EVENT;
			e->data2 = c0;
		//	if(!Oms && SysExPass) DriverWrite(ZERO,0,&e);
			return(OK);
			}
		if(c0 == SongPosition) {
			for(i=0; i < 2; i++) {
				e->time = 0;
				e->type = RAW_EVENT;
				e->data2 = c0;
		/*		if(!Oms && SongPosPass) DriverWrite(ZERO,0,&e);
				if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) break; */
				c0 = e->data2;
				}
			e->time = 0;
			e->type = RAW_EVENT;
			e->data2 = c0;
		//	if(!Oms && SongPosPass) DriverWrite(ZERO,0,&e);
			return(OK);
			}
		if(c0 == SongSelect) {
			e->time = 0;
			e->type = RAW_EVENT;
			e->data2 = c0;
		/*	if(!Oms && SongSelPass) DriverWrite(ZERO,0,&e);
			if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) return(OK); */
			c0 = e->data2;
			e->time = 0;
			e->type = RAW_EVENT;
			e->data2 = c0;
		//	if(!Oms && SongSelPass) DriverWrite(ZERO,0,&e);
			return(OK);
			}
		if(c == ProgramChange) {
			e->time = 0;
			e->type = RAW_EVENT;
			e->data2 = c0;
		//	if(!Oms && ProgramTypePass) DriverWrite(ZERO,0,&e);
		//	if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) return(OK);
			sprintf(Message,"%ld",(long)(e->data2) + ProgNrFrom);
			if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
			e->time = 0;
			e->type = RAW_EVENT;
		//	if(!Oms && ProgramTypePass) DriverWrite(ZERO,0,&e);
			sprintf(Message,"%ld",(long)((c0 % 16) + 1));
			if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,1,Message);
			if(!ScriptExecOn) AppendScript(71);
			return(OK);
			}
		if(c == ChannelPressure) {
			e->type = RAW_EVENT;
		//	if(GetNextMIDIevent(&e,FALSE,FALSE) != OK) return(OK);
			c1 = e->data2;
			e->time = 0;
			e->type = TWO_BYTE_EVENT;
			e->status = c;
		//	if(!Oms && ChannelPressurePass) DriverWrite(ZERO,0,&e);
			if(Jcontrol < 0) {
				sprintf(Message,"Pressure = %ld channel %ld",(long)c1,
					(long)(c0 - c + 1));
				if(Interactive && ShowMessages) ShowMessage(TRUE,wMessage,Message);
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

	RunningStatus = c0;
	c1 = e->data1;
	c2 = e->data2;

	INTERPRET:
	c = c0 - c0 % 16;
	if(filter && c2 != 0 && (x0 == 0 || x0 == c0) && (x1 == 0 || x1 == c1)
				&& (x2 == 0 || x2 == c2)) return(RESUME);

/*	if((Interactive || ScriptRecOn || ReadKeyBoardOn) && c == ControlChange && c1 > 95
			&& c1 < 122) {
		// Undefined controllers
		if(!ScriptExecOn) {
			sprintf(Message,"%ld",(long)c1);
			MystrcpyStringToTable(ScriptLine.arg,0,Message);
			sprintf(Message,"%ld",(long)c2);
			MystrcpyStringToTable(ScriptLine.arg,1,Message);
			sprintf(Message,"%ld",(long)(c0 - c + 1));
			MystrcpyStringToTable(ScriptLine.arg,2,Message);
			AppendScript(75);
			}
		sprintf(LineBuff,"Controller #%ld = %ld channel %ld",(long)c1,(long)c2,
			(long)c0-c+1);
		if(Interactive && ShowMessages) ShowMessage(TRUE,wMessage,LineBuff);
		return(OK);
		} */
	if((Interactive || ScriptRecOn) && c == ChannelMode && c1 > 121) {
	//	LineBuff[0] = '\0';
		strcpy(LineBuff,""); // Fixed by BB 2021-02-14

		sprintf(Message,"%ld",(long)(c0 - c + 1));
		if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
		switch(c1) {
			case 122:
				if(c2 == 0) {	/* Local control off */
					AppendScript(76); sprintf(LineBuff,"Local control off channel %ld",
						(long)(c0-c+1));
					break;
					}
				if(c2 == 127) {	/* Local control on */
					AppendScript(77); sprintf(LineBuff,"Local control on channel %ld",
						(long)(c0-c+1));
					break;
					}
				break;
			case 123: /* if(c2 == 0) AppendScript(78) */ break;	/* All notes off */
			case 124: if(c2 == 0) {	/* Omni mode off */
				AppendScript(79); sprintf(LineBuff,"Omni mode off channel %ld",(long)(c0-c+1));
				break;
				}
			case 125: if(c2 == 0) {	/* Omni mode on */
				AppendScript(80); sprintf(LineBuff,"Omni mode on channel %ld",(long)(c0-c+1));
				break;
				}
			case 127: if(c2 == 0) {	/* Poly mode on */
				AppendScript(82); sprintf(LineBuff,"Poly mode on channel %ld",(long)(c0-c+1));
				break;
				}
			case 126:	/* Mono mode on, c2 channels */
				sprintf(LineBuff,"Mono mode on (%ld channels) channel %ld",(long)c2,
					(long)(c0-c+1));
				sprintf(Message,"%ld",(long)(c0 - c + 1));
				if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,1,Message);
				sprintf(Message,"%ld",(long)c2);
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
				sprintf(Message,"Pitchbend = %ld channel %ld",(long) c1 + 128L*c2,
					(long)(c0 - c + 1));
				if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
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
					sprintf(Message,"Volume = %ld channel %ld",(long)c2,
						(long)(c0 - c + 1));
					if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
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
					sprintf(Message,"Panoramic = %ld channel %ld",(long)c2,
						(long)(c0 - c + 1));
					if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
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
				sprintf(Message,"Modulation = %ld channel %ld",(long)OldModulation,
					(long)(c0 - c + 1));
				if(ShowMessages && Jcontrol == -1) ShowMessage(TRUE,wMessage,Message);
				return(OK);
				}
			else
			if(c1 == 33) {	/* Modulation LSB */
				if(Jcontrol == -1) {
					sprintf(Message,"Modulation = %ld channel %ld",(long)(c2+OldModulation),
						(long)(c0 - c + 1));
					if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
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
					sprintf(Message,"%ld",(long)(c0-c+1));
					MystrcpyStringToTable(ScriptLine.arg,1,Message);
					}
				if(c2 == 0) {
					sprintf(LineBuff,"Switch %ld OFF channel %ld",(long)c1,(long)(c0-c+1));
					sprintf(Message,"%ld",(long)c1);
					if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
					AppendScript(87);
					}
				else {
					sprintf(LineBuff,"Switch %ld ON channel %ld",(long)c1,(long)(c0-c+1));
					sprintf(Message,"%ld",(long)c1);
					if(!ScriptExecOn) MystrcpyStringToTable(ScriptLine.arg,0,Message);
					AppendScript(86);
					}
				if(ShowMessages && Jcontrol == -1) ShowMessage(TRUE,wMessage,LineBuff);
				return(OK);
				}
			}
		}
	if(Interactive && (Ctrl_adjust(e,c0,c1,c2) == OK)) {
		if(e->type == NULL_EVENT) return(OK);
		RunningStatus = 0;
		c0 = e->data2;
		goto STARTCHECK;
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
			HideWindow(Window[wInfo]);
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
			sprintf(Message,"Playing %ld times... (",(long)Nplay);
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
			if(TraceMIDIinput) BPPrintMessage(odInfo,"Received NoteOn key = %d channel %d, checking %d script(s)\n",c1,channel,Jinscript);
			if(FirstMIDIevent) {
				if(TraceMIDIinput) BPPrintMessage(odInfo,"time_now = 0L in HandleInputEvent()\n");
				time_now = 0L;
				}
			else time_now = getClockTime() - initTime; // microseconds
			// Find the next expected NoteOn
			for(j=1; j <= Jinscript; j++) {
				if(((*p_INscript)[j]).chan == -1) { // This is a deactivated instruction
					if(j == Jinscript) Jinscript = 0; // No need to try later
					continue;
					}
				thisscripttime = ((*p_INscript)[j]).time + TimeStopped;
				// We won't verify that velocity (c2) is greater than zero, because a NoteOn with velocity zero can be used as a soundless instruction
				if(channel == ((*p_INscript)[j]).chan && c1 == ((*p_INscript)[j]).key && time_now >= thisscripttime) {
					if(TraceMIDIinput) BPPrintMessage(odInfo,"[%d] Good NoteOn key = %d\n",j,c1);
					if(TraceMIDIinput) BPPrintMessage(odInfo,"thisscripttime = %ul, time_now = %ul\n",thisscripttime / 1000L, time_now/1000L);
					StopPlay = FALSE;
					TimeStopped +=  1000 * MIDIsyncDelay; // Necessary to restore the timing of the next events
					((*p_INscript)[j]).chan = -1; // This input event is now deactivated
					return OK;
					}
				else return OK;
				}
			}
		}}}}}}}}}
		if(ParamControlChan > 0) {
			for(i=1; i < MAXPARAMCTRL; i++) {
				if(ParamChan[i] != -1 && ParamKey[i] == c1
									&& c0 == (NoteOn + ParamChan[i] - 1)) {
				/* IN Param key �Kx� = velocity �note� channel �1..16� */
					sprintf(Message,"K%ld = %ld",(long)i,(long)c2);
					ShowMessage(TRUE,wMessage,Message);
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
	int j;
	unsigned long thisscripttime;
	for(j=1; j <= Jinscript; j++) {
		if(((*p_INscript)[j]).chan == -1) continue;
		if(((*p_INscript)[j]).scriptline != 97) continue;
		thisscripttime = ((*p_INscript)[j]).time + TimeStopped;
		if(thisscripttime > time) continue;
		StopPlay = TRUE;
		Notify("Waiting for a note specified in score");
		if(time == 0L) {
			FirstMIDIevent = FALSE;
			initTime = (UInt64) getClockTime();
			if(TraceMIDIinput) BPPrintMessage(odInfo,"\nFirst MIDI event (script) at %ld ms\n",(long)time);
			}
		if(TraceMIDIinput) BPPrintMessage(odInfo,"Stopped sound at time %ld ms <= %ld ms as per instruction %d\n",(long)thisscripttime / 1000L,(long)time / 1000L,j);
		// BPPrintMessage(odInfo,"FirstMIDIevent = %d\n",FirstMIDIevent);
		break;
		}
	return OK;
	}

int Ctrl_adjust(MIDI_Event *p_e,int c0,int c1,int c2) {
	int speed_change,i,j,r,c11;
	long count = 12L,oldn,dt;

	r = MISSED;
	if(ParamControlChan > 0) {
		for(i=1; i < MAXPARAMCTRL; i++) {
			if(ParamChan[i] != -1 && c0 == (ControlChange + ParamChan[i] - 1)
				&& ParamControl[i] == c1) {
			/* IN Param �Kx� = controller #�0..127� channel �1..16� */
				ParamControlChan = ParamChan[i];
				sprintf(Message,"K%ld = %ld",(long)i,(long)c2);
				ShowMessage(TRUE,wMessage,Message);
				ParamValue[i] = c2;
				r= OK;
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
			ShowWindow(Window[wMetronom]);
			BringToFront(Window[wMetronom]);
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
		sprintf(Message,"Reset weights: %s",Reality[ResetWeights]);
		ShowMessage(TRUE,wMessage,Message);
		}
	else {
		if(UseEachSubChan > 0 && SUBthere && c1 == UseEachSubKey
			&& c0 == (NoteOn + UseEachSubChan - 1)) {
			UseEachSub = 1 - UseEachSub; if(!Improvize) 
			sprintf(Message,"Play each substitution: %s",Reality[UseEachSub]);
			ShowMessage(TRUE,wMessage,Message);
			}
		else {
			if(SynchronizeStartChan > 0 && c1 == SynchronizeStartKey
				&& c0 == (NoteOn + SynchronizeStartChan - 1)) {
				SynchronizeStart = 1 - SynchronizeStart; if(!Improvize) 
				sprintf(Message,"Synchronize start: %s",Reality[SynchronizeStart]);
				ShowMessage(TRUE,wMessage,Message);
				}
			else {
				if(c1 == SetTimeKey && SetTimeChan > 0
					&& Improvize && c2 > 0
						&& c0 == (NoteOn + SetTimeChan - 1)) {
					TimeMax = (long) c2 * 472;
					/* UseTimeLimit = */ LimCompute = LimTimeSet = TRUE;
					sprintf(Message,
						"Max computation time: %4.2f s.",(double) TimeMax/1000.);
					ShowMessage(TRUE,wMessage,Message);
					}
				else {
					if(StriatedChan > 0 && c1 == StriatedKey
								&& c0 == (NoteOn + StriatedChan - 1)) {
						Nature_of_time = 1 - Nature_of_time;
						sprintf(Message,
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
							sprintf(Message,
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
								ShowWindow(Window[wMetronom]);
								BringToFront(Window[wMetronom]);
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
	if(StartTypeIn[i]) ContTypeIn[i] = TRUE;
	if(ContTypeIn[i]) StartTypeIn[i] = TRUE;
	if(NoteOnIn[i]) NoteOffIn[i] = TRUE;
	if(NoteOffIn[i]) NoteOnIn[i] = TRUE;
	MIDIinputFilter[i] =
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
	if(StartTypePass[i]) ContTypePass[i] = TRUE;
	if(ContTypePass[i]) StartTypePass[i] = TRUE;
	if(NoteOnPass[i]) NoteOffPass[i] = TRUE;
	if(NoteOffPass[i]) NoteOnPass[i] = TRUE;
	MIDIoutputFilter[i] =
		ResetPass[i] + 2L * (ActiveSensePass[i] + 2L * (ContTypePass[i] + 2L * (StartTypePass[i] + 2L * (ClockTypePass[i]
		+ 2L * (EndSysExPass[i] + 2L * (TuneTypePass[i] + 2L * (SongSelPass[i] + 2L * (SongPosPass[i]
		+ 2L * (TimeCodePass[i] + 2L * (SysExPass[i] + 2L * (PitchBendPass[i] + 2L * (ChannelPressurePass[i]
		+ 2L * (ProgramTypePass[i] + 2L * (ControlTypePass[i] + 2L * (KeyPressurePass[i] + 2L * (NoteOnPass[i]
		+ 2L * NoteOffPass[i]))))))))))))))));
	/* To pass an event you should enable the driver to receive it... */
	MIDIinputFilter[i] = MIDIinputFilter[i] | MIDIoutputFilter[i];
	GetInputFilterWord(i);
//	BPPrintMessage(odInfo,"@@@ NoteOnPass = %d\n",NoteOnPass);
	return(OK);
	}


int GetInputFilterWord(int i) {
	long n = 1L;
    ResetIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    ActiveSenseIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    ContTypeIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    StartTypeIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    ClockTypeIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    EndSysExIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    TuneTypeIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    SongSelIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    SongPosIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    TimeCodeIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    SysExIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    PitchBendIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    ChannelPressureIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    ProgramTypeIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    ControlTypeIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    KeyPressureIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    NoteOnIn[i] = (MIDIinputFilter[i] & n) != 0; n <<= 1;
    NoteOffIn[i] = (MIDIinputFilter[i] & n) != 0; // Last use of n
/*	BPPrintMessage(odInfo,"@@@ MIDIinputFilter[%d] = %ld\n",i,MIDIinputFilter[i]);
	BPPrintMessage(odInfo,"@@@ NoteOnIn[%d] = %d\n",i,NoteOnIn[i]); */
	return(OK);
	}


int GetOutputFilterWord(int i) {
	long n = 1L;
 	ResetPass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    ActiveSensePass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    ContTypePass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    StartTypePass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    ClockTypePass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    EndSysExPass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    TuneTypePass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    SongSelPass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    SongPosPass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    TimeCodePass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    SysExPass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    PitchBendPass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    ChannelPressurePass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    ProgramTypePass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    ControlTypePass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    KeyPressurePass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    NoteOnPass[i] = (MIDIoutputFilter[i] & n) != 0; n <<= 1;
    NoteOffPass[i] = (MIDIoutputFilter[i] & n) != 0; // Last use of n
/*	char* binaryStr = printBinary18(MIDIoutputFilter[i],18);
	BPPrintMessage(odInfo,"MIDIoutputFilter[%d] = %s = %ld\n",i,binaryStr,MIDIoutputFilter[i]);
	BPPrintMessage(odInfo,"NoteOnPass[%d] = %d\n",i,NoteOnPass[i]);
	free(binaryStr); */
	return(OK);
	}

int ResetMIDIFilter(void) {
	int i;
	for(i= 0; i < MaxInputPorts; i++) {
		MIDIinputFilter[i] = MIDIinputFilterstartup;
		MIDIoutputFilter[i] = MIDIoutputFilterstartup;
		GetInputFilterWord(i);
		GetOutputFilterWord(i);
		SetOutputFilterWord(i);	/* Verifies consistency */
		}
	if(TraceMIDIinput) BPPrintMessage(odInfo,"MIDI filter has been reset\n");
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
	BPPrintMessage(odInfo,"NoteOnIn[%d] = %d\n",i,NoteOnIn[i]);
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
		case Continue:
		case Stop:
			if(StartTypeIn[i]) return(YES);
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


int PassEvent(int c, int i) {
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
		case Continue:
		case Stop:
			if(StartTypePass[i]) return(YES);
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
		sprintf(Message,"\nCan't send %ld as MIDI data.\n",(long)n);
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

#if WITH_REAL_TIME_MIDI_FORGET_THIS  // FIXME? do we need to reset BP's internal values anyways? - akozar
#if BP_CARBON_GUI_FORGET_THIS
SwitchOn(NULL,wControlPannel,bResetControllers);
#endif /* BP_CARBON_GUI_FORGET_THIS */

rs = 0;	/* Running status */

// givetime = (!Improvize || !ComputeOn) && !MIDIfileOn;

if(now) tcurr = ZERO;
else tcurr = Tcurr;

result = ABORT;
for(ch=0; ch < 16; ch++) {
	if(/* force || */ ChangedSwitch[ch]) {	/* 'force' suppressed on 18/11/97 */
		ChangedSwitch[ch] = FALSE;
		for(j=0; j < 32; j++) {	/* Switches */
			e.time = tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + ch;
			e.data1 = 64 + j;
			e.data2 = 0;
			if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
			if(givetime) Tcurr += 10L / Time_res;
			}
		}
	if(force || ChangedPitchbend[ch]) {
		ChangedPitchbend[ch] = FALSE;
		e.time = tcurr;	/* Pitchbend */
		e.type = NORMAL_EVENT;
		e.status = PitchBend + ch;
		(*p_Oldvalue)[ch].pitchbend = DEFTPITCHBEND;
		lsb = ((long)DEFTPITCHBEND) % 128;
		msb = (((long)DEFTPITCHBEND) - lsb) >> 7;
		e.data1 = lsb;
		e.data2 = msb;
		if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
		if(givetime) Tcurr += 10L / Time_res;
		}
	if(force || ChangedPressure[ch]) {
		ChangedPressure[ch] = FALSE;
		e.time = tcurr;	/* Pressure */
		e.type = TWO_BYTE_EVENT;
		e.status = ChannelPressure + ch;
		(*p_Oldvalue)[ch].pressure = DEFTPRESSURE;
		e.data2 = DEFTPRESSURE;
		if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
		if(givetime) Tcurr += 10L / Time_res;
		}
	if(force || ChangedModulation[ch]) {
		ChangedModulation[ch] = FALSE;
		e.time = tcurr;	/* Modulation */
		e.type = NORMAL_EVENT;
		e.status = ControlChange + ch;
		(*p_Oldvalue)[ch].modulation = DEFTMODULATION;
		lsb = ((long)DEFTMODULATION) % 128;
		msb = (((long)DEFTMODULATION) - lsb) >> 7;
		e.data1 = 1;
		e.data2 = msb;	/* MSB */
		if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
		e.time = tcurr;
		e.type = NORMAL_EVENT;
		e.status = ControlChange + ch;
		e.data1 = 33;
		e.data2 = lsb;	/* LSB */
		if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
		if(givetime) Tcurr += 10L / Time_res;
		}
	if(force || ChangedVolume[ch]) {
		ChangedVolume[ch] = FALSE;
		e.time = tcurr;	/* Volume */
		e.type = NORMAL_EVENT;
		e.status = ControlChange + ch;
		(*p_Oldvalue)[ch].volume = DeftVolume;
		e.data1 = VolumeControl[ch+1];
		e.data2 = DeftVolume;
		if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
		if(givetime) Tcurr += 10L / Time_res;
		}
	if(force || ChangedPanoramic[ch]) {
		ChangedPanoramic[ch] = FALSE;
		e.time = tcurr;	/* Panoramic */
		e.type = NORMAL_EVENT;
		e.status = ControlChange + ch;
		(*p_Oldvalue)[ch].panoramic = DeftPanoramic;
		e.data1 = PanoramicControl[ch+1];
		e.data2 = DeftPanoramic;
		if(SendToDriver(tcurr * Time_res,0,&rs,&e) != OK) goto OUT;
		if(givetime) Tcurr += 10L / Time_res;
		}
	}
HideTicks = FALSE;
if(now) WaitABit(500L);
result = OK;

OUT:
#if BP_CARBON_GUI_FORGET_THIS
SwitchOff(NULL,wControlPannel,bResetControllers);
#endif /* BP_CARBON_GUI_FORGET_THIS */
return(result);
}
#endif

#if BP_CARBON_GUI_FORGET_THIS
// I'm not sure whether we need PlayPrototypeTicks() and RecordTick()
// in ANSI console build, but seems unlikely.  - akozar

int PlayPrototypeTicks(int j)
{
MIDI_Event e;
/* long count = 12L;
MIDI_Parameters parms; */
int key,duration,channel,velocity,rep,rs;
double r,x,kx;
long p,q;
char line[MAXFIELDCONTENT];
unsigned long drivertime;

if(SoundOn || !rtMIDI || CheckEmergency() != OK) return(MISSED);

#if WITH_REAL_TIME_MIDI_FORGET_THIS
rep = NO;
if(!GetCtrlValue(wPrototype5,bPlayPrototypeTicks)) goto DOIT;
key = PrototypeTickKey;
channel = PrototypeTickChannel - 1;
velocity = PrototypeTickVelocity;
if(rep == OK) rep = GetField(NULL,TRUE,wPrototype5,fTref,line,&p,&q);
duration = p/q;
if(duration < EPSILON) {
	sprintf(Message,"You can't play ticks when Tref is only %ldms",(long)duration);
	Alert1(Message);
	rep = MISSED;
	}
if(j > 2 && j < Jbol && (*p_Tref)[j] != duration) {
	if(Answer("You want to play ticks although this object is not striated. Proceed anyway?",
		'Y') != OK) return(MISSED);
	}
duration = duration / Time_res / 2;

DOIT:
if(rep != OK) {
	ResetMIDI(FALSE);
//	#ifndef __POWERPC
	FlushEvents(everyEvent,0);
//	#endif
	Alert1("Recording MIDI input... Click when over");
	return(OK);
	}
else {
	SetCursor(&WatchCursor);
	FlashInfo("Receiving MIDI data. Click mouse when completed.");
	}
ResetMIDI(FALSE);
// #ifndef __POWERPC
FlushEvents(everyEvent,0);
// #endif
rs = 0;
while(TRUE) {
	e.type = NORMAL_EVENT;
	e.time = Tcurr;
	e.status = NoteOn + channel;
	e.data1 = key;
	e.data2 = velocity;
	SendToDriver(Tcurr * Time_res,0,&rs,&e);
	Tcurr += duration;
	e.type = NORMAL_EVENT;
	e.time = Tcurr;
	e.status = NoteOn + channel;
	e.data1 = key;
	e.data2 = 0;
	SendToDriver(Tcurr * Time_res,0,&rs,&e);
	drivertime = GetDriverTime();
	while((Tcurr > drivertime + (MIDIsetUpTime / Time_res)) && !Button()) {
		/* Wait for the end of preceding play */
		drivertime = GetDriverTime();
		}
	Tcurr += duration;
	if(Button()) break;
	}
while(Button());

FlushEvents(mDownMask+mUpMask,0);

HideWindow(Window[wInfo]);
return(OK);
#endif
}


int RecordTick(int where,int i)
{
MIDI_Event e;
int key,channel,c0,c1,velocity;
long count = 12L;
char line[MAXFIELDCONTENT];

if(!rtMIDI) {
	Alert1("Cannot record tick because MIDI output is inactive");
	return(MISSED);
	}
#if WITH_REAL_TIME_MIDI_FORGET_THIS
SetCursor(&WatchCursor);
ShowMessage(TRUE,wMessage,"Play tick on MIDI keyboard. (Click mouse to abort)");
key = -1;
ResetMIDI(FALSE);

FlushEvents(everyEvent,0);
ReadKeyBoardOn = TRUE;

while(!Button()) {
	MaintainCursor();
	if(GetNextMIDIevent(&e,TRUE,FALSE) != OK) break;
	c0 = e.data2;
	if(c0 < NoteOn || c0 >= (NoteOn+16)) continue;
	if(GetNextMIDIevent(&e,TRUE,FALSE) != OK) break;
	c1 = e.data2;
	if(GetNextMIDIevent(&e,TRUE,FALSE) != OK) break;
	velocity = e.data2;
	if(velocity > 0) { 	/* NoteOn */
		key = c1;
		channel = (c0 % 16) + 1;
		break;
		}
	}
	
ReadKeyBoardOn = FALSE;
FlushEvents(mDownMask+mUpMask,0);

HideWindow(Window[wMessage]);
SetDefaultCursor();

if(key > -1) {
	switch(where) {
		case 1:
			PrototypeTickKey = key;
			PrototypeTickChannel = channel;
			PrototypeTickVelocity = velocity;
			break;
		case 0:
			TickKey[i] = key;
			TickChannel[i] = channel;
			TickVelocity[i] = velocity;
			break;
		case 3:
			sprintf(line,"%ld",(long)velocity);
			SetField(NULL,wTickDialog,fThisTickVelocity,line);
			sprintf(line,"%ld",(long)key);
			SetField(NULL,wTickDialog,fThisTickKey,line);
			sprintf(line,"%ld",(long)channel);
			SetField(NULL,wTickDialog,fThisTickChannel,line);
			GetThisTick();
		}
	}
return(OK);
#endif
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

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
					sprintf(Message,
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
					sprintf(Message,
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
					sprintf(Message,
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
	case 8:	/* _pitchbend */
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
sprintf(Message,"%ld",(long)x);
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


int CheckMIDIOutPut(int channel)
{
MIDI_Event e;
int key,duration,minkey,maxkey,stepkey,ch,stop;
double r,x,kx;
unsigned long drivertime;

if (!rtMIDI) {
	Alert1("MIDI output is off,  Check the Drivers menu.");
	return(MISSED);
}

#if WITH_REAL_TIME_MIDI_FORGET_THIS
if(Mute) {
	Alert1("The 'Mute' button was checked on the control pannel...");
	Mute = FALSE;
	HideWindow(Window[wInfo]);
	MaintainMenus();
	BPActivateWindow(SLOW,wControlPannel);
	return(MISSED);
	}
if(SoundOn) {
	Alert1("BP3 is already sending messages to the MIDI output...");
	return(MISSED);
	}
ResetMIDIControllers(YES,YES,YES);

sprintf(Message,"Playing test on MIDI channel %ld... (Click to stop)",(long)(channel+1));
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

HideWindow(Window[wInfo]);
return(OK);
#endif
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

int SendToDriver(Milliseconds time,int nseq,int *p_rs,MIDI_Event *p_e) {
	long count = 12L;
	int c0,c1,c2,status,chan,result;
	unsigned long done;
	byte midibyte;

	LastTime = time;
	if(Panic || EmergencyExit) return(ABORT);
	if(!MIDIfileOn && !rtMIDI) return(OK);
	status = ByteToInt(p_e->status);
	if(rtMIDI) {
		// Sending to the event stack
        done = 0L;
    	if(trace_driver) BPPrintMessage(odInfo,"Sending MIDI event to stack, time = %ld ms,\tstatus = %ld,\tdata1 = %ld,\tdata2 = %ld\n",(long)time,(long)p_e->status,(long)p_e->data1,(long)p_e->data2);
		MIDIflush();
        while(eventCount > eventCountMax) {
			// The stack is full
            WaitABit(1); // Sleep for 1 millisecond
            if(MIDIflush() != OK) break;
        //  if(done++ == 0L) BPPrintMessage(odInfo,"Reached the limit of the buffer...\n");
            }
		eventStack[eventCount] = *p_e;
		eventStack[eventCount].time = 1000 * time;
		eventCount++;
		if(((status & 0xF0) == NoteOn) && FirstMIDIevent) {
			FirstMIDIevent = FALSE;
			initTime = (UInt64) getClockTime();
			if(TraceMIDIinput) BPPrintMessage(odInfo,"\nFirst MIDI event at %ld ms\n",(long)time);
			} 
		return(OK);
		}
	
	// The following is for MIDI files.
	if(p_e->type == RAW_EVENT || p_e->type == TWO_BYTE_EVENT) {
		if(p_e->type == TWO_BYTE_EVENT) {
			midibyte = p_e->status;
			if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
			}
		midibyte = p_e->data2;
		if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
		*p_rs = 0;
		goto OUT;
		}
	// The event is NORMAL_EVENT type
	/* Don't use running status when capturing event stream, or
	when sending to any MIDI driver that does not communicate
	directly with a Serial port (eg. OMS, CoreMIDI, etc.) */
	if(ItemCapture) *p_rs = 0;
	chan = status % 16;
	c0 = status - chan;
	if(trace_driver) BPPrintMessage(odInfo,"++ SendToDriver() time = %ld c0 = %d\tc1 = %d\tc2 = %d\n",(long)time,c0,ByteToInt(p_e->data1),ByteToInt(p_e->data2));
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
		/* Unexpectedly, the Roland D-50 seems to mess running status with ChannelMode */
		*p_rs = status;
		if(p_e->data1 > 127) {
			if(Beta) BPPrintMessage(odError,"=> Err. SendToDriver(). p_e->data1 > 127.");
			p_e->data1 = 127;
			}
		if(p_e->data2 > 127) {
			if(Beta) BPPrintMessage(odError,"=> Err. SendToDriver(). p_e->data2 > 127.");
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
			if(Beta) BPPrintMessage(odError,"=> Err. SendToDriver(). p_e->data1 > 127.");
			p_e->data1 = 127;
			}
		if(p_e->data2 > 127) {
			if(Beta) BPPrintMessage(odError,"=> Err. SendToDriver(). p_e->data2 > 127.");
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
			p_e->time = time / Time_res;
			p_e->type = RAW_EVENT;
			p_e->data2 = c2;
			midibyte = c2;
			if(MIDIfileOn && WriteMIDIbyte(time,midibyte) != OK) return(ABORT);
			}
		else if(Beta) BPPrintMessage(odError,"=> Err. SendToDriver(). c0 == ChannelPressure");
		}
	OUT:
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
	if(TraceMIDIinput) BPPrintMessage(odInfo,"Send AllNotesOff and reset controls on all channels\n");
	/* We can afford to mute the current output and send NoteOffs at a low level */
	// Mute++;
	for(channel=0; channel < MAXCHAN; channel++) {
		WaitABit(10); // Wait for 10 ms
		midiData[0] = ControlChange + channel;
		midiData[1] = 123; // All Notes Off
		midiData[2] = 0;
		sendMIDIEvent(midiData,dataSize,0); // Sending immediately
		midiData[0] = ControlChange + channel;
		midiData[1] = 64; // Pedal Off
		midiData[2] = 0;
		sendMIDIEvent(midiData,dataSize,0); // Sending immediately
		}
	// Mute--;
	WaitABit(10);
	return(OK);
	}


int SetMIDIPrograms(void)
{
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
			SendToDriver(Tcurr * Time_res,0,&rs,&e);
#endif
		}
	}
return(OK);
}


/* ??? Not sure what the purpose of this functions is.  To prevent sending
   too many Midi messages ?  CoreMIDI driver sets MaxMIDIbytes = LONG_MAX,
   so not sure if this function is likely to ever do anything.
   -- akozar 20130830
 */
int CheckMIDIbytes(int tell)
{
unsigned long drivertime;
long formertime,timeleft;
int rep,compiledmem;


if(Nbytes > (MaxMIDIbytes / 2) && Tbytes2 == ZERO) {
	HideWindow(Window[wInfo]); HideWindow(Window[wMessage]);
	Tbytes2 = Tcurr;
	}
if(Nbytes > MaxMIDIbytes) {
	HideWindow(Window[wInfo]); HideWindow(Window[wMessage]);
	// drivertime = GetDriverTime();
	formertime = ZERO;
	while((timeleft = Tbytes2 - drivertime) > ZERO) {
		if((timeleft * Time_res / 1000L) != formertime && tell) {
			formertime = timeleft * Time_res / 1000L;
			sprintf(Message,"Idling (%ld sec)",(long)formertime + 1L);
			PleaseWait();
			ShowMessage(FALSE,wMessage,Message);
			}
#if BP_CARBON_GUI_FORGET_THIS
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
		if((rep=MyButton(0)) != MISSED) {
			StopCount(0);
			
			compiledmem = CompiledGr;
			if(rep == OK)
				while((rep = MainEvent()) != RESUME && rep != STOP && rep != EXIT);
			if(rep == RESUME) {
				rep = OK; EventState = NO;
				}
			if(rep != OK) return(rep);
			if(compiledmem && !CompiledGr) return(ABORT);
			if(LoadedIn && (!CompiledIn && (rep=CompileInteraction()) != OK))
				 return(rep);
			}
		rep = OK;
		if(EventState != NO) return(EventState);
#endif /* BP_CARBON_GUI_FORGET_THIS */
		if((rep=ListenMIDI(0,0,0)) == ABORT || rep == ENDREPEAT
			|| rep == EXIT) return(rep);
//		drivertime = GetDriverTime();
		}
	HideWindow(Window[wMessage]);
	Tbytes2 = ZERO; Nbytes = MaxMIDIbytes/2;
	}
return(OK);
}

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
	ControlHandle itemhandle;
	Rect r;
	
	// Register program change to the MIDI orchestra
	channel = ByteToInt(p_e->status) % 16;
	thisevent = ByteToInt(p_e->status) - channel;
	if(thisevent == ProgramChange) {
		program = ByteToInt(p_e->data2) + 1;
		if(CurrentMIDIprogram[channel+1] != program) {
#if BP_CARBON_GUI_FORGET_THIS
			if(TestMIDIChannel == (channel+1) && CurrentMIDIprogram[TestMIDIChannel] > 0) {
				GetDialogItem(MIDIprogramPtr, (short)CurrentMIDIprogram[TestMIDIChannel],
							&itemtype, (Handle*)&itemhandle, &r);
				if(itemhandle != NULL) HiliteControl((ControlHandle) itemhandle,0);
				GetDialogItem(MIDIprogramPtr, (short)program, &itemtype, (Handle*)&itemhandle, &r);
				if(itemhandle != NULL) HiliteControl((ControlHandle) itemhandle,kControlButtonPart);
				WritePatchName();
				}
#endif /* BP_CARBON_GUI_FORGET_THIS */
			CurrentMIDIprogram[channel+1] = program;
#if BP_CARBON_GUI_FORGET_THIS
			for(j=0; j < 128; j++) {
				if((*p_GeneralMIDIpatchNdx)[j] == program) {
					sprintf(Message,"[%ld] %s",(long)program,*((*p_GeneralMIDIpatch)[j]));
					SetField(NULL,wMIDIorchestra,(channel+1),Message);
					break;
				}
			}
#endif /* BP_CARBON_GUI_FORGET_THIS */
		}
	}
}
