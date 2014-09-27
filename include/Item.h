#ifndef ITEM_H
#define ITEM_H

#include <iostream>

#include "Art.h"
#include "InventoryHandling.h"
#include "Converters.h"

class ItemDataT;
class Prop;
class Actor;

enum ItemActivateReturnType {
  itemActivate_keep, itemActivate_destroyed
};

class Item {
public:
  Item(ItemDataT* itemData);

  Item& operator=(Item& other) = delete;

  virtual ~Item() {}

  const ItemDataT&  getData()   const;
  virtual Clr       getClr()    const;
  char              getGlyph()  const;
  TileId            getTile()   const;

  std::string getName(const ItemRefType refType, const ItemRefInf inf = ItemRefInf::yes,
                      const ItemRefAttInf attInf = ItemRefAttInf::none) const;

  virtual std::vector<std::string> getDescr() const;

  virtual void identify(const bool IS_SILENT_IDENTIFY) {(void)IS_SILENT_IDENTIFY;}

  virtual void storeToSaveLines(std::vector<std::string>& lines)    {(void)lines;}
  virtual void setupFromSaveLines(std::vector<std::string>& lines)  {(void)lines;}

  int getWeight() const;

  std::string getWeightStr() const;

  virtual ConsumeItem activateDefault(Actor* const actor);

  virtual Clr getInterfaceClr() const {return clrBrown;}

  virtual void newTurnInInventory() {}

  int nrItems_;

  virtual void onWear() {}
  virtual void onTakeOff() {}

  //Properties to apply e.g. when wearing something like a ring of fire resistance
  std::vector<Prop*> propsEnabledOnCarrier;

  //Called by the ItemDrop class to make noise etc
  virtual void appplyDropEffects() {}

  int meleeDmgPlus_;

protected:
  void clearPropsEnabledOnCarrier();

  //E.g. "{Off}" for Lanterns, "{60}" for Medical Bags, or "4/7" for Pistols
  virtual std::string getNameInf() const {return "";}

  ItemDataT* data_;
};

class Armor: public Item {
public:
  Armor(ItemDataT* const itemData);

  ~Armor() {}

  int getDurability() const {return dur_;}

  std::string getArmorDataLine(const bool WITH_BRACKETS) const;

  int takeDurHitAndGetReducedDmg(const int DMG_BEFORE);

  void storeToSaveLines(std::vector<std::string>& lines) override {
    lines.push_back(toStr(dur_));
  }

  void setupFromSaveLines(std::vector<std::string>& lines) override {
    dur_ = toInt(lines.front());
    lines.erase(begin(lines));
  }

  inline bool isDestroyed() {return getAbsorptionPoints() <= 0;}

  Clr getInterfaceClr() const override {return clrGray;}

  void setMaxDurability() {dur_ = 100;}

  virtual void onWear()     override {}
  virtual void onTakeOff()  override {}

protected:
  int getAbsorptionPoints() const;

  std::string getNameInf() const override {return getArmorDataLine(true);}

  int dur_;
};

class ArmorAsbSuit: public Armor {
public:
  ArmorAsbSuit(ItemDataT* const itemData) :
    Armor(itemData) {}
  ~ArmorAsbSuit() {}

  void onWear()     override;
  void onTakeOff()  override;
};

class ArmorHeavyCoat: public Armor {
public:
  ArmorHeavyCoat(ItemDataT* const itemData) :
    Armor(itemData) {}
  ~ArmorHeavyCoat() {}

  void onWear()     override;
  void onTakeOff()  override;
};

class Wpn: public Item {
public:
  Wpn(ItemDataT* const itemData, ItemDataT* const ammoData);
  virtual ~Wpn() {}

  int nrAmmoLoaded;
  int effectiveRangeLmt;
  int ammoCapacity;
  bool clip;

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
    lines.push_back(toStr(meleeDmgPlus_));
    lines.push_back(toStr(nrAmmoLoaded));
  }

  void setupFromSaveLines(std::vector<std::string>& lines) override {
    meleeDmgPlus_ = toInt(lines.front());
    lines.erase(begin(lines));
    nrAmmoLoaded = toInt(lines.front());
    lines.erase(begin(lines));
  }

  Clr getClr() const override;

  Clr getInterfaceClr() const override {return clrGray;}

protected:
  Wpn& operator=(const Wpn& other) {
    (void) other;
    return *this;
  }

  std::string getNameInf() const override;

  ItemDataT* const ammoData_;
};

class SawedOff: public Wpn {
public:
  SawedOff(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData) {
    ammoCapacity = 2;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLmt = 3;
  }
  ~SawedOff() {}
};

class PumpShotgun: public Wpn {
public:
  PumpShotgun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData) {
    ammoCapacity = 8;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLmt = 3;
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
    effectiveRangeLmt = 6;
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
    effectiveRangeLmt = 6;
  }
  ~FlareGun() {}

private:
};

class MachineGun: public Wpn {
public:
  MachineGun(ItemDataT* const itemData, ItemDataT* const ammoData);
  ~MachineGun() {}
private:
};

class Incinerator: public Wpn {
public:
  Incinerator(ItemDataT* const itemData, ItemDataT* const ammoData);
  void projectileObstructed(const Pos& pos, Actor* actorHit);
  ~Incinerator() {}
private:
};

class TeslaCannon: public Wpn {
public:
  TeslaCannon(ItemDataT* const itemData, ItemDataT* const ammoData);
  ~TeslaCannon() {}
private:
};

class SpikeGun: public Wpn {
public:
  SpikeGun(ItemDataT* const itemData, ItemDataT* const ammoData);
  ~SpikeGun() {}
private:
};

