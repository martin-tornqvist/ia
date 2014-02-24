#include "Inventory.h"

#include <algorithm>

#include "Item.h"
#include "ItemWeapon.h"
#include "Engine.h"
#include "ItemDrop.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "GameTime.h"
#include "Renderer.h"
#include "ItemFactory.h"
#include "PlayerCreateCharacter.h"
#include "DungeonMaster.h"
#include "PlayerBonuses.h"

Inventory::Inventory(bool humanoid) {
  InventorySlot invSlot;

  if(humanoid) {
    invSlot.id = slot_wielded;
    invSlot.interfaceName       = "Wielding";
    invSlot.allowWieldedWeapon  = true;
    slots_.push_back(invSlot);
    invSlot.reset();

    invSlot.id = slot_wieldedAlt;
    invSlot.interfaceName       = "Prepared";
    invSlot.allowWieldedWeapon  = true;
    slots_.push_back(invSlot);
    invSlot.reset();

    invSlot.id = slot_missiles;
    invSlot.interfaceName       = "Missiles";
    invSlot.allowMissile        = true;
    slots_.push_back(invSlot);
    invSlot.reset();

    invSlot.id = slot_armorBody;
    invSlot.interfaceName       = "On body";
    invSlot.allowArmor          = true;
    slots_.push_back(invSlot);

    //    invSlot.id = slot_cloak;
    //    invSlot.interfaceName = "Cloak";
    //    invSlot.allowCloak = true;
    //    slots_.push_back(invSlot);
    //    invSlot.allowCloak = false;

    //    invSlot.id = slot_amulet;
    //    invSlot.interfaceName = "Amulet";
    //    invSlot.allowAmulet = true;
    //    slots_.push_back(invSlot);
    //    invSlot.allowAmulet = false;

    //    invSlot.id = slot_ringLeft;
    //    invSlot.interfaceName = "Left ring";
    //    invSlot.allowRing = true;
    //    slots_.push_back(invSlot);
    //    invSlot.id = slot_ringRight;
    //    invSlot.interfaceName = "Right ring";
    //    slots_.push_back(invSlot);
  }
}

Inventory::~Inventory() {
  for(InventorySlot & slot : slots_)  {if(slot.item) {delete slot.item;}}
  for(Item * item : general_)         {delete item;}
  for(Item * item : intrinsics_)      {delete item;}
}

void Inventory::addSaveLines(vector<string>& lines) const {
  for(unsigned int i = 0; i < slots_.size(); i++) {
    Item* const item = slots_.at(i).item;
    if(item == NULL) {
      lines.push_back("0");
    } else {
      lines.push_back(toString(item->getData().id));
      lines.push_back(toString(item->nrItems));
      item->addSaveLines_(lines);
    }
  }

  lines.push_back(toString(general_.size()));
  for(unsigned int i = 0; i < general_.size(); i++) {
    Item* const item = general_.at(i);
    lines.push_back(toString(item->getData().id));
    lines.push_back(toString(item->nrItems));
    item->addSaveLines_(lines);
  }
}

void Inventory::setParamsFromSaveLines(vector<string>& lines, Engine& engine) {
  for(InventorySlot & slot : slots_) {
    //Previous item is destroyed
    Item* item = slot.item;
    if(item != NULL) {
      delete item;
      slot.item = NULL;
    }

    const ItemId id = ItemId(toInt(lines.front()));
    lines.erase(lines.begin());
    if(id != item_empty) {
      item = engine.itemFactory->spawnItem(id);
      item->nrItems = toInt(lines.front());
      lines.erase(lines.begin());
      item->setParamsFromSaveLines_(lines);
      slot.item = item;
      //When loading the game, wear the item to apply properties from wearing
      item->onWear();
    }
  }

  while(general_.size() != 0) {
    deleteItemInGeneralWithElement(0);
  }

  const int NR_OF_GENERAL = toInt(lines.front());
  lines.erase(lines.begin());
  for(int i = 0; i < NR_OF_GENERAL; i++) {
    const ItemId id = ItemId(toInt(lines.front()));
    lines.erase(lines.begin());
    Item* item = engine.itemFactory->spawnItem(id);
    item->nrItems = toInt(lines.front());
    lines.erase(lines.begin());
    item->setParamsFromSaveLines_(lines);
    general_.push_back(item);
  }
}

bool Inventory::hasDynamiteInGeneral() const {
  return hasItemInGeneral(item_dynamite);
}

bool Inventory::hasItemInGeneral(const ItemId id) const {
  for(unsigned int i = 0; i < general_.size(); i++) {
    if(general_.at(i)->getData().id == id)
      return true;
  }

  return false;
}

