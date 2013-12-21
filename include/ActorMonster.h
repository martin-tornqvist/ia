#ifndef MONSTER_H
#define MONSTER_H

#include "CommonData.h"

#include "Actor.h"
#include "ItemWeapon.h"
#include "Sound.h"
#include "Spells.h"

struct BestAttack {
  BestAttack() : weapon(NULL), isMelee(true) {}

  Weapon* weapon;
  bool isMelee;
};

struct AttackOpport {
  AttackOpport() :
    isTimeToReload(false), isMelee(true) {
    weapons.resize(0);
  }

  AttackOpport(const AttackOpport& other) :
    weapons(other.weapons), isTimeToReload(other.isTimeToReload),
    isMelee(other.isMelee) {}

  AttackOpport& operator=(const AttackOpport& other) {
    weapons = other.weapons;
    isTimeToReload = other.isTimeToReload;
    isMelee = other.isMelee;

    return *this;
  }

  vector<Weapon*> weapons;
  bool isTimeToReload;
  bool isMelee;
};

class Weapon;

class Monster: public Actor {
public:
  Monster(Engine& engine);
  virtual ~Monster();

  void moveDir(Dir_t dir);

  AttackOpport getAttackOpport(Actor& defender);
  BestAttack getBestAttack(const AttackOpport& attackOpport);
  bool tryAttack(Actor& defender);

  virtual void specificSpawnStartItems() = 0;

  void hearSound(const Sound& snd);

  void becomeAware();

  void onActorTurn();

  virtual bool monsterSpecificOnActorTurn() {return false;}

  virtual void specificOnStandardTurn() {}

  int playerAwarenessCounter;

  bool messageMonsterInViewPrinted;

  Dir_t lastDirTraveled;

  vector<Spell*> spellsKnown;
  int spellCoolDownCurrent;

  bool isRoamingAllowed;

  bool isStealth;

  Actor* leader;
  Actor* target;

  bool waiting_;

  virtual string getAggroPhraseMonsterSeen() const {
    return data_->aggroTextMonsterSeen;
  }
  virtual string getAggroPhraseMonsterHidden() const {
    return data_->aggroTextMonsterHidden;
  }
  virtual Sfx_t getAggroSfxMonsterSeen() const {
    return data_->aggroSfxMonsterSeen;
  }
  virtual Sfx_t getAggroSfxMonsterHidden() const {
    return data_->aggroSfxMonsterHidden;
  }

  double shockCausedCurrent;

  void speakPhrase();

protected:
  void onMonsterHit(int& dmg);
};

class Rat: public Monster {
public:
  Rat(Engine& engine) : Monster(engine) {}
  ~Rat() {}
  virtual void specificSpawnStartItems();
};

class RatThing: public Rat {
public:
  RatThing(Engine& engine) : Rat(engine) {}
  ~RatThing() {}
  void specificSpawnStartItems();
};

class BrownJenkin: public RatThing {
public:
  BrownJenkin(Engine& engine) : RatThing(engine) {}
  ~BrownJenkin() {}
};

class Spider: public Monster {
public:
  Spider(Engine& engine) : Monster(engine) {}
  virtual ~Spider() {}
  bool monsterSpecificOnActorTurn();
};

class GreenSpider: public Spider {
public:
  GreenSpider(Engine& engine) : Spider(engine) {}
  ~GreenSpider() {}
  void specificSpawnStartItems();
};

class WhiteSpider: public Spider {
public:
  WhiteSpider(Engine& engine) : Spider(engine) {}
  ~WhiteSpider() {}
  void specificSpawnStartItems();
};

class RedSpider: public Spider {
public:
  RedSpider(Engine& engine) : Spider(engine) {}
  ~RedSpider() {}
  void specificSpawnStartItems();
};

class ShadowSpider: public Spider {
public:
  ShadowSpider(Engine& engine) : Spider(engine) {}
  ~ShadowSpider() {}
  void specificSpawnStartItems();
};

class LengSpider: public Spider {
public:
  LengSpider(Engine& engine) : Spider(engine) {}
  ~LengSpider() {}
  void specificSpawnStartItems();
};

class Zombie: public Monster {
public:
  Zombie(Engine& engine) : Monster(engine) {
    deadTurnCounter = 0;
    hasResurrected = false;
  }
  virtual ~Zombie() {}
  virtual bool monsterSpecificOnActorTurn();
  void specificDie();
protected:
  bool tryResurrect();
  int deadTurnCounter;
  bool hasResurrected;
};

class ZombieClaw: public Zombie {
public:
  ZombieClaw(Engine& engine) : Zombie(engine) {}
  ~ZombieClaw() {}
  void specificSpawnStartItems();
};

