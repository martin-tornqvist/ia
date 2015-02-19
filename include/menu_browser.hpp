#ifndef MENU_BROWSER_H
#define MENU_BROWSER_H

#include "cmn_types.hpp"

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

class Menu_browser
{
public:
    Menu_browser(const int NR_ITEMS_A, const int NR_ITEMS_B,
                 const int ELEM_ON_WRAP_UP_A = -1) :
        x0y0_(NR_ITEMS_A > 0 ? Pos(0, 0) : Pos(1, 0)),
        x1y0_(NR_ITEMS_B > 0 ? Pos(1, 0) : Pos(0, 0)),
        x0y1_(NR_ITEMS_A > 0 ? Pos(0, NR_ITEMS_A - 1) : Pos(1, NR_ITEMS_B - 1)),
        x1y1_(NR_ITEMS_B > 0 ? Pos(1, NR_ITEMS_B - 1) : Pos(0, NR_ITEMS_A - 1)),
        nr_a_(NR_ITEMS_A),
        nr_b_(NR_ITEMS_B),
        elem_on_wrap_up_a_(ELEM_ON_WRAP_UP_A),
        pos_(NR_ITEMS_A > 0 ? x0y0_ : x1y0_) {}

    Menu_browser() = delete;

    Menu_browser& operator=(const Menu_browser&) = default;

    void navigate(const Dir dir);

    int get_element() const;

    int get_nr_of_items_in_first_list()   const {return nr_a_;}
    int get_nr_of_items_in_second_list()  const {return nr_b_;}
    Pos get_pos()                    const {return pos_;}

    bool is_at_idx(const int ELEMENT) const;

    int get_y() const {return pos_.y;}

    void set_pos(const Pos& p);

    void set_good_pos();

private:
    Pos x0y0_;
    Pos x1y0_;
    Pos x0y1_;
    Pos x1y1_;
    int nr_a_;
    int nr_b_;
    int elem_on_wrap_up_a_;
    Pos pos_;
};

#endif
