#ifndef INPUT_H
#define INPUT_H

#include "SFML/Window/Keyboard.hpp"

#include "ConstTypes.h"

class Engine;

struct KeyboardReadReturnData {
public:
  KeyboardReadReturnData() : key_(-1), sfmlKey_(sf::Keyboard::KeyCount), isShiftHeld_(false), isCtrlHeld_(false) {
  }

  KeyboardReadReturnData(char key, sf::Keyboard::Key sfmlKey, bool isShiftHeld, bool isCtrlHeld) :
    key_(key), sfmlKey_(sfmlKey), isShiftHeld_(isShiftHeld), isCtrlHeld_(isCtrlHeld) {
  }

  KeyboardReadReturnData(char key) :
    key_(key), sfmlKey_(sf::Keyboard::KeyCount), isShiftHeld_(false), isCtrlHeld_(false) {
  }

  KeyboardReadReturnData(sf::Keyboard::Key sfmlKey) :
    key_(-1), sfmlKey_(sfmlKey), isShiftHeld_(false), isCtrlHeld_(false) {
  }

  char key_;
  sf::Keyboard::Key sfmlKey_;
  bool isShiftHeld_, isCtrlHeld_;
};

class Input {
public:
  Input(Engine* engine, bool* quitToMainMenu);

  void handleMapModeInputUntilFound();

  KeyboardReadReturnData readKeysUntilFound() const;

  void clearEvents() const;

private:
  friend class Bot;
  void handleKeyPress(const KeyboardReadReturnData& d);

  void clearLogMessages();
  int* dungeonLevel_;
  Engine* eng;
  bool* quitToMainMenu_;
};

#endif
