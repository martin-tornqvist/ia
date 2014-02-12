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

  void showMsg(const string& msg, const bool DRAW_MAP_AND_INTERFACE,
               const string& title = "",
               const SfxId sfx = endOfSfxId) const;

  int showMenuMsg(
    const string& msg, const bool DRAW_MAP_AND_INTERFACE,
    const vector<string>& choices, const string& title = "",
    const SfxId sfx = endOfSfxId) const;

private:
  int printBoxAndGetTitleYPos(const int TEXT_H_TOT,
                              const int WIDTH_OVERRIDE = -1) const;

  void menuMsgDrawingHelper(
    const vector<string>& lines, const vector<string>& choices,
    const bool DRAW_MAP_AND_INTERFACE, const unsigned int currentChoice,
    const int TEXT_H_TOT, const string title = "") const;

  Engine& eng;
};

#endif
