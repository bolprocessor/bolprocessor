/* SaveLoads1.c (BP3) */ 

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

int trace_load_settings = 0;
int trace_load_prototypes = 0;
int trace_load_csound_instruments = 0;
int trace_load_scales = 0;


int LoadTonality(void) {
	int result,length;
	char name[MAXLIN], note_names[MAXLIN], key_numbers[MAXLIN], fractions[MAXLIN], baseoctave_string[10];
	char **p_line, **p_completeline;
	long pos = 0L;
	int found = 0;
	FILE *csfile;
	p_line = p_completeline = NULL;
	int i_scale = 0;
	strcpy(name,""); strcpy(note_names,""); strcpy(key_numbers,""); strcpy(baseoctave_string,"");
	csfile = my_fopen(1, FileName[wTonality], "r");
	if(csfile == NULL) {
		BPPrintMessage(0,odError, "=> Could not open tonality file: %s\n", FileName[wTonality]);
		goto ERREUR;
		}
	else BPPrintMessage(0,odInfo, "Loading tonality: %s\n", FileName[wTonality]);
	while(TRUE) {
		if(ReadOne(FALSE,FALSE,TRUE,csfile,TRUE,&p_line,&p_completeline,&pos) != OK) goto QUITTER;
		if(Mystrcmp(p_line,"_begin tables") == 0) {
			found = 1;
			break;
			}
		}
	if(!found) goto ERREUR;
	while(TRUE) {
		if(ReadOne(FALSE,FALSE,TRUE,csfile,TRUE,&p_line,&p_completeline,&pos) != OK) goto QUITTER;
		Strip(*p_line);
		if(strlen(*p_line) == 0) goto QUITTER; // Required because 'pos' is not incremented when reading an empty line
		if(Mystrcmp(p_line,"_end tables") == 0) break;
		if(trace_load_scales) BPPrintMessage(0,odInfo, "table line = [%s]\n",*p_line);
		if((*p_line)[0] == '"') { // This line contains the name of the next scale
			MystrcpyHandleToString((strlen(*p_line) - 2),1,name,p_line);
	//		BPPrintMessage(0,odInfo, "name = [%s]\n",name);
			continue;
			}
		if((*p_line)[0] == '<') continue; // Ignore comments
		if((*p_line)[0] == '[') { // fractions
			MystrcpyHandleToString(strlen(*p_line),0,fractions,p_line);
			continue;
			}
		if((*p_line)[0] == 'c') continue; // Ignore comma
		if((*p_line)[0] == 's') continue; // Ignore series
		if((*p_line)[0] == '|') { // baseoctave
			MystrcpyHandleToString(strlen(*p_line)-2,1,baseoctave_string,p_line);
	//		BPPrintMessage(0,odInfo, "baseoctave_string = [%s]\n",baseoctave_string);
			continue;
			}
		if((*p_line)[0] == '/') { // This line contains note names for this scale
			MystrcpyHandleToString(strlen(*p_line),0,note_names,p_line);
			continue;
			}
		if((*p_line)[0] == 'k') { // This line contains key numbers for this scale
			MystrcpyHandleToString(strlen(*p_line),0,key_numbers,p_line);
			continue;
			}
		length = MyHandleLen(p_completeline);
		if(length > 0) {
			result = CreateMicrotonalScale(*p_line,name,note_names,key_numbers,fractions,baseoctave_string);
			strcpy(name,""); strcpy(note_names,""); strcpy(key_numbers,""); strcpy(baseoctave_string,"");
			if(result == OK) i_scale++;
			}
		}
	if(i_scale > 0) {
		BPPrintMessage(0,odInfo,"%d tonal scale(s) found\n\n",i_scale);
		}

QUITTER:
	MyDisposeHandle((Handle *)&p_line);
	MyDisposeHandle((Handle *)&p_completeline);
	my_fclose(csfile);
ERREUR:
	return OK;
	}


