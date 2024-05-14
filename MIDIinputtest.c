// Test receiving MIDI messages in real time for 30 seconds

// The following settings will be displayed at the time the application starts.
// Choose the correct ones and recompile if necessary!
int MIDIsource = 0; // Your MIDI input device (keyboard, etc.)
int MIDIoutput = 1; // Your MIDI output device (synthesizer, etc.)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define platform-specific constants, prototypes, and include headers
#if defined(_WIN64)
    #include <windows.h>
    #include <mmsystem.h>
    #pragma comment(lib, "winmm.lib")
    void CALLBACK MyMIDIInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    // Global variables for MIDI device handle
    static HMIDIOUT hMidiOut = NULL;
    static HMIDIOUT hMidiIn = NULL;
#elif defined(__APPLE__)
    #include <unistd.h>
    #include <CoreMIDI/CoreMIDI.h>
    #include <mach/mach_time.h>
    MIDIClientRef midiOutputClient,midiInputClient;
    MIDIEndpointRef MIDIoutputdestination,MIDIinputdestination;
    MIDIPortRef MIDIoutPort,MIDIinPort;
    MIDIEndpointRef MIDIoutputination_output;
    static void MyMIDIReadProc(const MIDIPacketList*,void*,void*);
    void MIDIInputCallback(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon) {
        MyMIDIReadProc(pktlist, readProcRefCon, srcConnRefCon);
        }
    Boolean IsMIDIDestinationActive(MIDIEndpointRef endpoint) {
        SInt32 offline;
        OSStatus result;
        // Check if the endpoint is offline
        result = MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyOffline, &offline);
        if(result != noErr) {
            printf("Error getting MIDIObjectGetIntegerProperty.\n");
            return false;
            }
        // Return true if the device is online (offline == 0)
        return offline == 0;
        }
#elif defined(__linux__)
    #include <unistd.h> 
    #include <alsa/asoundlib.h>
    void MyAlsaMidiInProc(snd_seq_event_t *ev, void *refCon);
    void handle_midi_input(snd_seq_event_t *ev);
    // Global variable for ALSA MIDI sequencer handle
    static snd_seq_t *seq_handle = NULL;
    static int out_port,in_port;
#endif

// Function prototypes
int initializeMIDISystem(void);
void closeMIDISystem(void);
void sendMIDIEvent(const unsigned char*,int);
void handleMIDIMessage(const unsigned char*,int);

#define MAXMIDIMESSAGES 1000
// Try small values, e.g. '5', to check the handling of limit
#define NoteOff 128
#define NoteOn 144

typedef struct {
    unsigned long eventTime; // Time in ms
    int dataSize;
    unsigned char midiData[4];
} MIDIEvent;

MIDIEvent eventStack[MAXMIDIMESSAGES];
long eventCount = 0L;
long eventCountMax = MAXMIDIMESSAGES - 2L;

