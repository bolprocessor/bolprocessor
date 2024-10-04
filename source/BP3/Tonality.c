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
	char c, curr_arg[MAXLIN], label[MAXLIN], this_note[MAXLIN], this_fraction_arg[MAXLIN], note_name[20],key_number_string[20];
	char** ptr;
	int i,n,i_note,j_note,temp_note,key,this_key,octave,temp_octave,numnotes,note_key_class,keyclass,keyclass_near,j,k,pos,n_args,space,numgrades,baseoctave,basekey,numerator,delta_key,pitchclass,num[100],den[100];
	double blockkey_temp_freq, frequency, temp_pitch, pitch_ratio, interval;
	
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
					BPPrintMessage(0,odError,"\n=> Too many scales! Maximum is %d\n",MAXCONVENTIONS);
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
			//	BPPrintMessage(0,odInfo,"Scale %d (%s), n_args = %d\n",NumberScales,name,n_args);
				(*Scale)[NumberScales].deviation = (short**) GiveSpace((Size)(MAXKEY * sizeof(short)));
				(*Scale)[NumberScales].blockkey_shift = (short**) GiveSpace((Size)(MAXKEY * sizeof(short)));
				}
			switch(n_args) {
				case 5:
					(*Scale)[NumberScales].numgrades = numgrades = (int) atol(curr_arg); /* Don't use atoi() because integers are 4 bytes */
				//	BPPrintMessage(0,odError,"numgrades = %d\n",numgrades);
					(*Scale)[NumberScales].tuningratio = (double**) GiveSpace((Size)((numgrades + 1) * sizeof(double)));
					(*Scale)[NumberScales].notenames = (char****) GiveSpace((Size)((numgrades + 1) * sizeof(char**)));
					(*Scale)[NumberScales].keys = (int**) GiveSpace((Size)((numgrades + 1) * sizeof(int)));
					(*Scale)[NumberScales].keyclass = (int**) GiveSpace((Size)((numgrades + 1) * sizeof(int)));
					for(i = 0; i <= numgrades; i++) num[i] = den[i] = 0;
					break;
				case 6: (*Scale)[NumberScales].interval = interval = strtod(curr_arg,NULL); break;
				case 7: (*Scale)[NumberScales].basefreq = strtod(curr_arg,NULL); break;
				case 8: (*Scale)[NumberScales].basekey = basekey = (int) atol(curr_arg);
					if(TraceMicrotonality) 
						BPPrintMessage(0,odInfo,"\nCreating key numbers for « %s » (scale %d) with base key #%d:\n%s\n",name,NumberScales,basekey,key_numbers);
					if(strlen(key_numbers) > 0) {
						j = i_note = 0; key_number_string[0] = '\0';
						for(i = 1; i < strlen(key_numbers); i++) {
							c = key_numbers[i];
							if(c == 'k') break;
							if(c != ' ' && c != 'k') key_number_string[j++] = c;
							else {
								key_number_string[j] = '\0';
								key = atoi(key_number_string);
								if(key >= basekey) key = key - basekey;
								(*((*Scale)[NumberScales].keys))[i_note] = key;
								if(trace_each_scale) 
									BPPrintMessage(0,odInfo,"Creating i_note = %d key = %d\n",i_note,key);
								j = 0; i_note++;
								}
							}
						}
					else {
						for(i_note = 0; i_note < numgrades; i_note++)
							(*((*Scale)[NumberScales].keys))[i_note] = i_note;
						}
					j = i_note = j_note  = numnotes = 0;
					if(TraceMicrotonality) 
						BPPrintMessage(0,odInfo,"\nLoading note names for « %s » (scale %d):\n%s\n",name,NumberScales,note_names);
					for(i = 1; i < strlen(note_names); i++) {
						c = note_names[i];
						this_note[j] = '\0';
						if(c == '/') break;
						if(c != ' ' && c != '/') this_note[j++] = c;
						else {
							ptr = NULL;
							MystrcpyStringToHandle(&ptr,this_note);
							(*((*Scale)[NumberScales].notenames))[j_note] = ptr;
							if(strcmp(*ptr,"•") != 0) {
								(*((*Scale)[NumberScales].keyclass))[i_note] = j_note;
								numnotes++;
								if(trace_each_scale)
									BPPrintMessage(0,odInfo,"Creating note %d = %s, keyclass = %d\n",i_note,this_note,(*((*Scale)[NumberScales].keyclass))[i_note]);
								i_note++; j_note++;
								}
							else {
								if(trace_each_scale)
									BPPrintMessage(0,odInfo,"Creating %s, keyclass = %d\n",this_note,j_note);
								j_note++;
								}
							j = 0; 
							}
						}
					if(numnotes == 0) {
						NumberScales--;
						return MISSED;
						}
					(*Scale)[NumberScales].numnotes =  numnotes;		
					(*((*Scale)[NumberScales].keyclass))[numnotes] = (*((*Scale)[NumberScales].keyclass))[0];
					if(strlen(fractions) > 0) {
						if(TraceMicrotonality)
							BPPrintMessage(0,odInfo,"\nLoading note ratios for « %s » (scale %d):\n%s\n",name,NumberScales,fractions);
						j = i_note = 0;
						numerator = TRUE;
						for(i = 1; i < strlen(fractions); i++) {
							c = fractions[i];
							if(c == ']') break;
							if(c != ' ' && c != ']') this_fraction_arg[j++] = c;
							else {
								this_fraction_arg[j] = '\0';
								if(numerator)
									num[i_note] = atoi(this_fraction_arg);
								else {
									den[i_note] = atoi(this_fraction_arg);
								if(trace_each_scale) 
									BPPrintMessage(0,odInfo,"Creating position = %d ratio %d/%d\n",i_note,num[i_note],den[i_note]);
									i_note++;
									}
								numerator = 1 - numerator;
								j = 0;
								}
							}
						}
					break;
				default:
					if(n_args > 8) {
						if((n_args - 9) >= numgrades) {
							BPPrintMessage(0,odError,"\n=> This GEN51 function table is incorrect because it contains more than %d ratios:\n%s\n",numgrades,line);
							NumberScales--;
							return(OK);
							}
						(*((*Scale)[NumberScales].tuningratio))[n_args-9] = strtod(curr_arg,NULL);
						if(trace_each_scale)
							BPPrintMessage(0,odInfo,"position = %d, floating ratio = %.3f\n",n_args-9,(*((*Scale)[NumberScales].tuningratio))[n_args-9]);
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
		BPPrintMessage(0,odError,"\n=> This GEN51 function table is incorrect because it contains %d ratios instead of %d:\n%s\n",(n_args - 8),(numgrades +1),line);
		NumberScales--;
		return(OK);
		}
	int i_scale = NumberScales;
	if(strlen(baseoctave_string) == 0) baseoctave = 4;
	else baseoctave = (int) atol(baseoctave_string);
	if(baseoctave <= 0 || baseoctave > 14) baseoctave = 4;
	(*Scale)[i_scale].baseoctave = baseoctave;
	if(trace_each_scale)
		BPPrintMessage(0,odInfo,"\nbaseoctave_string = « %s » (*Scale)[i_scale].baseoctave = %d\n",baseoctave_string,baseoctave);
	(*((*Scale)[i_scale].tuningratio))[n_args-9] = strtod(curr_arg,NULL);
	if(strlen(name) > 0) my_sprintf(label,"%s",name);
	else my_sprintf(label,"scale_%d",i_scale); // Name by default
	(*Scale)[i_scale].label = (char**) GiveSpace((Size)(1 + strlen(label) * sizeof(char)));
	MystrcpyStringToHandle(&((*Scale)[i_scale].label),label);
	if(TraceMicrotonality) {
		BPPrintMessage(0,odInfo,"👉 Microtonal scale #%d \"%s\" loaded from tonality resources:  %d grades, %d notes\n",i_scale,*((*Scale)[i_scale].label),(*Scale)[i_scale].numgrades,numnotes);
		BPPrintMessage(0,odInfo,"Deviations are corrections in cents from an equal tempered scale of ");
		if(numgrades <= 12) BPPrintMessage(0,odInfo,"12 tones\n");
		else BPPrintMessage(0,odInfo,"%d tones\n",numgrades);
		}
	blockkey_temp_freq = (*Scale)[i_scale].basefreq * exp((9. / 12) * log((*Scale)[i_scale].interval));
	
	if(strlen(fractions) > 0) { // Use fractions to calculate ratios more accurately
		for(i = 0; i <= (*Scale)[i_scale].numgrades; i++) {
			if(num[i] > 0 && den[i] > 0)
				(*((*Scale)[i_scale].tuningratio))[i] = ((double) num[i]) / den[i];
			}
		}

	if(basekey != C4key && !WarnedBasedKey) {
		BPPrintMessage(0,odError,"👉 The base key of your microtonal scale ‘%s’ (%d) is not the ‘C4’ (60) used by default\n",*((*Scale)[i_scale].label),basekey);
		BPPrintMessage(0,odInfo,"--> This may be the case with other scales. Check that it meets the requirements of your MIDI output device!\n");
		WarnedBasedKey = TRUE;
		}

	// Fill the table of cent corrections that will be used for MIDI microtonality
	
	int check_corrections = FALSE;
	for(key = 0; key < MAXKEY; key++) {
		if(numgrades <= 12) {
			keyclass = modulo(key - basekey, numgrades);
			octave = baseoctave + floor(((double)key - basekey) / numgrades);
			}
		else {
			i_note = modulo(key - basekey, numnotes);
			keyclass = (*((*Scale)[i_scale].keyclass))[i_note];
			octave = baseoctave + floor((((double)key - basekey)) / numnotes);
			}
		strcpy(name,*((*(*Scale)[i_scale].notenames)[keyclass]));
		temp_note = modulo(key - basekey, 12);
		temp_octave = baseoctave + floor((((double)key - basekey)) / 12);
		temp_pitch = (*Scale)[i_scale].basefreq * pow(2,((double)temp_note/12)) * pow(2,temp_octave - baseoctave);
		if(check_corrections) BPPrintMessage(0,odInfo,"‘%s’ temp_pitch = %.3f, keyclass = %d octave = %d, interval = %.3f\n",name,temp_pitch,keyclass,octave,interval);
		pitch_ratio = (*((*Scale)[i_scale].tuningratio))[keyclass];
	/*	double n = log(pitch_ratio * pow(interval,octave - baseoctave)) / log(interval);
		frequency = (*Scale)[i_scale].basefreq  * pow(interval,n); */
		frequency = (*Scale)[i_scale].basefreq  * pitch_ratio * pow(interval,(octave - baseoctave));
		keyclass_near = round(12. * log2(pitch_ratio));
	//	frequency_near = (*Scale)[i_scale].basefreq  * pow(2,((double) keyclass_near / 12)) * pow(interval,(octave - baseoctave));
		(*(*Scale)[i_scale].blockkey_shift)[key] = (short) 1200. * log2(pow(2,((double) keyclass_near / 12)) / pitch_ratio);
		if(check_corrections) BPPrintMessage(0,odInfo,"temp_pitch = %.3f, frequency = %.3f, pitch_ratio = %.3f, n = %.3f\n",temp_pitch,frequency,pitch_ratio,n);
		(*(*Scale)[i_scale].deviation)[key] = (short) (1200. * log2(frequency/temp_pitch));
		if(TraceMicrotonality) {
			BPPrintMessage(0,odInfo,"• [%d] %s deviation = %d c, [oct %d] %.3f Hz -> [oct %d] %.3f blockkey shift %d c\n",key,name,(*(*Scale)[i_scale].deviation)[key],temp_octave,temp_pitch,octave,frequency,(*(*Scale)[i_scale].blockkey_shift)[key]);
			}
		}
	if(TraceMicrotonality) {
		// for(i = 0; i <= (*Scale)[i_scale].numgrades; i++)
		//	BPPrintMessage(0,odInfo,"%.3f ",(*((*Scale)[i_scale].tuningratio))[i]);
		if(strlen(note_names) > 0) BPPrintMessage(0,odInfo,"\nNames of the %d notes in this scale: %s",numnotes,note_names);
		BPPrintMessage(0,odInfo,"\nWith 'interval' = %.3f, 'basefreq' = %.3f Hz, 'basekey' = %d and 'baseoctave' = %d\n",(*Scale)[i_scale].interval,(*Scale)[i_scale].basefreq,(*Scale)[i_scale].basekey,(*Scale)[i_scale].baseoctave);
		BPPrintMessage(0,odInfo,"A4 frequency of a tempered scale with the same 'basefreq' and 'interval' would be %.3f Hz\n",blockkey_temp_freq);
		PrintThisNote(-1,DefaultBlockKey,-1,-1,Message);
		BPPrintMessage(0,odInfo,"Frequency will be blocked for the base note key #%d = ‘%s’, but this may be changed in performance controls: \"_scale(some_scale, blockkey)\"\n",DefaultBlockKey,Message);
		}
	return(OK);
	}


