#include "ItemExplosive.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "GameTime.h"
#include "PlayerBon.h"
#include "Utils.h"

void Dynamite::setPlayerExplosive() const {
  eng.player->dynamiteFuseTurns = 6;

  const bool IS_SWIFT =
    PlayerBon::hasTrait(Trait::demolitionExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  eng.player->clr_ = clrYellow;
  eng.log->addMsg("I " + swiftStr + "light a dynamite stick.");
  Renderer::drawMapAndInterface();
  eng.gameTime->actorDidAct(IS_SWIFT);
}

void Molotov::setPlayerExplosive() const {
  eng.player->molotovFuseTurns = 12;

  const bool IS_SWIFT =
    PlayerBon::hasTrait(Trait::demolitionExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  eng.player->clr_ = clrYellow;
  eng.log->addMsg("I " + swiftStr + "light a Molotov Cocktail.");
  Renderer::drawMapAndInterface();
  eng.gameTime->actorDidAct(IS_SWIFT);
}

void Flare::setPlayerExplosive() const {
  eng.player->flareFuseTurns = 200;

  const bool IS_SWIFT =
    PlayerBon::hasTrait(Trait::demolitionExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  eng.player->clr_ = clrYellow;
  eng.log->addMsg("I " + swiftStr + "light a Flare.");
  eng.gameTime->updateLightMap();
  eng.player->updateFov();
  Renderer::drawMapAndInterface();
  eng.gameTime->actorDidAct(IS_SWIFT);
}
