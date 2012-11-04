#include "Reload.h"

#include "Engine.h"

#include "Converters.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "ItemAmmoClip.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "DungeonMaster.h"
#include "PlayerBonuses.h"

void Reload::printReloadMessages(Actor* actorReloading, Weapon* weapon, Item* ammoItem, ReloadResult_t result, bool isSwift) {
  const string actorName = actorReloading->getNameThe();
  const string weaponName = weapon == NULL ? "" : weapon->getDef().name.name;
  const string ammoCapacity = weapon == NULL ? "" : intToString(weapon->ammoCapacity);
  const string ammoCurrent = weapon == NULL ? "" : intToString(weapon->ammoLoaded);
  const bool isPlayer = actorReloading == eng->player;
  const int reloaderX = actorReloading->pos.x;
  const int reloaderY = actorReloading->pos.y;

  string ammoName = " ";
  bool isClip = false;

  if(ammoItem != NULL) {
    ammoName = ammoItem->getDef().name.name_a;
    isClip = ammoItem->getDef().isAmmoClip == true;
  }

  switch(result) {
  case reloadResult_weaponNotUsingAmmo: {
    if(isPlayer == true) {
      eng->log->addMessage("Weapon does not use ammo.");
//			eng->renderer->flip();
    }
  }
  break;

  case reloadResult_alreadyFull: {
    if(isPlayer == true) {
      eng->log->addMessage("Weapon already loaded.");
//			eng->renderer->flip();
    }
  }
  break;

  case reloadResult_noAmmo: {
    if(isPlayer == true) {
      eng->log->addMessage("I carry no ammunition for this weapon.");
//			eng->renderer->flip();
    }
  }
  break;

  case reloadResult_success: {
    const string swiftStr = isSwift ? " swiftly" : "";
    if(isPlayer) {
      if(isClip) {
        eng->log->addMessage("I" + swiftStr + " reload the " + weaponName + ".");
      } else {
        eng->log->addMessage("I" + swiftStr + " load " + ammoName + ".");
      }
      eng->renderer->drawMapAndInterface();
    } else {
      if(eng->map->playerVision[reloaderX][reloaderY]) {
        eng->log->addMessage(actorName + swiftStr + " reloads.");
      }
    }
  }
  break;

  default:
    break;
  }
}

bool Reload::reloadWeapon(Actor* actorReloading) {
  bool acted = false;

  Inventory* inv = actorReloading->getInventory();
  vector<Item*>* genInv = inv->getGeneral();
  Weapon* weaponToReload = dynamic_cast<Weapon*>(inv->getItemInSlot(slot_wielded));
  ReloadResult_t result = reloadResult_noAmmo;

  bool isSwiftReload = false;
  const bool IS_PLAYER = actorReloading == eng->player;
  if(IS_PLAYER) {
    isSwiftReload = eng->playerBonusHandler->isBonusPicked(playerBonus_nimble) && eng->dice.coinToss();
  }

  if(weaponToReload != NULL) {
    const int weaponAmmoCap = weaponToReload->ammoCapacity;

    if(weaponAmmoCap == 0) {
      printReloadMessages(actorReloading, NULL, NULL, reloadResult_weaponNotUsingAmmo, false);
    } else {
      const ItemDevNames_t ammoType = weaponToReload->getDef().rangedAmmoTypeUsed;
      Item* ammoItem = NULL;
      bool isClip = weaponToReload->clip;

      if(weaponToReload->ammoLoaded < weaponAmmoCap) {
        for(unsigned int i = 0; i < genInv->size(); i++) {
          ammoItem = genInv->at(i);

          if(ammoItem->getDef().devName == ammoType) {
            result = reloadResult_success;
            isClip = ammoItem->getDef().isAmmoClip;

            //If ammo comes in clips
            if(isClip == true) {

              const int previousAmmoCount = weaponToReload->ammoLoaded;

              ItemAmmoClip* clipItem = dynamic_cast<ItemAmmoClip*>(ammoItem);

              weaponToReload->ammoLoaded = clipItem->ammo;

              const Audio_t reloadAudio = weaponToReload->getDef().reloadAudio;
              if(reloadAudio != audio_none) {
                eng->audio->playSound(reloadAudio);
              }

              printReloadMessages(actorReloading, weaponToReload, ammoItem, result, isSwiftReload);

              //Erase loaded clip
              inv->deleteItemInGeneral(i);

              //If weapon previously contained ammo, create a new clip item
              if(previousAmmoCount > 0) {
                ammoItem = eng->itemFactory->spawnItem(ammoType);
                clipItem = dynamic_cast<ItemAmmoClip*>(ammoItem);
                clipItem->ammo = previousAmmoCount;
                inv->putItemInGeneral(clipItem);
              }
            }
            //Else ammo is a pile
            else {
              weaponToReload->ammoLoaded += 1;

              const Audio_t reloadAudio = weaponToReload->getDef().reloadAudio;
              if(reloadAudio != audio_none) {
                eng->audio->playSound(reloadAudio);
              }

              printReloadMessages(actorReloading, weaponToReload, ammoItem, result, isSwiftReload);

              //Decrease ammo "number" by one
              inv->decreaseItemInGeneral(i);
            }
            i = 999999;
          }
        }
        if(result == reloadResult_noAmmo) {
          printReloadMessages(actorReloading, weaponToReload, ammoItem, result, isSwiftReload);
        }
      } else {
        result = reloadResult_alreadyFull;
        printReloadMessages(actorReloading, weaponToReload, ammoItem, result, isSwiftReload);
      }
    }
  }

  if(result == reloadResult_success) {
    acted = true;
    if(isSwiftReload == false) {
      eng->gameTime->letNextAct();
    }
  }

  return acted;
}