int LoadCsoundInstruments(int checkversion,int tryname) {
	int i,io,iv,ip,jmax,j,result,y,maxticks,maxbeats,arg,length,i_table,ipmax;
	char **ptr;
	char line[MAXLIN];
	Handle **ptr2;
	CsoundParam **ptr3;
	long pos,x;
	char **p_line,**p_completeline;
	double r;
	FILE* csfile;

	if(trace_load_csound_instruments) BPPrintMessage(0,odInfo, "LoadCsoundInstruments(%d,%d)\n",checkversion,tryname);

	if(LoadedCsoundInstruments) {
		if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"Csound instruments file has already been loaded\n");
		return(OK);
		}

	iCsoundInstrument = 0;
	LoadOn++;
	pos = ZERO;
	Dirty[wCsoundResources] = CompiledRegressions = CompiledCsObjects = 0;
	p_line = p_completeline = NULL;

	if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"Opening Csound instruments file: %s\n",FileName[wCsoundResources]);

	// Check whether file is being saved
	my_sprintf(line,"%s_lock",FileName[wCsoundResources]);
	csfile = my_fopen(0,line,"r");
	if(csfile != NULL) {
		BPPrintMessage(0,odError,"\n=> As the Csound instruments file was locked I waited for 3 seconds...\n");
	//	sleep(2);
		delay(3); // Fixed by BB 2022-02-20
		}

	csfile = my_fopen(1,FileName[wCsoundResources],"r");
	if(csfile == NULL) {
		BPPrintMessage(0,odError,"=> Could not open Csound instruments file: %s\n",FileName[wCsoundResources]);
		goto ERR;
		}

	if(ReadOne(FALSE,FALSE,FALSE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	BPPrintMessage(0,odInfo,"Loading: %s\n",FileName[wCsoundResources]);
	if(trace_load_csound_instruments) BPPrintMessage(0,odInfo, "Line = %s\n",*p_line);
	if(CheckVersion(&iv,p_line,FileName[wCsoundResources]) != OK) goto ERR;
	if(ReadOne(FALSE,TRUE,FALSE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	// GetDateSaved(p_completeline,&(p_FileInfo[wCsoundResources]));
	if(ReadInteger(csfile,&jmax,&pos) == MISSED) goto ERR;
	if(jmax != MAXCHAN) {
		BPPrintMessage(0,odError,"=> This file is empty or in an unknown format\n");
		goto QUIT;
		}
	for(j=1; j <= jmax; j++) {
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		WhichCsoundInstrument[j] = i;
		}
	if(iv > 11) {
		if(ReadOne(FALSE,TRUE,FALSE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
		MystrcpyHandleToString(MAXNAME,0,CsoundOrchestraName,p_completeline);
		}
	else CsoundOrchestraName[0] = '\0';
	if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"CsoundOrchestraName = %s\n",CsoundOrchestraName);
	if(ReadInteger(csfile,&jmax,&pos) == MISSED) goto ERR;
	if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"jmax = %d\n",jmax);
	if(jmax < 0) {
		BPPrintMessage(0,odError,"=> This file is empty or in an unknown format\n");
		goto QUIT;
		}
	if(jmax > 0 && (result=ResizeCsoundInstrumentsSpace(jmax)) != OK) goto ERR;
	result = MISSED;

	if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"Jinstr = %d\n",Jinstr);

	for(j=0; j < jmax; j++) {
		ResetCsoundInstrument(j,YES,YES);
		if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"j = %d\n",j);
		if(ReadOne(FALSE,TRUE,FALSE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	//	ptr = (*pp_CsInstrumentName)[j];
		ptr = NULL; // Fixed by BB 2021-02-14
		if((*p_completeline)[0] != '\0') {
			MystrcpyHandleToString(MAXLIN,0,LineBuff,p_completeline);
		//	if(ShowMessages) ShowMessage(TRUE,wMessage,LineBuff);
			if(MySetHandleSize((Handle*)&ptr,(1L + MyHandleLen(p_completeline)) * sizeof(char)) != OK)
				goto ERR;
			MystrcpyHandleToHandle(0,&ptr,p_completeline);
			(*pp_CsInstrumentName)[j] = ptr;
			}
		else strcpy((*((*pp_CsInstrumentName)[j])),""); // (*((*pp_CsInstrumentName)[j]))[0] = '\0'; Fixed by BB 2021-02-14
		
		if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"Loading Csound instrument %d = \"%s\" out of %d\n",(j+1),(*((*pp_CsInstrumentName)[j])),jmax);
	//	p_line = p_completeline = NULL; 
		if(ReadOne(FALSE,TRUE,FALSE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	/*	if((*p_completeline)[0] != '\0') {
			if(MySetHandleSize((Handle*)&ptr,(1L + MyHandleLen(p_completeline)) * sizeof(char)) != OK)
				goto ERR;
			MystrcpyHandleToHandle(0,&ptr,p_completeline);
			(*pp_CsInstrumentComment)[j] = ptr;
			if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"Comment: %s\n",(*p_completeline));
			}
		else strcpy((*((*pp_CsInstrumentComment)[j])),""); // (*((*pp_CsInstrumentComment)[j]))[0] = '\0'; Fixed by BB 2021-02-14 */
		strcpy((*((*pp_CsInstrumentComment)[j])),""); // Forget comment! 2024-05-20
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].iargmax = i;
		if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"iargmax = %d\n",i);
		
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrumentIndex)[j] = i;
		if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"CsInstrumentIndex = %d\n",i);
		
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsDilationRatioIndex)[j] = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsAttackVelocityIndex)[j] = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsReleaseVelocityIndex)[j] = i;
		
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsPitchIndex)[j] = i;
		
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsPitchFormat)[j] = i;
		
		if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
		if(r == -1.) r = (float) DeftPitchbendRange;
		(*p_CsInstrument)[j].pitchbendrange = r;
		if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"pitchbendrange[%d] = %.3f\n",j,r);
		
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].rPitchBend.islogx = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].rPitchBend.islogy = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].rVolume.islogx = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].rVolume.islogy = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].rPressure.islogx = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].rPressure.islogy = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].rModulation.islogx = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].rModulation.islogy = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].rPanoramic.islogx = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].rPanoramic.islogy = i;
		
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsPitchBendStartIndex)[j] = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsVolumeStartIndex)[j] = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsPressureStartIndex)[j] = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsModulationStartIndex)[j] = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsPanoramicStartIndex)[j] = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsPitchBendEndIndex)[j] = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsVolumeEndIndex)[j] = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsPressureEndIndex)[j] = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsModulationEndIndex)[j] = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsPanoramicEndIndex)[j] = i;
		if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"PanoramicEndIndex = %d\n",i);

		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].pitchbendtable = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].volumetable = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].pressuretable = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].modulationtable = i;
		if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
		(*p_CsInstrument)[j].panoramictable = i;
		if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"panoramictable = %d\n",i);
		
		if(iv > 13) {
			if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
			(*p_CsInstrument)[j].pitchbendGEN = i;
			if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
			(*p_CsInstrument)[j].volumeGEN = i;
			if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
			(*p_CsInstrument)[j].pressureGEN = i;
			if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
			(*p_CsInstrument)[j].modulationGEN = i;
			if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
			(*p_CsInstrument)[j].panoramicGEN = i;
			if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"panoramicGEN = %d\n",i);
			}
		else {
			(*p_CsInstrument)[j].pitchbendGEN = (*p_CsInstrument)[j].volumeGEN
				= (*p_CsInstrument)[j].pressureGEN = (*p_CsInstrument)[j].modulationGEN
				= (*p_CsInstrument)[j].panoramicGEN = 7;
			}

		for(i=0; i < 6; i++) {
			if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
			(*(p_CsPitchBend[i]))[j] = r;
			if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
			(*(p_CsVolume[i]))[j] = r;
			if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
			(*(p_CsPressure[i]))[j] = r;
			if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
			(*(p_CsModulation[i]))[j] = r;
			if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
			(*(p_CsPanoramic[i]))[j] = r;
			}
		
		if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"ipmax = %d j = %d\n",(*p_CsInstrument)[j].ipmax,j);
		
		for(ip=0; ip < (*p_CsInstrument)[j].ipmax; ip++) {
			if((*p_CsInstrument)[j].paramlist == NULL) {
				BPPrintMessage(0,odError,"=> Err. LoadCsoundInstruments(). (*p_CsInstrument)[j].paramlist == NULL");
				break;
				}
			ptr = (*((*p_CsInstrument)[j].paramlist))[ip].name;
			MyDisposeHandle((Handle*)&ptr);
			(*((*p_CsInstrument)[j].paramlist))[ip].name = NULL;
			ptr = (*((*p_CsInstrument)[j].paramlist))[ip].comment;
			MyDisposeHandle((Handle*)&ptr);
			(*((*p_CsInstrument)[j].paramlist))[ip].comment = NULL;
			}
		ptr3 = (*p_CsInstrument)[j].paramlist;
		MyDisposeHandle((Handle*)&ptr3);
		(*p_CsInstrument)[j].paramlist = NULL;
		(*p_CsInstrument)[j].ipmax = 0;
		
		if(ReadInteger(csfile,&ipmax,&pos) == MISSED) goto ERR;
		if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"Read ipmax again = %d\n",ipmax);
		if(ipmax < 1) continue;
		if((ptr3=(CsoundParam**) GiveSpace((Size)(ipmax * sizeof(CsoundParam)))) == NULL)
			goto ERR;
		(*p_CsInstrument)[j].paramlist = ptr3;
		(*p_CsInstrument)[j].ipmax = ipmax;
		for(ip=0; ip < ipmax; ip++) {
			(*((*p_CsInstrument)[j].paramlist))[ip].name = NULL;
			(*((*p_CsInstrument)[j].paramlist))[ip].comment = NULL;
			ResetMoreParameter(j,ip);
			}
		
		for(ip=0; ip < (*p_CsInstrument)[j].ipmax; ip++) {
			if(ReadOne(FALSE,TRUE,FALSE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
			length = MyHandleLen(p_completeline);
			if(length > 0) {
				if((ptr=(char**) GiveSpace((Size)((1L + length)
					* sizeof(char)))) == NULL) goto ERR;
				MystrcpyHandleToHandle(0,&ptr,p_completeline);
				(*((*p_CsInstrument)[j].paramlist))[ip].name = ptr;
				i = FixStringConstant(*p_completeline);
				if(i >= 0) (*((*p_CsInstrument)[j].paramlist))[ip].nameindex = i;
				}
			
			if(trace_load_csound_instruments) BPPrintMessage(0,odInfo,"-> Parameter: \"%s\"\n",*p_completeline);
		//	strcpy(line,"");
			fseek(csfile,pos,SEEK_SET);
			if(fgets(line,sizeof(line),csfile) != NULL) { // Necessary to read a line that might be empty
				remove_carriage_returns(line);
				pos = ftell(csfile);
				}
			else {
				BPPrintMessage(0,odError,"=> Error reading line in LoadCsoundInstruments()\n");
				goto ERR;
				}
			// fscanf(csfile, "%[^\n]",line2); Obsolete, not good for Windows
			if(strlen(line) > 0) {
				MystrcpyStringToHandle(&ptr,line);
				(*((*p_CsInstrument)[j].paramlist))[ip].comment = ptr;
			//	pos += strlen(line);
				}
			if(trace_load_csound_instruments) BPPrintMessage(0,odInfo, "This line: %s\n",line);
			if(ReadInteger(csfile,&i,&pos) == MISSED) {
				BPPrintMessage(0,odError, "Missed the first integer in LoadCsoundInstruments()\n");
				goto ERR;
				}
			(*((*p_CsInstrument)[j].paramlist))[ip].startindex = i;
			if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
			(*((*p_CsInstrument)[j].paramlist))[ip].endindex = i;
			if(iv > 12) {
				if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
				(*((*p_CsInstrument)[j].paramlist))[ip].table = i;
				if(ReadFloat(csfile,&r,&pos) == MISSED) goto ERR;
				(*((*p_CsInstrument)[j].paramlist))[ip].defaultvalue = r;
				if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
				(*((*p_CsInstrument)[j].paramlist))[ip].GENtype = i;
				if(ReadInteger(csfile,&i,&pos) == MISSED) goto ERR;
				(*((*p_CsInstrument)[j].paramlist))[ip].combinationtype = i;
				if(i == MULT && fabs(r) < 0.01) {
					(*((*p_CsInstrument)[j].paramlist))[ip].defaultvalue = 1.;
					my_sprintf(Message,"In instrument %ld a default parameter value '%.3f' was replaced with '1' because its combination mode is multiplicative",
						(long)(*p_CsInstrumentIndex)[j],r);
					BPPrintMessage(0,odError,"%s",Message);
					}
				}
			else {
				(*((*p_CsInstrument)[j].paramlist))[ip].table = -1;
				(*((*p_CsInstrument)[j].paramlist))[ip].defaultvalue = 0.;
				(*((*p_CsInstrument)[j].paramlist))[ip].GENtype = 7;
				(*((*p_CsInstrument)[j].paramlist))[ip].combinationtype = ADD;
				}
			}
		}
	if(j > 0) BPPrintMessage(0,odInfo,"%d Csound instrument(s) found\n",j);
	else BPPrintMessage(0,odInfo,"No Csound instrument was found. The default one will be used.\n");
	if(ReadOne(FALSE,FALSE,TRUE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto QUIT;
	result = OK;
	i_table = 0;
	if(Mystrcmp(p_line,"_begin tables") == 0) {
		while(TRUE) {
			if(ReadOne(FALSE,FALSE,TRUE,csfile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto QUIT;
			Strip(*p_line);
			if(strlen(*p_line) == 0) goto QUIT; // Required because 'pos' is not incremented when reading an empty line
			if(Mystrcmp(p_line,"_end tables") == 0) break;
			if(trace_load_csound_instruments) BPPrintMessage(0,odInfo, "table line = [%s]\n",*p_line);
			if(i_table >= MaxCsoundTables) {
				p_CsoundTables = (char****) IncreaseSpace((Handle)p_CsoundTables);
				MaxCsoundTables = (MyGetHandleSize((Handle)p_CsoundTables) / sizeof(char**));
				for(i = i_table; i < MaxCsoundTables; i++) (*p_CsoundTables)[i] = NULL;
				}
			length = MyHandleLen(p_completeline);
			if(length > 0) {
				if((ptr=(char**) GiveSpace((Size)((1L + length) * sizeof(char)))) == NULL) goto ERR;
				MystrcpyHandleToHandle(0,&ptr,p_completeline);
				(*p_CsoundTables)[i_table] = ptr;
				i_table++;
				}
			}
		}
	goto QUIT;

	ERR:
	BPPrintMessage(0,odError,"=> Error reading Csound instruments file:  %s\n",FileName[wCsoundResources]);

	QUIT:
	MyDisposeHandle((Handle*)&p_line);
	MyDisposeHandle((Handle*)&p_completeline);
	my_fclose(csfile);
	if(result == OK) {
		Created[wCsoundResources] = TRUE;
		LoadedCsoundInstruments = TRUE;
	/*	if(FALSE && NumberScales == 1) {
			BPPrintMessage(0,odInfo, "\nThis microtonal scale will be used for Csound scores in replacement of the equal-tempered 12-tone scale\nPitch will be adjusted to the diapason\n");
			DefaultScaleParam = -1;
			}
		else */
		DefaultScaleParam = -1; // Don't use scales until the _scale() instruction has been found
		}
	else {
		Created[wCsoundResources] = FALSE;
		BPPrintMessage(0,odError, "=> Error reading Csound instruments file:  %s\n", FileName[wCsoundResources]);
		EmergencyExit = TRUE;
		}
	if(iCsoundInstrument >= Jinstr) iCsoundInstrument = 0;
	// SetCsoundInstrument(iCsoundInstrument,-1);
	LoadOn--;
	return(result);
	}

char* read_file(const char *filename) {
	// Load the content of a text file as a single string
    FILE *file = fopen(filename, "r");
    if(!file) {
        perror("Unable to open file");
        return NULL;
    	}

    // Get the file size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);
    // Allocate memory and read the file
    char *data = (char *)malloc(filesize + 1);
    if(!data) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
   		}
    fread(data, 1, filesize, file);
    data[filesize] = '\0';
    fclose(file);
    return data;
	}


int LoadSettings(const char *filename, int startup) {
	int i,j,jmax,rep,result,iv,w,wmax,oldoutmidi,oldoutcsound,oldwritemidifile,intvalue;
	FILE* sefile;
	float floatvalue;
	long pos,k;
	unsigned long kk;
	double x;
	char **p_line,**p_completeline;

	result = OK;
	oldoutmidi = rtMIDI;
	p_line = p_completeline = NULL;
	if(startup) {
		// FIXME: set filename = location of a startup settings file and continue? We'll see later (BB)
		return OK;
		}
	else {
		// filename cannot be NULL or empty
		if(filename == NULL || filename[0] == '\0') {
			BPPrintMessage(0,odError,"=> Err. LoadSettings(): filename was NULL or empty\n");
			return MISSED;
			}
		}
	EndFadeOut = 2.; C4key = 60; A4freq = 440.;
	Quantize = TRUE;
	Quantization = 10L; // milliseconds
	UseBullet = FALSE;
	Code[7] = '.';
	MIDIfileType = 1;
	CsoundFileFormat = UNIX;
	StrikeAgainDefault = TRUE;
	DeftVolume = DEFTVOLUME;
	VolumeController = VOLUMECONTROL;
	DeftVelocity = DEFTVELOCITY;
	DeftPanoramic = DEFTPANORAMIC;
	PanoramicController = PANORAMICCONTROL;
	SamplingRate = SAMPLINGRATE;
	DefaultBlockKey = 60;
	ShowObjectGraph = TRUE;
	ShowAllObjects = FALSE;
	ShowPianoRoll = FALSE;
	Token = Oms = FALSE;
	UseBufferLimit = FALSE;
//	OutCsound = FALSE;
	CsoundTrace = FALSE;
// 	WriteMIDIfile = FALSE;
	NoConstraint = FALSE;
	ResetControllers = FALSE;
	FileSaveMode = ALLSAME;
	FileWriteMode = NOW;
	ProgNrFrom = 1;
	NotSaidKpress = TRUE;
	NeverResetWeights = FALSE;
	ResetFlags = FALSE;
	ResetWeights = FALSE;
	NeverResetWeights = FALSE;
	MinPeriod = 0;
	MaxConsoleTime = 0; // seconds (not used)

	if(OutCsound) MIDIsetUpTime = 0;

   	char *json_data = read_file(filename);
    if(!json_data) {
		BPPrintMessage(0,odError,"=> Could not open settings file %s\n", filename);
		return MISSED;
		}
	cJSON *json = cJSON_Parse(json_data);
    if(!json) {
        BPPrintMessage(0,odError,"=> Could not parse JSON settings: %s\n",cJSON_GetErrorPtr());
		free(json_data);
        return MISSED;
    	}
    cJSON *current_element = NULL;
    cJSON_ArrayForEach(current_element,json) {
		if(!cJSON_IsObject(current_element)) continue;
		const char *key = current_element->string; // Get the key name (e.g., "Quantization")
		if(key == NULL) continue;
		cJSON *value_field = cJSON_GetObjectItem(current_element,"value");
		if(value_field == NULL) {
			BPPrintMessage(0,odError, "=> Key: %s has no 'value' field\n",key);
			continue;
			}
		if(cJSON_IsString(value_field)) {
			const char *string_value = cJSON_GetStringValue(value_field);
		//	BPPrintMessage(0, odInfo, "cJSON_IsString %s: %s\n", key, string_value);
			intvalue = atoi(string_value);
			floatvalue = strtof(string_value, NULL);
			}
		else if(cJSON_IsNumber(value_field)) {
			intvalue = cJSON_GetNumberValue(value_field);
			floatvalue = cJSON_GetNumberValue(value_field);
		//	BPPrintMessage(0, odInfo, "cJSON_IsNumber %s: %f\n", key, intvalue);
			}
		else {
			BPPrintMessage(0, odInfo, "%s: Unsupported 'value' type\n", key);
			continue;
			}
		if(strcmp(key,"Quantization") == 0) Quantization = (long) intvalue;
		else if(strcmp(key,"Quantize") == 0) Quantize = intvalue;
		else if(strcmp(key,"Time_res") == 0) Time_res = (long) intvalue;
		else if(strcmp(key,"MIDIsyncDelay") == 0) MIDIsyncDelay = intvalue;
		else if(strcmp(key,"Nature_of_time") == 0) Nature_of_time = intvalue;
		else if(strcmp(key,"Pclock") == 0) Pclock = (double) intvalue;
		else if(strcmp(key,"Qclock") == 0) Qclock = (double) intvalue;
		else if(strcmp(key,"Improvize") == 0) Improvize = intvalue;
		else if(strcmp(key,"Max_items_produced") == 0) MaxItemsProduce = intvalue;
		else if(strcmp(key,"MaxItemsProduce") == 0) UseEachSub = intvalue;
		else if(strcmp(key,"AllItems") == 0) AllItems = intvalue;
		else if(strcmp(key,"DisplayProduce") == 0) DisplayProduce = intvalue;
		else if(strcmp(key,"StepProduce") == 0) StepProduce = intvalue;
		else if(strcmp(key,"TraceMicrotonality") == 0) TraceMicrotonality = intvalue;
		else if(strcmp(key,"TraceProduce") == 0) TraceProduce = intvalue;
		else if(strcmp(key,"PlanProduce") == 0) PlanProduce = intvalue;
		else if(strcmp(key,"DisplayItems") == 0) DisplayItems = intvalue;
		else if(strcmp(key,"ShowGraphic") == 0) ShowGraphic = intvalue;
		else if(strcmp(key,"ShowAllObjects") == 0) ShowAllObjects = intvalue;
		else if(strcmp(key,"AllowRandomize") == 0) AllowRandomize = intvalue;
		else if(strcmp(key,"DisplayTimeSet") == 0) DisplayTimeSet = intvalue;
		else if(strcmp(key,"StepTimeSet") == 0) StepTimeSet = intvalue;
		else if(strcmp(key,"TraceTimeSet") == 0) TraceTimeSet = intvalue;
		else if(strcmp(key,"TraceNoteOn") == 0) TraceNoteOn = intvalue;
		else if(strcmp(key,"CsoundTrace") == 0) CsoundTrace = intvalue;
		else if(strcmp(key,"ResetNotes") == 0) ResetNotes = intvalue;
		else if(strcmp(key,"ComputeWhilePlay") == 0) ComputeWhilePlay = intvalue;
		else if(strcmp(key,"AdvanceTime") == 0) AdvanceTime = 1000. * floatvalue; // Milliseconds
		else if(strcmp(key,"TraceMIDIinteraction") == 0) TraceMIDIinteraction = intvalue;
		else if(strcmp(key,"ResetWeights") == 0) ResetWeights = intvalue;
		else if(strcmp(key,"ResetFlags") == 0) ResetFlags = intvalue;
		else if(strcmp(key,"ResetControllers") == 0) ResetControllers = intvalue;
		else if(strcmp(key,"NoConstraint") == 0) NoConstraint = intvalue;
		else if(strcmp(key,"SplitTimeObjects") == 0) SplitTimeObjects = intvalue;
		else if(strcmp(key,"Split_|SplitVariables|") == 0) SplitVariables = intvalue;
		else if(strcmp(key,"DeftBufferSize") == 0) DeftBufferSize = (long) intvalue;
		else if(strcmp(key,"MaxConsoleTime") == 0) MaxConsoleTime = (long) intvalue;
		else if(strcmp(key,"Seed") == 0) Seed = (unsigned) (((long) intvalue) % 32768L);
		else if(strcmp(key,"NoteConvention") == 0) NoteConvention = intvalue;
		else if(strcmp(key,"GraphicScaleP") == 0) GraphicScaleP = intvalue;
		else if(strcmp(key,"GraphicScaleQ") == 0) GraphicScaleQ = intvalue;
		else if(strcmp(key,"EndFadeOut") == 0) EndFadeOut = floatvalue;
		else if(strcmp(key,"C4key") == 0) C4key = intvalue;
		else if(strcmp(key,"A4freq") == 0) A4freq = floatvalue;
		else if(strcmp(key,"StrikeAgainDefault") == 0) StrikeAgainDefault = intvalue;
		else if(strcmp(key,"DeftVolume") == 0) DeftVolume = intvalue;
		else if(strcmp(key,"VolumeController") == 0) VolumeController = intvalue;
		else if(strcmp(key,"DeftVelocity") == 0) DeftVelocity = intvalue;
		else if(strcmp(key,"DeftPanoramic") == 0) DeftPanoramic = intvalue;
		else if(strcmp(key,"PanoramicController") == 0) PanoramicController = intvalue;
		else if(strcmp(key,"SamplingRate") == 0) SamplingRate = intvalue;
		else if(strcmp(key,"StopPauseContinue") == 0) StopPauseContinue = intvalue;
		else if(strcmp(key,"DefaultBlockKey") == 0) DefaultBlockKey = intvalue;
		else if(strcmp(key,"B#_instead_of_C") == 0) NameChoice[0] = intvalue;
		else if(strcmp(key,"Db_instead_of_C#") == 0) NameChoice[1] = intvalue;
		else if(strcmp(key,"Eb_instead_of_D#") == 0) NameChoice[3] = intvalue;
		else if(strcmp(key,"Fb_instead_of_E") == 0) NameChoice[4] = intvalue;
		else if(strcmp(key,"E#_instead_of_F") == 0) NameChoice[5] = intvalue;
		else if(strcmp(key,"Gb_instead_of_F#") == 0) NameChoice[6] = intvalue;
		else if(strcmp(key,"Ab_instead_of_G#") == 0) NameChoice[8] = intvalue;
		else if(strcmp(key,"Bb_instead_of_A#") == 0) NameChoice[10] = intvalue;
		else if(strcmp(key,"Cb_instead_of_B") == 0) NameChoice[11] = intvalue;
		else if(strcmp(key,"ShowObjectGraph") == 0) ShowObjectGraph = intvalue;
		else if(strcmp(key,"ShowPianoRoll") == 0) ShowPianoRoll = intvalue;
		else if(strcmp(key,"MinPeriod") == 0) MinPeriod = intvalue; // For processing MIDI data capture
		}
	if(DeftBufferSize < 100) DeftBufferSize = 1000;
	if(rtMIDI && !ComputeWhilePlay && (AdvanceTime <= 0.)) {
		AdvanceTime = 0.;
		ComputeWhilePlay = 1;
		BPPrintMessage(0,odError,"=> Compute while playing has been set to TRUE because Max advance time = 0\n");
		}
	BufferSize = DeftBufferSize;
	SetTempo();
	if(Seed > 0) {
		if(!PlaySelectionOn) BPPrintMessage(0,odInfo,"Random seed = %u as per settings\n", Seed);
		ResetRandom();
		}
	else {
		if(!PlaySelectionOn) BPPrintMessage(0,odInfo,"Not using a random seed: shuffling the cards\n");
		Randomize();
		}
	if(WriteMIDIfile || rtMIDI) BPPrintMessage(0,odInfo,"Time resolution = %ld ms as per settings\n",Time_res);
	if(rtMIDI && !ComputeWhilePlay) {
		BPPrintMessage(0,odInfo,"Compute while playing is off\n");
		if(AdvanceTime > 0.) BPPrintMessage(0,odInfo,"➡ Advance time limit = %.2f seconds\n",AdvanceTime / 1000.);
		else BPPrintMessage(0,odInfo,"➡ No advance time allowed\n");
		}
	if(rtMIDI && StopPauseContinue) {
		BPPrintMessage(0,odInfo,"Stop/Pause/Continue messages will control the performance\n");
		}
	if(!Quantize) MaxDeltaTime = 20L;
	else MaxDeltaTime = 2 * Quantization;
	if(MaxItemsProduce < 2) MaxItemsProduce = 20;
	if(PlaySelectionOn) Improvize = FALSE;
	if(AllItems) Improvize = FALSE;
	if(ShowObjectGraph || ShowPianoRoll) ShowGraphic = TRUE;
	if(!ShowGraphic) ShowObjectGraph = ShowPianoRoll = FALSE;
	if(ShowPianoRoll) BPPrintMessage(0,odInfo,"Pianoroll graphics will be displayed\n");
	if(ShowObjectGraph) BPPrintMessage(0,odInfo,"Object graphics will be displayed\n");
	BPPrintMessage(0,odInfo,"Metronome will be %.3f beats/mn by default (as per settings)\n",(Qclock * 60.)/Pclock); 
	if(Nature_of_time == STRIATED) BPPrintMessage(0,odInfo,"Time is STRIATED\n");
	else BPPrintMessage(0,odInfo,"Time is SMOOTH (no metronome)\n");
    cJSON_Delete(json);
    free(json_data);

/* OBSOLETE: non-JSON settings file
	// open the file for reading
	sefile = my_fopen(1,filename, "rb");
	if(sefile == NULL) {
		BPPrintMessage(0,odError, "=> Could not open settings file %s\n", filename);
		return MISSED;
	    }

	pos = ZERO; Dirty[iSettings] = Created[iSettings] = FALSE;
	if(ReadOne(FALSE,FALSE,FALSE,sefile,TRUE,&p_line,&p_completeline,&pos) != OK) {
        goto ERR;
        }
	if(trace_load_settings) 
		BPPrintMessage(0,odInfo, "Version line = %s\n",*p_completeline);
	if(CheckVersion(&iv,p_line,filename) != OK) {
		result = MISSED;
		goto QUIT;
		}
	if(trace_load_settings) BPPrintMessage(0,odError, "Settings file version %d\n",iv);
	if(ReadOne(FALSE,FALSE,FALSE,sefile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
    if(trace_load_settings) BPPrintMessage(0,odError, "%s\n",*p_completeline);

	if(ReadInteger(sefile,&j,&pos) == MISSED) {
        BPPrintMessage(0,odError,"=> Error ReadInteger() in LoadSettings()\n");
        goto ERR;	// serial port used by old built-in Midi driver
        }
	if(ReadOne(FALSE,FALSE,TRUE,sefile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;	// Not used but should be kept for consistency
	if(ReadLong(sefile,&k,&pos) == MISSED) goto ERR; Quantization = k;
    if(trace_load_settings) BPPrintMessage(0,odError, "Quantization = %ld\n",k);
	if(ReadLong(sefile,&k,&pos) == MISSED) goto ERR; Time_res = k;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR; MIDIsetUpTime = j;
	if(trace_load_settings) BPPrintMessage(0,odError, "MIDIsyncDelay = %d\n",MIDIsyncDelay);
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR; Quantize = j;
	if(trace_load_settings) BPPrintMessage(0,odError, "Quantize = %d\n",Quantize);
	NotSaidKpress = TRUE;

	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR; Nature_of_time = j;
	if(ReadUnsignedLong(sefile,&kk,&pos) == MISSED) goto ERR; Pclock = (double)kk;
	if(ReadUnsignedLong(sefile,&kk,&pos) == MISSED) goto ERR; Qclock = (double)kk;
	SetTempo(); Dirty[wMetronom] = Dirty[wTimeBase] = FALSE;

	if(ReadInteger(sefile,&jmax,&pos) == MISSED) goto ERR;
	if(jmax != Jbutt) {
		my_sprintf(Message,"\nError in settings file:  jmax = %d instead of %d\n",jmax,Jbutt);
		BPPrintMessage(0,odError,"%s",Message);
		goto ERR;
		}
		
	oldwritemidifile = WriteMIDIfile;
	oldoutcsound = OutCsound;

	if(ReadInteger(sefile,&Improvize,&pos) == MISSED) goto ERR;
	if(trace_load_settings) BPPrintMessage(0,odError, "Improvize = %d\n",Improvize);
	if(PlaySelectionOn) Improvize = FALSE;
	if(ReadInteger(sefile,&MaxItemsProduce,&pos) == MISSED) goto ERR;
	if(MaxItemsProduce < 2) MaxItemsProduce = 20;
	if(ReadInteger(sefile,&UseEachSub,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&AllItems,&pos) == MISSED) goto ERR;
	if(AllItems) Improvize = FALSE;
	if(ReadInteger(sefile,&DisplayProduce,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&StepProduce,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&TraceMicrotonality,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&TraceProduce,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&PlanProduce,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&DisplayItems,&pos) == MISSED) goto ERR; 
	if(ReadInteger(sefile,&ShowGraphic,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&AllowRandomize,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&DisplayTimeSet,&pos) == MISSED) goto ERR; 
	if(ReadInteger(sefile,&StepTimeSet,&pos) == MISSED) goto ERR; 
	if(ReadInteger(sefile,&TraceTimeSet,&pos) == MISSED) goto ERR; 
	if(jmax > 27) ReadInteger(sefile,&CsoundTrace,&pos);
	else CsoundTrace = FALSE;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&ResetNotes,&pos) == MISSED) goto ERR; 
	if(ReadInteger(sefile,&ComputeWhilePlay,&pos) == MISSED) goto ERR; 
	if(ReadInteger(sefile,&TraceMIDIinteraction,&pos) == MISSED) goto ERR; // Previously it was 'Interactive'
	if(jmax > 19) ReadInteger(sefile,&ResetWeights,&pos);
	else ResetWeights = FALSE;
	NeverResetWeights = FALSE;
	if(jmax > 20) ReadInteger(sefile,&ResetFlags,&pos);
	else ResetFlags = FALSE;
	if(jmax > 21) ReadInteger(sefile,&ResetControllers,&pos);
	else ResetControllers = FALSE; 
	if(jmax > 22) ReadInteger(sefile,&NoConstraint,&pos);
	else NoConstraint = FALSE;
	if(jmax > 23) ReadInteger(sefile,&WriteMIDIfile,&pos);
	else WriteMIDIfile = FALSE;
	WriteMIDIfile = FALSE; // This must be set by the command line
	if(jmax > 24) ReadInteger(sefile,&ShowMessages,&pos); 
	if(jmax > 25) ReadInteger(sefile,&OutCsound,&pos);
	else OutCsound = FALSE;
	if(jmax > 26) ReadInteger(sefile,&j,&pos); // used to read p_oms
	Oms = FALSE;	// OMS is no more
	if(ReadInteger(sefile,&SplitTimeObjects,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&SplitVariables,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	// UseTextColor = (j > 0);
	if(ReadLong(sefile,&k,&pos) == MISSED) goto ERR;
	if(k < 100) k = 1000;
	DeftBufferSize = BufferSize = k;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&UseBufferLimit,&pos) == MISSED) goto ERR;
	UseBufferLimit = FALSE;
	if(ReadLong(sefile,&MaxConsoleTime,&pos) == MISSED) goto ERR;
	if(MaxConsoleTime > 3600) MaxConsoleTime = 3600;
    
	if(ReadLong(sefile,&k,&pos) == MISSED) goto ERR;
	Seed = (unsigned) (k % 32768L);
	if(Seed > 0) {
		if(!PlaySelectionOn) BPPrintMessage(0,odInfo, "Random seed = %u as per settings\n", Seed);
		ResetRandom();
		}
	else {
		if(!PlaySelectionOn) BPPrintMessage(0,odInfo, "Not using a random seed: shuffling the cards\n");
		Randomize();
		}

	if(ReadInteger(sefile,&Token,&pos) == MISSED) goto ERR;
	if(Token > 0) Token = TRUE;
	else Token = FALSE;
	if(ReadInteger(sefile,&NoteConvention,&pos) == MISSED) goto ERR;
	if(NoteConvention > (KEYS + 1)) {
		BPPrintMessage(0,odInfo, "\n=> ERROR NoteConvention = %d\n",NoteConvention);
		goto ERR;
		}
	if(ReadInteger(sefile,&StartFromOne,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	// SmartCursor = (j == 1);
	if(ReadInteger(sefile,&GraphicScaleP,&pos) == MISSED) goto ERR;
	if(ReadInteger(sefile,&GraphicScaleQ,&pos) == MISSED) goto ERR;

	// Read OMS default input device, and ignore it
	if(ReadOne(FALSE,FALSE,TRUE,sefile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	// Read OMS default output device, and ignore it
	if(iv > 5) {
		if(ReadOne(FALSE,FALSE,TRUE,sefile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
		}

	if(iv > 11) {
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		if(iv > 15) UseBullet = j;
		else UseBullet = TRUE;
		UseBullet = FALSE; // FIXED by BB 2020-10-22
	//	if(UseBullet) Code[7] = '�'; Requires UTF8 format BB 2022-02-17
	//	else
		Code[7] = '.';
		}

	PlayTicks = FALSE;

	if(iv > 7) {
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		PlayTicks = j;
		}
	if(iv > 10) {
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		FileSaveMode = ALLSAME;  // was = j;
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		FileWriteMode = NOW;     // was = j;
		}
	else {
		FileSaveMode = ALLSAME;  // was = ALLSAMEPROMPT;
		FileWriteMode = NOW;     // was = LATER;
		}
	if(iv > 11) {
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		MIDIfileType = j;
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		CsoundFileFormat = j;
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		ProgNrFrom = j;
		if(ProgNrFrom == 0) {
			ProgNrFrom = 1;
			}
		if(ReadFloat(sefile,&x,&pos) == MISSED) goto ERR;
		if(iv > 19) EndFadeOut = x;
		else EndFadeOut = 2.;
		my_sprintf(Message,"EndFadeOut = %.2f sec\n",EndFadeOut);
	//	BPPrintMessage(0,odInfo,Message);
		
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		if(j > 1 && j < 128) C4key = j;
		else C4key = 60;
		ReadFloat(sefile,&x,&pos);
		if(x > 1.) A4freq = x;
		else A4freq = 440.;
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		StrikeAgainDefault = j;
		}
	else {
		MIDIfileType = 1;
		CsoundFileFormat = UNIX;
		StrikeAgainDefault = TRUE;
		// C4key = 48;
		// A4freq = 220.;
		C4key = 60;
		A4freq = 440.0;
		}
	if(iv > 15) {
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		DeftVolume = j;
	//	BPPrintMessage(0,odInfo,"Default volume = %d\n",j);
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		VolumeController = j;
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		DeftVelocity = j;
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		DeftPanoramic = j;
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		PanoramicController = j;
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		SamplingRate = j;
	//	BPPrintMessage(0,odInfo,"Sampling rate = %d\n",j);
		}
	else {
		DeftVolume = DEFTVOLUME;
		VolumeController = VOLUMECONTROL;
		DeftVelocity = DEFTVELOCITY;
		DeftPanoramic = DEFTPANORAMIC;
		PanoramicController = PANORAMICCONTROL;
		SamplingRate = SAMPLINGRATE;
		}

	// This block reads in font sizes for Carbon GUI text windows
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	wmax = j;
	// BPPrintMessage(0,odInfo,"Number of windows = %d\n",j);
	if(wmax > 0) {
		for(w=0; w < (wmax - 1); w++) {
			if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
			}
		}
	if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
	if(j <= 10 || j > 127) DefaultBlockKey = 60;
	else DefaultBlockKey = j;

	// ResetMIDIFilter();

	if(iv > 4) {
		if(ReadLong(sefile,&k,&pos) == MISSED) goto ERR;
		for(i=0; i < 12; i++) {
			if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
			NameChoice[i] = j;
		//	BPPrintMessage(0,odInfo,"NameChoice[%d] = %d\n",i,j);
			}
		}
	if(ReadLong(sefile,&k,&pos) == MISSED) goto ERR;

	if(iv > 19) {
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		ShowObjectGraph = j;
		if(ReadInteger(sefile,&j,&pos) == MISSED) goto ERR;
		ShowPianoRoll = j;
		// THIS IS WHERE THE SETTINGS FILE ENDS NOW IN BP3
		}
	else {
		ShowObjectGraph = TRUE;
		ShowPianoRoll = FALSE;
		}

	if(ShowObjectGraph || ShowPianoRoll) ShowGraphic = TRUE;
	if(!ShowGraphic) ShowObjectGraph = ShowPianoRoll = FALSE;
	if(ShowPianoRoll) BPPrintMessage(0,odInfo,"Pianoroll graphics will be displayed\n");
	if(ShowObjectGraph) BPPrintMessage(0,odInfo,"Object graphics will be displayed\n");
	BPPrintMessage(0,odInfo,"Metronome will be %.3f beats/mn by default (as per settings)\n",(Qclock * 60.)/Pclock); 
	if(Nature_of_time == STRIATED) BPPrintMessage(0,odInfo,"Time is STRIATED\n");
	else BPPrintMessage(0,odInfo,"Time is SMOOTH (no metronome)\n");

	if(PlaySelectionOn) Improvize = 0;			

	goto QUIT;

	ERR:
	result = MISSED;
	BPPrintMessage(0,odError,"=> Error reading '%s' settings file\n",filename);

	QUIT:
	MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
	my_fclose(sefile); */

	return(result);
	}


int LoadObjectPrototypes(int checkversion,int tryname) {
	char c,date[80],*newp,*name_of_file = NULL, *final_name = NULL;
	MIDIcode **p_b;
	char **p_line,**p_completeline,line[MAXLIN],line2[MAXLIN];
	int i,iv,j,jj,co,rep,okt1,diff,stop,maxsounds,s,objecttype,oldjbol,notsaid,
		pivbeg,pivend,pivbegon,pivendoff,pivcent,pivcentonoff,pivspec,newbols,okrescale,
		compilemem,newinstruments,type,dirtymem,longerCsound,result;
	long t,t1,t2,tm,d,kres;
	long pos,imax;
	long k,kk;
	double r;
	short refnum,refnum2;
	Handle h,ptr;
	FILE* sofile;

	CompileAlphabet();

	if(check_memory_use) BPPrintMessage(0,odInfo,"MemoryUsed start LoadObjectPrototypes = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);

	rep = MISSED;
	newinstruments = CompiledCsObjects = 0;
	pos = 0L;
	line2[0] = '\0';
	// if(!tryname) FileName[iObjects][0] = '\0';
	p_line = p_completeline = NULL;

	LoadOn++;

	sofile = my_fopen(1,FileName[iObjects], "r");
	if(sofile == NULL) {
		BPPrintMessage(0,odError, "=> Could not open prototypes file %s\n",FileName[iObjects]);
	//	return MISSED;
		return ABORT; // Fixed by BB 2022-02-18
		}

	if(ReadOne(FALSE,FALSE,FALSE,sofile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	my_sprintf(Message,"Loading %s...",FileName[iObjects]);
	ShowMessage(TRUE,wMessage,Message);
	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "Line = %s\n",*p_line);
	if(CheckVersion(&iv,p_line,FileName[iObjects]) != OK) goto ERR;
	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "Version = %d\n",iv);

	if(iv > 2) {
		if(ReadOne(FALSE,TRUE,FALSE,sofile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	//	GetDateSaved(p_completeline,&(p_FileInfo[iObjects]));
		}
	newbols = FALSE;

	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	PrototypeTickKey = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	PrototypeTickChannel = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	PrototypeTickVelocity = s;
	fseek(sofile,pos,SEEK_SET);
	if(fgets(line2,sizeof(line2),sofile) != NULL) { // Necessary to read a line that might be empty
		pos = ftell(sofile);
		remove_carriage_returns(line2);
		}
	else {
		BPPrintMessage(0,odError,"=> Error reading line2 in LoadObjectPrototypes()\n");
		goto ERR;
		}
	// fscanf(sofile, "%[^\n]",line2); Obsolete, not good for Windows
	remove_trailing_newline(line2);
	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "Line (empty?) = [%s]\n",line2);
	if(strlen(line2) > 5) { // Fixed by BB 2022-02-18
		// Note that line2 contains the path (csound_resources)
		my_sprintf(Message,"../%s",line2);
		strcpy(FileName[wCsoundResources], Message);
		BPPrintMessage(0,odInfo, "Trying to load Csound instruments: %s\n", FileName[wCsoundResources]);
		if((result = LoadCsoundInstruments(0,1)) != OK) return(result);
	//	pos += strlen(line2);
		}
MAXSOUNDS:
	if(ReadInteger(sofile,&s,&pos) == MISSED) {
		BPPrintMessage(0,odError, "Missed the first integer in LoadObjectPrototypes()\n");
		goto ERR;
		}
	maxsounds = s;
	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "maxsounds = %d Jbol = %d\n",s,Jbol);
//	if(CheckTerminalSpace() != OK) goto ERR;
	oldjbol = Jbol;
	Jbol += maxsounds;
	if(ResizeObjectSpace(YES,Jbol + Jpatt,0) != OK) goto ERR;
//	if(CheckTerminalSpace() != OK) goto ERR;
	Jbol = oldjbol; newbols = TRUE;

	NumberTables = 0;
	rep = notsaid = OK;

	// Be careful: the following loop will read data forever if the file
	// was not properly closed...

NEXTBOL:
	PleaseWait(); j = -1;
	if(ReadOne(FALSE,TRUE,TRUE,sofile,TRUE,&p_line,&p_completeline,&pos) == MISSED) {
		my_sprintf(Message,"Unexpected end of '%s' file...  May be old version?",
			FileName[iObjects]);
		BPPrintMessage(0,odInfo,Message);
		goto ERR;
		}
	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "line = %s\n",*p_line);

	if(p_completeline == NULL) {
		BPPrintMessage(0,odError,"=> Err. LoadObjectPrototypes(). p_completeline == NULL");
		goto SORTIR;
		}
	if(MyHandleLen(p_completeline) < 1) goto SORTIR;
	if(Mystrcmp(p_completeline,"DATA:") == 0) {
		if(ReadOne(FALSE,TRUE,TRUE,sofile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
		if((*p_completeline)[0] == '\0')
			MystrcpyStringToHandle(&p_completeline,"[Comment on prototype file]");
		MystrcpyHandleToString(MAXFIELDCONTENT,0,line,p_completeline);
		if(trace_load_prototypes) BPPrintMessage(0,odInfo, "General comment = %s\n",line);
		//	rep = SetField(NULL,wPrototype1,fPrototypeFileComment,line);
	//	else SetField(NULL,wPrototype1,fPrototypeFileComment,"[Comment on prototype file]");
		goto SORTIR;
		}
	if(Mystrcmp(p_completeline,"_endSoundObjectFile_") == 0) goto SORTIR;
	if((iv > 3) && (Mystrcmp(p_completeline,"TABLES:") == 0)) {
		if(ReadInteger(sofile,&s,&pos) == MISSED) goto SORTIR;
		if(s > 0 && 0) {
			NumberTables = s;
			for(i=1; i <= s; i++) {
				/*  Read table i */
				}
			}
		goto SORTIR;
		}
	if(iv > 4 && newbols) {
	/*	BPPrintMessage(0,odInfo,"This sound-object has not been created because it is not in the alphabet: %s\n",*p_completeline); // 2024-08-18
		goto NEXTBOL; */

		oldjbol = Jbol;
		if(Jbol == 0) {
			Jbol = 2;
			BPPrintMessage(0,odError,"=> Warning: probable error in CreateBol()\n");
			GetAlphabetSpace();
			}
	//	p_Bol = (char****) GiveSpace((Size)(Jbol) * sizeof(char**));
		if((jj=CreateBol(0,0,p_completeline,0,0,BOL)) < 0) goto ERR;
		if(jj >= Jbol) {
			BPPrintMessage(0,odError,"=> Err. LoadObjectPrototypes(). jj >= Jbol");
			goto ERR;
			}
		j = jj;
		if(trace_load_prototypes) BPPrintMessage(0,odInfo, "Trying to create sound-object for j = %d named %s\n",j,*p_completeline);
		if(Jbol > oldjbol) {
			Jbol = oldjbol;
			BPPrintMessage(0,odInfo,"This sound-object has not been created because it is not in the alphabet: %s\n",*p_completeline);
			}
		}

	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "Final Jbol = %d\n",Jbol);
	if(ReadInteger(sofile,&objecttype,&pos) == MISSED) goto ERR;
	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "object type = %d\n",objecttype);
	(*p_Type)[j] = objecttype;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_Resolution)[j] = s;
	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "(*p_Resolution)[%d] = %d\n",j,(*p_Resolution)[j]);
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_DefaultChannel)[j] = s;
	if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
	(*p_Tref)[j] = ((long) k * (*p_Resolution)[j]);
	if(ReadFloat(sofile,&r,&pos) == MISSED) goto ERR;
	(*p_Quan)[j] = r;
	if(ReadOne(FALSE,FALSE,TRUE,sofile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	if(trace_load_prototypes) BPPrintMessage(0,odInfo,"line3 = %s\n",*p_line);
	i = 0;
	pivbeg = (*p_line)[i++]-'0';
	pivend = (*p_line)[i++]-'0';
	pivbegon = (*p_line)[i++]-'0';
	pivendoff = (*p_line)[i++]-'0';
	pivcent = (*p_line)[i++]-'0';
	pivcentonoff = (*p_line)[i++]-'0';
	okrescale = (*p_line)[i++]-'0';
	(*p_FixScale)[j] = (*p_line)[i++]-'0';
	(*p_OkExpand)[j] = (*p_line)[i++]-'0';
	(*p_OkCompress)[j] = (*p_line)[i++]-'0';
	(*p_OkRelocate)[j] = (*p_line)[i++]-'0';
	(*p_BreakTempo)[j] = (*p_line)[i++]-'0';
	(*p_ContBeg)[j] = (*p_line)[i++]-'0';
	(*p_ContEnd)[j] = (*p_line)[i++]-'0';
	(*p_CoverBeg)[j] = (*p_line)[i++]-'0';
	(*p_CoverEnd)[j] = (*p_line)[i++]-'0';
	(*p_TruncBeg)[j] = (*p_line)[i++]-'0';
	(*p_TruncEnd)[j] = (*p_line)[i++]-'0';
	pivspec = (*p_line)[i++]-'0';
	(*p_PivType)[j] = pivbeg + 2 * pivend + 3 * pivbegon + 4 * pivendoff
		+ 5 * pivcent + 6 * pivcentonoff + 7 * pivspec;
	(*p_AlphaCtrl)[j] = (*p_line)[i++]-'0';

	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR; /* rescalemode */
	(*p_RescaleMode)[j] = s;
	if(ReadFloat(sofile,&r,&pos) == MISSED) goto ERR;
	(*p_AlphaMin)[j] = r;
	if(ReadFloat(sofile,&r,&pos) == MISSED) goto ERR;
	(*p_AlphaMax)[j] = r;
	if(iv < 5) {	/* Fixing a bug in default values */
		(*p_AlphaMax)[j] = 10L;
		}
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR; /* delaymode */
	(*p_DelayMode)[j] = s;
	if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
	(*p_MaxDelay)[j] = k;
	if(iv > 4) {
		if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR; /* forwardmode */
		(*p_ForwardMode)[j] = s;
		}
	if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
	(*p_MaxForward)[j] = k;
	if(iv < 5) {	/* Fixing a bug in default values */
		(*p_MaxDelay)[j] = (*p_MaxForward)[j] = ZERO;
		}
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR; /* brktempomode */
	(*p_BreakTempoMode)[j] = s;
	if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
/*	(*p_MaxBreakTempo)[j] = k;
	if(iv < 5) {
		(*p_MaxBreakTempo)[j] = ZERO;
		} */
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR; /* contbegmode */
	(*p_ContBegMode)[j] = s;
	if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
	(*p_MaxBegGap)[j] = k;
	if(iv < 5) {	/* Fixing a bug in default values */
		(*p_MaxBegGap)[j] = ZERO;
		}
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR; /* contendmode */
	(*p_ContEndMode)[j] = s;
	if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
	(*p_MaxEndGap)[j] = k;
	if(iv < 5) {	/* Fixing a bug in default values */
		(*p_MaxEndGap)[j] = ZERO;
		}
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR; /* coverbegmode */
	(*p_CoverBegMode)[j] = s;
	if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
	(*p_MaxCoverBeg)[j] = k;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR; /* coverendmode */
	(*p_CoverEndMode)[j] = s;
	if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
	(*p_MaxCoverEnd)[j] = k;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR; /* truncbegmode */
	(*p_TruncBegMode)[j] = s;
	if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
	(*p_MaxTruncBeg)[j] = k;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR; /* truncendmode */
	(*p_TruncEndMode)[j] = s;
	if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
	(*p_MaxTruncEnd)[j] = k;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_PivMode)[j] = s;
	if(ReadFloat(sofile,&r,&pos) == MISSED) goto ERR;
		(*p_PivPos)[j] = r;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_AlphaCtrlNr)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_AlphaCtrlChan)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_OkTransp)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_OkArticul)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_OkVolume)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_OkPan)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_OkMap)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
		(*p_OkVelocity)[j] = s;
	if(ReadFloat(sofile,&r,&pos) == MISSED) goto ERR;
	(*p_PreRoll)[j] = r;
	if(ReadFloat(sofile,&r,&pos) == MISSED) goto ERR;
	(*p_PostRoll)[j] = r;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_PreRollMode)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_PostRollMode)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_PeriodMode)[j] = s;
	if(ReadFloat(sofile,&r,&pos) == MISSED) goto ERR;
	(*p_BeforePeriod)[j] = r;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_ForceIntegerPeriod)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_DiscardNoteOffs)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_StrikeAgain)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	(*p_CsoundAssignedInstr)[j] = s;
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
		(*p_CsoundInstr)[j] = s;
	if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
	(*p_Tpict)[j] = k;
	if(iv > 21) { // These are no longer used
		if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
		// (*p_ObjectColor)[j].red = k;
		if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
		// (*p_ObjectColor)[j].green = k;
		if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
		// (*p_ObjectColor)[j].blue = k;
		if(trace_load_prototypes) BPPrintMessage(0,odInfo, "(*p_ObjectColor)[j].blue = %ld\n",k);
		}
	(*pp_CsoundTime)[j] = NULL;
	(*p_CompiledCsoundScore)[j] = 0; // Added 2024-07-04
	(*p_CsoundSize)[j]= 0; // Added 2024-07-04
	(*pp_CsoundScore)[j] = NULL; // Added 2024-07-04
	(*pp_CsoundScoreText)[j] = NULL; // Added 2024-07-04

	// Read pp_CsoundScoreText
	if(ReadOne(FALSE,FALSE,TRUE,sofile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	// StripHandle(p_line);
	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "line2 = %s\n",*p_line);

	if(Mystrcmp(p_line,"_beginCsoundScore_") != 0) goto ERR;

	if(ReadOne(FALSE,FALSE,TRUE,sofile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;

	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "line3 = %s\n",*p_line);

	if((ptr = (Handle) GiveSpace(MyGetHandleSize((Handle)p_completeline))) == NULL) goto ERR;
		(*pp_CsoundScoreText)[j] = (char**) ptr;
	if(MystrcpyHandleToHandle(0,&((*pp_CsoundScoreText)[j]),p_completeline) != OK) goto ERR;

	/* if((rep=CompileObjectScore(j,&longerCsound)) != OK) {
		OutCsound = FALSE;
		goto ERR;
		} */
	Dirty[iObjects] = Dirty[wPrototype7] = FALSE;
	if(trace_load_prototypes) BPPrintMessage(0,odInfo,"Compiled Csound score\n");

	if(ReadOne(FALSE,TRUE,TRUE,sofile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "line4 = %s\n",*p_line);
	if(Mystrcmp(p_completeline,"_endCsoundScore_") == 0) goto READSIZE;
	else goto ERR;

	READSIZE:
	if(ReadInteger(sofile,&s,&pos) == MISSED) goto ERR;
	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "Size of MIDI code = %d\n",s);
	imax = s;
		(*p_PasteDone)[j] = FALSE;
		(*p_MIDIsize)[j] = (*p_Ifrom)[j] = ZERO;
		(*pp_MIDIcode)[j] = NULL;
	if(imax > 0) {
		if((p_b = (MIDIcode**) GiveSpace((Size)sizeof(MIDIcode)*(imax+1))) == NULL) goto ERR;
		rep = OK;
		}
	if((*p_CsoundSize)[j] == 0) (*p_Dur)[j] = t1 = t2 = ZERO;
	if(imax > 0) {
		for(i=0,tm=ZERO,okt1=FALSE; i < imax; i++) {
			if(ReadLong(sofile,&k,&pos) == MISSED) goto ERR;
		//	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "k = %ld\n",k);
			if(Eucl(k,256L,(unsigned long*)&t,(unsigned long*)&kk) != 0) goto ERR;
			(*p_b)[i].byte = (int) kk;
			if(143L < kk  &&  kk < 160L) {   /* NoteOn or NoteOff */
				t2 = t;
				if(!okt1) {
					okt1 = TRUE;
					t1 = t;
					}
				}
			(*p_b)[i].time = (Milliseconds) t;
			}
		imax = i;
		if(trace_load_prototypes) BPPrintMessage(0,odInfo, "imax = %d\n",imax);

		if(MIDItoPrototype(FALSE,TRUE,j,p_b,imax) != OK) goto ERR;
		if(MyDisposeHandle((Handle*)&p_b) != OK) goto ERR;
		}

	if(CheckConsistency(j,TRUE) != OK) goto ERR;
	if(trace_load_prototypes) BPPrintMessage(0,odInfo, "CheckConsistency is OK for j = %d\n",j);
	if(iv > 9) {
		if(ReadOne(FALSE,TRUE,TRUE,sofile,TRUE,&p_line,&p_completeline,&pos) == MISSED) goto ERR;
		if(p_completeline == NULL) {
			BPPrintMessage(0,odError,"=> Err. LoadObjectPrototypes(). p_completeline == NULL");
			goto ERR;
			}
		s = MyHandleLen(p_completeline);
		if(s > MAXFIELDCONTENT) {
			s = MAXFIELDCONTENT;
			(*p_completeline)[s] = '\0';
			}
		if((h = (Handle) GiveSpace((Size)(1+s) * sizeof(char))) == NULL) goto ERR;
		(*pp_Comment)[j] = (char**) h;
		MystrcpyHandleToHandle(0,&((*pp_Comment)[j]),p_completeline);
		if(trace_load_prototypes) BPPrintMessage(0,odInfo, "comment for %d = %s\n\n",j,*p_completeline);
		}
	goto NEXTBOL;

	ERR:
	if(j > 1 && j < Jbol) ResetPrototype(j);
	if(CheckEmergency() == OK) {
		rep = MISSED;
		BPPrintMessage(0,odError,"=> This sound-object file may be corrupted or in some unknown format\n");
		if(j > 1 && j < Jbol) {
			BPPrintMessage(0,odError,"An error occured while reading '%s'\n",*((*p_Bol)[j]));
			}
		else BPPrintMessage(0,odError,"The error did not occur while reading a sound-object\n");
		BPPrintMessage(0,odInfo,"You may load the sound-object file in a text editor and try to fix inconsistencies\n");
		}
	else rep = ABORT;

	SORTIR:
	LoadOn--;
	MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
	my_fclose(sofile);
	// if(CheckEmergency() != OK) return(ABORT);
	// // HideWindow(Window[wMessage]);

	ObjectMode = ObjectTry = TRUE;
	if(check_memory_use) BPPrintMessage(0,odInfo,"MemoryUsed end LoadObjectPrototypes = %ld i_ptr = %d\n",(long)MemoryUsed,i_ptr);

	// 
	if(rep == OK) {
	//	SetName(iObjects,YES,TRUE);
		if(newbols) {
			ResizeObjectSpace(NO,Jbol + Jpatt,0);
			// BPPrintMessage(0,odInfo,"Resizing object space for Jbol = %d and Jpatt = %ld\n",Jbol,Jpatt);
			}
		}
	PrototypesLoaded = TRUE;
	return(rep);
	}

void delay(int number_of_seconds) {
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
  
    // Storing start time
    clock_t start_time = clock();
  
    // looping till required time is not achieved
    while(clock() < start_time + milli_seconds);
	}