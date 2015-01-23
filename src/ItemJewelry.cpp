#include "Init.h"

#include "ItemJewelry.h"

#include "Map.h"
#include "Utils.h"
#include "Log.h"
#include "GameTime.h"
#include "Render.h"
#include "ActorMon.h"

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
    case JewelryEffectId::rFire:
        ret = new JewelryEffectRFire(jewelry);
        break;

    case JewelryEffectId::rCold:
        ret = new JewelryEffectRCold(jewelry);
        break;

    case JewelryEffectId::rElec:
        ret = new JewelryEffectRElec(jewelry);
        break;

    case JewelryEffectId::rPoison:
        ret = new JewelryEffectRPoison(jewelry);
        break;

    case JewelryEffectId::rDisease:
        ret = new JewelryEffectRDisease(jewelry);
        break;

    case JewelryEffectId::teleCtrl:
        ret = new JewelryEffectTeleControl(jewelry);
        break;

    case JewelryEffectId::light:
        ret = new JewelryEffectLight(jewelry);
        break;

    case JewelryEffectId::spellReflect:
        ret = new JewelryEffectSpellReflect(jewelry);
        break;

    case JewelryEffectId::hpBon:
        ret = new JewelryEffectHpBon(jewelry);
        break;

    case JewelryEffectId::hpPen:
        ret = new JewelryEffectHpPen(jewelry);
        break;

    case JewelryEffectId::spiBon:
        ret = new JewelryEffectSpiBon(jewelry);
        break;

    case JewelryEffectId::spiPen:
        ret = new JewelryEffectSpiPen(jewelry);
        break;

    case JewelryEffectId::randomTele:
        ret = new JewelryEffectRandomTele(jewelry);
        break;

    case JewelryEffectId::conflict:
        ret = new JewelryEffectConflict(jewelry);
        break;

    case JewelryEffectId::burden:
        ret = new JewelryEffectBurden(jewelry);
        break;

    case JewelryEffectId::noise:
        ret = new JewelryEffectNoise(jewelry);
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

//--------------------------------------------------------- JEWELRY PROPERTY EFFECT
void JewelryPropertyEffect::onEquip()
{
    Prop* const prop = mkProp();

    assert(prop);

    jewelry_->carrierProps_.push_back(prop);

    GameTime::updateLightMap();
    Map::player->updateFov();
    Render::drawMapAndInterface();

    const auto&   propData  = PropData::data[size_t(prop->getId())];
    const string  msg       = propData.msg[propMsgOnStartPlayer];

    Log::addMsg(msg);

    reveal();
}

