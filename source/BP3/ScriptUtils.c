/* ScriptUtils.c (BP3) */

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


#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"

int trace_script = 0;

int DoScript(int i_script,char*** p_keyon,int wind,int check,int instr,long* p_posdir,
	int* p_changed,char* p_newarg,int quick) {
	/* 'quick' is not used. */
	int i,j,k,w,ww,pos,r,rs,firstchar,diffchar,channel,thekey,displayitems,
		oldoutmidi,oldwritemidifile,oldoutcsound,oms,changed,tried,oldparamcontrol,oldparamkey,oldparamchan;
	unsigned int seed;
	unsigned long timethisscript;
	// FSSpec spec;
	long x,jj;
	OSErr io;
	short refnum;
	Str255 fn;
	char thechar,c,*p,*q,line[MAXLIN],**p_line,type1[5],type2[5],type3[5];
	MIDI_Event e;
	long count = 12L;
	// AEEventClass theclass;
	// AEEventID theID;
	OSType thesignature;
	// AEAddressDesc thetarget;
	// AppleEvent theAppleEvent,reply;
	// AESendPriority priority;
	// ProcessInfoRec info;
	TextOffset selbegin, selend;

	r = OK; rs = 0;
	oldoutmidi = rtMIDI;
	if(trace_script) BPPrintMessage(0,odInfo,"ScriptNrArg(%d) = %d\n",i_script,ScriptNrArg(i_script));
	if(ScriptNrArg(i_script) > 0) MystrcpyTableToString(MAXLIN,line,ScriptLine.arg,0);
	else line[0] = '\0';
	if(trace_script) BPPrintMessage(0,odInfo,"DoScript instruction = %d\n",instr);
	switch(instr) {
	/*	case 0:	// Expand selection
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if((w=BPGetWindowIndex(line,0)) != ABORT) {
				if(check) return(OK);
				BPActivateWindow(SLOW,w); ScriptW = w;
				return(mExpandSelection(w));
				}
			else r = ABORT;
			break; */
		case 1:
			if(check) return(OK);
			NoteConvention = FRENCH;
			break;
		case 2:
			if(check) return(OK);
			NoteConvention = ENGLISH;
			break;
		case 3:
			if(check) return(OK);
			NoteConvention = KEYS;
			break;
		/* case 5:	// Load project
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(SoundOn) {
				Print(wTrace,"\nYou can't open new project while playing.\n");
				return(ABORT);
				}
			if(check == 2) {
				Print(wTrace,"\nYou can't open new project in 'INIT:'.\n");
				return(ABORT);
				}
			if(!check) {
				if(ResetProject(FALSE) != OK) return(ABORT);
				if(Beta && ScriptExecOn  && TraceRefnum != -1)
					WriteToFile(NO,MAC,"----------------",TraceRefnum);
				my_sprintf(Message,"Loading project: '%s'",line);
				ShowMessage(TRUE,wMessage,Message);
				if(TraceMemory) {
					my_sprintf(LineBuff,"%s %ld [%ld]\n", Message,
						(long) MemoryUsed,(long)MemoryUsed - MemoryUsedInit);
					ShowMessage(TRUE, wMessage, LineBuff);
					}
				}
			c2pstrcpy(fn, line);
			c2pstrcpy(spec.name, line);
			spec.vRefNum = TheVRefNum[wGrammar];
			spec.parID = WindowParID[wGrammar];
			if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
	GOTIT:
				ScriptW = wGrammar;
				if(check) {
					FSClose(refnum);
					*p_posdir = -1L;
					return(OK);
					}
				if(LoadGrammar(&spec,refnum) != OK) return(ABORT);
				p2cstrcpy(FileName[wGrammar],spec.name);
				SetName(wGrammar,TRUE,TRUE);
				if(LoadAlphabet(wGrammar,&spec) != OK) return(ABORT);
				BPActivateWindow(SLOW,wScript);
				}
			else {
				spec.vRefNum = CurrentVref;
				spec.parID = CurrentDir;
				if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) goto GOTIT;
				if(!check) {
					my_sprintf(Message,"Can't find '%s'. You should check script.\n",line);
					Print(wTrace,Message);
					}
				else {
					if(CheckFileName(wGrammar,line,&spec,&refnum,gFileType[wGrammar],FALSE)
						!= OK) return(ABORT);
					MystrcpyStringToTable(ScriptLine.arg,0,line);
					*p_newarg = TRUE;
					CurrentDir = WindowParID[wGrammar] = spec.parID;
					ChangeDirInfo(CurrentDir,spec.vRefNum,p_posdir);
					CurrentVref  = TheVRefNum[wGrammar] = spec.vRefNum;
					FSClose(refnum);
					*p_posdir = -1L;
					*p_changed = TRUE;
					return(OK);
					}
				return(ABORT);
				}
			break;
		case 6:	// Select all window:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if((w=BPGetWindowIndex(line,0)) != ABORT) {
				ScriptW = w;
				if(check) return(OK);
				BPActivateWindow(SLOW,w);
				SelectBehind(ZERO,GetTextLength(w),TEH[w]);
				}
			else r = MISSED;
			break;
		case 7:	// Print window:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if((w=BPGetWindowIndex(line,0)) != ABORT) {
				ScriptW = w;
				if(check) return(OK);
				BPActivateWindow(SLOW,w);
				r = mPrint(w);
				}
			else r = MISSED;
			break;
		case 12:	// Analyze selection window:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if((w=BPGetWindowIndex(line,0)) != ABORT) {
				ScriptW = w;
				if(check) return(OK);
				BPActivateWindow(SLOW,w);
				if(mAnalyze(w) == ABORT) r = ABORT;
				}
			else r = ABORT;
			break;
		case 13:	// Play selection window:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if((w=BPGetWindowIndex(line,0)) != ABORT) {
				ScriptW = w;
				if(check) return(OK);
				if((r=WaitForEmptyBuffer()) != OK) return(r);
				BPActivateWindow(SLOW,w);
				r = PlaySelection(w,0);
				if(r == MISSED) r = OK;
				}
			else r = ABORT;
			break; */
		case 14:	// Wait for:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(MIDIfileOn) return(OK);
			if(!check && (r=WaitForEmptyBuffer()) != OK) return(r);
			thechar = '\0';
		/*	if(strcmp(line,"space") == 0) {
				if(check) return(OK);
				thechar = ' ';
				if(WaitKeyStrokeOrAppleEvent(thechar,FALSE,KEYBOARDEVENT,0,0,NULL,NULL) != OK) return(MISSED);
				return(OK);
				}
			if(strcmp(line,"Start") == 0) {
				if(check) return(OK);
				if(WaitKeyStrokeOrAppleEvent(thechar,FALSE,STARTEVENT,0,0,NULL,NULL) != OK) return(MISSED);
				return(OK);
				}
			if(strcmp(line,"Continue") == 0) {
				if(check) return(OK);
				if(WaitKeyStrokeOrAppleEvent(thechar,FALSE,CONTINUEEVENT,0,0,NULL,NULL) != OK) return(MISSED);
				return(OK);
				}
			if(strcmp(line,"Stop") == 0) {
				if(check) return(OK);
				if(WaitKeyStrokeOrAppleEvent(thechar,FALSE,STOPEVENT,0,0,NULL,NULL) != OK) return(MISSED);
				return(OK);
				} */
			strcpy(Message,"cmd-");
			p = &line[0]; q = &(Message[0]);
			pos = 0;
		/*	if(Match(FALSE,&p,&q,strlen(Message))) {
				if(check) return(OK);
				pos += strlen(Message);
				thechar = line[pos];
				if(WaitKeyStrokeOrAppleEvent(thechar,TRUE,KEYBOARDEVENT,0,0,NULL,NULL) != OK) return(MISSED);
				return(OK);
				} */
		/*	if(GetNote(line,&thekey,&channel,FALSE) == OK) {
				if(check) return(OK);
				r = WaitForNoteOn(channel,thekey);
				if(r != ABORT) r = OK;
				return(r);
				} */
			if(check) return(MISSED);
			thechar = line[0];
		//	r = WaitKeyStrokeOrAppleEvent(thechar,FALSE,KEYBOARDEVENT,0,0,NULL,NULL);
			break;
		case 15:	// Velocity parameter _Kx_ from _note_ channel _1..16_
	//		if(wind == wGlossary) return(MISSED);
			if(!check) return(OK);
			i = (*(ScriptLine.intarg))[0];
			oldparamchan = ParamChan[i];
			oldparamkey = ParamKey[i];
	/*		if(ParamChan[i] != -1 && ParamControl[i] != -1) {
				my_sprintf(Message,"Parameter K%ld is already controlled by controller #%ld channel %ld\n",
					(long)i,(long)ParamControl[i],(long)ParamChan[i]);
				Print(wTrace,Message);
				return(MISSED);
				}
			if(ParamChan[i] != -1 && ParamKey[i] != -1) {
				my_sprintf(Message,"Parameter K%ld is already controlled by key #%ld channel %ld\n",
					(long)i,(long)ParamKey[i],(long)ParamChan[i]);
				Print(wTrace,Message);
				return(MISSED);
				} */
	//		if(CheckUsedKey(p_keyon,1,2) != OK) return(MISSED);
			ParamKey[i] = (*(ScriptLine.intarg))[1];
			ParamControlChan = ParamChan[i] = (*(ScriptLine.intarg))[2];
			if(TraceMIDIinteraction && (oldparamchan != ParamChan[i] || oldparamkey != ParamKey[i]))
				BPPrintMessage(0,odInfo,"Script: K%d will be set by key %d chan %d\n",i,ParamKey[i],ParamChan[i]);
			break;
		case 16:	/* Type */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!Editable[ScriptW]) {
				Print(wTrace,"Can't type to non-text window.\n");
				return(ABORT);
				}
			if(check) return(OK);
			if(strcmp(line,"<return>") == 0) {
				PrintBehind(ScriptW,"\n");
				ShowSelect(CENTRE,ScriptW);
				return(OK);
				}
			if((p_line=(char**) GiveSpace((Size)MyGetHandleSize((Handle)
				(*ScriptLine.arg)[0]))) == NULL) return(ABORT);
			if(MystrcpyHandleToHandle(0,&p_line,(*ScriptLine.arg)[0]) != OK) return(ABORT);
			for(k=0;; k++) {
				thechar = (*p_line)[k];
				if(thechar == '\0') break;
				// DoKey(thechar,0,TEH[ScriptW]);
				}
			ShowSelect(CENTRE,ScriptW);
			MyDisposeHandle((Handle*)&p_line);
			break;
		case 17:	/* Hold */
		case 190:	/* MIDI send Start */
		case 191:	/* MIDI send Continue */
		case 192:	/* MIDI send Stop */
	//		if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(instr == 17) {
				jj = (*(ScriptLine.intarg))[0];
				if(TraceMIDIinteraction) BPPrintMessage(0,odInfo,"Holding for %ld milliseconds\n",(long)jj);
				}
			if(++Joutscript >= Maxoutscript) {
				if((p_OUTscript = (OUTscripttype**) IncreaseSpace((Handle)p_OUTscript)) == NULL) return(ABORT);
				Maxoutscript = MyGetHandleSize((Handle)p_OUTscript) / sizeof(OUTscripttype);
				for(i = Joutscript; i < Maxoutscript; i++) (*p_OUTscript)[i].chan = -1;
				}
			((*p_OUTscript)[Joutscript]).scriptline = instr;
			((*p_OUTscript)[Joutscript]).chan = 0;
			if(instr == 17) ((*p_OUTscript)[Joutscript]).duration = 1000L * jj;
			else ((*p_OUTscript)[Joutscript]).duration = 0L;
			if(instr == 190) ((*p_OUTscript)[Joutscript]).key = Start;
			if(instr == 191) ((*p_OUTscript)[Joutscript]).key = Continue;
			if(instr == 192) ((*p_OUTscript)[Joutscript]).key = Stop;
			((*p_OUTscript)[Joutscript]).time = 1000 * Tcurr * Time_res; // microseconds
			if(TraceMIDIinteraction) BPPrintMessage(0,odInfo,"[%d] OUTscript instruction %d, duration = %lu ms, time = %lu ms\n",Joutscript,instr,((*p_OUTscript)[Joutscript]).duration / 1000L,((*p_OUTscript)[Joutscript]).time / 1000L);
			break;
		case 18:	/* Max time */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			jj = (*(ScriptLine.intarg))[0];
			if(!MoreTime) InitWait(jj);
			else WaitEndDate += jj;
			MoreTime = FALSE;
			break;
		case 19:	/* Delete n chars */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			jj = (*(ScriptLine.intarg))[0];
			while(jj > 0) {
			//	DoKey('\b',0,TEH[ScriptW]);
				jj--;
				}
			ShowSelect(CENTRE,ScriptW);
			break;
		case 20:	/* MIDI sound ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
	#if WITH_REAL_TIME_MIDI_FORGET_THIS
			rtMIDI = TRUE;
			
			if(rtMIDI && !oldoutmidi) ResetMIDI(FALSE);
	#else
			return(MISSED);
	#endif
			break;
		case 21:	/* MIDI sound OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			rtMIDI = FALSE;
			break;
		case 22:	/* Csound score ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			OutCsound = TRUE;
			
			break;
		case 23:	/* Csound score OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			oldoutcsound = OutCsound;
			OutCsound = FALSE;
			if(oldoutcsound) CloseCsScore();
			
			break;
		/* case 24:
		// Open file (only data, grammar, keyboard, interaction, glossary, prototypes or alphabet)
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check == 2) return(OK);	// Compiling 'INIT:' in grammar
			if(!check) {
				if((r=WaitForEmptyBuffer()) != OK) return(r);/
				if(Beta && ScriptExecOn  && TraceRefnum != -1)
					NoReturnWriteToFile("\n",TraceRefnum);
				my_sprintf(Message,"Loading file: '%s'",line);
				ShowMessage(TRUE,wMessage,Message);
				if(TraceMemory) {
					my_sprintf(LineBuff,"%s %ld [%ld]\n", Message,
						(long) MemoryUsed,(long)MemoryUsed - MemoryUsedInit);
					ShowMessage(TRUE,wMessage,LineBuff);
					}
				}
			for(w=0; w < WMAX; w++) {  // FIXME: need to match if no prefix
				if(FilePrefix[w][0] == '\0') continue;
				p = &(line[0]); q = &(FilePrefix[w][0]);
				if(Match(TRUE,&p,&q,4)) break;
				}
			if(w == WMAX) w = wTrace;
				
			if(!check) {
				strcpy(FileName[w],line);
				ClearWindow(FALSE,w);
				}
			switch(w) {
				case wGrammar:
				case wData:
				case wAlphabet:
					if(SoundOn) {
						my_sprintf(Message,"\nYou can't load '%s' while playing.\n",line);
						Print(wTrace,Message);
						return(MISSED);
						}
					break;
				case iObjects:
					if(check == 2) return(OK);
					c2pstrcpy(fn, line);
					c2pstrcpy(spec.name, line);
					spec.vRefNum = TheVRefNum[iObjects];
					spec.parID = WindowParID[iObjects];
					if(check) {
						if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
	GOTIT2:
							TheVRefNum[wCsoundResources] = TheVRefNum[iObjects] = spec.vRefNum;
							WindowParID[wCsoundResources] = WindowParID[iObjects] = spec.parID;
							FSClose(refnum);
							*p_posdir = -1L;
							return(OK);
							}
						else {
							spec.vRefNum = CurrentVref;
							spec.parID = CurrentDir;
							if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) goto GOTIT2;
							if(CheckFileName(iObjects,line,&spec,&refnum,gFileType[iObjects],FALSE) != OK)
								return(ABORT);
							MystrcpyStringToTable(ScriptLine.arg,0,line); *p_newarg = TRUE;
							CurrentDir = WindowParID[wCsoundResources]
								= WindowParID[iObjects] = spec.parID;
							ChangeDirInfo(CurrentDir,spec.vRefNum,p_posdir);
							CurrentVref = TheVRefNum[wCsoundResources]
								= TheVRefNum[iObjects] = spec.vRefNum;
							FSClose(refnum);
							*p_changed = TRUE;
							*p_posdir = -1L;
							return(OK);
							}
						}
					if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
						spec.vRefNum = TheVRefNum[wCsoundResources]
							= TheVRefNum[iObjects] = CurrentVref;
						spec.parID = WindowParID[wCsoundResources]
							= WindowParID[iObjects] = CurrentDir;
						if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
							my_sprintf(Message,"\nCan't find '%s'. You should check the script.\n",
								line);
							Print(wTrace,Message);
							return(ABORT);
							}
						}
					FSClose(refnum);
					strcpy(FileName[iObjects],line);
					if(!CompiledGr && (AddBolsInGrammar() > BolsInGrammar)) {
						CompiledAl = FALSE;
						}
					if(CompileCheck() != OK) return(MISSED);
					if(!NeedAlphabet) {
						NeedAlphabet = TRUE;
						ObjectMode = ObjectTry = FALSE;
						}
					if(LoadObjectPrototypes(YES,NO) != OK) {
						ObjectMode = ObjectTry = FALSE;
						FileName[iObjects][0] = '\0';
						SetName(iObjects,TRUE,TRUE);
						Dirty[iObjects] = Created[iObjects] = FALSE;
						iProto = 0;
						return(ABORT);
						}
					else iProto = 2;
					SetPrototype(iProto);
					SetCsoundScore(iProto);
					return(OK);
					break;
				case wInteraction:
					if(check == 2) return(OK);
					if(!Interactive) {
						Interactive = TRUE; 
						}
					c2pstrcpy(fn, line);
					c2pstrcpy(spec.name, line);
					spec.vRefNum = TheVRefNum[wInteraction];
					spec.parID = WindowParID[wInteraction];
					if(check) {
						if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
	GOTIT3:						
							TheVRefNum[wInteraction] = spec.vRefNum;
							WindowParID[wInteraction] = spec.parID;
							FSClose(refnum);
							*p_posdir = -1L;
							return(OK);
							}
						else {
							spec.vRefNum = CurrentVref;
							spec.parID = CurrentDir;
							if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) goto GOTIT3;
							if(CheckFileName(wInteraction,line,&spec,&refnum,gFileType[wInteraction],FALSE)
								!= OK) return(ABORT);
							MystrcpyStringToTable(ScriptLine.arg,0,line); *p_newarg = TRUE;
							CurrentDir = WindowParID[wInteraction] = spec.parID;
							ChangeDirInfo(CurrentDir,spec.vRefNum,p_posdir);
							CurrentVref = TheVRefNum[wInteraction] = spec.vRefNum;
							FSClose(refnum);
							*p_changed = TRUE;
							*p_posdir = -1L;
							return(OK);
							}
						}
					if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
						spec.vRefNum = TheVRefNum[wInteraction] = CurrentVref;
						spec.parID = WindowParID[wInteraction] = CurrentDir;
						if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
							my_sprintf(Message,"\nCan't find '%s'. You should check the script.\n",
								line);
							Print(wTrace,Message);
							return(ABORT);
							}
						}
					FSClose(refnum);
					strcpy(FileName[wInteraction],line);
					LoadedIn = FALSE;
					return(LoadInteraction(FALSE,FALSE));
					break;
				case wGlossary:
					if(check == 2) return(OK);
					c2pstrcpy(fn, line);
					c2pstrcpy(spec.name, line);
					spec.vRefNum = TheVRefNum[wGlossary];
					spec.parID = WindowParID[wGlossary];
					if(check) {
						if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
	GOTIT4:						
							TheVRefNum[wGlossary] = spec.vRefNum;
							WindowParID[wGlossary] = spec.parID;
							FSClose(refnum);
							*p_posdir = -1L;
							return(OK);
							}
						else {
							spec.vRefNum = CurrentVref;
							spec.parID = CurrentDir;
							if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) goto GOTIT4;
							if(CheckFileName(wGlossary,line,&spec,&refnum,gFileType[wGlossary],FALSE)
								!= OK) return(ABORT);
							MystrcpyStringToTable(ScriptLine.arg,0,line); *p_newarg = TRUE;
							CurrentDir = WindowParID[wGlossary] = spec.parID;
							ChangeDirInfo(CurrentDir,spec.vRefNum,p_posdir);
							CurrentVref = TheVRefNum[wGlossary] = spec.vRefNum;
							FSClose(refnum);
							*p_changed = TRUE;
							*p_posdir = -1L;
							return(OK);
							}
						}
					if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
						spec.vRefNum = TheVRefNum[wGlossary] = CurrentVref;
						spec.parID = WindowParID[wGlossary] = CurrentDir;
						if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
							my_sprintf(Message,"\nCan't find '%s'. You should check the script.\n",
								line);
							Print(wTrace,Message);
							return(ABORT);
							}
						}
					FSClose(refnum);
					LoadedGl = CompiledGl = FALSE;
					ForgetFileName(wGlossary);
					strcpy(FileName[wGlossary],line);
					return(LoadGlossary(FALSE,FALSE));
					break;
				}
			if (strlen(line) > MAXNAME) {
				my_sprintf(Message,"\nFilename '%s' is too long. You should check the script.\n", line);
				Print(wTrace,Message);
				return(ABORT);
			}
			c2pstrcpy(fn, line);
			c2pstrcpy(spec.name, line);
			spec.vRefNum = TheVRefNum[w];
			spec.parID = WindowParID[w];
			if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
	GOTIT5:
				TheVRefNum[w] = spec.vRefNum;
				WindowParID[w] = spec.parID;
				if(check) {
					FSClose(refnum);
					return(OK);
					}
				if(w < 0 || w >= WMAX || !Editable[w]) {
					my_sprintf(Message,"Can't load non-editable file '%s'\n",line);
					Print(wTrace,Message);
					FSClose(refnum);
					return(ABORT);
					}
				if(w == wKeyboard) {
					r = LoadKeyboard(refnum);
					return(r);
					}
				if(w == wCsoundResources) {
					r = LoadCsoundInstruments(refnum,FALSE);
					if(r == OK) SetName(wCsoundResources,TRUE,TRUE);
					return(r);
					}
				if(w == wGrammar) {
					r = LoadGrammar(&spec,refnum);
					if(r == OK) {
						p2cstrcpy(FileName[wGrammar],spec.name);
						SetName(wGrammar,TRUE,TRUE);
						r = LoadAlphabet(wGrammar,&spec);
						}
					return(r);
					}
				if(ReadFile(w,refnum) != OK) {
					my_sprintf(Message,"Can't read '%s'... (no data)",FileName[w]);
					Alert1(Message);
					FSClose(refnum);
					return(ABORT);
					}
				p2cstrcpy(FileName[w],fn);  // FIXME ? should copy spec.name ?? (delete fn then)
				SetName(w,TRUE,TRUE);
				if(w == wGrammar || w == wAlphabet || w == wData) {
					TheVRefNum[wGrammar] = TheVRefNum[wInteraction] = TheVRefNum[wGlossary]
					= TheVRefNum[iSettings] = TheVRefNum[wAlphabet] = TheVRefNum[iObjects]
					= TheVRefNum[wTimeBase] = TheVRefNum[wKeyboard]
					= TheVRefNum[wCsoundResources] = TheVRefNum[wMIDIorchestra]
					= spec.vRefNum;
					WindowParID[wGrammar]
					= WindowParID[wInteraction] = WindowParID[wGlossary]
					= WindowParID[iSettings] = WindowParID[wTimeBase]
					= WindowParID[wAlphabet] = WindowParID[iObjects]
					= WindowParID[wKeyboard] = WindowParID[wCsoundResources]
					= WindowParID[wMIDIorchestra] = spec.parID;
					}
				GetHeader(w);
				SelectBehind(ZERO,ZERO,TEH[w]);
				if(w == wGrammar) {  // FIXME: remove block since it is never run ?/
					LoadLinkedMidiDriverSettings(w);
					GetTimeBaseName(w);
					GetCsName(w);
					GetFileNameAndLoadIt(wMIDIorchestra,w,LoadMIDIorchestra);
					}
				if(w == wAlphabet) {
					GetMiName(); GetKbName(w); GetCsName(w);
					GetFileNameAndLoadIt(wMIDIorchestra,w,LoadMIDIorchestra);
					}
				if(w == wData) {
					LoadLinkedMidiDriverSettings(w);
					GetTimeBaseName(w);
					GetCsName(w);
					GetFileNameAndLoadIt(wMIDIorchestra,w,LoadMIDIorchestra);
					if(LoadAlphabet(w,&spec) != OK) return(ABORT);
					}
				FSClose(refnum);
				BPActivateWindow(SLOW,wScript);
				}
			else {
				spec.vRefNum = CurrentVref;
				spec.parID = CurrentDir;
				if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) goto GOTIT5;
				if(!check) {
					for(ww=0; ww < WMAX; ww++) {
						if(!NeedSave[ww]) continue;
						spec.vRefNum = TheVRefNum[ww];
						spec.parID = WindowParID[ww];
						if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) goto GOTIT5;
						}
					my_sprintf(Message,"Can't find '%s'. You should check script.\n",
						line);
					Print(wTrace,Message);
					}
				else {
					if(CheckFileName(w,line,&spec,&refnum,gFileType[w],FALSE) != OK) return(ABORT);
					MystrcpyStringToTable(ScriptLine.arg,0,line); *p_newarg = TRUE;
					CurrentDir = WindowParID[w] = spec.parID;
					ChangeDirInfo(CurrentDir,spec.vRefNum,p_posdir);
					CurrentVref = TheVRefNum[w] = spec.vRefNum;
					FSClose(refnum);
					(*p_changed) = TRUE;
					(*p_posdir) = -1L;
					return(OK);
					}
				return(ABORT);
				}
			break;
		case 25:	// Clear window
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if((w=BPGetWindowIndex(line,0)) != ABORT) {
				ScriptW = w;
				if(check) return(OK);
				return(mClearWindow(w));
				}
			else r = ABORT;
			break; */
		/* case 26:	// Load settings
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check == 2) return(OK);
			if(strcmp(FileName[iSettings],line) == 0) return(OK);
			c2pstrcpy(fn, line);
			c2pstrcpy(spec.name, line);
			spec.vRefNum = TheVRefNum[iSettings];
			spec.parID = WindowParID[iSettings];
			if(check) {
				if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
	GOTIT6:
					TheVRefNum[iSettings] = spec.vRefNum;
					WindowParID[iSettings] = spec.parID;
					FSClose(refnum);
					*p_posdir = -1L;
					return(OK);
					}
				else {
					spec.vRefNum = CurrentVref;
					spec.parID = CurrentDir;
					if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) goto GOTIT6;
					if(CheckFileName(iSettings,line,&spec,&refnum,gFileType[iSettings],FALSE)
							!= OK) return(ABORT);
					MystrcpyStringToTable(ScriptLine.arg,0,line); *p_newarg = TRUE;
					CurrentDir = WindowParID[iSettings] = spec.parID;
					ChangeDirInfo(CurrentDir,spec.vRefNum,p_posdir);
					CurrentVref = TheVRefNum[iSettings] = spec.vRefNum;
					FSClose(refnum);
					*p_changed = TRUE;
					*p_posdir = -1L;
					return(OK);
					}
				}
			if((r=WaitForEmptyBuffer()) != OK) return(r);
			if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
				spec.vRefNum = TheVRefNum[iSettings] = CurrentVref;
				spec.parID = WindowParID[iSettings] = CurrentDir;
				if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
					my_sprintf(Message,"\nCan't find '%s'. You should check the script.\n",
						line);
					Print(wTrace,Message);
					return(ABORT);
					}
				}
			FSClose(refnum);
			strcpy(FileName[iSettings],line);
			return((TRUE,TRUE,NO,NO,&oms));
			break;
		case 27:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wGrammar);
			break;
		case 28:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wAlphabet);
			break;
		case 29:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wData);
			break;
		case 30:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wSettingsTop);
			if(!check) BPActivateWindow(SLOW,wSettingsBottom);
			break;
		case 31:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wInteraction);
			break;
		case 32:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wStartString);
			break;
		case 33:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wPrototype1);
			break;
		case 34:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wTrace);
			break;
		case 35:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wGraphic);
			break;
		case 36:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wControlPannel);
			break;
		case 37:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wScript);
			break;
		case 38:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wKeyboard);
			break;
		case 39:	// Set vref:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			CurrentVref = (short)(*(ScriptLine.intarg))[0];
			break;
		case 40:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) FreezeWindows = TRUE;
			break;
		case 41:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) FreezeWindows = FALSE;
			break;
		case 42:	// Activate window
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if((w=BPGetWindowIndex(line,0)) != ABORT) {
				ScriptW = w;
				if(check) return(OK);
				BPActivateWindow(SLOW,w);
				}
			else r = ABORT;
			break; */
		case 43:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			Nature_of_time = SMOOTH;
			break;
		case 44:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			Nature_of_time = STRIATED;
			break;
		case 45:	/* tempo */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			Qclock = (double)(*(ScriptLine.intarg))[0];
			Pclock = (double)(*(ScriptLine.intarg))[1];
			SetTempo(); SetTimeBase();
			break;
		/* case 47:	// Ignore buffer limit
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			UseBufferLimit = FALSE; SetBufferSize(); break; */
		/* case 48:	// Buffer size
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			BufferSize = ((*(ScriptLine.intarg))[0] + 1L) * 2L;
			SetBufferSize();
			break; */
		case 49:	/* Number streaks from 0 */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) StartFromOne = FALSE; break;
		case 50:	/* Number streaks from 1 */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) StartFromOne = TRUE; break;
		case 51:	/* Graphic scaling */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			GraphicScaleP = (*(ScriptLine.intarg))[0];
			GraphicScaleQ = (*(ScriptLine.intarg))[1] / 10L;
			break;
		case 52:	/* Quantize on */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) QuantizeOK = TRUE;
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			break;
		/* case 53:	// Reset keyboard
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) ResetKeyboard(YES); break; */
		case 54:	/* Time resolution */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if((r=WaitForEmptyBuffer()) != OK) return(r);
			Time_res = (*(ScriptLine.intarg))[0];
		// 	ResetDriver();
			break;
		case 55:	/* Reset random sequence */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) ResetRandom();
			break;
		case 56:	/* Randomize */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) ReseedOrShuffle(NEWSEED);
			break;
		case 57:	/* Seed */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			seed = (Seed + (*(ScriptLine.intarg))[0]) % 32768;
			srand(seed);
			UsedRandom = TRUE;
			break;
		case 58:	/* MIDI set-up time */
			if(wind == wInteraction) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			j= (*(ScriptLine.intarg))[0];
			if(j < 0 || j > 2000) {
				Print(wTrace,"=> Incorrect MIDI set-up time.  (Range 0..2000)\n");
				return(ABORT);
				}
			MIDIsetUpTime = j;
			break;
		case 59:	/* Selection start */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			j= (*(ScriptLine.intarg))[0];
			if(Editable[ScriptW]) {
				if(check) return(OK);
				if (j > GetTextLength(ScriptW)) {
					my_sprintf(Message,"Selection start value '%d' is out of bounds.\n", j);
					Print(wTrace,Message);
					r = ABORT;
					break;
					}
				TextGetSelection(&selbegin, &selend, TEH[ScriptW]);
				if(selend >= j)
					SelectBehind((long)j,selend,TEH[ScriptW]);
				else
					SelectBehind((long)j,(long)j,TEH[ScriptW]);
				}
			else {
				my_sprintf(Message,"Window '%s' is not editable.\n",WindowName[ScriptW]);
				Print(wTrace,Message);
				r = ABORT;
				}
			break;
		case 60:	/* Selection end */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			j= (*(ScriptLine.intarg))[0];
			if(Editable[ScriptW]) {
				if(check) return(OK);
				if (j > GetTextLength(ScriptW)) {
					my_sprintf(Message,"Selection end value '%d' is out of bounds.\n", j);
					Print(wTrace,Message);
					r = ABORT;
					break;
				}
				TextGetSelection(&selbegin, &selend, TEH[ScriptW]);
				if(selbegin <= j)
					SelectBehind(selbegin,(long)j,TEH[ScriptW]);
				else
					SelectBehind((long)j,(long)j,TEH[ScriptW]);
				ShowSelect(CENTRE,ScriptW);
				}
			else {
				my_sprintf(Message,"Window '%s' is not editable.\n",WindowName[ScriptW]);
				Print(wTrace,Message);
				r = ABORT;
				}
			break;
		case 65:	/* Quantization */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			Quantization = (*(ScriptLine.intarg))[0];
			break;
		case 46:	// Wait for Space
		case 47:	// Wait forever
		case 66: /* Wait for Continue */
		case 67: /* Wait for Start */
		case 128: /* Wait for Stop */
			if(check) return(OK);
			timethisscript = 1000 * Tcurr * Time_res; // microseconds
			for(j = 1; j <= Jinscript; j++) { // Is this script already there? It happens in improvisation
				if(((*p_INscript)[j]).scriptline == instr && ((*p_INscript)[j]).chan == 0 && ((*p_INscript)[j]).time == timethisscript) return OK;
				}
			for(j = 1; j <= Jinscript; j++) {  // Find empty space
				if((*p_INscript)[j].chan == -1) break;
				}
			if(j > Jinscript) {
				j = Jinscript + 1;
				if(++Jinscript >= Maxinscript) {
					if((p_INscript = (INscripttype**) IncreaseSpace((Handle)p_INscript)) == NULL)
						return(ABORT);
					Maxinscript = MyGetHandleSize((Handle)p_INscript) / sizeof(INscripttype);
					for(i = Jinscript; i < Maxinscript; i++) (*p_INscript)[i].chan = -1; // 2024-05-24
					}
				}
			((*p_INscript)[j]).chan = 0;
			((*p_INscript)[j]).scriptline = instr;
			((*p_INscript)[j]).time = timethisscript; // microseconds
			if(instr == 46) ((*p_INscript)[j]).key = 0; // Wait for Space
			if(instr == 47) ((*p_INscript)[j]).key = 0; // Wait forever
			if(instr == 66) ((*p_INscript)[j]).key = Continue;
			if(instr == 67) ((*p_INscript)[j]).key = Start;
			if(instr == 128) ((*p_INscript)[j]).key = Stop;
			if(TraceMIDIinteraction) {
				if(instr == 46) BPPrintMessage(0,odInfo,"[%d] INscript instruction %d, wait for Space, time = %lu ms\n",j,instr,((*p_INscript)[j]).time / 1000L);
				else if(instr == 47) BPPrintMessage(0,odInfo,"[%d] INscript instruction %d, wait forever, time = %lu ms\n",j,instr,((*p_INscript)[j]).time / 1000L);
				else  BPPrintMessage(0,odInfo,"[%d] INscript instruction %d, wait for MIDI event %d, time = %lu ms\n",j,instr,((*p_INscript)[j]).key,((*p_INscript)[j]).time / 1000L);
				}
			break;
		case 68:	/* Return */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			return(FINISH);
			break;
	/*	case 69:	// Pause
			if(check) return(OK);
			if((r=WaitForEmptyBuffer()) != OK) return(r);
			return(mPause(0));
			break; */
		case 70:	/* Set directory: */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			CurrentDir = (*(ScriptLine.intarg))[0];
			TextGetSelection(&selbegin, &selend, TEH[wTrace]);
			*p_posdir = selbegin;
			if(check) {
				for(w=0; w < WMAX; w++) {
					if(w == wScript) continue;
					WindowParID[w] = CurrentDir;
					TheVRefNum[w] = CurrentVref;
					}
				}
			break;
		case 71:	/* MIDI program */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			e.time = Tcurr;
			e.type = TWO_BYTE_EVENT;
			e.status = ProgramChange + CurrentChannel - 1;
			if((*(ScriptLine.intarg))[0] <= 0 && ProgNrFrom > 0) {
				if(Beta) Alert1("=> Err. DoScript(). Program number <= 0");
				(*(ScriptLine.intarg))[0] = 0;
				}
			e.data2 = (*(ScriptLine.intarg))[0] - ProgNrFrom;
			if((r=SendToDriver(0,0,0,Tcurr * Time_res,0,&rs,&e)) != OK) return(r);
			break;
		case 72:	/* Quantize off */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) QuantizeOK = FALSE;
			break;
	/*	case 73:	// Default buffer size
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			DeftBufferSize = ((*(ScriptLine.intarg))[0] + 1L) * 2L;
			SetBufferSize();
			break; */
	/*	case 74:	// Run script "filename"
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check == 2) return(OK);
			if(MemberStringList(line)) {
				my_sprintf(Message,"Script '%s' is already running. Can't restart it\n",
					line);
				Print(wTrace,Message);
				return(ABORT);
				}
			if(AppendStringList(line) != OK) return(ABORT);
			c2pstrcpy(fn, line);
			c2pstrcpy(spec.name, line);
			spec.vRefNum = TheVRefNum[wScript];
			spec.parID = WindowParID[wScript];
			if(!check) {
				if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
					spec.vRefNum = TheVRefNum[wScript] = CurrentVref;
					spec.parID = WindowParID[wScript] = CurrentDir;
					if((io=MyOpen(&spec,fsCurPerm,&refnum)) != noErr) {
						my_sprintf(Message,"\nCan't find '%s'. You should check the script.\n",
							line);
						Print(wTrace,Message);
						return(MISSED);
						}
					}
				FSClose(refnum);
				if(RunScriptOnDisk(check,line,&changed) != OK) return(ABORT);
				}
			else {
				if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) {
	GOTIT7:
					TheVRefNum[wScript] = spec.vRefNum;
					WindowParID[wScript] = spec.parID;
					FSClose(refnum);
					*p_posdir = -1L;
					return(OK);
					}
				else {
					spec.vRefNum = CurrentVref;
					spec.parID = CurrentDir;
					if((io=MyOpen(&spec,fsCurPerm,&refnum)) == noErr) goto GOTIT7;
					if(CheckFileName(wScript,line,&spec,&refnum,gFileType[wScript],FALSE) != OK) return(ABORT);
					MystrcpyStringToTable(ScriptLine.arg,0,line); *p_newarg = TRUE;
					CurrentDir = WindowParID[wScript] = spec.parID;
					CurrentVref = TheVRefNum[wScript] = spec.vRefNum;
					ChangeDirInfo(CurrentDir,spec.vRefNum,p_posdir);
					FSClose(refnum);
					*p_changed = TRUE;
					*p_posdir = -1L;
					return(OK);
					}
				}
			break; */
		case 75:	/* MIDI (undefined) controller */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + (*(ScriptLine.intarg))[2] - 1;
			e.data1 = (*(ScriptLine.intarg))[0];
			e.data2 = (*(ScriptLine.intarg))[1];
			if((r=SendToDriver(0,0,0,Tcurr * Time_res,0,&rs,&e)) != OK) return(r);
			break;
		case 76: /* MIDI local control off */
			if(wind == wInteraction) return(MISSED);
			if(check) return(OK);
	/*		if((r=WaitForEmptyBuffer()) != OK) return(r); */
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + (*(ScriptLine.intarg))[0] - 1;
			e.data1 = 122;
			e.data2 = 0;
			if((r=SendToDriver(0,0,0,Tcurr * Time_res,0,&rs,&e)) != OK) return(r);
			break;
		case 77: /* MIDI local control on */
			if(wind == wInteraction) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + (*(ScriptLine.intarg))[0] - 1;
			e.data1 = 122;
			e.data2 = 127;
			if((r=SendToDriver(0,0,0,Tcurr * Time_res,0,&rs,&e)) != OK) return(r);
			break;
		case 78: /* MIDI all notes off */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
	/*		if((r=WaitForEmptyBuffer()) != OK) return(r); */
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + (*(ScriptLine.intarg))[0] - 1;
			e.data1 = 123;
			e.data2 = 0;
			if((r=SendToDriver(0,0,0,Tcurr * Time_res,0,&rs,&e)) != OK) return(r);
			break;
		case 79: /* MIDI Omni mode off */
			if(wind == wInteraction) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + (*(ScriptLine.intarg))[0] - 1;
			e.data1 = 124;
			e.data2 = 0;
			if((r=SendToDriver(0,0,0,Tcurr * Time_res,0,&rs,&e)) != OK) return(r);
			break;
		case 80: /* MIDI Omni mode on */
			if(wind == wInteraction) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + (*(ScriptLine.intarg))[0] - 1;
			e.data1 = 125;
			e.data2 = 0;
			if((r=SendToDriver(0,0,0,Tcurr * Time_res,0,&rs,&e)) != OK) return(r);
			break;
		case 81: /* MIDI Mono mode ON ["0..16" voices] channel "1..16" */
			if(wind == wInteraction) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + (*(ScriptLine.intarg))[1] - 1;
			e.data1 = 126;
			e.data2 = (*(ScriptLine.intarg))[0];
			if((r=SendToDriver(0,0,0,Tcurr * Time_res,0,&rs,&e)) != OK) return(r);
			break;
		case 82: /* Poly mode ON channel "1..16" */
			if(wind == wInteraction) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + (*(ScriptLine.intarg))[0] - 1;
			e.data1 = 127;
			e.data2 = 0;
			if((r=SendToDriver(0,0,0,Tcurr * Time_res,0,&rs,&e)) != OK) return(r);
			break;
		case 83: /* MIDI decimal send "decimal data" */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if((r=WaitForEmptyBuffer()) != OK) return(r);
			if((p_line=(char**) GiveSpace((Size)MyGetHandleSize((Handle)
				(*ScriptLine.arg)[0]))) == NULL) return(ABORT);
			if(MystrcpyHandleToHandle(0,&p_line,(*ScriptLine.arg)[0]) != OK) return(ABORT);
			r = SendMIDIstream(check,p_line,FALSE);
			MyDisposeHandle((Handle*)&p_line);
			break;
		case 84: /* MIDI hexa send "hexadecimal data" */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if((r=WaitForEmptyBuffer()) != OK) return(r);
			if((p_line=(char**) GiveSpace((Size)MyGetHandleSize((Handle)
				(*ScriptLine.arg)[0]))) == NULL) return(ABORT);
			if(MystrcpyHandleToHandle(0,&p_line,(*ScriptLine.arg)[0]) != OK) return(ABORT);
			r = SendMIDIstream(check,p_line,TRUE);
			MyDisposeHandle((Handle*)&p_line);
			break;
		case 85: /* Play item */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(SoundOn) {
				my_sprintf(Message,"\nYou can't play '%s' while playing another item.\n",
					line);
				Print(wTrace,Message);
				return(MISSED);
				}
			if(check == 2) {	/* Compiling 'INIT:' in grammar */
				/* Here *p_Posdir is actually line position in grammar window. */
				*p_posdir += MyHandleLen((p_ScriptLabelPart(85,0)));
				TextGetSelection(&selbegin, &selend, TEH[wGrammar]);
				SelectBehind(*p_posdir,selend,TEH[wGrammar]);
				if(SelectionToBuffer(FALSE,FALSE,wGrammar,&p_Initbuff,p_posdir,PROD) == OK)
					InitThere = ((int) LengthOf(&p_Initbuff) > 0);
				else {
					N_err++; my_sprintf(Message,"Compiling 'INIT:': ??? %s  This item can't be played.\n",line);
					Print(wTrace,Message);
					}
				*p_posdir = -1L;
				}
			if(check) return(OK);
			if((r=WaitForEmptyBuffer()) != OK) return(r);
			if((p_line=(char**) GiveSpace((Size)MyGetHandleSize((Handle)
				(*ScriptLine.arg)[0]))) == NULL) return(ABORT);
			if(MystrcpyHandleToHandle(0,&p_line,(*ScriptLine.arg)[0]) != OK) return(ABORT);
			r = PlayHandle(p_line,NO);
			if(MyDisposeHandle((Handle*)&p_line) != OK) return(ABORT);
			if(r != OK) return(r);
			if(!check && ScriptExecOn == 0 && wind == wScript) r = ABORT;
			else r = OK;
			break;
		case 86: /* MIDI switch ON "64..95" channel "1..16" */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + (*(ScriptLine.intarg))[1] - 1;
			e.data1 = (*(ScriptLine.intarg))[0];
			e.data2 = 127;
			if((r=SendToDriver(0,0,0,Tcurr * Time_res,0,&rs,&e)) != OK) return(r);
			break;
		case 87: /* MIDI switch OFF "64..95" channel "1..16" */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			e.time = Tcurr;
			e.type = NORMAL_EVENT;
			e.status = ControlChange + (*(ScriptLine.intarg))[1] - 1;
			e.data1 = (*(ScriptLine.intarg))[0];
			e.data2 = 0;
			if((r=SendToDriver(0,0,0,Tcurr * Time_res,0,&rs,&e)) != OK) return(r);
			break;
		case 88:	/* Indian convention */
			if(check) return(OK);
			NoteConvention = INDIAN;
			break;
		case 89:	/* IN Derive further key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && DeriveFurtherChan != -1) {
				Print(wTrace,"'Derive further' is already controlled by ");
				PrintNote(-1,DeriveFurtherKey,DeriveFurtherChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
		//	if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			DeriveFurtherKey = (*(ScriptLine.intarg))[0];
			DeriveFurtherChan = (*(ScriptLine.intarg))[1];
			break;
		case 90:	/* IN Reset weights key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && ResetWeightChan != -1) {
				Print(wTrace,"'Reset weights' is already controlled by ");
				PrintNote(-1,ResetWeightKey,ResetWeightChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			ResetWeightKey = (*(ScriptLine.intarg))[0];
			ResetWeightChan = (*(ScriptLine.intarg))[1];
			break;
		case 91:	/* IN Start play key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && PlayChan != -1) {
				Print(wTrace,"'Start play' is already controlled by ");
				PrintNote(-1,PlayKey,PlayChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			PlayKey = (*(ScriptLine.intarg))[0];
			PlayChan = (*(ScriptLine.intarg))[1];
			break;
		case 92:	/* IN Repeat v times (velocity) key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && RepeatChan != -1) {
				Print(wTrace,"'Repeat v times' is already controlled by ");
				PrintNote(-1,RepeatKey,RepeatChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			RepeatKey = (*(ScriptLine.intarg))[0];
			RepeatChan = (*(ScriptLine.intarg))[1];
			break;
		case 93:	/* IN End repeat key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && EndRepeatChan != -1) {
				Print(wTrace,"'End repeat' is already controlled by ");
				PrintNote(-1,EndRepeatKey,EndRepeatChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			EndRepeatKey = (*(ScriptLine.intarg))[0];
			EndRepeatChan = (*(ScriptLine.intarg))[1];
			break;
		case 94:	/* IN Repeat forever key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && EverChan != -1) {
				Print(wTrace,"'Repeat forever' is already controlled by ");
				PrintNote(-1,EverKey,EverChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			EverKey = (*(ScriptLine.intarg))[0];
			EverChan = (*(ScriptLine.intarg))[1];
			break;
		case 95:	/* IN Quit key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && QuitChan != -1) {
				Print(wTrace,"'Quit' is already controlled by ");
				PrintNote(-1,QuitKey,QuitChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			QuitKey = (*(ScriptLine.intarg))[0];
			QuitChan = (*(ScriptLine.intarg))[1];
			break;
		case 96:	/* IN Use each substitution key "note" channel "1..16" [toggle] */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && UseEachSubChan != -1) {
				Print(wTrace,"'Use each substitution' is already controlled by ");
				PrintNote(-1,UseEachSubKey,UseEachSubChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			UseEachSubKey = (*(ScriptLine.intarg))[0];
			UseEachSubChan = (*(ScriptLine.intarg))[1];
			break;
		case 97:	/* IN Wait for note */
	//		if(wind == wGlossary) return(MISSED);
			if(check) return OK;
			timethisscript = 1000 * Tcurr * Time_res; // microseconds
			for(j = 1; j <= Jinscript; j++) { // Is this script already there? It happens in improvisation
				if(((*p_INscript)[j]).key == (*(ScriptLine.intarg))[0] && ((*p_INscript)[j]).chan == (*(ScriptLine.intarg))[1] && ((*p_INscript)[j]).scriptline == instr && ((*p_INscript)[j]).time == timethisscript) return OK;
				}
			for(j = 1; j <= Jinscript; j++) {  // Find empty space
				if((*p_INscript)[j].chan == -1) break;
				}
			if(j > Jinscript) {
				j = Jinscript + 1;
				if(++Jinscript >= Maxinscript) {
					if((p_INscript = (INscripttype**) IncreaseSpace((Handle)p_INscript)) == NULL)
						return(ABORT);
					Maxinscript = MyGetHandleSize((Handle)p_INscript) / sizeof(INscripttype);
					for(i = Jinscript; i < Maxinscript; i++) (*p_INscript)[i].chan = -1; // 2024-05-24
					}
				}
			((*p_INscript)[j]).key = (*(ScriptLine.intarg))[0];
			((*p_INscript)[j]).chan = (*(ScriptLine.intarg))[1];
			((*p_INscript)[j]).scriptline = instr;
			((*p_INscript)[j]).time = timethisscript; // microseconds
			if(TraceMIDIinteraction) BPPrintMessage(0,odInfo,"[%d] INscript instruction %d, key = %d chan = %d, time = %lu ms\n",j,instr,(*(ScriptLine.intarg))[0],(*(ScriptLine.intarg))[1],((*p_INscript)[j]).time / 1000L);
			break;
		case 98:	/* IN Control tempo controller #"0..127" channel "1..16" range "float" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && SynchronizeStartChan != -1) {
				my_sprintf(Message,"Tempo is already controlled by controller #%ld channel %ld\n",
					(long)SpeedCtrl,(long)SpeedChan);
				Print(wTrace,Message);
				return(MISSED);
				}
			SpeedCtrl = (*(ScriptLine.intarg))[0];
			SpeedChan = (*(ScriptLine.intarg))[1];
			SpeedRange = (*(ScriptLine.floatarg))[2];
			break;
		case 99:	/* IN Set computation time to 'v' key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && SetTimeChan != -1) {
				Print(wTrace,"'Set computation time' is already controlled by ");
				PrintNote(-1,SetTimeKey,SetTimeChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			SetTimeKey = (*(ScriptLine.intarg))[0];
			SetTimeChan = (*(ScriptLine.intarg))[1];
			break;
		case 100:	/* IN Smooth/striated time (toggle) key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && StriatedChan != -1) {
				Print(wTrace,"'Smooth/striated time' is already controlled by ");
				PrintNote(-1,StriatedKey,StriatedChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			StriatedKey = (*(ScriptLine.intarg))[0];
			StriatedChan = (*(ScriptLine.intarg))[1];
			break;
		case 101:	/* IN Use/ignore object constraints (toggle) key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && NoConstraintChan != -1) {
				Print(wTrace,"'Use/ignore object constraints' is already controlled by ");
				PrintNote(-1,NoConstraintKey,NoConstraintChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			NoConstraintKey = (*(ScriptLine.intarg))[0];
			NoConstraintChan = (*(ScriptLine.intarg))[1];
			break;
		case 102:	/* IN Skip next item key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && SkipChan != -1) {
				Print(wTrace,"'Skip next item' is already controlled by ");
				PrintNote(-1,SkipKey,SkipChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			SkipKey = (*(ScriptLine.intarg))[0];
			SkipChan = (*(ScriptLine.intarg))[1];
			break;
		case 103:	/* IN Play again item key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && AgainChan != -1) {
				Print(wTrace,"'Play again item' is already controlled by ");
				PrintNote(-1,AgainKey,AgainChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			AgainKey = (*(ScriptLine.intarg))[0];
			AgainChan = (*(ScriptLine.intarg))[1];
			break;
		case 104:	/* IN Synchronisation tag "Wx" = key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			i = (*(ScriptLine.intarg))[0];
			if(wind == wInteraction && WaitChan[i] != -1) {
				my_sprintf(Message,"Synchronisation tag W%ld is already controlled by ",
					(long)i);
				Print(wTrace,Message);
				PrintNote(-1,WaitKey[i],WaitChan[i],wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,1,2) != OK) return(MISSED);
			WaitKey[i] = (*(ScriptLine.intarg))[1];
			WaitChan[i] = (*(ScriptLine.intarg))[2];
			break;
		case 105:	/* Control parameter _Kx_ from #_0..127_ channel _1..16_ */
		//	if(wind == wGlossary) return(MISSED);
			if(!check) return(OK);
			i = (*(ScriptLine.intarg))[0];
			oldparamchan = ParamChan[i];
			oldparamcontrol = ParamControl[i];
	/*		if(ParamChan[i] != -1 && ParamControl[i] != -1) {
				my_sprintf(Message,"Parameter K%ld is already controlled by controller #%ld channel %ld\n",
					(long)i,(long)ParamControl[i],(long)ParamChan[i]);
				Print(wTrace,Message);
				return(MISSED);
				} */
			ParamControl[i] = (*(ScriptLine.intarg))[1];
			ParamControlChan = ParamChan[i] = (*(ScriptLine.intarg))[2];
			if(TraceMIDIinteraction && (oldparamchan != ParamChan[i] || oldparamcontrol != ParamControl[i]))
				BPPrintMessage(0,odInfo,"Script: Parameter K%d will be set by controller #%d chan %d\n",i,ParamControl[i],ParamChan[i]);
			return OK; // 2024-06-19
			break;
		case 106:	/* IN Adjust tempo minimum tempo "long" ticks in "long" secs key "note" maximum tempo "long" ticks in "long" secs key "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && TclockChan != -1) {
				Print(wTrace,"'Adjust tempo' is already controlled by ");
				PrintNote(-1,MinTclockKey,TclockChan,wTrace,Message);
				Print(wTrace," and ");
				PrintNote(-1,MaxTclockKey,TclockChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,2,6) != OK) return(MISSED);
			if(CheckUsedKey(p_keyon,5,6) != OK) return(MISSED);
			MinQclock = (double)(*(ScriptLine.intarg))[0];
			MinPclock = (double)(*(ScriptLine.intarg))[1];
			MinTclockKey = (*(ScriptLine.intarg))[2];
			MaxQclock = (double)(*(ScriptLine.intarg))[3];
			MaxPclock = (double)(*(ScriptLine.intarg))[4];
			MaxTclockKey = (*(ScriptLine.intarg))[5];
			TclockChan = (*(ScriptLine.intarg))[6];
			break;
		/* case 107:	// Reset interaction
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			ResetInteraction();
			break; */
		case 108:	/* Display Scrap */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wScrap);
			break;
		case 109:	/* Display info */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wNotice);
			break;
		case 110:	/* BP2 script */
			return(OK); break;
		case 111:  	/* Compute while play OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			ComputeWhilePlay = FALSE; 
			break;
		case 112:  	/* Compute while play ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			ComputeWhilePlay = TRUE; 
			break;
		case 113:  	/* Non-stop improvize OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			Improvize = FALSE;  
			break;
		case 114:  	/* Non-stop improvize ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			Improvize = ON;  
			break;
		case 115:  	/* Cyclic play OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			CyclicPlay = FALSE;  
			break;
		case 116:  	/* Cyclic play ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			CyclicPlay = TRUE;  
			break;
		case 117:  	/* Use each substitution OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			UseEachSub = FALSE; 
			break;
		case 118:  	/* Use each substitution ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			UseEachSub = TRUE; 
			break;
		case 119:  	/* Produce all items OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			AllItems = FALSE;  
			break;
		case 120:  	/* Produce all items ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			AllItems = TRUE;  
			break;
		case 121:  	/* Display items OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			DisplayItems = FALSE; 
			break;
		case 122:  	/* Display items ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			DisplayItems = TRUE; 
			break;
		case 123:  	/* Show graphic OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			ShowGraphic = FALSE; 
			break;
		case 124:  	/* Show graphic ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			ShowGraphic = TRUE; 
			break;
		case 125:  	/* Autorandomize OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			AllowRandomize = FALSE; 
			break;
		case 126:  	/* Autorandomize ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			AllowRandomize = TRUE; 
			break;
		case 127:  	/* Use MIDI in/out OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			rtMIDI = FALSE;  
			break;
		case 129:  	/* Synchronize start OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			SynchronizeStart = FALSE; 
			break;
		case 130:  	/* Synchronize start ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			SynchronizeStart = TRUE; 
			break;
		case 131:  	/* Interactive (use input) OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
		//	Interactive = FALSE;  
			break;
		case 132:  	/* Interactive (use input) ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			Interactive = TRUE;  
			break;
		case 133:  	/* Reset rule weights OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			ResetWeights = FALSE; 
			break;
		case 134:  	/* Reset rule weights ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			ResetWeights = TRUE; 
			NeverResetWeights = FALSE;
			break;
		case 135:  	/* Reset rule flags OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			ResetFlags = FALSE; 
			break;
		case 136:  	/* Reset rule flags ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			ResetFlags = TRUE; 
			break;
		case 137:  	/* Ignore constraints OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			NoConstraint = OFF; 
			break;
		case 138:  	/* Ignore constraints ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			NoConstraint = ON; 
			break;
		case 139:  	/* Show messages OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			ShowMessages = OFF; 
			break;
		case 140:  	/* Show messages ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			ShowMessages = ON; 
			break;
		case 141:  	/* Reset controllers OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			ResetControllers = FALSE; 
			break;
		case 142:  	/* Reset controllers ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			ResetControllers = TRUE; 
			break;
		case 143:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			BPActivateWindow(SLOW,wGlossary);
			break;
		case 144: return(OK); break;
		case 145:	/* Define... */
			if(wind != wGlossary) {
				Print(wTrace,"'Define...' should appear only in the \"Glossary\" window.\n");
				return(MISSED);
				}
			break;
		case 146:	/* MIDI set default channel to "1..16" */
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			CurrentChannel = (*(ScriptLine.intarg))[0]; break;
		case 147:	/* Produce and play "int" items */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if((r=WaitForEmptyBuffer()) != OK) return(r);
			ItemNumber = ZERO;
			Maxitems = (*(ScriptLine.intarg))[0];
			ReadKeyBoardOn = FALSE; Jcontrol = -1;
		/*	if(rtMIDI && Interactive && !LoadedIn) {
				if(GetInName(wData) != OK) GetInName(wGrammar);
				if(LoadInteraction(TRUE,FALSE) != OK) return(OK);
				} */
			if(CompileCheck() != OK) return(MISSED);
			i = CyclicPlay; CyclicPlay = FALSE;
			j = Improvize; Improvize = TRUE;
			oldoutmidi = rtMIDI;
			if(!OutCsound) {
	#if WITH_REAL_TIME_MIDI_FORGET_THIS
				rtMIDI = TRUE;
				if(rtMIDI && !oldoutmidi) ResetMIDI(FALSE);
	#else
				CyclicPlay = i; Improvize = j;
				return(MISSED);
	#endif
				}
			displayitems = DisplayItems;
			DisplayItems = FALSE;
		//	
		//	GetValues(TRUE);
			r = OK;
			Ctrlinit();
			// HideWindow(Window[wMessage]);
			r = ProduceItems(wStartString,FALSE,FALSE,NULL);
			if(!check && ScriptExecOn == 0 && wind == wScript) r = ABORT;
			else if(r != EXIT) r = OK;
			CyclicPlay = i; Improvize = j;
			DisplayItems = displayitems;
			rtMIDI = oldoutmidi;
			
			if(r == EXIT) return(r);
			break;
		case 148:	/* Beep */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if((r=WaitForEmptyBuffer()) != OK) return(r);
			SysBeep(10);
			break;
		case 149:	/* Prompt ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			OkWait = TRUE;
			break;
		case 150:	/* Prompt OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			OkWait = FALSE;
			break;
		/* case 151:	// Play-show "int" times selection in window "windowname
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if((w=BPGetWindowIndex(line,0)) != ABORT) {
				ScriptW = w;
				if(check) return(OK);
				BPActivateWindow(SLOW,w);
				Nplay = (*(ScriptLine.intarg))[0];
				if((r=WaitForEmptyBuffer()) != OK) return(r);
				return(mPlaySelect(w));
				}
			else r = ABORT;
			break; */
		case 152:  	/* Display time setting OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			DisplayTimeSet = FALSE; 
			break;
		case 153:  	/* Display time setting ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			DisplayTimeSet = TRUE; 
			break;
		case 154:  	/* Step time setting OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			StepTimeSet = FALSE; 
			break;
		case 155:  	/* Step time setting ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			StepTimeSet = TRUE; 
			break;
		case 156:  	/* Trace time setting OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			TraceTimeSet = FALSE; 
			break;
		case 157:  	/* Trace time setting ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			TraceTimeSet = TRUE; 
			break;
	/*	case 158:	// Hide window "windowname 
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if((w=BPGetWindowIndex(line,0)) != ABORT) {
				ScriptW = w;
				if(check) return(OK);
				GoAway(w);
				}
			else r = ABORT;
			break; */
		case 159:	/* Text Color ON */
			if(check) return(OK);
			// ForceTextColor = +1; UseTextColor = TRUE;
			break;
		case 160:	/* Text Color OFF */
			if(check) return(OK);
			// ForceTextColor = -1; UseTextColor = FALSE;
			break;
		case 161:	/* IN On note channel "1..16" do "script instruction" */
		//	if(wind != wInteraction) return(MISSED);
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			if(check) return(OK);
			if(++Jinscript >= Maxinscript) {
				if((p_INscript = (INscripttype**) IncreaseSpace((Handle)p_INscript)) == NULL)
					return(ABORT);
				Maxinscript = MyGetHandleSize((Handle)p_INscript) / sizeof(INscripttype);
				for(i = Jinscript; i < Maxinscript; i++) (*p_INscript)[i].chan = -1; // 2024-05-24
				}
			((*p_INscript)[Jinscript]).key = (*(ScriptLine.intarg))[0];
			((*p_INscript)[Jinscript]).chan = (*(ScriptLine.intarg))[1]; 
			((*p_INscript)[Jinscript]).scriptline = (*(ScriptLine.intarg))[2];
			break;
		case 162:	/* Graphic Color ON */
			if(check) return(OK);
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			// ForceGraphicColor = +1; UseGraphicsColor = TRUE;
			break;
		case 163:	/* Graphic Color OFF */
			if(check) return(OK);
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			// ForceGraphicColor = -1; UseGraphicsColor = FALSE;
			break;
		case 164:	/* Set output window "windowname" */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if((w=BPGetWindowIndex(line,0)) != ABORT) {
				if(check) return(OK);
				OutputWindow = w;
				}
			else r = ABORT;
			break;
		case 165:	/* Tick cycle ON */
			if(check) return(OK);
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			HideTicks = FALSE;
			break;
		case 166:	/* Tick cycle OFF */
			if(check) return(OK);
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			HideTicks = TRUE;
			break;
	/*	case 167:	// AE send fast class '"AEclass"' ID '"AEID"' to application '"signature"'/
		case 168:	// AE send normal class '"AEclass"' ID '"AEID"' to application '"signature"' /
			if(check) return(OK);
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if((r=WaitForEmptyBuffer()) != OK) return(r);
			theclass = (*(ScriptLine.ularg))[0];
			theID = (*(ScriptLine.ularg))[1];
			thesignature = (*(ScriptLine.ularg))[2];
			MystrcpyTableToString(5,type1,ScriptLine.arg,0);
			MystrcpyTableToString(5,type2,ScriptLine.arg,1);
			MystrcpyTableToString(5,type3,ScriptLine.arg,2);
			my_sprintf(Message,"Sending Apple Event class '%s' ID '%s' to '%s'",
				type1,type2,type3);
			if(ShowMessages) ShowMessage(TRUE,wMessage,Message);
			tried = FALSE;
			
	SENDAE:
			io = AECreateDesc(typeApplSignature,(Ptr)&thesignature,sizeof(thesignature),
				&thetarget);
			if(io != noErr) goto BAD;
			io = AECreateAppleEvent(theclass,theID,&thetarget,kAutoGenerateReturnID,
				kAnyTransactionID,&theAppleEvent);
			if(io != noErr) goto BAD;
			if(instr == 167) priority = kAEHighPriority;
			else  priority = kAENormalPriority;
			
			io = AESend(&theAppleEvent,&reply,kAENoReply+kAECanSwitchLayer+kAEAlwaysInteract,
				priority,kAEDefaultTimeout,(AEIdleUPP)NULL,(AEFilterUPP)NULL);
			AEDisposeDesc(&theAppleEvent);
			AEDisposeDesc(&thetarget);
			
			if(io != noErr) {
				if(!tried && OpenApplication(thesignature) == OK) {
					tried = TRUE;
					goto SENDAE;
					}
	BAD:		
				my_sprintf(Message,"\nApple Event class '%s' ID '%s' couldn't reach application '%s'...",
					type1,type2,type3);
				Println(wTrace,Message);
				if(OkWait) r = MISSED;
				else r = OK;
				}
			break;
		case 169:	// AE wait class '"AEclass"' ID '"AEID"'
			if(check) return(OK);
			if((r=WaitForEmptyBuffer()) != OK) return(r);
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			theclass = (*(ScriptLine.ularg))[0];
			theID = (*(ScriptLine.ularg))[1];
			MystrcpyTableToString(5,type1,ScriptLine.arg,0);
			MystrcpyTableToString(5,type2,ScriptLine.arg,1);
			WaitKeyStrokeOrAppleEvent(' ',NO,APPLEVENT,theclass,theID,type1,type2);
			break; */
		case 170:	/* Stop */
			if(check) return(OK);
			if((r=WaitForEmptyBuffer()) != OK) return(r);
		//	return(mStop(0));
			break;
		case 171: break; /* (- */
		case 172: break; /* (- */
		case 173:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wTimeBase);
		case 174: break; /* Reset session time */
		case 175: break; /* Tell session time */
		case 176: /* Reset tick cycle */
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			ResetTickInItemFlag = TRUE;
			break;
		case 177:	/* Time ticks OFF */
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			PlayTicks = FALSE;
			SetTickParameters(0,MAXBEATS);
			break;
		case 178:	/* Time ticks ON */
			if(check) return(OK);
		/*	if((r=WaitForEmptyBuffer()) != OK) return(r); */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			PlayTicks = TRUE;
			ResetTickFlag = TRUE;
			SetTickParameters(0,MAXBEATS);
			break;
		case 179:
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(!check) BPActivateWindow(SLOW,wMetronom);
	/*	case 180: // Set output Csound file "filename"
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			CloseCsScore();
			if(MakeCsFile(line) != OK) return(MISSED);
			break; */
	/*	case 181: // Set output MIDI file "filename"
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			CloseMIDIFile();
			if(MakeMIDIFile(line) != OK) return(MISSED);
			break; */
	/*	case 182: // Opcode OMS ON
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(!Oms) mOMS(0);
			break;
		case 183: // Opcode OMS OFF
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(Oms) mOMS(0);
			break;
		case 184: // MIDI file ON
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			WriteMIDIfile = TRUE;
			break;
		case 185: // MIDI file OFF
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			oldwritemidifile = WriteMIDIfile;
			WriteMIDIfile = FALSE;
			if(oldwritemidifile) CloseMIDIFile();
			break; */
		case 186: /* Csound trace ON */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			CsoundTrace = TRUE;
			break;
		case 187: /* Csound trace OFF */
			if(wind == wInteraction || wind == wGlossary) return(MISSED);
			if(check) return(OK);
			CsoundTrace = FALSE;
			break;
		case 188:	/* IN Mute ON "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && MuteOnChan != -1) {
				Print(wTrace,"'Mute ON' is already controlled by ");
				PrintNote(-1,MuteOnKey,MuteOnChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			if(!Oms && !NEWTIMER_FORGET_THIS) {
				Print(wTrace,"'Mute' works only with OMS, not with the in-built MIDI driver. 'IN Mute ON' has been ignored\n");
				return(OK);
				}
			MuteOnKey = (*(ScriptLine.intarg))[0];
			MuteOnChan = (*(ScriptLine.intarg))[1];
			break;
		case 189:	/* IN Mute OFF "note" channel "1..16" */
			if(wind == wGlossary) return(MISSED);
			if(check) return(OK);
			if(wind == wInteraction && MuteOffChan != -1) {
				Print(wTrace,"'Mute OFF' is already controlled by ");
				PrintNote(-1,MuteOffKey,MuteOffChan,wTrace,Message);
				Print(wTrace,"\n");
				return(MISSED);
				}
			if(CheckUsedKey(p_keyon,0,1) != OK) return(MISSED);
			if(!Oms && !NEWTIMER_FORGET_THIS) {
				Print(wTrace,"'Mute' works only with OMS, not with the in-built MIDI driver. 'IN Mute OFF' has been ignored\n");
				return(OK);
				}
			MuteOffKey = (*(ScriptLine.intarg))[0];
			MuteOffChan = (*(ScriptLine.intarg))[1];
			break;
		default:
			return(MISSED);
		}
	return(r);
	}


