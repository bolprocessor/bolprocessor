/* PlayThings.c (BP3) */ 

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


int PlaySelection(int w, int all) {
	char c;
	int i,ch,r,improvize,asked,askedvariables,derivevariables,check;
	tokenbyte **p_a;
	long origin,originmem,next_origin,firstorigin,end,x;

	if(Panic || CheckEmergency() != OK) return(ABORT);

	if(OutCsound) PrepareCsFile(); // The same file will be used for all chunks or items
	if(WriteMIDIfile) PrepareMIDIFile();

	#if BP_CARBON_GUI_FORGET_THIS
	if(GetTuning() != OK) return(ABORT);
	#endif /* BP_CARBON_GUI_FORGET_THIS */

	asked = FALSE;
	/* if(w != LastComputeWindow && w >= 0 && w < WMAX && Editable[w]) LastComputeWindow = w;
	w = LastComputeWindow; */  
	/* if(w < 0 || w >= WMAX || !Editable[w]) {
		if(Beta) Alert1("=> Err. PlaySelection(). Incorrect window index");
		return(MISSED);
		} */
		
	#if BP_CARBON_GUI_FORGET_THIS
	if(w == wScript) {
		EndWriteScript();
		return(RunScript(wScript,FALSE));
		}
	#endif /* BP_CARBON_GUI_FORGET_THIS */

	// if(WillRandomize) ReseedOrShuffle(RANDOMIZE);

	BPActivateWindow(SLOW,w);
	TextGetSelection(&origin, &end, TEH[w]);
	improvize = FALSE;
	if(Improvize) {
		improvize = TRUE;
		Improvize = FALSE;
		
		}
	if(all) {
	//	ShowGraphic = ShowPianoRoll = ShowObjectGraph = FALSE;
		BPPrintMessage(odError,"\n(No message when playing chunks.)\n");
		PlayAllChunks = TRUE;
		}

	// PlaySelectionOn++; // Fixed by BB 2021-02-17
	// ResetMIDI(TRUE); // Fixed by BB 2022-02-18

	r = ABORT;

	#if BP_CARBON_GUI_FORGET_THIS
	if(SaveCheck(wAlphabet) == ABORT) goto END;
	if(SaveCheck(wGrammar) == ABORT) goto END;
	if(SaveCheck(wInteraction) == ABORT) goto END;
	if(SaveCheck(wGlossary) == ABORT) goto END;
	#endif /* BP_CARBON_GUI_FORGET_THIS */

	if(!CompiledAl  || (!CompiledGr && (AddBolsInGrammar() > BolsInGrammar))) {
		CompiledAl = FALSE;
		if(CompileAlphabet() != OK) goto END;
		}

	if((r=UpdateGlossary()) != OK) goto END;
	if(CompileRegressions() != OK) goto END;

	r = MISSED;
	if(ResetControllers) {
		for(ch=0; ch < MAXCHAN; ch++) {
			(*p_Oldvalue)[ch].volume = -1;
			(*p_Oldvalue)[ch].panoramic = -1;
			(*p_Oldvalue)[ch].pressure = -1;
			(*p_Oldvalue)[ch].pitchbend = -1;
			(*p_Oldvalue)[ch].modulation = -1;
			}
	//	if(ResetControllers) ResetMIDIControllers(NO,NO,YES);
		}
	if(InitThere == 1) FirstTime = TRUE;
	if(InitThere == 2) {
		if(!ScriptExecOn) {
			CurrentDir = WindowParID[wScript];
			CurrentVref = TheVRefNum[wScript];
			}
		check = 0; // This will create a script line
		r = ExecScriptLine(NULL,wScript,check,TRUE,p_InitScriptLine,x,&x,&i,&i);
		if(r != OK) goto END;
		}
	firstorigin = origin; p_a = NULL;
	askedvariables = derivevariables = FALSE;

	end = GetTextHandleLength(TEH[w]);

	end = origin;
	while(TRUE) {
		c = GetTextChar(w,end);
		if(c == '\0') break;
		end++;
		}

	// BPPrintMessage(odInfo,"@ origin = %ld next_origin = %ld end = %ld\n",(long)origin,(long)next_origin,(long)end);
	LastChunk = FALSE;
	while((originmem=origin) < end) {
		if((r=stop(0,"PlaySelection")) != OK) return ABORT;
		next_origin = origin;
		time(&ProductionStartTime);
		while(TRUE) {
			c = GetTextChar(w,next_origin);
			if(c == '\r' || c == '\n' || c == '\0') break;
			next_origin++;
			if(next_origin > end) {
				BPPrintMessage(odError,"=> Error searching for linefeed\n");
				SelectOn = FALSE; return(MISSED);
				}
			}
		if(next_origin == origin) break;
		if((next_origin + 1) == end) LastChunk = TRUE;
		r = OK;
		SetSelect(origin,next_origin,TEH[w]);
	//	BPPrintMessage(odInfo,"Playing selection\n");
		Nplay = 1;
		SaidTooComplex = ShownBufferSize = FALSE;
	//	BPPrintMessage(odInfo,"Playing selection %ld to %ld (up to %ld)\n",(long)origin,(long)next_origin,(long)end);
		if((r=SelectionToBuffer(FALSE,FALSE,w,&p_a,&origin,PROD)) != OK) {
			MyDisposeHandle((Handle*)&p_a);
			/* Could already be NULL because of PolyExpand() */
			if(ScriptExecOn) r = OK;
			if(stop(0,"PlaySelection") != OK) r = ABORT;
			goto END;
			}
		if(!NoVariable(&p_a)) {
		/*	if(!askedvariables) {
				askedvariables = TRUE;
				r = Answer("Selection contains variables. Use grammar to derive them",'Y');
				if(r == ABORT) {
					r = OK;
					MyDisposeHandle((Handle*)&p_a);
					goto END;
					}
				derivevariables = r;
				} */
			derivevariables = OK;
			if(!derivevariables) {
				r = OK;
				goto NOVARIABLE;
				}
			MyDisposeHandle((Handle*)&p_a);
		//	SetSelect(originmem,next_origin,TEH[w]);
		//	Sel1 = origin; Sel2 = firstorigin;
			if((r=CompileCheck()) != OK) goto END;
			// Selection could be changed while compiling if w = wGrammar, but this no longer happens
		//	origin = Sel1; firstorigin = Sel2;
		//	TextGetSelection(&originmem,&next_origin,TEH[w]);
			Ctrlinit();
			if(!AllowRandomize) ResetRandom();
			else {
				asked = TRUE;
				if(UsedRandom && !ScriptExecOn && !AllItems && !AEventOn && !asked) {
					if((r=Answer("Reset random sequence",'N')) == OK) ResetRandom();
					UsedRandom = FALSE;
					asked = TRUE;
					}
				}
			if((r=ProduceItems(w,FALSE,FALSE,NULL)) != OK) goto END;
			}
		else {
	NOVARIABLE:
	//		PleaseWait();
			if(r == OK) {
				r = PlayBuffer(&p_a,NO);	/* HERE WE DO IT */
				}
		//	MyDisposeHandle((Handle*)&p_a);
			/* Could already be NULL because of PolyExpand() */
			if(r == ABORT || r == EXIT) {
				if(Panic) return ABORT;
			//	if(CyclicPlay) ResetMIDI(!Oms && !NEWTIMER_FORGET_THIS);
				break;
				}
			}
		// ResetMIDI(TRUE);
		origin = next_origin + 1;
		// if(ReleasePhaseDiagram(nmax,&p_imaxseq) != OK) return(ABORT); // Added by BB 2021-03-21
		}

	END:
	if(r == OK) SetSelect(firstorigin,end,TEH[w]);
	// if(PlaySelectionOn > 0) PlaySelectionOn--; */ // Fixed by BB 2021-02-17

	ResetMIDIfile();

	if(improvize) {
		Improvize = TRUE;
		
		}
	
	if(r == OK) {
		BPActivateWindow(SLOW,LastComputeWindow);
		// ResetMIDI(TRUE);
		// if(ResetControllers) ResetMIDIControllers(YES,NO,NO);
		}
	return(r);
	}


