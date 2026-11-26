// TimeBase.c (BP3)

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

// Time base (2026-11-25)
// A time base ("-tb" file, option "-tb FILE") superposes ticks to the performance of each item, as in BP2, where they were sent
// to real-time MIDI only (see source/not_used/Ticks.c). They are now also written to MIDI files.
// The file is in JSON format (see tb_data_to_json() in php/_basic_tasks.php): a list of tracks, each with a key, a MIDI channel,
// a velocity, a cycle of 1 to 40 beats, a speed ratio Ptick/Qtick, the duration of a tick (ms) and the beats of the cycle having a tick.
// Beat n of a track is at n * Qtick / Ptick beats of the item from its start. It has a tick if the value of position (n modulo the cycle) is 1.
// The file sets the metronome of the time base, "Pclock" beats in "Qclock" seconds (2026-11-26), relative to 60 beats/mn: its beats are
// those of the metronome of the project multiplied by Pclock / Qclock, and follow the tempo of the item. E.g. 3 beats in 1 second: ticks
// 3 times faster than the beats of the project, every 333 ms at 60 beats/mn and 222 ms at 90 beats/mn (see TbRatio()).
// Without it (files saved before), the metronome of the time base is 1 beat in 1 second.
// A beat of the item is a symbol at its current tempo: 1000 * Pclock / Qclock ms (the metronome) divided by the tempo set by _tempo(),
// '/' or '\' at the top level of the item (e.g. "S --> _tempo(0.8) Waves": 1250 ms at 60 beats per minute). TimeBaseSetStructure() makes
// the tempo map from the work string after PolyMake(), which writes tempos as absolute markers '*scale /speed'. Inside polymetric
// expressions, markers also make rhythms, so only the tempos of the top level and those at the start of the first field of its
// polymetric expressions, at its start or before a note, are used (see TbWalk()). E.g. a measure imported from MusicXML "{_tempo(18/5) {3, ...}, _tempo(18/5) ...}".
// There are no ticks in smooth time, even with a metronome.
// MakeSound() calls TimeBaseStartItem() at the start of each performance of an item, then SendToDriver() calls TimeBaseSendUntil()
// before each MIDI event, so that ticks are sent in chronological order with the events of the item, which MIDI files require.
// TimeBaseEndItem() sends the remaining ticks up to the end of the item. A tick is not longer than the item, nor than its beat.
// No tick is played at date 0, where it would only blur the attack of the first note.
// In real-time Csound, SuperCollider and OSC (CsoundRT), ticks are sent to the MIDI output (option --rtmidi), queued at once for each item
// by TimeBaseStartItem() with CsRTMidiTick(); option --rt-dump writes them as "MIDI NoteOn" lines.
// With a time base, the vertical lines of the graphics show the ticks rather than beats (TimeBaseGraphicTicks(), DrawItemBackground()),
// in blue as the beats: the ticks of the first track (the upper track on timebase.php) as solid lines with a bar on top, like a "T",
// those of the other tracks as dotted lines. Without a time base, the blue lines of the beats follow the same tempo map (TimeBaseGraphicBeats()), as the ticks of -tb.onebeat.

#ifndef _H_BP3
#include "-BP3.h"
#endif

#include "-BP3decl.h"

#define TB_MAXTRACKS 32
#define TB_MAXBEATS 40

typedef struct {
	int key,channel,velocity,cycle,ptick,qtick;
	int upper;			// TRUE for the first track of the file (the upper track on timebase.php), whose lines are solid in the graphics
	long duration;
	char ticks[TB_MAXBEATS];
	// Performance
	int on;				// FALSE if its beats are too short to be played
	long n;				// Index of the next beat
	Milliseconds off;	// Date of the pending NoteOff, or -1
	} TimeBaseTrack;

