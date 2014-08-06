#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <string>
#include <iostream>

#include "ItemData.h"

enum class SlotId {wielded, wieldedAlt, thrown, body, head};

struct InvSlot {
  InvSlot(SlotId id_, std::string name_) : id(id_), name(name_), item(nullptr) {}

  InvSlot() : id(SlotId::wielded), name(""), item(nullptr) {}

  SlotId id;
  std::string name;
  Item* item;
};

class Inventory {
public:
  Inventory(bool IS_HUMANOID);

  ~Inventory();

  void dropAllNonIntrinsic(const Pos& pos, const bool ROLL_FOR_DESTRUCTION);

  bool hasItemInSlot(SlotId id) const;

  //Note: Will put item in general if slot occupied
  void putInSlot(const SlotId id, Item* item);

  void putInGeneral(Item* item);

  int getElementToStackItem(Item* item) const;

  void putInIntrinsics(Item* item);

  bool moveToGeneral(InvSlot* inventorySlot);

  void moveFromGeneralToIntrinsics(const size_t GENERAL_INV_ELEMENT);

  void moveItemToSlot(InvSlot* inventoryslot, const size_t GENERAL_INV_ELEMENT);

  void equipGeneralItemAndPossiblyEndTurn(
    const size_t GENERAL_INV_ELEMENT, const SlotId slot);

  void swapWieldedAndPrepared(const bool IS_FREE_TURN);

  bool hasAmmoForFirearmInInventory();

  int getElementWithItemType(const ItemId itemId) const;

  Item* getItemInSlot(SlotId slotName) const;
  Item* getItemInElement(const int GLOBAL_ELEMENT_NR) const;

  void removeInElementWithoutDeletingInstance(const int GLOBAL_ELEMENT);

  void decrItemInSlot(SlotId slotName);

  void decrItemInGeneral(unsigned element);

  void decrItemTypeInGeneral(const ItemId itemId);

  void deleteItemInGeneralWithElement(const unsigned ELEMENT);
  void removeItemInGeneralWithPointer(Item* const item, const bool DELETE_ITEM);

  int getIntrinsicsSize() const {return intrinsics_.size();}

  Item* getIntrinsicInElement(const int ELEMENT) const;

  Item* getLastItemInGeneral();

  bool hasItemInGeneral(const ItemId id) const;

  int getItemStackSizeInGeneral(const ItemId id) const;

  bool hasDynamiteInGeneral() const;

  void decrDynamiteInGeneral();

  InvSlot* getSlot(SlotId slotName);

  void sortGeneralInventory();

  std::vector<InvSlot>&       getSlots()          {return slots_;}
  std::vector<Item*>&         getGeneral()        {return general_;}
  const std::vector<InvSlot>& getSlots()    const {return slots_;}
  const std::vector<Item*>&   getGeneral()  const {return general_;}

  int getTotalItemWeight() const;

  void storeToSaveLines(std::vector<std::string>& lines) const;
  void setupFromSaveLines(std::vector<std::string>& lines);

private:
  std::vector<InvSlot> slots_;
  std::vector<Item*> general_;
  std::vector<Item*> intrinsics_;
};

#endif
