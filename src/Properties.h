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
class TimedEntity;
class Weapon;

//enum StatusEffects_t {
//  propWound, propBlind, statusBurning, propFlared, statusParalyzed,
//  statusTerrified, propConfused, statusWaiting, propSlowed ,
//  statusInfected, statusDiseased, statusPoisoned, statusFainted,
//  propWeakened, statusPerfectReflexes, statusPerfectAim,
//  propCursed, propBlessed, statusClairvoyant, statusNailed,
//
//  //Status for the steady aimer ability
//  statusStill,
//
//  //The following are mostly used for AI controll
//  statusDisabledAttack, statusDisabledMelee, statusDisabledRanged
//};

enum PropId_t {
  propRFire,
  propRCold,
  propRPoison,
  propRElectric,
  propRAcid,
  propRSleep,
  propFreeAction,
  propRFear,
  propRConfusion,
  propLightSensitive,
  propBlind,
  propFainted,
  propBurning,
  propFrozen,
  propPoisoned,
  propParalysed,
  propTerrified,
  propConfused,
  propStunned,
  propSlowed,
  propFlared,
  propNailed,
  propInfected,
  propDiseased,
  propWeakened,
  propJuggernaut,
//  propHeroic,
  propClairvoyant,
  propBlessed,
  propCursed,
  propStill,
  propWound,

  //The following are used for AI control
  propWaiting,
  propDisabledAttack,
  propDisabledMelee,
  propDisabledRanged,

  endOfPropIds
};

enum PropTurns_t {
  propTurnsSpecified,
  propTurnsIndefinite,
  propTurnsStandard
};

enum PropMsg_t {
  propMsgOnStartPlayer,
  propMsgOnStartMonster,
  propMsgOnEndPlayer,
  propMsgOnEndMonster,
  propMsgOnMorePlayer,
  propMsgOnMoreMonster,
  propMsgOnResistPlayer,
  propMsgOnResistMonster,
  endOfPropMsg
};

enum PropAlignment_t {
  propAlignmentGood,
  propAlignmentBad,
  propAlignmentNeutral
};

enum PropSrc_t {
  propSrcApplied,
  propSrcInv,
  propSrcAppliedAndInv
};

struct PropData {
  PropId_t id;
  Range stdRndTurns;
  string name;
  string nameShort;
  string msg[endOfPropMsg];
  bool isMakingMonsterAware;
  bool allowDisplayTurns;
  bool allowApplyMoreWhileActive;
  bool updatePlayerVisualWhenStartOrEnd;
  PropAlignment_t alignment;
};

class PropDataHandler {
public:
  PropDataHandler(Engine* engine) : eng(engine) {initDataList();}

  PropData dataList[endOfPropIds];

private:
  void initDataList();
  void addPropData(PropData& d);
  Engine* eng;
};

class Prop;

// Each actor has an instance of this
class PropHandler {
public:
  PropHandler(Actor* owningActor, Engine* engine);

  ~PropHandler();

  void tryApplyProp(Prop* const prop, const bool FORCE_EFFECT = false,
                    const bool NO_MESSAGES = false,
                    const bool DISABLE_REDRAW = false);

  void tryApplyPropFromWpn(const Weapon& wpn, const bool IS_MELEE);

  void changeMovePos(const Pos& actorPos, Pos& movePos);
  bool allowAttack(const bool ALLOW_MESSAGE_WHEN_FALSE);
  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE);
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE);
  bool allowSee();
  bool allowMove();
  bool allowAct();
  void onHit();
  int getAbilityMod(const Abilities_t ability);

  bool hasProp(const PropId_t id) const;

  Prop* getAppliedProp(const PropId_t id) const;

//  bool hasAnyBadEffect() const {
//    for(unsigned int i = 0; i < effects.size(); i++) {
//      if(effects.at(i)->isConsideredBeneficial() == false) {
//        return true;
//      }
//    }
//    return false;
//  }

  void endAppliedProp(const PropId_t id,
                      const bool visionBlockingArray[MAP_X_CELLS][MAP_Y_CELLS],
                      const bool RUN_PROP_END_EFFECTS = true);

  bool changeActorClr(SDL_Color& clr);

  vector<Prop*> appliedProps_;

  void newTurnAllProps(
    const bool visionBlockingArray[MAP_X_CELLS][MAP_Y_CELLS]);

