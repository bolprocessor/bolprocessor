/* Csound.c (BP3) */ 

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

int trace_csound = 0;
int trace_find_instrument = 0;


/* New argument "newinstr" should be YES when the instrument space
   is newly allocated and uninitialized.  This is the case when called from 
   ResizeCsoundInstrumentsSpace().  When "all" is NO, then the instrument
   index, name, and comment are preserved. */

int ResetCsoundInstrument(int j,int all, int newinstr) {
	Handle ptr;
	int i,channel;

	if(!newinstr) {
		// Jinstr has not been adjusted yet when resizing
		if(j < 0 || j >= Jinstr) { 
			BPPrintMessage(0,odError,"=> Err. ResetCsoundInstrument(). Incorrect index = %d\n",j);
			return(MISSED);
			}
		for(channel=1; channel <= MAXCHAN; channel++) {
			if(WhichCsoundInstrument[channel] == (*p_CsInstrumentIndex)[j]) {
				WhichCsoundInstrument[channel] = -1;
				break;
				}
			}
		}
	if(all || newinstr) {
		(*p_CsInstrumentIndex)[j] = -1;
		}
	(*p_CsDilationRatioIndex)[j] = (*p_CsAttackVelocityIndex)[j]
		= (*p_CsReleaseVelocityIndex)[j]
		= (*p_CsPressureStartIndex)[j] = (*p_CsModulationStartIndex)[j]
		= (*p_CsPanoramicStartIndex)[j]
		= (*p_CsPressureEndIndex)[j] = (*p_CsModulationEndIndex)[j]
		= (*p_CsPanoramicEndIndex)[j] = -1;
		
	(*p_CsPitchIndex)[j] = 4;
	(*p_CsPitchFormat)[j] = OPPC;

	(*p_CsVolumeStartIndex)[j] = 5;
	(*p_CsVolumeEndIndex)[j] = 6;
	(*p_CsInstrument)[j].volumetable = 7;

	(*p_CsPitchBendStartIndex)[j] = 8;
	(*p_CsPitchBendEndIndex)[j] = 9;
	(*p_CsInstrument)[j].pitchbendtable = 10;

	(*p_CsInstrument)[j].rPitchBend.islogx = (*p_CsInstrument)[j].rPitchBend.islogy = TRUE;

	(*p_CsInstrument)[j].rVolume.islogx = (*p_CsInstrument)[j].rVolume.islogy
		= (*p_CsInstrument)[j].rPressure.islogx = (*p_CsInstrument)[j].rPressure.islogy
		= (*p_CsInstrument)[j].rModulation.islogx = (*p_CsInstrument)[j].rModulation.islogy
		= (*p_CsInstrument)[j].rPanoramic.islogx = (*p_CsInstrument)[j].rPanoramic.islogy = FALSE;

	(*p_CsInstrument)[j].pitchbendrange = 200;

	(*p_CsInstrument)[j].pressuretable = (*p_CsInstrument)[j].modulationtable
		= (*p_CsInstrument)[j].panoramictable = -1;

	(*p_CsInstrument)[j].pitchbendGEN = (*p_CsInstrument)[j].volumeGEN
		= (*p_CsInstrument)[j].pressureGEN = (*p_CsInstrument)[j].modulationGEN
		= (*p_CsInstrument)[j].panoramicGEN = 7;

	(*p_CsInstrument)[j].iargmax = 10;	// Three Arguments are compulsory

	if(!newinstr) {
		for(i=0; i < (*p_CsInstrument)[j].ipmax; i++) {
			if((*p_CsInstrument)[j].paramlist == NULL) {
				BPPrintMessage(0,odError,"=> Err. ResetCsoundInstrument(). (*p_CsInstrument)[j].paramlist == NULL\n",j);
				continue;
				}
			
			ptr = (Handle) (*((*p_CsInstrument)[j].paramlist))[i].name;
			MyDisposeHandle(&ptr);
			ptr = (Handle) (*((*p_CsInstrument)[j].paramlist))[i].comment;
			MyDisposeHandle(&ptr);
			}
		ptr = (Handle) (*p_CsInstrument)[j].paramlist;
		MyDisposeHandle(&ptr);
		}
		
	(*p_CsInstrument)[j].paramlist = NULL;
	(*p_CsInstrument)[j].ipmax = 0;

	if(all) {
		ptr = (Handle)(*pp_CsInstrumentName)[j];
		MySetHandleSize(&ptr,(Size)2L * sizeof(char));
		(*pp_CsInstrumentName)[j] = (char**) ptr;
		strcpy((*((*pp_CsInstrumentName)[j])),"");
		
		ptr = (Handle)(*pp_CsInstrumentComment)[j];
		MySetHandleSize(&ptr,(Size)2L * sizeof(char));
		(*pp_CsInstrumentComment)[j] = (char**) ptr;
		strcpy((*((*pp_CsInstrumentComment)[j])),"");
		}

	(*(p_CsPitchBend[0]))[j] = 0.; (*(p_CsPitchBend[1]))[j] = DEFTPITCHBEND;
	(*(p_CsPitchBend[2]))[j] = 16383.;
	(*(p_CsVolume[0]))[j] = 0.; (*(p_CsVolume[1]))[j] = 64.;
	(*(p_CsVolume[2]))[j] = 127.;
	(*(p_CsPressure[0]))[j] = 0.; (*(p_CsPressure[1]))[j] = 64.;
	(*(p_CsPressure[2]))[j] = 127.;
	(*(p_CsModulation[0]))[j] = 0.; (*(p_CsModulation[1]))[j] = 8192.;
	(*(p_CsModulation[2]))[j] = 16383.;
	(*(p_CsPanoramic[0]))[j] = 0.; (*(p_CsPanoramic[1]))[j] = 64.;
	(*(p_CsPanoramic[2]))[j] = 127.;

	(*(p_CsPitchBend[3]))[j] = -200.; (*(p_CsPitchBend[4]))[j] = 0.;
	(*(p_CsPitchBend[5]))[j] = 200.;
	(*(p_CsVolume[3]))[j] = Infpos1; (*(p_CsVolume[4]))[j] = Infpos1;
	(*(p_CsVolume[5]))[j] = Infpos1;
	(*(p_CsPressure[3]))[j] = Infpos1; (*(p_CsPressure[4]))[j] = Infpos1;
	(*(p_CsPressure[5]))[j] = Infpos1;
	(*(p_CsModulation[3]))[j] = Infpos1; (*(p_CsModulation[4]))[j] = Infpos1;
	(*(p_CsModulation[5]))[j] = Infpos1;
	(*(p_CsPanoramic[3]))[j] = Infpos1; (*(p_CsPanoramic[4]))[j] = Infpos1;
	(*(p_CsPanoramic[5]))[j] = Infpos1;
	CompiledRegressions = FALSE;
	return(OK);
	}

