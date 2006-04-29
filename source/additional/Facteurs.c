/* Facteurs.c */ 

#ifndef _H_BP2
#include "-BP2.h"
#endif
#include "-BPdecl.h"
#include "Facteurs.h"

#include "Facteurs.proto.h"

/* -----  Prototypes ---- */
p_transition **SetTransition(p_state**,int,p_state**,int);
p_state **NewState(int);
p_state **NextState(p_state**,int);
int AutomatonToGrammar(p_state**,int,int,p_state**,p_state**,p_state**);
int WriteTransitions(p_state**,int**);
p_state **Suffix(p_state**,int);
int CopyState(p_state**,p_state***,int);
/* ---------- */

int MaxStateLabel=10,CurrStateLabel,
	MaxTransitionLabel=10,CurrTransitionLabel,**h_StateCopies;
char ****h_StateLabel,****h_TransitionLabel;

p_state **Init,**Art;


AutomatonToGrammar(p_state **q, int m, int n, p_state **r, p_state **r1, p_state **last)
{
int i,**h_label_list;
if((h_label_list =
	(int**) GiveSpace((1+CurrStateLabel) * sizeof(int))) == NULL)
								return(FAILED);
for(i=0; i <= CurrStateLabel; i++) (*h_label_list)[i] = 0;
sprintf(Message,"m = %d  n = %d  r = %s  r1 = %s  last = %s\r",m,n,
	*((*h_StateLabel)[(**r).label]),*((*h_StateLabel)[(**r1).label]),
	*((*h_StateLabel)[(**last).label]));
Print(wGrammar,Message);
return(WriteTransitions(q,h_label_list));
}


WriteTransitions(p_state **q, int **h_label_list)
{
int label1,label2,label3,label4;
p_transition **ptr;
p_state **q1;

if(q == NULL) {
	Alert1("Err.1  WriteTransitions()  Call Bel!");
	return(FAILED);
	}
label1 = (**q).label;
if((**q).S != NULL) {
	label4 = (**((**q).S)).label;
	}
else {
	Alert1("Err.2  WriteTransitions()  Call Bel!");
	return(FAILED);
	}
if((*h_label_list)[label1] > 0) return(OK);	/* State already seen */
else (*h_label_list)[label1] = 1;	/* Mark state */
ptr = (**q).out;
if(ptr == NULL) {
	sprintf(Message,"[S(%s) = %s]\r",*((*h_StateLabel)[label1]),
		*((*h_StateLabel)[label4]));
	Print(wGrammar,Message);
	}
while(ptr != NULL) {
	label2 = (**ptr).label;
	if((q1 = (**ptr).out) == NULL)  {
		Alert1("Err.3  WriteTransitions()  Call Bel!");
		return(FAILED);
		}
	label3 = (**q1).label;
	DrawRule(label1,label2,label3,(**ptr).offset,label4);
	if(WriteTransitions(q1,h_label_list) != OK) return(FAILED);
	ptr = (**ptr).next;
	}
return(OK);
}


DrawRule(int label1, int label2, int label3, int offset, int label4)
{
int w = wGrammar;
sprintf(Message,"[%d] %s --> %s %s  [S(%s) = %s]\r",offset,
	*((*h_StateLabel)[label1]),
	*((*h_TransitionLabel)[label2]),*((*h_StateLabel)[label3]),
	*((*h_StateLabel)[label1]),
	*((*h_StateLabel)[label4]));
Print(wGrammar,Message);
return(OK);
}


InitAutomaton(void)
{
if((h_StateLabel =
	(char****) GiveSpace(MaxStateLabel * sizeof(char**))) == NULL)
								return(FAILED);
if((h_StateCopies =
	(int**) GiveSpace(MaxStateLabel * sizeof(int))) == NULL)
								return(FAILED);
	CurrStateLabel = -1;
if((h_TransitionLabel =
	(char****) GiveSpace(MaxTransitionLabel * sizeof(char**))) == NULL)
								return(FAILED);
	CurrTransitionLabel = -1;
Init = Art = NULL;
return(OK);
}


int NewStateLabel(char s[])
{
int i;
char **ptr;
if(CurrStateLabel >= 0) {
	for(i=0; i <= CurrStateLabel; i++) {
		if(strcmp(s,*((*h_StateLabel)[i])) == 0) {
			sprintf(Message,"Duplicated states with same label <%s>",s);
			Alert1(Message);
			return(i);
			}
		}
	}
CurrStateLabel++;
if(CurrStateLabel >= MaxStateLabel) {
	if((h_StateLabel = (char****) IncreaseSpace((Handle)
		h_StateLabel)) == NULL) return(ABORT);
	if((h_StateCopies = (int**) IncreaseSpace((Handle)
		h_StateCopies)) == NULL) return(ABORT);
	MaxStateLabel = (MaxStateLabel * 3) / 2;
	}
if((ptr =
	(char**) GiveSpace((1 + strlen(s)) * sizeof(char))) == NULL)
								return(FAILED);
(*h_StateLabel)[CurrStateLabel] = ptr;
strcpy(*((*h_StateLabel)[CurrStateLabel]),s);
(*h_StateCopies)[CurrStateLabel] = 0;
return(CurrStateLabel);
}


