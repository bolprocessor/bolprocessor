/* Misc.c (BP3) */

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

#if defined(_WIN64)
	void mysleep(long waitTime) {
		LARGE_INTEGER perfCnt, start, now;
		QueryPerformanceFrequency(&perfCnt);
		QueryPerformanceCounter(&start);
		do QueryPerformanceCounter((LARGE_INTEGER*)&now);
		while((now.QuadPart - start.QuadPart) / (float)(perfCnt.QuadPart) * 1000 * 1000 < waitTime);	
		}
#elif defined(__APPLE__)
    void mysleep(long waitTime) {
        usleep(waitTime);
        }
#elif defined(__linux__)
    void mysleep(long waitTime) {
        usleep(waitTime);
        }
#elif defined(__BP3_WASM__)
    void mysleep(long waitTime) {
        /* No-op in WASM — cannot block the main thread */
        (void)waitTime;
        }
#endif


int trace_FixNumberConstant = 0;

char* str_replace(const char *search, const char *replace, const char *str) {
    char *result; // The return string
    const char *insert_point; // Next insert point
    int count; // Number of replacements
    int search_len;
    int replace_len;
    const char *temp; // Corrected type
    // Step 1: Counting the number of times the old word occurs in the string
    for (count = 0, insert_point = str; (temp = strstr(insert_point, search)); ++count) {
        insert_point = temp + strlen(search);
    	}
    // Step 2: Allocating space for the new string
    search_len = strlen(search);
    replace_len = strlen(replace);
    result = (char *)malloc(strlen(str) + (replace_len - search_len) * count + 1);
    if(!result) {
        return NULL; // Returns NULL if malloc fails
    	}
    // Step 3: Replacing each occurrence of the search string with the replacement string
    insert_point = str;
    char *temp_result = result;
    while(count--) {
        char *pos = strstr(insert_point, search); // Find next occurrence of search in str
        int len = pos - insert_point; // Calculate the length up to the found string
        memcpy(temp_result, insert_point, len); // Copy that part of the original string
        temp_result += len; // Move the pointer forward
        memcpy(temp_result, replace, replace_len); // Append the replacement string
        temp_result += replace_len; // Move the pointer forward
        insert_point = pos + search_len; // Move past the old part in the original string
    	}
    strcpy(temp_result, insert_point); // Append the rest of the string
    return result;
	}

int AppendStringList(char* line)
{
char** ptr;
int i;

if(line[0] == '\0') return(OK);
NrStrings++;
if(NrStrings >= SizeStringList) {
	p_StringList = (char****) IncreaseSpace((Handle) p_StringList);
	SizeStringList = (MyGetHandleSize((Handle)p_StringList) / sizeof(char**));
	for(i=NrStrings; i < SizeStringList; i++) (*p_StringList)[i] = NULL;
	}
if((ptr=(char**)GiveSpace((Size) (strlen(line) + 1) * sizeof(char))) == NULL)
	return(ABORT);
(*p_StringList)[NrStrings-1] = ptr;
MystrcpyStringToTable(p_StringList,NrStrings-1,line);
return(OK);
}


int MemberStringList(char* line)
{
int i,r;

r = NO;
for(i=0; i < NrStrings; i++) {
	if(strcmp(line,StringList(i)) == 0) {
		r = YES; break;
		}
	}
return(r);
}


int Expect(char c,char* string,char d)
{
char line[MAXLIN];

if(isspace(d))
	my_sprintf(line,"=> Expecting '%c' after '%s'. Found a space instead.\n",
		c,string);
else
	if(isgraph(d))
		my_sprintf(line,"=> Expecting '%c' after '%s'. Found '%c' instead.\n",
			c,string,d);
	else
		my_sprintf(line,"=> Expecting '%c' after '%s'. Found '%c' (ASCII %ld) instead.\n",
			c,string,d,(long)((256L + d) % 256L));
Print(wTrace,line);
return(OK);
}

int Notify(char* message,int up) { // Doesn't work on Mac because of authorisations, although the code is correct: it works when calling bp with Terminal command
	if(strcmp(message,"") == 0) return OK;
    BPPrintMessage(1,odInfo,"👉 %s\n",message);
	int timeout = 5;
    #if defined(_WIN64)
    if(up) MessageBox(NULL, message, "Alert", MB_OK | MB_ICONINFORMATION);
    #elif defined(__APPLE__)
    char command[1024];
    // Simplified AppleScript command without extra dialog properties
    snprintf(command,sizeof(command), 
        "osascript -e 'display notification \"%s\" with title \"BP3:\"'", message);
    system(command);
	#elif defined(__linux__)
    if(up) {
        char linuxCommand[1024];
        snprintf(linuxCommand, sizeof(linuxCommand), "zenity --info --text=\"%s\" --title=\"Alert\" --timeout=%d", message, timeout);
        system(linuxCommand);
        }
    #endif
    return OK;
	}


