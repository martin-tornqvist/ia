#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <string>

#include "AbilityValues.h"
#include "CommonData.h"
#include "Converters.h"
#include "CommonTypes.h"

using namespace std;

class Engine;

class Actor;
class Weapon;

enum PropId {
  propRPhys,
  propRFire,
  propRCold,
  propRPoison,
  propRElec,
  propRAcid,
  propRSleep,
  propRFear,
  propRConfusion,
  propLightSensitive,
  propBlind,
  propFainted,
  propBurning,
  propPoisoned,
  propParalysed,
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
  propWound,
  propAiming,
  propNailed,
  propFlared,
  propWarlockCharged,

  endOfPropIds
};

enum PropTurnMode {propTurnModeStandard, propTurnModeActor};

enum PropTurns {
  propTurnsSpecific, propTurnsIndefinite, propTurnsStd
};

enum PropMsgType {
  propMsgOnStartPlayer,
  propMsgOnStartMonster,
  propMsgOnEndPlayer,
  propMsgOnEndMonster,
  propMsgOnMorePlayer,
  propMsgOnMoreMonster,
  propMsgOnResPlayer,
  propMsgOnResMonster,
  endOfPropMsg
};

enum PropAlignment {
  propAlignmentGood, propAlignmentBad, propAlignmentNeutral
};

enum class PropSrc {applied, inv, endOfPropSrc};

struct PropData {
  PropData() :
    id(endOfPropIds),
    stdRndTurns(Range(10, 10)),
    name(""),
    nameShort(""),
    isMakingMonsterAware(false),
    allowDisplayTurns(true),
    allowApplyMoreWhileActive(true),
    updatePlayerVisualWhenStartOrEnd(false),
    isEndedByMagicHealing(false),
    allowTestingOnBot(false),
    alignment(propAlignmentBad) {
    for(int i = 0; i < endOfPropMsg; i++) {msg[i] = "";}
  }

  PropId id;
  Range stdRndTurns;
  string name;
  string nameShort;
  string msg[endOfPropMsg];
  bool isMakingMonsterAware;
  bool allowDisplayTurns;
  bool allowApplyMoreWhileActive;
  bool updatePlayerVisualWhenStartOrEnd;
  bool isEndedByMagicHealing;
  bool allowTestingOnBot;
  PropAlignment alignment;
};

class PropDataHandler {
public:
  PropDataHandler() {initDataList();}

  PropData dataList[endOfPropIds];

private:
  void initDataList();
  void addPropData(PropData& d);
};

class Prop;

//Each actor has an instance of this
class PropHandler {
public:
  PropHandler(Actor* owningActor, Engine& engine);

  ~PropHandler();

  void tryApplyProp(Prop* const prop, const bool FORCE_EFFECT = false,
                    const bool NO_MESSAGES = false,
                    const bool DISABLE_REDRAW = false,
                    const bool DISABLE_PROP_START_EFFECTS = false);

  void tryApplyPropFromWpn(const Weapon& wpn, const bool IS_MELEE);

  void changeMoveDir(const Pos& actorPos, Dir& dir) const;
  int getChangedMaxHp(const int HP_MAX) const;
  bool allowAttack(const bool ALLOW_MESSAGE_WHEN_FALSE) const;
  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const;
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const;
  bool allowSee() const;
  bool allowMove() const;
  bool allowAct() const;
  bool allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const;
  bool allowCastSpells(const bool ALLOW_MESSAGE_WHEN_FALSE) const;
  void onHit();
  void onDeath(const bool IS_PLAYER_SEE_OWNING_ACTOR);
  int getAbilityMod(const AbilityId ability) const;

  void getAllActivePropIds(vector<PropId>& idVectorRef) const;

  Prop* getProp(const PropId id, const PropSrc source) const;

  bool endAppliedProp(const PropId id,
                      const bool visionBlockers[MAP_W][MAP_H],
                      const bool RUN_PROP_END_EFFECTS = true);

  void endAppliedPropsByMagicHealing();

  bool changeActorClr(SDL_Color& clr) const;

