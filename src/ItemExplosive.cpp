#include "ItemExplosive.h"

#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "GameTime.h"
#include "PlayerBon.h"
#include "Utils.h"

void Dynamite::setPlayerExplosive() const {
  Map::player->dynamiteFuseTurns = 6;

  const bool IS_SWIFT =
    PlayerBon::hasTrait(Trait::demolitionExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  Map::player->clr_ = clrYellow;
  eng.log->addMsg("I " + swiftStr + "light a dynamite stick.");
  Renderer::drawMapAndInterface();
  GameTime::actorDidAct(IS_SWIFT);
}

void Molotov::setPlayerExplosive() const {
  Map::player->molotovFuseTurns = 12;

  const bool IS_SWIFT =
    PlayerBon::hasTrait(Trait::demolitionExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  Map::player->clr_ = clrYellow;
  eng.log->addMsg("I " + swiftStr + "light a Molotov Cocktail.");
  Renderer::drawMapAndInterface();
  GameTime::actorDidAct(IS_SWIFT);
}

void Flare::setPlayerExplosive() const {
  Map::player->flareFuseTurns = 200;

  const bool IS_SWIFT =
    PlayerBon::hasTrait(Trait::demolitionExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  Map::player->clr_ = clrYellow;
  eng.log->addMsg("I " + swiftStr + "light a Flare.");
  GameTime::updateLightMap();
  Map::player->updateFov();
  Renderer::drawMapAndInterface();
  GameTime::actorDidAct(IS_SWIFT);
}
