#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "MenuBrowser.h"
#include "CommonData.h"

class Engine;

class MainMenu {
public:
  MainMenu(Engine& engine) : quote(""), eng(engine) {}

  GameEntryMode run(bool& quit, int& introMusChannel);

private:
  void draw(const MenuBrowser& browser) const;

  string getHplQuote() const;

  string quote;

  Engine& eng;
};

#endif
