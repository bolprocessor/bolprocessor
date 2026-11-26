// CsoundRealTime.c (BP3)

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

// Real-time Csound (2026-11-16)
// With option "--csoundrt port", the lines of the Csound score are sent to a Csound server listening to UDP port 'port' on this computer,
// started for instance with: csound -odac --port=10789 --orc 0-default.orc
// The score is built as for a file (CscoreWrite(), MakeCsoundFunctionTable()...). BPPrintMessage() passes its text to CsRTText() instead of a file.
// Each "i" or "f" line is queued with its date, then sent CSRT_LOOKAHEAD seconds before this date as a real-time score event: "$i1 0.150 ..."
// Csound starts it at the time given in p2, counted from the moment it receives the message, which places notes to within a control cycle.
// Other score statements ("t", "s", "e", comments) are not sent: dates are in seconds since tempo is always 60 in BP scores.
// CsRTFlush() is called at the end of each item by MakeSound(): it waits until all lines of the item have been sent,
// so that the next item is computed while the last notes are sounding.
// Stop, panic and pause turn off the notes of the instruments used, including the ones already sent and not yet started.

// Real-time SuperCollider (2026-11-20)
// With option "--scrt port[@host]", CsoundRT is TRUE as well and the score is built in the same way, with pitches in Hz (see CscoreWrite()).
// Each "i" line is sent to a SuperCollider server (scsynth) as an OSC message "/s_new" in a bundle whose time tag is the date of the note,
// so that scsynth starts it at the exact sample. Instrument n is played by SynthDef "bp<n>", with named arguments (see SCRTNoteMessage()).
// Notes are created in group SCRT_GROUP. Their SynthDefs should end them after "sustain" seconds, and release them when "gate" is set to 0.
// "f" lines are not sent: SuperCollider has no function tables. Notes of duration 0 are not sent either.
// Stop, panic and pause clear the bundles scheduled by the server (/clearSched) and release the notes of the group.

// Real-time OSC (2026-11-21)
// With option "--osc port[@host]", CsoundRT is TRUE as well, and notes are sent as generic OSC messages to a program such as Max or Pd
// listening to this UDP port. Unlike SuperCollider, these programs act on a message as soon as it arrives, ignoring time tags:
// messages are sent at the date of each event, not in advance. The messages (see OSCNoteMessage()):
//    /bp/start                                     at the start of a performance
//    /bp/note id instrument name key freq duration velocity channel [name value]...   at the start of a note
//    /bp/off id instrument name key channel         at its end (not for notes of duration 0)
//    /bp/stop                                       when the performance is stopped or paused, or for panic: silence all notes
//    /bp/end                                        at the end of a performance
// 'id' identifies a note among those playing; 'name' is that of the instrument in the '-sc' or '-cs' file, or "bp<n>";
// 'freq' is in Hz (scales included), 'duration' in seconds, 'velocity' 0-127 and 'channel' 1-16 are those of the MIDI note,
// and the named values are those sent to SuperCollider SynthDefs (vol, vol2, vel, pan, pan2, mod, mod2, bend, bend2, parameters...).

// Option "--rt-dump file" (2026-11-22)
// With --csoundrt, --scrt or --osc, messages are written to 'file' as text instead of being sent, one per line preceded by its date
// in the score (seconds): "$i1 ..." lines for Csound, decoded OSC messages for SuperCollider and OSC (bundles without their time tags,
// which depend on the clock). Nothing waits for real time, scripts (waits, holds) are not executed, and the number of items is limited
// as for files. This lets run-ctests.sh compare the outputs of two builds without a server.

// Comments "// note N" in this file refer to the numbered notes of the technical description of this algorithm: https://bolprocessor.org/tech-analysis-of-real-time-csound/

#include "-BP3.h"
#include "-BP3decl.h"
#include <stdint.h> // uint32_t (UInt32 is a MacOS type, unknown on Windows)

#if defined(_WIN64)
	#include <winsock2.h>
	#include <ws2tcpip.h>
	typedef SOCKET CsRTsocket;
	#define CSRT_NOSOCKET INVALID_SOCKET
	#define CsRTclosesocket closesocket
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <netdb.h>
	#include <time.h>
	typedef int CsRTsocket;
	#define CSRT_NOSOCKET (-1)
	#define CsRTclosesocket close
#endif

// note 9
#define CSRT_LOOKAHEAD 0.15		// seconds: lines are sent this time before their date
#define CSRT_LATENCY 0.3		// seconds: the performance starts this time after the first item has been computed
#define CSRT_MAXMESSAGE 65000	// bytes: maximum size of a UDP message
#define CSRT_MAXINSTR 512		// maximum number of instruments turned off by CsRTStopAll()
#define SCRT_GROUP 2100000000	// node ID of the group of notes created in the SuperCollider server, above the IDs allocated by its clients
								// SCRT_GROUP + 1 is the node of the effects (SynthDef "bp_fx")
#define SCRT_MAXARGS 64			// maximum number of named arguments of a note sent to SuperCollider
#define SCRT_ENVSEGMENTS 16		// 2026-11-21 maximum number of segments of an envelope sent to SuperCollider (array argument of 4 + 4 * 16 values)
#define SCRT_MAXENVS 16			// maximum number of envelopes of a note

typedef struct {
	double time;	// date (seconds) in the score
	double target;	// date (seconds since CsRTOrigin) at which it was sent to start, only for lines in CsRTSent
	char* head;		// e.g. "i1" or "f101"
	char* rest;		// the fields after p2, starting with a space or empty
	unsigned char* osc;	// real-time SuperCollider: the OSC message, without its time tag
	int osclength;
	} CsRTEvent;

typedef struct {
	unsigned char* data;
	int length, max, ok;
	} OSCBuffer;

static CsRTsocket CsRTSocket = CSRT_NOSOCKET;
static struct sockaddr_in CsRTAddress;
static CsRTEvent* CsRTQueue = NULL;	// lines waiting to be sent, sorted by date
static long CsRTQueueSize = 0L, CsRTQueueMax = 0L;
static CsRTEvent* CsRTSent = NULL;	// lines sent and not yet started, sent again after a pause
static long CsRTSentSize = 0L, CsRTSentMax = 0L;
static char* CsRTPending = NULL;	// text received by CsRTText() until the end of the line
static size_t CsRTPendingLength = 0, CsRTPendingMax = 0;
static UInt64 CsRTOrigin = 0;		// clock time (microseconds) at which date 0 of the score is played
static int CsRTStarted = FALSE;
static double CsRTLastEnd = 0.;		// end (seconds) of the last note in the score
static double CsRTInstruments[CSRT_MAXINSTR];
static int CsRTNumberInstruments = 0;
static int CsRTSaidError = 0;
static UInt64 SCRTClockToNTP = 0;	// real-time SuperCollider: NTP time (32.32 fixed point) minus getClockTime() in the same unit
static long OSCNoteId = 0;			// real-time OSC: number of the last note
typedef struct {			// 2026-11-21 real-time SuperCollider: a function table of the score (GEN 7 or 5), sent as envelopes
	int number;			// table number, e.g. 101 for "f101"
	int points;			// number of values
	double* value;		// values
	double* length;		// length[k] is the length of the segment from value[k] to value[k + 1]
	} SCRTTable;

typedef struct {		// an envelope argument of /s_new, e.g. "slideenv"
	char name[20];
	int points;
	double level[SCRT_ENVSEGMENTS + 1];
	double time[SCRT_ENVSEGMENTS];	// fractions of the duration of the note
	} SCRTEnvelope;

static SCRTTable* SCRTTables = NULL;
static int SCRTNumberTables = 0, SCRTMaxTables = 0;
static FILE* CsRTDump = NULL;		// option "--rt-dump": file to which messages are written
static double CsRTDumpDate = 0.;	// date written before the next message
static int OSCInfoKey = -1, OSCInfoChan = 0, OSCInfoVelocity = 0; // real-time OSC: MIDI key, channel and velocity of the next note line, see CsRTNoteInfo()

typedef struct {
	double time;	// date (seconds) in the score
	int key, chan, instance;
	int velocity;	// 0 for the silent notes of CsRTMidiNote(), the velocity of a tick of the time base (CsRTMidiTick())
	} CsRTMidiEvent;

static CsRTMidiEvent* CsRTMidiQueue = NULL;	// silent MIDI notes waiting for their date, sorted by date (note 22)
static long CsRTMidiSize = 0L, CsRTMidiMax = 0L;

static int CsRTSend(const char* message);
static void CsRTLine(char* line);
static int CsRTPush(CsRTEvent** p_list,long* p_size,long* p_max,CsRTEvent* p_e,int sorted);
static void CsRTFreeEvent(CsRTEvent* p_e);
static void CsRTSleep(long milliseconds);
static double CsRTBarrier(void);
static int CsRTScripts(void);
static void CsRTSendMidiDue(int final);
static void CsRTMidiPush(CsRTMidiEvent* p_e);
static void CsRTDumpMidiUntil(double date);
static int CsRTScriptsDue(unsigned long date);
static int CsRTWaitInstruction(int instr);
static const char* CsRTName(void);
static int SCRTSend(OSCBuffer* b);
static void SCRTCalibrate(void);
static int SCRTInstrumentIndex(int instrument);
static void SCRTNoteMessage(OSCBuffer* b,int instrument,double duration,const char* fields);
static void OSCNoteMessage(OSCBuffer* b,OSCBuffer* off,int instrument,double duration,const char* fields);
static void OSCSendAddress(const char* address);
static void SCRTSendBundle(unsigned char* message,int length,UInt64 clock);
static void SCRTStartGroup(void);
static void OSCInit(OSCBuffer* b);
static void OSCFree(OSCBuffer* b);
static void OSCInt32(OSCBuffer* b,uint32_t x);
static void OSCFloat(OSCBuffer* b,double x);
static void OSCString(OSCBuffer* b,const char* s);
static void OSCDump(const unsigned char* data,int length);

static int CsRTOn(void) {
	// TRUE if messages are sent to a socket or written to the file of option "--rt-dump"
	return (CsRTSocket != CSRT_NOSOCKET || CsRTDump != NULL);
	}


