/* SoundObjects3.c (BP3) */

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

int trace_inconsistencies = 0;

int ResetPrototype(int j)
{
char line[MAXFIELDCONTENT];
Handle ptr;
int i;

if(j < 2 || j >= Jbol) {
   BPPrintMessage(0,odError,"=> Err. ResetPrototype()");
   return(MISSED);
   }
(*p_Type)[j] = 0;

ptr = (Handle)(*pp_MIDIcode)[j];
if(MyDisposeHandle(&ptr) != OK) return(ABORT);
(*pp_MIDIcode)[j] = NULL;
ptr = (Handle)(*pp_CsoundTime)[j];
if(MyDisposeHandle(&ptr) != OK) return(ABORT);
(*pp_CsoundTime)[j] = NULL;

(*p_ContBegMode)[j] = (*p_ContEndMode)[j] = FIXVALUE;
(*p_MaxBegGap)[j] = (*p_MaxEndGap)[j] = Infpos;
(*p_DelayMode)[j] = (*p_ForwardMode)[j] = FIXVALUE;
(*p_CoverBegMode)[j] = (*p_CoverEndMode)[j] = PERCENT;
(*p_TruncBegMode)[j] = (*p_TruncEndMode)[j] = (*p_PreRollMode)[j]
      = (*p_PostRollMode)[j] = PERCENT;
(*p_MaxCoverBeg)[j] = (*p_MaxCoverEnd)[j] = 100L;
(*p_MaxTruncBeg)[j] = (*p_MaxTruncEnd)[j] = 0L;
(*p_PivPos)[j] = (*p_PreRoll)[j] = (*p_PostRoll)[j] = (*p_CyclicAfter)[j] = ZERO;
(*p_CyclicMode)[j] = IRRELEVANT;

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
(*p_PivType)[j] = 1; (*p_PivMode)[j] = FIXVALUE;
(*p_RescaleMode)[j] = OK_RESCALE;
(*p_AlphaMin)[j] = 0; (*p_AlphaMax)[j] = 100.;
(*p_Dur)[j] = ZERO;
(*p_Resolution)[j] = 1;
(*p_Tref)[j] = 1000L;
(*p_ForceIntegerCycles)[j] = FALSE;
(*p_DefaultChannel)[j] = (*p_Quan)[j] = 0;
(*p_StrikeAgain)[j] = -1;
(*p_Tpict)[j] = ZERO;
ptr = (Handle)(*pp_Comment)[j];
if(MyDisposeHandle(&ptr) != OK) return(ABORT);
(*pp_Comment)[j] = NULL;
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
(*p_CsoundInstrumentMode)[j] = -1;
(*p_MIDIsize)[j] = (*p_CsoundSize)[j] = ZERO;
ChangedProtoType(j);
CompiledCsObjects = (*p_CompiledCsoundScore)[j] = 0;
return(OK);
}

