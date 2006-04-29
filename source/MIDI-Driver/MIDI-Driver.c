
/****************************************************************
 *																*
 *		  Two Port MIDI Driver for the Apple Macintosh.			*
 *			based on Midi Driver by Lee Boynton.				*
 *		Copyright (c) Lee Boynton, MIT Media Lab, March 1988.	*
 *																*
 ****************************************************************/

// Modified 31/10/96  Bernard Bel
// #include "Mac #includes.h"
#include "midi1.h"
#include "Other includes.h"
 
/*  Changes:
	2/16/90	 Joe
			 Attempted Pasteurization of driver.

	1/18/90	 Joe
			 Added pitch bend sampling. (Not officially supported)
			 
	12/9/89  Joe
	         Added MIDI Time Code output at 25 frames per second. (i.e.
	         a quarter frame every centisecond tick. (Not officially supported)
	         
	         Added read port Control interface to read a particular port
	         directly.
	         
	         Rewrote the Channelize code to take the event & time too.
	         
	7/1/89	 Joe
			 Fixed a bug which allowed scheduled messages to interrupt
			 and trash normal writes with catastrophic failure as the
			 result.
			 
	01/10/89 Joe
			 Added driver channelization of midievents. Call the
			 CHANNELIZE_CODE control with a 16bit word indicating
			 which channels.
			 
	01/09/89 Anthony Added receive filters for midi events.

	01/04/89 Anthony changed the size of the task table and the event
			 wait queue to 4*1024...

	12/17/88 Anthony and Joe
			 Modified to use ports A and B of the SCC. Added Last_Event stuff
			 to the driver data block etc....

	08/02/88 Joe
			 Fixed a bug that was letting real time messages trash
			 running status messages. Also, reset running status when
			 sysx or system common messages were received. Lastly,
			 initialized the driver status variable to 0 in driver_open.
	
	2/3/92 BB
			Adapted project to Think C 5
			
*/


/*
 *
 * Low Level stuff
 *
 */
