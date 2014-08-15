#include "Inventory.h"

#include <algorithm>
#include <vector>

#include "Init.h"
#include "Item.h"
#include "ItemDrop.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "GameTime.h"
#include "Renderer.h"
#include "ItemFactory.h"
#include "CreateCharacter.h"
#include "DungeonMaster.h"
#include "PlayerBon.h"
#include "Map.h"

using namespace std;

Inventory::Inventory(const bool IS_HUMANOID) {
  slots_.resize(0);

  if(IS_HUMANOID) {
    slots_ = vector<InvSlot> {
      {SlotId::wielded,     "Wielding"},
      {SlotId::wieldedAlt,  "Prepared"},
      {SlotId::thrown,      "Thrown"},
      {SlotId::body,        "On body"},
      {SlotId::head,        "On head"}
    };
  }
}

Inventory::~Inventory() {
  for(InvSlot& slot : slots_)  {if(slot.item) {delete slot.item;}}
  for(Item* item : general_)         {delete item;}
  for(Item* item : intrinsics_)      {delete item;}
}

void Inventory::storeToSaveLines(vector<string>& lines) const {
  for(const InvSlot& slot : slots_) {
    Item* const item = slot.item;
    if(item) {
      lines.push_back(toStr(int(item->getData().id)));
      lines.push_back(toStr(item->nrItems_));
      item->storeToSaveLines(lines);
    } else {
      lines.push_back("0");
    }
  }

  lines.push_back(toStr(general_.size()));
  for(Item* item : general_) {
    lines.push_back(toStr(int(item->getData().id)));
    lines.push_back(toStr(item->nrItems_));
    item->storeToSaveLines(lines);
  }
}

void Inventory::setupFromSaveLines(vector<string>& lines) {
  for(InvSlot& slot : slots_) {
    //Previous item is destroyed
    Item* item = slot.item;
    if(item) {
      delete item;
      slot.item = nullptr;
    }

    const ItemId id = ItemId(toInt(lines.front()));
    lines.erase(begin(lines));
    if(id != ItemId::empty) {
      item = ItemFactory::mk(id);
      item->nrItems_ = toInt(lines.front());
      lines.erase(begin(lines));
      item->setupFromSaveLines(lines);
      slot.item = item;
      //When loading the game, wear the item to apply properties from wearing
      item->onWear();
    }
  }

  while(general_.size() != 0) {
    deleteItemInGeneralWithElement(0);
  }

  const int NR_OF_GENERAL = toInt(lines.front());
  lines.erase(begin(lines));
  for(int i = 0; i < NR_OF_GENERAL; ++i) {
    const ItemId id = ItemId(toInt(lines.front()));
    lines.erase(begin(lines));
    Item* item = ItemFactory::mk(id);
    item->nrItems_ = toInt(lines.front());
    lines.erase(begin(lines));
    item->setupFromSaveLines(lines);
    general_.push_back(item);
  }
}

bool Inventory::hasDynamiteInGeneral() const {
  return hasItemInGeneral(ItemId::dynamite);
}

bool Inventory::hasItemInGeneral(const ItemId id) const {
  for(size_t i = 0; i < general_.size(); ++i) {
    if(general_.at(i)->getData().id == id)
      return true;
  }

  return false;
}

int Inventory::getItemStackSizeInGeneral(const ItemId id) const {
  for(size_t i = 0; i < general_.size(); ++i) {
    if(general_.at(i)->getData().id == id) {
      if(general_.at(i)->getData().isStackable) {
        return general_.at(i)->nrItems_;
      } else {
        return 1;
      }
    }
  }

  return 0;
}

void Inventory::decrDynamiteInGeneral() {
  for(size_t i = 0; i < general_.size(); ++i) {
    if(general_.at(i)->getData().id == ItemId::dynamite) {
      decrItemInGeneral(i);
      break;
    }
  }
}

/*
 bool Inventory::hasFirstAidInGeneral()
 {
 for(size_t i = 0; i < general_.size(); ++i) {
 if(general_.at(i)->getInstanceDefinition().id == ItemId::firstAidKit)
 return true;
 }

 return false;
 }

 void Inventory::decreaseFirstAidInGeneral()
 {
 for(size_t i = 0; i < general_.size(); ++i) {
 if(general_.at(i)->getInstanceDefinition().id == ItemId::firstAidKit) {
 decrItemInGeneral(i);
 break;
 }
 }
 }
 */

