# === Configuration ===
SRCDIR  = source/BP3
CC      = gcc
LIBS    = -lm
CFLAGS  = -O2 -fno-common -MMD -MP
EXE     = bp

# make            : optimised build (-O2), with debug symbols (-g does not slow the program)
# make ASAN=1     : AddressSanitizer build (-O0), slow, for hunting memory errors
# make WARN=1     : optimised build that also prints compiler warnings (diagnostics only)
#   Always "make clean" when switching between these modes.
ASAN ?= 0
WARN ?= 0

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

# Debug flags shared by macOS and Linux
    ifeq ($(ASAN),1)
        CFLAGS = -fsanitize=address -fsanitize-recover=address -g -O0 -fno-common -MMD -MP
    else
        CFLAGS += -g -fno-omit-frame-pointer
    endif

    ifeq ($(WARN),1)
        CFLAGS += -Wall -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label
    endif

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
DEPS = $(OBJS:.o=.d)

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

# Header dependencies written by -MMD (one .d file per .o).
# The "-" means: no error on the first build, when they don't exist yet.
-include $(DEPS)

clean:
ifeq ($(UNAME_S),Darwin)
	-rm -f $(EXE)
	-rm -f $(SRCDIR)/*.o $(SRCDIR)/*.d
else ifeq ($(UNAME_S),Linux)
	-rm -f $(EXE)
	-rm -f $(SRCDIR)/*.o $(SRCDIR)/*.d
else ifeq ($(OS),Windows_NT)
	@if exist $(EXE) (echo Deleting $(EXE) & del /Q $(EXE))
	@if exist $(subst /,\,$(SRCDIR))\*.o (echo Deleting object files & del /Q $(subst /,\,$(SRCDIR))\*.o)
	@if exist $(subst /,\,$(SRCDIR))\*.d (del /Q $(subst /,\,$(SRCDIR))\*.d)
endif