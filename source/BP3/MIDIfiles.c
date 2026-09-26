/* MIDIfiles.c (BP3) */
/* Thanks to Srikumar K. Subramanian */

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


#ifndef _H_BP3
#include "-BP3.h"
#endif

#include "-BP3decl.h"
int check_fade_out = 0;

#define  BITS0_6    0x7F
#define  BITS7_13   0x3F80
#define  BITS14_20  0x1FC000
#define  BITS21_27  0xFE00000

// A few predefined things

byte MTrk[] = { 'M', 'T', 'r', 'k' };

byte header1[] = {
0x4D, 0x54, 0x68, 0x64, /* MThd */
0x00, 0x00, 0x00, 0x06, /* 6 bytes in header chunk */
0x00};

byte header2[] = {0x00};

byte header3[] = {0x03, 0xE8};	/* = 1000 */
/* Delta-times are 1/1000th quarter-note */

byte key_sig[] = {0x00, 0xff, 0x59, 0x02, 0x00, 0x00};
/* key of C, major key */

byte time_sig[] = {0x00, 0xff, 0x58, 0x04, 0x01, 0x02, 0x18, 0x08};
/* 0x00, 0xff, 0x58, 0x04 identifies time_sig */
/* 0x01 = 1 beat per measure */
/* 0x02 = quarter-notes */
/* 0x18 = 24 clocks in metronome tick */
/* 0x08 = number of 32-th notes in a quarter-note (24 clocks) */

byte tempo_sig[] = {0x00, 0xff, 0x51, 0x03};

byte end_of_track[] = {0x00, 0xff, 0x2f, 0x00};

static int WriteMIDIFileHeader(FILE* fout);
static int WriteBeginningOfTrack(FILE* fout, int inclMetaEvts, int isTempoOnlyTrk);
static int WriteEndOfTrack(FILE* fout);
static int WriteRawBytes(FILE* fout, byte* data, size_t numbytes);
static int WriteVarLenQuantity(FILE* fout, dword value, dword *tracklen);
static int CloseMIDIFile2(void);

int trace_writing_midi_file = 0;

int MakeMIDIFile(OutFileInfo* finfo) {
	int result;
	FILE *fout;
	result = MISSED;
	char thename[MAXNAME];
	if(OpenMIDIfilePtr != NULL) {
		if(!Create_set) {
			BPPrintMessage(0,odError, "MIDI file is already open: %s\n", finfo->name);
			return OK;
			}
		else CloseMIDIFile();
		}
	if(Create_set) {
		snprintf(thename,sizeof(thename),"%d.mid",SetMidiFileNr);
		change_midifile_name(thename);
		}
	// BPPrintMessage(1,odInfo,"Creating new MIDI file...\n");
	fout = OpenOutputFile(finfo,"wb");
	if(!fout) {
		BPPrintMessage(0,odError, "=> Could not open file for MIDI %s\n", finfo->name);
		return MISSED;
		}
	else {
		OpenMIDIfilePtr = fout;
		MIDIfileOpened = MIDIfileTrackEmpty = TRUE;
		MIDIfileTrackNumber = 0;
		result = WriteMIDIFileHeader(fout);
		if(result == OK) {
			if(MIDIfileType == 1) {
				// write the tempo track for the whole file
				result = WriteBeginningOfTrack(fout, TRUE, TRUE);
				if(result == OK) {
					// start a new track for MIDI events without meta info
					result = WriteBeginningOfTrack(fout, FALSE, FALSE);
					}
				}
			else {
				// For both type 0 & type 2 files,
				// write tempo/meta info & events into the same track.
				result = WriteBeginningOfTrack(fout, TRUE, FALSE);
				}
			if(result == OK) WriteMIDIorchestra();
			}
		}
	if(result != OK) CloseMIDIFile2();
	return result;
	}

