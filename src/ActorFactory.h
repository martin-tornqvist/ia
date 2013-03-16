#ifndef ACTOR_FACTORY
#define ACTOR_FACTORY

#include <vector>
#include <string>

#include "GameTime.h"

using namespace std;

class Engine;

class ActorFactory {
public:
  ActorFactory(Engine* engine) : eng(engine) {
  }

  void deleteAllMonsters();

  Actor* spawnActor(const ActorId_t id, const coord& pos);

  Actor* spawnRandomActor(const coord& pos, const int SPAWN_LVL_OFFSET);

private:
  friend class DebugModeStatPrinter;
  Actor* makeActorFromId(const ActorId_t id);

  Engine* eng;
};

#endif
