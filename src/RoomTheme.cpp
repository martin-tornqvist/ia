#include "RoomTheme.h"

#include "Engine.h"
#include "MapGen.h"
#include "FeatureData.h"
#include "FeatureFactory.h"
#include "Map.h"
#include "PopulateItems.h"
#include "PopulateMonsters.h"
#include "PopulateTraps.h"
#include "Blood.h"
#include "Gods.h"
#include "MapParsing.h"

void RoomThemeMaker::run() {
  trace << "RoomThemeMaker::run()..." << endl;

  eng.gods->setNoGod();

  assignRoomThemes();

  vector<Room*>& rooms = eng.map->rooms;

  for(unsigned int i = 0; i < rooms.size(); i++) {
    applyThemeToRoom(*(rooms.at(i)));
    makeRoomDarkWithChance(*(rooms.at(i)));
  }

  trace << "RoomThemeMaker: Calling PopulateMonsters::populateRoomAndCorridorLevel()" << endl;
  trace << "DLVL: " << eng.map->getDlvl() << endl;
  eng.populateMonsters->populateRoomAndCorridorLevel(themeMap, rooms);

  trace << "RoomThemeMaker: Calling PopulateTraps::populateRoomAndCorridorLevel()" << endl;
  eng.populateTraps->populateRoomAndCorridorLevel(themeMap, rooms);

  trace << "RoomThemeMaker::run() [DONE]" << endl;
}

void RoomThemeMaker::applyThemeToRoom(Room& room) {
  placeThemeFeatures(room);

  makeThemeSpecificRoomModifications(room);

  switch(room.roomTheme) {
    case roomTheme_plain:   {room.roomDescr = "";}  break;
    case roomTheme_human:   {room.roomDescr = "";}  break;
    case roomTheme_ritual:  {room.roomDescr = "";}  break;
    case roomTheme_spider:  {room.roomDescr = "";}  break;
    case roomTheme_crypt:   {room.roomDescr = "";}  break;
    case roomTheme_monster: {room.roomDescr = "";}  break;
    case roomTheme_flooded: {room.roomDescr = "";}  break;
    case roomTheme_muddy:   {room.roomDescr = "";}  break;
    case endOfRoomThemes: {} break;
  }
}

int RoomThemeMaker::getRandomNrFeaturesForTheme(
  const RoomTheme_t theme) const {

  switch(theme) {
    case roomTheme_plain:     return eng.dice.oneIn(14) ?
                                       2 : eng.dice.oneIn(5) ? 1 : 0;
    case roomTheme_human:     return eng.dice.oneIn(7) ? 2 : 1;
    case roomTheme_ritual:    return eng.dice.range(1, 3);
    case roomTheme_spider:    return eng.dice.range(0, 2);
    case roomTheme_crypt:     return eng.dice.oneIn(7) ? 2 : 1;
    case roomTheme_monster:   return eng.dice.range(0, 6);
    case roomTheme_flooded:   return 0;
    case roomTheme_muddy:     return 0;
    case endOfRoomThemes: {} break;
  }
  return -1;
}

bool RoomThemeMaker::isThemeExistInMap(const RoomTheme_t theme) const {
  vector<Room*>& rooms = eng.map->rooms;
  for(unsigned int i = 0; i < rooms.size(); i++) {
    if(rooms.at(i)->roomTheme == theme) {
      return true;
    }
  }
  return false;
}

bool RoomThemeMaker::isThemeAllowed(
  const Room* const room, const RoomTheme_t theme,
  const bool blockers[MAP_W][MAP_H]) const {

  (void)blockers;

  const int ROOM_W  = room->getX1() - room->getX0() + 1;
  const int ROOM_H  = room->getY1() - room->getY0() + 1;
  const int MIN_DIM = min(ROOM_W, ROOM_H);
  const int MAX_DIM = max(ROOM_W, ROOM_H);

  switch(theme) {
    case roomTheme_plain: {return true;} break;

    case roomTheme_human: {
      return MAX_DIM >= 5 && MIN_DIM >= 4 &&
             isThemeExistInMap(roomTheme_human) == false;
    } break;

    case roomTheme_ritual: {
      return MAX_DIM >= 4 && MIN_DIM >= 3 &&
             isThemeExistInMap(roomTheme_ritual) == false;
    } break;

    case roomTheme_spider: {
      return MAX_DIM >= 4 && MIN_DIM >= 3;
    } break;

    case roomTheme_crypt: {
      return MAX_DIM >= 5 && MIN_DIM >= 4;
    } break;

    case roomTheme_monster: {
      return MAX_DIM >= 5 && MIN_DIM >= 4;
    } break;

//    case roomTheme_dungeon: {
//      return false;
//    } break;

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
  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, false, eng),
                   blockers);

