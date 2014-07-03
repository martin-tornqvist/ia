#include "MapGen.h"

#include <vector>

#include "ActorPlayer.h"
#include "FeatureFactory.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "Map.h"
#include "FeatureWall.h"
#include "Utils.h"
#include "ActorFactory.h"

using namespace std;

namespace MapGen {

namespace EgyptTomb {

bool run() {
  Map::resetMap();

  MapGenUtils::mkFromTempl(Pos(0, 0), MapTemplId::egypt);

  Map::player->pos = Pos(40, 11);

  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      Feature* const f = Map::cells[x][y].featureStatic;
      if(f->getId() == FeatureId::wall) {
        static_cast<Wall*>(f)->wallType = WallType::egypt;
      }
    }
  }

  if(Rnd::coinToss()) {
    FeatureFactory::mk(FeatureId::stairs, Pos(4, 2), nullptr);
  } else {
    FeatureFactory::mk(FeatureId::stairs, Pos(4, 19), nullptr);
  }


  vector<Actor*> actors;

  actors.push_back(ActorFactory::mk(actor_mummy,     Pos(12, 10)));
  actors.push_back(ActorFactory::mk(actor_khephren,  Pos(11, 11)));
  actors.push_back(ActorFactory::mk(actor_mummy,     Pos(12, 12)));
  actors.push_back(ActorFactory::mk(actor_cultist,   Pos(17, 10)));
  actors.push_back(ActorFactory::mk(actor_cultist,   Pos(17, 12)));

  for(Actor* a : actors) {static_cast<Monster*>(a)->isRoamingAllowed_ = true;}

  return true;
}

} //EgyptTomb

} //MapGen