#define Protect \
	asm { move.w sr,-(a7) } \
	asm {ori.w #0x0700,sr }

#define Unprotect asm { move.w (a7)+,sr }

#define BeginHandler(name) DriverDataPtr name=GetDriverDataPtr(); \
	asm { movem.l	a4-a6/d4-d7,-(a7) } \
	asm { move.l 0x904,a5 }

#define EndHandler() asm { movem.l	(a7)+,a4-a6/d4-d7 }

/*
 *
 * GetDriverDataPtr - this hacked routine returns a pointer to the data segment
 *
 */
DriverDataPtr GetDriverDataPtr ()
{
	asm { move.l	#0x12345678,d0 }
}

/*
 *
 * InstallHack - modify GetDriverDataPtr's code at run time to provide
 * an accurate pointer to wherever the data block got loaded.
 *
 */
InstallHack(baseAddr,data)
	long baseAddr,data;
{
	long *addr;
	if(*((short*)baseAddr) == 0x4ef9)
		baseAddr = *((long*)(baseAddr + 2L));
	addr = (long *)(baseAddr + 2L);
	*addr = data;
}

/*
 *
 *	Z8530 Serial Communications Controller (SCC) for modem port MIDI (port A)
 *
 */
SCC_WriteCtrlA(data)
	short data;
{
	asm {
		move.w	data,d0
		move.l	WRITEBASE,a0
		move.b	d0,ACTRL(a0)
		move.l (sp),(sp)
		move.l (sp),(sp)
		move.l (sp),(sp)
		move.l (sp),(sp)
	}
}

SCC_ReadDataA()
{
	asm {
		move.l	READBASE,a0
		clr.l	d0
		move.b	ADATA(a0),d0
	}
}

SCC_WriteDataA(data)
	short data;
{
	asm {
		move.w	data,d0
		move.l	WRITEBASE,a0
		move.b	d0,ADATA(a0)
	}
}

/*
 *
 *	Z8530 Serial Communications Controller (SCC) for printer port MIDI (port B)
 *
 */
SCC_WriteCtrlB(data)
	short data;
{
	asm {
		move.w	data,d0
		move.l	WRITEBASE,a0
		move.b	d0,BCTRL(a0)
		move.l (sp),(sp)
		move.l (sp),(sp)
		move.l (sp),(sp)
		move.l (sp),(sp)
	}
}

SCC_ReadDataB()
{
	asm {
		move.l	READBASE,a0
		clr.l	d0
		move.b	BDATA(a0),d0
	}
}

SCC_WriteDataB(data)
	short data;
{
	asm {
		move.w	data,d0
		move.l	WRITEBASE,a0
		move.b	d0,BDATA(a0)
	}
}


/*
 *
 * MIDI support
 *
 */
long MIDI_ReadA(driver)
	DriverDataPtr driver;
{
	long entry;
	if (driver->rcvCountA) {
		entry = driver->rcvFIFOA[driver->rcvGetIndexA++];
		driver->rcvCountA--;
		driver->rcvGetIndexA = driver->rcvGetIndexA & MIDI_FIFO_MAX;
		return(entry);
	} else {
		return(-1L);
	}
}

long MIDI_ReadB(driver)
	DriverDataPtr driver;
{
	long entry;
	if (driver->rcvCountB) {
		entry = driver->rcvFIFOB[driver->rcvGetIndexB++];
		driver->rcvCountB--;
		driver->rcvGetIndexB = driver->rcvGetIndexB & MIDI_FIFO_MAX;
		return(entry);
	} else {
		return(-1L);
	}
}

MIDI_WriteA(driver,byte)
	DriverDataPtr driver;
	unsigned char byte;
{
	if (driver->xmtCountA <= MIDI_FIFO_MAX) {
		driver->xmtFIFOA[driver->xmtPutIndexA++] = byte;
		driver->xmtPutIndexA = driver->xmtPutIndexA & MIDI_FIFO_MAX;
		if (driver->xmtIdleA) {
			driver->xmtCountA++;
			Transmit_HandlerA();
		} else
			driver->xmtCountA++;
	} else
		driver->xmtErrorA++;
}


MIDI_WriteB(driver,byte)
	DriverDataPtr driver;
	unsigned char byte;
{
	if (driver->xmtCountB <= MIDI_FIFO_MAX) {
		driver->xmtFIFOB[driver->xmtPutIndexB++] = byte;
		driver->xmtPutIndexB = driver->xmtPutIndexB & MIDI_FIFO_MAX;
		if (driver->xmtIdleB) {
			driver->xmtCountB++;
			Transmit_HandlerB();
		} else
			driver->xmtCountB++;
	} else
		driver->xmtErrorB++;
}

/*
 *
 * Time Code support
 *
 */

short RcvQuarterFrameA (driver, midibyte)
	DriverDataPtr driver;
	unsigned char midibyte;
{
	driver->time++;
	Scheduler_ExecuteReadyTasks(driver);
	(*(driver->rcvTCProcsA[midibyte >> 4]))(driver, midibyte);
}

short RcvFrameLsA(driver, midibyte)
	DriverDataPtr driver;
	unsigned char midibyte;
{
	driver->rcvTCFrameTmpA = (driver->rcvTCFrameTmpA & 0xf0) |
							 (midibyte & 0x0f);
}

short RcvFrameMsA(driver, midibyte)
	DriverDataPtr driver;
	unsigned char midibyte;
{
	driver->rcvTCFrameTmpA = (driver->rcvTCFrameTmpA & 0x0f) |
							 ((midibyte & 1) << 4);
}

short RcvSecondLsA(driver, midibyte)
	DriverDataPtr driver;
	unsigned char midibyte;
{
	driver->rcvTCSecondTmpA = (driver->rcvTCSecondTmpA & 0xf0) |
							  (midibyte & 0x0f);
}

short RcvSecondMsA(driver, midibyte)
	DriverDataPtr driver;
	unsigned char midibyte;
{
	driver->rcvTCSecondTmpA = (driver->rcvTCSecondTmpA & 0x0f) |
							  ((midibyte & 3) << 4);
}

short RcvMinuteLsA(driver, midibyte)
	DriverDataPtr driver;
	unsigned char midibyte;
{
	driver->rcvTCMinuteTmpA = (driver->rcvTCMinuteTmpA & 0xf0) |
							  (midibyte & 0x0f);
}

short RcvMinuteMsA(driver, midibyte)
	DriverDataPtr driver;
	unsigned char midibyte;
{
	driver->rcvTCMinuteTmpA = (driver->rcvTCMinuteTmpA & 0x0f) |
							  ((midibyte & 3) << 4);
}

short RcvHourLsA(driver, midibyte)
	DriverDataPtr driver;
	unsigned char midibyte;
{
	driver->rcvTCHourTmpA = (driver->rcvTCHourTmpA & 0xf0) |
							(midibyte & 0x0f);
}

short RcvHourMsA(driver, midibyte)
	DriverDataPtr driver;
	unsigned char midibyte;
{
	driver->rcvTCHourTmpA = (driver->rcvTCHourTmpA & 0x0f) |
							((midibyte & 1) << 4);
	driver->rcvTCHourA = driver->rcvTCHourTmpA;
	driver->rcvTCMinuteA = driver->rcvTCMinuteTmpA;
	driver->rcvTCSecondA = driver->rcvTCSecondTmpA;
	driver->rcvTCFrameA = driver->rcvTCFrameTmpA;
	driver->rcvTCTimeA = driver->time - 8L;
}

/*
 *
 * Interrupt handlers for transmit and receive
 *
 */

Transmit_HandlerA()
{
	BeginHandler(driver);
	if (!driver->xmtIdleA) {
		SCC_WriteCtrlA(0);
		SCC_WriteCtrlA(0x28);
	}
	if (driver->xmtCountA) {
		driver->xmtCountA--;
		driver->xmtIdleA = 0;
		SCC_WriteDataA(driver->xmtFIFOA[driver->xmtGetIndexA++]);
		driver->xmtGetIndexA = driver->xmtGetIndexA & MIDI_FIFO_MAX;
	} else
		{
		driver->xmtIdleA = 1;
		}
	EndHandler();
}

Transmit_HandlerB()
{
	BeginHandler(driver);
	if (!driver->xmtIdleB) {
		SCC_WriteCtrlB(0);
		SCC_WriteCtrlB(0x28);
	}
	if (driver->xmtCountB) {
		driver->xmtCountB--;
		driver->xmtIdleB = 0;
		SCC_WriteDataB(driver->xmtFIFOB[driver->xmtGetIndexB++]);
		driver->xmtGetIndexB = driver->xmtGetIndexB & MIDI_FIFO_MAX;
	} else
		driver->xmtIdleB = 1;
	EndHandler();
}


TErr_HandlerA()
{
	BeginHandler(driver);
	driver->xmtErrorA++;
	SCC_WriteCtrlA(0);
	SCC_WriteCtrlA(0x28);
	EndHandler();
}

TErr_HandlerB()
{
	BeginHandler(driver);
	driver->xmtErrorB++;
	SCC_WriteCtrlB(0);
	SCC_WriteCtrlB(0x28);
	EndHandler();
}


Receive_HandlerA()
{
	unsigned char midibyte;
	BeginHandler(driver);
	midibyte = SCC_ReadDataA();
	if (driver->rcvTCEnableA) {
		if (midibyte == MIDI_TimeCodeType) {
			driver->rcvTCGotF1A = 1;
		} else if (driver->rcvTCGotF1A) {
			if (midibyte < 0x80) {			/* normal data byte */
				RcvQuarterFrameA(driver,midibyte);
				driver->rcvTCGotF1A = 0;
			} else if (midibyte < 0xf8) 	/* oops! got an unexpected status byte! */
				driver->rcvTCGotF1A = 0;	/* forget that I ever saw the f1 */
			else; 							/* ignore intervening real time byte */
		}
	}
	if ((driver->rcvCountA <= MIDI_FIFO_MAX) && driver->rcvEnableA) {
		driver->rcvCountA++;
		driver->rcvFIFOA[driver->rcvPutIndexA++] =
			(((long)midibyte << 24) | (driver->time & 0x00ffffff));
		driver->rcvPutIndexA = driver->rcvPutIndexA & MIDI_FIFO_MAX;
	} else {
		driver->rcvErrorA++;
	}
	EndHandler();
}

Receive_HandlerB()
{
	BeginHandler(driver);
	if ((driver->rcvCountB <= MIDI_FIFO_MAX) && driver->rcvEnableB) {
		driver->rcvCountB++;
		driver->rcvFIFOB[driver->rcvPutIndexB++] =
			(((long)SCC_ReadDataB() << 24) | (driver->time & 0x00ffffff));
		driver->rcvPutIndexB = driver->rcvPutIndexB & MIDI_FIFO_MAX;
	} else {
		driver->rcvErrorB++;
		SCC_ReadDataB();
	}
	EndHandler();
}


RErr_HandlerA()
{
	BeginHandler(driver);
	SCC_ReadDataA();
	driver->rcvErrorA++;
	EndHandler();
}

RErr_HandlerB()
{
	BeginHandler(driver);
	SCC_ReadDataB();
	driver->rcvErrorB++;
	EndHandler();
}


/* 
 *	Transmit procedures 
 */
 
short XmtTCSet(driver, hour, minute, second, frame)
	DriverDataPtr driver;
	unsigned char hour, minute, second, frame;
{
	Protect;
	driver->xmtTCHourA = hour;
	driver->xmtTCMinuteA = minute;
	driver->xmtTCSecondA = second;
	driver->xmtTCFrameA = frame;
	driver->xmtTCMessageA = 0;
	
	MIDI_WriteA(driver, 0xf0);		/* write full message */
	MIDI_WriteA(driver, 0x7f);
	MIDI_WriteA(driver, 0x7f);
	MIDI_WriteA(driver, 0x01);
	MIDI_WriteA(driver, 0x01);
	MIDI_WriteA(driver, 0x20 | driver->xmtTCHourA);
	MIDI_WriteA(driver, driver->xmtTCMinuteA);
	MIDI_WriteA(driver, driver->xmtTCSecondA);
	MIDI_WriteA(driver, driver->xmtTCFrameA);
	MIDI_WriteA(driver, 0xf7);	
	Unprotect;
}

short XmtFrameLsA(driver)
	DriverDataPtr driver;
{
	MIDI_WriteA(driver, 0xf1);
	MIDI_WriteA(driver, (driver->xmtTCFrameA & 0x0f));
}

short XmtFrameMsA(driver)
	DriverDataPtr driver;
{
	MIDI_WriteA(driver, 0xf1);
	MIDI_WriteA(driver, (0x10 | (driver->xmtTCFrameA >> 4)));
}

short XmtSecondLsA(driver)
	DriverDataPtr driver;
{
	MIDI_WriteA(driver, 0xf1);
	MIDI_WriteA(driver, (0x20 | (driver->xmtTCSecondA & 0x0f)));
}

short XmtSecondMsA(driver)
	DriverDataPtr driver;
{
	MIDI_WriteA(driver, 0xf1);
	MIDI_WriteA(driver, (0x30 | (driver->xmtTCSecondA >> 4)));
}

short XmtMinuteLsA(driver)
	DriverDataPtr driver;
{
	MIDI_WriteA(driver, 0xf1);
	MIDI_WriteA(driver, (0x40 | (driver->xmtTCMinuteA & 0x0f)));
}

short XmtMinuteMsA(driver)
	DriverDataPtr driver;
{
	MIDI_WriteA(driver, 0xf1);
	MIDI_WriteA(driver, (0x50 | (driver->xmtTCMinuteA >> 4)));
}

short XmtHourLsA(driver)
	DriverDataPtr driver;
{
	MIDI_WriteA(driver, 0xf1);
	MIDI_WriteA(driver, (0x60 | (driver->xmtTCHourA & 0x0f)));
}

short XmtHourMsA(driver)
	DriverDataPtr driver;
{
	MIDI_WriteA(driver, 0xf1);
	MIDI_WriteA(driver, (0x72 | (driver->xmtTCHourA >> 4)));
	driver->xmtTCFrameA = driver->xmtTCFrameA + 2;
	if (driver->xmtTCFrameA > 24) {
		driver->xmtTCFrameA = driver->xmtTCFrameA - 25;
		driver->xmtTCSecondA++;
		if (driver->xmtTCSecondA > 59) {
			driver->xmtTCSecondA = 0;
			driver->xmtTCMinuteA++;
			if (driver->xmtTCMinuteA > 59) {
				driver->xmtTCMinuteA = 0;
				driver->xmtTCHourA++;
				if (driver->xmtTCHourA > 23) {
					driver->xmtTCHourA = 0;
				}
			}
		}
	}
}
/*
 *
 * MIDI_Open - install hacks in the MIDI interrupt handler, initialize vectors and the SCC.
 *
 */
MIDI_OpenA(driver)
	DriverDataPtr driver;
{
	int i;
	driver->rcvPutIndexA = driver->xmtPutIndexA = 0;
	driver->rcvGetIndexA = driver->xmtGetIndexA = 0;
	driver->rcvCountA = driver->xmtCountA = 0;
	driver->xmtIdleA = driver->rcvEnableA = 1;
	driver->rcvErrorA=driver->xmtErrorA=0;
	driver->FilterMaskA = 0x7fL;			/* Pass Channel Messages Only */
	for (i=0; i<16; i++)
		driver->PitchbendA[i] = 8192;
	driver->SamplePitchbendA = 0;
	
	driver->xmtTCProcsA[0] = XmtFrameLsA;
	driver->xmtTCProcsA[1] = XmtFrameMsA;
	driver->xmtTCProcsA[2] = XmtSecondLsA;
	driver->xmtTCProcsA[3] = XmtSecondMsA;
	driver->xmtTCProcsA[4] = XmtMinuteLsA;
	driver->xmtTCProcsA[5] = XmtMinuteMsA;
	driver->xmtTCProcsA[6] = XmtHourLsA;
	driver->xmtTCProcsA[7] = XmtHourMsA;
	driver->xmtTCEndA = driver->xmtTCStartA = 0;
	driver->rcvTCProcsA[0] = RcvFrameLsA;
	driver->rcvTCProcsA[1] = RcvFrameMsA;
	driver->rcvTCProcsA[2] = RcvSecondLsA;
	driver->rcvTCProcsA[3] = RcvSecondMsA;
	driver->rcvTCProcsA[4] = RcvMinuteLsA;
	driver->rcvTCProcsA[5] = RcvMinuteMsA;
	driver->rcvTCProcsA[6] = RcvHourLsA;
	driver->rcvTCProcsA[7] = RcvHourMsA;
	driver->rcvTCEnableA = driver->rcvTCGotF1A = 0;
		
	Protect;
	SCC_WriteCtrlA(0x09);
	SCC_WriteCtrlA(0x80);
	SCC_WriteCtrlA(0x04);
	SCC_WriteCtrlA(0x84);
	SCC_WriteCtrlA(0x01);
	SCC_WriteCtrlA(0x60);
	SCC_WriteCtrlA(0x03);
	SCC_WriteCtrlA(0xc0);
	SCC_WriteCtrlA(0x05);
	SCC_WriteCtrlA(0x62);
	SCC_WriteCtrlA(0x0b);
	SCC_WriteCtrlA(0x28);
	SCC_WriteCtrlA(0x03);
	SCC_WriteCtrlA(0xc1);
	SCC_WriteCtrlA(0x05);
	SCC_WriteCtrlA(0x6a);
	SCC_WriteCtrlA(0x0f);
	SCC_WriteCtrlA(0x08);
	SCC_WriteCtrlA(0x00);
	SCC_WriteCtrlA(0x10);
	SCC_WriteCtrlA(0x00);
	SCC_WriteCtrlA(0x10);
	SCC_WriteCtrlA(0x01);
	SCC_WriteCtrlA(0xf3);
	SCC_WriteCtrlA(0x09);
	SCC_WriteCtrlA(0x0a);

	*((long*)0x1c2) = (long)Transmit_HandlerA;
/*	*((long*)0x2c6) = (long)TErr_HandlerA;	*/
	*((long*)0x1ca) = (long)Receive_HandlerA;
	*((long*)0x1ce) = (long)RErr_HandlerA;
	driver->OpenA = 1;
	Unprotect;
}

MIDI_OpenB(driver)
	DriverDataPtr driver;
{
	int i;
	driver->rcvPutIndexB = driver->xmtPutIndexB = 0;
	driver->rcvGetIndexB = driver->xmtGetIndexB = 0;
	driver->rcvCountB = driver->xmtCountB = 0;
	driver->xmtIdleB = driver->rcvEnableB = 1;
	driver->rcvErrorB=driver->xmtErrorB=0;
	driver->FilterMaskB = 0x7fL;			/* Pass Channel Messages Only */
	for (i=0; i<16; i++)
		driver->PitchbendB[i] = 8192;
	driver->SamplePitchbendB = 0;
	
	Protect;
	SCC_WriteCtrlB(0x09);
	SCC_WriteCtrlB(0x40);				/* this changed to reset channel B */
	SCC_WriteCtrlB(0x04);				/* So far everything else is the same as for A */
	SCC_WriteCtrlB(0x84);
	SCC_WriteCtrlB(0x01);
	SCC_WriteCtrlB(0x60);
	SCC_WriteCtrlB(0x03);
	SCC_WriteCtrlB(0xc0);
	SCC_WriteCtrlB(0x05);
	SCC_WriteCtrlB(0x62);
	SCC_WriteCtrlB(0x0b);
	SCC_WriteCtrlB(0x28);
	SCC_WriteCtrlB(0x03);
	SCC_WriteCtrlB(0xc1);
	SCC_WriteCtrlB(0x05);
	SCC_WriteCtrlB(0x6a);
	SCC_WriteCtrlB(0x0f);
	SCC_WriteCtrlB(0x08);
	SCC_WriteCtrlB(0x00);
	SCC_WriteCtrlB(0x10);
	SCC_WriteCtrlB(0x00);
	SCC_WriteCtrlB(0x10);
	SCC_WriteCtrlB(0x01);
	SCC_WriteCtrlB(0xf3);
	SCC_WriteCtrlB(0x09);
	SCC_WriteCtrlB(0x0a);

	*((long*)0x1b2) = (long)Transmit_HandlerB;
/*	*((long*)0x2c6) = (long)TErr_HandlerB;	*/				/* We Aren't sure about this yet*/
	*((long*)0x1ba) = (long)Receive_HandlerB;
	*((long*)0x1be) = (long)RErr_HandlerB;
	driver->OpenB=1;
	Unprotect;
}

MIDI_CloseA(driver)
	DriverDataPtr driver;
{
	Protect;
	SCC_WriteCtrlA(1);
	SCC_WriteCtrlA(0xe1);
	Unprotect;
	driver->OpenA=0;
}

MIDI_CloseB(driver)
	DriverDataPtr driver;
{
	Protect;
	SCC_WriteCtrlB(1);
	SCC_WriteCtrlB(0xe1);
	Unprotect;
	driver->OpenB=0;
}


MIDI_ClearRcvA(driver)
	DriverDataPtr driver;
{
	Protect;
	driver->rcvPutIndexA = driver->xmtPutIndexA = 0;
	driver->rcvGetIndexA = driver->xmtGetIndexA = 0;
	driver->rcvCountA = driver->xmtCountA = 0;
	driver->xmtIdleA = 1;
	driver->rcvErrorA=0;
	Unprotect;
}

MIDI_ClearXmtA(driver)
	DriverDataPtr driver;
{
	Protect;
	driver->xmtPutIndexA = 0;	
	driver->xmtGetIndexA = 0;
	driver->xmtCountA = 0;
	driver->xmtIdleA = 1;
	driver->xmtErrorA = 0;
	Unprotect;
}

MIDI_ClearRcvB(driver)
	DriverDataPtr driver;
{
	Protect;
	driver->rcvPutIndexB = driver->xmtPutIndexB = 0;
	driver->rcvGetIndexB = driver->xmtGetIndexB = 0;
	driver->rcvCountB = driver->xmtCountB = 0;
	driver->xmtIdleB = 1;
	driver->rcvErrorB=0;
	Unprotect;
}

MIDI_ClearXmtB(driver)
	DriverDataPtr driver;
{
	Protect;
	driver->xmtPutIndexB = 0;	
	driver->xmtGetIndexB = 0;
	driver->xmtCountB = 0;
	driver->xmtIdleB = 1;
	driver->xmtErrorB = 0;
	Unprotect;
}

/*
 *
 * Clock support
 *
 */
Clock_Handler ()
{
	BeginHandler(driver);
	PrimeTime((QElemPtr) driver,driver->tickSize);
	if (!driver->rcvTCEnableA) {
		driver->time++;
		if ((driver->time < driver->xmtTCEndA) &&
			(driver->time >= driver->xmtTCStartA)) {
			(*(driver->xmtTCProcsA[driver->xmtTCMessageA++]))(driver);
			driver->xmtTCMessageA = driver->xmtTCMessageA & 0x07;
		}
		Scheduler_ExecuteReadyTasks(driver);
	}
	EndHandler();
}

Clock_Open(driver)
	DriverDataPtr driver;
{
	driver->time = 0;
	driver->outTime = driver->inTime = 0;
	driver->clockHandler = (ProcPtr) Clock_Handler;
	driver->tickSize = DEFAULT_TICK_SIZE;
	InsTime((QElemPtr) driver);
	PrimeTime((QElemPtr) driver,driver->tickSize);
}


Clock_Close(driver)
	DriverDataPtr driver;
{
	Protect;
	RmvTime((QElemPtr) driver);
	Unprotect;
}

/*
 *
 * Scheduler support
 *
 */
Scheduler_ExecuteReadyTasks(driver)
	DriverDataPtr driver;
{
	TaskPtr task,remainingTasks,laterHead,laterTail;
	long time=driver->time;
	short index;
	if(driver->active){
		index=(short)(time % (long)WAIT_CACHE_SIZE);
		remainingTasks=driver->waitQueue[index];
		laterHead = NIL;
		while (task=remainingTasks) {
			remainingTasks=task->link;
			if (task->exectime > time) {
				task->link = NIL;
				if (laterHead) {
					laterTail->link = task;
					laterTail = task;
				} else laterHead = laterTail = task;
			} else {
				MIDI_PutEvent(driver,&task->event);
				task->link = driver->freeTasks;
				driver->freeTasks = task;
			}
		}
		driver->waitQueue[index] = laterHead;
		driver->waitQueueTail[index] = laterTail;
	}
}

Scheduler_WaitTask(driver,task,exectime)
	DriverDataPtr driver;
	TaskPtr task;
	long exectime;
{
	short i=exectime%(long)WAIT_CACHE_SIZE;
	TaskPtr head;
	if (head = driver->waitQueue[i]) {
		driver->waitQueueTail[i]->link = task;
		driver->waitQueueTail[i] = task;
	} else driver->waitQueueTail[i] = driver->waitQueue[i] = task;
}

Schedule_Event(driver,event)
	DriverDataPtr driver;
	MIDI_Event *event;
{
	TaskPtr task=driver->freeTasks;

	if (task) {
		driver->freeTasks=task->link;
		task->event = *event;
		task->exectime = event->time;
		task->link = NIL;
		Scheduler_WaitTask(driver,task,task->exectime);
	} else {
		if(event->type&PORTB) driver->xmtErrorB++;
		else driver->xmtErrorA++;
	}
}

Scheduler_Open(driver)
	DriverDataPtr driver;
{
	short i;
	TaskPtr task;

	driver->active = false;
	driver->freeTasks=task=driver->taskTable;
	for (i=0; i<WAIT_CACHE_SIZE; i++)
		driver->waitQueue[i] = NIL;
	for (i=1; i<TASK_TABLE_SIZE; i++) {
		task->link = &(driver->taskTable[i]);
		task = task->link;
	}
	task->link = NIL;
	driver->active = true;
}

Scheduler_Clear(driver)
	DriverDataPtr driver;
{
	Scheduler_Open(driver);
}


Scheduler_Close(driver)
	DriverDataPtr driver;
{
	driver->active=false;
}

/*
 *
 * MIDI event structure support
 *
 */

MIDI_BuildEventA(driver,type,event)
	DriverDataPtr driver;
	unsigned char type;
	MIDI_Event *event;
{
	event->time=driver->inTime;
	event->type=type;
	event->status=driver->eventStatusA;
	event->data1=driver->eventData1A;
	event->data2=driver->eventData2A;
}

MIDI_BuildEventB(driver,type,event)
	DriverDataPtr driver;
	unsigned char type;
	MIDI_Event *event;
{
	event->time=driver->inTime;
	event->type=type;
	event->status=driver->eventStatusB;
	event->data1=driver->eventData1B;
	event->data2=driver->eventData2B;
}
		
		
/*
 *
 * MIDI_ParseStatusByte - parse the given status byte. If it is a valid single-byte
 *	status (like some SystemRealTime commands), return true. Otherwise, set up the
 *	parser state variables and return false. 
 *
 */
MIDI_ParseStatusByteA(driver,status)
	DriverDataPtr driver;
	short status;
{
	short type=status&240;
	
	switch (type) {
		case MIDI_ProgramType:
		case MIDI_AfterTouchType:
			driver->eventStatusA=driver->lastStatusA=status;
			driver->stateA=1;
			return(FALSE);
		case MIDI_NoteOffType:
		case MIDI_NoteOnType:
		case MIDI_PolyTouchType:
		case MIDI_ControlType:
			driver->eventStatusA=driver->lastStatusA=status;
			driver->stateA=2;
			driver->dataCountA=0;
			return(FALSE);
		case MIDI_PitchBendType:
			driver->eventStatusA=driver->lastStatusA=status;
			if (driver->SamplePitchbendA) driver->stateA=4; else driver->stateA=2;
			driver->dataCountA=0;
			return(FALSE);
		case MIDI_SysRealTimeType:
			driver->eventStatusA=status;
			switch (status) {
				case MIDI_TimeCodeType: /* added joe 12/21/89 */
					driver->stateA=driver->dataCountA=1;					
					driver->lastStatusA=0;
					return(FALSE);
				case MIDI_SongPosType:
					driver->stateA=2;
					driver->dataCountA=0;
					driver->lastStatusA=0; /* added joe 8/2/88 */
					return(FALSE);
				case MIDI_SongSelType:
					driver->stateA=driver->dataCountA=1;					
					driver->lastStatusA=0; /* added joe 8/2/88 */
					return(FALSE);
				case MIDI_SysXType:
					driver->stateA=3;
					driver->lastStatusA=0; /* added joe 8/2/88 */
					return(TRUE);
				case MIDI_EndSysXType:
					driver->stateA=0;
					driver->lastStatusA=0; /* added joe 8/2/88 */
					return(TRUE);
				case MIDI_TuneType:
					driver->lastStatusA=0; /* added joe 8/2/88 */
					return(TRUE);
					
				case MIDI_StartType:  /* joe - pass these */
				case MIDI_ContType:
				case MIDI_StopType:
				case MIDI_ClockType:
				case MIDI_ActiveSenseType:
				case MIDI_ResetType:
					return(TRUE); 
				default:
					return(FALSE); /* filter these out */

			}
		default:
			driver->stateA=0;
			return(FALSE);
	}
}

MIDI_ParseStatusByteB(driver,status)
	DriverDataPtr driver;
	short status;
{
	short type=status&240;
	
	switch (type) {
		case MIDI_ProgramType:
		case MIDI_AfterTouchType:
			driver->eventStatusB=driver->lastStatusB=status;
			driver->stateB=1;
			return(FALSE);
		case MIDI_NoteOffType:
		case MIDI_NoteOnType:
		case MIDI_PolyTouchType:
		case MIDI_ControlType:
			driver->eventStatusB=driver->lastStatusB=status;
			driver->stateB=2;
			driver->dataCountB=0;
			return(FALSE);
		case MIDI_PitchBendType:
			driver->eventStatusB=driver->lastStatusB=status;
			if (driver->SamplePitchbendB) driver->stateB=4; else driver->stateB=2;
			driver->dataCountB=0;
			return(FALSE);
		case MIDI_SysRealTimeType:
			driver->eventStatusB=status;
			switch (status) {
				case MIDI_TimeCodeType: /* added joe 12/21/89 */
					driver->stateB=driver->dataCountB=1;					
					driver->lastStatusB=0;
					return(FALSE);
				case MIDI_SongPosType:
					driver->stateB=2;
					driver->dataCountB=0;
					driver->lastStatusB=0; /* added joe 8/2/88 */
					return(FALSE);
				case MIDI_SongSelType:
					driver->stateB=driver->dataCountB=1;					
					driver->lastStatusB=0; /* added joe 8/2/88 */
					return(FALSE);
				case MIDI_SysXType:
					driver->stateB=3;
					driver->lastStatusB=0; /* added joe 8/2/88 */
					return(TRUE);
				case MIDI_EndSysXType:
					driver->stateB=0;
					driver->lastStatusB=0; /* added joe 8/2/88 */
					return(TRUE);
				case MIDI_TuneType:
					driver->lastStatusB=0; /* added joe 8/2/88 */
					return(TRUE);
				
				case MIDI_StartType:	/* joe - pass these... */
				case MIDI_ContType:
				case MIDI_StopType:
				case MIDI_ClockType:
				case MIDI_ActiveSenseType:
				case MIDI_ResetType:
				return(TRUE);
			
				default:
					return(FALSE); /* filter these out */
			}
		default:
			driver->stateB=0;
			return(FALSE);
	}
}

/*
 *
 * MIDI_ParseByte - parse the given MIDI byte. If the byte completes an event, then
 *	build it and return true, otherwise return false.
 *
 */
MIDI_ParseByteA(driver,taggedbyte,event)
	DriverDataPtr driver;
	long taggedbyte;
	MIDI_Event *event;
{
	short byte=(short)((taggedbyte>>24)&255);
	
	if(byte>127){
		driver->inTime=taggedbyte&0x00ffffff;
		if(MIDI_ParseStatusByteA(driver,byte)){
/*			driver->eventData1A=driver->eventData2A=0; shouldn't trash! joe 8/2/88 */
			MIDI_BuildEventA(driver,NORMAL_EVENT,event);
			return(TRUE);
		} else
			return(FALSE);
	} else switch (driver->stateA) {
		case 0:
			if(driver->lastStatusA){
				driver->inTime=taggedbyte&0x00ffffff;
				MIDI_ParseStatusByteA(driver,driver->lastStatusA);
				return(MIDI_ParseByteA(driver,taggedbyte,event));
			} else
				return(FALSE);
		case 1:
			driver->eventData1A=byte;
			driver->eventData2A=0;
			driver->stateA=0;
			MIDI_BuildEventA(driver,NORMAL_EVENT,event);
			return(TRUE);
		case 2:
			if(driver->dataCountA){
				driver->eventData2A=byte;
				driver->stateA=0;
				MIDI_BuildEventA(driver,NORMAL_EVENT,event);
				return(TRUE);
			} else {
				driver->eventData1A=byte;
				driver->dataCountA++;
			}
			break;
		case 3:
		return(FALSE);
		
		/* - joe 1/90  -- Eat sysex bytes if not in raw mode.
			driver->inTime=taggedbyte&0x00ffffff;
			driver->eventStatusA=driver->eventData1A=0;
			driver->eventData2A=byte;
			MIDI_BuildEventA(driver,RAW_EVENT,event);
			return(TRUE);
		*/
		
		case 4: /* grab the pitchbend & filter it */
			if(driver->dataCountA){
				driver->PitchbendA[driver->eventStatusA & 0x0f] = 
					byte << 7 | driver->eventData1A;
				driver->stateA=0;
				return(FALSE);
			} else {
				driver->eventData1A=byte;
				driver->dataCountA++;
			}
			break;
		default:
			break;
	}
	return(FALSE);
}

MIDI_ParseByteB(driver,taggedbyte,event)
	DriverDataPtr driver;
	long taggedbyte;
	MIDI_Event *event;
{
	short byte=(short)((taggedbyte>>24)&255);
	
	if(byte>127){
		driver->inTime=taggedbyte&0x00ffffff;
		if(MIDI_ParseStatusByteB(driver,byte)){
/*			driver->eventData1B=driver->eventData2B=0; shouldn't trash! joe 8/2/88 */
			MIDI_BuildEventB(driver,NORMAL_EVENT,event);
			return(TRUE);
		} else
			return(FALSE);
	} else switch (driver->stateB) {
		case 0:
			if(driver->lastStatusB){
				driver->inTime=taggedbyte&0x00ffffff;
				MIDI_ParseStatusByteB(driver,driver->lastStatusB);
				return(MIDI_ParseByteB(driver,taggedbyte,event));
			} else
				return(FALSE);
		case 1:
			driver->eventData1B=byte;
			driver->eventData2B=0;
			driver->stateB=0;
			MIDI_BuildEventB(driver,NORMAL_EVENT,event);
			return(TRUE);
		case 2:
			if(driver->dataCountB){
				driver->eventData2B=byte;
				driver->stateB=0;
				MIDI_BuildEventB(driver,NORMAL_EVENT,event);
				return(TRUE);
			} else {
				driver->eventData1B=byte;
				driver->dataCountB++;
			}
			break;
		case 3:
			return(FALSE);
		
		/*  - joe 1/90  -- Eat sysex bytes if not in raw mode.
			driver->inTime=taggedbyte&0x00ffffff;
			driver->eventStatusB=driver->eventData1B=0;
			driver->eventData2B=byte;
			MIDI_BuildEventB(driver,RAW_EVENT,event);
			return(TRUE);
		*/
		case 4: /* grab the pitchbend & filter it */
			if(driver->dataCountB){
				driver->PitchbendB[driver->eventStatusB & 0x0f] = 
					byte << 7 | driver->eventData1B;
				driver->stateB=0;
				return(FALSE);
			} else {
				driver->eventData1B=byte;
				driver->dataCountB++;
			}
			break;

		default:
			break;
	}
	return(FALSE);
}

MIDI_EventCount(status)
	short status;
{
	short type=status&0xf0;
	switch (type) {
		case MIDI_NoteOnType:
		case MIDI_NoteOffType:
		case MIDI_ControlType:
		case MIDI_PitchBendType:
		case MIDI_PolyTouchType:
			return(3);
		case MIDI_ProgramType:
		case MIDI_AfterTouchType:
			return(2);
		case MIDI_SysRealTimeType:
			switch (status) {
				case MIDI_SongPosType:
					return(3);
				case MIDI_TimeCodeType:
				case MIDI_SongSelType:
					return(2);
				default:
					return(1);
			}
		default:
			return(0);
	}
}

MIDI_PutEvent(driver,event)
	DriverDataPtr driver;
	MIDI_Event *event;
{
	short chanB=event->type&PORTB;
	short count=MIDI_EventCount(event->status);
	if (chanB) {
		if (driver->OpenB) MIDI_PutEventB(driver,event);
	} else if (driver->OpenA) MIDI_PutEventA(driver,event);
}

MIDI_PutEventA(driver,event)
	DriverDataPtr driver;
	MIDI_Event *event;
{
	short type=event->type&PORTB_COMPLEMENT;
	short count=MIDI_EventCount(event->status);
	if(driver->xmtCountA > (MIDI_FIFO_MAX-3)){
		driver->xmtErrorA++;
		return;
	}
	if (type==NORMAL_EVENT){
		if(count>0)
			MIDI_WriteA(driver,event->status);
		if(count>1)
			MIDI_WriteA(driver,event->data1);
		if(count>2)
			MIDI_WriteA(driver,event->data2);
	} else if (type==RAW_EVENT) MIDI_WriteA(driver,event->data2);
}

MIDI_PutEventB(driver,event)
	DriverDataPtr driver;
	MIDI_Event *event;
{
	short type=event->type&PORTB_COMPLEMENT;
	short count=MIDI_EventCount(event->status);
	if(driver->xmtCountB > (MIDI_FIFO_MAX-3)){
		driver->xmtErrorB++;
		return;
	}
	if (type==NORMAL_EVENT){
		if(count>0)
			MIDI_WriteB(driver,event->status);
		if(count>1)
			MIDI_WriteB(driver,event->data1);
		if(count>2)
			MIDI_WriteB(driver,event->data2);
	} else if (type==RAW_EVENT) MIDI_WriteB(driver,event->data2);
}


/*
 *
 * Driver_Read - Parse the next event and put it into the given event record.
 *
 */

Driver_Read(driver,event)
	DriverDataPtr driver;
	MIDI_Event *event;
{
	long taggedbyte;
	short tmp,res1,res2,result;
	MIDI_Event e;
	if (!driver->lastvalid) {
		res2 = Driver_ReadB(driver, &e);
		res1 = Driver_ReadA(driver, event);
		if (res1 && res2) {
			if (event->time < e.time) {		/* e is newer and can wait */
			    driver->lastevent = e;
			    driver->lastvalid=TRUE;
			    return(TRUE);
			} else {						/* event is newer and can wait */
				driver->lastevent = *event;
				driver->lastvalid=TRUE;
			    *event=e;
				return(TRUE);
			}
		}
		if (res1) return(TRUE);
		if (res2) {
			*event = e;
			return(TRUE);
		} else return(FALSE);
	} else {
	if (driver->lastevent.type&PORTB) result=Driver_ReadA(driver,event);
	    else result=Driver_ReadB(driver,event);
	if (!result) {
	    *event = driver->lastevent;
		driver->lastvalid=FALSE;
	    return(TRUE);
	}
	if (driver->lastevent.time > event->time)  /* this means that last is newer and can wait */
	    return(TRUE);
	else {
	    e=*event;
	    *event = driver->lastevent;
	    driver->lastevent=e;
	    return(TRUE);
	}
  }
}


/*
 *
 *	Filter procedures - used to filter out the midi events that the user doesn't
 *  want to see on receive....
 *
 *	Driver block will contain a long word, each bit of which represents a particular
 *	type of midi event.
 *	
 *	In fact the Driver Block will contain two longs... one for each port and the
 *	bits of each will represent a midi event type to be masked on that particular
 *	port.
 *
 *	We will address them as follows....
 * 	Midi Event		Bit No.
 *	Note Off		0
 *	Note ON			1
 *	PolyTouch		2
 *	ControlType		3
 *	ProgramType		4
 *	Aftertouch		5	(MONO of course)
 *	PitchBend		6
 *	SysEx			7
 *  TimeCode		8
 *	Songpos			9
 *	SongSel			10
 *	TuneType		13
 *	EnsSysEx		14
 *	ClockType		15
 *	StartType		17
 *	ContType		18
 *	StopType		19
 *	ActiveSense		21
 *	Reset			22
 *
 */

long PassEvent(status, filtermask)
	unsigned char status;
	long filtermask;
{
	short type;
	if (status < 0xf0) type = (status>>4) & 0x7;
		else type = (status & 0xf) + 7;
	return (filtermask & (1L<<type));
}

Driver_ReadA(driver,event)
	DriverDataPtr driver;
	MIDI_Event *event;
{
	long taggedbyte;
	MIDI_Event e;
	while((driver->rcvCountA > 0)&&(driver->OpenA)) {
		if((taggedbyte=MIDI_ReadA(driver)) != -1L) {
			if(driver->readModeA == RAW_EVENT) {
				event->time = taggedbyte&0x00ffffff;
				event->type = RAW_EVENT;
				event->status = event->data1 = 0;
				event->data2 = taggedbyte>>24;
				return(TRUE);
			} else if (MIDI_ParseByteA(driver,taggedbyte,&e)) {
				if (PassEvent(e.status, driver->FilterMaskA)) {
					*event = e;
					return(TRUE);
				}
			}
		} else 
			break;
	}
	event->type = NULL_EVENT;
	return(FALSE);
}

Driver_ReadB(driver,event)
	DriverDataPtr driver;
	MIDI_Event *event;
{
	long taggedbyte;
	MIDI_Event e;
	while((driver->rcvCountB > 0)&&(driver->OpenB)) {
		if((taggedbyte=MIDI_ReadB(driver)) != -1L) {
			if(driver->readModeB == RAW_EVENT) {
				event->time = taggedbyte&0x00ffffff;
				event->type = RAW_EVENT|PORTB;
				event->status = event->data1 = 0;
				event->data2 = taggedbyte>>24;
				return(TRUE);
			} else if (MIDI_ParseByteB(driver,taggedbyte,&e)) {				
				if (PassEvent(e.status, driver->FilterMaskB)) {
					*event = e;
					event->type = e.type|PORTB;
					return(TRUE);
				}
			}
		} else 
			break;
	}
	event->type = NULL_EVENT;
	return(FALSE);
}


/*
 *
 * Driver_Write - write the given event, if possible, from the given buffer.
 *
 */

Driver_Write(driver,event)
	DriverDataPtr driver;
	MIDI_Event *event;
{
	Protect;
	if(event->time <= driver->time)
		MIDI_PutEvent(driver,event);
	else
		Schedule_Event(driver,event);
	Unprotect;
}


/*
 * Channelize
 *
 * Channelize sends the same midievent to several channels.
 *
 */
Channelize(driver, event, channels)
	DriverDataPtr driver;
	MIDI_Event *event;
	unsigned short channels;
{
	int	which = 0;
	while (channels > 0) {
		if (channels & 1) {
			event->status = ((event->status & 0xf0) | which);
			Driver_Write(driver, event);
			}
		channels = channels >> 1;
		which++;
		}
}


/*
 *
 * Driver_Open - initialize, allocate space, enable interrupts
 *
 */
Driver_Open (dce)
	DCtlPtr dce;
{
	DriverDataPtr driver,*hdriver;
	Ptr f1=NewPtr(((long)MIDI_FIFO_MAX+1L)*4L),f2=NewPtr((long)MIDI_FIFO_MAX+1L);
	/* Get space for the FIFOs for channel B */
	Ptr f3=NewPtr(((long)MIDI_FIFO_MAX+1L)*4L),f4=NewPtr((long)MIDI_FIFO_MAX+1L);
	Ptr t1=NewPtr((long)TASK_TABLE_SIZE*sizeof(Task)),
		wq=NewPtr((long)WAIT_CACHE_SIZE*sizeof(TaskPtr)),
		wqt=NewPtr((long)WAIT_CACHE_SIZE*sizeof(TaskPtr));
	ResrvMem(sizeof(DriverData));
	hdriver=(DriverDataPtr *)NewHandle(sizeof(DriverData));
	if(!hdriver || !f1 || !f2 || !f3 || !f4 || !t1 || !wq){
		SysBeep(1);
		return(0);
	}
	HLock((Handle) hdriver);
	driver=*hdriver;
	driver->oldStorage = dce->dCtlStorage;
	dce->dCtlStorage = (Handle)hdriver;
	dce->dCtlFlags |= 0x5f00; 			/* all calls enabled, need goodbye, and locked */
	driver->rcvFIFOA = (long*)f1;
	driver->xmtFIFOA = (char*)f2;
	driver->rcvFIFOB = (long*)f3;		/* Set up FIFOs for channel B */
	driver->xmtFIFOB = (char*)f4;
	driver->taskTable = (TaskPtr)t1;
	driver->waitQueue = (TaskPtr *)wq;
	driver->waitQueueTail = (TaskPtr *)wqt;
	driver->readModeA = NORMAL_EVENT;
	driver->readModeB = NORMAL_EVENT;
	driver->lastvalid = 0;
	driver->OpenB=0;
	driver->OpenA=0;
	driver->xmtTCEndA = driver->xmtTCStartA = 0;
	driver->stateA = 0;  /* added joe 8/2/88 */
	driver->stateB = 0;
	InstallHack((long) GetDriverDataPtr,(long) driver);
	Scheduler_Open(driver);
	Clock_Open(driver);
}

/*
 *
 * Driver_Close - disable interrupts, release space, etc.
 *
 */
Driver_Close (dce)
	DCtlPtr dce;
{
	DriverDataPtr driver=GetDriverDataPtr();
	Handle hdriver;
	if (driver->OpenA) MIDI_CloseA(driver);
	if (driver->OpenB) MIDI_CloseB(driver);
	Scheduler_Close(driver);
	Clock_Close(driver);
	hdriver=dce->dCtlStorage;
	dce->dCtlStorage=driver->oldStorage;
	DisposPtr((Ptr) driver->taskTable);
	DisposPtr((Ptr) driver->waitQueue);
	DisposPtr((Ptr) driver->waitQueueTail);
	DisposPtr((Ptr) driver->rcvFIFOA);
	DisposPtr((Ptr) driver->xmtFIFOA);
	DisposPtr((Ptr) driver->rcvFIFOB);		/* Destroy pointers for B channel FIFOs */
	DisposPtr((Ptr) driver->xmtFIFOB);
	DisposHandle((Handle) hdriver);
}

/*
 *
 * Driver-Control - perform the given control function.
 *
 */
Driver_Control(code,params)
	short code;
	CntrlParam *params;
{
	long newtime;
	DriverDataPtr driver=GetDriverDataPtr();
	switch(code){
		case CHANNELIZE_CODE:
			Channelize(driver, (MIDI_Event*)params, 
					   	*((unsigned short*)((char*)params + 8)));
			break;
		case CLOCKTIME_CODE:
			Protect;
			newtime = *((long*)params);
			while (newtime > driver->time++)
				Scheduler_ExecuteReadyTasks(driver);
			driver->time = newtime;
			Unprotect;
			break;
		case TIMECODE_TRANSMIT_CODEA:
			XmtTCSet(driver,
						((unsigned char*)params)[0],
						((unsigned char*)params)[1],
						((unsigned char*)params)[2],
						((unsigned char*)params)[3]);
			break;
		case TIMECODE_TRANSMIT_ENABLE_CODEA:
			Protect;
			driver->xmtTCStartA = ((long*)params)[0];
			driver->xmtTCEndA = ((long*)params)[1];
			Unprotect;
			break;
		case TIMECODE_RECEIVE_ENABLE_CODEA:
			driver->rcvTCGotF1A = 0;
			driver->rcvTCEnableA = *((short*)params);
			break;

		case FILTER_CODEA:
			driver->FilterMaskA = *((long*)params);
			break;
		case FILTER_CODEB:
			driver->FilterMaskB = *((long*)params);
			break;
		case PITCHBENDA:
			driver->SamplePitchbendA = *((short*)params);
			break;
		case PITCHBENDB:
			driver->SamplePitchbendB = *((short*)params);
			break;

		case RECEIVEON_CODEA:
			if (driver->OpenA) driver->rcvEnableA = 1;
			break;
		case RECEIVEOFF_CODEA:
			if (driver->OpenA) driver->rcvEnableA = 0;
			break;
		case READMODE_CODEA:
			driver->readModeA = *((short*)params);
			break;
		case CLEAR_RECEIVE_CODEA:
			if (driver->OpenA) MIDI_ClearRcvA(driver);
			break;
		case CLEAR_TRANSMIT_CODEA:
			if (driver->OpenA) MIDI_ClearXmtA(driver);
			break;
		case RECEIVEON_CODEB:
			if (driver->OpenB) driver->rcvEnableB = 1;
			break;
		case RECEIVEOFF_CODEB:
			if (driver->OpenB) driver->rcvEnableB = 0;
			break;
		case READMODE_CODEB:
			driver->readModeB = *((short*)params);
			break;
		case CLEAR_RECEIVE_CODEB:
			if (driver->OpenB) MIDI_ClearRcvB(driver);
			break;
		case CLEAR_TRANSMIT_CODEB:
			if (driver->OpenB) MIDI_ClearXmtB(driver);
			break;
		case CLEAR_SCHEDULER_CODE:
			Scheduler_Clear(driver);
			break;
		case TICKSIZE_CODE:
			driver->tickSize = *((long*)params);
			break;
			
		case OPEN_CODEA:
			if (!driver->OpenA) MIDI_OpenA(driver);
			break;
		case OPEN_CODEB:
			if (!driver->OpenB) MIDI_OpenB(driver);
			break;
		case CLOSE_CODEA:
			if (driver->OpenA) MIDI_CloseA(driver);
			break;			
		case CLOSE_CODEB:
			if (driver->OpenB) MIDI_CloseB(driver);
			break;
			
		case SIMREAD_CODEA:
			if ((driver->rcvCountA <= MIDI_FIFO_MAX) && driver->rcvEnableA) {
				driver->rcvCountA++;
				driver->rcvFIFOA[driver->rcvPutIndexA++] = (*((long*)params)<<24L);
				driver->rcvPutIndexA = driver->rcvPutIndexA & MIDI_FIFO_MAX;
			} else
				driver->rcvErrorA++;
			break;
		case SIMREAD_CODEB:
			if ((driver->rcvCountB <= MIDI_FIFO_MAX) && driver->rcvEnableB) {
				driver->rcvCountB++;
				driver->rcvFIFOB[driver->rcvPutIndexB++] = (*((long*)params)<<24L);
				driver->rcvPutIndexB = driver->rcvPutIndexB & MIDI_FIFO_MAX;
			} else
				driver->rcvErrorB++;
			break;
		default:
			break;
	}
}

/*
 *
 * Driver-Status - return the requested status.
 *
 */
Driver_Status(code,params)
	short code;
	CntrlParam *params;
{
	DriverDataPtr driver=GetDriverDataPtr();
	switch(code){
	
		case READ_PORT_CODEA:
			if (driver->OpenA) Driver_ReadA(driver,(MIDI_Event*) params);	
			break;
			
		case READ_PORT_CODEB:
			if (driver->OpenB) Driver_ReadB(driver,(MIDI_Event*) params);
			break;			

		case CLOCKTIME_CODE:
			*((long*)params) = driver->time;
			break;
			
		case TIMECODE_RECEIVE_CODEA:
			Protect;
			((unsigned char*)params)[0] = driver->rcvTCHourA;
			((unsigned char*)params)[1] = driver->rcvTCMinuteA;
			((unsigned char*)params)[2] = driver->rcvTCSecondA;
			((unsigned char*)params)[3] = driver->rcvTCFrameA;
			*((long*)((char*)params + 4)) = driver->rcvTCTimeA;
			Unprotect;
			break;

		case PITCHBENDB:
			*((short*)params) = driver->PitchbendB[*((short*)params)];
			break;
		case PITCHBENDA:
			*((short*)params) = driver->PitchbendA[*((short*)params)];
			break;

		case TIMECODE_TRANSMIT_CODEA:
			Protect;
			((unsigned char*)params)[0] = driver->xmtTCHourA;
			((unsigned char*)params)[1] = driver->xmtTCMinuteA;
			((unsigned char*)params)[2] = driver->xmtTCSecondA;
			((unsigned char*)params)[3] = driver->xmtTCFrameA;
			Unprotect;
			break;

		case FILTER_CODEA:
			*((long*)params) = driver->FilterMaskA;
			break;
		case FILTER_CODEB:
			*((long*)params) = driver->FilterMaskB;
			break;
		
		case OPEN_CODEA:
			*((short*)params) = driver->OpenA;
			break;
		case READMODE_CODEA:
			*((short*)params) = driver->readModeA;
			break;
		case RECEIVE_STATUS_CODEA:
			*((short*)params) = driver->rcvCountA;
			if(driver->rcvErrorA)
				((short*)params)[1] = 1;
			else
				((short*)params)[1] = 0;
			break;
		case TRANSMIT_STATUS_CODEA:
			*((short*)params) = driver->xmtCountA;
			if(driver->xmtErrorA)
				((short*)params)[1] = 1;
			else
				((short*)params)[1] = 0;
			break;
			
		case OPEN_CODEB:
			*((short*)params) = driver->OpenB;
			break;
		case READMODE_CODEB:
			*((short*)params) = driver->readModeB;
			break;
		case RECEIVE_STATUS_CODEB:
			*((short*)params) = driver->rcvCountB;
			if(driver->rcvErrorB)
				((short*)params)[1] = 1;
			else
				((short*)params)[1] = 0;
			break;
		case TRANSMIT_STATUS_CODEB:
			*((short*)params) = driver->xmtCountB;
			if(driver->xmtErrorB)
				((short*)params)[1] = 1;
			else
				((short*)params)[1] = 0;
			break;

		case TICKSIZE_CODE:
			 *((long*)params) = driver->tickSize;
			break;

		case DRIVER_ADDRESS:
			*((long*)params)= (long)driver;
			break;
/*
		case 40:
			 *((long*)params)= (long)driver->rcvFIFOB+driver->rcvGetIndexB;
			break;
*/
		default:
			break;
	}
}


/*
 *
 * Driver_Prime - do a read or a write
 *
 */
Driver_Prime(d,p)		
	DCtlPtr d;
	IOParam *p;
{
	DriverDataPtr driver=GetDriverDataPtr();
	if ((p->ioTrap & 255) == 2) {
		Driver_Read(driver,(MIDI_Event*) p->ioBuffer);	/* Read */
		p->ioActCount=sizeof(MIDI_Event);
	} else {
		Driver_Write(driver,(MIDI_Event*) p->ioBuffer);	/* Write */
		p->ioActCount=sizeof(MIDI_Event);
	}
}


/*
 *
 * main - provide the Mac driver interface.
 *
 */
main(p, d, n)
	IOParam *p;
	DCtlPtr d;
	short n;
{
	CntrlParam *c;
	long i;
	short j;

	switch (n) {
		case 1:							/* Prime */
			Driver_Prime(d,p);
			break;
		case 3:							/* Status */
			c=(CntrlParam*)p;
			Driver_Status(c->csCode,(CntrlParam*) c->csParam);
			break;
		case 2:							/* Control */
			c=(CntrlParam*)p;
			if (c->csCode == goodBye)
				Driver_Close(d);
			else
				Driver_Control(c->csCode,(CntrlParam*) c->csParam);
			break;
		case 0:							/* Open */
			Driver_Open(d);
			break;
		case 4:							/* Close */
			Driver_Close(d);
			break;
	}
	return(0);
}



