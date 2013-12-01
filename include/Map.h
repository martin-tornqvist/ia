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

using namespace std;

class Engine;
class SaveHandler;
class FeatureStatic;

struct Cell {
  Cell() : isExplored(false), isSeenByPlayer(false), isLight(false),
    isDark(false), item(NULL), featureStatic(NULL) {
    playerVisualMemoryAscii.clear();
    playerVisualMemoryTiles.clear();
  }

  inline void reset();

  bool isExplored;
  bool isSeenByPlayer;
  bool isLight;
  bool isDark;
  Item* item;
  FeatureStatic* featureStatic;
  CellRenderDataAscii playerVisualMemoryAscii;
  CellRenderDataTiles playerVisualMemoryTiles;
};

class Map {
public:
  Map(Engine* engine);

  ~Map();

  Cell cells[MAP_X_CELLS][MAP_Y_CELLS];

  inline int getDLVL() {return dlvl_;}

  inline void incrDlvl(const int levels = 1) {dlvl_ += levels;}
  inline void decrDlvl(const int levels = 1) {dlvl_ -= levels;}

  void clearMap();

  void switchToDestroyedFeatAt(const Pos pos);

  void addSaveLines(vector<string>& lines) const {
    lines.push_back(toString(dlvl_));
  }

  void setParametersFromSaveLines(vector<string>& lines) {
    dlvl_ = toInt(lines.front());
    lines.erase(lines.begin());
  }

  vector<Room*> rooms;

private:
  void clearCells(const bool DELETE_INSTANCES);

  Engine* eng;

  friend class SaveHandler;
  friend class Bot;
  friend class Renderer;
  int dlvl_;
};

#endif