private:
  friend class Player;
  friend class ExplosionMaker;
  Prop* makePropFromId(const PropId_t id, PropTurns_t turnsInit,
                       const int NR_TURNS = -1);

  void getPropsFromSource(vector<Prop*>& propList,
                          const PropSrc_t source) const;

  bool hasProp(const PropId_t id,
               const vector<Prop*> propList) const;

  Actor* owningActor_;
  Engine* eng;
};

class Prop {
public:
  Prop(PropId_t id, Engine* engine, PropTurns_t turnsInit, int turns);

  virtual ~Prop() {}

  PropId_t getId() {return id_;}

  virtual bool isFinnished() {
    return turnsLeft_ == 0;
  }
  virtual PropAlignment_t getAlignment() {
    return data_->alignment;
  }
  virtual bool allowDisplayTurns() {
    return data_->allowDisplayTurns;
  }
  virtual bool isMakingMonsterAware() {
    return data_->isMakingMonsterAware;
  }
  virtual string getName() {
    return data_->name;
  }
  virtual string getNameShort() {
    return data_->nameShort;
  }
  virtual void getMsg(const PropMsg_t msgType, string& msgRef) {
    msgRef = data_->msg[msgType];
  }
  virtual bool allowApplyMoreWhileActive() {
    return data_->allowApplyMoreWhileActive;
  }
  virtual bool updatePlayerVisualWhenStartOrEnd() {
    return data_->updatePlayerVisualWhenStartOrEnd;
  }
  virtual bool allowSee()   {return true;}
  virtual bool allowMove()  {return true;}
  virtual bool allowAct()   {return true;}
  virtual void onHit()      {}
  virtual void onNewTurn()  {}
  virtual void onStart()    {}
  virtual void onEnd()      {}
  virtual void onMore()     {}

  virtual bool changeActorClr(SDL_Color& clr) {
    (void)clr;
    return false;
  }

  virtual bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return true;
  }
  virtual bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return true;
  }
  virtual int getAbilityMod(const Abilities_t ability) {
    (void)ability;
    return 0;
  }
  virtual void changeMovePos(const Pos& actorPos, Pos& movePos) {
    (void)actorPos;
    (void)movePos;
  }
  virtual bool tryResistOherProp(const PropId_t otherId) {
    (void)otherId;
    return false;
  }

  int turnsLeft_;

  Actor* owningActor_;

//  void setTurnsFromRandomStandard();

protected:
//  virtual DiceParam getRandomStandardNrTurns() = 0;

  const PropId_t id_;
  Engine* const eng;
  const PropData* const data_;
};

class PropWound: public Prop {
public:
  PropWound(Engine* engine, PropTurns_t turnsInit,
            int turns = -1) :
    Prop(propWound, engine, turnsInit, turns), nrWounds_(1) {}

  ~PropWound() {}

  string getNameShort() {return "Wound(" + intToString(nrWounds_) + ")";}

  int getAbilityMod(const Abilities_t ability) {
    if(ability == ability_accuracyMelee)  return nrWounds_ * -10;
    if(ability == ability_accuracyRanged) return nrWounds_ * -10;
    if(ability == ability_dodgeAttack)    return nrWounds_ * -10;
    if(ability == ability_dodgeTrap)      return nrWounds_ * -10;
    return 0;
  }

  void onMore();

  void healOneWound();

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 1, 0);}
  int nrWounds_;
};

class PropTerrified: public Prop {
public:
  PropTerrified(Engine* engine, PropTurns_t turnsInit,
                int turns = -1) :
    Prop(propTerrified, engine, turnsInit, turns) {}

  ~PropTerrified() {}

  int getAbilityMod(const Abilities_t ability) {
    if(ability == ability_dodgeAttack)      return 20;
    if(ability == ability_accuracyRanged)   return -20;
    return 0;
  }

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE);

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE);

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 8, 4);}
};

