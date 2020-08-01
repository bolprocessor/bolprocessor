/* Translit.c (BP2 version CVS) */

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

MakeGrammarFromTable(int appendtogrammar)
{
char c,**h_comment,**h_name,**p_line,**p_completeline;
int result,dirty,changed,ja,jb,jcomment,irowmin,irowmax,i,imax,j,irow,errors,rulefound,rr;
char apref,bpref,ord,rnd,lin,sub,sub1,poslong,lr,lrrl,rl,appendtoalphabet,quotes,
	leftis,leftisascii,leftistext,leftisnum,rightis,rightisascii,rightistext,rightisnum,
	removespacesleft,removespacesright;
long p,q,pos;
char line[MAXFIELDCONTENT],filename[64];
Rect r;
Handle itemhandle;
short item,itemtype,refnum;
FSSpec spec;
DialogPtr grammarfromtableptr;
Str255 fn;
OSErr io;

h_comment = h_name = p_line = p_completeline = NULL;
poslong = TRUE;
rnd = lin = sub = ord = sub1 = FALSE;
lr = TRUE; lrrl = rl = FALSE;
leftisascii = rightisascii = TRUE;
leftistext = leftisnum = rightistext = rightisnum = FALSE;
appendtoalphabet = removespacesleft = removespacesright = TRUE;
quotes = rulefound = FALSE;
errors = 0;
result = OK;

dirty = FALSE;

if(ClearWindow(NO,wScrap) != OK) return(ABORT);

ShowMessage(TRUE,wMessage,"Open table in tabulated text format...");

if(OldFile(-1,1,fn,&spec)) {
	p2cstrcpy(filename,fn);
	if((io=MyOpen(&spec,fsRdPerm,&refnum)) != noErr) {
		sprintf(Message,"Error opening %s",filename);
		ShowMessage(TRUE,wMessage,Message);
		TellError(100,io);
		ClearMessage();
		return(FAILED);
		}
	}
else return(FAILED);
ClearMessage();

SelectBehind(GetTextLength(wTrace),GetTextLength(wTrace),TEH[wTrace]);
PrintBehind(wTrace,"\n");
SelectBehind(GetTextLength(wGrammar),GetTextLength(wGrammar),TEH[wGrammar]);

LoadOn++;

pos = ZERO;

TRYREAD:

if(ReadOne(FALSE,FALSE,FALSE,refnum,FALSE,&p_line,&p_completeline,&pos) != OK) {
	Alert1("This file is empty or unreadable");
	result = ABORT;
	goto OVER;
	}
StripHandle(p_line);
imax = MyHandleLen(p_line);
if(imax == 0) goto TRYREAD;

grammarfromtableptr = GetNewDialog(GrammarFromTableID,0L,0L);
ResetGrammarFromTableDialog(grammarfromtableptr);

BPActivateWindow(SLOW,wNotice);
PreviewLine(p_line,TRUE);


TRYENTER:

TextUpdate(wNotice);
ShowSelect(CENTRE,wNotice);
ShowWindow(GetDialogWindow(grammarfromtableptr));
SelectWindow(GetDialogWindow(grammarfromtableptr));
result = FAILED;
changed = TRUE;
while(TRUE) {
	if(changed) {
		if(ord) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableORD);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableORD);
		if(rnd) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableRND);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableRND);
		if(lin) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableLIN);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableLIN);
		if(sub) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableSUB);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableSUB);
		if(sub1) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableSUB1);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableSUB1);
		if(poslong) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTablePOSLONG);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTablePOSLONG);
		
		if(lr) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableLR);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableLR);
		if(lrrl) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableLRRL);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableLRRL);
		if(rl) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableRL);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableRL);
		
		if(leftisascii) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableLeftIsASCII);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableLeftIsASCII);
		if(leftistext) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableLeftIsText);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableLeftIsText);
		if(leftisnum) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableLeftIsNum);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableLeftIsNum);
		
		if(rightisascii) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableRightIsASCII);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableRightIsASCII);
		if(rightistext) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableRightIsText);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableRightIsText);
		if(rightisnum) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableRightIsNum);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableRightIsNum);
		
		if(appendtoalphabet) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableAppend);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableAppend);
		if(quotes) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableQuotes);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableQuotes);
		
		if(removespacesleft) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableLeftRemoveSpaces);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableLeftRemoveSpaces);
		if(removespacesright) SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableRightRemoveSpaces);
		else SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableRightRemoveSpaces);

		BPUpdateDialog(grammarfromtableptr);
	}
	MaintainCursor();
	ModalDialog((ModalFilterUPP) 0L,&item);
	changed = FALSE;
	switch(item) {
		case bMakeGrammarFromTableProceed:
			result = OK; break;
		case bMakeGrammarFromTableCancel:
			result = ABORT; break;
		case bMakeGrammarFromTableLoad:
			/* $$$ */
			break;
		case bMakeGrammarFromTableSave:
//			SaveGrammarFromTableSettings();
			break;
		case bMakeGrammarFromTableSetDefault:
			if(!dirty || Answer("Reset all fields to default in this dialog",'N') == OK) {
				ResetGrammarFromTableDialog(grammarfromtableptr);
				poslong = TRUE;
				rnd = lin = sub = ord = sub1 = FALSE;
				lr = TRUE; lrrl = rl = FALSE;
				leftisascii = rightisascii = TRUE;
				leftistext = leftisnum = rightistext = rightisnum = FALSE;
				appendtoalphabet = TRUE;
				quotes = FALSE;
				dirty = FALSE;
				changed = TRUE;
				}
			break;
			
		case bMakeGrammarFromTableORD:
			changed = dirty = TRUE;
			ord = TRUE;
			rnd = lin = sub = sub1 = poslong = FALSE;
			break;
		case bMakeGrammarFromTableRND:
			changed = dirty = TRUE;
			rnd = TRUE;
			ord = lin = sub = sub1 = poslong = FALSE;
			break;
		case bMakeGrammarFromTableLIN:
			changed = dirty = TRUE;
			lin = TRUE;
			rnd = ord = sub = sub1 = poslong = FALSE;
			break;
		case bMakeGrammarFromTableSUB:
			changed = dirty = TRUE;
			sub = TRUE;
			rnd = lin = ord = sub1 = poslong = FALSE;
			break;
		case bMakeGrammarFromTableSUB1:
			changed = dirty = TRUE;
			sub1 = TRUE;
			rnd = lin = ord = sub = poslong = FALSE;
			break;
		case bMakeGrammarFromTablePOSLONG:
			changed = dirty = TRUE;
			poslong = TRUE;
			rnd = lin = ord = sub = sub1 = FALSE;
			break;
			
		case bMakeGrammarFromTableLR:
			changed = dirty = TRUE;
			lr = TRUE;
			lrrl = rl = FALSE;
			break;
		case bMakeGrammarFromTableLRRL:
			changed = dirty = TRUE;
			lrrl = TRUE;
			lr = rl = FALSE;
			break;
		case bMakeGrammarFromTableRL:
			changed = dirty = TRUE;
			rl = TRUE;
			lrrl = lr = FALSE;
			break;
			
		case bMakeGrammarFromTableLeftIsASCII:
			changed = dirty = TRUE;
			leftisascii = TRUE;
			leftistext = leftisnum = FALSE;
			break;
		case bMakeGrammarFromTableLeftIsText:
			changed = dirty = TRUE;
			leftistext = TRUE;
			leftisascii = leftisnum = FALSE;
			GetDialogItem(grammarfromtableptr,fMakeGrammarFromTableLeftPrefix,&itemtype,&itemhandle,&r);
			SetDialogItemText(itemhandle,"\p");
			break;	
		case bMakeGrammarFromTableLeftIsNum:
			changed = dirty = TRUE;
			if(rightisnum) {
				Alert1("Columns can\'t be both in numeric format...");
				break;
				}
			leftisnum = TRUE;
			leftistext = leftisascii = FALSE;
			GetDialogItem(grammarfromtableptr,fMakeGrammarFromTableLeftPrefix,&itemtype,&itemhandle,&r);
			SetDialogItemText(itemhandle,"\p#");
			break;
			
		case bMakeGrammarFromTableRightIsASCII:
			changed = dirty = TRUE;
			rightisascii = TRUE;
			rightistext = rightisnum = FALSE;
			break;
		case bMakeGrammarFromTableRightIsText:
			changed = dirty = TRUE;
			rightistext = TRUE;
			rightisascii = rightisnum = FALSE;
			GetDialogItem(grammarfromtableptr,fMakeGrammarFromTableRightPrefix,&itemtype,&itemhandle,&r);
			SetDialogItemText(itemhandle,"\p");
			break;	
		case bMakeGrammarFromTableRightIsNum:
			changed = dirty = TRUE;
			if(leftisnum) {
				Alert1("Columns can\'t be both in numeric format...");
				break;
				}
			rightisnum = TRUE;
			rightistext = rightisascii = FALSE;
			GetDialogItem(grammarfromtableptr,fMakeGrammarFromTableRightPrefix,&itemtype,&itemhandle,&r);
			SetDialogItemText(itemhandle,"\p#");
			break;
			
		case bMakeGrammarFromTableAppend:
			changed = dirty = TRUE;
			appendtoalphabet = TRUE;
			quotes = FALSE;
			break;
		case bMakeGrammarFromTableQuotes:
			changed = dirty = TRUE;
			quotes = TRUE;
			appendtoalphabet = FALSE;
			break;
			
		case bMakeGrammarFromTableLeftRemoveSpaces:
			changed = dirty = TRUE;
			if(!leftisascii) Alert1("This option is only effective in ASCII format");
			removespacesleft = 1 - removespacesleft;
			break;	
		case bMakeGrammarFromTableRightRemoveSpaces:
			changed = dirty = TRUE;
			if(!rightisascii) Alert1("This option is only effective in ASCII format");
			removespacesright = 1 - removespacesright;
			break;
		
		case bMakeGrammarFromTableShowNext:
			if(ReadOne(FALSE,FALSE,FALSE,refnum,FALSE,&p_line,&p_completeline,&pos) != FAILED) {
				PreviewLine(p_line,TRUE);
				goto TRYENTER;
				}
			break;
		}
	if(result == OK || result == ABORT) break;
	}