int TransitionLabel(char s[])
{
int i;
char **ptr;
if(CurrTransitionLabel >= 0) {
	for(i=0; i <= CurrTransitionLabel; i++) {
		if(strcmp(s,*((*h_TransitionLabel)[i])) == 0) return(i);
		}
	}
CurrTransitionLabel++;
if(CurrTransitionLabel >= MaxTransitionLabel) {
	if((h_TransitionLabel = (char****) IncreaseSpace((Handle)
		h_TransitionLabel)) == NULL) return(ABORT);
	MaxTransitionLabel = (MaxTransitionLabel * 3) / 2;
	}
if((ptr =
	(char**) GiveSpace((1+strlen(s)) * sizeof(char))) == NULL)
								return(FAILED);
(*h_TransitionLabel)[CurrTransitionLabel] = ptr;
strcpy(*((*h_TransitionLabel)[CurrTransitionLabel]),s);
return(CurrTransitionLabel);
}



KillTransition(p_state **q, int label)
{
p_transition **ptr1,**ptr2,**ptr3;
ptr1 = (**q).out;
if(ptr1 == NULL) return(FAILED);
if((**ptr1).label == label) {
	ptr2 = (**ptr1).next;
	MyEmptyHandle(&ptr1);
	(**q).out = ptr2;			/* Attach next transition or NULL */
	return(OK);
	}
while(ptr1 != NULL) {
	ptr2 = (**ptr1).next;
	if((**ptr2).label == label) {
		ptr3 = (**ptr2).next;
		MyEmptyHandle(&ptr2);
		(**ptr1).next = ptr3;	/* Attach next transition or NULL */
		return(OK);
		}
	else ptr1 = ptr2;
	}
return(FAILED);
}


p_state **NewState(int label)
{
p_state **ptr;
if((ptr =
	(p_state**) GiveSpace((Size)sizeof(p_state))) == NULL)
								return(NULL);
(**ptr).out = NULL;
(**ptr).label = label;
(**ptr).S = ptr;
return(ptr);
}


p_transition **SetTransition(p_state **p_state1, int label, p_state **p_state2, int offset)
{
p_transition **p,**ptr,**oldptr;
for(p=oldptr=(**p_state1).out; p != NULL;) {
	if((**p).label == label) {
		(**p).offset = offset;	/* Transition already existsÉ */
						/* É delete it to maintain determinism */
		(**p).out = p_state2;
		return(p);
		}
	oldptr = p;
	p = (**p).next;
	}
if((ptr =
	(p_transition**) GiveSpace((Size)sizeof(p_transition))) == NULL)
								return(NULL);
(**ptr).out = p_state2;
(**ptr).next = NULL;
(**ptr).label = label;
(**ptr).offset = offset;
if(oldptr == NULL) (**p_state1).out = ptr;
else (**oldptr).next = ptr;
return(ptr);
}


p_state **NextState(p_state **state, int label)
{
p_transition **ptr;
if(state == NULL) {
	Alert1("Err NextState().  Call Bel!");
	return(NULL);
	}
ptr = (**state).out;
if(ptr == NULL) return(NULL);
while(ptr != NULL) {
	if((**ptr).label == label) return((**ptr).out);
	ptr = (**ptr).next;
	}
return(NULL);
}


SelectionToString(int w, char ***pp_s, int *p_end)
{
char *p1,*p2,**p_buff,**h,**ptr;
int i,origin,end;

*p_end = 0;
if(!Editable[w]) return(FAILED);
PleaseWait();
SelectOn = 1;
origin = (int) (**(TEH[w])).selStart;
end = (int) (**(TEH[w])).selEnd;
*p_end = end;
if(origin >= end) {
	Alert1("No string selected");
	SelectOn = 0;
	return(FAILED);
	}
if((ptr = (char**) GiveSpace((Size)(end-origin+1) * sizeof(char)))
						== NULL) goto BAD;
(*pp_s) = ptr;
h = (char**) (**(TEH[w])).hText;
for(i=origin; i < end; i++) {
	(**pp_s)[i-origin] = (*h)[i];
	}
(**pp_s)[i-origin] = '\0';
*p_end = i - origin;
SelectOn = 0;
return(OK);

BAD:
Alert1("Can't create string.  Err in SelectionToString()");
SelectOn = 0;
return(FAILED);
}


