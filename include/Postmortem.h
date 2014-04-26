#ifndef POSTMORTEM_H
#define POSTMORTEM_H

#include <vector>
#include <string>

#include "MenuBrowser.h"

using namespace std;

class Postmortem {
public:
  Postmortem() {}

  void run(bool* const quitGame);

private:
  struct StrAndClr {
    StrAndClr(const string str_, const SDL_Color clr_) :
      str(str_), clr(clr_) {}
    StrAndClr() {}
    string str;
    SDL_Color clr;
  };

  void readKeysMenu(const vector<StrAndClr>& linesAndClr, bool* const quitGame);

  void renderMenu(const MenuBrowser& browser);

  void runInfo(const vector<StrAndClr>& lines);

  void render(const vector<StrAndClr>& linesAndClr, const int TOP_ELEMENT);

  void makeMemorialFile(const vector<StrAndClr>& lines);

  void makeInfoLines(vector<StrAndClr>& linesRef);
};

#endif
