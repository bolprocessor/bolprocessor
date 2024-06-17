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


// DECLARATIONS

#if defined(_WIN64)
    #include <windows.h>
    #include <mmsystem.h>
    #pragma comment(lib, "winmm.lib")
    void CALLBACK MyMIDIInProc(HMIDIIN, UINT wMsg, DWORD_PTR, DWORD, DWORD);
    // Global variables for MIDI device handle
    static HMIDIOUT hMidiOut[MAXPORTS];
    static HMIDIOUT hMidiIn[MAXPORTS];
    for(i = 0; i < MAXPORTS; i++) {
        hMidiOut[i] = hMidiIn[i] = NULL;
        }
#endif
#if defined(__APPLE__)
    MIDIClientRef MIDIoutputClient, MIDIinputClient;
    MIDIPortRef MIDIoutPort, MIDIinPort[MAXPORTS];
    MIDIEndpointRef MIDIoutputdestination[MAXPORTS],MIDIinputdestination[MAXPORTS];
    static int *sourceIndices[MAXPORTS];
    static void MyMIDIReadProc(const MIDIPacketList*,void*,void*);
    Boolean IsMIDIDestinationActive(MIDIEndpointRef endpoint) {
        SInt32 offline;
        OSStatus result;
        // Check if the endpoint is offline
        result = MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyOffline, &offline);
        if(result != noErr) {
            BPPrintMessage(odError,"=> Error getting MIDIObjectGetIntegerProperty.\n");
            return(false);
            }
        // Return true if the device is online (offline == 0)
        return(offline == 0);
        }
#endif
#if defined(__linux__)
    #include <unistd.h> 
    #include <alsa/asoundlib.h>
    void MyAlsaMidiInProc(const snd_seq_event_t*);
    static snd_seq_t *seq_handle = NULL;
    static int out_port[MAXPORTS],in_port[MAXPORTS];
	typedef size_t Size;s
	typedef struct s_handle_priv {
		void* memblock; // Pointer to the allocated memory block
		size_t size;    // Size of the memory block	
		} *Handle;
#endif

// OPEN MIDI SYSTEM