int FixCsoundScoreName(char* line) {
	return(OK);	// Csound doesn't care! -- akozar 031907
	}


int CompileCsoundObjects(void) { // NO LONGER USED
	int j,rep,longerCsound,maxsounds;

	if(CompiledCsObjects || (!ObjectMode && !EventListOn)) return(OK);
	if(Jbol < 2) return(OK);

	rep = OK;
	CompileOn++;
	maxsounds = MyGetHandleSize((Handle)p_Type) / sizeof(char);
	BPPrintMessage(0,odInfo,"Running CompileCsoundObjects() for %d sound-objects\n",maxsounds);

	for(j=2; j < maxsounds; j++) {
		if(trace_csound) 
			BPPrintMessage(0,odInfo,"%s --> Type[%d] = %d\n",*((*p_Bol)[j]),j,(*p_Type)[j]);
		if((*p_Type)[j] == 0) continue;
		if(!((*p_Type)[j] & 4)) {  // The third bit is not 1
			if((*pp_CsoundScoreText)[j] == NULL) {
				if(trace_csound) 
					BPPrintMessage(0,odInfo, "No Csound score in %d\n",j);
				continue;
				}
			(*p_Type)[j] |= 4; // Set the third bit to 1
			}
		if((rep=CompileObjectScore(j,&longerCsound)) != OK) {
			if(rep == ABORT) goto SORTIR;
			iProto = j;
			CompiledCsObjects = 0;
			rep = MISSED;
			goto SORTIR;
			}
		}
	CompiledCsObjects = 1;

	SORTIR:
	if(CompileOn) CompileOn--;
	return(rep);
	}


