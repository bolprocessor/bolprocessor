/* SoundObjects3.c (BP2 version CVS) */

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


ResetPrototype(int j)
{
char line[MAXFIELDCONTENT];
Handle ptr;
int i;

if(j < 2 || j >= Jbol) {
   if(Beta) Alert1("Err. ResetPrototype()");
   return(FAILED);
   }
(*p_Type)[j] = 0;

ptr = (Handle)(*pp_MIDIcode)[j];
if(MyDisposeHandle(&ptr) != OK) return(ABORT);
(*pp_MIDIcode)[j] = NULL;
ptr = (Handle)(*pp_CsoundTime)[j];
if(MyDisposeHandle(&ptr) != OK) return(ABORT);
(*pp_CsoundTime)[j] = NULL;

(*p_ContBegMode)[j] = (*p_ContEndMode)[j] = ABSOLUTE;
(*p_MaxBegGap)[j] = (*p_MaxEndGap)[j] = Infpos;
(*p_DelayMode)[j] = (*p_ForwardMode)[j] = (*p_BreakTempoMode)[j]
   = ABSOLUTE;
(*p_CoverBegMode)[j] = (*p_CoverEndMode)[j] = RELATIVE;
(*p_TruncBegMode)[j] = (*p_TruncEndMode)[j] = (*p_PreRollMode)[j]
      = (*p_PostRollMode)[j] = RELATIVE;
(*p_MaxCoverBeg)[j] = (*p_MaxCoverEnd)[j] = 100L;
(*p_MaxTruncBeg)[j] = (*p_MaxTruncEnd)[j] = 0L;
(*p_PivPos)[j] = (*p_PreRoll)[j] = (*p_PostRoll)[j] = (*p_BeforePeriod)[j] = ZERO;
(*p_PeriodMode)[j] = IRRELEVANT;

(*p_AlphaCtrlNr)[j] = 255; (*p_AlphaCtrlChan)[j] = 255;
(*p_Ifrom)[j] = 0;
(*p_PasteDone)[j] = FALSE;
(*p_FixScale)[j] = (*p_ContBeg)[j] = (*p_ContEnd)[j]
   = (*p_TruncBeg)[j] = (*p_TruncEnd)[j] = (*p_AlphaCtrl)[j] = FALSE;
(*p_MaxDelay)[j] = (*p_MaxForward)[j] = ZERO;
(*p_CoverBeg)[j] = (*p_CoverEnd)[j] = (*p_OkRelocate)[j] = (*p_OkExpand)[j]
   = (*p_OkCompress)[j] = TRUE;
(*p_OkTransp)[j] = (*p_OkPan)[j] = (*p_OkMap)[j] = (*p_OkVelocity)[j]
   = (*p_OkArticul)[j] = (*p_OkVolume)[j] = TRUE;
(*p_BreakTempo)[j] = (*p_DiscardNoteOffs)[j] = FALSE;
(*p_PivType)[j] = 1; (*p_PivMode)[j] = ABSOLUTE;
(*p_RescaleMode)[j] = LINEAR;
(*p_AlphaMin)[j] = 0; (*p_AlphaMax)[j] = 100.;
(*p_Dur)[j] = ZERO;
(*p_Resolution)[j] = 1;
(*p_Tref)[j] = 1000L;
(*p_ForceIntegerPeriod)[j] = FALSE;
(*p_DefaultChannel)[j] = (*p_Quan)[j] = 0;
(*p_StrikeAgain)[j] = -1;
(*p_Tpict)[j] = ZERO;
(*p_ObjectColor)[j].red = (*p_ObjectColor)[j].green = (*p_ObjectColor)[j].blue = -1L;
ptr = (Handle)(*pp_Comment)[j];
if(MyDisposeHandle(&ptr) != OK) return(ABORT);
(*pp_Comment)[j] = NULL;
ClearWindow(FALSE,wPrototype7);
ptr = (Handle)(*pp_CsoundScoreText)[j];
if(MyDisposeHandle(&ptr) != OK) return(ABORT);
(*pp_CsoundScoreText)[j] = NULL;
if((*pp_CsoundScore)[j] != NULL) {
   for(i=0; i < (*p_CsoundSize)[j]; i++) {
      ptr = (Handle) (*((*pp_CsoundScore)[j]))[i].h_param;
      if(MyDisposeHandle(&ptr) != OK) return(ABORT);
      (*((*pp_CsoundScore)[j]))[i].h_param = NULL;
      }
   }
ptr = (Handle)(*pp_CsoundScore)[j];
if(MyDisposeHandle(&ptr) != OK) return(ABORT);
(*pp_CsoundScore)[j] = NULL;
(*p_CsoundInstr)[j] = 0;
(*p_CsoundAssignedInstr)[j] = -1;
(*p_MIDIsize)[j] = (*p_CsoundSize)[j] = ZERO;
ChangedProtoType(j);
CompiledCsObjects = (*p_CompiledCsoundScore)[j] = FALSE;
return(OK);
}

