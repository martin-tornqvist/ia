#ifndef MENU_BROWSER_H
#define MENU_BROWSER_H

#include "CmnTypes.h"

class MenuBrowser {
public:
  MenuBrowser(int nrItemsA, int nrItemsB) :
    upLeft_(Pos(0, 0)),
    upRight_(nrItemsB == 0 ? Pos(0, 0) : Pos(1, 0)),
    btmLeft_(Pos(0, nrItemsA - 1)),
    btmRight_(nrItemsB == 0 ? Pos(0, nrItemsA - 1) : Pos(1, nrItemsB - 1)),
    nrItemsA_(nrItemsA),
    nrItemsB_(nrItemsB),
    pos_(upLeft_) {}

  void navigate(const Dir dir);

  int getElement() const;

  int getNrOfItemsInFirstList()   const {return nrItemsA_;}
  int getNrOfItemsInSecondList()  const {return nrItemsB_;}
  Pos getPos()                    const {return pos_;}

  bool isPosAtElement(const int ELEMENT) const;

  void setY(const int Y);

private:
  Pos upLeft_;
  Pos upRight_;
  Pos btmLeft_;
  Pos btmRight_;
  int nrItemsA_;
  int nrItemsB_;
  Pos pos_;
};

#endif