if(result == ABORT) goto CLEARDIALOG;

changed = TRUE;

GetField(grammarfromtableptr,FALSE,-1,fMakeGrammarFromTableLeftArgCol,line,&p,&q);
Strip(line);
if(strlen(line) > 1 || !isalpha(c=line[0]) || c < 'A' || c > 'Z') {
	Alert1("Column must be single character in range A..Z");
	SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableLeftArgCol,TRUE);
	goto TRYENTER;
	}
ja = c - 'A';

GetField(grammarfromtableptr,FALSE,-1,fMakeGrammarFromTableRightArgCol,line,&p,&q);
Strip(line);
if(strlen(line) > 1 || !isalpha(c=line[0]) || c < 'A' || c > 'Z') {
	Alert1("Column must be single character in range A..Z");
	SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableRightArgCol,TRUE);
	goto TRYENTER;
	}
jb = c - 'A';

jcomment = -1;
GetField(grammarfromtableptr,FALSE,-1,fMakeGrammarFromTableCommentCol,line,&p,&q);
Strip(line);
if(line[0] != '\0') {
	if(strlen(line) > 1 || !isalpha(c=line[0]) || c < 'A' || c > 'Z') {
		Alert1("Column must be single character in range A..Z");
		SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableCommentCol,TRUE);
		goto TRYENTER;
		}
	jcomment = c - 'A';
	if(jcomment == ja || jcomment == jb) {
		Alert1("This column can't be for comments as it is already used for data");
		SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableCommentCol,TRUE);
		goto TRYENTER;
		}
	}

if(ja == jb) {
	Alert1("Left-argument and right-argument columns can't be identical");
	SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableRightArgCol,TRUE);
	goto TRYENTER;
	}

