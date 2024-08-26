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


#ifndef _H_BP2
#include "-BP2.h"
#endif

#include "-BP2decl.h"

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
    BPPrintMessage(odError,"👉 %s\n",message); // We use 'odError' so that it displays even in Improvize mode 
	int timeout = 5;
    #if defined(_WIN32) || defined(_WIN64)
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
        if (str[i] == '#') {
            i++; // Move to the next character after '#'
            while (isdigit(str[i])) {
                char num[5] = ""; // To hold the current number, considering max 4 digits
                int k = 0;
                while (isdigit(str[i]) && k < 4) {
                    num[k++] = str[i++];
                }
                num[k] = '\0'; // Null-terminate the number string
                
                int num_len = strlen(num);
                if (num_len == 3) {
                    // Insert space between the last two digits
                    j += snprintf(result + j, sizeof(result) - j, "%c%c %c", num[0], num[1], num[2]);
                } else if (num_len == 4) {
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
        if (str[i] == '#' && i > 0 && str[i-1] == 'y' && str[i-2] == 'e' && str[i-3] == 'k') {
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

#if BP_CARBON_GUI_FORGET_THIS

int GetThisTick(void)
{
char line[MAXFIELDCONTENT];
long p,q,s,v,c,k;
short itemtype;
int jj;
Handle itemhandle;
Rect r;

s = (long) GetCtrlValue(wTickDialog,dTickOn);
if(GetCtrlValue(wTickDialog,dSpecialTick)) {
	GetField(NULL,TRUE,wTickDialog,fThisTickVelocity,line,&p,&q);
	v =  p/q;
	if(v < 0 || v > 127) {
		my_sprintf(line,"=> Velocity range is 0..127\nCan't accept %ld",(long)v);
		Alert1(line);
		SetField(NULL,wTickDialog,fThisTickVelocity,"[?]");
		SelectField(NULL,wTickDialog,fThisTickVelocity,TRUE);
		return(MISSED);
		}
	GetField(NULL,TRUE,wTickDialog,fThisTickChannel,line,&p,&q);
	c =  p/q;
	if(c < 1 || c > 16) {
		my_sprintf(line,"=> Channel range is 1..16\nCan't accept %ld",(long)c);
		Alert1(line);
		SetField(NULL,wTickDialog,fThisTickChannel,"[?]");
		SelectField(NULL,wTickDialog,fThisTickChannel,TRUE);
		return(MISSED);
		}
	GetField(NULL,TRUE,wTickDialog,fThisTickKey,line,&p,&q);
	k =  p/q;
	if(k < 0 || k > 127) {
		my_sprintf(line,"=> Key range is 0..127\nCan't accept %ld",(long)k);
		Alert1(line);
		SetField(NULL,wTickDialog,fThisTickKey,"[?]");
		SelectField(NULL,wTickDialog,fThisTickKey,TRUE);
		return(MISSED);
		}
	ThisTick[iTick][jTick] = s + 2L * (v + 128L * ((c - 1L) + 128L * k));
	}
else ThisTick[iTick][jTick] = s;
jj = dPlayBeat + 55*iTick + jTick;
GetDialogItem(gpDialogs[wTimeBase],(short)jj,&itemtype,&itemhandle,&r);
/* HiliteControl((ControlHandle) itemhandle,11); */
if(s) {
	SwitchOn(NULL,wTimeBase,jj);
	TickThere = TRUE;
	}
else SwitchOff(NULL,wTimeBase,jj);
return(OK);
}


int SetThisTick(void)
{
char line[MAXFIELDCONTENT];
int vel,ch,key;

if(iTick < 0 || jTick < 0) {
	if(Beta) Alert1("=> Err. SetThisTick()");
	return(MISSED);
	}
if(ThisTick[iTick][jTick] % 2L) {
	SwitchOn(NULL,wTickDialog,dTickOn);
	SwitchOff(NULL,wTickDialog,dTickOff);
	}
else {
	SwitchOn(NULL,wTickDialog,dTickOff);
	SwitchOff(NULL,wTickDialog,dTickOn);
	}
if(ThisTick[iTick][jTick] > 1) {
	SwitchOn(NULL,wTickDialog,dSpecialTick);
	SwitchOff(NULL,wTickDialog,dDefaultTick);
	ShowDialogItem(gpDialogs[wTickDialog],fThisTickVelocity);
	ShowDialogItem(gpDialogs[wTickDialog],fThisTickChannel);
	ShowDialogItem(gpDialogs[wTickDialog],fThisTickKey);
	}
else {
	SwitchOn(NULL,wTickDialog,dDefaultTick);
	SwitchOff(NULL,wTickDialog,dSpecialTick);
	HideDialogItem(gpDialogs[wTickDialog],fThisTickVelocity);
	HideDialogItem(gpDialogs[wTickDialog],fThisTickChannel);
	HideDialogItem(gpDialogs[wTickDialog],fThisTickKey);
	}
vel = TickVelocity[iTick];
ch = TickChannel[iTick];
key = TickKey[iTick];
FindTickValues(ThisTick[iTick][jTick],&vel,&ch,&key);
my_sprintf(line,"%ld",(long)vel);
SetField(NULL,wTickDialog,fThisTickVelocity,line);
my_sprintf(line,"%ld",(long)ch);
SetField(NULL,wTickDialog,fThisTickChannel,line);
my_sprintf(line,"%ld",(long)key);
SetField(NULL,wTickDialog,fThisTickKey,line);
return(OK);
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

/* Retrieve the name of a linked file of type doc from window w.
   doc is the "document index" of the name that is being looked for.
   filename should be storage with at least MAXNAME space. */
int GetLinkedFileName(int w, int doc, char* filename)
{
long pos,posmax;
char *p,*q,line[MAXLIN];

if (filename == NULL) {
	if(Beta) Alert1("=> Err. GetLinkedFileName(). filename == NULL.");
	return(MISSED);
	}
if(w < 0 || w >= WMAX || !Editable[w]) {
	if(Beta) BPPrintMessage(odError,"=> Err. GetLinkedFileName(). Bad window index %d\n",w);;
	return(MISSED);
	}
if(doc < 0 || doc >= WMAX || FilePrefix[doc][0] == '\0') {
	if(Beta) Alert1("=> Err. GetLinkedFileName(). Bad document index.");
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
		if (line[4] == ':')    // real filename does not begin with prefix
			p = &(line[5]);  // so, skip the prefix in this line
		else  p = line;
		if (strlen(p) > MAXNAME) continue;
		strcpy(filename,p);
		return(OK);
		}
	}
while(TRUE);
}

int GetAlphaName(int w)
{
if (GetLinkedFileName(w,wAlphabet,FileName[wAlphabet]) == OK) {
		NoAlphabet = FALSE;
		return(OK);
		}
else return(MISSED);
}

int GetMiName(void)
{
char name[MAXNAME];

if (GetLinkedFileName(wAlphabet,iObjects,name) == OK) {
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

if (GetLinkedFileName(w,wInteraction,name) == OK) {
	if(strcmp(FileName[wInteraction],name) != 0) {
		strcpy(FileName[wInteraction],name);
		LoadedIn = CompiledIn = FALSE;
		}
	return(OK);
	}
else return(MISSED);
}

int GetGlName(int w)
{
char name[MAXNAME];

if (GetLinkedFileName(w,wGlossary,name) == OK) {
	if(strcmp(FileName[wGlossary],name) != 0) {
		strcpy(FileName[wGlossary],name);
		LoadedGl = CompiledGl = FALSE;
		}
	return(OK);
	}
else return(MISSED);
}

int GetSeName(int w)
{
char name[MAXNAME];

if (GetLinkedFileName(w,iSettings,name) == OK) {
	if(strcmp(FileName[iSettings],name) != 0 || Dirty[iSettings]) {
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
if (GetLinkedFileName(w,wKeyboard,name) == OK) {
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
		if (MyOpen(&spec,fsCurPerm,&refnum) == noErr ||
		    CheckFileName(wKeyboard,FileName[wKeyboard],&spec,&refnum,type,TRUE) == OK) {
			result = LoadKeyboard(refnum);
			}
		}
	else {
		if(Token && FileName[wKeyboard][0] == '\0') {
			if(!ScriptExecOn) Alert1("=> You can't use tokens ('Misc' menu) unless you define '-kb.' file in alphabet");
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
	if(Beta) Alert1("=> Err. GetFileNameAndLoadIt().(wfile < 0 || wfile >= WMAX");
	return(MISSED);
	}

if (GetLinkedFileName(w,wfile,name) == OK) {
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

if (GetLinkedFileName(w,wCsoundResources,name) == OK) {
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

if (GetLinkedFileName(w,wTimeBase,name) == OK) {
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
			Alert1(Message);
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
			Alert1("=> Setting time to 'striated' is inconsistent with having no clock");
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
	if(newp != Pclock || newq != Qclock) BPPrintMessage(odInfo,"Metronome has been set to %.3f beats/mn while reading grammar/data\n",(newq * 60.)/newp); 
	Pclock = newp;
	Qclock = newq;
	if(striated) {
		if(Nature_of_time == SMOOTH) BPPrintMessage(odInfo,"Time has been set to STRIATED while reading grammar/data\n");
		Nature_of_time = STRIATED;
		}
	else {
		if(Nature_of_time == STRIATED) BPPrintMessage(odInfo,"Time has been set to SMOOTH while reading grammar/data\n");
		Nature_of_time = SMOOTH;
		}
	SetTempo();
	SetTimeBase();
#if BP_CARBON_GUI_FORGET_THIS
	ShowWindow(Window[wMetronom]);
	BringToFront(Window[wMetronom]);
	UpdateDirty(TRUE,iSettings);
#endif /* BP_CARBON_GUI_FORGET_THIS */
	SetTickParameters(0,MAXBEATS);
	ResetTickFlag = TRUE;
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

#if BP_CARBON_GUI_FORGET_THIS

int GetTempo(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char s[255];
Str255 t;
unsigned long p,q;
double oldp,oldq;

if(!Dirty[wMetronom]) return(OK);
GetDialogItem(gpDialogs[wMetronom],fTempo,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,s);
Dirty[wMetronom] = FALSE;
oldp = Pclock; oldq = Qclock;
if(FloatToNiceRatio(s,&p,&q) != OK) return(MISSED);
if(p == ZERO) {
	Pclock = ZERO; Qclock = 1L;
	SetTempo();
	SetTimeBase();
	return(OK);
	}
if(Simplify((double)INT_MAX,(double)p,(double)60L*q,&Qclock,&Pclock) != OK)
	Simplify((double)INT_MAX,floor((double)p/60L),(double)q,&Qclock,&Pclock);
if(oldp != Pclock || oldq != Qclock) {
	SetTickParameters(0,MAXBEATS);
	ResetTickFlag = TRUE;
	}
SetTempo();
SetTimeBase();
SetGrammarTempo();
return(OK);
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

int SetGrammarTempo(void)
// Here we only erase the line containing "_mm()" and tell BP2 that the grammar is not compiled
// so that a fresh line is  inserted during the compilation
{
int w,j,gap,rep;
char *q,**p_line,line1[MAXLIN],line2[MAXLIN],line3[MAXLIN];
long pos,posline,posmax;

if(IsEmpty(wGrammar) || !Dirty[wMetronom]) return(OK);
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

#if BP_CARBON_GUI_FORGET_THIS

int SetBufferSize(void)
{
Rect r;
ControlHandle itemhandle;
short itemtype;
char line[MAXFIELDCONTENT];

my_sprintf(line,"%ld",(long)BufferSize / 2L - 1L);
SetField(NULL, wBufferSize, fBufferSize, line);
my_sprintf(line,"%ld",(long)DeftBufferSize / 2L - 1L);
SetField(NULL, wBufferSize, fDeftBufferSize, line);
if(UseBufferLimit) {
	GetDialogItem(gpDialogs[wBufferSize],dNoSizeLimit,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,0);
	GetDialogItem(gpDialogs[wBufferSize],dYesSizeLimit,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,1);
	}
else {
	GetDialogItem(gpDialogs[wBufferSize],dNoSizeLimit,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,1);
	GetDialogItem(gpDialogs[wBufferSize],dYesSizeLimit,&itemtype,(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,0);
	}
return(OK);
}


int GetBufferSize(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char s[MAXFIELDCONTENT];
Str255 t;
long x;

if(!Dirty[wBufferSize]) return(OK);
InputOn++;
GetDialogItem(gpDialogs[wBufferSize],fBufferSize,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,s);
x = atol(s);
if(x < 2L) {
	Alert1("=> Minimum initial buffer size: 2 symbols");
	x = 2L;
	}
if(x > 100000L) {
	Alert1("=> Maximum initial buffer size: 100,000 symbols. (It may be expanded during computation)");
	x = 100000L;
	}
BufferSize = 2L * (x + 1L);
GetDialogItem(gpDialogs[wBufferSize],fDeftBufferSize,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,s);
x = atol(s);
if(x < 2L) {
	Alert1("=> Minimum initial buffer size: 2 symbols");
	x = 2L;
	}
if(x > 100000L) {
	Alert1("=> Maximum initial buffer size: 100,000 symbols. (It may be expanded during computation)");
	x = 100000L;
	}
DeftBufferSize = 2L * (x + 1L);
UpdateDirty(TRUE,iSettings);
InputOn--;
return(OK);
}


int SetGraphicSettings(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char line[MAXFIELDCONTENT];

WriteFloatToLine(line,(double) (GraphicScaleQ * 5.) / (double) GraphicScaleP);
SetField(NULL, wGraphicSettings, fGraphicScale, line);
if(StartFromOne) {
	GetDialogItem(gpDialogs[wGraphicSettings],dZero,&itemtype,&itemhandle,&r);
	SetControlValue((ControlHandle) itemhandle,0);
	GetDialogItem(gpDialogs[wGraphicSettings],dOne,&itemtype,&itemhandle,&r);
	SetControlValue((ControlHandle) itemhandle,1);
	}
else {
	GetDialogItem(gpDialogs[wGraphicSettings],dZero,&itemtype,&itemhandle,&r);
	SetControlValue((ControlHandle) itemhandle,1);
	GetDialogItem(gpDialogs[wGraphicSettings],dOne,&itemtype,&itemhandle,&r);
	SetControlValue((ControlHandle) itemhandle,0);
	}
return(OK);
}


int GetGraphicSettings(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char s[MAXFIELDCONTENT];
Str255 t;
unsigned long p,q;
double pp,qq;
int rep;

InputOn++;
rep = MISSED;
GetDialogItem(gpDialogs[wGraphicSettings],fGraphicScale,&itemtype,
	&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,s);
if((FloatToNiceRatio(s,&p,&q) != OK) || (p == ZERO)
		|| (Simplify((double)INT_MAX,(double)5. * q,(double)p,&pp,&qq) != OK)) {
	Alert1("=> Scale out of range");
	goto SORTIR;
	}
while((pp > INT_MAX) || (qq > INT_MAX)) {
	pp = (pp / 2.);
	qq = (qq / 2.);
	}
if(GraphicScaleP != pp || GraphicScaleQ != qq) {
	Dirty[wGraphicSettings] = Dirty[iSettings] = TRUE;
	}
GraphicScaleP = (int) pp;
GraphicScaleQ = (int) qq;
/* my_sprintf(Message,"Scale = %ld / %ld",(long)GraphicScaleP,(long)GraphicScaleQ);
ShowMessage(TRUE,wMessage,Message); */
rep = OK;

SORTIR:
InputOn--;
return(rep);
}


int SetTimeAccuracy(void)
{
Rect r;
ControlHandle itemhandle;
short itemtype;
char line[MAXFIELDCONTENT];

my_sprintf(line,"%ld",(long)Time_res);
SetField(NULL, wTimeAccuracy, fTimeRes, line);
my_sprintf(line,"%ld",(long)Quantization);
SetField(NULL, wTimeAccuracy, fQuantize, line);
my_sprintf(line,"%ld",(long)MIDIsetUpTime);
SetField(NULL, wTimeAccuracy, fSetUpTime, line);
if(QuantizeOK) {
	GetDialogItem(gpDialogs[wTimeAccuracy],dOff,&itemtype,
		(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,0);
	GetDialogItem(gpDialogs[wTimeAccuracy],dOn,&itemtype,
		(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,1);
	if(Pclock < 0.9 && !LoadOn) {
		Alert1("=> Quantization requires a metronom value. It has been set to mm = 60");
		Pclock = Qclock = 1000.;
		SetTempo();
		BPActivateWindow(SLOW,wMetronom);
		}
	}
else {
	GetDialogItem(gpDialogs[wTimeAccuracy],dOff,&itemtype,
		(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,1);
	GetDialogItem(gpDialogs[wTimeAccuracy],dOn,&itemtype,
		(Handle*)&itemhandle,&r);
	SetControlValue(itemhandle,0);
	}
return(OK);
}


int GetTimeAccuracy(void)
{
Rect r;
int i;
Handle itemhandle;
long k;
short itemtype;
char line[MAXFIELDCONTENT];
Str255 t;

InputOn++;
GetDialogItem(gpDialogs[wTimeAccuracy],fSetUpTime,&itemtype,
	&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,line);
i = (int) atol(line); 	/* Don't use atoi() because int's are 4 bytes */
if(i < 0 || i > 2000) {
	Alert1("=> Range of set-up time: 0 - 2000ms");
	if(i > 2000) i = 2000;
	else i = 0;
	}
MIDIsetUpTime = i;
GetDialogItem(gpDialogs[wTimeAccuracy],fTimeRes,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,line);
k = atol(line);
if(k < 1L) {
	Alert1("=> Minimum time resolution: 1ms");
	Time_res = 1L;
	SetTimeAccuracy();
	InputOn--;
	return(MISSED);
	}
if(k != Time_res) 
	Interrupted = Dirty[wTimeAccuracy] = Dirty[iSettings] = TRUE;
Time_res = k;
GetDialogItem(gpDialogs[wTimeAccuracy],fQuantize,&itemtype,&itemhandle,&r);
GetDialogItemText(itemhandle,t);
MyPtoCstr(MAXFIELDCONTENT,t,line);
k = atol(line);
if(k < Time_res) {
	my_sprintf(Message,"=> Minimum quantization: %ldms",(long)Time_res);
	Alert1(Message);
	Quantization = Time_res;
	SetTimeAccuracy();
	InputOn--;
	return(MISSED);
	}
if(k != Quantization) {
	Dirty[wTimeAccuracy] = Dirty[iSettings] = TRUE;
	FixedMaxQuantization = AskedTempMemory = FALSE;
	}
Quantization = k;
SetGraphicSettings();
InputOn--;
return(OK);
}


int SetKeyboard(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char line[MAXFIELDCONTENT];
int i,j;

for(i=0; i < 52; i++) {
	if(i < 26) j = i + fA;
	else j = i - 26 + fa;
	if((*p_Token)[Key(i,KeyboardType)] != NULL) {
		MystrcpyTableToString(MAXFIELDCONTENT,line,p_Token,Key(i,KeyboardType));
		SetField(NULL, wKeyboard, j, line);
		}
	}
return(OK);
}


int Key(int i,int keyboardtype)
{
int j;

switch(keyboardtype) {
	case AZERTY: break;
	case QWERTY:
		j = 0; if(i > 25) {
			j = 26; i -= 26;
			}
		switch(i) {
			case 0: i = 16; break;	/* A */
			case 1: i = i; break;
			case 2: i = i; break;
			case 3: i = i; break;
			case 4: i = i; break;
			case 5: i = i; break;
			case 6: i = i; break;
			case 7: i = i; break;
			case 8: i = i; break;
			case 9: i = i; break;
			case 10: i = i; break;
			case 11: i = i; break;
			case 12: i = i; break;
			case 13: i = i; break;
			case 14: i = i; break;
			case 15: i = i; break;
			case 16: i = 0; break;	/* Q */
			case 17: i = i; break;
			case 18: i = i; break;
			case 19: i = i; break;
			case 20: i = i; break;
			case 21: i = i; break;
			case 22: i = 25; break;	/* W */
			case 23: i = i; break;
			case 24: i = i; break;
			case 25: i = 22; break;	/* Z */
			}
		i += j;
	}
return(i);
}


int ResetKeyboard(int quick)
{
Rect r;
Handle itemhandle;
short itemtype;
char line[2];
int i,j,c,rep;

if(!quick && (rep=Answer("Suppress all tokens",'N')) != OK) return(rep);

DOIT:
if(SaveCheck(wKeyboard) == ABORT) return(ABORT);
line[1] = '\0';
for(i=0; i < 52; i++) {
	if(i <= 25) c = i + 'A';
	else c = '.';
	if(i < 26) j = i + fA;
	else j = i - 26 + fa;
	line[0] = c;
	SetField(NULL, wKeyboard, j, line);
	}
Dirty[wKeyboard] = FALSE;
Token = SpaceOn = FALSE;
SwitchOff(NULL,wKeyboard,dToken);
ForgetFileName(wKeyboard);
GetKeyboard();
return(OK);
}


int GetKeyboard(void)
{
Rect r;
Handle itemhandle;
short itemtype;
int i,j;
char line[MAXFIELDCONTENT],**ptr;
Str255 t;

for(i=0; i < 52; i++) {
	if(i < 26) j = i + fA;
	else j = i - 26 + fa;
	GetDialogItem(gpDialogs[wKeyboard],j,&itemtype,&itemhandle,&r);
	GetDialogItemText(itemhandle,t);
	MyPtoCstr(MAXFIELDCONTENT,t,line);
	ptr = (*p_Token)[Key(i,KeyboardType)];
	MyDisposeHandle((Handle*)&ptr);
	if((ptr = (char**) GiveSpace((Size)strlen(line)+1)) == NULL) return(ABORT);
	(*p_Token)[Key(i,KeyboardType)] = ptr;
	MystrcpyStringToTable(p_Token,Key(i,KeyboardType),line);
	}
return(OK);
}


int SetDefaultStrikeMode(void)
{
if(StrikeAgainDefault) {
	SwitchOn(StrikeModePtr,-1,bDefaultStrikeAgain);
	SwitchOff(StrikeModePtr,-1,bDefaultDontStrikeAgain);
	}
else {
	SwitchOn(StrikeModePtr,-1,bDefaultDontStrikeAgain);
	SwitchOff(StrikeModePtr,-1,bDefaultStrikeAgain);
	}
return(OK);
}


int SetFileSavePreferences(void)
{
char line[MAXFIELDCONTENT];
	
switch(FileSaveMode) {
	case ALLSAME:
		SwitchOn(FileSavePreferencesPtr,-1,bSaveAllToSame);
		SwitchOff(FileSavePreferencesPtr,-1,bSaveAllToSameWithPrompt);
		SwitchOff(FileSavePreferencesPtr,-1,bEachToNew);
		break;
	case ALLSAMEPROMPT:
		SwitchOn(FileSavePreferencesPtr,-1,bSaveAllToSameWithPrompt);
		SwitchOff(FileSavePreferencesPtr,-1,bSaveAllToSame);
		SwitchOff(FileSavePreferencesPtr,-1,bEachToNew);
		break;
	case NEWFILE:
		SwitchOn(FileSavePreferencesPtr,-1,bEachToNew);
		SwitchOff(FileSavePreferencesPtr,-1,bSaveAllToSame);
		SwitchOff(FileSavePreferencesPtr,-1,bSaveAllToSameWithPrompt);
		break;
	}
if(FileWriteMode == NOW) {
	SwitchOn(FileSavePreferencesPtr,-1,bWriteNow);
	SwitchOff(FileSavePreferencesPtr,-1,bWriteLater);
	}
else {
	SwitchOn(FileSavePreferencesPtr,-1,bWriteLater);
	SwitchOff(FileSavePreferencesPtr,-1,bWriteNow);
	}
switch(MIDIfileType) {
	case 0:
		SwitchOn(FileSavePreferencesPtr,-1,bType0);
		SwitchOff(FileSavePreferencesPtr,-1,bType1);
		SwitchOff(FileSavePreferencesPtr,-1,bType2);
		break;
	case 1:
		SwitchOn(FileSavePreferencesPtr,-1,bType1);
		SwitchOff(FileSavePreferencesPtr,-1,bType0);
		SwitchOff(FileSavePreferencesPtr,-1,bType2);
		break;
	case 2:
		SwitchOn(FileSavePreferencesPtr,-1,bType2);
		SwitchOff(FileSavePreferencesPtr,-1,bType1);
		SwitchOff(FileSavePreferencesPtr,-1,bType0);
		break;
	}
switch(CsoundFileFormat) {
	case MAC:
		SwitchOn(FileSavePreferencesPtr,-1,bTypeMac);
		SwitchOff(FileSavePreferencesPtr,-1,bTypeDos);
		SwitchOff(FileSavePreferencesPtr,-1,bTypeUnix);
		break;
	case DOS:
		SwitchOn(FileSavePreferencesPtr,-1,bTypeDos);
		SwitchOff(FileSavePreferencesPtr,-1,bTypeMac);
		SwitchOff(FileSavePreferencesPtr,-1,bTypeUnix);
		break;
	case UNIX:
		SwitchOn(FileSavePreferencesPtr,-1,bTypeUnix);
		SwitchOff(FileSavePreferencesPtr,-1,bTypeDos);
		SwitchOff(FileSavePreferencesPtr,-1,bTypeMac);
		break;
	}
if(OutCsound)
	SetField(FileSavePreferencesPtr,-1,fCsoundFileName,CsFileName);
if(WriteMIDIfile)
	SetField(FileSavePreferencesPtr,-1,fMIDIFileName,MIDIfileName);
my_sprintf(line,"%.2f",EndFadeOut);
SetField(FileSavePreferencesPtr,-1,fFadeOut,line);
return(OK);
}


int GetFileSavePreferences(void)
{
int result;
long p,q;
char line[MAXFIELDCONTENT];

if(FileSaveMode == ALLSAME || FileSaveMode == ALLSAMEPROMPT) {
	GetCsoundScoreName();
	GetMIDIfileName();
	}
if(GetField(FileSavePreferencesPtr,YES,-1,fFadeOut,LineBuff,&p,&q) != OK) {
	p = 3L; q = 1L;
	}

FADEOUTVALUE:
EndFadeOut = ((float)p) / q;
if(EndFadeOut < 0. || EndFadeOut > 100.) {
	ShowWindow(GetDialogWindow(FileSavePreferencesPtr));
	BringToFront(GetDialogWindow(FileSavePreferencesPtr));
	BPUpdateDialog(FileSavePreferencesPtr);
	SelectField(FileSavePreferencesPtr,-1,fFadeOut,TRUE);
	Alert1("Range for MIDI fade out is 0..100 seconds");
	result = AnswerWith("Set fade out to...","0.00",line);
	if(result != OK) goto ERR;
	else {
		Myatof(line,&p,&q);
		EndFadeOut = ((float)p) / q;
		my_sprintf(line,"%.2f",EndFadeOut);
		SetField(FileSavePreferencesPtr,-1,fFadeOut,line);
		BPUpdateDialog(FileSavePreferencesPtr);
		}
	goto FADEOUTVALUE;
	}
return(OK);

ERR:
SetField(FileSavePreferencesPtr,-1,fFadeOut,"[?]");
BPUpdateDialog(FileSavePreferencesPtr);
SelectField(FileSavePreferencesPtr,-1,fFadeOut,TRUE);
return(MISSED);
}


int GetControlParameters(void)
{
char line[MAXFIELDCONTENT];
long p,q;
int oldtransposevalue;

oldtransposevalue = TransposeValue;
GetField(NULL,TRUE,wControlPannel,fTransposeInput,line,&p,&q);
TransposeValue = (unsigned) p/q;
if(TransposeValue != oldtransposevalue && TransposeValue != 0
	&& TransposeInput && ReadKeyBoardOn && Jcontrol == -1 && LastEditWindow != wScript) {
	if(!EmptyBeat) Print(LastEditWindow," ");
	PrintHandle(LastEditWindow,(*p_PerformanceControl)[33]);
	my_sprintf(Message,"(%ld)",(long)-TransposeValue);
	Print(LastEditWindow,Message);
	EmptyBeat = FALSE;
	}
return(OK);
}


int GetTuning(void)
{
char line[MAXFIELDCONTENT];
long p,q;
int i,oldC4key;
double x;

oldC4key = C4key;
GetField(TuningPtr,TRUE,-1,fC4key,line,&p,&q);
i = p / q;
if(i < 2 || i > 127) {
	my_sprintf(Message,"=> Key for C4 should be in range 2..127 (typ. 60). Can't accept %ld",(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(TuningPtr));
	SelectWindow(GetDialogWindow(TuningPtr));
	SetField(TuningPtr,-1,fC4key,"[?]");
	return(MISSED);
	}
C4key = i;
if(C4key != oldC4key) CompiledGr = CompiledGl = FALSE;

int GetField(TuningPtr,TRUE,-1,fA4freq,line,&p,&q);
x = ((double) p) / q;
if(x < 25. || x > 2000.) {
	my_sprintf(Message,"=> Frequency for A4 should be in range 25..2000 (typ. 440). Can't accept %.2f",x);
	Alert1(Message);
	ShowWindow(GetDialogWindow(TuningPtr));
	SelectWindow(GetDialogWindow(TuningPtr));
	SetField(TuningPtr,-1,fA4freq,"[?]");
	return(MISSED);
	}
A4freq = x;
return(OK);
}


int SetTuning(void)
{
my_sprintf(Message,"%ld",(long)C4key);
SetField(TuningPtr,-1,fC4key,Message);
my_sprintf(Message,"%.4f",A4freq);
SetField(TuningPtr,-1,fA4freq,Message);
return(OK);
}


int GetDefaultPerformanceValues(void)
{
char line[MAXFIELDCONTENT];
long p,q;
int i;

GetField(DefaultPerformanceValuesPtr,TRUE,-1,fDeftVolume,line,&p,&q);
i = p / q;
if(i < 1 || i > 127) {
	my_sprintf(Message,"=> Default volume should be in range 1..127 (typ. 90). Can't accept %ld",
		(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SetField(DefaultPerformanceValuesPtr,-1,fDeftVolume,"90");
	return(MISSED);
	}
DeftVolume = i;

GetField(DefaultPerformanceValuesPtr,TRUE,-1,fDeftVelocity,line,&p,&q);
i = p / q;
if(i < 1 || i > 127) {
	my_sprintf(Message,"=> Default velocity should be in range 1..127 (typ. 64). Can't accept %ld",
		(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SetField(DefaultPerformanceValuesPtr,-1,fDeftVelocity,"64");
	return(MISSED);
	}
DeftVelocity = i;

GetField(DefaultPerformanceValuesPtr,TRUE,-1,fDeftPanoramic,line,&p,&q);
i = p / q;
if(i < 0 || i > 127) {
	my_sprintf(Message,"=> Default panoramic should be in range 0..127 (typ. 64). Can't accept %ld",
		(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SetField(DefaultPerformanceValuesPtr,-1,fDeftPanoramic,"64");
	return(MISSED);
	}
DeftPanoramic = i;

GetField(DefaultPerformanceValuesPtr,TRUE,-1,fPanoramicController,line,&p,&q);
i = p / q;
if(i < 0 || i > 127) {
	my_sprintf(Message,"=> Panoramic control index should be in range 0..127 (typ. 10). Can't accept %ld",
		(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SetField(DefaultPerformanceValuesPtr,-1,fPanoramicController,"10");
	return(MISSED);
	}
PanoramicController = i;

GetField(DefaultPerformanceValuesPtr,TRUE,-1,fVolumeController,line,&p,&q);
i = p / q;
if(i < 0 || i > 127) {
	my_sprintf(Message,"=> Volume control index should be in range 0..127 (typ. 7). Can't accept %ld",
		(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SetField(DefaultPerformanceValuesPtr,-1,fVolumeController,"7");
	return(MISSED);
	}
VolumeController = i;

GetField(DefaultPerformanceValuesPtr,TRUE,-1,fSamplingRate,line,&p,&q);
i = p / q;
if(i < 1 || i > 500) {
	my_sprintf(Message,"=> Default sample rate should be in range 1..500 (typ. 50). Can't accept %ld",
		(long) i);
	Alert1(Message);
	ShowWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SelectWindow(GetDialogWindow(DefaultPerformanceValuesPtr));
	SetField(DefaultPerformanceValuesPtr,-1,fSamplingRate,"50");
	return(MISSED);
	}
SamplingRate = i;

return(OK);
}


int SetDefaultPerformanceValues(void)
{
my_sprintf(Message,"%ld",(long)DeftVolume);
SetField(DefaultPerformanceValuesPtr,-1,fDeftVolume,Message);

my_sprintf(Message,"%ld",(long)DeftVelocity);
SetField(DefaultPerformanceValuesPtr,-1,fDeftVelocity,Message);

my_sprintf(Message,"%ld",(long)DeftPanoramic);
SetField(DefaultPerformanceValuesPtr,-1,fDeftPanoramic,Message);

my_sprintf(Message,"%ld",(long)PanoramicController);
SetField(DefaultPerformanceValuesPtr,-1,fPanoramicController,Message);

my_sprintf(Message,"%ld",(long)VolumeController);
SetField(DefaultPerformanceValuesPtr,-1,fVolumeController,Message);

my_sprintf(Message,"%ld",(long)SamplingRate);
SetField(DefaultPerformanceValuesPtr,-1,fSamplingRate,Message);
return(OK);
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

#if 0
pascal void DrawButtonBorder(DialogPtr thedialog)
{
Handle thecontrol;
short type;
Rect r;

GetDialogItem(thedialog,1,&type,&thecontrol,&r);
pen_size("canvas",3,3);
resize_rect(&r,-2,-2);
FrameRoundRect(&r,16,16);
}
#endif


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


char GetCap(void)
{
EventRecord theEvent;
int r,compiledmem,dirtymem;

ShowSelect(CENTRE,wTrace);
BPActivateWindow(SLOW,wTrace);
ShowMessage(TRUE,wMessage,"Type answer!");

#if BP_CARBON_GUI_FORGET_THIS
while(!WaitNextEvent(everyEvent,&theEvent,3L,NULL) || ((theEvent.what != keyDown)
		&& (theEvent.what != autoKey))) {
	ListenToEvents();
	if(Button()) {
		compiledmem = CompiledGr;
		dirtymem = Dirty[wAlphabet]; Dirty[wAlphabet] = FALSE;
		while((r = MainEvent()) != RESUME && r != STOP && r != EXIT){};
		if(r == EXIT) r = STOP;
		if(Dirty[wAlphabet]) {
			Alert1("Alphabet changed. Must recompile...");
			return('Q');
			}
		Dirty[wAlphabet] = dirtymem;
		if(compiledmem && !CompiledGr) {
			Alert1("Grammar changed. Must recompile...");
			return('Q');
			}
		if(r == STOP) return('Q');
		ShowMessage(TRUE,wMessage,"Type answer!");
		}
	}
return(UpperCase((char) (theEvent.message & charCodeMask)));
#else
// FIXME: do we need a console equivalent of this function ??
return ('Q');
#endif
}


int Date(char line[])
{
unsigned long datetime;
char dd[MAXNAME],tt[MAXNAME];

#if BP_CARBON_GUI_FORGET_THIS
Str255 pascalline;
Handle i1h;		        /* handle to an Intl1 or Intl0 Rec  */

/* GetIntlResource() seems to always return the same handles with Carbon,
   so I do not think that they need to be deallocated - akozar */
i1h = GetIntlResource(1); /* Note: does not return a resource handle on Carbon */
GetDateTime(&datetime);	  /* See DateTimeUtils.h */
if (i1h != NULL) {
	DateString(datetime,abbrevDate,pascalline, i1h);
	MyPtoCstr(MAXNAME,pascalline,dd);
	}
else  dd[0] = '\0';
i1h = GetIntlResource(0);
if (i1h != NULL) {
	TimeString(datetime,FALSE,pascalline, i1h);
	MyPtoCstr(MAXNAME,pascalline,tt);
	}
else  tt[0] = '\0';
#else
time_t curtime;
struct tm *loctime;

time(&curtime);
loctime = localtime(&curtime);
// format the date
if (strftime(dd, MAXNAME, "%a, %b %d, %Y", loctime) == 0) {
	if (Beta) Alert1("=> Err. Date(): strftime() results were too long for dd.");
	dd[0] = '\0';
	}
// format the time
if (strftime(tt, MAXNAME, "%I:%M %p", loctime) == 0) {
	if (Beta) Alert1("=> Err. Date(): strftime() results were too long for tt.");
	tt[0] = '\0';
	}
#endif

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
	Alert1(Message);
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
if(trace_scale) BPPrintMessage(odInfo, "FixStringConstant() j = %d, line = %s\n",j,line);
return(j);

ERR:
Alert1("Missing parameter name");
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
// if(trace_scale) BPPrintMessage(odInfo,"FixNumberConstant line = %s x = %.3f\n",line,x);

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
	if(trace_FixNumberConstant || trace_scale) BPPrintMessage(odInfo,"FixNumberConstant() line = %s j = %ld, x = %.3f\n",line,(long)j,x);
	return(j);
	}
if(j >= MAXSTRINGCONSTANTS) {
	my_sprintf(Message,
		"=> Too many numeric constants found (max %ld)\nCan't store '%s'\n",(long)MAXSTRINGCONSTANTS,line);
	Alert1(Message);
	return(ABORT);
	}
	
h = (Handle) p_NumberConstant;
if((h = IncreaseSpace(h)) == NULL) return(ABORT);
p_NumberConstant = (double**) h;
oldmaxparam = maxparam;
maxparam = (MyGetHandleSize((Handle)p_NumberConstant) / sizeof(double));
for(i = oldmaxparam + 1; i < maxparam; i++) (*p_NumberConstant)[i] = Infpos;
		
(*p_NumberConstant)[j] = x;
if(trace_FixNumberConstant) BPPrintMessage(odInfo,"FixNumberConstant() after increasing space line = '%s' j = %ld, x = %.3f\n",line,(long)j,x);
return(j);

ERR:
Alert1("Missing value");
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
/* register */ int  i;
tokenbyte m,p;

for(i=0; ((m=(**pp_X)[i]) != TEND) || ((**pp_X)[i+1] != TEND); i+=2) {
	if(m != T4) continue;
	p = (**pp_X)[i+1];
	if(p > Jvar || p_VarStatus == NULL) {
		if(Beta) Alert1("=> Err. NeedGlossary(). p > Jvar || p_VarStatus == NULL");
		return(NO);
		}
	if((*p_VarStatus)[p] & 4) return(YES);
	}
return(NO);
}

#if 0
pascal void MySoundProc(short sndNum)
/* sndNum will range from 0 to 3 */
{
SndChannelPtr myChan = 0L;
SndListHandle mySound;
OSErr err;

if(sndNum == 0 || AlertMute) return;
mySound = (SndListHandle) GetResource(soundListRsrc,kpopID);
err = SndNewChannel( &myChan, 0, 0, 0L );
HLock((Handle) mySound );
err = SndPlay(myChan,mySound,FALSE);
HUnlock((Handle) mySound);
err = SndDisposeChannel(myChan,FALSE);
}
#endif

// ------------------------  Random numbers -------------------------

#if BP_CARBON_GUI_FORGET_THIS

int SetSeed(void)
{
Rect r;
Handle itemhandle;
short itemtype;
char line[MAXFIELDCONTENT];

my_sprintf(line,"%.0f",(double) Seed);
SetField(NULL, wRandomSequence, fSeed, line);
return(OK);
}


int GetSeed(void)
{
char line[MAXFIELDCONTENT];
long p,q,newseed;

GetField(NULL,TRUE,wRandomSequence,fSeed,line,&p,&q);
newseed = p / q;
if(newseed < 0 || newseed > 32767) {
	Alert1("Random seed must be in range [0..32767]");
	ShowWindow(GetDialogWindow(gpDialogs[wRandomSequence]));
	BringToFront(GetDialogWindow(gpDialogs[wRandomSequence]));
	BPUpdateDialog(gpDialogs[wRandomSequence]);
	SelectField(NULL,wRandomSequence,fSeed,TRUE);
	return(MISSED);
	}
Seed = (unsigned int) newseed;
return(OK);
}

#endif /* BP_CARBON_GUI_FORGET_THIS */

int ResetRandom(void)
{
if(Seed > 0) {
	srand(Seed);
//	BPPrintMessage(odInfo, "Random seed reset to %u\n", Seed);
	UsedRandom = FALSE;
	}
else Randomize();
AppendScript(55);
return(OK);
}


int Randomize(void)
{
if(Seed > 0) return(OK);
ReseedOrShuffle(NEWSEED);
my_sprintf(Message,"%.0f",(double)Seed);
MystrcpyStringToTable(ScriptLine.arg,0,Message);
AppendScript(57);
return(OK);
}


int ReseedOrShuffle(int what)
{
unsigned int seed;
int randomnumber;

switch(what) {
	case NOSEED:
		Seed = 0;
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
			BPPrintMessage(odInfo, "New random seed = %u\n", seed);
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
			BPPrintMessage(odInfo, "Random seed = %u\n", seed);
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


double GetScalingValue(tokenbyte **p_a,unsigned long i)
{
tokenbyte m,p;
double value;

m = (*p_a)[i+3L];
p = (*p_a)[i+5L];
if(m < 0 || p < 0) {
	if(Beta) Alert1("=> Err. GetScalingValue(). m < 0 || p < 0");
	return(1.);
	}
value = ((double)TOKBASE * m) + p;
return(value);
}