/*
int CheckConsistency(int j,int check) {
   int k,bugg,longerCsound;
   long i,t,ton,toff;
   Milliseconds dur,maxcover1,maxcover2,maxtrunc1,maxtrunc2;
   double preroll,postroll;

   if(j >= Jbol || j < 2) return(OK);
   bugg = 0;

   if((*p_CsoundSize)[j] <= ZERO) {
      if((*p_Type)[j] & 4) {
         (*p_Type)[j] &= (255-4);
         (*p_CsoundInstr)[j] = 0;
         }
      }
   else  {
      if(!((*p_Type)[j] & 4)) {
         (*p_Type)[j] |= 4;
         }
      }
   if((*p_MIDIsize)[j] == ZERO) {
      if((*p_Type)[j] & 1) {
         (*p_Type)[j] &= (255-1);
         }
      }
      
   if((*p_Resolution)[j] <= ZERO) {
      if(trace_inconsistencies) BPPrintMessage(0,odError,"=> Err. CheckConsistency(). (*p_Resolution)[j] <= ZERO\n");
      return(MISSED);
      }
   if((*p_FixScale)[j]) {
      (*p_OkExpand)[j] = (*p_OkCompress)[j] = FALSE;
      }
   if((*p_Tref)[j] < EPSILON) {
      (*p_Tref)[j] = ZERO;
      }
   if((*p_PivType)[j] < 1 || (*p_PivType)[j] > 7) {
      if(trace_inconsistencies) BPPrintMessage(0,odInfo,"=> CheckConsistency() (*p_PivType)[%d] = %d\n",j,(*p_PivType)[j]);
      (*p_PivType)[j] = 1; bugg++;
      }

   SetPrototypeDuration(j,&longerCsound);
   dur = (*p_Dur)[j];
   BPPrintMessage(0,odInfo,"@@ %s, dur = %ld\n",*((*p_Bol)[j]),dur);
   if(dur < EPSILON) {
      (*p_PivType)[j] = 1; (*p_PivPos)[j] = ZERO;
      (*p_CoverBegMode)[j] = (*p_CoverEndMode)[j] = (*p_TruncBegMode)[j]
         = (*p_TruncEndMode)[j] = (*p_ContBegMode)[j] = (*p_ContEndMode)[j] = FIXVALUE;
      (*p_CoverBeg)[j] = (*p_CoverEnd)[j] = TRUE;
      (*p_MaxCoverBeg)[j] = (*p_MaxCoverEnd)[j] = ZERO;
      (*p_TruncBeg)[j] = (*p_TruncEnd)[j] = FALSE;
      (*p_MaxTruncBeg)[j] = (*p_MaxTruncEnd)[j] = ZERO;
      }
   return(OK);
   
   switch((*p_PivType)[j]) {
      case 1: (*p_PivPos)[j] = 0.; (*p_PivMode)[j] = FIXVALUE; break;
      case 2: (*p_PivPos)[j] = 100.; (*p_PivMode)[j] = PERCENT; break;
      case 5: (*p_PivPos)[j] = 50.; (*p_PivMode)[j] = PERCENT; break;
      case 7: break; // pivspec
      case 4: // endoff
      case 3: // begon
      case 6: // centonoff
      // if((*p_MIDIsize)[j] == ZERO && (*p_CsoundSize)[j] == ZERO) {
       //     if(trace_inconsistencies) BPPrintMessage(0,odInfo,"CheckConsistency() (*p_MIDIsize)[%d] = ZERO and (*p_CsoundSize)[%d] = ZERO\n",j,j);
        //   (*p_PivType)[j] = 1; bugg++; break;
      //     }
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
            if(trace_inconsistencies) BPPrintMessage(0,odInfo,"CheckConsistency() ton = %ld toff = %ld dur = %ld\n",(long)ton,(long)toff,(long)dur);
            (*p_PivType)[j] = 1; bugg++; break;
            }
         GetPrePostRoll(j,&preroll,&postroll);
         switch((*p_PivType)[j]) {
            case 3: // begon
               (*p_PivPos)[j] = ((float)(ton - preroll) * 100.) / dur;
               (*p_PivMode)[j] = PERCENT; break;
            case 4: // endoff
               (*p_PivPos)[j] = ((float)(toff - preroll) * 100.) / dur;
               (*p_PivMode)[j] = PERCENT; break;
            case 6: // centonoff
               (*p_PivPos)[j] = ((float)((ton+toff)/2. - preroll) * 100.) / dur;
               (*p_PivMode)[j] = PERCENT; break;
            }
      }
   if((*p_MaxCoverBeg)[j] < 0) {
      (*p_MaxCoverBeg)[j] = 0; bugg++;
      }
   if((*p_CoverBegMode)[j] == PERCENT) {
      if((*p_MaxCoverBeg)[j] > 100L) (*p_MaxCoverBeg)[j] = 100L;
      }
   if((*p_MaxCoverEnd)[j] < 0) {
      (*p_MaxCoverEnd)[j] = 0; bugg++;
      }
   if((*p_CoverEndMode)[j] == PERCENT) {
      if((*p_MaxCoverEnd)[j] > 100L) (*p_MaxCoverEnd)[j] = 100L;
      }
   if((*p_CoverBegMode)[j] == PERCENT && (*p_CoverEndMode)[j] == PERCENT) {
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
   if((*p_MaxTruncBeg)[j] < 0) {
      (*p_MaxTruncBeg)[j] = 0; bugg++;
      }
   if((*p_TruncBegMode)[j] == PERCENT) {
      if((*p_MaxTruncBeg)[j] > 100L) (*p_MaxTruncBeg)[j] = 100L;
      }
   if((*p_MaxTruncEnd)[j] < 0) {
      (*p_MaxTruncEnd)[j] = 0; bugg++;
      }
   if((*p_TruncEndMode)[j] == PERCENT) {
      if((*p_MaxTruncEnd)[j] > 100L) (*p_MaxTruncEnd)[j] = 100L;
      }
   if((*p_TruncBegMode)[j] == PERCENT && (*p_TruncEndMode)[j] == PERCENT) {
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
   if(check && (bugg > 0)) {
      BPPrintMessage(0,odInfo,"=> Found inconsistencies in sound-object prototype '%s'. These have been corrected.\n",(*p_Bol)[j]);
      }
   return(OK);
   } */


int PrototypeWindow(int w)
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