void Inventory::putInGeneral(Item* item) {
  bool isStacked = false;

  //If item stacks, see if there is other items of same type
  if(item->getData().isStackable) {

    const int stackIndex = getElementToStackItem(item);

    if(stackIndex != -1) {
      Item* compareItem = general_.at(stackIndex);

      //Keeping picked up item and destroying the one in the inventory,
      //to keep the parameter pointer valid.
      item->nrItems_ += compareItem->nrItems_;
      delete compareItem;
      general_.at(stackIndex) = item;
      isStacked = true;
    }
  }

  if(!isStacked) {general_.push_back(item);}
}

int Inventory::getElementToStackItem(Item* item) const {
  if(item->getData().isStackable) {
    for(size_t i = 0; i < general_.size(); ++i) {
      Item* compare = general_.at(i);

      if(compare->getData().id == item->getData().id) {
        return i;
      }
    }
  }

  return -1;
}

void Inventory::dropAllNonIntrinsic(
  const Pos& pos, const bool ROLL_FOR_DESTRUCTION) {

  Item* item;

  //Drop from slots
  for(InvSlot& slot : slots_) {
    item = slot.item;
    if(item) {
      if(ROLL_FOR_DESTRUCTION && Rnd::percentile() <
          CHANCE_TO_DESTROY_COMMON_ITEMS_ON_DROP) {
        delete slot.item;
      } else {
        ItemDrop::dropItemOnMap(pos, *item);
      }

      slot.item = nullptr;
    }
  }

  //Drop from general
  size_t i = 0;
  while(i < general_.size()) {
    item = general_.at(i);
    if(item) {
      if(ROLL_FOR_DESTRUCTION && Rnd::percentile() <
          CHANCE_TO_DESTROY_COMMON_ITEMS_ON_DROP) {
        delete general_.at(i);
      } else {
        ItemDrop::dropItemOnMap(pos, *item);
      }

      general_.erase(begin(general_) + i);
    }
    i++;
  }
}

bool Inventory::hasAmmoForFirearmInInventory() {
  Wpn* weapon = static_cast<Wpn*>(getItemInSlot(SlotId::wielded));

  //If weapon found
  if(weapon) {

    assert(!weapon->getData().ranged.hasInfiniteAmmo); //Should not happen

    //If weapon is a firearm
    if(weapon->getData().ranged.isRangedWpn) {

      //Get weapon ammo type
      const ItemId ammoId = weapon->getData().ranged.ammoItemId;

      //Look for that ammo type in inventory
      for(size_t i = 0; i < general_.size(); ++i) {
        if(general_.at(i)->getData().id == ammoId) {
          return true;
        }
      }
    }
  }
  return false;
}

void Inventory::decrItemInSlot(SlotId slotName) {
  Item* item = getItemInSlot(slotName);
  bool stack = item->getData().isStackable;
  bool deleteItem = true;

  if(stack) {
    item->nrItems_ -= 1;

    if(item->nrItems_ > 0) {
      deleteItem = false;
    }
  }

  if(deleteItem) {
    getSlot(slotName)->item = nullptr;
    delete item;
  }
}

void Inventory::deleteItemInGeneralWithElement(const unsigned ELEMENT) {
  if(general_.size() > ELEMENT) {
    delete general_.at(ELEMENT);
    general_.erase(begin(general_) + ELEMENT);
  }
}

void Inventory::removeItemInGeneralWithPointer(
  Item* const item, const bool DELETE_ITEM) {

  for(size_t i = 0; i < general_.size(); ++i) {
    if(general_.at(i) == item) {
      if(DELETE_ITEM) {delete item;}
      general_.erase(begin(general_) + i);
      return;
    }
  }
  assert(false && "Parameter item not in general inventory");
}

void Inventory::decrItemInGeneral(unsigned element) {
  Item* item = general_.at(element);
  bool stack = item->getData().isStackable;
  bool deleteItem = true;

  if(stack) {
    item->nrItems_ -= 1;

    if(item->nrItems_ > 0) {
      deleteItem = false;
    }
  }

  if(deleteItem) {
    general_.erase(begin(general_) + element);

    delete item;
  }
}

