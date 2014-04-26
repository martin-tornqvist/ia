#ifndef MONSTER_H
#define MONSTER_H

#include "CmnData.h"

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
  Monster();
  virtual ~Monster();

  virtual void place_() override {}

  void moveDir(Dir dir);

  AttackOpport getAttackOpport(Actor& defender);
  BestAttack getBestAttack(const AttackOpport& attackOpport);
  bool tryAttack(Actor& defender);

  virtual void spawnStartItems() override = 0;

  void hearSound(const Snd& snd);

  void becomeAware(const bool IS_FROM_SEEING);

  void playerBecomeAwareOfMe(const int DURATION_FACTOR = 1);

  void onActorTurn() override;
  virtual bool onActorTurn_() {return false;}
  virtual void onStandardTurn() override {}

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
  virtual void hit_(int& dmg, const bool ALLOW_WOUNDS) override;
};

class Rat: public Monster {
public:
  Rat() : Monster() {}
  ~Rat() {}
  virtual void spawnStartItems() override;
};

class RatThing: public Rat {
public:
  RatThing() : Rat() {}
  ~RatThing() {}
  void spawnStartItems() override;
};

class BrownJenkin: public RatThing {
public:
  BrownJenkin() : RatThing() {}
  ~BrownJenkin() {}
};

class Spider: public Monster {
public:
  Spider() : Monster() {}
  virtual ~Spider() {}
  bool onActorTurn_() override;
};

class GreenSpider: public Spider {
public:
  GreenSpider() : Spider() {}
  ~GreenSpider() {}
  void spawnStartItems() override;
};

class WhiteSpider: public Spider {
public:
  WhiteSpider() : Spider() {}
  ~WhiteSpider() {}
  void spawnStartItems() override;
};

class RedSpider: public Spider {
public:
  RedSpider() : Spider() {}
  ~RedSpider() {}
  void spawnStartItems() override;
};

class ShadowSpider: public Spider {
public:
  ShadowSpider() : Spider() {}
  ~ShadowSpider() {}
  void spawnStartItems() override;
};

class LengSpider: public Spider {
public:
  LengSpider() : Spider() {}
  ~LengSpider() {}
  void spawnStartItems() override;
};

class Zombie: public Monster {
public:
  Zombie() : Monster() {
    deadTurnCounter = 0;
    hasResurrected = false;
  }
  virtual ~Zombie() {}
  virtual bool onActorTurn_() override;
  void die_() override;
protected:
  bool tryResurrect();
  int deadTurnCounter;
  bool hasResurrected;
};

class ZombieClaw: public Zombie {
public:
  ZombieClaw() : Zombie() {}
  ~ZombieClaw() {}
  void spawnStartItems() override;
};

class ZombieAxe: public Zombie {
public:
  ZombieAxe() : Zombie() {}
  ~ZombieAxe() {}
  void spawnStartItems() override;
};

class BloatedZombie: public Zombie {
public:
  BloatedZombie() : Zombie() {}
  ~BloatedZombie() {}

  void spawnStartItems() override;
};

class MajorClaphamLee: public ZombieClaw {
public:
  MajorClaphamLee() :
    ZombieClaw(), hasSummonedTombLegions(false) {
  }
  ~MajorClaphamLee() {}

  bool onActorTurn_() override;
private:
  bool hasSummonedTombLegions;
};

class DeanHalsey: public ZombieClaw {
public:
  DeanHalsey() : ZombieClaw() {}
  ~DeanHalsey() {}
};

class KeziahMason: public Monster {
public:
  KeziahMason() : Monster(), hasSummonedJenkin(false) {}
  ~KeziahMason() {}
  bool onActorTurn_() override;
  void spawnStartItems() override;
private:
  bool hasSummonedJenkin;
};

class Cultist: public Monster {
public:
  Cultist() : Monster() {}

  virtual void spawnStartItems() override;

  static string getCultistPhrase();

  string getAggroPhraseMonsterSeen() const {
    return getNameThe() + ": " + getCultistPhrase();
  }
  string getAggroPhraseMonsterHidden() const {
    return "Voice: " + getCultistPhrase();
  }

  virtual ~Cultist() {}
};

class CultistTeslaCannon: public Cultist {
public:
  CultistTeslaCannon() : Cultist() {}
  ~CultistTeslaCannon() {}
  void spawnStartItems() override;
};

class CultistSpikeGun: public Cultist {
public:
  CultistSpikeGun() : Cultist() {}
  ~CultistSpikeGun() {}
  void spawnStartItems() override;
};

class CultistPriest: public Cultist {
public:
  CultistPriest() : Cultist() {}
  ~CultistPriest() {}
  void spawnStartItems() override;
};

class LordOfShadows: public Monster {
public:
  LordOfShadows() : Monster() {}
  ~LordOfShadows() {}
  bool onActorTurn_() override;
  void spawnStartItems() override;
};

class LordOfSpiders: public Monster {
public:
  LordOfSpiders() : Monster() {}
  ~LordOfSpiders() {}
  bool onActorTurn_() override;
  void spawnStartItems() override;
};

class LordOfSpirits: public Monster {
public:
  LordOfSpirits() : Monster() {}
  ~LordOfSpirits() {}
  bool onActorTurn_() override;
  void spawnStartItems() override;
};

class LordOfPestilence: public Monster {
public:
  LordOfPestilence() : Monster() {}
  ~LordOfPestilence() {}
  bool onActorTurn_() override;
  void spawnStartItems() override;
};

