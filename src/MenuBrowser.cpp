#include "MenuBrowser.h"

void MenuBrowser::setY(const int Y) {
  const int NR_ITEMS_THIS_X = pos.x == 0 ? NR_ITEMS_FIRST : NR_ITEMS_SECOND;
  pos.y =  min(NR_ITEMS_THIS_X - 1, max(0, Y));
}

void MenuBrowser::navigate(const char KEY) {
  const int ELEMENT = KEY >= 'a' ? int(KEY - 'a') : KEY >= 'A' ? int(KEY - 'A' + 'z' - 'a' + 1) : 0;
  pos.x = ELEMENT < NR_ITEMS_FIRST ? 0 : 1;
  pos.y = ELEMENT < NR_ITEMS_FIRST ? ELEMENT : ELEMENT - NR_ITEMS_FIRST;
}

char MenuBrowser::enter() const {
  return 'a' + pos.y + (pos.x * NR_ITEMS_FIRST);
}

bool MenuBrowser::isPosAtKey(const char KEY) const {
  const int ELEMENT = KEY >= 'a' ? int(KEY - 'a') : KEY >= 'A' ? int(KEY - 'A' + 'z' - 'a' + 1) : 0;
  coord positionAtKey;
  positionAtKey.x = ELEMENT < NR_ITEMS_FIRST ? 0 : 1;
  positionAtKey.y = ELEMENT < NR_ITEMS_FIRST ? ELEMENT : ELEMENT - NR_ITEMS_FIRST;

  return positionAtKey == pos;
}

void MenuBrowser::navigate(const Directions_t direction) {
  switch(direction) {
  case direction_up: {
    pos.y--;
    if(pos.y < 0) {
      if(pos.x == 0) {
        pos = btmRight;
      } else {
        pos = btmLeft;
      }
    }
  }
  break;
  case direction_down: {
    pos.y++;
    if(pos.x == 0) {
      if(pos.y > btmLeft.y) {
        pos = upRight;
      }
    } else {
      if(pos.y > btmRight.y) {
        pos = upLeft;
      }
    }
  }
  break;
  case direction_left:
  case direction_right: {
    pos.x = pos.x == 0 ? 1 : 0;

    pos.x = min(pos.x, upRight.x);

    if(pos.x == 0) {
      pos.y = min(pos.y, btmLeft.y);
    } else {
      pos.y = min(pos.y, btmRight.y);
    }
  }
  }
}

