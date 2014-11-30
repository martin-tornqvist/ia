#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <string>

#include "AbilityValues.h"
#include "CmnData.h"
#include "Converters.h"
#include "CmnTypes.h"

class Actor;
class Wpn;

enum PropId
{
  propRPhys,
  propRFire,
  propRCold,
  propRPoison,
  propRElec,
  propRAcid,
  propRSleep,
  propRFear,
  propRConfusion,
  propRBreath,
  propRDisease,
  propLightSensitive,
  propBlind,
  propFainted,
  propBurning,
  propRadiant,
  propPoisoned,
  propParalyzed,
  propTerrified,
  propConfused,
  propStunned,
  propSlowed,
  propInfected,
  propDiseased,
  propWeakened,
  propFrenzied,
  propBlessed,
  propCursed,

  //Properties affecting the actors body, or way of moving around
  propFlying,
  propEthereal,
  propOoze,
  propBurrowing,

  //The following are used for AI control
  propWaiting,
  propDisabledAttack,
  propDisabledMelee,
  propDisabledRanged,

  //Special (mostly for aiding other game mechanics)
  propPossessedByZuul,
  propAiming,
  propNailed,
  propFlared,
  propWarlockCharged,

  endOfPropIds
};

enum class PropTurnMode {std, actor};
enum class PropTurns    {std, specific, indefinite};

enum PropMsgType
{
  propMsgOnStartPlayer,
  propMsgOnStartMon,
  propMsgOnEndPlayer,
  propMsgOnEndMon,
  propMsgOnMorePlayer,
  propMsgOnMoreMon,
  propMsgOnResPlayer,
  propMsgOnResMon,
  endOfPropMsg
};

enum PropAlignment
{
  propAlignmentGood, propAlignmentBad, propAlignmentNeutral
};

enum class PropSrc {applied, inv, END};

struct PropDataT
{
  PropDataT() :
    id(endOfPropIds),
    stdRndTurns(Range(10, 10)),
    name(""),
    nameShort(""),
    isMakingMonAware(false),
    allowDisplayTurns(true),
    allowApplyMoreWhileActive(true),
    updatePlayerVisualWhenStartOrEnd(false),
    isEndedByMagicHealing(false),
    allowTestOnBot(false),
    alignment(propAlignmentBad)
  {
    for (int i = 0; i < endOfPropMsg; ++i) {msg[i] = "";}
  }

  PropId        id;
  Range         stdRndTurns;
  std::string   name;
  std::string   nameShort;
  std::string   msg[endOfPropMsg];
  bool          isMakingMonAware;
  bool          allowDisplayTurns;
  bool          allowApplyMoreWhileActive;
  bool          updatePlayerVisualWhenStartOrEnd;
  bool          isEndedByMagicHealing;
  bool          allowTestOnBot;
  PropAlignment alignment;
};

namespace PropData
{

extern PropDataT data[endOfPropIds];

void init();

} //PropData

class Prop;

//Each actor has an instance of this
class PropHandler
{
public:
  PropHandler(Actor* owningActor);

  ~PropHandler();

  void tryApplyProp(Prop* const prop, const bool FORCE_EFFECT = false,
                    const bool NO_MESSAGES = false,
                    const bool DISABLE_REDRAW = false,
                    const bool DISABLE_PROP_START_EFFECTS = false);

  void tryApplyPropFromWpn(const Wpn& wpn, const bool IS_MELEE);

  void changeMoveDir(const Pos& actorPos, Dir& dir) const;
  int getChangedMaxHp(const int HP_MAX) const;
  bool allowAttack      (const bool ALLOW_MESSAGE_WHEN_FALSE) const;
  bool allowAttackMelee (const bool ALLOW_MESSAGE_WHEN_FALSE) const;
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const;
  bool allowSee() const;
  bool allowMove() const;
  bool allowAct() const;
  bool allowRead        (const bool ALLOW_MESSAGE_WHEN_FALSE) const;
  bool allowCastSpells  (const bool ALLOW_MESSAGE_WHEN_FALSE) const;
  void onHit();
  void onDeath(const bool IS_PLAYER_SEE_OWNING_ACTOR);
  int getAbilityMod(const AbilityId ability) const;

  void getPropIds(bool out[endOfPropIds]) const;

  Prop* getProp(const PropId id, const PropSrc source) const;

  bool endAppliedProp(const PropId id,
                      const bool blockedLos[MAP_W][MAP_H],
                      const bool RUN_PROP_END_EFFECTS = true);