#if BP_CARBON_GUI
GetPrototype(int force)
// Copying from dialogs to memory
{
long p,q,dur;
char line[MAXFIELDCONTENT];
Handle h;
int rep;
double ratio;

if(iProto < 2 || iProto >= Jbol || (!force && !Dirty[iObjects])) return(OK);

GetField(NULL,FALSE,wPrototype1,fPrototypeComment,line,&p,&q);
h = (Handle) (*pp_Comment)[iProto];
if(MyDisposeHandle(&h) != OK) return(ABORT);
(*pp_Comment)[iProto] = NULL;
if((h = (Handle) GiveSpace((Size)(1+strlen(line)) * sizeof(char)))
      == NULL) return(ABORT);
(*pp_Comment)[iProto] = h;

if(MystrcpyStringToHandle(&((*pp_Comment)[iProto]),line) != OK) return(ABORT);

rep = GetField(NULL,TRUE,wPrototype2,fMinDilationRatio,line,&p,&q);
if(rep == OK) {
   if((ratio=((float)p)/q) < 0.) {
      Alert1("Minimum ratio can't be negative");
      SetField(NULL,wPrototype2,fMinDilationRatio,"[?]");
      }
   else (*p_AlphaMin)[iProto] = ratio;
   }

rep = GetField(NULL,TRUE,wPrototype2,fMaxDilationRatio,line,&p,&q);
if(rep == OK) {
   if((ratio=((float)p)/q) < 0.) {
      Alert1("Maximum ratio can't be negative");
      SetField(NULL,wPrototype2,fMaxDilationRatio,"[?]");
      }
   else (*p_AlphaMax)[iProto] = ((float)p)/q;
   }

rep = GetField(NULL,TRUE,wPrototype2,fSendAlphaControl,line,&p,&q);

if(rep == OK && p/q >= 0) (*p_AlphaCtrlNr)[iProto] = p/q;
else (*p_AlphaCtrlNr)[iProto] = 255;

rep = GetField(NULL,TRUE,wPrototype2,fSendAlphaChannel,line,&p,&q);
if(rep == OK && CheckChannelRange(&p,&q)) (*p_AlphaCtrlChan)[iProto] = p/q;
else (*p_AlphaCtrlChan)[iProto] = 255;

if((*p_PivType)[iProto] == 7) {
   if((*p_PivMode)[iProto] == RELATIVE)
      rep = GetField(NULL,TRUE,wPrototype2,fSetPivotPC,line,&p,&q);
   else
      rep = GetField(NULL,TRUE,wPrototype2,fSetPivotms,line,&p,&q);
   if(rep == OK) (*p_PivPos)[iProto] = ((float)p)/q;
   }
   
if((*p_DelayMode)[iProto] == ABSOLUTE)
   rep = GetField(NULL,TRUE,wPrototype2,fAllowDelayms,line,&p,&q);
else
   rep = GetField(NULL,TRUE,wPrototype2,fAllowDelayPC,line,&p,&q);
if(rep == OK) (*p_MaxDelay)[iProto] = (long) p/q;

if((*p_ForwardMode)[iProto] == ABSOLUTE)
   rep = GetField(NULL,TRUE,wPrototype2,fAllowFwdms,line,&p,&q);
else
   rep = GetField(NULL,TRUE,wPrototype2,fAllowFwdPC,line,&p,&q);
if(rep == OK) (*p_MaxForward)[iProto] = (long) p/q;

if((*p_CoverBegMode)[iProto] == ABSOLUTE)
   rep = GetField(NULL,TRUE,wPrototype3,fCoverBegLessThanms,line,&p,&q);
else
   rep = GetField(NULL,TRUE,wPrototype3,fCoverBegLessThanPC,line,&p,&q);
if(rep == OK) (*p_MaxCoverBeg)[iProto] = (long) p/q;

if((*p_CoverEndMode)[iProto] == ABSOLUTE)
   rep = GetField(NULL,TRUE,wPrototype3,fCoverEndLessThanms,line,&p,&q);
else
   rep = GetField(NULL,TRUE,wPrototype3,fCoverEndLessThanPC,line,&p,&q);
if(rep == OK) (*p_MaxCoverEnd)[iProto] = (long) p/q;

if((*p_TruncBegMode)[iProto] == ABSOLUTE)
   rep = GetField(NULL,TRUE,wPrototype3,fTruncBegLessThanms,line,&p,&q);
else
   rep = GetField(NULL,TRUE,wPrototype3,fTruncBegLessThanPC,line,&p,&q);
if(rep == OK) (*p_MaxTruncBeg)[iProto] = (long) p/q;

if((*p_TruncEndMode)[iProto] == ABSOLUTE)
   rep = GetField(NULL,TRUE,wPrototype3,fTruncEndLessThanms,line,&p,&q);
else
   rep = GetField(NULL,TRUE,wPrototype3,fTruncEndLessThanPC,line,&p,&q);
if(rep == OK) (*p_MaxTruncEnd)[iProto] = (long) p/q;

if((*p_ContBegMode)[iProto] == ABSOLUTE)
   rep = GetField(NULL,TRUE,wPrototype4,fAllowGapBegms,line,&p,&q);
else
   rep = GetField(NULL,TRUE,wPrototype4,fAllowGapBegPC,line,&p,&q);
if(rep == OK) (*p_MaxBegGap)[iProto] = (long) p/q;

if((*p_ContEndMode)[iProto] == ABSOLUTE)
   rep = GetField(NULL,TRUE,wPrototype4,fAllowGapEndms,line,&p,&q);
else
   rep = GetField(NULL,TRUE,wPrototype4,fAllowGapEndPC,line,&p,&q);
if(rep == OK) (*p_MaxEndGap)[iProto] = (long) p/q;

if(rep = GetField(NULL,TRUE,wPrototype4,fPreRollms,line,&p,&q) == OK)	// FIXME ?
   (*p_PreRoll)[iProto] = (long) p/q;

if(rep = GetField(NULL,TRUE,wPrototype4,fPostRollms,line,&p,&q) == OK)	// FIXME ?
   (*p_PostRoll)[iProto] = (long) p/q;

if(GetCtrlValue(wPrototype5,bStriatedObject) > 0) {
   if(GetField(NULL,TRUE,wPrototype5,fTref,line,&p,&q) == OK)
      (*p_Tref)[iProto] = p/q;
   }
else (*p_Tref)[iProto] = ZERO;

if(GetField(NULL,TRUE,wPrototype5,fQuantizeFractionBeat,line,&p,&q) == OK) {
   if((*p_Quan)[iProto] > 0.01 && p > ZERO) {
      (*p_Quan)[iProto] = ((double)(*p_Tref)[iProto] * q) / p;
      }
   }

if(GetField(NULL,TRUE,wPrototype5,fPrototypeTickKey,line,&p,&q) == OK) {
   if(p/q < 0 || p/q > 127) {
      sprintf(Message,"Tick key should be in range 0..127. Can't accept %ld",
         (long)(p/q));
      Alert1(Message);
      }
   else PrototypeTickKey = p/q;
   }

if(GetField(NULL,TRUE,wPrototype5,fPrototypeTickChannel,line,&p,&q) == OK) {
   if(CheckChannelRange(&p,&q)) PrototypeTickChannel = p/q;
   else SetField(NULL,wPrototype5,fPrototypeTickChannel,"1");
   }

if(GetField(NULL,TRUE,wPrototype5,fPrototypeTickVelocity,line,&p,&q) == OK) {
   if(p/q < 1 || p/q > 127) {
      sprintf(Message,"Tick velocity should be in range 1..127. Can't accept %ld",
         (long)(p/q));
      Alert1(Message);
      }
   else PrototypeTickVelocity = p/q;
   }

if((*p_PeriodMode)[iProto] == ABSOLUTE) {
   if(GetField(NULL,TRUE,wPrototype6,fBeforePeriodms,line,&p,&q) == OK) {
      if((dur=(*p_Dur)[iProto]) < p/q) {
         sprintf(Message,"Initial part before period can't be longer than %ldms",
            (long)dur);
         Alert1(Message);
         p = dur; q = 1L; sprintf(line,"%ld",(long)p);
         SetField(NULL,wPrototype6,fBeforePeriodms,line);
         }
      if(p < ZERO) {
         Alert1("Initial part before period can't be negative");
         p = ZERO; q = 1L; sprintf(line,"%ld",(long)p);
         SetField(NULL,wPrototype6,fBeforePeriodms,line);
         SetField(NULL,wPrototype6,fBeforePeriodPC,line);
         }
      }
   }
if((*p_PeriodMode)[iProto] == RELATIVE) {
   if(GetField(NULL,TRUE,wPrototype6,fBeforePeriodPC,line,&p,&q) == OK) {
      if(p/q > 100.) {
         Alert1("Initial part before period can't be longer than 100% duration");
         p = 100L; q = 1L; sprintf(line,"%ld",(long)p);
         SetField(NULL,wPrototype6,fBeforePeriodPC,line);
         }
      if(p < ZERO) {
         Alert1("Initial part before period can't be negative");
         p = ZERO; q = 1L; sprintf(line,"%ld",(long)p);
         SetField(NULL,wPrototype6,fBeforePeriodPC,line);
         SetField(NULL,wPrototype6,fBeforePeriodms,line);
         }
      }
   }

if((*p_PeriodMode)[iProto] != IRRELEVANT) (*p_BeforePeriod)[iProto] = ((float)p)/q;

if((*p_DefaultChannel)[iProto] > 0) {
   if(GetField(NULL,TRUE,wPrototype8,fForceToChannel,line,&p,&q) == OK
                                       && CheckChannelRange(&p,&q))
      (*p_DefaultChannel)[iProto] = (long) p/q;
   else {
      SetField(NULL,wPrototype8,fForceToChannel,"[?]");
      (*p_DefaultChannel)[iProto] = 0;
      }
   }
if((*p_CsoundInstr)[iProto] > 0) {
   if(GetField(NULL,TRUE,wPrototype8,fForceToInstrument,line,&p,&q) == OK) {
      (*p_CsoundInstr)[iProto] = (long) p/q;
      }
   }

GetCsoundScore(iProto);

if((GetField(NULL,TRUE,wPrototype8,fAssignInstrument,line,&p,&q) != OK) || p < ZERO) {
   SetField(NULL,wPrototype8,fAssignInstrument,"[None]");
   (*p_CsoundAssignedInstr)[iProto] = -1;
	}
else {
   if((*p_CsoundSize)[iProto] > ZERO) {
      ShowWindow(Window[wPrototype8]);
      BringToFront(Window[wPrototype8]);
      SetField(NULL,wPrototype8,fAssignInstrument,"[None]");
      SelectField(NULL,wPrototype8,fAssignInstrument,TRUE);
      Alert1("This sound-object prototype contains a Csound score.\nConversion from MIDI will therefore never occur");
      (*p_CsoundAssignedInstr)[iProto] = -1;
	  }
   else (*p_CsoundAssignedInstr)[iProto] = p/q;
	}
return(CheckConsistency(iProto,TRUE));
}
#endif /* BP_CARBON_GUI */


