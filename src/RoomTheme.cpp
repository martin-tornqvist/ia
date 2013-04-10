#include "RoomTheme.h"

#include "Engine.h"
#include "MapBuildBSP.h"
#include "FeatureData.h"
#include "FeatureFactory.h"
#include "Map.h"
#include "PopulateItems.h"
#include "PopulateMonsters.h"
#include "PopulateTraps.h"
#include "Blood.h"

void RoomThemeMaker::run(const vector<Room*>& rooms) {
  tracer << "RoomThemeMaker::run()..." << endl;

  roomList = rooms;

  assignRoomThemes();

  for(unsigned int i = 0; i < rooms.size(); i++) {
    applyThemeToRoom(*(rooms.at(i)));
    makeRoomDarkWithChance(*(rooms.at(i)));
  }

  tracer << "RoomThemeMaker: Calling PopulateMonsters::populateRoomAndCorridorLevel()" << endl;
  eng->populateMonsters->populateRoomAndCorridorLevel(themeMap, rooms);

  tracer << "RoomThemeMaker: Calling PopulateTraps::populateRoomAndCorridorLevel()" << endl;
  eng->populateTraps->populateRoomAndCorridorLevel(themeMap, rooms);

  tracer << "RoomThemeMaker::run() [DONE]" << endl;
}

void RoomThemeMaker::applyThemeToRoom(Room& room) {
  if(room.roomTheme != roomTheme_plain) {
    placeThemeFeatures(room);
    makeThemeSpecificRoomModifications(room);
  }

  switch(room.roomTheme) {
    case roomTheme_plain: {} break;
    case roomTheme_human:   {room.roomDescr = "Human quarters.";}   break;
    case roomTheme_ritual:  {room.roomDescr = "A ritual chamber.";} break;
    case roomTheme_spider:  {room.roomDescr = "A spider lair.";}    break;
//    case roomTheme_dungeon: {room.roomDescr = "A dungeon.";}        break;
    case roomTheme_crypt:   {room.roomDescr = "A crypt.";}          break;
    case roomTheme_monster: {room.roomDescr = "A gruesome room.";}  break;
    case roomTheme_flooded: {room.roomDescr = "A flooded room.";}   break;
    case roomTheme_muddy:   {room.roomDescr = "A muddy room.";}     break;
//    case roomTheme_chasm:   {room.roomDescr = "A chasm.";}          break;
    default: {} break;
  }
}

bool RoomThemeMaker::isRoomEligibleForTheme(const Room* const room, const RoomTheme_t theme,
    const bool blockers[MAP_X_CELLS][MAP_Y_CELLS]) const {

  const int ROOM_W  = room->getX1() - room->getX0() + 1;
  const int ROOM_H  = room->getY1() - room->getY0() + 1;
  const int MIN_DIM = min(ROOM_W, ROOM_H);
  const int MAX_DIM = max(ROOM_W, ROOM_H);

  switch(theme) {
    case roomTheme_plain:     {return true;} break;
    case roomTheme_human:     {return MAX_DIM >= 5 && MIN_DIM >= 4;} break;
    case roomTheme_ritual:    {return MAX_DIM >= 5 && MIN_DIM >= 4;} break;
    case roomTheme_spider:    {return MAX_DIM >= 4 && MIN_DIM >= 3;} break;
//    case roomTheme_dungeon: {
//      return false;
//    } break;
    case roomTheme_crypt:     {return MAX_DIM >= 5 && MIN_DIM >= 4;} break;
    case roomTheme_monster:   {return MAX_DIM >= 5 && MIN_DIM >= 4;} break;
//    case roomTheme_chasm: {
//      if(MIN_DIM >= 5) {
//        for(int y = room->getY0(); y <= room->getY1(); y++) {
//          for(int x = room->getX0(); x <= room->getX1(); x++) {
//            if(blockers[x][y]) {
//              return false;
//            }
//          }
//        }
//        return true;
//      }
//      return false;
//    } break;

    default: {} break;
  }
  return true;
}

void RoomThemeMaker::makeThemeSpecificRoomModifications(Room& room) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeWalkBlockingArrayFeaturesOnly(blockers);

//  if(room.roomTheme == roomTheme_dungeon) {}

  if(room.roomTheme == roomTheme_flooded || room.roomTheme == roomTheme_muddy) {
    const Feature_t featureId = room.roomTheme == roomTheme_flooded ? feature_shallowWater : feature_shallowMud;
    for(int y = room.getY0(); y <= room.getY1(); y++) {
      for(int x = room.getX0(); x <= room.getX1(); x++) {
        if(blockers[x][y] == false) {
          eng->featureFactory->spawnFeatureAt(featureId, coord(x, y));
        }
      }
    }
  }

