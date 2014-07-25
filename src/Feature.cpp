#include "Feature.h"

#include "Init.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "MapParsing.h"
#include "GameTime.h"
#include "MapTravel.h"
#include "Query.h"
#include "SaveHandling.h"
#include "Popup.h"
#include "Utils.h"
#include "Map.h"
#include "FeatureData.h"

using namespace std;

const FeatureDataT& Feature::getData() const {return FeatureData::getData(getId());}

void Feature::bump(Actor& actorBumping) {
  vector<PropId> props;
  actorBumping.getPropHandler().getAllActivePropIds(props);

  if(!canMove(props)) {
    if(&actorBumping == Map::player) {
      if(Map::player->getPropHandler().allowSee()) {
        Log::addMsg(getData().msgOnPlayerBlocked);
      } else {
        Log::addMsg(getData().msgOnPlayerBlockedBlind);
      }
    }
  }
}

void Feature::addLight(bool light[MAP_W][MAP_H]) const {(void)light;}

bool Feature::canMoveCmn() const {return getData().moveRules.canMoveCmn();}

bool Feature::canMove(const vector<PropId>& actorsProps) const {
  return getData().moveRules.canMove(actorsProps);
}

bool Feature::isSoundPassable()           const {return getData().isSoundPassable;}
bool Feature::isVisionPassable()          const {return getData().isVisionPassable;}
bool Feature::isProjectilePassable()      const {return getData().isProjectilePassable;}
bool Feature::isSmokePassable()           const {return getData().isSmokePassable;}
bool Feature::isBottomless()              const {return getData().isBottomless;}

void Feature::hit(const DmgType dmgType, const DmgMethod dmgMethod, Actor* const actor) {
  (void)dmgType; (void)dmgMethod; (void)actor;
}

char      Feature::getGlyph()             const {return getData().glyph;}
TileId    Feature::getTile()              const {return getData().tile;}
bool      Feature::canHaveCorpse()        const {return getData().canHaveCorpse;}
bool      Feature::canHaveRigid() const {return getData().canHaveRigid;}
bool      Feature::canHaveBlood()         const {return getData().canHaveBlood;}
bool      Feature::canHaveGore()          const {return getData().canHaveGore;}
bool      Feature::canHaveItem()          const {return getData().canHaveItem;}
FeatureId Feature::getId()                const {return getData().id;}
int       Feature::getDodgeModifier()     const {return getData().dodgeModifier;}
int       Feature::getShockWhenAdj()      const {return getData().shockWhenAdjacent;}
Matl      Feature::getMatl()              const {return getData().matlType;}
