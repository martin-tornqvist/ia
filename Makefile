CC=g++

# Directiories
SRC_DIR=src
DEBUG_MODE_INC_DIR=$(SRC_DIR)/debugModeIncl
RELEASE_MODE_INC_DIR=$(SRC_DIR)/releaseModeIncl
TARGET_DIR=target
ASSETS_DIR=assets

# Includes
INCLUDES=

# Target specific include files
_INCLUDES=
_CFLAGS=
debug : _INCLUDES=-I $(DEBUG_MODE_INC_DIR)
debug : _CFLAGS=-O0 -g
release : _INCLUDES=-I $(RELEASE_MODE_INC_DIR)
release : _CFLAGS=-O2

#Flags
CFLAGS=-Wall -Wextra $(shell sdl-config --cflags)
LDFLAGS=$(shell sdl-config --libs) -lSDL_image

# Output and sources
EXECUTABLE=ia
SOURCES=$(shell ls $(SRC_DIR)/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
OBJECTS_STAMP_FILE=obj_stamp

# Various bash commands
RM=rm -rf
MV=mv -f
MKDIR=mkdir -p
CP=cp -r

# Dependencies 
debug: $(SOURCES) $(EXECUTABLE)
release: $(SOURCES) $(EXECUTABLE)

# Make targets
.DEFAULT_GOAL=default_target
.PHONY: default_target
default_target:
	@echo "Use \"make debug\" or \"make release\""

.PHONY: $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS_STAMP_FILE)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)
	$(RM) $(TARGET_DIR)
	$(MKDIR) $(TARGET_DIR)
	$(MV) $(EXECUTABLE) $(TARGET_DIR)
	$(CP) $(ASSETS_DIR)/* $(TARGET_DIR)

.PHONY: $(OBJECTS_STAMP_FILE)
$(OBJECTS_STAMP_FILE): $(SOURCES)
	find . -type f -name '*.o' | xargs $(RM)
	$(CC) $(CFLAGS) $(_CFLAGS) $(INCLUDES) $(_INCLUDES) $(SOURCES) -c
	$(MV) *.o ./$(SRC_DIR)/

.PHONY: clean
clean:
	find . -type f -name '*~' | xargs $(RM)
	find . -type f -name '*.o' | xargs $(RM)
	$(RM) $(TARGET_DIR)
