/* Tonality.c (BP3) */ 

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

int trace_each_scale = 0;

int CreateMicrotonalScale(char* line, char* name, char* note_names, char* key_numbers, char* fractions, char* baseoctave_string) {
	// "line" contains the scale as defined in Csound GEN51 format
	char c, curr_arg[MAXLIN], label[MAXLIN], this_note[MAXLIN], this_fraction_arg[MAXLIN], this_key[10];
	char** ptr;
	int i,n,i_note,key,j,k,pos,n_args,space,numgrades,baseoctave,basekey,numerator,num[100],den[100];
	double blockkey_temp_freq, pitch, temp_pitch;
	
	if(strlen(line) == 0 || line[0] != 'f') return(OK);
	n_args = 0;
	for(i = 0; i < MAXLIN; i++) curr_arg[i] = '\0';
	space = FALSE;
	for(pos = 0; pos < strlen(line); pos++) {
		if((c=line[pos]) == ' ') {
			if(!space) n_args++;
			else continue;
			space = TRUE;
			if(n_args == 4) {
				if(abs((int) atol(curr_arg)) != 51) return(EXIT); // This is a different table function
				NumberScales++;
				if((NumberScales + 3) > MAXCONVENTIONS) {
					BPPrintMessage(odError,"\n=> Too many scales! Maximum is %d\n",MAXCONVENTIONS);
					return(ABORT);
					}
				if(Scale == NULL) Scale = (t_scale**) GiveSpace((Size)(MaxScales * sizeof(t_scale)));
				else {
					if(NumberScales >= MaxScales) {
						Scale = (t_scale**) IncreaseSpace((Handle)Scale);
						MaxScales = MyGetHandleSize((Handle)Scale) / sizeof(t_scale);
						}
					}
				}
			if(n_args == 5) {
			//	BPPrintMessage(odInfo,"Scale %d (%s), n_args = %d\n",NumberScales,name,n_args);
				(*Scale)[NumberScales].deviation = (short**) GiveSpace((Size)(MAXKEY * sizeof(short)));
				}
			switch(n_args) {
				case 5:
					(*Scale)[NumberScales].numgrades = numgrades = (int) atol(curr_arg); /* Don't use atoi() because int's are 4 bytes */
					(*Scale)[NumberScales].tuningratio = (double**) GiveSpace((Size)((numgrades + 1) * sizeof(double)));
					(*Scale)[NumberScales].notenames = (char****) GiveSpace((Size)((numgrades + 1) * sizeof(char**)));
					(*Scale)[NumberScales].keys = (int**) GiveSpace((Size)((numgrades + 1) * sizeof(int)));
					for(i = 0; i <= numgrades; i++) num[i] = den[i] = 0;
					break;
				case 6: (*Scale)[NumberScales].interval = strtod(curr_arg,NULL); break;
				case 7: (*Scale)[NumberScales].basefreq = strtod(curr_arg,NULL); break;
				case 8: (*Scale)[NumberScales].basekey = basekey = (int) atol(curr_arg);
					if(trace_each_scale) BPPrintMessage(odInfo,"\nCreating key numbers for « %s » (scale %d):\n%s\n",name,NumberScales,key_numbers);
					if(strlen(key_numbers) > 0) {
						j = i_note = 0;
						for(i = 1; i < strlen(key_numbers); i++) {
							c = key_numbers[i];
							if(c == 'k') break;
							if(c != ' ' && c != 'k') this_key[j++] = c;
							else {
								this_key[j] = '\0';
								if(trace_each_scale) BPPrintMessage(odInfo,"Creating i_note = %d key = %d\n",i_note,atoi(this_key) - basekey);
								key = atoi(this_key);
								if(key >= basekey) key = key - basekey;
								(*((*Scale)[NumberScales].keys))[i_note] = key;
								j = 0; i_note++;
								}
							}
						}
					else {
						for(i_note = 0; i_note < numgrades; i_note++)
							(*((*Scale)[NumberScales].keys))[i_note] = i_note;
						}
					j = i_note = 0;
					if(trace_each_scale) BPPrintMessage(odInfo,"\nLoading note names for « %s » (scale %d):\n%s\n",name,NumberScales,note_names);
					for(i = 1; i < strlen(note_names); i++) {
						c = note_names[i];
						if(c == '/') break;
						if(c != ' ' && c != '/') this_note[j++] = c;
						else {
							this_note[j] = '\0';
							ptr = NULL;
							MystrcpyStringToHandle(&ptr,this_note);
				//			k = (*((*Scale)[NumberScales].keys))[i_note];
							if(trace_each_scale) BPPrintMessage(odInfo,"Creating i_note = %d this_note = « %s »\n",i_note,this_note);
							(*((*Scale)[NumberScales].notenames))[i_note] = ptr;
							j = 0; // k++;
							i_note++;
							}
						}
					if(strlen(fractions) > 0) {
						j = i_note = 0;
						numerator = TRUE;
						for(i = 1; i < strlen(fractions); i++) {
							c = fractions[i];
							if(c == ']') break;
							if(c != ' ' && c != ']') this_fraction_arg[j++] = c;
							else {
								this_fraction_arg[j] = '\0';
					//			if(trace_each_scale) BPPrintMessage(odInfo,"Creating i_note = %d key = %d\n",i_note,atoi(this_key) - basekey);
								if(numerator)
									num[i_note] = atoi(this_fraction_arg);
								else
									den[i_note++] = atoi(this_fraction_arg);
								numerator = 1 - numerator;
								j = 0;
								}
							}
						}
					break;
				default:
					if(n_args > 8) {
						if((n_args - 9) >= numgrades) {
							BPPrintMessage(odError,"\n=> This GEN51 function table is incorrect because it contains more than %d ratios:\n%s\n",numgrades,line);
							NumberScales--;
							return(OK);
							}
						(*((*Scale)[NumberScales].tuningratio))[n_args-9] = strtod(curr_arg,NULL);
						}
					break;
				}
			for(i = 0; i < MAXLIN; i++) curr_arg[i] = '\0';
			}
		else {
			space = FALSE;
			curr_arg[strlen(curr_arg)] = c; 
			}
		}
	n_args++;
	if((n_args - 8) < (numgrades + 1)) {
		BPPrintMessage(odError,"\n=> This GEN51 function table is incorrect because it contains %d ratios instead of %d:\n%s\n",(n_args - 8),(numgrades +1),line);
		NumberScales--;
		return(OK);
		}
	if(strlen(baseoctave_string) == 0) baseoctave = 4;
	else baseoctave = (int) atol(baseoctave_string);
	if(baseoctave <= 0 || baseoctave > 14) baseoctave = 4;
	if(trace_each_scale) BPPrintMessage(odInfo,"\nbaseoctave_string = « %s » (*Scale)[NumberScales].baseoctave = %d\n",baseoctave_string,baseoctave);
	(*Scale)[NumberScales].baseoctave = baseoctave;
	(*((*Scale)[NumberScales].tuningratio))[n_args-9] = strtod(curr_arg,NULL);
	if(strlen(name) > 0) my_sprintf(label,"%s",name);
	else my_sprintf(label,"scale_%d",NumberScales); // Name by default
	(*Scale)[NumberScales].label = (char**) GiveSpace((Size)(strlen(label) * sizeof(char)));
	MystrcpyStringToHandle(&((*Scale)[NumberScales].label),label);
	if(TraceMicrotonality) BPPrintMessage(odInfo,"👉 Microtonal scale \"%s\" loaded from Tonality resources (%d grades)\n",*((*Scale)[NumberScales].label),(*Scale)[NumberScales].numgrades);
	blockkey_temp_freq = (*Scale)[NumberScales].basefreq * exp((9. / 12) * log((*Scale)[NumberScales].interval));
	
	if(strlen(fractions) > 0) { // Use fractions to calculate ratios more accurately
		for(i = 0; i <= (*Scale)[NumberScales].numgrades; i++) {
			if(num[i] > 0 && den[i] > 0)
				(*((*Scale)[NumberScales].tuningratio))[i] = ((double) num[i]) / den[i];
			}
		}

//	BPPrintMessage(odInfo,"blockkey_temp_freq = %.3f\n",blockkey_temp_freq);
	for(key = 0; key < MAXKEY; key++) {
		// Fill the table hat will be used for MPE microtonality
		pitch = GetPitchWithScale(NumberScales, key, 0., 0);
		temp_pitch = (*Scale)[NumberScales].basefreq * pow(2,(double)(key - (*Scale)[NumberScales].basekey)/ 12.);
	//	BPPrintMessage(odInfo,"• key %d %.3f %.3f\n",key,pitch,temp_pitch);
		(*(*Scale)[NumberScales].deviation)[key] = (short) (1200. * log2(pitch/temp_pitch));
		if(TraceMicrotonality) BPPrintMessage(odInfo,"• deviation [%d] = %d cents\n",key,(*(*Scale)[NumberScales].deviation)[key]);
		}
	if(DefaultScale == -1) DefaultScale = NumberScales;
	
	if(TraceMicrotonality) {
		for(i = 0; i <= (*Scale)[NumberScales].numgrades; i++)
			BPPrintMessage(odInfo,"%.3f ",(*((*Scale)[NumberScales].tuningratio))[i]);
		if(strlen(note_names) > 0) BPPrintMessage(odInfo,"\nNames of notes in this scale: %s",note_names);
		BPPrintMessage(odInfo,"\nWith 'interval' = %.3f, 'basefreq' = %.3f Hz, 'basekey' = %d and 'baseoctave' = %d\n",(*Scale)[NumberScales].interval,(*Scale)[NumberScales].basefreq,(*Scale)[NumberScales].basekey,(*Scale)[NumberScales].baseoctave);
		BPPrintMessage(odInfo,"A4 frequency of a tempered scale with the same 'basefreq' and 'interval' would be %.3f Hz\n",blockkey_temp_freq);
		PrintNote(-1,BlockScaleOnKey,-1,-1,Message);
		BPPrintMessage(odInfo,"As per your settings, frequency will be blocked for note key #%d = « %s » but this may be changed in \"_scale(some_scale, blockkey)\" statements\n",BlockScaleOnKey,Message);
		}
	return(OK);
	}


