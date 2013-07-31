# Infra Arcana Makefile

# optimized build; default is debug build
# RELEASE = 1

# build for macosx w/ sdl frameworks
# TARGETOS = macosx

# clang
# C = cc
# CXX = c++

# gcc
# CC = gcc
# CXX = g++

# Directories
SRC_DIR = ./src
OBJ_DIR = ./obj
DEBUG_INC_DIR = $(SRC_DIR)/debugModeIncl
RELEASE_INC_DIR = $(SRC_DIR)/releaseModeIncl
INSTALL_DIR = ./build
ASSETS_DIR = ./assets
OSX_DIR = ./osx

# Output and sources
TARGET = ia
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))

# check for sdl-config in PATH
ifndef FRAMEWORK
  SDLCONFIG_PATH := $(shell sdl-config --prefix 2>/dev/null 1>&2; echo $$?)
  ifneq ($(SDLCONFIG_PATH),0)
    $(error sdl-config not in PATH) 
  endif
endif

# Flags
# using '_EXTRA' to be able to 'export CXXFLAGS=-my-option'
# from the command line and including them when building, maybe
# this isn't the best way...
CFLAGS_EXTRA = $(shell sdl-config --cflags)
WARNINGS = -Wall -Wextra
OPTFLAGS = -O0 -g
LDFLAGS_EXTRA = $(shell sdl-config --libs) -lSDL -lSDL_image
BUILD_INC = -I$(DEBUG_INC_DIR)

# optimized build
ifdef RELEASE
  OPTFLAGS = -O2
  BUILD_INC = -I$(RELEASE_INC_DIR)
endif

# supress clang warnings
ifeq ($(CC),cc)
  WARNINGS += -Wno-mismatched-tags
endif

# OSX
ifeq ($(TARGETOS),macosx)
  OSX_MIN = 10.6
  # ARCH = -arch i386 -arch x86_64
  DEFS = -DMACOSX
  ifdef FRAMEWORK
    DEFS += -DOSX_SDL_FW
    CFLAGS_EXTRA = -F/Library/Frameworks \
		   -F$(HOME)/Library/Frameworks \
		   -I/Library/Frameworks/SDL.framework/Headers \
		   -I$(HOME)/Library/Frameworks/SDL.framework/Headers \
		   -I/Library/Frameworks/SDL_image.framework/Headers \
		   -I$(HOME)/Library/Frameworks/SDL_image.framework/Headers
    OPTFLAGS += $(ARCH) -mmacosx-version-min=$(OSX_MIN)
    LDFLAGS_EXTRA = -F/Library/Frameworks \
		    -F$(HOME)/Library/Frameworks \
		    -framework SDL -framework SDL_image -framework Cocoa $(ARCH)
  else
    DEFS += -DOSX_SDL_LIBS
    # headers could be in /path/include/ or /path/include/SDL/ (macports)
    CFLAGS_EXTRA += -I$(shell dirname $(shell sdl-config --cflags | sed 's/-I\(.[^ ]*\) .*/\1/'))
    CXXFLAGS += $(ARCH) -mmacosx-version-min=$(OSX_MIN)
  endif
  OBJECTS += $(OBJ_DIR)/SDLMain.o
endif

CXXFLAGS += $(WARNINGS) $(OPTFLAGS) $(BUILD_INC) $(CFLAGS_EXTRA) $(DEFS)
LDFLAGS += $(LDFLAGS_EXTRA)

all: $(OBJ_DIR)/$(TARGET)

$(OBJ_DIR)/$(TARGET): $(OBJ_DIR) $(OBJECTS)
	$(CXX) -o $@ $(DEFS) $(OBJECTS) $(LDFLAGS)
	rm -rf $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)
	cp -R $(ASSETS_DIR)/ $(INSTALL_DIR)/
	cp $(OBJ_DIR)/$(TARGET) $(INSTALL_DIR)
	@echo \`cd $(INSTALL_DIR)\; .\/$(TARGET)\` to run $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(OSX_DIR)/%.m
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)

uninstall:
	rm -rf $(INSTALL_DIR)

.PHONY: all clean uninstall
