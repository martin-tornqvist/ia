#include "MapGen.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "FeatureFactory.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "Map.h"
#include "FeatureWall.h"

bool MapGenEgyptTomb::run_() {
  eng.map->resetMap();

  buildFromTemplate(Pos(0, 0), mapTemplate_pharaohsChamber);

  eng.player->pos = Pos(40, 11);

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      Feature* const f = eng.map->cells[x][y].featureStatic;
      if(f->getId() == feature_stoneWall) {
        dynamic_cast<Wall*>(f)->wallType = wall_egypt;
      }
    }
  }

  if(eng.dice.coinToss()) {
    eng.featureFactory->spawnFeatureAt(feature_stairsDown, Pos(4, 2), NULL);
  } else {
    eng.featureFactory->spawnFeatureAt(feature_stairsDown, Pos(4, 19), NULL);
  }


  dynamic_cast<Monster*>(
    eng.actorFactory->spawnActor(
      actor_mummy, Pos(12, 10)))->isRoamingAllowed_ = false;
  dynamic_cast<Monster*>(
    eng.actorFactory->spawnActor(
      actor_khephren, Pos(11, 11)))->isRoamingAllowed_ = false;
  dynamic_cast<Monster*>(
    eng.actorFactory->spawnActor(
      actor_mummy, Pos(12, 12)))->isRoamingAllowed_ = false;

  dynamic_cast<Monster*>(
    eng.actorFactory->spawnActor(
      actor_cultist, Pos(17, 10)))->isRoamingAllowed_ = false;
  dynamic_cast<Monster*>(
    eng.actorFactory->spawnActor(
      actor_cultist, Pos(17, 12)))->isRoamingAllowed_ = false;

  return true;
}
