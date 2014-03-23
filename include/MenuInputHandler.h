#ifndef TOP_MENU_H
#define TOP_MENU_H

#include <vector>
#include <string>

#include "MenuBrowser.h"

class Engine;

enum class MenuAction {browsed, selected, selectedShift, space, esc};

class MenuInputHandler {
public:
  MenuAction getAction(MenuBrowser& browser);

private:
  Engine& eng;

  friend class Engine;
  MenuInputHandler(Engine& engine) : eng(engine) {}
};

#endif
