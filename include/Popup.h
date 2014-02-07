#ifndef POPUP_H
#define POPUP_H

#include <vector>
#include <string>

#include "AudioIds.h"

using namespace std;

class Engine;

class Popup {
public:
  Popup(Engine& engine) : eng(engine) {}

  void showMessage(const string& message, const bool DRAW_MAP_AND_INTERFACE,
                   const string& title = "",
                   const SfxId sfx = endOfSfxId) const;

  int showMultiChoiceMessage(
    const string& message, const bool DRAW_MAP_AND_INTERFACE,
    const vector<string>& choices, const string& title = "",
    const SfxId sfx = endOfSfxId) const;

private:
  int printBoxAndReturnTitleYPos(const int TEXT_H) const;

  void multiChoiceMessageDrawingHelper(
    const vector<string>& lines, const vector<string>& choices,
    const bool DRAW_MAP_AND_INTERFACE, const unsigned int currentChoice,
    const int TEXT_H, const string title = "") const;

  Engine& eng;
};

#endif