#if BP_CARBON_GUI	// CheckPrototypes() is only called from CheckiProto() & mObjectPrototypes()
CheckPrototypes(void)
{
int r,longerCsound;

r = FAILED;
if(!CompiledAl  || (!CompiledGr && (AddBolsInGrammar() > BolsInGrammar))) {
   CompiledAl = FALSE;
   }
if(SaveCheck(wAlphabet) == ABORT) goto END;
if(SaveCheck(wGrammar) == ABORT) goto END;
if(SaveCheck(wInteraction) == ABORT) goto END;
if((r=UpdateGlossary()) != OK) goto END;
if(SaveCheck(wGlossary) == ABORT) goto END;
if(ResetControllers) ResetMIDIControllers(NO,NO,YES);
if(CompileCheck() != OK) return(FAILED);

if(Jbol < 3) return(OK);
if(!NeedAlphabet) {
   NeedAlphabet = TRUE;
   ObjectMode = ObjectTry = FALSE;
   }
if(!ObjectMode && !ObjectTry) {
   if(LoadObjectPrototypes(YES,YES) != OK) {
      FileName[iObjects][0] = '\0';
      SetName(iObjects,TRUE,TRUE);
      Dirty[iObjects] = Created[iObjects] = FALSE;
      iProto = 0;
      return(FAILED);
      }
   else {
      if(iProto >= Jbol) iProto = 2;
      SetPrototype(iProto);
#if BP_CARBON_GUI
      SetCsoundScore(iProto);
#endif /* BP_CARBON_GUI */
      if((r=CompileObjectScore(iProto,&longerCsound)) != OK) goto END;
      StopWait();
      }
   }
r = OK;
   
END:
HideWindow(Window[wMessage]);
return(r);
}
#endif /* BP_CARBON_GUI */


