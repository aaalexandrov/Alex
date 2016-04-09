#

CC = g++

CFLAGS = -std=c++11 
LFLAGS =
LIBS =

ifeq ($(OS),Windows_NT)
	CFLAGS += -DWINVER=0x0500
	OSDIR = Windows
	ifeq ($(CC), clang++)
		CFLAGS += -target x86_64-w64-mingw32
	endif
else
	OSDIR = Linux
endif

CONFIG ?= Debug

ifeq ($(CONFIG), Debug)
	CFLAGS += -g -D_DEBUG
else
	CONFIG = Release
	CFLAGS += -O2
	LFLAGS += -s
endif

SRCDIRS = Src ../Framework/Base ../Framework/OS ../Framework/OS/$(OSDIR)
INCLUDES = $(foreach dir,$(SRCDIRS),-I../$(dir))

OUTDIR = ../Output/bin/$(CONFIG)
OUTNAME = SOS
OUTPUT = $(OUTDIR)/$(OUTNAME).exe
OBJDIR = ../Output/obj/$(CONFIG)/$(OUTNAME)
OBJDIRS = $(SRCDIRS:%=$(OBJDIR)/%/)

SRCS := $(foreach dir,$(SRCDIRS),$(wildcard ../$(dir)/*.cpp))
HDRS := $(foreach dir,$(SRCDIRS),$(wildcard ../$(dir)/*.h))
OBJS = $(SRCS:../%.cpp=$(OBJDIR)/%.o)
DEPS = $(OBJS:%.o=%.d)

build: $(OUTPUT)

$(DEPS): $(OBJDIR)/%.d: ../%.cpp $(HDRS) | $(OBJDIRS)
	$(CC) $(CFLAGS) $(INCLUDES) -MM $< -MT $(patsubst %.d,%.o,$@) -MF $@

$(OBJDIRS):  
	mkdir -p $@
	
$(OUTPUT): $(OBJS) | $(OUTDIR)
	$(CC) $(LFLAGS) -o $(OUTPUT) $(OBJS) $(LIBS)
	
$(OUTDIR):
	mkdir -p $(OUTDIR)
	
$(OBJS): $(OBJDIR)/%.o: ../%.cpp | $(OBJDIRS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean

clean:
	rm -rf ../Output/*

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
	