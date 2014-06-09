#ifndef WEAPON_H
#define WEAPON_H

#include "Item.h"
#include "Converters.h"

class Weapon: public Item {
public:
  Weapon(ItemDataT* const itemData, ItemDataT* const ammoData);
  virtual ~Weapon() {}

  int nrAmmoLoaded;
  int effectiveRangeLimit;
  int ammoCapacity;
  bool clip;

  int meleeDmgPlus;

  void setRandomMeleePlus();

  virtual std::vector<std::string> itemSpecificWriteToFile() {
    std::vector<std::string> lines;
    lines.push_back(toStr(nrAmmoLoaded));
    return lines;
  }

  virtual void itemSpecificReadFromFile(std::vector<std::string> lines) {
    nrAmmoLoaded = toInt(lines.at(0));
  }

  //actorHit may be nullptr
  virtual void weaponSpecific_projectileObstructed(
    const Pos& pos, Actor* actor) {
    (void) pos;
    (void) actor;
  }

  const ItemDataT& getAmmoData() {return *ammoData_;}

  void storeToSaveLines(std::vector<std::string>& lines) override {
    lines.push_back(toStr(meleeDmgPlus));
    lines.push_back(toStr(nrAmmoLoaded));
  }

  void setupFromSaveLines(std::vector<std::string>& lines) override {
    meleeDmgPlus = toInt(lines.front());
    lines.erase(lines.begin());
    nrAmmoLoaded = toInt(lines.front());
    lines.erase(lines.begin());
  }

  SDL_Color getClr() const {
    if(!data_->isRangedWeapon && data_->rangedHasInfiniteAmmo) {
      if(nrAmmoLoaded == 0) {
        SDL_Color ret = data_->clr;
        ret.r /= 2; ret.g /= 2; ret.b /= 2;
        return ret;
      }
    }
    return data_->clr;
  }

  SDL_Color getInterfaceClr() const {return clrGray;}


protected:
  Weapon& operator=(const Weapon& other) {
    (void) other;
    return *this;
  }

  ItemDataT* const ammoData_;
};

class SawedOff: public Weapon {
public:
  SawedOff(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Weapon(itemData, ammoData) {
    ammoCapacity = 2;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 3;
  }
  ~SawedOff() {}

private:
};

class PumpShotgun: public Weapon {
public:
  PumpShotgun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Weapon(itemData, ammoData) {
    ammoCapacity = 8;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 3;
  }
  ~PumpShotgun() {}

private:
};

class Pistol: public Weapon {
public:
  Pistol(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Weapon(itemData, ammoData) {
    ammoCapacity = 7;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 6;
  }
  ~Pistol() {}

private:
};

class FlareGun: public Weapon {
public:
  FlareGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Weapon(itemData, ammoData) {
    nrAmmoLoaded = 1;
    ammoCapacity = 1;
    effectiveRangeLimit = 6;
  }
  ~FlareGun() {}

private:
};

class MachineGun: public Weapon {
public:
  MachineGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Weapon(itemData, ammoData) {
    ammoCapacity = ammoData->ammoContainedInClip;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 8;
    clip = true;
  }
  ~MachineGun() {}

private:
};

class Incinerator: public Weapon {
public:
  Incinerator(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Weapon(itemData, ammoData) {
    ammoCapacity = ammoData->ammoContainedInClip;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 8;
    clip = false;
  }

  void weaponSpecific_projectileObstructed(const Pos& pos, Actor* actorHit);
  ~Incinerator() {}
private:
};

class TeslaCannon: public Weapon {
public:
  TeslaCannon(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Weapon(itemData, ammoData) {
    ammoCapacity = ammoData->ammoContainedInClip;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 8;
    clip = true;
  }
  ~TeslaCannon() {}
private:
};

class SpikeGun: public Weapon {
public:
  SpikeGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Weapon(itemData, ammoData) {
    ammoCapacity = 12;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 3;
    clip = true;
  }
  ~SpikeGun() {}
private:
};

#endif
