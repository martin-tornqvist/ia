#
# When calling this makefile, the "BUILD" variable can be set to either
# "release", or "debug", e.g.:
#
# > make BUILD=debug
#
# Running "make" alone will build in release mode
#

CXX?=g++
BUILD?=release

# Directiories
SRC_DIR=src
INC_DIR=include
TARGET_DIR=target
ASSETS_DIR=assets

# Includes
INC_DIR_debug=$(INC_DIR)/debugModeIncl
INC_DIR_release=$(INC_DIR)/releaseModeIncl
INCLUDES=-I $(INC_DIR) -I $(INC_DIR_$(BUILD))

#Flags
CXXFLAGS_release=-O2
CXXFLAGS_debug=-O0 -g
CXXFLAGS=-std=c++11 -Wall -Wextra -fno-rtti $(shell sdl-config --cflags) $(CXXFLAGS_$(BUILD))
# For building 32-bit binaries on x86_64 platform
# CXXFLAGS+=-m32 -march=i686
#LDFLAGS=-L/usr/lib/i386-linux-gnu -lSDL -lSDL_image -lSDL_mixer
LDFLAGS=$(shell sdl-config --libs) -lSDL_image -lSDL_mixer

# Output and sources
EXECUTABLE=ia
SOURCES=$(wildcard $(SRC_DIR)/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
DEPENDS=$(SOURCES:.cpp=.d)

# Various bash commands
RM=rm -rf
MV=mv -f
MKDIR=mkdir -p
CP=cp -r
CAT=cat

# Make targets
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS)
	$(RM) $(TARGET_DIR)
	$(MKDIR) $(TARGET_DIR)
	$(MV) $(EXECUTABLE) $(TARGET_DIR)
	$(CP) $(ASSETS_DIR)/* $(TARGET_DIR)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $< -o $@

# Optional auto dependency tracking
-include depends.mk

depends: $(DEPENDS)

%.d:
	$(CXX) -MM $(CXXFLAGS) $(INCLUDES) $(@:.d=.cpp) -MF depends.tmp -MT$(@:.d=.o)
	$(CAT) depends.tmp >> depends.mk
	$(RM) depends.tmp

clean-depends:
	$(RM) depends.mk

# Remove object files
clean:
	$(RM) $(TARGET_DIR) $(OBJECTS) $(EXECUTABLE)

.PHONY: all depends clean clean-depends
