#ifndef MONSTER_H
#define MONSTER_H

#include "CmnData.h"

#include "Actor.h"
#include "ItemWeapon.h"
#include "Sound.h"
#include "Spells.h"

struct BestAttack {
  BestAttack() : weapon(nullptr), isMelee(true) {}

  Wpn* weapon;
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

  std::vector<Wpn*> weapons;
  bool isTimeToReload;
  bool isMelee;
};

class Wpn;

class Monster: public Actor {
public:
  Monster();
  virtual ~Monster();

  virtual void place_() override {}

  void moveDir(Dir dir);

  AttackOpport getAttackOpport(Actor& defender);
  BestAttack getBestAttack(const AttackOpport& attackOpport);
  bool tryAttack(Actor& defender);

  virtual void mkStartItems() override = 0;

  void hearSound(const Snd& snd);

  void becomeAware(const bool IS_FROM_SEEING);

  void playerBecomeAwareOfMe(const int DURATION_FACTOR = 1);

  void onActorTurn() override;
  virtual bool onActorTurn_() {return false;}
  virtual void onStandardTurn() override {}

  virtual std::string getAggroPhraseMonsterSeen() const {
    return data_->aggroTextMonsterSeen;
  }
  virtual std::string getAggroPhraseMonsterHidden() const {
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

  std::vector<Spell*> spellsKnown;
  int spellCoolDownCur;

  bool isRoamingAllowed_;

  bool isStealth;

  Actor* leader;
  Actor* target;

  bool waiting_;

  void speakPhrase();

  double shockCausedCur_;

  bool hasGivenXpForSpotting_;

protected:
  virtual void hit_(int& dmg, const bool ALLOW_WOUNDS) override;
};

class Rat: public Monster {
public:
  Rat() : Monster() {}
  ~Rat() {}
  virtual void mkStartItems() override;
};

class RatThing: public Rat {
public:
  RatThing() : Rat() {}
  ~RatThing() {}
  void mkStartItems() override;
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
  void mkStartItems() override;
};

class WhiteSpider: public Spider {
public:
  WhiteSpider() : Spider() {}
  ~WhiteSpider() {}
  void mkStartItems() override;
};

class RedSpider: public Spider {
public:
  RedSpider() : Spider() {}
  ~RedSpider() {}
  void mkStartItems() override;
};

class ShadowSpider: public Spider {
public:
  ShadowSpider() : Spider() {}
  ~ShadowSpider() {}
  void mkStartItems() override;
};

class LengSpider: public Spider {
public:
  LengSpider() : Spider() {}
  ~LengSpider() {}
  void mkStartItems() override;
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
  void mkStartItems() override;
};

class ZombieAxe: public Zombie {
public:
  ZombieAxe() : Zombie() {}
  ~ZombieAxe() {}
  void mkStartItems() override;
};

class BloatedZombie: public Zombie {
public:
  BloatedZombie() : Zombie() {}
  ~BloatedZombie() {}

  void mkStartItems() override;
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
  void mkStartItems() override;
private:
  bool hasSummonedJenkin;
};

class LengElder: public Monster {
public:
  LengElder() : Monster() {}
  ~LengElder() {}
  void onStandardTurn() override;
  void mkStartItems()   override;
private:
  bool  hasGivenItemToPlayer_;
  int   nrTurnsToHostile_;
};

class Cultist: public Monster {
public:
  Cultist() : Monster() {}

  virtual void mkStartItems() override;

  static std::string getCultistPhrase();

  std::string getAggroPhraseMonsterSeen() const {
    return getNameThe() + ": " + getCultistPhrase();
  }
  std::string getAggroPhraseMonsterHidden() const {
    return "Voice: " + getCultistPhrase();
  }

  virtual ~Cultist() {}
};

class CultistTeslaCannon: public Cultist {
public:
  CultistTeslaCannon() : Cultist() {}
  ~CultistTeslaCannon() {}
  void mkStartItems() override;
};

class CultistSpikeGun: public Cultist {
public:
  CultistSpikeGun() : Cultist() {}
  ~CultistSpikeGun() {}
  void mkStartItems() override;
};

class CultistPriest: public Cultist {
public:
  CultistPriest() : Cultist() {}
  ~CultistPriest() {}
  void mkStartItems() override;
};

class LordOfShadows: public Monster {
public:
  LordOfShadows() : Monster() {}
  ~LordOfShadows() {}
  bool onActorTurn_() override;
  void mkStartItems() override;
};

class LordOfSpiders: public Monster {
public:
  LordOfSpiders() : Monster() {}
  ~LordOfSpiders() {}
  bool onActorTurn_() override;
  void mkStartItems() override;
};

class LordOfSpirits: public Monster {
public:
  LordOfSpirits() : Monster() {}
  ~LordOfSpirits() {}
  bool onActorTurn_() override;
  void mkStartItems() override;
};

class LordOfPestilence: public Monster {
public:
  LordOfPestilence() : Monster() {}
  ~LordOfPestilence() {}
  bool onActorTurn_() override;
  void mkStartItems() override;
};

class FireHound: public Monster {
public:
  FireHound() : Monster() {}
  ~FireHound() {}
  void mkStartItems() override;
};

class FrostHound: public Monster {
public:
  FrostHound() : Monster() {}
  ~FrostHound() {}
  void mkStartItems() override;
};

class Zuul: public Monster {
public:
  Zuul() : Monster() {}
  ~Zuul() {}

