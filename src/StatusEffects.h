#ifndef STATUS_EFFECTS_H
#define STATUS_EFFECTS_H

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

enum StatusEffects_t {
  statusWound,
  statusBlind,
  statusBurning,
  statusFlared,
  statusParalyzed,
  statusTerrified,
  statusConfused,
  statusWaiting,
  statusSlowed,
  statusInfected,
  statusDiseased,
  statusPoisoned,
  statusFainted,
  statusWeak,
  statusPerfectReflexes,
  statusPerfectAim,
  statusPerfectFortitude,
  statusPerfectToughness,
  statusCursed,
  statusBlessed,
  statusClairvoyant,

  statusNailed,

  //Status for the steady aimer ability
  statusStill,

  //The following are mostly used for AI controll
  statusDisabledAttack,
  statusDisabledMelee,
  statusDisabledRanged
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

  //Ability used for saving, and how it is modified.
  virtual Abilities_t getSaveAbility() = 0;
  virtual int getSaveAbilityModifier() = 0;

  virtual StatusEffect* copy() = 0;

  //Questions for the status to answer. This is how the status ends,
  //how it damages the actor, how it affects ability values, etc.
  virtual bool isConsideredBeneficial() = 0;
  virtual bool allowDisplayTurnsInInterface() = 0;
  virtual bool isMakingOwnerAwareOfPlayer() = 0;
  virtual string getInterfaceName() = 0;
  virtual string messageWhenStart() = 0;
  virtual string messageWhenStartOther() = 0;
  virtual string messageWhenEnd() = 0;
  virtual string messageWhenEndOther() = 0;
  virtual string messageWhenSaves() = 0;
  virtual string messageWhenSavesOther() = 0;
  virtual string messageWhenMore() = 0;
  virtual string messageWhenMoreOther() = 0;
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
  string messageWhenStart()       {return "I am wounded!";}
  string messageWhenStartOther()  {return "";}
  string messageWhenMore()        {return "I am more wounded!";}
  string messageWhenMoreOther()   {return "";}
  string messageWhenEnd()         {return "My wound is healed.";}
  string messageWhenSaves()       {return "I resist wounding!";}
  string messageWhenSavesOther()  {return "";}
  string messageWhenEndOther()    {return "";}

  Abilities_t getSaveAbility() {return ability_resistStatusBody;}
  int getSaveAbilityModifier() {return -10;}

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
  string messageWhenStart()       {return "I am terrified!";}
  string messageWhenStartOther()  {return "looks terrified.";}
  string messageWhenMore()        {return "I am more terrified.";}
  string messageWhenMoreOther()   {return "looks more terrified.";}
  string messageWhenEnd()         {return "I am no longer terrified!";}
  string messageWhenSaves()       {return "I resist fear.";}
  string messageWhenSavesOther()  {return "resists fear.";}
  string messageWhenEndOther()    {return "is no longer terrified.";}

  Abilities_t getSaveAbility() {return ability_resistStatusMind;}
  int getSaveAbilityModifier() {return 10;}

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
  string messageWhenStart()       {return "I feel weaker!";}
  string messageWhenStartOther()  {return "looks weaker.";}
  string messageWhenMore()        {return "I feel weaker.";}
  string messageWhenMoreOther()   {return "looks weaker!";}
  string messageWhenEnd()         {return "I feel stronger!";}
  string messageWhenSaves()       {return "I resist weakness.";}
  string messageWhenSavesOther()  {return "resists weakness.";}
  string messageWhenEndOther()    {return "looks stronger.";}

  Abilities_t getSaveAbility() {return ability_resistStatusBody;}
  int getSaveAbilityModifier() {return 0;}

  int getAbilityModifier(const Abilities_t ability) {
    if(ability == ability_resistStatusBody) return -30;
    return 0;
  }

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
  string messageWhenStart()       {return "I am infected!";}
  string messageWhenStartOther()  {return "is infected.";}
  string messageWhenMore()        {return "I am more infected.";}
  string messageWhenMoreOther()   {return "is more infected.";}
  string messageWhenEnd()         {return "My infection is cured!";}
  string messageWhenSaves()       {return "I resist infection.";}
  string messageWhenSavesOther()  {return "resists infection.";}
  string messageWhenEndOther()    {return "is no longer infected.";}

