#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>

#include "CommonData.h"

class Engine;

struct KeyboardReadReturnData {
public:
  KeyboardReadReturnData() :
    key_(-1), sdlKey_(SDLK_LAST), isShiftHeld_(false), isCtrlHeld_(false) {}

  KeyboardReadReturnData(char key, SDLKey sdlKey, bool isShiftHeld,
                         bool isCtrlHeld) :
    key_(key), sdlKey_(sdlKey), isShiftHeld_(isShiftHeld),
    isCtrlHeld_(isCtrlHeld) {}

  KeyboardReadReturnData(char key) :
    key_(key), sdlKey_(SDLK_LAST), isShiftHeld_(false), isCtrlHeld_(false) {}

  KeyboardReadReturnData(SDLKey sdlKey) :
    key_(-1), sdlKey_(sdlKey), isShiftHeld_(false), isCtrlHeld_(false) {}

  char key_;
  SDLKey sdlKey_;
  bool isShiftHeld_, isCtrlHeld_;
};

namespace Input {

void init();
void cleanup();

void handleMapModeInputUntilFound(Engine& eng);

KeyboardReadReturnData readKeysUntilFound(Engine& eng);

void clearEvents();

void setKeyRepeatDelays();

void handleKeyPress(const KeyboardReadReturnData& d, Engine& eng);

} //Input

#endif
