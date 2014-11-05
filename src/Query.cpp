#include "Query.h"

#include <iostream>

#include "Config.h"
#include "Input.h"
#include "Render.h"
#include "Converters.h"

using namespace std;

namespace Query
{

namespace
{

bool isInited_ = false;

} //namespace

void init()
{
  isInited_ = true;
}

void waitForKeyPress()
{
  if(isInited_ && !Config::isBotPlaying())
  {
    Input::readKeysUntilFound();
  }
}

YesNoAnswer yesOrNo(char keyForSpecialEvent)
{
  if(!isInited_ || Config::isBotPlaying()) {return YesNoAnswer::yes;}

  KeyData d = Input::readKeysUntilFound();
  while(
    d.key    != 'y'          &&
    d.key    != 'n'          &&
    d.sdlKey != SDLK_ESCAPE  &&
    d.sdlKey != SDLK_SPACE   &&
    (d.key != keyForSpecialEvent || keyForSpecialEvent == -1))
  {
    d = Input::readKeysUntilFound();
  }
  if(d.key == keyForSpecialEvent && keyForSpecialEvent != -1)
  {
    return YesNoAnswer::special;
  }
  if(d.key == 'y')
  {
    return YesNoAnswer::yes;
  }

  return YesNoAnswer::no;
}

KeyData letter(const bool ACCEPT_ENTER)
{
  if(!isInited_ || Config::isBotPlaying()) {return 'a';}

  while(true)
  {
    KeyData d = Input::readKeysUntilFound();

    if(
      (ACCEPT_ENTER && d.sdlKey == SDLK_RETURN) ||
      d.sdlKey == SDLK_ESCAPE ||
      d.sdlKey == SDLK_SPACE  ||
      (d.key >= 'a' && d.key <= 'z') || (d.key >= 'A' && d.key <= 'Z'))
    {
      return d;
    }
  }

  return KeyData();
}

int number(const Pos& pos, const Clr clr, const int MIN, const int MAX_NR_DIGITS,
           const int DEFAULT, const bool CANCEL_RETURNS_DEFAULT)
{
  if(!isInited_ || Config::isBotPlaying()) {return 0;}

  int retNum = max(MIN, DEFAULT);
  Render::coverArea(Panel::screen, pos, Pos(MAX_NR_DIGITS + 1, 1));
  const string str = (retNum == 0 ? "" : toStr(retNum)) + "_";
  Render::drawText(str, Panel::screen, pos, clr);
  Render::updateScreen();

  while(true)
  {
    KeyData d;
    while((d.key < '0' || d.key > '9') && d.sdlKey != SDLK_RETURN &&
          d.sdlKey != SDLK_SPACE && d.sdlKey != SDLK_ESCAPE &&
          d.sdlKey != SDLK_BACKSPACE)
    {
      d = Input::readKeysUntilFound();
    }

    if(d.sdlKey == SDLK_RETURN)
    {
      return max(MIN, retNum);
    }

    if(d.sdlKey == SDLK_SPACE || d.sdlKey == SDLK_ESCAPE)
    {
      return CANCEL_RETURNS_DEFAULT ? DEFAULT : -1;
    }

    const string retNumStr = toStr(retNum);
    const int CUR_NUM_DIGITS = retNumStr.size();

    if(d.sdlKey == SDLK_BACKSPACE)
    {
      retNum = retNum / 10;
      Render::coverArea(Panel::screen, pos, Pos(MAX_NR_DIGITS + 1, 1));
      Render::drawText((retNum == 0 ? "" : toStr(retNum)) + "_",
                       Panel::screen, pos, clr);
      Render::updateScreen();
      continue;
    }

    if(CUR_NUM_DIGITS < MAX_NR_DIGITS)
    {
      int curDigit = d.key - '0';
      retNum = max(MIN, retNum * 10 + curDigit);
      Render::coverArea(Panel::screen, pos, Pos(MAX_NR_DIGITS + 1, 1));
      Render::drawText((retNum == 0 ? "" : toStr(retNum)) + "_",
                       Panel::screen, pos, clr);
      Render::updateScreen();
    }
  }
  return -1;
}

void waitForEscOrSpace()
{
  if(isInited_ && !Config::isBotPlaying())
  {
    KeyData d = Input::readKeysUntilFound();
    while(d.sdlKey != SDLK_SPACE && d.sdlKey != SDLK_ESCAPE)
    {
      d = Input::readKeysUntilFound();
    }
  }
}

Dir dir()
{
  if(!isInited_ || Config::isBotPlaying()) {return Dir::END;}

  KeyData d = Input::readKeysUntilFound();

  while(d.sdlKey != SDLK_RIGHT   && d.sdlKey != SDLK_UP       &&
        d.sdlKey != SDLK_LEFT    && d.sdlKey != SDLK_DOWN     &&
        d.sdlKey != SDLK_ESCAPE  && d.sdlKey != SDLK_SPACE    &&
        d.sdlKey != SDLK_PAGEUP  && d.sdlKey != SDLK_HOME     &&
        d.sdlKey != SDLK_END     && d.sdlKey != SDLK_PAGEDOWN &&
        d.key != 'h' && d.key != 'j' && d.key != 'k' && d.key != 'l' &&
        d.key != 'y' && d.key != 'u' && d.key != 'b' && d.key != 'n' &&
        (d.key < '1' || d.key > '9' || d.key == '5'))
  {
    d = Input::readKeysUntilFound();
  }

  if(d.sdlKey == SDLK_SPACE || d.sdlKey == SDLK_ESCAPE)
  {
    return Dir::center;
  }
  if(d.sdlKey == SDLK_RIGHT    || d.key == '6' || d.key == 'l')
  {
    if(d.isShiftHeld)
    {
      return Dir::upRight;
    }
    else if(d.isCtrlHeld)
    {
      return Dir::downRight;
    }
    else
    {
      return Dir::right;
    }
  }
  if(d.sdlKey == SDLK_PAGEUP   || d.key == '9' || d.key == 'u')
  {
    return Dir::upRight;
  }
  if(d.sdlKey == SDLK_UP       || d.key == '8' || d.key == 'k')
  {
    return Dir::up;
  }
  if(d.sdlKey == SDLK_END      || d.key == '7' || d.key == 'y')
  {
    return Dir::upLeft;
  }
  if(d.sdlKey == SDLK_LEFT     || d.key == '4' || d.key == 'h')
  {
    if(d.isShiftHeld)
    {
      return Dir::upLeft;
    }
    else if(d.isCtrlHeld)
    {
      return Dir::downLeft;
    }
    else
    {
      return Dir::left;
    }
  }
  if(d.sdlKey == SDLK_END      || d.key == '1' || d.key == 'b')
  {
    return Dir::downLeft;
  }
  if(d.sdlKey == SDLK_DOWN     || d.key == '2' || d.key == 'j')
  {
    return Dir::down;
  }
  if(d.sdlKey == SDLK_PAGEDOWN || d.key == '3' || d.key == 'n')
  {
    return Dir::downRight;
  }

  return Dir::center;
}

} //Query