// note 2
int CsRTOpen(void) {
	// Called by ParsePostInitArgs() for option "--csoundrt port"
	if(CsRTOn()) return OK;
	if(CsRTDumpName != NULL) {
		// Option "--rt-dump": no socket, messages are written to a file
		CsRTDump = fopen(CsRTDumpName,"w");
		if(CsRTDump == NULL) {
			BPPrintMessage(0,odError,"=> Could not create file %s for option --rt-dump\n",CsRTDumpName);
			return ABORT;
			}
		BPPrintMessage(0,odInfo,"Real-time %s: writing messages to %s, without waiting for real time\n",CsRTName(),CsRTDumpName);
		goto START;
		}
	if(CsoundRTport < 1 || CsoundRTport > 65535) {
		BPPrintMessage(0,odError,"=> Incorrect UDP port %d for real-time Csound\n",CsoundRTport);
		return ABORT;
		}
#if defined(_WIN64)
	WSADATA wsadata;
	if(WSAStartup(MAKEWORD(2,2),&wsadata) != 0) {
		BPPrintMessage(0,odError,"=> Could not start Windows sockets for real-time Csound\n");
		return ABORT;
		}
#endif
	CsRTSocket = socket(AF_INET,SOCK_DGRAM,0);
	if(CsRTSocket == CSRT_NOSOCKET) {
		BPPrintMessage(0,odError,"=> Could not create a UDP socket for real-time Csound\n");
		return ABORT;
		}
	// note 18
	{
		// The default size limits messages to 9216 bytes on MacOS, too few for long function tables
		int size = 65536;
		setsockopt(CsRTSocket,SOL_SOCKET,SO_SNDBUF,(const char*)&size,sizeof(size));
		}
	memset(&CsRTAddress,0,sizeof(CsRTAddress));
	CsRTAddress.sin_family = AF_INET;
	CsRTAddress.sin_port = htons((unsigned short) CsoundRTport);
	CsRTAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
	if((SuperColliderRT || OSCRT) && strcmp(SuperColliderHost,"127.0.0.1") != 0) {
		struct addrinfo hints, *result;
		memset(&hints,0,sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		if(getaddrinfo(SuperColliderHost,NULL,&hints,&result) != 0 || result == NULL) {
			BPPrintMessage(0,odError,"=> Unknown host \"%s\" for real-time %s\n",SuperColliderHost,CsRTName());
			CsRTclosesocket(CsRTSocket);
			CsRTSocket = CSRT_NOSOCKET;
			return ABORT;
			}
		CsRTAddress.sin_addr = ((struct sockaddr_in*) result->ai_addr)->sin_addr;
		freeaddrinfo(result);
		}
START:
	CsRTQueueSize = CsRTSentSize = CsRTMidiSize = 0L;
	CsRTPendingLength = 0;
	CsRTStarted = FALSE;
	CsRTLastEnd = 0.;
	CsRTNumberInstruments = 0;
	CsRTSaidError = 0;
	CsRTDumpDate = 0.;
	if(CsRTDump != NULL) {} // Message given above
	else if(SuperColliderRT)
		BPPrintMessage(0,odInfo,"Real-time SuperCollider: sending notes to UDP port %d of %s\n",CsoundRTport,SuperColliderHost);
	else if(OSCRT)
		BPPrintMessage(0,odInfo,"Real-time OSC: sending notes to UDP port %d of %s\n",CsoundRTport,SuperColliderHost);
	else BPPrintMessage(0,odInfo,"Real-time Csound: sending score lines to UDP port %d of this computer\n",CsoundRTport);
	OSCNoteId = 0;
	return OK;
	}


// note 16
void CsRTClose(void) {
	long i;
	if(CsRTSocket != CSRT_NOSOCKET) {
		CsRTclosesocket(CsRTSocket);
		CsRTSocket = CSRT_NOSOCKET;
#if defined(_WIN64)
		WSACleanup();
#endif
		}
	if(CsRTDump != NULL) {
		fclose(CsRTDump);
		CsRTDump = NULL;
		}
	for(i = 0; i < CsRTQueueSize; i++) CsRTFreeEvent(&CsRTQueue[i]);
	for(i = 0; i < CsRTSentSize; i++) CsRTFreeEvent(&CsRTSent[i]);
	free(CsRTQueue); CsRTQueue = NULL;
	free(CsRTSent); CsRTSent = NULL;
	free(CsRTPending); CsRTPending = NULL;
	free(CsRTMidiQueue); CsRTMidiQueue = NULL;
	CsRTMidiSize = CsRTMidiMax = 0L;
	for(i = 0; i < SCRTNumberTables; i++) {
		free(SCRTTables[i].value);
		free(SCRTTables[i].length);
		}
	free(SCRTTables); SCRTTables = NULL;
	SCRTNumberTables = SCRTMaxTables = 0;
	CsRTQueueSize = CsRTQueueMax = CsRTSentSize = CsRTSentMax = 0L;
	CsRTPendingLength = CsRTPendingMax = 0;
	return;
	}


// note 4
void CsRTText(const char* format,va_list args) {
	// Called by BPPrintMessage() for destination odCsScore. Text is stored until the end of a line, then the line is processed.
	va_list args2;
	int length;
	size_t needed;
	char *start, *end;
	if(!CsRTOn()) return;
	va_copy(args2,args);
	length = vsnprintf(NULL,0,format,args2);
	va_end(args2);
	if(length <= 0) return;
	needed = CsRTPendingLength + (size_t) length + 1;
	if(needed > CsRTPendingMax) {
		char* p = (char*) realloc(CsRTPending,needed + 1024);
		if(p == NULL) {
			BPPrintMessage(0,odError,"=> Out of memory in CsRTText()\n");
			return;
			}
		CsRTPending = p;
		CsRTPendingMax = needed + 1024;
		}
	vsnprintf(CsRTPending + CsRTPendingLength,(size_t) length + 1,format,args);
	CsRTPendingLength += (size_t) length;
	start = CsRTPending;
	while((end = strchr(start,'\n')) != NULL) {
		*end = '\0';
		CsRTLine(start);
		start = end + 1;
		}
	CsRTPendingLength = strlen(start);
	memmove(CsRTPending,start,CsRTPendingLength + 1);
	return;
	}


// note 5
static void SCRTStoreTable(int number,const char* fields);
static void OSCPushEnvelopes(int instrument,double time,double duration,const char* fields);

static void CsRTLine(char* line) {
	// Queue an "i" or "f" line with its date
	char *p, *q, *end, type;
	double time, duration, instrument;
	size_t length;
	int i;
	CsRTEvent e;

	if((p = strchr(line,';')) != NULL) *p = '\0'; // Remove the comment
	while(*line == ' ' || *line == '\t') line++;
	length = strlen(line);
	while(length > 0 && isspace((unsigned char) line[length - 1])) line[--length] = '\0';
	type = line[0];
	if(type != 'i' && type != 'f') return; // "t", "s", "e"... are not sent
	p = line + 1;
	while(*p == ' ' || *p == '\t') p++; // The form "i 1 ..." is accepted
	q = p;
	while(*q != '\0' && !isspace((unsigned char) *q)) q++;
	if(q == p) return;
	instrument = atof(p);
	time = strtod(q,&end); // p2
	if(end == q) return;
	duration = strtod(end,&p); // p3
	if(p == end) duration = 0.;
	if((SuperColliderRT || OSCRT) && type == 'f') { // 2026-11-21 Function tables are not sent: they are kept and sent as envelopes with the notes using them (2026-11-23 also in real-time OSC, see OSCPushEnvelopes())
		SCRTStoreTable((int) floor(instrument),q); // The fields after p1
		return;
		}
	if(SuperColliderRT && duration <= 0.) return; // Notes of duration 0 are only synchronisation signals
	if(OSCRT && type != 'i') return; // No function tables
	e.time = time;
	e.target = 0.;
	e.osc = NULL;
	e.osclength = 0;
	e.head = (char*) malloc((size_t)(q - line) + 1);
	e.rest = (char*) malloc(strlen(end) + 1);
	if(e.head == NULL || e.rest == NULL) {
		free(e.head); free(e.rest);
		BPPrintMessage(0,odError,"=> Out of memory in CsRTLine()\n");
		return;
		}
	// The head is the type followed by p1, without spaces
	e.head[0] = type;
	memcpy(e.head + 1,line + 1,(size_t)(q - line) - 1);
	e.head[q - line] = '\0';
	for(p = e.head + 1; *p != '\0'; p++) {
		if(*p == ' ' || *p == '\t') memmove(p,p + 1,strlen(p));
		}
	strcpy(e.rest,end);
	if(OSCRT) {
		// The note, and its end as another event
		OSCBuffer b, off;
		CsRTEvent e2;
		OSCInit(&b);
		OSCInit(&off);
		OSCNoteMessage(&b,&off,(int) floor(instrument),duration,end);
		if(!b.ok || !off.ok) {
			OSCFree(&b);
			OSCFree(&off);
			CsRTFreeEvent(&e);
			BPPrintMessage(0,odError,"=> Out of memory in CsRTLine()\n");
			return;
			}
		e.osc = b.data;
		e.osclength = b.length;
		if(duration > 0.) {
			e2.time = time + duration;
			e2.target = 0.;
			e2.head = (char*) malloc(8);
			e2.rest = (char*) malloc(8);
			if(e2.head != NULL && e2.rest != NULL) {
				strcpy(e2.head,"off");
				strcpy(e2.rest,"");
				e2.osc = off.data;
				e2.osclength = off.length;
				off.data = NULL;
				if(CsRTPush(&CsRTQueue,&CsRTQueueSize,&CsRTQueueMax,&e2,TRUE) != OK) CsRTFreeEvent(&e2);
				}
			else {
				free(e2.head); free(e2.rest);
				}
			}
		OSCFree(&off);
		}
	else if(SuperColliderRT) {
		OSCBuffer b;
		OSCInit(&b);
		SCRTNoteMessage(&b,(int) floor(instrument),duration,end);
		if(!b.ok) {
			OSCFree(&b);
			CsRTFreeEvent(&e);
			BPPrintMessage(0,odError,"=> Out of memory in CsRTLine()\n");
			return;
			}
		e.osc = b.data;
		e.osclength = b.length;
		}
	if(CsRTPush(&CsRTQueue,&CsRTQueueSize,&CsRTQueueMax,&e,TRUE) != OK) {
		CsRTFreeEvent(&e);
		return;
		}
	if(OSCRT && duration > 0.) OSCPushEnvelopes((int) floor(instrument),time,duration,end); // After the note, at the same date
	if(type == 'i') {
		if(duration > 0. && (time + duration) > CsRTLastEnd) CsRTLastEnd = time + duration;
		if(instrument > 0.) {
			instrument = floor(instrument);
			for(i = 0; i < CsRTNumberInstruments; i++)
				if(CsRTInstruments[i] == instrument) break;
			if(i == CsRTNumberInstruments && CsRTNumberInstruments < CSRT_MAXINSTR)
				CsRTInstruments[CsRTNumberInstruments++] = instrument;
			}
		}
	return;
	}


// note 6
static int CsRTPush(CsRTEvent** p_list,long* p_size,long* p_max,CsRTEvent* p_e,int sorted) {
	// Add an event to a list. If 'sorted', it is inserted after the events with the same date or an earlier one.
	// The order of lines with the same date is kept: a function table is written before the note that uses it.
	long i;
	if(*p_size >= *p_max) {
		long newmax = (*p_max < 256L) ? 256L : 2L * (*p_max);
		CsRTEvent* p = (CsRTEvent*) realloc(*p_list,(size_t) newmax * sizeof(CsRTEvent));
		if(p == NULL) {
			BPPrintMessage(0,odError,"=> Out of memory in CsRTPush()\n");
			return ABORT;
			}
		*p_list = p;
		*p_max = newmax;
		}
	i = *p_size;
	if(sorted) {
		while(i > 0 && (*p_list)[i - 1].time > p_e->time) i--;
		memmove(&(*p_list)[i + 1],&(*p_list)[i],(size_t)(*p_size - i) * sizeof(CsRTEvent));
		}
	(*p_list)[i] = *p_e;
	(*p_size)++;
	return OK;
	}


// note 17
static void CsRTSleep(long milliseconds) {
	// Unlike mysleep(), which is a busy loop on Windows, this lets the processor rest during the performance
#if defined(_WIN64)
	Sleep((DWORD) milliseconds);
#else
	usleep((useconds_t)(milliseconds * 1000L));
#endif
	return;
	}


static void CsRTFreeEvent(CsRTEvent* p_e) {
	free(p_e->head); p_e->head = NULL;
	free(p_e->rest); p_e->rest = NULL;
	free(p_e->osc); p_e->osc = NULL;
	return;
	}


// note 9
static int CsRTSend(const char* message) {
	size_t length;
	length = strlen(message);
	// note 18
	if(length > CSRT_MAXMESSAGE) {
		if(CsRTSaidError++ < 5)
			BPPrintMessage(0,odError,"=> A line of %ld bytes is too long to be sent to %s: %.40s...\n",(long) length,CsRTName(),message);
		return MISSED;
		}
	if(CsRTDump != NULL) {
		fprintf(CsRTDump,"%.4f %s\n",CsRTDumpDate,message);
		return OK;
		}
	if(sendto(CsRTSocket,message,(int) length,0,(struct sockaddr*) &CsRTAddress,sizeof(CsRTAddress)) < 0) {
		if(CsRTSaidError++ < 5)
			BPPrintMessage(0,odError,"=> Could not send a message to Csound on UDP port %d\n",CsoundRTport);
		return MISSED;
		}
	if(CsoundTrace) BPPrintMessage(0,odInfo,"Csound <- %.200s\n",message);
	return OK;
	}


static UInt64 CsRTDateToClock(double date) {
	// Clock time (microseconds) of a date (seconds) relative to CsRTOrigin, which may be negative
	if(date >= 0.) return CsRTOrigin + (UInt64)(date * 1000000.);
	if((UInt64)(-date * 1000000.) > CsRTOrigin) return 0;
	return CsRTOrigin - (UInt64)(-date * 1000000.);
	}


// note 9
static int CsRTSendDue(void) {
	// Send the lines whose date is less than CSRT_LOOKAHEAD seconds away
	UInt64 now;
	double delay, barrier;
	long i, j, sent;
	char* message;
	size_t length;
	CsRTEvent e;

	now = getClockTime();
	// Forget lines that have started
	for(i = j = 0; i < CsRTSentSize; i++) {
		if(CsRTDateToClock(CsRTSent[i].target) <= now) CsRTFreeEvent(&CsRTSent[i]);
		else CsRTSent[j++] = CsRTSent[i];
		}
	CsRTSentSize = j;
	barrier = (CsRTDump != NULL) ? -1. : CsRTBarrier(); // Scripts are not executed with --rt-dump
	sent = 0L;
	while(sent < CsRTQueueSize) {
		e = CsRTQueue[sent];
		if(CsRTDump != NULL) {
			// Option "--rt-dump": every line is written at once, with its date
			CsRTDumpMidiUntil(e.time); // Ticks of the time base dated before this line
			CsRTDumpDate = e.time;
			now = CsRTDateToClock(e.time);
			}
		// note 20
		// Lines dated at or after a pending wait or hold are kept until it is over (half a millisecond is tolerated for rounding)
		if(barrier >= 0. && e.time >= barrier - 0.0005) break;
		delay = ((double) CsRTDateToClock(e.time) - (double) now) / 1000000.;
		if(delay > (OSCRT ? 0. : CSRT_LOOKAHEAD)) break; // Real-time OSC: at the date of the event
		// note 10
		if(OSCRT && delay < 0. && delay > -0.05) delay = 0.; // A few milliseconds late since the loop checks the queue every 2 ms
		if(delay < 0.) {
			// Late, e.g. because computing the item took longer than the end of the previous one: the rest of the performance is shifted
			CsRTOrigin += (UInt64)(-delay * 1000000.);
			delay = 0.;
			}
		if(OSCRT) {
			OSCBuffer b;
			b.data = e.osc;
			b.length = b.max = e.osclength;
			b.ok = TRUE;
			SCRTSend(&b);
			if(CsoundTrace) BPPrintMessage(0,odInfo,"OSC <- %s at %.3f s\n",(strcmp(e.head,"off") == 0) ? "/bp/off" : ((strcmp(e.head,"env") == 0) ? "/bp/env" : "/bp/note"),e.time);
			sent++;
			CsRTFreeEvent(&e); // Sent at its date: nothing to send again after a pause
			continue;
			}
		if(SuperColliderRT) {
			SCRTSendBundle(e.osc,e.osclength,now + (UInt64)(delay * 1000000.));
			if(CsoundTrace) BPPrintMessage(0,odInfo,"SuperCollider <- bp%s at +%.4f s:%.200s\n",e.head + 1,delay,e.rest);
			sent++;
			e.target = e.time;
			if(CsRTDump != NULL || CsRTPush(&CsRTSent,&CsRTSentSize,&CsRTSentMax,&e,FALSE) != OK) CsRTFreeEvent(&e);
			continue;
			}
		length = strlen(e.head) + strlen(e.rest) + 32;
		message = (char*) malloc(length);
		if(message == NULL) {
			BPPrintMessage(0,odError,"=> Out of memory in CsRTSendDue()\n");
			return ABORT;
			}
		snprintf(message,length,"$%s %.4f%s",e.head,delay,e.rest);
		CsRTSend(message);
		free(message);
		sent++;
		e.target = e.time;
		if(CsRTDump != NULL || CsRTPush(&CsRTSent,&CsRTSentSize,&CsRTSentMax,&e,FALSE) != OK) CsRTFreeEvent(&e);
		}
	if(sent > 0L) {
		memmove(&CsRTQueue[0],&CsRTQueue[sent],(size_t)(CsRTQueueSize - sent) * sizeof(CsRTEvent));
		CsRTQueueSize -= sent;
		}
	return OK;
	}


// note 11
static void CsRTStopAll(void) {
	// Turn off the notes of the instruments used, including the ones sent and not yet started (turnoff3).
	// Notes are released (turnoff2 mode 1) so that their envelopes end smoothly.
	char *code, line[128];
	size_t size;
	int i;
	if(!CsRTOn()) return;
	if(OSCRT) {
		OSCSendAddress("/bp/stop");
		return;
		}
	if(CsRTNumberInstruments == 0) return;
	if(SuperColliderRT) {
		// Bundles already sent are cancelled, and the notes playing are released
		OSCBuffer b;
		OSCInit(&b);
		OSCString(&b,"/clearSched");
		OSCString(&b,",");
		SCRTSend(&b);
		b.length = 0;
		OSCString(&b,"/n_set");
		OSCString(&b,",isf");
		OSCInt32(&b,(uint32_t) SCRT_GROUP);
		OSCString(&b,"gate");
		OSCFloat(&b,0.);
		SCRTSend(&b);
		OSCFree(&b);
		return;
		}
	size = 256 + (size_t) CsRTNumberInstruments * 64;
	code = (char*) malloc(size);
	if(code == NULL) return;
	strcpy(code,"instr BP_stop_all\n");
	for(i = 0; i < CsRTNumberInstruments; i++) {
		snprintf(line,sizeof(line)," turnoff3 %.0f\n turnoff2 %.0f, 0, 1\n",CsRTInstruments[i],CsRTInstruments[i]);
		strcat(code,line);
		}
	strcat(code," turnoff\nendin\nschedule \"BP_stop_all\", 0, 0.1\n");
	CsRTSend(code);
	free(code);
	return;
	}


// note 7
int CsRTFlush(int wait) {
	// Send the lines that are due. With 'wait', wait until all lines have been sent.
	// Stop, panic, pause and continue are handled by stop(), which also updates the "_time" file.
	int result;
	long i;
	UInt64 pausestart;
	if(!CsRTOn()) return OK;
	// note 8
	if(!CsRTStarted) {
		if(CsRTQueueSize == 0L) return OK;
		CsRTOrigin = getClockTime() + (UInt64)(CSRT_LATENCY * 1000000.);
		if(CsRTQueue[0].time < 0.) CsRTOrigin += (UInt64)(-CsRTQueue[0].time * 1000000.);
		CsRTStarted = TRUE;
		CsRTDumpDate = (CsRTQueue[0].time < 0.) ? CsRTQueue[0].time : 0.;
		if(SuperColliderRT) {
			SCRTCalibrate();
			SCRTStartGroup();
			}
		if(OSCRT) OSCSendAddress("/bp/start");
		}
	while(TRUE) {
		// note 11
		if((result = stop(0,"CsRTFlush")) != OK) {
			CsRTPanic();
			return result;
			}
		// note 12
		// A pause is requested by the PAUSE button (PausePlay), or by a MIDI Stop message received when the setting
		// "Respond to Stop/Continue" is on (StopPlay, set and reset by HandleInputEvent()), as in real-time MIDI
		if(PausePlay || StopPlay) {
			// Notes sent and not yet started will be sent again after the pause
			CsRTStopAll();
			for(i = 0; i < CsRTSentSize; i++) {
				if(CsRTPush(&CsRTQueue,&CsRTQueueSize,&CsRTQueueMax,&CsRTSent[i],TRUE) != OK) CsRTFreeEvent(&CsRTSent[i]);
				}
			CsRTSentSize = 0L;
			pausestart = getClockTime();
			while(PausePlay || StopPlay) {
				if((result = stop(1,"CsRTFlush pause")) != OK) {
					CsRTPanic();
					return result;
					}
				CsRTSleep(20L);
				}
			CsRTOrigin += getClockTime() - pausestart;
			}
		if(CsRTDump == NULL && (result = CsRTScripts()) != OK) { // note 20
			CsRTPanic();
			return result;
			}
		if((result = CsRTSendDue()) != OK) return result;
		CsRTSendMidiDue(wait && CsRTQueueSize == 0L); // note 22
		if(!wait || (CsRTQueueSize == 0L && CsRTMidiSize == 0L)) break;
		CsRTSleep(OSCRT ? 1L : 2L); // Real-time OSC sends events at their dates: they are checked more often
		}
	if(CsRTDump != NULL) fflush(CsRTDump); // The items already produced are kept if the console is stopped by a time limit
	return OK;
	}


// note 13
int CsRTEnd(void) {
	// Called by CloseCsScore(): send the remaining lines and wait for the end of the last note
	int result;
	if(!CsRTOn()) return OK;
	if(CsRTPendingLength > 0) {
		CsRTPending[CsRTPendingLength] = '\0';
		CsRTLine(CsRTPending);
		CsRTPendingLength = 0;
		}
	if(Panic || EmergencyExit) {
		CsRTPanic();
		return ABORT;
		}
	if((result = CsRTFlush(TRUE)) != OK) return result;
	if(!CsRTStarted) return OK;
	CsRTDumpDate = CsRTLastEnd;
	while(CsRTDump == NULL && getClockTime() < CsRTDateToClock(CsRTLastEnd)) {
		if((result = stop(0,"CsRTEnd")) != OK) {
			CsRTPanic();
			return result;
			}
		if(PausePlay || StopPlay) {
			if((result = CsRTFlush(FALSE)) != OK) return result;
			}
		if((result = CsRTScripts()) != OK) { // note 20
			CsRTPanic();
			return result;
			}
		CsRTSleep(20L);
		}
	// 2026-11-17: scripts dated at the end of the last note (e.g. "_script(MIDI send Continue) _script(wait for Stop)" after the last note) were never executed, since the loop ends at that date - fixed by Claude
	while(CsRTDump == NULL && CsRTScriptsDue((unsigned long)(CsRTLastEnd * 1000000.) + 1000L)) {
		if((result = stop(0,"CsRTEnd scripts")) != OK) {
			CsRTPanic();
			return result;
			}
		if((result = CsRTScripts()) != OK) {
			CsRTPanic();
			return result;
			}
		CsRTSleep(1L);
		}
	if(OSCRT) OSCSendAddress("/bp/end");
	BPPrintMessage(0,odInfo,"Real-time %s performance: %.3f seconds\n",CsRTName(),CsRTLastEnd);
	CsRTStarted = FALSE;
	CsRTLastEnd = 0.;
	return OK;
	}


// note 11
void CsRTPanic(void) {
	// Called after stop or panic: nothing more is sent, and the notes are turned off
	long i;
	if(!CsRTOn()) return;
	for(i = 0; i < CsRTQueueSize; i++) CsRTFreeEvent(&CsRTQueue[i]);
	CsRTQueueSize = 0L;
	for(i = 0; i < CsRTSentSize; i++) CsRTFreeEvent(&CsRTSent[i]);
	CsRTSentSize = 0L;
	CsRTMidiSize = 0L;
	CsRTStopAll();
	return;
	}


// note 14
long CsRTDate(long* p_end) {
	// Date of the performance (milliseconds) for the "_time" file, and expected end in *p_end
	UInt64 now;
	*p_end = (long)(CsRTLastEnd * 1000.);
	if(!CsRTStarted) return 0L;
	now = getClockTime();
	if(now <= CsRTOrigin) return 0L;
	return (long)((now - CsRTOrigin) / 1000L);
	}


// note 20
static int CsRTScriptsDue(unsigned long date) {
	// TRUE if a wait, hold or MIDI message (Start, Continue, Stop) dated at or before 'date' (microseconds) has not been executed
	int j, instr;
	for(j = 1; j <= Jinscript; j++) {
		if((*p_INscript)[j].chan == -1 || !CsRTWaitInstruction((*p_INscript)[j].scriptline)) continue;
		if((*p_INscript)[j].time <= date) return TRUE;
		}
	for(j = 1; j <= Joutscript; j++) {
		if((*p_OUTscript)[j].chan == -1) continue;
		instr = (*p_OUTscript)[j].scriptline;
		if(instr != 17 && instr != 190 && instr != 191 && instr != 192) continue;
		if((*p_OUTscript)[j].time <= date) return TRUE;
		}
	return FALSE;
	}


// note 20
static int CsRTWaitInstruction(int instr) {
	// Script instructions that wait for an event: Space, forever, a note, MIDI Start, Continue or Stop
	return (instr == 46 || instr == 47 || instr == 97 || instr == 66 || instr == 67 || instr == 128);
	}


// note 20
unsigned long CsRTScriptTime(void) {
	// Date of the performance (microseconds) on the timeline of script instructions, i.e. the date in the score.
	// Used by the MIDI input handlers instead of the clock of real-time MIDI.
	UInt64 now;
	if(!CsRTStarted) return 0L;
	now = getClockTime();
	if(now <= CsRTOrigin) return 0L;
	return (unsigned long)(now - CsRTOrigin);
	}


// note 20
static double CsRTBarrier(void) {
	// Earliest date (seconds) of a pending wait (input script) or hold (output script), or -1 if there is none
	double barrier, date;
	int j;
	barrier = -1.;
	for(j = 1; j <= Jinscript; j++) {
		if((*p_INscript)[j].chan == -1 || !CsRTWaitInstruction((*p_INscript)[j].scriptline)) continue;
		date = (double)(*p_INscript)[j].time / 1000000.;
		if(barrier < 0. || date < barrier) barrier = date;
		}
	for(j = 1; j <= Joutscript; j++) {
		if((*p_OUTscript)[j].chan == -1 || (*p_OUTscript)[j].scriptline != 17) continue; // Hold
		date = (double)(*p_OUTscript)[j].time / 1000000.;
		if(barrier < 0. || date < barrier) barrier = date;
		}
	return barrier;
	}


// note 20
static int CsRTScripts(void) {
	// Script instructions of the score, executed when their date is reached, as check_stop_instructions() does in real-time MIDI.
	// Their dates (microseconds) are those of the score. Instead of 'TimeStopped', the performance is shifted by moving CsRTOrigin.
	// - Hold: the performance is shifted by its duration
	// - MIDI send Start, Continue, Stop: the message is sent to the MIDI outputs if MIDI is running (option --rtmidi)
	// - Wait for a note, Start, Continue, Stop, Space, or forever: the performance waits until the MIDI input handler
	//   (HandleInputEvent(), StopWaiting()) resets StopPlay, then resumes at the date of the script after MIDIsyncDelay.
	//   Notes sounding go on, and lines dated at or after the script have not been sent (CsRTBarrier()).
	UInt64 now, resume;
	unsigned long t, date;
	int j, instr, result;
	unsigned char midiData[4];

	if(!CsRTStarted) return OK;
	now = getClockTime();
	if(now < CsRTOrigin) return OK;
	t = (unsigned long)(now - CsRTOrigin);
	for(j = 1; j <= Joutscript; j++) {
		if((*p_OUTscript)[j].chan == -1) continue;
		instr = (*p_OUTscript)[j].scriptline;
		if(instr != 17 && instr != 190 && instr != 191 && instr != 192) continue;
		if((*p_OUTscript)[j].time > t) continue;
		(*p_OUTscript)[j].chan = -1; // Done
		if(instr == 17) { // Hold
			CsRTOrigin += (UInt64)(*p_OUTscript)[j].duration;
			my_sprintf(Message,"Holding during %ld ms",(long)((*p_OUTscript)[j].duration / 1000L));
			Notify(Message,0);
			strcpy(Message,"");
			}
		else if(rtMIDI) {
			midiData[0] = (instr == 190) ? Start : ((instr == 191) ? Continue : Stop);
			sendMIDIEvent(-1,0,OUTmidi,0,midiData,1,0);
			}
		}
	for(j = 1; j <= Jinscript; j++) {
		if((*p_INscript)[j].chan == -1) continue;
		instr = (*p_INscript)[j].scriptline;
		if(!CsRTWaitInstruction(instr)) continue;
		date = (*p_INscript)[j].time;
		if(date > t) continue;
		StopPlay = TRUE;
		switch(instr) {
			case 46: WaitForSpace = TRUE; strcpy(Message,"Waiting for a spacebar hit"); break;
			case 47: WaitForEver = TRUE; strcpy(Message,"Waiting forever"); break;
			case 97: my_sprintf(Message,"Waiting for note #key %d channel %d at date %ld ms",(*p_INscript)[j].key,(*p_INscript)[j].chan,(long)(date / 1000L)); break;
			case 67: strcpy(Message,"Waiting for MIDI Start (250)"); break;
			case 66: strcpy(Message,"Waiting for MIDI Continue (251)"); break;
			case 128: strcpy(Message,"Waiting for MIDI Stop (252)"); break;
			}
		if(!rtMIDI && instr != 46 && instr != 47) BPPrintMessage(0,odWarning,"=> %s, but MIDI input is not active: only STOP can end this wait\n",Message);
		Notify(Message,0);
		strcpy(Message,"");
		while(StopPlay) {
			if((result = stop(1,"CsRTScripts")) != OK) return result;
			CsRTSleep(10L);
			}
		(*p_INscript)[j].chan = -1; // This wait is over, even if StopPlay was reset otherwise (e.g. by a MIDI Continue message)
		// The performance resumes at the date of the script, after MIDIsyncDelay milliseconds
		resume = getClockTime() + (UInt64)(MIDIsyncDelay > 0 ? MIDIsyncDelay : 0) * 1000L;
		if(resume > (UInt64) date && resume - (UInt64) date > CsRTOrigin) CsRTOrigin = resume - (UInt64) date;
		return OK; // Other scripts will be checked at the next call
		}
	return OK;
	}


// note 22
void CsRTMidiNote(int key,int chan,int instance,double date) {
	// Called by CscoreWrite() when it writes the line of a note of duration 0, such as an out-time note <<C0>>: if MIDI is running
	// (--rtmidi), a NoteOn of the same key and channel with velocity 0 will be sent at the date of the note. It is not audible,
	// but it can synchronise MIDI devices or another instance of BP3, whose input handlers accept NoteOn's with velocity 0.
	// Other notes are not sent, to avoid doubling the flow of events.
	CsRTMidiEvent e;
	if(!rtMIDI || CsRTSocket == CSRT_NOSOCKET) return; // Not with --rt-dump
	if(key < 0 || key > 127 || chan < 0 || chan > 15) return;
	e.time = date;
	e.key = key;
	e.chan = chan;
	e.instance = instance;
	e.velocity = 0;
	CsRTMidiPush(&e);
	return;
	}


int CsRTMidiTicksOn(void) {
	// 2026-11-25 TRUE if the ticks of a time base can be sent to MIDI (--rtmidi) or written to the file of option --rt-dump
	return (CsRTDump != NULL || (rtMIDI && CsRTSocket != CSRT_NOSOCKET));
	}


void CsRTMidiTick(int key,int chan,int velocity,double date) {
	// 2026-11-25 Called by TimeBaseStartItem() (TimeBase.c) for each tick of the time base and its end (velocity 0), at 'date' (seconds) in the score.
	// Ticks are sent to the MIDI output like the silent notes of CsRTMidiNote(), so that they can also synchronise other devices.
	CsRTMidiEvent e;
	if(!CsRTMidiTicksOn()) return;
	if(key < 0 || key > 127 || chan < 0 || chan > 15) return;
	e.time = date;
	e.key = key;
	e.chan = chan;
	e.instance = TB_TICK_INSTANCE; // Routed to the first output if no channel filter accepts its channel (sendMIDIEvent())
	e.velocity = velocity;
	CsRTMidiPush(&e);
	return;
	}


static void CsRTMidiPush(CsRTMidiEvent* p_e) {
	// Insert the event in CsRTMidiQueue, sorted by date (after those of the same date)
	long i;
	double date;
	if(CsRTMidiSize >= CsRTMidiMax) {
		long newmax = (CsRTMidiMax < 256L) ? 256L : 2L * CsRTMidiMax;
		CsRTMidiEvent* p = (CsRTMidiEvent*) realloc(CsRTMidiQueue,(size_t) newmax * sizeof(CsRTMidiEvent));
		if(p == NULL) {
			BPPrintMessage(0,odError,"=> Out of memory in CsRTMidiNote()\n");
			return;
			}
		CsRTMidiQueue = p;
		CsRTMidiMax = newmax;
		}
	date = p_e->time;
	i = CsRTMidiSize;
	while(i > 0 && CsRTMidiQueue[i - 1].time > date) i--;
	memmove(&CsRTMidiQueue[i + 1],&CsRTMidiQueue[i],(size_t)(CsRTMidiSize - i) * sizeof(CsRTMidiEvent));
	CsRTMidiQueue[i] = (*p_e);
	CsRTMidiSize++;
	return;
	}


// note 22
static void CsRTDumpMidiUntil(double date) {
	// Option --rt-dump: write the queued MIDI events (ticks of the time base) dated at or before 'date', or all of them if 'date' < 0
	long sent;
	if(CsRTDump == NULL) return;
	for(sent = 0L; sent < CsRTMidiSize; sent++) {
		if(date >= 0. && CsRTMidiQueue[sent].time > date + 0.0005) break;
		fprintf(CsRTDump,"%.4f MIDI NoteOn %d %d %d\n",CsRTMidiQueue[sent].time,CsRTMidiQueue[sent].chan + 1,CsRTMidiQueue[sent].key,CsRTMidiQueue[sent].velocity);
		}
	if(sent > 0L) {
		memmove(&CsRTMidiQueue[0],&CsRTMidiQueue[sent],(size_t)(CsRTMidiSize - sent) * sizeof(CsRTMidiEvent));
		CsRTMidiSize -= sent;
		}
	return;
	}


static void CsRTSendMidiDue(int final) {
	// Send the silent MIDI notes and ticks of the time base whose date has come. Unlike Csound lines, they are not sent in advance.
	// Those dated at or after a pending wait or hold are kept until it is over, like Csound lines (note 20).
	// With --rt-dump, they are written with the lines of the score (CsRTSendDue()), and the remaining ones when 'final' (end of the item).
	UInt64 now;
	double barrier;
	long sent;
	unsigned char midiData[4];
	if(CsRTDump != NULL) {
		if(final) CsRTDumpMidiUntil(-1.);
		return;
		}
	if(CsRTMidiSize == 0L || !CsRTStarted) return;
	now = getClockTime();
	barrier = CsRTBarrier();
	sent = 0L;
	while(sent < CsRTMidiSize) {
		if(barrier >= 0. && CsRTMidiQueue[sent].time >= barrier - 0.0005) break;
		// They are delayed by CsoundRTMidiDelay (option --csoundrt-delay), the time of the output buffer of the server, so that they come with the sound
		if(CsRTDateToClock(CsRTMidiQueue[sent].time + CsoundRTMidiDelay / 1000.) > now) break;
		midiData[0] = NoteOn + CsRTMidiQueue[sent].chan;
		midiData[1] = CsRTMidiQueue[sent].key;
		midiData[2] = CsRTMidiQueue[sent].velocity;
		sendMIDIEvent(CsRTMidiQueue[sent].instance,0,OUTmidi,0,midiData,3,0);
		sent++;
		}
	if(sent > 0L) {
		memmove(&CsRTMidiQueue[0],&CsRTMidiQueue[sent],(size_t)(CsRTMidiSize - sent) * sizeof(CsRTMidiEvent));
		CsRTMidiSize -= sent;
		}
	return;
	}


// ----------------------- Real-time SuperCollider (2026-11-20) -----------------------

static const char* CsRTName(void) {
	return SuperColliderRT ? "SuperCollider" : (OSCRT ? "OSC" : "Csound");
	}


void CsRTNoteInfo(int key,int chan,int velocity) {
	// 2026-11-21 Called by CscoreWrite() before it writes the line of a note: its MIDI key, channel (0-15) and velocity, sent by real-time OSC
	OSCInfoKey = key;
	OSCInfoChan = chan;
	OSCInfoVelocity = velocity;
	return;
	}


static void OSCSendAddress(const char* address) {
	// A message without arguments, e.g. "/bp/stop"
	OSCBuffer b;
	OSCInit(&b);
	OSCString(&b,address);
	OSCString(&b,",");
	SCRTSend(&b);
	OSCFree(&b);
	if(CsoundTrace) BPPrintMessage(0,odInfo,"OSC <- %s\n",address);
	return;
	}


static void OSCInit(OSCBuffer* b) {
	b->data = NULL;
	b->length = b->max = 0;
	b->ok = TRUE;
	return;
	}


static void OSCFree(OSCBuffer* b) {
	free(b->data);
	OSCInit(b);
	return;
	}


static int OSCRoom(OSCBuffer* b,int n) {
	// Make room for n more bytes
	unsigned char* p;
	int newmax;
	if(!b->ok) return FALSE;
	if(b->length + n <= b->max) return TRUE;
	newmax = 2 * (b->length + n) + 64;
	p = (unsigned char*) realloc(b->data,(size_t) newmax);
	if(p == NULL) {
		b->ok = FALSE;
		return FALSE;
		}
	b->data = p;
	b->max = newmax;
	return TRUE;
	}


static void OSCInt32(OSCBuffer* b,uint32_t x) {
	// OSC numbers are big-endian
	if(!OSCRoom(b,4)) return;
	b->data[b->length++] = (unsigned char)(x >> 24);
	b->data[b->length++] = (unsigned char)(x >> 16);
	b->data[b->length++] = (unsigned char)(x >> 8);
	b->data[b->length++] = (unsigned char) x;
	return;
	}


static void OSCFloat(OSCBuffer* b,double x) {
	float f;
	uint32_t u;
	f = (float) x;
	memcpy(&u,&f,4);
	OSCInt32(b,u);
	return;
	}


static void OSCString(OSCBuffer* b,const char* s) {
	// A string is followed by 1 to 4 null bytes, so that its length is a multiple of 4
	int n, padded;
	n = (int) strlen(s);
	padded = (n + 4) & ~3;
	if(!OSCRoom(b,padded)) return;
	memcpy(b->data + b->length,s,(size_t) n);
	memset(b->data + b->length + n,0,(size_t)(padded - n));
	b->length += padded;
	return;
	}


static int OSCDumpMessage(const unsigned char* data,int length,char* text,size_t size) {
	// Text of an OSC message: address, then its arguments (int32, float32, string, arrays [...]). Returns FALSE if it is not readable.
	int i, n, k, ntags;
	const char *address, *tags;
	size_t len;
	uint32_t u;
	float f;
	char item[300];

	#define OSC_STRING(p) { \
		if(i >= length) return FALSE; \
		n = (int) strnlen((const char*)(data + i),(size_t)(length - i)); \
		if(i + n >= length) return FALSE; \
		p = (const char*)(data + i); \
		i += (n + 4) & ~3; \
		}
	i = 0;
	OSC_STRING(address);
	OSC_STRING(tags);
	if(tags[0] != ',') return FALSE;
	snprintf(text,size,"%s",address);
	ntags = (int) strlen(tags);
	for(k = 1; k < ntags; k++) {
		switch(tags[k]) {
			case 'i':
			case 'f':
				if(i + 4 > length) return FALSE;
				u = ((uint32_t) data[i] << 24) | ((uint32_t) data[i + 1] << 16) | ((uint32_t) data[i + 2] << 8) | (uint32_t) data[i + 3];
				i += 4;
				if(tags[k] == 'i') snprintf(item,sizeof(item)," %d",(int)(int32_t) u);
				else {
					memcpy(&f,&u,4);
					snprintf(item,sizeof(item)," %.6g",(double) f);
					}
				break;
			case 's': {
				const char* str;
				OSC_STRING(str);
				snprintf(item,sizeof(item)," %.256s",str);
				break;
				}
			case '[': strcpy(item," ["); break;
			case ']': strcpy(item," ]"); break;
			default: snprintf(item,sizeof(item)," (type %c)",tags[k]); k = ntags; break;
			}
		len = strlen(text);
		if(len + strlen(item) + 1 < size) strcat(text,item);
		}
	#undef OSC_STRING
	return TRUE;
	}


