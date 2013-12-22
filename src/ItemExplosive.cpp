#include "ItemExplosive.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "GameTime.h"

void Dynamite::setPlayerExplosive() const {
  eng.player->dynamiteFuseTurns = 6;

  eng.player->clr_ = clrYellow;
  eng.log->addMsg("I light a dynamite stick.");
  eng.renderer->drawMapAndInterface();
  eng.gameTime->actorDidAct();
}

void Molotov::setPlayerExplosive() const {
  eng.player->molotovFuseTurns = 12;

  eng.player->clr_ = clrYellow;
  eng.log->addMsg("I light a Molotov Cocktail.");
  eng.renderer->drawMapAndInterface();
  eng.gameTime->actorDidAct();
}

void Flare::setPlayerExplosive() const {
  eng.player->flareFuseTurns = 200;

  eng.player->clr_ = clrYellow;
  eng.log->addMsg("I light a Flare.");
  eng.gameTime->updateLightMap();
  eng.player->updateFov();
  eng.renderer->drawMapAndInterface();
  eng.gameTime->actorDidAct();
}