int Inventory::getItemStackSizeInGeneral(const ItemId id) const {
  for(unsigned int i = 0; i < general_.size(); i++) {
    if(general_.at(i)->getData().id == id) {
      if(general_.at(i)->getData().isStackable == false) {
        return 1;
      } else {
        return general_.at(i)->nrItems;
      }
    }
  }

  return 0;
}

void Inventory::decrDynamiteInGeneral() {
  for(unsigned int i = 0; i < general_.size(); i++) {
    if(general_.at(i)->getData().id == item_dynamite) {
      decrItemInGeneral(i);
      break;
    }
  }
}

/*
 bool Inventory::hasFirstAidInGeneral()
 {
 for(unsigned int i = 0; i < general_.size(); i++) {
 if(general_.at(i)->getInstanceDefinition().id == item_firstAidKit)
 return true;
 }

 return false;
 }

 void Inventory::decreaseFirstAidInGeneral()
 {
 for(unsigned int i = 0; i < general_.size(); i++) {
 if(general_.at(i)->getInstanceDefinition().id == item_firstAidKit) {
 decrItemInGeneral(i);
 break;
 }
 }
 }
 */

void Inventory::putItemInGeneral(Item* item) {
  bool stackedItem = false;

  //If item stacks, see if there is other items of same type
  if(item->getData().isStackable == true) {

    const int stackIndex = getElementToStackItem(item);

    if(stackIndex != -1) {
      Item* compareItem = general_.at(stackIndex);

      //Keeping picked up item and destroying the one in the inventory,
      //to keep the parameter pointer valid.
      item->nrItems += compareItem->nrItems;
      delete compareItem;
      general_.at(stackIndex) = item;
      stackedItem = true;
    }
  }

  if(stackedItem == false) {
    general_.push_back(item);
  }
}

int Inventory::getElementToStackItem(Item* item) const {
  if(item->getData().isStackable == true) {
    for(unsigned int i = 0; i < general_.size(); i++) {
      Item* compare = general_.at(i);

      if(compare->getData().id == item->getData().id) {
        return i;
      }
    }
  }

  return -1;
}

void Inventory::dropAllNonIntrinsic(
  const Pos& pos, const bool ROLL_FOR_DESTRUCTION, Engine& engine) {

  Item* item;

  //Drop from slots
  for(unsigned int i = 0; i < slots_.size(); i++) {
    item = slots_.at(i).item;
    if(item != NULL) {
      if(ROLL_FOR_DESTRUCTION && Rnd::percentile() <
          CHANCE_TO_DESTROY_COMMON_ITEMS_ON_DROP) {
        delete slots_.at(i).item;
      } else {
        engine.itemDrop->dropItemOnMap(pos, *item);
      }

      slots_.at(i).item = NULL;
    }
  }

  //Drop from general
  unsigned int i = 0;
  while(i < general_.size()) {
    item = general_.at(i);
    if(item != NULL) {
      if(ROLL_FOR_DESTRUCTION && Rnd::percentile() <
          CHANCE_TO_DESTROY_COMMON_ITEMS_ON_DROP) {
        delete general_.at(i);
      } else {
        engine.itemDrop->dropItemOnMap(pos, *item);
      }

      general_.erase(general_.begin() + i);
    }
    i++;
  }
}

