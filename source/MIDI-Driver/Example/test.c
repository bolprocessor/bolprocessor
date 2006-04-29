// Test for MIDI driver.  Checked under Think C 5.0.2
// This folder should contain test.proj.rsrc created by MIDIdriver.proj.

#include <MacHeaders>
#include "midi1.h"
#include <console.h>

typedef struct {
	long clockTime;
	short extra[8];
} MIDI_Parameters, *MIDI_ParametersPtr;


OSErr DriverOpen(pb,name)
	IOParam *pb;
	StringPtr name;
{
	pb->ioCompletion = 0L;
	pb->ioNamePtr = name;
	pb->ioPermssn = 3;
	PBOpen((ParmBlkPtr)pb,0);
	return(pb->ioResult);
}

OSErr DriverClose(pb)
	IOParam *pb;
{
	return(PBClose((ParmBlkPtr)pb,0));
}

OSErr DriverRead(pb,ptr,count)
	IOParam *pb;
	Ptr ptr;
	long count;
{
	pb->ioBuffer = (Ptr) ptr;
	pb->ioReqCount = count;
	return(PBRead((ParmBlkPtr)pb,0));
}

OSErr DriverWrite(pb,ptr,count)
	IOParam *pb;
	MIDI_Event *ptr;
	long count;
{
	pb->ioBuffer = (Ptr) ptr;
	pb->ioReqCount = count;
	return(PBWrite((ParmBlkPtr)pb,0));
}

OSErr DriverStatus(pb,csCode,csParam)
	IOParam *pb;
	short csCode;
	int csParam[];
{
	short i;
	OSErr err;
	CntrlParam c;
	c.ioCompletion = 0L;
	c.ioVRefNum = pb->ioVRefNum;
	c.ioCRefNum = pb->ioRefNum;
	c.csCode = csCode;
	err=PBStatus((ParmBlkPtr)&c,0);	/* see Devices.h */
	for(i=0;i < 11;i++)
		csParam[i] = c.csParam[i];
	return(err);
}


OSErr DriverControl(pb,csCode,csParam)
	IOParam *pb;
	short csCode;
	int csParam[];
{
	short i;
	CntrlParam c;
	c.ioCompletion = 0L;
	c.ioVRefNum = pb->ioVRefNum;
	c.ioCRefNum = pb->ioRefNum;
	c.csCode = csCode;
	for(i=0;i < 11;i++)
		c.csParam[i] = csParam[i];
	return(PBControl((ParmBlkPtr)&c,0));
}


OSErr DriverKillIO(pb)
	IOParam *pb;
{
	CntrlParam c;
	c.ioCompletion = 0L;
	c.ioVRefNum = pb->ioVRefNum;
	c.ioCRefNum = pb->ioRefNum;
	c.csCode = 1;
	return(PBControl((ParmBlkPtr)&c,0));
}


Events(driver)
DriverDataPtr driver;
{
	short i=0, count=0;
	TaskPtr task, remtasks;
	while (i < WAIT_CACHE_SIZE) {
	if (driver->waitQueue[i]) {
		printf("\n %d:",i);
		remtasks = driver->waitQueue[i];
		while(task = remtasks) {
			printf("exec = %lx ",task->exectime);
			count++;
			remtasks = task->link;
			}
		}
	i++;
	}
	printf("\ncount = %d",count);
}

Play()
{
	short err;
	long temp, count=12L,time;
	IOParam pb;
	MIDI_Parameters parms;
	MIDI_Event e;
	

	err=DriverStatus(&pb,CLOCKTIME_CODE,(int*) &parms);/* read the clock */
	time=parms.clockTime;
	temp=time;

	for (; temp < time+300L; temp = temp+2L) {
		e.time = temp;
		e.type=NORMAL_EVENT;
		e.status=144;
		e.data1=60;
		e.data2=100;
		DriverWrite(&pb,&e,count);						/* NOTE ON */
		e.time = temp+20;
		e.data2=0;
		DriverWrite(&pb,&e,count);						/* NOTE OFF */
	}
}

DriverTime(driver)
DriverDataPtr driver;
{
printf("Time : %lx\n",driver->time);
}

Errors(driver)
DriverDataPtr driver;
{
printf("XmtCountA=%d XmtCountB=%d ",driver->xmtCountA, driver->xmtCountB);
printf("XmtErrorA=%d XmtErrorB=%d\n",driver->xmtErrorA, driver->xmtErrorB);
printf("RcvCountA=%d RcvCountB=%d ",driver->rcvCountA, driver->rcvCountB);
printf("RcvErrorA=%d RcvErrorB=%d ",driver->rcvErrorA, driver->rcvErrorB);
if (driver->lastvalid) printf("lastvalid\n");
}

