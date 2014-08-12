#ifndef MENU_BROWSER_H
#define MENU_BROWSER_H

#include "CmnTypes.h"

class MenuBrowser {
public:
  MenuBrowser(const int NR_ITEMS_A, const int NR_ITEMS_B) :
    x0y0_(NR_ITEMS_A > 0 ? Pos(0, 0) : Pos(1, 0)),
    x1y0_(NR_ITEMS_B > 0 ? Pos(1, 0) : Pos(0, 0)),
    x0y1_(NR_ITEMS_A > 0 ? Pos(0, NR_ITEMS_A - 1) : Pos(1, NR_ITEMS_B - 1)),
    x1y1_(NR_ITEMS_B > 0 ? Pos(1, NR_ITEMS_B - 1) : Pos(0, NR_ITEMS_A - 1)),
    NR_A_(NR_ITEMS_A),
    NR_B_(NR_ITEMS_B),
    pos_(NR_ITEMS_A > 0 ? x0y0_ : x1y0_) {}

  MenuBrowser() = delete;

  void navigate(const Dir dir);

  int getElement() const;

  int getNrOfItemsInFirstList()   const {return NR_A_;}
  int getNrOfItemsInSecondList()  const {return NR_B_;}
  Pos getPos()                    const {return pos_;}

  bool isAtIdx(const int ELEMENT) const;

  int getY() const {return pos_.y;}

  void setPos(const Pos& p);

  void setGoodPos();

private:
  const Pos x0y0_;
  const Pos x1y0_;
  const Pos x0y1_;
  const Pos x1y1_;
  int NR_A_;
  int NR_B_;
  Pos pos_;
};

#endif
