#ifndef POPUP_H
#define POPUP_H

#include <vector>
#include <string>

#include "Audio.h"

class Engine;

class Popup {
public:
  Popup(Engine& engine) : eng(engine) {}

  void showMsg(const std::string& msg, const bool DRAW_MAP_AND_INTERFACE,
               const std::string& title = "",
               const SfxId sfx = SfxId::endOfSfxId) const;

  int showMenuMsg(
    const std::string& msg, const bool DRAW_MAP_AND_INTERFACE,
    const std::vector<std::string>& choices, const std::string& title = "",
    const SfxId sfx = SfxId::endOfSfxId) const;

private:
  int printBoxAndGetTitleYPos(const int TEXT_H_TOT,
                              const int WIDTH_OVERRIDE = -1) const;

  void menuMsgDrawingHelper(
    const std::vector<std::string>& lines,
    const std::vector<std::string>& choices, const bool DRAW_MAP_AND_INTERFACE,
    const unsigned int currentChoice, const int TEXT_H_TOT,
    const std::string& title = "") const;

  Engine& eng;
};

#endif
