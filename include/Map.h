#ifndef MAP_HANDLER_H
#define MAP_HANDLER_H

#include <vector>

#include "CmnData.h"
#include "CmnTypes.h"
#include "Colors.h"
#include "ItemData.h"
#include "Feature.h"
#include "Config.h"
#include "ActorPlayer.h"

class SaveHandler;
class Rigid;

struct Cell
{
  Cell();
  ~Cell();

  bool            isExplored, isSeenByPlayer, isLit, isDark;
  Item*           item;
  Rigid*          rigid;
  CellRenderData  playerVisualMemory;
  Pos             pos;
};

enum class MapType {intro, std, egypt, leng, caves, trapezohedron};

namespace Map
{

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

Rigid* put(Rigid* const rigid);

//Makes a copy of the renderers current array
//TODO This is weird, and it's unclear how it should be used. Remove?
//Can it not be copied in the map drawing function instead?
void updateVisualMemory();

void mkBlood(const Pos& origin);
void mkGore(const Pos& origin);

void deleteAndRemoveRoomFromList(Room* const room);

bool isPosSeenByPlayer(const Pos& p);

} //Map

#endif
