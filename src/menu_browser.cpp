#include "menu_browser.h"

using namespace std;

void Menu_browser::set_pos(const Pos& p)
{
    pos_ = p;
    set_good_pos();
}

void Menu_browser::set_good_pos()
{
    const int NR_ITEMS = pos_.x == 0 ? nr_a_ : nr_b_;

    if (NR_ITEMS == 0)
    {
        pos_.x = pos_.x == 0 ? 1 : 0;
        pos_.y = 0;
    }
    else
    {
        pos_.y =  min(NR_ITEMS - 1, max(0, pos_.y));
    }
}

int Menu_browser::get_element() const
{
    return pos_.y + (pos_.x * nr_a_);
}

bool Menu_browser::is_at_idx(const int IDX) const
{
    const Pos idx_pos(IDX < nr_a_ ? 0 : 1, IDX < nr_a_ ? IDX : IDX - nr_a_);

    return pos_ == idx_pos;
}

void Menu_browser::navigate(const Dir dir)
{
    const int NR_ITEMS_OTHER  = pos_.x == 0 ? nr_b_ : nr_a_;

    switch (dir)
    {
    case Dir::up:
    {
        pos_.y--;
        if (pos_.y < 0)
        {
            pos_ = pos_.x == 0 ? x1y1_ : x0y1_;

            //Special wrap element set for A list
            if (pos_.x == 0 && elem_on_wrap_up_a_ >= 0) {pos_.y = elem_on_wrap_up_a_;}
        }
    } break;

    case Dir::down:
    {
        pos_.y++;
        if (pos_.x == 0)
        {
            if (pos_.y > x0y1_.y)   {pos_ = x1y0_;}
        }
        else
        {
            if (pos_.y > x1y1_.y)  {pos_ = x0y0_;}
        }
    } break;

    case Dir::left:
    case Dir::right:
    {
        if (NR_ITEMS_OTHER > 0)
        {
            pos_.x = pos_.x == 0 ? 1 : 0;
            pos_.x = min(pos_.x, x1y0_.x);
            pos_.y = pos_.x == 0 ? min(pos_.y, x0y1_.y) : min(pos_.y, x1y1_.y);
        }
    } break;

    case Dir::up_right:
    case Dir::up_left:
    case Dir::down_left:
    case Dir::down_right:
    case Dir::center:
    case Dir::END: {} break;
    }
    set_good_pos();
}

