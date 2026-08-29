/* Zouleb.c (BP3) */

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

int DeleteSerialTools(tokenbyte ***pp_a) {
	// This is called after Zouleb()
	unsigned long i,j;
	tokenbyte m,p;
	i = j = ZERO;
	while(TRUE) {
		m = (**pp_a)[i+j]; p = (**pp_a)[i+j+1];
		if((m == T12 && (p == 21 || p == 22 || p == 24)) || m == T39) {
			j += 2;
			m = (**pp_a)[i+j]; p = (**pp_a)[i+j+1];
			}
		// DisplayCode(TRUE,i,m,p);
		if(j > ZERO) {
			(**pp_a)[i] = m;
			(**pp_a)[i+1] = p;
			}
		if(m == TEND && p == TEND) break;
		i += 2;
		}
	return(OK);
	}

int Zouleb(tokenbyte ***pp_a,unsigned long *p_pos_init,int level,int retro,int rndseq,int rotate,long rotate_init,int orgseed,unsigned long *p_ilimit,unsigned long *p_endprocess,int repeat) {

	int result,seed,newrotate,store,more,next_is_bracket;
	long ichunk,ichunkmax,ichunkstart,nchunks,**p_index;
	unsigned int currentseed;
	unsigned long i,j,ib,iorg,origin,end,ibmax;
	tokenbyte m,p,**p_b;
	ChunkPointer **p_chunk;

	// return(OK);

	if(pp_a == NULL) {
		BPPrintMessage(0,odError,"=> Err. Zouleb(). pp_a == NULL\n");
		return(ABORT);
		}
	result = OK;
	ichunk = ZERO;
	origin = end = ZERO;
	ichunkmax = FIELDSIZE;
	ibmax = FIELDSIZE;
	more = 0;

	// if(TraceZouleb) BPPrintMessage(0,odInfo,"@ Creating p_chunk table\n");
	p_chunk = (ChunkPointer**) GiveSpace((Size)ichunkmax * sizeof(ChunkPointer));
	if(p_chunk == NULL) return(ABORT);

	p_b = (tokenbyte**) GiveSpace((ibmax + 6L) * (Size)sizeof(tokenbyte));
	if(p_b == NULL) return(ABORT);

	seed = orgseed;
	i = iorg = (*p_pos_init);
	next_is_bracket = FALSE;
	if(TraceZouleb) BPPrintMessage(0,odInfo,"@@ starting Zouleb(), pos_init = %ld, level = %d, rotate = %d, rotate_init = %ld, ilimit = %ld\n",(*p_pos_init),level,rotate,rotate_init,*p_ilimit);
	if(rotate_init == Infpos) {
		rotate_init = rotate;
		if(TraceZouleb) BPPrintMessage(0,odInfo,"@ Starting polymetric expression, i = %ld, rotate_init = rotate = %ld\n",i,rotate_init);
		}
	if(rotate != 0) {
		rotate = (int) FindValue(T39,rotate,0);
	//	if(TraceZouleb) BPPrintMessage(0,odInfo,"@ FindValue rotate = %d\n",rotate);
		}

	while(TRUE) {
		if(next_is_bracket && (*p_endprocess) > ZERO && i > (*p_endprocess)) {
			level--;
			if(TraceZouleb) 
				BPPrintMessage(0,odInfo,"=> i (%ld) > endprocess (%ld) => level = %d\n",i,(*p_endprocess),level);
			(*p_endprocess) = ZERO;
			}
		m = (**pp_a)[i]; p = (**pp_a)[i+1];
		if(TraceZouleb) DisplayCode(TRUE,i,m,p);
	//	if(rndseq) BPPrintMessage(0,odInfo,"§§§rand§§§\n");
		if(m == TEND && p == TEND) {
			if(TraceZouleb) 
				BPPrintMessage(0,odInfo,"Found TEND\n");
			end = i;
			goto PROCESS;
			}
		if(m == T0 && p == 14) {	//  ','
			if(rotate_init != Infpos) newrotate = rotate_init;
			else newrotate = rotate;
			origin = i; // Includes ','
			end = NextEnd(pp_a,i,&next_is_bracket); // Find the end to store the chunk
			if(TraceZouleb) 
				BPPrintMessage(0,odInfo,"§ origin of field = %ld, end = %ld, level = %d, rotate = %d, next_is_bracket = %d\n",origin,end,level+1,rotate,next_is_bracket);
			next_is_bracket = FALSE;
			i += 2L;
			if(TraceZouleb) BPPrintMessage(0,odInfo,"@ after comma, rotate_init = %ld, newrotate = %d, level = %d\n",rotate_init,newrotate,level);
			if((result = Zouleb(pp_a,&i,++level,retro,rndseq,newrotate,rotate_init,seed,p_ilimit,p_endprocess,repeat)) != OK) goto EXITZOULEB;
			level--;
			if(level < 0) BPPrintMessage(0,odError,"=> Error Zouleb() level = %d\n",level);
			if(TraceZouleb) {
				BPPrintMessage(0,odInfo,"@ after Zouleb ',', level = %d, i = %ld\n",level,i);
				}
			end = i;
			goto PROCESS;
			break;
			}
		if(m == T0 && (p == 13 || p == 23)) {	//  '}' or  temp '}'
			if(TraceZouleb) 
				BPPrintMessage(0,odInfo,"Found '}', i = %ld, level = %d\n",i,level);
			if(level < 0) BPPrintMessage(0,odInfo,"=> Error Zouleb() level = %d\n",level);
			end = i + 2L;
			goto PROCESS;
			}
		if(m == T0 && (p == 12 || p == 22)) {	// '{', temp '{'
			if(TraceZouleb) 
				BPPrintMessage(0,odInfo,"Found '{', i = %ld\n",i);
			origin = i; // Includes '{'
			end = NextEnd(pp_a,i,&next_is_bracket); // Find the end to store the chunk
			next_is_bracket = FALSE;
			if(TraceZouleb) 
				BPPrintMessage(0,odInfo,"§ origin of {} = %ld, end = %ld, level = %d, next_is_bracket = %d\n",origin,end,level+1,next_is_bracket);
			i += 2L;
			if((result = Zouleb(pp_a,&i,++level,retro,rndseq,rotate,Infpos,seed,p_ilimit,p_endprocess,repeat)) != OK) goto EXITZOULEB;
			level--;
			if(level < 0) BPPrintMessage(0,odError,"=> Error Zouleb() level = %d\n",level);
			if(TraceZouleb) {
				BPPrintMessage(0,odInfo,"@ after Zouleb {}, level = %d, i = %ld, origin = %ld, end = %ld, ichunk = %ld\n",level,i,origin,end,ichunk);
				BPPrintMessage(0,odInfo,"@ endprocess = %ld\n",*p_endprocess);
				BPPrintMessage(0,odInfo,"Level %d ",level);
				}
			if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
			}
		if(m == T12) {
			switch(p) {
				case 21:	// _retro
					origin = i; // Includes '_retro()'
					end = NextEnd(pp_a,i,&next_is_bracket); // Find the end to store the chunk
					if(TraceZouleb) 
						BPPrintMessage(0,odInfo,"§ origin of retro = %ld, end = %ld\n",origin,end);
					i += 2L;
					(*p_endprocess) = end;
					if((result = Zouleb(pp_a,&i,++level,1-retro,rndseq,rotate,rotate_init,seed,p_ilimit,p_endprocess,repeat)) != OK)
						goto EXITZOULEB;
					level--;
					rndseq = FALSE; // Needed for last items of -da.trySerialTools
					if(level < 0) BPPrintMessage(0,odError,"=> Error Zouleb() level = %d\n",level);
					if(TraceZouleb) {
						BPPrintMessage(0,odInfo,"@ after Zouleb _retro, level = %d, i = %ld\n",level,i);
						BPPrintMessage(0,odInfo,"Level %d ",level);
						}
					if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
					if(next_is_bracket) {
						if(TraceZouleb) 
							BPPrintMessage(0,odInfo,"=> next_is_bracket after _retro => continue, i = %ld\n",i);
						continue;
						}
					break;
				case 22:	// _rndseq
					origin = i; // Includes '_rndseq()'
					end = NextEnd(pp_a,i,&next_is_bracket); // Find the end to store the chunk
					if(TraceZouleb) BPPrintMessage(0,odInfo,"§ origin of _rndseq = %ld, end = %ld\n",origin,end);
					i += 2L;
					(*p_endprocess) = end;
					if((result = Zouleb(pp_a,&i,++level,retro,TRUE,rotate,rotate_init,seed,p_ilimit,p_endprocess,repeat)) != OK)
						goto EXITZOULEB;
					level--;
			//		rndseq = FALSE; // Needed for last items of -da.trySerialTools
					if(level < 0) BPPrintMessage(0,odError,"=> Error Zouleb() level = %d\n",level);
					if(TraceZouleb) {
						BPPrintMessage(0,odInfo,"@ after Zouleb _rndseq, level = %d, i = %ld, rndseq = %d\n",level,i,rndseq);
						BPPrintMessage(0,odInfo,"Level %d ",level);
						}
					if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
					if(next_is_bracket) {
						if(TraceZouleb) 
							BPPrintMessage(0,odInfo,"=> next_is_bracket after _rndseq => continue, i = %ld\n",i);
						continue;
						}
					break;
				case 23:	// _randomize
					origin = i; end = i + 2L;
					if(TraceZouleb) BPPrintMessage(0,odInfo,"@ Randomizing: _randomize\n");
					seed = RANDOMIZE;
					Seed = 0;
					if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
					break;
				case 24:	// _ordseq
					origin = i; // Includes '_ordseq()'
					end = NextEnd(pp_a,i,&next_is_bracket); // Find the end to store the chunk
					if(TraceZouleb) BPPrintMessage(0,odInfo,"§ origin of _ordseq = %ld, end = %ld\n",origin,end);
					i += 2L;
					(*p_endprocess) = end;
					if((result = Zouleb(pp_a,&i,++level,0,0,0,rotate_init,seed,p_ilimit,p_endprocess,repeat)) != OK)
						goto EXITZOULEB;
					level--;
					if(level < 0) BPPrintMessage(0,odError,"=> Error Zouleb() level = %d\n",level);
					if(TraceZouleb) {
						BPPrintMessage(0,odInfo,"@ after Zouleb _ordseq, level = %d, i = %ld\n",level,i);
						BPPrintMessage(0,odInfo,"Level %d ",level);
						}
					if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
					if(next_is_bracket) {
						if(TraceZouleb) 
							BPPrintMessage(0,odInfo,"=> next_is_bracket after _ordseq => continue, i = %ld\n",i);
						continue;
						}
					break;
				default:
					origin = i; end = i + 2L;
					if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
					break;
				}
			}
		switch(m) {
			case T39:	/* _rotate */
				newrotate = rotate + (int) FindValue(m,p,0);
				origin = i; // Includes '_rotate()'
				end = NextEnd(pp_a,i,&next_is_bracket); // Find the end to store the chunk
				if(TraceZouleb)
					BPPrintMessage(0,odInfo,"§ origin of _rotate = %ld, end = %ld, newrotate = %d, level = %d, next_is_bracket = %d\n",origin,end,newrotate,level+1,next_is_bracket);
				i += 2L;
				if(TraceZouleb) BPPrintMessage(0,odInfo,"@ old rotate = %d, newrotate = %d, rotate_init = %ld, end = %ld, level = %d\n",rotate,newrotate,rotate_init,end,level);
				(*p_endprocess) = end;
				if((result = Zouleb(pp_a,&i,++level,retro,rndseq,newrotate,rotate_init,seed,p_ilimit,p_endprocess,repeat)) != OK)
					goto EXITZOULEB;
				level--;
				rndseq = FALSE; // Needed for last items of -da.trySerialTools
				if(level < 0) BPPrintMessage(0,odError,"=> Error Zouleb() level = %d\n",level);
				if(TraceZouleb) {
					BPPrintMessage(0,odInfo,"@ after Zouleb _rotate, level = %d, i = %ld\n",level,i);
					BPPrintMessage(0,odInfo,"Level %d ",level);
					}
				if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
				if(next_is_bracket) {
					if(TraceZouleb) 
						BPPrintMessage(0,odInfo,"=> next_is_bracket after _rotate => continue, i = %ld\n",i);
					continue;
					}
				break;
			case T2:	// Parenthesis marker
				origin = i; end = i + 2L;
				if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
				break;
			case T5:	// Homomorphism
			case T6:	// Wild card
				origin = i; end = i + 2L;
				if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
				break;
			case T3:	// Terminal or silence
			case T4:	// Variable
			case T9:	// Time pattern
			case T25:	// Simple note
				origin = i;
				do {	// Include prolongation symbols '_'
					i += 2L;
					m = (**pp_a)[i]; p = (**pp_a)[i+1];
					}
				while((m == T3 || m == T25 || m == T4 || m == T9) && p == 0);
				end = i;
				m = (**pp_a)[i];
				p = (**pp_a)[i+1];
				if(m == T0 && p == 18) { // '&' following terminal or variable
					if(TraceZouleb) BPPrintMessage(0,odInfo,"@@@ '&' following terminal\n");
					i += 2L;
					end = i;
					}
				if(TraceZouleb) BPPrintMessage(0,odInfo,"Note level %d, ",level);
				if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
				continue;
				break;
			case T7:	// Out-time object or note
			case T8:	// Synchronization tag
			case T10:	// _chan()
			case T11:	// _vel()
			case T13:	// _script()
			case T14:	// _mod()
			case T15:	// _pitchbend()
			case T16:	// _press()
			case T17:	// _switchon()
			case T18:	// _switchoff()
			case T19:	// _volume()
			case T20:	// _legato()
			case T21:	// _pitchrange()
			case T22:	// _pitchrate()
			case T23:	// _modrate()
			case T24:	// _pressrate()
			case T26:	// _transpose()
			case T27:	// _volumerate()
			case T28:	// _volumecontrol()
			case T29:	// _pan()
			case T30:	// _panrate()
			case T31:	// _pancontrol()
			case T32:	// _ins()
			case T33:	// _step()
			case T34:	// _cont()
			case T35:	// _value()
			case T36:	// _fixed()
			case T38:	// _rndvel()
			case T40:	// _keyxpand()
			case T41:	// _rndtime()
			case T42:	// _srand()
			case T44:	// _scale()
			case T45:	// _capture()
			case T46:	// _part()
			case T47:	// Remaining variable processed as a silent sound-object
				origin = i; end = i + 2L;
				if(TraceZouleb) BPPrintMessage(0,odInfo,"4) ");
				if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
				break;
			case T37:	// _keymap()
			case T43:	// _tempo()
				origin = i; end = i + 4L;
				if(TraceZouleb) BPPrintMessage(0,odInfo,"§ origin of _tempo or _keymap = %ld, end = %ld\n",origin,end);
				if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
				i += 2L;
				break;
			}
		if(m == T1) {	/* Number */
			origin = i;
			do {
				i += 2L;
				m = (**pp_a)[i];
				}
			while(m == T1);
			if((**pp_a)[i] == T0 && (**pp_a)[i+1] == 11) {	// '/'
				do {
					i += 2L;
					m = (**pp_a)[i];
					}
				while(m == T1);
				}
			end = i;
			if(TraceZouleb) {
				BPPrintMessage(0,odInfo,"Level %d ",level);
				BPPrintMessage(0,odInfo,"Number ");
				}
			if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
			continue;
			}
		if(m == T0 && (p == 11 || p == 21 || p == 24 || p == 25)) {
			// '/' '*' '**' '\' 
			origin = i;
			do {
				i += 2L;
				m = (**pp_a)[i];
				}
			while(m == T1);
			end = i;
			if(TraceZouleb) BPPrintMessage(0,odInfo,"6) ");
			if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
			continue;
			}
		if(m == T0 && p == 19) { // '&' preceding terminal or variable
			origin = i;
			m = (**pp_a)[i+4];
			p = (**pp_a)[i+5];
			if(m == T0 && p == 18) // '&' following terminal or variable
				i += 6;
			else i += 4;
			end = i;
			if(TraceZouleb) BPPrintMessage(0,odInfo,"§ origin of '&' preceding = %ld, end = %ld\n",origin,end);
			if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
			continue;
			}
		if(m == T0) {
			switch(p) {
				case 1:	// Wildcard
				case 2:	// Negative context
				case 3:	// Structural marker '+'
				case 4:	// Structural marker ':'
				case 5:	// Structural marker ';'
				case 6:	// Structural marker '='
				case 7:	// Period
				case 8:	// '('
				case 9:	// ')'
				case 10: // 'S'
				case 15: // Not used
				case 16: // Not used
				case 17: // Rest '_'
				case 20: // Pattern marker '!'
					origin = i; end = i + 2L;
					if((result = StoreChunk(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto EXITZOULEB;
					break;
				}
			}
		i += 2L;
		}

PROCESS:
	// Now we process rotations, retro, rndseq
	p_index = NULL;
	nchunks = ichunk;
	if(nchunks < 2) goto EXITZOULEB; // Not necessary, only speeds up

	if(repeat) {
		if(ProduceStackDepth == -1) {
			my_sprintf(Message,"Can't repeat: more than %ld computations",(long)MAXDERIV);
			BPPrintMessage(0,odError,"%s",Message);
			result = ABORT; goto EXITZOULEB;
			}
		}
		
	store = FALSE;
	if(ComputeOn) {
		if(ProduceStackDepth > -1) store = TRUE;
		else BPPrintMessage(0,odError,"=> Err. Zouleb(). ProduceStackDepth == -1\n");
		}

	if(rndseq) {
		if(!repeat) ReseedOrShuffle(orgseed);
		if(TraceZouleb) BPPrintMessage(0,odInfo,"@ MakeRandomSequence, nchunks = %ld\n",nchunks);
		result = MakeRandomSequence(&p_index,nchunks,repeat,store);
		if(result != OK) goto EXITZOULEB;
		}
	else {
		if(rotate < -128 || rotate > 127) {
			BPPrintMessage(0,odError,"=> Err. Zouleb(). rotate < -128 || rotate > 127\n");
			rotate = 0;
			}
		ichunkstart = ZERO;
		if(TraceZouleb) BPPrintMessage(0,odInfo,"@ RotateSequence, level = %d, ifirstchunk = %ld, nchunks = %ld, rotate = %d\n",level,ichunkstart,nchunks,rotate);
		result = RotateSequence(&p_index,ichunkstart,nchunks,rotate);
		if(result != OK) goto EXITZOULEB;
		}

	// Copy processed chunks to p_b
	ib = ZERO;
	i = iorg;
	int istempbracket = 0;
	for(ichunk = 0; ichunk < nchunks; ichunk++) {
		if((result = GetChunk(p_chunk,&ichunk,nchunks,rndseq,retro,rotate,&ib,&ibmax,&i,*pp_a,p_b,p_index,istempbracket,&more)) != OK) goto EXITZOULEB;
		}
	if(TraceZouleb) 
		BPPrintMessage(0,odInfo,"§§ TEND for ib = %ld\n",ib);
	(*p_b)[ib++] = TEND;
	(*p_b)[ib++] = TEND;

	MyDisposeHandle((Handle*) &p_index);

	// Copy p_b to p_a
/*	if(TraceZouleb) BPPrintMessage(0,odInfo,"@@@ more = %d, iorg = %ld, level = %d\n",more,iorg,level);
	if(more > 0) {
		if(TraceZouleb) BPPrintMessage(0,odInfo,"=> more = %d\n",more);
		i = iorg;
		while((**pp_a)[i] != TEND && (**pp_a)[i+1] != TEND) i += 2L;
		j = i + more + more;
		if((result = CheckBuffer(j+2L,&ibmax,pp_a)) != OK) goto EXITZOULEB;
		for(j=j; ; i-=2L,j-=2L) {
			(**pp_a)[j] = (**pp_a)[i];
			(**pp_a)[j+1] = (**pp_a)[i+1];
			if(i == iorg) break;
			}
		}
	if(more < 0) {
		if(TraceZouleb) {
				BPPrintMessage(0,odInfo,"=> more = %d",more);
				DisplayCode(TRUE,i,m,p);
				}
		i = iorg - more - more;
		for(j=iorg; ; i+=2L,j+=2L) {
			m = (**pp_a)[i];
			p = (**pp_a)[i+1];
			if(m == T39) {
				if(TraceZouleb) BPPrintMessage(0,odInfo,"=> m == T39\n");
				i += 2L;
				m = (**pp_a)[i];
				p = (**pp_a)[i+1];
				more++;
				}
			(**pp_a)[j] = m;
			(**pp_a)[j+1] = p;
			if(m == TEND && p == TEND) break;
			}
		(**pp_a)[j] = (**pp_a)[j+1] = TEND;
		} */

	for(i = iorg, ib = - more - more; ; i+=2L, ib+=2L) {
		m = (*p_b)[ib];
		p = (*p_b)[ib+1];
		if(m == TEND && p == TEND) break;
		if(TraceZouleb) {
			BPPrintMessage(0,odInfo,"@ Copying from p_b: ");
			DisplayCode(TRUE,i,m,p);
			}
		(**pp_a)[i] = m;
		(**pp_a)[i+1] = p;
		}

EXITZOULEB:
	if(TraceZouleb) 
		BPPrintMessage(0,odInfo,"@@ exiting Zouleb(), pos_init = %ld, level = %d, result = %d, end = %ld, ilimit = %ld\n",*p_pos_init,level,result,end,(*p_ilimit));
	MyDisposeHandle((Handle*)&p_b);
	MyDisposeHandle((Handle*)&p_chunk);
	if(end > (*p_ilimit)) {
	//	BPPrintMessage(0,odInfo,"@@ § ilimit = %ld, end = %ld\n",(*p_ilimit),end);
		(*p_ilimit) = end;
		}
	(*p_pos_init) = (*p_ilimit) - 2L;
	return result;
	}


int GetChunk(ChunkPointer **p_chunk,long *p_ichunk,long nchunks,int rndseq,
	int retro,int rotate,unsigned long *p_ib,
	unsigned long *p_maxib,unsigned long *p_i,tokenbyte **p_a,tokenbyte **p_b,
	long **p_index,int istempbracket,int *p_more) {
	unsigned long i,j,origin,end;
	long ichunk;
	int s,found,key,foundit,needbracket,n;
	tokenbyte m,p,mm,pp;
	char line[MAXLIN];

	if(retro) ichunk = nchunks - (*p_ichunk) - 1L;
	else ichunk = (*p_ichunk);
	if(ichunk >= nchunks) {
		if(TraceZouleb) BPPrintMessage(0,odInfo,"=> ichunk = %ld ( >= nchunks)\n",ichunk);
		(*p_i) +=  2L;
		return(OK);
		}
	if(p_index != NULL) {	/* _rotate or _rndseq */
		ichunk = (*p_index)[ichunk];
		}
	if(ichunk > nchunks) {
		BPPrintMessage(0,odError,"=> Error GetChunk() ichunk (%ld) > nchunks (%ld)\n",ichunk,nchunks);
		return(STOP);
		}
	if(ichunk < ZERO) {
		BPPrintMessage(0,odError,"=> Err. GetChunk(). ichunk < ZERO\n");
		return(STOP);
		}
	origin = (*p_chunk)[ichunk].origin;
	end = (*p_chunk)[ichunk].end;
	if(TraceZouleb) 
		BPPrintMessage(0,odInfo,"@GetChunk chunk[%ld], origin = %ld, end = %ld, nchunks = %ld, ib = %ld\n",ichunk,origin,end,nchunks,(*p_ib));

/*	// The following is for future compatibility ???
	cutbracket = (istempbracket && (end - origin) > 2L);
	if(cutbracket) {
		m = (*p_a)[origin+2L]; p = (*p_a)[origin+3L];
		switch(m) {
			case T39:	// _rotate
			case T40:	 // _keyxpand
			case T26:	// _transpose
				cutbracket = FALSE;
				break;
			case T12:
				switch(p) {
					case 21:	// _retro 
					case 22:	 // _rndseq
					case 24:	 // _ordseq
						cutbracket = FALSE;
						break;
					}
				break;
			}
		}
	if(cutbracket) {
		(*p_more) -= 2;
		if(TraceZouleb) 
			BPPrintMessage(0,odInfo,"=> §§ cutbracket = TRUE, more = %d\n",(*p_more));
		} */
	for(i=origin; i < end; i += 2L) {
		m = (*p_a)[i]; p = (*p_a)[i+1L];
		if(m == TEND && p == TEND) break;
/*		if(cutbracket && (i == origin || i == (end-2L))) {
			if(TraceZouleb) BPPrintMessage(0,odInfo,"=> continue i = %d, origin = %d, end = %d\n",i,origin,end);
			continue;
			} */
//		if(m == T0 && p == 22) istempbracket++;	// temp '{'
//		if(m == T0 && p == 23) istempbracket--;	// temp '}'
		if(TraceZouleb) 
			DisplayCode(TRUE,i,m,p);
		(*p_b)[(*p_ib)++] = m;
		(*p_b)[(*p_ib)++] = p;
		if(CheckBuffer((*p_ib),p_maxib,&p_b) != OK) return(ABORT);
		}
	(*p_i) += end - origin;
	if(TraceZouleb) 
		BPPrintMessage(0,odInfo,"@end of GetChunk, i = %ld, origin = %ld, end = %ld\n",(*p_i),origin,end);
	return(OK);
	}


unsigned long NextEnd(tokenbyte ***pp_a,unsigned long iorg,int *p_next_is_bracket) {
	// Find the end of _rotate() or _rndseq() or _retro or polymetric structure
	tokenbyte m,p;
	int level,is_poly,is_field,is_ordseq,is_rndseq,is_retro,is_rotate;
	unsigned long i;

	level = 0; i = iorg;
	is_poly = is_field = is_ordseq = is_rndseq = is_retro = is_rotate = (*p_next_is_bracket) = FALSE;
	while(TRUE) {
		m = (**pp_a)[i]; p = (**pp_a)[i+1];
		if(m == TEND && p == TEND) return(i);
		if(i == iorg && m == T0 && p == 14) is_field = TRUE; // ','
		if(i == iorg && m == T12 && p == 24) is_ordseq = TRUE; // '_ordseq'
		if(i == iorg && m == T12 && p == 22) is_rndseq = TRUE; // '_rndseq'
		if(i == iorg && m == T12 && p == 21) is_retro = TRUE; // '_retro'
		if(i == iorg && m == T39) is_rotate = TRUE; // '_rotate'
		if(m == T0 && (p == 12 || p == 22)) {	// '{', temp '{'
			if(i > iorg) level++;
			else is_poly = TRUE;
			}
		if(m == T0 && (p == 13 || p == 23)) {	//  '}' or  temp '}'
			if(level <= 0) {
				(*p_next_is_bracket) = TRUE;
				if(is_field || is_ordseq) return(i);
				else return(i + 2L);
				}
			level--;
			}
		if(i > iorg && !is_poly && level == 0) { 
			if(m == T0 && p == 14) {	//  ','
				(*p_next_is_bracket) = TRUE;
				return(i);
				}
			if(m == T12 && (p == 21 || p == 22)) {
				//  _retro, _rndseq
				if(!is_retro && !is_rndseq && !is_rndseq && !is_rotate) (*p_next_is_bracket) = TRUE;
				return(i);
				}
			}
		i += 2L;
		}
	return(ZERO);
	}

void DisplayCode(int shownumber,unsigned long i, tokenbyte m, tokenbyte p) {
	char line[100];
	if(m == T3 && p > 1 && p < Jbol) my_sprintf(line,"%s",*((*p_Bol)[p]));
	else if(m == T25) PrintThisNote(-1,p,0,-1,line);
	else if(m == T11) my_sprintf(line,"_vel(%d)\n",p);
	else if(m == T20 && p >= 0) my_sprintf(line,"_legato(%d)\n",p);
	else if(m == T20 && p < 0) my_sprintf(line,"_staccato(%d)\n",-p);
	else if(m == T26) my_sprintf(line,"_transpose(%d)\n",p/100);
	else if(m == T39) my_sprintf(line,"_rotate(%d)\n",p);
	else if(m == T43) my_sprintf(line,"_tempo %d\n",p);
	else if(m == T12 && p == 1) my_sprintf(line,"_velcont\n");
	else if(m == T12 && p == 0) my_sprintf(line,"_velstep\n");
	else if(m == T0 && p == 11) my_sprintf(line,"/\n");
	else if(m == T0 && p == 12) my_sprintf(line,"{\n");
	else if(m == T0 && p == 13) my_sprintf(line,"}\n");
	else if(m == T0 && p == 14) my_sprintf(line,",\n");
	else if(m == TEND && p == TEND) my_sprintf(line,"END\n");
	else my_sprintf(line,"%d %d\n",m,p);
	if(shownumber) BPPrintMessage(0,odInfo,"-> [%ld] %s\n",i,line);
	else BPPrintMessage(0,odInfo,"%s\n",line);
	return;
	}

int StoreChunk(ChunkPointer ***pp_chunk,long *p_ichunk,long *p_imax,unsigned long origin,unsigned long end) {
	ChunkPointer** ptr;
	if(TraceZouleb) BPPrintMessage(0,odInfo,"StoreChunk[%ld] origin = %ld, end = %ld\n",*p_ichunk,origin,end);
	if((*pp_chunk) == NULL) BPPrintMessage(0,odError,"=> Error StoreChunk(), (*pp_chunk) is NULL\n");
	(**pp_chunk)[*p_ichunk].origin = origin;
	(**pp_chunk)[*p_ichunk].end = end;
	(*p_ichunk)++;
	if((*p_ichunk) >= (*p_imax)) {
		(*p_imax) = ((*p_imax) * 3L) / 2L;
		ptr = (*pp_chunk);
		if((ptr=(ChunkPointer**) IncreaseSpace((Handle)ptr)) == NULL) {
			return(ABORT);
			}
		(*pp_chunk) = ptr;
		}
	return(OK);
	}

int MakeRandomSequence(long ***pp_index, long nchunks, int repeat, int store) {
    long i, pos, x;
    int r1, r2;

    if(nchunks <= 0L) return(OK);
    if(*pp_index == NULL) {
        *pp_index = (long **)GiveSpace((Size)nchunks * sizeof(***pp_index));
        if(*pp_index == NULL) return(ABORT);
    	}
    for(i = 0L; i < nchunks; i++) {
        // At this point, positions 0 .. i-1 already form a shuffle.
        (**pp_index)[i] = i;
        if(!repeat) {
            // Select an integer uniformly in the interval 0 .. i.
            // The second draw supplies a fractional part.
            r1 = bp3_rand();
            r2 = bp3_rand();
            UsedRandom = TRUE;
            pos = (long)((double)(i + 1L) *
                ((double)r1 + (double)r2 / (double)(BP3_RAND_MAX + 1L)) /
                (double)(BP3_RAND_MAX + 1L));
            if(store) {
                (*p_MemPos)[ProduceStackDepth] = pos;
                if(++ProduceStackDepth >= MaxDeriv && IncreaseComputeSpace() != OK)
                    return(ABORT);
                ProduceStackIndex = ProduceStackDepth;
				}
			}
        else {
            pos = (*p_MemPos)[ProduceStackIndex++];
            if(ProduceStackIndex > ProduceStackDepth) {
                repeat = FALSE;
                if(Answer("End of known computation.\nContinue", 'Y') != YES)
                    return(ABORT);
				}
			}
        if(pos < 0L || pos > i) {
            BPPrintMessage(0, odError,
                "=> Err. MakeRandomSequence(): invalid position %ld\n", pos);
            return(ABORT);
        	}
        // Swap the new item with a randomly selected earlier position.
        x = (**pp_index)[pos];
        (**pp_index)[pos] = (**pp_index)[i];
        (**pp_index)[i] = x;
    	}
    return(OK);
	}

int RotateSequence(long ***pp_x,long ifirstchunk,long nchunks,int rotate) {
	long i,j;
	long size;
	if(nchunks <= ifirstchunk) {
		BPPrintMessage(0,odError,"=> Error RotateSequence() nchunks(%ld) <= ifirstchunk(%ld)\n",nchunks,ifirstchunk);
		return(STOP);
		}
	size = nchunks - ifirstchunk;
	if(*pp_x == NULL) {
		*pp_x = (long**) GiveSpace((Size) size * sizeof(long));
		if(*pp_x == NULL) return(ABORT);
		}
	j = rotate % size;
	if(j < ZERO) j += size;
	j += ifirstchunk;
	if(TraceZouleb) BPPrintMessage(0,odInfo,"@ Rotating size = %ld, j start = %ld\n",size,j);
	for(i=ZERO; i < size; i++) {
		(**pp_x)[i] = j;
		if(TraceZouleb) BPPrintMessage(0,odInfo,"(**pp_x)[%ld] = chunk %ld\n",i,j);
		j++;
		if(j > (nchunks - 1L)) j = ifirstchunk;
		}
	return(OK);
	}

int TransposeKey(int *p_key,int trans) {
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


int ExpandKey(int key,short xpandkey,short xpandval) {
	double val;
	short centerkey,up;
	int maxparam, newkey;

	if(xpandkey == -1) return(key);
	up = FALSE;
	if(key > 16383) {
		up = TRUE;
		key -= 16384;
		}	
	if(xpandkey > 127) centerkey = ParamValue[xpandkey-128];
	else centerkey = xpandkey;
	if(centerkey < 0) {
		BPPrintMessage(0,odError,"=> Err. ExpandKey(). centerkey < 0\n");
		return(key);
		}
	if(p_NumberConstant == NULL) maxparam = 0;
	else maxparam = (MyGetHandleSize((Handle)p_NumberConstant) / sizeof(double));
	if(xpandval >= maxparam) {
		BPPrintMessage(0,odError,"=> Err. ExpandKey(). xpandval >= maxparam\n");
		return(key);
		}
	val = (*p_NumberConstant)[xpandval];
	// BPPrintMessage(1,odInfo,"@@@ xpandval = %d, val = %.4f\n",xpandval,val);
	newkey = centerkey + Round(((double)(key - centerkey)) * val);
	while(newkey > 127) newkey -= 12;
	while(newkey < 0) newkey += 12;
	if(up) newkey += 16384;
	return(newkey);
	}

//--------- OLD VERSION ---------//

// This is the old code which was used upt to BP3.5.1 (August 2026) and is still active when IgnoreFields is TRUE.

int ZoulebOld(tokenbyte ***pp_a,int *p_level,unsigned long *p_pos_init,int retro,
	int rndseq,int rotate,int repeat,int isbracket,int orgseed) {
	
	// Modify order of time-objects in a sequence, due to instructions:
	// _retro, _ordseq, _rndseq, or _rotate

	unsigned long i,imax,ib,ibmax,iorg,j,newpos,origin,end;
	long ichunk,nchunks,ichunkmax,**p_index,x;
	tokenbyte m,p,**p_b;
	int r,orglevel,newlevel,more,store,newrotate,maxparam,seed,ischanged;
	unsigned int currentseed;
	ChunkPointer **p_chunk;
	char line[MAXLIN];

	PleaseWait();

	if(pp_a == NULL) {
		BPPrintMessage(0,odError,"=> Err. Zouleb(). pp_a == NULL");
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
		if(TraceZouleb) DisplayCode(TRUE,i,m,p);
		if(m == TEND && p == TEND) break;
		if(m == T0 && (p == 13 || p == 14 || p == 23))	/*  ',' or  '}' */
			break;
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
				if((r=StoreChunkOld(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK)
					goto SORTIR;
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
						if((r=ZoulebOld(pp_a,&newlevel,&i,1-retro,rndseq,rotate,repeat,FALSE,seed))
							!= OK) goto SORTIR;
						if(!retro) ischanged = FALSE;
						break;
					case 22:	/* _rndseq */
						if((r=ZoulebOld(pp_a,&newlevel,&i,FALSE,TRUE,0,repeat,FALSE,seed))
							!= OK) goto SORTIR;
						ischanged = FALSE;
						break;
					case 24:	/* _ordseq */
						if((r=ZoulebOld(pp_a,&newlevel,&i,FALSE,FALSE,0,repeat,FALSE,seed))
							!= OK) goto SORTIR;
						break;
					}
	STORE:
				end = i;
				if((r=StoreChunkOld(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK)
					goto SORTIR;
				more--;
				continue;
				break;
			case T39:	/* _rotate */
				if(p == 0) goto NEXT;
				newrotate = FindValue(m,p,0);
				i += 2L;
				origin = i;
				newlevel = orglevel;
				if((r=ZoulebOld(pp_a,&newlevel,&i,retro,rndseq,rotate+newrotate,
						repeat,FALSE,seed)) != OK)
					goto SORTIR;
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
			if((r=StoreChunkOld(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto SORTIR;
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
				r = ZoulebOld(pp_a,&newlevel,&newpos,retro,rndseq,rotate,repeat,TRUE,seed);
				if(r != OK) goto SORTIR;
				}
			while(newlevel > orglevel);
			end = i = newpos;
			ischanged = TRUE;
			if(retro || rndseq || (rotate != 0)) ischanged = FALSE;
			if((r=StoreChunkOld(&p_chunk,&ichunk,&ichunkmax,origin,end)) != OK) goto SORTIR;
			continue;
			}
	NEXT:
		i += 2L;
		}

	nchunks = ichunk;

	if(repeat) {
		if(ProduceStackDepth == -1) {
			my_sprintf(Message,"Can't repeat: more than %ld computations",(long)MAXDERIV);
			BPPrintMessage(0,odError,"%s",Message);
			r = ABORT; goto SORTIR;
			}
		}
		
	store = FALSE;
	if(ComputeOn) {
		if(ProduceStackDepth > -1) store = TRUE;
		else BPPrintMessage(0,odError,"=> Err. Zouleb(). ProduceStackDepth == -1\n");
		}

	p_index = NULL;

	if(rndseq) {
		if(!repeat) ReseedOrShuffle(orgseed);
		r = MakeRandomSequenceOld(&p_index,nchunks,repeat,store);
		if(r != OK) goto SORTIR;
		}
	else {
		if(rotate != 0) {
			if(rotate < -128 || rotate > 127) {
				BPPrintMessage(0,odError,"=> Err. Zouleb(). rotate < -128 || rotate > 127");
				rotate = 0;
				}
			r = RotateSequenceOld(&p_index,nchunks,rotate);
			if(r != OK) goto SORTIR;
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
			if((r=CheckBuffer(ib,&ibmax,&p_b)) != OK) goto SORTIR;
			GetChunkOld(p_chunk,&ichunk,nchunks,rndseq,retro,rotate,&ib,&ibmax,&i,*pp_a,p_b,p_index,(p == 22),
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
				GetChunkOld(p_chunk,&ichunk,nchunks,rndseq,retro,rotate,&ib,&ibmax,&i,*pp_a,p_b,p_index,NO,&more);
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
						GetChunkOld(p_chunk,&ichunk,nchunks,rndseq,retro,rotate,&ib,&ibmax,&i,*pp_a,p_b,p_index,NO,&more);
						continue;
					}
				break;
			case T39:	/* _rotate */
				goto GETITBACK;
				break;
			}
		
		if(m == T1) {	/* Number */
			GetChunkOld(p_chunk,&ichunk,nchunks,rndseq,retro,rotate,&ib,&ibmax,&i,*pp_a,p_b,p_index,NO,
				&more);
			continue;
			break;
			}
		
		if(m == T0 && (p == 11 || p == 21 || p == 24 || p == 25)) {	/* '/' '*' '**' '\' */
			do {
				(*p_b)[ib++] = m; (*p_b)[ib++] = p;
				if((r=CheckBuffer(ib,&ibmax,&p_b)) != OK) goto SORTIR;
				i += 2L;
				m = (**pp_a)[i]; p = (**pp_a)[i+1];
				}
			while(m == T1);
			continue;
			}
			
	NEXT2:
		(*p_b)[ib++] = m; (*p_b)[ib++] = p;
		if((r=CheckBuffer(ib,&ibmax,&p_b)) != OK) goto SORTIR;
		i += 2L;
		}
	(*p_b)[ib++] = TEND; (*p_b)[ib++] = TEND;
	if((r=CheckBuffer(ib,&ibmax,&p_b)) != OK) goto SORTIR;

	MyDisposeHandle((Handle*) &p_index);

	// Copy b to a

	if(more > 0) {
		i = iorg;
		while((**pp_a)[i] != TEND || (**pp_a)[i+1] != TEND) i += 2L;
		j = i + more + more;
		if((r=CheckBuffer(j+2L,&imax,pp_a)) != OK) goto SORTIR;
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

	SORTIR:
	MyDisposeHandle((Handle*)&p_chunk);
	MyDisposeHandle((Handle*)&p_b);
	return(r);
	}


int StoreChunkOld(ChunkPointer ***pp_chunk,long *p_i,long *p_imax,unsigned long origin,
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


int GetChunkOld(ChunkPointer **p_chunk,long *p_ichunk,long ichunkmax,int rndseq,
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
	BPPrintMessage(0,odError,"=> Err. GetChunk(). ichunk < ZERO");
	ichunk = ZERO;
	}
if(ichunk >= ichunkmax) {
	BPPrintMessage(0,odError,"=> Err. GetChunk(). ichunk >= ichunkmax");
	ichunk = ichunkmax - 1L;
	}

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


int MakeRandomSequenceOld(long ***pp_x,long size,int repeat,int store)
{
int result,r1,r2;
long pos,x;
double randomnumber;

if(size == ZERO) return(OK);
if(*pp_x == NULL) {
	*pp_x = (long**) GiveSpace((Size) size * sizeof(long));
	if(*pp_x == NULL) return(ABORT);
	}

if((result=MakeRandomSequenceOld(pp_x,size-1L,repeat,store)) != OK) return(result);

if(!repeat) {
	r1 = bp3_rand(); r2 = bp3_rand();
	UsedRandom = TRUE;
	randomnumber = r1 + (((double)r2) / ((double)(BP3_RAND_MAX + 1L)));
	pos = (((double) size) * randomnumber) / ((double)(BP3_RAND_MAX + 1L));
	if(pos >= size) {
		BPPrintMessage(0,odError,"=> Err. MakeRandomSequence(). pos >= size");
		pos = size - 1L;
		}
	if(pos < ZERO) {
		BPPrintMessage(0,odError,"=> Err. MakeRandomSequence(). pos < ZERO");
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


int RotateSequenceOld(long ***pp_x,long size,int rotate)
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