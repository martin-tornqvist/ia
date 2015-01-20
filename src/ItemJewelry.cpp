#include "Init.h"

#include "ItemJewelry.h"

#include "Map.h"
#include "Utils.h"
#include "Log.h"
#include "GameTime.h"
#include "Render.h"

using namespace std;

namespace
{

ItemId  effectList_   [int(JewelryEffectId::END)];
bool    effectsKnown_ [int(JewelryEffectId::END)];

JewelryEffect* mkEffect(const JewelryEffectId id, Jewelry* const jewelry)
{
    assert(id != JewelryEffectId::END);

    JewelryEffect* ret = nullptr;

    switch (id)
    {
    case JewelryEffectId::hpBon: ret = new JewelryEffectHpBon(jewelry);    break;

    case JewelryEffectId::hpPen:
        ret = new JewelryEffectHpPen(jewelry);
        break;

    case JewelryEffectId::spiBon:
        ret = new JewelryEffectSpiBon(jewelry);
        break;

    case JewelryEffectId::spiPen:
        ret = new JewelryEffectSpiPen(jewelry);
        break;

    case JewelryEffectId::rFire:
        ret = new JewelryEffectRFire(jewelry);
        break;

    case JewelryEffectId::teleCtrl: ret = new JewelryEffectTeleControl(jewelry); break;

    case JewelryEffectId::randomTele:
        ret = new JewelryEffectRandomTele(jewelry);
        break;

    case JewelryEffectId::light:
        ret = new JewelryEffectLight(jewelry);
        break;

    case JewelryEffectId::conflict:
        ret = new JewelryEffectConflict(jewelry);
        break;

    case JewelryEffectId::spellReflect:
        ret = new JewelryEffectSpellReflect(jewelry);
        break;

    case JewelryEffectId::strangle:
        ret = new JewelryEffectStrangle(jewelry);
        break;

    case JewelryEffectId::END: {}
        break;
    }

    assert(ret);
    return ret;
}

} //namespace

//--------------------------------------------------------- JEWELRY EFFECT
void JewelryEffect::reveal()
{
    const size_t EFFECT_IDX = size_t(getId());

    if (!effectsKnown_[EFFECT_IDX])
    {
        effectsKnown_[EFFECT_IDX] = true;

        const string name = jewelry_->getName(ItemRefType::plain, ItemRefInf::none);

        const string msg = jewelry_->isAllEffectsKnown() ?
                           "All properties of the " + name + " are now known to me." :
                           "I gained new knowledge about the " + name + ".";

        Log::addMsg(msg, clrWhite, false, true);

        jewelry_->onEffectRevealed();

        Map::player->incrShock(ShockLvl::heavy, ShockSrc::useStrangeItem);
    }
}

//--------------------------------------------------------- EFFECT: HP BONUS
string JewelryEffectHpBon::getDescr() const
{
    return "JewelryEffectHpBon.";
}

void JewelryEffectHpBon::onEquip()
{
    Map::player->changeMaxHp(4, true);
    reveal();
}

