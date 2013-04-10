#include "Disarm.h"

#include "Engine.h"
#include "Log.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "Query.h"
#include "Map.h"
#include "FeatureTrap.h"

void Disarm::playerDisarm() const {
  eng->log->addMessage("Disarm trap in what direction? | space/esc to cancel", clrWhiteHigh);
  eng->renderer->drawMapAndInterface();
  const coord disarmInPos(eng->player->pos + eng->query->direction());
  eng->log->clearLog();

  bool isDisarmableFound = false;

  Feature* const f = eng->map->featuresStatic[disarmInPos.x][disarmInPos.y];

  if(f->getId() == feature_trap) {
    Trap* const trap = dynamic_cast<Trap*>(f);
    if(trap->isHidden() == false) {
      isDisarmableFound = true;
      trap->trigger(eng->player);
      eng->gameTime->letNextAct();
    }
  }

  if(isDisarmableFound == false) {
    eng->log->addMessage("I see nothing there that I can disarm.");
  }
}
