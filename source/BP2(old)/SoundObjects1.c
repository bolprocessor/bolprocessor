/* SoundObjects1.c (BP3) */

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


CheckLoadedPrototypes(void)
{
int r;
// GrafPtr saveport;

// GetPort(&saveport);  // FIXME ? is this necessary?
r = OK;
if(!PrototypesLoaded AND !ObjectMode && !ObjectTry && (OutMIDI || OutCsound || WriteMIDIfile)) {
   ObjectTry = TRUE;
   if(ResizeObjectSpace(YES,Jbol + Jpatt,0) != OK) {
      r = ABORT; goto OUT;
      }
   if(NeedAlphabet) {
      if(/*(FileName[iObjects][0] == '\0'
   	     && Answer("Since MIDI, MIDI file, or Csound output is active, you may need to "
   	               "load an object prototypes file. Would you like to do so", 'Y') != YES)
   	   || */ (r=LoadObjectPrototypes(NO,YES)) != OK) {
         ObjectMode = FALSE;
     //    FileName[iObjects][0] = '\0';
    //     SetName(iObjects,TRUE,TRUE);
         Dirty[iObjects] = Created[iObjects] = FALSE;
         iProto = 0;
         goto OUT;
         }
      else if(iProto >= Jbol) iProto = 2;
      }
   }
r = CompileCsoundObjects();

OUT:
// if(saveport != NULL) SetPort(saveport);
// else if(Beta) Alert1("=> Err CheckLoadedPrototypes(). saveport == NULL");
return(r);
}

#if BP_CARBON_GUI
ShowObjects(int w)
{
char failedstring[15];
int r;

if(Jbol < 3) {
   Alert1("No sound-object prototype has been created/loaded");
   return(OK);
   }
sprintf(failedstring,"Empty object");
r = DoThings(p_Bol,2,Jbol,NULL,16,MINUSPROC,failedstring,(int) pushButProc);
if(r > 0) iProto = r;
return(OK);
}
#endif

#if BP_CARBON_GUI

SetPrototype(int j)
// Copying from memory to dialogs
{
int r;
char line[MAXFIELDCONTENT];

if(Jbol <= 2) {
   SetField(NULL,wPrototype1,fInstrumentFileName,"[no file]");
   return(FAILED);
   }
if(j < 2) j = iProto = 2;
if(j >= Jbol) j = iProto = Jbol - 1;
if(CheckConsistency(j,FALSE) != OK) return(FAILED);

if((r=SetPrototypePage1(j)) != OK) return(r);
if((r=SetPrototypePage2(j)) != OK) return(r);
if((r=SetPrototypePage3(j)) != OK) return(r);
if((r=SetPrototypePage4(j)) != OK) return(r);
if((r=SetPrototypePage5(j)) != OK) return(r);
if((r=SetPrototypePage6(j)) != OK) return(r);
if((r=SetPrototypePage7(j)) != OK) return(r);
if((r=SetPrototypePage8(j)) != OK) return(r);

return(OK);
}


SetPrototypePage1(int j)
{
char line[MAXFIELDCONTENT];

if(j < 2 || j >= Jbol) return(FAILED);

if(DrawPrototype(j,wPrototype1,&PictFrame) != OK) return(ABORT);

if(FileName[wCsoundResources][0] != '\0')
   SetField(NULL,wPrototype1,fInstrumentFileName,FileName[wCsoundResources]);
else
   SetField(NULL,wPrototype1,fInstrumentFileName,"[no file]");

if((*pp_Comment)[j] != NULL)
   MystrcpyHandleToString(MAXFIELDCONTENT,0,line,(*pp_Comment)[j]);
else line[0] = '\0';
if(line[0] == '\0') strcpy(line,"[Comment on this prototype]");
SetField(NULL,wPrototype1,fPrototypeComment,line);

MystrcpyTableToString(MAXFIELDCONTENT,line,p_Bol,j);
sprintf(Message,"<<%s>>",line);
SetField(NULL,wPrototype1,fProtoName,Message);

sprintf(line,"%ld",(long)(*p_Dur)[j]);
SetField(NULL,wPrototype1,fDuration,line);

if((*p_MIDIsize)[j] > ZERO) ShowPannel(wPrototype1,bConvertToCsound);
else HidePannel(wPrototype1,bConvertToCsound);

if((*p_Type)[j] & 1) SwitchOn(NULL,wPrototype1,bMIDIsequence);
else SwitchOff(NULL,wPrototype1,bMIDIsequence);
if((*p_Type)[j] & 2) SwitchOn(NULL,wPrototype1,bSampledSound);
else SwitchOff(NULL,wPrototype1,bSampledSound);
if((*p_Type)[j] & 4) SwitchOn(NULL,wPrototype1,bCsoundInstrument);
else SwitchOff(NULL,wPrototype1,bCsoundInstrument);

if(Stream.imax > ZERO) ShowPannel(wPrototype1,bPasteSelection);
else HidePannel(wPrototype1,bPasteSelection);

if((*p_PasteDone)[j]) ShowPannel(wPrototype1,bUndoPasteSelection);
else HidePannel(wPrototype1,bUndoPasteSelection);

if((*p_MIDIsize)[j] > ZERO) ShowPannel(wPrototype1,bCapture);
else HidePannel(wPrototype1,bCapture);

return(DoSystem());
}


