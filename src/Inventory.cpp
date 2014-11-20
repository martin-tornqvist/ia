#include "Inventory.h"

#include <algorithm>
#include <vector>

#include "Init.h"
#include "Item.h"
#include "ItemDrop.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "GameTime.h"
#include "Render.h"
#include "ItemFactory.h"
#include "CreateCharacter.h"
#include "DungeonMaster.h"
#include "PlayerBon.h"
#include "Map.h"

using namespace std;

Inventory::Inventory()
{
  slots_[int(SlotId::wielded)]    = InvSlot(SlotId::wielded,    "Wielding");
  slots_[int(SlotId::wieldedAlt)] = InvSlot(SlotId::wieldedAlt, "Prepared");
  slots_[int(SlotId::thrown)]     = InvSlot(SlotId::thrown,     "Thrown");
  slots_[int(SlotId::body)]       = InvSlot(SlotId::body,       "On body");
  slots_[int(SlotId::head)]       = InvSlot(SlotId::head,       "On head");
}

Inventory::~Inventory()
{
  for (size_t i = 0; i < int(SlotId::END); ++i)
  {
    auto& slot = slots_[i];
    if (slot.item) {delete slot.item;}
  }
  for (Item* item : general_)    {delete item;}
  for (Item* item : intrinsics_) {delete item;}
}

void Inventory::storeToSaveLines(vector<string>& lines) const
{
  for (const InvSlot& slot : slots_)
  {
    Item* const item = slot.item;
    if (item)
    {
      lines.push_back(toStr(int(item->getData().id)));
      lines.push_back(toStr(item->nrItems_));
      item->storeToSaveLines(lines);
    }
    else
    {
      lines.push_back(toStr(int(ItemId::END)));
    }
  }

  lines.push_back(toStr(general_.size()));
  for (Item* item : general_)
  {
    lines.push_back(toStr(int(item->getData().id)));
    lines.push_back(toStr(item->nrItems_));
    item->storeToSaveLines(lines);
  }
}

void Inventory::setupFromSaveLines(vector<string>& lines)
{
  for (InvSlot& slot : slots_)
  {
    //Previous item is destroyed
    Item* item = slot.item;
    if (item)
    {
      delete item;
      slot.item = nullptr;
    }

    const ItemId id = ItemId(toInt(lines.front()));
    lines.erase(begin(lines));
    if (id != ItemId::END)
    {
      item = ItemFactory::mk(id);
      item->nrItems_ = toInt(lines.front());
      lines.erase(begin(lines));
      item->setupFromSaveLines(lines);
      slot.item = item;
      //When loading the game, wear the item to apply properties from wearing
      item->onWear();
    }
  }

  while (general_.size() != 0)
  {
    deleteItemInGeneralWithElement(0);
  }

  const int NR_OF_GENERAL = toInt(lines.front());
  lines.erase(begin(lines));
  for (int i = 0; i < NR_OF_GENERAL; ++i)
  {
    const ItemId id = ItemId(toInt(lines.front()));
    lines.erase(begin(lines));
    Item* item = ItemFactory::mk(id);
    item->nrItems_ = toInt(lines.front());
    lines.erase(begin(lines));
    item->setupFromSaveLines(lines);
    general_.push_back(item);
  }
}

bool Inventory::hasDynamiteInGeneral() const
{
  return hasItemInGeneral(ItemId::dynamite);
}

bool Inventory::hasItemInGeneral(const ItemId id) const
{
  for (size_t i = 0; i < general_.size(); ++i)
  {
    if (general_[i]->getData().id == id)
      return true;
  }

  return false;
}

int Inventory::getItemStackSizeInGeneral(const ItemId id) const
{
  for (size_t i = 0; i < general_.size(); ++i)
  {
    if (general_[i]->getData().id == id)
    {
      if (general_[i]->getData().isStackable)
      {
        return general_[i]->nrItems_;
      }
      else
      {
        return 1;
      }
    }
  }

  return 0;
}

void Inventory::decrDynamiteInGeneral()
{
  for (size_t i = 0; i < general_.size(); ++i)
  {
    if (general_[i]->getData().id == ItemId::dynamite)
    {
      decrItemInGeneral(i);
      break;
    }
  }
}

