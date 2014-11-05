#ifndef ACTOR_FACTORY
#define ACTOR_FACTORY

#include <vector>

#include "ActorData.h"

class Actor;
class Mon;

namespace ActorFactory {

void deleteAllMon();

Actor* mk(const ActorId id, const Pos& pos);

void summonMon(const Pos& origin, const std::vector<ActorId>& monsterIds,
                    const bool MAKE_MONSTERS_AWARE,
                    Actor* const actorToSetAsLeader = nullptr,
                    std::vector<Mon*>* monstersRet = nullptr);

} //ActorFactory

#endif
