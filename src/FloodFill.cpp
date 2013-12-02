#include "FloodFill.h"

#include "Engine.h"

void FloodFill::run(
  const Pos& origin, bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
  int values[MAP_X_CELLS][MAP_Y_CELLS], int travelLimit, const Pos& target) {

  eng->basicUtils->resetArray(values);

  vector<Pos> positions;
  positions.resize(0);
  unsigned int nrElementsToSkip = 0;
  Pos c;

  int currentX = origin.x;
  int currentY = origin.y;

  int currentValue = 0;

  bool pathExists = true;
  bool isAtTarget = false;

  bool isStoppingAtTarget = target.x != -1;

  const Rect bounds(Pos(1, 1), Pos(MAP_X_CELLS - 2, MAP_Y_CELLS - 2));

  bool done = false;
  while(done == false) {
    for(int dx = -1; dx <= 1; dx++) {
      for(int dy = -1; dy <= 1; dy++) {
        if((dx != 0 || dy != 0)) {
          const Pos newPos(currentX + dx, currentY + dy);
          if(
            blockers[newPos.x][newPos.y] == false &&
            eng->basicUtils->isPosInside(Pos(newPos.x, newPos.y), bounds) &&
            values[newPos.x][newPos.y] == 0) {
            currentValue = values[currentX][currentY];

            if(currentValue < travelLimit) {
              values[newPos.x][newPos.y] = currentValue + 1;
            }

            if(isStoppingAtTarget) {
              if(currentX == target.x - dx && currentY == target.y - dy) {
                isAtTarget = true;
                dx = 9999;
                dy = 9999;
              }
            }

            if(isStoppingAtTarget == false || isAtTarget == false) {
              positions.push_back(newPos);
            }
          }
        }
      }
    }

    if(isStoppingAtTarget) {
      if(positions.size() == nrElementsToSkip) {
        pathExists = false;
      }
      if(isAtTarget || pathExists == false) {
        done = true;
      }
    } else if(positions.size() == nrElementsToSkip) {
      done = true;
    }

    if(currentValue == travelLimit) {
      done = true;
    }

    if(isStoppingAtTarget == false || isAtTarget == false) {
      if(positions.size() == nrElementsToSkip) {
        pathExists = false;
      } else {
        c = positions.at(nrElementsToSkip);
        currentX = c.x;
        currentY = c.y;
        //positions.erase(positions.begin()); //Slow!
        nrElementsToSkip++;
      }
    }
  }
}