static void OSCDump(const unsigned char* data,int length) {
	// Option "--rt-dump": write an OSC message or bundle to the file as text, preceded by its date.
	// The elements of a bundle are written on lines of their own, without the time tag of the bundle.
	char* text;
	int i, n;
	uint32_t u;
	if(CsRTDump == NULL || data == NULL || length <= 0) return;
	if(length >= 16 && memcmp(data,"#bundle",8) == 0) {
		i = 16;
		while(i + 4 <= length) {
			u = ((uint32_t) data[i] << 24) | ((uint32_t) data[i + 1] << 16) | ((uint32_t) data[i + 2] << 8) | (uint32_t) data[i + 3];
			n = (int) u;
			i += 4;
			if(n <= 0 || i + n > length) break;
			OSCDump(data + i,n);
			i += n;
			}
		return;
		}
	text = (char*) malloc(8 * (size_t) length + 64);
	if(text == NULL) return;
	if(OSCDumpMessage(data,length,text,8 * (size_t) length + 64)) fprintf(CsRTDump,"%.4f %s\n",CsRTDumpDate,text);
	else fprintf(CsRTDump,"%.4f (unreadable OSC message of %d bytes)\n",CsRTDumpDate,length);
	free(text);
	return;
	}


static int SCRTSend(OSCBuffer* b) {
	if(!b->ok || b->length == 0) return MISSED;
	if(b->length > CSRT_MAXMESSAGE) {
		if(CsRTSaidError++ < 5) BPPrintMessage(0,odError,"=> An OSC message of %d bytes is too long to be sent to SuperCollider\n",b->length);
		return MISSED;
		}
	if(CsRTDump != NULL) {
		OSCDump(b->data,b->length);
		return OK;
		}
	if(sendto(CsRTSocket,(const char*) b->data,b->length,0,(struct sockaddr*) &CsRTAddress,sizeof(CsRTAddress)) < 0) {
		if(CsRTSaidError++ < 5)
			BPPrintMessage(0,odError,"=> Could not send a message to SuperCollider on UDP port %d of %s\n",CsoundRTport,SuperColliderHost);
		return MISSED;
		}
	return OK;
	}


