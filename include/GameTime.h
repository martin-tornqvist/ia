#ifndef GAME_TIME_H
#define GAME_TIME_H

#include <vector>

#include "Feature.h"
#include "ActorData.h"

class Engine;

enum TurnType {
  turnType_fast,
  turnType_normal_1,
  turnType_slow,
  turnType_fastest,
  turnType_normal_2,
  endOfTurnType
};

class GameTime {
public:
  GameTime(Engine& engine) :
    currentTurnTypePos_(0),
    currentActorVectorPos_(0),
    turn_(0), eng(engine) {
    actors_.resize(0);
    featureMobs_.resize(0);
  }

  ~GameTime();

  void addSaveLines(vector<string>& lines) const;

  void setParamsFromSaveLines(vector<string>& lines);

  void insertActorInLoop(Actor* actor);

  void actorDidAct(const bool IS_FREE_TURN = false);

  inline int getTurn() {return turn_;}

  Actor* getCurrentActor();

  void eraseActorInElement(const unsigned int i);

  void getFeatureMobsAtPos(const Pos& pos, vector<FeatureMob*>& vectorRef);

  void addFeatureMob(FeatureMob* const feature) {
    featureMobs_.push_back(feature);
  }

  void eraseAllFeatureMobs() {
    for(FeatureMob * m : featureMobs_) {delete m;}
    featureMobs_.resize(0);
  }

  void eraseFeatureMob(FeatureMob* const feature, const bool DESTROY_OBJECT) {
    const int SIZE = featureMobs_.size();
    for(int i = 0; i < SIZE; i++) {
      if(featureMobs_.at(i) == feature) {
        if(DESTROY_OBJECT) {delete feature;}
        featureMobs_.erase(featureMobs_.begin() + i);
        return;
      }
    }
  }

  void resetTurnTypeAndActorCounters() {
    currentTurnTypePos_ = 0;
    currentActorVectorPos_ = 0;
  }

  void updateLightMap();

  vector<Actor*> actors_;
  vector<FeatureMob*> featureMobs_;

private:
  void runStandardTurnEvents();

  void runAtomicTurnEvents();

  bool isSpiRegenThisTurn(const int REGEN_N_TURNS);

  vector<ActorSpeed> turnTypeVector_;
  int currentTurnTypePos_;
  int currentActorVectorPos_;

  int turn_;

  Engine& eng;
};

#endif
