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
#include "GameTime.h"
#include "Audio.h"
#include "Renderer.h"

void Reload::printMsgAndPlaySfx(Actor& actorReloading, Weapon* const wpn,
                                Item* const ammo, const ReloadResult_t result,
                                const bool IS_SWIFT_RELOAD) {
  const string actorName    = actorReloading.getNameThe();
  const string ammoCapacity = wpn == NULL ? "" : toString(wpn->ammoCapacity);
  const string ammoCurrent  = wpn == NULL ? "" : toString(wpn->nrAmmoLoaded);
  const bool IS_PLAYER      = &actorReloading == eng->player;

  string ammoName = " ";
  bool isClip = false;

  if(ammo != NULL) {
    ammoName = eng->itemDataHandler->getItemRef(*ammo, itemRef_a);
    isClip = ammo->getData().isAmmoClip;
  }

  switch(result) {
    case reloadResult_notCarryingWpn: {
      if(IS_PLAYER) {
        eng->log->addMsg("I am not wielding a weapon.");
      }
    } break;

    case reloadResult_wpnNotUsingAmmo: {
      if(IS_PLAYER) {
        eng->log->addMsg("Weapon does not use ammo.");
      }
    } break;

    case reloadResult_alreadyFull: {
      if(IS_PLAYER) {
        eng->log->addMsg("Weapon already loaded.");
      }
    } break;

    case reloadResult_noAmmo: {
      if(IS_PLAYER) {
        eng->log->addMsg("I carry no ammunition for this weapon.");
      }
    } break;

    case reloadResult_success: {
      const string swiftStr = IS_SWIFT_RELOAD ? " swiftly" : "";
      if(IS_PLAYER) {

        eng->audio->play(wpn->getData().reloadSfx);

        if(isClip) {
          const string wpnName =
            eng->itemDataHandler->getItemRef(*wpn, itemRef_plain, true);
          eng->log->addMsg(
            "I" + swiftStr + " reload the " + wpnName + ".");
        } else {
          eng->log->addMsg("I" + swiftStr + " load " + ammoName + ".");
        }
        eng->renderer->drawMapAndInterface();
      } else {
        if(eng->player->checkIfSeeActor(actorReloading, NULL)) {
          eng->log->addMsg(actorName + swiftStr + " reloads.");
        }
      }
    } break;

    case reloadResult_fumble: {
      if(IS_PLAYER) {
        eng->log->addMsg("I fumble with the " + ammoName + ".");
      } else {
        if(eng->player->checkIfSeeActor(actorReloading, NULL)) {
          eng->log->addMsg(actorName + " fumbles with " + ammoName + ".");
        }
      }
    } break;
  }
}

bool Reload::reloadWieldedWpn(Actor& actorReloading) {
  bool didAct = false;

  Inventory* const inv  = actorReloading.getInventory();
  Item* const wpnItem   = inv->getItemInSlot(slot_wielded);

  if(wpnItem == NULL) {
    printMsgAndPlaySfx(actorReloading, NULL, NULL,
                       reloadResult_notCarryingWpn, false);
    return didAct;
  }

  Weapon* const wpn     = dynamic_cast<Weapon*>(wpnItem);
  ReloadResult_t result = reloadResult_noAmmo;
  bool isSwiftReload    = false;

  if(&actorReloading == eng->player) {
    isSwiftReload = eng->playerBonHandler->isTraitPicked(traitMarksman) &&
                    eng->dice.coinToss();
  }

  const int wpnAmmoCapacity = wpn->ammoCapacity;

  if(wpnAmmoCapacity == 0) {
    printMsgAndPlaySfx(actorReloading, wpn, NULL,
                       reloadResult_wpnNotUsingAmmo, false);
  } else {
    const ItemId_t ammoType = wpn->getData().rangedAmmoTypeUsed;
    Item* ammo = NULL;
    bool isClip = wpn->clip;

    if(wpn->nrAmmoLoaded < wpnAmmoCapacity) {

      const vector<Item*>* genInv = inv->getGeneral();

      for(unsigned int i = 0; i < genInv->size(); i++) {
        ammo = genInv->at(i);

        if(ammo->getData().id == ammoType) {
          const bool IS_RELOADER_BLIND =
            actorReloading.getPropHandler()->allowSee() == false;
          const bool IS_REALOADER_TERRIFIED =
            actorReloading.getPropHandler()->hasProp(propTerrified);
          const int CHANCE_TO_FUMBLE =
            (IS_RELOADER_BLIND ? 48 : 0) + (IS_REALOADER_TERRIFIED ? 48 : 0);

          if(eng->dice.percentile() < CHANCE_TO_FUMBLE) {
            isSwiftReload = false;
            result = reloadResult_fumble;
            printMsgAndPlaySfx(actorReloading, NULL, ammo,
                               reloadResult_fumble, false);
          } else {
            result = reloadResult_success;
            isClip = ammo->getData().isAmmoClip;

            //If ammo comes in clips
            if(isClip) {
              const int previousAmmoCount = wpn->nrAmmoLoaded;
              ItemAmmoClip* clipItem = dynamic_cast<ItemAmmoClip*>(ammo);
              wpn->nrAmmoLoaded = clipItem->ammo;

              printMsgAndPlaySfx(actorReloading, wpn, ammo, result,
                                 isSwiftReload);

              //Erase loaded clip
              inv->deleteItemInGeneralWithElement(i);

              //If weapon previously contained ammo, create a new clip item
              if(previousAmmoCount > 0) {
                ammo = eng->itemFactory->spawnItem(ammoType);
                clipItem = dynamic_cast<ItemAmmoClip*>(ammo);
                clipItem->ammo = previousAmmoCount;
                inv->putItemInGeneral(clipItem);
              }
            }
            //Else ammo is a pile
            else {
              wpn->nrAmmoLoaded += 1;

              printMsgAndPlaySfx(actorReloading, wpn, ammo, result,
                                 isSwiftReload);

              //Decrease ammo item number
              inv->decreaseItemInGeneral(i);
            }
          }
          break;
        }
      }
      if(result == reloadResult_noAmmo) {
        printMsgAndPlaySfx(actorReloading, wpn, ammo, result,
                           isSwiftReload);
      }
    } else {
      result = reloadResult_alreadyFull;
      printMsgAndPlaySfx(actorReloading, wpn, ammo, result,
                         isSwiftReload);
    }
  }

  if(result == reloadResult_success || result == reloadResult_fumble) {
    didAct = true;
    if(isSwiftReload == false) {
      eng->gameTime->endTurnOfCurrentActor();
    }
  }

  return didAct;
}