int PlayBuffer(tokenbyte ***pp_buff,int onlypianoroll) {
	int r;

	if(Panic || CheckEmergency() != OK) return(ABORT);
	if((r=stop(1,"PlayBuffer")) != OK) return(r);
	if(Jbol < 3) NoAlphabet = TRUE;
	else NoAlphabet = FALSE;

	// BPPrintMessage(odInfo,"Running PlayBuffer()\n");
	if(FirstTime && !onlypianoroll) {
		if(p_Initbuff == NULL) {
		//	if(Beta) Alert1("=> Err. PlayBuffer(). p_Initbuff = NULL. ");
			BPPrintMessage(odError,"=> Err. PlayBuffer(). p_Initbuff = NULL\n");
			return(ABORT);
			}
		if((r=PlayBuffer1(&p_Initbuff,NO)) != OK) {
			BPPrintMessage(odError,"=> PlayBuffer1() cancelled\n");
			return(r);
			}
		WaitABit(1000L);	/* This is necessary notably if sending a program change */
		FirstTime = FALSE;
		}
	if(Maxitems > ZERO /* && !ShowGraphic && !DisplayItems */) {
		sprintf(Message,"Item #%ld",(long)ItemNumber+1L);
		ShowMessage(TRUE,wMessage,Message);
		}
	// if(!Improvize) initTime = getClockTime();
	r = PlayBuffer1(pp_buff,onlypianoroll);

	// if(!PlaySelectionOn && ++ItemNumber > INT_MAX) ItemNumber = 1L;
	if(!PlaySelectionOn && ItemNumber > INT_MAX) ItemNumber = 1L;
	if(r != EXIT && Maxitems > ZERO && ItemNumber >= Maxitems && !onlypianoroll) {
		/* Script ordered to terminate production. */
		SoundOn = TRUE;
		r = WaitForEmptyBuffer();
		SoundOn = FALSE;
		if(r != OK) return(r);
		HideWindow(Window[wMessage]);
		BPPrintMessage(odInfo,"Aborted PlayBuffer()\n");
		return(ABORT);
		}
	// BPPrintMessage(odInfo,"End of PlayBuffer()\n");
	return(r);
	}


int PlayBuffer1(tokenbyte ***pp_buff,int onlypianoroll) {
	int result,i,j,nmax,dummy,finish,repeat,displayProducemem,
		showmessagesmem,usebufferlimitmem,again,store,a,b;
	clock_t time_end_compute;
	long tmin,tmax,length,kmax;
	unsigned long maxseq;
	double maxseqapprox;
	unsigned long **p_imaxseq;
	tokenbyte **p_b;

	length = LengthOf(pp_buff);
	if(length < 1) return(OK); 
	finish = FALSE; dummy = 0;
	CurrentChannel = 1;

	if(!NoVariable(pp_buff) && UpdateGlossary() != OK) return(ABORT);

	/* We need to store the item in its current format to be able to derive it further */
	p_b = NULL;
	store = FALSE;
	if(!Improvize && !PlaySelectionOn && !onlypianoroll) store = TRUE;
	if(store) {
		if((p_b=(tokenbyte**) GiveSpace((Size) MyGetHandleSize((Handle)*pp_buff))) == NULL)
			return(ABORT);
		if(CopyBuf(pp_buff,&p_b) == ABORT) return(ABORT);
		}
		
	//// Using glossary
	if(GlossGram.p_subgram != NULL && NeedGlossary(pp_buff) && !onlypianoroll) {
		displayProducemem = DisplayProduce;
		showmessagesmem = ShowMessages;
		usebufferlimitmem = UseBufferLimit;
		MaxDeriv = MAXDERIV;
		if((result=MakeComputeSpace(MaxDeriv)) != OK) goto OUT;
		DisplayProduce = ShowMessages = UseBufferLimit = finish = repeat = FALSE;
		time_end_compute = clock() + (MaxConsoleTime * CLOCKS_PER_SEC);
		
		//////  Make derivation (unique substitution) with glossary ///////
		result = ComputeInGram(pp_buff,&GlossGram,1,0,&length,&finish,&repeat,PROD,FALSE,
			&dummy,&dummy,time_end_compute);
		/////
		DisplayProduce = displayProducemem;
		ShowMessages = showmessagesmem;
		UseBufferLimit = usebufferlimitmem;
		if(result != OK) goto OUT;
		}

	result = OK;
	ShowMessages = TRUE; // 2024-05-23
	while((result=PolyMake(pp_buff,&maxseqapprox,YES)) == AGAIN){};

	/* if(trace_scale) {
		i = 0;
		while(TRUE) {
			a = (**pp_buff)[i]; b = (**pp_buff)[i+1];
			if(a == TEND && b == TEND) break;
			BPPrintMessage(odInfo,"PlayBuffer1() %d %d\n",a,b);
			i += 2;
			}
		} */

	if(result == EMPTY) {
		result = OK; goto OUT;
		}
	if(result != OK) goto OUT;

	if((result=MakeEventSpace(&p_imaxseq)) != OK) goto OUT;

	again = FALSE;
		
	SETTIME:
	if((result=CheckLoadedPrototypes()) != OK) {
		BPPrintMessage(odInfo, "No sound-object prototypes loaded\n");
		goto RELEASE;
		}
	#if BP_CARBON_GUI_FORGET_THIS
	if((result=LoadInteraction(TRUE,FALSE)) != OK) goto RELEASE;
	#endif /* BP_CARBON_GUI_FORGET_THIS */
	SetTimeOn = TRUE; nmax = 0;
	if((result = TimeSet(pp_buff,&kmax,&tmin,&tmax,&maxseq,&nmax,p_imaxseq,maxseqapprox))
								== MISSED || result == ABORT || result == EXIT) {
		SetTimeOn = FALSE;
		if(result != ABORT && ReleasePhaseDiagram(nmax,&p_imaxseq) != OK) result = ABORT;
		if(result == MISSED) ShowError(37,0,0);
	//	if((result == ABORT && !SkipFlag) || result == EXIT) goto RELEASE;
		if(Panic) return ABORT;
		if(result == ABORT || result == EXIT) goto RELEASE; // Fixed by BB 2021-02-26
		result = MISSED;
		goto RELEASE;
		}
	if(result == AGAIN) again = TRUE;
	result = OK;
	SetTimeOn = FALSE;
	// BPPrintMessage(odInfo,"\ntmin = %ld, tmax = %ld\n\n",(long)tmin,(long)tmax);

	// if(ShowGraphic) BPPrintMessage(odInfo, "Shall we draw graphics?\n");

//	if(!Improvize) initTime = getClockTime();
	if(onlypianoroll
			|| (ShowGraphic && p_Initbuff != (*pp_buff) && POLYconvert && (tmax > tmin || Nature_of_time == SMOOTH))) {
		if(!ShowPianoRoll && !onlypianoroll) {
			result = DrawItem(wGraphic,p_Instance,NULL,NULL,kmax,tmin,tmax,maxseq,0,nmax,p_imaxseq,TRUE,TRUE,NULL);
			if(OutCsound || WriteMIDIfile || rtMIDI) result = MakeSound(&kmax,maxseq,nmax+1,&p_b,tmin,tmax,NO,NULL);
			}
		else {
			result = MakeSound(&kmax,maxseq,nmax+1,&p_b,tmin,tmax,NO,NULL);
			result = DrawItem(wGraphic,p_Instance,NULL,NULL,kmax,tmin,tmax,maxseq,0,nmax,p_imaxseq,TRUE,TRUE,NULL);
			}
		}
	else if(OutCsound || WriteMIDIfile || rtMIDI) result = MakeSound(&kmax,maxseq,nmax+1,&p_b,tmin,tmax,NO,NULL);

	/*if(result == AGAIN) {    // NOT USED AT THE MOMENT
		again = TRUE;
		result = OK;
		}
	if((onlypianoroll || ShowPianoRoll) && !OutCsound) goto RELEASE;

	PLAYIT:
	if(result == OK) result = MakeSound(&kmax,maxseq,nmax+1,&p_b,tmin,tmax,YES,NULL);
	if(result == AGAIN) again = TRUE;
	if(again || EventState == AGAIN) {
		again = FALSE; result = OK;
		goto PLAYIT;
		} */

	RELEASE:
	// BPPrintMessage(odError,"@@ End MakeSound() nmax = %ld\n",(long)nmax);
	if(result == ABORT) return(result);
	if(ReleasePhaseDiagram(nmax,&p_imaxseq) != OK) return(ABORT);
	if(result == MISSED) BPPrintMessage(odInfo,"Item ignored\n");

	OUT:
	if(store) {
		/* Restore original item */
		if(CopyBuf(&p_b,pp_buff) == ABORT) return(ABORT);
		MyDisposeHandle((Handle*)&p_b);
		}
	// BPPrintMessage(odInfo,"@@ End PlayBuffer1()\n");
	return(result);
	}