class PropWeakened: public Prop {
public:
  PropWeakened(Engine* engine, PropTurns_t turnsInit,
               int turns = -1) :
    Prop(propWeakened, engine, turnsInit, turns) {}

  ~PropWeakened() {}

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 100, 100);}
};

class PropInfected: public Prop {
public:
  PropInfected(Engine* engine, PropTurns_t turnsInit,
               int turns = -1) :
    Prop(propInfected, engine, turnsInit, turns) {}

  ~PropInfected() {}

  void onNewTurn();

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 20, 100);}
};

class PropDiseased: public Prop {
public:
  PropDiseased(Engine* engine, PropTurns_t turnsInit,
               int turns = -1) :
    Prop(propDiseased, engine, turnsInit, turns) {}

  ~PropDiseased() {}

  void onStart();

//  void onNewTurn();

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 100, 1450);}
};

class PropPoisoned: public Prop {
public:
  PropPoisoned(Engine* engine, PropTurns_t turnsInit,
               int turns = -1) :
    Prop(propPoisoned, engine, turnsInit, turns) {}

  ~PropPoisoned() {}

  void onNewTurn();

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 25, 50);}
};

class PropStill: public Prop {
public:
  PropStill(Engine* engine, PropTurns_t turnsInit,
            int turns = -1) :
    Prop(propStill, engine, turnsInit, turns) {}

  ~PropStill() {}

  int getAbilityMod(const Abilities_t ability) {
    if(ability == ability_accuracyRanged) return 10;
    return 0;
  }

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(0, 0, 1);}
};

class PropBlind: public Prop {
public:
  PropBlind(Engine* engine, PropTurns_t turnsInit,
            int turns = -1) :
    Prop(propBlind, engine, turnsInit, turns) {}

  ~PropBlind() {}

  bool updatePlayerVisualWhenStartOrEnd();

  void onStart();

  bool allowSee() {return false;}

  int getAbilityMod(const Abilities_t ability) {
    if(ability == ability_searching)      return -9999;
    if(ability == ability_dodgeTrap ||
        ability == ability_dodgeAttack)   return -50;
    if(ability == ability_accuracyRanged) return -50;
    if(ability == ability_accuracyMelee)  return -25;
    return 0;
  }

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 8, 8);}
};

class PropBlessed: public Prop {
public:
  PropBlessed(Engine* engine, PropTurns_t turnsInit,
              int turns = -1) :
    Prop(propBlessed, engine, turnsInit, turns) {}

  ~PropBlessed() {}

  void onStart();

  int getAbilityMod(const Abilities_t ability) {
    if(ability == ability_searching)  return 0;
    return 10;
  }

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 100, 450);}
};

class PropCursed: public Prop {
public:
  PropCursed(Engine* engine, PropTurns_t turnsInit,
             int turns = -1) :
    Prop(propCursed, engine, turnsInit, turns) {}

  ~PropCursed() {}

  void onStart();

  int getAbilityMod(const Abilities_t ability) {
    (void)ability;
    return -10;
  }

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 500, 500);}
};

class PropClairvoyant: public Prop {
public:
  PropClairvoyant(Engine* engine, PropTurns_t turnsInit,
                  int turns = -1) :
    Prop(propClairvoyant, engine, turnsInit, turns) {}

  ~PropClairvoyant() {}

  bool updatePlayerVisualWhenStartOrEnd();

  void onStart();

//  void onNewTurn();

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 100, 450);}
};

class PropBurning: public Prop {
public:
  PropBurning(Engine* engine, PropTurns_t turnsInit,
              int turns = -1) :
    Prop(propBurning, engine, turnsInit, turns) {}

  ~PropBurning() {}

  bool changeActorClr(SDL_Color& clr) {
    clr = clrRedLgt;
    return true;
  }

  void onStart();
  void onNewTurn();

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 6, 3);}
};

class PropFlared: public Prop {
public:
  PropFlared(Engine* engine, PropTurns_t turnsInit,
             int turns = -1) :
    Prop(propFlared, engine, turnsInit, turns) {}

