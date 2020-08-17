/* MIDIfiles.c (BP2 version CVS) */
/* Thanks to Srikumar K. Subramanian */

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


#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"
#include "ConsoleGlobals.h"
#include "ConsoleMessages.h"

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

int MakeMIDIFile(OutFileInfo* finfo);
static int WriteMIDIFileHeader(FILE* fout);
static int WriteBeginningOfTrack(FILE* fout, Boolean inclMetaEvts, Boolean isTempoOnlyTrk);
static int WriteEndOfTrack(FILE* fout);
static int WriteRawBytes(FILE* fout, byte* data, size_t numbytes);
static int WriteVarLenQuantity(FILE* fout, dword value, dword *tracklen);
static int CloseMIDIFile2(void);

int MakeMIDIFile(OutFileInfo* finfo)
{
int result;
Str255 filename;
FILE *fout;

	BP_NOT_USED(filename);
	result = FAILED;

	ShowMessage(TRUE,wMessage,"Create new MIDI file...");
	fout = OpenOutputFile(finfo, "wb");
	if (!fout) {
		BPPrintMessage(odError, "Could not open file %s\n", finfo->name);
		return FAILED;
	}
	else {
		OpenMIDIfilePtr = fout;
		MIDIfileOpened = MIDIfileTrackEmpty = TRUE;
		MIDIfileTrackNumber = 0;

#if 0
		/* Below is some interface and scripting business */
		p2cstrcpy(MIDIfileName,filename);
		SetField(FileSavePreferencesPtr,-1,fMIDIFileName,MIDIfileName);
		sprintf(Message,"\"%s\"",MIDIfileName);
		MystrcpyStringToTable(ScriptLine.arg,0,Message);
		AppendScript(181);
		/* End of interface business */
#endif /* BP_CARBON_GUI */
		
		result = WriteMIDIFileHeader(fout);
		if (result == OK) {
			if (MIDIfileType == 1) {
				// write the tempo track for the whole file
				result = WriteBeginningOfTrack(fout, TRUE, TRUE);
				if (result == OK) {
					// start a new track for MIDI events without meta info
					result = WriteBeginningOfTrack(fout, FALSE, FALSE);
				}
			}
			else {
				// For both type 0 & type 2 files,
				// write tempo/meta info & events into the same track.
				result = WriteBeginningOfTrack(fout, TRUE, FALSE);
			}
			if (result == OK) WriteMIDIorchestra();
		}
	}
	
	if (result != OK) CloseMIDIFile2();
	return result;
}


static int WriteMIDIFileHeader(FILE* fout)
{
	int result;
	byte byteval,b0,b1;
	unsigned long deltatimesinquarternote;

	/* Write the MThd block to the new file */
	result = WriteRawBytes(fout, header1, sizeof(header1));
	if (result != OK)  return ABORT;

	/* Write the file type */
	byteval = (byte) (MIDIfileType & 0xff);
	result = WriteRawBytes(fout, &byteval, 1L);
	if (result != OK)  return ABORT;

	result = WriteRawBytes(fout, header2, sizeof(header2));
	if (result != OK)  return ABORT;

	/* Write number of tracks */
	/* This is a temporary value since type 1 & 2 files may contain more tracks */
	byteval = (byte) (1 & 0xff);
	result = WriteRawBytes(fout, &byteval, 1L);
	if (result != OK)  return ABORT;

	/* Write division of quarter-note */
	if(Pclock > 0.)
		deltatimesinquarternote = (Pclock * 1000L) / Qclock;
	else
		deltatimesinquarternote = 1000L;
	b0 = (byte) (deltatimesinquarternote & 0xff);
	b1 = (byte) ((deltatimesinquarternote >> 8) & 0xff);
	result = WriteRawBytes(fout, &b1, 1L);
	if (result != OK)  return ABORT;
	result = WriteRawBytes(fout, &b0, 1L);
	if (result != OK)  return ABORT;

	return OK;
}


/*	WriteBeginningOfTrack()
	
	Write the beginning of a new track chunk.
	
	Parameters:
	  inclMetaEvts - TRUE if track should include Time Sign., Tempo, & Key Sign. events
	  isTempoOnlyTrk - TRUE if track is the "tempo track" for a Type-1 MIDI file; only
		  meta events will be written to the track followed by an "End of Track" event.
 */