SetPrototypePage2(int j)
{
char line[MAXFIELDCONTENT];
int k;

if(j < 2 || j >= Jbol) return(FAILED);

// DURATION
if((*p_OkExpand)[j]) SwitchOn(NULL,wPrototype2,bExpandAtWill);
else SwitchOff(NULL,wPrototype2,bExpandAtWill);
if((*p_OkCompress)[j]) SwitchOn(NULL,wPrototype2,bCompressAtWill);
else SwitchOff(NULL,wPrototype2,bCompressAtWill);
if((*p_OkExpand)[j] && (*p_OkCompress)[j]) SwitchOn(NULL,wPrototype2,bOKrescale);
else SwitchOff(NULL,wPrototype2,bOKrescale);
if((*p_FixScale)[j]) SwitchOn(NULL,wPrototype2,bNeverRescale);
else SwitchOff(NULL,wPrototype2,bNeverRescale);
if(!(*p_OkExpand)[j] && !(*p_OkCompress)[j] && !(*p_FixScale)[j]) {
   sprintf(line,"%.2f",(*p_AlphaMin)[j]);
   SetField(NULL,wPrototype2,fMinDilationRatio,line);
   sprintf(line,"%.2f",(*p_AlphaMax)[j]);
   SetField(NULL,wPrototype2,fMaxDilationRatio,line);
   SwitchOn(NULL,wPrototype2,bDilationRatioRange);
   }
else SwitchOff(NULL,wPrototype2,bDilationRatioRange);
if((*p_AlphaCtrl)[j]) {
   SwitchOn(NULL,wPrototype2,bSendAlpha);
  // k = ByteToInt((*p_AlphaCtrlNr)[j]);
   k = (*p_AlphaCtrlNr)[j]; // Fixed by BB 2022-02-17
   sprintf(line,"%ld",(long) k);
   if((*p_AlphaCtrlNr)[j] >= 0 && (*p_AlphaCtrlNr)[j] < MAXPARAMCTRL)
      SetField(NULL,wPrototype2,fSendAlphaControl,line);
   else {
      SetField(NULL,wPrototype2,fSendAlphaControl,"[?]");
      SelectField(NULL,wPrototype2,fSendAlphaControl,TRUE);
      if((*p_AlphaCtrl)[iProto]) {
         Alert1("Indicate controller # for sending dilation ratio");
         BPActivateWindow(SLOW,wPrototype2);
         return(ABORT);
         }
      }
 //  k = ByteToInt((*p_AlphaCtrlChan)[j]);
   k = (*p_AlphaCtrlChan)[j]; // Fixed by BB 2022-02-18
   sprintf(line,"%ld",(long) k);
   if((*p_AlphaCtrlChan)[j] > 0 && (*p_AlphaCtrlChan)[j] <= MAXCHAN)
      SetField(NULL,wPrototype2,fSendAlphaChannel,line);
   else {
      SetField(NULL,wPrototype2,fSendAlphaChannel,"[?]");
      SelectField(NULL,wPrototype2,fSendAlphaChannel,TRUE);
      if((*p_AlphaCtrl)[iProto]) {
         Alert1("Indicate channel # for sending dilation ratio");
         BPActivateWindow(SLOW,wPrototype2);
         return(ABORT);
         }
      }
   }
else {
   SwitchOff(NULL,wPrototype2,bSendAlpha);
   }

// PIVOT
SwitchOff(NULL,wPrototype2,bPivBeg);
SwitchOff(NULL,wPrototype2,bPivEnd);
SwitchOff(NULL,wPrototype2,bPivFirstNoteOn);
SwitchOff(NULL,wPrototype2,bPivLastNoteOff);
SwitchOff(NULL,wPrototype2,bPivMiddle);
SwitchOff(NULL,wPrototype2,bPivMiddleNoteOnOff);
SwitchOff(NULL,wPrototype2,bSetPivotPC);
SwitchOff(NULL,wPrototype2,bSetPivotms);
switch((*p_PivType)[j]) {
   case 1: SwitchOn(NULL,wPrototype2,bPivBeg); break;
   case 2: SwitchOn(NULL,wPrototype2,bPivEnd); break;
   case 3: SwitchOn(NULL,wPrototype2,bPivFirstNoteOn); break;
   case 4: SwitchOn(NULL,wPrototype2,bPivLastNoteOff); break;
   case 5: SwitchOn(NULL,wPrototype2,bPivMiddle); break;
   case 6: SwitchOn(NULL,wPrototype2,bPivMiddleNoteOnOff); break;
   case 7:
      if((*p_PivMode)[j] == RELATIVE) {
         SwitchOn(NULL,wPrototype2,bSetPivotPC);
         WriteFloatToLine(line,(double)(*p_PivPos)[j]);
         SetField(NULL,wPrototype2,fSetPivotPC,line);
         sprintf(line,"%ld",(long)((*p_PivPos)[j] * (*p_Dur)[j] / 100.));
         SetField(NULL,wPrototype2,fSetPivotms,line);
         }
      if((*p_PivMode)[j] == ABSOLUTE) {
         SwitchOn(NULL,wPrototype2,bSetPivotms);
         sprintf(line,"%ld",(long)(*p_PivPos)[j]);
         SetField(NULL,wPrototype2,fSetPivotms,line);
         if((*p_Dur)[j] > EPSILON)
            WriteFloatToLine(line,(double)((*p_PivPos)[j] * 100.) / (*p_Dur)[j]);
         else sprintf(line,"0");
         SetField(NULL,wPrototype2,fSetPivotPC,line);
         }
      break;
   }

// LOCATION
if((*p_OkRelocate)[j]) {
   SwitchOn(NULL,wPrototype2,bRelocateAtWill);
   SwitchOff(NULL,wPrototype2,bNeverRelocate);
   SwitchOff(NULL,wPrototype2,bAllowDelayms);
   SwitchOff(NULL,wPrototype2,bAllowDelayPC);
   SwitchOff(NULL,wPrototype2,bAllowFwdms);
   SwitchOff(NULL,wPrototype2,bAllowFwdPC);
   }
else {
   SwitchOff(NULL,wPrototype2,bRelocateAtWill);
   if((*p_MaxDelay)[j] == ZERO && (*p_MaxForward)[j] == ZERO) {
      SwitchOn(NULL,wPrototype2,bNeverRelocate);
      SwitchOff(NULL,wPrototype2,bAllowDelayms);
      SwitchOff(NULL,wPrototype2,bAllowDelayPC);
      SwitchOff(NULL,wPrototype2,bAllowFwdms);
      SwitchOff(NULL,wPrototype2,bAllowFwdPC);
      }
   else {
      SwitchOff(NULL,wPrototype2,bNeverRelocate);
      if((*p_DelayMode)[j] == ABSOLUTE) {
         SwitchOn(NULL,wPrototype2,bAllowDelayms);
         SwitchOff(NULL,wPrototype2,bAllowDelayPC);
         sprintf(line,"%ld",(long)(*p_MaxDelay)[j]);
         SetField(NULL,wPrototype2,fAllowDelayms,line);
         if((*p_Dur)[j] > EPSILON)
            WriteFloatToLine(line,(double)((*p_MaxDelay)[j] * 100.) / (*p_Dur)[j]);
         else sprintf(line,"0");
         SetField(NULL,wPrototype2,fAllowDelayPC,line);
         }
      else {
         SwitchOn(NULL,wPrototype2,bAllowDelayPC);
         SwitchOff(NULL,wPrototype2,bAllowDelayms);
         WriteFloatToLine(line,(double)(*p_MaxDelay)[j]);
         SetField(NULL,wPrototype2,fAllowDelayPC,line);
         sprintf(line,"%ld",(long)((*p_MaxDelay)[j] * (*p_Dur)[j] / 100.));
         SetField(NULL,wPrototype2,fAllowDelayms,line);
         }
      if((*p_ForwardMode)[j] == ABSOLUTE) {
         SwitchOn(NULL,wPrototype2,bAllowFwdms);
         SwitchOff(NULL,wPrototype2,bAllowFwdPC);
         sprintf(line,"%ld",(long)(*p_MaxForward)[j]);
         SetField(NULL,wPrototype2,fAllowFwdms,line);
         if((*p_Dur)[j] > EPSILON)
            WriteFloatToLine(line,(double)((*p_MaxForward)[j] * 100.) / (*p_Dur)[j]);
         else sprintf(line,"0");
         SetField(NULL,wPrototype2,fAllowFwdPC,line);
         }
      else {
         SwitchOn(NULL,wPrototype2,bAllowFwdPC);
         SwitchOff(NULL,wPrototype2,bAllowFwdms);
         WriteFloatToLine(line,(double)(*p_MaxForward)[j]);
         SetField(NULL,wPrototype2,fAllowFwdPC,line);
         sprintf(line,"%ld",(long)((*p_MaxForward)[j] * (*p_Dur)[j] / 100.));
         SetField(NULL,wPrototype2,fAllowFwdms,line);
         }
      }
   }
return(DoSystem());
}


