#include "Init.h"

#include "ItemJewelry.h"

#include "Map.h"
#include "Utils.h"
#include "Log.h"
#include "GameTime.h"
#include "Render.h"
#include "ActorMon.h"
#include "TextFormat.h"
#include "ActorFactory.h"
#include "FeatureRigid.h"

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

    case JewelryEffectId::summon:
        ret = new JewelryEffectSummonMon(jewelry);
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

    case JewelryEffectId::fire:
        ret = new JewelryEffectFire(jewelry);
        break;

    case JewelryEffectId::conflict:
        ret = new JewelryEffectConflict(jewelry);
        break;

    case JewelryEffectId::burden:
        ret = new JewelryEffectBurden(jewelry);
        break;

    case JewelryEffectId::hpRegenBon:
        ret = new JewelryEffectHpRegenBon(jewelry);
        break;

    case JewelryEffectId::hpRegenPen:
        ret = new JewelryEffectHpRegenPen(jewelry);
        break;

    case JewelryEffectId::haste:
        ret = new JewelryEffectHaste(jewelry);
        break;

    case JewelryEffectId::shriek:
        ret = new JewelryEffectShriek(jewelry);
        break;

    case JewelryEffectId::START_OF_SECONDARY_EFFECTS:
    case JewelryEffectId::END: {} break;
    }

    assert(ret);
    return ret;
}

} //namespace

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

    jewelry_->effectNoticed(getId());
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

//--------------------------------------------------------- EFFECT: HASTE
Prop* JewelryEffectHaste::mkProp() const
{
    return new PropHasted(PropTurns::indefinite);
}

//--------------------------------------------------------- EFFECT: HP BONUS
void JewelryEffectHpBon::onEquip()
{
    Map::player->changeMaxHp(4, true);
    jewelry_->effectNoticed(getId());
}