GetField(grammarfromtableptr,TRUE,-1,fMakeGrammarFromTableFirstRow,line,&p,&q);
irowmin = p / q;
if(irowmin < 1) {
	Alert1("First row must be a positive integer");
	SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableFirstRow,TRUE);
	goto TRYENTER;
	}
	
GetField(grammarfromtableptr,FALSE,-1,fMakeGrammarFromTableLastRow,line,&p,&q);
Strip(line);
if(line[0] == '\0') irowmax = 0;
else {
	irowmax = p / q;
	if(irowmax < 1) {
		Alert1("Last row must be a positive integer - leave blank to read the table up to the end");
		SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableLastRow,TRUE);
		goto TRYENTER;
		}
	}
	
if(irowmax != 0 && irowmax < irowmin) {
	Alert1("Last row can't be less than first row");
	SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableLastRow,TRUE);
	goto TRYENTER;
	}

GetField(grammarfromtableptr,FALSE,-1,fMakeGrammarFromTableLeftPrefix,line,&p,&q);
Strip(line);
apref=line[0];
if(strlen(line) > 1) {
	Alert1("Prefix must be single character");
	SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableLeftPrefix,TRUE);
	goto TRYENTER;
	}
if(leftisnum) {
	if(apref == '\0') apref = '#';
	if(apref != '#') {
		Alert1("Prefix of terminal symbols must be \'#\' when their format is \'numeric\'");
		SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableLeftPrefix,TRUE);
		goto TRYENTER;
		}
	}
if(leftistext && apref != '\0') {
	Alert1("No prefix is needed when format is \'text\'");
	SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableLeftPrefix,TRUE);
	goto TRYENTER;
	}
if(leftisascii && (apref == '\0' || !islower(apref))) {
	Alert1("Prefix must be single character in range a..z");
	SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableLeftPrefix,TRUE);
	goto TRYENTER;
	}
	
GetField(grammarfromtableptr,FALSE,-1,fMakeGrammarFromTableRightPrefix,line,&p,&q);
Strip(line);
bpref=line[0];
if(strlen(line) > 1) {
	Alert1("Prefix must be single character");
	SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableRightPrefix,TRUE);
	goto TRYENTER;
	}
if(rightisnum) {
	if(bpref == '\0') bpref = '#';
	if(bpref != '#') {
		Alert1("Prefix of terminal symbols must be \'#\' when their format is \'numeric\'");
		SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableRightPrefix,TRUE);
		goto TRYENTER;
		}
	}
if(rightistext && bpref != '\0') {
	Alert1("No prefix is needed when format is \'text\'");
	SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableRightPrefix,TRUE);
	goto TRYENTER;
	}
if(rightisascii && (bpref == '\0' || !islower(bpref))) {
	Alert1("Prefix must be single character in range a..z");
	SelectField(grammarfromtableptr,-1,fMakeGrammarFromTableRightPrefix,TRUE);
	goto TRYENTER;
	}
	
GetField(grammarfromtableptr,FALSE,-1,fMakeGrammarFromTableSettingsComment,line,&p,&q);
Strip(line);
MystrcpyStringToHandle(&h_comment,line);
GetField(grammarfromtableptr,FALSE,-1,fMakeGrammarFromTableSettingsName,line,&p,&q);
Strip(line);
MystrcpyStringToHandle(&h_name,line);

/* if(dirty) {
	reply = Answer("Save this configuration",'Y');
	if(reply == CANCEL) goto TRYENTER;
	SaveGrammarFromTableSettings();
	} */

CLEARDIALOG:

DisposeDialog(grammarfromtableptr);
MyDisposeHandle(&h_comment);
MyDisposeHandle(&h_name);
if(result == ABORT) goto OVER;

if(leftisascii) leftis = 0;
if(leftistext) leftis = 1;
if(leftisnum) leftis = 2;
if(rightisascii) rightis = 0;
if(rightistext) rightis = 1;
if(rightisnum) rightis = 2;

// Now we read the table...

PrintBehind(wScrap,"Grammar with original line numbers (before compiling):\n\n");
if(appendtoalphabet) {
	if(!IsEmpty(wAlphabet)) {
		if((result=Answer("Clear existing alphabet",'N')) == ABORT) return(ABORT);
		if(result == YES) {
			if(ClearWindow(NO,wAlphabet) != OK) return(FAILED);
			RemoveFirstLine(wData,FilePrefix[wAlphabet]);
			RemoveFirstLine(wGrammar,FilePrefix[wAlphabet]);
			ForgetFileName(wAlphabet);
			}
		SelectBehind(GetTextLength(wAlphabet),GetTextLength(wAlphabet),TEH[wAlphabet]);
		PrintBehind(wAlphabet,"\n");
		}
	UpdateDirty(TRUE,wAlphabet);
	}
if(appendtogrammar) Print(wGrammar,"\n-------------\n");
Date(line);
sprintf(Message,"// Created from file '%s' - %s\n",filename,line);
Println(wGrammar,Message);
if(rnd) Println(wGrammar,"RND");
if(ord) Println(wGrammar,"ORD");
if(lin) Println(wGrammar,"LIN");
if(sub) Println(wGrammar,"SUB");
if(sub1) Println(wGrammar,"SUB1");
if(poslong) Println(wGrammar,"POSLONG");
UpdateDirty(TRUE,wGrammar);

pos = ZERO;
irow = 0;
result = OK;

pos = ZERO;
SetFPos(refnum,fsFromStart,ZERO);