// FIXME: GetInteger() likely originally assumed that ints were 2 bytes.
// Change to int16_t Get2ByteInteger() ?? (needs careful checking!)
int GetInteger(int test, const char* line, int* p_i)
{
long n;
int done,sign;
char c;

n = 0; done = FALSE; sign = 1;
if(test && *p_i >= strlen(line)) return(INT_MAX);
/* Sometimes reading special arrays that start with '\0' */
do {
	c = line[*p_i];
	if(!done && c == '-' && sign == 1) {
		sign = -1; (*p_i)++;
		continue;
		}
	if(c == '\0' || !isdigit(c)) break;
	n = 10L * n + (long) (c - '0'); done = TRUE;
	if(n > INT_MAX) {	// FIXME: can never be true when sizeof(long) == sizeof(int)
		my_sprintf(Message,"\n=> Maximum integer value: %ld.\n",(long)INT_MAX);
		Print(wTrace,Message);
		return(INT_MAX);
		}
	(*p_i)++;
	}
while(c != '\0');
if(!done) {
	if(!InitOn && !test) Print(wTrace,"\n=> Number missing.\n");
	return(INT_MAX);
	}
return((int) n * sign);
}


// FIXME: GetHexa() likely originally assumed that ints were 2 bytes ?
int GetHexa(const char* line, int* p_i)
{
long n;
int done,j;
char c;

n = 0; done = FALSE;
if(*p_i >= strlen(line)) return(INT_MAX);
do {
	c = line[*p_i];
	if(!done && c == '-') {
		Print(wTrace,"\n=> Hexadecimal number should not be negative");
		return(INT_MAX);
		}
	if(c == '\0') break;
	c = UpperCase(c);
	switch(c) {
		case 'A': j = 10; break;
		case 'B': j = 11; break;
		case 'C': j = 12; break;
		case 'D': j = 13; break;
		case 'E': j = 14; break;
		case 'F': j = 15; break;
		default:
			if(!isdigit(c)) goto END;
			j = c - '0';
			break;
		}
	n = 16L * n + (long) j; done = TRUE;
	if(n > INT_MAX) {	// FIXME: can never be true when sizeof(long) == sizeof(int)
		my_sprintf(Message,"\n=> Maximum integer value: %ld.\n",(long)INT_MAX);
		Print(wTrace,Message);
		return(INT_MAX);
		}
	(*p_i)++;
	}
while(c != '\0');
END:
if(!done) {
	Print(wTrace,"\n=> Number missing.\n");
	return(INT_MAX);
	}
return((int) n);
}


// FIXME: GetLong() likely originally assumed that longs are 4 bytes.
// Change to int32_t Get4ByteInteger() ?? (needs careful checking!)
long GetLong(const char* line, int* p_i)
{
long n;
int done,sign;
char c;

n = 0; done = FALSE; sign = 1;
if(*p_i >= strlen(line)) return(Infpos);
do {
	c = line[*p_i];
	if(!done && c == '-' && sign == 1) {
		sign = -1; (*p_i)++;
		continue;
		}
	c = line[*p_i];
	if(c == '\0' || !isdigit(c)) break;
	n = 10L * n + (long) (c - '0'); done = TRUE;
	if(n >= Infpos) {	// FIXME: can never be true (Infpos == LONG_MAX)
		my_sprintf(Message,"\n=> Maximum value: %ld.\n",(long)Infpos-1);
		Print(wTrace,Message);
		return(Infpos);
		}
	(*p_i)++;
	}
while(c != '\0');
if(!done) {
	Print(wTrace,"\n=> Number missing.\n");
	return(Infpos);
	}
return(n * sign);
}


// FIXME: GetUnsigned() likely originally assumed that unsigned ints were 2 bytes.
// Change to uint16_t Get2ByteUnsigned() ?? (needs careful checking!)
unsigned GetUnsigned(const char* line, int* p_i)
{
long n;
int done;
char c;

n = 0; done = FALSE;
if(*p_i >= strlen(line)) return(UINT16_MAX);	// FIXME: integer overflow (*2UL or UINT16_MAX ?)
do {
	c = line[*p_i];
	if(!done && c == '-') {
		Print(wTrace,"\n=> Unsigned integer should not be negative");
		return(UINT16_MAX);
		}
	c = line[*p_i];
	if(c == '\0' || !isdigit(c)) break;
	n = 10L * n + (long) (c - '0'); done = TRUE;
	if(n >= UINT16_MAX) {	// FIXME: use wider integers and proper max value
		my_sprintf(Message,"\nMaximum unsigned value: %ld.\n",
			(long) UINT16_MAX - 1L);	// FIXME (UINT16_MAX = INT16_MAX*2 + 1)
		Print(wTrace,Message);
		return(UINT16_MAX);
		}
	(*p_i)++;
	}
while(c != '\0');
if(!done) {
	Print(wTrace,"\n=> Number missing.\n");
	return(UINT16_MAX);
	}
return((unsigned) n);
}