int PlayHandle(char** p_line,int onlypianoroll)
{
tokenbyte **p_ti;
char c,*p1,*p2;
int i,i0,im,ch,r,improvize,meta=0,asked,askedvariables,derivevariables;
p_context *p_plx,*p_prx,plx,prx;
long x,tr;

if(p_line == NULL) {
	if(Beta) Alert1("=> Err. PlayHandle(). p_line == NULL");
	return(OK);
	}
if((*p_line)[0] == '\0') return(OK);

#if BP_CARBON_GUI_FORGET_THIS
if(GetTuning() != OK) return(ABORT);
#endif /* BP_CARBON_GUI_FORGET_THIS */
if(!rtMIDI && !OutCsound && !onlypianoroll) {
	Alert1("Both MIDI and Csound outputs are inactive. Item can't be played");
	BPActivateWindow(SLOW,wSettingsBottom);
	return(MISSED);
	}

r = MISSED;
asked = FALSE;
#if BP_CARBON_GUI_FORGET_THIS
if(SaveCheck(wAlphabet) == ABORT) return(r);
if(SaveCheck(wInteraction) == ABORT) return(r);
if(SaveCheck(wGlossary) == ABORT) return(r);
if(SaveCheck(wGrammar) == ABORT) return(r);
#endif /* BP_CARBON_GUI_FORGET_THIS */

if(!CompiledAl || (!CompiledGr && (AddBolsInGrammar() > BolsInGrammar))) {
	CompiledAl = FALSE;
	if(CompileAlphabet() != OK) return(r);
	}
if(!onlypianoroll) {
	if(CompileRegressions() != OK) return(r);
	// ResetMIDI(TRUE);  Fixed by BB 2022-02-18
	if(1 || ResetControllers) {
		for(ch=0; ch < MAXCHAN; ch++) {
			(*p_Oldvalue)[ch].volume = -1;
			(*p_Oldvalue)[ch].panoramic = -1;
			(*p_Oldvalue)[ch].pressure = -1;
			(*p_Oldvalue)[ch].pitchbend = -1;
			(*p_Oldvalue)[ch].modulation = -1;
			}
		// if(ResetControllers) ResetMIDIControllers(NO,NO,YES);
		}
	}

// if(WillRandomize) ReseedOrShuffle(RANDOMIZE);
	
r = MISSED; p_ti = NULL;
i = 0; im = MyHandleLen(p_line);
askedvariables = derivevariables = FALSE;

while(TRUE) {
	SaidTooComplex = ShownBufferSize = FALSE;
	if((r=stop(0,"PlayHandle")) != OK) return ABORT;
	while(i < im && (isspace(c=(*p_line)[i]) || c == '\0')) i++;
	i0 = i;
	if(i >= im) break;
ENCODE:
	if(c == '/' && i < (im-1) && (*p_line)[i+1] == '/') {	/* Skip C-like remarks */
		while(i < im && (*p_line)[i] != '\r' && (*p_line)[i] != '\n' && (*p_line)[i] != '\0')
			i++;
		continue;
		}
	MyLock(FALSE,(Handle)p_line);
	p1 = &((*p_line)[i]);
	while(i < im && (*p_line)[i] != '\r' && (*p_line)[i] != '\n' && (*p_line)[i] != '\0')
		i++;
	p2 = &((*p_line)[i-1]);
	i++;
	p_plx = &plx; p_prx = &prx;
	*p_plx = *p_prx = NULL;
	p_ti = Encode(FALSE,TRUE,0,0,&p1,&p2,p_plx,p_prx,&meta,0,NULL,FALSE,&r);
	MyUnlock((Handle)p_line);
	if(p_ti == NULL) {
		if(r != OK) {
			MyDisposeHandle((Handle*)&p_ti);
			Panic = TRUE;
			return(r);
			}
		continue;
		}
	else {
	//	PlaySelectionOn++; // Fixed by BB 2021-02-17
	//	if(!onlypianoroll) ResetMIDI(TRUE); Fixed by BB 2022-02-18
		improvize = FALSE;
		if(Improvize) {
			improvize = TRUE;
			Improvize = FALSE;
			
			}
		if(NoVariable(&p_ti)) {
NOVARIABLE:
			r = PlayBuffer(&p_ti,onlypianoroll);
			}
		else {
			if(!askedvariables) {
				r = Answer("Selection contains variables. Use grammar to derive them",'Y');
				askedvariables = TRUE;
				if(r == ABORT) {
					r = OK;
					MyDisposeHandle((Handle*)&p_ti);
					break;
					}
				derivevariables = r;
				}
			if(!derivevariables) goto NOVARIABLE;
			if(!CompiledGr || !CompiledGl) {
				MyDisposeHandle((Handle*)&p_ti);
			//	if(PlaySelectionOn > 0) PlaySelectionOn--; // Fixed by BB 2021-02-17
				if(improvize) {
					Improvize = TRUE;
					
					}
				if((r=CompileCheck()) != OK) break;
				i = i0;
				goto ENCODE;
				}
			if(!AllowRandomize) ResetRandom();
			else {
				if(UsedRandom && !ScriptExecOn && !AllItems && !AEventOn && !asked) {
					if((r=Answer("Reset random sequence",'N')) == OK) ResetRandom();
					UsedRandom = FALSE;
					asked = TRUE;
					}
				}
			r = ProduceItems(0,FALSE,FALSE,&p_ti);
			}
	//	if(PlaySelectionOn > 0) PlaySelectionOn--; // Fixed by BB 2021-02-17
		
		if(!onlypianoroll) ResetMIDIfile();
		
		if(improvize) {
			Improvize = TRUE;
			
			}
		}
	MyDisposeHandle((Handle*)&p_ti);
	if(r == ABORT || r == EXIT) break;
	}
return(r);
}


