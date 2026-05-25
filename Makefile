# === Configuration ===
SRCDIR  = source/BP3
CC      = gcc
LIBS    = -lm
CFLAGS  = -O2 -fno-common
EXE     = bp

# Avoid wrong libraries inherited from PHP/Apache/XAMPP
unexport LD_LIBRARY_PATH

# === OS Detection ===
ifeq ($(OS),Windows_NT)
    UNAME_S := Windows
    FRAMEWORKS = -lwinmm
    EXE = bp.exe

    CURL_DIR = C:/curl
    CFLAGS += -I$(CURL_DIR)/include
    LIBS += -L$(CURL_DIR)/lib -l:libcurl-x64.dll -lws2_32 -lcrypt32

else
    UNAME_S := $(shell uname -s)

    ifeq ($(UNAME_S),Darwin)
        FRAMEWORKS = -framework CoreMIDI -framework CoreFoundation
        CURL_CONFIG = /usr/bin/curl-config
        CURL_CFLAGS := $(shell $(CURL_CONFIG) --cflags)
        CURL_LIBS   := $(shell $(CURL_CONFIG) --libs)
        CFLAGS += $(CURL_CFLAGS)
        LIBS   += $(CURL_LIBS)
    endif

    ifeq ($(UNAME_S),Linux)
        FRAMEWORKS = -lasound
        EXE = bp3
        CURL_CONFIG = /usr/bin/curl-config
        CURL_CFLAGS := $(shell $(CURL_CONFIG) --cflags)
        CURL_LIBS   := $(shell $(CURL_CONFIG) --libs)
        CFLAGS += $(CURL_CFLAGS)
        LIBS   += -L/usr/lib/x86_64-linux-gnu $(CURL_LIBS)
    endif
endif

# === Source and Object Files ===
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:.c=.o)

# === Output OS Info ===
$(info Operating System: $(UNAME_S))
$(info CFLAGS: $(CFLAGS))
$(info LIBS: $(LIBS))

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