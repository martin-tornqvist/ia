#ifndef MAP_H
#define MAP_H

#include <vector>
#include <iostream>

#include "CommonTypes.h"
#include "CommonData.h"
#include "Colors.h"
#include "ItemData.h"
#include "Feature.h"
#include "Config.h"
#include "Render.h"

using namespace std;

class Engine;
class SaveHandler;
class FeatureStatic;

class Map {
public:
  Map(Engine* engine);

  ~Map();

  bool explored[MAP_X_CELLS][MAP_Y_CELLS];

  bool playerVision[MAP_X_CELLS][MAP_Y_CELLS];

  Item* items[MAP_X_CELLS][MAP_Y_CELLS];

  FeatureStatic* featuresStatic[MAP_X_CELLS][MAP_Y_CELLS];

  CellRenderDataAscii playerVisualMemoryAscii[MAP_X_CELLS][MAP_Y_CELLS];
  CellRenderDataTiles playerVisualMemoryTiles[MAP_X_CELLS][MAP_Y_CELLS];

  bool light[MAP_X_CELLS][MAP_Y_CELLS];

  bool darkness[MAP_X_CELLS][MAP_Y_CELLS];

  int getDungeonLevel() {
    return dungeonLevel_;
  }

  void incrDungeonLevel(const int levels = 1) {
    dungeonLevel_ += levels;
  }
  void decrDungeonLevel(const int levels = 1) {
    dungeonLevel_ -= levels;
  }

  void clearDungeon();

  void switchToDestroyedFeatAt(const coord pos);

  void addSaveLines(vector<string>& lines) const {
    lines.push_back(intToString(dungeonLevel_));
  }

  void setParametersFromSaveLines(vector<string>& lines) {
    dungeonLevel_ = stringToInt(lines.front());
    lines.erase(lines.begin());
  }

private:
  void clearGrids(const bool DELETE_INSTANCES);

  Engine* eng;

  friend class SaveHandler;
  friend class Bot;
  friend class Renderer;
  int dungeonLevel_;
};

#endif
