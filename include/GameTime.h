#ifndef GAME_TIME_H
#define GAME_TIME_H

#include <vector>

#include "Feature.h"
#include "ActorData.h"

class FeatureMob;

enum class TurnType {fast, normal1, slow, fastest, normal2, endOfTurnType};

namespace GameTime {

extern std::vector<Actor*>       actors_;
extern std::vector<FeatureMob*>  featureMobs_;

void init();
void cleanup();

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

void insertActorInLoop(Actor* actor);

void actorDidAct(const bool IS_FREE_TURN = false);

int getTurn();

Actor* getCurActor();

void eraseActorInElement(const unsigned int i);

void getFeatureMobsAtPos(const Pos& pos, std::vector<FeatureMob*>& vectorRef);

void addFeatureMob(FeatureMob* const feature);

void eraseFeatureMob(FeatureMob* const feature, const bool DESTROY_OBJECT);

void eraseAllFeatureMobs();

void resetTurnTypeAndActorCounters();

void updateLightMap();

} //GameTime

#endif