UnequipAllowed JewelryEffectHpBon::onUnequip()
{
    Map::player->changeMaxHp(-4, true);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: HP PENALTY
string JewelryEffectHpPen::getDescr() const
{
    return "JewelryEffectHpPen.";
}

void JewelryEffectHpPen::onEquip()
{
    Map::player->changeMaxHp(-2, true);
    reveal();
}

UnequipAllowed JewelryEffectHpPen::onUnequip()
{
    Map::player->changeMaxHp(2, true);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: SPI BONUS
string JewelryEffectSpiBon::getDescr() const
{
    return "JewelryEffectSpiBon.";
}

void JewelryEffectSpiBon::onEquip()
{
    Map::player->changeMaxSpi(2, true);
    reveal();
}

UnequipAllowed JewelryEffectSpiBon::onUnequip()
{
    Map::player->changeMaxSpi(-2, true);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: SPI PENALTY
string JewelryEffectSpiPen::getDescr() const
{
    return "JewelryEffectSpiPen.";
}

void JewelryEffectSpiPen::onEquip()
{
    Map::player->changeMaxSpi(-2, true);
    reveal();
}

UnequipAllowed JewelryEffectSpiPen::onUnequip()
{
    Map::player->changeMaxSpi(2, true);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: FIRE RESISTANCE
string JewelryEffectRFire::getDescr() const
{
    return "JewelryEffectRFire.";
}

void JewelryEffectRFire::onEquip()
{
    jewelry_->carrierProps_.push_back(new PropRFire(PropTurns::indefinite));

    const auto&   propData  = PropData::data[size_t(PropId::rFire)];
    const string  msg       = propData.msg[propMsgOnStartPlayer];

    Log::addMsg(msg);

    reveal();
}

UnequipAllowed JewelryEffectRFire::onUnequip()
{
    for (Prop* prop : jewelry_->carrierProps_) {delete prop;}
    jewelry_->carrierProps_.clear();

    const auto&   propData  = PropData::data[size_t(PropId::rFire)];
    const string  msg       = propData.msg[propMsgOnEndPlayer];

    Log::addMsg(msg);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: TELEPORT CONTROL
string JewelryEffectTeleControl::getDescr() const
{
    return "JewelryEffectTeleControl.";
}

void JewelryEffectTeleControl::onEquip()
{
    jewelry_->carrierProps_.push_back(new PropTeleControl(PropTurns::indefinite));

    const auto&   propData  = PropData::data[size_t(PropId::teleCtrl)];
    const string  msg       = propData.msg[propMsgOnStartPlayer];

    Log::addMsg(msg);

    reveal();
}

UnequipAllowed JewelryEffectTeleControl::onUnequip()
{
    for (Prop* prop : jewelry_->carrierProps_) {delete prop;}
    jewelry_->carrierProps_.clear();

    const auto&   propData  = PropData::data[size_t(PropId::teleCtrl)];
    const string  msg       = propData.msg[propMsgOnEndPlayer];

    Log::addMsg(msg);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: RANDOM TELEPORTATION
string JewelryEffectRandomTele::getDescr() const
{
    return "JewelryEffectRandomTele.";
}

void JewelryEffectRandomTele::onStdTurnEquiped()
{
    auto& propHandler = Map::player->getPropHandler();

    if (Rnd::oneIn(200) && propHandler.allowAct())
    {
        Log::addMsg("I am being teleported...", clrWhite, true, true);
        Map::player->teleport();
        reveal();
    }
}

//--------------------------------------------------------- EFFECT: LIGHT
string JewelryEffectLight::getDescr() const
{
    return "JewelryEffectLight.";
}

void JewelryEffectLight::onEquip()
{
    jewelry_->carrierProps_.push_back(new PropRadiant(PropTurns::indefinite));

    GameTime::updateLightMap();
    Map::player->updateFov();
    Render::drawMapAndInterface();

    const auto&   propData  = PropData::data[size_t(PropId::radiant)];
    const string  msg       = propData.msg[propMsgOnStartPlayer];

    Log::addMsg(msg);

    reveal();
}

UnequipAllowed JewelryEffectLight::onUnequip()
{
    for (Prop* prop : jewelry_->carrierProps_) {delete prop;}
    jewelry_->carrierProps_.clear();

    GameTime::updateLightMap();
    Map::player->updateFov();
    Render::drawMapAndInterface();

    const auto&   propData  = PropData::data[size_t(PropId::radiant)];
    const string  msg       = propData.msg[propMsgOnEndPlayer];

    Log::addMsg(msg);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: CONFLICT
string JewelryEffectConflict::getDescr() const
{
    return "JewelryEffectConflict.";
}

void JewelryEffectConflict::onStdTurnEquiped()
{

}

//--------------------------------------------------------- EFFECT: SPELL REFLECTION
string JewelryEffectSpellReflect::getDescr() const
{
    return "JewelryEffectSpellReflect.";
}

void JewelryEffectSpellReflect::onEquip()
{
    jewelry_->carrierProps_.push_back(new PropSpellReflect(PropTurns::indefinite));

    const auto&   propData  = PropData::data[size_t(PropId::spellReflect)];
    const string  msg       = propData.msg[propMsgOnStartPlayer];

    Log::addMsg(msg);

    reveal();
}

UnequipAllowed JewelryEffectSpellReflect::onUnequip()
{
    for (Prop* prop : jewelry_->carrierProps_) {delete prop;}
    jewelry_->carrierProps_.clear();

    const auto&   propData  = PropData::data[size_t(PropId::spellReflect)];
    const string  msg       = propData.msg[propMsgOnEndPlayer];

    Log::addMsg(msg);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: STRANGULATION
string JewelryEffectStrangle::getDescr() const
{
    return "JewelryEffectStrangle.";
}

void JewelryEffectStrangle::onEquip()
{
    jewelry_->carrierProps_.push_back(new PropStrangled(PropTurns::indefinite));
}

UnequipAllowed JewelryEffectStrangle::onUnequip()
{
    const string name = jewelry_->getName(ItemRefType::plain, ItemRefInf::none);

    Log::addMsg("The " + name + " is stuck!");

    return UnequipAllowed::no;
}

void JewelryEffectStrangle::onActorTurnEquiped()
{
    const string name = jewelry_->getName(ItemRefType::plain, ItemRefInf::none);

    Log::addMsg("The " + name + " constricts my throat!", clrMsgBad);

    const bool IS_TOUGH         = PlayerBon::traits[int(Trait::tough)];
    const bool IS_RUGGED        = PlayerBon::traits[int(Trait::rugged)];
    const bool IS_UNBREAKABLE   = PlayerBon::traits[int(Trait::unbreakable)];

    bool props[size_t(PropId::END)];

    Map::player->getPropHandler().getPropIds(props);

    const bool IS_WEAKENED = props[size_t(PropId::weakened)];
    const bool IS_FRENZIED = props[size_t(PropId::frenzied)];

    const int REMOVE_ONE_IN_N   = IS_UNBREAKABLE  ? 3 :
                                  IS_RUGGED       ? 6 :
                                  IS_TOUGH        ? 9 : 12;

    if (IS_WEAKENED)
    {
        Log::addMsg("I tear at it feebly...", clrWhite, false, true);
    }
    else if (!IS_FRENZIED)
    {
        Log::addMsg("I struggle to remove it...", clrWhite, false, true);
    }

    if (!IS_WEAKENED && (IS_FRENZIED || Rnd::oneIn(REMOVE_ONE_IN_N)))
    {
        Log::addMsg("I tear it off!", clrWhite, false, true);

        for (Prop* prop : jewelry_->carrierProps_) {delete prop;}
        jewelry_->carrierProps_.clear();

        Inventory& inv = Map::player->getInv();

        assert(inv.slots_[size_t(SlotId::neck)].item);
        assert(inv.slots_[size_t(SlotId::neck)].item->getData().type == ItemType::amulet);

        inv.moveToGeneral(SlotId::neck);
    }
    else //Failed to remove
    {
        Log::addMsg("It is stuck!", clrWhite, true);
    }

    reveal();
}

//--------------------------------------------------------- JEWELRY
Jewelry::Jewelry(ItemDataT* const itemData) :
    Item(itemData)
{
    for (size_t i = 0; i < int(JewelryEffectId::END); ++i)
    {
        if (effectList_[i] == itemData->id)
        {
            auto* const effect = mkEffect(JewelryEffectId(i), this);
            effects_.push_back(effect);
        }
    }

    //Unique item, do not allow spawning more
    data_->allowSpawn = false;
}

vector<string> Jewelry::getDescr() const
{
    vector<string> ret;

    for (auto* effect : effects_)
    {
        const size_t EFFECT_IDX = size_t(effect->getId());

        if (effectsKnown_[EFFECT_IDX])
        {
            ret.push_back(effect->getDescr());
        }
    }

    if (isAllEffectsKnown())
    {
        const string name = getName(ItemRefType::plain, ItemRefInf::none);

        ret.push_back("All properties of the " + name + " are known to me.");
    }

    return ret;
}

string Jewelry::getNameInf() const
{
    return isAllEffectsKnown() ? "{Known}" : "";
}

void Jewelry::onEquip()
{
    for (auto* const effect : effects_)
    {
        //This may cause the effect to set up carrier properties (e.g. fire resistance)
        effect->onEquip();
    }

    Log::morePrompt();
}

UnequipAllowed Jewelry::onUnequip()
{
    auto unequipAllowed = UnequipAllowed::yes;

    for (auto* const effect : effects_)
    {
        if (effect->onUnequip() == UnequipAllowed::no)
        {
            unequipAllowed = UnequipAllowed::no;
        }
    }

    Log::morePrompt();

    return unequipAllowed;
}

void Jewelry::onStdTurnInInv(const InvType invType)
{
    if (invType == InvType::slots)
    {
        for (auto* const effect : effects_)
        {
            effect->onStdTurnEquiped();
        }
    }
}

void Jewelry::onActorTurnInInv(const InvType invType)
{
    if (invType == InvType::slots)
    {
        for (auto* const effect : effects_)
        {
            effect->onActorTurnEquiped();
        }
    }
}

void Jewelry::identify(const bool IS_SILENT_IDENTIFY)
{
    (void)IS_SILENT_IDENTIFY;

    for (auto* effect : effects_)
    {
        const size_t EFFECT_IDX = size_t(effect->getId());

        effectsKnown_[EFFECT_IDX] = true;
    }

    data_->isIdentified = true;
}

void Jewelry::onEffectRevealed()
{
    assert(!data_->isIdentified);

    if (isAllEffectsKnown())
    {
        data_->isIdentified = true;
    }
}

bool Jewelry::isAllEffectsKnown() const
{
    for (auto* effect : effects_)
    {
        if (!effectsKnown_[int(effect->getId())])
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------- JEWELRY HANDLING
namespace
{

bool isEffectCombinationAllowed(const JewelryEffectId id1,
                                const JewelryEffectId id2)
{
    typedef JewelryEffectId Id;

    assert (id1 != Id::END && id2 != Id::END);

    if (id1 == id2)
    {
        return true;
    }

    switch (id1)
    {
    case Id::hpBon:         return id2 != Id::hpPen;
    case Id::hpPen:         return id2 != Id::hpBon;
    case Id::spiBon:        return id2 != Id::spiPen;
    case Id::spiPen:        return id2 != Id::spiBon;
    case Id::rFire:         return true;
    case Id::teleCtrl:      return id2 != Id::randomTele;
    case Id::randomTele:    return id2 != Id::teleCtrl;
    case Id::light:         return true;
    case Id::conflict:      return true;
    case Id::spellReflect:  return true;
    case Id::strangle:      return false;
    case Id::END: {} break;
    }
    return false;
}

int getRndItemBucketIdxForEffect(const JewelryEffectId  effectToAssign,
                                 const vector<ItemId>&  itemBucket)
{
    vector<int> itemIdxBucket;

    for (size_t itemBucketIdx = 0; itemBucketIdx < itemBucket.size(); ++itemBucketIdx)
    {
        bool canBePlacedOnItem = true;

        //Verify that the effect can be placed on this item by checking if it can
        //be combined with every effect currently assigned to this item.
        for (int i = 0; i < int(JewelryEffectId::END); ++i)
        {
            const JewelryEffectId curEffect = JewelryEffectId(i);

            if (
                effectList_[i] == itemBucket[itemBucketIdx] &&
                !isEffectCombinationAllowed(effectToAssign, curEffect))
            {
                //Combination with effect already assigned on item not allowed
                canBePlacedOnItem = false;
                break;
            }
        }

        if (canBePlacedOnItem)
        {
            itemIdxBucket.push_back(itemBucketIdx);
        }
    }

    if (itemIdxBucket.empty())
    {
        return -1;
    }

    const int IDX_BUCKET_IDX = Rnd::range(0, itemIdxBucket.size() - 1);
    return itemIdxBucket[IDX_BUCKET_IDX];
}

} //namespace

namespace JewelryHandling
{

void init()
{
    //Reset the effect list and knowledge status list
    for (size_t i = 0; i < int(JewelryEffectId::END); ++i)
    {
        effectList_   [i] = ItemId::END;
        effectsKnown_ [i] = false;
    }

    //First, assign strangulation to a random amulet.
    int firstAmuletIdx = int(ItemId::END);
    int lastAmuletIdx  = int(ItemId::END);

    for (size_t i = 0; i < size_t(ItemId::END); ++i)
    {
        if (ItemData::data[i]->type == ItemType::amulet)
        {
            if (firstAmuletIdx == int(ItemId::END))
            {
                firstAmuletIdx = int(i);
            }
            lastAmuletIdx = int(i);
        }
    }

    const ItemId strangleAmuletId = ItemId(Rnd::range(firstAmuletIdx, lastAmuletIdx));

    effectList_[size_t(JewelryEffectId::strangle)] = strangleAmuletId;


    //Assign random effects
    vector<JewelryEffectId> effectBucket;

    effectBucket.reserve(int(JewelryEffectId::END));

    for (size_t i = 0; i < size_t(JewelryEffectId::END); ++i)
    {
        if (effectList_[i] == ItemId::END)
        {
            effectBucket.push_back(JewelryEffectId(i));
        }
    }

    vector<ItemId> itemBucket;

    const int MAX_NR_EFFECTS_PER_ITEM = 2;

    for (auto* data : ItemData::data)
    {
        const auto id   = data->id;
        const auto type = data->type;

        if (
            id != strangleAmuletId &&
            (type == ItemType::amulet || type == ItemType::ring))
        {
            //Some amulets and rings are added multiple times to the bucket. These can
            //get multiple effects assigned.
            const int NR = Rnd::range(1, MAX_NR_EFFECTS_PER_ITEM);

            itemBucket.insert(end(itemBucket), NR, id);
        }
    }

    while (true)
    {
        //No more effects left to assign?
        if (effectBucket.empty())
        {
            //Mark each amulet and ring that has not yet had any effects assigned to
            //them as forbidden items.
            for (auto* data : ItemData::data)
            {
                const auto itBeg = begin(effectList_);
                const auto itEnd = end  (effectList_);
                if (
                    (data->type == ItemType::amulet || data->type == ItemType::ring) &&
                    find(itBeg, itEnd, data->id) == itEnd)
                {
                    data->allowSpawn = false;
                }
            }
            break;
        }

        //No more items left to assign effects to?
        if (itemBucket.empty())
        {
            break;
        }

        //Try to assign a random effect from the effect bucket to an item
        const int   EFFECT_IDX  = Rnd::range(0, effectBucket.size() - 1);
        const auto  effectId    = effectBucket[EFFECT_IDX];

        //This may fail to find an item bucket index. If so, it means that the effect
        //cannot be placed on any item (there are no free items, and the effect cannot be
        //combined with any of the effects currently assigned)
        const int   ITEM_IDX    = getRndItemBucketIdxForEffect(effectId, itemBucket);

        if (ITEM_IDX >= 0)
        {
            //Effect can be placed (an allowed effect combination or free item exists)
            const auto itemId           = itemBucket[ITEM_IDX];
            effectList_[int(effectId)]  = itemId;

            itemBucket.erase(begin(itemBucket) + ITEM_IDX);
        }

        effectBucket.erase(begin(effectBucket) + EFFECT_IDX);
    }
}

void storeToSaveLines(vector<string>& lines)
{
    for (size_t i = 0; i < size_t(JewelryEffectId::END); ++i)
    {
        lines.push_back(toStr(int(effectList_[i])));
        lines.push_back(effectsKnown_[i] ? "1" : "0");
    }
}

void setupFromSaveLines(vector<string>& lines)
{
    for (size_t i = 0; i < size_t(JewelryEffectId::END); ++i)
    {
        effectList_[i] = ItemId(toInt(lines.front()));
        lines.erase(begin(lines));
        effectsKnown_[i] = lines.front() == "1";
        lines.erase(begin(lines));
    }
}

} //JewelryHandling