int CompileObjectScore(int j,int *p_longerCsound) {
	// Compiling the Csound score of sound-object prototype 'j'

	char c,**p_line,line[MAXLIN];
	int i,ii,ipos,im,i0,i1,ievent,ip,ins,l,nparam,maxparam,maxevents,foundevent,result,
		istempo,overflow,finished,index,html;
	long p,q,count;;
	double param,tempo,dur,**h;
	Handle ptr,ptr2;
	CsoundParam **paramlist;

	if(j < 2 || j >= Jbol) return(OK);

	if(trace_csound) 
		BPPrintMessage(0,odInfo,"CompileObjectScore(%d)\n",j);
	p_line = (*pp_CsoundScoreText)[j];
	if(p_line == NULL) {
		if(trace_csound) BPPrintMessage(0,odInfo, "=> Warning CompileObjectScore(%d). p_line == NULL\n",j);
		return OK;
		}
	if(trace_csound) BPPrintMessage(0,odInfo, "Compiling object score line[%d] = %s\n",j,(*p_line));

	if(strlen(*p_line) == 0 || strcmp((*p_line),"<HTML></HTML>") == 0) {
	//	BPPrintMessage(0,odInfo, "@@@ j = %d empty line\n",j);
		if((*p_Type)[j] & 4) {
			(*p_Type)[j] &= (255-4);
			}
		return(OK);
		}
	// BPPrintMessage(0,odInfo,"%s\n",(*p_line));

	// Csound instructions are required in chronological order
	sort_csound_score(p_line);

	if((result=CompileRegressions()) != OK) return(result);

	// if(trace_csound) BPPrintMessage(0,odError,"Compiling ObjectScore(%d) ? CompiledCsoundScore = %d\n",j,(int)(*p_CompiledCsoundScore)[j]);

	if((*p_CompiledCsoundScore)[j]) {
		if(trace_csound) BPPrintMessage(0,odError,"ObjectScore « %s » is already compiled\n",*((*p_Bol)[j]));
		return(OK);
		}

	if(trace_csound) BPPrintMessage(0,odInfo, "Compiling Csound score in object « %s »\n",*((*p_Bol)[j]));

	maxevents = 12;

	ptr = (Handle) (*pp_CsoundScore)[j];
	if(ptr != NULL) {
		for(i=0; i < (*p_CsoundSize)[j]; i++) {
			ptr2 = (Handle) (*((*pp_CsoundScore)[j]))[i].h_param;
			MyDisposeHandle(&ptr2);
			(*((*pp_CsoundScore)[j]))[i].h_param = NULL;
			}
		MySetHandleSize((Handle*)&ptr,(Size)maxevents * sizeof(CsoundLine));
		}
	else {
		if((ptr=(Handle) GiveSpace((Size)(maxevents * sizeof(CsoundLine)))) == NULL)
			return(ABORT);
		}

	(*pp_CsoundScore)[j] = (CsoundLine**) ptr;

	for(i=0; i < maxevents; i++) {
		(*((*pp_CsoundScore)[j]))[i].h_param = NULL;
		}
		
	ptr = (Handle) (*pp_CsoundTime)[j];
	MyDisposeHandle(&ptr);
	if((ptr=(Handle) GiveSpace((Size)(maxevents * sizeof(Milliseconds)))) == NULL)
			return(ABORT);
	(*pp_CsoundTime)[j] = (Milliseconds**) ptr;
	(*p_CsoundSize)[j] = ZERO;

	ievent = 0; result = OK; finished = FALSE; h = NULL;

	CompileOn++;

	// BPPrintMessage(0,odError,"Now compiling ObjectScore(%d)\n",j);
	count = 1L + MyHandleLen(p_line);
	html = TRUE;
	CheckHTML(TRUE,0,p_line,&count,&html);
	ipos = 0; im = MyHandleLen(p_line);
	(*p_CsoundTempo)[j] = tempo = 60.;

	l = strlen((*p_line)); 
	if(l == 0) return(OK);
	if(trace_csound) BPPrintMessage(0,odInfo,"Compiling Csound score (length %d):\n%s\n",l,(*p_line));

	while(TRUE) {
		result = OK;
		while(ipos < im && (isspace(c=(*p_line)[ipos]) || c == '\0')) ipos++;
		i0 = ipos;
		if(ipos >= im) break;
		if(c == '/' && ipos < (im-1) && (*p_line)[ipos+1] == '/') {	// Skip C-like remarks
			while(ipos < im && (*p_line)[ipos] != '\r' && (*p_line)[ipos] != '\n' && (*p_line)[ipos] != '\0')
				ipos++;
			continue;
			}
		while(ipos < im && (c=(*p_line)[ipos]) != '\r' && c != '\n' && c != '\0')
			ipos++;
		i1 = ipos;
		istempo = FALSE;
	//	BPPrintMessage(0,odInfo,"this_line = %s\n",(*p_line));
		c = (*p_line)[i0];
		if(c == '_') goto NEXTLINE;
		if(c != 'i' && c != 't' && c != 'f' && c != ';' && c != 'e') {
			BPPrintMessage(0,odInfo,"\n=> Csound score line must start with 'i', 'f', 't' or a semi-colon. Can't accept '%c' in the score of object « %s ».\nPart of this score will be ignored:\n%s\n",c,*((*p_Bol)[j]),(*p_line));
			result = OK; goto SORTIR;
			}
		foundevent = FALSE;
		
		(*((*pp_CsoundScore)[j]))[ievent].nbparameters = 0;
		nparam = -1;
		maxparam = 1;
		if((h=(double**) GiveSpace((Size)(maxparam * sizeof(double)))) == NULL) {
			result = ABORT; goto SORTIR;	/* Will be resized later */
			}
		i0++;
		ip = 1;
		if(c == ';') goto NEXTLINE;
		if(finished) {
			BPPrintMessage(0,odError,"=> Unwanted events have been found beyond end of score");
			result = MISSED; goto SORTIR;
			}
		if(c == 't') istempo = TRUE;
		if(c == 'e') finished = TRUE;
		if(c == 'f') {
			BPPrintMessage(0,odInfo,"\n=> Opcode 'f' (table definition) is not supported in sound-object scores. The incorrect line has been ignored in object « %s »:\n%s\n",*((*p_Bol)[j]),(*p_line));
			goto NEXTLINE;
			}

	NEWPARAMETER:
		PleaseWait();
		while(i0 < i1 && (isspace(c=(*p_line)[i0]) || c == '\0')) i0++;
		for(ii=i0; ii < i1; ii++) {
			c = (*p_line)[ii];
			if(isspace(c) || c == ';') break;
			line[ii-i0] = c;
			}
		line[ii-i0] = '\0';
		i0 = ii;
		Strip(line);
		if((c=line[0]) == '\0' || c == ';') goto NEXTLINE;
		param = Myatof(line,&p,&q);
		if(istempo) {
			if(ip == 1 && param != 0) {
				my_sprintf(Message,"=> BP3 can't compile this Csound score: argument following 't' should be 0.");
				BPPrintMessage(0,odError,"%s\n",Message);
				result = MISSED;
				goto SORTIR;
				}
			if(ip > 2) {
				my_sprintf(Message,"=> BP3 can't compile this Csound score: more than %ld arguments following 't'",
					(long)ip-1L);
				BPPrintMessage(0,odError,"%s\n",Message);
				result = MISSED;
				goto SORTIR;
				}
			if(ip == 2) {
				if(param <= 0.) {
					my_sprintf(Message,"=> Incorrect Csound score: 2nd argument following 't' should be positive.");
					BPPrintMessage(0,odError,"%s\n",Message);
					result = MISSED;
					goto SORTIR;
					}
				tempo = (*p_CsoundTempo)[j] = param;
				}
			}
		else {
			foundevent = TRUE;
			switch(ip) {
				case 1:
					if((*p_CsoundInstr)[j] > 0) param = (*p_CsoundInstr)[j];
					for(ins=0; ins < Jinstr; ins++) {
						if((*p_CsInstrumentIndex)[ins] == param) break;
						}
					if(ins >= Jinstr || param != (*p_CsInstrumentIndex)[ins]) {
						if(Jinstr > 1 || ((*p_CsInstrumentIndex)[0] > 0
								&& param != (*p_CsInstrumentIndex)[0])) {
							my_sprintf(Message,
								"=> Can't compile Csound score because instrument %ld is not defined. You may modify or load the '-cs' file",
								(long)param);
							BPPrintMessage(0,odError,"%s\n",Message);
							if((*p_CsoundInstr)[j] > 0) {
								my_sprintf(Message,"=> Perhaps the problem is that you forced this sound-object to use instrument %ld",
									(long)param);
								BPPrintMessage(0,odError,"%s\n",Message);
								}
							result = MISSED;
							goto SORTIR;
							}
						ins = 0;
						}
					(*((*pp_CsoundScore)[j]))[ievent].instrument = param;
					
					paramlist = (*p_CsInstrument)[ins].paramlist;
					
					/* Check the number of arguments in this instrument */
					maxparam = (*p_CsInstrument)[ins].iargmax - 3;
					if(MySetHandleSize((Handle*)&h,(Size)maxparam * sizeof(double)) != OK) {
						result = ABORT; goto SORTIR;
						}
					(*((*pp_CsoundScore)[j]))[ievent].nbparameters = maxparam;
					
					// Set all arguments to default because the score line might be incomplete
					for(i=4; i <= (*p_CsInstrument)[ins].iargmax; i++) {
						(*h)[i-4] = 0.;
						if(i == (*p_CsPitchBendStartIndex)[ins] || i == (*p_CsPitchBendEndIndex)[ins]) {
							(*h)[i-4] = DEFTPITCHBEND; continue;
							}
						if(i == (*p_CsVolumeStartIndex)[ins] || i == (*p_CsVolumeEndIndex)[ins]) {
							(*h)[i-4] = DeftVolume; continue;
							}
						if(i == (*p_CsPressureStartIndex)[ins] || i == (*p_CsPressureEndIndex)[ins]) {
							(*h)[i-4] = DEFTPRESSURE; continue;
							}
						if(i == (*p_CsModulationStartIndex)[ins] || i == (*p_CsModulationEndIndex)[ins]) {
							(*h)[i-4] = DEFTMODULATION; continue;
							}
						if(i == (*p_CsPanoramicStartIndex)[ins] || i == (*p_CsPanoramicEndIndex)[ins]) {
							(*h)[i-4] = DeftPanoramic; continue;
							}
						}
					for(i = 0; i < (*p_CsInstrument)[ins].ipmax; i++) {
						if((index=(*paramlist)[i].startindex) > 0) {
							if(index < 4 || index >= (maxparam+4)) {
								BPPrintMessage(0,odError,"=> Err. CompileObjectScore(). index < 4 || index >= maxparam\n");
								}
							else (*h)[index-4] = (*paramlist)[i].defaultvalue;
							}
						if((index=(*paramlist)[i].endindex) > 0) {
							if(index < 4 || index >= (maxparam+4)) {
								BPPrintMessage(0,odError,"=> Err. CompileObjectScore(). index < 4 || index >= maxparam\n");
								}
							else (*h)[index-4] = (*paramlist)[i].defaultvalue;
							}
						}
					break;
				case 2:
					if(param < 0) {
						BPPrintMessage(0,odError,"=> Can't compile Csound score because a negative date was found: %.2f\n(2nd argument)",param);
						result = MISSED; goto SORTIR;
						}
					param = (param * 60000.) / tempo;
					(*((*pp_CsoundTime)[j]))[ievent] = param;
					break;
				case 3:
					if(param < 0) {
						// FIXME ? any way to allow neg. dur? This has multiple uses with Csound -- akozar
						BPPrintMessage(0,odError,"=> Can't compile Csound score because a negative duration was found.\n(3d argument)");
						result = MISSED; goto SORTIR;
						}
					param = (param * 60000.) / tempo;
					(*((*pp_CsoundScore)[j]))[ievent].duration = param;
					break;
				default:
					/* Modify param according to instrument mapping */
					if(ins < 0 || ins >= Jinstr) {
						BPPrintMessage(0,odError,"=> Err. CompileObjectScore(). ins < 0 || ins >= Jinstr\n");
						if(CompileOn) CompileOn--;
						return(ABORT);
						}
					if(ip == (*p_CsPitchBendStartIndex)[ins] || ip == (*p_CsPitchBendEndIndex)[ins]) {
						param = YtoX(param,&((*p_CsInstrument)[ins].rPitchBend),&overflow,
							(*((*pp_CsoundScore)[j]))[ievent].instrument);
						if(overflow) {
							result = ABORT; goto SORTIR;
							}
						}
					if(ip == (*p_CsVolumeStartIndex)[ins] || ip == (*p_CsVolumeEndIndex)[ins]) {
						param = YtoX(param,&((*p_CsInstrument)[ins].rVolume),&overflow,
							(*((*pp_CsoundScore)[j]))[ievent].instrument);
						if(overflow) {
							result = ABORT; goto SORTIR;
							}
						}
					if(ip == (*p_CsPressureStartIndex)[ins] || ip == (*p_CsPressureEndIndex)[ins]) {
						param = YtoX(param,&((*p_CsInstrument)[ins].rPressure),&overflow,
							(*((*pp_CsoundScore)[j]))[ievent].instrument);
						if(overflow) {
							result = ABORT; goto SORTIR;
							}
						}
					if(ip == (*p_CsModulationStartIndex)[ins] || ip == (*p_CsModulationEndIndex)[ins]) {
						param = YtoX(param,&((*p_CsInstrument)[ins].rModulation),&overflow,
							(*((*pp_CsoundScore)[j]))[ievent].instrument);
						if(overflow) {
							result = ABORT; goto SORTIR;
							}
						}
					if(ip == (*p_CsPanoramicStartIndex)[ins] || ip == (*p_CsPanoramicEndIndex)[ins]) {
						param = YtoX(param,&((*p_CsInstrument)[ins].rPanoramic),&overflow,
							(*((*pp_CsoundScore)[j]))[ievent].instrument);
						if(overflow) {
							result = ABORT; goto SORTIR;
							}
						}
					if(++nparam >= maxparam) {
						if((*p_CsInstrumentIndex)[ins] > 0)
							my_sprintf(Message,
								"=> Csound instrument %ld accepts %ld arguments but the score is supplying more",
								(long)(*p_CsInstrumentIndex)[ins],(long)(maxparam+3));
						else
							my_sprintf(Message,
								"=> Default Csound instrument requires %ld arguments but the score is supplying more",
								(long)(maxparam+3));
						BPPrintMessage(0,odError,"%s\n",Message);
						result = ABORT;
						OutCsound = FALSE;
						goto SORTIR;
						}
					(*h)[nparam] = param;
					break;
				}
			}
		ip++;
		if(i0 < i1) goto NEWPARAMETER;
		
	NEXTLINE:
		if(foundevent) {
			if(ip < 4) {
				if((*p_CsInstrumentIndex)[ins] > 0)
					my_sprintf(Message,
						"=> Csound instrument %ld requires at least 3 arguments whereas the score is supplying %ld",
						(long)(*p_CsInstrumentIndex)[ins],(long)(ip-1));
				else
					my_sprintf(Message,
						"=> Default Csound instrument requires at least 3 arguments whereas the score is supplying %ld",
						(long)(ip-1));
				BPPrintMessage(0,odError,"%s\n",Message);
				if(Jinstr < 2)
					BPPrintMessage(0,odError,"=> You probably forgot to create or load a '-cs' instrument file\n");
				if((*p_CsoundInstr)[j] > 0) {
					my_sprintf(Message,"=> Perhaps the problem is that this sound-object is instructed to use instrument %ld",
						(long)param);
					BPPrintMessage(0,odError,"%s\n",Message);
					}
				MyDisposeHandle((Handle*)&h);
				result = MISSED;
				OutCsound = FALSE;
				goto SORTIR;
				}
				
			(*((*pp_CsoundScore)[j]))[ievent].h_param = h;
			h = NULL;	
			ievent++;
			if(ievent >= maxevents) {
				ptr = (Handle) (*pp_CsoundScore)[j];
				if((ptr = IncreaseSpace((Handle)ptr)) == NULL) {
					result = ABORT; goto SORTIR;
					}
				(*pp_CsoundScore)[j] = (CsoundLine**) ptr;
				ptr = (Handle) (*pp_CsoundTime)[j];
				if((ptr = IncreaseSpace((Handle)ptr)) == NULL) {
					result = ABORT; goto SORTIR;
					}
				(*pp_CsoundTime)[j] = (Milliseconds**) ptr;
				maxevents = (3L * maxevents) / 2L;
				}
			}
		else MyDisposeHandle((Handle*)&h);
		ipos++;
		}

	SORTIR:
	if(CompileOn) CompileOn--;
	MyDisposeHandle((Handle*)&h); /* Useful when result != OK */

	if(result != OK) ievent = 0;
	(*p_CsoundSize)[j] = ievent;
	if(ievent > 0) {
		ptr = (Handle) (*pp_CsoundScore)[j];
		MySetHandleSize((Handle*)&ptr,(Size) ievent * sizeof(CsoundLine));
		(*pp_CsoundScore)[j] = (CsoundLine**) ptr;
		
		ptr = (Handle) (*pp_CsoundTime)[j];
		MySetHandleSize((Handle*)&ptr,(Size) ievent * sizeof(Milliseconds));
		(*pp_CsoundTime)[j] = (Milliseconds**) ptr;
		
		SortCsoundDates(ZERO,j);
		// This is important for the correct scheduling of events in MakeSound()
		}

	PointCsound = TRUE;
	// BPPrintMessage(0,odInfo,"CompiledObjectScore(%d)\n",j);

	if(PointToDuration(NULL,pp_CsoundTime,p_CsoundSize,j) != OK) return(ABORT); 

	SetPrototypeDuration(j,p_longerCsound);

	if(iProto == 0 && Jbol > 2) iProto = 2;
	(*p_CompiledCsoundScore)[j] = 1;
	if(result == OK) BPPrintMessage(0,odInfo,"Csound score of object « %s » (%d) successfully compiled\n",*((*p_Bol)[j]),j);
	else BPPrintMessage(0,odError,"Csound score of object « %s » (%d) not compiled\n",*((*p_Bol)[j]),j);
	return(OK);
	}


