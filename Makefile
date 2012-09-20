CC=g++

# Directiories
SRC_DIR=src
DEBUG_MODE_INC_DIR=$(SRC_DIR)/debugModeIncl
RELEASE_MODE_INC_DIR=$(SRC_DIR)/releaseModeIncl
TARGET_DIR=target
ASSETS_DIR=assets

#Flags
CFLAGS=-Wall -Wextra $(shell sdl-config --cflags)
LDFLAGS=$(shell sdl-config --libs)

# Output and sources
EXECUTABLE=ia
SOURCES=$(shell ls $(SRC_DIR)/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

# Various bash commands
RM_CMD=rm -rf
MV_CMD=mv -f
MKDIR_CMD=mkdir -p
CP_CMD=cp -r

# Target specific include files
_INCLUDES=

debug : _INCLUDES=-I $(DEBUG_MODE_INC_DIR)
release : _INCLUDES=-I $(RELEASE_MODE_INC_DIR)

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
	$(RM_CMD) $(TARGET_DIR)
	$(MKDIR_CMD) $(TARGET_DIR)
	$(MV_CMD) $(EXECUTABLE) $(TARGET_DIR)
	$(CP_CMD) $(ASSETS_DIR)/* $(TARGET_DIR)

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(_INCLUDES) $(SOURCES) -c
	$(MV_CMD) *.o ./$(SRC_DIR)/

.PHONY: clean
clean:
	find . -type f -name '*~' | xargs $(RM_CMD)
	find . -type f -name '*.o' | xargs $(RM_CMD)
	$(RM_CMD) $(TARGET_DIR)