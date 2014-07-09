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
    isDark(false), item(nullptr), featureStatic(nullptr), pos(Pos(-1, -1)) {
    playerVisualMemory.clear();
  }

  void clear();

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

extern Player*            player;
extern int                dlvl;
extern Cell               cells[MAP_W][MAP_H];
extern std::vector<Room*> roomList;               //Owns the rooms
extern Room*              roomMap[MAP_W][MAP_H];  //Helper array

void init();
void cleanup();
void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

void resetMap();

FeatureStatic* put(FeatureStatic* const staticFeature);

//Makes a copy of the renderers current array
//TODO This is weird, and it's unclear how it should be used. Remove?
//Can it not be copied in the map drawing function instead?
void updateVisualMemory();

void mkBlood(const Pos& origin);
void mkGore(const Pos& origin);

void deleteAndRemoveRoomFromList(Room* const room);

} //Map

#endif