SetPrototypePage3(int j)
{
char line[MAXFIELDCONTENT];

if(j < 2 || j >= Jbol) return(FAILED);

// COVER BEGINNING
SwitchOff(NULL,wPrototype3,bNeverCoverBeg);
SwitchOff(NULL,wPrototype3,bCoverBegLessThanms);
SwitchOff(NULL,wPrototype3,bCoverBegLessThanPC);
if((*p_CoverBeg)[j]) {
   SwitchOn(NULL,wPrototype3,bCoverBegAtWill);
   SetField(NULL,wPrototype3,fCoverBegLessThanms,"\0");
   SetField(NULL,wPrototype3,fCoverBegLessThanPC,"\0");
   }
else {
   SwitchOff(NULL,wPrototype3,bCoverBegAtWill);
   if((*p_MaxCoverBeg)[j] > ZERO) {
      if((*p_CoverBegMode)[j] == ABSOLUTE) {
         SwitchOn(NULL,wPrototype3,bCoverBegLessThanms);
         SwitchOff(NULL,wPrototype3,bCoverBegLessThanPC);
         sprintf(line,"%ld",(long)(*p_MaxCoverBeg)[j]);
         SetField(NULL,wPrototype3,fCoverBegLessThanms,line);
         if((*p_Dur)[j] > EPSILON)
            WriteFloatToLine(line,(double)((*p_MaxCoverBeg)[j] * 100.) / (*p_Dur)[j]);
         else sprintf(line,"0");
         SetField(NULL,wPrototype3,fCoverBegLessThanPC,line);
         }
      else {
         SwitchOn(NULL,wPrototype3,bCoverBegLessThanPC);
         SwitchOff(NULL,wPrototype3,bCoverBegLessThanms);
         WriteFloatToLine(line,(double)(*p_MaxCoverBeg)[j]);
         SetField(NULL,wPrototype3,fCoverBegLessThanPC,line);
         sprintf(line,"%ld",(long)((*p_MaxCoverBeg)[j] * (*p_Dur)[j] / 100.));
         SetField(NULL,wPrototype3,fCoverBegLessThanms,line);
         }
      }
   else {
      SwitchOn(NULL,wPrototype3,bNeverCoverBeg);
      SetField(NULL,wPrototype3,fCoverBegLessThanms,"\0");
      SetField(NULL,wPrototype3,fCoverBegLessThanPC,"\0");
      }
   }

// COVER END
SwitchOff(NULL,wPrototype3,bNeverCoverEnd);
SwitchOff(NULL,wPrototype3,bCoverEndLessThanms);
SwitchOff(NULL,wPrototype3,bCoverEndLessThanPC);
if((*p_CoverEnd)[j]) {
   SwitchOn(NULL,wPrototype3,bCoverEndAtWill);
   SetField(NULL,wPrototype3,fCoverEndLessThanms,"\0");
   SetField(NULL,wPrototype3,fCoverEndLessThanPC,"\0");
   }
else {
   SwitchOff(NULL,wPrototype3,bCoverEndAtWill);
   if((*p_MaxCoverEnd)[j] > ZERO) {
      if((*p_CoverEndMode)[j] == ABSOLUTE) {
         SwitchOn(NULL,wPrototype3,bCoverEndLessThanms);
         SwitchOff(NULL,wPrototype3,bCoverEndLessThanPC);
         sprintf(line,"%ld",(long)(*p_MaxCoverEnd)[j]);
         SetField(NULL,wPrototype3,fCoverEndLessThanms,line);
         if((*p_Dur)[j] > EPSILON)
            WriteFloatToLine(line,(double)((*p_MaxCoverEnd)[j] * 100.) / (*p_Dur)[j]);
         else sprintf(line,"0");
         SetField(NULL,wPrototype3,fCoverEndLessThanPC,line);
         }
      else {
         SwitchOn(NULL,wPrototype3,bCoverEndLessThanPC);
         SwitchOff(NULL,wPrototype3,bCoverEndLessThanms);
         WriteFloatToLine(line,(double)(*p_MaxCoverEnd)[j]);
         SetField(NULL,wPrototype3,fCoverEndLessThanPC,line);
         sprintf(line,"%ld",(long)((*p_MaxCoverEnd)[j] * (*p_Dur)[j] / 100.));
         SetField(NULL,wPrototype3,fCoverEndLessThanms,line);
         }
      }
   else {
      SwitchOn(NULL,wPrototype3,bNeverCoverEnd);
      SetField(NULL,wPrototype3,fCoverEndLessThanms,"\0");
      SetField(NULL,wPrototype3,fCoverEndLessThanPC,"\0");
      }
   }

// TRUNCATE BEGINNING
SwitchOff(NULL,wPrototype3,bNeverTruncBeg);
SwitchOff(NULL,wPrototype3,bTruncBegLessThanms);
SwitchOff(NULL,wPrototype3,bTruncBegLessThanPC);
if((*p_TruncBeg)[j]) {
   SwitchOn(NULL,wPrototype3,bTruncBegAtWill);
   SetField(NULL,wPrototype3,fTruncBegLessThanPC,"\0");
   SetField(NULL,wPrototype3,fTruncBegLessThanms,"\0");
   }
else {
   SwitchOff(NULL,wPrototype3,bTruncBegAtWill);
   if((*p_MaxTruncBeg)[j] > ZERO) {
      if((*p_TruncBegMode)[j] == ABSOLUTE) {
         SwitchOn(NULL,wPrototype3,bTruncBegLessThanms);
         SwitchOff(NULL,wPrototype3,bTruncBegLessThanPC);
         sprintf(line,"%ld",(long)(*p_MaxTruncBeg)[j]);
         SetField(NULL,wPrototype3,fTruncBegLessThanms,line);
         if((*p_Dur)[j] > EPSILON)
            WriteFloatToLine(line,(double)((*p_MaxTruncBeg)[j] * 100.) / (*p_Dur)[j]);
         else sprintf(line,"0");
         SetField(NULL,wPrototype3,fTruncBegLessThanPC,line);
         }
      else {
         SwitchOn(NULL,wPrototype3,bTruncBegLessThanPC);
         SwitchOff(NULL,wPrototype3,bTruncBegLessThanms);
         WriteFloatToLine(line,(double)(*p_MaxTruncBeg)[j]);
         SetField(NULL,wPrototype3,fTruncBegLessThanPC,line);
         sprintf(line,"%ld",(long)((*p_MaxTruncBeg)[j] * (*p_Dur)[j] / 100.));
         SetField(NULL,wPrototype3,fTruncBegLessThanms,line);
         }
      }
   else {
      SwitchOn(NULL,wPrototype3,bNeverTruncBeg);
      SetField(NULL,wPrototype3,fTruncBegLessThanms,"\0");
      SetField(NULL,wPrototype3,fTruncBegLessThanPC,"\0");
      }
   }

// TRUNCATE END
SwitchOff(NULL,wPrototype3,bNeverTruncEnd);
SwitchOff(NULL,wPrototype3,bTruncEndLessThanms);
SwitchOff(NULL,wPrototype3,bTruncEndLessThanPC);
if((*p_TruncEnd)[j]) {
   SwitchOn(NULL,wPrototype3,bTruncEndAtWill);
   SetField(NULL,wPrototype3,fTruncEndLessThanPC,"\0");
   SetField(NULL,wPrototype3,fTruncEndLessThanms,"\0");
   }
else {
   SwitchOff(NULL,wPrototype3,bTruncEndAtWill);
   if((*p_MaxTruncEnd)[j] > ZERO) {
      if((*p_TruncEndMode)[j] == ABSOLUTE) {
         SwitchOn(NULL,wPrototype3,bTruncEndLessThanms);
         SwitchOff(NULL,wPrototype3,bTruncEndLessThanPC);
         sprintf(line,"%ld",(long)(*p_MaxTruncEnd)[j]);
         SetField(NULL,wPrototype3,fTruncEndLessThanms,line);
         if((*p_Dur)[j] > EPSILON)
            WriteFloatToLine(line,(double)((*p_MaxTruncEnd)[j] * 100.) / (*p_Dur)[j]);
         else sprintf(line,"0");
         SetField(NULL,wPrototype3,fTruncEndLessThanPC,line);
         }
      else {
         SwitchOn(NULL,wPrototype3,bTruncEndLessThanPC);
         SwitchOff(NULL,wPrototype3,bTruncEndLessThanms);
         WriteFloatToLine(line,(double)(*p_MaxTruncEnd)[j]);
         SetField(NULL,wPrototype3,fTruncEndLessThanPC,line);
         sprintf(line,"%ld",(long)((*p_MaxTruncEnd)[j] * (*p_Dur)[j] / 100.));
         SetField(NULL,wPrototype3,fTruncEndLessThanms,line);
         }
      }
   else {
      SwitchOn(NULL,wPrototype3,bNeverTruncEnd);
      SetField(NULL,wPrototype3,fTruncEndLessThanms,"\0");
      SetField(NULL,wPrototype3,fTruncEndLessThanPC,"\0");
      }
   }

// BREAK TEMPO
if((*p_BreakTempo)[j]) {
   SwitchOn(NULL,wPrototype3,bBreakTempoAtWill);
   SwitchOff(NULL,wPrototype3,bNeverBreakTempo);
/*   SwitchOff(NULL,wPrototype3,bBreakTempoLessThanms);
   SwitchOff(NULL,wPrototype3,bBreakTempoLessThanPC); */
   }
else {
   SwitchOn(NULL,wPrototype3,bNeverBreakTempo);
   SwitchOff(NULL,wPrototype3,bBreakTempoAtWill);
/*   if((*p_MaxBreakTempo)[j] > ZERO) {
      sprintf(line,"%ld",(long)(*p_MaxBreakTempo)[j]);
      if((*p_BreakTempoMode)[j] == ABSOLUTE) {
         SwitchOn(NULL,wPrototype3,bBreakTempoLessThanms);
         SwitchOff(NULL,wPrototype3,bBreakTempoLessThanPC);
         SetField(NULL,wPrototype3,fBreakTempoLessThanms,line);
         }
      else {
         SwitchOn(NULL,wPrototype3,bBreakTempoLessThanPC);
         SwitchOff(NULL,wPrototype3,bBreakTempoLessThanms);
         SetField(NULL,wPrototype3,fBreakTempoLessThanPC,line);
         }
      }
   else SwitchOn(NULL,wPrototype3,bNeverBreakTempo); */
   }
return(DoSystem());
}


