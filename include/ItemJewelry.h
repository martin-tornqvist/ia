#ifndef ITEM_JEWELRY_H
#define ITEM_JEWELRY_H

#include "Item.h"

#include <vector>

class Prop;
class Jewelry;

enum class JewelryEffectId
{
    hpBon,
    hpPen,
    spiBon,
    spiPen,
    rFire,
    teleControl,
    randomTele,
    light,
    conflict,
    spellReflect,
    strangle, //This must of course be an amulet
    END
};

class JewelryEffect
{
public:
    JewelryEffect(Jewelry* const jewelry) :
        jewelry_(jewelry) {}

    virtual ~JewelryEffect() {}

    virtual JewelryEffectId  getId() const = 0;

    virtual void            onEquip()               {}
    virtual UnequipAllowed  onUnequip()             {return UnequipAllowed::yes;}
    virtual void            onStdTurnEquiped()      {}
    virtual void            onActorTurnEquiped()    {}

    void reveal();

    virtual std::string getDescr() const = 0;

    void storeToSaveLines   (std::vector<std::string>& lines);
    void setupFromSaveLines (std::vector<std::string>& lines);

protected:
    Jewelry* const jewelry_;
};

class JewelryEffectHpBon : public JewelryEffect
{
public:
    JewelryEffectHpBon(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectHpBon() {}

    JewelryEffectId getId() const override {return JewelryEffectId::hpBon;}

    std::string     getDescr()  const   override;
    void            onEquip()           override;
    UnequipAllowed  onUnequip()         override;
};

class JewelryEffectHpPen : public JewelryEffect
{
public:
    JewelryEffectHpPen(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectHpPen() {}

    JewelryEffectId getId() const override {return JewelryEffectId::hpPen;}

    std::string     getDescr()  const   override;
    void            onEquip()           override;
    UnequipAllowed  onUnequip()         override;
};

class JewelryEffectSpiBon : public JewelryEffect
{
public:
    JewelryEffectSpiBon(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectSpiBon() {}

    JewelryEffectId getId() const override {return JewelryEffectId::spiBon;}

    std::string     getDescr()  const override;
    void            onEquip()         override;
    UnequipAllowed  onUnequip()       override;
};

class JewelryEffectSpiPen : public JewelryEffect
{
public:
    JewelryEffectSpiPen(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectSpiPen() {}

    JewelryEffectId getId() const override {return JewelryEffectId::spiPen;}

    std::string     getDescr()  const   override;
    void            onEquip()           override;
    UnequipAllowed  onUnequip()         override;
};

class JewelryEffectRFire : public JewelryEffect
{
public:
    JewelryEffectRFire(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectRFire() {}

    JewelryEffectId getId() const override {return JewelryEffectId::rFire;}

    std::string     getDescr()  const   override;
    void            onEquip()           override;
    UnequipAllowed  onUnequip()         override;
};

class JewelryEffectTeleControl : public JewelryEffect
{
public:
    JewelryEffectTeleControl(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectTeleControl() {}

    JewelryEffectId getId() const override {return JewelryEffectId::teleControl;}

    std::string     getDescr()  const   override;
    void            onEquip()           override;
    UnequipAllowed  onUnequip()         override;
};

class JewelryEffectRandomTele : public JewelryEffect
{
public:
    JewelryEffectRandomTele(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectRandomTele() {}

    JewelryEffectId getId() const override {return JewelryEffectId::randomTele;}

    std::string     getDescr()          const   override;
    void            onStdTurnEquiped()          override;
};

class JewelryEffectLight : public JewelryEffect
{
public:
    JewelryEffectLight(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectLight() {}

    JewelryEffectId getId() const override {return JewelryEffectId::light;}

    std::string     getDescr()  const   override;
    void            onEquip()           override;
    UnequipAllowed  onUnequip()         override;
};

class JewelryEffectConflict : public JewelryEffect
{
public:
    JewelryEffectConflict(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectConflict() {}

    JewelryEffectId getId() const override {return JewelryEffectId::conflict;}

    std::string getDescr()          const   override;
    void        onStdTurnEquiped()          override;
};

class JewelryEffectSpellReflect : public JewelryEffect
{
public:
    JewelryEffectSpellReflect(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectSpellReflect() {}

    JewelryEffectId getId() const override {return JewelryEffectId::spellReflect;}

    std::string     getDescr()      const   override;
    void            onEquip()               override;
    UnequipAllowed  onUnequip()             override;
};

class JewelryEffectStrangle : public JewelryEffect
{
public:
    JewelryEffectStrangle(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectStrangle() {}

    JewelryEffectId getId() const override {return JewelryEffectId::strangle;}

    std::string     getDescr()              const   override;
    void            onEquip()                       override;
    UnequipAllowed  onUnequip()                     override;
    void            onActorTurnEquiped()            override;
};

class Jewelry : public Item
{
public:
    Jewelry(ItemDataT* const itemData);

    std::vector<std::string> getDescr() const override final;

    void            onEquip()                                   override final;
    UnequipAllowed  onUnequip()                                 override final;
    void            onStdTurnInInv      (const InvType invType) override final;
    void            onActorTurnInInv    (const InvType invType) override final;

    Clr getInterfaceClr() const override {return clrOrange;}

    void identify(const bool IS_SILENT_IDENTIFY) override final;

    //Called from the effects
    void onEffectRevealed();

    bool isAllEffectsKnown() const;

private:
    virtual std::string getNameInf() const;

    std::vector<JewelryEffect*> effects_;
};

namespace JewelryHandling
{

void init();

void storeToSaveLines  (std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

} //JewelryHandling

#endif // ITEM_JEWELRY_H
