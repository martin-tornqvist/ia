#include "Inventory.h"

#include <algorithm>
#include <vector>

#include "Init.h"
#include "Item.h"
#include "Drop.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "GameTime.h"
#include "Render.h"
#include "ItemFactory.h"
#include "CreateCharacter.h"
#include "DungeonMaster.h"
#include "PlayerBon.h"
#include "Map.h"
#include "Utils.h"

using namespace std;

Inventory::Inventory()
{
  auto setSlot = [&](const SlotId id, const string & name)
  {
    slots_[int(id)] = {id, name};
  };

  setSlot(SlotId::wielded,    "Wielding");
  setSlot(SlotId::wieldedAlt, "Prepared");
  setSlot(SlotId::thrown,     "Thrown");
  setSlot(SlotId::body,       "Body");
  setSlot(SlotId::head,       "Head");
  setSlot(SlotId::neck,       "Neck");
  setSlot(SlotId::ring1,      "Ring");
  setSlot(SlotId::ring2,      "Ring");
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
      lines.push_back(toStr(int(item->getId())));
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
    lines.push_back(toStr(int(item->getId())));
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
      item->onEquip();
    }
  }

  while (general_.size() != 0)
  {
    removeItemInBackpackWithIdx(0, true);
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

bool Inventory::hasItemInBackpack(const ItemId id) const
{
  auto it = std::find_if(begin(general_), end(general_), [id](Item * item)
  {
    return item->getId() == id;
  });

  return it != end(general_);
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

void Inventory::putInGeneral(Item * item)
{
  bool isStacked = false;

  //If item stacks, see if there is other items of same type
  if (item->getData().isStackable)
  {
    for (size_t i = 0; i < general_.size(); ++i)
    {
      Item* const other = general_[i];

      if (other->getId() == item->getId())
      {
        //Keeping picked up item and destroying the one in the inventory (then the
        //parameter pointer is still valid).
        item->nrItems_ += other->nrItems_;
        delete other;
        general_[i] = item;
        isStacked   = true;
        break;
      }
    }
  }

  if (!isStacked)
  {
    general_.push_back(item);
  }

  //Note: May destroy the item (e.g. a Medical Bag combining with another)
  item->onPickupToBackpack(*this);
}

void Inventory::dropAllNonIntrinsic(const Pos & pos)
{
  Item* item;

  //Drop from slots
  for (InvSlot& slot : slots_)
  {
    item = slot.item;
    if (item)
    {
      ItemDrop::dropItemOnMap(pos, *item);

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
      ItemDrop::dropItemOnMap(pos, *item);

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

void Inventory::decrItemInSlot(SlotId slotId)
{
  Item* item = getItemInSlot(slotId);
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
    slots_[int(slotId)].item = nullptr;
    delete item;
  }
}

void Inventory::removeItemInBackpackWithIdx(const size_t IDX, const bool DELETE_ITEM)
{
  if (general_.size() > IDX)
  {
    if (DELETE_ITEM)
    {
      delete general_[IDX];
    }
    general_.erase(begin(general_) + IDX);
  }
}

void Inventory::removeItemInBackpackWithPtr(Item * const item, const bool DELETE_ITEM)
{
  for (auto it = begin(general_); it < end(general_); ++it)
  {
    if (*it == item)
    {
      if (DELETE_ITEM)
      {
        delete *it;
      }
      general_.erase(it);
      return;
    }
  }

  TRACE << "Parameter item not in backpack" << endl;
  assert(false);
}

void Inventory::decrItemInGeneral(const size_t IDX)
{
  Item* item              = general_[IDX];
  bool  isStackable       = item->getData().isStackable;
  bool  shouldDeleteItem  = true;

  if (isStackable)
  {
    --item->nrItems_;
    shouldDeleteItem = item->nrItems_ <= 0;
  }

  if (shouldDeleteItem)
  {
    general_.erase(begin(general_) + IDX);
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

void Inventory::decrItem(Item * const item)
{
  for (InvSlot& slot : slots_)
  {
    if (slot.item == item)
    {
      decrItemInSlot(slot.id);
      return;
    }
  }

  for (size_t i = 0; i < general_.size(); ++i)
  {
    if (general_[i] == item)
    {
      decrItemInGeneral(i);
      return;
    }
  }
}

void Inventory::moveItemToSlot(InvSlot & slot, const size_t GEN_IDX)
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

void Inventory::equipGeneralItemAndEndTurn(const size_t GEN_IDX, const SlotId slotId)
{
  assert(slotId != SlotId::END);

  moveItemToSlot(slots_[int(slotId)], GEN_IDX);

  bool isFreeTurn = false;

  const bool IS_PLAYER  = this == &Map::player->getInv();

  if (IS_PLAYER)
  {
    Item* const   itemAfter = getItemInSlot(slotId);
    const string  name      = itemAfter->getName(ItemRefType::plural);
    string        msg       = "";

    switch (slotId)
    {
      case SlotId::wielded:
      {
        msg = "I am now wielding " + name + ".";
      } break;

      case SlotId::wieldedAlt:
      {
        msg = "I am now using " + name + " as a prepared weapon.";
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

      case SlotId::neck:
      {
        msg = "I am now wearing " + name + ".";
      } break;

      case SlotId::ring1:
      {
        msg = "I am now wearing " + name + ".";
      } break;

      case SlotId::ring2:
      {
        msg = "I am now wearing " + name + ".";
      } break;

      case SlotId::END: {}
        break;
    }

    Log::addMsg(msg, clrWhite, false, true);
  }

  GameTime::tick(isFreeTurn);
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

  GameTime::tick(IS_FREE_TURN);
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

bool Inventory::moveToGeneral(InvSlot & slot)
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

void Inventory::removeWithoutDestroying(const InvType invType, const size_t IDX)
{
  if (invType == InvType::slots)
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

Item* Inventory::getFirstItemInBackpackWithId(const ItemId id)
{
  auto it = std::find_if(begin(general_), end(general_), [id](Item * item)
  {
    return item->getId() == id;
  });

  if (it == end(general_))
  {
    return nullptr;
  }

  return *it;
}

int Inventory::getBackpackIdxWithItemId(const ItemId id) const
{
  for (size_t i = 0; i < general_.size(); ++i)
  {
    if (general_[i]->getId() == id)
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

void Inventory::putInIntrinsics(Item * item)
{
  assert(item->getData().type == ItemType::meleeWpnIntr ||
         item->getData().type == ItemType::rangedWpnIntr);

  intrinsics_.push_back(item);
}

Item* Inventory::getLastItemInGeneral()
{
  if (!general_.empty()) {return general_[general_.size() - 1];}
  return nullptr;
}

void Inventory::putInSlot(const SlotId id, Item * item)
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
  bool operator()(const Item * const item1, const Item * const item2)
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
