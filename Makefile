###############################################################################
# Targets available:
# - release (just running "make" will also build this)
# - debug
# - windows-release (cross compilation using mingw)
# - osx
# - osx-debug
# - clean
#


###############################################################################
# Directories
###############################################################################
SRC_DIR            = src
INC_DIR            = include
TARGET_DIR         = target
ASSETS_DIR         = assets
RL_UTILS_DIR       = rl_utils
RL_UTILS_SRC_DIR   = $(RL_UTILS_DIR)/src
RL_UTILS_INC_DIR   = $(RL_UTILS_DIR)/include

# Only used for Windows cross compilation on Linux
SDL_BASE_DIR       = SDL
SDL_DIR            = $(SDL_BASE_DIR)/SDL2-2.0.4
SDL_IMAGE_DIR      = $(SDL_BASE_DIR)/SDL2_image-2.0.1
SDL_MIXER_DIR      = $(SDL_BASE_DIR)/SDL2_mixer-2.0.1

SDL_ARCH           = i686-w64-mingw32

SDL_INC_DIR        = $(SDL_DIR)/$(SDL_ARCH)/include
SDL_IMAGE_INC_DIR  = $(SDL_IMAGE_DIR)/$(SDL_ARCH)/include
SDL_MIXER_INC_DIR  = $(SDL_MIXER_DIR)/$(SDL_ARCH)/include

SDL_LIB_DIR        = $(SDL_DIR)/$(SDL_ARCH)/lib
SDL_IMAGE_LIB_DIR  = $(SDL_IMAGE_DIR)/$(SDL_ARCH)/lib
SDL_MIXER_LIB_DIR  = $(SDL_MIXER_DIR)/$(SDL_ARCH)/lib

SDL_BIN_DIR        = $(SDL_DIR)/$(SDL_ARCH)/bin
SDL_IMAGE_BIN_DIR  = $(SDL_IMAGE_DIR)/$(SDL_ARCH)/bin
SDL_MIXER_BIN_DIR  = $(SDL_MIXER_DIR)/$(SDL_ARCH)/bin


###############################################################################
# Common setup
###############################################################################
# Includes
INCLUDES = \
  -I $(INC_DIR) \
  -I $(RL_UTILS_INC_DIR) \
  #

# Compiler flags
# The debug trace system generates warnings for trace levels higher than the
# compiled level, so sadly Wunused-value cannot be used
CXXFLAGS = \
  -std=c++11 \
  -Wall \
  -Wextra \
  -Werror \
  -Wno-unused-value \
  -fno-rtti \
  -fno-exceptions \
  #

# Linker flags
LD_FLAGS =


###############################################################################
# Linux specific
###############################################################################
# Compiler for linux versions
release debug: CXX ?= g++

# Linux specific compiler flags
release debug: CXXFLAGS += $(shell sdl2-config --cflags)

# Linux release specific compiler flags
release: CXXFLAGS += \
  -O2 \
  -DNDEBUG \
  #

# Linux debug specific compiler flags
debug: CXXFLAGS += \
  -O0 \
  -g \
  #

# Linux specific linker flags
release debug: LD_FLAGS = \
  $(shell sdl2-config --libs) \
  -lSDL2_image \
  -lSDL2_mixer \
  #

# Executable
LINUX_EXE = ia


###############################################################################
# Windows cross compilation specific
###############################################################################
# Cross compiler to build Windows releases on Linux
windows-release: CXX = i686-w64-mingw32-g++-win32

# Windows specific includes
windows-release: INCLUDES += \
  -I $(SDL_INC_DIR) \
  -I $(SDL_IMAGE_INC_DIR) \
  -I $(SDL_MIXER_INC_DIR) \
  #

# Windows specific compiler flags
windows-release: CXXFLAGS += \
  -O2 \
  -DNDEBUG \
  #

# Windows specific linker flags
windows-release: LD_FLAGS += \
  -L $(SDL_LIB_DIR) \
  -L $(SDL_IMAGE_LIB_DIR) \
  -L $(SDL_MIXER_LIB_DIR) \
  -lSDL2 \
  -lSDL2main \
  -lSDL2_image \
  -lSDL2_mixer \
  -static-libgcc \
  -static-libstdc++ \
  #

WINDOWS_EXE = ia.exe

