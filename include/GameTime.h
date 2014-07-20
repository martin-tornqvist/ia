#ifndef GAME_TIME_H
#define GAME_TIME_H

#include <vector>

#include "Feature.h"
#include "ActorData.h"

class FeatureMob;

enum class TurnType {fast, normal1, slow, fastest, normal2, END};

namespace GameTime {

extern std::vector<Actor*>       actors_;
extern std::vector<FeatureMob*>  featureMobs_;

void init();
void cleanup();

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

void addActor(Actor* actor);

void actorDidAct(const bool IS_FREE_TURN = false);

int getTurn();

Actor* getCurActor();

void eraseActorInElement(const size_t i);

void getFeatureMobsAtPos(const Pos& pos, std::vector<FeatureMob*>& vectorRef);

void addMob(FeatureMob* const f);

void eraseFeatureMob(FeatureMob* const f, const bool DESTROY_OBJECT);

void eraseAllFeatureMobs();

void resetTurnTypeAndActorCounters();

void updateLightMap();

} //GameTime

#endif