int initializeMIDISystem(void) {
    int found,changed,fixed,index;
    char filename[MAXNAME];
    int i, firstchoice, error, done_input, done_inputport[MAXPORTS], done_output, done_outputport[MAXPORTS], busyinput[MAXPORTS], busyoutput[MAXPORTS];
    char more_outputs, more_inputs;
    char name[MAXNAME],firstname[MAXNAME];
    FILE * ptr;

    read_midisetup(); // This will modify MIDIinput[], MIDIoutput[], MIDIoutputname[] and MIDIinputname
    changed = fixed = false;
    done_input = done_output = 0;
    for(index = 0; index < MAXPORTS; index++) {
        busyoutput[index] = busyinput[index] = done_inputport[index] = done_outputport[index] = 0;
        }
    #if defined(_WIN64)
        MIDIOUTCAPS moc;
        MIDIINCAPS mic;
        BPPrintMessage(odInfo,"Setting up Windows MIDI system\n");
        // Get the number of MIDI out devices in the system
        UINT numMidiOutDevs = midiOutGetNumDevs();
        if(numMidiOutDevs == 0) {
            BPPrintMessage(odError,"No MIDI output devices available.\n");
            }
        UINT numMidiInDevs = midiInGetNumDevs();
        if(numMidiInDevs == 0) {
            BPPrintMessage("No MIDI input devices available.\n");
            }
        // Assign output ports to those with names
        for(index = 0; index < MaxOutputPorts; index++) {
            for(i = 0; i < numMidiOutDevs; i++) {
                if(midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS)) != MMSYSERR_NOERROR) {
                    BPPrintMessage(odError,"Error retrieving MIDI device capabilities #%d\n",i);
                    busyoutput[i] = 1;
                    continue; // Skip to next device
                    }
                if(strcmp(moc.szPname,MIDIoutputname[index]) == 0) {
                    busyoutput[i] = 1;
                    if(midiOutOpen(&hMidiOut[i], i, 0,0,CALLBACK_NULL) != MMSYSERR_NOERROR) {
                        BPPrintMessage("Error opening MIDI output device#%d.\n",i);
                        continue;
                        }
                    BPPrintMessage(odInfo,"MIDI output = %d: “%s”",i,moc.szPname);
                    BPPrintMessage(odInfo," 👉 the name of your choice\n");
                    done_output++; done_outputport[index] = 1;
                    if(MIDIoutput[index] != i) fixed = 1;
                    MIDIoutput[index] = (int) i;
                    break;
                    }
                }
            }
        // Assign input ports to those with names
        for(index = 0; index < MaxInputPorts; index++) {
            for(i = 0; i < numMidiInDevs; i++) {
                if(midiInGetDevCaps(i, &mic, sizeof(mic)) != MMSYSERR_NOERROR) {
                    BPPrintMessage(odError,"Error retrieving MIDI input device capabilities #%d\n",i);
                    busyinput[i] = 1;
                    continue; // Skip to next device
                    }
                if(busyinput[i]) continue;
                if(strcmp(mic.szPname,MIDIinputname[index]) == 0) {
                    busyinput[i] = 1;
                    if(midiInOpen(&hMidiIn[i], i, (DWORD_PTR)MyMIDIInProc, (DWORD_PTR) i, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
                        BPPrintMessage("Error opening MIDI input device #%d.\n",i);
                        continue; // Skip to next device
                        }
                    if(midiInStart(hMidiIn[i]) != MMSYSERR_NOERROR) {
                        BPPrintMessage(odError,"Error starting MIDI input #%d.\n",i);
                        continue; // Skip to next device
                        }
                    BPPrintMessage(odInfo,"MIDI input = %d: “%s”",i,mic.szPname);
                    BPPrintMessage(odInfo," 👉 the name of your choice\n");
                    done_input++; done_inputport[index] = 1;
                    if(MIDIinput[index] != i) fixed = 1;
                    MIDIinput[index] = (int) i;
                    sourceIndices[index] = (int*)malloc(sizeof(int)); 
                    *sourceIndices[index] = index;
                    Interactive = TRUE;
                    BPPrintMessage(odInfo,"MIDI input %d makes BP3 interactive\n",MIDIinput[index]);
                    break;
                    }
                }
            }
        // Assign output ports to those without names but possibly with numbers
        if(done_output <  MaxOutputPorts) {
            BPPrintMessage(odInfo,"Trying to assign ports to %d output(s) without names but possibly with numbers\n",(MaxOutputPorts - done_output));
            for(index = 0; index < MaxOutputPorts; index++) {
                if(done_outputport[index]) continue;
                found = 0; firstchoice = -1;
                for(i = 0; i < numMidiOutDevs; i++) {
                    if(busyoutput[i]) continue;
                    busyoutput[i] = 1;
                    if(midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS)) != MMSYSERR_NOERROR) {
                        BPPrintMessage(odError,"Error retrieving MIDI device capabilities #%d\n",i);
                        continue; // Skip to next device
                        }
                    if(midiOutOpen(&hMidiOut[i], i, 0,0,CALLBACK_NULL) != MMSYSERR_NOERROR) {
                        BPPrintMessage("Error opening MIDI output device #%d.\n",i);
                        continue; // Skip to next device
                        }   
                    if(firstchoice < 0) firstchoice = i;
                    if(!found && MIDIoutput[index] == i) {
                        BPPrintMessage(odInfo,"MIDI output = %d: “%s”",i,name);
                        BPPrintMessage(odInfo," 👉 the number of your choice\n");
                        strcpy(MIDIoutputname[index], moc.szPname);
                        done_output++;
                        busyoutput[i] = 1;
                        found = 1;
                        changed = 1;
                        break;
                        }
                    }
                if(!found) {
                    if(firstchoice >= 0) {
                        midiOutGetDevCaps(firstchoice, &moc, sizeof(MIDIOUTCAPS));
                        midiOutOpen(&hMidiOut[firstchoice], firstchoice, 0,0,CALLBACK_NULL);
                        BPPrintMessage(odInfo,"MIDI output = %d: “%s”",firstchoice,moc.szPname);
                        BPPrintMessage(odInfo," 👉 choice by default\n");
                        strcpy(MIDIoutputname[index],name);
                        done_output++;
                        busyoutput[firstchoice] = 1;
                        MIDIoutput[index] = firstchoice;
                        changed = 1;
                        }
                    else BPPrintMessage(odInfo,"No port is available for the new output\n");
                    }
                }
            }
        else BPPrintMessage(odInfo,"No port is available for the new output\n");
        // Assign input ports to those without names but possibly with numbers
        if(done_input <  MaxInputPorts) {
            BPPrintMessage(odInfo,"Trying to assign ports to %d input(s) without names but possibly with numbers\n",(MaxInputPorts - done_input));
            for(index = 0; index < MaxInputPorts; index++) {
                if(done_inputport[index]) continue;
                found = 0; firstchoice = -1;
                for(i = 0; i < numMidiInDevs; i++) {
                    if(busyinput[i]) continue;
                    busyinput[i] = 1;
                    if(midiInGetDevCaps(i, &mic, sizeof(mic)) != MMSYSERR_NOERROR) {
                        BPPrintMessage(odError,"Error retrieving MIDI input device capabilities #%d\n",i);
                        continue; // Skip to next device
                        }
                    if(midiInOpen(&hMidiIn[i], i, (DWORD_PTR)MyMIDIInProc, (DWORD_PTR) i, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
                        BPPrintMessage("Error opening MIDI input device #%d.\n",i);
                        continue; // Skip to next device
                        }
                    if(midiInStart(hMidiIn[i]) != MMSYSERR_NOERROR) {
                        BPPrintMessage(odError,"Error starting MIDI input #%d.\n",i);
                        continue; // Skip to next device
                        }
                    if(firstchoice < 0) firstchoice = i;
                    if(!found && MIDIinput[index] == i) {
                        BPPrintMessage(odInfo,"MIDI input = %d: “%s”",i,mic.szPname);
                        BPPrintMessage(odInfo," 👉 the number of your choice\n");
                        strcpy(MIDIinputname[index],mic.szPname);
                        done_input++;
                        sourceIndices[index] = (int*)malloc(sizeof(int)); 
                        *sourceIndices[index] = index;
                        Interactive = TRUE;
                        BPPrintMessage(odInfo,"MIDI input %d makes BP3 interactive\n",MIDIinput[index]);
                        found = 1;
                        changed = 1;
                        break;
                        }
                    }
                if(!found) {
                    if(firstchoice >= 0) {
                        midiInGetDevCaps(firstchoice, &mic, sizeof(mic));
                        midiInOpen(&hMidiIn[firstchoice], firstchoice, (DWORD_PTR)MyMIDIInProc, (DWORD_PTR) firstchoice, CALLBACK_FUNCTION);
                        if(midiInStart(hMidiIn[firstchoice]) != MMSYSERR_NOERROR) {
                            BPPrintMessage(odError,"Error starting MIDI input #%d.\n",i);
                            continue; // Skip to next device
                            }
                        BPPrintMessage(odInfo,"MIDI input = %d: “%s”",firstchoice,mic.szPname);
                        BPPrintMessage(odInfo," 👉 choice by default\n");
                        strcpy(MIDIinputname[index],mic.szPname);
                        done_input++;
                        busyinput[firstchoice] = 1;
                        MIDIinput[index] = firstchoice;
                        changed = 1;
                        sourceIndices[index] = (int*)malloc(sizeof(int)); 
                        *sourceIndices[index] = index;
                        Interactive = TRUE;
                        BPPrintMessage(odInfo,"MIDI input %d makes BP3 interactive\n",MIDIinput[index]);
                        }
                    else BPPrintMessage(odInfo,"No port is available for the new input\n");
                    }
                }
            }
        else BPPrintMessage(odInfo,"No port is available for the new output\n");
        // Suggest more options
        more_outputs = 0;
        for(i = 0; i < numMidiOutDevs; i++) if(!busyoutput[i]) more_outputs = 1;
        for(i = 0; i < numMidiInDevs; i++) if(!busyinput[i]) more_inputs = 1;
        if(more_outputs) {
            BPPrintMessage(odInfo,"\n🎶 More MIDI output options were available:\n");
            for(i = 0; i < numMidiOutDevs; i++) {
                if(busyoutput[i]) continue;
                midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS));
                if(midiOutOpen(&hMidiOut[i], i, 0,0,CALLBACK_NULL) != MMSYSERR_NOERROR) {
                    continue; // Skip to next device
                    }
                BPPrintMessage(odInfo,"MIDI output = %d: “%s”\n",i,moc.szPname);   
                }
            }
        if(more_inputs) {
            BPPrintMessage(odInfo,"\n🎶 More MIDI input options were available:\n");
            for(i = 0; i < numMidiInDevs; i++) {
                if(busyinput[i]) continue;
                midiInGetDevCaps(i, &mic, sizeof(mic));
                if(midiInOpen(&hMidiIn[i], i, (DWORD_PTR)MyMIDIInProc, (DWORD_PTR) i, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
                    continue; // Skip to next device
                    }
                BPPrintMessage(odInfo,"MIDI intput = %d: “%s”\n",i,mic.szPname);
                }
            }
    #endif
    #if defined(__APPLE__)
        OSStatus status;
        MIDIEndpointRef src, outputdestination, inputdestination;
        CFStringRef endpointName;
        MIDIoutPort = -1;
        for(index = 0; index < MAXPORTS; index++) {
            sourceIndices[index] = NULL;
            }
        BPPrintMessage(odInfo,"🎹 Setting up MacOS MIDI system\n");
        status = MIDIClientCreate(CFSTR("MIDIcheck Client"),NULL,NULL,&MIDIoutputClient);
        if(status != noErr) {
            BPPrintMessage(odError,"=> Error: Could not create MIDI client.\n");
            return(FALSE);
            }
        status = MIDIOutputPortCreate(MIDIoutputClient,CFSTR("Output Port"),&MIDIoutPort);
        if(status != noErr) {
            BPPrintMessage(odError,"=> Error: Could not create output port.\n");
            return(FALSE);
            }
        ItemCount outputCount = MIDIGetNumberOfDestinations();
        if(outputCount == 0) {
            BPPrintMessage(odError,"=> Error: No MIDI output available.\n");
            return(FALSE);
            }
        status = MIDIClientCreate(CFSTR("MIDI Client"),NULL,NULL,&MIDIinputClient);
        if(status != noErr) {
            BPPrintMessage(odError,"=> Error: Could not create MIDI input client.\n");
            return(FALSE);
            }
        ItemCount inputCount = MIDIGetNumberOfSources();
        if(inputCount == 0) {
            BPPrintMessage(odError,"=> Error: No MIDI input available.\n");
            return(FALSE);
            }
        // Assign output ports to those with names
        for(index = 0; index < MaxOutputPorts; index++) {
            for(i = 0; i < outputCount; i++) {
                outputdestination = MIDIGetDestination(i);
                if(!IsMIDIDestinationActive(outputdestination)) {
                    BPPrintMessage(odError,"This MIDI (output) %d is inactive.\n",i);
                    continue;
                    }
                endpointName = NULL;
                if(MIDIObjectGetStringProperty(outputdestination, kMIDIPropertyName, &endpointName) == noErr) {
                    CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                    if(strcmp(name,MIDIoutputname[index]) == 0) {
                        BPPrintMessage(odInfo,"MIDI output = %d: “%s”",i,name);
                        BPPrintMessage(odInfo," 👉 the name of your choice\n");
                        done_output++; done_outputport[index] = 1;
                        if(MIDIoutput[index] != i) fixed = 1;
                        MIDIoutput[index] = (int) i;
                        busyoutput[i] = 1;
                        MIDIoutputdestination[index] = MIDIGetDestination(MIDIoutput[index]);
                        break;
                        }
                    }
                }
            }
        // Assign input ports to those with names
        for(index = 0; index < MaxInputPorts; index++) {
            for(i = 0; i < inputCount; i++) {
                status = MIDIInputPortCreate(MIDIinputClient, CFSTR("Input Port"),MyMIDIReadProc,NULL,&MIDIinPort[index]);
                if(status != noErr) {
                    BPPrintMessage(odError,"=> Error: Could not create input port with MIDI Protocol 1.0.\n");
                    return(FALSE);
                    }
                inputdestination = MIDIGetSource(i);
                endpointName = NULL;
                if(MIDIObjectGetStringProperty(inputdestination, kMIDIPropertyName, &endpointName) == noErr) {
                    CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                    if(!IsMIDIDestinationActive(inputdestination)) {
                        BPPrintMessage(odError,"This MIDI input %d is inactive.\n",i);
                        continue;
                        }
                    if(strcmp(name,MIDIinputname[index]) == 0) {
                        BPPrintMessage(odInfo,"MIDI input = %d: “%s”",i,name);
                        BPPrintMessage(odInfo," 👉 the name of your choice\n");
                        done_input++; done_inputport[index] = 1;
                        if(MIDIinput[index] != i) fixed = 1;
                        MIDIinput[index] = (int) i;
                        busyinput[i] = 1;
                        MIDIinputdestination[index] = MIDIGetDestination(MIDIinput[index]);
                        src = MIDIGetSource(MIDIinput[index]);
                        sourceIndices[index] = (int*)malloc(sizeof(int)); 
                        *sourceIndices[index] = index;
                        MIDIPortConnectSource(MIDIinPort[index],src,sourceIndices[index]);
                        Interactive = TRUE;
                        BPPrintMessage(odInfo,"MIDI input %d makes BP3 interactive\n",MIDIinput[index]);
                        break;
                        }
                    }
                }
            }
        // Assign output ports to those without names but possibly with numbers
        if(done_output <  MaxOutputPorts) {
            BPPrintMessage(odInfo,"Trying to assign ports to %d output(s) without names but possibly with numbers\n",(MaxOutputPorts - done_output));
            for(index = 0; index < MaxOutputPorts; index++) {
                if(done_outputport[index]) continue;
                found = 0; firstchoice = -1;
                for(i = 0; i < outputCount; i++) {
                    if(busyoutput[i]) continue;
                    outputdestination = MIDIGetDestination(i);
                    if(!IsMIDIDestinationActive(outputdestination)) {
                        BPPrintMessage(odError,"This MIDI (output) %d is inactive.\n",i);
                        continue;
                        }
                    endpointName = NULL;
                    if(MIDIObjectGetStringProperty(outputdestination, kMIDIPropertyName, &endpointName) == noErr) {
                        CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                        if(firstchoice < 0) firstchoice = i;
                        if(!found && MIDIoutput[index] == i) {
                            BPPrintMessage(odInfo,"MIDI output = %d: “%s”",i,name);
                            BPPrintMessage(odInfo," 👉 the number of your choice\n");
                            strcpy(MIDIoutputname[index],name);
                            done_output++;
                            busyoutput[i] = 1;
                            MIDIoutputdestination[index] = MIDIGetDestination(MIDIoutput[index]);
                            found = 1;
                            changed = 1;
                            break;
                            }
                        }
                    }
                if(!found) {
                    if(firstchoice >= 0) {
                        outputdestination = MIDIGetDestination(firstchoice);
                        endpointName = NULL;
                        if(MIDIObjectGetStringProperty(outputdestination, kMIDIPropertyName, &endpointName) == noErr) {
                            CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                            BPPrintMessage(odInfo,"MIDI output = %d: “%s”",firstchoice,name);
                            BPPrintMessage(odInfo," 👉 choice by default\n");
                            strcpy(MIDIoutputname[index],name);
                            done_output++;
                            busyoutput[firstchoice] = 1;
                            MIDIoutput[index] = firstchoice;
                            changed = 1;
                            MIDIoutputdestination[index] = MIDIGetDestination(MIDIoutput[index]);
                            }
                        }
                    else BPPrintMessage(odInfo,"No port is available for the new output\n");
                    }
                }
            }
        // Assign input ports to those without names but possibly with numbers
        if(done_input <  MaxInputPorts) {
            BPPrintMessage(odInfo,"Trying to assign ports to %d input(s) without names but possibly with numbers\n",(MaxInputPorts - done_input));
            for(index = 0; index < MaxInputPorts; index++) {
                if(done_inputport[index]) continue;
                found = 0; firstchoice = -1;
                for(i = 0; i < inputCount; i++) {
                    if(busyinput[i]) continue;
                    status = MIDIInputPortCreate(MIDIinputClient, CFSTR("Input Port"),MyMIDIReadProc,NULL,&MIDIinPort[index]);
                    if(status != noErr) {
                        BPPrintMessage(odError,"=> Error: Could not create input port with MIDI Protocol 1.0.\n");
                        return(FALSE);
                        }
                    inputdestination = MIDIGetSource(i);
                    endpointName = NULL;
                    if(MIDIObjectGetStringProperty(inputdestination, kMIDIPropertyName, &endpointName) == noErr) {
                        CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                        if(firstchoice < 0) firstchoice = i;
                        if(!found && MIDIinput[index] == i) {
                            BPPrintMessage(odInfo,"MIDI input = %d: “%s”",i,name);
                            BPPrintMessage(odInfo," 👉 the number of your choice\n");
                            strcpy(MIDIinputname[index],name);
                            done_input++;
                            busyinput[i] = 1;
                            MIDIinputdestination[index] = MIDIGetDestination(MIDIinput[index]);
                            src = MIDIGetSource(MIDIinput[index]);
                            sourceIndices[index] = (int*)malloc(sizeof(int)); 
                            *sourceIndices[index] = index;
                            MIDIPortConnectSource(MIDIinPort[index],src,sourceIndices[index]);
                            Interactive = TRUE;
                            BPPrintMessage(odInfo,"MIDI input %d makes BP3 interactive\n",MIDIinput[index]);
                            found = 1;
                            changed = 1;
                            break;
                            }
                        }
                    }
                if(!found) {
                    if(firstchoice >= 0) {
                        inputdestination = MIDIGetSource(firstchoice);
                        endpointName = NULL;
                        if(MIDIObjectGetStringProperty(inputdestination, kMIDIPropertyName, &endpointName) == noErr) {
                            CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                            BPPrintMessage(odInfo,"MIDI input = %d: “%s”",firstchoice,name);
                            BPPrintMessage(odInfo," 👉 choice by default\n");
                            strcpy(MIDIinputname[index],name);
                            done_input++;
                            busyinput[firstchoice] = 1;
                            MIDIinput[index] = firstchoice;
                            changed = 1;
                            MIDIinputdestination[index] = MIDIGetDestination(MIDIinput[index]);
                            src = MIDIGetSource(MIDIinput[index]);
                            sourceIndices[index] = (int*)malloc(sizeof(int)); 
                            *sourceIndices[index] = index;
                            MIDIPortConnectSource(MIDIinPort[index],src,sourceIndices[index]);
                            Interactive = TRUE;
                            BPPrintMessage(odInfo,"MIDI input %d makes BP3 interactive\n",MIDIinput[index]);
                            }
                        }
                    else BPPrintMessage(odInfo,"No port is available for the new input\n");
                    }
                }
            }
        // Suggest more options
        more_outputs = 0;
        for(i = 0; i < outputCount; i++) if(!busyoutput[i]) more_outputs = 1;
        for(i = 0; i < inputCount; i++) if(!busyinput[i]) more_inputs = 1;
        if(more_outputs) {
            BPPrintMessage(odInfo,"\n🎶 More MIDI output options were available:\n");
            for(i = 0; i < outputCount; i++) {
                if(busyoutput[i]) continue;
                outputdestination = MIDIGetDestination(i);
                endpointName = NULL;
                if(MIDIObjectGetStringProperty(outputdestination, kMIDIPropertyName, &endpointName) == noErr) {
                    CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                    BPPrintMessage(odInfo,"MIDI output = %d: “%s”\n",i,name);
                    }
                }
            }
        if(more_inputs) {
            BPPrintMessage(odInfo,"\n🎶 More MIDI input options were available:\n");
            for(i = 0; i < inputCount; i++) {
                if(busyinput[i]) continue;
                inputdestination = MIDIGetSource(i);
                endpointName = NULL;
                if(MIDIObjectGetStringProperty(inputdestination, kMIDIPropertyName, &endpointName) == noErr) {
                    CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                    BPPrintMessage(odInfo,"MIDI input = %d: “%s”\n",i,name);
                    }
                }
            }
    #endif
    #if defined(__linux__)
        snd_seq_client_info_t *cinfo;
        snd_seq_port_info_t *pinfo;
        const char *port_name;
        int client;
        BPPrintMessage(odInfo,"🎹 Setting up Linux MIDI system\n");
        // Open the ALSA sequencer
        if((error = snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0)) < 0) {
            BPPrintMessage(odError,"=> Error opening ALSA sequencer\n", snd_strerror(error));
            return ABORT;
            }
        snd_seq_set_client_name(seq_handle, "MIDIcheck Client");
        // Create the MIDI ports
        for(i = 0; i < MAXPORTS; i++) {
            char port_name[64];
            sprintf(port_name, "Output Port %d", i);
            out_port[i] = snd_seq_create_simple_port(seq_handle, port_name,
                SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
                SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
     /*       if(out_port[i] < 0)
                BPPrintMessage(odError,"Warning: Failed to create output port %d: %s\n", i,snd_strerror(out_port[i])); */
            }
        for(i = 0; i < MAXPORTS; i++) {
            char port_name[64];
            sprintf(port_name, "Input Port %d", i);
            in_port[i] = snd_seq_create_simple_port(seq_handle, port_name,
                SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
     /*       if(in_port[i] < 0)
                BPPrintMessage(odError,"Warning: Failed to create input port %d: %s\n", i,snd_strerror(in_port[i])); */
            }
        // Allocate temporary data for port info
        snd_seq_client_info_alloca(&cinfo);
        snd_seq_port_info_alloca(&pinfo);
        snd_seq_client_info_set_client(cinfo, -1);

        // Assign output ports to those with names
        BPPrintMessage(odInfo, "Assigning named MIDI output ports\n");
        for(index = 0; index < MaxOutputPorts; index++) {
            for(i = 0; i < MAXPORTS; i++) {
                if(out_port[i] < 0) continue;
                snd_seq_client_info_set_client(cinfo, -1);  // Reset client info
                while(snd_seq_query_next_client(seq_handle, cinfo) == 0) {
                    client = snd_seq_client_info_get_client(cinfo);
                    snd_seq_port_info_set_client(pinfo, client);
                    snd_seq_port_info_set_port(pinfo, -1);
                    while(snd_seq_query_next_port(seq_handle, pinfo) == 0) {
                        if(snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_WRITE &&
                            !(snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_NO_EXPORT)) {
                            port_name = snd_seq_port_info_get_name(pinfo);
                            if(strcmp(port_name, MIDIoutputname[index]) == 0) {
                                snd_seq_connect_from(seq_handle,out_port[i], client,snd_seq_port_info_get_port(pinfo));
                                BPPrintMessage(odInfo,"MIDI output = %d: “%s”",i,port_name);
                                BPPrintMessage(odInfo," 👉 the name of your choice\n");
                                done_output++; done_outputport[i] = 1;
                                busyoutput[i] = 1;
                                if(MIDIoutput[index] != i) fixed = 1;
                                MIDIoutput[index] = i;
                                break;
                                }
                            }
                        }
                    if(done_outputport[i]) break;   
                    }
                }
            }
        // Assign input ports to those with names
        BPPrintMessage(odInfo, "Assigning named MIDI input ports\n");
        for(index = 0; index < MaxInputPorts; index++) {
            for(i = 0; i < MAXPORTS; i++) {
                if(in_port[i] < 0) continue;
                snd_seq_client_info_set_client(cinfo, -1);  // Reset client info
                while(snd_seq_query_next_client(seq_handle, cinfo) == 0) {
                    client = snd_seq_client_info_get_client(cinfo);
                    snd_seq_port_info_set_client(pinfo, client);
                    snd_seq_port_info_set_port(pinfo, -1);
                    while(snd_seq_query_next_port(seq_handle, pinfo) == 0) {
                        // Check if the port is an output or input and is not our own ports
                        if(snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_READ &&
                            !(snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_NO_EXPORT)) {
                            // Connect to this port if it matches the desired name
                            port_name = snd_seq_port_info_get_name(pinfo);
                            if(strcmp(port_name, MIDIinputname[index]) == 0) {
                                snd_seq_connect_to(seq_handle,in_port[i],client,snd_seq_port_info_get_port(pinfo));
                                BPPrintMessage(odInfo,"MIDI input = %d: “%s”",i,port_name);
                                BPPrintMessage(odInfo," 👉 the name of your choice\n");
                                if(MIDIinput[index] != i) fixed = 1;
                                MIDIinput[index] = i;
                                done_input++; done_inputport[i] = 1;
                                busyinput[i] = 1;
                                Interactive = TRUE;
                                BPPrintMessage(odInfo,"MIDI input %d makes BP3 interactive\n", MIDIinput[index]);
                                break;
                                }
                            }
                        }
                    if(done_inputport[i]) break;
                    }
                }
            }

        // CONTINUE WRITING PROCEDURES!


    #endif
    if(fixed && !changed) BPPrintMessage(odInfo,"MIDI port number(s) may have changed and will be updated when saving the page of your project\n");
    save_midisetup();
    if(changed || fixed) {
        strcpy(filename,Midiportfilename);
        strcat(filename,"_refresh");
        ptr = fopen(filename,"w");
		fputs("refresh this midiport!\n",ptr);
		fclose(ptr);
        }
    if(changed) BPPrintMessage(odInfo,"=> Warning: name of MIDI source or/and output changed (see above)\n");
  //  Panic = TRUE; return(ABORT);
    return(OK);
    }

// CLOSE MIDI SYSTEM

void closeMIDISystem() {
    BPPrintMessage(odInfo,"Closing MIDI system\n");
    int index;
    #if defined(_WIN64)
        // Windows MIDI cleanup
        BPPrintMessage(odInfo,"Closing Windows MIDI system\n");
        for(index = 0; index < MAXPORTS; index++) {
            if(hMidiOut[index] != NULL) {
                midiOutClose(hMidiOut[index]);  // Close the MIDI output device
                hMidiOut[index] = NULL;         // Reset the handle to NULL after closing
                }
            if(hMidiIn[index] != NULL) {
                midiInStop(hMidiIn[index]);
                midiInClose(hMidiIn[index]);
                hMidiIn[index] = NULL;
                }
            }
    #elif defined(__APPLE__)
        // MacOS MIDI cleanup
        MIDIClientDispose(MIDIoutputClient);
        MIDIClientDispose(MIDIinputClient);
        MIDIPortDispose(MIDIoutPort);
        for(index = 0; index < MAXPORTS; index++) {
            if(sourceIndices[index] != NULL) {
                free(sourceIndices[index]);
                sourceIndices[index] = NULL;
                }
            if(MIDIinPort[index] >= 0) MIDIPortDispose(MIDIinPort[index]);
            }
    #elif defined(__linux__)
        // Linux MIDI cleanup
        if(seq_handle != NULL) {
            snd_seq_close(seq_handle);  // Close the ALSA sequencer
            seq_handle = NULL;          // Reset the handle to NULL after closing
            }
    #endif
    }


// RECEIVE MIDI EVENTS

#if defined(_WIN64)
    // Placeholder for Windows MIDI input callback
    void CALLBACK MyMIDIInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam2) {
        MIDIHDR* midiHdr;
        MIDI_Event e;
        int index = (int)dwInstance; // Assuming you pass the source index as the instance data
        switch (wMsg) {
            case MIM_OPEN:
                // MIDI device has been opened
                break;
            case MIM_CLOSE:
                // MIDI device has been closed
                break;
            case MIM_DATA:
                // MIDI Short Message received, dwParam1 is the MIDI data, dwParam2 is the timestamp
                e.status = (unsigned char)(dwParam1 & 0xFF);
                e.data1 = (unsigned char)((dwParam1 >> 8) & 0xFF);
                e.data2 = (unsigned char)((dwParam1 >> 16) & 0xFF);
                e.timestamp = dwParam2;  // Using dwParam2 as the timestamp
                if(AcceptEvent(dwParam1 & 0xFF, index)) {
                    HandleInputEvent((unsigned char*)&dwParam1, &e, index);
                    if(PassInEvent(dwParam1 & 0xFF, index)) {
                        sendMIDIEvent((unsigned char*)&dwParam1, 3, 0); // Assuming 3 bytes for a short message
                        }
                    }
                break;
            case MIM_LONGDATA:
                // MIDI Sysex or Long Message received, dwParam1 points to a MIDIHDR structure
                midiHdr = (MIDIHDR*)dwParam1;
                if(AcceptEvent(midiHdr->lpData[0], index)) {
                    // Assuming the first byte is the status byte for simplicity
                    e.status = midiHdr->lpData[0];
                    e.data1 = midiHdr->lpData[1];
                    e.data2 = midiHdr->lpData[2];
                    e.timestamp = dwParam2;  // Assume timestamp is passed as dwParam2
                    HandleInputEvent((unsigned char*)midiHdr->lpData, &e, index);
                    if(PassInEvent(midiHdr->lpData[0], index)) {
                        sendMIDIEvent((unsigned char*)midiHdr->lpData, midiHdr->dwBytesRecorded, 0);
                        }
                    }
                // Once processed, prepare it for more input
                midiInPrepareHeader(hMidiIn, midiHdr, sizeof(MIDIHDR));
                midiInAddBuffer(hMidiIn, midiHdr, sizeof(MIDIHDR));
                break;
            }
        }