/*
 bool Inventory::hasFirstAidInGeneral()
 {
 for(size_t i = 0; i < general_.size(); ++i) {
 if(general_[i]->getInstanceDefinition().id == ItemId::firstAidKit)
 return true;
 }

 return false;
 }

 void Inventory::decreaseFirstAidInGeneral()
 {
 for(size_t i = 0; i < general_.size(); ++i) {
 if(general_[i]->getInstanceDefinition().id == ItemId::firstAidKit) {
 decrItemInGeneral(i);
 break;
 }
 }
 }
 */

void Inventory::putInGeneral(Item* item)
{
  bool isStacked = false;

  //If item stacks, see if there is other items of same type
  if (item->getData().isStackable)
  {

    const int stackIndex = getElementToStackItem(item);

    if (stackIndex != -1)
    {
      Item* compareItem = general_[stackIndex];

      //Keeping picked up item and destroying the one in the inventory,
      //to keep the parameter pointer valid.
      item->nrItems_ += compareItem->nrItems_;
      delete compareItem;
      general_[stackIndex] = item;
      isStacked = true;
    }
  }

  if (!isStacked) {general_.push_back(item);}
}

int Inventory::getElementToStackItem(Item* item) const
{
  if (item->getData().isStackable)
  {
    for (size_t i = 0; i < general_.size(); ++i)
    {
      Item* compare = general_[i];

      if (compare->getData().id == item->getData().id)
      {
        return i;
      }
    }
  }

  return -1;
}

void Inventory::dropAllNonIntrinsic(
  const Pos& pos, const bool ROLL_FOR_DESTRUCTION)
{

  Item* item;

  //Drop from slots
  for (InvSlot& slot : slots_)
  {
    item = slot.item;
    if (item)
    {
      if (ROLL_FOR_DESTRUCTION && Rnd::percentile() <
          CHANCE_TO_DESTR_CMN_ITEMS_ON_DROP)
      {
        delete slot.item;
      }
      else
      {
        ItemDrop::dropItemOnMap(pos, *item);
      }

      slot.item = nullptr;
    }
  }

  //Drop from general
  size_t i = 0;
  while (i < general_.size())
  {
    item = general_[i];
    if (item)
    {
      if (ROLL_FOR_DESTRUCTION && Rnd::percentile() < CHANCE_TO_DESTR_CMN_ITEMS_ON_DROP)
      {
        delete general_[i];
      }
      else
      {
        ItemDrop::dropItemOnMap(pos, *item);
      }

      general_.erase(begin(general_) + i);
    }
    i++;
  }
}

bool Inventory::hasAmmoForFirearmInInventory()
{
  Wpn* weapon = static_cast<Wpn*>(getItemInSlot(SlotId::wielded));

  //If weapon found
  if (weapon)
  {

    assert(!weapon->getData().ranged.hasInfiniteAmmo); //Should not happen

    //If weapon is a firearm
    if (weapon->getData().ranged.isRangedWpn)
    {

      //Get weapon ammo type
      const ItemId ammoId = weapon->getData().ranged.ammoItemId;

      //Look for that ammo type in inventory
      for (size_t i = 0; i < general_.size(); ++i)
      {
        if (general_[i]->getData().id == ammoId)
        {
          return true;
        }
      }
    }
  }
  return false;
}

void Inventory::decrItemInSlot(SlotId slotName)
{
  Item* item = getItemInSlot(slotName);
  bool stack = item->getData().isStackable;
  bool deleteItem = true;

  if (stack)
  {
    item->nrItems_ -= 1;

    if (item->nrItems_ > 0)
    {
      deleteItem = false;
    }
  }

  if (deleteItem)
  {
    slots_[int(slotName)].item = nullptr;
    delete item;
  }
}

void Inventory::deleteItemInGeneralWithElement(const size_t IDX)
{
  if (general_.size() > IDX)
  {
    delete general_[IDX];
    general_.erase(begin(general_) + IDX);
  }
}

