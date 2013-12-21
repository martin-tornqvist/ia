#ifndef POSTMORTEM_H
#define POSTMORTEM_H

#include <vector>
#include <string>

#include "MenuBrowser.h"

using namespace std;

class Engine;

class Postmortem {
public:
  Postmortem(Engine& engine) :
    eng(engine) {
  }

  void run(bool* const quitGame);

private:
  struct StrAndClr {
    StrAndClr(const string str_, const SDL_Color clr_) :
      str(str_), clr(clr_) {
    }
    StrAndClr() {
    }
    string str;
    SDL_Color clr;
  };

  vector<StrAndClr> postmortemLines;

  void readKeysMenu(bool* const quitGame);

  void renderMenu(const MenuBrowser& browser);

  void runInfo();

  void renderInfo(const int TOP_ELEMENT);

  void makeMemorialFile();

  void makeInfoLines();

  Engine& eng;
};

#endif
