#include "MapParsing.h"

#include "Engine.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "GameTime.h"

//------------------------------------------------------------ CELL PREDICATES
bool CellPredBlocksVision::check(const Cell& c)  {
  return c.featureStatic.isVisionPassable() == false;
}

bool CellPredBlocksVision::check(const FeatureMob& f) {
  return f.isVisionPassable() == false;
}

bool CellPredBlocksBodyType::check(const FeatureMob& f) {
  return f.isBodyTypePassable(bodyType_) == false;
}

bool CellPredBlocksBodyType::check(const Actor& a) {
  return a.deadState == actorDeadState_alive;
}

bool CellPredBlocksProjectiles::check(const Cell& c)  {
  return c.featureStatic.isProjectilesPassable() == false;
}

bool CellPredBlocksProjectiles::check(const FeatureMob& f)  {
  return f.isProjectilesPassable() == false;
}

bool CellPredLivingActorsAdjToPos::check(const Actor& a) const {
  if(a.deadState != actorDeadState_alive) {
    return false;
  }
  return eng->basicUtils->isPosAdj(pos_, a.pos);
}

//------------------------------------------------------------ MAP PARSER
static void MapParser::parse(
  const CellPred& predicate, bool arrayOut[MAP_X_CELLS][MAP_Y_CELLS],
  const MapParseWriteRule writeRule = mapParseWriteAlways) {

  if(
    predicate.isCheckingCells()       == false &&
    predicate.isCheckingMobFeatures() == false &&
    predicate.isCheckingActors()      == false) {
    throw runtime_error("Predicate not checking anything");
  }

  const Engine* const eng = predicate.eng;

  const bool WRITE_T = writeRule == mapParseWriteAlways ||
                       writeRule == mapParseWriteOnlyTrue;

  const bool WRITE_F = writeRule == mapParseWriteAlways ||
                       writeRule == mapParseWriteOnlyFalse;

  if(predicate.isCheckingCells()) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      for(int x = 0; x < MAP_X_CELLS; x++) {
        const Cell& c = eng->map->cells[x][y];
        const bool IS_MATCH = predicate.check(c);
        if((IS_MATCH && WRITE_T) || (IS_MATCH == false && WRITE_F)) {
          arrayOut[x][y] = IS_MATCH;
        }
      }
    }
  }

  if(predicate.isCheckingMobFeatures()) {
    const int NR_MOB_FEATURES = eng->gameTime->getFeatureMobsSize();
    for(int i = 0; i < NR_MOB_FEATURES; i++) {
      const FeatureMob& f = eng->gameTime->getFeatureMobAtElement(i);
      const bool IS_MATCH = predicate.check(f);
      if((IS_MATCH && WRITE_T) || (IS_MATCH == false && WRITE_F)) {
        arrayOut[x][y] = IS_MATCH;
      }
    }
  }

  if(predicate.isCheckingActors()) {
    const int NR_ACTORS = eng->gameTime->getNrActors();
    for(int i = 0; i < NR_ACTORS; i++) {
      const Actor& a = eng->gameTime->getActorAtElement(i);
      const bool IS_MATCH = predicate.check(a);
      if((IS_MATCH && WRITE_T) || (IS_MATCH == false && WRITE_F)) {
        arrayOut[x][y] = IS_MATCH;
      }
    }
  }
}


bool IsCloserToOrigin::operator()(const Pos& c1, const Pos& c2) {
  const int chebDist1 = eng->basicUtils->chebyshevDist(c_.x, c_.y, c1.x, c1.y);
  const int chebDist2 = eng->basicUtils->chebyshevDist(c_.x, c_.y, c2.x, c2.y);
  return chebDist1 < chebDist2;
}

