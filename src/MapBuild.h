#ifndef MAPBUILD_H
#define MAPBUILD_H

#include <algorithm>

#include "MapTemplates.h"
#include "GameTime.h"
#include "CommonTypes.h"
#include "CommonSettings.h"

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
    const bool TUNNEL_THROUGH_ANY_FEATURE, const bool ONLY_STRAIGHT = true, const coord x0y0Lim =
      coord(1, 1), const coord x1y1Lim = coord(MAP_X_CELLS - 2, MAP_Y_CELLS - 2));

private:
  void makeStraightPathByPathfinder(
    const coord origin, const coord target, Feature_t feature, const bool SMOOTH,
    const bool TUNNEL_THROUGH_ANY_FEATURE);

  void buildForestLimit();
  void buildForestOuterTreeline();
  void buildForestTreePatch();
  void buildForestTrees(const coord& stairsCoord);

  void buildFromTemplate(const coord pos, MapTemplate* t);
  void buildFromTemplate(const coord pos, MapTemplateId_t templateId);

  Feature_t backup[MAP_X_CELLS][MAP_Y_CELLS];
  void backupMap();
  void restoreMap();

  Engine* eng;
};

#endif
