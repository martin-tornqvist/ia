#include "MenuBrowser.h"

using namespace std;

void MenuBrowser::setY(const int Y) {
  const int NR_ITEMS_THIS_X = pos_.x == 0 ? nrItemsA_ : nrItemsB_;
  pos_.y =  min(NR_ITEMS_THIS_X - 1, max(0, Y));
}

int MenuBrowser::getElement() const {
  return pos_.y + (pos_.x * nrItemsA_);
}

bool MenuBrowser::isPosAtElement(const int ELEMENT) const {
  Pos elementPos(
    ELEMENT < nrItemsA_ ? 0 : 1,
    ELEMENT < nrItemsA_ ? ELEMENT : ELEMENT - nrItemsA_);

  return pos_ == elementPos;
}

void MenuBrowser::navigate(const Dir dir) {
  switch(dir) {
    case Dir::up: {
      pos_.y--;
      if(pos_.y < 0) {pos_ = pos_.x == 0 ? btmRight_ : btmLeft_;}
    } break;

    case Dir::down: {
      pos_.y++;
      if(pos_.x == 0) {
        if(pos_.y > btmLeft_.y)   {pos_ = upRight_;}
      } else {
        if(pos_.y > btmRight_.y)  {pos_ = upLeft_;}
      }
    } break;

    case Dir::left:
    case Dir::right: {
      pos_.x = pos_.x == 0 ? 1 : 0;
      pos_.x = min(pos_.x, upRight_.x);
      pos_.y = pos_.x == 0 ? min(pos_.y, btmLeft_.y) : min(pos_.y, btmRight_.y);
    } break;

    case Dir::upRight:
    case Dir::upLeft:
    case Dir::downLeft:
    case Dir::downRight:
    case Dir::center:
    case Dir::endOfDirs: {} break;
  }
}

