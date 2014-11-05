#include "MenuBrowser.h"

using namespace std;

void MenuBrowser::setPos(const Pos& p)
{
  pos_ = p;
  setGoodPos();
}

void MenuBrowser::setGoodPos()
{
  const int NR_ITEMS = pos_.x == 0 ? nrA_ : nrB_;

  if(NR_ITEMS == 0)
  {
    pos_.x = pos_.x == 0 ? 1 : 0;
    pos_.y = 0;
  }
  else
  {
    pos_.y =  min(NR_ITEMS - 1, max(0, pos_.y));
  }
}

int MenuBrowser::getElement() const
{
  return pos_.y + (pos_.x * nrA_);
}

bool MenuBrowser::isAtIdx(const int IDX) const
{
  const Pos idxPos(IDX < nrA_ ? 0 : 1, IDX < nrA_ ? IDX : IDX - nrA_);

  return pos_ == idxPos;
}

void MenuBrowser::navigate(const Dir dir)
{
  const int NR_ITEMS_OTHER  = pos_.x == 0 ? nrB_ : nrA_;

  switch(dir)
  {
    case Dir::up:
    {
      pos_.y--;
      if(pos_.y < 0)
      {
        pos_ = pos_.x == 0 ? x1y1_ : x0y1_;

        //Special wrap element set for A list
        if(pos_.x == 0 && elemOnWrapUpA_ >= 0) {pos_.y = elemOnWrapUpA_;}
      }
    } break;

    case Dir::down:
    {
      pos_.y++;
      if(pos_.x == 0)
      {
        if(pos_.y > x0y1_.y)   {pos_ = x1y0_;}
      }
      else
      {
        if(pos_.y > x1y1_.y)  {pos_ = x0y0_;}
      }
    } break;

    case Dir::left:
    case Dir::right:
    {
      if(NR_ITEMS_OTHER > 0)
      {
        pos_.x = pos_.x == 0 ? 1 : 0;
        pos_.x = min(pos_.x, x1y0_.x);
        pos_.y = pos_.x == 0 ? min(pos_.y, x0y1_.y) : min(pos_.y, x1y1_.y);
      }
    } break;

    case Dir::upRight:
    case Dir::upLeft:
    case Dir::downLeft:
    case Dir::downRight:
    case Dir::center:
    case Dir::END: {} break;
  }
  setGoodPos();
}

