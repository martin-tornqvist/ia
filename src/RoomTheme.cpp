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
#include "Utils.h"

void RoomThemeMaker::run() {
  trace << "RoomThemeMaker::run()..." << endl;

  eng.gods->setNoGod();

  assignRoomThemes();

  vector<Room*>& rooms = eng.map->rooms;

  for(Room * const room : rooms) {
    applyThemeToRoom(*room);
    makeRoomDarkWithChance(*room);
  }

  trace << "RoomThemeMaker::run() [DONE]" << endl;
}

void RoomThemeMaker::applyThemeToRoom(Room& room) {
  placeThemeFeatures(room);

  makeThemeSpecificRoomModifications(room);

  switch(room.roomTheme) {
    case RoomThemeId::plain:   {room.roomDescr = "";} break;
    case RoomThemeId::human:   {room.roomDescr = "";} break;
    case RoomThemeId::ritual:  {room.roomDescr = "";} break;
    case RoomThemeId::spider:  {room.roomDescr = "";} break;
    case RoomThemeId::crypt:   {room.roomDescr = "";} break;
    case RoomThemeId::monster: {room.roomDescr = "";} break;
    case RoomThemeId::flooded: {room.roomDescr = "";} break;
    case RoomThemeId::muddy:   {room.roomDescr = "";} break;
    case RoomThemeId::endOfRoomThemes: {} break;
  }
}

int RoomThemeMaker::getRandomNrFeaturesForTheme(const RoomThemeId theme) const {
  switch(theme) {
    case RoomThemeId::plain:
      return Rnd::oneIn(14) ? 2 : Rnd::oneIn(5) ? 1 : 0;
    case RoomThemeId::human:
      return Rnd::range(3, 6);
    case RoomThemeId::ritual:
      return Rnd::range(1, 5);
    case RoomThemeId::spider:
      return Rnd::range(0, 3);
    case RoomThemeId::crypt:
      return Rnd::range(3, 6);
    case RoomThemeId::monster:
      return Rnd::range(0, 6);
    case RoomThemeId::flooded:
      return 0;
    case RoomThemeId::muddy:
      return 0;
    case RoomThemeId::endOfRoomThemes: {} break;
  }
  return -1;
}

int RoomThemeMaker::nrThemeInMap(const RoomThemeId theme) const {
  int nr = 0;
  for(Room * r : eng.map->rooms) {if(r->roomTheme == theme) nr++;}
  return nr;
}

bool RoomThemeMaker::isThemeAllowed(
  const Room* const room, const RoomThemeId theme,
  const bool blockers[MAP_W][MAP_H]) const {

  (void)blockers;

  const int ROOM_W  = room->getX1() - room->getX0() + 1;
  const int ROOM_H  = room->getY1() - room->getY0() + 1;
  const int MIN_DIM = min(ROOM_W, ROOM_H);
  const int MAX_DIM = max(ROOM_W, ROOM_H);

  switch(theme) {
    case RoomThemeId::plain: {return true;} break;

    case RoomThemeId::human: {
      return MIN_DIM >= 4 && MAX_DIM <= 8 &&
             nrThemeInMap(RoomThemeId::human) < 3;
    } break;

    case RoomThemeId::ritual: {
      return MIN_DIM >= 4 && MAX_DIM <= 8 &&
             nrThemeInMap(RoomThemeId::ritual) == 0;
    } break;

    case RoomThemeId::spider: {
      return MIN_DIM >= 3 && MAX_DIM <= 8;
    } break;

    case RoomThemeId::crypt: {
      return MIN_DIM >= 3 && MAX_DIM <= 12 &&
             nrThemeInMap(RoomThemeId::crypt) < 3;
    } break;

    case RoomThemeId::monster: {
      return MIN_DIM >= 4 && MAX_DIM <= 8 &&
             nrThemeInMap(RoomThemeId::monster) < 3;
    } break;

    default: {} break;
  }
  return true;
}