  void endAppliedPropsByMagicHealing();

  bool changeActorClr(Clr& clr) const;

  std::vector<Prop*> appliedProps_;
  std::vector<Prop*> actorTurnPropBuffer_;

  void applyActorTurnPropBuffer();

  void tick(const PropTurnMode turnMode,
            const bool blockedLos[MAP_W][MAP_H]);

  void getPropsInterfaceLine(std::vector<StrAndClr>& line) const;

  Prop* mkProp(const PropId id, PropTurns turnsInit,
               const int NR_TURNS = -1) const;

  bool tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG_WHEN_TRUE) const;

private:
  //Note: These two functions are responsible for collecting properties from all possible
  //specified sources. They must be "mirrored" in that they collect information from
  //exactly the same places - except one will return a list of Props, and the other will
  //return a list of PropIds. This is an optimization. The reasoning is that in many
  //cases (such as the very frequent Actor::addLight) it is sufficient to know the ids.
  //It would then be wasteful to first gather all Props, then gather the ids from those.
  //It's more efficient to just gather the ids at once.
  void getPropsFromSources(std::vector<Prop*>& out,
                           bool sources[int(PropSrc::END)]) const;
  void getPropIdsFromSources(bool out[endOfPropIds],
                             bool sources[int(PropSrc::END)]) const;

  bool tryResistProp(const PropId id, const std::vector<Prop*>& propList) const;

  Actor* owningActor_;
};

class Prop
{
public:
  Prop(PropId id, PropTurns turnsInit, int turns);

  virtual ~Prop() {}

  virtual void storeToSaveLines(std::vector<std::string>& lines) const
  {
    (void)lines;
  }
  virtual void setupFromSaveLines(std::vector<std::string>& lines)
  {
    (void)lines;
  }

  PropId getId() const {return id_;}

  virtual bool isFinnished() const {return turnsLeft_ == 0;}
  virtual PropAlignment getAlignment() const {return data_->alignment;}
  virtual bool allowDisplayTurns() const {return data_->allowDisplayTurns;}
  virtual bool isMakingMonAware() const
  {
    return data_->isMakingMonAware;
  }
  virtual std::string getName() const {return data_->name;}
  virtual std::string getNameShort() const {return data_->nameShort;}
  virtual void getMsg(const PropMsgType msgType, std::string& msgRef) const
  {
    msgRef = data_->msg[msgType];
  }
  virtual bool allowApplyMoreWhileActive() const
  {
    return data_->allowApplyMoreWhileActive;
  }
  virtual bool shouldUpdatePlayerVisualWhenStartOrEnd() const
  {
    return data_->updatePlayerVisualWhenStartOrEnd;
  }
  virtual bool isEndedByMagicHealing() const
  {
    return data_->isEndedByMagicHealing;
  }
  virtual bool allowSee()   const {return true;}
  virtual bool allowMove()  const {return true;}
  virtual bool allowAct()   const {return true;}
  virtual void onHit()            {}
  virtual void onNewTurn()        {}
  virtual void onStart()          {}
  virtual void onEnd()            {}
  virtual void onMore()           {}
  virtual void onDeath(const bool IS_PLAYER_SEE_OWNING_ACTOR)
  {
    (void)IS_PLAYER_SEE_OWNING_ACTOR;
  }

  virtual int getChangedMaxHp(const int HP_MAX) const {return HP_MAX;}

  virtual bool changeActorClr(Clr& clr) const {(void)clr; return false;}

  virtual bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return true;
  }
  virtual bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return true;
  }
  virtual bool allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return true;
  }
  virtual bool allowCastSpells(const bool ALLOW_MESSAGE_WHEN_FALSE) const
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return true;
  }
  virtual int getAbilityMod(const AbilityId ability) const
  {
    (void)ability;
    return 0;
  }
  virtual void changeMoveDir(const Pos& actorPos, Dir& dir)
  {
    (void)actorPos;
    (void)dir;
  }
  virtual bool isResistOtherProp(const PropId id) const
  {
    (void)id;
    return false;
  }
  virtual bool tryResistDmg(
    const DmgType dmgType, const bool ALLOW_MSG_WHEN_TRUE) const
  {
    (void)dmgType;
    (void)ALLOW_MSG_WHEN_TRUE;
    return false;
  }

  virtual PropTurnMode getTurnMode() const {return PropTurnMode::std;}

  PropTurns getTurnsInitType() const {return turnsInitType_;}

  int turnsLeft_;

  Actor* owningActor_;

