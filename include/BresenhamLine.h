#ifndef BRESENHAM_LINE_H
#define BRESENHAM_LINE_H

#include <vector>

#include "CommonData.h"

using namespace std;

class Engine;

class BresenhamLine {
public:
  BresenhamLine(Engine* engine) : eng(engine) {}

  //void getLine(vector<Pos>& lineVector, int x1, int y1, int x2, int y2);

private:
  Engine* eng;
};


#endif