  Abilities_t getSaveAbility() {return ability_resistStatusBody;}
  int getSaveAbilityModifier() {return 10;}

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
  string messageWhenStart()       {return "I am diseased!";}
  string messageWhenStartOther()  {return "is diseased.";}
  string messageWhenMore()        {return "I am more diseased.";}
  string messageWhenMoreOther()   {return "is more diseased.";}
  string messageWhenEnd()         {return "My disease is cured!";}
  string messageWhenSaves()       {return "I resist disease.";}
  string messageWhenSavesOther()  {return "resists disease.";}
  string messageWhenEndOther()    {return "is no longer diseased.";}

  Abilities_t getSaveAbility() {return ability_resistStatusBody;}
  int getSaveAbilityModifier() {return 10;}

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
  string messageWhenStart()       {return "I am poisoned!";}
  string messageWhenStartOther()  {return "is poisoned.";}
  string messageWhenMore()        {return "I am more poisoned.";}
  string messageWhenMoreOther()   {return "is more poisoned.";}
  string messageWhenEnd()         {return "My body is cleansed from poisoning!";}
  string messageWhenSaves()       {return "I resist poisoning.";}
  string messageWhenSavesOther()  {return "resists poisoning.";}
  string messageWhenEndOther()    {return "is cleansed from poisoning.";}

  Abilities_t getSaveAbility() {return ability_resistStatusBody;}
  int getSaveAbilityModifier() {return 0;}

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
  string messageWhenStart()       {return "";}
  string messageWhenMore()        {return "";}
  string messageWhenMoreOther()   {return "";}
  string messageWhenEnd()         {return "";}
  string messageWhenSaves()       {return "";}
  string messageWhenStartOther()  {return "";}
  string messageWhenEndOther()    {return "";}
  string messageWhenSavesOther()  {return "";}

  Abilities_t getSaveAbility() {return ability_empty;}
  int getSaveAbilityModifier() {return -9999;}

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
  string messageWhenStart()       {return "I am blind!";}
  string messageWhenMore()        {return "I am more blind.";}
  string messageWhenMoreOther()   {return "is more blind.";}
  string messageWhenEnd()         {return "I can see again!";}
  string messageWhenSaves()       {return "I resist blindness.";}
  string messageWhenStartOther()  {return "is blinded.";}
  string messageWhenEndOther()    {return "can see again.";}
  string messageWhenSavesOther()  {return "resists blindness.";}

  Abilities_t getSaveAbility() {return ability_resistStatusBody;}
  int getSaveAbilityModifier() {return 0;}

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
  string messageWhenStart()       {return "I feel luckier.";}
  string messageWhenMore()        {return "I feel luckier.";}
  string messageWhenMoreOther()   {return "is luckier.";}
  string messageWhenEnd()         {return "My good luck ends.";}
  string messageWhenSaves()       {return "";}
  string messageWhenStartOther()  {return "is more lucky.";}
  string messageWhenEndOther()    {return "has normal luck.";}
  string messageWhenSavesOther()  {return "";}

  Abilities_t getSaveAbility() {return ability_empty;}
  int getSaveAbilityModifier() {return 0;}

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
  string messageWhenStart()       {return "I feel misfortunate.";}
  string messageWhenMore()        {return "I feel more misfortunate.";}
  string messageWhenMoreOther()   {return "is misfortunate.";}
  string messageWhenEnd()         {return "My misfortune ends.";}
  string messageWhenSaves()       {return "";}
  string messageWhenStartOther()  {return "is more misfortunate.";}
  string messageWhenEndOther()    {return "has normal fortune.";}
  string messageWhenSavesOther()  {return "";}

  Abilities_t getSaveAbility() {return ability_empty;}
  int getSaveAbilityModifier() {return 0;}

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
  string messageWhenStart()       {return "I see far and beyond!";}
  string messageWhenMore()        {return "I see far and beyond!";}
  string messageWhenMoreOther()   {return "";}
  string messageWhenEnd()         {return "My sight is limited.";}
  string messageWhenSaves()       {return "";}
  string messageWhenStartOther()  {return "";}
  string messageWhenEndOther()    {return "";}
  string messageWhenSavesOther()  {return "";}

