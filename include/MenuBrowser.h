#ifndef MENU_BROWSER_H
#define MENU_BROWSER_H

#include "CmnTypes.h"

//NOTE: If "ELEM_ON_WRAP_UP_A" is set, the browser will jump to that position if the user
//presses up while the top element is marked. For example:
//
//  Element 0
//  Element 1
//  Element 2
//  Element 3
//
//If "ELEM_ON_WRAP_UP_A" is set to 2, then when "Element 0" is marked and the user
//presses up, the browser will jump to "Element 2".

class MenuBrowser
{
public:
    MenuBrowser(const int NR_ITEMS_A, const int NR_ITEMS_B,
                const int ELEM_ON_WRAP_UP_A = -1) :
        x0y0_(NR_ITEMS_A > 0 ? Pos(0, 0) : Pos(1, 0)),
        x1y0_(NR_ITEMS_B > 0 ? Pos(1, 0) : Pos(0, 0)),
        x0y1_(NR_ITEMS_A > 0 ? Pos(0, NR_ITEMS_A - 1) : Pos(1, NR_ITEMS_B - 1)),
        x1y1_(NR_ITEMS_B > 0 ? Pos(1, NR_ITEMS_B - 1) : Pos(0, NR_ITEMS_A - 1)),
        nrA_(NR_ITEMS_A),
        nrB_(NR_ITEMS_B),
        elemOnWrapUpA_(ELEM_ON_WRAP_UP_A),
        pos_(NR_ITEMS_A > 0 ? x0y0_ : x1y0_) {}

    MenuBrowser() = delete;

    MenuBrowser& operator=(const MenuBrowser&) = default;

    void navigate(const Dir dir);

    int getElement() const;

    int getNrOfItemsInFirstList()   const {return nrA_;}
    int getNrOfItemsInSecondList()  const {return nrB_;}
    Pos getPos()                    const {return pos_;}

    bool isAtIdx(const int ELEMENT) const;

    int getY() const {return pos_.y;}

    void setPos(const Pos& p);

    void setGoodPos();

private:
    Pos x0y0_;
    Pos x1y0_;
    Pos x0y1_;
    Pos x1y1_;
    int nrA_;
    int nrB_;
    int elemOnWrapUpA_;
    Pos pos_;
};

#endif