SetPrototypePage4(int j)
{
char line[MAXFIELDCONTENT];

if(j < 2 || j >= Jbol) return(FAILED);

// FORCE CONTINUITY BEGINNING
if((*p_ContBeg)[j]) {
   SwitchOn(NULL,wPrototype4,bForceContBeg);
   SwitchOff(NULL,wPrototype4,bDontForceContBeg);
   if((*p_MaxBegGap)[j] > ZERO) {
      if((*p_ContBegMode)[j] == ABSOLUTE) {
         SwitchOn(NULL,wPrototype4,bAllowGapBegms);
         SwitchOff(NULL,wPrototype4,bAllowGapBegPC);
         sprintf(line,"%ld",(long)(*p_MaxBegGap)[j]);
         SetField(NULL,wPrototype4,fAllowGapBegms,line);
         if((*p_Dur)[j] > EPSILON)
            WriteFloatToLine(line,(double)((*p_MaxBegGap)[j] * 100.) / (*p_Dur)[j]);
         else sprintf(line,"0");
         SetField(NULL,wPrototype4,fAllowGapBegPC,line);
         }
      else {
         SwitchOn(NULL,wPrototype4,bAllowGapBegPC);
         SwitchOff(NULL,wPrototype4,bAllowGapBegms);
         WriteFloatToLine(line,(double)(*p_MaxBegGap)[j]);
         SetField(NULL,wPrototype4,fAllowGapBegPC,line);
         sprintf(line,"%ld",(long)((*p_MaxBegGap)[j] * (*p_Dur)[j] / 100.));
         SetField(NULL,wPrototype4,fAllowGapBegms,line);
         }
      }
   else {
      sprintf(line,"0");
      SetField(NULL,wPrototype4,fAllowGapBegms,line);
      SetField(NULL,wPrototype4,fAllowGapBegPC,line);
      }
   }
else {
   SwitchOn(NULL,wPrototype4,bDontForceContBeg);
   SwitchOff(NULL,wPrototype4,bForceContBeg);
   SwitchOff(NULL,wPrototype4,bAllowGapBegms);
   SwitchOff(NULL,wPrototype4,bAllowGapBegPC);
   SetField(NULL,wPrototype4,fAllowGapBegPC,"\0");
   SetField(NULL,wPrototype4,fAllowGapBegms,"\0");
   }

// FORCE CONTINUITY END
if((*p_ContEnd)[j]) {
   SwitchOn(NULL,wPrototype4,bForceContEnd);
   SwitchOff(NULL,wPrototype4,bDontForceContEnd);
   if((*p_MaxEndGap)[j] > ZERO) {
      if((*p_ContEndMode)[j] == ABSOLUTE) {
         SwitchOn(NULL,wPrototype4,bAllowGapEndms);
         SwitchOff(NULL,wPrototype4,bAllowGapEndPC);
         sprintf(line,"%ld",(long)(*p_MaxEndGap)[j]);
         SetField(NULL,wPrototype4,fAllowGapEndms,line);
         if((*p_Dur)[j] > EPSILON)
            WriteFloatToLine(line,(double)((*p_MaxEndGap)[j] * 100.) / (*p_Dur)[j]);
         else sprintf(line,"0");
         SetField(NULL,wPrototype4,fAllowGapEndPC,line);
         }
      else {
         SwitchOn(NULL,wPrototype4,bAllowGapEndPC);
         SwitchOff(NULL,wPrototype4,bAllowGapEndms);
         WriteFloatToLine(line,(double)(*p_MaxEndGap)[j]);
         SetField(NULL,wPrototype4,fAllowGapEndPC,line);
         sprintf(line,"%ld",(long)((*p_MaxEndGap)[j] * (*p_Dur)[j] / 100.));
         SetField(NULL,wPrototype4,fAllowGapEndms,line);
         }
      }
   else {
      sprintf(line,"0");
      SetField(NULL,wPrototype4,fAllowGapEndms,line);
      SetField(NULL,wPrototype4,fAllowGapEndPC,line);
      }
   }
else {
   SwitchOn(NULL,wPrototype4,bDontForceContEnd);
   SwitchOff(NULL,wPrototype4,bForceContEnd);
   SwitchOff(NULL,wPrototype4,bAllowGapEndms);
   SwitchOff(NULL,wPrototype4,bAllowGapEndPC);
   SetField(NULL,wPrototype4,fAllowGapEndPC,"\0");
   SetField(NULL,wPrototype4,fAllowGapEndms,"\0");
   }

// PRE-ROLL, POST-ROLL
sprintf(line,"%ld",(long)(*p_PreRoll)[j]);
SetField(NULL,wPrototype4,fPreRollms,line);
if((*p_Dur)[j] > EPSILON)
   WriteFloatToLine(line,(double)((*p_PreRoll)[j] * 100.) / (*p_Dur)[j]);
else sprintf(line,"0");
SetField(NULL,wPrototype4,fPreRollPC,line);
if((*p_PreRollMode)[j] == ABSOLUTE) {
   SwitchOn(NULL,wPrototype4,bPreRollms);
   SwitchOff(NULL,wPrototype4,bPreRollPC);
   }
else {
   SwitchOn(NULL,wPrototype4,bPreRollPC);
   SwitchOff(NULL,wPrototype4,bPreRollms);
   }
sprintf(line,"%ld",(long)(*p_PostRoll)[j]);
SetField(NULL,wPrototype4,fPostRollms,line);
if((*p_Dur)[j] > EPSILON)
   WriteFloatToLine(line,(double)((*p_PostRoll)[j] * 100.) / (*p_Dur)[j]);
else sprintf(line,"0");
SetField(NULL,wPrototype4,fPostRollPC,line);
if((*p_PostRollMode)[j] == ABSOLUTE) {
   SwitchOn(NULL,wPrototype4,bPostRollms);
   SwitchOff(NULL,wPrototype4,bPostRollPC);
   }
else {
   SwitchOn(NULL,wPrototype4,bPostRollPC);
   SwitchOff(NULL,wPrototype4,bPostRollms);
   }
return(DoSystem());
}