void change_midifile_name(char *tail) {
	const char *base = PathToMidiFile;
    size_t base_len = strlen(base);
    int need_slash = (base_len > 0 && base[base_len - 1] == '/') ? 0 : 1;
    size_t total = base_len + (need_slash ? 1 : 0) + strlen(tail) + 1;
    char *combined = (char *)malloc(total);
    char *p = combined;
    memcpy(p, base, base_len); p += base_len;
    if (need_slash) *p++ = '/';
    memcpy(p, tail, strlen(tail) + 1); // includes '\0'
	// BPPrintMessage(1,odInfo,"\ncombined = %s\n",combined);
    // If you previously malloc'd the old value, free it here.
    // Otherwise skip the free (e.g., if it pointed at a literal).
    // free(gOptions.outputFiles[ofiMidiFile].name);
    gOptions.outputFiles[ofiMidiFile].name = combined;
	}

static int WriteMIDIFileHeader(FILE* fout)
{
	int result;
	byte byteval,b0,b1;
	unsigned long deltatimesinquarternote;

	/* Write the MThd block to the new file */
	result = WriteRawBytes(fout, header1, sizeof(header1));
	if(result != OK)  return ABORT;

	/* Write the file type */
	byteval = (byte) (MIDIfileType & 0xff);
	result = WriteRawBytes(fout, &byteval, 1L);
	if(result != OK)  return ABORT;

	result = WriteRawBytes(fout, header2, sizeof(header2));
	if(result != OK)  return ABORT;

	/* Write number of tracks */
	/* This is a temporary value since type 1 & 2 files may contain more tracks */
	byteval = (byte) (1 & 0xff);
	result = WriteRawBytes(fout, &byteval, 1L);
	if(result != OK)  return ABORT;

	/* Write division of quarter-note */
	if(Pclock > 0.)
		deltatimesinquarternote = (Pclock * 1000L) / Qclock;
	else
		deltatimesinquarternote = 1000L;
	b0 = (byte) (deltatimesinquarternote & 0xff);
	b1 = (byte) ((deltatimesinquarternote >> 8) & 0xff);
	result = WriteRawBytes(fout, &b1, 1L);
	if(result != OK)  return ABORT;
	result = WriteRawBytes(fout, &b0, 1L);
	if(result != OK)  return ABORT;

	return OK;
}


/*	WriteBeginningOfTrack()
	Write the beginning of a new track chunk.
	Parameters:
	  inclMetaEvts - TRUE if track should include Time Sign., Tempo, & Key Sign. events
	  isTempoOnlyTrk - TRUE if track is the "tempo track" for a Type-1 MIDI file; only
		  meta events will be written to the track followed by an "End of Track" event.
 */