class ZombieAxe: public Zombie {
public:
  ZombieAxe(Engine& engine) : Zombie(engine) {}
  ~ZombieAxe() {}
  void specificSpawnStartItems();
};

class BloatedZombie: public Zombie {
public:
  BloatedZombie(Engine& engine) : Zombie(engine) {}
  ~BloatedZombie() {}

  void specificSpawnStartItems();
};

class MajorClaphamLee: public ZombieClaw {
public:
  MajorClaphamLee(Engine& engine) :
    ZombieClaw(engine), hasSummonedTombLegions(false) {
  }
  ~MajorClaphamLee() {}

  bool monsterSpecificOnActorTurn();
private:
  bool hasSummonedTombLegions;
};

class DeanHalsey: public ZombieClaw {
public:
  DeanHalsey(Engine& engine) : ZombieClaw(engine) {}
  ~DeanHalsey() {}
};

class KeziahMason: public Monster {
public:
  KeziahMason(Engine& engine) : Monster(engine), hasSummonedJenkin(false) {}
  ~KeziahMason() {}
  bool monsterSpecificOnActorTurn();
  void specificSpawnStartItems();
private:
  bool hasSummonedJenkin;
};

class Cultist: public Monster {
public:
  Cultist(Engine& engine) : Monster(engine) {}

  virtual void specificSpawnStartItems();

  static string getCultistPhrase(Engine& engine);

  string getAggroPhraseMonsterSeen() const {
    return getNameThe() + ": " + getCultistPhrase(eng);
  }
  string getAggroPhraseMonsterHidden() const {
    return "Voice: " + getCultistPhrase(eng);
  }

  virtual ~Cultist() {}
};

class CultistTeslaCannon: public Cultist {
public:
  CultistTeslaCannon(Engine& engine) : Cultist(engine) {}
  ~CultistTeslaCannon() {}
  void specificSpawnStartItems();
};

class CultistSpikeGun: public Cultist {
public:
  CultistSpikeGun(Engine& engine) : Cultist(engine) {}
  ~CultistSpikeGun() {}
  void specificSpawnStartItems();
};

class CultistPriest: public Cultist {
public:
  CultistPriest(Engine& engine) : Cultist(engine) {}
  ~CultistPriest() {}
  void specificSpawnStartItems();
};

class LordOfShadows: public Monster {
public:
  LordOfShadows(Engine& engine) : Monster(engine) {}
  ~LordOfShadows() {}
  bool monsterSpecificOnActorTurn();
  void specificSpawnStartItems();
};

class LordOfSpiders: public Monster {
public:
  LordOfSpiders(Engine& engine) : Monster(engine) {}
  ~LordOfSpiders() {}
  bool monsterSpecificOnActorTurn();
  void specificSpawnStartItems();
};

class LordOfSpirits: public Monster {
public:
  LordOfSpirits(Engine& engine) : Monster(engine) {}
  ~LordOfSpirits() {}
  bool monsterSpecificOnActorTurn();
  void specificSpawnStartItems();
};

class LordOfPestilence: public Monster {
public:
  LordOfPestilence(Engine& engine) : Monster(engine) {}
  ~LordOfPestilence() {}
  bool monsterSpecificOnActorTurn();
  void specificSpawnStartItems();
};

class FireHound: public Monster {
public:
  FireHound(Engine& engine) : Monster(engine) {}
  ~FireHound() {}
  void specificSpawnStartItems();
};

class FrostHound: public Monster {
public:
  FrostHound(Engine& engine) : Monster(engine) {}
  ~FrostHound() {}
  void specificSpawnStartItems();
};

class Ghost: public Monster {
public:
  Ghost(Engine& engine) : Monster(engine) {}
  ~Ghost() {}
  bool monsterSpecificOnActorTurn();
  virtual void specificSpawnStartItems();
};

class Phantasm: public Ghost {
public:
  Phantasm(Engine& engine) : Ghost(engine) {}
  ~Phantasm() {}
  void specificSpawnStartItems();
};

class Wraith: public Ghost {
public:
  Wraith(Engine& engine) : Ghost(engine) {}
  ~Wraith() {}
  void specificSpawnStartItems();
};

class GiantBat: public Monster {
public:
  GiantBat(Engine& engine) : Monster(engine) {}
  ~GiantBat() {}
  void specificSpawnStartItems();
};

class Byakhee: public GiantBat {
public:
  Byakhee(Engine& engine) : GiantBat(engine) {}
  ~Byakhee() {}
  void specificSpawnStartItems();
};

class GiantMantis: public Monster {
public:
  GiantMantis(Engine& engine) : Monster(engine) {}
  ~GiantMantis() {}
  void specificSpawnStartItems();
};

