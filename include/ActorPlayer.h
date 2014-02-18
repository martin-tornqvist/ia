#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>

#include "Actor.h"
#include "CommonData.h"

const int MIN_SHOCK_WHEN_OBSESSION = 35;

enum InsanityPhobiaId {
  insanityPhobia_rat,
  insanityPhobia_spider,
  insanityPhobia_dog,
  insanityPhobia_undead,
  insanityPhobia_openPlace,
  insanityPhobia_closedPlace,
  insanityPhobia_deepPlaces,
  endOfInsanityPhobias
};

enum InsanityObsessionId {
  insanityObsession_sadism,
  insanityObsession_masochism,
  endOfInsanityObsessions
};

enum ShockSrc {
  shockSrc_seeMonster,
  shockSrc_useStrangeItem,
  shockSrc_castIntrSpell,
  shockSrc_time,
  shockSrc_misc,
  endOfShockSrc
};

class Monster;
class MedicalBag;

class Player: public Actor {
public:
  Player(Engine& eng);
  virtual ~Player() {}

  void updateFov();

  void moveDir(Dir dir);

  void spawnStartItems()  override;
  void onActorTurn()      override;
  void onStandardTurn()   override;

  void hearSound(const Snd& snd, const bool IS_ORIGIN_SEEN_BY_PLAYER,
                 const Dir dirToOrigin,
                 const int PERCENT_AUDIBLE_DISTANCE);

  void explosiveThrown();

  MedicalBag* activeMedicalBag;
  int waitTurnsLeft;

  void incrShock(const ShockValue shockValue, ShockSrc shockSrc);
  void incrShock(const int SHOCK, ShockSrc shockSrc);
  void restoreShock(const int amountRestored,
                    const bool IS_TEMP_SHOCK_RESTORED);
  inline int getShockTotal()  const {return int(floor(shock_ + shockTemp_));}
  inline int getInsanity()    const {return min(100, insanity_);}

  //The following is used for determining if '!'-marks should be drawn on the
  //player map symbol
  inline double getPermShockTakenCurTurn() const {
    return permShockTakenCurTurn_;
  }
  void resetPermShockTakenCurTurn() {
    permShockTakenCurTurn_ = 0.0;
  }

  void setTempShockFromFeatures();

  int getShockResistance(const ShockSrc shockSrc) const;
  double getShockTakenAfterMods(const int BASE_SHOCK,
                                const ShockSrc shockSrc) const;

  int getCarryWeightLimit() const;

  int dynamiteFuseTurns;
  int molotovFuseTurns;
  int flareFuseTurns;

  void addSaveLines(vector<string>& lines) const;
  void setParamsFromSaveLines(vector<string>& lines);

  bool insanityPhobias[endOfInsanityPhobias];
  bool insanityObsessions[endOfInsanityObsessions];

  Actor* target;

  void autoMelee();

  void kick(Actor& actorToKick);
  void punch(Actor& actorToPunch);

  void updateColor();

  void addLight_(bool light[MAP_W][MAP_H]) const;

private:
  friend class DungeonMaster;
  friend class GameTime;
  friend class Log;

  void incrInsanity();
  void testPhobias();
  void hit_(int& dmg, const bool ALLOW_WOUNDS) override;
  void FOVhack();
  void interruptActions();
  bool isStandingInOpenSpace() const;
  bool isStandingInCrampedSpace() const;

  int insanity_;
  double shock_, shockTemp_, permShockTakenCurTurn_;

  int nrMovesUntilFreeAction_;

  const int carryWeightBase;
};


#endif
