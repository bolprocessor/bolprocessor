/* midi1.h -- Modified by Bernard Bel */

/****************************************************************
 *       Constants and Types for:                               *
 *         Two port MIDI Driver for the Apple Macintosh         *
 *           based on Midi Driver by Lee Boynton.               *
 *       Copyright (c) Lee Boynton, MIT Media Lab, March 1988.  *
 *                                                              *
 ****************************************************************/

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

// Replaced OsErr with OSErr on 1/2/93 when using Think C 5.0.2
// Suppressed type ioParam1  (replaced with IOParam in files.h)

#ifndef BP2_MIDI1_H
#define BP2_MIDI1_H

#define NIL 0L

/* Set up addresses for the SCC registers so that we can access elegantly */
#define WRITEBASE  0x1dc
#define READBASE   0x1d8
#define ADATA	   6
#define BDATA      4
#define ACTRL      2
#define BCTRL      0

/*
 *
 * valid MIDI_Event types 
 *
 */ 
#define RAW_EVENT 0		/* data only in low order byte */
#define NORMAL_EVENT 1	/* data packed into three low order bytes */
#define NULL_EVENT 2	/* no data */
#define TWO_BYTE_EVENT 3	/* This I added on 5/10/97 (Bernard Bel) */

/* Add extensions for channel definitions as part of the type byte. */
#define PORTA 0
#define PORTB 32
#define PORTB_COMPLEMENT 0xffdf /* long complement of PORTB */

/*
 *
 * Control/Status codes 
 * These codes are common for both A & B ports
 */
#define CLOCKTIME_CODE 0		/* control/status */
#define TICKSIZE_CODE 1			/* control/status */
#define CHANNELIZE_CODE 2		/* control only */
#define CLEAR_SCHEDULER_CODE 3	/* control only */


	/* A port constants */

#define OPEN_CODEA 10			/* control only */
#define CLOSE_CODEA 11			/* control only */
#define RECEIVEON_CODEA  12		/* control only */
#define RECEIVEOFF_CODEA 13		/* control only */
#define RECEIVE_STATUS_CODEA 14	/* status only */
#define READMODE_CODEA 15       /* control/status */
#define FILTER_CODEA 16			/* control/status */
#define CLEAR_RECEIVE_CODEA 17	/* control only */
#define SIMREAD_CODEA 18		/* control only - simulate a midi receive interrupt*/
#define CLEAR_TRANSMIT_CODEA 19	/* control only */
#define TRANSMIT_STATUS_CODEA 20 /* status only */
#define TIMECODE_TRANSMIT_CODEA 21     /* control only */
#define TIMECODE_TRANSMIT_ENABLE_CODEA 22     /* control only */
#define TIMECODE_RECEIVE_CODEA 23     /* control only */
#define TIMECODE_RECEIVE_ENABLE_CODEA 24     /* control only */
#define READ_PORT_CODEA 25		 /* status only */
#define PITCHBENDA 26			/* control/status (with short channel argument) */


	/* Same constants for B */
	/* Just add 20 to codes for A */

#define OPEN_CODEB 30			/* control only */
#define CLOSE_CODEB 31			/* control only */
#define RECEIVEON_CODEB  32		/* control only */
#define RECEIVEOFF_CODEB 33		/* control only */
#define RECEIVE_STATUS_CODEB 34	/* status only */
#define READMODE_CODEB 35       /* control/status */
#define FILTER_CODEB 36			/* control/status */
#define CLEAR_RECEIVE_CODEB 37	/* control only */
#define SIMREAD_CODEB 38		/* control only - simulate a midi receive interrupt*/
#define CLEAR_TRANSMIT_CODEB 39	/* control only */
#define TRANSMIT_STATUS_CODEB 40 /* status only */
#define READ_PORT_CODEB 45		 /* status only */
#define PITCHBENDB 46			/* control/status (with short channel argument) */

#define DRIVER_ADDRESS 100
/*
 *
 * Operating parameters
 *
 */
#define DEFAULT_TICK_SIZE 10L   /* in milliseconds */

#define MIDI_FIFO_MAX 4096-1	/* must be power of two minus one */
#define TASK_TABLE_SIZE 2048*2
#define	WAIT_CACHE_SIZE 2048

/*
 *
 * MIDI opcodes
 *
 */
