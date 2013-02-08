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

  bool allowWieldedWeapon, allowMissile, allowArmor, allowCloak, allowAmulet, allowRing;

  string interfaceName;

  SlotTypes_t devName;
  Item* item;
};

class Engine;

class Inventory {
public:
  Inventory(bool humanoid);
  ~Inventory() {
    //Actors destructor deletes actors inventory.
    //Inventories destructor deletes all its items.
    for(unsigned int i = 0; i < m_slots.size(); i++)
      if(m_slots.at(i).item != NULL)
        delete m_slots.at(i).item;

    for(unsigned int i = 0; i < m_general.size(); i++)
      delete m_general.at(i);

    for(unsigned int i = 0; i < m_intrinsics.size(); i++)
      delete m_intrinsics.at(i);
  }

  void dropAllNonIntrinsic(const coord pos, const bool rollForDestruction, Engine* engine);

  bool hasItemInSlot(SlotTypes_t slotName);

  void putItemInSlot(SlotTypes_t slotName, Item* item, bool putInGeneral_ifOccupied = true, bool putInGeneral_ifSlotNotFound = true);

  void putItemInGeneral(Item* item);

  int getElementToStackItem(Item* item) const;

  void putItemInIntrinsics(Item* item);

  bool moveItemToGeneral(InventorySlot* inventorySlot);

  void moveItemFromGeneralToIntrinsics(const unsigned int GENERAL_INV_ELEMENT);

  void moveItemToSlot(InventorySlot* inventoryslot, const unsigned int GENERAL_INV_ELEMENT);

  void equipGeneralItemAndPossiblyEndTurn(const unsigned int GENERAL_INV_ELEMENT,
                                          const SlotTypes_t slotToEquip, Engine* engine);

//  void equipGeneralItemToAltAndPossiblyEndTurn(const unsigned int GENERAL_INV_ELEMENT, Engine* engine);

  void swapWieldedAndPrepared(const bool END_TURN, Engine* engine);

  bool hasAmmoForFirearmInInventory();

  int getElementWithItemType(const ItemDevNames_t itemDevName) const;

  Item* getItemInSlot(SlotTypes_t slotName);
  Item* getItemInElement(const int GLOBAL_ELEMENT_NR);

  void removeItemInElementWithoutDeletingInstance(int number);

  void decreaseItemInSlot(SlotTypes_t slotName);

  void decreaseItemInGeneral(unsigned element);

  void decreaseItemTypeInGeneral(const ItemDevNames_t devName);

  void deleteItemInGeneral(unsigned element);

  int getIntrinsicsSize() {
    return m_intrinsics.size();
  }

  Item* getIntrinsicInElement(int element);

  Item* getLastItemInGeneral();

  bool hasItemInGeneral(const ItemDevNames_t devName) const;

  int getItemStackSizeInGeneral(const ItemDevNames_t devName) const;

  bool hasDynamiteInGeneral() const;

  void decreaseDynamiteInGeneral();

  //bool hasFirstAidInGeneral();
  //void decreaseFirstAidInGeneral();

  InventorySlot* getSlot(SlotTypes_t slotName);

  void sortGeneralInventory(Engine* const engine);

  vector<InventorySlot>* getSlots() {
    return &m_slots;
  }
  vector<Item*>* getGeneral() {
    return &m_general;
  }

  int getTotalItemWeight() const;

  void addSaveLines(vector<string>& lines) const;
  void setParametersFromSaveLines(vector<string>& lines, Engine* const engine);

private:
  vector<InventorySlot> m_slots;
  vector<Item*> m_general;
  vector<Item*> m_intrinsics;
};

#endif