int DeCompileObjectScore(int j)
{
long ievent;
int k,ip,rep,ins,longerCsound,overflow,result,instrumentindex;
double param;

if(j < 2 || j >= Jbol) {
	BPPrintMessage(0,odError,"=> Err. DeCompileObjectScore(). j < 2 || j >= Jbol\n");
	return(MISSED);
	}
result = OK;
if((*p_CsoundSize)[j] < 1L) return(OK);
if(CompileRegressions() != OK) return(MISSED);

CompileOn++;

if(ShowMessages && !LoadOn)
	ShowMessage(TRUE,wMessage,"The Csound score of this object is being updated...");

if(DurationToPoint(NULL,pp_CsoundTime,p_CsoundSize,j) != OK) {
	if(CompileOn) CompileOn--;
	return(ABORT);
	}
PrintBehind(wPrototype7,"t 0 ");
WriteFloatToLine(LineBuff,(double)(*p_CsoundTempo)[j]);
PrintBehindln(wPrototype7,LineBuff);
for(ievent=ZERO; ievent < (*p_CsoundSize)[j]; ievent++) {
	result = OK;
	instrumentindex = (*((*pp_CsoundScore)[j]))[ievent].instrument;
	for(ins=0; ins < Jinstr; ins++) {
		if((*p_CsInstrumentIndex)[ins] == instrumentindex) break;
		}
	if(ins >= Jinstr) {
		if(instrumentindex > 1) {
			BPPrintMessage(0,odError,"=> Can't decompile this Csound score. We need a '-cs' file describing Csound instruments");
			PointToDuration(NULL,pp_CsoundTime,p_CsoundSize,j);
			BPActivateWindow(SLOW,wCsoundResources);
			DurationToPoint(NULL,pp_CsoundTime,p_CsoundSize,j);
			result = ABORT;
			goto SORTIR;
			}
		ins = 0; instrumentindex = 1;
		}
	my_sprintf(LineBuff,"i%ld ",(long)instrumentindex);
	PrintBehind(wPrototype7,LineBuff);
	WriteFloatToLine(LineBuff,(double)(*((*pp_CsoundTime)[j]))[ievent]
											* (*p_CsoundTempo)[j] / 60000.);
	PrintBehind(wPrototype7,LineBuff); PrintBehind(wPrototype7," ");
	WriteFloatToLine(LineBuff,(double)(*((*pp_CsoundScore)[j]))[ievent].duration
											* (*p_CsoundTempo)[j] / 60000.);
	PrintBehind(wPrototype7,LineBuff); PrintBehind(wPrototype7," ");
	if((*((*pp_CsoundScore)[j]))[ievent].h_param != NULL) {
		for(k=0; k < (*((*pp_CsoundScore)[j]))[ievent].nbparameters; k++) {
			param = (*(*((*pp_CsoundScore)[j]))[ievent].h_param)[k];
			ip = k + 4;
			if(ip == (*p_CsPitchBendStartIndex)[ins] || ip == (*p_CsPitchBendEndIndex)[ins]) {
				param = XtoY(param,&((*p_CsInstrument)[ins].rPitchBend),&overflow,
					instrumentindex);
				if(overflow) {
					result = ABORT; goto SORTIR;
					}
				}
			if(ip == (*p_CsVolumeStartIndex)[ins] || ip == (*p_CsVolumeEndIndex)[ins]) {
				param = XtoY(param,&((*p_CsInstrument)[ins].rVolume),&overflow,
					instrumentindex);
				if(overflow) {
					result = ABORT; goto SORTIR;
					}
				}
			if(ip == (*p_CsPressureStartIndex)[ins] || ip == (*p_CsPressureEndIndex)[ins]) {
				param = XtoY(param,&((*p_CsInstrument)[ins].rPressure),&overflow,
					instrumentindex);
				if(overflow) {
					result = ABORT; goto SORTIR;
					}
				}
			if(ip == (*p_CsModulationStartIndex)[ins] || ip == (*p_CsModulationEndIndex)[ins]) {
				param = XtoY(param,&((*p_CsInstrument)[ins].rModulation),&overflow,
					instrumentindex);
				if(overflow) {
					result = ABORT; goto SORTIR;
					}
				}
			if(ip == (*p_CsPanoramicStartIndex)[ins] || ip == (*p_CsPanoramicEndIndex)[ins]) {
				param = XtoY(param,&((*p_CsInstrument)[ins].rPanoramic),&overflow,
					instrumentindex);
				if(overflow) {
					result = ABORT; goto SORTIR;
					}
				}
			WriteFloatToLine(LineBuff,param);
			PrintBehind(wPrototype7,LineBuff); PrintBehind(wPrototype7," ");
			}
		}
	PrintBehind(wPrototype7,"\n");
	}
PrintBehind(wPrototype7,"\ne\n");

SORTIR:

if(PointToDuration(NULL,pp_CsoundTime,p_CsoundSize,j) != OK) result = ABORT;
if(result == OK) result = CompileObjectScore(j,&longerCsound);

if(CompileOn) CompileOn--;

return(result);
}