UnequipAllowed JewelryEffectHpBon::onUnequip()
{
    Map::player->changeMaxHp(-4, true);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: HP PENALTY
void JewelryEffectHpPen::onEquip()
{
    Map::player->changeMaxHp(-4, true);
    jewelry_->effectNoticed(getId());
}

UnequipAllowed JewelryEffectHpPen::onUnequip()
{
    Map::player->changeMaxHp(4, true);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: SPI BONUS
void JewelryEffectSpiBon::onEquip()
{
    Map::player->changeMaxSpi(4, true);
    jewelry_->effectNoticed(getId());
}

UnequipAllowed JewelryEffectSpiBon::onUnequip()
{
    Map::player->changeMaxSpi(-4, true);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: SPI PENALTY
void JewelryEffectSpiPen::onEquip()
{
    Map::player->changeMaxSpi(-4, true);
    jewelry_->effectNoticed(getId());
}

UnequipAllowed JewelryEffectSpiPen::onUnequip()
{
    Map::player->changeMaxSpi(4, true);

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
        jewelry_->effectNoticed(getId());
    }
}

//--------------------------------------------------------- EFFECT: SUMMON MON
void JewelryEffectSummonMon::onStdTurnEquiped()
{
    const int SOUND_ONE_IN_N  = 250;

    if (effectsKnown_[size_t(getId())] && Rnd::oneIn(SOUND_ONE_IN_N))
    {
        Log::addMsg("I hear a faint whistling sound coming nearer...", clrWhite, false,
                    true);

        Map::player->incrShock(ShockLvl::mild, ShockSrc::misc);
    }

    const int SUMMON_ONE_IN_N = 1200;

    if (Rnd::oneIn(SUMMON_ONE_IN_N))
    {
        Log::addMsg("There is a loud whistling sound.", clrWhite, false, true);

        const Pos origin = Map::player->pos;

        vector<Mon*> summonedMon;

        ActorFactory::summon(origin, {ActorId::greaterPolyp}, false, nullptr,
                                &summonedMon);

        const Mon* const mon = summonedMon[0];

        if (Map::player->canSeeActor(*mon, nullptr))
        {
            Log::addMsg(mon->getNameA() + " appears!", clrWhite, true, true);
        }

        jewelry_->effectNoticed(getId());
    }
}

//--------------------------------------------------------- EFFECT: FIRE
void JewelryEffectFire::onStdTurnEquiped()
{
    const int FIRE_ONE_IN_N = 300;

    if (Rnd::oneIn(FIRE_ONE_IN_N))
    {
        const Pos& origin   = Map::player->pos;
        const int D         = FOV_STD_RADI_INT - 2;
        const int X0        = max(1,            origin.x - D);
        const int Y0        = max(1,            origin.y - D);
        const int X1        = min(MAP_W - 2,    origin.x + D);
        const int Y1        = min(MAP_H - 2,    origin.y + D);

        const int FIRE_CELL_ONE_IN_N = 4;

        Log::addMsg("The surrounding area suddenly burst into flames!", clrWhite,
                    false, true);

        for (int x = X0; x <= X1; ++x)
        {
            for (int y = Y0; y <= Y1; ++y)
            {
                if (Rnd::oneIn(FIRE_CELL_ONE_IN_N) && Pos(x, y) != origin)
                {
                    Map::cells[x][y].rigid->hit(DmgType::fire, DmgMethod::elemental);
                }
            }
        }

        jewelry_->effectNoticed(getId());
    }
}

//--------------------------------------------------------- EFFECT: CONFLICT
void JewelryEffectConflict::onStdTurnEquiped()
{
    const int CONFLICT_ONE_IN_N = 50;

    if (Rnd::oneIn(CONFLICT_ONE_IN_N))
    {
        vector<Actor*> seenFoes;
        Map::player->getSeenFoes(seenFoes);

        if (!seenFoes.empty())
        {
            random_shuffle(begin(seenFoes), end(seenFoes));

            for (Actor* actor : seenFoes)
            {
                if (!actor->getData().isUnique)
                {
                    actor->getPropHandler().tryApplyProp(
                        new PropConflict(PropTurns::std));

                    jewelry_->effectNoticed(getId());

                    break;
                }
            }
        }
    }
}

//--------------------------------------------------------- EFFECT: SHRIEK
JewelryEffectShriek::JewelryEffectShriek(Jewelry* const jewelry) :
    JewelryEffect(jewelry)
{
    string playerName = Map::player->getNameThe();

    TextFormat::allToUpper(playerName);

    words_ =
    {
        playerName,
        playerName,
        playerName,
        "BHUUDESCO",
        "STRAGARANA",
        "INFIRMUX",
        "BHAAVA",
        "CRUENTO",
        "PRETIACRUENTO",
        "VILOMAXUS",
        "DEATH",
        "DYING",
        "TAKE",
        "BlOOD",
        "END",
        "SACRIFICE",
        "POSSESSED",
        "PROPHECY",
        "SIGNS",
        "OATH",
        "FATE",
        "SUFFER",
        "BEHOLD",
        "BEWARE",
        "WATCHES",
        "LIGHT",
        "DARK",
        "DISAPPEAR",
        "APPEAR",
        "DECAY",
        "IMMORTAL",
        "BOUNDLESS",
        "ETERNAL",
        "ANCIENT",
        "TIME",
        "NEVER-ENDING",
        "DIMENSIONS",
        "EYES",
        "STARS",
        "GAZE",
        "FORBIDDEN",
        "DOMINIONS",
        "RULER",
        "KING",
        "UNKNOWN",
        "ABYSS",
        "BENEATH",
        "BELOW",
        "PASSAGE",
        "PATH",
        "GATE",
        "SERPENT",
        "NYARLATHOTEP",
        "GOL-GOROTH",
        "ABHOLOS",
        "HASTUR",
        "ISTASHA",
        "ITHAQUA",
        "THOG",
        "TSATHOGGUA",
        "YMNAR",
        "XCTHOL",
        "ZATHOG",
        "ZINDARAK",
        "BASATAN",
        "CTHUGHA"
    };
}

void JewelryEffectShriek::onStdTurnEquiped()
{
    const int NOISE_ONE_IN_N = 300;

    if (Rnd::oneIn(NOISE_ONE_IN_N))
    {
        const string name = jewelry_->getName(ItemRefType::plain, ItemRefInf::none);

        Log::addMsg("The " + name + " shrieks...", clrWhite, false, true);

        const int NR_WORDS = 3;

        string phrase = "";

        for (int i = 0; i < NR_WORDS; ++i)
        {
            const int   IDX     = Rnd::range(0, words_.size() - 1);
            const auto& word    = words_[size_t(IDX)];

            phrase += word;

            if (i < NR_WORDS - 1)
            {
                phrase += " ... ";
            }
        }

        phrase += "!!!";

        SndEmit::emitSnd(Snd(phrase, SfxId::END, IgnoreMsgIfOriginSeen::no,
                             Map::player->pos, Map::player, SndVol::high, AlertsMon::yes
                            ));

        Map::player->incrShock(ShockLvl::mild, ShockSrc::misc);

        Log::morePrompt();

        jewelry_->effectNoticed(getId());
    }
}

//--------------------------------------------------------- EFFECT: BURDEN
void JewelryEffectBurden::onEquip()
{
    if (!effectsKnown_[size_t(getId())])
    {
        Log::addMsg("I suddenly feel more burdened.");
        jewelry_->effectNoticed(getId());
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

//--------------------------------------------------------- EFFECT: HP REGEN BONUS
void JewelryEffectHpRegenBon::onEquip()
{
    Log::addMsg("I heal faster.");
    jewelry_->effectNoticed(getId());
}

UnequipAllowed JewelryEffectHpRegenBon::onUnequip()
{
    Log::addMsg("I heal slower.");
    return UnequipAllowed::yes;
}

//--------------------------------------------------------- EFFECT: HP REGEN PENALTY
void JewelryEffectHpRegenPen::onEquip()
{
    Log::addMsg("I heal slower.");
    jewelry_->effectNoticed(getId());
}

UnequipAllowed JewelryEffectHpRegenPen::onUnequip()
{
    Log::addMsg("I heal faster.");
    return UnequipAllowed::yes;
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

    if (data_->isIdentified)
    {
        const string name = getName(ItemRefType::plain, ItemRefInf::none);

        ret.push_back("All properties of the " + name + " are known to me.");
    }

    ret.push_back(itemCarryShockDescr + " (+15% shock).");

    return ret;
}

string Jewelry::getNameInf() const
{
    return data_->isIdentified ? "{Known}" : "";
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

int Jewelry::getHpRegenChange(const InvType invType) const
{
    int change = 0;

    if (invType == InvType::slots)
    {
        for (auto* effect : effects_)
        {
            change += effect->getHpRegenChange();
        }
    }

    return change;
}

void Jewelry::effectNoticed(const JewelryEffectId effectId)
{
    const size_t EFFECT_IDX = size_t(effectId);

    if (!effectsKnown_[EFFECT_IDX])
    {
        effectsKnown_[EFFECT_IDX] = true;

        const int   MAX_NR_EFFECTS_ON_ITEM  = 2;
        int         nrEffectsKnownThisItem  = 0;

        for (auto* effect : effects_)
        {
            const size_t CHECK_EFFECT_IDX = size_t(effect->getId());

            if (effectsKnown_[CHECK_EFFECT_IDX])
            {
                ++nrEffectsKnownThisItem;
            }
        }

        assert(nrEffectsKnownThisItem <= MAX_NR_EFFECTS_ON_ITEM);

        const string name = getName(ItemRefType::plain, ItemRefInf::none);

        Log::addMsg("I gained new knowledge about the " + name + ".", clrWhite, false,
                    true);

        if (nrEffectsKnownThisItem == MAX_NR_EFFECTS_ON_ITEM)
        {
            Log::addMsg("I feel like all properties of the " + name +
                        " are known to me.", clrWhite, false, true);
            data_->isIdentified = true;
        }

        Map::player->incrShock(ShockLvl::heavy, ShockSrc::useStrangeItem);
    }
}

//--------------------------------------------------------- JEWELRY HANDLING
namespace
{

bool canEffectsBeCombined(const JewelryEffectId id1,
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
    case Id::hpBon:
        return id2 != Id::hpPen;

    case Id::hpPen:
        return id2 != Id::hpBon && id2 != Id::rDisease;

    case Id::spiBon:
        return id2 != Id::spiPen;

    case Id::spiPen:
        return id2 != Id::spiBon;

    case Id::rDisease:
        return id2 != Id::hpPen;

    case Id::teleCtrl:
        return id2 != Id::randomTele && id2 != Id::spellReflect;

    case Id::randomTele:
        return id2 != Id::teleCtrl;

    case Id::spellReflect:
        return id2 != Id::teleCtrl;

    case Id::hpRegenBon:
        return id2 != Id::hpRegenPen && id2 != Id::hpBon && id2 != Id::hpPen;

    case Id::hpRegenPen:
        return id2 != Id::hpRegenBon && id2 != Id::hpBon && id2 != Id::hpPen;

    case Id::summon:
    case Id::light:
    case Id::conflict:
    case Id::rFire:
    case Id::rCold:
    case Id::rElec:
    case Id::rPoison:
    case Id::burden:
    case Id::shriek:
    case Id::haste:
    case Id::fire:
        return true;

    case JewelryEffectId::START_OF_SECONDARY_EFFECTS:
    case Id::END:
        assert(false);
        return false;
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
                !canEffectsBeCombined(effectToAssign, curEffect))
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
    for (size_t i = 0; i < size_t(JewelryEffectId::END); ++i)
    {
        effectList_   [i] = ItemId::END;
        effectsKnown_ [i] = false;
    }

    vector<ItemId> itemBucket;

    for (auto* data : ItemData::data)
    {
        const auto type = data->type;

        if (type == ItemType::amulet || type == ItemType::ring)
        {
            itemBucket.push_back(data->id);
        }
    }
    random_shuffle(begin(itemBucket), end(itemBucket));

    vector<JewelryEffectId> primaryEffectBucket;
    vector<JewelryEffectId> secondaryEffectBucket;

    for (size_t i = 0; i < size_t(JewelryEffectId::END); ++i)
    {
        const auto id = JewelryEffectId(i);

        if (id <  JewelryEffectId::START_OF_SECONDARY_EFFECTS)
        {
            primaryEffectBucket.push_back(id);
        }
        else if (id >  JewelryEffectId::START_OF_SECONDARY_EFFECTS)
        {
            secondaryEffectBucket.push_back(id);
        }
    }
    random_shuffle(begin(primaryEffectBucket),   end(primaryEffectBucket));
    random_shuffle(begin(secondaryEffectBucket), end(secondaryEffectBucket));

    //Assuming there are more jewelry than primary or secondary effects (if this changes,
    //just add more amulets and rings to the item data)
    assert(itemBucket.size() > primaryEffectBucket.size());
    assert(itemBucket.size() > secondaryEffectBucket.size());

    //Assign primary effects
    for (size_t i = 0; i < itemBucket.size(); ++i)
    {
        const auto itemId = itemBucket[i];

        if (i < primaryEffectBucket.size())
        {
            const auto effectId = primaryEffectBucket[i];

            effectList_[size_t(effectId)] = itemId;
        }
        else //All primary effects are assigned
        {
            ItemData::data[size_t(itemId)]->allowSpawn = false;
        }
    }

    //Remove all items without a primary effect from the item bucket
    itemBucket.resize(primaryEffectBucket.size());

    //Assign secondary effects
    for (const auto secondaryEffectId : secondaryEffectBucket)
    {
        const int ITEM_IDX = getRndItemBucketIdxForEffect(secondaryEffectId, itemBucket);

        if (ITEM_IDX >= 0)
        {
            effectList_[size_t(secondaryEffectId)] = itemBucket[size_t(ITEM_IDX)];
            itemBucket.erase(begin(itemBucket) + size_t(ITEM_IDX));
        }
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
