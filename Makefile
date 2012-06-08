CC=g++

SRC_DIR=src
TARGET_DIR=target
ASSETS_DIR=assets

CFLAGS=-Wall -Wextra $(shell sdl-config --cflags)
#LDFLAGS=-lSDL
LDFLAGS=$(shell sdl-config --libs)

EXECUTABLE=ia

SOURCES=$(shell ls $(SRC_DIR)/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

RM_CMD=rm -rf
MV_CMD=mv -f
MKDIR_CMD=mkdir -p
CP_CMD=cp -r

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)
	$(RM_CMD) $(TARGET_DIR)
	$(MKDIR_CMD) $(TARGET_DIR)
	$(MV_CMD) $(EXECUTABLE) $(TARGET_DIR)
	$(CP_CMD) $(ASSETS_DIR)/* $(TARGET_DIR)

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -c
	$(MV_CMD) *.o ./$(SRC_DIR)/

.PHONY: clean
clean:
	$(RM_CMD) *~ *.o $(SRC_DIR)/*.o $(SRC_DIR)/*~ $(TARGET_DIR)


