SRCDIR = source/BP3
PREFIX = source/BP3/PrefixANSIDebug.h
CC     = gcc
LIBS   = -lm
EXE    = bp

UNAME_S := Windows
UNAME_M := x86_64

# Only set Unix-specific flags if on Unix (this part will just be skipped on Windows)
ifeq ($(OS),Windows_NT)
	CFLAGS =
	FRAMEWORKS = -lwinmm -Wall -Wextra
	EXE = bp.exe
else
	UNAME_S := $(shell uname -s)
	UNAME_M := $(shell uname -m)
	ifeq ($(UNAME_S),Darwin)
		CFLAGS =
		FRAMEWORKS = -framework CoreMIDI -framework CoreFoundation
	endif
	ifeq ($(UNAME_S),Linux)
		CFLAGS = 
		FRAMEWORKS = -lasound
	endif
endif

# Print the operating system detected (optional, for debugging)
$(info Operating System: $(UNAME_S))

SRCS = \
	$(SRCDIR)/Arithmetic.c \
	$(SRCDIR)/Automata.c \
	$(SRCDIR)/CompileGrammar.c \
	$(SRCDIR)/CompileProcs.c \
	$(SRCDIR)/Compute.c \
	$(SRCDIR)/ConsoleMain.c \
	$(SRCDIR)/ConsoleMemory.c \
	$(SRCDIR)/ConsoleMessages.c \
	$(SRCDIR)/ConsoleStubs.c \
	$(SRCDIR)/Csound.c \
	$(SRCDIR)/CsoundMaths.c \
	$(SRCDIR)/CsoundScoreMake.c \
	$(SRCDIR)/CTextHandles.c \
	$(SRCDIR)/DisplayArg.c \
	$(SRCDIR)/DisplayThings.c \
	$(SRCDIR)/Encode.c \
	$(SRCDIR)/FillPhaseDiagram.c \
	$(SRCDIR)/GetRelease.c \
	$(SRCDIR)/Glossary.c \
	$(SRCDIR)/Graphic.c \
	$(SRCDIR)/HTML.c \
	$(SRCDIR)/Inits.c \
	$(SRCDIR)/Interface2.c \
	$(SRCDIR)/MakeSound.c \
	$(SRCDIR)/MIDIdriver.c \
	$(SRCDIR)/MIDIfiles.c \
	$(SRCDIR)/MIDIloads.c \
	$(SRCDIR)/MIDIstuff.c \
	$(SRCDIR)/Misc.c \
	$(SRCDIR)/PlayThings.c \
	$(SRCDIR)/Polymetric.c \
	$(SRCDIR)/ProduceItems.c \
	$(SRCDIR)/SaveLoads1.c \
	$(SRCDIR)/SaveLoads3.c \
	$(SRCDIR)/Script.c \
	$(SRCDIR)/ScriptUtils.c \
	$(SRCDIR)/SetObjectFeatures.c \
	$(SRCDIR)/SoundObjects2.c \
	$(SRCDIR)/SoundObjects3.c \
	$(SRCDIR)/Strings.c \
	$(SRCDIR)/Ticks.c \
	$(SRCDIR)/TimeSet.c \
	$(SRCDIR)/TimeSetFunctions.c \
	$(SRCDIR)/Zouleb.c

OBJS = $(SRCS:.c=.o)

all:  $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(LIBS) $(FRAMEWORKS)

%.o : %.c
	$(CC) -c $(CFLAGS) -I $(SRCDIR) -include $(PREFIX)  $< -o $@

depend: 
	makedepend -I $(SRCDIR) -include $(PREFIX)  $(SRCS)

clean:
ifeq ($(UNAME_S),Darwin)
	-rm -f $(EXE)
	-rm -f $(SRCDIR)/*.o
else ifeq ($(UNAME_S),Linux)
	-rm -f $(EXE)
	-rm -f $(SRCDIR)/*.o
else ifeq ($(OS),Windows_NT)
	@if exist $(EXE) (echo Deleting $(EXE) & del /Q $(EXE))
	@if exist $(subst /,\,$(SRCDIR))\*.o (echo Deleting $(SRCDIR)\*.o & del /Q $(subst /,\,$(SRCDIR))\*.o)
endif
