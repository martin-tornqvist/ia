#ifndef WEAPON_H
#define WEAPON_H

#include "Item.h"
#include "Converters.h"

class Engine;

class Weapon: public Item {
public:
  Weapon(ItemData* const itemData, ItemData* const ammoData, Engine* engine);
  virtual ~Weapon() {}

  int ammoLoaded;
  int effectiveRangeLimit;
  int ammoCapacity;
  bool clip;

  int meleeDmgPlus;

  void setRandomMeleePlus();

  virtual vector<string> itemSpecificWriteToFile() {
    vector<string> lines;
    lines.push_back(intToString(ammoLoaded));
    return lines;
  }

  virtual void itemSpecificReadFromFile(vector<string> lines) {
    ammoLoaded = stringToInt(lines.at(0));
  }

  //actorHit may be NULL
  virtual void weaponSpecific_projectileObstructed(
    const Pos& pos, Actor* actor) {
    (void) pos;
    (void) actor;
  }

  const ItemData& getAmmoData() {
    return *ammoData_;
  }

  void itemSpecificAddSaveLines(vector<string>& lines) {
    lines.push_back(intToString(meleeDmgPlus));
    lines.push_back(intToString(ammoLoaded));
  }

  void itemSpecificSetParametersFromSaveLines(vector<string>& lines) {
    meleeDmgPlus = stringToInt(lines.front());
    lines.erase(lines.begin());
    ammoLoaded = stringToInt(lines.front());
    lines.erase(lines.begin());
  }

  SDL_Color getColor() const {
    if(data_->isRangedWeapon && data_->rangedHasInfiniteAmmo == false) {
      if(ammoLoaded == 0) {
        SDL_Color ret = data_->color;
        ret.r /= 2; ret.g /= 2; ret.b /= 2;
        return ret;
      }
    }
    return data_->color;
  }

  SDL_Color getInterfaceClr() const {return clrGray;}


protected:
  Weapon& operator=(const Weapon& other) {
    (void) other;
    return *this;
  }

  ItemData* const ammoData_;
};

class SawedOff: public Weapon {
public:
  SawedOff(ItemData* const itemData, ItemData* const ammoData,
           Engine* const engine) :
    Weapon(itemData, ammoData, engine) {
    ammoCapacity = 2;
    ammoLoaded = ammoCapacity;
    effectiveRangeLimit = 3;
  }
  ~SawedOff() {}

private:
};

class PumpShotgun: public Weapon {
public:
  PumpShotgun(ItemData* const itemData, ItemData* const ammoData,
              Engine* const engine) :
    Weapon(itemData, ammoData, engine) {
    ammoCapacity = 8;
    ammoLoaded = ammoCapacity;
    effectiveRangeLimit = 3;
  }
  ~PumpShotgun() {}

private:
};

class Pistol: public Weapon {
public:
  Pistol(ItemData* const itemData, ItemData* const ammoData,
         Engine* const engine) :
    Weapon(itemData, ammoData, engine) {
    ammoCapacity = 7;
    ammoLoaded = ammoCapacity;
    effectiveRangeLimit = 6;
  }
  ~Pistol() {}

private:
};

class FlareGun: public Weapon {
public:
  FlareGun(ItemData* const itemData, ItemData* const ammoData,
           Engine* const engine) :
    Weapon(itemData, ammoData, engine) {
    ammoLoaded = 1;
    ammoCapacity = 1;
    effectiveRangeLimit = 6;
  }
  ~FlareGun() {}

private:
};

class MachineGun: public Weapon {
public:
  MachineGun(ItemData* const itemData, ItemData* const ammoData,
             Engine* const engine) :
    Weapon(itemData, ammoData, engine) {
    ammoCapacity = ammoData->ammoContainedInClip;
    ammoLoaded = ammoCapacity;
    effectiveRangeLimit = 8;
    clip = true;
  }
  ~MachineGun() {}

private:
};

class Incinerator: public Weapon {
public:
  Incinerator(ItemData* const itemData, ItemData* const ammoData,
              Engine* const engine) :
    Weapon(itemData, ammoData, engine) {
    ammoCapacity = ammoData->ammoContainedInClip;
    ammoLoaded = ammoCapacity;
    effectiveRangeLimit = 8;
    clip = false;
  }

  void weaponSpecific_projectileObstructed(const Pos& pos, Actor* actorHit);
  ~Incinerator() {}
private:
};

class TeslaCanon: public Weapon {
public:
  TeslaCanon(ItemData* const itemData, ItemData* const ammoData,
             Engine* const engine) :
    Weapon(itemData, ammoData, engine) {
    ammoCapacity = ammoData->ammoContainedInClip;
    ammoLoaded = ammoCapacity;
    effectiveRangeLimit = 8;
    clip = true;
  }
  ~TeslaCanon() {}
private:
};

class SpikeGun: public Weapon {
public:
  SpikeGun(ItemData* const itemData, ItemData* const ammoData,
           Engine* const engine) :
    Weapon(itemData, ammoData, engine) {
    ammoCapacity = 12;
    ammoLoaded = ammoCapacity;
    effectiveRangeLimit = 3;
    clip = true;
  }
  ~SpikeGun() {}
private:
};

#endif