static int WriteBeginningOfTrack(FILE* fout, Boolean inclMetaEvts, Boolean isTempoOnlyTrk)
{
	int i, result;
	dword length;
	byte b0,b1,b2;
	unsigned long quarternotedur;
	
	if (isTempoOnlyTrk && !inclMetaEvts) {
		// this function does not support writing an empty track without meta events
		BPPrintMessage(odError, "Error in WriteBeginningOfTrack(): inconsistent parameters\n");
		return ABORT;
	}
	
	Midi_msg = ZERO;
	OldMIDIfileTime = -1L;
	MIDItracklength = ZERO;
	MIDIbytestate = 0;
	MIDIfileTrackNumber++;
	MIDIfileTrackEmpty = TRUE;
	
	result = WriteRawBytes(fout, MTrk, sizeof(MTrk));
	if (result != OK)  return ABORT;

	if (!isTempoOnlyTrk) {
		/* Remember where to write the track length */
		MidiLen_pos  = ftell(fout);
		if (MidiLen_pos < 0) {
			BPPrintMessage(odError, "Error in WriteBeginningOfTrack(): ftell() returned %ld.\n", MidiLen_pos);
			return ABORT;
		}
		if (WriteReverse(fout,(dword)0x00000000) != OK) return(ABORT);
	}
	else {
		// already know the length of a tempo-only track
		length = sizeof(time_sig) + sizeof(tempo_sig) + 3
			+ sizeof(key_sig) + sizeof(end_of_track);
		if(WriteReverse(fout,length) != OK) return(ABORT);
	}
	
	if (inclMetaEvts) {
		// write meta events for the time signature, tempo, & key signature
		result = WriteRawBytes(fout, time_sig, sizeof(time_sig));
		if (result != OK)  return ABORT;

		result = WriteRawBytes(fout, tempo_sig, sizeof(tempo_sig));
		if (result != OK)  return ABORT;

		if(Pclock > 0.)	/* Striated time, or measured smooth time */
			quarternotedur = (Pclock * 1000000L) / Qclock;
		else
			quarternotedur = 1000000L;
		b0 = (byte) (quarternotedur & 0xff);
		b1 = (byte) ((quarternotedur >> 8) & 0xff);
		b2 = (byte) ((quarternotedur >> 16) & 0xff);
		result = WriteRawBytes(fout, &b2, 1L);
		if (result != OK)  return ABORT;
		result = WriteRawBytes(fout, &b1, 1L);
		if (result != OK)  return ABORT;
		result = WriteRawBytes(fout, &b0, 1L);
		if (result != OK)  return ABORT;
		
		result = WriteRawBytes(fout, key_sig, sizeof(key_sig));
		if (result != OK)  return ABORT;

		MIDItracklength = sizeof(time_sig) + sizeof(tempo_sig) + 3
			+ sizeof(key_sig);
	}
	
	if (isTempoOnlyTrk) {
		// Write "End of Track" meta event because no normal MIDI events
		// will be added to this "tempo track".
		result = WriteRawBytes(fout, end_of_track, sizeof(end_of_track));
		if (result != OK)  return ABORT;
	}
	
	for(i=1; i <= MAXCHAN; i++) CurrentVolume[i] = -1;	
	return OK;
}


static int WriteEndOfTrack(FILE* fout)
{
	int result;
	long pos;
	
	result = FadeOut();
	if (result != OK)  return result;
	
	if (MIDIbytestate > 0) {
		/* Write out the final message. */
		result = Writedword(fout, Midi_msg, MIDIbytestate);
		if (result != OK) return result;
		MIDItracklength += MIDIbytestate;
		MIDIbytestate = 0;
		Midi_msg = ZERO;
		MIDIfileTrackEmpty = FALSE;
	}
	
	/* Finish off the track tail */
	result = WriteRawBytes(fout, end_of_track, sizeof(end_of_track));
	if (result != OK)  return result;
	
	MIDItracklength += sizeof(end_of_track);
	
	/* Remember where we are */
	pos  = ftell(fout);
	if (pos < 0) {
		BPPrintMessage(odError, "Error in WriteEndOfTrack(): ftell() returned %ld.\n", pos);
		return ABORT;
	}
	
	/* Write the track length at the right place. */
	result = fseek(fout, MidiLen_pos, SEEK_SET);
	if (result != 0) return ABORT;
	result = WriteReverse(fout, MIDItracklength);
	if (result != OK) return result;
	
	/* Restore the position at the end of track */
	result = fseek(fout, pos, SEEK_SET);
	if (result != 0) return ABORT;
	
	return OK;
}


static int WriteRawBytes(FILE* fout, byte* data, size_t numbytes)
{
	size_t written;
	
	written = fwrite(data, (size_t)1, numbytes, fout);
	if (written < numbytes)	{
		BPPrintMessage(odError, "Error while writing to Midi file.\n");
		return ABORT;
	}
	
	return OK;
}