main()
{
	register int i;
	OSErr err;
	long now,count=12L,cnt=0L,time;
	IOParam pb;
	MIDI_Parameters parms;
	char c;
	Ptr pointer;
	long time_res;		/* Must be 'long' even though it's converted to int... */
	MIDI_Event E,*e;
	DriverDataPtr driver;
	TaskPtr remtasks, task;
	
	e = &E;
	printf("Checking MIDI driver (MODEM output)\nTen NoteOn/NoteOff's will be sent on channel #1\n");
	err=DriverOpen(&pb,"\p.MIDI");
	if(err) {
		printf("Cannot open midi driver. Error : %d\n",err);
		ExitToShell();
	}
	
	time_res = 10L; /* Time resolution for MIDI codes, 10ms */
	
	parms.clockTime = 0;
	DriverControl(&pb,OPEN_CODEA,(int*) &parms);
	
/*	err  = DriverStatus(&pb,CLOCKTIME_CODE,(int*) &parms);
	if(err) {
		printf("Cannot read status of midi driver. Error : %d\n",
			err);
		ExitToShell();
		} */
		
	DriverControl(&pb,TICKSIZE_CODE,(int*) &time_res);
	parms.clockTime = 10L; /* Filter param: receive only NoteOn and ControlChange */
	DriverControl(&pb,FILTER_CODEA,(int*) &parms);
	parms.clockTime = 0L;
	DriverControl(&pb,CLOCKTIME_CODE,(int*) &parms);
/*	DriverStatus(&pb,CLOCKTIME_CODE,(int*) &parms); */
	time = 0L;
	for (i=0; i < 10; i++) {
		e->time = time;
		e->type   = NORMAL_EVENT;
		e->status = MIDI_NoteOnType /* 144 */;
		e->data1  = 60 + i;
		e->data2  = 127;	/* Velocity */
		DriverWrite(&pb,e,count);
		time += 100L;
		e->time  = time;
		e->type   = NORMAL_EVENT;
		e->status = MIDI_NoteOnType /* 144 */;
		e->data1  = 60 + i;
		e->data2  = 0;	/* This is a NoteOff */
		DriverWrite(&pb,e,count);
		time += 20L;
		cnt++;
	}
		
	printf("\nOK...  %ld notes have been sent.  <<click>>\n",cnt);
	while(!Button());
	DriverStatus(&pb,6,(int*) &parms);
	printf(" XmtCountA = %x, XmtErrorA = %x\n", parms.clockTime >> 16,
		parms.clockTime & 0xffff);
	DriverStatus(&pb,16,(int*) &parms);
	printf(" XmtCountB = %x, XmtErrorB = %x\n", parms.clockTime >> 16,
		parms.clockTime & 0xffff);
	DriverStatus(&pb,30,(int*) &parms);
	driver = (DriverDataPtr)parms.clockTime;
	Errors(driver);
	
	goto END;
	
	i = 8;
	while ((c=getchar())!='x') {
		err=DriverRead(&pb,(Ptr) e,count);
		while (e->type != NULL_EVENT) {
			printf("Event : [%lx %x %x %x %x]\n",e->time,e->type,
				e->status,e->data1,e->data2);
			err=DriverRead(&pb,(Ptr) e,count);
		} 

		DriverStatus(&pb,30,(int*) &parms);
		driver = (DriverDataPtr)parms.clockTime;
		if (c=='e') Errors(driver);
		if (c=='l') Events(driver);
		if (c=='t') DriverTime(driver);
		if (c=='m') Play();
		if (c=='p') {
			if (i==8) i=18; else i=8;
			printf(" i=%d \n",i);
		}
		if (c=='n') {
			parms.clockTime = 0;
			DriverControl(&pb,i,(int*) &parms);
		}
		if ((c<='9')&&(c>='0')) {
			parms.clockTime = 1<<((int)c-(int)'0');
			printf(" i=%d c=%lx\n",i, parms.clockTime);
			DriverControl(&pb,i,(int*) &parms);
			err=DriverStatus(&pb,i,(int*) &parms);
			printf(" i=%d c=%lx\n",i, parms.clockTime);
		}
		if (c=='b') {
			parms.clockTime = 0;
			DriverControl(&pb,OPEN_CODEB,(int*) &parms);
		}
		if (c=='a') {
			parms.clockTime = 0;
			DriverControl(&pb,OPEN_CODEA,(int*) &parms);
		}
		if (c=='d') {
			parms.clockTime = 0;
			DriverControl(&pb,CLOSE_CODEB,(int*) &parms);
		}
		if (c=='c') {
			parms.clockTime = 0;
			DriverControl(&pb,CLOSE_CODEA,(int*) &parms);
		}
	}
	
END:
	DriverClose(&pb);
}

