#include "ItemPickup.h"

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "ItemAmmoClip.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Query.h"
#include "ItemDrop.h"
#include "ItemFactory.h"

//--------------ITEM PICKUP EFFECTS--------------
void ItemPickup::pickupEffects(Actor* actor, Item* item) {
  (void)actor;
  (void)item;
}

// This function can always be called to check if something is there to be picked up.
void ItemPickup::tryPick() {
  Item* const item = eng->map->items[eng->player->pos.x][eng->player->pos.y];

  if(item != NULL) {
    Inventory* const playerInventory = eng->player->getInventory();

    const string ITEM_NAME = eng->itemData->itemInterfaceName(item, true);

    //If picked up item is missile weapon, try to add it to carried stack.
    if(item->getDef().isMissileWeapon) {
      Item* const carriedMissile = playerInventory->getItemInSlot(slot_missiles);
      if(carriedMissile != NULL) {
        if(item->getDef().devName == carriedMissile->getDef().devName) {
          eng->log->addMessage("I add " + ITEM_NAME + " to my missile stack.");
          carriedMissile->numberOfItems += item->numberOfItems;
          delete item;
          eng->map->items[eng->player->pos.x][eng->player->pos.y] = NULL;
          eng->gameTime->letNextAct();
          return;
        }
      }
    }

    if(isInventoryFull(playerInventory, item) == false) {
      eng->log->clearLog();
      eng->log->addMessage("I pick up " + ITEM_NAME + ".");

      playerInventory->putItemInGeneral(item);

      eng->map->items[eng->player->pos.x][eng->player->pos.y] = NULL;

      eng->gameTime->letNextAct();
    } else {
      eng->log->clearLog();
      eng->log->addMessage("I can not carry more.");
    }
  } else {
    eng->log->clearLog();
    eng->log->addMessage("I see nothing to pick up here.");
  }
}

bool ItemPickup::isInventoryFull(Inventory* inventory, Item* item) const {
  //If item can be stacked, the inventory is not considered full.
  if(inventory->getElementToStackItem(item) != -1) {
    return false;
  }

  const int NR_ITEMS = inventory->getSlots()->size() + inventory->getGeneral()->size();
  return NR_ITEMS + static_cast<int>('a') - 1 >= static_cast<int>('z') + 4;
}

void ItemPickup::tryUnloadWeaponOrPickupAmmoFromGround() {
  Item* item = eng->map->items[eng->player->pos.x][eng->player->pos.y];

  if(item != NULL) {
    if(item->getDef().isRangedWeapon) {
      Weapon* const weapon = dynamic_cast<Weapon*>(item);
      const int ammoLoaded = weapon->ammoLoaded;

      if(ammoLoaded > 0 && weapon->getDef().rangedHasInfiniteAmmo == false) {
        Inventory* const playerInventory = eng->player->getInventory();
        const ItemDevNames_t ammoType = weapon->getDef().rangedAmmoTypeUsed;

        ItemDefinition* const ammoDef = eng->itemData->itemDefinitions[ammoType];

        Item* spawnedAmmo = eng->itemFactory->spawnItem(ammoType);

        if(ammoDef->isAmmoClip == true) {
          //Unload a clip
          dynamic_cast<ItemAmmoClip*>(spawnedAmmo)->ammo = ammoLoaded;
        } else {
          //Unload loose ammo
          spawnedAmmo->numberOfItems = ammoLoaded;
        }
        const string WEAPON_REF_A = weapon->getDef().name.name_a;
        eng->log->addMessage("I unload " + WEAPON_REF_A);

        if(isInventoryFull(playerInventory, spawnedAmmo) == false) {
          playerInventory->putItemInGeneral(spawnedAmmo);
        } else {
          eng->itemDrop->dropItemOnMap(eng->player->pos, &spawnedAmmo);
          eng->log->addMessage("I have no room to keep the unloaded ammunition, item dropped on ground.");
        }

        dynamic_cast<Weapon*>(item)->ammoLoaded = 0;
//        dynamic_cast<Weapon*>(item)->setColorForAmmoStatus();

        eng->gameTime->letNextAct();
        return;
      }
    } else {
      if(item->getDef().isAmmo) {
        tryPick();
        return;
      }
    }
  }

  eng->log->addMessage("I see no ammo to unload or pick up here.");
}