class HuntingHorror: public GiantBat {
public:
  HuntingHorror(Engine& engine) : GiantBat(engine) {}
  ~HuntingHorror() {}
  void specificSpawnStartItems();
};

class Wolf: public Monster {
public:
  Wolf(Engine& engine) : Monster(engine) {}
  ~Wolf() {}
  void specificSpawnStartItems();
};

class MiGo: public Monster {
public:
  MiGo(Engine& engine) : Monster(engine) {}
  ~MiGo() {}
  void specificSpawnStartItems();
};

class Ghoul: public Monster {
public:
  Ghoul(Engine& engine) : Monster(engine) {}
  ~Ghoul() {}
  virtual void specificSpawnStartItems();
};

class DeepOne: public Monster {
public:
  DeepOne(Engine& engine) : Monster(engine) {}
  ~DeepOne() {}
  void specificSpawnStartItems();
};

class Mummy: public Monster {
public:
  Mummy(Engine& engine) : Monster(engine) {}
  ~Mummy() {}
  virtual void specificSpawnStartItems();
};

class MummyUnique: public Mummy {
public:
  MummyUnique(Engine& engine) : Mummy(engine) {}
  ~MummyUnique() {}
  void specificSpawnStartItems();
};

class Khephren: public MummyUnique {
public:
  Khephren(Engine& engine) : MummyUnique(engine) {}
  ~Khephren() {}

  bool monsterSpecificOnActorTurn();
private:
  bool hasSummonedLocusts;
};

class Shadow: public Monster {
public:
  Shadow(Engine& engine) : Monster(engine) {}
  ~Shadow() {}

  virtual void specificSpawnStartItems();
};

class WormMass: public Monster {
public:
  WormMass(Engine& engine) : Monster(engine), chanceToSpawnNew(12) {}
  ~WormMass() {}
  bool monsterSpecificOnActorTurn();
  virtual void specificSpawnStartItems();
private:
  int chanceToSpawnNew;
};

class GiantLocust: public Monster {
public:
  GiantLocust(Engine& engine) : Monster(engine), chanceToSpawnNew(5) {}
  ~GiantLocust() {}
  bool monsterSpecificOnActorTurn();
  virtual void specificSpawnStartItems();
private:
  int chanceToSpawnNew;
};

class Vortex: public Monster {
public:
  Vortex(Engine& engine) : Monster(engine), pullCooldown(0) {}
  virtual ~Vortex() {}

  bool monsterSpecificOnActorTurn();

  virtual void specificSpawnStartItems() = 0;
  virtual void onMonsterDeath() = 0;
private:
  int pullCooldown;
};

class DustVortex: public Vortex {
public:
  DustVortex(Engine& engine) : Vortex(engine) {}
  ~DustVortex() {}
  void specificSpawnStartItems();
  void onMonsterDeath();
};

class FireVortex: public Vortex {
public:
  FireVortex(Engine& engine) : Vortex(engine) {}
  ~FireVortex() {}
  void specificSpawnStartItems();
  void onMonsterDeath();
};

class FrostVortex: public Vortex {
public:
  FrostVortex(Engine& engine) : Vortex(engine) {}
  ~FrostVortex() {}
  void specificSpawnStartItems();
  void onMonsterDeath();
};

class Ooze: public Monster {
public:
  Ooze(Engine& engine) : Monster(engine) {}
  ~Ooze() {}
  virtual void specificOnStandardTurn();
  virtual void specificSpawnStartItems() = 0;
};

class OozeBlack: public Ooze {
public:
  OozeBlack(Engine& engine) : Ooze(engine) {}
  ~OozeBlack() {}
  void specificSpawnStartItems();
};

class OozeClear: public Ooze {
public:
  OozeClear(Engine& engine) : Ooze(engine) {}
  ~OozeClear() {}
  void specificSpawnStartItems();
};

class OozePutrid: public Ooze {
public:
  OozePutrid(Engine& engine) : Ooze(engine) {}
  ~OozePutrid() {}
  void specificSpawnStartItems();
};

class OozePoison: public Ooze {
public:
  OozePoison(Engine& engine) : Ooze(engine) {}
  ~OozePoison() {}
  void specificSpawnStartItems();
};

class ColourOutOfSpace: public Ooze {
public:
  ColourOutOfSpace(Engine& engine) : Ooze(engine),
    currentColor(clrMagentaLgt) {}
  ~ColourOutOfSpace() {}
//  bool monsterSpecificOnActorTurn();
  void specificOnStandardTurn();
  void specificSpawnStartItems();
  const SDL_Color& getColor();
private:
  SDL_Color currentColor;
};

#endif