class Ammo: public Item {
public:
  Ammo(ItemDataT* const itemData) : Item(itemData) {}
  virtual ~Ammo() {}
  Clr getInterfaceClr() const override {return clrWhite;}
};

class AmmoClip: public Ammo {
public:
  AmmoClip(ItemDataT* const itemData);

  ~AmmoClip() {}

  int ammo_;

  void setFullAmmo();

  void storeToSaveLines(std::vector<std::string>& lines) override {
    lines.push_back(toStr(ammo_));
  }

  void setupFromSaveLines(std::vector<std::string>& lines) {
    ammo_ = toInt(lines.front());
    lines.erase(begin(lines));
  }

protected:
  std::string getNameInf() const override {return "{" + toStr(ammo_) + "}";}
};

enum class MedBagAction {
  sanitizeInfection,
//  takeMorphine,
  treatWounds,
  END
};

class MedicalBag: public Item {
public:
  MedicalBag(ItemDataT* const itemData) :
    Item(itemData),
    nrSupplies_(60),
    nrTurnsUntilHealWounds_(-1),
    nrTurnsLeftSanitize_(-1) {}

  ~MedicalBag() {}

  ConsumeItem activateDefault(Actor* const actor) override;

  void continueAction();
  void interrupted();
  void finishCurAction();

  Clr getInterfaceClr() const override {return clrGreen;}

  void storeToSaveLines(std::vector<std::string>& lines) override {
    lines.push_back(toStr(nrSupplies_));
  }
  void setupFromSaveLines(std::vector<std::string>& lines) override {
    nrSupplies_ = toInt(lines.front());
    lines.erase(begin(lines));
  }

  inline int getNrSupplies() const {return nrSupplies_;}

protected:
  MedBagAction playerChooseAction() const;

  int getTotTurnsForSanitize() const;
  int getTotSupplForSanitize() const;

  std::string getNameInf() const override {return "{" + toStr(nrSupplies_) + "}";}

  int nrSupplies_;

  int nrTurnsUntilHealWounds_;
  int nrTurnsLeftSanitize_;

  MedBagAction curAction_;
};

class Headwear: public Item {
public:
  Headwear(ItemDataT* itemData) : Item(itemData) {}

  Clr getInterfaceClr() const override {return clrBrown;}
};

class HideousMask: public Headwear {
public:
  HideousMask(ItemDataT* itemData) : Headwear(itemData) {}

  void  newTurnInInventory() override;
};

class GasMask: public Headwear {
public:
  GasMask(ItemDataT* itemData) : Headwear(itemData) {}

  void onWear()     override;
  void onTakeOff()  override;
};

class Explosive : public Item {
public:
  virtual ~Explosive() {}

  Explosive() = delete;

  ConsumeItem activateDefault(Actor* const actor) override final;
  Clr getInterfaceClr() const override final {return clrRedLgt;}

  virtual void        onStdTurnPlayerHoldIgnited()          = 0;
  virtual void        onThrownIgnitedLanding(const Pos& p)  = 0;
  virtual void        onPlayerParalyzed()                   = 0;
  virtual Clr         getIgnitedProjectileClr() const       = 0;
  virtual std::string getStrOnPlayerThrow()     const       = 0;

protected:
  Explosive(ItemDataT* const itemData) : Item(itemData), fuseTurns_(-1) {}

  virtual int   getStdFuseTurns() const = 0;
  virtual void  onPlayerIgnite()  const = 0;

  int fuseTurns_;
};

class Dynamite: public Explosive {
public:
  Dynamite(ItemDataT* const itemData) : Explosive(itemData) {}

  void        onThrownIgnitedLanding(const Pos& p)  override;
  void        onStdTurnPlayerHoldIgnited()          override;
  void        onPlayerParalyzed()                   override;
  Clr         getIgnitedProjectileClr()       const override {return clrRedLgt;}
  std::string getStrOnPlayerThrow()           const override {
    return "I throw a lit dynamite stick.";
  }

protected:
  int getStdFuseTurns() const override {return 6;}
  void onPlayerIgnite() const override;
};

class Molotov: public Explosive {
public:
  Molotov(ItemDataT* const itemData) : Explosive(itemData) {}

  void        onThrownIgnitedLanding(const Pos& p)  override;
  void        onStdTurnPlayerHoldIgnited()          override;
  void        onPlayerParalyzed()                   override;
  Clr         getIgnitedProjectileClr()       const override {return clrYellow;}
  std::string getStrOnPlayerThrow()           const override {
    return "I throw a lit Molotov Cocktail.";
  }

protected:
  int getStdFuseTurns() const override {return 12;}
  void onPlayerIgnite() const override;
};

class Flare: public Explosive {
public:
  Flare(ItemDataT* const itemData) : Explosive(itemData) {}

  void        onThrownIgnitedLanding(const Pos& p)  override;
  void        onStdTurnPlayerHoldIgnited()          override;
  void        onPlayerParalyzed()                   override;
  Clr getIgnitedProjectileClr()               const override {return clrYellow;}
  std::string getStrOnPlayerThrow()           const override {
    return "I throw a lit flare.";
  }

protected:
  int getStdFuseTurns() const override {return 200;}
  void onPlayerIgnite() const override;
};

class SmokeGrenade: public Explosive {
public:
  SmokeGrenade(ItemDataT* const itemData) : Explosive(itemData) {}

  void        onThrownIgnitedLanding(const Pos& p)  override;
  void        onStdTurnPlayerHoldIgnited()          override;
  void        onPlayerParalyzed()                   override;
  Clr         getIgnitedProjectileClr()       const override;
  std::string getStrOnPlayerThrow()           const override {
    return "I throw a smoke grenade.";
  }

protected:
  int getStdFuseTurns() const override {return 12;}
  void onPlayerIgnite() const override;
};

#endif