static void SCRTCalibrate(void) {
	// OSC time tags are NTP times: seconds since 1900 in the upper 32 bits, fraction in the lower 32 bits.
	// The offset between the system time and getClockTime() is measured at the start of each performance,
	// so that time tags have the resolution of getClockTime(). The server compares them with the system time of its own computer.
	UInt64 seconds, fraction, clock;
#if defined(_WIN64)
	FILETIME ft;
	ULARGE_INTEGER t;
	GetSystemTimeAsFileTime(&ft); // 100 ns units since 1601
	clock = getClockTime();
	t.LowPart = ft.dwLowDateTime;
	t.HighPart = ft.dwHighDateTime;
	seconds = t.QuadPart / 10000000ULL - 9435484800ULL; // 1601 -> 1900
	fraction = ((t.QuadPart % 10000000ULL) << 32) / 10000000ULL;
#else
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME,&ts);
	clock = getClockTime();
	seconds = (UInt64) ts.tv_sec + 2208988800ULL; // 1970 -> 1900
	fraction = ((UInt64) ts.tv_nsec << 32) / 1000000000ULL;
#endif
	SCRTClockToNTP = ((seconds << 32) + fraction) - ((clock / 1000000ULL) << 32) - (((clock % 1000000ULL) << 32) / 1000000ULL);
	return;
	}


