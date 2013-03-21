#include "MapBuild.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "FeatureFactory.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "Map.h"
#include "FeatureWall.h"

void MapBuild::buildKingsTomb() {
  eng->map->clearDungeon();

  buildFromTemplate(coord(0,0), mapTemplate_pharaohsChamber);

  eng->player->pos = coord(3, 13);

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      Feature* const f = eng->map->featuresStatic[x][y];
      if(f->getId() == feature_stoneWall) {
        dynamic_cast<Wall*>(f)->wallType = wall_egypt;
      }
    }
  }

  eng->featureFactory->spawnFeatureAt(feature_stairsDown, coord(75, 16), NULL);

  dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_khephren, coord(65, 19)))->isRoamingAllowed = false;

  dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_mummy, coord(53, 19)))->isRoamingAllowed = false;
  dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_mummy, coord(57, 19)))->isRoamingAllowed = false;
  dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_mummy, coord(61, 19)))->isRoamingAllowed = false;

  dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_mummy, coord(65, 7)))->isRoamingAllowed = false;
  dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_mummy, coord(65, 11)))->isRoamingAllowed = false;
  dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_mummy, coord(65, 15)))->isRoamingAllowed = false;

//  dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_mummy, coord(64, 18)))->isRoamingAllowed = false;
//  dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_mummy, coord(64, 19)))->isRoamingAllowed = false;
//  dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_mummy, coord(64, 20)))->isRoamingAllowed = false;
//  dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_mummy, coord(65, 18)))->isRoamingAllowed = false;
//  dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_mummy, coord(65, 20)))->isRoamingAllowed = false;

//  const FeatureDef* const mimicFeature = eng->featureData->getFeatureDef(feature_caveFloor);
//  eng->featureFactory->spawnFeatureAt(
//    feature_trap,
//    coord(MAP_X_CELLS - 2, MAP_Y_CELLS_HALF),
//    new TrapSpawnData(mimicFeature, trap_teleport));

//  coord c(MAP_X_CELLS - 5, MAP_Y_CELLS_HALF - 1);
//  Monster* const mummyBoss = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_khephren, c));
//  for(int dx = -1; dx <= 1; dx++) {
//    for(int dy = -1; dy <= 1; dy++) {
//      Monster* const monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_fireVortex, c + coord(dx, dy)));
//      monster->leader = mummyBoss;
//    }
//  }
//
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);
//  eng->basicUtils->reverseBoolArray(blockers);
//  vector<coord> freeCells;
//  eng->mapTests->makeMapVectorFromArray(blockers, freeCells);
//
//  for(int i = eng->dice(1, 2) + 3; i > 0 && freeCells.size() != 0; i--) {
//    const unsigned int ELEMENT = eng->dice(1, freeCells.size()) - 1;
//    const coord pos = freeCells.at(ELEMENT);
//    freeCells.erase(freeCells.begin() + ELEMENT);
//    eng->actorFactory->spawnActor(actor_mummy, pos);
//  }
}