int FindCsoundInstrument(char* line) {
	// Find the instrument index in the compiled '-cs' file
	int i,j,isnumber;

	Strip(line);
	if(strlen(line) == 0) {
		if(trace_find_instrument) BPPrintMessage(0,odInfo,"@§ line is empty\n");
		isnumber = FALSE; goto ERR;
		}
	isnumber = TRUE;
	for(i = 0; i < strlen(line); i++) {
		if(!isdigit(line[i])) {
			if(trace_find_instrument) BPPrintMessage(0,odInfo,"@§ non-digit found\n");
			isnumber = FALSE; break;
			}
		}
	if(isnumber) {
		i = (int) atol(line);
		if(i < 1 || i >=  MAXINSTRUMENTS) {
			BPPrintMessage(0,odError,"=> ins(%d) is incorrect, beyond range 1 to %d\n",i,MAXINSTRUMENTS);
			return MISSED;
			}
		if(trace_find_instrument) BPPrintMessage(0,odInfo,"@§ found number i = %d\n",i);
		for(j = 0; j < Jinstr; j++) {
		//	BPPrintMessage(0,odInfo,"InstrumentIndex[%d] = %d, i = %d (%s)\n",j,(*p_CsInstrumentIndex)[j],i,(*(*pp_CsInstrumentName)[j]));
			if(i == (*p_CsInstrumentIndex)[j]) {
				if(trace_find_instrument) BPPrintMessage(0,odInfo,"@§ found instrument index j = %d\n",j);
				break;
				}
			}
		if(j < Jinstr) return(i);
		goto ERR;
		}
	for(j = 0; j < Jinstr; j++) {
	//	BPPrintMessage(0,odInfo,"InstrumentName[%d] = %s\n",j,(*(*pp_CsInstrumentName)[j]),j);
	//	if(Mystrcmp((*pp_CsInstrumentName)[j],line) == 0) break;
		if(strcmp(*((*pp_CsInstrumentName)[j]),line) == 0) {
			if(trace_find_instrument) BPPrintMessage(0,odInfo,"@§ found instrument name j = %d “%s”\n",j,line);
			break;
			}
		}
	if(j < Jinstr) return((*p_CsInstrumentIndex)[j]);

	ERR:
	if(Jinstr < 2) {
		BPPrintMessage(0,odInfo,"👉 You probably forgot to create or declare a '-cs' instrument file\n");
		}
	if(isnumber) {
		BPPrintMessage(0,odError,"=> Instrument #%d was not found in a '-cs' instrument file %d\n",i,Jinstr);
		return(1);
		}
	else BPPrintMessage(0,odError,"=> Instrument \"%s\" was not found in a '-cs' instrument file",line);
	return(1);
	}