int WriteMIDIbyte(Milliseconds time,byte midi_byte)
{
// time is in milliseconds
if(SoundOn && !MIDIfileOn) return(OK);
if(!MIDIfileOpened) return(OK);

PleaseWait();
MIDIfileTrackEmpty = FALSE;
if(midi_byte & 0x80) {  /* MSBit of MIDI byte is 1 */
	if(MIDIbytestate > 0) {	/* Write out the accumulated message. */
		if(Writedword(OpenMIDIfilePtr, Midi_msg, MIDIbytestate) != OK) goto BAD;
		MIDItracklength += MIDIbytestate;
		}
		
 	if(OldMIDIfileTime == -1L) OldMIDIfileTime = time;
 	/* This happens in the beginning of the file */
 		
	if(time < OldMIDIfileTime) OldMIDIfileTime = time;
	/* This could happen with a bad rounding. Normally BP2 sorts out events */
		
	/* Write out variable length delta time value. */
	if(WriteVarLenQuantity(OpenMIDIfilePtr, (dword)(time-OldMIDIfileTime),
			&MIDItracklength) != OK) goto BAD;
	
	OldMIDIfileTime = time;

	/* Grab the new byte read. */
	Midi_msg = (dword)midi_byte;
	MIDIbytestate = 1;
	}
else {
	if(MIDIbytestate > 3 || MIDIbytestate < 1) {
		if(Beta) Alert1("Err. WriteMIDIbyte(). MIDIbytestate > 3 || MIDIbytestate < 1");
		return(OK);
		}
	Midi_msg |= ((dword)midi_byte) << (8 * MIDIbytestate); /* accumulate msg */
	MIDIbytestate++;		/* Keep track of number of bytes in msg. */
	}
return(OK);

BAD:
CloseMIDIFile2();
return(ABORT);
}


int NewTrack(void)
{
int result;

if(!MIDIfileOpened) return(OK);

if(MIDIfileType != 2) {
	if(Beta) Alert1("Err.NewTrack(). This is not a type-2 file");
	}

// finish the current track
result = WriteEndOfTrack(OpenMIDIfilePtr);
if (result != OK)  return result;

// make a new track
result = WriteBeginningOfTrack(OpenMIDIfilePtr, TRUE, FALSE);
return result;
}


