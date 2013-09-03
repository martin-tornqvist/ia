#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <string>
#include <iostream>

#include "ItemData.h"
#include "Item.h"

using namespace std;

class Engine;

enum SlotTypes_t {
  slot_wielded,
  slot_wieldedAlt,
  slot_missiles,
  slot_armorBody
//slot_cloak,
//slot_ringLeft,
//slot_ringRight,
//slot_amulet
};

struct InventorySlot {
  InventorySlot() :
    allowWieldedWeapon(false),
    allowMissile(false),
    allowArmor(false),
    allowCloak(false),
    allowAmulet(false),
    allowRing(false),
    item(NULL) {
  }
  bool allowWieldedWeapon;
  bool allowMissile;
  bool allowArmor;
  bool allowCloak;
  bool allowAmulet;
  bool allowRing;
  string interfaceName;
  SlotTypes_t id;
  Item* item;
};

class Engine;

class Inventory {
public:
  Inventory(bool humanoid);
  ~Inventory() {
    for(unsigned int i = 0; i < slots_.size(); i++)
      if(slots_.at(i).item != NULL)
        delete slots_.at(i).item;

    for(unsigned int i = 0; i < general_.size(); i++)
      delete general_.at(i);

    for(unsigned int i = 0; i < intrinsics_.size(); i++)
      delete intrinsics_.at(i);
  }

  void dropAllNonIntrinsic(
    const Pos pos, const bool ROLL_FOR_DESTRUCTION, Engine* const engine);

  bool hasItemInSlot(SlotTypes_t slotName) const;

  void putItemInSlot(
    SlotTypes_t slotName, Item* item, bool putInGeneral_ifOccupied = true,
    bool putInGeneral_ifSlotNotFound = true);

  void putItemInGeneral(Item* item);

  int getElementToStackItem(Item* item) const;

  void putItemInIntrinsics(Item* item);

  bool moveItemToGeneral(InventorySlot* inventorySlot);

  void moveItemFromGeneralToIntrinsics(
    const unsigned int GENERAL_INV_ELEMENT);

  void moveItemToSlot(
    InventorySlot* inventoryslot,
    const unsigned int GENERAL_INV_ELEMENT);

  void equipGeneralItemAndPossiblyEndTurn(
    const unsigned int GENERAL_INV_ELEMENT,
    const SlotTypes_t slotToEquip, Engine* const engine);

//  void equipGeneralItemToAltAndPossiblyEndTurn(const unsigned int GENERAL_INV_ELEMENT, Engine* const engine);

  void swapWieldedAndPrepared(const bool END_TURN, Engine* const engine);

  bool hasAmmoForFirearmInInventory();

  int getElementWithItemType(const ItemId_t itemId) const;

  Item* getItemInSlot(SlotTypes_t slotName) const;
  Item* getItemInElement(const int GLOBAL_ELEMENT_NR) const;

  void removeItemInElementWithoutDeletingInstance(const int GLOBAL_ELEMENT);

  void decreaseItemInSlot(SlotTypes_t slotName);

  void decreaseItemInGeneral(unsigned element);

  void decreaseItemTypeInGeneral(const ItemId_t itemId);

  void deleteItemInGeneralWithElement(const unsigned ELEMENT);
  void removetemInGeneralWithPointer(
    Item* const item, const bool DELETE_ITEM);

  int getIntrinsicsSize() const {
    return intrinsics_.size();
  }

  Item* getIntrinsicInElement(const int ELEMENT) const;

  Item* getLastItemInGeneral();

  bool hasItemInGeneral(const ItemId_t id) const;

  int getItemStackSizeInGeneral(const ItemId_t id) const;

  bool hasDynamiteInGeneral() const;

  void decreaseDynamiteInGeneral();

  //bool hasFirstAidInGeneral();
  //void decreaseFirstAidInGeneral();

  InventorySlot* getSlot(SlotTypes_t slotName);

  void sortGeneralInventory(Engine* const engine);

  vector<InventorySlot>* getSlots() {
    return &slots_;
  }
  vector<Item*>* getGeneral() {
    return &general_;
  }

  int getTotalItemWeight() const;

  void getAllItems(vector<Item*>& itemList) const;

  void addSaveLines(vector<string>& lines) const;
  void setParametersFromSaveLines(
    vector<string>& lines, Engine* const engine);

private:
  vector<InventorySlot> slots_;
  vector<Item*> general_;
  vector<Item*> intrinsics_;
};

#endif
