#ifndef POSTMORTEM_H
#define POSTMORTEM_H

#include <vector>
#include <string>

#include "MenuBrowser.h"

class Postmortem {
public:
  Postmortem() {}

  void run(bool* const quitGame);

private:
  struct StrAndClr {
    StrAndClr(const std::string str_, const SDL_Color clr_) :
      str(str_), clr(clr_) {}
    StrAndClr() {}
    std::string str;
    SDL_Color clr;
  };

  void readKeysMenu(const std::vector<StrAndClr>& linesAndClr, bool* const quitGame);

  void renderMenu(const MenuBrowser& browser);

  void runInfo(const std::vector<StrAndClr>& lines);

  void render(const std::vector<StrAndClr>& linesAndClr, const int TOP_ELEMENT);

  void makeMemorialFile(const std::vector<StrAndClr>& lines);

  void makeInfoLines(std::vector<StrAndClr>& linesRef);
};

#endif
