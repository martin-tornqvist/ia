#ifndef ITEM_H
#define ITEM_H

#include <iostream>

#include "Art.h"
#include "InventoryHandling.h"
#include "Converters.h"

class ItemDataT;
class Prop;
class Actor;
class Spell;

enum ItemActivateReturnType
{
  itemActivate_keep, itemActivate_destroyed
};

class Item
{
public:
  Item(ItemDataT* itemData);

  Item& operator=(Item& other) = delete;

  virtual ~Item();

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

  virtual void            onEquip()   {}
  virtual UnequipAllowed  onUnequip() {return UnequipAllowed::yes;}

  //Called by the ItemDrop class to make noise etc
  virtual void appplyDropEffects() {}

  int nrItems_;

  //Properties to apply when wearing something like a ring of fire resistance
  std::vector<Prop*>  carrierProps_;

  //Spells granted to the carrier
  std::vector<Spell*> carrierSpells_;

  int meleeDmgPlus_;

protected:
  void clearCarrierProps();

  //E.g. "{Off}" for Lanterns, "{60}" for Medical Bags, or "4/7" for Pistols
  virtual std::string getNameInf() const {return "";}

  ItemDataT* data_;
};

class Armor: public Item
{
public:
  Armor(ItemDataT* const itemData);

  ~Armor() {}

  void storeToSaveLines  (std::vector<std::string>& lines) override;
  void setupFromSaveLines(std::vector<std::string>& lines) override;

  Clr                     getInterfaceClr() const override {return clrGray;}
  virtual void            onEquip()               override;
  virtual UnequipAllowed  onUnequip()             override;

  int   getDurability()     const {return dur_;}
  void  setMaxDurability()        {dur_ = 100;}
  bool  isDestroyed()       const {return getAbsorptionPoints() <= 0;}

  std::string getArmorDataLine(const bool WITH_BRACKETS) const;

  int takeDurHitAndGetReducedDmg(const int DMG_BEFORE);

protected:
  int getAbsorptionPoints() const;

  virtual void            onEquip_()    {}
  virtual UnequipAllowed  onUnequip_()  {return UnequipAllowed::yes;}

  std::string getNameInf() const override {return getArmorDataLine(true);}

  int dur_;
};

class ArmorAsbSuit: public Armor
{
public:
  ArmorAsbSuit(ItemDataT* const itemData) : Armor(itemData) {}
  ~ArmorAsbSuit() {}

private:
  void            onEquip_()    override;
  UnequipAllowed  onUnequip_()  override;
};

class ArmorHeavyCoat: public Armor
{
public:
  ArmorHeavyCoat(ItemDataT* const itemData) : Armor(itemData) {}
  ~ArmorHeavyCoat() {}

private:
  void            onEquip_()    override;
  UnequipAllowed  onUnequip_()  override;
};

class ArmorMigo: public Armor
{
public:
  ArmorMigo(ItemDataT* const itemData) : Armor(itemData) {}
  ~ArmorMigo() {}

  void newTurnInInventory() override;

private:
  void            onEquip_()    override;
  UnequipAllowed  onUnequip_()  override;
};

class Wpn: public Item
{
public:
  Wpn(ItemDataT* const itemData, ItemDataT* const ammoData);
  virtual ~Wpn() {}

  int nrAmmoLoaded;
  int effectiveRangeLmt;
  int ammoCapacity;
  bool clip;

  void setRandomMeleePlus();

  void storeToSaveLines  (std::vector<std::string>& lines) override;
  void setupFromSaveLines(std::vector<std::string>& lines) override;

  Clr getClr()          const override;
  Clr getInterfaceClr() const override {return clrGray;}

  //"actor" may be nullptr
  virtual void projectileObstructed(const Pos& pos, Actor* actor)
  {
    (void)pos;
    (void)actor;
  }

  const ItemDataT& getAmmoData() {return *ammoData_;}

protected:
  Wpn& operator=(const Wpn& other) = delete;
//  Wpn& operator=(const Wpn& other)
//  {
//    (void) other;
//    return *this;
//  }

  std::string getNameInf() const override;

  ItemDataT* const ammoData_;
};

class PharaohStaff: public Wpn
{
public:
  PharaohStaff(ItemDataT* const itemData);
};

class SawedOff: public Wpn
{
public:
  SawedOff(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData)
  {
    ammoCapacity = 2;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLmt = 3;
  }
  ~SawedOff() {}
};

class PumpShotgun: public Wpn
{
public:
  PumpShotgun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData)
  {
    ammoCapacity = 8;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLmt = 3;
  }
  ~PumpShotgun() {}
};

class Pistol: public Wpn
{
public:
  Pistol(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData)
  {
    ammoCapacity = 7;
    nrAmmoLoaded = ammoCapacity;
    effectiveRangeLmt = 6;
  }
  ~Pistol() {}
};

