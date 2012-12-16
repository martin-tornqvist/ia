#include "Reload.h"

#include "Engine.h"

#include "Converters.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "ItemAmmo.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "DungeonMaster.h"
#include "PlayerBonuses.h"

void Reload::printReloadMessages(Actor* actorReloading, Weapon* weapon, Item* ammoItem, ReloadResult_t result, bool isSwift) {
  const string actorName = actorReloading->getNameThe();
  const string weaponName = weapon == NULL ? "" : eng->itemData->getItemRef(weapon, itemRef_plain, true);
  const string ammoCapacity = weapon == NULL ? "" : intToString(weapon->ammoCapacity);
  const string ammoCurrent = weapon == NULL ? "" : intToString(weapon->ammoLoaded);
  const bool isPlayer = actorReloading == eng->player;

  string ammoName = " ";
  bool isClip = false;

  if(ammoItem != NULL) {
    ammoName = eng->itemData->getItemRef(ammoItem, itemRef_a);
    isClip = ammoItem->getDef().isAmmoClip == true;
  }

  switch(result) {
  case reloadResult_weaponNotUsingAmmo: {
    if(isPlayer == true) {
      eng->log->addMessage("Weapon does not use ammo.");
    }
  } break;

  case reloadResult_alreadyFull: {
    if(isPlayer == true) {
      eng->log->addMessage("Weapon already loaded.");
    }
  } break;

  case reloadResult_noAmmo: {
    if(isPlayer == true) {
      eng->log->addMessage("I carry no ammunition for this weapon.");
    }
  } break;

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
      if(eng->player->checkIfSeeActor(*actorReloading, NULL)) {
        eng->log->addMessage(actorName + swiftStr + " reloads.");
      }
    }
  } break;

  case reloadResult_fumble: {
    if(isPlayer) {
      eng->log->addMessage("I fumble with the " + ammoName + ".");
    } else {
      if(eng->player->checkIfSeeActor(*actorReloading, NULL)) {
        eng->log->addMessage(actorName + " fumbles with " + ammoName + ".");
      }
    }
  } break;

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
            const bool IS_RELOADER_BLIND = actorReloading->getStatusEffectsHandler()->allowSee() == false;
            const bool IS_REALOADER_TERRIFIED = actorReloading->getStatusEffectsHandler()->hasEffect(statusTerrified);
            const int CHANCE_TO_FUMBLE = (IS_RELOADER_BLIND ? 40 : 0) + (IS_REALOADER_TERRIFIED ? 40 : 0);

            if(eng->dice(1, 100) < CHANCE_TO_FUMBLE) {
              isSwiftReload = false;
              result = reloadResult_fumble;
              printReloadMessages(actorReloading, NULL, NULL, reloadResult_fumble, false);
            } else {
              result = reloadResult_success;
              isClip = ammoItem->getDef().isAmmoClip;

              //If ammo comes in clips
              if(isClip) {
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
            }
            break;
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

  if(result == reloadResult_success || result == reloadResult_fumble) {
    acted = true;
    if(isSwiftReload == false) {
      eng->gameTime->letNextAct();
    }
  }

  return acted;
}