  vector<Prop*> appliedProps_;
  vector<Prop*> actorTurnPropBuffer_;

  void applyActorTurnPropBuffer();

  void tick(const PropTurnMode turnMode,
            const bool visionBlockers[MAP_W][MAP_H]);

  void getPropsInterfaceLine(vector<StrAndClr>& line) const;

  Prop* makeProp(const PropId id, PropTurns turnsInit,
                 const int NR_TURNS = -1) const;

  bool tryResistDmg(const DmgTypes dmgType,
                    const bool ALLOW_MSG_WHEN_TRUE) const;

private:
  void getPropsFromSources(vector<Prop*>& propList,
                           bool sources[int(PropSrc::endOfPropSrc)]) const;

  bool tryResistProp(const PropId id, const vector<Prop*>& propList) const;

  Actor* owningActor_;
  Engine& eng;
};

class Prop {
public:
  Prop(PropId id, Engine& engine, PropTurns turnsInit, int turns);

  virtual ~Prop() {}

  virtual void addSaveLines(vector<string>& lines) const {
    (void)lines;
  }
  virtual void setParamsFromSaveLines(vector<string>& lines) {
    (void)lines;
  }

  PropId getId() {return id_;}

  virtual bool isFinnished() const {return turnsLeft_ == 0;}
  virtual PropAlignment getAlignment() const {return data_->alignment;}
  virtual bool allowDisplayTurns() const {return data_->allowDisplayTurns;}
  virtual bool isMakingMonsterAware() const {
    return data_->isMakingMonsterAware;
  }
  virtual string getName() const {return data_->name;}
  virtual string getNameShort() const {return data_->nameShort;}
  virtual void getMsg(const PropMsgType msgType, string& msgRef) const {
    msgRef = data_->msg[msgType];
  }
  virtual bool allowApplyMoreWhileActive() const {
    return data_->allowApplyMoreWhileActive;
  }
  virtual bool shouldUpdatePlayerVisualWhenStartOrEnd() const {
    return data_->updatePlayerVisualWhenStartOrEnd;
  }
  virtual bool isEndedByMagicHealing() const {
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
  virtual void onDeath(const bool IS_PLAYER_SEE_OWNING_ACTOR) {
    (void)IS_PLAYER_SEE_OWNING_ACTOR;
  }

  virtual int getChangedMaxHp(const int HP_MAX) const {return HP_MAX;}

  virtual bool changeActorClr(SDL_Color& clr) const {(void)clr; return false;}

  virtual bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return true;
  }
  virtual bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return true;
  }
  virtual bool allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return true;
  }
  virtual bool allowCastSpells(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return true;
  }
  virtual int getAbilityMod(const AbilityId ability) const {
    (void)ability;
    return 0;
  }
  virtual void changeMoveDir(const Pos& actorPos, Dir& dir) {
    (void)actorPos;
    (void)dir;
  }
  virtual bool tryResistOtherProp(const PropId id) const {
    (void)id;
    return false;
  }
  virtual bool tryResistDmg(
    const DmgTypes dmgType, const bool ALLOW_MSG_WHEN_TRUE) const {
    (void)dmgType;
    (void)ALLOW_MSG_WHEN_TRUE;
    return false;
  }

  virtual PropTurnMode getTurnMode() const {return propTurnModeStandard;}

  int turnsLeft_;

  Actor* owningActor_;

protected:
  const PropId id_;
  Engine& eng;
  const PropData* const data_;
};

class PropWound: public Prop {
public:
  PropWound(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propWound, engine, turnsInit, turns), nrWounds_(1) {}

  ~PropWound() override {}

  void addSaveLines(vector<string>& lines) const override {
    lines.push_back(toString(nrWounds_));
  }
  void setParamsFromSaveLines(vector<string>& lines) override {
    nrWounds_ = toInt(lines.front());
    lines.erase(lines.begin());
  }

  string getNameShort() const override {
    return "Wound(" + toString(nrWounds_) + ")";
  }

