#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>

#include "CmnData.h"

struct KeyboardReadRetData {
public:
  KeyboardReadRetData() :
    key_(-1), sdlKey_(SDLK_LAST), isShiftHeld_(false), isCtrlHeld_(false) {}

  KeyboardReadRetData(char key, SDLKey sdlKey, bool isShiftHeld,
                      bool isCtrlHeld) :
    key_(key), sdlKey_(sdlKey), isShiftHeld_(isShiftHeld),
    isCtrlHeld_(isCtrlHeld) {}

  KeyboardReadRetData(char key) :
    key_(key), sdlKey_(SDLK_LAST), isShiftHeld_(false), isCtrlHeld_(false) {}

  KeyboardReadRetData(SDLKey sdlKey) :
    key_(-1), sdlKey_(sdlKey), isShiftHeld_(false), isCtrlHeld_(false) {}

  char key_;
  SDLKey sdlKey_;
  bool isShiftHeld_, isCtrlHeld_;
};

namespace Input {

void init();
void cleanup();

void handleMapModeInputUntilFound();

KeyboardReadRetData readKeysUntilFound(const bool IS_O_RETURN = true);

void clearEvents();

void setKeyRepeatDelays();

void handleKeyPress(const KeyboardReadRetData& d);

} //Input

#endif
