#include "Examine.h"

#include "Engine.h"
#include "Feature.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"

void Examine::playerExamine() const {
  eng->log->addMsg("Which direction? | space/esc to cancel", clrWhiteHigh);
  eng->renderer->drawMapAndInterface();
  Pos examineInPos = eng->player->pos + eng->query->direction();
  eng->log->clearLog();

  Feature* const f = eng->map->featuresStatic[examineInPos.x][examineInPos.y];
  f->examine();
}