  int getAbilityMod(const AbilityId ability) const override {
    if(ability == ability_accuracyMelee)  return nrWounds_ * -10;
    if(ability == ability_accuracyRanged) return nrWounds_ * -5;
    if(ability == ability_dodgeAttack)    return nrWounds_ * -10;
    if(ability == ability_dodgeTrap)      return nrWounds_ * -10;
    return 0;
  }

  void getMsg(const PropMsgType msgType, string& msgRef) const override;

  void onMore() override;

  void healOneWound();

  inline int getNrWounds() const {return nrWounds_;}

private:
  int nrWounds_;
};

class PropTerrified: public Prop {
public:
  PropTerrified(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propTerrified, engine, turnsInit, turns) {}

  ~PropTerrified() override {}

  int getAbilityMod(const AbilityId ability) const override {
    if(ability == ability_dodgeAttack)      return 20;
    if(ability == ability_accuracyRanged)   return -20;
    return 0;
  }

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;
};

class PropWeakened: public Prop {
public:
  PropWeakened(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propWeakened, engine, turnsInit, turns) {}
  ~PropWeakened() override {}
};

class PropInfected: public Prop {
public:
  PropInfected(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propInfected, engine, turnsInit, turns) {}

  ~PropInfected() override {}

  void onNewTurn() override;
};

class PropDiseased: public Prop {
public:
  PropDiseased(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propDiseased, engine, turnsInit, turns) {}

  ~PropDiseased() override {}

  int getChangedMaxHp(const int HP_MAX) const override {
    return HP_MAX / 2;
  }

  bool tryResistOtherProp(const PropId id) const override;

  void onStart() override;
};

class PropFlying: public Prop {
public:
  PropFlying(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propFlying, engine, turnsInit, turns) {}
  ~PropFlying() override {}
};

class PropEthereal: public Prop {
public:
  PropEthereal(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propEthereal, engine, turnsInit, turns) {}
  ~PropEthereal() override {}
};

class PropOoze: public Prop {
public:
  PropOoze(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propOoze, engine, turnsInit, turns) {}
  ~PropOoze() override {}
};

class PropBurrowing: public Prop {
public:
  PropBurrowing(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propBurrowing, engine, turnsInit, turns) {}
  ~PropBurrowing() override {}

  void onNewTurn() override;
};

class PropPossessedByZuul: public Prop {
public:
  PropPossessedByZuul(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propPossessedByZuul, engine, turnsInit, turns) {}

  ~PropPossessedByZuul() override {}

  void onDeath(const bool IS_PLAYER_SEE_OWNING_ACTOR) override;

  int getChangedMaxHp(const int HP_MAX) const override {
    return HP_MAX * 2;
  }
};

class PropPoisoned: public Prop {
public:
  PropPoisoned(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propPoisoned, engine, turnsInit, turns) {}

  ~PropPoisoned() override {}

  void onNewTurn() override;
};

class PropAiming: public Prop {
public:
  PropAiming(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propAiming, engine, turnsInit, turns), nrTurnsAiming(1) {}

  ~PropAiming() override {}

  PropTurnMode getTurnMode() const override {return propTurnModeActor;}

  string getNameShort() const override {
    return data_->nameShort + (nrTurnsAiming >= 3 ? "(3)" : "");
  }

  int getAbilityMod(const AbilityId ability) const override {
    if(ability == ability_accuracyRanged) return nrTurnsAiming >= 3 ? 999 : 10;
    return 0;
  }

  bool isMaxRangedDmg() const {return nrTurnsAiming >= 3;}

  int nrTurnsAiming;
};

class PropBlind: public Prop {
public:
  PropBlind(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propBlind, engine, turnsInit, turns) {}

  ~PropBlind() override {}

  bool shouldUpdatePlayerVisualWhenStartOrEnd() const override;

  bool allowSee() const override {return false;}

  int getAbilityMod(const AbilityId ability) const override {
    if(ability == ability_searching)      return -9999;
    if(ability == ability_dodgeTrap ||
        ability == ability_dodgeAttack)   return -50;
    if(ability == ability_accuracyRanged) return -50;
    if(ability == ability_accuracyMelee)  return -25;
    return 0;
  }
};

