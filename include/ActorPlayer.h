#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>

#include "Actor.h"
#include "CommonData.h"

enum class Phobia {
  rat,
  spider,
  dog,
  undead,
  openPlace,
  closedPlace,
  deepPlaces,
  endOfPhobias
};

enum class Obsession {
  sadism,
  masochism,
  endOfObsessions
};

enum class ShockSrc {
  seeMonster,
  useStrangeItem,
  castIntrSpell,
  time,
  misc,
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

  void incrShock(const ShockValue shockValue, ShockSrc shockSrc);
  void incrShock(const int SHOCK, ShockSrc shockSrc);
  void restoreShock(const int amountRestored,
                    const bool IS_TEMP_SHOCK_RESTORED);
  inline int getShockTotal()  const {return int(floor(shock_ + shockTmp_));}
  inline int getInsanity()    const {return min(100, insanity_);}

  //The following is used for determining if '!'-marks should be drawn on the
  //player map symbol
  inline double getPermShockTakenCurTurn() const {
    return permShockTakenCurTurn_;
  }
  void resetPermShockTakenCurTurn() {permShockTakenCurTurn_ = 0.0;}

  void addTmpShockFromFeatures();

  int getShockResistance(const ShockSrc shockSrc) const;
  double getShockTakenAfterMods(const int BASE_SHOCK,
                                const ShockSrc shockSrc) const;

  void addSaveLines(vector<string>& lines) const;
  void setParamsFromSaveLines(vector<string>& lines);

  bool phobias[int(Phobia::endOfPhobias)];
  bool obsessions[int(Obsession::endOfObsessions)];

  void autoMelee();

  void kick(Actor& actorToKick);
  void punch(Actor& actorToPunch);

  void updateColor();

  void addLight_(bool light[MAP_W][MAP_H]) const;

  void interruptActions();

  int getEncPercent() const;

  MedicalBag* activeMedicalBag;
  int waitTurnsLeft;

  int dynamiteFuseTurns;
  int molotovFuseTurns;
  int flareFuseTurns;

  Actor* target;

  int insanity_;
  double shock_, shockTmp_, permShockTakenCurTurn_;

private:
  int getCarryWeightLimit() const;

  void incrInsanity();

  void testPhobias();

  void hit_(int& dmg, const bool ALLOW_WOUNDS) override;

  void FOVhack();

  bool isStandingInOpenSpace() const;

  bool isStandingInCrampedSpace() const;

  int nrMovesUntilFreeAction_;

  int nrTurnsUntilIns_;

  const int CARRY_WEIGHT_BASE_;
};


#endif
