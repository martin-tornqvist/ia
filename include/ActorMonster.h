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

  virtual void place_() {}

  void moveDir(Dir dir);

  AttackOpport getAttackOpport(Actor& defender);
  BestAttack getBestAttack(const AttackOpport& attackOpport);
  bool tryAttack(Actor& defender);

  virtual void spawnStartItems() = 0;

  void hearSound(const Sound& snd);

  void becomeAware();

  void playerBecomeAwareOfMe(const int DURATION_FACTOR = 1);

  void onActorTurn();

  virtual bool monsterSpecificOnActorTurn() {return false;}

  virtual void onStandardTurn_() {}

  virtual string getAggroPhraseMonsterSeen() const {
    return data_->aggroTextMonsterSeen;
  }
  virtual string getAggroPhraseMonsterHidden() const {
    return data_->aggroTextMonsterHidden;
  }
  virtual SfxId getAggroSfxMonsterSeen() const {
    return data_->aggroSfxMonsterSeen;
  }
  virtual SfxId getAggroSfxMonsterHidden() const {
    return data_->aggroSfxMonsterHidden;
  }

  int awareOfPlayerCounter_;
  int playerAwareOfMeCounter_;

  bool messageMonsterInViewPrinted;

  Dir lastDirTravelled_;

  vector<Spell*> spellsKnown;
  int spellCoolDownCurrent;

  bool isRoamingAllowed_;

  bool isStealth;

  Actor* leader;
  Actor* target;

  bool waiting_;

  void speakPhrase();

  double shockCausedCurrent_;

  bool hasGivenXpForSpotting_;

protected:
  void onMonsterHit(int& dmg);
};

class Rat: public Monster {
public:
  Rat(Engine& engine) : Monster(engine) {}
  ~Rat() {}
  virtual void spawnStartItems() override;
};

class RatThing: public Rat {
public:
  RatThing(Engine& engine) : Rat(engine) {}
  ~RatThing() {}
  void spawnStartItems() override;
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
  void spawnStartItems() override;
};

class WhiteSpider: public Spider {
public:
  WhiteSpider(Engine& engine) : Spider(engine) {}
  ~WhiteSpider() {}
  void spawnStartItems() override;
};

class RedSpider: public Spider {
public:
  RedSpider(Engine& engine) : Spider(engine) {}
  ~RedSpider() {}
  void spawnStartItems() override;
};

class ShadowSpider: public Spider {
public:
  ShadowSpider(Engine& engine) : Spider(engine) {}
  ~ShadowSpider() {}
  void spawnStartItems() override;
};

class LengSpider: public Spider {
public:
  LengSpider(Engine& engine) : Spider(engine) {}
  ~LengSpider() {}
  void spawnStartItems() override;
};

class Zombie: public Monster {
public:
  Zombie(Engine& engine) : Monster(engine) {
    deadTurnCounter = 0;
    hasResurrected = false;
  }
  virtual ~Zombie() {}
  virtual bool monsterSpecificOnActorTurn();
  void die_();
protected:
  bool tryResurrect();
  int deadTurnCounter;
  bool hasResurrected;
};

class ZombieClaw: public Zombie {
public:
  ZombieClaw(Engine& engine) : Zombie(engine) {}
  ~ZombieClaw() {}
  void spawnStartItems() override;
};

class ZombieAxe: public Zombie {
public:
  ZombieAxe(Engine& engine) : Zombie(engine) {}
  ~ZombieAxe() {}
  void spawnStartItems() override;
};

class BloatedZombie: public Zombie {
public:
  BloatedZombie(Engine& engine) : Zombie(engine) {}
  ~BloatedZombie() {}

  void spawnStartItems() override;
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
  void spawnStartItems() override;
private:
  bool hasSummonedJenkin;
};

class Cultist: public Monster {
public:
  Cultist(Engine& engine) : Monster(engine) {}

  virtual void spawnStartItems() override;

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
  void spawnStartItems() override;
};

class CultistSpikeGun: public Cultist {
public:
  CultistSpikeGun(Engine& engine) : Cultist(engine) {}
  ~CultistSpikeGun() {}
  void spawnStartItems() override;
};

class CultistPriest: public Cultist {
public:
  CultistPriest(Engine& engine) : Cultist(engine) {}
  ~CultistPriest() {}
  void spawnStartItems() override;
};

class LordOfShadows: public Monster {
public:
  LordOfShadows(Engine& engine) : Monster(engine) {}
  ~LordOfShadows() {}
  bool monsterSpecificOnActorTurn();
  void spawnStartItems() override;
};

class LordOfSpiders: public Monster {
public:
  LordOfSpiders(Engine& engine) : Monster(engine) {}
  ~LordOfSpiders() {}
  bool monsterSpecificOnActorTurn();
  void spawnStartItems() override;
};

class LordOfSpirits: public Monster {
public:
  LordOfSpirits(Engine& engine) : Monster(engine) {}
  ~LordOfSpirits() {}
  bool monsterSpecificOnActorTurn();
  void spawnStartItems() override;
};

class LordOfPestilence: public Monster {
public:
  LordOfPestilence(Engine& engine) : Monster(engine) {}
  ~LordOfPestilence() {}
  bool monsterSpecificOnActorTurn();
  void spawnStartItems() override;
};

class FireHound: public Monster {
public:
  FireHound(Engine& engine) : Monster(engine) {}
  ~FireHound() {}
  void spawnStartItems() override;
};

class FrostHound: public Monster {
public:
  FrostHound(Engine& engine) : Monster(engine) {}
  ~FrostHound() {}
  void spawnStartItems() override;
};

class Zuul: public Monster {
public:
  Zuul(Engine& engine) : Monster(engine) {}
  ~Zuul() {}

