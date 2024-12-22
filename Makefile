SRCDIR = source/BP3
PREFIX = source/BP3/PrefixANSIDebug.h
CC     = gcc
LIBS   = -lm
EXE    = bp

UNAME_S ?= Ubuntu
UNAME_M ?= x86_64

# Detect operating system
ifeq ($(OS),Windows_NT)
	CFLAGS =
	FRAMEWORKS = -lwinmm -Wall -Wextra
	EXE = bp.exe
else
	ifeq ($(UNAME_S),Darwin)
		CFLAGS =
		FRAMEWORKS = -framework CoreMIDI -framework CoreFoundation
	endif
	ifeq ($(UNAME_S),Linux)
		CFLAGS =
		FRAMEWORKS = -lasound
		EXE = bp3
	endif
endif

# Print the operating system detected (optional, for debugging)
$(info Operating System: $(UNAME_S))

SRCS = $(wildcard $(SRCDIR)/*.c)

OBJS = $(SRCS:.c=.o)

all:  $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LIBS) $(FRAMEWORKS)

%.o : %.c
	$(CC) $(CFLAGS) -c -I$(SRCDIR) -include $(PREFIX) $< -o $@

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