double GetPitchWithScale(int i_scale, int kcurrentinstance, int key, double cents, int blockkey) {
	int this_block_key, i_note, temp_note,note_class, octave, temp_octave,keyclass,blockkey_pitch_class, numgrades, numnotes, basekey, basekeyclass, baseoctave, delta_key, pitchbend_master;
	double basefreq, blockkey_pitch_ratio, blockkey_correction, basekey_ratio, diapason_correction, interval, n, frequency;
	char this_key[100];
	
	int check_getpitch = FALSE;

	if(i_scale > NumberScales) { 
		BPPrintMessage(0,odError,"\n=> Scale number %d is out of range (maximum %d). No Csound score produced\n\n",i_scale,NumberScales);
		return(Infpos);
		}
	if(key < 0 || key > 127) {
		BPPrintMessage(0,odError,"\n=> Key #%d is out of range [0..127]. No Csound score produced\n\n",key);
		return(Infpos);
		}
	numgrades = (*Scale)[i_scale].numgrades; // Most often 12
	numnotes = (*Scale)[i_scale].numnotes; // Most often 12
	interval = (*Scale)[i_scale].interval; // Most often 2
	basekey = (*Scale)[i_scale].basekey; // Starting point of scale ratios  (most often 60). Not used here.
	basefreq = (*Scale)[i_scale].basefreq; // Frequency of base key
	baseoctave = (*Scale)[i_scale].baseoctave;
	
	if(!ToldAboutScale && !TraceMicrotonality) {
		MystrcpyHandleToString(0,0,Message,(*Scale)[i_scale].label);
		BPPrintMessage(0,odInfo,"👉 Custom scale \"%s\" (and maybe more scales) used for microtonality\n",Message);
		ToldAboutScale = TRUE;
		}
	if(numgrades <= 12) {
		keyclass = modulo(key - basekey, numgrades);
		octave = baseoctave + floor(((double)key - basekey) / numgrades);
		}
	else {
		i_note = modulo(key - basekey, numnotes);
		keyclass = (*((*Scale)[i_scale].keyclass))[i_note];
		octave = baseoctave + floor((((double)key - basekey)) / numnotes);
		}
	temp_note = modulo(key - basekey, 12);
	temp_octave = baseoctave + floor((double) (key - basekey) / 12);
	frequency = (*Scale)[i_scale].basefreq * pow(2,((double)temp_note/12)) * pow(2,temp_octave - baseoctave);
	
	basekey_ratio = (*((*Scale)[i_scale].tuningratio))[0];
	frequency = frequency * basekey_ratio;
	diapason_correction = A4freq / 440.; 
	frequency = frequency * diapason_correction;
	frequency = frequency * pow(2, ((double) cents / 1200.));

	if(blockkey == 0) blockkey = DefaultBlockKey;

	// int correction = (*(*Scale)[i_scale].deviation)[key] - (*(*Scale)[i_scale].deviation)[blockkey];
	int correction = (*(*Scale)[i_scale].deviation)[key] + (*(*Scale)[i_scale].blockkey_shift)[blockkey];
	frequency = frequency * pow(2.,((double) correction / 1200.));

	my_sprintf(this_key,"%s%d",*((*(*Scale)[i_scale].notenames)[keyclass]),octave);
	trim_digits_after_key_hash(this_key); // Remove the octave number after key#xx

	if(TraceMicrotonality) {
		pitchbend_master = (int) PitchbendStart(kcurrentinstance);
		if(pitchbend_master > 0 && pitchbend_master < 16384) pitchbend_master -= DEFTPITCHBEND;
		else pitchbend_master = 0;
		BPPrintMessage(0,odInfo,"§ key %d: \"%s\" octave %d scale #%d, block key %d, corr %d cents, freq %.3f Hz\n",key,this_key,octave,i_scale,blockkey,correction,frequency);
		if(pitchbend_master != 0) BPPrintMessage(0,odInfo,"--> with additional pitchbend value of %d\n",pitchbend_master);
		}

	if(check_getpitch) BPPrintMessage(0,odInfo,"• key = %d, C4key = %d, basekey = %d, basekeyclass = %d, basefreq = %.3f baseoctave = %d, numgrades = %d interval = %.3f, keyclass = %d, diapason_correction = %.4f blockkey = %d, correction = %d, octave = %d, cents = %.1f, tuningratio[basekeyclass] = %.3f frequency = %.3f Hz\n",key,C4key,(*Scale)[i_scale].basekey,basekeyclass,basefreq,baseoctave,numgrades,interval,keyclass,diapason_correction,blockkey,correction,octave,cents,(*((*Scale)[i_scale].tuningratio))[basekeyclass],frequency);

	return frequency;
	}

