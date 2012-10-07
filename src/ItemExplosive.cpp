#include "ItemExplosive.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Render.h"

void Dynamite::setPlayerExplosive(Engine* const engine) const
{
	engine->player->dynamiteFuseTurns = 6;

	engine->player->getInstanceDefinition()->color = clrYellow;
	engine->log->addMessage("I light a dynamite stick.");
	engine->renderer->drawMapAndInterface();
	engine->gameTime->letNextAct();
}

void Molotov::setPlayerExplosive(Engine* const engine) const
{
	engine->player->molotovFuseTurns = 12;

	engine->player->getInstanceDefinition()->color = clrYellow;
	engine->log->addMessage("I light a Molotov Cocktail.");
	engine->renderer->drawMapAndInterface();
	engine->gameTime->letNextAct();
}

void Flare::setPlayerExplosive(Engine* const engine) const
{
	engine->player->flareFuseTurns = 200;

	engine->player->getInstanceDefinition()->color = clrYellow;
	engine->log->addMessage("I light a Flare.");
	engine->renderer->drawMapAndInterface();
	engine->gameTime->letNextAct();
}
