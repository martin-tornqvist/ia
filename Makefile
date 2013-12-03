CC=g++
BUILD=release

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
CFLAGS_release=-O2
CFLAGS_debug=-O0 -g
CFLAGS=-Wall -Wextra $(shell sdl-config --cflags) $(CFLAGS_$(BUILD))
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
	$(CC) $(LDFLAGS) $^ -o $@
	$(RM) $(TARGET_DIR)
	$(MKDIR) $(TARGET_DIR)
	$(MV) $(EXECUTABLE) $(TARGET_DIR)
	$(CP) $(ASSETS_DIR)/* $(TARGET_DIR)

%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

# Optional auto dependency tracking
-include depends.mk

depends: $(DEPENDS)

%.d:
	$(CC) -MM $(CFLAGS) $(INCLUDES) $(@:.d=.cpp) -MF depends.tmp -MT$(@:.d=.o)
	$(CAT) depends.tmp >> depends.mk
	$(RM) depends.tmp

clean-depends:
	$(RM) depends.mk

# Remove object files
clean:
	$(RM) $(TARGET_DIR) $(OBJECTS) $(EXECUTABLE)

.PHONY: all depends clean clean-depends
