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

  virtual void specificPlace() {}

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
  virtual void specificSpawnStartItems() override;
};

class RatThing: public Rat {
public:
  RatThing(Engine& engine) : Rat(engine) {}
  ~RatThing() {}
  void specificSpawnStartItems() override;
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
  void specificSpawnStartItems() override;
};

class WhiteSpider: public Spider {
public:
  WhiteSpider(Engine& engine) : Spider(engine) {}
  ~WhiteSpider() {}
  void specificSpawnStartItems() override;
};

class RedSpider: public Spider {
public:
  RedSpider(Engine& engine) : Spider(engine) {}
  ~RedSpider() {}
  void specificSpawnStartItems() override;
};

class ShadowSpider: public Spider {
public:
  ShadowSpider(Engine& engine) : Spider(engine) {}
  ~ShadowSpider() {}
  void specificSpawnStartItems() override;
};

class LengSpider: public Spider {
public:
  LengSpider(Engine& engine) : Spider(engine) {}
  ~LengSpider() {}
  void specificSpawnStartItems() override;
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
  void specificSpawnStartItems() override;
};

class ZombieAxe: public Zombie {
public:
  ZombieAxe(Engine& engine) : Zombie(engine) {}
  ~ZombieAxe() {}
  void specificSpawnStartItems() override;
};

class BloatedZombie: public Zombie {
public:
  BloatedZombie(Engine& engine) : Zombie(engine) {}
  ~BloatedZombie() {}

  void specificSpawnStartItems() override;
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
  void specificSpawnStartItems() override;
private:
  bool hasSummonedJenkin;
};

class Cultist: public Monster {
public:
  Cultist(Engine& engine) : Monster(engine) {}

  virtual void specificSpawnStartItems() override;

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
  void specificSpawnStartItems() override;
};

class CultistSpikeGun: public Cultist {
public:
  CultistSpikeGun(Engine& engine) : Cultist(engine) {}
  ~CultistSpikeGun() {}
  void specificSpawnStartItems() override;
};

class CultistPriest: public Cultist {
public:
  CultistPriest(Engine& engine) : Cultist(engine) {}
  ~CultistPriest() {}
  void specificSpawnStartItems() override;
};

class LordOfShadows: public Monster {
public:
  LordOfShadows(Engine& engine) : Monster(engine) {}
  ~LordOfShadows() {}
  bool monsterSpecificOnActorTurn();
  void specificSpawnStartItems() override;
};

class LordOfSpiders: public Monster {
public:
  LordOfSpiders(Engine& engine) : Monster(engine) {}
  ~LordOfSpiders() {}
  bool monsterSpecificOnActorTurn();
  void specificSpawnStartItems() override;
};

class LordOfSpirits: public Monster {
public:
  LordOfSpirits(Engine& engine) : Monster(engine) {}
  ~LordOfSpirits() {}
  bool monsterSpecificOnActorTurn();
  void specificSpawnStartItems() override;
};

class LordOfPestilence: public Monster {
public:
  LordOfPestilence(Engine& engine) : Monster(engine) {}
  ~LordOfPestilence() {}
  bool monsterSpecificOnActorTurn();
  void specificSpawnStartItems() override;
};

class FireHound: public Monster {
public:
  FireHound(Engine& engine) : Monster(engine) {}
  ~FireHound() {}
  void specificSpawnStartItems() override;
};

class FrostHound: public Monster {
public:
  FrostHound(Engine& engine) : Monster(engine) {}
  ~FrostHound() {}
  void specificSpawnStartItems() override;
};

class Zuul: public Monster {
public:
  Zuul(Engine& engine) : Monster(engine) {}
  ~Zuul() {}

  void specificPlace() override;

  void specificSpawnStartItems() override;
};

class Ghost: public Monster {
public:
  Ghost(Engine& engine) : Monster(engine) {}
  ~Ghost() {}
  bool monsterSpecificOnActorTurn();
  virtual void specificSpawnStartItems() override;
};

class Phantasm: public Ghost {
public:
  Phantasm(Engine& engine) : Ghost(engine) {}
  ~Phantasm() {}
  void specificSpawnStartItems() override;
};

