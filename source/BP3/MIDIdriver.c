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

int test_first_events = 0;
int trace_all_interactions = 0;

// DECLARATIONS

#if defined(_WIN64)
    void CALLBACK MyMIDIInProc(HMIDIIN, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);
    // Global variables for MIDI device handle
    static HMIDIOUT hMIDIout[MAXPORTS];
    static HMIDIIN hMIDIin[MAXPORTS];
    DWORD WINAPI MidiMessageThread(LPVOID);
#endif
#if defined(__APPLE__)
    MIDIClientRef MIDIoutputClient, MIDIinputClient;
    MIDIPortRef MIDIoutPort, MIDIinPort[MAXPORTS];
    MIDIEndpointRef MIDIoutputdestination[MAXPORTS],MIDIinputdestination[MAXPORTS];
    static int *sourceIndices[MAXPORTS];
    static void MyMIDIReadProc(const MIDIPacketList*,void*,void*);
    int IsMIDIDestinationActive(MIDIEndpointRef endpoint) {
        int32_t offline;
        OSStatus result;
        // Check if the endpoint is offline
        result = MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyOffline, &offline);
        if(result != noErr) {
            BPPrintMessage(odError,"=> Error getting MIDIObjectGetIntegerProperty\n");
            return(false);
            }
        // Return true if the device is online (offline == 0)
        return(offline == 0);
        }
#endif

// OPEN MIDI SYSTEM

