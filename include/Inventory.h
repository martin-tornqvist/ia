#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <string>

#include "CmnTypes.h"

class Item;

enum class ItemId;

enum class SlotId
{
  wielded,
  wieldedAlt,
  thrown,
  body,
  head,
  neck,
  ring1,
  ring2,
  END
};

struct InvSlot
{
  InvSlot(SlotId id_, std::string name_) :
    id    (id_),
    name  (name_),
    item  (nullptr) {}

  InvSlot() :
    id    (SlotId::wielded),
    name  (""),
    item  (nullptr) {}

  SlotId      id;
  std::string name;
  Item*       item;
};

class Inventory
{
public:
  Inventory();

  ~Inventory();

  void dropAllNonIntrinsic(const Pos& pos);

  bool hasItemInSlot(SlotId id) const;

  //Note: This will put the item in general if the slot is occupied
  void putInSlot(const SlotId id, Item* item);

  void putInGeneral(Item* item);

  void putInIntrinsics(Item* item);

  bool moveToGeneral(const SlotId id);

  void moveFromGeneralToIntrinsics(const size_t GEN_IDX);

  void moveItemToSlot(InvSlot& slot, const size_t GEN_IDX);

  void equipGeneralItem(const size_t GEN_IDX, const SlotId slotId);

  void swapWieldedAndPrepared(const bool IS_FREE_TURN);

  bool hasAmmoForFirearmInInventory();

  Item* getFirstItemInBackpackWithId(const ItemId id);

  int getBackpackIdxWithItemId(const ItemId itemId) const;

  Item* getItemInSlot(const SlotId id) const;

  void removeWithoutDestroying(const InvType invType, const size_t IDX);

  void decrItemInSlot(SlotId slotId);

  void decrItemInGeneral(const size_t IDX);

  void decrItem(Item* const item);

  void decrItemTypeInGeneral(const ItemId itemId);

  void removeItemInBackpackWithIdx(const size_t IDX, const bool DELETE_ITEM);
  void removeItemInBackpackWithPtr(Item* const item, const bool DELETE_ITEM);

  int getIntrinsicsSize() const {return intrinsics_.size();}

  Item* getIntrinsicInElement(const int IDX) const;

  Item* getLastItemInGeneral();

  bool hasItemInBackpack(const ItemId id) const;

  int getItemStackSizeInGeneral(const ItemId id) const;

  void decrDynamiteInGeneral();

  void sortGeneralInventory();

  int getTotalItemWeight() const;

  void storeToSaveLines  (std::vector<std::string>& lines) const;
  void setupFromSaveLines(std::vector<std::string>& lines);

  InvSlot               slots_[int(SlotId::END)];
  std::vector<Item*>    general_;
  std::vector<Item*>    intrinsics_;
};

#endif