class FireHound: public Monster {
public:
  FireHound() : Monster() {}
  ~FireHound() {}
  void spawnStartItems() override;
};

class FrostHound: public Monster {
public:
  FrostHound() : Monster() {}
  ~FrostHound() {}
  void spawnStartItems() override;
};

class Zuul: public Monster {
public:
  Zuul() : Monster() {}
  ~Zuul() {}

  void place_() override;

  void spawnStartItems() override;
};

class Ghost: public Monster {
public:
  Ghost() : Monster() {}
  ~Ghost() {}
  bool onActorTurn_() override;
  virtual void spawnStartItems() override;
};

class Phantasm: public Ghost {
public:
  Phantasm() : Ghost() {}
  ~Phantasm() {}
  void spawnStartItems() override;
};

class Wraith: public Ghost {
public:
  Wraith() : Ghost() {}
  ~Wraith() {}
  void spawnStartItems() override;
};

class GiantBat: public Monster {
public:
  GiantBat() : Monster() {}
  ~GiantBat() {}
  void spawnStartItems() override;
};

class Byakhee: public GiantBat {
public:
  Byakhee() : GiantBat() {}
  ~Byakhee() {}
  void spawnStartItems() override;
};

class GiantMantis: public Monster {
public:
  GiantMantis() : Monster() {}
  ~GiantMantis() {}
  void spawnStartItems() override;
};

class Chthonian: public Monster {
public:
  Chthonian() : Monster() {}
  ~Chthonian() {}
  void spawnStartItems() override;
};

class HuntingHorror: public GiantBat {
public:
  HuntingHorror() : GiantBat() {}
  ~HuntingHorror() {}
  void spawnStartItems() override;
};

class Wolf: public Monster {
public:
  Wolf() : Monster() {}
  ~Wolf() {}
  void spawnStartItems() override;
};

class MiGo: public Monster {
public:
  MiGo() : Monster() {}
  ~MiGo() {}
  void spawnStartItems() override;
};

class FlyingPolyp: public Monster {
public:
  FlyingPolyp() : Monster() {}
  ~FlyingPolyp() {}
  void spawnStartItems() override;
};

class Ghoul: public Monster {
public:
  Ghoul() : Monster() {}
  ~Ghoul() {}
  virtual void spawnStartItems() override;
};

class DeepOne: public Monster {
public:
  DeepOne() : Monster() {}
  ~DeepOne() {}
  void spawnStartItems() override;
};

class Mummy: public Monster {
public:
  Mummy() : Monster() {}
  ~Mummy() {}
  virtual void spawnStartItems() override;
};

class MummyUnique: public Mummy {
public:
  MummyUnique() : Mummy() {}
  ~MummyUnique() {}
  void spawnStartItems() override;
};

class Khephren: public MummyUnique {
public:
  Khephren() : MummyUnique() {}
  ~Khephren() {}

  bool onActorTurn_() override;
private:
  bool hasSummonedLocusts;
};

class Shadow: public Monster {
public:
  Shadow() : Monster() {}
  ~Shadow() {}

  virtual void spawnStartItems() override;
};

class WormMass: public Monster {
public:
  WormMass() : Monster(), chanceToSpawnNew(12) {}
  ~WormMass() {}
  bool onActorTurn_() override;
  virtual void spawnStartItems() override;
private:
  int chanceToSpawnNew;
};

class GiantLocust: public Monster {
public:
  GiantLocust() : Monster(), chanceToSpawnNew(5) {}
  ~GiantLocust() {}
  bool onActorTurn_() override;
  virtual void spawnStartItems() override;
private:
  int chanceToSpawnNew;
};

class Vortex: public Monster {
public:
  Vortex() : Monster(), pullCooldown(0) {}
  virtual ~Vortex() {}

  bool onActorTurn_() override;

  virtual void spawnStartItems() = 0;
  virtual void die_() = 0;
private:
  int pullCooldown;
};

class DustVortex: public Vortex {
public:
  DustVortex() : Vortex() {}
  ~DustVortex() {}
  void spawnStartItems() override;
  void die_();
};

class FireVortex: public Vortex {
public:
  FireVortex() : Vortex() {}
  ~FireVortex() {}
  void spawnStartItems() override;
  void die_();
};

class FrostVortex: public Vortex {
public:
  FrostVortex() : Vortex() {}
  ~FrostVortex() {}
  void spawnStartItems() override;
  void die_();
};

class Ooze: public Monster {
public:
  Ooze() : Monster() {}
  ~Ooze() {}
  virtual void onStandardTurn() override;
  virtual void spawnStartItems() = 0;
};

class OozeBlack: public Ooze {
public:
  OozeBlack() : Ooze() {}
  ~OozeBlack() {}
  void spawnStartItems() override;
};

class OozeClear: public Ooze {
public:
  OozeClear() : Ooze() {}
  ~OozeClear() {}
  void spawnStartItems() override;
};

class OozePutrid: public Ooze {
public:
  OozePutrid() : Ooze() {}
  ~OozePutrid() {}
  void spawnStartItems() override;
};

class OozePoison: public Ooze {
public:
  OozePoison() : Ooze() {}
  ~OozePoison() {}
  void spawnStartItems() override;
};

class ColourOOSpace: public Ooze {
public:
  ColourOOSpace() : Ooze(),
    currentColor(clrMagentaLgt) {}
  ~ColourOOSpace() {}
//  bool onActorTurn_() override;
  void onStandardTurn() override;
  void spawnStartItems() override;
  const SDL_Color& getClr();
private:
  SDL_Color currentColor;
};

#endif