  void place_() override;

  void mkStartItems() override;
};

class Ghost: public Monster {
public:
  Ghost() : Monster() {}
  ~Ghost() {}
  bool onActorTurn_() override;
  virtual void mkStartItems() override;
};

class Phantasm: public Ghost {
public:
  Phantasm() : Ghost() {}
  ~Phantasm() {}
  void mkStartItems() override;
};

class Wraith: public Ghost {
public:
  Wraith() : Ghost() {}
  ~Wraith() {}
  void mkStartItems() override;
};

class GiantBat: public Monster {
public:
  GiantBat() : Monster() {}
  ~GiantBat() {}
  void mkStartItems() override;
};

class Byakhee: public GiantBat {
public:
  Byakhee() : GiantBat() {}
  ~Byakhee() {}
  void mkStartItems() override;
};

class GiantMantis: public Monster {
public:
  GiantMantis() : Monster() {}
  ~GiantMantis() {}
  void mkStartItems() override;
};

class Chthonian: public Monster {
public:
  Chthonian() : Monster() {}
  ~Chthonian() {}
  void mkStartItems() override;
};

class HuntingHorror: public GiantBat {
public:
  HuntingHorror() : GiantBat() {}
  ~HuntingHorror() {}
  void mkStartItems() override;
};

class Wolf: public Monster {
public:
  Wolf() : Monster() {}
  ~Wolf() {}
  void mkStartItems() override;
};

class MiGo: public Monster {
public:
  MiGo() : Monster() {}
  ~MiGo() {}
  void mkStartItems() override;
};

class FlyingPolyp: public Monster {
public:
  FlyingPolyp() : Monster() {}
  ~FlyingPolyp() {}
  void mkStartItems() override;
};

class Ghoul: public Monster {
public:
  Ghoul() : Monster() {}
  ~Ghoul() {}
  virtual void mkStartItems() override;
};

class DeepOne: public Monster {
public:
  DeepOne() : Monster() {}
  ~DeepOne() {}
  void mkStartItems() override;
};

class Mummy: public Monster {
public:
  Mummy() : Monster() {}
  ~Mummy() {}
  virtual void mkStartItems() override;
};

class MummyUnique: public Mummy {
public:
  MummyUnique() : Mummy() {}
  ~MummyUnique() {}
  void mkStartItems() override;
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

  virtual void mkStartItems() override;
};

class WormMass: public Monster {
public:
  WormMass() : Monster(), chanceToSpawnNew(12) {}
  ~WormMass() {}
  bool onActorTurn_() override;
  virtual void mkStartItems() override;
private:
  int chanceToSpawnNew;
};

class GiantLocust: public Monster {
public:
  GiantLocust() : Monster(), chanceToSpawnNew(5) {}
  ~GiantLocust() {}
  bool onActorTurn_() override;
  virtual void mkStartItems() override;
private:
  int chanceToSpawnNew;
};

class Vortex: public Monster {
public:
  Vortex() : Monster(), pullCooldown(0) {}
  virtual ~Vortex() {}

  bool onActorTurn_() override;

  virtual void mkStartItems() = 0;
  virtual void die_() = 0;
private:
  int pullCooldown;
};

class DustVortex: public Vortex {
public:
  DustVortex() : Vortex() {}
  ~DustVortex() {}
  void mkStartItems() override;
  void die_();
};

class FireVortex: public Vortex {
public:
  FireVortex() : Vortex() {}
  ~FireVortex() {}
  void mkStartItems() override;
  void die_();
};

class FrostVortex: public Vortex {
public:
  FrostVortex() : Vortex() {}
  ~FrostVortex() {}
  void mkStartItems() override;
  void die_();
};

class Ooze: public Monster {
public:
  Ooze() : Monster() {}
  ~Ooze() {}
  virtual void onStandardTurn() override;
  virtual void mkStartItems() = 0;
};

class OozeBlack: public Ooze {
public:
  OozeBlack() : Ooze() {}
  ~OozeBlack() {}
  void mkStartItems() override;
};

class OozeClear: public Ooze {
public:
  OozeClear() : Ooze() {}
  ~OozeClear() {}
  void mkStartItems() override;
};

class OozePutrid: public Ooze {
public:
  OozePutrid() : Ooze() {}
  ~OozePutrid() {}
  void mkStartItems() override;
};

class OozePoison: public Ooze {
public:
  OozePoison() : Ooze() {}
  ~OozePoison() {}
  void mkStartItems() override;
};

class ColourOOSpace: public Ooze {
public:
  ColourOOSpace() : Ooze(),
    curColor(clrMagentaLgt) {}
  ~ColourOOSpace() {}
//  bool onActorTurn_() override;
  void onStandardTurn() override;
  void mkStartItems() override;
  const Clr& getClr();
private:
  Clr curColor;
};

#endif
