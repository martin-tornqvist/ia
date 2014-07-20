#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>

#include "Actor.h"
#include "CmnData.h"

enum class Phobia {
  rat,
  spider,
  dog,
  undead,
  openPlace,
  closedPlace,
  deepPlaces,
  END
};

enum class Obsession {
  sadism,
  masochism,
  END
};

enum class ShockSrc {
  seeMonster,
  useStrangeItem,
  castIntrSpell,
  time,
  misc,
  END
};

class Monster;
class MedicalBag;

class Player: public Actor {
public:
  Player();
  virtual ~Player() {}

  void updateFov();

  void moveDir(Dir dir);

  void mkStartItems()  override;
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
  inline int getInsanity()    const {return std::min(100, insanity_);}

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

  void storeToSaveLines(std::vector<std::string>& lines) const;
  void setupFromSaveLines(std::vector<std::string>& lines);

  bool phobias[int(Phobia::END)];
  bool obsessions[int(Obsession::END)];

  void autoMelee();

  void kickMonster(Actor& actorToKick);
  void punchMonster(Actor& actorToPunch);

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