/* Finishes writing the track and MIDI header, then calls CloseMIDIFile2() */
int CloseMIDIFile(void)
{
int result;
byte byteval;

if(!MIDIfileOpened) return(OK);

result = WriteEndOfTrack(OpenMIDIfilePtr);
if (result == OK) {
	// FIXME: Even if the track is empty, we should probably still count it, right?
	if(MIDIfileTrackEmpty) MIDIfileTrackNumber--;

	/* Write again number of tracks */
	result = fseek(OpenMIDIfilePtr, sizeof(header1) + 1 + sizeof(header2), SEEK_SET);
	if (result != 0) {
		result = FAILED;
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
static int CloseMIDIFile2(void)
{
if (gOptions.outputFiles[ofiMidiFile].isOpen) {
	fflush(gOptions.outputFiles[ofiMidiFile].fout);
	CloseOutputFile(&(gOptions.outputFiles[ofiMidiFile]));
	sprintf(Message,"Closed MIDI file '%s'",gOptions.outputFiles[ofiMidiFile].name);
	ShowMessage(TRUE,wMessage,Message);
}

MIDIfileOpened = FALSE;
NewOrchestra = TRUE;
OpenMIDIfilePtr = NULL;
MIDIfileName[0] = '\0';
return OK;
}


int WriteReverse(FILE* fout, dword x)
// Write a dword high byteval first
{
byte byteval;
int result;

byteval = (byte)((x >> 24) & 0xff);
result = WriteRawBytes(fout, &byteval, 1L);
if (result != OK)  return ABORT;
byteval = (byte)((x >> 16) & 0xff);
result = WriteRawBytes(fout, &byteval, 1L);
if (result != OK)  return ABORT;
byteval = (byte)((x >> 8) & 0xff);
result = WriteRawBytes(fout, &byteval, 1L);
if (result != OK)  return ABORT;
byteval = (byte)(x & 0xff);
result = WriteRawBytes(fout, &byteval, 1L);
if (result != OK)  return ABORT;

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
	if (result != OK)  return ABORT;
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
	
	if (value > 268435455)	{
		if (Beta)	{
			BPPrintMessage(odError, "Err. WriteVarLenQuantity(): value %u is out of range.", value);
		}
		return FAILED;
	}
	
	// split value into 7-bit chunks
	varlen[3] = (byte)(value & BITS0_6);
	varlen[2] = (byte)((value & BITS7_13) >> 7);
	varlen[1] = (byte)((value & BITS14_20) >> 14);
	varlen[0] = (byte)((value & BITS21_27) >> 21);
	
	// set the high bit of significant bytes (except least sig.)
	if (varlen[0]) {
		varlen[0] |= 0x80;
		varlen[1] |= 0x80;
		varlen[2] |= 0x80;
		numbytes = 4;
	}
	else if (varlen[1]) {
		varlen[1] |= 0x80;
		varlen[2] |= 0x80;
		numbytes = 3;
	}
	else if (varlen[2]) {
		varlen[2] |= 0x80;
		numbytes = 2;
	}
	
	// write only the significant bytes
	result = WriteRawBytes(fout, &(varlen[4-numbytes]), numbytes);
	*tracklen += numbytes;
	return result;
}


#if BP_CARBON_GUI
dword ReadVarLen(short refnum,int *p_result,long *p_nbytes)
{
dword value;
byte byteval;
long count;
int howmany;

*p_result = OK;
count = 1L;
if(FSRead(refnum,&count,&byteval) != noErr) goto ERR;
(*p_nbytes)++;
howmany = 1;

value = byteval;
if(value & 0x80) {
	value &= 0x7f;
	do {
		count = 1L;
		if(FSRead(refnum,&count,&byteval) != noErr) goto ERR;
		(*p_nbytes)++;
		howmany++;
		value = (value << 7) + (byteval & 0x7f);
		}
	while((byteval & 0x80) && howmany < 4);
	}
return(value);

ERR:
*p_result = FAILED;
return(value);
}
#endif /* BP_CARBON_GUI */


#if BP_CARBON_GUI
GetMIDIfileName(void)
{
char line[MAXFIELDCONTENT];
long p,q;

GetField(FileSavePreferencesPtr,FALSE,-1,fMIDIFileName,line,&p,&q);
if(TooLongFileName(line,FileSavePreferencesPtr,-1,fMIDIFileName)) return(FAILED);
if(strcmp(MIDIfileName,line) != 0) {
	CloseMIDIFile();
	strcpy(MIDIfileName,line);
	if(line[0] != '\0') {
		MakeMIDIFile(line);
		return(DONE);
		}
	}
return(OK);
}
#endif /* BP_CARBON_GUI */


int PrepareMIDIFile(void)
{
int rep;

WriteMIDIorchestra();
if(!MIDIfileOpened) {
	/* Console version of PrepareMIDIFile() assumes that score file name
	   has been set by a command-line argument */
	if (gOptions.outputFiles[ofiMidiFile].name != NULL) {
		return MakeMIDIFile(&(gOptions.outputFiles[ofiMidiFile]));
		}
	else {
		BPPrintMessage(odError, "Error in PrepareMIDIFile(): file name is NULL.\n");
		return FAILED;
		}
	}
// else MIDI file is already open
switch(FileSaveMode) {	// FIXME !!!!
	case ALLSAME:
		return(OK);
		break;
	case ALLSAMEPROMPT:
		sprintf(Message,"Current MIDI file is '%s'. Change it",MIDIfileName);
		rep = Answer(Message,'N');
		if(rep == ABORT) return(rep);
		if(rep == NO) return(OK);
		/* no break */
	case NEWFILE:
		CloseMIDIFile();
		return(MakeMIDIFile(&(gOptions.outputFiles[ofiMidiFile])));
		break;
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


#if BP_CARBON_GUI
int ImportMIDIfile(int j)
{
int result,preservechannel;
Handle ptr;

if(j < 2 || j >= Jbol) return(FAILED);

ptr = (Handle)(*pp_MIDIcode)[j];
MyDisposeHandle(&ptr);
(*pp_MIDIcode)[j] = NULL;
(*p_MIDIsize)[j] = ZERO;

CloseMIDIFile();

if(ReadMIDIfile(&preservechannel) != OK) return(DONE);

(*p_Tref)[j] = ((double)Stream.pclock * 1000.) / Stream.qclock;

result = PasteStreamToPrototype(j, bDeleteReplace);
MyDisposeHandle((Handle*)&Stream.code);
Stream.imax = ZERO;

if(preservechannel) (*p_DefaultChannel)[j] = -1;

ChangedProtoType(j);

#if BP_CARBON_GUI
if(j == iProto && !((*p_Type)[j] & 1)) {
	(*p_Type)[j] |= 1;
	if(iProto == j) SwitchOn(NULL,wPrototype1,bMIDIsequence);
	}
#endif /* BP_CARBON_GUI */
return(result);
}


int ReadMIDIfile(int *p_preservechannel)
{
int itrack,ibyte,ibytemax,anyfile,type,result,status,format,track,ntracks,trackmin,uniquechan,
	deltatimesinquarternote,reftrack,err,said,notsaidaboutchannel,channelevent,seentempo,
	chan,reply,severaltempo;
FSSpec spec;
short refnum;
Str255 fn;
OSErr io;
char line[MAXLIN];
byte byteval,metaeventtype,**p_buffer;
long i,j,count,nbytes,buffersize,maxevents,totalbytes;
dword chunklength,deltatime,eventlength;
unsigned long quarternotedur,qn;
double p,q;
Milliseconds currenttime;
MIDIstream **p_stream;
MIDIcode **ptr2;
Handle ptr;
Milliseconds mintime;

anyfile = said = channelevent = *p_preservechannel = seentempo = severaltempo = FALSE;
notsaidaboutchannel = TRUE;
result = OK;

p_stream = NULL;
ntracks = err = uniquechan = 0;
totalbytes = nbytes = ZERO;

anyfile = Answer("Show files other than 'Midi' type",'N');
if(anyfile == ABORT) return(FAILED);

TRYLOAD:
LastAction = NO;

sprintf(Message,"Locate MIDI file...");
ShowMessage(TRUE,wMessage,Message);

type = 11;
if(anyfile) type = 0;

buffersize = 100L;
if((p_buffer=(unsigned char**) GiveSpace((Size)(buffersize * sizeof(unsigned char))))
	== NULL) return(ABORT);

if(OldFile(-1,type,fn,&spec)) {
	p2cstrcpy(LineBuff,fn);
	if((io=MyOpen(&spec,fsRdPerm,&refnum)) == noErr) {
		sprintf(Message,"Loading MIDI file...");
		ShowMessage(TRUE,wMessage,Message);
		
		/* Read header */
		MyLock(NO,(Handle) p_buffer);
		count = 14L;
		io = FSRead(refnum,&count,*p_buffer);
		MyUnlock((Handle) p_buffer);
		if(io != noErr || count < 14L) {
			err = 1; goto ERR;
			}
		for(i=0; i < 4; i++) line[i] = (*p_buffer)[i];
		line[i] = '\0';
		if(strcmp("MThd",line) != 0) {
			err = 2; goto ERR;
			}
		chunklength = ZERO;
			for(i=0; i < 4; i++)
				chunklength = (chunklength << 8) + (*p_buffer)[i+4];
		if(chunklength != 6) {
			err = 3;
			goto ERR;
			}
		if(((*p_buffer)[8] + (*p_buffer)[10]) > 0) {
			err = 4;
			goto ERR;
			}
		if((format=(*p_buffer)[9]) > 2) {
			err = 5;
			goto ERR;
			}
		
		ntracks = (*p_buffer)[11];
		
		if(format == 2 && ntracks > 2) {
			sprintf(Message,
				"This type-2 MIDI file contains %ld items. BP2 will read only the first one",
				(long) (ntracks - 1));
			Alert1(Message);
			ntracks = 2;
			}
			
		if((p_stream=(MIDIstream**)GiveSpace((Size)ntracks * sizeof(MIDIstream)))
				== NULL)  {
			err = 6;
			goto ERR;
			}
			
		quarternotedur = 1000000L;
		Simplify((double)LONG_MAX,(double)quarternotedur,1000000.,&p,&q);
		
		for(track=0; track < ntracks; track++) {
			(*p_stream)[track].code = NULL;
			(*p_stream)[track].i = (*p_stream)[track].imax = ZERO;
			(*p_stream)[track].period = ZERO;
			(*p_stream)[track].cyclic = FALSE;
			(*p_stream)[track].tempo = quarternotedur;
			(*p_stream)[track].pclock = (long) p;
			(*p_stream)[track].qclock = (long) q;
			} 
		
		Stream.pclock = (long) Pclock;
		Stream.qclock = (long) Qclock;
		reftrack = -1;

		deltatimesinquarternote = 256L * (*p_buffer)[12] + (*p_buffer)[13];
		if(deltatimesinquarternote == ZERO)  {
			err = 7;
			goto ERR;
			}
			
		for(track=0; track < ntracks; track++) {
			/* Read track header */
			MyLock(NO,(Handle) p_buffer);
			count = 8L;
			io = FSRead(refnum,&count,*p_buffer);
			MyUnlock((Handle) p_buffer);
			if(io != noErr || count < 8L)  {
				err = 8;
				goto ERR;
				}
			for(i=0; i < 4; i++) line[i] = (*p_buffer)[i];
			line[i] = '\0';
			if(strcmp("MTrk",line) != 0)  {
				err = 9;
				goto ERR;
				}
			totalbytes += nbytes;
			chunklength = nbytes = currenttime = ZERO;
			status = 0;
			for(i=0; i < 4; i++)
				chunklength = (chunklength << 8) + (*p_buffer)[i+4];
			if((ptr2=(MIDIcode**)GiveSpace((Size)2L * (chunklength + 1L) * sizeof(MIDIcode)))
					== NULL)  {
				err = 10;
				goto ERR;
				}
			(*p_stream)[track].code = ptr2;
			(*p_stream)[track].imax = 2L * chunklength;
			
			while(TRUE) {
				PleaseWait();
#if BP_CARBON_GUI
				// FIXME ? Should non-Carbon builds call a "poll events" callback here ? (probably not)
				if((result=MyButton(2)) != FAILED
						&& Answer("Continue reading MIDI file",'Y') != OK) {
					result = FAILED;
					goto OUT;
					}
#endif /* BP_CARBON_GUI */
				result = OK;
				if(nbytes >= chunklength) {
					err = 24;
					goto ERR;
					}
		//		totalbytes += nbytes;
				deltatime = ReadVarLen(refnum,&result,&nbytes);
				if(result != OK) {
					err = 11;
					goto ERR;
					}
	/*			currenttime
					+= ((*p_stream)[track].tempo * deltatime) / deltatimesinquarternote / 1000L; */
				currenttime
					+= (quarternotedur * deltatime) / deltatimesinquarternote / 1000L;
				if(status == 0) {
					ibyte = 0;
					ibytemax = 1;
					}
				else {
					ibyte = 1;
					ibytemax = 3;
					if(status == ProgramChange || status == ChannelPressure)
						ibytemax = 2;
					}
NEXTBYTE:				
				count = 1L;
				io = FSRead(refnum,&count,&byteval);
				nbytes++;
				if(io != noErr) {
					err = 12;
					goto ERR;
					}

META:	
				if(byteval == 0xff) {	/* meta-event */
					count = 1L;
					io = FSRead(refnum,&count,&metaeventtype);
					nbytes++;
					if(io != noErr) {
						err = 13;
						goto ERR;
						}
		//			totalbytes += nbytes;
					eventlength = ReadVarLen(refnum,&result,&nbytes);
					if(result != OK) {
						err = 0;
						goto ERR;
						}
					if(eventlength > ZERO) {
						if(eventlength >= buffersize) {	/* Skip long meta-events */
							for(i=0; i < eventlength; i++) {
								count = 1L;
								io = FSRead(refnum,&count,&byteval);
								nbytes++;
								if(io != noErr) {
									err = 14;
									goto ERR;
									}
								}
							continue;
							}
						MyLock(NO,(Handle) p_buffer);
						count = eventlength;
						io = FSRead(refnum,&count,*p_buffer);
						MyUnlock((Handle) p_buffer);
						nbytes += eventlength;
						if(io != noErr || count < eventlength) {
							err = 15;
							goto ERR;
							}
						}
					if(metaeventtype == 0x2f) {
						/* End of track */
						if(eventlength > ZERO) {
							err = 16;
							goto ERR;
							}
						if(nbytes != chunklength) {
							err = 17;
							goto ERR;
							}
						break;
						}
					if(metaeventtype == 0x51) {
						/* Tempo signature */
						if(eventlength != 3L) {
							err = 18;
							goto ERR;
							}
						qn = ZERO;
						for(i=0; i < 3; i++)
							qn = (qn << 8) + (*p_buffer)[i];
						
						if(seentempo) {
							severaltempo = TRUE;
							continue;
							}
						seentempo = TRUE;
						quarternotedur = qn;
						Simplify((long)LONG_MAX,(double)quarternotedur,1000000.,&p,&q);
						if(track > 0) {
							if(Beta) {
								sprintf(Message,"Found tempo signature in track #%ld",
									(long)track);
								Println(wTrace,Message);
								}
							(*p_stream)[track].tempo = quarternotedur;
							(*p_stream)[track].pclock = (long) p;
							(*p_stream)[track].qclock = (long) q;
							}
						else {
							for(itrack=0; itrack < ntracks; itrack++) {	/* Fixed 9/3/98 */
								(*p_stream)[itrack].tempo = quarternotedur;
								(*p_stream)[itrack].pclock = (long) p;
								(*p_stream)[itrack].qclock = (long) q;
								}
							}
						if((p > 1. || q > 1.) && reftrack == -1) {
							reftrack = track;
							Stream.pclock = (long) p;
							Stream.qclock = (long) q;
							}
						}
					continue;
					}
					
				if(byteval == 0xf0 || byteval == 0xf7) {	/* System exclusive */
//					totalbytes += nbytes;
					eventlength = ReadVarLen(refnum,&result,&nbytes);
					if(result != OK) {
						err = 0;
						goto ERR;
						}
					if(eventlength > ZERO) {
						/* Skip these messages */
						for(i=0; i < eventlength; i++) {
							count = 1L;
							io = FSRead(refnum,&count,&byteval);
							if(io != noErr) {
								err = 19;
								goto ERR;
								}
							}
						nbytes += eventlength;
						}
					continue;
					}
					
STORE:
				/* 'byteval' is a MIDI message */
				(*p_buffer)[ibyte++] = byteval;
				
				if(byteval & 0x80) {
					/* byteval >= 128 */
					status = ByteToInt(byteval);
					channelevent = FALSE;
					if(ChannelEvent(status)) {
						channelevent = TRUE;
						chan = status % 16;
						status -= chan;
						if(notsaidaboutchannel && chan > 0) {
							notsaidaboutchannel = FALSE;
PRESERVE:
							reply = Answer("Preserve channel information",'Y');
							if(reply == ABORT) {
								result = ABORT; goto OUT;
								}
							(*p_preservechannel) = reply;
							if(!(*p_preservechannel)) {
GETCHANNEL:
								if((reply=AnswerWith("Force to channel...","1",Message)) == ABORT) {
									result = ABORT; goto OUT;
									}
								if(reply == NO) goto PRESERVE;
								uniquechan = ((int) atol(Message)) - 1;
								if(uniquechan < 0 || uniquechan > 15) {
									sprintf(Message,"Channel range 1..16\nCan't accept '%ld'",
										(long)uniquechan + 1L);
									Alert1(Message);
									goto GETCHANNEL;
									}
								}
							}
						}
					ibyte = 1;
					if(ThreeByteEvent(status)) {
						ibytemax = 3;
						}
					else {
						switch(status) {
							case SongSelect:
							case ProgramChange:
							case ChannelPressure:
								ibytemax = 2;
								break;
							case Start:
							case Continue:
							case Stop:
							case ActiveSensing:
							case TuneRequest:
								ibytemax = 1;
								goto STREAM;
								break;
							default:
								if(Beta) {
									sprintf(Message,"Err. ReadMIDIfile(). Unknown byte %ld",status);
									Println(wTrace,Message);
									if(!said) Alert1(Message);
									said = TRUE;
									}
								break;
							}
						}
					goto NEXTBYTE;
					}
STREAM:
				if(ibyte >= ibytemax) {
					if(status < 128) {
						err = 22;
						goto ERR;
						}
					if(channelevent) {
						if(*p_preservechannel) status += chan;
						else status += uniquechan;
						}
					j = (*p_stream)[track].i;
					if(j >= (*p_stream)[track].imax) {
						if(Beta) Alert1("Err. ReadMIDIfile(). j >= (*p_stream)[track].imax");
						return(FAILED);
						}
					(*((*p_stream)[track].code))[j].time = currenttime;
					(*((*p_stream)[track].code))[j].byte = status;
					(*((*p_stream)[track].code))[j].sequence = track;
					((*p_stream)[track].i)++;
					for(i=1; i < ibyte; i++) {
						/* We start with i = 1 because 'status' has already been streamed */
						j = (*p_stream)[track].i;
						(*((*p_stream)[track].code))[j].time = currenttime;
						(*((*p_stream)[track].code))[j].byte = ByteToInt((*p_buffer)[i]);
						(*((*p_stream)[track].code))[j].sequence = track;
						((*p_stream)[track].i)++;
						}
					continue;
					}
				else goto NEXTBYTE;
				}
			}
		}
	else {
		sprintf(Message,"Unexpected error opening '%s'",LineBuff);
		ShowMessage(TRUE,wMessage,Message);
		TellError(75,io);
		result = FAILED;
		}
	}
else {
	MyDisposeHandle((Handle*)&p_buffer);
	return (ABORT);
	}

maxevents = ZERO;
for(track=0; track < ntracks; track++) {
	if(p_stream == NULL) {
		if(Beta) Alert1("Err. ReadMIDIfile(). p_stream == NULL");
		continue;
		}
	(*p_stream)[track].imax = (*p_stream)[track].i;
	maxevents += (*p_stream)[track].imax;
	(*p_stream)[track].i = ZERO;
	}
	
// Now the MIDI streams of different tracks will be merged and sorted

MyDisposeHandle((Handle*)&Stream.code);
Stream.imax = Stream.i = ZERO;
Stream.period = ZERO;
Stream.cyclic = FALSE;
if(maxevents == ZERO) goto OUT;

if((ptr2=(MIDIcode**) GiveSpace((Size) maxevents * sizeof(MIDIcode))) == NULL) {
	err = 23;
	goto ERR;
	}
Stream.code = ptr2;

for(i=ZERO; i < maxevents; i++) {
	mintime = -1L;
	for(track=0; track < ntracks; track++) {
		j = (*p_stream)[track].i;
		if(j >= (*p_stream)[track].imax) continue;
		if(mintime < ZERO || mintime > (*((*p_stream)[track].code))[j].time) {
			trackmin = track;
			mintime = (*((*p_stream)[track].code))[j].time;
			}
		}
	if(mintime < ZERO) break;
	j = (*p_stream)[trackmin].i;
	(*(Stream.code))[i] = (*((*p_stream)[trackmin].code))[j];
	Stream.i++;
	(*p_stream)[trackmin].i++;
	}
Stream.imax = Stream.i;
Stream.i = ZERO;
ptr2 = Stream.code;
MySetHandleSize((Handle*)&ptr2,(Size)(Stream.imax) * sizeof(MIDIcode));
Stream.code = ptr2;

OUT:
MyDisposeHandle((Handle*)&p_buffer);

if(p_stream != NULL) {
	for(track=0; track < ntracks; track++) {
		ptr = (Handle)(*p_stream)[track].code;
		MyDisposeHandle(&ptr);
		}
	MyDisposeHandle((Handle*)&p_stream);
	}
sprintf(Message,"%ld bytes have been read on %ld track(s)",(long) totalbytes + nbytes,(long) ntracks);
ShowMessage(TRUE,wInfo,Message);
if(severaltempo) 
	Alert1("This MIDI file contains changes of tempo that were ignored by this import procedure");
return(result);

ERR:
if(track >= 0 && track < ntracks)
	sprintf(Message,"Error on byte #%ld, track %ld. ",(long) totalbytes + nbytes,(long) track);
else
	sprintf(Message,"Error on byte #%ld. ",(long) totalbytes + nbytes);
switch(err) {
	case 0:
		break;
	case 1:
	case 8:
	case 12:
	case 13:
	case 14:
	case 15:
		strcat(Message,"Unexpected end of file...");
		break;
	case 19:
	case 20:
		strcat(Message,"Unexpected end of file while reading system-exclusive codes...");
		break;
	case 2:
		strcat(Message,"'MThd' not found...");
		break;
	case 5:
		strcat(Message,"Format > 2");
		break;
	case 9:
		strcat(Message,"'MTrk' not found...");
		break;
	case 6:
	case 10:
		strcat(Message,"Out of memory...");
		break;
	case 24:
		strcat(Message,"Missing 'End of track' meta-event. May be this file is still open for writing?");
		break;
	default: break;
	}
ShowMessage(TRUE,wMessage,Message);
Alert1("This file is incomplete or in a format that BP2 can't read");
result = FAILED;
goto OUT;
}
#endif /* BP_CARBON_GUI */


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
		SendToDriver(Tcurr * Time_res,0,&rs,&e);
		}
	}
return(OK);
}


int FadeOut(void)
{
int i,maxfadeout,rs,chan,value;
float fadeout;
Milliseconds time,timeorigin;
MIDI_Event e;

#if BP_CARBON_GUI
GetFileSavePreferences();
#endif /* BP_CARBON_GUI */

if(MIDIfadeOut > 0.) {
	maxfadeout = MIDIfadeOut * SamplingRate;
	rs = 0;
	timeorigin = LastTcurr * Time_res;
	for(i=1; i <= maxfadeout; i++) {
		time = timeorigin + ((1000L * i) / SamplingRate);
		for(chan=1; chan <= MAXCHAN; chan++) {
			if(CurrentVolume[chan] < 1) continue;
			value = CurrentVolume[chan] * ((((float)maxfadeout) - i) / maxfadeout);
			e.time = time / Time_res;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + chan - 1;
			e.data1 = VolumeControl[chan];
			e.data2 = value;
			SendToDriver(time,0,&rs,&e);
			}
		}
	for(chan=1; chan <= MAXCHAN; chan++) CurrentVolume[chan] = -1;
	}
return(OK);
}
