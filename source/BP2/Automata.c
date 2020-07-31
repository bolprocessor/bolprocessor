/* Automata.c (BP2 version CVS) */

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

// PROCEDURES FOR BUILDING TREE AUTOMATA RECOGNIZING TOKENS

UpdateAutomata(void)
{
int j,s,jmax;

ShowMessage(TRUE,wMessage,"Building tree automata for 'smart cursor'");
for(j=0; j < MaxProc; j++) AddWordToTrees((*p_GramProcedure)[j],PROC);
for(j=0; j < MaxPerformanceControl; j++)
	AddWordToTrees((*p_PerformanceControl)[j],CTRL);
jmax = MyGetHandleSize((Handle)p_Bol) / sizeof(char**);
for(j=2; j < jmax; j++) AddWordToTrees((*p_Bol)[j],TERM);
if(p_VarStatus == NULL) jmax = 0;
else jmax = MyGetHandleSize((Handle)p_VarStatus) / sizeof(int);
for(j=1; j < jmax; j++) {
	s = (*p_VarStatus)[j];
	// Take only variables in grammar, not glossary
	// (too long otherwise)
	if((s & 1) || (s & 2)) AddWordToTrees((*p_Var)[j],VAR);
	}
for(j=0; j < Jpatt; j++) AddWordToTrees((*p_Patt)[j],PAT);
HideWindow(Window[wMessage]);
return(OK);
}


AddWordToTrees(char** p_word,int type)
{
AddWordToTree(p_word,1,type);
AddWordToTree(p_word,-1,type);
return(OK);
}


AddWordToTree(char** p_word,int dir,int type)
// dir = 1 : prefix tree
// suffix tree otherwise
{
int i,i0,length;
node *p_n,**h_n;
char c;
arc **h_a,**newh_a;

PleaseWait();
length = MyHandleLen(p_word);
if(dir == 1) {
	p_n = &PrefixTree;
	i0 = 0;
	}
else {
	p_n = &SuffixTree;
	i0 = length - 1;
	dir = -1;
	}
h_n = &p_n;
for(i=i0; i >= 0 && i < length; i += dir) {
	c = (*p_word)[i];
	if((**h_n).p == NULL) {
		h_a = (arc**) GiveSpace(sizeof(arc));
		(**h_n).p = h_a;
		goto NEWARC;
		}
	h_a = newh_a = (**h_n).p;
	do {
		h_a = newh_a;
		if((**h_a).x == c) {
			h_n = (**h_a).p;
			goto NEXTCHAR;
			}
		newh_a = (**h_a).next;
		}
	while(newh_a != NULL);
	newh_a = (arc**) GiveSpace(sizeof(arc));
	(**h_a).next = newh_a;
	h_a = newh_a;
NEWARC:
	(**h_a).x = c;
	(**h_a).next = NULL;
	h_n = (node**) GiveSpace(sizeof(node));
	(**h_a).p = h_n;
	(**h_n).accept = FALSE;
	(**h_n).p = NULL;
NEXTCHAR: ;
	}
(**h_n).accept = type;
return(OK);
}