protected:
  const PropId id_;

  const PropDataT* const data_;

  //How the prop turns was inited (std, specific, indefinite). This is used for example
  //to make copies of a property to apply on melee attacks.
  PropTurns turnsInitType_;
};

class PropTerrified: public Prop
{
public:
  PropTerrified(PropTurns turnsInit, int turns = -1) :
    Prop(propTerrified, turnsInit, turns) {}

  int getAbilityMod(const AbilityId ability) const override
  {
    if (ability == AbilityId::dodgeAttack)      return 20;
    if (ability == AbilityId::ranged)   return -20;
    return 0;
  }

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;
};

class PropWeakened: public Prop
{
public:
  PropWeakened(PropTurns turnsInit, int turns = -1) :
    Prop(propWeakened, turnsInit, turns) {}
};

class PropInfected: public Prop
{
public:
  PropInfected(PropTurns turnsInit, int turns = -1) :
    Prop(propInfected, turnsInit, turns) {}

  void onNewTurn() override;
};

class PropDiseased: public Prop
{
public:
  PropDiseased(PropTurns turnsInit, int turns = -1) :
    Prop(propDiseased, turnsInit, turns) {}

  int getChangedMaxHp(const int HP_MAX) const override;

  bool isResistOtherProp(const PropId id) const override;

  void onStart() override;
};

class PropFlying: public Prop
{
public:
  PropFlying(PropTurns turnsInit, int turns = -1) :
    Prop(propFlying, turnsInit, turns) {}
};

class PropEthereal: public Prop
{
public:
  PropEthereal(PropTurns turnsInit, int turns = -1) :
    Prop(propEthereal, turnsInit, turns) {}
};

class PropOoze: public Prop
{
public:
  PropOoze(PropTurns turnsInit, int turns = -1) :
    Prop(propOoze, turnsInit, turns) {}
};

class PropBurrowing: public Prop
{
public:
  PropBurrowing(PropTurns turnsInit, int turns = -1) :
    Prop(propBurrowing, turnsInit, turns) {}

  void onNewTurn() override;
};

class PropPossessedByZuul: public Prop
{
public:
  PropPossessedByZuul(PropTurns turnsInit, int turns = -1) :
    Prop(propPossessedByZuul, turnsInit, turns) {}

  void onDeath(const bool IS_PLAYER_SEE_OWNING_ACTOR) override;

  int getChangedMaxHp(const int HP_MAX) const override
  {
    return HP_MAX * 2;
  }
};

class PropPoisoned: public Prop
{
public:
  PropPoisoned(PropTurns turnsInit, int turns = -1) :
    Prop(propPoisoned, turnsInit, turns) {}

  void onNewTurn() override;
};

class PropAiming: public Prop
{
public:
  PropAiming(PropTurns turnsInit, int turns = -1) :
    Prop(propAiming, turnsInit, turns), nrTurnsAiming(1) {}

  PropTurnMode getTurnMode() const override {return PropTurnMode::actor;}

  std::string getNameShort() const override
  {
    return data_->nameShort + (nrTurnsAiming >= 3 ? "(3)" : "");
  }

  int getAbilityMod(const AbilityId ability) const override
  {
    if (ability == AbilityId::ranged) return nrTurnsAiming >= 3 ? 999 : 10;
    return 0;
  }

  bool isMaxRangedDmg() const {return nrTurnsAiming >= 3;}

  int nrTurnsAiming;
};

class PropBlind: public Prop
{
public:
  PropBlind(PropTurns turnsInit, int turns = -1) :
    Prop(propBlind, turnsInit, turns) {}

  bool shouldUpdatePlayerVisualWhenStartOrEnd() const override;

  bool allowSee() const override {return false;}

  int getAbilityMod(const AbilityId ability) const override
  {
    if (ability == AbilityId::searching) {return -9999;}
    if (ability == AbilityId::ranged)    {return -50;}
    if (ability == AbilityId::melee)     {return -25;}
    if (ability == AbilityId::dodgeTrap || ability == AbilityId::dodgeAttack)
    {
      return -50;
    }
    return 0;
  }
};

class PropRadiant: public Prop
{
public:
  PropRadiant(PropTurns turnsInit, int turns = -1) :
    Prop(propRadiant, turnsInit, turns) {}
};

