#ifndef ROOM_THEME_H
#define ROOM_THEME_H

#include <vector>

#include "ConstTypes.h"
#include "ConstDungeonSettings.h"

using namespace std;

class Engine;
class FeatureDef;

enum RoomTheme_t {
  roomTheme_plain,
  roomTheme_human,
  roomTheme_ritual,
  roomTheme_spider,
//  roomTheme_dungeon,
  roomTheme_crypt,
  roomTheme_monster,
  roomTheme_flooded,
  roomTheme_muddy,
//  roomTheme_chasm,

  endOfRoomThemes
};

struct Room;

class RoomThemeMaker {
public:
  RoomThemeMaker(Engine* engine) : eng(engine) {
  }

  void run(const vector<Room*>& rooms);

  vector<Room*> roomList;

private:
  //This array supports placing items, monsters and traps
  RoomTheme_t themeMap[MAP_X_CELLS][MAP_Y_CELLS];

  void applyThemeToRoom(Room& room);

  void placeThemeFeatures(Room& room);

  void makeThemeSpecificRoomModifications(Room& room);

  void makeRoomDarkWithChance(const Room& room);

  int attemptSetFeatureToPlace(const FeatureDef** def, coord& pos, vector<coord>& nextToWalls,
                               vector<coord>& awayFromWalls, vector<const FeatureDef*> featureDefsBelongingToTheme);

  void eraseAdjacentCellsFromVectors(const coord& pos,  vector<coord>& nextToWalls, vector<coord>& awayFromWalls);

  void assignRoomThemes();

  bool isRoomEligibleForTheme(const Room* const room, const RoomTheme_t theme, const bool blockers[MAP_X_CELLS][MAP_Y_CELLS]) const;

  Engine* eng;
};


#endif
