#include "Blood.h"

#include "Engine.h"
#include "Map.h"

void Gore::makeBlood(const Pos& origin) {
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      const Pos c = origin + Pos(dx, dy);
      FeatureStatic* const f  = eng->map->cells[c.x][c.y].featureStatic;
      if(f->canHaveBlood()) {
        if(eng->dice.percentile() > 66) {
          f->setHasBlood(true);
        }
      }
    }
  }
}

void Gore::makeGore(const Pos& origin) {
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      const Pos c = origin + Pos(dx, dy);
      if(eng->dice.percentile() > 66) {
        eng->map->cells[c.x][c.y].featureStatic->setGoreIfPossible();
      }
    }
  }
}
