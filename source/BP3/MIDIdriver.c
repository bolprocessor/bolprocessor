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

// The following are system-dependent codes for handling reaal-time MIDI

#if defined(__APPLE__)
    MIDIClientRef MIDIoutputClient, MIDIinputClient;
    MIDIPortRef MIDIoutPort, MIDIinPort;
    MIDIEndpointRef MIDIoutputdestination[MAXPORTS],MIDIinputdestination;
    static int *sourceIndices[MAXPORTS];
#endif

#if defined(_WIN64)
    #include <windows.h>
    #include <mmsystem.h>
    #pragma comment(lib, "winmm.lib")
    void CALLBACK MyMIDIInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    // Global variables for MIDI device handle
    static HMIDIOUT hMidiOut[MAXPORTS];
    static HMIDIOUT hMidiIn[MAXPORTS];
    for(i = 0; i < MAXPORTS; i++) {
        hMidiOut[i] = hMidiIn[i] = NULL;
        }
#elif defined(__APPLE__)
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
#elif defined(__linux__)
    #include <unistd.h> 
    #include <alsa/asoundlib.h>
    void MyAlsaMidiInProc(snd_seq_event_t *ev, void *refCon);
    void handle_midi_input(snd_seq_event_t *ev);
    // Global variable for ALSA MIDI sequencer handle
    static snd_seq_t *seq_handle = NULL;
    static int out_port[MAXPORTS],in_port[MAXPORTS];
#endif