###############################################################################
# Mac OS X
###############################################################################
osx osx-debug: CXX ?= c++
osx osx-debug: CXXFLAGS = -std=c++11
osx osx-debug: INCLUDES = -I$(INC_DIR) -I$(RL_UTILS_INC_DIR) \
    -F/Library/Frameworks \
    -I/Library/Frameworks/SDL2.framework/Headers \
    -I/Library/Frameworks/SDL2_image.framework/Headers \
    -I/Library/Frameworks/SDL2_mixer.framework/Headers
osx osx-debug: LD_FLAGS = -F/Library/Frameworks \
    -framework Cocoa \
    -framework SDL2 -framework SDL2_image -framework SDL2_mixer
osx osx-debug: CXXFLAGS += -DMACOSX
osx: CXXFLAGS += -Os -DNDEBUG
osx-debug: CXXFLAGS += -O0 -g

###############################################################################
# Common output and sources
###############################################################################
SRC               = $(wildcard $(SRC_DIR)/*.cpp)
RL_UTILS_SRC      = $(wildcard $(RL_UTILS_SRC_DIR)/*.cpp)
OBJECTS           = $(SRC:.cpp=.o)
RL_UTILS_OBJECTS  = $(RL_UTILS_SRC:.cpp=.o)
# DEPENDS          = $(SRC:.cpp=.d)


###############################################################################
# Targets and recipes
###############################################################################
all: release

release debug: $(LINUX_EXE)

# The Windows version needs to copy some DLLs and licenses
windows-release: $(WINDOWS_EXE)
	cp \
	  $(SDL_BIN_DIR)/SDL2.dll \
	  $(SDL_IMAGE_BIN_DIR)/SDL2_image.dll \
	  $(SDL_IMAGE_BIN_DIR)/zlib1.dll \
	  $(SDL_IMAGE_BIN_DIR)/libpng16-16.dll \
	  $(SDL_IMAGE_BIN_DIR)/LICENSE.zlib.txt \
	  $(SDL_IMAGE_BIN_DIR)/LICENSE.png.txt \
	  $(SDL_MIXER_BIN_DIR)/SDL2_mixer.dll \
	  $(SDL_MIXER_BIN_DIR)/libogg-0.dll \
	  $(SDL_MIXER_BIN_DIR)/libvorbis-0.dll \
	  $(SDL_MIXER_BIN_DIR)/libvorbisfile-3.dll \
	  $(SDL_MIXER_BIN_DIR)/LICENSE.ogg-vorbis.txt \
	  $(TARGET_DIR)

osx osx-debug: $(LINUX_EXE)

$(LINUX_EXE) $(WINDOWS_EXE): $(RL_UTILS_OBJECTS) $(OBJECTS)
	$(CXX) $^ -o $@ $(LD_FLAGS)
	mkdir -p $(TARGET_DIR)
	mv -f $@ $(TARGET_DIR)
	cp -r $(ASSETS_DIR)/* $(TARGET_DIR)

%.o: %.cpp | check-rl-utils
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $< -o $@

# Make sure the RL Utils submodule exists
check-rl-utils :
	@echo "Checking..."
	@if [ -z "$(RL_UTILS_SRC)" ]; then \
	  echo ""; \
	  echo "***********************************************************"; \
	  echo "Error: No source files found in Git submodule \"$(RL_UTILS_DIR)\"!"; \
	  echo ""; \
	  echo "Please execute the following before building IA:"; \
	  echo ""; \
	  echo "  $$ git submodule init"; \
	  echo "  $$ git submodule update"; \
	  echo ""; \
	  echo "(Or use the equivalent GUI function)"; \
	  echo ""; \
	  echo "***********************************************************"; \
	  echo ""; \
	  exit 1; \
	fi

# Optional auto dependency tracking
#  -include depends.mk

# depends: $(DEPENDS)

# %.d:
# 	$(CXX) -MM $(CXXFLAGS) $(INCLUDES) $(@:.d=.cpp) -MF depends.tmp -MT$(@:.d=.o)
# 	cat depends.tmp >> depends.mk
# 	rm -rf depends.tmp

# clean-depends:
# 	rm -rf depends.mk

# Remove object files
clean:
	rm -rf $(TARGET_DIR) $(OBJECTS) $(RL_UTILS_OBJECTS)

.PHONY: all depends clean clean-depends check-rl-utils \
    release debug windows-release windows-debug osx osx-debug
