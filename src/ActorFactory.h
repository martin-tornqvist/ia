#ifndef ACTOR_FACTORY
#define ACTOR_FACTORY

#include <vector>
#include <string>

#include "GameTime.h"

using namespace std;

class Engine;

class ActorFactory {
public:
	ActorFactory(Engine* engine) {
		eng = engine;
	}
	void deleteAllMonsters();
    Actor* spawnActor(const ActorDevNames_t characterType, const coord& pos);

	Actor* spawnRandomActor(const coord& pos, const int SPAWN_LVL_OFFSET);

//	Actor* spawnRandomActorRelatedToSpecialRoom(const coord& pos, const SpecialRoom_t roomType, const int SPAWN_LEVEL_OFFSET);

private:
	Engine* eng;
};

#endif
