#ifndef WEAPON_H
#define WEAPON_H

#include "Item.h"
#include "Converters.h"

class Wpn: public Item {
public:
  Wpn(ItemDataT* const itemData, ItemDataT* const ammoData);
  virtual ~Wpn() {}

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
  virtual void projectileObstructed(const Pos& pos, Actor* actor) {
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
    lines.erase(begin(lines));
    nrAmmoLoaded = toInt(lines.front());
    lines.erase(begin(lines));
  }

  Clr getClr() const override {
    if(data_->ranged.isRangedWpn && !data_->ranged.hasInfiniteAmmo) {
      if(nrAmmoLoaded == 0) {
        Clr ret = data_->clr;
        ret.r /= 2; ret.g /= 2; ret.b /= 2;
        return ret;
      }
    }
    return data_->clr;
  }

  Clr getInterfaceClr() const override {return clrGray;}


protected:
  Wpn& operator=(const Wpn& other) {
    (void) other;
    return *this;
  }

  ItemDataT* const ammoData_;
};

class SawedOff: public Wpn {
public:
  SawedOff(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData) {
    ammoCapacity = 2;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 3;
  }
  ~SawedOff() {}
};

class PumpShotgun: public Wpn {
public:
  PumpShotgun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData) {
    ammoCapacity = 8;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 3;
  }
  ~PumpShotgun() {}

private:
};

class Pistol: public Wpn {
public:
  Pistol(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData) {
    ammoCapacity = 7;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 6;
  }
  ~Pistol() {}

private:
};

class FlareGun: public Wpn {
public:
  FlareGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData) {
    nrAmmoLoaded = 1;
    ammoCapacity = 1;
    effectiveRangeLimit = 6;
  }
  ~FlareGun() {}

private:
};

class MachineGun: public Wpn {
public:
  MachineGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData) {
    ammoCapacity = ammoData->ranged.ammoContainedInClip;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 8;
    clip = true;
  }
  ~MachineGun() {}

private:
};

class Incinerator: public Wpn {
public:
  Incinerator(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData) {
    ammoCapacity = ammoData->ranged.ammoContainedInClip;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 8;
    clip = false;
  }

  void projectileObstructed(const Pos& pos, Actor* actorHit);
  ~Incinerator() {}
private:
};

class TeslaCannon: public Wpn {
public:
  TeslaCannon(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData) {
    ammoCapacity = ammoData->ranged.ammoContainedInClip;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 8;
    clip = true;
  }
  ~TeslaCannon() {}
private:
};

class SpikeGun: public Wpn {
public:
  SpikeGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData) {
    ammoCapacity = 12;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLimit = 3;
    clip = true;
  }
  ~SpikeGun() {}
private:
};

#endif
