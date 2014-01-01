#include "Hide.h"

#include "Engine.h"

#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"

void Hide::playerTryHide() {
  eng.gameTime->actorDidAct();

  const int NR_ACTORS = eng.gameTime->getNrActors();
  for(int i = 0; i < NR_ACTORS; i++) {
    Actor* const actor = &eng.gameTime->getActorAtElement(i);
    if(actor != eng.player) {
      Monster* const monster = dynamic_cast<Monster*>(actor);
      monster->playerAwarenessCounter = -1;
    }
  }
}
