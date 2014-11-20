#include "MenuInputHandling.h"

#include "Input.h"
#include "Config.h"

namespace MenuInputHandling
{

MenuAction getAction(MenuBrowser& browser)
{
  while (true)
  {
    KeyData d = Input::readKeysUntilFound();

    if (d.sdlKey == SDLK_RIGHT        || d.key == '6' || d.key == 'l')
    {
      browser.navigate(Dir::right);
      return MenuAction::browsed;
    }
    else if (d.sdlKey == SDLK_LEFT  || d.key == '4' || d.key == 'h')
    {
      browser.navigate(Dir::left);
      return MenuAction::browsed;
    }
    else if (d.sdlKey == SDLK_UP    || d.key == '8' || d.key == 'k')
    {
      browser.navigate(Dir::up);
      return MenuAction::browsed;
    }
    else if (d.sdlKey == SDLK_DOWN  || d.key == '2' || d.key == 'j')
    {
      browser.navigate(Dir::down);
      return MenuAction::browsed;
    }
    else if (d.sdlKey == SDLK_RETURN)
    {
      return d.isShiftHeld ? MenuAction::selectedShift : MenuAction::selected;
    }
    else if (d.sdlKey == SDLK_SPACE)
    {
      return MenuAction::space;
    }
    else if (d.sdlKey == SDLK_ESCAPE)
    {
      return MenuAction::esc;
    }
  }
  return MenuAction::esc;
}

} //MenuInputHandling