//  if(room.roomTheme == roomTheme_chasm) {
//    for(int y = room.getY0() + 1; y <= room.getY1() - 1; y++) {
//      for(int x = room.getX0() + 1; x <= room.getX1() - 1; x++) {
//        eng->featureFactory->spawnFeatureAt(feature_chasm, coord(x, y));
//      }
//    }
//  }

  if(room.roomTheme == roomTheme_monster) {
    bool done = false;
    int nrBloodPut = 0;
    while(done == false) {
      for(int y = room.getY0(); y <= room.getY1(); y++) {
        for(int x = room.getX0(); x <= room.getX1(); x++) {
          if(blockers[x][y] == false) {
            const int CHANCE_TO_PUT_BLOOD = 40;
            if(eng->dice.percentile() < CHANCE_TO_PUT_BLOOD) {
              eng->gore->makeGore(coord(x, y));
              eng->gore->makeBlood(coord(x, y));
              nrBloodPut++;
            }
          }
        }
      }
      done = nrBloodPut > 0;
    }
  }

  //Ritual chamber, somtimes make gore at altar (or random pos if no altar)
  if(room.roomTheme == roomTheme_ritual) {
    const int CHANCE_FOR_BLOODY_RITUAL_CHAMBER = 60;
    if(eng->dice.percentile() < CHANCE_FOR_BLOODY_RITUAL_CHAMBER) {

      coord origin(-1, -1);
      vector<coord> originCandidates;
      for(int y = room.getY0(); y <= room.getY1(); y++) {
        for(int x = room.getX0(); x <= room.getX1(); x++) {
          if(eng->map->featuresStatic[x][y]->getId() == feature_altar) {
            origin = coord(x, y);
            y = 999;
            x = 999;
          } else {
            if(blockers[x][y] == false) {
              originCandidates.push_back(coord(x, y));
            }
          }
        }
      }
      if(originCandidates.empty() == false) {
        if(origin.x == -1) {
          origin = originCandidates.at(eng->dice.getInRange(0, originCandidates.size() - 1));
        }
        for(int dy = -1; dy <= 1; dy++) {
          for(int dx = -1; dx <= 1; dx++) {
            if((dx == 0 && dy == 0) || (eng->dice.percentile() < CHANCE_FOR_BLOODY_RITUAL_CHAMBER / 2)) {
              const coord pos = origin + coord(dx, dy);
              if(blockers[pos.x][pos.y] == false) {
                eng->gore->makeGore(pos);
                eng->gore->makeBlood(pos);
              }
            }
          }
        }
      }
    }
  }
}

void RoomThemeMaker::placeThemeFeatures(Room& room) {
  tracer << "RoomThemeMaker::placeThemeFeatures()" << endl;
  vector<const FeatureDef*> featureDefsBelongingToTheme;
  featureDefsBelongingToTheme.resize(0);

  for(unsigned int i = 0; i < endOfFeatures; i++) {
    const FeatureDef* const d = eng->featureData->getFeatureDef(static_cast<Feature_t>(i));
    if(d->themedFeatureSpawnRules.isBelongingToTheme(room.roomTheme)) {
      featureDefsBelongingToTheme.push_back(d);
    }
  }

  vector<coord> nextToWalls;
  vector<coord> awayFromWalls;
  eng->mapPatterns->setPositionsInArea(room.getDims(), nextToWalls, awayFromWalls);

  vector<int> featuresSpawnCount(featureDefsBelongingToTheme.size(), 0);

  int featuresLeftToPlace = eng->dice.getInRange(3, 8);
  while(true) {
    const FeatureDef* d = NULL;
    coord pos(-1, -1);
    const int FEATURE_CANDIDATE_ELEMENT =
      attemptSetFeatureToPlace(&d, pos, nextToWalls, awayFromWalls, featureDefsBelongingToTheme);

    if(d == NULL) {
      tracer << "RoomThemeMaker: Could not find any more spots to place feature, returning" << endl;
      return;
    } else {
      tracer << "RoomThemeMaker: Placing " << d->name_a << endl;
      eng->featureFactory->spawnFeatureAt(d->id, pos);
      featuresSpawnCount.at(FEATURE_CANDIDATE_ELEMENT)++;

      // Check if more of this feature can be spawned, if not, delete it from feature candidates
      if(featuresSpawnCount.at(FEATURE_CANDIDATE_ELEMENT) >= d->themedFeatureSpawnRules.getMaxNrInRoom()) {
        featuresSpawnCount.erase(featuresSpawnCount.begin() + FEATURE_CANDIDATE_ELEMENT);
        featureDefsBelongingToTheme.erase(featureDefsBelongingToTheme.begin() + FEATURE_CANDIDATE_ELEMENT);

        // Are there any more features left to place at all?
        if(featureDefsBelongingToTheme.empty()) {return;}
      }

      eraseAdjacentCellsFromVectors(pos, nextToWalls, awayFromWalls);

      featuresLeftToPlace--;
      if(featuresLeftToPlace == 0) {
        tracer << "RoomThemeMaker: Placer enough features, returning" << endl;
        return;
      }
    }
  }
}