static void SCRTSendBundle(unsigned char* message,int length,UInt64 clock) {
	// Send a message in a bundle to be executed at 'clock' (microseconds of getClockTime())
	OSCBuffer b;
	UInt64 tag;
	if(message == NULL || length <= 0) return;
	tag = SCRTClockToNTP + ((clock / 1000000ULL) << 32) + (((clock % 1000000ULL) << 32) / 1000000ULL);
	OSCInit(&b);
	OSCString(&b,"#bundle");
	OSCInt32(&b,(uint32_t)(tag >> 32));
	OSCInt32(&b,(uint32_t)(tag & 0xFFFFFFFFULL));
	OSCInt32(&b,(uint32_t) length);
	if(OSCRoom(&b,length)) {
		memcpy(b.data + b.length,message,(size_t) length);
		b.length += length;
		}
	SCRTSend(&b);
	OSCFree(&b);
	return;
	}


static void SCRTStartGroup(void) {
	// Create the group of notes at the head of the root node. It may already exist (previous performance):
	// the error message of the server is then turned off for this bundle by "/error -1".
	OSCBuffer b, m;
	OSCInit(&b);
	OSCInit(&m);
	OSCString(&b,"#bundle");
	OSCInt32(&b,0);
	OSCInt32(&b,1); // Time tag 1: immediately
	OSCString(&m,"/error");
	OSCString(&m,",i");
	OSCInt32(&m,(uint32_t) -1);
	OSCInt32(&b,(uint32_t) m.length);
	if(m.ok && OSCRoom(&b,m.length)) {
		memcpy(b.data + b.length,m.data,(size_t) m.length);
		b.length += m.length;
		}
	m.length = 0;
	OSCString(&m,"/g_new");
	OSCString(&m,",iii");
	OSCInt32(&m,(uint32_t) SCRT_GROUP);
	OSCInt32(&m,0); // add to head
	OSCInt32(&m,0); // of the root node
	OSCInt32(&b,(uint32_t) m.length);
	if(m.ok && OSCRoom(&b,m.length)) {
		memcpy(b.data + b.length,m.data,(size_t) m.length);
		b.length += m.length;
		}
	// The effects (SynthDef "bp_fx", e.g. a reverb reading a bus to which notes send part of their sound) are started after the group.
	// Nothing happens if they are already running, or if this SynthDef is not defined.
	m.length = 0;
	OSCString(&m,"/s_new");
	OSCString(&m,",siii");
	OSCString(&m,"bp_fx");
	OSCInt32(&m,(uint32_t)(SCRT_GROUP + 1));
	OSCInt32(&m,3); // add after
	OSCInt32(&m,(uint32_t) SCRT_GROUP);
	OSCInt32(&b,(uint32_t) m.length);
	if(m.ok && OSCRoom(&b,m.length)) {
		memcpy(b.data + b.length,m.data,(size_t) m.length);
		b.length += m.length;
		}
	SCRTSend(&b);
	OSCFree(&b);
	OSCFree(&m);
	return;
	}


