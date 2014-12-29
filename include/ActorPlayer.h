#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>

#include "Actor.h"
#include "CmnData.h"

enum class Phobia
{
  rat,
  spider,
  dog,
  undead,
  openPlace,
  crampedPlace,
  deepPlaces,
  dark,
  END
};

enum class Obsession
{
  sadism,
  masochism,
  END
};

enum class ShockSrc
{
  seeMon,
  useStrangeItem,
  castIntrSpell,
  time,
  misc,
  END
};

class Mon;
class MedicalBag;
class Explosive;

class Player: public Actor
{
public:
  Player();
  ~Player();

  void updateFov();

  void moveDir(Dir dir);

  void mkStartItems() override;
  void onActorTurn()  override;
  void onStdTurn()    override;

  void hearSound(const Snd& snd, const bool IS_ORIGIN_SEEN_BY_PLAYER,
                 const Dir dirToOrigin,
                 const int PERCENT_AUDIBLE_DISTANCE);

  void incrShock(const ShockLvl shockValue, ShockSrc shockSrc);
  void incrShock(const int SHOCK, ShockSrc shockSrc);
  void restoreShock(const int amountRestored,
                    const bool IS_TEMP_SHOCK_RESTORED);
  int getShockTotal()  const {return int(floor(shock_ + shockTmp_));}
  int getInsanity()    const {return std::min(100, ins_);}

  //The following is used for determining if '!'-marks should be drawn on the
  //player map symbol
  double getPermShockTakenCurTurn() const
  {
    return permShockTakenCurTurn_;
  }
  void resetPermShockTakenCurTurn() {permShockTakenCurTurn_ = 0.0;}

  int getShockResistance(const ShockSrc shockSrc) const;
  double getShockTakenAfterMods(const int BASE_SHOCK,
                                const ShockSrc shockSrc) const;

  void storeToSaveLines(std::vector<std::string>& lines) const;
  void setupFromSaveLines(std::vector<std::string>& lines);

  void autoMelee();

  void kickMon(Actor& actorToKick);
  void punchMon(Actor& actorToPunch);

  void updateClr();

  void addLight_(bool lightMap[MAP_W][MAP_H]) const;

  void onLogMsgPrinted();   //Aborts e.g. searching and quick move
  void interruptActions();  //Aborts e.g. healing

  int getEncPercent() const;

  int getCarryWeightLmt() const;

  void setQuickMove(const Dir dir);

  bool phobias    [int(Phobia::END)];
  bool obsessions [int(Obsession::END)];

  bool isLeaderOf(const Actor* const actor)       const override;
  bool isActorMyLeader(const Actor* const actor)  const override;

  MedicalBag* activeMedicalBag;
  int         waitTurnsLeft;
  Explosive*  activeExplosive;
  Actor*      tgt_;
  int         ins_;
  double      shock_, shockTmp_, permShockTakenCurTurn_;

private:
  void incrInsanity();

  void testPhobias();

  void hit_(int& dmg) override;

  void FOVhack();

  bool isStandingInOpenSpace() const;

  bool isStandingInCrampedSpace() const;

  int       nrMovesUntilFreeAction_;
  int       nrTurnsUntilIns_;

  int       nrQuickMoveStepsLeft_;
  Dir       quickMoveDir_;

  const int CARRY_WEIGHT_BASE_;
};


#endif
