SRCDIR = source/BP3
CC     = gcc
LIBS   = -lm
EXE    = bp

UNAME_S := Windows
UNAME_M := x86_64

# Detect operating system
ifeq ($(OS),Windows_NT)
    UNAME_S := Windows
    CFLAGS =
    FRAMEWORKS = -lwinmm -Wall -Wextra
    EXE = bp.exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        CFLAGS = -fsanitize=address -fsanitize-recover=address -g
        FRAMEWORKS = -framework CoreMIDI -framework CoreFoundation
    endif
    ifeq ($(UNAME_S),Linux)
        CFLAGS = -fsanitize=address -fsanitize-recover=address -g
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
ifeq ($(OS),Windows_NT)
	$(CC) -g -o $(EXE) $(OBJS) $(LIBS) $(FRAMEWORKS)
else
	$(CC) -fsanitize=address -fsanitize-recover=address -g -o $(EXE) $(OBJS) $(LIBS) $(FRAMEWORKS)
endif

%.o : %.c
ifeq ($(OS),Windows_NT)
	$(CC) -g -c $(CFLAGS) -I $(SRCDIR) $< -o $@
else
	$(CC) -fsanitize=address -fsanitize-recover=address -g -c $(CFLAGS) -I $(SRCDIR) $< -o $@
endif

depend: 
	makedepend -I $(SRCDIR)  $(SRCS)

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
