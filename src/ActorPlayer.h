#ifndef PLAYER_H
#define PLAYER_H

#include "Actor.h"

#include "ConstTypes.h"

//const int HUNGER_LEVEL_FULL             = 1400;
//const int HUNGER_LEVEL_FIRST_WARNING    = 400;
//const int HUNGER_LEVEL_FINAL_WARNING    = 200;

const int MIN_SHOCK_WHEN_OBSESSION = 25;

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

  void FOVupdate();

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

  int mythosKnowledge;

  void incrShock(const ShockValues_t shockValue, const int MODIFIER);
  void restoreShock();
  int getShockTotal() {
    return shock_ + shockTemp_;
  }
  int getShockTmp() {
    return shockTemp_;
  }
  void incrInsanity();
  int getInsanity() const {
    return insanity_;
  }
  void setTempShockFromFeatures();

  int getShockResistance() const;

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
  int insanity_, shock_, shockTemp_;

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