SetPrototypePage5(int j)
{
char line[MAXFIELDCONTENT];
double beats;

if(j < 2 || j >= Jbol) return(FAILED);

if((*p_Tref)[j] > EPSILON) {
   sprintf(line,"%ld",(long)(*p_Tref)[j]);
   SetField(NULL,wPrototype5,fTref,line);
   SwitchOn(NULL,wPrototype5,bStriatedObject);
   }
else {
   SwitchOff(NULL,wPrototype5,bStriatedObject);
   }
if((*p_Quan)[j] > 0.01) {
   sprintf(line,"%ld",(long)((double)(*p_Tref)[j] / (*p_Quan)[j]));
   SetField(NULL,wPrototype5,fQuantizeFractionBeat,line);
   }
sprintf(line,"%ld",(long)(*p_Dur)[j]);
SetField(NULL,wPrototype5,fDurationAdjustms,line);
if((*p_Tref)[j] > EPSILON && (*p_Dur)[j] > EPSILON) {
   beats = ((double)(*p_Dur)[j]) / (*p_Tref)[j];
   sprintf(line,"%.3f",beats);
   SetField(NULL,wPrototype5,fCurrentBeats,line);
   if((beats - (int)beats) > 0.5) beats = 1. + (int) beats;
   else beats = (int) beats;
   if(beats < 1.) beats = 1.;
   sprintf(line,"%.1f",beats);
   SetField(NULL,wPrototype5,fDurationAdjustbeats,line);
   }
else {
   SetField(NULL,wPrototype5,fCurrentBeats,"\0");
   SetField(NULL,wPrototype5,fDurationAdjustbeats,"\0");
   }
sprintf(line,"%ld",(long)PrototypeTickKey);
SetField(NULL,wPrototype5,fPrototypeTickKey,line);
sprintf(line,"%ld",(long)PrototypeTickChannel);
SetField(NULL,wPrototype5,fPrototypeTickChannel,line);
sprintf(line,"%ld",(long)PrototypeTickVelocity);
SetField(NULL,wPrototype5,fPrototypeTickVelocity,line);
return(DoSystem());
}


