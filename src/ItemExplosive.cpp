#include "ItemExplosive.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Render.h"

Explosive::~Explosive() {

}

void Explosive::setPlayerExplosive(Engine* const engine) const {
  (void)engine;
}

void Dynamite::setPlayerExplosive(Engine* const engine) const {
	engine->player->dynamiteFuseTurns = 6;

	engine->player->clr_ = clrYellow;
	engine->log->addMessage("I light a dynamite stick.");
	engine->renderer->drawMapAndInterface();
	engine->gameTime->endTurnOfCurrentActor();
}

void Molotov::setPlayerExplosive(Engine* const engine) const {
	engine->player->molotovFuseTurns = 12;

	engine->player->clr_ = clrYellow;
	engine->log->addMessage("I light a Molotov Cocktail.");
	engine->renderer->drawMapAndInterface();
	engine->gameTime->endTurnOfCurrentActor();
}

void Flare::setPlayerExplosive(Engine* const engine) const {
	engine->player->flareFuseTurns = 200;

	engine->player->clr_ = clrYellow;
	engine->log->addMessage("I light a Flare.");
  engine->gameTime->updateLightMap();
  engine->player->updateFov();
	engine->renderer->drawMapAndInterface();
	engine->gameTime->endTurnOfCurrentActor();
}
