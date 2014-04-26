#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <string>
#include <iostream>

#include "ItemData.h"

using namespace std;

enum class SlotId {
  wielded,
  wieldedAlt,
  missiles,
  armorBody
};

struct InvSlot {
  InvSlot() {reset();}

  void reset() {
    allowWieldedWeapon  = false;
    allowMissile        = false;
    allowArmor          = false;
    allowCloak          = false;
    allowAmulet         = false;
    allowRing           = false;
    interfaceName       = "";
    id                  = SlotId::wielded;
    item                = NULL;
  }
  bool allowWieldedWeapon;
  bool allowMissile;
  bool allowArmor;
  bool allowCloak;
  bool allowAmulet;
  bool allowRing;
  string interfaceName;
  SlotId id;
  Item* item;
};

class Inventory {
public:
  Inventory(bool humanoid);

  ~Inventory();

  void dropAllNonIntrinsic(
    const Pos& pos, const bool ROLL_FOR_DESTRUCTION);

  bool hasItemInSlot(SlotId slotName) const;

  void putItemInSlot(
    SlotId slotName, Item* item, bool putInGeneral_ifOccupied = true,
    bool putInGeneral_ifSlotNotFound = true);

  void putItemInGeneral(Item* item);

  int getElementToStackItem(Item* item) const;

  void putItemInIntrinsics(Item* item);

  bool moveItemToGeneral(InvSlot* inventorySlot);

  void moveItemFromGeneralToIntrinsics(
    const unsigned int GENERAL_INV_ELEMENT);

  void moveItemToSlot(
    InvSlot* inventoryslot,
    const unsigned int GENERAL_INV_ELEMENT);

  void equipGeneralItemAndPossiblyEndTurn(
    const unsigned int GENERAL_INV_ELEMENT,
    const SlotId slotToEquip);

  void swapWieldedAndPrepared(const bool IS_FREE_TURN);

  bool hasAmmoForFirearmInInventory();

  int getElementWithItemType(const ItemId itemId) const;

  Item* getItemInSlot(SlotId slotName) const;
  Item* getItemInElement(const int GLOBAL_ELEMENT_NR) const;

  void removeItemInElementWithoutDeletingInstance(const int GLOBAL_ELEMENT);

  void decrItemInSlot(SlotId slotName);

  void decrItemInGeneral(unsigned element);

  void decrItemTypeInGeneral(const ItemId itemId);

  void deleteItemInGeneralWithElement(const unsigned ELEMENT);
  void removetemInGeneralWithPointer(
    Item* const item, const bool DELETE_ITEM);

  int getIntrinsicsSize() const {return intrinsics_.size();}

  Item* getIntrinsicInElement(const int ELEMENT) const;

  Item* getLastItemInGeneral();

  bool hasItemInGeneral(const ItemId id) const;

  int getItemStackSizeInGeneral(const ItemId id) const;

  bool hasDynamiteInGeneral() const;

  void decrDynamiteInGeneral();

  InvSlot* getSlot(SlotId slotName);

  void sortGeneralInventory();

  vector<InvSlot>& getSlots() {return slots_;}

  vector<Item*>& getGeneral() {return general_;}

  int getTotalItemWeight() const;

  void getAllItems(vector<Item*>& itemList) const;

  void storeToSaveLines(vector<string>& lines) const;
  void setupFromSaveLines(
    vector<string>& lines);

private:
  vector<InvSlot> slots_;
  vector<Item*> general_;
  vector<Item*> intrinsics_;
};

#endif
