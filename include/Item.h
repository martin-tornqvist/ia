#ifndef ITEM_H
#define ITEM_H

#include "Art.h"
#include "InventoryHandling.h"
#include "Converters.h"
#include "CmnData.h"

class ItemDataT;
class Prop;
class Actor;
class Spell;

enum class ItemActivateRetType {keep, destroyed};

class Item
{
public:
    Item(ItemDataT* itemData);

    Item& operator=(Item& other) = delete;

    virtual ~Item();

    ItemId            getId()       const;
    const ItemDataT&  getData()     const;
    virtual Clr       getClr()      const;
    char              getGlyph()    const;
    TileId            getTile()     const;

    virtual LgtSize   getLgtSize()  const {return LgtSize::none;}

    std::string getName(const ItemRefType   refType,
                        const ItemRefInf    inf     = ItemRefInf::yes,
                        const ItemRefAttInf attInf  = ItemRefAttInf::none) const;

    virtual std::vector<std::string> getDescr() const;

    virtual void identify(const bool IS_SILENT_IDENTIFY) {(void)IS_SILENT_IDENTIFY;}

    virtual void storeToSaveLines  (std::vector<std::string>& lines)  {(void)lines;}
    virtual void setupFromSaveLines(std::vector<std::string>& lines)  {(void)lines;}

    virtual int getWeight() const;

    std::string getWeightStr() const;

    virtual ConsumeItem activate(Actor* const actor);

    virtual Clr getInterfaceClr() const {return clrBrown;}

    virtual void            onStdTurnInInv    (const InvType invType) {(void)invType;}
    virtual void            onActorTurnInInv  (const InvType invType) {(void)invType;}
    virtual void            onPickupToBackpack(Inventory& inv)    {(void)inv;}

    virtual void onEquip(const bool IS_SILENT)
    {
        (void)IS_SILENT;
    }

    virtual UnequipAllowed  onUnequip() {return UnequipAllowed::yes;}

    virtual int getHpRegenChange(const InvType invType) const
    {
        (void)invType;
        return 0;
    }

    //Used when attempting to fire or throw an item
    bool isInEffectiveRangeLmt(const Pos& p0, const Pos& p1) const;

    int nrItems_;

    //Properties to apply when wearing something like a ring of fire resistance
    std::vector<Prop*>  carrierProps_;

    //Spells granted to the carrier
    std::vector<Spell*> carrierSpells_;

    int meleeDmgPlus_;

protected:
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
    virtual void            onEquip(const bool IS_SILENT)   override;
    virtual UnequipAllowed  onUnequip()                     override;

    int   getDurability()     const {return dur_;}
    void  setMaxDurability()        {dur_ = 100;}
    bool  isDestroyed()       const {return getArmorPoints() <= 0;}

    std::string getArmorDataLine(const bool WITH_BRACKETS) const;

    int takeDurHitAndGetReducedDmg(const int DMG_BEFORE);

protected:
    int getArmorPoints() const;

    virtual void onEquip_(const bool IS_SILENT)
    {
        (void)IS_SILENT;
    }
    virtual UnequipAllowed  onUnequip_() {return UnequipAllowed::yes;}

    std::string getNameInf() const override {return getArmorDataLine(true);}

    int dur_;
};

class ArmorAsbSuit: public Armor
{
public:
    ArmorAsbSuit(ItemDataT* const itemData) : Armor(itemData) {}
    ~ArmorAsbSuit() {}

private:
    void            onEquip_(const bool IS_SILENT)  override;
    UnequipAllowed  onUnequip_()                    override;
};

class ArmorHeavyCoat: public Armor
{
public:
    ArmorHeavyCoat(ItemDataT* const itemData) : Armor(itemData) {}
    ~ArmorHeavyCoat() {}

private:
    void            onEquip_(const bool IS_SILENT)  override;
    UnequipAllowed  onUnequip_()                    override;
};

class ArmorMiGo: public Armor
{
public:
    ArmorMiGo(ItemDataT* const itemData) : Armor(itemData) {}
    ~ArmorMiGo() {}

    void onStdTurnInInv(const InvType invType) override;

private:
    void            onEquip_(const bool IS_SILENT)  override;
    UnequipAllowed  onUnequip_()                    override;
};

class Wpn: public Item
{
public:
    Wpn(ItemDataT* const itemData, ItemDataT* const ammoData, int ammoCap = 0,
        bool isUsingClip = false);

    virtual ~Wpn() {}

    Wpn& operator=(const Wpn& other) = delete;

    void setRandomMeleePlus();

    void storeToSaveLines  (std::vector<std::string>& lines) override;
    void setupFromSaveLines(std::vector<std::string>& lines) override;

    Clr getClr()          const override;
    Clr getInterfaceClr() const override {return clrGray;}

    const ItemDataT& getAmmoData() {return *ammoData_;}

    virtual void onProjectileBlocked(const Pos& pos, Actor* actor)
    {
        (void)pos;
        (void)actor;
    }

    const int   AMMO_CAP;
    const bool  IS_USING_CLIP;

    int nrAmmoLoaded;

protected:
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
        Wpn(itemData, ammoData, 2, false) {}
    ~SawedOff() {}
};

class PumpShotgun: public Wpn
{
public:
    PumpShotgun(ItemDataT* const itemData, ItemDataT* const ammoData) :
        Wpn(itemData, ammoData, 8, false) {}
    ~PumpShotgun() {}
};

class Pistol: public Wpn
{
public:
    Pistol(ItemDataT* const itemData, ItemDataT* const ammoData) :
        Wpn(itemData, ammoData, 7, true) {}
    ~Pistol() {}
};

class FlareGun: public Wpn
{
public:
    FlareGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
        Wpn(itemData, ammoData, 1, false) {}
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
    void onProjectileBlocked(const Pos& pos, Actor* actorHit);
    ~Incinerator() {}
};

class MiGoGun: public Wpn
{
public:
    MiGoGun(ItemDataT* const itemData, ItemDataT* const ammoData);
    ~MiGoGun() {}
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
        Item                    (itemData),
        nrSupplies_             (40),
        nrTurnsUntilHealWounds_ (-1),
        nrTurnsLeftSanitize_    (-1),
        curAction_              (MedBagAction::END) {}

    ~MedicalBag() {}

    void onPickupToBackpack(Inventory& inv) override;

    ConsumeItem activate(Actor* const actor) override;

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

    int getNrSupplies() const {return nrSupplies_;}

protected:
    MedBagAction chooseAction() const;

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

//class HideousMask: public Headwear
//{
//public:
//    HideousMask(ItemDataT* itemData);
//
//    void onStdTurnInInv(const InvType invType) override;
//};

class GasMask: public Headwear
{
public:
    GasMask(ItemDataT* itemData) : Headwear(itemData), nrTurnsLeft_(60) {}

    void            onEquip(const bool IS_SILENT)   override;
    UnequipAllowed  onUnequip()                     override;

    void            decrTurnsLeft(Inventory& carrierInv);

protected:
    std::string getNameInf() const override {return "{" + toStr(nrTurnsLeft_) + "}";}

    int nrTurnsLeft_;
};

class Explosive : public Item
{
public:
    virtual ~Explosive() {}

    Explosive() = delete;

    ConsumeItem activate(Actor* const actor) override final;
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
