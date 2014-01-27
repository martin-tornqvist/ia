#ifndef MAP_HANDLER_H
#define MAP_HANDLER_H

#include <vector>
#include <iostream>

#include "CommonData.h"
#include "CommonTypes.h"
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
    isDark(false), item(NULL), featureStatic(NULL), pos(Pos(-1, -1)) {
    playerVisualMemoryAscii.clear();
    playerVisualMemoryTiles.clear();
  }

  inline void clear();

  bool isExplored;
  bool isSeenByPlayer;
  bool isLight;
  bool isDark;
  Item* item;
  FeatureStatic* featureStatic;
  CellRenderDataAscii playerVisualMemoryAscii;
  CellRenderDataTiles playerVisualMemoryTiles;
  Pos pos;
};

class Map {
public:
  Map(Engine& engine);

  ~Map();

  Cell cells[MAP_W][MAP_H];

  inline int getDlvl() {return dlvl_;}

  inline void incrDlvl(const int levels = 1) {dlvl_ += levels;}
  inline void decrDlvl(const int levels = 1) {dlvl_ -= levels;}

  void resetMap();

  void switchToDestroyedFeatAt(const Pos pos);

  void addSaveLines(vector<string>& lines) const {
    lines.push_back(toString(dlvl_));
  }

  void setParamsFromSaveLines(vector<string>& lines) {
    dlvl_ = toInt(lines.front());
    lines.erase(lines.begin());
  }

  vector<Room*> rooms;

private:
  void resetCells(const bool MAKE_STONE_WALLS);

  Engine& eng;

  friend class SaveHandler;
  friend class Bot;
  friend class Renderer;
  int dlvl_;
};

#endif
