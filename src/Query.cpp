#include "Query.h"

#include <iostream>

#include "Engine.h"
#include "Config.h"
#include "Input.h"
#include "Renderer.h"
#include "Converters.h"

using namespace std;

void Query::waitForKeyPress() const {
  if(Config::isBotPlaying() == false) {
    Input::readKeysUntilFound(eng);
  }
}

YesNoAnswer Query::yesOrNo(char keyForSpecialEvent) const {
  if(Config::isBotPlaying()) {
    return YesNoAnswer::yes;
  }

  KeyboardReadReturnData d = Input::readKeysUntilFound(eng);
  while(
    d.key_    != 'y'          &&
    d.key_    != 'n'          &&
    d.sdlKey_ != SDLK_ESCAPE  &&
    d.sdlKey_ != SDLK_SPACE   &&
    (d.key_ != keyForSpecialEvent || keyForSpecialEvent == -1)) {
    d = Input::readKeysUntilFound(eng);
  }
  if(d.key_ == keyForSpecialEvent && keyForSpecialEvent != -1) {
    return YesNoAnswer::special;
  }
  if(d.key_ == 'y') {
    return YesNoAnswer::yes;
  }
  return YesNoAnswer::no;
}

int Query::number(const Pos& pos, const SDL_Color clr, const int MIN,
                  const int MAX_NR_DIGITS, const int DEFAULT,
                  const bool CANCEL_RETURNS_DEFAULT) const {
  int retNum = max(MIN, DEFAULT);
  Renderer::coverArea(panel_screen, pos, Pos(MAX_NR_DIGITS + 1, 1));
  const string str = (retNum == 0 ? "" : toString(retNum)) + "_";
  Renderer::drawText(str, panel_screen, pos, clr);
  Renderer::updateScreen();

  while(true) {
    KeyboardReadReturnData d;
    while((d.key_ < '0' || d.key_ > '9') && d.sdlKey_ != SDLK_RETURN &&
          d.sdlKey_ != SDLK_SPACE && d.sdlKey_ != SDLK_ESCAPE &&
          d.sdlKey_ != SDLK_BACKSPACE) {
      d = Input::readKeysUntilFound(eng);
    }

    if(d.sdlKey_ == SDLK_RETURN) {
      return max(MIN, retNum);
    }

    if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      return CANCEL_RETURNS_DEFAULT ? DEFAULT : -1;
    }

    const string retNumStr = toString(retNum);
    const int CUR_NUM_DIGITS = retNumStr.size();

    if(d.sdlKey_ == SDLK_BACKSPACE) {
      retNum = retNum / 10;
      Renderer::coverArea(panel_screen, pos, Pos(MAX_NR_DIGITS + 1, 1));
      Renderer::drawText((retNum == 0 ? "" : toString(retNum)) + "_",
                         panel_screen, pos, clr);
      Renderer::updateScreen();
      continue;
    }

    if(CUR_NUM_DIGITS < MAX_NR_DIGITS) {
      int curDigit = d.key_ - '0';
      retNum = max(MIN, retNum * 10 + curDigit);
      Renderer::coverArea(panel_screen, pos, Pos(MAX_NR_DIGITS + 1, 1));
      Renderer::drawText((retNum == 0 ? "" : toString(retNum)) + "_",
                         panel_screen, pos, clr);
      Renderer::updateScreen();
    }
  }
  return -1;
}

void Query::waitForEscOrSpace() const {
  if(Config::isBotPlaying() == false) {
    KeyboardReadReturnData d = Input::readKeysUntilFound(eng);
    while(d.sdlKey_ != SDLK_SPACE && d.sdlKey_ != SDLK_ESCAPE) {
      d = Input::readKeysUntilFound(eng);
    }
  }
}

Pos Query::dir() const {
  KeyboardReadReturnData d = Input::readKeysUntilFound(eng);

  while(d.sdlKey_ != SDLK_RIGHT   && d.sdlKey_ != SDLK_UP       &&
        d.sdlKey_ != SDLK_LEFT    && d.sdlKey_ != SDLK_DOWN     &&
        d.sdlKey_ != SDLK_ESCAPE  && d.sdlKey_ != SDLK_SPACE    &&
        d.sdlKey_ != SDLK_PAGEUP  && d.sdlKey_ != SDLK_HOME     &&
        d.sdlKey_ != SDLK_END     && d.sdlKey_ != SDLK_PAGEDOWN &&
        d.key_ != 'h' && d.key_ != 'j' && d.key_ != 'k' && d.key_ != 'l' &&
        d.key_ != 'y' && d.key_ != 'u' && d.key_ != 'b' && d.key_ != 'n' &&
        (d.key_ < '1' || d.key_ > '9' || d.key_ == '5')) {
    d = Input::readKeysUntilFound(eng);
  }

  if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
    return Pos(0, 0);
  }
  if(d.sdlKey_ == SDLK_RIGHT    || d.key_ == '6' || d.key_ == 'l') {
    if(d.isShiftHeld_) {
      return Pos(1, -1);
    } else if(d.isCtrlHeld_) {
      return Pos(1, 1);
    } else {
      return Pos(1, 0);
    }
  }
  if(d.sdlKey_ == SDLK_PAGEUP   || d.key_ == '9' || d.key_ == 'u') {
    return Pos(1, -1);
  }
  if(d.sdlKey_ == SDLK_UP       || d.key_ == '8' || d.key_ == 'k') {
    return Pos(0, -1);
  }
  if(d.sdlKey_ == SDLK_END      || d.key_ == '7' || d.key_ == 'y') {
    return Pos(-1, -1);
  }
  if(d.sdlKey_ == SDLK_LEFT     || d.key_ == '4' || d.key_ == 'h') {
    if(d.isShiftHeld_) {
      return Pos(-1, -1);
    } else if(d.isCtrlHeld_) {
      return Pos(-1, 1);
    } else {
      return Pos(-1, 0);
    }
  }
  if(d.sdlKey_ == SDLK_END      || d.key_ == '1' || d.key_ == 'b') {
    return Pos(-1, 1);
  }
  if(d.sdlKey_ == SDLK_DOWN     || d.key_ == '2' || d.key_ == 'j') {
    return Pos(0, 1);
  }
  if(d.sdlKey_ == SDLK_PAGEDOWN || d.key_ == '3' || d.key_ == 'n') {
    return Pos(1, 1);
  }

  return Pos(0, 0);
}