  Abilities_t getSaveAbility() {return ability_empty;}
  int getSaveAbilityModifier() {return 0;}

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
  string messageWhenStart()       {return "I am Burning!";}
  string messageWhenMore()        {return "I am further engulfed by flames!";}
  string messageWhenMoreOther()   {return "is burning more.";}
  string messageWhenEnd()         {return "The flames are put out.";}
  string messageWhenSaves()       {return "I resist burning.";}
  string messageWhenStartOther()  {return "is burning.";}
  string messageWhenEndOther()    {return "is no longer burning.";}
  string messageWhenSavesOther()  {return "resists burning.";}

  Abilities_t getSaveAbility() {return ability_resistStatusBody;}
  int getSaveAbilityModifier() {return -20;}

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
  string messageWhenStart()       {return "";}
  string messageWhenMore()        {return ".";}
  string messageWhenMoreOther()   {return "is perforated by another flare.";}
  string messageWhenEnd()         {return "";}
  string messageWhenSaves()       {return "";}
  string messageWhenStartOther()  {return "is perforated by a flare.";}
  string messageWhenEndOther()    {return "recovers from a flare.";}
  string messageWhenSavesOther()  {return "resists a flare.";}

  Abilities_t getSaveAbility() {return ability_resistStatusBody;}
  int getSaveAbilityModifier() {return -100;}

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
  string messageWhenStart()       {return "I am confused!";}
  string messageWhenStartOther()  {return "is confused.";}
  string messageWhenMore()        {return "I am more confused.";}
  string messageWhenMoreOther()   {return "is more confused.";}
  string messageWhenEnd()         {return "I am no longer confused";}
  string messageWhenEndOther()    {return "is no longer confused.";}
  string messageWhenSaves()       {return "I resist confusion.";}
  string messageWhenSavesOther()  {return "resists confusion.";}

  Abilities_t getSaveAbility() {return ability_resistStatusMind;}
  int getSaveAbilityModifier() {return 0;}

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
  string messageWhenStart()       {return "I am fastened by a spike!";}
  string messageWhenStartOther()  {return "is fastened by a spike.";}
  string messageWhenMore()        {return "I am fastened by another spike.";}
  string messageWhenMoreOther()   {return "is fastened by another spike.";}
  string messageWhenEnd()         {return "I tear free!";}
  string messageWhenEndOther()    {return "tears free!";}
  string messageWhenSaves()       {return "";}
  string messageWhenSavesOther()  {return "";}

  Abilities_t getSaveAbility() {return ability_resistStatusBody;}
  int getSaveAbilityModifier() {return 27;}

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

