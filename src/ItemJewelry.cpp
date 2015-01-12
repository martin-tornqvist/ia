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
bool    effectKnown_  [int(JewelryEffectId::END)];

JewelryEffect* mkEffect(const JewelryEffectId id, Jewelry* const jewelry)
{
  assert(id != JewelryEffectId::END);

  JewelryEffect* ret = nullptr;

  switch (id)
  {
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

    case JewelryEffectId::rFire:
      ret = new JewelryEffectRFire(jewelry);
      break;

    case JewelryEffectId::teleControl:
      ret = new JewelryEffectTeleControl(jewelry);
      break;

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
  const size_t EFFECT_IDX = size_t(getEffectId());

  if (!effectKnown_[EFFECT_IDX])
  {
    effectKnown_[EFFECT_IDX] = true;

    const string name = jewelry_->getName(ItemRefType::plain, ItemRefInf::none);

    Log::addMsg("I gained new knowledge about the " + name + ".", clrWhite, false, true);
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

void JewelryEffectHpBon::onUnequip()
{
  Map::player->changeMaxHp(-4, true);
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

void JewelryEffectHpPen::onUnequip()
{
  Map::player->changeMaxHp(2, true);
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

void JewelryEffectSpiBon::onUnequip()
{
  Map::player->changeMaxSpi(-2, true);
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

void JewelryEffectSpiPen::onUnequip()
{
  Map::player->changeMaxSpi(2, true);
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

void JewelryEffectRFire::onUnequip()
{
  for (Prop* prop : jewelry_->carrierProps_) {delete prop;}
  jewelry_->carrierProps_.clear();

  const auto&   propData  = PropData::data[size_t(PropId::rFire)];
  const string  msg       = propData.msg[propMsgOnEndPlayer];

  Log::addMsg(msg);
}

//--------------------------------------------------------- EFFECT: TELEPORT CONTROL
string JewelryEffectTeleControl::getDescr() const
{
  return "JewelryEffectTeleControl.";
}

void JewelryEffectTeleControl::onEquip()
{
  jewelry_->carrierProps_.push_back(new PropTeleControl(PropTurns::indefinite));

  const auto&   propData  = PropData::data[size_t(PropId::teleControl)];
  const string  msg       = propData.msg[propMsgOnStartPlayer];

  Log::addMsg(msg);

  reveal();
}

void JewelryEffectTeleControl::onUnequip()
{
  for (Prop* prop : jewelry_->carrierProps_) {delete prop;}
  jewelry_->carrierProps_.clear();

  const auto&   propData  = PropData::data[size_t(PropId::teleControl)];
  const string  msg       = propData.msg[propMsgOnEndPlayer];

  Log::addMsg(msg);
}

//--------------------------------------------------------- EFFECT: RANDOM TELEPORTATION
string JewelryEffectRandomTele::getDescr() const
{
  return "JewelryEffectRandomTele.";
}

void JewelryEffectRandomTele::onNewTurnEquiped()
{
  auto& propHandler = Map::player->getPropHandler();

  if (Rnd::oneIn(150) && propHandler.allowAct())
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

void JewelryEffectLight::onUnequip()
{
  for (Prop* prop : jewelry_->carrierProps_) {delete prop;}
  jewelry_->carrierProps_.clear();

  GameTime::updateLightMap();
  Map::player->updateFov();
  Render::drawMapAndInterface();

  const auto&   propData  = PropData::data[size_t(PropId::radiant)];
  const string  msg       = propData.msg[propMsgOnEndPlayer];

  Log::addMsg(msg);
}

//--------------------------------------------------------- EFFECT: CONFLICT
string JewelryEffectConflict::getDescr() const
{
  return "JewelryEffectConflict.";
}

void JewelryEffectConflict::onNewTurnEquiped()
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

void JewelryEffectSpellReflect::onUnequip()
{
  for (Prop* prop : jewelry_->carrierProps_) {delete prop;}
  jewelry_->carrierProps_.clear();

  const auto&   propData  = PropData::data[size_t(PropId::spellReflect)];
  const string  msg       = propData.msg[propMsgOnEndPlayer];

  Log::addMsg(msg);
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
    const size_t EFFECT_IDX = size_t(effect->getEffectId());

    if (effectKnown_[EFFECT_IDX])
    {
      ret.push_back(effect->getDescr());
    }
  }

  return ret;
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
  for (auto* const effect : effects_)
  {
    effect->onUnequip();
  }

  Log::morePrompt();

  return UnequipAllowed::yes;
}

void Jewelry::onNewTurnInInv(const InvType invType)
{
  if (invType == InvType::slots)
  {
    for (auto* const effect : effects_)
    {
      effect->onNewTurnEquiped();
    }
  }
}

//--------------------------------------------------------- JEWELRY HANDLING
namespace JewelryHandling
{

void init()
{
  //Reset the effect list and knowledge status list
  for (size_t i = 0; i < int(JewelryEffectId::END); ++i)
  {
    effectList_   [i] = ItemId::END;
    effectKnown_  [i] = false;
  }

  vector<JewelryEffectId> effectBucket;

  effectBucket.reserve(int(JewelryEffectId::END));

  for (int i = 0; i < int(JewelryEffectId::END); ++i)
  {
    effectBucket.push_back(JewelryEffectId(i));
  }

  vector<ItemId> itemIdBucket;

  const int MAX_NR_EFFECTS_PER_ITEM = 2;

  for (auto* data : ItemData::data)
  {
    if (data->type == ItemType::amulet || data->type == ItemType::ring)
    {
      //Note: Some amulets and rings are added multiple times to the bucket. These can
      //get multiple effects assigned.

      const int NR = Rnd::range(1, MAX_NR_EFFECTS_PER_ITEM);

      itemIdBucket.insert(end(itemIdBucket), NR, data->id);
    }
  }

  while (true)
  {
    //No more effects left to assign?
    if (effectBucket.empty())
    {
      //Mark each amulet and ring that has not yet had any effects assigned to them as
      //forbidden items.
      for (auto* data : ItemData::data)
      {
        if ((data->type == ItemType::amulet || data->type == ItemType::ring) &&
            find(begin(effectList_), end(effectList_), data->id) == end(effectList_)
           )
        {
          data->allowSpawn = false;
        }
      }
      break;
    }

    //No more items left to assign effects to?
    if (itemIdBucket.empty())
    {
      break;
    }

    //Assign an effect to an item
    const size_t  ITEM_IDX      = Rnd::range(0, itemIdBucket.size() - 1);
    const size_t  EFFECT_IDX    = Rnd::range(0, effectBucket.size() - 1);
    const auto    itemId        = itemIdBucket[ITEM_IDX];
    const auto    effectId      = effectBucket[EFFECT_IDX];

    effectList_[int(effectId)]  = itemId;

    itemIdBucket.erase(begin(itemIdBucket) + ITEM_IDX);
    effectBucket.erase(begin(effectBucket) + EFFECT_IDX);
  }
}

void storeToSaveLines(vector<string>& lines)
{
  for (size_t i = 0; i < size_t(JewelryEffectId::END); ++i)
  {
    lines.push_back(toStr(int(effectList_[i])));
    lines.push_back(effectKnown_[i] ? "1" : "0");
  }
}

void setupFromSaveLines(vector<string>& lines)
{
  for (size_t i = 0; i < size_t(JewelryEffectId::END); ++i)
  {
    effectList_[i] = ItemId(toInt(lines.front()));
    lines.erase(begin(lines));
    effectKnown_[i] = lines.front() == "1";
    lines.erase(begin(lines));
  }
}

} //JewelryHandling