// FIXME ? Use strtod() instead ?
double GetDouble(const char* line, int* p_i)
{
double n,sign;
int done,signfound;
long dec;
char c;

n = 0.; done = signfound = FALSE; sign = 1.;
dec = ZERO;
if(*p_i >= strlen(line)) return(-1.);
do {
	c = line[*p_i];
	if(c == '+') {
		if(!done && sign == 1 && !signfound) {
			(*p_i)++; signfound = TRUE; continue;
			}
		else {
			Print(wTrace,"\n=> Incorrect sign in float number.\n");
			return(-1.);
			}
		}
	if(c == '-') {
		if(!done && sign == 1 && !signfound) {
			(*p_i)++; signfound = TRUE; sign = -1.; continue;
			}
		else {
			Print(wTrace,"\n=> Incorrect sign in float number.\n");
			return(-1.);
			}
		}
	if(c == '.') {
		if(dec == ZERO) {
			dec = 10L; (*p_i)++;
			continue;
			}
		else {
			Print(wTrace,"\n=> Several decimal points in float number.\n");
			return(-1.);
			}
		}
	if(c == '\0' || !isdigit(c)) break;
	done = TRUE;
	if(dec == ZERO) n = 10.0 * n + (double) (c - '0');
	else  {
		n = n + (((double) (c - '0')) / dec);
		dec = 10 * dec;
		}
	(*p_i)++;
	}
while(c != '\0');
if(!done) {
	Print(wTrace,"\n=> Float number incorrect or missing.\n");
	return(-1.);
	}
return(n * sign);
}

int MySpace(char c)
{
if(c == '\r' || c == '\n' || c == '\0') return(NO);
if(isspace(c)) return(YES);
// -54 = 0xCA (202) which is a non-breaking space (in Mac Roman)
// '\20' = 0x10 (16) which is control-P and ???
if(c == (char) -54 || c == '\20') return(YES);	/* option space, ctrl-p */
return(NO);
}


int ByteToInt(char x)
{
int i;

i = x;
if(i < 0) i += 256;
return(i);
}


int MoveDown(tokenbyte ***pp_buff, long *p_i, long *p_k, long *p_imax)
{
long j;
j = *p_i;
while((*p_k) < (*p_imax)+2) (**pp_buff)[j++] = (**pp_buff)[(*p_k)++];
*p_imax = j - 2L;
return(OK);
}


void insert_space_between_digits(char* str) {
    char result[20] = ""; // Buffer to store the resulting string
    int i = 0, j = 0;

    while (str[i] != '\0') {
        if(str[i] == '#') {
            i++; // Move to the next character after '#'
            while (isdigit(str[i])) {
                char num[5] = ""; // To hold the current number, considering max 4 digits
                int k = 0;
                while (isdigit(str[i]) && k < 4) {
                    num[k++] = str[i++];
                }
                num[k] = '\0'; // Null-terminate the number string
                
                int num_len = strlen(num);
                if(num_len == 3) {
                    // Insert space between the last two digits
                    j += snprintf(result + j, sizeof(result) - j, "%c%c %c", num[0], num[1], num[2]);
                } else if(num_len == 4) {
                    // Insert space in the middle (between second and third digit)
                    j += snprintf(result + j, sizeof(result) - j, "%c%c %c%c", num[0], num[1], num[2], num[3]);
                } else {
                    // If it's not 3 or 4 digits, just append the number as it is
                    j += snprintf(result + j, sizeof(result) - j, "%s", num);
                }
            }
        } else {
            result[j++] = str[i++];
			}
		}
    result[j] = '\0'; // Null-terminate the result string
    // Copy the result back to the original string
    strcpy(str, result);
	}


void trim_digits_after_key_hash(char* str) {
    char result[20] = ""; // Buffer to store the resulting string
    int i = 0, j = 0;

    while (str[i] != '\0') {
        if(str[i] == '#' && i > 0 && str[i-1] == 'y' && str[i-2] == 'e' && str[i-3] == 'k') {
            result[j++] = str[i++]; // Copy the '#' character
            int digit_count = 0;
            while (isdigit(str[i]) && digit_count < 2) {
                result[j++] = str[i++];
                digit_count++;
            }
            // Skip any remaining digits after the first two
            while (isdigit(str[i])) {
                i++;
            }
        } else {
            result[j++] = str[i++];
        }
    }

    result[j] = '\0'; // Null-terminate the result string

    // Copy the result back to the original string
    strncpy(str, result, strlen(result) + 1);
	}

int ConvertSpecialChars(char* line) {
int i,j;
char c;

for(i=j=0;; i++) {
	if(line[i+j] == '\r') {
		line[i+j] = '\0';
		break;
		}
	if(line[i+j] == '^') {
		j++;
		switch(line[i+j]) {
		//	case 'n': c = '�'; break; Fixed by BB 2022-02-17  UTF8
			case 'r':
			case 'p': c = '\r'; break;
			case 't': c = '\t'; break;
			default: continue;
			}
		line[i] = c;
	/*	if(c == '�') { Fixed by BB 2022-02-17  UTF8
			line[++i] = '\r';
			j--;
			} */
		continue;
		}
	line[i] = line[i+j];
	if(line[i] == '\0') break;
	}
return(OK);
}