int TextToMIDIstream(int w) {
	int i,r,improvize,t,showmessages;
	MIDIcode **ptr1;
	tokenbyte **p_a;
	long origin,end,tr,x;

	if(CheckEmergency() != OK) return(ABORT);

	if(w < 0 || w >= WMAX || !Editable[w]) {
		if(Beta)
			Alert1("=> Err. TextToMIDIstream(). Incorrect window index");
		return(MISSED);
		}
	if(!StrikeAgainDefault) {
	#if !BP_CARBON_GUI_FORGET_THIS
		r = Alert1("The strike mode setting is \"Don't strike again NoteOn's\", which is unusual.");
	#else
		r = Answer("The strike mode setting is 'Don't strike again NoteOn's', which is unusual. Change it?",
			'N');
		if(r == ABORT) return(r);
		if(r == YES) {
			ShowWindow(GetDialogWindow(StrikeModePtr));
			SelectWindow(GetDialogWindow(StrikeModePtr));
			SetDefaultStrikeMode();
			BPUpdateDialog(StrikeModePtr);
			return(ABORT);
			}
	#endif /* BP_CARBON_GUI_FORGET_THIS */
		}
	TextGetSelection(&origin, &end, TEH[w]);
	if(end <= origin) {
		Alert1("Selection is empty");
		BPActivateWindow(SLOW,w);
		return(MISSED);
		}
	showmessages = ShowMessages;
	ShowMessages = FALSE;
	improvize = Improvize;
	Improvize = FALSE;
	

	// PlaySelectionOn++; // Fixed by BB 2021-02-17
	/* ResetMIDI(TRUE); */

	r = ABORT; p_a = NULL;
	#if BP_CARBON_GUI_FORGET_THIS
	if(SaveCheck(wAlphabet) == ABORT) goto END;
	if(SaveCheck(wGrammar) == ABORT) goto END;
	if(SaveCheck(wInteraction) == ABORT) goto END;
	if(SaveCheck(wGlossary) == ABORT) goto END;
	#endif /* BP_CARBON_GUI_FORGET_THIS */
	if(!CompiledAl  || (!CompiledGr && (AddBolsInGrammar() > BolsInGrammar))) {
		CompiledAl = FALSE;
		if(CompileAlphabet() != OK) goto END;
		}
	r = MISSED;
	// FIXME ? Why is this done a second time? - akozar 20130830
	if(!CompiledAl  || (!CompiledGr && (AddBolsInGrammar() > BolsInGrammar))) {
		CompiledAl = FALSE;
		if(CompileAlphabet() != OK) goto END;
		}
	r = OK;
	SetSelect(origin,end,TEH[w]);
	ShowMessage(TRUE,wMessage,"Converting selected text to sound-object prototype...");
	Nplay = 1;
	if((r=SelectionToBuffer(FALSE,FALSE,w,&p_a,&origin,PROD)) != OK) {
		MyDisposeHandle((Handle*)&p_a);
		if(ScriptExecOn) r = OK;
		goto END;
		}
	if(!NoVariable(&p_a)) {
		Alert1("You can't convert the selection because it contains text that BP2 interprets as variables");
		MyDisposeHandle((Handle*)&p_a);
		goto END;
		}
	else {
		PleaseWait();
		if(Stream.code == NULL) {
			if((ptr1 = (MIDIcode**) GiveSpace((Size) 200L * sizeof(MIDIcode))) == NULL) {
				r = ABORT; goto END;
				}
			Stream.code = ptr1;
			}
		ItemCapture = TRUE;
		Stream.i = Stream.imax = ZERO;
		Stream.period = ZERO;
		Stream.cyclic = FALSE;
		if(r == OK) r = PlayBuffer(&p_a,NO);
		MyDisposeHandle((Handle*)&p_a);
		if(Stream.i <= ZERO) Alert1("No events were captured...");
		else Stream.imax = Stream.i;
		Stream.pclock = (long) Pclock;
		Stream.qclock = (long) Qclock;
		ItemCapture = FALSE;
		}
	// if(ResetControllers) ResetMIDIControllers(NO,YES,YES);

	END:
	if(PlaySelectionOn) {
		PlaySelectionOn = FALSE; // Fixed by BB 2021-02-17
		BPPrintMessage(odInfo,"End of playing selection\n");
		}
	ShowMessages = showmessages;
	Improvize = improvize;
	
	HideWindow(Window[wMessage]);
	return(r);
	}


/* 20130819: Allowed 'what' (the paste action) to be passed as a parameter.
   Valid values are listed in -BP2.h as "PasteSelectionAlert button indexes".
   A new value (bAskPasteAction) indicates to ask the user for how to paste.
 */
