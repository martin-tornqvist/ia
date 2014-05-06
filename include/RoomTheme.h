#ifndef ROOM_THEME_H
#define ROOM_THEME_H

#include <vector>

#include "CmnData.h"
#include "CmnTypes.h"

struct FeatureDataT;

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

namespace RoomThemeMaking {

//This array is a support for placing items, monsters and traps
extern RoomThemeId themeMap[MAP_W][MAP_H];

void run();

} //RoomThemeMaking


#endif