int ResetMoreParameter(int j,int ip) {
	char **ptr;
	Handle h;
	CsoundParam **paramlist;

	paramlist = (*p_CsInstrument)[j].paramlist;

	if(paramlist == NULL) {
		BPPrintMessage(0,odError,"=> Err. ResetMoreParameter(). paramlist == NULL");
		return(OK);
		}
	h = (Handle) (*paramlist)[ip].name;
	MyDisposeHandle(&h);
	(*paramlist)[ip].name = NULL;
	h = (Handle) (*paramlist)[ip].comment;
	MyDisposeHandle(&h);
	(*paramlist)[ip].comment = NULL;

	(*paramlist)[ip].startindex
		= (*paramlist)[ip].endindex
		= (*paramlist)[ip].nameindex
		= (*paramlist)[ip].table = -1;

	(*paramlist)[ip].defaultvalue = 0.;
	(*paramlist)[ip].GENtype = 7;
	(*paramlist)[ip].combinationtype = ADD;
	return(OK);
	}

/* Returns 1 and sets *date if the line (content_len chars, without <BR>)
   is an i-statement with a numeric p2. */
static int parse_i_statement(const char *s, size_t content_len, double *date)
{
    const char *p = s, *end = s + content_len;
    char *q;
    double d;

    while (p < end && isspace((unsigned char)*p)) p++;
    if (p >= end || *p != 'i') return 0;
    p++;
    while (p < end && isspace((unsigned char)*p)) p++;   /* "i 1" form      */
    while (p < end && !isspace((unsigned char)*p)) p++;  /* instrument p1   */
    while (p < end && isspace((unsigned char)*p)) p++;
    if (p >= end) return 0;

    d = strtod(p, &q);
    if (q == p || q > end) return 0;   /* no numeric p2 (e.g. "+" or ".") */
    *date = d;
    return 1;
}

static int compare_lines(const void *a, const void *b)
{
    const ScoreLine *x = (const ScoreLine *)a;
    const ScoreLine *y = (const ScoreLine *)b;
    if (x->date < y->date) return -1;
    if (x->date > y->date) return  1;
    return (x->order < y->order) ? -1 : (x->order > y->order); /* stable */
}


int sort_csound_score(char **p_line) {
// Sorts each block of consecutive i-statements by start date (p2)
    char *str, *buf, *dst;
    const char *p, *next;
    size_t total, n = 1, k = 0, a, b;
    ScoreLine *lines;

    if (p_line == NULL || *p_line == NULL) return ABORT;
    str = *p_line;
    total = strlen(str);

    // Count lines
    for (p = str; (p = strstr(p, LINE_SEP)) != NULL; p += LINE_SEP_LEN) n++;

    lines = (ScoreLine *)malloc(n * sizeof(ScoreLine));
    buf   = (char *)malloc(total + 1);
    if (lines == NULL || buf == NULL) { free(lines); free(buf); return ABORT; }

	// Split into lines, each keeping its trailing <BR>
    p = str;
    for (;;) {
        size_t content_len;
        next = strstr(p, LINE_SEP);
        content_len = next ? (size_t)(next - p) : strlen(p);
        lines[k].text  = p;
        lines[k].len   = next ? content_len + LINE_SEP_LEN : content_len;
        lines[k].order = (long)k;
        lines[k].date  = 0.;
        lines[k].is_i  = parse_i_statement(p, content_len, &lines[k].date);
        k++;
        if (next == NULL) break;
        p = next + LINE_SEP_LEN;
    	}

    // Sort each run of consecutive i-statements; other lines (t, f, s, comments, <HTML>...) stay where they are.
    for (a = 0; a < n; a = b) {
        if (!lines[a].is_i) { b = a + 1; continue; }
        for (b = a; b < n && lines[b].is_i; b++) ;
        if (b - a > 1)
            qsort(lines + a, b - a, sizeof(ScoreLine), compare_lines);
    	}

    // Rebuild, then copy back (same length, so the handle size is unchanged)
    dst = buf;
    for (k = 0; k < n; k++) {
        memcpy(dst, lines[k].text, lines[k].len);
        dst += lines[k].len;
    	}
    *dst = '\0';
    memcpy(str, buf, total + 1);
    free(lines);
    free(buf);
    return OK;
	}


/* Escape a C string for JSON (quotes, backslashes, control characters).
   Returns a malloc'd string the caller must free(), or NULL. */
static char *JsonEscapeString(const char*);

