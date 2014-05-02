#include "ItemPickup.h"

#include <string>

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

using namespace std;

namespace ItemPickup {

namespace {

void pickupEffects(Actor* actor, Item* item) {
  (void)actor;
  (void)item;
}

bool isInvFull(Inventory& inv, Item& item) {
  //If item can be stacked, the inventory is not considered full.
  if(inv.getElementToStackItem(&item) != -1) {return false;}
  const int INV_SIZE = inv.getGeneral().size();
  return INV_SIZE >= (SCREEN_H - 1);
}

} //namespace

//Can always be called, to check if something is there to be picked up.
void tryPick() {
  const Pos& pos = Map::player->pos;
  Item* const item = Map::cells[pos.x][pos.y].item;

  if(item == NULL) {
    Log::clearLog();
    Log::addMsg("I see nothing to pick up here.");
  } else {
    Inventory& playerInv = Map::player->getInv();

    const string ITEM_NAME = ItemData::getItemInterfaceRef(*item, true);

    //If picked up item is missile weapon, try to add it to carried stack.
    if(item->getData().isMissileWeapon) {
      Item* const carriedMissile = playerInv.getItemInSlot(SlotId::missiles);
      if(carriedMissile != NULL) {
        if(item->getData().id == carriedMissile->getData().id) {
          Audio::play(SfxId::pickup);

          Log::addMsg("I add " + ITEM_NAME + " to my missile stack.");
          carriedMissile->nrItems += item->nrItems;
          delete item;
          Map::cells[pos.x][pos.y].item = NULL;
          GameTime::actorDidAct();
          return;
        }
      }
    }

    if(isInvFull(playerInv, *item)) {
      Log::clearLog();
      Log::addMsg("I cannot carry more.");
    } else {
      Audio::play(SfxId::pickup);

      Log::clearLog();
      Log::addMsg("I pick up " + ITEM_NAME + ".");

      playerInv.putInGeneral(item);

      Map::cells[pos.x][pos.y].item = NULL;

      GameTime::actorDidAct();
    }
  }
}

void tryUnloadWeaponOrPickupAmmoFromGround() {
  Item* item = Map::cells[Map::player->pos.x][Map::player->pos.y].item;

  if(item != NULL) {
    if(item->getData().isRangedWeapon) {
      Weapon* const weapon = dynamic_cast<Weapon*>(item);
      const int nrAmmoLoaded = weapon->nrAmmoLoaded;

      if(nrAmmoLoaded > 0 && weapon->getData().rangedHasInfiniteAmmo == false) {
        Inventory& playerInv = Map::player->getInv();
        const ItemId ammoType = weapon->getData().rangedAmmoTypeUsed;

        ItemDataT* const ammoData = ItemData::data[int(ammoType)];

        Item* spawnedAmmo = ItemFactory::spawnItem(ammoType);

        if(ammoData->isAmmoClip == true) {
          //Unload a clip
          dynamic_cast<ItemAmmoClip*>(spawnedAmmo)->ammo = nrAmmoLoaded;
        } else {
          //Unload loose ammo
          spawnedAmmo->nrItems = nrAmmoLoaded;
        }
        const string WEAPON_REF_A =
          ItemData::getItemRef(*weapon, ItemRefType::a);
        Log::addMsg("I unload " + WEAPON_REF_A);

        if(isInvFull(playerInv, *spawnedAmmo) == false) {
          Audio::play(SfxId::pickup);
          playerInv.putInGeneral(spawnedAmmo);
        } else {
          Audio::play(SfxId::pickup);
          ItemDrop::dropItemOnMap(Map::player->pos, *spawnedAmmo);
          string str =  "I have no room to keep the unloaded ammunition.";
          Log::addMsg(str);
        }

        dynamic_cast<Weapon*>(item)->nrAmmoLoaded = 0;

        GameTime::actorDidAct();
        return;
      }
    } else {
      if(item->getData().isAmmo) {
        tryPick();
        return;
      }
    }
  }

  Log::addMsg("I see no ammo to unload or pick up here.");
}

} //ItemPickup
