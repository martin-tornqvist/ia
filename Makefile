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
SRC_DIR          = src
INC_DIR          = include
TARGET_DIR       = target
ASSETS_DIR       = assets
RL_UTILS_SRC_DIR = rl_utils/src
RL_UTILS_INC_DIR = rl_utils/include

# Includes
INCLUDES= \
  -I $(INC_DIR) \
  -I $(RL_UTILS_INC_DIR) \
  #

# Flags
CXXFLAGS_release = \
  -O2 \
  -DNDEBUG \
  #

CXXFLAGS_debug = \
  -O0 \
  -g \
  #

# The debug trace system generates warnings for trace levels higher than the
# compiled level, so sadly Wunused-value cannot be used
CXXFLAGS= \
  -std=c++11 \
  -Wall \
  -Wextra \
  -Werror \
  -Wno-unused-value \
  -fno-rtti \
  -fno-exceptions \
  $(shell sdl2-config --cflags) $(CXXFLAGS_$(BUILD))

# For building 32-bit binaries on x86_64 platform
# CXXFLAGS+=-m32 -march=i686

LD_FLAGS=$(shell sdl2-config --libs) -lSDL2_image -lSDL2_mixer

# Output and sources
EXECUTABLE       = ia
SRC              = $(wildcard $(SRC_DIR)/*.cpp)
RL_UTILS_SRC     = $(wildcard $(RL_UTILS_SRC_DIR)/*.cpp)
OBJECTS          = $(SRC:.cpp=.o)
RL_UTILS_OBJECTS = $(RL_UTILS_SRC:.cpp=.o)
DEPENDS          = $(SRC:.cpp=.d)

# Various bash commands
RM=rm -rf
MV=mv -f
MKDIR=mkdir -p
CP=cp -r
CAT=cat

# Make targets
all: $(EXECUTABLE)

$(EXECUTABLE): $(RL_UTILS_OBJECTS) $(OBJECTS)
	$(CXX) $^ -o $@ $(LD_FLAGS)
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