static int WriteBeginningOfTrack(FILE* fout, int inclMetaEvts, int isTempoOnlyTrk) {
	int i, result;
	dword length;
	byte b0,b1,b2;
	unsigned long quarternotedur;
	
	if(isTempoOnlyTrk && !inclMetaEvts) {
		// this function does not support writing an empty track without meta events
		BPPrintMessage(0,odError, "=> Error in WriteBeginningOfTrack(): inconsistent parameters\n");
		return ABORT;
		}
	Midi_msg = ZERO;
	OldMIDIfileTime = -1L;
	MIDItracklength = ZERO;
	MIDIbytestate = 0;
	MIDIfileTrackNumber++;
	MIDIfileTrackEmpty = TRUE;
	
	result = WriteRawBytes(fout, MTrk, sizeof(MTrk));
	if(result != OK)  return ABORT;
	if(!isTempoOnlyTrk) {
		/* Remember where to write the track length */
		MidiLen_pos  = ftell(fout);
		if(MidiLen_pos < 0) {
			BPPrintMessage(0,odError, "=> Error in WriteBeginningOfTrack(): ftell() returned %ld.\n", MidiLen_pos);
			return ABORT;
			}
		if(WriteReverse(fout,(dword)0x00000000) != OK) return(ABORT);
		}
	else {
		// already know the length of a tempo-only track
		length = sizeof(time_sig) + sizeof(tempo_sig) + 3
			+ sizeof(key_sig) + sizeof(end_of_track);
		if(WriteReverse(fout,length) != OK) return(ABORT);
		}
	if(inclMetaEvts) {
		// write meta events for the time signature, tempo, & key signature
		result = WriteRawBytes(fout, time_sig, sizeof(time_sig));
		if(result != OK)  return ABORT;

		result = WriteRawBytes(fout, tempo_sig, sizeof(tempo_sig));
		if(result != OK)  return ABORT;

		if(Pclock > 0.)	/* Striated time, or measured smooth time */
			quarternotedur = (Pclock * 1000000L) / Qclock;
		else
			quarternotedur = 1000000L;
		b0 = (byte) (quarternotedur & 0xff);
		b1 = (byte) ((quarternotedur >> 8) & 0xff);
		b2 = (byte) ((quarternotedur >> 16) & 0xff);
		result = WriteRawBytes(fout, &b2, 1L);
		if(result != OK)  return ABORT;
		result = WriteRawBytes(fout, &b1, 1L);
		if(result != OK)  return ABORT;
		result = WriteRawBytes(fout, &b0, 1L);
		if(result != OK)  return ABORT;
		
		result = WriteRawBytes(fout, key_sig, sizeof(key_sig));
		if(result != OK)  return ABORT;

		MIDItracklength = sizeof(time_sig) + sizeof(tempo_sig) + 3
			+ sizeof(key_sig);
		}
	if(isTempoOnlyTrk) {
		// Write "End of Track" meta event because no normal MIDI events
		// will be added to this "tempo track".
		result = WriteRawBytes(fout, end_of_track, sizeof(end_of_track));
		if(result != OK)  return ABORT;
		}
	for(i=1; i <= MAXCHAN; i++) CurrentVolume[i] = -1;	
	return OK;
	}


static int WriteEndOfTrack(FILE* fout)
{
	int result;
	long pos;
	
	if(trace_writing_midi_file) BPPrintMessage(0,odInfo, "Writing end of track\n");
	result = FadeOut();
	if(result != OK)  return result;
	
	if(MIDIbytestate > 0) {
		/* Write out the final message. */
		result = Writedword(fout, Midi_msg, MIDIbytestate);
		if(result != OK) return result;
		MIDItracklength += MIDIbytestate;
		MIDIbytestate = 0;
		Midi_msg = ZERO;
		MIDIfileTrackEmpty = FALSE;
	}
	
	/* Finish off the track tail */
	result = WriteRawBytes(fout, end_of_track, sizeof(end_of_track));
	if(result != OK)  return result;
	
	MIDItracklength += sizeof(end_of_track);
	
	/* Remember where we are */
	pos  = ftell(fout);
	if(pos < 0) {
		BPPrintMessage(0,odError, "=> Error in WriteEndOfTrack(): ftell() returned %ld.\n", pos);
		return ABORT;
	}
	
	/* Write the track length at the right place. */
	result = fseek(fout, MidiLen_pos, SEEK_SET);
	if(result != 0) return ABORT;
	result = WriteReverse(fout, MIDItracklength);
	if(result != OK) return result;
	
	/* Restore the position at the end of track */
	result = fseek(fout, pos, SEEK_SET);
	if(result != 0) return ABORT;
	
	return OK;
}

static int WriteRawBytes(FILE* fout, byte* data, size_t numbytes) {
	size_t written;
	written = fwrite(data, (size_t)1, numbytes, fout);
	if(written < numbytes)	{
		BPPrintMessage(0,odError, "=> Error while writing to Midi file.\n");
		return ABORT;
		}
	return OK;
	}


