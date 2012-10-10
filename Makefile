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
debug : _INCLUDES=-I $(DEBUG_MODE_INC_DIR)
release : _INCLUDES=-I $(RELEASE_MODE_INC_DIR)

#Flags
CFLAGS=-Wall -Wextra $(shell sdl-config --cflags)
LDFLAGS=$(shell sdl-config --libs)

# Output and sources
EXECUTABLE=ia
SOURCES=$(shell ls $(SRC_DIR)/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

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

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)
	$(RM) $(TARGET_DIR)
	$(MKDIR) $(TARGET_DIR)
	$(MV) $(EXECUTABLE) $(TARGET_DIR)
	$(CP) $(ASSETS_DIR)/* $(TARGET_DIR)

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(_INCLUDES) $(SOURCES) -c
	$(MV) *.o ./$(SRC_DIR)/

.PHONY: clean
clean:
	find . -type f -name '*~' | xargs $(RM)
	find . -type f -name '*.o' | xargs $(RM)
	$(RM) $(TARGET_DIR)