# === Configuration ===
SRCDIR  = source/BP3
CC      = gcc
LIBS    = -lm
CFLAGS  = -O2 -fno-common
EXE     = bp

UNAME_S := Windows

# === OS Detection ===
ifeq ($(OS),Windows_NT)
    UNAME_S := Windows
    FRAMEWORKS = -lwinmm
    EXE = bp.exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        FRAMEWORKS = -framework CoreMIDI -framework CoreFoundation
    endif
    ifeq ($(UNAME_S),Linux)
        FRAMEWORKS = -lasound
        EXE = bp3
    endif
endif

# === Source and Object Files ===
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:.c=.o)

# === Output OS Info ===
$(info Operating System: $(UNAME_S))

# === Build Rules ===
all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LIBS) $(FRAMEWORKS)

%.o : %.c
	$(CC) $(CFLAGS) -c -I$(SRCDIR) $< -o $@

depend:
	makedepend -I$(SRCDIR) $(SRCS)

clean:
ifeq ($(UNAME_S),Darwin)
	-rm -f $(EXE)
	-rm -f $(SRCDIR)/*.o
else ifeq ($(UNAME_S),Linux)
	-rm -f $(EXE)
	-rm -f $(SRCDIR)/*.o
else ifeq ($(OS),Windows_NT)
	@if exist $(EXE) (echo Deleting $(EXE) & del /Q $(EXE))
	@if exist $(subst /,\,$(SRCDIR))\*.o (echo Deleting object files & del /Q $(subst /,\,$(SRCDIR))\*.o)
endif
