#ifndef ACTOR_FACTORY
#define ACTOR_FACTORY

#include <vector>
#include <string>

#include "GameTime.h"

using namespace std;


class Monster;

class ActorFactory {
public:
  ActorFactory() {}

  void deleteAllMonsters() const;

  Actor* spawnActor(const ActorId id, const Pos& pos) const;

  void summonMonsters(const Pos& origin, const vector<ActorId>& monsterIds,
                      const bool MAKE_MONSTERS_AWARE,
                      Actor* const actorToSetAsLeader = NULL,
                      vector<Monster*>* monstersRet = NULL) const;

private:
  friend class DebugModeStatPrinter;
  Actor* makeActorFromId(const ActorId id) const;


};

#endif