CheckConsistency(int j,int check)
{
int k,bugg,longerCsound;
long i,t,ton,toff;
Milliseconds dur,maxcover1,maxcover2,maxtrunc1,maxtrunc2;
double preroll,postroll;

if(j >= Jbol || j < 2) return(OK);
bugg = FALSE;

if((*p_CsoundSize)[j] <= ZERO) {
	if((*p_Type)[j] & 4) {
		(*p_Type)[j] &= (255-4);
		(*p_CsoundInstr)[j] = 0;
#if BP_CARBON_GUI
		if(iProto == j) {
			SwitchOff(NULL,wPrototype1,bCsoundInstrument);
	    	SwitchOn(NULL,wPrototype8,bForceCurrInstrument);
	    	SwitchOff(NULL,wPrototype8,bDontChangeInstrument);
	    	SwitchOff(NULL,wPrototype8,bForceToInstrument);
			}
#endif /* BP_CARBON_GUI */
		}
   }
else  {
   if(!((*p_Type)[j] & 4)) {
		(*p_Type)[j] |= 4;
#if BP_CARBON_GUI
		if(iProto == j) SwitchOn(NULL,wPrototype1,bCsoundInstrument);
#endif /* BP_CARBON_GUI */
		}
   }
if((*p_MIDIsize)[j] == ZERO) {
	if((*p_Type)[j] & 1) {
		(*p_Type)[j] &= (255-1);
#if BP_CARBON_GUI
		if(iProto == j) SwitchOff(NULL,wPrototype1,bMIDIsequence);
#endif /* BP_CARBON_GUI */
		}
	}
else  {
/*	if(!((*p_Type)[j] & 1)) {
		(*p_Type)[j] |= 1;
		if(iProto == j) SwitchOn(NULL,wPrototype1,bMIDIsequence);
		} */
	}
   
if((*p_Resolution)[j] <= ZERO) {
   if(Beta) Alert1("Err. CheckConsistency(). (*p_Resolution)[j] <= ZERO");
   return(FAILED);
   }
if((*p_FixScale)[j]) {
   (*p_OkExpand)[j] = (*p_OkCompress)[j] = FALSE;
   }
if((*p_Tref)[j] < EPSILON) {
   (*p_Tref)[j] = ZERO;
   (*p_OkRelocate)[j] = TRUE;
   }
if((*p_PivType)[j] < 1 || (*p_PivType)[j] > 7) {
   (*p_PivType)[j] = 1; bugg = TRUE;
   }
if((*p_PivMode)[j] < ABSOLUTE || (*p_PivMode)[j] > RELATIVE) {
   (*p_PivMode)[j] = ABSOLUTE; bugg = TRUE;
   }

SetPrototypeDuration(j,&longerCsound);
dur = (*p_Dur)[j];
if(dur < EPSILON) {
   (*p_PivType)[j] = 1; (*p_PivPos)[j] = ZERO;
   (*p_CoverBegMode)[j] = (*p_CoverEndMode)[j] = (*p_TruncBegMode)[j]
      = (*p_TruncEndMode)[j] = (*p_ContBegMode)[j] = (*p_ContEndMode)[j] = ABSOLUTE;
   (*p_CoverBeg)[j] = (*p_CoverEnd)[j] = TRUE;
   (*p_MaxCoverBeg)[j] = (*p_MaxCoverEnd)[j] = ZERO;
   (*p_TruncBeg)[j] = (*p_TruncEnd)[j] = FALSE;
   (*p_MaxTruncBeg)[j] = (*p_MaxTruncEnd)[j] = ZERO;
   }
switch((*p_PivType)[j]) {
   case 1: (*p_PivPos)[j] = 0.; (*p_PivMode)[j] = ABSOLUTE; break;
   case 2: (*p_PivPos)[j] = 100.; (*p_PivMode)[j] = RELATIVE; break;
   case 5: (*p_PivPos)[j] = 50.; (*p_PivMode)[j] = RELATIVE; break;
   case 7: /* pivspec */ break;
   case 4: /* endoff */
   case 3: /* begon */
   case 6: /* centonoff */
      if((*p_MIDIsize)[j] == ZERO && (*p_CsoundSize)[j] == ZERO) {
         (*p_PivType)[j] = 1; bugg = TRUE; break;
         }
      ton = toff = -1L;
      for(i=t=ZERO; i < (*p_MIDIsize)[j]-2; i++) {
         t += (*((*pp_MIDIcode)[j]))[i].time;
         k = (*((*pp_MIDIcode)[j]))[i].byte;
         if((NoteOn <= k && k < NoteOn+16 && (*((*pp_MIDIcode)[j]))[i+2].byte == 0)
               || (NoteOff <= k && k < NoteOff+16)) {
            toff = t;
            }
         if(NoteOn <= k && k < NoteOn+16 && (*((*pp_MIDIcode)[j]))[i+2].byte > 0) {
            if(ton == -1L) ton = t;
            }
         }
      if(ton < ZERO || toff < ZERO || dur < EPSILON) {
         (*p_PivType)[j] = 1; bugg = TRUE; break;
         }
      GetPrePostRoll(j,&preroll,&postroll);
      switch((*p_PivType)[j]) {
         case 3: /* begon */
            (*p_PivPos)[j] = ((float)(ton - preroll) * 100.) / dur;
            (*p_PivMode)[j] = RELATIVE; break;
         case 4: /* endoff */
            (*p_PivPos)[j] = ((float)(toff - preroll) * 100.) / dur;
            (*p_PivMode)[j] = RELATIVE; break;
         case 6: /* centonoff */
            (*p_PivPos)[j] = ((float)((ton+toff)/2. - preroll) * 100.) / dur;
            (*p_PivMode)[j] = RELATIVE; break;
         }
   }
   
if((*p_CoverBeg)[j] && dur > EPSILON) {
   (*p_MaxCoverBeg)[j] = 100L; (*p_CoverBegMode)[j] = RELATIVE;
   }
if((*p_CoverEnd)[j] && dur > EPSILON) {
   (*p_MaxCoverEnd)[j] = 100L; (*p_CoverEndMode)[j] = RELATIVE;
   }
if((*p_MaxCoverBeg)[j] < 0) {
   (*p_MaxCoverBeg)[j] = 0; bugg++;
   }
if((*p_CoverBegMode)[j] == RELATIVE) {
   if((*p_MaxCoverBeg)[j] > 100L) (*p_MaxCoverBeg)[j] = 100L;
   }
if((*p_MaxCoverEnd)[j] < 0) {
   (*p_MaxCoverEnd)[j] = 0; bugg++;
   }
if((*p_CoverEndMode)[j] == RELATIVE) {
   if((*p_MaxCoverEnd)[j] > 100L) (*p_MaxCoverEnd)[j] = 100L;
   }
if((*p_CoverBegMode)[j] == RELATIVE && (*p_CoverEndMode)[j] == RELATIVE) {
   maxcover1 = maxcover2 = INT_MAX;
   if(!(*p_CoverBeg)[j]) maxcover1 = (dur * (*p_MaxCoverBeg)[j]) / 100.;
   if(!(*p_CoverEnd)[j]) maxcover2 = (dur * (*p_MaxCoverEnd)[j]) / 100.;
   if(maxcover1 < dur && maxcover2 < dur && maxcover2 >= (dur - maxcover1)) {
      maxcover2 = dur - maxcover1 - 1;
      (*p_MaxCoverEnd)[j] = (maxcover2 * 100) / dur;
      }
   if(maxcover1 < dur && maxcover2 < dur && maxcover1 >= (dur - maxcover2)) {
      maxcover1 = dur - maxcover2 - 1;
      (*p_MaxCoverBeg)[j] = (maxcover1 * 100) / dur;
      }
   }
if((*p_CoverBegMode)[j] == RELATIVE && (*p_MaxCoverBeg)[j] == 100L)
   (*p_CoverBeg)[j] = TRUE;
if((*p_CoverEndMode)[j] == RELATIVE && (*p_MaxCoverEnd)[j] == 100L)
   (*p_CoverEnd)[j] = TRUE;
   
if((*p_TruncBeg)[j] && dur > EPSILON) {
   (*p_MaxTruncBeg)[j] = 100L; (*p_TruncBegMode)[j] = RELATIVE;
   }
if((*p_TruncEnd)[j] && dur > EPSILON) {
   (*p_MaxTruncEnd)[j] = 100L; (*p_TruncEndMode)[j] = RELATIVE;
   }
if((*p_MaxTruncBeg)[j] < 0) {
   (*p_MaxTruncBeg)[j] = 0; bugg++;
   }
if((*p_TruncBegMode)[j] == RELATIVE) {
   if((*p_MaxTruncBeg)[j] > 100L) (*p_MaxTruncBeg)[j] = 100L;
   }
if((*p_MaxTruncEnd)[j] < 0) {
   (*p_MaxTruncEnd)[j] = 0; bugg++;
   }
if((*p_TruncEndMode)[j] == RELATIVE) {
   if((*p_MaxTruncEnd)[j] > 100L) (*p_MaxTruncEnd)[j] = 100L;
   }
if((*p_TruncBegMode)[j] == RELATIVE && (*p_TruncEndMode)[j] == RELATIVE) {
   maxtrunc1 = maxtrunc2 = INT_MAX;
   if(!(*p_TruncBeg)[j]) maxtrunc1 = (dur * (*p_MaxTruncBeg)[j]) / 100.;
   if(!(*p_TruncEnd)[j]) maxtrunc2 = (dur * (*p_MaxTruncEnd)[j]) / 100.;
   if(maxtrunc1 < dur && maxtrunc2 < dur && maxtrunc2 > (dur - maxtrunc1)) {
      maxtrunc2 = dur - maxtrunc1 - 1;
      (*p_MaxTruncEnd)[j] = (maxtrunc2 * 100) / dur;
      }
   if(maxtrunc1 < dur && maxtrunc2 < dur && maxtrunc1 > (dur - maxtrunc2)) {
      maxtrunc1 = dur - maxtrunc2 - 1;
      (*p_MaxTruncBeg)[j] = (maxtrunc1 * 100) / dur;
      }
   }
if((*p_TruncBegMode)[j] == RELATIVE && (*p_MaxTruncBeg)[j] == 100L)
   (*p_TruncBeg)[j] = TRUE;
if((*p_TruncEndMode)[j] == RELATIVE && (*p_MaxTruncEnd)[j] == 100L)
   (*p_TruncEnd)[j] = TRUE;
if(check && bugg) {
   sprintf(Message,"Found inconsistencies in sound-object prototype '%s'. These have been corrected.\n",
      *((*p_Bol)[j]));
   Print(wTrace,Message);
   }
return(OK);
}