int WriteMIDIbyte(Milliseconds time,byte midi_byte) {
	// time is in milliseconds
	if(SoundOn && !MIDIfileOn) return(OK);
	if(!MIDIfileOpened) return(OK);
	MIDIfileTrackEmpty = FALSE;

	if(midi_byte & 0x80) {  /* MSBit of MIDI byte is 1 */
		if(MIDIbytestate > 0) {	/* Write out the accumulated message. */
			if(Writedword(OpenMIDIfilePtr, Midi_msg, MIDIbytestate) != OK) goto BAD;
			MIDItracklength += MIDIbytestate;
			}
		if(OldMIDIfileTime == -1L) OldMIDIfileTime = time;
		/* This happens in the beginning of the file */
			
		if(time < OldMIDIfileTime) OldMIDIfileTime = time;
		/* This could happen with a bad rounding. Normally BP3 sorts out events */
		
		/* Write out variable length delta time value. */
		if(WriteVarLenQuantity(OpenMIDIfilePtr, (dword)(time-OldMIDIfileTime),
				&MIDItracklength) != OK) goto BAD;
				
		OldMIDIfileTime = time;
		/* Grab the new byte read. */
		Midi_msg = (dword)midi_byte;
		MIDIbytestate = 1;
		if(trace_writing_midi_file)
			BPPrintMessage(0,odInfo,"midi_byte = %d time = %ld OldMIDIfileTime = %ld MIDItracklength = %ld Midi_msg = %ld\n",midi_byte,(long)time,(long)OldMIDIfileTime,(long)MIDItracklength,(long)Midi_msg);
		}
	else {
		if(trace_writing_midi_file)
			BPPrintMessage(0,odInfo,"midi_byte = %d time = %ld OldMIDIfileTime = %ld MIDItracklength = %ld Midi_msg = %ld\n",midi_byte,(long)time,(long)OldMIDIfileTime,(long)MIDItracklength,(long)Midi_msg);
		if(MIDIbytestate > 3 || MIDIbytestate < 1) {
		//	BPPrintMessage(0,odError,"=> Err. WriteMIDIbyte(). MIDIbytestate > 3 || MIDIbytestate < 1");
		//	BPPrintMessage(0,odError, "=> Correcting the byte state (%d) in MIDI file\n",MIDIbytestate);
			return(OK);
			}
		Midi_msg |= ((dword)midi_byte) << (8 * MIDIbytestate); /* accumulate msg */
		MIDIbytestate++;		/* Keep track of number of bytes in msg. */
		}
	return(OK);

	BAD:
	BPPrintMessage(0,odError,"=> Canceling creation of MIDIfile\n");
	CloseMIDIFile2();
	return(ABORT);
	}


int NewTrack(void)
{
int result;

if(!MIDIfileOpened) return(OK);

if(MIDIfileType != 2) {
	BPPrintMessage(0,odError,"=> Err.NewTrack(). This is not a type-2 file");
	}

// finish the current track
result = WriteEndOfTrack(OpenMIDIfilePtr);
if(result != OK)  return result;

// make a new track
result = WriteBeginningOfTrack(OpenMIDIfilePtr, TRUE, FALSE);
return result;
}

/* Finishes writing the track and MIDI header, then calls CloseMIDIFile2() */
int CloseMIDIFile(void) {
	int result;
	byte byteval;

	if(!MIDIfileOpened) return(OK);
	// BPPrintMessage(0,odInfo,"Closing MIDI file\n");
	result = WriteEndOfTrack(OpenMIDIfilePtr);
	if(result == OK) {
		// FIXME: Even if the track is empty, we should probably still count it, right?
		if(MIDIfileTrackEmpty) MIDIfileTrackNumber--;
		/* Write again number of tracks */
		result = fseek(OpenMIDIfilePtr, sizeof(header1) + 1 + sizeof(header2), SEEK_SET);
		if(result != 0) {
			result = MISSED;
			}
		else {
			byteval = (byte) (MIDIfileTrackNumber & 0xff);
			result = WriteRawBytes(OpenMIDIfilePtr, &byteval, 1L);
			}
		}
	// Need to close the file and cleanup even if the above fails!
	CloseMIDIFile2();
	return result;
	}


