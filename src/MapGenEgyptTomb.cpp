#include "MapGen.h"

#include "ActorPlayer.h"
#include "FeatureFactory.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "Map.h"
#include "FeatureWall.h"
#include "Utils.h"

bool MapGenEgyptTomb::run_() {
  Map::resetMap();

  buildFromTemplate(Pos(0, 0), mapTemplate_pharaohsChamber);

  Map::player->pos = Pos(40, 11);

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      Feature* const f = Map::cells[x][y].featureStatic;
      if(f->getId() == FeatureId::stoneWall) {
        dynamic_cast<Wall*>(f)->wallType = wall_egypt;
      }
    }
  }

  if(Rnd::coinToss()) {
    FeatureFactory::spawnFeatureAt(FeatureId::stairs, Pos(4, 2), NULL);
  } else {
    FeatureFactory::spawnFeatureAt(FeatureId::stairs, Pos(4, 19), NULL);
  }


  dynamic_cast<Monster*>(
    ActorFactory::spawnActor(
      actor_mummy, Pos(12, 10)))->isRoamingAllowed_ = false;
  dynamic_cast<Monster*>(
    ActorFactory::spawnActor(
      actor_khephren, Pos(11, 11)))->isRoamingAllowed_ = false;
  dynamic_cast<Monster*>(
    ActorFactory::spawnActor(
      actor_mummy, Pos(12, 12)))->isRoamingAllowed_ = false;

  dynamic_cast<Monster*>(
    ActorFactory::spawnActor(
      actor_cultist, Pos(17, 10)))->isRoamingAllowed_ = false;
  dynamic_cast<Monster*>(
    ActorFactory::spawnActor(
      actor_cultist, Pos(17, 12)))->isRoamingAllowed_ = false;

  return true;
}
