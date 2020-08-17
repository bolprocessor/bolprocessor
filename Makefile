SRCDIR = source/BP2
PREFIX = source/prefix/PrefixANSIDebug.h
CC     = gcc
CFLAGS = 
LIBS   = -lm
EXE    = bp

all:  $(EXE)

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

OBJS = \
  $(SRCDIR)/Arithmetic.o \
  $(SRCDIR)/Automata.o \
  $(SRCDIR)/CompileGrammar.o \
  $(SRCDIR)/CompileProcs.o \
  $(SRCDIR)/Compute.o \
  $(SRCDIR)/ConsoleMain.o \
  $(SRCDIR)/ConsoleMemory.o \
  $(SRCDIR)/ConsoleMessages.o \
  $(SRCDIR)/ConsoleStubs.o \
  $(SRCDIR)/Csound.o \
  $(SRCDIR)/CsoundMaths.o \
  $(SRCDIR)/CsoundScoreMake.o \
  $(SRCDIR)/CTextHandles.o \
  $(SRCDIR)/DisplayArg.o \
  $(SRCDIR)/DisplayThings.o \
  $(SRCDIR)/Encode.o \
  $(SRCDIR)/FillPhaseDiagram.o \
  $(SRCDIR)/GetRelease.o \
  $(SRCDIR)/Glossary.o \
  $(SRCDIR)/HTML.o \
  $(SRCDIR)/Inits.o \
  $(SRCDIR)/Interface2.o \
  $(SRCDIR)/MakeSound.o \
  $(SRCDIR)/MIDIfiles.o \
  $(SRCDIR)/MIDIloads.o \
  $(SRCDIR)/MIDIstubs.o \
  $(SRCDIR)/MIDIstuff.o \
  $(SRCDIR)/Misc.o \
  $(SRCDIR)/PlayThings.o \
  $(SRCDIR)/Polymetric.o \
  $(SRCDIR)/ProduceItems.o \
  $(SRCDIR)/SaveLoads1.o \
  $(SRCDIR)/SaveLoads3.o \
  $(SRCDIR)/SetObjectFeatures.o \
  $(SRCDIR)/SoundObjects2.o \
  $(SRCDIR)/SoundObjects3.o \
  $(SRCDIR)/Strings.o \
  $(SRCDIR)/Ticks.o \
  $(SRCDIR)/TimeSet.o \
  $(SRCDIR)/TimeSetFunctions.o \
  $(SRCDIR)/Zouleb.o

%.o : %.c
	$(CC) -c $(CFLAGS) -I $(SRCDIR) -include $(PREFIX)  $< -o $@

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(LIBS)

depend: 
	makedepend -I $(SRCDIR) -include $(PREFIX)  $(SRCS)

