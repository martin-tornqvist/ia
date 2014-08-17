#include "ItemPickup.h"

#include <string>

#include "Item.h"

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

} //namespace

//Can always be called, to check if something is there to be picked up.
void tryPick() {
  const Pos& pos = Map::player->pos;
  Item* const item = Map::cells[pos.x][pos.y].item;

  if(item) {
    Inventory& playerInv = Map::player->getInv();

    const string ITEM_NAME = item->getName(ItemRefType::plural);

    //If picked up item is missile weapon, try to add it to carried stack.
    if(item->getData().ranged.isThrowingWpn) {
      Item* const carriedMissile = playerInv.getItemInSlot(SlotId::thrown);
      if(carriedMissile) {
        if(item->getData().id == carriedMissile->getData().id) {
          Audio::play(SfxId::pickup);

          Log::addMsg("I add " + ITEM_NAME + " to my missile stack.");
          carriedMissile->nrItems_ += item->nrItems_;
          delete item;
          Map::cells[pos.x][pos.y].item = nullptr;
          GameTime::actorDidAct();
          return;
        }
      }
    }

    Audio::play(SfxId::pickup);

    Log::clearLog();
    Log::addMsg("I pick up " + ITEM_NAME + ".");

    playerInv.putInGeneral(item);

    Map::cells[pos.x][pos.y].item = nullptr;

    GameTime::actorDidAct();

  } else {
    Log::clearLog();
    Log::addMsg("I see nothing to pick up here.");
  }
}

void tryUnloadWpnOrPickupAmmo() {
  Item* item = Map::cells[Map::player->pos.x][Map::player->pos.y].item;

  if(item) {
    if(item->getData().ranged.isRangedWpn) {
      Wpn* const wpn = static_cast<Wpn*>(item);
      const int nrAmmoLoaded = wpn->nrAmmoLoaded;

      if(nrAmmoLoaded > 0 && !wpn->getData().ranged.hasInfiniteAmmo) {
        Inventory& playerInv = Map::player->getInv();
        const ItemId ammoType = wpn->getData().ranged.ammoItemId;

        ItemDataT* const ammoData = ItemData::data[int(ammoType)];

        Item* spawnedAmmo = ItemFactory::mk(ammoType);

        if(ammoData->isAmmoClip) {
          //Unload a clip
          static_cast<AmmoClip*>(spawnedAmmo)->ammo_ = nrAmmoLoaded;
        } else {
          //Unload loose ammo
          spawnedAmmo->nrItems_ = nrAmmoLoaded;
        }
        Log::addMsg("I unload " + wpn->getName(ItemRefType::a));

        Audio::play(SfxId::pickup);

        playerInv.putInGeneral(spawnedAmmo);

        static_cast<Wpn*>(item)->nrAmmoLoaded = 0;

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