PrototypeWindow(int w)
{
switch(w) {
   case wPrototype1:
   case wPrototype2:
   case wPrototype3:
   case wPrototype4:
   case wPrototype5:
   case wPrototype6:
   case wPrototype7:
   case wPrototype8:
      return(TRUE);
   }
return(FALSE);
}

#if BP_CARBON_GUI

CopyFrom(int w)
{
int rep,j;

if(Jbol < 3) {
   Alert1("No sound-object prototype has been created/loaded");
   return(OK);
   }
if(SaveCheck(w) == ABORT) return(FAILED);
sprintf(Message,"This object is empty");
j = DoThings(p_Bol,2,Jbol,NULL,16,CheckPrototypeSize,Message,(int) pushButProc);
if(j == iProto || j < 2) return(OK);
switch(w) {
   case wPrototype1:
      sprintf(Message,"Copy all properties, MIDI codes, Csound score from '%s'",
         *((*p_Bol)[j])); break;
   case wPrototype2:
      sprintf(Message,"Copy Duration/Pivot/Location properties from '%s'",
         *((*p_Bol)[j])); break;
   case wPrototype3:
      sprintf(Message,"Copy Cover/Truncate/Break tempo properties from '%s'",
         *((*p_Bol)[j])); break;
   case wPrototype4:
      sprintf(Message,"Copy Continuity/Pre-postroll properties from '%s'",
         *((*p_Bol)[j])); break;
   case wPrototype5:
      sprintf(Message,"Copy MIDI codes (sound) from '%s'",
         *((*p_Bol)[j])); break;
   case wPrototype6:
      sprintf(Message,"Copy period settings from '%s'",
         *((*p_Bol)[j])); break;
   case wPrototype7:
      sprintf(Message,"Copy Csound score from '%s'",
         *((*p_Bol)[j])); break;
   case wPrototype8:
      sprintf(Message,"Copy MIDI and Csound properties from '%s'",
         *((*p_Bol)[j])); break;
   }
rep = Answer(Message,'N');
if(rep != OK) return(OK);
if(w == wPrototype1) {
   SetPrototypePage1(j);
   CopyPage1(j,iProto);
   ChangedProtoType(iProto);
   }
if(w == wPrototype1 || w == wPrototype2) {
   SetPrototypePage2(j);
   CopyPage2(j,iProto);
   }
if(w == wPrototype1 || w == wPrototype3) {
   SetPrototypePage3(j);
   CopyPage3(j,iProto);
   }
if(w == wPrototype1 || w == wPrototype4) {
   SetPrototypePage4(j);
   CopyPage4(j,iProto);
   }
if(w == wPrototype1 || w == wPrototype5) {
   SetPrototypePage5(j);
   CopyPage5(j,iProto);
   ChangedProtoType(iProto);
   }
if(w == wPrototype1 || w == wPrototype6) {
   SetPrototypePage6(j);
   CopyPage6(j,iProto);
   }
if(w == wPrototype1 || w == wPrototype7) {
   SetPrototypePage7(j);
   CopyPage7(j,iProto);
   }
if(w == wPrototype1 || w == wPrototype8) {
   SetPrototypePage8(j);
   CopyPage8(j,iProto);
   }
UpdateDirty(TRUE,w);
GetPrototype(YES);
return(OK);
}

