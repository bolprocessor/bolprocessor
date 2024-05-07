SRCDIR = source/BP3
PREFIX = source/prefix/PrefixANSIDebug.h
CC     = gcc
CFLAGS =
LIBS   = -lm
EXE    = bp

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Darwin)
    FRAMEWORKS = -framework CoreMIDI -framework CoreFoundation
endif
ifeq ($(UNAME_S),Linux)
    FRAMEWORKS = -lasound  # Example: linking with ALSA for MIDI handling on Linux
endif
ifneq (,$(findstring CYGWIN,$(UNAME_S)))
    FRAMEWORKS = -lwinmm  # Example: linking with WinMM library on Cygwin
endif
ifneq (,$(findstring MINGW,$(UNAME_S)))
    FRAMEWORKS = -lwinmm  # Example: linking with WinMM library on MinGW
endif


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
  $(SRCDIR)/MIDIfiles.c \
  $(SRCDIR)/MIDIloads.c \
  $(SRCDIR)/MIDIstubs.c \
  $(SRCDIR)/MIDIstuff.c \
  $(SRCDIR)/Misc.c \
  $(SRCDIR)/PlayThings.c \
  $(SRCDIR)/Polymetric.c \
  $(SRCDIR)/ProduceItems.c \
  $(SRCDIR)/SaveLoads1.c \
  $(SRCDIR)/SaveLoads3.c \
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
	rm $(EXE)
	rm $(SRCDIR)/*.o

# DO NOT DELETE

source/BP3/Arithmetic.o: source/prefix/PrefixANSIDebug.h
source/BP3/Arithmetic.o: source/BP3/MissingMacTypes.h
source/BP3/Arithmetic.o: source/BP3/MissingMacPrototypes.h
source/BP3/Arithmetic.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Arithmetic.o: source/BP3/midi1.h
source/BP3/Arithmetic.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/Automata.o: source/prefix/PrefixANSIDebug.h
source/BP3/Automata.o: source/BP3/MissingMacTypes.h 
source/BP3/Automata.o: source/BP3/MissingMacPrototypes.h
source/BP3/Automata.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Automata.o: source/BP3/midi1.h
source/BP3/Automata.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/CompileGrammar.o: source/prefix/PrefixANSIDebug.h
source/BP3/CompileGrammar.o: source/BP3/MissingMacTypes.h
source/BP3/CompileGrammar.o: source/BP3/MissingMacPrototypes.h
source/BP3/CompileGrammar.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/CompileGrammar.o: source/BP3/midi1.h
source/BP3/CompileGrammar.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/CompileProcs.o: source/prefix/PrefixANSIDebug.h
source/BP3/CompileProcs.o: source/BP3/MissingMacTypes.h 
source/BP3/CompileProcs.o: source/BP3/MissingMacPrototypes.h
source/BP3/CompileProcs.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/CompileProcs.o: source/BP3/midi1.h
source/BP3/CompileProcs.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/Compute.o: source/prefix/PrefixANSIDebug.h
source/BP3/Compute.o: source/BP3/MissingMacTypes.h 
source/BP3/Compute.o: source/BP3/MissingMacPrototypes.h
source/BP3/Compute.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Compute.o: source/BP3/midi1.h
source/BP3/Compute.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/ConsoleMain.o: source/prefix/PrefixANSIDebug.h
source/BP3/ConsoleMain.o: source/BP3/MissingMacTypes.h 
source/BP3/ConsoleMain.o: source/BP3/MissingMacPrototypes.h
source/BP3/ConsoleMain.o: source/BP3/CTextHandles.h 
source/BP3/ConsoleMain.o: source/BP3/-BP2.h
source/BP3/ConsoleMain.o: source/BP3/midi1.h
source/BP3/ConsoleMain.o: source/BP3/-BP2.proto.h source/BP3/-BP2main.h
source/BP3/ConsoleMain.o: source/BP3/ConsoleMessages.h
source/BP3/ConsoleMain.o: source/BP3/ConsoleGlobals.h
source/BP3/ConsoleMemory.o: source/prefix/PrefixANSIDebug.h
source/BP3/ConsoleMemory.o: source/BP3/MissingMacTypes.h
source/BP3/ConsoleMemory.o: source/BP3/MissingMacPrototypes.h
source/BP3/ConsoleMemory.o: source/BP3/CTextHandles.h 
source/BP3/ConsoleMemory.o: source/BP3/-BP2.h 
source/BP3/ConsoleMemory.o: source/BP3/midi1.h
source/BP3/ConsoleMemory.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/ConsoleMessages.o: source/prefix/PrefixANSIDebug.h
source/BP3/ConsoleMessages.o: source/BP3/MissingMacTypes.h
source/BP3/ConsoleMessages.o: source/BP3/MissingMacPrototypes.h
source/BP3/ConsoleMessages.o: source/BP3/CTextHandles.h 
source/BP3/ConsoleMessages.o: source/BP3/-BP2.h
source/BP3/ConsoleMessages.o: source/BP3/midi1.h
source/BP3/ConsoleMessages.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/ConsoleMessages.o: source/BP3/ConsoleMessages.h
source/BP3/ConsoleStubs.o: source/prefix/PrefixANSIDebug.h
source/BP3/ConsoleStubs.o: source/BP3/MissingMacTypes.h 
source/BP3/ConsoleStubs.o: source/BP3/MissingMacPrototypes.h
source/BP3/ConsoleStubs.o: source/BP3/CTextHandles.h 
source/BP3/ConsoleStubs.o: source/BP3/-BP2.h
source/BP3/ConsoleStubs.o: source/BP3/midi1.h
source/BP3/ConsoleStubs.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/ConsoleStubs.o: source/BP3/ConsoleMessages.h
source/BP3/ConsoleStubs.o: source/BP3/ConsoleGlobals.h
source/BP3/Csound.o: source/prefix/PrefixANSIDebug.h
source/BP3/Csound.o: source/BP3/MissingMacTypes.h 
source/BP3/Csound.o: source/BP3/MissingMacPrototypes.h
source/BP3/Csound.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Csound.o: source/BP3/midi1.h
source/BP3/Csound.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/CsoundMaths.o: source/prefix/PrefixANSIDebug.h
source/BP3/CsoundMaths.o: source/BP3/MissingMacTypes.h 
source/BP3/CsoundMaths.o: source/BP3/MissingMacPrototypes.h
source/BP3/CsoundMaths.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/CsoundMaths.o: source/BP3/midi1.h
source/BP3/CsoundMaths.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/CsoundScoreMake.o: source/prefix/PrefixANSIDebug.h
source/BP3/CsoundScoreMake.o: source/BP3/MissingMacTypes.h
source/BP3/CsoundScoreMake.o: source/BP3/MissingMacPrototypes.h
source/BP3/CsoundScoreMake.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/CsoundScoreMake.o: source/BP3/midi1.h
source/BP3/CsoundScoreMake.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/CTextHandles.o: source/prefix/PrefixANSIDebug.h
source/BP3/CTextHandles.o: source/BP3/MissingMacTypes.h 
source/BP3/CTextHandles.o: source/BP3/MissingMacPrototypes.h
source/BP3/CTextHandles.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/CTextHandles.o: source/BP3/midi1.h
source/BP3/CTextHandles.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/DisplayArg.o: source/prefix/PrefixANSIDebug.h
source/BP3/DisplayArg.o: source/BP3/MissingMacTypes.h 
source/BP3/DisplayArg.o: source/BP3/MissingMacPrototypes.h
source/BP3/DisplayArg.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/DisplayArg.o: source/BP3/midi1.h
source/BP3/DisplayArg.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/DisplayThings.o: source/prefix/PrefixANSIDebug.h
source/BP3/DisplayThings.o: source/BP3/MissingMacTypes.h
source/BP3/DisplayThings.o: source/BP3/MissingMacPrototypes.h
source/BP3/DisplayThings.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/DisplayThings.o: source/BP3/midi1.h
source/BP3/DisplayThings.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/Encode.o: source/prefix/PrefixANSIDebug.h
source/BP3/Encode.o: source/BP3/MissingMacTypes.h 
source/BP3/Encode.o: source/BP3/MissingMacPrototypes.h
source/BP3/Encode.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Encode.o: source/BP3/midi1.h
source/BP3/Encode.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/FillPhaseDiagram.o: source/prefix/PrefixANSIDebug.h
source/BP3/FillPhaseDiagram.o: source/BP3/MissingMacTypes.h
source/BP3/FillPhaseDiagram.o: source/BP3/MissingMacPrototypes.h
source/BP3/FillPhaseDiagram.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/FillPhaseDiagram.o: source/BP3/midi1.h source/BP3/-BP2.proto.h
source/BP3/FillPhaseDiagram.o: source/BP3/-BP2decl.h
source/BP3/GetRelease.o: source/prefix/PrefixANSIDebug.h
source/BP3/GetRelease.o: source/BP3/MissingMacTypes.h 
source/BP3/GetRelease.o: source/BP3/MissingMacPrototypes.h
source/BP3/GetRelease.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/GetRelease.o: source/BP3/midi1.h
source/BP3/GetRelease.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/Glossary.o: source/prefix/PrefixANSIDebug.h
source/BP3/Glossary.o: source/BP3/MissingMacTypes.h 
source/BP3/Glossary.o: source/BP3/MissingMacPrototypes.h
source/BP3/Glossary.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Glossary.o: source/BP3/midi1.h
source/BP3/Glossary.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/Graphic.o: source/prefix/PrefixANSIDebug.h
source/BP3/Graphic.o: source/BP3/MissingMacTypes.h 
source/BP3/Graphic.o: source/BP3/MissingMacPrototypes.h
source/BP3/Graphic.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Graphic.o: source/BP3/midi1.h
source/BP3/Graphic.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/HTML.o: source/prefix/PrefixANSIDebug.h
source/BP3/HTML.o: source/BP3/MissingMacTypes.h 
source/BP3/HTML.o: source/BP3/MissingMacPrototypes.h
source/BP3/HTML.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/HTML.o: source/BP3/midi1.h source/BP3/-BP2.proto.h
source/BP3/HTML.o: source/BP3/-BP2decl.h
source/BP3/Inits.o: source/prefix/PrefixANSIDebug.h
source/BP3/Inits.o: source/BP3/MissingMacTypes.h 
source/BP3/Inits.o: source/BP3/MissingMacPrototypes.h
source/BP3/Inits.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Inits.o: source/BP3/midi1.h source/BP3/-BP2.proto.h
source/BP3/Inits.o: source/BP3/-BP2decl.h source/BP3/StringLists.h
source/BP3/Interface2.o: source/prefix/PrefixANSIDebug.h
source/BP3/Interface2.o: source/BP3/MissingMacTypes.h 
source/BP3/Interface2.o: source/BP3/MissingMacPrototypes.h
source/BP3/Interface2.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Interface2.o: source/BP3/midi1.h
source/BP3/Interface2.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/MakeSound.o: source/prefix/PrefixANSIDebug.h
source/BP3/MakeSound.o: source/BP3/MissingMacTypes.h 
source/BP3/MakeSound.o: source/BP3/MissingMacPrototypes.h
source/BP3/MakeSound.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/MakeSound.o: source/BP3/midi1.h
source/BP3/MakeSound.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/MIDIfiles.o: source/prefix/PrefixANSIDebug.h
source/BP3/MIDIfiles.o: source/BP3/MissingMacTypes.h 
source/BP3/MIDIfiles.o: source/BP3/MissingMacPrototypes.h
source/BP3/MIDIfiles.o: source/BP3/CTextHandles.h 
source/BP3/MIDIfiles.o: source/BP3/-BP2.h
source/BP3/MIDIfiles.o: source/BP3/midi1.h
source/BP3/MIDIfiles.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/MIDIfiles.o: source/BP3/ConsoleMessages.h
source/BP3/MIDIfiles.o: source/BP3/ConsoleGlobals.h
source/BP3/MIDIloads.o: source/prefix/PrefixANSIDebug.h
source/BP3/MIDIloads.o: source/BP3/MissingMacTypes.h 
source/BP3/MIDIloads.o: source/BP3/MissingMacPrototypes.h
source/BP3/MIDIloads.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/MIDIloads.o: source/BP3/midi1.h
source/BP3/MIDIloads.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/MIDIstubs.o: source/prefix/PrefixANSIDebug.h
source/BP3/MIDIstubs.o: source/BP3/MissingMacTypes.h 
source/BP3/MIDIstubs.o: source/BP3/MissingMacPrototypes.h
source/BP3/MIDIstubs.o: source/BP3/CTextHandles.h 
source/BP3/MIDIstubs.o: source/BP3/-BP2.h
source/BP3/MIDIstubs.o: source/BP3/midi1.h
source/BP3/MIDIstubs.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/MIDIstuff.o: source/prefix/PrefixANSIDebug.h
source/BP3/MIDIstuff.o: source/BP3/MissingMacTypes.h 
source/BP3/MIDIstuff.o: source/BP3/MissingMacPrototypes.h
source/BP3/MIDIstuff.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/MIDIstuff.o: source/BP3/midi1.h
source/BP3/MIDIstuff.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/Misc.o: source/prefix/PrefixANSIDebug.h
source/BP3/Misc.o: source/BP3/MissingMacTypes.h 
source/BP3/Misc.o: source/BP3/MissingMacPrototypes.h
source/BP3/Misc.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Misc.o: source/BP3/midi1.h source/BP3/-BP2.proto.h
source/BP3/Misc.o: source/BP3/-BP2decl.h
source/BP3/PlayThings.o: source/prefix/PrefixANSIDebug.h
source/BP3/PlayThings.o: source/BP3/MissingMacTypes.h 
source/BP3/PlayThings.o: source/BP3/MissingMacPrototypes.h
source/BP3/PlayThings.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/PlayThings.o: source/BP3/midi1.h
source/BP3/PlayThings.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/Polymetric.o: source/prefix/PrefixANSIDebug.h
source/BP3/Polymetric.o: source/BP3/MissingMacTypes.h 
source/BP3/Polymetric.o: source/BP3/MissingMacPrototypes.h
source/BP3/Polymetric.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Polymetric.o: source/BP3/midi1.h
source/BP3/Polymetric.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/ProduceItems.o: source/prefix/PrefixANSIDebug.h
source/BP3/ProduceItems.o: source/BP3/MissingMacTypes.h 
source/BP3/ProduceItems.o: source/BP3/MissingMacPrototypes.h
source/BP3/ProduceItems.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/ProduceItems.o: source/BP3/midi1.h
source/BP3/ProduceItems.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/SaveLoads1.o: source/prefix/PrefixANSIDebug.h
source/BP3/SaveLoads1.o: source/BP3/MissingMacTypes.h 
source/BP3/SaveLoads1.o: source/BP3/MissingMacPrototypes.h
source/BP3/SaveLoads1.o: source/BP3/CTextHandles.h 
source/BP3/SaveLoads1.o: source/BP3/-BP2.h
source/BP3/SaveLoads1.o: source/BP3/midi1.h
source/BP3/SaveLoads1.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/SaveLoads3.o: source/prefix/PrefixANSIDebug.h
source/BP3/SaveLoads3.o: source/BP3/MissingMacTypes.h 
source/BP3/SaveLoads3.o: source/BP3/MissingMacPrototypes.h
source/BP3/SaveLoads3.o: source/BP3/CTextHandles.h 
source/BP3/SaveLoads3.o: source/BP3/-BP2.h
source/BP3/SaveLoads3.o: source/BP3/midi1.h
source/BP3/SaveLoads3.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/SaveLoads3.o: source/BP3/ConsoleMessages.h
source/BP3/SetObjectFeatures.o: source/prefix/PrefixANSIDebug.h
source/BP3/SetObjectFeatures.o: source/BP3/MissingMacTypes.h
source/BP3/SetObjectFeatures.o: source/BP3/MissingMacPrototypes.h
source/BP3/SetObjectFeatures.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/SetObjectFeatures.o: source/BP3/midi1.h source/BP3/-BP2.proto.h
source/BP3/SetObjectFeatures.o: source/BP3/-BP2decl.h
source/BP3/SoundObjects2.o: source/prefix/PrefixANSIDebug.h
source/BP3/SoundObjects2.o: source/BP3/MissingMacTypes.h
source/BP3/SoundObjects2.o: source/BP3/MissingMacPrototypes.h
source/BP3/SoundObjects2.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/SoundObjects2.o: source/BP3/midi1.h
source/BP3/SoundObjects2.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/SoundObjects3.o: source/prefix/PrefixANSIDebug.h
source/BP3/SoundObjects3.o: source/BP3/MissingMacTypes.h
source/BP3/SoundObjects3.o: source/BP3/MissingMacPrototypes.h
source/BP3/SoundObjects3.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/SoundObjects3.o: source/BP3/midi1.h
source/BP3/SoundObjects3.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/Strings.o: source/prefix/PrefixANSIDebug.h
source/BP3/Strings.o: source/BP3/MissingMacTypes.h 
source/BP3/Strings.o: source/BP3/MissingMacPrototypes.h
source/BP3/Strings.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Strings.o: source/BP3/midi1.h
source/BP3/Strings.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/Ticks.o: source/prefix/PrefixANSIDebug.h
source/BP3/Ticks.o: source/BP3/MissingMacTypes.h 
source/BP3/Ticks.o: source/BP3/MissingMacPrototypes.h
source/BP3/Ticks.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Ticks.o: source/BP3/midi1.h source/BP3/-BP2.proto.h
source/BP3/Ticks.o: source/BP3/-BP2decl.h
source/BP3/TimeSet.o: source/prefix/PrefixANSIDebug.h
source/BP3/TimeSet.o: source/BP3/MissingMacTypes.h 
source/BP3/TimeSet.o: source/BP3/MissingMacPrototypes.h
source/BP3/TimeSet.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/TimeSet.o: source/BP3/midi1.h
source/BP3/TimeSet.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
source/BP3/TimeSetFunctions.o: source/prefix/PrefixANSIDebug.h
source/BP3/TimeSetFunctions.o: source/BP3/MissingMacTypes.h
source/BP3/TimeSetFunctions.o: source/BP3/MissingMacPrototypes.h
source/BP3/TimeSetFunctions.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/TimeSetFunctions.o: source/BP3/midi1.h source/BP3/-BP2.proto.h
source/BP3/TimeSetFunctions.o: source/BP3/-BP2decl.h
source/BP3/Zouleb.o: source/prefix/PrefixANSIDebug.h
source/BP3/Zouleb.o: source/BP3/MissingMacTypes.h 
source/BP3/Zouleb.o: source/BP3/MissingMacPrototypes.h
source/BP3/Zouleb.o: source/BP3/CTextHandles.h source/BP3/-BP2.h
source/BP3/Zouleb.o: source/BP3/midi1.h
source/BP3/Zouleb.o: source/BP3/-BP2.proto.h source/BP3/-BP2decl.h
