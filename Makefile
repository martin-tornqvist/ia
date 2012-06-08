CC=g++

SRC_DIR=src
TARGET_DIR=target

CFLAGS=-Wall -Wextra -I./sdl/include/
LDFLAGS=-lSDL

EXECUTABLE=ia

SOURCES=$(shell ls $(SRC_DIR)/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	mkdir -p $(TARGET_DIR)
	mv -f $(EXECUTABLE) $(TARGET_DIR)

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -c
	mv -f *.o ./$(SRC_DIR)/

RM_CMD=rm -rf

.PHONY: clean
clean:
	$(RM_CMD) *.o
	$(RM_CMD) $(SRC_DIR)/*.o $(TARGET_DIR)