while(TRUE) {
	PleaseWait();
#if BP_CARBON_GUI
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	if(MyButton(1)) {
		while((result = MainEvent()) != RESUME && result != STOP && result != EXIT && result != ABORT){};
		if(result == EXIT || result == ABORT || result == STOP) goto OVER;
		result = OK;
		}
#endif /* BP_CARBON_GUI */

	if((rr=ReadOne(FALSE,FALSE,FALSE,refnum,FALSE,&p_line,&p_completeline,&pos)) == FAILED) break;
	irow++;
	if((*p_line)[0] == '\0') goto READMORE;
	if(irow < irowmin) goto READMORE;
	if(irowmax > 0 && irow >= irowmax) break;
	sprintf(Message,"[%ld] ",(long)irow);
	Print(wGrammar,Message);
	PrintBehind(wScrap,Message);
	imax = MyHandleLen(p_line);
	i = 0;
	for(j=0; j < ja; j++) {
		while((c=(*p_line)[i]) != '\t' && c != '\0') {
			if(i >= imax) break;
			i++;
			}
		if(c == '\0' || i >= imax) break;
		i++;
		}
	if(GetArgumentInTableLine(1,p_line,i,apref,leftis,appendtoalphabet,removespacesleft) != OK) {
		Print(wGrammar,"\n");
		PrintBehind(wScrap,"\n");
		errors++;
		goto READMORE;
		}
	else rulefound = TRUE;
	
	if(lr) sprintf(Message," %s ",Arrow[1]);
	if(lrrl) sprintf(Message," %s ",Arrow[0]);
	if(rl) sprintf(Message," %s ",Arrow[2]);
	Print(wGrammar,Message);
	PrintBehind(wScrap,Message);
	
	i = 0;
	for(j=0; j < jb; j++) {
		while((c=(*p_line)[i]) != '\t' && c != '\0') {
			if(i >= imax) break;
			i++;
			}
		if(c == '\0' || i >= imax) break;
		i++;
		}
	GetArgumentInTableLine(2,p_line,i,bpref,rightis,appendtoalphabet,removespacesright);
	if(jcomment > -1) {
		i = 0;
		for(j=0; j < jcomment; j++) {
			while((c=(*p_line)[i]) != '\t' && c != '\0') {
				if(i >= imax) break;
				i++;
				}
			if(c == '\0' || i >= imax) break;
			i++;
			}
		j = 0;
		while((c=(*p_line)[i]) != '\t' && c != '\0') {
			if(i >= imax || j >= (MAXFIELDCONTENT-1)) break;
			if(isspace(c)) c = ' ';
			line[j++] = c;
			i++;
			}
		line[j] = '\0';
		if(line[0]  != '\0') {
			sprintf(Message," [%s]",line);
			Print(wGrammar,Message);
			PrintBehind(wScrap,Message);
			}
		}
	Print(wGrammar,"\n");
	PrintBehind(wScrap,"\n");

READMORE:
	if(rr == STOP) break;
	}

OVER:

LoadOn--;

if(result != OK) Println(wGrammar,"\n\n<< Reading grammar from table aborted >>");
if(appendtoalphabet) {
	BPActivateWindow(SLOW,wAlphabet);
	ShowSelect(CENTRE,wAlphabet);
	}
BPActivateWindow(SLOW,wGrammar);
ShowSelect(CENTRE,wGrammar);
if(errors > 0) {
	Alert1("Incomplete lines (no left argument) were ignored. See empty rules in grammar");
	}
MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
if(FSClose(refnum) != noErr) {
	MyPtoCstr(MAXNAME,fn,filename);
	sprintf(Message,"Unknown error closing '%s' table file...",filename);
	Alert1(Message);
	return(FAILED);
	}
Dirty[wScrap] = FALSE;
if(result == OK && rulefound && Answer("Check determinism",'Y') == YES) {
	if(CompileCheck() != OK) return(FAILED);
	if(CheckDeterminism(&Gram) == FAILED) {
		BPActivateWindow(SLOW,wScrap);
		Alert1("Check original line numbers in 'Scrap' window");
		}
	}
return(result);
}


ResetGrammarFromTableDialog(DialogPtr grammarfromtableptr)
{
Rect r;
Handle itemhandle;
short itemtype;

GetDialogItem(grammarfromtableptr,fMakeGrammarFromTableLeftArgCol,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,"\pA");
GetDialogItem(grammarfromtableptr,fMakeGrammarFromTableRightArgCol,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,"\pB");
GetDialogItem(grammarfromtableptr,fMakeGrammarFromTableFirstRow,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,"\p2");
GetDialogItem(grammarfromtableptr,fMakeGrammarFromTableLastRow,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,"\p");
GetDialogItem(grammarfromtableptr,fMakeGrammarFromTableLeftPrefix,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,"\pa");
GetDialogItem(grammarfromtableptr,fMakeGrammarFromTableRightPrefix,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,"\pb");

SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableORD);
SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableRND);
SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableLIN);
SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableSUB);
SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableSUB1);
SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTablePOSLONG);

SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableLR);
SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableLRRL);
SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableRL);

SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableLeftIsASCII);
SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableLeftIsText);
SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableLeftIsNum);
SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableRightIsASCII);
SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableRightIsText);
SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableRightIsNum);

SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableAppend);
SwitchOff(grammarfromtableptr,-1,bMakeGrammarFromTableQuotes);

SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableLeftRemoveSpaces);
SwitchOn(grammarfromtableptr,-1,bMakeGrammarFromTableRightRemoveSpaces);
return(OK);
}


GetArgumentInTableLine(int arg_nr,char **p_line,int i0,char pref,char format,char appendtoalphabet,
	char removespaces)
{
int i,imax,k;
char c,line[MAXLIN];

i = i0;
c = (*p_line)[i];
if(c == '\0' || c == '\t') return(FAILED);

READ:
imax = i0 + MAXLIN - 5;
if(format == 2) { /* numeric */
	line[0] = pref;
	i++;
	k = 1;
	}
else k = 0;
while(TRUE) {
	c = (*p_line)[i-k];
	if(i >= imax) {
		Print(wGrammar," << too long line >> ");
		PrintBehind(wScrap," << too long line >> ");
		break;
		}
	if(c == '\0' || c == '\t') break;
	if(c == ' ' && format > 0) break;
	if(c == '+' && format == 2) break;
	if(format == 0) {	/* ASCII machine code */
		if(c == '?') {
			if(isdigit((*p_line)[i-k+1])) {	/* wild card */
				sprintf(line," %c%c ",c,(*p_line)[i-k+1]);
				PrintBehind(wGrammar,line);
				PrintBehind(wScrap,line);
				i += 2;
				continue;
				}
			}
		if(c == '-') {
				sprintf(line," %c",c);
				PrintBehind(wGrammar,line);
				PrintBehind(wScrap,line);
				i++;
				continue;
			}
		if(c != ' ' || !removespaces) {
			sprintf(line,"%c%ld",pref,(long)ByteToInt(c));
			WriteTerminal(line,appendtoalphabet);
			}
		i++;
		continue;
		}
	line[i-i0] = c;
	i++;
	}
line[i-i0] = '\0';
if(format > 0) {
	WriteTerminal(line,appendtoalphabet);
	if(c == ' ' || c == '+') {
		i++;
		i -= k;
		i0 = i;
		goto READ;
		}
	}
// Print(wGrammar,line);
return(OK);
}


