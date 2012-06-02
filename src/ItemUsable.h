#ifndef ITEM_USABLE_H
#define ITEM_USABLE_H

#include "Item.h"
#include "Actor.h"

class ItemUsable: public Item
{
public:
ItemUsable(ItemDefinition* const itemDefinition) : 
   Item(itemDefinition) {}
   virtual ~ItemUsable() {}

   virtual ItemActivateReturn_t use(Actor* actor, Engine* engine) = 0;
  
private:
};

/*
class ItemMedicinePouch: public ItemUsable
{
public:
ItemMedicinePouch(ItemDefinition* const itemDefinition) : 
   ItemUsable(itemDefinition) {}

   ~ItemMedicinePouch() {}

   ItemActivateReturnData use(Actor* actor, Engine* engine);
private:
};
*/

/*
class ItemFirstAidKit: public ItemUsable
{
public:
ItemFirstAidKit(ItemDefinition* const itemDefinition) : 
   ItemUsable(itemDefinition) {}

   ~ItemFirstAidKit() {}

   ItemActivateReturnData use(Actor* actor, Engine* engine);
private:
};
*/

/*
class ItemDoctorsBag: public ItemUsable
{
public:
ItemDoctorsBag(ItemDefinition* const itemDefinition) : 
   ItemUsable(itemDefinition) {}

   ~ItemDoctorsBag() {}

   ItemActivateReturnData use(Actor* actor, Engine* engine);
private:
};
*/

#endif