int initializeMIDISystem(void) {
    int found,changed,fixed,index;
    char filename[MAXNAME];
    int i, firstchoice, result, error, done_input, done_inputport[MAXPORTS], done_output, done_outputport[MAXPORTS], busyinput[MAXPORTS], busyoutput[MAXPORTS],doneMIDIinput[MAXPORTS],MIDIinputclient[MAXPORTS],doneMIDIoutput[MAXCLIENTS];
    char more_outputs, more_inputs;
    char name[MAXNAME],firstname[MAXNAME];
    FILE * ptr;

    read_midisetup(); // This will modify MIDIinput[], MIDIoutput[], MIDIoutputname[] and MIDIinputname
    changed = fixed = 0;
    done_input = done_output = 0;
    for(i = 0; i < MAXPORTS; i++) {
        busyoutput[i] = done_outputport[i] = 0;
        doneMIDIoutput[i] = 0;
        busyinput[i] = done_inputport[i] = 0;
        doneMIDIinput[i] = 0;
        }
    #if defined(_WIN64)
        MIDIOUTCAPS moc;
        MIDIINCAPS mic;
        int theport, ok;
        MMRESULT resultopen;
        for (theport = 0; theport < MAXPORTS; theport++) {
            hMIDIout[theport] = NULL;
            hMIDIin[theport] = NULL;
            }
        BPPrintMessage(odInfo, "🎹 Setting up MIDI\n");
        // Get the number of MIDI out devices in the system
        UINT numMidiOutDevs = midiOutGetNumDevs();
        if(numMidiOutDevs == 0) {
            BPPrintMessage(odInfo,"No MIDI output device available\n");
            }
        else BPPrintMessage(odInfo,"%d MIDI output devices available, we need %d\n",numMidiOutDevs,MaxOutputPorts);
        UINT numMidiInDevs = midiInGetNumDevs();
        if(numMidiInDevs == 0) {
            BPPrintMessage(odInfo,"No MIDI input device available\n");
            }
        else BPPrintMessage(odInfo,"%d MIDI input devices available, we need %d\n",numMidiInDevs,MaxInputPorts);

        // Assign output ports to those with names
        for(index = 0; index < MaxOutputPorts; index++) {
            if (strlen(MIDIoutputname[index]) == 0) continue;
            for(i = 0; i < numMidiOutDevs; i++) {
                if(midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS)) != MMSYSERR_NOERROR) {
                    BPPrintMessage(odError,"Error retrieving MIDI device capabilities #%d\n",i);
                    busyoutput[i] = 1;
                    continue; // Skip to next device
                    }
                if(strcmp(moc.szPname,MIDIoutputname[index]) == 0) {
                    busyoutput[i] = 1;
                    resultopen = midiOutOpen(&hMIDIout[i], i, 0,0,CALLBACK_NULL);
                    if (resultopen != MMSYSERR_NOERROR) {
                        char errorText[256]; // Buffer to hold the error description
                        midiInGetErrorText(resultopen, errorText, sizeof(errorText)); // Get the error text
                        BPPrintMessage(odError, "=> Error opening MIDI output on %s\n%s\n", MIDIoutputname[index], errorText);
                        continue;
                        }
            //        BPPrintMessage(odInfo,"MIDI output = %d: “%s”",i,moc.szPname);
                    BPPrintMessage(odInfo, "MIDI output = %d: “%s” (handle: %p)", i, moc.szPname, (void*)hMIDIout[i]);
                    BPPrintMessage(odInfo," 👉 the name of your choice\n");
                    done_output++; done_outputport[index] = 1;
                    if(MIDIoutput[index] != i) fixed = 1;
                    MIDIoutput[index] = (int) i;
                    break;
                    }
                }
            if(!done_outputport[index]) BPPrintMessage(odError,"No valid output for “%s”\n",MIDIoutputname[index]);
            }

        // Assign input ports to those with names
        for(index = 0; index < MaxInputPorts; index++) {
            if(strlen(MIDIinputname[index]) == 0) continue;
            for(i = 0; i < numMidiInDevs; i++) {
                if (busyinput[i]) continue;
                if(midiInGetDevCaps(i, &mic, sizeof(mic)) != MMSYSERR_NOERROR) {
                    BPPrintMessage(odError,"Error retrieving MIDI input device capabilities #%d\n",i);
                    busyinput[i] = 1;
                    continue; // Skip to next device
                    }
                if(strcmp(mic.szPname,MIDIinputname[index]) == 0) {
                    busyinput[i] = 1;
                    resultopen = midiInOpen(&hMIDIin[i], i, (DWORD_PTR)MyMIDIInProc, (DWORD_PTR)i, CALLBACK_FUNCTION);
                    if (resultopen != MMSYSERR_NOERROR) {
                        char errorText[256]; // Buffer to hold the error description
                        midiInGetErrorText(resultopen, errorText, sizeof(errorText)); // Get the error text
                        BPPrintMessage(odError, "=> Error opening MIDI input on “%s”\n%s\n", MIDIinputname[index], errorText);
                        continue; // Skip to the next device
                        }
                    if(midiInStart(hMIDIin[i]) != MMSYSERR_NOERROR) {
                        BPPrintMessage(odError,"=> Error starting MIDI input on “%s”\n",MIDIinputname[index]);
                        continue; // Skip to next device
                        }
                    BPPrintMessage(odInfo,"MIDI input = %d: “%s” (handle: %p)",i,mic.szPname,(void*)hMIDIin[i]);
        //             BPPrintMessage(odInfo,"MIDI input = %d: “%s”",i,mic.szPname);
                    BPPrintMessage(odInfo," 👉 the name of your choice\n");
                    done_input++; done_inputport[index] = 1;
                    if(MIDIinput[index] != i) fixed = 1;
                    MIDIinput[index] = (int)i;
                    /*  sourceIndices[index] = (int*)malloc(sizeof(int));
                     *sourceIndices[index] = index; */
                    Interactive = TRUE;
                    BPPrintMessage(odInfo,"MIDI input %d makes BP3 interactive\n",MIDIinput[index]);
                    break;
                    }
                }
            if(!done_inputport[index]) BPPrintMessage(odError,"=> No valid input for “%s”\n",MIDIinputname[index]);
            }

        // Assign output ports to those without names but possibly with numbers
        if(done_output <  MaxOutputPorts) {
            BPPrintMessage(odInfo,"Trying to assign ports to %d output(s) without names but possibly with numbers\n",(MaxOutputPorts - done_output));
            for(index = 0; index < MaxOutputPorts; index++) {
                if(done_outputport[index]) continue;
                found = 0; firstchoice = -1;
                for(i = 0; i < numMidiOutDevs; i++) {
                    if(busyoutput[i]) continue;
                    if(midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS)) != MMSYSERR_NOERROR) {
                        BPPrintMessage(odError,"Error retrieving MIDI device capabilities #%d\n",i);
                        busyoutput[i] = 1;
                        continue; // Skip to next device
                        }
                    resultopen = midiOutOpen(&hMIDIout[i], i, 0,0,CALLBACK_NULL);
                    if (resultopen != MMSYSERR_NOERROR) {
                        char errorText[256]; // Buffer to hold the error description
                        midiInGetErrorText(resultopen, errorText, sizeof(errorText)); // Get the error text
                        BPPrintMessage(odError, "=> Error opening MIDI output device #%d\n%s\n", i, errorText);
                        busyoutput[i] = 1;
                        continue;
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
                        BPPrintMessage(odInfo,"MIDI output = %d: “%s”",firstchoice,moc.szPname);
                        BPPrintMessage(odInfo," 👉 choice by default\n");
                        strcpy(MIDIoutputname[index],name);
                        done_output++;
                        busyoutput[firstchoice] = 1;
                        MIDIoutput[index] = firstchoice;
                        changed = 1;
                        }
                    }
                }
            }
        if(!done_output && MaxOutputPorts > 0) BPPrintMessage(odError,"=> No port is available for a MIDI output\n");

        // Assign input ports to those without names but possibly with numbers
        if(done_input <  MaxInputPorts) {
            BPPrintMessage(odInfo,"Trying to assign ports to %d input(s) without names but possibly with numbers\n",(MaxInputPorts - done_input));
            for(index = 0; index < MaxInputPorts; index++) {
                if(done_inputport[index]) continue;
                found = 0; firstchoice = -1;
                for(i = 0; i < numMidiInDevs; i++) {
                    if(busyinput[i]) continue;
                    if(midiInGetDevCaps(i, &mic, sizeof(mic)) != MMSYSERR_NOERROR) {
                        BPPrintMessage(odError,"=> Error retrieving MIDI input device capabilities #%d\n",i);
                        busyinput[i] = 1;
                        continue; // Skip to next device
                        }
                    resultopen = midiInOpen(&hMIDIin[i], i, (DWORD_PTR)MyMIDIInProc, (DWORD_PTR)i, CALLBACK_FUNCTION);
                    if (resultopen != MMSYSERR_NOERROR) {
                        char errorText[256]; // Buffer to hold the error description
                        midiInGetErrorText(resultopen, errorText, sizeof(errorText)); // Get the error text
                        BPPrintMessage(odError, "=> Error opening MIDI input device #%d.\n%s\n", i, errorText);
                        busyinput[i] = 1;
                        continue; // Skip to the next device
                        }
                    if(midiInStart(hMIDIin[i]) != MMSYSERR_NOERROR) {
                        BPPrintMessage(odError,"=> Error starting MIDI input #%d\n",i);
                        busyinput[i] = 1;
                        continue; // Skip to next device
                        }
                    if(firstchoice < 0) firstchoice = i;
                    if(!found && MIDIinput[index] == i) {
                        BPPrintMessage(odInfo,"MIDI input = %d: “%s”",i,mic.szPname);
                        BPPrintMessage(odInfo," 👉 the number of your choice\n");
                        strcpy(MIDIinputname[index],mic.szPname);
                        busyinput[i] = 1;
                        done_input++;
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
                        BPPrintMessage(odInfo,"MIDI input = %d: “%s”",firstchoice,mic.szPname);
                        BPPrintMessage(odInfo," 👉 choice by default\n");
                        strcpy(MIDIinputname[index],mic.szPname);
                        done_input++;
                        busyinput[firstchoice] = 1;
                        MIDIinput[index] = firstchoice;
                        changed = 1;
                        Interactive = TRUE;
                        BPPrintMessage(odInfo,"MIDI input %d makes BP3 interactive\n",MIDIinput[index]);
                        continue;
                        }
                    }
                }
            }
        if(!done_input && MaxInputPorts > 0) BPPrintMessage(odError,"=> No port is available for a MIDI input\n");

        // Suggest more options
        more_outputs = more_inputs = 0;
        for(i = 0; i < numMidiOutDevs; i++) if(!busyoutput[i]) more_outputs = 1;
        for(i = 0; i < numMidiInDevs; i++) if(!busyinput[i]) more_inputs = 1;
        if(more_outputs) {
            BPPrintMessage(odInfo,"\n🎶 More MIDI output options are available:\n");
            for(i = 0; i < numMidiOutDevs; i++) {
                if(busyoutput[i]) continue;
                midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS));
                if(0 && midiOutOpen(&hMIDIout[i], i, 0,0,CALLBACK_NULL) != MMSYSERR_NOERROR) {
                    continue; // Skip to next device
                    }
                BPPrintMessage(odInfo,"MIDI output = %d: “%s”\n",i,moc.szPname); 
           //     BPPrintMessage(odInfo,"MIDI output = %d: “%s” (handle %p)\n",i,moc.szPname,(void*)hMIDIout[i]);
           //     midiOutClose(hMIDIout[i]);
                hMIDIout[i] = NULL;
                }
            }
        if(more_inputs) {
            BPPrintMessage(odInfo,"\n🎶 More MIDI input options are available:\n");
            for(i = 0; i < numMidiInDevs; i++) {
                if(busyinput[i]) continue;
                midiInGetDevCaps(i, &mic, sizeof(mic));
                if(0 && midiInOpen(&hMIDIin[i], i, (DWORD_PTR)MyMIDIInProc, (DWORD_PTR) i, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
                    continue; // Skip to next device
                    }
                BPPrintMessage(odInfo,"MIDI intput = %d: “%s”\n",i,mic.szPname);
           /*     BPPrintMessage(odInfo,"MIDI intput = %d: “%s” (handle %p)\n",i,mic.szPname,(void*)hMIDIin[i]);
                midiInClose(hMIDIin[i]); */
                hMIDIin[i] = NULL;
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
        BPPrintMessage(odInfo,"\n🎹 Setting up MacOS MIDI system\n");
        status = MIDIClientCreate(CFSTR("MIDIcheck Client"),NULL,NULL,&MIDIoutputClient);
        if(status != noErr) {
            BPPrintMessage(odError,"=> Error: Could not create MIDI client\n");
            return(FALSE);
            }
        status = MIDIOutputPortCreate(MIDIoutputClient,CFSTR("Output Port"),&MIDIoutPort);
        if(status != noErr) {
            BPPrintMessage(odError,"=> Error: Could not create output port\n");
            return(FALSE);
            }
        ItemCount outputCount = MIDIGetNumberOfDestinations();
        if(outputCount == 0) {
            BPPrintMessage(odError,"=> Error: No MIDI output available\n");
            return(FALSE);
            }
        status = MIDIClientCreate(CFSTR("MIDI Client"),NULL,NULL,&MIDIinputClient);
        if(status != noErr) {
            BPPrintMessage(odError,"=> Error: Could not create MIDI input client\n");
            return(FALSE);
            }
        ItemCount inputCount = MIDIGetNumberOfSources();
        if(inputCount == 0) {
            BPPrintMessage(odError,"=> Error: No MIDI input available\n");
            return(FALSE);
            }
        // Assign output ports to those with names
        for(index = 0; index < MaxOutputPorts; index++) {
            for(i = 0; i < outputCount; i++) {
                outputdestination = MIDIGetDestination(i);
                if(!IsMIDIDestinationActive(outputdestination)) {
                    BPPrintMessage(odError,"This MIDI (output) %d is inactive\n",i);
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
                    BPPrintMessage(odError,"=> Error: Could not create input port with MIDI Protocol 1.0\n");
                    return(FALSE);
                    }
                inputdestination = MIDIGetSource(i);
                endpointName = NULL;
                if(MIDIObjectGetStringProperty(inputdestination, kMIDIPropertyName, &endpointName) == noErr) {
                    CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                    if(!IsMIDIDestinationActive(inputdestination)) {
                        BPPrintMessage(odError,"This MIDI input %d is inactive\n",i);
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
                        BPPrintMessage(odError,"This MIDI (output) %d is inactive\n",i);
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
                    else BPPrintMessage(odInfo,"No port is available for a new output\n");
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
                        BPPrintMessage(odError,"=> Error: Could not create input port with MIDI Protocol 1.0\n");
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
        more_outputs = more_inputs = 0;
        for(i = 0; i < outputCount; i++) if(!busyoutput[i]) more_outputs = 1;
        for(i = 0; i < inputCount; i++) if(!busyinput[i]) more_inputs = 1;
        if(more_outputs) {
            BPPrintMessage(odInfo,"\n🎶 More MIDI output options are available:\n");
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
            BPPrintMessage(odInfo,"\n🎶 More MIDI input options are available:\n");
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
        int client, firstclient, port_id, port_cap;
        char somename[64];
        BPPrintMessage(odInfo,"\n🎹 Setting up Linux MIDI system\n");
        if((error = snd_seq_open(&Seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0)) < 0) {
            BPPrintMessage(odError, "=> Error opening ALSA sequencer “%s”\n", snd_strerror(error));
            return ABORT;
            }
        snd_seq_set_client_name(Seq_handle, "BP3 client");

        // Create the MIDI ports
        for(i = 0; i < MaxOutputPorts; i++) {
            my_sprintf(somename, "Output Port %d", i);
            Out_port[i] = snd_seq_create_simple_port(Seq_handle, somename,SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
            if(Out_port[i] < 0)
                BPPrintMessage(odError,"=> Failed to create output port %d. %s\n", i,snd_strerror(Out_port[i]));
        //    else BPPrintMessage(odInfo,"Created output port [%d] = %d\n", i,Out_port[i]);
            }
        for(i = 0; i < MaxInputPorts; i++) {
            my_sprintf(somename, "Input Port %d", i);
            In_port[i] = snd_seq_create_simple_port(Seq_handle, somename,SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
            if(In_port[i] < 0)
                BPPrintMessage(odError,"=> Failed to create input port %d. %s\n", i,snd_strerror(In_port[i]));
         //   else BPPrintMessage(odInfo,"Created input port [%d] = %d\n", i,In_port[i]);
            }

        // Allocate temporary data for port info
        snd_seq_client_info_alloca(&cinfo);
        snd_seq_port_info_alloca(&pinfo);

        rtMIDI = 0; // It will be reset to 1 if at least an output is connected

        if(MaxInputPorts > 0) {
            BPPrintMessage(odInfo, "Assigning MIDI input port(s) by names\n");
            for(index = 0; index < MaxInputPorts; index++) {
                snd_seq_client_info_set_client(cinfo, -1);  // Reset client info
                while (snd_seq_query_next_client(Seq_handle, cinfo) == 0) {
                    client = snd_seq_client_info_get_client(cinfo);
                    if(client == 0) continue;
                    if(doneMIDIinput[client]) continue; 
                    snd_seq_port_info_set_client(pinfo, client);
                    snd_seq_port_info_set_port(pinfo, -1); // Reset port info
                    while (snd_seq_query_next_port(Seq_handle, pinfo) == 0) {
                        port_cap = snd_seq_port_info_get_capability(pinfo);
                        port_id = snd_seq_port_info_get_port(pinfo);
                        port_name = snd_seq_port_info_get_name(pinfo);
                  //      BPPrintMessage(odInfo, "#??? MIDI input = %d: \"%s\" (port %d)\n",client,port_name,port_id);
                        if((port_cap & SND_SEQ_PORT_CAP_READ) && !(port_cap & SND_SEQ_PORT_CAP_NO_EXPORT)) {
                   //         BPPrintMessage(odInfo, "??? MIDI input = %d: \"%s\" (port %d)\n",client,port_name,port_id);
                            if(strcmp(port_name, MIDIinputname[index]) == 0) {
                                port_id = snd_seq_port_info_get_port(pinfo);
                                for (i = 0; i < MaxInputPorts; i++) {
                                    if(In_port[i] < 0) continue;
                          // Unexpectedly, In_port[i] = 1 although the port is 0. So, we do the following:
                                    if(1 || snd_seq_connect_from(Seq_handle, In_port[i], client, port_id) < 0) {
                                        BPPrintMessage(odInfo,"Unexpectedly forcing the following port to %d\n",port_id);
                                        snd_seq_connect_from(Seq_handle, port_id, client, port_id);
                                        }
                                    BPPrintMessage(odInfo, "MIDI input = %d: \"%s\" (port %d) 👉 the name of your choice\n",client,port_name,port_id);
                                    doneMIDIinput[client] = 1;
                                    done_input++;
                                    busyinput[index] = 1;
                                    if(MIDIinput[index] != client) fixed = 1;
                                    MIDIinput[index] = client;
                                    MIDIinputport[index] = port_id;
                                    if(!Interactive) BPPrintMessage(odInfo,"😀 MIDI input %d makes the application interactive\n", client);
                                    Interactive = TRUE;
                                    break;
                                    }
                                }
                            }
                        }
                    if (doneMIDIinput[client]) break;
                    }
                }
            }

        if(done_input < MaxInputPorts) {
            BPPrintMessage(odInfo, "Assigning MIDI input port(s) by client numbers\n");
            for(index = 0; index < MaxInputPorts; index++) {
                snd_seq_client_info_set_client(cinfo, -1);  // Reset client info
                if(busyinput[index]) continue;  // Skip already assigned input ports
                firstchoice = -1; found = 0;
                snd_seq_client_info_set_client(cinfo, -1);  // Reset client info
                while (snd_seq_query_next_client(Seq_handle, cinfo) == 0) {
                    client = snd_seq_client_info_get_client(cinfo);
                    if(client == 0) continue;
                    if(doneMIDIinput[client]) continue; 
                    snd_seq_port_info_set_client(pinfo, client);
                    snd_seq_port_info_set_port(pinfo, -1); // Reset port info
                    while (snd_seq_query_next_port(Seq_handle, pinfo) == 0) {
                        port_cap = snd_seq_port_info_get_capability(pinfo);
                        port_id = snd_seq_port_info_get_port(pinfo);
                        port_name = snd_seq_port_info_get_name(pinfo);
                        if((port_cap & SND_SEQ_PORT_CAP_READ) && !(port_cap & SND_SEQ_PORT_CAP_NO_EXPORT)) {
                            if(firstchoice < 0) {
                                firstchoice = port_id;
                                firstclient = client;
                                strcpy(firstname,port_name);
                                }
                            if(client == MIDIinput[index]) {
                                for (i = 0; i < MaxInputPorts; i++) {
                                    if(In_port[i] < 0) continue;
                                    // Again port_id instead of In_port[i]:
                                    snd_seq_connect_from(Seq_handle, port_id, client, port_id);
                                    BPPrintMessage(odInfo, "MIDI input = %d: \"%s\" (port %d) 👉 the number of your choice\n",client,port_name,port_id);
                                    doneMIDIinput[client] = 1;
                                    done_input++;
                                    busyinput[index] = 1;
                                    changed = 1;
                                    MIDIinputport[index] = port_id;
                                    strcpy(MIDIinputname[index],port_name);
                                    if(!Interactive) BPPrintMessage(odInfo,"😀 MIDI input %d makes the application interactive\n", client);
                                    Interactive = found = 1;
                                    break;
                                    }
                                }
                            }
                        }
                    if (doneMIDIinput[client]) break;
                    }
                if(!found) {
                    if(firstchoice >= 0) {
                        port_id = firstchoice;
                        client = firstclient;
                        for (int i = 0; i < MaxInputPorts; i++) {
                            if(In_port[i] < 0) continue;
                            // Again port_id instead of In_port[i]:
                            snd_seq_connect_from(Seq_handle, port_id, client, port_id);
                            BPPrintMessage(odInfo, "MIDI input = %d: \"%s\" (port %d)  👉 choice by default\n",client,firstname,port_id);
                            done_input++;
                            busyinput[index] = 1;
                            changed = 1;
                            MIDIinput[index] = client;
                            MIDIinputport[index] = port_id;
                            strcpy(MIDIinputname[index],firstname);
                            doneMIDIinput[client] = 1;
                            found = 1;
                            break;
                            }
                        }
                    }
                }
            }

        // Allocate temporary data for port info
        snd_seq_client_info_alloca(&cinfo);
        snd_seq_port_info_alloca(&pinfo);

        BPPrintMessage(odInfo, "Assigning MIDI output port(s) by names\n");
        for(index = 0; index < MaxOutputPorts; index++) {
            snd_seq_client_info_set_client(cinfo, -1);  // Reset client info
            while(snd_seq_query_next_client(Seq_handle, cinfo) == 0) {
                client = snd_seq_client_info_get_client(cinfo);
                if(client == 0) continue;
                if(doneMIDIoutput[client]) continue; 
                snd_seq_port_info_set_client(pinfo, client);
                snd_seq_port_info_set_port(pinfo, -1);
                while(snd_seq_query_next_port(Seq_handle, pinfo) == 0) {
                    port_cap = snd_seq_port_info_get_capability(pinfo);
                    if(!(port_cap & SND_SEQ_PORT_CAP_WRITE) ||
                        (port_cap & SND_SEQ_PORT_CAP_NO_EXPORT)) continue;
                    port_name = snd_seq_port_info_get_name(pinfo);
                    port_id = snd_seq_port_info_get_port(pinfo);
                    if(strcmp(port_name, MIDIoutputname[index]) == 0) {
                        for (i = 0; i < MaxOutputPorts; i++) {
                            if(Out_port[i] < 0) continue;
                            result = snd_seq_connect_to(Seq_handle, Out_port[i], client, port_id);
                         //   BPPrintMessage(odInfo,"Out_port[%d] = %d\n",i,Out_port[i]);
                            if (result < 0) {
                                BPPrintMessage(odError,"=> Error connecting to MIDI port: Client %d, Port %d, Error: %s\n", client, port_id, snd_strerror(result));
                                return ABORT;
                                }
                            BPPrintMessage(odInfo,"MIDI output = %d: \"%s\" (port %d) 👉 the name of your choice\n",client,port_name,port_id);
                            doneMIDIoutput[client] = 1;
                            done_output++;
                            if(MIDIoutput[index] != client) fixed = 1;
                            MIDIoutput[index] = client;
                            MIDIoutputport[index] = port_id;
                            rtMIDI = 1;
                            break;
                            }
                        }
                    }
                if(doneMIDIoutput[client]) break;   
                }
            }

        if(done_output <  MaxOutputPorts) {
            BPPrintMessage(odInfo, "Assigning MIDI output port(s) by numbers\n");
            for(index = 0; index < MaxOutputPorts; index++) {
                firstchoice = -1; found = 0;
                snd_seq_client_info_set_client(cinfo, -1);  // Reset client info
                while(snd_seq_query_next_client(Seq_handle, cinfo) == 0) {
                    client = snd_seq_client_info_get_client(cinfo);
                    if(doneMIDIoutput[client]) continue;
                    if(client == 0) continue;
                    snd_seq_port_info_set_client(pinfo, client);
                    snd_seq_port_info_set_port(pinfo, -1);
                    while(snd_seq_query_next_port(Seq_handle, pinfo) == 0) {
                        port_cap = snd_seq_port_info_get_capability(pinfo);
                        port_id = snd_seq_port_info_get_port(pinfo);
                        if (!(port_cap & SND_SEQ_PORT_CAP_WRITE) || (port_cap & SND_SEQ_PORT_CAP_NO_EXPORT)) continue;
                        port_name = snd_seq_port_info_get_name(pinfo);
                     // BPPrintMessage(odInfo, "??? MIDI output = %d: \"%s\" client %d\n", port_id, port_name, client);
                        if(firstchoice < 0) {
                            firstchoice = port_id;
                            firstclient = client;
                            strcpy(firstname,port_name);
                            }
                        if(MIDIoutput[index] == client) {
                            for (int i = 0; i < MaxOutputPorts; i++) {
                                if(Out_port[i] < 0) continue;
                                snd_seq_connect_to(Seq_handle, Out_port[i], client, port_id);
                                BPPrintMessage(odInfo, "MIDI output = %d: \"%s\" (port %d) 👉 the number of your choice\n",client,port_name,port_id);
                                done_output++;
                                busyoutput[index] = 1;
                                doneMIDIoutput[client] = 1;
                                MIDIoutputport[index] = port_id;
                                strcpy(MIDIoutputname[index],port_name);
                                changed = 1;
                                found = 1;
                                rtMIDI = 1;
                                break;
                                }
                            }
                        }
                    if(doneMIDIoutput[client]) break;
                    }
                if(!found) {
                    if(firstchoice >= 0) {
                        port_id = firstchoice;
                        client = firstclient;
                  //    BPPrintMessage(odInfo, "### MIDI output = %d: \"%s\" client %d\n", port_id, firstname, client);
                        for (int i = 0; i < MaxOutputPorts; i++) {
                            if(Out_port[i] < 0) continue;
                            snd_seq_connect_to(Seq_handle, Out_port[i], client, port_id);
                            BPPrintMessage(odInfo, "MIDI output = %d: \"%s\" (port %d)  👉 choice by default\n",client,firstname,port_id);
                            done_output++;
                            busyoutput[index] = 1;
                            changed = 1;
                            MIDIoutput[index] = client;
                            MIDIoutputport[index] = port_id;
                            strcpy(MIDIoutputname[index],firstname);
                            doneMIDIoutput[client] = 1;
                            found = 1;
                            rtMIDI = 1;
                            break;
                            }
                        }
                    }
                }
            }


        // Set non-blocking mode using snd_seq_nonblock()
        if(Interactive) snd_seq_nonblock(Seq_handle, 1);  // 1 turns on non-blocking mode
        
        // Suggest more options
        more_outputs = more_inputs = 0;
        for(i = 0; i < MAXPORTS; i++) if(!busyoutput[i]) more_outputs = 1;
        for(i = 0; i < MAXPORTS; i++) if(!busyinput[i]) more_inputs = 1;
        if(more_outputs) {
            BPPrintMessage(odInfo,"\n🎶 More MIDI output options are available:\n");
            snd_seq_client_info_alloca(&cinfo);
            snd_seq_client_info_set_client(cinfo, -1); // Start with the first client
            snd_seq_port_info_alloca(&pinfo);
            while (snd_seq_query_next_client(Seq_handle, cinfo) == 0) {
                client = snd_seq_client_info_get_client(cinfo);
                if(client == 0) continue;
                if(doneMIDIoutput[client]) continue;
                snd_seq_port_info_set_client(pinfo, client);
                snd_seq_port_info_set_port(pinfo, -1); // Reset port count for each client
                while (snd_seq_query_next_port(Seq_handle, pinfo) == 0) {
                    port_cap = snd_seq_port_info_get_capability(pinfo);
                    port_id = snd_seq_port_info_get_port(pinfo);
                    // Check for writable and not no-export capabilities
                    if ((port_cap & SND_SEQ_PORT_CAP_WRITE) && !(port_cap & SND_SEQ_PORT_CAP_NO_EXPORT)) {
                        BPPrintMessage(odInfo,"MIDI output = %d: \"%s\" (port %d)\n",
                            client,
                            snd_seq_port_info_get_name(pinfo),
                            port_id);
                        }
                    }
                }
            }
        if(more_inputs) {
            BPPrintMessage(odInfo,"\n🎶 More MIDI input options are available:\n");
            snd_seq_client_info_alloca(&cinfo);
            snd_seq_client_info_set_client(cinfo, -1); // Start with the first client
            snd_seq_port_info_alloca(&pinfo);
            while (snd_seq_query_next_client(Seq_handle, cinfo) == 0) {
                client = snd_seq_client_info_get_client(cinfo);
                if(client == 0) continue;
                if(doneMIDIinput[client]) continue;
                snd_seq_port_info_set_client(pinfo, client);
                snd_seq_port_info_set_port(pinfo, -1); // Reset port count for each client
                while (snd_seq_query_next_port(Seq_handle, pinfo) == 0) {
                    port_cap = snd_seq_port_info_get_capability(pinfo);
                    port_id = snd_seq_port_info_get_port(pinfo);
                    // Check for readable and not no-export capabilities
                    if ((port_cap & SND_SEQ_PORT_CAP_READ) && !(port_cap & SND_SEQ_PORT_CAP_NO_EXPORT)) {
                        BPPrintMessage(odInfo,"MIDI input = %d: \"%s\" (port %d)\n",
                            client,
                            snd_seq_port_info_get_name(pinfo),
                            port_id);
                        }
                    }
                }
            }
    #endif
    if(rtMIDI) {
        save_midisetup();
        if(changed || fixed) {
            strcpy(filename,Midiportfilename);
            strcat(filename,"_refresh");
            ptr = my_fopen(1,filename,"w");
            fputs("refresh this midiport!\n",ptr);
            my_fclose(ptr);
            }
        if(fixed) BPPrintMessage(odInfo,"🎹 Number(s) of MIDI input or/and output changed and will be updated when saving the page of your project\n\n");
        if(changed) BPPrintMessage(odInfo,"=> 🎹 Name(s) of MIDI input or/and output changed and will be updated when saving the page of your project\n\n");
    //  Panic = TRUE; return(ABORT);
        }
    return(OK);
    }

// CLOSE MIDI SYSTEM

void closeMIDISystem() {
    BPPrintMessage(odInfo,"Closing MIDI system\n");
    int index,port;
    #if defined(_WIN64)
        // Windows MIDI cleanup
        BPPrintMessage(odInfo,"Closing Windows MIDI system\n");
        for(index = 0; index < MAXPORTS; index++) {
            port = MIDIoutput[index];
            if(hMIDIout[port] != NULL) {
                midiOutClose(hMIDIout[port]);  // Close the MIDI output device
                hMIDIout[port] = NULL;         // Reset the handle to NULL after closing
                }
            port = MIDIinput[index];
            if(hMIDIin[port] != NULL) {
                midiInStop(hMIDIin[port]);
                midiInClose(hMIDIin[port]);
                hMIDIin[port] = NULL;
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
    if (Seq_handle != NULL) {
        snd_seq_query_subscribe_t *query;
        const snd_seq_addr_t *sender;
        const snd_seq_addr_t *dest;

        snd_seq_drain_output(Seq_handle);

        WaitABit(1000); // Sleep for 1000 milliseconds
        snd_seq_query_subscribe_alloca(&query);

        // Set to read all subscribers of our client
 /*       snd_seq_query_subscribe_set_root(query, snd_seq_client_id(Seq_handle));
        snd_seq_query_subscribe_set_type(query, SND_SEQ_QUERY_SUBS_READ);
        snd_seq_query_subscribe_set_index(query, 0); */

        while (snd_seq_query_port_subscribers(Seq_handle, query) == 0) {
            sender = snd_seq_port_subscribe_get_sender((const snd_seq_port_subscribe_t *)query);
            dest = snd_seq_port_subscribe_get_dest((const snd_seq_port_subscribe_t *)query);
            int my_port = 0; // You need to determine which of your ports is connected

            // Disconnect each subscriber
            fprintf(stderr, "Disconnecting from sender client %d, port %d at my port %d\n",
                    sender->client, sender->port, my_port);
            fprintf(stderr, "Disconnecting to dest client %d, port %d from my port %d\n",
                    dest->client, dest->port, my_port);
            if (snd_seq_disconnect_from(Seq_handle, my_port, sender->client, sender->port) < 0 ||
                snd_seq_disconnect_to(Seq_handle, my_port, dest->client, dest->port) < 0) {
                fprintf(stderr, "Error disconnecting\n");
                }

            // Move to the next subscription
            snd_seq_query_subscribe_set_index(query, snd_seq_query_subscribe_get_index(query) + 1);
            }

        // Close the sequencer handle
        snd_seq_close(Seq_handle);
        Seq_handle = NULL;
        }
    #endif
    }


// RECEIVE MIDI EVENTS

#if defined(_WIN64)
    // Placeholder for Windows MIDI input callback
    void CALLBACK MyMIDIInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
        MIDIHDR *midiHdr;
        MIDI_Event e;
        static MIDI_Event last_e = {0, 0, 0};
        int index, type, status;
        static int last_index = -1;
        static int already = 0;
        // Find index of this MIDI input
        for (index = 0; index < MaxInputPorts; index++) {
            if (hMIDIin[MIDIinput[index]] == hMidiIn) break;
            }
        if(index >= MaxInputPorts) {
        //      if(trace_all_interactions) BPPrintMessage(odError, "=> Error index in MyMIDIInProc() for event #%d\n",wMsg);
            return;
            }
        // if(trace_all_interactions) BPPrintMessage(odInfo, "OK %d index = %d\n",(int)wMsg,index);
        switch (wMsg) {
            case MIM_ERROR:
            case MIM_LONGERROR:
            case MIM_OPEN:
            case MIM_CLOSE:
                break;
            case MIM_DATA:
                status = e.status = dwParam1;
                e.data1 = e.data2 = 0;
                if (AcceptEvent(status, index)) {
                    if (status == TimingClock || status == ActiveSensing) break;
                    if (status > TimingClock) {
                        if(trace_all_interactions) BPPrintMessage(odInfo, "+ Received: %d\n",status);
                        // These are real-time messages that can be processed immediately
                        unsigned char midiData[1] = {status};
                        MIDIPacket packet = {
                            .data = midiData,
                            .length = 1,
                            .timestamp = dwParam2
                            };
                       HandleInputEvent(&packet, &e, index);
                        if (PassInEvent(status, index)) {
                            sendMIDIEvent(midiData, 1, 0); // Note: timestamp is not used here
                            }
                        }
                    else {
                        // MIDI Short Message received, dwParam1 is the MIDI data, dwParam2 is the timestamp
            // if(trace_all_interactions) BPPrintMessage(odInfo, "+++ Received: %d\n",status);
                        e.status = (unsigned char)(dwParam1 & 0xFF);
                        e.data1 = (unsigned char)((dwParam1 >> 8) & 0xFF);
                        e.data2 = (unsigned char)((dwParam1 >> 16) & 0xFF);
                        type = e.status & 0xF0;
                        unsigned char midiData[3] = {e.status, e.data1, e.data2};
                        MIDIPacket packet = {
                            .data = midiData,
                            .length = 3,
                            .timestamp = dwParam2 // Use dwParam2 as timestamp
                            };
                        HandleInputEvent(&packet, &e, index);
                        if (PassInEvent(type, index)) {
                            switch (type) {
                                case NoteOn:
                                case NoteOff:
                                case ControlChange:
                                case ProgramChange:
                                case ChannelPressure:
                                case PitchBend:
                                    // Check for duplicate events to stop bouncing
                                    if (e.status == last_e.status && e.data1 == last_e.data1 && e.data2 == last_e.data2 && index == last_index) {
                                        if (already < 10) {
                                            BPPrintMessage(odError, "=> Duplicate event detected, skipping:\n");
                                            BPPrintMessage(odError, "➡ Event Type = %d, Data1 = %d, Data2 = %d\n", e.status, e.data1, e.data2);
                                            }
                                        already++;
                                        return;
                                        }
                                    last_e = e;
                                    last_index = index;
                                    break;
                                default:
                                    // Handle system real-time messages or others here
                                    break;
                                }
                            sendMIDIEvent(midiData, 3, 0);
                            }
                        }
                    }
                break;
            case MIM_LONGDATA:
                // MIDI Sysex or Long Message received, dwParam1 points to a MIDIHDR structure
                midiHdr = (MIDIHDR *)dwParam1;
                // Handle SysEx or long messages
                break;
            }
        return;
        }
#endif

#if defined(__APPLE__)
    // Placeholder for MacOS MIDI input
    static void MyMIDIReadProc(const MIDIPacketList* packetList, void* readProcRefCon, void* srcConnRefCon) {
        (void)readProcRefCon;  // Unused parameter. This is necessary in order to avoid warnings.
        const MIDIPacket* packet;
        MIDI_Event e;
        static MIDI_Event last_e = {0, 0, 0};
        int index, status;
        static int last_index = -1;
        static int already = 0;
        packet = &packetList->packet[0];
        if(!srcConnRefCon) {
            BPPrintMessage(odError,"=> Received NULL srcConnRefCon\n");
            return;
            }
        index = *(int*)srcConnRefCon;
    //   BPPrintMessage(odInfo,"sourceIndex = %d\n",index);
        if(packet == NULL) {
            BPPrintMessage(odError,"=> No packets received\n");
            return;  // Early exit if packet is NULL
            }
        if(AcceptEvent(ByteToInt(packet->data[0]),index)) {
            for(unsigned int i = 0; i < packetList->numPackets; i++) {
                HandleInputEvent(packet,&e,index);
                if(PassInEvent(ByteToInt(packet->data[0]),index)) {
                    status = e.type & 0xF0;
                    // Check for duplicate events to stop bouncing
                    if (status == NoteOn || status == NoteOff) {
                        if (e.type == last_e.type && e.data1 == last_e.data1 && e.data2 == last_e.data2 && index == last_index) {
                            if (trace_all_interactions) already = 0;
                            if (already < 10) {
                                BPPrintMessage(odError, "=> Duplicate event detected, skipping:\n");
                                BPPrintMessage(odError, "➡ Note = %d velocity = %d\n", e.data1, e.data2);
                                }
                            already++;
                            return;
                            }
                        last_e = e;
                        last_index = index;
                        }
                    sendMIDIEvent((unsigned char*) packet->data,packet->length,0);
                    }
                packet = MIDIPacketNext(packet); // Move to the next packet
                }
            }
        return;
        }
#endif

#if defined(__linux__)
    void MyAlsaMidiInProc(snd_seq_event_t* ev) {
        MIDI_Event e;
        MIDIPacket packet;
        int index, midiData[3];
        static snd_seq_event_t* lastEvent = NULL;
        static int lastClient = -1;
        if (!ev) {
            BPPrintMessage(odError,"=> Invalid event pointer passed to MyAlsaMidiInProc()\n");
            return;
            }
        int source_port = ev->source.port;  // ALSA event source port
        int source_client = ev->source.client;

     //   BPPrintMessage(odInfo,"Processing Event: %p source %d port %d\n",ev,source_client,source_port);
        for(index = 0; index < MaxInputPorts; index++)
            if(MIDIinput[index] == source_client) break;
        if(index >= MaxInputPorts) goto SORTIE;
        unsigned char data_buffer[256];  // Adjust size as needed, especially for SysEx events
        packet.data = data_buffer;
        packet.timestamp = ev->time.time.tv_sec * 1000000000L + ev->time.time.tv_nsec;
        switch (ev->type) {
            case SND_SEQ_EVENT_NOTEON:
                e.status = 0x90 | (ev->data.note.channel & 0x0F);  // Note On command with channel
                e.data1 = ev->data.note.note;  // Note value
                e.data2 = ev->data.note.velocity;  // Velocity
                packet.length = 3;
                packet.data[0] = e.status;
                packet.data[1] = e.data1;
                packet.data[2] = e.data2;
                //  if(TraceMIDIinteraction && AcceptEvent(e.status, index)) BPPrintMessage(odInfo,"👉 Received NoteOn %d velocity %d, client %d\n",e.data1,e.data2,source_client);
                break;
            case SND_SEQ_EVENT_NOTEOFF:
                e.status = 0x80 | (ev->data.note.channel & 0x0F);  // Note Off command with channel
                e.data1 = ev->data.note.note;  // Note value
                e.data2 = ev->data.note.velocity;  // Velocity
                packet.length = 3;
                packet.data[0] = e.status;
                packet.data[1] = e.data1;
                packet.data[2] = e.data2;
                break;
            case SND_SEQ_EVENT_CONTROLLER:
                e.status = 0xB0 | (ev->data.control.channel & 0x0F);  // Control Change command with channel
                e.data1 = ev->data.control.param;  // Controller number
                e.data2 = ev->data.control.value;  // Controller value
                packet.length = 3;  // Common MIDI messages are 3 bytes
                packet.data[0] = e.status;
                packet.data[1] = e.data1;
                packet.data[2] = e.data2;
                break;
            case SND_SEQ_EVENT_PITCHBEND:
                e.status = 0xE0 | (ev->data.control.channel & 0x0F);  // Pitch Bend Change command with channel
                e.data1 = ev->data.control.value & 0x7F;  // LSB (7 bits)
                e.data2 = (ev->data.control.value >> 7) & 0x7F;  // MSB (7 bits)
                packet.length = 3;  // Common MIDI messages are 3 bytes
                packet.data[0] = e.status;
                packet.data[1] = e.data1;
                packet.data[2] = e.data2;
                break;
            case SND_SEQ_EVENT_KEYPRESS:
                e.status = 0xA0 | (ev->data.note.channel & 0x0F);  // Polyphonic Key Pressure with channel
                e.data1 = ev->data.note.note;  // Note number
                e.data2 = ev->data.note.velocity;  // Pressure amount
                packet.length = 3;
                packet.data[0] = e.status;
                packet.data[1] = e.data1;
                packet.data[2] = e.data2;
                break;
            case SND_SEQ_EVENT_CHANPRESS:
                e.status = 0xD0 | (ev->data.control.channel & 0x0F);  // Channel Pressure with channel
                e.data1 = ev->data.control.value;  // Pressure amount
                packet.length = 2;  // Channel Pressure messages are 2 bytes
                packet.data[0] = e.status;
                packet.data[1] = e.data1;
                break;
            case SND_SEQ_EVENT_PGMCHANGE:
                e.status = 0xC0 | (ev->data.control.channel & 0x0F);  // Program Change with channel
                e.data1 = ev->data.control.value;  // Program number
                packet.length = 2;  // Program Change messages are 2 bytes
                packet.data[0] = e.status;
                packet.data[1] = e.data1;
                break;
            case SND_SEQ_EVENT_START:
                e.status = 0xFA;  // Start command (250)
                packet.length = 1;
                packet.data[0] = e.status;
                if(TraceMIDIinteraction && AcceptEvent(e.status, index)) BPPrintMessage(odInfo,"👉 Received START\n");
                break;
            case SND_SEQ_EVENT_CONTINUE:
                e.status = 0xFB;  // Continue command (251)
                packet.length = 1;
                packet.data[0] = e.status;
                if(TraceMIDIinteraction && AcceptEvent(e.status, index)) BPPrintMessage(odInfo,"👉 Received CONTINUE\n");
                break;
            case SND_SEQ_EVENT_STOP:
                e.status = 0xFC;  // Stop command (252)
                packet.length = 1;
                packet.data[0] = e.status;
                if(TraceMIDIinteraction && AcceptEvent(e.status, index)) BPPrintMessage(odInfo,"👉 Received STOP\n");
                break;
            case SND_SEQ_EVENT_SENSING:
                e.status = 0xFE;  // Active Sensing
                packet.length = 1;
                packet.data[0] = e.status;
                break;
            // Add more cases here for different types of MIDI messages
            default:
                // Handle other types of messages or ignore them
                goto SORTIE;
            }
        if(AcceptEvent(e.status, index)) {
    //         BPPrintMessage(odInfo, "Received client = %d, status = %d, data1 = %d, data2 = %d\n", source_client, e.status, e.data1, e.data2);
            HandleInputEvent(&packet,&e,index);
            if (PassInEvent(e.status, index)) {
                // Check for duplicate events to stop bouncing
                if (ev->type == SND_SEQ_EVENT_NOTEON || ev->type == SND_SEQ_EVENT_NOTEOFF) {
                    if(ev == lastEvent && ev->data.note.note == lastEvent->data.note.note &&
                ev->data.note.velocity == lastEvent->data.note.velocity && source_client == lastClient) {
                        if(trace_all_interactions) already = 0;
                        if(already < 10) {
                            BPPrintMessage(odError,"=> Duplicate event detected, skipping:\n");
                            BPPrintMessage(odError,"➡ Note = %d velocity = %d, client = %d\n",ev->data.note.note,ev->data.note.velocity,lastClient);
                            }
                        already++;
                        goto SORTIE;
                        }
                    lastEvent = ev;
                    lastClient = source_client;
                    }
                if(trace_all_interactions) BPPrintMessage(odInfo, "Forwarding client = %d, status = %d, data1 = %d, data2 = %d, ev = %p\n", source_client, e.status, e.data1, e.data2,ev);
                sendMIDIEvent(packet.data, packet.length, packet.timestamp);
                }
            }
    SORTIE:
        snd_seq_free_event(ev);
        return;
        }
#endif


// LISTEN TO MIDI

int ListenMIDI(int x0, int x1, int x2) {
    int r = OK;
    if (EmergencyExit || Panic) return ABORT;
    if (!Interactive) return r;
    #if defined(__linux__)
    snd_seq_event_t* ev = NULL;
    int result;

    if ((result = snd_seq_event_input(Seq_handle, &ev)) >= 0) {
        if(ev == NULL) {
            return r;  // Skip this iteration if the event is NULL
            }
        MyAlsaMidiInProc(ev);  // Process the event
        snd_seq_free_event(ev); // Free the event data structure
        }
    if (result == -EAGAIN) return r;  // No events to process, just return
    if (result < 0) {
        BPPrintMessage(odError, "=> Error reading MIDI event: %s\n", snd_strerror(result));
        return ABORT;  // Return error if it's not just an empty queue
        }
    #endif
    return r;
    }


// SEND MIDI EVENTS

void sendMIDIEvent(unsigned char* midiData,int dataSize,long time) {
    int note,status,value,improvize,index,channel;
    unsigned long clocktime;
    status = midiData[0];
    if(dataSize == 3) {
        status &= 0xF0;
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
        }
    #if defined(_WIN64)
        DWORD message = 0;
        status = midiData[0];
        channel = -1;  // Default to -1 for messages that do not use channels
        if (dataSize == 1)
            message = midiData[0];
        else if (dataSize == 3) {
            status = midiData[0] & 0xF0;
            channel = midiData[0] & 0x0F;
            message = midiData[0] | (midiData[1] << 8) | (midiData[2] << 16);
            }
        for (index = 0; index < MaxOutputPorts; index++) {
            if (hMIDIout[MIDIoutput[index]] == NULL) continue;
// BPPrintMessage(odInfo,"1) MIDI event time = %ld ms, status = %ld, note %ld, velocity = %ld channel = %d\n",(long)time/1000L,(long)status,(long)note,(long)value,channel);
            if (channel != -1 && MIDIchannelFilter[index][channel] == '0') continue;
// BPPrintMessage(odInfo,"2) MIDI event time = %ld ms, status = %ld, note %ld, velocity = %ld channel = %d\n",(long)time/1000L,(long)status,(long)note,(long)value,channel);
            if (!PassOutEvent(status, index)) continue;
            if (dataSize <= 3) {
             //    if (trace_all_interactions && dataSize == 3) BPPrintMessage(odInfo,"Sending MIDI event time = %ld ms, status = %ld, note %ld, velocity = %ld, output = %d\n",(long)time/1000L,(long)status,(long)note,(long)value,MIDIoutput[index]);
                 if (trace_all_interactions && dataSize == 1) BPPrintMessage(odInfo,"Sending MIDI event time = %ld ms, status = %ld, ouput = %d\n",(long)time/1000L,(long)status,MIDIoutput[index]);
                midiOutShortMsg(hMIDIout[MIDIoutput[index]], message);
                }
            else {
                // Handle long messages such as SysEx
                MIDIHDR midiHdr;
                memset(&midiHdr, 0, sizeof(MIDIHDR));
                midiHdr.lpData = (LPSTR)midiData;
                midiHdr.dwBufferLength = dataSize;
                midiHdr.dwBytesRecorded = dataSize;
                midiOutPrepareHeader(hMIDIout[MIDIoutput[index]], &midiHdr, sizeof(MIDIHDR));
                midiOutLongMsg(hMIDIout[MIDIoutput[index]], &midiHdr, sizeof(MIDIHDR));
                midiOutUnprepareHeader(hMIDIout[MIDIoutput[index]], &midiHdr, sizeof(MIDIHDR));
                }
            }
    #endif
    #if defined(__APPLE__)
        // MacOS MIDI event sending
        MIDIPacketList packetList;
        MIDIPortRef targetPort;
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
        if(Seq_handle == NULL) {
            BPPrintMessage(odError,"=> ALSA sequencer has not been initialised\n");
            return;
            }
        char line[1000];
        snd_seq_event_t ev;
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);
        int bytes_queued;
         // Set the correct event type
        if(TraceMIDIinteraction) strcpy(Message,"");
        switch (status) {
            case 0x90: // Note On
               if(trace_all_interactions) sprintf(Message,"Sending NoteOn %d velocity %d ",note,value);
                if (value == 0) // Velocity of 0 should be treated as Note Off
                    snd_seq_ev_set_noteoff(&ev, channel, note, value);
                else
                    snd_seq_ev_set_noteon(&ev, channel, note, value);
                break;
            case 0x80: // Note Off
             sprintf(Message,"Sending NoteOff %d value %d",note,value);
                snd_seq_ev_set_noteoff(&ev, channel, note, value);
                break;
            case 0xB0: // Control Change
                snd_seq_ev_set_controller(&ev, channel, note, value);
                break;
            case 0xE0:  // Pitch Bend Change
                int bend_value = (value << 7) | note; // Combining MSB and LSB
                snd_seq_ev_set_pitchbend(&ev, channel, bend_value - 8192);
                break;
            case 0xA0: // Key Pressure (Aftertouch)
                snd_seq_ev_set_keypress(&ev, channel, note, value);
                break;
            case 0xD0: // Channel Pressure (Aftertouch)
                snd_seq_ev_set_chanpress(&ev, channel, value);
                break;
            case 0xC0: // Program Change
                snd_seq_ev_set_pgmchange(&ev, channel, value);
                break;
            case 0xFA: // MIDI Start (250)
                snd_seq_ev_set_fixed(&ev);
                ev.type = SND_SEQ_EVENT_START;
                if(TraceMIDIinteraction) sprintf(Message,"👉 Sent START");
                break;
            case 0xFB: // MIDI Continue (251)
                snd_seq_ev_set_fixed(&ev);
                ev.type = SND_SEQ_EVENT_CONTINUE;
                if(TraceMIDIinteraction) sprintf(Message,"👉 Sent CONTINUE");
                break;
            case 0xFC: // MIDI Stop (252)
                snd_seq_ev_set_fixed(&ev);
                ev.type = SND_SEQ_EVENT_STOP;
                if(TraceMIDIinteraction) sprintf(Message,"👉 Sent STOP");
                break;
                // There is no direct handling for Active Sensing and Timing Clock in the ALSA sequencer
            default:
                if(trace_all_interactions) BPPrintMessage(odError,"=> Unsupported MIDI message  %d %d %d\n", status, note, value);
                return;
                break;
            }
        for (int index = 0; index < MaxOutputPorts; index++) {
          // Retrieve the port number for the given client
            int client = MIDIoutput[index];
            if (MIDIchannelFilter[index][channel] == '0') continue;
            if (!PassOutEvent(status,index)) continue;
            int port = MIDIoutputport[index];
            if(TraceMIDIinteraction || trace_all_interactions) {
                if(strlen(Message) > 0) {
                    sprintf(line,"%s, client %d, port %d",Message,client,port);
                    BPPrintMessage(odInfo,"%s\n",line);
                    }
                else {
                    BPPrintMessage(odInfo,"Some message sent to client %d, port %d\n",client,port);
                    }
                }
            snd_seq_ev_set_source(&ev,port);
            bytes_queued = snd_seq_event_output(Seq_handle,&ev);
            if (bytes_queued < 0) {
                if(trace_all_interactions) BPPrintMessage(odError,"=> Error sending MIDI event: %s\n", snd_strerror(bytes_queued));
                }
            else {
                if(trace_all_interactions) BPPrintMessage(odInfo,"Event queued successfully, %d bytes queued, index = %d\n", bytes_queued,index);
                }
            snd_seq_drain_output(Seq_handle);
            }
        snd_seq_free_event(&ev);
        return;   
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