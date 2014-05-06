#include "MenuInputHandling.h"

#include "Input.h"
#include "Config.h"

namespace MenuInputHandling {

MenuAction getAction(MenuBrowser& browser) {
  while(true) {
    KeyboardReadRetData d = Input::readKeysUntilFound();

    if(d.sdlKey_ == SDLK_RIGHT        || d.key_ == '6' || d.key_ == 'l') {
      browser.navigate(Dir::right);
      return MenuAction::browsed;
    } else if(d.sdlKey_ == SDLK_LEFT  || d.key_ == '4' || d.key_ == 'h') {
      browser.navigate(Dir::left);
      return MenuAction::browsed;
    } else if(d.sdlKey_ == SDLK_UP    || d.key_ == '8' || d.key_ == 'k') {
      browser.navigate(Dir::up);
      return MenuAction::browsed;
    } else if(d.sdlKey_ == SDLK_DOWN  || d.key_ == '2' || d.key_ == 'j') {
      browser.navigate(Dir::down);
      return MenuAction::browsed;
    } else if(d.sdlKey_ == SDLK_RETURN) {
      return d.isShiftHeld_ ? MenuAction::selectedShift : MenuAction::selected;
    } else if(d.sdlKey_ == SDLK_SPACE) {
      return MenuAction::space;
    } else if(d.sdlKey_ == SDLK_ESCAPE) {
      return MenuAction::esc;
    }
  }
  return MenuAction::esc;
}

} //MenuInputHandling
