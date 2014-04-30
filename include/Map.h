#ifndef MAP_HANDLER_H
#define MAP_HANDLER_H

#include <vector>
#include <iostream>

#include "CmnData.h"
#include "CmnTypes.h"
#include "Colors.h"
#include "ItemData.h"
#include "Feature.h"
#include "Config.h"
#include "ActorPlayer.h"

class SaveHandler;
class FeatureStatic;

struct Cell {
  Cell() : isExplored(false), isSeenByPlayer(false), isLight(false),
    isDark(false), item(NULL), featureStatic(NULL), pos(Pos(-1, -1)) {
    playerVisualMemory.clear();
  }

  inline void clear();

  bool isExplored;
  bool isSeenByPlayer;
  bool isLight;
  bool isDark;
  Item* item;
  FeatureStatic* featureStatic;
  CellRenderData playerVisualMemory;
  Pos pos;
};

namespace Map {

extern Player*        player;
extern int            dlvl;
extern Cell           cells[MAP_W][MAP_H];
extern std::vector<Room*>  rooms;

void init();
void cleanup();
void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

void resetMap();

//Makes a copy of the renderers current array
//TODO This is weird, and it's unclear how it should be used. Remove?
//Can it not be copied in the map drawing function instead?
void updateVisualMemory();

void switchToDestroyedFeatAt(const Pos& pos);

void makeBlood(const Pos& origin);
void makeGore(const Pos& origin);

} //Map

#endif
