#ifndef ROOM_THEME_H
#define ROOM_THEME_H

#include <vector>

#include "CommonData.h"
#include "CommonTypes.h"

using namespace std;

class Engine;
struct FeatureData;

enum class RoomThemeId {
  plain,
  human,
  ritual,
  spider,
//  dungeon,
  crypt,
  monster,
  flooded,
  muddy,
//  chasm,

  endOfRoomThemes
};

struct Room;

class RoomThemeMaker {
public:
  RoomThemeMaker(Engine& engine) : eng(engine) {}

  void run();

  //This array is a support for placing items, monsters and traps
  RoomThemeId themeMap[MAP_W][MAP_H];

private:
  void applyThemeToRoom(Room& room);

  int placeThemeFeatures(Room& room);

  int getRandomNrFeaturesForTheme(const RoomThemeId theme) const;

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

  bool isThemeAllowed(const Room* const room, const RoomThemeId theme,
                      const bool blockers[MAP_W][MAP_H]) const;

  int nrThemeInMap(const RoomThemeId theme) const;

  Engine& eng;
};


#endif
