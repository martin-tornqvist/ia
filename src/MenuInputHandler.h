#ifndef TOP_MENU_H
#define TOP_MENU_H

#include <vector>
#include <string>

#include "MenuBrowser.h"

using namespace std;

class Engine;

enum MenuAction_t {
  menuAction_browsed,
  menuAction_selected,
  menuAction_canceled
};

class MenuInputHandler {
public:
  MenuAction_t getAction(MenuBrowser& browser);

private:
  Engine* eng;

  friend class Engine;
  MenuInputHandler(Engine* engine) :
    eng(engine) {
  }
};

#endif