#endif /* BP_CARBON_GUI */

CopyPage1(int i,int j)
{
(*p_Type)[j] = (*p_Type)[i];
return(OK);
}


CopyPage2(int i,int j)
{
(*p_OkExpand)[j] = (*p_OkExpand)[i];
(*p_OkCompress)[j] = (*p_OkCompress)[i];
(*p_FixScale)[j] = (*p_FixScale)[i];
(*p_PivType)[j] = (*p_PivType)[i];
(*p_PivMode)[j] = (*p_PivMode)[i];
(*p_OkRelocate)[j] = (*p_OkRelocate)[i];
(*p_MaxDelay)[j] = (*p_MaxDelay)[i];
(*p_MaxForward)[j] = (*p_MaxForward)[i];
return(OK);
}


CopyPage3(int i,int j)
{
(*p_CoverBeg)[j] = (*p_CoverBeg)[i];
(*p_MaxCoverBeg)[j] = (*p_MaxCoverBeg)[i];
(*p_CoverBegMode)[j] = (*p_CoverBegMode)[i];
(*p_CoverEnd)[j] = (*p_CoverEnd)[i];
(*p_MaxCoverEnd)[j] = (*p_MaxCoverEnd)[i];
(*p_CoverEndMode)[j] = (*p_CoverEndMode)[i];
(*p_TruncBeg)[j] = (*p_TruncBeg)[i];
(*p_MaxTruncBeg)[j] = (*p_MaxTruncBeg)[i];
(*p_TruncBegMode)[j] = (*p_TruncBegMode)[i];
(*p_TruncEnd)[j] = (*p_TruncEnd)[i];
(*p_MaxTruncEnd)[j] = (*p_MaxTruncEnd)[i];
(*p_TruncEndMode)[j] = (*p_TruncEndMode)[i];
(*p_BreakTempo)[j] = (*p_BreakTempo)[i];
return(OK);
}


