#ifndef ITEM_JEWELRY_H
#define ITEM_JEWELRY_H

#include "Item.h"
#include "ActorData.h"

#include <vector>

class Prop;
class Jewelry;

enum class JewelryEffectId
{
    //"Primary" effects (i.e. one of these must be on the jewelry)
    rFire,
    rCold,
    rElec,
    rPoison,
    rDisease,
    teleCtrl,
    light,
    haste,
    spellReflect,
    hpBon,
    spiBon,
    conflict,

    //Secondary
    START_OF_SECONDARY_EFFECTS,
    hpPen,
    spiPen,
    burden,
    shriek,
    randomTele,
    summonMon,

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
    virtual void            changeItemWeight(int& weightRef)        {(void)weightRef;}
    virtual void            changeActorSpeed(ActorSpeed& speedRef)  {(void)speedRef;}

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
        return "It shields the wearer against fire.";
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
        return "It shields the wearer against cold.";
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
        return "It shields the wearer against electricity.";
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
        return "It shields the wearer against poison.";
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
        return "It shields the wearer against disease.";
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
        return "It grants the wearer power to control teleportation.";
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
        return "It illuminates the area around the wearer.";
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
        return "It shields the wearer against spells, and reflects them back on the "
               "caster.";
    }

protected:
    Prop* mkProp() const override;
};

class JewelryEffectHaste : public JewelryPropertyEffect
{
public:
    JewelryEffectHaste(Jewelry* const jewelry) :
        JewelryPropertyEffect(jewelry) {}

    ~JewelryEffectHaste() {}

    JewelryEffectId getId() const override {return JewelryEffectId::haste;}

    std::string getDescr() const override
    {
        return "It speeds up the wearer.";
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
        return "It grants stronger vitality.";
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
        return "It makes the wearer frailer.";
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
        return "It strengthens the spirit of the wearer.";
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
        return "It weakens the spirit of the wearer.";
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
        return "It occasionally teleports the wearer.";
    }

    void onStdTurnEquiped() override;
};

class JewelryEffectSummonMon : public JewelryEffect
{
public:
    JewelryEffectSummonMon(Jewelry* const jewelry) :
        JewelryEffect(jewelry) {}

    ~JewelryEffectSummonMon() {}

    JewelryEffectId getId() const override {return JewelryEffectId::summonMon;}

    std::string getDescr() const override
    {
        return "It calls deadly interdimensional beings into the existence of the "
               "wearer.";
    }

    void onStdTurnEquiped() override;
};

class JewelryEffectShriek : public JewelryEffect
{
public:
    JewelryEffectShriek(Jewelry* const jewelry);

    ~JewelryEffectShriek() {}

    JewelryEffectId getId() const override {return JewelryEffectId::shriek;}

    std::string getDescr() const override
    {
        return "It occasionally emits a disembodied voice in a horrible shrieking tone.";
    }

    void onStdTurnEquiped() override;

private:
    std::vector<std::string> words_;
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
        return "It sows hatred in the minds of nearby creatures, and turns allies "
               "against each other.";
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
        return "It burdens the wearer, as if there was an invisible weight to carry.";
    }

    void onEquip()                          override;
    void changeItemWeight(int& weightRef)   override;
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
    void effectNoticed(const JewelryEffectId effectId);

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