//  if(room.roomTheme == roomTheme_dungeon) {}
  switch(room.roomTheme) {
    case roomTheme_flooded:
    case roomTheme_muddy: {
      const Feature_t featureId =
        room.roomTheme == roomTheme_flooded ? feature_shallowWater :
        feature_shallowMud;
      for(int y = room.getY0(); y <= room.getY1(); y++) {
        for(int x = room.getX0(); x <= room.getX1(); x++) {
          if(blockers[x][y] == false) {
            eng.featureFactory->spawnFeatureAt(featureId, Pos(x, y));
          }
        }
      }
    } break;

//  if(room.roomTheme == roomTheme_chasm) {
//    for(int y = room.getY0() + 1; y <= room.getY1() - 1; y++) {
//      for(int x = room.getX0() + 1; x <= room.getX1() - 1; x++) {
//        eng.featureFactory->spawnFeatureAt(feature_chasm, Pos(x, y));
//      }
//    }
//  }

    case roomTheme_monster: {
      int nrBloodPut = 0;
      const int NR_TRIES = 1000; //TODO Hacky, needs improving
      for(int i = 0; i < NR_TRIES; i++) {
        for(int y = room.getY0(); y <= room.getY1(); y++) {
          for(int x = room.getX0(); x <= room.getX1(); x++) {
            if(blockers[x][y] == false) {
              const int CHANCE_TO_PUT_BLOOD = 40;
              if(eng.dice.percentile() < CHANCE_TO_PUT_BLOOD) {
                eng.gore->makeGore(Pos(x, y));
                eng.gore->makeBlood(Pos(x, y));
                nrBloodPut++;
              }
            }
          }
        }
        if(nrBloodPut > 0) {
          break;
        }
      }
    } break;

    //Ritual chamber, set a random god for this level, sometimes make gore
    //at altar (or at random pos if no altar)
    case roomTheme_ritual: {

      eng.gods->setRandomGod();

      const int CHANCE_FOR_BLOODY_CHAMBER = 60;
      if(eng.dice.percentile() < CHANCE_FOR_BLOODY_CHAMBER) {

        Pos origin(-1, -1);
        vector<Pos> originCandidates;
        for(int y = room.getY0(); y <= room.getY1(); y++) {
          for(int x = room.getX0(); x <= room.getX1(); x++) {
            if(eng.map->cells[x][y].featureStatic->getId() == feature_altar) {
              origin = Pos(x, y);
              y = 999;
              x = 999;
            } else {
              if(blockers[x][y] == false) {
                originCandidates.push_back(Pos(x, y));
              }
            }
          }
        }
        if(originCandidates.empty() == false) {
          if(origin.x == -1) {
            const int ELEMENT =
              eng.dice.range(0, originCandidates.size() - 1);
            origin = originCandidates.at(ELEMENT);
          }
          for(int dy = -1; dy <= 1; dy++) {
            for(int dx = -1; dx <= 1; dx++) {
              if(
                (dx == 0 && dy == 0) ||
                (eng.dice.percentile() < CHANCE_FOR_BLOODY_CHAMBER / 2)) {
                const Pos pos = origin + Pos(dx, dy);
                if(blockers[pos.x][pos.y] == false) {
                  eng.gore->makeGore(pos);
                  eng.gore->makeBlood(pos);
                }
              }
            }
          }
        }
      }
    } break;

    case roomTheme_plain: {} break;
    case roomTheme_human: {} break;
    case roomTheme_spider: {} break;
    case roomTheme_crypt: {} break;
    case endOfRoomThemes: {} break;
  }
}