/* Retrieve the name of a linked file of type doc from window w.
   doc is the "document index" of the name that is being looked for.
   filename should be storage with at least MAXNAME space. */
int GetLinkedFileName(int w, int doc, char* filename)
{
long pos,posmax;
char *p,*q,line[MAXLIN];

if(filename == NULL) {
	BPPrintMessage(0,odError,"=> Err. GetLinkedFileName(). filename == NULL.");
	return(MISSED);
	}
if(w < 0 || w >= WMAX || !Editable[w]) {
	BPPrintMessage(0,odError,"=> Err. GetLinkedFileName(). Bad window index %d\n",w);;
	return(MISSED);
	}
if(doc < 0 || doc >= WMAX || FilePrefix[doc][0] == '\0') {
	BPPrintMessage(0,odError,"=> Err. GetLinkedFileName(). Bad document index.");
	return(MISSED);
	}
pos = ZERO;
posmax = GetTextLength(w);
do {
	if(ReadLine1(FALSE,w,&pos,posmax,line,MAXLIN) != OK) return(MISSED);
	if(line[0] == '\0') continue;
	if(line[0] == '-' && line[1] == '-') return(MISSED);
	adjust_prefix(line);
	p = line; q = FilePrefix[doc];
	if(Match(TRUE,&p,&q,4) && line[4] != '<' && line[4] != '\334') {
		Strip(line); // does this make sense?
		if(line[4] == ':')    // real filename does not begin with prefix
			p = &(line[5]);  // so, skip the prefix in this line
		else  p = line;
		if(strlen(p) > MAXNAME) continue;
		strcpy(filename,p);
		return(OK);
		}
	}
while(TRUE);
}

int GetAlphaName(int w)
{
if(GetLinkedFileName(w,wAlphabet,FileName[wAlphabet]) == OK) {
		NoAlphabet = FALSE;
		return(OK);
		}
else return(MISSED);
}

int GetMiName(void)
{
char name[MAXNAME];

if(GetLinkedFileName(wAlphabet,iObjects,name) == OK) {
	if(strcmp(FileName[iObjects],name) != 0) {
		strcpy(FileName[iObjects],name);
		ObjectMode = ObjectTry = FALSE;
		}
	return(OK);
	}
else return(MISSED);
}

int GetInName(int w)
{
char name[MAXNAME];

if(GetLinkedFileName(w,wInteraction,name) == OK) {
	if(strcmp(FileName[wInteraction],name) != 0) {
		strcpy(FileName[wInteraction],name);
		LoadedIn = CompiledIn = FALSE;
		}
	return(OK);
	}
else return(MISSED);
}


int GetSeName(int w)
{
char name[MAXNAME];

if(GetLinkedFileName(w,iSettings,name) == OK) {
	if(strcmp(FileName[iSettings],name) != 0) {
		strcpy(FileName[iSettings],name);
		Created[iSettings] = FALSE;
		return(OK);
		}
	else return(MISSED); 	/* Name is unchanged: no need to reload */
	}
else return(MISSED);
}

#if BP_CARBON_GUI_FORGET_THIS

int GetKbName(int w)
{
int type,result;
FSSpec spec;
short refnum;
char name[MAXNAME];

result = MISSED;
if(GetLinkedFileName(w,wKeyboard,name) == OK) {
	if(strcmp(FileName[wKeyboard],name) != 0) {
		strcpy(FileName[wKeyboard],name);
		if(Token == FALSE && !ScriptExecOn) {
			if(Answer("=> Alphabet file indicated keyboard encoding.\nType tokens instead of normal text",
				'N') == OK) Token = TRUE;
			}
		type = gFileType[wKeyboard];
		c2pstrcpy(spec.name, name);
		spec.vRefNum = TheVRefNum[wKeyboard];
		spec.parID = WindowParID[wKeyboard];
		if(MyOpen(&spec,fsCurPerm,&refnum) == noErr ||
		    CheckFileName(wKeyboard,FileName[wKeyboard],&spec,&refnum,type,TRUE) == OK) {
			result = LoadKeyboard(refnum);
			}
		}
	else {
		if(Token && FileName[wKeyboard][0] == '\0') {
			if(!ScriptExecOn) BPPrintMessage(0,odError,"=> You can't use tokens ('Misc' menu) unless you define '-kb.' file in alphabet");
			Token = FALSE;
			result = ABORT;
			}
		}
}
if(Token && LoadOn && FileName[wKeyboard][0] == '\0') {
	Token = FALSE; MaintainMenus();
	}
return(result);
}

