#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "MenuBrowser.h"
#include "ConstTypes.h"

class Engine;

class MainMenu {
public:
  MainMenu(Engine* engine) {
    eng = engine;
  }
  GameEntry_t run(bool& quit);

private:
  Engine* eng;
  void draw(const MenuBrowser& browser);

  string getHplQuote();
};

#endif