int BPGetWindowIndex(char* arg,int pos)
{
char *p,*q,line[MAXLIN];
int w,j;

strcpy(line,arg);
for(w=0; w < WMAX; w++) {
	p = &(line[pos]);
	q = &(WindowName[w][0]);
	j = strlen(WindowName[w]);
	if(Match(NO,&p,&q,j)) return(w);
	}
return(ABORT);
}


/* RecordVrefInScript(FSSpec *p_spec)
{
char line[20];
long parid;
short vref;

if(!ScriptRecOn) return(OK);
parid = p_spec->parID;
vref = p_spec->vRefNum;
if(CurrentVref != vref) {
	my_sprintf(Message,"%ld",(long)vref);
	MystrcpyStringToTable(ScriptLine.arg,0,Message);
	AppendScript(39);
	CurrentVref = vref;
	}
if(CurrentDir != parid) {
	my_sprintf(Message,"%ld",(long)parid);
	MystrcpyStringToTable(ScriptLine.arg,0,Message);
	AppendScript(70);
	CurrentDir = parid;
	}
return(OK);
} */


int GetScriptArguments(int k,char** p_args,int istart)
{
int i,imax,ii,j,n,r,nospace,bar,quote;
char c,*p,*q,line[MAXLIN],type[5];
Handle h;
unsigned long x;

i = istart;
r = MISSED;
(*((*(ScriptLine.arg))[0]))[0] = '\0';
for(j=0; j < ScriptNrArg(k); j++) {
	if(i >= MyHandleLen(p_args)) {
		Print(wTrace,"\n=> Incorrect number of arguments\n");
		goto QUIT;
		}
	imax = MyGetHandleSize((Handle)(*(ScriptLine.arg))[j]);
	imax = (imax / sizeof(char)) - 1;
	while(MySpace((*p_args)[i])) i++;
	MystrcpyHandleToString(MAXLIN,0,line,p_ScriptLabelPart(k,j));
	p = &((*p_args)[i]); q = &(line[0]);
	n = strlen(line);
	if(!Match(FALSE,&p,&q,n)) {
		BPPrintMessage(0,odError,"=> Mismatch of script argument\n");
		goto QUIT;
		}
	i += n; while(MySpace((*p_args)[i])) i++;
	MystrcpyHandleToString(MAXLIN,0,line,p_ScriptArgPart(k,j));
//	BPPrintMessage(0,odError,"line = %s\n",line);
	if(strcmp(line,"int") == 0) {
		if(((*(ScriptLine.intarg))[j] = n = GetInteger(YES,(*p_args),&i)) == INT_MAX)
			goto QUIT;
		continue;
		}
	if(strcmp(line,"0..127") == 0) {
		if(((*(ScriptLine.intarg))[j] = n = GetInteger(YES,(*p_args),&i)) == INT_MAX)
			goto QUIT;
		if(n < 0 || n > 127) {
			Print(wTrace,"\nNumber out of range [0..127]\n");
			goto QUIT;
			}
		continue;
		}
	if(strcmp(line,"1..128") == 0) {
		if(((*(ScriptLine.intarg))[j] = n = GetInteger(YES,(*p_args),&i)) == INT_MAX)
			goto QUIT;
		if(n < 1 || n > 128) {
			Print(wTrace,"\nNumber out of range [1..128]\n");
			goto QUIT;
			}
		continue;
		}
	if(strcmp(line,"64..95") == 0) {
		if(((*(ScriptLine.intarg))[j] = n = GetInteger(YES,(*p_args),&i)) == INT_MAX)
			goto QUIT;
		if(n < 64 || n > 95) {
			Print(wTrace,"\nNumber out of range [64..95]\n");
			goto QUIT;
			}
		continue;
		}
	if(strcmp(line,"1..16") == 0) {
		if(((*(ScriptLine.intarg))[j] = n = GetInteger(YES,(*p_args),&i)) == INT_MAX)
			goto QUIT;
		if(n < 1 || n > 16) {
			Print(wTrace,"\nNumber out of range [1..16]\n");
			goto QUIT;
			}
		continue;
		}
	if(strcmp(line,"0..16") == 0) {
		if(((*(ScriptLine.intarg))[j] = n = GetInteger(YES,(*p_args),&i)) == INT_MAX)
			goto QUIT;
		if(n < 0 || n > 16) {
			Print(wTrace,"\nNumber out of range [0..16]\n");
			goto QUIT;
			}
		continue;
		}
	if(strcmp(line,"long") == 0) {
		if(((*(ScriptLine.intarg))[j] = GetLong((*p_args),&i)) == Infpos)
			goto QUIT;
		if((*(ScriptLine.intarg))[j] < ZERO) {
			Print(wTrace,"\nNumber should be positive\n");
			goto QUIT;
			}
		continue;
		}
	if(strcmp(line,"unsigned") == 0) {
		if(((*(ScriptLine.intarg))[j] = (long) GetUnsigned((*p_args),&i)) == UINT16_MAX)
			goto QUIT;
		continue;
		}
	if(strcmp(line,"float") == 0 || strcmp(line,"double") == 0) {
		if(((*(ScriptLine.floatarg))[j] = GetDouble((*p_args),&i)) == -1.)
			goto QUIT;
		continue;
		}
	if(strcmp(line,"note") == 0) {
		if(GetNote(&((*p_args)[i]),&n,&ii,TRUE) != OK) {
			my_sprintf(Message,"\n=> Incorrect note. (May be wrong note convention)\n");
			Print(wTrace,Message);
			goto QUIT;
			}
		(*(ScriptLine.intarg))[j] = n;
		while(!MySpace((*p_args)[i])) i++;
		while(MySpace((*p_args)[i])) i++;
		if(isdigit((*p_args)[i])) while(isdigit((*p_args)[i])) i++;
		else i--;
		continue;
		}
	if(strcmp(line,"Kx") == 0) {
		if((c=(*p_args)[i]) != 'K') {
			Expect('K',*(p_ScriptLabelPart(105,0)),c);
			goto QUIT;
			}
		i++;
		if(((*(ScriptLine.intarg))[j] = n = GetInteger(YES,(*p_args),&i)) == INT_MAX)
			goto QUIT;
		if(n < 1 || n >= MAXPARAMCTRL) {
			my_sprintf(Message,"\nController 'Kx' out of range: 0 < x < %ld\n",(long)MAXPARAMCTRL);
			Print(wTrace,Message);
			goto QUIT;
			}
		continue;
		}
	if(strcmp(line,"Wx") == 0) {
		if((c=(*p_args)[i]) != 'W') {
			Expect('W',*(p_ScriptLabelPart(104,0)),c);
			goto QUIT;
			}
		i++;
		if(((*(ScriptLine.intarg))[j] = n = GetInteger(YES,(*p_args),&i)) == INT_MAX)
			goto QUIT;
		if(n < 1 || n > MAXWAIT) {
			my_sprintf(Message,"\nSynchro tag 'Wx' out of range: 0 < x < %ld\n",(long)MAXWAIT+1);
			Print(wTrace,Message);
			goto QUIT;
			}
		continue;
		}
	if(strcmp(line,"variable") == 0) {
		if(!isupper(c=(*p_args)[i]) && c != '|') goto BADVAR;
		if(c == '|') bar = TRUE; else bar = FALSE;
		i++; while(!MySpace(c=(*p_args)[i])) {
			if(!(bar && c == '|') && !OkChar(c)) goto BADVAR;
			i++;
			}
		if(bar && (*p_args)[i-1] != '|') goto BADVAR;
		continue;
BADVAR:
		Print(wTrace,"\n=> Incorrect variable\n");
		goto QUIT;
		}
/*	if(strcmp(line,"AEclass") == 0) {
		x = ZERO;
		for(ii=i; ii < i+4; ii++) {
			type[ii-i] = (*p_args)[ii];
			x = (256 * x) + (unsigned)(*p_args)[ii];
			}
		type[ii-i] = '\0';
		i = ii;
		if((*p_args)[i] != '\'') goto BADCLASS;
		ii = 0;
		(*(ScriptLine.ularg))[j] = x;
		MystrcpyStringToTable(ScriptLine.arg,j,type);
		continue;
	BADCLASS:
		my_sprintf(Message,"\n=> Incorrect Apple Event class '%s'\n",type);
		Print(wTrace,Message);
		goto QUIT;
		}
	if(strcmp(line,"AEID") == 0) {
		x = ZERO;
		for(ii=i; ii < i+4; ii++) {
			type[ii-i] = (*p_args)[ii];
			x = (256 * x) + (unsigned)(*p_args)[ii];
			}
		type[ii-i] = '\0';
		i = ii;
		if((*p_args)[i] != '\'') goto BADID;
		ii = 0;
		(*(ScriptLine.ularg))[j] = x;
		MystrcpyStringToTable(ScriptLine.arg,j,type);
		continue;
BADID:
		my_sprintf(Message,"\n=> Incorrect Apple Event ID '%s'\n",type);
		Print(wTrace,Message);
		goto QUIT;
		}
	if(strcmp(line,"signature") == 0) {
		x = ZERO;
		for(ii=i; ii < i+4; ii++) {
			type[ii-i] = (*p_args)[ii];
			x = (256 * x) + (unsigned)(*p_args)[ii];
			}
		type[ii-i] = '\0';
		i = ii;
		if((*p_args)[i] != '\'') goto BADCREATOR;
		ii = 0;
		(*(ScriptLine.ularg))[j] = x;
		MystrcpyStringToTable(ScriptLine.arg,j,type);
		continue;
BADCREATOR:
		my_sprintf(Message,"\n=> Incorrect application creator '%s'\n",type);
		Print(wTrace,Message);
		goto QUIT;
		} */
	if(strcmp(line,"script instruction") == 0) {
	//	continue; // 2024-05-24
		if(p_Script == NULL && GetScriptSpace() != OK) {
			r = ABORT; goto QUIT;
			}
		n = CreateEventScript(&(*p_args)[i],FALSE);
		if(n < 0) {
			r = n; goto QUIT;
			}
		(*(ScriptLine.intarg))[j] = n;
		while(!MySpace((*p_args)[i])) i++;
		while(MySpace((*p_args)[i])) i++;
		continue;
		}
	ii = 0;
	nospace = TRUE;
	switch(k) {
		case 14:
		case 16:
		case 83:
		case 84:
		case 85:
			nospace = FALSE; break;
		}
	if((*p_args)[i] == '"') {
		i++; quote = TRUE;
		}
	else quote = FALSE;
	while((!MySpace((c=(*p_args)[i])) || !nospace) && c != '\0') {
		if(c == '"') quote = 1 - quote;
		else (*((*(ScriptLine.arg))[j]))[ii++] = c;
		if(ii >= imax) {
			h = (Handle) (*(ScriptLine.arg))[j];
			imax = MyGetHandleSize((Handle)p_args);
			MySetHandleSize((Handle*)&h,(Size)imax);
			/* It will be resized down after use */
			(*(ScriptLine.arg))[j] = h;
			imax = (imax / sizeof(char)) - 1;
			}
		i++;
		}
	(*((*(ScriptLine.arg))[j]))[ii] = '\0';
	if(quote) {
		Print(wTrace,"\nMissing terminating quote\n");
		goto QUIT;
		}
	}
r = OK;

QUIT:
return(r);
}


