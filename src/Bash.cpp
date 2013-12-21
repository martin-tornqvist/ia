#include "Bash.h"

#include "Engine.h"

#include "GameTime.h"
#include "Feature.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"
#include "Renderer.h"
#include "MapParsing.h"

void Bash::playerBash() const {
  trace << "Bash::playerBash()" << endl;

  eng.log->clearLog();
  eng.log->addMsg("Which direction? | space/esc cancel", clrWhiteHigh);
  eng.renderer->drawMapAndInterface();
  Pos bashPos(eng.player->pos + eng.query->dir());
  eng.log->clearLog();

  if(bashPos != eng.player->pos) {
    Actor* actor = eng.basicUtils->getActorAtPos(bashPos);

    if(actor == NULL) {
      trace << "Bash: No actor at bash pos, ";
      trace << "attempting to bash feature instead" << endl;
      Cell& cell = eng.map->cells[bashPos.x][bashPos.y];
      cell.featureStatic->tryBash(*eng.player);
    }  else {
      trace << "Bash: Actor found at bash pos, attempt kicking actor" << endl;
      if(eng.player->getPropHandler()->allowAttackMelee(true)) {
        trace << "Bash: Player is allowed to do melee attack" << endl;
        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        MapParser::parse(CellPredBlocksVision(eng), blockers);

        trace << "Bash: Player can see actor" << endl;
        eng.player->kick(*actor);
        return;
      }
    }
  }
}