clean:
	rm $(EXE)
	rm $(SRCDIR)/*.o

# DO NOT DELETE

source/BP2/Arithmetic.o: source/prefix/PrefixANSIDebug.h
source/BP2/Arithmetic.o: source/BP2/MissingMacTypes.h
source/BP2/Arithmetic.o: source/BP2/MissingMacPrototypes.h
source/BP2/Arithmetic.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Arithmetic.o: source/BP2/midi1.h
source/BP2/Arithmetic.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/Automata.o: source/prefix/PrefixANSIDebug.h
source/BP2/Automata.o: source/BP2/MissingMacTypes.h 
source/BP2/Automata.o: source/BP2/MissingMacPrototypes.h
source/BP2/Automata.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Automata.o: source/BP2/midi1.h
source/BP2/Automata.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/CompileGrammar.o: source/prefix/PrefixANSIDebug.h
source/BP2/CompileGrammar.o: source/BP2/MissingMacTypes.h
source/BP2/CompileGrammar.o: source/BP2/MissingMacPrototypes.h
source/BP2/CompileGrammar.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/CompileGrammar.o: source/BP2/midi1.h
source/BP2/CompileGrammar.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/CompileProcs.o: source/prefix/PrefixANSIDebug.h
source/BP2/CompileProcs.o: source/BP2/MissingMacTypes.h 
source/BP2/CompileProcs.o: source/BP2/MissingMacPrototypes.h
source/BP2/CompileProcs.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/CompileProcs.o: source/BP2/midi1.h
source/BP2/CompileProcs.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/Compute.o: source/prefix/PrefixANSIDebug.h
source/BP2/Compute.o: source/BP2/MissingMacTypes.h 
source/BP2/Compute.o: source/BP2/MissingMacPrototypes.h
source/BP2/Compute.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Compute.o: source/BP2/midi1.h
source/BP2/Compute.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/ConsoleMain.o: source/prefix/PrefixANSIDebug.h
source/BP2/ConsoleMain.o: source/BP2/MissingMacTypes.h 
source/BP2/ConsoleMain.o: source/BP2/MissingMacPrototypes.h
source/BP2/ConsoleMain.o: source/BP2/CTextHandles.h 
source/BP2/ConsoleMain.o: source/BP2/-BP2.h
source/BP2/ConsoleMain.o: source/BP2/midi1.h
source/BP2/ConsoleMain.o: source/BP2/-BP2.proto.h source/BP2/-BP2main.h
source/BP2/ConsoleMain.o: source/BP2/ConsoleMessages.h
source/BP2/ConsoleMain.o: source/BP2/ConsoleGlobals.h
source/BP2/ConsoleMemory.o: source/prefix/PrefixANSIDebug.h
source/BP2/ConsoleMemory.o: source/BP2/MissingMacTypes.h
source/BP2/ConsoleMemory.o: source/BP2/MissingMacPrototypes.h
source/BP2/ConsoleMemory.o: source/BP2/CTextHandles.h 
source/BP2/ConsoleMemory.o: source/BP2/-BP2.h 
source/BP2/ConsoleMemory.o: source/BP2/midi1.h
source/BP2/ConsoleMemory.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/ConsoleMessages.o: source/prefix/PrefixANSIDebug.h
source/BP2/ConsoleMessages.o: source/BP2/MissingMacTypes.h
source/BP2/ConsoleMessages.o: source/BP2/MissingMacPrototypes.h
source/BP2/ConsoleMessages.o: source/BP2/CTextHandles.h 
source/BP2/ConsoleMessages.o: source/BP2/-BP2.h
source/BP2/ConsoleMessages.o: source/BP2/midi1.h
source/BP2/ConsoleMessages.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/ConsoleMessages.o: source/BP2/ConsoleMessages.h
source/BP2/ConsoleStubs.o: source/prefix/PrefixANSIDebug.h
source/BP2/ConsoleStubs.o: source/BP2/MissingMacTypes.h 
source/BP2/ConsoleStubs.o: source/BP2/MissingMacPrototypes.h
source/BP2/ConsoleStubs.o: source/BP2/CTextHandles.h 
source/BP2/ConsoleStubs.o: source/BP2/-BP2.h
source/BP2/ConsoleStubs.o: source/BP2/midi1.h
source/BP2/ConsoleStubs.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/ConsoleStubs.o: source/BP2/ConsoleMessages.h
source/BP2/ConsoleStubs.o: source/BP2/ConsoleGlobals.h
source/BP2/Csound.o: source/prefix/PrefixANSIDebug.h
source/BP2/Csound.o: source/BP2/MissingMacTypes.h 
source/BP2/Csound.o: source/BP2/MissingMacPrototypes.h
source/BP2/Csound.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Csound.o: source/BP2/midi1.h
source/BP2/Csound.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/CsoundMaths.o: source/prefix/PrefixANSIDebug.h
source/BP2/CsoundMaths.o: source/BP2/MissingMacTypes.h 
source/BP2/CsoundMaths.o: source/BP2/MissingMacPrototypes.h
source/BP2/CsoundMaths.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/CsoundMaths.o: source/BP2/midi1.h
source/BP2/CsoundMaths.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/CsoundScoreMake.o: source/prefix/PrefixANSIDebug.h
source/BP2/CsoundScoreMake.o: source/BP2/MissingMacTypes.h
source/BP2/CsoundScoreMake.o: source/BP2/MissingMacPrototypes.h
source/BP2/CsoundScoreMake.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/CsoundScoreMake.o: source/BP2/midi1.h
source/BP2/CsoundScoreMake.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/CTextHandles.o: source/prefix/PrefixANSIDebug.h
source/BP2/CTextHandles.o: source/BP2/MissingMacTypes.h 
source/BP2/CTextHandles.o: source/BP2/MissingMacPrototypes.h
source/BP2/CTextHandles.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/CTextHandles.o: source/BP2/midi1.h
source/BP2/CTextHandles.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/DisplayArg.o: source/prefix/PrefixANSIDebug.h
source/BP2/DisplayArg.o: source/BP2/MissingMacTypes.h 
source/BP2/DisplayArg.o: source/BP2/MissingMacPrototypes.h
source/BP2/DisplayArg.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/DisplayArg.o: source/BP2/midi1.h
source/BP2/DisplayArg.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/DisplayThings.o: source/prefix/PrefixANSIDebug.h
source/BP2/DisplayThings.o: source/BP2/MissingMacTypes.h
source/BP2/DisplayThings.o: source/BP2/MissingMacPrototypes.h
source/BP2/DisplayThings.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/DisplayThings.o: source/BP2/midi1.h
source/BP2/DisplayThings.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/Encode.o: source/prefix/PrefixANSIDebug.h
source/BP2/Encode.o: source/BP2/MissingMacTypes.h 
source/BP2/Encode.o: source/BP2/MissingMacPrototypes.h
source/BP2/Encode.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Encode.o: source/BP2/midi1.h
source/BP2/Encode.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/FillPhaseDiagram.o: source/prefix/PrefixANSIDebug.h
source/BP2/FillPhaseDiagram.o: source/BP2/MissingMacTypes.h
source/BP2/FillPhaseDiagram.o: source/BP2/MissingMacPrototypes.h
source/BP2/FillPhaseDiagram.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/FillPhaseDiagram.o: source/BP2/midi1.h source/BP2/-BP2.proto.h
source/BP2/FillPhaseDiagram.o: source/BP2/-BP2decl.h
source/BP2/GetRelease.o: source/prefix/PrefixANSIDebug.h
source/BP2/GetRelease.o: source/BP2/MissingMacTypes.h 
source/BP2/GetRelease.o: source/BP2/MissingMacPrototypes.h
source/BP2/GetRelease.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/GetRelease.o: source/BP2/midi1.h
source/BP2/GetRelease.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/Glossary.o: source/prefix/PrefixANSIDebug.h
source/BP2/Glossary.o: source/BP2/MissingMacTypes.h 
source/BP2/Glossary.o: source/BP2/MissingMacPrototypes.h
source/BP2/Glossary.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Glossary.o: source/BP2/midi1.h
source/BP2/Glossary.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/HTML.o: source/prefix/PrefixANSIDebug.h
source/BP2/HTML.o: source/BP2/MissingMacTypes.h 
source/BP2/HTML.o: source/BP2/MissingMacPrototypes.h
source/BP2/HTML.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/HTML.o: source/BP2/midi1.h source/BP2/-BP2.proto.h
source/BP2/HTML.o: source/BP2/-BP2decl.h
source/BP2/Inits.o: source/prefix/PrefixANSIDebug.h
source/BP2/Inits.o: source/BP2/MissingMacTypes.h 
source/BP2/Inits.o: source/BP2/MissingMacPrototypes.h
source/BP2/Inits.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Inits.o: source/BP2/midi1.h source/BP2/-BP2.proto.h
source/BP2/Inits.o: source/BP2/-BP2decl.h source/BP2/StringLists.h
source/BP2/Interface2.o: source/prefix/PrefixANSIDebug.h
source/BP2/Interface2.o: source/BP2/MissingMacTypes.h 
source/BP2/Interface2.o: source/BP2/MissingMacPrototypes.h
source/BP2/Interface2.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Interface2.o: source/BP2/midi1.h
source/BP2/Interface2.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/MakeSound.o: source/prefix/PrefixANSIDebug.h
source/BP2/MakeSound.o: source/BP2/MissingMacTypes.h 
source/BP2/MakeSound.o: source/BP2/MissingMacPrototypes.h
source/BP2/MakeSound.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/MakeSound.o: source/BP2/midi1.h
source/BP2/MakeSound.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/MIDIfiles.o: source/prefix/PrefixANSIDebug.h
source/BP2/MIDIfiles.o: source/BP2/MissingMacTypes.h 
source/BP2/MIDIfiles.o: source/BP2/MissingMacPrototypes.h
source/BP2/MIDIfiles.o: source/BP2/CTextHandles.h 
source/BP2/MIDIfiles.o: source/BP2/-BP2.h
source/BP2/MIDIfiles.o: source/BP2/midi1.h
source/BP2/MIDIfiles.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/MIDIfiles.o: source/BP2/ConsoleMessages.h
source/BP2/MIDIfiles.o: source/BP2/ConsoleGlobals.h
source/BP2/MIDIloads.o: source/prefix/PrefixANSIDebug.h
source/BP2/MIDIloads.o: source/BP2/MissingMacTypes.h 
source/BP2/MIDIloads.o: source/BP2/MissingMacPrototypes.h
source/BP2/MIDIloads.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/MIDIloads.o: source/BP2/midi1.h
source/BP2/MIDIloads.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/MIDIstubs.o: source/prefix/PrefixANSIDebug.h
source/BP2/MIDIstubs.o: source/BP2/MissingMacTypes.h 
source/BP2/MIDIstubs.o: source/BP2/MissingMacPrototypes.h
source/BP2/MIDIstubs.o: source/BP2/CTextHandles.h 
source/BP2/MIDIstubs.o: source/BP2/-BP2.h
source/BP2/MIDIstubs.o: source/BP2/midi1.h
source/BP2/MIDIstubs.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/MIDIstuff.o: source/prefix/PrefixANSIDebug.h
source/BP2/MIDIstuff.o: source/BP2/MissingMacTypes.h 
source/BP2/MIDIstuff.o: source/BP2/MissingMacPrototypes.h
source/BP2/MIDIstuff.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/MIDIstuff.o: source/BP2/midi1.h
source/BP2/MIDIstuff.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/Misc.o: source/prefix/PrefixANSIDebug.h
source/BP2/Misc.o: source/BP2/MissingMacTypes.h 
source/BP2/Misc.o: source/BP2/MissingMacPrototypes.h
source/BP2/Misc.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Misc.o: source/BP2/midi1.h source/BP2/-BP2.proto.h
source/BP2/Misc.o: source/BP2/-BP2decl.h
source/BP2/PlayThings.o: source/prefix/PrefixANSIDebug.h
source/BP2/PlayThings.o: source/BP2/MissingMacTypes.h 
source/BP2/PlayThings.o: source/BP2/MissingMacPrototypes.h
source/BP2/PlayThings.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/PlayThings.o: source/BP2/midi1.h
source/BP2/PlayThings.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/Polymetric.o: source/prefix/PrefixANSIDebug.h
source/BP2/Polymetric.o: source/BP2/MissingMacTypes.h 
source/BP2/Polymetric.o: source/BP2/MissingMacPrototypes.h
source/BP2/Polymetric.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Polymetric.o: source/BP2/midi1.h
source/BP2/Polymetric.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/ProduceItems.o: source/prefix/PrefixANSIDebug.h
source/BP2/ProduceItems.o: source/BP2/MissingMacTypes.h 
source/BP2/ProduceItems.o: source/BP2/MissingMacPrototypes.h
source/BP2/ProduceItems.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/ProduceItems.o: source/BP2/midi1.h
source/BP2/ProduceItems.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/SaveLoads1.o: source/prefix/PrefixANSIDebug.h
source/BP2/SaveLoads1.o: source/BP2/MissingMacTypes.h 
source/BP2/SaveLoads1.o: source/BP2/MissingMacPrototypes.h
source/BP2/SaveLoads1.o: source/BP2/CTextHandles.h 
source/BP2/SaveLoads1.o: source/BP2/-BP2.h
source/BP2/SaveLoads1.o: source/BP2/midi1.h
source/BP2/SaveLoads1.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/SaveLoads3.o: source/prefix/PrefixANSIDebug.h
source/BP2/SaveLoads3.o: source/BP2/MissingMacTypes.h 
source/BP2/SaveLoads3.o: source/BP2/MissingMacPrototypes.h
source/BP2/SaveLoads3.o: source/BP2/CTextHandles.h 
source/BP2/SaveLoads3.o: source/BP2/-BP2.h
source/BP2/SaveLoads3.o: source/BP2/midi1.h
source/BP2/SaveLoads3.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/SaveLoads3.o: source/BP2/ConsoleMessages.h
source/BP2/SetObjectFeatures.o: source/prefix/PrefixANSIDebug.h
source/BP2/SetObjectFeatures.o: source/BP2/MissingMacTypes.h
source/BP2/SetObjectFeatures.o: source/BP2/MissingMacPrototypes.h
source/BP2/SetObjectFeatures.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/SetObjectFeatures.o: source/BP2/midi1.h source/BP2/-BP2.proto.h
source/BP2/SetObjectFeatures.o: source/BP2/-BP2decl.h
source/BP2/SoundObjects2.o: source/prefix/PrefixANSIDebug.h
source/BP2/SoundObjects2.o: source/BP2/MissingMacTypes.h
source/BP2/SoundObjects2.o: source/BP2/MissingMacPrototypes.h
source/BP2/SoundObjects2.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/SoundObjects2.o: source/BP2/midi1.h
source/BP2/SoundObjects2.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/SoundObjects3.o: source/prefix/PrefixANSIDebug.h
source/BP2/SoundObjects3.o: source/BP2/MissingMacTypes.h
source/BP2/SoundObjects3.o: source/BP2/MissingMacPrototypes.h
source/BP2/SoundObjects3.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/SoundObjects3.o: source/BP2/midi1.h
source/BP2/SoundObjects3.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/Strings.o: source/prefix/PrefixANSIDebug.h
source/BP2/Strings.o: source/BP2/MissingMacTypes.h 
source/BP2/Strings.o: source/BP2/MissingMacPrototypes.h
source/BP2/Strings.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Strings.o: source/BP2/midi1.h
source/BP2/Strings.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/Ticks.o: source/prefix/PrefixANSIDebug.h
source/BP2/Ticks.o: source/BP2/MissingMacTypes.h 
source/BP2/Ticks.o: source/BP2/MissingMacPrototypes.h
source/BP2/Ticks.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Ticks.o: source/BP2/midi1.h source/BP2/-BP2.proto.h
source/BP2/Ticks.o: source/BP2/-BP2decl.h
source/BP2/TimeSet.o: source/prefix/PrefixANSIDebug.h
source/BP2/TimeSet.o: source/BP2/MissingMacTypes.h 
source/BP2/TimeSet.o: source/BP2/MissingMacPrototypes.h
source/BP2/TimeSet.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/TimeSet.o: source/BP2/midi1.h
source/BP2/TimeSet.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
source/BP2/TimeSetFunctions.o: source/prefix/PrefixANSIDebug.h
source/BP2/TimeSetFunctions.o: source/BP2/MissingMacTypes.h
source/BP2/TimeSetFunctions.o: source/BP2/MissingMacPrototypes.h
source/BP2/TimeSetFunctions.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/TimeSetFunctions.o: source/BP2/midi1.h source/BP2/-BP2.proto.h
source/BP2/TimeSetFunctions.o: source/BP2/-BP2decl.h
source/BP2/Zouleb.o: source/prefix/PrefixANSIDebug.h
source/BP2/Zouleb.o: source/BP2/MissingMacTypes.h 
source/BP2/Zouleb.o: source/BP2/MissingMacPrototypes.h
source/BP2/Zouleb.o: source/BP2/CTextHandles.h source/BP2/-BP2.h
source/BP2/Zouleb.o: source/BP2/midi1.h
source/BP2/Zouleb.o: source/BP2/-BP2.proto.h source/BP2/-BP2decl.h