static int SCRTInstrumentIndex(int instrument) {
	// Index in the '-cs' or '-sc' file of the instrument numbered 'instrument', or -1 if there is none
	int ins;
	if(NoCsoundInstrumentFile() || p_CsInstrumentIndex == NULL) return -1;
	for(ins = 0; ins < Jinstr; ins++)
		if((*p_CsInstrumentIndex)[ins] == instrument) return ins;
	return -1;
	}


static int SCRTArguments(int instrument,double duration,const char* fields,char names[][16],double* values,char* name,size_t namesize,int rawfields);
static int SCRTEnvelopes(int instrument,const char* fields,SCRTEnvelope* envs);

static void SCRTNoteMessage(OSCBuffer* b,int instrument,double duration,const char* fields) {
	// The message "/s_new <SynthDef> -1 0 SCRT_GROUP name value name value..." creating a note, see SCRTArguments(),
	// followed by envelopes "name [array]", see SCRTEnvelopes()
	char names[SCRT_MAXARGS][16], tags[2 * SCRT_MAXARGS + 8 + SCRT_MAXENVS * (8 + 4 * SCRT_ENVSEGMENTS)], name[128];
	double values[SCRT_MAXARGS];
	SCRTEnvelope envs[SCRT_MAXENVS];
	int nargs, nenvs, i, k;
	nargs = SCRTArguments(instrument,duration,fields,names,values,name,sizeof(name),TRUE);
	nenvs = SCRTEnvelopes(instrument,fields,envs);
	strcpy(tags,",siii");
	for(i = 0; i < nargs; i++) strcat(tags,"sf");
	for(i = 0; i < nenvs; i++) {
		strcat(tags,"s[");
		for(k = 0; k < 4 * envs[i].points; k++) strcat(tags,"f"); // 4 + 4 * (points - 1) values
		strcat(tags,"]");
		}
	OSCString(b,"/s_new");
	OSCString(b,tags);
	OSCString(b,name);
	OSCInt32(b,(uint32_t) -1);	// node ID chosen by the server
	OSCInt32(b,0);				// add to head
	OSCInt32(b,(uint32_t) SCRT_GROUP);
	for(i = 0; i < nargs; i++) {
		OSCString(b,names[i]);
		OSCFloat(b,values[i]);
		}
	for(i = 0; i < nenvs; i++) {
		// The array of an Env in SuperCollider: initial level, number of segments, release node, loop node (-99 = none),
		// then for each segment: level, time, shape (1 = linear), curve
		OSCString(b,envs[i].name);
		OSCFloat(b,envs[i].level[0]);
		OSCFloat(b,(double) (envs[i].points - 1));
		OSCFloat(b,-99.);
		OSCFloat(b,-99.);
		for(k = 1; k < envs[i].points; k++) {
			OSCFloat(b,envs[i].level[k]);
			OSCFloat(b,envs[i].time[k - 1]);
			OSCFloat(b,1.);
			OSCFloat(b,0.);
			}
		}
	return;
	}


