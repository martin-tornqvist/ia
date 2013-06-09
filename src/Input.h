#ifndef INPUT_H
#define INPUT_H

#include "SDL/SDL_keyboard.h"
#include "SDL/SDL_events.h"

#include "ConstTypes.h"

class Engine;

struct KeyboardReadReturnData {
public:
  KeyboardReadReturnData() : key_(-1), sdlKey_(SDLK_LAST), isShiftHeld_(false),
    isCtrlHeld_(false) {
  }

  KeyboardReadReturnData(char key, SDLKey sdlKey, bool isShiftHeld, bool isCtrlHeld) :
    key_(key), sdlKey_(sdlKey), isShiftHeld_(isShiftHeld), isCtrlHeld_(isCtrlHeld) {
  }

  KeyboardReadReturnData(char key) :
    key_(key), sdlKey_(SDLK_LAST), isShiftHeld_(false), isCtrlHeld_(false) {
  }

  KeyboardReadReturnData(SDLKey sdlKey) :
    key_(-1), sdlKey_(sdlKey), isShiftHeld_(false), isCtrlHeld_(false) {
  }

  char key_;
  SDLKey sdlKey_;
  bool isShiftHeld_, isCtrlHeld_;
};

class Input {
public:
  Input(Engine* engine, bool* quitToMainMenu);

  void handleMapModeInputUntilFound();

  KeyboardReadReturnData readKeysUntilFound();

  void clearEvents();

  void setKeyRepeatDelays();

private:
  friend class Bot;
  void handleKeyPress(const KeyboardReadReturnData& d);
  void clearLogMessages();
  SDL_Event event_;
  int* dungeonLevel_;
  Engine* eng;
  bool* quitToMainMenu_;
};

#endif