  ~PropFlared() {}

  void onNewTurn();

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 2, 2);}
};

class PropConfused: public Prop {
public:
  PropConfused(Engine* engine, PropTurns_t turnsInit,
               int turns = -1) :
    Prop(propConfused, engine, turnsInit, turns) {}

  ~PropConfused() {}

  void changeMovePos(const Pos& actorPos, Pos& movePos);

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE);
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE);

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(3, 6, 30);}
};

class PropNailed: public Prop {
public:
  PropNailed(Engine* engine, PropTurns_t turnsInit,
             int turns = -1) :
    Prop(propNailed, engine, turnsInit, turns), nrSpikes_(1) {}
  ~PropNailed() {}

  string getNameShort() {return "Nailed(" + intToString(nrSpikes_) + ")";}

  void changeMovePos(const Pos& actorPos, Pos& movePos);

  void more() {nrSpikes_++;}

  bool isFinnished() {return nrSpikes_ <= 0;}

private:
  int nrSpikes_;
//  DiceParam getRandomStandardNrTurns() {return DiceParam(0, 0, 999);}
};


class PropWaiting: public Prop {
public:
  PropWaiting(Engine* engine, PropTurns_t turnsInit,
              int turns = -1) :
    Prop(propWaiting, engine, turnsInit, turns) {}

  ~PropWaiting() {}

  bool allowMove()  {return false;}
  bool allowAct()   {return false;}
  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(0, 0, 1);}
};

class PropDisabledAttack: public Prop {
public:
  PropDisabledAttack(Engine* engine, PropTurns_t turnsInit,
                     int turns = -1) :
    Prop(propDisabledAttack, engine, turnsInit, turns) {}

  ~PropDisabledAttack() {}

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackMleee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(0, 0, 1);}
};

class PropDisabledMelee: public Prop {
public:
  PropDisabledMelee(Engine* engine, PropTurns_t turnsInit,
                    int turns = -1) :
    Prop(propDisabledMelee, engine, turnsInit, turns) {}

  ~PropDisabledMelee() {}

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(0, 0, 1);}
};

class PropDisabledRanged: public Prop {
public:
  PropDisabledRanged(Engine* engine, PropTurns_t turnsInit,
                     int turns = -1) :
    Prop(propDisabledRanged, engine, turnsInit, turns) {}

  ~PropDisabledRanged() {}

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(0, 0, 1);}
};

class PropParalyzed: public Prop {
public:
  PropParalyzed(Engine* engine, PropTurns_t turnsInit,
                int turns = -1) :
    Prop(propParalysed, engine, turnsInit, turns) {}

  ~PropParalyzed() {}

  void onStart();

  bool allowAct() {return false;}

  int getAbilityMod(const Abilities_t ability) {
    if(ability == ability_dodgeTrap || ability == ability_dodgeAttack)
      return -999;
    return 0;
  }
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackMleee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 3, 6);}
};

class PropFrozen: public Prop {
public:
  PropFrozen(Engine* engine, PropTurns_t turnsInit,
             int turns = -1) :
    Prop(propFrozen, engine, turnsInit, turns) {}
  ~PropFrozen() {}
private:
};

class PropFreeAction: public Prop {
public:
  PropFreeAction(Engine* engine, PropTurns_t turnsInit,
                 int turns = -1) :
    Prop(propFreeAction, engine, turnsInit, turns) {}
  ~PropFreeAction() {}

  bool tryResistOherProp(const PropId_t otherId);

private:
};

class PropFainted: public Prop {
public:
  PropFainted(Engine* engine, PropTurns_t turnsInit,
              int turns = -1) :
    Prop(propFainted, engine, turnsInit, turns) {}

  ~PropFainted() {}

  bool updatePlayerVisualWhenStartOrEnd();

  void onStart();

  bool allowAct() {return false;}

  bool allowSee() {return false;}

  int getAbilityMod(const Abilities_t ability) {
    if(
      ability == ability_dodgeTrap ||
      ability == ability_dodgeAttack)
      return -999;
    return 0;
  }

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackMleee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }

  void onHit() {turnsLeft_ = 0;}

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 50, 50);}
};