void RoomThemeMaker::makeRoomDarkWithChance(const Room& room) {
  const int ROOM_W = room.getX1() - room.getX0() + 1;
  const int ROOM_H = room.getY1() - room.getY0() + 1;
  if(ROOM_W >= 4 && ROOM_H >= 4) {
    int chanceToMakeDark = 0;

    switch(room.roomTheme) {
      case roomTheme_plain:     chanceToMakeDark = 5;   break;
      case roomTheme_human:     chanceToMakeDark = 10;  break;
      case roomTheme_ritual:    chanceToMakeDark = 15;  break;
      case roomTheme_spider:    chanceToMakeDark = 33;  break;
//      case roomTheme_dungeon:   chanceToMakeDark = 50;  break;
      case roomTheme_crypt:     chanceToMakeDark = 75;  break;
      case roomTheme_monster:   chanceToMakeDark = 75;  break;
      case roomTheme_flooded:   chanceToMakeDark = 50;  break;
      case roomTheme_muddy:     chanceToMakeDark = 50;  break;
//      case roomTheme_chasm:     chanceToMakeDark = 50;  break;
      default: break;
    }

    chanceToMakeDark += eng->map->getDungeonLevel() - 1;

    if(eng->dice.getInRange(1, 100) < chanceToMakeDark) {
      for(int y = room.getY0(); y <= room.getY1(); y++) {
        for(int x = room.getX0(); x <= room.getX1(); x++) {
          eng->map->darkness[x][y] = true;
        }
      }
    }
  }
}

int RoomThemeMaker::attemptSetFeatureToPlace(const FeatureDef** def, coord& pos, vector<coord>& nextToWalls,
    vector<coord>& awayFromWalls, vector<const FeatureDef*> featureDefsBelongingToTheme) {
  tracer << "RoomThemeMaker::attemptSetFeatureToPlace()" << endl;

  if(featureDefsBelongingToTheme.empty()) {
    return -1;
  }

  const bool IS_NEXT_TO_WALL_AVAIL = nextToWalls.size() != 0;
  const bool IS_AWAY_FROM_WALLS_AVAIL = awayFromWalls.size() != 0;

  if(IS_NEXT_TO_WALL_AVAIL == false && IS_AWAY_FROM_WALLS_AVAIL == false) {
    tracer << "RoomThemeMaker: Neither cells next to walls or away from walls found, returning" << endl;
    def = NULL;
    return -1;
  }

  const int NR_ATTEMPTS_TO_FIND_POS = 100;
  for(int i = 0; i < NR_ATTEMPTS_TO_FIND_POS; i++) {
    const int FEATURE_DEF_ELEMENT = eng->dice.getInRange(0, featureDefsBelongingToTheme.size() - 1);
    const FeatureDef* const dTemp = featureDefsBelongingToTheme.at(FEATURE_DEF_ELEMENT);

    if(dTemp->themedFeatureSpawnRules.getPlacementRule() == placementRule_nextToWalls) {
      if(IS_NEXT_TO_WALL_AVAIL) {
        const int POS_ELEMENT = eng->dice.getInRange(0, nextToWalls.size() - 1);
        pos = nextToWalls.at(POS_ELEMENT);
        *def = dTemp;
        return FEATURE_DEF_ELEMENT;
      }
    }

    if(dTemp->themedFeatureSpawnRules.getPlacementRule() == placementRule_awayFromWalls) {
      if(IS_AWAY_FROM_WALLS_AVAIL) {
        const int POS_ELEMENT = eng->dice.getInRange(0, awayFromWalls.size() - 1);
        pos = awayFromWalls.at(POS_ELEMENT);
        *def = dTemp;
        return FEATURE_DEF_ELEMENT;
      }
    }

    if(dTemp->themedFeatureSpawnRules.getPlacementRule() == placementRule_nextToWallsOrAwayFromWalls) {
      if(eng->dice.coinToss()) {
        if(IS_NEXT_TO_WALL_AVAIL) {
          const int POS_ELEMENT = eng->dice.getInRange(0, nextToWalls.size() - 1);
          pos = nextToWalls.at(POS_ELEMENT);
          *def = dTemp;
          return FEATURE_DEF_ELEMENT;
        }
      } else {
        if(IS_AWAY_FROM_WALLS_AVAIL) {
          const int POS_ELEMENT = eng->dice.getInRange(0, awayFromWalls.size() - 1);
          pos = awayFromWalls.at(POS_ELEMENT);
          *def = dTemp;
          return FEATURE_DEF_ELEMENT;
        }
      }
    }
  }
  return -1;
}