int initializeMIDISystem(void) {
    char sourcename[MAXNAME], outputname[MAXNAME];
    int foundnum,foundname,changed,fixed,index;
    char newname[MAXNAME],filename[MAXNAME];
    FILE * ptr;

    changed = fixed = false;
    read_midisetup(); // This will modify MIDIinput and MIDIoutput
    #if defined(_WIN64)
        BPPrintMessage(odInfo,"Setting up Windows MIDI system\n");
        // Get the number of MIDI out devices in the system
        UINT numDevs = midiOutGetNumDevs();
        if(numDevs == 0) {
            BPPrintMessage(odError,"No MIDI output devices available.\n");
            return(FALSE);
            }
        // Iterate through all available devices to list them
        MIDIOUTCAPS moc;
        foundname = foundnum = 0;
        for(UINT i = 0; i < numDevs; i++) {
            MMRESULT result = midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS));
            if(result != MMSYSERR_NOERROR) {
                BPPrintMessage(odError,"Error retrieving MIDI device capabilities.\n");
                continue; // Skip to next device
                }
            BPPrintMessage(odInfo,"MIDI (output) %u: “%s”", i, moc.szPname);
            if(!foundname && strcmp(moc.szPname,outputname) == 0) {  // Name is a priority choice
                BPPrintMessage(odInfo," 👉 the name of your choice");
                MIDIoutput = (int) i;
                foundname = 1;
                }
            else if(!foundname && (int)i == MIDIoutput) {
                BPPrintMessage(odInfo," 👉 your number?");
                strcpy(newname,name);
                foundnum = changed = 1;
                }
            BPPrintMessage(odInfo,"\n");
            }
        if(foundnum && !foundname) strcpy(outputname,newname);
        if(!foundnum && !foundname && (int)numDevs <= MIDIoutput) {
            BPPrintMessage(odError,"=> Error: MIDI output should be zero to %d\n",(int)numDevs);
            return(FALSE);
            }
        // Open the default MIDI output device (or the first one found)
        UINT deviceId = 0; // Typically, 0 represents the default MIDI device
        // HMIDIOUT hMidiOut; is a global variable
        MMRESULT result = midiOutOpen(&hMidiOut, deviceId,0,0,CALLBACK_NULL);
        if(result != MMSYSERR_NOERROR) {
            BPPrintMessage(odError,"Error opening MIDI output device.\n");
            return(FALSE);
            }
        // Get the number of MIDI input devices in the system
        UINT numInDevs = midiInGetNumDevs();
        if(numInDevs == 0) {
            BPPrintMessage(odError,"No MIDI input devices available.\n");
            return(FALSE);
            }
        // Iterate through all available input devices to list them
        MIDIINCAPS mic;
        foundname = foundnum = 0;
        for(UINT i = 0; i < numInDevs; i++) {
            if(midiInGetDevCaps(i, &mic, sizeof(mic)) != MMSYSERR_NOERROR) {
                BPPrintMessage(odError,"Error retrieving MIDI input device capabilities.\n");
                continue; // Skip to next device
                }
            BPPrintMessage(odInfo,"MIDI (source) %u: “%s”\n", i, mic.szPname);
            if(!foundname && strcmp(name,sourcename) == 0) {  // Name is a priority choice
                BPPrintMessage(odInfo," 👉 the name of your choice");
                MIDIinput = (int) i;
                foundname = 1;
                }
            else if(!foundname && (int)i == MIDIinput) {
                BPPrintMessage(odInfo," = proposed MIDI source");
                strcpy(newname,mic.szPname);
                foundnum = changed = 1;
                }
            BPPrintMessage(odInfo,"\n");
            }
        if(foundnum && !foundname) strcpy(sourcename,newname);
        if(!foundnum && !foundname && (int)numInDevs <= MIDIinput) {
            BPPrintMessage(odError,"=> Error: MIDI input should be zero to %d\n",(int)numInDevs);
            return(FALSE);
            }
        // Open the default MIDI input device (or the first one found)
        if(midiInOpen(&hMidiIn, MIDIinput, (DWORD_PTR)midiInCallback, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
            BPPrintMessage(odError,"Error opening MIDI input device.\n");
            return(FALSE);
            }
        // Start the MIDI input processing
        if(midiInStart(hMidiIn) != MMSYSERR_NOERROR) {
            BPPrintMessage(odError,"Error starting MIDI input.\n");
            return(FALSE);
            }
    #elif defined(__APPLE__)
        OSStatus status;
        MIDIEndpointRef src;
        CFStringRef endpointName;
        int i, j, bad, firstchoice;
        char name[MAXNAME],firstname[MAXNAME];
        for(index = 0; index < MAXPORTS; index++) sourceIndices[index] = NULL;
        BPPrintMessage(odInfo,"Setting up MacOS MIDI system\n");
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
        ItemCount MIDIoutputinationCount = MIDIGetNumberOfDestinations();
        if(MIDIoutputinationCount == 0) {
            BPPrintMessage(odError,"=> Error: No MIDI destinations available.\n");
            return(FALSE);
            }
        for(index = 0; index < MaxOutputPorts; index++) {
            if(MIDIoutput[index] < 0) continue;
            foundname = foundnum = 0;
            for(ItemCount i = 0; i < MIDIoutputinationCount; ++i) {
                MIDIoutputdestination[index] = MIDIGetDestination(i);
                endpointName = NULL;
                if(MIDIObjectGetStringProperty(MIDIoutputdestination[index], kMIDIPropertyName, &endpointName) == noErr) {
                    CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                    BPPrintMessage(odInfo,"MIDI output [%d] = %lu: “%s”",index,i,name);
                    if(!foundname && strcmp(name,OutputMIDIportName[index]) == 0) {  // Name is a priority choice
                        BPPrintMessage(odInfo," 👉 the name of your choice");
                        if(MIDIoutput[index] != i) fixed = 1;
                        MIDIoutput[index] = (int) i;
                        foundname = 1;
                        }
                    else if(!foundname && (int)i == MIDIoutput[index]) {
                        BPPrintMessage(odInfo," 👉 your number?");
                        strcpy(newname,name);
                        foundnum = changed = 1;
                        }
                    BPPrintMessage(odInfo,"\n");
                    CFRelease(endpointName);
                    }
                if(!IsMIDIDestinationActive(MIDIoutputdestination[index]))
                    BPPrintMessage(odError,"This MIDI (output) %lu is inactive.\n",i);
                }
            if(foundnum && !foundname) strcpy(OutputMIDIportName[index],newname);
            if(!foundnum && !foundname && MIDIoutputinationCount <= MIDIoutput[index]) {
                BPPrintMessage(odError,"=> Error: MIDI output number should be zero to %d\n",(int)MIDIoutputinationCount);
                return(FALSE);
                }
            MIDIoutputdestination[index] = MIDIGetDestination(MIDIoutput[index]);
            }

        // Create MIDI input client and port
        status = MIDIClientCreate(CFSTR("MIDI Client"),NULL,NULL,&MIDIinputClient);
        if(status != noErr) {
            BPPrintMessage(odError,"Could not create MIDI input client.\n");
            return(FALSE);
            }
        /*   status = MIDIInputPortCreateWithProtocol(MIDIinputClient, CFSTR("Input Port"), kMIDIProtocol_1_0, &MIDIinPort,     MIDIInputCallback); would be better but difficult to handle! */
        status = MIDIInputPortCreate(MIDIinputClient, CFSTR("Input Port"),MyMIDIReadProc,NULL,&MIDIinPort);
        if(status != noErr) {
            BPPrintMessage(odError,"=> Could not create input port with MIDI Protocol 1.0.\n");
            return(FALSE);
            }
        ItemCount sourceCount = MIDIGetNumberOfSources();
        if(sourceCount == 0) {
            BPPrintMessage(odError,"=> No MIDI sources found.\n");
            return(FALSE);
            }
        for(index = 0; index < MaxInputPorts; index++) {
            foundname = foundnum = 0; firstchoice = -1;
            for(i = 0; i < sourceCount; ++i) {
                bad = FALSE;
                for(j = 0; j < MaxOutputPorts; j++) {
                    if(i == MIDIoutput[j]) {
                        BPPrintMessage(odError,"An input should not be on the same port (%d) as an output\n",i);
                        bad = TRUE; break;
                        }
                    }
                if(bad) continue;
                bad = FALSE;
                for(j = 0; j < index; j++) {
                    if(i == MIDIinput[j]) {
                        BPPrintMessage(odError,"MIDI input = %d already used\n",i);
                        bad = TRUE; break;
                        }
                    }
                if(bad) continue;
                MIDIinputdestination = MIDIGetSource(i);
                endpointName = NULL;
                if(MIDIObjectGetStringProperty(MIDIinputdestination, kMIDIPropertyName, &endpointName) == noErr) {
                    CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                    BPPrintMessage(odInfo,"MIDI input [%d] = %d: “%s”",index,i,name);
                    if(firstchoice == -1) {
                        firstchoice = i;
                        strcpy(firstname,name);
                        }
                    if(!foundname && strcmp(name,InputMIDIportName[index]) == 0) {  // Name is a priority choice
                        BPPrintMessage(odInfo," 👉 the name of your choice");
                        if(MIDIinput[index] != i) fixed = 1;
                        MIDIinput[index] = (int) i;
                        foundname = 1;
                        }
                    else if(!foundname && (int)i == MIDIinput[index]) {
                        BPPrintMessage(odInfo," 👉 your number?");
                        strcpy(newname,name);
                        foundnum = changed = 1;
                        }
                    BPPrintMessage(odInfo,"\n");
                    CFRelease(endpointName);
                    }
                if(!IsMIDIDestinationActive(MIDIinputdestination))
                    BPPrintMessage(odError,"=> This MIDI input %d is inactive.\n",i);
                }
            if(foundnum && !foundname) strcpy(InputMIDIportName[index],newname);
            if(!foundnum && !foundname && firstchoice >= 0) {
                MIDIinput[index] = firstchoice;
                strcpy(InputMIDIportName[index],firstname);
                BPPrintMessage(odInfo,"MIDIinput[%d] = %d: “%s” 👉 choice by default\n",index,firstchoice,InputMIDIportName[index]);
                changed = 1;
                }
            if(!foundnum && !foundname && sourceCount <= MIDIinput[index]) {
                BPPrintMessage(odError,"=> Error: MIDI input should be zero to %d\n",(int)(sourceCount - 1));
                return(FALSE);
                }
            src = MIDIGetSource(MIDIinput[index]);
            sourceIndices[index] = (int*)malloc(sizeof(int)); 
            *sourceIndices[index] = index;
            MIDIPortConnectSource(MIDIinPort,src,sourceIndices[index]);
            Interactive = TRUE;
            BPPrintMessage(odInfo,"BP3 will be interactive (MIDI input %d)\n",MIDIinput[index]);
            }
    #elif defined(__linux__)
        BPPrintMessage(odInfo,"Setting up Linux MIDI system\n");
        // Open the ALSA sequencer
        if(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
            BPPrintMessage(odError,"Error opening ALSA sequencer.\n");
            return(FALSE);
            }
        snd_seq_set_client_name(seq_handle, "My MIDI Application");
        // Create an output port (global variable)
        out_port = snd_seq_create_simple_port(seq_handle, "Out Port",
                                            SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
                                            SND_SEQ_PORT_TYPE_APPLICATION);
        if(out_port < 0) {
            BPPrintMessage(odError,"Error creating sequencer port\n");
            return(FALSE);
            }
        // Enumerate and list all clients and ports
        snd_seq_client_info_t *cinfo;
        snd_seq_port_info_t *pinfo;
        snd_seq_client_info_alloca(&cinfo);
        snd_seq_port_info_alloca(&pinfo);
        int first_client = -1, first_port = -1;
        snd_seq_client_info_set_client(cinfo, -1);
        while(snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
            int client = snd_seq_client_info_get_client(cinfo);
            snd_seq_port_info_set_client(pinfo, client);
            snd_seq_port_info_set_port(pinfo, -1);
            while(snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
                // Check if the port is an output port
                if((snd_seq_port_info_get_capability(pinfo) & (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)) == (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)) {
                    BPPrintMessage(odInfo,"Output port found: Client %d, Port %d, Name: “%s”\n",
                        snd_seq_port_info_get_client(pinfo),
                        snd_seq_port_info_get_port(pinfo),
                        snd_seq_port_info_get_name(pinfo));
                    if(first_client == -1) { // Check if this is the first available port
                        first_client = client;
                        first_port = snd_seq_port_info_get_port(pinfo);
                        }
                    }
                }
            }
        // Connect to the first available output port if found
        if(first_client != -1 && first_port != -1) {
            if(snd_seq_connect_to(seq_handle, out_port, first_client, first_port) < 0) {
                BPPrintMessage(odError,"Error connecting to MIDI port: Client %d, Port %d\n", first_client, first_port);
                return(FALSE);
                }
            BPPrintMessage(odInfo,"Connected to Client %d, Port %d\n", first_client, first_port);
            }
        else {
            BPPrintMessage(odError,"No available MIDI output ports found.\n");
            return(FALSE);
            }
        // Create an input port (global variable)
        in_port = snd_seq_create_simple_port(seq_handle, "Input Port",
                                            SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                            SND_SEQ_PORT_TYPE_APPLICATION);
        if(in_port < 0) {
            BPPrintMessage(odError,"Error creating input sequencer port.\n");
            return(FALSE);
            }
        // Connect all available sources to this input port
        snd_seq_client_info_alloca(&cinfo);
        snd_seq_port_info_alloca(&pinfo);
        snd_seq_client_info_set_client(cinfo, -1);
        while(snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
            int client = snd_seq_client_info_get_client(cinfo);
            snd_seq_port_info_set_client(pinfo, client);
            snd_seq_port_info_set_port(pinfo, -1);
            while(snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
                // Check if the port can be connected to our input port
                if((snd_seq_port_info_get_capability(pinfo) & (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ)) == (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ)) {
                    int src_client = snd_seq_port_info_get_client(pinfo);
                    int src_port = snd_seq_port_info_get_port(pinfo);
                    // Connect source to our input port
                    if(snd_seq_connect_from(seq_handle, in_port, src_client, src_port) < 0) {
                        BPPrintMessage(odError,"Error connecting from src_client %d, src_port %d\n", src_client, src_port);
                        }
                    else {
                        BPPrintMessage(odInfo,"Connected from Client %d, Port %d\n", src_client, src_port);
                        }
                    }
                }
            }
    #endif
    if(fixed && !changed) BPPrintMessage(odInfo,"MIDI port number(s) may have changed and will be updated when saving the page of your project\n");
    if(changed || fixed) {
        save_midisetup();
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

void closeMIDISystem() {
    BPPrintMessage(odInfo,"Closing MIDI system\n");
    int index;
    #if defined(_WIN64)
        // Windows MIDI cleanup
        BPPrintMessage(odInfo,"Closing Windows MIDI system\n");
        if(hMidiOut != NULL) {
            midiOutClose(hMidiOut);  // Close the MIDI output device
            hMidiOut = NULL;         // Reset the handle to NULL after closing
            }
        if(hMidiIn != NULL) {
            midiInStop(hMidiIn);
            midiInClose(hMidiIn);
            hMidiIn = NULL;
            }
    #elif defined(__APPLE__)
    // MacOS MIDI cleanup
    MIDIClientDispose(MIDIoutputClient);
    MIDIClientDispose(MIDIinputClient);
    MIDIPortDispose(MIDIinPort);
    MIDIPortDispose(MIDIoutPort);
    for(index = 0; index < MAXPORTS; index++) {
        if(sourceIndices[index] != NULL) {
            free(sourceIndices[index]);
            sourceIndices[index] = NULL;
            }
        }
    #elif defined(__linux__)
        // Linux MIDI cleanup
        if(seq_handle != NULL) {
            snd_seq_close(seq_handle);  // Close the ALSA sequencer
            seq_handle = NULL;          // Reset the handle to NULL after closing
            }
    #endif
    }

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
        printf("No packets received.\n");
        return;  // Early exit if packet is NULL
        }
    if(AcceptEvent(ByteToInt(packet->data[0]),index)) {
        for(unsigned int i = 0; i < packetList->numPackets; i++) {
            HandleInputEvent(packet,&e,index);
            if(PassEvent(ByteToInt(packet->data[0]),index)) sendMIDIEvent((unsigned char*) packet->data,packet->length,0);
            // Sending immediately
            packet = MIDIPacketNext(packet);
            }
        }
    }