int GetFileNameAndLoadIt(int wfile,int w,Int2ProcPtr loadit)
{
int r,type;
FSSpec spec;
short refnum;
char name[MAXNAME];

if(wfile < 0 || wfile >= WMAX) {
	BPPrintMessage(0,odError,"=> Err. GetFileNameAndLoadIt().(wfile < 0 || wfile >= WMAX");
	return(MISSED);
	}

if(GetLinkedFileName(w,wfile,name) == OK) {
		if(strcmp(FileName[wfile],name) != 0) {
			strcpy(FileName[wfile],name);
			type = gFileType[wfile];
			c2pstrcpy(spec.name, name);
			spec.vRefNum = TheVRefNum[wfile];
			spec.parID = WindowParID[wfile];
			if(MyOpen(&spec,fsCurPerm,&refnum) != noErr) {
				if(CheckFileName(wfile,FileName[wfile],&spec,&refnum,type,TRUE)
					!= OK) return(ABORT);
				}
			r = ((int (*)(short,int)) loadit)(refnum,FALSE);
			if(r == OK) SetName(wfile,TRUE,FALSE);
			return(r);
			}
		else return(MISSED); // is this correct? -- akozar
		}
else return(MISSED);
}

int GetCsName(int w)
{
int r,type;
FSSpec spec;
short refnum;
char name[MAXNAME];

if(GetLinkedFileName(w,wCsoundResources,name) == OK) {
	if(strcmp(FileName[wCsoundResources],name) != 0) {
		strcpy(FileName[wCsoundResources],name);
		type = gFileType[wCsoundResources];
		c2pstrcpy(spec.name, name);
		spec.vRefNum = TheVRefNum[wCsoundResources];
		spec.parID = WindowParID[wCsoundResources];
		if(MyOpen(&spec,fsCurPerm,&refnum) != noErr) {
			if(CheckFileName(wCsoundResources,FileName[wCsoundResources],&spec,&refnum,type,TRUE)
				!= OK) return(MISSED);
			}
		r = LoadCsoundInstruments(refnum,FALSE);
		if(r == OK) SetName(wCsoundResources,TRUE,FALSE);
		return(r);
		}
	else return(MISSED); // is this right? -- akozar
	}
else return(MISSED);
}

