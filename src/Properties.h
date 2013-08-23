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
//  statusWound, statusBlind, statusBurning, statusFlared, statusParalyzed,
//  statusTerrified, statusConfused, statusWaiting, statusSlowed,
//  statusInfected, statusDiseased, statusPoisoned, statusFainted,
//  statusWeak, statusPerfectReflexes, statusPerfectAim,
//  statusCursed, statusBlessed, statusClairvoyant, statusNailed,
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
  propRParalysis,
  propRFear,
  propRConfusion,
  propLightSensitive,
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
  propHeroic,
  propClairvoyant,
  propBlessed,
  propCursed,
  propStill,

  //The following are mostly used for AI controll
  propDisabledAttack, propDisabledMelee, propDisabledRanged

  endOfPropIds
};

class StatusEffect {
public:
  StatusEffect(const int TURNS, const StatusEffects_t effectId) :
    turnsLeft(TURNS), owningActor(NULL), effectId_(effectId) {}

  StatusEffect(const StatusEffects_t effectId) : effectId_(effectId) {}

  StatusEffect(const StatusEffect& other) : turnsLeft(other.turnsLeft) {}

  virtual ~StatusEffect();

  StatusEffects_t getEffectId() {return effectId_;}

  virtual bool isFinnished() {return turnsLeft <= 0;}

  int turnsLeft;

  virtual StatusEffect* copy() = 0;

  //Questions for the status to answer. This is how the status ends,
  //how it damages the actor, how it affects ability values, etc.
  virtual bool isConsideredBeneficial() = 0;
  virtual bool allowDisplayTurnsInInterface() = 0;
  virtual bool isMakingOwnerAwareOfPlayer() = 0;
  virtual string getInterfaceName() = 0;
  virtual string msgOnStartPlayer() = 0;
  virtual string msgOnStartMonster() = 0;
  virtual string msgOnEndPlayer() = 0;
  virtual string msgOnEndMonster() = 0;
  virtual string msgOnSavePlayer() = 0;
  virtual string msgOnSaveMonster() = 0;
  virtual string msgOnMorePlayer() = 0;
  virtual string msgOnMoreMonster() = 0;
  virtual void newTurn(Engine* const engine) = 0;
  virtual void start(Engine* const engine) = 0;
  virtual void end(Engine* const engine) = 0;

  virtual bool canBeAppliedWhileSameEffectOngoing() {return true;}

  virtual bool isPlayerVisualUpdateNeededWhenStartOrEnd() {return false;}

  virtual void more() {}

  virtual bool allowSee() {return true;}
  virtual bool allowMove() {return true;}
  virtual bool allowAct() {return true;}
  virtual bool isSlowed() {return false;}
  virtual void isHit() {}

  virtual SDL_Color getColorOwningActor() {return clrBlack;}

  virtual bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return true;
  }
  virtual bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return true;
  }

  virtual int getAbilityModifier(const Abilities_t ability) {
    (void)ability;
    return 0;
  }

  virtual Pos changeMovePos(const Pos& actorPos, const Pos& movePos,
                            Engine* const engine) {
    (void)actorPos;
    (void)engine;
    return movePos;
  }

  virtual bool isEthereal() {return false;}

  Actor* owningActor;

protected:
  virtual DiceParam getRandomStandardNrTurns() = 0;
  void setTurnsFromRandomStandard(Engine* const engine);

  StatusEffects_t effectId_;
};

class StatusWound: public StatusEffect {
public:
  StatusWound(Engine* const engine) :
    StatusEffect(statusWound), nrWounds(1) {(void)engine;}

  StatusWound(const int TURNS) :
    StatusEffect(TURNS, statusWound), nrWounds(TURNS) {}

  ~StatusWound() {}

  StatusWound* copy() {
    StatusWound* cpy = new StatusWound(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return true;}
  bool isConsideredBeneficial()       {return false;}
  bool allowDisplayTurnsInInterface() {return false;}

  string getInterfaceName()       {return "Wound(" + intToString(nrWounds) + ")";}
  string msgOnStartPlayer()       {return "I am wounded!";}
  string msgOnStartMonster()  {return "";}
  string msgOnMorePlayer()        {return "I am more wounded!";}
  string msgOnMoreMonster()   {return "";}
  string msgOnEndPlayer()         {return "My wound is healed.";}
  string msgOnSavePlayer()       {return "I resist wounding!";}
  string msgOnSaveMonster()  {return "";}
  string msgOnEndMonster()    {return "";}

  int getAbilityModifier(const Abilities_t ability) {
    if(ability == ability_accuracyMelee)  return nrWounds * -10;
    if(ability == ability_accuracyRanged) return nrWounds * -10;
    if(ability == ability_dodgeAttack)    return nrWounds * -10;
    if(ability == ability_dodgeTrap)      return nrWounds * -10;
    return 0;
  }

  void more();

  void start(Engine* const engine) {(void)engine;}
  void end(Engine* const engine) {(void)engine;}

  void newTurn(Engine* const engine) {(void)engine;}

  void healOneWound(Engine* const engine);

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 1, 0);}
  int nrWounds;
};

class StatusTerrified: public StatusEffect {
public:
  StatusTerrified(Engine* const engine) : StatusEffect(statusTerrified) {
    setTurnsFromRandomStandard(engine);
  }
  StatusTerrified(const int TURNS) : StatusEffect(TURNS, statusTerrified) {}
  ~StatusTerrified() {}