class PropSlowed: public Prop {
public:
  PropSlowed(Engine* engine, PropTurns_t turnsInit,
             int turns = -1) :
    Prop(propSlowed, engine, turnsInit, turns) {}

  ~PropSlowed() {}

private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(3, 6, 6);}
};

class PropJuggernaut: public Prop {
public:
  PropJuggernaut(Engine* engine, PropTurns_t turnsInit,
                 int turns = -1) :
    Prop(propJuggernaut, engine, turnsInit, turns) {}

  ~PropJuggernaut() {}

  int getAbilityMod(const Abilities_t ability) {
    if(ability == ability_accuracyMelee)
      return 999;
    if(ability == ability_accuracyRanged)
      return 999;
    if(ability == ability_dodgeTrap)
      return 999;
    if(ability == ability_dodgeAttack)
      return 999;
    return 0;
  }
private:
//  DiceParam getRandomStandardNrTurns() {return DiceParam(3, 8, 24);}
};

class PropRAcid: public Prop {
public:
  PropRAcid(Engine* engine, PropTurns_t turnsInit,
            int turns = -1) :
    Prop(propRAcid, engine, turnsInit, turns) {}
  ~PropRAcid() {}

  bool tryResistOherProp(const PropId_t otherId);

private:
};

class PropRCold: public Prop {
public:
  PropRCold(Engine* engine, PropTurns_t turnsInit,
            int turns = -1) :
    Prop(propRCold, engine, turnsInit, turns) {}
  ~PropRCold() {}

  bool tryResistOherProp(const PropId_t otherId);

private:
};

class PropRConfusion: public Prop {
public:
  PropRConfusion(Engine* engine, PropTurns_t turnsInit,
                 int turns = -1) :
    Prop(propRConfusion, engine, turnsInit, turns) {}
  ~PropRConfusion() {}

  bool tryResistOherProp(const PropId_t otherId);

private:
};

class PropStunned: public Prop {
public:
  PropStunned(Engine* engine, PropTurns_t turnsInit,
              int turns = -1) :
    Prop(propStunned, engine, turnsInit, turns) {}
  ~PropStunned() {}
private:
};

class PropRElectric: public Prop {
public:
  PropRElectric(Engine* engine, PropTurns_t turnsInit,
                int turns = -1) :
    Prop(propRElectric, engine, turnsInit, turns) {}
  ~PropRElectric() {}

  bool tryResistOherProp(const PropId_t otherId);

private:
};

class PropRFear: public Prop {
public:
  PropRFear(Engine* engine, PropTurns_t turnsInit,
            int turns = -1) :
    Prop(propRFear, engine, turnsInit, turns) {}
  ~PropRFear() {}

  bool tryResistOherProp(const PropId_t otherId);

private:
};

class PropRFire: public Prop {
public:
  PropRFire(Engine* engine, PropTurns_t turnsInit,
            int turns = -1) :
    Prop(propRFire, engine, turnsInit, turns) {}
  ~PropRFire() {}

  bool tryResistOherProp(const PropId_t otherId);

private:
};

class PropRPoison: public Prop {
public:
  PropRPoison(Engine* engine, PropTurns_t turnsInit,
              int turns = -1) :
    Prop(propRPoison, engine, turnsInit, turns) {}
  ~PropRPoison() {}

  bool tryResistOherProp(const PropId_t otherId);

private:
};

class PropRSleep: public Prop {
public:
  PropRSleep(Engine* engine, PropTurns_t turnsInit,
             int turns = -1) :
    Prop(propRSleep, engine, turnsInit, turns) {}
  ~PropRSleep() {}

  bool tryResistOherProp(const PropId_t otherId);

private:
};

class PropLightSensitive: public Prop {
public:
  PropLightSensitive(Engine* engine, PropTurns_t turnsInit,
                     int turns = -1) :
    Prop(propLightSensitive, engine, turnsInit, turns) {}
  ~PropLightSensitive() {}
private:
};

#endif