static void SCRTStoreTable(int number,const char* fields) {
	// 2026-11-21 Keep function table 'number' of the score, if it is made of straight segments (GEN 7, or 5 taken as straight)
	// 'fields' is the text after p1: start date, size, GEN number, then value, length, value, length... value
	double x[2 * SCRT_MAXARGS + 1], gen;
	int n, i, k;
	const char* p;
	char* end;
	SCRTTable* t;

	p = fields;
	for(i = 0; i < 3; i++) { // start date, size, GEN
		gen = strtod(p,&end);
		if(end == p) return;
		p = end;
		}
	n = 0;
	while(n < 2 * SCRT_MAXARGS + 1) {
		x[n] = strtod(p,&end);
		if(end == p) break;
		n++;
		p = end;
		}
	for(k = 0; k < SCRTNumberTables; k++) if(SCRTTables[k].number == number) break; // A table may be defined again
	if(k < SCRTNumberTables) {
		free(SCRTTables[k].value);
		free(SCRTTables[k].length);
		SCRTTables[k].value = SCRTTables[k].length = NULL;
		SCRTTables[k].points = 0;
		}
	if((fabs(gen) != 7. && fabs(gen) != 5.) || n < 3 || (n % 2) == 0) return; // Other GENs are not sent
	if(k == SCRTNumberTables) {
		if(SCRTNumberTables >= SCRTMaxTables) {
			int max = (SCRTMaxTables == 0) ? 64 : 2 * SCRTMaxTables;
			SCRTTable* more = (SCRTTable*) realloc(SCRTTables,max * sizeof(SCRTTable));
			if(more == NULL) return;
			SCRTTables = more;
			SCRTMaxTables = max;
			}
		SCRTNumberTables++;
		}
	t = &SCRTTables[k];
	t->number = number;
	t->points = (n + 1) / 2;
	t->value = (double*) malloc(t->points * sizeof(double));
	t->length = (double*) malloc(t->points * sizeof(double));
	if(t->value == NULL || t->length == NULL) {
		free(t->value); free(t->length);
		t->value = t->length = NULL;
		t->points = 0;
		return;
		}
	for(i = 0; i < t->points; i++) {
		t->value[i] = x[2 * i];
		t->length[i] = (i < t->points - 1) ? x[2 * i + 1] : 0.;
		}
	return;
	}


static int SCRTEnvelope1(SCRTEnvelope* env,const char* name,int start,int end,int table,const double* field,int nfields) {
	// The envelope of one parameter: table number field[table - 4] if it is a table kept by SCRTStoreTable(),
	// otherwise a straight line from field[start - 4] to field[end - 4] (or to the start value if there is no end)
	int k, i, n;
	double total, position, step, a, b;
	SCRTTable* t;

	if(start < 4 || start - 4 >= nfields) return FALSE;
	snprintf(env->name,sizeof(env->name),"%s",name);
	t = NULL;
	if(table >= 4 && table - 4 < nfields && field[table - 4] > 0.) {
		for(k = 0; k < SCRTNumberTables; k++)
			if(SCRTTables[k].number == (int) field[table - 4] && SCRTTables[k].points > 1) {
				t = &SCRTTables[k];
				break;
				}
		}
	if(t == NULL) {
		env->points = 2;
		env->level[0] = field[start - 4];
		env->level[1] = (end >= 4 && end - 4 < nfields) ? field[end - 4] : field[start - 4];
		env->time[0] = 1.;
		return TRUE;
		}
	total = 0.;
	for(i = 0; i < t->points - 1; i++) total += t->length[i];
	if(total <= 0.) return FALSE;
	if(t->points <= SCRT_ENVSEGMENTS + 1) {
		env->points = t->points;
		for(i = 0; i < t->points; i++) env->level[i] = t->value[i];
		for(i = 0; i < t->points - 1; i++) env->time[i] = t->length[i] / total;
		return TRUE;
		}
	// Too many segments: the table is sampled at SCRT_ENVSEGMENTS + 1 regular positions
	env->points = SCRT_ENVSEGMENTS + 1;
	step = total / SCRT_ENVSEGMENTS;
	i = 0;
	a = 0.; // position of value i
	for(n = 0; n <= SCRT_ENVSEGMENTS; n++) {
		position = n * step;
		while(i < t->points - 2 && a + t->length[i] < position) {
			a += t->length[i];
			i++;
			}
		b = (t->length[i] > 0.) ? (position - a) / t->length[i] : 0.;
		if(b > 1.) b = 1.;
		if(b < 0.) b = 0.;
		env->level[n] = t->value[i] + b * (t->value[i + 1] - t->value[i]);
		if(n < SCRT_ENVSEGMENTS) env->time[n] = 1. / SCRT_ENVSEGMENTS;
		}
	return TRUE;
	}


static int SCRTEnvelopes(int instrument,const char* fields,SCRTEnvelope* envs) {
	// 2026-11-21 Real-time SuperCollider: the envelopes of the continuous parameters of the instrument described in the '-sc' file,
	// over the duration of the note (times are fractions of it, so that a SynthDef reads them with EnvGen.kr(..., timeScale: sustain)):
	// "volenv", "bendenv", "pressenv", "modenv", "panenv", and NAME + "env" for an additional parameter NAME (e.g. "slideenv").
	// Each follows the function table named in the score line if there is one (GEN 7), otherwise goes straight from the start value to the end value.
	double field[SCRT_MAXARGS];
	int nfields, nenvs, ins, k;
	const char* p;
	char* end;
	char name[24];

	ins = SCRTInstrumentIndex(instrument);
	if(ins < 0 || p_CsInstrument == NULL) return 0;
	nfields = 0;
	strtod(fields,&end); // p3
	p = end;
	while(nfields < SCRT_MAXARGS) {
		double x = strtod(p,&end);
		if(end == p) break;
		field[nfields++] = x;
		p = end;
		}
	nenvs = 0;
	if(SCRTEnvelope1(&envs[nenvs],"volenv",(*p_CsVolumeStartIndex)[ins],(*p_CsVolumeEndIndex)[ins],(*p_CsInstrument)[ins].volumetable,field,nfields)) nenvs++;
	if(SCRTEnvelope1(&envs[nenvs],"bendenv",(*p_CsPitchBendStartIndex)[ins],(*p_CsPitchBendEndIndex)[ins],(*p_CsInstrument)[ins].pitchbendtable,field,nfields)) nenvs++;
	if(SCRTEnvelope1(&envs[nenvs],"pressenv",(*p_CsPressureStartIndex)[ins],(*p_CsPressureEndIndex)[ins],(*p_CsInstrument)[ins].pressuretable,field,nfields)) nenvs++;
	if(SCRTEnvelope1(&envs[nenvs],"modenv",(*p_CsModulationStartIndex)[ins],(*p_CsModulationEndIndex)[ins],(*p_CsInstrument)[ins].modulationtable,field,nfields)) nenvs++;
	if(SCRTEnvelope1(&envs[nenvs],"panenv",(*p_CsPanoramicStartIndex)[ins],(*p_CsPanoramicEndIndex)[ins],(*p_CsInstrument)[ins].panoramictable,field,nfields)) nenvs++;
	if((*p_CsInstrument)[ins].paramlist != NULL) {
		CsoundParam* param;
		for(k = 0; k < (*p_CsInstrument)[ins].ipmax && nenvs < SCRT_MAXENVS; k++) {
			param = &(*((*p_CsInstrument)[ins].paramlist))[k];
			if(param->name == NULL || *(param->name) == NULL || (*(param->name))[0] == '\0') continue;
			snprintf(name,sizeof(name),"%.13senv",*(param->name));
			if(SCRTEnvelope1(&envs[nenvs],name,param->startindex,param->endindex,param->table,field,nfields)) nenvs++;
			}
		}
	return nenvs;
	}