int GetTimeBaseName(int w)
{
int type;
FSSpec spec;
short refnum;
char name[MAXNAME];

if(GetLinkedFileName(w,wTimeBase,name) == OK) {
	if(strcmp(FileName[wTimeBase],name) != 0) {
		strcpy(FileName[wTimeBase],name);
		type = gFileType[wTimeBase];
		c2pstrcpy(spec.name, name);
		spec.vRefNum = TheVRefNum[wTimeBase];
		spec.parID = WindowParID[wTimeBase];
		if(MyOpen(&spec,fsCurPerm,&refnum) != noErr) {
			if(CheckFileName(wTimeBase,FileName[wTimeBase],&spec,&refnum,type,TRUE)
				!= OK) return(MISSED);
			}
		LoadTimeBase(refnum);
		return(OK);
		}
	else return(MISSED);  // is this correct? -- akozar
	}
else return(MISSED);
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

int ChangeMetronom(int j,double x)
{
int striated;
double p,q,newp,newq;

striated = (Nature_of_time == STRIATED);
newp = Pclock; newq = Qclock;

switch(j) {
	case 13:
		MakeRatio((double)ULONG_MAX,x,&p,&q);
		if(p < 0.) {
			my_sprintf(Message,"=> Metronome cannot be set to negative value. '%.4f' not accepted",
				x);
			BPPrintMessage(0,odError,"%s",Message);
			return(ABORT);
			}
		if(p == ZERO) {
			newp = 0.; newq = 1.;
			}
		else {
			if(Simplify((double)INT_MAX,p,60.*q,&newq,&newp) != OK) 
				Simplify((double)INT_MAX,floor(p/60.),q,&newq,&newp);
			}
		break;
	case 14:
		if(Pclock < 1. && !NotFoundMetronom) {
			BPPrintMessage(0,odError,"=> Setting time to 'striated' is inconsistent with having no clock");
			striated = FALSE;
			goto MAKECHANGE;
			}
		striated = TRUE;
		break;
	case 15:
		striated = FALSE;
		break;
	}
if((striated && Nature_of_time != STRIATED)
		|| (!striated && Nature_of_time == STRIATED)
		|| newp != Pclock || newq != Qclock) {
MAKECHANGE:
	if(newp != Pclock || newq != Qclock) BPPrintMessage(0,odInfo,"Metronome has been set to %.3f beats/mn while reading grammar/data\n",(newq * 60.)/newp); 
	Pclock = newp;
	Qclock = newq;
	if(striated) {
		if(Nature_of_time == SMOOTH) BPPrintMessage(0,odInfo,"Time has been set to STRIATED while reading grammar/data\n");
		Nature_of_time = STRIATED;
		}
	else {
		if(Nature_of_time == STRIATED) BPPrintMessage(0,odInfo,"Time has been set to SMOOTH while reading grammar/data\n");
		Nature_of_time = SMOOTH;
		}
	SetTempo();
	}
return(OK);
}


int SetTempo(void)
{
Rect r;
// ControlHandle itemhandle;
short itemtype;
int speed_change;
double p,q;
char line[MAXFIELDCONTENT];
double speedratio;

if(PedalOrigin != -1) {
	speed_change = PedalPosition - PedalOrigin;
	speedratio = exp((double)((double) speed_change * Ke));
	if(speedratio > 20.) speedratio = 20.;
	if(speedratio < 0.05) speedratio = 0.05;
	Nalpha = (long) (speedratio * 100L);
	Simplify((double)INT_MAX,100. * OldPclock,Nalpha * OldQclock,&Pclock,&Qclock);
	Nalpha = 100L;
	}
if(Pclock == 0.) {
	strcpy(line,"[no clock]");
	Nature_of_time = SMOOTH;
	}
else {
	if(Simplify((double)INT_MAX,(double)60L*Qclock,Pclock,&p,&q) != OK)
		Simplify((double)INT_MAX,Qclock,floor((double)Pclock/60.),&p,&q);
	my_sprintf(line,"%.4f", ((double)p)/q);
	}

#if BP_CARBON_GUI_FORGET_THIS
SetField(NULL, wMetronom, fTempo, line);

/* Set smooth or striated */
if(Nature_of_time == STRIATED) {
	GetDialogItem(gpDialogs[wMetronom],dSmooth,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,0);
	GetDialogItem(gpDialogs[wMetronom],dStriated,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,1);
	}
else {
	GetDialogItem(gpDialogs[wMetronom],dSmooth,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,1);
	GetDialogItem(gpDialogs[wMetronom],dStriated,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,0);
	}
#endif /* BP_CARBON_GUI_FORGET_THIS */
return(OK);
}

int SetGrammarTempo(void)
// Here we only erase the line containing "_mm()" and tell BP3 that the grammar is not compiled
// so that a fresh line is  inserted during the compilation
{
int w,j,gap,rep;
char *q,**p_line,line1[MAXLIN],line2[MAXLIN],line3[MAXLIN];
long pos,posline,posmax;

if(IsEmpty(wGrammar)) return(OK);
w = wGrammar;
posmax = GetTextLength(w);
p_line = NULL;
pos = posline = ZERO;
rep = OK;
MystrcpyHandleToString(MAXLIN,0,line1,(*p_GramProcedure)[13]);	/* _mm() */
MystrcpyHandleToString(MAXLIN,0,line2,(*p_GramProcedure)[14]);	/* _striated */
MystrcpyHandleToString(MAXLIN,0,line3,(*p_GramProcedure)[15]);	/* _smooth() */
while(ReadLine(YES,w,&pos,posmax,&p_line,&gap) == OK) {
	if((*p_line)[0] == '\0' || (*p_line)[0] == '\r') continue;
	for(j=0; j < WMAX; j++) {
		if(FilePrefix[j][0] == '\0') continue;
		q = &(FilePrefix[j][0]);
		if(Match(TRUE,p_line,&q,4)) goto NEXTLINE;
		}
	if(Mystrcmp(p_line,"DATA:") == 0) break;
	if(Mystrcmp(p_line,"COMMENT:") == 0) break;
	if(Mystrcmp(p_line,"TIMEPATTERNS:") == 0) {
		do {
			if(ReadLine(YES,wGrammar,&pos,posmax,&p_line,&gap) != OK) goto END;
			if((*p_line)[0] == '\0') continue;
			}
		while((*p_line)[0] != '-' || (*p_line)[1] != '-');
		goto NEXTLINE;
		}
	q = GRAMstring;
	if(Match(FALSE,p_line,&q,strlen(GRAMstring))) break;
	MyLock(TRUE,(Handle)p_line);
	if(strstr(*p_line,line1) != NULLSTR) {	/* _mm() */
FOUNDIT:
		SelectBehind(posline,pos,TEH[w]);
		TextDelete(w);
		CompiledGr = FALSE;
		MyUnlock((Handle)p_line);
		break;
		}
	if(strstr(*p_line,line2) != NULLSTR) goto FOUNDIT;
	if(strstr(*p_line,line3) != NULLSTR) goto FOUNDIT;
	MyUnlock((Handle)p_line);
NEXTLINE:
	posline = pos;
	}
	
END:
MyDisposeHandle((Handle*)&p_line);
return(OK);
}


/* Pause() and GetCap() are used to get user input while stepping thru timesetting.
   Will need to provide an alternative for user-interactivity OR create a "stepping
   API" for the timesetting code if we want to retain step functionality. */

// NOTE: the return value of this function is not checked in several places ...
int Pause(void)
{
char c;

c = 0;
StopWait();
#if BP_CARBON_GUI_FORGET_THIS
switch(Alert(StepAlert,0L)) {
	case aContinue: return(' ');
	case aStep: return('S');
	case aUndo: return('U');
	case aAbort: return('Q');
	}
#else
// FIXME: do we need a console equivalent of this function ??
#endif
return(' ');
}



int Date(char line[])
{
unsigned long datetime;
char dd[MAXNAME],tt[MAXNAME];

time_t curtime;
struct tm *loctime;

time(&curtime);
loctime = localtime(&curtime);
// format the date
if(strftime(dd, MAXNAME, "%a, %b %d, %Y", loctime) == 0) {
	 BPPrintMessage(0,odError,"=> Err. Date(): strftime() results were too long for dd.");
	dd[0] = '\0';
	}
// format the time
if(strftime(tt, MAXNAME, "%I:%M %p", loctime) == 0) {
	 BPPrintMessage(0,odError,"=> Err. Date(): strftime() results were too long for tt.");
	tt[0] = '\0';
	}

my_sprintf(line,"%s %s -- %s",DateMark,dd,tt);
return(OK);
}


int FixStringConstant(char* line)
{
int i,j,maxparam,found;
Handle h;
char ****pp_h,**ptr;

Strip(line);
if(line[0] == '\0') goto ERR;

found = FALSE;

if(p_StringConstant == NULL) maxparam = 0;
else maxparam = (MyGetHandleSize((Handle)p_StringConstant) / sizeof(char**));

if(maxparam == 0) {
	if((pp_h = (char****) GiveSpace((Size)(10) * sizeof(char**))) == NULL) return(ABORT);
	p_StringConstant = pp_h;
	maxparam = 10;
	if((ptr = (char**) GiveSpace((Size)(strlen("pitchbend")+1))) == NULL) return(ABORT);
	(*p_StringConstant)[IPITCHBEND] = ptr;
	MystrcpyStringToTable(p_StringConstant,IPITCHBEND,"pitchbend");
	if((ptr = (char**) GiveSpace((Size)(strlen("volume")+1))) == NULL) return(ABORT);
	(*p_StringConstant)[IVOLUME] = ptr;
	MystrcpyStringToTable(p_StringConstant,IVOLUME,"volume");
	if((ptr = (char**) GiveSpace((Size)(strlen("modulation")+1))) == NULL) return(ABORT);
	(*p_StringConstant)[IMODULATION] = ptr;
	MystrcpyStringToTable(p_StringConstant,IMODULATION,"modulation");
	if((ptr = (char**) GiveSpace((Size)(strlen("pressure")+1))) == NULL) return(ABORT);
	(*p_StringConstant)[IPRESSURE] = ptr;
	MystrcpyStringToTable(p_StringConstant,IPRESSURE,"pressure");
	if((ptr = (char**) GiveSpace((Size)(strlen("panoramic")+1))) == NULL) return(ABORT);
	(*p_StringConstant)[IPANORAMIC] = ptr;
	MystrcpyStringToTable(p_StringConstant,IPANORAMIC,"panoramic");
	
	for(i=5; i < maxparam; i++) (*p_StringConstant)[i] = NULL;
	}

for(j=0; j < maxparam; j++) {
	if((*p_StringConstant)[j] == NULL) break;
	if(Mystrcmp((*p_StringConstant)[j],line) == 0) {
		found = TRUE;
		break;
		}
	}
if(found) return(j);

if(j >= MAXSTRINGCONSTANTS) {
	my_sprintf(Message,
			"Too many identifiers found (max %ld)\nCan't store '%s'\n",
				(long)MAXSTRINGCONSTANTS,line);
	BPPrintMessage(0,odError,"%s",Message);
	return(ABORT);
	}

if(j >= maxparam) {
	h = (Handle) p_StringConstant;
	if((h = IncreaseSpace(h)) == NULL) return(ABORT);
	p_StringConstant = (char****) h;
	maxparam = (MyGetHandleSize((Handle)p_StringConstant) / sizeof(char**));
	for(i=j; i < maxparam; i++) (*p_StringConstant)[i] = NULL;
	}
	
if((ptr = (char**) GiveSpace((Size)(strlen(line)+1))) == NULL) return(ABORT);
(*p_StringConstant)[j] = ptr;
MystrcpyStringToTable(p_StringConstant,j,line);
if(trace_scale) BPPrintMessage(0,odInfo, "FixStringConstant() j = %d, line = %s\n",j,line);
return(j);

ERR:
BPPrintMessage(0,odError,"Missing parameter name");
return(ABORT);
}


int FixNumberConstant(char* line)
{
int i,j,maxparam,oldmaxparam;
Handle h;
double x;
long p,q;

Strip(line);
if(line[0] == '\0') goto ERR;

x = Myatof(line,&p,&q);
// if(trace_scale) BPPrintMessage(0,odInfo,"FixNumberConstant line = %s x = %.3f\n",line,x);

if(p_NumberConstant == NULL) maxparam = 0;
else maxparam = (MyGetHandleSize((Handle)p_NumberConstant) / sizeof(double));

if(maxparam == 0) {
	maxparam = 10;
	if((p_NumberConstant=(double**) GiveSpace((Size)(maxparam) * sizeof(double))) == NULL)
		return(ABORT);
	for(i=0; i < maxparam; i++) (*p_NumberConstant)[i] = Infpos;
	}
else {
	}

for(j = 1; j < maxparam; j++) {
	if((*p_NumberConstant)[j] == x) break;
	if((*p_NumberConstant)[j] == Infpos) {
		(*p_NumberConstant)[j] = x;
		break;
		}
	}
if(j < maxparam) {
	if(trace_FixNumberConstant || trace_scale) BPPrintMessage(0,odInfo,"FixNumberConstant() line = %s j = %ld, x = %.3f\n",line,(long)j,x);
	return(j);
	}
if(j >= MAXSTRINGCONSTANTS) {
	my_sprintf(Message,
		"=> Too many numeric constants found (max %ld)\nCan't store '%s'\n",(long)MAXSTRINGCONSTANTS,line);
	BPPrintMessage(0,odError,"%s",Message);
	return(ABORT);
	}
	
h = (Handle) p_NumberConstant;
if((h = IncreaseSpace(h)) == NULL) return(ABORT);
p_NumberConstant = (double**) h;
oldmaxparam = maxparam;
maxparam = (MyGetHandleSize((Handle)p_NumberConstant) / sizeof(double));
for(i = oldmaxparam + 1; i < maxparam; i++) (*p_NumberConstant)[i] = Infpos;
		
(*p_NumberConstant)[j] = x;
if(trace_FixNumberConstant) BPPrintMessage(0,odInfo,"FixNumberConstant() after increasing space line = '%s' j = %ld, x = %.3f\n",line,(long)j,x);
return(j);

ERR:
BPPrintMessage(0,odError,"Missing value");
return(ABORT);
}

int WaitABit(long thedelay) {
	// Wait for thedelay milliseconds
	int result;
	if(thedelay > 100L && (result = stop(1,"WaitABit")) != OK) return result;
	mysleep(1000L * thedelay);
	return(OK);
	}


int NeedGlossary(tokenbyte ***pp_X)
{
int  i;
tokenbyte m,p;

for(i=0; ((m=(**pp_X)[i]) != TEND) || ((**pp_X)[i+1] != TEND); i+=2) {
	if(m != T4) continue;
	p = (**pp_X)[i+1];
	if(p > Jvar || p_VarStatus == NULL) {
		BPPrintMessage(0,odError,"=> Err. NeedGlossary(). p > Jvar || p_VarStatus == NULL\n");
		return(NO);
		}
	if((*p_VarStatus)[p] & 4) return(YES);
	}
return(NO);
}


// ------------------------  Random numbers -------------------------

int ResetRandom(void) {
	if(Seed > 0) {
		srand(Seed);
	//	BPPrintMessage(0,odInfo, "Random seed reset to %u\n", Seed);
		UsedRandom = FALSE;
		}
	else Randomize();
	AppendScript(55);
	return(OK);
	}


int Randomize(void) {
	if(Seed > 0) return(OK);
	ReseedOrShuffle(NEWSEED);
	my_sprintf(Message,"%.0f",(double)Seed);
	MystrcpyStringToTable(ScriptLine.arg,0,Message);
	AppendScript(57);
	return(OK);
	}


int ReseedOrShuffle(int what) {
	unsigned int seed;
	int randomnumber;

	switch(what) {
		case NOSEED:
			Seed = 0L;
			break;
		case NEWSEED:
			if(Seed == 0) {
				seed = (unsigned int) time(NULL);
				srand(seed);
				}
			randomnumber = rand();
			seed = (unsigned int) (randomnumber % 32768);
			if(seed == 0) seed = 1;
			Seed = seed;
			if(Seed > 0) {
				BPPrintMessage(1,odInfo, "New random seed = %u\n", seed);
				srand(Seed);
				UsedRandom = FALSE;
				}
			break;
		case RANDOMIZE:
			if(Seed == 0) {
				// We need this initial srand() so that sequences of rand() are not identical
				seed = (unsigned int) time(NULL);
				srand(seed);
				// FIXME ? Why seed a second time (with a restricted range for the seed too) ?
				randomnumber = rand();
				seed = (unsigned int) (randomnumber % 32768);
				BPPrintMessage(1,odInfo, "Random seed = %u\n", seed);
				srand(seed);
				UsedRandom = TRUE;
				}
			break;
		default:
			seed = (unsigned int) ((Seed + what) % 32768);
			srand(seed);
			UsedRandom = TRUE;
			break;
		}
	return(OK);
	}


double GetScalingValue(tokenbyte **p_a,unsigned long i) {
	tokenbyte m,p;
	double value;

	m = (*p_a)[i+3L];
	p = (*p_a)[i+5L];
	if(m < 0 || p < 0) {
		BPPrintMessage(0,odError,"=> Err. GetScalingValue(). m < 0 || p < 0");
		return(1.);
		}
	value = ((double)TOKBASE * m) + p;
	return(value);
	}
