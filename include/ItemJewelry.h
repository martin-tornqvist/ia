#ifndef ITEM_JEWELRY_H
#define ITEM_JEWELRY_H

#include "Item.h"

#include <vector>

class Prop;
class Jewelry;

enum class JewelryEffectId
{
    //Good
    rFire,
    rCold,
    rElec,
    rPoison,
    rDisease,
    teleCtrl,
    hpBon,
    spiBon,

    //Neutral
    light,          //Harder to sneak
    spellReflect,   //Very powerful, but cannot cast spells on self
    randomTele,
    conflict,       //Powerful, but draws a lot of attention
    noise,

    //Just bad
    hpPen,
    spiPen,
    burden,

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
    virtual void            changeItemWeight(int& weightRef) {(void)weightRef;}

    void reveal();

    virtual std::string getDescr() const = 0;

    void storeToSaveLines   (std::vector<std::string>& lines);
    void setupFromSaveLines (std::vector<std::string>& lines);

protected:
    Jewelry* const jewelry_;
};

//Base class for effects which just apply a property
class JewelryPropertyEffect : public JewelryEffect
{
public:
    JewelryPropertyEffect(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    virtual ~JewelryPropertyEffect() {}

    void            onEquip()           override final;
    UnequipAllowed  onUnequip()         override final;

protected:
    virtual Prop* mkProp() const = 0;
};

//--------------------------------------------------------- EFFECTS
class JewelryEffectRFire : public JewelryPropertyEffect
{
public:
    JewelryEffectRFire(Jewelry* const jewelry) :
        JewelryPropertyEffect(jewelry) {}

    ~JewelryEffectRFire() {}

    JewelryEffectId getId() const override {return JewelryEffectId::rFire;}

    std::string getDescr() const override
    {
        return "JewelryEffectRFire";
    }

protected:
    Prop* mkProp() const override;
};

class JewelryEffectRCold : public JewelryPropertyEffect
{
public:
    JewelryEffectRCold(Jewelry* const jewelry) :
        JewelryPropertyEffect(jewelry) {}

    ~JewelryEffectRCold() {}

    JewelryEffectId getId() const override {return JewelryEffectId::rCold;}

    std::string getDescr() const override
    {
        return "JewelryEffectRCold";
    }

protected:
    Prop* mkProp() const override;
};

class JewelryEffectRElec : public JewelryPropertyEffect
{
public:
    JewelryEffectRElec(Jewelry* const jewelry) :
        JewelryPropertyEffect(jewelry) {}

    ~JewelryEffectRElec() {}

    JewelryEffectId getId() const override {return JewelryEffectId::rElec;}

    std::string getDescr() const override
    {
        return "JewelryEffectRElec";
    }

protected:
    Prop* mkProp() const override;
};

class JewelryEffectRPoison : public JewelryPropertyEffect
{
public:
    JewelryEffectRPoison(Jewelry* const jewelry) :
        JewelryPropertyEffect(jewelry) {}

    ~JewelryEffectRPoison() {}

    JewelryEffectId getId() const override {return JewelryEffectId::rPoison;}

    std::string getDescr() const override
    {
        return "JewelryEffectRPoison";
    }

protected:
    Prop* mkProp() const override;
};

class JewelryEffectRDisease : public JewelryPropertyEffect
{
public:
    JewelryEffectRDisease(Jewelry* const jewelry) :
        JewelryPropertyEffect(jewelry) {}

    ~JewelryEffectRDisease() {}

    JewelryEffectId getId() const override {return JewelryEffectId::rDisease;}

    std::string getDescr() const override
    {
        return "JewelryEffectRDisease";
    }

protected:
    Prop* mkProp() const override;
};

class JewelryEffectTeleControl : public JewelryPropertyEffect
{
public:
    JewelryEffectTeleControl(Jewelry* const jewelry) :
        JewelryPropertyEffect(jewelry) {}

    ~JewelryEffectTeleControl() {}

    JewelryEffectId getId() const override {return JewelryEffectId::teleCtrl;}

    std::string getDescr() const override
    {
        return "JewelryEffectTeleControl";
    }

protected:
    Prop* mkProp() const override;
};

class JewelryEffectLight : public JewelryPropertyEffect
{
public:
    JewelryEffectLight(Jewelry* const jewelry) :
        JewelryPropertyEffect(jewelry) {}

    ~JewelryEffectLight() {}

    JewelryEffectId getId() const override {return JewelryEffectId::light;}

    std::string getDescr() const override
    {
        return "JewelryEffectLight";
    }

protected:
    Prop* mkProp() const override;
};

class JewelryEffectSpellReflect : public JewelryPropertyEffect
{
public:
    JewelryEffectSpellReflect(Jewelry* const jewelry) :
        JewelryPropertyEffect(jewelry) {}

    ~JewelryEffectSpellReflect() {}

    JewelryEffectId getId() const override {return JewelryEffectId::spellReflect;}

    std::string getDescr() const override
    {
        return "JewelryEffectSpellReflect";
    }

protected:
    Prop* mkProp() const override;
};

class JewelryEffectHpBon : public JewelryEffect
{
public:
    JewelryEffectHpBon(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectHpBon() {}

    JewelryEffectId getId() const override {return JewelryEffectId::hpBon;}

    std::string getDescr() const override
    {
        return "JewelryEffectHpBon";
    }

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

    std::string getDescr() const override
    {
        return "JewelryEffectHpPen";
    }

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

    std::string getDescr() const override
    {
        return "JewelryEffectSpiBon";
    }

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

    std::string getDescr() const override
    {
        return "JewelryEffectSpiPen";
    }

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

    std::string getDescr() const override
    {
        return "JewelryEffectRandomTele";
    }

    void onStdTurnEquiped() override;
};

class JewelryEffectNoise : public JewelryEffect
{
public:
    JewelryEffectNoise(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectNoise() {}

    JewelryEffectId getId() const override {return JewelryEffectId::noise;}

    std::string getDescr() const override
    {
        return "JewelryEffectNoise";
    }

    void onStdTurnEquiped() override;
};

class JewelryEffectConflict : public JewelryEffect
{
public:
    JewelryEffectConflict(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectConflict() {}

    JewelryEffectId getId() const override {return JewelryEffectId::conflict;}

    std::string getDescr() const override
    {
        return "JewelryEffectConflict";
    }

    void onStdTurnEquiped() override;
};

class JewelryEffectBurden : public JewelryEffect
{
public:
    JewelryEffectBurden(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectBurden() {}

    JewelryEffectId getId() const override {return JewelryEffectId::burden;}

    std::string getDescr() const override
    {
        return "JewelryEffectBurden";
    }

    void            onEquip()           override;
    void            changeItemWeight(int& weightRef);
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

    int getWeight() const override;

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