CopyPage4(int i,int j)
{
(*p_ContBeg)[j] = (*p_ContBeg)[i];
(*p_MaxBegGap)[j] = (*p_MaxBegGap)[i];
(*p_ContBegMode)[j] = (*p_ContBegMode)[i];
(*p_ContEnd)[j] = (*p_ContEnd)[i];
(*p_MaxEndGap)[j] = (*p_MaxEndGap)[i];
(*p_ContEndMode)[j] =(*p_ContEndMode)[i];
(*p_PreRollMode)[j] = (*p_PreRollMode)[i];
(*p_PostRollMode)[j] = (*p_PostRollMode)[i];
(*p_PreRoll)[j] = (*p_PreRoll)[i];
(*p_PostRoll)[j] = (*p_PostRoll)[i];

return(OK);
}


CopyPage5(int i,int j)
{
Handle ptr1,ptr2;
int k;

(*p_Tref)[j] = (*p_Tref)[i];
(*p_Quan)[j] = (*p_Quan)[i];
(*p_Dur)[j] = (*p_Dur)[i];
(*p_MIDIsize)[j] = (*p_MIDIsize)[i];

/* Copy MIDI codes */
if((*p_MIDIsize)[i] > 0) {
	ptr1 = (Handle)(*pp_MIDIcode)[j];
	ptr2 = (Handle)(*pp_MIDIcode)[i];
	if(ptr2 == NULL) {
		if(Beta) Alert1("Err. CopyPage5(). ptr2 = NULL (1)");
		return(ABORT);
		}
	MySetHandleSize(&ptr1,MyGetHandleSize(ptr2));
	(*pp_MIDIcode)[j] = (MIDIcode**) ptr1;
   
	for(k=0; k < (*p_MIDIsize)[i]; k++) {
		(*((*pp_MIDIcode)[j]))[k] = (*((*pp_MIDIcode)[i]))[k];
		}
	}
else {
   ptr1 = (Handle)(*pp_MIDIcode)[j];
   if(MyDisposeHandle(&ptr1) != OK) return(ABORT);
   (*pp_MIDIcode)[j] = NULL;
   }
return(OK);
}


