CC=g++

# Directiories
SRC_DIR=src
DEBUG_MODE_INC_DIR=$(SRC_DIR)/debugModeIncl
RELEASE_MODE_INC_DIR=$(SRC_DIR)/releaseModeIncl
TARGET_DIR=target
ASSETS_DIR=assets
SFML_LIB_DIR=API/SFML/lib_linux_shared

# Includes
INCLUDES=-I API/SFML/include

# Target specific include files
_INCLUDES=
_CFLAGS=
debug : _INCLUDES=-I $(DEBUG_MODE_INC_DIR)
debug : _CFLAGS=-O0 -g
release : _INCLUDES=-I $(RELEASE_MODE_INC_DIR)
release : _CFLAGS=-O2

#Flags
CFLAGS=-Wall -Wextra
LDFLAGS=-L $(SFML_LIB_DIR) -lsfml-graphics -lsfml-window -lsfml-system

# Output and sources
EXECUTABLE=bin
SOURCES=$(shell ls $(SRC_DIR)/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
OBJECTS_STAMP_FILE=obj_stamp
RUN_GAME_SCRIPT=ia

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
	$(MKDIR) $(TARGET_DIR)/$(SFML_LIB_DIR)
	$(CP) $(SFML_LIB_DIR)/* $(TARGET_DIR)/$(SFML_LIB_DIR)
	echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(SFML_LIB_DIR) ./$(EXECUTABLE)" > $(TARGET_DIR)/$(RUN_GAME_SCRIPT)
	chmod 777 $(TARGET_DIR)/$(RUN_GAME_SCRIPT)

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