void Inventory::removeItemInGeneralWithPtr(
  Item* const item, const bool DELETE_ITEM)
{

  for (size_t i = 0; i < general_.size(); ++i)
  {
    if (general_[i] == item)
    {
      if (DELETE_ITEM) {delete item;}
      general_.erase(begin(general_) + i);
      return;
    }
  }
  assert(false && "Parameter item not in general inventory");
}

void Inventory::decrItemInGeneral(size_t idx)
{
  Item* item = general_[idx];
  bool stack = item->getData().isStackable;
  bool deleteItem = true;

  if (stack)
  {
    item->nrItems_ -= 1;

    if (item->nrItems_ > 0)
    {
      deleteItem = false;
    }
  }

  if (deleteItem)
  {
    general_.erase(begin(general_) + idx);

    delete item;
  }
}

void Inventory::decrItemTypeInGeneral(const ItemId id)
{
  for (size_t i = 0; i < general_.size(); ++i)
  {
    if (general_[i]->getData().id == id)
    {
      decrItemInGeneral(i);
      return;
    }
  }
}

void Inventory::moveItemToSlot(InvSlot& slot, const size_t GEN_IDX)
{
  bool generalSlotExists  = GEN_IDX < general_.size();
  Item* item              = nullptr;
  Item* slotItem          = slot.item;

  if (generalSlotExists) {item = general_[GEN_IDX];}

  if (generalSlotExists && item)
  {
    if (slotItem)
    {
      general_.erase(begin(general_) + GEN_IDX);
      general_.push_back(slotItem);
      slot.item = item;
    }
    else
    {
      slot.item = item;
      general_.erase(begin(general_) + GEN_IDX);
    }
  }
}

void Inventory::equipGeneralItemAndEndTurn(const size_t GEN_IDX, const SlotId slot)
{
  moveItemToSlot(slots_[int(slot)], GEN_IDX);

  bool isFreeTurn = false;

  const bool IS_PLAYER  = this == &Map::player->getInv();

  if (IS_PLAYER)
  {
    Item* const itemAfter = getItemInSlot(slot);
    const string name     = itemAfter->getName(ItemRefType::plural);

    string  msg     = "";

    switch (slot)
    {
      case SlotId::wielded:
      {
        msg = "I am now wielding " + name + ".";
      } break;

      case SlotId::wieldedAlt:
      {
        msg = "I am now wielding " + name + " as a prepared weapon.";
      } break;

      case SlotId::thrown:
      {
        msg = "I am now using " + name + " as missile weapon.";
      } break;

      case SlotId::body:
      {
        msg = "I am now wearing " + name + ".";
        isFreeTurn = false;
      } break;

      case SlotId::head:
      {
        msg = "I am now wearing " + name + ".";
      } break;

      case SlotId::END:
      {
        TRACE << "Illegal slot id: " << int(slot) << endl;
        assert(false);
      } break;
    }

    Log::addMsg(msg, clrWhite, false, true);
  }

  GameTime::actorDidAct(isFreeTurn);
}

void Inventory::swapWieldedAndPrepared(
  const bool IS_FREE_TURN)
{

  auto& slot1 = slots_[int(SlotId::wielded)];
  auto& slot2 = slots_[int(SlotId::wieldedAlt)];
  Item* item1 = slot1.item;
  Item* item2 = slot2.item;
  slot1.item  = item2;
  slot2.item  = item1;

  Render::drawMapAndInterface();

  GameTime::actorDidAct(IS_FREE_TURN);
}

void Inventory::moveFromGeneralToIntrinsics(const size_t GEN_IDX)
{
  bool generalSlotExists = GEN_IDX < general_.size();

  if (generalSlotExists)
  {
    Item* item = general_[GEN_IDX];
    bool itemExistsInGeneralSlot = item;

    if (itemExistsInGeneralSlot)
    {
      intrinsics_.push_back(item);
      general_.erase(begin(general_) + GEN_IDX);
    }
  }
}

bool Inventory::moveToGeneral(InvSlot& slot)
{
  Item* const item = slot.item;
  if (item)
  {
    slot.item = nullptr;
    putInGeneral(item);
    return true;
  }
  else
  {
    return false;
  }
}