void Inventory::decrItemTypeInGeneral(const ItemId id) {
  for(size_t i = 0; i < general_.size(); ++i) {
    if(general_.at(i)->getData().id == id) {
      decrItemInGeneral(i);
      return;
    }
  }
}

void Inventory::moveItemToSlot(
  InvSlot* inventorySlot,
  const size_t GENERAL_INV_ELEMENT) {

  bool generalSlotExists = GENERAL_INV_ELEMENT < general_.size();
  Item* item = nullptr;
  Item* slotItem = inventorySlot->item;

  if(generalSlotExists) {
    item = general_.at(GENERAL_INV_ELEMENT);
  }

  if(generalSlotExists && item) {
    if(slotItem) {
      general_.erase(begin(general_) + GENERAL_INV_ELEMENT);
      general_.push_back(slotItem);
      inventorySlot->item = item;
    } else {
      inventorySlot->item = item;
      general_.erase(begin(general_) + GENERAL_INV_ELEMENT);
    }
  }
}

void Inventory::equipGeneralItemAndPossiblyEndTurn(
  const size_t GENERAL_INV_ELEMENT, const SlotId slot) {

  const bool IS_PLAYER  = this == &Map::player->getInv();
  bool isFreeTurn       = false;
  Item* item            = general_.at(GENERAL_INV_ELEMENT);
  const ItemDataT& d    = item->getData();

  if(IS_PLAYER && !d.isArmor) {isFreeTurn = false;}

  moveItemToSlot(getSlot(slot), GENERAL_INV_ELEMENT);
  Item* const itemAfter = getItemInSlot(slot);

  switch(slot) {
    case SlotId::wielded: {
      if(IS_PLAYER) {
        const string name = itemAfter->getName(ItemRefType::a);
        Log::addMsg("I am now wielding " + name + ".");
      }
    } break;

    case SlotId::wieldedAlt: {
      if(IS_PLAYER) {
        const string name = itemAfter->getName(ItemRefType::a);
        Log::addMsg("I am now wielding " + name + " as a prepared weapon.");
      }
    } break;

    case SlotId::thrown: {
      if(IS_PLAYER) {
        const string name = itemAfter->getName(ItemRefType::plural);
        Log::addMsg("I am now using " + name + " as missile weapon.");
      }
    } break;

    case SlotId::body: {
      if(IS_PLAYER) {
        const string name = itemAfter->getName(ItemRefType::a);
        Log::addMsg("I am now wearing " + name + ".");
      }
      isFreeTurn = false;
    } break;

    case SlotId::head: {
      if(IS_PLAYER) {
        const string name = itemAfter->getName(ItemRefType::a);
        Log::addMsg("I am now wearing " + name + ".");
      }
    } break;
  }

  GameTime::actorDidAct(isFreeTurn);
}

void Inventory::swapWieldedAndPrepared(
  const bool IS_FREE_TURN) {

  InvSlot* slot1 = getSlot(SlotId::wielded);
  InvSlot* slot2 = getSlot(SlotId::wieldedAlt);
  Item* item1 = slot1->item;
  Item* item2 = slot2->item;
  slot1->item = item2;
  slot2->item = item1;

  Renderer::drawMapAndInterface();

  GameTime::actorDidAct(IS_FREE_TURN);
}

void Inventory::moveFromGeneralToIntrinsics(
  const size_t GENERAL_INV_ELEMENT) {
  bool generalSlotExists = GENERAL_INV_ELEMENT < general_.size();

  if(generalSlotExists) {
    Item* item = general_.at(GENERAL_INV_ELEMENT);
    bool itemExistsInGeneralSlot = item;

    if(itemExistsInGeneralSlot) {
      intrinsics_.push_back(item);
      general_.erase(begin(general_) + GENERAL_INV_ELEMENT);
    }
  }
}

bool Inventory::moveToGeneral(InvSlot* inventorySlot) {
  Item* const item = inventorySlot->item;
  if(item) {
    inventorySlot->item = nullptr;
    putInGeneral(item);
    return true;
  } else {
    return false;
  }
}

bool Inventory::hasItemInSlot(SlotId id) const {
  for(size_t i = 0; i < slots_.size(); ++i) {
    if(slots_[i].id == id) {
      if(slots_[i].item) {
        return true;
      }
    }
  }

  return false;
}