class PropBlessed: public Prop
{
public:
  PropBlessed(PropTurns turnsInit, int turns = -1) :
    Prop(propBlessed, turnsInit, turns) {}

  void onStart() override;

  int getAbilityMod(const AbilityId ability) const override
  {
    if (ability == AbilityId::searching)  return 0;
    return 10;
  }
};

class PropCursed: public Prop
{
public:
  PropCursed(PropTurns turnsInit, int turns = -1) :
    Prop(propCursed, turnsInit, turns) {}

  void onStart() override;

  int getAbilityMod(const AbilityId ability) const override
  {
    (void)ability;
    return -10;
  }
};

class PropBurning: public Prop
{
public:
  PropBurning(PropTurns turnsInit, int turns = -1) :
    Prop(propBurning, turnsInit, turns) {}

  bool allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;

  bool changeActorClr(Clr& clr) const override
  {
    clr = clrRedLgt;
    return true;
  }

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;

  void onStart() override;
  void onNewTurn() override;
};

class PropFlared: public Prop
{
public:
  PropFlared(PropTurns turnsInit, int turns = -1) :
    Prop(propFlared, turnsInit, turns) {}

  void onNewTurn() override;
};

class PropWarlockCharged: public Prop
{
public:
  PropWarlockCharged(PropTurns turnsInit, int turns = -1) :
    Prop(propWarlockCharged, turnsInit, turns) {}

  PropTurnMode getTurnMode() const override {return PropTurnMode::actor;}
};

class PropConfused: public Prop
{
public:
  PropConfused(PropTurns turnsInit, int turns = -1) :
    Prop(propConfused, turnsInit, turns) {}

  void changeMoveDir(const Pos& actorPos, Dir& dir) override;

  bool allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;
};

class PropStunned: public Prop
{
public:
  PropStunned(PropTurns turnsInit, int turns = -1) :
    Prop(propStunned, turnsInit, turns) {}
};

class PropNailed: public Prop
{
public:
  PropNailed(PropTurns turnsInit, int turns = -1) :
    Prop(propNailed, turnsInit, turns), nrSpikes_(1) {}

  std::string getNameShort() const override
  {
    return "Nailed(" + toStr(nrSpikes_) + ")";
  }

  void changeMoveDir(const Pos& actorPos, Dir& dir) override;

  void onMore() override {nrSpikes_++;}

  bool isFinnished() const override {return nrSpikes_ <= 0;}

private:
  int nrSpikes_;
};

class PropWaiting: public Prop
{
public:
  PropWaiting(PropTurns turnsInit, int turns = -1) :
    Prop(propWaiting, turnsInit, turns) {}

  PropTurnMode getTurnMode() const override {return PropTurnMode::actor;}

  bool allowMove() const override  {return false;}
  bool allowAct() const override   {return false;}
  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
};

class PropDisabledAttack: public Prop
{
public:
  PropDisabledAttack(PropTurns turnsInit, int turns = -1) :
    Prop(propDisabledAttack, turnsInit, turns) {}

  PropTurnMode getTurnMode() const override {return PropTurnMode::actor;}

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
};

class PropDisabledMelee: public Prop
{
public:
  PropDisabledMelee(PropTurns turnsInit, int turns = -1) :
    Prop(propDisabledMelee, turnsInit, turns) {}

  PropTurnMode getTurnMode() const override {return PropTurnMode::actor;}

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
};

class PropDisabledRanged: public Prop
{
public:
  PropDisabledRanged(PropTurns turnsInit, int turns = -1) :
    Prop(propDisabledRanged, turnsInit, turns) {}

  PropTurnMode getTurnMode() const override {return PropTurnMode::actor;}

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
};

class PropParalyzed: public Prop
{
public:
  PropParalyzed(PropTurns turnsInit, int turns = -1) :
    Prop(propParalyzed, turnsInit, turns) {}

  PropTurnMode getTurnMode() const override {return PropTurnMode::actor;}

  void onStart() override;

  bool allowAct() const override {return false;}

  int getAbilityMod(const AbilityId ability) const override
  {
    if (ability == AbilityId::dodgeTrap || ability == AbilityId::dodgeAttack)
      return -999;
    return 0;
  }
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
};

class PropFainted: public Prop
{
public:
  PropFainted(PropTurns turnsInit, int turns = -1) :
    Prop(propFainted, turnsInit, turns) {}