bool Inventory::hasItemInSlot(SlotId id) const
{
  assert(id != SlotId::END && "Illegal slot id");
  return slots_[int(id)].item;
}

void Inventory::removeWithoutDestroying(const InvList invList, const size_t IDX)
{
  if (invList == InvList::slots)
  {
    assert(IDX != int(SlotId::END));
    slots_[IDX].item = nullptr;
  }
  else
  {
    assert(IDX < general_.size());
    general_.erase(begin(general_) + IDX);
  }
}

int Inventory::getElementWithItemType(const ItemId id) const
{
  for (size_t i = 0; i < general_.size(); ++i)
  {
    if (general_[i]->getData().id == id)
    {
      return i;
    }
  }
  return -1;
}

Item* Inventory::getItemInSlot(SlotId id) const
{
  assert(id != SlotId::END && "Illegal slot id");
  return slots_[int(id)].item;
}

Item* Inventory::getIntrinsicInElement(int idx) const
{
  if (getIntrinsicsSize() > idx)
  {
    return intrinsics_[idx];
  }

  return nullptr;
}

void Inventory::putInIntrinsics(Item* item)
{
  if (item->getData().isIntrinsic)
  {
    intrinsics_.push_back(item);
  }
  else
  {
    TRACE << "Tried to put non-intrinsic weapon in intrinsics" << endl;
    assert(false);
  }
}

Item* Inventory::getLastItemInGeneral()
{
  if (!general_.empty()) {return general_[general_.size() - 1];}
  return nullptr;
}

void Inventory::putInSlot(const SlotId id, Item* item)
{
  for (InvSlot& slot : slots_)
  {
    if (slot.id == id)
    {
      if (slot.item)
      {
        general_.push_back(item);
      }
      else
      {
        slot.item = item;
      }
      return;
    }
  }

  general_.push_back(item);
  assert(false && "Bad slot id");
}

int Inventory::getTotalItemWeight() const
{
  int weight = 0;
  for (size_t i = 0; i < size_t(SlotId::END); ++i)
  {
    if (slots_[i].item) {weight += slots_[i].item->getWeight();}
  }
  for (size_t i = 0; i < general_.size(); ++i)
  {
    weight += general_[i]->getWeight();
  }
  return weight;
}

// Function for lexicographically comparing two items
struct LexicograhicalCompareItems
{
public:
  bool operator()(const Item* const item1, const Item* const item2)
  {
    const string& itemName1 = item1->getName(ItemRefType::plain);
    const string& itemName2 = item2->getName(ItemRefType::plain);
    return lexicographical_compare(itemName1.begin(), itemName1.end(),
                                   itemName2.begin(), itemName2.end());
  }
};

void Inventory::sortGeneralInventory()
{
  vector< vector<Item*> > sortBuffer;

  //Sort according to item interface color first
  for (Item* item : general_)
  {

    bool isAddedToBuffer = false;

    //Check if item should be added to any existing color group
    for (vector<Item*>& group : sortBuffer)
    {
      const Clr clrCurGroup = group[0]->getInterfaceClr();
      if (Utils::isClrEq(item->getInterfaceClr(), clrCurGroup))
      {
        group.push_back(item);
        isAddedToBuffer = true;
        break;
      }
    }

    if (isAddedToBuffer)
    {
      continue;
    }
    else
    {
      //Item is a new color, create a new color group
      vector<Item*> newGroup;
      newGroup.push_back(item);
      sortBuffer.push_back(newGroup);
    }
  }

  //Sort lexicographically secondarily
  LexicograhicalCompareItems cmp;
  for (vector<Item*>& group : sortBuffer)
  {
    std::sort(group.begin(), group.end(), cmp);
  }

  //Set the inventory from the sorting buffer
  general_.clear();
  for (size_t i = 0; i < sortBuffer.size(); ++i)
  {
    for (size_t ii = 0; ii < sortBuffer[i].size(); ii++)
    {
      general_.push_back(sortBuffer[i][ii]);
    }
  }
}