static TimeBaseTrack TBtrack[TB_MAXTRACKS];
static int TBtracks = 0;		// Number of tracks with at least one tick
static int TBactive = FALSE;	// Between TimeBaseStartItem() and TimeBaseEndItem()
static int TBsending = FALSE;	// Avoids the recursion of SendToDriver() -> TimeBaseSendUntil() -> SendToDriver()
static Milliseconds TBorigin,TBend;
static int TBwarned = FALSE;
static int TBmute = FALSE;		// Option --mute-ticks: ticks are not played ("Play ticks" unchecked on the project page)
static Milliseconds TBitemend = -1L;	// End of the item being played (TimeBaseSetItemEnd()), for the graphics
static unsigned long TBmaxseq = 0L;	// 2026-11-26 Last column of the phase diagram of the item (TimeBaseSetItemEnd()), 0 if unknown
static double TBminbeat;		// Shortest beat of the item (ms)
// Tempo map of the item (TimeBaseSetStructure()): segment k starts at beat TBmapbeat[k] of the item and date TBmapdate[k] (ms),
// with beats of TBmapms[k] ms. Without a map, beats are those of the metronome. The last segment goes on until the end of the item.
static double *TBmapbeat = NULL,*TBmapdate = NULL,*TBmapms = NULL;
static long TBmapsize = 0L,TBmapmax = 0L;
static double TBpclock = 1.,TBqclock = 1.;	// 2026-11-26 Metronome of the time base: TBpclock beats in TBqclock seconds

static int TbNumber(cJSON *track,const char *key,int deft,int min,int max) {
	// Value of the item of 'track' whose "key" is 'key', within limits, or 'deft' if not found
	cJSON *item,*itemkey,*value;
	double x;
	x = deft;
	cJSON_ArrayForEach(item,track) {
		if(!cJSON_IsObject(item)) continue;
		itemkey = cJSON_GetObjectItem(item,"key");
		if(!cJSON_IsString(itemkey) || strcmp(itemkey->valuestring,key) != 0) continue;
		value = cJSON_GetObjectItem(item,"value");
		if(cJSON_IsNumber(value)) x = value->valuedouble;
		else if(cJSON_IsString(value) && value->valuestring[0] != '\0') x = atof(value->valuestring);
		break;
		}
	if(x < min) x = min;
	if(max > min && x > max) x = max;
	return((int) x);
	}

static cJSON *TbValue(cJSON *object,const char *key) {
	// "value" of the item of 'object' whose "key" is 'key', or NULL
	cJSON *item,*itemkey;
	cJSON_ArrayForEach(item,object) {
		if(!cJSON_IsObject(item)) continue;
		itemkey = cJSON_GetObjectItem(item,"key");
		if(cJSON_IsString(itemkey) && strcmp(itemkey->valuestring,key) == 0)
			return(cJSON_GetObjectItem(item,"value"));
		}
	return(NULL);
	}

static double TbClock(cJSON *json,const char *key) {
	// Value of "Pclock" or "Qclock" of the time base (its metronome), or 0 if not set
	cJSON *value;
	value = TbValue(json,key);
	if(cJSON_IsNumber(value)) return(value->valuedouble);
	if(cJSON_IsString(value) && value->valuestring[0] != '\0') return(atof(value->valuestring));
	return(0.);
	}