WriteTerminal(char *line,char appendtoalphabet)
{
int i,imax;

imax = strlen(line);
if(imax == 0) return(FAILED);
if(!appendtoalphabet) {
	for(i=imax-1; i >= 0; i--) line[i+1] = line[i];
	line[0] = '\'';
	line[imax+1] = '\'';
	line[imax+2] = '\0';
	}
strcat(line," ");
PrintBehind(wGrammar,line);
PrintBehind(wScrap,line);
if(appendtoalphabet && OkBolChar(line[0])) PrintBehind(wAlphabet,line);
return(OK);
}


TransliterateFile(void)
{
char **p_line,**p_completeline,line[MAXFIELDCONTENT],filename[MAXNAME],
	defaultvalue[MAXFIELDCONTENT],value[MAXFIELDCONTENT];
int result,i,j,imax,iwork,err,irecord,irecordmin,rr,nfields,maxfields,outfields,saveonly,willbedone;
long pos,p,q,count;
Rect r;
Handle itemhandle;
short item,itemtype,refnum,refnumout;
FSSpec spec;
DialogPtr filepreviewptr;
Str255 fn;
OSErr io;
FieldProcess **p_fieldlist;
NSWReply reply;

io = NSWInitReply(&reply);
p_fieldlist = NULL;
p_line = p_completeline = NULL;
Interrupted = FALSE;
err = irecord = 0;
irecordmin = 1;

ShowMessage(TRUE,wMessage,"Open text file or table in tabulated text format...");

result = OldFile(-1,1,fn,&spec);
if(result == OK) {
	p2cstrcpy(filename,fn);
	if((io=MyOpen(&spec,fsRdPerm,&refnum)) != noErr) {
		sprintf(Message,"Error opening %s",filename);
		ShowMessage(TRUE,wMessage,Message);
		TellError(100,io);
		ClearMessage();
		return(FAILED);
		}
	}
else return(FAILED);
ClearMessage();

if((p_fieldlist = (FieldProcess**) GiveSpace((Size) MAXDATABASEFIELDS * sizeof(FieldProcess))) == NULL) return(ABORT);

pos = ZERO;
maxfields = 0;

TRYREAD:

if(ReadOne(FALSE,FALSE,FALSE,refnum,FALSE,&p_line,&p_completeline,&pos) == FAILED) {
	Alert1("This file is empty or unreadable");
	goto OVER;
	}
imax = MyHandleLen(p_completeline);
if(imax == 0) goto TRYREAD;

nfields = 1;
for(i=0; i < imax; i++) {
	if((*p_completeline)[i] == '\t') nfields++;
	}
if(nfields > maxfields) maxfields = nfields;
BPActivateWindow(SLOW,wNotice);
PreviewLine(p_completeline,FALSE);

filepreviewptr = GetNewDialog(SelectFieldID,0L,0L);
ShowWindow(GetDialogWindow(filepreviewptr));

GetDialogItem(filepreviewptr,fSelectFieldNumber,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,"\p1");
GetDialogItem(filepreviewptr,fSubGramStart,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,"\p1");
GetDialogItem(filepreviewptr,fSubGramEnd,&itemtype,&itemhandle,&r);
sprintf(Message,"%ld",(long)Gram.number_gram);
SetDialogItemText(itemhandle,in_place_c2pstr(Message));
GetDialogItem(filepreviewptr,fSaveToFieldNumber,&itemtype,&itemhandle,&r);
SetDialogItemText(itemhandle,"\p1");

saveonly = FALSE;

TRYENTER:

for(iwork=0; iwork < MAXDATABASEFIELDS; iwork++)
	(*p_fieldlist)[iwork].field = (*p_fieldlist)[iwork].subgramstart = (*p_fieldlist)[iwork].subgramend
		= (*p_fieldlist)[iwork].savefield = -1;

result = OK;
TextUpdate(wNotice);
ShowSelect(CENTRE,wNotice);
TextUpdate(wHelp);
ShowSelect(CENTRE,wHelp);
SelectWindow(GetDialogWindow(filepreviewptr));
BPUpdateDialog(filepreviewptr);
while(TRUE) {
	MaintainCursor();
	ModalDialog((ModalFilterUPP) 0L,&item);
	switch(item) {
		case bSelectFieldProceed:
			result = OK;
			goto DONENTER;
			break;
		case bSelectFieldCancel:
			result = ABORT;
			goto DONENTER;
			break;
		case bSelectFieldShowNextRecord:
			if(ReadOne(FALSE,FALSE,FALSE,refnum,FALSE,&p_line,&p_completeline,&pos) != FAILED) {
				PreviewLine(p_completeline,FALSE);
				imax = MyHandleLen(p_completeline);
				nfields = 1;
				for(i=0; i < imax; i++) {
					if((*p_completeline)[i] == '\t') nfields++;
					}
				if(nfields > maxfields) maxfields = nfields;
				goto TRYENTER;
				}
			break;
		case bSelectFieldHelp:
			DisplayHelp("Transliterate text file...");
			goto TRYENTER;
		}
	}

DONENTER:

if(result == ABORT) goto CLEARDIALOG;

outfields = 0;

for(iwork=0; iwork < 13; iwork++) {
	GetField(filepreviewptr,FALSE,-1,fSelectFieldNumber + (4 * iwork),line,&p,&q);
	Strip(line);
	if(line[0] != '\0') {
		i = p / q;
		if(i < 1 || i > maxfields) {
			SelectField(filepreviewptr,-1,fSelectFieldNumber + (4 * iwork),TRUE);
			sprintf(Message,"Field range seems to be 1 to %ld. Try to display next record, it might increase...",
				(long)maxfields);
			Alert1(Message);
			goto TRYENTER;
			}
/*		for(j=0; j < 13; j++) {
			if(i == (*p_fieldlist)[j].field && j != iwork) {
				SelectField(filepreviewptr,-1,fSelectFieldNumber + (4 * iwork),TRUE);
				sprintf(Message,"Field %ld is already used",(long)i);
				Alert1(Message);
				goto TRYENTER;
				}
			} */
		(*p_fieldlist)[iwork].field = i;
		}
		
	GetField(filepreviewptr,FALSE,-1,fSubGramStart + (4 * iwork),line,&p,&q);
	Strip(line);
	if(line[0] != '\0') {
		i = p / q;
		if(i < 1 || i > Gram.number_gram) {
			SelectField(filepreviewptr,-1,fSubGramStart + (4 * iwork),TRUE);
			sprintf(Message,"Subgrammar range is 1 to %ld",(long)Gram.number_gram);
			Alert1(Message);
			goto TRYENTER;
			}
		if((*p_fieldlist)[iwork].field == -1) {
			SelectField(filepreviewptr,-1,fSelectFieldNumber + (4 * iwork),TRUE);
			Alert1("Field hasn't been specified");
			goto TRYENTER;
			}
		(*p_fieldlist)[iwork].subgramstart = i;
		}
		
	GetField(filepreviewptr,FALSE,-1,fSubGramEnd + (4 * iwork),line,&p,&q);
	Strip(line);
	if(line[0] != '\0') {
		i = p / q;
		if(i < 1 || i > Gram.number_gram) {
			SelectField(filepreviewptr,-1,fSubGramEnd + (4 * iwork),TRUE);
			sprintf(Message,"Subgrammar range is 1 to %ld",(long)Gram.number_gram);
			Alert1(Message);
			goto TRYENTER;
			}
		if((*p_fieldlist)[iwork].subgramstart == -1) {
			SelectField(filepreviewptr,-1,fSubGramStart + (4 * iwork),TRUE);
			Alert1("First subgrammar hasn't been specified");
			goto TRYENTER;
			}
		if((*p_fieldlist)[iwork].subgramstart > i) {
			SelectField(filepreviewptr,-1,fSubGramEnd + (4 * iwork),TRUE);
			Alert1("Index of last subgrammar must be greater than that of first subgrammar");
			goto TRYENTER;
			}
		(*p_fieldlist)[iwork].subgramend = i;
		}
		
	GetField(filepreviewptr,FALSE,-1,fSaveToFieldNumber + iwork,line,&p,&q);
	Strip(line);
	if(line[0] != '\0') {
		i = p / q;
		if(i < 1) {
			SelectField(filepreviewptr,-1,fSaveToFieldNumber + iwork,TRUE);
			sprintf(Message,"Field number must be positive");
			Alert1(Message);
			goto TRYENTER;
			}
		for(j=0; j < 13; j++) {
			if(i == (*p_fieldlist)[j].savefield && j != iwork) {
				SelectField(filepreviewptr,-1,fSaveToFieldNumber + iwork,TRUE);
				sprintf(Message,"Field %ld is already used",(long)i);
				Alert1(Message);
				goto TRYENTER;
				}
			}
		(*p_fieldlist)[iwork].savefield = i;
		if(i > outfields) outfields = i;
		}
	else {
		if((*p_fieldlist)[iwork].field > 0 && (*p_fieldlist)[iwork].subgramstart > 0) {
			sprintf(Message,"Which field will receive modified field #%ld?",(long)(*p_fieldlist)[iwork].field);
			Alert1(Message);
			SelectField(filepreviewptr,-1,fSaveToFieldNumber + iwork,TRUE);
			goto TRYENTER;
			}
		}
	}

if(saveonly) goto CLEARDIALOG;

for(i=1; i <= outfields; i++) {
	willbedone = FALSE;
	for(iwork=0; iwork < MAXDATABASEFIELDS; iwork++) {
		if((*p_fieldlist)[iwork].savefield == i) goto NEXTFIELD;
//		if((*p_fieldlist)[iwork].field == i && (*p_fieldlist)[iwork].subgramstart > 0) willbedone = TRUE;
		}
	if(!willbedone) {
		for(iwork=0; iwork < MAXDATABASEFIELDS; iwork++)
			if((*p_fieldlist)[iwork].savefield == -1) goto MAPFIELD;
		sprintf(Message,"Number of saved fields will exceed %ld. Save only modified fields",
			(long) MAXDATABASEFIELDS);
		result = Answer(Message,'Y');
		if(result == ABORT) goto CLEARDIALOG;
		saveonly = result;
		if(saveonly) goto TRYENTER;
		saveonly
			= Answer("No space to save more fields. Save only modified fields (otherwise task will abort)",
				'Y');
		if(saveonly) goto TRYENTER;
		result = ABORT;
		goto CLEARDIALOG;
		
MAPFIELD:
		(*p_fieldlist)[iwork].field = i;
		(*p_fieldlist)[iwork].savefield = i;
		sprintf(Message,"Field #%ld will be copied (unmodified) to new file", (long)i);
		Println(wTrace,Message);
		}
		
NEXTFIELD:
	continue;
	}
		
CLEARDIALOG:

DisposeDialog(filepreviewptr);
if(result != OK) goto OVER;

PROCESS:

strcpy(defaultvalue,"1");
if((result=AnswerWith("Start from record...",defaultvalue,value)) != OK) goto OVER;
irecordmin = (int) atol(value);
if(irecordmin < 1) {
	Alert1("You must start at least with record '1'");
	}
strcpy(Message,filename);
Message[MAXNAME-5] = '\0';	// FIXME: should replace MAXNAME with strlen(Message) or actually search for a file extension - 011607 akozar
strcat(Message,".new");
c2pstrcpy(fn, Message);
if(!NewFile(-1,1,fn,&reply)) goto OVER;
result = CreateFile(-1,-1,1,fn,&reply,&refnumout);
if(result != OK) goto OVER;

ComputeOn++;

pos = ZERO;
SetFPos(refnum,fsFromStart,ZERO);

while(TRUE) {
	PleaseWait();
	if((rr=ReadOne(FALSE,FALSE,FALSE,refnum,FALSE,&p_line,&p_completeline,&pos)) == FAILED) break;
	imax = MyHandleLen(p_completeline);
	irecord++;
	if(imax == 0) {
		if(rr == STOP) break;
		count = 1L;
		io = FSWrite(refnumout,&count,"\n");
		if(io != noErr)	{
			if(io == dskFulErr) Alert1("No more space on this disk... Task aborted");
			else TellError(101,io);
			result = ABORT;
			goto CLOSE;
			}
		goto READMORE;
		}
	if(irecord < irecordmin) goto READMORE;
	result = TransliterateRecord(p_completeline,p_fieldlist,refnumout,outfields,&err);
	sprintf(Message,"%ld records processed...",(long)irecord);
	FlashInfo(Message);
	if(result != OK) {
		err++; break;
		}

READMORE:
	if(rr == STOP) break;
	}

CLOSE:
GetFPos(refnumout,&count);
SetEOF(refnumout,count);
FlushFile(refnumout);
FSClose(refnumout);
reply.saveCompleted = true;

ComputeOn--;

OVER:

io = NSWCleanupReply(&reply);
MyDisposeHandle((Handle*)&p_line); MyDisposeHandle((Handle*)&p_completeline);
MyDisposeHandle((Handle*)&p_fieldlist);
if(FSClose(refnum) != noErr) {
	MyPtoCstr(MAXNAME,fn,LineBuff);
	sprintf(Message,"Unknown error closing '%s' table file...",LineBuff);
	Alert1(Message);
	return(FAILED);
	}
if(err > 0) {
	sprintf(Message,"%ld records have been processed (NOT all of them)...",(long)irecord);
	Alert1(Message);
	}
else {
	sprintf(Message,"%ld records have been processed...",(long)irecord);
	if(irecord > 0) Alert1(Message);
	}
HideWindow(Window[wInfo]);
return(result);
}