/* ChangeDirInfo(long dir,short vref,long *p_posdir)
{
TextOffset dummy;
long pos,posmax;
int i,j;

pos = *p_posdir;
UpdateThisWindow(FALSE,Window[wTrace]);
ShowSelect(CENTRE,wTrace);
posmax = GetTextLength(wTrace);
if(pos < posmax) {
	if(pos >= ZERO) {
		for(i=pos,j=0; i <= posmax; i++) {
			if(GetTextChar(wTrace,i) == '\r') {
				j++;
				if(j == 2) break;
				}
			}
		}
	else {
		TextGetSelection(&pos, &dummy, TEH[wTrace]);
		i = pos;
		}
	SelectBehind(pos,(long)i,TEH[wTrace]);
	TextDelete(wTrace);
	if(vref != CurrentVref) {
		PrintBehind(wTrace,*(p_ScriptLabelPart(39,0)));
		PrintBehind(wTrace," ");
		my_sprintf(Message,"%ld\n",(long)vref);
		PrintBehind(wTrace,Message);
		}
	PrintBehind(wTrace,*(p_ScriptLabelPart(70,0)));
	PrintBehind(wTrace," ");
	my_sprintf(Message,"%ld\n",(long)dir);
	PrintBehind(wTrace,Message);
	UpdateThisWindow(FALSE,Window[wTrace]);
	ShowSelect(CENTRE,wTrace);
	SelectBehind(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
	}
else return(MISSED);
return(OK);
} */


