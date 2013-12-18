#ifndef GAME_TIME_H
#define GAME_TIME_H

#include <vector>

#include "Feature.h"
#include "ActorData.h"

using namespace std;

class Engine;

enum TurnType_t {
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

  void setParametersFromSaveLines(vector<string>& lines);

  void insertActorInLoop(Actor* actor);

  void endTurnOfCurrentActor();

  int getTurn() {
    return turn_;
  }

  Actor* getCurrentActor();

  int getNrActors() const {
    return actors_.size();
  }

  int getNrFeatureMobs() {
    return featureMobs_.size();
  }

  Actor& getActorAtElement(const int i) {
    return *(actors_.at(i));
  }

  FeatureMob& getFeatureMobAtElement(const int ELEMENT) {
    return *(featureMobs_.at(ELEMENT));
  }

  void eraseActorInElement(const unsigned int i);

  vector<FeatureMob*> getFeatureMobsAtPos(const Pos& pos);

  void addFeatureMob(FeatureMob* const feature) {
    featureMobs_.push_back(feature);
  }

  void eraseAllFeatureMobs() {
    for(unsigned int i = 0; i < featureMobs_.size(); i++) {
      delete featureMobs_.at(i);
    }
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

private:
  friend class Renderer;
  friend class Dungeon;
  friend class DungeonFeatureFactory;

  void runStandardTurnEvents();

  void runAtomicTurnEvents();

  vector<ActorSpeed_t> turnTypeVector_;
  int currentTurnTypePos_;
  vector<Actor*> actors_;
  vector<FeatureMob*> featureMobs_;
  int currentActorVectorPos_;

  int turn_;

  Engine& eng;
};

#endif