CopyState(p_state **q1, p_state ***p_q2, int label)
{
p_transition **ptr1,**ptr2;
if(q1 == NULL) {
	Alert1("Err. CopyState(). Call Bel!");
	}
*p_q2 = NewState(label);
(***p_q2).l = (**q1).l;
(***p_q2).p = (**q1).p;
ptr1 = (**q1).out;
/* if(ptr1 != NULL) {
	SetTransition(*p_q2,(**ptr1).label,(**ptr1).out,(**ptr1).offset);
	}
else return(OK);
ptr1 = (**ptr1).next; */
while(ptr1 != NULL) {
	ptr2 = SetTransition(*p_q2,(**ptr1).label,(**ptr1).out,(**ptr1).offset);
	(**ptr2).offset = (**ptr1).offset;
	ptr1 = (**ptr1).next;
	}
return(OK);
}


BuildAutomaton(void)
{
p_state **state,**last,**q,**q1,**r,**r1,**rc,**rd;
int i,j,m,n,nn,nmax,c,d;
char **p_x,s[2];
MIDI_Parameters parms;

if(InitAutomaton() != OK) return(FAILED);
if(SelectionToString(wGrammar,&p_x,&nmax) != OK) return(FAILED);
TESetSelect((**(TEH[wGrammar])).teLength,(**(TEH[wGrammar])).teLength,
	TEH[wGrammar]);
Print(wGrammar,"\r\r");
i = 1; j = 0;	/* i and j are state indexes */
Art = NewState(NewStateLabel("Art"));
(**Art).l = (**Art).p = -1;
Init = NewState(NewStateLabel("Q0"));
(**Init).l = (**Init).p = 0;
last = Init;
(**Init).S = Art;
m = n = 0; r = Art; r1 = Init; s[1] = '\0';
parms.clockTime = 0;
DriverControl(CLOCKTIME_CODE,&parms);

while(n < nmax) {
	s[0] = (*p_x)[n]; c = TransitionLabel(s);
	sprintf(Message,"Reading '%c'É\r",(*p_x)[n]);
	Print(wGrammar,Message);
	SetTransition(Art,c,Init,0);
	sprintf(Message,"Q%d",i++);
	q = NewState(NewStateLabel(Message));
	(**q).l = (**last).l + 1;
	(**q).p = (**last).p + 1;
	SetTransition(last,c,q,0);
	if((state = NextState((**last).S,c)) != NULL) {
		(**q).S = state;
		}
	else {
		while(m < n) {
			s[0] = (*p_x)[m]; d = TransitionLabel(s);
			m++;
			r1 = NextState(r1,d);
			if(r1 == NULL) {
				Alert1("Err. BuildAutomaton()  Call Bel!");
				return(FAILED);
				}
			/* Create state q1 with É as r.d */
			rd = NextState(r,d);
			sprintf(Message,"%s.%d",*((*h_StateLabel)[(**rd).label]),
			++((*h_StateCopies)[(**rd).label]));
			j++;
			CopyState(rd,&q1,NewStateLabel(Message));
			(**q1).l = (**r).l + 1;
			(**rd).S = q1;
			(**r1).S = q1;
			SetTransition(r,d,q1,(**q1).p - (**r).p - 1);
			(**q1).S = NextState(Suffix(r,d),d);
			r = q1;
			}
		r = Suffix(last,c);
		(**q).S = NextState(r,c);
		}
	rc = NextState(r,c);
	if((r1 == last) && ((**rc).l == ((**r).l + 1))) {
		m++;
		r = rc;
		r1 = NextState(r1,c);
		}
	last = q;
	n++;
	Print(wGrammar,"C(");
	for(nn=0; nn < n; nn++) {
		sprintf(Message,"%c",(*p_x)[nn]);
		Print(wGrammar,Message);
		}
	Print(wGrammar,"):\r");
	AutomatonToGrammar(Init,m,n,r,r1,last);
	Print(wGrammar,"------------------------------\r");
	ShowSelect(wGrammar);
/*	while(!Button()); */
	}
DriverStatus(CLOCKTIME_CODE,&parms);
Tcurr = parms.clockTime;
Print(wGrammar,"FINAL TRANSDUCER:\r");
sprintf(Message,
	"%d states  (length of string = %d)  Computation time %ldms\r",i+j,
	nmax,10L * Tcurr);
Print(wGrammar,Message);
AutomatonToGrammar(Init,m,n,r,r1,last);
ShowSelect(wGrammar);
MyEmptyHandle(&p_x);
return(OK);
}


p_state **Suffix(p_state **q, int c)
{
p_state **sqc,**qc;
sqc = NextState((**q).S,c);
qc = NextState(q,c);
if(sqc == NULL || ((**sqc).l >= (**qc).l)) {
	SetTransition((**q).S,c,qc,(**qc).p - (**((**q).S)).p - 1);
	return(Suffix((**q).S,c));
	}
else {
	return((**q).S);
	}
}