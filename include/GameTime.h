#ifndef GAME_TIME_H
#define GAME_TIME_H

#include <vector>

#include "Feature.h"
#include "ActorData.h"

class Mob;

enum class TurnType
{
    fast,
    normal1,
    slow,
    fastest,
    normal2,
    END
};

namespace GameTime
{

extern std::vector<Actor*>       actors_;
extern std::vector<Mob*>  mobs_;

void init();
void cleanup();

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

void addActor(Actor* actor);

void tick(const bool IS_FREE_TURN = false);

int getTurn();

Actor* getCurActor();

void eraseActorInElement(const size_t i);

void getMobsAtPos(const Pos& pos, std::vector<Mob*>& vectorRef);

void addMob(Mob* const f);

void eraseMob(Mob* const f, const bool DESTROY_OBJECT);

void eraseAllMobs();

void resetTurnTypeAndActorCounters();

void updateLightMap();

} //GameTime

#endif