UnequipAllowed JewelryPropertyEffect::onUnequip()
{
    vector<PropId> propIdsEnded;

    for (Prop* const prop : jewelry_->carrierProps_)
    {
        propIdsEnded.push_back(prop->getId());
        delete prop;
    }
    jewelry_->carrierProps_.clear();

    GameTime::updateLightMap();
    Map::player->updateFov();
    Render::drawMapAndInterface();

    for (const PropId propId : propIdsEnded)
    {
        const auto&     propData    = PropData::data[size_t(propId)];
        const string    msg         = propData.msg[propMsgOnEndPlayer];
        Log::addMsg(msg);
    }

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: FIRE RESISTANCE
Prop* JewelryEffectRFire::mkProp() const
{
    return new PropRFire(PropTurns::indefinite);
}

//--------------------------------------------------------- EFFECT: COLD RESISTANCE
Prop* JewelryEffectRCold::mkProp() const
{
    return new PropRCold(PropTurns::indefinite);
}

//--------------------------------------------------------- EFFECT: ELEC RESISTANCE
Prop* JewelryEffectRElec::mkProp() const
{
    return new PropRElec(PropTurns::indefinite);
}

//--------------------------------------------------------- EFFECT: POISON RESISTANCE
Prop* JewelryEffectRPoison::mkProp() const
{
    return new PropRPoison(PropTurns::indefinite);
}

//--------------------------------------------------------- EFFECT: DISEASE RESISTANCE
Prop* JewelryEffectRDisease::mkProp() const
{
    return new PropRDisease(PropTurns::indefinite);
}

//--------------------------------------------------------- EFFECT: TELEPORT CONTROL
Prop* JewelryEffectTeleControl::mkProp() const
{
    return new PropTeleControl(PropTurns::indefinite);
}

//--------------------------------------------------------- EFFECT: LIGHT
Prop* JewelryEffectLight::mkProp() const
{
    return new PropRadiant(PropTurns::indefinite);
}

//--------------------------------------------------------- EFFECT: SPELL REFLECTION
Prop* JewelryEffectSpellReflect::mkProp() const
{
    return new PropSpellReflect(PropTurns::indefinite);
}

//--------------------------------------------------------- EFFECT: HP BONUS
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

//--------------------------------------------------------- EFFECT: RANDOM TELEPORTATION
void JewelryEffectRandomTele::onStdTurnEquiped()
{
    auto& propHandler = Map::player->getPropHandler();

    const int TELE_ON_IN_N = 200;

    if (Rnd::oneIn(TELE_ON_IN_N) && propHandler.allowAct())
    {
        Log::addMsg("I am being teleported...", clrWhite, true, true);
        Map::player->teleport();
        reveal();
    }
}

//--------------------------------------------------------- EFFECT: CONFLICT
void JewelryEffectConflict::onStdTurnEquiped()
{
    const int CONFLICT_ONE_IN_N = 15;

    if (Rnd::oneIn(CONFLICT_ONE_IN_N))
    {
        vector<Actor*> seenFoes;
        Map::player->getSeenFoes(seenFoes);

        if (!seenFoes.empty())
        {
            const int   IDX = Rnd::range(0, seenFoes.size() - 1);
            auto*       mon = static_cast<Mon*>(seenFoes[IDX]);

            mon->getPropHandler().tryApplyProp(new PropConflict(PropTurns::std));

            reveal();
        }
    }
}

//--------------------------------------------------------- EFFECT: NOISE
void JewelryEffectNoise::onStdTurnEquiped()
{
    const int NOISE_ONE_IN_N = 150;

    if (Rnd::oneIn(NOISE_ONE_IN_N))
    {
        const string name = jewelry_->getName(ItemRefType::plain, ItemRefInf::none);

        SndEmit::emitSnd(Snd("The " + name + " makes a loud noise!", SfxId::END,
                             IgnoreMsgIfOriginSeen::no, Map::player->pos, Map::player,
                             SndVol::high, AlertsMon::yes
                            ));

        Log::morePrompt();

        reveal();
    }
}

//--------------------------------------------------------- EFFECT: BURDEN
void JewelryEffectBurden::onEquip()
{
    if (!effectsKnown_[size_t(getId())])
    {
        const string name = jewelry_->getName(ItemRefType::plain, ItemRefInf::none);
        Log::addMsg("The " + name + " suddenly feels strangely heavy to carry.");
        reveal();
    }
}

void JewelryEffectBurden::changeItemWeight(int& weightRef)
{
    if (effectsKnown_[size_t(getId())])
    {
        //If revealed, this item weighs the average of "heavy" and "medium"
        weightRef = (int(ItemWeight::heavy) + int(ItemWeight::medium)) / 2;
    }
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

int Jewelry::getWeight() const
{
    int weight = Item::getWeight();

    for (auto* effect : effects_)
    {
        effect->changeItemWeight(weight);
    }

    return weight;
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
    case Id::rCold:         return true;
    case Id::rElec:         return true;
    case Id::rPoison:       return true;
    case Id::rDisease:      return true;
    case Id::teleCtrl:      return id2 != Id::randomTele && id2 != Id::spellReflect;
    case Id::randomTele:    return id2 != Id::teleCtrl;
    case Id::light:         return true;
    case Id::conflict:      return true;
    case Id::spellReflect:  return id2 != Id::teleCtrl;
    case Id::burden:        return true;
    case Id::noise:         return true;
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

    //Assign random effects
    vector<JewelryEffectId> effectBucket;

    effectBucket.reserve(int(JewelryEffectId::END));

    for (size_t i = 0; i < size_t(JewelryEffectId::END); ++i)
    {
        effectBucket.push_back(JewelryEffectId(i));
    }

    vector<ItemId> itemBucket;

    const int MAX_NR_EFFECTS_PER_ITEM = 2;

    for (auto* data : ItemData::data)
    {
        const auto id   = data->id;
        const auto type = data->type;

        if (type == ItemType::amulet || type == ItemType::ring)
        {
            //Add all amulets and rings twice to the bucket, so they can get multiple
            //effects assigned.
            itemBucket.insert(end(itemBucket), MAX_NR_EFFECTS_PER_ITEM, id);
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