  bool shouldUpdatePlayerVisualWhenStartOrEnd() const override;

  bool allowAct() const override {return false;}

  bool allowSee() const override {return false;}

  int getAbilityMod(const AbilityId ability) const override
  {
    if (ability == AbilityId::dodgeTrap ||
        ability == AbilityId::dodgeAttack)
    {
      return -999;
    }
    return 0;
  }

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override
  {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }

  void onHit() override {turnsLeft_ = 0;}
};

class PropSlowed: public Prop
{
public:
  PropSlowed(PropTurns turnsInit,
             int turns = -1) :
    Prop(propSlowed, turnsInit, turns) {}

  int getAbilityMod(const AbilityId ability) const override
  {
    if (ability == AbilityId::dodgeAttack)    return -30;
    if (ability == AbilityId::ranged) return -10;
    if (ability == AbilityId::melee)  return -10;
    return 0;
  }
};

class PropFrenzied: public Prop
{
public:
  PropFrenzied(PropTurns turnsInit, int turns = -1) :
    Prop(propFrenzied, turnsInit, turns) {}

  void onStart() override;
  void onEnd() override;

  void changeMoveDir(const Pos& actorPos, Dir& dir) override;

  bool allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;
  bool allowCastSpells(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;

  bool isResistOtherProp(const PropId id) const override;

  int getAbilityMod(const AbilityId ability) const override
  {
    if (ability == AbilityId::melee) return 999;
    return 0;
  }
};

class PropRAcid: public Prop
{
public:
  PropRAcid(PropTurns turnsInit, int turns = -1) :
    Prop(propRAcid, turnsInit, turns) {}

  bool tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG_WHEN_TRUE) const override;
};

class PropRCold: public Prop
{
public:
  PropRCold(PropTurns turnsInit, int turns = -1) :
    Prop(propRCold, turnsInit, turns) {}

  bool tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG_WHEN_TRUE) const override;
};

class PropRConfusion: public Prop
{
public:
  PropRConfusion(PropTurns turnsInit, int turns = -1) :
    Prop(propRConfusion, turnsInit, turns) {}

  void onStart() override;

  bool isResistOtherProp(const PropId id) const override;
};

class PropRElec: public Prop
{
public:
  PropRElec(PropTurns turnsInit, int turns = -1) :
    Prop(propRElec, turnsInit, turns) {}

  bool tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG_WHEN_TRUE) const override;
};

class PropRFear: public Prop
{
public:
  PropRFear(PropTurns turnsInit, int turns = -1) :
    Prop(propRFear, turnsInit, turns) {}

  void onStart() override;

  bool isResistOtherProp(const PropId id) const override;
};

class PropRPhys: public Prop
{
public:
  PropRPhys(PropTurns turnsInit, int turns = -1) :
    Prop(propRPhys, turnsInit, turns) {}

  void onStart() override;

  bool isResistOtherProp(const PropId id) const override;

  bool tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG_WHEN_TRUE) const override;
};

class PropRFire: public Prop
{
public:
  PropRFire(PropTurns turnsInit, int turns = -1) :
    Prop(propRFire, turnsInit, turns) {}

  void onStart() override;

  bool isResistOtherProp(const PropId id) const override;

  bool tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG_WHEN_TRUE) const override;
};

class PropRPoison: public Prop
{
public:
  PropRPoison(PropTurns turnsInit, int turns = -1) :
    Prop(propRPoison, turnsInit, turns) {}

  void onStart() override;

  bool isResistOtherProp(const PropId id) const override;
};

class PropRSleep: public Prop
{
public:
  PropRSleep(PropTurns turnsInit, int turns = -1) :
    Prop(propRSleep, turnsInit, turns) {}

  void onStart() override;

  bool isResistOtherProp(const PropId id) const override;
};

class PropRDisease: public Prop
{
public:
  PropRDisease(PropTurns turnsInit, int turns = -1) :
    Prop(propRDisease, turnsInit, turns) {}

  void onStart() override;

  bool isResistOtherProp(const PropId id) const override;
};

class PropRBreath: public Prop
{
public:
  PropRBreath(PropTurns turnsInit, int turns = -1) :
    Prop(propRBreath, turnsInit, turns) {}
};

class PropLightSensitive: public Prop
{
public:
  PropLightSensitive(PropTurns turnsInit, int turns = -1) :
    Prop(propLightSensitive, turnsInit, turns) {}
  ~PropLightSensitive() override {}
};

#endif
