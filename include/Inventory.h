#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <string>
#include <iostream>

#include "CmnTypes.h"

class Item;

enum class ItemId;

enum class SlotId {wielded, wieldedAlt, thrown, body, head, END};

struct InvSlot
{
  InvSlot(SlotId id_, std::string name_) : id(id_), name(name_), item(nullptr) {}

  InvSlot() : id(SlotId::wielded), name(""), item(nullptr) {}

  SlotId      id;
  std::string name;
  Item*       item;
};

class Inventory
{
public:
  Inventory();

  ~Inventory();

  void dropAllNonIntrinsic(const Pos& pos, const bool ROLL_FOR_DESTRUCTION);

  bool hasItemInSlot(SlotId id) const;

  //Note: Will put item in general if slot occupied
  void putInSlot(const SlotId id, Item* item);

  void putInGeneral(Item* item);

  int getElementToStackItem(Item* item) const;

  void putInIntrinsics(Item* item);

  bool moveToGeneral(InvSlot& slot);

  void moveFromGeneralToIntrinsics(const size_t GEN_IDX);

  void moveItemToSlot(InvSlot& slot, const size_t GEN_IDX);

  void equipGeneralItemAndEndTurn(const size_t GEN_IDX, const SlotId slot);

  void swapWieldedAndPrepared(const bool IS_FREE_TURN);

  bool hasAmmoForFirearmInInventory();

  int getElementWithItemType(const ItemId itemId) const;

  Item* getItemInSlot(SlotId id) const;

  void removeWithoutDestroying(const InvList invList, const size_t IDX);

  void decrItemInSlot(SlotId slotName);

  void decrItemInGeneral(size_t idx);

  void decrItemTypeInGeneral(const ItemId itemId);

  void deleteItemInGeneralWithElement(const size_t IDX);
  void removeItemInGeneralWithPtr(Item* const item, const bool DELETE_ITEM);

  int getIntrinsicsSize() const {return intrinsics_.size();}

  Item* getIntrinsicInElement(const int IDX) const;

  Item* getLastItemInGeneral();

  bool hasItemInGeneral(const ItemId id) const;

  int getItemStackSizeInGeneral(const ItemId id) const;

  bool hasDynamiteInGeneral() const;

  void decrDynamiteInGeneral();

  void sortGeneralInventory();

  int getTotalItemWeight() const;

  void storeToSaveLines(std::vector<std::string>& lines) const;
  void setupFromSaveLines(std::vector<std::string>& lines);

  InvSlot               slots_[int(SlotId::END)];
  std::vector<Item*>    general_;
  std::vector<Item*>    intrinsics_;
};

#endif