PreviewLine(char **p_line,int makealpha)
{
int i,j,ifield,imax,nfields;
char c,**p_field;
GrafPtr saveport;

GetPort(&saveport);

imax = MyHandleLen(p_line);
if(imax == 0) return(FAILED);

nfields = 1;
for(i=0; i < imax; i++) {
	if((*p_line)[i] == '\t') nfields++;
	}
	
if((p_field = (char**) GiveSpace((Size) MyGetHandleSize((Handle)p_line))) == NULL) return(ABORT);

i = 0;
for(ifield=0; ifield < nfields; ifield++) {
	Reformat(wNotice,-1,-1,(int) bold,&Blue,NO,YES);
	if(makealpha) {
		sprintf(Message,"[%c] ",(char)(ifield + 'A'));
		}
	else sprintf(Message,"[%ld] ",(long)(ifield+1L));
	PrintBehind(wNotice,Message);
	Reformat(wNotice,-1,-1,(int) normal,&Black,NO,YES);
	j = 0;
	while((c=(*p_line)[i]) != '\t' && c != '\0') {
		(*p_field)[j++] = c;
		i++; if(i >= imax) break;
		}
	(*p_field)[j] = '\0';
	i++;
	PrintHandleBehind(wNotice,p_field);
	PrintBehind(wNotice,"\n");
	}
PrintBehindln(wNotice,"------------------------");
ShowSelect(CENTRE,wNotice);
MyDisposeHandle((Handle*)&p_field);
if(saveport != NULL) SetPort(saveport);
else if(Beta) Alert1("Err PreviewLine(). saveport == NULL");
return(OK);
}