  StatusTerrified* copy() {
    StatusTerrified* cpy = new StatusTerrified(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return true;}
  bool isConsideredBeneficial()       {return false;}
  bool allowDisplayTurnsInInterface() {return true;}

  string getInterfaceName()       {return "Terrified";}
  string msgOnStartPlayer()       {return "I am terrified!";}
  string msgOnStartMonster()  {return "looks terrified.";}
  string msgOnMorePlayer()        {return "I am more terrified.";}
  string msgOnMoreMonster()   {return "looks more terrified.";}
  string msgOnEndPlayer()         {return "I am no longer terrified!";}
  string msgOnSavePlayer()       {return "I resist fear.";}
  string msgOnSaveMonster()  {return "resists fear.";}
  string msgOnEndMonster()    {return "is no longer terrified.";}

  int getAbilityModifier(const Abilities_t ability) {
    if(ability == ability_dodgeAttack)      return 20;
    if(ability == ability_accuracyRanged)   return -20;
    return 0;
  }

  void start(Engine* const engine) {(void)engine;}
  void end(Engine* const engine) {(void)engine;}

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE);

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE);

  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 8, 4);}
};

class StatusWeak: public StatusEffect {
public:
  StatusWeak(Engine* const engine) : StatusEffect(statusWeak) {
    setTurnsFromRandomStandard(engine);
  }
  StatusWeak(const int TURNS) : StatusEffect(TURNS, statusWeak) {}
  ~StatusWeak() {}

  StatusWeak* copy() {
    StatusWeak* cpy = new StatusWeak(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return true;}
  bool isConsideredBeneficial()       {return false;}
  bool allowDisplayTurnsInInterface() {return true;}

  string getInterfaceName()       {return "Weak";}
  string msgOnStartPlayer()       {return "I feel weaker!";}
  string msgOnStartMonster()  {return "looks weaker.";}
  string msgOnMorePlayer()        {return "I feel weaker.";}
  string msgOnMoreMonster()   {return "looks weaker!";}
  string msgOnEndPlayer()         {return "I feel stronger!";}
  string msgOnSavePlayer()       {return "I resist weakness.";}
  string msgOnSaveMonster()  {return "resists weakness.";}
  string msgOnEndMonster()    {return "looks stronger.";}

  void start(Engine* const engine) {(void)engine;}
  void end(Engine* const engine) {(void)engine;}

  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 100, 100);}
};

class StatusInfected: public StatusEffect {
public:
  StatusInfected(Engine* const engine) : StatusEffect(statusInfected) {
    setTurnsFromRandomStandard(engine);
  }
  ~StatusInfected() {}

  StatusInfected* copy() {
    StatusInfected* cpy = new StatusInfected(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return false;}
  bool isConsideredBeneficial()       {return false;}
  bool allowDisplayTurnsInInterface() {return true;}

  string getInterfaceName()       {return "Infected";}
  string msgOnStartPlayer()       {return "I am infected!";}
  string msgOnStartMonster()  {return "is infected.";}
  string msgOnMorePlayer()        {return "I am more infected.";}
  string msgOnMoreMonster()   {return "is more infected.";}
  string msgOnEndPlayer()         {return "My infection is cured!";}
  string msgOnSavePlayer()       {return "I resist infection.";}
  string msgOnSaveMonster()  {return "resists infection.";}
  string msgOnEndMonster()    {return "is no longer infected.";}

  void start(Engine* const engine)  {(void)engine;}
  void end(Engine* const engine)    {(void)engine;}

  void newTurn(Engine* const engine);

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 20, 100);}
  friend class StatusHandler;
  StatusInfected(const int TURNS) : StatusEffect(TURNS, statusDiseased) {}
};

class StatusDiseased: public StatusEffect {
public:
  StatusDiseased(Engine* const engine) : StatusEffect(statusDiseased) {
    setTurnsFromRandomStandard(engine);
  }
  ~StatusDiseased() {}

  StatusDiseased* copy() {
    StatusDiseased* cpy = new StatusDiseased(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return false;}
  bool isConsideredBeneficial()       {return false;}
  bool allowDisplayTurnsInInterface() {return true;}

  string getInterfaceName()       {return "Diseased";}
  string msgOnStartPlayer()       {return "I am diseased!";}
  string msgOnStartMonster()  {return "is diseased.";}
  string msgOnMorePlayer()        {return "I am more diseased.";}
  string msgOnMoreMonster()   {return "is more diseased.";}
  string msgOnEndPlayer()         {return "My disease is cured!";}
  string msgOnSavePlayer()       {return "I resist disease.";}
  string msgOnSaveMonster()  {return "resists disease.";}
  string msgOnEndMonster()    {return "is no longer diseased.";}

  void start(Engine* const engine);

  void end(Engine* const engine) {(void)engine;}

  void newTurn(Engine* const engine);

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 100, 1450);}
  friend class StatusHandler;
  StatusDiseased(const int TURNS) : StatusEffect(TURNS, statusDiseased) {}
};

class StatusPoisoned: public StatusEffect {
public:
  StatusPoisoned(Engine* const engine) : StatusEffect(statusPoisoned) {setTurnsFromRandomStandard(engine);}
  ~StatusPoisoned() {}

  StatusPoisoned* copy() {
    StatusPoisoned* cpy = new StatusPoisoned(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return true;}
  bool isConsideredBeneficial()       {return false;}
  bool allowDisplayTurnsInInterface() {return true;}

  string getInterfaceName()       {return "Poisoned";}
  string msgOnStartPlayer()       {return "I am poisoned!";}
  string msgOnStartMonster()  {return "is poisoned.";}
  string msgOnMorePlayer()        {return "I am more poisoned.";}
  string msgOnMoreMonster()   {return "is more poisoned.";}
  string msgOnEndPlayer()         {return "My body is cleansed from poisoning!";}
  string msgOnSavePlayer()       {return "I resist poisoning.";}
  string msgOnSaveMonster()  {return "resists poisoning.";}
  string msgOnEndMonster()    {return "is cleansed from poisoning.";}

  void start(Engine* const engine) {(void)engine;}
  void end(Engine* const engine) {(void)engine;}

  void newTurn(Engine* const engine);

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 25, 50);}
  friend class StatusHandler;
  StatusPoisoned(const int TURNS) : StatusEffect(TURNS, statusPoisoned) {}
};

