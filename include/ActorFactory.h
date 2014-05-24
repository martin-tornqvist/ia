#ifndef ACTOR_FACTORY
#define ACTOR_FACTORY

#include <vector>

#include "ActorData.h"

class Actor;
class Monster;

namespace ActorFactory {

void deleteAllMonsters();

Actor* mk(const ActorId id, const Pos& pos);

void summonMonsters(const Pos& origin, const std::vector<ActorId>& monsterIds,
                    const bool MAKE_MONSTERS_AWARE,
                    Actor* const actorToSetAsLeader = nullptr,
                    std::vector<Monster*>* monstersRet = nullptr);

} //ActorFactory

#endif