static char *JsonEscapeString(const char *s) {
	size_t len;
	const unsigned char *p;
	char *out,*q;

	if(s == NULL) s = "";
	len = strlen(s);
	out = (char*) malloc(6 * len + 1);
	if(out == NULL) return(NULL);
	q = out;
	for(p = (const unsigned char*) s; *p != '\0'; p++) {
		switch(*p) {
			case '"':  *q++ = '\\'; *q++ = '"';  break;
			case '\\': *q++ = '\\'; *q++ = '\\'; break;
			case '\n': *q++ = '\\'; *q++ = 'n';  break;
			case '\r': *q++ = '\\'; *q++ = 'r';  break;
			case '\t': *q++ = '\\'; *q++ = 't';  break;
			default:
				if(*p < 0x20) {
					my_sprintf(q,"\\u%04x",(unsigned int) *p);
					q += 6;
					}
				else *q++ = (char) *p;
			}
		}
	*q = '\0';
	return(out);
	}

static int SaveEscapedStringAsJson(FILE*,const char*,const char*,const char*,const char*);

static int SaveEscapedStringAsJson(FILE *csjson,const char *key,const char *unit,const char *name,const char *value) {
	char *esc;
	int result;

	esc = JsonEscapeString(value);
	if(esc == NULL) return(MISSED);
	result = SaveStringAsJson(csjson,key,unit,name,esc);
	free(esc);
	return(result);
	}

static const char *CsPitchFormatName(int);

static const char *CsPitchFormatName(int format) {
	switch(format) {
		case OPPC:    return("OPPC");
		case OPD:     return("OPD");
		case CPS:     return("CPS");
		case IGNORER: return("IGNORER");
		}
	return("UNKNOWN");
	}

/* Returns the description if it is not empty, otherwise the fallback (variable name) */
static const char *JsonLabel(const char*,const char*);

static const char *JsonLabel(const char *description,const char *fallback) {
	if(description == NULL || description[0] == '\0') return(fallback);
	return(description);
	}

/* Shortcuts: #var turns the variable name into the JSON key.
   The last argument is the description used as the JSON label;
   if it is "", the variable name is used as the label. */
#define CSI ((*p_CsInstrument)[j])

