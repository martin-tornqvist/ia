#ifndef TOP_MENU_H
#define TOP_MENU_H

#include <vector>
#include <string>

#include "MenuBrowser.h"

class Engine;

enum MenuAction {
  menuAction_browsed,
  menuAction_selected,
  menuAction_selectedWithShift,
  menuAction_space,
  menuAction_esc,
};

class MenuInputHandler {
public:
  MenuAction getAction(MenuBrowser& browser);

private:
  Engine& eng;

  friend class Engine;
  MenuInputHandler(Engine& engine) :
    eng(engine) {
  }
};

#endif