void RoomThemeMaker::makeThemeSpecificRoomModifications(Room& room) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false, eng), blockers);

  switch(room.roomTheme) {
    case RoomThemeId::flooded:
    case RoomThemeId::muddy: {
      const FeatureId featureId =
        room.roomTheme == RoomThemeId::flooded ? feature_shallowWater :
        feature_shallowMud;
      for(int y = room.getY0(); y <= room.getY1(); y++) {
        for(int x = room.getX0(); x <= room.getX1(); x++) {
          if(blockers[x][y] == false) {
            eng.featureFactory->spawnFeatureAt(featureId, Pos(x, y));
          }
        }
      }
    } break;

    case RoomThemeId::monster: {
      int nrBloodPut = 0;
      const int NR_TRIES = 1000; //TODO Hacky, needs improving
      for(int i = 0; i < NR_TRIES; i++) {
        for(int y = room.getY0(); y <= room.getY1(); y++) {
          for(int x = room.getX0(); x <= room.getX1(); x++) {
            if(blockers[x][y] == false) {
              const int CHANCE_TO_PUT_BLOOD = 40;
              if(Rnd::percentile() < CHANCE_TO_PUT_BLOOD) {
                eng.gore->makeGore(Pos(x, y));
                eng.gore->makeBlood(Pos(x, y));
                nrBloodPut++;
              }
            }
          }
        }
        if(nrBloodPut > 0) {break;}
      }
    } break;

    //Ritual chamber, set a random god for this level, sometimes make gore
    //at altar (or at random pos if no altar)
    case RoomThemeId::ritual: {

      eng.gods->setRandomGod();

      const int CHANCE_FOR_BLOODY_CHAMBER = 60;
      if(Rnd::percentile() < CHANCE_FOR_BLOODY_CHAMBER) {

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
              Rnd::range(0, originCandidates.size() - 1);
            origin = originCandidates.at(ELEMENT);
          }
          for(int dy = -1; dy <= 1; dy++) {
            for(int dx = -1; dx <= 1; dx++) {
              if(
                (dx == 0 && dy == 0) ||
                (Rnd::percentile() < CHANCE_FOR_BLOODY_CHAMBER / 2)) {
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

    case RoomThemeId::plain:            {} break;
    case RoomThemeId::human:            {} break;
    case RoomThemeId::spider:           {} break;
    case RoomThemeId::crypt:            {} break;
    case RoomThemeId::endOfRoomThemes:  {} break;
  }
}

int RoomThemeMaker::placeThemeFeatures(Room& room) {
  trace << "RoomThemeMaker::placeThemeFeatures()" << endl;
  vector<const FeatureData*> featureDataBelongingToTheme;
  featureDataBelongingToTheme.resize(0);

  for(unsigned int i = 0; i < endOfFeatureId; i++) {
    const FeatureData* const d =
      eng.featureDataHandler->getData((FeatureId)(i));
    if(d->featureThemeSpawnRules.isBelongingToTheme(room.roomTheme)) {
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
        d->featureThemeSpawnRules.getMaxNrInRoom()) {
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
      case RoomThemeId::plain:     chanceToMakeDark = 5;   break;
      case RoomThemeId::human:     chanceToMakeDark = 10;  break;
      case RoomThemeId::ritual:    chanceToMakeDark = 15;  break;
      case RoomThemeId::spider:    chanceToMakeDark = 33;  break;
      case RoomThemeId::crypt:     chanceToMakeDark = 75;  break;
      case RoomThemeId::monster:   chanceToMakeDark = 75;  break;
      case RoomThemeId::flooded:   chanceToMakeDark = 50;  break;
      case RoomThemeId::muddy:     chanceToMakeDark = 50;  break;
      default: break;
    }

    chanceToMakeDark += eng.map->getDlvl() - 1;

    if(Rnd::range(1, 100) < chanceToMakeDark) {
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
      Rnd::range(0, featureDataBelongingToTheme.size() - 1);
    const FeatureData* const dTemp =
      featureDataBelongingToTheme.at(FEATURE_DEF_ELEMENT);

    if(
      dTemp->featureThemeSpawnRules.getPlacementRule() ==
      placementRule_nextToWalls) {
      if(IS_NEXT_TO_WALL_AVAIL) {
        const int POS_ELEMENT =
          Rnd::range(0, nextToWalls.size() - 1);
        pos = nextToWalls.at(POS_ELEMENT);
        *def = dTemp;
        return FEATURE_DEF_ELEMENT;
      }
    }

    if(
      dTemp->featureThemeSpawnRules.getPlacementRule() ==
      placementRule_awayFromWalls) {
      if(IS_AWAY_FROM_WALLS_AVAIL) {
        const int POS_ELEMENT =
          Rnd::range(0, awayFromWalls.size() - 1);
        pos = awayFromWalls.at(POS_ELEMENT);
        *def = dTemp;
        return FEATURE_DEF_ELEMENT;
      }
    }

    if(
      dTemp->featureThemeSpawnRules.getPlacementRule() ==
      placementRule_nextToWallsOrAwayFromWalls) {
      if(Rnd::coinToss()) {
        if(IS_NEXT_TO_WALL_AVAIL) {
          const int POS_ELEMENT =
            Rnd::range(0, nextToWalls.size() - 1);
          pos = nextToWalls.at(POS_ELEMENT);
          *def = dTemp;
          return FEATURE_DEF_ELEMENT;
        }
      } else {
        if(IS_AWAY_FROM_WALLS_AVAIL) {
          const int POS_ELEMENT =
            Rnd::range(0, awayFromWalls.size() - 1);
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
    if(Utils::isPosAdj(pos, nextToWalls.at(i), true)) {
      nextToWalls.erase(nextToWalls.begin() + i);
      i--;
    }
  }
  for(unsigned int i = 0; i < awayFromWalls.size(); i++) {
    if(Utils::isPosAdj(pos, awayFromWalls.at(i), true)) {
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
      themeMap[x][y] = RoomThemeId::plain;
    }
  }

  const int MIN_DIM = 3;
  const int MAX_DIM = 12;
  const int NR_NON_PLAIN_THEMED = Rnd::range(1, 4);

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
        r->roomTheme = RoomThemeId::plain;
        isAssigned.at(i) = true;
        continue;
      }
    }
  }

  trace << "RoomThemeMaker: Trying to set non-plain themes ";
  trace << "for some rooms" << endl;
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false, eng), blockers);
  const int NR_TRIES_TO_ASSIGN = 100;
  for(int i = 0; i < NR_NON_PLAIN_THEMED; i++) {
    for(int ii = 0; ii < NR_TRIES_TO_ASSIGN; ii++) {
      const int ELEMENT = Rnd::range(0, NR_ROOMS - 1);
      if(isAssigned.at(ELEMENT) == false) {
        const RoomThemeId theme =
          (RoomThemeId)(Rnd::range(1, int(RoomThemeId::endOfRoomThemes) - 1));
        Room* const room = rooms.at(ELEMENT);

        if(isThemeAllowed(room, theme, blockers)) {
          room->roomTheme = theme;
          trace << "RoomThemeMaker: Assigned non-plain theme";
          trace << "(" << int(theme) << ") to room" << endl;
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
      rooms.at(i)->roomTheme = RoomThemeId::plain;
      isAssigned.at(i) = true;
    }
  }

  trace << "RoomThemeMaker::assignRoomThemes() [DONE]" << endl;
}

