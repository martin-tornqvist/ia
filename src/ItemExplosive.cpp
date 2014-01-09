#include "ItemExplosive.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "GameTime.h"
#include "PlayerBonuses.h"

void Dynamite::setPlayerExplosive() const {
  eng.player->dynamiteFuseTurns = 6;

  const bool IS_SWIFT_LIGHT =
    eng.playerBonHandler->hasTrait(traitDemolitionExpert) &&
    eng.dice.coinToss();
  const string swiftStr = IS_SWIFT_LIGHT ? "swiftly " : "";

  eng.player->clr_ = clrYellow;
  eng.log->addMsg("I " + swiftStr + "light a dynamite stick.");
  eng.renderer->drawMapAndInterface();
  eng.gameTime->actorDidAct(IS_SWIFT_LIGHT);
}

void Molotov::setPlayerExplosive() const {
  eng.player->molotovFuseTurns = 12;

  const bool IS_SWIFT_LIGHT =
    eng.playerBonHandler->hasTrait(traitDemolitionExpert) &&
    eng.dice.coinToss();
  const string swiftStr = IS_SWIFT_LIGHT ? "swiftly " : "";

  eng.player->clr_ = clrYellow;
  eng.log->addMsg("I " + swiftStr + "light a Molotov Cocktail.");
  eng.renderer->drawMapAndInterface();
  eng.gameTime->actorDidAct(IS_SWIFT_LIGHT);
}

void Flare::setPlayerExplosive() const {
  eng.player->flareFuseTurns = 200;

  const bool IS_SWIFT_LIGHT =
    eng.playerBonHandler->hasTrait(traitDemolitionExpert) &&
    eng.dice.coinToss();
  const string swiftStr = IS_SWIFT_LIGHT ? "swiftly " : "";

  eng.player->clr_ = clrYellow;
  eng.log->addMsg("I " + swiftStr + "light a Flare.");
  eng.gameTime->updateLightMap();
  eng.player->updateFov();
  eng.renderer->drawMapAndInterface();
  eng.gameTime->actorDidAct(IS_SWIFT_LIGHT);
}