  void place_() override;

  void spawnStartItems() override;
};

class Ghost: public Monster {
public:
  Ghost(Engine& engine) : Monster(engine) {}
  ~Ghost() {}
  bool monsterSpecificOnActorTurn();
  virtual void spawnStartItems() override;
};

class Phantasm: public Ghost {
public:
  Phantasm(Engine& engine) : Ghost(engine) {}
  ~Phantasm() {}
  void spawnStartItems() override;
};

class Wraith: public Ghost {
public:
  Wraith(Engine& engine) : Ghost(engine) {}
  ~Wraith() {}
  void spawnStartItems() override;
};

class GiantBat: public Monster {
public:
  GiantBat(Engine& engine) : Monster(engine) {}
  ~GiantBat() {}
  void spawnStartItems() override;
};

class Byakhee: public GiantBat {
public:
  Byakhee(Engine& engine) : GiantBat(engine) {}
  ~Byakhee() {}
  void spawnStartItems() override;
};

class GiantMantis: public Monster {
public:
  GiantMantis(Engine& engine) : Monster(engine) {}
  ~GiantMantis() {}
  void spawnStartItems() override;
};

class Chthonian: public Monster {
public:
  Chthonian(Engine& engine) : Monster(engine) {}
  ~Chthonian() {}
  void spawnStartItems() override;
};

class HuntingHorror: public GiantBat {
public:
  HuntingHorror(Engine& engine) : GiantBat(engine) {}
  ~HuntingHorror() {}
  void spawnStartItems() override;
};

class Wolf: public Monster {
public:
  Wolf(Engine& engine) : Monster(engine) {}
  ~Wolf() {}
  void spawnStartItems() override;
};

class MiGo: public Monster {
public:
  MiGo(Engine& engine) : Monster(engine) {}
  ~MiGo() {}
  void spawnStartItems() override;
};

class FlyingPolyp: public Monster {
public:
  FlyingPolyp(Engine& engine) : Monster(engine) {}
  ~FlyingPolyp() {}
  void spawnStartItems() override;
};

class Ghoul: public Monster {
public:
  Ghoul(Engine& engine) : Monster(engine) {}
  ~Ghoul() {}
  virtual void spawnStartItems() override;
};

class DeepOne: public Monster {
public:
  DeepOne(Engine& engine) : Monster(engine) {}
  ~DeepOne() {}
  void spawnStartItems() override;
};

class Mummy: public Monster {
public:
  Mummy(Engine& engine) : Monster(engine) {}
  ~Mummy() {}
  virtual void spawnStartItems() override;
};

class MummyUnique: public Mummy {
public:
  MummyUnique(Engine& engine) : Mummy(engine) {}
  ~MummyUnique() {}
  void spawnStartItems() override;
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

  virtual void spawnStartItems() override;
};

class WormMass: public Monster {
public:
  WormMass(Engine& engine) : Monster(engine), chanceToSpawnNew(12) {}
  ~WormMass() {}
  bool monsterSpecificOnActorTurn();
  virtual void spawnStartItems() override;
private:
  int chanceToSpawnNew;
};

class GiantLocust: public Monster {
public:
  GiantLocust(Engine& engine) : Monster(engine), chanceToSpawnNew(5) {}
  ~GiantLocust() {}
  bool monsterSpecificOnActorTurn();
  virtual void spawnStartItems() override;
private:
  int chanceToSpawnNew;
};

class Vortex: public Monster {
public:
  Vortex(Engine& engine) : Monster(engine), pullCooldown(0) {}
  virtual ~Vortex() {}

  bool monsterSpecificOnActorTurn();

  virtual void spawnStartItems() = 0;
  virtual void onMonsterDeath() = 0;
private:
  int pullCooldown;
};

class DustVortex: public Vortex {
public:
  DustVortex(Engine& engine) : Vortex(engine) {}
  ~DustVortex() {}
  void spawnStartItems() override;
  void onMonsterDeath();
};

class FireVortex: public Vortex {
public:
  FireVortex(Engine& engine) : Vortex(engine) {}
  ~FireVortex() {}
  void spawnStartItems() override;
  void onMonsterDeath();
};

class FrostVortex: public Vortex {
public:
  FrostVortex(Engine& engine) : Vortex(engine) {}
  ~FrostVortex() {}
  void spawnStartItems() override;
  void onMonsterDeath();
};

class Ooze: public Monster {
public:
  Ooze(Engine& engine) : Monster(engine) {}
  ~Ooze() {}
  virtual void onStandardTurn_();
  virtual void spawnStartItems() = 0;
};

class OozeBlack: public Ooze {
public:
  OozeBlack(Engine& engine) : Ooze(engine) {}
  ~OozeBlack() {}
  void spawnStartItems() override;
};

class OozeClear: public Ooze {
public:
  OozeClear(Engine& engine) : Ooze(engine) {}
  ~OozeClear() {}
  void spawnStartItems() override;
};

class OozePutrid: public Ooze {
public:
  OozePutrid(Engine& engine) : Ooze(engine) {}
  ~OozePutrid() {}
  void spawnStartItems() override;
};

class OozePoison: public Ooze {
public:
  OozePoison(Engine& engine) : Ooze(engine) {}
  ~OozePoison() {}
  void spawnStartItems() override;
};

class ColourOutOfSpace: public Ooze {
public:
  ColourOutOfSpace(Engine& engine) : Ooze(engine),
    currentColor(clrMagentaLgt) {}
  ~ColourOutOfSpace() {}
//  bool monsterSpecificOnActorTurn();
  void onStandardTurn_();
  void spawnStartItems() override;
  const SDL_Color& getColor();
private:
  SDL_Color currentColor;
};

#endif
