// HTMLfix.h

#include <stdio.h>
#include <Printing.h>
#include <string.h>
#include <Aliases.h>
#include <Picker.h>
#include <LowMem.h>
#include <ctype.h>
#include <stdlib.h>

#define MAXNAME 28
#define MAXLIN 100
#define MAXLIN2 400
#define MAXRECORD 800
#define MAXEXTENSIONS 30
#define MAXNAMES 100
#define MAXCODES 100
#define MAXCODELENGTH 20
#define MAXEXTENSIONLENGTH 10

#define ZERO 0L;

/* Constants used in function returns */
#define SINGLE 2
#define OK 1
#define YES 1
#define TRUE 1
#define FAILED 0
#define NO 0
#define FALSE 0
#define BACKTRACK -1
#define UNDO -2
#define ENDREPEAT -3
#define ABORT -4
#define QUICK -5
#define AGAIN -6
#define STOP -7
#define RESUME -8
#define FINISH -9
#define SLOW -10
#define EXIT -11

#define OFF 0
#define ON 1

#define MAC 0
#define DOS 1
#define UNIX 2

#define NONE 0
#define QUOTED_PRINTABLE 1
#define FILENAME 2

#define NETSCAPE 0
#define SIMPLETEXT 1

typedef struct {
	char domain[MAXLIN];
	char institution_name[MAXLIN];
	int nb_users,maxusers;
	char ****h_userid;
	char ****h_username;
	} t_emailnode;

typedef struct {
	int id;
	int tag;
	char name[MAXLIN];
	int **p_link;
	int maxlinks;
	} hyperlink;

#include "HTMLfix.proto.h"
#include "files.proto.h"
#include "misc.proto.h"