int RoomThemeMaker::placeThemeFeatures(Room& room) {
  trace << "RoomThemeMaker::placeThemeFeatures()" << endl;
  vector<const FeatureData*> featureDataBelongingToTheme;
  featureDataBelongingToTheme.resize(0);

  for(unsigned int i = 0; i < endOfFeatures; i++) {
    const FeatureData* const d =
      eng.featureDataHandler->getData((Feature_t)(i));
    if(d->themedFeatureSpawnRules.isBelongingToTheme(room.roomTheme)) {
      featureDataBelongingToTheme.push_back(d);
    }
  }

  vector<Pos> nextToWalls;
  vector<Pos> awayFromWalls;
  eng.mapPatterns->setPositionsInArea(
    room.getDims(), nextToWalls, awayFromWalls);

  vector<int> featuresSpawnCount(featureDataBelongingToTheme.size(), 0);

  int nrFeaturesLeftToPlace = getRandomNrFeaturesForTheme(room.roomTheme);

  int nrFeaturesPlaced = 0;

  while(true) {
    if(nrFeaturesLeftToPlace == 0) {
      trace << "RoomThemeMaker: Placed enough features, returning" << endl;
      return nrFeaturesPlaced;
    }

    const FeatureData* d = NULL;
    Pos pos(-1, -1);
    const int FEATURE_CANDIDATE_ELEMENT =
      trySetFeatureToPlace(&d, pos, nextToWalls, awayFromWalls,
                           featureDataBelongingToTheme);

    if(d == NULL) {
      trace << "RoomThemeMaker: Could not find any more spots ";
      trace << "to place feature, returning" << endl;
      return nrFeaturesPlaced;
    } else {
      trace << "RoomThemeMaker: Placing " << d->name_a << endl;
      eng.featureFactory->spawnFeatureAt(d->id, pos);
      featuresSpawnCount.at(FEATURE_CANDIDATE_ELEMENT)++;

      nrFeaturesLeftToPlace--;
      nrFeaturesPlaced++;

      //Check if more of this feature can be spawned,
      //if not, delete it from feature candidates
      if(
        featuresSpawnCount.at(FEATURE_CANDIDATE_ELEMENT) >=
        d->themedFeatureSpawnRules.getMaxNrInRoom()) {
        featuresSpawnCount.erase(
          featuresSpawnCount.begin() + FEATURE_CANDIDATE_ELEMENT);
        featureDataBelongingToTheme.erase(
          featureDataBelongingToTheme.begin() + FEATURE_CANDIDATE_ELEMENT);

        //Are there any more features left to place at all?
        if(featureDataBelongingToTheme.empty()) {return nrFeaturesPlaced;}
      }

      eraseAdjacentCellsFromVectors(pos, nextToWalls, awayFromWalls);
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
      case roomTheme_crypt:     chanceToMakeDark = 75;  break;
      case roomTheme_monster:   chanceToMakeDark = 75;  break;
      case roomTheme_flooded:   chanceToMakeDark = 50;  break;
      case roomTheme_muddy:     chanceToMakeDark = 50;  break;
//      case roomTheme_dungeon:   chanceToMakeDark = 50;  break;
//      case roomTheme_chasm:     chanceToMakeDark = 50;  break;
      default: break;
    }

    chanceToMakeDark += eng.map->getDlvl() - 1;

    if(eng.dice.range(1, 100) < chanceToMakeDark) {
      for(int y = room.getY0(); y <= room.getY1(); y++) {
        for(int x = room.getX0(); x <= room.getX1(); x++) {
          eng.map->cells[x][y].isDark = true;
        }
      }
    }
  }
}

