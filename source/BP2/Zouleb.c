/* Zouleb.c (BP2 version CVS) */

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


Zouleb(tokenbyte ***pp_a,int *p_level,unsigned long *p_pos_init,int retro,
	int rndseq,int rotate,int repeat,int isbracket,int orgseed)
{
unsigned long i,imax,ib,ibmax,iorg,j,newpos,origin,end;
long ichunk,nchunks,ichunkmax,**p_index,x;
tokenbyte m,p,**p_b;
int r,orglevel,newlevel,more,store,newrotate,maxparam,seed,ischanged;
unsigned int currentseed;
ChunkPointer **p_chunk;
char line[MAXLIN];

PleaseWait();

if(pp_a == NULL) {
	if(Beta) Alert1("Err. Zouleb(). pp_a == NULL");
	return(OK);
	}

if(p_NumberConstant == NULL) maxparam = 0;
else maxparam = (MyGetHandleSize((Handle)p_NumberConstant) / sizeof(double));

imax = MyGetHandleSize((Handle)(*pp_a)) / sizeof(tokenbyte) - 6L;

ichunkmax = FIELDSIZE;
p_chunk =  (ChunkPointer**) GiveSpace((Size)ichunkmax * sizeof(ChunkPointer));
if(p_chunk == NULL) return(ABORT);
				
ibmax = FIELDSIZE;
p_b =  (tokenbyte**) GiveSpace((ibmax + 6L) * (Size)sizeof(tokenbyte));
if(p_b == NULL) return(ABORT);


orglevel = (*p_level);
seed = orgseed;
r = OK;

// First we read the sequence and store its chunks

ichunk = ZERO;
more = 0;
i = iorg = (*p_pos_init);

if(rotate != 0) rotate = FindValue(T39,rotate,0);

while(TRUE) {
	
	m = (**pp_a)[i]; p = (**pp_a)[i+1];
	
	if(m == TEND && p == TEND) {
		break;
		}
	if(m == T0 && (p == 13 || p == 14 || p == 23)) {	/*  ',' or  '}' */
		break;
		}
	switch(m) {
		case T3:	/* terminal */
		case T4:	/* variable */
		case T9:	/* time pattern */
		case T25:	/* simple note */
			origin = i;
			do {	/* Include prolongation symbols '_' */
				i += 2L;
				m = (**pp_a)[i]; p = (**pp_a)[i+1];
				}
			while(m == T3 && p == 0);
			goto STOREOBJECT;
			break;
		case T6:	/* wildcard */
		case T7:	/* out-time object or simple note */
			origin = i;
			i += 2L;
STOREOBJECT:
			end = i;
			if((r=StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK)
				goto OUT;
			continue;
			break;
		case T12:
			if(p == 23) {	/* _randomize */
				seed = RANDOMIZE;
				goto NEXT;
				break;
				}
			if(p == 21 || p == 22 || p == 24) {
				i += 2L;
				origin = i;
				newlevel = orglevel;
				}
			else goto NEXT;
			ischanged = TRUE;
			switch(p) {
				case 21:	/* _retro */
					if((r=Zouleb(pp_a,&newlevel,&i,1-retro,rndseq,rotate,repeat,FALSE,seed))
						!= OK) goto OUT;
					if(!retro) ischanged = FALSE;
					break;
				case 22:	/* _rndseq */
					if((r=Zouleb(pp_a,&newlevel,&i,FALSE,TRUE,0,repeat,FALSE,seed))
						!= OK) goto OUT;
					ischanged = FALSE;
					break;
				case 24:	/* _ordseq */
					if((r=Zouleb(pp_a,&newlevel,&i,FALSE,FALSE,0,repeat,FALSE,seed))
						!= OK) goto OUT;
					break;
				}
STORE:
			end = i;
			if((r=StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK)
				goto OUT;
			more--;
			continue;
			break;
		case T39:	/* _rotate */
			if(p == 0) goto NEXT;
			newrotate = FindValue(m,p,0);
			i += 2L;
			origin = i;
			newlevel = orglevel;
			if((r=Zouleb(pp_a,&newlevel,&i,retro,rndseq,rotate+newrotate,
					repeat,FALSE,seed)) != OK)
				goto OUT;
			ischanged = ((rotate+newrotate) == 0);
			goto STORE;
			break;
		case T42:	/* _srand */
			seed = p;
			goto NEXT;
			break;
		}
	
	if(m == T1) {	/* Number */
		origin = i;
		do {
			i += 2L;
			m = (**pp_a)[i];
			}
		while(m == T1);
		if((**pp_a)[i] == T0 && (**pp_a)[i+1] == 11) {	/* '/' */
			do {
				i += 2L;
				m = (**pp_a)[i];
				}
			while(m == T1);
			}
		end = i;
		if((r=StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto OUT;
		continue;
		}
	
	if(m == T0 && (p == 11 || p == 21 || p == 24 || p == 25)) {	/* '/' '*' '**' '\' */
		do {
			i += 2L;
			m = (**pp_a)[i];
			}
		while(m == T1);
		continue;
		}
		
	if(m == T0 && (p == 12 || p == 22)) {	/* '{' */
		origin = i;
		newpos = i + 2L;
		newlevel = orglevel + 1;
		do {
			r = Zouleb(pp_a,&newlevel,&newpos,retro,rndseq,rotate,repeat,TRUE,seed);
			if(r != OK) goto OUT;
			}
		while(newlevel > orglevel);
		end = i = newpos;
		ischanged = TRUE;
		if(retro || rndseq || (rotate != 0)) ischanged = FALSE;
		if((r=StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto OUT;
		continue;
		}
NEXT:
	i += 2L;
	}

nchunks = ichunk;

if(repeat) {
	if(ProduceStackDepth == -1) {
		sprintf(Message,"Can't repeat: more than %ld computations",(long)MAXDERIV);
		Alert1(Message);
		r = ABORT; goto OUT;
		}
	}
	
store = FALSE;
if(ComputeOn) {
	if(ProduceStackDepth > -1) store = TRUE;
	else if(Beta) Println(wTrace,"Err. Zouleb(). ProduceStackDepth == -1");
	}

p_index = NULL;

if(rndseq) {
	if(!repeat) ReseedOrShuffle(orgseed);
	r = MakeRandomSequence(&p_index,nchunks,repeat,store);
	if(r != OK) goto OUT;
	}
else {
	if(rotate != 0) {
		if(rotate < -128 || rotate > 127) {
			if(Beta) Alert1("Err. Zouleb(). rotate < -128 || rotate > 127");
			rotate = 0;
			}
		r = RotateSequence(&p_index,nchunks,rotate);
		if(r != OK) goto OUT;
		}
	}

// Now we copy the modified structure

i = iorg;
ib = ZERO;

ichunk = ZERO;

while(TRUE) {
	m = (**pp_a)[i]; p = (**pp_a)[i+1];
	
	if(m == TEND && p == TEND) {
		(*p_level)--;
		(*p_pos_init) = i + (2L * more);
		break;
		}
	if(m == T0 && p == 14) {		/* ',' */
		(*p_pos_init) = i + (2L * (more + 1));
		break;
		}
	if(m == T0 && (p == 13 || p == 23)) {	/* '}' */
		(*p_level)--;
		(*p_pos_init) = i + (2L * (isbracket + more));
		break;
		}
		
	if(m == T0 && (p == 12 || p == 22)) {	/* '{' */
		if((r=CheckBuffer(ib,&ibmax,&p_b)) != OK) goto OUT;
		GetChunk(p_chunk,&ichunk,nchunks,rndseq,retro,rotate,&ib,&ibmax,&i,*pp_a,p_b,p_index,(p == 22),
			&more);
		continue;
		}
		
	switch(m) {
		case T3:	/* sound-object */
		case T4:	/* variable */
		case T6:	/* wildcard */
		case T7:	/* out-time object or simple note */
		case T9:	/* time pattern */
		case T25:	/* simple note */
			GetChunk(p_chunk,&ichunk,nchunks,rndseq,retro,rotate,&ib,&ibmax,&i,*pp_a,p_b,p_index,NO,&more);
			continue;
			break;
		case T12:
			switch(p) {
				case 21:	/* _retro */
				case 22:	/* _rndseq */
				case 24:	/* _ordseq */
GETITBACK:
					i += 2L;	/* skip instruction */
					NeedZouleb--;
					GetChunk(p_chunk,&ichunk,nchunks,rndseq,retro,rotate,&ib,&ibmax,&i,*pp_a,p_b,p_index,NO,&more);
					continue;
				}
			break;
		case T39:	/* _rotate */
			goto GETITBACK;
			break;
		}
	
	if(m == T1) {	/* Number */
		GetChunk(p_chunk,&ichunk,nchunks,rndseq,retro,rotate,&ib,&ibmax,&i,*pp_a,p_b,p_index,NO,
			&more);
		continue;
		break;
		}
	
	if(m == T0 && (p == 11 || p == 21 || p == 24 || p == 25)) {	/* '/' '*' '**' '\' */
		do {
			(*p_b)[ib++] = m; (*p_b)[ib++] = p;
			if((r=CheckBuffer(ib,&ibmax,&p_b)) != OK) goto OUT;
			i += 2L;
			m = (**pp_a)[i]; p = (**pp_a)[i+1];
			}
		while(m == T1);
		continue;
		}
		
NEXT2:
	(*p_b)[ib++] = m; (*p_b)[ib++] = p;
	if((r=CheckBuffer(ib,&ibmax,&p_b)) != OK) goto OUT;
	i += 2L;
	}
(*p_b)[ib++] = TEND; (*p_b)[ib++] = TEND;
if((r=CheckBuffer(ib,&ibmax,&p_b)) != OK) goto OUT;

MyDisposeHandle((Handle*) &p_index);

// Copy b to a

if(more > 0) {
	i = iorg;
	while((**pp_a)[i] != TEND || (**pp_a)[i+1] != TEND) i += 2L;
	j = i + more + more;
	if((r=CheckBuffer(j+2L,&imax,pp_a)) != OK) goto OUT;
	for(j=j; ; i-=2L,j-=2L) {
		(**pp_a)[j] = (**pp_a)[i];
		(**pp_a)[j+1] = (**pp_a)[i+1];
		if(i == iorg) break;
		}
	}
if(more < 0) {
	i = iorg - more - more;
	for(j=iorg; (**pp_a)[i] != TEND || (**pp_a)[i+1] != TEND; i+=2L,j+=2L) {
		(**pp_a)[j] = (**pp_a)[i];
		(**pp_a)[j+1] = (**pp_a)[i+1];
		}
	(**pp_a)[j] = (**pp_a)[j+1] = TEND;
	}
for(i=iorg, ib=0; ; i+=2L, ib+=2L) {
	m = (*p_b)[ib];
	p = (*p_b)[ib+1];
	if(m == TEND && p == TEND) break;
	(**pp_a)[i] = m;
	(**pp_a)[i+1] = p;
	}

OUT:
MyDisposeHandle((Handle*)&p_chunk);
MyDisposeHandle((Handle*)&p_b);
return(r);
}


StoreChunk(ChunkPointer ***pp_chunk,long *p_i,long *p_imax,unsigned long origin,
	unsigned long end)
{
ChunkPointer** ptr;

(**pp_chunk)[*p_i].origin = origin;
(**pp_chunk)[*p_i].end = end;
(*p_i)++;
if((*p_i) >= (*p_imax)) {
	(*p_imax) = ((*p_imax) * 3L) / 2L;
	ptr = (*pp_chunk);
	if((ptr=(ChunkPointer**) IncreaseSpace((Handle)ptr)) == NULL) {
		return(ABORT);
		}
	(*pp_chunk) = ptr;
	}
return(OK);
}


GetChunk(ChunkPointer **p_chunk,long *p_ichunk,long ichunkmax,int rndseq,
	int retro,int rotate,unsigned long *p_ib,
	unsigned long *p_maxib,unsigned long *p_i,tokenbyte **p_a,tokenbyte **p_b,
	long **p_index,int istempbracket,int *p_more)
{
unsigned long i,j,origin,end;
long ichunk;
int s,cutbracket,found,key,foundit,
	needbracket,level,newlevel;
tokenbyte m,p,mm,pp;
char line[MAXLIN];

if(retro) ichunk = ichunkmax - (*p_ichunk) - 1L;
else ichunk = (*p_ichunk);

if(p_index != NULL) {	/* _rotate or _rndseq */
	ichunk = (*p_index)[ichunk];
	}

if(ichunk < ZERO) {
	if(Beta) Alert1("Err. GetChunk(). ichunk < ZERO");
	ichunk = ZERO;
	}
if(ichunk >= ichunkmax) {
	if(Beta) Alert1("Err. GetChunk(). ichunk >= ichunkmax");
	ichunk = ichunkmax - 1L;
	}

PleaseWait();

origin = (*p_chunk)[ichunk].origin;
end = (*p_chunk)[ichunk].end;

m = (*p_a)[origin]; p = (*p_a)[origin+1L];

// The following is for future compatibility
cutbracket = (istempbracket && (end - origin) > 2L);
if(cutbracket) {
	m = (*p_a)[origin+2L]; p = (*p_a)[origin+3L];
	switch(m) {
		case T39:	/* _rotate */
		case T40:	/* _keyxpand */
		case T26:	/* _transpose */
			cutbracket = FALSE;
			break;
		case T12:
			switch(p) {
				case 21:	/* _retro */
				case 22:	/* _rndseq */
				case 24:	/* _ordseq */
					cutbracket = FALSE;
					break;
				}
			break;
		}
	}
if(cutbracket) (*p_more) -= 2;

GETIT:

for(i=origin,level=0; i < end; i += 2L) {
	m = (*p_a)[i]; p = (*p_a)[i+1L];
	
	if(m == TEND && p == TEND) break;
	
	if(cutbracket && (i == origin || i == (end-2L))) continue;
	
	if(m == T0 && p == 22) istempbracket++;
	if(m == T0 && p == 23) istempbracket--;
	
	if(m == T0 && p == 13) level--;
		
	(*p_b)[(*p_ib)++] = m;
	(*p_b)[(*p_ib)++] = p;
	if(CheckBuffer((*p_ib),p_maxib,&p_b) != OK) return(ABORT);
	}

(*p_i) += ((*p_chunk)[*p_ichunk].end - (*p_chunk)[*p_ichunk].origin);
(*p_ichunk)++;

return(OK);
}


MakeRandomSequence(long ***pp_x,long size,int repeat,int store)
{
int result,r1,r2;
long pos,x;
double randomnumber;

if(size == ZERO) return(OK);
if(*pp_x == NULL) {
	*pp_x = (long**) GiveSpace((Size) size * sizeof(long));
	if(*pp_x == NULL) return(ABORT);
	}

if((result=MakeRandomSequence(pp_x,size-1L,repeat,store)) != OK) return(result);

if(!repeat) {
	r1 = rand(); r2 = rand();
	UsedRandom = TRUE;
	randomnumber = r1 + (((double)r2) / ((double)(RAND_MAX + 1L)));
	pos = (((double) size) * randomnumber) / ((double)(RAND_MAX + 1L));
	if(pos >= size) {
		if(Beta) Alert1("Err. MakeRandomSequence(). pos >= size");
		pos = size - 1L;
		}
	if(pos < ZERO) {
		if(Beta) Alert1("Err. MakeRandomSequence(). pos < ZERO");
		pos = ZERO;
		}
	if(store) {
		(*p_MemPos)[ProduceStackDepth] = pos;
		if(++ProduceStackDepth >= MaxDeriv && IncreaseComputeSpace() != OK) return(ABORT);
		ProduceStackIndex = ProduceStackDepth;
		}
	}
else {
	pos = (*p_MemPos)[ProduceStackIndex++];
	if(ProduceStackIndex > ProduceStackDepth) {
		repeat = FALSE;
		if(Answer("End of known computation.\nContinue",'Y') != YES) return(ABORT);
		}
	}
if(pos < (size - 1L)) {
	x = (**pp_x)[pos];
	(**pp_x)[pos] = size - 1L;
	}
else x = size - 1L;

(**pp_x)[size-1L] = x;

return(OK);
}


RotateSequence(long ***pp_x,long size,int rotate)
{
long i,j;

if(size < 1) return(OK);

if(*pp_x == NULL) {
	*pp_x = (long**) GiveSpace((Size) size * sizeof(long));
	if(*pp_x == NULL) return(ABORT);
	}

j = rotate % size;	/* Direction changed on 30/4/98 */
if(j < ZERO) j += size;

for(i=ZERO; i < size; i++) {
	(**pp_x)[i] = j;
	j++;
	if(j >= size) j = ZERO;
	}
return(OK);
}


TransposeKey(int *p_key,int trans)
{
int key,up;

key = (*p_key);
if(trans == 0) return(OK);

up = FALSE;
if(key > 16383) {
	up = TRUE;
	key -= 16384;
	}
	
key += (int) Round(((double)(trans)) / 100.);
while(key > 127) key -= 12;
while(key < 0) key += 12;
if(up) key += 16384;
(*p_key) = key;
return(OK);
}


ExpandKey(int key,short xpandkey,short xpandval)
{
tokenbyte newkey;
double val;
short centerkey,up;
int maxparam;

if(xpandkey == -1) return(key);

up = FALSE;
if(key > 16383) {
	up = TRUE;
	key -= 16384;
	}
	
if(xpandkey > 127) centerkey = ParamValue[xpandkey-128];
else centerkey = xpandkey;

if(centerkey < 0) {
	if(Beta) Println(wTrace,"Err. ExpandKey(). centerkey < 0");
	return(key);
	}

if(p_NumberConstant == NULL) maxparam = 0;
else maxparam = (MyGetHandleSize((Handle)p_NumberConstant) / sizeof(double));

if(xpandval >= maxparam) {
	if(Beta) Println(wTrace,"Err. ExpandKey(). xpandval >= maxparam");
	return(key);
	}
val = (*p_NumberConstant)[xpandval];
newkey = centerkey + Round(((double)(key - centerkey)) * val);
while(newkey > 127) newkey -= 12;
while(newkey < 0) newkey += 12;
if(up) newkey += 16384;
return(newkey);
}

