#ifndef POPUP_H
#define POPUP_H

class Engine;

#include <vector>

#include "ConstTypes.h"

using namespace std;

class Popup {
public:
  Popup(Engine* engine) :
    eng(engine) {
  }

  void showMessage(const string message, const bool DRAW_MAP_AND_INTERFACE) const;

private:
  struct BoxReturnData {
public:
    BoxReturnData(coord x0y0Text_, coord x1y1Text_) :
      x0y0Text(x0y0Text_), x1y1Text(x1y1Text_) {
    }

    const coord x0y0Text;
    const coord x1y1Text;
private:
    BoxReturnData& operator=(BoxReturnData& other) {
      (void)other;
      return *this;
    }
  };

  BoxReturnData printBox(const int BOX_HALF_WIDTH) const;

  Engine* eng;
};

#endif