#define MIDI_NoteOffType 0x80
#define MIDI_NoteOnType 0x90
#define MIDI_PolyTouchType 0xa0
#define MIDI_ControlType 0xb0
#define MIDI_ProgramType 0xc0
#define MIDI_AfterTouchType 0xd0
#define MIDI_PitchBendType 0xe0
#define MIDI_SysRealTimeType 0xf0

#define MIDI_SysXType 0xf0
#define MIDI_TimeCodeType 0xf1
#define MIDI_SongPosType 0xf2
#define MIDI_SongSelType 0xf3
#define MIDI_TuneType 0xf6
#define MIDI_EndSysXType 0xf7
#define MIDI_ClockType 0xf8
#define MIDI_StartType 0xfa
#define MIDI_ContType 0xfb
#define MIDI_StopType 0xfc
#define MIDI_ActiveSenseType 0xfe
#define MIDI_ResetType 0xff


/*
 *
 * MIDI_Event structure used by Read and Write.
 *
 */
typedef struct {
	long time;
	unsigned char type,status,data1,data2;
} MIDI_Event, *MIDI_EventPtr;

/*
 *
 * Task structure (used internally only)
 *
 */
typedef struct _Task {
	long exectime;
	MIDI_Event event;
	struct _Task *link;
} Task, *TaskPtr;

/*
 *
 * Driver Data Block (used internally only)
 *
 * Placing this structure in the .h file is a severe ABSTRACTION VIOLATION !!!
 * It is very usefull and efficient to have access to this for debugging.
 *
 */
typedef struct {
	long Mac_qLink;			/**													**/
	short Mac_qType;		/** The first 4 entries are for the Time Manager	**/
	ProcPtr clockHandler;	/**													**/
	long Mac_tCount;		/**													**/
	long time;
	Handle oldStorage;
	long inTime,outTime,tickSize;
	long *timeCache;
	char *ctrlA,*ctrlB,*ctrl,*data,*stat;
	
	/* Things here are separated for both channels for some data.
	   The data for each channel is denoted by the uppercase letter
	   end of the name for each field.
	   The fields may change as we uderstand more about how we want
	   each channel top act. (ie as we know more about what info we
	   want to keep about each channel.
	 */
	 
	long *rcvFIFOA;
	char *xmtFIFOA;
	short xmtCountA,rcvCountA,rcvPutIndexA,rcvGetIndexA,xmtPutIndexA,xmtGetIndexA;
	short xmtIdleA,rcvErrorA,xmtErrorA,rcvEnableA,readModeA;
	
	long *rcvFIFOB;
	char *xmtFIFOB;
	short xmtCountB,rcvCountB,rcvPutIndexB,rcvGetIndexB,xmtPutIndexB,xmtGetIndexB;
	short xmtIdleB,rcvErrorB,xmtErrorB,rcvEnableB,readModeB;

	short eventStatusA,eventData1A,eventData2A,stateA,lastStatusA,dataCountA, OpenA;
	short eventStatusB,eventData1B,eventData2B,stateB,lastStatusB,dataCountB, OpenB;

	long FilterMaskA, FilterMaskB;
	short active;
	TaskPtr taskTable,*waitQueue,*waitQueueTail,freeTasks;
	
	unsigned char xmtTCMessageA,xmtTCHourA,xmtTCMinuteA;
	unsigned char xmtTCSecondA,xmtTCFrameA;
	long xmtTCStartA,xmtTCEndA;
	short (*xmtTCProcsA[8])();
	
	unsigned char rcvTCHourA,rcvTCMinuteA,rcvTCSecondA,rcvTCFrameA;
	unsigned char rcvTCHourTmpA,rcvTCMinuteTmpA;
	unsigned char rcvTCSecondTmpA,rcvTCFrameTmpA;
	short rcvTCEnableA,rcvTCGotF1A;
	long rcvTCTimeA;
	short (*rcvTCProcsA[8])();
	
	short PitchbendA[16], PitchbendB[16], SamplePitchbendA, SamplePitchbendB;
		
	/* Last event stuff added to be able to handle time-dependent reading
	   from either port
	 */
	
	MIDI_Event lastevent;
	short lastvalid;
	
} DriverData,*DriverDataPtr;

#endif /* BP2_MIDI1_H */