class StatusStill: public StatusEffect {
public:
  StatusStill(Engine* const engine) : StatusEffect(statusStill) {setTurnsFromRandomStandard(engine);}
  StatusStill(const int TURNS) : StatusEffect(TURNS, statusStill) {}
  ~StatusStill() {}

  StatusStill* copy() {
    StatusStill* cpy = new StatusStill(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return false;}
  bool isConsideredBeneficial()       {return true;}
  bool allowDisplayTurnsInInterface() {return false;}

  string getInterfaceName()       {return "Still";}
  string msgOnStartPlayer()       {return "";}
  string msgOnMorePlayer()        {return "";}
  string msgOnMoreMonster()   {return "";}
  string msgOnEndPlayer()         {return "";}
  string msgOnSavePlayer()       {return "";}
  string msgOnStartMonster()  {return "";}
  string msgOnEndMonster()    {return "";}
  string msgOnSaveMonster()  {return "";}

  void start(Engine* const engine) {(void)engine;}
  void end(Engine* const engine) {(void)engine;}

  int getAbilityModifier(const Abilities_t ability) {
    if(ability == ability_accuracyRanged) return 10;
    return 0;
  }

  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(0, 0, 1);}
};

class StatusBlind: public StatusEffect {
public:
  StatusBlind(Engine* const engine) : StatusEffect(statusBlind) {setTurnsFromRandomStandard(engine);}
  StatusBlind(const int TURNS) : StatusEffect(TURNS, statusBlind) {}
  ~StatusBlind() {}

  StatusBlind* copy() {
    StatusBlind* cpy = new StatusBlind(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return true;}
  bool isConsideredBeneficial()       {return false;}
  bool allowDisplayTurnsInInterface() {return true;}

  bool isPlayerVisualUpdateNeededWhenStartOrEnd();

  string getInterfaceName()       {return "Blind";}
  string msgOnStartPlayer()       {return "I am blind!";}
  string msgOnMorePlayer()        {return "I am more blind.";}
  string msgOnMoreMonster()   {return "is more blind.";}
  string msgOnEndPlayer()         {return "I can see again!";}
  string msgOnSavePlayer()       {return "I resist blindness.";}
  string msgOnStartMonster()  {return "is blinded.";}
  string msgOnEndMonster()    {return "can see again.";}
  string msgOnSaveMonster()  {return "resists blindness.";}

  void start(Engine* const engine);
  void end(Engine* const engine);

  bool allowSee() {return false;}

  int getAbilityModifier(const Abilities_t ability) {
    if(ability == ability_searching)      return -9999;
    if(ability == ability_dodgeTrap ||
        ability == ability_dodgeAttack)   return -50;
    if(ability == ability_accuracyRanged) return -50;
    if(ability == ability_accuracyMelee)  return -25;
    return 0;
  }

  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 8, 8);}
};

class StatusBlessed: public StatusEffect {
public:
  StatusBlessed(Engine* const engine) : StatusEffect(statusBlessed) {setTurnsFromRandomStandard(engine);}
  StatusBlessed(const int TURNS) : StatusEffect(TURNS, statusBlessed) {}
  ~StatusBlessed() {}

  StatusBlessed* copy() {
    StatusBlessed* cpy = new StatusBlessed(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return false;}
  bool isConsideredBeneficial()       {return true;}
  bool allowDisplayTurnsInInterface() {return true;}

  string getInterfaceName()       {return "Blessed";}
  string msgOnStartPlayer()       {return "I feel luckier.";}
  string msgOnMorePlayer()        {return "I feel luckier.";}
  string msgOnMoreMonster()   {return "is luckier.";}
  string msgOnEndPlayer()         {return "My good luck ends.";}
  string msgOnSavePlayer()       {return "";}
  string msgOnStartMonster()  {return "is more lucky.";}
  string msgOnEndMonster()    {return "has normal luck.";}
  string msgOnSaveMonster()  {return "";}

  void start(Engine* const engine);
  void end(Engine* const engine) {(void)engine;}

  int getAbilityModifier(const Abilities_t ability) {
    if(ability == ability_searching)  return 0;
    return 10;
  }

  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 100, 450);}
};

class StatusCursed: public StatusEffect {
public:
  StatusCursed(Engine* const engine) : StatusEffect(statusCursed) {
    setTurnsFromRandomStandard(engine);
  }
  StatusCursed(const int TURNS) : StatusEffect(TURNS, statusCursed) {}
  ~StatusCursed() {}

  StatusCursed* copy() {
    StatusCursed* cpy = new StatusCursed(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return false;}
  bool isConsideredBeneficial()       {return false;}
  bool allowDisplayTurnsInInterface() {return true;}

  string getInterfaceName()       {return "Cursed";}
  string msgOnStartPlayer()       {return "I feel misfortunate.";}
  string msgOnMorePlayer()        {return "I feel more misfortunate.";}
  string msgOnMoreMonster()   {return "is misfortunate.";}
  string msgOnEndPlayer()         {return "My misfortune ends.";}
  string msgOnSavePlayer()       {return "";}
  string msgOnStartMonster()  {return "is more misfortunate.";}
  string msgOnEndMonster()    {return "has normal fortune.";}
  string msgOnSaveMonster()  {return "";}

  void start(Engine* const engine);

  void end(Engine* const engine) {(void)engine;}

  int getAbilityModifier(const Abilities_t ability) {
    (void)ability;
    return -10;
  }

  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 500, 500);}
};

