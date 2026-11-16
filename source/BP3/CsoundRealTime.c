/* CsoundRealTime.c (BP3) */

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

#include "-BP3.h"
#include "-BP3decl.h"

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
	typedef int CsRTsocket;
	#define CSRT_NOSOCKET (-1)
	#define CsRTclosesocket close
#endif

#define CSRT_LOOKAHEAD 0.15		// seconds: lines are sent this time before their date
#define CSRT_LATENCY 0.3		// seconds: the performance starts this time after the first item has been computed
#define CSRT_MAXMESSAGE 65000	// bytes: maximum size of a UDP message
#define CSRT_MAXINSTR 512		// maximum number of instruments turned off by CsRTStopAll()

typedef struct {
	double time;	// date (seconds) in the score
	double target;	// date (seconds since CsRTOrigin) at which it was sent to start, only for lines in CsRTSent
	char* head;		// e.g. "i1" or "f101"
	char* rest;		// the fields after p2, starting with a space or empty
	} CsRTEvent;

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

static int CsRTSend(const char* message);
static void CsRTLine(char* line);
static int CsRTPush(CsRTEvent** p_list,long* p_size,long* p_max,CsRTEvent* p_e,int sorted);
static void CsRTFreeEvent(CsRTEvent* p_e);
static void CsRTSleep(long milliseconds);


int CsRTOpen(void) {
	// Called by ParsePostInitArgs() for option "--csoundrt port"
	if(CsRTSocket != CSRT_NOSOCKET) return OK;
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
	{
		// The default size limits messages to 9216 bytes on MacOS, too few for long function tables
		int size = 65536;
		setsockopt(CsRTSocket,SOL_SOCKET,SO_SNDBUF,(const char*)&size,sizeof(size));
		}
	memset(&CsRTAddress,0,sizeof(CsRTAddress));
	CsRTAddress.sin_family = AF_INET;
	CsRTAddress.sin_port = htons((unsigned short) CsoundRTport);
	CsRTAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
	CsRTQueueSize = CsRTSentSize = 0L;
	CsRTPendingLength = 0;
	CsRTStarted = FALSE;
	CsRTLastEnd = 0.;
	CsRTNumberInstruments = 0;
	CsRTSaidError = 0;
	BPPrintMessage(0,odInfo,"Real-time Csound: sending score lines to UDP port %d of this computer\n",CsoundRTport);
	return OK;
	}


void CsRTClose(void) {
	long i;
	if(CsRTSocket != CSRT_NOSOCKET) {
		CsRTclosesocket(CsRTSocket);
		CsRTSocket = CSRT_NOSOCKET;
#if defined(_WIN64)
		WSACleanup();
#endif
		}
	for(i = 0; i < CsRTQueueSize; i++) CsRTFreeEvent(&CsRTQueue[i]);
	for(i = 0; i < CsRTSentSize; i++) CsRTFreeEvent(&CsRTSent[i]);
	free(CsRTQueue); CsRTQueue = NULL;
	free(CsRTSent); CsRTSent = NULL;
	free(CsRTPending); CsRTPending = NULL;
	CsRTQueueSize = CsRTQueueMax = CsRTSentSize = CsRTSentMax = 0L;
	CsRTPendingLength = CsRTPendingMax = 0;
	return;
	}


void CsRTText(const char* format,va_list args) {
	// Called by BPPrintMessage() for destination odCsScore. Text is stored until the end of a line, then the line is processed.
	va_list args2;
	int length;
	size_t needed;
	char *start, *end;
	if(CsRTSocket == CSRT_NOSOCKET) return;
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
	e.time = time;
	e.target = 0.;
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
	if(CsRTPush(&CsRTQueue,&CsRTQueueSize,&CsRTQueueMax,&e,TRUE) != OK) {
		CsRTFreeEvent(&e);
		return;
		}
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
	return;
	}