static void OSCNoteMessage(OSCBuffer* b,OSCBuffer* off,int instrument,double duration,const char* fields) {
	// 2026-11-21 Real-time OSC: "/bp/note id instrument name key freq duration velocity channel [name value]..." in 'b',
	// and "/bp/off id instrument name key channel" in 'off'. The named values are those of SCRTArguments(), except
	// "sustain" and "freq" which are given before, and the raw fields p4, p5...
	char names[SCRT_MAXARGS][16], tags[2 * SCRT_MAXARGS + 16], name[128];
	double values[SCRT_MAXARGS], freq;
	int nargs, i, key, chan, velocity;
	long id;
	nargs = SCRTArguments(instrument,duration,fields,names,values,name,sizeof(name),FALSE);
	freq = 0.;
	for(i = 0; i < nargs; i++) if(strcmp(names[i],"freq") == 0) freq = values[i];
	key = OSCInfoKey;
	if(key < 0 || key > 127) key = (freq > 0.) ? (int) lround(69. + 12. * log(freq / 440.) / log(2.)) : 0;
	if(key < 0) key = 0;
	if(key > 127) key = 127;
	chan = OSCInfoChan + 1;
	velocity = OSCInfoVelocity;
	OSCInfoKey = -1; // Used by this line only
	id = ++OSCNoteId;
	strcpy(tags,",iisifffi");
	for(i = 0; i < nargs; i++) if(strcmp(names[i],"sustain") != 0 && strcmp(names[i],"freq") != 0) strcat(tags,"sf");
	OSCString(b,"/bp/note");
	OSCString(b,tags);
	OSCInt32(b,(uint32_t) id);
	OSCInt32(b,(uint32_t) instrument);
	OSCString(b,name);
	OSCInt32(b,(uint32_t) key);
	OSCFloat(b,freq);
	OSCFloat(b,duration);
	OSCFloat(b,(double) velocity);
	OSCInt32(b,(uint32_t) chan);
	for(i = 0; i < nargs; i++) {
		if(strcmp(names[i],"sustain") == 0 || strcmp(names[i],"freq") == 0) continue;
		OSCString(b,names[i]);
		OSCFloat(b,values[i]);
		}
	OSCString(off,"/bp/off");
	OSCString(off,",iisii");
	OSCInt32(off,(uint32_t) id);
	OSCInt32(off,(uint32_t) instrument);
	OSCString(off,name);
	OSCInt32(off,(uint32_t) key);
	OSCInt32(off,(uint32_t) chan);
	return;
	}


static void OSCPushEnvelopes(int instrument,double time,double duration,const char* fields) {
	// 2026-11-23 Real-time OSC: for each continuous parameter of the note that follows a function table with intermediate values
	// (see SCRTEnvelopes()), the message "/bp/env id instrument name parameter time value time value...", queued after the /bp/note
	// of the same date. Times are seconds from the start of the note, from 0 to its duration; 'parameter' is the name used in /bp/note
	// ("vol", "bend", "press", "mod", "pan" or the name of an additional parameter). Parameters going straight from their start value
	// to their end value get no /bp/env, since /bp/note gives both values.
	SCRTEnvelope envs[SCRT_MAXENVS];
	char names[SCRT_MAXARGS][16], tags[8 + 2 * (SCRT_ENVSEGMENTS + 1)], name[128], param[24];
	double values[SCRT_MAXARGS], t;
	int nenvs, i, k;
	size_t length;
	OSCBuffer b;
	CsRTEvent e;

	nenvs = SCRTEnvelopes(instrument,fields,envs);
	if(nenvs == 0) return;
	SCRTArguments(instrument,duration,fields,names,values,name,sizeof(name),FALSE); // Only for the name of the instrument
	for(i = 0; i < nenvs; i++) {
		if(envs[i].points <= 2) continue;
		// Intermediate values on the straight line from the start value to the end value add nothing to /bp/note
		t = 0.;
		for(k = 1; k < envs[i].points - 1; k++) {
			t += envs[i].time[k - 1];
			if(fabs(envs[i].level[k] - (envs[i].level[0] + t * (envs[i].level[envs[i].points - 1] - envs[i].level[0]))) > 0.001) break;
			}
		if(k == envs[i].points - 1) continue;
		snprintf(param,sizeof(param),"%.*s",(int) sizeof(envs[i].name),envs[i].name); // The precision tells GCC that the name fits
		length = strlen(param);
		if(length > 3 && strcmp(param + length - 3,"env") == 0) param[length - 3] = '\0'; // "slideenv" -> "slide"
		strcpy(tags,",iiss");
		for(k = 0; k < envs[i].points; k++) strcat(tags,"ff");
		OSCInit(&b);
		OSCString(&b,"/bp/env");
		OSCString(&b,tags);
		OSCInt32(&b,(uint32_t) OSCNoteId); // The note just made by OSCNoteMessage()
		OSCInt32(&b,(uint32_t) instrument);
		OSCString(&b,name);
		OSCString(&b,param);
		t = 0.;
		for(k = 0; k < envs[i].points; k++) {
			if(k > 0) t += envs[i].time[k - 1] * duration;
			if(k == envs[i].points - 1) t = duration; // No rounding error on the last date
			OSCFloat(&b,t);
			OSCFloat(&b,envs[i].level[k]);
			}
		e.time = time;
		e.target = 0.;
		e.head = (char*) malloc(8);
		e.rest = (char*) malloc(8);
		if(!b.ok || e.head == NULL || e.rest == NULL) {
			free(e.head); free(e.rest);
			OSCFree(&b);
			BPPrintMessage(0,odError,"=> Out of memory in OSCPushEnvelopes()\n");
			return;
			}
		strcpy(e.head,"env");
		strcpy(e.rest,"");
		e.osc = b.data;
		e.osclength = b.length;
		if(CsRTPush(&CsRTQueue,&CsRTQueueSize,&CsRTQueueMax,&e,TRUE) != OK) CsRTFreeEvent(&e);
		}
	return;
	}


static int SCRTArguments(int instrument,double duration,const char* fields,char names[][16],double* values,char* name,size_t namesize,int rawfields) {
	// 'fields' is the text of the score line after p2. Returns the number of named arguments in names[] and values[], and the name of the instrument.
	// The message "/s_new bp<instrument> -1 0 SCRT_GROUP name value name value..." creating a note.
	// Arguments: "sustain" (p3, seconds), then the fields of the instrument described in the '-cs' file (or the default fields):
	// "freq" (Hz), "vol", "vol2" (start and end volume), "vel" (attack velocity), "relvel", "pan", "pan2", "mod", "mod2",
	// "press", "press2", "bend", "bend2", the additional parameters of the instrument under their names (start and end values,
	// e.g. "splash" and "splash2"), and every field p4, p5... under the name "p4", "p5"...
	// A SynthDef ignores the arguments it does not declare.
	// 2026-11-20 With a '-sc' file (option "-sc"), the SynthDef is the one named like the instrument, e.g. "violin" for _ins(violin).
	// Otherwise, or if the instrument has no name, it is "bp<n>".
	double field[SCRT_MAXARGS];
	int nfields, nargs, i, ip, ins, found;
	const char* p;
	char* end;
	struct { int index; const char* name; } named[13];

	nfields = 0; // field[k] is p(k+4)
	strtod(fields,&end); // 'fields' starts with p3, which is 'duration'
	p = end;
	while(nfields < SCRT_MAXARGS) {
		double x = strtod(p,&end);
		if(end == p) break;
		field[nfields++] = x;
		p = end;
		}
	nargs = 0;
	strcpy(names[nargs],"sustain");
	values[nargs++] = duration;
	ins = SCRTInstrumentIndex(instrument);
	found = (ins >= 0);
	if(!found) ins = 0; // Default fields, as in CscoreWrite()
	named[0].index = (p_CsPitchFormat != NULL && (*p_CsPitchFormat)[ins] == IGNORER) ? -1 : (*p_CsPitchIndex)[ins]; named[0].name = "freq";
	named[1].index = (*p_CsVolumeStartIndex)[ins]; named[1].name = "vol";
	named[2].index = (*p_CsVolumeEndIndex)[ins]; named[2].name = "vol2";
	named[3].index = (*p_CsAttackVelocityIndex)[ins]; named[3].name = "vel";
	named[4].index = (*p_CsReleaseVelocityIndex)[ins]; named[4].name = "relvel";
	named[5].index = (*p_CsPanoramicStartIndex)[ins]; named[5].name = "pan";
	named[6].index = (*p_CsPanoramicEndIndex)[ins]; named[6].name = "pan2";
	named[7].index = (*p_CsModulationStartIndex)[ins]; named[7].name = "mod";
	named[8].index = (*p_CsModulationEndIndex)[ins]; named[8].name = "mod2";
	named[9].index = (*p_CsPressureStartIndex)[ins]; named[9].name = "press";
	named[10].index = (*p_CsPressureEndIndex)[ins]; named[10].name = "press2";
	named[11].index = (*p_CsPitchBendStartIndex)[ins]; named[11].name = "bend";
	named[12].index = (*p_CsPitchBendEndIndex)[ins]; named[12].name = "bend2";
	for(i = 0; i < 13 && nargs < SCRT_MAXARGS; i++) {
		ip = named[i].index;
		if(ip < 4 || ip - 4 >= nfields) continue;
		strcpy(names[nargs],named[i].name);
		values[nargs++] = field[ip - 4];
		}
	if(found && (*p_CsInstrument)[ins].paramlist != NULL) {
		// Additional parameters of the instrument
		CsoundParam* param;
		int k, index;
		for(k = 0; k < (*p_CsInstrument)[ins].ipmax && nargs < SCRT_MAXARGS - 1; k++) {
			param = &(*((*p_CsInstrument)[ins].paramlist))[k];
			if(param->name == NULL || *(param->name) == NULL || (*(param->name))[0] == '\0') continue;
			for(i = 0; i < 2; i++) {
				index = (i == 0) ? param->startindex : param->endindex;
				if(index < 4 || index - 4 >= nfields) continue;
				snprintf(names[nargs],sizeof(names[nargs]),"%.13s%s",*(param->name),(i == 0) ? "" : "2");
				values[nargs++] = field[index - 4];
				}
			}
		}
	for(i = 0; rawfields && i < nfields && nargs < SCRT_MAXARGS; i++) {
		snprintf(names[nargs],16,"p%d",i + 4);
		values[nargs++] = field[i];
		}
	// SuperCollider: the instrument's name is used with a '-sc' file only. Real-time OSC: with any '-sc' or '-cs' file.
	if((SuperColliderResource || OSCRT) && found && pp_CsInstrumentName != NULL && (*pp_CsInstrumentName)[ins] != NULL && (*((*pp_CsInstrumentName)[ins]))[0] != '\0')
		snprintf(name,namesize,"%s",*((*pp_CsInstrumentName)[ins]));
	else snprintf(name,namesize,"bp%d",instrument);
	return nargs;
	}