class StatusClairvoyant: public StatusEffect {
public:
  StatusClairvoyant(Engine* const engine) : StatusEffect(statusClairvoyant) {setTurnsFromRandomStandard(engine);}
  StatusClairvoyant(const int TURNS) : StatusEffect(TURNS, statusClairvoyant) {}
  ~StatusClairvoyant() {}

  StatusClairvoyant* copy() {
    StatusClairvoyant* cpy = new StatusClairvoyant(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return true;}
  bool isConsideredBeneficial()       {return true;}
  bool allowDisplayTurnsInInterface() {return true;}

  bool isPlayerVisualUpdateNeededWhenStartOrEnd();

  string getInterfaceName()       {return "Clairvoyant";}
  string msgOnStartPlayer()       {return "I see far and beyond!";}
  string msgOnMorePlayer()        {return "I see far and beyond!";}
  string msgOnMoreMonster()   {return "";}
  string msgOnEndPlayer()         {return "My sight is limited.";}
  string msgOnSavePlayer()       {return "";}
  string msgOnStartMonster()  {return "";}
  string msgOnEndMonster()    {return "";}
  string msgOnSaveMonster()  {return "";}

  void start(Engine* const engine);
  void end(Engine* const engine);

  int getAbilityModifier(const Abilities_t ability) {
    (void)ability;
    return 0;
  }

  void newTurn(Engine* const engine);

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 100, 450);}
};

class StatusBurning: public StatusEffect {
public:
  StatusBurning(Engine* const engine) : StatusEffect(statusBurning) {setTurnsFromRandomStandard(engine);}
  ~StatusBurning() {}

  StatusBurning* copy() {
    StatusBurning* cpy = new StatusBurning(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return true;}
  bool isConsideredBeneficial()       {return false;}
  bool allowDisplayTurnsInInterface() {return true;}

  bool isPlayerVisualUpdateNeededWhenStartOrEnd();

  string getInterfaceName()       {return "Burning";}
  string msgOnStartPlayer()       {return "I am Burning!";}
  string msgOnMorePlayer()        {return "I am further engulfed by flames!";}
  string msgOnMoreMonster()   {return "is burning more.";}
  string msgOnEndPlayer()         {return "The flames are put out.";}
  string msgOnSavePlayer()       {return "I resist burning.";}
  string msgOnStartMonster()  {return "is burning.";}
  string msgOnEndMonster()    {return "is no longer burning.";}
  string msgOnSaveMonster()  {return "resists burning.";}

  SDL_Color getColorOwningActor() {return clrRedLgt;}

  void start(Engine* const engine);
  void end(Engine* const engine);
  void newTurn(Engine* const engine);

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 6, 3);}
  friend class StatusHandler;
  StatusBurning(const int TURNS) : StatusEffect(TURNS, statusBurning) {}
  void doDamage(Engine* const engine);
};

class StatusFlared: public StatusEffect {
public:
  StatusFlared(Engine* const engine) : StatusEffect(statusFlared) {setTurnsFromRandomStandard(engine);}
  ~StatusFlared() {}

  StatusFlared* copy() {
    StatusFlared* cpy = new StatusFlared(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return true;}
  bool isConsideredBeneficial()       {return false;}
  bool allowDisplayTurnsInInterface() {return false;}

  string getInterfaceName()       {return "";}
  string msgOnStartPlayer()       {return "";}
  string msgOnMorePlayer()        {return ".";}
  string msgOnMoreMonster()   {return "is perforated by another flare.";}
  string msgOnEndPlayer()         {return "";}
  string msgOnSavePlayer()       {return "";}
  string msgOnStartMonster()  {return "is perforated by a flare.";}
  string msgOnEndMonster()    {return "recovers from a flare.";}
  string msgOnSaveMonster()  {return "resists a flare.";}

  void start(Engine* const engine);
  void end(Engine* const engine);
  void newTurn(Engine* const engine);

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(1, 2, 2);}
  friend class StatusHandler;
  StatusFlared(const int TURNS) : StatusEffect(TURNS, statusFlared) {}
};

class StatusConfused: public StatusEffect {
public:
  StatusConfused(Engine* const engine) : StatusEffect(statusConfused) {setTurnsFromRandomStandard(engine);}
  StatusConfused(const int TURNS) : StatusEffect(TURNS, statusConfused) {}
  ~StatusConfused() {}

  StatusConfused* copy() {
    StatusConfused* cpy = new StatusConfused(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return true;}
  bool isConsideredBeneficial()       {return false;}
  bool allowDisplayTurnsInInterface() {return true;}

  string getInterfaceName()       {return "Confused";}
  string msgOnStartPlayer()       {return "I am confused!";}
  string msgOnStartMonster()  {return "is confused.";}
  string msgOnMorePlayer()        {return "I am more confused.";}
  string msgOnMoreMonster()   {return "is more confused.";}
  string msgOnEndPlayer()         {return "I am no longer confused";}
  string msgOnEndMonster()    {return "is no longer confused.";}
  string msgOnSavePlayer()       {return "I resist confusion.";}
  string msgOnSaveMonster()  {return "resists confusion.";}

  Pos changeMovePos(const Pos& actorPos, const Pos& movePos, Engine* const engine);

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE);
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE);

  void start(Engine* const engine) {(void)engine;}
  void end(Engine* const engine) {(void)engine;}

  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }

private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(3, 6, 30);}
};

class StatusNailed: public StatusEffect {
public:
  StatusNailed(Engine* const engine) :
    StatusEffect(statusNailed), nrOfSpikes(1) {
    setTurnsFromRandomStandard(engine);
  }
  StatusNailed(const int TURNS) :
    StatusEffect(TURNS, statusNailed), nrOfSpikes(1) {}
  ~StatusNailed() {}