#elif defined(_WIN64)
// Placeholder for Windows MIDI input callback
void CALLBACK MyMIDIInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    (void)dwInstance; // Typically unused
    switch (wMsg) {
        case MIM_OPEN:
            printf("MIDI Device Opened\n");
            break;
        case MIM_CLOSE:
            printf("MIDI Device Closed\n");
            break;
        case MIM_DATA:
            unsigned char* midiData = (unsigned char*)&dwParam1;
            sendMIDIEvent(midiData,3);  // MIDI message by dwParam1 are typically 3 bytes long
            break;
        case MIM_LONGDATA:
            MIDIHDR *midiHdr = (MIDIHDR*)dwParam1;
            sendMIDIEvent((unsigned char*)midiHdr->lpData, midiHdr->dwBytesRecorded);
            // Once processed, prepare header for more input
            midiInPrepareHeader(hMidiIn, midiHdr, sizeof(MIDIHDR));
            midiInAddBuffer(hMidiIn, midiHdr, sizeof(MIDIHDR));
            break;
        case MIM_ERROR:
            fprintf(stderr, "MIDI Input Error Occurred.\n");
            break;
        }
    }
#elif defined(__linux__)
// Placeholder for ALSA MIDI input callback
void MyAlsaMidiInProc(snd_seq_event_t *ev, void *refCon) {
    (void)refCon; // Unused parameter
    if (ev->type == SND_SEQ_EVENT_NOTEON || ev->type == SND_SEQ_EVENT_NOTEOFF) {
        unsigned char midiData[3];
        midiData[0] = (ev->type == SND_SEQ_EVENT_NOTEON) ? 0x90 : 0x80;
        midiData[1] = ev->data.note.note;
        midiData[2] = ev->data.note.velocity;
        sendMIDIEvent(midiData, 3);
        }
    }