SetPrototypePage6(int j)
{
char line[MAXFIELDCONTENT];

if(j < 2 || j >= Jbol) return(FAILED);

// PERIOD
WriteFloatToLine(line,(double)(*p_BeforePeriod)[j]);
if((*p_PeriodMode)[j] == ABSOLUTE) {
   SwitchOn(NULL,wPrototype6,bBeforePeriodms);
   SwitchOff(NULL,wPrototype6,bBeforePeriodPC);
   SwitchOff(NULL,wPrototype6,bIrrelevantPeriod);
   SetField(NULL,wPrototype6,fBeforePeriodms,line);
   if((*p_Dur)[j] > EPSILON)
      WriteFloatToLine(line,((double)100. * (*p_BeforePeriod)[j]) / (*p_Dur)[j]);
   else
      sprintf(line,"0");
   SetField(NULL,wPrototype6,fBeforePeriodPC,line);
   }
if((*p_PeriodMode)[j] == RELATIVE) {
   SwitchOn(NULL,wPrototype6,bBeforePeriodPC);
   SwitchOff(NULL,wPrototype6,bBeforePeriodms);
   SwitchOff(NULL,wPrototype6,bIrrelevantPeriod);
   SetField(NULL,wPrototype6,fBeforePeriodPC,line);
   sprintf(line,"%.1f",((double)(*p_BeforePeriod)[j] * (*p_Dur)[j]) / 100L);
   SetField(NULL,wPrototype6,fBeforePeriodms,line);
   }
if((*p_PeriodMode)[j] == IRRELEVANT) {
   SwitchOff(NULL,wPrototype6,bBeforePeriodPC);
   SwitchOff(NULL,wPrototype6,bBeforePeriodms);
   SwitchOn(NULL,wPrototype6,bIrrelevantPeriod);
   SetField(NULL,wPrototype6,fBeforePeriodPC,"\0");
   SetField(NULL,wPrototype6,fBeforePeriodms,"\0");
   }
if((*p_ForceIntegerPeriod)[j]) SwitchOn(NULL,wPrototype6,bForceIntegerPeriod);
else SwitchOff(NULL,wPrototype6,bForceIntegerPeriod);
if((*p_DiscardNoteOffs)[j]) SwitchOn(NULL,wPrototype6,bDiscardNoteOffs);
else SwitchOff(NULL,wPrototype6,bDiscardNoteOffs);
switch((*p_StrikeAgain)[j]) {
   case -1:
      SwitchOff(NULL,wPrototype6,bStrikeAgain);
      SwitchOff(NULL,wPrototype6,bDontStrikeAgain);
      SwitchOn(NULL,wPrototype6,bStrikeDefault);
      break;
   case TRUE:
      SwitchOn(NULL,wPrototype6,bStrikeAgain);
      SwitchOff(NULL,wPrototype6,bDontStrikeAgain);
      SwitchOff(NULL,wPrototype6,bStrikeDefault);
      break;
   case FALSE:
      SwitchOff(NULL,wPrototype6,bStrikeAgain);
      SwitchOn(NULL,wPrototype6,bDontStrikeAgain);
      SwitchOff(NULL,wPrototype6,bStrikeDefault);
      break;
   }
return(DoSystem());
}


