CC=g++

# Directiories
SRC_DIR=src
DEBUG_MODE_INC_DIR=$(SRC_DIR)/debugModeIncl
RELEASE_MODE_INC_DIR=$(SRC_DIR)/releaseModeIncl
TARGET_DIR=target
ASSETS_DIR=assets

# Includes
INCLUDES=-I API/SFML/include

# Target specific include files
_INCLUDES=
_CFLAGS=
debug : _INCLUDES=-I $(DEBUG_MODE_INC_DIR)
debug: _CFLAGS=-O0 -g
release : _INCLUDES=-I $(RELEASE_MODE_INC_DIR)
release : _CFLAGS=-O2

#Flags
CFLAGS=-Wall -Wextra
LDFLAGS=-L API/SFML/lib_linux -lsfml-graphics -lsfml-window -lsfml-system

# Output and sources
EXECUTABLE=ia
SOURCES=$(shell ls $(SRC_DIR)/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
GAME_DOC_FILE=auto_game_doc.txt

# Various bash commands
RM=rm -rf
MV=mv -f
MKDIR=mkdir -p
CP=cp -r

# Dependencies 
debug: $(SOURCES) $(EXECUTABLE) $(GAME_DOC_FILE)
.PHONY: release
release: $(SOURCES) $(EXECUTABLE)

# Make targets
.DEFAULT_GOAL=default_target
.PHONY: default_target
default_target:
	@echo "Use \"make debug\" or \"make release\""

.PHONY: $(GAME_DOC_FILE)
$(GAME_DOC_FILE):
	echo "This is a generated document containing game mechanics information for Infra Arcana." > $@ && \
	echo "" >> $@ && \
	echo "Information is added here in two steps:" >> $@ && \
	echo "1. When making the debug target, all files in the source folder are parsed" >> $@ && \
	echo "for lines containing double percentage signs. These lines are handwritten" >> $@ && \
	echo "descriptions of various game mechanics." >> $@ && \
	echo "2. When the game is started in debug mode, it appends monster and item info." >> $@ && \
	echo "This is only done if there is no previous such information appended." >> $@ && \
	echo "" >> $@ && \
	echo "" >> $@ && \
	echo "1. STATIC SOURCE FILES DOCUMENTATION" >> $@ && \
	echo "------------------------------------------------" >> $@ && \
	grep -ri %% $(SRC_DIR)/* >> $@ && \
	sed -i 's/%%//g' $@ && \
	echo "" >> $@ && \
	echo "" >> $@ && \
	echo "" >> $@ && \
	echo "2. RUN-TIME GENERATED INFORMATION" >> $@ && \
	echo "------------------------------------------------" >> $@

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)
	$(RM) $(TARGET_DIR)
	$(MKDIR) $(TARGET_DIR)
	$(MV) $(EXECUTABLE) $(TARGET_DIR)
	$(CP) $(ASSETS_DIR)/* $(TARGET_DIR)

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(_CFLAGS) $(INCLUDES) $(_INCLUDES) $(SOURCES) -c
	$(MV) *.o ./$(SRC_DIR)/

.PHONY: clean
clean:
	find . -type f -name '*~' | xargs $(RM)
	find . -type f -name '*.o' | xargs $(RM)
	$(RM) $(TARGET_DIR)