int RoomThemeMaker::trySetFeatureToPlace(const FeatureData** def, Pos& pos,
    vector<Pos>& nextToWalls,
    vector<Pos>& awayFromWalls,
    vector<const FeatureData*> featureDataBelongingToTheme) {
  trace << "RoomThemeMaker::trySetFeatureToPlace()" << endl;

  if(featureDataBelongingToTheme.empty()) {
    return -1;
  }

  const bool IS_NEXT_TO_WALL_AVAIL = nextToWalls.size() != 0;
  const bool IS_AWAY_FROM_WALLS_AVAIL = awayFromWalls.size() != 0;

  if(IS_NEXT_TO_WALL_AVAIL == false && IS_AWAY_FROM_WALLS_AVAIL == false) {
    trace << "RoomThemeMaker: Neither cells next to walls or away from ";
    trace << "walls found, returning" << endl;
    def = NULL;
    return -1;
  }

  const int NR_ATTEMPTS_TO_FIND_POS = 100;
  for(int i = 0; i < NR_ATTEMPTS_TO_FIND_POS; i++) {
    const int FEATURE_DEF_ELEMENT =
      eng.dice.range(0, featureDataBelongingToTheme.size() - 1);
    const FeatureData* const dTemp =
      featureDataBelongingToTheme.at(FEATURE_DEF_ELEMENT);

    if(
      dTemp->themedFeatureSpawnRules.getPlacementRule() ==
      placementRule_nextToWalls) {
      if(IS_NEXT_TO_WALL_AVAIL) {
        const int POS_ELEMENT =
          eng.dice.range(0, nextToWalls.size() - 1);
        pos = nextToWalls.at(POS_ELEMENT);
        *def = dTemp;
        return FEATURE_DEF_ELEMENT;
      }
    }

    if(
      dTemp->themedFeatureSpawnRules.getPlacementRule() ==
      placementRule_awayFromWalls) {
      if(IS_AWAY_FROM_WALLS_AVAIL) {
        const int POS_ELEMENT =
          eng.dice.range(0, awayFromWalls.size() - 1);
        pos = awayFromWalls.at(POS_ELEMENT);
        *def = dTemp;
        return FEATURE_DEF_ELEMENT;
      }
    }

    if(
      dTemp->themedFeatureSpawnRules.getPlacementRule() ==
      placementRule_nextToWallsOrAwayFromWalls) {
      if(eng.dice.coinToss()) {
        if(IS_NEXT_TO_WALL_AVAIL) {
          const int POS_ELEMENT =
            eng.dice.range(0, nextToWalls.size() - 1);
          pos = nextToWalls.at(POS_ELEMENT);
          *def = dTemp;
          return FEATURE_DEF_ELEMENT;
        }
      } else {
        if(IS_AWAY_FROM_WALLS_AVAIL) {
          const int POS_ELEMENT =
            eng.dice.range(0, awayFromWalls.size() - 1);
          pos = awayFromWalls.at(POS_ELEMENT);
          *def = dTemp;
          return FEATURE_DEF_ELEMENT;
        }
      }
    }
  }
  return -1;
}

void RoomThemeMaker::eraseAdjacentCellsFromVectors(
  const Pos& pos,  vector<Pos>& nextToWalls, vector<Pos>& awayFromWalls) {
  trace << "RoomThemeMaker::eraseAdjacentCellsFromVectors()..." << endl;
  for(unsigned int i = 0; i < nextToWalls.size(); i++) {
    if(eng.basicUtils->isPosAdj(pos, nextToWalls.at(i), true)) {
      nextToWalls.erase(nextToWalls.begin() + i);
      i--;
    }
  }
  for(unsigned int i = 0; i < awayFromWalls.size(); i++) {
    if(eng.basicUtils->isPosAdj(pos, awayFromWalls.at(i), true)) {
      awayFromWalls.erase(awayFromWalls.begin() + i);
      i--;
    }
  }
  trace << "RoomThemeMaker::eraseAdjacentCellsFromVectors() [DONE]" << endl;
}

void RoomThemeMaker::assignRoomThemes() {
  trace << "RoomThemeMaker::assignRoomThemes()..." << endl;

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      themeMap[x][y] = roomTheme_plain;
    }
  }

  const int MIN_DIM = 3;
  const int MAX_DIM = 12;
  const int NR_NON_PLAIN_THEMED = eng.dice.range(1, 3);

  vector<Room*>& rooms = eng.map->rooms;
  const int NR_ROOMS = rooms.size();

  vector<bool> isAssigned(NR_ROOMS, false);

  trace << "RoomThemeMaker: Assigning plain theme to rooms with ";
  trace << "wrong dimensions" << endl;
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

  trace << "RoomThemeMaker: Trying to set non-plain themes ";
  trace << "for some rooms" << endl;
  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, false, eng),
                   blockers);
  const int NR_TRIES_TO_ASSIGN = 100;
  for(int i = 0; i < NR_NON_PLAIN_THEMED; i++) {
    for(int ii = 0; ii < NR_TRIES_TO_ASSIGN; ii++) {
      const int ELEMENT = eng.dice.range(0, NR_ROOMS - 1);
      if(isAssigned.at(ELEMENT) == false) {
        const RoomTheme_t theme =
          (RoomTheme_t)(eng.dice.range(1, endOfRoomThemes - 1));
        Room* const room = rooms.at(ELEMENT);

        if(isThemeAllowed(room, theme, blockers)) {
          room->roomTheme = theme;
          trace << "RoomThemeMaker: Assigned non-plain theme";
          trace << "(" << theme << ") to room" << endl;
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

  trace << "RoomThemeMaker: Assigning plain theme to remaining rooms" << endl;
  for(int i = 0; i < NR_ROOMS; i++) {
    if(isAssigned.at(i) == false) {
      rooms.at(i)->roomTheme = roomTheme_plain;
      isAssigned.at(i) = true;
    }
  }

  trace << "RoomThemeMaker::assignRoomThemes() [DONE]" << endl;
}