  StatusNailed* copy() {
    StatusNailed* cpy = new StatusNailed(turnsLeft);
    cpy->nrOfSpikes = nrOfSpikes;
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer()   {return true;}
  bool isConsideredBeneficial()       {return false;}
  bool allowDisplayTurnsInInterface() {return false;}

  string getInterfaceName()       {return "Nailed(" + intToString(nrOfSpikes) + ")";}
  string msgOnStartPlayer()       {return "I am fastened by a spike!";}
  string msgOnStartMonster()  {return "is fastened by a spike.";}
  string msgOnMorePlayer()        {return "I am fastened by another spike.";}
  string msgOnMoreMonster()   {return "is fastened by another spike.";}
  string msgOnEndPlayer()         {return "I tear free!";}
  string msgOnEndMonster()    {return "tears free!";}
  string msgOnSavePlayer()       {return "";}
  string msgOnSaveMonster()  {return "";}

  Pos changeMovePos(const Pos& actorPos , const Pos& movePos,
                    Engine* const engine);

  void more() {nrOfSpikes++;}

  void start(Engine* const engine) {(void)engine;}
  void end(Engine* const engine) {(void)engine;}
  void newTurn(Engine* const engine) {(void)engine;}

  bool isFinnished() {return nrOfSpikes <= 0;}

private:
  int nrOfSpikes;
  DiceParam getRandomStandardNrTurns() {return DiceParam(0, 0, 999);}
};


class StatusWaiting: public StatusEffect {
public:
  StatusWaiting(Engine* const engine) :
    StatusEffect(statusWaiting) {
    setTurnsFromRandomStandard(engine);
  }
  StatusWaiting(const int TURNS) :
    StatusEffect(TURNS, statusWaiting) {
  }
  ~StatusWaiting() {
  }

  StatusWaiting* copy() {
    StatusWaiting* cpy = new StatusWaiting(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer() {
    return false;
  }

  bool allowDisplayTurnsInInterface() {
    return false;
  }

  bool isConsideredBeneficial() {
    return false;
  }

  string getInterfaceName() {
    return "";
  }
  string msgOnStartPlayer() {
    return "";
  }
  string msgOnMorePlayer() {
    return "";
  }
  string msgOnMoreMonster() {
    return "";
  }
  string msgOnEndPlayer() {
    return "";
  }
  string msgOnSavePlayer() {
    return "";
  }
  string msgOnStartMonster() {
    return "";
  }
  string msgOnEndMonster() {
    return "";
  }
  string msgOnSaveMonster() {
    return "";
  }
  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }
  void start(Engine* const engine) {
    (void)engine;
  }
  void end(Engine* const engine) {
    (void)engine;
  }

  bool allowMove() {
    return false;
  }
  bool allowAct() {
    return false;
  }
  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }

private:
  DiceParam getRandomStandardNrTurns() {
    return DiceParam(0, 0, 1);
  }
};

class StatusDisabledAttack: public StatusEffect {
public:
  StatusDisabledAttack(Engine* const engine) :
    StatusEffect(statusDisabledAttack) {
    setTurnsFromRandomStandard(engine);
  }
  StatusDisabledAttack(const int TURNS) :
    StatusEffect(TURNS, statusDisabledAttack) {
  }
  ~StatusDisabledAttack() {}

