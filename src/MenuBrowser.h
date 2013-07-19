#ifndef MENU_BROWSER_H
#define MENU_BROWSER_H

#include "CommonTypes.h"

class MenuBrowser {
public:
  MenuBrowser(int nrItemsFirst, int nrItemsSecond) :
    upLeft(Pos(0, 0)),
    upRight(nrItemsSecond == 0 ? Pos(0, 0) : Pos(1, 0)),
    btmLeft(Pos(0, nrItemsFirst - 1)),
    btmRight(nrItemsSecond == 0 ? Pos(0, nrItemsFirst - 1) : Pos(1, nrItemsSecond - 1)),
    pos(upLeft), NR_ITEMS_FIRST(nrItemsFirst),
    NR_ITEMS_SECOND(nrItemsSecond) {
  }

  MenuBrowser& operator=(const MenuBrowser& other) {
    (void)other;
    return *this;
  }

  void navigate(const Directions_t direction);
  void navigate(const char KEY);

  char enter() const;

  int getNrOfItemsInFirstList() const {
    return NR_ITEMS_FIRST;
  }
  int getNrOfItemsInSecondList() const {
    return NR_ITEMS_SECOND;
  }
  Pos getPos() const {
    return pos;
  }

  bool isPosAtKey(const char KEY) const;

  void setY(const int Y);

private:
  const Pos upLeft;
  const Pos upRight;
  const Pos btmLeft;
  const Pos btmRight;
  Pos pos;
  const int NR_ITEMS_FIRST;
  const int NR_ITEMS_SECOND;
};

#endif