void RoomThemeMaker::eraseAdjacentCellsFromVectors(const coord& pos,  vector<coord>& nextToWalls, vector<coord>& awayFromWalls) {
  tracer << "RoomThemeMaker::eraseAdjacentCellsFromVectors()..." << endl;
  for(unsigned int i = 0; i < nextToWalls.size(); i++) {
    if(eng->mapTests->isCellsNeighbours(pos, nextToWalls.at(i), true)) {
      nextToWalls.erase(nextToWalls.begin() + i);
      i--;
    }
  }
  for(unsigned int i = 0; i < awayFromWalls.size(); i++) {
    if(eng->mapTests->isCellsNeighbours(pos, awayFromWalls.at(i), true)) {
      awayFromWalls.erase(awayFromWalls.begin() + i);
      i--;
    }
  }
  tracer << "RoomThemeMaker::eraseAdjacentCellsFromVectors() [DONE]" << endl;
}

void RoomThemeMaker::assignRoomThemes() {
  tracer << "RoomThemeMaker::assignRoomThemes()..." << endl;

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      themeMap[x][y] = roomTheme_plain;
    }
  }

  const int MIN_DIM = 3;
  const int MAX_DIM = 12;
  const int NR_NON_PLAIN_THEMED = eng->dice.getInRange(1, 4);

  const int NR_ROOMS = roomList.size();

  vector<bool> isAssigned(roomList.size(), false);

  tracer << "RoomThemeMaker: Assigning plain theme to rooms with wrong dimensions" << endl;
  for(int i = 0; i < NR_ROOMS; i++) {
    Room* const r = roomList.at(i);

    // Check dimensions, assign plain if too small or too big
    if(isAssigned.at(i) == false) {
      const int W = r->getX1() - r->getX0() + 1;
      const int H = r->getY1() - r->getY0() + 1;
      if(W < MIN_DIM || W > MAX_DIM || H < MIN_DIM || H > MAX_DIM) {
        r->roomTheme = roomTheme_plain;
        isAssigned.at(i) = true;
        continue;
      }
    }
  }

  tracer << "RoomThemeMaker: Attempting to assign non-plain themes to some rooms" << endl;
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeWalkBlockingArrayFeaturesOnly(blockers);
  const int NR_TRIES_TO_ASSIGN = 100;
  for(int i = 0; i < NR_NON_PLAIN_THEMED; i++) {
    for(int ii = 0; ii < NR_TRIES_TO_ASSIGN; ii++) {
      const int ELEMENT = eng->dice.getInRange(0, NR_ROOMS - 1);
      if(isAssigned.at(ELEMENT) == false) {
        const RoomTheme_t theme = static_cast<RoomTheme_t>(eng->dice.getInRange(1, endOfRoomThemes - 1));
        Room* const room = roomList.at(ELEMENT);

        if(isRoomEligibleForTheme(room, theme, blockers)) {
          room->roomTheme = theme;
          tracer << "RoomThemeMaker: Assigned non-plain theme (" << theme << ") to room" << endl;
          isAssigned.at(ELEMENT) = true;
          for(int y = room->getY0(); y < room->getY1(); y++) {
            for(int x = room->getX0(); x < room->getX1(); x++) {
              themeMap[x][y] = theme;
            }
          }
          break;
        }
      }
    }
  }

  tracer << "RoomThemeMaker: Assigning plain theme to remaining rooms" << endl;
  for(int i = 0; i < NR_ROOMS; i++) {
    if(isAssigned.at(i) == false) {
      roomList.at(i)->roomTheme = roomTheme_plain;
      isAssigned.at(i) = true;
    }
  }

  tracer << "RoomThemeMaker::assignRoomThemes() [DONE]" << endl;
}

