#include "Query.h"

#include <iostream>

#include "Config.h"
#include "Input.h"
#include "Renderer.h"
#include "Converters.h"

using namespace std;

namespace Query {

void waitForKeyPress() {
  if(!Config::isBotPlaying()) {Input::readKeysUntilFound();}
}

YesNoAnswer yesOrNo(char keyForSpecialEvent) {
  if(Config::isBotPlaying()) {return YesNoAnswer::yes;}

  KeyData d = Input::readKeysUntilFound();
  while(
    d.key    != 'y'          &&
    d.key    != 'n'          &&
    d.sdlKey != SDLK_ESCAPE  &&
    d.sdlKey != SDLK_SPACE   &&
    (d.key != keyForSpecialEvent || keyForSpecialEvent == -1)) {
    d = Input::readKeysUntilFound();
  }
  if(d.key == keyForSpecialEvent && keyForSpecialEvent != -1) {
    return YesNoAnswer::special;
  }
  if(d.key == 'y') {
    return YesNoAnswer::yes;
  }

  return YesNoAnswer::no;
}

int number(const Pos& pos, const Clr clr, const int MIN,
           const int MAX_NR_DIGITS, const int DEFAULT,
           const bool CANCEL_RETURNS_DEFAULT) {
  int retNum = max(MIN, DEFAULT);
  Renderer::coverArea(Panel::screen, pos, Pos(MAX_NR_DIGITS + 1, 1));
  const string str = (retNum == 0 ? "" : toStr(retNum)) + "_";
  Renderer::drawText(str, Panel::screen, pos, clr);
  Renderer::updateScreen();

  while(true) {
    KeyData d;
    while((d.key < '0' || d.key > '9') && d.sdlKey != SDLK_RETURN &&
          d.sdlKey != SDLK_SPACE && d.sdlKey != SDLK_ESCAPE &&
          d.sdlKey != SDLK_BACKSPACE) {
      d = Input::readKeysUntilFound();
    }

    if(d.sdlKey == SDLK_RETURN) {
      return max(MIN, retNum);
    }

    if(d.sdlKey == SDLK_SPACE || d.sdlKey == SDLK_ESCAPE) {
      return CANCEL_RETURNS_DEFAULT ? DEFAULT : -1;
    }

    const string retNumStr = toStr(retNum);
    const int CUR_NUM_DIGITS = retNumStr.size();

    if(d.sdlKey == SDLK_BACKSPACE) {
      retNum = retNum / 10;
      Renderer::coverArea(Panel::screen, pos, Pos(MAX_NR_DIGITS + 1, 1));
      Renderer::drawText((retNum == 0 ? "" : toStr(retNum)) + "_",
                         Panel::screen, pos, clr);
      Renderer::updateScreen();
      continue;
    }

    if(CUR_NUM_DIGITS < MAX_NR_DIGITS) {
      int curDigit = d.key - '0';
      retNum = max(MIN, retNum * 10 + curDigit);
      Renderer::coverArea(Panel::screen, pos, Pos(MAX_NR_DIGITS + 1, 1));
      Renderer::drawText((retNum == 0 ? "" : toStr(retNum)) + "_",
                         Panel::screen, pos, clr);
      Renderer::updateScreen();
    }
  }
  return -1;
}

void waitForEscOrSpace() {
  if(!Config::isBotPlaying()) {
    KeyData d = Input::readKeysUntilFound();
    while(d.sdlKey != SDLK_SPACE && d.sdlKey != SDLK_ESCAPE) {
      d = Input::readKeysUntilFound();
    }
  }
}

Pos dir() {
  KeyData d = Input::readKeysUntilFound();

  while(d.sdlKey != SDLK_RIGHT   && d.sdlKey != SDLK_UP       &&
        d.sdlKey != SDLK_LEFT    && d.sdlKey != SDLK_DOWN     &&
        d.sdlKey != SDLK_ESCAPE  && d.sdlKey != SDLK_SPACE    &&
        d.sdlKey != SDLK_PAGEUP  && d.sdlKey != SDLK_HOME     &&
        d.sdlKey != SDLK_END     && d.sdlKey != SDLK_PAGEDOWN &&
        d.key != 'h' && d.key != 'j' && d.key != 'k' && d.key != 'l' &&
        d.key != 'y' && d.key != 'u' && d.key != 'b' && d.key != 'n' &&
        (d.key < '1' || d.key > '9' || d.key == '5')) {
    d = Input::readKeysUntilFound();
  }

  if(d.sdlKey == SDLK_SPACE || d.sdlKey == SDLK_ESCAPE) {
    return Pos(0, 0);
  }
  if(d.sdlKey == SDLK_RIGHT    || d.key == '6' || d.key == 'l') {
    if(d.isShiftHeld) {
      return Pos(1, -1);
    } else if(d.isCtrlHeld) {
      return Pos(1, 1);
    } else {
      return Pos(1, 0);
    }
  }
  if(d.sdlKey == SDLK_PAGEUP   || d.key == '9' || d.key == 'u') {
    return Pos(1, -1);
  }
  if(d.sdlKey == SDLK_UP       || d.key == '8' || d.key == 'k') {
    return Pos(0, -1);
  }
  if(d.sdlKey == SDLK_END      || d.key == '7' || d.key == 'y') {
    return Pos(-1, -1);
  }
  if(d.sdlKey == SDLK_LEFT     || d.key == '4' || d.key == 'h') {
    if(d.isShiftHeld) {
      return Pos(-1, -1);
    } else if(d.isCtrlHeld) {
      return Pos(-1, 1);
    } else {
      return Pos(-1, 0);
    }
  }
  if(d.sdlKey == SDLK_END      || d.key == '1' || d.key == 'b') {
    return Pos(-1, 1);
  }
  if(d.sdlKey == SDLK_DOWN     || d.key == '2' || d.key == 'j') {
    return Pos(0, 1);
  }
  if(d.sdlKey == SDLK_PAGEDOWN || d.key == '3' || d.key == 'n') {
    return Pos(1, 1);
  }

  return Pos(0, 0);
}

} //Query
