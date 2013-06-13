#include "Bash.h"

#include "Engine.h"

#include "GameTime.h"
#include "Feature.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"

void Bash::playerBash() const {
  tracer << "Bash::playerBash()" << endl;

  eng->log->clearLog();
  eng->log->addMessage("Kick or bash in which direction? | space/esc cancel", clrWhiteHigh);
  eng->renderer->drawMapAndInterface();
  coord bashInPos(eng->player->pos + eng->query->direction());
  eng->log->clearLog();

  if(bashInPos != eng->player->pos) {
    Actor* actor = eng->mapTests->getActorAtPos(bashInPos);

    if(actor == NULL) {
      tracer << "Bash: No actor at bash pos, attempting to bash feature instead" << endl;
      playerBashFeature(eng->map->featuresStatic[bashInPos.x][bashInPos.y]);
    }  else {
      tracer << "Bash: Actor found at bash pos, attempt kicking actor" << endl;
      if(eng->player->getStatusEffectsHandler()->allowAttackMelee(true)) {
        tracer << "Bash: Player is allowed to do melee attack" << endl;
        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        eng->mapTests->makeVisionBlockerArray(eng->player->pos, blockers);

        if(eng->player->checkIfSeeActor(*actor, blockers)) {
          tracer << "Bash: Player can see actor" << endl;
          eng->player->kick(*actor);
          return;
        }
      }
    }
  }
}

void Bash::playerBashFeature(Feature* const feature) const {
  bool bashableObjectFound = false;

  if(feature->getId() == feature_door) {
    Door* const door = dynamic_cast<Door*>(feature);
    door->tryBash(eng->player);
    bashableObjectFound = true;
  }

  if(bashableObjectFound == false) {
    const bool PLAYER_IS_BLIND = eng->player->getStatusEffectsHandler()->allowSee();
    if(PLAYER_IS_BLIND == false) {
      eng->log->addMessage("I see nothing there to bash.");
    } else {
      eng->log->addMessage("I find nothing there to bash.");
    }
  }
}