class Wraith: public Ghost {
public:
  Wraith(Engine& engine) : Ghost(engine) {}
  ~Wraith() {}
  void specificSpawnStartItems() override;
};

class GiantBat: public Monster {
public:
  GiantBat(Engine& engine) : Monster(engine) {}
  ~GiantBat() {}
  void specificSpawnStartItems() override;
};

class Byakhee: public GiantBat {
public:
  Byakhee(Engine& engine) : GiantBat(engine) {}
  ~Byakhee() {}
  void specificSpawnStartItems() override;
};

class GiantMantis: public Monster {
public:
  GiantMantis(Engine& engine) : Monster(engine) {}
  ~GiantMantis() {}
  void specificSpawnStartItems() override;
};

class HuntingHorror: public GiantBat {
public:
  HuntingHorror(Engine& engine) : GiantBat(engine) {}
  ~HuntingHorror() {}
  void specificSpawnStartItems() override;
};

class Wolf: public Monster {
public:
  Wolf(Engine& engine) : Monster(engine) {}
  ~Wolf() {}
  void specificSpawnStartItems() override;
};

class MiGo: public Monster {
public:
  MiGo(Engine& engine) : Monster(engine) {}
  ~MiGo() {}
  void specificSpawnStartItems() override;
};

class Ghoul: public Monster {
public:
  Ghoul(Engine& engine) : Monster(engine) {}
  ~Ghoul() {}
  virtual void specificSpawnStartItems() override;
};

class DeepOne: public Monster {
public:
  DeepOne(Engine& engine) : Monster(engine) {}
  ~DeepOne() {}
  void specificSpawnStartItems() override;
};

class Mummy: public Monster {
public:
  Mummy(Engine& engine) : Monster(engine) {}
  ~Mummy() {}
  virtual void specificSpawnStartItems() override;
};

class MummyUnique: public Mummy {
public:
  MummyUnique(Engine& engine) : Mummy(engine) {}
  ~MummyUnique() {}
  void specificSpawnStartItems() override;
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

  virtual void specificSpawnStartItems() override;
};

class WormMass: public Monster {
public:
  WormMass(Engine& engine) : Monster(engine), chanceToSpawnNew(12) {}
  ~WormMass() {}
  bool monsterSpecificOnActorTurn();
  virtual void specificSpawnStartItems() override;
private:
  int chanceToSpawnNew;
};

class GiantLocust: public Monster {
public:
  GiantLocust(Engine& engine) : Monster(engine), chanceToSpawnNew(5) {}
  ~GiantLocust() {}
  bool monsterSpecificOnActorTurn();
  virtual void specificSpawnStartItems() override;
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
  void specificSpawnStartItems() override;
  void onMonsterDeath();
};

class FireVortex: public Vortex {
public:
  FireVortex(Engine& engine) : Vortex(engine) {}
  ~FireVortex() {}
  void specificSpawnStartItems() override;
  void onMonsterDeath();
};

class FrostVortex: public Vortex {
public:
  FrostVortex(Engine& engine) : Vortex(engine) {}
  ~FrostVortex() {}
  void specificSpawnStartItems() override;
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
  void specificSpawnStartItems() override;
};

class OozeClear: public Ooze {
public:
  OozeClear(Engine& engine) : Ooze(engine) {}
  ~OozeClear() {}
  void specificSpawnStartItems() override;
};

class OozePutrid: public Ooze {
public:
  OozePutrid(Engine& engine) : Ooze(engine) {}
  ~OozePutrid() {}
  void specificSpawnStartItems() override;
};

class OozePoison: public Ooze {
public:
  OozePoison(Engine& engine) : Ooze(engine) {}
  ~OozePoison() {}
  void specificSpawnStartItems() override;
};

class ColourOutOfSpace: public Ooze {
public:
  ColourOutOfSpace(Engine& engine) : Ooze(engine),
    currentColor(clrMagentaLgt) {}
  ~ColourOutOfSpace() {}
//  bool monsterSpecificOnActorTurn();
  void specificOnStandardTurn();
  void specificSpawnStartItems() override;
  const SDL_Color& getColor();
private:
  SDL_Color currentColor;
};

#endif