  StatusDisabledAttack* copy() {
    StatusDisabledAttack* cpy = new StatusDisabledAttack(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer() {return false;}

  bool isConsideredBeneficial() {return false;}

  bool allowDisplayTurnsInInterface() {return false;}

  string getInterfaceName() {return "";}
  string msgOnStartPlayer() {return "";}
  string msgOnMorePlayer() {return "";}
  string msgOnMoreMonster() {return "";}
  string msgOnEndPlayer() {return "";}
  string msgOnSavePlayer() {return "";}
  string msgOnStartMonster() {return "";}
  string msgOnEndMonster() {return "";}
  string msgOnSaveMonster() {return "";}
  void newTurn(Engine* const engine) {(void)engine; turnsLeft--;}
  void start(Engine* const engine) {(void)engine;}
  void end(Engine* const engine) {(void)engine;}

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
  bool allowAttackMleee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
private:
  DiceParam getRandomStandardNrTurns() {return DiceParam(0, 0, 1);}
};

class StatusDisabledAttackMelee: public StatusEffect {
public:
  StatusDisabledAttackMelee(Engine* const engine) :
    StatusEffect(statusDisabledMelee) {
    setTurnsFromRandomStandard(engine);
  }
  StatusDisabledAttackMelee(const int TURNS) :
    StatusEffect(TURNS, statusDisabledMelee) {
  }
  ~StatusDisabledAttackMelee() {
  }

  StatusDisabledAttackMelee* copy() {
    StatusDisabledAttackMelee* cpy = new StatusDisabledAttackMelee(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer() {
    return false;
  }

  bool isConsideredBeneficial() {
    return false;
  }

  bool allowDisplayTurnsInInterface() {
    return false;
  }

  string getInterfaceName() {
    return "";
  }
  string msgOnStartPlayer() {
    return "";
  }
  string msgOnMorePlayer() {
    return "";
  }
  string msgOnMoreMonster() {
    return "";
  }
  string msgOnEndPlayer() {
    return "";
  }
  string msgOnSavePlayer() {
    return "";
  }
  string msgOnStartMonster() {
    return "";
  }
  string msgOnEndMonster() {
    return "";
  }
  string msgOnSaveMonster() {
    return "";
  }
  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }
  void start(Engine* const engine) {
    (void)engine;
  }
  void end(Engine* const engine) {
    (void)engine;
  }

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
private:
  DiceParam getRandomStandardNrTurns() {
    return DiceParam(0, 0, 1);
  }
};

class StatusDisabledAttackRanged: public StatusEffect {
public:
  StatusDisabledAttackRanged(Engine* const engine) :
    StatusEffect(statusDisabledRanged) {
    setTurnsFromRandomStandard(engine);
  }
  StatusDisabledAttackRanged(const int TURNS) :
    StatusEffect(TURNS, statusDisabledRanged) {
  }
  ~StatusDisabledAttackRanged() {
  }

  StatusDisabledAttackRanged* copy() {
    StatusDisabledAttackRanged* cpy = new StatusDisabledAttackRanged(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer() {
    return false;
  }

  bool isConsideredBeneficial() {
    return false;
  }

  bool allowDisplayTurnsInInterface() {
    return false;
  }

  string getInterfaceName() {
    return "";
  }
  string msgOnStartPlayer() {
    return "";
  }
  string msgOnMorePlayer() {
    return "";
  }
  string msgOnMoreMonster() {
    return "";
  }
  string msgOnEndPlayer() {
    return "";
  }
  string msgOnSavePlayer() {
    return "";
  }
  string msgOnStartMonster() {
    return "";
  }
  string msgOnEndMonster() {
    return "";
  }
  string msgOnSaveMonster() {
    return "";
  }
  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }
  void start(Engine* const engine) {
    (void)engine;
  }
  void end(Engine* const engine) {
    (void)engine;
  }

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
private:
  DiceParam getRandomStandardNrTurns() {
    return DiceParam(0, 0, 1);
  }
};

class StatusParalyzed: public StatusEffect {
public:
  StatusParalyzed(Engine* const engine) :
    StatusEffect(statusParalyzed) {
    setTurnsFromRandomStandard(engine);
  }
  StatusParalyzed(const int TURNS) :
    StatusEffect(TURNS, statusParalyzed) {
  }
  ~StatusParalyzed() {
  }

  StatusParalyzed* copy() {
    StatusParalyzed* cpy = new StatusParalyzed(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer() {
    return true;
  }

  bool isConsideredBeneficial() {
    return false;
  }

  bool allowDisplayTurnsInInterface() {
    return true;
  }

  string getInterfaceName() {
    return "Paralyzed";
  }
  string msgOnStartPlayer() {
    return "I am paralyzed!";
  }
  string msgOnMorePlayer() {
    return "";
  }
  string msgOnMoreMonster() {
    return "";
  }
  string msgOnEndPlayer() {
    return "I can move again!";
  }
  string msgOnSavePlayer() {
    return "I resist paralyzation.";
  }
  string msgOnStartMonster() {
    return "is paralyzed.";
  }
  string msgOnEndMonster() {
    return "can move again.";
  }
  string msgOnSaveMonster() {
    return "resists paralyzation.";
  }

  void start(Engine* const engine);

  void end(Engine* const engine) {
    (void)engine;
  }

  bool allowAct() {
    return false;
  }

  int getAbilityModifier(const Abilities_t ability) {
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

  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }

  bool canBeAppliedWhileSameEffectOngoing() {
    return false;
  }

private:
  DiceParam getRandomStandardNrTurns() {
    return DiceParam(1, 3, 6);
  }
};

class StatusFainted: public StatusEffect {
public:
  StatusFainted(Engine* const engine) :
    StatusEffect(statusFainted) {
    setTurnsFromRandomStandard(engine);
  }
  ~StatusFainted() {
  }

  StatusFainted* copy() {
    StatusFainted* cpy = new StatusFainted(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer() {
    return true;
  }

  bool isConsideredBeneficial() {
    return false;
  }

  bool allowDisplayTurnsInInterface() {
    return true;
  }

  bool isPlayerVisualUpdateNeededWhenStartOrEnd();

  string getInterfaceName() {
    return "Fainted";
  }
  string msgOnStartPlayer() {
    return "I faint!";
  }
  string msgOnMorePlayer() {
    return "I faint deeper.";
  }
  string msgOnMoreMonster() {
    return "faints deeper.";
  }
  string msgOnEndPlayer() {
    return "I am awake.";
  }
  string msgOnSavePlayer() {
    return "I resist fainting.";
  }
  string msgOnStartMonster() {
    return "faints.";
  }
  string msgOnEndMonster() {
    return "wakes up.";
  }
  string msgOnSaveMonster() {
    return "resists fainting.";
  }

  void start(Engine* const engine);

  void end(Engine* const engine);

  bool allowAct() {
    return false;
  }

  bool allowSee() {
    return false;
  }

  int getAbilityModifier(const Abilities_t ability) {
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

  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }

  void isHit() {
    turnsLeft = 0;
  }

private:
  DiceParam getRandomStandardNrTurns() {
    return DiceParam(1, 50, 50);
  }

  friend class StatusHandler;
  StatusFainted(const int TURNS) :
    StatusEffect(TURNS, statusFainted) {
  }
};

class StatusSlowed: public StatusEffect {
public:
  StatusSlowed(Engine* const engine) :
    StatusEffect(statusSlowed) {
    setTurnsFromRandomStandard(engine);
  }
  StatusSlowed(const int TURNS) :
    StatusEffect(TURNS, statusSlowed) {
  }
  ~StatusSlowed() {
  }

  StatusSlowed* copy() {
    StatusSlowed* cpy = new StatusSlowed(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer() {
    return true;
  }

  bool isConsideredBeneficial() {
    return false;
  }

  bool allowDisplayTurnsInInterface() {
    return true;
  }

  string getInterfaceName() {
    return "Slowed";
  }
  string msgOnStartPlayer() {
    return "Everything around me seems to speed up.";
  }
  string msgOnMorePlayer() {
    return "I am more slowed.";
  }
  string msgOnMoreMonster() {
    return "slows down more.";
  }
  string msgOnEndPlayer() {
    return "Everything around me seems to slow down.";
  }
  string msgOnSavePlayer() {
    return "I resist slowness.";
  }
  string msgOnStartMonster() {
    return "slows down.";
  }
  string msgOnEndMonster() {
    return "speeds up.";
  }
  string msgOnSaveMonster() {
    return "resists slowness.";
  }

  void start(Engine* const engine) {
    (void)engine;
  }

  void end(Engine* const engine) {
    (void)engine;
  }

  bool isSlowed() {
    return true;
  }

  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }

private:
  DiceParam getRandomStandardNrTurns() {
    return DiceParam(3, 6, 6);
  }
};

class StatusPerfectReflexes: public StatusEffect {
public:
  StatusPerfectReflexes(Engine* const engine) :
    StatusEffect(statusPerfectReflexes) {
    setTurnsFromRandomStandard(engine);
  }
  StatusPerfectReflexes(const int TURNS) :
    StatusEffect(TURNS, statusPerfectReflexes) {
  }
  ~StatusPerfectReflexes() {
  }

  StatusPerfectReflexes* copy() {
    StatusPerfectReflexes* cpy = new StatusPerfectReflexes(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer() {
    return false;
  }

  bool isConsideredBeneficial() {
    return true;
  }

  bool allowDisplayTurnsInInterface() {
    return true;
  }

  string getInterfaceName() {
    return "Reflexes";
  }
  string msgOnStartPlayer() {
    return "I have mighty reflexes!";
  }
  string msgOnMorePlayer() {
    return "I have mighty reflexes!";
  }
  string msgOnMoreMonster() {
    return "has mighty reflexes.";
  }
  string msgOnEndPlayer() {
    return "My reflexes are normal.";
  }
  string msgOnSavePlayer() {
    return "";
  }
  string msgOnStartMonster() {
    return "has mighty reflexes.";
  }
  string msgOnEndMonster() {
    return "has normal reflexes.";
  }
  string msgOnSaveMonster() {
    return "";
  }

  int getAbilityModifier(const Abilities_t ability) {
    if(ability == ability_dodgeTrap || ability == ability_dodgeAttack)
      return 999;
    return 0;
  }

  void start(Engine* const engine) {
    (void)engine;
  }

  void end(Engine* const engine) {
    (void)engine;
  }

  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }

private:
  DiceParam getRandomStandardNrTurns() {
    return DiceParam(3, 8, 24);
  }
};

class StatusPerfectAim: public StatusEffect {
public:
  StatusPerfectAim(Engine* const engine) :
    StatusEffect(statusPerfectAim) {
    setTurnsFromRandomStandard(engine);
  }
  StatusPerfectAim(const int TURNS) :
    StatusEffect(TURNS, statusPerfectAim) {
  }
  ~StatusPerfectAim() {
  }

  StatusPerfectAim* copy() {
    StatusPerfectAim* cpy = new StatusPerfectAim(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer() {
    return false;
  }

  bool isConsideredBeneficial() {
    return true;
  }

  bool allowDisplayTurnsInInterface() {
    return true;
  }

  string getInterfaceName() {
    return "Aiming";
  }
  string msgOnStartPlayer() {
    return "I have excellent aiming!";
  }
  string msgOnMorePlayer() {
    return "I have excellent aiming!";
  }
  string msgOnMoreMonster() {
    return "has excellent aiming.";
  }
  string msgOnEndPlayer() {
    return "My aiming is normal.";
  }
  string msgOnSavePlayer() {
    return "";
  }
  string msgOnStartMonster() {
    return "has excellent aiming.";
  }
  string msgOnEndMonster() {
    return "has normal aiming.";
  }
  string msgOnSaveMonster() {
    return "";
  }

  int getAbilityModifier(const Abilities_t ability) {
    if(ability == ability_accuracyMelee)
      return 999;
    if(ability == ability_accuracyRanged)
      return 999;
    return 0;
  }

  void start(Engine* const engine) {
    (void)engine;
  }

  void end(Engine* const engine) {
    (void)engine;
  }

  void newTurn(Engine* const engine) {
    (void)engine;
    turnsLeft--;
  }

private:
  DiceParam getRandomStandardNrTurns() {
    return DiceParam(3, 8, 24);
  }
};

//class StatusPerfectStealth: public StatusEffect {
//public:
//  StatusPerfectStealth(Engine* const engine) :
//    StatusEffect(statusPerfectStealth) {
//    setTurnsFromRandomStandard(engine);
//  }
//  StatusPerfectStealth(const int TURNS) :
//    StatusEffect(TURNS, statusPerfectStealth) {
//  }
//  ~StatusPerfectStealth() {
//  }
//
//  StatusPerfectStealth* copy() {
//    StatusPerfectStealth* cpy = new StatusPerfectStealth(turnsLeft);
//    return cpy;
//  }
//
//  bool isMakingOwnerAwareOfPlayer() {
//    return false;
//  }
//
//  bool isConsideredBeneficial() {
//    return true;
//  }
//
//  bool allowDisplayTurnsInInterface() {
//    return true;
//  }
//
//  string getInterfaceName() {
//    return "Stealth";
//  }
//  string msgOnStartPlayer() {
//    return "I move silent like a ghost!";
//  }
//  string msgOnMorePlayer() {
//    return "I move silent like a ghost!";
//  }
//  string msgOnMoreMonster() {
//    return "";
//  }
//  string msgOnEndPlayer() {
//    return "My movements make noises again.";
//  }
//  string msgOnSavePlayer() {
//    return "";
//  }
//  string msgOnStartMonster() {
//    return "";
//  }
//  string msgOnEndMonster() {
//    return "";
//  }
//  string msgOnSaveMonster() {
//    return "";
//  }
//
//  int getAbilityModifier(const Abilities_t ability) {
//    if(ability == ability_stealth)
//      return 999;
//    return 0;
//  }
//
//  void start(Engine* const engine) {
//    (void)engine;
//  }
//
//  void end(Engine* const engine) {
//    (void)engine;
//  }
//
//  void newTurn(Engine* const engine) {
//    (void)engine;
//    turnsLeft--;
//  }
//
//private:
//  DiceParam getRandomStandardNrTurns() {
//    return DiceParam(8, 8, 100);
//  }
//};

class StatusHandler {
public:
  StatusHandler(Actor* const owningActor_, Engine* const engine) :
    owningActor(owningActor_), eng(engine) {
    effects.resize(0);
  }
  ~StatusHandler() {
    for(unsigned int i = 0; i < effects.size(); i++) {
      delete effects.at(i);
    }
    effects.resize(0);
  }

  void tryAddEffect(StatusEffect* const effect, const bool FORCE_EFFECT = false,
                    const bool NO_MESSAGES = false,
                    const bool DISABLE_REDRAW = false);

  void tryAddEffectsFromWeapon(const Weapon& wpn, const bool IS_MELEE);

  Pos changeMovePos(const Pos& actorPos, const Pos& movePos) {
    Pos ret = movePos;
    for(unsigned int i = 0; i < effects.size(); i++) {
      ret = effects.at(i)->changeMovePos(actorPos, movePos, eng);
    }
    return ret;
  }

  bool allowAttack(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    for(unsigned int i = 0; i < effects.size(); i++) {
      if(
        effects.at(i)->allowAttackMelee(ALLOW_MESSAGE_WHEN_FALSE) == false &&
        effects.at(i)->allowAttackRanged(ALLOW_MESSAGE_WHEN_FALSE) == false) {
        return false;
      }
    }
    return true;
  }

  bool allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE);

  bool allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE);

  bool isSlowed() {
    for(unsigned int i = 0; i < effects.size(); i++) {
      if(effects.at(i)->isSlowed() == true) {
        return true;
      }
    }
    return false;
  }

  bool allowSee();

  bool allowMove() {
    for(unsigned int i = 0; i < effects.size(); i++) {
      if(effects.at(i)->allowMove() == false) {
        return false;
      }
    }
    return true;
  }

  void isHit() {
    for(unsigned int i = 0; i < effects.size(); i++) {
      effects.at(i)->isHit();
    }
  }

  bool allowAct() {
    for(unsigned int i = 0; i < effects.size(); i++) {
      if(effects.at(i)->allowAct() == false) {
        return false;
      }
    }
    return true;
  }

  int getAbilityModifier(const Abilities_t ability) {
    int modifier = 0;
    for(unsigned int i = 0; i < effects.size(); i++) {
      modifier += effects.at(i)->getAbilityModifier(ability);
    }
    return modifier;
  }

  bool isEthereal() {
    for(unsigned int i = 0; i < effects.size(); i++) {
      if(effects.at(i)->isEthereal() == true) {
        return true;
      }
    }
    return false;
  }

  bool hasEffect(const StatusEffects_t effect) const {
    for(unsigned int i = 0; i < effects.size(); i++) {
      if(effects.at(i)->getEffectId() == effect) {
        return true;
      }
    }
    return false;
  }

  StatusEffect* getEffect(const StatusEffects_t effect) const {
    for(unsigned int i = 0; i < effects.size(); i++) {
      if(effects.at(i)->getEffectId() == effect) {
        return effects.at(i);
      }
    }
    return NULL;
  }

  bool hasAnyBadEffect() const {
    for(unsigned int i = 0; i < effects.size(); i++) {
      if(effects.at(i)->isConsideredBeneficial() == false) {
        return true;
      }
    }
    return false;
  }

  void endEffect(const StatusEffects_t effect,
                 const bool visionBlockingArray[MAP_X_CELLS][MAP_Y_CELLS],
                 const bool RUN_STATUS_END_EFFECTS = true) {
    for(unsigned int i = 0; i < effects.size(); i++) {
      if(effects.at(i)->getEffectId() == effect) {
        if(RUN_STATUS_END_EFFECTS) {
          runEffectEndAndRemoveFromList(i, visionBlockingArray);
        } else {
          delete effects.at(i);
          effects.erase(effects.begin() + i);
        }
        return;
      }
    }
  }

  SDL_Color getColor() {
    for(unsigned int i = 0; i < effects.size(); i++) {
      const SDL_Color& clr = effects.at(i)->getColorOwningActor();
      if(clr.r != 0 || clr.g != 0 || clr.b != 0) {
        return effects.at(i)->getColorOwningActor();
      }
    }
    return clrBlack;
  }

  vector<StatusEffect*> effects;

private:
  //This function was created so that the id can be saved to file when saving
  //the game, and then have the effect recreated from this.
  //In other cases, a status effect should be created by simply using "new"
  friend class Player;
  StatusEffect* makeEffectFromId(
    const StatusEffects_t id, const int TURNS_LEFT);

  Actor* owningActor;
  Engine* eng;

  void runEffectEndAndRemoveFromList(
    const unsigned int index,
    const bool visionBlockingArray[MAP_X_CELLS][MAP_Y_CELLS]);

  friend class GameTime;
  void newTurnAllEffects(
    const bool visionBlockingArray[MAP_X_CELLS][MAP_Y_CELLS]);
};

#endif