class FlareGun: public Wpn
{
public:
  FlareGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData)
  {
    nrAmmoLoaded = 1;
    ammoCapacity = 1;
    effectiveRangeLmt = 6;
  }
  ~FlareGun() {}
};

class MachineGun: public Wpn
{
public:
  MachineGun(ItemDataT* const itemData, ItemDataT* const ammoData);
  ~MachineGun() {}
};

class Incinerator: public Wpn
{
public:
  Incinerator(ItemDataT* const itemData, ItemDataT* const ammoData);
  void projectileObstructed(const Pos& pos, Actor* actorHit);
  ~Incinerator() {}
};

class MigoGun: public Wpn
{
public:
  MigoGun(ItemDataT* const itemData, ItemDataT* const ammoData);
  ~MigoGun() {}
};

class SpikeGun: public Wpn
{
public:
  SpikeGun(ItemDataT* const itemData, ItemDataT* const ammoData);
  ~SpikeGun() {}
};

class Ammo: public Item
{
public:
  Ammo(ItemDataT* const itemData) : Item(itemData) {}
  virtual ~Ammo() {}
  Clr getInterfaceClr() const override {return clrWhite;}
};

class AmmoClip: public Ammo
{
public:
  AmmoClip(ItemDataT* const itemData);

  ~AmmoClip() {}

  int ammo_;

  void setFullAmmo();

  void storeToSaveLines(std::vector<std::string>& lines) override
  {
    lines.push_back(toStr(ammo_));
  }

  void setupFromSaveLines(std::vector<std::string>& lines)
  {
    ammo_ = toInt(lines.front());
    lines.erase(begin(lines));
  }

protected:
  std::string getNameInf() const override {return "{" + toStr(ammo_) + "}";}
};

enum class MedBagAction
{
  sanitizeInfection,
//  takeMorphine,
  treatWounds,
  END
};

class MedicalBag: public Item
{
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

  void storeToSaveLines(std::vector<std::string>& lines) override
  {
    lines.push_back(toStr(nrSupplies_));
  }
  void setupFromSaveLines(std::vector<std::string>& lines) override
  {
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

class Headwear: public Item
{
public:
  Headwear(ItemDataT* itemData) : Item(itemData) {}

  Clr getInterfaceClr() const override {return clrBrown;}
};

class HideousMask: public Headwear
{
public:
  HideousMask(ItemDataT* itemData) : Headwear(itemData) {}

  void  newTurnInInventory() override;
};

class GasMask: public Headwear
{
public:
  GasMask(ItemDataT* itemData) : Headwear(itemData) {}

  void            onEquip()   override;
  UnequipAllowed  onUnequip() override;
};

class Explosive : public Item
{
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

class Dynamite: public Explosive
{
public:
  Dynamite(ItemDataT* const itemData) : Explosive(itemData) {}

  void        onThrownIgnitedLanding(const Pos& p)  override;
  void        onStdTurnPlayerHoldIgnited()          override;
  void        onPlayerParalyzed()                   override;
  Clr         getIgnitedProjectileClr()       const override {return clrRedLgt;}
  std::string getStrOnPlayerThrow()           const override
  {
    return "I throw a lit dynamite stick.";
  }

protected:
  int getStdFuseTurns() const override {return 6;}
  void onPlayerIgnite() const override;
};

class Molotov: public Explosive
{
public:
  Molotov(ItemDataT* const itemData) : Explosive(itemData) {}

  void        onThrownIgnitedLanding(const Pos& p)  override;
  void        onStdTurnPlayerHoldIgnited()          override;
  void        onPlayerParalyzed()                   override;
  Clr         getIgnitedProjectileClr()       const override {return clrYellow;}
  std::string getStrOnPlayerThrow()           const override
  {
    return "I throw a lit Molotov Cocktail.";
  }

protected:
  int getStdFuseTurns() const override {return 12;}
  void onPlayerIgnite() const override;
};

class Flare: public Explosive
{
public:
  Flare(ItemDataT* const itemData) : Explosive(itemData) {}

  void        onThrownIgnitedLanding(const Pos& p)  override;
  void        onStdTurnPlayerHoldIgnited()          override;
  void        onPlayerParalyzed()                   override;
  Clr getIgnitedProjectileClr()               const override {return clrYellow;}
  std::string getStrOnPlayerThrow()           const override
  {
    return "I throw a lit flare.";
  }

protected:
  int getStdFuseTurns() const override {return 200;}
  void onPlayerIgnite() const override;
};

class SmokeGrenade: public Explosive
{
public:
  SmokeGrenade(ItemDataT* const itemData) : Explosive(itemData) {}

  void        onThrownIgnitedLanding(const Pos& p)  override;
  void        onStdTurnPlayerHoldIgnited()          override;
  void        onPlayerParalyzed()                   override;
  Clr         getIgnitedProjectileClr()       const override;
  std::string getStrOnPlayerThrow()           const override
  {
    return "I throw a smoke grenade.";
  }

protected:
  int getStdFuseTurns() const override {return 12;}
  void onPlayerIgnite() const override;
};

#endif
