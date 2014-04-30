#ifndef MENU_INPUT_HANDLING_H
#define MENU_INPUT_HANDLING_H

#include "MenuBrowser.h"

enum class MenuAction {browsed, selected, selectedShift, space, esc};

namespace MenuInputHandling {

MenuAction getAction(MenuBrowser& browser);

} //MenuInputHandling

#endif