#endif
#if defined(__APPLE__)
    // Placeholder for MacOS MIDI input
    static void MyMIDIReadProc(const MIDIPacketList* packetList, void* readProcRefCon, void* srcConnRefCon) {
        (void)readProcRefCon;  // Unused parameter. This is necessary in order to avoid warnings.
        const MIDIPacket* packet;
        MIDI_Event e;
        int index;
        packet = &packetList->packet[0];
        if(!srcConnRefCon) {
            BPPrintMessage(odError,"=> Received NULL srcConnRefCon\n");
            return;
            }
        index = *(int*)srcConnRefCon;
    //   BPPrintMessage(odInfo,"sourceIndex = %d\n",index);
        if(packet == NULL) {
            BPPrintMessage(odError,"=> No packets received.\n");
            return;  // Early exit if packet is NULL
            }
        if(AcceptEvent(ByteToInt(packet->data[0]),index)) {
            for(unsigned int i = 0; i < packetList->numPackets; i++) {
                HandleInputEvent(packet,&e,index);
                if(PassInEvent(ByteToInt(packet->data[0]),index)) sendMIDIEvent((unsigned char*) packet->data,packet->length,0);
                // Sending immediately
                packet = MIDIPacketNext(packet);
                }
            }
        }
#endif
#if defined(__linux__)
    typedef struct {
        unsigned char* data;  // MIDI data bytes
        int length;           // Number of bytes in the data array
        unsigned long timestamp;  // Timestamp (optional, can be adapted to your needs)
        } MIDIPacket;

    // Placeholder for ALSA MIDI input callback
    void MyAlsaMidiInProc(const snd_seq_event_t *ev) {
        MIDI_Event e;
        MIDIPacket packet;
        int index, midiData[3];
        // Allocate data for the worst-case MIDI event size
        int source_port = ev->source.port;  // ALSA event source port
        for(index = 0; index < MaxInputPorts; index++) {
            if(MIDIinput[index] == source_port) break;
            }
        if(index >= MaxInputPorts) return;
        unsigned char data_buffer[256];  // Adjust size as needed, especially for SysEx events
        packet.data = data_buffer;
        packet.timestamp = ev->time.tick;  // Using tick time; adapt as necessary
        e.status = ev->data.note.channel;
        e.data1 = ev->data.note.note;
        e.data2 = ev->data.note.velocity;
        packet.length = 3;  // Common MIDI messages are 3 bytes
        packet.data[0] = e.status;
        packet.data[1] = e.data1;
        packet.data[2] = e.data2;
        if(!AcceptEvent(e.status,index)) return;
        HandleInputEvent(&packet, &e, index);
        switch (ev->type) {
            case SND_SEQ_EVENT_NOTEON:
                midiData[0] = 0x90 | (ev->data.note.channel & 0x0F);  // Note On command with channel
                midiData[1] = ev->data.note.note;  // Note value
                midiData[2] = ev->data.note.velocity;  // Velocity
                if(PassInEvent(e.status,index)) sendMIDIEvent(midiData,3,packet.timestamp);
                break;
            case SND_SEQ_EVENT_NOTEOFF:
                midiData[0] = 0x80 | (ev->data.note.channel & 0x0F);  // Note Off command with channel
                midiData[1] = ev->data.note.note;  // Note value
                midiData[2] = 0;  // Velocity is often 0 for Note Off
                if(PassInEvent(e.status,index)) sendMIDIEvent(midiData,3,packet.timestamp);
                break;
            case SND_SEQ_EVENT_CONTROLLER:
                midiData[0] = 0xB0 | (ev->data.control.channel & 0x0F);  // Controller command with channel
                midiData[1] = ev->data.control.param;  // Controller number
                midiData[2] = ev->data.control.value;  // Controller value
                if(PassInEvent(e.status,index)) sendMIDIEvent(midiData,3,packet.timestamp);
                break;
            // Add more cases here for different types of MIDI messages
            default:
                // Handle other types of messages or ignore them
                break;
            }
        }
