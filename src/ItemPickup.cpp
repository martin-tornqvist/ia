#include "ItemPickup.h"

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "ItemAmmo.h"
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

    const string ITEM_NAME =
      eng->itemDataHandler->getItemInterfaceRef(*item, true);

    //If picked up item is missile weapon, try to add it to carried stack.
    if(item->getData().isMissileWeapon) {
      Item* const carriedMissile = playerInventory->getItemInSlot(slot_missiles);
      if(carriedMissile != NULL) {
        if(item->getData().id == carriedMissile->getData().id) {
          eng->log->addMsg("I add " + ITEM_NAME + " to my missile stack.");
          carriedMissile->nrItems += item->nrItems;
          delete item;
          eng->map->items[eng->player->pos.x][eng->player->pos.y] = NULL;
          eng->gameTime->endTurnOfCurrentActor();
          return;
        }
      }
    }

    if(isInventoryFull(playerInventory, item) == false) {
      eng->log->clearLog();
      eng->log->addMsg("I pick up " + ITEM_NAME + ".");

      playerInventory->putItemInGeneral(item);

      eng->map->items[eng->player->pos.x][eng->player->pos.y] = NULL;

      eng->gameTime->endTurnOfCurrentActor();
    } else {
      eng->log->clearLog();
      eng->log->addMsg("I cannot carry more.");
    }
  } else {
    eng->log->clearLog();
    eng->log->addMsg("I see nothing to pick up here.");
  }
}

bool ItemPickup::isInventoryFull(Inventory* inventory, Item* item) const {
  //If item can be stacked, the inventory is not considered full.
  if(inventory->getElementToStackItem(item) != -1) {
    return false;
  }

  // Old way (full = slots + general >= z + 4):
//  const int NR_ITEMS = inventory->getSlots()->size() + inventory->getGeneral()->size();
//  return NR_ITEMS + int('a') - 1 >= int('z') + 4;
  return inventory->getGeneral()->size() + 'a' - 1 >= 'z';
}

void ItemPickup::tryUnloadWeaponOrPickupAmmoFromGround() {
  Item* item = eng->map->items[eng->player->pos.x][eng->player->pos.y];

  if(item != NULL) {
    if(item->getData().isRangedWeapon) {
      Weapon* const weapon = dynamic_cast<Weapon*>(item);
      const int nrAmmoLoaded = weapon->nrAmmoLoaded;

      if(nrAmmoLoaded > 0 && weapon->getData().rangedHasInfiniteAmmo == false) {
        Inventory* const playerInventory = eng->player->getInventory();
        const ItemId_t ammoType = weapon->getData().rangedAmmoTypeUsed;

        ItemData* const ammoData = eng->itemDataHandler->dataList[ammoType];

        Item* spawnedAmmo = eng->itemFactory->spawnItem(ammoType);

        if(ammoData->isAmmoClip == true) {
          //Unload a clip
          dynamic_cast<ItemAmmoClip*>(spawnedAmmo)->ammo = nrAmmoLoaded;
        } else {
          //Unload loose ammo
          spawnedAmmo->nrItems = nrAmmoLoaded;
        }
        const string WEAPON_REF_A =
          eng->itemDataHandler->getItemRef(*weapon, itemRef_a);
        eng->log->addMsg("I unload " + WEAPON_REF_A);

        if(isInventoryFull(playerInventory, spawnedAmmo) == false) {
          playerInventory->putItemInGeneral(spawnedAmmo);
        } else {
          eng->itemDrop->dropItemOnMap(eng->player->pos, *spawnedAmmo);
          string str =  "I have no room to keep the unloaded ammunition.";
          eng->log->addMsg(str);
        }

        dynamic_cast<Weapon*>(item)->nrAmmoLoaded = 0;
//        dynamic_cast<Weapon*>(item)->setColorForAmmoStatus();

        eng->gameTime->endTurnOfCurrentActor();
        return;
      }
    } else {
      if(item->getData().isAmmo) {
        tryPick();
        return;
      }
    }
  }

  eng->log->addMsg("I see no ammo to unload or pick up here.");
}