#endif

void sendMIDIEvent(unsigned char* midiData,int dataSize,long time) {
    int note,status,value,test_first_events,improvize,index;
    unsigned long clocktime;
    test_first_events = 0;
    status = midiData[0];
    note = midiData[1];
    value = midiData[2];

    if(test_first_events && NumEventsWritten < 100) {
        clocktime = getClockTime() - initTime; // microseconds
        improvize = Improvize;
        Improvize = 0; // Necessary to activate BPPrintMessage(odInfo,...
        if(status == NoteOn || status == NoteOff)
            BPPrintMessage(odInfo,"%.3f => %.3f s status = %d, note = %d, value = %d\n",(float)clocktime/1000000,(float)time/1000000,status,note,value);
        Improvize = improvize;
        }
    if(NumEventsWritten < LONG_MAX) NumEventsWritten++;
 //   BPPrintMessage(odInfo,"Sending MIDI event time = %ld ms, status = %ld, note %ld, value = %ld\n",(long)time/1000L,(long)status,(long)note,(long)value);
    #if defined(_WIN64)
    // Windows MIDI event sending
    // Pack the bytes into a DWORD message
    DWORD msg = 0;
    for(int i = 0; i < dataSize; i++) {
        msg |= (midiData[i] << (i * 8));
        }
    // Send the MIDI message
    for(index = 0; index < MaxOutputPorts; index++)
        midiOutShortMsg(hMidiOut[index],msg);
    #elif defined(__APPLE__)
        // MacOS MIDI event sending
        MIDIPacketList packetList;
        MIDIPacket *packet = MIDIPacketListInit(&packetList);
        packet = MIDIPacketListAdd(&packetList, sizeof(packetList), packet, mach_absolute_time(), dataSize, midiData);
        if(packet) {
            for(index = 0; index < MaxOutputPorts; index++)
                MIDISend(MIDIoutPort, MIDIoutputdestination[index], &packetList);
            }
    #elif defined(__linux__)
        // Ensure ALSA sequencer is setup
        if(seq_handle == NULL) {
            if(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
                BPPrintMessage(odError,"=> Error opening ALSA sequencer.\n");
                return(ABORT);
                }
            snd_seq_set_client_name(seq_handle, "MIDI Sender");
            out_port = snd_seq_create_simple_port(seq_handle, "Out",SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,SND_SEQ_PORT_TYPE_APPLICATION);
            }
        // Create an ALSA MIDI event
        snd_seq_event_t ev;
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);
        for(index = 0; index < MaxOutputPorts; index++) {
            snd_seq_ev_set_source(&ev, out_port[index]);
            // Send the data
            for(int i = 0; i < dataSize; i++) {
                snd_seq_ev_set_noteon(&ev, 0, midiData[i], midiData[++i]);  // Channel, note, velocity
                snd_seq_event_output(seq_handle, &ev);
                snd_seq_drain_output(seq_handle);
                }
            }
    #endif
    }

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