TransliterateRecord(char **p_line,FieldProcess **p_fieldlist,short refnum,int outfields,int *p_err)
{
int i,imax,j,k,result,ifield,iwork,infields,orgfield,
	compiledmem,dirtymem;
char c,**p_field;
long count;
OSErr io;

result = OK;
if((p_field = (char**) GiveSpace((Size) MyGetHandleSize((Handle)p_line))) == NULL) return(ABORT);

imax = MyHandleLen(p_line);
if(imax == 0) return(OK);

infields = 1;
for(i=0; i < imax; i++) {
	if((*p_line)[i] == '\t') infields++;
	}

for(ifield=1; ifield <= outfields; ifield++) {
#if BP_CARBON_GUI
	// FIXME ? Should non-Carbon builds call a "poll events" callback here ?
	if((result=MyButton(0)) != FAILED) {
		SetButtons(TRUE);
		Interrupted = TRUE;
		dirtymem = Dirty[wAlphabet]; Dirty[wAlphabet] = FALSE;
		compiledmem = CompiledGr;
		if(result == OK) while((result = MainEvent()) != RESUME && result != STOP && result != EXIT){};
		if(result == EXIT) goto OUT;
		if(Dirty[wAlphabet]) {
			Alert1("Alphabet changed. Must recompile...");
			result = ABORT; goto OUT;
			}
		Dirty[wAlphabet] = dirtymem;
		if((result == STOP) || (compiledmem && (CompileCheck() != OK))) {
			result = ABORT; goto OUT;
			}
		}
#endif /* BP_CARBON_GUI */
	result = OK;
	for(iwork=0; iwork < MAXDATABASEFIELDS; iwork++) {
		if((*p_fieldlist)[iwork].savefield == ifield) goto DOFIELD;
		}
	continue;
	
DOFIELD:
	orgfield = (*p_fieldlist)[iwork].field;
	i = 0;
	for(j=1; j < orgfield; j++) {
		while((c=(*p_line)[i]) != '\t' && c != '\0') {
			if(i >= imax) break;
			i++;
			}
		if(c == '\0' || i >= imax) break;
		i++;
		}
	k = 0;
	while((c=(*p_line)[i]) != '\t' && c != '\0') {
		(*p_field)[k++] = c;
		i++;
		if(i >= imax) break;
		}
	(*p_field)[k] = '\0';
	if((*p_fieldlist)[iwork].subgramstart > 0) {
		result
			= ComputeField(p_field,(*p_fieldlist)[iwork].subgramstart,(*p_fieldlist)[iwork].subgramend);
		if(result == ABORT) goto OUT;
		if(result == FAILED) (*p_err)++;
		}
	count = (long) MyHandleLen(p_field);
	MyLock(FALSE,(Handle)p_field);
	io = FSWrite(refnum,&count,*p_field);
	MyUnlock((Handle)p_field);
	if(io == noErr) {
		count = 1L;
		if(ifield < outfields) io = FSWrite(refnum,&count,"\t");
		}
	if(io != noErr)	{
		if(io == dskFulErr) Alert1("No more space on this disk... Task aborted");
		else TellError(101,io);
		result = ABORT;
		goto OUT;
		}
	}
count = 1L;
io = FSWrite(refnum,&count,"\n");

OUT:
MyDisposeHandle((Handle*)&p_field);
return(result);
}


