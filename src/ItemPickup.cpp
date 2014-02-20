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
#include "GameTime.h"
#include "Audio.h"

//--------------ITEM PICKUP EFFECTS--------------
void ItemPickup::pickupEffects(Actor* actor, Item* item) {
  (void)actor;
  (void)item;
}

//Can always be called, to check if something is there to be picked up.
void ItemPickup::tryPick() {
  const Pos& pos = eng.player->pos;
  Item* const item = eng.map->cells[pos.x][pos.y].item;

  if(item == NULL) {
    eng.log->clearLog();
    eng.log->addMsg("I see nothing to pick up here.");
  } else {
    Inventory& playerInv = eng.player->getInv();

    const string ITEM_NAME =
      eng.itemDataHandler->getItemInterfaceRef(*item, true);

    //If picked up item is missile weapon, try to add it to carried stack.
    if(item->getData().isMissileWeapon) {
      Item* const carriedMissile = playerInv.getItemInSlot(slot_missiles);
      if(carriedMissile != NULL) {
        if(item->getData().id == carriedMissile->getData().id) {
          Audio::play(SfxId::pickup);

          eng.log->addMsg("I add " + ITEM_NAME + " to my missile stack.");
          carriedMissile->nrItems += item->nrItems;
          delete item;
          eng.map->cells[pos.x][pos.y].item = NULL;
          eng.gameTime->actorDidAct();
          return;
        }
      }
    }

    if(isInvFull(playerInv, *item)) {
      eng.log->clearLog();
      eng.log->addMsg("I cannot carry more.");
    } else {
      Audio::play(SfxId::pickup);

      eng.log->clearLog();
      eng.log->addMsg("I pick up " + ITEM_NAME + ".");

      playerInv.putItemInGeneral(item);

      eng.map->cells[pos.x][pos.y].item = NULL;

      eng.gameTime->actorDidAct();
    }
  }
}

bool ItemPickup::isInvFull(Inventory& inv, Item& item) const {
  //If item can be stacked, the inventory is not considered full.
  if(inv.getElementToStackItem(&item) != -1) {return false;}
  const int INV_SIZE = inv.getGeneral().size();
  return INV_SIZE >= (SCREEN_H - 1);
}

void ItemPickup::tryUnloadWeaponOrPickupAmmoFromGround() {
  Item* item = eng.map->cells[eng.player->pos.x][eng.player->pos.y].item;

  if(item != NULL) {
    if(item->getData().isRangedWeapon) {
      Weapon* const weapon = dynamic_cast<Weapon*>(item);
      const int nrAmmoLoaded = weapon->nrAmmoLoaded;

      if(nrAmmoLoaded > 0 && weapon->getData().rangedHasInfiniteAmmo == false) {
        Inventory& playerInv = eng.player->getInv();
        const ItemId ammoType = weapon->getData().rangedAmmoTypeUsed;

        ItemData* const ammoData = eng.itemDataHandler->dataList[ammoType];

        Item* spawnedAmmo = eng.itemFactory->spawnItem(ammoType);

        if(ammoData->isAmmoClip == true) {
          //Unload a clip
          dynamic_cast<ItemAmmoClip*>(spawnedAmmo)->ammo = nrAmmoLoaded;
        } else {
          //Unload loose ammo
          spawnedAmmo->nrItems = nrAmmoLoaded;
        }
        const string WEAPON_REF_A =
          eng.itemDataHandler->getItemRef(*weapon, itemRef_a);
        eng.log->addMsg("I unload " + WEAPON_REF_A);

        if(isInvFull(playerInv, *spawnedAmmo) == false) {
          Audio::play(SfxId::pickup);
          playerInv.putItemInGeneral(spawnedAmmo);
        } else {
          Audio::play(SfxId::pickup);
          eng.itemDrop->dropItemOnMap(eng.player->pos, *spawnedAmmo);
          string str =  "I have no room to keep the unloaded ammunition.";
          eng.log->addMsg(str);
        }

        dynamic_cast<Weapon*>(item)->nrAmmoLoaded = 0;

        eng.gameTime->actorDidAct();
        return;
      }
    } else {
      if(item->getData().isAmmo) {
        tryPick();
        return;
      }
    }
  }

  eng.log->addMsg("I see no ammo to unload or pick up here.");
}

