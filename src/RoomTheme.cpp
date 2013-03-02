#include "RoomTheme.h"

#include "Engine.h"
#include "MapBuildBSP.h"
#include "FeatureData.h"
#include "FeatureFactory.h"
#include "Map.h"
#include "PopulateItems.h"
#include "PopulateMonsters.h"
#include "PopulateTraps.h"

// Criteria:
// * If a room contains other rooms, the outer room has no theme
// * Rooms inside other rooms may have any theme
// * If a room is too small or too big, it is always plain (if not none)

void RoomThemeMaker::run(const vector<Room*>& rooms) {
  tracer << "RoomThemeMaker::run()..." << endl;
  assignRoomThemes(rooms);

  for(unsigned int i = 0; i < rooms.size(); i++) {
    applyThemeToRoom(*(rooms.at(i)));
    makeRoomDarkWithChance(*(rooms.at(i)));
  }

  tracer << "RoomThemeMaker: Calling PopulateMonsters::populateRoomAndCorridorLevel()" << endl;
  eng->populateMonsters->populateRoomAndCorridorLevel(themeMap, rooms);

  tracer << "RoomThemeMaker::run() [DONE]" << endl;
}

void RoomThemeMaker::applyThemeToRoom(Room& room) {
//  tracer << "RoomThemeMaker::applyThemeToRoom()..." << endl; //Spammy
  if(room.roomTheme != roomTheme_plain) {
    placeThemeFeatures(room);
  }
//  tracer << "RoomThemeMaker::applyThemeToRoom() [DONE]" << endl;  //Spammy
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
        if(featureDefsBelongingToTheme.empty()) {
          return;
        }
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
  int chanceToMakeDark = 0;

  switch(room.roomTheme) {
  case roomTheme_plain:
    chanceToMakeDark = 5;
    break;
  case roomTheme_human:
    chanceToMakeDark = 10;
    break;
  case roomTheme_ritual:
    chanceToMakeDark = 15;
    break;
  case roomTheme_spider:
    chanceToMakeDark = 25;
    break;
  case roomTheme_jail:
    chanceToMakeDark = 20;
    break;
  case roomTheme_tomb:
    chanceToMakeDark = 75;
    break;
  case roomTheme_monster:
    chanceToMakeDark = 75;
    break;
  default:
    break;
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

int RoomThemeMaker::attemptSetFeatureToPlace(const FeatureDef** def, coord& pos, vector<coord>& nextToWalls,
    vector<coord>& awayFromWalls, vector<const FeatureDef*> featureDefsBelongingToTheme) {
  tracer << "RoomThemeMaker::attemptSetFeatureToPlace()" << endl;

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

void RoomThemeMaker::assignRoomThemes(const vector<Room*>& rooms) {
  tracer << "RoomThemeMaker::assignRoomThemes()..." << endl;

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      themeMap[x][y] = roomTheme_plain;
    }
  }

  const int MIN_DIM = 3;
  const int MAX_DIM = 12;
  const int NR_NON_PLAIN_THEMED = eng->dice.getInRange(2, 3);

  const int NR_ROOMS = rooms.size();

  vector<bool> isAssigned(rooms.size(), false);

  tracer << "RoomThemeMaker: Assigning plain theme to rooms with wrong dimensions" << endl;
  for(int i = 0; i < NR_ROOMS; i++) {
    Room* const r = rooms.at(i);

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
  const int NR_TRIES_TO_ASSIGN = 100;
  for(int i = 0; i < NR_NON_PLAIN_THEMED; i++) {
    for(int ii = 0; ii < NR_TRIES_TO_ASSIGN; ii++) {
      const int ELEMENT = eng->dice.getInRange(0, NR_ROOMS - 1);
      if(isAssigned.at(ELEMENT) == false) {
        const RoomTheme_t theme = static_cast<RoomTheme_t>(eng->dice.getInRange(1, endOfRoomThemes - 1));
        Room* const room = rooms.at(ELEMENT);
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

  tracer << "RoomThemeMaker: Assigning plain theme to remaining rooms" << endl;
  for(int i = 0; i < NR_ROOMS; i++) {
    if(isAssigned.at(i) == false) {
      rooms.at(i)->roomTheme = roomTheme_plain;
      isAssigned.at(i) = true;
    }
  }

  tracer << "RoomThemeMaker::assignRoomThemes() [DONE]" << endl;
}