class PropBlessed: public Prop {
public:
  PropBlessed(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propBlessed, engine, turnsInit, turns) {}

  ~PropBlessed() override {}

  void onStart() override;

  int getAbilityMod(const AbilityId ability) const override {
    if(ability == ability_searching)  return 0;
    return 10;
  }
};

class PropCursed: public Prop {
public:
  PropCursed(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propCursed, engine, turnsInit, turns) {}

  ~PropCursed() override {}

  void onStart() override;

  int getAbilityMod(const AbilityId ability) const override {
    (void)ability;
    return -10;
  }
};

class PropBurning: public Prop {
public:
  PropBurning(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propBurning, engine, turnsInit, turns) {}

  ~PropBurning() override {}

  bool allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;

  bool changeActorClr(SDL_Color& clr) const override {
    clr = clrRedLgt;
    return true;
  }

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;

  void onStart() override;
  void onNewTurn() override;
};

class PropFlared: public Prop {
public:
  PropFlared(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propFlared, engine, turnsInit, turns) {}

  ~PropFlared() override {}

  void onNewTurn() override;
};

class PropWarlockCharged: public Prop {
public:
  PropWarlockCharged(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propWarlockCharged, engine, turnsInit, turns) {}
  ~PropWarlockCharged() override {}

  PropTurnMode getTurnMode() const override {return propTurnModeActor;}
};

class PropConfused: public Prop {
public:
  PropConfused(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propConfused, engine, turnsInit, turns) {}

  ~PropConfused() override {}

  void changeMoveDir(const Pos& actorPos, Dir& dir) override;

  bool allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;
};

class PropStunned: public Prop {
public:
  PropStunned(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propStunned, engine, turnsInit, turns) {}
  ~PropStunned() override {}
};

class PropNailed: public Prop {
public:
  PropNailed(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propNailed, engine, turnsInit, turns), nrSpikes_(1) {}
  ~PropNailed() override {}

  string getNameShort() const override {
    return "Nailed(" + toString(nrSpikes_) + ")";
  }

  void changeMoveDir(const Pos& actorPos, Dir& dir) override;

  void onMore() override {nrSpikes_++;}

  bool isFinnished() const override {return nrSpikes_ <= 0;}

private:
  int nrSpikes_;
};

class PropWaiting: public Prop {
public:
  PropWaiting(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propWaiting, engine, turnsInit, turns) {}

  ~PropWaiting() override {}

  PropTurnMode getTurnMode() const override {return propTurnModeActor;}

  bool allowMove() const override  {return false;}
  bool allowAct() const override   {return false;}
  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
};

class PropDisabledAttack: public Prop {
public:
  PropDisabledAttack(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propDisabledAttack, engine, turnsInit, turns) {}

  ~PropDisabledAttack() override {}

  PropTurnMode getTurnMode() const override {return propTurnModeActor;}

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
};

class PropDisabledMelee: public Prop {
public:
  PropDisabledMelee(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propDisabledMelee, engine, turnsInit, turns) {}

  ~PropDisabledMelee() override {}

  PropTurnMode getTurnMode() const override {return propTurnModeActor;}

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
};

class PropDisabledRanged: public Prop {
public:
  PropDisabledRanged(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propDisabledRanged, engine, turnsInit, turns) {}

  ~PropDisabledRanged() override {}

  PropTurnMode getTurnMode() const override {return propTurnModeActor;}

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
};

class PropParalyzed: public Prop {
public:
  PropParalyzed(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propParalysed, engine, turnsInit, turns) {}

  ~PropParalyzed() override {}

  PropTurnMode getTurnMode() const override {return propTurnModeActor;}

  void onStart() override;

  bool allowAct() const override {return false;}

  int getAbilityMod(const AbilityId ability) const override {
    if(ability == ability_dodgeTrap || ability == ability_dodgeAttack)
      return -999;
    return 0;
  }
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
};

class PropFainted: public Prop {
public:
  PropFainted(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propFainted, engine, turnsInit, turns) {}

  ~PropFainted() override {}