double GetPitchWithScale(int i_scale, int key, double cents, int blockkey) {
	int this_key, this_block_key, note_class, octave, pitchclass, blockkey_pitch_class, numgrades, C4_octave, basekey, basefreq_octave, delta_key;
	double basefreq, pitch_ratio, fix_ratio_0, basekey_pitch_ratio, blockkey_pitch_ratio, blockkey_correction, diapason_correction, interval, x, x0, x1, x2;
	
	if(i_scale > NumberScales) { 
		BPPrintMessage(odError,"\n=> Scale number %d is out of range (maximum %d). No Csound score produced\n\n",i_scale,NumberScales);
		return(Infpos);
		}
	if(key < 0 || key > 127) {
		BPPrintMessage(odError,"\n=> Key #%d is out of range [0..127]. No Csound score produced\n\n",key);
		return(Infpos);
		}
	numgrades = (*Scale)[i_scale].numgrades; // Most often 12
	interval = (*Scale)[i_scale].interval; // Most often 2
	basekey = (*Scale)[i_scale].basekey; // Starting point of scale ratios  (most often 60)
	basefreq = (*Scale)[i_scale].basefreq; // Frequency of base key
	
	if(!ToldAboutScale && !TraceMicrotonality) {
		MystrcpyHandleToString(0,0,Message,(*Scale)[i_scale].label);
		BPPrintMessage(odInfo,"👉 Custom scale \"%s\" (and maybe more scales) used for microtonality\n",Message);
		ToldAboutScale = TRUE;
		}
	
	delta_key =  basekey - C4key; // C4key is defined in the settings (60 by default)
	
	C4_octave = floor(C4key / 12);
	basefreq_octave = floor((double)basekey / 12);
	
	pitchclass = modulo(key - basekey + delta_key, 12);
	octave = floor(((double)key - pitchclass) / 12);
	
	for(this_key = 0; this_key < numgrades; this_key++) {
		note_class = (*((*Scale)[i_scale].keys))[this_key];
		if(note_class == pitchclass) break;
		}
	if(this_key == numgrades) {
		this_key = 0;
		if(!WarnedRangeKey) {
			MystrcpyHandleToString(0,0,Message,(*Scale)[i_scale].label);
			BPPrintMessage(odInfo,"\n=> ERROR Pitch class ‘%d’ does not exist in _scale(%s) which has numgrades = %d. No Csound score produced\n",pitchclass,Message,numgrades);
			WarnedRangeKey = TRUE;
			return(Infpos);
			}
		}
	
	if(blockkey == 0) blockkey = BlockScaleOnKey;
	blockkey_pitch_class = modulo(blockkey - basekey, 12);
		
	if(blockkey_pitch_class == 0) 
		fix_ratio_0 = (*((*Scale)[i_scale].tuningratio))[0]; // Most often 1
	else fix_ratio_0 = 1;
	
	for(this_block_key = 0; this_block_key < numgrades; this_block_key++) {
		note_class = (*((*Scale)[i_scale].keys))[this_block_key];
		if(note_class == blockkey_pitch_class) break;
		}
	if(this_block_key == numgrades) {
		this_block_key = 0;
		if(!WarnedBlockKey) {
			MystrcpyHandleToString(0,0,Message,(*Scale)[i_scale].label);
			BPPrintMessage(odError,"=> ERROR choice of blockkey in _scale(%s,...) instruction\n",Message);
			WarnedBlockKey = TRUE;
			}
		}
		
	blockkey_pitch_ratio = (*((*Scale)[i_scale].tuningratio))[this_block_key];
	blockkey_correction = fix_ratio_0 * exp(((double)blockkey_pitch_class / 12) * log(interval)) / blockkey_pitch_ratio;
	
	pitch_ratio = (*((*Scale)[i_scale].tuningratio))[this_key];
	
	diapason_correction = A4freq / 440.;
	
	x = x0 = basefreq * pitch_ratio;
	x = x1 = x * blockkey_correction;
	x = x2 = x * diapason_correction;
	x = x * exp((octave - basefreq_octave) * log(interval));
	x = x * exp((cents / 1200.) * log(interval));
	
	if(trace_each_scale) BPPrintMessage(odInfo,"• key = %d, this_key = %d, C4key = %d, delta_key = %d basekey = %d basefreq = %.3f basefreq_octave = %d, numgrades = %d interval = %.3f, pitchclass = %d pitch_ratio = %.3f, fix_ratio_0 = %.3f, diapason_correction = %.4f blockkey = %d blockkey_pitch_class = %d this_block_key = %d, blockkey_pitch_ratio = %.3f  blockkey_correction = %.4f, octave = %d, x0 = %.3f x1 = %.3f x2 = %.3f x = %.3f\n",key,this_key,C4key,delta_key,(*Scale)[i_scale].basekey,basefreq,basefreq_octave,numgrades,interval,pitchclass,pitch_ratio,fix_ratio_0,diapason_correction,blockkey,blockkey_pitch_class,this_block_key,blockkey_pitch_ratio,blockkey_correction,octave,x0,x1,x2,x);
	return x;
	}

