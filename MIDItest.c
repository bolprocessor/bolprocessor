// Test sending MIDI messages in real time using an in-built event manager


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes
int initializeMIDISystem(void);
void sendMIDIEvent(unsigned char*, int);
void closeMIDISystem(void);
long getClockTime(void);
void MIDIflush(void);
long getClockTime(void);

// Define platform-specific constants and include headers
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
// Global variable for MIDI device handle
static HMIDIOUT hMidiOut = NULL;
typedef unsigned long long UInt64;
void usleep(DWORD waitTime) {
    LARGE_INTEGER perfCnt, start, now;
    QueryPerformanceFrequency(&perfCnt);
    QueryPerformanceCounter(&start);
    do {
        QueryPerformanceCounter((LARGE_INTEGER*)&now);
    } while ((now.QuadPart - start.QuadPart) / float(perfCnt.QuadPart) * 1000 * 1000 < waitTime);
}

#elif defined(__APPLE__)
#include <unistd.h>
#include <CoreMIDI/CoreMIDI.h>
#include <mach/mach_time.h>
MIDIClientRef midiClient;
MIDIPortRef MIDIoutPort;
MIDIEndpointRef MIDIdestination;
#elif defined(__linux__)
#include <unistd.h> 
#include <alsa/asoundlib.h>
// Global variable for ALSA MIDI sequencer handle
static snd_seq_t* seq_handle = NULL;
static int out_port;
#endif

#define MAXMIDIMESSAGES 1000
// Try small values, e.g. '5', to check the handling of limit

typedef struct {
    UInt64 eventTime; // Time in ms
    int dataSize;
    unsigned char midiData[3];
} MIDI_Event;

MIDI_Event eventStack[MAXMIDIMESSAGES];
long eventCount = 0L;
long eventCountMax = MAXMIDIMESSAGES - 2L;

UInt64 initTime;

// Function to send MIDI message
void sendMIDIEvent(unsigned char* midiData, int dataSize) {
#if defined(_WIN32) || defined(_WIN64)
    // Windows MIDI event sending
    // Ensure MIDI device is opened
    if (hMidiOut == NULL) {
        if (midiOutOpen(&hMidiOut, 0, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
            fprintf(stderr, "Error opening MIDI output.\n");
            return;
        }
    }
    // Pack the bytes into a DWORD message
    DWORD msg = 0;
    for (int i = 0; i < dataSize; i++) {
        msg |= (midiData[i] << (i * 8));
    }
    // Send the MIDI message
    midiOutShortMsg(hMidiOut, msg);
#elif defined(__APPLE__)
    // MacOS MIDI event sending
    MIDIPacketList packetList;
    MIDIPacket* packet = MIDIPacketListInit(&packetList);
    packet = MIDIPacketListAdd(&packetList, sizeof(packetList), packet, mach_absolute_time(), dataSize, midiData);
    if (packet) MIDISend(MIDIoutPort, MIDIdestination, &packetList);
#elif defined(__linux__)
    // Ensure ALSA sequencer is setup
    if (seq_handle == NULL) {
        if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
            fprintf(stderr, "Error opening ALSA sequencer.\n");
            return;
        }
        snd_seq_set_client_name(seq_handle, "MIDI Sender");
        out_port = snd_seq_create_simple_port(seq_handle, "Out", SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_APPLICATION);
    }
    // Create an ALSA MIDI event
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);
    snd_seq_ev_set_source(&ev, out_port);
    // Send the data
    for (int i = 0; i < dataSize; i++) {
        snd_seq_ev_set_noteon(&ev, 0, midiData[i], midiData[++i]);  // Channel, note, velocity
        snd_seq_event_output(seq_handle, &ev);
        snd_seq_drain_output(seq_handle);
    }
#endif
}

// Function to process and flush MIDI events
void MIDIflush() {
    UInt64 currentTime = getClockTime();
    currentTime -= initTime;
    long i = 0;
    while (i < eventCount) {
        if (eventStack[i].eventTime <= currentTime) {
            fprintf(stderr, "Sending note #%d time = %ld ms, currentTime = %ld microseconds\n", eventStack[i].midiData[1], (long)eventStack[i].eventTime / 1000L, (long)currentTime);
            sendMIDIEvent(eventStack[i].midiData, eventStack[i].dataSize);
            // Move remaining events forward
            memmove(&eventStack[i], &eventStack[i + 1], (eventCount - i - 1) * sizeof(MIDI_Event));
            eventCount--;
        }
        else i++;
    }
}