/* RecordButtonClick(int i,int v)
{
int j;

j = -1;
if(v == 0) {
	switch(i) {
		case bMIDI: j = 127; break;
		case bImprovize: j = 113; break;
		case bUseEachSub: j = 117; break;
		case bAllItems: j = 119; break;
		case bAllowRandomize: j = 125; break;
		case bCyclicPlay: j = 115; break;
		case bSynchronizeStart: j = 129; break;
		case bDisplayItems: j = 121; break;
		case bResetWeights: j = 133; break;
		case bResetFlags: j = 135; break;
		case bResetControllers: j = 141; break;
		case bNoConstraint: j = 137; break;
		case bShowGraphic: j = 123; break;
		case bInteractive: j = 131; break;
		case bShowMessages: j = 139; break;
		case bDisplayTimeSet: j = 152; break;
		case bStepTimeSet: j = 154; break;
		case bTraceTimeSet: j = 156; break;
		case bComputeWhilePlay: j = 111; break;
		case bWriteMIDIfile: j = 185; break;
		case bCsound: j = 23; break;
		case bCsoundTrace: j = 187; break;
		case bOMS: j = -1; break;	// j = 183
		}
	}
else {
	switch(i) {
		case bMIDI: j = 128; break;
		case bImprovize: j = 114; break;
		case bUseEachSub: j = 118; break;
		case bAllItems: j = 120; break;
		case bAllowRandomize: j = 126; break;
		case bCyclicPlay: j = 116; break;
		case bSynchronizeStart: j = 130; break;
		case bDisplayItems: j = 122; break;
		case bResetWeights: j = 134; break;
		case bResetControllers: j = 142; break;
		case bResetFlags: j = 136; break;
		case bNoConstraint: j = 138; break;
		case bShowGraphic: j = 124; break;
		case bInteractive: j = 132; break;
		case bShowMessages: j = 140; break;
		case bDisplayTimeSet: j = 153; break;
		case bStepTimeSet: j = 155; break;
		case bTraceTimeSet: j = 157; break;
		case bComputeWhilePlay: j = 112; break;
		case bWriteMIDIfile: j = 184; break;
		case bCsound: j = 22; break;
		case bCsoundTrace: j = 186; break;
		case bOMS: j = -1; break;	// j = 182;
		}
	}
if(j > 0) AppendScript(j);
return(OK);
} */


int CheckUsedKey(char*** p_keyon,int i1,int i2)
{
int key,k,chan,ch;

key = (*(ScriptLine.intarg))[i1];
chan = (*(ScriptLine.intarg))[i2];
if(p_keyon == NULL) { // 2024-06-18
	return OK;
/*	for(ch=0; ch < MAXCHAN; ch++) {
		if((p_keyon[ch] = (char**) GiveSpace((Size)(MAXKEY+1)*sizeof(char))) == NULL) return(ABORT);
		for(k=0; k < MAXKEY; k++) (*p_keyon[ch])[k] = 0;
		}
	return OK; */
	}
// BPPrintMessage(0,odInfo,"Script key = %d chan = %d, on = %d\n",key,chan,(int)(*p_keyon[chan])[key]);
if((*p_keyon[chan])[key]) {
	Print(wTrace,"Key '");
	PrintNote(-1,key,chan,wTrace,Message);
	Print(wTrace,"' is already used.\n");
	return(MISSED);
	}
else (*p_keyon[chan])[key] = TRUE;
return(OK);
}