/* Actually closes the MIDI file and resets globals */
static int CloseMIDIFile2(void) {
	if(gOptions.outputFiles[ofiMidiFile].isOpen) {
		fflush(gOptions.outputFiles[ofiMidiFile].fout);
		CloseOutputFile(&(gOptions.outputFiles[ofiMidiFile]));
		my_sprintf(Message,"Closing MIDI file %s",gOptions.outputFiles[ofiMidiFile].name);
		ShowMessage(TRUE,wMessage,Message);
		}
	MIDIfileOpened = FALSE;
	NewOrchestra = TRUE;
	OpenMIDIfilePtr = NULL;
	// MIDIfileName[0] = '\0';
	strcpy(MIDIfileName,""); // Fixed by BB 2021-02-14
	return OK;
	}


int WriteReverse(FILE* fout, dword x)
// Write a dword high byteval first
{
byte byteval;
int result;

byteval = (byte)((x >> 24) & 0xff);
result = WriteRawBytes(fout, &byteval, 1L);
if(result != OK)  return ABORT;
byteval = (byte)((x >> 16) & 0xff);
result = WriteRawBytes(fout, &byteval, 1L);
if(result != OK)  return ABORT;
byteval = (byte)((x >> 8) & 0xff);
result = WriteRawBytes(fout, &byteval, 1L);
if(result != OK)  return ABORT;
byteval = (byte)(x & 0xff);
result = WriteRawBytes(fout, &byteval, 1L);
if(result != OK)  return ABORT;

return(OK);
}


int Writedword(FILE* fout, dword x, int n)
//  Writes n bytes of the dword x to the MIDI file, high byteval last
{
byte byteval;
int result;

while(n > 0) {
	byteval = (byte)(x & 0xff);
	result = WriteRawBytes(fout, &byteval, 1L);
	if(result != OK)  return ABORT;
	x >>= 8;
	n--;
	}
return(OK);
}


/*  "Variable Length Quantities" are used in Midi files to represent
	timestamps and other values.  They can be 1-4 bytes long depending
	on the value to be represented.  Each byte stores only 7 significant 
	bits of the value.  The most significant bit of each byte is 1 except 
	for the last byte where it is 0.  The bytes of the "VLQ" are ordered
	so that the most significant bits of the value come first.  Values
	that can be stored in n bytes are
	
		# bytes		range
		----------------------------------
			1		0-127
			2		128-16383
			3		16384-2097151
			4		2097152-268435455
	
	Ex. 3,500,000 (0x35 67 E0) in binary is 
		00110101 01100111 11100000  in octets, or
		0000001 1010101 1001111 1100000  in heptets.
		
		So, the VLQ encoding would be
		10000001 11010101 11001111 01100000
		which is 0x81 D5 CF 60 in hexadecimal.
 */
static int WriteVarLenQuantity(FILE* fout, dword value, dword *tracklen)
{
	int  result;
	byte varlen[4];
	long numbytes = 1;
	
	if(value > 268435455)	{
			{
			BPPrintMessage(0,odError, "=> Err. WriteVarLenQuantity(): value %u is out of range in chunk #%d\n",value,Chunk_number);
		}
		return ABORT; // Fixed by BB 2021-02-25
	}
	
	// split value into 7-bit chunks
	varlen[3] = (byte)(value & BITS0_6);
	varlen[2] = (byte)((value & BITS7_13) >> 7);
	varlen[1] = (byte)((value & BITS14_20) >> 14);
	varlen[0] = (byte)((value & BITS21_27) >> 21);
	
	// set the high bit of significant bytes (except least sig.)
	if(varlen[0]) {
		varlen[0] |= 0x80;
		varlen[1] |= 0x80;
		varlen[2] |= 0x80;
		numbytes = 4;
	}
	else if(varlen[1]) {
		varlen[1] |= 0x80;
		varlen[2] |= 0x80;
		numbytes = 3;
	}
	else if(varlen[2]) {
		varlen[2] |= 0x80;
		numbytes = 2;
	}
	
	// write only the significant bytes
	result = WriteRawBytes(fout, &(varlen[4-numbytes]), numbytes);
	*tracklen += numbytes;
	return result;
}