int PasteStreamToPrototype(int j, int what)
{
long maxsize,newsize,i,ifrom,ito,k,p,offset;
Size n;
MIDIcode **ptr1;
int longerCsound;
Milliseconds gap,tfrom,tto,datestream,dateproto;
double alpha;

if(j < 2 || j >= Jbol) return(MISSED);
if(Stream.imax <= ZERO) return(OK);

if((*pp_MIDIcode)[j] == NULL) {
	if((ptr1 = (MIDIcode**) GiveSpace((Size) 50L * sizeof(MIDIcode))) == NULL) return(ABORT);
	(*pp_MIDIcode)[j] = ptr1;
	}
maxsize = (long) MyGetHandleSize((Handle)(*pp_MIDIcode)[j]);
maxsize = (maxsize / sizeof(MIDIcode));

if(DurationToPoint(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

ifrom = ito = -1L;;
tfrom = ZERO;
if((*p_Tpict)[j] != Infneg) {
	if((*p_Tpict)[j] >= ZERO) tfrom = (*p_Tpict)[j];
	else tfrom = ZERO;
	tto = tfrom + (*Stream.code)[Stream.imax - 1L].time;
	for(i=ZERO; i < (*p_MIDIsize)[j]; i++) {
		if(ifrom == -1L && (*((*pp_MIDIcode)[j]))[i].time >= tfrom) {
			ifrom = FindGoodInsertPoint(j,i);
			}
		if((*((*pp_MIDIcode)[j]))[i].time >= tto) {
			ito = FindGoodInsertPoint(j,i);
			break;
			}
		}
	if(ifrom == -1L) ifrom = ito = (*p_MIDIsize)[j];
	else if(ito == -1L) ito = (*p_MIDIsize)[j];
	}
StopWait();

if((*p_MIDIsize)[j] == ZERO) what = bDeleteReplace;
#if BP_CARBON_GUI_FORGET_THIS
else if (what == bAskPasteAction) what = Alert(PasteSelectionAlert,0L);
#endif /* BP_CARBON_GUI_FORGET_THIS */
switch(what) {
	case bCancelPasteSelection:
		if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
		goto OUT;
		break;
	case bInsertBefore:
		ifrom = ito = tfrom = ZERO;
		newsize = Stream.imax + (*p_MIDIsize)[j];
		(*p_PasteDone)[j] = TRUE;
		break;
	case bAppend:
		newsize = Stream.imax + (*p_MIDIsize)[j];
		ifrom = ito = (*p_MIDIsize)[j];
		tfrom = (*((*pp_MIDIcode)[j]))[(*p_MIDIsize)[j] - 1L].time;
		(*p_PasteDone)[j] = TRUE;
		break;
	case bInsertAtInsertPoint:
		if((*p_Tpict)[j] == Infneg) {
			Alert1("Can't insert because insert point is not defined or out of range");
			if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
			goto OUT;
			}
		ito = ifrom;
		newsize = Stream.imax + (*p_MIDIsize)[j];
		(*p_PasteDone)[j] = TRUE;
		break;
	case bReplaceFromInsertpoint:
		if((*p_Tpict)[j] == Infneg) {
			Alert1("Can't replace because insert point is not defined or out of range");
			if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
			goto OUT;
			}
		if(Answer("Part of the sound-object prototype will be replaced with the selection. Can't be undone. Proceed",
				'N') != OK) {
			if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
			goto OUT;
			}
		newsize = (*p_MIDIsize)[j] - (ito - ifrom) + Stream.imax;
		(*p_PasteDone)[j] = FALSE;
		break;
	case bDeleteReplace:
		if((*p_MIDIsize)[j] > ZERO
			&& Answer("The sound-object prototype will be replaced with the selection. Can't be undone. Proceed",
				'N') != OK) {
			if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
			goto OUT;
			}
		newsize = Stream.imax;
		ifrom = ito = tfrom = ZERO;
		(*p_PasteDone)[j] = FALSE;
		break;
	case bMergeFromInsertPoint:
		if((*p_Tpict)[j] == Infneg) {
			Alert1("Can't merge because insert point is not defined or out of range");
			if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
			goto OUT;
			}
		newsize = Stream.imax + (*p_MIDIsize)[j];
		(*p_PasteDone)[j] = FALSE;
		break;
	case bHelpPasteSelection:
#if BP_CARBON_GUI_FORGET_THIS
		DisplayHelp("Paste text selection to sound-object prototype");
#endif /* BP_CARBON_GUI_FORGET_THIS */
		if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
		goto OUT;
		break;
	default:
		sprintf(Message, "=> Err. PasteStreamToPrototype(): Invalid value for parameter 'what' (%d).", what);
		if(Beta) Alert1(Message);
		if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
		goto OUT;
		break;
	}
(*p_Ifrom)[j] = ifrom;
if(ifrom < ZERO || ito < ZERO) {
	if(Beta) Alert1("=> Err. in PasteStreamToPrototype(). ifrom < ZERO || ito < ZERO");
	ifrom = ZERO;
	}

// Resize handles if necessary
while((newsize+1L) >= maxsize) {	/* +1L for the case Stream.period > 0 */
	ptr1 = (*pp_MIDIcode)[j];
	if((ptr1 = (MIDIcode**)IncreaseSpace((Handle)ptr1)) == NULL) goto ERR;
	(*pp_MIDIcode)[j] = ptr1;
	maxsize = (long) MyGetHandleSize((Handle)(*pp_MIDIcode)[j]);
	maxsize = (maxsize / sizeof(MIDIcode));
	}

// Move up codes before inserting
switch(what) {
	case bInsertAtInsertPoint:
	case bInsertBefore:
	case bReplaceFromInsertpoint:
	case bMergeFromInsertPoint:
		offset = Stream.imax - (ito - ifrom);
		if(what == bMergeFromInsertPoint) offset = Stream.imax;
		for(i=(*p_MIDIsize)[j]-1L; i >= ito; i--) {
			(*((*pp_MIDIcode)[j]))[i+offset] = (*((*pp_MIDIcode)[j]))[i];
			}
		break;
	}
	
// Calculate dilation ratio
if(Stream.pclock > ZERO) {
	if((*p_Tref)[j] > EPSILON) {
		alpha = (((double)Stream.pclock * 1000.) / Stream.qclock) / (*p_Tref)[j];
		}
	else {
		alpha = 1.;
		(*p_Tref)[j] = ((double)Stream.pclock * 1000.) / Stream.qclock;
		}
	}
else alpha = 1.;

// Merge codes
if(what == bMergeFromInsertPoint) {
	/* Create a handle to a stream containing the sorted codes */
	n = Stream.imax + (ito - ifrom);
	if((ptr1 = (MIDIcode**) GiveSpace(n * sizeof(MIDIcode))) == NULL) goto ERR;
	for(i=p=ZERO,k=ifrom;;) {
		if(p >= n) break;
		if(i < Stream.imax) datestream = (*Stream.code)[i].time / alpha + tfrom;
		else datestream = Infpos;
		if(k < ito) dateproto = (*((*pp_MIDIcode)[j]))[k].time;
		else dateproto = Infpos;
		if(datestream > dateproto) {
			(*ptr1)[p].time = dateproto;
			(*ptr1)[p].byte = (*((*pp_MIDIcode)[j]))[k].byte;
			(*ptr1)[p].sequence = (*((*pp_MIDIcode)[j]))[k].sequence;
			k++;
			}
		else {
			(*ptr1)[p].time = datestream;
			(*ptr1)[p].byte = (*Stream.code)[i].byte;
			(*ptr1)[p].sequence = (*Stream.code)[i].sequence;
			i++;
			}
		p++;
		}
	/* Copy the sorted stream to the prototype */
	for(p=ZERO; p < n; p++) {
		(*((*pp_MIDIcode)[j]))[ifrom+p] = (*ptr1)[p];
		}
	MyDisposeHandle((Handle*)&ptr1);
	}
	
// Copy codes
if(what != bMergeFromInsertPoint) {
	for(i=ZERO; i < Stream.imax; i++) {
		(*((*pp_MIDIcode)[j]))[ifrom+i].time = (*Stream.code)[i].time / alpha + tfrom;
		(*((*pp_MIDIcode)[j]))[ifrom+i].byte = (*Stream.code)[i].byte;
		(*((*pp_MIDIcode)[j]))[ifrom+i].sequence = (*Stream.code)[i].sequence;
		}
	}

// Increment dates after insertion;
gap = (*Stream.code)[Stream.imax-1L].time / alpha;
if(what == bInsertAtInsertPoint || what == bInsertBefore) {
	for(i=(ifrom+Stream.imax); i < newsize; i++) {
		(*((*pp_MIDIcode)[j]))[i].time += gap;
		}
	}

(*p_Dur)[j] = ((*((*pp_MIDIcode)[j]))[newsize-1].time);
/* This value will be corrected by SetPrototypeDuration() */

(*p_MIDIsize)[j] = newsize;

if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
	
(*p_Quan)[j] = 0.;
SetPrototypeDuration(j,&longerCsound);
if(Stream.cyclic && what == bDeleteReplace) {
	(*p_PeriodMode)[j] = RELATIF;
	(*p_BeforePeriod)[j] = ZERO;
#if BP_CARBON_GUI_FORGET_THIS
	SetPrototypePage6(j);
#endif /* BP_CARBON_GUI_FORGET_THIS */
	}

#if BP_CARBON_GUI_FORGET_THIS
SetPrototypePage5(j);
SetPrototype(j);

UpdateDirty(TRUE,wPrototype1);
#endif /* BP_CARBON_GUI_FORGET_THIS */

OUT:
return(OK);

ERR:
PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j);
return(ABORT);
}


int UndoPasteSelection(int j)
{
long i;
int longerCsound;

for(i=((*p_Ifrom)[j]+Stream.imax); i < (*p_MIDIsize)[j]; i++) {
	(*((*pp_MIDIcode)[j]))[i-Stream.imax] = (*((*pp_MIDIcode)[j]))[i];
	}
(*p_MIDIsize)[j] -= Stream.imax;
SetPrototypeDuration(j,&longerCsound);
#if BP_CARBON_GUI_FORGET_THIS
SetPrototypePage5(j);
SetPrototype(j);
ChangedProtoType(j);
UpdateDirty(TRUE,wPrototype1);
#endif /* BP_CARBON_GUI_FORGET_THIS */
return(OK);
}


int ChangedProtoType(int j)
{
(*p_PasteDone)[j] = FALSE;
return(OK);
}


long FindGoodInsertPoint(int j,long i)
{
long k,oldk;
int c,status;
Milliseconds date;

if(j < 2 || j >= Jbol) return(i);
date = (*((*pp_MIDIcode)[j]))[i].time;	/* We're in "point" representation */
for(k=oldk=i; k < (*p_MIDIsize)[j]; k++) {
	c = (*((*pp_MIDIcode)[j]))[k].byte;
	if((*((*pp_MIDIcode)[j]))[k].time != date) {
		k = oldk;
		break;
		}
	status = c - (c % 16);
	if(status == NoteOff || (status == NoteOn && (*((*pp_MIDIcode)[j]))[k+2].byte == 0)) {
		oldk = k;
		k += 2;
		continue;
		}
	break;
	}
return(k);
}


int CaptureCodes(int j)
{
long i,maxcodes;

if(Answer("Capture the MIDI stream of this sound-object prototype to paste it later to another prototype",'Y')
	!= OK) return(OK);
if(Stream.imax > ZERO) {
	if(Answer("This will delete currently captured codes. Proceed anyway",'N') != OK) return(OK);
	}
PleaseWait();
maxcodes = (*p_MIDIsize)[j];
if(Stream.code == NULL) {
	if((Stream.code = (MIDIcode**) GiveSpace((Size)maxcodes * sizeof(MIDIcode))) == NULL)
		return(ABORT);
	}
else {
	MySetHandleSize((Handle*)&(Stream.code),(Size)maxcodes * sizeof(MIDIcode));
	}
Stream.imax = maxcodes;
Stream.pclock = (*p_Tref)[j];
Stream.qclock = 1000L;
Stream.period = ZERO;
Stream.cyclic = FALSE;

if(DurationToPoint(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);

for(i=0; i < maxcodes; i++) {
	(*Stream.code)[i] = (*((*pp_MIDIcode)[j]))[i];
	}
if(PointToDuration(pp_MIDIcode,NULL,p_MIDIsize,j) != OK) return(ABORT);
return(OK);
}


int ExpandSelection(int w) {
	int r,wout,finish,ifunc,dirtymem,hastabs;
	tokenbyte **p_a;
	long origin,end,neworigin,newend,dummy,length;
	double maxseq;
	
	if(CheckEmergency() != OK) return(ABORT);
	if(!CompiledAl  || (!CompiledGr && (AddBolsInGrammar() > BolsInGrammar))) {
		CompiledAl = FALSE;
		if(CompileAlphabet() != OK) return(MISSED);
		}
	p_a = NULL;
	wout = OutputWindow;
	ExpandOn = OkShowExpand = TRUE;
	
	origin = 0L;
	end = GetTextHandleLength(TEH[w]);

	SetSelect(origin,end,TEH[w]);
	
	if((r = SelectionToBuffer(FALSE,FALSE,w,&p_a,&origin,PROD)) != OK) {
		MyDisposeHandle((Handle*)&p_a);
		/* Could be NULL because of PolyExpand() */
		goto OUT;
		}

	while((r=PolyMake(&p_a,&maxseq,NO)) == AGAIN);
	
	if(r == ABORT || r == EXIT) goto OUT;
	if(r == OK) {
	//	SetSelect(newend,newend,TEH[wout]);
	//	if(newitem) Print(wout,"\n");
	//	else TextGetSelection(&dummy, &neworigin, TEH[wout]);
		Print(wout,"\n");
		sprintf(Message,"Ratio = %lu, Prod = %lu, Kpress = %lu\n\n",(unsigned long)Ratio,(unsigned long)Prod,(unsigned long)Kpress);
		Print(wout,Message);
		if(Prod == Ratio) r = PrintArg(FALSE,FALSE,TRUE,FALSE,FALSE,FALSE,stdout,wout,pp_Scrap,&p_a);
		else {
			sprintf(Message,"[Rescaled, dilation ratio = %.0f] ",(Prod/Ratio));
			Print(wout,Message);
			Print(wout,"\n");
			r = PrintArg(FALSE,FALSE,TRUE,FALSE,FALSE,FALSE,stdout,wout,pp_Scrap,&p_a);
			}
		if(OkShowExpand) {
			Print(wout,"\n");
			r = PrintArg(DisplayMode(&p_a,&ifunc,&hastabs),FALSE,FALSE,TRUE,FALSE,FALSE,stdout,wout,pp_Scrap,&p_a);
			}
		else Print(wout,"\nThis expression is too complex for being displayed comprehensively!\n");
		TextGetSelection(&dummy, &newend, TEH[wout]);
		BPActivateWindow(SLOW,wout);
		}
	MyDisposeHandle((Handle*)&p_a);
	/* Could be NULL because of PolyExpand() */
/*	if(r == EXIT || r == STOP || r == ABORT) goto OUT;
	newitem = TRUE;
	}
	SetSelect(neworigin,newend,TEH[wout]);
	ShowSelect(CENTRE,wout); */
	
	OUT:
	ExpandOn = FALSE;
	return(r);
	}


int ShowPeriods(int w)
{
int r,finish,ifunc,hastabs;
tokenbyte **p_a;
long origin,end,oldorigin,neworigin,oldend,newend,dummy,length;
double maxseq;

if(CheckEmergency() != OK) return(ABORT);
if(w < 0 || w >= WMAX) {
	if(Beta) Alert1("=> Err. ShowPeriods(). Incorrect window index");
	return(MISSED);
	}
if(w != LastEditWindow && Editable[w]) LastEditWindow = w;
w = LastEditWindow;
TextGetSelection(&origin, &end, TEH[w]);
if(origin >= end) return(MISSED);
if(!CompiledAl  || (!CompiledGr && (AddBolsInGrammar() > BolsInGrammar))) {
	CompiledAl = FALSE;
	if(CompileAlphabet() != OK) return(MISSED);
	}
newend = end;
p_a = NULL;
while(origin < end) {
	SetSelect(origin,end,TEH[w]);
	oldorigin = origin;
	if((r = SelectionToBuffer(FALSE,FALSE,w,&p_a,&origin,PROD)) != OK) {
		MyDisposeHandle((Handle*)&p_a);
		/* Could be NULL because of PolyExpand() */
		return(r);
		}
	if(LengthOf(&p_a) > 0) {
		oldend = origin - 1;
		while(isspace(GetTextChar(w,oldend))) oldend--;
		oldend++;
		SetSelect(oldorigin,oldend,TEH[w]);
#if BP_CARBON_GUI_FORGET_THIS
		// FIXME: this destroys the contents of the clipboard
		// so we need another way to remember the deleted text for Undo-ing
		TextCopy(w);
#endif /* BP_CARBON_GUI_FORGET_THIS */
		LastAction = SPACESELECTION;
		UndoWindow = w;
		TextGetSelection(&UndoPos, &dummy, TEH[w]);
		TextDelete(w);
		while((r=PolyMake(&p_a,&maxseq,NO)) == AGAIN);
		if(r == ABORT || r == EXIT) goto BAD;
		r = PrintArg(DisplayMode(&p_a,&ifunc,&hastabs),FALSE,FALSE,TRUE,FALSE,FALSE,stdout,w,pp_Scrap,&p_a);
		if(r != OK) goto BAD;
#if BP_CARBON_GUI_FORGET_THIS
		BPActivateWindow(SLOW,w);
		UpdateDirty(TRUE,w);
#endif /* BP_CARBON_GUI_FORGET_THIS */
		TextGetSelection(&dummy, &newend, TEH[w]);
		end += newend - oldend;
		origin += newend - oldend;
		}
	MyDisposeHandle((Handle*)&p_a);
	/* Could be NULL because of PolyExpand() */
	}
if(Dirty[w]) SetSelect(UndoPos,newend,TEH[w]);
ShowSelect(CENTRE,w);
return(r);

BAD:
MyDisposeHandle((Handle*)&p_a);
return(r);
}


long LengthOf(tokenbyte ***pp_X)
{
#if 1  /* ! _FASTCODE */
long i,l,imax;
long lOffset;

if(*pp_X == NULL) return(ZERO);
imax = MyGetHandleSize((Handle) *pp_X) / sizeof(tokenbyte);
// OPTIMIZE: use & incr temp pointers instead of **pp_X[i]
for(i=0,l=0; ((**pp_X)[i] != TEND) || ((**pp_X)[i+1] != TEND); i+=2, l++) {
	if(i >= imax) {
		sprintf(Message,"=> Err. LengthOf(). i=%ld  imax=%ld",(long)i,(long)imax);
		if(Beta) Println(wTrace,Message);
		return(ZERO);
		}
	}
return(l << 1);	// OPTIMIZE: isn't this always i ?
#else
long lOffset;

if(*pp_X == NULL) return(ZERO);
lOffset = Munger((Handle)*pp_X,ZERO,EndStr,4L,NULL,ZERO);
/* Problem because Endstr may be found too early if some parameter goes for instance 16383 */
if(lOffset < ZERO) {
	if(Beta) Alert1("=> Err. LengthOf()");
	return(ZERO);
	}
return((int) (lOffset >> 1));
#endif
}
	

long CopyBuf(tokenbyte ***pp_X,tokenbyte ***pp_Y)	/* Copy X to Y */
#if ! _FASTCODE
{
unsigned long i=0,im,l=0;
tokenbyte **ptr;

if(*p_maxy > INT_MAX) {
	if(Beta) Alert1("=> Err. CopyBuf().  *p_maxy > INT_MAX. ");
	return(ABORT);
	}
im = (int) *p_maxy - 2;
blocksize = (length + 2L) * sizeof(tokenbyte);
maxsize = MyGetHandleSize((Handle)*pp_Y);
if(maxsize <= blocksize) {
	maxsize = (blocksize * 3L) / 2L;
	MySetHandleSize((Handle*)pp_Y,(Size)maxsize);
	}
while(((**pp_X)[i] != TEND) || ((**pp_X)[i+1] != TEND)) {
	(**pp_Y)[i] = (**pp_X)[i]; i++;
	(**pp_Y)[i] = (**pp_X)[i]; i++;
	if(i > im) {
		if((*p_maxy) == MaxBufferSize) {
			if(ExpandBuffers() != OK) return(ABORT);
			(*p_maxy) = MaxBufferSize;
			im = (int) *p_maxy - 2;
			}
		else {
			if(Beta) Alert1("Increasing buffer in CopyBuf(). ");
			if(ThreeOverTwo(p_maxy) != OK) return(ABORT);
			im = (int) *p_maxy - 2;
			ptr = *pp_Y;
			if((ptr = (tokenbyte**) IncreaseSpace((Handle)ptr)) == NULL) return(ABORT);
			*pp_Y = ptr;
			}
		}
	l++;
	}
(**pp_Y)[i++] = TEND; (**pp_Y)[i] = TEND;
return(l+l);
}
#else
{
Size length;
Size blocksize,maxsize;
tokenbyte *ptr1,*ptr2;
Size oldsize;

length = (Size) LengthOf(pp_X);
blocksize = (length + 2L) * sizeof(tokenbyte);
if(*pp_X == NULL) {
	if(Beta) Alert1("=> Err. CopyBuf(). *pp_X = NULL");
	return(ABORT);
	}
maxsize = oldsize = MyGetHandleSize((Handle)*pp_X);
if(maxsize < blocksize) {
	maxsize = (blocksize * 3L) / 2L;
/*	MemoryUsed += (maxsize - oldsize);
	if(MemoryUsed > MaxMemoryUsed) {
		MaxMemoryUsed = MemoryUsed;
		} */
	if(MySetHandleSize((Handle*)pp_X, maxsize) != OK) return(ABORT);
	}
if((*pp_Y) == NULL) {
	if(Beta) Alert1("=> Err. CopyBuf(). *pp_Y = NULL");
	return(ABORT);
	}
maxsize = oldsize = MyGetHandleSize((Handle)*pp_Y);
if(maxsize < blocksize) {
	maxsize = (blocksize * 3L) / 2L;
/*	MemoryUsed += (maxsize - oldsize);
	if(MemoryUsed > MaxMemoryUsed) {
		MaxMemoryUsed = MemoryUsed;
		} */
	if(MySetHandleSize((Handle*)pp_Y, maxsize) != OK) return(ABORT);
	}
MyLock(FALSE,(Handle)*pp_X);
MyLock(FALSE,(Handle)*pp_Y);
ptr1 = &(**pp_X)[0]; ptr2 = &(**pp_Y)[0];
memmove(ptr2, ptr1, blocksize);
MyUnlock((Handle)*pp_X);
MyUnlock((Handle)*pp_Y);
return(length);
}
#endif


int SelectionToBuffer(int sequence,int noreturn,int w,tokenbyte ***pp_X,
	long *p_end,int mode)
{
char c,*p1,*p2,**ptr,**p_buff,***pp_buff;
p_context *p_plx,*p_prx;
int i,notargument,meta=0,jbolmem,rep,ret;
long origin,end,length;
tokenbyte **p_ti;

if(!CompiledPt) {
	if((rep=CompilePatterns()) != OK) return(rep);
	}
rep = MISSED;
MyDisposeHandle((Handle*)pp_X);
pp_buff = &p_buff; p_buff = NULL;
if(!Editable[w]) return(MISSED);
TextGetSelection(&origin, &end, TEH[w]);
*p_end = end;
SelectOn = TRUE;

POSITION:
while(MySpace(c=GetTextChar(w,origin))) {
	origin++;
	if(origin >= end) {
		SelectOn = FALSE;
		BPPrintMessage(odError,"=> SelectionToBuffer error 1\n");
		return(MISSED);
		}
	}
if(GetTextChar(w,origin) == '[') {
	while((c=GetTextChar(w,origin)) != ']') {
		origin++;
		if(origin >= end) {
			SelectOn = FALSE;
			Panic = TRUE;
			BPPrintMessage(odError,"=> SelectionToBuffer error 2, can't find ']‘\n");
			return(MISSED);
			}
		}
	origin++; goto POSITION;
	}
if(origin >= end) {
	SelectOn = FALSE;
		BPPrintMessage(odError,"=> SelectionToBuffer error 3\n");
	return(MISSED);
	}
length = end - origin + 4L;
/* if(length > 32000L) {
	if(!ScriptExecOn) Alert1("Selection larger than 32,000 chars.  Can't encode");
	else PrintBehind(wTrace,"Selection larger than 32,000 chars.  Can't encode.\n");
	SelectOn = FALSE; return(MISSED);
	} */
if((ptr = (char**) GiveSpace((Size)(length * sizeof(char)))) == NULL) {
	rep = ABORT;
	if(Beta) Alert1("=> Err. SelectionToBuffer(). ptr == NULL");
	goto OUT;
	}
*pp_buff = ptr;
// BPPrintMessage(odInfo,"Selection %d %d length %d\n",origin,end,length);
if(ReadToBuff(YES,noreturn,w,&origin,end,pp_buff) != OK) goto BAD;

*p_end = origin;
MyLock(TRUE,(Handle)*pp_buff);
p1 = **pp_buff; p2 = p1; i = 0; ret = FALSE;
// OPTIMIZE? Is all of this "re-checking" necessary? Look at ReadToBuff() - akozar
// We'll check it later - BB
while(((*p2) != '\0') && (ret || (*p2) != '\r')) {
	// if((*p2) == '�') ret = TRUE; Fixed by BB 2022-02-18
	if(!MySpace((*p2))) ret = FALSE;
	p2++;
	if(++i > length) {
		if(Beta) Alert1("=> Err. SelectionToBuffer(). i > length");
		MyUnlock((Handle)*pp_buff);
		MyDisposeHandle((Handle*)pp_buff);
		SelectOn = FALSE;
		Panic = TRUE;
		return(MISSED);
		}
	} 
if(p1 == p2) {
	MyUnlock((Handle)*pp_buff);
	goto BAD;
	}
jbolmem = Jbol;
notargument = TRUE;
p_ti = Encode(sequence,notargument,0,0,&p1,&p2,p_plx,p_prx,&meta,0,NULL,FALSE,&rep);
MyUnlock((Handle)*pp_buff);
MyDisposeHandle((Handle*)pp_buff);
if(p_ti == NULL) {
	SelectOn = FALSE;
	if(EmergencyExit) return(ABORT);
	else {
		if(rep == OK) return(MISSED);
		else return(rep);
		}
	}
*pp_X = p_ti;
SelectOn = FALSE;
return(OK);

BAD:
MyDisposeHandle((Handle*)pp_buff);

OUT:
if(!ScriptExecOn) {
	Alert1("No data selected");
	BPPrintMessage(odError,"=> No data selected");
	}
else {
	PrintBehind(wTrace,"No data selected.\n");
	}
SelectOn = FALSE;
return(rep);
}


int ReadToBuff(int nocomment,int noreturn,int w,long *p_i,long im,char ***pp_buff)
/* Read TExt buffer */
{
int first;
long j,size,k,length;
char c,oldc,**ptr;

if(*pp_buff == NULL) {
	if(Beta) Alert1("=> Err. ReadToBuff(). *pp_buff == NULL");
	return(ABORT);
	}
size = (long) MyGetHandleSize((Handle)*pp_buff);
size = (long) (size / sizeof(char)) - 1L;
if(size < 2L) {
	if(Beta) Alert1("=> Err. ReadToBuff(). size < 2 ");
	return(ABORT);
	}
if(*p_i >= im) return(MISSED);
first = TRUE; oldc = '\0';
if(stop(0,"ReadToBuff") != OK) return ABORT;
for(j=*p_i,k=0; j < im; j++) {
	c = GetTextChar(w,j);
//	BPPrintMessage(odInfo,"%c",c);
	if(nocomment && c == '*' && oldc == '/') {
		/* Skip C-type remark */
		oldc = '\0'; j++; k--;
		while(TRUE) {
			c = GetTextChar(w,j);
			if(j >= im) {
				c = '\r';
				break;
				}
			if(c == '/' && oldc == '*') {
				j++;
				c = GetTextChar(w,j);
				break;
				}
			oldc = c;
			j++;
			}
		}
	if(c == '\r' /* && oldc != '�' */) {
		if(first || noreturn) continue;
		else break;
		}
	if(c == '\r' /* && oldc == '�' */) {
		c = ' ';
		}
	oldc = c;
	first = FALSE;
	if(noreturn && nocomment && c == '[') {
		j--; break;
		}
//	if(c == '�') (**pp_buff)[k++] = ' ';
	c = Filter(c);
	if(/* c != '�' && */ (c != '\r' || noreturn)) (**pp_buff)[k++] = c;
	if(k >= size) {
		if(ThreeOverTwo(&size) != OK) {
			*p_i = ++j;
			if(!ScriptExecOn) Alert1("Too long paragraph in selection");
			else PrintBehind(wTrace,"Too long paragraph in selection. Aborted.\n");
			return(MISSED);
			}
		ptr = *pp_buff;
		if((ptr = (char**) IncreaseSpace((Handle)ptr)) == NULL) {
			*p_i = ++j;
			return(ABORT);
			}
		*pp_buff = ptr;
		}
	}
(**pp_buff)[k] = '\0';
*p_i = ++j;

CLEAN:
length = MyHandleLen(*pp_buff);
if(length > 0 && ((c=(**pp_buff)[length-1]) == 10 || MySpace(c))) {
	(**pp_buff)[length-1] = '\0';
	goto CLEAN;
	}
return(OK);
}