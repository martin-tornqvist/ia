#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>

#include "Actor.h"
#include "ConstTypes.h"

//const int HUNGER_LEVEL_FULL             = 1400;
//const int HUNGER_LEVEL_FIRST_WARNING    = 400;
//const int HUNGER_LEVEL_FINAL_WARNING    = 200;

const int MIN_SHOCK_WHEN_OBSESSION = 35;

enum InsanityPhobia_t {
  insanityPhobia_rat,
  insanityPhobia_spider,
  insanityPhobia_dog,
  insanityPhobia_undead,
  insanityPhobia_openPlace,
  insanityPhobia_closedPlace,
  insanityPhobia_deepPlaces,
  endOfInsanityPhobias
};

enum InsanityObsession_t {
  insanityObsession_sadism,
  insanityObsession_masochism,
  endOfInsanityObsessions
};

class Monster;

class Player: public Actor {
public:
  Player();
  ~Player() {}

  void updateFov();

  void moveDirection(const coord& dir) {
    moveDirection(dir.x, dir.y);
  }
  void moveDirection(const int X_DIR, int Y_DIR);

  void actorSpecific_init() {}

  void actorSpecific_spawnStartItems();

  void act();

  void hearSound(const Sound& sound);

  void explosiveThrown();

  int firstAidTurnsLeft;
  int waitTurnsLeft;

  void incrShock(const ShockValues_t shockValue);
  void incrShock(const int VAL);
  void restoreShock(const bool IS_TEMP_SHOCK_RESTORED);
  int getShockTotal() {
    return static_cast<int>(floor(shock_ + shockTemp_));
  }
  int getShockTmp() {
    return shockTemp_;
  }
  void incrInsanity();
  int getInsanity() const {
    return min(100, insanity_ + mythosKnowledge);
  }
  void setTempShockFromFeatures();

  int getShockResistance() const;

  int getMythosKnowledge() const {
    return mythosKnowledge;
  }

  int getSanityPenaltyFromMythosKnowledge() const {
    return mythosKnowledge;
  }

  int dynamiteFuseTurns;
  int molotovFuseTurns;
  int flareFuseTurns;

  void addSaveLines(vector<string>& lines) const;
  void setParametersFromSaveLines(vector<string>& lines);

  bool insanityPhobias[endOfInsanityPhobias];
  bool insanityObsessions[endOfInsanityObsessions];

  const Actor* target;

  void autoMelee();

  void kick(Actor& actorToKick);

  void updateColor();

  int getHealingTimeTotal() const;

  void actorSpecific_addLight(bool light[MAP_X_CELLS][MAP_Y_CELLS]) const;

private:
  friend class DungeonMaster;
  friend class GameTime;
  int insanity_;
  double shock_, shockTemp_;

  int mythosKnowledge;

  int nrMovesUntilFreeAction;

  void attemptIdentifyItems();
  void testPhobias();
  void actorSpecific_hit(const int DMG);
  void FOVhack();
  void interruptActions(const bool PROMPT_FOR_ABORT);
  friend class MessageLog;
  bool isStandingInOpenSpace() const;
  bool isStandingInCrampedSpace() const;
};


#endif
