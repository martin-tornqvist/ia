#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "MenuBrowser.h"
#include "CmnData.h"

class MainMenu {
public:
  MainMenu() : quote("") {}

  GameEntryMode run(bool& quit, int& introMusChannel);

private:
  void draw(const MenuBrowser& browser) const;

  std::string getHplQuote() const;

  std::string quote;
};

#endif