static int CsRTSend(const char* message) {
	size_t length;
	length = strlen(message);
	if(length > CSRT_MAXMESSAGE) {
		if(CsRTSaidError++ < 5)
			BPPrintMessage(0,odError,"=> A line of %ld bytes is too long to be sent to Csound: %.40s...\n",(long) length,message);
		return MISSED;
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


static int CsRTSendDue(void) {
	// Send the lines whose date is less than CSRT_LOOKAHEAD seconds away
	UInt64 now;
	double delay;
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
	sent = 0L;
	while(sent < CsRTQueueSize) {
		e = CsRTQueue[sent];
		delay = ((double) CsRTDateToClock(e.time) - (double) now) / 1000000.;
		if(delay > CSRT_LOOKAHEAD) break;
		if(delay < 0.) {
			// Late, e.g. because computing the item took longer than the end of the previous one: the rest of the performance is shifted
			CsRTOrigin += (UInt64)(-delay * 1000000.);
			delay = 0.;
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
		if(CsRTPush(&CsRTSent,&CsRTSentSize,&CsRTSentMax,&e,FALSE) != OK) CsRTFreeEvent(&e);
		}
	if(sent > 0L) {
		memmove(&CsRTQueue[0],&CsRTQueue[sent],(size_t)(CsRTQueueSize - sent) * sizeof(CsRTEvent));
		CsRTQueueSize -= sent;
		}
	return OK;
	}


static void CsRTStopAll(void) {
	// Turn off the notes of the instruments used, including the ones sent and not yet started (turnoff3).
	// Notes are released (turnoff2 mode 1) so that their envelopes end smoothly.
	char *code, line[128];
	size_t size;
	int i;
	if(CsRTSocket == CSRT_NOSOCKET || CsRTNumberInstruments == 0) return;
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


int CsRTFlush(int wait) {
	// Send the lines that are due. With 'wait', wait until all lines have been sent.
	// Stop, panic, pause and continue are handled by stop(), which also updates the "_time" file.
	int result;
	long i;
	UInt64 pausestart;
	if(CsRTSocket == CSRT_NOSOCKET) return OK;
	if(!CsRTStarted) {
		if(CsRTQueueSize == 0L) return OK;
		CsRTOrigin = getClockTime() + (UInt64)(CSRT_LATENCY * 1000000.);
		if(CsRTQueue[0].time < 0.) CsRTOrigin += (UInt64)(-CsRTQueue[0].time * 1000000.);
		CsRTStarted = TRUE;
		}
	while(TRUE) {
		if((result = stop(0,"CsRTFlush")) != OK) {
			CsRTPanic();
			return result;
			}
		if(PausePlay) {
			// Notes sent and not yet started will be sent again after the pause
			CsRTStopAll();
			for(i = 0; i < CsRTSentSize; i++) {
				if(CsRTPush(&CsRTQueue,&CsRTQueueSize,&CsRTQueueMax,&CsRTSent[i],TRUE) != OK) CsRTFreeEvent(&CsRTSent[i]);
				}
			CsRTSentSize = 0L;
			pausestart = getClockTime();
			while(PausePlay) {
				if((result = stop(1,"CsRTFlush pause")) != OK) {
					CsRTPanic();
					return result;
					}
				CsRTSleep(20L);
				}
			CsRTOrigin += getClockTime() - pausestart;
			}
		if((result = CsRTSendDue()) != OK) return result;
		if(!wait || CsRTQueueSize == 0L) break;
		CsRTSleep(2L);
		}
	return OK;
	}


int CsRTEnd(void) {
	// Called by CloseCsScore(): send the remaining lines and wait for the end of the last note
	int result;
	if(CsRTSocket == CSRT_NOSOCKET) return OK;
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
	while(getClockTime() < CsRTDateToClock(CsRTLastEnd)) {
		if((result = stop(0,"CsRTEnd")) != OK) {
			CsRTPanic();
			return result;
			}
		if(PausePlay) {
			if((result = CsRTFlush(FALSE)) != OK) return result;
			}
		CsRTSleep(20L);
		}
	BPPrintMessage(0,odInfo,"Real-time Csound performance: %.3f seconds\n",CsRTLastEnd);
	CsRTStarted = FALSE;
	CsRTLastEnd = 0.;
	return OK;
	}


void CsRTPanic(void) {
	// Called after stop or panic: nothing more is sent, and the notes are turned off
	long i;
	if(CsRTSocket == CSRT_NOSOCKET) return;
	for(i = 0; i < CsRTQueueSize; i++) CsRTFreeEvent(&CsRTQueue[i]);
	CsRTQueueSize = 0L;
	for(i = 0; i < CsRTSentSize; i++) CsRTFreeEvent(&CsRTSent[i]);
	CsRTSentSize = 0L;
	CsRTStopAll();
	return;
	}


long CsRTDate(long* p_end) {
	// Date of the performance (milliseconds) for the "_time" file, and expected end in *p_end
	UInt64 now;
	*p_end = (long)(CsRTLastEnd * 1000.);
	if(!CsRTStarted) return 0L;
	now = getClockTime();
	if(now <= CsRTOrigin) return 0L;
	return (long)((now - CsRTOrigin) / 1000L);
	}
