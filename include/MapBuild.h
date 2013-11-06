#ifndef MAPBUILD_H
#define MAPBUILD_H

#include <algorithm>

#include "MapTemplates.h"
#include "GameTime.h"
#include "CommonData.h"
#include "CommonTypes.h"

using namespace std;

class Engine;
class Door;

class MapBuild {
public:
  MapBuild(Engine* engine) : eng(engine) {
  }

  void buildForest();

  void buildCavern();

  void buildKingsTomb();

  void buildTrapezohedronLevel();

  void makePathByRandomWalk(
    int originX, int originY, int len, Feature_t featureToMake,
    const bool TUNNEL_THROUGH_ANY_FEATURE, const bool ONLY_STRAIGHT = true, const Pos x0y0Lim =
      Pos(1, 1), const Pos x1y1Lim = Pos(MAP_X_CELLS - 2, MAP_Y_CELLS - 2));

private:
  void makeStraightPathByPathfinder(
    const Pos origin, const Pos target, Feature_t feature, const bool SMOOTH,
    const bool TUNNEL_THROUGH_ANY_FEATURE);

  void buildForestLimit();
  void buildForestOuterTreeline();
  void buildForestTreePatch();
  void buildForestTrees(const Pos& stairsPos);

  void buildFromTemplate(const Pos pos, MapTemplate* t);
  void buildFromTemplate(const Pos pos, MapTemplateId_t templateId);

  Feature_t backup[MAP_X_CELLS][MAP_Y_CELLS];
  void backupMap();
  void restoreMap();

  Engine* eng;
};

#endif
