#ifndef TOP_MENU_H
#define TOP_MENU_H

#include <vector>
#include <string>

#include "MenuBrowser.h"

using namespace std;

class Engine;

enum MenuAction_t {
  menuAction_browsed, menuAction_selected, menuAction_canceled
};

class MenuInputHandler {
public:
  MenuAction_t getAction(MenuBrowser& browser);

private:
  SDL_Event m_event;
  Uint8* m_keystates;

  Engine* eng;

  void clearKeyEvents() {
    while(SDL_PollEvent(&m_event)) {
    }
  }

  friend class Engine;
  MenuInputHandler(Engine* engine) :
    eng(engine) {
  }
};

#endif