#define SAVE_CS_INT(var,desc) \
	SaveIntAsJson(csjson,#var,"",JsonLabel(desc,#var),(int)((*p_##var)[j]))

#define SAVE_INSTR_INT(field,unit,desc) \
	SaveIntAsJson(csjson,#field,unit,JsonLabel(desc,#field),(int)(CSI.field))

#define SAVE_INSTR_BOOL(reg,field,desc) \
	SaveIntAsJson(csjson,#reg "." #field,"boolean",JsonLabel(desc,#reg "." #field),(CSI.reg.field != 0))

#define SAVE_CS_TABLE(var,desc) do { \
	for(k = 0; k < 6; k++) { \
		snprintf(label,sizeof(label),"%s[%d]",#var,k); \
		snprintf(label2,sizeof(label2),"%s#%d",JsonLabel(desc,#var),(k + 1)); \
		SaveFloatAsJson(csjson,label,"",label2,(double)((*(p_##var[k]))[j])); \
		} \
	} while(0)



int ExportJsonCsoundInstrument(int instrument) {
	int i,j,k,ipmax,chan;
	CsoundParam **params;
	FILE *csjson;
	char instrument_path[1024],label[MAXNAME],label2[MAXNAME];
	int length;

	// BPPrintMessage(0,odInfo,"👉 Creating json file ??? instrument = %d\n",instrument);
	if(instrument < 1 || instrument >= MAXINSTRUMENTS) return(OK);
	for(j = 0; j < Jinstr; j++)
		if((*p_CsInstrumentIndex)[j] == instrument) break;
	if(j < Jinstr) snprintf(label,sizeof label,"%s",*((*pp_CsInstrumentName)[j]));
	else return(OK);

	Exported_instrument[instrument] = TRUE;
	csjson = NULL;

	BPPrintMessage(0,odInfo,"👉 Creating json file of Csound instr #%d « %s »\n",instrument,label);

	length = snprintf(instrument_path, sizeof instrument_path,"%sinstr-%s.json",OutputDir,label);
	if(length < 0 || (size_t)length >= sizeof instrument_path) {
		BPPrintMessage(0,odError,"=> Csound instrument file path is too long\n");
		return(MISSED);
		}
	else csjson = fopen(instrument_path,"w");

	if(csjson == NULL) return(MISSED);

	fprintf(csjson,"{\n\"header\": \"Bol Processor Csound instrument\",\n");

	/* Identification */
	my_sprintf(label,"%s",BaseName(FileName[wCsoundResources]));
	SaveEscapedStringAsJson(csjson,"FileName","","Csound resource file containing this instrument",label);

	SAVE_CS_INT(CsInstrumentIndex,"Index of this instrument");
	SaveEscapedStringAsJson(csjson,"CsInstrumentName","","Name of this instrument",*((*pp_CsInstrumentName)[j]));
	SaveEscapedStringAsJson(csjson,"CsInstrumentComment","","Comment on this instrument",*((*pp_CsInstrumentComment)[j]));
	for(i = k = 0; i < MaxCsoundTables; i++) {
		if((*p_CsoundTables)[i] == NULL || strlen((*(*p_CsoundTables)[i])) == 0) continue;
		my_sprintf(label,"CsoundTables[%d]",k);
		my_sprintf(label2,"Csound table line #%d",(k+1));
		SaveEscapedStringAsJson(csjson,label,"",label2,(*(*p_CsoundTables)[i]));
		k++;
		}
	if(strlen(CsoundOrchestraName) > 0)
		SaveEscapedStringAsJson(csjson,"CsoundOrchestraName","","Csound orchestra file",CsoundOrchestraName);
	else SaveEscapedStringAsJson(csjson,"CsoundOrchestraName","","Csound orchestra file","no file");
	SAVE_INSTR_INT(iargmax,"","Number of arguments");

	for(chan = 1; chan <= MAXCHAN; chan++) {
		if(WhichCsoundInstrument[chan] > 0) {
			if(WhichCsoundInstrument[chan] == instrument) {
				my_sprintf(label,"WhichCsoundInstrument[%d]",chan);
				my_sprintf(label2,"%d",chan);
				SaveEscapedStringAsJson(csjson,label,"","MIDI channel associated with this instrument",label2);
				}
			}
		}

	/* Argument indexes */
	SAVE_CS_INT(CsDilationRatioIndex,"Argument of dilation ratio (-1 means empty)");
	SAVE_CS_INT(CsAttackVelocityIndex,"Argument of attack velocity (-1 means empty)");
	SAVE_CS_INT(CsReleaseVelocityIndex,"Argument of release velocity (-1 means empty)");
	// SAVE_CS_INT(CsoundTempo,"");
	SAVE_CS_INT(CsPitchIndex,"Argument of pitch");
	SaveStringAsJson(csjson,"CsPitchFormat","enum","CsPitchFormat",CsPitchFormatName((int)(*p_CsPitchFormat)[j]));
	SAVE_CS_INT(CsVolumeStartIndex,"Argument of start volume");
	SAVE_CS_INT(CsVolumeEndIndex,"Argument of end volume");
	SAVE_CS_INT(CsPitchBendStartIndex,"Argument of start pitchbend");
	SAVE_CS_INT(CsPitchBendEndIndex,"Argument of end pitchbend");
	SAVE_CS_INT(CsPressureStartIndex,"Argument of start pressure");
	SAVE_CS_INT(CsPressureEndIndex,"Argument of end pressure");
	SAVE_CS_INT(CsModulationStartIndex,"Argument of start modulation");
	SAVE_CS_INT(CsModulationEndIndex,"Argument of end modulation");
	SAVE_CS_INT(CsPanoramicStartIndex,"Argument of start panoramic");
	SAVE_CS_INT(CsPanoramicEndIndex,"Argument of end panoramic");

	/* Pitchbend range */
	SaveFloatAsJson(csjson,"pitchbendrange","cents","pitchbendrange",CSI.pitchbendrange);

	/* Tables */
	SAVE_INSTR_INT(pitchbendtable,"","Argument of pitchbend table (-1 means empty)");
	SAVE_INSTR_INT(volumetable,"","Argument of volume table (-1 means empty)");
	SAVE_INSTR_INT(pressuretable,"","Argument of pressure table (-1 means empty)");
	SAVE_INSTR_INT(modulationtable,"","Argument of modulation table (-1 means empty)");
	SAVE_INSTR_INT(panoramictable,"","Argument of panoramic table (-1 means empty)");

	/* GEN types */
	SAVE_INSTR_INT(pitchbendGEN,"","Pitchbend GEN type");
	SAVE_INSTR_INT(volumeGEN,"","Volume GEN type");
	SAVE_INSTR_INT(pressureGEN,"","Pressure GEN type");
	SAVE_INSTR_INT(modulationGEN,"","Modulation GEN type");
	SAVE_INSTR_INT(panoramicGEN,"","Panoramic GEN type");

	/* Logarithmic scales of the regressions */
	SAVE_INSTR_BOOL(rPitchBend,islogx,"MIDI pitchbend is logarithmic");
	SAVE_INSTR_BOOL(rPitchBend,islogy,"Csound pitchbend is logarithmic");
	SAVE_INSTR_BOOL(rVolume,islogx,"MIDI volume is logarithmic");
	SAVE_INSTR_BOOL(rVolume,islogy,"Csound volume is logarithmic");
	SAVE_INSTR_BOOL(rPressure,islogx,"MIDI pressure is logarithmic");
	SAVE_INSTR_BOOL(rPressure,islogy,"Csound pressure is logarithmic");
	SAVE_INSTR_BOOL(rModulation,islogx,"MIDI modulation is logarithmic");
	SAVE_INSTR_BOOL(rModulation,islogy,"Csound modulation is logarithmic");
	SAVE_INSTR_BOOL(rPanoramic,islogx,"MIDI panoramic is logarithmic");
	SAVE_INSTR_BOOL(rPanoramic,islogy,"Csound panoramic is logarithmic");

	/* MIDI -> Csound mapping points: [0..2] MIDI values, [3..5] Csound values */
	SAVE_CS_TABLE(CsPitchBend,"MIDI to Csound pitchbend conversion (number > 100000 means no mapping) ");
	SAVE_CS_TABLE(CsVolume,"MIDI to Csound volume mapping (number > 100000 means no mapping) ");
	SAVE_CS_TABLE(CsPressure,"MIDI to Csound pressure mapping (number > 100000 means no mapping) ");
	SAVE_CS_TABLE(CsModulation,"MIDI to Csound modulation mapping (number > 100000 means no mapping) ");
	SAVE_CS_TABLE(CsPanoramic,"MIDI to Csound panoramic mapping (number > 100000 means no mapping) ");

	/* Associated (untyped) parameters */
	ipmax = CSI.ipmax;
	SAVE_INSTR_INT(ipmax,"","Number of additional parameters");
	params = CSI.paramlist;
	for(i = 0; i < ipmax; i++) {
		snprintf(label,sizeof(label),"paramlist[%d].name",i);
		snprintf(label2,sizeof(label2),"Name of additional parameter #%d",(i + 1));
		SaveEscapedStringAsJson(csjson,label,"",label2,*((*params)[i].name));
		snprintf(label,sizeof(label),"paramlist[%d].comment",i);
		snprintf(label2,sizeof(label2),"Comment on additional parameter #%d",(i + 1));
		SaveEscapedStringAsJson(csjson,label,"",label2,*((*params)[i].comment));
		snprintf(label,sizeof(label),"paramlist[%d].startindex",i);
		snprintf(label2,sizeof(label2),"Start index of additional parameter #%d",(i + 1));
		SaveIntAsJson(csjson,label,"",label2,(*params)[i].startindex);
		snprintf(label,sizeof(label),"paramlist[%d].endindex",i);
		snprintf(label2,sizeof(label2),"End index of additional parameter #%d",(i + 1));
		SaveIntAsJson(csjson,label,"",label2,(*params)[i].endindex);
		snprintf(label,sizeof(label),"paramlist[%d].table",i);
		snprintf(label2,sizeof(label2),"Table index of additional parameter #%d",(i + 1));
		SaveIntAsJson(csjson,label,"",label2,(*params)[i].table);
		snprintf(label,sizeof(label),"paramlist[%d].defaultvalue",i);
		snprintf(label2,sizeof(label2),"Default value of additional parameter #%d",(i + 1));
		SaveFloatAsJson(csjson,label,"",label2,(*params)[i].defaultvalue);
		snprintf(label,sizeof(label),"paramlist[%d].GENtype",i);
		snprintf(label2,sizeof(label2),"GEN type of additional parameter #%d",(i + 1));
		SaveIntAsJson(csjson,label,"",label2,(*params)[i].GENtype);
		snprintf(label,sizeof(label),"paramlist[%d].combinationtype",i);
		snprintf(label2,sizeof(label2),"Combination mode of additional parameter #%d",(i + 1));
		SaveStringAsJson(csjson,label,"enum",label2,
			((*params)[i].combinationtype == MULT) ? "MULT" : "ADD");
		}
		
	fprintf(csjson,"    \"End of file\": {\n      }\n  }");
	fclose(csjson);
	return(OK);
	}

#undef CSI
#undef SAVE_CS_INT
#undef SAVE_INSTR_INT
#undef SAVE_INSTR_BOOL
#undef SAVE_CS_TABLE