#endif


// LISTEN TO MIDI

int ListenMIDI(int x0, int x1, int x2) {
	int r = OK;
	if(EmergencyExit || Panic) return(ABORT); // 2024-05-03
#if defined(__linux__)
	snd_seq_event_t* ev;
    snd_seq_event_input(seq_handle, &ev);
    MyAlsaMidiInProc(ev);
    snd_seq_free_event(ev);
#endif
	if((r=stop(0,"ListenMIDI")) != OK) return r;
	return(r);
	}


// SEND MIDI EVENTS

void sendMIDIEvent(unsigned char* midiData,int dataSize,long time) {
    int note,status,value,test_first_events,improvize,index,channel;
    unsigned long clocktime;
    MIDIPortRef targetPort;
    test_first_events = 0;
    if(dataSize == 3) status = midiData[0] & 0xF0;
    else status = midiData[0];
    note = midiData[1];
    value = midiData[2];
    channel = midiData[0] & 0x0F;
    if(test_first_events && NumEventsWritten < 100) {
        clocktime = getClockTime() - initTime; // microseconds
        improvize = Improvize;
        Improvize = 0; // Necessary to activate BPPrintMessage(odInfo,...
        if(status == NoteOn || status == NoteOff)
            BPPrintMessage(odInfo,"%.3f => %.3f s status = %d, note = %d, value = %d channel = %d\n",(float)clocktime/1000000,(float)time/1000000,status,note,value,(channel + 1));
        Improvize = improvize;
        }
    if(NumEventsWritten < LONG_MAX) NumEventsWritten++;
 //   BPPrintMessage(odInfo,"Sending MIDI event time = %ld ms, status = %ld, note %ld, value = %ld\n",(long)time/1000L,(long)status,(long)note,(long)value);
    #if defined(_WIN64)
    // Windows MIDI event sending
    DWORD message = 0;
/*   for(int i = 0; i < dataSize; i++) {
        message |= (midiData[i] << (i * 8));
        } */
    if(dataSize <= 3) {
        message = midiData[0] | (midiData[1] << 8) | (midiData[2] << 16);
        }
    // Send the MIDI message
    for(index = 0; index < MaxOutputPorts; index++) {
        // Filter out based on channel and other criteria
        if(MIDIchannelFilter[index][channel] == '0') continue;
        if(!PassOutEvent(status, index)) continue;
        // Send the MIDI message
        if(hMidiOut[index]) {  // Check if the output handle is valid
            if(dataSize <= 3) {
                midiOutShortMsg(hMidiOut[index], message);
                }
            else {
                // For long messages, you would need to use midiOutLongMsg
                MIDIHDR midiHdr;
                midiHdr.lpData = (LPSTR)midiData;
                midiHdr.dwBufferLength = dataSize;
                midiHdr.dwFlags = 0;
                midiOutPrepareHeader(hMidiOut[index], &midiHdr, sizeof(MIDIHDR));
                midiOutLongMsg(hMidiOut[index], &midiHdr, sizeof(MIDIHDR));
                midiOutUnprepareHeader(hMidiOut[index], &midiHdr, sizeof(MIDIHDR));
                }
            }
        }
    #endif
    #if defined(__APPLE__)
        // MacOS MIDI event sending
        MIDIPacketList packetList;
        MIDIPacket *packet = MIDIPacketListInit(&packetList);
        packet = MIDIPacketListAdd(&packetList, sizeof(packetList), packet, mach_absolute_time(), dataSize, midiData);
        targetPort = MIDIoutPort;
        channel = midiData[0] & 0x0F; // Extract MIDI channel from status byte
        if(packet) {
            for(index = 0; index < MaxOutputPorts; index++) {
                // Here use filters to decide which port(s) this event should be sent to.
                if(MIDIchannelFilter[index][channel] == '0') continue;
                if(!PassOutEvent(status,index)) continue;
                MIDISend(targetPort, MIDIoutputdestination[index], &packetList);
                }
            }
    #endif
    #if defined(__linux__)
        snd_seq_event_t ev;
        if(seq_handle == NULL) {
            BPPrintMessage(odError,"=> ALSA sequencer has not been initialised\n");
            return(ABORT);
            }
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);
        switch(status) {
            case 0x90: // Note On
                if(value == 0) // A Note On with velocity 0 is a Note Off in MIDI
                    snd_seq_ev_set_noteoff(&ev, channel, note, value);
                else snd_seq_ev_set_noteon(&ev, channel, note, value);
                break;
            case 0x80: // Note Off
                snd_seq_ev_set_noteoff(&ev, channel, note, value);
                break;
            case 0xB0: // Controller
                snd_seq_ev_set_controller(&ev, channel, note, value);
                break;
            // Add more cases as necessary for different types of MIDI messages
            }
        for(index = 0; index < MaxOutputPorts; index++) {
            if(MIDIchannelFilter[index][channel] == '0') continue;
            if(!PassOutEvent(status,index)) continue;
            // Set the source of the event as the output port
            snd_seq_ev_set_source(&ev,out_port[MIDIoutput[index]]);
       //     snd_seq_ev_set_dest(&ev,SND_SEQ_ADDRESS_SUBSCRIBERS,out_port[MIDIoutput[index]]);
         //   snd_seq_event_output_direct(seq_handle,&ev);
            snd_seq_ev_set_direct(&ev);  // Makes the event immediate
            }
    #endif
    }


// GET CLOCK TIME

unsigned long getClockTime(void) {  // Microseconds
    unsigned long the_time;
    #if defined(_WIN64)
        LARGE_INTEGER freq, count;
        QueryPerformanceFrequency(&freq); // Get the frequency of the high-resolution performance counter
        QueryPerformanceCounter(&count);  // Get the current value of the performance counter
        the_time = (unsigned long)((count.QuadPart * 1000000) / freq.QuadPart);
    #elif defined(__APPLE__)
        the_time = clock_gettime_nsec_np(CLOCK_UPTIME_RAW_APPROX) / 1000L;
    #elif defined(__linux__)
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts); // CLOCK_MONOTONIC provides uptime, not affected by system time changes
        the_time = (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
    #endif
    return(the_time);
    }