//void MapTests::getActorsPositions(const vector<Actor*>& actors,
//                                  vector<Pos>& vectorToFill) {
//  vectorToFill.resize(0);
//  const unsigned int NR_ACTORS = actors.size();
//  for(unsigned int i = 0; i < NR_ACTORS; i++) {
//    vectorToFill.push_back(actors.at(i)->pos);
//  }
//}
//
//
//void MapTests::makeVisionBlockerArray(
//  const Pos& origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS],
//  const int MAX_VISION_RANGE) {
//
//  const int X0 = max(0, origin.x - MAX_VISION_RANGE - 1);
//  const int Y0 = max(0, origin.y - MAX_VISION_RANGE - 1);
//  const int X1 = min(MAP_X_CELLS - 1, origin.x + MAX_VISION_RANGE + 1);
//  const int Y1 = min(MAP_Y_CELLS - 1, origin.y + MAX_VISION_RANGE + 1);
//
//  const Map* const map = eng->map;
//
//  for(int y = Y0; y <= Y1; y++) {
//    for(int x = X0; x <= X1; x++) {
//      arrayToFill[x][y] =
//        map->featuresStatic[x][y]->isVisionPassable() == false;
//    }
//  }
//
//  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
//  int x = 0;
//  int y = 0;
//  FeatureMob* f = NULL;
//  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
//    f = eng->gameTime->getFeatureMobAt(i);
//    x = f->getX();
//    y = f->getY();
//    if(map->featuresStatic[x][y]->isVisionPassable()) {
//      arrayToFill[x][y] = f->isVisionPassable() == false;
//    }
//  }
//}
//
//void MapTests::makeMoveBlockerArray(
//  const Actor* const actorMoving,
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  makeMoveBlockerArrayForBodyType(actorMoving->getBodyType(), arrayToFill);
//}
//
//void MapTests::makeMoveBlockerArrayFeaturesOnly(
//  const Actor* const actorMoving,
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  makeMoveBlockerArrayForBodyTypeFeaturesOnly(
//    actorMoving->getBodyType(), arrayToFill);
//}
//
//void MapTests::makeWalkBlockingArrayFeaturesOnly(
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  makeMoveBlockerArrayForBodyTypeFeaturesOnly(
//    bodyType_normal, arrayToFill);
//}
//
//void MapTests::makeMoveBlockerArrayForBodyTypeFeaturesOnly(
//  const BodyType_t bodyType,
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  const Map* const map = eng->map;
//
//  for(int y = 0; y < MAP_Y_CELLS; y++) {
//    for(int x = 0; x < MAP_X_CELLS; x++) {
//      arrayToFill[x][y] =
//        map->featuresStatic[x][y]->isBodyTypePassable(bodyType) == false;
//    }
//  }
//  GameTime* const gameTime = eng->gameTime;
//  FeatureMob* f = NULL;
//  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
//  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
//    f = gameTime->getFeatureMobAt(i);
//    const Pos& pos = f->getPos();
//    if(arrayToFill[pos.x][pos.y] == false) {
//      arrayToFill[pos.x][pos.y] = f->isBodyTypePassable(bodyType) == false;
//    }
//  }
//}
//
//void MapTests::makeShootBlockerFeaturesArray(
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  for(int y = 0; y < MAP_Y_CELLS; y++) {
//    for(int x = 0; x < MAP_X_CELLS; x++) {
//      arrayToFill[x][y] = eng->map->featuresStatic[x][y]->isProjectilesPassable() == false;
//    }
//  }
//  FeatureMob* f = NULL;
//  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
//  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
//    f = eng->gameTime->getFeatureMobAt(i);
//    if(arrayToFill[f->getX()][f->getY()] == false) {
//      arrayToFill[f->getX()][f->getY()] = f->isProjectilesPassable() == false;
//    }
//  }
//}
//
//void MapTests::addItemsToBlockerArray(
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  for(int y = 0; y < MAP_Y_CELLS; y++) {
//    for(int x = 0; x < MAP_X_CELLS; x++) {
//      if(eng->map->items[x][y] != NULL) {
//        arrayToFill[x][y] = true;
//      }
//    }
//  }
//}
//
//void MapTests::makeMoveBlockerArrayForBodyType(
//  const BodyType_t bodyType,
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  for(int y = 0; y < MAP_Y_CELLS; y++) {
//    for(int x = 0; x < MAP_X_CELLS; x++) {
//      arrayToFill[x][y] =
//        eng->map->featuresStatic[x][y]->isBodyTypePassable(bodyType) == false;
//    }
//  }
//  FeatureMob* f = NULL;
//  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
//  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
//    f = eng->gameTime->getFeatureMobAt(i);
//    const Pos& pos = f->getPos();
//    if(arrayToFill[pos.x][pos.y] == false) {
//      arrayToFill[pos.x][pos.y] = f->isBodyTypePassable(bodyType) == false;
//    }
//  }
//  addLivingActorsToBlockerArray(arrayToFill);
//}
//
//void MapTests::makeItemBlockerArray(
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  for(int y = MAP_Y_CELLS - 1; y >= 0; y--) {
//    for(int x = MAP_X_CELLS - 1; x >= 0; x--) {
//      arrayToFill[x][y] = !eng->map->featuresStatic[x][y]->canHaveItem();
//    }
//  }
//  FeatureMob* f = NULL;
//  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
//  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
//    f = eng->gameTime->getFeatureMobAt(i);
//    if(arrayToFill[f->getX()][f->getY()] == false) {
//      arrayToFill[f->getX()][f->getY()] = !f->canHaveItem();
//    }
//  }
//  //addActorsToBlockerArray(arrayToFill); //Why?
//}
//
//void MapTests::addLivingActorsToBlockerArray(
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  Actor* a = NULL;
//  const unsigned int NR_ACTORS = eng->gameTime->getLoopSize();
//  for(unsigned int i = 0; i < NR_ACTORS; i++) {
//    a = eng->gameTime->getActorAtElement(i);
//    if(a->deadState == actorDeadState_alive) {
//      if(arrayToFill[a->pos.x][a->pos.y] == false) {
//        arrayToFill[a->pos.x][a->pos.y] = true;
//      }
//    }
//  }
//}
//
//void MapTests::addAllActorsToBlockerArray(
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  Actor* a = NULL;
//  const unsigned int NR_ACTORS = eng->gameTime->getLoopSize();
//  for(unsigned int i = 0; i < NR_ACTORS; i++) {
//    a = eng->gameTime->getActorAtElement(i);
//    if(arrayToFill[a->pos.x][a->pos.y] == false) {
//      arrayToFill[a->pos.x][a->pos.y] = true;
//    }
//  }
//}
//
//void MapTests::addAdjLivingActorsToBlockerArray(
//  const Pos& origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  Actor* a = NULL;
//  const unsigned int NR_ACTORS = eng->gameTime->getLoopSize();
//  for(unsigned int i = 0; i < NR_ACTORS; i++) {
//    a = eng->gameTime->getActorAtElement(i);
//    if(a->deadState == actorDeadState_alive) {
//      if(arrayToFill[a->pos.x][a->pos.y] == false && a->pos != origin) {
//        if(isCellsAdj(origin, a->pos, false)) {
//          arrayToFill[a->pos.x][a->pos.y] = true;
//        }
//      }
//    }
//  }
//}
//
//bool MapTests::isCellNextToPlayer(
//  const Pos& pos,
//  const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const {
//
//  return isCellsAdj(pos, eng->player->pos, COUNT_SAME_CELL_AS_NEIGHBOUR);
//}