int LoadTimeBase(const char *path) {
	// Reads a time base file in JSON format. Tracks without any tick are ignored.
	// A time base that cannot be read is reported, and the work goes on without ticks: it returns OK in all cases but ABORT (memory).
	FILE *fin;
	long size;
	char *text,*p;
	cJSON *json,*tracks,*track,*ticks,*tick;
	int i,itrack,any,ignored;
	TimeBaseTrack *t;

	TBtracks = 0;
	fin = my_fopen(0,path,"rb");
	if(fin == NULL) {
		BPPrintMessage(0,odError,"=> Could not open time base file ‘%s’. No ticks will be played\n",path);
		return(OK);
		}
	fseek(fin,0L,SEEK_END);
	size = ftell(fin);
	fseek(fin,0L,SEEK_SET);
	text = (char*) malloc((size_t) size + 1);
	if(text == NULL) {
		my_fclose(fin);
		return(ABORT);
		}
	size = (long) fread(text,1,(size_t) size,fin);
	text[size] = '\0';
	my_fclose(fin);
	p = text;
	if((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB && (unsigned char)p[2] == 0xBF) p += 3; // UTF-8 byte order mark
	while(isspace((unsigned char)(*p))) p++;
	if(*p != '{') {
		free(text);
		BPPrintMessage(0,odError,"=> Time base file ‘%s’ is in the old format. Open it in the PHP interface to convert it to JSON. No ticks will be played\n",path);
		return(OK);
		}
	json = cJSON_Parse(p);
	free(text);
	if(json == NULL) {
		BPPrintMessage(0,odError,"=> Time base file ‘%s’ is not valid JSON. No ticks will be played\n",path);
		return(OK);
		}
	tracks = TbValue(json,"Tracks");
	if(!cJSON_IsArray(tracks)) {
		cJSON_Delete(json);
		BPPrintMessage(0,odError,"=> No tracks in time base file ‘%s’. No ticks will be played\n",path);
		return(OK);
		}
	TBpclock = TbClock(json,"Pclock"); // 2026-11-26 Metronome of the time base, if set
	TBqclock = TbClock(json,"Qclock");
	if(TBpclock <= 0. || TBqclock <= 0.) TBpclock = TBqclock = 1.; // Not set in files saved before 2026-11-26: 1 beat in 1 second
	itrack = ignored = 0;
	cJSON_ArrayForEach(track,tracks) {
		itrack++;
		if(TBtracks >= TB_MAXTRACKS) {
			ignored++;
			continue;
			}
		t = &(TBtrack[TBtracks]);
		t->upper = (itrack == 1);
		t->key = TbNumber(track,"TickKey",60,0,127);
		t->channel = TbNumber(track,"TickChannel",1,1,16);
		t->velocity = TbNumber(track,"TickVelocity",64,0,127);
		t->cycle = TbNumber(track,"TickCycle",4,1,TB_MAXBEATS);
		t->ptick = TbNumber(track,"Ptick",1,1,0);
		t->qtick = TbNumber(track,"Qtick",1,1,0);
		t->duration = TbNumber(track,"TickDuration",50,1,0);
		ticks = TbValue(track,"Ticks");
		any = FALSE;
		for(i = 0; i < TB_MAXBEATS; i++) t->ticks[i] = FALSE;
		if(cJSON_IsArray(ticks)) {
			i = 0;
			cJSON_ArrayForEach(tick,ticks) {
				if(i >= TB_MAXBEATS) break;
				// A tick is on if its value is odd, as in BP2 and tb_check_track()
				if(cJSON_IsNumber(tick) && (((long) tick->valuedouble) % 2L) != 0) t->ticks[i] = TRUE;
				if(i < t->cycle && t->ticks[i]) any = TRUE;
				i++;
				}
			}
		if(any && t->velocity > 0) TBtracks++;
		}
	cJSON_Delete(json);
	if(ignored > 0) BPPrintMessage(0,odWarning,"=> Only the first %d tracks of the time base are played\n",TB_MAXTRACKS);
	BPPrintMessage(0,odInfo,"Time base ‘%s’: %d track(s) with ticks out of %d\n",path,TBtracks,itrack);
	return(OK);
	}

static double TbRatio(TimeBaseTrack *t) {
	// Number of beats of the item between two beats of track t: Qtick / Ptick, times the period of the metronome of the time base
	// (TBqclock / TBpclock seconds) over 1 second. The metronome of the time base is relative to 60 beats/mn, so that both it and the metronome
	// of the project (whose beats are the beats of the item at tempo 1) set the spacing of ticks
	double ratio;
	ratio = ((double) t->qtick) / t->ptick;
	if(TBpclock > 0. && TBqclock > 0.) ratio *= TBqclock / TBpclock;
	return(ratio);
	}

static double TbRealDate(double date) {
	// 2026-11-26 Date at which the performance plays a position of the item whose ideal date is 'date' (ms). TimeSet() places notes on the columns
	// of the phase diagram, whose dates (*p_T)[] include the correction of the duration (CorrectionFactor) and, with compression (Kpress > 1),
	// round positions to a column, up to about 10 ms later, even the start of the item (e.g. "_tempo(1.3) C4 D4 {E4 F4 _tempo(2.7) G4 F4}", Kpress = 53):
	// ticks take the date of the column of their position, as the notes do (see TimeSet.c). MIDI files are shifted by the date of the first note.
	// Position 'in' is counted in units of 1 / Ratio beat of the metronome from 1, as in TimeSet(). It is rounded when it is a whole number
	// but for rounding errors (which put some ticks of -gr.Mozart 45 ms before their note), and a fractional position is interpolated between the columns of its two positions.
	double in,metronome,f;
	unsigned long j,j2;

	if(date < 0.) date = 0.; // The start of the item is the date of its first column, which may be later than 0 with compression
	if(p_T == NULL || TBmaxseq == 0L || Nature_of_time != STRIATED || Ratio < 1. || Kpress < 1. || Pclock < 1. || Qclock < 1.) return(date);
	metronome = 1000. * Pclock / Qclock;
	in = 1. + date * Ratio / metronome;
	if(fabs(in - floor(in + 0.5)) < 1e-6) in = floor(in + 0.5);
	j = Class(in);
	if(j > TBmaxseq) return(((double)(*p_T)[TBmaxseq]) + (date - ((double)TBmaxseq * Kpress * metronome) / Ratio) * CorrectionFactor); // Beyond the end of the item
	f = in - floor(in);
	if(f > 0.) { // 2026-11-26 Between two positions (e.g. 8 ticks per beat in "C4 _ _ _", whose columns are 1 beat apart): interpolated between their columns, otherwise all ticks of a beat took its date
		j2 = Class(floor(in) + 1.);
		if(j2 != j && j2 <= TBmaxseq) return(((double)(*p_T)[j]) + f * ((double)(*p_T)[j2] - (double)(*p_T)[j]));
		}
	return((double)(*p_T)[j]);
	}

static double TbBeatToDate(double beat) {
	// Date (ms from the start of the item) of a beat of the item, which may be fractional
	long k,lo,hi;
	double metronome;

	metronome = 1000. * Pclock / Qclock;
	if(TBmapsize == 0L) return(TbRealDate(beat * metronome));
	if(beat <= 0.) return(TbRealDate(0.));
	lo = 0L; hi = TBmapsize - 1L; // Last segment starting at or before 'beat'
	while(lo < hi) {
		k = (lo + hi + 1L) / 2L;
		if(TBmapbeat[k] <= beat) lo = k;
		else hi = k - 1L;
		}
	return(TbRealDate(TBmapdate[lo] + (beat - TBmapbeat[lo]) * TBmapms[lo]));
	}

static double TbMinBeat(void) {
	// Shortest beat of the item (ms), to ignore tracks whose ticks would be too dense
	long k;
	double x,min;

	if(TBmapsize == 0L) return(1000. * Pclock / Qclock);
	min = TBmapms[0];
	for(k = 1L; k < TBmapsize; k++) {
		x = TBmapms[k];
		if(x < min) min = x;
		}
	return(min);
	}

int TimeBaseStartItem(Milliseconds origin,Milliseconds end) {
	// Called by MakeSound() at the start of each performance of an item. 'origin' is the absolute date of date 0 of the item,
	// 'end' the date of its end. Ticks are sent to MIDI files and real-time MIDI. In real-time Csound, SuperCollider and OSC,
	// they are sent to the MIDI output (option --rtmidi): all ticks of the item are queued at once (see CsRTMidiTick()).
	int itrack,result;
	TBactive = FALSE;
	if(TBmute || TBtracks == 0 || Nature_of_time == SMOOTH || Pclock < 1. || Qclock < 1.) return(OK); // 2026-11-25 No ticks in smooth time, even with a metronome
	if(CsoundRT) {
		if(!CsRTMidiTicksOn()) return(OK);
		}
	else if(!MIDIfileOn && !rtMIDI) return(OK);
	if(ItemCapture || PlayPrototypeOn || end <= origin) return(OK);
	TBorigin = origin;
	TBend = end;
	TBminbeat = TbMinBeat();
	for(itrack = 0; itrack < TBtracks; itrack++) {
		TBtrack[itrack].n = 1L; // 2026-11-25 No tick at date 0: played with the first note of the item, it would only blur its attack
		TBtrack[itrack].off = -1L;
		// A track whose beats are shorter than 5 ms (extreme speed ratio) would fill the MIDI output with ticks
		TBtrack[itrack].on = (TBminbeat * TbRatio(&(TBtrack[itrack])) >= 5.);
		if(!TBtrack[itrack].on && !TBwarned) {
			BPPrintMessage(0,odWarning,"=> Track %d of the time base is ignored: its beats would be shorter than 5 ms\n",itrack + 1);
			TBwarned = TRUE;
			}
		}
	TBactive = TRUE;
	if(CsoundRT) {
		result = TimeBaseSendUntil(end,NULL);
		TBactive = FALSE;
		return(result);
		}
	return(OK);
	}

static int TbSend(TimeBaseTrack *t,int velocity,Milliseconds date,int *p_rs) {
	// A tick (NoteOn) or its end (NoteOn with velocity 0)
	if(CsoundRT) {
		CsRTMidiTick(t->key,t->channel - 1,velocity,((double) date) / 1000.); // Dates of the score are in seconds
		return(OK);
		}
	return(SendMIDIevent(NORMAL_EVENT,NoteOn + t->channel - 1,t->key,velocity,TB_TICK_INSTANCE,-1,0,date,0,p_rs)); // See sendMIDIEvent() for TB_TICK_INSTANCE
	}

static Milliseconds TbBeatDate(TimeBaseTrack *t,long n) {
	// Absolute date of beat n of track t, computed from the origin to avoid an accumulation of rounding errors
	return(TBorigin + (Milliseconds) floor(0.5 + TbBeatToDate((double) n * TbRatio(t))));
	}

int TimeBaseSendUntil(Milliseconds time,int *p_rs) {
	// Sends the ticks and their NoteOffs whose dates are not later than 'time', in chronological order. Called by SendToDriver()
	int itrack,inext,isoff,result;
	Milliseconds date,next,limit;
	TimeBaseTrack *t;

	if(!TBactive || TBsending) return(OK);
	TBsending = TRUE;
	result = OK;
	while(TRUE) {
		// Find the earliest event among NoteOffs and next ticks. At the same date, a NoteOff comes first.
		inext = -1; isoff = FALSE; next = 0L;
		for(itrack = 0; itrack < TBtracks; itrack++) {
			t = &(TBtrack[itrack]);
			if(!t->on) continue;
			if(t->off >= 0L && (inext < 0 || t->off < next || (t->off == next && !isoff))) {
				inext = itrack; isoff = TRUE; next = t->off;
				}
			while(!t->ticks[t->n % t->cycle] && TbBeatDate(t,t->n) < TBend) t->n++; // Skip beats without a tick
			date = TbBeatDate(t,t->n);
			if(date < TBend && (inext < 0 || date < next)) {
				inext = itrack; isoff = FALSE; next = date;
				}
			}
		if(inext < 0 || next > time) break;
		t = &(TBtrack[inext]);
		if(isoff) {
			result = TbSend(t,0,next,p_rs);
			t->off = -1L;
			}
		else {
			if(t->off >= 0L) { // A tick longer than its beat ends before the next one
				result = TbSend(t,0,next,p_rs);
				t->off = -1L;
				if(result != OK) break;
				}
			result = TbSend(t,t->velocity,next,p_rs);
			limit = next + t->duration;
			if(limit > TBend) limit = TBend;
			if(limit <= next) limit = next + 1L;
			t->off = limit;
			t->n++;
			}
		if(result != OK) break;
		}
	TBsending = FALSE;
	return(result);
	}

int TimeBaseEndItem(int *p_rs) {
	// Called by MakeSound() at the end of each performance of an item: sends the ticks remaining before its end
	int result;
	if(!TBactive) return(OK);
	result = TimeBaseSendUntil(TBend,p_rs);
	TBactive = FALSE;
	return(result);
	}

void TimeBaseCancel(void) {
	// The performance was interrupted: no more ticks
	TBactive = FALSE;
	}

void TimeBaseMute(int mute) {
	// Option --mute-ticks
	TBmute = mute;
	}

void TimeBaseSetItemEnd(Milliseconds tmax,unsigned long maxseq) {
	// Called by PlayBuffer1() after TimeSet(): end of the item (ms), after which no tick is played or drawn, and last column of its phase diagram
	TBitemend = tmax;
	TBmaxseq = maxseq;
	}

static int TbMapAdd(double beat,double date,double ms) {
	// Adds a segment to the tempo map, or changes the last one if it starts at the same beat
	double *x;
	long newmax;

	if(TBmapsize > 0L && TBmapms[TBmapsize - 1L] == ms) return(OK); // Same tempo: the segment goes on
	if(TBmapsize > 0L && TBmapbeat[TBmapsize - 1L] >= beat) {
		TBmapms[TBmapsize - 1L] = ms;
		return(OK);
		}
	if(TBmapsize >= TBmapmax) {
		newmax = TBmapmax + 64L;
		if((x = (double*) realloc(TBmapbeat,newmax * sizeof(double))) == NULL) return(ABORT);
		TBmapbeat = x;
		if((x = (double*) realloc(TBmapdate,newmax * sizeof(double))) == NULL) return(ABORT);
		TBmapdate = x;
		if((x = (double*) realloc(TBmapms,newmax * sizeof(double))) == NULL) return(ABORT);
		TBmapms = x;
		TBmapmax = newmax;
		}
	TBmapbeat[TBmapsize] = beat;
	TBmapdate[TBmapsize] = date;
	TBmapms[TBmapsize] = ms;
	TBmapsize++;
	return(OK);
	}

static double TbMarkerValue(tokenbyte **p_buff,long i) {
	// Value of a marker T0 x T1 a T1 b at position i: a * TOKBASE + b, or 0 if the format is not the expected one
	if((*p_buff)[i+2] != T1 || (*p_buff)[i+4] != T1) return(0.);
	return(((double)(*p_buff)[i+3]) * ((double) TOKBASE) + (double)(*p_buff)[i+5]);
	}

static int TbPush(double **p_stack,long *p_size,long *p_max,double x) {
	double *stack;
	if((*p_size) >= (*p_max)) {
		if((stack = (double*) realloc(*p_stack,((*p_max) + 64L) * sizeof(double))) == NULL) return(ABORT);
		(*p_stack) = stack;
		(*p_max) += 64L;
		}
	(*p_stack)[(*p_size)++] = x;
	return(OK);
	}

static double TbWalk(tokenbyte **p_buff,int probe) {
	// Follows the work string after PolyMake(). Speed markers are absolute: '*' (T0 21) and '**' (T0 24) set the scale, '/' (T0 11)
	// and '\' (T0 25) the speed, and a symbol lasts scale / speed metronome beats. Inside polymetric expressions, markers also make
	// rhythms ('/3', beat separators...), so the tempo of the item is set only:
	// - at the top level, by the markers before a sound-object or note. Silences ('-' and '_') keep the current tempo, so that a
	//   fractional silence such as "1/2", written '*a /2b -', doesn't change it;
	// - for a polymetric expression of the top level, by the markers at the start of its first field (e.g. a measure
	//   "{_tempo(18/5) {3, ...}, _tempo(18/5) ...}" imported from MusicXML), before its first symbol or expression, and (2026-11-26)
	//   by the markers before a note inside this first field, e.g. "{E4 F4 _tempo(2.7) G4 F4}", so that ticks still follow the notes
	//   after the '}'. Markers inside deeper expressions are rhythms and are ignored.
	// The duration of a polymetric expression is that of its first field, which is followed; its other fields are skipped.
	// With 'probe', returns the duration (ms) of the first beat of the item, or 0 if none. Otherwise makes the tempo map, starting
	// with the duration of beats TBmapms[0] set by the probe, and returns the duration of the item (ms), or -1 on error.
	long i,depth,skip,stacksize,stackmax;
	tokenbyte m,p;
	double scale,speed,x,ms,unit,beat,date,metronome,result;
	double *stack;
	int firstfield;

	metronome = 1000. * Pclock / Qclock;
	unit = probe ? 0. : TBmapms[0]; // Duration of a beat of the item (ms)
	TBmapsize = 0L;
	stack = NULL; stacksize = stackmax = 0L;
	scale = speed = 1.;
	beat = date = 0.;
	result = probe ? 0. : -1.;
	firstfield = FALSE; // TRUE at the start of the first field of a polymetric expression of the top level
	depth = skip = 0L; // 'skip' is the depth of the expression whose other fields are being skipped, or 0
	for(i = 0L; ; i += 2L) {
		m = (*p_buff)[i]; p = (*p_buff)[i+1];
		if(m == TEND && p == TEND) {
			if(!probe) result = date;
			break;
			}
		if(m == T0) {
			if(p == 21 || p == 24 || p == 11 || p == 25) {
				x = TbMarkerValue(p_buff,i);
				i += 4L;
				if(x <= 0. || skip > 0L) continue;
				if(p == 21) scale = x;
				else if(p == 24) scale = 1. / x;
				else if(p == 11) speed = x;
				else speed = 1. / x;
				continue;
				}
			if(p == 12) { // '{'
				if(skip == 0L && firstfield) { // An expression starting the first field of an expression of the top level
					unit = metronome * scale / speed;
					firstfield = FALSE;
					if(probe) break;
					}
				if(depth == 0L) firstfield = TRUE;
				depth++;
				if(skip > 0L) continue;
				if(TbPush(&stack,&stacksize,&stackmax,scale) != OK || TbPush(&stack,&stacksize,&stackmax,speed) != OK) {
					result = probe ? 0. : -1.;
					break;
					}
				continue;
				}
			if(p == 14) { // ',': the other fields of this expression are skipped
				if(skip == 0L) skip = depth;
				firstfield = FALSE;
				continue;
				}
			if(p == 13) { // '}'
				if(skip == depth) skip = 0L;
				depth--;
				firstfield = FALSE;
				if(skip == 0L && stacksize >= 2L) {
					speed = stack[--stacksize]; scale = stack[--stacksize];
					}
				continue;
				}
			continue;
			}
		if(skip > 0L) continue;
		if(m != T25 && m != T3 && m != T4) continue; // Out-time objects, synchronisation tags, performance controls... take no time
		ms = metronome * scale / speed; // Duration of this symbol
		if(firstfield || (depth <= 1L && (m == T25 || m == T4 || p > 1))) { // 2026-11-26 Also in the first field of an expression of the top level, e.g. {E4 F4 _tempo(2.7) G4 F4}
			unit = ms;
			firstfield = FALSE;
			if(probe) {
				result = unit;
				break;
				}
			}
		if(probe) continue;
		if(TbMapAdd(beat,date,unit) != OK) break;
		beat += ms / unit;
		date += ms;
		}
	if(probe && result <= 0. && unit > 0.) result = unit;
	if(stack != NULL) free(stack);
	return(result);
	}

void TimeBaseSetStructure(tokenbyte **p_buff) {
	// Called by PlayBuffer1() after PolyMake(): makes the tempo map of the item (see TbWalk())
	double first;

	// Also made without a time base: the beats of the graphics follow it (TimeBaseGraphicBeats())
	TBmapsize = 0L;
	TBmaxseq = 0L; // Set by TimeBaseSetItemEnd() after TimeSet()
	if(p_buff == NULL || Nature_of_time == SMOOTH || Pclock < 1. || Qclock < 1.) return;
	first = TbWalk(p_buff,TRUE);
	if(first <= 0.) first = 1000. * Pclock / Qclock; // No symbol: beats of the metronome
	TBmapsize = 0L;
	if(TbMapAdd(0.,0.,first) != OK) return; // TBmapms[0] is the first beat
	if(TbWalk(p_buff,FALSE) <= 0.) TBmapsize = 0L;
	}

int TimeBaseGraphicBeats(Milliseconds tmax,Milliseconds **p_date) {
	// Dates (ms from date 0 of the item) of the beats of the item before 'tmax', following its tempo map as the ticks of a time base
	// whose single track has a tick on every beat (e.g. -tb.onebeat), for the vertical lines of the graphics without a time base
	// (DrawItemBackground()). Beat 0 is included. Returns their number, or 0 if there is no tempo map (smooth time, no metronome...),
	// in which case the graphics show the beats of the phase diagram. The array is allocated with malloc() and freed by the caller.
	long k,max;
	double date;
	Milliseconds *dates,*d;

	(*p_date) = NULL;
	if(TBmapsize == 0L || Nature_of_time == SMOOTH || Pclock < 1. || Qclock < 1. || tmax <= 0L) return(0);
	if(TbMinBeat() < 5.) return(0);
	dates = NULL; max = 0L;
	for(k = 0L; k <= 100000L; k++) {
		date = floor(0.5 + TbBeatToDate((double) k));
		if(date >= tmax) break;
		if(k >= max) {
			max = (max < 256L) ? 256L : 2L * max;
			if((d = (Milliseconds*) realloc(dates,(size_t) max * sizeof(Milliseconds))) == NULL) {
				free(dates);
				return(0);
				}
			dates = d;
			}
		dates[k] = (Milliseconds) date;
		}
	if(k == 0L) {
		free(dates);
		return(0);
		}
	(*p_date) = dates;
	return((int) k);
	}

int TimeBaseGraphicTicks(Milliseconds tmax,Milliseconds **p_date,int **p_upper) {
	// Dates (ms from date 0 of the item) of the ticks played before 'tmax', and of a tick at the end of the item, in chronological order, for the vertical lines of the graphics
	// (DrawItemBackground()). Simultaneous ticks are listed once. (*p_upper)[i] is TRUE if tick i belongs to the first track of the file
	// (the upper track on timebase.php), drawn as a solid line with a bar on top, the ticks of other tracks being dotted lines.
	// Returns their number, or 0 if there is no time base. The arrays are allocated with malloc() and freed by the caller.
	int itrack,first,upper;
	long n,count,max,tn[TB_MAXTRACKS];
	Milliseconds date,next;
	Milliseconds *dates;
	int *uppers;
	double beat;
	TimeBaseTrack *t;

	(*p_date) = NULL; (*p_upper) = NULL;
	if(TBitemend > 0L && TBitemend <= tmax) tmax = TBitemend + 3L; // 2026-11-26 A tick at the end of the item (within 2 ms) is drawn although it is not played, closing its last beat
	if(TBtracks == 0 || TBmute || Nature_of_time == SMOOTH || Pclock < 1. || Qclock < 1. || tmax <= 0L) return(0); // Muted ticks ("Play ticks" unchecked): the graphics show the beats
	beat = TbMinBeat();
	count = max = 0L;
	dates = NULL; uppers = NULL;
	for(itrack = 0; itrack < TBtracks; itrack++) tn[itrack] = 0L; // 2026-11-26 The tick at date 0 is drawn although it is not played (see TimeBaseStartItem()). The beats of the performance (TBtrack[].n) are not changed
	while(TRUE) {
		first = TRUE; next = 0L; upper = FALSE;
		for(itrack = 0; itrack < TBtracks; itrack++) {
			t = &(TBtrack[itrack]);
			if(beat * TbRatio(t) < 5.) continue; // Ignored as in TimeBaseStartItem()
			while(TRUE) {
				date = (Milliseconds) floor(0.5 + TbBeatToDate((double) tn[itrack] * TbRatio(t)));
				if(date >= tmax || t->ticks[tn[itrack] % t->cycle]) break;
				tn[itrack]++;
				}
			if(date < tmax && (first || date < next)) {
				first = FALSE; next = date;
				}
			}
		if(first) break;
		for(itrack = 0; itrack < TBtracks; itrack++) { // Every track with a tick at this date moves to its next beat
			t = &(TBtrack[itrack]);
			n = tn[itrack];
			if((Milliseconds) floor(0.5 + TbBeatToDate((double) n * TbRatio(t))) == next) {
				if(t->upper && t->ticks[n % t->cycle] && beat * TbRatio(t) >= 5.) upper = TRUE;
				tn[itrack]++;
				}
			}
		if(count >= max) {
			max = (max < 256L) ? 256L : 2L * max;
			Milliseconds *d = (Milliseconds*) realloc(dates,(size_t) max * sizeof(Milliseconds));
			if(d != NULL) dates = d;
			int *q = (int*) realloc(uppers,(size_t) max * sizeof(int));
			if(q != NULL) uppers = q;
			if(d == NULL || q == NULL) {
				free(dates); free(uppers);
				return(0);
				}
			}
		dates[count] = next;
		uppers[count] = upper;
		count++;
		if(count > 100000L) break;
		}
	(*p_date) = dates; (*p_upper) = uppers;
	return((int) count);
	}