CopyPage6(int i,int j)
{
(*p_BeforePeriod)[j] = (*p_BeforePeriod)[i];
(*p_PeriodMode)[j] = (*p_PeriodMode)[i];
(*p_StrikeAgain)[j] = (*p_StrikeAgain)[i];
(*p_DiscardNoteOffs)[j] = (*p_DiscardNoteOffs)[i];
(*p_ForceIntegerPeriod)[j] = (*p_ForceIntegerPeriod)[i];
return(OK);
}


CopyPage7(int i,int j)
{
Handle ptr;
int ievent;

if((*pp_CsoundScoreText)[i] != NULL) {
   if((*pp_CsoundScoreText)[j] == NULL) {
      if((ptr = (Handle) GiveSpace(MyGetHandleSize((*pp_CsoundScoreText)[i])))
         == NULL) return(ABORT);
      (*pp_CsoundScoreText)[j] = ptr;
      }
   MystrcpyHandleToHandle(0,&((*pp_CsoundScoreText)[j]),(*pp_CsoundScoreText)[i]);
   }
else {
   ptr = (Handle) (*pp_CsoundScoreText)[j];
   if(MyDisposeHandle(&ptr) != OK) return(ABORT);
   (*pp_CsoundScoreText)[j] = NULL;
   if((*pp_CsoundScore)[j] != NULL) {
      for(ievent=0; ievent < (*p_CsoundSize)[j]; ievent++) {
         ptr = (Handle) (*((*pp_CsoundScore)[j]))[ievent].h_param;
         if(MyDisposeHandle(&ptr) != OK) return(ABORT);
         (*((*pp_CsoundScore)[j]))[ievent].h_param = NULL;
         }
      }
   ptr = (Handle) (*pp_CsoundScore)[j];
   if(MyDisposeHandle(&ptr) != OK) return(ABORT);
   (*pp_CsoundScore)[j] = NULL;
   ptr = (Handle) (*pp_CsoundTime)[j];
   if(MyDisposeHandle(&ptr) != OK) return(ABORT);
   (*pp_CsoundTime)[j] = NULL;
   (*p_CsoundSize)[j] = 0;
   CompiledCsObjects = (*p_CompiledCsoundScore)[j] = FALSE;
   }
#if BP_CARBON_GUI
SetCsoundScore(j);
#endif /* BP_CARBON_GUI */
return(OK);
}


CopyPage8(int i,int j)
{
(*p_DefaultChannel)[j] = (*p_DefaultChannel)[i];
(*p_OkTransp)[j] = (*p_OkTransp)[i];
(*p_OkPan)[j] = (*p_OkPan)[i];
(*p_OkMap)[j] = (*p_OkMap)[i];
(*p_OkVelocity)[j] = (*p_OkVelocity)[i];
(*p_OkArticul)[j] = (*p_OkArticul)[i];
(*p_OkVolume)[j] = (*p_OkVolume)[i];
(*p_CsoundInstr)[j] = (*p_CsoundInstr)[i];
(*p_CsoundAssignedInstr)[j] = (*p_CsoundAssignedInstr)[i];
return(OK);
}
