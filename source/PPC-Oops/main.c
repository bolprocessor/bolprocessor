// main.c

// Size flags for this project are 08C8
// Don't forget to include MacTraps and ANSI

#include <string.h>

WindowPtr Window;
TEHandle TEH;

int MakeWindow(Str255 name, Rect *p_rBounds);
int MyPrint(char line[]);

main (void)
{
Rect rBounds;

InitGraf(&thePort);
InitFonts();
FlushEvents(everyEvent,0);
InitWindows();
InitMenus();
TEInit();
InitDialogs(0L);
InitCursor();


// Create the display window
rBounds.top = 200; rBounds.left = 200;
rBounds.bottom = 260; rBounds.right = 400;
MakeWindow("\p",&rBounds);
MyPrint("Oops… this program runs only on PowerPC computers!\r\r      << click mouse >>");
SysBeep(10);
while(!Button());
return;
}


MyPrint(char line[])
{
SelectWindow(Window);
TEInsert(line,(long) strlen(line),TEH);
TESetSelect((**TEH).selEnd,(**TEH).selEnd,TEH);
TESelView(TEH);
return(OK);
}


MakeWindow(Str255 name, Rect *p_rBounds)
{
Rect viewRect;
long refcon;

Window = NewWindow(0L,p_rBounds,name,TRUE,altDBoxProc,(WindowPtr)-1L,FALSE,refcon);
SetPort(Window);
TextFont(courier);
TextSize(10);
viewRect = thePort->portRect;
InsetRect(&viewRect,4,4);
TEH = TENew(&viewRect,&viewRect);
TEAutoView(TRUE,TEH);
return(OK);
}