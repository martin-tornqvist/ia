#ifndef ROOM_THEME_H
#define ROOM_THEME_H

#include <vector>

#include "CommonData.h"
#include "CommonTypes.h"

using namespace std;

class Engine;
struct FeatureData;

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
  RoomThemeMaker(Engine& engine) : eng(engine) {}

  void run();

private:
  //This array supports placing items, monsters and traps
  RoomTheme_t themeMap[MAP_W][MAP_H];

  void applyThemeToRoom(Room& room);

  int placeThemeFeatures(Room& room);

  int getRandomNrFeaturesForTheme(const RoomTheme_t theme) const;

  void makeThemeSpecificRoomModifications(Room& room);

  void makeRoomDarkWithChance(const Room& room);

  int trySetFeatureToPlace(
    const FeatureData** data, Pos& pos, vector<Pos>& nextToWalls,
    vector<Pos>& awayFromWalls,
    vector<const FeatureData*> featureDataBelongingToTheme);

  void eraseAdjacentCellsFromVectors(
    const Pos& pos,  vector<Pos>& nextToWalls,
    vector<Pos>& awayFromWalls);

  void assignRoomThemes();

  bool isThemeAllowed(const Room* const room, const RoomTheme_t theme,
                      const bool blockers[MAP_W][MAP_H]) const;

  bool isThemeExistInMap(const RoomTheme_t theme) const;

  Engine& eng;
};


#endif