// Function to send MIDI message
void sendMIDIEvent(const unsigned char* midiData,int dataSize) {
    handleMIDIMessage(midiData,3); // Display NoteOns and NoteOffs
    #if defined(_WIN64)
        // Windows MIDI event sending
        // Ensure MIDI device is opened
        if(hMidiOut == NULL) {
            if (midiOutOpen(&hMidiOut, 0, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
                fprintf(stderr, "Error opening MIDI output.\n");
                return;
                }
            }
        // Pack the bytes into a DWORD message
        DWORD msg = 0;
        for(int i = 0; i < dataSize; i++) {
            msg |= (midiData[i] << (i * 8));
            }
        // Send the MIDI message
        midiOutShortMsg(hMidiOut, msg);
    #elif defined(__APPLE__)
        // MacOS MIDI event sending
        MIDIPacketList packetList;
        MIDIPacket *packet = MIDIPacketListInit(&packetList);
        packet = MIDIPacketListAdd(&packetList, sizeof(packetList), packet, mach_absolute_time(), dataSize, midiData);
        if(packet) MIDISend(MIDIoutPort, MIDIoutputdestination, &packetList);
    #elif defined(__linux__)
        // Ensure ALSA sequencer is setup
        if(seq_handle == NULL) {
            if(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
                fprintf(stderr, "Error opening ALSA sequencer.\n");
                return;
                }
            snd_seq_set_client_name(seq_handle, "MIDI Sender");
            out_port = snd_seq_create_simple_port(seq_handle, "Out",SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,SND_SEQ_PORT_TYPE_APPLICATION);
            }
        // Create an ALSA MIDI event
        snd_seq_event_t ev;
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_source(&ev, out_port);
        // Send the data
        for(int i = 0; i < dataSize; i++) {
            snd_seq_ev_set_noteon(&ev, 0, midiData[i], midiData[++i]);  // Channel, note, velocity
            snd_seq_event_output(seq_handle, &ev);
            snd_seq_drain_output(seq_handle);
            }
    #endif
    }

void handleMIDIMessage(const unsigned char* message,int length) {
    if(length < 3) return; // Basic validation for 3-byte messages like NoteOn/Off
    unsigned char statusByte = message[0];
    unsigned char messageType = statusByte & 0xF0; // Get the message type by masking with 0xF0
    if(messageType == 0x90 || messageType == 0x80) { // Check if it's NoteOn or NoteOff
        int channel = statusByte & 0x0F; // Get the channel
        int note = message[1]; // Note number
        int velocity = message[2]; // Velocity
        printf("Message Type: %s, Channel: %d, Note: %d, Velocity: %d\n",
               messageType == 0x90 ? "NoteOn" : "NoteOff",
               channel + 1, // Display channel in user-friendly format (1-16)
               note,
               velocity);
        }
    }

#if defined(__linux__)
void handle_midi_input(snd_seq_event_t *ev) {
    unsigned char midiData[3];
    switch (ev->type) {
        case SND_SEQ_EVENT_NOTEON:
            midiData[0] = 0x90 | (ev->data.note.channel & 0x0F);  // Note On command with channel
            midiData[1] = ev->data.note.note;  // Note value
            midiData[2] = ev->data.note.velocity;  // Velocity
            sendMIDIEvent(midiData,3);
            break;
        case SND_SEQ_EVENT_NOTEOFF:
            midiData[0] = 0x80 | (ev->data.note.channel & 0x0F);  // Note Off command with channel
            midiData[1] = ev->data.note.note;  // Note value
            midiData[2] = 0;  // Velocity is often 0 for Note Off
            sendMIDIEvent(midiData,3);
            break;
        case SND_SEQ_EVENT_CONTROLLER:
            midiData[0] = 0xB0 | (ev->data.control.channel & 0x0F);  // Controller command with channel
            midiData[1] = ev->data.control.param;  // Controller number
            midiData[2] = ev->data.control.value;  // Controller value
            sendMIDIEvent(midiData,3);
            break;
        // Add more cases here for different types of MIDI messages
        default:
            // Handle other types of messages or ignore them
            break;
    }  
#endif

#if defined(__APPLE__)
// Placeholder for MacOS MIDI input
static void MyMIDIReadProc(const MIDIPacketList* packetList, void* readProcRefCon, void* srcConnRefCon) {
    (void)readProcRefCon;  // Unused parameter. This is necessary in order to avoid warnings.
    (void)srcConnRefCon;  // Unused parameter
    const MIDIPacket* packet = &packetList->packet[0];
    if (packet == NULL) {
        printf("No packets received.\n");
        return;  // Early exit if packet is NULL
        }
 //   printf("packetList->numPackets = %d\n",(int)packetList->numPackets);
    for (unsigned int i = 0; i < packetList->numPackets; i++) {
        sendMIDIEvent(packet->data, packet->length);
        packet = MIDIPacketNext(packet);
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

int main() {
    int resultinit = initializeMIDISystem();
    if(resultinit != 0) return -1;
    printf("Waiting for MIDI events. Press Ctrl+C to exit.\n");
    #if defined(_WIN64)
    // Apparently there is nothing to do!
    #elif defined(__linux__)
    int npfd;
    struct pollfd *pfd;
    // Use polling to wait for MIDI events
    npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
    pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
    snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
    while(true) {
        if (poll(pfd, npfd, 100) > 0) { // Timeout after 100 ms
            snd_seq_event_t *event = NULL;
            while (snd_seq_event_input(seq_handle, &event) >= 0) {
                handle_midi_input(event);
                snd_seq_free_event(event);
            }
        }
    #elif defined(__APPLE__)
        CFRunLoopRun();  // Start the CoreMIDI run loop to process incoming MIDI messages.
    #endif
    closeMIDISystem();
    return 0;
    }

int initializeMIDISystem() {
    #if defined(_WIN64)
        printf("Initializing Windows MIDI system (WinMM)\n");
        // Get the number of MIDI out devices in the system
        UINT numDevs = midiOutGetNumDevs();
        if (numDevs == 0) {
            fprintf(stderr, "No MIDI output devices available.\n");
            return -1;
            }
        // Iterate through all available devices to list them
        MIDIOUTCAPS moc;
        for (UINT i = 0; i < numDevs; i++) {
            MMRESULT result = midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS));
            if (result != MMSYSERR_NOERROR) {
                fprintf(stderr, "Error retrieving MIDI device capabilities.\n");
                continue; // Skip to next device
                }
            printf("MIDI (output) %u: %s", i, moc.szPname);
            if((int)i == MIDIoutput) printf(" => MIDIoutput (your choice at the top of MIDIinputtest.c)");
            printf("\n");
            }
        // Open the default MIDI output device (or the first one found)
        UINT deviceId = 0; // Typically, 0 represents the default MIDI device
        // HMIDIOUT hMidiOut; is a global variable
        MMRESULT result = midiOutOpen(&hMidiOut, deviceId,0,0,CALLBACK_NULL);
        if (result != MMSYSERR_NOERROR) {
            fprintf(stderr, "Error opening MIDI output device.\n");
            return -1;
            }
        // Get the number of MIDI input devices in the system
        UINT numInDevs = midiInGetNumDevs();
        if (numInDevs == 0) {
            fprintf(stderr, "No MIDI input devices available.\n");
            return -1;
            }
        // Iterate through all available input devices to list them
        MIDIINCAPS mic;
        for (UINT i = 0; i < numInDevs; i++) {
            if (midiInGetDevCaps(i, &mic, sizeof(mic)) != MMSYSERR_NOERROR) {
                fprintf(stderr, "Error retrieving MIDI input device capabilities.\n");
                continue; // Skip to next device
                }
            printf("MIDI (source) %u: %s\n", i, mic.szPname);
            if((int)i == MIDIsource) printf(" => MIDIsource (your choice at the top of MIDIinputtest.c)");
            printf("\n");
            }
        // Open the default MIDI input device (or the first one found)
        if (midiInOpen(&hMidiIn, MIDIsource, (DWORD_PTR)midiInCallback, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
            fprintf(stderr, "Error opening MIDI input device.\n");
            return -1;
            }
        // Start the MIDI input processing
        if (midiInStart(hMidiIn) != MMSYSERR_NOERROR) {
            fprintf(stderr, "Error starting MIDI input.\n");
            return -1;
            }
        return(0);
    #elif defined(__APPLE__)
        OSStatus status;
        MIDIEndpointRef src;
        ItemCount i;
        CFStringRef endpointName;
        printf("Initializing MacOS MIDI system (CoreMIDI)\n");
        status = MIDIClientCreate(CFSTR("MIDIcheck output Client"),NULL,NULL,&midiOutputClient);
        if(status != noErr) {
            fprintf(stderr, "Could not create MIDI output client.\n");
            return -1;
            }
        status = MIDIOutputPortCreate(midiOutputClient, CFSTR("Output Port"),&MIDIoutPort);
        if(status != noErr) {
            fprintf(stderr, "Could not create output port.\n");
            return -1;
            }
        ItemCount MIDIoutputinationCount = MIDIGetNumberOfDestinations();
        if(MIDIoutputinationCount == 0) {
            fprintf(stderr, "No MIDI destinationinations available.\n");
            return -1;
            }
        if((int)MIDIoutputinationCount < MIDIoutput) {
            fprintf(stderr,"=> Error: MIDIoutput (%d) should be lower than %d\n",(int)MIDIoutput,(int)MIDIoutputinationCount);
            return -1;
            }
        for (i = 0; i < MIDIoutputinationCount; ++i) {
            MIDIoutputdestination = MIDIGetDestination(i);
            endpointName = NULL;
            if (MIDIObjectGetStringProperty(MIDIoutputdestination, kMIDIPropertyName, &endpointName) == noErr) {
                char name[64];
                CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                printf("MIDI (output) %lu: %s",i,name);
                if((int)i == MIDIoutput) printf(" => MIDIoutput (your choice at the top of MIDIinputtest.c)");
                printf("\n");
                CFRelease(endpointName);
                }
            if(!IsMIDIDestinationActive(MIDIoutputdestination))
                printf("This MIDI (output) %lu is inactive.\n",i);
            }
        MIDIoutputdestination = MIDIGetDestination(MIDIoutput);

        // Create MIDI input client and port
        status = MIDIClientCreate(CFSTR("MIDI Client"),NULL,NULL,&midiInputClient);
        if(status != noErr) {
            fprintf(stderr, "Could not create MIDI input client.\n");
            return -1;
            }
   /*   status = MIDIInputPortCreateWithProtocol(midiInputClient, CFSTR("Input Port"), kMIDIProtocol_1_0, &MIDIinPort,     MIDIInputCallback); would be better but too difficult to handle! */
        status = MIDIInputPortCreate(midiInputClient, CFSTR("Input Port"),MIDIInputCallback,NULL,&MIDIinPort);
        if (status != noErr) {
            fprintf(stderr, "Could not create input port with MIDI Protocol 1.0.\n");
            return -1;
            }
        // Connect first source to input port
        ItemCount sourceCount = MIDIGetNumberOfSources();
        if (sourceCount == 0) {
            fprintf(stderr, "No MIDI sources found.\n");
            return -1;
            }
        if((int)sourceCount < MIDIsource) {
            fprintf(stderr, "=> Error: MIDIsource (%d) should be lower than %d\n",(int)MIDIsource,(int)sourceCount);
            return -1;
            }
        for (i = 0; i < sourceCount; ++i) {
            MIDIinputdestination = MIDIGetSource(i);
            endpointName = NULL;
            if (MIDIObjectGetStringProperty(MIDIinputdestination, kMIDIPropertyName, &endpointName) == noErr) {
                char name[64];
                CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
                printf("MIDI (source) %lu: %s",i,name);
                if((int)i == MIDIsource) printf(" => MIDIsource (your choice at the top of MIDIinputtest.c)");
                printf("\n");
                CFRelease(endpointName);
                }
            if(!IsMIDIDestinationActive(MIDIinputdestination))
                printf("This MIDI (source) %lu is inactive.\n",i);
            }
        src = MIDIGetSource(MIDIsource);
        MIDIPortConnectSource(MIDIinPort,src,NULL);
        return(0);
    #elif defined(__linux__)
        printf("Initializing Linux MIDI system (ALSA)\n");
        // Open the ALSA sequencer
        if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
            fprintf(stderr, "Error opening ALSA sequencer.\n");
            return -1;
            }
        snd_seq_set_client_name(seq_handle, "My MIDI Application");
        // Create an output port
        out_port = snd_seq_create_simple_port(seq_handle, "Out Port",
                                            SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
                                            SND_SEQ_PORT_TYPE_APPLICATION);
        if (out_port < 0) {
            fprintf(stderr, "Error creating sequencer port.\n");
            return -1;
            }
        // Enumerate and list all clients and ports
        snd_seq_client_info_t *cinfo;
        snd_seq_port_info_t *pinfo;
        snd_seq_client_info_alloca(&cinfo);
        snd_seq_port_info_alloca(&pinfo);
        int first_client = -1, first_port = -1;
        snd_seq_client_info_set_client(cinfo, -1);

        while (snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
            int client = snd_seq_client_info_get_client(cinfo);
            snd_seq_port_info_set_client(pinfo, client);
            snd_seq_port_info_set_port(pinfo, -1);
            while (snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
                // Check if the port is an output port
                if ((snd_seq_port_info_get_capability(pinfo) & (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)) == (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)) {
                    printf("Output port found: Client %d, Port %d, Name: %s\n",
                        snd_seq_port_info_get_client(pinfo),
                        snd_seq_port_info_get_port(pinfo),
                        snd_seq_port_info_get_name(pinfo));
                    if (first_client == -1) { // Check if this is the first available port
                        first_client = client;
                        first_port = snd_seq_port_info_get_port(pinfo);
                        }
                    }
                }
            }
        // Connect to the first available output port if found
        if(first_client != -1 && first_port != -1) {
            if (snd_seq_connect_to(seq_handle, out_port, first_client, first_port) < 0) {
                fprintf(stderr, "Error connecting to MIDI port: Client %d, Port %d\n", first_client, first_port);
                return -1;
                }
            printf("Connected to Client %d, Port %d\n", first_client, first_port);
            }
        else {
            fprintf(stderr, "No available MIDI output ports found.\n");
            return -1;
            }
        // Create an input port
        in_port = snd_seq_create_simple_port(seq_handle, "Input Port",
                                            SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                            SND_SEQ_PORT_TYPE_APPLICATION);
        if(in_port < 0) {
            fprintf(stderr, "Error creating input sequencer port.\n");
            return -1;
            }
        // Connect all available sources to this input port
        snd_seq_client_info_alloca(&cinfo);
        snd_seq_port_info_alloca(&pinfo);
        snd_seq_client_info_set_client(cinfo, -1);
        while (snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
            int client = snd_seq_client_info_get_client(cinfo);
            snd_seq_port_info_set_client(pinfo, client);
            snd_seq_port_info_set_port(pinfo, -1);
            while (snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
                // Check if the port can be connected to our input port
                if ((snd_seq_port_info_get_capability(pinfo) & (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ)) == (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ)) {
                    int src_client = snd_seq_port_info_get_client(pinfo);
                    int src_port = snd_seq_port_info_get_port(pinfo);
                    // Connect source to our input port
                    if (snd_seq_connect_from(seq_handle, in_port, src_client, src_port) < 0) {
                        fprintf(stderr, "Error connecting from src_client %d, src_port %d\n", src_client, src_port);
                        }
                    else {
                        printf("Connected from Client %d, Port %d\n", src_client, src_port);
                        }
                    }
                }
            }
        return 0;
    #endif
    }

void closeMIDISystem() {
    #if defined(_WIN64)
        // Windows MIDI cleanup
        printf("Closing Windows MIDI system\n");
        if(hMidiOut != NULL) {
            midiOutClose(hMidiOut);  // Close the MIDI output device
            hMidiOut = NULL;         // Reset the handle to NULL after closing
            }
        if (hMidiIn != NULL) {
            midiInStop(hMidiIn);
            midiInClose(hMidiIn);
            hMidiIn = NULL;
            }
    #elif defined(__APPLE__)
        // MacOS MIDI cleanup
        printf("\nClosing MacOS MIDI system\n");
        MIDIPortDispose(MIDIoutPort);
        MIDIPortDispose(MIDIinPort);
        MIDIClientDispose(midiOutputClient);
        MIDIClientDispose(midiInputClient);
    #elif defined(__linux__)
        // Linux MIDI cleanup
        printf("Closing Linux MIDI system\n");
        if(seq_handle != NULL) {
            snd_seq_close(seq_handle);  // Close the ALSA sequencer
            seq_handle = NULL;          // Reset the handle to NULL after closing
            }
    #endif
    }