  Abilities_t getSaveAbility() {
    return ability_empty;
  }
  int getSaveAbilityModifier() {
    return -999;
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
  string messageWhenStart() {
    return "";
  }
  string messageWhenMore() {
    return "";
  }
  string messageWhenMoreOther() {
    return "";
  }
  string messageWhenEnd() {
    return "";
  }
  string messageWhenSaves() {
    return "";
  }
  string messageWhenStartOther() {
    return "";
  }
  string messageWhenEndOther() {
    return "";
  }
  string messageWhenSavesOther() {
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
  ~StatusDisabledAttack() {
  }

  StatusDisabledAttack* copy() {
    StatusDisabledAttack* cpy = new StatusDisabledAttack(turnsLeft);
    return cpy;
  }

  bool isMakingOwnerAwareOfPlayer() {
    return false;
  }

  Abilities_t getSaveAbility() {
    return ability_empty;
  }
  int getSaveAbilityModifier() {
    return -999;
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
  string messageWhenStart() {
    return "";
  }
  string messageWhenMore() {
    return "";
  }
  string messageWhenMoreOther() {
    return "";
  }
  string messageWhenEnd() {
    return "";
  }
  string messageWhenSaves() {
    return "";
  }
  string messageWhenStartOther() {
    return "";
  }
  string messageWhenEndOther() {
    return "";
  }
  string messageWhenSavesOther() {
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
  bool allowAttackMleee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
    (void)ALLOW_MESSAGE_WHEN_FALSE;
    return false;
  }
private:
  DiceParam getRandomStandardNrTurns() {
    return DiceParam(0, 0, 1);
  }
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

  Abilities_t getSaveAbility() {
    return ability_empty;
  }
  int getSaveAbilityModifier() {
    return -999;
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
  string messageWhenStart() {
    return "";
  }
  string messageWhenMore() {
    return "";
  }
  string messageWhenMoreOther() {
    return "";
  }
  string messageWhenEnd() {
    return "";
  }
  string messageWhenSaves() {
    return "";
  }
  string messageWhenStartOther() {
    return "";
  }
  string messageWhenEndOther() {
    return "";
  }
  string messageWhenSavesOther() {
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

  Abilities_t getSaveAbility() {
    return ability_empty;
  }
  int getSaveAbilityModifier() {
    return -999;
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
  string messageWhenStart() {
    return "";
  }
  string messageWhenMore() {
    return "";
  }
  string messageWhenMoreOther() {
    return "";
  }
  string messageWhenEnd() {
    return "";
  }
  string messageWhenSaves() {
    return "";
  }
  string messageWhenStartOther() {
    return "";
  }
  string messageWhenEndOther() {
    return "";
  }
  string messageWhenSavesOther() {
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
  string messageWhenStart() {
    return "I am paralyzed!";
  }
  string messageWhenMore() {
    return "";
  }
  string messageWhenMoreOther() {
    return "";
  }
  string messageWhenEnd() {
    return "I can move again!";
  }
  string messageWhenSaves() {
    return "I resist paralyzation.";
  }
  string messageWhenStartOther() {
    return "is paralyzed.";
  }
  string messageWhenEndOther() {
    return "can move again.";
  }
  string messageWhenSavesOther() {
    return "resists paralyzation.";
  }

  Abilities_t getSaveAbility() {
    return ability_resistStatusBody;
  }
  int getSaveAbilityModifier() {
    return 0;
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
  string messageWhenStart() {
    return "I faint!";
  }
  string messageWhenMore() {
    return "I faint deeper.";
  }
  string messageWhenMoreOther() {
    return "faints deeper.";
  }
  string messageWhenEnd() {
    return "I am awake.";
  }
  string messageWhenSaves() {
    return "I resist fainting.";
  }
  string messageWhenStartOther() {
    return "faints.";
  }
  string messageWhenEndOther() {
    return "wakes up.";
  }
  string messageWhenSavesOther() {
    return "resists fainting.";
  }

  Abilities_t getSaveAbility() {
    return ability_resistStatusMind;
  }
  int getSaveAbilityModifier() {
    return -15;
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
  string messageWhenStart() {
    return "Everything around me seems to speed up.";
  }
  string messageWhenMore() {
    return "I am more slowed.";
  }
  string messageWhenMoreOther() {
    return "slows down more.";
  }
  string messageWhenEnd() {
    return "Everything around me seems to slow down.";
  }
  string messageWhenSaves() {
    return "I resist slowness.";
  }
  string messageWhenStartOther() {
    return "slows down.";
  }
  string messageWhenEndOther() {
    return "speeds up.";
  }
  string messageWhenSavesOther() {
    return "resists slowness.";
  }

  Abilities_t getSaveAbility() {
    return ability_resistStatusBody;
  }
  int getSaveAbilityModifier() {
    return 0;
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
  string messageWhenStart() {
    return "I have mighty reflexes!";
  }
  string messageWhenMore() {
    return "I have mighty reflexes!";
  }
  string messageWhenMoreOther() {
    return "has mighty reflexes.";
  }
  string messageWhenEnd() {
    return "My reflexes are normal.";
  }
  string messageWhenSaves() {
    return "";
  }
  string messageWhenStartOther() {
    return "has mighty reflexes.";
  }
  string messageWhenEndOther() {
    return "has normal reflexes.";
  }
  string messageWhenSavesOther() {
    return "";
  }

  Abilities_t getSaveAbility() {
    return ability_empty;
  }
  int getSaveAbilityModifier() {
    return 0;
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
  string messageWhenStart() {
    return "I have excellent aiming!";
  }
  string messageWhenMore() {
    return "I have excellent aiming!";
  }
  string messageWhenMoreOther() {
    return "has excellent aiming.";
  }
  string messageWhenEnd() {
    return "My aiming is normal.";
  }
  string messageWhenSaves() {
    return "";
  }
  string messageWhenStartOther() {
    return "has excellent aiming.";
  }
  string messageWhenEndOther() {
    return "has normal aiming.";
  }
  string messageWhenSavesOther() {
    return "";
  }

  Abilities_t getSaveAbility() {
    return ability_empty;
  }
  int getSaveAbilityModifier() {
    return 0;
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
//  string messageWhenStart() {
//    return "I move silent like a ghost!";
//  }
//  string messageWhenMore() {
//    return "I move silent like a ghost!";
//  }
//  string messageWhenMoreOther() {
//    return "";
//  }
//  string messageWhenEnd() {
//    return "My movements make noises again.";
//  }
//  string messageWhenSaves() {
//    return "";
//  }
//  string messageWhenStartOther() {
//    return "";
//  }
//  string messageWhenEndOther() {
//    return "";
//  }
//  string messageWhenSavesOther() {
//    return "";
//  }
//
//  Abilities_t getSaveAbility() {
//    return ability_empty;
//  }
//  int getSaveAbilityModifier() {
//    return 0;
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

class StatusPerfectFortitude: public StatusEffect {
public:
  StatusPerfectFortitude(Engine* const engine) :
    StatusEffect(statusPerfectFortitude) {
    setTurnsFromRandomStandard(engine);
  }
  StatusPerfectFortitude(const int TURNS) :
    StatusEffect(TURNS, statusPerfectFortitude) {
  }
  ~StatusPerfectFortitude() {
  }

  StatusPerfectFortitude* copy() {
    StatusPerfectFortitude* cpy = new StatusPerfectFortitude(turnsLeft);
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
    return "Fortitude";
  }
  string messageWhenStart() {
    return "I have mighty fortitude!";
  }
  string messageWhenMore() {
    return "I have mighty fortitude!";
  }
  string messageWhenMoreOther() {
    return "has mighty fortitude!";
  }
  string messageWhenEnd() {
    return "My fortitude is normal.";
  }
  string messageWhenSaves() {
    return "";
  }
  string messageWhenStartOther() {
    return "has mighty fortitude!";
  }
  string messageWhenEndOther() {
    return "has normal fortitude.";
  }
  string messageWhenSavesOther() {
    return "";
  }

  Abilities_t getSaveAbility() {
    return ability_empty;
  }
  int getSaveAbilityModifier() {
    return 0;
  }

  int getAbilityModifier(const Abilities_t ability) {
    if(ability == ability_resistStatusMind)
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

class StatusPerfectToughness: public StatusEffect {
public:
  StatusPerfectToughness(Engine* const engine) :
    StatusEffect(statusPerfectToughness) {
    setTurnsFromRandomStandard(engine);
  }
  StatusPerfectToughness(const int TURNS) :
    StatusEffect(TURNS, statusPerfectToughness) {
  }
  ~StatusPerfectToughness() {
  }

  StatusPerfectToughness* copy() {
    StatusPerfectToughness* cpy = new StatusPerfectToughness(turnsLeft);
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
    return "Toughness";
  }
  string messageWhenStart() {
    return "I have mighty toughness!";
  }
  string messageWhenMore() {
    return "I have mighty toughness!";
  }
  string messageWhenMoreOther() {
    return "has mighty toughness!";
  }
  string messageWhenEnd() {
    return "My toughness is normal.";
  }
  string messageWhenSaves() {
    return "";
  }
  string messageWhenStartOther() {
    return "has mighty toughness!";
  }
  string messageWhenEndOther() {
    return "has normal toughness.";
  }
  string messageWhenSavesOther() {
    return "";
  }

  Abilities_t getSaveAbility() {
    return ability_empty;
  }
  int getSaveAbilityModifier() {
    return 0;
  }

  int getAbilityModifier(const Abilities_t ability) {
    if(ability == ability_resistStatusBody)
      return 999;
    return 0;
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
    return DiceParam(3, 8, 24);
  }
};

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

  void endEffectsOfAbility(
    const Abilities_t ability,
    const bool visionBlockingArray[MAP_X_CELLS][MAP_Y_CELLS]) {
    for(int i = 0; i < int(effects.size()); i++) {
      const unsigned int ELEMENT = static_cast<unsigned int>(i);
      if(effects.at(ELEMENT)->getSaveAbility() == ability) {
        runEffectEndAndRemoveFromList(i, visionBlockingArray);
        i--;
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
