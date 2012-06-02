#include "ItemUsable.h"

#include "Actor.h"
#include "Engine.h"

/*
ItemActivateReturnData ItemMedicinePouch::use(Actor* actor, Engine* engine)
{
ItemActivateReturnData retValues;
retValues.removed = true;

if(actor == engine->player) {
engine->log->addMessage("You use a Pouch of Medicine.");
}
actor->restoreHP(engine->basicUtils->dice(1,6));
engine->gameTime->letNextAct();

return retValues;
}
*/

/*
ItemActivateReturnData ItemFirstAidKit::use(Actor* actor, Engine* engine)
{
	ItemActivateReturnData retValues;
	retValues.removed = false;

	const int TURNS_TO_APPLY	= 12;
	const string TURNS_STR		= intToString(TURNS_TO_APPLY);

	if(actor == engine->player) {
		if(engine->player->getHP() >= engine->player->getHP_max()) {
			engine->log->addMessage("You are already at good health.");
		} else {
			engine->log->addMessage("You start applying first aid (" + TURNS_STR + " turns).");
			engine->player->firstAidTurnsLeft = TURNS_TO_APPLY;
			engine->gameTime->letNextAct();
		}
	}

	return retValues;
}
*/

/*
ItemActivateReturnData ItemDoctorsBag::use(Actor* actor, Engine* engine)
{
ItemActivateReturnData retValues;
retValues.removed = true;

if(actor == engine->player) {
engine->log->addMessage("You use a Doctor's Bag.");
}
actor->restoreHP(engine->basicUtils->dice(1,6)+12);
engine->gameTime->letNextAct();

return retValues;
}
*/