  bool shouldUpdatePlayerVisualWhenStartOrEnd() const override;

  bool allowAct() const override {return false;}

  bool allowSee() const override {return false;}

  int getAbilityMod(const AbilityId ability) const override {
    if(
      ability == ability_dodgeTrap ||
      ability == ability_dodgeAttack)
      return -999;
    return 0;
  }

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const override {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const override {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }

  void onHit() override {turnsLeft_ = 0;}
};

class PropSlowed: public Prop {
public:
  PropSlowed(Engine& engine, PropTurns turnsInit,
             int turns = -1) :
    Prop(propSlowed, engine, turnsInit, turns) {}

  ~PropSlowed() override {}
};

class PropFrenzied: public Prop {
public:
  PropFrenzied(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propFrenzied, engine, turnsInit, turns) {}

  ~PropFrenzied() override {}

  void onStart() override;
  void onEnd() override;

  void changeMoveDir(const Pos& actorPos, Dir& dir) override;

  bool allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;
  bool allowCastSpells(const bool ALLOW_MESSAGE_WHEN_FALSE) const override;

  bool tryResistOtherProp(const PropId id) const override;

  int getAbilityMod(const AbilityId ability) const override {
    if(ability == ability_accuracyMelee) return 999;
    return 0;
  }
};

class PropRAcid: public Prop {
public:
  PropRAcid(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propRAcid, engine, turnsInit, turns) {}
  ~PropRAcid() override {}

  bool tryResistDmg(const DmgTypes dmgType,
                    const bool ALLOW_MSG_WHEN_TRUE) const override;
};

class PropRCold: public Prop {
public:
  PropRCold(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propRCold, engine, turnsInit, turns) {}
  ~PropRCold() override {}

  bool tryResistDmg(const DmgTypes dmgType,
                    const bool ALLOW_MSG_WHEN_TRUE) const override;
};

class PropRConfusion: public Prop {
public:
  PropRConfusion(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propRConfusion, engine, turnsInit, turns) {}
  ~PropRConfusion() override {}

  void onStart() override;

  bool tryResistOtherProp(const PropId id) const override;
};

class PropRElec: public Prop {
public:
  PropRElec(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propRElec, engine, turnsInit, turns) {}
  ~PropRElec() override {}

  bool tryResistDmg(const DmgTypes dmgType,
                    const bool ALLOW_MSG_WHEN_TRUE) const override;
};

class PropRFear: public Prop {
public:
  PropRFear(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propRFear, engine, turnsInit, turns) {}
  ~PropRFear() override {}

  void onStart() override;

  bool tryResistOtherProp(const PropId id) const override;
};

class PropRPhys: public Prop {
public:
  PropRPhys(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propRPhys, engine, turnsInit, turns) {}
  ~PropRPhys() override {}

  void onStart() override;

  bool tryResistOtherProp(const PropId id) const override;

  bool tryResistDmg(const DmgTypes dmgType,
                    const bool ALLOW_MSG_WHEN_TRUE) const override;
};

class PropRFire: public Prop {
public:
  PropRFire(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propRFire, engine, turnsInit, turns) {}
  ~PropRFire() override {}

  void onStart() override;

  bool tryResistOtherProp(const PropId id) const override;

  bool tryResistDmg(const DmgTypes dmgType,
                    const bool ALLOW_MSG_WHEN_TRUE) const override;
};

class PropRPoison: public Prop {
public:
  PropRPoison(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propRPoison, engine, turnsInit, turns) {}
  ~PropRPoison() override {}

  void onStart() override;

  bool tryResistOtherProp(const PropId id) const override;
};

class PropRSleep: public Prop {
public:
  PropRSleep(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propRSleep, engine, turnsInit, turns) {}
  ~PropRSleep() override {}

  void onStart() override;

  bool tryResistOtherProp(const PropId id) const override;
};

class PropLightSensitive: public Prop {
public:
  PropLightSensitive(Engine& engine, PropTurns turnsInit, int turns = -1) :
    Prop(propLightSensitive, engine, turnsInit, turns) {}
  ~PropLightSensitive() override {}
};

#endif
