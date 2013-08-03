SHELL=/bin/sh

# optimized build; default is debug build
# RELEASE = 1

# build for macosx
# TARGETOS = macosx

# use -lSDL -lSDL_image for macosx; default uses frameworks
# OSXLIBSDL = 1

# clang
# C ?= cc
# CXX ?= c++

# gcc
# CC ?= gcc
# CXX ?= g++

# Directories
SRCDIR = ./src
OBJDIR = ./obj
DEBUGDIR = $(SRCDIR)/debugModeIncl
RELEASEDIR = $(SRCDIR)/releaseModeIncl
ASSETSDIR = ./assets
OSXDIR = ./osx
BINDIR = ./build

# Output and sources
PROG = ia
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

# Flags
WARNINGS = -Wall -Wextra
OPTFLAGS = -O0 -g -pipe
INCLUDES = $(shell sdl-config --cflags)
BUILDINC = -I$(DEBUGDIR)
LIBS = $(shell sdl-config --libs) -lSDL -lSDL_image

# optimized build
ifdef RELEASE
  OPTFLAGS = -O2 -pipe
  BUILDINC = -I$(RELEASEDIR)
endif

# supress clang warnings
ifeq ($(CXX),c++)
  WARNINGS += -Wno-mismatched-tags
endif

# OSX
ifeq ($(TARGETOS),macosx)
  OSXMIN ?= 10.6
  DEFS = -DMACOSX
  OPTFLAGS += -mmacosx-version-min=$(OSXMIN)
  OBJECTS += $(OBJDIR)/SDLMain.o
  ifdef OSXLIBSDL
    DEFS += -DOSX_SDL_LIBS
    # handle #include "SDL/SDL.h" and "SDL.h", etc.
    INCLUDES += -I$(shell dirname $(shell sdl-config --cflags | sed 's/-I\(.[^ ]*\) .*/\1/'))
  else
    OSXFW = 1
    DEFS += -DOSX_SDL_FW
    INCLUDES = -F/Library/Frameworks \
	       -F$(HOME)/Library/Frameworks \
	       -I/Library/Frameworks/SDL.framework/Headers \
	       -I$(HOME)/Library/Frameworks/SDL.framework/Headers \
	       -I/Library/Frameworks/SDL_image.framework/Headers \
	       -I$(HOME)/Library/Frameworks/SDL_image.framework/Headers
    LIBS = -F/Library/Frameworks \
	   -F$(HOME)/Library/Frameworks \
	   -framework SDL -framework SDL_image -framework Cocoa
  endif
endif

# check for sdl-config in PATH
ifndef OSXFW
  SDLCFG := $(shell sdl-config --prefix 2>/dev/null 1>&2; echo $$?)
  ifneq ($(SDLCFG),0)
    $(error sdl-config not in PATH) 
  endif
endif

CFLAGS_EXTRA = $(WARNINGS) $(OPTFLAGS) $(INCLUDES) $(BUILDINC) $(DEFS)

all: $(OBJDIR)/$(PROG)

$(OBJDIR)/$(PROG): $(OBJDIR) $(OBJECTS)
	$(CXX) -o $@ $(DEFS) $(OBJECTS) $(LIBS) $(LDFLAGS)
	rm -rf $(BINDIR)
	mkdir -p $(BINDIR)
	cp -R $(ASSETSDIR)/ $(BINDIR)
	cp $(OBJDIR)/$(PROG) $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS_EXTRA) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(OSXDIR)/%.m
	$(CC) $(CFLAGS_EXTRA) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)
	rm -f $(PROG)

uninstall:
	rm -rf $(BINDIR)

.PHONY: all clean uninstall
