#ifndef TOP_MENU_H
#define TOP_MENU_H

#include <vector>
#include <string>

#include "MenuBrowser.h"



enum class MenuAction {browsed, selected, selectedShift, space, esc};

class MenuInputHandler {
public:
  MenuAction getAction(MenuBrowser& browser);

private:


  friend
  MenuInputHandler() {}
};

#endif