bool Inventory::hasAmmoForFirearmInInventory() {
  Weapon* weapon = dynamic_cast<Weapon*>(getItemInSlot(slot_wielded));

  //If weapon found
  if(weapon != NULL) {

    //If weapon has infinite ammo, return true but with a warning
    if(weapon->getData().rangedHasInfiniteAmmo) {
      trace << "[WARNING] Inventory::hasAmmoForFirearm...() ran on weapon with infinite ammo" << endl;
      return true;
    }

    //If weapon is a firearm
    if(weapon->getData().isRangedWeapon == true) {

      //Get weapon ammo type
      const ItemId ammoId = weapon->getData().rangedAmmoTypeUsed;

      //Look for that ammo type in inventory
      for(unsigned int i = 0; i < general_.size(); i++) {
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

  if(stack == true) {
    item->nrItems -= 1;

    if(item->nrItems > 0) {
      deleteItem = false;
    }
  }

  if(deleteItem == true) {
    getSlot(slotName)->item = NULL;
    delete item;
  }
}

void Inventory::deleteItemInGeneralWithElement(const unsigned ELEMENT) {
  if(general_.size() > ELEMENT) {
    delete general_.at(ELEMENT);
    general_.erase(general_.begin() + ELEMENT);
  }
}

void Inventory::removetemInGeneralWithPointer(
  Item* const item, const bool DELETE_ITEM) {

  for(unsigned int i = 0; i < general_.size(); i++) {
    if(general_.at(i) == item) {
      if(DELETE_ITEM) {
        delete item;
      }
      general_.erase(general_.begin() + i);
      return;
    }
  }
  trace << "[WARNING] Could not find parameter item in general inventory, in Inventory::deleteItemInGeneralWithPointer()" << endl;
}


void Inventory::decrItemInGeneral(unsigned element) {
  Item* item = general_.at(element);
  bool stack = item->getData().isStackable;
  bool deleteItem = true;

  if(stack == true) {
    item->nrItems -= 1;

    if(item->nrItems > 0) {
      deleteItem = false;
    }
  }

  if(deleteItem == true) {
    general_.erase(general_.begin() + element);

    delete item;
  }
}

void Inventory::decrItemTypeInGeneral(const ItemId id) {
  for(unsigned int i = 0; i < general_.size(); i++) {
    if(general_.at(i)->getData().id == id) {
      decrItemInGeneral(i);
      return;
    }
  }
}

void Inventory::moveItemToSlot(
  InventorySlot* inventorySlot,
  const unsigned int GENERAL_INV_ELEMENT) {

  bool generalSlotExists = GENERAL_INV_ELEMENT < general_.size();
  Item* item = NULL;
  Item* slotItem = inventorySlot->item;

  if(generalSlotExists) {
    item = general_.at(GENERAL_INV_ELEMENT);
  }

  if(generalSlotExists && item != NULL) {
    if(slotItem == NULL) {
      inventorySlot->item = item;
      general_.erase(general_.begin() + GENERAL_INV_ELEMENT);
    } else {
      general_.erase(general_.begin() + GENERAL_INV_ELEMENT);
      general_.push_back(slotItem);
      inventorySlot->item = item;
    }
  }
}

void Inventory::equipGeneralItemAndPossiblyEndTurn(
  const unsigned int GENERAL_INV_ELEMENT,
  const SlotId slotToEquip, Engine& engine) {

  const bool IS_PLAYER = this == &engine.player->getInv();

  bool isFreeTurn = false;

  Item* item = general_.at(GENERAL_INV_ELEMENT);
  const ItemData& d = item->getData();

  if(IS_PLAYER) {
    if(d.isArmor == false) {
      isFreeTurn = false;
    }
  }

  if(slotToEquip == slot_wielded) {
    Item* const itemBefore = getItemInSlot(slot_wielded);
    moveItemToSlot(getSlot(slot_wielded), GENERAL_INV_ELEMENT);
    Item* const itemAfter = getItemInSlot(slot_wielded);
    if(IS_PLAYER) {
      if(itemBefore != NULL) {
        const string nameBefore =
          engine.itemDataHandler->getItemRef(*itemBefore, itemRef_a);
        engine.log->addMsg(
          "I was wielding " + nameBefore + ".");
      }
      const string nameAfter =
        engine.itemDataHandler->getItemRef(*itemAfter, itemRef_a);
      engine.log->addMsg(
        "I am now wielding " + nameAfter + ".");
    }
  }

  if(slotToEquip == slot_wieldedAlt) {
    Item* const itemBefore = getItemInSlot(slot_wieldedAlt);
    moveItemToSlot(getSlot(slot_wieldedAlt), GENERAL_INV_ELEMENT);
    Item* const itemAfter = getItemInSlot(slot_wieldedAlt);
    if(IS_PLAYER) {
      if(itemBefore != NULL) {
        const string nameBefore =
          engine.itemDataHandler->getItemRef(*itemBefore, itemRef_a);
        engine.log->addMsg(
          "I was wielding " + nameBefore + " as a prepared weapon.");
      }
      const string nameAfter =
        engine.itemDataHandler->getItemRef(*itemAfter, itemRef_a);
      engine.log->addMsg(
        "I am now wielding " + nameAfter + " as a prepared weapon.");
    }
  }

  if(slotToEquip == slot_armorBody) {
    Item* const itemBefore = getItemInSlot(slot_armorBody);
    moveItemToSlot(getSlot(slot_armorBody), GENERAL_INV_ELEMENT);
    Item* const itemAfter = getItemInSlot(slot_armorBody);
    if(IS_PLAYER) {
      if(itemBefore != NULL) {
        const string nameBefore =
          engine.itemDataHandler->getItemRef(*itemBefore, itemRef_a);
        engine.log->addMsg("I wore " + nameBefore + ".");
      }
      const string nameAfter =
        engine.itemDataHandler->getItemRef(*itemAfter, itemRef_plural);
      engine.log->addMsg("I am now wearing " + nameAfter + ".");
    }
    isFreeTurn = false;
  }

  if(slotToEquip == slot_missiles) {
    Item* const itemBefore = getItemInSlot(slot_missiles);
    moveItemToSlot(getSlot(slot_missiles), GENERAL_INV_ELEMENT);
    Item* const itemAfter = getItemInSlot(slot_missiles);
    if(IS_PLAYER) {
      if(itemBefore != NULL) {
        const string nameBefore =
          engine.itemDataHandler->getItemRef(*itemBefore, itemRef_plural);
        engine.log->addMsg(
          "I was using " + nameBefore + " as missile weapon.");
      }
      const string nameAfter =
        engine.itemDataHandler->getItemRef(*itemAfter, itemRef_plural);
      engine.log->addMsg(
        "I am now using " + nameAfter + " as missile weapon.");
    }
  }
  engine.gameTime->actorDidAct(isFreeTurn);
}

void Inventory::swapWieldedAndPrepared(
  const bool IS_FREE_TURN, Engine& engine) {
  InventorySlot* slot1 = getSlot(slot_wielded);
  InventorySlot* slot2 = getSlot(slot_wieldedAlt);
  Item* item1 = slot1->item;
  Item* item2 = slot2->item;
  slot1->item = item2;
  slot2->item = item1;

  Renderer::drawMapAndInterface();

  engine.gameTime->actorDidAct(IS_FREE_TURN);
}

void Inventory::moveItemFromGeneralToIntrinsics(
  const unsigned int GENERAL_INV_ELEMENT) {
  bool generalSlotExists = GENERAL_INV_ELEMENT < general_.size();

  if(generalSlotExists) {
    Item* item = general_.at(GENERAL_INV_ELEMENT);
    bool itemExistsInGeneralSlot = item != NULL;

    if(itemExistsInGeneralSlot) {
      intrinsics_.push_back(item);
      general_.erase(general_.begin() + GENERAL_INV_ELEMENT);
    }
  }
}

bool Inventory::moveItemToGeneral(InventorySlot* inventorySlot) {
  Item* const item = inventorySlot->item;
  if(item == NULL) {
    return false;
  } else {
    inventorySlot->item = NULL;
    putItemInGeneral(item);
    return true;
  }
}

bool Inventory::hasItemInSlot(SlotId slotName) const {
  for(unsigned int i = 0; i < slots_.size(); i++) {
    if(slots_[i].id == slotName) {
      if(slots_[i].item != NULL) {
        return true;
      }
    }
  }

  return false;
}

void Inventory::removeItemInElementWithoutDeletingInstance(const int GLOBAL_ELEMENT) {
  //If parameter element corresponds to equipped slots, remove item in that slot
  if(GLOBAL_ELEMENT >= 0 && GLOBAL_ELEMENT < signed(slots_.size())) {
    slots_.at(GLOBAL_ELEMENT).item = NULL;
  } else {
    //If parameter element corresponds to general slot, remove that slot
    const int GENERAL_ELEMENT = GLOBAL_ELEMENT - slots_.size();
    if(GENERAL_ELEMENT >= 0 && GENERAL_ELEMENT < signed(general_.size())) {
      general_.erase(general_.begin() + GENERAL_ELEMENT);
    }
  }
}

int Inventory::getElementWithItemType(const ItemId id) const {
  for(unsigned int i = 0; i < general_.size(); i++) {
    if(general_.at(i)->getData().id == id) {
      return i;
    }
  }
  return -1;
}

Item* Inventory::getItemInElement(const int GLOBAL_ELEMENT_NR) const {
  if(GLOBAL_ELEMENT_NR >= 0 && GLOBAL_ELEMENT_NR < signed(slots_.size())) {
    return slots_.at(GLOBAL_ELEMENT_NR).item;
  }

  const int GENERAL_ELEMENT_NR = GLOBAL_ELEMENT_NR - slots_.size();
  if(GENERAL_ELEMENT_NR >= 0 && GENERAL_ELEMENT_NR < signed(general_.size())) {
    return general_.at(GENERAL_ELEMENT_NR);
  }

  return NULL;
}

Item* Inventory::getItemInSlot(SlotId slotName) const {
  if(hasItemInSlot(slotName)) {
    for(unsigned int i = 0; i < slots_.size(); i++) {
      if(slots_[i].id == slotName) {
        return slots_[i].item;
      }
    }
  }

  return NULL;
}

Item* Inventory::getIntrinsicInElement(int element) const {
  if(getIntrinsicsSize() > element)
    return intrinsics_[element];

  return NULL;
}

void Inventory::putItemInIntrinsics(Item* item) {
  if(item->getData().isIntrinsic) {
    intrinsics_.push_back(item);
  } else {
    trace << "[WARNING] Tried to put non-intrinsic weapon in intrinsics, ";
    trace << "in putItemInIntrinsics()" << endl;
  }
}

Item* Inventory::getLastItemInGeneral() {
  int s = general_.size();

  if(s != 0)
    return general_.at(general_.size() - 1);

  return NULL;
}

InventorySlot* Inventory::getSlot(SlotId slotName) {
  InventorySlot* slot = NULL;

  for(unsigned int i = 0; i < slots_.size(); i++) {
    if(slots_[i].id == slotName) {
      slot = &slots_[i];
    }
  }
  return slot;
}

void Inventory::putItemInSlot(SlotId slotName, Item* item,
                              bool putInGeneral_ifOccupied,
                              bool putInGeneral_ifSlotNotFound) {
  bool hasSlot = false;

  for(unsigned int i = 0; i < slots_.size(); i++) {
    if(slots_[i].id == slotName) {
      hasSlot = true;
      if(slots_[i].item == NULL)
        slots_[i].item = item;
      else if(putInGeneral_ifOccupied)
        general_.push_back(item);
    }
  }

  if(putInGeneral_ifSlotNotFound && hasSlot == false) {
    general_.push_back(item);
  }
}

int Inventory::getTotalItemWeight() const {
  int weight = 0;
  for(unsigned int i = 0; i < slots_.size(); i++) {
    if(slots_.at(i).item != NULL) {
      weight += slots_.at(i).item->getWeight();
    }
  }
  for(unsigned int i = 0; i < general_.size(); i++) {
    weight += general_.at(i)->getWeight();
  }
  return weight;
}

// Function for lexicographically comparing two items
struct LexicograhicalCompareItems {
public:
  LexicograhicalCompareItems(Engine& engine) : eng(engine) {
  }
  bool operator()(Item* const item1, Item* const item2) {
    const string& itemName1 =
      eng.itemDataHandler->getItemRef(*item1, itemRef_plain, true);
    const string& itemName2 =
      eng.itemDataHandler->getItemRef(*item2, itemRef_plain, true);
    return std::lexicographical_compare(itemName1.begin(), itemName1.end(),
                                        itemName2.begin(), itemName2.end());
  }
  Engine& eng;
};

void Inventory::sortGeneralInventory(Engine& engine) {
  vector< vector<Item*> > sortBuffer;

  // Sort according to item interface color first
  for(unsigned int iGeneral = 0; iGeneral < general_.size(); iGeneral++) {
    bool isAddedToBuffer = false;
    for(unsigned int iBuffer = 0;  iBuffer < sortBuffer.size(); iBuffer++) {
      const SDL_Color clrCurrentGroup = sortBuffer.at(iBuffer).at(0)->getInterfaceClr();
      if(Utils::isClrEq(general_.at(iGeneral)->getInterfaceClr(), clrCurrentGroup)) {
        sortBuffer.at(iBuffer).push_back(general_.at(iGeneral));
        isAddedToBuffer = true;
        break;
      }
    }
    if(isAddedToBuffer) {
      continue;
    } else {
      vector<Item*> newItemCategory;
      newItemCategory.push_back(general_.at(iGeneral));
      sortBuffer.push_back(newItemCategory);
    }
  }

  // Sort lexicographically sedond
  LexicograhicalCompareItems cmp(engine);
  for(unsigned int i = 0; i < sortBuffer.size(); i++) {
    std::sort(sortBuffer.at(i).begin(), sortBuffer.at(i).end(), cmp);
  }

  // Set the inventory from the sorting buffer
  general_.resize(0);
  for(unsigned int i = 0; i < sortBuffer.size(); i++) {
    for(unsigned int ii = 0; ii < sortBuffer.at(i).size(); ii++) {
      general_.push_back(sortBuffer.at(i).at(ii));
    }
  }
}

void Inventory::getAllItems(vector<Item*>& itemList) const {
  itemList.resize(0);
  itemList.reserve(slots_.size() + general_.size());

  for(const InventorySlot & slot : slots_) {
    Item* const item = slot.item;
    if(item != NULL) {
      itemList.push_back(item);
    }
  }

  for(Item * item : general_) {itemList.push_back(item);}
}