ComputeField(char **p_field,int subgramstart,int subgramend)
{
int grtype,irul,i,imax,j,k,result,repeat,start;
long pos,posmax,length;
t_subgram subgram;
t_rule rule;
tokenbyte **p_a,m,p;
char c,apref,line[MAXLIN],**p_x;

imax = MyHandleLen(p_field);
result = OK;
p_x = NULL;
p_a = NULL;

// Find prefix of first terminal used in the left argument of first valid rule in subgramstart
subgram = (*(Gram.p_subgram))[subgramstart];
grtype = subgram.type;
irul = 0;

TRYRULE:
do {
	irul++;
	if(irul > subgram.number_rule) {
		sprintf(Message,"\nNo valid rule in first subgrammar (%ld). Can't use this grammar...",
			(long)subgramstart);
		Println(wTrace,Message);
		return(ABORT);
		}
	rule = (*(subgram.p_rule))[irul];
	}
while(rule.operator == 2); /*  <--  */
p_a = rule.p_leftarg;
for(pos=ZERO; ((*p_a)[pos] != TEND || (*p_a)[pos+1] != TEND); pos += 2L) {
	m = (*p_a)[pos]; p = (*p_a)[pos+1];
	if(m == T3 && p < Jbol) goto FOUND;
	}
goto TRYRULE;

FOUND:
// We got terminal (m,p). Now we look for its prefix.
apref = (*((*p_Bol)[p]))[0];

// Tokenizing the field
if((p_a = (tokenbyte**) GiveSpace((Size) 2L * ((2L*imax)+2L) * sizeof(tokenbyte))) == NULL) return(ABORT);
/* We keep enough space to insert '-' */
posmax = MyGetHandleSize((Handle)p_a) / sizeof(tokenbyte) - 6L;

pos = ZERO;
if((*p_field)[0] != ' ') {
	(*p_a)[pos++] = T3;
	(*p_a)[pos++] = 1;	/* every line will start with '-' */
	}
	
for(i=0; i < imax; i++) {
	if(pos >= posmax) {
		posmax = (((posmax + 6L) * 3L) / 2L) - 6L;
		if((p_a=(tokenbyte**) IncreaseSpace((Handle)p_a)) == NULL) return(ABORT);
		}
	c = (*p_field)[i];
	switch(c) {
		case ' ':	/* Space */
			(*p_a)[pos++] = T3; (*p_a)[pos++] = 1;	/* replace space with '-' */
			continue;
			break;
		case '\n':	/* Line-feed */
			/* (FIXME? There was an embedded vertical tab character btw '' above.
		       Was \v or \n intended?  -- akozar 20200731 */
			(*p_a)[pos++] = T3; (*p_a)[pos++] = 1;	/* every line will end with '-' */
			break;
		}
	k = ByteToInt(c);
	sprintf(line,"%c%ld",apref,(long)k);
	MystrcpyStringToHandle(&p_x,line);
	if((j=CreateBol(TRUE,FALSE,p_x,FALSE,TRUE,BOL)) < 0) {
		result = ABORT; goto OUT;
		}
	(*p_a)[pos++] = T3; (*p_a)[pos++] = j;
	}
if((*p_a)[pos-2] != T3 || (*p_a)[pos-1] != 1) {
	(*p_a)[pos++] = T3; (*p_a)[pos++] = 1;	/* every line will end with '-' */
	}
(*p_a)[pos++] = TEND;
(*p_a)[pos] = TEND;

// Now transliterating...
repeat = FALSE;
length = LengthOf(&p_a);
result = Compute(&p_a,subgramstart,subgramend,&length,&repeat);
if(result != OK) goto OUT;

start = TRUE;

for(i=0,pos=ZERO; ((*p_a)[pos] != TEND || (*p_a)[pos+1] != TEND); pos += 2L) {
	m = (*p_a)[pos]; p = (*p_a)[pos+1];
	if(m == T3 && p == 1) {		/* '-' */
		if(start) continue;
		else {
			(*p_field)[i] = ' ';
			goto MORE;
			}
		}
	start = FALSE;
	if(p < Jbol) sprintf(line,"%s",(*((*p_Bol)[p])));
	else {
		if(Beta) Alert1("Err. ComputeField(). Either p >= Jbol");
		goto ERR;
		}
	k = 1;
	j = GetInteger(FALSE,line,&k);
	if(j == INT_MAX) {
		if(Beta) Alert1("Err. ComputeField(). GetInteger() returned INT_MAX");
		goto ERR;
		}
	(*p_field)[i] = j;

MORE:
	i++;
	if(i >= imax) {
		imax = (imax * 3L) / 2L;
		if((p_field=(char**) IncreaseSpace((Handle)p_field)) == NULL) {
			result = ABORT; goto OUT;
			}
		}
	}
(*p_field)[i] = '\0';
while((*p_field)[--i] == ' ') (*p_field)[i] = '\0';

OUT:
MyDisposeHandle((Handle*)&p_a);
MyDisposeHandle((Handle*)&p_x);
return(result);

ERR:
if(Beta) result = ABORT;
goto OUT;
}