int PrepareMIDIFile(void) {
	int rep;
	if(!Create_set) WriteMIDIorchestra();
	if(!MIDIfileOpened) {
		if(gOptions.outputFiles[ofiMidiFile].name != NULL) {
			return MakeMIDIFile(&(gOptions.outputFiles[ofiMidiFile]));
			}
		else {
			BPPrintMessage(0,odError, "=> Error in PrepareMIDIFile(): file name is NULL.\n");
			return MISSED;
			}
		}
	return(OK);
	}


int ResetMIDIfile(void)
{
if(MIDIfileOpened) {
	if(FileSaveMode == NEWFILE) CloseMIDIFile();
	else if(MIDIfileType == 2) {
		NewTrack();
		if(!SoundOn) Nbytes = ZERO; 	/* Added 14/11/98 - avoids waiting on ResetMIDI() */
		}
	}
return(OK);
}

int WriteMIDIorchestra(void)
{
int i,w,rs;
MIDI_Event e;

if(!NewOrchestra) return(OK);
if(!MIDIfileOn || !MIDIfileOpened) return(OK);

for(i=1; i <= MAXCHAN; i++) {
	w = CurrentMIDIprogram[i];
	if(w > 0 && w <= 128) {
		NewOrchestra = FALSE;
		e.time = Tcurr;
		e.type = TWO_BYTE_EVENT;
		e.status = ProgramChange + i - 1;
		e.data2 = w - 1;
		rs = 0;
		SendToDriver(0,0,0,Tcurr * Time_res,0,&rs,&e);
		}
	}
return(OK);
}


int FadeOut(void)
{
int i_event,i_event_max,rs,chan,value,value2,this_volume,rep;
Milliseconds time,timeorigin,time_end,current_volume[MAXCHAN+1];
MIDI_Event e;
float ratio;
unsigned char this_char;

#if BP_CARBON_GUI_FORGET_THIS
GetFileSavePreferences();
#endif /* BP_CARBON_GUI_FORGET_THIS */

if(EndFadeOut > 0.) {
	rs = 0;
	timeorigin = LastTime;
	time_end = timeorigin + (1000 * EndFadeOut);
	i_event_max = (int)(EndFadeOut * SamplingRate);
	my_sprintf(Message,"Fading out MIDI stream %.3f sec as requested by the settings (or by default)\n",(float)EndFadeOut);
	BPPrintMessage(0,odInfo,Message);
	for(chan=1; chan <= MAXCHAN; chan++)
		current_volume[chan] = CurrentVolume[chan];
	for(i_event = 1; i_event <= i_event_max; i_event++) {
		time = timeorigin + ((1000 * i_event) / SamplingRate);
	//	if(time > time_end) break;
		ratio = ((float)(i_event_max - i_event)) / i_event_max;
		for(chan=1; chan <= MAXCHAN; chan++) {
			this_volume = current_volume[chan];
			if(this_volume < 1) continue;
			value2 = (float)this_volume * ratio;
			this_char = (unsigned char)(time / Time_res);
			e.time = this_char;
			e.type = NORMAL_EVENT;
			this_char = (unsigned char)(ControlChange + chan - 1);
			e.status = this_char;
			this_char = (unsigned char)VolumeControl[chan];
			e.data1 = this_char;
			this_char = (unsigned char)value2;
			e.data2 = this_char;
			if((rep=SendToDriver(0,0,0,time,0,&rs,&e)) != OK) {
				BPPrintMessage(0,odInfo,"SendToDriver aborted! rep = %ld\n",(long)rep);
				goto SORTIR;
				}
			my_sprintf(Message,"%d/%d ratio = %.3f  channel %d: time = %ld ms, current_volume = %ld, e.data1 = %d, e.data2 = %d\n",i_event,i_event_max,ratio,chan,(long)time,(long)this_volume,e.data1,e.data2);
			if(check_fade_out) BPPrintMessage(0,odInfo,Message);
			}
		}
SORTIR:
	for(chan=1; chan <= MAXCHAN; chan++) CurrentVolume[chan] = -1;
	}
return(OK);
}