SetPrototypePage7(int j)
{
char line[MAXFIELDCONTENT];

if(j < 2 || j >= Jbol) return(FAILED);

return(OK);
}


SetPrototypePage8(int j)
{
char line[MAXFIELDCONTENT];

if(j < 2 || j >= Jbol) return(FAILED);

// MIDI
if((*p_DefaultChannel)[j] > 0) {
   sprintf(line,"%ld",(long)(*p_DefaultChannel)[j]);
   SetField(NULL,wPrototype8,fForceToChannel,line);
   SwitchOn(NULL,wPrototype8,bForceToChannel);
   SwitchOff(NULL,wPrototype8,bDontChangeChannel);
   SwitchOff(NULL,wPrototype8,bForceCurrChannel);
   }
else {
   SwitchOff(NULL,wPrototype8,bForceToChannel);
   if((*p_DefaultChannel)[j] < 0) {
      SwitchOn(NULL,wPrototype8,bDontChangeChannel);
      SwitchOff(NULL,wPrototype8,bForceCurrChannel);
      }
   else {
      SwitchOn(NULL,wPrototype8,bForceCurrChannel);
      SwitchOff(NULL,wPrototype8,bDontChangeChannel);
      }
   }

// CSOUND
if((*p_CsoundInstr)[j] > 0) {
   sprintf(line,"%ld",(long)(*p_CsoundInstr)[j]);
   SetField(NULL,wPrototype8,fForceToInstrument,line);
   SwitchOn(NULL,wPrototype8,bForceToInstrument);
   SwitchOff(NULL,wPrototype8,bDontChangeInstrument);
   SwitchOff(NULL,wPrototype8,bForceCurrInstrument);
   }
else {
   SwitchOff(NULL,wPrototype8,bForceToInstrument);
   if((*p_CsoundInstr)[j] < 0) {
      SwitchOn(NULL,wPrototype8,bDontChangeInstrument);
      SwitchOff(NULL,wPrototype8,bForceCurrInstrument);
      }
   else {
      SwitchOn(NULL,wPrototype8,bForceCurrInstrument);
      SwitchOff(NULL,wPrototype8,bDontChangeInstrument);
      }
   }

// CHANGES
if((*p_OkTransp)[j]) SwitchOn(NULL,wPrototype8,bAcceptTransposition);
else SwitchOff(NULL,wPrototype8,bAcceptTransposition);
if((*p_OkPan)[j]) SwitchOn(NULL,wPrototype8,bAcceptPanoramic);
else SwitchOff(NULL,wPrototype8,bAcceptPanoramic);
if((*p_OkArticul)[j]) SwitchOn(NULL,wPrototype8,bAcceptArticulation);
else SwitchOff(NULL,wPrototype8,bAcceptArticulation);
if((*p_OkVolume)[j]) SwitchOn(NULL,wPrototype8,bAcceptVolume);
else SwitchOff(NULL,wPrototype8,bAcceptVolume);
if((*p_OkMap)[j]) SwitchOn(NULL,wPrototype8,bAcceptMap);
else SwitchOff(NULL,wPrototype8,bAcceptMap);
if((*p_OkVelocity)[j]) SwitchOn(NULL,wPrototype8,bAcceptVelocity);
else SwitchOff(NULL,wPrototype8,bAcceptVelocity);

if((*p_CsoundAssignedInstr)[j] < 1) strcpy(line,"[None]");
else sprintf(line,"%ld",(long)(*p_CsoundAssignedInstr)[j]);
SetField(NULL,wPrototype8,fAssignInstrument,line);

return(DoSystem());
}

#endif /* BP_CARBON_GUI */