void Inventory::removeWithoutDestroying(const InvList invList, const size_t ELEMENT) {
  if(invList == InvList::slots) {
    assert(ELEMENT < slots_.size());
    slots_.at(ELEMENT).item = nullptr;
  } else {
    assert(ELEMENT < general_.size());
    general_.erase(begin(general_) + ELEMENT);
  }
}

int Inventory::getElementWithItemType(const ItemId id) const {
  for(size_t i = 0; i < general_.size(); ++i) {
    if(general_.at(i)->getData().id == id) {
      return i;
    }
  }
  return -1;
}

Item* Inventory::getItemInSlot(SlotId slotName) const {
  if(hasItemInSlot(slotName)) {
    for(size_t i = 0; i < slots_.size(); ++i) {
      if(slots_[i].id == slotName) {
        return slots_[i].item;
      }
    }
  }

  return nullptr;
}

Item* Inventory::getIntrinsicInElement(int element) const {
  if(getIntrinsicsSize() > element)
    return intrinsics_[element];

  return nullptr;
}

void Inventory::putInIntrinsics(Item* item) {
  if(item->getData().isIntrinsic) {
    intrinsics_.push_back(item);
  } else {
    TRACE << "[WARNING] Tried to put non-intrinsic weapon in intrinsics, ";
    TRACE << "in putInIntrinsics()" << endl;
  }
}

Item* Inventory::getLastItemInGeneral() {
  if(!general_.empty()) {return general_.at(general_.size() - 1);}
  return nullptr;
}

InvSlot* Inventory::getSlot(SlotId slotName) {
  InvSlot* slot = nullptr;

  for(size_t i = 0; i < slots_.size(); ++i) {
    if(slots_[i].id == slotName) {
      slot = &slots_[i];
    }
  }
  return slot;
}

void Inventory::putInSlot(const SlotId id, Item* item) {
  for(InvSlot& slot : slots_) {
    if(slot.id == id) {
      if(slot.item) {
        general_.push_back(item);
      } else {
        slot.item = item;
      }
      return;
    }
  }

  general_.push_back(item);
  assert(false && "Bad slot id");
}

int Inventory::getTotalItemWeight() const {
  int weight = 0;
  for(size_t i = 0; i < slots_.size(); ++i) {
    if(slots_.at(i).item) {
      weight += slots_.at(i).item->getWeight();
    }
  }
  for(size_t i = 0; i < general_.size(); ++i) {
    weight += general_.at(i)->getWeight();
  }
  return weight;
}

// Function for lexicographically comparing two items
struct LexicograhicalCompareItems {
public:
  bool operator()(const Item* const item1, const Item* const item2) {
    const string& itemName1 = item1->getName(ItemRefType::plain);
    const string& itemName2 = item2->getName(ItemRefType::plain);
    return lexicographical_compare(itemName1.begin(), itemName1.end(),
                                   itemName2.begin(), itemName2.end());
  }
};

void Inventory::sortGeneralInventory() {
  vector< vector<Item*> > sortBuffer;

  //Sort according to item interface color first
  for(Item* item : general_) {

    bool isAddedToBuffer = false;

    //Check if item should be added to any existing color group
    for(vector<Item*>& group : sortBuffer) {
      const Clr clrCurGroup = group.at(0)->getInterfaceClr();
      if(Utils::isClrEq(item->getInterfaceClr(), clrCurGroup)) {
        group.push_back(item);
        isAddedToBuffer = true;
        break;
      }
    }

    if(isAddedToBuffer) {
      continue;
    } else {
      //Item is a new color, create a new color group
      vector<Item*> newGroup;
      newGroup.push_back(item);
      sortBuffer.push_back(newGroup);
    }
  }

  //Sort lexicographically secondarily
  LexicograhicalCompareItems cmp;
  for(vector<Item*>& group : sortBuffer) {
    std::sort(group.begin(), group.end(), cmp);
  }

  //Set the inventory from the sorting buffer
  general_.resize(0);
  for(size_t i = 0; i < sortBuffer.size(); ++i) {
    for(size_t ii = 0; ii < sortBuffer.at(i).size(); ii++) {
      general_.push_back(sortBuffer.at(i).at(ii));
    }
  }
}
