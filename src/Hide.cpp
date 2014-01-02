#include "Hide.h"

#include <algorithm>

#include "Engine.h"

#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Log.h"

void Hide::playerTryHide() {

  const int NR_ACTORS = eng.gameTime->getNrActors();

  bool isHideSucceed = true;

  if(eng.dice.oneIn(3)) {
    isHideSucceed = false;
  } else {
    vector<Actor*> actors; actors.resize(0);

    for(int i = 0; i < NR_ACTORS; i++) {
      Actor* const actor = &eng.gameTime->getActorAtElement(i);
      if(actor != eng.player) {
//        if(dynamic_cast<Monster*>(actor)->playerAwarenessCounter > 0) {
          actors.push_back(actor);
//        }
      }
    }

    //Cap number of monsters attempting to spot player
    const Pos& playerPos = eng.player->pos;
    sort(actors.begin(), actors.end(),
    [this, playerPos](const Actor * a1, const Actor * a2) {
      const int A1_DIST = eng.basicUtils->chebyshevDist(a1->pos, playerPos);
      const int A2_DIST = eng.basicUtils->chebyshevDist(a2->pos, playerPos);
      return A1_DIST < A2_DIST;
    });
    actors.resize(min(int(actors.size()), 3));

    for(Actor * actor : actors) {
      if(actor->isSpottingHiddenActor(*eng.player)) {
        isHideSucceed = false;
        break;
      }
    }
  }

  if(isHideSucceed) {
    for(int i = 0; i < NR_ACTORS; i++) {
      Actor* const actor = &eng.gameTime->getActorAtElement(i);
      if(actor != eng.player) {
        Monster* const monster = dynamic_cast<Monster*>(actor);
        monster->playerAwarenessCounter = 0;
      }
    }
//    eng.log->addMsg("I hide!");
  } else {
//    eng.log->addMsg("I fail to hide!");
  }

//  eng.gameTime->actorDidAct();
}
