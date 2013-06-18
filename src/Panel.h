#ifndef INTERFACE_PANEL_H
#define INTERFACE_PANEL_H

#include "CommonTypes.h"

class Panel {
public:
  Panel() {}
  ~Panel() {}
  virtual void draw() const;
  coord getPixelDims() const;
private:
};

class LogPanel : public Panel  {
public:
private:
};

class CharacterLinesPanel : public Panel {
public:
private:
};

#endif