// Read events from file and store them in the stack
void loadEventsFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file.\n");
        exit(EXIT_FAILURE);
    }
    char type[4];
    int key, time, done;
    while (fscanf(file, "%s %d %d", type, &key, &time) == 3) {
        done = 0;
        while (eventCount > eventCountMax) {
            //    fprintf(stderr,"Event buffer overflow.\n");
            usleep(1000); // Sleep for 1 millisecond
            MIDIflush();
            if (done++ == 0) fprintf(stderr, "Reached the limit of the buffer...\n");
        }
        eventStack[eventCount].eventTime = (UInt64)1000 * time;
        eventStack[eventCount].midiData[0] = (strcmp(type, "on") == 0) ? 0x90 : 0x80;
        eventStack[eventCount].midiData[1] = (unsigned char)key;
        eventStack[eventCount].midiData[2] = 80;  // Assuming velocity 80 for NoteOn; ignored for NoteOff
        eventStack[eventCount].dataSize = 3;
        eventCount++;
        MIDIflush();
    }
    fclose(file);
}

int main() {
    int resultinit;
    resultinit = initializeMIDISystem();
    if (resultinit != 0) return -1;
    initTime = getClockTime();
    fprintf(stderr, "eventCountMax = %ld\n", (long)eventCountMax);
    loadEventsFromFile("MIDItest-list.txt");  // Load MIDI events from file
    fprintf(stderr, "Remaining eventCount after loading file = %ld\n", (long)eventCount);
    while (eventCount > 0L) {
        MIDIflush();  // Process MIDI events
        usleep(1000); // Sleep for 1 millisecond
    }
    closeMIDISystem();
    return 0;
}

long getClockTime(void) {
    long the_time;
#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq); // Get the frequency of the high-resolution performance counter
    QueryPerformanceCounter(&count);  // Get the current value of the performance counter
    the_time = (unsigned long)((count.QuadPart * 1000000) / freq.QuadPart);
#elif defined(__APPLE__)
    the_time = clock_gettime_nsec_np(CLOCK_UPTIME_RAW) / 1000L;
#elif defined(__linux__)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts); // CLOCK_MONOTONIC provides uptime, not affected by system time changes
    the_time = (unsigned long)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
#endif
    return the_time;
}

int initializeMIDISystem() {
#if defined(_WIN32) || defined(_WIN64)
    UINT deviceId = 0; // Typically, 0 represents the default MIDI device
    MMRESULT result = midiOutOpen(&hMidiOut, deviceId, 0, 0, CALLBACK_NULL);
    if (result != MMSYSERR_NOERROR) {
        fprintf(stderr, "Error opening MIDI output device.\n");
        return -1;
    }
    else return(0);
#elif defined(__APPLE__)
    OSStatus status;
    printf("Initializing MacOS MIDI system (CoreMIDI)\n");
    status = MIDIClientCreate(CFSTR("MIDIcheck Client"), NULL, NULL, &midiClient);
    if (status != noErr) {
        fprintf(stderr, "Could not create MIDI client.\n");
        return -1;
    }
    status = MIDIOutputPortCreate(midiClient, CFSTR("Output Port"), &MIDIoutPort);
    if (status != noErr) {
        fprintf(stderr, "Could not create output port.\n");
        return -1;
    }
    ItemCount MIDIdestinationCount = MIDIGetNumberOfDestinations();
    if (MIDIdestinationCount == 0) {
        fprintf(stderr, "No MIDI destinationinations available.\n");
        return -1;
    }
    for (ItemCount i = 0; i < MIDIdestinationCount; ++i) {
        MIDIEndpointRef MIDIdestination = MIDIGetDestination(i);
        CFStringRef endpointName = NULL;
        if (MIDIObjectGetStringProperty(MIDIdestination, kMIDIPropertyName, &endpointName) == noErr) {
            char name[64];
            CFStringGetCString(endpointName, name, sizeof(name), kCFStringEncodingUTF8);
            printf("MIDIdestinationination %lu: %s\n", i + 1, name);
            CFRelease(endpointName);
        }
    }
    // Assuming the first destination
    MIDIdestination = MIDIGetDestination(0);
    return(0);
#elif defined(__linux__)
    printf("Initializing Linux MIDI system\n");
    if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
        fprintf(stderr, "Error opening ALSA sequencer.\n");
        return -1;
    }
    snd_seq_set_client_name(seq_handle, "My MIDI Application");
    out_port = snd_seq_create_simple_port(seq_handle, "Out Port", SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_APPLICATION);
    if (out_port < 0) {
        fprintf(stderr, "Error creating sequencer port.\n");
        return -1;
    }
    else return(0);
#endif
}

void closeMIDISystem() {
#if defined(_WIN32) || defined(_WIN64)
    // Windows MIDI cleanup
    printf("Closing Windows MIDI system\n");
    if (hMidiOut != NULL) {
        midiOutClose(hMidiOut);  // Close the MIDI output device
        hMidiOut = NULL;         // Reset the handle to NULL after closing
    }
#elif defined(__APPLE__)
    // MacOS MIDI cleanup
    MIDIPortDispose(MIDIoutPort);
    MIDIClientDispose(midiClient);
#elif defined(__linux__)
    // Linux MIDI cleanup
    printf("Closing Linux MIDI system\n");
    if (seq_handle != NULL) {
        snd_seq_close(seq_handle);  // Close the ALSA sequencer
        seq_handle = NULL;          // Reset the handle to NULL after closing
    }
#endif
}