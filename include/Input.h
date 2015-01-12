#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>

#include "CmnData.h"

struct KeyData
{
  KeyData() :
    key         (-1),
    sdlKey      (SDLK_LAST),
    isShiftHeld (false),
    isCtrlHeld  (false) {}

  KeyData(char key_, SDLKey sdlKey_, bool isShiftHeld_,  bool isCtrlHeld_) :
    key         (key_),
    sdlKey      (sdlKey_),
    isShiftHeld (isShiftHeld_),
    isCtrlHeld  (isCtrlHeld_) {}

  KeyData(char key_) :
    key         (key_),
    sdlKey      (SDLK_LAST),
    isShiftHeld (false),
    isCtrlHeld  (false) {}

  KeyData(SDLKey sdlKey_) :
    key         (-1),
    sdlKey      (sdlKey_),
    isShiftHeld (false),
    isCtrlHeld  (false) {}

  char    key;
  SDLKey  sdlKey;
  bool    isShiftHeld, isCtrlHeld;
};

namespace Input
{

void init();
void cleanup();

//Reads input until a valid map mode command is executed
void mapModeInput();

KeyData getInput(const bool IS_O_RETURN = true);

void clearEvents();

void setKeyRepeatDelays();

void handleMapModeKeyPress(const KeyData& d);

bool isKeyHeld(const SDLKey key);

} //Input

#endif
